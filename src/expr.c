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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/* Forward declarations */
ਸਥਿਰ ਖਾਲੀ ਵਿਅੰਜਕ_ਕੋਡ_Between(Parse*,ਵਿਅੰਜਕ*,ਪੂਰਨ,ਖਾਲੀ(*)(Parse*,ਵਿਅੰਜਕ*,ਪੂਰਨ,ਪੂਰਨ),ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਕੋਡ_ਸਦਿਸ਼(Parse *pParse, ਵਿਅੰਜਕ *p, ਪੂਰਨ *pi_ਮੰਜ਼ਿਲ_Free);

/*
** Return the affinity character for a single column of a table.
*/
ਅੱਖਰ sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_Affinity(ਅਚਲ ਸਾਰਣੀ *pTab, ਪੂਰਨ i_ਖਾਨਾ){
  ਜੇ( i_ਖਾਨਾ<0 || NEVER(i_ਖਾਨਾ>=pTab->n_ਖਾਨਾ) ) ਵਾਪਸ SQLITE_AFF_INTEGER;
  ਵਾਪਸ pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].affinity;
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
ਅੱਖਰ sqlite3_ਵਿਅੰਜਕ_Affinity(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ op;
  op = p_ਵਿਅੰਜਕ->op;
  ਜਦੋਂ( 1 /* exit-by-break */ ){
    ਜੇ( op==TK_ਕਾਲਮ || (op==TK_AGG_ਕਾਲਮ && p_ਵਿਅੰਜਕ->y.pTab!=0) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->y.pTab!=0 );
      ਵਾਪਸ sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_Affinity(p_ਵਿਅੰਜਕ->y.pTab, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
    }
    ਜੇ( op==TK_SELECT ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pSelect!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pSelect->pEList!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pSelect->pEList->a[0].p_ਵਿਅੰਜਕ!=0 );
      ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ->x.pSelect->pEList->a[0].p_ਵਿਅੰਜਕ);
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CAST
    ਜੇ( op==TK_CAST ){
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      ਵਾਪਸ sqlite3AffinityType(p_ਵਿਅੰਜਕ->u.zToken, 0);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( op==TK_SELECT_ਕਾਲਮ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ!=0 && ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ < p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ >= 0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pSelect->pEList->n_ਵਿਅੰਜਕ );
      ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_Affinity(
          p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pSelect->pEList->a[p_ਵਿਅੰਜਕ->i_ਕਾਲਮ].p_ਵਿਅੰਜਕ
      );
    }
    ਜੇ( op==TK_ਸਦਿਸ਼
     || (op==TK_FUNCTION && p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ==SQLITE_AFF_DEFER)
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
      ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ);
    }
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Skip|EP_If_Null_ਪੰਕਤੀ) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_COLLATE
           || p_ਵਿਅੰਜਕ->op==TK_IF_NULL_ਪੰਕਤੀ
           || (p_ਵਿਅੰਜਕ->op==TK_ਰਜਿਸਟਰ && p_ਵਿਅੰਜਕ->op2==TK_IF_NULL_ਪੰਕਤੀ) );
      p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
      op = p_ਵਿਅੰਜਕ->op;
      ਜਾਰੀ;
    }
    ਜੇ( op!=TK_ਰਜਿਸਟਰ ) ਰੋਕੋ;
    op = p_ਵਿਅੰਜਕ->op2;
    ਜੇ( NEVER( op==TK_ਰਜਿਸਟਰ ) ) ਰੋਕੋ;
  }
  ਵਾਪਸ p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਅੰਕੜੇ_Type(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜਦੋਂ( p_ਵਿਅੰਜਕ ){
    ਚੋਣ( p_ਵਿਅੰਜਕ->op ){
      ਹਾਲਤ TK_COLLATE:
      ਹਾਲਤ TK_IF_NULL_ਪੰਕਤੀ:
      ਹਾਲਤ TK_UPLUS:  {
        p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_NULL: {
        p_ਵਿਅੰਜਕ = 0;
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_STRING: {
        ਵਾਪਸ 0x02;
      }
      ਹਾਲਤ TK_BLOB: {
        ਵਾਪਸ 0x04;
      }
      ਹਾਲਤ TK_CONCAT: {
        ਵਾਪਸ 0x06;
      }
      ਹਾਲਤ TK_VARIABLE:
      ਹਾਲਤ TK_AGG_FUNCTION:
      ਹਾਲਤ TK_FUNCTION: {
        ਵਾਪਸ 0x07;
      }
      ਹਾਲਤ TK_ਕਾਲਮ:
      ਹਾਲਤ TK_AGG_ਕਾਲਮ:
      ਹਾਲਤ TK_SELECT:
      ਹਾਲਤ TK_CAST:
      ਹਾਲਤ TK_SELECT_ਕਾਲਮ:
      ਹਾਲਤ TK_ਸਦਿਸ਼:  {
        ਪੂਰਨ aff = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ);
        ਜੇ( aff>=SQLITE_AFF_NUMERIC ) ਵਾਪਸ 0x05;
        ਜੇ( aff==SQLITE_AFF_ਪਾਠ )    ਵਾਪਸ 0x06;
        ਵਾਪਸ 0x07;
      }
      ਹਾਲਤ TK_CASE: {
        ਪੂਰਨ res = 0;
        ਪੂਰਨ ii;
        ਵਿਅੰਜਕ_List *pList = p_ਵਿਅੰਜਕ->x.pList;
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) && pList!=0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( pList->n_ਵਿਅੰਜਕ > 0);
        ਲਈ(ii=1; ii<pList->n_ਵਿਅੰਜਕ; ii+=2){
          res |= sqlite3_ਵਿਅੰਜਕ_ਅੰਕੜੇ_Type(pList->a[ii].p_ਵਿਅੰਜਕ);
        }
        ਜੇ( pList->n_ਵਿਅੰਜਕ % 2 ){
          res |= sqlite3_ਵਿਅੰਜਕ_ਅੰਕੜੇ_Type(pList->a[pList->n_ਵਿਅੰਜਕ-1].p_ਵਿਅੰਜਕ);
        }
        ਵਾਪਸ res;
      }
      ਮੂਲ: {
        ਵਾਪਸ 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  ਵਾਪਸ 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_Token(
  Parse *pParse,           /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,             /* Add the "COLLATE" clause to this expression */
  ਅਚਲ Token *pCollName,  /* Name of collating sequence */
  ਪੂਰਨ dequote              /* True to dequote pCollName */
){
  ਜੇ( pCollName->n>0 && pParse->nErr==0 ){
    ਵਿਅੰਜਕ *pNew = sqlite3_ਵਿਅੰਜਕ_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
    ਜੇ( pNew ){
      pNew->p_ਖੱਬਾ = p_ਵਿਅੰਜਕ;
      pNew->flags |= EP_Collate|EP_Skip;
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
      pNew->n_ਉਚਾਈ = p_ਵਿਅੰਜਕ->n_ਉਚਾਈ+1;
      sqlite3_ਵਿਅੰਜਕ_Check_ਉਚਾਈ(pParse, pNew->n_ਉਚਾਈ);
      testcase( pParse->nErr>0 );
#ਜੇ_ਅੰਤ
      p_ਵਿਅੰਜਕ = pNew;
    }
  }
  ਵਾਪਸ p_ਵਿਅੰਜਕ;
}
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(
  Parse *pParse,        /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,          /* Add the "COLLATE" clause to this expression */
  ਅਚਲ ਅੱਖਰ *zC        /* The collating sequence name */
){
  Token s;
  ਪ੍ਰਤਿਜ੍ਞਾ( zC!=0 );
  sqlite3TokenInit(&s, (ਅੱਖਰ*)zC);
  ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_Token(pParse, p_ਵਿਅੰਜਕ, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Skip_Collate(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜਦੋਂ( p_ਵਿਅੰਜਕ && ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Skip) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_COLLATE );
    p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
  }  
  ਵਾਪਸ p_ਵਿਅੰਜਕ;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜਦੋਂ( p_ਵਿਅੰਜਕ && ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Skip|EP_Unlikely) ){
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Unlikely) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ>0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION );
      p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ;
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_COLLATE ){
      p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
    }ਨਹੀਂਤਾਂ{
      ਰੋਕੋ;
    }
  }  
  ਵਾਪਸ p_ਵਿਅੰਜਕ;
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
CollSeq *sqlite3_ਵਿਅੰਜਕ_Coll_Seq(Parse *pParse, ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ਅਚਲ ਵਿਅੰਜਕ *p = p_ਵਿਅੰਜਕ;
  ਜਦੋਂ( p ){
    ਪੂਰਨ op = p->op;
    ਜੇ( op==TK_ਰਜਿਸਟਰ ) op = p->op2;
    ਜੇ( (op==TK_AGG_ਕਾਲਮ && p->y.pTab!=0)
     || op==TK_ਕਾਲਮ || op==TK_TRIGGER
    ){
      ਪੂਰਨ j;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p->y.pTab!=0 );
      ਜੇ( (j = p->i_ਕਾਲਮ)>=0 ){
        ਅਚਲ ਅੱਖਰ *zColl = sqlite3_ਕਾਲਮ_Coll(&p->y.pTab->a_ਖਾਨਾ[j]);
        pColl = sqlite3_ਲੱਭੋ_Coll_Seq(db, ENC(db), zColl, 0);
      }
      ਰੋਕੋ;
    }
    ਜੇ( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_ਖੱਬਾ;
      ਜਾਰੀ;
    }
    ਜੇ( op==TK_ਸਦਿਸ਼
     || (op==TK_FUNCTION && p->aff_ਵਿਅੰਜਕ==SQLITE_AFF_DEFER)
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p) );
      p = p->x.pList->a[0].p_ਵਿਅੰਜਕ;
      ਜਾਰੀ;
    }
    ਜੇ( op==TK_COLLATE ){
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Int_ਮੁੱਲ) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      ਰੋਕੋ;
    }
    ਜੇ( p->flags & EP_Collate ){
      ਜੇ( p->p_ਖੱਬਾ && (p->p_ਖੱਬਾ->flags & EP_Collate)!=0 ){
        p = p->p_ਖੱਬਾ;
      }ਨਹੀਂਤਾਂ{
        ਵਿਅੰਜਕ *p_ਅਗਲਾ  = p->p_ਸੱਜਾ;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_XList(p) || p->x.pList==0 || p->p_ਸੱਜਾ==0 );
        ਜੇ( ਵਿਅੰਜਕ_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          ਪੂਰਨ i;
          ਲਈ(i=0; i<p->x.pList->n_ਵਿਅੰਜਕ; i++){
            ਜੇ( ਵਿਅੰਜਕ_Has_Property(p->x.pList->a[i].p_ਵਿਅੰਜਕ, EP_Collate) ){
              p_ਅਗਲਾ = p->x.pList->a[i].p_ਵਿਅੰਜਕ;
              ਰੋਕੋ;
            }
          }
        }
        p = p_ਅਗਲਾ;
      }
    }ਨਹੀਂਤਾਂ{
      ਰੋਕੋ;
    }
  }
  ਜੇ( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  ਵਾਪਸ pColl;
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
CollSeq *sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(Parse *pParse, ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  CollSeq *p = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਵਿਅੰਜਕ);
  ਜੇ( p==0 ) p = pParse->db->pDfltColl;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਵਾਪਸ p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Coll_Seq_ਮੇਲ(Parse *pParse, ਅਚਲ ਵਿਅੰਜਕ *pE1, ਅਚਲ ਵਿਅੰਜਕ *pE2){
  CollSeq *pColl1 = sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(pParse, pE2);
  ਪ੍ਰਤਿਜ੍ਞਾ( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  ਵਾਪਸ pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
ਅੱਖਰ sqlite3CompareAffinity(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਅੱਖਰ aff2){
  ਅੱਖਰ aff1 = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ);
  ਜੇ( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    ਜੇ( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      ਵਾਪਸ SQLITE_AFF_NUMERIC;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ SQLITE_AFF_BLOB;
    }
  }ਨਹੀਂਤਾਂ{
    /* One side is a column, the other is not. Use the columns affinity. */
    ਪ੍ਰਤਿਜ੍ਞਾ( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    ਵਾਪਸ (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
ਸਥਿਰ ਅੱਖਰ comparisonAffinity(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਅੱਖਰ aff;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_EQ || p_ਵਿਅੰਜਕ->op==TK_ਵਿੱਚ || p_ਵਿਅੰਜਕ->op==TK_LT ||
          p_ਵਿਅੰਜਕ->op==TK_GT || p_ਵਿਅੰਜਕ->op==TK_GE || p_ਵਿਅੰਜਕ->op==TK_LE ||
          p_ਵਿਅੰਜਕ->op==TK_NE || p_ਵਿਅੰਜਕ->op==TK_IS || p_ਵਿਅੰਜਕ->op==TK_ISNOT );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
  aff = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
  ਜੇ( p_ਵਿਅੰਜਕ->p_ਸੱਜਾ ){
    aff = sqlite3CompareAffinity(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, aff);
  }ਨਹੀਂਤਾਂ ਜੇ( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  ਵਾਪਸ aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
ਪੂਰਨ sqlite3_ਸੂਚਕਾਂਕ_Affinity_Ok(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਅੱਖਰ idx_affinity){
  ਅੱਖਰ aff = comparisonAffinity(p_ਵਿਅੰਜਕ);
  ਜੇ( aff<SQLITE_AFF_ਪਾਠ ){
    ਵਾਪਸ 1;
  }
  ਜੇ( aff==SQLITE_AFF_ਪਾਠ ){
    ਵਾਪਸ idx_affinity==SQLITE_AFF_ਪਾਠ;
  }
  ਵਾਪਸ sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ਸਥਿਰ u8 ਦੁਆਧਾਰੀ_Compare_P5(
  ਅਚਲ ਵਿਅੰਜਕ *pExpr1,   /* Left operand */
  ਅਚਲ ਵਿਅੰਜਕ *pExpr2,   /* Right operand */
  ਪੂਰਨ jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (ਅੱਖਰ)sqlite3_ਵਿਅੰਜਕ_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  ਵਾਪਸ aff;
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
CollSeq *sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(
  Parse *pParse,
  ਅਚਲ ਵਿਅੰਜਕ *p_ਖੱਬਾ,
  ਅਚਲ ਵਿਅੰਜਕ *p_ਸੱਜਾ
){
  CollSeq *pColl;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ );
  ਜੇ( p_ਖੱਬਾ->flags & EP_Collate ){
    pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਖੱਬਾ);
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਸੱਜਾ && (p_ਸੱਜਾ->flags & EP_Collate)!=0 ){
    pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਸੱਜਾ);
  }ਨਹੀਂਤਾਂ{
    pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਖੱਬਾ);
    ਜੇ( !pColl ){
      pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਸੱਜਾ);
    }
  }
  ਵਾਪਸ pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(Parse *pParse, ਅਚਲ ਵਿਅੰਜਕ *p){
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p, EP_Commuted) ){
    ਵਾਪਸ sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, p->p_ਸੱਜਾ, p->p_ਖੱਬਾ);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, p->p_ਖੱਬਾ, p->p_ਸੱਜਾ);
  }
}

/*
** Generate code for a comparison operator.
*/
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_Compare(
  Parse *pParse,    /* The parsing (and code generating) context */
  ਵਿਅੰਜਕ *p_ਖੱਬਾ,      /* The left operand */
  ਵਿਅੰਜਕ *p_ਸੱਜਾ,     /* The right operand */
  ਪੂਰਨ opcode,       /* The comparison opcode */
  ਪੂਰਨ in1, ਪੂਰਨ in2, /* Register holding operands */
  ਪੂਰਨ dest,         /* Jump here if true.  */
  ਪੂਰਨ jumpIfNull,   /* If true, jump if either operand is NULL */
  ਪੂਰਨ isCommuted    /* The comparison has been commuted */
){
  ਪੂਰਨ p5;
  ਪੂਰਨ addr;
  CollSeq *p4;

  ਜੇ( pParse->nErr ) ਵਾਪਸ 0;
  ਜੇ( isCommuted ){
    p4 = sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, p_ਸੱਜਾ, p_ਖੱਬਾ);
  }ਨਹੀਂਤਾਂ{
    p4 = sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, p_ਖੱਬਾ, p_ਸੱਜਾ);
  }
  p5 = ਦੁਆਧਾਰੀ_Compare_P5(p_ਖੱਬਾ, p_ਸੱਜਾ, jumpIfNull);
  addr = sqlite3_Vdbe_ਜੋੜੋ_Op4(pParse->pVdbe, opcode, in2, dest, in1,
                           (ਖਾਲੀ*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  ਵਾਪਸ addr;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  u8 op = p_ਵਿਅੰਜਕ->op;
  ਜੇ( op==TK_ਰਜਿਸਟਰ ) op = p_ਵਿਅੰਜਕ->op2;
  ਜੇ( op==TK_ਸਦਿਸ਼ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
    ਵਾਪਸ p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ;
  }ਨਹੀਂਤਾਂ ਜੇ( op==TK_SELECT ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
    ਵਾਪਸ p_ਵਿਅੰਜਕ->x.pSelect->pEList->n_ਵਿਅੰਜਕ;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 1;
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
ਵਿਅੰਜਕ *sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(ਵਿਅੰਜਕ *p_ਸਦਿਸ਼, ਪੂਰਨ i){
  ਪ੍ਰਤਿਜ੍ਞਾ( i<sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਸਦਿਸ਼) || p_ਸਦਿਸ਼->op==TK_ERROR );
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(p_ਸਦਿਸ਼) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਦਿਸ਼->op2==0 || p_ਸਦਿਸ਼->op==TK_ਰਜਿਸਟਰ );
    ਜੇ( p_ਸਦਿਸ਼->op==TK_SELECT || p_ਸਦਿਸ਼->op2==TK_SELECT ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਸਦਿਸ਼) );
      ਵਾਪਸ p_ਸਦਿਸ਼->x.pSelect->pEList->a[i].p_ਵਿਅੰਜਕ;
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਸਦਿਸ਼) );
      ਵਾਪਸ p_ਸਦਿਸ਼->x.pList->a[i].p_ਵਿਅੰਜਕ;
    }
  }
  ਵਾਪਸ p_ਸਦਿਸ਼;
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
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_For_ਸਦਿਸ਼_ਖੇਤਰ(
  Parse *pParse,       /* Parsing context */
  ਵਿਅੰਜਕ *p_ਸਦਿਸ਼,       /* The vector.  List of expressions or a sub-SELECT */
  ਪੂਰਨ i_ਖੇਤਰ,          /* Which column of the vector to return */
  ਪੂਰਨ n_ਖੇਤਰ           /* Total number of columns in the vector */
){
  ਵਿਅੰਜਕ *pRet;
  ਜੇ( p_ਸਦਿਸ਼->op==TK_SELECT ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਸਦਿਸ਼) );
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
    pRet = sqlite3PExpr(pParse, TK_SELECT_ਕਾਲਮ, 0, 0);
    ਜੇ( pRet ){
      ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pRet, EP_ਭਰਿਆ_ਮਾਪ);
      pRet->i_ਸਾਰਣੀ = n_ਖੇਤਰ;
      pRet->i_ਕਾਲਮ = i_ਖੇਤਰ;
      pRet->p_ਖੱਬਾ = p_ਸਦਿਸ਼;
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( p_ਸਦਿਸ਼->op==TK_ਸਦਿਸ਼ ){
      ਵਿਅੰਜਕ **pp_ਸਦਿਸ਼;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਸਦਿਸ਼) );
      pp_ਸਦਿਸ਼ = &p_ਸਦਿਸ਼->x.pList->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ;
      p_ਸਦਿਸ਼ = *pp_ਸਦਿਸ਼;
      ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *pp_ਸਦਿਸ਼ = 0;
        ਵਾਪਸ p_ਸਦਿਸ਼;
      }
    }
    pRet = sqlite3_ਵਿਅੰਜਕ_Dup(pParse->db, p_ਸਦਿਸ਼, 0);
  }
  ਵਾਪਸ pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਕੋਡ_Subselect(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ reg = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_SELECT ){
    reg = sqlite3_ਕੋਡ_Subselect(pParse, p_ਵਿਅੰਜਕ);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ reg;
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
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਰਜਿਸਟਰ(
  Parse *pParse,                  /* Parse context */
  ਵਿਅੰਜਕ *p_ਸਦਿਸ਼,                  /* Vector to extract element from */
  ਪੂਰਨ i_ਖੇਤਰ,                     /* Field to extract from pVector */
  ਪੂਰਨ regSelect,                  /* First in array of registers */
  ਵਿਅੰਜਕ *pTmp,                     /* Temporary space */
  ਵਿਅੰਜਕ **pp_ਵਿਅੰਜਕ,                  /* OUT: Expression element */
  ਪੂਰਨ *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = p_ਸਦਿਸ਼->op;
  ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_ਸਦਿਸ਼ || op==TK_ਰਜਿਸਟਰ || op==TK_SELECT || op==TK_ERROR );
  ਜੇ( op==TK_ਰਜਿਸਟਰ ){
    *pp_ਵਿਅੰਜਕ = sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(p_ਸਦਿਸ਼, i_ਖੇਤਰ);
    ਵਾਪਸ p_ਸਦਿਸ਼->i_ਸਾਰਣੀ+i_ਖੇਤਰ;
  }
  ਜੇ( op==TK_SELECT ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਸਦਿਸ਼) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, ਆਕਾਰ(ਵਿਅੰਜਕ));
    pTmp->op = TK_SELECT_ਕਾਲਮ;
    pTmp->p_ਖੱਬਾ = p_ਸਦਿਸ਼;
    pTmp->i_ਕਾਲਮ = i_ਖੇਤਰ;
    pTmp->i_ਸਾਰਣੀ = p_ਸਦਿਸ਼->x.pSelect->pEList->n_ਵਿਅੰਜਕ;
    *pp_ਵਿਅੰਜਕ = pTmp;
    ਵਾਪਸ regSelect+i_ਖੇਤਰ;
  }
  ਜੇ( op==TK_ਸਦਿਸ਼ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਸਦਿਸ਼) );
    *pp_ਵਿਅੰਜਕ = p_ਸਦਿਸ਼->x.pList->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ;
    ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, *pp_ਵਿਅੰਜਕ, pRegFree);
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_ਸਦਿਸ਼_Compare(
  Parse *pParse,        /* Code generator context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,          /* The comparison operation */
  ਪੂਰਨ dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
  ਵਿਅੰਜਕ *p_ਸੱਜਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
  ਪੂਰਨ n_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਖੱਬਾ);
  ਪੂਰਨ i;
  ਪੂਰਨ reg_ਖੱਬਾ = 0;
  ਪੂਰਨ reg_ਸੱਜਾ = 0;
  u8 opx = op;
  ਪੂਰਨ addrCmp = 0;
  ਪੂਰਨ addr_ਪੂਰਾ = sqlite3VdbeMakeLabel(pParse);
  ਪੂਰਨ isCommuted = ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_Commuted);

  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_VVAProperty(p_ਵਿਅੰਜਕ,EP_Immutable) );
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  ਜੇ( n_ਖੱਬਾ!=sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਸੱਜਾ) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "row value misused");
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_EQ || p_ਵਿਅੰਜਕ->op==TK_NE
       || p_ਵਿਅੰਜਕ->op==TK_IS || p_ਵਿਅੰਜਕ->op==TK_ISNOT
       || p_ਵਿਅੰਜਕ->op==TK_LT || p_ਵਿਅੰਜਕ->op==TK_GT
       || p_ਵਿਅੰਜਕ->op==TK_LE || p_ਵਿਅੰਜਕ->op==TK_GE
  );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==op || (p_ਵਿਅੰਜਕ->op==TK_IS && op==TK_EQ)
            || (p_ਵਿਅੰਜਕ->op==TK_ISNOT && op==TK_NE) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p5==0 || p_ਵਿਅੰਜਕ->op!=op );
  ਪ੍ਰਤਿਜ੍ਞਾ( p5==SQLITE_NULLEQ || p_ਵਿਅੰਜਕ->op==op );

  ਜੇ( op==TK_LE ) opx = TK_LT;
  ਜੇ( op==TK_GE ) opx = TK_GT;
  ਜੇ( op==TK_NE ) opx = TK_EQ;

  reg_ਖੱਬਾ = ਵਿਅੰਜਕ_ਕੋਡ_Subselect(pParse, p_ਖੱਬਾ);
  reg_ਸੱਜਾ = ਵਿਅੰਜਕ_ਕੋਡ_Subselect(pParse, p_ਸੱਜਾ);

  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, dest);
  ਲਈ(i=0; 1 /*Loop exits by "break"*/; i++){
    ਪੂਰਨ regFree1 = 0, regFree2 = 0;
    ਵਿਅੰਜਕ *pL = 0, *pR = 0;
    ਵਿਅੰਜਕ tmp1, tmp2;
    ਪੂਰਨ r1, r2;
    ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<n_ਖੱਬਾ );
    ਜੇ( addrCmp ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrCmp);
    r1 = ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਰਜਿਸਟਰ(pParse, p_ਖੱਬਾ, i, reg_ਖੱਬਾ, &tmp1, &pL, &regFree1);
    r2 = ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਰਜਿਸਟਰ(pParse, p_ਸੱਜਾ, i, reg_ਸੱਜਾ, &tmp2, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    ਕੋਡ_Compare(pParse, pL, pR, opx, r1, r2, addr_ਪੂਰਾ, p5, isCommuted);
    testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
    testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
    testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
    testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
    testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
    testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree1);
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree2);
    ਜੇ( (opx==TK_LT || opx==TK_GT) && i<n_ਖੱਬਾ-1 ){
      addrCmp = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    ਜੇ( p5==SQLITE_NULLEQ ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, dest);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    ਜੇ( i==n_ਖੱਬਾ-1 ){
      ਰੋਕੋ;
    }
    ਜੇ( opx==TK_EQ ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NotNull, dest, addr_ਪੂਰਾ); VdbeCoverage(v);
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, addr_ਪੂਰਾ);
      ਜੇ( i==n_ਖੱਬਾ-2 ) opx = op;
    }
  }
  sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addr_ਪੂਰਾ);
  ਜੇ( op==TK_NE ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Not, dest, dest);
  }
}

#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Check_ਉਚਾਈ(Parse *pParse, ਪੂਰਨ n_ਉਚਾਈ){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ mx_ਉਚਾਈ = pParse->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ];
  ਜੇ( n_ਉਚਾਈ>mx_ਉਚਾਈ ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse,
       "Expression tree is too large (maximum depth %d)", mx_ਉਚਾਈ
    );
    rc = SQLITE_ERROR;
  }
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਖਾਲੀ ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ(ਅਚਲ ਵਿਅੰਜਕ *p, ਪੂਰਨ *pn_ਉਚਾਈ){
  ਜੇ( p ){
    ਜੇ( p->n_ਉਚਾਈ>*pn_ਉਚਾਈ ){
      *pn_ਉਚਾਈ = p->n_ਉਚਾਈ;
    }
  }
}
ਸਥਿਰ ਖਾਲੀ ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ_List(ਅਚਲ ਵਿਅੰਜਕ_List *p, ਪੂਰਨ *pn_ਉਚਾਈ){
  ਜੇ( p ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<p->n_ਵਿਅੰਜਕ; i++){
      ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ(p->a[i].p_ਵਿਅੰਜਕ, pn_ਉਚਾਈ);
    }
  }
}
ਸਥਿਰ ਖਾਲੀ ਉਚਾਈ_ਦਾ_Select(ਅਚਲ Select *pSelect, ਪੂਰਨ *pn_ਉਚਾਈ){
  ਅਚਲ Select *p;
  ਲਈ(p=pSelect; p; p=p->p_ਪੂਰਵ){
    ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ(p->pWhere, pn_ਉਚਾਈ);
    ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ(p->pHaving, pn_ਉਚਾਈ);
    ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ(p->p_ਹੱਦਬੰਦੀ, pn_ਉਚਾਈ);
    ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ_List(p->pEList, pn_ਉਚਾਈ);
    ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ_List(p->p_ਸਮੂਹ_By, pn_ਉਚਾਈ);
    ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ_List(p->p_ਕ੍ਰਮ_By, pn_ਉਚਾਈ);
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
ਸਥਿਰ ਖਾਲੀ ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ(ਵਿਅੰਜਕ *p){
  ਪੂਰਨ n_ਉਚਾਈ = p->p_ਖੱਬਾ ? p->p_ਖੱਬਾ->n_ਉਚਾਈ : 0;
  ਜੇ( NEVER(p->p_ਸੱਜਾ) && p->p_ਸੱਜਾ->n_ਉਚਾਈ>n_ਉਚਾਈ ){
    n_ਉਚਾਈ = p->p_ਸੱਜਾ->n_ਉਚਾਈ;
  }
  ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p) ){
    ਉਚਾਈ_ਦਾ_Select(p->x.pSelect, &n_ਉਚਾਈ);
  }ਨਹੀਂਤਾਂ ਜੇ( p->x.pList ){
    ਉਚਾਈ_ਦਾ_ਵਿਅੰਜਕ_List(p->x.pList, &n_ਉਚਾਈ);
    p->flags |= EP_Propagate & sqlite3_ਵਿਅੰਜਕ_List_Flags(p->x.pList);
  }
  p->n_ਉਚਾਈ = n_ਉਚਾਈ + 1;
}

/*
** Set the Expr.nHeight variable using the exprSetHeight() function. If
** the height is greater than the maximum allowed expression depth,
** leave an error in pParse.
**
** Also propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ_And_Flags(Parse *pParse, ਵਿਅੰਜਕ *p){
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ(p);
  sqlite3_ਵਿਅੰਜਕ_Check_ਉਚਾਈ(pParse, p->n_ਉਚਾਈ);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
ਪੂਰਨ sqlite3_Select_ਵਿਅੰਜਕ_ਉਚਾਈ(ਅਚਲ Select *p){
  ਪੂਰਨ n_ਉਚਾਈ = 0;
  ਉਚਾਈ_ਦਾ_Select(p, &n_ਉਚਾਈ);
  ਵਾਪਸ n_ਉਚਾਈ;
}
#ਨਹੀਂਤਾਂ /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ_And_Flags(Parse *pParse, ਵਿਅੰਜਕ *p){
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  ਜੇ( p && ਵਿਅੰਜਕ_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_ਵਿਅੰਜਕ_List_Flags(p->x.pList);
  }
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ(y)
#ਜੇ_ਅੰਤ /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_Error_ਵਿੱਥ(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ iOfst){
  ਜੇ( p_ਵਿਅੰਜਕ==0 ) ਵਾਪਸ;
  ਜੇ( NEVER(ਵਿਅੰਜਕ_Use_WJoin(p_ਵਿਅੰਜਕ)) ) ਵਾਪਸ;
  p_ਵਿਅੰਜਕ->w.iOfst = iOfst;
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
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  ਪੂਰਨ op,                 /* Expression opcode */
  ਅਚਲ Token *pToken,    /* Token argument.  Might be NULL */
  ਪੂਰਨ dequote             /* True to dequote */
){
  ਵਿਅੰਜਕ *pNew;
  ਪੂਰਨ nExtra = pToken ? pToken->n+1 : 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, ਆਕਾਰ(ਵਿਅੰਜਕ)+nExtra);
  ਜੇ( pNew ){
    memset(pNew, 0, ਆਕਾਰ(ਵਿਅੰਜਕ));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    ਜੇ( nExtra ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pToken!=0 );
      pNew->u.zToken = (ਅੱਖਰ*)&pNew[1];
      ਪ੍ਰਤਿਜ੍ਞਾ( pToken->z!=0 || pToken->n==0 );
      ਜੇ( pToken->n ) ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      ਜੇ( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_ਵਿਅੰਜਕ(pNew);
      }
    }
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
    pNew->n_ਉਚਾਈ = 1;
