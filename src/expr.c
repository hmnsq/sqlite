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
#समावेश "sqliteInt.h"

/* Forward declarations */
अटल व्योम व्यंजक_Code_Between(Parse*,व्यंजक*,पूर्णांक,व्योम(*)(Parse*,व्यंजक*,पूर्णांक,पूर्णांक),पूर्णांक);
अटल पूर्णांक व्यंजक_Code_Vector(Parse *pParse, व्यंजक *p, पूर्णांक *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
अक्षर sqlite3TableColumnAffinity(स्थिर Table *pTab, पूर्णांक iCol){
  अगर( iCol<0 || NEVER(iCol>=pTab->nCol) ) वापस SQLITE_AFF_INTEGER;
  वापस pTab->aCol[iCol].affinity;
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
अक्षर sqlite3_व्यंजक_Affinity(स्थिर व्यंजक *p_व्यंजक){
  पूर्णांक op;
  op = p_व्यंजक->op;
  जबतक( 1 /* exit-by-break */ ){
    अगर( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_व्यंजक->y.pTab!=0) ){
      निश्चित( व्यंजक_Use_YTab(p_व्यंजक) );
      निश्चित( p_व्यंजक->y.pTab!=0 );
      वापस sqlite3TableColumnAffinity(p_व्यंजक->y.pTab, p_व्यंजक->iColumn);
    }
    अगर( op==TK_SELECT ){
      निश्चित( व्यंजक_Use_XSelect(p_व्यंजक) );
      निश्चित( p_व्यंजक->x.pSelect!=0 );
      निश्चित( p_व्यंजक->x.pSelect->pEList!=0 );
      निश्चित( p_व्यंजक->x.pSelect->pEList->a[0].p_व्यंजक!=0 );
      वापस sqlite3_व्यंजक_Affinity(p_व्यंजक->x.pSelect->pEList->a[0].p_व्यंजक);
    }
#अगर_अघोषित SQLITE_OMIT_CAST
    अगर( op==TK_CAST ){
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      वापस sqlite3AffinityType(p_व्यंजक->u.zToken, 0);
    }
#पूर्ण_अगर
    अगर( op==TK_SELECT_COLUMN ){
      निश्चित( p_व्यंजक->p_बायाँ!=0 && व्यंजक_Use_XSelect(p_व्यंजक->p_बायाँ) );
      निश्चित( p_व्यंजक->iColumn < p_व्यंजक->iTable );
      निश्चित( p_व्यंजक->iColumn >= 0 );
      निश्चित( p_व्यंजक->iTable==p_व्यंजक->p_बायाँ->x.pSelect->pEList->n_व्यंजक );
      वापस sqlite3_व्यंजक_Affinity(
          p_व्यंजक->p_बायाँ->x.pSelect->pEList->a[p_व्यंजक->iColumn].p_व्यंजक
      );
    }
    अगर( op==TK_VECTOR
     || (op==TK_FUNCTION && p_व्यंजक->aff_व्यंजक==SQLITE_AFF_DEFER)
    ){
      निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
      वापस sqlite3_व्यंजक_Affinity(p_व्यंजक->x.pList->a[0].p_व्यंजक);
    }
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Skip|EP_IfNullRow) ){
      निश्चित( p_व्यंजक->op==TK_COLLATE
           || p_व्यंजक->op==TK_IF_NULL_ROW
           || (p_व्यंजक->op==TK_REGISTER && p_व्यंजक->op2==TK_IF_NULL_ROW) );
      p_व्यंजक = p_व्यंजक->p_बायाँ;
      op = p_व्यंजक->op;
      जारी;
    }
    अगर( op!=TK_REGISTER ) अवरोध;
    op = p_व्यंजक->op2;
    अगर( NEVER( op==TK_REGISTER ) ) अवरोध;
  }
  वापस p_व्यंजक->aff_व्यंजक;
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
पूर्णांक sqlite3_व्यंजक_तथ्य_Type(स्थिर व्यंजक *p_व्यंजक){
  जबतक( p_व्यंजक ){
    चयन( p_व्यंजक->op ){
      हाल TK_COLLATE:
      हाल TK_IF_NULL_ROW:
      हाल TK_UPLUS:  {
        p_व्यंजक = p_व्यंजक->p_बायाँ;
        अवरोध;
      }
      हाल TK_NULL: {
        p_व्यंजक = 0;
        अवरोध;
      }
      हाल TK_STRING: {
        वापस 0x02;
      }
      हाल TK_BLOB: {
        वापस 0x04;
      }
      हाल TK_CONCAT: {
        वापस 0x06;
      }
      हाल TK_VARIABLE:
      हाल TK_AGG_FUNCTION:
      हाल TK_FUNCTION: {
        वापस 0x07;
      }
      हाल TK_COLUMN:
      हाल TK_AGG_COLUMN:
      हाल TK_SELECT:
      हाल TK_CAST:
      हाल TK_SELECT_COLUMN:
      हाल TK_VECTOR:  {
        पूर्णांक aff = sqlite3_व्यंजक_Affinity(p_व्यंजक);
        अगर( aff>=SQLITE_AFF_NUMERIC ) वापस 0x05;
        अगर( aff==SQLITE_AFF_TEXT )    वापस 0x06;
        वापस 0x07;
      }
      हाल TK_CASE: {
        पूर्णांक res = 0;
        पूर्णांक ii;
        व्यंजक_List *pList = p_व्यंजक->x.pList;
        निश्चित( व्यंजक_Use_XList(p_व्यंजक) && pList!=0 );
        निश्चित( pList->n_व्यंजक > 0);
        क्रम(ii=1; ii<pList->n_व्यंजक; ii+=2){
          res |= sqlite3_व्यंजक_तथ्य_Type(pList->a[ii].p_व्यंजक);
        }
        अगर( pList->n_व्यंजक % 2 ){
          res |= sqlite3_व्यंजक_तथ्य_Type(pList->a[pList->n_व्यंजक-1].p_व्यंजक);
        }
        वापस res;
      }
      शेष: {
        वापस 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  वापस 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
व्यंजक *sqlite3_व्यंजक_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  व्यंजक *p_व्यंजक,             /* Add the "COLLATE" clause to this expression */
  स्थिर Token *pCollName,  /* Name of collating sequence */
  पूर्णांक dequote              /* True to dequote pCollName */
){
  अगर( pCollName->n>0 && pParse->nErr==0 ){
    व्यंजक *pNew = sqlite3_व्यंजक_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    निश्चित( p_व्यंजक!=0 );
    अगर( pNew ){
      pNew->p_बायाँ = p_व्यंजक;
      pNew->flags |= EP_Collate|EP_Skip;
#अगर SQLITE_MAX_व्यंजक_DEPTH>0
      pNew->nHeight = p_व्यंजक->nHeight+1;
      sqlite3_व्यंजक_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#पूर्ण_अगर
      p_व्यंजक = pNew;
    }
  }
  वापस p_व्यंजक;
}
व्यंजक *sqlite3_व्यंजक_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  व्यंजक *p_व्यंजक,          /* Add the "COLLATE" clause to this expression */
  स्थिर अक्षर *zC        /* The collating sequence name */
){
  Token s;
  निश्चित( zC!=0 );
  sqlite3TokenInit(&s, (अक्षर*)zC);
  वापस sqlite3_व्यंजक_Add_Collate_Token(pParse, p_व्यंजक, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
व्यंजक *sqlite3_व्यंजक_Skip_Collate(व्यंजक *p_व्यंजक){
  जबतक( p_व्यंजक && व्यंजक_Has_Property(p_व्यंजक, EP_Skip) ){
    निश्चित( p_व्यंजक->op==TK_COLLATE );
    p_व्यंजक = p_व्यंजक->p_बायाँ;
  }  
  वापस p_व्यंजक;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
व्यंजक *sqlite3_व्यंजक_Skip_Collate_And_Likely(व्यंजक *p_व्यंजक){
  जबतक( p_व्यंजक && व्यंजक_Has_Property(p_व्यंजक, EP_Skip|EP_Unlikely) ){
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Unlikely) ){
      निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
      निश्चित( p_व्यंजक->x.pList->n_व्यंजक>0 );
      निश्चित( p_व्यंजक->op==TK_FUNCTION );
      p_व्यंजक = p_व्यंजक->x.pList->a[0].p_व्यंजक;
    }अन्यथा अगर( p_व्यंजक->op==TK_COLLATE ){
      p_व्यंजक = p_व्यंजक->p_बायाँ;
    }अन्यथा{
      अवरोध;
    }
  }  
  वापस p_व्यंजक;
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
CollSeq *sqlite3_व्यंजक_Coll_Seq(Parse *pParse, स्थिर व्यंजक *p_व्यंजक){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  स्थिर व्यंजक *p = p_व्यंजक;
  जबतक( p ){
    पूर्णांक op = p->op;
    अगर( op==TK_REGISTER ) op = p->op2;
    अगर( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      पूर्णांक j;
      निश्चित( व्यंजक_Use_YTab(p) );
      निश्चित( p->y.pTab!=0 );
      अगर( (j = p->iColumn)>=0 ){
        स्थिर अक्षर *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      अवरोध;
    }
    अगर( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_बायाँ;
      जारी;
    }
    अगर( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_व्यंजक==SQLITE_AFF_DEFER)
    ){
      निश्चित( व्यंजक_Use_XList(p) );
      p = p->x.pList->a[0].p_व्यंजक;
      जारी;
    }
    अगर( op==TK_COLLATE ){
      निश्चित( !व्यंजक_Has_Property(p, EP_Int_मूल्य) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      अवरोध;
    }
    अगर( p->flags & EP_Collate ){
      अगर( p->p_बायाँ && (p->p_बायाँ->flags & EP_Collate)!=0 ){
        p = p->p_बायाँ;
      }अन्यथा{
        व्यंजक *p_अग्र  = p->p_दायाँ;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        निश्चित( !व्यंजक_Use_XList(p) || p->x.pList==0 || p->p_दायाँ==0 );
        अगर( व्यंजक_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          पूर्णांक i;
          क्रम(i=0; i<p->x.pList->n_व्यंजक; i++){
            अगर( व्यंजक_Has_Property(p->x.pList->a[i].p_व्यंजक, EP_Collate) ){
              p_अग्र = p->x.pList->a[i].p_व्यंजक;
              अवरोध;
            }
          }
        }
        p = p_अग्र;
      }
    }अन्यथा{
      अवरोध;
    }
  }
  अगर( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  वापस pColl;
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
CollSeq *sqlite3_व्यंजक_NNColl_Seq(Parse *pParse, स्थिर व्यंजक *p_व्यंजक){
  CollSeq *p = sqlite3_व्यंजक_Coll_Seq(pParse, p_व्यंजक);
  अगर( p==0 ) p = pParse->db->pDfltColl;
  निश्चित( p!=0 );
  वापस p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
पूर्णांक sqlite3_व्यंजक_Coll_Seq_Match(Parse *pParse, स्थिर व्यंजक *pE1, स्थिर व्यंजक *pE2){
  CollSeq *pColl1 = sqlite3_व्यंजक_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_व्यंजक_NNColl_Seq(pParse, pE2);
  निश्चित( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  वापस pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
अक्षर sqlite3CompareAffinity(स्थिर व्यंजक *p_व्यंजक, अक्षर aff2){
  अक्षर aff1 = sqlite3_व्यंजक_Affinity(p_व्यंजक);
  अगर( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    अगर( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      वापस SQLITE_AFF_NUMERIC;
    }अन्यथा{
      वापस SQLITE_AFF_BLOB;
    }
  }अन्यथा{
    /* One side is a column, the other is not. Use the columns affinity. */
    निश्चित( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    वापस (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
अटल अक्षर comparisonAffinity(स्थिर व्यंजक *p_व्यंजक){
  अक्षर aff;
  निश्चित( p_व्यंजक->op==TK_EQ || p_व्यंजक->op==TK_IN || p_व्यंजक->op==TK_LT ||
          p_व्यंजक->op==TK_GT || p_व्यंजक->op==TK_GE || p_व्यंजक->op==TK_LE ||
          p_व्यंजक->op==TK_NE || p_व्यंजक->op==TK_IS || p_व्यंजक->op==TK_ISNOT );
  निश्चित( p_व्यंजक->p_बायाँ );
  निश्चित( !व्यंजक_Use_XSelect(p_व्यंजक) );
  aff = sqlite3_व्यंजक_Affinity(p_व्यंजक->p_बायाँ);
  अगर( p_व्यंजक->p_दायाँ ){
    aff = sqlite3CompareAffinity(p_व्यंजक->p_दायाँ, aff);
  }अन्यथा अगर( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  वापस aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
पूर्णांक sqlite3_सूचकांक_Affinity_Ok(स्थिर व्यंजक *p_व्यंजक, अक्षर idx_affinity){
  अक्षर aff = comparisonAffinity(p_व्यंजक);
  अगर( aff<SQLITE_AFF_TEXT ){
    वापस 1;
  }
  अगर( aff==SQLITE_AFF_TEXT ){
    वापस idx_affinity==SQLITE_AFF_TEXT;
  }
  वापस sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
अटल u8 द्विआधारी_Compare_P5(
  स्थिर व्यंजक *pExpr1,   /* Left operand */
  स्थिर व्यंजक *pExpr2,   /* Right operand */
  पूर्णांक jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (अक्षर)sqlite3_व्यंजक_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  वापस aff;
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
CollSeq *sqlite3_द्विआधारी_Compare_Coll_Seq(
  Parse *pParse,
  स्थिर व्यंजक *p_बायाँ,
  स्थिर व्यंजक *p_दायाँ
){
  CollSeq *pColl;
  निश्चित( p_बायाँ );
  अगर( p_बायाँ->flags & EP_Collate ){
    pColl = sqlite3_व्यंजक_Coll_Seq(pParse, p_बायाँ);
  }अन्यथा अगर( p_दायाँ && (p_दायाँ->flags & EP_Collate)!=0 ){
    pColl = sqlite3_व्यंजक_Coll_Seq(pParse, p_दायाँ);
  }अन्यथा{
    pColl = sqlite3_व्यंजक_Coll_Seq(pParse, p_बायाँ);
    अगर( !pColl ){
      pColl = sqlite3_व्यंजक_Coll_Seq(pParse, p_दायाँ);
    }
  }
  वापस pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_व्यंजक_Compare_Coll_Seq(Parse *pParse, स्थिर व्यंजक *p){
  अगर( व्यंजक_Has_Property(p, EP_Commuted) ){
    वापस sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, p->p_दायाँ, p->p_बायाँ);
  }अन्यथा{
    वापस sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, p->p_बायाँ, p->p_दायाँ);
  }
}

/*
** Generate code for a comparison operator.
*/
अटल पूर्णांक codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  व्यंजक *p_बायाँ,      /* The left operand */
  व्यंजक *p_दायाँ,     /* The right operand */
  पूर्णांक opcode,       /* The comparison opcode */
  पूर्णांक in1, पूर्णांक in2, /* Register holding operands */
  पूर्णांक dest,         /* Jump here if true.  */
  पूर्णांक jumpIfNull,   /* If true, jump if either operand is NULL */
  पूर्णांक isCommuted    /* The comparison has been commuted */
){
  पूर्णांक p5;
  पूर्णांक addr;
  CollSeq *p4;

  अगर( pParse->nErr ) वापस 0;
  अगर( isCommuted ){
    p4 = sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, p_दायाँ, p_बायाँ);
  }अन्यथा{
    p4 = sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, p_बायाँ, p_दायाँ);
  }
  p5 = द्विआधारी_Compare_P5(p_बायाँ, p_दायाँ, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (व्योम*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  वापस addr;
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
पूर्णांक sqlite3_व्यंजक_Is_Vector(स्थिर व्यंजक *p_व्यंजक){
  वापस sqlite3_व्यंजक_Vector_आकार(p_व्यंजक)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
पूर्णांक sqlite3_व्यंजक_Vector_आकार(स्थिर व्यंजक *p_व्यंजक){
  u8 op = p_व्यंजक->op;
  अगर( op==TK_REGISTER ) op = p_व्यंजक->op2;
  अगर( op==TK_VECTOR ){
    निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
    वापस p_व्यंजक->x.pList->n_व्यंजक;
  }अन्यथा अगर( op==TK_SELECT ){
    निश्चित( व्यंजक_Use_XSelect(p_व्यंजक) );
    वापस p_व्यंजक->x.pSelect->pEList->n_व्यंजक;
  }अन्यथा{
    वापस 1;
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
व्यंजक *sqlite3VectorFieldSubexpr(व्यंजक *pVector, पूर्णांक i){
  निश्चित( i<sqlite3_व्यंजक_Vector_आकार(pVector) || pVector->op==TK_ERROR );
  अगर( sqlite3_व्यंजक_Is_Vector(pVector) ){
    निश्चित( pVector->op2==0 || pVector->op==TK_REGISTER );
    अगर( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      निश्चित( व्यंजक_Use_XSelect(pVector) );
      वापस pVector->x.pSelect->pEList->a[i].p_व्यंजक;
    }अन्यथा{
      निश्चित( व्यंजक_Use_XList(pVector) );
      वापस pVector->x.pList->a[i].p_व्यंजक;
    }
  }
  वापस pVector;
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
व्यंजक *sqlite3_व्यंजक_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  व्यंजक *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  पूर्णांक iField,          /* Which column of the vector to return */
  पूर्णांक nField           /* Total number of columns in the vector */
){
  व्यंजक *pRet;
  अगर( pVector->op==TK_SELECT ){
    निश्चित( व्यंजक_Use_XSelect(pVector) );
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
    अगर( pRet ){
      व्यंजक_Set_Property(pRet, EP_Full_आकार);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_बायाँ = pVector;
    }
  }अन्यथा{
    अगर( pVector->op==TK_VECTOR ){
      व्यंजक **ppVector;
      निश्चित( व्यंजक_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_व्यंजक;
      pVector = *ppVector;
      अगर( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        वापस pVector;
      }
    }
    pRet = sqlite3_व्यंजक_Dup(pParse->db, pVector, 0);
  }
  वापस pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
अटल पूर्णांक व्यंजक_Code_Subselect(Parse *pParse, व्यंजक *p_व्यंजक){
  पूर्णांक reg = 0;
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
  अगर( p_व्यंजक->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_व्यंजक);
  }
#पूर्ण_अगर
  वापस reg;
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
अटल पूर्णांक व्यंजक_Vector_Register(
  Parse *pParse,                  /* Parse context */
  व्यंजक *pVector,                  /* Vector to extract element from */
  पूर्णांक iField,                     /* Field to extract from pVector */
  पूर्णांक regSelect,                  /* First in array of registers */
  व्यंजक *pTmp,                     /* Temporary space */
  व्यंजक **pp_व्यंजक,                  /* OUT: Expression element */
  पूर्णांक *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  निश्चित( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  अगर( op==TK_REGISTER ){
    *pp_व्यंजक = sqlite3VectorFieldSubexpr(pVector, iField);
    वापस pVector->iTable+iField;
  }
  अगर( op==TK_SELECT ){
    निश्चित( व्यंजक_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    स_रखो(pTmp, 0, माप(व्यंजक));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_बायाँ = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_व्यंजक;
    *pp_व्यंजक = pTmp;
    वापस regSelect+iField;
  }
  अगर( op==TK_VECTOR ){
    निश्चित( व्यंजक_Use_XList(pVector) );
    *pp_व्यंजक = pVector->x.pList->a[iField].p_व्यंजक;
    वापस sqlite3_व्यंजक_Code_अस्थायी(pParse, *pp_व्यंजक, pRegFree);
  }
  वापस 0;
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
अटल व्योम codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  व्यंजक *p_व्यंजक,          /* The comparison operation */
  पूर्णांक dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
  व्यंजक *p_दायाँ = p_व्यंजक->p_दायाँ;
  पूर्णांक n_बायाँ = sqlite3_व्यंजक_Vector_आकार(p_बायाँ);
  पूर्णांक i;
  पूर्णांक reg_बायाँ = 0;
  पूर्णांक reg_दायाँ = 0;
  u8 opx = op;
  पूर्णांक addrCmp = 0;
  पूर्णांक addrDone = sqlite3VdbeMakeLabel(pParse);
  पूर्णांक isCommuted = व्यंजक_Has_Property(p_व्यंजक,EP_Commuted);

  निश्चित( !व्यंजक_Has_VVAProperty(p_व्यंजक,EP_Immutable) );
  अगर( pParse->nErr ) वापस;
  अगर( n_बायाँ!=sqlite3_व्यंजक_Vector_आकार(p_दायाँ) ){
    sqlite3_Error_संदेश(pParse, "row value misused");
    वापस;
  }
  निश्चित( p_व्यंजक->op==TK_EQ || p_व्यंजक->op==TK_NE
       || p_व्यंजक->op==TK_IS || p_व्यंजक->op==TK_ISNOT
       || p_व्यंजक->op==TK_LT || p_व्यंजक->op==TK_GT
       || p_व्यंजक->op==TK_LE || p_व्यंजक->op==TK_GE
  );
  निश्चित( p_व्यंजक->op==op || (p_व्यंजक->op==TK_IS && op==TK_EQ)
            || (p_व्यंजक->op==TK_ISNOT && op==TK_NE) );
  निश्चित( p5==0 || p_व्यंजक->op!=op );
  निश्चित( p5==SQLITE_NULLEQ || p_व्यंजक->op==op );

  अगर( op==TK_LE ) opx = TK_LT;
  अगर( op==TK_GE ) opx = TK_GT;
  अगर( op==TK_NE ) opx = TK_EQ;

  reg_बायाँ = व्यंजक_Code_Subselect(pParse, p_बायाँ);
  reg_दायाँ = व्यंजक_Code_Subselect(pParse, p_दायाँ);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  क्रम(i=0; 1 /*Loop exits by "break"*/; i++){
    पूर्णांक regFree1 = 0, regFree2 = 0;
    व्यंजक *pL = 0, *pR = 0;
    व्यंजक tmp1, tmp2;
    पूर्णांक r1, r2;
    निश्चित( i>=0 && i<n_बायाँ );
    अगर( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = व्यंजक_Vector_Register(pParse, p_बायाँ, i, reg_बायाँ, &tmp1, &pL, &regFree1);
    r2 = व्यंजक_Vector_Register(pParse, p_दायाँ, i, reg_दायाँ, &tmp2, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    codeCompare(pParse, pL, pR, opx, r1, r2, addrDone, p5, isCommuted);
    testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
    testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
    testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
    testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
    testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
    testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
    sqlite3_Release_अस्थायी_Reg(pParse, regFree1);
    sqlite3_Release_अस्थायी_Reg(pParse, regFree2);
    अगर( (opx==TK_LT || opx==TK_GT) && i<n_बायाँ-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    अगर( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }अन्यथा{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    अगर( i==n_बायाँ-1 ){
      अवरोध;
    }
    अगर( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }अन्यथा{
      निश्चित( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      अगर( i==n_बायाँ-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  अगर( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#अगर SQLITE_MAX_व्यंजक_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
पूर्णांक sqlite3_व्यंजक_Check_Height(Parse *pParse, पूर्णांक nHeight){
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक mxHeight = pParse->db->aLimit[SQLITE_LIMIT_व्यंजक_DEPTH];
  अगर( nHeight>mxHeight ){
    sqlite3_Error_संदेश(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  वापस rc;
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
अटल व्योम height_Of_व्यंजक(स्थिर व्यंजक *p, पूर्णांक *pnHeight){
  अगर( p ){
    अगर( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
अटल व्योम height_Of_व्यंजक_List(स्थिर व्यंजक_List *p, पूर्णांक *pnHeight){
  अगर( p ){
    पूर्णांक i;
    क्रम(i=0; i<p->n_व्यंजक; i++){
      height_Of_व्यंजक(p->a[i].p_व्यंजक, pnHeight);
    }
  }
}
अटल व्योम heightOfSelect(स्थिर Select *pSelect, पूर्णांक *pnHeight){
  स्थिर Select *p;
  क्रम(p=pSelect; p; p=p->pPrior){
    height_Of_व्यंजक(p->pWhere, pnHeight);
    height_Of_व्यंजक(p->pHaving, pnHeight);
    height_Of_व्यंजक(p->pLimit, pnHeight);
    height_Of_व्यंजक_List(p->pEList, pnHeight);
    height_Of_व्यंजक_List(p->pGroupBy, pnHeight);
    height_Of_व्यंजक_List(p->pOrderBy, pnHeight);
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
अटल व्योम व्यंजक_Set_Height(व्यंजक *p){
  पूर्णांक nHeight = p->p_बायाँ ? p->p_बायाँ->nHeight : 0;
  अगर( NEVER(p->p_दायाँ) && p->p_दायाँ->nHeight>nHeight ){
    nHeight = p->p_दायाँ->nHeight;
  }
  अगर( व्यंजक_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }अन्यथा अगर( p->x.pList ){
    height_Of_व्यंजक_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_व्यंजक_List_Flags(p->x.pList);
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
व्योम sqlite3_व्यंजक_Set_Height_And_Flags(Parse *pParse, व्यंजक *p){
  अगर( pParse->nErr ) वापस;
  व्यंजक_Set_Height(p);
  sqlite3_व्यंजक_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
पूर्णांक sqlite3_Select_व्यंजक_Height(स्थिर Select *p){
  पूर्णांक nHeight = 0;
  heightOfSelect(p, &nHeight);
  वापस nHeight;
}
#अन्यथा /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
व्योम sqlite3_व्यंजक_Set_Height_And_Flags(Parse *pParse, व्यंजक *p){
  अगर( pParse->nErr ) वापस;
  अगर( p && व्यंजक_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_व्यंजक_List_Flags(p->x.pList);
  }
}
#घोषणा व्यंजक_Set_Height(y)
#पूर्ण_अगर /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
व्योम sqlite3_व्यंजक_Set_Error_Offset(व्यंजक *p_व्यंजक, पूर्णांक iOfst){
  अगर( p_व्यंजक==0 ) वापस;
  अगर( NEVER(व्यंजक_Use_WJoin(p_व्यंजक)) ) वापस;
  p_व्यंजक->w.iOfst = iOfst;
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
व्यंजक *sqlite3_व्यंजक_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  पूर्णांक op,                 /* Expression opcode */
  स्थिर Token *pToken,    /* Token argument.  Might be NULL */
  पूर्णांक dequote             /* True to dequote */
){
  व्यंजक *pNew;
  पूर्णांक nExtra = pToken ? pToken->n+1 : 0;

  निश्चित( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, माप(व्यंजक)+nExtra);
  अगर( pNew ){
    स_रखो(pNew, 0, माप(व्यंजक));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    अगर( nExtra ){
      निश्चित( pToken!=0 );
      pNew->u.zToken = (अक्षर*)&pNew[1];
      निश्चित( pToken->z!=0 || pToken->n==0 );
      अगर( pToken->n ) स_नकल(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      अगर( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_व्यंजक(pNew);
      }
    }
#अगर SQLITE_MAX_व्यंजक_DEPTH>0
    pNew->nHeight = 1;
#पूर्ण_अगर 
  }
  वापस pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
व्यंजक *sqlite3_व्यंजक(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  पूर्णांक op,                 /* Expression opcode */
  स्थिर अक्षर *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  वापस sqlite3_व्यंजक_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
व्यंजक *sqlite3_व्यंजक_Int32(sqlite3 *db, पूर्णांक iVal){
  व्यंजक *pNew = sqlite3DbMallocRawNN(db, माप(व्यंजक));
  अगर( pNew ){
    स_रखो(pNew, 0, माप(व्यंजक));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_मूल्य|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_मूल्य = iVal;
#अगर SQLITE_MAX_व्यंजक_DEPTH>0
    pNew->nHeight = 1;
#पूर्ण_अगर 
  }
  वापस pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
व्योम sqlite3_व्यंजक_Attach_Subtrees(
  sqlite3 *db,
  व्यंजक *p_मूल,
  व्यंजक *p_बायाँ,
  व्यंजक *p_दायाँ
){
  अगर( p_मूल==0 ){
    निश्चित( db->mallocFailed );
    sqlite3_व्यंजक_Delete(db, p_बायाँ);
    sqlite3_व्यंजक_Delete(db, p_दायाँ);
  }अन्यथा{
    निश्चित( व्यंजक_Use_XList(p_मूल) );
    निश्चित( p_मूल->x.pSelect==0 );
    अगर( p_दायाँ ){
      p_मूल->p_दायाँ = p_दायाँ;
      p_मूल->flags |= EP_Propagate & p_दायाँ->flags;
#अगर SQLITE_MAX_व्यंजक_DEPTH>0
      p_मूल->nHeight = p_दायाँ->nHeight+1;
    }अन्यथा{
      p_मूल->nHeight = 1;
#पूर्ण_अगर
    }
    अगर( p_बायाँ ){
      p_मूल->p_बायाँ = p_बायाँ;
      p_मूल->flags |= EP_Propagate & p_बायाँ->flags;
#अगर SQLITE_MAX_व्यंजक_DEPTH>0
      अगर( p_बायाँ->nHeight>=p_मूल->nHeight ){
        p_मूल->nHeight = p_बायाँ->nHeight+1;
      }
#पूर्ण_अगर
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
व्यंजक *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  पूर्णांक op,                 /* Expression opcode */
  व्यंजक *p_बायाँ,            /* Left operand */
  व्यंजक *p_दायाँ            /* Right operand */
){
  व्यंजक *p;
  p = sqlite3DbMallocRawNN(pParse->db, माप(व्यंजक));
  अगर( p ){
    स_रखो(p, 0, माप(व्यंजक));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_व्यंजक_Attach_Subtrees(pParse->db, p, p_बायाँ, p_दायाँ);
    sqlite3_व्यंजक_Check_Height(pParse, p->nHeight);
  }अन्यथा{
    sqlite3_व्यंजक_Delete(pParse->db, p_बायाँ);
    sqlite3_व्यंजक_Delete(pParse->db, p_दायाँ);
  }
  वापस p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
व्योम sqlite3PExprAddSelect(Parse *pParse, व्यंजक *p_व्यंजक, Select *pSelect){
  अगर( p_व्यंजक ){
    p_व्यंजक->x.pSelect = pSelect;
    व्यंजक_Set_Property(p_व्यंजक, EP_xIsSelect|EP_Subquery);
    sqlite3_व्यंजक_Set_Height_And_Flags(pParse, p_व्यंजक);
  }अन्यथा{
    निश्चित( pParse->db->mallocFailed );
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
Select *sqlite3_व्यंजक_List_To_मूल्यावली(Parse *pParse, पूर्णांक nElem, व्यंजक_List *pEList){
  पूर्णांक ii;
  Select *pRet = 0;
  निश्चित( nElem>1 );
  क्रम(ii=0; ii<pEList->n_व्यंजक; ii++){
    Select *pSel;
    व्यंजक *p_व्यंजक = pEList->a[ii].p_व्यंजक;
    पूर्णांक n_व्यंजक_Elem;
    अगर( p_व्यंजक->op==TK_VECTOR ){
      निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
      n_व्यंजक_Elem = p_व्यंजक->x.pList->n_व्यंजक;
    }अन्यथा{
      n_व्यंजक_Elem = 1;
    }
    अगर( n_व्यंजक_Elem!=nElem ){
      sqlite3_Error_संदेश(pParse, "IN(...) element has %d term%s - expected %d",
          n_व्यंजक_Elem, n_व्यंजक_Elem>1?"s":"", nElem
      );
      अवरोध;
    }
    निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
    pSel = sqlite3SelectNew(pParse, p_व्यंजक->x.pList, 0, 0, 0, 0, 0, SF_मूल्यावली,0);
    p_व्यंजक->x.pList = 0;
    अगर( pSel ){
      अगर( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  अगर( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_मूल्य;
  }
  sqlite3_व्यंजक_List_Delete(pParse->db, pEList);
  वापस pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
व्यंजक *sqlite3_व्यंजक_And(Parse *pParse, व्यंजक *p_बायाँ, व्यंजक *p_दायाँ){
  sqlite3 *db = pParse->db;
  अगर( p_बायाँ==0  ){
    वापस p_दायाँ;
  }अन्यथा अगर( p_दायाँ==0 ){
    वापस p_बायाँ;
  }अन्यथा{
    u32 f = p_बायाँ->flags | p_दायाँ->flags;
    अगर( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      व्यंजक *p;
      sqlite3_व्यंजक_Deferred_Delete(pParse, p_बायाँ);
      sqlite3_व्यंजक_Deferred_Delete(pParse, p_दायाँ);
      p = sqlite3_व्यंजक_Int32(db, 0);
      अगर( p ) p->op2 = 1;
      वापस p;
    }अन्यथा{
      वापस sqlite3PExpr(pParse, TK_AND, p_बायाँ, p_दायाँ);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
व्यंजक *sqlite3_व्यंजक_Function(
  Parse *pParse,        /* Parsing context */
  व्यंजक_List *pList,      /* Argument list */
  स्थिर Token *pToken,  /* Name of the function */
  पूर्णांक eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  व्यंजक *pNew;
  sqlite3 *db = pParse->db;
  निश्चित( pToken );
  pNew = sqlite3_व्यंजक_Alloc(db, TK_FUNCTION, pToken, 1);
  अगर( pNew==0 ){
    sqlite3_व्यंजक_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    वापस 0;
  }
  निश्चित( !व्यंजक_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (पूर्णांक)(pToken->z - pParse->z_पुच्छ);
  अगर( pList
   && pList->n_व्यंजक > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_संदेश(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  व्यंजक_Set_Property(pNew, EP_HasFunc);
  निश्चित( व्यंजक_Use_XList(pNew) );
  sqlite3_व्यंजक_Set_Height_And_Flags(pParse, pNew);
  अगर( eDistinct==SF_Distinct ) व्यंजक_Set_Property(pNew, EP_Distinct);
  वापस pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
व्योम sqlite3_व्यंजक_Order_By_Aggregate_Error(Parse *pParse, व्यंजक *p){
  sqlite3_Error_संदेश(pParse,
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
व्योम sqlite3_व्यंजक_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  व्यंजक *p_व्यंजक,          /* The function call to which ORDER BY is to be added */
  व्यंजक_List *pOrderBy    /* The ORDER BY clause to add */
){
  व्यंजक *pOB;
  sqlite3 *db = pParse->db;
  अगर( NEVER(pOrderBy==0) ){
    निश्चित( db->mallocFailed );
    वापस;
  }
  अगर( p_व्यंजक==0 ){
    निश्चित( db->mallocFailed );
    sqlite3_व्यंजक_List_Delete(db, pOrderBy);
    वापस;
  }
  निश्चित( p_व्यंजक->op==TK_FUNCTION );
  निश्चित( p_व्यंजक->p_बायाँ==0 );
  निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
  अगर( p_व्यंजक->x.pList==0 || NEVER(p_व्यंजक->x.pList->n_व्यंजक==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_व्यंजक_List_Delete_Generic, pOrderBy);
    वापस;
  }
  अगर( IsWindowFunc(p_व्यंजक) ){
    sqlite3_व्यंजक_Order_By_Aggregate_Error(pParse, p_व्यंजक);
    sqlite3_व्यंजक_List_Delete(db, pOrderBy);
    वापस;
  }
  अगर( pOrderBy->n_व्यंजक>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_संदेश(pParse, "too many terms in ORDER BY clause");
    sqlite3_व्यंजक_List_Delete(db, pOrderBy);
    वापस;
  }

  pOB = sqlite3_व्यंजक_Alloc(db, TK_ORDER, 0, 0);
  अगर( pOB==0 ){
    sqlite3_व्यंजक_List_Delete(db, pOrderBy);
    वापस;
  }
  pOB->x.pList = pOrderBy;
  निश्चित( व्यंजक_Use_XList(pOB) );
  p_व्यंजक->p_बायाँ = pOB;
  व्यंजक_Set_Property(pOB, EP_Full_आकार);
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
व्योम sqlite3_व्यंजक_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  स्थिर व्यंजक *p_व्यंजक,     /* The function invocation */
  स्थिर FuncDef *pDef    /* The function being invoked */
){
  निश्चित( !IN_RENAME_OBJECT );
  निश्चित( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  अगर( व्यंजक_Has_Property(p_व्यंजक, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    अगर( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_संदेश(pParse, "unsafe use of %#T()", p_व्यंजक);
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
व्योम sqlite3_व्यंजक_Assign_Var_संख्या(Parse *pParse, व्यंजक *p_व्यंजक, u32 n){
  sqlite3 *db = pParse->db;
  स्थिर अक्षर *z;
  ynVar x;

  अगर( p_व्यंजक==0 ) वापस;
  निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य|EP_Reduced|EP_TokenOnly) );
  z = p_व्यंजक->u.zToken;
  निश्चित( z!=0 );
  निश्चित( z[0]!=0 );
  निश्चित( n==(u32)sqlite3Strlen30(z) );
  अगर( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    निश्चित( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }अन्यथा{
    पूर्णांक doAdd = 0;
    अगर( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      पूर्णांक bOk;
      अगर( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }अन्यथा{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_संख्या]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_संख्या] );
      अगर( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_संख्या] ){
        sqlite3_Error_संदेश(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_संख्या]);
        sqlite3_Record_Error_Offset_Of_व्यंजक(pParse->db, p_व्यंजक);
        वापस;
      }
      x = (ynVar)i;
      अगर( x>pParse->nVar ){
        pParse->nVar = (पूर्णांक)x;
        doAdd = 1;
      }अन्यथा अगर( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }अन्यथा अगर( sqlite3_VList_अंक_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }अन्यथा{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_अंक(pParse->pVList, z, n);
      अगर( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    अगर( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      अगर( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_व्यंजक->iColumn = x;
  अगर( x>db->aLimit[SQLITE_LIMIT_VARIABLE_संख्या] ){
    sqlite3_Error_संदेश(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_व्यंजक(pParse->db, p_व्यंजक);
  }
}

/*
** Recursively delete an expression tree.
*/
अटल SQLITE_NOINLINE व्योम sqlite3_व्यंजक_Delete_NN(sqlite3 *db, व्यंजक *p){
  निश्चित( p!=0 );
  निश्चित( db!=0 );
व्यंजक_Delete_Restart:
  निश्चित( !व्यंजक_Use_UValue(p) || p->u.i_मूल्य>=0 );
  निश्चित( !व्यंजक_Use_YWin(p) || !व्यंजक_Use_YSub(p) );
  निश्चित( !व्यंजक_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  निश्चित( p->op!=TK_FUNCTION || !व्यंजक_Use_YSub(p) );
#अगर_घोषित SQLITE_DEBUG
  अगर( व्यंजक_Has_Property(p, EP_Leaf) && !व्यंजक_Has_Property(p, EP_TokenOnly) ){
    निश्चित( p->p_बायाँ==0 );
    निश्चित( p->p_दायाँ==0 );
    निश्चित( !व्यंजक_Use_XSelect(p) || p->x.pSelect==0 );
    निश्चित( !व्यंजक_Use_XList(p) || p->x.pList==0 );
  }
#पूर्ण_अगर
  अगर( !व्यंजक_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    निश्चित( (व्यंजक_Use_XList(p) && p->x.pList==0) || p->p_दायाँ==0 );
    अगर( p->p_दायाँ ){
      निश्चित( !व्यंजक_Has_Property(p, EP_WinFunc) );
      sqlite3_व्यंजक_Delete_NN(db, p->p_दायाँ);
    }अन्यथा अगर( व्यंजक_Use_XSelect(p) ){
      निश्चित( !व्यंजक_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }अन्यथा{
      sqlite3_व्यंजक_List_Delete(db, p->x.pList);
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      अगर( व्यंजक_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#पूर्ण_अगर
    }
    अगर( p->p_बायाँ && p->op!=TK_SELECT_COLUMN ){
      व्यंजक *p_बायाँ = p->p_बायाँ;
      अगर( !व्यंजक_Has_Property(p, EP_Static)
       && !व्यंजक_Has_Property(p_बायाँ, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_बायाँ;
        जाओ व्यंजक_Delete_Restart;
      }अन्यथा{
        sqlite3_व्यंजक_Delete_NN(db, p_बायाँ);
      }
    }
  }
  अगर( !व्यंजक_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
व्योम sqlite3_व्यंजक_Delete(sqlite3 *db, व्यंजक *p){
  अगर( p ) sqlite3_व्यंजक_Delete_NN(db, p);
}
व्योम sqlite3_व्यंजक_Delete_Generic(sqlite3 *db, व्योम *p){
  अगर( ALWAYS(p) ) sqlite3_व्यंजक_Delete_NN(db, (व्यंजक*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
व्योम sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  अगर( p==0 ){
    /* Nothing to clear */
  }अन्यथा अगर( p->pOn ){
    sqlite3_व्यंजक_Delete_NN(db, p->pOn);
  }अन्यथा अगर( p->pUsing ){
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
पूर्णांक sqlite3_व्यंजक_Deferred_Delete(Parse *pParse, व्यंजक *p_व्यंजक){
  वापस 0==sqlite3ParserAddCleanup(pParse, sqlite3_व्यंजक_Delete_Generic, p_व्यंजक);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
व्योम sqlite3_व्यंजक_Unmap_And_Delete(Parse *pParse, व्यंजक *p){
  अगर( p ){
    अगर( IN_RENAME_OBJECT ){
      sqlite3_Rename_व्यंजक_Unmap(pParse, p);
    }
    sqlite3_व्यंजक_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
अटल पूर्णांक व्यंजक_Struct_आकार(स्थिर व्यंजक *p){
  अगर( व्यंजक_Has_Property(p, EP_TokenOnly) ) वापस व्यंजक_TOKENONLYSIZE;
  अगर( व्यंजक_Has_Property(p, EP_Reduced) ) वापस व्यंजक_REDUCEDSIZE;
  वापस व्यंजक_FULLSIZE;
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
अटल पूर्णांक duped_व्यंजक_Struct_आकार(स्थिर व्यंजक *p, पूर्णांक flags){
  पूर्णांक n_आकार;
  निश्चित( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  निश्चित( व्यंजक_FULLSIZE<=0xfff );
  निश्चित( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  अगर( 0==flags || व्यंजक_Has_Property(p, EP_Full_आकार) ){
    n_आकार = व्यंजक_FULLSIZE;
  }अन्यथा{
    निश्चित( !व्यंजक_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    निश्चित( !व्यंजक_Has_Property(p, EP_OuterON) );
    निश्चित( !व्यंजक_Has_VVAProperty(p, EP_NoReduce) );
    अगर( p->p_बायाँ || p->x.pList ){
      n_आकार = व्यंजक_REDUCEDSIZE | EP_Reduced;
    }अन्यथा{
      निश्चित( p->p_दायाँ==0 );
      n_आकार = व्यंजक_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  वापस n_आकार;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
अटल पूर्णांक duped_व्यंजक_गाँठ_आकार(स्थिर व्यंजक *p, पूर्णांक flags){
  पूर्णांक nByte = duped_व्यंजक_Struct_आकार(p, flags) & 0xfff;
  अगर( !व्यंजक_Has_Property(p, EP_Int_मूल्य) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  वापस ROUND8(nByte);
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
अटल पूर्णांक duped_व्यंजक_आकार(स्थिर व्यंजक *p){
  पूर्णांक nByte;
  निश्चित( p!=0 );
  nByte = duped_व्यंजक_गाँठ_आकार(p, EXPRDUP_REDUCE);
  अगर( p->p_बायाँ ) nByte += duped_व्यंजक_आकार(p->p_बायाँ);
  अगर( p->p_दायाँ ) nByte += duped_व्यंजक_आकार(p->p_दायाँ);
  निश्चित( nByte==ROUND8(nByte) );
  वापस nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
प्रकार काष्ठा EdupBuf EdupBuf;
काष्ठा EdupBuf {
  u8 *zAlloc;          /* Memory space available for storage */
#अगर_घोषित SQLITE_DEBUG
  u8 *zEnd;            /* First byte past the end of memory */
#पूर्ण_अगर
};

/*
** This function is similar to sqlite3ExprDup(), except that if pEdupBuf
** is not NULL then it points to memory that can be used to store a copy
** of the input Expr p together with its p->u.zToken (if any).  pEdupBuf
** is updated with the new buffer tail prior to returning.
*/
अटल व्यंजक *व्यंजक_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  स्थिर व्यंजक *p,        /* Expr tree to be duplicated */
  पूर्णांक dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  व्यंजक *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  पूर्णांक nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  निश्चित( db!=0 );
  निश्चित( p );
  निश्चित( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  निश्चित( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  अगर( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#अगर_घोषित SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#पूर्ण_अगर
    staticFlag = EP_Static;
    निश्चित( sEdupBuf.zAlloc!=0 );
    निश्चित( dupFlags==EXPRDUP_REDUCE );
  }अन्यथा{
    पूर्णांक nAlloc;
    अगर( dupFlags ){
      nAlloc = duped_व्यंजक_आकार(p);
    }अन्यथा अगर( !व्यंजक_Has_Property(p, EP_Int_मूल्य) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(व्यंजक_FULLSIZE + nToken);
    }अन्यथा{
      nToken = 0;
      nAlloc = ROUND8(व्यंजक_FULLSIZE);
    }
    निश्चित( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#अगर_घोषित SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#पूर्ण_अगर
    
    staticFlag = 0;
  }
  pNew = (व्यंजक *)sEdupBuf.zAlloc;
  निश्चित( EIGHT_BYTE_ALIGNMENT(pNew) );

  अगर( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    स्थिर अचिन्हित n_Struct_आकार = duped_व्यंजक_Struct_आकार(p, dupFlags);
    पूर्णांक n_New_आकार = n_Struct_आकार & 0xfff;
    अगर( nToken<0 ){
      अगर( !व्यंजक_Has_Property(p, EP_Int_मूल्य) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }अन्यथा{
        nToken = 0;
      }
    }
    अगर( dupFlags ){
      निश्चित( (पूर्णांक)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= n_New_आकार+nToken );
      निश्चित( व्यंजक_Has_Property(p, EP_Reduced)==0 );
      स_नकल(sEdupBuf.zAlloc, p, n_New_आकार);
    }अन्यथा{
      u32 n_आकार = (u32)व्यंजक_Struct_आकार(p);
      निश्चित( (पूर्णांक)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (पूर्णांक)व्यंजक_FULLSIZE+nToken );
      स_नकल(sEdupBuf.zAlloc, p, n_आकार);
      अगर( n_आकार<व्यंजक_FULLSIZE ){
        स_रखो(&sEdupBuf.zAlloc[n_आकार], 0, व्यंजक_FULLSIZE-n_आकार);
      }
      n_New_आकार = व्यंजक_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= n_Struct_आकार & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    व्यंजक_Clear_VVAProperties(pNew);
    अगर( dupFlags ){
      व्यंजक_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    निश्चित( nToken>=0 );
    अगर( nToken>0 ){
      अक्षर *zToken = pNew->u.zToken = (अक्षर*)&sEdupBuf.zAlloc[n_New_आकार];
      स_नकल(zToken, p->u.zToken, nToken);
      n_New_आकार += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(n_New_आकार);

    अगर( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      अगर( व्यंजक_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }अन्यथा{
        pNew->x.pList = sqlite3_व्यंजक_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      अगर( व्यंजक_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        निश्चित( व्यंजक_Has_Property(pNew, EP_WinFunc) );
      }
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      अगर( dupFlags ){
        अगर( p->op==TK_SELECT_COLUMN ){
          pNew->p_बायाँ = p->p_बायाँ;
          निश्चित( p->p_दायाँ==0 
               || p->p_दायाँ==p->p_बायाँ
               || व्यंजक_Has_Property(p->p_बायाँ, EP_Subquery) );
        }अन्यथा{
          pNew->p_बायाँ = p->p_बायाँ ?
                      व्यंजक_Dup(db, p->p_बायाँ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_दायाँ = p->p_दायाँ ?
                       व्यंजक_Dup(db, p->p_दायाँ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }अन्यथा{
        अगर( p->op==TK_SELECT_COLUMN ){
          pNew->p_बायाँ = p->p_बायाँ;
          निश्चित( p->p_दायाँ==0 
               || p->p_दायाँ==p->p_बायाँ
               || व्यंजक_Has_Property(p->p_बायाँ, EP_Subquery) );
        }अन्यथा{
          pNew->p_बायाँ = sqlite3_व्यंजक_Dup(db, p->p_बायाँ, 0);
        }
        pNew->p_दायाँ = sqlite3_व्यंजक_Dup(db, p->p_दायाँ, 0);
      }
    }
  }
  अगर( pEdupBuf ) स_नकल(pEdupBuf, &sEdupBuf, माप(sEdupBuf));
  निश्चित( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  वापस pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#अगर_अघोषित SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  अगर( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    अगर( pRet ){
      पूर्णांक i;
      pRet->nCte = p->nCte;
      क्रम(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_व्यंजक_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  वापस pRet;
}
#अन्यथा
# define sqlite3WithDup(x,y) 0
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
अटल पूर्णांक gatherSelectWindowsCallback(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_FUNCTION && व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_व्यंजक->y.pWin;
    निश्चित( pWin );
    निश्चित( IsWindowFunc(p_व्यंजक) );
    निश्चित( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  वापस WRC_Continue;
}
अटल पूर्णांक gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  वापस p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
अटल व्योम gatherSelectWindows(Select *p){
  Walker w;
  w.x_व्यंजक_Callback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}
#पूर्ण_अगर


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
व्यंजक *sqlite3_व्यंजक_Dup(sqlite3 *db, स्थिर व्यंजक *p, पूर्णांक flags){
  निश्चित( flags==0 || flags==EXPRDUP_REDUCE );
  वापस p ? व्यंजक_Dup(db, p, flags, 0) : 0;
}
व्यंजक_List *sqlite3_व्यंजक_List_Dup(sqlite3 *db, स्थिर व्यंजक_List *p, पूर्णांक flags){
  व्यंजक_List *pNew;
  काष्ठा व्यंजक_List_item *pItem;
  स्थिर काष्ठा व्यंजक_List_item *pOldItem;
  पूर्णांक i;
  व्यंजक *pPriorSelectColOld = 0;
  व्यंजक *pPriorSelectColNew = 0;
  निश्चित( db!=0 );
  अगर( p==0 ) वापस 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3_Db_Malloc_आकार(db, p));
  अगर( pNew==0 ) वापस 0;
  pNew->n_व्यंजक = p->n_व्यंजक;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  क्रम(i=0; i<p->n_व्यंजक; i++, pItem++, pOldItem++){
    व्यंजक *p_Old_व्यंजक = pOldItem->p_व्यंजक;
    व्यंजक *p_New_व्यंजक;
    pItem->p_व्यंजक = sqlite3_व्यंजक_Dup(db, p_Old_व्यंजक, flags);
    अगर( p_Old_व्यंजक
     && p_Old_व्यंजक->op==TK_SELECT_COLUMN
     && (p_New_व्यंजक = pItem->p_व्यंजक)!=0
    ){
      अगर( p_New_व्यंजक->p_दायाँ ){
        pPriorSelectColOld = p_Old_व्यंजक->p_दायाँ;
        pPriorSelectColNew = p_New_व्यंजक->p_दायाँ;
        p_New_व्यंजक->p_बायाँ = p_New_व्यंजक->p_दायाँ;
      }अन्यथा{
        अगर( p_Old_व्यंजक->p_बायाँ!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_व्यंजक->p_बायाँ;
          pPriorSelectColNew = sqlite3_व्यंजक_Dup(db, pPriorSelectColOld, flags);
          p_New_व्यंजक->p_दायाँ = pPriorSelectColNew;
        }
        p_New_व्यंजक->p_बायाँ = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  वापस pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#अगर !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, स्थिर SrcList *p, पूर्णांक flags){
  SrcList *pNew;
  पूर्णांक i;
  निश्चित( db!=0 );
  अगर( p==0 ) वापस 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  अगर( pNew==0 ) वापस 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  क्रम(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    स्थिर SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    अगर( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, माप(Subquery));
      अगर( pNewSubq==0 ){
        निश्चित( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }अन्यथा{
        स_नकल(pNewSubq, pOldItem->u4.pSubq, माप(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        अगर( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }अन्यथा अगर( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }अन्यथा{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    अगर( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }अन्यथा अगर( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_व्यंजक_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }अन्यथा{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    अगर( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    अगर( pTab ){
      pTab->nTabRef++;
    }
    अगर( pOldItem->fg.isUsing ){
      निश्चित( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }अन्यथा{
      pNewItem->u3.pOn = sqlite3_व्यंजक_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  वापस pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, स्थिर IdList *p){
  IdList *pNew;
  पूर्णांक i;
  निश्चित( db!=0 );
  अगर( p==0 ) वापस 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  अगर( pNew==0 ) वापस 0;
  pNew->nId = p->nId;
  क्रम(i=0; i<p->nId; i++){
    काष्ठा IdList_item *pNewItem = &pNew->a[i];
    स्थिर काष्ठा IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  वापस pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, स्थिर Select *pDup, पूर्णांक flags){
  Select *pRet = 0;
  Select *p_अग्र = 0;
  Select **pp = &pRet;
  स्थिर Select *p;

  निश्चित( db!=0 );
  क्रम(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, माप(*p) );
    अगर( pNew==0 ) अवरोध;
    pNew->pEList = sqlite3_व्यंजक_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_व्यंजक_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_व्यंजक_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_व्यंजक_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_व्यंजक_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->p_अग्र = p_अग्र;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_व्यंजक_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    अगर( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#पूर्ण_अगर
    pNew->selId = p->selId;
    अगर( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->p_अग्र = 0;
      sqlite3SelectDelete(db, pNew);
      अवरोध;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    p_अग्र = pNew;
  }
  वापस pRet;
}
#अन्यथा
Select *sqlite3SelectDup(sqlite3 *db, स्थिर Select *p, पूर्णांक flags){
  निश्चित( p==0 );
  वापस 0;
}
#पूर्ण_अगर


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
अटल स्थिर काष्ठा व्यंजक_List_item zeroItem = {0};
SQLITE_NOINLINE व्यंजक_List *sqlite3_व्यंजक_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  व्यंजक *p_व्यंजक             /* Expression to be appended. Might be NULL */
){
  काष्ठा व्यंजक_List_item *pItem;
  व्यंजक_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  अगर( pList==0 ){
    sqlite3_व्यंजक_Delete(db, p_व्यंजक);
    वापस 0;
  }
  pList->nAlloc = 4;
  pList->n_व्यंजक = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_व्यंजक = p_व्यंजक;
  वापस pList;
}
SQLITE_NOINLINE व्यंजक_List *sqlite3_व्यंजक_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  व्यंजक_List *pList,        /* List to which to append. Might be NULL */
  व्यंजक *p_व्यंजक             /* Expression to be appended. Might be NULL */
){
  काष्ठा व्यंजक_List_item *pItem;
  व्यंजक_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  अगर( pNew==0 ){
    sqlite3_व्यंजक_List_Delete(db, pList);
    sqlite3_व्यंजक_Delete(db, p_व्यंजक);
    वापस 0;
  }अन्यथा{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_व्यंजक++];
  *pItem = zeroItem;
  pItem->p_व्यंजक = p_व्यंजक;
  वापस pList;
}
व्यंजक_List *sqlite3_व्यंजक_List_Append(
  Parse *pParse,          /* Parsing context */
  व्यंजक_List *pList,        /* List to which to append. Might be NULL */
  व्यंजक *p_व्यंजक             /* Expression to be appended. Might be NULL */
){
  काष्ठा व्यंजक_List_item *pItem;
  अगर( pList==0 ){
    वापस sqlite3_व्यंजक_List_Append_New(pParse->db,p_व्यंजक);
  }
  अगर( pList->nAlloc<pList->n_व्यंजक+1 ){
    वापस sqlite3_व्यंजक_List_Append_Grow(pParse->db,pList,p_व्यंजक);
  }
  pItem = &pList->a[pList->n_व्यंजक++];
  *pItem = zeroItem;
  pItem->p_व्यंजक = p_व्यंजक;
  वापस pList;
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
व्यंजक_List *sqlite3_व्यंजक_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  व्यंजक_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  व्यंजक *p_व्यंजक            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  पूर्णांक n;
  पूर्णांक i;
  पूर्णांक iFirst = pList ? pList->n_व्यंजक : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  अगर( NEVER(pColumns==0) ) जाओ vector_append_error;
  अगर( p_व्यंजक==0 ) जाओ vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  अगर( p_व्यंजक->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_व्यंजक_Vector_आकार(p_व्यंजक)) ){
    sqlite3_Error_संदेश(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    जाओ vector_append_error;
  }

  क्रम(i=0; i<pColumns->nId; i++){
    व्यंजक *p_Sub_व्यंजक = sqlite3_व्यंजक_For_Vector_Field(pParse, p_व्यंजक, i, pColumns->nId);
    निश्चित( p_Sub_व्यंजक!=0 || db->mallocFailed );
    अगर( p_Sub_व्यंजक==0 ) जारी;
    pList = sqlite3_व्यंजक_List_Append(pParse, pList, p_Sub_व्यंजक);
    अगर( pList ){
      निश्चित( pList->n_व्यंजक==iFirst+i+1 );
      pList->a[pList->n_व्यंजक-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  अगर( !db->mallocFailed && p_व्यंजक->op==TK_SELECT && ALWAYS(pList!=0) ){
    व्यंजक *pFirst = pList->a[iFirst].p_व्यंजक;
    निश्चित( pFirst!=0 );
    निश्चित( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_दायाँ = p_व्यंजक;
    p_व्यंजक = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_व्यंजक_Unmap_And_Delete(pParse, p_व्यंजक);
  sqlite3IdListDelete(db, pColumns);
  वापस pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
व्योम sqlite3_व्यंजक_List_Set_Sort_Order(व्यंजक_List *p, पूर्णांक iSortOrder, पूर्णांक eNulls){
  काष्ठा व्यंजक_List_item *pItem;
  अगर( p==0 ) वापस;
  निश्चित( p->n_व्यंजक>0 );

  निश्चित( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  निश्चित( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  निश्चित( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_व्यंजक-1];
  निश्चित( pItem->fg.bNulls==0 );
  अगर( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  अगर( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    अगर( iSortOrder!=eNulls ){
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
व्योम sqlite3_व्यंजक_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  व्यंजक_List *pList,        /* List to which to add the span. */
  स्थिर Token *pName,     /* Name to be added */
  पूर्णांक dequote             /* True to cause the name to be dequoted */
){
  निश्चित( pList!=0 || pParse->db->mallocFailed!=0 );
  निश्चित( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  अगर( pList ){
    काष्ठा व्यंजक_List_item *pItem;
    निश्चित( pList->n_व्यंजक>0 );
    pItem = &pList->a[pList->n_व्यंजक-1];
    निश्चित( pItem->zEName==0 );
    निश्चित( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    अगर( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      अगर( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (स्थिर व्योम*)pItem->zEName, pName);
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
व्योम sqlite3_व्यंजक_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  व्यंजक_List *pList,        /* List to which to add the span. */
  स्थिर अक्षर *zStart,     /* Start of the span */
  स्थिर अक्षर *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  निश्चित( pList!=0 || db->mallocFailed!=0 );
  अगर( pList ){
    काष्ठा व्यंजक_List_item *pItem = &pList->a[pList->n_व्यंजक-1];
    निश्चित( pList->n_व्यंजक>0 );
    अगर( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
व्योम sqlite3_व्यंजक_List_Check_लंबाई(
  Parse *pParse,
  व्यंजक_List *pEList,
  स्थिर अक्षर *zObject
){
  पूर्णांक mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_व्यंजक==mx );
  testcase( pEList && pEList->n_व्यंजक==mx+1 );
  अगर( pEList && pEList->n_व्यंजक>mx ){
    sqlite3_Error_संदेश(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
अटल SQLITE_NOINLINE व्योम व्यंजक_List_Delete_NN(sqlite3 *db, व्यंजक_List *pList){
  पूर्णांक i = pList->n_व्यंजक;
  काष्ठा व्यंजक_List_item *pItem =  pList->a;
  निश्चित( pList->n_व्यंजक>0 );
  निश्चित( db!=0 );
  करो{
    sqlite3_व्यंजक_Delete(db, pItem->p_व्यंजक);
    अगर( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }जबतक( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
व्योम sqlite3_व्यंजक_List_Delete(sqlite3 *db, व्यंजक_List *pList){
  अगर( pList ) व्यंजक_List_Delete_NN(db, pList);
}
व्योम sqlite3_व्यंजक_List_Delete_Generic(sqlite3 *db, व्योम *pList){
  अगर( ALWAYS(pList) ) व्यंजक_List_Delete_NN(db, (व्यंजक_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_व्यंजक_List_Flags(स्थिर व्यंजक_List *pList){
  पूर्णांक i;
  u32 m = 0;
  निश्चित( pList!=0 );
  क्रम(i=0; i<pList->n_व्यंजक; i++){
     व्यंजक *p_व्यंजक = pList->a[i].p_व्यंजक;
     निश्चित( p_व्यंजक!=0 );
     m |= p_व्यंजक->flags;
  }
  वापस m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
पूर्णांक sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  वापस WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(स्थिर अक्षर *zIn){
  अगर( sqlite3StrICmp(zIn, "true")==0  ) वापस EP_IsTrue;
  अगर( sqlite3StrICmp(zIn, "false")==0 ) वापस EP_IsFalse;
  वापस 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
पूर्णांक sqlite3_व्यंजक_Id_To_True_False(व्यंजक *p_व्यंजक){
  u32 v;
  निश्चित( p_व्यंजक->op==TK_ID || p_व्यंजक->op==TK_STRING );
  अगर( !व्यंजक_Has_Property(p_व्यंजक, EP_Quoted|EP_Int_मूल्य)
   && (v = sqlite3IsTrueOrFalse(p_व्यंजक->u.zToken))!=0
  ){
    p_व्यंजक->op = TK_TRUEFALSE;
    व्यंजक_Set_Property(p_व्यंजक, v);
    वापस 1;
  }
  वापस 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
पूर्णांक sqlite3_व्यंजक_Truth_मूल्य(स्थिर व्यंजक *p_व्यंजक){
  p_व्यंजक = sqlite3_व्यंजक_Skip_Collate_And_Likely((व्यंजक*)p_व्यंजक);
  निश्चित( p_व्यंजक->op==TK_TRUEFALSE );
  निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
  निश्चित( sqlite3StrICmp(p_व्यंजक->u.zToken,"true")==0
       || sqlite3StrICmp(p_व्यंजक->u.zToken,"false")==0 );
  वापस p_व्यंजक->u.zToken[4]==0;
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
व्यंजक *sqlite3_व्यंजक_Simplified_And_Or(व्यंजक *p_व्यंजक){
  निश्चित( p_व्यंजक!=0 );
  अगर( p_व्यंजक->op==TK_AND || p_व्यंजक->op==TK_OR ){
    व्यंजक *p_दायाँ = sqlite3_व्यंजक_Simplified_And_Or(p_व्यंजक->p_दायाँ);
    व्यंजक *p_बायाँ = sqlite3_व्यंजक_Simplified_And_Or(p_व्यंजक->p_बायाँ);
    अगर( व्यंजक_Always_True(p_बायाँ) || व्यंजक_Always_False(p_दायाँ) ){
      p_व्यंजक = p_व्यंजक->op==TK_AND ? p_दायाँ : p_बायाँ;
    }अन्यथा अगर( व्यंजक_Always_True(p_दायाँ) || व्यंजक_Always_False(p_बायाँ) ){
      p_व्यंजक = p_व्यंजक->op==TK_AND ? p_बायाँ : p_दायाँ;
    }
  }
  वापस p_व्यंजक;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
अटल पूर्णांक व्यंजक_Eval_Rhs_First(व्यंजक *p_व्यंजक){
  अगर( व्यंजक_Has_Property(p_व्यंजक->p_बायाँ, EP_Subquery)
   && !व्यंजक_Has_Property(p_व्यंजक->p_दायाँ, EP_Subquery)
  ){
    वापस 1;
  }अन्यथा{
    वापस 0;
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
अटल पूर्णांक व्यंजक_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  व्यंजक *p_व्यंजक,       /* The comparison expression */
  पूर्णांक *pR1,          /* OUT: Register holding the left operand */
  पूर्णांक *pR2,          /* OUT: Register holding the right operand */
  पूर्णांक *pFree1,       /* OUT: Temp register to free if not zero */
  पूर्णांक *pFree2        /* OUT: Another temp register to free if not zero */
){
  पूर्णांक addrIsNull;
  पूर्णांक r1, r2;
  Vdbe *v = pParse->pVdbe;

  निश्चित( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  अगर( व्यंजक_Eval_Rhs_First(p_व्यंजक) && sqlite3_व्यंजक_Can_Be_Null(p_व्यंजक->p_दायाँ) ){
    r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_दायाँ, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }अन्यथा{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, pFree1);
  अगर( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    अगर( व्यंजक_Has_Property(p_व्यंजक->p_दायाँ, EP_Subquery)
     && sqlite3_व्यंजक_Can_Be_Null(p_व्यंजक->p_बायाँ)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_दायाँ, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  वापस addrIsNull;
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
अटल SQLITE_NOINLINE पूर्णांक व्यंजक_गाँठ_Is_Constant_Function(
  Walker *pWalker,
  व्यंजक *p_व्यंजक
){
  पूर्णांक n;             /* Number of arguments */
  व्यंजक_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  निश्चित( p_व्यंजक->op==TK_FUNCTION );
  अगर( व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly)
   || (pList = p_व्यंजक->x.pList)==0
  ){;
    n = 0;
  }अन्यथा{
    n = pList->n_व्यंजक;
    sqlite3_Walk_व्यंजक_List(pWalker, pList);
    अगर( pWalker->eCode==0 ) वापस WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_व्यंजक->u.zToken, n, ENC(db), 0);
  अगर( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    वापस WRC_Abort;
  }
  वापस WRC_Prune;
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
अटल पूर्णांक व्यंजक_गाँठ_Is_Constant(Walker *pWalker, व्यंजक *p_व्यंजक){
  निश्चित( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  अगर( pWalker->eCode==2 && व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) ){
    pWalker->eCode = 0;
    वापस WRC_Abort;
  }

  चयन( p_व्यंजक->op ){
    हाल TK_FUNCTION:
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        वापस WRC_Abort;
      }अन्यथा अगर( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        अगर( pWalker->eCode==5 ) व्यंजक_Set_Property(p_व्यंजक, EP_FromDDL);
        वापस WRC_Continue;
      }अन्यथा अगर( व्यंजक_Has_Property(p_व्यंजक,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        वापस WRC_Continue;
      }अन्यथा अगर( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        वापस व्यंजक_गाँठ_Is_Constant_Function(pWalker, p_व्यंजक);
      }अन्यथा{
        pWalker->eCode = 0;  /* Function call is not constant */
        वापस WRC_Abort;
      }
    हाल TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      अगर( sqlite3_व्यंजक_Id_To_True_False(p_व्यंजक) ){
        वापस WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    हाल TK_COLUMN:
    हाल TK_AGG_FUNCTION:
    हाल TK_AGG_COLUMN:
      testcase( p_व्यंजक->op==TK_ID );
      testcase( p_व्यंजक->op==TK_COLUMN );
      testcase( p_व्यंजक->op==TK_AGG_FUNCTION );
      testcase( p_व्यंजक->op==TK_AGG_COLUMN );
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_FixedCol) && pWalker->eCode!=2 ){
        वापस WRC_Continue;
      }
      अगर( pWalker->eCode==3 && p_व्यंजक->iTable==pWalker->u.iCur ){
        वापस WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    हाल TK_IF_NULL_ROW:
    हाल TK_REGISTER:
    हाल TK_DOT:
    हाल TK_RAISE:
      testcase( p_व्यंजक->op==TK_REGISTER );
      testcase( p_व्यंजक->op==TK_IF_NULL_ROW );
      testcase( p_व्यंजक->op==TK_DOT );
      testcase( p_व्यंजक->op==TK_RAISE );
      pWalker->eCode = 0;
      वापस WRC_Abort;
    हाल TK_IS:
    हाल TK_ISNOT:
      अगर( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        वापस WRC_Abort;
      }
      वापस WRC_Continue;
    हाल TK_VARIABLE:
      अगर( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_व्यंजक->op = TK_NULL;
      }अन्यथा अगर( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        वापस WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    शेष:
      testcase( p_व्यंजक->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_व्यंजक->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      वापस WRC_Continue;
  }
}
अटल पूर्णांक व्यंजक_Is_Const(Parse *pParse, व्यंजक *p, पूर्णांक initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_व्यंजक_Callback = व्यंजक_गाँठ_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#अगर_घोषित SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#पूर्ण_अगर
  sqlite3_Walk_व्यंजक(&w, p);
  वापस w.eCode;
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
पूर्णांक sqlite3_व्यंजक_Is_Constant(Parse *pParse, व्यंजक *p){
  वापस व्यंजक_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
पूर्णांक sqlite3_व्यंजक_List_Is_Constant(Parse *pParse, व्यंजक_List *pList, पूर्णांक bNoIs){
  स्थिर पूर्णांक iInit = bNoIs ? 6 : 1;
  पूर्णांक ii;
  क्रम(ii=0; ii<pList->n_व्यंजक; ii++){
    अगर( 0==व्यंजक_Is_Const(pParse, pList->a[ii].p_व्यंजक, iInit) ) वापस 0;
  }
  वापस 1;
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
अटल पूर्णांक sqlite3_व्यंजक_Is_Constant_Not_Join(Parse *pParse, व्यंजक *p){
  वापस व्यंजक_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
अटल पूर्णांक व्यंजक_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  निश्चित( pSelect!=0 );
  निश्चित( pWalker->eCode==3 || pWalker->eCode==0 );
  अगर( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    वापस WRC_Abort;
  }
  वापस WRC_Prune;
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
पूर्णांक sqlite3_व्यंजक_Is_Table_Constant(व्यंजक *p, पूर्णांक iCur, पूर्णांक bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_व्यंजक_Callback = व्यंजक_गाँठ_Is_Constant;
  अगर( bAllowSubq ){
    w.xSelectCallback = व्यंजक_Select_Walk_Table_Constant;
  }अन्यथा{
    w.xSelectCallback = sqlite3SelectWalkFail;
#अगर_घोषित SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#पूर्ण_अगर
  }
  w.u.iCur = iCur;
  sqlite3_Walk_व्यंजक(&w, p);
  वापस w.eCode;
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
पूर्णांक sqlite3_व्यंजक_Is_Single_Table_Constraint(
  व्यंजक *p_व्यंजक,                 /* The constraint */
  स्थिर SrcList *pSrcList,     /* Complete FROM clause */
  पूर्णांक iSrc,                    /* Which element of pSrcList to use */
  पूर्णांक bAllowSubq               /* Allow non-correlated subqueries */
){
  स्थिर SrcItem *pSrc = &pSrcList->a[iSrc];
  अगर( pSrc->fg.jointype & JT_LTORJ ){
    वापस 0;  /* rule (3) */
  }
  अगर( pSrc->fg.jointype & JT_बायाँ ){
    अगर( !व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) ) वापस 0;   /* rule (4a) */
    अगर( p_व्यंजक->w.iJoin!=pSrc->iCursor ) वापस 0;         /* rule (4b) */
  }अन्यथा{
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) ) वापस 0;    /* rule (5) */
  }
  अगर( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    पूर्णांक jj;
    क्रम(jj=0; jj<iSrc; jj++){
      अगर( p_व्यंजक->w.iJoin==pSrcList->a[jj].iCursor ){
        अगर( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          वापस 0;  /* restriction (6) */
        }
        अवरोध;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  वापस sqlite3_व्यंजक_Is_Table_Constant(p_व्यंजक, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
अटल पूर्णांक व्यंजक_गाँठ_Is_Constant_Or_Group_By(Walker *pWalker, व्यंजक *p_व्यंजक){
  व्यंजक_List *pGroupBy = pWalker->u.pGroupBy;
  पूर्णांक i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  क्रम(i=0; i<pGroupBy->n_व्यंजक; i++){
    व्यंजक *p = pGroupBy->a[i].p_व्यंजक;
    अगर( sqlite3_व्यंजक_Compare(0, p_व्यंजक, p, -1)<2 ){
      CollSeq *pColl = sqlite3_व्यंजक_NNColl_Seq(pWalker->pParse, p);
      अगर( sqlite3_Is_द्विआधारी(pColl) ){
        वापस WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
    pWalker->eCode = 0;
    वापस WRC_Abort;
  }

  वापस व्यंजक_गाँठ_Is_Constant(pWalker, p_व्यंजक);
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
पूर्णांक sqlite3_व्यंजक_Is_Constant_Or_Group_By(Parse *pParse, व्यंजक *p, व्यंजक_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_व्यंजक_Callback = व्यंजक_गाँठ_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_व्यंजक(&w, p);
  वापस w.eCode;
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
पूर्णांक sqlite3_व्यंजक_Is_Constant_Or_Function(व्यंजक *p, u8 isInit){
  निश्चित( isInit==0 || isInit==1 );
  वापस व्यंजक_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
पूर्णांक sqlite3_व्यंजक_Contains_Subquery(व्यंजक *p){
  Walker w;
  w.eCode = 1;
  w.x_व्यंजक_Callback = sqlite3_व्यंजक_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#अगर_घोषित SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#पूर्ण_अगर
  sqlite3_Walk_व्यंजक(&w, p);
  वापस w.eCode==0;
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
पूर्णांक sqlite3_व्यंजक_Is_Integer(स्थिर व्यंजक *p, पूर्णांक *p_मूल्य, Parse *pParse, पूर्णांक bRSI){
  पूर्णांक iSign = 1;   /* Either +1 or -1. */
  जबतक( 1/*exit-by-break*/ ){
    अगर( NEVER(p==0) ) वापस 0;
    अगर( व्यंजक_Use_UValue(p) ){
      निश्चित( p->op==TK_INTEGER );
      *p_मूल्य = p->u.i_मूल्य*iSign;
      अगर( bRSI && pParse==0 && p->op2 ){
        वापस 0;
      }
      वापस 1;
    }
    अगर( p->op==TK_UPLUS ){
      p = p->p_बायाँ;
      pParse = 0;
      जारी;
    }
    अगर( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_बायाँ;
      pParse = 0;
      जारी;
    }
    अगर( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_मूल्य *pVal; /* The variable */
      पूर्णांक rc = 0;          /* 1 if successful, 0 if failed */
      निश्चित( iSign==1 );
      अगर( NEVER(pParse->pVdbe==0) ) अवरोध;
      अगर( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) अवरोध;
      pVal = sqlite3_Vdbe_Get_Bound_मूल्य(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      अगर( pVal ){
        i64 vv;
        अगर( sqlite3_मूल्य_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_मूल्य_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_मूल्य = (पूर्णांक)vv;
          rc = 1;       
        }
        sqlite3_मूल्य_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      वापस rc;
    }
    अवरोध;
  }
  वापस 0;
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
पूर्णांक sqlite3_व्यंजक_Can_Be_Null(स्थिर व्यंजक *p){
  u8 op;
  निश्चित( p!=0 );
  जबतक( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_बायाँ;
    निश्चित( p!=0 );
  }
  op = p->op;
  अगर( op==TK_REGISTER ) op = p->op2;
  चयन( op ){
    हाल TK_INTEGER:
    हाल TK_STRING:
    हाल TK_FLOAT:
    हाल TK_BLOB:
      वापस 0;
    हाल TK_COLUMN:
      निश्चित( व्यंजक_Use_YTab(p) );
      वापस व्यंजक_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#अगर_घोषित SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#पूर्ण_अगर
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    शेष:
      वापस 1;
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
पूर्णांक sqlite3_व्यंजक_Needs_No_Affinity_Change(स्थिर व्यंजक *p, अक्षर aff){
  u8 op;
  पूर्णांक unaryMinus = 0;
  अगर( aff==SQLITE_AFF_BLOB ) वापस 1;
  जबतक( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    अगर( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_बायाँ;
  }
  op = p->op;
  अगर( op==TK_REGISTER ) op = p->op2;
  चयन( op ){
    हाल TK_INTEGER: {
      वापस aff>=SQLITE_AFF_NUMERIC;
    }
    हाल TK_FLOAT: {
      वापस aff>=SQLITE_AFF_NUMERIC;
    }
    हाल TK_STRING: {
      वापस !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    हाल TK_BLOB: {
      वापस !unaryMinus;
    }
    हाल TK_COLUMN: {
      निश्चित( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      वापस aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    शेष: {
      वापस 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
पूर्णांक sqlite3IsRowid(स्थिर अक्षर *z){
  अगर( sqlite3StrICmp(z, "_ROWID_")==0 ) वापस 1;
  अगर( sqlite3StrICmp(z, "ROWID")==0 ) वापस 1;
  अगर( sqlite3StrICmp(z, "OID")==0 ) वापस 1;
  वापस 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
स्थिर अक्षर *sqlite3RowidAlias(Table *pTab){
  स्थिर अक्षर *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  पूर्णांक ii;
  निश्चित( VisibleRowid(pTab) );
  क्रम(ii=0; ii<Array_आकार(azOpt); ii++){
    अगर( sqlite3_Column_सूचकांक(pTab, azOpt[ii])<0 ) वापस azOpt[ii];
  }
  वापस 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
अटल Select *isCandidateForInOpt(स्थिर व्यंजक *pX){
  Select *p;
  SrcList *pSrc;
  व्यंजक_List *pEList;
  Table *pTab;
  पूर्णांक i;
  अगर( !व्यंजक_Use_XSelect(pX) ) वापस 0;                 /* Not a subquery */
  अगर( व्यंजक_Has_Property(pX, EP_VarSelect)  ) वापस 0;  /* Correlated subq */
  p = pX->x.pSelect;
  अगर( p->pPrior ) वापस 0;              /* Not a compound SELECT */
  अगर( p->selFlags & SF_Aggregate ){
    वापस 0; /* No GROUP BY keyword or aggregate functions */
  }
  निश्चित( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  अगर( p->pLimit ) वापस 0;              /* Has no LIMIT clause */
  अगर( p->pWhere ) वापस 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  निश्चित( pSrc!=0 );
  अगर( pSrc->nSrc!=1 ) वापस 0;          /* Single term in FROM clause */
  अगर( pSrc->a[0].fg.isSubquery) वापस 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  निश्चित( pTab!=0 );
  निश्चित( !IsView(pTab)  );              /* FROM clause is not a view */
  अगर( IsVirtual(pTab) ) वापस 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  निश्चित( pEList!=0 );
  /* All SELECT results must be columns. */
  क्रम(i=0; i<pEList->n_व्यंजक; i++){
    व्यंजक *pRes = pEList->a[i].p_व्यंजक;
    अगर( pRes->op!=TK_COLUMN ) वापस 0;
    निश्चित( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  वापस p;
}
#पूर्ण_अगर /* SQLITE_OMIT_SUBQUERY */

#अगर_अघोषित SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
अटल व्योम sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  पूर्णांक iCur,          /* Cursor for the index */
  पूर्णांक regHasNull,    /* Register in which to store hasNull flag */
  पूर्णांक eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  पूर्णांक addr1;
  पूर्णांक op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  अगर( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }अन्यथा{
    op = OP_Last;
  }
  addr1 = sqlite3VdbeAddOp1(v, op, iCur); VdbeCoverage(v);
  sqlite3VdbeAddOp3(v, OP_Column, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
  VdbeComment((v, op==OP_Last?"last_entry_in(%d)":"first_entry_in(%d)", iCur));
  sqlite3VdbeJumpHere(v, addr1);
}
#पूर्ण_अगर


#अगर_अघोषित SQLITE_OMIT_SUBQUERY
/*
** The argument is an IN operator with a list (not a subquery) on the
** right-hand side.  Return TRUE if that list is constant.
*/
अटल पूर्णांक sqlite3InRhsIsConstant(Parse *pParse, व्यंजक *pIn){
  व्यंजक *pLHS;
  पूर्णांक res;
  निश्चित( !व्यंजक_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_बायाँ;
  pIn->p_बायाँ = 0;
  res = sqlite3_व्यंजक_Is_Constant(pParse, pIn);
  pIn->p_बायाँ = pLHS;
  वापस res;
}
#पूर्ण_अगर

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
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
पूर्णांक sqlite3_Find_In_सूचकांक(
  Parse *pParse,             /* Parsing context */
  व्यंजक *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  पूर्णांक *prRhsHasNull,         /* Register holding NULL status.  See notes */
  पूर्णांक *aiMap,                /* Mapping from Index fields to RHS fields */
  पूर्णांक *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  पूर्णांक eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  पूर्णांक iTab;                             /* Cursor of the RHS table */
  पूर्णांक mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  निश्चित( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_सूचकांक_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  अगर( prRhsHasNull && व्यंजक_Use_XSelect(pX) ){
    पूर्णांक i;
    व्यंजक_List *pEList = pX->x.pSelect->pEList;
    क्रम(i=0; i<pEList->n_व्यंजक; i++){
      अगर( sqlite3_व्यंजक_Can_Be_Null(pEList->a[i].p_व्यंजक) ) अवरोध;
    }
    अगर( i==pEList->n_व्यंजक ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  अगर( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    पूर्णांक iDb;                               /* Database idx for pTab */
    व्यंजक_List *pEList = p->pEList;
    पूर्णांक n_व्यंजक = pEList->n_व्यंजक;

    निश्चित( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    निश्चित( p->pEList->a[0].p_व्यंजक!=0 ); /* Because of isCandidateForInOpt(p) */
    निश्चित( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
    निश्चित( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    निश्चित(v);  /* sqlite3GetVdbe() has always been previously called */
    अगर( n_व्यंजक==1 && pEList->a[0].p_व्यंजक->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      पूर्णांक iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_सूचकांक_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }अन्यथा{
      सूचकांक *pIdx;                         /* Iterator variable */
      पूर्णांक affinity_ok = 1;
      पूर्णांक i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      क्रम(i=0; i<n_व्यंजक && affinity_ok; i++){
        व्यंजक *pLhs = sqlite3VectorFieldSubexpr(pX->p_बायाँ, i);
        पूर्णांक iCol = pEList->a[i].p_व्यंजक->iColumn;
        अक्षर idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        अक्षर cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        चयन( cmpaff ){
          हाल SQLITE_AFF_BLOB:
            अवरोध;
          हाल SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            निश्चित( idxaff==SQLITE_AFF_TEXT );
            अवरोध;
          शेष:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      अगर( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        क्रम(pIdx=pTab->p_सूचकांक; pIdx && eType==0; pIdx=pIdx->p_अग्र){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          अगर( pIdx->nColumn<n_व्यंजक ) जारी;
          अगर( pIdx->pPartIdxWhere!=0 ) जारी;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          अगर( pIdx->nColumn>=BMS-1 ) जारी;
          अगर( mustBeUnique ){
            अगर( pIdx->n_कुंजी_Col>n_व्यंजक
             ||(pIdx->nColumn>n_व्यंजक && !Is_Unique_सूचकांक(pIdx))
            ){
              जारी;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          क्रम(i=0; i<n_व्यंजक; i++){
            व्यंजक *pLhs = sqlite3VectorFieldSubexpr(pX->p_बायाँ, i);
            व्यंजक *pRhs = pEList->a[i].p_व्यंजक;
            CollSeq *pReq = sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, pLhs, pRhs);
            पूर्णांक j;
 
            क्रम(j=0; j<n_व्यंजक; j++){
              अगर( pIdx->aiColumn[j]!=pRhs->iColumn ) जारी;
              निश्चित( pIdx->azColl[j] );
              अगर( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                जारी;
              }
              अवरोध;
            }
            अगर( j==n_व्यंजक ) अवरोध;
            mCol = MASKBIT(j);
            अगर( mCol & colUsed ) अवरोध; /* Each column used only once */
            colUsed |= mCol;
            अगर( aiMap ) aiMap[i] = j;
          }
 
          निश्चित( n_व्यंजक>0 && n_व्यंजक<BMS );
          निश्चित( i==n_व्यंजक || colUsed!=(MASKBIT(n_व्यंजक)-1) );
          अगर( colUsed==(MASKBIT(n_व्यंजक)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            पूर्णांक iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            निश्चित( IN_सूचकांक_सूचकांक_DESC == IN_सूचकांक_सूचकांक_ASC+1 );
            eType = IN_सूचकांक_सूचकांक_ASC + pIdx->aSortOrder[0];
 
            अगर( prRhsHasNull ){
#अगर_घोषित SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_व्यंजक)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#पूर्ण_अगर
              *prRhsHasNull = ++pParse->nMem;
              अगर( n_व्यंजक==1 ){
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
  अगर( eType==0
   && (inFlags & IN_सूचकांक_NOOP_OK)
   && व्यंजक_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_व्यंजक<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_सूचकांक_NOOP;
  }

  अगर( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    पूर्णांक rMayHaveNull = 0;
    पूर्णांक bloomOk = (inFlags & IN_सूचकांक_MEMBERSHIP)!=0;
    eType = IN_सूचकांक_EPH;
    अगर( inFlags & IN_सूचकांक_LOOP ){
      pParse->nQueryLoop = 0;
    }अन्यथा अगर( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    निश्चित( pX->op==TK_IN );
    अगर( !bloomOk
     && व्यंजक_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    अगर( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  अगर( aiMap && eType!=IN_सूचकांक_सूचकांक_ASC && eType!=IN_सूचकांक_सूचकांक_DESC ){
    पूर्णांक i, n;
    n = sqlite3_व्यंजक_Vector_आकार(pX->p_बायाँ);
    क्रम(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  वापस eType;
}
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_SUBQUERY
/*
** Argument pExpr is an (?, ?...) IN(...) expression. This
** function allocates and returns a nul-terminated string containing
** the affinities to be used for each column of the comparison.
**
** It is the responsibility of the caller to ensure that the returned
** string is eventually freed using sqlite3DbFree().
*/
अटल अक्षर *व्यंजक_INAffinity(Parse *pParse, स्थिर व्यंजक *p_व्यंजक){
  व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
  पूर्णांक nVal = sqlite3_व्यंजक_Vector_आकार(p_बायाँ);
  Select *pSelect = व्यंजक_Use_XSelect(p_व्यंजक) ? p_व्यंजक->x.pSelect : 0;
  अक्षर *zRet;

  निश्चित( p_व्यंजक->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  अगर( zRet ){
    पूर्णांक i;
    क्रम(i=0; i<nVal; i++){
      व्यंजक *pA = sqlite3VectorFieldSubexpr(p_बायाँ, i);
      अक्षर a = sqlite3_व्यंजक_Affinity(pA);
      अगर( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_व्यंजक, a);
      }अन्यथा{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  वापस zRet;
}
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
व्योम sqlite3SubselectError(Parse *pParse, पूर्णांक nActual, पूर्णांक nExpect){
  अगर( pParse->nErr==0 ){
    स्थिर अक्षर *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_संदेश(pParse, zFmt, nActual, nExpect);
  }
}
#पूर्ण_अगर

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
व्योम sqlite3_Vector_Error_संदेश(Parse *pParse, व्यंजक *p_व्यंजक){
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
  अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
    sqlite3SubselectError(pParse, p_व्यंजक->x.pSelect->pEList->n_व्यंजक, 1);
  }अन्यथा
#पूर्ण_अगर
  {
    sqlite3_Error_संदेश(pParse, "row value misused");
  }
}

#अगर_अघोषित SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
अटल पूर्णांक findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  व्यंजक *p_व्यंजक,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  अगर( pNewSig==0 ) वापस 0;
  अगर( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) वापस 0;
  निश्चित( p_व्यंजक->op==TK_IN );
  निश्चित( !व्यंजक_Use_YSub(p_व्यंजक) );
  निश्चित( व्यंजक_Use_XSelect(p_व्यंजक) );
  निश्चित( p_व्यंजक->x.pSelect!=0 );
  निश्चित( (p_व्यंजक->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  निश्चित( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  क्रम(; pOp<pEnd; pOp++){
    अगर( pOp->p4type!=P4_SUBRTNSIG ) जारी;
    निश्चित( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    निश्चित( pSig!=0 );
    अगर( !pSig->bComplete ) जारी;
    अगर( pNewSig->selId!=pSig->selId ) जारी;
    अगर( म_भेद(pNewSig->zAff,pSig->zAff)!=0 ) जारी;
    p_व्यंजक->y.sub.iAddr = pSig->iAddr;
    p_व्यंजक->y.sub.regReturn = pSig->regReturn;
    p_व्यंजक->iTable = pSig->iTable;
    व्यंजक_Set_Property(p_व्यंजक, EP_Subrtn);
    वापस 1;
  }
  वापस 0;
}
#पूर्ण_अगर /* SQLITE_OMIT_SUBQUERY */

#अगर_अघोषित SQLITE_OMIT_SUBQUERY
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
व्योम sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  व्यंजक *p_व्यंजक,            /* The IN operator */
  पूर्णांक iTab,               /* Use this cursor number */
  पूर्णांक allowBloom          /* True to allow the use of a Bloom filter */
){
  पूर्णांक addrOnce = 0;           /* Address of the OP_Once instruction at top */
  पूर्णांक addr;                   /* Address of OP_OpenEphemeral instruction */
  व्यंजक *p_बायाँ;                /* the LHS of the IN operator */
  कुंजी_Info *p_कुंजी_Info = 0;      /* Key information */
  पूर्णांक nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  निश्चित( v!=0 );

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
  अगर( !व्यंजक_Has_Property(p_व्यंजक, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    निश्चित( !व्यंजक_Use_XSelect(p_व्यंजक) || p_व्यंजक->x.pSelect!=0 );
    अगर( व्यंजक_Use_XSelect(p_व्यंजक) && (p_व्यंजक->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, माप(pSig[0]));
      अगर( pSig ){
        pSig->selId = p_व्यंजक->x.pSelect->selId;
        pSig->zAff = व्यंजक_INAffinity(pParse, p_व्यंजक);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_व्यंजक, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_व्यंजक->x.pSelect->selId));
      }
      निश्चित( व्यंजक_Use_YSub(p_व्यंजक) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_व्यंजक->y.sub.regReturn,
                        p_व्यंजक->y.sub.iAddr);
      निश्चित( iTab!=p_व्यंजक->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_व्यंजक->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      अगर( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      वापस;
    }

    /* Begin coding the subroutine */
    निश्चित( !व्यंजक_Use_YWin(p_व्यंजक) );
    व्यंजक_Set_Property(p_व्यंजक, EP_Subrtn);
    निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly|EP_Reduced) );
    p_व्यंजक->y.sub.regReturn = ++pParse->nMem;
    p_व्यंजक->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_व्यंजक->y.sub.regReturn) + 1;
    अगर( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_व्यंजक->y.sub.iAddr;
      pSig->regReturn = p_व्यंजक->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (स्थिर अक्षर*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_बायाँ = p_व्यंजक->p_बायाँ;
  nVal = sqlite3_व्यंजक_Vector_आकार(p_बायाँ);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_व्यंजक->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_व्यंजक->iTable, nVal);
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
  अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
    VdbeComment((v, "Result of SELECT %u", p_व्यंजक->x.pSelect->selId));
  }अन्यथा{
    VdbeComment((v, "RHS of IN operator"));
  }
#पूर्ण_अगर
  p_कुंजी_Info = sqlite3_कुंजी_Info_Alloc(pParse->db, nVal, 1);
  निश्चित( p_कुंजी_Info!=0 || pParse->nErr );
  अगर( p_कुंजी_Info==0 ) वापस;

  अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_व्यंजक->x.pSelect;
    व्यंजक_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    अगर( ALWAYS(pEList->n_व्यंजक==nVal) ){
      Select *pCopy;
      SelectDest dest;
      पूर्णांक i;
      पूर्णांक rc;
      पूर्णांक addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = व्यंजक_INAffinity(pParse, p_व्यंजक);
      pSelect->iLimit = 0;
      निश्चित( pEList!=0 );
      निश्चित( pEList->n_व्यंजक>0 );
      निश्चित( sqlite3_कुंजी_Info_Is_Writeable(p_कुंजी_Info) );
      क्रम(i=0; i<nVal; i++){
        व्यंजक *p = sqlite3VectorFieldSubexpr(p_बायाँ, i);
        CollSeq *pColl;
        p_कुंजी_Info->aColl[i] = pColl = sqlite3_द्विआधारी_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_व्यंजक
        );
        अगर( !sqlite3_Is_द्विआधारी(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      अगर( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        पूर्णांक regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (व्योम *)p_कुंजी_Info, P4_KEYINFO);
        p_कुंजी_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      अगर( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        अगर( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      अगर( rc ){
        sqlite3_कुंजी_Info_Unref(p_कुंजी_Info);
        वापस;
      }
    }
  }अन्यथा अगर( ALWAYS(p_व्यंजक->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    अक्षर affinity;            /* Affinity of the LHS of the IN */
    पूर्णांक i;
    व्यंजक_List *pList = p_व्यंजक->x.pList;
    काष्ठा व्यंजक_List_item *pItem;
    पूर्णांक r1, r2;
    affinity = sqlite3_व्यंजक_Affinity(p_बायाँ);
    अगर( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }अन्यथा अगर( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    निश्चित( p_कुंजी_Info!=0 );
    निश्चित( sqlite3_कुंजी_Info_Is_Writeable(p_कुंजी_Info) );
    p_कुंजी_Info->aColl[0] = sqlite3_व्यंजक_Coll_Seq(pParse, p_व्यंजक->p_बायाँ);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3_Get_अस्थायी_Reg(pParse);
    r2 = sqlite3_Get_अस्थायी_Reg(pParse);
    क्रम(i=pList->n_व्यंजक, pItem=pList->a; i>0; i--, pItem++){
      व्यंजक *pE2 = pItem->p_व्यंजक;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      अगर( addrOnce && !sqlite3_व्यंजक_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        व्यंजक_Clear_Property(p_व्यंजक, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_व्यंजक_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_जोड़ो, iTab, r2, r1, 1);
    }
    sqlite3_Release_अस्थायी_Reg(pParse, r1);
    sqlite3_Release_अस्थायी_Reg(pParse, r2);
  }
  अगर( pSig ) pSig->bComplete = 1;
  अगर( p_कुंजी_Info ){
    sqlite3VdbeChangeP4(v, addr, (व्योम *)p_कुंजी_Info, P4_KEYINFO);
  }
  अगर( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    निश्चित( व्यंजक_Use_YSub(p_व्यंजक) );
    निश्चित( sqlite3VdbeGetOp(v,p_व्यंजक->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_व्यंजक->y.sub.regReturn,
                      p_व्यंजक->y.sub.iAddr, 1);
    VdbeCoverage(v);
    sqlite3_Clear_अस्थायी_Reg_Cache(pParse);
  }
}
#पूर्ण_अगर /* SQLITE_OMIT_SUBQUERY */

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
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
पूर्णांक sqlite3CodeSubselect(Parse *pParse, व्यंजक *p_व्यंजक){
  पूर्णांक addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  पूर्णांक rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  पूर्णांक nReg;                   /* Registers to allocate */
  व्यंजक *pLimit;               /* New limit expression */
#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णांक addrExplain;            /* Address of OP_Explain instruction */
#पूर्ण_अगर

  Vdbe *v = pParse->pVdbe;
  निश्चित( v!=0 );
  अगर( pParse->nErr ) वापस 0;
  testcase( p_व्यंजक->op==TK_EXISTS );
  testcase( p_व्यंजक->op==TK_SELECT );
  निश्चित( p_व्यंजक->op==TK_EXISTS || p_व्यंजक->op==TK_SELECT );
  निश्चित( व्यंजक_Use_XSelect(p_व्यंजक) );
  pSel = p_व्यंजक->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    निश्चित( व्यंजक_Use_YSub(p_व्यंजक) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_व्यंजक->y.sub.regReturn,
                      p_व्यंजक->y.sub.iAddr);
    वापस p_व्यंजक->iTable;
  }

  /* Begin coding the subroutine */
  निश्चित( !व्यंजक_Use_YWin(p_व्यंजक) );
  निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Reduced|EP_TokenOnly) );
  व्यंजक_Set_Property(p_व्यंजक, EP_Subrtn);
  p_व्यंजक->y.sub.regReturn = ++pParse->nMem;
  p_व्यंजक->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_व्यंजक->y.sub.regReturn) + 1;

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
  अगर( !व्यंजक_Has_Property(p_व्यंजक, EP_VarSelect) ){
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
  nReg = p_व्यंजक->op==TK_SELECT ? pSel->pEList->n_व्यंजक : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  अगर( p_व्यंजक->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    अगर( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_दायाँ ){
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
    }अन्यथा{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }अन्यथा{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  अगर( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    व्यंजक *p_बायाँ = pSel->pLimit->p_बायाँ;
    अगर( व्यंजक_Has_Property(p_बायाँ, EP_Int_मूल्य)==0
     || (p_बायाँ->u.i_मूल्य!=1 && p_बायाँ->u.i_मूल्य!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_व्यंजक_Int32(db, 0);
      अगर( pLimit ){
        pLimit->aff_व्यंजक = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_व्यंजक_Dup(db, p_बायाँ, 0), pLimit);
      }
      sqlite3_व्यंजक_Deferred_Delete(pParse, p_बायाँ);
      pSel->pLimit->p_बायाँ = pLimit;
    }
  }अन्यथा{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_व्यंजक_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  अगर( sqlite3Select(pParse, pSel, &dest) ){
    p_व्यंजक->op2 = p_व्यंजक->op;
    p_व्यंजक->op = TK_ERROR;
    वापस 0;
  }
  p_व्यंजक->iTable = rReg = dest.iSDParm;
  व्यंजक_Set_VVAProperty(p_व्यंजक, EP_NoReduce);
  अगर( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  अगर( dest.eDest==SRT_Mem 
   && sqlite3_व्यंजक_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_व्यंजक) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  निश्चित( व्यंजक_Use_YSub(p_व्यंजक) );
  निश्चित( sqlite3VdbeGetOp(v,p_व्यंजक->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_व्यंजक->y.sub.regReturn,
                    p_व्यंजक->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3_Clear_अस्थायी_Reg_Cache(pParse);
  वापस rReg;
}
#पूर्ण_अगर /* SQLITE_OMIT_SUBQUERY */

#अगर_अघोषित SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
पूर्णांक sqlite3_व्यंजक_Check_IN(Parse *pParse, व्यंजक *pIn){
  पूर्णांक nVector = sqlite3_व्यंजक_Vector_आकार(pIn->p_बायाँ);
  अगर( व्यंजक_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    अगर( nVector!=pIn->x.pSelect->pEList->n_व्यंजक ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_व्यंजक, nVector);
      वापस 1;
    }
  }अन्यथा अगर( nVector!=1 ){
    sqlite3_Vector_Error_संदेश(pParse, pIn->p_बायाँ);
    वापस 1;
  }
  वापस 0;
}
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_SUBQUERY
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
अटल व्योम sqlite3_व्यंजक_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  व्यंजक *p_व्यंजक,          /* The IN expression */
  पूर्णांक destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  पूर्णांक destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  पूर्णांक rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  पूर्णांक eType;            /* Type of the RHS */
  पूर्णांक rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  पूर्णांक *aiMap = 0;       /* Map from vector field to index column */
  अक्षर *zAff = 0;       /* Affinity string for comparisons */
  पूर्णांक nVector;          /* Size of vectors for this IN operator */
  पूर्णांक iDummy;           /* Dummy parameter to exprCodeVector() */
  व्यंजक *p_बायाँ;          /* The LHS of the IN operator */
  पूर्णांक i;                /* loop counter */
  पूर्णांक destStep2;        /* Where to jump when NULLs seen in step 2 */
  पूर्णांक destStep6 = 0;    /* Start of code for Step 6 */
  पूर्णांक addrTruthOp;      /* Address of opcode that determines the IN is true */
  पूर्णांक destNotNull;      /* Jump here if a comparison is not true in step 6 */
  पूर्णांक addr_शीर्ष;          /* Top of the step-6 loop */
  पूर्णांक iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  निश्चित( !व्यंजक_Has_VVAProperty(p_व्यंजक,EP_Immutable) );
  p_बायाँ = p_व्यंजक->p_बायाँ;
  अगर( sqlite3_व्यंजक_Check_IN(pParse, p_व्यंजक) ) वापस;
  zAff = व्यंजक_INAffinity(pParse, p_व्यंजक);
  nVector = sqlite3_व्यंजक_Vector_आकार(p_व्यंजक->p_बायाँ);
  aiMap = (पूर्णांक*)sqlite3DbMallocZero(pParse->db, nVector*माप(पूर्णांक));
  अगर( pParse->db->mallocFailed ) जाओ sqlite3_व्यंजक_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  निश्चित( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3_Find_In_सूचकांक(pParse, p_व्यंजक,
                             IN_सूचकांक_MEMBERSHIP | IN_सूचकांक_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  निश्चित( pParse->nErr || nVector==1 || eType==IN_सूचकांक_EPH
       || eType==IN_सूचकांक_सूचकांक_ASC || eType==IN_सूचकांक_सूचकांक_DESC
  );
#अगर_घोषित SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  क्रम(i=0; i<nVector; i++){
    पूर्णांक j, cnt;
    क्रम(cnt=j=0; j<nVector; j++) अगर( aiMap[j]==i ) cnt++;
    निश्चित( cnt==1 );
  }
#पूर्ण_अगर

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
  निश्चित( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = व्यंजक_Code_Vector(pParse, p_बायाँ, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  अगर( eType==IN_सूचकांक_NOOP ){
    व्यंजक_List *pList;
    CollSeq *pColl;
    पूर्णांक labelOk = sqlite3VdbeMakeLabel(pParse);
    पूर्णांक r2, regToFree;
    पूर्णांक regCkNull = 0;
    पूर्णांक ii;
    निश्चित( nVector==1 );
    निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
    pList = p_व्यंजक->x.pList;
    pColl = sqlite3_व्यंजक_Coll_Seq(pParse, p_व्यंजक->p_बायाँ);
    अगर( destIfNull!=destIfFalse ){
      regCkNull = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    क्रम(ii=0; ii<pList->n_व्यंजक; ii++){
      r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, pList->a[ii].p_व्यंजक, &regToFree);
      अगर( regCkNull && sqlite3_व्यंजक_Can_Be_Null(pList->a[ii].p_व्यंजक) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3_Release_अस्थायी_Reg(pParse, regToFree);
      अगर( ii<pList->n_व्यंजक-1 || destIfNull!=destIfFalse ){
        पूर्णांक op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (व्योम*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_व्यंजक-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_व्यंजक-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_व्यंजक-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_व्यंजक-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }अन्यथा{
        पूर्णांक op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        निश्चित( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (व्योम*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    अगर( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3_Release_अस्थायी_Reg(pParse, regCkNull);
    जाओ sqlite3_व्यंजक_Code_IN_finished;
  }

  अगर( eType!=IN_सूचकांक_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    अगर( nVector==1 ){
      अक्षर aff = zAff[0];
      अगर( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_व्यंजक_Affinity(p_बायाँ) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        पूर्णांक rTmp = sqlite3_Get_अस्थायी_Reg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }अन्यथा{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    क्रम(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    अगर( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      पूर्णांक rLhsOrig = rLhs;
      rLhs = sqlite3_Get_अस्थायी_Range(pParse, nVector);
      क्रम(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  अगर( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }अन्यथा{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  क्रम(i=0; i<nVector; i++){
    व्यंजक *p = sqlite3VectorFieldSubexpr(p_व्यंजक->p_बायाँ, i);
    अगर( pParse->nErr ) जाओ sqlite3_व्यंजक_Code_IN_oom_error;
    अगर( sqlite3_व्यंजक_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  अगर( eType==IN_सूचकांक_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    निश्चित( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }अन्यथा{
    अगर( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Subrtn) ){
        स्थिर VdbeOp *pOp = sqlite3VdbeGetOp(v, p_व्यंजक->y.sub.iAddr);
        निश्चित( pOp->opcode==OP_Once || pParse->nErr );
        अगर( pOp->p3>0 ){  /* tag-202407032019 */
          निश्चित( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      जाओ sqlite3_व्यंजक_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  अगर( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  अगर( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  अगर( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  अगर( eType==IN_सूचकांक_सूचकांक_DESC ){
    addr_शीर्ष = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }अन्यथा{
    testcase( eType==IN_सूचकांक_EPH );
    testcase( eType==IN_सूचकांक_ROWID );
    addr_शीर्ष = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  अगर( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }अन्यथा{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  क्रम(i=0; i<nVector; i++){
    व्यंजक *p;
    CollSeq *pColl;
    पूर्णांक r3 = sqlite3_Get_अस्थायी_Reg(pParse);
    p = sqlite3VectorFieldSubexpr(p_बायाँ, i);
    अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
      व्यंजक *pRhs = p_व्यंजक->x.pSelect->pEList->a[i].p_व्यंजक;
      pColl = sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, p, pRhs);
    }अन्यथा{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_व्यंजक_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (व्योम*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3_Release_अस्थायी_Reg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  अगर( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_अग्र, iTab, addr_शीर्ष+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_व्यंजक_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_व्यंजक_Code_IN_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}
#पूर्ण_अगर /* SQLITE_OMIT_SUBQUERY */

#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
अटल व्योम codeReal(Vdbe *v, स्थिर अक्षर *z, पूर्णांक negateFlag, पूर्णांक iMem){
  अगर( ALWAYS(z!=0) ){
    द्विगुन मूल्य;
    sqlite3AtoF(z, &मूल्य);
    निश्चित( !sqlite3IsNaN(मूल्य) ); /* The new AtoF never returns NaN */
    अगर( negateFlag ) मूल्य = -मूल्य;
    sqlite3VdbeAddDouble(v, iMem, मूल्य);
  }
}
#पूर्ण_अगर


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
अटल व्योम codeInteger(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक negFlag, पूर्णांक iMem){
  Vdbe *v = pParse->pVdbe;
  अगर( p_व्यंजक->flags & EP_Int_मूल्य ){
    पूर्णांक i = p_व्यंजक->u.i_मूल्य;
    निश्चित( i>=0 );
    अगर( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }अन्यथा{
    पूर्णांक c;
    i64 मूल्य;
    स्थिर अक्षर *z = p_व्यंजक->u.zToken;
    निश्चित( z!=0 );
    c = sqlite3DecOrHexToI64(z, &मूल्य);
    अगर( (c==3 && !negFlag) || (c==2) || (negFlag && मूल्य==SMALLEST_INT64)){
#अगर_घोषित SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_संदेश(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_व्यंजक);
#अन्यथा
#अगर_अघोषित SQLITE_OMIT_HEX_INTEGER
      अगर( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_संदेश(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_व्यंजक);
      }अन्यथा
#पूर्ण_अगर
      {
        codeReal(v, z, negFlag, iMem);
      }
#पूर्ण_अगर
    }अन्यथा{
      अगर( negFlag ){ मूल्य = c==3 ? SMALLEST_INT64 : -मूल्य; }
      sqlite3VdbeAddInt64(v, iMem, मूल्य);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
व्योम sqlite3_व्यंजक_Code_Load_सूचकांक_Column(
  Parse *pParse,  /* The parsing context */
  सूचकांक *pIdx,    /* The index whose column is to be loaded */
  पूर्णांक iTabCur,    /* Cursor pointing to a table row */
  पूर्णांक iIdxCol,    /* The column of the index to be loaded */
  पूर्णांक regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  अगर( iTabCol==XN_व्यंजक ){
    निश्चित( pIdx->a_Col_व्यंजक );
    निश्चित( pIdx->a_Col_व्यंजक->n_व्यंजक>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_व्यंजक_Code_Copy(pParse, pIdx->a_Col_व्यंजक->a[iIdxCol].p_व्यंजक, regOut);
    pParse->iSelfTab = 0;
  }अन्यथा{
    sqlite3_व्यंजक_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
व्योम sqlite3_व्यंजक_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  पूर्णांक regOut         /* Put the result in this register */
){
  पूर्णांक iAddr;
  Vdbe *v = pParse->pVdbe;
  पूर्णांक nErr = pParse->nErr;
  निश्चित( v!=0 );
  निश्चित( pParse->iSelfTab!=0 );
  अगर( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }अन्यथा{
    iAddr = 0;
  }
  sqlite3_व्यंजक_Code_Copy(pParse, sqlite3_Column_व्यंजक(pTab,pCol), regOut);
  अगर( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    पूर्णांक p3 = 2+(पूर्णांक)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (अक्षर*)pTab, P4_TABLE);
  }अन्यथा अगर( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  अगर( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  अगर( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#पूर्ण_अगर /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
व्योम sqlite3_व्यंजक_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  पूर्णांक iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  पूर्णांक iCol,       /* Index of the column to extract */
  पूर्णांक regOut      /* Extract the value into this register */
){
  Column *pCol;
  निश्चित( v!=0 );
  निश्चित( pTab!=0 );
  निश्चित( iCol!=XN_व्यंजक );
  अगर( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }अन्यथा{
    पूर्णांक x;
    अगर( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      वापस;
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
    }अन्यथा अगर( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      अगर( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_संदेश(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }अन्यथा{
        पूर्णांक savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_व्यंजक_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      वापस;
#पूर्ण_अगर
    }अन्यथा अगर( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3_Table_Column_To_सूचकांक(sqlite3_Primary_कुंजी_सूचकांक(pTab), iCol);
    }अन्यथा{
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
पूर्णांक sqlite3_व्यंजक_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  पूर्णांक iColumn,     /* Index of the table column */
  पूर्णांक iTable,      /* The cursor pointing to the table */
  पूर्णांक iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  निश्चित( pParse->pVdbe!=0 );
  निश्चित( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  निश्चित( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_व्यंजक_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  अगर( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    अगर( pOp->opcode==OP_Column ) pOp->p5 = p5;
    अगर( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  वापस iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
व्योम sqlite3_व्यंजक_Code_Move(Parse *pParse, पूर्णांक iFrom, पूर्णांक iTo, पूर्णांक nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
व्योम sqlite3_व्यंजक_To_Register(व्यंजक *p_व्यंजक, पूर्णांक iReg){
  व्यंजक *p = sqlite3_व्यंजक_Skip_Collate_And_Likely(p_व्यंजक);
  अगर( NEVER(p==0) ) वापस;
  अगर( p->op==TK_REGISTER ){
    निश्चित( p->iTable==iReg );
  }अन्यथा{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    व्यंजक_Clear_Property(p, EP_Skip);
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
अटल पूर्णांक व्यंजक_Code_Vector(Parse *pParse, व्यंजक *p, पूर्णांक *piFreeable){
  पूर्णांक i_परिणाम;
  पूर्णांक n_परिणाम = sqlite3_व्यंजक_Vector_आकार(p);
  अगर( n_परिणाम==1 ){
    i_परिणाम = sqlite3_व्यंजक_Code_अस्थायी(pParse, p, piFreeable);
  }अन्यथा{
    *piFreeable = 0;
    अगर( p->op==TK_SELECT ){
#अगर SQLITE_OMIT_SUBQUERY
      i_परिणाम = 0;
#अन्यथा
      i_परिणाम = sqlite3CodeSubselect(pParse, p);
#पूर्ण_अगर
    }अन्यथा{
      पूर्णांक i;
      i_परिणाम = pParse->nMem+1;
      pParse->nMem += n_परिणाम;
      निश्चित( व्यंजक_Use_XList(p) );
      क्रम(i=0; i<n_परिणाम; i++){
        sqlite3_व्यंजक_Code_Factorable(pParse, p->x.pList->a[i].p_व्यंजक, i+i_परिणाम);
      }
    }
  }
  वापस i_परिणाम;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
अटल व्योम setDoNotMergeFlagOnCopy(Vdbe *v){
  अगर( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
अटल पूर्णांक व्यंजक_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  व्यंजक_List *pFarg,      /* List of function arguments */
  पूर्णांक iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  पूर्णांक target            /* Store function result in this register */
){
  पूर्णांक nFarg;
  Vdbe *v = pParse->pVdbe;
  निश्चित( v!=0 );
  निश्चित( pFarg!=0 );
  nFarg = pFarg->n_व्यंजक;
  निश्चित( nFarg>0 );  /* All in-line functions have at least one argument */
  चयन( iFuncId ){
    हाल INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      पूर्णांक endCoalesce = sqlite3VdbeMakeLabel(pParse);
      पूर्णांक i;
      निश्चित( nFarg>=2 );
      sqlite3_व्यंजक_Code(pParse, pFarg->a[0].p_व्यंजक, target);
      क्रम(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_व्यंजक_Code(pParse, pFarg->a[i].p_व्यंजक, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      अवरोध;
    }
    हाल INLINEFUNC_iif: {
      व्यंजक case_व्यंजक;
      स_रखो(&case_व्यंजक, 0, माप(case_व्यंजक));
      case_व्यंजक.op = TK_CASE;
      case_व्यंजक.x.pList = pFarg;
      वापस sqlite3_व्यंजक_Code_Target(pParse, &case_व्यंजक, target);
    }
#अगर_घोषित SQLITE_ENABLE_OFFSET_SQL_FUNC
    हाल INLINEFUNC_sqlite_offset: {
      व्यंजक *pArg = pFarg->a[0].p_व्यंजक;
      अगर( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      अवरोध;
    }
#पूर्ण_अगर
    शेष: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      निश्चित( nFarg==1 || nFarg==2 );
      target = sqlite3_व्यंजक_Code_Target(pParse, pFarg->a[0].p_व्यंजक, target);
      अवरोध;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#अगर !defined(SQLITE_UNTESTABLE)
    हाल INLINEFUNC_व्यंजक_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      निश्चित( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_व्यंजक_Compare(0,pFarg->a[0].p_व्यंजक, pFarg->a[1].p_व्यंजक,-1),
         target);
      अवरोध;
    }

    हाल INLINEFUNC_व्यंजक_implies_व्यंजक: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      निश्चित( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_व्यंजक_Implies_व्यंजक(pParse,pFarg->a[0].p_व्यंजक, pFarg->a[1].p_व्यंजक,-1),
         target);
      अवरोध;
    }

    हाल INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      व्यंजक *pA1;
      निश्चित( nFarg==2 );
      pA1 = pFarg->a[1].p_व्यंजक;
      अगर( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_व्यंजक_Implies_Non_Null_Row(pFarg->a[0].p_व्यंजक,pA1->iTable,1),
           target);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      अवरोध;
    }

    हाल INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      स्थिर अक्षर *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      अक्षर aff;
      निश्चित( nFarg==1 );
      aff = sqlite3_व्यंजक_Affinity(pFarg->a[0].p_व्यंजक);
      निश्चित( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      अवरोध;
    }
#पूर्ण_अगर /* !defined(SQLITE_UNTESTABLE) */
  }
  वापस target;
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
अटल पूर्णांक व्यंजक_गाँठ_Can_Return_Subtype(Walker *pWalker, व्यंजक *p_व्यंजक){
  पूर्णांक n;
  FuncDef *pDef;
  sqlite3 *db;
  अगर( p_व्यंजक->op==TK_CASE || p_व्यंजक->op==TK_UPLUS 
   || p_व्यंजक->op==TK_COLLATE || p_व्यंजक->op==TK_CAST 
  ){
    वापस WRC_Continue;
  }
  अगर( p_व्यंजक->op!=TK_FUNCTION && p_व्यंजक->op!=TK_AGG_FUNCTION ){
    वापस WRC_Prune;
  }
  निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
  db = pWalker->pParse->db;
  n = p_व्यंजक->x.pList ? p_व्यंजक->x.pList->n_व्यंजक : 0;
  pDef = sqlite3FindFunction(db, p_व्यंजक->u.zToken, n, ENC(db), 0);
#अगर_घोषित SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  अगर( pDef==0 ) वापस WRC_Continue;
#अन्यथा
  निश्चित( pDef!=0 );
#पूर्ण_अगर
  अगर( (pDef->funcFlags & SQLITE_परिणाम_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    वापस WRC_Abort;
  }
  वापस WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
पूर्णांक sqlite3_व्यंजक_Can_Return_Subtype(Parse *pParse, व्यंजक *p_व्यंजक){
  Walker w;
  स_रखो(&w, 0, माप(w));
  w.pParse = pParse;
  w.x_व्यंजक_Callback = व्यंजक_गाँठ_Can_Return_Subtype;
  sqlite3_Walk_व्यंजक(&w, p_व्यंजक);
  वापस w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
अटल SQLITE_NOINLINE पूर्णांक sqlite3_Indexed_व्यंजक_Lookup(
  Parse *pParse,   /* The parsing context */
  व्यंजक *p_व्यंजक,     /* The expression to potentially bypass */
  पूर्णांक target       /* Where to store the result of the expression */
){
  Indexed_व्यंजक *p;
  Vdbe *v;
  क्रम(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 व्यंजक_Aff;
    पूर्णांक i_तथ्य_Cur = p->i_तथ्य_Cur;
    अगर( i_तथ्य_Cur<0 ) जारी;
    अगर( pParse->iSelfTab ){
      अगर( p->i_तथ्य_Cur!=pParse->iSelfTab-1 ) जारी;
      i_तथ्य_Cur = -1;
    }
    अगर( sqlite3_व्यंजक_Compare(0, p_व्यंजक, p->p_व्यंजक, i_तथ्य_Cur)!=0 ) जारी;
    निश्चित( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    व्यंजक_Aff = sqlite3_व्यंजक_Affinity(p_व्यंजक);
    अगर( (व्यंजक_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (व्यंजक_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (व्यंजक_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      जारी;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_SubtArg)
     && sqlite3_व्यंजक_Can_Return_Subtype(pParse, p_व्यंजक) 
    ){
      जारी;
    }

    v = pParse->pVdbe;
    निश्चित( v!=0 );
    अगर( p->bMaybeNullRow || sqlite3_व्यंजक_Is_Constant(pParse, p_व्यंजक) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      पूर्णांक addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_व्यंजक_Code(pParse, p_व्यंजक, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }अन्यथा{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    वापस target;
  }
  वापस -1;  /* Not found */
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
अटल पूर्णांक व्यंजक_Partidx_व्यंजक_Lookup(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक iTarget){
  Indexed_व्यंजक *p;
  क्रम(p=pParse->p_Idx_Part_व्यंजक; p; p=p->pIENext){
    अगर( p_व्यंजक->iColumn==p->iIdxCol && p_व्यंजक->iTable==p->i_तथ्य_Cur ){
      Vdbe *v = pParse->pVdbe;
      पूर्णांक addr = 0;
      पूर्णांक ret;

      अगर( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_व्यंजक_Code_Target(pParse, p->p_व्यंजक, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (स्थिर अक्षर*)&p->aff, 1);
      अगर( sqlite3_व्यंजक_Can_Return_Subtype(pParse, p->p_व्यंजक) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      अगर( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      वापस ret;
    }
  }
  वापस 0;
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
अटल SQLITE_NOINLINE पूर्णांक व्यंजक_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  व्यंजक *p_व्यंजक,       /* AND or OR expression to be coded */
  पूर्णांक target,        /* Put result in this register, guaranteed */
  पूर्णांक *pTmpReg       /* Write a temporary register here */
){
  पूर्णांक op;            /* The opcode.  TK_AND or TK_OR */
  पूर्णांक skipOp;        /* Opcode for the branch that skips one operand */
  पूर्णांक addrSkip;      /* Branch instruction that skips one of the operands */
  पूर्णांक regSS = 0;     /* Register holding computed operand when other omitted */
  पूर्णांक r1, r2;        /* Registers for left and right operands, respectively */
  व्यंजक *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  निश्चित( p_व्यंजक!=0 );
  op = p_व्यंजक->op;
  निश्चित( op==TK_AND || op==TK_OR );
  निश्चित( TK_AND==OP_And );            testcase( op==TK_AND );
  निश्चित( TK_OR==OP_Or );              testcase( op==TK_OR );
  निश्चित( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_व्यंजक_Simplified_And_Or(p_व्यंजक);
  अगर( pAlt!=p_व्यंजक ){
    r1 = sqlite3_व्यंजक_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    वापस target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  अगर( व्यंजक_Eval_Rhs_First(p_व्यंजक) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_व्यंजक_Code_Target(pParse, p_व्यंजक->p_दायाँ, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, pTmpReg);
  }अन्यथा{
    /* Compute the left operand first */
    r1 = sqlite3_व्यंजक_Code_Target(pParse, p_व्यंजक->p_बायाँ, target);
    अगर( व्यंजक_Has_Property(p_व्यंजक->p_दायाँ, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }अन्यथा{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_दायाँ, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  अगर( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  वापस target;
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
पूर्णांक sqlite3_व्यंजक_Code_Target(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  पूर्णांक op;                   /* The opcode being coded */
  पूर्णांक inReg = target;       /* Results stored in register inReg */
  पूर्णांक regFree1 = 0;         /* If non-zero free this temporary register */
  पूर्णांक regFree2 = 0;         /* If non-zero free this temporary register */
  पूर्णांक r1, r2;               /* Various register numbers */
  व्यंजक अस्थायी_X;               /* Temporary expression node */
  पूर्णांक p5 = 0;

  निश्चित( target>0 && target<=pParse->nMem );
  निश्चित( v!=0 );

व्यंजक_code_doover:
  अगर( p_व्यंजक==0 ){
    op = TK_NULL;
  }अन्यथा अगर( pParse->pIdxEpr!=0
   && !व्यंजक_Has_Property(p_व्यंजक, EP_Leaf)
   && (r1 = sqlite3_Indexed_व्यंजक_Lookup(pParse, p_व्यंजक, target))>=0
  ){
    वापस r1;
  }अन्यथा{
    निश्चित( !व्यंजक_Has_VVAProperty(p_व्यंजक,EP_Immutable) );
    op = p_व्यंजक->op;
  }
  निश्चित( op!=TK_ORDER );
  चयन( op ){
    हाल TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_व्यंजक->pAggInfo;
      काष्ठा AggInfo_col *pCol;
      निश्चित( pAggInfo!=0 );
      निश्चित( p_व्यंजक->iAgg>=0 );
      अगर( p_व्यंजक->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#अगर_घोषित SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#पूर्ण_अगर
        अवरोध;
      }
      pCol = &pAggInfo->aCol[p_व्यंजक->iAgg];
      अगर( !pAggInfo->directMode ){
        वापस AggInfoColumnReg(pAggInfo, p_व्यंजक->iAgg);
      }अन्यथा अगर( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        अगर( pTab==0 ){
          /* No comment added */
        }अन्यथा अगर( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }अन्यथा{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          अगर( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        वापस target;
      }अन्यथा अगर( p_व्यंजक->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_व्यंजक->iTable, p_व्यंजक->iColumn, target);
        वापस target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    हाल TK_COLUMN: {
      पूर्णांक iTab = p_व्यंजक->iTable;
      पूर्णांक iReg;
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        पूर्णांक aff;
        iReg = sqlite3_व्यंजक_Code_Target(pParse, p_व्यंजक->p_बायाँ,target);
        निश्चित( व्यंजक_Use_YTab(p_व्यंजक) );
        निश्चित( p_व्यंजक->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_व्यंजक->y.pTab, p_व्यंजक->iColumn);
        अगर( aff>SQLITE_AFF_BLOB ){
          अटल स्थिर अक्षर zAff[] = "B\000C\000D\000E\000F";
          निश्चित( SQLITE_AFF_BLOB=='A' );
          निश्चित( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        वापस iReg;
      }
      अगर( iTab<0 ){
        अगर( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          पूर्णांक iSrc;
          पूर्णांक iCol = p_व्यंजक->iColumn;
          निश्चित( व्यंजक_Use_YTab(p_व्यंजक) );
          pTab = p_व्यंजक->y.pTab;
          निश्चित( pTab!=0 );
          निश्चित( iCol>=XN_ROWID );
          निश्चित( iCol<pTab->nCol );
          अगर( iCol<0 ){
            वापस -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
          अगर( pCol->colFlags & COLFLAG_GENERATED ){
            अगर( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_संदेश(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              वापस 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            अगर( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_व्यंजक_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            वापस iSrc;
          }अन्यथा
#पूर्ण_अगर /* SQLITE_OMIT_GENERATED_COLUMNS */
          अगर( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            वापस target;
          }अन्यथा{
            वापस iSrc;
          }
        }अन्यथा{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      अन्यथा अगर( pParse->p_Idx_Part_व्यंजक 
       && 0!=(r1 = व्यंजक_Partidx_व्यंजक_Lookup(pParse, p_व्यंजक, target))
      ){
        वापस r1;
      }
      निश्चित( व्यंजक_Use_YTab(p_व्यंजक) );
      निश्चित( p_व्यंजक->y.pTab!=0 );
      iReg = sqlite3_व्यंजक_Code_Get_Column(pParse, p_व्यंजक->y.pTab,
                               p_व्यंजक->iColumn, iTab, target,
                               p_व्यंजक->op2);
      वापस iReg;
    }
    हाल TK_INTEGER: {
      codeInteger(pParse, p_व्यंजक, 0, target);
      वापस target;
    }
    हाल TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक), target);
      वापस target;
    }
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
    हाल TK_FLOAT: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      codeReal(v, p_व्यंजक->u.zToken, 0, target);
      वापस target;
    }
#पूर्ण_अगर
    हाल TK_STRING: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3VdbeLoadString(v, target, p_व्यंजक->u.zToken);
      वापस target;
    }
    हाल TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_व्यंजक->y.nReg - 1);
      वापस target;
    }
    शेष: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      निश्चित( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      वापस target;
    }
#अगर_अघोषित SQLITE_OMIT_BLOB_LITERAL
    हाल TK_BLOB: {
      पूर्णांक n;
      स्थिर अक्षर *z;
      अक्षर *zBlob;
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      निश्चित( p_व्यंजक->u.zToken[0]=='x' || p_व्यंजक->u.zToken[0]=='X' );
      निश्चित( p_व्यंजक->u.zToken[1]=='\'' );
      z = &p_व्यंजक->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      निश्चित( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      वापस target;
    }
#पूर्ण_अगर
    हाल TK_VARIABLE: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      निश्चित( p_व्यंजक->u.zToken!=0 );
      निश्चित( p_व्यंजक->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_व्यंजक->iColumn, target);
      वापस target;
    }
    हाल TK_REGISTER: {
      वापस p_व्यंजक->iTable;
    }
#अगर_अघोषित SQLITE_OMIT_CAST
    हाल TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_व्यंजक_Code(pParse, p_व्यंजक->p_बायाँ, target);
      निश्चित( inReg==target );
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_व्यंजक->u.zToken, 0));
      वापस inReg;
    }
#पूर्ण_अगर /* SQLITE_OMIT_CAST */
    हाल TK_IS:
    हाल TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    हाल TK_LT:
    हाल TK_LE:
    हाल TK_GT:
    हाल TK_GE:
    हाल TK_NE:
    हाल TK_EQ: {
      व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
      पूर्णांक addrIsNull = 0;
      अगर( sqlite3_व्यंजक_Is_Vector(p_बायाँ) ){
        codeVectorCompare(pParse, p_व्यंजक, target, op, p5);
      }अन्यथा{
        अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = व्यंजक_Compute_Operands(pParse, p_व्यंजक,
                                     &r1, &r2, &regFree1, &regFree2);
        }अन्यथा{
          r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
          r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_दायाँ, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_बायाँ, p_व्यंजक->p_दायाँ, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            व्यंजक_Has_Property(p_व्यंजक,EP_Commuted));
        निश्चित(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        निश्चित(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        निश्चित(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        निश्चित(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        निश्चित(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        निश्चित(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        अगर( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }अन्यथा{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          अगर( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      अवरोध;
    }
    हाल TK_AND:
    हाल TK_OR: {
      inReg = व्यंजक_Code_Target_And_Or(pParse, p_व्यंजक, target, &regFree1);
      अवरोध;
    }
    हाल TK_PLUS:
    हाल TK_STAR:
    हाल TK_MINUS:
    हाल TK_REM:
    हाल TK_BITAND:
    हाल TK_BITOR:
    हाल TK_SLASH:
    हाल TK_LSHIFT:
    हाल TK_RSHIFT:
    हाल TK_CONCAT: {
      पूर्णांक addrIsNull;
      निश्चित( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      निश्चित( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      निश्चित( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      निश्चित( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      निश्चित( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      निश्चित( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      निश्चित( TK_LSHIFT==OP_Shift_बायाँ );   testcase( op==TK_LSHIFT );
      निश्चित( TK_RSHIFT==OP_Shift_दायाँ );  testcase( op==TK_RSHIFT );
      निश्चित( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Subquery) ){
        addrIsNull = व्यंजक_Compute_Operands(pParse, p_व्यंजक,
                                   &r1, &r2, &regFree1, &regFree2);
      }अन्यथा{
        r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
        r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_दायाँ, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      अगर( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      अवरोध;
    }
    हाल TK_UMINUS: {
      व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
      निश्चित( p_बायाँ );
      अगर( p_बायाँ->op==TK_INTEGER ){
        codeInteger(pParse, p_बायाँ, 1, target);
        वापस target;
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
      }अन्यथा अगर( p_बायाँ->op==TK_FLOAT ){
        निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
        codeReal(v, p_बायाँ->u.zToken, 1, target);
        वापस target;
#पूर्ण_अगर
      }अन्यथा{
        अस्थायी_X.op = TK_INTEGER;
        अस्थायी_X.flags = EP_Int_मूल्य|EP_TokenOnly;
        अस्थायी_X.u.i_मूल्य = 0;
        व्यंजक_Clear_VVAProperties(&अस्थायी_X);
        r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, &अस्थायी_X, &regFree1);
        r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      अवरोध;
    }
    हाल TK_BITNOT:
    हाल TK_NOT: {
      निश्चित( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      निश्चित( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      अवरोध;
    }
    हाल TK_TRUTH: {
      पूर्णांक isTrue;    /* IS TRUE or IS NOT TRUE */
      पूर्णांक bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक->p_दायाँ);
      bNormal = p_व्यंजक->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      अवरोध;
    }
    हाल TK_ISNULL:
    हाल TK_NOTNULL: {
      पूर्णांक addr;
      निश्चित( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      निश्चित( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      अवरोध;
    }
    हाल TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_व्यंजक->pAggInfo;
      अगर( pInfo==0
       || NEVER(p_व्यंजक->iAgg<0)
       || NEVER(p_व्यंजक->iAgg>=pInfo->nFunc)
      ){
        निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
        sqlite3_Error_संदेश(pParse, "misuse of aggregate: %#T()", p_व्यंजक);
      }अन्यथा{
        वापस AggInfoFuncReg(pInfo, p_व्यंजक->iAgg);
      }
      अवरोध;
    }
    हाल TK_FUNCTION: {
      व्यंजक_List *pFarg;       /* List of function arguments */
      पूर्णांक nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      स्थिर अक्षर *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      पूर्णांक i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
        वापस p_व्यंजक->y.pWin->reg_परिणाम;
      }
#पूर्ण_अगर

      अगर( ConstFactorOk(pParse)
       && sqlite3_व्यंजक_Is_Constant_Not_Join(pParse,p_व्यंजक)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        वापस sqlite3_व्यंजक_Code_Run_Just_Once(pParse, p_व्यंजक, -1);
      }
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly) );
      निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
      pFarg = p_व्यंजक->x.pList;
      nFarg = pFarg ? pFarg->n_व्यंजक : 0;
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      zId = p_व्यंजक->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#अगर_घोषित SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      अगर( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#पूर्ण_अगर
      अगर( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_संदेश(pParse, "unknown function: %#T()", p_व्यंजक);
        अवरोध;
      }
      अगर( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        निश्चित( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        निश्चित( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        वापस व्यंजक_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->p_User_तथ्य), target);
      }अन्यथा अगर( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_व्यंजक_Function_Usable(pParse, p_व्यंजक, pDef);
      }

      क्रम(i=0; i<nFarg; i++){
        अगर( i<32 && sqlite3_व्यंजक_Is_Constant(pParse, pFarg->a[i].p_व्यंजक) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        अगर( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_व्यंजक_Coll_Seq(pParse, pFarg->a[i].p_व्यंजक);
        }
      }
      अगर( pFarg ){
        अगर( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }अन्यथा{
          r1 = sqlite3_Get_अस्थायी_Range(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        अगर( (pDef->funcFlags & (SQLITE_FUNC_लंबाई|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 व्यंजक_Op;
          निश्चित( nFarg==1 );
          निश्चित( pFarg->a[0].p_व्यंजक!=0 );
          व्यंजक_Op = pFarg->a[0].p_व्यंजक->op;
          अगर( व्यंजक_Op==TK_COLUMN || व्यंजक_Op==TK_AGG_COLUMN ){
            निश्चित( SQLITE_FUNC_लंबाई==OPFLAG_LENGTHARG );
            निश्चित( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            निश्चित( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            निश्चित( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_व्यंजक->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_व्यंजक_Code_व्यंजक_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }अन्यथा{
        r1 = 0;
      }
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
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
      अगर( nFarg>=2 && व्यंजक_Has_Property(p_व्यंजक, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_व्यंजक);
      }अन्यथा अगर( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_व्यंजक);
      }
#पूर्ण_अगर
      अगर( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        अगर( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (अक्षर *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_व्यंजक->op2);
      अगर( nFarg ){
        अगर( constMask==0 ){
          sqlite3_Release_अस्थायी_Range(pParse, r1, nFarg);
        }अन्यथा{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      वापस target;
    }
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
    हाल TK_EXISTS:
    हाल TK_SELECT: {
      पूर्णांक nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      अगर( pParse->db->mallocFailed ){
        वापस 0;
      }अन्यथा अगर( op==TK_SELECT
             && ALWAYS( व्यंजक_Use_XSelect(p_व्यंजक) )
             && (nCol = p_व्यंजक->x.pSelect->pEList->n_व्यंजक)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }अन्यथा{
        वापस sqlite3CodeSubselect(pParse, p_व्यंजक);
      }
      अवरोध;
    }
    हाल TK_SELECT_COLUMN: {
      पूर्णांक n;
      व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
      अगर( p_बायाँ->iTable==0 || pParse->withinRJSubrtn > p_बायाँ->op2 ){
        p_बायाँ->iTable = sqlite3CodeSubselect(pParse, p_बायाँ);
        p_बायाँ->op2 = pParse->withinRJSubrtn;
      }
      निश्चित( p_बायाँ->op==TK_SELECT || p_बायाँ->op==TK_ERROR );
      n = sqlite3_व्यंजक_Vector_आकार(p_बायाँ);
      अगर( p_व्यंजक->iTable!=n ){
        sqlite3_Error_संदेश(pParse, "%d columns assigned %d values",
                                p_व्यंजक->iTable, n);
      }
      वापस p_बायाँ->iTable + p_व्यंजक->iColumn;
    }
    हाल TK_IN: {
      पूर्णांक destIfFalse = sqlite3VdbeMakeLabel(pParse);
      पूर्णांक destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_व्यंजक_Code_IN(pParse, p_व्यंजक, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      वापस target;
    }
#पूर्ण_अगर /* SQLITE_OMIT_SUBQUERY */


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
    हाल TK_BETWEEN: {
      व्यंजक_Code_Between(pParse, p_व्यंजक, target, 0, 0);
      वापस target;
    }
    हाल TK_COLLATE: {
      अगर( !व्यंजक_Has_Property(p_व्यंजक, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        निश्चित( p_व्यंजक->p_बायाँ );
        sqlite3_व्यंजक_Code(pParse, p_व्यंजक->p_बायाँ, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        वापस target;
      }अन्यथा{
        p_व्यंजक = p_व्यंजक->p_बायाँ;
        जाओ व्यंजक_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    हाल TK_SPAN:
    हाल TK_UPLUS: {
      p_व्यंजक = p_व्यंजक->p_बायाँ;
      जाओ व्यंजक_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    हाल TK_TRIGGER: {
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
      पूर्णांक iCol;
      पूर्णांक p1;

      निश्चित( व्यंजक_Use_YTab(p_व्यंजक) );
      pTab = p_व्यंजक->y.pTab;
      iCol = p_व्यंजक->iColumn;
      p1 = p_व्यंजक->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      निश्चित( p_व्यंजक->iTable==0 || p_व्यंजक->iTable==1 );
      निश्चित( iCol>=-1 && iCol<pTab->nCol );
      निश्चित( pTab->iPKey<0 || iCol!=pTab->iPKey );
      निश्चित( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_व्यंजक->iTable ? "new" : "old"),
        (p_व्यंजक->iColumn<0 ? "rowid" : p_व्यंजक->y.pTab->aCol[iCol].zCnName)
      ));

#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      अगर( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#पूर्ण_अगर
      अवरोध;
    }

    हाल TK_VECTOR: {
      sqlite3_Error_संदेश(pParse, "row value misused");
      अवरोध;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    हाल TK_IF_NULL_ROW: {
      पूर्णांक addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_व्यंजक->pAggInfo;
      अगर( pAggInfo ){
        निश्चित( p_व्यंजक->iAgg>=0 && p_व्यंजक->iAgg<pAggInfo->nColumn );
        अगर( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_व्यंजक->iAgg);
          अवरोध;
        }
        अगर( p_व्यंजक->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_व्यंजक->iAgg].iSorterColumn,
                            target);
          inReg = target;
          अवरोध;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_व्यंजक->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_व्यंजक_Code(pParse, p_व्यंजक->p_बायाँ, target);
      निश्चित( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      अवरोध;
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
    हाल TK_CASE: {
      पूर्णांक endLabel;                     /* GOTO label for end of CASE stmt */
      पूर्णांक अग्र_Case;                     /* GOTO label for next WHEN clause */
      पूर्णांक n_व्यंजक;                        /* 2x number of WHEN terms */
      पूर्णांक i;                            /* Loop counter */
      व्यंजक_List *pEList;                 /* List of WHEN terms */
      काष्ठा व्यंजक_List_item *aListelem;  /* Array of WHEN terms */
      व्यंजक opCompare;                   /* The X==Ei expression */
      व्यंजक *pX;                         /* The X expression */
      व्यंजक *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      व्यंजक *pDel = 0;
      sqlite3 *db = pParse->db;

      निश्चित( व्यंजक_Use_XList(p_व्यंजक) && p_व्यंजक->x.pList!=0 );
      निश्चित(p_व्यंजक->x.pList->n_व्यंजक > 0);
      pEList = p_व्यंजक->x.pList;
      aListelem = pEList->a;
      n_व्यंजक = pEList->n_व्यंजक;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      अगर( (pX = p_व्यंजक->p_बायाँ)!=0 ){
        pDel = sqlite3_व्यंजक_Dup(db, pX, 0);
        अगर( db->mallocFailed ){
          sqlite3_व्यंजक_Delete(db, pDel);
          अवरोध;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_व्यंजक_To_Register(pDel, व्यंजक_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        स_रखो(&opCompare, 0, माप(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_बायाँ = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      क्रम(i=0; i<n_व्यंजक-1; i=i+2){
        अगर( pX ){
          निश्चित( pTest!=0 );
          opCompare.p_दायाँ = aListelem[i].p_व्यंजक;
        }अन्यथा{
          pTest = aListelem[i].p_व्यंजक;
        }
        अग्र_Case = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_व्यंजक_If_False(pParse, pTest, अग्र_Case, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_व्यंजक->op==TK_COLUMN );
        sqlite3_व्यंजक_Code(pParse, aListelem[i+1].p_व्यंजक, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, अग्र_Case);
      }
      अगर( (n_व्यंजक&1)!=0 ){
        sqlite3_व्यंजक_Code(pParse, pEList->a[n_व्यंजक-1].p_व्यंजक, target);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_व्यंजक_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_TRIGGER
    हाल TK_RAISE: {
      निश्चित( p_व्यंजक->aff_व्यंजक==OE_Rollback
           || p_व्यंजक->aff_व्यंजक==OE_Abort
           || p_व्यंजक->aff_व्यंजक==OE_Fail
           || p_व्यंजक->aff_व्यंजक==OE_Ignore
      );
      अगर( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_संदेश(pParse,
                       "RAISE() may only be used within a trigger-program");
        वापस 0;
      }
      अगर( p_व्यंजक->aff_व्यंजक==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      अगर( p_व्यंजक->aff_व्यंजक==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }अन्यथा{
        r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_व्यंजक->aff_व्यंजक, r1);
      }
      अवरोध;
    }
#पूर्ण_अगर
  }
  sqlite3_Release_अस्थायी_Reg(pParse, regFree1);
  sqlite3_Release_अस्थायी_Reg(pParse, regFree2);
  वापस inReg;
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
पूर्णांक sqlite3_व्यंजक_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  व्यंजक *p_व्यंजक,      /* The expression to code when the VDBE initializes */
  पूर्णांक regDest       /* Store the value in this register */
){
  व्यंजक_List *p;
  निश्चित( ConstFactorOk(pParse) );
  निश्चित( regDest!=0 );
  p = pParse->p_Const_व्यंजक;
  अगर( regDest<0 && p ){
    काष्ठा व्यंजक_List_item *pItem;
    पूर्णांक i;
    क्रम(pItem=p->a, i=p->n_व्यंजक; i>0; pItem++, i--){
      अगर( pItem->fg.reusable
       && sqlite3_व्यंजक_Compare(0,pItem->p_व्यंजक,p_व्यंजक,-1)==0
      ){
        वापस pItem->u.i_Const_व्यंजक_Reg;
      }
    }
  }
  p_व्यंजक = sqlite3_व्यंजक_Dup(pParse->db, p_व्यंजक, 0);
  अगर( p_व्यंजक!=0 && व्यंजक_Has_Property(p_व्यंजक, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    पूर्णांक addr;
    निश्चित( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    अगर( !pParse->db->mallocFailed ){
      अगर( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_व्यंजक_Code(pParse, p_व्यंजक, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_व्यंजक_Delete(pParse->db, p_व्यंजक);
    sqlite3VdbeJumpHere(v, addr);
  }अन्यथा{
    p = sqlite3_व्यंजक_List_Append(pParse, p, p_व्यंजक);
    अगर( p ){
       काष्ठा व्यंजक_List_item *pItem = &p->a[p->n_व्यंजक-1];
       pItem->fg.reusable = regDest<0;
       अगर( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_व्यंजक_Reg = regDest;
    }
    pParse->p_Const_व्यंजक = p;
  }
  वापस regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE व्योम sqlite3_व्यंजक_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  पूर्णांक iReg,        /* First register to set to NULL */
  पूर्णांक nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  व्यंजक t;
  स_रखो(&t, 0, माप(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_व्यंजक_Code_Run_Just_Once(pParse, &t, iReg);
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
पूर्णांक sqlite3_व्यंजक_Code_अस्थायी(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक *pReg){
  पूर्णांक r2;
  p_व्यंजक = sqlite3_व्यंजक_Skip_Collate_And_Likely(p_व्यंजक);
  अगर( ConstFactorOk(pParse)
   && ALWAYS(p_व्यंजक!=0)
   && p_व्यंजक->op!=TK_REGISTER
   && sqlite3_व्यंजक_Is_Constant_Not_Join(pParse, p_व्यंजक)
  ){
    *pReg  = 0;
    r2 = sqlite3_व्यंजक_Code_Run_Just_Once(pParse, p_व्यंजक, -1);
  }अन्यथा{
    पूर्णांक r1 = sqlite3_Get_अस्थायी_Reg(pParse);
    r2 = sqlite3_व्यंजक_Code_Target(pParse, p_व्यंजक, r1);
    अगर( r2==r1 ){
      *pReg = r1;
    }अन्यथा{
      sqlite3_Release_अस्थायी_Reg(pParse, r1);
      *pReg = 0;
    }
  }
  वापस r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
व्योम sqlite3_व्यंजक_Code(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक target){
  पूर्णांक inReg;

  निश्चित( p_व्यंजक==0 || !व्यंजक_Has_VVAProperty(p_व्यंजक,EP_Immutable) );
  निश्चित( target>0 && target<=pParse->nMem );
  निश्चित( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  अगर( pParse->pVdbe==0 ) वापस;
  inReg = sqlite3_व्यंजक_Code_Target(pParse, p_व्यंजक, target);
  अगर( inReg!=target ){
    u8 op;
    व्यंजक *pX = sqlite3_व्यंजक_Skip_Collate_And_Likely(p_व्यंजक);
    testcase( pX!=p_व्यंजक );
    अगर( ALWAYS(pX)
     && (व्यंजक_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }अन्यथा{
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
व्योम sqlite3_व्यंजक_Code_Copy(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक target){
  sqlite3 *db = pParse->db;
  p_व्यंजक = sqlite3_व्यंजक_Dup(db, p_व्यंजक, 0);
  अगर( !db->mallocFailed ) sqlite3_व्यंजक_Code(pParse, p_व्यंजक, target);
  sqlite3_व्यंजक_Delete(db, p_व्यंजक);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
व्योम sqlite3_व्यंजक_Code_Factorable(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक target){
  अगर( pParse->okConstFactor && sqlite3_व्यंजक_Is_Constant_Not_Join(pParse,p_व्यंजक) ){
    sqlite3_व्यंजक_Code_Run_Just_Once(pParse, p_व्यंजक, target);
  }अन्यथा{
    sqlite3_व्यंजक_Code_Copy(pParse, p_व्यंजक, target);
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
पूर्णांक sqlite3_व्यंजक_Code_व्यंजक_List(
  Parse *pParse,     /* Parsing context */
  व्यंजक_List *pList,   /* The expression list to be coded */
  पूर्णांक target,        /* Where to write results */
  पूर्णांक srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  काष्ठा व्यंजक_List_item *pItem;
  पूर्णांक i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  निश्चित( pList!=0 );
  निश्चित( target>0 );
  निश्चित( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_व्यंजक;
  अगर( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  क्रम(pItem=pList->a, i=0; i<n; i++, pItem++){
    व्यंजक *p_व्यंजक = pItem->p_व्यंजक;
#अगर_घोषित SQLITE_ENABLE_SORTER_REFERENCES
    अगर( pItem->fg.bSorterRef ){
      i--;
      n--;
    }अन्यथा
#पूर्ण_अगर
    अगर( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      अगर( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }अन्यथा{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }अन्यथा अगर( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_व्यंजक_Is_Constant_Not_Join(pParse,p_व्यंजक)
    ){
      sqlite3_व्यंजक_Code_Run_Just_Once(pParse, p_व्यंजक, target+i);
    }अन्यथा{
      पूर्णांक inReg = sqlite3_व्यंजक_Code_Target(pParse, p_व्यंजक, target+i);
      अगर( inReg!=target+i ){
        VdbeOp *pOp;
        अगर( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }अन्यथा{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  वापस n;
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
अटल व्योम व्यंजक_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  व्यंजक *p_व्यंजक,      /* The BETWEEN expression */
  पूर्णांक dest,         /* Jump destination or storage location */
  व्योम (*xJump)(Parse*,व्यंजक*,पूर्णांक,पूर्णांक), /* Action to take */
  पूर्णांक jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  व्यंजक व्यंजक_And;     /* The AND operator in  x>=y AND x<=z  */
  व्यंजक comp_बायाँ;    /* The  x>=y  term */
  व्यंजक comp_दायाँ;   /* The  x<=z  term */
  पूर्णांक regFree1 = 0; /* Temporary use register */
  व्यंजक *pDel = 0;
  sqlite3 *db = pParse->db;

  स_रखो(&comp_बायाँ, 0, माप(व्यंजक));
  स_रखो(&comp_दायाँ, 0, माप(व्यंजक));
  स_रखो(&व्यंजक_And, 0, माप(व्यंजक));

  निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
  pDel = sqlite3_व्यंजक_Dup(db, p_व्यंजक->p_बायाँ, 0);
  अगर( db->mallocFailed==0 ){
    व्यंजक_And.op = TK_AND;
    व्यंजक_And.p_बायाँ = &comp_बायाँ;
    व्यंजक_And.p_दायाँ = &comp_दायाँ;
    comp_बायाँ.op = TK_GE;
    comp_बायाँ.p_बायाँ = pDel;
    comp_बायाँ.p_दायाँ = p_व्यंजक->x.pList->a[0].p_व्यंजक;
    comp_दायाँ.op = TK_LE;
    comp_दायाँ.p_बायाँ = pDel;
    comp_दायाँ.p_दायाँ = p_व्यंजक->x.pList->a[1].p_व्यंजक;
    sqlite3_व्यंजक_To_Register(pDel, व्यंजक_Code_Vector(pParse, pDel, &regFree1));
    अगर( xJump ){
      xJump(pParse, &व्यंजक_And, dest, jumpIfNull);
    }अन्यथा{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_व्यंजक_Code_Target(pParse, &व्यंजक_And, dest);
    }
    sqlite3_Release_अस्थायी_Reg(pParse, regFree1);
  }
  sqlite3_व्यंजक_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_व्यंजक_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_व्यंजक_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_व्यंजक_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_व्यंजक_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_व्यंजक_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_व्यंजक_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_व्यंजक_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_व्यंजक_If_False && jumpIfNull!=0 && regFree1!=0 );
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
व्योम sqlite3_व्यंजक_If_True(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक dest, पूर्णांक jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  पूर्णांक op = 0;
  पूर्णांक regFree1 = 0;
  पूर्णांक regFree2 = 0;
  पूर्णांक r1, r2;

  निश्चित( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  अगर( NEVER(v==0) )     वापस;  /* Existence of VDBE checked by caller */
  अगर( NEVER(p_व्यंजक==0) ) वापस;  /* No way this can happen */
  निश्चित( !व्यंजक_Has_VVAProperty(p_व्यंजक, EP_Immutable) );
  op = p_व्यंजक->op;
  चयन( op ){
    हाल TK_AND:
    हाल TK_OR: {
      व्यंजक *pAlt = sqlite3_व्यंजक_Simplified_And_Or(p_व्यंजक);
      अगर( pAlt!=p_व्यंजक ){
        sqlite3_व्यंजक_If_True(pParse, pAlt, dest, jumpIfNull);
      }अन्यथा{
        व्यंजक *pFirst, *pSecond;
        अगर( व्यंजक_Eval_Rhs_First(p_व्यंजक) ){
          pFirst = p_व्यंजक->p_दायाँ;
          pSecond = p_व्यंजक->p_बायाँ;
        }अन्यथा{
          pFirst = p_व्यंजक->p_बायाँ;
          pSecond = p_व्यंजक->p_दायाँ;
        }
        अगर( op==TK_AND ){
          पूर्णांक d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_व्यंजक_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_व्यंजक_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }अन्यथा{
          testcase( jumpIfNull==0 );
          sqlite3_व्यंजक_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_व्यंजक_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      अवरोध;
    }
    हाल TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_व्यंजक_If_False(pParse, p_व्यंजक->p_बायाँ, dest, jumpIfNull);
      अवरोध;
    }
    हाल TK_TRUTH: {
      पूर्णांक isNot;      /* IS NOT TRUE or IS NOT FALSE */
      पूर्णांक isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_व्यंजक->op2==TK_ISNOT;
      isTrue = sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक->p_दायाँ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      अगर( isTrue ^ isNot ){
        sqlite3_व्यंजक_If_True(pParse, p_व्यंजक->p_बायाँ, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }अन्यथा{
        sqlite3_व्यंजक_If_False(pParse, p_व्यंजक->p_बायाँ, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      अवरोध;
    }
    हाल TK_IS:
    हाल TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    हाल TK_LT:
    हाल TK_LE:
    हाल TK_GT:
    हाल TK_GE:
    हाल TK_NE:
    हाल TK_EQ: {
      पूर्णांक addrIsNull;
      अगर( sqlite3_व्यंजक_Is_Vector(p_व्यंजक->p_बायाँ) ) जाओ default_व्यंजक;
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = व्यंजक_Compute_Operands(pParse, p_व्यंजक,
                                   &r1, &r2, &regFree1, &regFree2);
      }अन्यथा{
        r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
        r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_दायाँ, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_व्यंजक->p_बायाँ, p_व्यंजक->p_दायाँ, op,
                  r1, r2, dest, jumpIfNull, व्यंजक_Has_Property(p_व्यंजक,EP_Commuted));
      निश्चित(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      निश्चित(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      निश्चित(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      निश्चित(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      निश्चित(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      निश्चित(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      अगर( addrIsNull ){
        अगर( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }अन्यथा{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      अवरोध;
    }
    हाल TK_ISNULL:
    हाल TK_NOTNULL: {
      निश्चित( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      निश्चित( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
      निश्चित( regFree1==0 || regFree1==r1 );
      अगर( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      अवरोध;
    }
    हाल TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      व्यंजक_Code_Between(pParse, p_व्यंजक, dest, sqlite3_व्यंजक_If_True, jumpIfNull);
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
    हाल TK_IN: {
      पूर्णांक destIfFalse = sqlite3VdbeMakeLabel(pParse);
      पूर्णांक destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_व्यंजक_Code_IN(pParse, p_व्यंजक, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      अवरोध;
    }
#पूर्ण_अगर
    शेष: {
    default_व्यंजक:
      अगर( व्यंजक_Always_True(p_व्यंजक) ){
        sqlite3VdbeGoto(v, dest);
      }अन्यथा अगर( व्यंजक_Always_False(p_व्यंजक) ){
        /* No-op */
      }अन्यथा{
        r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      अवरोध;
    }
  }
  sqlite3_Release_अस्थायी_Reg(pParse, regFree1);
  sqlite3_Release_अस्थायी_Reg(pParse, regFree2); 
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
व्योम sqlite3_व्यंजक_If_False(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक dest, पूर्णांक jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  पूर्णांक op = 0;
  पूर्णांक regFree1 = 0;
  पूर्णांक regFree2 = 0;
  पूर्णांक r1, r2;

  निश्चित( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  अगर( NEVER(v==0) ) वापस; /* Existence of VDBE checked by caller */
  अगर( p_व्यंजक==0 )    वापस;
  निश्चित( !व्यंजक_Has_VVAProperty(p_व्यंजक,EP_Immutable) );

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
  op = ((p_व्यंजक->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  निश्चित( p_व्यंजक->op!=TK_ISNULL || op==OP_NotNull );
  निश्चित( p_व्यंजक->op!=TK_NOTNULL || op==OP_IsNull );
  निश्चित( p_व्यंजक->op!=TK_NE || op==OP_Eq );
  निश्चित( p_व्यंजक->op!=TK_EQ || op==OP_Ne );
  निश्चित( p_व्यंजक->op!=TK_LT || op==OP_Ge );
  निश्चित( p_व्यंजक->op!=TK_LE || op==OP_Gt );
  निश्चित( p_व्यंजक->op!=TK_GT || op==OP_Le );
  निश्चित( p_व्यंजक->op!=TK_GE || op==OP_Lt );

  चयन( p_व्यंजक->op ){
    हाल TK_AND:
    हाल TK_OR: {
      व्यंजक *pAlt = sqlite3_व्यंजक_Simplified_And_Or(p_व्यंजक);
      अगर( pAlt!=p_व्यंजक ){
        sqlite3_व्यंजक_If_False(pParse, pAlt, dest, jumpIfNull);
      }अन्यथा{
        व्यंजक *pFirst, *pSecond;
        अगर( व्यंजक_Eval_Rhs_First(p_व्यंजक) ){
          pFirst = p_व्यंजक->p_दायाँ;
          pSecond = p_व्यंजक->p_बायाँ;
        }अन्यथा{
          pFirst = p_व्यंजक->p_बायाँ;
          pSecond = p_व्यंजक->p_दायाँ;
        }
        अगर( p_व्यंजक->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_व्यंजक_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_व्यंजक_If_False(pParse, pSecond, dest, jumpIfNull);
        }अन्यथा{
          पूर्णांक d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_व्यंजक_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_व्यंजक_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      अवरोध;
    }
    हाल TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_व्यंजक_If_True(pParse, p_व्यंजक->p_बायाँ, dest, jumpIfNull);
      अवरोध;
    }
    हाल TK_TRUTH: {
      पूर्णांक isNot;   /* IS NOT TRUE or IS NOT FALSE */
      पूर्णांक isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_व्यंजक->op2==TK_ISNOT;
      isTrue = sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक->p_दायाँ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      अगर( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_व्यंजक_If_False(pParse, p_व्यंजक->p_बायाँ, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }अन्यथा{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_व्यंजक_If_True(pParse, p_व्यंजक->p_बायाँ, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      अवरोध;
    }
    हाल TK_IS:
    हाल TK_ISNOT:
      testcase( p_व्यंजक->op==TK_IS );
      testcase( p_व्यंजक->op==TK_ISNOT );
      op = (p_व्यंजक->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    हाल TK_LT:
    हाल TK_LE:
    हाल TK_GT:
    हाल TK_GE:
    हाल TK_NE:
    हाल TK_EQ: {
      पूर्णांक addrIsNull;
      अगर( sqlite3_व्यंजक_Is_Vector(p_व्यंजक->p_बायाँ) ) जाओ default_व्यंजक;
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = व्यंजक_Compute_Operands(pParse, p_व्यंजक,
                                   &r1, &r2, &regFree1, &regFree2);
      }अन्यथा{
        r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
        r2 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_दायाँ, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_व्यंजक->p_बायाँ, p_व्यंजक->p_दायाँ, op,
                  r1, r2, dest, jumpIfNull,व्यंजक_Has_Property(p_व्यंजक,EP_Commuted));
      निश्चित(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      निश्चित(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      निश्चित(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      निश्चित(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      निश्चित(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      निश्चित(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      अगर( addrIsNull ){
        अगर( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }अन्यथा{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      अवरोध;
    }
    हाल TK_ISNULL:
    हाल TK_NOTNULL: {
      r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक->p_बायाँ, &regFree1);
      निश्चित( regFree1==0 || regFree1==r1 );
      अगर( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      अवरोध;
    }
    हाल TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      व्यंजक_Code_Between(pParse, p_व्यंजक, dest, sqlite3_व्यंजक_If_False, jumpIfNull);
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
    हाल TK_IN: {
      अगर( jumpIfNull ){
        sqlite3_व्यंजक_Code_IN(pParse, p_व्यंजक, dest, dest);
      }अन्यथा{
        पूर्णांक destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_व्यंजक_Code_IN(pParse, p_व्यंजक, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      अवरोध;
    }
#पूर्ण_अगर
    शेष: {
    default_व्यंजक:
      अगर( व्यंजक_Always_False(p_व्यंजक) ){
        sqlite3VdbeGoto(v, dest);
      }अन्यथा अगर( व्यंजक_Always_True(p_व्यंजक) ){
        /* no-op */
      }अन्यथा{
        r1 = sqlite3_व्यंजक_Code_अस्थायी(pParse, p_व्यंजक, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      अवरोध;
    }
  }
  sqlite3_Release_अस्थायी_Reg(pParse, regFree1);
  sqlite3_Release_अस्थायी_Reg(pParse, regFree2);
}

/*
** Like sqlite3ExprIfFalse() except that a copy is made of pExpr before
** code generation, and that copy is deleted after code generation. This
** ensures that the original pExpr is unchanged.
*/
व्योम sqlite3_व्यंजक_If_False_Dup(Parse *pParse, व्यंजक *p_व्यंजक, पूर्णांक dest,पूर्णांक jumpIfNull){
  sqlite3 *db = pParse->db;
  व्यंजक *pCopy = sqlite3_व्यंजक_Dup(db, p_व्यंजक, 0);
  अगर( db->mallocFailed==0 ){
    sqlite3_व्यंजक_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_व्यंजक_Delete(db, pCopy);
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
अटल SQLITE_NOINLINE पूर्णांक व्यंजक_Compare_Variable(
  स्थिर Parse *pParse,
  स्थिर व्यंजक *pVar,
  स्थिर व्यंजक *p_व्यंजक
){
  पूर्णांक res = 2;
  पूर्णांक iVar;
  sqlite3_मूल्य *pL, *pR = 0;
 
  अगर( p_व्यंजक->op==TK_VARIABLE && pVar->iColumn==p_व्यंजक->iColumn ){
    वापस 0;
  }
  अगर( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) वापस 2;
  sqlite3_मूल्य_From_व्यंजक(pParse->db, p_व्यंजक, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  अगर( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_मूल्य(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    अगर( pL ){
      अगर( sqlite3_मूल्य_type(pL)==SQLITE_TEXT ){
        sqlite3_मूल्य_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_मूल्य_Free(pR);
    sqlite3_मूल्य_Free(pL);
  }
  वापस res;
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
पूर्णांक sqlite3_व्यंजक_Compare(
  स्थिर Parse *pParse,
  स्थिर व्यंजक *pA,
  स्थिर व्यंजक *pB,
  पूर्णांक iTab
){
  u32 combinedFlags;
  अगर( pA==0 || pB==0 ){
    वापस pB==pA ? 0 : 2;
  }
  अगर( pParse && pA->op==TK_VARIABLE ){
    वापस व्यंजक_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  अगर( combinedFlags & EP_Int_मूल्य ){
    अगर( (pA->flags&pB->flags&EP_Int_मूल्य)!=0 && pA->u.i_मूल्य==pB->u.i_मूल्य ){
      वापस 0;
    }
    वापस 2;
  }
  अगर( pA->op!=pB->op || pA->op==TK_RAISE ){
    अगर( pA->op==TK_COLLATE && sqlite3_व्यंजक_Compare(pParse, pA->p_बायाँ,pB,iTab)<2 ){
      वापस 1;
    }
    अगर( pB->op==TK_COLLATE && sqlite3_व्यंजक_Compare(pParse, pA,pB->p_बायाँ,iTab)<2 ){
      वापस 1;
    }
    अगर( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }अन्यथा{
      वापस 2;
    }
  }
  निश्चित( !व्यंजक_Has_Property(pA, EP_Int_मूल्य) );
  निश्चित( !व्यंजक_Has_Property(pB, EP_Int_मूल्य) );
  अगर( pA->u.zToken ){
    अगर( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      अगर( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) वापस 2;
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      निश्चित( pA->op==pB->op );
      अगर( व्यंजक_Has_Property(pA,EP_WinFunc)!=व्यंजक_Has_Property(pB,EP_WinFunc) ){
        वापस 2;
      }
      अगर( व्यंजक_Has_Property(pA,EP_WinFunc) ){
        अगर( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          वापस 2;
        }
      }
#पूर्ण_अगर
    }अन्यथा अगर( pA->op==TK_NULL ){
      वापस 0;
    }अन्यथा अगर( pA->op==TK_COLLATE ){
      अगर( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) वापस 2;
    }अन्यथा
    अगर( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && म_भेद(pA->u.zToken,pB->u.zToken)!=0
    ){
      वापस 2;
    }
  }
  अगर( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) वापस 2;
  अगर( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    अगर( combinedFlags & EP_xIsSelect ) वापस 2;
    अगर( (combinedFlags & EP_FixedCol)==0
     && sqlite3_व्यंजक_Compare(pParse, pA->p_बायाँ, pB->p_बायाँ, iTab) ) वापस 2;
    अगर( sqlite3_व्यंजक_Compare(pParse, pA->p_दायाँ, pB->p_दायाँ, iTab) ) वापस 2;
    अगर( sqlite3_व्यंजक_List_Compare(pA->x.pList, pB->x.pList, iTab) ) वापस 2;
    अगर( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      अगर( pA->iColumn!=pB->iColumn ) वापस 2;
      अगर( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) वापस 2;
      अगर( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        वापस 2;
      }
    }
  }
  वापस 0;
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
पूर्णांक sqlite3_व्यंजक_List_Compare(स्थिर व्यंजक_List *pA, स्थिर व्यंजक_List *pB, पूर्णांक iTab){
  पूर्णांक i;
  अगर( pA==0 && pB==0 ) वापस 0;
  अगर( pA==0 || pB==0 ) वापस 1;
  अगर( pA->n_व्यंजक!=pB->n_व्यंजक ) वापस 1;
  क्रम(i=0; i<pA->n_व्यंजक; i++){
    पूर्णांक res;
    व्यंजक *p_व्यंजक_A = pA->a[i].p_व्यंजक;
    व्यंजक *p_व्यंजक_B = pB->a[i].p_व्यंजक;
    अगर( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) वापस 1;
    अगर( (res = sqlite3_व्यंजक_Compare(0, p_व्यंजक_A, p_व्यंजक_B, iTab)) ) वापस res;
  }
  वापस 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
पूर्णांक sqlite3_व्यंजक_Compare_Skip(व्यंजक *pA,व्यंजक *pB, पूर्णांक iTab){
  वापस sqlite3_व्यंजक_Compare(0,
             sqlite3_व्यंजक_Skip_Collate(pA),
             sqlite3_व्यंजक_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
अटल पूर्णांक व्यंजक_Implies_Not_Null(
  स्थिर Parse *pParse,/* Parsing context */
  स्थिर व्यंजक *p,      /* The expression to be checked */
  स्थिर व्यंजक *pNN,    /* The expression that is NOT NULL */
  पूर्णांक iTab,           /* Table being evaluated */
  पूर्णांक seenNot         /* Return true only if p can be any non-NULL value */
){
  निश्चित( p );
  निश्चित( pNN );
  अगर( sqlite3_व्यंजक_Compare(pParse, p, pNN, iTab)==0 ){
    वापस pNN->op!=TK_NULL;
  }
  चयन( p->op ){
    हाल TK_IN: {
      अगर( seenNot && व्यंजक_Has_Property(p, EP_xIsSelect) ) वापस 0;
      निश्चित( व्यंजक_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_व्यंजक>0) );
      वापस व्यंजक_Implies_Not_Null(pParse, p->p_बायाँ, pNN, iTab, 1);
    }
    हाल TK_BETWEEN: {
      व्यंजक_List *pList;
      निश्चित( व्यंजक_Use_XList(p) );
      pList = p->x.pList;
      निश्चित( pList!=0 );
      निश्चित( pList->n_व्यंजक==2 );
      अगर( seenNot ) वापस 0;
      अगर( व्यंजक_Implies_Not_Null(pParse, pList->a[0].p_व्यंजक, pNN, iTab, 1)
       || व्यंजक_Implies_Not_Null(pParse, pList->a[1].p_व्यंजक, pNN, iTab, 1)
      ){
        वापस 1;
      }
      वापस व्यंजक_Implies_Not_Null(pParse, p->p_बायाँ, pNN, iTab, 1);
    }
    हाल TK_EQ:
    हाल TK_NE:
    हाल TK_LT:
    हाल TK_LE:
    हाल TK_GT:
    हाल TK_GE:
    हाल TK_PLUS:
    हाल TK_MINUS:
    हाल TK_BITOR:
    हाल TK_LSHIFT:
    हाल TK_RSHIFT:
    हाल TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    हाल TK_STAR:
    हाल TK_REM:
    हाल TK_BITAND:
    हाल TK_SLASH: {
      अगर( व्यंजक_Implies_Not_Null(pParse, p->p_दायाँ, pNN, iTab, seenNot) ) वापस 1;
      /* no break */ deliberate_fall_through
    }
    हाल TK_SPAN:
    हाल TK_COLLATE:
    हाल TK_UPLUS:
    हाल TK_UMINUS: {
      वापस व्यंजक_Implies_Not_Null(pParse, p->p_बायाँ, pNN, iTab, seenNot);
    }
    हाल TK_TRUTH: {
      अगर( seenNot ) वापस 0;
      अगर( p->op2!=TK_IS ) वापस 0;
      वापस व्यंजक_Implies_Not_Null(pParse, p->p_बायाँ, pNN, iTab, 1);
    }
    हाल TK_BITNOT:
    हाल TK_NOT: {
      वापस व्यंजक_Implies_Not_Null(pParse, p->p_बायाँ, pNN, iTab, 1);
    }
  }
  वापस 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
अटल पूर्णांक sqlite3_व्यंजक_Is_Not_True(व्यंजक *p_व्यंजक){
  पूर्णांक v;
  अगर( p_व्यंजक->op==TK_NULL ) वापस 1;
  अगर( p_व्यंजक->op==TK_TRUEFALSE && sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक)==0 ) वापस 1;
  v = 1;
  अगर( sqlite3_व्यंजक_Is_Integer(p_व्यंजक, &v, 0, 0) && v==0 ) वापस 1;
  वापस 0;
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
अटल पूर्णांक sqlite3_व्यंजक_Is_IIF(sqlite3 *db, स्थिर व्यंजक *p_व्यंजक){
  व्यंजक_List *pList;
  अगर( p_व्यंजक->op==TK_FUNCTION ){
    स्थिर अक्षर *z = p_व्यंजक->u.zToken;
    FuncDef *pDef;
    अगर( (z[0]!='i' && z[0]!='I') ) वापस 0;
    अगर( p_व्यंजक->x.pList==0 ) वापस 0;
    pDef = sqlite3FindFunction(db, z, p_व्यंजक->x.pList->n_व्यंजक, ENC(db), 0);
#अगर_घोषित SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    अगर( pDef==0 ) वापस 0;
#अन्यथा
    अगर( NEVER(pDef==0) ) वापस 0;
#पूर्ण_अगर
    अगर( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) वापस 0;
    अगर( SQLITE_PTR_TO_INT(pDef->p_User_तथ्य)!=INLINEFUNC_iif ) वापस 0;
  }अन्यथा अगर( p_व्यंजक->op==TK_CASE ){
    अगर( p_व्यंजक->p_बायाँ!=0 ) वापस 0;
  }अन्यथा{
    वापस 0;
  }
  pList = p_व्यंजक->x.pList;
  निश्चित( pList!=0 );
  अगर( pList->n_व्यंजक==2 ) वापस 1;
  अगर( pList->n_व्यंजक==3 && sqlite3_व्यंजक_Is_Not_True(pList->a[2].p_व्यंजक) ) वापस 1;
  वापस 0;
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
पूर्णांक sqlite3_व्यंजक_Implies_व्यंजक(
  स्थिर Parse *pParse,
  स्थिर व्यंजक *pE1,
  स्थिर व्यंजक *pE2,
  पूर्णांक iTab
){
  अगर( sqlite3_व्यंजक_Compare(pParse, pE1, pE2, iTab)==0 ){
    वापस 1;
  }
  अगर( pE2->op==TK_OR
   && (sqlite3_व्यंजक_Implies_व्यंजक(pParse, pE1, pE2->p_बायाँ, iTab)
             || sqlite3_व्यंजक_Implies_व्यंजक(pParse, pE1, pE2->p_दायाँ, iTab) )
  ){
    वापस 1;
  }
  अगर( pE2->op==TK_NOTNULL
   && व्यंजक_Implies_Not_Null(pParse, pE1, pE2->p_बायाँ, iTab, 0)
  ){
    वापस 1;
  }
  अगर( sqlite3_व्यंजक_Is_IIF(pParse->db, pE1) ){
    वापस sqlite3_व्यंजक_Implies_व्यंजक(pParse,pE1->x.pList->a[0].p_व्यंजक,pE2,iTab);
  }
  वापस 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
अटल व्योम bothImplyNotNullRow(Walker *pWalker, व्यंजक *pE1, व्यंजक *pE2){
  अगर( pWalker->eCode==0 ){
    sqlite3_Walk_व्यंजक(pWalker, pE1);
    अगर( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_व्यंजक(pWalker, pE2);
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
अटल पूर्णांक impliesNotNullRow(Walker *pWalker, व्यंजक *p_व्यंजक){
  testcase( p_व्यंजक->op==TK_AGG_COLUMN );
  testcase( p_व्यंजक->op==TK_AGG_FUNCTION );
  अगर( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) ) वापस WRC_Prune;
  अगर( व्यंजक_Has_Property(p_व्यंजक, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    वापस WRC_Prune;
  }
  चयन( p_व्यंजक->op ){
    हाल TK_ISNOT:
    हाल TK_ISNULL:
    हाल TK_NOTNULL:
    हाल TK_IS:
    हाल TK_VECTOR:
    हाल TK_FUNCTION:
    हाल TK_TRUTH:
    हाल TK_CASE:
      testcase( p_व्यंजक->op==TK_ISNOT );
      testcase( p_व्यंजक->op==TK_ISNULL );
      testcase( p_व्यंजक->op==TK_NOTNULL );
      testcase( p_व्यंजक->op==TK_IS );
      testcase( p_व्यंजक->op==TK_VECTOR );
      testcase( p_व्यंजक->op==TK_FUNCTION );
      testcase( p_व्यंजक->op==TK_TRUTH );
      testcase( p_व्यंजक->op==TK_CASE );
      वापस WRC_Prune;

    हाल TK_COLUMN:
      अगर( pWalker->u.iCur==p_व्यंजक->iTable ){
        pWalker->eCode = 1;
        वापस WRC_Abort;
      }
      वापस WRC_Prune;

    हाल TK_OR:
    हाल TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_व्यंजक->op==TK_OR );
      testcase( p_व्यंजक->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_व्यंजक->p_बायाँ, p_व्यंजक->p_दायाँ);
      वापस WRC_Prune;
       
    हाल TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      अगर( व्यंजक_Use_XList(p_व्यंजक) && ALWAYS(p_व्यंजक->x.pList->n_व्यंजक>0) ){
        sqlite3_Walk_व्यंजक(pWalker, p_व्यंजक->p_बायाँ);
      }
      वापस WRC_Prune;

    हाल TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
      निश्चित( p_व्यंजक->x.pList->n_व्यंजक==2 );
      sqlite3_Walk_व्यंजक(pWalker, p_व्यंजक->p_बायाँ);
      bothImplyNotNullRow(pWalker, p_व्यंजक->x.pList->a[0].p_व्यंजक,
                                   p_व्यंजक->x.pList->a[1].p_व्यंजक);
      वापस WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    हाल TK_EQ:
    हाल TK_NE:
    हाल TK_LT:
    हाल TK_LE:
    हाल TK_GT:
    हाल TK_GE: {
      व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
      व्यंजक *p_दायाँ = p_व्यंजक->p_दायाँ;
      testcase( p_व्यंजक->op==TK_EQ );
      testcase( p_व्यंजक->op==TK_NE );
      testcase( p_व्यंजक->op==TK_LT );
      testcase( p_व्यंजक->op==TK_LE );
      testcase( p_व्यंजक->op==TK_GT );
      testcase( p_व्यंजक->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      निश्चित( p_बायाँ->op!=TK_COLUMN || व्यंजक_Use_YTab(p_बायाँ) );
      निश्चित( p_दायाँ->op!=TK_COLUMN || व्यंजक_Use_YTab(p_दायाँ) );
      अगर( (p_बायाँ->op==TK_COLUMN
           && ALWAYS(p_बायाँ->y.pTab!=0)
           && IsVirtual(p_बायाँ->y.pTab))
       || (p_दायाँ->op==TK_COLUMN
           && ALWAYS(p_दायाँ->y.pTab!=0)
           && IsVirtual(p_दायाँ->y.pTab))
      ){
        वापस WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    शेष:
      वापस WRC_Continue;
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
पूर्णांक sqlite3_व्यंजक_Implies_Non_Null_Row(व्यंजक *p, पूर्णांक iTab, पूर्णांक isRJ){
  Walker w;
  p = sqlite3_व्यंजक_Skip_Collate_And_Likely(p);
  अगर( p==0 ) वापस 0;
  अगर( p->op==TK_NOTNULL ){
    p = p->p_बायाँ;
  }अन्यथा{
    जबतक( p->op==TK_AND ){
      अगर( sqlite3_व्यंजक_Implies_Non_Null_Row(p->p_बायाँ, iTab, isRJ) ) वापस 1;
      p = p->p_दायाँ;
    }
  }
  w.x_व्यंजक_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_व्यंजक(&w, p);
  वापस w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
काष्ठा IdxCover {
  सूचकांक *pIdx;     /* The index to be tested for coverage */
  पूर्णांक iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
अटल पूर्णांक व्यंजक_Idx_Cover(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_COLUMN
   && p_व्यंजक->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3_Table_Column_To_सूचकांक(pWalker->u.pIdxCover->pIdx, p_व्यंजक->iColumn)<0
  ){
    pWalker->eCode = 1;
    वापस WRC_Abort;
  }
  वापस WRC_Continue;
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
पूर्णांक sqlite3_व्यंजक_Covered_By_सूचकांक(
  व्यंजक *p_व्यंजक,        /* The index to be tested */
  पूर्णांक iCur,           /* The cursor number for the corresponding table */
  सूचकांक *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  काष्ठा IdxCover xcov;
  स_रखो(&w, 0, माप(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_व्यंजक_Callback = व्यंजक_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_व्यंजक(&w, p_व्यंजक);
  वापस !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
काष्ठा RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  पूर्णांक *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
अटल पूर्णांक selectRefEnter(Walker *pWalker, Select *pSelect){
  काष्ठा RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  पूर्णांक *piNew;
  अगर( pSrc->nSrc==0 ) वापस WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*माप(पूर्णांक));
  अगर( piNew==0 ){
    p->nExclude = 0;
    वापस WRC_Abort;
  }अन्यथा{
    p->aiExclude = piNew;
  }
  क्रम(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  वापस WRC_Continue;
}
अटल व्योम selectRefLeave(Walker *pWalker, Select *pSelect){
  काष्ठा RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  अगर( p->nExclude ){
    निश्चित( p->nExclude>=pSrc->nSrc );
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
अटल पूर्णांक व्यंजक_Ref_To_Src_List(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_COLUMN
   || p_व्यंजक->op==TK_AGG_COLUMN
  ){
    पूर्णांक i;
    काष्ठा RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    पूर्णांक nSrc = pSrc ? pSrc->nSrc : 0;
    क्रम(i=0; i<nSrc; i++){
      अगर( p_व्यंजक->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        वापस WRC_Continue;
      }
    }
    क्रम(i=0; i<p->nExclude && p->aiExclude[i]!=p_व्यंजक->iTable; i++){}
    अगर( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  वापस WRC_Continue;
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
पूर्णांक sqlite3ReferencesSrcList(Parse *pParse, व्यंजक *p_व्यंजक, SrcList *pSrcList){
  Walker w;
  काष्ठा RefSrcList x;
  निश्चित( pParse->db!=0 );
  स_रखो(&w, 0, माप(w));
  स_रखो(&x, 0, माप(x));
  w.x_व्यंजक_Callback = व्यंजक_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  निश्चित( p_व्यंजक->op==TK_AGG_FUNCTION );
  निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
  sqlite3_Walk_व्यंजक_List(&w, p_व्यंजक->x.pList);
  अगर( p_व्यंजक->p_बायाँ ){
    निश्चित( p_व्यंजक->p_बायाँ->op==TK_ORDER );
    निश्चित( व्यंजक_Use_XList(p_व्यंजक->p_बायाँ) );
    निश्चित( p_व्यंजक->p_बायाँ->x.pList!=0 );
    sqlite3_Walk_व्यंजक_List(&w, p_व्यंजक->p_बायाँ->x.pList);
  }
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
  अगर( व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
    sqlite3_Walk_व्यंजक(&w, p_व्यंजक->y.pWin->pFilter);
  }
#पूर्ण_अगर
  अगर( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  अगर( w.eCode & 0x01 ){
    वापस 1;
  }अन्यथा अगर( w.eCode ){
    वापस 0;
  }अन्यथा{
    वापस -1;
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
अटल पूर्णांक agginfo_Persist_व्यंजक_Cb(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( ALWAYS(!व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly|EP_Reduced))
   && p_व्यंजक->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_व्यंजक->pAggInfo;
    पूर्णांक iAgg = p_व्यंजक->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    निश्चित( iAgg>=0 );
    अगर( p_व्यंजक->op!=TK_AGG_FUNCTION ){
      अगर( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_व्यंजक
      ){
        p_व्यंजक = sqlite3_व्यंजक_Dup(db, p_व्यंजक, 0);
        अगर( p_व्यंजक && !sqlite3_व्यंजक_Deferred_Delete(pParse, p_व्यंजक) ){
          pAggInfo->aCol[iAgg].pCExpr = p_व्यंजक;
        }
      }
    }अन्यथा{
      निश्चित( p_व्यंजक->op==TK_AGG_FUNCTION );
      अगर( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_व्यंजक
      ){
        p_व्यंजक = sqlite3_व्यंजक_Dup(db, p_व्यंजक, 0);
        अगर( p_व्यंजक && !sqlite3_व्यंजक_Deferred_Delete(pParse, p_व्यंजक) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_व्यंजक;
        }
      }
    }
  }
  वापस WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
व्योम sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  स_रखो(pWalker, 0, माप(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_व्यंजक_Callback = agginfo_Persist_व्यंजक_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
अटल पूर्णांक addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  पूर्णांक i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       माप(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  वापस i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
अटल पूर्णांक addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  पूर्णांक i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       माप(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  वापस i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
अटल व्योम findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  व्यंजक *p_व्यंजक          /* Expr describing the column to find or insert */
){
  काष्ठा AggInfo_col *pCol;
  पूर्णांक k;
  पूर्णांक mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  निश्चित( mxTerm <= SMXV(i16) );
  निश्चित( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  क्रम(k=0; k<pAggInfo->nColumn; k++, pCol++){
    अगर( pCol->pCExpr==p_व्यंजक ) वापस;
    अगर( pCol->iTable==p_व्यंजक->iTable
     && pCol->iColumn==p_व्यंजक->iColumn
     && p_व्यंजक->op!=TK_IF_NULL_ROW
    ){
      जाओ fix_up_व्यंजक;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  अगर( k<0 ){
    /* OOM on resize */
    निश्चित( pParse->db->mallocFailed );
    वापस;
  }
  अगर( k>mxTerm ){
    sqlite3_Error_संदेश(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  निश्चित( व्यंजक_Use_YTab(p_व्यंजक) );
  pCol->pTab = p_व्यंजक->y.pTab;
  pCol->iTable = p_व्यंजक->iTable;
  pCol->iColumn = p_व्यंजक->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_व्यंजक;
  अगर( pAggInfo->pGroupBy && p_व्यंजक->op!=TK_IF_NULL_ROW ){
    पूर्णांक j, n;
    व्यंजक_List *pGB = pAggInfo->pGroupBy;
    काष्ठा व्यंजक_List_item *pTerm = pGB->a;
    n = pGB->n_व्यंजक;
    क्रम(j=0; j<n; j++, pTerm++){
      व्यंजक *pE = pTerm->p_व्यंजक;
      अगर( pE->op==TK_COLUMN
       && pE->iTable==p_व्यंजक->iTable
       && pE->iColumn==p_व्यंजक->iColumn
      ){
        pCol->iSorterColumn = j;
        अवरोध;
      }
    }
  }
  अगर( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_व्यंजक:
  व्यंजक_Set_VVAProperty(p_व्यंजक, EP_NoReduce);
  निश्चित( p_व्यंजक->pAggInfo==0 || p_व्यंजक->pAggInfo==pAggInfo );
  p_व्यंजक->pAggInfo = pAggInfo;
  अगर( p_व्यंजक->op==TK_COLUMN ){
    p_व्यंजक->op = TK_AGG_COLUMN;
  }
  निश्चित( k <= SMXV(p_व्यंजक->iAgg) );
  p_व्यंजक->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
अटल पूर्णांक analyzeAggregate(Walker *pWalker, व्यंजक *p_व्यंजक){
  पूर्णांक i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  निश्चित( pNC->ncFlags & NC_UAggInfo );
  निश्चित( pAggInfo->iFirstReg==0 );
  चयन( p_व्यंजक->op ){
    शेष: {
      Indexed_व्यंजक *pIEpr;
      व्यंजक tmp;
      निश्चित( pParse->iSelfTab==0 );
      अगर( (pNC->ncFlags & NC_InAggFunc)==0 ) अवरोध;
      अगर( pParse->pIdxEpr==0 ) अवरोध;
      क्रम(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        पूर्णांक i_तथ्य_Cur = pIEpr->i_तथ्य_Cur;
        अगर( i_तथ्य_Cur<0 ) जारी;
        अगर( sqlite3_व्यंजक_Compare(0, p_व्यंजक, pIEpr->p_व्यंजक, i_तथ्य_Cur)==0 ) अवरोध;
      }
      अगर( pIEpr==0 ) अवरोध;
      अगर( NEVER(!व्यंजक_Use_YTab(p_व्यंजक)) ) अवरोध;
      क्रम(i=0; i<pSrcList->nSrc; i++){
         अगर( pSrcList->a[i].iCursor==pIEpr->i_तथ्य_Cur ){
           testcase( i>0 );
           अवरोध;
         }
      }
      अगर( i>=pSrcList->nSrc ) अवरोध;
      अगर( NEVER(p_व्यंजक->pAggInfo!=0) ) अवरोध; /* Resolved by outer context */
      अगर( pParse->nErr ){ वापस WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      स_रखो(&tmp, 0, माप(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      अगर( pParse->nErr ){ वापस WRC_Abort; }
      निश्चित( pAggInfo->aCol!=0 );
      निश्चित( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_व्यंजक;
      p_व्यंजक->pAggInfo = pAggInfo;
      p_व्यंजक->iAgg = tmp.iAgg;
      वापस WRC_Prune;
    }
    हाल TK_IF_NULL_ROW:
    हाल TK_AGG_COLUMN:
    हाल TK_COLUMN: {
      testcase( p_व्यंजक->op==TK_AGG_COLUMN );
      testcase( p_व्यंजक->op==TK_COLUMN );
      testcase( p_व्यंजक->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      अगर( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        क्रम(i=0; i<pSrcList->nSrc; i++, pItem++){
          निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly|EP_Reduced) );
          अगर( p_व्यंजक->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_व्यंजक);
            अवरोध;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      वापस WRC_Continue;
    }
    हाल TK_AGG_FUNCTION: {
      अगर( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_व्यंजक->op2
       && p_व्यंजक->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        काष्ठा AggInfo_func *pItem = pAggInfo->aFunc;
        पूर्णांक mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        निश्चित( mxTerm <= SMXV(i16) );
        क्रम(i=0; i<pAggInfo->nFunc; i++, pItem++){
          अगर( NEVER(pItem->pFExpr==p_व्यंजक) ) अवरोध;
          अगर( sqlite3_व्यंजक_Compare(0, pItem->pFExpr, p_व्यंजक, -1)==0 ){
            अवरोध;
          }
        }
        अगर( i>mxTerm ){
          sqlite3_Error_संदेश(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          निश्चित( i<pAggInfo->nFunc );
        }अन्यथा अगर( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          अगर( i>=0 ){
            पूर्णांक nArg;
            निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_व्यंजक;
            निश्चित( व्यंजक_Use_UToken(p_व्यंजक) );
            nArg = p_व्यंजक->x.pList ? p_व्यंजक->x.pList->n_व्यंजक : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_व्यंजक->u.zToken, nArg, enc, 0);
            निश्चित( pItem->bOBUnique==0 );
            अगर( p_व्यंजक->p_बायाँ
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              व्यंजक_List *pOBList;
              निश्चित( nArg>0 );
              निश्चित( p_व्यंजक->p_बायाँ->op==TK_ORDER );
              निश्चित( व्यंजक_Use_XList(p_व्यंजक->p_बायाँ) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_व्यंजक->p_बायाँ->x.pList;
              निश्चित( pOBList->n_व्यंजक>0 );
              निश्चित( pItem->bOBUnique==0 );
              अगर( pOBList->n_व्यंजक==1
               && nArg==1
               && sqlite3_व्यंजक_Compare(0,pOBList->a[0].p_व्यंजक,
                               p_व्यंजक->x.pList->a[0].p_व्यंजक,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = व्यंजक_Has_Property(p_व्यंजक, EP_Distinct);
              }अन्यथा{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }अन्यथा{
              pItem->iOBTab = -1;
            }
            अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }अन्यथा{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly|EP_Reduced) );
        व्यंजक_Set_VVAProperty(p_व्यंजक, EP_NoReduce);
        निश्चित( i <= SMXV(p_व्यंजक->iAgg) );
        p_व्यंजक->iAgg = (i16)i;
        p_व्यंजक->pAggInfo = pAggInfo;
        वापस WRC_Prune;
      }अन्यथा{
        वापस WRC_Continue;
      }
    }
  }
  वापस WRC_Continue;
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
व्योम sqlite3_व्यंजक_Analyze_Aggregates(NameContext *pNC, व्यंजक *p_व्यंजक){
  Walker w;
  w.x_व्यंजक_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  निश्चित( pNC->pSrcList!=0 );
  sqlite3_Walk_व्यंजक(&w, p_व्यंजक);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
व्योम sqlite3_व्यंजक_Analyze_Agg_List(NameContext *pNC, व्यंजक_List *pList){
  काष्ठा व्यंजक_List_item *pItem;
  पूर्णांक i;
  अगर( pList ){
    क्रम(pItem=pList->a, i=0; i<pList->n_व्यंजक; i++, pItem++){
      sqlite3_व्यंजक_Analyze_Aggregates(pNC, pItem->p_व्यंजक);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
पूर्णांक sqlite3_Get_अस्थायी_Reg(Parse *pParse){
  अगर( pParse->n_अस्थायी_Reg==0 ){
    वापस ++pParse->nMem;
  }
  वापस pParse->a_अस्थायी_Reg[--pParse->n_अस्थायी_Reg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
व्योम sqlite3_Release_अस्थायी_Reg(Parse *pParse, पूर्णांक iReg){
  अगर( iReg ){
#अगर_घोषित SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    पूर्णांक kk;
    क्रम(kk=0; kk<pParse->n_अस्थायी_Reg; kk++){
      निश्चित( pParse->a_अस्थायी_Reg[kk]!=iReg );
    }
#पूर्ण_अगर
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    अगर( pParse->n_अस्थायी_Reg<Array_आकार(pParse->a_अस्थायी_Reg) ){
      pParse->a_अस्थायी_Reg[pParse->n_अस्थायी_Reg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
पूर्णांक sqlite3_Get_अस्थायी_Range(Parse *pParse, पूर्णांक nReg){
  पूर्णांक i, n;
  अगर( nReg==1 ) वापस sqlite3_Get_अस्थायी_Reg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  अगर( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }अन्यथा{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  वापस i;
}
व्योम sqlite3_Release_अस्थायी_Range(Parse *pParse, पूर्णांक iReg, पूर्णांक nReg){
  अगर( nReg==1 ){
    sqlite3_Release_अस्थायी_Reg(pParse, iReg);
    वापस;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  अगर( nReg>pParse->nRangeReg ){
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
व्योम sqlite3_Clear_अस्थायी_Reg_Cache(Parse *pParse){
  pParse->n_अस्थायी_Reg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
व्योम sqlite3TouchRegister(Parse *pParse, पूर्णांक iReg){
  अगर( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#अगर defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
पूर्णांक sqlite3FirstAvailableRegister(Parse *pParse, पूर्णांक iMin){
  स्थिर व्यंजक_List *pList = pParse->p_Const_व्यंजक;
  अगर( pList ){
    पूर्णांक i;
    क्रम(i=0; i<pList->n_व्यंजक; i++){
      अगर( pList->a[i].u.i_Const_व्यंजक_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_व्यंजक_Reg + 1;
      }
    }
  }
  pParse->n_अस्थायी_Reg = 0;
  pParse->nRangeReg = 0;
  वापस iMin;
}
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#अगर_घोषित SQLITE_DEBUG
पूर्णांक sqlite3NoTempsInRange(Parse *pParse, पूर्णांक iFirst, पूर्णांक iLast){
  पूर्णांक i;
  अगर( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     वापस 0;
  }
  क्रम(i=0; i<pParse->n_अस्थायी_Reg; i++){
    अगर( pParse->a_अस्थायी_Reg[i]>=iFirst && pParse->a_अस्थायी_Reg[i]<=iLast ){
      वापस 0;
    }
  }
  अगर( pParse->p_Const_व्यंजक ){
    व्यंजक_List *pList = pParse->p_Const_व्यंजक;
    क्रम(i=0; i<pList->n_व्यंजक; i++){
      पूर्णांक iReg = pList->a[i].u.i_Const_व्यंजक_Reg;
      अगर( iReg==0 ) जारी;
      अगर( iReg>=iFirst && iReg<=iLast ) वापस 0;
    }
  }
  वापस 1;
}
#पूर्ण_अगर /* SQLITE_DEBUG */
