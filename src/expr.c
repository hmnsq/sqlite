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
#अन्तर्भाव "sqliteInt.h"

/* Forward declarations */
स्थिर शून्य व्यञ्जकम्_Code_Between(Parse*,व्यञ्जकम्*,पूर्णाङ्क,शून्य(*)(Parse*,व्यञ्जकम्*,पूर्णाङ्क,पूर्णाङ्क),पूर्णाङ्क);
स्थिर पूर्णाङ्क व्यञ्जकम्_Code_Vector(Parse *pParse, व्यञ्जकम् *p, पूर्णाङ्क *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
वर्ण sqlite3TableColumnAffinity(ध्रुव Table *pTab, पूर्णाङ्क iCol){
  जइ( iCol<0 || NEVER(iCol>=pTab->nCol) ) प्रत्यागम SQLITE_AFF_INTEGER;
  प्रत्यागम pTab->aCol[iCol].affinity;
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
वर्ण sqlite3_व्यञ्जकम्_Affinity(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क op;
  op = p_व्यञ्जकम्->op;
  जाव( 1 /* exit-by-break */ ){
    जइ( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_व्यञ्जकम्->y.pTab!=0) ){
      प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) );
      प्रतिज्ञा( p_व्यञ्जकम्->y.pTab!=0 );
      प्रत्यागम sqlite3TableColumnAffinity(p_व्यञ्जकम्->y.pTab, p_व्यञ्जकम्->iColumn);
    }
    जइ( op==TK_SELECT ){
      प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) );
      प्रतिज्ञा( p_व्यञ्जकम्->x.pSelect!=0 );
      प्रतिज्ञा( p_व्यञ्जकम्->x.pSelect->pEList!=0 );
      प्रतिज्ञा( p_व्यञ्जकम्->x.pSelect->pEList->a[0].p_व्यञ्जकम्!=0 );
      प्रत्यागम sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्->x.pSelect->pEList->a[0].p_व्यञ्जकम्);
    }
#ifndef SQLITE_OMIT_CAST
    जइ( op==TK_CAST ){
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      प्रत्यागम sqlite3AffinityType(p_व्यञ्जकम्->u.zToken, 0);
    }