#ਜੇ_ਅੰਤ 
  }
  ਵਾਪਸ pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  ਪੂਰਨ op,                 /* Expression opcode */
  ਅਚਲ ਅੱਖਰ *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Int32(sqlite3 *db, ਪੂਰਨ iVal){
  ਵਿਅੰਜਕ *pNew = sqlite3DbMallocRawNN(db, ਆਕਾਰ(ਵਿਅੰਜਕ));
  ਜੇ( pNew ){
    memset(pNew, 0, ਆਕਾਰ(ਵਿਅੰਜਕ));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_ਮੁੱਲ|EP_ਪੱਤੀ|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_ਮੁੱਲ = iVal;
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
    pNew->n_ਉਚਾਈ = 1;
#ਜੇ_ਅੰਤ 
  }
  ਵਾਪਸ pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Attach_Subtrees(
  sqlite3 *db,
  ਵਿਅੰਜਕ *p_ਜੜ੍ਹ,
  ਵਿਅੰਜਕ *p_ਖੱਬਾ,
  ਵਿਅੰਜਕ *p_ਸੱਜਾ
){
  ਜੇ( p_ਜੜ੍ਹ==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed );
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਖੱਬਾ);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਸੱਜਾ);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਜੜ੍ਹ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਜੜ੍ਹ->x.pSelect==0 );
    ਜੇ( p_ਸੱਜਾ ){
      p_ਜੜ੍ਹ->p_ਸੱਜਾ = p_ਸੱਜਾ;
      p_ਜੜ੍ਹ->flags |= EP_Propagate & p_ਸੱਜਾ->flags;
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
      p_ਜੜ੍ਹ->n_ਉਚਾਈ = p_ਸੱਜਾ->n_ਉਚਾਈ+1;
    }ਨਹੀਂਤਾਂ{
      p_ਜੜ੍ਹ->n_ਉਚਾਈ = 1;
#ਜੇ_ਅੰਤ
    }
    ਜੇ( p_ਖੱਬਾ ){
      p_ਜੜ੍ਹ->p_ਖੱਬਾ = p_ਖੱਬਾ;
      p_ਜੜ੍ਹ->flags |= EP_Propagate & p_ਖੱਬਾ->flags;
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
      ਜੇ( p_ਖੱਬਾ->n_ਉਚਾਈ>=p_ਜੜ੍ਹ->n_ਉਚਾਈ ){
        p_ਜੜ੍ਹ->n_ਉਚਾਈ = p_ਖੱਬਾ->n_ਉਚਾਈ+1;
      }
#ਜੇ_ਅੰਤ
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
ਵਿਅੰਜਕ *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  ਪੂਰਨ op,                 /* Expression opcode */
  ਵਿਅੰਜਕ *p_ਖੱਬਾ,            /* Left operand */
  ਵਿਅੰਜਕ *p_ਸੱਜਾ            /* Right operand */
){
  ਵਿਅੰਜਕ *p;
  p = sqlite3DbMallocRawNN(pParse->db, ਆਕਾਰ(ਵਿਅੰਜਕ));
  ਜੇ( p ){
    memset(p, 0, ਆਕਾਰ(ਵਿਅੰਜਕ));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_ਵਿਅੰਜਕ_Attach_Subtrees(pParse->db, p, p_ਖੱਬਾ, p_ਸੱਜਾ);
    sqlite3_ਵਿਅੰਜਕ_Check_ਉਚਾਈ(pParse, p->n_ਉਚਾਈ);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, p_ਖੱਬਾ);
    sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, p_ਸੱਜਾ);
  }
  ਵਾਪਸ p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
ਖਾਲੀ sqlite3_PExpr_ਜੋੜੋ_Select(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, Select *pSelect){
  ਜੇ( p_ਵਿਅੰਜਕ ){
    p_ਵਿਅੰਜਕ->x.pSelect = pSelect;
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_xIsSelect|EP_Subquery);
    sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ_And_Flags(pParse, p_ਵਿਅੰਜਕ);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed );
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
Select *sqlite3_ਵਿਅੰਜਕ_List_ਮੰਜ਼ਿਲ_ਮੁੱਲਾਂ(Parse *pParse, ਪੂਰਨ nElem, ਵਿਅੰਜਕ_List *pEList){
  ਪੂਰਨ ii;
  Select *pRet = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( nElem>1 );
  ਲਈ(ii=0; ii<pEList->n_ਵਿਅੰਜਕ; ii++){
    Select *pSel;
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pEList->a[ii].p_ਵਿਅੰਜਕ;
    ਪੂਰਨ n_ਵਿਅੰਜਕ_Elem;
    ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਸਦਿਸ਼ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
      n_ਵਿਅੰਜਕ_Elem = p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ;
    }ਨਹੀਂਤਾਂ{
      n_ਵਿਅੰਜਕ_Elem = 1;
    }
    ਜੇ( n_ਵਿਅੰਜਕ_Elem!=nElem ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "IN(...) element has %d term%s - expected %d",
          n_ਵਿਅੰਜਕ_Elem, n_ਵਿਅੰਜਕ_Elem>1?"s":"", nElem
      );
      ਰੋਕੋ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
    pSel = sqlite3SelectNew(pParse, p_ਵਿਅੰਜਕ->x.pList, 0, 0, 0, 0, 0, SF_ਮੁੱਲਾਂ,0);
    p_ਵਿਅੰਜਕ->x.pList = 0;
    ਜੇ( pSel ){
      ਜੇ( pRet ){
        pSel->op = TK_ALL;
        pSel->p_ਪੂਰਵ = pRet;
      }
      pRet = pSel;
    }
  }

  ਜੇ( pRet && pRet->p_ਪੂਰਵ ){
    pRet->selFlags |= SF_Multi_ਮੁੱਲ;
  }
  sqlite3_ਵਿਅੰਜਕ_List_Delete(pParse->db, pEList);
  ਵਾਪਸ pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_And(Parse *pParse, ਵਿਅੰਜਕ *p_ਖੱਬਾ, ਵਿਅੰਜਕ *p_ਸੱਜਾ){
  sqlite3 *db = pParse->db;
  ਜੇ( p_ਖੱਬਾ==0  ){
    ਵਾਪਸ p_ਸੱਜਾ;
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਸੱਜਾ==0 ){
    ਵਾਪਸ p_ਖੱਬਾ;
  }ਨਹੀਂਤਾਂ{
    u32 f = p_ਖੱਬਾ->flags | p_ਸੱਜਾ->flags;
    ਜੇ( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !ਵਿੱਚ_RENAME_OBJECT
    ){
      ਵਿਅੰਜਕ *p;
      sqlite3_ਵਿਅੰਜਕ_Deferred_Delete(pParse, p_ਖੱਬਾ);
      sqlite3_ਵਿਅੰਜਕ_Deferred_Delete(pParse, p_ਸੱਜਾ);
      p = sqlite3_ਵਿਅੰਜਕ_Int32(db, 0);
      ਜੇ( p ) p->op2 = 1;
      ਵਾਪਸ p;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ sqlite3PExpr(pParse, TK_AND, p_ਖੱਬਾ, p_ਸੱਜਾ);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Function(
  Parse *pParse,        /* Parsing context */
  ਵਿਅੰਜਕ_List *pList,      /* Argument list */
  ਅਚਲ Token *pToken,  /* Name of the function */
  ਪੂਰਨ eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  ਵਿਅੰਜਕ *pNew;
  sqlite3 *db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( pToken );
  pNew = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_FUNCTION, pToken, 1);
  ਜੇ( pNew==0 ){
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    ਵਾਪਸ 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (ਪੂਰਨ)(pToken->z - pParse->z_ਪੂਛ);
  ਜੇ( pList
   && pList->n_ਵਿਅੰਜਕ > pParse->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pNew, EP_HasFunc);
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pNew) );
  sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ_And_Flags(pParse, pNew);
  ਜੇ( eDistinct==SF_Distinct ) ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pNew, EP_Distinct);
  ਵਾਪਸ pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕ੍ਰਮ_By_Aggregate_Error(Parse *pParse, ਵਿਅੰਜਕ *p){
  sqlite3_Error_ਸੁਨੇਹਾ(pParse,
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
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Function_ਕ੍ਰਮ_By(
  Parse *pParse,        /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,          /* The function call to which ORDER BY is to be added */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By    /* The ORDER BY clause to add */
){
  ਵਿਅੰਜਕ *pOB;
  sqlite3 *db = pParse->db;
  ਜੇ( NEVER(p_ਕ੍ਰਮ_By==0) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed );
    ਵਾਪਸ;
  }
  ਜੇ( p_ਵਿਅੰਜਕ==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed );
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਕ੍ਰਮ_By);
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
  ਜੇ( p_ਵਿਅੰਜਕ->x.pList==0 || NEVER(p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3_ਵਿਅੰਜਕ_List_Delete_Generic, p_ਕ੍ਰਮ_By);
    ਵਾਪਸ;
  }
  ਜੇ( Is_ਖਿੜਕੀ_Func(p_ਵਿਅੰਜਕ) ){
    sqlite3_ਵਿਅੰਜਕ_ਕ੍ਰਮ_By_Aggregate_Error(pParse, p_ਵਿਅੰਜਕ);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਕ੍ਰਮ_By);
    ਵਾਪਸ;
  }
  ਜੇ( p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ>db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ] ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many terms in ORDER BY clause");
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਕ੍ਰਮ_By);
    ਵਾਪਸ;
  }

  pOB = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ਕ੍ਰਮ, 0, 0);
  ਜੇ( pOB==0 ){
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਕ੍ਰਮ_By);
    ਵਾਪਸ;
  }
  pOB->x.pList = p_ਕ੍ਰਮ_By;
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pOB) );
  p_ਵਿਅੰਜਕ->p_ਖੱਬਾ = pOB;
  ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pOB, EP_ਭਰਿਆ_ਮਾਪ);
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
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,     /* The function invocation */
  ਅਚਲ FuncDef *pDef    /* The function being invoked */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿੱਚ_RENAME_OBJECT );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pDef->funcFlags & (SQLITE_FUNC_ਸਿੱਧਾ|SQLITE_FUNC_UNSAFE))!=0 );
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    ਜੇ( (pDef->funcFlags & SQLITE_FUNC_ਸਿੱਧਾ)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "unsafe use of %#T()", p_ਵਿਅੰਜਕ);
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
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਸੌਂਪੋ_Var_ਸੰਖਿਆ(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, u32 n){
  sqlite3 *db = pParse->db;
  ਅਚਲ ਅੱਖਰ *z;
  ynVar x;

  ਜੇ( p_ਵਿਅੰਜਕ==0 ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ|EP_Reduced|EP_TokenOnly) );
  z = p_ਵਿਅੰਜਕ->u.zToken;
  ਪ੍ਰਤਿਜ੍ਞਾ( z!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( z[0]!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( n==(u32)sqlite3Strlen30(z) );
  ਜੇ( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    ਪ੍ਰਤਿਜ੍ਞਾ( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ do_ਜੋੜੋ = 0;
    ਜੇ( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      ਪੂਰਨ bOk;
      ਜੇ( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }ਨਹੀਂਤਾਂ{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_VARIABLE_ਸੰਖਿਆ]-1 );
      testcase( i==db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_VARIABLE_ਸੰਖਿਆ] );
      ਜੇ( bOk==0 || i<1 || i>db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_VARIABLE_ਸੰਖਿਆ] ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "variable number must be between ?1 and ?%d",
            db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_VARIABLE_ਸੰਖਿਆ]);
        sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(pParse->db, p_ਵਿਅੰਜਕ);
        ਵਾਪਸ;
      }
      x = (ynVar)i;
      ਜੇ( x>pParse->nVar ){
        pParse->nVar = (ਪੂਰਨ)x;
        do_ਜੋੜੋ = 1;
      }ਨਹੀਂਤਾਂ ਜੇ( x<SQLITE_VNBMC*64 ){
        do_ਜੋੜੋ = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_VList_ਅੰਕ_ਮੰਜ਼ਿਲ_Name(pParse->pVList, x)==0 ){
        do_ਜੋੜੋ = 1;
      }
    }ਨਹੀਂਤਾਂ{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_ਮੰਜ਼ਿਲ_ਅੰਕ(pParse->pVList, z, n);
      ਜੇ( x==0 ){
        x = (ynVar)(++pParse->nVar);
        do_ਜੋੜੋ = 1;
      }
    }
    ਜੇ( do_ਜੋੜੋ ){
      pParse->pVList = sqlite3_VList_ਜੋੜੋ(db, pParse->pVList, z, n, x);
      ਜੇ( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = x;
  ਜੇ( x>db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_VARIABLE_ਸੰਖਿਆ] ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many SQL variables");
    sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(pParse->db, p_ਵਿਅੰਜਕ);
  }
}

/*
** Recursively delete an expression tree.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Delete_NN(sqlite3 *db, ਵਿਅੰਜਕ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
ਵਿਅੰਜਕ_Delete_Restart:
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_UValue(p) || p->u.i_ਮੁੱਲ>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_YWin(p) || !ਵਿਅੰਜਕ_Use_YSub(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->op!=TK_FUNCTION || !ਵਿਅੰਜਕ_Use_YSub(p) );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p, EP_ਪੱਤੀ) && !ਵਿਅੰਜਕ_Has_Property(p, EP_TokenOnly) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਖੱਬਾ==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਸੱਜਾ==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_XSelect(p) || p->x.pSelect==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_XList(p) || p->x.pList==0 );
  }
#ਜੇ_ਅੰਤ
  ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p, (EP_TokenOnly|EP_ਪੱਤੀ)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    ਪ੍ਰਤਿਜ੍ਞਾ( (ਵਿਅੰਜਕ_Use_XList(p) && p->x.pList==0) || p->p_ਸੱਜਾ==0 );
    ਜੇ( p->p_ਸੱਜਾ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_WinFunc) );
      sqlite3_ਵਿਅੰਜਕ_Delete_NN(db, p->p_ਸੱਜਾ);
    }ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p->x.pList);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p, EP_WinFunc) ){
        sqlite3_ਖਿੜਕੀ_Delete(db, p->y.pWin);
      }
#ਜੇ_ਅੰਤ
    }
    ਜੇ( p->p_ਖੱਬਾ && p->op!=TK_SELECT_ਕਾਲਮ ){
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = p->p_ਖੱਬਾ;
      ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Static)
       && !ਵਿਅੰਜਕ_Has_Property(p_ਖੱਬਾ, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_ਖੱਬਾ;
        ਜਾਓ ਵਿਅੰਜਕ_Delete_Restart;
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਵਿਅੰਜਕ_Delete_NN(db, p_ਖੱਬਾ);
      }
    }
  }
  ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Delete(sqlite3 *db, ਵਿਅੰਜਕ *p){
  ਜੇ( p ) sqlite3_ਵਿਅੰਜਕ_Delete_NN(db, p);
}
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Delete_Generic(sqlite3 *db, ਖਾਲੀ *p){
  ਜੇ( ALWAYS(p) ) sqlite3_ਵਿਅੰਜਕ_Delete_NN(db, (ਵਿਅੰਜਕ*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
ਖਾਲੀ sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  ਜੇ( p==0 ){
    /* Nothing to clear */
  }ਨਹੀਂਤਾਂ ਜੇ( p->pOn ){
    sqlite3_ਵਿਅੰਜਕ_Delete_NN(db, p->pOn);
  }ਨਹੀਂਤਾਂ ਜੇ( p->pUsing ){
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Deferred_Delete(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਵਾਪਸ 0==sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3_ਵਿਅੰਜਕ_Delete_Generic, p_ਵਿਅੰਜਕ);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Unmap_And_Delete(Parse *pParse, ਵਿਅੰਜਕ *p){
  ਜੇ( p ){
    ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
      sqlite3_Rename_ਵਿਅੰਜਕ_Unmap(pParse, p);
    }
    sqlite3_ਵਿਅੰਜਕ_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Struct_ਮਾਪ(ਅਚਲ ਵਿਅੰਜਕ *p){
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p, EP_TokenOnly) ) ਵਾਪਸ ਵਿਅੰਜਕ_TOKENONLYSIZE;
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p, EP_Reduced) ) ਵਾਪਸ ਵਿਅੰਜਕ_REDUCEDSIZE;
  ਵਾਪਸ ਵਿਅੰਜਕ_FULLSIZE;
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
ਸਥਿਰ ਪੂਰਨ duped_ਵਿਅੰਜਕ_Struct_ਮਾਪ(ਅਚਲ ਵਿਅੰਜਕ *p, ਪੂਰਨ flags){
  ਪੂਰਨ n_ਮਾਪ;
  ਪ੍ਰਤਿਜ੍ਞਾ( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_FULLSIZE<=0xfff );
  ਪ੍ਰਤਿਜ੍ਞਾ( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  ਜੇ( 0==flags || ਵਿਅੰਜਕ_Has_Property(p, EP_ਭਰਿਆ_ਮਾਪ) ){
    n_ਮਾਪ = ਵਿਅੰਜਕ_FULLSIZE;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_OuterON) );
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_VVAProperty(p, EP_NoReduce) );
    ਜੇ( p->p_ਖੱਬਾ || p->x.pList ){
      n_ਮਾਪ = ਵਿਅੰਜਕ_REDUCEDSIZE | EP_Reduced;
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਸੱਜਾ==0 );
      n_ਮਾਪ = ਵਿਅੰਜਕ_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  ਵਾਪਸ n_ਮਾਪ;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ਸਥਿਰ ਪੂਰਨ duped_ਵਿਅੰਜਕ_ਗੰਢ_ਮਾਪ(ਅਚਲ ਵਿਅੰਜਕ *p, ਪੂਰਨ flags){
  ਪੂਰਨ nByte = duped_ਵਿਅੰਜਕ_Struct_ਮਾਪ(p, flags) & 0xfff;
  ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Int_ਮੁੱਲ) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  ਵਾਪਸ ROUND8(nByte);
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
ਸਥਿਰ ਪੂਰਨ duped_ਵਿਅੰਜਕ_ਮਾਪ(ਅਚਲ ਵਿਅੰਜਕ *p){
  ਪੂਰਨ nByte;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  nByte = duped_ਵਿਅੰਜਕ_ਗੰਢ_ਮਾਪ(p, EXPRDUP_REDUCE);
  ਜੇ( p->p_ਖੱਬਾ ) nByte += duped_ਵਿਅੰਜਕ_ਮਾਪ(p->p_ਖੱਬਾ);
  ਜੇ( p->p_ਸੱਜਾ ) nByte += duped_ਵਿਅੰਜਕ_ਮਾਪ(p->p_ਸੱਜਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( nByte==ROUND8(nByte) );
  ਵਾਪਸ nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ EdupBuf EdupBuf;
ਬਣਤਰ EdupBuf {
  u8 *zAlloc;          /* Memory space available for storage */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  u8 *z_ਸਿਰਾ;            /* First byte past the end of memory */
#ਜੇ_ਅੰਤ
};

/*
** This function is similar to sqlite3ExprDup(), except that if pEdupBuf
** is not NULL then it points to memory that can be used to store a copy
** of the input Expr p together with its p->u.zToken (if any).  pEdupBuf
** is updated with the new buffer tail prior to returning.
*/
ਸਥਿਰ ਵਿਅੰਜਕ *ਵਿਅੰਜਕ_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ਅਚਲ ਵਿਅੰਜਕ *p,        /* Expr tree to be duplicated */
  ਪੂਰਨ dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  ਵਿਅੰਜਕ *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  ਪੂਰਨ nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
  ਪ੍ਰਤਿਜ੍ਞਾ( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  ਪ੍ਰਤਿਜ੍ਞਾ( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  ਜੇ( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    sEdupBuf.z_ਸਿਰਾ = pEdupBuf->z_ਸਿਰਾ;
#ਜੇ_ਅੰਤ
    staticFlag = EP_Static;
    ਪ੍ਰਤਿਜ੍ਞਾ( sEdupBuf.zAlloc!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( dupFlags==EXPRDUP_REDUCE );
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ nAlloc;
    ਜੇ( dupFlags ){
      nAlloc = duped_ਵਿਅੰਜਕ_ਮਾਪ(p);
    }ਨਹੀਂਤਾਂ ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Int_ਮੁੱਲ) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(ਵਿਅੰਜਕ_FULLSIZE + nToken);
    }ਨਹੀਂਤਾਂ{
      nToken = 0;
      nAlloc = ROUND8(ਵਿਅੰਜਕ_FULLSIZE);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    sEdupBuf.z_ਸਿਰਾ = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#ਜੇ_ਅੰਤ
    
    staticFlag = 0;
  }
  pNew = (ਵਿਅੰਜਕ *)sEdupBuf.zAlloc;
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pNew) );

  ਜੇ( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ n_Struct_ਮਾਪ = duped_ਵਿਅੰਜਕ_Struct_ਮਾਪ(p, dupFlags);
    ਪੂਰਨ n_New_ਮਾਪ = n_Struct_ਮਾਪ & 0xfff;
    ਜੇ( nToken<0 ){
      ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Int_ਮੁੱਲ) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }ਨਹੀਂਤਾਂ{
        nToken = 0;
      }
    }
    ਜੇ( dupFlags ){
      ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੂਰਨ)(sEdupBuf.z_ਸਿਰਾ - sEdupBuf.zAlloc) >= n_New_ਮਾਪ+nToken );
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Has_Property(p, EP_Reduced)==0 );
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(sEdupBuf.zAlloc, p, n_New_ਮਾਪ);
    }ਨਹੀਂਤਾਂ{
      u32 n_ਮਾਪ = (u32)ਵਿਅੰਜਕ_Struct_ਮਾਪ(p);
      ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੂਰਨ)(sEdupBuf.z_ਸਿਰਾ - sEdupBuf.zAlloc) >=
                                                   (ਪੂਰਨ)ਵਿਅੰਜਕ_FULLSIZE+nToken );
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(sEdupBuf.zAlloc, p, n_ਮਾਪ);
      ਜੇ( n_ਮਾਪ<ਵਿਅੰਜਕ_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[n_ਮਾਪ], 0, ਵਿਅੰਜਕ_FULLSIZE-n_ਮਾਪ);
      }
      n_New_ਮਾਪ = ਵਿਅੰਜਕ_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= n_Struct_ਮਾਪ & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    ਵਿਅੰਜਕ_Clear_VVAProperties(pNew);
    ਜੇ( dupFlags ){
      ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    ਪ੍ਰਤਿਜ੍ਞਾ( nToken>=0 );
    ਜੇ( nToken>0 ){
      ਅੱਖਰ *zToken = pNew->u.zToken = (ਅੱਖਰ*)&sEdupBuf.zAlloc[n_New_ਮਾਪ];
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zToken, p->u.zToken, nToken);
      n_New_ਮਾਪ += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(n_New_ਮਾਪ);

    ਜੇ( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_ਪੱਤੀ))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }ਨਹੀਂਤਾਂ{
        pNew->x.pList = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p->x.pList,
                           p->op!=TK_ਕ੍ਰਮ ? dupFlags : 0);
      }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3_ਖਿੜਕੀ_Dup(db, pNew, p->y.pWin);
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Has_Property(pNew, EP_WinFunc) );
      }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      ਜੇ( dupFlags ){
        ਜੇ( p->op==TK_SELECT_ਕਾਲਮ ){
          pNew->p_ਖੱਬਾ = p->p_ਖੱਬਾ;
          ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਸੱਜਾ==0 
               || p->p_ਸੱਜਾ==p->p_ਖੱਬਾ
               || ਵਿਅੰਜਕ_Has_Property(p->p_ਖੱਬਾ, EP_Subquery) );
        }ਨਹੀਂਤਾਂ{
          pNew->p_ਖੱਬਾ = p->p_ਖੱਬਾ ?
                      ਵਿਅੰਜਕ_Dup(db, p->p_ਖੱਬਾ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_ਸੱਜਾ = p->p_ਸੱਜਾ ?
                       ਵਿਅੰਜਕ_Dup(db, p->p_ਸੱਜਾ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( p->op==TK_SELECT_ਕਾਲਮ ){
          pNew->p_ਖੱਬਾ = p->p_ਖੱਬਾ;
          ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਸੱਜਾ==0 
               || p->p_ਸੱਜਾ==p->p_ਖੱਬਾ
               || ਵਿਅੰਜਕ_Has_Property(p->p_ਖੱਬਾ, EP_Subquery) );
        }ਨਹੀਂਤਾਂ{
          pNew->p_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p->p_ਖੱਬਾ, 0);
        }
        pNew->p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p->p_ਸੱਜਾ, 0);
      }
    }
  }
  ਜੇ( pEdupBuf ) ਸਮ੍ਰਿਤੀ_ਨਕਲ(pEdupBuf, &sEdupBuf, ਆਕਾਰ(sEdupBuf));
  ਪ੍ਰਤਿਜ੍ਞਾ( sEdupBuf.zAlloc <= sEdupBuf.z_ਸਿਰਾ );
  ਵਾਪਸ pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  ਜੇ( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    ਜੇ( pRet ){
      ਪੂਰਨ i;
      pRet->nCte = p->nCte;
      ਲਈ(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].p_ਖਾਨੇ_ਗਿਣਤੀ = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p->a[i].p_ਖਾਨੇ_ਗਿਣਤੀ, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  ਵਾਪਸ pRet;
}
#ਨਹੀਂਤਾਂ
# define sqlite3WithDup(x,y) 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ਸਥਿਰ ਪੂਰਨ gatherSelectWindowsCallback(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION && ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    ਖਿੜਕੀ *pWin = p_ਵਿਅੰਜਕ->y.pWin;
    ਪ੍ਰਤਿਜ੍ਞਾ( pWin );
    ਪ੍ਰਤਿਜ੍ਞਾ( Is_ਖਿੜਕੀ_Func(p_ਵਿਅੰਜਕ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( pWin->ppThis==0 );
    sqlite3_ਖਿੜਕੀ_Link(pSelect, pWin);
  }
  ਵਾਪਸ WRC_Continue;
}
ਸਥਿਰ ਪੂਰਨ gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  ਵਾਪਸ p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ਸਥਿਰ ਖਾਲੀ gatherSelectWindows(Select *p){
  Walker w;
  w.x_ਵਿਅੰਜਕ_Callback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}
#ਜੇ_ਅੰਤ


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
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Dup(sqlite3 *db, ਅਚਲ ਵਿਅੰਜਕ *p, ਪੂਰਨ flags){
  ਪ੍ਰਤਿਜ੍ਞਾ( flags==0 || flags==EXPRDUP_REDUCE );
  ਵਾਪਸ p ? ਵਿਅੰਜਕ_Dup(db, p, flags, 0) : 0;
}
ਵਿਅੰਜਕ_List *sqlite3_ਵਿਅੰਜਕ_List_Dup(sqlite3 *db, ਅਚਲ ਵਿਅੰਜਕ_List *p, ਪੂਰਨ flags){
  ਵਿਅੰਜਕ_List *pNew;
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  ਅਚਲ ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_Old_ਵਸਤੂ;
  ਪੂਰਨ i;
  ਵਿਅੰਜਕ *p_ਪੂਰਵ_Select_ਖਾਨਾ_Old = 0;
  ਵਿਅੰਜਕ *p_ਪੂਰਵ_Select_ਖਾਨਾ_New = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3_Db_Malloc_ਮਾਪ(db, p));
  ਜੇ( pNew==0 ) ਵਾਪਸ 0;
  pNew->n_ਵਿਅੰਜਕ = p->n_ਵਿਅੰਜਕ;
  pNew->nAlloc = p->nAlloc;
  p_ਵਸਤੂ = pNew->a;
  p_Old_ਵਸਤੂ = p->a;
  ਲਈ(i=0; i<p->n_ਵਿਅੰਜਕ; i++, p_ਵਸਤੂ++, p_Old_ਵਸਤੂ++){
    ਵਿਅੰਜਕ *p_Old_ਵਿਅੰਜਕ = p_Old_ਵਸਤੂ->p_ਵਿਅੰਜਕ;
    ਵਿਅੰਜਕ *p_New_ਵਿਅੰਜਕ;
    p_ਵਸਤੂ->p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_Old_ਵਿਅੰਜਕ, flags);
    ਜੇ( p_Old_ਵਿਅੰਜਕ
     && p_Old_ਵਿਅੰਜਕ->op==TK_SELECT_ਕਾਲਮ
     && (p_New_ਵਿਅੰਜਕ = p_ਵਸਤੂ->p_ਵਿਅੰਜਕ)!=0
    ){
      ਜੇ( p_New_ਵਿਅੰਜਕ->p_ਸੱਜਾ ){
        p_ਪੂਰਵ_Select_ਖਾਨਾ_Old = p_Old_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        p_ਪੂਰਵ_Select_ਖਾਨਾ_New = p_New_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        p_New_ਵਿਅੰਜਕ->p_ਖੱਬਾ = p_New_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( p_Old_ਵਿਅੰਜਕ->p_ਖੱਬਾ!=p_ਪੂਰਵ_Select_ਖਾਨਾ_Old ){
          p_ਪੂਰਵ_Select_ਖਾਨਾ_Old = p_Old_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
          p_ਪੂਰਵ_Select_ਖਾਨਾ_New = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਪੂਰਵ_Select_ਖਾਨਾ_Old, flags);
          p_New_ਵਿਅੰਜਕ->p_ਸੱਜਾ = p_ਪੂਰਵ_Select_ਖਾਨਾ_New;
        }
        p_New_ਵਿਅੰਜਕ->p_ਖੱਬਾ = p_ਪੂਰਵ_Select_ਖਾਨਾ_New;
      }
    }
    p_ਵਸਤੂ->zEName = sqlite3DbStrDup(db, p_Old_ਵਸਤੂ->zEName);
    p_ਵਸਤੂ->fg = p_Old_ਵਸਤੂ->fg;
    p_ਵਸਤੂ->u = p_Old_ਵਸਤੂ->u;
  }
  ਵਾਪਸ pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#ਜੇ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ਅਚਲ SrcList *p, ਪੂਰਨ flags){
  SrcList *pNew;
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  ਜੇ( pNew==0 ) ਵਾਪਸ 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  ਲਈ(i=0; i<p->nSrc; i++){
    Src_ਵਸਤੂ *p_New_ਵਸਤੂ = &pNew->a[i];
    ਅਚਲ Src_ਵਸਤੂ *p_Old_ਵਸਤੂ = &p->a[i];
    ਸਾਰਣੀ *pTab;
    p_New_ਵਸਤੂ->fg = p_Old_ਵਸਤੂ->fg;
    ਜੇ( p_Old_ਵਸਤੂ->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, ਆਕਾਰ(Subquery));
      ਜੇ( pNewSubq==0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed );
        p_New_ਵਸਤੂ->fg.isSubquery = 0;
      }ਨਹੀਂਤਾਂ{
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNewSubq, p_Old_ਵਸਤੂ->u4.pSubq, ਆਕਾਰ(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        ਜੇ( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          p_New_ਵਸਤੂ->fg.isSubquery = 0;
        }
      }
      p_New_ਵਸਤੂ->u4.pSubq = pNewSubq;
    }ਨਹੀਂਤਾਂ ਜੇ( p_Old_ਵਸਤੂ->fg.fixedSchema ){
      p_New_ਵਸਤੂ->u4.pSchema = p_Old_ਵਸਤੂ->u4.pSchema;
    }ਨਹੀਂਤਾਂ{
      p_New_ਵਸਤੂ->u4.zDatabase = sqlite3DbStrDup(db, p_Old_ਵਸਤੂ->u4.zDatabase);
    }
    p_New_ਵਸਤੂ->zName = sqlite3DbStrDup(db, p_Old_ਵਸਤੂ->zName);
    p_New_ਵਸਤੂ->zAlias = sqlite3DbStrDup(db, p_Old_ਵਸਤੂ->zAlias);
    p_New_ਵਸਤੂ->iCursor = p_Old_ਵਸਤੂ->iCursor;
    ਜੇ( p_New_ਵਸਤੂ->fg.is_ਸੂਚਕਾਂਕਿਤ_By ){
      p_New_ਵਸਤੂ->u1.z_ਸੂਚਕਾਂਕਿਤ_By = sqlite3DbStrDup(db, p_Old_ਵਸਤੂ->u1.z_ਸੂਚਕਾਂਕਿਤ_By);
    }ਨਹੀਂਤਾਂ ਜੇ( p_New_ਵਸਤੂ->fg.isTabFunc ){
      p_New_ਵਸਤੂ->u1.pFuncArg =
          sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p_Old_ਵਸਤੂ->u1.pFuncArg, flags);
    }ਨਹੀਂਤਾਂ{
      p_New_ਵਸਤੂ->u1.n_ਪੰਕਤੀ = p_Old_ਵਸਤੂ->u1.n_ਪੰਕਤੀ;
    }
    p_New_ਵਸਤੂ->u2 = p_Old_ਵਸਤੂ->u2;
    ਜੇ( p_New_ਵਸਤੂ->fg.isCte ){
      p_New_ਵਸਤੂ->u2.pCteUse->nUse++;
    }
    pTab = p_New_ਵਸਤੂ->pSTab = p_Old_ਵਸਤੂ->pSTab;
    ਜੇ( pTab ){
      pTab->nTabRef++;
    }
    ਜੇ( p_Old_ਵਸਤੂ->fg.isUsing ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_New_ਵਸਤੂ->fg.isUsing );
      p_New_ਵਸਤੂ->u3.pUsing = sqlite3IdListDup(db, p_Old_ਵਸਤੂ->u3.pUsing);
    }ਨਹੀਂਤਾਂ{
      p_New_ਵਸਤੂ->u3.pOn = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_Old_ਵਸਤੂ->u3.pOn, flags);
    }
    p_New_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ = p_Old_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ;
  }
  ਵਾਪਸ pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ਅਚਲ IdList *p){
  IdList *pNew;
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  ਜੇ( pNew==0 ) ਵਾਪਸ 0;
  pNew->nId = p->nId;
  ਲਈ(i=0; i<p->nId; i++){
    ਬਣਤਰ Id_List_ਵਸਤੂ *p_New_ਵਸਤੂ = &pNew->a[i];
    ਅਚਲ ਬਣਤਰ Id_List_ਵਸਤੂ *p_Old_ਵਸਤੂ = &p->a[i];
    p_New_ਵਸਤੂ->zName = sqlite3DbStrDup(db, p_Old_ਵਸਤੂ->zName);
  }
  ਵਾਪਸ pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ਅਚਲ Select *pDup, ਪੂਰਨ flags){
  Select *pRet = 0;
  Select *p_ਅਗਲਾ = 0;
  Select **pp = &pRet;
  ਅਚਲ Select *p;

  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਲਈ(p=pDup; p; p=p->p_ਪੂਰਵ){
    Select *pNew = sqlite3DbMallocRawNN(db, ਆਕਾਰ(*p) );
    ਜੇ( pNew==0 ) ਰੋਕੋ;
    pNew->pEList = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_ਵਿਅੰਜਕ_Dup(db, p->pWhere, flags);
    pNew->p_ਸਮੂਹ_By = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p->p_ਸਮੂਹ_By, flags);
    pNew->pHaving = sqlite3_ਵਿਅੰਜਕ_Dup(db, p->pHaving, flags);
    pNew->p_ਕ੍ਰਮ_By = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p->p_ਕ੍ਰਮ_By, flags);
    pNew->op = p->op;
    pNew->p_ਅਗਲਾ = p_ਅਗਲਾ;
    pNew->p_ਪੂਰਵ = 0;
    pNew->p_ਹੱਦਬੰਦੀ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p->p_ਹੱਦਬੰਦੀ, flags);
    pNew->i_ਹੱਦਬੰਦੀ = 0;
    pNew->i_ਵਿੱਥ = 0;
    pNew->selFlags = p->selFlags;
    pNew->n_Select_ਪੰਕਤੀ = p->n_Select_ਪੰਕਤੀ;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3_ਖਿੜਕੀ_List_Dup(db, p->pWinDefn);
    ਜੇ( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#ਜੇ_ਅੰਤ
    pNew->selId = p->selId;
    ਜੇ( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->p_ਅਗਲਾ = 0;
      sqlite3SelectDelete(db, pNew);
      ਰੋਕੋ;
    }
    *pp = pNew;
    pp = &pNew->p_ਪੂਰਵ;
    p_ਅਗਲਾ = pNew;
  }
  ਵਾਪਸ pRet;
}
#ਨਹੀਂਤਾਂ
Select *sqlite3SelectDup(sqlite3 *db, ਅਚਲ Select *p, ਪੂਰਨ flags){
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 );
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ


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
ਸਥਿਰ ਅਚਲ ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ zero_ਵਸਤੂ = {0};
SQLITE_NOINLINE ਵਿਅੰਜਕ_List *sqlite3_ਵਿਅੰਜਕ_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ             /* Expression to be appended. Might be NULL */
){
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  ਵਿਅੰਜਕ_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  ਜੇ( pList==0 ){
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ);
    ਵਾਪਸ 0;
  }
  pList->nAlloc = 4;
  pList->n_ਵਿਅੰਜਕ = 1;
  p_ਵਸਤੂ = &pList->a[0];
  *p_ਵਸਤੂ = zero_ਵਸਤੂ;
  p_ਵਸਤੂ->p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ;
  ਵਾਪਸ pList;
}
SQLITE_NOINLINE ਵਿਅੰਜਕ_List *sqlite3_ਵਿਅੰਜਕ_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ਵਿਅੰਜਕ_List *pList,        /* List to which to append. Might be NULL */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ             /* Expression to be appended. Might be NULL */
){
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  ਵਿਅੰਜਕ_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  ਜੇ( pNew==0 ){
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pList);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ);
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    pList = pNew;
  }
  p_ਵਸਤੂ = &pList->a[pList->n_ਵਿਅੰਜਕ++];
  *p_ਵਸਤੂ = zero_ਵਸਤੂ;
  p_ਵਸਤੂ->p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ;
  ਵਾਪਸ pList;
}
ਵਿਅੰਜਕ_List *sqlite3_ਵਿਅੰਜਕ_List_Append(
  Parse *pParse,          /* Parsing context */
  ਵਿਅੰਜਕ_List *pList,        /* List to which to append. Might be NULL */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ             /* Expression to be appended. Might be NULL */
){
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  ਜੇ( pList==0 ){
    ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_List_Append_New(pParse->db,p_ਵਿਅੰਜਕ);
  }
  ਜੇ( pList->nAlloc<pList->n_ਵਿਅੰਜਕ+1 ){
    ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_List_Append_Grow(pParse->db,pList,p_ਵਿਅੰਜਕ);
  }
  p_ਵਸਤੂ = &pList->a[pList->n_ਵਿਅੰਜਕ++];
  *p_ਵਸਤੂ = zero_ਵਸਤੂ;
  p_ਵਸਤੂ->p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ;
  ਵਾਪਸ pList;
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
ਵਿਅੰਜਕ_List *sqlite3_ਵਿਅੰਜਕ_List_Append_ਸਦਿਸ਼(
  Parse *pParse,         /* Parsing context */
  ਵਿਅੰਜਕ_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  ਪੂਰਨ n;
  ਪੂਰਨ i;
  ਪੂਰਨ i_ਪਹਿਲਾ = pList ? pList->n_ਵਿਅੰਜਕ : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  ਜੇ( NEVER(pColumns==0) ) ਜਾਓ ਸਦਿਸ਼_append_error;
  ਜੇ( p_ਵਿਅੰਜਕ==0 ) ਜਾਓ ਸਦਿਸ਼_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ)) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    ਜਾਓ ਸਦਿਸ਼_append_error;
  }

  ਲਈ(i=0; i<pColumns->nId; i++){
    ਵਿਅੰਜਕ *p_Sub_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_For_ਸਦਿਸ਼_ਖੇਤਰ(pParse, p_ਵਿਅੰਜਕ, i, pColumns->nId);
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Sub_ਵਿਅੰਜਕ!=0 || db->mallocFailed );
    ਜੇ( p_Sub_ਵਿਅੰਜਕ==0 ) ਜਾਰੀ;
    pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pList, p_Sub_ਵਿਅੰਜਕ);
    ਜੇ( pList ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pList->n_ਵਿਅੰਜਕ==i_ਪਹਿਲਾ+i+1 );
      pList->a[pList->n_ਵਿਅੰਜਕ-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  ਜੇ( !db->mallocFailed && p_ਵਿਅੰਜਕ->op==TK_SELECT && ALWAYS(pList!=0) ){
    ਵਿਅੰਜਕ *p_ਪਹਿਲਾ = pList->a[i_ਪਹਿਲਾ].p_ਵਿਅੰਜਕ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਹਿਲਾ!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਹਿਲਾ->op==TK_SELECT_ਕਾਲਮ );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    p_ਪਹਿਲਾ->p_ਸੱਜਾ = p_ਵਿਅੰਜਕ;
    p_ਵਿਅੰਜਕ = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    p_ਪਹਿਲਾ->i_ਸਾਰਣੀ = pColumns->nId;
  }

