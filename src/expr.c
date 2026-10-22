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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

/* Forward declarations */
ସ୍ଥିର ଶୂନ୍ୟ ପଦସମଷ୍ଟି_Code_Between(Parse*,ପଦସମଷ୍ଟି*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(Parse*,ପଦସମଷ୍ଟି*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା),ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Code_Vector(Parse *pParse, ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
ଅକ୍ଷର sqlite3TableColumnAffinity(ଅଚଳ Table *pTab, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol){
  ଯଦି( iCol<0 || NEVER(iCol>=pTab->nCol) ) ଫେରନ୍ତୁ SQLITE_AFF_INTEGER;
  ଫେରନ୍ତୁ pTab->aCol[iCol].affinity;
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
ଅକ୍ଷର sqlite3_ପଦସମଷ୍ଟି_Affinity(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
  op = p_ପଦସମଷ୍ଟି->op;
  ଯେତେବେଳେ( 1 /* exit-by-break */ ){
    ଯଦି( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_ପଦସମଷ୍ଟି->y.pTab!=0) ){
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->y.pTab!=0 );
      ଫେରନ୍ତୁ sqlite3TableColumnAffinity(p_ପଦସମଷ୍ଟି->y.pTab, p_ପଦସମଷ୍ଟି->iColumn);
    }
    ଯଦି( op==TK_SELECT ){
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pSelect!=0 );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pSelect->pEList!=0 );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pSelect->pEList->a[0].p_ପଦସମଷ୍ଟି!=0 );
      ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି->x.pSelect->pEList->a[0].p_ପଦସମଷ୍ଟି);
    }
#ifndef SQLITE_OMIT_CAST
    ଯଦି( op==TK_CAST ){
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      ଫେରନ୍ତୁ sqlite3AffinityType(p_ପଦସମଷ୍ଟି->u.zToken, 0);
    }