#endif
    जइ( op==TK_SELECT_COLUMN ){
      प्रतिज्ञा( p_व्यञ्जकम्->p_वामः!=0 && व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्->p_वामः) );
      प्रतिज्ञा( p_व्यञ्जकम्->iColumn < p_व्यञ्जकम्->iTable );
      प्रतिज्ञा( p_व्यञ्जकम्->iColumn >= 0 );
      प्रतिज्ञा( p_व्यञ्जकम्->iTable==p_व्यञ्जकम्->p_वामः->x.pSelect->pEList->n_व्यञ्जकम् );
      प्रत्यागम sqlite3_व्यञ्जकम्_Affinity(
          p_व्यञ्जकम्->p_वामः->x.pSelect->pEList->a[p_व्यञ्जकम्->iColumn].p_व्यञ्जकम्
      );
    }
    जइ( op==TK_VECTOR
     || (op==TK_FUNCTION && p_व्यञ्जकम्->aff_व्यञ्जकम्==SQLITE_AFF_DEFER)
    ){
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
      प्रत्यागम sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्->x.pList->a[0].p_व्यञ्जकम्);
    }
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Skip|EP_IfNullRow) ){
      प्रतिज्ञा( p_व्यञ्जकम्->op==TK_COLLATE
           || p_व्यञ्जकम्->op==TK_IF_NULL_ROW
           || (p_व्यञ्जकम्->op==TK_REGISTER && p_व्यञ्जकम्->op2==TK_IF_NULL_ROW) );
      p_व्यञ्जकम् = p_व्यञ्जकम्->p_वामः;
      op = p_व्यञ्जकम्->op;
      अनुवर्त;
    }
    जइ( op!=TK_REGISTER ) भङ्ग;
    op = p_व्यञ्जकम्->op2;
    जइ( NEVER( op==TK_REGISTER ) ) भङ्ग;
  }
  प्रत्यागम p_व्यञ्जकम्->aff_व्यञ्जकम्;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Data_Type(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  जाव( p_व्यञ्जकम् ){
    विकल्प( p_व्यञ्जकम्->op ){
      पक्ष TK_COLLATE:
      पक्ष TK_IF_NULL_ROW:
      पक्ष TK_UPLUS:  {
        p_व्यञ्जकम् = p_व्यञ्जकम्->p_वामः;
        भङ्ग;
      }
      पक्ष TK_NULL: {
        p_व्यञ्जकम् = 0;
        भङ्ग;
      }
      पक्ष TK_STRING: {
        प्रत्यागम 0x02;
      }
      पक्ष TK_BLOB: {
        प्रत्यागम 0x04;
      }
      पक्ष TK_CONCAT: {
        प्रत्यागम 0x06;
      }
      पक्ष TK_VARIABLE:
      पक्ष TK_AGG_FUNCTION:
      पक्ष TK_FUNCTION: {
        प्रत्यागम 0x07;
      }
      पक्ष TK_COLUMN:
      पक्ष TK_AGG_COLUMN:
      पक्ष TK_SELECT:
      पक्ष TK_CAST:
      पक्ष TK_SELECT_COLUMN:
      पक्ष TK_VECTOR:  {
        पूर्णाङ्क aff = sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्);
        जइ( aff>=SQLITE_AFF_NUMERIC ) प्रत्यागम 0x05;
        जइ( aff==SQLITE_AFF_TEXT )    प्रत्यागम 0x06;
        प्रत्यागम 0x07;
      }
      पक्ष TK_CASE: {
        पूर्णाङ्क res = 0;
        पूर्णाङ्क ii;
        व्यञ्जकम्_List *pList = p_व्यञ्जकम्->x.pList;
        प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) && pList!=0 );
        प्रतिज्ञा( pList->n_व्यञ्जकम् > 0);
        प्रति(ii=1; ii<pList->n_व्यञ्जकम्; ii+=2){
          res |= sqlite3_व्यञ्जकम्_Data_Type(pList->a[ii].p_व्यञ्जकम्);
        }
        जइ( pList->n_व्यञ्जकम् % 2 ){
          res |= sqlite3_व्यञ्जकम्_Data_Type(pList->a[pList->n_व्यञ्जकम्-1].p_व्यञ्जकम्);
        }
        प्रत्यागम res;
      }
      स्वभाव: {
        प्रत्यागम 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  प्रत्यागम 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,             /* Add the "COLLATE" clause to this expression */
  ध्रुव Token *pCollName,  /* Name of collating sequence */
  पूर्णाङ्क dequote              /* True to dequote pCollName */
){
  जइ( pCollName->n>0 && pParse->nErr==0 ){
    व्यञ्जकम् *pNew = sqlite3_व्यञ्जकम्_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    प्रतिज्ञा( p_व्यञ्जकम्!=0 );
    जइ( pNew ){
      pNew->p_वामः = p_व्यञ्जकम्;
      pNew->flags |= EP_Collate|EP_Skip;
#जइ SQLITE_MAX_व्यञ्जकम्_DEPTH>0
      pNew->nHeight = p_व्यञ्जकम्->nHeight+1;
      sqlite3_व्यञ्जकम्_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_व्यञ्जकम् = pNew;
    }
  }
  प्रत्यागम p_व्यञ्जकम्;
}
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,          /* Add the "COLLATE" clause to this expression */
  ध्रुव वर्ण *zC        /* The collating sequence name */
){
  Token s;
  प्रतिज्ञा( zC!=0 );
  sqlite3TokenInit(&s, (वर्ण*)zC);
  प्रत्यागम sqlite3_व्यञ्जकम्_Add_Collate_Token(pParse, p_व्यञ्जकम्, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Skip_Collate(व्यञ्जकम् *p_व्यञ्जकम्){
  जाव( p_व्यञ्जकम् && व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Skip) ){
    प्रतिज्ञा( p_व्यञ्जकम्->op==TK_COLLATE );
    p_व्यञ्जकम् = p_व्यञ्जकम्->p_वामः;
  }  
  प्रत्यागम p_व्यञ्जकम्;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(व्यञ्जकम् *p_व्यञ्जकम्){
  जाव( p_व्यञ्जकम् && व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Skip|EP_Unlikely) ){
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Unlikely) ){
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
      प्रतिज्ञा( p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्>0 );
      प्रतिज्ञा( p_व्यञ्जकम्->op==TK_FUNCTION );
      p_व्यञ्जकम् = p_व्यञ्जकम्->x.pList->a[0].p_व्यञ्जकम्;
    }अण्णहा जइ( p_व्यञ्जकम्->op==TK_COLLATE ){
      p_व्यञ्जकम् = p_व्यञ्जकम्->p_वामः;
    }अण्णहा{
      भङ्ग;
    }
  }  
  प्रत्यागम p_व्यञ्जकम्;
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
CollSeq *sqlite3_व्यञ्जकम्_Coll_Seq(Parse *pParse, ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ध्रुव व्यञ्जकम् *p = p_व्यञ्जकम्;
  जाव( p ){
    पूर्णाङ्क op = p->op;
    जइ( op==TK_REGISTER ) op = p->op2;
    जइ( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      पूर्णाङ्क j;
      प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p) );
      प्रतिज्ञा( p->y.pTab!=0 );
      जइ( (j = p->iColumn)>=0 ){
        ध्रुव वर्ण *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      भङ्ग;
    }
    जइ( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_वामः;
      अनुवर्त;
    }
    जइ( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_व्यञ्जकम्==SQLITE_AFF_DEFER)
    ){
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p) );
      p = p->x.pList->a[0].p_व्यञ्जकम्;
      अनुवर्त;
    }
    जइ( op==TK_COLLATE ){
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p, EP_Int_मानम्) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      भङ्ग;
    }
    जइ( p->flags & EP_Collate ){
      जइ( p->p_वामः && (p->p_वामः->flags & EP_Collate)!=0 ){
        p = p->p_वामः;
      }अण्णहा{
        व्यञ्जकम् *pNext  = p->p_दक्षिणः;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        प्रतिज्ञा( !व्यञ्जकम्_Use_XList(p) || p->x.pList==0 || p->p_दक्षिणः==0 );
        जइ( व्यञ्जकम्_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          पूर्णाङ्क i;
          प्रति(i=0; i<p->x.pList->n_व्यञ्जकम्; i++){
            जइ( व्यञ्जकम्_Has_Property(p->x.pList->a[i].p_व्यञ्जकम्, EP_Collate) ){
              pNext = p->x.pList->a[i].p_व्यञ्जकम्;
              भङ्ग;
            }
          }
        }
        p = pNext;
      }
    }अण्णहा{
      भङ्ग;
    }
  }
  जइ( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  प्रत्यागम pColl;
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
CollSeq *sqlite3_व्यञ्जकम्_NNColl_Seq(Parse *pParse, ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  CollSeq *p = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p_व्यञ्जकम्);
  जइ( p==0 ) p = pParse->db->pDfltColl;
  प्रतिज्ञा( p!=0 );
  प्रत्यागम p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Coll_Seq_Match(Parse *pParse, ध्रुव व्यञ्जकम् *pE1, ध्रुव व्यञ्जकम् *pE2){
  CollSeq *pColl1 = sqlite3_व्यञ्जकम्_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_व्यञ्जकम्_NNColl_Seq(pParse, pE2);
  प्रतिज्ञा( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  प्रत्यागम pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
वर्ण sqlite3CompareAffinity(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्, वर्ण aff2){
  वर्ण aff1 = sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्);
  जइ( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    जइ( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      प्रत्यागम SQLITE_AFF_NUMERIC;
    }अण्णहा{
      प्रत्यागम SQLITE_AFF_BLOB;
    }
  }अण्णहा{
    /* One side is a column, the other is not. Use the columns affinity. */
    प्रतिज्ञा( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    प्रत्यागम (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
स्थिर वर्ण comparisonAffinity(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  वर्ण aff;
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_EQ || p_व्यञ्जकम्->op==TK_IN || p_व्यञ्जकम्->op==TK_LT ||
          p_व्यञ्जकम्->op==TK_GT || p_व्यञ्जकम्->op==TK_GE || p_व्यञ्जकम्->op==TK_LE ||
          p_व्यञ्जकम्->op==TK_NE || p_व्यञ्जकम्->op==TK_IS || p_व्यञ्जकम्->op==TK_ISNOT );
  प्रतिज्ञा( p_व्यञ्जकम्->p_वामः );
  प्रतिज्ञा( !व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) );
  aff = sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्->p_वामः);
  जइ( p_व्यञ्जकम्->p_दक्षिणः ){
    aff = sqlite3CompareAffinity(p_व्यञ्जकम्->p_दक्षिणः, aff);
  }अण्णहा जइ( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  प्रत्यागम aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
पूर्णाङ्क sqlite3IndexAffinityOk(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्, वर्ण idx_affinity){
  वर्ण aff = comparisonAffinity(p_व्यञ्जकम्);
  जइ( aff<SQLITE_AFF_TEXT ){
    प्रत्यागम 1;
  }
  जइ( aff==SQLITE_AFF_TEXT ){
    प्रत्यागम idx_affinity==SQLITE_AFF_TEXT;
  }
  प्रत्यागम sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
स्थिर u8 द्विमान_Compare_P5(
  ध्रुव व्यञ्जकम् *pExpr1,   /* Left operand */
  ध्रुव व्यञ्जकम् *pExpr2,   /* Right operand */
  पूर्णाङ्क jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (वर्ण)sqlite3_व्यञ्जकम्_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  प्रत्यागम aff;
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
CollSeq *sqlite3_द्विमान_Compare_Coll_Seq(
  Parse *pParse,
  ध्रुव व्यञ्जकम् *p_वामः,
  ध्रुव व्यञ्जकम् *p_दक्षिणः
){
  CollSeq *pColl;
  प्रतिज्ञा( p_वामः );
  जइ( p_वामः->flags & EP_Collate ){
    pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p_वामः);
  }अण्णहा जइ( p_दक्षिणः && (p_दक्षिणः->flags & EP_Collate)!=0 ){
    pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p_दक्षिणः);
  }अण्णहा{
    pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p_वामः);
    जइ( !pColl ){
      pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p_दक्षिणः);
    }
  }
  प्रत्यागम pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_व्यञ्जकम्_Compare_Coll_Seq(Parse *pParse, ध्रुव व्यञ्जकम् *p){
  जइ( व्यञ्जकम्_Has_Property(p, EP_Commuted) ){
    प्रत्यागम sqlite3_द्विमान_Compare_Coll_Seq(pParse, p->p_दक्षिणः, p->p_वामः);
  }अण्णहा{
    प्रत्यागम sqlite3_द्विमान_Compare_Coll_Seq(pParse, p->p_वामः, p->p_दक्षिणः);
  }
}

/*
** Generate code for a comparison operator.
*/
स्थिर पूर्णाङ्क codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  व्यञ्जकम् *p_वामः,      /* The left operand */
  व्यञ्जकम् *p_दक्षिणः,     /* The right operand */
  पूर्णाङ्क opcode,       /* The comparison opcode */
  पूर्णाङ्क in1, पूर्णाङ्क in2, /* Register holding operands */
  पूर्णाङ्क dest,         /* Jump here if true.  */
  पूर्णाङ्क jumpIfNull,   /* If true, jump if either operand is NULL */
  पूर्णाङ्क isCommuted    /* The comparison has been commuted */
){
  पूर्णाङ्क p5;
  पूर्णाङ्क addr;
  CollSeq *p4;

  जइ( pParse->nErr ) प्रत्यागम 0;
  जइ( isCommuted ){
    p4 = sqlite3_द्विमान_Compare_Coll_Seq(pParse, p_दक्षिणः, p_वामः);
  }अण्णहा{
    p4 = sqlite3_द्विमान_Compare_Coll_Seq(pParse, p_वामः, p_दक्षिणः);
  }
  p5 = द्विमान_Compare_P5(p_वामः, p_दक्षिणः, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (शून्य*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  प्रत्यागम addr;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Vector(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  प्रत्यागम sqlite3_व्यञ्जकम्_Vector_Size(p_व्यञ्जकम्)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Vector_Size(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  u8 op = p_व्यञ्जकम्->op;
  जइ( op==TK_REGISTER ) op = p_व्यञ्जकम्->op2;
  जइ( op==TK_VECTOR ){
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
    प्रत्यागम p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्;
  }अण्णहा जइ( op==TK_SELECT ){
    प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) );
    प्रत्यागम p_व्यञ्जकम्->x.pSelect->pEList->n_व्यञ्जकम्;
  }अण्णहा{
    प्रत्यागम 1;
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
व्यञ्जकम् *sqlite3VectorFieldSubexpr(व्यञ्जकम् *pVector, पूर्णाङ्क i){
  प्रतिज्ञा( i<sqlite3_व्यञ्जकम्_Vector_Size(pVector) || pVector->op==TK_ERROR );
  जइ( sqlite3_व्यञ्जकम्_Is_Vector(pVector) ){
    प्रतिज्ञा( pVector->op2==0 || pVector->op==TK_REGISTER );
    जइ( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(pVector) );
      प्रत्यागम pVector->x.pSelect->pEList->a[i].p_व्यञ्जकम्;
    }अण्णहा{
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(pVector) );
      प्रत्यागम pVector->x.pList->a[i].p_व्यञ्जकम्;
    }
  }
  प्रत्यागम pVector;
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
व्यञ्जकम् *sqlite3_व्यञ्जकम्_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  व्यञ्जकम् *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  पूर्णाङ्क iField,          /* Which column of the vector to return */
  पूर्णाङ्क nField           /* Total number of columns in the vector */
){
  व्यञ्जकम् *pRet;
  जइ( pVector->op==TK_SELECT ){
    प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(pVector) );
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
    जइ( pRet ){
      व्यञ्जकम्_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_वामः = pVector;
    }
  }अण्णहा{
    जइ( pVector->op==TK_VECTOR ){
      व्यञ्जकम् **ppVector;
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_व्यञ्जकम्;
      pVector = *ppVector;
      जइ( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        प्रत्यागम pVector;
      }
    }
    pRet = sqlite3_व्यञ्जकम्_Dup(pParse->db, pVector, 0);
  }
  प्रत्यागम pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Code_Subselect(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  जइ( p_व्यञ्जकम्->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_व्यञ्जकम्);
  }
#endif
  प्रत्यागम reg;
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
स्थिर पूर्णाङ्क व्यञ्जकम्_Vector_Register(
  Parse *pParse,                  /* Parse context */
  व्यञ्जकम् *pVector,                  /* Vector to extract element from */
  पूर्णाङ्क iField,                     /* Field to extract from pVector */
  पूर्णाङ्क regSelect,                  /* First in array of registers */
  व्यञ्जकम् *pTmp,                     /* Temporary space */
  व्यञ्जकम् **pp_व्यञ्जकम्,                  /* OUT: Expression element */
  पूर्णाङ्क *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  प्रतिज्ञा( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  जइ( op==TK_REGISTER ){
    *pp_व्यञ्जकम् = sqlite3VectorFieldSubexpr(pVector, iField);
    प्रत्यागम pVector->iTable+iField;
  }
  जइ( op==TK_SELECT ){
    प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, परिमाण(व्यञ्जकम्));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_वामः = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_व्यञ्जकम्;
    *pp_व्यञ्जकम् = pTmp;
    प्रत्यागम regSelect+iField;
  }
  जइ( op==TK_VECTOR ){
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(pVector) );
    *pp_व्यञ्जकम् = pVector->x.pList->a[iField].p_व्यञ्जकम्;
    प्रत्यागम sqlite3_व्यञ्जकम्_Code_Temp(pParse, *pp_व्यञ्जकम्, pRegFree);
  }
  प्रत्यागम 0;
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
स्थिर शून्य codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  व्यञ्जकम् *p_व्यञ्जकम्,          /* The comparison operation */
  पूर्णाङ्क dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  व्यञ्जकम् *p_वामः = p_व्यञ्जकम्->p_वामः;
  व्यञ्जकम् *p_दक्षिणः = p_व्यञ्जकम्->p_दक्षिणः;
  पूर्णाङ्क n_वामः = sqlite3_व्यञ्जकम्_Vector_Size(p_वामः);
  पूर्णाङ्क i;
  पूर्णाङ्क reg_वामः = 0;
  पूर्णाङ्क reg_दक्षिणः = 0;
  u8 opx = op;
  पूर्णाङ्क addrCmp = 0;
  पूर्णाङ्क addrDone = sqlite3VdbeMakeLabel(pParse);
  पूर्णाङ्क isCommuted = व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_Commuted);

  प्रतिज्ञा( !व्यञ्जकम्_Has_VVAProperty(p_व्यञ्जकम्,EP_Immutable) );
  जइ( pParse->nErr ) प्रत्यागम;
  जइ( n_वामः!=sqlite3_व्यञ्जकम्_Vector_Size(p_दक्षिणः) ){
    sqlite3_Error_सन्देशः(pParse, "row value misused");
    प्रत्यागम;
  }
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_EQ || p_व्यञ्जकम्->op==TK_NE
       || p_व्यञ्जकम्->op==TK_IS || p_व्यञ्जकम्->op==TK_ISNOT
       || p_व्यञ्जकम्->op==TK_LT || p_व्यञ्जकम्->op==TK_GT
       || p_व्यञ्जकम्->op==TK_LE || p_व्यञ्जकम्->op==TK_GE
  );
  प्रतिज्ञा( p_व्यञ्जकम्->op==op || (p_व्यञ्जकम्->op==TK_IS && op==TK_EQ)
            || (p_व्यञ्जकम्->op==TK_ISNOT && op==TK_NE) );
  प्रतिज्ञा( p5==0 || p_व्यञ्जकम्->op!=op );
  प्रतिज्ञा( p5==SQLITE_NULLEQ || p_व्यञ्जकम्->op==op );

  जइ( op==TK_LE ) opx = TK_LT;
  जइ( op==TK_GE ) opx = TK_GT;
  जइ( op==TK_NE ) opx = TK_EQ;

  reg_वामः = व्यञ्जकम्_Code_Subselect(pParse, p_वामः);
  reg_दक्षिणः = व्यञ्जकम्_Code_Subselect(pParse, p_दक्षिणः);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  प्रति(i=0; 1 /*Loop exits by "break"*/; i++){
    पूर्णाङ्क regFree1 = 0, regFree2 = 0;
    व्यञ्जकम् *pL = 0, *pR = 0;
    व्यञ्जकम् tmp1, tmp2;
    पूर्णाङ्क r1, r2;
    प्रतिज्ञा( i>=0 && i<n_वामः );
    जइ( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = व्यञ्जकम्_Vector_Register(pParse, p_वामः, i, reg_वामः, &tmp1, &pL, &regFree1);
    r2 = व्यञ्जकम्_Vector_Register(pParse, p_दक्षिणः, i, reg_दक्षिणः, &tmp2, &pR, &regFree2);
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
    जइ( (opx==TK_LT || opx==TK_GT) && i<n_वामः-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    जइ( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }अण्णहा{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    जइ( i==n_वामः-1 ){
      भङ्ग;
    }
    जइ( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }अण्णहा{
      प्रतिज्ञा( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      जइ( i==n_वामः-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  जइ( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#जइ SQLITE_MAX_व्यञ्जकम्_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Check_Height(Parse *pParse, पूर्णाङ्क nHeight){
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क mxHeight = pParse->db->aLimit[SQLITE_LIMIT_व्यञ्जकम्_DEPTH];
  जइ( nHeight>mxHeight ){
    sqlite3_Error_सन्देशः(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  प्रत्यागम rc;
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
स्थिर शून्य height_Of_व्यञ्जकम्(ध्रुव व्यञ्जकम् *p, पूर्णाङ्क *pnHeight){
  जइ( p ){
    जइ( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
स्थिर शून्य height_Of_व्यञ्जकम्_List(ध्रुव व्यञ्जकम्_List *p, पूर्णाङ्क *pnHeight){
  जइ( p ){
    पूर्णाङ्क i;
    प्रति(i=0; i<p->n_व्यञ्जकम्; i++){
      height_Of_व्यञ्जकम्(p->a[i].p_व्यञ्जकम्, pnHeight);
    }
  }
}
स्थिर शून्य heightOfSelect(ध्रुव Select *pSelect, पूर्णाङ्क *pnHeight){
  ध्रुव Select *p;
  प्रति(p=pSelect; p; p=p->pPrior){
    height_Of_व्यञ्जकम्(p->pWhere, pnHeight);
    height_Of_व्यञ्जकम्(p->pHaving, pnHeight);
    height_Of_व्यञ्जकम्(p->pLimit, pnHeight);
    height_Of_व्यञ्जकम्_List(p->pEList, pnHeight);
    height_Of_व्यञ्जकम्_List(p->pGroupBy, pnHeight);
    height_Of_व्यञ्जकम्_List(p->pOrderBy, pnHeight);
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
स्थिर शून्य व्यञ्जकम्_Set_Height(व्यञ्जकम् *p){
  पूर्णाङ्क nHeight = p->p_वामः ? p->p_वामः->nHeight : 0;
  जइ( NEVER(p->p_दक्षिणः) && p->p_दक्षिणः->nHeight>nHeight ){
    nHeight = p->p_दक्षिणः->nHeight;
  }
  जइ( व्यञ्जकम्_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }अण्णहा जइ( p->x.pList ){
    height_Of_व्यञ्जकम्_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_व्यञ्जकम्_List_Flags(p->x.pList);
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
शून्य sqlite3_व्यञ्जकम्_Set_Height_And_Flags(Parse *pParse, व्यञ्जकम् *p){
  जइ( pParse->nErr ) प्रत्यागम;
  व्यञ्जकम्_Set_Height(p);
  sqlite3_व्यञ्जकम्_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
पूर्णाङ्क sqlite3_Select_व्यञ्जकम्_Height(ध्रुव Select *p){
  पूर्णाङ्क nHeight = 0;
  heightOfSelect(p, &nHeight);
  प्रत्यागम nHeight;
}
#अण्णहा /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
शून्य sqlite3_व्यञ्जकम्_Set_Height_And_Flags(Parse *pParse, व्यञ्जकम् *p){
  जइ( pParse->nErr ) प्रत्यागम;
  जइ( p && व्यञ्जकम्_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_व्यञ्जकम्_List_Flags(p->x.pList);
  }
}
#define व्यञ्जकम्_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
शून्य sqlite3_व्यञ्जकम्_Set_Error_Offset(व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क iOfst){
  जइ( p_व्यञ्जकम्==0 ) प्रत्यागम;
  जइ( NEVER(व्यञ्जकम्_Use_WJoin(p_व्यञ्जकम्)) ) प्रत्यागम;
  p_व्यञ्जकम्->w.iOfst = iOfst;
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
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  पूर्णाङ्क op,                 /* Expression opcode */
  ध्रुव Token *pToken,    /* Token argument.  Might be NULL */
  पूर्णाङ्क dequote             /* True to dequote */
){
  व्यञ्जकम् *pNew;
  पूर्णाङ्क nExtra = pToken ? pToken->n+1 : 0;

  प्रतिज्ञा( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, परिमाण(व्यञ्जकम्)+nExtra);
  जइ( pNew ){
    memset(pNew, 0, परिमाण(व्यञ्जकम्));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    जइ( nExtra ){
      प्रतिज्ञा( pToken!=0 );
      pNew->u.zToken = (वर्ण*)&pNew[1];
      प्रतिज्ञा( pToken->z!=0 || pToken->n==0 );
      जइ( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      जइ( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_व्यञ्जकम्(pNew);
      }
    }
#जइ SQLITE_MAX_व्यञ्जकम्_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  प्रत्यागम pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
व्यञ्जकम् *sqlite3_व्यञ्जकम्(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  पूर्णाङ्क op,                 /* Expression opcode */
  ध्रुव वर्ण *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  प्रत्यागम sqlite3_व्यञ्जकम्_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Int32(sqlite3 *db, पूर्णाङ्क iVal){
  व्यञ्जकम् *pNew = sqlite3DbMallocRawNN(db, परिमाण(व्यञ्जकम्));
  जइ( pNew ){
    memset(pNew, 0, परिमाण(व्यञ्जकम्));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_मानम्|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_मानम् = iVal;
#जइ SQLITE_MAX_व्यञ्जकम्_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  प्रत्यागम pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
शून्य sqlite3_व्यञ्जकम्_Attach_Subtrees(
  sqlite3 *db,
  व्यञ्जकम् *p_मूलम्,
  व्यञ्जकम् *p_वामः,
  व्यञ्जकम् *p_दक्षिणः
){
  जइ( p_मूलम्==0 ){
    प्रतिज्ञा( db->mallocFailed );
    sqlite3_व्यञ्जकम्_Delete(db, p_वामः);
    sqlite3_व्यञ्जकम्_Delete(db, p_दक्षिणः);
  }अण्णहा{
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_मूलम्) );
    प्रतिज्ञा( p_मूलम्->x.pSelect==0 );
    जइ( p_दक्षिणः ){
      p_मूलम्->p_दक्षिणः = p_दक्षिणः;
      p_मूलम्->flags |= EP_Propagate & p_दक्षिणः->flags;
#जइ SQLITE_MAX_व्यञ्जकम्_DEPTH>0
      p_मूलम्->nHeight = p_दक्षिणः->nHeight+1;
    }अण्णहा{
      p_मूलम्->nHeight = 1;
#endif
    }
    जइ( p_वामः ){
      p_मूलम्->p_वामः = p_वामः;
      p_मूलम्->flags |= EP_Propagate & p_वामः->flags;
#जइ SQLITE_MAX_व्यञ्जकम्_DEPTH>0
      जइ( p_वामः->nHeight>=p_मूलम्->nHeight ){
        p_मूलम्->nHeight = p_वामः->nHeight+1;
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
व्यञ्जकम् *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  पूर्णाङ्क op,                 /* Expression opcode */
  व्यञ्जकम् *p_वामः,            /* Left operand */
  व्यञ्जकम् *p_दक्षिणः            /* Right operand */
){
  व्यञ्जकम् *p;
  p = sqlite3DbMallocRawNN(pParse->db, परिमाण(व्यञ्जकम्));
  जइ( p ){
    memset(p, 0, परिमाण(व्यञ्जकम्));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_व्यञ्जकम्_Attach_Subtrees(pParse->db, p, p_वामः, p_दक्षिणः);
    sqlite3_व्यञ्जकम्_Check_Height(pParse, p->nHeight);
  }अण्णहा{
    sqlite3_व्यञ्जकम्_Delete(pParse->db, p_वामः);
    sqlite3_व्यञ्जकम्_Delete(pParse->db, p_दक्षिणः);
  }
  प्रत्यागम p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
शून्य sqlite3PExprAddSelect(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, Select *pSelect){
  जइ( p_व्यञ्जकम् ){
    p_व्यञ्जकम्->x.pSelect = pSelect;
    व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, EP_xIsSelect|EP_Subquery);
    sqlite3_व्यञ्जकम्_Set_Height_And_Flags(pParse, p_व्यञ्जकम्);
  }अण्णहा{
    प्रतिज्ञा( pParse->db->mallocFailed );
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
Select *sqlite3_व्यञ्जकम्_List_To_मानानि(Parse *pParse, पूर्णाङ्क nElem, व्यञ्जकम्_List *pEList){
  पूर्णाङ्क ii;
  Select *pRet = 0;
  प्रतिज्ञा( nElem>1 );
  प्रति(ii=0; ii<pEList->n_व्यञ्जकम्; ii++){
    Select *pSel;
    व्यञ्जकम् *p_व्यञ्जकम् = pEList->a[ii].p_व्यञ्जकम्;
    पूर्णाङ्क n_व्यञ्जकम्_Elem;
    जइ( p_व्यञ्जकम्->op==TK_VECTOR ){
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
      n_व्यञ्जकम्_Elem = p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्;
    }अण्णहा{
      n_व्यञ्जकम्_Elem = 1;
    }
    जइ( n_व्यञ्जकम्_Elem!=nElem ){
      sqlite3_Error_सन्देशः(pParse, "IN(...) element has %d term%s - expected %d",
          n_व्यञ्जकम्_Elem, n_व्यञ्जकम्_Elem>1?"s":"", nElem
      );
      भङ्ग;
    }
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
    pSel = sqlite3SelectNew(pParse, p_व्यञ्जकम्->x.pList, 0, 0, 0, 0, 0, SF_मानानि,0);
    p_व्यञ्जकम्->x.pList = 0;
    जइ( pSel ){
      जइ( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  जइ( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_मानम्;
  }
  sqlite3_व्यञ्जकम्_List_Delete(pParse->db, pEList);
  प्रत्यागम pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
व्यञ्जकम् *sqlite3_व्यञ्जकम्_And(Parse *pParse, व्यञ्जकम् *p_वामः, व्यञ्जकम् *p_दक्षिणः){
  sqlite3 *db = pParse->db;
  जइ( p_वामः==0  ){
    प्रत्यागम p_दक्षिणः;
  }अण्णहा जइ( p_दक्षिणः==0 ){
    प्रत्यागम p_वामः;
  }अण्णहा{
    u32 f = p_वामः->flags | p_दक्षिणः->flags;
    जइ( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      व्यञ्जकम् *p;
      sqlite3_व्यञ्जकम्_Deferred_Delete(pParse, p_वामः);
      sqlite3_व्यञ्जकम्_Deferred_Delete(pParse, p_दक्षिणः);
      p = sqlite3_व्यञ्जकम्_Int32(db, 0);
      जइ( p ) p->op2 = 1;
      प्रत्यागम p;
    }अण्णहा{
      प्रत्यागम sqlite3PExpr(pParse, TK_AND, p_वामः, p_दक्षिणः);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Function(
  Parse *pParse,        /* Parsing context */
  व्यञ्जकम्_List *pList,      /* Argument list */
  ध्रुव Token *pToken,  /* Name of the function */
  पूर्णाङ्क eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  व्यञ्जकम् *pNew;
  sqlite3 *db = pParse->db;
  प्रतिज्ञा( pToken );
  pNew = sqlite3_व्यञ्जकम्_Alloc(db, TK_FUNCTION, pToken, 1);
  जइ( pNew==0 ){
    sqlite3_व्यञ्जकम्_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    प्रत्यागम 0;
  }
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (पूर्णाङ्क)(pToken->z - pParse->zTail);
  जइ( pList
   && pList->n_व्यञ्जकम् > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_सन्देशः(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  व्यञ्जकम्_Set_Property(pNew, EP_HasFunc);
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(pNew) );
  sqlite3_व्यञ्जकम्_Set_Height_And_Flags(pParse, pNew);
  जइ( eDistinct==SF_Distinct ) व्यञ्जकम्_Set_Property(pNew, EP_Distinct);
  प्रत्यागम pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
शून्य sqlite3_व्यञ्जकम्_Order_By_Aggregate_Error(Parse *pParse, व्यञ्जकम् *p){
  sqlite3_Error_सन्देशः(pParse,
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
शून्य sqlite3_व्यञ्जकम्_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,          /* The function call to which ORDER BY is to be added */
  व्यञ्जकम्_List *pOrderBy    /* The ORDER BY clause to add */
){
  व्यञ्जकम् *pOB;
  sqlite3 *db = pParse->db;
  जइ( NEVER(pOrderBy==0) ){
    प्रतिज्ञा( db->mallocFailed );
    प्रत्यागम;
  }
  जइ( p_व्यञ्जकम्==0 ){
    प्रतिज्ञा( db->mallocFailed );
    sqlite3_व्यञ्जकम्_List_Delete(db, pOrderBy);
    प्रत्यागम;
  }
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_FUNCTION );
  प्रतिज्ञा( p_व्यञ्जकम्->p_वामः==0 );
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
  जइ( p_व्यञ्जकम्->x.pList==0 || NEVER(p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_व्यञ्जकम्_List_Delete_Generic, pOrderBy);
    प्रत्यागम;
  }
  जइ( IsWindowFunc(p_व्यञ्जकम्) ){
    sqlite3_व्यञ्जकम्_Order_By_Aggregate_Error(pParse, p_व्यञ्जकम्);
    sqlite3_व्यञ्जकम्_List_Delete(db, pOrderBy);
    प्रत्यागम;
  }
  जइ( pOrderBy->n_व्यञ्जकम्>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_सन्देशः(pParse, "too many terms in ORDER BY clause");
    sqlite3_व्यञ्जकम्_List_Delete(db, pOrderBy);
    प्रत्यागम;
  }

  pOB = sqlite3_व्यञ्जकम्_Alloc(db, TK_ORDER, 0, 0);
  जइ( pOB==0 ){
    sqlite3_व्यञ्जकम्_List_Delete(db, pOrderBy);
    प्रत्यागम;
  }
  pOB->x.pList = pOrderBy;
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(pOB) );
  p_व्यञ्जकम्->p_वामः = pOB;
  व्यञ्जकम्_Set_Property(pOB, EP_FullSize);
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
शून्य sqlite3_व्यञ्जकम्_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ध्रुव व्यञ्जकम् *p_व्यञ्जकम्,     /* The function invocation */
  ध्रुव FuncDef *pDef    /* The function being invoked */
){
  प्रतिज्ञा( !IN_RENAME_OBJECT );
  प्रतिज्ञा( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    जइ( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_सन्देशः(pParse, "unsafe use of %#T()", p_व्यञ्जकम्);
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
शून्य sqlite3_व्यञ्जकम्_Assign_Var_सङ्ख्या(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, u32 n){
  sqlite3 *db = pParse->db;
  ध्रुव वर्ण *z;
  ynVar x;

  जइ( p_व्यञ्जकम्==0 ) प्रत्यागम;
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्|EP_Reduced|EP_TokenOnly) );
  z = p_व्यञ्जकम्->u.zToken;
  प्रतिज्ञा( z!=0 );
  प्रतिज्ञा( z[0]!=0 );
  प्रतिज्ञा( n==(u32)sqlite3Strlen30(z) );
  जइ( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    प्रतिज्ञा( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }अण्णहा{
    पूर्णाङ्क doAdd = 0;
    जइ( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      पूर्णाङ्क bOk;
      जइ( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }अण्णहा{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_सङ्ख्या]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_सङ्ख्या] );
      जइ( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_सङ्ख्या] ){
        sqlite3_Error_सन्देशः(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_सङ्ख्या]);
        sqlite3_Record_Error_Offset_Of_व्यञ्जकम्(pParse->db, p_व्यञ्जकम्);
        प्रत्यागम;
      }
      x = (ynVar)i;
      जइ( x>pParse->nVar ){
        pParse->nVar = (पूर्णाङ्क)x;
        doAdd = 1;
      }अण्णहा जइ( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }अण्णहा जइ( sqlite3_VList_अङ्कः_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }अण्णहा{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_अङ्कः(pParse->pVList, z, n);
      जइ( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    जइ( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      जइ( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_व्यञ्जकम्->iColumn = x;
  जइ( x>db->aLimit[SQLITE_LIMIT_VARIABLE_सङ्ख्या] ){
    sqlite3_Error_सन्देशः(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_व्यञ्जकम्(pParse->db, p_व्यञ्जकम्);
  }
}

/*
** Recursively delete an expression tree.
*/
स्थिर SQLITE_NOINLINE शून्य sqlite3_व्यञ्जकम्_Delete_NN(sqlite3 *db, व्यञ्जकम् *p){
  प्रतिज्ञा( p!=0 );
  प्रतिज्ञा( db!=0 );
व्यञ्जकम्_Delete_Restart:
  प्रतिज्ञा( !व्यञ्जकम्_Use_UValue(p) || p->u.i_मानम्>=0 );
  प्रतिज्ञा( !व्यञ्जकम्_Use_YWin(p) || !व्यञ्जकम्_Use_YSub(p) );
  प्रतिज्ञा( !व्यञ्जकम्_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  प्रतिज्ञा( p->op!=TK_FUNCTION || !व्यञ्जकम्_Use_YSub(p) );
#ifdef SQLITE_DEBUG
  जइ( व्यञ्जकम्_Has_Property(p, EP_Leaf) && !व्यञ्जकम्_Has_Property(p, EP_TokenOnly) ){
    प्रतिज्ञा( p->p_वामः==0 );
    प्रतिज्ञा( p->p_दक्षिणः==0 );
    प्रतिज्ञा( !व्यञ्जकम्_Use_XSelect(p) || p->x.pSelect==0 );
    प्रतिज्ञा( !व्यञ्जकम्_Use_XList(p) || p->x.pList==0 );
  }
#endif
  जइ( !व्यञ्जकम्_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    प्रतिज्ञा( (व्यञ्जकम्_Use_XList(p) && p->x.pList==0) || p->p_दक्षिणः==0 );
    जइ( p->p_दक्षिणः ){
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p, EP_WinFunc) );
      sqlite3_व्यञ्जकम्_Delete_NN(db, p->p_दक्षिणः);
    }अण्णहा जइ( व्यञ्जकम्_Use_XSelect(p) ){
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }अण्णहा{
      sqlite3_व्यञ्जकम्_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      जइ( व्यञ्जकम्_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    जइ( p->p_वामः && p->op!=TK_SELECT_COLUMN ){
      व्यञ्जकम् *p_वामः = p->p_वामः;
      जइ( !व्यञ्जकम्_Has_Property(p, EP_Static)
       && !व्यञ्जकम्_Has_Property(p_वामः, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_वामः;
        गच्छ व्यञ्जकम्_Delete_Restart;
      }अण्णहा{
        sqlite3_व्यञ्जकम्_Delete_NN(db, p_वामः);
      }
    }
  }
  जइ( !व्यञ्जकम्_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
शून्य sqlite3_व्यञ्जकम्_Delete(sqlite3 *db, व्यञ्जकम् *p){
  जइ( p ) sqlite3_व्यञ्जकम्_Delete_NN(db, p);
}
शून्य sqlite3_व्यञ्जकम्_Delete_Generic(sqlite3 *db, शून्य *p){
  जइ( ALWAYS(p) ) sqlite3_व्यञ्जकम्_Delete_NN(db, (व्यञ्जकम्*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
शून्य sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  जइ( p==0 ){
    /* Nothing to clear */
  }अण्णहा जइ( p->pOn ){
    sqlite3_व्यञ्जकम्_Delete_NN(db, p->pOn);
  }अण्णहा जइ( p->pUsing ){
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Deferred_Delete(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्){
  प्रत्यागम 0==sqlite3ParserAddCleanup(pParse, sqlite3_व्यञ्जकम्_Delete_Generic, p_व्यञ्जकम्);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
शून्य sqlite3_व्यञ्जकम्_Unmap_And_Delete(Parse *pParse, व्यञ्जकम् *p){
  जइ( p ){
    जइ( IN_RENAME_OBJECT ){
      sqlite3_Rename_व्यञ्जकम्_Unmap(pParse, p);
    }
    sqlite3_व्यञ्जकम्_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Struct_Size(ध्रुव व्यञ्जकम् *p){
  जइ( व्यञ्जकम्_Has_Property(p, EP_TokenOnly) ) प्रत्यागम व्यञ्जकम्_TOKENONLYSIZE;
  जइ( व्यञ्जकम्_Has_Property(p, EP_Reduced) ) प्रत्यागम व्यञ्जकम्_REDUCEDSIZE;
  प्रत्यागम व्यञ्जकम्_FULLSIZE;
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
स्थिर पूर्णाङ्क duped_व्यञ्जकम्_Struct_Size(ध्रुव व्यञ्जकम् *p, पूर्णाङ्क flags){
  पूर्णाङ्क nSize;
  प्रतिज्ञा( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  प्रतिज्ञा( व्यञ्जकम्_FULLSIZE<=0xfff );
  प्रतिज्ञा( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  जइ( 0==flags || व्यञ्जकम्_Has_Property(p, EP_FullSize) ){
    nSize = व्यञ्जकम्_FULLSIZE;
  }अण्णहा{
    प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p, EP_OuterON) );
    प्रतिज्ञा( !व्यञ्जकम्_Has_VVAProperty(p, EP_NoReduce) );
    जइ( p->p_वामः || p->x.pList ){
      nSize = व्यञ्जकम्_REDUCEDSIZE | EP_Reduced;
    }अण्णहा{
      प्रतिज्ञा( p->p_दक्षिणः==0 );
      nSize = व्यञ्जकम्_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  प्रत्यागम nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
स्थिर पूर्णाङ्क duped_व्यञ्जकम्_ग्रन्थिः_Size(ध्रुव व्यञ्जकम् *p, पूर्णाङ्क flags){
  पूर्णाङ्क nByte = duped_व्यञ्जकम्_Struct_Size(p, flags) & 0xfff;
  जइ( !व्यञ्जकम्_Has_Property(p, EP_Int_मानम्) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  प्रत्यागम ROUND8(nByte);
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
स्थिर पूर्णाङ्क duped_व्यञ्जकम्_Size(ध्रुव व्यञ्जकम् *p){
  पूर्णाङ्क nByte;
  प्रतिज्ञा( p!=0 );
  nByte = duped_व्यञ्जकम्_ग्रन्थिः_Size(p, EXPRDUP_REDUCE);
  जइ( p->p_वामः ) nByte += duped_व्यञ्जकम्_Size(p->p_वामः);
  जइ( p->p_दक्षिणः ) nByte += duped_व्यञ्जकम्_Size(p->p_दक्षिणः);
  प्रतिज्ञा( nByte==ROUND8(nByte) );
  प्रत्यागम nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
प्रकारनिर्देश संरचना EdupBuf EdupBuf;
संरचना EdupBuf {
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
स्थिर व्यञ्जकम् *व्यञ्जकम्_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ध्रुव व्यञ्जकम् *p,        /* Expr tree to be duplicated */
  पूर्णाङ्क dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  व्यञ्जकम् *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  पूर्णाङ्क nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  प्रतिज्ञा( db!=0 );
  प्रतिज्ञा( p );
  प्रतिज्ञा( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  प्रतिज्ञा( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  जइ( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    प्रतिज्ञा( sEdupBuf.zAlloc!=0 );
    प्रतिज्ञा( dupFlags==EXPRDUP_REDUCE );
  }अण्णहा{
    पूर्णाङ्क nAlloc;
    जइ( dupFlags ){
      nAlloc = duped_व्यञ्जकम्_Size(p);
    }अण्णहा जइ( !व्यञ्जकम्_Has_Property(p, EP_Int_मानम्) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(व्यञ्जकम्_FULLSIZE + nToken);
    }अण्णहा{
      nToken = 0;
      nAlloc = ROUND8(व्यञ्जकम्_FULLSIZE);
    }
    प्रतिज्ञा( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (व्यञ्जकम् *)sEdupBuf.zAlloc;
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(pNew) );

  जइ( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ध्रुव unsigned nStructSize = duped_व्यञ्जकम्_Struct_Size(p, dupFlags);
    पूर्णाङ्क nNewSize = nStructSize & 0xfff;
    जइ( nToken<0 ){
      जइ( !व्यञ्जकम्_Has_Property(p, EP_Int_मानम्) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }अण्णहा{
        nToken = 0;
      }
    }
    जइ( dupFlags ){
      प्रतिज्ञा( (पूर्णाङ्क)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      प्रतिज्ञा( व्यञ्जकम्_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }अण्णहा{
      u32 nSize = (u32)व्यञ्जकम्_Struct_Size(p);
      प्रतिज्ञा( (पूर्णाङ्क)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (पूर्णाङ्क)व्यञ्जकम्_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      जइ( nSize<व्यञ्जकम्_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, व्यञ्जकम्_FULLSIZE-nSize);
      }
      nNewSize = व्यञ्जकम्_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    व्यञ्जकम्_Clear_VVAProperties(pNew);
    जइ( dupFlags ){
      व्यञ्जकम्_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    प्रतिज्ञा( nToken>=0 );
    जइ( nToken>0 ){
      वर्ण *zToken = pNew->u.zToken = (वर्ण*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    जइ( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      जइ( व्यञ्जकम्_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }अण्णहा{
        pNew->x.pList = sqlite3_व्यञ्जकम्_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      जइ( व्यञ्जकम्_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        प्रतिज्ञा( व्यञ्जकम्_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      जइ( dupFlags ){
        जइ( p->op==TK_SELECT_COLUMN ){
          pNew->p_वामः = p->p_वामः;
          प्रतिज्ञा( p->p_दक्षिणः==0 
               || p->p_दक्षिणः==p->p_वामः
               || व्यञ्जकम्_Has_Property(p->p_वामः, EP_Subquery) );
        }अण्णहा{
          pNew->p_वामः = p->p_वामः ?
                      व्यञ्जकम्_Dup(db, p->p_वामः, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_दक्षिणः = p->p_दक्षिणः ?
                       व्यञ्जकम्_Dup(db, p->p_दक्षिणः, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }अण्णहा{
        जइ( p->op==TK_SELECT_COLUMN ){
          pNew->p_वामः = p->p_वामः;
          प्रतिज्ञा( p->p_दक्षिणः==0 
               || p->p_दक्षिणः==p->p_वामः
               || व्यञ्जकम्_Has_Property(p->p_वामः, EP_Subquery) );
        }अण्णहा{
          pNew->p_वामः = sqlite3_व्यञ्जकम्_Dup(db, p->p_वामः, 0);
        }
        pNew->p_दक्षिणः = sqlite3_व्यञ्जकम्_Dup(db, p->p_दक्षिणः, 0);
      }
    }
  }
  जइ( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, परिमाण(sEdupBuf));
  प्रतिज्ञा( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  प्रत्यागम pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  जइ( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    जइ( pRet ){
      पूर्णाङ्क i;
      pRet->nCte = p->nCte;
      प्रति(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_व्यञ्जकम्_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  प्रत्यागम pRet;
}
#अण्णहा
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
स्थिर पूर्णाङ्क gatherSelectWindowsCallback(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  जइ( p_व्यञ्जकम्->op==TK_FUNCTION && व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_व्यञ्जकम्->y.pWin;
    प्रतिज्ञा( pWin );
    प्रतिज्ञा( IsWindowFunc(p_व्यञ्जकम्) );
    प्रतिज्ञा( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  प्रत्यागम WRC_Continue;
}
स्थिर पूर्णाङ्क gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  प्रत्यागम p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
स्थिर शून्य gatherSelectWindows(Select *p){
  Walker w;
  w.x_व्यञ्जकम्_Callback = gatherSelectWindowsCallback;
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
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Dup(sqlite3 *db, ध्रुव व्यञ्जकम् *p, पूर्णाङ्क flags){
  प्रतिज्ञा( flags==0 || flags==EXPRDUP_REDUCE );
  प्रत्यागम p ? व्यञ्जकम्_Dup(db, p, flags, 0) : 0;
}
व्यञ्जकम्_List *sqlite3_व्यञ्जकम्_List_Dup(sqlite3 *db, ध्रुव व्यञ्जकम्_List *p, पूर्णाङ्क flags){
  व्यञ्जकम्_List *pNew;
  संरचना व्यञ्जकम्_List_item *pItem;
  ध्रुव संरचना व्यञ्जकम्_List_item *pOldItem;
  पूर्णाङ्क i;
  व्यञ्जकम् *pPriorSelectColOld = 0;
  व्यञ्जकम् *pPriorSelectColNew = 0;
  प्रतिज्ञा( db!=0 );
  जइ( p==0 ) प्रत्यागम 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  जइ( pNew==0 ) प्रत्यागम 0;
  pNew->n_व्यञ्जकम् = p->n_व्यञ्जकम्;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  प्रति(i=0; i<p->n_व्यञ्जकम्; i++, pItem++, pOldItem++){
    व्यञ्जकम् *p_Old_व्यञ्जकम् = pOldItem->p_व्यञ्जकम्;
    व्यञ्जकम् *p_New_व्यञ्जकम्;
    pItem->p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_Old_व्यञ्जकम्, flags);
    जइ( p_Old_व्यञ्जकम्
     && p_Old_व्यञ्जकम्->op==TK_SELECT_COLUMN
     && (p_New_व्यञ्जकम् = pItem->p_व्यञ्जकम्)!=0
    ){
      जइ( p_New_व्यञ्जकम्->p_दक्षिणः ){
        pPriorSelectColOld = p_Old_व्यञ्जकम्->p_दक्षिणः;
        pPriorSelectColNew = p_New_व्यञ्जकम्->p_दक्षिणः;
        p_New_व्यञ्जकम्->p_वामः = p_New_व्यञ्जकम्->p_दक्षिणः;
      }अण्णहा{
        जइ( p_Old_व्यञ्जकम्->p_वामः!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_व्यञ्जकम्->p_वामः;
          pPriorSelectColNew = sqlite3_व्यञ्जकम्_Dup(db, pPriorSelectColOld, flags);
          p_New_व्यञ्जकम्->p_दक्षिणः = pPriorSelectColNew;
        }
        p_New_व्यञ्जकम्->p_वामः = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  प्रत्यागम pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#जइ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ध्रुव SrcList *p, पूर्णाङ्क flags){
  SrcList *pNew;
  पूर्णाङ्क i;
  प्रतिज्ञा( db!=0 );
  जइ( p==0 ) प्रत्यागम 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  जइ( pNew==0 ) प्रत्यागम 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  प्रति(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    ध्रुव SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    जइ( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, परिमाण(Subquery));
      जइ( pNewSubq==0 ){
        प्रतिज्ञा( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }अण्णहा{
        memcpy(pNewSubq, pOldItem->u4.pSubq, परिमाण(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        जइ( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }अण्णहा जइ( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }अण्णहा{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    जइ( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }अण्णहा जइ( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_व्यञ्जकम्_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }अण्णहा{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    जइ( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    जइ( pTab ){
      pTab->nTabRef++;
    }
    जइ( pOldItem->fg.isUsing ){
      प्रतिज्ञा( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }अण्णहा{
      pNewItem->u3.pOn = sqlite3_व्यञ्जकम्_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  प्रत्यागम pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ध्रुव IdList *p){
  IdList *pNew;
  पूर्णाङ्क i;
  प्रतिज्ञा( db!=0 );
  जइ( p==0 ) प्रत्यागम 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  जइ( pNew==0 ) प्रत्यागम 0;
  pNew->nId = p->nId;
  प्रति(i=0; i<p->nId; i++){
    संरचना IdList_item *pNewItem = &pNew->a[i];
    ध्रुव संरचना IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  प्रत्यागम pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ध्रुव Select *pDup, पूर्णाङ्क flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  ध्रुव Select *p;

  प्रतिज्ञा( db!=0 );
  प्रति(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, परिमाण(*p) );
    जइ( pNew==0 ) भङ्ग;
    pNew->pEList = sqlite3_व्यञ्जकम्_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_व्यञ्जकम्_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_व्यञ्जकम्_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_व्यञ्जकम्_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_व्यञ्जकम्_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_व्यञ्जकम्_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    जइ( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    जइ( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      भङ्ग;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  प्रत्यागम pRet;
}
#अण्णहा
Select *sqlite3SelectDup(sqlite3 *db, ध्रुव Select *p, पूर्णाङ्क flags){
  प्रतिज्ञा( p==0 );
  प्रत्यागम 0;
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
स्थिर ध्रुव संरचना व्यञ्जकम्_List_item zeroItem = {0};
SQLITE_NOINLINE व्यञ्जकम्_List *sqlite3_व्यञ्जकम्_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  व्यञ्जकम् *p_व्यञ्जकम्             /* Expression to be appended. Might be NULL */
){
  संरचना व्यञ्जकम्_List_item *pItem;
  व्यञ्जकम्_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  जइ( pList==0 ){
    sqlite3_व्यञ्जकम्_Delete(db, p_व्यञ्जकम्);
    प्रत्यागम 0;
  }
  pList->nAlloc = 4;
  pList->n_व्यञ्जकम् = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_व्यञ्जकम् = p_व्यञ्जकम्;
  प्रत्यागम pList;
}
SQLITE_NOINLINE व्यञ्जकम्_List *sqlite3_व्यञ्जकम्_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  व्यञ्जकम्_List *pList,        /* List to which to append. Might be NULL */
  व्यञ्जकम् *p_व्यञ्जकम्             /* Expression to be appended. Might be NULL */
){
  संरचना व्यञ्जकम्_List_item *pItem;
  व्यञ्जकम्_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  जइ( pNew==0 ){
    sqlite3_व्यञ्जकम्_List_Delete(db, pList);
    sqlite3_व्यञ्जकम्_Delete(db, p_व्यञ्जकम्);
    प्रत्यागम 0;
  }अण्णहा{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_व्यञ्जकम्++];
  *pItem = zeroItem;
  pItem->p_व्यञ्जकम् = p_व्यञ्जकम्;
  प्रत्यागम pList;
}
व्यञ्जकम्_List *sqlite3_व्यञ्जकम्_List_Append(
  Parse *pParse,          /* Parsing context */
  व्यञ्जकम्_List *pList,        /* List to which to append. Might be NULL */
  व्यञ्जकम् *p_व्यञ्जकम्             /* Expression to be appended. Might be NULL */
){
  संरचना व्यञ्जकम्_List_item *pItem;
  जइ( pList==0 ){
    प्रत्यागम sqlite3_व्यञ्जकम्_List_Append_New(pParse->db,p_व्यञ्जकम्);
  }
  जइ( pList->nAlloc<pList->n_व्यञ्जकम्+1 ){
    प्रत्यागम sqlite3_व्यञ्जकम्_List_Append_Grow(pParse->db,pList,p_व्यञ्जकम्);
  }
  pItem = &pList->a[pList->n_व्यञ्जकम्++];
  *pItem = zeroItem;
  pItem->p_व्यञ्जकम् = p_व्यञ्जकम्;
  प्रत्यागम pList;
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
व्यञ्जकम्_List *sqlite3_व्यञ्जकम्_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  व्यञ्जकम्_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  व्यञ्जकम् *p_व्यञ्जकम्            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  पूर्णाङ्क n;
  पूर्णाङ्क i;
  पूर्णाङ्क iFirst = pList ? pList->n_व्यञ्जकम् : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  जइ( NEVER(pColumns==0) ) गच्छ vector_append_error;
  जइ( p_व्यञ्जकम्==0 ) गच्छ vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  जइ( p_व्यञ्जकम्->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_व्यञ्जकम्_Vector_Size(p_व्यञ्जकम्)) ){
    sqlite3_Error_सन्देशः(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    गच्छ vector_append_error;
  }

  प्रति(i=0; i<pColumns->nId; i++){
    व्यञ्जकम् *p_Sub_व्यञ्जकम् = sqlite3_व्यञ्जकम्_For_Vector_Field(pParse, p_व्यञ्जकम्, i, pColumns->nId);
    प्रतिज्ञा( p_Sub_व्यञ्जकम्!=0 || db->mallocFailed );
    जइ( p_Sub_व्यञ्जकम्==0 ) अनुवर्त;
    pList = sqlite3_व्यञ्जकम्_List_Append(pParse, pList, p_Sub_व्यञ्जकम्);
    जइ( pList ){
      प्रतिज्ञा( pList->n_व्यञ्जकम्==iFirst+i+1 );
      pList->a[pList->n_व्यञ्जकम्-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  जइ( !db->mallocFailed && p_व्यञ्जकम्->op==TK_SELECT && ALWAYS(pList!=0) ){
    व्यञ्जकम् *pFirst = pList->a[iFirst].p_व्यञ्जकम्;
    प्रतिज्ञा( pFirst!=0 );
    प्रतिज्ञा( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_दक्षिणः = p_व्यञ्जकम्;
    p_व्यञ्जकम् = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_व्यञ्जकम्_Unmap_And_Delete(pParse, p_व्यञ्जकम्);
  sqlite3IdListDelete(db, pColumns);
  प्रत्यागम pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
शून्य sqlite3_व्यञ्जकम्_List_Set_Sort_Order(व्यञ्जकम्_List *p, पूर्णाङ्क iSortOrder, पूर्णाङ्क eNulls){
  संरचना व्यञ्जकम्_List_item *pItem;
  जइ( p==0 ) प्रत्यागम;
  प्रतिज्ञा( p->n_व्यञ्जकम्>0 );

  प्रतिज्ञा( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  प्रतिज्ञा( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  प्रतिज्ञा( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_व्यञ्जकम्-1];
  प्रतिज्ञा( pItem->fg.bNulls==0 );
  जइ( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  जइ( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    जइ( iSortOrder!=eNulls ){
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
शून्य sqlite3_व्यञ्जकम्_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  व्यञ्जकम्_List *pList,        /* List to which to add the span. */
  ध्रुव Token *pName,     /* Name to be added */
  पूर्णाङ्क dequote             /* True to cause the name to be dequoted */
){
  प्रतिज्ञा( pList!=0 || pParse->db->mallocFailed!=0 );
  प्रतिज्ञा( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  जइ( pList ){
    संरचना व्यञ्जकम्_List_item *pItem;
    प्रतिज्ञा( pList->n_व्यञ्जकम्>0 );
    pItem = &pList->a[pList->n_व्यञ्जकम्-1];
    प्रतिज्ञा( pItem->zEName==0 );
    प्रतिज्ञा( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    जइ( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      जइ( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ध्रुव शून्य*)pItem->zEName, pName);
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
शून्य sqlite3_व्यञ्जकम्_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  व्यञ्जकम्_List *pList,        /* List to which to add the span. */
  ध्रुव वर्ण *zStart,     /* Start of the span */
  ध्रुव वर्ण *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  प्रतिज्ञा( pList!=0 || db->mallocFailed!=0 );
  जइ( pList ){
    संरचना व्यञ्जकम्_List_item *pItem = &pList->a[pList->n_व्यञ्जकम्-1];
    प्रतिज्ञा( pList->n_व्यञ्जकम्>0 );
    जइ( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
शून्य sqlite3_व्यञ्जकम्_List_Check_Length(
  Parse *pParse,
  व्यञ्जकम्_List *pEList,
  ध्रुव वर्ण *zObject
){
  पूर्णाङ्क mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_व्यञ्जकम्==mx );
  testcase( pEList && pEList->n_व्यञ्जकम्==mx+1 );
  जइ( pEList && pEList->n_व्यञ्जकम्>mx ){
    sqlite3_Error_सन्देशः(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
स्थिर SQLITE_NOINLINE शून्य व्यञ्जकम्_List_Delete_NN(sqlite3 *db, व्यञ्जकम्_List *pList){
  पूर्णाङ्क i = pList->n_व्यञ्जकम्;
  संरचना व्यञ्जकम्_List_item *pItem =  pList->a;
  प्रतिज्ञा( pList->n_व्यञ्जकम्>0 );
  प्रतिज्ञा( db!=0 );
  करेहि{
    sqlite3_व्यञ्जकम्_Delete(db, pItem->p_व्यञ्जकम्);
    जइ( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }जाव( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
शून्य sqlite3_व्यञ्जकम्_List_Delete(sqlite3 *db, व्यञ्जकम्_List *pList){
  जइ( pList ) व्यञ्जकम्_List_Delete_NN(db, pList);
}
शून्य sqlite3_व्यञ्जकम्_List_Delete_Generic(sqlite3 *db, शून्य *pList){
  जइ( ALWAYS(pList) ) व्यञ्जकम्_List_Delete_NN(db, (व्यञ्जकम्_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_व्यञ्जकम्_List_Flags(ध्रुव व्यञ्जकम्_List *pList){
  पूर्णाङ्क i;
  u32 m = 0;
  प्रतिज्ञा( pList!=0 );
  प्रति(i=0; i<pList->n_व्यञ्जकम्; i++){
     व्यञ्जकम् *p_व्यञ्जकम् = pList->a[i].p_व्यञ्जकम्;
     प्रतिज्ञा( p_व्यञ्जकम्!=0 );
     m |= p_व्यञ्जकम्->flags;
  }
  प्रत्यागम m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
पूर्णाङ्क sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  प्रत्यागम WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ध्रुव वर्ण *zIn){
  जइ( sqlite3StrICmp(zIn, "true")==0  ) प्रत्यागम EP_IsTrue;
  जइ( sqlite3StrICmp(zIn, "false")==0 ) प्रत्यागम EP_IsFalse;
  प्रत्यागम 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Id_To_True_False(व्यञ्जकम् *p_व्यञ्जकम्){
  u32 v;
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_ID || p_व्यञ्जकम्->op==TK_STRING );
  जइ( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Quoted|EP_Int_मानम्)
   && (v = sqlite3IsTrueOrFalse(p_व्यञ्जकम्->u.zToken))!=0
  ){
    p_व्यञ्जकम्->op = TK_TRUEFALSE;
    व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, v);
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Truth_मानम्(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely((व्यञ्जकम्*)p_व्यञ्जकम्);
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_TRUEFALSE );
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
  प्रतिज्ञा( sqlite3StrICmp(p_व्यञ्जकम्->u.zToken,"true")==0
       || sqlite3StrICmp(p_व्यञ्जकम्->u.zToken,"false")==0 );
  प्रत्यागम p_व्यञ्जकम्->u.zToken[4]==0;
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
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Simplified_And_Or(व्यञ्जकम् *p_व्यञ्जकम्){
  प्रतिज्ञा( p_व्यञ्जकम्!=0 );
  जइ( p_व्यञ्जकम्->op==TK_AND || p_व्यञ्जकम्->op==TK_OR ){
    व्यञ्जकम् *p_दक्षिणः = sqlite3_व्यञ्जकम्_Simplified_And_Or(p_व्यञ्जकम्->p_दक्षिणः);
    व्यञ्जकम् *p_वामः = sqlite3_व्यञ्जकम्_Simplified_And_Or(p_व्यञ्जकम्->p_वामः);
    जइ( व्यञ्जकम्_Always_True(p_वामः) || व्यञ्जकम्_Always_False(p_दक्षिणः) ){
      p_व्यञ्जकम् = p_व्यञ्जकम्->op==TK_AND ? p_दक्षिणः : p_वामः;
    }अण्णहा जइ( व्यञ्जकम्_Always_True(p_दक्षिणः) || व्यञ्जकम्_Always_False(p_वामः) ){
      p_व्यञ्जकम् = p_व्यञ्जकम्->op==TK_AND ? p_वामः : p_दक्षिणः;
    }
  }
  प्रत्यागम p_व्यञ्जकम्;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Eval_Rhs_First(व्यञ्जकम् *p_व्यञ्जकम्){
  जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्->p_वामः, EP_Subquery)
   && !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्->p_दक्षिणः, EP_Subquery)
  ){
    प्रत्यागम 1;
  }अण्णहा{
    प्रत्यागम 0;
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
स्थिर पूर्णाङ्क व्यञ्जकम्_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,       /* The comparison expression */
  पूर्णाङ्क *pR1,          /* OUT: Register holding the left operand */
  पूर्णाङ्क *pR2,          /* OUT: Register holding the right operand */
  पूर्णाङ्क *pFree1,       /* OUT: Temp register to free if not zero */
  पूर्णाङ्क *pFree2        /* OUT: Another temp register to free if not zero */
){
  पूर्णाङ्क addrIsNull;
  पूर्णाङ्क r1, r2;
  Vdbe *v = pParse->pVdbe;

  प्रतिज्ञा( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  जइ( व्यञ्जकम्_Eval_Rhs_First(p_व्यञ्जकम्) && sqlite3_व्यञ्जकम्_Can_Be_Null(p_व्यञ्जकम्->p_दक्षिणः) ){
    r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_दक्षिणः, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }अण्णहा{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, pFree1);
  जइ( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्->p_दक्षिणः, EP_Subquery)
     && sqlite3_व्यञ्जकम्_Can_Be_Null(p_व्यञ्जकम्->p_वामः)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_दक्षिणः, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  प्रत्यागम addrIsNull;
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
स्थिर SQLITE_NOINLINE पूर्णाङ्क व्यञ्जकम्_ग्रन्थिः_Is_Constant_Function(
  Walker *pWalker,
  व्यञ्जकम् *p_व्यञ्जकम्
){
  पूर्णाङ्क n;             /* Number of arguments */
  व्यञ्जकम्_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_FUNCTION );
  जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_TokenOnly)
   || (pList = p_व्यञ्जकम्->x.pList)==0
  ){;
    n = 0;
  }अण्णहा{
    n = pList->n_व्यञ्जकम्;
    sqlite3_Walk_व्यञ्जकम्_List(pWalker, pList);
    जइ( pWalker->eCode==0 ) प्रत्यागम WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_व्यञ्जकम्->u.zToken, n, ENC(db), 0);
  जइ( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    प्रत्यागम WRC_Abort;
  }
  प्रत्यागम WRC_Prune;
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
स्थिर पूर्णाङ्क व्यञ्जकम्_ग्रन्थिः_Is_Constant(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  प्रतिज्ञा( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  जइ( pWalker->eCode==2 && व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) ){
    pWalker->eCode = 0;
    प्रत्यागम WRC_Abort;
  }

  विकल्प( p_व्यञ्जकम्->op ){
    पक्ष TK_FUNCTION:
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        प्रत्यागम WRC_Abort;
      }अण्णहा जइ( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        जइ( pWalker->eCode==5 ) व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, EP_FromDDL);
        प्रत्यागम WRC_Continue;
      }अण्णहा जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        प्रत्यागम WRC_Continue;
      }अण्णहा जइ( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        प्रत्यागम व्यञ्जकम्_ग्रन्थिः_Is_Constant_Function(pWalker, p_व्यञ्जकम्);
      }अण्णहा{
        pWalker->eCode = 0;  /* Function call is not constant */
        प्रत्यागम WRC_Abort;
      }
    पक्ष TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      जइ( sqlite3_व्यञ्जकम्_Id_To_True_False(p_व्यञ्जकम्) ){
        प्रत्यागम WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    पक्ष TK_COLUMN:
    पक्ष TK_AGG_FUNCTION:
    पक्ष TK_AGG_COLUMN:
      testcase( p_व्यञ्जकम्->op==TK_ID );
      testcase( p_व्यञ्जकम्->op==TK_COLUMN );
      testcase( p_व्यञ्जकम्->op==TK_AGG_FUNCTION );
      testcase( p_व्यञ्जकम्->op==TK_AGG_COLUMN );
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FixedCol) && pWalker->eCode!=2 ){
        प्रत्यागम WRC_Continue;
      }
      जइ( pWalker->eCode==3 && p_व्यञ्जकम्->iTable==pWalker->u.iCur ){
        प्रत्यागम WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    पक्ष TK_IF_NULL_ROW:
    पक्ष TK_REGISTER:
    पक्ष TK_DOT:
    पक्ष TK_RAISE:
      testcase( p_व्यञ्जकम्->op==TK_REGISTER );
      testcase( p_व्यञ्जकम्->op==TK_IF_NULL_ROW );
      testcase( p_व्यञ्जकम्->op==TK_DOT );
      testcase( p_व्यञ्जकम्->op==TK_RAISE );
      pWalker->eCode = 0;
      प्रत्यागम WRC_Abort;
    पक्ष TK_IS:
    पक्ष TK_ISNOT:
      जइ( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        प्रत्यागम WRC_Abort;
      }
      प्रत्यागम WRC_Continue;
    पक्ष TK_VARIABLE:
      जइ( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_व्यञ्जकम्->op = TK_NULL;
      }अण्णहा जइ( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        प्रत्यागम WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    स्वभाव:
      testcase( p_व्यञ्जकम्->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_व्यञ्जकम्->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      प्रत्यागम WRC_Continue;
  }
}
स्थिर पूर्णाङ्क व्यञ्जकम्_Is_Const(Parse *pParse, व्यञ्जकम् *p, पूर्णाङ्क initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_व्यञ्जकम्_Callback = व्यञ्जकम्_ग्रन्थिः_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_व्यञ्जकम्(&w, p);
  प्रत्यागम w.eCode;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Constant(Parse *pParse, व्यञ्जकम् *p){
  प्रत्यागम व्यञ्जकम्_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_List_Is_Constant(Parse *pParse, व्यञ्जकम्_List *pList, पूर्णाङ्क bNoIs){
  ध्रुव पूर्णाङ्क iInit = bNoIs ? 6 : 1;
  पूर्णाङ्क ii;
  प्रति(ii=0; ii<pList->n_व्यञ्जकम्; ii++){
    जइ( 0==व्यञ्जकम्_Is_Const(pParse, pList->a[ii].p_व्यञ्जकम्, iInit) ) प्रत्यागम 0;
  }
  प्रत्यागम 1;
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
स्थिर पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Constant_Not_Join(Parse *pParse, व्यञ्जकम् *p){
  प्रत्यागम व्यञ्जकम्_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  प्रतिज्ञा( pSelect!=0 );
  प्रतिज्ञा( pWalker->eCode==3 || pWalker->eCode==0 );
  जइ( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    प्रत्यागम WRC_Abort;
  }
  प्रत्यागम WRC_Prune;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Table_Constant(व्यञ्जकम् *p, पूर्णाङ्क iCur, पूर्णाङ्क bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_व्यञ्जकम्_Callback = व्यञ्जकम्_ग्रन्थिः_Is_Constant;
  जइ( bAllowSubq ){
    w.xSelectCallback = व्यञ्जकम्_Select_Walk_Table_Constant;
  }अण्णहा{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_व्यञ्जकम्(&w, p);
  प्रत्यागम w.eCode;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Single_Table_Constraint(
  व्यञ्जकम् *p_व्यञ्जकम्,                 /* The constraint */
  ध्रुव SrcList *pSrcList,     /* Complete FROM clause */
  पूर्णाङ्क iSrc,                    /* Which element of pSrcList to use */
  पूर्णाङ्क bAllowSubq               /* Allow non-correlated subqueries */
){
  ध्रुव SrcItem *pSrc = &pSrcList->a[iSrc];
  जइ( pSrc->fg.jointype & JT_LTORJ ){
    प्रत्यागम 0;  /* rule (3) */
  }
  जइ( pSrc->fg.jointype & JT_वामः ){
    जइ( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) ) प्रत्यागम 0;   /* rule (4a) */
    जइ( p_व्यञ्जकम्->w.iJoin!=pSrc->iCursor ) प्रत्यागम 0;         /* rule (4b) */
  }अण्णहा{
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) ) प्रत्यागम 0;    /* rule (5) */
  }
  जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    पूर्णाङ्क jj;
    प्रति(jj=0; jj<iSrc; jj++){
      जइ( p_व्यञ्जकम्->w.iJoin==pSrcList->a[jj].iCursor ){
        जइ( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          प्रत्यागम 0;  /* restriction (6) */
        }
        भङ्ग;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  प्रत्यागम sqlite3_व्यञ्जकम्_Is_Table_Constant(p_व्यञ्जकम्, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_ग्रन्थिः_Is_Constant_Or_Group_By(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  व्यञ्जकम्_List *pGroupBy = pWalker->u.pGroupBy;
  पूर्णाङ्क i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  प्रति(i=0; i<pGroupBy->n_व्यञ्जकम्; i++){
    व्यञ्जकम् *p = pGroupBy->a[i].p_व्यञ्जकम्;
    जइ( sqlite3_व्यञ्जकम्_Compare(0, p_व्यञ्जकम्, p, -1)<2 ){
      CollSeq *pColl = sqlite3_व्यञ्जकम्_NNColl_Seq(pWalker->pParse, p);
      जइ( sqlite3_Is_द्विमान(pColl) ){
        प्रत्यागम WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
    pWalker->eCode = 0;
    प्रत्यागम WRC_Abort;
  }

  प्रत्यागम व्यञ्जकम्_ग्रन्थिः_Is_Constant(pWalker, p_व्यञ्जकम्);
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Constant_Or_Group_By(Parse *pParse, व्यञ्जकम् *p, व्यञ्जकम्_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_व्यञ्जकम्_Callback = व्यञ्जकम्_ग्रन्थिः_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_व्यञ्जकम्(&w, p);
  प्रत्यागम w.eCode;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Constant_Or_Function(व्यञ्जकम् *p, u8 isInit){
  प्रतिज्ञा( isInit==0 || isInit==1 );
  प्रत्यागम व्यञ्जकम्_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Contains_Subquery(व्यञ्जकम् *p){
  Walker w;
  w.eCode = 1;
  w.x_व्यञ्जकम्_Callback = sqlite3_व्यञ्जकम्_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_व्यञ्जकम्(&w, p);
  प्रत्यागम w.eCode==0;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Integer(ध्रुव व्यञ्जकम् *p, पूर्णाङ्क *p_मानम्, Parse *pParse, पूर्णाङ्क bRSI){
  पूर्णाङ्क iSign = 1;   /* Either +1 or -1. */
  जाव( 1/*exit-by-break*/ ){
    जइ( NEVER(p==0) ) प्रत्यागम 0;
    जइ( व्यञ्जकम्_Use_UValue(p) ){
      प्रतिज्ञा( p->op==TK_INTEGER );
      *p_मानम् = p->u.i_मानम्*iSign;
      जइ( bRSI && pParse==0 && p->op2 ){
        प्रत्यागम 0;
      }
      प्रत्यागम 1;
    }
    जइ( p->op==TK_UPLUS ){
      p = p->p_वामः;
      pParse = 0;
      अनुवर्त;
    }
    जइ( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_वामः;
      pParse = 0;
      अनुवर्त;
    }
    जइ( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_मानम् *pVal; /* The variable */
      पूर्णाङ्क rc = 0;          /* 1 if successful, 0 if failed */
      प्रतिज्ञा( iSign==1 );
      जइ( NEVER(pParse->pVdbe==0) ) भङ्ग;
      जइ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) भङ्ग;
      pVal = sqlite3_Vdbe_Get_Bound_मानम्(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      जइ( pVal ){
        i64 vv;
        जइ( sqlite3_मानम्_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_मानम्_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_मानम् = (पूर्णाङ्क)vv;
          rc = 1;       
        }
        sqlite3_मानम्_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      प्रत्यागम rc;
    }
    भङ्ग;
  }
  प्रत्यागम 0;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Can_Be_Null(ध्रुव व्यञ्जकम् *p){
  u8 op;
  प्रतिज्ञा( p!=0 );
  जाव( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_वामः;
    प्रतिज्ञा( p!=0 );
  }
  op = p->op;
  जइ( op==TK_REGISTER ) op = p->op2;
  विकल्प( op ){
    पक्ष TK_INTEGER:
    पक्ष TK_STRING:
    पक्ष TK_FLOAT:
    पक्ष TK_BLOB:
      प्रत्यागम 0;
    पक्ष TK_COLUMN:
      प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p) );
      प्रत्यागम व्यञ्जकम्_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    स्वभाव:
      प्रत्यागम 1;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Needs_No_Affinity_Change(ध्रुव व्यञ्जकम् *p, वर्ण aff){
  u8 op;
  पूर्णाङ्क unaryMinus = 0;
  जइ( aff==SQLITE_AFF_BLOB ) प्रत्यागम 1;
  जाव( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    जइ( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_वामः;
  }
  op = p->op;
  जइ( op==TK_REGISTER ) op = p->op2;
  विकल्प( op ){
    पक्ष TK_INTEGER: {
      प्रत्यागम aff>=SQLITE_AFF_NUMERIC;
    }
    पक्ष TK_FLOAT: {
      प्रत्यागम aff>=SQLITE_AFF_NUMERIC;
    }
    पक्ष TK_STRING: {
      प्रत्यागम !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    पक्ष TK_BLOB: {
      प्रत्यागम !unaryMinus;
    }
    पक्ष TK_COLUMN: {
      प्रतिज्ञा( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      प्रत्यागम aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    स्वभाव: {
      प्रत्यागम 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
पूर्णाङ्क sqlite3IsRowid(ध्रुव वर्ण *z){
  जइ( sqlite3StrICmp(z, "_ROWID_")==0 ) प्रत्यागम 1;
  जइ( sqlite3StrICmp(z, "ROWID")==0 ) प्रत्यागम 1;
  जइ( sqlite3StrICmp(z, "OID")==0 ) प्रत्यागम 1;
  प्रत्यागम 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ध्रुव वर्ण *sqlite3RowidAlias(Table *pTab){
  ध्रुव वर्ण *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  पूर्णाङ्क ii;
  प्रतिज्ञा( VisibleRowid(pTab) );
  प्रति(ii=0; ii<ArraySize(azOpt); ii++){
    जइ( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) प्रत्यागम azOpt[ii];
  }
  प्रत्यागम 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
स्थिर Select *isCandidateForInOpt(ध्रुव व्यञ्जकम् *pX){
  Select *p;
  SrcList *pSrc;
  व्यञ्जकम्_List *pEList;
  Table *pTab;
  पूर्णाङ्क i;
  जइ( !व्यञ्जकम्_Use_XSelect(pX) ) प्रत्यागम 0;                 /* Not a subquery */
  जइ( व्यञ्जकम्_Has_Property(pX, EP_VarSelect)  ) प्रत्यागम 0;  /* Correlated subq */
  p = pX->x.pSelect;
  जइ( p->pPrior ) प्रत्यागम 0;              /* Not a compound SELECT */
  जइ( p->selFlags & SF_Aggregate ){
    प्रत्यागम 0; /* No GROUP BY keyword or aggregate functions */
  }
  प्रतिज्ञा( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  जइ( p->pLimit ) प्रत्यागम 0;              /* Has no LIMIT clause */
  जइ( p->pWhere ) प्रत्यागम 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  प्रतिज्ञा( pSrc!=0 );
  जइ( pSrc->nSrc!=1 ) प्रत्यागम 0;          /* Single term in FROM clause */
  जइ( pSrc->a[0].fg.isSubquery) प्रत्यागम 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( !IsView(pTab)  );              /* FROM clause is not a view */
  जइ( IsVirtual(pTab) ) प्रत्यागम 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  प्रतिज्ञा( pEList!=0 );
  /* All SELECT results must be columns. */
  प्रति(i=0; i<pEList->n_व्यञ्जकम्; i++){
    व्यञ्जकम् *pRes = pEList->a[i].p_व्यञ्जकम्;
    जइ( pRes->op!=TK_COLUMN ) प्रत्यागम 0;
    प्रतिज्ञा( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  प्रत्यागम p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
स्थिर शून्य sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  पूर्णाङ्क iCur,          /* Cursor for the index */
  पूर्णाङ्क regHasNull,    /* Register in which to store hasNull flag */
  पूर्णाङ्क eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  पूर्णाङ्क addr1;
  पूर्णाङ्क op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  जइ( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }अण्णहा{
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
स्थिर पूर्णाङ्क sqlite3InRhsIsConstant(Parse *pParse, व्यञ्जकम् *pIn){
  व्यञ्जकम् *pLHS;
  पूर्णाङ्क res;
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_वामः;
  pIn->p_वामः = 0;
  res = sqlite3_व्यञ्जकम्_Is_Constant(pParse, pIn);
  pIn->p_वामः = pLHS;
  प्रत्यागम res;
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
पूर्णाङ्क sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  व्यञ्जकम् *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  पूर्णाङ्क *prRhsHasNull,         /* Register holding NULL status.  See notes */
  पूर्णाङ्क *aiMap,                /* Mapping from Index fields to RHS fields */
  पूर्णाङ्क *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  पूर्णाङ्क eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  पूर्णाङ्क iTab;                             /* Cursor of the RHS table */
  पूर्णाङ्क mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  प्रतिज्ञा( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  जइ( prRhsHasNull && व्यञ्जकम्_Use_XSelect(pX) ){
    पूर्णाङ्क i;
    व्यञ्जकम्_List *pEList = pX->x.pSelect->pEList;
    प्रति(i=0; i<pEList->n_व्यञ्जकम्; i++){
      जइ( sqlite3_व्यञ्जकम्_Can_Be_Null(pEList->a[i].p_व्यञ्जकम्) ) भङ्ग;
    }
    जइ( i==pEList->n_व्यञ्जकम् ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  जइ( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    पूर्णाङ्क iDb;                               /* Database idx for pTab */
    व्यञ्जकम्_List *pEList = p->pEList;
    पूर्णाङ्क n_व्यञ्जकम् = pEList->n_व्यञ्जकम्;

    प्रतिज्ञा( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    प्रतिज्ञा( p->pEList->a[0].p_व्यञ्जकम्!=0 ); /* Because of isCandidateForInOpt(p) */
    प्रतिज्ञा( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    प्रतिज्ञा( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    प्रतिज्ञा(v);  /* sqlite3GetVdbe() has always been previously called */
    जइ( n_व्यञ्जकम्==1 && pEList->a[0].p_व्यञ्जकम्->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      पूर्णाङ्क iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }अण्णहा{
      Index *pIdx;                         /* Iterator variable */
      पूर्णाङ्क affinity_ok = 1;
      पूर्णाङ्क i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      प्रति(i=0; i<n_व्यञ्जकम् && affinity_ok; i++){
        व्यञ्जकम् *pLhs = sqlite3VectorFieldSubexpr(pX->p_वामः, i);
        पूर्णाङ्क iCol = pEList->a[i].p_व्यञ्जकम्->iColumn;
        वर्ण idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        वर्ण cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        विकल्प( cmpaff ){
          पक्ष SQLITE_AFF_BLOB:
            भङ्ग;
          पक्ष SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            प्रतिज्ञा( idxaff==SQLITE_AFF_TEXT );
            भङ्ग;
          स्वभाव:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      जइ( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        प्रति(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          जइ( pIdx->nColumn<n_व्यञ्जकम् ) अनुवर्त;
          जइ( pIdx->pPartIdxWhere!=0 ) अनुवर्त;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          जइ( pIdx->nColumn>=BMS-1 ) अनुवर्त;
          जइ( mustBeUnique ){
            जइ( pIdx->n_कुञ्चिका_Col>n_व्यञ्जकम्
             ||(pIdx->nColumn>n_व्यञ्जकम् && !IsUniqueIndex(pIdx))
            ){
              अनुवर्त;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          प्रति(i=0; i<n_व्यञ्जकम्; i++){
            व्यञ्जकम् *pLhs = sqlite3VectorFieldSubexpr(pX->p_वामः, i);
            व्यञ्जकम् *pRhs = pEList->a[i].p_व्यञ्जकम्;
            CollSeq *pReq = sqlite3_द्विमान_Compare_Coll_Seq(pParse, pLhs, pRhs);
            पूर्णाङ्क j;
 
            प्रति(j=0; j<n_व्यञ्जकम्; j++){
              जइ( pIdx->aiColumn[j]!=pRhs->iColumn ) अनुवर्त;
              प्रतिज्ञा( pIdx->azColl[j] );
              जइ( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                अनुवर्त;
              }
              भङ्ग;
            }
            जइ( j==n_व्यञ्जकम् ) भङ्ग;
            mCol = MASKBIT(j);
            जइ( mCol & colUsed ) भङ्ग; /* Each column used only once */
            colUsed |= mCol;
            जइ( aiMap ) aiMap[i] = j;
          }
 
          प्रतिज्ञा( n_व्यञ्जकम्>0 && n_व्यञ्जकम्<BMS );
          प्रतिज्ञा( i==n_व्यञ्जकम् || colUsed!=(MASKBIT(n_व्यञ्जकम्)-1) );
          जइ( colUsed==(MASKBIT(n_व्यञ्जकम्)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            पूर्णाङ्क iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_कुञ्चिका_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            प्रतिज्ञा( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            जइ( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_व्यञ्जकम्)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              जइ( n_व्यञ्जकम्==1 ){
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
  जइ( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && व्यञ्जकम्_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_व्यञ्जकम्<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  जइ( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    पूर्णाङ्क rMayHaveNull = 0;
    पूर्णाङ्क bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    जइ( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }अण्णहा जइ( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    प्रतिज्ञा( pX->op==TK_IN );
    जइ( !bloomOk
     && व्यञ्जकम्_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    जइ( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  जइ( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    पूर्णाङ्क i, n;
    n = sqlite3_व्यञ्जकम्_Vector_Size(pX->p_वामः);
    प्रति(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  प्रत्यागम eType;
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
स्थिर वर्ण *व्यञ्जकम्_INAffinity(Parse *pParse, ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  व्यञ्जकम् *p_वामः = p_व्यञ्जकम्->p_वामः;
  पूर्णाङ्क nVal = sqlite3_व्यञ्जकम्_Vector_Size(p_वामः);
  Select *pSelect = व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ? p_व्यञ्जकम्->x.pSelect : 0;
  वर्ण *zRet;

  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  जइ( zRet ){
    पूर्णाङ्क i;
    प्रति(i=0; i<nVal; i++){
      व्यञ्जकम् *pA = sqlite3VectorFieldSubexpr(p_वामः, i);
      वर्ण a = sqlite3_व्यञ्जकम्_Affinity(pA);
      जइ( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_व्यञ्जकम्, a);
      }अण्णहा{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  प्रत्यागम zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
शून्य sqlite3SubselectError(Parse *pParse, पूर्णाङ्क nActual, पूर्णाङ्क nExpect){
  जइ( pParse->nErr==0 ){
    ध्रुव वर्ण *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_सन्देशः(pParse, zFmt, nActual, nExpect);
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
शून्य sqlite3_Vector_Error_सन्देशः(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्){
#ifndef SQLITE_OMIT_SUBQUERY
  जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
    sqlite3SubselectError(pParse, p_व्यञ्जकम्->x.pSelect->pEList->n_व्यञ्जकम्, 1);
  }अण्णहा
#endif
  {
    sqlite3_Error_सन्देशः(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
स्थिर पूर्णाङ्क findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  जइ( pNewSig==0 ) प्रत्यागम 0;
  जइ( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) प्रत्यागम 0;
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_IN );
  प्रतिज्ञा( !व्यञ्जकम्_Use_YSub(p_व्यञ्जकम्) );
  प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) );
  प्रतिज्ञा( p_व्यञ्जकम्->x.pSelect!=0 );
  प्रतिज्ञा( (p_व्यञ्जकम्->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  प्रतिज्ञा( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  प्रति(; pOp<pEnd; pOp++){
    जइ( pOp->p4type!=P4_SUBRTNSIG ) अनुवर्त;
    प्रतिज्ञा( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    प्रतिज्ञा( pSig!=0 );
    जइ( !pSig->bComplete ) अनुवर्त;
    जइ( pNewSig->selId!=pSig->selId ) अनुवर्त;
    जइ( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) अनुवर्त;
    p_व्यञ्जकम्->y.sub.iAddr = pSig->iAddr;
    p_व्यञ्जकम्->y.sub.regReturn = pSig->regReturn;
    p_व्यञ्जकम्->iTable = pSig->iTable;
    व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, EP_Subrtn);
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
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
शून्य sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,            /* The IN operator */
  पूर्णाङ्क iTab,               /* Use this cursor number */
  पूर्णाङ्क allowBloom          /* True to allow the use of a Bloom filter */
){
  पूर्णाङ्क addrOnce = 0;           /* Address of the OP_Once instruction at top */
  पूर्णाङ्क addr;                   /* Address of OP_OpenEphemeral instruction */
  व्यञ्जकम् *p_वामः;                /* the LHS of the IN operator */
  कुञ्चिका_Info *p_कुञ्चिका_Info = 0;      /* Key information */
  पूर्णाङ्क nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  प्रतिज्ञा( v!=0 );

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
  जइ( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    प्रतिज्ञा( !व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) || p_व्यञ्जकम्->x.pSelect!=0 );
    जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) && (p_व्यञ्जकम्->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, परिमाण(pSig[0]));
      जइ( pSig ){
        pSig->selId = p_व्यञ्जकम्->x.pSelect->selId;
        pSig->zAff = व्यञ्जकम्_INAffinity(pParse, p_व्यञ्जकम्);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_व्यञ्जकम्, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_व्यञ्जकम्->x.pSelect->selId));
      }
      प्रतिज्ञा( व्यञ्जकम्_Use_YSub(p_व्यञ्जकम्) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_व्यञ्जकम्->y.sub.regReturn,
                        p_व्यञ्जकम्->y.sub.iAddr);
      प्रतिज्ञा( iTab!=p_व्यञ्जकम्->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_व्यञ्जकम्->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      जइ( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      प्रत्यागम;
    }

    /* Begin coding the subroutine */
    प्रतिज्ञा( !व्यञ्जकम्_Use_YWin(p_व्यञ्जकम्) );
    व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, EP_Subrtn);
    प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_TokenOnly|EP_Reduced) );
    p_व्यञ्जकम्->y.sub.regReturn = ++pParse->nMem;
    p_व्यञ्जकम्->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_व्यञ्जकम्->y.sub.regReturn) + 1;
    जइ( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_व्यञ्जकम्->y.sub.iAddr;
      pSig->regReturn = p_व्यञ्जकम्->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ध्रुव वर्ण*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_वामः = p_व्यञ्जकम्->p_वामः;
  nVal = sqlite3_व्यञ्जकम्_Vector_Size(p_वामः);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_व्यञ्जकम्->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_व्यञ्जकम्->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
    VdbeComment((v, "Result of SELECT %u", p_व्यञ्जकम्->x.pSelect->selId));
  }अण्णहा{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_Alloc(pParse->db, nVal, 1);
  प्रतिज्ञा( p_कुञ्चिका_Info!=0 || pParse->nErr );
  जइ( p_कुञ्चिका_Info==0 ) प्रत्यागम;

  जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_व्यञ्जकम्->x.pSelect;
    व्यञ्जकम्_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    जइ( ALWAYS(pEList->n_व्यञ्जकम्==nVal) ){
      Select *pCopy;
      SelectDest dest;
      पूर्णाङ्क i;
      पूर्णाङ्क rc;
      पूर्णाङ्क addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = व्यञ्जकम्_INAffinity(pParse, p_व्यञ्जकम्);
      pSelect->iLimit = 0;
      प्रतिज्ञा( pEList!=0 );
      प्रतिज्ञा( pEList->n_व्यञ्जकम्>0 );
      प्रतिज्ञा( sqlite3_कुञ्चिका_Info_Is_Writeable(p_कुञ्चिका_Info) );
      प्रति(i=0; i<nVal; i++){
        व्यञ्जकम् *p = sqlite3VectorFieldSubexpr(p_वामः, i);
        CollSeq *pColl;
        p_कुञ्चिका_Info->aColl[i] = pColl = sqlite3_द्विमान_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_व्यञ्जकम्
        );
        जइ( !sqlite3_Is_द्विमान(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      जइ( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        पूर्णाङ्क regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (शून्य *)p_कुञ्चिका_Info, P4_KEYINFO);
        p_कुञ्चिका_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      जइ( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        जइ( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      जइ( rc ){
        sqlite3_कुञ्चिका_Info_Unref(p_कुञ्चिका_Info);
        प्रत्यागम;
      }
    }
  }अण्णहा जइ( ALWAYS(p_व्यञ्जकम्->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    वर्ण affinity;            /* Affinity of the LHS of the IN */
    पूर्णाङ्क i;
    व्यञ्जकम्_List *pList = p_व्यञ्जकम्->x.pList;
    संरचना व्यञ्जकम्_List_item *pItem;
    पूर्णाङ्क r1, r2;
    affinity = sqlite3_व्यञ्जकम्_Affinity(p_वामः);
    जइ( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }अण्णहा जइ( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    प्रतिज्ञा( p_कुञ्चिका_Info!=0 );
    प्रतिज्ञा( sqlite3_कुञ्चिका_Info_Is_Writeable(p_कुञ्चिका_Info) );
    p_कुञ्चिका_Info->aColl[0] = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p_व्यञ्जकम्->p_वामः);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    प्रति(i=pList->n_व्यञ्जकम्, pItem=pList->a; i>0; i--, pItem++){
      व्यञ्जकम् *pE2 = pItem->p_व्यञ्जकम्;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      जइ( addrOnce && !sqlite3_व्यञ्जकम्_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        व्यञ्जकम्_Clear_Property(p_व्यञ्जकम्, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_व्यञ्जकम्_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_निवेशय, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  जइ( pSig ) pSig->bComplete = 1;
  जइ( p_कुञ्चिका_Info ){
    sqlite3VdbeChangeP4(v, addr, (शून्य *)p_कुञ्चिका_Info, P4_KEYINFO);
  }
  जइ( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    प्रतिज्ञा( व्यञ्जकम्_Use_YSub(p_व्यञ्जकम्) );
    प्रतिज्ञा( sqlite3VdbeGetOp(v,p_व्यञ्जकम्->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_व्यञ्जकम्->y.sub.regReturn,
                      p_व्यञ्जकम्->y.sub.iAddr, 1);
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
पूर्णाङ्क sqlite3CodeSubselect(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  पूर्णाङ्क rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  पूर्णाङ्क nReg;                   /* Registers to allocate */
  व्यञ्जकम् *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णाङ्क addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  प्रतिज्ञा( v!=0 );
  जइ( pParse->nErr ) प्रत्यागम 0;
  testcase( p_व्यञ्जकम्->op==TK_EXISTS );
  testcase( p_व्यञ्जकम्->op==TK_SELECT );
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_EXISTS || p_व्यञ्जकम्->op==TK_SELECT );
  प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) );
  pSel = p_व्यञ्जकम्->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    प्रतिज्ञा( व्यञ्जकम्_Use_YSub(p_व्यञ्जकम्) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_व्यञ्जकम्->y.sub.regReturn,
                      p_व्यञ्जकम्->y.sub.iAddr);
    प्रत्यागम p_व्यञ्जकम्->iTable;
  }

  /* Begin coding the subroutine */
  प्रतिज्ञा( !व्यञ्जकम्_Use_YWin(p_व्यञ्जकम्) );
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Reduced|EP_TokenOnly) );
  व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, EP_Subrtn);
  p_व्यञ्जकम्->y.sub.regReturn = ++pParse->nMem;
  p_व्यञ्जकम्->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_व्यञ्जकम्->y.sub.regReturn) + 1;

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
  जइ( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_VarSelect) ){
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
  nReg = p_व्यञ्जकम्->op==TK_SELECT ? pSel->pEList->n_व्यञ्जकम् : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  जइ( p_व्यञ्जकम्->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    जइ( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_दक्षिणः ){
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
    }अण्णहा{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }अण्णहा{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  जइ( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    व्यञ्जकम् *p_वामः = pSel->pLimit->p_वामः;
    जइ( व्यञ्जकम्_Has_Property(p_वामः, EP_Int_मानम्)==0
     || (p_वामः->u.i_मानम्!=1 && p_वामः->u.i_मानम्!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_व्यञ्जकम्_Int32(db, 0);
      जइ( pLimit ){
        pLimit->aff_व्यञ्जकम् = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_व्यञ्जकम्_Dup(db, p_वामः, 0), pLimit);
      }
      sqlite3_व्यञ्जकम्_Deferred_Delete(pParse, p_वामः);
      pSel->pLimit->p_वामः = pLimit;
    }
  }अण्णहा{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_व्यञ्जकम्_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  जइ( sqlite3Select(pParse, pSel, &dest) ){
    p_व्यञ्जकम्->op2 = p_व्यञ्जकम्->op;
    p_व्यञ्जकम्->op = TK_ERROR;
    प्रत्यागम 0;
  }
  p_व्यञ्जकम्->iTable = rReg = dest.iSDParm;
  व्यञ्जकम्_Set_VVAProperty(p_व्यञ्जकम्, EP_NoReduce);
  जइ( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  जइ( dest.eDest==SRT_Mem 
   && sqlite3_व्यञ्जकम्_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_व्यञ्जकम्) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  प्रतिज्ञा( व्यञ्जकम्_Use_YSub(p_व्यञ्जकम्) );
  प्रतिज्ञा( sqlite3VdbeGetOp(v,p_व्यञ्जकम्->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_व्यञ्जकम्->y.sub.regReturn,
                    p_व्यञ्जकम्->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  प्रत्यागम rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Check_IN(Parse *pParse, व्यञ्जकम् *pIn){
  पूर्णाङ्क nVector = sqlite3_व्यञ्जकम्_Vector_Size(pIn->p_वामः);
  जइ( व्यञ्जकम्_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    जइ( nVector!=pIn->x.pSelect->pEList->n_व्यञ्जकम् ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_व्यञ्जकम्, nVector);
      प्रत्यागम 1;
    }
  }अण्णहा जइ( nVector!=1 ){
    sqlite3_Vector_Error_सन्देशः(pParse, pIn->p_वामः);
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
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
स्थिर शून्य sqlite3_व्यञ्जकम्_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  व्यञ्जकम् *p_व्यञ्जकम्,          /* The IN expression */
  पूर्णाङ्क destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  पूर्णाङ्क destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  पूर्णाङ्क rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  पूर्णाङ्क eType;            /* Type of the RHS */
  पूर्णाङ्क rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  पूर्णाङ्क *aiMap = 0;       /* Map from vector field to index column */
  वर्ण *zAff = 0;       /* Affinity string for comparisons */
  पूर्णाङ्क nVector;          /* Size of vectors for this IN operator */
  पूर्णाङ्क iDummy;           /* Dummy parameter to exprCodeVector() */
  व्यञ्जकम् *p_वामः;          /* The LHS of the IN operator */
  पूर्णाङ्क i;                /* loop counter */
  पूर्णाङ्क destStep2;        /* Where to jump when NULLs seen in step 2 */
  पूर्णाङ्क destStep6 = 0;    /* Start of code for Step 6 */
  पूर्णाङ्क addrTruthOp;      /* Address of opcode that determines the IN is true */
  पूर्णाङ्क destNotNull;      /* Jump here if a comparison is not true in step 6 */
  पूर्णाङ्क addr_शिखरम्;          /* Top of the step-6 loop */
  पूर्णाङ्क iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  प्रतिज्ञा( !व्यञ्जकम्_Has_VVAProperty(p_व्यञ्जकम्,EP_Immutable) );
  p_वामः = p_व्यञ्जकम्->p_वामः;
  जइ( sqlite3_व्यञ्जकम्_Check_IN(pParse, p_व्यञ्जकम्) ) प्रत्यागम;
  zAff = व्यञ्जकम्_INAffinity(pParse, p_व्यञ्जकम्);
  nVector = sqlite3_व्यञ्जकम्_Vector_Size(p_व्यञ्जकम्->p_वामः);
  aiMap = (पूर्णाङ्क*)sqlite3DbMallocZero(pParse->db, nVector*परिमाण(पूर्णाङ्क));
  जइ( pParse->db->mallocFailed ) गच्छ sqlite3_व्यञ्जकम्_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  प्रतिज्ञा( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_व्यञ्जकम्,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  प्रतिज्ञा( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  प्रति(i=0; i<nVector; i++){
    पूर्णाङ्क j, cnt;
    प्रति(cnt=j=0; j<nVector; j++) जइ( aiMap[j]==i ) cnt++;
    प्रतिज्ञा( cnt==1 );
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
  प्रतिज्ञा( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = व्यञ्जकम्_Code_Vector(pParse, p_वामः, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  जइ( eType==IN_INDEX_NOOP ){
    व्यञ्जकम्_List *pList;
    CollSeq *pColl;
    पूर्णाङ्क labelOk = sqlite3VdbeMakeLabel(pParse);
    पूर्णाङ्क r2, regToFree;
    पूर्णाङ्क regCkNull = 0;
    पूर्णाङ्क ii;
    प्रतिज्ञा( nVector==1 );
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
    pList = p_व्यञ्जकम्->x.pList;
    pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p_व्यञ्जकम्->p_वामः);
    जइ( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    प्रति(ii=0; ii<pList->n_व्यञ्जकम्; ii++){
      r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, pList->a[ii].p_व्यञ्जकम्, &regToFree);
      जइ( regCkNull && sqlite3_व्यञ्जकम्_Can_Be_Null(pList->a[ii].p_व्यञ्जकम्) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      जइ( ii<pList->n_व्यञ्जकम्-1 || destIfNull!=destIfFalse ){
        पूर्णाङ्क op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (शून्य*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_व्यञ्जकम्-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_व्यञ्जकम्-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_व्यञ्जकम्-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_व्यञ्जकम्-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }अण्णहा{
        पूर्णाङ्क op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        प्रतिज्ञा( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (शून्य*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    जइ( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    गच्छ sqlite3_व्यञ्जकम्_Code_IN_finished;
  }

  जइ( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    जइ( nVector==1 ){
      वर्ण aff = zAff[0];
      जइ( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_व्यञ्जकम्_Affinity(p_वामः) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        पूर्णाङ्क rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }अण्णहा{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    प्रति(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    जइ( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      पूर्णाङ्क rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      प्रति(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  जइ( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }अण्णहा{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  प्रति(i=0; i<nVector; i++){
    व्यञ्जकम् *p = sqlite3VectorFieldSubexpr(p_व्यञ्जकम्->p_वामः, i);
    जइ( pParse->nErr ) गच्छ sqlite3_व्यञ्जकम्_Code_IN_oom_error;
    जइ( sqlite3_व्यञ्जकम्_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  जइ( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    प्रतिज्ञा( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }अण्णहा{
    जइ( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subrtn) ){
        ध्रुव VdbeOp *pOp = sqlite3VdbeGetOp(v, p_व्यञ्जकम्->y.sub.iAddr);
        प्रतिज्ञा( pOp->opcode==OP_Once || pParse->nErr );
        जइ( pOp->p3>0 ){  /* tag-202407032019 */
          प्रतिज्ञा( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      गच्छ sqlite3_व्यञ्जकम्_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  जइ( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  जइ( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  जइ( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  जइ( eType==IN_INDEX_INDEX_DESC ){
    addr_शिखरम् = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }अण्णहा{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_शिखरम् = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  जइ( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }अण्णहा{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  प्रति(i=0; i<nVector; i++){
    व्यञ्जकम् *p;
    CollSeq *pColl;
    पूर्णाङ्क r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_वामः, i);
    जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
      व्यञ्जकम् *pRhs = p_व्यञ्जकम्->x.pSelect->pEList->a[i].p_व्यञ्जकम्;
      pColl = sqlite3_द्विमान_Compare_Coll_Seq(pParse, p, pRhs);
    }अण्णहा{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (शून्य*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  जइ( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_शिखरम्+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_व्यञ्जकम्_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_व्यञ्जकम्_Code_IN_oom_error:
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
स्थिर शून्य codeReal(Vdbe *v, ध्रुव वर्ण *z, पूर्णाङ्क negateFlag, पूर्णाङ्क iMem){
  जइ( ALWAYS(z!=0) ){
    द्विगुण मानम्;
    sqlite3AtoF(z, &मानम्);
    प्रतिज्ञा( !sqlite3IsNaN(मानम्) ); /* The new AtoF never returns NaN */
    जइ( negateFlag ) मानम् = -मानम्;
    sqlite3VdbeAddDouble(v, iMem, मानम्);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
स्थिर शून्य codeInteger(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क negFlag, पूर्णाङ्क iMem){
  Vdbe *v = pParse->pVdbe;
  जइ( p_व्यञ्जकम्->flags & EP_Int_मानम् ){
    पूर्णाङ्क i = p_व्यञ्जकम्->u.i_मानम्;
    प्रतिज्ञा( i>=0 );
    जइ( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }अण्णहा{
    पूर्णाङ्क c;
    i64 मानम्;
    ध्रुव वर्ण *z = p_व्यञ्जकम्->u.zToken;
    प्रतिज्ञा( z!=0 );
    c = sqlite3DecOrHexToI64(z, &मानम्);
    जइ( (c==3 && !negFlag) || (c==2) || (negFlag && मानम्==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_सन्देशः(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_व्यञ्जकम्);
#अण्णहा
#ifndef SQLITE_OMIT_HEX_INTEGER
      जइ( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_सन्देशः(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_व्यञ्जकम्);
      }अण्णहा
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }अण्णहा{
      जइ( negFlag ){ मानम् = c==3 ? SMALLEST_INT64 : -मानम्; }
      sqlite3VdbeAddInt64(v, iMem, मानम्);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
शून्य sqlite3_व्यञ्जकम्_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  पूर्णाङ्क iTabCur,    /* Cursor pointing to a table row */
  पूर्णाङ्क iIdxCol,    /* The column of the index to be loaded */
  पूर्णाङ्क regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  जइ( iTabCol==XN_व्यञ्जकम् ){
    प्रतिज्ञा( pIdx->a_Col_व्यञ्जकम् );
    प्रतिज्ञा( pIdx->a_Col_व्यञ्जकम्->n_व्यञ्जकम्>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_व्यञ्जकम्_Code_Copy(pParse, pIdx->a_Col_व्यञ्जकम्->a[iIdxCol].p_व्यञ्जकम्, regOut);
    pParse->iSelfTab = 0;
  }अण्णहा{
    sqlite3_व्यञ्जकम्_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
शून्य sqlite3_व्यञ्जकम्_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  पूर्णाङ्क regOut         /* Put the result in this register */
){
  पूर्णाङ्क iAddr;
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क nErr = pParse->nErr;
  प्रतिज्ञा( v!=0 );
  प्रतिज्ञा( pParse->iSelfTab!=0 );
  जइ( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }अण्णहा{
    iAddr = 0;
  }
  sqlite3_व्यञ्जकम्_Code_Copy(pParse, sqlite3_Column_व्यञ्जकम्(pTab,pCol), regOut);
  जइ( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    पूर्णाङ्क p3 = 2+(पूर्णाङ्क)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (वर्ण*)pTab, P4_TABLE);
  }अण्णहा जइ( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  जइ( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  जइ( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
शून्य sqlite3_व्यञ्जकम्_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  पूर्णाङ्क iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  पूर्णाङ्क iCol,       /* Index of the column to extract */
  पूर्णाङ्क regOut      /* Extract the value into this register */
){
  Column *pCol;
  प्रतिज्ञा( v!=0 );
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( iCol!=XN_व्यञ्जकम् );
  जइ( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }अण्णहा{
    पूर्णाङ्क x;
    जइ( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      प्रत्यागम;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }अण्णहा जइ( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      जइ( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_सन्देशः(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }अण्णहा{
        पूर्णाङ्क savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_व्यञ्जकम्_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      प्रत्यागम;
#endif
    }अण्णहा जइ( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_कुञ्चिका_Index(pTab), iCol);
    }अण्णहा{
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  पूर्णाङ्क iColumn,     /* Index of the table column */
  पूर्णाङ्क iTable,      /* The cursor pointing to the table */
  पूर्णाङ्क iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  प्रतिज्ञा( pParse->pVdbe!=0 );
  प्रतिज्ञा( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  प्रतिज्ञा( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_व्यञ्जकम्_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  जइ( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    जइ( pOp->opcode==OP_Column ) pOp->p5 = p5;
    जइ( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  प्रत्यागम iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
शून्य sqlite3_व्यञ्जकम्_Code_Move(Parse *pParse, पूर्णाङ्क iFrom, पूर्णाङ्क iTo, पूर्णाङ्क nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
शून्य sqlite3_व्यञ्जकम्_To_Register(व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क iReg){
  व्यञ्जकम् *p = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p_व्यञ्जकम्);
  जइ( NEVER(p==0) ) प्रत्यागम;
  जइ( p->op==TK_REGISTER ){
    प्रतिज्ञा( p->iTable==iReg );
  }अण्णहा{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    व्यञ्जकम्_Clear_Property(p, EP_Skip);
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
स्थिर पूर्णाङ्क व्यञ्जकम्_Code_Vector(Parse *pParse, व्यञ्जकम् *p, पूर्णाङ्क *piFreeable){
  पूर्णाङ्क iResult;
  पूर्णाङ्क nResult = sqlite3_व्यञ्जकम्_Vector_Size(p);
  जइ( nResult==1 ){
    iResult = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p, piFreeable);
  }अण्णहा{
    *piFreeable = 0;
    जइ( p->op==TK_SELECT ){
#जइ SQLITE_OMIT_SUBQUERY
      iResult = 0;
#अण्णहा
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }अण्णहा{
      पूर्णाङ्क i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p) );
      प्रति(i=0; i<nResult; i++){
        sqlite3_व्यञ्जकम्_Code_Factorable(pParse, p->x.pList->a[i].p_व्यञ्जकम्, i+iResult);
      }
    }
  }
  प्रत्यागम iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
स्थिर शून्य setDoNotMergeFlagOnCopy(Vdbe *v){
  जइ( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  व्यञ्जकम्_List *pFarg,      /* List of function arguments */
  पूर्णाङ्क iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  पूर्णाङ्क target            /* Store function result in this register */
){
  पूर्णाङ्क nFarg;
  Vdbe *v = pParse->pVdbe;
  प्रतिज्ञा( v!=0 );
  प्रतिज्ञा( pFarg!=0 );
  nFarg = pFarg->n_व्यञ्जकम्;
  प्रतिज्ञा( nFarg>0 );  /* All in-line functions have at least one argument */
  विकल्प( iFuncId ){
    पक्ष INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      पूर्णाङ्क endCoalesce = sqlite3VdbeMakeLabel(pParse);
      पूर्णाङ्क i;
      प्रतिज्ञा( nFarg>=2 );
      sqlite3_व्यञ्जकम्_Code(pParse, pFarg->a[0].p_व्यञ्जकम्, target);
      प्रति(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_व्यञ्जकम्_Code(pParse, pFarg->a[i].p_व्यञ्जकम्, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      भङ्ग;
    }
    पक्ष INLINEFUNC_iif: {
      व्यञ्जकम् case_व्यञ्जकम्;
      memset(&case_व्यञ्जकम्, 0, परिमाण(case_व्यञ्जकम्));
      case_व्यञ्जकम्.op = TK_CASE;
      case_व्यञ्जकम्.x.pList = pFarg;
      प्रत्यागम sqlite3_व्यञ्जकम्_Code_Target(pParse, &case_व्यञ्जकम्, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    पक्ष INLINEFUNC_sqlite_offset: {
      व्यञ्जकम् *pArg = pFarg->a[0].p_व्यञ्जकम्;
      जइ( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }अण्णहा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      भङ्ग;
    }
#endif
    स्वभाव: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      प्रतिज्ञा( nFarg==1 || nFarg==2 );
      target = sqlite3_व्यञ्जकम्_Code_Target(pParse, pFarg->a[0].p_व्यञ्जकम्, target);
      भङ्ग;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#जइ !defined(SQLITE_UNTESTABLE)
    पक्ष INLINEFUNC_व्यञ्जकम्_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      प्रतिज्ञा( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_व्यञ्जकम्_Compare(0,pFarg->a[0].p_व्यञ्जकम्, pFarg->a[1].p_व्यञ्जकम्,-1),
         target);
      भङ्ग;
    }

    पक्ष INLINEFUNC_व्यञ्जकम्_implies_व्यञ्जकम्: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      प्रतिज्ञा( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_व्यञ्जकम्_Implies_व्यञ्जकम्(pParse,pFarg->a[0].p_व्यञ्जकम्, pFarg->a[1].p_व्यञ्जकम्,-1),
         target);
      भङ्ग;
    }

    पक्ष INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      व्यञ्जकम् *pA1;
      प्रतिज्ञा( nFarg==2 );
      pA1 = pFarg->a[1].p_व्यञ्जकम्;
      जइ( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_व्यञ्जकम्_Implies_Non_Null_Row(pFarg->a[0].p_व्यञ्जकम्,pA1->iTable,1),
           target);
      }अण्णहा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      भङ्ग;
    }

    पक्ष INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      ध्रुव वर्ण *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      वर्ण aff;
      प्रतिज्ञा( nFarg==1 );
      aff = sqlite3_व्यञ्जकम्_Affinity(pFarg->a[0].p_व्यञ्जकम्);
      प्रतिज्ञा( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      भङ्ग;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  प्रत्यागम target;
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
स्थिर पूर्णाङ्क व्यञ्जकम्_ग्रन्थिः_Can_Return_Subtype(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क n;
  FuncDef *pDef;
  sqlite3 *db;
  जइ( p_व्यञ्जकम्->op==TK_CASE || p_व्यञ्जकम्->op==TK_UPLUS 
   || p_व्यञ्जकम्->op==TK_COLLATE || p_व्यञ्जकम्->op==TK_CAST 
  ){
    प्रत्यागम WRC_Continue;
  }
  जइ( p_व्यञ्जकम्->op!=TK_FUNCTION && p_व्यञ्जकम्->op!=TK_AGG_FUNCTION ){
    प्रत्यागम WRC_Prune;
  }
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
  db = pWalker->pParse->db;
  n = p_व्यञ्जकम्->x.pList ? p_व्यञ्जकम्->x.pList->n_व्यञ्जकम् : 0;
  pDef = sqlite3FindFunction(db, p_व्यञ्जकम्->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  जइ( pDef==0 ) प्रत्यागम WRC_Continue;
#अण्णहा
  प्रतिज्ञा( pDef!=0 );
#endif
  जइ( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    प्रत्यागम WRC_Abort;
  }
  प्रत्यागम WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Can_Return_Subtype(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्){
  Walker w;
  memset(&w, 0, परिमाण(w));
  w.pParse = pParse;
  w.x_व्यञ्जकम्_Callback = व्यञ्जकम्_ग्रन्थिः_Can_Return_Subtype;
  sqlite3_Walk_व्यञ्जकम्(&w, p_व्यञ्जकम्);
  प्रत्यागम w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
स्थिर SQLITE_NOINLINE पूर्णाङ्क sqlite3_Indexed_व्यञ्जकम्_Lookup(
  Parse *pParse,   /* The parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,     /* The expression to potentially bypass */
  पूर्णाङ्क target       /* Where to store the result of the expression */
){
  Indexed_व्यञ्जकम् *p;
  Vdbe *v;
  प्रति(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 व्यञ्जकम्_Aff;
    पूर्णाङ्क iDataCur = p->iDataCur;
    जइ( iDataCur<0 ) अनुवर्त;
    जइ( pParse->iSelfTab ){
      जइ( p->iDataCur!=pParse->iSelfTab-1 ) अनुवर्त;
      iDataCur = -1;
    }
    जइ( sqlite3_व्यञ्जकम्_Compare(0, p_व्यञ्जकम्, p->p_व्यञ्जकम्, iDataCur)!=0 ) अनुवर्त;
    प्रतिज्ञा( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    व्यञ्जकम्_Aff = sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्);
    जइ( (व्यञ्जकम्_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (व्यञ्जकम्_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (व्यञ्जकम्_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      अनुवर्त;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_SubtArg)
     && sqlite3_व्यञ्जकम्_Can_Return_Subtype(pParse, p_व्यञ्जकम्) 
    ){
      अनुवर्त;
    }

    v = pParse->pVdbe;
    प्रतिज्ञा( v!=0 );
    जइ( p->bMaybeNullRow || sqlite3_व्यञ्जकम्_Is_Constant(pParse, p_व्यञ्जकम्) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      पूर्णाङ्क addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_व्यञ्जकम्_Code(pParse, p_व्यञ्जकम्, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }अण्णहा{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    प्रत्यागम target;
  }
  प्रत्यागम -1;  /* Not found */
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
स्थिर पूर्णाङ्क व्यञ्जकम्_Partidx_व्यञ्जकम्_Lookup(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क iTarget){
  Indexed_व्यञ्जकम् *p;
  प्रति(p=pParse->p_Idx_Part_व्यञ्जकम्; p; p=p->pIENext){
    जइ( p_व्यञ्जकम्->iColumn==p->iIdxCol && p_व्यञ्जकम्->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      पूर्णाङ्क addr = 0;
      पूर्णाङ्क ret;

      जइ( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_व्यञ्जकम्_Code_Target(pParse, p->p_व्यञ्जकम्, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ध्रुव वर्ण*)&p->aff, 1);
      जइ( sqlite3_व्यञ्जकम्_Can_Return_Subtype(pParse, p->p_व्यञ्जकम्) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      जइ( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      प्रत्यागम ret;
    }
  }
  प्रत्यागम 0;
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
स्थिर SQLITE_NOINLINE पूर्णाङ्क व्यञ्जकम्_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,       /* AND or OR expression to be coded */
  पूर्णाङ्क target,        /* Put result in this register, guaranteed */
  पूर्णाङ्क *pTmpReg       /* Write a temporary register here */
){
  पूर्णाङ्क op;            /* The opcode.  TK_AND or TK_OR */
  पूर्णाङ्क skipOp;        /* Opcode for the branch that skips one operand */
  पूर्णाङ्क addrSkip;      /* Branch instruction that skips one of the operands */
  पूर्णाङ्क regSS = 0;     /* Register holding computed operand when other omitted */
  पूर्णाङ्क r1, r2;        /* Registers for left and right operands, respectively */
  व्यञ्जकम् *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  प्रतिज्ञा( p_व्यञ्जकम्!=0 );
  op = p_व्यञ्जकम्->op;
  प्रतिज्ञा( op==TK_AND || op==TK_OR );
  प्रतिज्ञा( TK_AND==OP_And );            testcase( op==TK_AND );
  प्रतिज्ञा( TK_OR==OP_Or );              testcase( op==TK_OR );
  प्रतिज्ञा( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_व्यञ्जकम्_Simplified_And_Or(p_व्यञ्जकम्);
  जइ( pAlt!=p_व्यञ्जकम् ){
    r1 = sqlite3_व्यञ्जकम्_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    प्रत्यागम target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  जइ( व्यञ्जकम्_Eval_Rhs_First(p_व्यञ्जकम्) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_व्यञ्जकम्_Code_Target(pParse, p_व्यञ्जकम्->p_दक्षिणः, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, pTmpReg);
  }अण्णहा{
    /* Compute the left operand first */
    r1 = sqlite3_व्यञ्जकम्_Code_Target(pParse, p_व्यञ्जकम्->p_वामः, target);
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्->p_दक्षिणः, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }अण्णहा{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_दक्षिणः, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  जइ( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  प्रत्यागम target;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Code_Target(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  पूर्णाङ्क op;                   /* The opcode being coded */
  पूर्णाङ्क inReg = target;       /* Results stored in register inReg */
  पूर्णाङ्क regFree1 = 0;         /* If non-zero free this temporary register */
  पूर्णाङ्क regFree2 = 0;         /* If non-zero free this temporary register */
  पूर्णाङ्क r1, r2;               /* Various register numbers */
  व्यञ्जकम् tempX;               /* Temporary expression node */
  पूर्णाङ्क p5 = 0;

  प्रतिज्ञा( target>0 && target<=pParse->nMem );
  प्रतिज्ञा( v!=0 );

व्यञ्जकम्_code_doover:
  जइ( p_व्यञ्जकम्==0 ){
    op = TK_NULL;
  }अण्णहा जइ( pParse->pIdxEpr!=0
   && !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Leaf)
   && (r1 = sqlite3_Indexed_व्यञ्जकम्_Lookup(pParse, p_व्यञ्जकम्, target))>=0
  ){
    प्रत्यागम r1;
  }अण्णहा{
    प्रतिज्ञा( !व्यञ्जकम्_Has_VVAProperty(p_व्यञ्जकम्,EP_Immutable) );
    op = p_व्यञ्जकम्->op;
  }
  प्रतिज्ञा( op!=TK_ORDER );
  विकल्प( op ){
    पक्ष TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_व्यञ्जकम्->pAggInfo;
      संरचना AggInfo_col *pCol;
      प्रतिज्ञा( pAggInfo!=0 );
      प्रतिज्ञा( p_व्यञ्जकम्->iAgg>=0 );
      जइ( p_व्यञ्जकम्->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        भङ्ग;
      }
      pCol = &pAggInfo->aCol[p_व्यञ्जकम्->iAgg];
      जइ( !pAggInfo->directMode ){
        प्रत्यागम AggInfoColumnReg(pAggInfo, p_व्यञ्जकम्->iAgg);
      }अण्णहा जइ( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        जइ( pTab==0 ){
          /* No comment added */
        }अण्णहा जइ( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }अण्णहा{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          जइ( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        प्रत्यागम target;
      }अण्णहा जइ( p_व्यञ्जकम्->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_व्यञ्जकम्->iTable, p_व्यञ्जकम्->iColumn, target);
        प्रत्यागम target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    पक्ष TK_COLUMN: {
      पूर्णाङ्क iTab = p_व्यञ्जकम्->iTable;
      पूर्णाङ्क iReg;
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        पूर्णाङ्क aff;
        iReg = sqlite3_व्यञ्जकम्_Code_Target(pParse, p_व्यञ्जकम्->p_वामः,target);
        प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) );
        प्रतिज्ञा( p_व्यञ्जकम्->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_व्यञ्जकम्->y.pTab, p_व्यञ्जकम्->iColumn);
        जइ( aff>SQLITE_AFF_BLOB ){
          स्थिर ध्रुव वर्ण zAff[] = "B\000C\000D\000E\000F";
          प्रतिज्ञा( SQLITE_AFF_BLOB=='A' );
          प्रतिज्ञा( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        प्रत्यागम iReg;
      }
      जइ( iTab<0 ){
        जइ( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          पूर्णाङ्क iSrc;
          पूर्णाङ्क iCol = p_व्यञ्जकम्->iColumn;
          प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) );
          pTab = p_व्यञ्जकम्->y.pTab;
          प्रतिज्ञा( pTab!=0 );
          प्रतिज्ञा( iCol>=XN_ROWID );
          प्रतिज्ञा( iCol<pTab->nCol );
          जइ( iCol<0 ){
            प्रत्यागम -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          जइ( pCol->colFlags & COLFLAG_GENERATED ){
            जइ( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_सन्देशः(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              प्रत्यागम 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            जइ( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_व्यञ्जकम्_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            प्रत्यागम iSrc;
          }अण्णहा
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          जइ( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            प्रत्यागम target;
          }अण्णहा{
            प्रत्यागम iSrc;
          }
        }अण्णहा{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      अण्णहा जइ( pParse->p_Idx_Part_व्यञ्जकम् 
       && 0!=(r1 = व्यञ्जकम्_Partidx_व्यञ्जकम्_Lookup(pParse, p_व्यञ्जकम्, target))
      ){
        प्रत्यागम r1;
      }
      प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) );
      प्रतिज्ञा( p_व्यञ्जकम्->y.pTab!=0 );
      iReg = sqlite3_व्यञ्जकम्_Code_Get_Column(pParse, p_व्यञ्जकम्->y.pTab,
                               p_व्यञ्जकम्->iColumn, iTab, target,
                               p_व्यञ्जकम्->op2);
      प्रत्यागम iReg;
    }
    पक्ष TK_INTEGER: {
      codeInteger(pParse, p_व्यञ्जकम्, 0, target);
      प्रत्यागम target;
    }
    पक्ष TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_व्यञ्जकम्_Truth_मानम्(p_व्यञ्जकम्), target);
      प्रत्यागम target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    पक्ष TK_FLOAT: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      codeReal(v, p_व्यञ्जकम्->u.zToken, 0, target);
      प्रत्यागम target;
    }
#endif
    पक्ष TK_STRING: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3VdbeLoadString(v, target, p_व्यञ्जकम्->u.zToken);
      प्रत्यागम target;
    }
    पक्ष TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_व्यञ्जकम्->y.nReg - 1);
      प्रत्यागम target;
    }
    स्वभाव: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      प्रतिज्ञा( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      प्रत्यागम target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    पक्ष TK_BLOB: {
      पूर्णाङ्क n;
      ध्रुव वर्ण *z;
      वर्ण *zBlob;
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      प्रतिज्ञा( p_व्यञ्जकम्->u.zToken[0]=='x' || p_व्यञ्जकम्->u.zToken[0]=='X' );
      प्रतिज्ञा( p_व्यञ्जकम्->u.zToken[1]=='\'' );
      z = &p_व्यञ्जकम्->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      प्रतिज्ञा( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      प्रत्यागम target;
    }
#endif
    पक्ष TK_VARIABLE: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      प्रतिज्ञा( p_व्यञ्जकम्->u.zToken!=0 );
      प्रतिज्ञा( p_व्यञ्जकम्->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_व्यञ्जकम्->iColumn, target);
      प्रत्यागम target;
    }
    पक्ष TK_REGISTER: {
      प्रत्यागम p_व्यञ्जकम्->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    पक्ष TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_व्यञ्जकम्_Code(pParse, p_व्यञ्जकम्->p_वामः, target);
      प्रतिज्ञा( inReg==target );
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_व्यञ्जकम्->u.zToken, 0));
      प्रत्यागम inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    पक्ष TK_IS:
    पक्ष TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    पक्ष TK_LT:
    पक्ष TK_LE:
    पक्ष TK_GT:
    पक्ष TK_GE:
    पक्ष TK_NE:
    पक्ष TK_EQ: {
      व्यञ्जकम् *p_वामः = p_व्यञ्जकम्->p_वामः;
      पूर्णाङ्क addrIsNull = 0;
      जइ( sqlite3_व्यञ्जकम्_Is_Vector(p_वामः) ){
        codeVectorCompare(pParse, p_व्यञ्जकम्, target, op, p5);
      }अण्णहा{
        जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = व्यञ्जकम्_Compute_Operands(pParse, p_व्यञ्जकम्,
                                     &r1, &r2, &regFree1, &regFree2);
        }अण्णहा{
          r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
          r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_दक्षिणः, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_वामः, p_व्यञ्जकम्->p_दक्षिणः, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_Commuted));
        प्रतिज्ञा(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        प्रतिज्ञा(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        प्रतिज्ञा(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        प्रतिज्ञा(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        प्रतिज्ञा(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        प्रतिज्ञा(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        जइ( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }अण्णहा{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          जइ( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      भङ्ग;
    }
    पक्ष TK_AND:
    पक्ष TK_OR: {
      inReg = व्यञ्जकम्_Code_Target_And_Or(pParse, p_व्यञ्जकम्, target, &regFree1);
      भङ्ग;
    }
    पक्ष TK_PLUS:
    पक्ष TK_STAR:
    पक्ष TK_MINUS:
    पक्ष TK_REM:
    पक्ष TK_BITAND:
    पक्ष TK_BITOR:
    पक्ष TK_SLASH:
    पक्ष TK_LSHIFT:
    पक्ष TK_RSHIFT:
    पक्ष TK_CONCAT: {
      पूर्णाङ्क addrIsNull;
      प्रतिज्ञा( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      प्रतिज्ञा( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      प्रतिज्ञा( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      प्रतिज्ञा( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      प्रतिज्ञा( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      प्रतिज्ञा( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      प्रतिज्ञा( TK_LSHIFT==OP_Shift_वामः );   testcase( op==TK_LSHIFT );
      प्रतिज्ञा( TK_RSHIFT==OP_Shift_दक्षिणः );  testcase( op==TK_RSHIFT );
      प्रतिज्ञा( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subquery) ){
        addrIsNull = व्यञ्जकम्_Compute_Operands(pParse, p_व्यञ्जकम्,
                                   &r1, &r2, &regFree1, &regFree2);
      }अण्णहा{
        r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
        r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_दक्षिणः, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      जइ( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      भङ्ग;
    }
    पक्ष TK_UMINUS: {
      व्यञ्जकम् *p_वामः = p_व्यञ्जकम्->p_वामः;
      प्रतिज्ञा( p_वामः );
      जइ( p_वामः->op==TK_INTEGER ){
        codeInteger(pParse, p_वामः, 1, target);
        प्रत्यागम target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }अण्णहा जइ( p_वामः->op==TK_FLOAT ){
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
        codeReal(v, p_वामः->u.zToken, 1, target);
        प्रत्यागम target;
#endif
      }अण्णहा{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_मानम्|EP_TokenOnly;
        tempX.u.i_मानम् = 0;
        व्यञ्जकम्_Clear_VVAProperties(&tempX);
        r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      भङ्ग;
    }
    पक्ष TK_BITNOT:
    पक्ष TK_NOT: {
      प्रतिज्ञा( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      प्रतिज्ञा( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      भङ्ग;
    }
    पक्ष TK_TRUTH: {
      पूर्णाङ्क isTrue;    /* IS TRUE or IS NOT TRUE */
      पूर्णाङ्क bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_व्यञ्जकम्_Truth_मानम्(p_व्यञ्जकम्->p_दक्षिणः);
      bNormal = p_व्यञ्जकम्->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      भङ्ग;
    }
    पक्ष TK_ISNULL:
    पक्ष TK_NOTNULL: {
      पूर्णाङ्क addr;
      प्रतिज्ञा( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      प्रतिज्ञा( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      भङ्ग;
    }
    पक्ष TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_व्यञ्जकम्->pAggInfo;
      जइ( pInfo==0
       || NEVER(p_व्यञ्जकम्->iAgg<0)
       || NEVER(p_व्यञ्जकम्->iAgg>=pInfo->nFunc)
      ){
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
        sqlite3_Error_सन्देशः(pParse, "misuse of aggregate: %#T()", p_व्यञ्जकम्);
      }अण्णहा{
        प्रत्यागम AggInfoFuncReg(pInfo, p_व्यञ्जकम्->iAgg);
      }
      भङ्ग;
    }
    पक्ष TK_FUNCTION: {
      व्यञ्जकम्_List *pFarg;       /* List of function arguments */
      पूर्णाङ्क nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ध्रुव वर्ण *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      पूर्णाङ्क i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) ){
        प्रत्यागम p_व्यञ्जकम्->y.pWin->regResult;
      }
#endif

      जइ( ConstFactorOk(pParse)
       && sqlite3_व्यञ्जकम्_Is_Constant_Not_Join(pParse,p_व्यञ्जकम्)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        प्रत्यागम sqlite3_व्यञ्जकम्_Code_Run_Just_Once(pParse, p_व्यञ्जकम्, -1);
      }
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_TokenOnly) );
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
      pFarg = p_व्यञ्जकम्->x.pList;
      nFarg = pFarg ? pFarg->n_व्यञ्जकम् : 0;
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      zId = p_व्यञ्जकम्->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      जइ( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      जइ( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_सन्देशः(pParse, "unknown function: %#T()", p_व्यञ्जकम्);
        भङ्ग;
      }
      जइ( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        प्रतिज्ञा( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        प्रतिज्ञा( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        प्रत्यागम व्यञ्जकम्_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }अण्णहा जइ( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_व्यञ्जकम्_Function_Usable(pParse, p_व्यञ्जकम्, pDef);
      }

      प्रति(i=0; i<nFarg; i++){
        जइ( i<32 && sqlite3_व्यञ्जकम्_Is_Constant(pParse, pFarg->a[i].p_व्यञ्जकम्) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        जइ( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, pFarg->a[i].p_व्यञ्जकम्);
        }
      }
      जइ( pFarg ){
        जइ( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }अण्णहा{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        जइ( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 व्यञ्जकम्_Op;
          प्रतिज्ञा( nFarg==1 );
          प्रतिज्ञा( pFarg->a[0].p_व्यञ्जकम्!=0 );
          व्यञ्जकम्_Op = pFarg->a[0].p_व्यञ्जकम्->op;
          जइ( व्यञ्जकम्_Op==TK_COLUMN || व्यञ्जकम्_Op==TK_AGG_COLUMN ){
            प्रतिज्ञा( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            प्रतिज्ञा( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            प्रतिज्ञा( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            प्रतिज्ञा( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_व्यञ्जकम्->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_व्यञ्जकम्_Code_व्यञ्जकम्_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }अण्णहा{
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
      जइ( nFarg>=2 && व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_व्यञ्जकम्);
      }अण्णहा जइ( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_व्यञ्जकम्);
      }
#endif
      जइ( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        जइ( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (वर्ण *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_व्यञ्जकम्->op2);
      जइ( nFarg ){
        जइ( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }अण्णहा{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      प्रत्यागम target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    पक्ष TK_EXISTS:
    पक्ष TK_SELECT: {
      पूर्णाङ्क nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      जइ( pParse->db->mallocFailed ){
        प्रत्यागम 0;
      }अण्णहा जइ( op==TK_SELECT
             && ALWAYS( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) )
             && (nCol = p_व्यञ्जकम्->x.pSelect->pEList->n_व्यञ्जकम्)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }अण्णहा{
        प्रत्यागम sqlite3CodeSubselect(pParse, p_व्यञ्जकम्);
      }
      भङ्ग;
    }
    पक्ष TK_SELECT_COLUMN: {
      पूर्णाङ्क n;
      व्यञ्जकम् *p_वामः = p_व्यञ्जकम्->p_वामः;
      जइ( p_वामः->iTable==0 || pParse->withinRJSubrtn > p_वामः->op2 ){
        p_वामः->iTable = sqlite3CodeSubselect(pParse, p_वामः);
        p_वामः->op2 = pParse->withinRJSubrtn;
      }
      प्रतिज्ञा( p_वामः->op==TK_SELECT || p_वामः->op==TK_ERROR );
      n = sqlite3_व्यञ्जकम्_Vector_Size(p_वामः);
      जइ( p_व्यञ्जकम्->iTable!=n ){
        sqlite3_Error_सन्देशः(pParse, "%d columns assigned %d values",
                                p_व्यञ्जकम्->iTable, n);
      }
      प्रत्यागम p_वामः->iTable + p_व्यञ्जकम्->iColumn;
    }
    पक्ष TK_IN: {
      पूर्णाङ्क destIfFalse = sqlite3VdbeMakeLabel(pParse);
      पूर्णाङ्क destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_व्यञ्जकम्_Code_IN(pParse, p_व्यञ्जकम्, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      प्रत्यागम target;
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
    पक्ष TK_BETWEEN: {
      व्यञ्जकम्_Code_Between(pParse, p_व्यञ्जकम्, target, 0, 0);
      प्रत्यागम target;
    }
    पक्ष TK_COLLATE: {
      जइ( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        प्रतिज्ञा( p_व्यञ्जकम्->p_वामः );
        sqlite3_व्यञ्जकम्_Code(pParse, p_व्यञ्जकम्->p_वामः, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        प्रत्यागम target;
      }अण्णहा{
        p_व्यञ्जकम् = p_व्यञ्जकम्->p_वामः;
        गच्छ व्यञ्जकम्_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    पक्ष TK_SPAN:
    पक्ष TK_UPLUS: {
      p_व्यञ्जकम् = p_व्यञ्जकम्->p_वामः;
      गच्छ व्यञ्जकम्_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    पक्ष TK_TRIGGER: {
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
      पूर्णाङ्क iCol;
      पूर्णाङ्क p1;

      प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) );
      pTab = p_व्यञ्जकम्->y.pTab;
      iCol = p_व्यञ्जकम्->iColumn;
      p1 = p_व्यञ्जकम्->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      प्रतिज्ञा( p_व्यञ्जकम्->iTable==0 || p_व्यञ्जकम्->iTable==1 );
      प्रतिज्ञा( iCol>=-1 && iCol<pTab->nCol );
      प्रतिज्ञा( pTab->iPKey<0 || iCol!=pTab->iPKey );
      प्रतिज्ञा( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_व्यञ्जकम्->iTable ? "new" : "old"),
        (p_व्यञ्जकम्->iColumn<0 ? "rowid" : p_व्यञ्जकम्->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      जइ( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      भङ्ग;
    }

    पक्ष TK_VECTOR: {
      sqlite3_Error_सन्देशः(pParse, "row value misused");
      भङ्ग;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    पक्ष TK_IF_NULL_ROW: {
      पूर्णाङ्क addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_व्यञ्जकम्->pAggInfo;
      जइ( pAggInfo ){
        प्रतिज्ञा( p_व्यञ्जकम्->iAgg>=0 && p_व्यञ्जकम्->iAgg<pAggInfo->nColumn );
        जइ( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_व्यञ्जकम्->iAgg);
          भङ्ग;
        }
        जइ( p_व्यञ्जकम्->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_व्यञ्जकम्->iAgg].iSorterColumn,
                            target);
          inReg = target;
          भङ्ग;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_व्यञ्जकम्->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_व्यञ्जकम्_Code(pParse, p_व्यञ्जकम्->p_वामः, target);
      प्रतिज्ञा( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      भङ्ग;
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
    पक्ष TK_CASE: {
      पूर्णाङ्क endLabel;                     /* GOTO label for end of CASE stmt */
      पूर्णाङ्क nextCase;                     /* GOTO label for next WHEN clause */
      पूर्णाङ्क n_व्यञ्जकम्;                        /* 2x number of WHEN terms */
      पूर्णाङ्क i;                            /* Loop counter */
      व्यञ्जकम्_List *pEList;                 /* List of WHEN terms */
      संरचना व्यञ्जकम्_List_item *aListelem;  /* Array of WHEN terms */
      व्यञ्जकम् opCompare;                   /* The X==Ei expression */
      व्यञ्जकम् *pX;                         /* The X expression */
      व्यञ्जकम् *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      व्यञ्जकम् *pDel = 0;
      sqlite3 *db = pParse->db;

      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) && p_व्यञ्जकम्->x.pList!=0 );
      प्रतिज्ञा(p_व्यञ्जकम्->x.pList->n_व्यञ्जकम् > 0);
      pEList = p_व्यञ्जकम्->x.pList;
      aListelem = pEList->a;
      n_व्यञ्जकम् = pEList->n_व्यञ्जकम्;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      जइ( (pX = p_व्यञ्जकम्->p_वामः)!=0 ){
        pDel = sqlite3_व्यञ्जकम्_Dup(db, pX, 0);
        जइ( db->mallocFailed ){
          sqlite3_व्यञ्जकम्_Delete(db, pDel);
          भङ्ग;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_व्यञ्जकम्_To_Register(pDel, व्यञ्जकम्_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, परिमाण(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_वामः = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      प्रति(i=0; i<n_व्यञ्जकम्-1; i=i+2){
        जइ( pX ){
          प्रतिज्ञा( pTest!=0 );
          opCompare.p_दक्षिणः = aListelem[i].p_व्यञ्जकम्;
        }अण्णहा{
          pTest = aListelem[i].p_व्यञ्जकम्;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_व्यञ्जकम्_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_व्यञ्जकम्->op==TK_COLUMN );
        sqlite3_व्यञ्जकम्_Code(pParse, aListelem[i+1].p_व्यञ्जकम्, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      जइ( (n_व्यञ्जकम्&1)!=0 ){
        sqlite3_व्यञ्जकम्_Code(pParse, pEList->a[n_व्यञ्जकम्-1].p_व्यञ्जकम्, target);
      }अण्णहा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_व्यञ्जकम्_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      भङ्ग;
    }
#ifndef SQLITE_OMIT_TRIGGER
    पक्ष TK_RAISE: {
      प्रतिज्ञा( p_व्यञ्जकम्->aff_व्यञ्जकम्==OE_Rollback
           || p_व्यञ्जकम्->aff_व्यञ्जकम्==OE_Abort
           || p_व्यञ्जकम्->aff_व्यञ्जकम्==OE_Fail
           || p_व्यञ्जकम्->aff_व्यञ्जकम्==OE_Ignore
      );
      जइ( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_सन्देशः(pParse,
                       "RAISE() may only be used within a trigger-program");
        प्रत्यागम 0;
      }
      जइ( p_व्यञ्जकम्->aff_व्यञ्जकम्==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      जइ( p_व्यञ्जकम्->aff_व्यञ्जकम्==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }अण्णहा{
        r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_व्यञ्जकम्->aff_व्यञ्जकम्, r1);
      }
      भङ्ग;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  प्रत्यागम inReg;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  व्यञ्जकम् *p_व्यञ्जकम्,      /* The expression to code when the VDBE initializes */
  पूर्णाङ्क regDest       /* Store the value in this register */
){
  व्यञ्जकम्_List *p;
  प्रतिज्ञा( ConstFactorOk(pParse) );
  प्रतिज्ञा( regDest!=0 );
  p = pParse->p_Const_व्यञ्जकम्;
  जइ( regDest<0 && p ){
    संरचना व्यञ्जकम्_List_item *pItem;
    पूर्णाङ्क i;
    प्रति(pItem=p->a, i=p->n_व्यञ्जकम्; i>0; pItem++, i--){
      जइ( pItem->fg.reusable
       && sqlite3_व्यञ्जकम्_Compare(0,pItem->p_व्यञ्जकम्,p_व्यञ्जकम्,-1)==0
      ){
        प्रत्यागम pItem->u.i_Const_व्यञ्जकम्_Reg;
      }
    }
  }
  p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(pParse->db, p_व्यञ्जकम्, 0);
  जइ( p_व्यञ्जकम्!=0 && व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    पूर्णाङ्क addr;
    प्रतिज्ञा( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    जइ( !pParse->db->mallocFailed ){
      जइ( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_व्यञ्जकम्_Code(pParse, p_व्यञ्जकम्, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_व्यञ्जकम्_Delete(pParse->db, p_व्यञ्जकम्);
    sqlite3VdbeJumpHere(v, addr);
  }अण्णहा{
    p = sqlite3_व्यञ्जकम्_List_Append(pParse, p, p_व्यञ्जकम्);
    जइ( p ){
       संरचना व्यञ्जकम्_List_item *pItem = &p->a[p->n_व्यञ्जकम्-1];
       pItem->fg.reusable = regDest<0;
       जइ( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_व्यञ्जकम्_Reg = regDest;
    }
    pParse->p_Const_व्यञ्जकम् = p;
  }
  प्रत्यागम regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE शून्य sqlite3_व्यञ्जकम्_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  पूर्णाङ्क iReg,        /* First register to set to NULL */
  पूर्णाङ्क nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  व्यञ्जकम् t;
  memset(&t, 0, परिमाण(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_व्यञ्जकम्_Code_Run_Just_Once(pParse, &t, iReg);
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Code_Temp(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क *pReg){
  पूर्णाङ्क r2;
  p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p_व्यञ्जकम्);
  जइ( ConstFactorOk(pParse)
   && ALWAYS(p_व्यञ्जकम्!=0)
   && p_व्यञ्जकम्->op!=TK_REGISTER
   && sqlite3_व्यञ्जकम्_Is_Constant_Not_Join(pParse, p_व्यञ्जकम्)
  ){
    *pReg  = 0;
    r2 = sqlite3_व्यञ्जकम्_Code_Run_Just_Once(pParse, p_व्यञ्जकम्, -1);
  }अण्णहा{
    पूर्णाङ्क r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_व्यञ्जकम्_Code_Target(pParse, p_व्यञ्जकम्, r1);
    जइ( r2==r1 ){
      *pReg = r1;
    }अण्णहा{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  प्रत्यागम r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
शून्य sqlite3_व्यञ्जकम्_Code(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क target){
  पूर्णाङ्क inReg;

  प्रतिज्ञा( p_व्यञ्जकम्==0 || !व्यञ्जकम्_Has_VVAProperty(p_व्यञ्जकम्,EP_Immutable) );
  प्रतिज्ञा( target>0 && target<=pParse->nMem );
  प्रतिज्ञा( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  जइ( pParse->pVdbe==0 ) प्रत्यागम;
  inReg = sqlite3_व्यञ्जकम्_Code_Target(pParse, p_व्यञ्जकम्, target);
  जइ( inReg!=target ){
    u8 op;
    व्यञ्जकम् *pX = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p_व्यञ्जकम्);
    testcase( pX!=p_व्यञ्जकम् );
    जइ( ALWAYS(pX)
     && (व्यञ्जकम्_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }अण्णहा{
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
शून्य sqlite3_व्यञ्जकम्_Code_Copy(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क target){
  sqlite3 *db = pParse->db;
  p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्, 0);
  जइ( !db->mallocFailed ) sqlite3_व्यञ्जकम्_Code(pParse, p_व्यञ्जकम्, target);
  sqlite3_व्यञ्जकम्_Delete(db, p_व्यञ्जकम्);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
शून्य sqlite3_व्यञ्जकम्_Code_Factorable(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क target){
  जइ( pParse->okConstFactor && sqlite3_व्यञ्जकम्_Is_Constant_Not_Join(pParse,p_व्यञ्जकम्) ){
    sqlite3_व्यञ्जकम्_Code_Run_Just_Once(pParse, p_व्यञ्जकम्, target);
  }अण्णहा{
    sqlite3_व्यञ्जकम्_Code_Copy(pParse, p_व्यञ्जकम्, target);
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Code_व्यञ्जकम्_List(
  Parse *pParse,     /* Parsing context */
  व्यञ्जकम्_List *pList,   /* The expression list to be coded */
  पूर्णाङ्क target,        /* Where to write results */
  पूर्णाङ्क srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  संरचना व्यञ्जकम्_List_item *pItem;
  पूर्णाङ्क i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  प्रतिज्ञा( pList!=0 );
  प्रतिज्ञा( target>0 );
  प्रतिज्ञा( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_व्यञ्जकम्;
  जइ( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  प्रति(pItem=pList->a, i=0; i<n; i++, pItem++){
    व्यञ्जकम् *p_व्यञ्जकम् = pItem->p_व्यञ्जकम्;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    जइ( pItem->fg.bSorterRef ){
      i--;
      n--;
    }अण्णहा
#endif
    जइ( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      जइ( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }अण्णहा{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }अण्णहा जइ( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_व्यञ्जकम्_Is_Constant_Not_Join(pParse,p_व्यञ्जकम्)
    ){
      sqlite3_व्यञ्जकम्_Code_Run_Just_Once(pParse, p_व्यञ्जकम्, target+i);
    }अण्णहा{
      पूर्णाङ्क inReg = sqlite3_व्यञ्जकम्_Code_Target(pParse, p_व्यञ्जकम्, target+i);
      जइ( inReg!=target+i ){
        VdbeOp *pOp;
        जइ( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }अण्णहा{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  प्रत्यागम n;
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
स्थिर शून्य व्यञ्जकम्_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  व्यञ्जकम् *p_व्यञ्जकम्,      /* The BETWEEN expression */
  पूर्णाङ्क dest,         /* Jump destination or storage location */
  शून्य (*xJump)(Parse*,व्यञ्जकम्*,पूर्णाङ्क,पूर्णाङ्क), /* Action to take */
  पूर्णाङ्क jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  व्यञ्जकम् व्यञ्जकम्_And;     /* The AND operator in  x>=y AND x<=z  */
  व्यञ्जकम् comp_वामः;    /* The  x>=y  term */
  व्यञ्जकम् comp_दक्षिणः;   /* The  x<=z  term */
  पूर्णाङ्क regFree1 = 0; /* Temporary use register */
  व्यञ्जकम् *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_वामः, 0, परिमाण(व्यञ्जकम्));
  memset(&comp_दक्षिणः, 0, परिमाण(व्यञ्जकम्));
  memset(&व्यञ्जकम्_And, 0, परिमाण(व्यञ्जकम्));

  प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
  pDel = sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्->p_वामः, 0);
  जइ( db->mallocFailed==0 ){
    व्यञ्जकम्_And.op = TK_AND;
    व्यञ्जकम्_And.p_वामः = &comp_वामः;
    व्यञ्जकम्_And.p_दक्षिणः = &comp_दक्षिणः;
    comp_वामः.op = TK_GE;
    comp_वामः.p_वामः = pDel;
    comp_वामः.p_दक्षिणः = p_व्यञ्जकम्->x.pList->a[0].p_व्यञ्जकम्;
    comp_दक्षिणः.op = TK_LE;
    comp_दक्षिणः.p_वामः = pDel;
    comp_दक्षिणः.p_दक्षिणः = p_व्यञ्जकम्->x.pList->a[1].p_व्यञ्जकम्;
    sqlite3_व्यञ्जकम्_To_Register(pDel, व्यञ्जकम्_Code_Vector(pParse, pDel, &regFree1));
    जइ( xJump ){
      xJump(pParse, &व्यञ्जकम्_And, dest, jumpIfNull);
    }अण्णहा{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_व्यञ्जकम्_Code_Target(pParse, &व्यञ्जकम्_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3_व्यञ्जकम्_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_व्यञ्जकम्_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_व्यञ्जकम्_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_व्यञ्जकम्_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_व्यञ्जकम्_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_व्यञ्जकम्_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_व्यञ्जकम्_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_व्यञ्जकम्_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_व्यञ्जकम्_If_False && jumpIfNull!=0 && regFree1!=0 );
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
शून्य sqlite3_व्यञ्जकम्_If_True(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क dest, पूर्णाङ्क jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क op = 0;
  पूर्णाङ्क regFree1 = 0;
  पूर्णाङ्क regFree2 = 0;
  पूर्णाङ्क r1, r2;

  प्रतिज्ञा( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  जइ( NEVER(v==0) )     प्रत्यागम;  /* Existence of VDBE checked by caller */
  जइ( NEVER(p_व्यञ्जकम्==0) ) प्रत्यागम;  /* No way this can happen */
  प्रतिज्ञा( !व्यञ्जकम्_Has_VVAProperty(p_व्यञ्जकम्, EP_Immutable) );
  op = p_व्यञ्जकम्->op;
  विकल्प( op ){
    पक्ष TK_AND:
    पक्ष TK_OR: {
      व्यञ्जकम् *pAlt = sqlite3_व्यञ्जकम्_Simplified_And_Or(p_व्यञ्जकम्);
      जइ( pAlt!=p_व्यञ्जकम् ){
        sqlite3_व्यञ्जकम्_If_True(pParse, pAlt, dest, jumpIfNull);
      }अण्णहा{
        व्यञ्जकम् *pFirst, *pSecond;
        जइ( व्यञ्जकम्_Eval_Rhs_First(p_व्यञ्जकम्) ){
          pFirst = p_व्यञ्जकम्->p_दक्षिणः;
          pSecond = p_व्यञ्जकम्->p_वामः;
        }अण्णहा{
          pFirst = p_व्यञ्जकम्->p_वामः;
          pSecond = p_व्यञ्जकम्->p_दक्षिणः;
        }
        जइ( op==TK_AND ){
          पूर्णाङ्क d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_व्यञ्जकम्_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_व्यञ्जकम्_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }अण्णहा{
          testcase( jumpIfNull==0 );
          sqlite3_व्यञ्जकम्_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_व्यञ्जकम्_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      भङ्ग;
    }
    पक्ष TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_व्यञ्जकम्_If_False(pParse, p_व्यञ्जकम्->p_वामः, dest, jumpIfNull);
      भङ्ग;
    }
    पक्ष TK_TRUTH: {
      पूर्णाङ्क isNot;      /* IS NOT TRUE or IS NOT FALSE */
      पूर्णाङ्क isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_व्यञ्जकम्->op2==TK_ISNOT;
      isTrue = sqlite3_व्यञ्जकम्_Truth_मानम्(p_व्यञ्जकम्->p_दक्षिणः);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      जइ( isTrue ^ isNot ){
        sqlite3_व्यञ्जकम्_If_True(pParse, p_व्यञ्जकम्->p_वामः, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }अण्णहा{
        sqlite3_व्यञ्जकम्_If_False(pParse, p_व्यञ्जकम्->p_वामः, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      भङ्ग;
    }
    पक्ष TK_IS:
    पक्ष TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    पक्ष TK_LT:
    पक्ष TK_LE:
    पक्ष TK_GT:
    पक्ष TK_GE:
    पक्ष TK_NE:
    पक्ष TK_EQ: {
      पूर्णाङ्क addrIsNull;
      जइ( sqlite3_व्यञ्जकम्_Is_Vector(p_व्यञ्जकम्->p_वामः) ) गच्छ default_व्यञ्जकम्;
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = व्यञ्जकम्_Compute_Operands(pParse, p_व्यञ्जकम्,
                                   &r1, &r2, &regFree1, &regFree2);
      }अण्णहा{
        r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
        r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_दक्षिणः, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_व्यञ्जकम्->p_वामः, p_व्यञ्जकम्->p_दक्षिणः, op,
                  r1, r2, dest, jumpIfNull, व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_Commuted));
      प्रतिज्ञा(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      प्रतिज्ञा(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      प्रतिज्ञा(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      प्रतिज्ञा(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      प्रतिज्ञा(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      प्रतिज्ञा(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      जइ( addrIsNull ){
        जइ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }अण्णहा{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      भङ्ग;
    }
    पक्ष TK_ISNULL:
    पक्ष TK_NOTNULL: {
      प्रतिज्ञा( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      प्रतिज्ञा( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
      प्रतिज्ञा( regFree1==0 || regFree1==r1 );
      जइ( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      भङ्ग;
    }
    पक्ष TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      व्यञ्जकम्_Code_Between(pParse, p_व्यञ्जकम्, dest, sqlite3_व्यञ्जकम्_If_True, jumpIfNull);
      भङ्ग;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    पक्ष TK_IN: {
      पूर्णाङ्क destIfFalse = sqlite3VdbeMakeLabel(pParse);
      पूर्णाङ्क destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_व्यञ्जकम्_Code_IN(pParse, p_व्यञ्जकम्, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      भङ्ग;
    }
#endif
    स्वभाव: {
    default_व्यञ्जकम्:
      जइ( व्यञ्जकम्_Always_True(p_व्यञ्जकम्) ){
        sqlite3VdbeGoto(v, dest);
      }अण्णहा जइ( व्यञ्जकम्_Always_False(p_व्यञ्जकम्) ){
        /* No-op */
      }अण्णहा{
        r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      भङ्ग;
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
शून्य sqlite3_व्यञ्जकम्_If_False(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क dest, पूर्णाङ्क jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क op = 0;
  पूर्णाङ्क regFree1 = 0;
  पूर्णाङ्क regFree2 = 0;
  पूर्णाङ्क r1, r2;

  प्रतिज्ञा( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  जइ( NEVER(v==0) ) प्रत्यागम; /* Existence of VDBE checked by caller */
  जइ( p_व्यञ्जकम्==0 )    प्रत्यागम;
  प्रतिज्ञा( !व्यञ्जकम्_Has_VVAProperty(p_व्यञ्जकम्,EP_Immutable) );

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
  op = ((p_व्यञ्जकम्->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_ISNULL || op==OP_NotNull );
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_NOTNULL || op==OP_IsNull );
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_NE || op==OP_Eq );
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_EQ || op==OP_Ne );
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_LT || op==OP_Ge );
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_LE || op==OP_Gt );
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_GT || op==OP_Le );
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_GE || op==OP_Lt );

  विकल्प( p_व्यञ्जकम्->op ){
    पक्ष TK_AND:
    पक्ष TK_OR: {
      व्यञ्जकम् *pAlt = sqlite3_व्यञ्जकम्_Simplified_And_Or(p_व्यञ्जकम्);
      जइ( pAlt!=p_व्यञ्जकम् ){
        sqlite3_व्यञ्जकम्_If_False(pParse, pAlt, dest, jumpIfNull);
      }अण्णहा{
        व्यञ्जकम् *pFirst, *pSecond;
        जइ( व्यञ्जकम्_Eval_Rhs_First(p_व्यञ्जकम्) ){
          pFirst = p_व्यञ्जकम्->p_दक्षिणः;
          pSecond = p_व्यञ्जकम्->p_वामः;
        }अण्णहा{
          pFirst = p_व्यञ्जकम्->p_वामः;
          pSecond = p_व्यञ्जकम्->p_दक्षिणः;
        }
        जइ( p_व्यञ्जकम्->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_व्यञ्जकम्_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_व्यञ्जकम्_If_False(pParse, pSecond, dest, jumpIfNull);
        }अण्णहा{
          पूर्णाङ्क d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_व्यञ्जकम्_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_व्यञ्जकम्_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      भङ्ग;
    }
    पक्ष TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_व्यञ्जकम्_If_True(pParse, p_व्यञ्जकम्->p_वामः, dest, jumpIfNull);
      भङ्ग;
    }
    पक्ष TK_TRUTH: {
      पूर्णाङ्क isNot;   /* IS NOT TRUE or IS NOT FALSE */
      पूर्णाङ्क isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_व्यञ्जकम्->op2==TK_ISNOT;
      isTrue = sqlite3_व्यञ्जकम्_Truth_मानम्(p_व्यञ्जकम्->p_दक्षिणः);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      जइ( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_व्यञ्जकम्_If_False(pParse, p_व्यञ्जकम्->p_वामः, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }अण्णहा{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_व्यञ्जकम्_If_True(pParse, p_व्यञ्जकम्->p_वामः, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      भङ्ग;
    }
    पक्ष TK_IS:
    पक्ष TK_ISNOT:
      testcase( p_व्यञ्जकम्->op==TK_IS );
      testcase( p_व्यञ्जकम्->op==TK_ISNOT );
      op = (p_व्यञ्जकम्->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    पक्ष TK_LT:
    पक्ष TK_LE:
    पक्ष TK_GT:
    पक्ष TK_GE:
    पक्ष TK_NE:
    पक्ष TK_EQ: {
      पूर्णाङ्क addrIsNull;
      जइ( sqlite3_व्यञ्जकम्_Is_Vector(p_व्यञ्जकम्->p_वामः) ) गच्छ default_व्यञ्जकम्;
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = व्यञ्जकम्_Compute_Operands(pParse, p_व्यञ्जकम्,
                                   &r1, &r2, &regFree1, &regFree2);
      }अण्णहा{
        r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
        r2 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_दक्षिणः, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_व्यञ्जकम्->p_वामः, p_व्यञ्जकम्->p_दक्षिणः, op,
                  r1, r2, dest, jumpIfNull,व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_Commuted));
      प्रतिज्ञा(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      प्रतिज्ञा(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      प्रतिज्ञा(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      प्रतिज्ञा(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      प्रतिज्ञा(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      प्रतिज्ञा(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      जइ( addrIsNull ){
        जइ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }अण्णहा{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      भङ्ग;
    }
    पक्ष TK_ISNULL:
    पक्ष TK_NOTNULL: {
      r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्->p_वामः, &regFree1);
      प्रतिज्ञा( regFree1==0 || regFree1==r1 );
      जइ( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      भङ्ग;
    }
    पक्ष TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      व्यञ्जकम्_Code_Between(pParse, p_व्यञ्जकम्, dest, sqlite3_व्यञ्जकम्_If_False, jumpIfNull);
      भङ्ग;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    पक्ष TK_IN: {
      जइ( jumpIfNull ){
        sqlite3_व्यञ्जकम्_Code_IN(pParse, p_व्यञ्जकम्, dest, dest);
      }अण्णहा{
        पूर्णाङ्क destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_व्यञ्जकम्_Code_IN(pParse, p_व्यञ्जकम्, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      भङ्ग;
    }
#endif
    स्वभाव: {
    default_व्यञ्जकम्:
      जइ( व्यञ्जकम्_Always_False(p_व्यञ्जकम्) ){
        sqlite3VdbeGoto(v, dest);
      }अण्णहा जइ( व्यञ्जकम्_Always_True(p_व्यञ्जकम्) ){
        /* no-op */
      }अण्णहा{
        r1 = sqlite3_व्यञ्जकम्_Code_Temp(pParse, p_व्यञ्जकम्, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      भङ्ग;
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
शून्य sqlite3_व्यञ्जकम्_If_False_Dup(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क dest,पूर्णाङ्क jumpIfNull){
  sqlite3 *db = pParse->db;
  व्यञ्जकम् *pCopy = sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्, 0);
  जइ( db->mallocFailed==0 ){
    sqlite3_व्यञ्जकम्_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_व्यञ्जकम्_Delete(db, pCopy);
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
स्थिर SQLITE_NOINLINE पूर्णाङ्क व्यञ्जकम्_Compare_Variable(
  ध्रुव Parse *pParse,
  ध्रुव व्यञ्जकम् *pVar,
  ध्रुव व्यञ्जकम् *p_व्यञ्जकम्
){
  पूर्णाङ्क res = 2;
  पूर्णाङ्क iVar;
  sqlite3_मानम् *pL, *pR = 0;
 
  जइ( p_व्यञ्जकम्->op==TK_VARIABLE && pVar->iColumn==p_व्यञ्जकम्->iColumn ){
    प्रत्यागम 0;
  }
  जइ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) प्रत्यागम 2;
  sqlite3_मानम्_From_व्यञ्जकम्(pParse->db, p_व्यञ्जकम्, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  जइ( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_मानम्(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    जइ( pL ){
      जइ( sqlite3_मानम्_type(pL)==SQLITE_TEXT ){
        sqlite3_मानम्_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_मानम्_Free(pR);
    sqlite3_मानम्_Free(pL);
  }
  प्रत्यागम res;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Compare(
  ध्रुव Parse *pParse,
  ध्रुव व्यञ्जकम् *pA,
  ध्रुव व्यञ्जकम् *pB,
  पूर्णाङ्क iTab
){
  u32 combinedFlags;
  जइ( pA==0 || pB==0 ){
    प्रत्यागम pB==pA ? 0 : 2;
  }
  जइ( pParse && pA->op==TK_VARIABLE ){
    प्रत्यागम व्यञ्जकम्_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  जइ( combinedFlags & EP_Int_मानम् ){
    जइ( (pA->flags&pB->flags&EP_Int_मानम्)!=0 && pA->u.i_मानम्==pB->u.i_मानम् ){
      प्रत्यागम 0;
    }
    प्रत्यागम 2;
  }
  जइ( pA->op!=pB->op || pA->op==TK_RAISE ){
    जइ( pA->op==TK_COLLATE && sqlite3_व्यञ्जकम्_Compare(pParse, pA->p_वामः,pB,iTab)<2 ){
      प्रत्यागम 1;
    }
    जइ( pB->op==TK_COLLATE && sqlite3_व्यञ्जकम्_Compare(pParse, pA,pB->p_वामः,iTab)<2 ){
      प्रत्यागम 1;
    }
    जइ( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }अण्णहा{
      प्रत्यागम 2;
    }
  }
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pA, EP_Int_मानम्) );
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pB, EP_Int_मानम्) );
  जइ( pA->u.zToken ){
    जइ( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      जइ( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) प्रत्यागम 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      प्रतिज्ञा( pA->op==pB->op );
      जइ( व्यञ्जकम्_Has_Property(pA,EP_WinFunc)!=व्यञ्जकम्_Has_Property(pB,EP_WinFunc) ){
        प्रत्यागम 2;
      }
      जइ( व्यञ्जकम्_Has_Property(pA,EP_WinFunc) ){
        जइ( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          प्रत्यागम 2;
        }
      }
#endif
    }अण्णहा जइ( pA->op==TK_NULL ){
      प्रत्यागम 0;
    }अण्णहा जइ( pA->op==TK_COLLATE ){
      जइ( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) प्रत्यागम 2;
    }अण्णहा
    जइ( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      प्रत्यागम 2;
    }
  }
  जइ( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) प्रत्यागम 2;
  जइ( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    जइ( combinedFlags & EP_xIsSelect ) प्रत्यागम 2;
    जइ( (combinedFlags & EP_FixedCol)==0
     && sqlite3_व्यञ्जकम्_Compare(pParse, pA->p_वामः, pB->p_वामः, iTab) ) प्रत्यागम 2;
    जइ( sqlite3_व्यञ्जकम्_Compare(pParse, pA->p_दक्षिणः, pB->p_दक्षिणः, iTab) ) प्रत्यागम 2;
    जइ( sqlite3_व्यञ्जकम्_List_Compare(pA->x.pList, pB->x.pList, iTab) ) प्रत्यागम 2;
    जइ( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      जइ( pA->iColumn!=pB->iColumn ) प्रत्यागम 2;
      जइ( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) प्रत्यागम 2;
      जइ( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        प्रत्यागम 2;
      }
    }
  }
  प्रत्यागम 0;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_List_Compare(ध्रुव व्यञ्जकम्_List *pA, ध्रुव व्यञ्जकम्_List *pB, पूर्णाङ्क iTab){
  पूर्णाङ्क i;
  जइ( pA==0 && pB==0 ) प्रत्यागम 0;
  जइ( pA==0 || pB==0 ) प्रत्यागम 1;
  जइ( pA->n_व्यञ्जकम्!=pB->n_व्यञ्जकम् ) प्रत्यागम 1;
  प्रति(i=0; i<pA->n_व्यञ्जकम्; i++){
    पूर्णाङ्क res;
    व्यञ्जकम् *p_व्यञ्जकम्_A = pA->a[i].p_व्यञ्जकम्;
    व्यञ्जकम् *p_व्यञ्जकम्_B = pB->a[i].p_व्यञ्जकम्;
    जइ( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) प्रत्यागम 1;
    जइ( (res = sqlite3_व्यञ्जकम्_Compare(0, p_व्यञ्जकम्_A, p_व्यञ्जकम्_B, iTab)) ) प्रत्यागम res;
  }
  प्रत्यागम 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Compare_Skip(व्यञ्जकम् *pA,व्यञ्जकम् *pB, पूर्णाङ्क iTab){
  प्रत्यागम sqlite3_व्यञ्जकम्_Compare(0,
             sqlite3_व्यञ्जकम्_Skip_Collate(pA),
             sqlite3_व्यञ्जकम्_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Implies_Not_Null(
  ध्रुव Parse *pParse,/* Parsing context */
  ध्रुव व्यञ्जकम् *p,      /* The expression to be checked */
  ध्रुव व्यञ्जकम् *pNN,    /* The expression that is NOT NULL */
  पूर्णाङ्क iTab,           /* Table being evaluated */
  पूर्णाङ्क seenNot         /* Return true only if p can be any non-NULL value */
){
  प्रतिज्ञा( p );
  प्रतिज्ञा( pNN );
  जइ( sqlite3_व्यञ्जकम्_Compare(pParse, p, pNN, iTab)==0 ){
    प्रत्यागम pNN->op!=TK_NULL;
  }
  विकल्प( p->op ){
    पक्ष TK_IN: {
      जइ( seenNot && व्यञ्जकम्_Has_Property(p, EP_xIsSelect) ) प्रत्यागम 0;
      प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_व्यञ्जकम्>0) );
      प्रत्यागम व्यञ्जकम्_Implies_Not_Null(pParse, p->p_वामः, pNN, iTab, 1);
    }
    पक्ष TK_BETWEEN: {
      व्यञ्जकम्_List *pList;
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p) );
      pList = p->x.pList;
      प्रतिज्ञा( pList!=0 );
      प्रतिज्ञा( pList->n_व्यञ्जकम्==2 );
      जइ( seenNot ) प्रत्यागम 0;
      जइ( व्यञ्जकम्_Implies_Not_Null(pParse, pList->a[0].p_व्यञ्जकम्, pNN, iTab, 1)
       || व्यञ्जकम्_Implies_Not_Null(pParse, pList->a[1].p_व्यञ्जकम्, pNN, iTab, 1)
      ){
        प्रत्यागम 1;
      }
      प्रत्यागम व्यञ्जकम्_Implies_Not_Null(pParse, p->p_वामः, pNN, iTab, 1);
    }
    पक्ष TK_EQ:
    पक्ष TK_NE:
    पक्ष TK_LT:
    पक्ष TK_LE:
    पक्ष TK_GT:
    पक्ष TK_GE:
    पक्ष TK_PLUS:
    पक्ष TK_MINUS:
    पक्ष TK_BITOR:
    पक्ष TK_LSHIFT:
    पक्ष TK_RSHIFT:
    पक्ष TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    पक्ष TK_STAR:
    पक्ष TK_REM:
    पक्ष TK_BITAND:
    पक्ष TK_SLASH: {
      जइ( व्यञ्जकम्_Implies_Not_Null(pParse, p->p_दक्षिणः, pNN, iTab, seenNot) ) प्रत्यागम 1;
      /* no break */ deliberate_fall_through
    }
    पक्ष TK_SPAN:
    पक्ष TK_COLLATE:
    पक्ष TK_UPLUS:
    पक्ष TK_UMINUS: {
      प्रत्यागम व्यञ्जकम्_Implies_Not_Null(pParse, p->p_वामः, pNN, iTab, seenNot);
    }
    पक्ष TK_TRUTH: {
      जइ( seenNot ) प्रत्यागम 0;
      जइ( p->op2!=TK_IS ) प्रत्यागम 0;
      प्रत्यागम व्यञ्जकम्_Implies_Not_Null(pParse, p->p_वामः, pNN, iTab, 1);
    }
    पक्ष TK_BITNOT:
    पक्ष TK_NOT: {
      प्रत्यागम व्यञ्जकम्_Implies_Not_Null(pParse, p->p_वामः, pNN, iTab, 1);
    }
  }
  प्रत्यागम 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
स्थिर पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Not_True(व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क v;
  जइ( p_व्यञ्जकम्->op==TK_NULL ) प्रत्यागम 1;
  जइ( p_व्यञ्जकम्->op==TK_TRUEFALSE && sqlite3_व्यञ्जकम्_Truth_मानम्(p_व्यञ्जकम्)==0 ) प्रत्यागम 1;
  v = 1;
  जइ( sqlite3_व्यञ्जकम्_Is_Integer(p_व्यञ्जकम्, &v, 0, 0) && v==0 ) प्रत्यागम 1;
  प्रत्यागम 0;
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
स्थिर पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_IIF(sqlite3 *db, ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  व्यञ्जकम्_List *pList;
  जइ( p_व्यञ्जकम्->op==TK_FUNCTION ){
    ध्रुव वर्ण *z = p_व्यञ्जकम्->u.zToken;
    FuncDef *pDef;
    जइ( (z[0]!='i' && z[0]!='I') ) प्रत्यागम 0;
    जइ( p_व्यञ्जकम्->x.pList==0 ) प्रत्यागम 0;
    pDef = sqlite3FindFunction(db, z, p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    जइ( pDef==0 ) प्रत्यागम 0;
#अण्णहा
    जइ( NEVER(pDef==0) ) प्रत्यागम 0;
#endif
    जइ( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) प्रत्यागम 0;
    जइ( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) प्रत्यागम 0;
  }अण्णहा जइ( p_व्यञ्जकम्->op==TK_CASE ){
    जइ( p_व्यञ्जकम्->p_वामः!=0 ) प्रत्यागम 0;
  }अण्णहा{
    प्रत्यागम 0;
  }
  pList = p_व्यञ्जकम्->x.pList;
  प्रतिज्ञा( pList!=0 );
  जइ( pList->n_व्यञ्जकम्==2 ) प्रत्यागम 1;
  जइ( pList->n_व्यञ्जकम्==3 && sqlite3_व्यञ्जकम्_Is_Not_True(pList->a[2].p_व्यञ्जकम्) ) प्रत्यागम 1;
  प्रत्यागम 0;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Implies_व्यञ्जकम्(
  ध्रुव Parse *pParse,
  ध्रुव व्यञ्जकम् *pE1,
  ध्रुव व्यञ्जकम् *pE2,
  पूर्णाङ्क iTab
){
  जइ( sqlite3_व्यञ्जकम्_Compare(pParse, pE1, pE2, iTab)==0 ){
    प्रत्यागम 1;
  }
  जइ( pE2->op==TK_OR
   && (sqlite3_व्यञ्जकम्_Implies_व्यञ्जकम्(pParse, pE1, pE2->p_वामः, iTab)
             || sqlite3_व्यञ्जकम्_Implies_व्यञ्जकम्(pParse, pE1, pE2->p_दक्षिणः, iTab) )
  ){
    प्रत्यागम 1;
  }
  जइ( pE2->op==TK_NOTNULL
   && व्यञ्जकम्_Implies_Not_Null(pParse, pE1, pE2->p_वामः, iTab, 0)
  ){
    प्रत्यागम 1;
  }
  जइ( sqlite3_व्यञ्जकम्_Is_IIF(pParse->db, pE1) ){
    प्रत्यागम sqlite3_व्यञ्जकम्_Implies_व्यञ्जकम्(pParse,pE1->x.pList->a[0].p_व्यञ्जकम्,pE2,iTab);
  }
  प्रत्यागम 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
स्थिर शून्य bothImplyNotNullRow(Walker *pWalker, व्यञ्जकम् *pE1, व्यञ्जकम् *pE2){
  जइ( pWalker->eCode==0 ){
    sqlite3_Walk_व्यञ्जकम्(pWalker, pE1);
    जइ( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_व्यञ्जकम्(pWalker, pE2);
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
स्थिर पूर्णाङ्क impliesNotNullRow(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  testcase( p_व्यञ्जकम्->op==TK_AGG_COLUMN );
  testcase( p_व्यञ्जकम्->op==TK_AGG_FUNCTION );
  जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) ) प्रत्यागम WRC_Prune;
  जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    प्रत्यागम WRC_Prune;
  }
  विकल्प( p_व्यञ्जकम्->op ){
    पक्ष TK_ISNOT:
    पक्ष TK_ISNULL:
    पक्ष TK_NOTNULL:
    पक्ष TK_IS:
    पक्ष TK_VECTOR:
    पक्ष TK_FUNCTION:
    पक्ष TK_TRUTH:
    पक्ष TK_CASE:
      testcase( p_व्यञ्जकम्->op==TK_ISNOT );
      testcase( p_व्यञ्जकम्->op==TK_ISNULL );
      testcase( p_व्यञ्जकम्->op==TK_NOTNULL );
      testcase( p_व्यञ्जकम्->op==TK_IS );
      testcase( p_व्यञ्जकम्->op==TK_VECTOR );
      testcase( p_व्यञ्जकम्->op==TK_FUNCTION );
      testcase( p_व्यञ्जकम्->op==TK_TRUTH );
      testcase( p_व्यञ्जकम्->op==TK_CASE );
      प्रत्यागम WRC_Prune;

    पक्ष TK_COLUMN:
      जइ( pWalker->u.iCur==p_व्यञ्जकम्->iTable ){
        pWalker->eCode = 1;
        प्रत्यागम WRC_Abort;
      }
      प्रत्यागम WRC_Prune;

    पक्ष TK_OR:
    पक्ष TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_व्यञ्जकम्->op==TK_OR );
      testcase( p_व्यञ्जकम्->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_व्यञ्जकम्->p_वामः, p_व्यञ्जकम्->p_दक्षिणः);
      प्रत्यागम WRC_Prune;
       
    पक्ष TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      जइ( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) && ALWAYS(p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्>0) ){
        sqlite3_Walk_व्यञ्जकम्(pWalker, p_व्यञ्जकम्->p_वामः);
      }
      प्रत्यागम WRC_Prune;

    पक्ष TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
      प्रतिज्ञा( p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्==2 );
      sqlite3_Walk_व्यञ्जकम्(pWalker, p_व्यञ्जकम्->p_वामः);
      bothImplyNotNullRow(pWalker, p_व्यञ्जकम्->x.pList->a[0].p_व्यञ्जकम्,
                                   p_व्यञ्जकम्->x.pList->a[1].p_व्यञ्जकम्);
      प्रत्यागम WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    पक्ष TK_EQ:
    पक्ष TK_NE:
    पक्ष TK_LT:
    पक्ष TK_LE:
    पक्ष TK_GT:
    पक्ष TK_GE: {
      व्यञ्जकम् *p_वामः = p_व्यञ्जकम्->p_वामः;
      व्यञ्जकम् *p_दक्षिणः = p_व्यञ्जकम्->p_दक्षिणः;
      testcase( p_व्यञ्जकम्->op==TK_EQ );
      testcase( p_व्यञ्जकम्->op==TK_NE );
      testcase( p_व्यञ्जकम्->op==TK_LT );
      testcase( p_व्यञ्जकम्->op==TK_LE );
      testcase( p_व्यञ्जकम्->op==TK_GT );
      testcase( p_व्यञ्जकम्->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      प्रतिज्ञा( p_वामः->op!=TK_COLUMN || व्यञ्जकम्_Use_YTab(p_वामः) );
      प्रतिज्ञा( p_दक्षिणः->op!=TK_COLUMN || व्यञ्जकम्_Use_YTab(p_दक्षिणः) );
      जइ( (p_वामः->op==TK_COLUMN
           && ALWAYS(p_वामः->y.pTab!=0)
           && IsVirtual(p_वामः->y.pTab))
       || (p_दक्षिणः->op==TK_COLUMN
           && ALWAYS(p_दक्षिणः->y.pTab!=0)
           && IsVirtual(p_दक्षिणः->y.pTab))
      ){
        प्रत्यागम WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    स्वभाव:
      प्रत्यागम WRC_Continue;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Implies_Non_Null_Row(व्यञ्जकम् *p, पूर्णाङ्क iTab, पूर्णाङ्क isRJ){
  Walker w;
  p = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p);
  जइ( p==0 ) प्रत्यागम 0;
  जइ( p->op==TK_NOTNULL ){
    p = p->p_वामः;
  }अण्णहा{
    जाव( p->op==TK_AND ){
      जइ( sqlite3_व्यञ्जकम्_Implies_Non_Null_Row(p->p_वामः, iTab, isRJ) ) प्रत्यागम 1;
      p = p->p_दक्षिणः;
    }
  }
  w.x_व्यञ्जकम्_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_व्यञ्जकम्(&w, p);
  प्रत्यागम w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
संरचना IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  पूर्णाङ्क iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Idx_Cover(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  जइ( p_व्यञ्जकम्->op==TK_COLUMN
   && p_व्यञ्जकम्->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_व्यञ्जकम्->iColumn)<0
  ){
    pWalker->eCode = 1;
    प्रत्यागम WRC_Abort;
  }
  प्रत्यागम WRC_Continue;
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Covered_By_Index(
  व्यञ्जकम् *p_व्यञ्जकम्,        /* The index to be tested */
  पूर्णाङ्क iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  संरचना IdxCover xcov;
  memset(&w, 0, परिमाण(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_व्यञ्जकम्_Callback = व्यञ्जकम्_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_व्यञ्जकम्(&w, p_व्यञ्जकम्);
  प्रत्यागम !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
संरचना RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  पूर्णाङ्क *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
स्थिर पूर्णाङ्क selectRefEnter(Walker *pWalker, Select *pSelect){
  संरचना RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  पूर्णाङ्क *piNew;
  जइ( pSrc->nSrc==0 ) प्रत्यागम WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*परिमाण(पूर्णाङ्क));
  जइ( piNew==0 ){
    p->nExclude = 0;
    प्रत्यागम WRC_Abort;
  }अण्णहा{
    p->aiExclude = piNew;
  }
  प्रति(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  प्रत्यागम WRC_Continue;
}
स्थिर शून्य selectRefLeave(Walker *pWalker, Select *pSelect){
  संरचना RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  जइ( p->nExclude ){
    प्रतिज्ञा( p->nExclude>=pSrc->nSrc );
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
स्थिर पूर्णाङ्क व्यञ्जकम्_Ref_To_Src_List(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  जइ( p_व्यञ्जकम्->op==TK_COLUMN
   || p_व्यञ्जकम्->op==TK_AGG_COLUMN
  ){
    पूर्णाङ्क i;
    संरचना RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    पूर्णाङ्क nSrc = pSrc ? pSrc->nSrc : 0;
    प्रति(i=0; i<nSrc; i++){
      जइ( p_व्यञ्जकम्->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        प्रत्यागम WRC_Continue;
      }
    }
    प्रति(i=0; i<p->nExclude && p->aiExclude[i]!=p_व्यञ्जकम्->iTable; i++){}
    जइ( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  प्रत्यागम WRC_Continue;
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
पूर्णाङ्क sqlite3ReferencesSrcList(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, SrcList *pSrcList){
  Walker w;
  संरचना RefSrcList x;
  प्रतिज्ञा( pParse->db!=0 );
  memset(&w, 0, परिमाण(w));
  memset(&x, 0, परिमाण(x));
  w.x_व्यञ्जकम्_Callback = व्यञ्जकम्_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_AGG_FUNCTION );
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
  sqlite3_Walk_व्यञ्जकम्_List(&w, p_व्यञ्जकम्->x.pList);
  जइ( p_व्यञ्जकम्->p_वामः ){
    प्रतिज्ञा( p_व्यञ्जकम्->p_वामः->op==TK_ORDER );
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्->p_वामः) );
    प्रतिज्ञा( p_व्यञ्जकम्->p_वामः->x.pList!=0 );
    sqlite3_Walk_व्यञ्जकम्_List(&w, p_व्यञ्जकम्->p_वामः->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) ){
    sqlite3_Walk_व्यञ्जकम्(&w, p_व्यञ्जकम्->y.pWin->pFilter);
  }
#endif
  जइ( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  जइ( w.eCode & 0x01 ){
    प्रत्यागम 1;
  }अण्णहा जइ( w.eCode ){
    प्रत्यागम 0;
  }अण्णहा{
    प्रत्यागम -1;
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
स्थिर पूर्णाङ्क agginfo_Persist_व्यञ्जकम्_Cb(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  जइ( ALWAYS(!व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_TokenOnly|EP_Reduced))
   && p_व्यञ्जकम्->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_व्यञ्जकम्->pAggInfo;
    पूर्णाङ्क iAgg = p_व्यञ्जकम्->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    प्रतिज्ञा( iAgg>=0 );
    जइ( p_व्यञ्जकम्->op!=TK_AGG_FUNCTION ){
      जइ( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_व्यञ्जकम्
      ){
        p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्, 0);
        जइ( p_व्यञ्जकम् && !sqlite3_व्यञ्जकम्_Deferred_Delete(pParse, p_व्यञ्जकम्) ){
          pAggInfo->aCol[iAgg].pCExpr = p_व्यञ्जकम्;
        }
      }
    }अण्णहा{
      प्रतिज्ञा( p_व्यञ्जकम्->op==TK_AGG_FUNCTION );
      जइ( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_व्यञ्जकम्
      ){
        p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्, 0);
        जइ( p_व्यञ्जकम् && !sqlite3_व्यञ्जकम्_Deferred_Delete(pParse, p_व्यञ्जकम्) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_व्यञ्जकम्;
        }
      }
    }
  }
  प्रत्यागम WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
शून्य sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, परिमाण(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_व्यञ्जकम्_Callback = agginfo_Persist_व्यञ्जकम्_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
स्थिर पूर्णाङ्क addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  पूर्णाङ्क i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       परिमाण(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  प्रत्यागम i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
स्थिर पूर्णाङ्क addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  पूर्णाङ्क i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       परिमाण(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  प्रत्यागम i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
स्थिर शून्य findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  व्यञ्जकम् *p_व्यञ्जकम्          /* Expr describing the column to find or insert */
){
  संरचना AggInfo_col *pCol;
  पूर्णाङ्क k;
  पूर्णाङ्क mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  प्रतिज्ञा( mxTerm <= SMXV(i16) );
  प्रतिज्ञा( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  प्रति(k=0; k<pAggInfo->nColumn; k++, pCol++){
    जइ( pCol->pCExpr==p_व्यञ्जकम् ) प्रत्यागम;
    जइ( pCol->iTable==p_व्यञ्जकम्->iTable
     && pCol->iColumn==p_व्यञ्जकम्->iColumn
     && p_व्यञ्जकम्->op!=TK_IF_NULL_ROW
    ){
      गच्छ fix_up_व्यञ्जकम्;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  जइ( k<0 ){
    /* OOM on resize */
    प्रतिज्ञा( pParse->db->mallocFailed );
    प्रत्यागम;
  }
  जइ( k>mxTerm ){
    sqlite3_Error_सन्देशः(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) );
  pCol->pTab = p_व्यञ्जकम्->y.pTab;
  pCol->iTable = p_व्यञ्जकम्->iTable;
  pCol->iColumn = p_व्यञ्जकम्->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_व्यञ्जकम्;
  जइ( pAggInfo->pGroupBy && p_व्यञ्जकम्->op!=TK_IF_NULL_ROW ){
    पूर्णाङ्क j, n;
    व्यञ्जकम्_List *pGB = pAggInfo->pGroupBy;
    संरचना व्यञ्जकम्_List_item *pTerm = pGB->a;
    n = pGB->n_व्यञ्जकम्;
    प्रति(j=0; j<n; j++, pTerm++){
      व्यञ्जकम् *pE = pTerm->p_व्यञ्जकम्;
      जइ( pE->op==TK_COLUMN
       && pE->iTable==p_व्यञ्जकम्->iTable
       && pE->iColumn==p_व्यञ्जकम्->iColumn
      ){
        pCol->iSorterColumn = j;
        भङ्ग;
      }
    }
  }
  जइ( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_व्यञ्जकम्:
  व्यञ्जकम्_Set_VVAProperty(p_व्यञ्जकम्, EP_NoReduce);
  प्रतिज्ञा( p_व्यञ्जकम्->pAggInfo==0 || p_व्यञ्जकम्->pAggInfo==pAggInfo );
  p_व्यञ्जकम्->pAggInfo = pAggInfo;
  जइ( p_व्यञ्जकम्->op==TK_COLUMN ){
    p_व्यञ्जकम्->op = TK_AGG_COLUMN;
  }
  प्रतिज्ञा( k <= SMXV(p_व्यञ्जकम्->iAgg) );
  p_व्यञ्जकम्->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
स्थिर पूर्णाङ्क analyzeAggregate(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  प्रतिज्ञा( pNC->ncFlags & NC_UAggInfo );
  प्रतिज्ञा( pAggInfo->iFirstReg==0 );
  विकल्प( p_व्यञ्जकम्->op ){
    स्वभाव: {
      Indexed_व्यञ्जकम् *pIEpr;
      व्यञ्जकम् tmp;
      प्रतिज्ञा( pParse->iSelfTab==0 );
      जइ( (pNC->ncFlags & NC_InAggFunc)==0 ) भङ्ग;
      जइ( pParse->pIdxEpr==0 ) भङ्ग;
      प्रति(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        पूर्णाङ्क iDataCur = pIEpr->iDataCur;
        जइ( iDataCur<0 ) अनुवर्त;
        जइ( sqlite3_व्यञ्जकम्_Compare(0, p_व्यञ्जकम्, pIEpr->p_व्यञ्जकम्, iDataCur)==0 ) भङ्ग;
      }
      जइ( pIEpr==0 ) भङ्ग;
      जइ( NEVER(!व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्)) ) भङ्ग;
      प्रति(i=0; i<pSrcList->nSrc; i++){
         जइ( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           भङ्ग;
         }
      }
      जइ( i>=pSrcList->nSrc ) भङ्ग;
      जइ( NEVER(p_व्यञ्जकम्->pAggInfo!=0) ) भङ्ग; /* Resolved by outer context */
      जइ( pParse->nErr ){ प्रत्यागम WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, परिमाण(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      जइ( pParse->nErr ){ प्रत्यागम WRC_Abort; }
      प्रतिज्ञा( pAggInfo->aCol!=0 );
      प्रतिज्ञा( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_व्यञ्जकम्;
      p_व्यञ्जकम्->pAggInfo = pAggInfo;
      p_व्यञ्जकम्->iAgg = tmp.iAgg;
      प्रत्यागम WRC_Prune;
    }
    पक्ष TK_IF_NULL_ROW:
    पक्ष TK_AGG_COLUMN:
    पक्ष TK_COLUMN: {
      testcase( p_व्यञ्जकम्->op==TK_AGG_COLUMN );
      testcase( p_व्यञ्जकम्->op==TK_COLUMN );
      testcase( p_व्यञ्जकम्->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      जइ( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        प्रति(i=0; i<pSrcList->nSrc; i++, pItem++){
          प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_TokenOnly|EP_Reduced) );
          जइ( p_व्यञ्जकम्->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_व्यञ्जकम्);
            भङ्ग;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      प्रत्यागम WRC_Continue;
    }
    पक्ष TK_AGG_FUNCTION: {
      जइ( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_व्यञ्जकम्->op2
       && p_व्यञ्जकम्->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        संरचना AggInfo_func *pItem = pAggInfo->aFunc;
        पूर्णाङ्क mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        प्रतिज्ञा( mxTerm <= SMXV(i16) );
        प्रति(i=0; i<pAggInfo->nFunc; i++, pItem++){
          जइ( NEVER(pItem->pFExpr==p_व्यञ्जकम्) ) भङ्ग;
          जइ( sqlite3_व्यञ्जकम्_Compare(0, pItem->pFExpr, p_व्यञ्जकम्, -1)==0 ){
            भङ्ग;
          }
        }
        जइ( i>mxTerm ){
          sqlite3_Error_सन्देशः(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          प्रतिज्ञा( i<pAggInfo->nFunc );
        }अण्णहा जइ( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          जइ( i>=0 ){
            पूर्णाङ्क nArg;
            प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_व्यञ्जकम्;
            प्रतिज्ञा( व्यञ्जकम्_Use_UToken(p_व्यञ्जकम्) );
            nArg = p_व्यञ्जकम्->x.pList ? p_व्यञ्जकम्->x.pList->n_व्यञ्जकम् : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_व्यञ्जकम्->u.zToken, nArg, enc, 0);
            प्रतिज्ञा( pItem->bOBUnique==0 );
            जइ( p_व्यञ्जकम्->p_वामः
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              व्यञ्जकम्_List *pOBList;
              प्रतिज्ञा( nArg>0 );
              प्रतिज्ञा( p_व्यञ्जकम्->p_वामः->op==TK_ORDER );
              प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्->p_वामः) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_व्यञ्जकम्->p_वामः->x.pList;
              प्रतिज्ञा( pOBList->n_व्यञ्जकम्>0 );
              प्रतिज्ञा( pItem->bOBUnique==0 );
              जइ( pOBList->n_व्यञ्जकम्==1
               && nArg==1
               && sqlite3_व्यञ्जकम्_Compare(0,pOBList->a[0].p_व्यञ्जकम्,
                               p_व्यञ्जकम्->x.pList->a[0].p_व्यञ्जकम्,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Distinct);
              }अण्णहा{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }अण्णहा{
              pItem->iOBTab = -1;
            }
            जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }अण्णहा{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_TokenOnly|EP_Reduced) );
        व्यञ्जकम्_Set_VVAProperty(p_व्यञ्जकम्, EP_NoReduce);
        प्रतिज्ञा( i <= SMXV(p_व्यञ्जकम्->iAgg) );
        p_व्यञ्जकम्->iAgg = (i16)i;
        p_व्यञ्जकम्->pAggInfo = pAggInfo;
        प्रत्यागम WRC_Prune;
      }अण्णहा{
        प्रत्यागम WRC_Continue;
      }
    }
  }
  प्रत्यागम WRC_Continue;
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
शून्य sqlite3_व्यञ्जकम्_Analyze_Aggregates(NameContext *pNC, व्यञ्जकम् *p_व्यञ्जकम्){
  Walker w;
  w.x_व्यञ्जकम्_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  प्रतिज्ञा( pNC->pSrcList!=0 );
  sqlite3_Walk_व्यञ्जकम्(&w, p_व्यञ्जकम्);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
शून्य sqlite3_व्यञ्जकम्_Analyze_Agg_List(NameContext *pNC, व्यञ्जकम्_List *pList){
  संरचना व्यञ्जकम्_List_item *pItem;
  पूर्णाङ्क i;
  जइ( pList ){
    प्रति(pItem=pList->a, i=0; i<pList->n_व्यञ्जकम्; i++, pItem++){
      sqlite3_व्यञ्जकम्_Analyze_Aggregates(pNC, pItem->p_व्यञ्जकम्);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
पूर्णाङ्क sqlite3GetTempReg(Parse *pParse){
  जइ( pParse->nTempReg==0 ){
    प्रत्यागम ++pParse->nMem;
  }
  प्रत्यागम pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
शून्य sqlite3ReleaseTempReg(Parse *pParse, पूर्णाङ्क iReg){
  जइ( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    पूर्णाङ्क kk;
    प्रति(kk=0; kk<pParse->nTempReg; kk++){
      प्रतिज्ञा( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    जइ( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
पूर्णाङ्क sqlite3GetTempRange(Parse *pParse, पूर्णाङ्क nReg){
  पूर्णाङ्क i, n;
  जइ( nReg==1 ) प्रत्यागम sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  जइ( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }अण्णहा{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  प्रत्यागम i;
}
शून्य sqlite3ReleaseTempRange(Parse *pParse, पूर्णाङ्क iReg, पूर्णाङ्क nReg){
  जइ( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    प्रत्यागम;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  जइ( nReg>pParse->nRangeReg ){
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
शून्य sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
शून्य sqlite3TouchRegister(Parse *pParse, पूर्णाङ्क iReg){
  जइ( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#जइ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
पूर्णाङ्क sqlite3FirstAvailableRegister(Parse *pParse, पूर्णाङ्क iMin){
  ध्रुव व्यञ्जकम्_List *pList = pParse->p_Const_व्यञ्जकम्;
  जइ( pList ){
    पूर्णाङ्क i;
    प्रति(i=0; i<pList->n_व्यञ्जकम्; i++){
      जइ( pList->a[i].u.i_Const_व्यञ्जकम्_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_व्यञ्जकम्_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  प्रत्यागम iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
पूर्णाङ्क sqlite3NoTempsInRange(Parse *pParse, पूर्णाङ्क iFirst, पूर्णाङ्क iLast){
  पूर्णाङ्क i;
  जइ( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     प्रत्यागम 0;
  }
  प्रति(i=0; i<pParse->nTempReg; i++){
    जइ( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      प्रत्यागम 0;
    }
  }
  जइ( pParse->p_Const_व्यञ्जकम् ){
    व्यञ्जकम्_List *pList = pParse->p_Const_व्यञ्जकम्;
    प्रति(i=0; i<pList->n_व्यञ्जकम्; i++){
      पूर्णाङ्क iReg = pList->a[i].u.i_Const_व्यञ्जकम्_Reg;
      जइ( iReg==0 ) अनुवर्त;
      जइ( iReg>=iFirst && iReg<=iLast ) प्रत्यागम 0;
    }
  }
  प्रत्यागम 1;
}
#endif /* SQLITE_DEBUG */