ਸਦਿਸ਼_append_error:
  sqlite3_ਵਿਅੰਜਕ_Unmap_And_Delete(pParse, p_ਵਿਅੰਜਕ);
  sqlite3IdListDelete(db, pColumns);
  ਵਾਪਸ pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_ਸੈੱਟ_ਛਾਂਟੋ_ਕ੍ਰਮ(ਵਿਅੰਜਕ_List *p, ਪੂਰਨ i_ਛਾਂਟੋ_ਕ੍ਰਮ, ਪੂਰਨ eNulls){
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਵਿਅੰਜਕ>0 );

  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਛਾਂਟੋ_ਕ੍ਰਮ==SQLITE_SO_UNDEFINED
       || i_ਛਾਂਟੋ_ਕ੍ਰਮ==SQLITE_SO_ASC
       || i_ਛਾਂਟੋ_ਕ੍ਰਮ==SQLITE_SO_DESC
  );
  ਪ੍ਰਤਿਜ੍ਞਾ( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  p_ਵਸਤੂ = &p->a[p->n_ਵਿਅੰਜਕ-1];
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.bNulls==0 );
  ਜੇ( i_ਛਾਂਟੋ_ਕ੍ਰਮ==SQLITE_SO_UNDEFINED ){
    i_ਛਾਂਟੋ_ਕ੍ਰਮ = SQLITE_SO_ASC;
  }
  p_ਵਸਤੂ->fg.ਛਾਂਟੋ_Flags = (u8)i_ਛਾਂਟੋ_ਕ੍ਰਮ;

  ਜੇ( eNulls!=SQLITE_SO_UNDEFINED ){
    p_ਵਸਤੂ->fg.bNulls = 1;
    ਜੇ( i_ਛਾਂਟੋ_ਕ੍ਰਮ!=eNulls ){
      p_ਵਸਤੂ->fg.ਛਾਂਟੋ_Flags |= KEYINFO_ਕ੍ਰਮ_BIGNULL;
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
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_ਸੈੱਟ_Name(
  Parse *pParse,          /* Parsing context */
  ਵਿਅੰਜਕ_List *pList,        /* List to which to add the span. */
  ਅਚਲ Token *pName,     /* Name to be added */
  ਪੂਰਨ dequote             /* True to cause the name to be dequoted */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 || pParse->db->mallocFailed!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->e_Parse_ਬਹੁਲਕ!=PARSE_ਬਹੁਲਕ_UNMAP || dequote==0 );
  ਜੇ( pList ){
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pList->n_ਵਿਅੰਜਕ>0 );
    p_ਵਸਤੂ = &pList->a[pList->n_ਵਿਅੰਜਕ-1];
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->zEName==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.eEName==ENAME_NAME );
    p_ਵਸਤੂ->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    ਜੇ( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(p_ਵਸਤੂ->zEName);
      ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ਅਚਲ ਖਾਲੀ*)p_ਵਸਤੂ->zEName, pName);
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
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_ਸੈੱਟ_ਵਿਸਤਾਰ(
  Parse *pParse,          /* Parsing context */
  ਵਿਅੰਜਕ_List *pList,        /* List to which to add the span. */
  ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ,     /* Start of the span */
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ        /* End of the span */
){
  sqlite3 *db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 || db->mallocFailed!=0 );
  ਜੇ( pList ){
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ = &pList->a[pList->n_ਵਿਅੰਜਕ-1];
    ਪ੍ਰਤਿਜ੍ਞਾ( pList->n_ਵਿਅੰਜਕ>0 );
    ਜੇ( p_ਵਸਤੂ->zEName==0 ){
      p_ਵਸਤੂ->zEName = sqlite3_Db_ਵਿਸਤਾਰ_Dup(db, z_ਸ਼ੁਰੂ, z_ਸਿਰਾ);
      p_ਵਸਤੂ->fg.eEName = ENAME_ਵਿਸਤਾਰ;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_Check_ਲੰਮਾਈ(
  Parse *pParse,
  ਵਿਅੰਜਕ_List *pEList,
  ਅਚਲ ਅੱਖਰ *zObject
){
  ਪੂਰਨ mx = pParse->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ];
  testcase( pEList && pEList->n_ਵਿਅੰਜਕ==mx );
  testcase( pEList && pEList->n_ਵਿਅੰਜਕ==mx+1 );
  ਜੇ( pEList && pEList->n_ਵਿਅੰਜਕ>mx ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ ਵਿਅੰਜਕ_List_Delete_NN(sqlite3 *db, ਵਿਅੰਜਕ_List *pList){
  ਪੂਰਨ i = pList->n_ਵਿਅੰਜਕ;
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ =  pList->a;
  ਪ੍ਰਤਿਜ੍ਞਾ( pList->n_ਵਿਅੰਜਕ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਕਰੋ{
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਸਤੂ->p_ਵਿਅੰਜਕ);
    ਜੇ( p_ਵਸਤੂ->zEName ) sqlite3DbNNFreeNN(db, p_ਵਸਤੂ->zEName);
    p_ਵਸਤੂ++;
  }ਜਦੋਂ( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_Delete(sqlite3 *db, ਵਿਅੰਜਕ_List *pList){
  ਜੇ( pList ) ਵਿਅੰਜਕ_List_Delete_NN(db, pList);
}
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_Delete_Generic(sqlite3 *db, ਖਾਲੀ *pList){
  ਜੇ( ALWAYS(pList) ) ਵਿਅੰਜਕ_List_Delete_NN(db, (ਵਿਅੰਜਕ_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_ਵਿਅੰਜਕ_List_Flags(ਅਚਲ ਵਿਅੰਜਕ_List *pList){
  ਪੂਰਨ i;
  u32 m = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
  ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
     ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pList->a[i].p_ਵਿਅੰਜਕ;
     ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
     m |= p_ਵਿਅੰਜਕ->flags;
  }
  ਵਾਪਸ m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
ਪੂਰਨ sqlite3SelectWalkFail(Walker *pWalker, Select *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  pWalker->e_ਕੋਡ = 0;
  ਵਾਪਸ WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ){
  ਜੇ( sqlite3StrICmp(z_ਵਿੱਚ, "true")==0  ) ਵਾਪਸ EP_IsTrue;
  ਜੇ( sqlite3StrICmp(z_ਵਿੱਚ, "false")==0 ) ਵਾਪਸ EP_IsFalse;
  ਵਾਪਸ 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Id_ਮੰਜ਼ਿਲ_True_False(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  u32 v;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_ID || p_ਵਿਅੰਜਕ->op==TK_STRING );
  ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Quoted|EP_Int_ਮੁੱਲ)
   && (v = sqlite3IsTrueOrFalse(p_ਵਿਅੰਜਕ->u.zToken))!=0
  ){
    p_ਵਿਅੰਜਕ->op = TK_TRUEFALSE;
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, v);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely((ਵਿਅੰਜਕ*)p_ਵਿਅੰਜਕ);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_TRUEFALSE );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3StrICmp(p_ਵਿਅੰਜਕ->u.zToken,"true")==0
       || sqlite3StrICmp(p_ਵਿਅੰਜਕ->u.zToken,"false")==0 );
  ਵਾਪਸ p_ਵਿਅੰਜਕ->u.zToken[4]==0;
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
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Simplified_And_Or(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_AND || p_ਵਿਅੰਜਕ->op==TK_OR ){
    ਵਿਅੰਜਕ *p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Simplified_And_Or(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
    ਵਿਅੰਜਕ *p_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_Simplified_And_Or(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
    ਜੇ( ਵਿਅੰਜਕ_Always_True(p_ਖੱਬਾ) || ਵਿਅੰਜਕ_Always_False(p_ਸੱਜਾ) ){
      p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->op==TK_AND ? p_ਸੱਜਾ : p_ਖੱਬਾ;
    }ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Always_True(p_ਸੱਜਾ) || ਵਿਅੰਜਕ_Always_False(p_ਖੱਬਾ) ){
      p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->op==TK_AND ? p_ਖੱਬਾ : p_ਸੱਜਾ;
    }
  }
  ਵਾਪਸ p_ਵਿਅੰਜਕ;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Eval_Rhs_ਪਹਿਲਾ(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, EP_Subquery)
   && !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, EP_Subquery)
  ){
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,       /* The comparison expression */
  ਪੂਰਨ *pR1,          /* OUT: Register holding the left operand */
  ਪੂਰਨ *pR2,          /* OUT: Register holding the right operand */
  ਪੂਰਨ *pFree1,       /* OUT: Temp register to free if not zero */
  ਪੂਰਨ *pFree2        /* OUT: Another temp register to free if not zero */
){
  ਪੂਰਨ addrIsNull;
  ਪੂਰਨ r1, r2;
  Vdbe *v = pParse->pVdbe;

  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  ਜੇ( ਵਿਅੰਜਕ_Eval_Rhs_ਪਹਿਲਾ(p_ਵਿਅੰਜਕ) && sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ) ){
    r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, pFree2);
    addrIsNull = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }ਨਹੀਂਤਾਂ{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, pFree1);
  ਜੇ( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, EP_Subquery)
     && sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ)
    ){
      addrIsNull = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  ਵਾਪਸ addrIsNull;
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ ਵਿਅੰਜਕ_ਗੰਢ_Is_Constant_Function(
  Walker *pWalker,
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ
){
  ਪੂਰਨ n;             /* Number of arguments */
  ਵਿਅੰਜਕ_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION );
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly)
   || (pList = p_ਵਿਅੰਜਕ->x.pList)==0
  ){;
    n = 0;
  }ਨਹੀਂਤਾਂ{
    n = pList->n_ਵਿਅੰਜਕ;
    sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, pList);
    ਜੇ( pWalker->e_ਕੋਡ==0 ) ਵਾਪਸ WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3_ਲੱਭੋ_Function(db, p_ਵਿਅੰਜਕ->u.zToken, n, ENC(db), 0);
  ਜੇ( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc))
  ){
    pWalker->e_ਕੋਡ = 0;
    ਵਾਪਸ WRC_Abort;
  }
  ਵਾਪਸ WRC_Prune;
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
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਗੰਢ_Is_Constant(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪ੍ਰਤਿਜ੍ਞਾ( pWalker->e_ਕੋਡ>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  ਜੇ( pWalker->e_ਕੋਡ==2 && ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) ){
    pWalker->e_ਕੋਡ = 0;
    ਵਾਪਸ WRC_Abort;
  }

  ਚੋਣ( p_ਵਿਅੰਜਕ->op ){
    ਹਾਲਤ TK_FUNCTION:
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
        pWalker->e_ਕੋਡ = 0;  /* Window functions are never constant */
        ਵਾਪਸ WRC_Abort;
      }ਨਹੀਂਤਾਂ ਜੇ( pWalker->e_ਕੋਡ>=4 && pWalker->e_ਕੋਡ<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        ਜੇ( pWalker->e_ਕੋਡ==5 ) ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_FromDDL);
        ਵਾਪਸ WRC_Continue;
      }ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        ਵਾਪਸ WRC_Continue;
      }ਨਹੀਂਤਾਂ ਜੇ( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        ਵਾਪਸ ਵਿਅੰਜਕ_ਗੰਢ_Is_Constant_Function(pWalker, p_ਵਿਅੰਜਕ);
      }ਨਹੀਂਤਾਂ{
        pWalker->e_ਕੋਡ = 0;  /* Function call is not constant */
        ਵਾਪਸ WRC_Abort;
      }
    ਹਾਲਤ TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Id_ਮੰਜ਼ਿਲ_True_False(p_ਵਿਅੰਜਕ) ){
        ਵਾਪਸ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    ਹਾਲਤ TK_ਕਾਲਮ:
    ਹਾਲਤ TK_AGG_FUNCTION:
    ਹਾਲਤ TK_AGG_ਕਾਲਮ:
      testcase( p_ਵਿਅੰਜਕ->op==TK_ID );
      testcase( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ );
      testcase( p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION );
      testcase( p_ਵਿਅੰਜਕ->op==TK_AGG_ਕਾਲਮ );
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Fixed_ਖਾਨਾ) && pWalker->e_ਕੋਡ!=2 ){
        ਵਾਪਸ WRC_Continue;
      }
      ਜੇ( pWalker->e_ਕੋਡ==3 && p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==pWalker->u.iCur ){
        ਵਾਪਸ WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    ਹਾਲਤ TK_IF_NULL_ਪੰਕਤੀ:
    ਹਾਲਤ TK_ਰਜਿਸਟਰ:
    ਹਾਲਤ TK_DOT:
    ਹਾਲਤ TK_RAISE:
      testcase( p_ਵਿਅੰਜਕ->op==TK_ਰਜਿਸਟਰ );
      testcase( p_ਵਿਅੰਜਕ->op==TK_IF_NULL_ਪੰਕਤੀ );
      testcase( p_ਵਿਅੰਜਕ->op==TK_DOT );
      testcase( p_ਵਿਅੰਜਕ->op==TK_RAISE );
      pWalker->e_ਕੋਡ = 0;
      ਵਾਪਸ WRC_Abort;
    ਹਾਲਤ TK_IS:
    ਹਾਲਤ TK_ISNOT:
      ਜੇ( pWalker->e_ਕੋਡ==6 ){
        pWalker->e_ਕੋਡ = 0;
        ਵਾਪਸ WRC_Abort;
      }
      ਵਾਪਸ WRC_Continue;
    ਹਾਲਤ TK_VARIABLE:
      ਜੇ( pWalker->e_ਕੋਡ==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_ਵਿਅੰਜਕ->op = TK_NULL;
      }ਨਹੀਂਤਾਂ ਜੇ( pWalker->e_ਕੋਡ==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->e_ਕੋਡ = 0;
        ਵਾਪਸ WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    ਮੂਲ:
      testcase( p_ਵਿਅੰਜਕ->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_ਵਿਅੰਜਕ->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      ਵਾਪਸ WRC_Continue;
  }
}
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Is_Const(Parse *pParse, ਵਿਅੰਜਕ *p, ਪੂਰਨ initFlag){
  Walker w;
  w.e_ਕੋਡ = initFlag;
  w.pParse = pParse;
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_ਗੰਢ_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#ਜੇ_ਅੰਤ
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p);
  ਵਾਪਸ w.e_ਕੋਡ;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Constant(Parse *pParse, ਵਿਅੰਜਕ *p){
  ਵਾਪਸ ਵਿਅੰਜਕ_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_List_Is_Constant(Parse *pParse, ਵਿਅੰਜਕ_List *pList, ਪੂਰਨ bNoIs){
  ਅਚਲ ਪੂਰਨ iInit = bNoIs ? 6 : 1;
  ਪੂਰਨ ii;
  ਲਈ(ii=0; ii<pList->n_ਵਿਅੰਜਕ; ii++){
    ਜੇ( 0==ਵਿਅੰਜਕ_Is_Const(pParse, pList->a[ii].p_ਵਿਅੰਜਕ, iInit) ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Constant_Not_Join(Parse *pParse, ਵਿਅੰਜਕ *p){
  ਵਾਪਸ ਵਿਅੰਜਕ_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Select_Walk_ਸਾਰਣੀ_Constant(Walker *pWalker, Select *pSelect){
  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pWalker->e_ਕੋਡ==3 || pWalker->e_ਕੋਡ==0 );
  ਜੇ( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->e_ਕੋਡ = 0;
    ਵਾਪਸ WRC_Abort;
  }
  ਵਾਪਸ WRC_Prune;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_ਸਾਰਣੀ_Constant(ਵਿਅੰਜਕ *p, ਪੂਰਨ iCur, ਪੂਰਨ bAllowSubq){
  Walker w;
  w.e_ਕੋਡ = 3;
  w.pParse = 0;
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_ਗੰਢ_Is_Constant;
  ਜੇ( bAllowSubq ){
    w.xSelectCallback = ਵਿਅੰਜਕ_Select_Walk_ਸਾਰਣੀ_Constant;
  }ਨਹੀਂਤਾਂ{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#ਜੇ_ਅੰਤ
  }
  w.u.iCur = iCur;
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p);
  ਵਾਪਸ w.e_ਕੋਡ;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Single_ਸਾਰਣੀ_Constraint(
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,                 /* The constraint */
  ਅਚਲ SrcList *pSrcList,     /* Complete FROM clause */
  ਪੂਰਨ iSrc,                    /* Which element of pSrcList to use */
  ਪੂਰਨ bAllowSubq               /* Allow non-correlated subqueries */
){
  ਅਚਲ Src_ਵਸਤੂ *pSrc = &pSrcList->a[iSrc];
  ਜੇ( pSrc->fg.jointype & JT_LTORJ ){
    ਵਾਪਸ 0;  /* rule (3) */
  }
  ਜੇ( pSrc->fg.jointype & JT_ਖੱਬਾ ){
    ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) ) ਵਾਪਸ 0;   /* rule (4a) */
    ਜੇ( p_ਵਿਅੰਜਕ->w.iJoin!=pSrc->iCursor ) ਵਾਪਸ 0;         /* rule (4b) */
  }ਨਹੀਂਤਾਂ{
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) ) ਵਾਪਸ 0;    /* rule (5) */
  }
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    ਪੂਰਨ jj;
    ਲਈ(jj=0; jj<iSrc; jj++){
      ਜੇ( p_ਵਿਅੰਜਕ->w.iJoin==pSrcList->a[jj].iCursor ){
        ਜੇ( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          ਵਾਪਸ 0;  /* restriction (6) */
        }
        ਰੋਕੋ;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_Is_ਸਾਰਣੀ_Constant(p_ਵਿਅੰਜਕ, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਗੰਢ_Is_Constant_Or_ਸਮੂਹ_By(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਵਿਅੰਜਕ_List *p_ਸਮੂਹ_By = pWalker->u.p_ਸਮੂਹ_By;
  ਪੂਰਨ i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  ਲਈ(i=0; i<p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ; i++){
    ਵਿਅੰਜਕ *p = p_ਸਮੂਹ_By->a[i].p_ਵਿਅੰਜਕ;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, p_ਵਿਅੰਜਕ, p, -1)<2 ){
      CollSeq *pColl = sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(pWalker->pParse, p);
      ਜੇ( sqlite3_Is_ਦੁਆਧਾਰੀ(pColl) ){
        ਵਾਪਸ WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
    pWalker->e_ਕੋਡ = 0;
    ਵਾਪਸ WRC_Abort;
  }

  ਵਾਪਸ ਵਿਅੰਜਕ_ਗੰਢ_Is_Constant(pWalker, p_ਵਿਅੰਜਕ);
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Constant_Or_ਸਮੂਹ_By(Parse *pParse, ਵਿਅੰਜਕ *p, ਵਿਅੰਜਕ_List *p_ਸਮੂਹ_By){
  Walker w;
  w.e_ਕੋਡ = 1;
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_ਗੰਢ_Is_Constant_Or_ਸਮੂਹ_By;
  w.xSelectCallback = 0;
  w.u.p_ਸਮੂਹ_By = p_ਸਮੂਹ_By;
  w.pParse = pParse;
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p);
  ਵਾਪਸ w.e_ਕੋਡ;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Constant_Or_Function(ਵਿਅੰਜਕ *p, u8 isInit){
  ਪ੍ਰਤਿਜ੍ਞਾ( isInit==0 || isInit==1 );
  ਵਾਪਸ ਵਿਅੰਜਕ_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਸ਼ਾਮਲ_ਹੈ_Subquery(ਵਿਅੰਜਕ *p){
  Walker w;
  w.e_ਕੋਡ = 1;
  w.x_ਵਿਅੰਜਕ_Callback = sqlite3_ਵਿਅੰਜਕ_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#ਜੇ_ਅੰਤ
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p);
  ਵਾਪਸ w.e_ਕੋਡ==0;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Integer(ਅਚਲ ਵਿਅੰਜਕ *p, ਪੂਰਨ *p_ਮੁੱਲ, Parse *pParse, ਪੂਰਨ bRSI){
  ਪੂਰਨ iSign = 1;   /* Either +1 or -1. */
  ਜਦੋਂ( 1/*exit-by-break*/ ){
    ਜੇ( NEVER(p==0) ) ਵਾਪਸ 0;
    ਜੇ( ਵਿਅੰਜਕ_Use_UValue(p) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->op==TK_INTEGER );
      *p_ਮੁੱਲ = p->u.i_ਮੁੱਲ*iSign;
      ਜੇ( bRSI && pParse==0 && p->op2 ){
        ਵਾਪਸ 0;
      }
      ਵਾਪਸ 1;
    }
    ਜੇ( p->op==TK_UPLUS ){
      p = p->p_ਖੱਬਾ;
      pParse = 0;
      ਜਾਰੀ;
    }
    ਜੇ( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_ਖੱਬਾ;
      pParse = 0;
      ਜਾਰੀ;
    }
    ਜੇ( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_ਮੁੱਲ *pVal; /* The variable */
      ਪੂਰਨ rc = 0;          /* 1 if successful, 0 if failed */
      ਪ੍ਰਤਿਜ੍ਞਾ( iSign==1 );
      ਜੇ( NEVER(pParse->pVdbe==0) ) ਰੋਕੋ;
      ਜੇ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ਰੋਕੋ;
      pVal = sqlite3_Vdbe_Get_Bound_ਮੁੱਲ(pParse->pReprepare, p->i_ਕਾਲਮ,
                                      SQLITE_AFF_BLOB);
      ਜੇ( pVal ){
        i64 vv;
        ਜੇ( sqlite3_ਮੁੱਲ_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_ਮੁੱਲ_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_ਮੁੱਲ = (ਪੂਰਨ)vv;
          rc = 1;       
        }
        sqlite3_ਮੁੱਲ_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->i_ਕਾਲਮ, bRSI);
      ਵਾਪਸ rc;
    }
    ਰੋਕੋ;
  }
  ਵਾਪਸ 0;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(ਅਚਲ ਵਿਅੰਜਕ *p){
  u8 op;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਜਦੋਂ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_ਖੱਬਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  }
  op = p->op;
  ਜੇ( op==TK_ਰਜਿਸਟਰ ) op = p->op2;
  ਚੋਣ( op ){
    ਹਾਲਤ TK_INTEGER:
    ਹਾਲਤ TK_STRING:
    ਹਾਲਤ TK_FLOAT:
    ਹਾਲਤ TK_BLOB:
      ਵਾਪਸ 0;
    ਹਾਲਤ TK_ਕਾਲਮ:
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p) );
      ਵਾਪਸ ਵਿਅੰਜਕ_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
          || (p->i_ਕਾਲਮ==XN_ROWID && IsView(p->y.pTab))
#ਜੇ_ਅੰਤ
          || (p->i_ਕਾਲਮ>=0
              && p->y.pTab->a_ਖਾਨਾ!=0 /* Possible due to prior error */
              && ALWAYS(p->i_ਕਾਲਮ<p->y.pTab->n_ਖਾਨਾ)
              && p->y.pTab->a_ਖਾਨਾ[p->i_ਕਾਲਮ].notNull==0);
    ਮੂਲ:
      ਵਾਪਸ 1;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Needs_No_Affinity_Change(ਅਚਲ ਵਿਅੰਜਕ *p, ਅੱਖਰ aff){
  u8 op;
  ਪੂਰਨ unaryMinus = 0;
  ਜੇ( aff==SQLITE_AFF_BLOB ) ਵਾਪਸ 1;
  ਜਦੋਂ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    ਜੇ( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_ਖੱਬਾ;
  }
  op = p->op;
  ਜੇ( op==TK_ਰਜਿਸਟਰ ) op = p->op2;
  ਚੋਣ( op ){
    ਹਾਲਤ TK_INTEGER: {
      ਵਾਪਸ aff>=SQLITE_AFF_NUMERIC;
    }
    ਹਾਲਤ TK_FLOAT: {
      ਵਾਪਸ aff>=SQLITE_AFF_NUMERIC;
    }
    ਹਾਲਤ TK_STRING: {
      ਵਾਪਸ !unaryMinus && aff==SQLITE_AFF_ਪਾਠ;
    }
    ਹਾਲਤ TK_BLOB: {
      ਵਾਪਸ !unaryMinus;
    }
    ਹਾਲਤ TK_ਕਾਲਮ: {
      ਪ੍ਰਤਿਜ੍ਞਾ( p->i_ਸਾਰਣੀ>=0 );  /* p cannot be part of a CHECK constraint */
      ਵਾਪਸ aff>=SQLITE_AFF_NUMERIC && p->i_ਕਾਲਮ<0;
    }
    ਮੂਲ: {
      ਵਾਪਸ 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
ਪੂਰਨ sqlite3IsRowid(ਅਚਲ ਅੱਖਰ *z){
  ਜੇ( sqlite3StrICmp(z, "_ROWID_")==0 ) ਵਾਪਸ 1;
  ਜੇ( sqlite3StrICmp(z, "ROWID")==0 ) ਵਾਪਸ 1;
  ਜੇ( sqlite3StrICmp(z, "OID")==0 ) ਵਾਪਸ 1;
  ਵਾਪਸ 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ਅਚਲ ਅੱਖਰ *sqlite3RowidAlias(ਸਾਰਣੀ *pTab){
  ਅਚਲ ਅੱਖਰ *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  ਪੂਰਨ ii;
  ਪ੍ਰਤਿਜ੍ਞਾ( VisibleRowid(pTab) );
  ਲਈ(ii=0; ii<Array_ਮਾਪ(azOpt); ii++){
    ਜੇ( sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, azOpt[ii])<0 ) ਵਾਪਸ azOpt[ii];
  }
  ਵਾਪਸ 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
ਸਥਿਰ Select *is_Candidate_For_ਵਿੱਚ_Opt(ਅਚਲ ਵਿਅੰਜਕ *pX){
  Select *p;
  SrcList *pSrc;
  ਵਿਅੰਜਕ_List *pEList;
  ਸਾਰਣੀ *pTab;
  ਪੂਰਨ i;
  ਜੇ( !ਵਿਅੰਜਕ_Use_XSelect(pX) ) ਵਾਪਸ 0;                 /* Not a subquery */
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(pX, EP_VarSelect)  ) ਵਾਪਸ 0;  /* Correlated subq */
  p = pX->x.pSelect;
  ਜੇ( p->p_ਪੂਰਵ ) ਵਾਪਸ 0;              /* Not a compound SELECT */
  ਜੇ( p->selFlags & SF_Aggregate ){
    ਵਾਪਸ 0; /* No GROUP BY keyword or aggregate functions */
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਸਮੂਹ_By==0 );              /* Has no GROUP BY clause */
  ਜੇ( p->p_ਹੱਦਬੰਦੀ ) ਵਾਪਸ 0;              /* Has no LIMIT clause */
  ਜੇ( p->pWhere ) ਵਾਪਸ 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc!=0 );
  ਜੇ( pSrc->nSrc!=1 ) ਵਾਪਸ 0;          /* Single term in FROM clause */
  ਜੇ( pSrc->a[0].fg.isSubquery) ਵਾਪਸ 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsView(pTab)  );              /* FROM clause is not a view */
  ਜੇ( IsVirtual(pTab) ) ਵਾਪਸ 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  ਪ੍ਰਤਿਜ੍ਞਾ( pEList!=0 );
  /* All SELECT results must be columns. */
  ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
    ਵਿਅੰਜਕ *pRes = pEList->a[i].p_ਵਿਅੰਜਕ;
    ਜੇ( pRes->op!=TK_ਕਾਲਮ ) ਵਾਪਸ 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( pRes->i_ਸਾਰਣੀ==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  ਵਾਪਸ p;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SUBQUERY */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3_ਸੈੱਟ_Has_Null_Flag(
  Vdbe *v,           /* Write new code into this statement under construction */
  ਪੂਰਨ iCur,          /* Cursor for the index */
  ਪੂਰਨ regHasNull,    /* Register in which to store hasNull flag */
  ਪੂਰਨ e_ਛਾਂਟੋ_ਕ੍ਰਮ     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  ਪੂਰਨ addr1;
  ਪੂਰਨ op;
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, regHasNull);
  ਜੇ( e_ਛਾਂਟੋ_ਕ੍ਰਮ==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }ਨਹੀਂਤਾਂ{
    op = OP_ਆਖਰੀ;
  }
  addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, op, iCur); VdbeCoverage(v);
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
  VdbeComment((v, op==OP_ਆਖਰੀ?"last_entry_in(%d)":"first_entry_in(%d)", iCur));
  sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
}
#ਜੇ_ਅੰਤ


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
/*
** The argument is an IN operator with a list (not a subquery) on the
** right-hand side.  Return TRUE if that list is constant.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3_ਵਿੱਚ_Rhs_Is_Constant(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿੱਚ){
  ਵਿਅੰਜਕ *pLHS;
  ਪੂਰਨ res;
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿੱਚ, EP_xIsSelect) );
  pLHS = p_ਵਿੱਚ->p_ਖੱਬਾ;
  p_ਵਿੱਚ->p_ਖੱਬਾ = 0;
  res = sqlite3_ਵਿਅੰਜਕ_Is_Constant(pParse, p_ਵਿੱਚ);
  p_ਵਿੱਚ->p_ਖੱਬਾ = pLHS;
  ਵਾਪਸ res;
}
#ਜੇ_ਅੰਤ

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
ਪੂਰਨ sqlite3_ਲੱਭੋ_ਵਿੱਚ_ਸੂਚਕਾਂਕ(
  Parse *pParse,             /* Parsing context */
  ਵਿਅੰਜਕ *pX,                  /* The IN expression */
  u32 ਵਿੱਚ_Flags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  ਪੂਰਨ *prRhsHasNull,         /* Register holding NULL status.  See notes */
  ਪੂਰਨ *aiMap,                /* Mapping from Index fields to RHS fields */
  ਪੂਰਨ *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  ਪੂਰਨ eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  ਪੂਰਨ iTab;                             /* Cursor of the RHS table */
  ਪੂਰਨ mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  ਪ੍ਰਤਿਜ੍ਞਾ( pX->op==TK_ਵਿੱਚ );
  mustBeUnique = (ਵਿੱਚ_Flags & ਵਿੱਚ_ਸੂਚਕਾਂਕ_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  ਜੇ( prRhsHasNull && ਵਿਅੰਜਕ_Use_XSelect(pX) ){
    ਪੂਰਨ i;
    ਵਿਅੰਜਕ_List *pEList = pX->x.pSelect->pEList;
    ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(pEList->a[i].p_ਵਿਅੰਜਕ) ) ਰੋਕੋ;
    }
    ਜੇ( i==pEList->n_ਵਿਅੰਜਕ ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  ਜੇ( pParse->nErr==0 && (p = is_Candidate_For_ਵਿੱਚ_Opt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    ਸਾਰਣੀ *pTab;                           /* Table <table>. */
    ਪੂਰਨ iDb;                               /* Database idx for pTab */
    ਵਿਅੰਜਕ_List *pEList = p->pEList;
    ਪੂਰਨ n_ਵਿਅੰਜਕ = pEList->n_ਵਿਅੰਜਕ;

    ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList->a[0].p_ਵਿਅੰਜਕ!=0 ); /* Because of isCandidateForInOpt(p) */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
    ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3_ਕੋਡ_Verify_Schema(pParse, iDb);
    sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, pTab->tnum, 0, pTab->zName);

    ਪ੍ਰਤਿਜ੍ਞਾ(v);  /* sqlite3GetVdbe() has always been previously called */
    ਜੇ( n_ਵਿਅੰਜਕ==1 && pEList->a[0].p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      ਪੂਰਨ iAddr = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3_Open_ਸਾਰਣੀ(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = ਵਿੱਚ_ਸੂਚਕਾਂਕ_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, iAddr);
    }ਨਹੀਂਤਾਂ{
      ਸੂਚਕਾਂਕ *pIdx;                         /* Iterator variable */
      ਪੂਰਨ affinity_ok = 1;
      ਪੂਰਨ i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      ਲਈ(i=0; i<n_ਵਿਅੰਜਕ && affinity_ok; i++){
        ਵਿਅੰਜਕ *pLhs = sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(pX->p_ਖੱਬਾ, i);
        ਪੂਰਨ i_ਖਾਨਾ = pEList->a[i].p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
        ਅੱਖਰ idxaff = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_Affinity(pTab,i_ਖਾਨਾ); /* RHS table */
        ਅੱਖਰ cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_ਪਾਠ );
        ਚੋਣ( cmpaff ){
          ਹਾਲਤ SQLITE_AFF_BLOB:
            ਰੋਕੋ;
          ਹਾਲਤ SQLITE_AFF_ਪਾਠ:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            ਪ੍ਰਤਿਜ੍ਞਾ( idxaff==SQLITE_AFF_ਪਾਠ );
            ਰੋਕੋ;
          ਮੂਲ:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      ਜੇ( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx && eType==0; pIdx=pIdx->p_ਅਗਲਾ){
          Bitmask ਖਾਨਾ_ਵਰਤਿਆ;      /* Columns of the index used */
          Bitmask m_ਖਾਨਾ;         /* Mask for the current column */
          ਜੇ( pIdx->n_ਕਾਲਮ<n_ਵਿਅੰਜਕ ) ਜਾਰੀ;
          ਜੇ( pIdx->pPartIdxWhere!=0 ) ਜਾਰੀ;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->n_ਕਾਲਮ==BMS-2 );
          testcase( pIdx->n_ਕਾਲਮ==BMS-1 );
          ਜੇ( pIdx->n_ਕਾਲਮ>=BMS-1 ) ਜਾਰੀ;
          ਜੇ( mustBeUnique ){
            ਜੇ( pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ>n_ਵਿਅੰਜਕ
             ||(pIdx->n_ਕਾਲਮ>n_ਵਿਅੰਜਕ && !Is_Unique_ਸੂਚਕਾਂਕ(pIdx))
            ){
              ਜਾਰੀ;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          ਖਾਨਾ_ਵਰਤਿਆ = 0;   /* Columns of index used so far */
          ਲਈ(i=0; i<n_ਵਿਅੰਜਕ; i++){
            ਵਿਅੰਜਕ *pLhs = sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(pX->p_ਖੱਬਾ, i);
            ਵਿਅੰਜਕ *pRhs = pEList->a[i].p_ਵਿਅੰਜਕ;
            CollSeq *pReq = sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, pLhs, pRhs);
            ਪੂਰਨ j;
 
            ਲਈ(j=0; j<n_ਵਿਅੰਜਕ; j++){
              ਜੇ( pIdx->ai_ਕਾਲਮ[j]!=pRhs->i_ਕਾਲਮ ) ਜਾਰੀ;
              ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->azColl[j] );
              ਜੇ( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                ਜਾਰੀ;
              }
              ਰੋਕੋ;
            }
            ਜੇ( j==n_ਵਿਅੰਜਕ ) ਰੋਕੋ;
            m_ਖਾਨਾ = MASKBIT(j);
            ਜੇ( m_ਖਾਨਾ & ਖਾਨਾ_ਵਰਤਿਆ ) ਰੋਕੋ; /* Each column used only once */
            ਖਾਨਾ_ਵਰਤਿਆ |= m_ਖਾਨਾ;
            ਜੇ( aiMap ) aiMap[i] = j;
          }
 
          ਪ੍ਰਤਿਜ੍ਞਾ( n_ਵਿਅੰਜਕ>0 && n_ਵਿਅੰਜਕ<BMS );
          ਪ੍ਰਤਿਜ੍ਞਾ( i==n_ਵਿਅੰਜਕ || ਖਾਨਾ_ਵਰਤਿਆ!=(MASKBIT(n_ਵਿਅੰਜਕ)-1) );
          ਜੇ( ਖਾਨਾ_ਵਰਤਿਆ==(MASKBIT(n_ਵਿਅੰਜਕ)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            ਪੂਰਨ iAddr = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_DESC == ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_ASC+1 );
            eType = ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_ASC + pIdx->a_ਛਾਂਟੋ_ਕ੍ਰਮ[0];
 
            ਜੇ( prRhsHasNull ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_ਵਰਤਿਆ_ਨਕਾਬ
              u64 ਨਕਾਬ = (1<<n_ਵਿਅੰਜਕ)-1;
              sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Columns_ਵਰਤਿਆ,
                                iTab, LOWER32(ਨਕਾਬ), UPPER32(ਨਕਾਬ));
#ਜੇ_ਅੰਤ
              *prRhsHasNull = ++pParse->nMem;
              ਜੇ( n_ਵਿਅੰਜਕ==1 ){
                sqlite3_ਸੈੱਟ_Has_Null_Flag(v, iTab, *prRhsHasNull,
                                      pIdx->a_ਛਾਂਟੋ_ਕ੍ਰਮ[0]);
              }
            }
            sqlite3_Vdbe_Jump_ਇੱਥੇ(v, iAddr);
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
  ਜੇ( eType==0
   && (ਵਿੱਚ_Flags & ਵਿੱਚ_ਸੂਚਕਾਂਕ_NOOP_OK)
   && ਵਿਅੰਜਕ_Use_XList(pX)
   && (!sqlite3_ਵਿੱਚ_Rhs_Is_Constant(pParse,pX) || pX->x.pList->n_ਵਿਅੰਜਕ<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = ਵਿੱਚ_ਸੂਚਕਾਂਕ_NOOP;
  }

  ਜੇ( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    ਪੂਰਨ rMayHaveNull = 0;
    ਪੂਰਨ bloomOk = (ਵਿੱਚ_Flags & ਵਿੱਚ_ਸੂਚਕਾਂਕ_MEMBERSHIP)!=0;
    eType = ਵਿੱਚ_ਸੂਚਕਾਂਕ_EPH;
    ਜੇ( ਵਿੱਚ_Flags & ਵਿੱਚ_ਸੂਚਕਾਂਕ_LOOP ){
      pParse->nQueryLoop = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pX->op==TK_ਵਿੱਚ );
    ਜੇ( !bloomOk
     && ਵਿਅੰਜਕ_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_Cloned_Rhs_ਵਿੱਚ)!=0
    ){
      bloomOk = 1;
    }
    sqlite3_ਕੋਡ_Rhs_ਦਾ_ਵਿੱਚ(pParse, pX, iTab, bloomOk);
    ਜੇ( rMayHaveNull ){
      sqlite3_ਸੈੱਟ_Has_Null_Flag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  ਜੇ( aiMap && eType!=ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_ASC && eType!=ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_DESC ){
    ਪੂਰਨ i, n;
    n = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(pX->p_ਖੱਬਾ);
    ਲਈ(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  ਵਾਪਸ eType;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
/*
** Argument pExpr is an (?, ?...) IN(...) expression. This
** function allocates and returns a nul-terminated string containing
** the affinities to be used for each column of the comparison.
**
** It is the responsibility of the caller to ensure that the returned
** string is eventually freed using sqlite3DbFree().
*/
ਸਥਿਰ ਅੱਖਰ *ਵਿਅੰਜਕ_INAffinity(Parse *pParse, ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
  ਪੂਰਨ nVal = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਖੱਬਾ);
  Select *pSelect = ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ? p_ਵਿਅੰਜਕ->x.pSelect : 0;
  ਅੱਖਰ *zRet;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_ਵਿੱਚ );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  ਜੇ( zRet ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<nVal; i++){
      ਵਿਅੰਜਕ *pA = sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(p_ਖੱਬਾ, i);
      ਅੱਖਰ a = sqlite3_ਵਿਅੰਜਕ_Affinity(pA);
      ਜੇ( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_ਵਿਅੰਜਕ, a);
      }ਨਹੀਂਤਾਂ{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  ਵਾਪਸ zRet;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
ਖਾਲੀ sqlite3SubselectError(Parse *pParse, ਪੂਰਨ nActual, ਪੂਰਨ nExpect){
  ਜੇ( pParse->nErr==0 ){
    ਅਚਲ ਅੱਖਰ *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, zFmt, nActual, nExpect);
  }
}
#ਜੇ_ਅੰਤ

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
ਖਾਲੀ sqlite3_ਸਦਿਸ਼_Error_ਸੁਨੇਹਾ(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
  ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
    sqlite3SubselectError(pParse, p_ਵਿਅੰਜਕ->x.pSelect->pEList->n_ਵਿਅੰਜਕ, 1);
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  {
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "row value misused");
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_Compatible_ਵਿੱਚ_Rhs_Subrtn(
  Parse *pParse,          /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *p_ਸਿਰਾ;
  SubrtnSig *pSig;
  Vdbe *v;

  ਜੇ( pNewSig==0 ) ਵਾਪਸ 0;
  ਜੇ( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_ਵਿੱਚ );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_YSub(p_ਵਿਅੰਜਕ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pSelect!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਵਿਅੰਜਕ->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  p_ਸਿਰਾ = sqlite3_Vdbe_Get_ਆਖਰੀ_Op(v);
  ਲਈ(; pOp<p_ਸਿਰਾ; pOp++){
    ਜੇ( pOp->p4type!=P4_SUBRTNSIG ) ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    ਪ੍ਰਤਿਜ੍ਞਾ( pSig!=0 );
    ਜੇ( !pSig->bComplete ) ਜਾਰੀ;
    ਜੇ( pNewSig->selId!=pSig->selId ) ਜਾਰੀ;
    ਜੇ( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) ਜਾਰੀ;
    p_ਵਿਅੰਜਕ->y.sub.iAddr = pSig->iAddr;
    p_ਵਿਅੰਜਕ->y.sub.regReturn = pSig->regReturn;
    p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = pSig->i_ਸਾਰਣੀ;
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_Subrtn);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SUBQUERY */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
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
ਖਾਲੀ sqlite3_ਕੋਡ_Rhs_ਦਾ_ਵਿੱਚ(
  Parse *pParse,          /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,            /* The IN operator */
  ਪੂਰਨ iTab,               /* Use this cursor number */
  ਪੂਰਨ allowBloom          /* True to allow the use of a Bloom filter */
){
  ਪੂਰਨ addrOnce = 0;           /* Address of the OP_Once instruction at top */
  ਪੂਰਨ addr;                   /* Address of OP_OpenEphemeral instruction */
  ਵਿਅੰਜਕ *p_ਖੱਬਾ;                /* the LHS of the IN operator */
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info = 0;      /* Key information */
  ਪੂਰਨ nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );

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
  ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) || p_ਵਿਅੰਜਕ->x.pSelect!=0 );
    ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) && (p_ਵਿਅੰਜਕ->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, ਆਕਾਰ(pSig[0]));
      ਜੇ( pSig ){
        pSig->selId = p_ਵਿਅੰਜਕ->x.pSelect->selId;
        pSig->zAff = ਵਿਅੰਜਕ_INAffinity(pParse, p_ਵਿਅੰਜਕ);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subrtn) 
     || ਲੱਭੋ_Compatible_ਵਿੱਚ_Rhs_Subrtn(pParse, p_ਵਿਅੰਜਕ, pSig)
    ){
      addrOnce = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
      ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_ਵਿਅੰਜਕ->x.pSelect->selId));
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YSub(p_ਵਿਅੰਜਕ) );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, p_ਵਿਅੰਜਕ->y.sub.regReturn,
                        p_ਵਿਅੰਜਕ->y.sub.iAddr);
      ਪ੍ਰਤਿਜ੍ਞਾ( iTab!=p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenDup, iTab, p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrOnce);
      ਜੇ( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      ਵਾਪਸ;
    }

    /* Begin coding the subroutine */
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_YWin(p_ਵਿਅੰਜਕ) );
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_Subrtn);
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly|EP_Reduced) );
    p_ਵਿਅੰਜਕ->y.sub.regReturn = ++pParse->nMem;
    p_ਵਿਅੰਜਕ->y.sub.iAddr =
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_BeginSubrtn, 0, p_ਵਿਅੰਜਕ->y.sub.regReturn) + 1;
    ਜੇ( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_ਵਿਅੰਜਕ->y.sub.iAddr;
      pSig->regReturn = p_ਵਿਅੰਜਕ->y.sub.regReturn;
      pSig->i_ਸਾਰਣੀ = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ਅਚਲ ਅੱਖਰ*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
  nVal = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਖੱਬਾ);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = iTab;
  addr = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ, nVal);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
  ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
    VdbeComment((v, "Result of SELECT %u", p_ਵਿਅੰਜਕ->x.pSelect->selId));
  }ਨਹੀਂਤਾਂ{
    VdbeComment((v, "RHS of IN operator"));
  }
#ਜੇ_ਅੰਤ
  p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_Alloc(pParse->db, nVal, 1);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Info!=0 || pParse->nErr );
  ਜੇ( p_ਕੁੰਜੀ_Info==0 ) ਵਾਪਸ;

  ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_ਵਿਅੰਜਕ->x.pSelect;
    ਵਿਅੰਜਕ_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    ਜੇ( ALWAYS(pEList->n_ਵਿਅੰਜਕ==nVal) ){
      Select *pCopy;
      SelectDest dest;
      ਪੂਰਨ i;
      ਪੂਰਨ rc;
      ਪੂਰਨ addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_ਸੈੱਟ, iTab);
      dest.zAffSdst = ਵਿਅੰਜਕ_INAffinity(pParse, p_ਵਿਅੰਜਕ);
      pSelect->i_ਹੱਦਬੰਦੀ = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( pEList!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pEList->n_ਵਿਅੰਜਕ>0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਕੁੰਜੀ_Info_Is_Writeable(p_ਕੁੰਜੀ_Info) );
      ਲਈ(i=0; i<nVal; i++){
        ਵਿਅੰਜਕ *p = sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(p_ਖੱਬਾ, i);
        CollSeq *pColl;
        p_ਕੁੰਜੀ_Info->aColl[i] = pColl = sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_ਵਿਅੰਜਕ
        );
        ਜੇ( !sqlite3_Is_ਦੁਆਧਾਰੀ(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      ਜੇ( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        ਪੂਰਨ regBloom = ++pParse->nMem;
        addrBloom = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (ਖਾਲੀ *)p_ਕੁੰਜੀ_Info, P4_KEYINFO);
        p_ਕੁੰਜੀ_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      ਜੇ( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        ਜੇ( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      ਜੇ( rc ){
        sqlite3_ਕੁੰਜੀ_Info_Unref(p_ਕੁੰਜੀ_Info);
        ਵਾਪਸ;
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( ALWAYS(p_ਵਿਅੰਜਕ->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    ਅੱਖਰ affinity;            /* Affinity of the LHS of the IN */
    ਪੂਰਨ i;
    ਵਿਅੰਜਕ_List *pList = p_ਵਿਅੰਜਕ->x.pList;
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
    ਪੂਰਨ r1, r2;
    affinity = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਖੱਬਾ);
    ਜੇ( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }ਨਹੀਂਤਾਂ ਜੇ( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Info!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਕੁੰਜੀ_Info_Is_Writeable(p_ਕੁੰਜੀ_Info) );
    p_ਕੁੰਜੀ_Info->aColl[0] = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
    r2 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
    ਲਈ(i=pList->n_ਵਿਅੰਜਕ, p_ਵਸਤੂ=pList->a; i>0; i--, p_ਵਸਤੂ++){
      ਵਿਅੰਜਕ *pE2 = p_ਵਸਤੂ->p_ਵਿਅੰਜਕ;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      ਜੇ( addrOnce && !sqlite3_ਵਿਅੰਜਕ_Is_Constant(pParse, pE2) ){
        sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, addrOnce-1);
        sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, addrOnce);
        ਵਿਅੰਜਕ_Clear_Property(p_ਵਿਅੰਜਕ, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pE2, r1);
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Make_ਰਿਕਾਰਡ, r1, 1, r2, &affinity, 1);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iTab, r2, r1, 1);
    }
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r2);
  }
  ਜੇ( pSig ) pSig->bComplete = 1;
  ਜੇ( p_ਕੁੰਜੀ_Info ){
    sqlite3VdbeChangeP4(v, addr, (ਖਾਲੀ *)p_ਕੁੰਜੀ_Info, P4_KEYINFO);
  }
  ਜੇ( addrOnce ){
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, iTab);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrOnce);
    /* Subroutine return */
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YSub(p_ਵਿਅੰਜਕ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeGetOp(v,p_ਵਿਅੰਜਕ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Return, p_ਵਿਅੰਜਕ->y.sub.regReturn,
                      p_ਵਿਅੰਜਕ->y.sub.iAddr, 1);
    VdbeCoverage(v);
    sqlite3_Clear_ਆਰਜ਼ੀ_Reg_Cache(pParse);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SUBQUERY */

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
ਪੂਰਨ sqlite3_ਕੋਡ_Subselect(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  ਪੂਰਨ rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  ਪੂਰਨ nReg;                   /* Registers to allocate */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ;               /* New limit expression */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  ਪੂਰਨ addrExplain;            /* Address of OP_Explain instruction */
#ਜੇ_ਅੰਤ

  Vdbe *v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਜੇ( pParse->nErr ) ਵਾਪਸ 0;
  testcase( p_ਵਿਅੰਜਕ->op==TK_EXISTS );
  testcase( p_ਵਿਅੰਜਕ->op==TK_SELECT );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_EXISTS || p_ਵਿਅੰਜਕ->op==TK_SELECT );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
  pSel = p_ਵਿਅੰਜਕ->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YSub(p_ਵਿਅੰਜਕ) );
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, p_ਵਿਅੰਜਕ->y.sub.regReturn,
                      p_ਵਿਅੰਜਕ->y.sub.iAddr);
    ਵਾਪਸ p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;
  }

  /* Begin coding the subroutine */
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Use_YWin(p_ਵਿਅੰਜਕ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Reduced|EP_TokenOnly) );
  ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_Subrtn);
  p_ਵਿਅੰਜਕ->y.sub.regReturn = ++pParse->nMem;
  p_ਵਿਅੰਜਕ->y.sub.iAddr =
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_BeginSubrtn, 0, p_ਵਿਅੰਜਕ->y.sub.regReturn) + 1;

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
  ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_VarSelect) ){
    addrOnce = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
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
  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Counters(v, addrExplain, addrExplain, -1);
  nReg = p_ਵਿਅੰਜਕ->op==TK_SELECT ? pSel->pEList->n_ਵਿਅੰਜਕ : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    ਜੇ( (pSel->selFlags&SF_Distinct) && pSel->p_ਹੱਦਬੰਦੀ && pSel->p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ ){
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
    }ਨਹੀਂਤਾਂ{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }ਨਹੀਂਤਾਂ{
    dest.eDest = SRT_Exists;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  ਜੇ( pSel->p_ਹੱਦਬੰਦੀ ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    ਵਿਅੰਜਕ *p_ਖੱਬਾ = pSel->p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ;
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਖੱਬਾ, EP_Int_ਮੁੱਲ)==0
     || (p_ਖੱਬਾ->u.i_ਮੁੱਲ!=1 && p_ਖੱਬਾ->u.i_ਮੁੱਲ!=0)
    ){
      sqlite3 *db = pParse->db;
      p_ਹੱਦਬੰਦੀ = sqlite3_ਵਿਅੰਜਕ_Int32(db, 0);
      ਜੇ( p_ਹੱਦਬੰਦੀ ){
        p_ਹੱਦਬੰਦੀ->aff_ਵਿਅੰਜਕ = SQLITE_AFF_NUMERIC;
        p_ਹੱਦਬੰਦੀ = sqlite3PExpr(pParse, TK_NE,
            sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਖੱਬਾ, 0), p_ਹੱਦਬੰਦੀ);
      }
      sqlite3_ਵਿਅੰਜਕ_Deferred_Delete(pParse, p_ਖੱਬਾ);
      pSel->p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ = p_ਹੱਦਬੰਦੀ;
    }
  }ਨਹੀਂਤਾਂ{
    /* If there is no pre-existing limit add a limit of 1 */
    p_ਹੱਦਬੰਦੀ = sqlite3_ਵਿਅੰਜਕ_Int32(pParse->db, 1);
    pSel->p_ਹੱਦਬੰਦੀ = sqlite3PExpr(pParse, TK_ਹੱਦਬੰਦੀ, p_ਹੱਦਬੰਦੀ, 0);
  }
  pSel->i_ਹੱਦਬੰਦੀ = 0;
  ਜੇ( sqlite3Select(pParse, pSel, &dest) ){
    p_ਵਿਅੰਜਕ->op2 = p_ਵਿਅੰਜਕ->op;
    p_ਵਿਅੰਜਕ->op = TK_ERROR;
    ਵਾਪਸ 0;
  }
  p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = rReg = dest.iSDParm;
  ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(p_ਵਿਅੰਜਕ, EP_NoReduce);
  ਜੇ( addrOnce ){
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrOnce);
  }
  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExplain, addrExplain, -1);

  ਜੇ( dest.eDest==SRT_Mem 
   && sqlite3_ਵਿਅੰਜਕ_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_ਵਿਅੰਜਕ) 
  ){
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YSub(p_ਵਿਅੰਜਕ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeGetOp(v,p_ਵਿਅੰਜਕ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Return, p_ਵਿਅੰਜਕ->y.sub.regReturn,
                    p_ਵਿਅੰਜਕ->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3_Clear_ਆਰਜ਼ੀ_Reg_Cache(pParse);
  ਵਾਪਸ rReg;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SUBQUERY */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Check_ਵਿੱਚ(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿੱਚ){
  ਪੂਰਨ n_ਸਦਿਸ਼ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿੱਚ->p_ਖੱਬਾ);
  ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿੱਚ) && !pParse->db->mallocFailed ){
    ਜੇ( n_ਸਦਿਸ਼!=p_ਵਿੱਚ->x.pSelect->pEList->n_ਵਿਅੰਜਕ ){
      sqlite3SubselectError(pParse, p_ਵਿੱਚ->x.pSelect->pEList->n_ਵਿਅੰਜਕ, n_ਸਦਿਸ਼);
      ਵਾਪਸ 1;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( n_ਸਦਿਸ਼!=1 ){
    sqlite3_ਸਦਿਸ਼_Error_ਸੁਨੇਹਾ(pParse, p_ਵਿੱਚ->p_ਖੱਬਾ);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
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
ਸਥਿਰ ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ(
  Parse *pParse,        /* Parsing and code generating context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,          /* The IN expression */
  ਪੂਰਨ destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  ਪੂਰਨ destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  ਪੂਰਨ rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  ਪੂਰਨ eType;            /* Type of the RHS */
  ਪੂਰਨ rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  ਪੂਰਨ *aiMap = 0;       /* Map from vector field to index column */
  ਅੱਖਰ *zAff = 0;       /* Affinity string for comparisons */
  ਪੂਰਨ n_ਸਦਿਸ਼;          /* Size of vectors for this IN operator */
  ਪੂਰਨ iDummy;           /* Dummy parameter to exprCodeVector() */
  ਵਿਅੰਜਕ *p_ਖੱਬਾ;          /* The LHS of the IN operator */
  ਪੂਰਨ i;                /* loop counter */
  ਪੂਰਨ destStep2;        /* Where to jump when NULLs seen in step 2 */
  ਪੂਰਨ destStep6 = 0;    /* Start of code for Step 6 */
  ਪੂਰਨ addrTruthOp;      /* Address of opcode that determines the IN is true */
  ਪੂਰਨ destNotNull;      /* Jump here if a comparison is not true in step 6 */
  ਪੂਰਨ addr_ਸਿਖਰ;          /* Top of the step-6 loop */
  ਪੂਰਨ iTab = 0;         /* Index to use */
  u8 ok_Const_ਗੁਣਕ = pParse->ok_Const_ਗੁਣਕ;

  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_VVAProperty(p_ਵਿਅੰਜਕ,EP_Immutable) );
  p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Check_ਵਿੱਚ(pParse, p_ਵਿਅੰਜਕ) ) ਵਾਪਸ;
  zAff = ਵਿਅੰਜਕ_INAffinity(pParse, p_ਵਿਅੰਜਕ);
  n_ਸਦਿਸ਼ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
  aiMap = (ਪੂਰਨ*)sqlite3DbMallocZero(pParse->db, n_ਸਦਿਸ਼*ਆਕਾਰ(ਪੂਰਨ));
  ਜੇ( pParse->db->mallocFailed ) ਜਾਓ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3_ਲੱਭੋ_ਵਿੱਚ_ਸੂਚਕਾਂਕ(pParse, p_ਵਿਅੰਜਕ,
                             ਵਿੱਚ_ਸੂਚਕਾਂਕ_MEMBERSHIP | ਵਿੱਚ_ਸੂਚਕਾਂਕ_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nErr || n_ਸਦਿਸ਼==1 || eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_EPH
       || eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_ASC || eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_DESC
  );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  ਲਈ(i=0; i<n_ਸਦਿਸ਼; i++){
    ਪੂਰਨ j, cnt;
    ਲਈ(cnt=j=0; j<n_ਸਦਿਸ਼; j++) ਜੇ( aiMap[j]==i ) cnt++;
    ਪ੍ਰਤਿਜ੍ਞਾ( cnt==1 );
  }
#ਜੇ_ਅੰਤ

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
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->ok_Const_ਗੁਣਕ==ok_Const_ਗੁਣਕ );
  pParse->ok_Const_ਗੁਣਕ = 0;
  rLhs = ਵਿਅੰਜਕ_ਕੋਡ_ਸਦਿਸ਼(pParse, p_ਖੱਬਾ, &iDummy);
  pParse->ok_Const_ਗੁਣਕ = ok_Const_ਗੁਣਕ;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  ਜੇ( eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_NOOP ){
    ਵਿਅੰਜਕ_List *pList;
    CollSeq *pColl;
    ਪੂਰਨ labelOk = sqlite3VdbeMakeLabel(pParse);
    ਪੂਰਨ r2, reg_ਮੰਜ਼ਿਲ_Free;
    ਪੂਰਨ regCkNull = 0;
    ਪੂਰਨ ii;
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਸਦਿਸ਼==1 );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
    pList = p_ਵਿਅੰਜਕ->x.pList;
    pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
    ਜੇ( destIfNull!=destIfFalse ){
      regCkNull = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਦੋਆਧਾਰੀ_ਅੰਕ_And, rLhs, rLhs, regCkNull);
    }
    ਲਈ(ii=0; ii<pList->n_ਵਿਅੰਜਕ; ii++){
      r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, pList->a[ii].p_ਵਿਅੰਜਕ, &reg_ਮੰਜ਼ਿਲ_Free);
      ਜੇ( regCkNull && sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(pList->a[ii].p_ਵਿਅੰਜਕ) ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਦੋਆਧਾਰੀ_ਅੰਕ_And, regCkNull, r2, regCkNull);
      }
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, reg_ਮੰਜ਼ਿਲ_Free);
      ਜੇ( ii<pList->n_ਵਿਅੰਜਕ-1 || destIfNull!=destIfFalse ){
        ਪੂਰਨ op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, op, rLhs, labelOk, r2,
                          (ਖਾਲੀ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_ਵਿਅੰਜਕ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_ਵਿਅੰਜਕ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_ਵਿਅੰਜਕ-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_ਵਿਅੰਜਕ-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        ਪ੍ਰਤਿਜ੍ਞਾ( destIfNull==destIfFalse );
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, op, rLhs, destIfFalse, r2,
                          (ਖਾਲੀ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    ਜੇ( regCkNull ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regCkNull);
    ਜਾਓ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ_ਮੁਕੰਮਲ;
  }

  ਜੇ( eType!=ਵਿੱਚ_ਸੂਚਕਾਂਕ_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    ਜੇ( n_ਸਦਿਸ਼==1 ){
      ਅੱਖਰ aff = zAff[0];
      ਜੇ( aff>=SQLITE_AFF_ਪਾਠ && aff!=sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਖੱਬਾ) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        ਪੂਰਨ rTmp = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, rLhs, n_ਸਦਿਸ਼, 0, zAff, n_ਸਦਿਸ਼);
    }

    ਲਈ(i=0; i<n_ਸਦਿਸ਼ && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    ਜੇ( i!=n_ਸਦਿਸ਼ ){
      /* Need to reorder the LHS fields according to aiMap */
      ਪੂਰਨ rLhsOrig = rLhs;
      rLhs = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_ਸਦਿਸ਼);
      ਲਈ(i=0; i<n_ਸਦਿਸ਼; i++){
        testcase( aiMap[i]!=i );
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  ਜੇ( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }ਨਹੀਂਤਾਂ{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  ਲਈ(i=0; i<n_ਸਦਿਸ਼; i++){
    ਵਿਅੰਜਕ *p = sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, i);
    ਜੇ( pParse->nErr ) ਜਾਓ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ_oom_error;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  ਜੇ( eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਸਦਿਸ਼==1 );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);  /* Return True */
  }ਨਹੀਂਤਾਂ{
    ਜੇ( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subrtn) ){
        ਅਚਲ VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ਵਿਅੰਜਕ->y.sub.iAddr);
        ਪ੍ਰਤਿਜ੍ਞਾ( pOp->opcode==OP_Once || pParse->nErr );
        ਜੇ( pOp->p3>0 ){  /* tag-202407032019 */
          ਪ੍ਰਤਿਜ੍ਞਾ( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, n_ਸਦਿਸ਼); VdbeCoverage(v);
        }
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Not_ਮਿਲਿਆ, iTab, destIfFalse,
                           rLhs, n_ਸਦਿਸ਼); VdbeCoverage(v);
      ਜਾਓ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ_ਮੁਕੰਮਲ;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, iTab, 0,
                                      rLhs, n_ਸਦਿਸ਼); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  ਜੇ( rRhsHasNull && n_ਸਦਿਸ਼==1 ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  ਜੇ( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  ਜੇ( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  ਜੇ( eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_DESC ){
    addr_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਆਖਰੀ, iTab, destIfFalse);
  }ਨਹੀਂਤਾਂ{
    testcase( eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_EPH );
    testcase( eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_ROWID );
    addr_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  ਜੇ( n_ਸਦਿਸ਼>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }ਨਹੀਂਤਾਂ{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  ਲਈ(i=0; i<n_ਸਦਿਸ਼; i++){
    ਵਿਅੰਜਕ *p;
    CollSeq *pColl;
    ਪੂਰਨ r3 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
    p = sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(p_ਖੱਬਾ, i);
    ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
      ਵਿਅੰਜਕ *pRhs = p_ਵਿਅੰਜਕ->x.pSelect->pEList->a[i].p_ਵਿਅੰਜਕ;
      pColl = sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, p, pRhs);
    }ਨਹੀਂਤਾਂ{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iTab, aiMap[i], r3);
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (ਖਾਲੀ*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r3);
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, destIfNull);
  ਜੇ( n_ਸਦਿਸ਼>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iTab, addr_ਸਿਖਰ+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrTruthOp);

sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ_ਮੁਕੰਮਲ:
  VdbeComment((v, "end IN expr"));
sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SUBQUERY */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_Real(Vdbe *v, ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ negateFlag, ਪੂਰਨ iMem){
  ਜੇ( ALWAYS(z!=0) ){
    ਦੁੱਗਣਾ ਮੁੱਲ;
    sqlite3AtoF(z, &ਮੁੱਲ);
    ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3IsNaN(ਮੁੱਲ) ); /* The new AtoF never returns NaN */
    ਜੇ( negateFlag ) ਮੁੱਲ = -ਮੁੱਲ;
    sqlite3_Vdbe_ਜੋੜੋ_Double(v, iMem, ਮੁੱਲ);
  }
}
#ਜੇ_ਅੰਤ


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_Integer(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ negFlag, ਪੂਰਨ iMem){
  Vdbe *v = pParse->pVdbe;
  ਜੇ( p_ਵਿਅੰਜਕ->flags & EP_Int_ਮੁੱਲ ){
    ਪੂਰਨ i = p_ਵਿਅੰਜਕ->u.i_ਮੁੱਲ;
    ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 );
    ਜੇ( negFlag ) i = -i;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, i, iMem);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ c;
    i64 ਮੁੱਲ;
    ਅਚਲ ਅੱਖਰ *z = p_ਵਿਅੰਜਕ->u.zToken;
    ਪ੍ਰਤਿਜ੍ਞਾ( z!=0 );
    c = sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z, &ਮੁੱਲ);
    ਜੇ( (c==3 && !negFlag) || (c==2) || (negFlag && ਮੁੱਲ==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64)){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_ਵਿਅੰਜਕ);
#ਨਹੀਂਤਾਂ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_HEX_INTEGER
      ਜੇ( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_ਵਿਅੰਜਕ);
      }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
      {
        ਕੋਡ_Real(v, z, negFlag, iMem);
      }
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ{
      ਜੇ( negFlag ){ ਮੁੱਲ = c==3 ? ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 : -ਮੁੱਲ; }
      sqlite3_Vdbe_ਜੋੜੋ_Int64(v, iMem, ਮੁੱਲ);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੱਦੋ_ਸੂਚਕਾਂਕ_ਕਾਲਮ(
  Parse *pParse,  /* The parsing context */
  ਸੂਚਕਾਂਕ *pIdx,    /* The index whose column is to be loaded */
  ਪੂਰਨ iTabCur,    /* Cursor pointing to a table row */
  ਪੂਰਨ i_Idx_ਖਾਨਾ,    /* The column of the index to be loaded */
  ਪੂਰਨ reg_ਬਾਹਰਲਾ      /* Store the index column value in this register */
){
  i16 i_Tab_ਖਾਨਾ = pIdx->ai_ਕਾਲਮ[i_Idx_ਖਾਨਾ];
  ਜੇ( i_Tab_ਖਾਨਾ==XN_ਵਿਅੰਜਕ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ );
    ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->n_ਵਿਅੰਜਕ>i_Idx_ਖਾਨਾ );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Copy(pParse, pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i_Idx_ਖਾਨਾ].p_ਵਿਅੰਜਕ, reg_ਬਾਹਰਲਾ);
    pParse->iSelfTab = 0;
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(pParse->pVdbe, pIdx->p_ਸਾਰਣੀ, iTabCur,
                                    i_Tab_ਖਾਨਾ, reg_ਬਾਹਰਲਾ);
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Generated_ਕਾਲਮ(
  Parse *pParse,     /* Parsing context */
  ਸਾਰਣੀ *pTab,       /* Table containing the generated column */
  ਕਾਲਮ *p_ਖਾਨਾ,      /* The generated column */
  ਪੂਰਨ reg_ਬਾਹਰਲਾ         /* Put the result in this register */
){
  ਪੂਰਨ iAddr;
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ nErr = pParse->nErr;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->iSelfTab!=0 );
  ਜੇ( pParse->iSelfTab>0 ){
    iAddr = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_If_Null_ਪੰਕਤੀ, pParse->iSelfTab-1, 0, reg_ਬਾਹਰਲਾ);
  }ਨਹੀਂਤਾਂ{
    iAddr = 0;
  }
  sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Copy(pParse, sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pTab,p_ਖਾਨਾ), reg_ਬਾਹਰਲਾ);
  ਜੇ( (p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    ਪੂਰਨ p3 = 2+(ਪੂਰਨ)(p_ਖਾਨਾ - pTab->a_ਖਾਨਾ);
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_TypeCheck, reg_ਬਾਹਰਲਾ, 1, p3, (ਅੱਖਰ*)pTab, P4_ਸਾਰਣੀ);
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਖਾਨਾ->affinity>=SQLITE_AFF_ਪਾਠ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, reg_ਬਾਹਰਲਾ, 1, 0, &p_ਖਾਨਾ->affinity, 1);
  }
  ਜੇ( iAddr ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, iAddr);
  ਜੇ( pParse->nErr>nErr ) pParse->db->err_Byte_ਵਿੱਥ = -1;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(
  Vdbe *v,        /* Parsing context */
  ਸਾਰਣੀ *pTab,    /* The table containing the value */
  ਪੂਰਨ iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  ਪੂਰਨ i_ਖਾਨਾ,       /* Index of the column to extract */
  ਪੂਰਨ reg_ਬਾਹਰਲਾ      /* Extract the value into this register */
){
  ਕਾਲਮ *p_ਖਾਨਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ!=XN_ਵਿਅੰਜਕ );
  ਜੇ( i_ਖਾਨਾ<0 || i_ਖਾਨਾ==pTab->iPKey ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iTabCur, reg_ਬਾਹਰਲਾ);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ x;
    ਜੇ( IsVirtual(pTab) ){
      x = i_ਖਾਨਾ;
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_VColumn, iTabCur, x, reg_ਬਾਹਰਲਾ);
      ਵਾਪਸ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
    }ਨਹੀਂਤਾਂ ਜੇ( (p_ਖਾਨਾ = &pTab->a_ਖਾਨਾ[i_ਖਾਨਾ])->ਖਾਨਾ_Flags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_BUSY ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "generated column loop on \"%s\"",
                        p_ਖਾਨਾ->zCnName);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ savedSelfTab = pParse->iSelfTab;
        p_ਖਾਨਾ->ਖਾਨਾ_Flags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Generated_ਕਾਲਮ(pParse, pTab, p_ਖਾਨਾ, reg_ਬਾਹਰਲਾ);
        pParse->iSelfTab = savedSelfTab;
        p_ਖਾਨਾ->ਖਾਨਾ_Flags &= ~COLFLAG_BUSY;
      }
      ਵਾਪਸ;
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ ਜੇ( !HasRowid(pTab) ){
      testcase( i_ਖਾਨਾ!=sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i_ਖਾਨਾ) );
      x = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab), i_ਖਾਨਾ);
    }ਨਹੀਂਤਾਂ{
      x = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab,i_ਖਾਨਾ);
      testcase( x!=i_ਖਾਨਾ );
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iTabCur, x, reg_ਬਾਹਰਲਾ);
    sqlite3_ਕਾਲਮ_Default(v, pTab, i_ਖਾਨਾ, reg_ਬਾਹਰਲਾ);
  }
}