#endif
    ଯଦି( op==TK_SELECT_COLUMN ){
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ!=0 && ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି->p_ବାମ) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iColumn < p_ପଦସମଷ୍ଟି->iTable );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iColumn >= 0 );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iTable==p_ପଦସମଷ୍ଟି->p_ବାମ->x.pSelect->pEList->n_ପଦସମଷ୍ଟି );
      ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Affinity(
          p_ପଦସମଷ୍ଟି->p_ବାମ->x.pSelect->pEList->a[p_ପଦସମଷ୍ଟି->iColumn].p_ପଦସମଷ୍ଟି
      );
    }
    ଯଦି( op==TK_VECTOR
     || (op==TK_FUNCTION && p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି==SQLITE_AFF_DEFER)
    ){
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
      ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି->x.pList->a[0].p_ପଦସମଷ୍ଟି);
    }
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Skip|EP_IfNullRow) ){
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_COLLATE
           || p_ପଦସମଷ୍ଟି->op==TK_IF_NULL_ROW
           || (p_ପଦସମଷ୍ଟି->op==TK_REGISTER && p_ପଦସମଷ୍ଟି->op2==TK_IF_NULL_ROW) );
      p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ବାମ;
      op = p_ପଦସମଷ୍ଟି->op;
      ଜାରି;
    }
    ଯଦି( op!=TK_REGISTER ) ଭାଙ୍ଗ;
    op = p_ପଦସମଷ୍ଟି->op2;
    ଯଦି( NEVER( op==TK_REGISTER ) ) ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Data_Type(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯେତେବେଳେ( p_ପଦସମଷ୍ଟି ){
    ବାଛ( p_ପଦସମଷ୍ଟି->op ){
      ସ୍ଥିତି TK_COLLATE:
      ସ୍ଥିତି TK_IF_NULL_ROW:
      ସ୍ଥିତି TK_UPLUS:  {
        p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ବାମ;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_NULL: {
        p_ପଦସମଷ୍ଟି = 0;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_STRING: {
        ଫେରନ୍ତୁ 0x02;
      }
      ସ୍ଥିତି TK_BLOB: {
        ଫେରନ୍ତୁ 0x04;
      }
      ସ୍ଥିତି TK_CONCAT: {
        ଫେରନ୍ତୁ 0x06;
      }
      ସ୍ଥିତି TK_VARIABLE:
      ସ୍ଥିତି TK_AGG_FUNCTION:
      ସ୍ଥିତି TK_FUNCTION: {
        ଫେରନ୍ତୁ 0x07;
      }
      ସ୍ଥିତି TK_COLUMN:
      ସ୍ଥିତି TK_AGG_COLUMN:
      ସ୍ଥିତି TK_SELECT:
      ସ୍ଥିତି TK_CAST:
      ସ୍ଥିତି TK_SELECT_COLUMN:
      ସ୍ଥିତି TK_VECTOR:  {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା aff = sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି);
        ଯଦି( aff>=SQLITE_AFF_NUMERIC ) ଫେରନ୍ତୁ 0x05;
        ଯଦି( aff==SQLITE_AFF_TEXT )    ଫେରନ୍ତୁ 0x06;
        ଫେରନ୍ତୁ 0x07;
      }
      ସ୍ଥିତି TK_CASE: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
        ପଦସମଷ୍ଟି_List *pList = p_ପଦସମଷ୍ଟି->x.pList;
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) && pList!=0 );
        ପ୍ରତିଜ୍ଞା( pList->n_ପଦସମଷ୍ଟି > 0);
        ପାଇଁ(ii=1; ii<pList->n_ପଦସମଷ୍ଟି; ii+=2){
          res |= sqlite3_ପଦସମଷ୍ଟି_Data_Type(pList->a[ii].p_ପଦସମଷ୍ଟି);
        }
        ଯଦି( pList->n_ପଦସମଷ୍ଟି % 2 ){
          res |= sqlite3_ପଦସମଷ୍ଟି_Data_Type(pList->a[pList->n_ପଦସମଷ୍ଟି-1].p_ପଦସମଷ୍ଟି);
        }
        ଫେରନ୍ତୁ res;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        ଫେରନ୍ତୁ 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  ଫେରନ୍ତୁ 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,             /* Add the "COLLATE" clause to this expression */
  ଅଚଳ Token *pCollName,  /* Name of collating sequence */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dequote              /* True to dequote pCollName */
){
  ଯଦି( pCollName->n>0 && pParse->nErr==0 ){
    ପଦସମଷ୍ଟି *pNew = sqlite3_ପଦସମଷ୍ଟି_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
    ଯଦି( pNew ){
      pNew->p_ବାମ = p_ପଦସମଷ୍ଟି;
      pNew->flags |= EP_Collate|EP_Skip;
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
      pNew->nHeight = p_ପଦସମଷ୍ଟି->nHeight+1;
      sqlite3_ପଦସମଷ୍ଟି_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_ପଦସମଷ୍ଟି = pNew;
    }
  }
  ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି;
}
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,          /* Add the "COLLATE" clause to this expression */
  ଅଚଳ ଅକ୍ଷର *zC        /* The collating sequence name */
){
  Token s;
  ପ୍ରତିଜ୍ଞା( zC!=0 );
  sqlite3TokenInit(&s, (ଅକ୍ଷର*)zC);
  ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Add_Collate_Token(pParse, p_ପଦସମଷ୍ଟି, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Skip_Collate(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯେତେବେଳେ( p_ପଦସମଷ୍ଟି && ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Skip) ){
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_COLLATE );
    p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ବାମ;
  }  
  ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯେତେବେଳେ( p_ପଦସମଷ୍ଟି && ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Skip|EP_Unlikely) ){
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Unlikely) ){
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି>0 );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION );
      p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->x.pList->a[0].p_ପଦସମଷ୍ଟି;
    }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLLATE ){
      p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ବାମ;
    }ଅନ୍ୟଥା{
      ଭାଙ୍ଗ;
    }
  }  
  ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି;
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
CollSeq *sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(Parse *pParse, ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ଅଚଳ ପଦସମଷ୍ଟି *p = p_ପଦସମଷ୍ଟି;
  ଯେତେବେଳେ( p ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = p->op;
    ଯଦି( op==TK_REGISTER ) op = p->op2;
    ଯଦି( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p) );
      ପ୍ରତିଜ୍ଞା( p->y.pTab!=0 );
      ଯଦି( (j = p->iColumn)>=0 ){
        ଅଚଳ ଅକ୍ଷର *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      ଭାଙ୍ଗ;
    }
    ଯଦି( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_ବାମ;
      ଜାରି;
    }
    ଯଦି( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_ପଦସମଷ୍ଟି==SQLITE_AFF_DEFER)
    ){
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p) );
      p = p->x.pList->a[0].p_ପଦସମଷ୍ଟି;
      ଜାରି;
    }
    ଯଦି( op==TK_COLLATE ){
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p, EP_Int_ମୂଲ୍ୟ) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      ଭାଙ୍ଗ;
    }
    ଯଦି( p->flags & EP_Collate ){
      ଯଦି( p->p_ବାମ && (p->p_ବାମ->flags & EP_Collate)!=0 ){
        p = p->p_ବାମ;
      }ଅନ୍ୟଥା{
        ପଦସମଷ୍ଟି *pNext  = p->p_ଡାହାଣ;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_XList(p) || p->x.pList==0 || p->p_ଡାହାଣ==0 );
        ଯଦି( ପଦସମଷ୍ଟି_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
          ପାଇଁ(i=0; i<p->x.pList->n_ପଦସମଷ୍ଟି; i++){
            ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p->x.pList->a[i].p_ପଦସମଷ୍ଟି, EP_Collate) ){
              pNext = p->x.pList->a[i].p_ପଦସମଷ୍ଟି;
              ଭାଙ୍ଗ;
            }
          }
        }
        p = pNext;
      }
    }ଅନ୍ୟଥା{
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  ଫେରନ୍ତୁ pColl;
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
CollSeq *sqlite3_ପଦସମଷ୍ଟି_NNColl_Seq(Parse *pParse, ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  CollSeq *p = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p_ପଦସମଷ୍ଟି);
  ଯଦି( p==0 ) p = pParse->db->pDfltColl;
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ଫେରନ୍ତୁ p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Coll_Seq_Match(Parse *pParse, ଅଚଳ ପଦସମଷ୍ଟି *pE1, ଅଚଳ ପଦସମଷ୍ଟି *pE2){
  CollSeq *pColl1 = sqlite3_ପଦସମଷ୍ଟି_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_ପଦସମଷ୍ଟି_NNColl_Seq(pParse, pE2);
  ପ୍ରତିଜ୍ଞା( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  ଫେରନ୍ତୁ pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
ଅକ୍ଷର sqlite3CompareAffinity(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ଅକ୍ଷର aff2){
  ଅକ୍ଷର aff1 = sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି);
  ଯଦି( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    ଯଦି( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      ଫେରନ୍ତୁ SQLITE_AFF_NUMERIC;
    }ଅନ୍ୟଥା{
      ଫେରନ୍ତୁ SQLITE_AFF_BLOB;
    }
  }ଅନ୍ୟଥା{
    /* One side is a column, the other is not. Use the columns affinity. */
    ପ୍ରତିଜ୍ଞା( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    ଫେରନ୍ତୁ (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
ସ୍ଥିର ଅକ୍ଷର comparisonAffinity(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଅକ୍ଷର aff;
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_EQ || p_ପଦସମଷ୍ଟି->op==TK_IN || p_ପଦସମଷ୍ଟି->op==TK_LT ||
          p_ପଦସମଷ୍ଟି->op==TK_GT || p_ପଦସମଷ୍ଟି->op==TK_GE || p_ପଦସମଷ୍ଟି->op==TK_LE ||
          p_ପଦସମଷ୍ଟି->op==TK_NE || p_ପଦସମଷ୍ଟି->op==TK_IS || p_ପଦସମଷ୍ଟି->op==TK_ISNOT );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
  aff = sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି->p_ବାମ);
  ଯଦି( p_ପଦସମଷ୍ଟି->p_ଡାହାଣ ){
    aff = sqlite3CompareAffinity(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, aff);
  }ଅନ୍ୟଥା ଯଦି( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  ଫେରନ୍ତୁ aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3IndexAffinityOk(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ଅକ୍ଷର idx_affinity){
  ଅକ୍ଷର aff = comparisonAffinity(p_ପଦସମଷ୍ଟି);
  ଯଦି( aff<SQLITE_AFF_TEXT ){
    ଫେରନ୍ତୁ 1;
  }
  ଯଦି( aff==SQLITE_AFF_TEXT ){
    ଫେରନ୍ତୁ idx_affinity==SQLITE_AFF_TEXT;
  }
  ଫେରନ୍ତୁ sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ସ୍ଥିର u8 ଦ୍ୱିମିକ_Compare_P5(
  ଅଚଳ ପଦସମଷ୍ଟି *pExpr1,   /* Left operand */
  ଅଚଳ ପଦସମଷ୍ଟି *pExpr2,   /* Right operand */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (ଅକ୍ଷର)sqlite3_ପଦସମଷ୍ଟି_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  ଫେରନ୍ତୁ aff;
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
CollSeq *sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(
  Parse *pParse,
  ଅଚଳ ପଦସମଷ୍ଟି *p_ବାମ,
  ଅଚଳ ପଦସମଷ୍ଟି *p_ଡାହାଣ
){
  CollSeq *pColl;
  ପ୍ରତିଜ୍ଞା( p_ବାମ );
  ଯଦି( p_ବାମ->flags & EP_Collate ){
    pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p_ବାମ);
  }ଅନ୍ୟଥା ଯଦି( p_ଡାହାଣ && (p_ଡାହାଣ->flags & EP_Collate)!=0 ){
    pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p_ଡାହାଣ);
  }ଅନ୍ୟଥା{
    pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p_ବାମ);
    ଯଦି( !pColl ){
      pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p_ଡାହାଣ);
    }
  }
  ଫେରନ୍ତୁ pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_ପଦସମଷ୍ଟି_Compare_Coll_Seq(Parse *pParse, ଅଚଳ ପଦସମଷ୍ଟି *p){
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p, EP_Commuted) ){
    ଫେରନ୍ତୁ sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(pParse, p->p_ଡାହାଣ, p->p_ବାମ);
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(pParse, p->p_ବାମ, p->p_ଡାହାଣ);
  }
}

/*
** Generate code for a comparison operator.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  ପଦସମଷ୍ଟି *p_ବାମ,      /* The left operand */
  ପଦସମଷ୍ଟି *p_ଡାହାଣ,     /* The right operand */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା opcode,       /* The comparison opcode */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା in1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା in2, /* Register holding operands */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dest,         /* Jump here if true.  */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା jumpIfNull,   /* If true, jump if either operand is NULL */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isCommuted    /* The comparison has been commuted */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା p5;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;
  CollSeq *p4;

  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ 0;
  ଯଦି( isCommuted ){
    p4 = sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(pParse, p_ଡାହାଣ, p_ବାମ);
  }ଅନ୍ୟଥା{
    p4 = sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(pParse, p_ବାମ, p_ଡାହାଣ);
  }
  p5 = ଦ୍ୱିମିକ_Compare_P5(p_ବାମ, p_ଡାହାଣ, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (ଶୂନ୍ୟ*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  ଫେରନ୍ତୁ addr;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Vector(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Vector_Size(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  u8 op = p_ପଦସମଷ୍ଟି->op;
  ଯଦି( op==TK_REGISTER ) op = p_ପଦସମଷ୍ଟି->op2;
  ଯଦି( op==TK_VECTOR ){
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
    ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି;
  }ଅନ୍ୟଥା ଯଦି( op==TK_SELECT ){
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
    ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି->x.pSelect->pEList->n_ପଦସମଷ୍ଟି;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 1;
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
ପଦସମଷ୍ଟି *sqlite3VectorFieldSubexpr(ପଦସମଷ୍ଟି *pVector, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  ପ୍ରତିଜ୍ଞା( i<sqlite3_ପଦସମଷ୍ଟି_Vector_Size(pVector) || pVector->op==TK_ERROR );
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Vector(pVector) ){
    ପ୍ରତିଜ୍ଞା( pVector->op2==0 || pVector->op==TK_REGISTER );
    ଯଦି( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(pVector) );
      ଫେରନ୍ତୁ pVector->x.pSelect->pEList->a[i].p_ପଦସମଷ୍ଟି;
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pVector) );
      ଫେରନ୍ତୁ pVector->x.pList->a[i].p_ପଦସମଷ୍ଟି;
    }
  }
  ଫେରନ୍ତୁ pVector;
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
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  ପଦସମଷ୍ଟି *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iField,          /* Which column of the vector to return */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nField           /* Total number of columns in the vector */
){
  ପଦସମଷ୍ଟି *pRet;
  ଯଦି( pVector->op==TK_SELECT ){
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(pVector) );
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
    ଯଦି( pRet ){
      ପଦସମଷ୍ଟି_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_ବାମ = pVector;
    }
  }ଅନ୍ୟଥା{
    ଯଦି( pVector->op==TK_VECTOR ){
      ପଦସମଷ୍ଟି **ppVector;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_ପଦସମଷ୍ଟି;
      pVector = *ppVector;
      ଯଦି( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        ଫେରନ୍ତୁ pVector;
      }
    }
    pRet = sqlite3_ପଦସମଷ୍ଟି_Dup(pParse->db, pVector, 0);
  }
  ଫେରନ୍ତୁ pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Code_Subselect(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_ପଦସମଷ୍ଟି);
  }
#endif
  ଫେରନ୍ତୁ reg;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Vector_Register(
  Parse *pParse,                  /* Parse context */
  ପଦସମଷ୍ଟି *pVector,                  /* Vector to extract element from */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iField,                     /* Field to extract from pVector */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regSelect,                  /* First in array of registers */
  ପଦସମଷ୍ଟି *pTmp,                     /* Temporary space */
  ପଦସମଷ୍ଟି **pp_ପଦସମଷ୍ଟି,                  /* OUT: Expression element */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  ପ୍ରତିଜ୍ଞା( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  ଯଦି( op==TK_REGISTER ){
    *pp_ପଦସମଷ୍ଟି = sqlite3VectorFieldSubexpr(pVector, iField);
    ଫେରନ୍ତୁ pVector->iTable+iField;
  }
  ଯଦି( op==TK_SELECT ){
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, ଆକାର(ପଦସମଷ୍ଟି));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_ବାମ = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_ପଦସମଷ୍ଟି;
    *pp_ପଦସମଷ୍ଟି = pTmp;
    ଫେରନ୍ତୁ regSelect+iField;
  }
  ଯଦି( op==TK_VECTOR ){
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pVector) );
    *pp_ପଦସମଷ୍ଟି = pVector->x.pList->a[iField].p_ପଦସମଷ୍ଟି;
    ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, *pp_ପଦସମଷ୍ଟି, pRegFree);
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ଶୂନ୍ୟ codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,          /* The comparison operation */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
  ପଦସମଷ୍ଟି *p_ଡାହାଣ = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ବାମ = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ବାମ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg_ବାମ = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg_ଡାହାଣ = 0;
  u8 opx = op;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrCmp = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrDone = sqlite3VdbeMakeLabel(pParse);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isCommuted = ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_Commuted);

  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_VVAProperty(p_ପଦସମଷ୍ଟି,EP_Immutable) );
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  ଯଦି( n_ବାମ!=sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ଡାହାଣ) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "row value misused");
    ଫେରନ୍ତୁ;
  }
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_EQ || p_ପଦସମଷ୍ଟି->op==TK_NE
       || p_ପଦସମଷ୍ଟି->op==TK_IS || p_ପଦସମଷ୍ଟି->op==TK_ISNOT
       || p_ପଦସମଷ୍ଟି->op==TK_LT || p_ପଦସମଷ୍ଟି->op==TK_GT
       || p_ପଦସମଷ୍ଟି->op==TK_LE || p_ପଦସମଷ୍ଟି->op==TK_GE
  );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==op || (p_ପଦସମଷ୍ଟି->op==TK_IS && op==TK_EQ)
            || (p_ପଦସମଷ୍ଟି->op==TK_ISNOT && op==TK_NE) );
  ପ୍ରତିଜ୍ଞା( p5==0 || p_ପଦସମଷ୍ଟି->op!=op );
  ପ୍ରତିଜ୍ଞା( p5==SQLITE_NULLEQ || p_ପଦସମଷ୍ଟି->op==op );

  ଯଦି( op==TK_LE ) opx = TK_LT;
  ଯଦି( op==TK_GE ) opx = TK_GT;
  ଯଦି( op==TK_NE ) opx = TK_EQ;

  reg_ବାମ = ପଦସମଷ୍ଟି_Code_Subselect(pParse, p_ବାମ);
  reg_ଡାହାଣ = ପଦସମଷ୍ଟି_Code_Subselect(pParse, p_ଡାହାଣ);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  ପାଇଁ(i=0; 1 /*Loop exits by "break"*/; i++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା regFree1 = 0, regFree2 = 0;
    ପଦସମଷ୍ଟି *pL = 0, *pR = 0;
    ପଦସମଷ୍ଟି tmp1, tmp2;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2;
    ପ୍ରତିଜ୍ଞା( i>=0 && i<n_ବାମ );
    ଯଦି( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = ପଦସମଷ୍ଟି_Vector_Register(pParse, p_ବାମ, i, reg_ବାମ, &tmp1, &pL, &regFree1);
    r2 = ପଦସମଷ୍ଟି_Vector_Register(pParse, p_ଡାହାଣ, i, reg_ଡାହାଣ, &tmp2, &pR, &regFree2);
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
    ଯଦି( (opx==TK_LT || opx==TK_GT) && i<n_ବାମ-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    ଯଦି( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }ଅନ୍ୟଥା{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    ଯଦି( i==n_ବାମ-1 ){
      ଭାଙ୍ଗ;
    }
    ଯଦି( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      ଯଦି( i==n_ବାମ-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  ଯଦି( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Check_Height(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nHeight){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mxHeight = pParse->db->aLimit[SQLITE_LIMIT_ପଦସମଷ୍ଟି_DEPTH];
  ଯଦି( nHeight>mxHeight ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ଶୂନ୍ୟ height_Of_ପଦସମଷ୍ଟି(ଅଚଳ ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnHeight){
  ଯଦି( p ){
    ଯଦି( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
ସ୍ଥିର ଶୂନ୍ୟ height_Of_ପଦସମଷ୍ଟି_List(ଅଚଳ ପଦସମଷ୍ଟି_List *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnHeight){
  ଯଦି( p ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<p->n_ପଦସମଷ୍ଟି; i++){
      height_Of_ପଦସମଷ୍ଟି(p->a[i].p_ପଦସମଷ୍ଟି, pnHeight);
    }
  }
}
ସ୍ଥିର ଶୂନ୍ୟ heightOfSelect(ଅଚଳ Select *pSelect, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnHeight){
  ଅଚଳ Select *p;
  ପାଇଁ(p=pSelect; p; p=p->pPrior){
    height_Of_ପଦସମଷ୍ଟି(p->pWhere, pnHeight);
    height_Of_ପଦସମଷ୍ଟି(p->pHaving, pnHeight);
    height_Of_ପଦସମଷ୍ଟି(p->pLimit, pnHeight);
    height_Of_ପଦସମଷ୍ଟି_List(p->pEList, pnHeight);
    height_Of_ପଦସମଷ୍ଟି_List(p->pGroupBy, pnHeight);
    height_Of_ପଦସମଷ୍ଟି_List(p->pOrderBy, pnHeight);
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
ସ୍ଥିର ଶୂନ୍ୟ ପଦସମଷ୍ଟି_Set_Height(ପଦସମଷ୍ଟି *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nHeight = p->p_ବାମ ? p->p_ବାମ->nHeight : 0;
  ଯଦି( NEVER(p->p_ଡାହାଣ) && p->p_ଡାହାଣ->nHeight>nHeight ){
    nHeight = p->p_ଡାହାଣ->nHeight;
  }
  ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }ଅନ୍ୟଥା ଯଦି( p->x.pList ){
    height_Of_ପଦସମଷ୍ଟି_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_ପଦସମଷ୍ଟି_List_Flags(p->x.pList);
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Set_Height_And_Flags(Parse *pParse, ପଦସମଷ୍ଟି *p){
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  ପଦସମଷ୍ଟି_Set_Height(p);
  sqlite3_ପଦସମଷ୍ଟି_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Select_ପଦସମଷ୍ଟି_Height(ଅଚଳ Select *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nHeight = 0;
  heightOfSelect(p, &nHeight);
  ଫେରନ୍ତୁ nHeight;
}
#ଅନ୍ୟଥା /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Set_Height_And_Flags(Parse *pParse, ପଦସମଷ୍ଟି *p){
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  ଯଦି( p && ପଦସମଷ୍ଟି_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_ପଦସମଷ୍ଟି_List_Flags(p->x.pList);
  }
}
#define ପଦସମଷ୍ଟି_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Set_Error_Offset(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOfst){
  ଯଦି( p_ପଦସମଷ୍ଟି==0 ) ଫେରନ୍ତୁ;
  ଯଦି( NEVER(ପଦସମଷ୍ଟି_Use_WJoin(p_ପଦସମଷ୍ଟି)) ) ଫେରନ୍ତୁ;
  p_ପଦସମଷ୍ଟି->w.iOfst = iOfst;
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
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,                 /* Expression opcode */
  ଅଚଳ Token *pToken,    /* Token argument.  Might be NULL */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dequote             /* True to dequote */
){
  ପଦସମଷ୍ଟି *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nExtra = pToken ? pToken->n+1 : 0;

  ପ୍ରତିଜ୍ଞା( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, ଆକାର(ପଦସମଷ୍ଟି)+nExtra);
  ଯଦି( pNew ){
    memset(pNew, 0, ଆକାର(ପଦସମଷ୍ଟି));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    ଯଦି( nExtra ){
      ପ୍ରତିଜ୍ଞା( pToken!=0 );
      pNew->u.zToken = (ଅକ୍ଷର*)&pNew[1];
      ପ୍ରତିଜ୍ଞା( pToken->z!=0 || pToken->n==0 );
      ଯଦି( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      ଯଦି( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_ପଦସମଷ୍ଟି(pNew);
      }
    }
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  ଫେରନ୍ତୁ pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,                 /* Expression opcode */
  ଅଚଳ ଅକ୍ଷର *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Int32(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iVal){
  ପଦସମଷ୍ଟି *pNew = sqlite3DbMallocRawNN(db, ଆକାର(ପଦସମଷ୍ଟି));
  ଯଦି( pNew ){
    memset(pNew, 0, ଆକାର(ପଦସମଷ୍ଟି));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_ମୂଲ୍ୟ|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_ମୂଲ୍ୟ = iVal;
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  ଫେରନ୍ତୁ pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Attach_Subtrees(
  sqlite3 *db,
  ପଦସମଷ୍ଟି *p_ମୂଳ,
  ପଦସମଷ୍ଟି *p_ବାମ,
  ପଦସମଷ୍ଟି *p_ଡାହାଣ
){
  ଯଦି( p_ମୂଳ==0 ){
    ପ୍ରତିଜ୍ଞା( db->mallocFailed );
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ବାମ);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ଡାହାଣ);
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ମୂଳ) );
    ପ୍ରତିଜ୍ଞା( p_ମୂଳ->x.pSelect==0 );
    ଯଦି( p_ଡାହାଣ ){
      p_ମୂଳ->p_ଡାହାଣ = p_ଡାହାଣ;
      p_ମୂଳ->flags |= EP_Propagate & p_ଡାହାଣ->flags;
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
      p_ମୂଳ->nHeight = p_ଡାହାଣ->nHeight+1;
    }ଅନ୍ୟଥା{
      p_ମୂଳ->nHeight = 1;
#endif
    }
    ଯଦି( p_ବାମ ){
      p_ମୂଳ->p_ବାମ = p_ବାମ;
      p_ମୂଳ->flags |= EP_Propagate & p_ବାମ->flags;
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
      ଯଦି( p_ବାମ->nHeight>=p_ମୂଳ->nHeight ){
        p_ମୂଳ->nHeight = p_ବାମ->nHeight+1;
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
ପଦସମଷ୍ଟି *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,                 /* Expression opcode */
  ପଦସମଷ୍ଟି *p_ବାମ,            /* Left operand */
  ପଦସମଷ୍ଟି *p_ଡାହାଣ            /* Right operand */
){
  ପଦସମଷ୍ଟି *p;
  p = sqlite3DbMallocRawNN(pParse->db, ଆକାର(ପଦସମଷ୍ଟି));
  ଯଦି( p ){
    memset(p, 0, ଆକାର(ପଦସମଷ୍ଟି));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_ପଦସମଷ୍ଟି_Attach_Subtrees(pParse->db, p, p_ବାମ, p_ଡାହାଣ);
    sqlite3_ପଦସମଷ୍ଟି_Check_Height(pParse, p->nHeight);
  }ଅନ୍ୟଥା{
    sqlite3_ପଦସମଷ୍ଟି_Delete(pParse->db, p_ବାମ);
    sqlite3_ପଦସମଷ୍ଟି_Delete(pParse->db, p_ଡାହାଣ);
  }
  ଫେରନ୍ତୁ p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
ଶୂନ୍ୟ sqlite3PExprAddSelect(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, Select *pSelect){
  ଯଦି( p_ପଦସମଷ୍ଟି ){
    p_ପଦସମଷ୍ଟି->x.pSelect = pSelect;
    ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_xIsSelect|EP_Subquery);
    sqlite3_ପଦସମଷ୍ଟି_Set_Height_And_Flags(pParse, p_ପଦସମଷ୍ଟି);
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( pParse->db->mallocFailed );
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
Select *sqlite3_ପଦସମଷ୍ଟି_List_To_ମୂଲ୍ୟଗୁଡ଼ିକ(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nElem, ପଦସମଷ୍ଟି_List *pEList){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  Select *pRet = 0;
  ପ୍ରତିଜ୍ଞା( nElem>1 );
  ପାଇଁ(ii=0; ii<pEList->n_ପଦସମଷ୍ଟି; ii++){
    Select *pSel;
    ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pEList->a[ii].p_ପଦସମଷ୍ଟି;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ପଦସମଷ୍ଟି_Elem;
    ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_VECTOR ){
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
      n_ପଦସମଷ୍ଟି_Elem = p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି;
    }ଅନ୍ୟଥା{
      n_ପଦସମଷ୍ଟି_Elem = 1;
    }
    ଯଦି( n_ପଦସମଷ୍ଟି_Elem!=nElem ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "IN(...) element has %d term%s - expected %d",
          n_ପଦସମଷ୍ଟି_Elem, n_ପଦସମଷ୍ଟି_Elem>1?"s":"", nElem
      );
      ଭାଙ୍ଗ;
    }
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
    pSel = sqlite3SelectNew(pParse, p_ପଦସମଷ୍ଟି->x.pList, 0, 0, 0, 0, 0, SF_ମୂଲ୍ୟଗୁଡ଼ିକ,0);
    p_ପଦସମଷ୍ଟି->x.pList = 0;
    ଯଦି( pSel ){
      ଯଦି( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  ଯଦି( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_ମୂଲ୍ୟ;
  }
  sqlite3_ପଦସମଷ୍ଟି_List_Delete(pParse->db, pEList);
  ଫେରନ୍ତୁ pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_And(Parse *pParse, ପଦସମଷ୍ଟି *p_ବାମ, ପଦସମଷ୍ଟି *p_ଡାହାଣ){
  sqlite3 *db = pParse->db;
  ଯଦି( p_ବାମ==0  ){
    ଫେରନ୍ତୁ p_ଡାହାଣ;
  }ଅନ୍ୟଥା ଯଦି( p_ଡାହାଣ==0 ){
    ଫେରନ୍ତୁ p_ବାମ;
  }ଅନ୍ୟଥା{
    u32 f = p_ବାମ->flags | p_ଡାହାଣ->flags;
    ଯଦି( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      ପଦସମଷ୍ଟି *p;
      sqlite3_ପଦସମଷ୍ଟି_Deferred_Delete(pParse, p_ବାମ);
      sqlite3_ପଦସମଷ୍ଟି_Deferred_Delete(pParse, p_ଡାହାଣ);
      p = sqlite3_ପଦସମଷ୍ଟି_Int32(db, 0);
      ଯଦି( p ) p->op2 = 1;
      ଫେରନ୍ତୁ p;
    }ଅନ୍ୟଥା{
      ଫେରନ୍ତୁ sqlite3PExpr(pParse, TK_AND, p_ବାମ, p_ଡାହାଣ);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Function(
  Parse *pParse,        /* Parsing context */
  ପଦସମଷ୍ଟି_List *pList,      /* Argument list */
  ଅଚଳ Token *pToken,  /* Name of the function */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  ପଦସମଷ୍ଟି *pNew;
  sqlite3 *db = pParse->db;
  ପ୍ରତିଜ୍ଞା( pToken );
  pNew = sqlite3_ପଦସମଷ୍ଟି_Alloc(db, TK_FUNCTION, pToken, 1);
  ଯଦି( pNew==0 ){
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    ଫେରନ୍ତୁ 0;
  }
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pToken->z - pParse->zTail);
  ଯଦି( pList
   && pList->n_ପଦସମଷ୍ଟି > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  ପଦସମଷ୍ଟି_Set_Property(pNew, EP_HasFunc);
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pNew) );
  sqlite3_ପଦସମଷ୍ଟି_Set_Height_And_Flags(pParse, pNew);
  ଯଦି( eDistinct==SF_Distinct ) ପଦସମଷ୍ଟି_Set_Property(pNew, EP_Distinct);
  ଫେରନ୍ତୁ pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Order_By_Aggregate_Error(Parse *pParse, ପଦସମଷ୍ଟି *p){
  sqlite3_Error_ସନ୍ଦେଶ(pParse,
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,          /* The function call to which ORDER BY is to be added */
  ପଦସମଷ୍ଟି_List *pOrderBy    /* The ORDER BY clause to add */
){
  ପଦସମଷ୍ଟି *pOB;
  sqlite3 *db = pParse->db;
  ଯଦି( NEVER(pOrderBy==0) ){
    ପ୍ରତିଜ୍ଞା( db->mallocFailed );
    ଫେରନ୍ତୁ;
  }
  ଯଦି( p_ପଦସମଷ୍ଟି==0 ){
    ପ୍ରତିଜ୍ଞା( db->mallocFailed );
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pOrderBy);
    ଫେରନ୍ତୁ;
  }
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ==0 );
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
  ଯଦି( p_ପଦସମଷ୍ଟି->x.pList==0 || NEVER(p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_ପଦସମଷ୍ଟି_List_Delete_Generic, pOrderBy);
    ଫେରନ୍ତୁ;
  }
  ଯଦି( IsWindowFunc(p_ପଦସମଷ୍ଟି) ){
    sqlite3_ପଦସମଷ୍ଟି_Order_By_Aggregate_Error(pParse, p_ପଦସମଷ୍ଟି);
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pOrderBy);
    ଫେରନ୍ତୁ;
  }
  ଯଦି( pOrderBy->n_ପଦସମଷ୍ଟି>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many terms in ORDER BY clause");
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pOrderBy);
    ଫେରନ୍ତୁ;
  }

  pOB = sqlite3_ପଦସମଷ୍ଟି_Alloc(db, TK_ORDER, 0, 0);
  ଯଦି( pOB==0 ){
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pOrderBy);
    ଫେରନ୍ତୁ;
  }
  pOB->x.pList = pOrderBy;
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pOB) );
  p_ପଦସମଷ୍ଟି->p_ବାମ = pOB;
  ପଦସମଷ୍ଟି_Set_Property(pOB, EP_FullSize);
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,     /* The function invocation */
  ଅଚଳ FuncDef *pDef    /* The function being invoked */
){
  ପ୍ରତିଜ୍ଞା( !IN_RENAME_OBJECT );
  ପ୍ରତିଜ୍ଞା( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    ଯଦି( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "unsafe use of %#T()", p_ପଦସମଷ୍ଟି);
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Assign_Var_ସଂଖ୍ୟା(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, u32 n){
  sqlite3 *db = pParse->db;
  ଅଚଳ ଅକ୍ଷର *z;
  ynVar x;

  ଯଦି( p_ପଦସମଷ୍ଟି==0 ) ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ|EP_Reduced|EP_TokenOnly) );
  z = p_ପଦସମଷ୍ଟି->u.zToken;
  ପ୍ରତିଜ୍ଞା( z!=0 );
  ପ୍ରତିଜ୍ଞା( z[0]!=0 );
  ପ୍ରତିଜ୍ଞା( n==(u32)sqlite3Strlen30(z) );
  ଯଦି( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    ପ୍ରତିଜ୍ଞା( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା doAdd = 0;
    ଯଦି( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bOk;
      ଯଦି( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }ଅନ୍ୟଥା{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_ସଂଖ୍ୟା]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_ସଂଖ୍ୟା] );
      ଯଦି( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_ସଂଖ୍ୟା] ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_ସଂଖ୍ୟା]);
        sqlite3_Record_Error_Offset_Of_ପଦସମଷ୍ଟି(pParse->db, p_ପଦସମଷ୍ଟି);
        ଫେରନ୍ତୁ;
      }
      x = (ynVar)i;
      ଯଦି( x>pParse->nVar ){
        pParse->nVar = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)x;
        doAdd = 1;
      }ଅନ୍ୟଥା ଯଦି( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }ଅନ୍ୟଥା ଯଦି( sqlite3_VList_ଅଙ୍କ_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }ଅନ୍ୟଥା{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_ଅଙ୍କ(pParse->pVList, z, n);
      ଯଦି( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    ଯଦି( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      ଯଦି( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_ପଦସମଷ୍ଟି->iColumn = x;
  ଯଦି( x>db->aLimit[SQLITE_LIMIT_VARIABLE_ସଂଖ୍ୟା] ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_ପଦସମଷ୍ଟି(pParse->db, p_ପଦସମଷ୍ଟି);
  }
}

/*
** Recursively delete an expression tree.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Delete_NN(sqlite3 *db, ପଦସମଷ୍ଟି *p){
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ପ୍ରତିଜ୍ଞା( db!=0 );
ପଦସମଷ୍ଟି_Delete_Restart:
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_UValue(p) || p->u.i_ମୂଲ୍ୟ>=0 );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_YWin(p) || !ପଦସମଷ୍ଟି_Use_YSub(p) );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  ପ୍ରତିଜ୍ଞା( p->op!=TK_FUNCTION || !ପଦସମଷ୍ଟି_Use_YSub(p) );
#ifdef SQLITE_DEBUG
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p, EP_Leaf) && !ପଦସମଷ୍ଟି_Has_Property(p, EP_TokenOnly) ){
    ପ୍ରତିଜ୍ଞା( p->p_ବାମ==0 );
    ପ୍ରତିଜ୍ଞା( p->p_ଡାହାଣ==0 );
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_XSelect(p) || p->x.pSelect==0 );
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_XList(p) || p->x.pList==0 );
  }
#endif
  ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    ପ୍ରତିଜ୍ଞା( (ପଦସମଷ୍ଟି_Use_XList(p) && p->x.pList==0) || p->p_ଡାହାଣ==0 );
    ଯଦି( p->p_ଡାହାଣ ){
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p, EP_WinFunc) );
      sqlite3_ପଦସମଷ୍ଟି_Delete_NN(db, p->p_ଡାହାଣ);
    }ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p) ){
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }ଅନ୍ୟଥା{
      sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    ଯଦି( p->p_ବାମ && p->op!=TK_SELECT_COLUMN ){
      ପଦସମଷ୍ଟି *p_ବାମ = p->p_ବାମ;
      ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p, EP_Static)
       && !ପଦସମଷ୍ଟି_Has_Property(p_ବାମ, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_ବାମ;
        ଯାଅ ପଦସମଷ୍ଟି_Delete_Restart;
      }ଅନ୍ୟଥା{
        sqlite3_ପଦସମଷ୍ଟି_Delete_NN(db, p_ବାମ);
      }
    }
  }
  ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Delete(sqlite3 *db, ପଦସମଷ୍ଟି *p){
  ଯଦି( p ) sqlite3_ପଦସମଷ୍ଟି_Delete_NN(db, p);
}
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Delete_Generic(sqlite3 *db, ଶୂନ୍ୟ *p){
  ଯଦି( ALWAYS(p) ) sqlite3_ପଦସମଷ୍ଟି_Delete_NN(db, (ପଦସମଷ୍ଟି*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
ଶୂନ୍ୟ sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  ଯଦି( p==0 ){
    /* Nothing to clear */
  }ଅନ୍ୟଥା ଯଦି( p->pOn ){
    sqlite3_ପଦସମଷ୍ଟି_Delete_NN(db, p->pOn);
  }ଅନ୍ୟଥା ଯଦି( p->pUsing ){
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Deferred_Delete(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଫେରନ୍ତୁ 0==sqlite3ParserAddCleanup(pParse, sqlite3_ପଦସମଷ୍ଟି_Delete_Generic, p_ପଦସମଷ୍ଟି);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Unmap_And_Delete(Parse *pParse, ପଦସମଷ୍ଟି *p){
  ଯଦି( p ){
    ଯଦି( IN_RENAME_OBJECT ){
      sqlite3_Rename_ପଦସମଷ୍ଟି_Unmap(pParse, p);
    }
    sqlite3_ପଦସମଷ୍ଟି_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Struct_Size(ଅଚଳ ପଦସମଷ୍ଟି *p){
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p, EP_TokenOnly) ) ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_TOKENONLYSIZE;
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p, EP_Reduced) ) ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_REDUCEDSIZE;
  ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_FULLSIZE;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା duped_ପଦସମଷ୍ଟି_Struct_Size(ଅଚଳ ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSize;
  ପ୍ରତିଜ୍ଞା( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_FULLSIZE<=0xfff );
  ପ୍ରତିଜ୍ଞା( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  ଯଦି( 0==flags || ପଦସମଷ୍ଟି_Has_Property(p, EP_FullSize) ){
    nSize = ପଦସମଷ୍ଟି_FULLSIZE;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p, EP_OuterON) );
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_VVAProperty(p, EP_NoReduce) );
    ଯଦି( p->p_ବାମ || p->x.pList ){
      nSize = ପଦସମଷ୍ଟି_REDUCEDSIZE | EP_Reduced;
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( p->p_ଡାହାଣ==0 );
      nSize = ପଦସମଷ୍ଟି_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  ଫେରନ୍ତୁ nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା duped_ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Size(ଅଚଳ ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte = duped_ପଦସମଷ୍ଟି_Struct_Size(p, flags) & 0xfff;
  ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p, EP_Int_ମୂଲ୍ୟ) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  ଫେରନ୍ତୁ ROUND8(nByte);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା duped_ପଦସମଷ୍ଟି_Size(ଅଚଳ ପଦସମଷ୍ଟି *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ପ୍ରତିଜ୍ଞା( p!=0 );
  nByte = duped_ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Size(p, EXPRDUP_REDUCE);
  ଯଦି( p->p_ବାମ ) nByte += duped_ପଦସମଷ୍ଟି_Size(p->p_ବାମ);
  ଯଦି( p->p_ଡାହାଣ ) nByte += duped_ପଦସମଷ୍ଟି_Size(p->p_ଡାହାଣ);
  ପ୍ରତିଜ୍ଞା( nByte==ROUND8(nByte) );
  ଫେରନ୍ତୁ nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା EdupBuf EdupBuf;
ସଂରଚନା EdupBuf {
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
ସ୍ଥିର ପଦସମଷ୍ଟି *ପଦସମଷ୍ଟି_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ଅଚଳ ପଦସମଷ୍ଟି *p,        /* Expr tree to be duplicated */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  ପଦସମଷ୍ଟି *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  ପ୍ରତିଜ୍ଞା( db!=0 );
  ପ୍ରତିଜ୍ଞା( p );
  ପ୍ରତିଜ୍ଞା( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  ପ୍ରତିଜ୍ଞା( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  ଯଦି( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    ପ୍ରତିଜ୍ଞା( sEdupBuf.zAlloc!=0 );
    ପ୍ରତିଜ୍ଞା( dupFlags==EXPRDUP_REDUCE );
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nAlloc;
    ଯଦି( dupFlags ){
      nAlloc = duped_ପଦସମଷ୍ଟି_Size(p);
    }ଅନ୍ୟଥା ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p, EP_Int_ମୂଲ୍ୟ) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(ପଦସମଷ୍ଟି_FULLSIZE + nToken);
    }ଅନ୍ୟଥା{
      nToken = 0;
      nAlloc = ROUND8(ପଦସମଷ୍ଟି_FULLSIZE);
    }
    ପ୍ରତିଜ୍ଞା( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (ପଦସମଷ୍ଟି *)sEdupBuf.zAlloc;
  ପ୍ରତିଜ୍ଞା( EIGHT_BYTE_ALIGNMENT(pNew) );

  ଯଦି( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ଅଚଳ unsigned nStructSize = duped_ପଦସମଷ୍ଟି_Struct_Size(p, dupFlags);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nNewSize = nStructSize & 0xfff;
    ଯଦି( nToken<0 ){
      ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p, EP_Int_ମୂଲ୍ୟ) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }ଅନ୍ୟଥା{
        nToken = 0;
      }
    }
    ଯଦି( dupFlags ){
      ପ୍ରତିଜ୍ଞା( (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }ଅନ୍ୟଥା{
      u32 nSize = (u32)ପଦସମଷ୍ଟି_Struct_Size(p);
      ପ୍ରତିଜ୍ଞା( (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ପଦସମଷ୍ଟି_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      ଯଦି( nSize<ପଦସମଷ୍ଟି_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, ପଦସମଷ୍ଟି_FULLSIZE-nSize);
      }
      nNewSize = ପଦସମଷ୍ଟି_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    ପଦସମଷ୍ଟି_Clear_VVAProperties(pNew);
    ଯଦି( dupFlags ){
      ପଦସମଷ୍ଟି_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    ପ୍ରତିଜ୍ଞା( nToken>=0 );
    ଯଦି( nToken>0 ){
      ଅକ୍ଷର *zToken = pNew->u.zToken = (ଅକ୍ଷର*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    ଯଦି( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }ଅନ୍ୟଥା{
        pNew->x.pList = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      ଯଦି( dupFlags ){
        ଯଦି( p->op==TK_SELECT_COLUMN ){
          pNew->p_ବାମ = p->p_ବାମ;
          ପ୍ରତିଜ୍ଞା( p->p_ଡାହାଣ==0 
               || p->p_ଡାହାଣ==p->p_ବାମ
               || ପଦସମଷ୍ଟି_Has_Property(p->p_ବାମ, EP_Subquery) );
        }ଅନ୍ୟଥା{
          pNew->p_ବାମ = p->p_ବାମ ?
                      ପଦସମଷ୍ଟି_Dup(db, p->p_ବାମ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_ଡାହାଣ = p->p_ଡାହାଣ ?
                       ପଦସମଷ୍ଟି_Dup(db, p->p_ଡାହାଣ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }ଅନ୍ୟଥା{
        ଯଦି( p->op==TK_SELECT_COLUMN ){
          pNew->p_ବାମ = p->p_ବାମ;
          ପ୍ରତିଜ୍ଞା( p->p_ଡାହାଣ==0 
               || p->p_ଡାହାଣ==p->p_ବାମ
               || ପଦସମଷ୍ଟି_Has_Property(p->p_ବାମ, EP_Subquery) );
        }ଅନ୍ୟଥା{
          pNew->p_ବାମ = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p->p_ବାମ, 0);
        }
        pNew->p_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p->p_ଡାହାଣ, 0);
      }
    }
  }
  ଯଦି( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, ଆକାର(sEdupBuf));
  ପ୍ରତିଜ୍ଞା( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  ଫେରନ୍ତୁ pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  ଯଦି( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    ଯଦି( pRet ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      pRet->nCte = p->nCte;
      ପାଇଁ(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  ଫେରନ୍ତୁ pRet;
}
#ଅନ୍ୟଥା
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gatherSelectWindowsCallback(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION && ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_ପଦସମଷ୍ଟି->y.pWin;
    ପ୍ରତିଜ୍ଞା( pWin );
    ପ୍ରତିଜ୍ଞା( IsWindowFunc(p_ପଦସମଷ୍ଟି) );
    ପ୍ରତିଜ୍ଞା( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  ଫେରନ୍ତୁ WRC_Continue;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  ଫେରନ୍ତୁ p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ସ୍ଥିର ଶୂନ୍ୟ gatherSelectWindows(Select *p){
  Walker w;
  w.x_ପଦସମଷ୍ଟି_Callback = gatherSelectWindowsCallback;
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
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Dup(sqlite3 *db, ଅଚଳ ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପ୍ରତିଜ୍ଞା( flags==0 || flags==EXPRDUP_REDUCE );
  ଫେରନ୍ତୁ p ? ପଦସମଷ୍ଟି_Dup(db, p, flags, 0) : 0;
}
ପଦସମଷ୍ଟି_List *sqlite3_ପଦସମଷ୍ଟି_List_Dup(sqlite3 *db, ଅଚଳ ପଦସମଷ୍ଟି_List *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପଦସମଷ୍ଟି_List *pNew;
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  ଅଚଳ ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pOldItem;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପଦସମଷ୍ଟି *pPriorSelectColOld = 0;
  ପଦସମଷ୍ଟି *pPriorSelectColNew = 0;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  ଯଦି( pNew==0 ) ଫେରନ୍ତୁ 0;
  pNew->n_ପଦସମଷ୍ଟି = p->n_ପଦସମଷ୍ଟି;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  ପାଇଁ(i=0; i<p->n_ପଦସମଷ୍ଟି; i++, pItem++, pOldItem++){
    ପଦସମଷ୍ଟି *p_Old_ପଦସମଷ୍ଟି = pOldItem->p_ପଦସମଷ୍ଟି;
    ପଦସମଷ୍ଟି *p_New_ପଦସମଷ୍ଟି;
    pItem->p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_Old_ପଦସମଷ୍ଟି, flags);
    ଯଦି( p_Old_ପଦସମଷ୍ଟି
     && p_Old_ପଦସମଷ୍ଟି->op==TK_SELECT_COLUMN
     && (p_New_ପଦସମଷ୍ଟି = pItem->p_ପଦସମଷ୍ଟି)!=0
    ){
      ଯଦି( p_New_ପଦସମଷ୍ଟି->p_ଡାହାଣ ){
        pPriorSelectColOld = p_Old_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
        pPriorSelectColNew = p_New_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
        p_New_ପଦସମଷ୍ଟି->p_ବାମ = p_New_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
      }ଅନ୍ୟଥା{
        ଯଦି( p_Old_ପଦସମଷ୍ଟି->p_ବାମ!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_ପଦସମଷ୍ଟି->p_ବାମ;
          pPriorSelectColNew = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pPriorSelectColOld, flags);
          p_New_ପଦସମଷ୍ଟି->p_ଡାହାଣ = pPriorSelectColNew;
        }
        p_New_ପଦସମଷ୍ଟି->p_ବାମ = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  ଫେରନ୍ତୁ pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#ଯଦି !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ଅଚଳ SrcList *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  SrcList *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  ଯଦି( pNew==0 ) ଫେରନ୍ତୁ 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  ପାଇଁ(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    ଅଚଳ SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    ଯଦି( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, ଆକାର(Subquery));
      ଯଦି( pNewSubq==0 ){
        ପ୍ରତିଜ୍ଞା( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }ଅନ୍ୟଥା{
        memcpy(pNewSubq, pOldItem->u4.pSubq, ଆକାର(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        ଯଦି( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }ଅନ୍ୟଥା ଯଦି( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }ଅନ୍ୟଥା{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    ଯଦି( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }ଅନ୍ୟଥା ଯଦି( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }ଅନ୍ୟଥା{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    ଯଦି( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    ଯଦି( pTab ){
      pTab->nTabRef++;
    }
    ଯଦି( pOldItem->fg.isUsing ){
      ପ୍ରତିଜ୍ଞା( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }ଅନ୍ୟଥା{
      pNewItem->u3.pOn = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  ଫେରନ୍ତୁ pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ଅଚଳ IdList *p){
  IdList *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  ଯଦି( pNew==0 ) ଫେରନ୍ତୁ 0;
  pNew->nId = p->nId;
  ପାଇଁ(i=0; i<p->nId; i++){
    ସଂରଚନା IdList_item *pNewItem = &pNew->a[i];
    ଅଚଳ ସଂରଚନା IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  ଫେରନ୍ତୁ pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ଅଚଳ Select *pDup, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  ଅଚଳ Select *p;

  ପ୍ରତିଜ୍ଞା( db!=0 );
  ପାଇଁ(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, ଆକାର(*p) );
    ଯଦି( pNew==0 ) ଭାଙ୍ଗ;
    pNew->pEList = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    ଯଦି( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    ଯଦି( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      ଭାଙ୍ଗ;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  ଫେରନ୍ତୁ pRet;
}
#ଅନ୍ୟଥା
Select *sqlite3SelectDup(sqlite3 *db, ଅଚଳ Select *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପ୍ରତିଜ୍ଞା( p==0 );
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ଅଚଳ ସଂରଚନା ପଦସମଷ୍ଟି_List_item zeroItem = {0};
SQLITE_NOINLINE ପଦସମଷ୍ଟି_List *sqlite3_ପଦସମଷ୍ଟି_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି             /* Expression to be appended. Might be NULL */
){
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  ପଦସମଷ୍ଟି_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  ଯଦି( pList==0 ){
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି);
    ଫେରନ୍ତୁ 0;
  }
  pList->nAlloc = 4;
  pList->n_ପଦସମଷ୍ଟି = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି;
  ଫେରନ୍ତୁ pList;
}
SQLITE_NOINLINE ପଦସମଷ୍ଟି_List *sqlite3_ପଦସମଷ୍ଟି_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ପଦସମଷ୍ଟି_List *pList,        /* List to which to append. Might be NULL */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି             /* Expression to be appended. Might be NULL */
){
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  ପଦସମଷ୍ଟି_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  ଯଦି( pNew==0 ){
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pList);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି);
    ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_ପଦସମଷ୍ଟି++];
  *pItem = zeroItem;
  pItem->p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି;
  ଫେରନ୍ତୁ pList;
}
ପଦସମଷ୍ଟି_List *sqlite3_ପଦସମଷ୍ଟି_List_Append(
  Parse *pParse,          /* Parsing context */
  ପଦସମଷ୍ଟି_List *pList,        /* List to which to append. Might be NULL */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି             /* Expression to be appended. Might be NULL */
){
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  ଯଦି( pList==0 ){
    ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_List_Append_New(pParse->db,p_ପଦସମଷ୍ଟି);
  }
  ଯଦି( pList->nAlloc<pList->n_ପଦସମଷ୍ଟି+1 ){
    ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_List_Append_Grow(pParse->db,pList,p_ପଦସମଷ୍ଟି);
  }
  pItem = &pList->a[pList->n_ପଦସମଷ୍ଟି++];
  *pItem = zeroItem;
  pItem->p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି;
  ଫେରନ୍ତୁ pList;
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
ପଦସମଷ୍ଟି_List *sqlite3_ପଦସମଷ୍ଟି_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  ପଦସମଷ୍ଟି_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFirst = pList ? pList->n_ପଦସମଷ୍ଟି : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  ଯଦି( NEVER(pColumns==0) ) ଯାଅ vector_append_error;
  ଯଦି( p_ପଦସମଷ୍ଟି==0 ) ଯାଅ vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି)) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    ଯାଅ vector_append_error;
  }

  ପାଇଁ(i=0; i<pColumns->nId; i++){
    ପଦସମଷ୍ଟି *p_Sub_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_For_Vector_Field(pParse, p_ପଦସମଷ୍ଟି, i, pColumns->nId);
    ପ୍ରତିଜ୍ଞା( p_Sub_ପଦସମଷ୍ଟି!=0 || db->mallocFailed );
    ଯଦି( p_Sub_ପଦସମଷ୍ଟି==0 ) ଜାରି;
    pList = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pList, p_Sub_ପଦସମଷ୍ଟି);
    ଯଦି( pList ){
      ପ୍ରତିଜ୍ଞା( pList->n_ପଦସମଷ୍ଟି==iFirst+i+1 );
      pList->a[pList->n_ପଦସମଷ୍ଟି-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  ଯଦି( !db->mallocFailed && p_ପଦସମଷ୍ଟି->op==TK_SELECT && ALWAYS(pList!=0) ){
    ପଦସମଷ୍ଟି *pFirst = pList->a[iFirst].p_ପଦସମଷ୍ଟି;
    ପ୍ରତିଜ୍ଞା( pFirst!=0 );
    ପ୍ରତିଜ୍ଞା( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_ଡାହାଣ = p_ପଦସମଷ୍ଟି;
    p_ପଦସମଷ୍ଟି = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_ପଦସମଷ୍ଟି_Unmap_And_Delete(pParse, p_ପଦସମଷ୍ଟି);
  sqlite3IdListDelete(db, pColumns);
  ଫେରନ୍ତୁ pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_List_Set_Sort_Order(ପଦସମଷ୍ଟି_List *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSortOrder, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eNulls){
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( p->n_ପଦସମଷ୍ଟି>0 );

  ପ୍ରତିଜ୍ଞା( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  ପ୍ରତିଜ୍ଞା( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  ପ୍ରତିଜ୍ଞା( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_ପଦସମଷ୍ଟି-1];
  ପ୍ରତିଜ୍ଞା( pItem->fg.bNulls==0 );
  ଯଦି( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  ଯଦି( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    ଯଦି( iSortOrder!=eNulls ){
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  ପଦସମଷ୍ଟି_List *pList,        /* List to which to add the span. */
  ଅଚଳ Token *pName,     /* Name to be added */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dequote             /* True to cause the name to be dequoted */
){
  ପ୍ରତିଜ୍ଞା( pList!=0 || pParse->db->mallocFailed!=0 );
  ପ୍ରତିଜ୍ଞା( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  ଯଦି( pList ){
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
    ପ୍ରତିଜ୍ଞା( pList->n_ପଦସମଷ୍ଟି>0 );
    pItem = &pList->a[pList->n_ପଦସମଷ୍ଟି-1];
    ପ୍ରତିଜ୍ଞା( pItem->zEName==0 );
    ପ୍ରତିଜ୍ଞା( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    ଯଦି( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      ଯଦି( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ଅଚଳ ଶୂନ୍ୟ*)pItem->zEName, pName);
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  ପଦସମଷ୍ଟି_List *pList,        /* List to which to add the span. */
  ଅଚଳ ଅକ୍ଷର *zStart,     /* Start of the span */
  ଅଚଳ ଅକ୍ଷର *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  ପ୍ରତିଜ୍ଞା( pList!=0 || db->mallocFailed!=0 );
  ଯଦି( pList ){
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem = &pList->a[pList->n_ପଦସମଷ୍ଟି-1];
    ପ୍ରତିଜ୍ଞା( pList->n_ପଦସମଷ୍ଟି>0 );
    ଯଦି( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_List_Check_Length(
  Parse *pParse,
  ପଦସମଷ୍ଟି_List *pEList,
  ଅଚଳ ଅକ୍ଷର *zObject
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_ପଦସମଷ୍ଟି==mx );
  testcase( pEList && pEList->n_ପଦସମଷ୍ଟି==mx+1 );
  ଯଦି( pEList && pEList->n_ପଦସମଷ୍ଟି>mx ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ ପଦସମଷ୍ଟି_List_Delete_NN(sqlite3 *db, ପଦସମଷ୍ଟି_List *pList){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = pList->n_ପଦସମଷ୍ଟି;
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem =  pList->a;
  ପ୍ରତିଜ୍ଞା( pList->n_ପଦସମଷ୍ଟି>0 );
  ପ୍ରତିଜ୍ଞା( db!=0 );
  କର{
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, pItem->p_ପଦସମଷ୍ଟି);
    ଯଦି( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }ଯେତେବେଳେ( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_List_Delete(sqlite3 *db, ପଦସମଷ୍ଟି_List *pList){
  ଯଦି( pList ) ପଦସମଷ୍ଟି_List_Delete_NN(db, pList);
}
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_List_Delete_Generic(sqlite3 *db, ଶୂନ୍ୟ *pList){
  ଯଦି( ALWAYS(pList) ) ପଦସମଷ୍ଟି_List_Delete_NN(db, (ପଦସମଷ୍ଟି_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_ପଦସମଷ୍ଟି_List_Flags(ଅଚଳ ପଦସମଷ୍ଟି_List *pList){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  u32 m = 0;
  ପ୍ରତିଜ୍ଞା( pList!=0 );
  ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
     ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pList->a[i].p_ପଦସମଷ୍ଟି;
     ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
     m |= p_ପଦସମଷ୍ଟି->flags;
  }
  ଫେରନ୍ତୁ m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  ଫେରନ୍ତୁ WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ଅଚଳ ଅକ୍ଷର *zIn){
  ଯଦି( sqlite3StrICmp(zIn, "true")==0  ) ଫେରନ୍ତୁ EP_IsTrue;
  ଯଦି( sqlite3StrICmp(zIn, "false")==0 ) ଫେରନ୍ତୁ EP_IsFalse;
  ଫେରନ୍ତୁ 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Id_To_True_False(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  u32 v;
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_ID || p_ପଦସମଷ୍ଟି->op==TK_STRING );
  ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Quoted|EP_Int_ମୂଲ୍ୟ)
   && (v = sqlite3IsTrueOrFalse(p_ପଦସମଷ୍ଟି->u.zToken))!=0
  ){
    p_ପଦସମଷ୍ଟି->op = TK_TRUEFALSE;
    ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, v);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely((ପଦସମଷ୍ଟି*)p_ପଦସମଷ୍ଟି);
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_TRUEFALSE );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
  ପ୍ରତିଜ୍ଞା( sqlite3StrICmp(p_ପଦସମଷ୍ଟି->u.zToken,"true")==0
       || sqlite3StrICmp(p_ପଦସମଷ୍ଟି->u.zToken,"false")==0 );
  ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି->u.zToken[4]==0;
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
ପଦସମଷ୍ଟି *sqlite3_ପଦସମଷ୍ଟି_Simplified_And_Or(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_AND || p_ପଦସମଷ୍ଟି->op==TK_OR ){
    ପଦସମଷ୍ଟି *p_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Simplified_And_Or(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
    ପଦସମଷ୍ଟି *p_ବାମ = sqlite3_ପଦସମଷ୍ଟି_Simplified_And_Or(p_ପଦସମଷ୍ଟି->p_ବାମ);
    ଯଦି( ପଦସମଷ୍ଟି_Always_True(p_ବାମ) || ପଦସମଷ୍ଟି_Always_False(p_ଡାହାଣ) ){
      p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->op==TK_AND ? p_ଡାହାଣ : p_ବାମ;
    }ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Always_True(p_ଡାହାଣ) || ପଦସମଷ୍ଟି_Always_False(p_ବାମ) ){
      p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->op==TK_AND ? p_ବାମ : p_ଡାହାଣ;
    }
  }
  ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Eval_Rhs_First(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି->p_ବାମ, EP_Subquery)
   && !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, EP_Subquery)
  ){
    ଫେରନ୍ତୁ 1;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,       /* The comparison expression */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pR1,          /* OUT: Register holding the left operand */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pR2,          /* OUT: Register holding the right operand */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pFree1,       /* OUT: Temp register to free if not zero */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pFree2        /* OUT: Another temp register to free if not zero */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrIsNull;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2;
  Vdbe *v = pParse->pVdbe;

  ପ୍ରତିଜ୍ଞା( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  ଯଦି( ପଦସମଷ୍ଟି_Eval_Rhs_First(p_ପଦସମଷ୍ଟି) && sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ) ){
    r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }ଅନ୍ୟଥା{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, pFree1);
  ଯଦି( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, EP_Subquery)
     && sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(p_ପଦସମଷ୍ଟି->p_ବାମ)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  ଫେରନ୍ତୁ addrIsNull;
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
ସ୍ଥିର SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Is_Constant_Function(
  Walker *pWalker,
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;             /* Number of arguments */
  ପଦସମଷ୍ଟି_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION );
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly)
   || (pList = p_ପଦସମଷ୍ଟି->x.pList)==0
  ){;
    n = 0;
  }ଅନ୍ୟଥା{
    n = pList->n_ପଦସମଷ୍ଟି;
    sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pList);
    ଯଦି( pWalker->eCode==0 ) ଫେରନ୍ତୁ WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_ପଦସମଷ୍ଟି->u.zToken, n, ENC(db), 0);
  ଯଦି( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    ଫେରନ୍ତୁ WRC_Abort;
  }
  ଫେରନ୍ତୁ WRC_Prune;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Is_Constant(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପ୍ରତିଜ୍ଞା( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  ଯଦି( pWalker->eCode==2 && ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) ){
    pWalker->eCode = 0;
    ଫେରନ୍ତୁ WRC_Abort;
  }

  ବାଛ( p_ପଦସମଷ୍ଟି->op ){
    ସ୍ଥିତି TK_FUNCTION:
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        ଫେରନ୍ତୁ WRC_Abort;
      }ଅନ୍ୟଥା ଯଦି( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        ଯଦି( pWalker->eCode==5 ) ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_FromDDL);
        ଫେରନ୍ତୁ WRC_Continue;
      }ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        ଫେରନ୍ତୁ WRC_Continue;
      }ଅନ୍ୟଥା ଯଦି( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Is_Constant_Function(pWalker, p_ପଦସମଷ୍ଟି);
      }ଅନ୍ୟଥା{
        pWalker->eCode = 0;  /* Function call is not constant */
        ଫେରନ୍ତୁ WRC_Abort;
      }
    ସ୍ଥିତି TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Id_To_True_False(p_ପଦସମଷ୍ଟି) ){
        ଫେରନ୍ତୁ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    ସ୍ଥିତି TK_COLUMN:
    ସ୍ଥିତି TK_AGG_FUNCTION:
    ସ୍ଥିତି TK_AGG_COLUMN:
      testcase( p_ପଦସମଷ୍ଟି->op==TK_ID );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_COLUMN );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_AGG_COLUMN );
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FixedCol) && pWalker->eCode!=2 ){
        ଫେରନ୍ତୁ WRC_Continue;
      }
      ଯଦି( pWalker->eCode==3 && p_ପଦସମଷ୍ଟି->iTable==pWalker->u.iCur ){
        ଫେରନ୍ତୁ WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    ସ୍ଥିତି TK_IF_NULL_ROW:
    ସ୍ଥିତି TK_REGISTER:
    ସ୍ଥିତି TK_DOT:
    ସ୍ଥିତି TK_RAISE:
      testcase( p_ପଦସମଷ୍ଟି->op==TK_REGISTER );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_IF_NULL_ROW );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_DOT );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_RAISE );
      pWalker->eCode = 0;
      ଫେରନ୍ତୁ WRC_Abort;
    ସ୍ଥିତି TK_IS:
    ସ୍ଥିତି TK_ISNOT:
      ଯଦି( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        ଫେରନ୍ତୁ WRC_Abort;
      }
      ଫେରନ୍ତୁ WRC_Continue;
    ସ୍ଥିତି TK_VARIABLE:
      ଯଦି( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_ପଦସମଷ୍ଟି->op = TK_NULL;
      }ଅନ୍ୟଥା ଯଦି( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        ଫେରନ୍ତୁ WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      testcase( p_ପଦସମଷ୍ଟି->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_ପଦସମଷ୍ଟି->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      ଫେରନ୍ତୁ WRC_Continue;
  }
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Is_Const(Parse *pParse, ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_ପଦସମଷ୍ଟି_Callback = ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p);
  ଫେରନ୍ତୁ w.eCode;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Constant(Parse *pParse, ପଦସମଷ୍ଟି *p){
  ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_List_Is_Constant(Parse *pParse, ପଦସମଷ୍ଟି_List *pList, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bNoIs){
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା iInit = bNoIs ? 6 : 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପାଇଁ(ii=0; ii<pList->n_ପଦସମଷ୍ଟି; ii++){
    ଯଦି( 0==ପଦସମଷ୍ଟି_Is_Const(pParse, pList->a[ii].p_ପଦସମଷ୍ଟି, iInit) ) ଫେରନ୍ତୁ 0;
  }
  ଫେରନ୍ତୁ 1;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Not_Join(Parse *pParse, ପଦସମଷ୍ଟି *p){
  ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  ପ୍ରତିଜ୍ଞା( pSelect!=0 );
  ପ୍ରତିଜ୍ଞା( pWalker->eCode==3 || pWalker->eCode==0 );
  ଯଦି( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    ଫେରନ୍ତୁ WRC_Abort;
  }
  ଫେରନ୍ତୁ WRC_Prune;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Table_Constant(ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_ପଦସମଷ୍ଟି_Callback = ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Is_Constant;
  ଯଦି( bAllowSubq ){
    w.xSelectCallback = ପଦସମଷ୍ଟି_Select_Walk_Table_Constant;
  }ଅନ୍ୟଥା{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p);
  ଫେରନ୍ତୁ w.eCode;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Single_Table_Constraint(
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,                 /* The constraint */
  ଅଚଳ SrcList *pSrcList,     /* Complete FROM clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSrc,                    /* Which element of pSrcList to use */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bAllowSubq               /* Allow non-correlated subqueries */
){
  ଅଚଳ SrcItem *pSrc = &pSrcList->a[iSrc];
  ଯଦି( pSrc->fg.jointype & JT_LTORJ ){
    ଫେରନ୍ତୁ 0;  /* rule (3) */
  }
  ଯଦି( pSrc->fg.jointype & JT_ବାମ ){
    ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) ) ଫେରନ୍ତୁ 0;   /* rule (4a) */
    ଯଦି( p_ପଦସମଷ୍ଟି->w.iJoin!=pSrc->iCursor ) ଫେରନ୍ତୁ 0;         /* rule (4b) */
  }ଅନ୍ୟଥା{
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) ) ଫେରନ୍ତୁ 0;    /* rule (5) */
  }
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା jj;
    ପାଇଁ(jj=0; jj<iSrc; jj++){
      ଯଦି( p_ପଦସମଷ୍ଟି->w.iJoin==pSrcList->a[jj].iCursor ){
        ଯଦି( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          ଫେରନ୍ତୁ 0;  /* restriction (6) */
        }
        ଭାଙ୍ଗ;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Is_Table_Constant(p_ପଦସମଷ୍ଟି, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Is_Constant_Or_Group_By(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପଦସମଷ୍ଟି_List *pGroupBy = pWalker->u.pGroupBy;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  ପାଇଁ(i=0; i<pGroupBy->n_ପଦସମଷ୍ଟି; i++){
    ପଦସମଷ୍ଟି *p = pGroupBy->a[i].p_ପଦସମଷ୍ଟି;
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0, p_ପଦସମଷ୍ଟି, p, -1)<2 ){
      CollSeq *pColl = sqlite3_ପଦସମଷ୍ଟି_NNColl_Seq(pWalker->pParse, p);
      ଯଦି( sqlite3_Is_ଦ୍ୱିମିକ(pColl) ){
        ଫେରନ୍ତୁ WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
    pWalker->eCode = 0;
    ଫେରନ୍ତୁ WRC_Abort;
  }

  ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Is_Constant(pWalker, p_ପଦସମଷ୍ଟି);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Or_Group_By(Parse *pParse, ପଦସମଷ୍ଟି *p, ପଦସମଷ୍ଟି_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_ପଦସମଷ୍ଟି_Callback = ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p);
  ଫେରନ୍ତୁ w.eCode;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Or_Function(ପଦସମଷ୍ଟି *p, u8 isInit){
  ପ୍ରତିଜ୍ଞା( isInit==0 || isInit==1 );
  ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Contains_Subquery(ପଦସମଷ୍ଟି *p){
  Walker w;
  w.eCode = 1;
  w.x_ପଦସମଷ୍ଟି_Callback = sqlite3_ପଦସମଷ୍ଟି_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p);
  ଫେରନ୍ତୁ w.eCode==0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Integer(ଅଚଳ ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *p_ମୂଲ୍ୟ, Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bRSI){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSign = 1;   /* Either +1 or -1. */
  ଯେତେବେଳେ( 1/*exit-by-break*/ ){
    ଯଦି( NEVER(p==0) ) ଫେରନ୍ତୁ 0;
    ଯଦି( ପଦସମଷ୍ଟି_Use_UValue(p) ){
      ପ୍ରତିଜ୍ଞା( p->op==TK_INTEGER );
      *p_ମୂଲ୍ୟ = p->u.i_ମୂଲ୍ୟ*iSign;
      ଯଦି( bRSI && pParse==0 && p->op2 ){
        ଫେରନ୍ତୁ 0;
      }
      ଫେରନ୍ତୁ 1;
    }
    ଯଦି( p->op==TK_UPLUS ){
      p = p->p_ବାମ;
      pParse = 0;
      ଜାରି;
    }
    ଯଦି( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_ବାମ;
      pParse = 0;
      ଜାରି;
    }
    ଯଦି( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_ମୂଲ୍ୟ *pVal; /* The variable */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;          /* 1 if successful, 0 if failed */
      ପ୍ରତିଜ୍ଞା( iSign==1 );
      ଯଦି( NEVER(pParse->pVdbe==0) ) ଭାଙ୍ଗ;
      ଯଦି( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ଭାଙ୍ଗ;
      pVal = sqlite3_Vdbe_Get_Bound_ମୂଲ୍ୟ(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      ଯଦି( pVal ){
        i64 vv;
        ଯଦି( sqlite3_ମୂଲ୍ୟ_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_ମୂଲ୍ୟ_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_ମୂଲ୍ୟ = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)vv;
          rc = 1;       
        }
        sqlite3_ମୂଲ୍ୟ_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      ଫେରନ୍ତୁ rc;
    }
    ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(ଅଚଳ ପଦସମଷ୍ଟି *p){
  u8 op;
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ଯେତେବେଳେ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_ବାମ;
    ପ୍ରତିଜ୍ଞା( p!=0 );
  }
  op = p->op;
  ଯଦି( op==TK_REGISTER ) op = p->op2;
  ବାଛ( op ){
    ସ୍ଥିତି TK_INTEGER:
    ସ୍ଥିତି TK_STRING:
    ସ୍ଥିତି TK_FLOAT:
    ସ୍ଥିତି TK_BLOB:
      ଫେରନ୍ତୁ 0;
    ସ୍ଥିତି TK_COLUMN:
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p) );
      ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ 1;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Needs_No_Affinity_Change(ଅଚଳ ପଦସମଷ୍ଟି *p, ଅକ୍ଷର aff){
  u8 op;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା unaryMinus = 0;
  ଯଦି( aff==SQLITE_AFF_BLOB ) ଫେରନ୍ତୁ 1;
  ଯେତେବେଳେ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    ଯଦି( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_ବାମ;
  }
  op = p->op;
  ଯଦି( op==TK_REGISTER ) op = p->op2;
  ବାଛ( op ){
    ସ୍ଥିତି TK_INTEGER: {
      ଫେରନ୍ତୁ aff>=SQLITE_AFF_NUMERIC;
    }
    ସ୍ଥିତି TK_FLOAT: {
      ଫେରନ୍ତୁ aff>=SQLITE_AFF_NUMERIC;
    }
    ସ୍ଥିତି TK_STRING: {
      ଫେରନ୍ତୁ !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    ସ୍ଥିତି TK_BLOB: {
      ଫେରନ୍ତୁ !unaryMinus;
    }
    ସ୍ଥିତି TK_COLUMN: {
      ପ୍ରତିଜ୍ଞା( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      ଫେରନ୍ତୁ aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ଫେରନ୍ତୁ 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3IsRowid(ଅଚଳ ଅକ୍ଷର *z){
  ଯଦି( sqlite3StrICmp(z, "_ROWID_")==0 ) ଫେରନ୍ତୁ 1;
  ଯଦି( sqlite3StrICmp(z, "ROWID")==0 ) ଫେରନ୍ତୁ 1;
  ଯଦି( sqlite3StrICmp(z, "OID")==0 ) ଫେରନ୍ତୁ 1;
  ଫେରନ୍ତୁ 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3RowidAlias(Table *pTab){
  ଅଚଳ ଅକ୍ଷର *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପ୍ରତିଜ୍ଞା( VisibleRowid(pTab) );
  ପାଇଁ(ii=0; ii<ArraySize(azOpt); ii++){
    ଯଦି( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) ଫେରନ୍ତୁ azOpt[ii];
  }
  ଫେରନ୍ତୁ 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
ସ୍ଥିର Select *isCandidateForInOpt(ଅଚଳ ପଦସମଷ୍ଟି *pX){
  Select *p;
  SrcList *pSrc;
  ପଦସମଷ୍ଟି_List *pEList;
  Table *pTab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( !ପଦସମଷ୍ଟି_Use_XSelect(pX) ) ଫେରନ୍ତୁ 0;                 /* Not a subquery */
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pX, EP_VarSelect)  ) ଫେରନ୍ତୁ 0;  /* Correlated subq */
  p = pX->x.pSelect;
  ଯଦି( p->pPrior ) ଫେରନ୍ତୁ 0;              /* Not a compound SELECT */
  ଯଦି( p->selFlags & SF_Aggregate ){
    ଫେରନ୍ତୁ 0; /* No GROUP BY keyword or aggregate functions */
  }
  ପ୍ରତିଜ୍ଞା( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  ଯଦି( p->pLimit ) ଫେରନ୍ତୁ 0;              /* Has no LIMIT clause */
  ଯଦି( p->pWhere ) ଫେରନ୍ତୁ 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  ପ୍ରତିଜ୍ଞା( pSrc!=0 );
  ଯଦି( pSrc->nSrc!=1 ) ଫେରନ୍ତୁ 0;          /* Single term in FROM clause */
  ଯଦି( pSrc->a[0].fg.isSubquery) ଫେରନ୍ତୁ 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  ପ୍ରତିଜ୍ଞା( pTab!=0 );
  ପ୍ରତିଜ୍ଞା( !IsView(pTab)  );              /* FROM clause is not a view */
  ଯଦି( IsVirtual(pTab) ) ଫେରନ୍ତୁ 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  ପ୍ରତିଜ୍ଞା( pEList!=0 );
  /* All SELECT results must be columns. */
  ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
    ପଦସମଷ୍ଟି *pRes = pEList->a[i].p_ପଦସମଷ୍ଟି;
    ଯଦି( pRes->op!=TK_COLUMN ) ଫେରନ୍ତୁ 0;
    ପ୍ରତିଜ୍ଞା( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  ଫେରନ୍ତୁ p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur,          /* Cursor for the index */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regHasNull,    /* Register in which to store hasNull flag */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  ଯଦି( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3InRhsIsConstant(Parse *pParse, ପଦସମଷ୍ଟି *pIn){
  ପଦସମଷ୍ଟି *pLHS;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_ବାମ;
  pIn->p_ବାମ = 0;
  res = sqlite3_ପଦସମଷ୍ଟି_Is_Constant(pParse, pIn);
  pIn->p_ବାମ = pLHS;
  ଫେରନ୍ତୁ res;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  ପଦସମଷ୍ଟି *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *prRhsHasNull,         /* Register holding NULL status.  See notes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aiMap,                /* Mapping from Index fields to RHS fields */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab;                             /* Cursor of the RHS table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  ପ୍ରତିଜ୍ଞା( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  ଯଦି( prRhsHasNull && ପଦସମଷ୍ଟି_Use_XSelect(pX) ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପଦସମଷ୍ଟି_List *pEList = pX->x.pSelect->pEList;
    ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(pEList->a[i].p_ପଦସମଷ୍ଟି) ) ଭାଙ୍ଗ;
    }
    ଯଦି( i==pEList->n_ପଦସମଷ୍ଟି ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  ଯଦି( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                               /* Database idx for pTab */
    ପଦସମଷ୍ଟି_List *pEList = p->pEList;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ପଦସମଷ୍ଟି = pEList->n_ପଦସମଷ୍ଟି;

    ପ୍ରତିଜ୍ଞା( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    ପ୍ରତିଜ୍ଞା( p->pEList->a[0].p_ପଦସମଷ୍ଟି!=0 ); /* Because of isCandidateForInOpt(p) */
    ପ୍ରତିଜ୍ଞା( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    ପ୍ରତିଜ୍ଞା( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    ପ୍ରତିଜ୍ଞା(v);  /* sqlite3GetVdbe() has always been previously called */
    ଯଦି( n_ପଦସମଷ୍ଟି==1 && pEList->a[0].p_ପଦସମଷ୍ଟି->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }ଅନ୍ୟଥା{
      Index *pIdx;                         /* Iterator variable */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା affinity_ok = 1;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      ପାଇଁ(i=0; i<n_ପଦସମଷ୍ଟି && affinity_ok; i++){
        ପଦସମଷ୍ଟି *pLhs = sqlite3VectorFieldSubexpr(pX->p_ବାମ, i);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = pEList->a[i].p_ପଦସମଷ୍ଟି->iColumn;
        ଅକ୍ଷର idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        ଅକ୍ଷର cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        ବାଛ( cmpaff ){
          ସ୍ଥିତି SQLITE_AFF_BLOB:
            ଭାଙ୍ଗ;
          ସ୍ଥିତି SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            ପ୍ରତିଜ୍ଞା( idxaff==SQLITE_AFF_TEXT );
            ଭାଙ୍ଗ;
          ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      ଯଦି( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        ପାଇଁ(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          ଯଦି( pIdx->nColumn<n_ପଦସମଷ୍ଟି ) ଜାରି;
          ଯଦି( pIdx->pPartIdxWhere!=0 ) ଜାରି;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          ଯଦି( pIdx->nColumn>=BMS-1 ) ଜାରି;
          ଯଦି( mustBeUnique ){
            ଯଦି( pIdx->n_ଚାବି_Col>n_ପଦସମଷ୍ଟି
             ||(pIdx->nColumn>n_ପଦସମଷ୍ଟି && !IsUniqueIndex(pIdx))
            ){
              ଜାରି;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          ପାଇଁ(i=0; i<n_ପଦସମଷ୍ଟି; i++){
            ପଦସମଷ୍ଟି *pLhs = sqlite3VectorFieldSubexpr(pX->p_ବାମ, i);
            ପଦସମଷ୍ଟି *pRhs = pEList->a[i].p_ପଦସମଷ୍ଟି;
            CollSeq *pReq = sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(pParse, pLhs, pRhs);
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
 
            ପାଇଁ(j=0; j<n_ପଦସମଷ୍ଟି; j++){
              ଯଦି( pIdx->aiColumn[j]!=pRhs->iColumn ) ଜାରି;
              ପ୍ରତିଜ୍ଞା( pIdx->azColl[j] );
              ଯଦି( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                ଜାରି;
              }
              ଭାଙ୍ଗ;
            }
            ଯଦି( j==n_ପଦସମଷ୍ଟି ) ଭାଙ୍ଗ;
            mCol = MASKBIT(j);
            ଯଦି( mCol & colUsed ) ଭାଙ୍ଗ; /* Each column used only once */
            colUsed |= mCol;
            ଯଦି( aiMap ) aiMap[i] = j;
          }
 
          ପ୍ରତିଜ୍ଞା( n_ପଦସମଷ୍ଟି>0 && n_ପଦସମଷ୍ଟି<BMS );
          ପ୍ରତିଜ୍ଞା( i==n_ପଦସମଷ୍ଟି || colUsed!=(MASKBIT(n_ପଦସମଷ୍ଟି)-1) );
          ଯଦି( colUsed==(MASKBIT(n_ପଦସମଷ୍ଟି)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_ଚାବି_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            ପ୍ରତିଜ୍ଞା( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            ଯଦି( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_ପଦସମଷ୍ଟି)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              ଯଦି( n_ପଦସମଷ୍ଟି==1 ){
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
  ଯଦି( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && ପଦସମଷ୍ଟି_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_ପଦସମଷ୍ଟି<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  ଯଦି( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rMayHaveNull = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    ଯଦି( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }ଅନ୍ୟଥା ଯଦି( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    ପ୍ରତିଜ୍ଞା( pX->op==TK_IN );
    ଯଦି( !bloomOk
     && ପଦସମଷ୍ଟି_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    ଯଦି( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  ଯଦି( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
    n = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(pX->p_ବାମ);
    ପାଇଁ(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  ଫେରନ୍ତୁ eType;
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
ସ୍ଥିର ଅକ୍ଷର *ପଦସମଷ୍ଟି_INAffinity(Parse *pParse, ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVal = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ବାମ);
  Select *pSelect = ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ? p_ପଦସମଷ୍ଟି->x.pSelect : 0;
  ଅକ୍ଷର *zRet;

  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  ଯଦି( zRet ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<nVal; i++){
      ପଦସମଷ୍ଟି *pA = sqlite3VectorFieldSubexpr(p_ବାମ, i);
      ଅକ୍ଷର a = sqlite3_ପଦସମଷ୍ଟି_Affinity(pA);
      ଯଦି( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_ପଦସମଷ୍ଟି, a);
      }ଅନ୍ୟଥା{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  ଫେରନ୍ତୁ zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
ଶୂନ୍ୟ sqlite3SubselectError(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nActual, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nExpect){
  ଯଦି( pParse->nErr==0 ){
    ଅଚଳ ଅକ୍ଷର *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_ସନ୍ଦେଶ(pParse, zFmt, nActual, nExpect);
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
ଶୂନ୍ୟ sqlite3_Vector_Error_ସନ୍ଦେଶ(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
#ifndef SQLITE_OMIT_SUBQUERY
  ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
    sqlite3SubselectError(pParse, p_ପଦସମଷ୍ଟି->x.pSelect->pEList->n_ପଦସମଷ୍ଟି, 1);
  }ଅନ୍ୟଥା
#endif
  {
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  ଯଦି( pNewSig==0 ) ଫେରନ୍ତୁ 0;
  ଯଦି( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) ଫେରନ୍ତୁ 0;
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_IN );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_YSub(p_ପଦସମଷ୍ଟି) );
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pSelect!=0 );
  ପ୍ରତିଜ୍ଞା( (p_ପଦସମଷ୍ଟି->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  ପ୍ରତିଜ୍ଞା( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  ପାଇଁ(; pOp<pEnd; pOp++){
    ଯଦି( pOp->p4type!=P4_SUBRTNSIG ) ଜାରି;
    ପ୍ରତିଜ୍ଞା( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    ପ୍ରତିଜ୍ଞା( pSig!=0 );
    ଯଦି( !pSig->bComplete ) ଜାରି;
    ଯଦି( pNewSig->selId!=pSig->selId ) ଜାରି;
    ଯଦି( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) ଜାରି;
    p_ପଦସମଷ୍ଟି->y.sub.iAddr = pSig->iAddr;
    p_ପଦସମଷ୍ଟି->y.sub.regReturn = pSig->regReturn;
    p_ପଦସମଷ୍ଟି->iTable = pSig->iTable;
    ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_Subrtn);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
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
ଶୂନ୍ୟ sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,            /* The IN operator */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab,               /* Use this cursor number */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା allowBloom          /* True to allow the use of a Bloom filter */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrOnce = 0;           /* Address of the OP_Once instruction at top */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;                   /* Address of OP_OpenEphemeral instruction */
  ପଦସମଷ୍ଟି *p_ବାମ;                /* the LHS of the IN operator */
  ଚାବି_Info *p_ଚାବି_Info = 0;      /* Key information */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  ପ୍ରତିଜ୍ଞା( v!=0 );

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
  ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) || p_ପଦସମଷ୍ଟି->x.pSelect!=0 );
    ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) && (p_ପଦସମଷ୍ଟି->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, ଆକାର(pSig[0]));
      ଯଦି( pSig ){
        pSig->selId = p_ପଦସମଷ୍ଟି->x.pSelect->selId;
        pSig->zAff = ପଦସମଷ୍ଟି_INAffinity(pParse, p_ପଦସମଷ୍ଟି);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_ପଦସମଷ୍ଟି, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_ପଦସମଷ୍ଟି->x.pSelect->selId));
      }
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YSub(p_ପଦସମଷ୍ଟି) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_ପଦସମଷ୍ଟି->y.sub.regReturn,
                        p_ପଦସମଷ୍ଟି->y.sub.iAddr);
      ପ୍ରତିଜ୍ଞା( iTab!=p_ପଦସମଷ୍ଟି->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_ପଦସମଷ୍ଟି->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      ଯଦି( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      ଫେରନ୍ତୁ;
    }

    /* Begin coding the subroutine */
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_YWin(p_ପଦସମଷ୍ଟି) );
    ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_Subrtn);
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly|EP_Reduced) );
    p_ପଦସମଷ୍ଟି->y.sub.regReturn = ++pParse->nMem;
    p_ପଦସମଷ୍ଟି->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_ପଦସମଷ୍ଟି->y.sub.regReturn) + 1;
    ଯଦି( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_ପଦସମଷ୍ଟି->y.sub.iAddr;
      pSig->regReturn = p_ପଦସମଷ୍ଟି->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ଅଚଳ ଅକ୍ଷର*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
  nVal = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ବାମ);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_ପଦସମଷ୍ଟି->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_ପଦସମଷ୍ଟି->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
    VdbeComment((v, "Result of SELECT %u", p_ପଦସମଷ୍ଟି->x.pSelect->selId));
  }ଅନ୍ୟଥା{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_ଚାବି_Info = sqlite3_ଚାବି_Info_Alloc(pParse->db, nVal, 1);
  ପ୍ରତିଜ୍ଞା( p_ଚାବି_Info!=0 || pParse->nErr );
  ଯଦି( p_ଚାବି_Info==0 ) ଫେରନ୍ତୁ;

  ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_ପଦସମଷ୍ଟି->x.pSelect;
    ପଦସମଷ୍ଟି_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    ଯଦି( ALWAYS(pEList->n_ପଦସମଷ୍ଟି==nVal) ){
      Select *pCopy;
      SelectDest dest;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = ପଦସମଷ୍ଟି_INAffinity(pParse, p_ପଦସମଷ୍ଟି);
      pSelect->iLimit = 0;
      ପ୍ରତିଜ୍ଞା( pEList!=0 );
      ପ୍ରତିଜ୍ଞା( pEList->n_ପଦସମଷ୍ଟି>0 );
      ପ୍ରତିଜ୍ଞା( sqlite3_ଚାବି_Info_Is_Writeable(p_ଚାବି_Info) );
      ପାଇଁ(i=0; i<nVal; i++){
        ପଦସମଷ୍ଟି *p = sqlite3VectorFieldSubexpr(p_ବାମ, i);
        CollSeq *pColl;
        p_ଚାବି_Info->aColl[i] = pColl = sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_ପଦସମଷ୍ଟି
        );
        ଯଦି( !sqlite3_Is_ଦ୍ୱିମିକ(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      ଯଦି( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (ଶୂନ୍ୟ *)p_ଚାବି_Info, P4_KEYINFO);
        p_ଚାବି_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      ଯଦି( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        ଯଦି( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      ଯଦି( rc ){
        sqlite3_ଚାବି_Info_Unref(p_ଚାବି_Info);
        ଫେରନ୍ତୁ;
      }
    }
  }ଅନ୍ୟଥା ଯଦି( ALWAYS(p_ପଦସମଷ୍ଟି->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    ଅକ୍ଷର affinity;            /* Affinity of the LHS of the IN */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପଦସମଷ୍ଟି_List *pList = p_ପଦସମଷ୍ଟି->x.pList;
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2;
    affinity = sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ବାମ);
    ଯଦି( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }ଅନ୍ୟଥା ଯଦି( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    ପ୍ରତିଜ୍ଞା( p_ଚାବି_Info!=0 );
    ପ୍ରତିଜ୍ଞା( sqlite3_ଚାବି_Info_Is_Writeable(p_ଚାବି_Info) );
    p_ଚାବି_Info->aColl[0] = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    ପାଇଁ(i=pList->n_ପଦସମଷ୍ଟି, pItem=pList->a; i>0; i--, pItem++){
      ପଦସମଷ୍ଟି *pE2 = pItem->p_ପଦସମଷ୍ଟି;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      ଯଦି( addrOnce && !sqlite3_ପଦସମଷ୍ଟି_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        ପଦସମଷ୍ଟି_Clear_Property(p_ପଦସମଷ୍ଟି, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  ଯଦି( pSig ) pSig->bComplete = 1;
  ଯଦି( p_ଚାବି_Info ){
    sqlite3VdbeChangeP4(v, addr, (ଶୂନ୍ୟ *)p_ଚାବି_Info, P4_KEYINFO);
  }
  ଯଦି( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YSub(p_ପଦସମଷ୍ଟି) );
    ପ୍ରତିଜ୍ଞା( sqlite3VdbeGetOp(v,p_ପଦସମଷ୍ଟି->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_ପଦସମଷ୍ଟି->y.sub.regReturn,
                      p_ପଦସମଷ୍ଟି->y.sub.iAddr, 1);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3CodeSubselect(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nReg;                   /* Registers to allocate */
  ପଦସମଷ୍ଟି *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  ପ୍ରତିଜ୍ଞା( v!=0 );
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ 0;
  testcase( p_ପଦସମଷ୍ଟି->op==TK_EXISTS );
  testcase( p_ପଦସମଷ୍ଟି->op==TK_SELECT );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_EXISTS || p_ପଦସମଷ୍ଟି->op==TK_SELECT );
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
  pSel = p_ପଦସମଷ୍ଟି->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YSub(p_ପଦସମଷ୍ଟି) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_ପଦସମଷ୍ଟି->y.sub.regReturn,
                      p_ପଦସମଷ୍ଟି->y.sub.iAddr);
    ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି->iTable;
  }

  /* Begin coding the subroutine */
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Use_YWin(p_ପଦସମଷ୍ଟି) );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Reduced|EP_TokenOnly) );
  ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_Subrtn);
  p_ପଦସମଷ୍ଟି->y.sub.regReturn = ++pParse->nMem;
  p_ପଦସମଷ୍ଟି->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_ପଦସମଷ୍ଟି->y.sub.regReturn) + 1;

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
  ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_VarSelect) ){
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
  nReg = p_ପଦସମଷ୍ଟି->op==TK_SELECT ? pSel->pEList->n_ପଦସମଷ୍ଟି : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    ଯଦି( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_ଡାହାଣ ){
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
    }ଅନ୍ୟଥା{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }ଅନ୍ୟଥା{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  ଯଦି( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    ପଦସମଷ୍ଟି *p_ବାମ = pSel->pLimit->p_ବାମ;
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ବାମ, EP_Int_ମୂଲ୍ୟ)==0
     || (p_ବାମ->u.i_ମୂଲ୍ୟ!=1 && p_ବାମ->u.i_ମୂଲ୍ୟ!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_ପଦସମଷ୍ଟି_Int32(db, 0);
      ଯଦି( pLimit ){
        pLimit->aff_ପଦସମଷ୍ଟି = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ବାମ, 0), pLimit);
      }
      sqlite3_ପଦସମଷ୍ଟି_Deferred_Delete(pParse, p_ବାମ);
      pSel->pLimit->p_ବାମ = pLimit;
    }
  }ଅନ୍ୟଥା{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_ପଦସମଷ୍ଟି_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  ଯଦି( sqlite3Select(pParse, pSel, &dest) ){
    p_ପଦସମଷ୍ଟି->op2 = p_ପଦସମଷ୍ଟି->op;
    p_ପଦସମଷ୍ଟି->op = TK_ERROR;
    ଫେରନ୍ତୁ 0;
  }
  p_ପଦସମଷ୍ଟି->iTable = rReg = dest.iSDParm;
  ପଦସମଷ୍ଟି_Set_VVAProperty(p_ପଦସମଷ୍ଟି, EP_NoReduce);
  ଯଦି( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  ଯଦି( dest.eDest==SRT_Mem 
   && sqlite3_ପଦସମଷ୍ଟି_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_ପଦସମଷ୍ଟି) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YSub(p_ପଦସମଷ୍ଟି) );
  ପ୍ରତିଜ୍ଞା( sqlite3VdbeGetOp(v,p_ପଦସମଷ୍ଟି->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_ପଦସମଷ୍ଟି->y.sub.regReturn,
                    p_ପଦସମଷ୍ଟି->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  ଫେରନ୍ତୁ rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Check_IN(Parse *pParse, ପଦସମଷ୍ଟି *pIn){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVector = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(pIn->p_ବାମ);
  ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    ଯଦି( nVector!=pIn->x.pSelect->pEList->n_ପଦସମଷ୍ଟି ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_ପଦସମଷ୍ଟି, nVector);
      ଫେରନ୍ତୁ 1;
    }
  }ଅନ୍ୟଥା ଯଦି( nVector!=1 ){
    sqlite3_Vector_Error_ସନ୍ଦେଶ(pParse, pIn->p_ବାମ);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,          /* The IN expression */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType;            /* Type of the RHS */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aiMap = 0;       /* Map from vector field to index column */
  ଅକ୍ଷର *zAff = 0;       /* Affinity string for comparisons */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVector;          /* Size of vectors for this IN operator */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDummy;           /* Dummy parameter to exprCodeVector() */
  ପଦସମଷ୍ଟି *p_ବାମ;          /* The LHS of the IN operator */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                /* loop counter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା destStep2;        /* Where to jump when NULLs seen in step 2 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା destStep6 = 0;    /* Start of code for Step 6 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrTruthOp;      /* Address of opcode that determines the IN is true */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା destNotNull;      /* Jump here if a comparison is not true in step 6 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr_ଶୀର୍ଷ;          /* Top of the step-6 loop */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_VVAProperty(p_ପଦସମଷ୍ଟି,EP_Immutable) );
  p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Check_IN(pParse, p_ପଦସମଷ୍ଟି) ) ଫେରନ୍ତୁ;
  zAff = ପଦସମଷ୍ଟି_INAffinity(pParse, p_ପଦସମଷ୍ଟି);
  nVector = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି->p_ବାମ);
  aiMap = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)sqlite3DbMallocZero(pParse->db, nVector*ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
  ଯଦି( pParse->db->mallocFailed ) ଯାଅ sqlite3_ପଦସମଷ୍ଟି_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  ପ୍ରତିଜ୍ଞା( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_ପଦସମଷ୍ଟି,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  ପ୍ରତିଜ୍ଞା( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  ପାଇଁ(i=0; i<nVector; i++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା j, cnt;
    ପାଇଁ(cnt=j=0; j<nVector; j++) ଯଦି( aiMap[j]==i ) cnt++;
    ପ୍ରତିଜ୍ଞା( cnt==1 );
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
  ପ୍ରତିଜ୍ଞା( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = ପଦସମଷ୍ଟି_Code_Vector(pParse, p_ବାମ, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  ଯଦି( eType==IN_INDEX_NOOP ){
    ପଦସମଷ୍ଟି_List *pList;
    CollSeq *pColl;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା labelOk = sqlite3VdbeMakeLabel(pParse);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା r2, regToFree;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା regCkNull = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
    ପ୍ରତିଜ୍ଞା( nVector==1 );
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
    pList = p_ପଦସମଷ୍ଟି->x.pList;
    pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ);
    ଯଦି( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    ପାଇଁ(ii=0; ii<pList->n_ପଦସମଷ୍ଟି; ii++){
      r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, pList->a[ii].p_ପଦସମଷ୍ଟି, &regToFree);
      ଯଦି( regCkNull && sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(pList->a[ii].p_ପଦସମଷ୍ଟି) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      ଯଦି( ii<pList->n_ପଦସମଷ୍ଟି-1 || destIfNull!=destIfFalse ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (ଶୂନ୍ୟ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_ପଦସମଷ୍ଟି-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_ପଦସମଷ୍ଟି-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_ପଦସମଷ୍ଟି-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_ପଦସମଷ୍ଟି-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        ପ୍ରତିଜ୍ଞା( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (ଶୂନ୍ୟ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    ଯଦି( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    ଯାଅ sqlite3_ପଦସମଷ୍ଟି_Code_IN_finished;
  }

  ଯଦି( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    ଯଦି( nVector==1 ){
      ଅକ୍ଷର aff = zAff[0];
      ଯଦି( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ବାମ) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }ଅନ୍ୟଥା{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    ପାଇଁ(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    ଯଦି( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      ପାଇଁ(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  ଯଦି( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }ଅନ୍ୟଥା{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  ପାଇଁ(i=0; i<nVector; i++){
    ପଦସମଷ୍ଟି *p = sqlite3VectorFieldSubexpr(p_ପଦସମଷ୍ଟି->p_ବାମ, i);
    ଯଦି( pParse->nErr ) ଯାଅ sqlite3_ପଦସମଷ୍ଟି_Code_IN_oom_error;
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  ଯଦି( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    ପ୍ରତିଜ୍ଞା( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }ଅନ୍ୟଥା{
    ଯଦି( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Subrtn) ){
        ଅଚଳ VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ପଦସମଷ୍ଟି->y.sub.iAddr);
        ପ୍ରତିଜ୍ଞା( pOp->opcode==OP_Once || pParse->nErr );
        ଯଦି( pOp->p3>0 ){  /* tag-202407032019 */
          ପ୍ରତିଜ୍ଞା( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      ଯାଅ sqlite3_ପଦସମଷ୍ଟି_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  ଯଦି( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  ଯଦି( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  ଯଦି( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  ଯଦି( eType==IN_INDEX_INDEX_DESC ){
    addr_ଶୀର୍ଷ = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }ଅନ୍ୟଥା{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_ଶୀର୍ଷ = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  ଯଦି( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }ଅନ୍ୟଥା{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  ପାଇଁ(i=0; i<nVector; i++){
    ପଦସମଷ୍ଟି *p;
    CollSeq *pColl;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_ବାମ, i);
    ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
      ପଦସମଷ୍ଟି *pRhs = p_ପଦସମଷ୍ଟି->x.pSelect->pEList->a[i].p_ପଦସମଷ୍ଟି;
      pColl = sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(pParse, p, pRhs);
    }ଅନ୍ୟଥା{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (ଶୂନ୍ୟ*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  ଯଦି( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_ଶୀର୍ଷ+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_ପଦସମଷ୍ଟି_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_ପଦସମଷ୍ଟି_Code_IN_oom_error:
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
ସ୍ଥିର ଶୂନ୍ୟ codeReal(Vdbe *v, ଅଚଳ ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା negateFlag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iMem){
  ଯଦି( ALWAYS(z!=0) ){
    ଦ୍ୱିଗୁଣ ମୂଲ୍ୟ;
    sqlite3AtoF(z, &ମୂଲ୍ୟ);
    ପ୍ରତିଜ୍ଞା( !sqlite3IsNaN(ମୂଲ୍ୟ) ); /* The new AtoF never returns NaN */
    ଯଦି( negateFlag ) ମୂଲ୍ୟ = -ମୂଲ୍ୟ;
    sqlite3VdbeAddDouble(v, iMem, ମୂଲ୍ୟ);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ସ୍ଥିର ଶୂନ୍ୟ codeInteger(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା negFlag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iMem){
  Vdbe *v = pParse->pVdbe;
  ଯଦି( p_ପଦସମଷ୍ଟି->flags & EP_Int_ମୂଲ୍ୟ ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = p_ପଦସମଷ୍ଟି->u.i_ମୂଲ୍ୟ;
    ପ୍ରତିଜ୍ଞା( i>=0 );
    ଯଦି( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
    i64 ମୂଲ୍ୟ;
    ଅଚଳ ଅକ୍ଷର *z = p_ପଦସମଷ୍ଟି->u.zToken;
    ପ୍ରତିଜ୍ଞା( z!=0 );
    c = sqlite3DecOrHexToI64(z, &ମୂଲ୍ୟ);
    ଯଦି( (c==3 && !negFlag) || (c==2) || (negFlag && ମୂଲ୍ୟ==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_ପଦସମଷ୍ଟି);
#ଅନ୍ୟଥା
#ifndef SQLITE_OMIT_HEX_INTEGER
      ଯଦି( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_ପଦସମଷ୍ଟି);
      }ଅନ୍ୟଥା
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }ଅନ୍ୟଥା{
      ଯଦି( negFlag ){ ମୂଲ୍ୟ = c==3 ? SMALLEST_INT64 : -ମୂଲ୍ୟ; }
      sqlite3VdbeAddInt64(v, iMem, ମୂଲ୍ୟ);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTabCur,    /* Cursor pointing to a table row */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdxCol,    /* The column of the index to be loaded */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  ଯଦି( iTabCol==XN_ପଦସମଷ୍ଟି ){
    ପ୍ରତିଜ୍ଞା( pIdx->a_Col_ପଦସମଷ୍ଟି );
    ପ୍ରତିଜ୍ଞା( pIdx->a_Col_ପଦସମଷ୍ଟି->n_ପଦସମଷ୍ଟି>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_ପଦସମଷ୍ଟି_Code_Copy(pParse, pIdx->a_Col_ପଦସମଷ୍ଟି->a[iIdxCol].p_ପଦସମଷ୍ଟି, regOut);
    pParse->iSelfTab = 0;
  }ଅନ୍ୟଥା{
    sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOut         /* Put the result in this register */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAddr;
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nErr = pParse->nErr;
  ପ୍ରତିଜ୍ଞା( v!=0 );
  ପ୍ରତିଜ୍ଞା( pParse->iSelfTab!=0 );
  ଯଦି( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }ଅନ୍ୟଥା{
    iAddr = 0;
  }
  sqlite3_ପଦସମଷ୍ଟି_Code_Copy(pParse, sqlite3_Column_ପଦସମଷ୍ଟି(pTab,pCol), regOut);
  ଯଦି( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା p3 = 2+(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (ଅକ୍ଷର*)pTab, P4_TABLE);
  }ଅନ୍ୟଥା ଯଦି( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  ଯଦି( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  ଯଦି( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol,       /* Index of the column to extract */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOut      /* Extract the value into this register */
){
  Column *pCol;
  ପ୍ରତିଜ୍ଞା( v!=0 );
  ପ୍ରତିଜ୍ଞା( pTab!=0 );
  ପ୍ରତିଜ୍ଞା( iCol!=XN_ପଦସମଷ୍ଟି );
  ଯଦି( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
    ଯଦି( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      ଫେରନ୍ତୁ;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }ଅନ୍ୟଥା ଯଦି( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      ଯଦି( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_ପଦସମଷ୍ଟି_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      ଫେରନ୍ତୁ;
#endif
    }ଅନ୍ୟଥା ଯଦି( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_ଚାବି_Index(pTab), iCol);
    }ଅନ୍ୟଥା{
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iColumn,     /* Index of the table column */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTable,      /* The cursor pointing to the table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  ପ୍ରତିଜ୍ଞା( pParse->pVdbe!=0 );
  ପ୍ରତିଜ୍ଞା( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  ପ୍ରତିଜ୍ଞା( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  ଯଦି( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    ଯଦି( pOp->opcode==OP_Column ) pOp->p5 = p5;
    ଯଦି( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  ଫେରନ୍ତୁ iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Code_Move(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFrom, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTo, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_To_Register(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg){
  ପଦସମଷ୍ଟି *p = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(p_ପଦସମଷ୍ଟି);
  ଯଦି( NEVER(p==0) ) ଫେରନ୍ତୁ;
  ଯଦି( p->op==TK_REGISTER ){
    ପ୍ରତିଜ୍ଞା( p->iTable==iReg );
  }ଅନ୍ୟଥା{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    ପଦସମଷ୍ଟି_Clear_Property(p, EP_Skip);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Code_Vector(Parse *pParse, ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piFreeable){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iResult;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nResult = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p);
  ଯଦି( nResult==1 ){
    iResult = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p, piFreeable);
  }ଅନ୍ୟଥା{
    *piFreeable = 0;
    ଯଦି( p->op==TK_SELECT ){
#ଯଦି SQLITE_OMIT_SUBQUERY
      iResult = 0;
#ଅନ୍ୟଥା
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p) );
      ପାଇଁ(i=0; i<nResult; i++){
        sqlite3_ପଦସମଷ୍ଟି_Code_Factorable(pParse, p->x.pList->a[i].p_ପଦସମଷ୍ଟି, i+iResult);
      }
    }
  }
  ଫେରନ୍ତୁ iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ସ୍ଥିର ଶୂନ୍ୟ setDoNotMergeFlagOnCopy(Vdbe *v){
  ଯଦି( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  ପଦସମଷ୍ଟି_List *pFarg,      /* List of function arguments */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା target            /* Store function result in this register */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFarg;
  Vdbe *v = pParse->pVdbe;
  ପ୍ରତିଜ୍ଞା( v!=0 );
  ପ୍ରତିଜ୍ଞା( pFarg!=0 );
  nFarg = pFarg->n_ପଦସମଷ୍ଟି;
  ପ୍ରତିଜ୍ଞା( nFarg>0 );  /* All in-line functions have at least one argument */
  ବାଛ( iFuncId ){
    ସ୍ଥିତି INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା endCoalesce = sqlite3VdbeMakeLabel(pParse);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପ୍ରତିଜ୍ଞା( nFarg>=2 );
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pFarg->a[0].p_ପଦସମଷ୍ଟି, target);
      ପାଇଁ(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pFarg->a[i].p_ପଦସମଷ୍ଟି, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି INLINEFUNC_iif: {
      ପଦସମଷ୍ଟି case_ପଦସମଷ୍ଟି;
      memset(&case_ପଦସମଷ୍ଟି, 0, ଆକାର(case_ପଦସମଷ୍ଟି));
      case_ପଦସମଷ୍ଟି.op = TK_CASE;
      case_ପଦସମଷ୍ଟି.x.pList = pFarg;
      ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, &case_ପଦସମଷ୍ଟି, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    ସ୍ଥିତି INLINEFUNC_sqlite_offset: {
      ପଦସମଷ୍ଟି *pArg = pFarg->a[0].p_ପଦସମଷ୍ଟି;
      ଯଦି( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      ଭାଙ୍ଗ;
    }
#endif
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      ପ୍ରତିଜ୍ଞା( nFarg==1 || nFarg==2 );
      target = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, pFarg->a[0].p_ପଦସମଷ୍ଟି, target);
      ଭାଙ୍ଗ;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#ଯଦି !defined(SQLITE_UNTESTABLE)
    ସ୍ଥିତି INLINEFUNC_ପଦସମଷ୍ଟି_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      ପ୍ରତିଜ୍ଞା( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_ପଦସମଷ୍ଟି_Compare(0,pFarg->a[0].p_ପଦସମଷ୍ଟି, pFarg->a[1].p_ପଦସମଷ୍ଟି,-1),
         target);
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି INLINEFUNC_ପଦସମଷ୍ଟି_implies_ପଦସମଷ୍ଟି: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      ପ୍ରତିଜ୍ଞା( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_ପଦସମଷ୍ଟି_Implies_ପଦସମଷ୍ଟି(pParse,pFarg->a[0].p_ପଦସମଷ୍ଟି, pFarg->a[1].p_ପଦସମଷ୍ଟି,-1),
         target);
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      ପଦସମଷ୍ଟି *pA1;
      ପ୍ରତିଜ୍ଞା( nFarg==2 );
      pA1 = pFarg->a[1].p_ପଦସମଷ୍ଟି;
      ଯଦି( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_ପଦସମଷ୍ଟି_Implies_Non_Null_Row(pFarg->a[0].p_ପଦସମଷ୍ଟି,pA1->iTable,1),
           target);
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      ଅଚଳ ଅକ୍ଷର *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      ଅକ୍ଷର aff;
      ପ୍ରତିଜ୍ଞା( nFarg==1 );
      aff = sqlite3_ପଦସମଷ୍ଟି_Affinity(pFarg->a[0].p_ପଦସମଷ୍ଟି);
      ପ୍ରତିଜ୍ଞା( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      ଭାଙ୍ଗ;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  ଫେରନ୍ତୁ target;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Can_Return_Subtype(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  FuncDef *pDef;
  sqlite3 *db;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_CASE || p_ପଦସମଷ୍ଟି->op==TK_UPLUS 
   || p_ପଦସମଷ୍ଟି->op==TK_COLLATE || p_ପଦସମଷ୍ଟି->op==TK_CAST 
  ){
    ଫେରନ୍ତୁ WRC_Continue;
  }
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_FUNCTION && p_ପଦସମଷ୍ଟି->op!=TK_AGG_FUNCTION ){
    ଫେରନ୍ତୁ WRC_Prune;
  }
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
  db = pWalker->pParse->db;
  n = p_ପଦସମଷ୍ଟି->x.pList ? p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି : 0;
  pDef = sqlite3FindFunction(db, p_ପଦସମଷ୍ଟି->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ଯଦି( pDef==0 ) ଫେରନ୍ତୁ WRC_Continue;
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା( pDef!=0 );
#endif
  ଯଦି( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    ଫେରନ୍ତୁ WRC_Abort;
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Can_Return_Subtype(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  Walker w;
  memset(&w, 0, ଆକାର(w));
  w.pParse = pParse;
  w.x_ପଦସମଷ୍ଟି_Callback = ପଦସମଷ୍ଟି_ଗଣ୍ଠି_Can_Return_Subtype;
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p_ପଦସମଷ୍ଟି);
  ଫେରନ୍ତୁ w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ସ୍ଥିର SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Indexed_ପଦସମଷ୍ଟି_Lookup(
  Parse *pParse,   /* The parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,     /* The expression to potentially bypass */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା target       /* Where to store the result of the expression */
){
  Indexed_ପଦସମଷ୍ଟି *p;
  Vdbe *v;
  ପାଇଁ(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 ପଦସମଷ୍ଟି_Aff;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDataCur = p->iDataCur;
    ଯଦି( iDataCur<0 ) ଜାରି;
    ଯଦି( pParse->iSelfTab ){
      ଯଦି( p->iDataCur!=pParse->iSelfTab-1 ) ଜାରି;
      iDataCur = -1;
    }
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0, p_ପଦସମଷ୍ଟି, p->p_ପଦସମଷ୍ଟି, iDataCur)!=0 ) ଜାରି;
    ପ୍ରତିଜ୍ଞା( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    ପଦସମଷ୍ଟି_Aff = sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି);
    ଯଦି( (ପଦସମଷ୍ଟି_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (ପଦସମଷ୍ଟି_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (ପଦସମଷ୍ଟି_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      ଜାରି;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_SubtArg)
     && sqlite3_ପଦସମଷ୍ଟି_Can_Return_Subtype(pParse, p_ପଦସମଷ୍ଟି) 
    ){
      ଜାରି;
    }

    v = pParse->pVdbe;
    ପ୍ରତିଜ୍ଞା( v!=0 );
    ଯଦି( p->bMaybeNullRow || sqlite3_ପଦସମଷ୍ଟି_Is_Constant(pParse, p_ପଦସମଷ୍ଟି) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, p_ପଦସମଷ୍ଟି, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }ଅନ୍ୟଥା{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    ଫେରନ୍ତୁ target;
  }
  ଫେରନ୍ତୁ -1;  /* Not found */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Partidx_ପଦସମଷ୍ଟି_Lookup(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTarget){
  Indexed_ପଦସମଷ୍ଟି *p;
  ପାଇଁ(p=pParse->p_Idx_Part_ପଦସମଷ୍ଟି; p; p=p->pIENext){
    ଯଦି( p_ପଦସମଷ୍ଟି->iColumn==p->iIdxCol && p_ପଦସମଷ୍ଟି->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

      ଯଦି( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, p->p_ପଦସମଷ୍ଟି, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ଅଚଳ ଅକ୍ଷର*)&p->aff, 1);
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Can_Return_Subtype(pParse, p->p_ପଦସମଷ୍ଟି) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      ଯଦି( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      ଫେରନ୍ତୁ ret;
    }
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,       /* AND or OR expression to be coded */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା target,        /* Put result in this register, guaranteed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pTmpReg       /* Write a temporary register here */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;            /* The opcode.  TK_AND or TK_OR */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା skipOp;        /* Opcode for the branch that skips one operand */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrSkip;      /* Branch instruction that skips one of the operands */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regSS = 0;     /* Register holding computed operand when other omitted */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2;        /* Registers for left and right operands, respectively */
  ପଦସମଷ୍ଟି *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
  op = p_ପଦସମଷ୍ଟି->op;
  ପ୍ରତିଜ୍ଞା( op==TK_AND || op==TK_OR );
  ପ୍ରତିଜ୍ଞା( TK_AND==OP_And );            testcase( op==TK_AND );
  ପ୍ରତିଜ୍ଞା( TK_OR==OP_Or );              testcase( op==TK_OR );
  ପ୍ରତିଜ୍ଞା( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_ପଦସମଷ୍ଟି_Simplified_And_Or(p_ପଦସମଷ୍ଟି);
  ଯଦି( pAlt!=p_ପଦସମଷ୍ଟି ){
    r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    ଫେରନ୍ତୁ target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  ଯଦି( ପଦସମଷ୍ଟି_Eval_Rhs_First(p_ପଦସମଷ୍ଟି) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, pTmpReg);
  }ଅନ୍ୟଥା{
    /* Compute the left operand first */
    r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, target);
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }ଅନ୍ୟଥା{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  ଯଦି( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  ଫେରନ୍ତୁ target;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Code_Target(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;                   /* The opcode being coded */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା inReg = target;       /* Results stored in register inReg */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regFree1 = 0;         /* If non-zero free this temporary register */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regFree2 = 0;         /* If non-zero free this temporary register */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2;               /* Various register numbers */
  ପଦସମଷ୍ଟି tempX;               /* Temporary expression node */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା p5 = 0;

  ପ୍ରତିଜ୍ଞା( target>0 && target<=pParse->nMem );
  ପ୍ରତିଜ୍ଞା( v!=0 );

ପଦସମଷ୍ଟି_code_doover:
  ଯଦି( p_ପଦସମଷ୍ଟି==0 ){
    op = TK_NULL;
  }ଅନ୍ୟଥା ଯଦି( pParse->pIdxEpr!=0
   && !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Leaf)
   && (r1 = sqlite3_Indexed_ପଦସମଷ୍ଟି_Lookup(pParse, p_ପଦସମଷ୍ଟି, target))>=0
  ){
    ଫେରନ୍ତୁ r1;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_VVAProperty(p_ପଦସମଷ୍ଟି,EP_Immutable) );
    op = p_ପଦସମଷ୍ଟି->op;
  }
  ପ୍ରତିଜ୍ଞା( op!=TK_ORDER );
  ବାଛ( op ){
    ସ୍ଥିତି TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_ପଦସମଷ୍ଟି->pAggInfo;
      ସଂରଚନା AggInfo_col *pCol;
      ପ୍ରତିଜ୍ଞା( pAggInfo!=0 );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iAgg>=0 );
      ଯଦି( p_ପଦସମଷ୍ଟି->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        ଭାଙ୍ଗ;
      }
      pCol = &pAggInfo->aCol[p_ପଦସମଷ୍ଟି->iAgg];
      ଯଦି( !pAggInfo->directMode ){
        ଫେରନ୍ତୁ AggInfoColumnReg(pAggInfo, p_ପଦସମଷ୍ଟି->iAgg);
      }ଅନ୍ୟଥା ଯଦି( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        ଯଦି( pTab==0 ){
          /* No comment added */
        }ଅନ୍ୟଥା ଯଦି( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }ଅନ୍ୟଥା{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          ଯଦି( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        ଫେରନ୍ତୁ target;
      }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_ପଦସମଷ୍ଟି->iTable, p_ପଦସମଷ୍ଟି->iColumn, target);
        ଫେରନ୍ତୁ target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    ସ୍ଥିତି TK_COLUMN: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab = p_ପଦସମଷ୍ଟି->iTable;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg;
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା aff;
        iReg = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ,target);
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_ପଦସମଷ୍ଟି->y.pTab, p_ପଦସମଷ୍ଟି->iColumn);
        ଯଦି( aff>SQLITE_AFF_BLOB ){
          ସ୍ଥିର ଅଚଳ ଅକ୍ଷର zAff[] = "B\000C\000D\000E\000F";
          ପ୍ରତିଜ୍ଞା( SQLITE_AFF_BLOB=='A' );
          ପ୍ରତିଜ୍ଞା( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        ଫେରନ୍ତୁ iReg;
      }
      ଯଦି( iTab<0 ){
        ଯଦି( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSrc;
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = p_ପଦସମଷ୍ଟି->iColumn;
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
          pTab = p_ପଦସମଷ୍ଟି->y.pTab;
          ପ୍ରତିଜ୍ଞା( pTab!=0 );
          ପ୍ରତିଜ୍ଞା( iCol>=XN_ROWID );
          ପ୍ରତିଜ୍ଞା( iCol<pTab->nCol );
          ଯଦି( iCol<0 ){
            ଫେରନ୍ତୁ -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          ଯଦି( pCol->colFlags & COLFLAG_GENERATED ){
            ଯଦି( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_ସନ୍ଦେଶ(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              ଫେରନ୍ତୁ 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            ଯଦି( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_ପଦସମଷ୍ଟି_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            ଫେରନ୍ତୁ iSrc;
          }ଅନ୍ୟଥା
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          ଯଦି( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            ଫେରନ୍ତୁ target;
          }ଅନ୍ୟଥା{
            ଫେରନ୍ତୁ iSrc;
          }
        }ଅନ୍ୟଥା{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      ଅନ୍ୟଥା ଯଦି( pParse->p_Idx_Part_ପଦସମଷ୍ଟି 
       && 0!=(r1 = ପଦସମଷ୍ଟି_Partidx_ପଦସମଷ୍ଟି_Lookup(pParse, p_ପଦସମଷ୍ଟି, target))
      ){
        ଫେରନ୍ତୁ r1;
      }
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->y.pTab!=0 );
      iReg = sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column(pParse, p_ପଦସମଷ୍ଟି->y.pTab,
                               p_ପଦସମଷ୍ଟି->iColumn, iTab, target,
                               p_ପଦସମଷ୍ଟି->op2);
      ଫେରନ୍ତୁ iReg;
    }
    ସ୍ଥିତି TK_INTEGER: {
      codeInteger(pParse, p_ପଦସମଷ୍ଟି, 0, target);
      ଫେରନ୍ତୁ target;
    }
    ସ୍ଥିତି TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(p_ପଦସମଷ୍ଟି), target);
      ଫେରନ୍ତୁ target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    ସ୍ଥିତି TK_FLOAT: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      codeReal(v, p_ପଦସମଷ୍ଟି->u.zToken, 0, target);
      ଫେରନ୍ତୁ target;
    }
#endif
    ସ୍ଥିତି TK_STRING: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3VdbeLoadString(v, target, p_ପଦସମଷ୍ଟି->u.zToken);
      ଫେରନ୍ତୁ target;
    }
    ସ୍ଥିତି TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_ପଦସମଷ୍ଟି->y.nReg - 1);
      ଫେରନ୍ତୁ target;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      ପ୍ରତିଜ୍ଞା( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      ଫେରନ୍ତୁ target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    ସ୍ଥିତି TK_BLOB: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
      ଅଚଳ ଅକ୍ଷର *z;
      ଅକ୍ଷର *zBlob;
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->u.zToken[0]=='x' || p_ପଦସମଷ୍ଟି->u.zToken[0]=='X' );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->u.zToken[1]=='\'' );
      z = &p_ପଦସମଷ୍ଟି->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      ପ୍ରତିଜ୍ଞା( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      ଫେରନ୍ତୁ target;
    }
#endif
    ସ୍ଥିତି TK_VARIABLE: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->u.zToken!=0 );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_ପଦସମଷ୍ଟି->iColumn, target);
      ଫେରନ୍ତୁ target;
    }
    ସ୍ଥିତି TK_REGISTER: {
      ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    ସ୍ଥିତି TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, target);
      ପ୍ରତିଜ୍ଞା( inReg==target );
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_ପଦସମଷ୍ଟି->u.zToken, 0));
      ଫେରନ୍ତୁ inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    ସ୍ଥିତି TK_IS:
    ସ୍ଥିତି TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ସ୍ଥିତି TK_LT:
    ସ୍ଥିତି TK_LE:
    ସ୍ଥିତି TK_GT:
    ସ୍ଥିତି TK_GE:
    ସ୍ଥିତି TK_NE:
    ସ୍ଥିତି TK_EQ: {
      ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrIsNull = 0;
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Vector(p_ବାମ) ){
        codeVectorCompare(pParse, p_ପଦସମଷ୍ଟି, target, op, p5);
      }ଅନ୍ୟଥା{
        ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = ପଦସମଷ୍ଟି_Compute_Operands(pParse, p_ପଦସମଷ୍ଟି,
                                     &r1, &r2, &regFree1, &regFree2);
        }ଅନ୍ୟଥା{
          r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
          r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_ବାମ, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_Commuted));
        ପ୍ରତିଜ୍ଞା(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        ପ୍ରତିଜ୍ଞା(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        ପ୍ରତିଜ୍ଞା(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        ପ୍ରତିଜ୍ଞା(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        ପ୍ରତିଜ୍ଞା(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        ପ୍ରତିଜ୍ଞା(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        ଯଦି( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }ଅନ୍ୟଥା{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          ଯଦି( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_AND:
    ସ୍ଥିତି TK_OR: {
      inReg = ପଦସମଷ୍ଟି_Code_Target_And_Or(pParse, p_ପଦସମଷ୍ଟି, target, &regFree1);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_PLUS:
    ସ୍ଥିତି TK_STAR:
    ସ୍ଥିତି TK_MINUS:
    ସ୍ଥିତି TK_REM:
    ସ୍ଥିତି TK_BITAND:
    ସ୍ଥିତି TK_BITOR:
    ସ୍ଥିତି TK_SLASH:
    ସ୍ଥିତି TK_LSHIFT:
    ସ୍ଥିତି TK_RSHIFT:
    ସ୍ଥିତି TK_CONCAT: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrIsNull;
      ପ୍ରତିଜ୍ଞା( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      ପ୍ରତିଜ୍ଞା( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      ପ୍ରତିଜ୍ଞା( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      ପ୍ରତିଜ୍ଞା( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      ପ୍ରତିଜ୍ଞା( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      ପ୍ରତିଜ୍ଞା( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      ପ୍ରତିଜ୍ଞା( TK_LSHIFT==OP_Shift_ବାମ );   testcase( op==TK_LSHIFT );
      ପ୍ରତିଜ୍ଞା( TK_RSHIFT==OP_Shift_ଡାହାଣ );  testcase( op==TK_RSHIFT );
      ପ୍ରତିଜ୍ଞା( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Subquery) ){
        addrIsNull = ପଦସମଷ୍ଟି_Compute_Operands(pParse, p_ପଦସମଷ୍ଟି,
                                   &r1, &r2, &regFree1, &regFree2);
      }ଅନ୍ୟଥା{
        r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
        r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ଯଦି( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_UMINUS: {
      ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
      ପ୍ରତିଜ୍ଞା( p_ବାମ );
      ଯଦି( p_ବାମ->op==TK_INTEGER ){
        codeInteger(pParse, p_ବାମ, 1, target);
        ଫେରନ୍ତୁ target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }ଅନ୍ୟଥା ଯଦି( p_ବାମ->op==TK_FLOAT ){
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
        codeReal(v, p_ବାମ->u.zToken, 1, target);
        ଫେରନ୍ତୁ target;
#endif
      }ଅନ୍ୟଥା{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_ମୂଲ୍ୟ|EP_TokenOnly;
        tempX.u.i_ମୂଲ୍ୟ = 0;
        ପଦସମଷ୍ଟି_Clear_VVAProperties(&tempX);
        r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_BITNOT:
    ସ୍ଥିତି TK_NOT: {
      ପ୍ରତିଜ୍ଞା( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      ପ୍ରତିଜ୍ଞା( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_TRUTH: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା isTrue;    /* IS TRUE or IS NOT TRUE */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
      bNormal = p_ପଦସମଷ୍ଟି->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_ISNULL:
    ସ୍ଥିତି TK_NOTNULL: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;
      ପ୍ରତିଜ୍ଞା( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ପ୍ରତିଜ୍ଞା( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_ପଦସମଷ୍ଟି->pAggInfo;
      ଯଦି( pInfo==0
       || NEVER(p_ପଦସମଷ୍ଟି->iAgg<0)
       || NEVER(p_ପଦସମଷ୍ଟି->iAgg>=pInfo->nFunc)
      ){
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "misuse of aggregate: %#T()", p_ପଦସମଷ୍ଟି);
      }ଅନ୍ୟଥା{
        ଫେରନ୍ତୁ AggInfoFuncReg(pInfo, p_ପଦସମଷ୍ଟି->iAgg);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_FUNCTION: {
      ପଦସମଷ୍ଟି_List *pFarg;       /* List of function arguments */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ଅଚଳ ଅକ୍ଷର *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
        ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି->y.pWin->regResult;
      }
#endif

      ଯଦି( ConstFactorOk(pParse)
       && sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Not_Join(pParse,p_ପଦସମଷ୍ଟି)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Code_Run_Just_Once(pParse, p_ପଦସମଷ୍ଟି, -1);
      }
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly) );
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
      pFarg = p_ପଦସମଷ୍ଟି->x.pList;
      nFarg = pFarg ? pFarg->n_ପଦସମଷ୍ଟି : 0;
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      zId = p_ପଦସମଷ୍ଟି->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      ଯଦି( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      ଯଦି( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "unknown function: %#T()", p_ପଦସମଷ୍ଟି);
        ଭାଙ୍ଗ;
      }
      ଯଦି( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        ପ୍ରତିଜ୍ଞା( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        ପ୍ରତିଜ୍ଞା( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }ଅନ୍ୟଥା ଯଦି( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_ପଦସମଷ୍ଟି_Function_Usable(pParse, p_ପଦସମଷ୍ଟି, pDef);
      }

      ପାଇଁ(i=0; i<nFarg; i++){
        ଯଦି( i<32 && sqlite3_ପଦସମଷ୍ଟି_Is_Constant(pParse, pFarg->a[i].p_ପଦସମଷ୍ଟି) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        ଯଦି( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, pFarg->a[i].p_ପଦସମଷ୍ଟି);
        }
      }
      ଯଦି( pFarg ){
        ଯଦି( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }ଅନ୍ୟଥା{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        ଯଦି( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 ପଦସମଷ୍ଟି_Op;
          ପ୍ରତିଜ୍ଞା( nFarg==1 );
          ପ୍ରତିଜ୍ଞା( pFarg->a[0].p_ପଦସମଷ୍ଟି!=0 );
          ପଦସମଷ୍ଟି_Op = pFarg->a[0].p_ପଦସମଷ୍ଟି->op;
          ଯଦି( ପଦସମଷ୍ଟି_Op==TK_COLUMN || ପଦସମଷ୍ଟି_Op==TK_AGG_COLUMN ){
            ପ୍ରତିଜ୍ଞା( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            ପ୍ରତିଜ୍ଞା( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            ପ୍ରତିଜ୍ଞା( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            ପ୍ରତିଜ୍ଞା( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_ପଦସମଷ୍ଟି->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }ଅନ୍ୟଥା{
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
      ଯଦି( nFarg>=2 && ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_ପଦସମଷ୍ଟି);
      }ଅନ୍ୟଥା ଯଦି( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_ପଦସମଷ୍ଟି);
      }
#endif
      ଯଦି( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        ଯଦି( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (ଅକ୍ଷର *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_ପଦସମଷ୍ଟି->op2);
      ଯଦି( nFarg ){
        ଯଦି( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }ଅନ୍ୟଥା{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      ଫେରନ୍ତୁ target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ସ୍ଥିତି TK_EXISTS:
    ସ୍ଥିତି TK_SELECT: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      ଯଦି( pParse->db->mallocFailed ){
        ଫେରନ୍ତୁ 0;
      }ଅନ୍ୟଥା ଯଦି( op==TK_SELECT
             && ALWAYS( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) )
             && (nCol = p_ପଦସମଷ୍ଟି->x.pSelect->pEList->n_ପଦସମଷ୍ଟି)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }ଅନ୍ୟଥା{
        ଫେରନ୍ତୁ sqlite3CodeSubselect(pParse, p_ପଦସମଷ୍ଟି);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_SELECT_COLUMN: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
      ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
      ଯଦି( p_ବାମ->iTable==0 || pParse->withinRJSubrtn > p_ବାମ->op2 ){
        p_ବାମ->iTable = sqlite3CodeSubselect(pParse, p_ବାମ);
        p_ବାମ->op2 = pParse->withinRJSubrtn;
      }
      ପ୍ରତିଜ୍ଞା( p_ବାମ->op==TK_SELECT || p_ବାମ->op==TK_ERROR );
      n = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ବାମ);
      ଯଦି( p_ପଦସମଷ୍ଟି->iTable!=n ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "%d columns assigned %d values",
                                p_ପଦସମଷ୍ଟି->iTable, n);
      }
      ଫେରନ୍ତୁ p_ବାମ->iTable + p_ପଦସମଷ୍ଟି->iColumn;
    }
    ସ୍ଥିତି TK_IN: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_ପଦସମଷ୍ଟି_Code_IN(pParse, p_ପଦସମଷ୍ଟି, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      ଫେରନ୍ତୁ target;
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
    ସ୍ଥିତି TK_BETWEEN: {
      ପଦସମଷ୍ଟି_Code_Between(pParse, p_ପଦସମଷ୍ଟି, target, 0, 0);
      ଫେରନ୍ତୁ target;
    }
    ସ୍ଥିତି TK_COLLATE: {
      ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ );
        sqlite3_ପଦସମଷ୍ଟି_Code(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        ଫେରନ୍ତୁ target;
      }ଅନ୍ୟଥା{
        p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ବାମ;
        ଯାଅ ପଦସମଷ୍ଟି_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    ସ୍ଥିତି TK_SPAN:
    ସ୍ଥିତି TK_UPLUS: {
      p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ବାମ;
      ଯାଅ ପଦସମଷ୍ଟି_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    ସ୍ଥିତି TK_TRIGGER: {
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
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା p1;

      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
      pTab = p_ପଦସମଷ୍ଟି->y.pTab;
      iCol = p_ପଦସମଷ୍ଟି->iColumn;
      p1 = p_ପଦସମଷ୍ଟି->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iTable==0 || p_ପଦସମଷ୍ଟି->iTable==1 );
      ପ୍ରତିଜ୍ଞା( iCol>=-1 && iCol<pTab->nCol );
      ପ୍ରତିଜ୍ଞା( pTab->iPKey<0 || iCol!=pTab->iPKey );
      ପ୍ରତିଜ୍ଞା( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_ପଦସମଷ୍ଟି->iTable ? "new" : "old"),
        (p_ପଦସମଷ୍ଟି->iColumn<0 ? "rowid" : p_ପଦସମଷ୍ଟି->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      ଯଦି( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି TK_VECTOR: {
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "row value misused");
      ଭାଙ୍ଗ;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    ସ୍ଥିତି TK_IF_NULL_ROW: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_ପଦସମଷ୍ଟି->pAggInfo;
      ଯଦି( pAggInfo ){
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iAgg>=0 && p_ପଦସମଷ୍ଟି->iAgg<pAggInfo->nColumn );
        ଯଦି( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_ପଦସମଷ୍ଟି->iAgg);
          ଭାଙ୍ଗ;
        }
        ଯଦି( p_ପଦସମଷ୍ଟି->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_ପଦସମଷ୍ଟି->iAgg].iSorterColumn,
                            target);
          inReg = target;
          ଭାଙ୍ଗ;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_ପଦସମଷ୍ଟି->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, target);
      ପ୍ରତିଜ୍ଞା( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      ଭାଙ୍ଗ;
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
    ସ୍ଥିତି TK_CASE: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା endLabel;                     /* GOTO label for end of CASE stmt */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextCase;                     /* GOTO label for next WHEN clause */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ପଦସମଷ୍ଟି;                        /* 2x number of WHEN terms */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                            /* Loop counter */
      ପଦସମଷ୍ଟି_List *pEList;                 /* List of WHEN terms */
      ସଂରଚନା ପଦସମଷ୍ଟି_List_item *aListelem;  /* Array of WHEN terms */
      ପଦସମଷ୍ଟି opCompare;                   /* The X==Ei expression */
      ପଦସମଷ୍ଟି *pX;                         /* The X expression */
      ପଦସମଷ୍ଟି *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      ପଦସମଷ୍ଟି *pDel = 0;
      sqlite3 *db = pParse->db;

      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) && p_ପଦସମଷ୍ଟି->x.pList!=0 );
      ପ୍ରତିଜ୍ଞା(p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି > 0);
      pEList = p_ପଦସମଷ୍ଟି->x.pList;
      aListelem = pEList->a;
      n_ପଦସମଷ୍ଟି = pEList->n_ପଦସମଷ୍ଟି;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      ଯଦି( (pX = p_ପଦସମଷ୍ଟି->p_ବାମ)!=0 ){
        pDel = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pX, 0);
        ଯଦି( db->mallocFailed ){
          sqlite3_ପଦସମଷ୍ଟି_Delete(db, pDel);
          ଭାଙ୍ଗ;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_ପଦସମଷ୍ଟି_To_Register(pDel, ପଦସମଷ୍ଟି_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, ଆକାର(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_ବାମ = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      ପାଇଁ(i=0; i<n_ପଦସମଷ୍ଟି-1; i=i+2){
        ଯଦି( pX ){
          ପ୍ରତିଜ୍ଞା( pTest!=0 );
          opCompare.p_ଡାହାଣ = aListelem[i].p_ପଦସମଷ୍ଟି;
        }ଅନ୍ୟଥା{
          pTest = aListelem[i].p_ପଦସମଷ୍ଟି;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_ପଦସମଷ୍ଟି->op==TK_COLUMN );
        sqlite3_ପଦସମଷ୍ଟି_Code(pParse, aListelem[i+1].p_ପଦସମଷ୍ଟି, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      ଯଦି( (n_ପଦସମଷ୍ଟି&1)!=0 ){
        sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pEList->a[n_ପଦସମଷ୍ଟି-1].p_ପଦସମଷ୍ଟି, target);
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_ପଦସମଷ୍ଟି_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_TRIGGER
    ସ୍ଥିତି TK_RAISE: {
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି==OE_Rollback
           || p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି==OE_Abort
           || p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି==OE_Fail
           || p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି==OE_Ignore
      );
      ଯଦି( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse,
                       "RAISE() may only be used within a trigger-program");
        ଫେରନ୍ତୁ 0;
      }
      ଯଦି( p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      ଯଦି( p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }ଅନ୍ୟଥା{
        r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି, r1);
      }
      ଭାଙ୍ଗ;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  ଫେରନ୍ତୁ inReg;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,      /* The expression to code when the VDBE initializes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regDest       /* Store the value in this register */
){
  ପଦସମଷ୍ଟି_List *p;
  ପ୍ରତିଜ୍ଞା( ConstFactorOk(pParse) );
  ପ୍ରତିଜ୍ଞା( regDest!=0 );
  p = pParse->p_Const_ପଦସମଷ୍ଟି;
  ଯଦି( regDest<0 && p ){
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(pItem=p->a, i=p->n_ପଦସମଷ୍ଟି; i>0; pItem++, i--){
      ଯଦି( pItem->fg.reusable
       && sqlite3_ପଦସମଷ୍ଟି_Compare(0,pItem->p_ପଦସମଷ୍ଟି,p_ପଦସମଷ୍ଟି,-1)==0
      ){
        ଫେରନ୍ତୁ pItem->u.i_Const_ପଦସମଷ୍ଟି_Reg;
      }
    }
  }
  p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Dup(pParse->db, p_ପଦସମଷ୍ଟି, 0);
  ଯଦି( p_ପଦସମଷ୍ଟି!=0 && ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;
    ପ୍ରତିଜ୍ଞା( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    ଯଦି( !pParse->db->mallocFailed ){
      ଯଦି( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, p_ପଦସମଷ୍ଟି, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_ପଦସମଷ୍ଟି_Delete(pParse->db, p_ପଦସମଷ୍ଟି);
    sqlite3VdbeJumpHere(v, addr);
  }ଅନ୍ୟଥା{
    p = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, p, p_ପଦସମଷ୍ଟି);
    ଯଦି( p ){
       ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem = &p->a[p->n_ପଦସମଷ୍ଟି-1];
       pItem->fg.reusable = regDest<0;
       ଯଦି( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_ପଦସମଷ୍ଟି_Reg = regDest;
    }
    pParse->p_Const_ପଦସମଷ୍ଟି = p;
  }
  ଫେରନ୍ତୁ regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg,        /* First register to set to NULL */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  ପଦସମଷ୍ଟି t;
  memset(&t, 0, ଆକାର(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_ପଦସମଷ୍ଟି_Code_Run_Just_Once(pParse, &t, iReg);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Code_Temp(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pReg){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r2;
  p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(p_ପଦସମଷ୍ଟି);
  ଯଦି( ConstFactorOk(pParse)
   && ALWAYS(p_ପଦସମଷ୍ଟି!=0)
   && p_ପଦସମଷ୍ଟି->op!=TK_REGISTER
   && sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Not_Join(pParse, p_ପଦସମଷ୍ଟି)
  ){
    *pReg  = 0;
    r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Run_Just_Once(pParse, p_ପଦସମଷ୍ଟି, -1);
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, p_ପଦସମଷ୍ଟି, r1);
    ଯଦି( r2==r1 ){
      *pReg = r1;
    }ଅନ୍ୟଥା{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  ଫେରନ୍ତୁ r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Code(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା inReg;

  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି==0 || !ପଦସମଷ୍ଟି_Has_VVAProperty(p_ପଦସମଷ୍ଟି,EP_Immutable) );
  ପ୍ରତିଜ୍ଞା( target>0 && target<=pParse->nMem );
  ପ୍ରତିଜ୍ଞା( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  ଯଦି( pParse->pVdbe==0 ) ଫେରନ୍ତୁ;
  inReg = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, p_ପଦସମଷ୍ଟି, target);
  ଯଦି( inReg!=target ){
    u8 op;
    ପଦସମଷ୍ଟି *pX = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(p_ପଦସମଷ୍ଟି);
    testcase( pX!=p_ପଦସମଷ୍ଟି );
    ଯଦି( ALWAYS(pX)
     && (ପଦସମଷ୍ଟି_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }ଅନ୍ୟଥା{
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Code_Copy(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target){
  sqlite3 *db = pParse->db;
  p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ପଦସମଷ୍ଟି, 0);
  ଯଦି( !db->mallocFailed ) sqlite3_ପଦସମଷ୍ଟି_Code(pParse, p_ପଦସମଷ୍ଟି, target);
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Code_Factorable(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target){
  ଯଦି( pParse->okConstFactor && sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Not_Join(pParse,p_ପଦସମଷ୍ଟି) ){
    sqlite3_ପଦସମଷ୍ଟି_Code_Run_Just_Once(pParse, p_ପଦସମଷ୍ଟି, target);
  }ଅନ୍ୟଥା{
    sqlite3_ପଦସମଷ୍ଟି_Code_Copy(pParse, p_ପଦସମଷ୍ଟି, target);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(
  Parse *pParse,     /* Parsing context */
  ପଦସମଷ୍ଟି_List *pList,   /* The expression list to be coded */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା target,        /* Where to write results */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  ପ୍ରତିଜ୍ଞା( pList!=0 );
  ପ୍ରତିଜ୍ଞା( target>0 );
  ପ୍ରତିଜ୍ଞା( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_ପଦସମଷ୍ଟି;
  ଯଦି( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  ପାଇଁ(pItem=pList->a, i=0; i<n; i++, pItem++){
    ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pItem->p_ପଦସମଷ୍ଟି;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ଯଦି( pItem->fg.bSorterRef ){
      i--;
      n--;
    }ଅନ୍ୟଥା
#endif
    ଯଦି( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      ଯଦି( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }ଅନ୍ୟଥା ଯଦି( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Not_Join(pParse,p_ପଦସମଷ୍ଟି)
    ){
      sqlite3_ପଦସମଷ୍ଟି_Code_Run_Just_Once(pParse, p_ପଦସମଷ୍ଟି, target+i);
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା inReg = sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, p_ପଦସମଷ୍ଟି, target+i);
      ଯଦି( inReg!=target+i ){
        VdbeOp *pOp;
        ଯଦି( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }ଅନ୍ୟଥା{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  ଫେରନ୍ତୁ n;
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
ସ୍ଥିର ଶୂନ୍ୟ ପଦସମଷ୍ଟି_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,      /* The BETWEEN expression */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dest,         /* Jump destination or storage location */
  ଶୂନ୍ୟ (*xJump)(Parse*,ପଦସମଷ୍ଟି*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା), /* Action to take */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  ପଦସମଷ୍ଟି ପଦସମଷ୍ଟି_And;     /* The AND operator in  x>=y AND x<=z  */
  ପଦସମଷ୍ଟି comp_ବାମ;    /* The  x>=y  term */
  ପଦସମଷ୍ଟି comp_ଡାହାଣ;   /* The  x<=z  term */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regFree1 = 0; /* Temporary use register */
  ପଦସମଷ୍ଟି *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_ବାମ, 0, ଆକାର(ପଦସମଷ୍ଟି));
  memset(&comp_ଡାହାଣ, 0, ଆକାର(ପଦସମଷ୍ଟି));
  memset(&ପଦସମଷ୍ଟି_And, 0, ଆକାର(ପଦସମଷ୍ଟି));

  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
  pDel = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
  ଯଦି( db->mallocFailed==0 ){
    ପଦସମଷ୍ଟି_And.op = TK_AND;
    ପଦସମଷ୍ଟି_And.p_ବାମ = &comp_ବାମ;
    ପଦସମଷ୍ଟି_And.p_ଡାହାଣ = &comp_ଡାହାଣ;
    comp_ବାମ.op = TK_GE;
    comp_ବାମ.p_ବାମ = pDel;
    comp_ବାମ.p_ଡାହାଣ = p_ପଦସମଷ୍ଟି->x.pList->a[0].p_ପଦସମଷ୍ଟି;
    comp_ଡାହାଣ.op = TK_LE;
    comp_ଡାହାଣ.p_ବାମ = pDel;
    comp_ଡାହାଣ.p_ଡାହାଣ = p_ପଦସମଷ୍ଟି->x.pList->a[1].p_ପଦସମଷ୍ଟି;
    sqlite3_ପଦସମଷ୍ଟି_To_Register(pDel, ପଦସମଷ୍ଟି_Code_Vector(pParse, pDel, &regFree1));
    ଯଦି( xJump ){
      xJump(pParse, &ପଦସମଷ୍ଟି_And, dest, jumpIfNull);
    }ଅନ୍ୟଥା{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, &ପଦସମଷ୍ଟି_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_ପଦସମଷ୍ଟି_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_ପଦସମଷ୍ଟି_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_ପଦସମଷ୍ଟି_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_ପଦସମଷ୍ଟି_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_ପଦସମଷ୍ଟି_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_ପଦସମଷ୍ଟି_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_ପଦସମଷ୍ଟି_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_ପଦସମଷ୍ଟି_If_False && jumpIfNull!=0 && regFree1!=0 );
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_If_True(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regFree1 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regFree2 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2;

  ପ୍ରତିଜ୍ଞା( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ଯଦି( NEVER(v==0) )     ଫେରନ୍ତୁ;  /* Existence of VDBE checked by caller */
  ଯଦି( NEVER(p_ପଦସମଷ୍ଟି==0) ) ଫେରନ୍ତୁ;  /* No way this can happen */
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_VVAProperty(p_ପଦସମଷ୍ଟି, EP_Immutable) );
  op = p_ପଦସମଷ୍ଟି->op;
  ବାଛ( op ){
    ସ୍ଥିତି TK_AND:
    ସ୍ଥିତି TK_OR: {
      ପଦସମଷ୍ଟି *pAlt = sqlite3_ପଦସମଷ୍ଟି_Simplified_And_Or(p_ପଦସମଷ୍ଟି);
      ଯଦି( pAlt!=p_ପଦସମଷ୍ଟି ){
        sqlite3_ପଦସମଷ୍ଟି_If_True(pParse, pAlt, dest, jumpIfNull);
      }ଅନ୍ୟଥା{
        ପଦସମଷ୍ଟି *pFirst, *pSecond;
        ଯଦି( ପଦସମଷ୍ଟି_Eval_Rhs_First(p_ପଦସମଷ୍ଟି) ){
          pFirst = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
          pSecond = p_ପଦସମଷ୍ଟି->p_ବାମ;
        }ଅନ୍ୟଥା{
          pFirst = p_ପଦସମଷ୍ଟି->p_ବାମ;
          pSecond = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
        }
        ଯଦି( op==TK_AND ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_ପଦସମଷ୍ଟି_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }ଅନ୍ୟଥା{
          testcase( jumpIfNull==0 );
          sqlite3_ପଦସମଷ୍ଟି_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_ପଦସମଷ୍ଟି_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, dest, jumpIfNull);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_TRUTH: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା isNot;      /* IS NOT TRUE or IS NOT FALSE */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_ପଦସମଷ୍ଟି->op2==TK_ISNOT;
      isTrue = sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ଯଦି( isTrue ^ isNot ){
        sqlite3_ପଦସମଷ୍ଟି_If_True(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }ଅନ୍ୟଥା{
        sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_IS:
    ସ୍ଥିତି TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ସ୍ଥିତି TK_LT:
    ସ୍ଥିତି TK_LE:
    ସ୍ଥିତି TK_GT:
    ସ୍ଥିତି TK_GE:
    ସ୍ଥିତି TK_NE:
    ସ୍ଥିତି TK_EQ: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrIsNull;
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Vector(p_ପଦସମଷ୍ଟି->p_ବାମ) ) ଯାଅ default_ପଦସମଷ୍ଟି;
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = ପଦସମଷ୍ଟି_Compute_Operands(pParse, p_ପଦସମଷ୍ଟି,
                                   &r1, &r2, &regFree1, &regFree2);
      }ଅନ୍ୟଥା{
        r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
        r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, op,
                  r1, r2, dest, jumpIfNull, ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_Commuted));
      ପ୍ରତିଜ୍ଞା(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ପ୍ରତିଜ୍ଞା(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ପ୍ରତିଜ୍ଞା(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ପ୍ରତିଜ୍ଞା(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ପ୍ରତିଜ୍ଞା(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      ପ୍ରତିଜ୍ଞା(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ଯଦି( addrIsNull ){
        ଯଦି( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ଅନ୍ୟଥା{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_ISNULL:
    ସ୍ଥିତି TK_NOTNULL: {
      ପ୍ରତିଜ୍ଞା( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ପ୍ରତିଜ୍ଞା( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
      ପ୍ରତିଜ୍ଞା( regFree1==0 || regFree1==r1 );
      ଯଦି( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      ପଦସମଷ୍ଟି_Code_Between(pParse, p_ପଦସମଷ୍ଟି, dest, sqlite3_ପଦସମଷ୍ଟି_If_True, jumpIfNull);
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ସ୍ଥିତି TK_IN: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_ପଦସମଷ୍ଟି_Code_IN(pParse, p_ପଦସମଷ୍ଟି, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      ଭାଙ୍ଗ;
    }
#endif
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
    default_ପଦସମଷ୍ଟି:
      ଯଦି( ପଦସମଷ୍ଟି_Always_True(p_ପଦସମଷ୍ଟି) ){
        sqlite3VdbeGoto(v, dest);
      }ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Always_False(p_ପଦସମଷ୍ଟି) ){
        /* No-op */
      }ଅନ୍ୟଥା{
        r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ଭାଙ୍ଗ;
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_If_False(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regFree1 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regFree2 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2;

  ପ୍ରତିଜ୍ଞା( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ଯଦି( NEVER(v==0) ) ଫେରନ୍ତୁ; /* Existence of VDBE checked by caller */
  ଯଦି( p_ପଦସମଷ୍ଟି==0 )    ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_VVAProperty(p_ପଦସମଷ୍ଟି,EP_Immutable) );

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
  op = ((p_ପଦସମଷ୍ଟି->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_ISNULL || op==OP_NotNull );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_NOTNULL || op==OP_IsNull );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_NE || op==OP_Eq );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_EQ || op==OP_Ne );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_LT || op==OP_Ge );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_LE || op==OP_Gt );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_GT || op==OP_Le );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_GE || op==OP_Lt );

  ବାଛ( p_ପଦସମଷ୍ଟି->op ){
    ସ୍ଥିତି TK_AND:
    ସ୍ଥିତି TK_OR: {
      ପଦସମଷ୍ଟି *pAlt = sqlite3_ପଦସମଷ୍ଟି_Simplified_And_Or(p_ପଦସମଷ୍ଟି);
      ଯଦି( pAlt!=p_ପଦସମଷ୍ଟି ){
        sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pAlt, dest, jumpIfNull);
      }ଅନ୍ୟଥା{
        ପଦସମଷ୍ଟି *pFirst, *pSecond;
        ଯଦି( ପଦସମଷ୍ଟି_Eval_Rhs_First(p_ପଦସମଷ୍ଟି) ){
          pFirst = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
          pSecond = p_ପଦସମଷ୍ଟି->p_ବାମ;
        }ଅନ୍ୟଥା{
          pFirst = p_ପଦସମଷ୍ଟି->p_ବାମ;
          pSecond = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
        }
        ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pSecond, dest, jumpIfNull);
        }ଅନ୍ୟଥା{
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_ପଦସମଷ୍ଟି_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_ପଦସମଷ୍ଟି_If_True(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, dest, jumpIfNull);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_TRUTH: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା isNot;   /* IS NOT TRUE or IS NOT FALSE */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_ପଦସମଷ୍ଟି->op2==TK_ISNOT;
      isTrue = sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ଯଦି( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }ଅନ୍ୟଥା{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_ପଦସମଷ୍ଟି_If_True(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_IS:
    ସ୍ଥିତି TK_ISNOT:
      testcase( p_ପଦସମଷ୍ଟି->op==TK_IS );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_ISNOT );
      op = (p_ପଦସମଷ୍ଟି->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ସ୍ଥିତି TK_LT:
    ସ୍ଥିତି TK_LE:
    ସ୍ଥିତି TK_GT:
    ସ୍ଥିତି TK_GE:
    ସ୍ଥିତି TK_NE:
    ସ୍ଥିତି TK_EQ: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrIsNull;
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Vector(p_ପଦସମଷ୍ଟି->p_ବାମ) ) ଯାଅ default_ପଦସମଷ୍ଟି;
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = ପଦସମଷ୍ଟି_Compute_Operands(pParse, p_ପଦସମଷ୍ଟି,
                                   &r1, &r2, &regFree1, &regFree2);
      }ଅନ୍ୟଥା{
        r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
        r2 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, op,
                  r1, r2, dest, jumpIfNull,ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_Commuted));
      ପ୍ରତିଜ୍ଞା(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ପ୍ରତିଜ୍ଞା(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ପ୍ରତିଜ୍ଞା(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ପ୍ରତିଜ୍ଞା(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ପ୍ରତିଜ୍ଞା(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      ପ୍ରତିଜ୍ଞା(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ଯଦି( addrIsNull ){
        ଯଦି( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ଅନ୍ୟଥା{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_ISNULL:
    ସ୍ଥିତି TK_NOTNULL: {
      r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, &regFree1);
      ପ୍ରତିଜ୍ଞା( regFree1==0 || regFree1==r1 );
      ଯଦି( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      ପଦସମଷ୍ଟି_Code_Between(pParse, p_ପଦସମଷ୍ଟି, dest, sqlite3_ପଦସମଷ୍ଟି_If_False, jumpIfNull);
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ସ୍ଥିତି TK_IN: {
      ଯଦି( jumpIfNull ){
        sqlite3_ପଦସମଷ୍ଟି_Code_IN(pParse, p_ପଦସମଷ୍ଟି, dest, dest);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_ପଦସମଷ୍ଟି_Code_IN(pParse, p_ପଦସମଷ୍ଟି, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      ଭାଙ୍ଗ;
    }
#endif
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
    default_ପଦସମଷ୍ଟି:
      ଯଦି( ପଦସମଷ୍ଟି_Always_False(p_ପଦସମଷ୍ଟି) ){
        sqlite3VdbeGoto(v, dest);
      }ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Always_True(p_ପଦସମଷ୍ଟି) ){
        /* no-op */
      }ଅନ୍ୟଥା{
        r1 = sqlite3_ପଦସମଷ୍ଟି_Code_Temp(pParse, p_ପଦସମଷ୍ଟି, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ଭାଙ୍ଗ;
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_If_False_Dup(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dest,ପୂର୍ଣ୍ଣସଂଖ୍ୟା jumpIfNull){
  sqlite3 *db = pParse->db;
  ପଦସମଷ୍ଟି *pCopy = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ପଦସମଷ୍ଟି, 0);
  ଯଦି( db->mallocFailed==0 ){
    sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pCopy);
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
ସ୍ଥିର SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Compare_Variable(
  ଅଚଳ Parse *pParse,
  ଅଚଳ ପଦସମଷ୍ଟି *pVar,
  ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 2;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iVar;
  sqlite3_ମୂଲ୍ୟ *pL, *pR = 0;
 
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_VARIABLE && pVar->iColumn==p_ପଦସମଷ୍ଟି->iColumn ){
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ଫେରନ୍ତୁ 2;
  sqlite3_ମୂଲ୍ୟ_From_ପଦସମଷ୍ଟି(pParse->db, p_ପଦସମଷ୍ଟି, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  ଯଦି( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_ମୂଲ୍ୟ(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    ଯଦି( pL ){
      ଯଦି( sqlite3_ମୂଲ୍ୟ_type(pL)==SQLITE_TEXT ){
        sqlite3_ମୂଲ୍ୟ_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_ମୂଲ୍ୟ_Free(pR);
    sqlite3_ମୂଲ୍ୟ_Free(pL);
  }
  ଫେରନ୍ତୁ res;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Compare(
  ଅଚଳ Parse *pParse,
  ଅଚଳ ପଦସମଷ୍ଟି *pA,
  ଅଚଳ ପଦସମଷ୍ଟି *pB,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab
){
  u32 combinedFlags;
  ଯଦି( pA==0 || pB==0 ){
    ଫେରନ୍ତୁ pB==pA ? 0 : 2;
  }
  ଯଦି( pParse && pA->op==TK_VARIABLE ){
    ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  ଯଦି( combinedFlags & EP_Int_ମୂଲ୍ୟ ){
    ଯଦି( (pA->flags&pB->flags&EP_Int_ମୂଲ୍ୟ)!=0 && pA->u.i_ମୂଲ୍ୟ==pB->u.i_ମୂଲ୍ୟ ){
      ଫେରନ୍ତୁ 0;
    }
    ଫେରନ୍ତୁ 2;
  }
  ଯଦି( pA->op!=pB->op || pA->op==TK_RAISE ){
    ଯଦି( pA->op==TK_COLLATE && sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, pA->p_ବାମ,pB,iTab)<2 ){
      ଫେରନ୍ତୁ 1;
    }
    ଯଦି( pB->op==TK_COLLATE && sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, pA,pB->p_ବାମ,iTab)<2 ){
      ଫେରନ୍ତୁ 1;
    }
    ଯଦି( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }ଅନ୍ୟଥା{
      ଫେରନ୍ତୁ 2;
    }
  }
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pA, EP_Int_ମୂଲ୍ୟ) );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pB, EP_Int_ମୂଲ୍ୟ) );
  ଯଦି( pA->u.zToken ){
    ଯଦି( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      ଯଦି( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) ଫେରନ୍ତୁ 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      ପ୍ରତିଜ୍ଞା( pA->op==pB->op );
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pA,EP_WinFunc)!=ପଦସମଷ୍ଟି_Has_Property(pB,EP_WinFunc) ){
        ଫେରନ୍ତୁ 2;
      }
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pA,EP_WinFunc) ){
        ଯଦି( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          ଫେରନ୍ତୁ 2;
        }
      }
#endif
    }ଅନ୍ୟଥା ଯଦି( pA->op==TK_NULL ){
      ଫେରନ୍ତୁ 0;
    }ଅନ୍ୟଥା ଯଦି( pA->op==TK_COLLATE ){
      ଯଦି( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) ଫେରନ୍ତୁ 2;
    }ଅନ୍ୟଥା
    ଯଦି( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      ଫେରନ୍ତୁ 2;
    }
  }
  ଯଦି( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) ଫେରନ୍ତୁ 2;
  ଯଦି( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    ଯଦି( combinedFlags & EP_xIsSelect ) ଫେରନ୍ତୁ 2;
    ଯଦି( (combinedFlags & EP_FixedCol)==0
     && sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, pA->p_ବାମ, pB->p_ବାମ, iTab) ) ଫେରନ୍ତୁ 2;
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, pA->p_ଡାହାଣ, pB->p_ଡାହାଣ, iTab) ) ଫେରନ୍ତୁ 2;
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_List_Compare(pA->x.pList, pB->x.pList, iTab) ) ଫେରନ୍ତୁ 2;
    ଯଦି( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      ଯଦି( pA->iColumn!=pB->iColumn ) ଫେରନ୍ତୁ 2;
      ଯଦି( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) ଫେରନ୍ତୁ 2;
      ଯଦି( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        ଫେରନ୍ତୁ 2;
      }
    }
  }
  ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_List_Compare(ଅଚଳ ପଦସମଷ୍ଟି_List *pA, ଅଚଳ ପଦସମଷ୍ଟି_List *pB, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( pA==0 && pB==0 ) ଫେରନ୍ତୁ 0;
  ଯଦି( pA==0 || pB==0 ) ଫେରନ୍ତୁ 1;
  ଯଦି( pA->n_ପଦସମଷ୍ଟି!=pB->n_ପଦସମଷ୍ଟି ) ଫେରନ୍ତୁ 1;
  ପାଇଁ(i=0; i<pA->n_ପଦସମଷ୍ଟି; i++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
    ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି_A = pA->a[i].p_ପଦସମଷ୍ଟି;
    ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି_B = pB->a[i].p_ପଦସମଷ୍ଟି;
    ଯଦି( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) ଫେରନ୍ତୁ 1;
    ଯଦି( (res = sqlite3_ପଦସମଷ୍ଟି_Compare(0, p_ପଦସମଷ୍ଟି_A, p_ପଦସମଷ୍ଟି_B, iTab)) ) ଫେରନ୍ତୁ res;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Compare_Skip(ପଦସମଷ୍ଟି *pA,ପଦସମଷ୍ଟି *pB, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab){
  ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Compare(0,
             sqlite3_ପଦସମଷ୍ଟି_Skip_Collate(pA),
             sqlite3_ପଦସମଷ୍ଟି_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Implies_Not_Null(
  ଅଚଳ Parse *pParse,/* Parsing context */
  ଅଚଳ ପଦସମଷ୍ଟି *p,      /* The expression to be checked */
  ଅଚଳ ପଦସମଷ୍ଟି *pNN,    /* The expression that is NOT NULL */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab,           /* Table being evaluated */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା seenNot         /* Return true only if p can be any non-NULL value */
){
  ପ୍ରତିଜ୍ଞା( p );
  ପ୍ରତିଜ୍ଞା( pNN );
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, p, pNN, iTab)==0 ){
    ଫେରନ୍ତୁ pNN->op!=TK_NULL;
  }
  ବାଛ( p->op ){
    ସ୍ଥିତି TK_IN: {
      ଯଦି( seenNot && ପଦସମଷ୍ଟି_Has_Property(p, EP_xIsSelect) ) ଫେରନ୍ତୁ 0;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_ପଦସମଷ୍ଟି>0) );
      ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, p->p_ବାମ, pNN, iTab, 1);
    }
    ସ୍ଥିତି TK_BETWEEN: {
      ପଦସମଷ୍ଟି_List *pList;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p) );
      pList = p->x.pList;
      ପ୍ରତିଜ୍ଞା( pList!=0 );
      ପ୍ରତିଜ୍ଞା( pList->n_ପଦସମଷ୍ଟି==2 );
      ଯଦି( seenNot ) ଫେରନ୍ତୁ 0;
      ଯଦି( ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, pList->a[0].p_ପଦସମଷ୍ଟି, pNN, iTab, 1)
       || ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, pList->a[1].p_ପଦସମଷ୍ଟି, pNN, iTab, 1)
      ){
        ଫେରନ୍ତୁ 1;
      }
      ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, p->p_ବାମ, pNN, iTab, 1);
    }
    ସ୍ଥିତି TK_EQ:
    ସ୍ଥିତି TK_NE:
    ସ୍ଥିତି TK_LT:
    ସ୍ଥିତି TK_LE:
    ସ୍ଥିତି TK_GT:
    ସ୍ଥିତି TK_GE:
    ସ୍ଥିତି TK_PLUS:
    ସ୍ଥିତି TK_MINUS:
    ସ୍ଥିତି TK_BITOR:
    ସ୍ଥିତି TK_LSHIFT:
    ସ୍ଥିତି TK_RSHIFT:
    ସ୍ଥିତି TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    ସ୍ଥିତି TK_STAR:
    ସ୍ଥିତି TK_REM:
    ସ୍ଥିତି TK_BITAND:
    ସ୍ଥିତି TK_SLASH: {
      ଯଦି( ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, p->p_ଡାହାଣ, pNN, iTab, seenNot) ) ଫେରନ୍ତୁ 1;
      /* no break */ deliberate_fall_through
    }
    ସ୍ଥିତି TK_SPAN:
    ସ୍ଥିତି TK_COLLATE:
    ସ୍ଥିତି TK_UPLUS:
    ସ୍ଥିତି TK_UMINUS: {
      ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, p->p_ବାମ, pNN, iTab, seenNot);
    }
    ସ୍ଥିତି TK_TRUTH: {
      ଯଦି( seenNot ) ଫେରନ୍ତୁ 0;
      ଯଦି( p->op2!=TK_IS ) ଫେରନ୍ତୁ 0;
      ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, p->p_ବାମ, pNN, iTab, 1);
    }
    ସ୍ଥିତି TK_BITNOT:
    ସ୍ଥିତି TK_NOT: {
      ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, p->p_ବାମ, pNN, iTab, 1);
    }
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Not_True(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା v;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_NULL ) ଫେରନ୍ତୁ 1;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_TRUEFALSE && sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(p_ପଦସମଷ୍ଟି)==0 ) ଫେରନ୍ତୁ 1;
  v = 1;
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Integer(p_ପଦସମଷ୍ଟି, &v, 0, 0) && v==0 ) ଫେରନ୍ତୁ 1;
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_IIF(sqlite3 *db, ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପଦସମଷ୍ଟି_List *pList;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION ){
    ଅଚଳ ଅକ୍ଷର *z = p_ପଦସମଷ୍ଟି->u.zToken;
    FuncDef *pDef;
    ଯଦି( (z[0]!='i' && z[0]!='I') ) ଫେରନ୍ତୁ 0;
    ଯଦି( p_ପଦସମଷ୍ଟି->x.pList==0 ) ଫେରନ୍ତୁ 0;
    pDef = sqlite3FindFunction(db, z, p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    ଯଦି( pDef==0 ) ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
    ଯଦି( NEVER(pDef==0) ) ଫେରନ୍ତୁ 0;
#endif
    ଯଦି( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) ଫେରନ୍ତୁ 0;
    ଯଦି( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_CASE ){
    ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ!=0 ) ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
  }
  pList = p_ପଦସମଷ୍ଟି->x.pList;
  ପ୍ରତିଜ୍ଞା( pList!=0 );
  ଯଦି( pList->n_ପଦସମଷ୍ଟି==2 ) ଫେରନ୍ତୁ 1;
  ଯଦି( pList->n_ପଦସମଷ୍ଟି==3 && sqlite3_ପଦସମଷ୍ଟି_Is_Not_True(pList->a[2].p_ପଦସମଷ୍ଟି) ) ଫେରନ୍ତୁ 1;
  ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Implies_ପଦସମଷ୍ଟି(
  ଅଚଳ Parse *pParse,
  ଅଚଳ ପଦସମଷ୍ଟି *pE1,
  ଅଚଳ ପଦସମଷ୍ଟି *pE2,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab
){
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, pE1, pE2, iTab)==0 ){
    ଫେରନ୍ତୁ 1;
  }
  ଯଦି( pE2->op==TK_OR
   && (sqlite3_ପଦସମଷ୍ଟି_Implies_ପଦସମଷ୍ଟି(pParse, pE1, pE2->p_ବାମ, iTab)
             || sqlite3_ପଦସମଷ୍ଟି_Implies_ପଦସମଷ୍ଟି(pParse, pE1, pE2->p_ଡାହାଣ, iTab) )
  ){
    ଫେରନ୍ତୁ 1;
  }
  ଯଦି( pE2->op==TK_NOTNULL
   && ପଦସମଷ୍ଟି_Implies_Not_Null(pParse, pE1, pE2->p_ବାମ, iTab, 0)
  ){
    ଫେରନ୍ତୁ 1;
  }
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_IIF(pParse->db, pE1) ){
    ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Implies_ପଦସମଷ୍ଟି(pParse,pE1->x.pList->a[0].p_ପଦସମଷ୍ଟି,pE2,iTab);
  }
  ଫେରନ୍ତୁ 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ସ୍ଥିର ଶୂନ୍ୟ bothImplyNotNullRow(Walker *pWalker, ପଦସମଷ୍ଟି *pE1, ପଦସମଷ୍ଟି *pE2){
  ଯଦି( pWalker->eCode==0 ){
    sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pE1);
    ଯଦି( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pE2);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା impliesNotNullRow(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  testcase( p_ପଦସମଷ୍ଟି->op==TK_AGG_COLUMN );
  testcase( p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION );
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) ) ଫେରନ୍ତୁ WRC_Prune;
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    ଫେରନ୍ତୁ WRC_Prune;
  }
  ବାଛ( p_ପଦସମଷ୍ଟି->op ){
    ସ୍ଥିତି TK_ISNOT:
    ସ୍ଥିତି TK_ISNULL:
    ସ୍ଥିତି TK_NOTNULL:
    ସ୍ଥିତି TK_IS:
    ସ୍ଥିତି TK_VECTOR:
    ସ୍ଥିତି TK_FUNCTION:
    ସ୍ଥିତି TK_TRUTH:
    ସ୍ଥିତି TK_CASE:
      testcase( p_ପଦସମଷ୍ଟି->op==TK_ISNOT );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_ISNULL );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_NOTNULL );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_IS );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_VECTOR );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_TRUTH );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_CASE );
      ଫେରନ୍ତୁ WRC_Prune;

    ସ୍ଥିତି TK_COLUMN:
      ଯଦି( pWalker->u.iCur==p_ପଦସମଷ୍ଟି->iTable ){
        pWalker->eCode = 1;
        ଫେରନ୍ତୁ WRC_Abort;
      }
      ଫେରନ୍ତୁ WRC_Prune;

    ସ୍ଥିତି TK_OR:
    ସ୍ଥିତି TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_ପଦସମଷ୍ଟି->op==TK_OR );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_ପଦସମଷ୍ଟି->p_ବାମ, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
      ଫେରନ୍ତୁ WRC_Prune;
       
    ସ୍ଥିତି TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      ଯଦି( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) && ALWAYS(p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି>0) ){
        sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, p_ପଦସମଷ୍ଟି->p_ବାମ);
      }
      ଫେରନ୍ତୁ WRC_Prune;

    ସ୍ଥିତି TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି==2 );
      sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, p_ପଦସମଷ୍ଟି->p_ବାମ);
      bothImplyNotNullRow(pWalker, p_ପଦସମଷ୍ଟି->x.pList->a[0].p_ପଦସମଷ୍ଟି,
                                   p_ପଦସମଷ୍ଟି->x.pList->a[1].p_ପଦସମଷ୍ଟି);
      ଫେରନ୍ତୁ WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    ସ୍ଥିତି TK_EQ:
    ସ୍ଥିତି TK_NE:
    ସ୍ଥିତି TK_LT:
    ସ୍ଥିତି TK_LE:
    ସ୍ଥିତି TK_GT:
    ସ୍ଥିତି TK_GE: {
      ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
      ପଦସମଷ୍ଟି *p_ଡାହାଣ = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
      testcase( p_ପଦସମଷ୍ଟି->op==TK_EQ );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_NE );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_LT );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_LE );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_GT );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      ପ୍ରତିଜ୍ଞା( p_ବାମ->op!=TK_COLUMN || ପଦସମଷ୍ଟି_Use_YTab(p_ବାମ) );
      ପ୍ରତିଜ୍ଞା( p_ଡାହାଣ->op!=TK_COLUMN || ପଦସମଷ୍ଟି_Use_YTab(p_ଡାହାଣ) );
      ଯଦି( (p_ବାମ->op==TK_COLUMN
           && ALWAYS(p_ବାମ->y.pTab!=0)
           && IsVirtual(p_ବାମ->y.pTab))
       || (p_ଡାହାଣ->op==TK_COLUMN
           && ALWAYS(p_ଡାହାଣ->y.pTab!=0)
           && IsVirtual(p_ଡାହାଣ->y.pTab))
      ){
        ଫେରନ୍ତୁ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ WRC_Continue;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Implies_Non_Null_Row(ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isRJ){
  Walker w;
  p = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(p);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  ଯଦି( p->op==TK_NOTNULL ){
    p = p->p_ବାମ;
  }ଅନ୍ୟଥା{
    ଯେତେବେଳେ( p->op==TK_AND ){
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Implies_Non_Null_Row(p->p_ବାମ, iTab, isRJ) ) ଫେରନ୍ତୁ 1;
      p = p->p_ଡାହାଣ;
    }
  }
  w.x_ପଦସମଷ୍ଟି_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p);
  ଫେରନ୍ତୁ w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
ସଂରଚନା IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Idx_Cover(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN
   && p_ପଦସମଷ୍ଟି->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_ପଦସମଷ୍ଟି->iColumn)<0
  ){
    pWalker->eCode = 1;
    ଫେରନ୍ତୁ WRC_Abort;
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Covered_By_Index(
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,        /* The index to be tested */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  ସଂରଚନା IdxCover xcov;
  memset(&w, 0, ଆକାର(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_ପଦସମଷ୍ଟି_Callback = ପଦସମଷ୍ଟି_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p_ପଦସମଷ୍ଟି);
  ଫେରନ୍ତୁ !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
ସଂରଚନା RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା selectRefEnter(Walker *pWalker, Select *pSelect){
  ସଂରଚନା RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piNew;
  ଯଦି( pSrc->nSrc==0 ) ଫେରନ୍ତୁ WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
  ଯଦି( piNew==0 ){
    p->nExclude = 0;
    ଫେରନ୍ତୁ WRC_Abort;
  }ଅନ୍ୟଥା{
    p->aiExclude = piNew;
  }
  ପାଇଁ(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  ଫେରନ୍ତୁ WRC_Continue;
}
ସ୍ଥିର ଶୂନ୍ୟ selectRefLeave(Walker *pWalker, Select *pSelect){
  ସଂରଚନା RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  ଯଦି( p->nExclude ){
    ପ୍ରତିଜ୍ଞା( p->nExclude>=pSrc->nSrc );
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Ref_To_Src_List(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN
   || p_ପଦସମଷ୍ଟି->op==TK_AGG_COLUMN
  ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ସଂରଚନା RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSrc = pSrc ? pSrc->nSrc : 0;
    ପାଇଁ(i=0; i<nSrc; i++){
      ଯଦି( p_ପଦସମଷ୍ଟି->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        ଫେରନ୍ତୁ WRC_Continue;
      }
    }
    ପାଇଁ(i=0; i<p->nExclude && p->aiExclude[i]!=p_ପଦସମଷ୍ଟି->iTable; i++){}
    ଯଦି( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ReferencesSrcList(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, SrcList *pSrcList){
  Walker w;
  ସଂରଚନା RefSrcList x;
  ପ୍ରତିଜ୍ଞା( pParse->db!=0 );
  memset(&w, 0, ଆକାର(w));
  memset(&x, 0, ଆକାର(x));
  w.x_ପଦସମଷ୍ଟି_Callback = ପଦସମଷ୍ଟି_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION );
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
  sqlite3_Walk_ପଦସମଷ୍ଟି_List(&w, p_ପଦସମଷ୍ଟି->x.pList);
  ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ ){
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ->op==TK_ORDER );
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି->p_ବାମ) );
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ->x.pList!=0 );
    sqlite3_Walk_ପଦସମଷ୍ଟି_List(&w, p_ପଦସମଷ୍ଟି->p_ବାମ->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
    sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p_ପଦସମଷ୍ଟି->y.pWin->pFilter);
  }
#endif
  ଯଦି( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  ଯଦି( w.eCode & 0x01 ){
    ଫେରନ୍ତୁ 1;
  }ଅନ୍ୟଥା ଯଦି( w.eCode ){
    ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ -1;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା agginfo_Persist_ପଦସମଷ୍ଟି_Cb(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( ALWAYS(!ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly|EP_Reduced))
   && p_ପଦସମଷ୍ଟି->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_ପଦସମଷ୍ଟି->pAggInfo;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAgg = p_ପଦସମଷ୍ଟି->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    ପ୍ରତିଜ୍ଞା( iAgg>=0 );
    ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_AGG_FUNCTION ){
      ଯଦି( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_ପଦସମଷ୍ଟି
      ){
        p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ପଦସମଷ୍ଟି, 0);
        ଯଦି( p_ପଦସମଷ୍ଟି && !sqlite3_ପଦସମଷ୍ଟି_Deferred_Delete(pParse, p_ପଦସମଷ୍ଟି) ){
          pAggInfo->aCol[iAgg].pCExpr = p_ପଦସମଷ୍ଟି;
        }
      }
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION );
      ଯଦି( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_ପଦସମଷ୍ଟି
      ){
        p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ପଦସମଷ୍ଟି, 0);
        ଯଦି( p_ପଦସମଷ୍ଟି && !sqlite3_ପଦସମଷ୍ଟି_Deferred_Delete(pParse, p_ପଦସମଷ୍ଟି) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_ପଦସମଷ୍ଟି;
        }
      }
    }
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
ଶୂନ୍ୟ sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, ଆକାର(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_ପଦସମଷ୍ଟି_Callback = agginfo_Persist_ପଦସମଷ୍ଟି_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       ଆକାର(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  ଫେରନ୍ତୁ i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       ଆକାର(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  ଫେରନ୍ତୁ i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
ସ୍ଥିର ଶୂନ୍ୟ findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି          /* Expr describing the column to find or insert */
){
  ସଂରଚନା AggInfo_col *pCol;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା k;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  ପ୍ରତିଜ୍ଞା( mxTerm <= SMXV(i16) );
  ପ୍ରତିଜ୍ଞା( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  ପାଇଁ(k=0; k<pAggInfo->nColumn; k++, pCol++){
    ଯଦି( pCol->pCExpr==p_ପଦସମଷ୍ଟି ) ଫେରନ୍ତୁ;
    ଯଦି( pCol->iTable==p_ପଦସମଷ୍ଟି->iTable
     && pCol->iColumn==p_ପଦସମଷ୍ଟି->iColumn
     && p_ପଦସମଷ୍ଟି->op!=TK_IF_NULL_ROW
    ){
      ଯାଅ fix_up_ପଦସମଷ୍ଟି;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  ଯଦି( k<0 ){
    /* OOM on resize */
    ପ୍ରତିଜ୍ଞା( pParse->db->mallocFailed );
    ଫେରନ୍ତୁ;
  }
  ଯଦି( k>mxTerm ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
  pCol->pTab = p_ପଦସମଷ୍ଟି->y.pTab;
  pCol->iTable = p_ପଦସମଷ୍ଟି->iTable;
  pCol->iColumn = p_ପଦସମଷ୍ଟି->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_ପଦସମଷ୍ଟି;
  ଯଦି( pAggInfo->pGroupBy && p_ପଦସମଷ୍ଟି->op!=TK_IF_NULL_ROW ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା j, n;
    ପଦସମଷ୍ଟି_List *pGB = pAggInfo->pGroupBy;
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pTerm = pGB->a;
    n = pGB->n_ପଦସମଷ୍ଟି;
    ପାଇଁ(j=0; j<n; j++, pTerm++){
      ପଦସମଷ୍ଟି *pE = pTerm->p_ପଦସମଷ୍ଟି;
      ଯଦି( pE->op==TK_COLUMN
       && pE->iTable==p_ପଦସମଷ୍ଟି->iTable
       && pE->iColumn==p_ପଦସମଷ୍ଟି->iColumn
      ){
        pCol->iSorterColumn = j;
        ଭାଙ୍ଗ;
      }
    }
  }
  ଯଦି( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_ପଦସମଷ୍ଟି:
  ପଦସମଷ୍ଟି_Set_VVAProperty(p_ପଦସମଷ୍ଟି, EP_NoReduce);
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->pAggInfo==0 || p_ପଦସମଷ୍ଟି->pAggInfo==pAggInfo );
  p_ପଦସମଷ୍ଟି->pAggInfo = pAggInfo;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN ){
    p_ପଦସମଷ୍ଟି->op = TK_AGG_COLUMN;
  }
  ପ୍ରତିଜ୍ଞା( k <= SMXV(p_ପଦସମଷ୍ଟି->iAgg) );
  p_ପଦସମଷ୍ଟି->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା analyzeAggregate(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  ପ୍ରତିଜ୍ଞା( pNC->ncFlags & NC_UAggInfo );
  ପ୍ରତିଜ୍ଞା( pAggInfo->iFirstReg==0 );
  ବାଛ( p_ପଦସମଷ୍ଟି->op ){
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      Indexed_ପଦସମଷ୍ଟି *pIEpr;
      ପଦସମଷ୍ଟି tmp;
      ପ୍ରତିଜ୍ଞା( pParse->iSelfTab==0 );
      ଯଦି( (pNC->ncFlags & NC_InAggFunc)==0 ) ଭାଙ୍ଗ;
      ଯଦି( pParse->pIdxEpr==0 ) ଭାଙ୍ଗ;
      ପାଇଁ(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDataCur = pIEpr->iDataCur;
        ଯଦି( iDataCur<0 ) ଜାରି;
        ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0, p_ପଦସମଷ୍ଟି, pIEpr->p_ପଦସମଷ୍ଟି, iDataCur)==0 ) ଭାଙ୍ଗ;
      }
      ଯଦି( pIEpr==0 ) ଭାଙ୍ଗ;
      ଯଦି( NEVER(!ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି)) ) ଭାଙ୍ଗ;
      ପାଇଁ(i=0; i<pSrcList->nSrc; i++){
         ଯଦି( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           ଭାଙ୍ଗ;
         }
      }
      ଯଦି( i>=pSrcList->nSrc ) ଭାଙ୍ଗ;
      ଯଦି( NEVER(p_ପଦସମଷ୍ଟି->pAggInfo!=0) ) ଭାଙ୍ଗ; /* Resolved by outer context */
      ଯଦି( pParse->nErr ){ ଫେରନ୍ତୁ WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, ଆକାର(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      ଯଦି( pParse->nErr ){ ଫେରନ୍ତୁ WRC_Abort; }
      ପ୍ରତିଜ୍ଞା( pAggInfo->aCol!=0 );
      ପ୍ରତିଜ୍ଞା( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_ପଦସମଷ୍ଟି;
      p_ପଦସମଷ୍ଟି->pAggInfo = pAggInfo;
      p_ପଦସମଷ୍ଟି->iAgg = tmp.iAgg;
      ଫେରନ୍ତୁ WRC_Prune;
    }
    ସ୍ଥିତି TK_IF_NULL_ROW:
    ସ୍ଥିତି TK_AGG_COLUMN:
    ସ୍ଥିତି TK_COLUMN: {
      testcase( p_ପଦସମଷ୍ଟି->op==TK_AGG_COLUMN );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_COLUMN );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      ଯଦି( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        ପାଇଁ(i=0; i<pSrcList->nSrc; i++, pItem++){
          ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly|EP_Reduced) );
          ଯଦି( p_ପଦସମଷ୍ଟି->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_ପଦସମଷ୍ଟି);
            ଭାଙ୍ଗ;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      ଫେରନ୍ତୁ WRC_Continue;
    }
    ସ୍ଥିତି TK_AGG_FUNCTION: {
      ଯଦି( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_ପଦସମଷ୍ଟି->op2
       && p_ପଦସମଷ୍ଟି->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        ସଂରଚନା AggInfo_func *pItem = pAggInfo->aFunc;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        ପ୍ରତିଜ୍ଞା( mxTerm <= SMXV(i16) );
        ପାଇଁ(i=0; i<pAggInfo->nFunc; i++, pItem++){
          ଯଦି( NEVER(pItem->pFExpr==p_ପଦସମଷ୍ଟି) ) ଭାଙ୍ଗ;
          ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0, pItem->pFExpr, p_ପଦସମଷ୍ଟି, -1)==0 ){
            ଭାଙ୍ଗ;
          }
        }
        ଯଦି( i>mxTerm ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          ପ୍ରତିଜ୍ଞା( i<pAggInfo->nFunc );
        }ଅନ୍ୟଥା ଯଦି( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          ଯଦି( i>=0 ){
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg;
            ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_ପଦସମଷ୍ଟି;
            ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_UToken(p_ପଦସମଷ୍ଟି) );
            nArg = p_ପଦସମଷ୍ଟି->x.pList ? p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_ପଦସମଷ୍ଟି->u.zToken, nArg, enc, 0);
            ପ୍ରତିଜ୍ଞା( pItem->bOBUnique==0 );
            ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              ପଦସମଷ୍ଟି_List *pOBList;
              ପ୍ରତିଜ୍ଞା( nArg>0 );
              ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ->op==TK_ORDER );
              ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି->p_ବାମ) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_ପଦସମଷ୍ଟି->p_ବାମ->x.pList;
              ପ୍ରତିଜ୍ଞା( pOBList->n_ପଦସମଷ୍ଟି>0 );
              ପ୍ରତିଜ୍ଞା( pItem->bOBUnique==0 );
              ଯଦି( pOBList->n_ପଦସମଷ୍ଟି==1
               && nArg==1
               && sqlite3_ପଦସମଷ୍ଟି_Compare(0,pOBList->a[0].p_ପଦସମଷ୍ଟି,
                               p_ପଦସମଷ୍ଟି->x.pList->a[0].p_ପଦସମଷ୍ଟି,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Distinct);
              }ଅନ୍ୟଥା{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }ଅନ୍ୟଥା{
              pItem->iOBTab = -1;
            }
            ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }ଅନ୍ୟଥା{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly|EP_Reduced) );
        ପଦସମଷ୍ଟି_Set_VVAProperty(p_ପଦସମଷ୍ଟି, EP_NoReduce);
        ପ୍ରତିଜ୍ଞା( i <= SMXV(p_ପଦସମଷ୍ଟି->iAgg) );
        p_ପଦସମଷ୍ଟି->iAgg = (i16)i;
        p_ପଦସମଷ୍ଟି->pAggInfo = pAggInfo;
        ଫେରନ୍ତୁ WRC_Prune;
      }ଅନ୍ୟଥା{
        ଫେରନ୍ତୁ WRC_Continue;
      }
    }
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Analyze_Aggregates(NameContext *pNC, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  Walker w;
  w.x_ପଦସମଷ୍ଟି_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  ପ୍ରତିଜ୍ଞା( pNC->pSrcList!=0 );
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p_ପଦସମଷ୍ଟି);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
ଶୂନ୍ୟ sqlite3_ପଦସମଷ୍ଟି_Analyze_Agg_List(NameContext *pNC, ପଦସମଷ୍ଟି_List *pList){
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( pList ){
    ପାଇଁ(pItem=pList->a, i=0; i<pList->n_ପଦସମଷ୍ଟି; i++, pItem++){
      sqlite3_ପଦସମଷ୍ଟି_Analyze_Aggregates(pNC, pItem->p_ପଦସମଷ୍ଟି);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3GetTempReg(Parse *pParse){
  ଯଦି( pParse->nTempReg==0 ){
    ଫେରନ୍ତୁ ++pParse->nMem;
  }
  ଫେରନ୍ତୁ pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
ଶୂନ୍ୟ sqlite3ReleaseTempReg(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg){
  ଯଦି( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା kk;
    ପାଇଁ(kk=0; kk<pParse->nTempReg; kk++){
      ପ୍ରତିଜ୍ଞା( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    ଯଦି( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3GetTempRange(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nReg){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
  ଯଦି( nReg==1 ) ଫେରନ୍ତୁ sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  ଯଦି( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }ଅନ୍ୟଥା{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  ଫେରନ୍ତୁ i;
}
ଶୂନ୍ୟ sqlite3ReleaseTempRange(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nReg){
  ଯଦି( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    ଫେରନ୍ତୁ;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  ଯଦି( nReg>pParse->nRangeReg ){
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
ଶୂନ୍ୟ sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
ଶୂନ୍ୟ sqlite3TouchRegister(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg){
  ଯଦି( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#ଯଦି defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3FirstAvailableRegister(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iMin){
  ଅଚଳ ପଦସମଷ୍ଟି_List *pList = pParse->p_Const_ପଦସମଷ୍ଟି;
  ଯଦି( pList ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
      ଯଦି( pList->a[i].u.i_Const_ପଦସମଷ୍ଟି_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_ପଦସମଷ୍ଟି_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  ଫେରନ୍ତୁ iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3NoTempsInRange(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFirst, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iLast){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     ଫେରନ୍ତୁ 0;
  }
  ପାଇଁ(i=0; i<pParse->nTempReg; i++){
    ଯଦି( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      ଫେରନ୍ତୁ 0;
    }
  }
  ଯଦି( pParse->p_Const_ପଦସମଷ୍ଟି ){
    ପଦସମଷ୍ଟି_List *pList = pParse->p_Const_ପଦସମଷ୍ଟି;
    ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg = pList->a[i].u.i_Const_ପଦସମଷ୍ଟି_Reg;
      ଯଦି( iReg==0 ) ଜାରି;
      ଯଦି( iReg>=iFirst && iReg<=iLast ) ଫେରନ୍ତୁ 0;
    }
  }
  ଫେରନ୍ତୁ 1;
}
#endif /* SQLITE_DEBUG */
