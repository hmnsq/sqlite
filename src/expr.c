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
#അന്തര്ഭാവ "sqliteInt.h"

/* Forward declarations */
സ്ഥിതം ശൂന്യം വ്യഞ്ജകം_Code_Between(Parse*,വ്യഞ്ജകം*,പൂർണ്ണസംഖ്യ,ശൂന്യം(*)(Parse*,വ്യഞ്ജകം*,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ),പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Code_Vector(Parse *pParse, വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
അക്ഷരം sqlite3TableColumnAffinity(സ്ഥിരം Table *pTab, പൂർണ്ണസംഖ്യ iCol){
  എങ്കിൽ( iCol<0 || NEVER(iCol>=pTab->nCol) ) തിരികെ SQLITE_AFF_INTEGER;
  തിരികെ pTab->aCol[iCol].affinity;
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
അക്ഷരം sqlite3_വ്യഞ്ജകം_Affinity(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പൂർണ്ണസംഖ്യ op;
  op = p_വ്യഞ്ജകം->op;
  ആകുമ്പോൾ( 1 /* exit-by-break */ ){
    എങ്കിൽ( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_വ്യഞ്ജകം->y.pTab!=0) ){
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->y.pTab!=0 );
      തിരികെ sqlite3TableColumnAffinity(p_വ്യഞ്ജകം->y.pTab, p_വ്യഞ്ജകം->iColumn);
    }
    എങ്കിൽ( op==TK_SELECT ){
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pSelect!=0 );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pSelect->pEList!=0 );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pSelect->pEList->a[0].p_വ്യഞ്ജകം!=0 );
      തിരികെ sqlite3_വ്യഞ്ജകം_Affinity(p_വ്യഞ്ജകം->x.pSelect->pEList->a[0].p_വ്യഞ്ജകം);
    }
#ifndef SQLITE_OMIT_CAST
    എങ്കിൽ( op==TK_CAST ){
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      തിരികെ sqlite3AffinityType(p_വ്യഞ്ജകം->u.zToken, 0);
    }