/*
** Generate code that will extract the iColumn-th column from
** table pTab and store the column value in register iReg.
**
** There must be an open cursor to pTab in iTable when this routine
** is called.  If iColumn<0 then code is generated that extracts the rowid.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ(
  Parse *pParse,   /* Parsing and code generating context */
  ਸਾਰਣੀ *pTab,     /* Description of the table we are reading from */
  ਪੂਰਨ i_ਕਾਲਮ,     /* Index of the table column */
  ਪੂਰਨ i_ਸਾਰਣੀ,      /* The cursor pointing to the table */
  ਪੂਰਨ iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pVdbe!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  ਪ੍ਰਤਿਜ੍ਞਾ( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(pParse->pVdbe, pTab, i_ਸਾਰਣੀ, i_ਕਾਲਮ, iReg);
  ਜੇ( p5 ){
    VdbeOp *pOp = sqlite3_Vdbe_Get_ਆਖਰੀ_Op(pParse->pVdbe);
    ਜੇ( pOp->opcode==OP_ਕਾਲਮ ) pOp->p5 = p5;
    ਜੇ( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  ਵਾਪਸ iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Move(Parse *pParse, ਪੂਰਨ iFrom, ਪੂਰਨ i_ਮੰਜ਼ਿਲ, ਪੂਰਨ nReg){
  sqlite3_Vdbe_ਜੋੜੋ_Op3(pParse->pVdbe, OP_Move, iFrom, i_ਮੰਜ਼ਿਲ, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ iReg){
  ਵਿਅੰਜਕ *p = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਵਿਅੰਜਕ);
  ਜੇ( NEVER(p==0) ) ਵਾਪਸ;
  ਜੇ( p->op==TK_ਰਜਿਸਟਰ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->i_ਸਾਰਣੀ==iReg );
  }ਨਹੀਂਤਾਂ{
    p->op2 = p->op;
    p->op = TK_ਰਜਿਸਟਰ;
    p->i_ਸਾਰਣੀ = iReg;
    ਵਿਅੰਜਕ_Clear_Property(p, EP_Skip);
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
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਕੋਡ_ਸਦਿਸ਼(Parse *pParse, ਵਿਅੰਜਕ *p, ਪੂਰਨ *piFreeable){
  ਪੂਰਨ i_ਨਤੀਜਾ;
  ਪੂਰਨ n_ਨਤੀਜਾ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p);
  ਜੇ( n_ਨਤੀਜਾ==1 ){
    i_ਨਤੀਜਾ = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p, piFreeable);
  }ਨਹੀਂਤਾਂ{
    *piFreeable = 0;
    ਜੇ( p->op==TK_SELECT ){
#ਜੇ SQLITE_OMIT_SUBQUERY
      i_ਨਤੀਜਾ = 0;
#ਨਹੀਂਤਾਂ
      i_ਨਤੀਜਾ = sqlite3_ਕੋਡ_Subselect(pParse, p);
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ i;
      i_ਨਤੀਜਾ = pParse->nMem+1;
      pParse->nMem += n_ਨਤੀਜਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p) );
      ਲਈ(i=0; i<n_ਨਤੀਜਾ; i++){
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Factorable(pParse, p->x.pList->a[i].p_ਵਿਅੰਜਕ, i+i_ਨਤੀਜਾ);
      }
    }
  }
  ਵਾਪਸ i_ਨਤੀਜਾ;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Do_Not_ਮਿਲਾਓ_Flag_On_Copy(Vdbe *v){
  ਜੇ( sqlite3_Vdbe_Get_ਆਖਰੀ_Op(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਕੋਡ_Inline_Function(
  Parse *pParse,        /* Parsing context */
  ਵਿਅੰਜਕ_List *pFarg,      /* List of function arguments */
  ਪੂਰਨ iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  ਪੂਰਨ ਨਿਸ਼ਾਨਾ            /* Store function result in this register */
){
  ਪੂਰਨ nFarg;
  Vdbe *v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFarg!=0 );
  nFarg = pFarg->n_ਵਿਅੰਜਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( nFarg>0 );  /* All in-line functions have at least one argument */
  ਚੋਣ( iFuncId ){
    ਹਾਲਤ INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      ਪੂਰਨ ਸਿਰਾ_Coalesce = sqlite3VdbeMakeLabel(pParse);
      ਪੂਰਨ i;
      ਪ੍ਰਤਿਜ੍ਞਾ( nFarg>=2 );
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pFarg->a[0].p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
      ਲਈ(i=1; i<nFarg; i++){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NotNull, ਨਿਸ਼ਾਨਾ, ਸਿਰਾ_Coalesce);
        VdbeCoverage(v);
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pFarg->a[i].p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
      }
      ਸੈੱਟ_Do_Not_ਮਿਲਾਓ_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, ਸਿਰਾ_Coalesce);
      ਰੋਕੋ;
    }
    ਹਾਲਤ INLINEFUNC_iif: {
      ਵਿਅੰਜਕ case_ਵਿਅੰਜਕ;
      memset(&case_ਵਿਅੰਜਕ, 0, ਆਕਾਰ(case_ਵਿਅੰਜਕ));
      case_ਵਿਅੰਜਕ.op = TK_CASE;
      case_ਵਿਅੰਜਕ.x.pList = pFarg;
      ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, &case_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਵਿੱਥ_SQL_FUNC
    ਹਾਲਤ INLINEFUNC_sqlite_ਵਿੱਥ: {
      ਵਿਅੰਜਕ *pArg = pFarg->a[0].p_ਵਿਅੰਜਕ;
      ਜੇ( pArg->op==TK_ਕਾਲਮ && pArg->i_ਸਾਰਣੀ>=0 ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਵਿੱਥ, pArg->i_ਸਾਰਣੀ, pArg->i_ਕਾਲਮ, ਨਿਸ਼ਾਨਾ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, ਨਿਸ਼ਾਨਾ);
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਮੂਲ: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      ਪ੍ਰਤਿਜ੍ਞਾ( nFarg==1 || nFarg==2 );
      ਨਿਸ਼ਾਨਾ = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, pFarg->a[0].p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
      ਰੋਕੋ;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#ਜੇ !defined(SQLITE_UNTESTABLE)
    ਹਾਲਤ INLINEFUNC_ਵਿਅੰਜਕ_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      ਪ੍ਰਤਿਜ੍ਞਾ( nFarg==2 );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer,
         sqlite3_ਵਿਅੰਜਕ_Compare(0,pFarg->a[0].p_ਵਿਅੰਜਕ, pFarg->a[1].p_ਵਿਅੰਜਕ,-1),
         ਨਿਸ਼ਾਨਾ);
      ਰੋਕੋ;
    }

    ਹਾਲਤ INLINEFUNC_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      ਪ੍ਰਤਿਜ੍ਞਾ( nFarg==2 );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer,
         sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ(pParse,pFarg->a[0].p_ਵਿਅੰਜਕ, pFarg->a[1].p_ਵਿਅੰਜਕ,-1),
         ਨਿਸ਼ਾਨਾ);
      ਰੋਕੋ;
    }

    ਹਾਲਤ INLINEFUNC_ਸਿੱਟਾ_ਦਿੰਦਾ_nonnull_ਪੰਕਤੀ: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      ਵਿਅੰਜਕ *pA1;
      ਪ੍ਰਤਿਜ੍ਞਾ( nFarg==2 );
      pA1 = pFarg->a[1].p_ਵਿਅੰਜਕ;
      ਜੇ( pA1->op==TK_ਕਾਲਮ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer,
           sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Non_Null_ਪੰਕਤੀ(pFarg->a[0].p_ਵਿਅੰਜਕ,pA1->i_ਸਾਰਣੀ,1),
           ਨਿਸ਼ਾਨਾ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, ਨਿਸ਼ਾਨਾ);
      }
      ਰੋਕੋ;
    }

    ਹਾਲਤ INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      ਅਚਲ ਅੱਖਰ *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      ਅੱਖਰ aff;
      ਪ੍ਰਤਿਜ੍ਞਾ( nFarg==1 );
      aff = sqlite3_ਵਿਅੰਜਕ_Affinity(pFarg->a[0].p_ਵਿਅੰਜਕ);
      ਪ੍ਰਤਿਜ੍ਞਾ( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3_Vdbe_ਲੱਦੋ_String(v, ਨਿਸ਼ਾਨਾ,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* !defined(SQLITE_UNTESTABLE) */
  }
  ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
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
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਗੰਢ_Can_Return_Subtype(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ n;
  FuncDef *pDef;
  sqlite3 *db;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_CASE || p_ਵਿਅੰਜਕ->op==TK_UPLUS 
   || p_ਵਿਅੰਜਕ->op==TK_COLLATE || p_ਵਿਅੰਜਕ->op==TK_CAST 
  ){
    ਵਾਪਸ WRC_Continue;
  }
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_FUNCTION && p_ਵਿਅੰਜਕ->op!=TK_AGG_FUNCTION ){
    ਵਾਪਸ WRC_Prune;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
  db = pWalker->pParse->db;
  n = p_ਵਿਅੰਜਕ->x.pList ? p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ : 0;
  pDef = sqlite3_ਲੱਭੋ_Function(db, p_ਵਿਅੰਜਕ->u.zToken, n, ENC(db), 0);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ਜੇ( pDef==0 ) ਵਾਪਸ WRC_Continue;
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( pDef!=0 );
#ਜੇ_ਅੰਤ
  ਜੇ( (pDef->funcFlags & SQLITE_ਨਤੀਜਾ_SUBTYPE)!=0 ){
    pWalker->e_ਕੋਡ = 1;
    ਵਾਪਸ WRC_Abort;
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Can_Return_Subtype(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  Walker w;
  memset(&w, 0, ਆਕਾਰ(w));
  w.pParse = pParse;
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_ਗੰਢ_Can_Return_Subtype;
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p_ਵਿਅੰਜਕ);
  ਵਾਪਸ w.e_ਕੋਡ;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ sqlite3_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ_Lookup(
  Parse *pParse,   /* The parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,     /* The expression to potentially bypass */
  ਪੂਰਨ ਨਿਸ਼ਾਨਾ       /* Where to store the result of the expression */
){
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *p;
  Vdbe *v;
  ਲਈ(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 ਵਿਅੰਜਕ_Aff;
    ਪੂਰਨ i_ਅੰਕੜੇ_Cur = p->i_ਅੰਕੜੇ_Cur;
    ਜੇ( i_ਅੰਕੜੇ_Cur<0 ) ਜਾਰੀ;
    ਜੇ( pParse->iSelfTab ){
      ਜੇ( p->i_ਅੰਕੜੇ_Cur!=pParse->iSelfTab-1 ) ਜਾਰੀ;
      i_ਅੰਕੜੇ_Cur = -1;
    }
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, p_ਵਿਅੰਜਕ, p->p_ਵਿਅੰਜਕ, i_ਅੰਕੜੇ_Cur)!=0 ) ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    ਵਿਅੰਜਕ_Aff = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ);
    ਜੇ( (ਵਿਅੰਜਕ_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (ਵਿਅੰਜਕ_Aff==SQLITE_AFF_ਪਾਠ && p->aff!=SQLITE_AFF_ਪਾਠ)
     || (ਵਿਅੰਜਕ_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      ਜਾਰੀ;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_SubtArg)
     && sqlite3_ਵਿਅੰਜਕ_Can_Return_Subtype(pParse, p_ਵਿਅੰਜਕ) 
    ){
      ਜਾਰੀ;
    }

    v = pParse->pVdbe;
    ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
    ਜੇ( p->b_Maybe_Null_ਪੰਕਤੀ || sqlite3_ਵਿਅੰਜਕ_Is_Constant(pParse, p_ਵਿਅੰਜਕ) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      ਪੂਰਨ addr = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_If_Null_ਪੰਕਤੀ, p->iIdxCur, addr+3, ਨਿਸ਼ਾਨਾ);
      VdbeCoverage(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, p->iIdxCur, p->i_Idx_ਖਾਨਾ, ਨਿਸ਼ਾਨਾ);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_ਖਾਨਾ));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
      pParse->pIdxEpr = p;
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr+2);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, p->iIdxCur, p->i_Idx_ਖਾਨਾ, ਨਿਸ਼ਾਨਾ);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_ਖਾਨਾ));
    }
    ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
  }
  ਵਾਪਸ -1;  /* Not found */
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
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Partidx_ਵਿਅੰਜਕ_Lookup(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ i_ਨਿਸ਼ਾਨਾ){
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *p;
  ਲਈ(p=pParse->p_Idx_Part_ਵਿਅੰਜਕ; p; p=p->pIENext){
    ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ==p->i_Idx_ਖਾਨਾ && p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==p->i_ਅੰਕੜੇ_Cur ){
      Vdbe *v = pParse->pVdbe;
      ਪੂਰਨ addr = 0;
      ਪੂਰਨ ret;

      ਜੇ( p->b_Maybe_Null_ਪੰਕਤੀ ){
        addr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_If_Null_ਪੰਕਤੀ, p->iIdxCur);
      }
      ret = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, p->p_ਵਿਅੰਜਕ, i_ਨਿਸ਼ਾਨਾ);
      sqlite3_Vdbe_ਜੋੜੋ_Op4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ਅਚਲ ਅੱਖਰ*)&p->aff, 1);
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Can_Return_Subtype(pParse, p->p_ਵਿਅੰਜਕ) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3_Vdbe_ਜੋੜੋ_Op1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      ਜੇ( addr ){
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      ਵਾਪਸ ret;
    }
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ_And_Or(
  Parse *pParse,     /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,       /* AND or OR expression to be coded */
  ਪੂਰਨ ਨਿਸ਼ਾਨਾ,        /* Put result in this register, guaranteed */
  ਪੂਰਨ *pTmpReg       /* Write a temporary register here */
){
  ਪੂਰਨ op;            /* The opcode.  TK_AND or TK_OR */
  ਪੂਰਨ skipOp;        /* Opcode for the branch that skips one operand */
  ਪੂਰਨ addrSkip;      /* Branch instruction that skips one of the operands */
  ਪੂਰਨ regSS = 0;     /* Register holding computed operand when other omitted */
  ਪੂਰਨ r1, r2;        /* Registers for left and right operands, respectively */
  ਵਿਅੰਜਕ *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
  op = p_ਵਿਅੰਜਕ->op;
  ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_AND || op==TK_OR );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_AND==OP_And );            testcase( op==TK_AND );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_OR==OP_Or );              testcase( op==TK_OR );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_ਵਿਅੰਜਕ_Simplified_And_Or(p_ਵਿਅੰਜਕ);
  ਜੇ( pAlt!=p_ਵਿਅੰਜਕ ){
    r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, pAlt, ਨਿਸ਼ਾਨਾ);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_And, r1, r1, ਨਿਸ਼ਾਨਾ);
    ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  ਜੇ( ਵਿਅੰਜਕ_Eval_Rhs_ਪਹਿਲਾ(p_ਵਿਅੰਜਕ) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, ਨਿਸ਼ਾਨਾ);
    addrSkip = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, pTmpReg);
  }ਨਹੀਂਤਾਂ{
    /* Compute the left operand first */
    r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, ਨਿਸ਼ਾਨਾ);
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }ਨਹੀਂਤਾਂ{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, pTmpReg);
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, op, r2, r1, ਨਿਸ਼ਾਨਾ);
  testcase( (*pTmpReg)==0 );
  ਜੇ( addrSkip ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrSkip);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Or, regSS, regSS, ਨਿਸ਼ਾਨਾ);
    VdbeComment((v, "short-circut value"));
  }
  ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ ਨਿਸ਼ਾਨਾ){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  ਪੂਰਨ op;                   /* The opcode being coded */
  ਪੂਰਨ ਵਿੱਚ_Reg = ਨਿਸ਼ਾਨਾ;       /* Results stored in register inReg */
  ਪੂਰਨ regFree1 = 0;         /* If non-zero free this temporary register */
  ਪੂਰਨ regFree2 = 0;         /* If non-zero free this temporary register */
  ਪੂਰਨ r1, r2;               /* Various register numbers */
  ਵਿਅੰਜਕ ਆਰਜ਼ੀ_X;               /* Temporary expression node */
  ਪੂਰਨ p5 = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( ਨਿਸ਼ਾਨਾ>0 && ਨਿਸ਼ਾਨਾ<=pParse->nMem );
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );

