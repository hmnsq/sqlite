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
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_Code_Between(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ),ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Code_Vector(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
ವರ್ಣ sqlite3TableColumnAffinity(ಧ್ರುವ Table *pTab, ಪೂರ್ಣಾಙ್ಕ iCol){
  ಯದಿ( iCol<0 || NEVER(iCol>=pTab->nCol) ) ಪ್ರತ್ಯಾಗಮ SQLITE_AFF_INTEGER;
  ಪ್ರತ್ಯಾಗಮ pTab->aCol[iCol].affinity;
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
ವರ್ಣ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ op;
  op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಯಾವತ್( 1 /* exit-by-break */ ){
    ಯದಿ( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0) ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0 );
      ಪ್ರತ್ಯಾಗಮ sqlite3TableColumnAffinity(p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn);
    }
    ಯದಿ( op==TK_SELECT ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
#ifndef SQLITE_OMIT_CAST
    ಯದಿ( op==TK_CAST ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಪ್ರತ್ಯಾಗಮ sqlite3AffinityType(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, 0);
    }
#endif
    ಯದಿ( op==TK_SELECT_COLUMN ){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ!=0 && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iColumn < p_ಅಭಿವ್ಯಕ್ತಿ->iTable );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iColumn >= 0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable==p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
      ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(
          p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pSelect->pEList->a[p_ಅಭಿವ್ಯಕ್ತಿ->iColumn].p_ಅಭಿವ್ಯಕ್ತಿ
      );
    }
    ಯದಿ( op==TK_VECTOR
     || (op==TK_FUNCTION && p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==SQLITE_AFF_DEFER)
    ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Skip|EP_IfNullRow) ){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE
           || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ROW
           || (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_REGISTER && p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_IF_NULL_ROW) );
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
      ಅನುವರ್ತ;
    }
    ಯದಿ( op!=TK_REGISTER ) ಭಙ್ಗ;
    op = p_ಅಭಿವ್ಯಕ್ತಿ->op2;
    ಯದಿ( NEVER( op==TK_REGISTER ) ) ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Data_Type(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯಾವತ್( p_ಅಭಿವ್ಯಕ್ತಿ ){
    ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
      ಪಕ್ಷ TK_COLLATE:
      ಪಕ್ಷ TK_IF_NULL_ROW:
      ಪಕ್ಷ TK_UPLUS:  {
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_NULL: {
        p_ಅಭಿವ್ಯಕ್ತಿ = 0;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_STRING: {
        ಪ್ರತ್ಯಾಗಮ 0x02;
      }
      ಪಕ್ಷ TK_BLOB: {
        ಪ್ರತ್ಯಾಗಮ 0x04;
      }
      ಪಕ್ಷ TK_CONCAT: {
        ಪ್ರತ್ಯಾಗಮ 0x06;
      }
      ಪಕ್ಷ TK_VARIABLE:
      ಪಕ್ಷ TK_AGG_FUNCTION:
      ಪಕ್ಷ TK_FUNCTION: {
        ಪ್ರತ್ಯಾಗಮ 0x07;
      }
      ಪಕ್ಷ TK_COLUMN:
      ಪಕ್ಷ TK_AGG_COLUMN:
      ಪಕ್ಷ TK_SELECT:
      ಪಕ್ಷ TK_CAST:
      ಪಕ್ಷ TK_SELECT_COLUMN:
      ಪಕ್ಷ TK_VECTOR:  {
        ಪೂರ್ಣಾಙ್ಕ aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ);
        ಯದಿ( aff>=SQLITE_AFF_NUMERIC ) ಪ್ರತ್ಯಾಗಮ 0x05;
        ಯದಿ( aff==SQLITE_AFF_TEXT )    ಪ್ರತ್ಯಾಗಮ 0x06;
        ಪ್ರತ್ಯಾಗಮ 0x07;
      }
      ಪಕ್ಷ TK_CASE: {
        ಪೂರ್ಣಾಙ್ಕ res = 0;
        ಪೂರ್ಣಾಙ್ಕ ii;
        ಅಭಿವ್ಯಕ್ತಿ_List *pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) && pList!=0 );
        ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ > 0);
        ಪ್ರತಿ(ii=1; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii+=2){
          res |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Data_Type(pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ);
        }
        ಯದಿ( pList->n_ಅಭಿವ್ಯಕ್ತಿ % 2 ){
          res |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Data_Type(pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ-1].p_ಅಭಿವ್ಯಕ್ತಿ);
        }
        ಪ್ರತ್ಯಾಗಮ res;
      }
      ಸ್ವಭಾವ: {
        ಪ್ರತ್ಯಾಗಮ 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  ಪ್ರತ್ಯಾಗಮ 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,             /* Add the "COLLATE" clause to this expression */
  ಧ್ರುವ Token *pCollName,  /* Name of collating sequence */
  ಪೂರ್ಣಾಙ್ಕ dequote              /* True to dequote pCollName */
){
  ಯದಿ( pCollName->n>0 && pParse->nErr==0 ){
    ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
    ಯದಿ( pNew ){
      pNew->p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ;
      pNew->flags |= EP_Collate|EP_Skip;
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
      pNew->nHeight = p_ಅಭಿವ್ಯಕ್ತಿ->nHeight+1;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_ಅಭಿವ್ಯಕ್ತಿ = pNew;
    }
  }
  ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ;
}
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* Add the "COLLATE" clause to this expression */
  ಧ್ರುವ ವರ್ಣ *zC        /* The collating sequence name */
){
  Token s;
  ಪ್ರತಿಜ್ಞಾ( zC!=0 );
  sqlite3TokenInit(&s, (ವರ್ಣ*)zC);
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_Token(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯಾವತ್( p_ಅಭಿವ್ಯಕ್ತಿ && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Skip) ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE );
    p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  }  
  ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯಾವತ್( p_ಅಭಿವ್ಯಕ್ತಿ && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Skip|EP_Unlikely) ){
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Unlikely) ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
    }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
    }ಅನ್ಯಥಾ{
      ಭಙ್ಗ;
    }
  }  
  ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ;
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
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(Parse *pParse, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಯಾವತ್( p ){
    ಪೂರ್ಣಾಙ್ಕ op = p->op;
    ಯದಿ( op==TK_REGISTER ) op = p->op2;
    ಯದಿ( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      ಪೂರ್ಣಾಙ್ಕ j;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p) );
      ಪ್ರತಿಜ್ಞಾ( p->y.pTab!=0 );
      ಯದಿ( (j = p->iColumn)>=0 ){
        ಧ್ರುವ ವರ್ಣ *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      ಭಙ್ಗ;
    }
    ಯದಿ( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_ಎಡ;
      ಅನುವರ್ತ;
    }
    ಯದಿ( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_ಅಭಿವ್ಯಕ್ತಿ==SQLITE_AFF_DEFER)
    ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      p = p->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಅನುವರ್ತ;
    }
    ಯದಿ( op==TK_COLLATE ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      ಭಙ್ಗ;
    }
    ಯದಿ( p->flags & EP_Collate ){
      ಯದಿ( p->p_ಎಡ && (p->p_ಎಡ->flags & EP_Collate)!=0 ){
        p = p->p_ಎಡ;
      }ಅನ್ಯಥಾ{
        ಅಭಿವ್ಯಕ್ತಿ *pNext  = p->p_ಬಲ;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) || p->x.pList==0 || p->p_ಬಲ==0 );
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          ಪೂರ್ಣಾಙ್ಕ i;
          ಪ್ರತಿ(i=0; i<p->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
            ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, EP_Collate) ){
              pNext = p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
              ಭಙ್ಗ;
            }
          }
        }
        p = pNext;
      }
    }ಅನ್ಯಥಾ{
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  ಪ್ರತ್ಯಾಗಮ pColl;
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
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(Parse *pParse, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  CollSeq *p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಯದಿ( p==0 ) p = pParse->db->pDfltColl;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq_Match(Parse *pParse, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pE1, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pE2){
  CollSeq *pColl1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(pParse, pE2);
  ಪ್ರತಿಜ್ಞಾ( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  ಪ್ರತ್ಯಾಗಮ pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
ವರ್ಣ sqlite3CompareAffinity(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ವರ್ಣ aff2){
  ವರ್ಣ aff1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಯದಿ( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    ಯದಿ( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_AFF_NUMERIC;
    }ಅನ್ಯಥಾ{
      ಪ್ರತ್ಯಾಗಮ SQLITE_AFF_BLOB;
    }
  }ಅನ್ಯಥಾ{
    /* One side is a column, the other is not. Use the columns affinity. */
    ಪ್ರತಿಜ್ಞಾ( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    ಪ್ರತ್ಯಾಗಮ (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
ಸ್ಥಿರ ವರ್ಣ comparisonAffinity(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ವರ್ಣ aff;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EQ || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IN || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LT ||
          p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GT || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LE ||
          p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
  aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ ){
    aff = sqlite3CompareAffinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, aff);
  }ಅನ್ಯಥಾ ಯದಿ( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  ಪ್ರತ್ಯಾಗಮ aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3IndexAffinityOk(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ವರ್ಣ idx_affinity){
  ವರ್ಣ aff = comparisonAffinity(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಯದಿ( aff<SQLITE_AFF_TEXT ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಯದಿ( aff==SQLITE_AFF_TEXT ){
    ಪ್ರತ್ಯಾಗಮ idx_affinity==SQLITE_AFF_TEXT;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ಸ್ಥಿರ u8 ದ್ವಿಮಾನ_Compare_P5(
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pExpr1,   /* Left operand */
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pExpr2,   /* Right operand */
  ಪೂರ್ಣಾಙ್ಕ jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (ವರ್ಣ)sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  ಪ್ರತ್ಯಾಗಮ aff;
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
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ
){
  CollSeq *pColl;
  ಪ್ರತಿಜ್ಞಾ( p_ಎಡ );
  ಯದಿ( p_ಎಡ->flags & EP_Collate ){
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಎಡ);
  }ಅನ್ಯಥಾ ಯದಿ( p_ಬಲ && (p_ಬಲ->flags & EP_Collate)!=0 ){
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಬಲ);
  }ಅನ್ಯಥಾ{
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಎಡ);
    ಯದಿ( !pColl ){
      pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಬಲ);
    }
  }
  ಪ್ರತ್ಯಾಗಮ pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(Parse *pParse, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p){
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Commuted) ){
    ಪ್ರತ್ಯಾಗಮ sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p->p_ಬಲ, p->p_ಎಡ);
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p->p_ಎಡ, p->p_ಬಲ);
  }
}

/*
** Generate code for a comparison operator.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ,      /* The left operand */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ,     /* The right operand */
  ಪೂರ್ಣಾಙ್ಕ opcode,       /* The comparison opcode */
  ಪೂರ್ಣಾಙ್ಕ in1, ಪೂರ್ಣಾಙ್ಕ in2, /* Register holding operands */
  ಪೂರ್ಣಾಙ್ಕ dest,         /* Jump here if true.  */
  ಪೂರ್ಣಾಙ್ಕ jumpIfNull,   /* If true, jump if either operand is NULL */
  ಪೂರ್ಣಾಙ್ಕ isCommuted    /* The comparison has been commuted */
){
  ಪೂರ್ಣಾಙ್ಕ p5;
  ಪೂರ್ಣಾಙ್ಕ addr;
  CollSeq *p4;

  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( isCommuted ){
    p4 = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p_ಬಲ, p_ಎಡ);
  }ಅನ್ಯಥಾ{
    p4 = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p_ಎಡ, p_ಬಲ);
  }
  p5 = ದ್ವಿಮಾನ_Compare_P5(p_ಎಡ, p_ಬಲ, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (ಶೂನ್ಯ*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  ಪ್ರತ್ಯಾಗಮ addr;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Vector(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  u8 op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಯದಿ( op==TK_REGISTER ) op = p_ಅಭಿವ್ಯಕ್ತಿ->op2;
  ಯದಿ( op==TK_VECTOR ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  }ಅನ್ಯಥಾ ಯದಿ( op==TK_SELECT ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 1;
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3VectorFieldSubexpr(ಅಭಿವ್ಯಕ್ತಿ *pVector, ಪೂರ್ಣಾಙ್ಕ i){
  ಪ್ರತಿಜ್ಞಾ( i<sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(pVector) || pVector->op==TK_ERROR );
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Vector(pVector) ){
    ಪ್ರತಿಜ್ಞಾ( pVector->op2==0 || pVector->op==TK_REGISTER );
    ಯದಿ( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pVector) );
      ಪ್ರತ್ಯಾಗಮ pVector->x.pSelect->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pVector) );
      ಪ್ರತ್ಯಾಗಮ pVector->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ pVector;
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  ಪೂರ್ಣಾಙ್ಕ iField,          /* Which column of the vector to return */
  ಪೂರ್ಣಾಙ್ಕ nField           /* Total number of columns in the vector */
){
  ಅಭಿವ್ಯಕ್ತಿ *pRet;
  ಯದಿ( pVector->op==TK_SELECT ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pVector) );
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
    ಯದಿ( pRet ){
      ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_ಎಡ = pVector;
    }
  }ಅನ್ಯಥಾ{
    ಯದಿ( pVector->op==TK_VECTOR ){
      ಅಭಿವ್ಯಕ್ತಿ **ppVector;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_ಅಭಿವ್ಯಕ್ತಿ;
      pVector = *ppVector;
      ಯದಿ( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        ಪ್ರತ್ಯಾಗಮ pVector;
      }
    }
    pRet = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, pVector, 0);
  }
  ಪ್ರತ್ಯಾಗಮ pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Code_Subselect(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ reg;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Vector_Register(
  Parse *pParse,                  /* Parse context */
  ಅಭಿವ್ಯಕ್ತಿ *pVector,                  /* Vector to extract element from */
  ಪೂರ್ಣಾಙ್ಕ iField,                     /* Field to extract from pVector */
  ಪೂರ್ಣಾಙ್ಕ regSelect,                  /* First in array of registers */
  ಅಭಿವ್ಯಕ್ತಿ *pTmp,                     /* Temporary space */
  ಅಭಿವ್ಯಕ್ತಿ **pp_ಅಭಿವ್ಯಕ್ತಿ,                  /* OUT: Expression element */
  ಪೂರ್ಣಾಙ್ಕ *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  ಪ್ರತಿಜ್ಞಾ( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  ಯದಿ( op==TK_REGISTER ){
    *pp_ಅಭಿವ್ಯಕ್ತಿ = sqlite3VectorFieldSubexpr(pVector, iField);
    ಪ್ರತ್ಯಾಗಮ pVector->iTable+iField;
  }
  ಯದಿ( op==TK_SELECT ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_ಎಡ = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
    *pp_ಅಭಿವ್ಯಕ್ತಿ = pTmp;
    ಪ್ರತ್ಯಾಗಮ regSelect+iField;
  }
  ಯದಿ( op==TK_VECTOR ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pVector) );
    *pp_ಅಭಿವ್ಯಕ್ತಿ = pVector->x.pList->a[iField].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, *pp_ಅಭಿವ್ಯಕ್ತಿ, pRegFree);
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* The comparison operation */
  ಪೂರ್ಣಾಙ್ಕ dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
  ಪೂರ್ಣಾಙ್ಕ n_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಎಡ);
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ reg_ಎಡ = 0;
  ಪೂರ್ಣಾಙ್ಕ reg_ಬಲ = 0;
  u8 opx = op;
  ಪೂರ್ಣಾಙ್ಕ addrCmp = 0;
  ಪೂರ್ಣಾಙ್ಕ addrDone = sqlite3VdbeMakeLabel(pParse);
  ಪೂರ್ಣಾಙ್ಕ isCommuted = ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Commuted);

  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( n_ಎಡ!=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಬಲ) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
    ಪ್ರತ್ಯಾಗಮ;
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

  ಯದಿ( op==TK_LE ) opx = TK_LT;
  ಯದಿ( op==TK_GE ) opx = TK_GT;
  ಯದಿ( op==TK_NE ) opx = TK_EQ;

  reg_ಎಡ = ಅಭಿವ್ಯಕ್ತಿ_Code_Subselect(pParse, p_ಎಡ);
  reg_ಬಲ = ಅಭಿವ್ಯಕ್ತಿ_Code_Subselect(pParse, p_ಬಲ);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  ಪ್ರತಿ(i=0; 1 /*Loop exits by "break"*/; i++){
    ಪೂರ್ಣಾಙ್ಕ regFree1 = 0, regFree2 = 0;
    ಅಭಿವ್ಯಕ್ತಿ *pL = 0, *pR = 0;
    ಅಭಿವ್ಯಕ್ತಿ tmp1, tmp2;
    ಪೂರ್ಣಾಙ್ಕ r1, r2;
    ಪ್ರತಿಜ್ಞಾ( i>=0 && i<n_ಎಡ );
    ಯದಿ( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = ಅಭಿವ್ಯಕ್ತಿ_Vector_Register(pParse, p_ಎಡ, i, reg_ಎಡ, &tmp1, &pL, &regFree1);
    r2 = ಅಭಿವ್ಯಕ್ತಿ_Vector_Register(pParse, p_ಬಲ, i, reg_ಬಲ, &tmp2, &pR, &regFree2);
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
    ಯದಿ( (opx==TK_LT || opx==TK_GT) && i<n_ಎಡ-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    ಯದಿ( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }ಅನ್ಯಥಾ{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    ಯದಿ( i==n_ಎಡ-1 ){
      ಭಙ್ಗ;
    }
    ಯದಿ( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      ಯದಿ( i==n_ಎಡ-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  ಯದಿ( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_Height(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ nHeight){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ mxHeight = pParse->db->aLimit[SQLITE_LIMIT_ಅಭಿವ್ಯಕ್ತಿ_DEPTH];
  ಯದಿ( nHeight>mxHeight ){
    sqlite3_Error_ಸಂದೇಶ(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಸ್ಥಿರ ಶೂನ್ಯ height_Of_ಅಭಿವ್ಯಕ್ತಿ(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ *pnHeight){
  ಯದಿ( p ){
    ಯದಿ( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ height_Of_ಅಭಿವ್ಯಕ್ತಿ_List(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *p, ಪೂರ್ಣಾಙ್ಕ *pnHeight){
  ಯದಿ( p ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<p->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      height_Of_ಅಭಿವ್ಯಕ್ತಿ(p->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, pnHeight);
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ heightOfSelect(ಧ್ರುವ Select *pSelect, ಪೂರ್ಣಾಙ್ಕ *pnHeight){
  ಧ್ರುವ Select *p;
  ಪ್ರತಿ(p=pSelect; p; p=p->pPrior){
    height_Of_ಅಭಿವ್ಯಕ್ತಿ(p->pWhere, pnHeight);
    height_Of_ಅಭಿವ್ಯಕ್ತಿ(p->pHaving, pnHeight);
    height_Of_ಅಭಿವ್ಯಕ್ತಿ(p->pLimit, pnHeight);
    height_Of_ಅಭಿವ್ಯಕ್ತಿ_List(p->pEList, pnHeight);
    height_Of_ಅಭಿವ್ಯಕ್ತಿ_List(p->pGroupBy, pnHeight);
    height_Of_ಅಭಿವ್ಯಕ್ತಿ_List(p->pOrderBy, pnHeight);
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
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_Set_Height(ಅಭಿವ್ಯಕ್ತಿ *p){
  ಪೂರ್ಣಾಙ್ಕ nHeight = p->p_ಎಡ ? p->p_ಎಡ->nHeight : 0;
  ಯದಿ( NEVER(p->p_ಬಲ) && p->p_ಬಲ->nHeight>nHeight ){
    nHeight = p->p_ಬಲ->nHeight;
  }
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }ಅನ್ಯಥಾ ಯದಿ( p->x.pList ){
    height_Of_ಅಭಿವ್ಯಕ್ತಿ_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Flags(p->x.pList);
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Set_Height_And_Flags(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  ಅಭಿವ್ಯಕ್ತಿ_Set_Height(p);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_Select_ಅಭಿವ್ಯಕ್ತಿ_Height(ಧ್ರುವ Select *p){
  ಪೂರ್ಣಾಙ್ಕ nHeight = 0;
  heightOfSelect(p, &nHeight);
  ಪ್ರತ್ಯಾಗಮ nHeight;
}
#ಅನ್ಯಥಾ /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Set_Height_And_Flags(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( p && ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Flags(p->x.pList);
  }
}
#define ಅಭಿವ್ಯಕ್ತಿ_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Set_Error_Offset(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ iOfst){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( NEVER(ಅಭಿವ್ಯಕ್ತಿ_Use_WJoin(p_ಅಭಿವ್ಯಕ್ತಿ)) ) ಪ್ರತ್ಯಾಗಮ;
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
  ಪೂರ್ಣಾಙ್ಕ op,                 /* Expression opcode */
  ಧ್ರುವ Token *pToken,    /* Token argument.  Might be NULL */
  ಪೂರ್ಣಾಙ್ಕ dequote             /* True to dequote */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  ಪೂರ್ಣಾಙ್ಕ nExtra = pToken ? pToken->n+1 : 0;

  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ)+nExtra);
  ಯದಿ( pNew ){
    memset(pNew, 0, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    ಯದಿ( nExtra ){
      ಪ್ರತಿಜ್ಞಾ( pToken!=0 );
      pNew->u.zToken = (ವರ್ಣ*)&pNew[1];
      ಪ್ರತಿಜ್ಞಾ( pToken->z!=0 || pToken->n==0 );
      ಯದಿ( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      ಯದಿ( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_ಅಭಿವ್ಯಕ್ತಿ(pNew);
      }
    }
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  ಪೂರ್ಣಾಙ್ಕ op,                 /* Expression opcode */
  ಧ್ರುವ ವರ್ಣ *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ iVal){
  ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3DbMallocRawNN(db, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
  ಯದಿ( pNew ){
    memset(pNew, 0, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_ಮೌಲ್ಯ|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_ಮೌಲ್ಯ = iVal;
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
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
  ಯದಿ( p_ಬೇರು==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಎಡ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಬಲ);
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಬೇರು) );
    ಪ್ರತಿಜ್ಞಾ( p_ಬೇರು->x.pSelect==0 );
    ಯದಿ( p_ಬಲ ){
      p_ಬೇರು->p_ಬಲ = p_ಬಲ;
      p_ಬೇರು->flags |= EP_Propagate & p_ಬಲ->flags;
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
      p_ಬೇರು->nHeight = p_ಬಲ->nHeight+1;
    }ಅನ್ಯಥಾ{
      p_ಬೇರು->nHeight = 1;
#endif
    }
    ಯದಿ( p_ಎಡ ){
      p_ಬೇರು->p_ಎಡ = p_ಎಡ;
      p_ಬೇರು->flags |= EP_Propagate & p_ಎಡ->flags;
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
      ಯದಿ( p_ಎಡ->nHeight>=p_ಬೇರು->nHeight ){
        p_ಬೇರು->nHeight = p_ಎಡ->nHeight+1;
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  ಪೂರ್ಣಾಙ್ಕ op,                 /* Expression opcode */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ,            /* Left operand */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ            /* Right operand */
){
  ಅಭಿವ್ಯಕ್ತಿ *p;
  p = sqlite3DbMallocRawNN(pParse->db, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
  ಯದಿ( p ){
    memset(p, 0, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Attach_Subtrees(pParse->db, p, p_ಎಡ, p_ಬಲ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_Height(pParse, p->nHeight);
  }ಅನ್ಯಥಾ{
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಎಡ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಬಲ);
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
ಶೂನ್ಯ sqlite3PExprAddSelect(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, Select *pSelect){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ ){
    p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect = pSelect;
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect|EP_Subquery);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Set_Height_And_Flags(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  }ಅನ್ಯಥಾ{
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
Select *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_To_ಮೌಲ್ಯಗಳು(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ nElem, ಅಭಿವ್ಯಕ್ತಿ_List *pEList){
  ಪೂರ್ಣಾಙ್ಕ ii;
  Select *pRet = 0;
  ಪ್ರತಿಜ್ಞಾ( nElem>1 );
  ಪ್ರತಿ(ii=0; ii<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
    Select *pSel;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪೂರ್ಣಾಙ್ಕ n_ಅಭಿವ್ಯಕ್ತಿ_Elem;
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_VECTOR ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      n_ಅಭಿವ್ಯಕ್ತಿ_Elem = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
    }ಅನ್ಯಥಾ{
      n_ಅಭಿವ್ಯಕ್ತಿ_Elem = 1;
    }
    ಯದಿ( n_ಅಭಿವ್ಯಕ್ತಿ_Elem!=nElem ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "IN(...) element has %d term%s - expected %d",
          n_ಅಭಿವ್ಯಕ್ತಿ_Elem, n_ಅಭಿವ್ಯಕ್ತಿ_Elem>1?"s":"", nElem
      );
      ಭಙ್ಗ;
    }
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    pSel = sqlite3SelectNew(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, 0, 0, 0, 0, 0, SF_ಮೌಲ್ಯಗಳು,0);
    p_ಅಭಿವ್ಯಕ್ತಿ->x.pList = 0;
    ಯದಿ( pSel ){
      ಯದಿ( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  ಯದಿ( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_ಮೌಲ್ಯ;
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(pParse->db, pEList);
  ಪ್ರತ್ಯಾಗಮ pRet;
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
  ಯದಿ( p_ಎಡ==0  ){
    ಪ್ರತ್ಯಾಗಮ p_ಬಲ;
  }ಅನ್ಯಥಾ ಯದಿ( p_ಬಲ==0 ){
    ಪ್ರತ್ಯಾಗಮ p_ಎಡ;
  }ಅನ್ಯಥಾ{
    u32 f = p_ಎಡ->flags | p_ಬಲ->flags;
    ಯದಿ( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      ಅಭಿವ್ಯಕ್ತಿ *p;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಎಡ);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಬಲ);
      p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, 0);
      ಯದಿ( p ) p->op2 = 1;
      ಪ್ರತ್ಯಾಗಮ p;
    }ಅನ್ಯಥಾ{
      ಪ್ರತ್ಯಾಗಮ sqlite3PExpr(pParse, TK_AND, p_ಎಡ, p_ಬಲ);
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
  ಧ್ರುವ Token *pToken,  /* Name of the function */
  ಪೂರ್ಣಾಙ್ಕ eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( pToken );
  pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_FUNCTION, pToken, 1);
  ಯದಿ( pNew==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (ಪೂರ್ಣಾಙ್ಕ)(pToken->z - pParse->zTail);
  ಯದಿ( pList
   && pList->n_ಅಭಿವ್ಯಕ್ತಿ > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pNew, EP_HasFunc);
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pNew) );
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Set_Height_And_Flags(pParse, pNew);
  ಯದಿ( eDistinct==SF_Distinct ) ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pNew, EP_Distinct);
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Order_By_Aggregate_Error(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* The function call to which ORDER BY is to be added */
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy    /* The ORDER BY clause to add */
){
  ಅಭಿವ್ಯಕ್ತಿ *pOB;
  sqlite3 *db = pParse->db;
  ಯದಿ( NEVER(pOrderBy==0) ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pOrderBy);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0 );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==0 || NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic, pOrderBy);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( IsWindowFunc(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Order_By_Aggregate_Error(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pOrderBy);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many terms in ORDER BY clause");
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pOrderBy);
    ಪ್ರತ್ಯಾಗಮ;
  }

  pOB = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ORDER, 0, 0);
  ಯದಿ( pOB==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pOrderBy);
    ಪ್ರತ್ಯಾಗಮ;
  }
  pOB->x.pList = pOrderBy;
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pOB) );
  p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = pOB;
  ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pOB, EP_FullSize);
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
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,     /* The function invocation */
  ಧ್ರುವ FuncDef *pDef    /* The function being invoked */
){
  ಪ್ರತಿಜ್ಞಾ( !IN_RENAME_OBJECT );
  ಪ್ರತಿಜ್ಞಾ( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    ಯದಿ( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Assign_Var_ಸಂಖ್ಯೆ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, u32 n){
  sqlite3 *db = pParse->db;
  ಧ್ರುವ ವರ್ಣ *z;
  ynVar x;

  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ|EP_Reduced|EP_TokenOnly) );
  z = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
  ಪ್ರತಿಜ್ಞಾ( z!=0 );
  ಪ್ರತಿಜ್ಞಾ( z[0]!=0 );
  ಪ್ರತಿಜ್ಞಾ( n==(u32)sqlite3Strlen30(z) );
  ಯದಿ( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    ಪ್ರತಿಜ್ಞಾ( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ doAdd = 0;
    ಯದಿ( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      ಪೂರ್ಣಾಙ್ಕ bOk;
      ಯದಿ( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }ಅನ್ಯಥಾ{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_ಸಂಖ್ಯೆ]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_ಸಂಖ್ಯೆ] );
      ಯದಿ( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_ಸಂಖ್ಯೆ] ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_ಸಂಖ್ಯೆ]);
        sqlite3_Record_Error_Offset_Of_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
        ಪ್ರತ್ಯಾಗಮ;
      }
      x = (ynVar)i;
      ಯದಿ( x>pParse->nVar ){
        pParse->nVar = (ಪೂರ್ಣಾಙ್ಕ)x;
        doAdd = 1;
      }ಅನ್ಯಥಾ ಯದಿ( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }ಅನ್ಯಥಾ ಯದಿ( sqlite3_VList_ಅಂಕೆ_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }ಅನ್ಯಥಾ{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_ಅಂಕೆ(pParse->pVList, z, n);
      ಯದಿ( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    ಯದಿ( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      ಯದಿ( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_ಅಭಿವ್ಯಕ್ತಿ->iColumn = x;
  ಯದಿ( x>db->aLimit[SQLITE_LIMIT_VARIABLE_ಸಂಖ್ಯೆ] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
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
#ifdef SQLITE_DEBUG
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Leaf) && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly) ){
    ಪ್ರತಿಜ್ಞಾ( p->p_ಎಡ==0 );
    ಪ್ರತಿಜ್ಞಾ( p->p_ಬಲ==0 );
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) || p->x.pSelect==0 );
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) || p->x.pList==0 );
  }
#endif
  ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    ಪ್ರತಿಜ್ಞಾ( (ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) && p->x.pList==0) || p->p_ಬಲ==0 );
    ಯದಿ( p->p_ಬಲ ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_WinFunc) );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, p->p_ಬಲ);
    }ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }ಅನ್ಯಥಾ{
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    ಯದಿ( p->p_ಎಡ && p->op!=TK_SELECT_COLUMN ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p->p_ಎಡ;
      ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Static)
       && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಎಡ, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_ಎಡ;
        ಗಚ್ಛ ಅಭಿವ್ಯಕ್ತಿ_Delete_Restart;
      }ಅನ್ಯಥಾ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, p_ಎಡ);
      }
    }
  }
  ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಯದಿ( p ) sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, p);
}
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_Generic(sqlite3 *db, ಶೂನ್ಯ *p){
  ಯದಿ( ALWAYS(p) ) sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, (ಅಭಿವ್ಯಕ್ತಿ*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
ಶೂನ್ಯ sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  ಯದಿ( p==0 ){
    /* Nothing to clear */
  }ಅನ್ಯಥಾ ಯದಿ( p->pOn ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, p->pOn);
  }ಅನ್ಯಥಾ ಯದಿ( p->pUsing ){
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪ್ರತ್ಯಾಗಮ 0==sqlite3ParserAddCleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_Generic, p_ಅಭಿವ್ಯಕ್ತಿ);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Unmap_And_Delete(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಯದಿ( p ){
    ಯದಿ( IN_RENAME_OBJECT ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Struct_Size(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p){
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly) ) ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_TOKENONLYSIZE;
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Reduced) ) ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_REDUCEDSIZE;
  ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ duped_ಅಭಿವ್ಯಕ್ತಿ_Struct_Size(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪೂರ್ಣಾಙ್ಕ nSize;
  ಪ್ರತಿಜ್ಞಾ( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE<=0xfff );
  ಪ್ರತಿಜ್ಞಾ( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  ಯದಿ( 0==flags || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_FullSize) ){
    nSize = ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_OuterON) );
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p, EP_NoReduce) );
    ಯದಿ( p->p_ಎಡ || p->x.pList ){
      nSize = ಅಭಿವ್ಯಕ್ತಿ_REDUCEDSIZE | EP_Reduced;
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( p->p_ಬಲ==0 );
      nSize = ಅಭಿವ್ಯಕ್ತಿ_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  ಪ್ರತ್ಯಾಗಮ nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ duped_ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Size(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪೂರ್ಣಾಙ್ಕ nByte = duped_ಅಭಿವ್ಯಕ್ತಿ_Struct_Size(p, flags) & 0xfff;
  ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  ಪ್ರತ್ಯಾಗಮ ROUND8(nByte);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ duped_ಅಭಿವ್ಯಕ್ತಿ_Size(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p){
  ಪೂರ್ಣಾಙ್ಕ nByte;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  nByte = duped_ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Size(p, EXPRDUP_REDUCE);
  ಯದಿ( p->p_ಎಡ ) nByte += duped_ಅಭಿವ್ಯಕ್ತಿ_Size(p->p_ಎಡ);
  ಯದಿ( p->p_ಬಲ ) nByte += duped_ಅಭಿವ್ಯಕ್ತಿ_Size(p->p_ಬಲ);
  ಪ್ರತಿಜ್ಞಾ( nByte==ROUND8(nByte) );
  ಪ್ರತ್ಯಾಗಮ nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ EdupBuf EdupBuf;
ಸಂರಚನಾ EdupBuf {
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
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ *ಅಭಿವ್ಯಕ್ತಿ_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p,        /* Expr tree to be duplicated */
  ಪೂರ್ಣಾಙ್ಕ dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  ಪೂರ್ಣಾಙ್ಕ nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  ಪ್ರತಿಜ್ಞಾ( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  ಯದಿ( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    ಪ್ರತಿಜ್ಞಾ( sEdupBuf.zAlloc!=0 );
    ಪ್ರತಿಜ್ಞಾ( dupFlags==EXPRDUP_REDUCE );
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ nAlloc;
    ಯದಿ( dupFlags ){
      nAlloc = duped_ಅಭಿವ್ಯಕ್ತಿ_Size(p);
    }ಅನ್ಯಥಾ ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE + nToken);
    }ಅನ್ಯಥಾ{
      nToken = 0;
      nAlloc = ROUND8(ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE);
    }
    ಪ್ರತಿಜ್ಞಾ( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (ಅಭಿವ್ಯಕ್ತಿ *)sEdupBuf.zAlloc;
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pNew) );

  ಯದಿ( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ಧ್ರುವ unsigned nStructSize = duped_ಅಭಿವ್ಯಕ್ತಿ_Struct_Size(p, dupFlags);
    ಪೂರ್ಣಾಙ್ಕ nNewSize = nStructSize & 0xfff;
    ಯದಿ( nToken<0 ){
      ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }ಅನ್ಯಥಾ{
        nToken = 0;
      }
    }
    ಯದಿ( dupFlags ){
      ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಙ್ಕ)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }ಅನ್ಯಥಾ{
      u32 nSize = (u32)ಅಭಿವ್ಯಕ್ತಿ_Struct_Size(p);
      ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಙ್ಕ)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (ಪೂರ್ಣಾಙ್ಕ)ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      ಯದಿ( nSize<ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE-nSize);
      }
      nNewSize = ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    ಅಭಿವ್ಯಕ್ತಿ_Clear_VVAProperties(pNew);
    ಯದಿ( dupFlags ){
      ಅಭಿವ್ಯಕ್ತಿ_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    ಪ್ರತಿಜ್ಞಾ( nToken>=0 );
    ಯದಿ( nToken>0 ){
      ವರ್ಣ *zToken = pNew->u.zToken = (ವರ್ಣ*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    ಯದಿ( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }ಅನ್ಯಥಾ{
        pNew->x.pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      ಯದಿ( dupFlags ){
        ಯದಿ( p->op==TK_SELECT_COLUMN ){
          pNew->p_ಎಡ = p->p_ಎಡ;
          ಪ್ರತಿಜ್ಞಾ( p->p_ಬಲ==0 
               || p->p_ಬಲ==p->p_ಎಡ
               || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p->p_ಎಡ, EP_Subquery) );
        }ಅನ್ಯಥಾ{
          pNew->p_ಎಡ = p->p_ಎಡ ?
                      ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಎಡ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_ಬಲ = p->p_ಬಲ ?
                       ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಬಲ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }ಅನ್ಯಥಾ{
        ಯದಿ( p->op==TK_SELECT_COLUMN ){
          pNew->p_ಎಡ = p->p_ಎಡ;
          ಪ್ರತಿಜ್ಞಾ( p->p_ಬಲ==0 
               || p->p_ಬಲ==p->p_ಎಡ
               || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p->p_ಎಡ, EP_Subquery) );
        }ಅನ್ಯಥಾ{
          pNew->p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಎಡ, 0);
        }
        pNew->p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಬಲ, 0);
      }
    }
  }
  ಯದಿ( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, ಪರಿಮಾಣ(sEdupBuf));
  ಪ್ರತಿಜ್ಞಾ( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  ಯದಿ( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    ಯದಿ( pRet ){
      ಪೂರ್ಣಾಙ್ಕ i;
      pRet->nCte = p->nCte;
      ಪ್ರತಿ(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ pRet;
}
#ಅನ್ಯಥಾ
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gatherSelectWindowsCallback(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin;
    ಪ್ರತಿಜ್ಞಾ( pWin );
    ಪ್ರತಿಜ್ಞಾ( IsWindowFunc(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಪ್ರತಿಜ್ಞಾ( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  ಪ್ರತ್ಯಾಗಮ p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(sqlite3 *db, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪ್ರತಿಜ್ಞಾ( flags==0 || flags==EXPRDUP_REDUCE );
  ಪ್ರತ್ಯಾಗಮ p ? ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p, flags, 0) : 0;
}
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(sqlite3 *db, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *p, ಪೂರ್ಣಾಙ್ಕ flags){
  ಅಭಿವ್ಯಕ್ತಿ_List *pNew;
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  ಧ್ರುವ ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pOldItem;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಅಭಿವ್ಯಕ್ತಿ *pPriorSelectColOld = 0;
  ಅಭಿವ್ಯಕ್ತಿ *pPriorSelectColNew = 0;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  pNew->n_ಅಭಿವ್ಯಕ್ತಿ = p->n_ಅಭಿವ್ಯಕ್ತಿ;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  ಪ್ರತಿ(i=0; i<p->n_ಅಭಿವ್ಯಕ್ತಿ; i++, pItem++, pOldItem++){
    ಅಭಿವ್ಯಕ್ತಿ *p_Old_ಅಭಿವ್ಯಕ್ತಿ = pOldItem->p_ಅಭಿವ್ಯಕ್ತಿ;
    ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
    pItem->p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_Old_ಅಭಿವ್ಯಕ್ತಿ, flags);
    ಯದಿ( p_Old_ಅಭಿವ್ಯಕ್ತಿ
     && p_Old_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT_COLUMN
     && (p_New_ಅಭಿವ್ಯಕ್ತಿ = pItem->p_ಅಭಿವ್ಯಕ್ತಿ)!=0
    ){
      ಯದಿ( p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ ){
        pPriorSelectColOld = p_Old_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        pPriorSelectColNew = p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
      }ಅನ್ಯಥಾ{
        ಯದಿ( p_Old_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          pPriorSelectColNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pPriorSelectColOld, flags);
          p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ = pPriorSelectColNew;
        }
        p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#ಯದಿ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ಧ್ರುವ SrcList *p, ಪೂರ್ಣಾಙ್ಕ flags){
  SrcList *pNew;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  ಪ್ರತಿ(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    ಧ್ರುವ SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    ಯದಿ( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, ಪರಿಮಾಣ(Subquery));
      ಯದಿ( pNewSubq==0 ){
        ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }ಅನ್ಯಥಾ{
        memcpy(pNewSubq, pOldItem->u4.pSubq, ಪರಿಮಾಣ(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        ಯದಿ( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }ಅನ್ಯಥಾ ಯದಿ( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }ಅನ್ಯಥಾ{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    ಯದಿ( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }ಅನ್ಯಥಾ ಯದಿ( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }ಅನ್ಯಥಾ{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    ಯದಿ( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    ಯದಿ( pTab ){
      pTab->nTabRef++;
    }
    ಯದಿ( pOldItem->fg.isUsing ){
      ಪ್ರತಿಜ್ಞಾ( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }ಅನ್ಯಥಾ{
      pNewItem->u3.pOn = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ಧ್ರುವ IdList *p){
  IdList *pNew;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  pNew->nId = p->nId;
  ಪ್ರತಿ(i=0; i<p->nId; i++){
    ಸಂರಚನಾ IdList_item *pNewItem = &pNew->a[i];
    ಧ್ರುವ ಸಂರಚನಾ IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ಧ್ರುವ Select *pDup, ಪೂರ್ಣಾಙ್ಕ flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  ಧ್ರುವ Select *p;

  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿ(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, ಪರಿಮಾಣ(*p) );
    ಯದಿ( pNew==0 ) ಭಙ್ಗ;
    pNew->pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    ಯದಿ( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    ಯದಿ( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      ಭಙ್ಗ;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  ಪ್ರತ್ಯಾಗಮ pRet;
}
#ಅನ್ಯಥಾ
Select *sqlite3SelectDup(sqlite3 *db, ಧ್ರುವ Select *p, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪ್ರತಿಜ್ಞಾ( p==0 );
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item zeroItem = {0};
SQLITE_NOINLINE ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ             /* Expression to be appended. Might be NULL */
){
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  ಯದಿ( pList==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pList->nAlloc = 4;
  pList->n_ಅಭಿವ್ಯಕ್ತಿ = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತ್ಯಾಗಮ pList;
}
SQLITE_NOINLINE ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,        /* List to which to append. Might be NULL */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ             /* Expression to be appended. Might be NULL */
){
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  ಅಭಿವ್ಯಕ್ತಿ_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  ಯದಿ( pNew==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ++];
  *pItem = zeroItem;
  pItem->p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತ್ಯಾಗಮ pList;
}
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,        /* List to which to append. Might be NULL */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ             /* Expression to be appended. Might be NULL */
){
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  ಯದಿ( pList==0 ){
    ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_New(pParse->db,p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಯದಿ( pList->nAlloc<pList->n_ಅಭಿವ್ಯಕ್ತಿ+1 ){
    ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_Grow(pParse->db,pList,p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  pItem = &pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ++];
  *pItem = zeroItem;
  pItem->p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತ್ಯಾಗಮ pList;
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
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಙ್ಕ n;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ iFirst = pList ? pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  ಯದಿ( NEVER(pColumns==0) ) ಗಚ್ಛ vector_append_error;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಗಚ್ಛ vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ)) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    ಗಚ್ಛ vector_append_error;
  }

  ಪ್ರತಿ(i=0; i<pColumns->nId; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_Sub_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_Vector_Field(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, i, pColumns->nId);
    ಪ್ರತಿಜ್ಞಾ( p_Sub_ಅಭಿವ್ಯಕ್ತಿ!=0 || db->mallocFailed );
    ಯದಿ( p_Sub_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಅನುವರ್ತ;
    pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList, p_Sub_ಅಭಿವ್ಯಕ್ತಿ);
    ಯದಿ( pList ){
      ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==iFirst+i+1 );
      pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  ಯದಿ( !db->mallocFailed && p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT && ALWAYS(pList!=0) ){
    ಅಭಿವ್ಯಕ್ತಿ *pFirst = pList->a[iFirst].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( pFirst!=0 );
    ಪ್ರತಿಜ್ಞಾ( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ;
    p_ಅಭಿವ್ಯಕ್ತಿ = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Unmap_And_Delete(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  sqlite3IdListDelete(db, pColumns);
  ಪ್ರತ್ಯಾಗಮ pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Set_Sort_Order(ಅಭಿವ್ಯಕ್ತಿ_List *p, ಪೂರ್ಣಾಙ್ಕ iSortOrder, ಪೂರ್ಣಾಙ್ಕ eNulls){
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿಜ್ಞಾ( p->n_ಅಭಿವ್ಯಕ್ತಿ>0 );

  ಪ್ರತಿಜ್ಞಾ( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  ಪ್ರತಿಜ್ಞಾ( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  ಪ್ರತಿಜ್ಞಾ( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_ಅಭಿವ್ಯಕ್ತಿ-1];
  ಪ್ರತಿಜ್ಞಾ( pItem->fg.bNulls==0 );
  ಯದಿ( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  ಯದಿ( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    ಯದಿ( iSortOrder!=eNulls ){
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,        /* List to which to add the span. */
  ಧ್ರುವ Token *pName,     /* Name to be added */
  ಪೂರ್ಣಾಙ್ಕ dequote             /* True to cause the name to be dequoted */
){
  ಪ್ರತಿಜ್ಞಾ( pList!=0 || pParse->db->mallocFailed!=0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  ಯದಿ( pList ){
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
    ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
    pItem = &pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ-1];
    ಪ್ರತಿಜ್ಞಾ( pItem->zEName==0 );
    ಪ್ರತಿಜ್ಞಾ( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    ಯದಿ( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      ಯದಿ( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ಧ್ರುವ ಶೂನ್ಯ*)pItem->zEName, pName);
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,        /* List to which to add the span. */
  ಧ್ರುವ ವರ್ಣ *zStart,     /* Start of the span */
  ಧ್ರುವ ವರ್ಣ *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( pList!=0 || db->mallocFailed!=0 );
  ಯದಿ( pList ){
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem = &pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ-1];
    ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
    ಯದಿ( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Check_Length(
  Parse *pParse,
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,
  ಧ್ರುವ ವರ್ಣ *zObject
){
  ಪೂರ್ಣಾಙ್ಕ mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_ಅಭಿವ್ಯಕ್ತಿ==mx );
  testcase( pEList && pEList->n_ಅಭಿವ್ಯಕ್ತಿ==mx+1 );
  ಯದಿ( pEList && pEList->n_ಅಭಿವ್ಯಕ್ತಿ>mx ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_List_Delete_NN(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಪೂರ್ಣಾಙ್ಕ i = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem =  pList->a;
  ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಕುರು{
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pItem->p_ಅಭಿವ್ಯಕ್ತಿ);
    ಯದಿ( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }ಯಾವತ್( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಯದಿ( pList ) ಅಭಿವ್ಯಕ್ತಿ_List_Delete_NN(db, pList);
}
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic(sqlite3 *db, ಶೂನ್ಯ *pList){
  ಯದಿ( ALWAYS(pList) ) ಅಭಿವ್ಯಕ್ತಿ_List_Delete_NN(db, (ಅಭಿವ್ಯಕ್ತಿ_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Flags(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಪೂರ್ಣಾಙ್ಕ i;
  u32 m = 0;
  ಪ್ರತಿಜ್ಞಾ( pList!=0 );
  ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
     ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
     ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
     m |= p_ಅಭಿವ್ಯಕ್ತಿ->flags;
  }
  ಪ್ರತ್ಯಾಗಮ m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  ಪ್ರತ್ಯಾಗಮ WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ಧ್ರುವ ವರ್ಣ *zIn){
  ಯದಿ( sqlite3StrICmp(zIn, "true")==0  ) ಪ್ರತ್ಯಾಗಮ EP_IsTrue;
  ಯದಿ( sqlite3StrICmp(zIn, "false")==0 ) ಪ್ರತ್ಯಾಗಮ EP_IsFalse;
  ಪ್ರತ್ಯಾಗಮ 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Id_To_True_False(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  u32 v;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_STRING );
  ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Quoted|EP_Int_ಮೌಲ್ಯ)
   && (v = sqlite3IsTrueOrFalse(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken))!=0
  ){
    p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_TRUEFALSE;
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, v);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely((ಅಭಿವ್ಯಕ್ತಿ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRUEFALSE );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3StrICmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,"true")==0
       || sqlite3StrICmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,"false")==0 );
  ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[4]==0;
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
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_OR ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Always_True(p_ಎಡ) || ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಬಲ) ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND ? p_ಬಲ : p_ಎಡ;
    }ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Always_True(p_ಬಲ) || ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಎಡ) ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND ? p_ಎಡ : p_ಬಲ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_First(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, EP_Subquery)
   && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, EP_Subquery)
  ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,       /* The comparison expression */
  ಪೂರ್ಣಾಙ್ಕ *pR1,          /* OUT: Register holding the left operand */
  ಪೂರ್ಣಾಙ್ಕ *pR2,          /* OUT: Register holding the right operand */
  ಪೂರ್ಣಾಙ್ಕ *pFree1,       /* OUT: Temp register to free if not zero */
  ಪೂರ್ಣಾಙ್ಕ *pFree2        /* OUT: Another temp register to free if not zero */
){
  ಪೂರ್ಣಾಙ್ಕ addrIsNull;
  ಪೂರ್ಣಾಙ್ಕ r1, r2;
  Vdbe *v = pParse->pVdbe;

  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_First(p_ಅಭಿವ್ಯಕ್ತಿ) && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ) ){
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }ಅನ್ಯಥಾ{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, pFree1);
  ಯದಿ( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, EP_Subquery)
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  ಪ್ರತ್ಯಾಗಮ addrIsNull;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant_Function(
  Walker *pWalker,
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ
){
  ಪೂರ್ಣಾಙ್ಕ n;             /* Number of arguments */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly)
   || (pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList)==0
  ){;
    n = 0;
  }ಅನ್ಯಥಾ{
    n = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pList);
    ಯದಿ( pWalker->eCode==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, n, ENC(db), 0);
  ಯದಿ( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Prune;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪ್ರತಿಜ್ಞಾ( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  ಯದಿ( pWalker->eCode==2 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ){
    pWalker->eCode = 0;
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }

  ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಪಕ್ಷ TK_FUNCTION:
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }ಅನ್ಯಥಾ ಯದಿ( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        ಯದಿ( pWalker->eCode==5 ) ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL);
        ಪ್ರತ್ಯಾಗಮ WRC_Continue;
      }ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        ಪ್ರತ್ಯಾಗಮ WRC_Continue;
      }ಅನ್ಯಥಾ ಯದಿ( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant_Function(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
      }ಅನ್ಯಥಾ{
        pWalker->eCode = 0;  /* Function call is not constant */
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
    ಪಕ್ಷ TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Id_To_True_False(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        ಪ್ರತ್ಯಾಗಮ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    ಪಕ್ಷ TK_COLUMN:
    ಪಕ್ಷ TK_AGG_FUNCTION:
    ಪಕ್ಷ TK_AGG_COLUMN:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_COLUMN );
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FixedCol) && pWalker->eCode!=2 ){
        ಪ್ರತ್ಯಾಗಮ WRC_Continue;
      }
      ಯದಿ( pWalker->eCode==3 && p_ಅಭಿವ್ಯಕ್ತಿ->iTable==pWalker->u.iCur ){
        ಪ್ರತ್ಯಾಗಮ WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    ಪಕ್ಷ TK_IF_NULL_ROW:
    ಪಕ್ಷ TK_REGISTER:
    ಪಕ್ಷ TK_DOT:
    ಪಕ್ಷ TK_RAISE:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_REGISTER );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ROW );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_DOT );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_RAISE );
      pWalker->eCode = 0;
      ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    ಪಕ್ಷ TK_IS:
    ಪಕ್ಷ TK_ISNOT:
      ಯದಿ( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
      ಪ್ರತ್ಯಾಗಮ WRC_Continue;
    ಪಕ್ಷ TK_VARIABLE:
      ಯದಿ( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
      }ಅನ್ಯಥಾ ಯದಿ( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    ಸ್ವಭಾವ:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Is_Const(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಪ್ರತ್ಯಾಗಮ w.eCode;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Is_Constant(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ_List *pList, ಪೂರ್ಣಾಙ್ಕ bNoIs){
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ iInit = bNoIs ? 6 : 1;
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪ್ರತಿ(ii=0; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
    ಯದಿ( 0==ಅಭಿವ್ಯಕ್ತಿ_Is_Const(pParse, pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ, iInit) ) ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತ್ಯಾಗಮ 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  ಪ್ರತಿಜ್ಞಾ( pWalker->eCode==3 || pWalker->eCode==0 );
  ಯದಿ( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Prune;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Table_Constant(ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ iCur, ಪೂರ್ಣಾಙ್ಕ bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant;
  ಯದಿ( bAllowSubq ){
    w.xSelectCallback = ಅಭಿವ್ಯಕ್ತಿ_Select_Walk_Table_Constant;
  }ಅನ್ಯಥಾ{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಪ್ರತ್ಯಾಗಮ w.eCode;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Single_Table_Constraint(
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,                 /* The constraint */
  ಧ್ರುವ SrcList *pSrcList,     /* Complete FROM clause */
  ಪೂರ್ಣಾಙ್ಕ iSrc,                    /* Which element of pSrcList to use */
  ಪೂರ್ಣಾಙ್ಕ bAllowSubq               /* Allow non-correlated subqueries */
){
  ಧ್ರುವ SrcItem *pSrc = &pSrcList->a[iSrc];
  ಯದಿ( pSrc->fg.jointype & JT_LTORJ ){
    ಪ್ರತ್ಯಾಗಮ 0;  /* rule (3) */
  }
  ಯದಿ( pSrc->fg.jointype & JT_ಎಡ ){
    ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ) ಪ್ರತ್ಯಾಗಮ 0;   /* rule (4a) */
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin!=pSrc->iCursor ) ಪ್ರತ್ಯಾಗಮ 0;         /* rule (4b) */
  }ಅನ್ಯಥಾ{
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ) ಪ್ರತ್ಯಾಗಮ 0;    /* rule (5) */
  }
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    ಪೂರ್ಣಾಙ್ಕ jj;
    ಪ್ರತಿ(jj=0; jj<iSrc; jj++){
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin==pSrcList->a[jj].iCursor ){
        ಯದಿ( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          ಪ್ರತ್ಯಾಗಮ 0;  /* restriction (6) */
        }
        ಭಙ್ಗ;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Table_Constant(p_ಅಭಿವ್ಯಕ್ತಿ, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant_Or_Group_By(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಭಿವ್ಯಕ್ತಿ_List *pGroupBy = pWalker->u.pGroupBy;
  ಪೂರ್ಣಾಙ್ಕ i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  ಪ್ರತಿ(i=0; i<pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = pGroupBy->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ, p, -1)<2 ){
      CollSeq *pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(pWalker->pParse, p);
      ಯದಿ( sqlite3_Is_ದ್ವಿಮಾನ(pColl) ){
        ಪ್ರತ್ಯಾಗಮ WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    pWalker->eCode = 0;
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }

  ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_Group_By(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಅಭಿವ್ಯಕ್ತಿ_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಪ್ರತ್ಯಾಗಮ w.eCode;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_Function(ಅಭಿವ್ಯಕ್ತಿ *p, u8 isInit){
  ಪ್ರತಿಜ್ಞಾ( isInit==0 || isInit==1 );
  ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Contains_Subquery(ಅಭಿವ್ಯಕ್ತಿ *p){
  Walker w;
  w.eCode = 1;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಪ್ರತ್ಯಾಗಮ w.eCode==0;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ *p_ಮೌಲ್ಯ, Parse *pParse, ಪೂರ್ಣಾಙ್ಕ bRSI){
  ಪೂರ್ಣಾಙ್ಕ iSign = 1;   /* Either +1 or -1. */
  ಯಾವತ್( 1/*exit-by-break*/ ){
    ಯದಿ( NEVER(p==0) ) ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_UValue(p) ){
      ಪ್ರತಿಜ್ಞಾ( p->op==TK_INTEGER );
      *p_ಮೌಲ್ಯ = p->u.i_ಮೌಲ್ಯ*iSign;
      ಯದಿ( bRSI && pParse==0 && p->op2 ){
        ಪ್ರತ್ಯಾಗಮ 0;
      }
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಯದಿ( p->op==TK_UPLUS ){
      p = p->p_ಎಡ;
      pParse = 0;
      ಅನುವರ್ತ;
    }
    ಯದಿ( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_ಎಡ;
      pParse = 0;
      ಅನುವರ್ತ;
    }
    ಯದಿ( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_ಮೌಲ್ಯ *pVal; /* The variable */
      ಪೂರ್ಣಾಙ್ಕ rc = 0;          /* 1 if successful, 0 if failed */
      ಪ್ರತಿಜ್ಞಾ( iSign==1 );
      ಯದಿ( NEVER(pParse->pVdbe==0) ) ಭಙ್ಗ;
      ಯದಿ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ಭಙ್ಗ;
      pVal = sqlite3_Vdbe_Get_Bound_ಮೌಲ್ಯ(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      ಯದಿ( pVal ){
        i64 vv;
        ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_ಮೌಲ್ಯ_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_ಮೌಲ್ಯ = (ಪೂರ್ಣಾಙ್ಕ)vv;
          rc = 1;       
        }
        sqlite3_ಮೌಲ್ಯ_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      ಪ್ರತ್ಯಾಗಮ rc;
    }
    ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p){
  u8 op;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಯಾವತ್( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_ಎಡ;
    ಪ್ರತಿಜ್ಞಾ( p!=0 );
  }
  op = p->op;
  ಯದಿ( op==TK_REGISTER ) op = p->op2;
  ವಿಕಲ್ಪ( op ){
    ಪಕ್ಷ TK_INTEGER:
    ಪಕ್ಷ TK_STRING:
    ಪಕ್ಷ TK_FLOAT:
    ಪಕ್ಷ TK_BLOB:
      ಪ್ರತ್ಯಾಗಮ 0;
    ಪಕ್ಷ TK_COLUMN:
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p) );
      ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ 1;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Needs_No_Affinity_Change(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p, ವರ್ಣ aff){
  u8 op;
  ಪೂರ್ಣಾಙ್ಕ unaryMinus = 0;
  ಯದಿ( aff==SQLITE_AFF_BLOB ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯಾವತ್( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    ಯದಿ( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_ಎಡ;
  }
  op = p->op;
  ಯದಿ( op==TK_REGISTER ) op = p->op2;
  ವಿಕಲ್ಪ( op ){
    ಪಕ್ಷ TK_INTEGER: {
      ಪ್ರತ್ಯಾಗಮ aff>=SQLITE_AFF_NUMERIC;
    }
    ಪಕ್ಷ TK_FLOAT: {
      ಪ್ರತ್ಯಾಗಮ aff>=SQLITE_AFF_NUMERIC;
    }
    ಪಕ್ಷ TK_STRING: {
      ಪ್ರತ್ಯಾಗಮ !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    ಪಕ್ಷ TK_BLOB: {
      ಪ್ರತ್ಯಾಗಮ !unaryMinus;
    }
    ಪಕ್ಷ TK_COLUMN: {
      ಪ್ರತಿಜ್ಞಾ( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      ಪ್ರತ್ಯಾಗಮ aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    ಸ್ವಭಾವ: {
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3IsRowid(ಧ್ರುವ ವರ್ಣ *z){
  ಯದಿ( sqlite3StrICmp(z, "_ROWID_")==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯದಿ( sqlite3StrICmp(z, "ROWID")==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯದಿ( sqlite3StrICmp(z, "OID")==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3RowidAlias(Table *pTab){
  ಧ್ರುವ ವರ್ಣ *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪ್ರತಿಜ್ಞಾ( VisibleRowid(pTab) );
  ಪ್ರತಿ(ii=0; ii<ArraySize(azOpt); ii++){
    ಯದಿ( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) ಪ್ರತ್ಯಾಗಮ azOpt[ii];
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
ಸ್ಥಿರ Select *isCandidateForInOpt(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pX){
  Select *p;
  SrcList *pSrc;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  Table *pTab;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pX) ) ಪ್ರತ್ಯಾಗಮ 0;                 /* Not a subquery */
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pX, EP_VarSelect)  ) ಪ್ರತ್ಯಾಗಮ 0;  /* Correlated subq */
  p = pX->x.pSelect;
  ಯದಿ( p->pPrior ) ಪ್ರತ್ಯಾಗಮ 0;              /* Not a compound SELECT */
  ಯದಿ( p->selFlags & SF_Aggregate ){
    ಪ್ರತ್ಯಾಗಮ 0; /* No GROUP BY keyword or aggregate functions */
  }
  ಪ್ರತಿಜ್ಞಾ( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  ಯದಿ( p->pLimit ) ಪ್ರತ್ಯಾಗಮ 0;              /* Has no LIMIT clause */
  ಯದಿ( p->pWhere ) ಪ್ರತ್ಯಾಗಮ 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSrc!=0 );
  ಯದಿ( pSrc->nSrc!=1 ) ಪ್ರತ್ಯಾಗಮ 0;          /* Single term in FROM clause */
  ಯದಿ( pSrc->a[0].fg.isSubquery) ಪ್ರತ್ಯಾಗಮ 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( !IsView(pTab)  );              /* FROM clause is not a view */
  ಯದಿ( IsVirtual(pTab) ) ಪ್ರತ್ಯಾಗಮ 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
  /* All SELECT results must be columns. */
  ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *pRes = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಯದಿ( pRes->op!=TK_COLUMN ) ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತಿಜ್ಞಾ( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  ಪ್ರತ್ಯಾಗಮ p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  ಪೂರ್ಣಾಙ್ಕ iCur,          /* Cursor for the index */
  ಪೂರ್ಣಾಙ್ಕ regHasNull,    /* Register in which to store hasNull flag */
  ಪೂರ್ಣಾಙ್ಕ eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  ಪೂರ್ಣಾಙ್ಕ addr1;
  ಪೂರ್ಣಾಙ್ಕ op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  ಯದಿ( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3InRhsIsConstant(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *pIn){
  ಅಭಿವ್ಯಕ್ತಿ *pLHS;
  ಪೂರ್ಣಾಙ್ಕ res;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_ಎಡ;
  pIn->p_ಎಡ = 0;
  res = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pParse, pIn);
  pIn->p_ಎಡ = pLHS;
  ಪ್ರತ್ಯಾಗಮ res;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  ಪೂರ್ಣಾಙ್ಕ *prRhsHasNull,         /* Register holding NULL status.  See notes */
  ಪೂರ್ಣಾಙ್ಕ *aiMap,                /* Mapping from Index fields to RHS fields */
  ಪೂರ್ಣಾಙ್ಕ *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  ಪೂರ್ಣಾಙ್ಕ eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  ಪೂರ್ಣಾಙ್ಕ iTab;                             /* Cursor of the RHS table */
  ಪೂರ್ಣಾಙ್ಕ mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  ಪ್ರತಿಜ್ಞಾ( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  ಯದಿ( prRhsHasNull && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pX) ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList = pX->x.pSelect->pEList;
    ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ) ) ಭಙ್ಗ;
    }
    ಯದಿ( i==pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  ಯದಿ( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    ಪೂರ್ಣಾಙ್ಕ iDb;                               /* Database idx for pTab */
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList = p->pEList;
    ಪೂರ್ಣಾಙ್ಕ n_ಅಭಿವ್ಯಕ್ತಿ = pEList->n_ಅಭಿವ್ಯಕ್ತಿ;

    ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    ಪ್ರತಿಜ್ಞಾ( p->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ!=0 ); /* Because of isCandidateForInOpt(p) */
    ಪ್ರತಿಜ್ಞಾ( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    ಪ್ರತಿಜ್ಞಾ(v);  /* sqlite3GetVdbe() has always been previously called */
    ಯದಿ( n_ಅಭಿವ್ಯಕ್ತಿ==1 && pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      ಪೂರ್ಣಾಙ್ಕ iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }ಅನ್ಯಥಾ{
      Index *pIdx;                         /* Iterator variable */
      ಪೂರ್ಣಾಙ್ಕ affinity_ok = 1;
      ಪೂರ್ಣಾಙ್ಕ i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      ಪ್ರತಿ(i=0; i<n_ಅಭಿವ್ಯಕ್ತಿ && affinity_ok; i++){
        ಅಭಿವ್ಯಕ್ತಿ *pLhs = sqlite3VectorFieldSubexpr(pX->p_ಎಡ, i);
        ಪೂರ್ಣಾಙ್ಕ iCol = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
        ವರ್ಣ idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        ವರ್ಣ cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        ವಿಕಲ್ಪ( cmpaff ){
          ಪಕ್ಷ SQLITE_AFF_BLOB:
            ಭಙ್ಗ;
          ಪಕ್ಷ SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            ಪ್ರತಿಜ್ಞಾ( idxaff==SQLITE_AFF_TEXT );
            ಭಙ್ಗ;
          ಸ್ವಭಾವ:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      ಯದಿ( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          ಯದಿ( pIdx->nColumn<n_ಅಭಿವ್ಯಕ್ತಿ ) ಅನುವರ್ತ;
          ಯದಿ( pIdx->pPartIdxWhere!=0 ) ಅನುವರ್ತ;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          ಯದಿ( pIdx->nColumn>=BMS-1 ) ಅನುವರ್ತ;
          ಯದಿ( mustBeUnique ){
            ಯದಿ( pIdx->n_ಕೀಲಿ_Col>n_ಅಭಿವ್ಯಕ್ತಿ
             ||(pIdx->nColumn>n_ಅಭಿವ್ಯಕ್ತಿ && !IsUniqueIndex(pIdx))
            ){
              ಅನುವರ್ತ;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          ಪ್ರತಿ(i=0; i<n_ಅಭಿವ್ಯಕ್ತಿ; i++){
            ಅಭಿವ್ಯಕ್ತಿ *pLhs = sqlite3VectorFieldSubexpr(pX->p_ಎಡ, i);
            ಅಭಿವ್ಯಕ್ತಿ *pRhs = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
            CollSeq *pReq = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, pLhs, pRhs);
            ಪೂರ್ಣಾಙ್ಕ j;
 
            ಪ್ರತಿ(j=0; j<n_ಅಭಿವ್ಯಕ್ತಿ; j++){
              ಯದಿ( pIdx->aiColumn[j]!=pRhs->iColumn ) ಅನುವರ್ತ;
              ಪ್ರತಿಜ್ಞಾ( pIdx->azColl[j] );
              ಯದಿ( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                ಅನುವರ್ತ;
              }
              ಭಙ್ಗ;
            }
            ಯದಿ( j==n_ಅಭಿವ್ಯಕ್ತಿ ) ಭಙ್ಗ;
            mCol = MASKBIT(j);
            ಯದಿ( mCol & colUsed ) ಭಙ್ಗ; /* Each column used only once */
            colUsed |= mCol;
            ಯದಿ( aiMap ) aiMap[i] = j;
          }
 
          ಪ್ರತಿಜ್ಞಾ( n_ಅಭಿವ್ಯಕ್ತಿ>0 && n_ಅಭಿವ್ಯಕ್ತಿ<BMS );
          ಪ್ರತಿಜ್ಞಾ( i==n_ಅಭಿವ್ಯಕ್ತಿ || colUsed!=(MASKBIT(n_ಅಭಿವ್ಯಕ್ತಿ)-1) );
          ಯದಿ( colUsed==(MASKBIT(n_ಅಭಿವ್ಯಕ್ತಿ)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            ಪೂರ್ಣಾಙ್ಕ iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_ಕೀಲಿ_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            ಪ್ರತಿಜ್ಞಾ( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            ಯದಿ( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_ಅಭಿವ್ಯಕ್ತಿ)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              ಯದಿ( n_ಅಭಿವ್ಯಕ್ತಿ==1 ){
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
  ಯದಿ( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  ಯದಿ( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    ಪೂರ್ಣಾಙ್ಕ rMayHaveNull = 0;
    ಪೂರ್ಣಾಙ್ಕ bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    ಯದಿ( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }ಅನ್ಯಥಾ ಯದಿ( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    ಪ್ರತಿಜ್ಞಾ( pX->op==TK_IN );
    ಯದಿ( !bloomOk
     && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    ಯದಿ( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  ಯದಿ( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    ಪೂರ್ಣಾಙ್ಕ i, n;
    n = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(pX->p_ಎಡ);
    ಪ್ರತಿ(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  ಪ್ರತ್ಯಾಗಮ eType;
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
ಸ್ಥಿರ ವರ್ಣ *ಅಭಿವ್ಯಕ್ತಿ_INAffinity(Parse *pParse, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಪೂರ್ಣಾಙ್ಕ nVal = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಎಡ);
  Select *pSelect = ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ? p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect : 0;
  ವರ್ಣ *zRet;

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  ಯದಿ( zRet ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<nVal; i++){
      ಅಭಿವ್ಯಕ್ತಿ *pA = sqlite3VectorFieldSubexpr(p_ಎಡ, i);
      ವರ್ಣ a = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pA);
      ಯದಿ( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, a);
      }ಅನ್ಯಥಾ{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  ಪ್ರತ್ಯಾಗಮ zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
ಶೂನ್ಯ sqlite3SubselectError(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ nActual, ಪೂರ್ಣಾಙ್ಕ nExpect){
  ಯದಿ( pParse->nErr==0 ){
    ಧ್ರುವ ವರ್ಣ *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_ಸಂದೇಶ(pParse, zFmt, nActual, nExpect);
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
ಶೂನ್ಯ sqlite3_Vector_Error_ಸಂದೇಶ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
#ifndef SQLITE_OMIT_SUBQUERY
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    sqlite3SubselectError(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ, 1);
  }ಅನ್ಯಥಾ
#endif
  {
    sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  ಯದಿ( pNewSig==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IN );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect!=0 );
  ಪ್ರತಿಜ್ಞಾ( (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  ಪ್ರತಿ(; pOp<pEnd; pOp++){
    ಯದಿ( pOp->p4type!=P4_SUBRTNSIG ) ಅನುವರ್ತ;
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    ಪ್ರತಿಜ್ಞಾ( pSig!=0 );
    ಯದಿ( !pSig->bComplete ) ಅನುವರ್ತ;
    ಯದಿ( pNewSig->selId!=pSig->selId ) ಅನುವರ್ತ;
    ಯದಿ( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) ಅನುವರ್ತ;
    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr = pSig->iAddr;
    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn = pSig->regReturn;
    p_ಅಭಿವ್ಯಕ್ತಿ->iTable = pSig->iTable;
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಶೂನ್ಯ sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,            /* The IN operator */
  ಪೂರ್ಣಾಙ್ಕ iTab,               /* Use this cursor number */
  ಪೂರ್ಣಾಙ್ಕ allowBloom          /* True to allow the use of a Bloom filter */
){
  ಪೂರ್ಣಾಙ್ಕ addrOnce = 0;           /* Address of the OP_Once instruction at top */
  ಪೂರ್ಣಾಙ್ಕ addr;                   /* Address of OP_OpenEphemeral instruction */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;                /* the LHS of the IN operator */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = 0;      /* Key information */
  ಪೂರ್ಣಾಙ್ಕ nVal;                   /* Size of vector pLeft */
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
  ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) || p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect!=0 );
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) && (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, ಪರಿಮಾಣ(pSig[0]));
      ಯದಿ( pSig ){
        pSig->selId = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selId;
        pSig->zAff = ಅಭಿವ್ಯಕ್ತಿ_INAffinity(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selId));
      }
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn,
                        p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr);
      ಪ್ರತಿಜ್ಞಾ( iTab!=p_ಅಭಿವ್ಯಕ್ತಿ->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_ಅಭಿವ್ಯಕ್ತಿ->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      ಯದಿ( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      ಪ್ರತ್ಯಾಗಮ;
    }

    /* Begin coding the subroutine */
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn);
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced) );
    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn = ++pParse->nMem;
    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn) + 1;
    ಯದಿ( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr;
      pSig->regReturn = p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ಧ್ರುವ ವರ್ಣ*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  nVal = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಎಡ);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_ಅಭಿವ್ಯಕ್ತಿ->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_ಅಭಿವ್ಯಕ್ತಿ->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    VdbeComment((v, "Result of SELECT %u", p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selId));
  }ಅನ್ಯಥಾ{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_Alloc(pParse->db, nVal, 1);
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info!=0 || pParse->nErr );
  ಯದಿ( p_ಕೀಲಿ_Info==0 ) ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
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
    ಯದಿ( ALWAYS(pEList->n_ಅಭಿವ್ಯಕ್ತಿ==nVal) ){
      Select *pCopy;
      SelectDest dest;
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪೂರ್ಣಾಙ್ಕ rc;
      ಪೂರ್ಣಾಙ್ಕ addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = ಅಭಿವ್ಯಕ್ತಿ_INAffinity(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
      pSelect->iLimit = 0;
      ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
      ಪ್ರತಿಜ್ಞಾ( pEList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(p_ಕೀಲಿ_Info) );
      ಪ್ರತಿ(i=0; i<nVal; i++){
        ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3VectorFieldSubexpr(p_ಎಡ, i);
        CollSeq *pColl;
        p_ಕೀಲಿ_Info->aColl[i] = pColl = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ
        );
        ಯದಿ( !sqlite3_Is_ದ್ವಿಮಾನ(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      ಯದಿ( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        ಪೂರ್ಣಾಙ್ಕ regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
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
      ಯದಿ( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        ಯದಿ( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      ಯದಿ( rc ){
        sqlite3_ಕೀಲಿ_Info_Unref(p_ಕೀಲಿ_Info);
        ಪ್ರತ್ಯಾಗಮ;
      }
    }
  }ಅನ್ಯಥಾ ಯದಿ( ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    ವರ್ಣ affinity;            /* Affinity of the LHS of the IN */
    ಪೂರ್ಣಾಙ್ಕ i;
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
    ಪೂರ್ಣಾಙ್ಕ r1, r2;
    affinity = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಎಡ);
    ಯದಿ( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }ಅನ್ಯಥಾ ಯದಿ( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info!=0 );
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(p_ಕೀಲಿ_Info) );
    p_ಕೀಲಿ_Info->aColl[0] = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    ಪ್ರತಿ(i=pList->n_ಅಭಿವ್ಯಕ್ತಿ, pItem=pList->a; i>0; i--, pItem++){
      ಅಭಿವ್ಯಕ್ತಿ *pE2 = pItem->p_ಅಭಿವ್ಯಕ್ತಿ;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      ಯದಿ( addrOnce && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  ಯದಿ( pSig ) pSig->bComplete = 1;
  ಯದಿ( p_ಕೀಲಿ_Info ){
    sqlite3VdbeChangeP4(v, addr, (ಶೂನ್ಯ *)p_ಕೀಲಿ_Info, P4_KEYINFO);
  }
  ಯದಿ( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeGetOp(v,p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn,
                      p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr, 1);
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
ಪೂರ್ಣಾಙ್ಕ sqlite3CodeSubselect(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  ಪೂರ್ಣಾಙ್ಕ rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  ಪೂರ್ಣಾಙ್ಕ nReg;                   /* Registers to allocate */
  ಅಭಿವ್ಯಕ್ತಿ *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  ಪೂರ್ಣಾಙ್ಕ addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ 0;
  testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS );
  testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pSel = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn,
                      p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr);
    ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ->iTable;
  }

  /* Begin coding the subroutine */
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced|EP_TokenOnly) );
  ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn);
  p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn = ++pParse->nMem;
  p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn) + 1;

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
  ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect) ){
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
  nReg = p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ? pSel->pEList->n_ಅಭಿವ್ಯಕ್ತಿ : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    ಯದಿ( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_ಬಲ ){
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
    }ಅನ್ಯಥಾ{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }ಅನ್ಯಥಾ{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  ಯದಿ( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = pSel->pLimit->p_ಎಡ;
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಎಡ, EP_Int_ಮೌಲ್ಯ)==0
     || (p_ಎಡ->u.i_ಮೌಲ್ಯ!=1 && p_ಎಡ->u.i_ಮೌಲ್ಯ!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, 0);
      ಯದಿ( pLimit ){
        pLimit->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0), pLimit);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಎಡ);
      pSel->pLimit->p_ಎಡ = pLimit;
    }
  }ಅನ್ಯಥಾ{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  ಯದಿ( sqlite3Select(pParse, pSel, &dest) ){
    p_ಅಭಿವ್ಯಕ್ತಿ->op2 = p_ಅಭಿವ್ಯಕ್ತಿ->op;
    p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_ERROR;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  p_ಅಭಿವ್ಯಕ್ತಿ->iTable = rReg = dest.iSDParm;
  ಅಭಿವ್ಯಕ್ತಿ_Set_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_NoReduce);
  ಯದಿ( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  ಯದಿ( dest.eDest==SRT_Mem 
   && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeGetOp(v,p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn,
                    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  ಪ್ರತ್ಯಾಗಮ rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_IN(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *pIn){
  ಪೂರ್ಣಾಙ್ಕ nVector = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(pIn->p_ಎಡ);
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    ಯದಿ( nVector!=pIn->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ, nVector);
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }ಅನ್ಯಥಾ ಯದಿ( nVector!=1 ){
    sqlite3_Vector_Error_ಸಂದೇಶ(pParse, pIn->p_ಎಡ);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* The IN expression */
  ಪೂರ್ಣಾಙ್ಕ destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  ಪೂರ್ಣಾಙ್ಕ destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  ಪೂರ್ಣಾಙ್ಕ rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  ಪೂರ್ಣಾಙ್ಕ eType;            /* Type of the RHS */
  ಪೂರ್ಣಾಙ್ಕ rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  ಪೂರ್ಣಾಙ್ಕ *aiMap = 0;       /* Map from vector field to index column */
  ವರ್ಣ *zAff = 0;       /* Affinity string for comparisons */
  ಪೂರ್ಣಾಙ್ಕ nVector;          /* Size of vectors for this IN operator */
  ಪೂರ್ಣಾಙ್ಕ iDummy;           /* Dummy parameter to exprCodeVector() */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;          /* The LHS of the IN operator */
  ಪೂರ್ಣಾಙ್ಕ i;                /* loop counter */
  ಪೂರ್ಣಾಙ್ಕ destStep2;        /* Where to jump when NULLs seen in step 2 */
  ಪೂರ್ಣಾಙ್ಕ destStep6 = 0;    /* Start of code for Step 6 */
  ಪೂರ್ಣಾಙ್ಕ addrTruthOp;      /* Address of opcode that determines the IN is true */
  ಪೂರ್ಣಾಙ್ಕ destNotNull;      /* Jump here if a comparison is not true in step 6 */
  ಪೂರ್ಣಾಙ್ಕ addr_ತುದಿ;          /* Top of the step-6 loop */
  ಪೂರ್ಣಾಙ್ಕ iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );
  p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_IN(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ) ಪ್ರತ್ಯಾಗಮ;
  zAff = ಅಭಿವ್ಯಕ್ತಿ_INAffinity(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  nVector = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  aiMap = (ಪೂರ್ಣಾಙ್ಕ*)sqlite3DbMallocZero(pParse->db, nVector*ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
  ಯದಿ( pParse->db->mallocFailed ) ಗಚ್ಛ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  ಪ್ರತಿಜ್ಞಾ( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  ಪ್ರತಿ(i=0; i<nVector; i++){
    ಪೂರ್ಣಾಙ್ಕ j, cnt;
    ಪ್ರತಿ(cnt=j=0; j<nVector; j++) ಯದಿ( aiMap[j]==i ) cnt++;
    ಪ್ರತಿಜ್ಞಾ( cnt==1 );
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
  ಪ್ರತಿಜ್ಞಾ( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = ಅಭಿವ್ಯಕ್ತಿ_Code_Vector(pParse, p_ಎಡ, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  ಯದಿ( eType==IN_INDEX_NOOP ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    CollSeq *pColl;
    ಪೂರ್ಣಾಙ್ಕ labelOk = sqlite3VdbeMakeLabel(pParse);
    ಪೂರ್ಣಾಙ್ಕ r2, regToFree;
    ಪೂರ್ಣಾಙ್ಕ regCkNull = 0;
    ಪೂರ್ಣಾಙ್ಕ ii;
    ಪ್ರತಿಜ್ಞಾ( nVector==1 );
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಯದಿ( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    ಪ್ರತಿ(ii=0; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
      r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ, &regToFree);
      ಯದಿ( regCkNull && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      ಯದಿ( ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 || destIfNull!=destIfFalse ){
        ಪೂರ್ಣಾಙ್ಕ op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (ಶೂನ್ಯ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        ಪ್ರತಿಜ್ಞಾ( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (ಶೂನ್ಯ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    ಯದಿ( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    ಗಚ್ಛ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN_finished;
  }

  ಯದಿ( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    ಯದಿ( nVector==1 ){
      ವರ್ಣ aff = zAff[0];
      ಯದಿ( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಎಡ) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        ಪೂರ್ಣಾಙ್ಕ rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }ಅನ್ಯಥಾ{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    ಪ್ರತಿ(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    ಯದಿ( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      ಪೂರ್ಣಾಙ್ಕ rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      ಪ್ರತಿ(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  ಯದಿ( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }ಅನ್ಯಥಾ{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  ಪ್ರತಿ(i=0; i<nVector; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3VectorFieldSubexpr(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, i);
    ಯದಿ( pParse->nErr ) ಗಚ್ಛ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN_oom_error;
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  ಯದಿ( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    ಪ್ರತಿಜ್ಞಾ( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }ಅನ್ಯಥಾ{
    ಯದಿ( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn) ){
        ಧ್ರುವ VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr);
        ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_Once || pParse->nErr );
        ಯದಿ( pOp->p3>0 ){  /* tag-202407032019 */
          ಪ್ರತಿಜ್ಞಾ( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      ಗಚ್ಛ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  ಯದಿ( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  ಯದಿ( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  ಯದಿ( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  ಯದಿ( eType==IN_INDEX_INDEX_DESC ){
    addr_ತುದಿ = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }ಅನ್ಯಥಾ{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_ತುದಿ = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  ಯದಿ( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }ಅನ್ಯಥಾ{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  ಪ್ರತಿ(i=0; i<nVector; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p;
    CollSeq *pColl;
    ಪೂರ್ಣಾಙ್ಕ r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_ಎಡ, i);
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
      ಅಭಿವ್ಯಕ್ತಿ *pRhs = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
      pColl = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p, pRhs);
    }ಅನ್ಯಥಾ{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (ಶೂನ್ಯ*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  ಯದಿ( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_ತುದಿ+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN_oom_error:
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
ಸ್ಥಿರ ಶೂನ್ಯ codeReal(Vdbe *v, ಧ್ರುವ ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ negateFlag, ಪೂರ್ಣಾಙ್ಕ iMem){
  ಯದಿ( ALWAYS(z!=0) ){
    ದ್ವಿಗುಣ ಮೌಲ್ಯ;
    sqlite3AtoF(z, &ಮೌಲ್ಯ);
    ಪ್ರತಿಜ್ಞಾ( !sqlite3IsNaN(ಮೌಲ್ಯ) ); /* The new AtoF never returns NaN */
    ಯದಿ( negateFlag ) ಮೌಲ್ಯ = -ಮೌಲ್ಯ;
    sqlite3VdbeAddDouble(v, iMem, ಮೌಲ್ಯ);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codeInteger(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ negFlag, ಪೂರ್ಣಾಙ್ಕ iMem){
  Vdbe *v = pParse->pVdbe;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_Int_ಮೌಲ್ಯ ){
    ಪೂರ್ಣಾಙ್ಕ i = p_ಅಭಿವ್ಯಕ್ತಿ->u.i_ಮೌಲ್ಯ;
    ಪ್ರತಿಜ್ಞಾ( i>=0 );
    ಯದಿ( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ c;
    i64 ಮೌಲ್ಯ;
    ಧ್ರುವ ವರ್ಣ *z = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
    ಪ್ರತಿಜ್ಞಾ( z!=0 );
    c = sqlite3DecOrHexToI64(z, &ಮೌಲ್ಯ);
    ಯದಿ( (c==3 && !negFlag) || (c==2) || (negFlag && ಮೌಲ್ಯ==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_ಸಂದೇಶ(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_ಅಭಿವ್ಯಕ್ತಿ);
#ಅನ್ಯಥಾ
#ifndef SQLITE_OMIT_HEX_INTEGER
      ಯದಿ( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_ಅಭಿವ್ಯಕ್ತಿ);
      }ಅನ್ಯಥಾ
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }ಅನ್ಯಥಾ{
      ಯದಿ( negFlag ){ ಮೌಲ್ಯ = c==3 ? SMALLEST_INT64 : -ಮೌಲ್ಯ; }
      sqlite3VdbeAddInt64(v, iMem, ಮೌಲ್ಯ);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  ಪೂರ್ಣಾಙ್ಕ iTabCur,    /* Cursor pointing to a table row */
  ಪೂರ್ಣಾಙ್ಕ iIdxCol,    /* The column of the index to be loaded */
  ಪೂರ್ಣಾಙ್ಕ regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  ಯದಿ( iTabCol==XN_ಅಭಿವ್ಯಕ್ತಿ ){
    ಪ್ರತಿಜ್ಞಾ( pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ );
    ಪ್ರತಿಜ್ಞಾ( pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ->n_ಅಭಿವ್ಯಕ್ತಿ>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Copy(pParse, pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ->a[iIdxCol].p_ಅಭಿವ್ಯಕ್ತಿ, regOut);
    pParse->iSelfTab = 0;
  }ಅನ್ಯಥಾ{
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  ಪೂರ್ಣಾಙ್ಕ regOut         /* Put the result in this register */
){
  ಪೂರ್ಣಾಙ್ಕ iAddr;
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ nErr = pParse->nErr;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->iSelfTab!=0 );
  ಯದಿ( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }ಅನ್ಯಥಾ{
    iAddr = 0;
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Copy(pParse, sqlite3_Column_ಅಭಿವ್ಯಕ್ತಿ(pTab,pCol), regOut);
  ಯದಿ( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    ಪೂರ್ಣಾಙ್ಕ p3 = 2+(ಪೂರ್ಣಾಙ್ಕ)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (ವರ್ಣ*)pTab, P4_TABLE);
  }ಅನ್ಯಥಾ ಯದಿ( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  ಯದಿ( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  ಯದಿ( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  ಪೂರ್ಣಾಙ್ಕ iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  ಪೂರ್ಣಾಙ್ಕ iCol,       /* Index of the column to extract */
  ಪೂರ್ಣಾಙ್ಕ regOut      /* Extract the value into this register */
){
  Column *pCol;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( iCol!=XN_ಅಭಿವ್ಯಕ್ತಿ );
  ಯದಿ( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ x;
    ಯದಿ( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      ಪ್ರತ್ಯಾಗಮ;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }ಅನ್ಯಥಾ ಯದಿ( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      ಯದಿ( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      ಪ್ರತ್ಯಾಗಮ;
#endif
    }ಅನ್ಯಥಾ ಯದಿ( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_ಕೀಲಿ_Index(pTab), iCol);
    }ಅನ್ಯಥಾ{
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  ಪೂರ್ಣಾಙ್ಕ iColumn,     /* Index of the table column */
  ಪೂರ್ಣಾಙ್ಕ iTable,      /* The cursor pointing to the table */
  ಪೂರ್ಣಾಙ್ಕ iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe!=0 );
  ಪ್ರತಿಜ್ಞಾ( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  ಪ್ರತಿಜ್ಞಾ( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  ಯದಿ( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    ಯದಿ( pOp->opcode==OP_Column ) pOp->p5 = p5;
    ಯದಿ( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  ಪ್ರತ್ಯಾಗಮ iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Move(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iFrom, ಪೂರ್ಣಾಙ್ಕ iTo, ಪೂರ್ಣಾಙ್ಕ nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_To_Register(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ iReg){
  ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಯದಿ( NEVER(p==0) ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( p->op==TK_REGISTER ){
    ಪ್ರತಿಜ್ಞಾ( p->iTable==iReg );
  }ಅನ್ಯಥಾ{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Code_Vector(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ *piFreeable){
  ಪೂರ್ಣಾಙ್ಕ iResult;
  ಪೂರ್ಣಾಙ್ಕ nResult = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p);
  ಯದಿ( nResult==1 ){
    iResult = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p, piFreeable);
  }ಅನ್ಯಥಾ{
    *piFreeable = 0;
    ಯದಿ( p->op==TK_SELECT ){
#ಯದಿ SQLITE_OMIT_SUBQUERY
      iResult = 0;
#ಅನ್ಯಥಾ
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      ಪ್ರತಿ(i=0; i<nResult; i++){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Factorable(pParse, p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, i+iResult);
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setDoNotMergeFlagOnCopy(Vdbe *v){
  ಯದಿ( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pFarg,      /* List of function arguments */
  ಪೂರ್ಣಾಙ್ಕ iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  ಪೂರ್ಣಾಙ್ಕ target            /* Store function result in this register */
){
  ಪೂರ್ಣಾಙ್ಕ nFarg;
  Vdbe *v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pFarg!=0 );
  nFarg = pFarg->n_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( nFarg>0 );  /* All in-line functions have at least one argument */
  ವಿಕಲ್ಪ( iFuncId ){
    ಪಕ್ಷ INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      ಪೂರ್ಣಾಙ್ಕ endCoalesce = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪ್ರತಿಜ್ಞಾ( nFarg>=2 );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, target);
      ಪ್ರತಿ(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pFarg->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ INLINEFUNC_iif: {
      ಅಭಿವ್ಯಕ್ತಿ case_ಅಭಿವ್ಯಕ್ತಿ;
      memset(&case_ಅಭಿವ್ಯಕ್ತಿ, 0, ಪರಿಮಾಣ(case_ಅಭಿವ್ಯಕ್ತಿ));
      case_ಅಭಿವ್ಯಕ್ತಿ.op = TK_CASE;
      case_ಅಭಿವ್ಯಕ್ತಿ.x.pList = pFarg;
      ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, &case_ಅಭಿವ್ಯಕ್ತಿ, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    ಪಕ್ಷ INLINEFUNC_sqlite_offset: {
      ಅಭಿವ್ಯಕ್ತಿ *pArg = pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }ಅನ್ಯಥಾ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      ಭಙ್ಗ;
    }
#endif
    ಸ್ವಭಾವ: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      ಪ್ರತಿಜ್ಞಾ( nFarg==1 || nFarg==2 );
      target = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, target);
      ಭಙ್ಗ;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#ಯದಿ !defined(SQLITE_UNTESTABLE)
    ಪಕ್ಷ INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      ಪ್ರತಿಜ್ಞಾ( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, pFarg->a[1].p_ಅಭಿವ್ಯಕ್ತಿ,-1),
         target);
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_implies_ಅಭಿವ್ಯಕ್ತಿ: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      ಪ್ರತಿಜ್ಞಾ( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_ಅಭಿವ್ಯಕ್ತಿ(pParse,pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, pFarg->a[1].p_ಅಭಿವ್ಯಕ್ತಿ,-1),
         target);
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      ಅಭಿವ್ಯಕ್ತಿ *pA1;
      ಪ್ರತಿಜ್ಞಾ( nFarg==2 );
      pA1 = pFarg->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_Non_Null_Row(pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,pA1->iTable,1),
           target);
      }ಅನ್ಯಥಾ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      ಧ್ರುವ ವರ್ಣ *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      ವರ್ಣ aff;
      ಪ್ರತಿಜ್ಞಾ( nFarg==1 );
      aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
      ಪ್ರತಿಜ್ಞಾ( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      ಭಙ್ಗ;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  ಪ್ರತ್ಯಾಗಮ target;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Can_Return_Subtype(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ n;
  FuncDef *pDef;
  sqlite3 *db;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CASE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_UPLUS 
   || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CAST 
  ){
    ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  }
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_FUNCTION && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AGG_FUNCTION ){
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  db = pWalker->pParse->db;
  n = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList ? p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  pDef = sqlite3FindFunction(db, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ಯದಿ( pDef==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( pDef!=0 );
#endif
  ಯದಿ( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  Walker w;
  memset(&w, 0, ಪರಿಮಾಣ(w));
  w.pParse = pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Can_Return_Subtype;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಪ್ರತ್ಯಾಗಮ w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ sqlite3_Indexed_ಅಭಿವ್ಯಕ್ತಿ_Lookup(
  Parse *pParse,   /* The parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,     /* The expression to potentially bypass */
  ಪೂರ್ಣಾಙ್ಕ target       /* Where to store the result of the expression */
){
  Indexed_ಅಭಿವ್ಯಕ್ತಿ *p;
  Vdbe *v;
  ಪ್ರತಿ(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 ಅಭಿವ್ಯಕ್ತಿ_Aff;
    ಪೂರ್ಣಾಙ್ಕ iDataCur = p->iDataCur;
    ಯದಿ( iDataCur<0 ) ಅನುವರ್ತ;
    ಯದಿ( pParse->iSelfTab ){
      ಯದಿ( p->iDataCur!=pParse->iSelfTab-1 ) ಅನುವರ್ತ;
      iDataCur = -1;
    }
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ, p->p_ಅಭಿವ್ಯಕ್ತಿ, iDataCur)!=0 ) ಅನುವರ್ತ;
    ಪ್ರತಿಜ್ಞಾ( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    ಅಭಿವ್ಯಕ್ತಿ_Aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ);
    ಯದಿ( (ಅಭಿವ್ಯಕ್ತಿ_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (ಅಭಿವ್ಯಕ್ತಿ_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (ಅಭಿವ್ಯಕ್ತಿ_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      ಅನುವರ್ತ;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_SubtArg)
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) 
    ){
      ಅನುವರ್ತ;
    }

    v = pParse->pVdbe;
    ಪ್ರತಿಜ್ಞಾ( v!=0 );
    ಯದಿ( p->bMaybeNullRow || sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      ಪೂರ್ಣಾಙ್ಕ addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }ಅನ್ಯಥಾ{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    ಪ್ರತ್ಯಾಗಮ target;
  }
  ಪ್ರತ್ಯಾಗಮ -1;  /* Not found */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Partidx_ಅಭಿವ್ಯಕ್ತಿ_Lookup(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ iTarget){
  Indexed_ಅಭಿವ್ಯಕ್ತಿ *p;
  ಪ್ರತಿ(p=pParse->p_Idx_Part_ಅಭಿವ್ಯಕ್ತಿ; p; p=p->pIENext){
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iColumn==p->iIdxCol && p_ಅಭಿವ್ಯಕ್ತಿ->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      ಪೂರ್ಣಾಙ್ಕ addr = 0;
      ಪೂರ್ಣಾಙ್ಕ ret;

      ಯದಿ( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, p->p_ಅಭಿವ್ಯಕ್ತಿ, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ಧ್ರುವ ವರ್ಣ*)&p->aff, 1);
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(pParse, p->p_ಅಭಿವ್ಯಕ್ತಿ) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      ಯದಿ( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      ಪ್ರತ್ಯಾಗಮ ret;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,       /* AND or OR expression to be coded */
  ಪೂರ್ಣಾಙ್ಕ target,        /* Put result in this register, guaranteed */
  ಪೂರ್ಣಾಙ್ಕ *pTmpReg       /* Write a temporary register here */
){
  ಪೂರ್ಣಾಙ್ಕ op;            /* The opcode.  TK_AND or TK_OR */
  ಪೂರ್ಣಾಙ್ಕ skipOp;        /* Opcode for the branch that skips one operand */
  ಪೂರ್ಣಾಙ್ಕ addrSkip;      /* Branch instruction that skips one of the operands */
  ಪೂರ್ಣಾಙ್ಕ regSS = 0;     /* Register holding computed operand when other omitted */
  ಪೂರ್ಣಾಙ್ಕ r1, r2;        /* Registers for left and right operands, respectively */
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
  ಯದಿ( pAlt!=p_ಅಭಿವ್ಯಕ್ತಿ ){
    r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    ಪ್ರತ್ಯಾಗಮ target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_First(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, pTmpReg);
  }ಅನ್ಯಥಾ{
    /* Compute the left operand first */
    r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, target);
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }ಅನ್ಯಥಾ{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  ಯದಿ( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  ಪ್ರತ್ಯಾಗಮ target;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  ಪೂರ್ಣಾಙ್ಕ op;                   /* The opcode being coded */
  ಪೂರ್ಣಾಙ್ಕ inReg = target;       /* Results stored in register inReg */
  ಪೂರ್ಣಾಙ್ಕ regFree1 = 0;         /* If non-zero free this temporary register */
  ಪೂರ್ಣಾಙ್ಕ regFree2 = 0;         /* If non-zero free this temporary register */
  ಪೂರ್ಣಾಙ್ಕ r1, r2;               /* Various register numbers */
  ಅಭಿವ್ಯಕ್ತಿ tempX;               /* Temporary expression node */
  ಪೂರ್ಣಾಙ್ಕ p5 = 0;

  ಪ್ರತಿಜ್ಞಾ( target>0 && target<=pParse->nMem );
  ಪ್ರತಿಜ್ಞಾ( v!=0 );

ಅಭಿವ್ಯಕ್ತಿ_code_doover:
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ){
    op = TK_NULL;
  }ಅನ್ಯಥಾ ಯದಿ( pParse->pIdxEpr!=0
   && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Leaf)
   && (r1 = sqlite3_Indexed_ಅಭಿವ್ಯಕ್ತಿ_Lookup(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target))>=0
  ){
    ಪ್ರತ್ಯಾಗಮ r1;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );
    op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  }
  ಪ್ರತಿಜ್ಞಾ( op!=TK_ORDER );
  ವಿಕಲ್ಪ( op ){
    ಪಕ್ಷ TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
      ಸಂರಚನಾ AggInfo_col *pCol;
      ಪ್ರತಿಜ್ಞಾ( pAggInfo!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=0 );
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        ಭಙ್ಗ;
      }
      pCol = &pAggInfo->aCol[p_ಅಭಿವ್ಯಕ್ತಿ->iAgg];
      ಯದಿ( !pAggInfo->directMode ){
        ಪ್ರತ್ಯಾಗಮ AggInfoColumnReg(pAggInfo, p_ಅಭಿವ್ಯಕ್ತಿ->iAgg);
      }ಅನ್ಯಥಾ ಯದಿ( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        ಯದಿ( pTab==0 ){
          /* No comment added */
        }ಅನ್ಯಥಾ ಯದಿ( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }ಅನ್ಯಥಾ{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          ಯದಿ( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        ಪ್ರತ್ಯಾಗಮ target;
      }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_ಅಭಿವ್ಯಕ್ತಿ->iTable, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn, target);
        ಪ್ರತ್ಯಾಗಮ target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    ಪಕ್ಷ TK_COLUMN: {
      ಪೂರ್ಣಾಙ್ಕ iTab = p_ಅಭಿವ್ಯಕ್ತಿ->iTable;
      ಪೂರ್ಣಾಙ್ಕ iReg;
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        ಪೂರ್ಣಾಙ್ಕ aff;
        iReg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ,target);
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn);
        ಯದಿ( aff>SQLITE_AFF_BLOB ){
          ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ zAff[] = "B\000C\000D\000E\000F";
          ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_BLOB=='A' );
          ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        ಪ್ರತ್ಯಾಗಮ iReg;
      }
      ಯದಿ( iTab<0 ){
        ಯದಿ( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          ಪೂರ್ಣಾಙ್ಕ iSrc;
          ಪೂರ್ಣಾಙ್ಕ iCol = p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
          pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
          ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
          ಪ್ರತಿಜ್ಞಾ( iCol>=XN_ROWID );
          ಪ್ರತಿಜ್ಞಾ( iCol<pTab->nCol );
          ಯದಿ( iCol<0 ){
            ಪ್ರತ್ಯಾಗಮ -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          ಯದಿ( pCol->colFlags & COLFLAG_GENERATED ){
            ಯದಿ( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_ಸಂದೇಶ(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              ಪ್ರತ್ಯಾಗಮ 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            ಯದಿ( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            ಪ್ರತ್ಯಾಗಮ iSrc;
          }ಅನ್ಯಥಾ
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          ಯದಿ( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            ಪ್ರತ್ಯಾಗಮ target;
          }ಅನ್ಯಥಾ{
            ಪ್ರತ್ಯಾಗಮ iSrc;
          }
        }ಅನ್ಯಥಾ{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      ಅನ್ಯಥಾ ಯದಿ( pParse->p_Idx_Part_ಅಭಿವ್ಯಕ್ತಿ 
       && 0!=(r1 = ಅಭಿವ್ಯಕ್ತಿ_Partidx_ಅಭಿವ್ಯಕ್ತಿ_Lookup(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target))
      ){
        ಪ್ರತ್ಯಾಗಮ r1;
      }
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0 );
      iReg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab,
                               p_ಅಭಿವ್ಯಕ್ತಿ->iColumn, iTab, target,
                               p_ಅಭಿವ್ಯಕ್ತಿ->op2);
      ಪ್ರತ್ಯಾಗಮ iReg;
    }
    ಪಕ್ಷ TK_INTEGER: {
      codeInteger(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, 0, target);
      ಪ್ರತ್ಯಾಗಮ target;
    }
    ಪಕ್ಷ TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ), target);
      ಪ್ರತ್ಯಾಗಮ target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    ಪಕ್ಷ TK_FLOAT: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      codeReal(v, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, 0, target);
      ಪ್ರತ್ಯಾಗಮ target;
    }
#endif
    ಪಕ್ಷ TK_STRING: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3VdbeLoadString(v, target, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ಪ್ರತ್ಯಾಗಮ target;
    }
    ಪಕ್ಷ TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_ಅಭಿವ್ಯಕ್ತಿ->y.nReg - 1);
      ಪ್ರತ್ಯಾಗಮ target;
    }
    ಸ್ವಭಾವ: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      ಪ್ರತಿಜ್ಞಾ( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      ಪ್ರತ್ಯಾಗಮ target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    ಪಕ್ಷ TK_BLOB: {
      ಪೂರ್ಣಾಙ್ಕ n;
      ಧ್ರುವ ವರ್ಣ *z;
      ವರ್ಣ *zBlob;
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[0]=='x' || p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[0]=='X' );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[1]=='\'' );
      z = &p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      ಪ್ರತಿಜ್ಞಾ( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      ಪ್ರತ್ಯಾಗಮ target;
    }
#endif
    ಪಕ್ಷ TK_VARIABLE: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn, target);
      ಪ್ರತ್ಯಾಗಮ target;
    }
    ಪಕ್ಷ TK_REGISTER: {
      ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    ಪಕ್ಷ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, target);
      ಪ್ರತಿಜ್ಞಾ( inReg==target );
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, 0));
      ಪ್ರತ್ಯಾಗಮ inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    ಪಕ್ಷ TK_IS:
    ಪಕ್ಷ TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ಪಕ್ಷ TK_LT:
    ಪಕ್ಷ TK_LE:
    ಪಕ್ಷ TK_GT:
    ಪಕ್ಷ TK_GE:
    ಪಕ್ಷ TK_NE:
    ಪಕ್ಷ TK_EQ: {
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಪೂರ್ಣಾಙ್ಕ addrIsNull = 0;
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Vector(p_ಎಡ) ){
        codeVectorCompare(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target, op, p5);
      }ಅನ್ಯಥಾ{
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                                     &r1, &r2, &regFree1, &regFree2);
        }ಅನ್ಯಥಾ{
          r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
          r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Commuted));
        ಪ್ರತಿಜ್ಞಾ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        ಪ್ರತಿಜ್ಞಾ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        ಪ್ರತಿಜ್ಞಾ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        ಪ್ರತಿಜ್ಞಾ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        ಪ್ರತಿಜ್ಞಾ(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        ಪ್ರತಿಜ್ಞಾ(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        ಯದಿ( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          ಯದಿ( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_AND:
    ಪಕ್ಷ TK_OR: {
      inReg = ಅಭಿವ್ಯಕ್ತಿ_Code_Target_And_Or(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target, &regFree1);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_PLUS:
    ಪಕ್ಷ TK_STAR:
    ಪಕ್ಷ TK_MINUS:
    ಪಕ್ಷ TK_REM:
    ಪಕ್ಷ TK_BITAND:
    ಪಕ್ಷ TK_BITOR:
    ಪಕ್ಷ TK_SLASH:
    ಪಕ್ಷ TK_LSHIFT:
    ಪಕ್ಷ TK_RSHIFT:
    ಪಕ್ಷ TK_CONCAT: {
      ಪೂರ್ಣಾಙ್ಕ addrIsNull;
      ಪ್ರತಿಜ್ಞಾ( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      ಪ್ರತಿಜ್ಞಾ( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      ಪ್ರತಿಜ್ಞಾ( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      ಪ್ರತಿಜ್ಞಾ( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      ಪ್ರತಿಜ್ಞಾ( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      ಪ್ರತಿಜ್ಞಾ( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      ಪ್ರತಿಜ್ಞಾ( TK_LSHIFT==OP_Shift_ಎಡ );   testcase( op==TK_LSHIFT );
      ಪ್ರತಿಜ್ಞಾ( TK_RSHIFT==OP_Shift_ಬಲ );  testcase( op==TK_RSHIFT );
      ಪ್ರತಿಜ್ಞಾ( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) ){
        addrIsNull = ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ಅನ್ಯಥಾ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
        r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ಯದಿ( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_UMINUS: {
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಪ್ರತಿಜ್ಞಾ( p_ಎಡ );
      ಯದಿ( p_ಎಡ->op==TK_INTEGER ){
        codeInteger(pParse, p_ಎಡ, 1, target);
        ಪ್ರತ್ಯಾಗಮ target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }ಅನ್ಯಥಾ ಯದಿ( p_ಎಡ->op==TK_FLOAT ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
        codeReal(v, p_ಎಡ->u.zToken, 1, target);
        ಪ್ರತ್ಯಾಗಮ target;
#endif
      }ಅನ್ಯಥಾ{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_ಮೌಲ್ಯ|EP_TokenOnly;
        tempX.u.i_ಮೌಲ್ಯ = 0;
        ಅಭಿವ್ಯಕ್ತಿ_Clear_VVAProperties(&tempX);
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_BITNOT:
    ಪಕ್ಷ TK_NOT: {
      ಪ್ರತಿಜ್ಞಾ( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      ಪ್ರತಿಜ್ಞಾ( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_TRUTH: {
      ಪೂರ್ಣಾಙ್ಕ isTrue;    /* IS TRUE or IS NOT TRUE */
      ಪೂರ್ಣಾಙ್ಕ bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      bNormal = p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_ISNULL:
    ಪಕ್ಷ TK_NOTNULL: {
      ಪೂರ್ಣಾಙ್ಕ addr;
      ಪ್ರತಿಜ್ಞಾ( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ಪ್ರತಿಜ್ಞಾ( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
      ಯದಿ( pInfo==0
       || NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg<0)
       || NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=pInfo->nFunc)
      ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
        sqlite3_Error_ಸಂದೇಶ(pParse, "misuse of aggregate: %#T()", p_ಅಭಿವ್ಯಕ್ತಿ);
      }ಅನ್ಯಥಾ{
        ಪ್ರತ್ಯಾಗಮ AggInfoFuncReg(pInfo, p_ಅಭಿವ್ಯಕ್ತಿ->iAgg);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_FUNCTION: {
      ಅಭಿವ್ಯಕ್ತಿ_List *pFarg;       /* List of function arguments */
      ಪೂರ್ಣಾಙ್ಕ nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ಧ್ರುವ ವರ್ಣ *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      ಪೂರ್ಣಾಙ್ಕ i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
        ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->regResult;
      }
#endif

      ಯದಿ( ConstFactorOk(pParse)
       && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(pParse,p_ಅಭಿವ್ಯಕ್ತಿ)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Run_Just_Once(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, -1);
      }
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly) );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      pFarg = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
      nFarg = pFarg ? pFarg->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      zId = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      ಯದಿ( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      ಯದಿ( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "unknown function: %#T()", p_ಅಭಿವ್ಯಕ್ತಿ);
        ಭಙ್ಗ;
      }
      ಯದಿ( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        ಪ್ರತಿಜ್ಞಾ( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        ಪ್ರತಿಜ್ಞಾ( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }ಅನ್ಯಥಾ ಯದಿ( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function_Usable(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pDef);
      }

      ಪ್ರತಿ(i=0; i<nFarg; i++){
        ಯದಿ( i<32 && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pParse, pFarg->a[i].p_ಅಭಿವ್ಯಕ್ತಿ) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        ಯದಿ( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pFarg->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
        }
      }
      ಯದಿ( pFarg ){
        ಯದಿ( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }ಅನ್ಯಥಾ{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        ಯದಿ( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 ಅಭಿವ್ಯಕ್ತಿ_Op;
          ಪ್ರತಿಜ್ಞಾ( nFarg==1 );
          ಪ್ರತಿಜ್ಞಾ( pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
          ಅಭಿವ್ಯಕ್ತಿ_Op = pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ->op;
          ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Op==TK_COLUMN || ಅಭಿವ್ಯಕ್ತಿ_Op==TK_AGG_COLUMN ){
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            ಪ್ರತಿಜ್ಞಾ( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }ಅನ್ಯಥಾ{
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
      ಯದಿ( nFarg>=2 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_ಅಭಿವ್ಯಕ್ತಿ);
      }ಅನ್ಯಥಾ ಯದಿ( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
      }
#endif
      ಯದಿ( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        ಯದಿ( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (ವರ್ಣ *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_ಅಭಿವ್ಯಕ್ತಿ->op2);
      ಯದಿ( nFarg ){
        ಯದಿ( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      ಪ್ರತ್ಯಾಗಮ target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ಪಕ್ಷ TK_EXISTS:
    ಪಕ್ಷ TK_SELECT: {
      ಪೂರ್ಣಾಙ್ಕ nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      ಯದಿ( pParse->db->mallocFailed ){
        ಪ್ರತ್ಯಾಗಮ 0;
      }ಅನ್ಯಥಾ ಯದಿ( op==TK_SELECT
             && ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) )
             && (nCol = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }ಅನ್ಯಥಾ{
        ಪ್ರತ್ಯಾಗಮ sqlite3CodeSubselect(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_SELECT_COLUMN: {
      ಪೂರ್ಣಾಙ್ಕ n;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಯದಿ( p_ಎಡ->iTable==0 || pParse->withinRJSubrtn > p_ಎಡ->op2 ){
        p_ಎಡ->iTable = sqlite3CodeSubselect(pParse, p_ಎಡ);
        p_ಎಡ->op2 = pParse->withinRJSubrtn;
      }
      ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op==TK_SELECT || p_ಎಡ->op==TK_ERROR );
      n = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಎಡ);
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable!=n ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "%d columns assigned %d values",
                                p_ಅಭಿವ್ಯಕ್ತಿ->iTable, n);
      }
      ಪ್ರತ್ಯಾಗಮ p_ಎಡ->iTable + p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
    }
    ಪಕ್ಷ TK_IN: {
      ಪೂರ್ಣಾಙ್ಕ destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಙ್ಕ destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      ಪ್ರತ್ಯಾಗಮ target;
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
    ಪಕ್ಷ TK_BETWEEN: {
      ಅಭಿವ್ಯಕ್ತಿ_Code_Between(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target, 0, 0);
      ಪ್ರತ್ಯಾಗಮ target;
    }
    ಪಕ್ಷ TK_COLLATE: {
      ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        ಪ್ರತ್ಯಾಗಮ target;
      }ಅನ್ಯಥಾ{
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        ಗಚ್ಛ ಅಭಿವ್ಯಕ್ತಿ_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    ಪಕ್ಷ TK_SPAN:
    ಪಕ್ಷ TK_UPLUS: {
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಗಚ್ಛ ಅಭಿವ್ಯಕ್ತಿ_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    ಪಕ್ಷ TK_TRIGGER: {
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
      ಪೂರ್ಣಾಙ್ಕ iCol;
      ಪೂರ್ಣಾಙ್ಕ p1;

      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
      pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
      iCol = p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
      p1 = p_ಅಭಿವ್ಯಕ್ತಿ->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable==0 || p_ಅಭಿವ್ಯಕ್ತಿ->iTable==1 );
      ಪ್ರತಿಜ್ಞಾ( iCol>=-1 && iCol<pTab->nCol );
      ಪ್ರತಿಜ್ಞಾ( pTab->iPKey<0 || iCol!=pTab->iPKey );
      ಪ್ರತಿಜ್ಞಾ( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_ಅಭಿವ್ಯಕ್ತಿ->iTable ? "new" : "old"),
        (p_ಅಭಿವ್ಯಕ್ತಿ->iColumn<0 ? "rowid" : p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      ಯದಿ( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ TK_VECTOR: {
      sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
      ಭಙ್ಗ;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    ಪಕ್ಷ TK_IF_NULL_ROW: {
      ಪೂರ್ಣಾಙ್ಕ addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
      ಯದಿ( pAggInfo ){
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=0 && p_ಅಭಿವ್ಯಕ್ತಿ->iAgg<pAggInfo->nColumn );
        ಯದಿ( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_ಅಭಿವ್ಯಕ್ತಿ->iAgg);
          ಭಙ್ಗ;
        }
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_ಅಭಿವ್ಯಕ್ತಿ->iAgg].iSorterColumn,
                            target);
          inReg = target;
          ಭಙ್ಗ;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_ಅಭಿವ್ಯಕ್ತಿ->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, target);
      ಪ್ರತಿಜ್ಞಾ( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      ಭಙ್ಗ;
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
    ಪಕ್ಷ TK_CASE: {
      ಪೂರ್ಣಾಙ್ಕ endLabel;                     /* GOTO label for end of CASE stmt */
      ಪೂರ್ಣಾಙ್ಕ nextCase;                     /* GOTO label for next WHEN clause */
      ಪೂರ್ಣಾಙ್ಕ n_ಅಭಿವ್ಯಕ್ತಿ;                        /* 2x number of WHEN terms */
      ಪೂರ್ಣಾಙ್ಕ i;                            /* Loop counter */
      ಅಭಿವ್ಯಕ್ತಿ_List *pEList;                 /* List of WHEN terms */
      ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *aListelem;  /* Array of WHEN terms */
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
      endLabel = sqlite3VdbeMakeLabel(pParse);
      ಯದಿ( (pX = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ)!=0 ){
        pDel = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pX, 0);
        ಯದಿ( db->mallocFailed ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDel);
          ಭಙ್ಗ;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_To_Register(pDel, ಅಭಿವ್ಯಕ್ತಿ_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, ಪರಿಮಾಣ(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_ಎಡ = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      ಪ್ರತಿ(i=0; i<n_ಅಭಿವ್ಯಕ್ತಿ-1; i=i+2){
        ಯದಿ( pX ){
          ಪ್ರತಿಜ್ಞಾ( pTest!=0 );
          opCompare.p_ಬಲ = aListelem[i].p_ಅಭಿವ್ಯಕ್ತಿ;
        }ಅನ್ಯಥಾ{
          pTest = aListelem[i].p_ಅಭಿವ್ಯಕ್ತಿ;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, aListelem[i+1].p_ಅಭಿವ್ಯಕ್ತಿ, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      ಯದಿ( (n_ಅಭಿವ್ಯಕ್ತಿ&1)!=0 ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pEList->a[n_ಅಭಿವ್ಯಕ್ತಿ-1].p_ಅಭಿವ್ಯಕ್ತಿ, target);
      }ಅನ್ಯಥಾ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_TRIGGER
    ಪಕ್ಷ TK_RAISE: {
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Rollback
           || p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Abort
           || p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Fail
           || p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Ignore
      );
      ಯದಿ( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_ಸಂದೇಶ(pParse,
                       "RAISE() may only be used within a trigger-program");
        ಪ್ರತ್ಯಾಗಮ 0;
      }
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }ಅನ್ಯಥಾ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ, r1);
      }
      ಭಙ್ಗ;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  ಪ್ರತ್ಯಾಗಮ inReg;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,      /* The expression to code when the VDBE initializes */
  ಪೂರ್ಣಾಙ್ಕ regDest       /* Store the value in this register */
){
  ಅಭಿವ್ಯಕ್ತಿ_List *p;
  ಪ್ರತಿಜ್ಞಾ( ConstFactorOk(pParse) );
  ಪ್ರತಿಜ್ಞಾ( regDest!=0 );
  p = pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ;
  ಯದಿ( regDest<0 && p ){
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(pItem=p->a, i=p->n_ಅಭಿವ್ಯಕ್ತಿ; i>0; pItem++, i--){
      ಯದಿ( pItem->fg.reusable
       && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pItem->p_ಅಭಿವ್ಯಕ್ತಿ,p_ಅಭಿವ್ಯಕ್ತಿ,-1)==0
      ){
        ಪ್ರತ್ಯಾಗಮ pItem->u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg;
      }
    }
  }
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    ಪೂರ್ಣಾಙ್ಕ addr;
    ಪ್ರತಿಜ್ಞಾ( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    ಯದಿ( !pParse->db->mallocFailed ){
      ಯದಿ( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
    sqlite3VdbeJumpHere(v, addr);
  }ಅನ್ಯಥಾ{
    p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, p, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಯದಿ( p ){
       ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem = &p->a[p->n_ಅಭಿವ್ಯಕ್ತಿ-1];
       pItem->fg.reusable = regDest<0;
       ಯದಿ( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg = regDest;
    }
    pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ = p;
  }
  ಪ್ರತ್ಯಾಗಮ regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  ಪೂರ್ಣಾಙ್ಕ iReg,        /* First register to set to NULL */
  ಪೂರ್ಣಾಙ್ಕ nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  ಅಭಿವ್ಯಕ್ತಿ t;
  memset(&t, 0, ಪರಿಮಾಣ(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Run_Just_Once(pParse, &t, iReg);
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ *pReg){
  ಪೂರ್ಣಾಙ್ಕ r2;
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಯದಿ( ConstFactorOk(pParse)
   && ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ!=0)
   && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_REGISTER
   && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(pParse, p_ಅಭಿವ್ಯಕ್ತಿ)
  ){
    *pReg  = 0;
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Run_Just_Once(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, -1);
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, r1);
    ಯದಿ( r2==r1 ){
      *pReg = r1;
    }ಅನ್ಯಥಾ{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ target){
  ಪೂರ್ಣಾಙ್ಕ inReg;

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );
  ಪ್ರತಿಜ್ಞಾ( target>0 && target<=pParse->nMem );
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  ಯದಿ( pParse->pVdbe==0 ) ಪ್ರತ್ಯಾಗಮ;
  inReg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target);
  ಯದಿ( inReg!=target ){
    u8 op;
    ಅಭಿವ್ಯಕ್ತಿ *pX = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ);
    testcase( pX!=p_ಅಭಿವ್ಯಕ್ತಿ );
    ಯದಿ( ALWAYS(pX)
     && (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }ಅನ್ಯಥಾ{
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Copy(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ target){
  sqlite3 *db = pParse->db;
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  ಯದಿ( !db->mallocFailed ) sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Factorable(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ target){
  ಯದಿ( pParse->okConstFactor && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(pParse,p_ಅಭಿವ್ಯಕ್ತಿ) ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Run_Just_Once(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target);
  }ಅನ್ಯಥಾ{
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Copy(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target);
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(
  Parse *pParse,     /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,   /* The expression list to be coded */
  ಪೂರ್ಣಾಙ್ಕ target,        /* Where to write results */
  ಪೂರ್ಣಾಙ್ಕ srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  ಪೂರ್ಣಾಙ್ಕ i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( pList!=0 );
  ಪ್ರತಿಜ್ಞಾ( target>0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  ಯದಿ( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  ಪ್ರತಿ(pItem=pList->a, i=0; i<n; i++, pItem++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pItem->p_ಅಭಿವ್ಯಕ್ತಿ;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ಯದಿ( pItem->fg.bSorterRef ){
      i--;
      n--;
    }ಅನ್ಯಥಾ
#endif
    ಯದಿ( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      ಯದಿ( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }ಅನ್ಯಥಾ{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }ಅನ್ಯಥಾ ಯದಿ( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(pParse,p_ಅಭಿವ್ಯಕ್ತಿ)
    ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Run_Just_Once(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target+i);
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ inReg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, target+i);
      ಯದಿ( inReg!=target+i ){
        VdbeOp *pOp;
        ಯದಿ( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }ಅನ್ಯಥಾ{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ n;
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
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,      /* The BETWEEN expression */
  ಪೂರ್ಣಾಙ್ಕ dest,         /* Jump destination or storage location */
  ಶೂನ್ಯ (*xJump)(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ), /* Action to take */
  ಪೂರ್ಣಾಙ್ಕ jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  ಅಭಿವ್ಯಕ್ತಿ ಅಭಿವ್ಯಕ್ತಿ_And;     /* The AND operator in  x>=y AND x<=z  */
  ಅಭಿವ್ಯಕ್ತಿ comp_ಎಡ;    /* The  x>=y  term */
  ಅಭಿವ್ಯಕ್ತಿ comp_ಬಲ;   /* The  x<=z  term */
  ಪೂರ್ಣಾಙ್ಕ regFree1 = 0; /* Temporary use register */
  ಅಭಿವ್ಯಕ್ತಿ *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_ಎಡ, 0, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
  memset(&comp_ಬಲ, 0, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
  memset(&ಅಭಿವ್ಯಕ್ತಿ_And, 0, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));

  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pDel = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
  ಯದಿ( db->mallocFailed==0 ){
    ಅಭಿವ್ಯಕ್ತಿ_And.op = TK_AND;
    ಅಭಿವ್ಯಕ್ತಿ_And.p_ಎಡ = &comp_ಎಡ;
    ಅಭಿವ್ಯಕ್ತಿ_And.p_ಬಲ = &comp_ಬಲ;
    comp_ಎಡ.op = TK_GE;
    comp_ಎಡ.p_ಎಡ = pDel;
    comp_ಎಡ.p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
    comp_ಬಲ.op = TK_LE;
    comp_ಬಲ.p_ಎಡ = pDel;
    comp_ಬಲ.p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_To_Register(pDel, ಅಭಿವ್ಯಕ್ತಿ_Code_Vector(pParse, pDel, &regFree1));
    ಯದಿ( xJump ){
      xJump(pParse, &ಅಭಿವ್ಯಕ್ತಿ_And, dest, jumpIfNull);
    }ಅನ್ಯಥಾ{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, &ಅಭಿವ್ಯಕ್ತಿ_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ dest, ಪೂರ್ಣಾಙ್ಕ jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ op = 0;
  ಪೂರ್ಣಾಙ್ಕ regFree1 = 0;
  ಪೂರ್ಣಾಙ್ಕ regFree2 = 0;
  ಪೂರ್ಣಾಙ್ಕ r1, r2;

  ಪ್ರತಿಜ್ಞಾ( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ಯದಿ( NEVER(v==0) )     ಪ್ರತ್ಯಾಗಮ;  /* Existence of VDBE checked by caller */
  ಯದಿ( NEVER(p_ಅಭಿವ್ಯಕ್ತಿ==0) ) ಪ್ರತ್ಯಾಗಮ;  /* No way this can happen */
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Immutable) );
  op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ವಿಕಲ್ಪ( op ){
    ಪಕ್ಷ TK_AND:
    ಪಕ್ಷ TK_OR: {
      ಅಭಿವ್ಯಕ್ತಿ *pAlt = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ);
      ಯದಿ( pAlt!=p_ಅಭಿವ್ಯಕ್ತಿ ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pAlt, dest, jumpIfNull);
      }ಅನ್ಯಥಾ{
        ಅಭಿವ್ಯಕ್ತಿ *pFirst, *pSecond;
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_First(p_ಅಭಿವ್ಯಕ್ತಿ) ){
          pFirst = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
          pSecond = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        }ಅನ್ಯಥಾ{
          pFirst = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          pSecond = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        }
        ಯದಿ( op==TK_AND ){
          ಪೂರ್ಣಾಙ್ಕ d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }ಅನ್ಯಥಾ{
          testcase( jumpIfNull==0 );
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest, jumpIfNull);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_TRUTH: {
      ಪೂರ್ಣಾಙ್ಕ isNot;      /* IS NOT TRUE or IS NOT FALSE */
      ಪೂರ್ಣಾಙ್ಕ isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_ISNOT;
      isTrue = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ಯದಿ( isTrue ^ isNot ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }ಅನ್ಯಥಾ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_IS:
    ಪಕ್ಷ TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ಪಕ್ಷ TK_LT:
    ಪಕ್ಷ TK_LE:
    ಪಕ್ಷ TK_GT:
    ಪಕ್ಷ TK_GE:
    ಪಕ್ಷ TK_NE:
    ಪಕ್ಷ TK_EQ: {
      ಪೂರ್ಣಾಙ್ಕ addrIsNull;
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Vector(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) ) ಗಚ್ಛ default_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ಅನ್ಯಥಾ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
        r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, op,
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
      ಯದಿ( addrIsNull ){
        ಯದಿ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_ISNULL:
    ಪಕ್ಷ TK_NOTNULL: {
      ಪ್ರತಿಜ್ಞಾ( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ಪ್ರತಿಜ್ಞಾ( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      ಪ್ರತಿಜ್ಞಾ( regFree1==0 || regFree1==r1 );
      ಯದಿ( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      ಅಭಿವ್ಯಕ್ತಿ_Code_Between(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, dest, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True, jumpIfNull);
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ಪಕ್ಷ TK_IN: {
      ಪೂರ್ಣಾಙ್ಕ destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಙ್ಕ destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      ಭಙ್ಗ;
    }
#endif
    ಸ್ವಭಾವ: {
    default_ಅಭಿವ್ಯಕ್ತಿ:
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Always_True(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        sqlite3VdbeGoto(v, dest);
      }ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        /* No-op */
      }ಅನ್ಯಥಾ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ಭಙ್ಗ;
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ dest, ಪೂರ್ಣಾಙ್ಕ jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ op = 0;
  ಪೂರ್ಣಾಙ್ಕ regFree1 = 0;
  ಪೂರ್ಣಾಙ್ಕ regFree2 = 0;
  ಪೂರ್ಣಾಙ್ಕ r1, r2;

  ಪ್ರತಿಜ್ಞಾ( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ಯದಿ( NEVER(v==0) ) ಪ್ರತ್ಯಾಗಮ; /* Existence of VDBE checked by caller */
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 )    ಪ್ರತ್ಯಾಗಮ;
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

  ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಪಕ್ಷ TK_AND:
    ಪಕ್ಷ TK_OR: {
      ಅಭಿವ್ಯಕ್ತಿ *pAlt = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ);
      ಯದಿ( pAlt!=p_ಅಭಿವ್ಯಕ್ತಿ ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pAlt, dest, jumpIfNull);
      }ಅನ್ಯಥಾ{
        ಅಭಿವ್ಯಕ್ತಿ *pFirst, *pSecond;
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_First(p_ಅಭಿವ್ಯಕ್ತಿ) ){
          pFirst = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
          pSecond = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        }ಅನ್ಯಥಾ{
          pFirst = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          pSecond = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        }
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pSecond, dest, jumpIfNull);
        }ಅನ್ಯಥಾ{
          ಪೂರ್ಣಾಙ್ಕ d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest, jumpIfNull);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_TRUTH: {
      ಪೂರ್ಣಾಙ್ಕ isNot;   /* IS NOT TRUE or IS NOT FALSE */
      ಪೂರ್ಣಾಙ್ಕ isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_ISNOT;
      isTrue = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ಯದಿ( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }ಅನ್ಯಥಾ{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_IS:
    ಪಕ್ಷ TK_ISNOT:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT );
      op = (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ಪಕ್ಷ TK_LT:
    ಪಕ್ಷ TK_LE:
    ಪಕ್ಷ TK_GT:
    ಪಕ್ಷ TK_GE:
    ಪಕ್ಷ TK_NE:
    ಪಕ್ಷ TK_EQ: {
      ಪೂರ್ಣಾಙ್ಕ addrIsNull;
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Vector(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) ) ಗಚ್ಛ default_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ಅನ್ಯಥಾ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
        r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, op,
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
      ಯದಿ( addrIsNull ){
        ಯದಿ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_ISNULL:
    ಪಕ್ಷ TK_NOTNULL: {
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      ಪ್ರತಿಜ್ಞಾ( regFree1==0 || regFree1==r1 );
      ಯದಿ( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      ಅಭಿವ್ಯಕ್ತಿ_Code_Between(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, dest, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False, jumpIfNull);
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ಪಕ್ಷ TK_IN: {
      ಯದಿ( jumpIfNull ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, dest, dest);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_IN(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      ಭಙ್ಗ;
    }
#endif
    ಸ್ವಭಾವ: {
    default_ಅಭಿವ್ಯಕ್ತಿ:
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        sqlite3VdbeGoto(v, dest);
      }ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Always_True(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        /* no-op */
      }ಅನ್ಯಥಾ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Temp(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ಭಙ್ಗ;
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False_Dup(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ dest,ಪೂರ್ಣಾಙ್ಕ jumpIfNull){
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ *pCopy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  ಯದಿ( db->mallocFailed==0 ){
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Compare_Variable(
  ಧ್ರುವ Parse *pParse,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pVar,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ
){
  ಪೂರ್ಣಾಙ್ಕ res = 2;
  ಪೂರ್ಣಾಙ್ಕ iVar;
  sqlite3_ಮೌಲ್ಯ *pL, *pR = 0;
 
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_VARIABLE && pVar->iColumn==p_ಅಭಿವ್ಯಕ್ತಿ->iColumn ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ಪ್ರತ್ಯಾಗಮ 2;
  sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  ಯದಿ( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_ಮೌಲ್ಯ(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    ಯದಿ( pL ){
      ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(pL)==SQLITE_TEXT ){
        sqlite3_ಮೌಲ್ಯ_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_ಮೌಲ್ಯ_Free(pR);
    sqlite3_ಮೌಲ್ಯ_Free(pL);
  }
  ಪ್ರತ್ಯಾಗಮ res;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(
  ಧ್ರುವ Parse *pParse,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pA,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pB,
  ಪೂರ್ಣಾಙ್ಕ iTab
){
  u32 combinedFlags;
  ಯದಿ( pA==0 || pB==0 ){
    ಪ್ರತ್ಯಾಗಮ pB==pA ? 0 : 2;
  }
  ಯದಿ( pParse && pA->op==TK_VARIABLE ){
    ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  ಯದಿ( combinedFlags & EP_Int_ಮೌಲ್ಯ ){
    ಯದಿ( (pA->flags&pB->flags&EP_Int_ಮೌಲ್ಯ)!=0 && pA->u.i_ಮೌಲ್ಯ==pB->u.i_ಮೌಲ್ಯ ){
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪ್ರತ್ಯಾಗಮ 2;
  }
  ಯದಿ( pA->op!=pB->op || pA->op==TK_RAISE ){
    ಯದಿ( pA->op==TK_COLLATE && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pA->p_ಎಡ,pB,iTab)<2 ){
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಯದಿ( pB->op==TK_COLLATE && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pA,pB->p_ಎಡ,iTab)<2 ){
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಯದಿ( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }ಅನ್ಯಥಾ{
      ಪ್ರತ್ಯಾಗಮ 2;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pA, EP_Int_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pB, EP_Int_ಮೌಲ್ಯ) );
  ಯದಿ( pA->u.zToken ){
    ಯದಿ( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      ಯದಿ( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) ಪ್ರತ್ಯಾಗಮ 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      ಪ್ರತಿಜ್ಞಾ( pA->op==pB->op );
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pA,EP_WinFunc)!=ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pB,EP_WinFunc) ){
        ಪ್ರತ್ಯಾಗಮ 2;
      }
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pA,EP_WinFunc) ){
        ಯದಿ( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          ಪ್ರತ್ಯಾಗಮ 2;
        }
      }
#endif
    }ಅನ್ಯಥಾ ಯದಿ( pA->op==TK_NULL ){
      ಪ್ರತ್ಯಾಗಮ 0;
    }ಅನ್ಯಥಾ ಯದಿ( pA->op==TK_COLLATE ){
      ಯದಿ( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) ಪ್ರತ್ಯಾಗಮ 2;
    }ಅನ್ಯಥಾ
    ಯದಿ( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      ಪ್ರತ್ಯಾಗಮ 2;
    }
  }
  ಯದಿ( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) ಪ್ರತ್ಯಾಗಮ 2;
  ಯದಿ( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    ಯದಿ( combinedFlags & EP_xIsSelect ) ಪ್ರತ್ಯಾಗಮ 2;
    ಯದಿ( (combinedFlags & EP_FixedCol)==0
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pA->p_ಎಡ, pB->p_ಎಡ, iTab) ) ಪ್ರತ್ಯಾಗಮ 2;
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pA->p_ಬಲ, pB->p_ಬಲ, iTab) ) ಪ್ರತ್ಯಾಗಮ 2;
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(pA->x.pList, pB->x.pList, iTab) ) ಪ್ರತ್ಯಾಗಮ 2;
    ಯದಿ( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      ಯದಿ( pA->iColumn!=pB->iColumn ) ಪ್ರತ್ಯಾಗಮ 2;
      ಯದಿ( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) ಪ್ರತ್ಯಾಗಮ 2;
      ಯದಿ( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        ಪ್ರತ್ಯಾಗಮ 2;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pA, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pB, ಪೂರ್ಣಾಙ್ಕ iTab){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( pA==0 && pB==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( pA==0 || pB==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯದಿ( pA->n_ಅಭಿವ್ಯಕ್ತಿ!=pB->n_ಅಭಿವ್ಯಕ್ತಿ ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತಿ(i=0; i<pA->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಪೂರ್ಣಾಙ್ಕ res;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ_A = pA->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ_B = pB->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಯದಿ( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) ಪ್ರತ್ಯಾಗಮ 1;
    ಯದಿ( (res = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ_A, p_ಅಭಿವ್ಯಕ್ತಿ_B, iTab)) ) ಪ್ರತ್ಯಾಗಮ res;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Skip(ಅಭಿವ್ಯಕ್ತಿ *pA,ಅಭಿವ್ಯಕ್ತಿ *pB, ಪೂರ್ಣಾಙ್ಕ iTab){
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(
  ಧ್ರುವ Parse *pParse,/* Parsing context */
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p,      /* The expression to be checked */
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pNN,    /* The expression that is NOT NULL */
  ಪೂರ್ಣಾಙ್ಕ iTab,           /* Table being evaluated */
  ಪೂರ್ಣಾಙ್ಕ seenNot         /* Return true only if p can be any non-NULL value */
){
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( pNN );
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, p, pNN, iTab)==0 ){
    ಪ್ರತ್ಯಾಗಮ pNN->op!=TK_NULL;
  }
  ವಿಕಲ್ಪ( p->op ){
    ಪಕ್ಷ TK_IN: {
      ಯದಿ( seenNot && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_xIsSelect) ) ಪ್ರತ್ಯಾಗಮ 0;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ>0) );
      ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, 1);
    }
    ಪಕ್ಷ TK_BETWEEN: {
      ಅಭಿವ್ಯಕ್ತಿ_List *pList;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      pList = p->x.pList;
      ಪ್ರತಿಜ್ಞಾ( pList!=0 );
      ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 );
      ಯದಿ( seenNot ) ಪ್ರತ್ಯಾಗಮ 0;
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, pNN, iTab, 1)
       || ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ, pNN, iTab, 1)
      ){
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, 1);
    }
    ಪಕ್ಷ TK_EQ:
    ಪಕ್ಷ TK_NE:
    ಪಕ್ಷ TK_LT:
    ಪಕ್ಷ TK_LE:
    ಪಕ್ಷ TK_GT:
    ಪಕ್ಷ TK_GE:
    ಪಕ್ಷ TK_PLUS:
    ಪಕ್ಷ TK_MINUS:
    ಪಕ್ಷ TK_BITOR:
    ಪಕ್ಷ TK_LSHIFT:
    ಪಕ್ಷ TK_RSHIFT:
    ಪಕ್ಷ TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    ಪಕ್ಷ TK_STAR:
    ಪಕ್ಷ TK_REM:
    ಪಕ್ಷ TK_BITAND:
    ಪಕ್ಷ TK_SLASH: {
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, p->p_ಬಲ, pNN, iTab, seenNot) ) ಪ್ರತ್ಯಾಗಮ 1;
      /* no break */ deliberate_fall_through
    }
    ಪಕ್ಷ TK_SPAN:
    ಪಕ್ಷ TK_COLLATE:
    ಪಕ್ಷ TK_UPLUS:
    ಪಕ್ಷ TK_UMINUS: {
      ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, seenNot);
    }
    ಪಕ್ಷ TK_TRUTH: {
      ಯದಿ( seenNot ) ಪ್ರತ್ಯಾಗಮ 0;
      ಯದಿ( p->op2!=TK_IS ) ಪ್ರತ್ಯಾಗಮ 0;
      ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, 1);
    }
    ಪಕ್ಷ TK_BITNOT:
    ಪಕ್ಷ TK_NOT: {
      ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, 1);
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Not_True(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ v;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NULL ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRUEFALSE && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ)==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  v = 1;
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p_ಅಭಿವ್ಯಕ್ತಿ, &v, 0, 0) && v==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_IIF(sqlite3 *db, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION ){
    ಧ್ರುವ ವರ್ಣ *z = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
    FuncDef *pDef;
    ಯದಿ( (z[0]!='i' && z[0]!='I') ) ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==0 ) ಪ್ರತ್ಯಾಗಮ 0;
    pDef = sqlite3FindFunction(db, z, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    ಯದಿ( pDef==0 ) ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
    ಯದಿ( NEVER(pDef==0) ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
    ಯದಿ( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CASE ){
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ!=0 ) ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
  ಪ್ರತಿಜ್ಞಾ( pList!=0 );
  ಯದಿ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯದಿ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==3 && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Not_True(pList->a[2].p_ಅಭಿವ್ಯಕ್ತಿ) ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_ಅಭಿವ್ಯಕ್ತಿ(
  ಧ್ರುವ Parse *pParse,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pE1,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pE2,
  ಪೂರ್ಣಾಙ್ಕ iTab
){
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pE1, pE2, iTab)==0 ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಯದಿ( pE2->op==TK_OR
   && (sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_ಅಭಿವ್ಯಕ್ತಿ(pParse, pE1, pE2->p_ಎಡ, iTab)
             || sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_ಅಭಿವ್ಯಕ್ತಿ(pParse, pE1, pE2->p_ಬಲ, iTab) )
  ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಯದಿ( pE2->op==TK_NOTNULL
   && ಅಭಿವ್ಯಕ್ತಿ_Implies_Not_Null(pParse, pE1, pE2->p_ಎಡ, iTab, 0)
  ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_IIF(pParse->db, pE1) ){
    ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_ಅಭಿವ್ಯಕ್ತಿ(pParse,pE1->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,pE2,iTab);
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ಸ್ಥಿರ ಶೂನ್ಯ bothImplyNotNullRow(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *pE1, ಅಭಿವ್ಯಕ್ತಿ *pE2){
  ಯದಿ( pWalker->eCode==0 ){
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pE1);
    ಯದಿ( pWalker->eCode ){
      pWalker->eCode = 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ impliesNotNullRow(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_COLUMN );
  testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ) ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
  ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಪಕ್ಷ TK_ISNOT:
    ಪಕ್ಷ TK_ISNULL:
    ಪಕ್ಷ TK_NOTNULL:
    ಪಕ್ಷ TK_IS:
    ಪಕ್ಷ TK_VECTOR:
    ಪಕ್ಷ TK_FUNCTION:
    ಪಕ್ಷ TK_TRUTH:
    ಪಕ್ಷ TK_CASE:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNULL );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_VECTOR );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRUTH );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CASE );
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;

    ಪಕ್ಷ TK_COLUMN:
      ಯದಿ( pWalker->u.iCur==p_ಅಭಿವ್ಯಕ್ತಿ->iTable ){
        pWalker->eCode = 1;
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;

    ಪಕ್ಷ TK_OR:
    ಪಕ್ಷ TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_OR );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;
       
    ಪಕ್ಷ TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) && ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ>0) ){
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      }
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;

    ಪಕ್ಷ TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 );
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      bothImplyNotNullRow(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,
                                   p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ);
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    ಪಕ್ಷ TK_EQ:
    ಪಕ್ಷ TK_NE:
    ಪಕ್ಷ TK_LT:
    ಪಕ್ಷ TK_LE:
    ಪಕ್ಷ TK_GT:
    ಪಕ್ಷ TK_GE: {
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
      ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op!=TK_COLUMN || ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಎಡ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಬಲ->op!=TK_COLUMN || ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಬಲ) );
      ಯದಿ( (p_ಎಡ->op==TK_COLUMN
           && ALWAYS(p_ಎಡ->y.pTab!=0)
           && IsVirtual(p_ಎಡ->y.pTab))
       || (p_ಬಲ->op==TK_COLUMN
           && ALWAYS(p_ಬಲ->y.pTab!=0)
           && IsVirtual(p_ಬಲ->y.pTab))
      ){
        ಪ್ರತ್ಯಾಗಮ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_Non_Null_Row(ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ iTab, ಪೂರ್ಣಾಙ್ಕ isRJ){
  Walker w;
  p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p->op==TK_NOTNULL ){
    p = p->p_ಎಡ;
  }ಅನ್ಯಥಾ{
    ಯಾವತ್( p->op==TK_AND ){
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_Non_Null_Row(p->p_ಎಡ, iTab, isRJ) ) ಪ್ರತ್ಯಾಗಮ 1;
      p = p->p_ಬಲ;
    }
  }
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಪ್ರತ್ಯಾಗಮ w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
ಸಂರಚನಾ IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  ಪೂರ್ಣಾಙ್ಕ iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Idx_Cover(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN
   && p_ಅಭಿವ್ಯಕ್ತಿ->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn)<0
  ){
    pWalker->eCode = 1;
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Covered_By_Index(
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,        /* The index to be tested */
  ಪೂರ್ಣಾಙ್ಕ iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  ಸಂರಚನಾ IdxCover xcov;
  memset(&w, 0, ಪರಿಮಾಣ(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಪ್ರತ್ಯಾಗಮ !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
ಸಂರಚನಾ RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  ಪೂರ್ಣಾಙ್ಕ *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ selectRefEnter(Walker *pWalker, Select *pSelect){
  ಸಂರಚನಾ RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  ಪೂರ್ಣಾಙ್ಕ *piNew;
  ಯದಿ( pSrc->nSrc==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
  ಯದಿ( piNew==0 ){
    p->nExclude = 0;
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }ಅನ್ಯಥಾ{
    p->aiExclude = piNew;
  }
  ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}
ಸ್ಥಿರ ಶೂನ್ಯ selectRefLeave(Walker *pWalker, Select *pSelect){
  ಸಂರಚನಾ RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  ಯದಿ( p->nExclude ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Ref_To_Src_List(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN
   || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_COLUMN
  ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಸಂರಚನಾ RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    ಪೂರ್ಣಾಙ್ಕ nSrc = pSrc ? pSrc->nSrc : 0;
    ಪ್ರತಿ(i=0; i<nSrc; i++){
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        ಪ್ರತ್ಯಾಗಮ WRC_Continue;
      }
    }
    ಪ್ರತಿ(i=0; i<p->nExclude && p->aiExclude[i]!=p_ಅಭಿವ್ಯಕ್ತಿ->iTable; i++){}
    ಯದಿ( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3ReferencesSrcList(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, SrcList *pSrcList){
  Walker w;
  ಸಂರಚನಾ RefSrcList x;
  ಪ್ರತಿಜ್ಞಾ( pParse->db!=0 );
  memset(&w, 0, ಪರಿಮಾಣ(w));
  memset(&x, 0, ಪರಿಮಾಣ(x));
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList);
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ORDER );
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList!=0 );
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->pFilter);
  }
#endif
  ಯದಿ( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  ಯದಿ( w.eCode & 0x01 ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ ಯದಿ( w.eCode ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ -1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ agginfo_Persist_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( ALWAYS(!ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced))
   && p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
    ಪೂರ್ಣಾಙ್ಕ iAgg = p_ಅಭಿವ್ಯಕ್ತಿ->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    ಪ್ರತಿಜ್ಞಾ( iAgg>=0 );
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AGG_FUNCTION ){
      ಯದಿ( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_ಅಭಿವ್ಯಕ್ತಿ
      ){
        p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ){
          pAggInfo->aCol[iAgg].pCExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
        }
      }
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
      ಯದಿ( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_ಅಭಿವ್ಯಕ್ತಿ
      ){
        p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
        }
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
ಶೂನ್ಯ sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, ಪರಿಮಾಣ(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_ಅಭಿವ್ಯಕ್ತಿ_Callback = agginfo_Persist_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  ಪೂರ್ಣಾಙ್ಕ i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       ಪರಿಮಾಣ(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  ಪ್ರತ್ಯಾಗಮ i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  ಪೂರ್ಣಾಙ್ಕ i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       ಪರಿಮಾಣ(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  ಪ್ರತ್ಯಾಗಮ i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
ಸ್ಥಿರ ಶೂನ್ಯ findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ          /* Expr describing the column to find or insert */
){
  ಸಂರಚನಾ AggInfo_col *pCol;
  ಪೂರ್ಣಾಙ್ಕ k;
  ಪೂರ್ಣಾಙ್ಕ mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  ಪ್ರತಿಜ್ಞಾ( mxTerm <= SMXV(i16) );
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  ಪ್ರತಿ(k=0; k<pAggInfo->nColumn; k++, pCol++){
    ಯದಿ( pCol->pCExpr==p_ಅಭಿವ್ಯಕ್ತಿ ) ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ( pCol->iTable==p_ಅಭಿವ್ಯಕ್ತಿ->iTable
     && pCol->iColumn==p_ಅಭಿವ್ಯಕ್ತಿ->iColumn
     && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_IF_NULL_ROW
    ){
      ಗಚ್ಛ fix_up_ಅಭಿವ್ಯಕ್ತಿ;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  ಯದಿ( k<0 ){
    /* OOM on resize */
    ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed );
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( k>mxTerm ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pCol->pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
  pCol->iTable = p_ಅಭಿವ್ಯಕ್ತಿ->iTable;
  pCol->iColumn = p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಯದಿ( pAggInfo->pGroupBy && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_IF_NULL_ROW ){
    ಪೂರ್ಣಾಙ್ಕ j, n;
    ಅಭಿವ್ಯಕ್ತಿ_List *pGB = pAggInfo->pGroupBy;
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pTerm = pGB->a;
    n = pGB->n_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿ(j=0; j<n; j++, pTerm++){
      ಅಭಿವ್ಯಕ್ತಿ *pE = pTerm->p_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( pE->op==TK_COLUMN
       && pE->iTable==p_ಅಭಿವ್ಯಕ್ತಿ->iTable
       && pE->iColumn==p_ಅಭಿವ್ಯಕ್ತಿ->iColumn
      ){
        pCol->iSorterColumn = j;
        ಭಙ್ಗ;
      }
    }
  }
  ಯದಿ( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_ಅಭಿವ್ಯಕ್ತಿ:
  ಅಭಿವ್ಯಕ್ತಿ_Set_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_NoReduce);
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==0 || p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==pAggInfo );
  p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo = pAggInfo;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN ){
    p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_AGG_COLUMN;
  }
  ಪ್ರತಿಜ್ಞಾ( k <= SMXV(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg) );
  p_ಅಭಿವ್ಯಕ್ತಿ->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ analyzeAggregate(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  ಪ್ರತಿಜ್ಞಾ( pNC->ncFlags & NC_UAggInfo );
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->iFirstReg==0 );
  ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಸ್ವಭಾವ: {
      Indexed_ಅಭಿವ್ಯಕ್ತಿ *pIEpr;
      ಅಭಿವ್ಯಕ್ತಿ tmp;
      ಪ್ರತಿಜ್ಞಾ( pParse->iSelfTab==0 );
      ಯದಿ( (pNC->ncFlags & NC_InAggFunc)==0 ) ಭಙ್ಗ;
      ಯದಿ( pParse->pIdxEpr==0 ) ಭಙ್ಗ;
      ಪ್ರತಿ(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        ಪೂರ್ಣಾಙ್ಕ iDataCur = pIEpr->iDataCur;
        ಯದಿ( iDataCur<0 ) ಅನುವರ್ತ;
        ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ, pIEpr->p_ಅಭಿವ್ಯಕ್ತಿ, iDataCur)==0 ) ಭಙ್ಗ;
      }
      ಯದಿ( pIEpr==0 ) ಭಙ್ಗ;
      ಯದಿ( NEVER(!ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ)) ) ಭಙ್ಗ;
      ಪ್ರತಿ(i=0; i<pSrcList->nSrc; i++){
         ಯದಿ( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           ಭಙ್ಗ;
         }
      }
      ಯದಿ( i>=pSrcList->nSrc ) ಭಙ್ಗ;
      ಯದಿ( NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo!=0) ) ಭಙ್ಗ; /* Resolved by outer context */
      ಯದಿ( pParse->nErr ){ ಪ್ರತ್ಯಾಗಮ WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, ಪರಿಮಾಣ(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      ಯದಿ( pParse->nErr ){ ಪ್ರತ್ಯಾಗಮ WRC_Abort; }
      ಪ್ರತಿಜ್ಞಾ( pAggInfo->aCol!=0 );
      ಪ್ರತಿಜ್ಞಾ( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
      p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo = pAggInfo;
      p_ಅಭಿವ್ಯಕ್ತಿ->iAgg = tmp.iAgg;
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;
    }
    ಪಕ್ಷ TK_IF_NULL_ROW:
    ಪಕ್ಷ TK_AGG_COLUMN:
    ಪಕ್ಷ TK_COLUMN: {
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_COLUMN );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      ಯದಿ( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        ಪ್ರತಿ(i=0; i<pSrcList->nSrc; i++, pItem++){
          ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced) );
          ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_ಅಭಿವ್ಯಕ್ತಿ);
            ಭಙ್ಗ;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      ಪ್ರತ್ಯಾಗಮ WRC_Continue;
    }
    ಪಕ್ಷ TK_AGG_FUNCTION: {
      ಯದಿ( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_ಅಭಿವ್ಯಕ್ತಿ->op2
       && p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        ಸಂರಚನಾ AggInfo_func *pItem = pAggInfo->aFunc;
        ಪೂರ್ಣಾಙ್ಕ mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        ಪ್ರತಿಜ್ಞಾ( mxTerm <= SMXV(i16) );
        ಪ್ರತಿ(i=0; i<pAggInfo->nFunc; i++, pItem++){
          ಯದಿ( NEVER(pItem->pFExpr==p_ಅಭಿವ್ಯಕ್ತಿ) ) ಭಙ್ಗ;
          ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, pItem->pFExpr, p_ಅಭಿವ್ಯಕ್ತಿ, -1)==0 ){
            ಭಙ್ಗ;
          }
        }
        ಯದಿ( i>mxTerm ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          ಪ್ರತಿಜ್ಞಾ( i<pAggInfo->nFunc );
        }ಅನ್ಯಥಾ ಯದಿ( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          ಯದಿ( i>=0 ){
            ಪೂರ್ಣಾಙ್ಕ nArg;
            ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
            ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(p_ಅಭಿವ್ಯಕ್ತಿ) );
            nArg = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList ? p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, nArg, enc, 0);
            ಪ್ರತಿಜ್ಞಾ( pItem->bOBUnique==0 );
            ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              ಅಭಿವ್ಯಕ್ತಿ_List *pOBList;
              ಪ್ರತಿಜ್ಞಾ( nArg>0 );
              ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ORDER );
              ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList;
              ಪ್ರತಿಜ್ಞಾ( pOBList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
              ಪ್ರತಿಜ್ಞಾ( pItem->bOBUnique==0 );
              ಯದಿ( pOBList->n_ಅಭಿವ್ಯಕ್ತಿ==1
               && nArg==1
               && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pOBList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,
                               p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Distinct);
              }ಅನ್ಯಥಾ{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }ಅನ್ಯಥಾ{
              pItem->iOBTab = -1;
            }
            ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }ಅನ್ಯಥಾ{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced) );
        ಅಭಿವ್ಯಕ್ತಿ_Set_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_NoReduce);
        ಪ್ರತಿಜ್ಞಾ( i <= SMXV(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg) );
        p_ಅಭಿವ್ಯಕ್ತಿ->iAgg = (i16)i;
        p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo = pAggInfo;
        ಪ್ರತ್ಯಾಗಮ WRC_Prune;
      }ಅನ್ಯಥಾ{
        ಪ್ರತ್ಯಾಗಮ WRC_Continue;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
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
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( pList ){
    ಪ್ರತಿ(pItem=pList->a, i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++, pItem++){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Aggregates(pNC, pItem->p_ಅಭಿವ್ಯಕ್ತಿ);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3GetTempReg(Parse *pParse){
  ಯದಿ( pParse->nTempReg==0 ){
    ಪ್ರತ್ಯಾಗಮ ++pParse->nMem;
  }
  ಪ್ರತ್ಯಾಗಮ pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
ಶೂನ್ಯ sqlite3ReleaseTempReg(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iReg){
  ಯದಿ( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    ಪೂರ್ಣಾಙ್ಕ kk;
    ಪ್ರತಿ(kk=0; kk<pParse->nTempReg; kk++){
      ಪ್ರತಿಜ್ಞಾ( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    ಯದಿ( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3GetTempRange(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ nReg){
  ಪೂರ್ಣಾಙ್ಕ i, n;
  ಯದಿ( nReg==1 ) ಪ್ರತ್ಯಾಗಮ sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  ಯದಿ( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }ಅನ್ಯಥಾ{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  ಪ್ರತ್ಯಾಗಮ i;
}
ಶೂನ್ಯ sqlite3ReleaseTempRange(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iReg, ಪೂರ್ಣಾಙ್ಕ nReg){
  ಯದಿ( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    ಪ್ರತ್ಯಾಗಮ;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  ಯದಿ( nReg>pParse->nRangeReg ){
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
ಶೂನ್ಯ sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
ಶೂನ್ಯ sqlite3TouchRegister(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iReg){
  ಯದಿ( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#ಯದಿ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3FirstAvailableRegister(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iMin){
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pList = pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ;
  ಯದಿ( pList ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಯದಿ( pList->a[i].u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  ಪ್ರತ್ಯಾಗಮ iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
ಪೂರ್ಣಾಙ್ಕ sqlite3NoTempsInRange(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iFirst, ಪೂರ್ಣಾಙ್ಕ iLast){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿ(i=0; i<pParse->nTempReg; i++){
    ಯದಿ( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  }
  ಯದಿ( pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಪೂರ್ಣಾಙ್ಕ iReg = pList->a[i].u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg;
      ಯದಿ( iReg==0 ) ಅನುವರ್ತ;
      ಯದಿ( iReg>=iFirst && iReg<=iLast ) ಪ್ರತ್ಯಾಗಮ 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}
#endif /* SQLITE_DEBUG */