#endif
    എങ്കിൽ( op==TK_SELECT_COLUMN ){
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്!=0 && വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം->p_ഇടത്) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->iColumn < p_വ്യഞ്ജകം->iTable );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->iColumn >= 0 );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->iTable==p_വ്യഞ്ജകം->p_ഇടത്->x.pSelect->pEList->n_വ്യഞ്ജകം );
      തിരികെ sqlite3_വ്യഞ്ജകം_Affinity(
          p_വ്യഞ്ജകം->p_ഇടത്->x.pSelect->pEList->a[p_വ്യഞ്ജകം->iColumn].p_വ്യഞ്ജകം
      );
    }
    എങ്കിൽ( op==TK_VECTOR
     || (op==TK_FUNCTION && p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം==SQLITE_AFF_DEFER)
    ){
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
      തിരികെ sqlite3_വ്യഞ്ജകം_Affinity(p_വ്യഞ്ജകം->x.pList->a[0].p_വ്യഞ്ജകം);
    }
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Skip|EP_IfNullRow) ){
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_COLLATE
           || p_വ്യഞ്ജകം->op==TK_IF_NULL_ROW
           || (p_വ്യഞ്ജകം->op==TK_REGISTER && p_വ്യഞ്ജകം->op2==TK_IF_NULL_ROW) );
      p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
      op = p_വ്യഞ്ജകം->op;
      തുടരുക;
    }
    എങ്കിൽ( op!=TK_REGISTER ) നിർത്തുക;
    op = p_വ്യഞ്ജകം->op2;
    എങ്കിൽ( NEVER( op==TK_REGISTER ) ) നിർത്തുക;
  }
  തിരികെ p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Data_Type(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  ആകുമ്പോൾ( p_വ്യഞ്ജകം ){
    തിരഞ്ഞെടുപ്പ്( p_വ്യഞ്ജകം->op ){
      അവസ്ഥ TK_COLLATE:
      അവസ്ഥ TK_IF_NULL_ROW:
      അവസ്ഥ TK_UPLUS:  {
        p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
        നിർത്തുക;
      }
      അവസ്ഥ TK_NULL: {
        p_വ്യഞ്ജകം = 0;
        നിർത്തുക;
      }
      അവസ്ഥ TK_STRING: {
        തിരികെ 0x02;
      }
      അവസ്ഥ TK_BLOB: {
        തിരികെ 0x04;
      }
      അവസ്ഥ TK_CONCAT: {
        തിരികെ 0x06;
      }
      അവസ്ഥ TK_VARIABLE:
      അവസ്ഥ TK_AGG_FUNCTION:
      അവസ്ഥ TK_FUNCTION: {
        തിരികെ 0x07;
      }
      അവസ്ഥ TK_COLUMN:
      അവസ്ഥ TK_AGG_COLUMN:
      അവസ്ഥ TK_SELECT:
      അവസ്ഥ TK_CAST:
      അവസ്ഥ TK_SELECT_COLUMN:
      അവസ്ഥ TK_VECTOR:  {
        പൂർണ്ണസംഖ്യ aff = sqlite3_വ്യഞ്ജകം_Affinity(p_വ്യഞ്ജകം);
        എങ്കിൽ( aff>=SQLITE_AFF_NUMERIC ) തിരികെ 0x05;
        എങ്കിൽ( aff==SQLITE_AFF_TEXT )    തിരികെ 0x06;
        തിരികെ 0x07;
      }
      അവസ്ഥ TK_CASE: {
        പൂർണ്ണസംഖ്യ res = 0;
        പൂർണ്ണസംഖ്യ ii;
        വ്യഞ്ജകം_List *pList = p_വ്യഞ്ജകം->x.pList;
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) && pList!=0 );
        പ്രതിജ്ഞാ( pList->n_വ്യഞ്ജകം > 0);
        വേണ്ടി(ii=1; ii<pList->n_വ്യഞ്ജകം; ii+=2){
          res |= sqlite3_വ്യഞ്ജകം_Data_Type(pList->a[ii].p_വ്യഞ്ജകം);
        }
        എങ്കിൽ( pList->n_വ്യഞ്ജകം % 2 ){
          res |= sqlite3_വ്യഞ്ജകം_Data_Type(pList->a[pList->n_വ്യഞ്ജകം-1].p_വ്യഞ്ജകം);
        }
        തിരികെ res;
      }
      സ്ഥിരസ്ഥിതി: {
        തിരികെ 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  തിരികെ 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,             /* Add the "COLLATE" clause to this expression */
  സ്ഥിരം Token *pCollName,  /* Name of collating sequence */
  പൂർണ്ണസംഖ്യ dequote              /* True to dequote pCollName */
){
  എങ്കിൽ( pCollName->n>0 && pParse->nErr==0 ){
    വ്യഞ്ജകം *pNew = sqlite3_വ്യഞ്ജകം_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 );
    എങ്കിൽ( pNew ){
      pNew->p_ഇടത് = p_വ്യഞ്ജകം;
      pNew->flags |= EP_Collate|EP_Skip;
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
      pNew->nHeight = p_വ്യഞ്ജകം->nHeight+1;
      sqlite3_വ്യഞ്ജകം_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_വ്യഞ്ജകം = pNew;
    }
  }
  തിരികെ p_വ്യഞ്ജകം;
}
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,          /* Add the "COLLATE" clause to this expression */
  സ്ഥിരം അക്ഷരം *zC        /* The collating sequence name */
){
  Token s;
  പ്രതിജ്ഞാ( zC!=0 );
  sqlite3TokenInit(&s, (അക്ഷരം*)zC);
  തിരികെ sqlite3_വ്യഞ്ജകം_Add_Collate_Token(pParse, p_വ്യഞ്ജകം, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Skip_Collate(വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  ആകുമ്പോൾ( p_വ്യഞ്ജകം && വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Skip) ){
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_COLLATE );
    p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
  }  
  തിരികെ p_വ്യഞ്ജകം;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  ആകുമ്പോൾ( p_വ്യഞ്ജകം && വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Skip|EP_Unlikely) ){
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Unlikely) ){
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം>0 );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_FUNCTION );
      p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->x.pList->a[0].p_വ്യഞ്ജകം;
    }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_COLLATE ){
      p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
    }അല്ലെങ്കിൽ{
      നിർത്തുക;
    }
  }  
  തിരികെ p_വ്യഞ്ജകം;
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
CollSeq *sqlite3_വ്യഞ്ജകം_Coll_Seq(Parse *pParse, സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  സ്ഥിരം വ്യഞ്ജകം *p = p_വ്യഞ്ജകം;
  ആകുമ്പോൾ( p ){
    പൂർണ്ണസംഖ്യ op = p->op;
    എങ്കിൽ( op==TK_REGISTER ) op = p->op2;
    എങ്കിൽ( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      പൂർണ്ണസംഖ്യ j;
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p) );
      പ്രതിജ്ഞാ( p->y.pTab!=0 );
      എങ്കിൽ( (j = p->iColumn)>=0 ){
        സ്ഥിരം അക്ഷരം *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      നിർത്തുക;
    }
    എങ്കിൽ( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_ഇടത്;
      തുടരുക;
    }
    എങ്കിൽ( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_വ്യഞ്ജകം==SQLITE_AFF_DEFER)
    ){
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p) );
      p = p->x.pList->a[0].p_വ്യഞ്ജകം;
      തുടരുക;
    }
    എങ്കിൽ( op==TK_COLLATE ){
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p, EP_Int_മൂല്യം) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      നിർത്തുക;
    }
    എങ്കിൽ( p->flags & EP_Collate ){
      എങ്കിൽ( p->p_ഇടത് && (p->p_ഇടത്->flags & EP_Collate)!=0 ){
        p = p->p_ഇടത്;
      }അല്ലെങ്കിൽ{
        വ്യഞ്ജകം *pNext  = p->p_വലത്;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_XList(p) || p->x.pList==0 || p->p_വലത്==0 );
        എങ്കിൽ( വ്യഞ്ജകം_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          പൂർണ്ണസംഖ്യ i;
          വേണ്ടി(i=0; i<p->x.pList->n_വ്യഞ്ജകം; i++){
            എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p->x.pList->a[i].p_വ്യഞ്ജകം, EP_Collate) ){
              pNext = p->x.pList->a[i].p_വ്യഞ്ജകം;
              നിർത്തുക;
            }
          }
        }
        p = pNext;
      }
    }അല്ലെങ്കിൽ{
      നിർത്തുക;
    }
  }
  എങ്കിൽ( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  തിരികെ pColl;
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
CollSeq *sqlite3_വ്യഞ്ജകം_NNColl_Seq(Parse *pParse, സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  CollSeq *p = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, p_വ്യഞ്ജകം);
  എങ്കിൽ( p==0 ) p = pParse->db->pDfltColl;
  പ്രതിജ്ഞാ( p!=0 );
  തിരികെ p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Coll_Seq_Match(Parse *pParse, സ്ഥിരം വ്യഞ്ജകം *pE1, സ്ഥിരം വ്യഞ്ജകം *pE2){
  CollSeq *pColl1 = sqlite3_വ്യഞ്ജകം_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_വ്യഞ്ജകം_NNColl_Seq(pParse, pE2);
  പ്രതിജ്ഞാ( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  തിരികെ pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
അക്ഷരം sqlite3CompareAffinity(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം, അക്ഷരം aff2){
  അക്ഷരം aff1 = sqlite3_വ്യഞ്ജകം_Affinity(p_വ്യഞ്ജകം);
  എങ്കിൽ( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    എങ്കിൽ( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      തിരികെ SQLITE_AFF_NUMERIC;
    }അല്ലെങ്കിൽ{
      തിരികെ SQLITE_AFF_BLOB;
    }
  }അല്ലെങ്കിൽ{
    /* One side is a column, the other is not. Use the columns affinity. */
    പ്രതിജ്ഞാ( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    തിരികെ (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
സ്ഥിതം അക്ഷരം comparisonAffinity(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  അക്ഷരം aff;
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_EQ || p_വ്യഞ്ജകം->op==TK_IN || p_വ്യഞ്ജകം->op==TK_LT ||
          p_വ്യഞ്ജകം->op==TK_GT || p_വ്യഞ്ജകം->op==TK_GE || p_വ്യഞ്ജകം->op==TK_LE ||
          p_വ്യഞ്ജകം->op==TK_NE || p_വ്യഞ്ജകം->op==TK_IS || p_വ്യഞ്ജകം->op==TK_ISNOT );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത് );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) );
  aff = sqlite3_വ്യഞ്ജകം_Affinity(p_വ്യഞ്ജകം->p_ഇടത്);
  എങ്കിൽ( p_വ്യഞ്ജകം->p_വലത് ){
    aff = sqlite3CompareAffinity(p_വ്യഞ്ജകം->p_വലത്, aff);
  }അല്ലെങ്കിൽ എങ്കിൽ( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  തിരികെ aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
പൂർണ്ണസംഖ്യ sqlite3IndexAffinityOk(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം, അക്ഷരം idx_affinity){
  അക്ഷരം aff = comparisonAffinity(p_വ്യഞ്ജകം);
  എങ്കിൽ( aff<SQLITE_AFF_TEXT ){
    തിരികെ 1;
  }
  എങ്കിൽ( aff==SQLITE_AFF_TEXT ){
    തിരികെ idx_affinity==SQLITE_AFF_TEXT;
  }
  തിരികെ sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
സ്ഥിതം u8 ദ്വിമാന_Compare_P5(
  സ്ഥിരം വ്യഞ്ജകം *pExpr1,   /* Left operand */
  സ്ഥിരം വ്യഞ്ജകം *pExpr2,   /* Right operand */
  പൂർണ്ണസംഖ്യ jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (അക്ഷരം)sqlite3_വ്യഞ്ജകം_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  തിരികെ aff;
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
CollSeq *sqlite3_ദ്വിമാന_Compare_Coll_Seq(
  Parse *pParse,
  സ്ഥിരം വ്യഞ്ജകം *p_ഇടത്,
  സ്ഥിരം വ്യഞ്ജകം *p_വലത്
){
  CollSeq *pColl;
  പ്രതിജ്ഞാ( p_ഇടത് );
  എങ്കിൽ( p_ഇടത്->flags & EP_Collate ){
    pColl = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, p_ഇടത്);
  }അല്ലെങ്കിൽ എങ്കിൽ( p_വലത് && (p_വലത്->flags & EP_Collate)!=0 ){
    pColl = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, p_വലത്);
  }അല്ലെങ്കിൽ{
    pColl = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, p_ഇടത്);
    എങ്കിൽ( !pColl ){
      pColl = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, p_വലത്);
    }
  }
  തിരികെ pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_വ്യഞ്ജകം_Compare_Coll_Seq(Parse *pParse, സ്ഥിരം വ്യഞ്ജകം *p){
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p, EP_Commuted) ){
    തിരികെ sqlite3_ദ്വിമാന_Compare_Coll_Seq(pParse, p->p_വലത്, p->p_ഇടത്);
  }അല്ലെങ്കിൽ{
    തിരികെ sqlite3_ദ്വിമാന_Compare_Coll_Seq(pParse, p->p_ഇടത്, p->p_വലത്);
  }
}

/*
** Generate code for a comparison operator.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  വ്യഞ്ജകം *p_ഇടത്,      /* The left operand */
  വ്യഞ്ജകം *p_വലത്,     /* The right operand */
  പൂർണ്ണസംഖ്യ opcode,       /* The comparison opcode */
  പൂർണ്ണസംഖ്യ in1, പൂർണ്ണസംഖ്യ in2, /* Register holding operands */
  പൂർണ്ണസംഖ്യ dest,         /* Jump here if true.  */
  പൂർണ്ണസംഖ്യ jumpIfNull,   /* If true, jump if either operand is NULL */
  പൂർണ്ണസംഖ്യ isCommuted    /* The comparison has been commuted */
){
  പൂർണ്ണസംഖ്യ p5;
  പൂർണ്ണസംഖ്യ addr;
  CollSeq *p4;

  എങ്കിൽ( pParse->nErr ) തിരികെ 0;
  എങ്കിൽ( isCommuted ){
    p4 = sqlite3_ദ്വിമാന_Compare_Coll_Seq(pParse, p_വലത്, p_ഇടത്);
  }അല്ലെങ്കിൽ{
    p4 = sqlite3_ദ്വിമാന_Compare_Coll_Seq(pParse, p_ഇടത്, p_വലത്);
  }
  p5 = ദ്വിമാന_Compare_P5(p_ഇടത്, p_വലത്, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (ശൂന്യം*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  തിരികെ addr;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Vector(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  തിരികെ sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Vector_Size(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  u8 op = p_വ്യഞ്ജകം->op;
  എങ്കിൽ( op==TK_REGISTER ) op = p_വ്യഞ്ജകം->op2;
  എങ്കിൽ( op==TK_VECTOR ){
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
    തിരികെ p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം;
  }അല്ലെങ്കിൽ എങ്കിൽ( op==TK_SELECT ){
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) );
    തിരികെ p_വ്യഞ്ജകം->x.pSelect->pEList->n_വ്യഞ്ജകം;
  }അല്ലെങ്കിൽ{
    തിരികെ 1;
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
വ്യഞ്ജകം *sqlite3VectorFieldSubexpr(വ്യഞ്ജകം *pVector, പൂർണ്ണസംഖ്യ i){
  പ്രതിജ്ഞാ( i<sqlite3_വ്യഞ്ജകം_Vector_Size(pVector) || pVector->op==TK_ERROR );
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_Vector(pVector) ){
    പ്രതിജ്ഞാ( pVector->op2==0 || pVector->op==TK_REGISTER );
    എങ്കിൽ( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(pVector) );
      തിരികെ pVector->x.pSelect->pEList->a[i].p_വ്യഞ്ജകം;
    }അല്ലെങ്കിൽ{
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(pVector) );
      തിരികെ pVector->x.pList->a[i].p_വ്യഞ്ജകം;
    }
  }
  തിരികെ pVector;
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
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  വ്യഞ്ജകം *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  പൂർണ്ണസംഖ്യ iField,          /* Which column of the vector to return */
  പൂർണ്ണസംഖ്യ nField           /* Total number of columns in the vector */
){
  വ്യഞ്ജകം *pRet;
  എങ്കിൽ( pVector->op==TK_SELECT ){
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(pVector) );
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
    എങ്കിൽ( pRet ){
      വ്യഞ്ജകം_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_ഇടത് = pVector;
    }
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( pVector->op==TK_VECTOR ){
      വ്യഞ്ജകം **ppVector;
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_വ്യഞ്ജകം;
      pVector = *ppVector;
      എങ്കിൽ( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        തിരികെ pVector;
      }
    }
    pRet = sqlite3_വ്യഞ്ജകം_Dup(pParse->db, pVector, 0);
  }
  തിരികെ pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Code_Subselect(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പൂർണ്ണസംഖ്യ reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_വ്യഞ്ജകം);
  }
#endif
  തിരികെ reg;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Vector_Register(
  Parse *pParse,                  /* Parse context */
  വ്യഞ്ജകം *pVector,                  /* Vector to extract element from */
  പൂർണ്ണസംഖ്യ iField,                     /* Field to extract from pVector */
  പൂർണ്ണസംഖ്യ regSelect,                  /* First in array of registers */
  വ്യഞ്ജകം *pTmp,                     /* Temporary space */
  വ്യഞ്ജകം **pp_വ്യഞ്ജകം,                  /* OUT: Expression element */
  പൂർണ്ണസംഖ്യ *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  പ്രതിജ്ഞാ( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  എങ്കിൽ( op==TK_REGISTER ){
    *pp_വ്യഞ്ജകം = sqlite3VectorFieldSubexpr(pVector, iField);
    തിരികെ pVector->iTable+iField;
  }
  എങ്കിൽ( op==TK_SELECT ){
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, വലുപ്പം(വ്യഞ്ജകം));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_ഇടത് = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_വ്യഞ്ജകം;
    *pp_വ്യഞ്ജകം = pTmp;
    തിരികെ regSelect+iField;
  }
  എങ്കിൽ( op==TK_VECTOR ){
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(pVector) );
    *pp_വ്യഞ്ജകം = pVector->x.pList->a[iField].p_വ്യഞ്ജകം;
    തിരികെ sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, *pp_വ്യഞ്ജകം, pRegFree);
  }
  തിരികെ 0;
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
സ്ഥിതം ശൂന്യം codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,          /* The comparison operation */
  പൂർണ്ണസംഖ്യ dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
  വ്യഞ്ജകം *p_വലത് = p_വ്യഞ്ജകം->p_വലത്;
  പൂർണ്ണസംഖ്യ n_ഇടത് = sqlite3_വ്യഞ്ജകം_Vector_Size(p_ഇടത്);
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ reg_ഇടത് = 0;
  പൂർണ്ണസംഖ്യ reg_വലത് = 0;
  u8 opx = op;
  പൂർണ്ണസംഖ്യ addrCmp = 0;
  പൂർണ്ണസംഖ്യ addrDone = sqlite3VdbeMakeLabel(pParse);
  പൂർണ്ണസംഖ്യ isCommuted = വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_Commuted);

  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_VVAProperty(p_വ്യഞ്ജകം,EP_Immutable) );
  എങ്കിൽ( pParse->nErr ) തിരികെ;
  എങ്കിൽ( n_ഇടത്!=sqlite3_വ്യഞ്ജകം_Vector_Size(p_വലത്) ){
    sqlite3_Error_സന്ദേശം(pParse, "row value misused");
    തിരികെ;
  }
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_EQ || p_വ്യഞ്ജകം->op==TK_NE
       || p_വ്യഞ്ജകം->op==TK_IS || p_വ്യഞ്ജകം->op==TK_ISNOT
       || p_വ്യഞ്ജകം->op==TK_LT || p_വ്യഞ്ജകം->op==TK_GT
       || p_വ്യഞ്ജകം->op==TK_LE || p_വ്യഞ്ജകം->op==TK_GE
  );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==op || (p_വ്യഞ്ജകം->op==TK_IS && op==TK_EQ)
            || (p_വ്യഞ്ജകം->op==TK_ISNOT && op==TK_NE) );
  പ്രതിജ്ഞാ( p5==0 || p_വ്യഞ്ജകം->op!=op );
  പ്രതിജ്ഞാ( p5==SQLITE_NULLEQ || p_വ്യഞ്ജകം->op==op );

  എങ്കിൽ( op==TK_LE ) opx = TK_LT;
  എങ്കിൽ( op==TK_GE ) opx = TK_GT;
  എങ്കിൽ( op==TK_NE ) opx = TK_EQ;

  reg_ഇടത് = വ്യഞ്ജകം_Code_Subselect(pParse, p_ഇടത്);
  reg_വലത് = വ്യഞ്ജകം_Code_Subselect(pParse, p_വലത്);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  വേണ്ടി(i=0; 1 /*Loop exits by "break"*/; i++){
    പൂർണ്ണസംഖ്യ regFree1 = 0, regFree2 = 0;
    വ്യഞ്ജകം *pL = 0, *pR = 0;
    വ്യഞ്ജകം tmp1, tmp2;
    പൂർണ്ണസംഖ്യ r1, r2;
    പ്രതിജ്ഞാ( i>=0 && i<n_ഇടത് );
    എങ്കിൽ( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = വ്യഞ്ജകം_Vector_Register(pParse, p_ഇടത്, i, reg_ഇടത്, &tmp1, &pL, &regFree1);
    r2 = വ്യഞ്ജകം_Vector_Register(pParse, p_വലത്, i, reg_വലത്, &tmp2, &pR, &regFree2);
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
    എങ്കിൽ( (opx==TK_LT || opx==TK_GT) && i<n_ഇടത്-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    എങ്കിൽ( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }അല്ലെങ്കിൽ{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    എങ്കിൽ( i==n_ഇടത്-1 ){
      നിർത്തുക;
    }
    എങ്കിൽ( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }അല്ലെങ്കിൽ{
      പ്രതിജ്ഞാ( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      എങ്കിൽ( i==n_ഇടത്-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  എങ്കിൽ( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Check_Height(Parse *pParse, പൂർണ്ണസംഖ്യ nHeight){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ mxHeight = pParse->db->aLimit[SQLITE_LIMIT_വ്യഞ്ജകം_DEPTH];
  എങ്കിൽ( nHeight>mxHeight ){
    sqlite3_Error_സന്ദേശം(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  തിരികെ rc;
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
സ്ഥിതം ശൂന്യം height_Of_വ്യഞ്ജകം(സ്ഥിരം വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ *pnHeight){
  എങ്കിൽ( p ){
    എങ്കിൽ( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
സ്ഥിതം ശൂന്യം height_Of_വ്യഞ്ജകം_List(സ്ഥിരം വ്യഞ്ജകം_List *p, പൂർണ്ണസംഖ്യ *pnHeight){
  എങ്കിൽ( p ){
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<p->n_വ്യഞ്ജകം; i++){
      height_Of_വ്യഞ്ജകം(p->a[i].p_വ്യഞ്ജകം, pnHeight);
    }
  }
}
സ്ഥിതം ശൂന്യം heightOfSelect(സ്ഥിരം Select *pSelect, പൂർണ്ണസംഖ്യ *pnHeight){
  സ്ഥിരം Select *p;
  വേണ്ടി(p=pSelect; p; p=p->pPrior){
    height_Of_വ്യഞ്ജകം(p->pWhere, pnHeight);
    height_Of_വ്യഞ്ജകം(p->pHaving, pnHeight);
    height_Of_വ്യഞ്ജകം(p->pLimit, pnHeight);
    height_Of_വ്യഞ്ജകം_List(p->pEList, pnHeight);
    height_Of_വ്യഞ്ജകം_List(p->pGroupBy, pnHeight);
    height_Of_വ്യഞ്ജകം_List(p->pOrderBy, pnHeight);
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
സ്ഥിതം ശൂന്യം വ്യഞ്ജകം_Set_Height(വ്യഞ്ജകം *p){
  പൂർണ്ണസംഖ്യ nHeight = p->p_ഇടത് ? p->p_ഇടത്->nHeight : 0;
  എങ്കിൽ( NEVER(p->p_വലത്) && p->p_വലത്->nHeight>nHeight ){
    nHeight = p->p_വലത്->nHeight;
  }
  എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }അല്ലെങ്കിൽ എങ്കിൽ( p->x.pList ){
    height_Of_വ്യഞ്ജകം_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_വ്യഞ്ജകം_List_Flags(p->x.pList);
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_Set_Height_And_Flags(Parse *pParse, വ്യഞ്ജകം *p){
  എങ്കിൽ( pParse->nErr ) തിരികെ;
  വ്യഞ്ജകം_Set_Height(p);
  sqlite3_വ്യഞ്ജകം_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
പൂർണ്ണസംഖ്യ sqlite3_Select_വ്യഞ്ജകം_Height(സ്ഥിരം Select *p){
  പൂർണ്ണസംഖ്യ nHeight = 0;
  heightOfSelect(p, &nHeight);
  തിരികെ nHeight;
}
#അല്ലെങ്കിൽ /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Set_Height_And_Flags(Parse *pParse, വ്യഞ്ജകം *p){
  എങ്കിൽ( pParse->nErr ) തിരികെ;
  എങ്കിൽ( p && വ്യഞ്ജകം_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_വ്യഞ്ജകം_List_Flags(p->x.pList);
  }
}
#define വ്യഞ്ജകം_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Set_Error_Offset(വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ iOfst){
  എങ്കിൽ( p_വ്യഞ്ജകം==0 ) തിരികെ;
  എങ്കിൽ( NEVER(വ്യഞ്ജകം_Use_WJoin(p_വ്യഞ്ജകം)) ) തിരികെ;
  p_വ്യഞ്ജകം->w.iOfst = iOfst;
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
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  പൂർണ്ണസംഖ്യ op,                 /* Expression opcode */
  സ്ഥിരം Token *pToken,    /* Token argument.  Might be NULL */
  പൂർണ്ണസംഖ്യ dequote             /* True to dequote */
){
  വ്യഞ്ജകം *pNew;
  പൂർണ്ണസംഖ്യ nExtra = pToken ? pToken->n+1 : 0;

  പ്രതിജ്ഞാ( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, വലുപ്പം(വ്യഞ്ജകം)+nExtra);
  എങ്കിൽ( pNew ){
    memset(pNew, 0, വലുപ്പം(വ്യഞ്ജകം));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    എങ്കിൽ( nExtra ){
      പ്രതിജ്ഞാ( pToken!=0 );
      pNew->u.zToken = (അക്ഷരം*)&pNew[1];
      പ്രതിജ്ഞാ( pToken->z!=0 || pToken->n==0 );
      എങ്കിൽ( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      എങ്കിൽ( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_വ്യഞ്ജകം(pNew);
      }
    }
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  തിരികെ pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  പൂർണ്ണസംഖ്യ op,                 /* Expression opcode */
  സ്ഥിരം അക്ഷരം *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  തിരികെ sqlite3_വ്യഞ്ജകം_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Int32(sqlite3 *db, പൂർണ്ണസംഖ്യ iVal){
  വ്യഞ്ജകം *pNew = sqlite3DbMallocRawNN(db, വലുപ്പം(വ്യഞ്ജകം));
  എങ്കിൽ( pNew ){
    memset(pNew, 0, വലുപ്പം(വ്യഞ്ജകം));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_മൂല്യം|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_മൂല്യം = iVal;
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  തിരികെ pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Attach_Subtrees(
  sqlite3 *db,
  വ്യഞ്ജകം *p_വേര്,
  വ്യഞ്ജകം *p_ഇടത്,
  വ്യഞ്ജകം *p_വലത്
){
  എങ്കിൽ( p_വേര്==0 ){
    പ്രതിജ്ഞാ( db->mallocFailed );
    sqlite3_വ്യഞ്ജകം_Delete(db, p_ഇടത്);
    sqlite3_വ്യഞ്ജകം_Delete(db, p_വലത്);
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വേര്) );
    പ്രതിജ്ഞാ( p_വേര്->x.pSelect==0 );
    എങ്കിൽ( p_വലത് ){
      p_വേര്->p_വലത് = p_വലത്;
      p_വേര്->flags |= EP_Propagate & p_വലത്->flags;
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
      p_വേര്->nHeight = p_വലത്->nHeight+1;
    }അല്ലെങ്കിൽ{
      p_വേര്->nHeight = 1;
#endif
    }
    എങ്കിൽ( p_ഇടത് ){
      p_വേര്->p_ഇടത് = p_ഇടത്;
      p_വേര്->flags |= EP_Propagate & p_ഇടത്->flags;
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
      എങ്കിൽ( p_ഇടത്->nHeight>=p_വേര്->nHeight ){
        p_വേര്->nHeight = p_ഇടത്->nHeight+1;
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
വ്യഞ്ജകം *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  പൂർണ്ണസംഖ്യ op,                 /* Expression opcode */
  വ്യഞ്ജകം *p_ഇടത്,            /* Left operand */
  വ്യഞ്ജകം *p_വലത്            /* Right operand */
){
  വ്യഞ്ജകം *p;
  p = sqlite3DbMallocRawNN(pParse->db, വലുപ്പം(വ്യഞ്ജകം));
  എങ്കിൽ( p ){
    memset(p, 0, വലുപ്പം(വ്യഞ്ജകം));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_വ്യഞ്ജകം_Attach_Subtrees(pParse->db, p, p_ഇടത്, p_വലത്);
    sqlite3_വ്യഞ്ജകം_Check_Height(pParse, p->nHeight);
  }അല്ലെങ്കിൽ{
    sqlite3_വ്യഞ്ജകം_Delete(pParse->db, p_ഇടത്);
    sqlite3_വ്യഞ്ജകം_Delete(pParse->db, p_വലത്);
  }
  തിരികെ p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
ശൂന്യം sqlite3PExprAddSelect(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, Select *pSelect){
  എങ്കിൽ( p_വ്യഞ്ജകം ){
    p_വ്യഞ്ജകം->x.pSelect = pSelect;
    വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_xIsSelect|EP_Subquery);
    sqlite3_വ്യഞ്ജകം_Set_Height_And_Flags(pParse, p_വ്യഞ്ജകം);
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( pParse->db->mallocFailed );
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
Select *sqlite3_വ്യഞ്ജകം_List_To_മൂല്യങ്ങൾ(Parse *pParse, പൂർണ്ണസംഖ്യ nElem, വ്യഞ്ജകം_List *pEList){
  പൂർണ്ണസംഖ്യ ii;
  Select *pRet = 0;
  പ്രതിജ്ഞാ( nElem>1 );
  വേണ്ടി(ii=0; ii<pEList->n_വ്യഞ്ജകം; ii++){
    Select *pSel;
    വ്യഞ്ജകം *p_വ്യഞ്ജകം = pEList->a[ii].p_വ്യഞ്ജകം;
    പൂർണ്ണസംഖ്യ n_വ്യഞ്ജകം_Elem;
    എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_VECTOR ){
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
      n_വ്യഞ്ജകം_Elem = p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം;
    }അല്ലെങ്കിൽ{
      n_വ്യഞ്ജകം_Elem = 1;
    }
    എങ്കിൽ( n_വ്യഞ്ജകം_Elem!=nElem ){
      sqlite3_Error_സന്ദേശം(pParse, "IN(...) element has %d term%s - expected %d",
          n_വ്യഞ്ജകം_Elem, n_വ്യഞ്ജകം_Elem>1?"s":"", nElem
      );
      നിർത്തുക;
    }
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
    pSel = sqlite3SelectNew(pParse, p_വ്യഞ്ജകം->x.pList, 0, 0, 0, 0, 0, SF_മൂല്യങ്ങൾ,0);
    p_വ്യഞ്ജകം->x.pList = 0;
    എങ്കിൽ( pSel ){
      എങ്കിൽ( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  എങ്കിൽ( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_മൂല്യം;
  }
  sqlite3_വ്യഞ്ജകം_List_Delete(pParse->db, pEList);
  തിരികെ pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_And(Parse *pParse, വ്യഞ്ജകം *p_ഇടത്, വ്യഞ്ജകം *p_വലത്){
  sqlite3 *db = pParse->db;
  എങ്കിൽ( p_ഇടത്==0  ){
    തിരികെ p_വലത്;
  }അല്ലെങ്കിൽ എങ്കിൽ( p_വലത്==0 ){
    തിരികെ p_ഇടത്;
  }അല്ലെങ്കിൽ{
    u32 f = p_ഇടത്->flags | p_വലത്->flags;
    എങ്കിൽ( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      വ്യഞ്ജകം *p;
      sqlite3_വ്യഞ്ജകം_Deferred_Delete(pParse, p_ഇടത്);
      sqlite3_വ്യഞ്ജകം_Deferred_Delete(pParse, p_വലത്);
      p = sqlite3_വ്യഞ്ജകം_Int32(db, 0);
      എങ്കിൽ( p ) p->op2 = 1;
      തിരികെ p;
    }അല്ലെങ്കിൽ{
      തിരികെ sqlite3PExpr(pParse, TK_AND, p_ഇടത്, p_വലത്);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Function(
  Parse *pParse,        /* Parsing context */
  വ്യഞ്ജകം_List *pList,      /* Argument list */
  സ്ഥിരം Token *pToken,  /* Name of the function */
  പൂർണ്ണസംഖ്യ eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  വ്യഞ്ജകം *pNew;
  sqlite3 *db = pParse->db;
  പ്രതിജ്ഞാ( pToken );
  pNew = sqlite3_വ്യഞ്ജകം_Alloc(db, TK_FUNCTION, pToken, 1);
  എങ്കിൽ( pNew==0 ){
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    തിരികെ 0;
  }
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (പൂർണ്ണസംഖ്യ)(pToken->z - pParse->zTail);
  എങ്കിൽ( pList
   && pList->n_വ്യഞ്ജകം > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_സന്ദേശം(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  വ്യഞ്ജകം_Set_Property(pNew, EP_HasFunc);
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(pNew) );
  sqlite3_വ്യഞ്ജകം_Set_Height_And_Flags(pParse, pNew);
  എങ്കിൽ( eDistinct==SF_Distinct ) വ്യഞ്ജകം_Set_Property(pNew, EP_Distinct);
  തിരികെ pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Order_By_Aggregate_Error(Parse *pParse, വ്യഞ്ജകം *p){
  sqlite3_Error_സന്ദേശം(pParse,
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,          /* The function call to which ORDER BY is to be added */
  വ്യഞ്ജകം_List *pOrderBy    /* The ORDER BY clause to add */
){
  വ്യഞ്ജകം *pOB;
  sqlite3 *db = pParse->db;
  എങ്കിൽ( NEVER(pOrderBy==0) ){
    പ്രതിജ്ഞാ( db->mallocFailed );
    തിരികെ;
  }
  എങ്കിൽ( p_വ്യഞ്ജകം==0 ){
    പ്രതിജ്ഞാ( db->mallocFailed );
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pOrderBy);
    തിരികെ;
  }
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_FUNCTION );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്==0 );
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
  എങ്കിൽ( p_വ്യഞ്ജകം->x.pList==0 || NEVER(p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_വ്യഞ്ജകം_List_Delete_Generic, pOrderBy);
    തിരികെ;
  }
  എങ്കിൽ( IsWindowFunc(p_വ്യഞ്ജകം) ){
    sqlite3_വ്യഞ്ജകം_Order_By_Aggregate_Error(pParse, p_വ്യഞ്ജകം);
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pOrderBy);
    തിരികെ;
  }
  എങ്കിൽ( pOrderBy->n_വ്യഞ്ജകം>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_സന്ദേശം(pParse, "too many terms in ORDER BY clause");
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pOrderBy);
    തിരികെ;
  }

  pOB = sqlite3_വ്യഞ്ജകം_Alloc(db, TK_ORDER, 0, 0);
  എങ്കിൽ( pOB==0 ){
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pOrderBy);
    തിരികെ;
  }
  pOB->x.pList = pOrderBy;
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(pOB) );
  p_വ്യഞ്ജകം->p_ഇടത് = pOB;
  വ്യഞ്ജകം_Set_Property(pOB, EP_FullSize);
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം,     /* The function invocation */
  സ്ഥിരം FuncDef *pDef    /* The function being invoked */
){
  പ്രതിജ്ഞാ( !IN_RENAME_OBJECT );
  പ്രതിജ്ഞാ( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    എങ്കിൽ( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_സന്ദേശം(pParse, "unsafe use of %#T()", p_വ്യഞ്ജകം);
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_Assign_Var_സംഖ്യ(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, u32 n){
  sqlite3 *db = pParse->db;
  സ്ഥിരം അക്ഷരം *z;
  ynVar x;

  എങ്കിൽ( p_വ്യഞ്ജകം==0 ) തിരികെ;
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം|EP_Reduced|EP_TokenOnly) );
  z = p_വ്യഞ്ജകം->u.zToken;
  പ്രതിജ്ഞാ( z!=0 );
  പ്രതിജ്ഞാ( z[0]!=0 );
  പ്രതിജ്ഞാ( n==(u32)sqlite3Strlen30(z) );
  എങ്കിൽ( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    പ്രതിജ്ഞാ( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ doAdd = 0;
    എങ്കിൽ( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      പൂർണ്ണസംഖ്യ bOk;
      എങ്കിൽ( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }അല്ലെങ്കിൽ{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_സംഖ്യ]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_സംഖ്യ] );
      എങ്കിൽ( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_സംഖ്യ] ){
        sqlite3_Error_സന്ദേശം(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_സംഖ്യ]);
        sqlite3_Record_Error_Offset_Of_വ്യഞ്ജകം(pParse->db, p_വ്യഞ്ജകം);
        തിരികെ;
      }
      x = (ynVar)i;
      എങ്കിൽ( x>pParse->nVar ){
        pParse->nVar = (പൂർണ്ണസംഖ്യ)x;
        doAdd = 1;
      }അല്ലെങ്കിൽ എങ്കിൽ( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3_VList_അക്കം_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }അല്ലെങ്കിൽ{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_അക്കം(pParse->pVList, z, n);
      എങ്കിൽ( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    എങ്കിൽ( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      എങ്കിൽ( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_വ്യഞ്ജകം->iColumn = x;
  എങ്കിൽ( x>db->aLimit[SQLITE_LIMIT_VARIABLE_സംഖ്യ] ){
    sqlite3_Error_സന്ദേശം(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_വ്യഞ്ജകം(pParse->db, p_വ്യഞ്ജകം);
  }
}

/*
** Recursively delete an expression tree.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം sqlite3_വ്യഞ്ജകം_Delete_NN(sqlite3 *db, വ്യഞ്ജകം *p){
  പ്രതിജ്ഞാ( p!=0 );
  പ്രതിജ്ഞാ( db!=0 );
വ്യഞ്ജകം_Delete_Restart:
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_UValue(p) || p->u.i_മൂല്യം>=0 );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_YWin(p) || !വ്യഞ്ജകം_Use_YSub(p) );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  പ്രതിജ്ഞാ( p->op!=TK_FUNCTION || !വ്യഞ്ജകം_Use_YSub(p) );
#ifdef SQLITE_DEBUG
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p, EP_Leaf) && !വ്യഞ്ജകം_Has_Property(p, EP_TokenOnly) ){
    പ്രതിജ്ഞാ( p->p_ഇടത്==0 );
    പ്രതിജ്ഞാ( p->p_വലത്==0 );
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_XSelect(p) || p->x.pSelect==0 );
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_XList(p) || p->x.pList==0 );
  }
#endif
  എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    പ്രതിജ്ഞാ( (വ്യഞ്ജകം_Use_XList(p) && p->x.pList==0) || p->p_വലത്==0 );
    എങ്കിൽ( p->p_വലത് ){
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p, EP_WinFunc) );
      sqlite3_വ്യഞ്ജകം_Delete_NN(db, p->p_വലത്);
    }അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p) ){
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }അല്ലെങ്കിൽ{
      sqlite3_വ്യഞ്ജകം_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    എങ്കിൽ( p->p_ഇടത് && p->op!=TK_SELECT_COLUMN ){
      വ്യഞ്ജകം *p_ഇടത് = p->p_ഇടത്;
      എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p, EP_Static)
       && !വ്യഞ്ജകം_Has_Property(p_ഇടത്, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_ഇടത്;
        പോകുക വ്യഞ്ജകം_Delete_Restart;
      }അല്ലെങ്കിൽ{
        sqlite3_വ്യഞ്ജകം_Delete_NN(db, p_ഇടത്);
      }
    }
  }
  എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
ശൂന്യം sqlite3_വ്യഞ്ജകം_Delete(sqlite3 *db, വ്യഞ്ജകം *p){
  എങ്കിൽ( p ) sqlite3_വ്യഞ്ജകം_Delete_NN(db, p);
}
ശൂന്യം sqlite3_വ്യഞ്ജകം_Delete_Generic(sqlite3 *db, ശൂന്യം *p){
  എങ്കിൽ( ALWAYS(p) ) sqlite3_വ്യഞ്ജകം_Delete_NN(db, (വ്യഞ്ജകം*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
ശൂന്യം sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  എങ്കിൽ( p==0 ){
    /* Nothing to clear */
  }അല്ലെങ്കിൽ എങ്കിൽ( p->pOn ){
    sqlite3_വ്യഞ്ജകം_Delete_NN(db, p->pOn);
  }അല്ലെങ്കിൽ എങ്കിൽ( p->pUsing ){
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Deferred_Delete(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  തിരികെ 0==sqlite3ParserAddCleanup(pParse, sqlite3_വ്യഞ്ജകം_Delete_Generic, p_വ്യഞ്ജകം);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Unmap_And_Delete(Parse *pParse, വ്യഞ്ജകം *p){
  എങ്കിൽ( p ){
    എങ്കിൽ( IN_RENAME_OBJECT ){
      sqlite3_Rename_വ്യഞ്ജകം_Unmap(pParse, p);
    }
    sqlite3_വ്യഞ്ജകം_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Struct_Size(സ്ഥിരം വ്യഞ്ജകം *p){
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p, EP_TokenOnly) ) തിരികെ വ്യഞ്ജകം_TOKENONLYSIZE;
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p, EP_Reduced) ) തിരികെ വ്യഞ്ജകം_REDUCEDSIZE;
  തിരികെ വ്യഞ്ജകം_FULLSIZE;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ duped_വ്യഞ്ജകം_Struct_Size(സ്ഥിരം വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ flags){
  പൂർണ്ണസംഖ്യ nSize;
  പ്രതിജ്ഞാ( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_FULLSIZE<=0xfff );
  പ്രതിജ്ഞാ( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  എങ്കിൽ( 0==flags || വ്യഞ്ജകം_Has_Property(p, EP_FullSize) ){
    nSize = വ്യഞ്ജകം_FULLSIZE;
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p, EP_OuterON) );
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_VVAProperty(p, EP_NoReduce) );
    എങ്കിൽ( p->p_ഇടത് || p->x.pList ){
      nSize = വ്യഞ്ജകം_REDUCEDSIZE | EP_Reduced;
    }അല്ലെങ്കിൽ{
      പ്രതിജ്ഞാ( p->p_വലത്==0 );
      nSize = വ്യഞ്ജകം_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  തിരികെ nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ duped_വ്യഞ്ജകം_കണ്ണി_Size(സ്ഥിരം വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ flags){
  പൂർണ്ണസംഖ്യ nByte = duped_വ്യഞ്ജകം_Struct_Size(p, flags) & 0xfff;
  എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p, EP_Int_മൂല്യം) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  തിരികെ ROUND8(nByte);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ duped_വ്യഞ്ജകം_Size(സ്ഥിരം വ്യഞ്ജകം *p){
  പൂർണ്ണസംഖ്യ nByte;
  പ്രതിജ്ഞാ( p!=0 );
  nByte = duped_വ്യഞ്ജകം_കണ്ണി_Size(p, EXPRDUP_REDUCE);
  എങ്കിൽ( p->p_ഇടത് ) nByte += duped_വ്യഞ്ജകം_Size(p->p_ഇടത്);
  എങ്കിൽ( p->p_വലത് ) nByte += duped_വ്യഞ്ജകം_Size(p->p_വലത്);
  പ്രതിജ്ഞാ( nByte==ROUND8(nByte) );
  തിരികെ nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
തരനിർവചനം ഘടന EdupBuf EdupBuf;
ഘടന EdupBuf {
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
സ്ഥിതം വ്യഞ്ജകം *വ്യഞ്ജകം_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  സ്ഥിരം വ്യഞ്ജകം *p,        /* Expr tree to be duplicated */
  പൂർണ്ണസംഖ്യ dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  വ്യഞ്ജകം *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  പൂർണ്ണസംഖ്യ nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  പ്രതിജ്ഞാ( db!=0 );
  പ്രതിജ്ഞാ( p );
  പ്രതിജ്ഞാ( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  പ്രതിജ്ഞാ( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  എങ്കിൽ( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    പ്രതിജ്ഞാ( sEdupBuf.zAlloc!=0 );
    പ്രതിജ്ഞാ( dupFlags==EXPRDUP_REDUCE );
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ nAlloc;
    എങ്കിൽ( dupFlags ){
      nAlloc = duped_വ്യഞ്ജകം_Size(p);
    }അല്ലെങ്കിൽ എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p, EP_Int_മൂല്യം) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(വ്യഞ്ജകം_FULLSIZE + nToken);
    }അല്ലെങ്കിൽ{
      nToken = 0;
      nAlloc = ROUND8(വ്യഞ്ജകം_FULLSIZE);
    }
    പ്രതിജ്ഞാ( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (വ്യഞ്ജകം *)sEdupBuf.zAlloc;
  പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT(pNew) );

  എങ്കിൽ( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    സ്ഥിരം unsigned nStructSize = duped_വ്യഞ്ജകം_Struct_Size(p, dupFlags);
    പൂർണ്ണസംഖ്യ nNewSize = nStructSize & 0xfff;
    എങ്കിൽ( nToken<0 ){
      എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p, EP_Int_മൂല്യം) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }അല്ലെങ്കിൽ{
        nToken = 0;
      }
    }
    എങ്കിൽ( dupFlags ){
      പ്രതിജ്ഞാ( (പൂർണ്ണസംഖ്യ)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }അല്ലെങ്കിൽ{
      u32 nSize = (u32)വ്യഞ്ജകം_Struct_Size(p);
      പ്രതിജ്ഞാ( (പൂർണ്ണസംഖ്യ)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (പൂർണ്ണസംഖ്യ)വ്യഞ്ജകം_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      എങ്കിൽ( nSize<വ്യഞ്ജകം_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, വ്യഞ്ജകം_FULLSIZE-nSize);
      }
      nNewSize = വ്യഞ്ജകം_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    വ്യഞ്ജകം_Clear_VVAProperties(pNew);
    എങ്കിൽ( dupFlags ){
      വ്യഞ്ജകം_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    പ്രതിജ്ഞാ( nToken>=0 );
    എങ്കിൽ( nToken>0 ){
      അക്ഷരം *zToken = pNew->u.zToken = (അക്ഷരം*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    എങ്കിൽ( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }അല്ലെങ്കിൽ{
        pNew->x.pList = sqlite3_വ്യഞ്ജകം_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      എങ്കിൽ( dupFlags ){
        എങ്കിൽ( p->op==TK_SELECT_COLUMN ){
          pNew->p_ഇടത് = p->p_ഇടത്;
          പ്രതിജ്ഞാ( p->p_വലത്==0 
               || p->p_വലത്==p->p_ഇടത്
               || വ്യഞ്ജകം_Has_Property(p->p_ഇടത്, EP_Subquery) );
        }അല്ലെങ്കിൽ{
          pNew->p_ഇടത് = p->p_ഇടത് ?
                      വ്യഞ്ജകം_Dup(db, p->p_ഇടത്, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_വലത് = p->p_വലത് ?
                       വ്യഞ്ജകം_Dup(db, p->p_വലത്, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( p->op==TK_SELECT_COLUMN ){
          pNew->p_ഇടത് = p->p_ഇടത്;
          പ്രതിജ്ഞാ( p->p_വലത്==0 
               || p->p_വലത്==p->p_ഇടത്
               || വ്യഞ്ജകം_Has_Property(p->p_ഇടത്, EP_Subquery) );
        }അല്ലെങ്കിൽ{
          pNew->p_ഇടത് = sqlite3_വ്യഞ്ജകം_Dup(db, p->p_ഇടത്, 0);
        }
        pNew->p_വലത് = sqlite3_വ്യഞ്ജകം_Dup(db, p->p_വലത്, 0);
      }
    }
  }
  എങ്കിൽ( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, വലുപ്പം(sEdupBuf));
  പ്രതിജ്ഞാ( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  തിരികെ pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  എങ്കിൽ( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    എങ്കിൽ( pRet ){
      പൂർണ്ണസംഖ്യ i;
      pRet->nCte = p->nCte;
      വേണ്ടി(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_വ്യഞ്ജകം_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  തിരികെ pRet;
}
#അല്ലെങ്കിൽ
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ gatherSelectWindowsCallback(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_FUNCTION && വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_വ്യഞ്ജകം->y.pWin;
    പ്രതിജ്ഞാ( pWin );
    പ്രതിജ്ഞാ( IsWindowFunc(p_വ്യഞ്ജകം) );
    പ്രതിജ്ഞാ( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  തിരികെ WRC_Continue;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  തിരികെ p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
സ്ഥിതം ശൂന്യം gatherSelectWindows(Select *p){
  Walker w;
  w.x_വ്യഞ്ജകം_Callback = gatherSelectWindowsCallback;
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
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Dup(sqlite3 *db, സ്ഥിരം വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ flags){
  പ്രതിജ്ഞാ( flags==0 || flags==EXPRDUP_REDUCE );
  തിരികെ p ? വ്യഞ്ജകം_Dup(db, p, flags, 0) : 0;
}
വ്യഞ്ജകം_List *sqlite3_വ്യഞ്ജകം_List_Dup(sqlite3 *db, സ്ഥിരം വ്യഞ്ജകം_List *p, പൂർണ്ണസംഖ്യ flags){
  വ്യഞ്ജകം_List *pNew;
  ഘടന വ്യഞ്ജകം_List_item *pItem;
  സ്ഥിരം ഘടന വ്യഞ്ജകം_List_item *pOldItem;
  പൂർണ്ണസംഖ്യ i;
  വ്യഞ്ജകം *pPriorSelectColOld = 0;
  വ്യഞ്ജകം *pPriorSelectColNew = 0;
  പ്രതിജ്ഞാ( db!=0 );
  എങ്കിൽ( p==0 ) തിരികെ 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  എങ്കിൽ( pNew==0 ) തിരികെ 0;
  pNew->n_വ്യഞ്ജകം = p->n_വ്യഞ്ജകം;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  വേണ്ടി(i=0; i<p->n_വ്യഞ്ജകം; i++, pItem++, pOldItem++){
    വ്യഞ്ജകം *p_Old_വ്യഞ്ജകം = pOldItem->p_വ്യഞ്ജകം;
    വ്യഞ്ജകം *p_New_വ്യഞ്ജകം;
    pItem->p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Dup(db, p_Old_വ്യഞ്ജകം, flags);
    എങ്കിൽ( p_Old_വ്യഞ്ജകം
     && p_Old_വ്യഞ്ജകം->op==TK_SELECT_COLUMN
     && (p_New_വ്യഞ്ജകം = pItem->p_വ്യഞ്ജകം)!=0
    ){
      എങ്കിൽ( p_New_വ്യഞ്ജകം->p_വലത് ){
        pPriorSelectColOld = p_Old_വ്യഞ്ജകം->p_വലത്;
        pPriorSelectColNew = p_New_വ്യഞ്ജകം->p_വലത്;
        p_New_വ്യഞ്ജകം->p_ഇടത് = p_New_വ്യഞ്ജകം->p_വലത്;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( p_Old_വ്യഞ്ജകം->p_ഇടത്!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_വ്യഞ്ജകം->p_ഇടത്;
          pPriorSelectColNew = sqlite3_വ്യഞ്ജകം_Dup(db, pPriorSelectColOld, flags);
          p_New_വ്യഞ്ജകം->p_വലത് = pPriorSelectColNew;
        }
        p_New_വ്യഞ്ജകം->p_ഇടത് = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  തിരികെ pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#എങ്കിൽ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, സ്ഥിരം SrcList *p, പൂർണ്ണസംഖ്യ flags){
  SrcList *pNew;
  പൂർണ്ണസംഖ്യ i;
  പ്രതിജ്ഞാ( db!=0 );
  എങ്കിൽ( p==0 ) തിരികെ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  എങ്കിൽ( pNew==0 ) തിരികെ 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  വേണ്ടി(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    സ്ഥിരം SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    എങ്കിൽ( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, വലുപ്പം(Subquery));
      എങ്കിൽ( pNewSubq==0 ){
        പ്രതിജ്ഞാ( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }അല്ലെങ്കിൽ{
        memcpy(pNewSubq, pOldItem->u4.pSubq, വലുപ്പം(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        എങ്കിൽ( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }അല്ലെങ്കിൽ എങ്കിൽ( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }അല്ലെങ്കിൽ{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    എങ്കിൽ( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }അല്ലെങ്കിൽ എങ്കിൽ( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_വ്യഞ്ജകം_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }അല്ലെങ്കിൽ{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    എങ്കിൽ( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    എങ്കിൽ( pTab ){
      pTab->nTabRef++;
    }
    എങ്കിൽ( pOldItem->fg.isUsing ){
      പ്രതിജ്ഞാ( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }അല്ലെങ്കിൽ{
      pNewItem->u3.pOn = sqlite3_വ്യഞ്ജകം_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  തിരികെ pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, സ്ഥിരം IdList *p){
  IdList *pNew;
  പൂർണ്ണസംഖ്യ i;
  പ്രതിജ്ഞാ( db!=0 );
  എങ്കിൽ( p==0 ) തിരികെ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  എങ്കിൽ( pNew==0 ) തിരികെ 0;
  pNew->nId = p->nId;
  വേണ്ടി(i=0; i<p->nId; i++){
    ഘടന IdList_item *pNewItem = &pNew->a[i];
    സ്ഥിരം ഘടന IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  തിരികെ pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, സ്ഥിരം Select *pDup, പൂർണ്ണസംഖ്യ flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  സ്ഥിരം Select *p;

  പ്രതിജ്ഞാ( db!=0 );
  വേണ്ടി(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, വലുപ്പം(*p) );
    എങ്കിൽ( pNew==0 ) നിർത്തുക;
    pNew->pEList = sqlite3_വ്യഞ്ജകം_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_വ്യഞ്ജകം_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_വ്യഞ്ജകം_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_വ്യഞ്ജകം_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_വ്യഞ്ജകം_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_വ്യഞ്ജകം_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    എങ്കിൽ( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    എങ്കിൽ( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      നിർത്തുക;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  തിരികെ pRet;
}
#അല്ലെങ്കിൽ
Select *sqlite3SelectDup(sqlite3 *db, സ്ഥിരം Select *p, പൂർണ്ണസംഖ്യ flags){
  പ്രതിജ്ഞാ( p==0 );
  തിരികെ 0;
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
സ്ഥിതം സ്ഥിരം ഘടന വ്യഞ്ജകം_List_item zeroItem = {0};
SQLITE_NOINLINE വ്യഞ്ജകം_List *sqlite3_വ്യഞ്ജകം_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം             /* Expression to be appended. Might be NULL */
){
  ഘടന വ്യഞ്ജകം_List_item *pItem;
  വ്യഞ്ജകം_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  എങ്കിൽ( pList==0 ){
    sqlite3_വ്യഞ്ജകം_Delete(db, p_വ്യഞ്ജകം);
    തിരികെ 0;
  }
  pList->nAlloc = 4;
  pList->n_വ്യഞ്ജകം = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_വ്യഞ്ജകം = p_വ്യഞ്ജകം;
  തിരികെ pList;
}
SQLITE_NOINLINE വ്യഞ്ജകം_List *sqlite3_വ്യഞ്ജകം_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  വ്യഞ്ജകം_List *pList,        /* List to which to append. Might be NULL */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം             /* Expression to be appended. Might be NULL */
){
  ഘടന വ്യഞ്ജകം_List_item *pItem;
  വ്യഞ്ജകം_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  എങ്കിൽ( pNew==0 ){
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pList);
    sqlite3_വ്യഞ്ജകം_Delete(db, p_വ്യഞ്ജകം);
    തിരികെ 0;
  }അല്ലെങ്കിൽ{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_വ്യഞ്ജകം++];
  *pItem = zeroItem;
  pItem->p_വ്യഞ്ജകം = p_വ്യഞ്ജകം;
  തിരികെ pList;
}
വ്യഞ്ജകം_List *sqlite3_വ്യഞ്ജകം_List_Append(
  Parse *pParse,          /* Parsing context */
  വ്യഞ്ജകം_List *pList,        /* List to which to append. Might be NULL */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം             /* Expression to be appended. Might be NULL */
){
  ഘടന വ്യഞ്ജകം_List_item *pItem;
  എങ്കിൽ( pList==0 ){
    തിരികെ sqlite3_വ്യഞ്ജകം_List_Append_New(pParse->db,p_വ്യഞ്ജകം);
  }
  എങ്കിൽ( pList->nAlloc<pList->n_വ്യഞ്ജകം+1 ){
    തിരികെ sqlite3_വ്യഞ്ജകം_List_Append_Grow(pParse->db,pList,p_വ്യഞ്ജകം);
  }
  pItem = &pList->a[pList->n_വ്യഞ്ജകം++];
  *pItem = zeroItem;
  pItem->p_വ്യഞ്ജകം = p_വ്യഞ്ജകം;
  തിരികെ pList;
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
വ്യഞ്ജകം_List *sqlite3_വ്യഞ്ജകം_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  വ്യഞ്ജകം_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  പൂർണ്ണസംഖ്യ n;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ iFirst = pList ? pList->n_വ്യഞ്ജകം : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  എങ്കിൽ( NEVER(pColumns==0) ) പോകുക vector_append_error;
  എങ്കിൽ( p_വ്യഞ്ജകം==0 ) പോകുക vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  എങ്കിൽ( p_വ്യഞ്ജകം->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം)) ){
    sqlite3_Error_സന്ദേശം(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    പോകുക vector_append_error;
  }

  വേണ്ടി(i=0; i<pColumns->nId; i++){
    വ്യഞ്ജകം *p_Sub_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_For_Vector_Field(pParse, p_വ്യഞ്ജകം, i, pColumns->nId);
    പ്രതിജ്ഞാ( p_Sub_വ്യഞ്ജകം!=0 || db->mallocFailed );
    എങ്കിൽ( p_Sub_വ്യഞ്ജകം==0 ) തുടരുക;
    pList = sqlite3_വ്യഞ്ജകം_List_Append(pParse, pList, p_Sub_വ്യഞ്ജകം);
    എങ്കിൽ( pList ){
      പ്രതിജ്ഞാ( pList->n_വ്യഞ്ജകം==iFirst+i+1 );
      pList->a[pList->n_വ്യഞ്ജകം-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  എങ്കിൽ( !db->mallocFailed && p_വ്യഞ്ജകം->op==TK_SELECT && ALWAYS(pList!=0) ){
    വ്യഞ്ജകം *pFirst = pList->a[iFirst].p_വ്യഞ്ജകം;
    പ്രതിജ്ഞാ( pFirst!=0 );
    പ്രതിജ്ഞാ( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_വലത് = p_വ്യഞ്ജകം;
    p_വ്യഞ്ജകം = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_വ്യഞ്ജകം_Unmap_And_Delete(pParse, p_വ്യഞ്ജകം);
  sqlite3IdListDelete(db, pColumns);
  തിരികെ pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_List_Set_Sort_Order(വ്യഞ്ജകം_List *p, പൂർണ്ണസംഖ്യ iSortOrder, പൂർണ്ണസംഖ്യ eNulls){
  ഘടന വ്യഞ്ജകം_List_item *pItem;
  എങ്കിൽ( p==0 ) തിരികെ;
  പ്രതിജ്ഞാ( p->n_വ്യഞ്ജകം>0 );

  പ്രതിജ്ഞാ( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  പ്രതിജ്ഞാ( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  പ്രതിജ്ഞാ( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_വ്യഞ്ജകം-1];
  പ്രതിജ്ഞാ( pItem->fg.bNulls==0 );
  എങ്കിൽ( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  എങ്കിൽ( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    എങ്കിൽ( iSortOrder!=eNulls ){
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  വ്യഞ്ജകം_List *pList,        /* List to which to add the span. */
  സ്ഥിരം Token *pName,     /* Name to be added */
  പൂർണ്ണസംഖ്യ dequote             /* True to cause the name to be dequoted */
){
  പ്രതിജ്ഞാ( pList!=0 || pParse->db->mallocFailed!=0 );
  പ്രതിജ്ഞാ( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  എങ്കിൽ( pList ){
    ഘടന വ്യഞ്ജകം_List_item *pItem;
    പ്രതിജ്ഞാ( pList->n_വ്യഞ്ജകം>0 );
    pItem = &pList->a[pList->n_വ്യഞ്ജകം-1];
    പ്രതിജ്ഞാ( pItem->zEName==0 );
    പ്രതിജ്ഞാ( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    എങ്കിൽ( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      എങ്കിൽ( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (സ്ഥിരം ശൂന്യം*)pItem->zEName, pName);
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  വ്യഞ്ജകം_List *pList,        /* List to which to add the span. */
  സ്ഥിരം അക്ഷരം *zStart,     /* Start of the span */
  സ്ഥിരം അക്ഷരം *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  പ്രതിജ്ഞാ( pList!=0 || db->mallocFailed!=0 );
  എങ്കിൽ( pList ){
    ഘടന വ്യഞ്ജകം_List_item *pItem = &pList->a[pList->n_വ്യഞ്ജകം-1];
    പ്രതിജ്ഞാ( pList->n_വ്യഞ്ജകം>0 );
    എങ്കിൽ( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_List_Check_Length(
  Parse *pParse,
  വ്യഞ്ജകം_List *pEList,
  സ്ഥിരം അക്ഷരം *zObject
){
  പൂർണ്ണസംഖ്യ mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_വ്യഞ്ജകം==mx );
  testcase( pEList && pEList->n_വ്യഞ്ജകം==mx+1 );
  എങ്കിൽ( pEList && pEList->n_വ്യഞ്ജകം>mx ){
    sqlite3_Error_സന്ദേശം(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം വ്യഞ്ജകം_List_Delete_NN(sqlite3 *db, വ്യഞ്ജകം_List *pList){
  പൂർണ്ണസംഖ്യ i = pList->n_വ്യഞ്ജകം;
  ഘടന വ്യഞ്ജകം_List_item *pItem =  pList->a;
  പ്രതിജ്ഞാ( pList->n_വ്യഞ്ജകം>0 );
  പ്രതിജ്ഞാ( db!=0 );
  ചെയ്യുക{
    sqlite3_വ്യഞ്ജകം_Delete(db, pItem->p_വ്യഞ്ജകം);
    എങ്കിൽ( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }ആകുമ്പോൾ( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
ശൂന്യം sqlite3_വ്യഞ്ജകം_List_Delete(sqlite3 *db, വ്യഞ്ജകം_List *pList){
  എങ്കിൽ( pList ) വ്യഞ്ജകം_List_Delete_NN(db, pList);
}
ശൂന്യം sqlite3_വ്യഞ്ജകം_List_Delete_Generic(sqlite3 *db, ശൂന്യം *pList){
  എങ്കിൽ( ALWAYS(pList) ) വ്യഞ്ജകം_List_Delete_NN(db, (വ്യഞ്ജകം_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_വ്യഞ്ജകം_List_Flags(സ്ഥിരം വ്യഞ്ജകം_List *pList){
  പൂർണ്ണസംഖ്യ i;
  u32 m = 0;
  പ്രതിജ്ഞാ( pList!=0 );
  വേണ്ടി(i=0; i<pList->n_വ്യഞ്ജകം; i++){
     വ്യഞ്ജകം *p_വ്യഞ്ജകം = pList->a[i].p_വ്യഞ്ജകം;
     പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 );
     m |= p_വ്യഞ്ജകം->flags;
  }
  തിരികെ m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
പൂർണ്ണസംഖ്യ sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  തിരികെ WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(സ്ഥിരം അക്ഷരം *zIn){
  എങ്കിൽ( sqlite3StrICmp(zIn, "true")==0  ) തിരികെ EP_IsTrue;
  എങ്കിൽ( sqlite3StrICmp(zIn, "false")==0 ) തിരികെ EP_IsFalse;
  തിരികെ 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Id_To_True_False(വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  u32 v;
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_ID || p_വ്യഞ്ജകം->op==TK_STRING );
  എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Quoted|EP_Int_മൂല്യം)
   && (v = sqlite3IsTrueOrFalse(p_വ്യഞ്ജകം->u.zToken))!=0
  ){
    p_വ്യഞ്ജകം->op = TK_TRUEFALSE;
    വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, v);
    തിരികെ 1;
  }
  തിരികെ 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Truth_മൂല്യം(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely((വ്യഞ്ജകം*)p_വ്യഞ്ജകം);
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_TRUEFALSE );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
  പ്രതിജ്ഞാ( sqlite3StrICmp(p_വ്യഞ്ജകം->u.zToken,"true")==0
       || sqlite3StrICmp(p_വ്യഞ്ജകം->u.zToken,"false")==0 );
  തിരികെ p_വ്യഞ്ജകം->u.zToken[4]==0;
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
വ്യഞ്ജകം *sqlite3_വ്യഞ്ജകം_Simplified_And_Or(വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 );
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_AND || p_വ്യഞ്ജകം->op==TK_OR ){
    വ്യഞ്ജകം *p_വലത് = sqlite3_വ്യഞ്ജകം_Simplified_And_Or(p_വ്യഞ്ജകം->p_വലത്);
    വ്യഞ്ജകം *p_ഇടത് = sqlite3_വ്യഞ്ജകം_Simplified_And_Or(p_വ്യഞ്ജകം->p_ഇടത്);
    എങ്കിൽ( വ്യഞ്ജകം_Always_True(p_ഇടത്) || വ്യഞ്ജകം_Always_False(p_വലത്) ){
      p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->op==TK_AND ? p_വലത് : p_ഇടത്;
    }അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Always_True(p_വലത്) || വ്യഞ്ജകം_Always_False(p_ഇടത്) ){
      p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->op==TK_AND ? p_ഇടത് : p_വലത്;
    }
  }
  തിരികെ p_വ്യഞ്ജകം;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Eval_Rhs_First(വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം->p_ഇടത്, EP_Subquery)
   && !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം->p_വലത്, EP_Subquery)
  ){
    തിരികെ 1;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,       /* The comparison expression */
  പൂർണ്ണസംഖ്യ *pR1,          /* OUT: Register holding the left operand */
  പൂർണ്ണസംഖ്യ *pR2,          /* OUT: Register holding the right operand */
  പൂർണ്ണസംഖ്യ *pFree1,       /* OUT: Temp register to free if not zero */
  പൂർണ്ണസംഖ്യ *pFree2        /* OUT: Another temp register to free if not zero */
){
  പൂർണ്ണസംഖ്യ addrIsNull;
  പൂർണ്ണസംഖ്യ r1, r2;
  Vdbe *v = pParse->pVdbe;

  പ്രതിജ്ഞാ( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  എങ്കിൽ( വ്യഞ്ജകം_Eval_Rhs_First(p_വ്യഞ്ജകം) && sqlite3_വ്യഞ്ജകം_Can_Be_Null(p_വ്യഞ്ജകം->p_വലത്) ){
    r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_വലത്, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }അല്ലെങ്കിൽ{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, pFree1);
  എങ്കിൽ( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം->p_വലത്, EP_Subquery)
     && sqlite3_വ്യഞ്ജകം_Can_Be_Null(p_വ്യഞ്ജകം->p_ഇടത്)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_വലത്, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  തിരികെ addrIsNull;
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
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_കണ്ണി_Is_Constant_Function(
  Walker *pWalker,
  വ്യഞ്ജകം *p_വ്യഞ്ജകം
){
  പൂർണ്ണസംഖ്യ n;             /* Number of arguments */
  വ്യഞ്ജകം_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_FUNCTION );
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly)
   || (pList = p_വ്യഞ്ജകം->x.pList)==0
  ){;
    n = 0;
  }അല്ലെങ്കിൽ{
    n = pList->n_വ്യഞ്ജകം;
    sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, pList);
    എങ്കിൽ( pWalker->eCode==0 ) തിരികെ WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_വ്യഞ്ജകം->u.zToken, n, ENC(db), 0);
  എങ്കിൽ( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    തിരികെ WRC_Abort;
  }
  തിരികെ WRC_Prune;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_കണ്ണി_Is_Constant(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പ്രതിജ്ഞാ( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  എങ്കിൽ( pWalker->eCode==2 && വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON) ){
    pWalker->eCode = 0;
    തിരികെ WRC_Abort;
  }

  തിരഞ്ഞെടുപ്പ്( p_വ്യഞ്ജകം->op ){
    അവസ്ഥ TK_FUNCTION:
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        തിരികെ WRC_Abort;
      }അല്ലെങ്കിൽ എങ്കിൽ( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        എങ്കിൽ( pWalker->eCode==5 ) വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_FromDDL);
        തിരികെ WRC_Continue;
      }അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        തിരികെ WRC_Continue;
      }അല്ലെങ്കിൽ എങ്കിൽ( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        തിരികെ വ്യഞ്ജകം_കണ്ണി_Is_Constant_Function(pWalker, p_വ്യഞ്ജകം);
      }അല്ലെങ്കിൽ{
        pWalker->eCode = 0;  /* Function call is not constant */
        തിരികെ WRC_Abort;
      }
    അവസ്ഥ TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Id_To_True_False(p_വ്യഞ്ജകം) ){
        തിരികെ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    അവസ്ഥ TK_COLUMN:
    അവസ്ഥ TK_AGG_FUNCTION:
    അവസ്ഥ TK_AGG_COLUMN:
      testcase( p_വ്യഞ്ജകം->op==TK_ID );
      testcase( p_വ്യഞ്ജകം->op==TK_COLUMN );
      testcase( p_വ്യഞ്ജകം->op==TK_AGG_FUNCTION );
      testcase( p_വ്യഞ്ജകം->op==TK_AGG_COLUMN );
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_FixedCol) && pWalker->eCode!=2 ){
        തിരികെ WRC_Continue;
      }
      എങ്കിൽ( pWalker->eCode==3 && p_വ്യഞ്ജകം->iTable==pWalker->u.iCur ){
        തിരികെ WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    അവസ്ഥ TK_IF_NULL_ROW:
    അവസ്ഥ TK_REGISTER:
    അവസ്ഥ TK_DOT:
    അവസ്ഥ TK_RAISE:
      testcase( p_വ്യഞ്ജകം->op==TK_REGISTER );
      testcase( p_വ്യഞ്ജകം->op==TK_IF_NULL_ROW );
      testcase( p_വ്യഞ്ജകം->op==TK_DOT );
      testcase( p_വ്യഞ്ജകം->op==TK_RAISE );
      pWalker->eCode = 0;
      തിരികെ WRC_Abort;
    അവസ്ഥ TK_IS:
    അവസ്ഥ TK_ISNOT:
      എങ്കിൽ( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        തിരികെ WRC_Abort;
      }
      തിരികെ WRC_Continue;
    അവസ്ഥ TK_VARIABLE:
      എങ്കിൽ( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_വ്യഞ്ജകം->op = TK_NULL;
      }അല്ലെങ്കിൽ എങ്കിൽ( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        തിരികെ WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    സ്ഥിരസ്ഥിതി:
      testcase( p_വ്യഞ്ജകം->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_വ്യഞ്ജകം->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      തിരികെ WRC_Continue;
  }
}
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Is_Const(Parse *pParse, വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_വ്യഞ്ജകം_Callback = വ്യഞ്ജകം_കണ്ണി_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_വ്യഞ്ജകം(&w, p);
  തിരികെ w.eCode;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Constant(Parse *pParse, വ്യഞ്ജകം *p){
  തിരികെ വ്യഞ്ജകം_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_List_Is_Constant(Parse *pParse, വ്യഞ്ജകം_List *pList, പൂർണ്ണസംഖ്യ bNoIs){
  സ്ഥിരം പൂർണ്ണസംഖ്യ iInit = bNoIs ? 6 : 1;
  പൂർണ്ണസംഖ്യ ii;
  വേണ്ടി(ii=0; ii<pList->n_വ്യഞ്ജകം; ii++){
    എങ്കിൽ( 0==വ്യഞ്ജകം_Is_Const(pParse, pList->a[ii].p_വ്യഞ്ജകം, iInit) ) തിരികെ 0;
  }
  തിരികെ 1;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Constant_Not_Join(Parse *pParse, വ്യഞ്ജകം *p){
  തിരികെ വ്യഞ്ജകം_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  പ്രതിജ്ഞാ( pSelect!=0 );
  പ്രതിജ്ഞാ( pWalker->eCode==3 || pWalker->eCode==0 );
  എങ്കിൽ( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    തിരികെ WRC_Abort;
  }
  തിരികെ WRC_Prune;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Table_Constant(വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ iCur, പൂർണ്ണസംഖ്യ bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_വ്യഞ്ജകം_Callback = വ്യഞ്ജകം_കണ്ണി_Is_Constant;
  എങ്കിൽ( bAllowSubq ){
    w.xSelectCallback = വ്യഞ്ജകം_Select_Walk_Table_Constant;
  }അല്ലെങ്കിൽ{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_വ്യഞ്ജകം(&w, p);
  തിരികെ w.eCode;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Single_Table_Constraint(
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,                 /* The constraint */
  സ്ഥിരം SrcList *pSrcList,     /* Complete FROM clause */
  പൂർണ്ണസംഖ്യ iSrc,                    /* Which element of pSrcList to use */
  പൂർണ്ണസംഖ്യ bAllowSubq               /* Allow non-correlated subqueries */
){
  സ്ഥിരം SrcItem *pSrc = &pSrcList->a[iSrc];
  എങ്കിൽ( pSrc->fg.jointype & JT_LTORJ ){
    തിരികെ 0;  /* rule (3) */
  }
  എങ്കിൽ( pSrc->fg.jointype & JT_ഇടത് ){
    എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON) ) തിരികെ 0;   /* rule (4a) */
    എങ്കിൽ( p_വ്യഞ്ജകം->w.iJoin!=pSrc->iCursor ) തിരികെ 0;         /* rule (4b) */
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON) ) തിരികെ 0;    /* rule (5) */
  }
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    പൂർണ്ണസംഖ്യ jj;
    വേണ്ടി(jj=0; jj<iSrc; jj++){
      എങ്കിൽ( p_വ്യഞ്ജകം->w.iJoin==pSrcList->a[jj].iCursor ){
        എങ്കിൽ( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          തിരികെ 0;  /* restriction (6) */
        }
        നിർത്തുക;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  തിരികെ sqlite3_വ്യഞ്ജകം_Is_Table_Constant(p_വ്യഞ്ജകം, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_കണ്ണി_Is_Constant_Or_Group_By(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  വ്യഞ്ജകം_List *pGroupBy = pWalker->u.pGroupBy;
  പൂർണ്ണസംഖ്യ i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  വേണ്ടി(i=0; i<pGroupBy->n_വ്യഞ്ജകം; i++){
    വ്യഞ്ജകം *p = pGroupBy->a[i].p_വ്യഞ്ജകം;
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0, p_വ്യഞ്ജകം, p, -1)<2 ){
      CollSeq *pColl = sqlite3_വ്യഞ്ജകം_NNColl_Seq(pWalker->pParse, p);
      എങ്കിൽ( sqlite3_Is_ദ്വിമാന(pColl) ){
        തിരികെ WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
    pWalker->eCode = 0;
    തിരികെ WRC_Abort;
  }

  തിരികെ വ്യഞ്ജകം_കണ്ണി_Is_Constant(pWalker, p_വ്യഞ്ജകം);
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Constant_Or_Group_By(Parse *pParse, വ്യഞ്ജകം *p, വ്യഞ്ജകം_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_വ്യഞ്ജകം_Callback = വ്യഞ്ജകം_കണ്ണി_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_വ്യഞ്ജകം(&w, p);
  തിരികെ w.eCode;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Constant_Or_Function(വ്യഞ്ജകം *p, u8 isInit){
  പ്രതിജ്ഞാ( isInit==0 || isInit==1 );
  തിരികെ വ്യഞ്ജകം_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Contains_Subquery(വ്യഞ്ജകം *p){
  Walker w;
  w.eCode = 1;
  w.x_വ്യഞ്ജകം_Callback = sqlite3_വ്യഞ്ജകം_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_വ്യഞ്ജകം(&w, p);
  തിരികെ w.eCode==0;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Integer(സ്ഥിരം വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ *p_മൂല്യം, Parse *pParse, പൂർണ്ണസംഖ്യ bRSI){
  പൂർണ്ണസംഖ്യ iSign = 1;   /* Either +1 or -1. */
  ആകുമ്പോൾ( 1/*exit-by-break*/ ){
    എങ്കിൽ( NEVER(p==0) ) തിരികെ 0;
    എങ്കിൽ( വ്യഞ്ജകം_Use_UValue(p) ){
      പ്രതിജ്ഞാ( p->op==TK_INTEGER );
      *p_മൂല്യം = p->u.i_മൂല്യം*iSign;
      എങ്കിൽ( bRSI && pParse==0 && p->op2 ){
        തിരികെ 0;
      }
      തിരികെ 1;
    }
    എങ്കിൽ( p->op==TK_UPLUS ){
      p = p->p_ഇടത്;
      pParse = 0;
      തുടരുക;
    }
    എങ്കിൽ( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_ഇടത്;
      pParse = 0;
      തുടരുക;
    }
    എങ്കിൽ( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_മൂല്യം *pVal; /* The variable */
      പൂർണ്ണസംഖ്യ rc = 0;          /* 1 if successful, 0 if failed */
      പ്രതിജ്ഞാ( iSign==1 );
      എങ്കിൽ( NEVER(pParse->pVdbe==0) ) നിർത്തുക;
      എങ്കിൽ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) നിർത്തുക;
      pVal = sqlite3_Vdbe_Get_Bound_മൂല്യം(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      എങ്കിൽ( pVal ){
        i64 vv;
        എങ്കിൽ( sqlite3_മൂല്യം_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_മൂല്യം_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_മൂല്യം = (പൂർണ്ണസംഖ്യ)vv;
          rc = 1;       
        }
        sqlite3_മൂല്യം_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      തിരികെ rc;
    }
    നിർത്തുക;
  }
  തിരികെ 0;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Can_Be_Null(സ്ഥിരം വ്യഞ്ജകം *p){
  u8 op;
  പ്രതിജ്ഞാ( p!=0 );
  ആകുമ്പോൾ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_ഇടത്;
    പ്രതിജ്ഞാ( p!=0 );
  }
  op = p->op;
  എങ്കിൽ( op==TK_REGISTER ) op = p->op2;
  തിരഞ്ഞെടുപ്പ്( op ){
    അവസ്ഥ TK_INTEGER:
    അവസ്ഥ TK_STRING:
    അവസ്ഥ TK_FLOAT:
    അവസ്ഥ TK_BLOB:
      തിരികെ 0;
    അവസ്ഥ TK_COLUMN:
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p) );
      തിരികെ വ്യഞ്ജകം_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    സ്ഥിരസ്ഥിതി:
      തിരികെ 1;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Needs_No_Affinity_Change(സ്ഥിരം വ്യഞ്ജകം *p, അക്ഷരം aff){
  u8 op;
  പൂർണ്ണസംഖ്യ unaryMinus = 0;
  എങ്കിൽ( aff==SQLITE_AFF_BLOB ) തിരികെ 1;
  ആകുമ്പോൾ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    എങ്കിൽ( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_ഇടത്;
  }
  op = p->op;
  എങ്കിൽ( op==TK_REGISTER ) op = p->op2;
  തിരഞ്ഞെടുപ്പ്( op ){
    അവസ്ഥ TK_INTEGER: {
      തിരികെ aff>=SQLITE_AFF_NUMERIC;
    }
    അവസ്ഥ TK_FLOAT: {
      തിരികെ aff>=SQLITE_AFF_NUMERIC;
    }
    അവസ്ഥ TK_STRING: {
      തിരികെ !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    അവസ്ഥ TK_BLOB: {
      തിരികെ !unaryMinus;
    }
    അവസ്ഥ TK_COLUMN: {
      പ്രതിജ്ഞാ( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      തിരികെ aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    സ്ഥിരസ്ഥിതി: {
      തിരികെ 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
പൂർണ്ണസംഖ്യ sqlite3IsRowid(സ്ഥിരം അക്ഷരം *z){
  എങ്കിൽ( sqlite3StrICmp(z, "_ROWID_")==0 ) തിരികെ 1;
  എങ്കിൽ( sqlite3StrICmp(z, "ROWID")==0 ) തിരികെ 1;
  എങ്കിൽ( sqlite3StrICmp(z, "OID")==0 ) തിരികെ 1;
  തിരികെ 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
സ്ഥിരം അക്ഷരം *sqlite3RowidAlias(Table *pTab){
  സ്ഥിരം അക്ഷരം *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  പൂർണ്ണസംഖ്യ ii;
  പ്രതിജ്ഞാ( VisibleRowid(pTab) );
  വേണ്ടി(ii=0; ii<ArraySize(azOpt); ii++){
    എങ്കിൽ( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) തിരികെ azOpt[ii];
  }
  തിരികെ 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
സ്ഥിതം Select *isCandidateForInOpt(സ്ഥിരം വ്യഞ്ജകം *pX){
  Select *p;
  SrcList *pSrc;
  വ്യഞ്ജകം_List *pEList;
  Table *pTab;
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( !വ്യഞ്ജകം_Use_XSelect(pX) ) തിരികെ 0;                 /* Not a subquery */
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(pX, EP_VarSelect)  ) തിരികെ 0;  /* Correlated subq */
  p = pX->x.pSelect;
  എങ്കിൽ( p->pPrior ) തിരികെ 0;              /* Not a compound SELECT */
  എങ്കിൽ( p->selFlags & SF_Aggregate ){
    തിരികെ 0; /* No GROUP BY keyword or aggregate functions */
  }
  പ്രതിജ്ഞാ( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  എങ്കിൽ( p->pLimit ) തിരികെ 0;              /* Has no LIMIT clause */
  എങ്കിൽ( p->pWhere ) തിരികെ 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  പ്രതിജ്ഞാ( pSrc!=0 );
  എങ്കിൽ( pSrc->nSrc!=1 ) തിരികെ 0;          /* Single term in FROM clause */
  എങ്കിൽ( pSrc->a[0].fg.isSubquery) തിരികെ 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  പ്രതിജ്ഞാ( pTab!=0 );
  പ്രതിജ്ഞാ( !IsView(pTab)  );              /* FROM clause is not a view */
  എങ്കിൽ( IsVirtual(pTab) ) തിരികെ 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  പ്രതിജ്ഞാ( pEList!=0 );
  /* All SELECT results must be columns. */
  വേണ്ടി(i=0; i<pEList->n_വ്യഞ്ജകം; i++){
    വ്യഞ്ജകം *pRes = pEList->a[i].p_വ്യഞ്ജകം;
    എങ്കിൽ( pRes->op!=TK_COLUMN ) തിരികെ 0;
    പ്രതിജ്ഞാ( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  തിരികെ p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
സ്ഥിതം ശൂന്യം sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  പൂർണ്ണസംഖ്യ iCur,          /* Cursor for the index */
  പൂർണ്ണസംഖ്യ regHasNull,    /* Register in which to store hasNull flag */
  പൂർണ്ണസംഖ്യ eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  പൂർണ്ണസംഖ്യ addr1;
  പൂർണ്ണസംഖ്യ op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  എങ്കിൽ( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3InRhsIsConstant(Parse *pParse, വ്യഞ്ജകം *pIn){
  വ്യഞ്ജകം *pLHS;
  പൂർണ്ണസംഖ്യ res;
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_ഇടത്;
  pIn->p_ഇടത് = 0;
  res = sqlite3_വ്യഞ്ജകം_Is_Constant(pParse, pIn);
  pIn->p_ഇടത് = pLHS;
  തിരികെ res;
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
പൂർണ്ണസംഖ്യ sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  വ്യഞ്ജകം *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  പൂർണ്ണസംഖ്യ *prRhsHasNull,         /* Register holding NULL status.  See notes */
  പൂർണ്ണസംഖ്യ *aiMap,                /* Mapping from Index fields to RHS fields */
  പൂർണ്ണസംഖ്യ *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  പൂർണ്ണസംഖ്യ eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  പൂർണ്ണസംഖ്യ iTab;                             /* Cursor of the RHS table */
  പൂർണ്ണസംഖ്യ mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  പ്രതിജ്ഞാ( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  എങ്കിൽ( prRhsHasNull && വ്യഞ്ജകം_Use_XSelect(pX) ){
    പൂർണ്ണസംഖ്യ i;
    വ്യഞ്ജകം_List *pEList = pX->x.pSelect->pEList;
    വേണ്ടി(i=0; i<pEList->n_വ്യഞ്ജകം; i++){
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Can_Be_Null(pEList->a[i].p_വ്യഞ്ജകം) ) നിർത്തുക;
    }
    എങ്കിൽ( i==pEList->n_വ്യഞ്ജകം ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  എങ്കിൽ( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    പൂർണ്ണസംഖ്യ iDb;                               /* Database idx for pTab */
    വ്യഞ്ജകം_List *pEList = p->pEList;
    പൂർണ്ണസംഖ്യ n_വ്യഞ്ജകം = pEList->n_വ്യഞ്ജകം;

    പ്രതിജ്ഞാ( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    പ്രതിജ്ഞാ( p->pEList->a[0].p_വ്യഞ്ജകം!=0 ); /* Because of isCandidateForInOpt(p) */
    പ്രതിജ്ഞാ( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    പ്രതിജ്ഞാ( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    പ്രതിജ്ഞാ(v);  /* sqlite3GetVdbe() has always been previously called */
    എങ്കിൽ( n_വ്യഞ്ജകം==1 && pEList->a[0].p_വ്യഞ്ജകം->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      പൂർണ്ണസംഖ്യ iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }അല്ലെങ്കിൽ{
      Index *pIdx;                         /* Iterator variable */
      പൂർണ്ണസംഖ്യ affinity_ok = 1;
      പൂർണ്ണസംഖ്യ i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      വേണ്ടി(i=0; i<n_വ്യഞ്ജകം && affinity_ok; i++){
        വ്യഞ്ജകം *pLhs = sqlite3VectorFieldSubexpr(pX->p_ഇടത്, i);
        പൂർണ്ണസംഖ്യ iCol = pEList->a[i].p_വ്യഞ്ജകം->iColumn;
        അക്ഷരം idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        അക്ഷരം cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        തിരഞ്ഞെടുപ്പ്( cmpaff ){
          അവസ്ഥ SQLITE_AFF_BLOB:
            നിർത്തുക;
          അവസ്ഥ SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            പ്രതിജ്ഞാ( idxaff==SQLITE_AFF_TEXT );
            നിർത്തുക;
          സ്ഥിരസ്ഥിതി:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      എങ്കിൽ( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        വേണ്ടി(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          എങ്കിൽ( pIdx->nColumn<n_വ്യഞ്ജകം ) തുടരുക;
          എങ്കിൽ( pIdx->pPartIdxWhere!=0 ) തുടരുക;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          എങ്കിൽ( pIdx->nColumn>=BMS-1 ) തുടരുക;
          എങ്കിൽ( mustBeUnique ){
            എങ്കിൽ( pIdx->n_താക്കോൽ_Col>n_വ്യഞ്ജകം
             ||(pIdx->nColumn>n_വ്യഞ്ജകം && !IsUniqueIndex(pIdx))
            ){
              തുടരുക;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          വേണ്ടി(i=0; i<n_വ്യഞ്ജകം; i++){
            വ്യഞ്ജകം *pLhs = sqlite3VectorFieldSubexpr(pX->p_ഇടത്, i);
            വ്യഞ്ജകം *pRhs = pEList->a[i].p_വ്യഞ്ജകം;
            CollSeq *pReq = sqlite3_ദ്വിമാന_Compare_Coll_Seq(pParse, pLhs, pRhs);
            പൂർണ്ണസംഖ്യ j;
 
            വേണ്ടി(j=0; j<n_വ്യഞ്ജകം; j++){
              എങ്കിൽ( pIdx->aiColumn[j]!=pRhs->iColumn ) തുടരുക;
              പ്രതിജ്ഞാ( pIdx->azColl[j] );
              എങ്കിൽ( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                തുടരുക;
              }
              നിർത്തുക;
            }
            എങ്കിൽ( j==n_വ്യഞ്ജകം ) നിർത്തുക;
            mCol = MASKBIT(j);
            എങ്കിൽ( mCol & colUsed ) നിർത്തുക; /* Each column used only once */
            colUsed |= mCol;
            എങ്കിൽ( aiMap ) aiMap[i] = j;
          }
 
          പ്രതിജ്ഞാ( n_വ്യഞ്ജകം>0 && n_വ്യഞ്ജകം<BMS );
          പ്രതിജ്ഞാ( i==n_വ്യഞ്ജകം || colUsed!=(MASKBIT(n_വ്യഞ്ജകം)-1) );
          എങ്കിൽ( colUsed==(MASKBIT(n_വ്യഞ്ജകം)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            പൂർണ്ണസംഖ്യ iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_താക്കോൽ_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            പ്രതിജ്ഞാ( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            എങ്കിൽ( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_വ്യഞ്ജകം)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              എങ്കിൽ( n_വ്യഞ്ജകം==1 ){
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
  എങ്കിൽ( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && വ്യഞ്ജകം_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_വ്യഞ്ജകം<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  എങ്കിൽ( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    പൂർണ്ണസംഖ്യ rMayHaveNull = 0;
    പൂർണ്ണസംഖ്യ bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    എങ്കിൽ( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    പ്രതിജ്ഞാ( pX->op==TK_IN );
    എങ്കിൽ( !bloomOk
     && വ്യഞ്ജകം_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    എങ്കിൽ( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  എങ്കിൽ( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    പൂർണ്ണസംഖ്യ i, n;
    n = sqlite3_വ്യഞ്ജകം_Vector_Size(pX->p_ഇടത്);
    വേണ്ടി(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  തിരികെ eType;
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
സ്ഥിതം അക്ഷരം *വ്യഞ്ജകം_INAffinity(Parse *pParse, സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
  പൂർണ്ണസംഖ്യ nVal = sqlite3_വ്യഞ്ജകം_Vector_Size(p_ഇടത്);
  Select *pSelect = വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ? p_വ്യഞ്ജകം->x.pSelect : 0;
  അക്ഷരം *zRet;

  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  എങ്കിൽ( zRet ){
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<nVal; i++){
      വ്യഞ്ജകം *pA = sqlite3VectorFieldSubexpr(p_ഇടത്, i);
      അക്ഷരം a = sqlite3_വ്യഞ്ജകം_Affinity(pA);
      എങ്കിൽ( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_വ്യഞ്ജകം, a);
      }അല്ലെങ്കിൽ{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  തിരികെ zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
ശൂന്യം sqlite3SubselectError(Parse *pParse, പൂർണ്ണസംഖ്യ nActual, പൂർണ്ണസംഖ്യ nExpect){
  എങ്കിൽ( pParse->nErr==0 ){
    സ്ഥിരം അക്ഷരം *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_സന്ദേശം(pParse, zFmt, nActual, nExpect);
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
ശൂന്യം sqlite3_Vector_Error_സന്ദേശം(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
#ifndef SQLITE_OMIT_SUBQUERY
  എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
    sqlite3SubselectError(pParse, p_വ്യഞ്ജകം->x.pSelect->pEList->n_വ്യഞ്ജകം, 1);
  }അല്ലെങ്കിൽ
#endif
  {
    sqlite3_Error_സന്ദേശം(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  എങ്കിൽ( pNewSig==0 ) തിരികെ 0;
  എങ്കിൽ( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) തിരികെ 0;
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_IN );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_YSub(p_വ്യഞ്ജകം) );
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pSelect!=0 );
  പ്രതിജ്ഞാ( (p_വ്യഞ്ജകം->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  പ്രതിജ്ഞാ( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  വേണ്ടി(; pOp<pEnd; pOp++){
    എങ്കിൽ( pOp->p4type!=P4_SUBRTNSIG ) തുടരുക;
    പ്രതിജ്ഞാ( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    പ്രതിജ്ഞാ( pSig!=0 );
    എങ്കിൽ( !pSig->bComplete ) തുടരുക;
    എങ്കിൽ( pNewSig->selId!=pSig->selId ) തുടരുക;
    എങ്കിൽ( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) തുടരുക;
    p_വ്യഞ്ജകം->y.sub.iAddr = pSig->iAddr;
    p_വ്യഞ്ജകം->y.sub.regReturn = pSig->regReturn;
    p_വ്യഞ്ജകം->iTable = pSig->iTable;
    വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_Subrtn);
    തിരികെ 1;
  }
  തിരികെ 0;
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
ശൂന്യം sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,            /* The IN operator */
  പൂർണ്ണസംഖ്യ iTab,               /* Use this cursor number */
  പൂർണ്ണസംഖ്യ allowBloom          /* True to allow the use of a Bloom filter */
){
  പൂർണ്ണസംഖ്യ addrOnce = 0;           /* Address of the OP_Once instruction at top */
  പൂർണ്ണസംഖ്യ addr;                   /* Address of OP_OpenEphemeral instruction */
  വ്യഞ്ജകം *p_ഇടത്;                /* the LHS of the IN operator */
  താക്കോൽ_Info *p_താക്കോൽ_Info = 0;      /* Key information */
  പൂർണ്ണസംഖ്യ nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  പ്രതിജ്ഞാ( v!=0 );

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
  എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) || p_വ്യഞ്ജകം->x.pSelect!=0 );
    എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) && (p_വ്യഞ്ജകം->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, വലുപ്പം(pSig[0]));
      എങ്കിൽ( pSig ){
        pSig->selId = p_വ്യഞ്ജകം->x.pSelect->selId;
        pSig->zAff = വ്യഞ്ജകം_INAffinity(pParse, p_വ്യഞ്ജകം);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_വ്യഞ്ജകം, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_വ്യഞ്ജകം->x.pSelect->selId));
      }
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YSub(p_വ്യഞ്ജകം) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_വ്യഞ്ജകം->y.sub.regReturn,
                        p_വ്യഞ്ജകം->y.sub.iAddr);
      പ്രതിജ്ഞാ( iTab!=p_വ്യഞ്ജകം->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_വ്യഞ്ജകം->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      എങ്കിൽ( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      തിരികെ;
    }

    /* Begin coding the subroutine */
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_YWin(p_വ്യഞ്ജകം) );
    വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_Subrtn);
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly|EP_Reduced) );
    p_വ്യഞ്ജകം->y.sub.regReturn = ++pParse->nMem;
    p_വ്യഞ്ജകം->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_വ്യഞ്ജകം->y.sub.regReturn) + 1;
    എങ്കിൽ( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_വ്യഞ്ജകം->y.sub.iAddr;
      pSig->regReturn = p_വ്യഞ്ജകം->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (സ്ഥിരം അക്ഷരം*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
  nVal = sqlite3_വ്യഞ്ജകം_Vector_Size(p_ഇടത്);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_വ്യഞ്ജകം->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_വ്യഞ്ജകം->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
    VdbeComment((v, "Result of SELECT %u", p_വ്യഞ്ജകം->x.pSelect->selId));
  }അല്ലെങ്കിൽ{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_താക്കോൽ_Info = sqlite3_താക്കോൽ_Info_Alloc(pParse->db, nVal, 1);
  പ്രതിജ്ഞാ( p_താക്കോൽ_Info!=0 || pParse->nErr );
  എങ്കിൽ( p_താക്കോൽ_Info==0 ) തിരികെ;

  എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_വ്യഞ്ജകം->x.pSelect;
    വ്യഞ്ജകം_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    എങ്കിൽ( ALWAYS(pEList->n_വ്യഞ്ജകം==nVal) ){
      Select *pCopy;
      SelectDest dest;
      പൂർണ്ണസംഖ്യ i;
      പൂർണ്ണസംഖ്യ rc;
      പൂർണ്ണസംഖ്യ addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = വ്യഞ്ജകം_INAffinity(pParse, p_വ്യഞ്ജകം);
      pSelect->iLimit = 0;
      പ്രതിജ്ഞാ( pEList!=0 );
      പ്രതിജ്ഞാ( pEList->n_വ്യഞ്ജകം>0 );
      പ്രതിജ്ഞാ( sqlite3_താക്കോൽ_Info_Is_Writeable(p_താക്കോൽ_Info) );
      വേണ്ടി(i=0; i<nVal; i++){
        വ്യഞ്ജകം *p = sqlite3VectorFieldSubexpr(p_ഇടത്, i);
        CollSeq *pColl;
        p_താക്കോൽ_Info->aColl[i] = pColl = sqlite3_ദ്വിമാന_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_വ്യഞ്ജകം
        );
        എങ്കിൽ( !sqlite3_Is_ദ്വിമാന(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      എങ്കിൽ( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        പൂർണ്ണസംഖ്യ regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (ശൂന്യം *)p_താക്കോൽ_Info, P4_KEYINFO);
        p_താക്കോൽ_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      എങ്കിൽ( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        എങ്കിൽ( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      എങ്കിൽ( rc ){
        sqlite3_താക്കോൽ_Info_Unref(p_താക്കോൽ_Info);
        തിരികെ;
      }
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( ALWAYS(p_വ്യഞ്ജകം->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    അക്ഷരം affinity;            /* Affinity of the LHS of the IN */
    പൂർണ്ണസംഖ്യ i;
    വ്യഞ്ജകം_List *pList = p_വ്യഞ്ജകം->x.pList;
    ഘടന വ്യഞ്ജകം_List_item *pItem;
    പൂർണ്ണസംഖ്യ r1, r2;
    affinity = sqlite3_വ്യഞ്ജകം_Affinity(p_ഇടത്);
    എങ്കിൽ( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }അല്ലെങ്കിൽ എങ്കിൽ( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    പ്രതിജ്ഞാ( p_താക്കോൽ_Info!=0 );
    പ്രതിജ്ഞാ( sqlite3_താക്കോൽ_Info_Is_Writeable(p_താക്കോൽ_Info) );
    p_താക്കോൽ_Info->aColl[0] = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, p_വ്യഞ്ജകം->p_ഇടത്);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    വേണ്ടി(i=pList->n_വ്യഞ്ജകം, pItem=pList->a; i>0; i--, pItem++){
      വ്യഞ്ജകം *pE2 = pItem->p_വ്യഞ്ജകം;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      എങ്കിൽ( addrOnce && !sqlite3_വ്യഞ്ജകം_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        വ്യഞ്ജകം_Clear_Property(p_വ്യഞ്ജകം, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_വ്യഞ്ജകം_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_തിരുകുക, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  എങ്കിൽ( pSig ) pSig->bComplete = 1;
  എങ്കിൽ( p_താക്കോൽ_Info ){
    sqlite3VdbeChangeP4(v, addr, (ശൂന്യം *)p_താക്കോൽ_Info, P4_KEYINFO);
  }
  എങ്കിൽ( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YSub(p_വ്യഞ്ജകം) );
    പ്രതിജ്ഞാ( sqlite3VdbeGetOp(v,p_വ്യഞ്ജകം->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_വ്യഞ്ജകം->y.sub.regReturn,
                      p_വ്യഞ്ജകം->y.sub.iAddr, 1);
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
പൂർണ്ണസംഖ്യ sqlite3CodeSubselect(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പൂർണ്ണസംഖ്യ addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  പൂർണ്ണസംഖ്യ rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  പൂർണ്ണസംഖ്യ nReg;                   /* Registers to allocate */
  വ്യഞ്ജകം *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  പൂർണ്ണസംഖ്യ addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  പ്രതിജ്ഞാ( v!=0 );
  എങ്കിൽ( pParse->nErr ) തിരികെ 0;
  testcase( p_വ്യഞ്ജകം->op==TK_EXISTS );
  testcase( p_വ്യഞ്ജകം->op==TK_SELECT );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_EXISTS || p_വ്യഞ്ജകം->op==TK_SELECT );
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) );
  pSel = p_വ്യഞ്ജകം->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YSub(p_വ്യഞ്ജകം) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_വ്യഞ്ജകം->y.sub.regReturn,
                      p_വ്യഞ്ജകം->y.sub.iAddr);
    തിരികെ p_വ്യഞ്ജകം->iTable;
  }

  /* Begin coding the subroutine */
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Use_YWin(p_വ്യഞ്ജകം) );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Reduced|EP_TokenOnly) );
  വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_Subrtn);
  p_വ്യഞ്ജകം->y.sub.regReturn = ++pParse->nMem;
  p_വ്യഞ്ജകം->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_വ്യഞ്ജകം->y.sub.regReturn) + 1;

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
  എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_VarSelect) ){
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
  nReg = p_വ്യഞ്ജകം->op==TK_SELECT ? pSel->pEList->n_വ്യഞ്ജകം : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    എങ്കിൽ( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_വലത് ){
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
    }അല്ലെങ്കിൽ{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }അല്ലെങ്കിൽ{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  എങ്കിൽ( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    വ്യഞ്ജകം *p_ഇടത് = pSel->pLimit->p_ഇടത്;
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_ഇടത്, EP_Int_മൂല്യം)==0
     || (p_ഇടത്->u.i_മൂല്യം!=1 && p_ഇടത്->u.i_മൂല്യം!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_വ്യഞ്ജകം_Int32(db, 0);
      എങ്കിൽ( pLimit ){
        pLimit->aff_വ്യഞ്ജകം = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_വ്യഞ്ജകം_Dup(db, p_ഇടത്, 0), pLimit);
      }
      sqlite3_വ്യഞ്ജകം_Deferred_Delete(pParse, p_ഇടത്);
      pSel->pLimit->p_ഇടത് = pLimit;
    }
  }അല്ലെങ്കിൽ{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_വ്യഞ്ജകം_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  എങ്കിൽ( sqlite3Select(pParse, pSel, &dest) ){
    p_വ്യഞ്ജകം->op2 = p_വ്യഞ്ജകം->op;
    p_വ്യഞ്ജകം->op = TK_ERROR;
    തിരികെ 0;
  }
  p_വ്യഞ്ജകം->iTable = rReg = dest.iSDParm;
  വ്യഞ്ജകം_Set_VVAProperty(p_വ്യഞ്ജകം, EP_NoReduce);
  എങ്കിൽ( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  എങ്കിൽ( dest.eDest==SRT_Mem 
   && sqlite3_വ്യഞ്ജകം_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_വ്യഞ്ജകം) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YSub(p_വ്യഞ്ജകം) );
  പ്രതിജ്ഞാ( sqlite3VdbeGetOp(v,p_വ്യഞ്ജകം->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_വ്യഞ്ജകം->y.sub.regReturn,
                    p_വ്യഞ്ജകം->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  തിരികെ rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Check_IN(Parse *pParse, വ്യഞ്ജകം *pIn){
  പൂർണ്ണസംഖ്യ nVector = sqlite3_വ്യഞ്ജകം_Vector_Size(pIn->p_ഇടത്);
  എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    എങ്കിൽ( nVector!=pIn->x.pSelect->pEList->n_വ്യഞ്ജകം ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_വ്യഞ്ജകം, nVector);
      തിരികെ 1;
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( nVector!=1 ){
    sqlite3_Vector_Error_സന്ദേശം(pParse, pIn->p_ഇടത്);
    തിരികെ 1;
  }
  തിരികെ 0;
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
സ്ഥിതം ശൂന്യം sqlite3_വ്യഞ്ജകം_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,          /* The IN expression */
  പൂർണ്ണസംഖ്യ destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  പൂർണ്ണസംഖ്യ destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  പൂർണ്ണസംഖ്യ rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  പൂർണ്ണസംഖ്യ eType;            /* Type of the RHS */
  പൂർണ്ണസംഖ്യ rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  പൂർണ്ണസംഖ്യ *aiMap = 0;       /* Map from vector field to index column */
  അക്ഷരം *zAff = 0;       /* Affinity string for comparisons */
  പൂർണ്ണസംഖ്യ nVector;          /* Size of vectors for this IN operator */
  പൂർണ്ണസംഖ്യ iDummy;           /* Dummy parameter to exprCodeVector() */
  വ്യഞ്ജകം *p_ഇടത്;          /* The LHS of the IN operator */
  പൂർണ്ണസംഖ്യ i;                /* loop counter */
  പൂർണ്ണസംഖ്യ destStep2;        /* Where to jump when NULLs seen in step 2 */
  പൂർണ്ണസംഖ്യ destStep6 = 0;    /* Start of code for Step 6 */
  പൂർണ്ണസംഖ്യ addrTruthOp;      /* Address of opcode that determines the IN is true */
  പൂർണ്ണസംഖ്യ destNotNull;      /* Jump here if a comparison is not true in step 6 */
  പൂർണ്ണസംഖ്യ addr_മുകൾ;          /* Top of the step-6 loop */
  പൂർണ്ണസംഖ്യ iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_VVAProperty(p_വ്യഞ്ജകം,EP_Immutable) );
  p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Check_IN(pParse, p_വ്യഞ്ജകം) ) തിരികെ;
  zAff = വ്യഞ്ജകം_INAffinity(pParse, p_വ്യഞ്ജകം);
  nVector = sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം->p_ഇടത്);
  aiMap = (പൂർണ്ണസംഖ്യ*)sqlite3DbMallocZero(pParse->db, nVector*വലുപ്പം(പൂർണ്ണസംഖ്യ));
  എങ്കിൽ( pParse->db->mallocFailed ) പോകുക sqlite3_വ്യഞ്ജകം_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  പ്രതിജ്ഞാ( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_വ്യഞ്ജകം,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  പ്രതിജ്ഞാ( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  വേണ്ടി(i=0; i<nVector; i++){
    പൂർണ്ണസംഖ്യ j, cnt;
    വേണ്ടി(cnt=j=0; j<nVector; j++) എങ്കിൽ( aiMap[j]==i ) cnt++;
    പ്രതിജ്ഞാ( cnt==1 );
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
  പ്രതിജ്ഞാ( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = വ്യഞ്ജകം_Code_Vector(pParse, p_ഇടത്, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  എങ്കിൽ( eType==IN_INDEX_NOOP ){
    വ്യഞ്ജകം_List *pList;
    CollSeq *pColl;
    പൂർണ്ണസംഖ്യ labelOk = sqlite3VdbeMakeLabel(pParse);
    പൂർണ്ണസംഖ്യ r2, regToFree;
    പൂർണ്ണസംഖ്യ regCkNull = 0;
    പൂർണ്ണസംഖ്യ ii;
    പ്രതിജ്ഞാ( nVector==1 );
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
    pList = p_വ്യഞ്ജകം->x.pList;
    pColl = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, p_വ്യഞ്ജകം->p_ഇടത്);
    എങ്കിൽ( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    വേണ്ടി(ii=0; ii<pList->n_വ്യഞ്ജകം; ii++){
      r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, pList->a[ii].p_വ്യഞ്ജകം, &regToFree);
      എങ്കിൽ( regCkNull && sqlite3_വ്യഞ്ജകം_Can_Be_Null(pList->a[ii].p_വ്യഞ്ജകം) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      എങ്കിൽ( ii<pList->n_വ്യഞ്ജകം-1 || destIfNull!=destIfFalse ){
        പൂർണ്ണസംഖ്യ op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (ശൂന്യം*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_വ്യഞ്ജകം-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_വ്യഞ്ജകം-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_വ്യഞ്ജകം-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_വ്യഞ്ജകം-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        പ്രതിജ്ഞാ( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (ശൂന്യം*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    എങ്കിൽ( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    പോകുക sqlite3_വ്യഞ്ജകം_Code_IN_finished;
  }

  എങ്കിൽ( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    എങ്കിൽ( nVector==1 ){
      അക്ഷരം aff = zAff[0];
      എങ്കിൽ( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_വ്യഞ്ജകം_Affinity(p_ഇടത്) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        പൂർണ്ണസംഖ്യ rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }അല്ലെങ്കിൽ{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    വേണ്ടി(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    എങ്കിൽ( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      പൂർണ്ണസംഖ്യ rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      വേണ്ടി(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  എങ്കിൽ( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }അല്ലെങ്കിൽ{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  വേണ്ടി(i=0; i<nVector; i++){
    വ്യഞ്ജകം *p = sqlite3VectorFieldSubexpr(p_വ്യഞ്ജകം->p_ഇടത്, i);
    എങ്കിൽ( pParse->nErr ) പോകുക sqlite3_വ്യഞ്ജകം_Code_IN_oom_error;
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  എങ്കിൽ( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    പ്രതിജ്ഞാ( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Subrtn) ){
        സ്ഥിരം VdbeOp *pOp = sqlite3VdbeGetOp(v, p_വ്യഞ്ജകം->y.sub.iAddr);
        പ്രതിജ്ഞാ( pOp->opcode==OP_Once || pParse->nErr );
        എങ്കിൽ( pOp->p3>0 ){  /* tag-202407032019 */
          പ്രതിജ്ഞാ( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      പോകുക sqlite3_വ്യഞ്ജകം_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  എങ്കിൽ( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  എങ്കിൽ( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  എങ്കിൽ( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  എങ്കിൽ( eType==IN_INDEX_INDEX_DESC ){
    addr_മുകൾ = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }അല്ലെങ്കിൽ{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_മുകൾ = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  എങ്കിൽ( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }അല്ലെങ്കിൽ{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  വേണ്ടി(i=0; i<nVector; i++){
    വ്യഞ്ജകം *p;
    CollSeq *pColl;
    പൂർണ്ണസംഖ്യ r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_ഇടത്, i);
    എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
      വ്യഞ്ജകം *pRhs = p_വ്യഞ്ജകം->x.pSelect->pEList->a[i].p_വ്യഞ്ജകം;
      pColl = sqlite3_ദ്വിമാന_Compare_Coll_Seq(pParse, p, pRhs);
    }അല്ലെങ്കിൽ{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (ശൂന്യം*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  എങ്കിൽ( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_മുകൾ+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_വ്യഞ്ജകം_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_വ്യഞ്ജകം_Code_IN_oom_error:
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
സ്ഥിതം ശൂന്യം codeReal(Vdbe *v, സ്ഥിരം അക്ഷരം *z, പൂർണ്ണസംഖ്യ negateFlag, പൂർണ്ണസംഖ്യ iMem){
  എങ്കിൽ( ALWAYS(z!=0) ){
    ഇരട്ട മൂല്യം;
    sqlite3AtoF(z, &മൂല്യം);
    പ്രതിജ്ഞാ( !sqlite3IsNaN(മൂല്യം) ); /* The new AtoF never returns NaN */
    എങ്കിൽ( negateFlag ) മൂല്യം = -മൂല്യം;
    sqlite3VdbeAddDouble(v, iMem, മൂല്യം);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
സ്ഥിതം ശൂന്യം codeInteger(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ negFlag, പൂർണ്ണസംഖ്യ iMem){
  Vdbe *v = pParse->pVdbe;
  എങ്കിൽ( p_വ്യഞ്ജകം->flags & EP_Int_മൂല്യം ){
    പൂർണ്ണസംഖ്യ i = p_വ്യഞ്ജകം->u.i_മൂല്യം;
    പ്രതിജ്ഞാ( i>=0 );
    എങ്കിൽ( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ c;
    i64 മൂല്യം;
    സ്ഥിരം അക്ഷരം *z = p_വ്യഞ്ജകം->u.zToken;
    പ്രതിജ്ഞാ( z!=0 );
    c = sqlite3DecOrHexToI64(z, &മൂല്യം);
    എങ്കിൽ( (c==3 && !negFlag) || (c==2) || (negFlag && മൂല്യം==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_സന്ദേശം(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_വ്യഞ്ജകം);
#അല്ലെങ്കിൽ
#ifndef SQLITE_OMIT_HEX_INTEGER
      എങ്കിൽ( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_സന്ദേശം(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_വ്യഞ്ജകം);
      }അല്ലെങ്കിൽ
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( negFlag ){ മൂല്യം = c==3 ? SMALLEST_INT64 : -മൂല്യം; }
      sqlite3VdbeAddInt64(v, iMem, മൂല്യം);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  പൂർണ്ണസംഖ്യ iTabCur,    /* Cursor pointing to a table row */
  പൂർണ്ണസംഖ്യ iIdxCol,    /* The column of the index to be loaded */
  പൂർണ്ണസംഖ്യ regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  എങ്കിൽ( iTabCol==XN_വ്യഞ്ജകം ){
    പ്രതിജ്ഞാ( pIdx->a_Col_വ്യഞ്ജകം );
    പ്രതിജ്ഞാ( pIdx->a_Col_വ്യഞ്ജകം->n_വ്യഞ്ജകം>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_വ്യഞ്ജകം_Code_Copy(pParse, pIdx->a_Col_വ്യഞ്ജകം->a[iIdxCol].p_വ്യഞ്ജകം, regOut);
    pParse->iSelfTab = 0;
  }അല്ലെങ്കിൽ{
    sqlite3_വ്യഞ്ജകം_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  പൂർണ്ണസംഖ്യ regOut         /* Put the result in this register */
){
  പൂർണ്ണസംഖ്യ iAddr;
  Vdbe *v = pParse->pVdbe;
  പൂർണ്ണസംഖ്യ nErr = pParse->nErr;
  പ്രതിജ്ഞാ( v!=0 );
  പ്രതിജ്ഞാ( pParse->iSelfTab!=0 );
  എങ്കിൽ( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }അല്ലെങ്കിൽ{
    iAddr = 0;
  }
  sqlite3_വ്യഞ്ജകം_Code_Copy(pParse, sqlite3_Column_വ്യഞ്ജകം(pTab,pCol), regOut);
  എങ്കിൽ( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    പൂർണ്ണസംഖ്യ p3 = 2+(പൂർണ്ണസംഖ്യ)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (അക്ഷരം*)pTab, P4_TABLE);
  }അല്ലെങ്കിൽ എങ്കിൽ( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  എങ്കിൽ( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  എങ്കിൽ( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  പൂർണ്ണസംഖ്യ iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  പൂർണ്ണസംഖ്യ iCol,       /* Index of the column to extract */
  പൂർണ്ണസംഖ്യ regOut      /* Extract the value into this register */
){
  Column *pCol;
  പ്രതിജ്ഞാ( v!=0 );
  പ്രതിജ്ഞാ( pTab!=0 );
  പ്രതിജ്ഞാ( iCol!=XN_വ്യഞ്ജകം );
  എങ്കിൽ( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ x;
    എങ്കിൽ( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      തിരികെ;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }അല്ലെങ്കിൽ എങ്കിൽ( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      എങ്കിൽ( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_സന്ദേശം(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_വ്യഞ്ജകം_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      തിരികെ;
#endif
    }അല്ലെങ്കിൽ എങ്കിൽ( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_താക്കോൽ_Index(pTab), iCol);
    }അല്ലെങ്കിൽ{
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  പൂർണ്ണസംഖ്യ iColumn,     /* Index of the table column */
  പൂർണ്ണസംഖ്യ iTable,      /* The cursor pointing to the table */
  പൂർണ്ണസംഖ്യ iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  പ്രതിജ്ഞാ( pParse->pVdbe!=0 );
  പ്രതിജ്ഞാ( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  പ്രതിജ്ഞാ( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_വ്യഞ്ജകം_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  എങ്കിൽ( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    എങ്കിൽ( pOp->opcode==OP_Column ) pOp->p5 = p5;
    എങ്കിൽ( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  തിരികെ iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Code_Move(Parse *pParse, പൂർണ്ണസംഖ്യ iFrom, പൂർണ്ണസംഖ്യ iTo, പൂർണ്ണസംഖ്യ nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_To_Register(വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ iReg){
  വ്യഞ്ജകം *p = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(p_വ്യഞ്ജകം);
  എങ്കിൽ( NEVER(p==0) ) തിരികെ;
  എങ്കിൽ( p->op==TK_REGISTER ){
    പ്രതിജ്ഞാ( p->iTable==iReg );
  }അല്ലെങ്കിൽ{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    വ്യഞ്ജകം_Clear_Property(p, EP_Skip);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Code_Vector(Parse *pParse, വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ *piFreeable){
  പൂർണ്ണസംഖ്യ iResult;
  പൂർണ്ണസംഖ്യ nResult = sqlite3_വ്യഞ്ജകം_Vector_Size(p);
  എങ്കിൽ( nResult==1 ){
    iResult = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p, piFreeable);
  }അല്ലെങ്കിൽ{
    *piFreeable = 0;
    എങ്കിൽ( p->op==TK_SELECT ){
#എങ്കിൽ SQLITE_OMIT_SUBQUERY
      iResult = 0;
#അല്ലെങ്കിൽ
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p) );
      വേണ്ടി(i=0; i<nResult; i++){
        sqlite3_വ്യഞ്ജകം_Code_Factorable(pParse, p->x.pList->a[i].p_വ്യഞ്ജകം, i+iResult);
      }
    }
  }
  തിരികെ iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
സ്ഥിതം ശൂന്യം setDoNotMergeFlagOnCopy(Vdbe *v){
  എങ്കിൽ( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  വ്യഞ്ജകം_List *pFarg,      /* List of function arguments */
  പൂർണ്ണസംഖ്യ iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  പൂർണ്ണസംഖ്യ target            /* Store function result in this register */
){
  പൂർണ്ണസംഖ്യ nFarg;
  Vdbe *v = pParse->pVdbe;
  പ്രതിജ്ഞാ( v!=0 );
  പ്രതിജ്ഞാ( pFarg!=0 );
  nFarg = pFarg->n_വ്യഞ്ജകം;
  പ്രതിജ്ഞാ( nFarg>0 );  /* All in-line functions have at least one argument */
  തിരഞ്ഞെടുപ്പ്( iFuncId ){
    അവസ്ഥ INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      പൂർണ്ണസംഖ്യ endCoalesce = sqlite3VdbeMakeLabel(pParse);
      പൂർണ്ണസംഖ്യ i;
      പ്രതിജ്ഞാ( nFarg>=2 );
      sqlite3_വ്യഞ്ജകം_Code(pParse, pFarg->a[0].p_വ്യഞ്ജകം, target);
      വേണ്ടി(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_വ്യഞ്ജകം_Code(pParse, pFarg->a[i].p_വ്യഞ്ജകം, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      നിർത്തുക;
    }
    അവസ്ഥ INLINEFUNC_iif: {
      വ്യഞ്ജകം case_വ്യഞ്ജകം;
      memset(&case_വ്യഞ്ജകം, 0, വലുപ്പം(case_വ്യഞ്ജകം));
      case_വ്യഞ്ജകം.op = TK_CASE;
      case_വ്യഞ്ജകം.x.pList = pFarg;
      തിരികെ sqlite3_വ്യഞ്ജകം_Code_Target(pParse, &case_വ്യഞ്ജകം, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    അവസ്ഥ INLINEFUNC_sqlite_offset: {
      വ്യഞ്ജകം *pArg = pFarg->a[0].p_വ്യഞ്ജകം;
      എങ്കിൽ( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }അല്ലെങ്കിൽ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      നിർത്തുക;
    }
#endif
    സ്ഥിരസ്ഥിതി: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      പ്രതിജ്ഞാ( nFarg==1 || nFarg==2 );
      target = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, pFarg->a[0].p_വ്യഞ്ജകം, target);
      നിർത്തുക;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#എങ്കിൽ !defined(SQLITE_UNTESTABLE)
    അവസ്ഥ INLINEFUNC_വ്യഞ്ജകം_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      പ്രതിജ്ഞാ( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_വ്യഞ്ജകം_Compare(0,pFarg->a[0].p_വ്യഞ്ജകം, pFarg->a[1].p_വ്യഞ്ജകം,-1),
         target);
      നിർത്തുക;
    }

    അവസ്ഥ INLINEFUNC_വ്യഞ്ജകം_implies_വ്യഞ്ജകം: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      പ്രതിജ്ഞാ( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_വ്യഞ്ജകം_Implies_വ്യഞ്ജകം(pParse,pFarg->a[0].p_വ്യഞ്ജകം, pFarg->a[1].p_വ്യഞ്ജകം,-1),
         target);
      നിർത്തുക;
    }

    അവസ്ഥ INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      വ്യഞ്ജകം *pA1;
      പ്രതിജ്ഞാ( nFarg==2 );
      pA1 = pFarg->a[1].p_വ്യഞ്ജകം;
      എങ്കിൽ( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_വ്യഞ്ജകം_Implies_Non_Null_Row(pFarg->a[0].p_വ്യഞ്ജകം,pA1->iTable,1),
           target);
      }അല്ലെങ്കിൽ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      നിർത്തുക;
    }

    അവസ്ഥ INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      സ്ഥിരം അക്ഷരം *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      അക്ഷരം aff;
      പ്രതിജ്ഞാ( nFarg==1 );
      aff = sqlite3_വ്യഞ്ജകം_Affinity(pFarg->a[0].p_വ്യഞ്ജകം);
      പ്രതിജ്ഞാ( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      നിർത്തുക;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  തിരികെ target;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_കണ്ണി_Can_Return_Subtype(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പൂർണ്ണസംഖ്യ n;
  FuncDef *pDef;
  sqlite3 *db;
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_CASE || p_വ്യഞ്ജകം->op==TK_UPLUS 
   || p_വ്യഞ്ജകം->op==TK_COLLATE || p_വ്യഞ്ജകം->op==TK_CAST 
  ){
    തിരികെ WRC_Continue;
  }
  എങ്കിൽ( p_വ്യഞ്ജകം->op!=TK_FUNCTION && p_വ്യഞ്ജകം->op!=TK_AGG_FUNCTION ){
    തിരികെ WRC_Prune;
  }
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
  db = pWalker->pParse->db;
  n = p_വ്യഞ്ജകം->x.pList ? p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം : 0;
  pDef = sqlite3FindFunction(db, p_വ്യഞ്ജകം->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  എങ്കിൽ( pDef==0 ) തിരികെ WRC_Continue;
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( pDef!=0 );
#endif
  എങ്കിൽ( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    തിരികെ WRC_Abort;
  }
  തിരികെ WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Can_Return_Subtype(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  Walker w;
  memset(&w, 0, വലുപ്പം(w));
  w.pParse = pParse;
  w.x_വ്യഞ്ജകം_Callback = വ്യഞ്ജകം_കണ്ണി_Can_Return_Subtype;
  sqlite3_Walk_വ്യഞ്ജകം(&w, p_വ്യഞ്ജകം);
  തിരികെ w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ sqlite3_Indexed_വ്യഞ്ജകം_Lookup(
  Parse *pParse,   /* The parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,     /* The expression to potentially bypass */
  പൂർണ്ണസംഖ്യ target       /* Where to store the result of the expression */
){
  Indexed_വ്യഞ്ജകം *p;
  Vdbe *v;
  വേണ്ടി(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 വ്യഞ്ജകം_Aff;
    പൂർണ്ണസംഖ്യ iDataCur = p->iDataCur;
    എങ്കിൽ( iDataCur<0 ) തുടരുക;
    എങ്കിൽ( pParse->iSelfTab ){
      എങ്കിൽ( p->iDataCur!=pParse->iSelfTab-1 ) തുടരുക;
      iDataCur = -1;
    }
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0, p_വ്യഞ്ജകം, p->p_വ്യഞ്ജകം, iDataCur)!=0 ) തുടരുക;
    പ്രതിജ്ഞാ( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    വ്യഞ്ജകം_Aff = sqlite3_വ്യഞ്ജകം_Affinity(p_വ്യഞ്ജകം);
    എങ്കിൽ( (വ്യഞ്ജകം_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (വ്യഞ്ജകം_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (വ്യഞ്ജകം_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      തുടരുക;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_SubtArg)
     && sqlite3_വ്യഞ്ജകം_Can_Return_Subtype(pParse, p_വ്യഞ്ജകം) 
    ){
      തുടരുക;
    }

    v = pParse->pVdbe;
    പ്രതിജ്ഞാ( v!=0 );
    എങ്കിൽ( p->bMaybeNullRow || sqlite3_വ്യഞ്ജകം_Is_Constant(pParse, p_വ്യഞ്ജകം) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      പൂർണ്ണസംഖ്യ addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_വ്യഞ്ജകം_Code(pParse, p_വ്യഞ്ജകം, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }അല്ലെങ്കിൽ{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    തിരികെ target;
  }
  തിരികെ -1;  /* Not found */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Partidx_വ്യഞ്ജകം_Lookup(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ iTarget){
  Indexed_വ്യഞ്ജകം *p;
  വേണ്ടി(p=pParse->p_Idx_Part_വ്യഞ്ജകം; p; p=p->pIENext){
    എങ്കിൽ( p_വ്യഞ്ജകം->iColumn==p->iIdxCol && p_വ്യഞ്ജകം->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      പൂർണ്ണസംഖ്യ addr = 0;
      പൂർണ്ണസംഖ്യ ret;

      എങ്കിൽ( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, p->p_വ്യഞ്ജകം, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (സ്ഥിരം അക്ഷരം*)&p->aff, 1);
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Can_Return_Subtype(pParse, p->p_വ്യഞ്ജകം) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      എങ്കിൽ( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      തിരികെ ret;
    }
  }
  തിരികെ 0;
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
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,       /* AND or OR expression to be coded */
  പൂർണ്ണസംഖ്യ target,        /* Put result in this register, guaranteed */
  പൂർണ്ണസംഖ്യ *pTmpReg       /* Write a temporary register here */
){
  പൂർണ്ണസംഖ്യ op;            /* The opcode.  TK_AND or TK_OR */
  പൂർണ്ണസംഖ്യ skipOp;        /* Opcode for the branch that skips one operand */
  പൂർണ്ണസംഖ്യ addrSkip;      /* Branch instruction that skips one of the operands */
  പൂർണ്ണസംഖ്യ regSS = 0;     /* Register holding computed operand when other omitted */
  പൂർണ്ണസംഖ്യ r1, r2;        /* Registers for left and right operands, respectively */
  വ്യഞ്ജകം *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 );
  op = p_വ്യഞ്ജകം->op;
  പ്രതിജ്ഞാ( op==TK_AND || op==TK_OR );
  പ്രതിജ്ഞാ( TK_AND==OP_And );            testcase( op==TK_AND );
  പ്രതിജ്ഞാ( TK_OR==OP_Or );              testcase( op==TK_OR );
  പ്രതിജ്ഞാ( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_വ്യഞ്ജകം_Simplified_And_Or(p_വ്യഞ്ജകം);
  എങ്കിൽ( pAlt!=p_വ്യഞ്ജകം ){
    r1 = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    തിരികെ target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  എങ്കിൽ( വ്യഞ്ജകം_Eval_Rhs_First(p_വ്യഞ്ജകം) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, p_വ്യഞ്ജകം->p_വലത്, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, pTmpReg);
  }അല്ലെങ്കിൽ{
    /* Compute the left operand first */
    r1 = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, p_വ്യഞ്ജകം->p_ഇടത്, target);
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം->p_വലത്, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }അല്ലെങ്കിൽ{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_വലത്, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  എങ്കിൽ( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  തിരികെ target;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Code_Target(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  പൂർണ്ണസംഖ്യ op;                   /* The opcode being coded */
  പൂർണ്ണസംഖ്യ inReg = target;       /* Results stored in register inReg */
  പൂർണ്ണസംഖ്യ regFree1 = 0;         /* If non-zero free this temporary register */
  പൂർണ്ണസംഖ്യ regFree2 = 0;         /* If non-zero free this temporary register */
  പൂർണ്ണസംഖ്യ r1, r2;               /* Various register numbers */
  വ്യഞ്ജകം tempX;               /* Temporary expression node */
  പൂർണ്ണസംഖ്യ p5 = 0;

  പ്രതിജ്ഞാ( target>0 && target<=pParse->nMem );
  പ്രതിജ്ഞാ( v!=0 );

വ്യഞ്ജകം_code_doover:
  എങ്കിൽ( p_വ്യഞ്ജകം==0 ){
    op = TK_NULL;
  }അല്ലെങ്കിൽ എങ്കിൽ( pParse->pIdxEpr!=0
   && !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Leaf)
   && (r1 = sqlite3_Indexed_വ്യഞ്ജകം_Lookup(pParse, p_വ്യഞ്ജകം, target))>=0
  ){
    തിരികെ r1;
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_VVAProperty(p_വ്യഞ്ജകം,EP_Immutable) );
    op = p_വ്യഞ്ജകം->op;
  }
  പ്രതിജ്ഞാ( op!=TK_ORDER );
  തിരഞ്ഞെടുപ്പ്( op ){
    അവസ്ഥ TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_വ്യഞ്ജകം->pAggInfo;
      ഘടന AggInfo_col *pCol;
      പ്രതിജ്ഞാ( pAggInfo!=0 );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->iAgg>=0 );
      എങ്കിൽ( p_വ്യഞ്ജകം->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        നിർത്തുക;
      }
      pCol = &pAggInfo->aCol[p_വ്യഞ്ജകം->iAgg];
      എങ്കിൽ( !pAggInfo->directMode ){
        തിരികെ AggInfoColumnReg(pAggInfo, p_വ്യഞ്ജകം->iAgg);
      }അല്ലെങ്കിൽ എങ്കിൽ( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        എങ്കിൽ( pTab==0 ){
          /* No comment added */
        }അല്ലെങ്കിൽ എങ്കിൽ( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }അല്ലെങ്കിൽ{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          എങ്കിൽ( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        തിരികെ target;
      }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_വ്യഞ്ജകം->iTable, p_വ്യഞ്ജകം->iColumn, target);
        തിരികെ target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    അവസ്ഥ TK_COLUMN: {
      പൂർണ്ണസംഖ്യ iTab = p_വ്യഞ്ജകം->iTable;
      പൂർണ്ണസംഖ്യ iReg;
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        പൂർണ്ണസംഖ്യ aff;
        iReg = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, p_വ്യഞ്ജകം->p_ഇടത്,target);
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_വ്യഞ്ജകം->y.pTab, p_വ്യഞ്ജകം->iColumn);
        എങ്കിൽ( aff>SQLITE_AFF_BLOB ){
          സ്ഥിതം സ്ഥിരം അക്ഷരം zAff[] = "B\000C\000D\000E\000F";
          പ്രതിജ്ഞാ( SQLITE_AFF_BLOB=='A' );
          പ്രതിജ്ഞാ( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        തിരികെ iReg;
      }
      എങ്കിൽ( iTab<0 ){
        എങ്കിൽ( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          പൂർണ്ണസംഖ്യ iSrc;
          പൂർണ്ണസംഖ്യ iCol = p_വ്യഞ്ജകം->iColumn;
          പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
          pTab = p_വ്യഞ്ജകം->y.pTab;
          പ്രതിജ്ഞാ( pTab!=0 );
          പ്രതിജ്ഞാ( iCol>=XN_ROWID );
          പ്രതിജ്ഞാ( iCol<pTab->nCol );
          എങ്കിൽ( iCol<0 ){
            തിരികെ -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          എങ്കിൽ( pCol->colFlags & COLFLAG_GENERATED ){
            എങ്കിൽ( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_സന്ദേശം(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              തിരികെ 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            എങ്കിൽ( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_വ്യഞ്ജകം_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            തിരികെ iSrc;
          }അല്ലെങ്കിൽ
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          എങ്കിൽ( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            തിരികെ target;
          }അല്ലെങ്കിൽ{
            തിരികെ iSrc;
          }
        }അല്ലെങ്കിൽ{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      അല്ലെങ്കിൽ എങ്കിൽ( pParse->p_Idx_Part_വ്യഞ്ജകം 
       && 0!=(r1 = വ്യഞ്ജകം_Partidx_വ്യഞ്ജകം_Lookup(pParse, p_വ്യഞ്ജകം, target))
      ){
        തിരികെ r1;
      }
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->y.pTab!=0 );
      iReg = sqlite3_വ്യഞ്ജകം_Code_Get_Column(pParse, p_വ്യഞ്ജകം->y.pTab,
                               p_വ്യഞ്ജകം->iColumn, iTab, target,
                               p_വ്യഞ്ജകം->op2);
      തിരികെ iReg;
    }
    അവസ്ഥ TK_INTEGER: {
      codeInteger(pParse, p_വ്യഞ്ജകം, 0, target);
      തിരികെ target;
    }
    അവസ്ഥ TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_വ്യഞ്ജകം_Truth_മൂല്യം(p_വ്യഞ്ജകം), target);
      തിരികെ target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    അവസ്ഥ TK_FLOAT: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      codeReal(v, p_വ്യഞ്ജകം->u.zToken, 0, target);
      തിരികെ target;
    }
#endif
    അവസ്ഥ TK_STRING: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3VdbeLoadString(v, target, p_വ്യഞ്ജകം->u.zToken);
      തിരികെ target;
    }
    അവസ്ഥ TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_വ്യഞ്ജകം->y.nReg - 1);
      തിരികെ target;
    }
    സ്ഥിരസ്ഥിതി: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      പ്രതിജ്ഞാ( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      തിരികെ target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    അവസ്ഥ TK_BLOB: {
      പൂർണ്ണസംഖ്യ n;
      സ്ഥിരം അക്ഷരം *z;
      അക്ഷരം *zBlob;
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->u.zToken[0]=='x' || p_വ്യഞ്ജകം->u.zToken[0]=='X' );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->u.zToken[1]=='\'' );
      z = &p_വ്യഞ്ജകം->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      പ്രതിജ്ഞാ( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      തിരികെ target;
    }
#endif
    അവസ്ഥ TK_VARIABLE: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->u.zToken!=0 );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_വ്യഞ്ജകം->iColumn, target);
      തിരികെ target;
    }
    അവസ്ഥ TK_REGISTER: {
      തിരികെ p_വ്യഞ്ജകം->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    അവസ്ഥ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_വ്യഞ്ജകം_Code(pParse, p_വ്യഞ്ജകം->p_ഇടത്, target);
      പ്രതിജ്ഞാ( inReg==target );
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_വ്യഞ്ജകം->u.zToken, 0));
      തിരികെ inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    അവസ്ഥ TK_IS:
    അവസ്ഥ TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    അവസ്ഥ TK_LT:
    അവസ്ഥ TK_LE:
    അവസ്ഥ TK_GT:
    അവസ്ഥ TK_GE:
    അവസ്ഥ TK_NE:
    അവസ്ഥ TK_EQ: {
      വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
      പൂർണ്ണസംഖ്യ addrIsNull = 0;
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_Vector(p_ഇടത്) ){
        codeVectorCompare(pParse, p_വ്യഞ്ജകം, target, op, p5);
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = വ്യഞ്ജകം_Compute_Operands(pParse, p_വ്യഞ്ജകം,
                                     &r1, &r2, &regFree1, &regFree2);
        }അല്ലെങ്കിൽ{
          r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
          r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_വലത്, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_ഇടത്, p_വ്യഞ്ജകം->p_വലത്, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_Commuted));
        പ്രതിജ്ഞാ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        പ്രതിജ്ഞാ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        പ്രതിജ്ഞാ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        പ്രതിജ്ഞാ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        പ്രതിജ്ഞാ(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        പ്രതിജ്ഞാ(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        എങ്കിൽ( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }അല്ലെങ്കിൽ{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          എങ്കിൽ( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_AND:
    അവസ്ഥ TK_OR: {
      inReg = വ്യഞ്ജകം_Code_Target_And_Or(pParse, p_വ്യഞ്ജകം, target, &regFree1);
      നിർത്തുക;
    }
    അവസ്ഥ TK_PLUS:
    അവസ്ഥ TK_STAR:
    അവസ്ഥ TK_MINUS:
    അവസ്ഥ TK_REM:
    അവസ്ഥ TK_BITAND:
    അവസ്ഥ TK_BITOR:
    അവസ്ഥ TK_SLASH:
    അവസ്ഥ TK_LSHIFT:
    അവസ്ഥ TK_RSHIFT:
    അവസ്ഥ TK_CONCAT: {
      പൂർണ്ണസംഖ്യ addrIsNull;
      പ്രതിജ്ഞാ( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      പ്രതിജ്ഞാ( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      പ്രതിജ്ഞാ( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      പ്രതിജ്ഞാ( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      പ്രതിജ്ഞാ( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      പ്രതിജ്ഞാ( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      പ്രതിജ്ഞാ( TK_LSHIFT==OP_Shift_ഇടത് );   testcase( op==TK_LSHIFT );
      പ്രതിജ്ഞാ( TK_RSHIFT==OP_Shift_വലത് );  testcase( op==TK_RSHIFT );
      പ്രതിജ്ഞാ( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Subquery) ){
        addrIsNull = വ്യഞ്ജകം_Compute_Operands(pParse, p_വ്യഞ്ജകം,
                                   &r1, &r2, &regFree1, &regFree2);
      }അല്ലെങ്കിൽ{
        r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
        r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_വലത്, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      എങ്കിൽ( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_UMINUS: {
      വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
      പ്രതിജ്ഞാ( p_ഇടത് );
      എങ്കിൽ( p_ഇടത്->op==TK_INTEGER ){
        codeInteger(pParse, p_ഇടത്, 1, target);
        തിരികെ target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }അല്ലെങ്കിൽ എങ്കിൽ( p_ഇടത്->op==TK_FLOAT ){
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
        codeReal(v, p_ഇടത്->u.zToken, 1, target);
        തിരികെ target;
#endif
      }അല്ലെങ്കിൽ{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_മൂല്യം|EP_TokenOnly;
        tempX.u.i_മൂല്യം = 0;
        വ്യഞ്ജകം_Clear_VVAProperties(&tempX);
        r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_BITNOT:
    അവസ്ഥ TK_NOT: {
      പ്രതിജ്ഞാ( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      പ്രതിജ്ഞാ( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      നിർത്തുക;
    }
    അവസ്ഥ TK_TRUTH: {
      പൂർണ്ണസംഖ്യ isTrue;    /* IS TRUE or IS NOT TRUE */
      പൂർണ്ണസംഖ്യ bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_വ്യഞ്ജകം_Truth_മൂല്യം(p_വ്യഞ്ജകം->p_വലത്);
      bNormal = p_വ്യഞ്ജകം->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      നിർത്തുക;
    }
    അവസ്ഥ TK_ISNULL:
    അവസ്ഥ TK_NOTNULL: {
      പൂർണ്ണസംഖ്യ addr;
      പ്രതിജ്ഞാ( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      പ്രതിജ്ഞാ( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      നിർത്തുക;
    }
    അവസ്ഥ TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_വ്യഞ്ജകം->pAggInfo;
      എങ്കിൽ( pInfo==0
       || NEVER(p_വ്യഞ്ജകം->iAgg<0)
       || NEVER(p_വ്യഞ്ജകം->iAgg>=pInfo->nFunc)
      ){
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
        sqlite3_Error_സന്ദേശം(pParse, "misuse of aggregate: %#T()", p_വ്യഞ്ജകം);
      }അല്ലെങ്കിൽ{
        തിരികെ AggInfoFuncReg(pInfo, p_വ്യഞ്ജകം->iAgg);
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_FUNCTION: {
      വ്യഞ്ജകം_List *pFarg;       /* List of function arguments */
      പൂർണ്ണസംഖ്യ nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      സ്ഥിരം അക്ഷരം *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      പൂർണ്ണസംഖ്യ i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
        തിരികെ p_വ്യഞ്ജകം->y.pWin->regResult;
      }
#endif

      എങ്കിൽ( ConstFactorOk(pParse)
       && sqlite3_വ്യഞ്ജകം_Is_Constant_Not_Join(pParse,p_വ്യഞ്ജകം)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        തിരികെ sqlite3_വ്യഞ്ജകം_Code_Run_Just_Once(pParse, p_വ്യഞ്ജകം, -1);
      }
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly) );
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
      pFarg = p_വ്യഞ്ജകം->x.pList;
      nFarg = pFarg ? pFarg->n_വ്യഞ്ജകം : 0;
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      zId = p_വ്യഞ്ജകം->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      എങ്കിൽ( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      എങ്കിൽ( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_സന്ദേശം(pParse, "unknown function: %#T()", p_വ്യഞ്ജകം);
        നിർത്തുക;
      }
      എങ്കിൽ( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        പ്രതിജ്ഞാ( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        പ്രതിജ്ഞാ( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        തിരികെ വ്യഞ്ജകം_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }അല്ലെങ്കിൽ എങ്കിൽ( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_വ്യഞ്ജകം_Function_Usable(pParse, p_വ്യഞ്ജകം, pDef);
      }

      വേണ്ടി(i=0; i<nFarg; i++){
        എങ്കിൽ( i<32 && sqlite3_വ്യഞ്ജകം_Is_Constant(pParse, pFarg->a[i].p_വ്യഞ്ജകം) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        എങ്കിൽ( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, pFarg->a[i].p_വ്യഞ്ജകം);
        }
      }
      എങ്കിൽ( pFarg ){
        എങ്കിൽ( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }അല്ലെങ്കിൽ{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        എങ്കിൽ( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 വ്യഞ്ജകം_Op;
          പ്രതിജ്ഞാ( nFarg==1 );
          പ്രതിജ്ഞാ( pFarg->a[0].p_വ്യഞ്ജകം!=0 );
          വ്യഞ്ജകം_Op = pFarg->a[0].p_വ്യഞ്ജകം->op;
          എങ്കിൽ( വ്യഞ്ജകം_Op==TK_COLUMN || വ്യഞ്ജകം_Op==TK_AGG_COLUMN ){
            പ്രതിജ്ഞാ( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            പ്രതിജ്ഞാ( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            പ്രതിജ്ഞാ( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            പ്രതിജ്ഞാ( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_വ്യഞ്ജകം->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_വ്യഞ്ജകം_Code_വ്യഞ്ജകം_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }അല്ലെങ്കിൽ{
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
      എങ്കിൽ( nFarg>=2 && വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_വ്യഞ്ജകം);
      }അല്ലെങ്കിൽ എങ്കിൽ( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_വ്യഞ്ജകം);
      }
#endif
      എങ്കിൽ( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        എങ്കിൽ( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (അക്ഷരം *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_വ്യഞ്ജകം->op2);
      എങ്കിൽ( nFarg ){
        എങ്കിൽ( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }അല്ലെങ്കിൽ{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      തിരികെ target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    അവസ്ഥ TK_EXISTS:
    അവസ്ഥ TK_SELECT: {
      പൂർണ്ണസംഖ്യ nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      എങ്കിൽ( pParse->db->mallocFailed ){
        തിരികെ 0;
      }അല്ലെങ്കിൽ എങ്കിൽ( op==TK_SELECT
             && ALWAYS( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) )
             && (nCol = p_വ്യഞ്ജകം->x.pSelect->pEList->n_വ്യഞ്ജകം)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }അല്ലെങ്കിൽ{
        തിരികെ sqlite3CodeSubselect(pParse, p_വ്യഞ്ജകം);
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_SELECT_COLUMN: {
      പൂർണ്ണസംഖ്യ n;
      വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
      എങ്കിൽ( p_ഇടത്->iTable==0 || pParse->withinRJSubrtn > p_ഇടത്->op2 ){
        p_ഇടത്->iTable = sqlite3CodeSubselect(pParse, p_ഇടത്);
        p_ഇടത്->op2 = pParse->withinRJSubrtn;
      }
      പ്രതിജ്ഞാ( p_ഇടത്->op==TK_SELECT || p_ഇടത്->op==TK_ERROR );
      n = sqlite3_വ്യഞ്ജകം_Vector_Size(p_ഇടത്);
      എങ്കിൽ( p_വ്യഞ്ജകം->iTable!=n ){
        sqlite3_Error_സന്ദേശം(pParse, "%d columns assigned %d values",
                                p_വ്യഞ്ജകം->iTable, n);
      }
      തിരികെ p_ഇടത്->iTable + p_വ്യഞ്ജകം->iColumn;
    }
    അവസ്ഥ TK_IN: {
      പൂർണ്ണസംഖ്യ destIfFalse = sqlite3VdbeMakeLabel(pParse);
      പൂർണ്ണസംഖ്യ destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_വ്യഞ്ജകം_Code_IN(pParse, p_വ്യഞ്ജകം, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      തിരികെ target;
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
    അവസ്ഥ TK_BETWEEN: {
      വ്യഞ്ജകം_Code_Between(pParse, p_വ്യഞ്ജകം, target, 0, 0);
      തിരികെ target;
    }
    അവസ്ഥ TK_COLLATE: {
      എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത് );
        sqlite3_വ്യഞ്ജകം_Code(pParse, p_വ്യഞ്ജകം->p_ഇടത്, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        തിരികെ target;
      }അല്ലെങ്കിൽ{
        p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
        പോകുക വ്യഞ്ജകം_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    അവസ്ഥ TK_SPAN:
    അവസ്ഥ TK_UPLUS: {
      p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
      പോകുക വ്യഞ്ജകം_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    അവസ്ഥ TK_TRIGGER: {
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
      പൂർണ്ണസംഖ്യ iCol;
      പൂർണ്ണസംഖ്യ p1;

      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
      pTab = p_വ്യഞ്ജകം->y.pTab;
      iCol = p_വ്യഞ്ജകം->iColumn;
      p1 = p_വ്യഞ്ജകം->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->iTable==0 || p_വ്യഞ്ജകം->iTable==1 );
      പ്രതിജ്ഞാ( iCol>=-1 && iCol<pTab->nCol );
      പ്രതിജ്ഞാ( pTab->iPKey<0 || iCol!=pTab->iPKey );
      പ്രതിജ്ഞാ( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_വ്യഞ്ജകം->iTable ? "new" : "old"),
        (p_വ്യഞ്ജകം->iColumn<0 ? "rowid" : p_വ്യഞ്ജകം->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      എങ്കിൽ( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      നിർത്തുക;
    }

    അവസ്ഥ TK_VECTOR: {
      sqlite3_Error_സന്ദേശം(pParse, "row value misused");
      നിർത്തുക;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    അവസ്ഥ TK_IF_NULL_ROW: {
      പൂർണ്ണസംഖ്യ addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_വ്യഞ്ജകം->pAggInfo;
      എങ്കിൽ( pAggInfo ){
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->iAgg>=0 && p_വ്യഞ്ജകം->iAgg<pAggInfo->nColumn );
        എങ്കിൽ( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_വ്യഞ്ജകം->iAgg);
          നിർത്തുക;
        }
        എങ്കിൽ( p_വ്യഞ്ജകം->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_വ്യഞ്ജകം->iAgg].iSorterColumn,
                            target);
          inReg = target;
          നിർത്തുക;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_വ്യഞ്ജകം->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_വ്യഞ്ജകം_Code(pParse, p_വ്യഞ്ജകം->p_ഇടത്, target);
      പ്രതിജ്ഞാ( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      നിർത്തുക;
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
    അവസ്ഥ TK_CASE: {
      പൂർണ്ണസംഖ്യ endLabel;                     /* GOTO label for end of CASE stmt */
      പൂർണ്ണസംഖ്യ nextCase;                     /* GOTO label for next WHEN clause */
      പൂർണ്ണസംഖ്യ n_വ്യഞ്ജകം;                        /* 2x number of WHEN terms */
      പൂർണ്ണസംഖ്യ i;                            /* Loop counter */
      വ്യഞ്ജകം_List *pEList;                 /* List of WHEN terms */
      ഘടന വ്യഞ്ജകം_List_item *aListelem;  /* Array of WHEN terms */
      വ്യഞ്ജകം opCompare;                   /* The X==Ei expression */
      വ്യഞ്ജകം *pX;                         /* The X expression */
      വ്യഞ്ജകം *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      വ്യഞ്ജകം *pDel = 0;
      sqlite3 *db = pParse->db;

      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) && p_വ്യഞ്ജകം->x.pList!=0 );
      പ്രതിജ്ഞാ(p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം > 0);
      pEList = p_വ്യഞ്ജകം->x.pList;
      aListelem = pEList->a;
      n_വ്യഞ്ജകം = pEList->n_വ്യഞ്ജകം;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      എങ്കിൽ( (pX = p_വ്യഞ്ജകം->p_ഇടത്)!=0 ){
        pDel = sqlite3_വ്യഞ്ജകം_Dup(db, pX, 0);
        എങ്കിൽ( db->mallocFailed ){
          sqlite3_വ്യഞ്ജകം_Delete(db, pDel);
          നിർത്തുക;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_വ്യഞ്ജകം_To_Register(pDel, വ്യഞ്ജകം_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, വലുപ്പം(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_ഇടത് = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      വേണ്ടി(i=0; i<n_വ്യഞ്ജകം-1; i=i+2){
        എങ്കിൽ( pX ){
          പ്രതിജ്ഞാ( pTest!=0 );
          opCompare.p_വലത് = aListelem[i].p_വ്യഞ്ജകം;
        }അല്ലെങ്കിൽ{
          pTest = aListelem[i].p_വ്യഞ്ജകം;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_വ്യഞ്ജകം_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_വ്യഞ്ജകം->op==TK_COLUMN );
        sqlite3_വ്യഞ്ജകം_Code(pParse, aListelem[i+1].p_വ്യഞ്ജകം, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      എങ്കിൽ( (n_വ്യഞ്ജകം&1)!=0 ){
        sqlite3_വ്യഞ്ജകം_Code(pParse, pEList->a[n_വ്യഞ്ജകം-1].p_വ്യഞ്ജകം, target);
      }അല്ലെങ്കിൽ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_വ്യഞ്ജകം_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      നിർത്തുക;
    }
#ifndef SQLITE_OMIT_TRIGGER
    അവസ്ഥ TK_RAISE: {
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം==OE_Rollback
           || p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം==OE_Abort
           || p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം==OE_Fail
           || p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം==OE_Ignore
      );
      എങ്കിൽ( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_സന്ദേശം(pParse,
                       "RAISE() may only be used within a trigger-program");
        തിരികെ 0;
      }
      എങ്കിൽ( p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      എങ്കിൽ( p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }അല്ലെങ്കിൽ{
        r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം, r1);
      }
      നിർത്തുക;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  തിരികെ inReg;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,      /* The expression to code when the VDBE initializes */
  പൂർണ്ണസംഖ്യ regDest       /* Store the value in this register */
){
  വ്യഞ്ജകം_List *p;
  പ്രതിജ്ഞാ( ConstFactorOk(pParse) );
  പ്രതിജ്ഞാ( regDest!=0 );
  p = pParse->p_Const_വ്യഞ്ജകം;
  എങ്കിൽ( regDest<0 && p ){
    ഘടന വ്യഞ്ജകം_List_item *pItem;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(pItem=p->a, i=p->n_വ്യഞ്ജകം; i>0; pItem++, i--){
      എങ്കിൽ( pItem->fg.reusable
       && sqlite3_വ്യഞ്ജകം_Compare(0,pItem->p_വ്യഞ്ജകം,p_വ്യഞ്ജകം,-1)==0
      ){
        തിരികെ pItem->u.i_Const_വ്യഞ്ജകം_Reg;
      }
    }
  }
  p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Dup(pParse->db, p_വ്യഞ്ജകം, 0);
  എങ്കിൽ( p_വ്യഞ്ജകം!=0 && വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    പൂർണ്ണസംഖ്യ addr;
    പ്രതിജ്ഞാ( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    എങ്കിൽ( !pParse->db->mallocFailed ){
      എങ്കിൽ( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_വ്യഞ്ജകം_Code(pParse, p_വ്യഞ്ജകം, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_വ്യഞ്ജകം_Delete(pParse->db, p_വ്യഞ്ജകം);
    sqlite3VdbeJumpHere(v, addr);
  }അല്ലെങ്കിൽ{
    p = sqlite3_വ്യഞ്ജകം_List_Append(pParse, p, p_വ്യഞ്ജകം);
    എങ്കിൽ( p ){
       ഘടന വ്യഞ്ജകം_List_item *pItem = &p->a[p->n_വ്യഞ്ജകം-1];
       pItem->fg.reusable = regDest<0;
       എങ്കിൽ( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_വ്യഞ്ജകം_Reg = regDest;
    }
    pParse->p_Const_വ്യഞ്ജകം = p;
  }
  തിരികെ regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE ശൂന്യം sqlite3_വ്യഞ്ജകം_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  പൂർണ്ണസംഖ്യ iReg,        /* First register to set to NULL */
  പൂർണ്ണസംഖ്യ nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  വ്യഞ്ജകം t;
  memset(&t, 0, വലുപ്പം(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_വ്യഞ്ജകം_Code_Run_Just_Once(pParse, &t, iReg);
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Code_Temp(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ *pReg){
  പൂർണ്ണസംഖ്യ r2;
  p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(p_വ്യഞ്ജകം);
  എങ്കിൽ( ConstFactorOk(pParse)
   && ALWAYS(p_വ്യഞ്ജകം!=0)
   && p_വ്യഞ്ജകം->op!=TK_REGISTER
   && sqlite3_വ്യഞ്ജകം_Is_Constant_Not_Join(pParse, p_വ്യഞ്ജകം)
  ){
    *pReg  = 0;
    r2 = sqlite3_വ്യഞ്ജകം_Code_Run_Just_Once(pParse, p_വ്യഞ്ജകം, -1);
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, p_വ്യഞ്ജകം, r1);
    എങ്കിൽ( r2==r1 ){
      *pReg = r1;
    }അല്ലെങ്കിൽ{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  തിരികെ r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Code(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ target){
  പൂർണ്ണസംഖ്യ inReg;

  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം==0 || !വ്യഞ്ജകം_Has_VVAProperty(p_വ്യഞ്ജകം,EP_Immutable) );
  പ്രതിജ്ഞാ( target>0 && target<=pParse->nMem );
  പ്രതിജ്ഞാ( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  എങ്കിൽ( pParse->pVdbe==0 ) തിരികെ;
  inReg = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, p_വ്യഞ്ജകം, target);
  എങ്കിൽ( inReg!=target ){
    u8 op;
    വ്യഞ്ജകം *pX = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(p_വ്യഞ്ജകം);
    testcase( pX!=p_വ്യഞ്ജകം );
    എങ്കിൽ( ALWAYS(pX)
     && (വ്യഞ്ജകം_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }അല്ലെങ്കിൽ{
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_Code_Copy(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ target){
  sqlite3 *db = pParse->db;
  p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Dup(db, p_വ്യഞ്ജകം, 0);
  എങ്കിൽ( !db->mallocFailed ) sqlite3_വ്യഞ്ജകം_Code(pParse, p_വ്യഞ്ജകം, target);
  sqlite3_വ്യഞ്ജകം_Delete(db, p_വ്യഞ്ജകം);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Code_Factorable(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ target){
  എങ്കിൽ( pParse->okConstFactor && sqlite3_വ്യഞ്ജകം_Is_Constant_Not_Join(pParse,p_വ്യഞ്ജകം) ){
    sqlite3_വ്യഞ്ജകം_Code_Run_Just_Once(pParse, p_വ്യഞ്ജകം, target);
  }അല്ലെങ്കിൽ{
    sqlite3_വ്യഞ്ജകം_Code_Copy(pParse, p_വ്യഞ്ജകം, target);
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Code_വ്യഞ്ജകം_List(
  Parse *pParse,     /* Parsing context */
  വ്യഞ്ജകം_List *pList,   /* The expression list to be coded */
  പൂർണ്ണസംഖ്യ target,        /* Where to write results */
  പൂർണ്ണസംഖ്യ srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  ഘടന വ്യഞ്ജകം_List_item *pItem;
  പൂർണ്ണസംഖ്യ i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  പ്രതിജ്ഞാ( pList!=0 );
  പ്രതിജ്ഞാ( target>0 );
  പ്രതിജ്ഞാ( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_വ്യഞ്ജകം;
  എങ്കിൽ( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  വേണ്ടി(pItem=pList->a, i=0; i<n; i++, pItem++){
    വ്യഞ്ജകം *p_വ്യഞ്ജകം = pItem->p_വ്യഞ്ജകം;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    എങ്കിൽ( pItem->fg.bSorterRef ){
      i--;
      n--;
    }അല്ലെങ്കിൽ
#endif
    എങ്കിൽ( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      എങ്കിൽ( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }അല്ലെങ്കിൽ{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_വ്യഞ്ജകം_Is_Constant_Not_Join(pParse,p_വ്യഞ്ജകം)
    ){
      sqlite3_വ്യഞ്ജകം_Code_Run_Just_Once(pParse, p_വ്യഞ്ജകം, target+i);
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ inReg = sqlite3_വ്യഞ്ജകം_Code_Target(pParse, p_വ്യഞ്ജകം, target+i);
      എങ്കിൽ( inReg!=target+i ){
        VdbeOp *pOp;
        എങ്കിൽ( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }അല്ലെങ്കിൽ{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  തിരികെ n;
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
സ്ഥിതം ശൂന്യം വ്യഞ്ജകം_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,      /* The BETWEEN expression */
  പൂർണ്ണസംഖ്യ dest,         /* Jump destination or storage location */
  ശൂന്യം (*xJump)(Parse*,വ്യഞ്ജകം*,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ), /* Action to take */
  പൂർണ്ണസംഖ്യ jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  വ്യഞ്ജകം വ്യഞ്ജകം_And;     /* The AND operator in  x>=y AND x<=z  */
  വ്യഞ്ജകം comp_ഇടത്;    /* The  x>=y  term */
  വ്യഞ്ജകം comp_വലത്;   /* The  x<=z  term */
  പൂർണ്ണസംഖ്യ regFree1 = 0; /* Temporary use register */
  വ്യഞ്ജകം *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_ഇടത്, 0, വലുപ്പം(വ്യഞ്ജകം));
  memset(&comp_വലത്, 0, വലുപ്പം(വ്യഞ്ജകം));
  memset(&വ്യഞ്ജകം_And, 0, വലുപ്പം(വ്യഞ്ജകം));

  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
  pDel = sqlite3_വ്യഞ്ജകം_Dup(db, p_വ്യഞ്ജകം->p_ഇടത്, 0);
  എങ്കിൽ( db->mallocFailed==0 ){
    വ്യഞ്ജകം_And.op = TK_AND;
    വ്യഞ്ജകം_And.p_ഇടത് = &comp_ഇടത്;
    വ്യഞ്ജകം_And.p_വലത് = &comp_വലത്;
    comp_ഇടത്.op = TK_GE;
    comp_ഇടത്.p_ഇടത് = pDel;
    comp_ഇടത്.p_വലത് = p_വ്യഞ്ജകം->x.pList->a[0].p_വ്യഞ്ജകം;
    comp_വലത്.op = TK_LE;
    comp_വലത്.p_ഇടത് = pDel;
    comp_വലത്.p_വലത് = p_വ്യഞ്ജകം->x.pList->a[1].p_വ്യഞ്ജകം;
    sqlite3_വ്യഞ്ജകം_To_Register(pDel, വ്യഞ്ജകം_Code_Vector(pParse, pDel, &regFree1));
    എങ്കിൽ( xJump ){
      xJump(pParse, &വ്യഞ്ജകം_And, dest, jumpIfNull);
    }അല്ലെങ്കിൽ{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_വ്യഞ്ജകം_Code_Target(pParse, &വ്യഞ്ജകം_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3_വ്യഞ്ജകം_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_വ്യഞ്ജകം_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_വ്യഞ്ജകം_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_വ്യഞ്ജകം_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_വ്യഞ്ജകം_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_വ്യഞ്ജകം_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_വ്യഞ്ജകം_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_വ്യഞ്ജകം_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_വ്യഞ്ജകം_If_False && jumpIfNull!=0 && regFree1!=0 );
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_If_True(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ dest, പൂർണ്ണസംഖ്യ jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  പൂർണ്ണസംഖ്യ op = 0;
  പൂർണ്ണസംഖ്യ regFree1 = 0;
  പൂർണ്ണസംഖ്യ regFree2 = 0;
  പൂർണ്ണസംഖ്യ r1, r2;

  പ്രതിജ്ഞാ( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  എങ്കിൽ( NEVER(v==0) )     തിരികെ;  /* Existence of VDBE checked by caller */
  എങ്കിൽ( NEVER(p_വ്യഞ്ജകം==0) ) തിരികെ;  /* No way this can happen */
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_VVAProperty(p_വ്യഞ്ജകം, EP_Immutable) );
  op = p_വ്യഞ്ജകം->op;
  തിരഞ്ഞെടുപ്പ്( op ){
    അവസ്ഥ TK_AND:
    അവസ്ഥ TK_OR: {
      വ്യഞ്ജകം *pAlt = sqlite3_വ്യഞ്ജകം_Simplified_And_Or(p_വ്യഞ്ജകം);
      എങ്കിൽ( pAlt!=p_വ്യഞ്ജകം ){
        sqlite3_വ്യഞ്ജകം_If_True(pParse, pAlt, dest, jumpIfNull);
      }അല്ലെങ്കിൽ{
        വ്യഞ്ജകം *pFirst, *pSecond;
        എങ്കിൽ( വ്യഞ്ജകം_Eval_Rhs_First(p_വ്യഞ്ജകം) ){
          pFirst = p_വ്യഞ്ജകം->p_വലത്;
          pSecond = p_വ്യഞ്ജകം->p_ഇടത്;
        }അല്ലെങ്കിൽ{
          pFirst = p_വ്യഞ്ജകം->p_ഇടത്;
          pSecond = p_വ്യഞ്ജകം->p_വലത്;
        }
        എങ്കിൽ( op==TK_AND ){
          പൂർണ്ണസംഖ്യ d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_വ്യഞ്ജകം_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_വ്യഞ്ജകം_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }അല്ലെങ്കിൽ{
          testcase( jumpIfNull==0 );
          sqlite3_വ്യഞ്ജകം_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_വ്യഞ്ജകം_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_വ്യഞ്ജകം_If_False(pParse, p_വ്യഞ്ജകം->p_ഇടത്, dest, jumpIfNull);
      നിർത്തുക;
    }
    അവസ്ഥ TK_TRUTH: {
      പൂർണ്ണസംഖ്യ isNot;      /* IS NOT TRUE or IS NOT FALSE */
      പൂർണ്ണസംഖ്യ isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_വ്യഞ്ജകം->op2==TK_ISNOT;
      isTrue = sqlite3_വ്യഞ്ജകം_Truth_മൂല്യം(p_വ്യഞ്ജകം->p_വലത്);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      എങ്കിൽ( isTrue ^ isNot ){
        sqlite3_വ്യഞ്ജകം_If_True(pParse, p_വ്യഞ്ജകം->p_ഇടത്, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }അല്ലെങ്കിൽ{
        sqlite3_വ്യഞ്ജകം_If_False(pParse, p_വ്യഞ്ജകം->p_ഇടത്, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_IS:
    അവസ്ഥ TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    അവസ്ഥ TK_LT:
    അവസ്ഥ TK_LE:
    അവസ്ഥ TK_GT:
    അവസ്ഥ TK_GE:
    അവസ്ഥ TK_NE:
    അവസ്ഥ TK_EQ: {
      പൂർണ്ണസംഖ്യ addrIsNull;
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_Vector(p_വ്യഞ്ജകം->p_ഇടത്) ) പോകുക default_വ്യഞ്ജകം;
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = വ്യഞ്ജകം_Compute_Operands(pParse, p_വ്യഞ്ജകം,
                                   &r1, &r2, &regFree1, &regFree2);
      }അല്ലെങ്കിൽ{
        r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
        r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_വലത്, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_വ്യഞ്ജകം->p_ഇടത്, p_വ്യഞ്ജകം->p_വലത്, op,
                  r1, r2, dest, jumpIfNull, വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_Commuted));
      പ്രതിജ്ഞാ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      പ്രതിജ്ഞാ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      പ്രതിജ്ഞാ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      പ്രതിജ്ഞാ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      പ്രതിജ്ഞാ(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      പ്രതിജ്ഞാ(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      എങ്കിൽ( addrIsNull ){
        എങ്കിൽ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }അല്ലെങ്കിൽ{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_ISNULL:
    അവസ്ഥ TK_NOTNULL: {
      പ്രതിജ്ഞാ( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      പ്രതിജ്ഞാ( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
      പ്രതിജ്ഞാ( regFree1==0 || regFree1==r1 );
      എങ്കിൽ( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      നിർത്തുക;
    }
    അവസ്ഥ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      വ്യഞ്ജകം_Code_Between(pParse, p_വ്യഞ്ജകം, dest, sqlite3_വ്യഞ്ജകം_If_True, jumpIfNull);
      നിർത്തുക;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    അവസ്ഥ TK_IN: {
      പൂർണ്ണസംഖ്യ destIfFalse = sqlite3VdbeMakeLabel(pParse);
      പൂർണ്ണസംഖ്യ destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_വ്യഞ്ജകം_Code_IN(pParse, p_വ്യഞ്ജകം, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      നിർത്തുക;
    }
#endif
    സ്ഥിരസ്ഥിതി: {
    default_വ്യഞ്ജകം:
      എങ്കിൽ( വ്യഞ്ജകം_Always_True(p_വ്യഞ്ജകം) ){
        sqlite3VdbeGoto(v, dest);
      }അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Always_False(p_വ്യഞ്ജകം) ){
        /* No-op */
      }അല്ലെങ്കിൽ{
        r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      നിർത്തുക;
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_If_False(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ dest, പൂർണ്ണസംഖ്യ jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  പൂർണ്ണസംഖ്യ op = 0;
  പൂർണ്ണസംഖ്യ regFree1 = 0;
  പൂർണ്ണസംഖ്യ regFree2 = 0;
  പൂർണ്ണസംഖ്യ r1, r2;

  പ്രതിജ്ഞാ( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  എങ്കിൽ( NEVER(v==0) ) തിരികെ; /* Existence of VDBE checked by caller */
  എങ്കിൽ( p_വ്യഞ്ജകം==0 )    തിരികെ;
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_VVAProperty(p_വ്യഞ്ജകം,EP_Immutable) );

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
  op = ((p_വ്യഞ്ജകം->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_ISNULL || op==OP_NotNull );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_NOTNULL || op==OP_IsNull );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_NE || op==OP_Eq );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_EQ || op==OP_Ne );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_LT || op==OP_Ge );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_LE || op==OP_Gt );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_GT || op==OP_Le );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_GE || op==OP_Lt );

  തിരഞ്ഞെടുപ്പ്( p_വ്യഞ്ജകം->op ){
    അവസ്ഥ TK_AND:
    അവസ്ഥ TK_OR: {
      വ്യഞ്ജകം *pAlt = sqlite3_വ്യഞ്ജകം_Simplified_And_Or(p_വ്യഞ്ജകം);
      എങ്കിൽ( pAlt!=p_വ്യഞ്ജകം ){
        sqlite3_വ്യഞ്ജകം_If_False(pParse, pAlt, dest, jumpIfNull);
      }അല്ലെങ്കിൽ{
        വ്യഞ്ജകം *pFirst, *pSecond;
        എങ്കിൽ( വ്യഞ്ജകം_Eval_Rhs_First(p_വ്യഞ്ജകം) ){
          pFirst = p_വ്യഞ്ജകം->p_വലത്;
          pSecond = p_വ്യഞ്ജകം->p_ഇടത്;
        }അല്ലെങ്കിൽ{
          pFirst = p_വ്യഞ്ജകം->p_ഇടത്;
          pSecond = p_വ്യഞ്ജകം->p_വലത്;
        }
        എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_വ്യഞ്ജകം_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_വ്യഞ്ജകം_If_False(pParse, pSecond, dest, jumpIfNull);
        }അല്ലെങ്കിൽ{
          പൂർണ്ണസംഖ്യ d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_വ്യഞ്ജകം_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_വ്യഞ്ജകം_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_വ്യഞ്ജകം_If_True(pParse, p_വ്യഞ്ജകം->p_ഇടത്, dest, jumpIfNull);
      നിർത്തുക;
    }
    അവസ്ഥ TK_TRUTH: {
      പൂർണ്ണസംഖ്യ isNot;   /* IS NOT TRUE or IS NOT FALSE */
      പൂർണ്ണസംഖ്യ isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_വ്യഞ്ജകം->op2==TK_ISNOT;
      isTrue = sqlite3_വ്യഞ്ജകം_Truth_മൂല്യം(p_വ്യഞ്ജകം->p_വലത്);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      എങ്കിൽ( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_വ്യഞ്ജകം_If_False(pParse, p_വ്യഞ്ജകം->p_ഇടത്, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }അല്ലെങ്കിൽ{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_വ്യഞ്ജകം_If_True(pParse, p_വ്യഞ്ജകം->p_ഇടത്, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_IS:
    അവസ്ഥ TK_ISNOT:
      testcase( p_വ്യഞ്ജകം->op==TK_IS );
      testcase( p_വ്യഞ്ജകം->op==TK_ISNOT );
      op = (p_വ്യഞ്ജകം->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    അവസ്ഥ TK_LT:
    അവസ്ഥ TK_LE:
    അവസ്ഥ TK_GT:
    അവസ്ഥ TK_GE:
    അവസ്ഥ TK_NE:
    അവസ്ഥ TK_EQ: {
      പൂർണ്ണസംഖ്യ addrIsNull;
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_Vector(p_വ്യഞ്ജകം->p_ഇടത്) ) പോകുക default_വ്യഞ്ജകം;
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = വ്യഞ്ജകം_Compute_Operands(pParse, p_വ്യഞ്ജകം,
                                   &r1, &r2, &regFree1, &regFree2);
      }അല്ലെങ്കിൽ{
        r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
        r2 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_വലത്, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_വ്യഞ്ജകം->p_ഇടത്, p_വ്യഞ്ജകം->p_വലത്, op,
                  r1, r2, dest, jumpIfNull,വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_Commuted));
      പ്രതിജ്ഞാ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      പ്രതിജ്ഞാ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      പ്രതിജ്ഞാ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      പ്രതിജ്ഞാ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      പ്രതിജ്ഞാ(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      പ്രതിജ്ഞാ(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      എങ്കിൽ( addrIsNull ){
        എങ്കിൽ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }അല്ലെങ്കിൽ{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_ISNULL:
    അവസ്ഥ TK_NOTNULL: {
      r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം->p_ഇടത്, &regFree1);
      പ്രതിജ്ഞാ( regFree1==0 || regFree1==r1 );
      എങ്കിൽ( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      നിർത്തുക;
    }
    അവസ്ഥ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      വ്യഞ്ജകം_Code_Between(pParse, p_വ്യഞ്ജകം, dest, sqlite3_വ്യഞ്ജകം_If_False, jumpIfNull);
      നിർത്തുക;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    അവസ്ഥ TK_IN: {
      എങ്കിൽ( jumpIfNull ){
        sqlite3_വ്യഞ്ജകം_Code_IN(pParse, p_വ്യഞ്ജകം, dest, dest);
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_വ്യഞ്ജകം_Code_IN(pParse, p_വ്യഞ്ജകം, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      നിർത്തുക;
    }
#endif
    സ്ഥിരസ്ഥിതി: {
    default_വ്യഞ്ജകം:
      എങ്കിൽ( വ്യഞ്ജകം_Always_False(p_വ്യഞ്ജകം) ){
        sqlite3VdbeGoto(v, dest);
      }അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Always_True(p_വ്യഞ്ജകം) ){
        /* no-op */
      }അല്ലെങ്കിൽ{
        r1 = sqlite3_വ്യഞ്ജകം_Code_Temp(pParse, p_വ്യഞ്ജകം, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      നിർത്തുക;
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_If_False_Dup(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ dest,പൂർണ്ണസംഖ്യ jumpIfNull){
  sqlite3 *db = pParse->db;
  വ്യഞ്ജകം *pCopy = sqlite3_വ്യഞ്ജകം_Dup(db, p_വ്യഞ്ജകം, 0);
  എങ്കിൽ( db->mallocFailed==0 ){
    sqlite3_വ്യഞ്ജകം_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_വ്യഞ്ജകം_Delete(db, pCopy);
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
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Compare_Variable(
  സ്ഥിരം Parse *pParse,
  സ്ഥിരം വ്യഞ്ജകം *pVar,
  സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം
){
  പൂർണ്ണസംഖ്യ res = 2;
  പൂർണ്ണസംഖ്യ iVar;
  sqlite3_മൂല്യം *pL, *pR = 0;
 
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_VARIABLE && pVar->iColumn==p_വ്യഞ്ജകം->iColumn ){
    തിരികെ 0;
  }
  എങ്കിൽ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) തിരികെ 2;
  sqlite3_മൂല്യം_From_വ്യഞ്ജകം(pParse->db, p_വ്യഞ്ജകം, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  എങ്കിൽ( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_മൂല്യം(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    എങ്കിൽ( pL ){
      എങ്കിൽ( sqlite3_മൂല്യം_type(pL)==SQLITE_TEXT ){
        sqlite3_മൂല്യം_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_മൂല്യം_Free(pR);
    sqlite3_മൂല്യം_Free(pL);
  }
  തിരികെ res;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Compare(
  സ്ഥിരം Parse *pParse,
  സ്ഥിരം വ്യഞ്ജകം *pA,
  സ്ഥിരം വ്യഞ്ജകം *pB,
  പൂർണ്ണസംഖ്യ iTab
){
  u32 combinedFlags;
  എങ്കിൽ( pA==0 || pB==0 ){
    തിരികെ pB==pA ? 0 : 2;
  }
  എങ്കിൽ( pParse && pA->op==TK_VARIABLE ){
    തിരികെ വ്യഞ്ജകം_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  എങ്കിൽ( combinedFlags & EP_Int_മൂല്യം ){
    എങ്കിൽ( (pA->flags&pB->flags&EP_Int_മൂല്യം)!=0 && pA->u.i_മൂല്യം==pB->u.i_മൂല്യം ){
      തിരികെ 0;
    }
    തിരികെ 2;
  }
  എങ്കിൽ( pA->op!=pB->op || pA->op==TK_RAISE ){
    എങ്കിൽ( pA->op==TK_COLLATE && sqlite3_വ്യഞ്ജകം_Compare(pParse, pA->p_ഇടത്,pB,iTab)<2 ){
      തിരികെ 1;
    }
    എങ്കിൽ( pB->op==TK_COLLATE && sqlite3_വ്യഞ്ജകം_Compare(pParse, pA,pB->p_ഇടത്,iTab)<2 ){
      തിരികെ 1;
    }
    എങ്കിൽ( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }അല്ലെങ്കിൽ{
      തിരികെ 2;
    }
  }
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(pA, EP_Int_മൂല്യം) );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(pB, EP_Int_മൂല്യം) );
  എങ്കിൽ( pA->u.zToken ){
    എങ്കിൽ( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      എങ്കിൽ( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) തിരികെ 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      പ്രതിജ്ഞാ( pA->op==pB->op );
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(pA,EP_WinFunc)!=വ്യഞ്ജകം_Has_Property(pB,EP_WinFunc) ){
        തിരികെ 2;
      }
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(pA,EP_WinFunc) ){
        എങ്കിൽ( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          തിരികെ 2;
        }
      }
#endif
    }അല്ലെങ്കിൽ എങ്കിൽ( pA->op==TK_NULL ){
      തിരികെ 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( pA->op==TK_COLLATE ){
      എങ്കിൽ( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) തിരികെ 2;
    }അല്ലെങ്കിൽ
    എങ്കിൽ( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      തിരികെ 2;
    }
  }
  എങ്കിൽ( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) തിരികെ 2;
  എങ്കിൽ( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    എങ്കിൽ( combinedFlags & EP_xIsSelect ) തിരികെ 2;
    എങ്കിൽ( (combinedFlags & EP_FixedCol)==0
     && sqlite3_വ്യഞ്ജകം_Compare(pParse, pA->p_ഇടത്, pB->p_ഇടത്, iTab) ) തിരികെ 2;
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(pParse, pA->p_വലത്, pB->p_വലത്, iTab) ) തിരികെ 2;
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_List_Compare(pA->x.pList, pB->x.pList, iTab) ) തിരികെ 2;
    എങ്കിൽ( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      എങ്കിൽ( pA->iColumn!=pB->iColumn ) തിരികെ 2;
      എങ്കിൽ( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) തിരികെ 2;
      എങ്കിൽ( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        തിരികെ 2;
      }
    }
  }
  തിരികെ 0;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_List_Compare(സ്ഥിരം വ്യഞ്ജകം_List *pA, സ്ഥിരം വ്യഞ്ജകം_List *pB, പൂർണ്ണസംഖ്യ iTab){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( pA==0 && pB==0 ) തിരികെ 0;
  എങ്കിൽ( pA==0 || pB==0 ) തിരികെ 1;
  എങ്കിൽ( pA->n_വ്യഞ്ജകം!=pB->n_വ്യഞ്ജകം ) തിരികെ 1;
  വേണ്ടി(i=0; i<pA->n_വ്യഞ്ജകം; i++){
    പൂർണ്ണസംഖ്യ res;
    വ്യഞ്ജകം *p_വ്യഞ്ജകം_A = pA->a[i].p_വ്യഞ്ജകം;
    വ്യഞ്ജകം *p_വ്യഞ്ജകം_B = pB->a[i].p_വ്യഞ്ജകം;
    എങ്കിൽ( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) തിരികെ 1;
    എങ്കിൽ( (res = sqlite3_വ്യഞ്ജകം_Compare(0, p_വ്യഞ്ജകം_A, p_വ്യഞ്ജകം_B, iTab)) ) തിരികെ res;
  }
  തിരികെ 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Compare_Skip(വ്യഞ്ജകം *pA,വ്യഞ്ജകം *pB, പൂർണ്ണസംഖ്യ iTab){
  തിരികെ sqlite3_വ്യഞ്ജകം_Compare(0,
             sqlite3_വ്യഞ്ജകം_Skip_Collate(pA),
             sqlite3_വ്യഞ്ജകം_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Implies_Not_Null(
  സ്ഥിരം Parse *pParse,/* Parsing context */
  സ്ഥിരം വ്യഞ്ജകം *p,      /* The expression to be checked */
  സ്ഥിരം വ്യഞ്ജകം *pNN,    /* The expression that is NOT NULL */
  പൂർണ്ണസംഖ്യ iTab,           /* Table being evaluated */
  പൂർണ്ണസംഖ്യ seenNot         /* Return true only if p can be any non-NULL value */
){
  പ്രതിജ്ഞാ( p );
  പ്രതിജ്ഞാ( pNN );
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(pParse, p, pNN, iTab)==0 ){
    തിരികെ pNN->op!=TK_NULL;
  }
  തിരഞ്ഞെടുപ്പ്( p->op ){
    അവസ്ഥ TK_IN: {
      എങ്കിൽ( seenNot && വ്യഞ്ജകം_Has_Property(p, EP_xIsSelect) ) തിരികെ 0;
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_വ്യഞ്ജകം>0) );
      തിരികെ വ്യഞ്ജകം_Implies_Not_Null(pParse, p->p_ഇടത്, pNN, iTab, 1);
    }
    അവസ്ഥ TK_BETWEEN: {
      വ്യഞ്ജകം_List *pList;
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p) );
      pList = p->x.pList;
      പ്രതിജ്ഞാ( pList!=0 );
      പ്രതിജ്ഞാ( pList->n_വ്യഞ്ജകം==2 );
      എങ്കിൽ( seenNot ) തിരികെ 0;
      എങ്കിൽ( വ്യഞ്ജകം_Implies_Not_Null(pParse, pList->a[0].p_വ്യഞ്ജകം, pNN, iTab, 1)
       || വ്യഞ്ജകം_Implies_Not_Null(pParse, pList->a[1].p_വ്യഞ്ജകം, pNN, iTab, 1)
      ){
        തിരികെ 1;
      }
      തിരികെ വ്യഞ്ജകം_Implies_Not_Null(pParse, p->p_ഇടത്, pNN, iTab, 1);
    }
    അവസ്ഥ TK_EQ:
    അവസ്ഥ TK_NE:
    അവസ്ഥ TK_LT:
    അവസ്ഥ TK_LE:
    അവസ്ഥ TK_GT:
    അവസ്ഥ TK_GE:
    അവസ്ഥ TK_PLUS:
    അവസ്ഥ TK_MINUS:
    അവസ്ഥ TK_BITOR:
    അവസ്ഥ TK_LSHIFT:
    അവസ്ഥ TK_RSHIFT:
    അവസ്ഥ TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    അവസ്ഥ TK_STAR:
    അവസ്ഥ TK_REM:
    അവസ്ഥ TK_BITAND:
    അവസ്ഥ TK_SLASH: {
      എങ്കിൽ( വ്യഞ്ജകം_Implies_Not_Null(pParse, p->p_വലത്, pNN, iTab, seenNot) ) തിരികെ 1;
      /* no break */ deliberate_fall_through
    }
    അവസ്ഥ TK_SPAN:
    അവസ്ഥ TK_COLLATE:
    അവസ്ഥ TK_UPLUS:
    അവസ്ഥ TK_UMINUS: {
      തിരികെ വ്യഞ്ജകം_Implies_Not_Null(pParse, p->p_ഇടത്, pNN, iTab, seenNot);
    }
    അവസ്ഥ TK_TRUTH: {
      എങ്കിൽ( seenNot ) തിരികെ 0;
      എങ്കിൽ( p->op2!=TK_IS ) തിരികെ 0;
      തിരികെ വ്യഞ്ജകം_Implies_Not_Null(pParse, p->p_ഇടത്, pNN, iTab, 1);
    }
    അവസ്ഥ TK_BITNOT:
    അവസ്ഥ TK_NOT: {
      തിരികെ വ്യഞ്ജകം_Implies_Not_Null(pParse, p->p_ഇടത്, pNN, iTab, 1);
    }
  }
  തിരികെ 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Not_True(വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പൂർണ്ണസംഖ്യ v;
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_NULL ) തിരികെ 1;
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_TRUEFALSE && sqlite3_വ്യഞ്ജകം_Truth_മൂല്യം(p_വ്യഞ്ജകം)==0 ) തിരികെ 1;
  v = 1;
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_Integer(p_വ്യഞ്ജകം, &v, 0, 0) && v==0 ) തിരികെ 1;
  തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_IIF(sqlite3 *db, സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  വ്യഞ്ജകം_List *pList;
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_FUNCTION ){
    സ്ഥിരം അക്ഷരം *z = p_വ്യഞ്ജകം->u.zToken;
    FuncDef *pDef;
    എങ്കിൽ( (z[0]!='i' && z[0]!='I') ) തിരികെ 0;
    എങ്കിൽ( p_വ്യഞ്ജകം->x.pList==0 ) തിരികെ 0;
    pDef = sqlite3FindFunction(db, z, p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    എങ്കിൽ( pDef==0 ) തിരികെ 0;
#അല്ലെങ്കിൽ
    എങ്കിൽ( NEVER(pDef==0) ) തിരികെ 0;
#endif
    എങ്കിൽ( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) തിരികെ 0;
    എങ്കിൽ( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) തിരികെ 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_CASE ){
    എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത്!=0 ) തിരികെ 0;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
  }
  pList = p_വ്യഞ്ജകം->x.pList;
  പ്രതിജ്ഞാ( pList!=0 );
  എങ്കിൽ( pList->n_വ്യഞ്ജകം==2 ) തിരികെ 1;
  എങ്കിൽ( pList->n_വ്യഞ്ജകം==3 && sqlite3_വ്യഞ്ജകം_Is_Not_True(pList->a[2].p_വ്യഞ്ജകം) ) തിരികെ 1;
  തിരികെ 0;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Implies_വ്യഞ്ജകം(
  സ്ഥിരം Parse *pParse,
  സ്ഥിരം വ്യഞ്ജകം *pE1,
  സ്ഥിരം വ്യഞ്ജകം *pE2,
  പൂർണ്ണസംഖ്യ iTab
){
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(pParse, pE1, pE2, iTab)==0 ){
    തിരികെ 1;
  }
  എങ്കിൽ( pE2->op==TK_OR
   && (sqlite3_വ്യഞ്ജകം_Implies_വ്യഞ്ജകം(pParse, pE1, pE2->p_ഇടത്, iTab)
             || sqlite3_വ്യഞ്ജകം_Implies_വ്യഞ്ജകം(pParse, pE1, pE2->p_വലത്, iTab) )
  ){
    തിരികെ 1;
  }
  എങ്കിൽ( pE2->op==TK_NOTNULL
   && വ്യഞ്ജകം_Implies_Not_Null(pParse, pE1, pE2->p_ഇടത്, iTab, 0)
  ){
    തിരികെ 1;
  }
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_IIF(pParse->db, pE1) ){
    തിരികെ sqlite3_വ്യഞ്ജകം_Implies_വ്യഞ്ജകം(pParse,pE1->x.pList->a[0].p_വ്യഞ്ജകം,pE2,iTab);
  }
  തിരികെ 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
സ്ഥിതം ശൂന്യം bothImplyNotNullRow(Walker *pWalker, വ്യഞ്ജകം *pE1, വ്യഞ്ജകം *pE2){
  എങ്കിൽ( pWalker->eCode==0 ){
    sqlite3_Walk_വ്യഞ്ജകം(pWalker, pE1);
    എങ്കിൽ( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_വ്യഞ്ജകം(pWalker, pE2);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ impliesNotNullRow(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  testcase( p_വ്യഞ്ജകം->op==TK_AGG_COLUMN );
  testcase( p_വ്യഞ്ജകം->op==TK_AGG_FUNCTION );
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON) ) തിരികെ WRC_Prune;
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    തിരികെ WRC_Prune;
  }
  തിരഞ്ഞെടുപ്പ്( p_വ്യഞ്ജകം->op ){
    അവസ്ഥ TK_ISNOT:
    അവസ്ഥ TK_ISNULL:
    അവസ്ഥ TK_NOTNULL:
    അവസ്ഥ TK_IS:
    അവസ്ഥ TK_VECTOR:
    അവസ്ഥ TK_FUNCTION:
    അവസ്ഥ TK_TRUTH:
    അവസ്ഥ TK_CASE:
      testcase( p_വ്യഞ്ജകം->op==TK_ISNOT );
      testcase( p_വ്യഞ്ജകം->op==TK_ISNULL );
      testcase( p_വ്യഞ്ജകം->op==TK_NOTNULL );
      testcase( p_വ്യഞ്ജകം->op==TK_IS );
      testcase( p_വ്യഞ്ജകം->op==TK_VECTOR );
      testcase( p_വ്യഞ്ജകം->op==TK_FUNCTION );
      testcase( p_വ്യഞ്ജകം->op==TK_TRUTH );
      testcase( p_വ്യഞ്ജകം->op==TK_CASE );
      തിരികെ WRC_Prune;

    അവസ്ഥ TK_COLUMN:
      എങ്കിൽ( pWalker->u.iCur==p_വ്യഞ്ജകം->iTable ){
        pWalker->eCode = 1;
        തിരികെ WRC_Abort;
      }
      തിരികെ WRC_Prune;

    അവസ്ഥ TK_OR:
    അവസ്ഥ TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_വ്യഞ്ജകം->op==TK_OR );
      testcase( p_വ്യഞ്ജകം->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_വ്യഞ്ജകം->p_ഇടത്, p_വ്യഞ്ജകം->p_വലത്);
      തിരികെ WRC_Prune;
       
    അവസ്ഥ TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      എങ്കിൽ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) && ALWAYS(p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം>0) ){
        sqlite3_Walk_വ്യഞ്ജകം(pWalker, p_വ്യഞ്ജകം->p_ഇടത്);
      }
      തിരികെ WRC_Prune;

    അവസ്ഥ TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം==2 );
      sqlite3_Walk_വ്യഞ്ജകം(pWalker, p_വ്യഞ്ജകം->p_ഇടത്);
      bothImplyNotNullRow(pWalker, p_വ്യഞ്ജകം->x.pList->a[0].p_വ്യഞ്ജകം,
                                   p_വ്യഞ്ജകം->x.pList->a[1].p_വ്യഞ്ജകം);
      തിരികെ WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    അവസ്ഥ TK_EQ:
    അവസ്ഥ TK_NE:
    അവസ്ഥ TK_LT:
    അവസ്ഥ TK_LE:
    അവസ്ഥ TK_GT:
    അവസ്ഥ TK_GE: {
      വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
      വ്യഞ്ജകം *p_വലത് = p_വ്യഞ്ജകം->p_വലത്;
      testcase( p_വ്യഞ്ജകം->op==TK_EQ );
      testcase( p_വ്യഞ്ജകം->op==TK_NE );
      testcase( p_വ്യഞ്ജകം->op==TK_LT );
      testcase( p_വ്യഞ്ജകം->op==TK_LE );
      testcase( p_വ്യഞ്ജകം->op==TK_GT );
      testcase( p_വ്യഞ്ജകം->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      പ്രതിജ്ഞാ( p_ഇടത്->op!=TK_COLUMN || വ്യഞ്ജകം_Use_YTab(p_ഇടത്) );
      പ്രതിജ്ഞാ( p_വലത്->op!=TK_COLUMN || വ്യഞ്ജകം_Use_YTab(p_വലത്) );
      എങ്കിൽ( (p_ഇടത്->op==TK_COLUMN
           && ALWAYS(p_ഇടത്->y.pTab!=0)
           && IsVirtual(p_ഇടത്->y.pTab))
       || (p_വലത്->op==TK_COLUMN
           && ALWAYS(p_വലത്->y.pTab!=0)
           && IsVirtual(p_വലത്->y.pTab))
      ){
        തിരികെ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    സ്ഥിരസ്ഥിതി:
      തിരികെ WRC_Continue;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Implies_Non_Null_Row(വ്യഞ്ജകം *p, പൂർണ്ണസംഖ്യ iTab, പൂർണ്ണസംഖ്യ isRJ){
  Walker w;
  p = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(p);
  എങ്കിൽ( p==0 ) തിരികെ 0;
  എങ്കിൽ( p->op==TK_NOTNULL ){
    p = p->p_ഇടത്;
  }അല്ലെങ്കിൽ{
    ആകുമ്പോൾ( p->op==TK_AND ){
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Implies_Non_Null_Row(p->p_ഇടത്, iTab, isRJ) ) തിരികെ 1;
      p = p->p_വലത്;
    }
  }
  w.x_വ്യഞ്ജകം_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_വ്യഞ്ജകം(&w, p);
  തിരികെ w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
ഘടന IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  പൂർണ്ണസംഖ്യ iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Idx_Cover(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_COLUMN
   && p_വ്യഞ്ജകം->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_വ്യഞ്ജകം->iColumn)<0
  ){
    pWalker->eCode = 1;
    തിരികെ WRC_Abort;
  }
  തിരികെ WRC_Continue;
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
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Covered_By_Index(
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,        /* The index to be tested */
  പൂർണ്ണസംഖ്യ iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  ഘടന IdxCover xcov;
  memset(&w, 0, വലുപ്പം(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_വ്യഞ്ജകം_Callback = വ്യഞ്ജകം_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_വ്യഞ്ജകം(&w, p_വ്യഞ്ജകം);
  തിരികെ !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
ഘടന RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  പൂർണ്ണസംഖ്യ *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ selectRefEnter(Walker *pWalker, Select *pSelect){
  ഘടന RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  പൂർണ്ണസംഖ്യ *piNew;
  എങ്കിൽ( pSrc->nSrc==0 ) തിരികെ WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*വലുപ്പം(പൂർണ്ണസംഖ്യ));
  എങ്കിൽ( piNew==0 ){
    p->nExclude = 0;
    തിരികെ WRC_Abort;
  }അല്ലെങ്കിൽ{
    p->aiExclude = piNew;
  }
  വേണ്ടി(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  തിരികെ WRC_Continue;
}
സ്ഥിതം ശൂന്യം selectRefLeave(Walker *pWalker, Select *pSelect){
  ഘടന RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  എങ്കിൽ( p->nExclude ){
    പ്രതിജ്ഞാ( p->nExclude>=pSrc->nSrc );
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
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Ref_To_Src_List(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_COLUMN
   || p_വ്യഞ്ജകം->op==TK_AGG_COLUMN
  ){
    പൂർണ്ണസംഖ്യ i;
    ഘടന RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    പൂർണ്ണസംഖ്യ nSrc = pSrc ? pSrc->nSrc : 0;
    വേണ്ടി(i=0; i<nSrc; i++){
      എങ്കിൽ( p_വ്യഞ്ജകം->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        തിരികെ WRC_Continue;
      }
    }
    വേണ്ടി(i=0; i<p->nExclude && p->aiExclude[i]!=p_വ്യഞ്ജകം->iTable; i++){}
    എങ്കിൽ( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  തിരികെ WRC_Continue;
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
പൂർണ്ണസംഖ്യ sqlite3ReferencesSrcList(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, SrcList *pSrcList){
  Walker w;
  ഘടന RefSrcList x;
  പ്രതിജ്ഞാ( pParse->db!=0 );
  memset(&w, 0, വലുപ്പം(w));
  memset(&x, 0, വലുപ്പം(x));
  w.x_വ്യഞ്ജകം_Callback = വ്യഞ്ജകം_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_AGG_FUNCTION );
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
  sqlite3_Walk_വ്യഞ്ജകം_List(&w, p_വ്യഞ്ജകം->x.pList);
  എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത് ){
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്->op==TK_ORDER );
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം->p_ഇടത്) );
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്->x.pList!=0 );
    sqlite3_Walk_വ്യഞ്ജകം_List(&w, p_വ്യഞ്ജകം->p_ഇടത്->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
    sqlite3_Walk_വ്യഞ്ജകം(&w, p_വ്യഞ്ജകം->y.pWin->pFilter);
  }
#endif
  എങ്കിൽ( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  എങ്കിൽ( w.eCode & 0x01 ){
    തിരികെ 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( w.eCode ){
    തിരികെ 0;
  }അല്ലെങ്കിൽ{
    തിരികെ -1;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ agginfo_Persist_വ്യഞ്ജകം_Cb(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  എങ്കിൽ( ALWAYS(!വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly|EP_Reduced))
   && p_വ്യഞ്ജകം->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_വ്യഞ്ജകം->pAggInfo;
    പൂർണ്ണസംഖ്യ iAgg = p_വ്യഞ്ജകം->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    പ്രതിജ്ഞാ( iAgg>=0 );
    എങ്കിൽ( p_വ്യഞ്ജകം->op!=TK_AGG_FUNCTION ){
      എങ്കിൽ( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_വ്യഞ്ജകം
      ){
        p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Dup(db, p_വ്യഞ്ജകം, 0);
        എങ്കിൽ( p_വ്യഞ്ജകം && !sqlite3_വ്യഞ്ജകം_Deferred_Delete(pParse, p_വ്യഞ്ജകം) ){
          pAggInfo->aCol[iAgg].pCExpr = p_വ്യഞ്ജകം;
        }
      }
    }അല്ലെങ്കിൽ{
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_AGG_FUNCTION );
      എങ്കിൽ( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_വ്യഞ്ജകം
      ){
        p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Dup(db, p_വ്യഞ്ജകം, 0);
        എങ്കിൽ( p_വ്യഞ്ജകം && !sqlite3_വ്യഞ്ജകം_Deferred_Delete(pParse, p_വ്യഞ്ജകം) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_വ്യഞ്ജകം;
        }
      }
    }
  }
  തിരികെ WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
ശൂന്യം sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, വലുപ്പം(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_വ്യഞ്ജകം_Callback = agginfo_Persist_വ്യഞ്ജകം_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  പൂർണ്ണസംഖ്യ i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       വലുപ്പം(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  തിരികെ i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  പൂർണ്ണസംഖ്യ i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       വലുപ്പം(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  തിരികെ i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
സ്ഥിതം ശൂന്യം findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം          /* Expr describing the column to find or insert */
){
  ഘടന AggInfo_col *pCol;
  പൂർണ്ണസംഖ്യ k;
  പൂർണ്ണസംഖ്യ mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  പ്രതിജ്ഞാ( mxTerm <= SMXV(i16) );
  പ്രതിജ്ഞാ( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  വേണ്ടി(k=0; k<pAggInfo->nColumn; k++, pCol++){
    എങ്കിൽ( pCol->pCExpr==p_വ്യഞ്ജകം ) തിരികെ;
    എങ്കിൽ( pCol->iTable==p_വ്യഞ്ജകം->iTable
     && pCol->iColumn==p_വ്യഞ്ജകം->iColumn
     && p_വ്യഞ്ജകം->op!=TK_IF_NULL_ROW
    ){
      പോകുക fix_up_വ്യഞ്ജകം;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  എങ്കിൽ( k<0 ){
    /* OOM on resize */
    പ്രതിജ്ഞാ( pParse->db->mallocFailed );
    തിരികെ;
  }
  എങ്കിൽ( k>mxTerm ){
    sqlite3_Error_സന്ദേശം(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
  pCol->pTab = p_വ്യഞ്ജകം->y.pTab;
  pCol->iTable = p_വ്യഞ്ജകം->iTable;
  pCol->iColumn = p_വ്യഞ്ജകം->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_വ്യഞ്ജകം;
  എങ്കിൽ( pAggInfo->pGroupBy && p_വ്യഞ്ജകം->op!=TK_IF_NULL_ROW ){
    പൂർണ്ണസംഖ്യ j, n;
    വ്യഞ്ജകം_List *pGB = pAggInfo->pGroupBy;
    ഘടന വ്യഞ്ജകം_List_item *pTerm = pGB->a;
    n = pGB->n_വ്യഞ്ജകം;
    വേണ്ടി(j=0; j<n; j++, pTerm++){
      വ്യഞ്ജകം *pE = pTerm->p_വ്യഞ്ജകം;
      എങ്കിൽ( pE->op==TK_COLUMN
       && pE->iTable==p_വ്യഞ്ജകം->iTable
       && pE->iColumn==p_വ്യഞ്ജകം->iColumn
      ){
        pCol->iSorterColumn = j;
        നിർത്തുക;
      }
    }
  }
  എങ്കിൽ( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_വ്യഞ്ജകം:
  വ്യഞ്ജകം_Set_VVAProperty(p_വ്യഞ്ജകം, EP_NoReduce);
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->pAggInfo==0 || p_വ്യഞ്ജകം->pAggInfo==pAggInfo );
  p_വ്യഞ്ജകം->pAggInfo = pAggInfo;
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_COLUMN ){
    p_വ്യഞ്ജകം->op = TK_AGG_COLUMN;
  }
  പ്രതിജ്ഞാ( k <= SMXV(p_വ്യഞ്ജകം->iAgg) );
  p_വ്യഞ്ജകം->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ analyzeAggregate(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പൂർണ്ണസംഖ്യ i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  പ്രതിജ്ഞാ( pNC->ncFlags & NC_UAggInfo );
  പ്രതിജ്ഞാ( pAggInfo->iFirstReg==0 );
  തിരഞ്ഞെടുപ്പ്( p_വ്യഞ്ജകം->op ){
    സ്ഥിരസ്ഥിതി: {
      Indexed_വ്യഞ്ജകം *pIEpr;
      വ്യഞ്ജകം tmp;
      പ്രതിജ്ഞാ( pParse->iSelfTab==0 );
      എങ്കിൽ( (pNC->ncFlags & NC_InAggFunc)==0 ) നിർത്തുക;
      എങ്കിൽ( pParse->pIdxEpr==0 ) നിർത്തുക;
      വേണ്ടി(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        പൂർണ്ണസംഖ്യ iDataCur = pIEpr->iDataCur;
        എങ്കിൽ( iDataCur<0 ) തുടരുക;
        എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0, p_വ്യഞ്ജകം, pIEpr->p_വ്യഞ്ജകം, iDataCur)==0 ) നിർത്തുക;
      }
      എങ്കിൽ( pIEpr==0 ) നിർത്തുക;
      എങ്കിൽ( NEVER(!വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം)) ) നിർത്തുക;
      വേണ്ടി(i=0; i<pSrcList->nSrc; i++){
         എങ്കിൽ( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           നിർത്തുക;
         }
      }
      എങ്കിൽ( i>=pSrcList->nSrc ) നിർത്തുക;
      എങ്കിൽ( NEVER(p_വ്യഞ്ജകം->pAggInfo!=0) ) നിർത്തുക; /* Resolved by outer context */
      എങ്കിൽ( pParse->nErr ){ തിരികെ WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, വലുപ്പം(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      എങ്കിൽ( pParse->nErr ){ തിരികെ WRC_Abort; }
      പ്രതിജ്ഞാ( pAggInfo->aCol!=0 );
      പ്രതിജ്ഞാ( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_വ്യഞ്ജകം;
      p_വ്യഞ്ജകം->pAggInfo = pAggInfo;
      p_വ്യഞ്ജകം->iAgg = tmp.iAgg;
      തിരികെ WRC_Prune;
    }
    അവസ്ഥ TK_IF_NULL_ROW:
    അവസ്ഥ TK_AGG_COLUMN:
    അവസ്ഥ TK_COLUMN: {
      testcase( p_വ്യഞ്ജകം->op==TK_AGG_COLUMN );
      testcase( p_വ്യഞ്ജകം->op==TK_COLUMN );
      testcase( p_വ്യഞ്ജകം->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      എങ്കിൽ( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        വേണ്ടി(i=0; i<pSrcList->nSrc; i++, pItem++){
          പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly|EP_Reduced) );
          എങ്കിൽ( p_വ്യഞ്ജകം->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_വ്യഞ്ജകം);
            നിർത്തുക;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      തിരികെ WRC_Continue;
    }
    അവസ്ഥ TK_AGG_FUNCTION: {
      എങ്കിൽ( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_വ്യഞ്ജകം->op2
       && p_വ്യഞ്ജകം->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        ഘടന AggInfo_func *pItem = pAggInfo->aFunc;
        പൂർണ്ണസംഖ്യ mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        പ്രതിജ്ഞാ( mxTerm <= SMXV(i16) );
        വേണ്ടി(i=0; i<pAggInfo->nFunc; i++, pItem++){
          എങ്കിൽ( NEVER(pItem->pFExpr==p_വ്യഞ്ജകം) ) നിർത്തുക;
          എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0, pItem->pFExpr, p_വ്യഞ്ജകം, -1)==0 ){
            നിർത്തുക;
          }
        }
        എങ്കിൽ( i>mxTerm ){
          sqlite3_Error_സന്ദേശം(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          പ്രതിജ്ഞാ( i<pAggInfo->nFunc );
        }അല്ലെങ്കിൽ എങ്കിൽ( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          എങ്കിൽ( i>=0 ){
            പൂർണ്ണസംഖ്യ nArg;
            പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_വ്യഞ്ജകം;
            പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_UToken(p_വ്യഞ്ജകം) );
            nArg = p_വ്യഞ്ജകം->x.pList ? p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_വ്യഞ്ജകം->u.zToken, nArg, enc, 0);
            പ്രതിജ്ഞാ( pItem->bOBUnique==0 );
            എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത്
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              വ്യഞ്ജകം_List *pOBList;
              പ്രതിജ്ഞാ( nArg>0 );
              പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്->op==TK_ORDER );
              പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം->p_ഇടത്) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_വ്യഞ്ജകം->p_ഇടത്->x.pList;
              പ്രതിജ്ഞാ( pOBList->n_വ്യഞ്ജകം>0 );
              പ്രതിജ്ഞാ( pItem->bOBUnique==0 );
              എങ്കിൽ( pOBList->n_വ്യഞ്ജകം==1
               && nArg==1
               && sqlite3_വ്യഞ്ജകം_Compare(0,pOBList->a[0].p_വ്യഞ്ജകം,
                               p_വ്യഞ്ജകം->x.pList->a[0].p_വ്യഞ്ജകം,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Distinct);
              }അല്ലെങ്കിൽ{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }അല്ലെങ്കിൽ{
              pItem->iOBTab = -1;
            }
            എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }അല്ലെങ്കിൽ{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly|EP_Reduced) );
        വ്യഞ്ജകം_Set_VVAProperty(p_വ്യഞ്ജകം, EP_NoReduce);
        പ്രതിജ്ഞാ( i <= SMXV(p_വ്യഞ്ജകം->iAgg) );
        p_വ്യഞ്ജകം->iAgg = (i16)i;
        p_വ്യഞ്ജകം->pAggInfo = pAggInfo;
        തിരികെ WRC_Prune;
      }അല്ലെങ്കിൽ{
        തിരികെ WRC_Continue;
      }
    }
  }
  തിരികെ WRC_Continue;
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
ശൂന്യം sqlite3_വ്യഞ്ജകം_Analyze_Aggregates(NameContext *pNC, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  Walker w;
  w.x_വ്യഞ്ജകം_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  പ്രതിജ്ഞാ( pNC->pSrcList!=0 );
  sqlite3_Walk_വ്യഞ്ജകം(&w, p_വ്യഞ്ജകം);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
ശൂന്യം sqlite3_വ്യഞ്ജകം_Analyze_Agg_List(NameContext *pNC, വ്യഞ്ജകം_List *pList){
  ഘടന വ്യഞ്ജകം_List_item *pItem;
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( pList ){
    വേണ്ടി(pItem=pList->a, i=0; i<pList->n_വ്യഞ്ജകം; i++, pItem++){
      sqlite3_വ്യഞ്ജകം_Analyze_Aggregates(pNC, pItem->p_വ്യഞ്ജകം);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
പൂർണ്ണസംഖ്യ sqlite3GetTempReg(Parse *pParse){
  എങ്കിൽ( pParse->nTempReg==0 ){
    തിരികെ ++pParse->nMem;
  }
  തിരികെ pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
ശൂന്യം sqlite3ReleaseTempReg(Parse *pParse, പൂർണ്ണസംഖ്യ iReg){
  എങ്കിൽ( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    പൂർണ്ണസംഖ്യ kk;
    വേണ്ടി(kk=0; kk<pParse->nTempReg; kk++){
      പ്രതിജ്ഞാ( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    എങ്കിൽ( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
പൂർണ്ണസംഖ്യ sqlite3GetTempRange(Parse *pParse, പൂർണ്ണസംഖ്യ nReg){
  പൂർണ്ണസംഖ്യ i, n;
  എങ്കിൽ( nReg==1 ) തിരികെ sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  എങ്കിൽ( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }അല്ലെങ്കിൽ{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  തിരികെ i;
}
ശൂന്യം sqlite3ReleaseTempRange(Parse *pParse, പൂർണ്ണസംഖ്യ iReg, പൂർണ്ണസംഖ്യ nReg){
  എങ്കിൽ( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    തിരികെ;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  എങ്കിൽ( nReg>pParse->nRangeReg ){
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
ശൂന്യം sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
ശൂന്യം sqlite3TouchRegister(Parse *pParse, പൂർണ്ണസംഖ്യ iReg){
  എങ്കിൽ( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#എങ്കിൽ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
പൂർണ്ണസംഖ്യ sqlite3FirstAvailableRegister(Parse *pParse, പൂർണ്ണസംഖ്യ iMin){
  സ്ഥിരം വ്യഞ്ജകം_List *pList = pParse->p_Const_വ്യഞ്ജകം;
  എങ്കിൽ( pList ){
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<pList->n_വ്യഞ്ജകം; i++){
      എങ്കിൽ( pList->a[i].u.i_Const_വ്യഞ്ജകം_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_വ്യഞ്ജകം_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  തിരികെ iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
പൂർണ്ണസംഖ്യ sqlite3NoTempsInRange(Parse *pParse, പൂർണ്ണസംഖ്യ iFirst, പൂർണ്ണസംഖ്യ iLast){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     തിരികെ 0;
  }
  വേണ്ടി(i=0; i<pParse->nTempReg; i++){
    എങ്കിൽ( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      തിരികെ 0;
    }
  }
  എങ്കിൽ( pParse->p_Const_വ്യഞ്ജകം ){
    വ്യഞ്ജകം_List *pList = pParse->p_Const_വ്യഞ്ജകം;
    വേണ്ടി(i=0; i<pList->n_വ്യഞ്ജകം; i++){
      പൂർണ്ണസംഖ്യ iReg = pList->a[i].u.i_Const_വ്യഞ്ജകം_Reg;
      എങ്കിൽ( iReg==0 ) തുടരുക;
      എങ്കിൽ( iReg>=iFirst && iReg<=iLast ) തിരികെ 0;
    }
  }
  തിരികെ 1;
}
#endif /* SQLITE_DEBUG */