ਵਿਅੰਜਕ_ਕੋਡ_doover:
  ਜੇ( p_ਵਿਅੰਜਕ==0 ){
    op = TK_NULL;
  }ਨਹੀਂਤਾਂ ਜੇ( pParse->pIdxEpr!=0
   && !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_ਪੱਤੀ)
   && (r1 = sqlite3_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ_Lookup(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ))>=0
  ){
    ਵਾਪਸ r1;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_VVAProperty(p_ਵਿਅੰਜਕ,EP_Immutable) );
    op = p_ਵਿਅੰਜਕ->op;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_ਕ੍ਰਮ );
  ਚੋਣ( op ){
    ਹਾਲਤ TK_AGG_ਕਾਲਮ: {
      AggInfo *pAggInfo = p_ਵਿਅੰਜਕ->pAggInfo;
      ਬਣਤਰ Agg_Info_ਖਾਨਾ *p_ਖਾਨਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->iAgg>=0 );
      ਜੇ( p_ਵਿਅੰਜਕ->iAgg>=pAggInfo->n_ਕਾਲਮ ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, ਨਿਸ਼ਾਨਾ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NotNull, ਨਿਸ਼ਾਨਾ, 1, 20230325);
        Vdbe_Coverage_Never_ਲਿਆ(v);
#ਜੇ_ਅੰਤ
        ਰੋਕੋ;
      }
      p_ਖਾਨਾ = &pAggInfo->a_ਖਾਨਾ[p_ਵਿਅੰਜਕ->iAgg];
      ਜੇ( !pAggInfo->ਸਿੱਧਾ_ਬਹੁਲਕ ){
        ਵਾਪਸ Agg_Info_ਕਾਲਮ_Reg(pAggInfo, p_ਵਿਅੰਜਕ->iAgg);
      }ਨਹੀਂਤਾਂ ਜੇ( pAggInfo->use_ਛਾਂਟੀ_Idx ){
        ਸਾਰਣੀ *pTab = p_ਖਾਨਾ->pTab;
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, pAggInfo->ਛਾਂਟੀ_Idx_PTab,
                              p_ਖਾਨਾ->i_Sorter_ਕਾਲਮ, ਨਿਸ਼ਾਨਾ);
        ਜੇ( pTab==0 ){
          /* No comment added */
        }ਨਹੀਂਤਾਂ ਜੇ( p_ਖਾਨਾ->i_ਕਾਲਮ<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }ਨਹੀਂਤਾਂ{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->a_ਖਾਨਾ[p_ਖਾਨਾ->i_ਕਾਲਮ].zCnName));
          ਜੇ( pTab->a_ਖਾਨਾ[p_ਖਾਨਾ->i_ਕਾਲਮ].affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_RealAffinity, ਨਿਸ਼ਾਨਾ);
          }
        }
        ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
      }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ, ਨਿਸ਼ਾਨਾ);
        ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    ਹਾਲਤ TK_ਕਾਲਮ: {
      ਪੂਰਨ iTab = p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;
      ਪੂਰਨ iReg;
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Fixed_ਖਾਨਾ) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        ਪੂਰਨ aff;
        iReg = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ,ਨਿਸ਼ਾਨਾ);
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->y.pTab!=0 );
        aff = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_Affinity(p_ਵਿਅੰਜਕ->y.pTab, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
        ਜੇ( aff>SQLITE_AFF_BLOB ){
          ਸਥਿਰ ਅਚਲ ਅੱਖਰ zAff[] = "B\000C\000D\000E\000F";
          ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_AFF_BLOB=='A' );
          ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_AFF_ਪਾਠ=='B' );
          sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        ਵਾਪਸ iReg;
      }
      ਜੇ( iTab<0 ){
        ਜੇ( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          ਕਾਲਮ *p_ਖਾਨਾ;
          ਸਾਰਣੀ *pTab;
          ਪੂਰਨ iSrc;
          ਪੂਰਨ i_ਖਾਨਾ = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
          pTab = p_ਵਿਅੰਜਕ->y.pTab;
          ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=XN_ROWID );
          ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ<pTab->n_ਖਾਨਾ );
          ਜੇ( i_ਖਾਨਾ<0 ){
            ਵਾਪਸ -1-pParse->iSelfTab;
          }
          p_ਖਾਨਾ = pTab->a_ਖਾਨਾ + i_ਖਾਨਾ;
          testcase( i_ਖਾਨਾ!=sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab,i_ਖਾਨਾ) );
          iSrc = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i_ਖਾਨਾ) - pParse->iSelfTab;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
          ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED ){
            ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_BUSY ){
              sqlite3_Error_ਸੁਨੇਹਾ(pParse, "generated column loop on \"%s\"",
                              p_ਖਾਨਾ->zCnName);
              ਵਾਪਸ 0;
            }
            p_ਖਾਨਾ->ਖਾਨਾ_Flags |= COLFLAG_BUSY;
            ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_NOTAVAIL ){
              sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Generated_ਕਾਲਮ(pParse, pTab, p_ਖਾਨਾ, iSrc);
            }
            p_ਖਾਨਾ->ਖਾਨਾ_Flags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            ਵਾਪਸ iSrc;
          }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_GENERATED_COLUMNS */
          ਜੇ( p_ਖਾਨਾ->affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_SCopy, iSrc, ਨਿਸ਼ਾਨਾ);
            sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_RealAffinity, ਨਿਸ਼ਾਨਾ);
            ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
          }ਨਹੀਂਤਾਂ{
            ਵਾਪਸ iSrc;
          }
        }ਨਹੀਂਤਾਂ{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      ਨਹੀਂਤਾਂ ਜੇ( pParse->p_Idx_Part_ਵਿਅੰਜਕ 
       && 0!=(r1 = ਵਿਅੰਜਕ_Partidx_ਵਿਅੰਜਕ_Lookup(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ))
      ){
        ਵਾਪਸ r1;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->y.pTab!=0 );
      iReg = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ(pParse, p_ਵਿਅੰਜਕ->y.pTab,
                               p_ਵਿਅੰਜਕ->i_ਕਾਲਮ, iTab, ਨਿਸ਼ਾਨਾ,
                               p_ਵਿਅੰਜਕ->op2);
      ਵਾਪਸ iReg;
    }
    ਹਾਲਤ TK_INTEGER: {
      ਕੋਡ_Integer(pParse, p_ਵਿਅੰਜਕ, 0, ਨਿਸ਼ਾਨਾ);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
    ਹਾਲਤ TK_TRUEFALSE: {
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(p_ਵਿਅੰਜਕ), ਨਿਸ਼ਾਨਾ);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
    ਹਾਲਤ TK_FLOAT: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      ਕੋਡ_Real(v, p_ਵਿਅੰਜਕ->u.zToken, 0, ਨਿਸ਼ਾਨਾ);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ TK_STRING: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_Vdbe_ਲੱਦੋ_String(v, ਨਿਸ਼ਾਨਾ, p_ਵਿਅੰਜਕ->u.zToken);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
    ਹਾਲਤ TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, ਨਿਸ਼ਾਨਾ, ਨਿਸ਼ਾਨਾ + p_ਵਿਅੰਜਕ->y.nReg - 1);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
    ਮੂਲ: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, ਨਿਸ਼ਾਨਾ);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_BLOB_LITERAL
    ਹਾਲਤ TK_BLOB: {
      ਪੂਰਨ n;
      ਅਚਲ ਅੱਖਰ *z;
      ਅੱਖਰ *zBlob;
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->u.zToken[0]=='x' || p_ਵਿਅੰਜਕ->u.zToken[0]=='X' );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->u.zToken[1]=='\'' );
      z = &p_ਵਿਅੰਜਕ->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      ਪ੍ਰਤਿਜ੍ਞਾ( z[n]=='\'' );
      zBlob = sqlite3_Hex_ਮੰਜ਼ਿਲ_Blob(sqlite3VdbeDb(v), z, n);
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Blob, n/2, ਨਿਸ਼ਾਨਾ, 0, zBlob, P4_DYNAMIC);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ TK_VARIABLE: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->u.zToken!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->u.zToken[0]!=0 );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Variable, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ, ਨਿਸ਼ਾਨਾ);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
    ਹਾਲਤ TK_ਰਜਿਸਟਰ: {
      ਵਾਪਸ p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CAST
    ਹਾਲਤ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, ਨਿਸ਼ਾਨਾ);
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿੱਚ_Reg==ਨਿਸ਼ਾਨਾ );
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Cast, ਨਿਸ਼ਾਨਾ,
                        sqlite3AffinityType(p_ਵਿਅੰਜਕ->u.zToken, 0));
      ਵਾਪਸ ਵਿੱਚ_Reg;
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_CAST */
    ਹਾਲਤ TK_IS:
    ਹਾਲਤ TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ਹਾਲਤ TK_LT:
    ਹਾਲਤ TK_LE:
    ਹਾਲਤ TK_GT:
    ਹਾਲਤ TK_GE:
    ਹਾਲਤ TK_NE:
    ਹਾਲਤ TK_EQ: {
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
      ਪੂਰਨ addrIsNull = 0;
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(p_ਖੱਬਾ) ){
        ਕੋਡ_ਸਦਿਸ਼_Compare(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ, op, p5);
      }ਨਹੀਂਤਾਂ{
        ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = ਵਿਅੰਜਕ_Compute_Operands(pParse, p_ਵਿਅੰਜਕ,
                                     &r1, &r2, &regFree1, &regFree2);
        }ਨਹੀਂਤਾਂ{
          r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
          r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, &regFree2);
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, ਵਿੱਚ_Reg);
        ਕੋਡ_Compare(pParse, p_ਖੱਬਾ, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_Commuted));
        ਪ੍ਰਤਿਜ੍ਞਾ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        ਪ੍ਰਤਿਜ੍ਞਾ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        ਪ੍ਰਤਿਜ੍ਞਾ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        ਪ੍ਰਤਿਜ੍ਞਾ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        ਪ੍ਰਤਿਜ੍ਞਾ(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        ਪ੍ਰਤਿਜ੍ਞਾ(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        ਜੇ( p5==SQLITE_NULLEQ ){
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, ਵਿੱਚ_Reg);
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ZeroOrNull, r1, ਵਿੱਚ_Reg, r2);
          ਜੇ( addrIsNull ){
            sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrIsNull);
            sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, ਵਿੱਚ_Reg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_AND:
    ਹਾਲਤ TK_OR: {
      ਵਿੱਚ_Reg = ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ_And_Or(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ, &regFree1);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_PLUS:
    ਹਾਲਤ TK_ਤਾਰਾ:
    ਹਾਲਤ TK_MINUS:
    ਹਾਲਤ TK_REM:
    ਹਾਲਤ TK_BITAND:
    ਹਾਲਤ TK_BITOR:
    ਹਾਲਤ TK_SLASH:
    ਹਾਲਤ TK_LSHIFT:
    ਹਾਲਤ TK_RSHIFT:
    ਹਾਲਤ TK_CONCAT: {
      ਪੂਰਨ addrIsNull;
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_PLUS==OP_ਜੋੜੋ );           testcase( op==TK_PLUS );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_MINUS==OP_ਘਟਾਓ );     testcase( op==TK_MINUS );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_BITAND==OP_ਦੋਆਧਾਰੀ_ਅੰਕ_And );      testcase( op==TK_BITAND );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_BITOR==OP_ਦੋਆਧਾਰੀ_ਅੰਕ_Or );        testcase( op==TK_BITOR );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_LSHIFT==OP_ਖਿਸਕਾਓ_ਖੱਬਾ );   testcase( op==TK_LSHIFT );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_RSHIFT==OP_ਖਿਸਕਾਓ_ਸੱਜਾ );  testcase( op==TK_RSHIFT );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subquery) ){
        addrIsNull = ਵਿਅੰਜਕ_Compute_Operands(pParse, p_ਵਿਅੰਜਕ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ਨਹੀਂਤਾਂ{
        r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
        r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, &regFree2);
        addrIsNull = 0;
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, op, r2, r1, ਨਿਸ਼ਾਨਾ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ਜੇ( addrIsNull ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrIsNull);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, ਨਿਸ਼ਾਨਾ);
        VdbeComment((v, "short-circut value"));
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_UMINUS: {
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ );
      ਜੇ( p_ਖੱਬਾ->op==TK_INTEGER ){
        ਕੋਡ_Integer(pParse, p_ਖੱਬਾ, 1, ਨਿਸ਼ਾਨਾ);
        ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
      }ਨਹੀਂਤਾਂ ਜੇ( p_ਖੱਬਾ->op==TK_FLOAT ){
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
        ਕੋਡ_Real(v, p_ਖੱਬਾ->u.zToken, 1, ਨਿਸ਼ਾਨਾ);
        ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
#ਜੇ_ਅੰਤ
      }ਨਹੀਂਤਾਂ{
        ਆਰਜ਼ੀ_X.op = TK_INTEGER;
        ਆਰਜ਼ੀ_X.flags = EP_Int_ਮੁੱਲ|EP_TokenOnly;
        ਆਰਜ਼ੀ_X.u.i_ਮੁੱਲ = 0;
        ਵਿਅੰਜਕ_Clear_VVAProperties(&ਆਰਜ਼ੀ_X);
        r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, &ਆਰਜ਼ੀ_X, &regFree1);
        r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree2);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਘਟਾਓ, r2, r1, ਨਿਸ਼ਾਨਾ);
        testcase( regFree2==0 );
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_BITNOT:
    ਹਾਲਤ TK_NOT: {
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_BITNOT==OP_ਦੋਆਧਾਰੀ_ਅੰਕ_Not );   testcase( op==TK_BITNOT );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
      testcase( regFree1==0 );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, op, r1, ਵਿੱਚ_Reg);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_TRUTH: {
      ਪੂਰਨ isTrue;    /* IS TRUE or IS NOT TRUE */
      ਪੂਰਨ b_ਸਾਧਾਰਨ;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
      b_ਸਾਧਾਰਨ = p_ਵਿਅੰਜਕ->op2==TK_IS;
      testcase( isTrue && b_ਸਾਧਾਰਨ);
      testcase( !isTrue && b_ਸਾਧਾਰਨ);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IsTrue, r1, ਵਿੱਚ_Reg, !isTrue, isTrue ^ b_ਸਾਧਾਰਨ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ISNULL:
    ਹਾਲਤ TK_NOTNULL: {
      ਪੂਰਨ addr;
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, ਨਿਸ਼ਾਨਾ);
      r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, ਨਿਸ਼ਾਨਾ);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_ਵਿਅੰਜਕ->pAggInfo;
      ਜੇ( pInfo==0
       || NEVER(p_ਵਿਅੰਜਕ->iAgg<0)
       || NEVER(p_ਵਿਅੰਜਕ->iAgg>=pInfo->nFunc)
      ){
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "misuse of aggregate: %#T()", p_ਵਿਅੰਜਕ);
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ AggInfoFuncReg(pInfo, p_ਵਿਅੰਜਕ->iAgg);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_FUNCTION: {
      ਵਿਅੰਜਕ_List *pFarg;       /* List of function arguments */
      ਪੂਰਨ nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ਅਚਲ ਅੱਖਰ *zId;       /* The function name */
      u32 const_ਨਕਾਬ = 0;     /* Mask of function arguments that are constant */
      ਪੂਰਨ i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
        ਵਾਪਸ p_ਵਿਅੰਜਕ->y.pWin->reg_ਨਤੀਜਾ;
      }
#ਜੇ_ਅੰਤ

      ਜੇ( Const_ਗੁਣਕ_Ok(pParse)
       && sqlite3_ਵਿਅੰਜਕ_Is_Constant_Not_Join(pParse,p_ਵਿਅੰਜਕ)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੜੀਵਾਰ_Just_Once(pParse, p_ਵਿਅੰਜਕ, -1);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly) );
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
      pFarg = p_ਵਿਅੰਜਕ->x.pList;
      nFarg = pFarg ? pFarg->n_ਵਿਅੰਜਕ : 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      zId = p_ਵਿਅੰਜਕ->u.zToken;
      pDef = sqlite3_ਲੱਭੋ_Function(db, zId, nFarg, enc, 0);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      ਜੇ( pDef==0 && pParse->explain ){
        pDef = sqlite3_ਲੱਭੋ_Function(db, "unknown", nFarg, enc, 0);
      }
#ਜੇ_ਅੰਤ
      ਜੇ( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_ਅੰਦਰੂਨੀ)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_ਅੰਦਰੂਨੀ_Func)==0)
      ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "unknown function: %#T()", p_ਵਿਅੰਜਕ);
        ਰੋਕੋ;
      }
      ਜੇ( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        ਪ੍ਰਤਿਜ੍ਞਾ( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( (pDef->funcFlags & SQLITE_FUNC_ਸਿੱਧਾ)==0 );
        ਵਾਪਸ ਵਿਅੰਜਕ_ਕੋਡ_Inline_Function(pParse, pFarg,
             SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(pDef->p_User_ਅੰਕੜੇ), ਨਿਸ਼ਾਨਾ);
      }ਨਹੀਂਤਾਂ ਜੇ( pDef->funcFlags & (SQLITE_FUNC_ਸਿੱਧਾ|SQLITE_FUNC_UNSAFE) ){
        sqlite3_ਵਿਅੰਜਕ_Function_Usable(pParse, p_ਵਿਅੰਜਕ, pDef);
      }

      ਲਈ(i=0; i<nFarg; i++){
        ਜੇ( i<32 && sqlite3_ਵਿਅੰਜਕ_Is_Constant(pParse, pFarg->a[i].p_ਵਿਅੰਜਕ) ){
          testcase( i==31 );
          const_ਨਕਾਬ |= MASKBIT32(i);
        }
        ਜੇ( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, pFarg->a[i].p_ਵਿਅੰਜਕ);
        }
      }
      ਜੇ( pFarg ){
        ਜੇ( const_ਨਕਾਬ ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }ਨਹੀਂਤਾਂ{
          r1 = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        ਜੇ( (pDef->funcFlags & (SQLITE_FUNC_ਲੰਮਾਈ|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 ਵਿਅੰਜਕ_Op;
          ਪ੍ਰਤਿਜ੍ਞਾ( nFarg==1 );
          ਪ੍ਰਤਿਜ੍ਞਾ( pFarg->a[0].p_ਵਿਅੰਜਕ!=0 );
          ਵਿਅੰਜਕ_Op = pFarg->a[0].p_ਵਿਅੰਜਕ->op;
          ਜੇ( ਵਿਅੰਜਕ_Op==TK_ਕਾਲਮ || ਵਿਅੰਜਕ_Op==TK_AGG_ਕਾਲਮ ){
            ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_ਲੰਮਾਈ==OPFLAG_LENGTHARG );
            ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            ਪ੍ਰਤਿਜ੍ਞਾ( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_ਵਿਅੰਜਕ->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, pFarg, r1, 0, SQLITE_ECEL_ਗੁਣਕ);
      }ਨਹੀਂਤਾਂ{
        r1 = 0;
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
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
      ਜੇ( nFarg>=2 && ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_ਵਿਅੰਜਕ);
      }ਨਹੀਂਤਾਂ ਜੇ( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_ਵਿਅੰਜਕ);
      }
#ਜੇ_ਅੰਤ
      ਜੇ( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        ਜੇ( !pColl ) pColl = db->pDfltColl;
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_CollSeq, 0, 0, 0, (ਅੱਖਰ *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Function_Call(pParse, const_ਨਕਾਬ, r1, ਨਿਸ਼ਾਨਾ, nFarg,
                                 pDef, p_ਵਿਅੰਜਕ->op2);
      ਜੇ( nFarg ){
        ਜੇ( const_ਨਕਾਬ==0 ){
          sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, r1, nFarg);
        }ਨਹੀਂਤਾਂ{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, const_ਨਕਾਬ, 1);
        }
      }
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    ਹਾਲਤ TK_EXISTS:
    ਹਾਲਤ TK_SELECT: {
      ਪੂਰਨ n_ਖਾਨਾ;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      ਜੇ( pParse->db->mallocFailed ){
        ਵਾਪਸ 0;
      }ਨਹੀਂਤਾਂ ਜੇ( op==TK_SELECT
             && ALWAYS( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) )
             && (n_ਖਾਨਾ = p_ਵਿਅੰਜਕ->x.pSelect->pEList->n_ਵਿਅੰਜਕ)!=1
      ){
        sqlite3SubselectError(pParse, n_ਖਾਨਾ, 1);
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ sqlite3_ਕੋਡ_Subselect(pParse, p_ਵਿਅੰਜਕ);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_SELECT_ਕਾਲਮ: {
      ਪੂਰਨ n;
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
      ਜੇ( p_ਖੱਬਾ->i_ਸਾਰਣੀ==0 || pParse->withinRJSubrtn > p_ਖੱਬਾ->op2 ){
        p_ਖੱਬਾ->i_ਸਾਰਣੀ = sqlite3_ਕੋਡ_Subselect(pParse, p_ਖੱਬਾ);
        p_ਖੱਬਾ->op2 = pParse->withinRJSubrtn;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ->op==TK_SELECT || p_ਖੱਬਾ->op==TK_ERROR );
      n = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਖੱਬਾ);
      ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ!=n ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%d columns assigned %d values",
                                p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ, n);
      }
      ਵਾਪਸ p_ਖੱਬਾ->i_ਸਾਰਣੀ + p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
    }
    ਹਾਲਤ TK_ਵਿੱਚ: {
      ਪੂਰਨ destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ਪੂਰਨ destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, ਨਿਸ਼ਾਨਾ);
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ(pParse, p_ਵਿਅੰਜਕ, destIfFalse, destIfNull);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, ਨਿਸ਼ਾਨਾ);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, ਨਿਸ਼ਾਨਾ, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SUBQUERY */


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
    ਹਾਲਤ TK_BETWEEN: {
      ਵਿਅੰਜਕ_ਕੋਡ_Between(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ, 0, 0);
      ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
    }
    ਹਾਲਤ TK_COLLATE: {
      ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ );
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, ਨਿਸ਼ਾਨਾ);
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ClrSubtype, ਨਿਸ਼ਾਨਾ);
        ਵਾਪਸ ਨਿਸ਼ਾਨਾ;
      }ਨਹੀਂਤਾਂ{
        p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
        ਜਾਓ ਵਿਅੰਜਕ_ਕੋਡ_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    ਹਾਲਤ TK_ਵਿਸਤਾਰ:
    ਹਾਲਤ TK_UPLUS: {
      p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
      ਜਾਓ ਵਿਅੰਜਕ_ਕੋਡ_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    ਹਾਲਤ TK_TRIGGER: {
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
      ਸਾਰਣੀ *pTab;
      ਪੂਰਨ i_ਖਾਨਾ;
      ਪੂਰਨ p1;

      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
      pTab = p_ਵਿਅੰਜਕ->y.pTab;
      i_ਖਾਨਾ = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
      p1 = p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ * (pTab->n_ਖਾਨਾ+1) + 1
                     + sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i_ਖਾਨਾ);

      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==0 || p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==1 );
      ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=-1 && i_ਖਾਨਾ<pTab->n_ਖਾਨਾ );
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab->iPKey<0 || i_ਖਾਨਾ!=pTab->iPKey );
      ਪ੍ਰਤਿਜ੍ਞਾ( p1>=0 && p1<(pTab->n_ਖਾਨਾ*2+2) );

      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Param, p1, ਨਿਸ਼ਾਨਾ);
      VdbeComment((v, "r[%d]=%s.%s", ਨਿਸ਼ਾਨਾ,
        (p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ ? "new" : "old"),
        (p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<0 ? "rowid" : p_ਵਿਅੰਜਕ->y.pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName)
      ));

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      ਜੇ( i_ਖਾਨਾ>=0 && pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].affinity==SQLITE_AFF_REAL ){
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_RealAffinity, ਨਿਸ਼ਾਨਾ);
      }
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    }

    ਹਾਲਤ TK_ਸਦਿਸ਼: {
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "row value misused");
      ਰੋਕੋ;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    ਹਾਲਤ TK_IF_NULL_ਪੰਕਤੀ: {
      ਪੂਰਨ addrINR;
      u8 ok_Const_ਗੁਣਕ = pParse->ok_Const_ਗੁਣਕ;
      AggInfo *pAggInfo = p_ਵਿਅੰਜਕ->pAggInfo;
      ਜੇ( pAggInfo ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->iAgg>=0 && p_ਵਿਅੰਜਕ->iAgg<pAggInfo->n_ਕਾਲਮ );
        ਜੇ( !pAggInfo->ਸਿੱਧਾ_ਬਹੁਲਕ ){
          ਵਿੱਚ_Reg = Agg_Info_ਕਾਲਮ_Reg(pAggInfo, p_ਵਿਅੰਜਕ->iAgg);
          ਰੋਕੋ;
        }
        ਜੇ( p_ਵਿਅੰਜਕ->pAggInfo->use_ਛਾਂਟੀ_Idx ){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, pAggInfo->ਛਾਂਟੀ_Idx_PTab,
                            pAggInfo->a_ਖਾਨਾ[p_ਵਿਅੰਜਕ->iAgg].i_Sorter_ਕਾਲਮ,
                            ਨਿਸ਼ਾਨਾ);
          ਵਿੱਚ_Reg = ਨਿਸ਼ਾਨਾ;
          ਰੋਕੋ;
        }
      }
      addrINR = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_If_Null_ਪੰਕਤੀ, p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ, 0, ਨਿਸ਼ਾਨਾ);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->ok_Const_ਗੁਣਕ = 0;   /* note (1) above */
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, ਨਿਸ਼ਾਨਾ);
      ਪ੍ਰਤਿਜ੍ਞਾ( ਨਿਸ਼ਾਨਾ==ਵਿੱਚ_Reg );
      pParse->ok_Const_ਗੁਣਕ = ok_Const_ਗੁਣਕ;
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrINR);
      ਰੋਕੋ;
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
    ਹਾਲਤ TK_CASE: {
      ਪੂਰਨ ਸਿਰਾ_Label;                     /* GOTO label for end of CASE stmt */
      ਪੂਰਨ ਅਗਲਾ_Case;                     /* GOTO label for next WHEN clause */
      ਪੂਰਨ n_ਵਿਅੰਜਕ;                        /* 2x number of WHEN terms */
      ਪੂਰਨ i;                            /* Loop counter */
      ਵਿਅੰਜਕ_List *pEList;                 /* List of WHEN terms */
      ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *aListelem;  /* Array of WHEN terms */
      ਵਿਅੰਜਕ opCompare;                   /* The X==Ei expression */
      ਵਿਅੰਜਕ *pX;                         /* The X expression */
      ਵਿਅੰਜਕ *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      ਵਿਅੰਜਕ *pDel = 0;
      sqlite3 *db = pParse->db;

      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) && p_ਵਿਅੰਜਕ->x.pList!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ(p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ > 0);
      pEList = p_ਵਿਅੰਜਕ->x.pList;
      aListelem = pEList->a;
      n_ਵਿਅੰਜਕ = pEList->n_ਵਿਅੰਜਕ;
      ਸਿਰਾ_Label = sqlite3VdbeMakeLabel(pParse);
      ਜੇ( (pX = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ)!=0 ){
        pDel = sqlite3_ਵਿਅੰਜਕ_Dup(db, pX, 0);
        ਜੇ( db->mallocFailed ){
          sqlite3_ਵਿਅੰਜਕ_Delete(db, pDel);
          ਰੋਕੋ;
        }
        testcase( pX->op==TK_ਕਾਲਮ );
        sqlite3_ਵਿਅੰਜਕ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ(pDel, ਵਿਅੰਜਕ_ਕੋਡ_ਸਦਿਸ਼(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, ਆਕਾਰ(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_ਖੱਬਾ = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      ਲਈ(i=0; i<n_ਵਿਅੰਜਕ-1; i=i+2){
        ਜੇ( pX ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pTest!=0 );
          opCompare.p_ਸੱਜਾ = aListelem[i].p_ਵਿਅੰਜਕ;
        }ਨਹੀਂਤਾਂ{
          pTest = aListelem[i].p_ਵਿਅੰਜਕ;
        }
        ਅਗਲਾ_Case = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_ਕਾਲਮ );
        sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pTest, ਅਗਲਾ_Case, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ );
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, aListelem[i+1].p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
        sqlite3VdbeGoto(v, ਸਿਰਾ_Label);
        sqlite3VdbeResolveLabel(v, ਅਗਲਾ_Case);
      }
      ਜੇ( (n_ਵਿਅੰਜਕ&1)!=0 ){
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pEList->a[n_ਵਿਅੰਜਕ-1].p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, ਨਿਸ਼ਾਨਾ);
      }
      sqlite3_ਵਿਅੰਜਕ_Delete(db, pDel);
      ਸੈੱਟ_Do_Not_ਮਿਲਾਓ_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, ਸਿਰਾ_Label);
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
    ਹਾਲਤ TK_RAISE: {
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ==OE_Rollback
           || p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ==OE_Abort
           || p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ==OE_Fail
           || p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ==OE_Ignore
      );
      ਜੇ( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse,
                       "RAISE() may only be used within a trigger-program");
        ਵਾਪਸ 0;
      }
      ਜੇ( p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      ਜੇ( p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ==OE_Ignore ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }ਨਹੀਂਤਾਂ{
        r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ, r1);
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
  }
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree1);
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree2);
  ਵਾਪਸ ਵਿੱਚ_Reg;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੜੀਵਾਰ_Just_Once(
  Parse *pParse,    /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,      /* The expression to code when the VDBE initializes */
  ਪੂਰਨ regDest       /* Store the value in this register */
){
  ਵਿਅੰਜਕ_List *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( Const_ਗੁਣਕ_Ok(pParse) );
  ਪ੍ਰਤਿਜ੍ਞਾ( regDest!=0 );
  p = pParse->p_Const_ਵਿਅੰਜਕ;
  ਜੇ( regDest<0 && p ){
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
    ਪੂਰਨ i;
    ਲਈ(p_ਵਸਤੂ=p->a, i=p->n_ਵਿਅੰਜਕ; i>0; p_ਵਸਤੂ++, i--){
      ਜੇ( p_ਵਸਤੂ->fg.reusable
       && sqlite3_ਵਿਅੰਜਕ_Compare(0,p_ਵਸਤੂ->p_ਵਿਅੰਜਕ,p_ਵਿਅੰਜਕ,-1)==0
      ){
        ਵਾਪਸ p_ਵਸਤੂ->u.i_Const_ਵਿਅੰਜਕ_Reg;
      }
    }
  }
  p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(pParse->db, p_ਵਿਅੰਜਕ, 0);
  ਜੇ( p_ਵਿਅੰਜਕ!=0 && ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    ਪੂਰਨ addr;
    ਪ੍ਰਤਿਜ੍ਞਾ( v );
    addr = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
    pParse->ok_Const_ਗੁਣਕ = 0;
    ਜੇ( !pParse->db->mallocFailed ){
      ਜੇ( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਵਿਅੰਜਕ, regDest);
    }
    pParse->ok_Const_ਗੁਣਕ = 1;
    sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, p_ਵਿਅੰਜਕ);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
  }ਨਹੀਂਤਾਂ{
    p = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, p, p_ਵਿਅੰਜਕ);
    ਜੇ( p ){
       ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ = &p->a[p->n_ਵਿਅੰਜਕ-1];
       p_ਵਸਤੂ->fg.reusable = regDest<0;
       ਜੇ( regDest<0 ) regDest = ++pParse->nMem;
       p_ਵਸਤੂ->u.i_Const_ਵਿਅੰਜਕ_Reg = regDest;
    }
    pParse->p_Const_ਵਿਅੰਜਕ = p;
  }
  ਵਾਪਸ regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Null_ਰਜਿਸਟਰ_Range(
  Parse *pParse,   /* Parsing context */
  ਪੂਰਨ iReg,        /* First register to set to NULL */
  ਪੂਰਨ nReg         /* Number of sequential registers to NULL out */
){
  u8 ok_Const_ਗੁਣਕ = pParse->ok_Const_ਗੁਣਕ;
  ਵਿਅੰਜਕ t;
  memset(&t, 0, ਆਕਾਰ(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->ok_Const_ਗੁਣਕ = 1;
  sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੜੀਵਾਰ_Just_Once(pParse, &t, iReg);
  pParse->ok_Const_ਗੁਣਕ = ok_Const_ਗੁਣਕ;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ *pReg){
  ਪੂਰਨ r2;
  p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਵਿਅੰਜਕ);
  ਜੇ( Const_ਗੁਣਕ_Ok(pParse)
   && ALWAYS(p_ਵਿਅੰਜਕ!=0)
   && p_ਵਿਅੰਜਕ->op!=TK_ਰਜਿਸਟਰ
   && sqlite3_ਵਿਅੰਜਕ_Is_Constant_Not_Join(pParse, p_ਵਿਅੰਜਕ)
  ){
    *pReg  = 0;
    r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੜੀਵਾਰ_Just_Once(pParse, p_ਵਿਅੰਜਕ, -1);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
    r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, p_ਵਿਅੰਜਕ, r1);
    ਜੇ( r2==r1 ){
      *pReg = r1;
    }ਨਹੀਂਤਾਂ{
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
      *pReg = 0;
    }
  }
  ਵਾਪਸ r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ ਨਿਸ਼ਾਨਾ){
  ਪੂਰਨ ਵਿੱਚ_Reg;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ==0 || !ਵਿਅੰਜਕ_Has_VVAProperty(p_ਵਿਅੰਜਕ,EP_Immutable) );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਨਿਸ਼ਾਨਾ>0 && ਨਿਸ਼ਾਨਾ<=pParse->nMem );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  ਜੇ( pParse->pVdbe==0 ) ਵਾਪਸ;
  ਵਿੱਚ_Reg = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
  ਜੇ( ਵਿੱਚ_Reg!=ਨਿਸ਼ਾਨਾ ){
    u8 op;
    ਵਿਅੰਜਕ *pX = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਵਿਅੰਜਕ);
    testcase( pX!=p_ਵਿਅੰਜਕ );
    ਜੇ( ALWAYS(pX)
     && (ਵਿਅੰਜਕ_Has_Property(pX,EP_Subquery) || pX->op==TK_ਰਜਿਸਟਰ)
    ){
      op = OP_Copy;
    }ਨਹੀਂਤਾਂ{
      op = OP_SCopy;
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(pParse->pVdbe, op, ਵਿੱਚ_Reg, ਨਿਸ਼ਾਨਾ);
  }
}

/*
** Make a transient copy of expression pExpr and then code it using
** sqlite3ExprCode().  This routine works just like sqlite3ExprCode()
** except that the input expression is guaranteed to be unchanged.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Copy(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ ਨਿਸ਼ਾਨਾ){
  sqlite3 *db = pParse->db;
  p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ, 0);
  ਜੇ( !db->mallocFailed ) sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Factorable(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ ਨਿਸ਼ਾਨਾ){
  ਜੇ( pParse->ok_Const_ਗੁਣਕ && sqlite3_ਵਿਅੰਜਕ_Is_Constant_Not_Join(pParse,p_ਵਿਅੰਜਕ) ){
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੜੀਵਾਰ_Just_Once(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Copy(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ);
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(
  Parse *pParse,     /* Parsing context */
  ਵਿਅੰਜਕ_List *pList,   /* The expression list to be coded */
  ਪੂਰਨ ਨਿਸ਼ਾਨਾ,        /* Where to write results */
  ਪੂਰਨ srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  ਪੂਰਨ i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਨਿਸ਼ਾਨਾ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_ਵਿਅੰਜਕ;
  ਜੇ( !Const_ਗੁਣਕ_Ok(pParse) ) flags &= ~SQLITE_ECEL_ਗੁਣਕ;
  ਲਈ(p_ਵਸਤੂ=pList->a, i=0; i<n; i++, p_ਵਸਤੂ++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਵਸਤੂ->p_ਵਿਅੰਜਕ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
    ਜੇ( p_ਵਸਤੂ->fg.bSorterRef ){
      i--;
      n--;
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    ਜੇ( (flags & SQLITE_ECEL_REF)!=0 && (j = p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ)>0 ){
      ਜੇ( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, copyOp, j+srcReg-1, ਨਿਸ਼ਾਨਾ+i);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( (flags & SQLITE_ECEL_ਗੁਣਕ)!=0
           && sqlite3_ਵਿਅੰਜਕ_Is_Constant_Not_Join(pParse,p_ਵਿਅੰਜਕ)
    ){
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੜੀਵਾਰ_Just_Once(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ+i);
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ ਵਿੱਚ_Reg = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, p_ਵਿਅੰਜਕ, ਨਿਸ਼ਾਨਾ+i);
      ਜੇ( ਵਿੱਚ_Reg!=ਨਿਸ਼ਾਨਾ+i ){
        VdbeOp *pOp;
        ਜੇ( copyOp==OP_Copy
         && (pOp=sqlite3_Vdbe_Get_ਆਖਰੀ_Op(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==ਵਿੱਚ_Reg
         && pOp->p2+pOp->p3+1==ਨਿਸ਼ਾਨਾ+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, copyOp, ਵਿੱਚ_Reg, ਨਿਸ਼ਾਨਾ+i);
        }
      }
    }
  }
  ਵਾਪਸ n;
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
ਸਥਿਰ ਖਾਲੀ ਵਿਅੰਜਕ_ਕੋਡ_Between(
  Parse *pParse,    /* Parsing and code generating context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,      /* The BETWEEN expression */
  ਪੂਰਨ dest,         /* Jump destination or storage location */
  ਖਾਲੀ (*xJump)(Parse*,ਵਿਅੰਜਕ*,ਪੂਰਨ,ਪੂਰਨ), /* Action to take */
  ਪੂਰਨ jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  ਵਿਅੰਜਕ ਵਿਅੰਜਕ_And;     /* The AND operator in  x>=y AND x<=z  */
  ਵਿਅੰਜਕ comp_ਖੱਬਾ;    /* The  x>=y  term */
  ਵਿਅੰਜਕ comp_ਸੱਜਾ;   /* The  x<=z  term */
  ਪੂਰਨ regFree1 = 0; /* Temporary use register */
  ਵਿਅੰਜਕ *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_ਖੱਬਾ, 0, ਆਕਾਰ(ਵਿਅੰਜਕ));
  memset(&comp_ਸੱਜਾ, 0, ਆਕਾਰ(ਵਿਅੰਜਕ));
  memset(&ਵਿਅੰਜਕ_And, 0, ਆਕਾਰ(ਵਿਅੰਜਕ));

  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
  pDel = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
  ਜੇ( db->mallocFailed==0 ){
    ਵਿਅੰਜਕ_And.op = TK_AND;
    ਵਿਅੰਜਕ_And.p_ਖੱਬਾ = &comp_ਖੱਬਾ;
    ਵਿਅੰਜਕ_And.p_ਸੱਜਾ = &comp_ਸੱਜਾ;
    comp_ਖੱਬਾ.op = TK_GE;
    comp_ਖੱਬਾ.p_ਖੱਬਾ = pDel;
    comp_ਖੱਬਾ.p_ਸੱਜਾ = p_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ;
    comp_ਸੱਜਾ.op = TK_LE;
    comp_ਸੱਜਾ.p_ਖੱਬਾ = pDel;
    comp_ਸੱਜਾ.p_ਸੱਜਾ = p_ਵਿਅੰਜਕ->x.pList->a[1].p_ਵਿਅੰਜਕ;
    sqlite3_ਵਿਅੰਜਕ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ(pDel, ਵਿਅੰਜਕ_ਕੋਡ_ਸਦਿਸ਼(pParse, pDel, &regFree1));
    ਜੇ( xJump ){
      xJump(pParse, &ਵਿਅੰਜਕ_And, dest, jumpIfNull);
    }ਨਹੀਂਤਾਂ{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, &ਵਿਅੰਜਕ_And, dest);
    }
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree1);
  }
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_ਵਿਅੰਜਕ_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_ਵਿਅੰਜਕ_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_ਵਿਅੰਜਕ_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_ਵਿਅੰਜਕ_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_ਵਿਅੰਜਕ_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_ਵਿਅੰਜਕ_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_ਵਿਅੰਜਕ_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_ਵਿਅੰਜਕ_If_False && jumpIfNull!=0 && regFree1!=0 );
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
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_If_True(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ dest, ਪੂਰਨ jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ op = 0;
  ਪੂਰਨ regFree1 = 0;
  ਪੂਰਨ regFree2 = 0;
  ਪੂਰਨ r1, r2;

  ਪ੍ਰਤਿਜ੍ਞਾ( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ਜੇ( NEVER(v==0) )     ਵਾਪਸ;  /* Existence of VDBE checked by caller */
  ਜੇ( NEVER(p_ਵਿਅੰਜਕ==0) ) ਵਾਪਸ;  /* No way this can happen */
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_VVAProperty(p_ਵਿਅੰਜਕ, EP_Immutable) );
  op = p_ਵਿਅੰਜਕ->op;
  ਚੋਣ( op ){
    ਹਾਲਤ TK_AND:
    ਹਾਲਤ TK_OR: {
      ਵਿਅੰਜਕ *pAlt = sqlite3_ਵਿਅੰਜਕ_Simplified_And_Or(p_ਵਿਅੰਜਕ);
      ਜੇ( pAlt!=p_ਵਿਅੰਜਕ ){
        sqlite3_ਵਿਅੰਜਕ_If_True(pParse, pAlt, dest, jumpIfNull);
      }ਨਹੀਂਤਾਂ{
        ਵਿਅੰਜਕ *p_ਪਹਿਲਾ, *p_ਦੂਜਾ;
        ਜੇ( ਵਿਅੰਜਕ_Eval_Rhs_ਪਹਿਲਾ(p_ਵਿਅੰਜਕ) ){
          p_ਪਹਿਲਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
          p_ਦੂਜਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
        }ਨਹੀਂਤਾਂ{
          p_ਪਹਿਲਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
          p_ਦੂਜਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        }
        ਜੇ( op==TK_AND ){
          ਪੂਰਨ d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਪਹਿਲਾ, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_ਵਿਅੰਜਕ_If_True(pParse, p_ਦੂਜਾ, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }ਨਹੀਂਤਾਂ{
          testcase( jumpIfNull==0 );
          sqlite3_ਵਿਅੰਜਕ_If_True(pParse, p_ਪਹਿਲਾ, dest, jumpIfNull);
          sqlite3_ਵਿਅੰਜਕ_If_True(pParse, p_ਦੂਜਾ, dest, jumpIfNull);
        }
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, dest, jumpIfNull);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_TRUTH: {
      ਪੂਰਨ isNot;      /* IS NOT TRUE or IS NOT FALSE */
      ਪੂਰਨ isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_ਵਿਅੰਜਕ->op2==TK_ISNOT;
      isTrue = sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ਜੇ( isTrue ^ isNot ){
        sqlite3_ਵਿਅੰਜਕ_If_True(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_IS:
    ਹਾਲਤ TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ਹਾਲਤ TK_LT:
    ਹਾਲਤ TK_LE:
    ਹਾਲਤ TK_GT:
    ਹਾਲਤ TK_GE:
    ਹਾਲਤ TK_NE:
    ਹਾਲਤ TK_EQ: {
      ਪੂਰਨ addrIsNull;
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) ) ਜਾਓ default_ਵਿਅੰਜਕ;
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = ਵਿਅੰਜਕ_Compute_Operands(pParse, p_ਵਿਅੰਜਕ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ਨਹੀਂਤਾਂ{
        r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
        r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, &regFree2);
        addrIsNull = 0;
      }
      ਕੋਡ_Compare(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, op,
                  r1, r2, dest, jumpIfNull, ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_Commuted));
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ਜੇ( addrIsNull ){
        ਜੇ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrIsNull);
        }
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ISNULL:
    ਹਾਲਤ TK_NOTNULL: {
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
      ਪ੍ਰਤਿਜ੍ਞਾ( regFree1==0 || regFree1==r1 );
      ਜੇ( regFree1 ) sqlite3_Vdbe_Typeof_ਕਾਲਮ(v, r1);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      ਵਿਅੰਜਕ_ਕੋਡ_Between(pParse, p_ਵਿਅੰਜਕ, dest, sqlite3_ਵਿਅੰਜਕ_If_True, jumpIfNull);
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    ਹਾਲਤ TK_ਵਿੱਚ: {
      ਪੂਰਨ destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ਪੂਰਨ destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ(pParse, p_ਵਿਅੰਜਕ, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਮੂਲ: {
    default_ਵਿਅੰਜਕ:
      ਜੇ( ਵਿਅੰਜਕ_Always_True(p_ਵਿਅੰਜਕ) ){
        sqlite3VdbeGoto(v, dest);
      }ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Always_False(p_ਵਿਅੰਜਕ) ){
        /* No-op */
      }ਨਹੀਂਤਾਂ{
        r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ, &regFree1);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ਰੋਕੋ;
    }
  }
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree1);
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree2); 
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
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_If_False(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ dest, ਪੂਰਨ jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ op = 0;
  ਪੂਰਨ regFree1 = 0;
  ਪੂਰਨ regFree2 = 0;
  ਪੂਰਨ r1, r2;

  ਪ੍ਰਤਿਜ੍ਞਾ( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ਜੇ( NEVER(v==0) ) ਵਾਪਸ; /* Existence of VDBE checked by caller */
  ਜੇ( p_ਵਿਅੰਜਕ==0 )    ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_VVAProperty(p_ਵਿਅੰਜਕ,EP_Immutable) );

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
  op = ((p_ਵਿਅੰਜਕ->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_ISNULL || op==OP_NotNull );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_NOTNULL || op==OP_IsNull );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_NE || op==OP_Eq );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_EQ || op==OP_Ne );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_LT || op==OP_Ge );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_LE || op==OP_Gt );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_GT || op==OP_Le );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_GE || op==OP_Lt );

  ਚੋਣ( p_ਵਿਅੰਜਕ->op ){
    ਹਾਲਤ TK_AND:
    ਹਾਲਤ TK_OR: {
      ਵਿਅੰਜਕ *pAlt = sqlite3_ਵਿਅੰਜਕ_Simplified_And_Or(p_ਵਿਅੰਜਕ);
      ਜੇ( pAlt!=p_ਵਿਅੰਜਕ ){
        sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pAlt, dest, jumpIfNull);
      }ਨਹੀਂਤਾਂ{
        ਵਿਅੰਜਕ *p_ਪਹਿਲਾ, *p_ਦੂਜਾ;
        ਜੇ( ਵਿਅੰਜਕ_Eval_Rhs_ਪਹਿਲਾ(p_ਵਿਅੰਜਕ) ){
          p_ਪਹਿਲਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
          p_ਦੂਜਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
        }ਨਹੀਂਤਾਂ{
          p_ਪਹਿਲਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
          p_ਦੂਜਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        }
        ਜੇ( p_ਵਿਅੰਜਕ->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਪਹਿਲਾ, dest, jumpIfNull);
          sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਦੂਜਾ, dest, jumpIfNull);
        }ਨਹੀਂਤਾਂ{
          ਪੂਰਨ d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_ਵਿਅੰਜਕ_If_True(pParse, p_ਪਹਿਲਾ, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਦੂਜਾ, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_ਵਿਅੰਜਕ_If_True(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, dest, jumpIfNull);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_TRUTH: {
      ਪੂਰਨ isNot;   /* IS NOT TRUE or IS NOT FALSE */
      ਪੂਰਨ isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_ਵਿਅੰਜਕ->op2==TK_ISNOT;
      isTrue = sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ਜੇ( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }ਨਹੀਂਤਾਂ{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_ਵਿਅੰਜਕ_If_True(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_IS:
    ਹਾਲਤ TK_ISNOT:
      testcase( p_ਵਿਅੰਜਕ->op==TK_IS );
      testcase( p_ਵਿਅੰਜਕ->op==TK_ISNOT );
      op = (p_ਵਿਅੰਜਕ->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ਹਾਲਤ TK_LT:
    ਹਾਲਤ TK_LE:
    ਹਾਲਤ TK_GT:
    ਹਾਲਤ TK_GE:
    ਹਾਲਤ TK_NE:
    ਹਾਲਤ TK_EQ: {
      ਪੂਰਨ addrIsNull;
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) ) ਜਾਓ default_ਵਿਅੰਜਕ;
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = ਵਿਅੰਜਕ_Compute_Operands(pParse, p_ਵਿਅੰਜਕ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ਨਹੀਂਤਾਂ{
        r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
        r2 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, &regFree2);
        addrIsNull = 0;
      }
      ਕੋਡ_Compare(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, op,
                  r1, r2, dest, jumpIfNull,ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_Commuted));
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      ਪ੍ਰਤਿਜ੍ਞਾ(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ਜੇ( addrIsNull ){
        ਜੇ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrIsNull);
        }
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ISNULL:
    ਹਾਲਤ TK_NOTNULL: {
      r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, &regFree1);
      ਪ੍ਰਤਿਜ੍ਞਾ( regFree1==0 || regFree1==r1 );
      ਜੇ( regFree1 ) sqlite3_Vdbe_Typeof_ਕਾਲਮ(v, r1);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      ਵਿਅੰਜਕ_ਕੋਡ_Between(pParse, p_ਵਿਅੰਜਕ, dest, sqlite3_ਵਿਅੰਜਕ_If_False, jumpIfNull);
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    ਹਾਲਤ TK_ਵਿੱਚ: {
      ਜੇ( jumpIfNull ){
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ(pParse, p_ਵਿਅੰਜਕ, dest, dest);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿੱਚ(pParse, p_ਵਿਅੰਜਕ, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਮੂਲ: {
    default_ਵਿਅੰਜਕ:
      ਜੇ( ਵਿਅੰਜਕ_Always_False(p_ਵਿਅੰਜਕ) ){
        sqlite3VdbeGoto(v, dest);
      }ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Always_True(p_ਵਿਅੰਜਕ) ){
        /* no-op */
      }ਨਹੀਂਤਾਂ{
        r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, p_ਵਿਅੰਜਕ, &regFree1);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ਰੋਕੋ;
    }
  }
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree1);
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regFree2);
}

/*
** Like sqlite3ExprIfFalse() except that a copy is made of pExpr before
** code generation, and that copy is deleted after code generation. This
** ensures that the original pExpr is unchanged.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_If_False_Dup(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ dest,ਪੂਰਨ jumpIfNull){
  sqlite3 *db = pParse->db;
  ਵਿਅੰਜਕ *pCopy = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ, 0);
  ਜੇ( db->mallocFailed==0 ){
    sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pCopy);
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ ਵਿਅੰਜਕ_Compare_Variable(
  ਅਚਲ Parse *pParse,
  ਅਚਲ ਵਿਅੰਜਕ *pVar,
  ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ
){
  ਪੂਰਨ res = 2;
  ਪੂਰਨ iVar;
  sqlite3_ਮੁੱਲ *pL, *pR = 0;
 
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_VARIABLE && pVar->i_ਕਾਲਮ==p_ਵਿਅੰਜਕ->i_ਕਾਲਮ ){
    ਵਾਪਸ 0;
  }
  ਜੇ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ਵਾਪਸ 2;
  sqlite3_ਮੁੱਲ_From_ਵਿਅੰਜਕ(pParse->db, p_ਵਿਅੰਜਕ, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  ਜੇ( pR ){
    iVar = pVar->i_ਕਾਲਮ;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_ਮੁੱਲ(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    ਜੇ( pL ){
      ਜੇ( sqlite3_ਮੁੱਲ_type(pL)==SQLITE_ਪਾਠ ){
        sqlite3_ਮੁੱਲ_ਪਾਠ(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_ਮੁੱਲ_Free(pR);
    sqlite3_ਮੁੱਲ_Free(pL);
  }
  ਵਾਪਸ res;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Compare(
  ਅਚਲ Parse *pParse,
  ਅਚਲ ਵਿਅੰਜਕ *pA,
  ਅਚਲ ਵਿਅੰਜਕ *pB,
  ਪੂਰਨ iTab
){
  u32 combinedFlags;
  ਜੇ( pA==0 || pB==0 ){
    ਵਾਪਸ pB==pA ? 0 : 2;
  }
  ਜੇ( pParse && pA->op==TK_VARIABLE ){
    ਵਾਪਸ ਵਿਅੰਜਕ_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  ਜੇ( combinedFlags & EP_Int_ਮੁੱਲ ){
    ਜੇ( (pA->flags&pB->flags&EP_Int_ਮੁੱਲ)!=0 && pA->u.i_ਮੁੱਲ==pB->u.i_ਮੁੱਲ ){
      ਵਾਪਸ 0;
    }
    ਵਾਪਸ 2;
  }
  ਜੇ( pA->op!=pB->op || pA->op==TK_RAISE ){
    ਜੇ( pA->op==TK_COLLATE && sqlite3_ਵਿਅੰਜਕ_Compare(pParse, pA->p_ਖੱਬਾ,pB,iTab)<2 ){
      ਵਾਪਸ 1;
    }
    ਜੇ( pB->op==TK_COLLATE && sqlite3_ਵਿਅੰਜਕ_Compare(pParse, pA,pB->p_ਖੱਬਾ,iTab)<2 ){
      ਵਾਪਸ 1;
    }
    ਜੇ( pA->op==TK_AGG_ਕਾਲਮ && pB->op==TK_ਕਾਲਮ
     && pB->i_ਸਾਰਣੀ<0 && pA->i_ਸਾਰਣੀ==iTab
    ){
      /* fall through */
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ 2;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pA, EP_Int_ਮੁੱਲ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pB, EP_Int_ਮੁੱਲ) );
  ਜੇ( pA->u.zToken ){
    ਜੇ( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      ਜੇ( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) ਵਾਪਸ 2;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਪ੍ਰਤਿਜ੍ਞਾ( pA->op==pB->op );
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(pA,EP_WinFunc)!=ਵਿਅੰਜਕ_Has_Property(pB,EP_WinFunc) ){
        ਵਾਪਸ 2;
      }
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(pA,EP_WinFunc) ){
        ਜੇ( sqlite3_ਖਿੜਕੀ_Compare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          ਵਾਪਸ 2;
        }
      }
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ ਜੇ( pA->op==TK_NULL ){
      ਵਾਪਸ 0;
    }ਨਹੀਂਤਾਂ ਜੇ( pA->op==TK_COLLATE ){
      ਜੇ( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) ਵਾਪਸ 2;
    }ਨਹੀਂਤਾਂ
    ਜੇ( pB->u.zToken!=0
     && pA->op!=TK_ਕਾਲਮ
     && pA->op!=TK_AGG_ਕਾਲਮ
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      ਵਾਪਸ 2;
    }
  }
  ਜੇ( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) ਵਾਪਸ 2;
  ਜੇ( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    ਜੇ( combinedFlags & EP_xIsSelect ) ਵਾਪਸ 2;
    ਜੇ( (combinedFlags & EP_Fixed_ਖਾਨਾ)==0
     && sqlite3_ਵਿਅੰਜਕ_Compare(pParse, pA->p_ਖੱਬਾ, pB->p_ਖੱਬਾ, iTab) ) ਵਾਪਸ 2;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(pParse, pA->p_ਸੱਜਾ, pB->p_ਸੱਜਾ, iTab) ) ਵਾਪਸ 2;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_List_Compare(pA->x.pList, pB->x.pList, iTab) ) ਵਾਪਸ 2;
    ਜੇ( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      ਜੇ( pA->i_ਕਾਲਮ!=pB->i_ਕਾਲਮ ) ਵਾਪਸ 2;
      ਜੇ( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) ਵਾਪਸ 2;
      ਜੇ( pA->op!=TK_ਵਿੱਚ && pA->i_ਸਾਰਣੀ!=pB->i_ਸਾਰਣੀ && pA->i_ਸਾਰਣੀ!=iTab ){
        ਵਾਪਸ 2;
      }
    }
  }
  ਵਾਪਸ 0;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_List_Compare(ਅਚਲ ਵਿਅੰਜਕ_List *pA, ਅਚਲ ਵਿਅੰਜਕ_List *pB, ਪੂਰਨ iTab){
  ਪੂਰਨ i;
  ਜੇ( pA==0 && pB==0 ) ਵਾਪਸ 0;
  ਜੇ( pA==0 || pB==0 ) ਵਾਪਸ 1;
  ਜੇ( pA->n_ਵਿਅੰਜਕ!=pB->n_ਵਿਅੰਜਕ ) ਵਾਪਸ 1;
  ਲਈ(i=0; i<pA->n_ਵਿਅੰਜਕ; i++){
    ਪੂਰਨ res;
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ_A = pA->a[i].p_ਵਿਅੰਜਕ;
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ_B = pB->a[i].p_ਵਿਅੰਜਕ;
    ਜੇ( pA->a[i].fg.ਛਾਂਟੋ_Flags!=pB->a[i].fg.ਛਾਂਟੋ_Flags ) ਵਾਪਸ 1;
    ਜੇ( (res = sqlite3_ਵਿਅੰਜਕ_Compare(0, p_ਵਿਅੰਜਕ_A, p_ਵਿਅੰਜਕ_B, iTab)) ) ਵਾਪਸ res;
  }
  ਵਾਪਸ 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Compare_Skip(ਵਿਅੰਜਕ *pA,ਵਿਅੰਜਕ *pB, ਪੂਰਨ iTab){
  ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_Compare(0,
             sqlite3_ਵਿਅੰਜਕ_Skip_Collate(pA),
             sqlite3_ਵਿਅੰਜਕ_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(
  ਅਚਲ Parse *pParse,/* Parsing context */
  ਅਚਲ ਵਿਅੰਜਕ *p,      /* The expression to be checked */
  ਅਚਲ ਵਿਅੰਜਕ *pNN,    /* The expression that is NOT NULL */
  ਪੂਰਨ iTab,           /* Table being evaluated */
  ਪੂਰਨ ਵੇਖਿਆ_Not         /* Return true only if p can be any non-NULL value */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
  ਪ੍ਰਤਿਜ੍ਞਾ( pNN );
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(pParse, p, pNN, iTab)==0 ){
    ਵਾਪਸ pNN->op!=TK_NULL;
  }
  ਚੋਣ( p->op ){
    ਹਾਲਤ TK_ਵਿੱਚ: {
      ਜੇ( ਵੇਖਿਆ_Not && ਵਿਅੰਜਕ_Has_Property(p, EP_xIsSelect) ) ਵਾਪਸ 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_ਵਿਅੰਜਕ>0) );
      ਵਾਪਸ ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, p->p_ਖੱਬਾ, pNN, iTab, 1);
    }
    ਹਾਲਤ TK_BETWEEN: {
      ਵਿਅੰਜਕ_List *pList;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p) );
      pList = p->x.pList;
      ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pList->n_ਵਿਅੰਜਕ==2 );
      ਜੇ( ਵੇਖਿਆ_Not ) ਵਾਪਸ 0;
      ਜੇ( ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, pList->a[0].p_ਵਿਅੰਜਕ, pNN, iTab, 1)
       || ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, pList->a[1].p_ਵਿਅੰਜਕ, pNN, iTab, 1)
      ){
        ਵਾਪਸ 1;
      }
      ਵਾਪਸ ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, p->p_ਖੱਬਾ, pNN, iTab, 1);
    }
    ਹਾਲਤ TK_EQ:
    ਹਾਲਤ TK_NE:
    ਹਾਲਤ TK_LT:
    ਹਾਲਤ TK_LE:
    ਹਾਲਤ TK_GT:
    ਹਾਲਤ TK_GE:
    ਹਾਲਤ TK_PLUS:
    ਹਾਲਤ TK_MINUS:
    ਹਾਲਤ TK_BITOR:
    ਹਾਲਤ TK_LSHIFT:
    ਹਾਲਤ TK_RSHIFT:
    ਹਾਲਤ TK_CONCAT:
      ਵੇਖਿਆ_Not = 1;
      /* no break */ deliberate_fall_through
    ਹਾਲਤ TK_ਤਾਰਾ:
    ਹਾਲਤ TK_REM:
    ਹਾਲਤ TK_BITAND:
    ਹਾਲਤ TK_SLASH: {
      ਜੇ( ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, p->p_ਸੱਜਾ, pNN, iTab, ਵੇਖਿਆ_Not) ) ਵਾਪਸ 1;
      /* no break */ deliberate_fall_through
    }
    ਹਾਲਤ TK_ਵਿਸਤਾਰ:
    ਹਾਲਤ TK_COLLATE:
    ਹਾਲਤ TK_UPLUS:
    ਹਾਲਤ TK_UMINUS: {
      ਵਾਪਸ ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, p->p_ਖੱਬਾ, pNN, iTab, ਵੇਖਿਆ_Not);
    }
    ਹਾਲਤ TK_TRUTH: {
      ਜੇ( ਵੇਖਿਆ_Not ) ਵਾਪਸ 0;
      ਜੇ( p->op2!=TK_IS ) ਵਾਪਸ 0;
      ਵਾਪਸ ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, p->p_ਖੱਬਾ, pNN, iTab, 1);
    }
    ਹਾਲਤ TK_BITNOT:
    ਹਾਲਤ TK_NOT: {
      ਵਾਪਸ ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, p->p_ਖੱਬਾ, pNN, iTab, 1);
    }
  }
  ਵਾਪਸ 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Not_True(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ v;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_NULL ) ਵਾਪਸ 1;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_TRUEFALSE && sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(p_ਵਿਅੰਜਕ)==0 ) ਵਾਪਸ 1;
  v = 1;
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Integer(p_ਵਿਅੰਜਕ, &v, 0, 0) && v==0 ) ਵਾਪਸ 1;
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_IIF(sqlite3 *db, ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਵਿਅੰਜਕ_List *pList;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION ){
    ਅਚਲ ਅੱਖਰ *z = p_ਵਿਅੰਜਕ->u.zToken;
    FuncDef *pDef;
    ਜੇ( (z[0]!='i' && z[0]!='I') ) ਵਾਪਸ 0;
    ਜੇ( p_ਵਿਅੰਜਕ->x.pList==0 ) ਵਾਪਸ 0;
    pDef = sqlite3_ਲੱਭੋ_Function(db, z, p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ, ENC(db), 0);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    ਜੇ( pDef==0 ) ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
    ਜੇ( NEVER(pDef==0) ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
    ਜੇ( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) ਵਾਪਸ 0;
    ਜੇ( SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(pDef->p_User_ਅੰਕੜੇ)!=INLINEFUNC_iif ) ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_CASE ){
    ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ!=0 ) ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
  pList = p_ਵਿਅੰਜਕ->x.pList;
  ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
  ਜੇ( pList->n_ਵਿਅੰਜਕ==2 ) ਵਾਪਸ 1;
  ਜੇ( pList->n_ਵਿਅੰਜਕ==3 && sqlite3_ਵਿਅੰਜਕ_Is_Not_True(pList->a[2].p_ਵਿਅੰਜਕ) ) ਵਾਪਸ 1;
  ਵਾਪਸ 0;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ(
  ਅਚਲ Parse *pParse,
  ਅਚਲ ਵਿਅੰਜਕ *pE1,
  ਅਚਲ ਵਿਅੰਜਕ *pE2,
  ਪੂਰਨ iTab
){
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(pParse, pE1, pE2, iTab)==0 ){
    ਵਾਪਸ 1;
  }
  ਜੇ( pE2->op==TK_OR
   && (sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ(pParse, pE1, pE2->p_ਖੱਬਾ, iTab)
             || sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ(pParse, pE1, pE2->p_ਸੱਜਾ, iTab) )
  ){
    ਵਾਪਸ 1;
  }
  ਜੇ( pE2->op==TK_NOTNULL
   && ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null(pParse, pE1, pE2->p_ਖੱਬਾ, iTab, 0)
  ){
    ਵਾਪਸ 1;
  }
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_IIF(pParse->db, pE1) ){
    ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ(pParse,pE1->x.pList->a[0].p_ਵਿਅੰਜਕ,pE2,iTab);
  }
  ਵਾਪਸ 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ਸਥਿਰ ਖਾਲੀ both_Imply_Not_Null_ਪੰਕਤੀ(Walker *pWalker, ਵਿਅੰਜਕ *pE1, ਵਿਅੰਜਕ *pE2){
  ਜੇ( pWalker->e_ਕੋਡ==0 ){
    sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pE1);
    ਜੇ( pWalker->e_ਕੋਡ ){
      pWalker->e_ਕੋਡ = 0;
      sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pE2);
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
ਸਥਿਰ ਪੂਰਨ ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null_ਪੰਕਤੀ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  testcase( p_ਵਿਅੰਜਕ->op==TK_AGG_ਕਾਲਮ );
  testcase( p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION );
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) ) ਵਾਪਸ WRC_Prune;
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    ਵਾਪਸ WRC_Prune;
  }
  ਚੋਣ( p_ਵਿਅੰਜਕ->op ){
    ਹਾਲਤ TK_ISNOT:
    ਹਾਲਤ TK_ISNULL:
    ਹਾਲਤ TK_NOTNULL:
    ਹਾਲਤ TK_IS:
    ਹਾਲਤ TK_ਸਦਿਸ਼:
    ਹਾਲਤ TK_FUNCTION:
    ਹਾਲਤ TK_TRUTH:
    ਹਾਲਤ TK_CASE:
      testcase( p_ਵਿਅੰਜਕ->op==TK_ISNOT );
      testcase( p_ਵਿਅੰਜਕ->op==TK_ISNULL );
      testcase( p_ਵਿਅੰਜਕ->op==TK_NOTNULL );
      testcase( p_ਵਿਅੰਜਕ->op==TK_IS );
      testcase( p_ਵਿਅੰਜਕ->op==TK_ਸਦਿਸ਼ );
      testcase( p_ਵਿਅੰਜਕ->op==TK_FUNCTION );
      testcase( p_ਵਿਅੰਜਕ->op==TK_TRUTH );
      testcase( p_ਵਿਅੰਜਕ->op==TK_CASE );
      ਵਾਪਸ WRC_Prune;

    ਹਾਲਤ TK_ਕਾਲਮ:
      ਜੇ( pWalker->u.iCur==p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ ){
        pWalker->e_ਕੋਡ = 1;
        ਵਾਪਸ WRC_Abort;
      }
      ਵਾਪਸ WRC_Prune;

    ਹਾਲਤ TK_OR:
    ਹਾਲਤ TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_ਵਿਅੰਜਕ->op==TK_OR );
      testcase( p_ਵਿਅੰਜਕ->op==TK_AND );
      both_Imply_Not_Null_ਪੰਕਤੀ(pWalker, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
      ਵਾਪਸ WRC_Prune;
       
    ਹਾਲਤ TK_ਵਿੱਚ:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      ਜੇ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) && ALWAYS(p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ>0) ){
        sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
      }
      ਵਾਪਸ WRC_Prune;

    ਹਾਲਤ TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ==2 );
      sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
      both_Imply_Not_Null_ਪੰਕਤੀ(pWalker, p_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ,
                                   p_ਵਿਅੰਜਕ->x.pList->a[1].p_ਵਿਅੰਜਕ);
      ਵਾਪਸ WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    ਹਾਲਤ TK_EQ:
    ਹਾਲਤ TK_NE:
    ਹਾਲਤ TK_LT:
    ਹਾਲਤ TK_LE:
    ਹਾਲਤ TK_GT:
    ਹਾਲਤ TK_GE: {
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
      ਵਿਅੰਜਕ *p_ਸੱਜਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
      testcase( p_ਵਿਅੰਜਕ->op==TK_EQ );
      testcase( p_ਵਿਅੰਜਕ->op==TK_NE );
      testcase( p_ਵਿਅੰਜਕ->op==TK_LT );
      testcase( p_ਵਿਅੰਜਕ->op==TK_LE );
      testcase( p_ਵਿਅੰਜਕ->op==TK_GT );
      testcase( p_ਵਿਅੰਜਕ->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ->op!=TK_ਕਾਲਮ || ਵਿਅੰਜਕ_Use_YTab(p_ਖੱਬਾ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸੱਜਾ->op!=TK_ਕਾਲਮ || ਵਿਅੰਜਕ_Use_YTab(p_ਸੱਜਾ) );
      ਜੇ( (p_ਖੱਬਾ->op==TK_ਕਾਲਮ
           && ALWAYS(p_ਖੱਬਾ->y.pTab!=0)
           && IsVirtual(p_ਖੱਬਾ->y.pTab))
       || (p_ਸੱਜਾ->op==TK_ਕਾਲਮ
           && ALWAYS(p_ਸੱਜਾ->y.pTab!=0)
           && IsVirtual(p_ਸੱਜਾ->y.pTab))
      ){
        ਵਾਪਸ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    ਮੂਲ:
      ਵਾਪਸ WRC_Continue;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Non_Null_ਪੰਕਤੀ(ਵਿਅੰਜਕ *p, ਪੂਰਨ iTab, ਪੂਰਨ isRJ){
  Walker w;
  p = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p);
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  ਜੇ( p->op==TK_NOTNULL ){
    p = p->p_ਖੱਬਾ;
  }ਨਹੀਂਤਾਂ{
    ਜਦੋਂ( p->op==TK_AND ){
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Non_Null_ਪੰਕਤੀ(p->p_ਖੱਬਾ, iTab, isRJ) ) ਵਾਪਸ 1;
      p = p->p_ਸੱਜਾ;
    }
  }
  w.x_ਵਿਅੰਜਕ_Callback = ਸਿੱਟਾ_ਦਿੰਦਾ_Not_Null_ਪੰਕਤੀ;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.e_ਕੋਡ = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p);
  ਵਾਪਸ w.e_ਕੋਡ;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
ਬਣਤਰ IdxCover {
  ਸੂਚਕਾਂਕ *pIdx;     /* The index to be tested for coverage */
  ਪੂਰਨ iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Idx_Cover(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
   && p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==pWalker->u.pIdxCover->iCur
   && sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pWalker->u.pIdxCover->pIdx, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ)<0
  ){
    pWalker->e_ਕੋਡ = 1;
    ਵਾਪਸ WRC_Abort;
  }
  ਵਾਪਸ WRC_Continue;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Covered_By_ਸੂਚਕਾਂਕ(
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,        /* The index to be tested */
  ਪੂਰਨ iCur,           /* The cursor number for the corresponding table */
  ਸੂਚਕਾਂਕ *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  ਬਣਤਰ IdxCover xcov;
  memset(&w, 0, ਆਕਾਰ(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p_ਵਿਅੰਜਕ);
  ਵਾਪਸ !w.e_ਕੋਡ;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
ਬਣਤਰ RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  ਪੂਰਨ *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
ਸਥਿਰ ਪੂਰਨ selectRefEnter(Walker *pWalker, Select *pSelect){
  ਬਣਤਰ RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  ਪੂਰਨ *piNew;
  ਜੇ( pSrc->nSrc==0 ) ਵਾਪਸ WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*ਆਕਾਰ(ਪੂਰਨ));
  ਜੇ( piNew==0 ){
    p->nExclude = 0;
    ਵਾਪਸ WRC_Abort;
  }ਨਹੀਂਤਾਂ{
    p->aiExclude = piNew;
  }
  ਲਈ(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  ਵਾਪਸ WRC_Continue;
}
ਸਥਿਰ ਖਾਲੀ selectRefLeave(Walker *pWalker, Select *pSelect){
  ਬਣਤਰ RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  ਜੇ( p->nExclude ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->nExclude>=pSrc->nSrc );
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
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Ref_ਮੰਜ਼ਿਲ_Src_List(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
   || p_ਵਿਅੰਜਕ->op==TK_AGG_ਕਾਲਮ
  ){
    ਪੂਰਨ i;
    ਬਣਤਰ RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    ਪੂਰਨ nSrc = pSrc ? pSrc->nSrc : 0;
    ਲਈ(i=0; i<nSrc; i++){
      ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==pSrc->a[i].iCursor ){
        pWalker->e_ਕੋਡ |= 1;
        ਵਾਪਸ WRC_Continue;
      }
    }
    ਲਈ(i=0; i<p->nExclude && p->aiExclude[i]!=p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ; i++){}
    ਜੇ( i>=p->nExclude ){
      pWalker->e_ਕੋਡ |= 2;
    }
  }
  ਵਾਪਸ WRC_Continue;
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
ਪੂਰਨ sqlite3ReferencesSrcList(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, SrcList *pSrcList){
  Walker w;
  ਬਣਤਰ RefSrcList x;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db!=0 );
  memset(&w, 0, ਆਕਾਰ(w));
  memset(&x, 0, ਆਕਾਰ(x));
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_Ref_ਮੰਜ਼ਿਲ_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
  sqlite3_Walk_ਵਿਅੰਜਕ_List(&w, p_ਵਿਅੰਜਕ->x.pList);
  ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਕ੍ਰਮ );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pList!=0 );
    sqlite3_Walk_ਵਿਅੰਜਕ_List(&w, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pList);
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
    sqlite3_Walk_ਵਿਅੰਜਕ(&w, p_ਵਿਅੰਜਕ->y.pWin->pFilter);
  }
#ਜੇ_ਅੰਤ
  ਜੇ( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  ਜੇ( w.e_ਕੋਡ & 0x01 ){
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ ਜੇ( w.e_ਕੋਡ ){
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ -1;
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
ਸਥਿਰ ਪੂਰਨ agginfo_Persist_ਵਿਅੰਜਕ_Cb(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( ALWAYS(!ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly|EP_Reduced))
   && p_ਵਿਅੰਜਕ->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_ਵਿਅੰਜਕ->pAggInfo;
    ਪੂਰਨ iAgg = p_ਵਿਅੰਜਕ->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    ਪ੍ਰਤਿਜ੍ਞਾ( iAgg>=0 );
    ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_AGG_FUNCTION ){
      ਜੇ( iAgg<pAggInfo->n_ਕਾਲਮ
       && pAggInfo->a_ਖਾਨਾ[iAgg].pCExpr==p_ਵਿਅੰਜਕ
      ){
        p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ, 0);
        ਜੇ( p_ਵਿਅੰਜਕ && !sqlite3_ਵਿਅੰਜਕ_Deferred_Delete(pParse, p_ਵਿਅੰਜਕ) ){
          pAggInfo->a_ਖਾਨਾ[iAgg].pCExpr = p_ਵਿਅੰਜਕ;
        }
      }
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION );
      ਜੇ( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_ਵਿਅੰਜਕ
      ){
        p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ, 0);
        ਜੇ( p_ਵਿਅੰਜਕ && !sqlite3_ਵਿਅੰਜਕ_Deferred_Delete(pParse, p_ਵਿਅੰਜਕ) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_ਵਿਅੰਜਕ;
        }
      }
    }
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
ਖਾਲੀ sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, ਆਕਾਰ(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_ਵਿਅੰਜਕ_Callback = agginfo_Persist_ਵਿਅੰਜਕ_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_Agg_Info_ਕਾਲਮ(sqlite3 *db, AggInfo *pInfo){
  ਪੂਰਨ i;
  pInfo->a_ਖਾਨਾ = sqlite3ArrayAllocate(
       db,
       pInfo->a_ਖਾਨਾ,
       ਆਕਾਰ(pInfo->a_ਖਾਨਾ[0]),
       &pInfo->n_ਕਾਲਮ,
       &i
  );
  ਵਾਪਸ i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_Agg_Info_Func(sqlite3 *db, AggInfo *pInfo){
  ਪੂਰਨ i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       ਆਕਾਰ(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  ਵਾਪਸ i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
ਸਥਿਰ ਖਾਲੀ ਲੱਭੋ_Or_Create_Agg_Info_ਕਾਲਮ(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ          /* Expr describing the column to find or insert */
){
  ਬਣਤਰ Agg_Info_ਖਾਨਾ *p_ਖਾਨਾ;
  ਪੂਰਨ k;
  ਪੂਰਨ mx_ਪਦ = pParse->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ];

  ਪ੍ਰਤਿਜ੍ਞਾ( mx_ਪਦ <= SMXV(i16) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->i_ਪਹਿਲਾ_Reg==0 );
  p_ਖਾਨਾ = pAggInfo->a_ਖਾਨਾ;
  ਲਈ(k=0; k<pAggInfo->n_ਕਾਲਮ; k++, p_ਖਾਨਾ++){
    ਜੇ( p_ਖਾਨਾ->pCExpr==p_ਵਿਅੰਜਕ ) ਵਾਪਸ;
    ਜੇ( p_ਖਾਨਾ->i_ਸਾਰਣੀ==p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ
     && p_ਖਾਨਾ->i_ਕਾਲਮ==p_ਵਿਅੰਜਕ->i_ਕਾਲਮ
     && p_ਵਿਅੰਜਕ->op!=TK_IF_NULL_ਪੰਕਤੀ
    ){
      ਜਾਓ fix_ਉੱਪਰ_ਵਿਅੰਜਕ;
    }
  }
  k = ਜੋੜੋ_Agg_Info_ਕਾਲਮ(pParse->db, pAggInfo);
  ਜੇ( k<0 ){
    /* OOM on resize */
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed );
    ਵਾਪਸ;
  }
  ਜੇ( k>mx_ਪਦ ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "more than %d aggregate terms", mx_ਪਦ);
    k = mx_ਪਦ;
  }
  p_ਖਾਨਾ = &pAggInfo->a_ਖਾਨਾ[k];
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
  p_ਖਾਨਾ->pTab = p_ਵਿਅੰਜਕ->y.pTab;
  p_ਖਾਨਾ->i_ਸਾਰਣੀ = p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;
  p_ਖਾਨਾ->i_ਕਾਲਮ = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
  p_ਖਾਨਾ->i_Sorter_ਕਾਲਮ = -1;
  p_ਖਾਨਾ->pCExpr = p_ਵਿਅੰਜਕ;
  ਜੇ( pAggInfo->p_ਸਮੂਹ_By && p_ਵਿਅੰਜਕ->op!=TK_IF_NULL_ਪੰਕਤੀ ){
    ਪੂਰਨ j, n;
    ਵਿਅੰਜਕ_List *pGB = pAggInfo->p_ਸਮੂਹ_By;
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਪਦ = pGB->a;
    n = pGB->n_ਵਿਅੰਜਕ;
    ਲਈ(j=0; j<n; j++, p_ਪਦ++){
      ਵਿਅੰਜਕ *pE = p_ਪਦ->p_ਵਿਅੰਜਕ;
      ਜੇ( pE->op==TK_ਕਾਲਮ
       && pE->i_ਸਾਰਣੀ==p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ
       && pE->i_ਕਾਲਮ==p_ਵਿਅੰਜਕ->i_ਕਾਲਮ
      ){
        p_ਖਾਨਾ->i_Sorter_ਕਾਲਮ = j;
        ਰੋਕੋ;
      }
    }
  }
  ਜੇ( p_ਖਾਨਾ->i_Sorter_ਕਾਲਮ<0 ){
    p_ਖਾਨਾ->i_Sorter_ਕਾਲਮ = pAggInfo->n_ਛਾਂਟੀ_ਕਾਲਮ++;
  }
fix_ਉੱਪਰ_ਵਿਅੰਜਕ:
  ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(p_ਵਿਅੰਜਕ, EP_NoReduce);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->pAggInfo==0 || p_ਵਿਅੰਜਕ->pAggInfo==pAggInfo );
  p_ਵਿਅੰਜਕ->pAggInfo = pAggInfo;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ ){
    p_ਵਿਅੰਜਕ->op = TK_AGG_ਕਾਲਮ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( k <= SMXV(p_ਵਿਅੰਜਕ->iAgg) );
  p_ਵਿਅੰਜਕ->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ਸਥਿਰ ਪੂਰਨ analyzeAggregate(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  ਪ੍ਰਤਿਜ੍ਞਾ( pNC->ncFlags & NC_UAggInfo );
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->i_ਪਹਿਲਾ_Reg==0 );
  ਚੋਣ( p_ਵਿਅੰਜਕ->op ){
    ਮੂਲ: {
      ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *pIEpr;
      ਵਿਅੰਜਕ tmp;
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->iSelfTab==0 );
      ਜੇ( (pNC->ncFlags & NC_ਵਿੱਚ_Agg_Func)==0 ) ਰੋਕੋ;
      ਜੇ( pParse->pIdxEpr==0 ) ਰੋਕੋ;
      ਲਈ(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        ਪੂਰਨ i_ਅੰਕੜੇ_Cur = pIEpr->i_ਅੰਕੜੇ_Cur;
        ਜੇ( i_ਅੰਕੜੇ_Cur<0 ) ਜਾਰੀ;
        ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, p_ਵਿਅੰਜਕ, pIEpr->p_ਵਿਅੰਜਕ, i_ਅੰਕੜੇ_Cur)==0 ) ਰੋਕੋ;
      }
      ਜੇ( pIEpr==0 ) ਰੋਕੋ;
      ਜੇ( NEVER(!ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ)) ) ਰੋਕੋ;
      ਲਈ(i=0; i<pSrcList->nSrc; i++){
         ਜੇ( pSrcList->a[i].iCursor==pIEpr->i_ਅੰਕੜੇ_Cur ){
           testcase( i>0 );
           ਰੋਕੋ;
         }
      }
      ਜੇ( i>=pSrcList->nSrc ) ਰੋਕੋ;
      ਜੇ( NEVER(p_ਵਿਅੰਜਕ->pAggInfo!=0) ) ਰੋਕੋ; /* Resolved by outer context */
      ਜੇ( pParse->nErr ){ ਵਾਪਸ WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, ਆਕਾਰ(tmp));
      tmp.op = TK_AGG_ਕਾਲਮ;
      tmp.i_ਸਾਰਣੀ = pIEpr->iIdxCur;
      tmp.i_ਕਾਲਮ = pIEpr->i_Idx_ਖਾਨਾ;
      ਲੱਭੋ_Or_Create_Agg_Info_ਕਾਲਮ(pParse, pAggInfo, &tmp);
      ਜੇ( pParse->nErr ){ ਵਾਪਸ WRC_Abort; }
      ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->a_ਖਾਨਾ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( tmp.iAgg<pAggInfo->n_ਕਾਲਮ );
      pAggInfo->a_ਖਾਨਾ[tmp.iAgg].pCExpr = p_ਵਿਅੰਜਕ;
      p_ਵਿਅੰਜਕ->pAggInfo = pAggInfo;
      p_ਵਿਅੰਜਕ->iAgg = tmp.iAgg;
      ਵਾਪਸ WRC_Prune;
    }
    ਹਾਲਤ TK_IF_NULL_ਪੰਕਤੀ:
    ਹਾਲਤ TK_AGG_ਕਾਲਮ:
    ਹਾਲਤ TK_ਕਾਲਮ: {
      testcase( p_ਵਿਅੰਜਕ->op==TK_AGG_ਕਾਲਮ );
      testcase( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ );
      testcase( p_ਵਿਅੰਜਕ->op==TK_IF_NULL_ਪੰਕਤੀ );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      ਜੇ( ALWAYS(pSrcList!=0) ){
        Src_ਵਸਤੂ *p_ਵਸਤੂ = pSrcList->a;
        ਲਈ(i=0; i<pSrcList->nSrc; i++, p_ਵਸਤੂ++){
          ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly|EP_Reduced) );
          ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==p_ਵਸਤੂ->iCursor ){
            ਲੱਭੋ_Or_Create_Agg_Info_ਕਾਲਮ(pParse, pAggInfo, p_ਵਿਅੰਜਕ);
            ਰੋਕੋ;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      ਵਾਪਸ WRC_Continue;
    }
    ਹਾਲਤ TK_AGG_FUNCTION: {
      ਜੇ( (pNC->ncFlags & NC_ਵਿੱਚ_Agg_Func)==0
       && pWalker->walker_ਡੂੰਘਾਈ==p_ਵਿਅੰਜਕ->op2
       && p_ਵਿਅੰਜਕ->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        ਬਣਤਰ AggInfo_func *p_ਵਸਤੂ = pAggInfo->aFunc;
        ਪੂਰਨ mx_ਪਦ = pParse->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ];
        ਪ੍ਰਤਿਜ੍ਞਾ( mx_ਪਦ <= SMXV(i16) );
        ਲਈ(i=0; i<pAggInfo->nFunc; i++, p_ਵਸਤੂ++){
          ਜੇ( NEVER(p_ਵਸਤੂ->pFExpr==p_ਵਿਅੰਜਕ) ) ਰੋਕੋ;
          ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, p_ਵਸਤੂ->pFExpr, p_ਵਿਅੰਜਕ, -1)==0 ){
            ਰੋਕੋ;
          }
        }
        ਜੇ( i>mx_ਪਦ ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "more than %d aggregate terms", mx_ਪਦ);
          i = mx_ਪਦ;
          ਪ੍ਰਤਿਜ੍ਞਾ( i<pAggInfo->nFunc );
        }ਨਹੀਂਤਾਂ ਜੇ( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = ਜੋੜੋ_Agg_Info_Func(pParse->db, pAggInfo);
          ਜੇ( i>=0 ){
            ਪੂਰਨ nArg;
            ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_xIsSelect) );
            p_ਵਸਤੂ = &pAggInfo->aFunc[i];
            p_ਵਸਤੂ->pFExpr = p_ਵਿਅੰਜਕ;
            ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_UToken(p_ਵਿਅੰਜਕ) );
            nArg = p_ਵਿਅੰਜਕ->x.pList ? p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ : 0;
            p_ਵਸਤੂ->pFunc = sqlite3_ਲੱਭੋ_Function(pParse->db,
                                         p_ਵਿਅੰਜਕ->u.zToken, nArg, enc, 0);
            ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->bOBUnique==0 );
            ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ
             && (p_ਵਸਤੂ->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              ਵਿਅੰਜਕ_List *pOBList;
              ਪ੍ਰਤਿਜ੍ਞਾ( nArg>0 );
              ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਕ੍ਰਮ );
              ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) );
              p_ਵਸਤੂ->iOBTab = pParse->nTab++;
              pOBList = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pList;
              ਪ੍ਰਤਿਜ੍ਞਾ( pOBList->n_ਵਿਅੰਜਕ>0 );
              ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->bOBUnique==0 );
              ਜੇ( pOBList->n_ਵਿਅੰਜਕ==1
               && nArg==1
               && sqlite3_ਵਿਅੰਜਕ_Compare(0,pOBList->a[0].p_ਵਿਅੰਜਕ,
                               p_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ,-1)==0
              ){
                p_ਵਸਤੂ->bOBPayload = 0;
                p_ਵਸਤੂ->bOBUnique = ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Distinct);
              }ਨਹੀਂਤਾਂ{
                p_ਵਸਤੂ->bOBPayload = 1;
              }
              p_ਵਸਤੂ->bUseSubtype =
                    (p_ਵਸਤੂ->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }ਨਹੀਂਤਾਂ{
              p_ਵਸਤੂ->iOBTab = -1;
            }
            ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Distinct) && !p_ਵਸਤੂ->bOBUnique ){
              p_ਵਸਤੂ->iDistinct = pParse->nTab++;
            }ਨਹੀਂਤਾਂ{
              p_ਵਸਤੂ->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly|EP_Reduced) );
        ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(p_ਵਿਅੰਜਕ, EP_NoReduce);
        ਪ੍ਰਤਿਜ੍ਞਾ( i <= SMXV(p_ਵਿਅੰਜਕ->iAgg) );
        p_ਵਿਅੰਜਕ->iAgg = (i16)i;
        p_ਵਿਅੰਜਕ->pAggInfo = pAggInfo;
        ਵਾਪਸ WRC_Prune;
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ WRC_Continue;
      }
    }
  }
  ਵਾਪਸ WRC_Continue;
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
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Analyze_Aggregates(NameContext *pNC, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  Walker w;
  w.x_ਵਿਅੰਜਕ_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3_Walker_ਡੂੰਘਾਈ_Increase;
  w.xSelectCallback2 = sqlite3_Walker_ਡੂੰਘਾਈ_Decrease;
  w.walker_ਡੂੰਘਾਈ = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNC->pSrcList!=0 );
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p_ਵਿਅੰਜਕ);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Analyze_Agg_List(NameContext *pNC, ਵਿਅੰਜਕ_List *pList){
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  ਪੂਰਨ i;
  ਜੇ( pList ){
    ਲਈ(p_ਵਸਤੂ=pList->a, i=0; i<pList->n_ਵਿਅੰਜਕ; i++, p_ਵਸਤੂ++){
      sqlite3_ਵਿਅੰਜਕ_Analyze_Aggregates(pNC, p_ਵਸਤੂ->p_ਵਿਅੰਜਕ);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
ਪੂਰਨ sqlite3_Get_ਆਰਜ਼ੀ_Reg(Parse *pParse){
  ਜੇ( pParse->n_ਆਰਜ਼ੀ_Reg==0 ){
    ਵਾਪਸ ++pParse->nMem;
  }
  ਵਾਪਸ pParse->a_ਆਰਜ਼ੀ_Reg[--pParse->n_ਆਰਜ਼ੀ_Reg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
ਖਾਲੀ sqlite3_Release_ਆਰਜ਼ੀ_Reg(Parse *pParse, ਪੂਰਨ iReg){
  ਜੇ( iReg ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    ਪੂਰਨ kk;
    ਲਈ(kk=0; kk<pParse->n_ਆਰਜ਼ੀ_Reg; kk++){
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->a_ਆਰਜ਼ੀ_Reg[kk]!=iReg );
    }
#ਜੇ_ਅੰਤ
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    ਜੇ( pParse->n_ਆਰਜ਼ੀ_Reg<Array_ਮਾਪ(pParse->a_ਆਰਜ਼ੀ_Reg) ){
      pParse->a_ਆਰਜ਼ੀ_Reg[pParse->n_ਆਰਜ਼ੀ_Reg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
ਪੂਰਨ sqlite3_Get_ਆਰਜ਼ੀ_Range(Parse *pParse, ਪੂਰਨ nReg){
  ਪੂਰਨ i, n;
  ਜੇ( nReg==1 ) ਵਾਪਸ sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  ਜੇ( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }ਨਹੀਂਤਾਂ{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  ਵਾਪਸ i;
}
ਖਾਲੀ sqlite3_Release_ਆਰਜ਼ੀ_Range(Parse *pParse, ਪੂਰਨ iReg, ਪੂਰਨ nReg){
  ਜੇ( nReg==1 ){
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, iReg);
    ਵਾਪਸ;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  ਜੇ( nReg>pParse->nRangeReg ){
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
ਖਾਲੀ sqlite3_Clear_ਆਰਜ਼ੀ_Reg_Cache(Parse *pParse){
  pParse->n_ਆਰਜ਼ੀ_Reg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
ਖਾਲੀ sqlite3_Touch_ਰਜਿਸਟਰ(Parse *pParse, ਪੂਰਨ iReg){
  ਜੇ( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#ਜੇ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
ਪੂਰਨ sqlite3_ਪਹਿਲਾ_ਉਪਲਬਧ_ਰਜਿਸਟਰ(Parse *pParse, ਪੂਰਨ iMin){
  ਅਚਲ ਵਿਅੰਜਕ_List *pList = pParse->p_Const_ਵਿਅੰਜਕ;
  ਜੇ( pList ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
      ਜੇ( pList->a[i].u.i_Const_ਵਿਅੰਜਕ_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_ਵਿਅੰਜਕ_Reg + 1;
      }
    }
  }
  pParse->n_ਆਰਜ਼ੀ_Reg = 0;
  pParse->nRangeReg = 0;
  ਵਾਪਸ iMin;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ sqlite3_No_Temps_ਵਿੱਚ_Range(Parse *pParse, ਪੂਰਨ i_ਪਹਿਲਾ, ਪੂਰਨ i_ਆਖਰੀ){
  ਪੂਰਨ i;
  ਜੇ( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > i_ਪਹਿਲਾ
   && pParse->iRangeReg <= i_ਆਖਰੀ
  ){
     ਵਾਪਸ 0;
  }
  ਲਈ(i=0; i<pParse->n_ਆਰਜ਼ੀ_Reg; i++){
    ਜੇ( pParse->a_ਆਰਜ਼ੀ_Reg[i]>=i_ਪਹਿਲਾ && pParse->a_ਆਰਜ਼ੀ_Reg[i]<=i_ਆਖਰੀ ){
      ਵਾਪਸ 0;
    }
  }
  ਜੇ( pParse->p_Const_ਵਿਅੰਜਕ ){
    ਵਿਅੰਜਕ_List *pList = pParse->p_Const_ਵਿਅੰਜਕ;
    ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
      ਪੂਰਨ iReg = pList->a[i].u.i_Const_ਵਿਅੰਜਕ_Reg;
      ਜੇ( iReg==0 ) ਜਾਰੀ;
      ਜੇ( iReg>=i_ਪਹਿਲਾ && iReg<=i_ਆਖਰੀ ) ਵਾਪਸ 0;
    }
  }
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */
