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
#அந்தர்பாவ "sqliteInt.h"

/* Forward declarations */
நிலையான வெறுமை கோவை_Code_Between(Parse*,கோவை*,முழு,வெறுமை(*)(Parse*,கோவை*,முழு,முழு),முழு);
நிலையான முழு கோவை_Code_Vector(Parse *pParse, கோவை *p, முழு *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
எழுத்து sqlite3TableColumnAffinity(மாறிலி Table *pTab, முழு iCol){
  எனில்( iCol<0 || NEVER(iCol>=pTab->nCol) ) திரும்பு SQLITE_AFF_INTEGER;
  திரும்பு pTab->aCol[iCol].affinity;
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
எழுத்து sqlite3_கோவை_Affinity(மாறிலி கோவை *p_கோவை){
  முழு op;
  op = p_கோவை->op;
  போது( 1 /* exit-by-break */ ){
    எனில்( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_கோவை->y.pTab!=0) ){
      ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
      ப்ரதிஜ்ஞா( p_கோவை->y.pTab!=0 );
      திரும்பு sqlite3TableColumnAffinity(p_கோவை->y.pTab, p_கோவை->iColumn);
    }
    எனில்( op==TK_SELECT ){
      ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை) );
      ப்ரதிஜ்ஞா( p_கோவை->x.pSelect!=0 );
      ப்ரதிஜ்ஞா( p_கோவை->x.pSelect->pEList!=0 );
      ப்ரதிஜ்ஞா( p_கோவை->x.pSelect->pEList->a[0].p_கோவை!=0 );
      திரும்பு sqlite3_கோவை_Affinity(p_கோவை->x.pSelect->pEList->a[0].p_கோவை);
    }
#ifndef SQLITE_OMIT_CAST
    எனில்( op==TK_CAST ){
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      திரும்பு sqlite3AffinityType(p_கோவை->u.zToken, 0);
    }
#endif
    எனில்( op==TK_SELECT_COLUMN ){
      ப்ரதிஜ்ஞா( p_கோவை->p_இடது!=0 && கோவை_Use_XSelect(p_கோவை->p_இடது) );
      ப்ரதிஜ்ஞா( p_கோவை->iColumn < p_கோவை->iTable );
      ப்ரதிஜ்ஞா( p_கோவை->iColumn >= 0 );
      ப்ரதிஜ்ஞா( p_கோவை->iTable==p_கோவை->p_இடது->x.pSelect->pEList->n_கோவை );
      திரும்பு sqlite3_கோவை_Affinity(
          p_கோவை->p_இடது->x.pSelect->pEList->a[p_கோவை->iColumn].p_கோவை
      );
    }
    எனில்( op==TK_VECTOR
     || (op==TK_FUNCTION && p_கோவை->aff_கோவை==SQLITE_AFF_DEFER)
    ){
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
      திரும்பு sqlite3_கோவை_Affinity(p_கோவை->x.pList->a[0].p_கோவை);
    }
    எனில்( கோவை_Has_Property(p_கோவை, EP_Skip|EP_IfNullRow) ){
      ப்ரதிஜ்ஞா( p_கோவை->op==TK_COLLATE
           || p_கோவை->op==TK_IF_NULL_ROW
           || (p_கோவை->op==TK_REGISTER && p_கோவை->op2==TK_IF_NULL_ROW) );
      p_கோவை = p_கோவை->p_இடது;
      op = p_கோவை->op;
      தொடர்;
    }
    எனில்( op!=TK_REGISTER ) நிறுத்து;
    op = p_கோவை->op2;
    எனில்( NEVER( op==TK_REGISTER ) ) நிறுத்து;
  }
  திரும்பு p_கோவை->aff_கோவை;
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
முழு sqlite3_கோவை_Data_Type(மாறிலி கோவை *p_கோவை){
  போது( p_கோவை ){
    தேர்வு( p_கோவை->op ){
      நிலை TK_COLLATE:
      நிலை TK_IF_NULL_ROW:
      நிலை TK_UPLUS:  {
        p_கோவை = p_கோவை->p_இடது;
        நிறுத்து;
      }
      நிலை TK_NULL: {
        p_கோவை = 0;
        நிறுத்து;
      }
      நிலை TK_STRING: {
        திரும்பு 0x02;
      }
      நிலை TK_BLOB: {
        திரும்பு 0x04;
      }
      நிலை TK_CONCAT: {
        திரும்பு 0x06;
      }
      நிலை TK_VARIABLE:
      நிலை TK_AGG_FUNCTION:
      நிலை TK_FUNCTION: {
        திரும்பு 0x07;
      }
      நிலை TK_COLUMN:
      நிலை TK_AGG_COLUMN:
      நிலை TK_SELECT:
      நிலை TK_CAST:
      நிலை TK_SELECT_COLUMN:
      நிலை TK_VECTOR:  {
        முழு aff = sqlite3_கோவை_Affinity(p_கோவை);
        எனில்( aff>=SQLITE_AFF_NUMERIC ) திரும்பு 0x05;
        எனில்( aff==SQLITE_AFF_TEXT )    திரும்பு 0x06;
        திரும்பு 0x07;
      }
      நிலை TK_CASE: {
        முழு res = 0;
        முழு ii;
        கோவை_List *pList = p_கோவை->x.pList;
        ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) && pList!=0 );
        ப்ரதிஜ்ஞா( pList->n_கோவை > 0);
        க்கு(ii=1; ii<pList->n_கோவை; ii+=2){
          res |= sqlite3_கோவை_Data_Type(pList->a[ii].p_கோவை);
        }
        எனில்( pList->n_கோவை % 2 ){
          res |= sqlite3_கோவை_Data_Type(pList->a[pList->n_கோவை-1].p_கோவை);
        }
        திரும்பு res;
      }
      இயல்பு: {
        திரும்பு 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  திரும்பு 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
கோவை *sqlite3_கோவை_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  கோவை *p_கோவை,             /* Add the "COLLATE" clause to this expression */
  மாறிலி Token *pCollName,  /* Name of collating sequence */
  முழு dequote              /* True to dequote pCollName */
){
  எனில்( pCollName->n>0 && pParse->nErr==0 ){
    கோவை *pNew = sqlite3_கோவை_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    ப்ரதிஜ்ஞா( p_கோவை!=0 );
    எனில்( pNew ){
      pNew->p_இடது = p_கோவை;
      pNew->flags |= EP_Collate|EP_Skip;
#எனில் SQLITE_MAX_கோவை_DEPTH>0
      pNew->nHeight = p_கோவை->nHeight+1;
      sqlite3_கோவை_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_கோவை = pNew;
    }
  }
  திரும்பு p_கோவை;
}
கோவை *sqlite3_கோவை_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  கோவை *p_கோவை,          /* Add the "COLLATE" clause to this expression */
  மாறிலி எழுத்து *zC        /* The collating sequence name */
){
  Token s;
  ப்ரதிஜ்ஞா( zC!=0 );
  sqlite3TokenInit(&s, (எழுத்து*)zC);
  திரும்பு sqlite3_கோவை_Add_Collate_Token(pParse, p_கோவை, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
கோவை *sqlite3_கோவை_Skip_Collate(கோவை *p_கோவை){
  போது( p_கோவை && கோவை_Has_Property(p_கோவை, EP_Skip) ){
    ப்ரதிஜ்ஞா( p_கோவை->op==TK_COLLATE );
    p_கோவை = p_கோவை->p_இடது;
  }  
  திரும்பு p_கோவை;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
கோவை *sqlite3_கோவை_Skip_Collate_And_Likely(கோவை *p_கோவை){
  போது( p_கோவை && கோவை_Has_Property(p_கோவை, EP_Skip|EP_Unlikely) ){
    எனில்( கோவை_Has_Property(p_கோவை, EP_Unlikely) ){
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
      ப்ரதிஜ்ஞா( p_கோவை->x.pList->n_கோவை>0 );
      ப்ரதிஜ்ஞா( p_கோவை->op==TK_FUNCTION );
      p_கோவை = p_கோவை->x.pList->a[0].p_கோவை;
    }இல்லையெனில் எனில்( p_கோவை->op==TK_COLLATE ){
      p_கோவை = p_கோவை->p_இடது;
    }இல்லையெனில்{
      நிறுத்து;
    }
  }  
  திரும்பு p_கோவை;
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
CollSeq *sqlite3_கோவை_Coll_Seq(Parse *pParse, மாறிலி கோவை *p_கோவை){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  மாறிலி கோவை *p = p_கோவை;
  போது( p ){
    முழு op = p->op;
    எனில்( op==TK_REGISTER ) op = p->op2;
    எனில்( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      முழு j;
      ப்ரதிஜ்ஞா( கோவை_Use_YTab(p) );
      ப்ரதிஜ்ஞா( p->y.pTab!=0 );
      எனில்( (j = p->iColumn)>=0 ){
        மாறிலி எழுத்து *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      நிறுத்து;
    }
    எனில்( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_இடது;
      தொடர்;
    }
    எனில்( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_கோவை==SQLITE_AFF_DEFER)
    ){
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p) );
      p = p->x.pList->a[0].p_கோவை;
      தொடர்;
    }
    எனில்( op==TK_COLLATE ){
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p, EP_Int_மதிப்பு) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      நிறுத்து;
    }
    எனில்( p->flags & EP_Collate ){
      எனில்( p->p_இடது && (p->p_இடது->flags & EP_Collate)!=0 ){
        p = p->p_இடது;
      }இல்லையெனில்{
        கோவை *pNext  = p->p_வலது;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        ப்ரதிஜ்ஞா( !கோவை_Use_XList(p) || p->x.pList==0 || p->p_வலது==0 );
        எனில்( கோவை_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          முழு i;
          க்கு(i=0; i<p->x.pList->n_கோவை; i++){
            எனில்( கோவை_Has_Property(p->x.pList->a[i].p_கோவை, EP_Collate) ){
              pNext = p->x.pList->a[i].p_கோவை;
              நிறுத்து;
            }
          }
        }
        p = pNext;
      }
    }இல்லையெனில்{
      நிறுத்து;
    }
  }
  எனில்( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  திரும்பு pColl;
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
CollSeq *sqlite3_கோவை_NNColl_Seq(Parse *pParse, மாறிலி கோவை *p_கோவை){
  CollSeq *p = sqlite3_கோவை_Coll_Seq(pParse, p_கோவை);
  எனில்( p==0 ) p = pParse->db->pDfltColl;
  ப்ரதிஜ்ஞா( p!=0 );
  திரும்பு p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
முழு sqlite3_கோவை_Coll_Seq_Match(Parse *pParse, மாறிலி கோவை *pE1, மாறிலி கோவை *pE2){
  CollSeq *pColl1 = sqlite3_கோவை_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_கோவை_NNColl_Seq(pParse, pE2);
  ப்ரதிஜ்ஞா( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  திரும்பு pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
எழுத்து sqlite3CompareAffinity(மாறிலி கோவை *p_கோவை, எழுத்து aff2){
  எழுத்து aff1 = sqlite3_கோவை_Affinity(p_கோவை);
  எனில்( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    எனில்( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      திரும்பு SQLITE_AFF_NUMERIC;
    }இல்லையெனில்{
      திரும்பு SQLITE_AFF_BLOB;
    }
  }இல்லையெனில்{
    /* One side is a column, the other is not. Use the columns affinity. */
    ப்ரதிஜ்ஞா( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    திரும்பு (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
நிலையான எழுத்து comparisonAffinity(மாறிலி கோவை *p_கோவை){
  எழுத்து aff;
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_EQ || p_கோவை->op==TK_IN || p_கோவை->op==TK_LT ||
          p_கோவை->op==TK_GT || p_கோவை->op==TK_GE || p_கோவை->op==TK_LE ||
          p_கோவை->op==TK_NE || p_கோவை->op==TK_IS || p_கோவை->op==TK_ISNOT );
  ப்ரதிஜ்ஞா( p_கோவை->p_இடது );
  ப்ரதிஜ்ஞா( !கோவை_Use_XSelect(p_கோவை) );
  aff = sqlite3_கோவை_Affinity(p_கோவை->p_இடது);
  எனில்( p_கோவை->p_வலது ){
    aff = sqlite3CompareAffinity(p_கோவை->p_வலது, aff);
  }இல்லையெனில் எனில்( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  திரும்பு aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
முழு sqlite3IndexAffinityOk(மாறிலி கோவை *p_கோவை, எழுத்து idx_affinity){
  எழுத்து aff = comparisonAffinity(p_கோவை);
  எனில்( aff<SQLITE_AFF_TEXT ){
    திரும்பு 1;
  }
  எனில்( aff==SQLITE_AFF_TEXT ){
    திரும்பு idx_affinity==SQLITE_AFF_TEXT;
  }
  திரும்பு sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
நிலையான u8 இருமம்_Compare_P5(
  மாறிலி கோவை *pExpr1,   /* Left operand */
  மாறிலி கோவை *pExpr2,   /* Right operand */
  முழு jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (எழுத்து)sqlite3_கோவை_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  திரும்பு aff;
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
CollSeq *sqlite3_இருமம்_Compare_Coll_Seq(
  Parse *pParse,
  மாறிலி கோவை *p_இடது,
  மாறிலி கோவை *p_வலது
){
  CollSeq *pColl;
  ப்ரதிஜ்ஞா( p_இடது );
  எனில்( p_இடது->flags & EP_Collate ){
    pColl = sqlite3_கோவை_Coll_Seq(pParse, p_இடது);
  }இல்லையெனில் எனில்( p_வலது && (p_வலது->flags & EP_Collate)!=0 ){
    pColl = sqlite3_கோவை_Coll_Seq(pParse, p_வலது);
  }இல்லையெனில்{
    pColl = sqlite3_கோவை_Coll_Seq(pParse, p_இடது);
    எனில்( !pColl ){
      pColl = sqlite3_கோவை_Coll_Seq(pParse, p_வலது);
    }
  }
  திரும்பு pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_கோவை_Compare_Coll_Seq(Parse *pParse, மாறிலி கோவை *p){
  எனில்( கோவை_Has_Property(p, EP_Commuted) ){
    திரும்பு sqlite3_இருமம்_Compare_Coll_Seq(pParse, p->p_வலது, p->p_இடது);
  }இல்லையெனில்{
    திரும்பு sqlite3_இருமம்_Compare_Coll_Seq(pParse, p->p_இடது, p->p_வலது);
  }
}

/*
** Generate code for a comparison operator.
*/
நிலையான முழு codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  கோவை *p_இடது,      /* The left operand */
  கோவை *p_வலது,     /* The right operand */
  முழு opcode,       /* The comparison opcode */
  முழு in1, முழு in2, /* Register holding operands */
  முழு dest,         /* Jump here if true.  */
  முழு jumpIfNull,   /* If true, jump if either operand is NULL */
  முழு isCommuted    /* The comparison has been commuted */
){
  முழு p5;
  முழு addr;
  CollSeq *p4;

  எனில்( pParse->nErr ) திரும்பு 0;
  எனில்( isCommuted ){
    p4 = sqlite3_இருமம்_Compare_Coll_Seq(pParse, p_வலது, p_இடது);
  }இல்லையெனில்{
    p4 = sqlite3_இருமம்_Compare_Coll_Seq(pParse, p_இடது, p_வலது);
  }
  p5 = இருமம்_Compare_P5(p_இடது, p_வலது, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (வெறுமை*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  திரும்பு addr;
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
முழு sqlite3_கோவை_Is_Vector(மாறிலி கோவை *p_கோவை){
  திரும்பு sqlite3_கோவை_Vector_Size(p_கோவை)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
முழு sqlite3_கோவை_Vector_Size(மாறிலி கோவை *p_கோவை){
  u8 op = p_கோவை->op;
  எனில்( op==TK_REGISTER ) op = p_கோவை->op2;
  எனில்( op==TK_VECTOR ){
    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
    திரும்பு p_கோவை->x.pList->n_கோவை;
  }இல்லையெனில் எனில்( op==TK_SELECT ){
    ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை) );
    திரும்பு p_கோவை->x.pSelect->pEList->n_கோவை;
  }இல்லையெனில்{
    திரும்பு 1;
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
கோவை *sqlite3VectorFieldSubexpr(கோவை *pVector, முழு i){
  ப்ரதிஜ்ஞா( i<sqlite3_கோவை_Vector_Size(pVector) || pVector->op==TK_ERROR );
  எனில்( sqlite3_கோவை_Is_Vector(pVector) ){
    ப்ரதிஜ்ஞா( pVector->op2==0 || pVector->op==TK_REGISTER );
    எனில்( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      ப்ரதிஜ்ஞா( கோவை_Use_XSelect(pVector) );
      திரும்பு pVector->x.pSelect->pEList->a[i].p_கோவை;
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( கோவை_Use_XList(pVector) );
      திரும்பு pVector->x.pList->a[i].p_கோவை;
    }
  }
  திரும்பு pVector;
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
கோவை *sqlite3_கோவை_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  கோவை *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  முழு iField,          /* Which column of the vector to return */
  முழு nField           /* Total number of columns in the vector */
){
  கோவை *pRet;
  எனில்( pVector->op==TK_SELECT ){
    ப்ரதிஜ்ஞா( கோவை_Use_XSelect(pVector) );
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
    எனில்( pRet ){
      கோவை_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_இடது = pVector;
    }
  }இல்லையெனில்{
    எனில்( pVector->op==TK_VECTOR ){
      கோவை **ppVector;
      ப்ரதிஜ்ஞா( கோவை_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_கோவை;
      pVector = *ppVector;
      எனில்( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        திரும்பு pVector;
      }
    }
    pRet = sqlite3_கோவை_Dup(pParse->db, pVector, 0);
  }
  திரும்பு pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
நிலையான முழு கோவை_Code_Subselect(Parse *pParse, கோவை *p_கோவை){
  முழு reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  எனில்( p_கோவை->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_கோவை);
  }
#endif
  திரும்பு reg;
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
நிலையான முழு கோவை_Vector_Register(
  Parse *pParse,                  /* Parse context */
  கோவை *pVector,                  /* Vector to extract element from */
  முழு iField,                     /* Field to extract from pVector */
  முழு regSelect,                  /* First in array of registers */
  கோவை *pTmp,                     /* Temporary space */
  கோவை **pp_கோவை,                  /* OUT: Expression element */
  முழு *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  ப்ரதிஜ்ஞா( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  எனில்( op==TK_REGISTER ){
    *pp_கோவை = sqlite3VectorFieldSubexpr(pVector, iField);
    திரும்பு pVector->iTable+iField;
  }
  எனில்( op==TK_SELECT ){
    ப்ரதிஜ்ஞா( கோவை_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, அளவு(கோவை));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_இடது = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_கோவை;
    *pp_கோவை = pTmp;
    திரும்பு regSelect+iField;
  }
  எனில்( op==TK_VECTOR ){
    ப்ரதிஜ்ஞா( கோவை_Use_XList(pVector) );
    *pp_கோவை = pVector->x.pList->a[iField].p_கோவை;
    திரும்பு sqlite3_கோவை_Code_Temp(pParse, *pp_கோவை, pRegFree);
  }
  திரும்பு 0;
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
நிலையான வெறுமை codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  கோவை *p_கோவை,          /* The comparison operation */
  முழு dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  கோவை *p_இடது = p_கோவை->p_இடது;
  கோவை *p_வலது = p_கோவை->p_வலது;
  முழு n_இடது = sqlite3_கோவை_Vector_Size(p_இடது);
  முழு i;
  முழு reg_இடது = 0;
  முழு reg_வலது = 0;
  u8 opx = op;
  முழு addrCmp = 0;
  முழு addrDone = sqlite3VdbeMakeLabel(pParse);
  முழு isCommuted = கோவை_Has_Property(p_கோவை,EP_Commuted);

  ப்ரதிஜ்ஞா( !கோவை_Has_VVAProperty(p_கோவை,EP_Immutable) );
  எனில்( pParse->nErr ) திரும்பு;
  எனில்( n_இடது!=sqlite3_கோவை_Vector_Size(p_வலது) ){
    sqlite3_Error_செய்தி(pParse, "row value misused");
    திரும்பு;
  }
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_EQ || p_கோவை->op==TK_NE
       || p_கோவை->op==TK_IS || p_கோவை->op==TK_ISNOT
       || p_கோவை->op==TK_LT || p_கோவை->op==TK_GT
       || p_கோவை->op==TK_LE || p_கோவை->op==TK_GE
  );
  ப்ரதிஜ்ஞா( p_கோவை->op==op || (p_கோவை->op==TK_IS && op==TK_EQ)
            || (p_கோவை->op==TK_ISNOT && op==TK_NE) );
  ப்ரதிஜ்ஞா( p5==0 || p_கோவை->op!=op );
  ப்ரதிஜ்ஞா( p5==SQLITE_NULLEQ || p_கோவை->op==op );

  எனில்( op==TK_LE ) opx = TK_LT;
  எனில்( op==TK_GE ) opx = TK_GT;
  எனில்( op==TK_NE ) opx = TK_EQ;

  reg_இடது = கோவை_Code_Subselect(pParse, p_இடது);
  reg_வலது = கோவை_Code_Subselect(pParse, p_வலது);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  க்கு(i=0; 1 /*Loop exits by "break"*/; i++){
    முழு regFree1 = 0, regFree2 = 0;
    கோவை *pL = 0, *pR = 0;
    கோவை tmp1, tmp2;
    முழு r1, r2;
    ப்ரதிஜ்ஞா( i>=0 && i<n_இடது );
    எனில்( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = கோவை_Vector_Register(pParse, p_இடது, i, reg_இடது, &tmp1, &pL, &regFree1);
    r2 = கோவை_Vector_Register(pParse, p_வலது, i, reg_வலது, &tmp2, &pR, &regFree2);
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
    எனில்( (opx==TK_LT || opx==TK_GT) && i<n_இடது-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    எனில்( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }இல்லையெனில்{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    எனில்( i==n_இடது-1 ){
      நிறுத்து;
    }
    எனில்( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      எனில்( i==n_இடது-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  எனில்( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#எனில் SQLITE_MAX_கோவை_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
முழு sqlite3_கோவை_Check_Height(Parse *pParse, முழு nHeight){
  முழு rc = SQLITE_OK;
  முழு mxHeight = pParse->db->aLimit[SQLITE_LIMIT_கோவை_DEPTH];
  எனில்( nHeight>mxHeight ){
    sqlite3_Error_செய்தி(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  திரும்பு rc;
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
நிலையான வெறுமை height_Of_கோவை(மாறிலி கோவை *p, முழு *pnHeight){
  எனில்( p ){
    எனில்( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
நிலையான வெறுமை height_Of_கோவை_List(மாறிலி கோவை_List *p, முழு *pnHeight){
  எனில்( p ){
    முழு i;
    க்கு(i=0; i<p->n_கோவை; i++){
      height_Of_கோவை(p->a[i].p_கோவை, pnHeight);
    }
  }
}
நிலையான வெறுமை heightOfSelect(மாறிலி Select *pSelect, முழு *pnHeight){
  மாறிலி Select *p;
  க்கு(p=pSelect; p; p=p->pPrior){
    height_Of_கோவை(p->pWhere, pnHeight);
    height_Of_கோவை(p->pHaving, pnHeight);
    height_Of_கோவை(p->pLimit, pnHeight);
    height_Of_கோவை_List(p->pEList, pnHeight);
    height_Of_கோவை_List(p->pGroupBy, pnHeight);
    height_Of_கோவை_List(p->pOrderBy, pnHeight);
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
நிலையான வெறுமை கோவை_Set_Height(கோவை *p){
  முழு nHeight = p->p_இடது ? p->p_இடது->nHeight : 0;
  எனில்( NEVER(p->p_வலது) && p->p_வலது->nHeight>nHeight ){
    nHeight = p->p_வலது->nHeight;
  }
  எனில்( கோவை_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }இல்லையெனில் எனில்( p->x.pList ){
    height_Of_கோவை_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_கோவை_List_Flags(p->x.pList);
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
வெறுமை sqlite3_கோவை_Set_Height_And_Flags(Parse *pParse, கோவை *p){
  எனில்( pParse->nErr ) திரும்பு;
  கோவை_Set_Height(p);
  sqlite3_கோவை_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
முழு sqlite3_Select_கோவை_Height(மாறிலி Select *p){
  முழு nHeight = 0;
  heightOfSelect(p, &nHeight);
  திரும்பு nHeight;
}
#இல்லையெனில் /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
வெறுமை sqlite3_கோவை_Set_Height_And_Flags(Parse *pParse, கோவை *p){
  எனில்( pParse->nErr ) திரும்பு;
  எனில்( p && கோவை_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_கோவை_List_Flags(p->x.pList);
  }
}
#define கோவை_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
வெறுமை sqlite3_கோவை_Set_Error_Offset(கோவை *p_கோவை, முழு iOfst){
  எனில்( p_கோவை==0 ) திரும்பு;
  எனில்( NEVER(கோவை_Use_WJoin(p_கோவை)) ) திரும்பு;
  p_கோவை->w.iOfst = iOfst;
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
கோவை *sqlite3_கோவை_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  முழு op,                 /* Expression opcode */
  மாறிலி Token *pToken,    /* Token argument.  Might be NULL */
  முழு dequote             /* True to dequote */
){
  கோவை *pNew;
  முழு nExtra = pToken ? pToken->n+1 : 0;

  ப்ரதிஜ்ஞா( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, அளவு(கோவை)+nExtra);
  எனில்( pNew ){
    memset(pNew, 0, அளவு(கோவை));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    எனில்( nExtra ){
      ப்ரதிஜ்ஞா( pToken!=0 );
      pNew->u.zToken = (எழுத்து*)&pNew[1];
      ப்ரதிஜ்ஞா( pToken->z!=0 || pToken->n==0 );
      எனில்( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      எனில்( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_கோவை(pNew);
      }
    }
#எனில் SQLITE_MAX_கோவை_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  திரும்பு pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
கோவை *sqlite3_கோவை(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  முழு op,                 /* Expression opcode */
  மாறிலி எழுத்து *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  திரும்பு sqlite3_கோவை_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
கோவை *sqlite3_கோவை_Int32(sqlite3 *db, முழு iVal){
  கோவை *pNew = sqlite3DbMallocRawNN(db, அளவு(கோவை));
  எனில்( pNew ){
    memset(pNew, 0, அளவு(கோவை));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_மதிப்பு|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_மதிப்பு = iVal;
#எனில் SQLITE_MAX_கோவை_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  திரும்பு pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
வெறுமை sqlite3_கோவை_Attach_Subtrees(
  sqlite3 *db,
  கோவை *p_வேர்,
  கோவை *p_இடது,
  கோவை *p_வலது
){
  எனில்( p_வேர்==0 ){
    ப்ரதிஜ்ஞா( db->mallocFailed );
    sqlite3_கோவை_Delete(db, p_இடது);
    sqlite3_கோவை_Delete(db, p_வலது);
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_வேர்) );
    ப்ரதிஜ்ஞா( p_வேர்->x.pSelect==0 );
    எனில்( p_வலது ){
      p_வேர்->p_வலது = p_வலது;
      p_வேர்->flags |= EP_Propagate & p_வலது->flags;
#எனில் SQLITE_MAX_கோவை_DEPTH>0
      p_வேர்->nHeight = p_வலது->nHeight+1;
    }இல்லையெனில்{
      p_வேர்->nHeight = 1;
#endif
    }
    எனில்( p_இடது ){
      p_வேர்->p_இடது = p_இடது;
      p_வேர்->flags |= EP_Propagate & p_இடது->flags;
#எனில் SQLITE_MAX_கோவை_DEPTH>0
      எனில்( p_இடது->nHeight>=p_வேர்->nHeight ){
        p_வேர்->nHeight = p_இடது->nHeight+1;
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
கோவை *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  முழு op,                 /* Expression opcode */
  கோவை *p_இடது,            /* Left operand */
  கோவை *p_வலது            /* Right operand */
){
  கோவை *p;
  p = sqlite3DbMallocRawNN(pParse->db, அளவு(கோவை));
  எனில்( p ){
    memset(p, 0, அளவு(கோவை));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_கோவை_Attach_Subtrees(pParse->db, p, p_இடது, p_வலது);
    sqlite3_கோவை_Check_Height(pParse, p->nHeight);
  }இல்லையெனில்{
    sqlite3_கோவை_Delete(pParse->db, p_இடது);
    sqlite3_கோவை_Delete(pParse->db, p_வலது);
  }
  திரும்பு p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
வெறுமை sqlite3PExprAddSelect(Parse *pParse, கோவை *p_கோவை, Select *pSelect){
  எனில்( p_கோவை ){
    p_கோவை->x.pSelect = pSelect;
    கோவை_Set_Property(p_கோவை, EP_xIsSelect|EP_Subquery);
    sqlite3_கோவை_Set_Height_And_Flags(pParse, p_கோவை);
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( pParse->db->mallocFailed );
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
Select *sqlite3_கோவை_List_To_மதிப்புகள்(Parse *pParse, முழு nElem, கோவை_List *pEList){
  முழு ii;
  Select *pRet = 0;
  ப்ரதிஜ்ஞா( nElem>1 );
  க்கு(ii=0; ii<pEList->n_கோவை; ii++){
    Select *pSel;
    கோவை *p_கோவை = pEList->a[ii].p_கோவை;
    முழு n_கோவை_Elem;
    எனில்( p_கோவை->op==TK_VECTOR ){
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
      n_கோவை_Elem = p_கோவை->x.pList->n_கோவை;
    }இல்லையெனில்{
      n_கோவை_Elem = 1;
    }
    எனில்( n_கோவை_Elem!=nElem ){
      sqlite3_Error_செய்தி(pParse, "IN(...) element has %d term%s - expected %d",
          n_கோவை_Elem, n_கோவை_Elem>1?"s":"", nElem
      );
      நிறுத்து;
    }
    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
    pSel = sqlite3SelectNew(pParse, p_கோவை->x.pList, 0, 0, 0, 0, 0, SF_மதிப்புகள்,0);
    p_கோவை->x.pList = 0;
    எனில்( pSel ){
      எனில்( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  எனில்( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_மதிப்பு;
  }
  sqlite3_கோவை_List_Delete(pParse->db, pEList);
  திரும்பு pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
கோவை *sqlite3_கோவை_And(Parse *pParse, கோவை *p_இடது, கோவை *p_வலது){
  sqlite3 *db = pParse->db;
  எனில்( p_இடது==0  ){
    திரும்பு p_வலது;
  }இல்லையெனில் எனில்( p_வலது==0 ){
    திரும்பு p_இடது;
  }இல்லையெனில்{
    u32 f = p_இடது->flags | p_வலது->flags;
    எனில்( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      கோவை *p;
      sqlite3_கோவை_Deferred_Delete(pParse, p_இடது);
      sqlite3_கோவை_Deferred_Delete(pParse, p_வலது);
      p = sqlite3_கோவை_Int32(db, 0);
      எனில்( p ) p->op2 = 1;
      திரும்பு p;
    }இல்லையெனில்{
      திரும்பு sqlite3PExpr(pParse, TK_AND, p_இடது, p_வலது);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
கோவை *sqlite3_கோவை_Function(
  Parse *pParse,        /* Parsing context */
  கோவை_List *pList,      /* Argument list */
  மாறிலி Token *pToken,  /* Name of the function */
  முழு eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  கோவை *pNew;
  sqlite3 *db = pParse->db;
  ப்ரதிஜ்ஞா( pToken );
  pNew = sqlite3_கோவை_Alloc(db, TK_FUNCTION, pToken, 1);
  எனில்( pNew==0 ){
    sqlite3_கோவை_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    திரும்பு 0;
  }
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (முழு)(pToken->z - pParse->zTail);
  எனில்( pList
   && pList->n_கோவை > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_செய்தி(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  கோவை_Set_Property(pNew, EP_HasFunc);
  ப்ரதிஜ்ஞா( கோவை_Use_XList(pNew) );
  sqlite3_கோவை_Set_Height_And_Flags(pParse, pNew);
  எனில்( eDistinct==SF_Distinct ) கோவை_Set_Property(pNew, EP_Distinct);
  திரும்பு pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
வெறுமை sqlite3_கோவை_Order_By_Aggregate_Error(Parse *pParse, கோவை *p){
  sqlite3_Error_செய்தி(pParse,
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
வெறுமை sqlite3_கோவை_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  கோவை *p_கோவை,          /* The function call to which ORDER BY is to be added */
  கோவை_List *pOrderBy    /* The ORDER BY clause to add */
){
  கோவை *pOB;
  sqlite3 *db = pParse->db;
  எனில்( NEVER(pOrderBy==0) ){
    ப்ரதிஜ்ஞா( db->mallocFailed );
    திரும்பு;
  }
  எனில்( p_கோவை==0 ){
    ப்ரதிஜ்ஞா( db->mallocFailed );
    sqlite3_கோவை_List_Delete(db, pOrderBy);
    திரும்பு;
  }
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_FUNCTION );
  ப்ரதிஜ்ஞா( p_கோவை->p_இடது==0 );
  ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
  எனில்( p_கோவை->x.pList==0 || NEVER(p_கோவை->x.pList->n_கோவை==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_கோவை_List_Delete_Generic, pOrderBy);
    திரும்பு;
  }
  எனில்( IsWindowFunc(p_கோவை) ){
    sqlite3_கோவை_Order_By_Aggregate_Error(pParse, p_கோவை);
    sqlite3_கோவை_List_Delete(db, pOrderBy);
    திரும்பு;
  }
  எனில்( pOrderBy->n_கோவை>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_செய்தி(pParse, "too many terms in ORDER BY clause");
    sqlite3_கோவை_List_Delete(db, pOrderBy);
    திரும்பு;
  }

  pOB = sqlite3_கோவை_Alloc(db, TK_ORDER, 0, 0);
  எனில்( pOB==0 ){
    sqlite3_கோவை_List_Delete(db, pOrderBy);
    திரும்பு;
  }
  pOB->x.pList = pOrderBy;
  ப்ரதிஜ்ஞா( கோவை_Use_XList(pOB) );
  p_கோவை->p_இடது = pOB;
  கோவை_Set_Property(pOB, EP_FullSize);
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
வெறுமை sqlite3_கோவை_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  மாறிலி கோவை *p_கோவை,     /* The function invocation */
  மாறிலி FuncDef *pDef    /* The function being invoked */
){
  ப்ரதிஜ்ஞா( !IN_RENAME_OBJECT );
  ப்ரதிஜ்ஞா( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  எனில்( கோவை_Has_Property(p_கோவை, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    எனில்( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_செய்தி(pParse, "unsafe use of %#T()", p_கோவை);
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
வெறுமை sqlite3_கோவை_Assign_Var_எண்(Parse *pParse, கோவை *p_கோவை, u32 n){
  sqlite3 *db = pParse->db;
  மாறிலி எழுத்து *z;
  ynVar x;

  எனில்( p_கோவை==0 ) திரும்பு;
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு|EP_Reduced|EP_TokenOnly) );
  z = p_கோவை->u.zToken;
  ப்ரதிஜ்ஞா( z!=0 );
  ப்ரதிஜ்ஞா( z[0]!=0 );
  ப்ரதிஜ்ஞா( n==(u32)sqlite3Strlen30(z) );
  எனில்( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    ப்ரதிஜ்ஞா( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }இல்லையெனில்{
    முழு doAdd = 0;
    எனில்( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      முழு bOk;
      எனில்( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }இல்லையெனில்{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_எண்]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_எண்] );
      எனில்( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_எண்] ){
        sqlite3_Error_செய்தி(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_எண்]);
        sqlite3_Record_Error_Offset_Of_கோவை(pParse->db, p_கோவை);
        திரும்பு;
      }
      x = (ynVar)i;
      எனில்( x>pParse->nVar ){
        pParse->nVar = (முழு)x;
        doAdd = 1;
      }இல்லையெனில் எனில்( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }இல்லையெனில் எனில்( sqlite3_VList_இலக்கம்_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }இல்லையெனில்{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_இலக்கம்(pParse->pVList, z, n);
      எனில்( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    எனில்( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      எனில்( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_கோவை->iColumn = x;
  எனில்( x>db->aLimit[SQLITE_LIMIT_VARIABLE_எண்] ){
    sqlite3_Error_செய்தி(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_கோவை(pParse->db, p_கோவை);
  }
}

/*
** Recursively delete an expression tree.
*/
நிலையான SQLITE_NOINLINE வெறுமை sqlite3_கோவை_Delete_NN(sqlite3 *db, கோவை *p){
  ப்ரதிஜ்ஞா( p!=0 );
  ப்ரதிஜ்ஞா( db!=0 );
கோவை_Delete_Restart:
  ப்ரதிஜ்ஞா( !கோவை_Use_UValue(p) || p->u.i_மதிப்பு>=0 );
  ப்ரதிஜ்ஞா( !கோவை_Use_YWin(p) || !கோவை_Use_YSub(p) );
  ப்ரதிஜ்ஞா( !கோவை_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  ப்ரதிஜ்ஞா( p->op!=TK_FUNCTION || !கோவை_Use_YSub(p) );
#ifdef SQLITE_DEBUG
  எனில்( கோவை_Has_Property(p, EP_Leaf) && !கோவை_Has_Property(p, EP_TokenOnly) ){
    ப்ரதிஜ்ஞா( p->p_இடது==0 );
    ப்ரதிஜ்ஞா( p->p_வலது==0 );
    ப்ரதிஜ்ஞா( !கோவை_Use_XSelect(p) || p->x.pSelect==0 );
    ப்ரதிஜ்ஞா( !கோவை_Use_XList(p) || p->x.pList==0 );
  }
#endif
  எனில்( !கோவை_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    ப்ரதிஜ்ஞா( (கோவை_Use_XList(p) && p->x.pList==0) || p->p_வலது==0 );
    எனில்( p->p_வலது ){
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p, EP_WinFunc) );
      sqlite3_கோவை_Delete_NN(db, p->p_வலது);
    }இல்லையெனில் எனில்( கோவை_Use_XSelect(p) ){
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }இல்லையெனில்{
      sqlite3_கோவை_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      எனில்( கோவை_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    எனில்( p->p_இடது && p->op!=TK_SELECT_COLUMN ){
      கோவை *p_இடது = p->p_இடது;
      எனில்( !கோவை_Has_Property(p, EP_Static)
       && !கோவை_Has_Property(p_இடது, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_இடது;
        செல் கோவை_Delete_Restart;
      }இல்லையெனில்{
        sqlite3_கோவை_Delete_NN(db, p_இடது);
      }
    }
  }
  எனில்( !கோவை_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
வெறுமை sqlite3_கோவை_Delete(sqlite3 *db, கோவை *p){
  எனில்( p ) sqlite3_கோவை_Delete_NN(db, p);
}
வெறுமை sqlite3_கோவை_Delete_Generic(sqlite3 *db, வெறுமை *p){
  எனில்( ALWAYS(p) ) sqlite3_கோவை_Delete_NN(db, (கோவை*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
வெறுமை sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  எனில்( p==0 ){
    /* Nothing to clear */
  }இல்லையெனில் எனில்( p->pOn ){
    sqlite3_கோவை_Delete_NN(db, p->pOn);
  }இல்லையெனில் எனில்( p->pUsing ){
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
முழு sqlite3_கோவை_Deferred_Delete(Parse *pParse, கோவை *p_கோவை){
  திரும்பு 0==sqlite3ParserAddCleanup(pParse, sqlite3_கோவை_Delete_Generic, p_கோவை);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
வெறுமை sqlite3_கோவை_Unmap_And_Delete(Parse *pParse, கோவை *p){
  எனில்( p ){
    எனில்( IN_RENAME_OBJECT ){
      sqlite3_Rename_கோவை_Unmap(pParse, p);
    }
    sqlite3_கோவை_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
நிலையான முழு கோவை_Struct_Size(மாறிலி கோவை *p){
  எனில்( கோவை_Has_Property(p, EP_TokenOnly) ) திரும்பு கோவை_TOKENONLYSIZE;
  எனில்( கோவை_Has_Property(p, EP_Reduced) ) திரும்பு கோவை_REDUCEDSIZE;
  திரும்பு கோவை_FULLSIZE;
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
நிலையான முழு duped_கோவை_Struct_Size(மாறிலி கோவை *p, முழு flags){
  முழு nSize;
  ப்ரதிஜ்ஞா( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  ப்ரதிஜ்ஞா( கோவை_FULLSIZE<=0xfff );
  ப்ரதிஜ்ஞா( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  எனில்( 0==flags || கோவை_Has_Property(p, EP_FullSize) ){
    nSize = கோவை_FULLSIZE;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(p, EP_OuterON) );
    ப்ரதிஜ்ஞா( !கோவை_Has_VVAProperty(p, EP_NoReduce) );
    எனில்( p->p_இடது || p->x.pList ){
      nSize = கோவை_REDUCEDSIZE | EP_Reduced;
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( p->p_வலது==0 );
      nSize = கோவை_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  திரும்பு nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
நிலையான முழு duped_கோவை_முனை_Size(மாறிலி கோவை *p, முழு flags){
  முழு nByte = duped_கோவை_Struct_Size(p, flags) & 0xfff;
  எனில்( !கோவை_Has_Property(p, EP_Int_மதிப்பு) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  திரும்பு ROUND8(nByte);
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
நிலையான முழு duped_கோவை_Size(மாறிலி கோவை *p){
  முழு nByte;
  ப்ரதிஜ்ஞா( p!=0 );
  nByte = duped_கோவை_முனை_Size(p, EXPRDUP_REDUCE);
  எனில்( p->p_இடது ) nByte += duped_கோவை_Size(p->p_இடது);
  எனில்( p->p_வலது ) nByte += duped_கோவை_Size(p->p_வலது);
  ப்ரதிஜ்ஞா( nByte==ROUND8(nByte) );
  திரும்பு nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
வகைவரை கட்டமைப்பு EdupBuf EdupBuf;
கட்டமைப்பு EdupBuf {
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
நிலையான கோவை *கோவை_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  மாறிலி கோவை *p,        /* Expr tree to be duplicated */
  முழு dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  கோவை *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  முழு nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( p );
  ப்ரதிஜ்ஞா( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  ப்ரதிஜ்ஞா( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  எனில்( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    ப்ரதிஜ்ஞா( sEdupBuf.zAlloc!=0 );
    ப்ரதிஜ்ஞா( dupFlags==EXPRDUP_REDUCE );
  }இல்லையெனில்{
    முழு nAlloc;
    எனில்( dupFlags ){
      nAlloc = duped_கோவை_Size(p);
    }இல்லையெனில் எனில்( !கோவை_Has_Property(p, EP_Int_மதிப்பு) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(கோவை_FULLSIZE + nToken);
    }இல்லையெனில்{
      nToken = 0;
      nAlloc = ROUND8(கோவை_FULLSIZE);
    }
    ப்ரதிஜ்ஞா( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (கோவை *)sEdupBuf.zAlloc;
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pNew) );

  எனில்( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    மாறிலி unsigned nStructSize = duped_கோவை_Struct_Size(p, dupFlags);
    முழு nNewSize = nStructSize & 0xfff;
    எனில்( nToken<0 ){
      எனில்( !கோவை_Has_Property(p, EP_Int_மதிப்பு) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }இல்லையெனில்{
        nToken = 0;
      }
    }
    எனில்( dupFlags ){
      ப்ரதிஜ்ஞா( (முழு)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      ப்ரதிஜ்ஞா( கோவை_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }இல்லையெனில்{
      u32 nSize = (u32)கோவை_Struct_Size(p);
      ப்ரதிஜ்ஞா( (முழு)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (முழு)கோவை_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      எனில்( nSize<கோவை_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, கோவை_FULLSIZE-nSize);
      }
      nNewSize = கோவை_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    கோவை_Clear_VVAProperties(pNew);
    எனில்( dupFlags ){
      கோவை_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    ப்ரதிஜ்ஞா( nToken>=0 );
    எனில்( nToken>0 ){
      எழுத்து *zToken = pNew->u.zToken = (எழுத்து*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    எனில்( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      எனில்( கோவை_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }இல்லையெனில்{
        pNew->x.pList = sqlite3_கோவை_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      எனில்( கோவை_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        ப்ரதிஜ்ஞா( கோவை_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      எனில்( dupFlags ){
        எனில்( p->op==TK_SELECT_COLUMN ){
          pNew->p_இடது = p->p_இடது;
          ப்ரதிஜ்ஞா( p->p_வலது==0 
               || p->p_வலது==p->p_இடது
               || கோவை_Has_Property(p->p_இடது, EP_Subquery) );
        }இல்லையெனில்{
          pNew->p_இடது = p->p_இடது ?
                      கோவை_Dup(db, p->p_இடது, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_வலது = p->p_வலது ?
                       கோவை_Dup(db, p->p_வலது, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }இல்லையெனில்{
        எனில்( p->op==TK_SELECT_COLUMN ){
          pNew->p_இடது = p->p_இடது;
          ப்ரதிஜ்ஞா( p->p_வலது==0 
               || p->p_வலது==p->p_இடது
               || கோவை_Has_Property(p->p_இடது, EP_Subquery) );
        }இல்லையெனில்{
          pNew->p_இடது = sqlite3_கோவை_Dup(db, p->p_இடது, 0);
        }
        pNew->p_வலது = sqlite3_கோவை_Dup(db, p->p_வலது, 0);
      }
    }
  }
  எனில்( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, அளவு(sEdupBuf));
  ப்ரதிஜ்ஞா( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  திரும்பு pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  எனில்( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    எனில்( pRet ){
      முழு i;
      pRet->nCte = p->nCte;
      க்கு(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_கோவை_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  திரும்பு pRet;
}
#இல்லையெனில்
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
நிலையான முழு gatherSelectWindowsCallback(Walker *pWalker, கோவை *p_கோவை){
  எனில்( p_கோவை->op==TK_FUNCTION && கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_கோவை->y.pWin;
    ப்ரதிஜ்ஞா( pWin );
    ப்ரதிஜ்ஞா( IsWindowFunc(p_கோவை) );
    ப்ரதிஜ்ஞா( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  திரும்பு WRC_Continue;
}
நிலையான முழு gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  திரும்பு p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
நிலையான வெறுமை gatherSelectWindows(Select *p){
  Walker w;
  w.x_கோவை_Callback = gatherSelectWindowsCallback;
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
கோவை *sqlite3_கோவை_Dup(sqlite3 *db, மாறிலி கோவை *p, முழு flags){
  ப்ரதிஜ்ஞா( flags==0 || flags==EXPRDUP_REDUCE );
  திரும்பு p ? கோவை_Dup(db, p, flags, 0) : 0;
}
கோவை_List *sqlite3_கோவை_List_Dup(sqlite3 *db, மாறிலி கோவை_List *p, முழு flags){
  கோவை_List *pNew;
  கட்டமைப்பு கோவை_List_item *pItem;
  மாறிலி கட்டமைப்பு கோவை_List_item *pOldItem;
  முழு i;
  கோவை *pPriorSelectColOld = 0;
  கோவை *pPriorSelectColNew = 0;
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( p==0 ) திரும்பு 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  எனில்( pNew==0 ) திரும்பு 0;
  pNew->n_கோவை = p->n_கோவை;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  க்கு(i=0; i<p->n_கோவை; i++, pItem++, pOldItem++){
    கோவை *p_Old_கோவை = pOldItem->p_கோவை;
    கோவை *p_New_கோவை;
    pItem->p_கோவை = sqlite3_கோவை_Dup(db, p_Old_கோவை, flags);
    எனில்( p_Old_கோவை
     && p_Old_கோவை->op==TK_SELECT_COLUMN
     && (p_New_கோவை = pItem->p_கோவை)!=0
    ){
      எனில்( p_New_கோவை->p_வலது ){
        pPriorSelectColOld = p_Old_கோவை->p_வலது;
        pPriorSelectColNew = p_New_கோவை->p_வலது;
        p_New_கோவை->p_இடது = p_New_கோவை->p_வலது;
      }இல்லையெனில்{
        எனில்( p_Old_கோவை->p_இடது!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_கோவை->p_இடது;
          pPriorSelectColNew = sqlite3_கோவை_Dup(db, pPriorSelectColOld, flags);
          p_New_கோவை->p_வலது = pPriorSelectColNew;
        }
        p_New_கோவை->p_இடது = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  திரும்பு pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#எனில் !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, மாறிலி SrcList *p, முழு flags){
  SrcList *pNew;
  முழு i;
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( p==0 ) திரும்பு 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  எனில்( pNew==0 ) திரும்பு 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  க்கு(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    மாறிலி SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    எனில்( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, அளவு(Subquery));
      எனில்( pNewSubq==0 ){
        ப்ரதிஜ்ஞா( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }இல்லையெனில்{
        memcpy(pNewSubq, pOldItem->u4.pSubq, அளவு(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        எனில்( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }இல்லையெனில் எனில்( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }இல்லையெனில்{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    எனில்( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }இல்லையெனில் எனில்( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_கோவை_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }இல்லையெனில்{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    எனில்( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    எனில்( pTab ){
      pTab->nTabRef++;
    }
    எனில்( pOldItem->fg.isUsing ){
      ப்ரதிஜ்ஞா( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }இல்லையெனில்{
      pNewItem->u3.pOn = sqlite3_கோவை_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  திரும்பு pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, மாறிலி IdList *p){
  IdList *pNew;
  முழு i;
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( p==0 ) திரும்பு 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  எனில்( pNew==0 ) திரும்பு 0;
  pNew->nId = p->nId;
  க்கு(i=0; i<p->nId; i++){
    கட்டமைப்பு IdList_item *pNewItem = &pNew->a[i];
    மாறிலி கட்டமைப்பு IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  திரும்பு pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, மாறிலி Select *pDup, முழு flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  மாறிலி Select *p;

  ப்ரதிஜ்ஞா( db!=0 );
  க்கு(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, அளவு(*p) );
    எனில்( pNew==0 ) நிறுத்து;
    pNew->pEList = sqlite3_கோவை_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_கோவை_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_கோவை_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_கோவை_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_கோவை_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_கோவை_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    எனில்( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    எனில்( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      நிறுத்து;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  திரும்பு pRet;
}
#இல்லையெனில்
Select *sqlite3SelectDup(sqlite3 *db, மாறிலி Select *p, முழு flags){
  ப்ரதிஜ்ஞா( p==0 );
  திரும்பு 0;
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
நிலையான மாறிலி கட்டமைப்பு கோவை_List_item zeroItem = {0};
SQLITE_NOINLINE கோவை_List *sqlite3_கோவை_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  கோவை *p_கோவை             /* Expression to be appended. Might be NULL */
){
  கட்டமைப்பு கோவை_List_item *pItem;
  கோவை_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  எனில்( pList==0 ){
    sqlite3_கோவை_Delete(db, p_கோவை);
    திரும்பு 0;
  }
  pList->nAlloc = 4;
  pList->n_கோவை = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_கோவை = p_கோவை;
  திரும்பு pList;
}
SQLITE_NOINLINE கோவை_List *sqlite3_கோவை_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  கோவை_List *pList,        /* List to which to append. Might be NULL */
  கோவை *p_கோவை             /* Expression to be appended. Might be NULL */
){
  கட்டமைப்பு கோவை_List_item *pItem;
  கோவை_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  எனில்( pNew==0 ){
    sqlite3_கோவை_List_Delete(db, pList);
    sqlite3_கோவை_Delete(db, p_கோவை);
    திரும்பு 0;
  }இல்லையெனில்{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_கோவை++];
  *pItem = zeroItem;
  pItem->p_கோவை = p_கோவை;
  திரும்பு pList;
}
கோவை_List *sqlite3_கோவை_List_Append(
  Parse *pParse,          /* Parsing context */
  கோவை_List *pList,        /* List to which to append. Might be NULL */
  கோவை *p_கோவை             /* Expression to be appended. Might be NULL */
){
  கட்டமைப்பு கோவை_List_item *pItem;
  எனில்( pList==0 ){
    திரும்பு sqlite3_கோவை_List_Append_New(pParse->db,p_கோவை);
  }
  எனில்( pList->nAlloc<pList->n_கோவை+1 ){
    திரும்பு sqlite3_கோவை_List_Append_Grow(pParse->db,pList,p_கோவை);
  }
  pItem = &pList->a[pList->n_கோவை++];
  *pItem = zeroItem;
  pItem->p_கோவை = p_கோவை;
  திரும்பு pList;
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
கோவை_List *sqlite3_கோவை_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  கோவை_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  கோவை *p_கோவை            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  முழு n;
  முழு i;
  முழு iFirst = pList ? pList->n_கோவை : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  எனில்( NEVER(pColumns==0) ) செல் vector_append_error;
  எனில்( p_கோவை==0 ) செல் vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  எனில்( p_கோவை->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_கோவை_Vector_Size(p_கோவை)) ){
    sqlite3_Error_செய்தி(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    செல் vector_append_error;
  }

  க்கு(i=0; i<pColumns->nId; i++){
    கோவை *p_Sub_கோவை = sqlite3_கோவை_For_Vector_Field(pParse, p_கோவை, i, pColumns->nId);
    ப்ரதிஜ்ஞா( p_Sub_கோவை!=0 || db->mallocFailed );
    எனில்( p_Sub_கோவை==0 ) தொடர்;
    pList = sqlite3_கோவை_List_Append(pParse, pList, p_Sub_கோவை);
    எனில்( pList ){
      ப்ரதிஜ்ஞா( pList->n_கோவை==iFirst+i+1 );
      pList->a[pList->n_கோவை-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  எனில்( !db->mallocFailed && p_கோவை->op==TK_SELECT && ALWAYS(pList!=0) ){
    கோவை *pFirst = pList->a[iFirst].p_கோவை;
    ப்ரதிஜ்ஞா( pFirst!=0 );
    ப்ரதிஜ்ஞா( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_வலது = p_கோவை;
    p_கோவை = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_கோவை_Unmap_And_Delete(pParse, p_கோவை);
  sqlite3IdListDelete(db, pColumns);
  திரும்பு pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
வெறுமை sqlite3_கோவை_List_Set_Sort_Order(கோவை_List *p, முழு iSortOrder, முழு eNulls){
  கட்டமைப்பு கோவை_List_item *pItem;
  எனில்( p==0 ) திரும்பு;
  ப்ரதிஜ்ஞா( p->n_கோவை>0 );

  ப்ரதிஜ்ஞா( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  ப்ரதிஜ்ஞா( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  ப்ரதிஜ்ஞா( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_கோவை-1];
  ப்ரதிஜ்ஞா( pItem->fg.bNulls==0 );
  எனில்( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  எனில்( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    எனில்( iSortOrder!=eNulls ){
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
வெறுமை sqlite3_கோவை_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  கோவை_List *pList,        /* List to which to add the span. */
  மாறிலி Token *pName,     /* Name to be added */
  முழு dequote             /* True to cause the name to be dequoted */
){
  ப்ரதிஜ்ஞா( pList!=0 || pParse->db->mallocFailed!=0 );
  ப்ரதிஜ்ஞா( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  எனில்( pList ){
    கட்டமைப்பு கோவை_List_item *pItem;
    ப்ரதிஜ்ஞா( pList->n_கோவை>0 );
    pItem = &pList->a[pList->n_கோவை-1];
    ப்ரதிஜ்ஞா( pItem->zEName==0 );
    ப்ரதிஜ்ஞா( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    எனில்( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      எனில்( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (மாறிலி வெறுமை*)pItem->zEName, pName);
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
வெறுமை sqlite3_கோவை_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  கோவை_List *pList,        /* List to which to add the span. */
  மாறிலி எழுத்து *zStart,     /* Start of the span */
  மாறிலி எழுத்து *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  ப்ரதிஜ்ஞா( pList!=0 || db->mallocFailed!=0 );
  எனில்( pList ){
    கட்டமைப்பு கோவை_List_item *pItem = &pList->a[pList->n_கோவை-1];
    ப்ரதிஜ்ஞா( pList->n_கோவை>0 );
    எனில்( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
வெறுமை sqlite3_கோவை_List_Check_Length(
  Parse *pParse,
  கோவை_List *pEList,
  மாறிலி எழுத்து *zObject
){
  முழு mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_கோவை==mx );
  testcase( pEList && pEList->n_கோவை==mx+1 );
  எனில்( pEList && pEList->n_கோவை>mx ){
    sqlite3_Error_செய்தி(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
நிலையான SQLITE_NOINLINE வெறுமை கோவை_List_Delete_NN(sqlite3 *db, கோவை_List *pList){
  முழு i = pList->n_கோவை;
  கட்டமைப்பு கோவை_List_item *pItem =  pList->a;
  ப்ரதிஜ்ஞா( pList->n_கோவை>0 );
  ப்ரதிஜ்ஞா( db!=0 );
  செய்{
    sqlite3_கோவை_Delete(db, pItem->p_கோவை);
    எனில்( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }போது( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
வெறுமை sqlite3_கோவை_List_Delete(sqlite3 *db, கோவை_List *pList){
  எனில்( pList ) கோவை_List_Delete_NN(db, pList);
}
வெறுமை sqlite3_கோவை_List_Delete_Generic(sqlite3 *db, வெறுமை *pList){
  எனில்( ALWAYS(pList) ) கோவை_List_Delete_NN(db, (கோவை_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_கோவை_List_Flags(மாறிலி கோவை_List *pList){
  முழு i;
  u32 m = 0;
  ப்ரதிஜ்ஞா( pList!=0 );
  க்கு(i=0; i<pList->n_கோவை; i++){
     கோவை *p_கோவை = pList->a[i].p_கோவை;
     ப்ரதிஜ்ஞா( p_கோவை!=0 );
     m |= p_கோவை->flags;
  }
  திரும்பு m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
முழு sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  திரும்பு WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(மாறிலி எழுத்து *zIn){
  எனில்( sqlite3StrICmp(zIn, "true")==0  ) திரும்பு EP_IsTrue;
  எனில்( sqlite3StrICmp(zIn, "false")==0 ) திரும்பு EP_IsFalse;
  திரும்பு 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
முழு sqlite3_கோவை_Id_To_True_False(கோவை *p_கோவை){
  u32 v;
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_ID || p_கோவை->op==TK_STRING );
  எனில்( !கோவை_Has_Property(p_கோவை, EP_Quoted|EP_Int_மதிப்பு)
   && (v = sqlite3IsTrueOrFalse(p_கோவை->u.zToken))!=0
  ){
    p_கோவை->op = TK_TRUEFALSE;
    கோவை_Set_Property(p_கோவை, v);
    திரும்பு 1;
  }
  திரும்பு 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
முழு sqlite3_கோவை_Truth_மதிப்பு(மாறிலி கோவை *p_கோவை){
  p_கோவை = sqlite3_கோவை_Skip_Collate_And_Likely((கோவை*)p_கோவை);
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_TRUEFALSE );
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
  ப்ரதிஜ்ஞா( sqlite3StrICmp(p_கோவை->u.zToken,"true")==0
       || sqlite3StrICmp(p_கோவை->u.zToken,"false")==0 );
  திரும்பு p_கோவை->u.zToken[4]==0;
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
கோவை *sqlite3_கோவை_Simplified_And_Or(கோவை *p_கோவை){
  ப்ரதிஜ்ஞா( p_கோவை!=0 );
  எனில்( p_கோவை->op==TK_AND || p_கோவை->op==TK_OR ){
    கோவை *p_வலது = sqlite3_கோவை_Simplified_And_Or(p_கோவை->p_வலது);
    கோவை *p_இடது = sqlite3_கோவை_Simplified_And_Or(p_கோவை->p_இடது);
    எனில்( கோவை_Always_True(p_இடது) || கோவை_Always_False(p_வலது) ){
      p_கோவை = p_கோவை->op==TK_AND ? p_வலது : p_இடது;
    }இல்லையெனில் எனில்( கோவை_Always_True(p_வலது) || கோவை_Always_False(p_இடது) ){
      p_கோவை = p_கோவை->op==TK_AND ? p_இடது : p_வலது;
    }
  }
  திரும்பு p_கோவை;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
நிலையான முழு கோவை_Eval_Rhs_First(கோவை *p_கோவை){
  எனில்( கோவை_Has_Property(p_கோவை->p_இடது, EP_Subquery)
   && !கோவை_Has_Property(p_கோவை->p_வலது, EP_Subquery)
  ){
    திரும்பு 1;
  }இல்லையெனில்{
    திரும்பு 0;
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
நிலையான முழு கோவை_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  கோவை *p_கோவை,       /* The comparison expression */
  முழு *pR1,          /* OUT: Register holding the left operand */
  முழு *pR2,          /* OUT: Register holding the right operand */
  முழு *pFree1,       /* OUT: Temp register to free if not zero */
  முழு *pFree2        /* OUT: Another temp register to free if not zero */
){
  முழு addrIsNull;
  முழு r1, r2;
  Vdbe *v = pParse->pVdbe;

  ப்ரதிஜ்ஞா( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  எனில்( கோவை_Eval_Rhs_First(p_கோவை) && sqlite3_கோவை_Can_Be_Null(p_கோவை->p_வலது) ){
    r2 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_வலது, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }இல்லையெனில்{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, pFree1);
  எனில்( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    எனில்( கோவை_Has_Property(p_கோவை->p_வலது, EP_Subquery)
     && sqlite3_கோவை_Can_Be_Null(p_கோவை->p_இடது)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_வலது, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  திரும்பு addrIsNull;
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
நிலையான SQLITE_NOINLINE முழு கோவை_முனை_Is_Constant_Function(
  Walker *pWalker,
  கோவை *p_கோவை
){
  முழு n;             /* Number of arguments */
  கோவை_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  ப்ரதிஜ்ஞா( p_கோவை->op==TK_FUNCTION );
  எனில்( கோவை_Has_Property(p_கோவை, EP_TokenOnly)
   || (pList = p_கோவை->x.pList)==0
  ){;
    n = 0;
  }இல்லையெனில்{
    n = pList->n_கோவை;
    sqlite3_Walk_கோவை_List(pWalker, pList);
    எனில்( pWalker->eCode==0 ) திரும்பு WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_கோவை->u.zToken, n, ENC(db), 0);
  எனில்( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(கோவை_Has_Property(p_கோவை, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    திரும்பு WRC_Abort;
  }
  திரும்பு WRC_Prune;
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
நிலையான முழு கோவை_முனை_Is_Constant(Walker *pWalker, கோவை *p_கோவை){
  ப்ரதிஜ்ஞா( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  எனில்( pWalker->eCode==2 && கோவை_Has_Property(p_கோவை, EP_OuterON) ){
    pWalker->eCode = 0;
    திரும்பு WRC_Abort;
  }

  தேர்வு( p_கோவை->op ){
    நிலை TK_FUNCTION:
      எனில்( கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        திரும்பு WRC_Abort;
      }இல்லையெனில் எனில்( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        எனில்( pWalker->eCode==5 ) கோவை_Set_Property(p_கோவை, EP_FromDDL);
        திரும்பு WRC_Continue;
      }இல்லையெனில் எனில்( கோவை_Has_Property(p_கோவை,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        திரும்பு WRC_Continue;
      }இல்லையெனில் எனில்( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        திரும்பு கோவை_முனை_Is_Constant_Function(pWalker, p_கோவை);
      }இல்லையெனில்{
        pWalker->eCode = 0;  /* Function call is not constant */
        திரும்பு WRC_Abort;
      }
    நிலை TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      எனில்( sqlite3_கோவை_Id_To_True_False(p_கோவை) ){
        திரும்பு WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    நிலை TK_COLUMN:
    நிலை TK_AGG_FUNCTION:
    நிலை TK_AGG_COLUMN:
      testcase( p_கோவை->op==TK_ID );
      testcase( p_கோவை->op==TK_COLUMN );
      testcase( p_கோவை->op==TK_AGG_FUNCTION );
      testcase( p_கோவை->op==TK_AGG_COLUMN );
      எனில்( கோவை_Has_Property(p_கோவை, EP_FixedCol) && pWalker->eCode!=2 ){
        திரும்பு WRC_Continue;
      }
      எனில்( pWalker->eCode==3 && p_கோவை->iTable==pWalker->u.iCur ){
        திரும்பு WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    நிலை TK_IF_NULL_ROW:
    நிலை TK_REGISTER:
    நிலை TK_DOT:
    நிலை TK_RAISE:
      testcase( p_கோவை->op==TK_REGISTER );
      testcase( p_கோவை->op==TK_IF_NULL_ROW );
      testcase( p_கோவை->op==TK_DOT );
      testcase( p_கோவை->op==TK_RAISE );
      pWalker->eCode = 0;
      திரும்பு WRC_Abort;
    நிலை TK_IS:
    நிலை TK_ISNOT:
      எனில்( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        திரும்பு WRC_Abort;
      }
      திரும்பு WRC_Continue;
    நிலை TK_VARIABLE:
      எனில்( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_கோவை->op = TK_NULL;
      }இல்லையெனில் எனில்( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        திரும்பு WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    இயல்பு:
      testcase( p_கோவை->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_கோவை->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      திரும்பு WRC_Continue;
  }
}
நிலையான முழு கோவை_Is_Const(Parse *pParse, கோவை *p, முழு initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_கோவை_Callback = கோவை_முனை_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_கோவை(&w, p);
  திரும்பு w.eCode;
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
முழு sqlite3_கோவை_Is_Constant(Parse *pParse, கோவை *p){
  திரும்பு கோவை_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
முழு sqlite3_கோவை_List_Is_Constant(Parse *pParse, கோவை_List *pList, முழு bNoIs){
  மாறிலி முழு iInit = bNoIs ? 6 : 1;
  முழு ii;
  க்கு(ii=0; ii<pList->n_கோவை; ii++){
    எனில்( 0==கோவை_Is_Const(pParse, pList->a[ii].p_கோவை, iInit) ) திரும்பு 0;
  }
  திரும்பு 1;
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
நிலையான முழு sqlite3_கோவை_Is_Constant_Not_Join(Parse *pParse, கோவை *p){
  திரும்பு கோவை_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
நிலையான முழு கோவை_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  ப்ரதிஜ்ஞா( pSelect!=0 );
  ப்ரதிஜ்ஞா( pWalker->eCode==3 || pWalker->eCode==0 );
  எனில்( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    திரும்பு WRC_Abort;
  }
  திரும்பு WRC_Prune;
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
முழு sqlite3_கோவை_Is_Table_Constant(கோவை *p, முழு iCur, முழு bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_கோவை_Callback = கோவை_முனை_Is_Constant;
  எனில்( bAllowSubq ){
    w.xSelectCallback = கோவை_Select_Walk_Table_Constant;
  }இல்லையெனில்{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_கோவை(&w, p);
  திரும்பு w.eCode;
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
முழு sqlite3_கோவை_Is_Single_Table_Constraint(
  கோவை *p_கோவை,                 /* The constraint */
  மாறிலி SrcList *pSrcList,     /* Complete FROM clause */
  முழு iSrc,                    /* Which element of pSrcList to use */
  முழு bAllowSubq               /* Allow non-correlated subqueries */
){
  மாறிலி SrcItem *pSrc = &pSrcList->a[iSrc];
  எனில்( pSrc->fg.jointype & JT_LTORJ ){
    திரும்பு 0;  /* rule (3) */
  }
  எனில்( pSrc->fg.jointype & JT_இடது ){
    எனில்( !கோவை_Has_Property(p_கோவை, EP_OuterON) ) திரும்பு 0;   /* rule (4a) */
    எனில்( p_கோவை->w.iJoin!=pSrc->iCursor ) திரும்பு 0;         /* rule (4b) */
  }இல்லையெனில்{
    எனில்( கோவை_Has_Property(p_கோவை, EP_OuterON) ) திரும்பு 0;    /* rule (5) */
  }
  எனில்( கோவை_Has_Property(p_கோவை, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    முழு jj;
    க்கு(jj=0; jj<iSrc; jj++){
      எனில்( p_கோவை->w.iJoin==pSrcList->a[jj].iCursor ){
        எனில்( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          திரும்பு 0;  /* restriction (6) */
        }
        நிறுத்து;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  திரும்பு sqlite3_கோவை_Is_Table_Constant(p_கோவை, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
நிலையான முழு கோவை_முனை_Is_Constant_Or_Group_By(Walker *pWalker, கோவை *p_கோவை){
  கோவை_List *pGroupBy = pWalker->u.pGroupBy;
  முழு i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  க்கு(i=0; i<pGroupBy->n_கோவை; i++){
    கோவை *p = pGroupBy->a[i].p_கோவை;
    எனில்( sqlite3_கோவை_Compare(0, p_கோவை, p, -1)<2 ){
      CollSeq *pColl = sqlite3_கோவை_NNColl_Seq(pWalker->pParse, p);
      எனில்( sqlite3_Is_இருமம்(pColl) ){
        திரும்பு WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  எனில்( கோவை_Use_XSelect(p_கோவை) ){
    pWalker->eCode = 0;
    திரும்பு WRC_Abort;
  }

  திரும்பு கோவை_முனை_Is_Constant(pWalker, p_கோவை);
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
முழு sqlite3_கோவை_Is_Constant_Or_Group_By(Parse *pParse, கோவை *p, கோவை_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_கோவை_Callback = கோவை_முனை_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_கோவை(&w, p);
  திரும்பு w.eCode;
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
முழு sqlite3_கோவை_Is_Constant_Or_Function(கோவை *p, u8 isInit){
  ப்ரதிஜ்ஞா( isInit==0 || isInit==1 );
  திரும்பு கோவை_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
முழு sqlite3_கோவை_Contains_Subquery(கோவை *p){
  Walker w;
  w.eCode = 1;
  w.x_கோவை_Callback = sqlite3_கோவை_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_கோவை(&w, p);
  திரும்பு w.eCode==0;
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
முழு sqlite3_கோவை_Is_Integer(மாறிலி கோவை *p, முழு *p_மதிப்பு, Parse *pParse, முழு bRSI){
  முழு iSign = 1;   /* Either +1 or -1. */
  போது( 1/*exit-by-break*/ ){
    எனில்( NEVER(p==0) ) திரும்பு 0;
    எனில்( கோவை_Use_UValue(p) ){
      ப்ரதிஜ்ஞா( p->op==TK_INTEGER );
      *p_மதிப்பு = p->u.i_மதிப்பு*iSign;
      எனில்( bRSI && pParse==0 && p->op2 ){
        திரும்பு 0;
      }
      திரும்பு 1;
    }
    எனில்( p->op==TK_UPLUS ){
      p = p->p_இடது;
      pParse = 0;
      தொடர்;
    }
    எனில்( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_இடது;
      pParse = 0;
      தொடர்;
    }
    எனில்( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_மதிப்பு *pVal; /* The variable */
      முழு rc = 0;          /* 1 if successful, 0 if failed */
      ப்ரதிஜ்ஞா( iSign==1 );
      எனில்( NEVER(pParse->pVdbe==0) ) நிறுத்து;
      எனில்( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) நிறுத்து;
      pVal = sqlite3_Vdbe_Get_Bound_மதிப்பு(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      எனில்( pVal ){
        i64 vv;
        எனில்( sqlite3_மதிப்பு_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_மதிப்பு_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_மதிப்பு = (முழு)vv;
          rc = 1;       
        }
        sqlite3_மதிப்பு_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      திரும்பு rc;
    }
    நிறுத்து;
  }
  திரும்பு 0;
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
முழு sqlite3_கோவை_Can_Be_Null(மாறிலி கோவை *p){
  u8 op;
  ப்ரதிஜ்ஞா( p!=0 );
  போது( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_இடது;
    ப்ரதிஜ்ஞா( p!=0 );
  }
  op = p->op;
  எனில்( op==TK_REGISTER ) op = p->op2;
  தேர்வு( op ){
    நிலை TK_INTEGER:
    நிலை TK_STRING:
    நிலை TK_FLOAT:
    நிலை TK_BLOB:
      திரும்பு 0;
    நிலை TK_COLUMN:
      ப்ரதிஜ்ஞா( கோவை_Use_YTab(p) );
      திரும்பு கோவை_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    இயல்பு:
      திரும்பு 1;
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
முழு sqlite3_கோவை_Needs_No_Affinity_Change(மாறிலி கோவை *p, எழுத்து aff){
  u8 op;
  முழு unaryMinus = 0;
  எனில்( aff==SQLITE_AFF_BLOB ) திரும்பு 1;
  போது( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    எனில்( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_இடது;
  }
  op = p->op;
  எனில்( op==TK_REGISTER ) op = p->op2;
  தேர்வு( op ){
    நிலை TK_INTEGER: {
      திரும்பு aff>=SQLITE_AFF_NUMERIC;
    }
    நிலை TK_FLOAT: {
      திரும்பு aff>=SQLITE_AFF_NUMERIC;
    }
    நிலை TK_STRING: {
      திரும்பு !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    நிலை TK_BLOB: {
      திரும்பு !unaryMinus;
    }
    நிலை TK_COLUMN: {
      ப்ரதிஜ்ஞா( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      திரும்பு aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    இயல்பு: {
      திரும்பு 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
முழு sqlite3IsRowid(மாறிலி எழுத்து *z){
  எனில்( sqlite3StrICmp(z, "_ROWID_")==0 ) திரும்பு 1;
  எனில்( sqlite3StrICmp(z, "ROWID")==0 ) திரும்பு 1;
  எனில்( sqlite3StrICmp(z, "OID")==0 ) திரும்பு 1;
  திரும்பு 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
மாறிலி எழுத்து *sqlite3RowidAlias(Table *pTab){
  மாறிலி எழுத்து *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  முழு ii;
  ப்ரதிஜ்ஞா( VisibleRowid(pTab) );
  க்கு(ii=0; ii<ArraySize(azOpt); ii++){
    எனில்( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) திரும்பு azOpt[ii];
  }
  திரும்பு 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
நிலையான Select *isCandidateForInOpt(மாறிலி கோவை *pX){
  Select *p;
  SrcList *pSrc;
  கோவை_List *pEList;
  Table *pTab;
  முழு i;
  எனில்( !கோவை_Use_XSelect(pX) ) திரும்பு 0;                 /* Not a subquery */
  எனில்( கோவை_Has_Property(pX, EP_VarSelect)  ) திரும்பு 0;  /* Correlated subq */
  p = pX->x.pSelect;
  எனில்( p->pPrior ) திரும்பு 0;              /* Not a compound SELECT */
  எனில்( p->selFlags & SF_Aggregate ){
    திரும்பு 0; /* No GROUP BY keyword or aggregate functions */
  }
  ப்ரதிஜ்ஞா( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  எனில்( p->pLimit ) திரும்பு 0;              /* Has no LIMIT clause */
  எனில்( p->pWhere ) திரும்பு 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  ப்ரதிஜ்ஞா( pSrc!=0 );
  எனில்( pSrc->nSrc!=1 ) திரும்பு 0;          /* Single term in FROM clause */
  எனில்( pSrc->a[0].fg.isSubquery) திரும்பு 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  ப்ரதிஜ்ஞா( pTab!=0 );
  ப்ரதிஜ்ஞா( !IsView(pTab)  );              /* FROM clause is not a view */
  எனில்( IsVirtual(pTab) ) திரும்பு 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  ப்ரதிஜ்ஞா( pEList!=0 );
  /* All SELECT results must be columns. */
  க்கு(i=0; i<pEList->n_கோவை; i++){
    கோவை *pRes = pEList->a[i].p_கோவை;
    எனில்( pRes->op!=TK_COLUMN ) திரும்பு 0;
    ப்ரதிஜ்ஞா( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  திரும்பு p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
நிலையான வெறுமை sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  முழு iCur,          /* Cursor for the index */
  முழு regHasNull,    /* Register in which to store hasNull flag */
  முழு eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  முழு addr1;
  முழு op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  எனில்( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }இல்லையெனில்{
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
நிலையான முழு sqlite3InRhsIsConstant(Parse *pParse, கோவை *pIn){
  கோவை *pLHS;
  முழு res;
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_இடது;
  pIn->p_இடது = 0;
  res = sqlite3_கோவை_Is_Constant(pParse, pIn);
  pIn->p_இடது = pLHS;
  திரும்பு res;
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
முழு sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  கோவை *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  முழு *prRhsHasNull,         /* Register holding NULL status.  See notes */
  முழு *aiMap,                /* Mapping from Index fields to RHS fields */
  முழு *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  முழு eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  முழு iTab;                             /* Cursor of the RHS table */
  முழு mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  ப்ரதிஜ்ஞா( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  எனில்( prRhsHasNull && கோவை_Use_XSelect(pX) ){
    முழு i;
    கோவை_List *pEList = pX->x.pSelect->pEList;
    க்கு(i=0; i<pEList->n_கோவை; i++){
      எனில்( sqlite3_கோவை_Can_Be_Null(pEList->a[i].p_கோவை) ) நிறுத்து;
    }
    எனில்( i==pEList->n_கோவை ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  எனில்( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    முழு iDb;                               /* Database idx for pTab */
    கோவை_List *pEList = p->pEList;
    முழு n_கோவை = pEList->n_கோவை;

    ப்ரதிஜ்ஞா( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    ப்ரதிஜ்ஞா( p->pEList->a[0].p_கோவை!=0 ); /* Because of isCandidateForInOpt(p) */
    ப்ரதிஜ்ஞா( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    ப்ரதிஜ்ஞா( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    ப்ரதிஜ்ஞா(v);  /* sqlite3GetVdbe() has always been previously called */
    எனில்( n_கோவை==1 && pEList->a[0].p_கோவை->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      முழு iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }இல்லையெனில்{
      Index *pIdx;                         /* Iterator variable */
      முழு affinity_ok = 1;
      முழு i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      க்கு(i=0; i<n_கோவை && affinity_ok; i++){
        கோவை *pLhs = sqlite3VectorFieldSubexpr(pX->p_இடது, i);
        முழு iCol = pEList->a[i].p_கோவை->iColumn;
        எழுத்து idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        எழுத்து cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        தேர்வு( cmpaff ){
          நிலை SQLITE_AFF_BLOB:
            நிறுத்து;
          நிலை SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            ப்ரதிஜ்ஞா( idxaff==SQLITE_AFF_TEXT );
            நிறுத்து;
          இயல்பு:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      எனில்( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        க்கு(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          எனில்( pIdx->nColumn<n_கோவை ) தொடர்;
          எனில்( pIdx->pPartIdxWhere!=0 ) தொடர்;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          எனில்( pIdx->nColumn>=BMS-1 ) தொடர்;
          எனில்( mustBeUnique ){
            எனில்( pIdx->n_சாவி_Col>n_கோவை
             ||(pIdx->nColumn>n_கோவை && !IsUniqueIndex(pIdx))
            ){
              தொடர்;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          க்கு(i=0; i<n_கோவை; i++){
            கோவை *pLhs = sqlite3VectorFieldSubexpr(pX->p_இடது, i);
            கோவை *pRhs = pEList->a[i].p_கோவை;
            CollSeq *pReq = sqlite3_இருமம்_Compare_Coll_Seq(pParse, pLhs, pRhs);
            முழு j;
 
            க்கு(j=0; j<n_கோவை; j++){
              எனில்( pIdx->aiColumn[j]!=pRhs->iColumn ) தொடர்;
              ப்ரதிஜ்ஞா( pIdx->azColl[j] );
              எனில்( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                தொடர்;
              }
              நிறுத்து;
            }
            எனில்( j==n_கோவை ) நிறுத்து;
            mCol = MASKBIT(j);
            எனில்( mCol & colUsed ) நிறுத்து; /* Each column used only once */
            colUsed |= mCol;
            எனில்( aiMap ) aiMap[i] = j;
          }
 
          ப்ரதிஜ்ஞா( n_கோவை>0 && n_கோவை<BMS );
          ப்ரதிஜ்ஞா( i==n_கோவை || colUsed!=(MASKBIT(n_கோவை)-1) );
          எனில்( colUsed==(MASKBIT(n_கோவை)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            முழு iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_சாவி_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            ப்ரதிஜ்ஞா( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            எனில்( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_கோவை)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              எனில்( n_கோவை==1 ){
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
  எனில்( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && கோவை_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_கோவை<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  எனில்( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    முழு rMayHaveNull = 0;
    முழு bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    எனில்( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }இல்லையெனில் எனில்( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    ப்ரதிஜ்ஞா( pX->op==TK_IN );
    எனில்( !bloomOk
     && கோவை_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    எனில்( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  எனில்( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    முழு i, n;
    n = sqlite3_கோவை_Vector_Size(pX->p_இடது);
    க்கு(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  திரும்பு eType;
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
நிலையான எழுத்து *கோவை_INAffinity(Parse *pParse, மாறிலி கோவை *p_கோவை){
  கோவை *p_இடது = p_கோவை->p_இடது;
  முழு nVal = sqlite3_கோவை_Vector_Size(p_இடது);
  Select *pSelect = கோவை_Use_XSelect(p_கோவை) ? p_கோவை->x.pSelect : 0;
  எழுத்து *zRet;

  ப்ரதிஜ்ஞா( p_கோவை->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  எனில்( zRet ){
    முழு i;
    க்கு(i=0; i<nVal; i++){
      கோவை *pA = sqlite3VectorFieldSubexpr(p_இடது, i);
      எழுத்து a = sqlite3_கோவை_Affinity(pA);
      எனில்( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_கோவை, a);
      }இல்லையெனில்{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  திரும்பு zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
வெறுமை sqlite3SubselectError(Parse *pParse, முழு nActual, முழு nExpect){
  எனில்( pParse->nErr==0 ){
    மாறிலி எழுத்து *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_செய்தி(pParse, zFmt, nActual, nExpect);
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
வெறுமை sqlite3_Vector_Error_செய்தி(Parse *pParse, கோவை *p_கோவை){
#ifndef SQLITE_OMIT_SUBQUERY
  எனில்( கோவை_Use_XSelect(p_கோவை) ){
    sqlite3SubselectError(pParse, p_கோவை->x.pSelect->pEList->n_கோவை, 1);
  }இல்லையெனில்
#endif
  {
    sqlite3_Error_செய்தி(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
நிலையான முழு findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  கோவை *p_கோவை,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  எனில்( pNewSig==0 ) திரும்பு 0;
  எனில்( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) திரும்பு 0;
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_IN );
  ப்ரதிஜ்ஞா( !கோவை_Use_YSub(p_கோவை) );
  ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை) );
  ப்ரதிஜ்ஞா( p_கோவை->x.pSelect!=0 );
  ப்ரதிஜ்ஞா( (p_கோவை->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  ப்ரதிஜ்ஞா( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  க்கு(; pOp<pEnd; pOp++){
    எனில்( pOp->p4type!=P4_SUBRTNSIG ) தொடர்;
    ப்ரதிஜ்ஞா( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    ப்ரதிஜ்ஞா( pSig!=0 );
    எனில்( !pSig->bComplete ) தொடர்;
    எனில்( pNewSig->selId!=pSig->selId ) தொடர்;
    எனில்( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) தொடர்;
    p_கோவை->y.sub.iAddr = pSig->iAddr;
    p_கோவை->y.sub.regReturn = pSig->regReturn;
    p_கோவை->iTable = pSig->iTable;
    கோவை_Set_Property(p_கோவை, EP_Subrtn);
    திரும்பு 1;
  }
  திரும்பு 0;
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
வெறுமை sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  கோவை *p_கோவை,            /* The IN operator */
  முழு iTab,               /* Use this cursor number */
  முழு allowBloom          /* True to allow the use of a Bloom filter */
){
  முழு addrOnce = 0;           /* Address of the OP_Once instruction at top */
  முழு addr;                   /* Address of OP_OpenEphemeral instruction */
  கோவை *p_இடது;                /* the LHS of the IN operator */
  சாவி_Info *p_சாவி_Info = 0;      /* Key information */
  முழு nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  ப்ரதிஜ்ஞா( v!=0 );

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
  எனில்( !கோவை_Has_Property(p_கோவை, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    ப்ரதிஜ்ஞா( !கோவை_Use_XSelect(p_கோவை) || p_கோவை->x.pSelect!=0 );
    எனில்( கோவை_Use_XSelect(p_கோவை) && (p_கோவை->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, அளவு(pSig[0]));
      எனில்( pSig ){
        pSig->selId = p_கோவை->x.pSelect->selId;
        pSig->zAff = கோவை_INAffinity(pParse, p_கோவை);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    எனில்( கோவை_Has_Property(p_கோவை, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_கோவை, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      எனில்( கோவை_Use_XSelect(p_கோவை) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_கோவை->x.pSelect->selId));
      }
      ப்ரதிஜ்ஞா( கோவை_Use_YSub(p_கோவை) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_கோவை->y.sub.regReturn,
                        p_கோவை->y.sub.iAddr);
      ப்ரதிஜ்ஞா( iTab!=p_கோவை->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_கோவை->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      எனில்( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      திரும்பு;
    }

    /* Begin coding the subroutine */
    ப்ரதிஜ்ஞா( !கோவை_Use_YWin(p_கோவை) );
    கோவை_Set_Property(p_கோவை, EP_Subrtn);
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_TokenOnly|EP_Reduced) );
    p_கோவை->y.sub.regReturn = ++pParse->nMem;
    p_கோவை->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_கோவை->y.sub.regReturn) + 1;
    எனில்( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_கோவை->y.sub.iAddr;
      pSig->regReturn = p_கோவை->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (மாறிலி எழுத்து*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_இடது = p_கோவை->p_இடது;
  nVal = sqlite3_கோவை_Vector_Size(p_இடது);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_கோவை->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_கோவை->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  எனில்( கோவை_Use_XSelect(p_கோவை) ){
    VdbeComment((v, "Result of SELECT %u", p_கோவை->x.pSelect->selId));
  }இல்லையெனில்{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_சாவி_Info = sqlite3_சாவி_Info_Alloc(pParse->db, nVal, 1);
  ப்ரதிஜ்ஞா( p_சாவி_Info!=0 || pParse->nErr );
  எனில்( p_சாவி_Info==0 ) திரும்பு;

  எனில்( கோவை_Use_XSelect(p_கோவை) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_கோவை->x.pSelect;
    கோவை_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    எனில்( ALWAYS(pEList->n_கோவை==nVal) ){
      Select *pCopy;
      SelectDest dest;
      முழு i;
      முழு rc;
      முழு addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = கோவை_INAffinity(pParse, p_கோவை);
      pSelect->iLimit = 0;
      ப்ரதிஜ்ஞா( pEList!=0 );
      ப்ரதிஜ்ஞா( pEList->n_கோவை>0 );
      ப்ரதிஜ்ஞா( sqlite3_சாவி_Info_Is_Writeable(p_சாவி_Info) );
      க்கு(i=0; i<nVal; i++){
        கோவை *p = sqlite3VectorFieldSubexpr(p_இடது, i);
        CollSeq *pColl;
        p_சாவி_Info->aColl[i] = pColl = sqlite3_இருமம்_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_கோவை
        );
        எனில்( !sqlite3_Is_இருமம்(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      எனில்( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        முழு regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (வெறுமை *)p_சாவி_Info, P4_KEYINFO);
        p_சாவி_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      எனில்( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        எனில்( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      எனில்( rc ){
        sqlite3_சாவி_Info_Unref(p_சாவி_Info);
        திரும்பு;
      }
    }
  }இல்லையெனில் எனில்( ALWAYS(p_கோவை->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    எழுத்து affinity;            /* Affinity of the LHS of the IN */
    முழு i;
    கோவை_List *pList = p_கோவை->x.pList;
    கட்டமைப்பு கோவை_List_item *pItem;
    முழு r1, r2;
    affinity = sqlite3_கோவை_Affinity(p_இடது);
    எனில்( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }இல்லையெனில் எனில்( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    ப்ரதிஜ்ஞா( p_சாவி_Info!=0 );
    ப்ரதிஜ்ஞா( sqlite3_சாவி_Info_Is_Writeable(p_சாவி_Info) );
    p_சாவி_Info->aColl[0] = sqlite3_கோவை_Coll_Seq(pParse, p_கோவை->p_இடது);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    க்கு(i=pList->n_கோவை, pItem=pList->a; i>0; i--, pItem++){
      கோவை *pE2 = pItem->p_கோவை;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      எனில்( addrOnce && !sqlite3_கோவை_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        கோவை_Clear_Property(p_கோவை, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_கோவை_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_செருகு, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  எனில்( pSig ) pSig->bComplete = 1;
  எனில்( p_சாவி_Info ){
    sqlite3VdbeChangeP4(v, addr, (வெறுமை *)p_சாவி_Info, P4_KEYINFO);
  }
  எனில்( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    ப்ரதிஜ்ஞா( கோவை_Use_YSub(p_கோவை) );
    ப்ரதிஜ்ஞா( sqlite3VdbeGetOp(v,p_கோவை->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_கோவை->y.sub.regReturn,
                      p_கோவை->y.sub.iAddr, 1);
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
முழு sqlite3CodeSubselect(Parse *pParse, கோவை *p_கோவை){
  முழு addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  முழு rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  முழு nReg;                   /* Registers to allocate */
  கோவை *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  முழு addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  ப்ரதிஜ்ஞா( v!=0 );
  எனில்( pParse->nErr ) திரும்பு 0;
  testcase( p_கோவை->op==TK_EXISTS );
  testcase( p_கோவை->op==TK_SELECT );
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_EXISTS || p_கோவை->op==TK_SELECT );
  ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை) );
  pSel = p_கோவை->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  எனில்( கோவை_Has_Property(p_கோவை, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    ப்ரதிஜ்ஞா( கோவை_Use_YSub(p_கோவை) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_கோவை->y.sub.regReturn,
                      p_கோவை->y.sub.iAddr);
    திரும்பு p_கோவை->iTable;
  }

  /* Begin coding the subroutine */
  ப்ரதிஜ்ஞா( !கோவை_Use_YWin(p_கோவை) );
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Reduced|EP_TokenOnly) );
  கோவை_Set_Property(p_கோவை, EP_Subrtn);
  p_கோவை->y.sub.regReturn = ++pParse->nMem;
  p_கோவை->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_கோவை->y.sub.regReturn) + 1;

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
  எனில்( !கோவை_Has_Property(p_கோவை, EP_VarSelect) ){
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
  nReg = p_கோவை->op==TK_SELECT ? pSel->pEList->n_கோவை : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  எனில்( p_கோவை->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    எனில்( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_வலது ){
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
    }இல்லையெனில்{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }இல்லையெனில்{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  எனில்( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    கோவை *p_இடது = pSel->pLimit->p_இடது;
    எனில்( கோவை_Has_Property(p_இடது, EP_Int_மதிப்பு)==0
     || (p_இடது->u.i_மதிப்பு!=1 && p_இடது->u.i_மதிப்பு!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_கோவை_Int32(db, 0);
      எனில்( pLimit ){
        pLimit->aff_கோவை = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_கோவை_Dup(db, p_இடது, 0), pLimit);
      }
      sqlite3_கோவை_Deferred_Delete(pParse, p_இடது);
      pSel->pLimit->p_இடது = pLimit;
    }
  }இல்லையெனில்{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_கோவை_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  எனில்( sqlite3Select(pParse, pSel, &dest) ){
    p_கோவை->op2 = p_கோவை->op;
    p_கோவை->op = TK_ERROR;
    திரும்பு 0;
  }
  p_கோவை->iTable = rReg = dest.iSDParm;
  கோவை_Set_VVAProperty(p_கோவை, EP_NoReduce);
  எனில்( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  எனில்( dest.eDest==SRT_Mem 
   && sqlite3_கோவை_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_கோவை) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  ப்ரதிஜ்ஞா( கோவை_Use_YSub(p_கோவை) );
  ப்ரதிஜ்ஞா( sqlite3VdbeGetOp(v,p_கோவை->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_கோவை->y.sub.regReturn,
                    p_கோவை->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  திரும்பு rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
முழு sqlite3_கோவை_Check_IN(Parse *pParse, கோவை *pIn){
  முழு nVector = sqlite3_கோவை_Vector_Size(pIn->p_இடது);
  எனில்( கோவை_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    எனில்( nVector!=pIn->x.pSelect->pEList->n_கோவை ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_கோவை, nVector);
      திரும்பு 1;
    }
  }இல்லையெனில் எனில்( nVector!=1 ){
    sqlite3_Vector_Error_செய்தி(pParse, pIn->p_இடது);
    திரும்பு 1;
  }
  திரும்பு 0;
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
நிலையான வெறுமை sqlite3_கோவை_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  கோவை *p_கோவை,          /* The IN expression */
  முழு destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  முழு destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  முழு rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  முழு eType;            /* Type of the RHS */
  முழு rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  முழு *aiMap = 0;       /* Map from vector field to index column */
  எழுத்து *zAff = 0;       /* Affinity string for comparisons */
  முழு nVector;          /* Size of vectors for this IN operator */
  முழு iDummy;           /* Dummy parameter to exprCodeVector() */
  கோவை *p_இடது;          /* The LHS of the IN operator */
  முழு i;                /* loop counter */
  முழு destStep2;        /* Where to jump when NULLs seen in step 2 */
  முழு destStep6 = 0;    /* Start of code for Step 6 */
  முழு addrTruthOp;      /* Address of opcode that determines the IN is true */
  முழு destNotNull;      /* Jump here if a comparison is not true in step 6 */
  முழு addr_உச்சி;          /* Top of the step-6 loop */
  முழு iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  ப்ரதிஜ்ஞா( !கோவை_Has_VVAProperty(p_கோவை,EP_Immutable) );
  p_இடது = p_கோவை->p_இடது;
  எனில்( sqlite3_கோவை_Check_IN(pParse, p_கோவை) ) திரும்பு;
  zAff = கோவை_INAffinity(pParse, p_கோவை);
  nVector = sqlite3_கோவை_Vector_Size(p_கோவை->p_இடது);
  aiMap = (முழு*)sqlite3DbMallocZero(pParse->db, nVector*அளவு(முழு));
  எனில்( pParse->db->mallocFailed ) செல் sqlite3_கோவை_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  ப்ரதிஜ்ஞா( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_கோவை,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  ப்ரதிஜ்ஞா( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  க்கு(i=0; i<nVector; i++){
    முழு j, cnt;
    க்கு(cnt=j=0; j<nVector; j++) எனில்( aiMap[j]==i ) cnt++;
    ப்ரதிஜ்ஞா( cnt==1 );
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
  ப்ரதிஜ்ஞா( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = கோவை_Code_Vector(pParse, p_இடது, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  எனில்( eType==IN_INDEX_NOOP ){
    கோவை_List *pList;
    CollSeq *pColl;
    முழு labelOk = sqlite3VdbeMakeLabel(pParse);
    முழு r2, regToFree;
    முழு regCkNull = 0;
    முழு ii;
    ப்ரதிஜ்ஞா( nVector==1 );
    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
    pList = p_கோவை->x.pList;
    pColl = sqlite3_கோவை_Coll_Seq(pParse, p_கோவை->p_இடது);
    எனில்( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    க்கு(ii=0; ii<pList->n_கோவை; ii++){
      r2 = sqlite3_கோவை_Code_Temp(pParse, pList->a[ii].p_கோவை, &regToFree);
      எனில்( regCkNull && sqlite3_கோவை_Can_Be_Null(pList->a[ii].p_கோவை) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      எனில்( ii<pList->n_கோவை-1 || destIfNull!=destIfFalse ){
        முழு op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (வெறுமை*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_கோவை-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_கோவை-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_கோவை-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_கோவை-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }இல்லையெனில்{
        முழு op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        ப்ரதிஜ்ஞா( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (வெறுமை*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    எனில்( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    செல் sqlite3_கோவை_Code_IN_finished;
  }

  எனில்( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    எனில்( nVector==1 ){
      எழுத்து aff = zAff[0];
      எனில்( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_கோவை_Affinity(p_இடது) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        முழு rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }இல்லையெனில்{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    க்கு(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    எனில்( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      முழு rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      க்கு(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  எனில்( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }இல்லையெனில்{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  க்கு(i=0; i<nVector; i++){
    கோவை *p = sqlite3VectorFieldSubexpr(p_கோவை->p_இடது, i);
    எனில்( pParse->nErr ) செல் sqlite3_கோவை_Code_IN_oom_error;
    எனில்( sqlite3_கோவை_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  எனில்( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    ப்ரதிஜ்ஞா( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }இல்லையெனில்{
    எனில்( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      எனில்( கோவை_Has_Property(p_கோவை, EP_Subrtn) ){
        மாறிலி VdbeOp *pOp = sqlite3VdbeGetOp(v, p_கோவை->y.sub.iAddr);
        ப்ரதிஜ்ஞா( pOp->opcode==OP_Once || pParse->nErr );
        எனில்( pOp->p3>0 ){  /* tag-202407032019 */
          ப்ரதிஜ்ஞா( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      செல் sqlite3_கோவை_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  எனில்( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  எனில்( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  எனில்( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  எனில்( eType==IN_INDEX_INDEX_DESC ){
    addr_உச்சி = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }இல்லையெனில்{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_உச்சி = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  எனில்( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }இல்லையெனில்{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  க்கு(i=0; i<nVector; i++){
    கோவை *p;
    CollSeq *pColl;
    முழு r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_இடது, i);
    எனில்( கோவை_Use_XSelect(p_கோவை) ){
      கோவை *pRhs = p_கோவை->x.pSelect->pEList->a[i].p_கோவை;
      pColl = sqlite3_இருமம்_Compare_Coll_Seq(pParse, p, pRhs);
    }இல்லையெனில்{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_கோவை_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (வெறுமை*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  எனில்( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_உச்சி+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_கோவை_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_கோவை_Code_IN_oom_error:
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
நிலையான வெறுமை codeReal(Vdbe *v, மாறிலி எழுத்து *z, முழு negateFlag, முழு iMem){
  எனில்( ALWAYS(z!=0) ){
    இரட்டை மதிப்பு;
    sqlite3AtoF(z, &மதிப்பு);
    ப்ரதிஜ்ஞா( !sqlite3IsNaN(மதிப்பு) ); /* The new AtoF never returns NaN */
    எனில்( negateFlag ) மதிப்பு = -மதிப்பு;
    sqlite3VdbeAddDouble(v, iMem, மதிப்பு);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
நிலையான வெறுமை codeInteger(Parse *pParse, கோவை *p_கோவை, முழு negFlag, முழு iMem){
  Vdbe *v = pParse->pVdbe;
  எனில்( p_கோவை->flags & EP_Int_மதிப்பு ){
    முழு i = p_கோவை->u.i_மதிப்பு;
    ப்ரதிஜ்ஞா( i>=0 );
    எனில்( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }இல்லையெனில்{
    முழு c;
    i64 மதிப்பு;
    மாறிலி எழுத்து *z = p_கோவை->u.zToken;
    ப்ரதிஜ்ஞா( z!=0 );
    c = sqlite3DecOrHexToI64(z, &மதிப்பு);
    எனில்( (c==3 && !negFlag) || (c==2) || (negFlag && மதிப்பு==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_செய்தி(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_கோவை);
#இல்லையெனில்
#ifndef SQLITE_OMIT_HEX_INTEGER
      எனில்( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_செய்தி(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_கோவை);
      }இல்லையெனில்
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }இல்லையெனில்{
      எனில்( negFlag ){ மதிப்பு = c==3 ? SMALLEST_INT64 : -மதிப்பு; }
      sqlite3VdbeAddInt64(v, iMem, மதிப்பு);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
வெறுமை sqlite3_கோவை_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  முழு iTabCur,    /* Cursor pointing to a table row */
  முழு iIdxCol,    /* The column of the index to be loaded */
  முழு regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  எனில்( iTabCol==XN_கோவை ){
    ப்ரதிஜ்ஞா( pIdx->a_Col_கோவை );
    ப்ரதிஜ்ஞா( pIdx->a_Col_கோவை->n_கோவை>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_கோவை_Code_Copy(pParse, pIdx->a_Col_கோவை->a[iIdxCol].p_கோவை, regOut);
    pParse->iSelfTab = 0;
  }இல்லையெனில்{
    sqlite3_கோவை_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
வெறுமை sqlite3_கோவை_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  முழு regOut         /* Put the result in this register */
){
  முழு iAddr;
  Vdbe *v = pParse->pVdbe;
  முழு nErr = pParse->nErr;
  ப்ரதிஜ்ஞா( v!=0 );
  ப்ரதிஜ்ஞா( pParse->iSelfTab!=0 );
  எனில்( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }இல்லையெனில்{
    iAddr = 0;
  }
  sqlite3_கோவை_Code_Copy(pParse, sqlite3_Column_கோவை(pTab,pCol), regOut);
  எனில்( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    முழு p3 = 2+(முழு)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (எழுத்து*)pTab, P4_TABLE);
  }இல்லையெனில் எனில்( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  எனில்( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  எனில்( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
வெறுமை sqlite3_கோவை_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  முழு iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  முழு iCol,       /* Index of the column to extract */
  முழு regOut      /* Extract the value into this register */
){
  Column *pCol;
  ப்ரதிஜ்ஞா( v!=0 );
  ப்ரதிஜ்ஞா( pTab!=0 );
  ப்ரதிஜ்ஞா( iCol!=XN_கோவை );
  எனில்( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }இல்லையெனில்{
    முழு x;
    எனில்( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      திரும்பு;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }இல்லையெனில் எனில்( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      எனில்( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_செய்தி(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }இல்லையெனில்{
        முழு savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_கோவை_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      திரும்பு;
#endif
    }இல்லையெனில் எனில்( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_சாவி_Index(pTab), iCol);
    }இல்லையெனில்{
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
முழு sqlite3_கோவை_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  முழு iColumn,     /* Index of the table column */
  முழு iTable,      /* The cursor pointing to the table */
  முழு iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  ப்ரதிஜ்ஞா( pParse->pVdbe!=0 );
  ப்ரதிஜ்ஞா( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  ப்ரதிஜ்ஞா( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_கோவை_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  எனில்( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    எனில்( pOp->opcode==OP_Column ) pOp->p5 = p5;
    எனில்( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  திரும்பு iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
வெறுமை sqlite3_கோவை_Code_Move(Parse *pParse, முழு iFrom, முழு iTo, முழு nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
வெறுமை sqlite3_கோவை_To_Register(கோவை *p_கோவை, முழு iReg){
  கோவை *p = sqlite3_கோவை_Skip_Collate_And_Likely(p_கோவை);
  எனில்( NEVER(p==0) ) திரும்பு;
  எனில்( p->op==TK_REGISTER ){
    ப்ரதிஜ்ஞா( p->iTable==iReg );
  }இல்லையெனில்{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    கோவை_Clear_Property(p, EP_Skip);
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
நிலையான முழு கோவை_Code_Vector(Parse *pParse, கோவை *p, முழு *piFreeable){
  முழு iResult;
  முழு nResult = sqlite3_கோவை_Vector_Size(p);
  எனில்( nResult==1 ){
    iResult = sqlite3_கோவை_Code_Temp(pParse, p, piFreeable);
  }இல்லையெனில்{
    *piFreeable = 0;
    எனில்( p->op==TK_SELECT ){
#எனில் SQLITE_OMIT_SUBQUERY
      iResult = 0;
#இல்லையெனில்
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }இல்லையெனில்{
      முழு i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p) );
      க்கு(i=0; i<nResult; i++){
        sqlite3_கோவை_Code_Factorable(pParse, p->x.pList->a[i].p_கோவை, i+iResult);
      }
    }
  }
  திரும்பு iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
நிலையான வெறுமை setDoNotMergeFlagOnCopy(Vdbe *v){
  எனில்( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
நிலையான முழு கோவை_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  கோவை_List *pFarg,      /* List of function arguments */
  முழு iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  முழு target            /* Store function result in this register */
){
  முழு nFarg;
  Vdbe *v = pParse->pVdbe;
  ப்ரதிஜ்ஞா( v!=0 );
  ப்ரதிஜ்ஞா( pFarg!=0 );
  nFarg = pFarg->n_கோவை;
  ப்ரதிஜ்ஞா( nFarg>0 );  /* All in-line functions have at least one argument */
  தேர்வு( iFuncId ){
    நிலை INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      முழு endCoalesce = sqlite3VdbeMakeLabel(pParse);
      முழு i;
      ப்ரதிஜ்ஞா( nFarg>=2 );
      sqlite3_கோவை_Code(pParse, pFarg->a[0].p_கோவை, target);
      க்கு(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_கோவை_Code(pParse, pFarg->a[i].p_கோவை, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      நிறுத்து;
    }
    நிலை INLINEFUNC_iif: {
      கோவை case_கோவை;
      memset(&case_கோவை, 0, அளவு(case_கோவை));
      case_கோவை.op = TK_CASE;
      case_கோவை.x.pList = pFarg;
      திரும்பு sqlite3_கோவை_Code_Target(pParse, &case_கோவை, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    நிலை INLINEFUNC_sqlite_offset: {
      கோவை *pArg = pFarg->a[0].p_கோவை;
      எனில்( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }இல்லையெனில்{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      நிறுத்து;
    }
#endif
    இயல்பு: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      ப்ரதிஜ்ஞா( nFarg==1 || nFarg==2 );
      target = sqlite3_கோவை_Code_Target(pParse, pFarg->a[0].p_கோவை, target);
      நிறுத்து;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#எனில் !defined(SQLITE_UNTESTABLE)
    நிலை INLINEFUNC_கோவை_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      ப்ரதிஜ்ஞா( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_கோவை_Compare(0,pFarg->a[0].p_கோவை, pFarg->a[1].p_கோவை,-1),
         target);
      நிறுத்து;
    }

    நிலை INLINEFUNC_கோவை_implies_கோவை: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      ப்ரதிஜ்ஞா( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_கோவை_Implies_கோவை(pParse,pFarg->a[0].p_கோவை, pFarg->a[1].p_கோவை,-1),
         target);
      நிறுத்து;
    }

    நிலை INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      கோவை *pA1;
      ப்ரதிஜ்ஞா( nFarg==2 );
      pA1 = pFarg->a[1].p_கோவை;
      எனில்( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_கோவை_Implies_Non_Null_Row(pFarg->a[0].p_கோவை,pA1->iTable,1),
           target);
      }இல்லையெனில்{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      நிறுத்து;
    }

    நிலை INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      மாறிலி எழுத்து *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      எழுத்து aff;
      ப்ரதிஜ்ஞா( nFarg==1 );
      aff = sqlite3_கோவை_Affinity(pFarg->a[0].p_கோவை);
      ப்ரதிஜ்ஞா( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      நிறுத்து;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  திரும்பு target;
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
நிலையான முழு கோவை_முனை_Can_Return_Subtype(Walker *pWalker, கோவை *p_கோவை){
  முழு n;
  FuncDef *pDef;
  sqlite3 *db;
  எனில்( p_கோவை->op==TK_CASE || p_கோவை->op==TK_UPLUS 
   || p_கோவை->op==TK_COLLATE || p_கோவை->op==TK_CAST 
  ){
    திரும்பு WRC_Continue;
  }
  எனில்( p_கோவை->op!=TK_FUNCTION && p_கோவை->op!=TK_AGG_FUNCTION ){
    திரும்பு WRC_Prune;
  }
  ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
  db = pWalker->pParse->db;
  n = p_கோவை->x.pList ? p_கோவை->x.pList->n_கோவை : 0;
  pDef = sqlite3FindFunction(db, p_கோவை->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  எனில்( pDef==0 ) திரும்பு WRC_Continue;
#இல்லையெனில்
  ப்ரதிஜ்ஞா( pDef!=0 );
#endif
  எனில்( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    திரும்பு WRC_Abort;
  }
  திரும்பு WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
முழு sqlite3_கோவை_Can_Return_Subtype(Parse *pParse, கோவை *p_கோவை){
  Walker w;
  memset(&w, 0, அளவு(w));
  w.pParse = pParse;
  w.x_கோவை_Callback = கோவை_முனை_Can_Return_Subtype;
  sqlite3_Walk_கோவை(&w, p_கோவை);
  திரும்பு w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
நிலையான SQLITE_NOINLINE முழு sqlite3_Indexed_கோவை_Lookup(
  Parse *pParse,   /* The parsing context */
  கோவை *p_கோவை,     /* The expression to potentially bypass */
  முழு target       /* Where to store the result of the expression */
){
  Indexed_கோவை *p;
  Vdbe *v;
  க்கு(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 கோவை_Aff;
    முழு iDataCur = p->iDataCur;
    எனில்( iDataCur<0 ) தொடர்;
    எனில்( pParse->iSelfTab ){
      எனில்( p->iDataCur!=pParse->iSelfTab-1 ) தொடர்;
      iDataCur = -1;
    }
    எனில்( sqlite3_கோவை_Compare(0, p_கோவை, p->p_கோவை, iDataCur)!=0 ) தொடர்;
    ப்ரதிஜ்ஞா( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    கோவை_Aff = sqlite3_கோவை_Affinity(p_கோவை);
    எனில்( (கோவை_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (கோவை_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (கோவை_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      தொடர்;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    எனில்( கோவை_Has_Property(p_கோவை, EP_SubtArg)
     && sqlite3_கோவை_Can_Return_Subtype(pParse, p_கோவை) 
    ){
      தொடர்;
    }

    v = pParse->pVdbe;
    ப்ரதிஜ்ஞா( v!=0 );
    எனில்( p->bMaybeNullRow || sqlite3_கோவை_Is_Constant(pParse, p_கோவை) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      முழு addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_கோவை_Code(pParse, p_கோவை, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }இல்லையெனில்{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    திரும்பு target;
  }
  திரும்பு -1;  /* Not found */
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
நிலையான முழு கோவை_Partidx_கோவை_Lookup(Parse *pParse, கோவை *p_கோவை, முழு iTarget){
  Indexed_கோவை *p;
  க்கு(p=pParse->p_Idx_Part_கோவை; p; p=p->pIENext){
    எனில்( p_கோவை->iColumn==p->iIdxCol && p_கோவை->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      முழு addr = 0;
      முழு ret;

      எனில்( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_கோவை_Code_Target(pParse, p->p_கோவை, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (மாறிலி எழுத்து*)&p->aff, 1);
      எனில்( sqlite3_கோவை_Can_Return_Subtype(pParse, p->p_கோவை) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      எனில்( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      திரும்பு ret;
    }
  }
  திரும்பு 0;
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
நிலையான SQLITE_NOINLINE முழு கோவை_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  கோவை *p_கோவை,       /* AND or OR expression to be coded */
  முழு target,        /* Put result in this register, guaranteed */
  முழு *pTmpReg       /* Write a temporary register here */
){
  முழு op;            /* The opcode.  TK_AND or TK_OR */
  முழு skipOp;        /* Opcode for the branch that skips one operand */
  முழு addrSkip;      /* Branch instruction that skips one of the operands */
  முழு regSS = 0;     /* Register holding computed operand when other omitted */
  முழு r1, r2;        /* Registers for left and right operands, respectively */
  கோவை *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  ப்ரதிஜ்ஞா( p_கோவை!=0 );
  op = p_கோவை->op;
  ப்ரதிஜ்ஞா( op==TK_AND || op==TK_OR );
  ப்ரதிஜ்ஞா( TK_AND==OP_And );            testcase( op==TK_AND );
  ப்ரதிஜ்ஞா( TK_OR==OP_Or );              testcase( op==TK_OR );
  ப்ரதிஜ்ஞா( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_கோவை_Simplified_And_Or(p_கோவை);
  எனில்( pAlt!=p_கோவை ){
    r1 = sqlite3_கோவை_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    திரும்பு target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  எனில்( கோவை_Eval_Rhs_First(p_கோவை) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_கோவை_Code_Target(pParse, p_கோவை->p_வலது, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, pTmpReg);
  }இல்லையெனில்{
    /* Compute the left operand first */
    r1 = sqlite3_கோவை_Code_Target(pParse, p_கோவை->p_இடது, target);
    எனில்( கோவை_Has_Property(p_கோவை->p_வலது, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }இல்லையெனில்{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_வலது, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  எனில்( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  திரும்பு target;
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
முழு sqlite3_கோவை_Code_Target(Parse *pParse, கோவை *p_கோவை, முழு target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  முழு op;                   /* The opcode being coded */
  முழு inReg = target;       /* Results stored in register inReg */
  முழு regFree1 = 0;         /* If non-zero free this temporary register */
  முழு regFree2 = 0;         /* If non-zero free this temporary register */
  முழு r1, r2;               /* Various register numbers */
  கோவை tempX;               /* Temporary expression node */
  முழு p5 = 0;

  ப்ரதிஜ்ஞா( target>0 && target<=pParse->nMem );
  ப்ரதிஜ்ஞா( v!=0 );

கோவை_code_doover:
  எனில்( p_கோவை==0 ){
    op = TK_NULL;
  }இல்லையெனில் எனில்( pParse->pIdxEpr!=0
   && !கோவை_Has_Property(p_கோவை, EP_Leaf)
   && (r1 = sqlite3_Indexed_கோவை_Lookup(pParse, p_கோவை, target))>=0
  ){
    திரும்பு r1;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( !கோவை_Has_VVAProperty(p_கோவை,EP_Immutable) );
    op = p_கோவை->op;
  }
  ப்ரதிஜ்ஞா( op!=TK_ORDER );
  தேர்வு( op ){
    நிலை TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_கோவை->pAggInfo;
      கட்டமைப்பு AggInfo_col *pCol;
      ப்ரதிஜ்ஞா( pAggInfo!=0 );
      ப்ரதிஜ்ஞா( p_கோவை->iAgg>=0 );
      எனில்( p_கோவை->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        நிறுத்து;
      }
      pCol = &pAggInfo->aCol[p_கோவை->iAgg];
      எனில்( !pAggInfo->directMode ){
        திரும்பு AggInfoColumnReg(pAggInfo, p_கோவை->iAgg);
      }இல்லையெனில் எனில்( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        எனில்( pTab==0 ){
          /* No comment added */
        }இல்லையெனில் எனில்( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }இல்லையெனில்{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          எனில்( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        திரும்பு target;
      }இல்லையெனில் எனில்( p_கோவை->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_கோவை->iTable, p_கோவை->iColumn, target);
        திரும்பு target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    நிலை TK_COLUMN: {
      முழு iTab = p_கோவை->iTable;
      முழு iReg;
      எனில்( கோவை_Has_Property(p_கோவை, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        முழு aff;
        iReg = sqlite3_கோவை_Code_Target(pParse, p_கோவை->p_இடது,target);
        ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
        ப்ரதிஜ்ஞா( p_கோவை->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_கோவை->y.pTab, p_கோவை->iColumn);
        எனில்( aff>SQLITE_AFF_BLOB ){
          நிலையான மாறிலி எழுத்து zAff[] = "B\000C\000D\000E\000F";
          ப்ரதிஜ்ஞா( SQLITE_AFF_BLOB=='A' );
          ப்ரதிஜ்ஞா( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        திரும்பு iReg;
      }
      எனில்( iTab<0 ){
        எனில்( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          முழு iSrc;
          முழு iCol = p_கோவை->iColumn;
          ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
          pTab = p_கோவை->y.pTab;
          ப்ரதிஜ்ஞா( pTab!=0 );
          ப்ரதிஜ்ஞா( iCol>=XN_ROWID );
          ப்ரதிஜ்ஞா( iCol<pTab->nCol );
          எனில்( iCol<0 ){
            திரும்பு -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          எனில்( pCol->colFlags & COLFLAG_GENERATED ){
            எனில்( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_செய்தி(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              திரும்பு 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            எனில்( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_கோவை_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            திரும்பு iSrc;
          }இல்லையெனில்
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          எனில்( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            திரும்பு target;
          }இல்லையெனில்{
            திரும்பு iSrc;
          }
        }இல்லையெனில்{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      இல்லையெனில் எனில்( pParse->p_Idx_Part_கோவை 
       && 0!=(r1 = கோவை_Partidx_கோவை_Lookup(pParse, p_கோவை, target))
      ){
        திரும்பு r1;
      }
      ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
      ப்ரதிஜ்ஞா( p_கோவை->y.pTab!=0 );
      iReg = sqlite3_கோவை_Code_Get_Column(pParse, p_கோவை->y.pTab,
                               p_கோவை->iColumn, iTab, target,
                               p_கோவை->op2);
      திரும்பு iReg;
    }
    நிலை TK_INTEGER: {
      codeInteger(pParse, p_கோவை, 0, target);
      திரும்பு target;
    }
    நிலை TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_கோவை_Truth_மதிப்பு(p_கோவை), target);
      திரும்பு target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    நிலை TK_FLOAT: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      codeReal(v, p_கோவை->u.zToken, 0, target);
      திரும்பு target;
    }
#endif
    நிலை TK_STRING: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3VdbeLoadString(v, target, p_கோவை->u.zToken);
      திரும்பு target;
    }
    நிலை TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_கோவை->y.nReg - 1);
      திரும்பு target;
    }
    இயல்பு: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      ப்ரதிஜ்ஞா( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      திரும்பு target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    நிலை TK_BLOB: {
      முழு n;
      மாறிலி எழுத்து *z;
      எழுத்து *zBlob;
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      ப்ரதிஜ்ஞா( p_கோவை->u.zToken[0]=='x' || p_கோவை->u.zToken[0]=='X' );
      ப்ரதிஜ்ஞா( p_கோவை->u.zToken[1]=='\'' );
      z = &p_கோவை->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      ப்ரதிஜ்ஞா( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      திரும்பு target;
    }
#endif
    நிலை TK_VARIABLE: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      ப்ரதிஜ்ஞா( p_கோவை->u.zToken!=0 );
      ப்ரதிஜ்ஞா( p_கோவை->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_கோவை->iColumn, target);
      திரும்பு target;
    }
    நிலை TK_REGISTER: {
      திரும்பு p_கோவை->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    நிலை TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_கோவை_Code(pParse, p_கோவை->p_இடது, target);
      ப்ரதிஜ்ஞா( inReg==target );
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_கோவை->u.zToken, 0));
      திரும்பு inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    நிலை TK_IS:
    நிலை TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    நிலை TK_LT:
    நிலை TK_LE:
    நிலை TK_GT:
    நிலை TK_GE:
    நிலை TK_NE:
    நிலை TK_EQ: {
      கோவை *p_இடது = p_கோவை->p_இடது;
      முழு addrIsNull = 0;
      எனில்( sqlite3_கோவை_Is_Vector(p_இடது) ){
        codeVectorCompare(pParse, p_கோவை, target, op, p5);
      }இல்லையெனில்{
        எனில்( கோவை_Has_Property(p_கோவை, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = கோவை_Compute_Operands(pParse, p_கோவை,
                                     &r1, &r2, &regFree1, &regFree2);
        }இல்லையெனில்{
          r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
          r2 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_வலது, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_இடது, p_கோவை->p_வலது, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            கோவை_Has_Property(p_கோவை,EP_Commuted));
        ப்ரதிஜ்ஞா(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        ப்ரதிஜ்ஞா(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        ப்ரதிஜ்ஞா(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        ப்ரதிஜ்ஞா(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        ப்ரதிஜ்ஞா(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        ப்ரதிஜ்ஞா(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        எனில்( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }இல்லையெனில்{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          எனில்( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      நிறுத்து;
    }
    நிலை TK_AND:
    நிலை TK_OR: {
      inReg = கோவை_Code_Target_And_Or(pParse, p_கோவை, target, &regFree1);
      நிறுத்து;
    }
    நிலை TK_PLUS:
    நிலை TK_STAR:
    நிலை TK_MINUS:
    நிலை TK_REM:
    நிலை TK_BITAND:
    நிலை TK_BITOR:
    நிலை TK_SLASH:
    நிலை TK_LSHIFT:
    நிலை TK_RSHIFT:
    நிலை TK_CONCAT: {
      முழு addrIsNull;
      ப்ரதிஜ்ஞா( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      ப்ரதிஜ்ஞா( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      ப்ரதிஜ்ஞா( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      ப்ரதிஜ்ஞா( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      ப்ரதிஜ்ஞா( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      ப்ரதிஜ்ஞா( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      ப்ரதிஜ்ஞா( TK_LSHIFT==OP_Shift_இடது );   testcase( op==TK_LSHIFT );
      ப்ரதிஜ்ஞா( TK_RSHIFT==OP_Shift_வலது );  testcase( op==TK_RSHIFT );
      ப்ரதிஜ்ஞா( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      எனில்( கோவை_Has_Property(p_கோவை, EP_Subquery) ){
        addrIsNull = கோவை_Compute_Operands(pParse, p_கோவை,
                                   &r1, &r2, &regFree1, &regFree2);
      }இல்லையெனில்{
        r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
        r2 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_வலது, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      எனில்( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      நிறுத்து;
    }
    நிலை TK_UMINUS: {
      கோவை *p_இடது = p_கோவை->p_இடது;
      ப்ரதிஜ்ஞா( p_இடது );
      எனில்( p_இடது->op==TK_INTEGER ){
        codeInteger(pParse, p_இடது, 1, target);
        திரும்பு target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }இல்லையெனில் எனில்( p_இடது->op==TK_FLOAT ){
        ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
        codeReal(v, p_இடது->u.zToken, 1, target);
        திரும்பு target;
#endif
      }இல்லையெனில்{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_மதிப்பு|EP_TokenOnly;
        tempX.u.i_மதிப்பு = 0;
        கோவை_Clear_VVAProperties(&tempX);
        r1 = sqlite3_கோவை_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      நிறுத்து;
    }
    நிலை TK_BITNOT:
    நிலை TK_NOT: {
      ப்ரதிஜ்ஞா( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      ப்ரதிஜ்ஞா( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      நிறுத்து;
    }
    நிலை TK_TRUTH: {
      முழு isTrue;    /* IS TRUE or IS NOT TRUE */
      முழு bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_கோவை_Truth_மதிப்பு(p_கோவை->p_வலது);
      bNormal = p_கோவை->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      நிறுத்து;
    }
    நிலை TK_ISNULL:
    நிலை TK_NOTNULL: {
      முழு addr;
      ப்ரதிஜ்ஞா( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ப்ரதிஜ்ஞா( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      நிறுத்து;
    }
    நிலை TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_கோவை->pAggInfo;
      எனில்( pInfo==0
       || NEVER(p_கோவை->iAgg<0)
       || NEVER(p_கோவை->iAgg>=pInfo->nFunc)
      ){
        ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
        sqlite3_Error_செய்தி(pParse, "misuse of aggregate: %#T()", p_கோவை);
      }இல்லையெனில்{
        திரும்பு AggInfoFuncReg(pInfo, p_கோவை->iAgg);
      }
      நிறுத்து;
    }
    நிலை TK_FUNCTION: {
      கோவை_List *pFarg;       /* List of function arguments */
      முழு nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      மாறிலி எழுத்து *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      முழு i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      எனில்( கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
        திரும்பு p_கோவை->y.pWin->regResult;
      }
#endif

      எனில்( ConstFactorOk(pParse)
       && sqlite3_கோவை_Is_Constant_Not_Join(pParse,p_கோவை)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        திரும்பு sqlite3_கோவை_Code_Run_Just_Once(pParse, p_கோவை, -1);
      }
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_TokenOnly) );
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
      pFarg = p_கோவை->x.pList;
      nFarg = pFarg ? pFarg->n_கோவை : 0;
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      zId = p_கோவை->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      எனில்( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      எனில்( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_செய்தி(pParse, "unknown function: %#T()", p_கோவை);
        நிறுத்து;
      }
      எனில்( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        ப்ரதிஜ்ஞா( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        ப்ரதிஜ்ஞா( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        திரும்பு கோவை_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }இல்லையெனில் எனில்( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_கோவை_Function_Usable(pParse, p_கோவை, pDef);
      }

      க்கு(i=0; i<nFarg; i++){
        எனில்( i<32 && sqlite3_கோவை_Is_Constant(pParse, pFarg->a[i].p_கோவை) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        எனில்( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_கோவை_Coll_Seq(pParse, pFarg->a[i].p_கோவை);
        }
      }
      எனில்( pFarg ){
        எனில்( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }இல்லையெனில்{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        எனில்( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 கோவை_Op;
          ப்ரதிஜ்ஞா( nFarg==1 );
          ப்ரதிஜ்ஞா( pFarg->a[0].p_கோவை!=0 );
          கோவை_Op = pFarg->a[0].p_கோவை->op;
          எனில்( கோவை_Op==TK_COLUMN || கோவை_Op==TK_AGG_COLUMN ){
            ப்ரதிஜ்ஞா( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            ப்ரதிஜ்ஞா( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            ப்ரதிஜ்ஞா( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            ப்ரதிஜ்ஞா( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_கோவை->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_கோவை_Code_கோவை_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }இல்லையெனில்{
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
      எனில்( nFarg>=2 && கோவை_Has_Property(p_கோவை, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_கோவை);
      }இல்லையெனில் எனில்( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_கோவை);
      }
#endif
      எனில்( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        எனில்( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (எழுத்து *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_கோவை->op2);
      எனில்( nFarg ){
        எனில்( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }இல்லையெனில்{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      திரும்பு target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    நிலை TK_EXISTS:
    நிலை TK_SELECT: {
      முழு nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      எனில்( pParse->db->mallocFailed ){
        திரும்பு 0;
      }இல்லையெனில் எனில்( op==TK_SELECT
             && ALWAYS( கோவை_Use_XSelect(p_கோவை) )
             && (nCol = p_கோவை->x.pSelect->pEList->n_கோவை)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }இல்லையெனில்{
        திரும்பு sqlite3CodeSubselect(pParse, p_கோவை);
      }
      நிறுத்து;
    }
    நிலை TK_SELECT_COLUMN: {
      முழு n;
      கோவை *p_இடது = p_கோவை->p_இடது;
      எனில்( p_இடது->iTable==0 || pParse->withinRJSubrtn > p_இடது->op2 ){
        p_இடது->iTable = sqlite3CodeSubselect(pParse, p_இடது);
        p_இடது->op2 = pParse->withinRJSubrtn;
      }
      ப்ரதிஜ்ஞா( p_இடது->op==TK_SELECT || p_இடது->op==TK_ERROR );
      n = sqlite3_கோவை_Vector_Size(p_இடது);
      எனில்( p_கோவை->iTable!=n ){
        sqlite3_Error_செய்தி(pParse, "%d columns assigned %d values",
                                p_கோவை->iTable, n);
      }
      திரும்பு p_இடது->iTable + p_கோவை->iColumn;
    }
    நிலை TK_IN: {
      முழு destIfFalse = sqlite3VdbeMakeLabel(pParse);
      முழு destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_கோவை_Code_IN(pParse, p_கோவை, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      திரும்பு target;
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
    நிலை TK_BETWEEN: {
      கோவை_Code_Between(pParse, p_கோவை, target, 0, 0);
      திரும்பு target;
    }
    நிலை TK_COLLATE: {
      எனில்( !கோவை_Has_Property(p_கோவை, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        ப்ரதிஜ்ஞா( p_கோவை->p_இடது );
        sqlite3_கோவை_Code(pParse, p_கோவை->p_இடது, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        திரும்பு target;
      }இல்லையெனில்{
        p_கோவை = p_கோவை->p_இடது;
        செல் கோவை_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    நிலை TK_SPAN:
    நிலை TK_UPLUS: {
      p_கோவை = p_கோவை->p_இடது;
      செல் கோவை_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    நிலை TK_TRIGGER: {
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
      முழு iCol;
      முழு p1;

      ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
      pTab = p_கோவை->y.pTab;
      iCol = p_கோவை->iColumn;
      p1 = p_கோவை->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      ப்ரதிஜ்ஞா( p_கோவை->iTable==0 || p_கோவை->iTable==1 );
      ப்ரதிஜ்ஞா( iCol>=-1 && iCol<pTab->nCol );
      ப்ரதிஜ்ஞா( pTab->iPKey<0 || iCol!=pTab->iPKey );
      ப்ரதிஜ்ஞா( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_கோவை->iTable ? "new" : "old"),
        (p_கோவை->iColumn<0 ? "rowid" : p_கோவை->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      எனில்( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      நிறுத்து;
    }

    நிலை TK_VECTOR: {
      sqlite3_Error_செய்தி(pParse, "row value misused");
      நிறுத்து;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    நிலை TK_IF_NULL_ROW: {
      முழு addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_கோவை->pAggInfo;
      எனில்( pAggInfo ){
        ப்ரதிஜ்ஞா( p_கோவை->iAgg>=0 && p_கோவை->iAgg<pAggInfo->nColumn );
        எனில்( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_கோவை->iAgg);
          நிறுத்து;
        }
        எனில்( p_கோவை->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_கோவை->iAgg].iSorterColumn,
                            target);
          inReg = target;
          நிறுத்து;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_கோவை->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_கோவை_Code(pParse, p_கோவை->p_இடது, target);
      ப்ரதிஜ்ஞா( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      நிறுத்து;
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
    நிலை TK_CASE: {
      முழு endLabel;                     /* GOTO label for end of CASE stmt */
      முழு nextCase;                     /* GOTO label for next WHEN clause */
      முழு n_கோவை;                        /* 2x number of WHEN terms */
      முழு i;                            /* Loop counter */
      கோவை_List *pEList;                 /* List of WHEN terms */
      கட்டமைப்பு கோவை_List_item *aListelem;  /* Array of WHEN terms */
      கோவை opCompare;                   /* The X==Ei expression */
      கோவை *pX;                         /* The X expression */
      கோவை *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      கோவை *pDel = 0;
      sqlite3 *db = pParse->db;

      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) && p_கோவை->x.pList!=0 );
      ப்ரதிஜ்ஞா(p_கோவை->x.pList->n_கோவை > 0);
      pEList = p_கோவை->x.pList;
      aListelem = pEList->a;
      n_கோவை = pEList->n_கோவை;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      எனில்( (pX = p_கோவை->p_இடது)!=0 ){
        pDel = sqlite3_கோவை_Dup(db, pX, 0);
        எனில்( db->mallocFailed ){
          sqlite3_கோவை_Delete(db, pDel);
          நிறுத்து;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_கோவை_To_Register(pDel, கோவை_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, அளவு(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_இடது = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      க்கு(i=0; i<n_கோவை-1; i=i+2){
        எனில்( pX ){
          ப்ரதிஜ்ஞா( pTest!=0 );
          opCompare.p_வலது = aListelem[i].p_கோவை;
        }இல்லையெனில்{
          pTest = aListelem[i].p_கோவை;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_கோவை_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_கோவை->op==TK_COLUMN );
        sqlite3_கோவை_Code(pParse, aListelem[i+1].p_கோவை, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      எனில்( (n_கோவை&1)!=0 ){
        sqlite3_கோவை_Code(pParse, pEList->a[n_கோவை-1].p_கோவை, target);
      }இல்லையெனில்{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_கோவை_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_TRIGGER
    நிலை TK_RAISE: {
      ப்ரதிஜ்ஞா( p_கோவை->aff_கோவை==OE_Rollback
           || p_கோவை->aff_கோவை==OE_Abort
           || p_கோவை->aff_கோவை==OE_Fail
           || p_கோவை->aff_கோவை==OE_Ignore
      );
      எனில்( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_செய்தி(pParse,
                       "RAISE() may only be used within a trigger-program");
        திரும்பு 0;
      }
      எனில்( p_கோவை->aff_கோவை==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      எனில்( p_கோவை->aff_கோவை==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }இல்லையெனில்{
        r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_கோவை->aff_கோவை, r1);
      }
      நிறுத்து;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  திரும்பு inReg;
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
முழு sqlite3_கோவை_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  கோவை *p_கோவை,      /* The expression to code when the VDBE initializes */
  முழு regDest       /* Store the value in this register */
){
  கோவை_List *p;
  ப்ரதிஜ்ஞா( ConstFactorOk(pParse) );
  ப்ரதிஜ்ஞா( regDest!=0 );
  p = pParse->p_Const_கோவை;
  எனில்( regDest<0 && p ){
    கட்டமைப்பு கோவை_List_item *pItem;
    முழு i;
    க்கு(pItem=p->a, i=p->n_கோவை; i>0; pItem++, i--){
      எனில்( pItem->fg.reusable
       && sqlite3_கோவை_Compare(0,pItem->p_கோவை,p_கோவை,-1)==0
      ){
        திரும்பு pItem->u.i_Const_கோவை_Reg;
      }
    }
  }
  p_கோவை = sqlite3_கோவை_Dup(pParse->db, p_கோவை, 0);
  எனில்( p_கோவை!=0 && கோவை_Has_Property(p_கோவை, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    முழு addr;
    ப்ரதிஜ்ஞா( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    எனில்( !pParse->db->mallocFailed ){
      எனில்( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_கோவை_Code(pParse, p_கோவை, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_கோவை_Delete(pParse->db, p_கோவை);
    sqlite3VdbeJumpHere(v, addr);
  }இல்லையெனில்{
    p = sqlite3_கோவை_List_Append(pParse, p, p_கோவை);
    எனில்( p ){
       கட்டமைப்பு கோவை_List_item *pItem = &p->a[p->n_கோவை-1];
       pItem->fg.reusable = regDest<0;
       எனில்( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_கோவை_Reg = regDest;
    }
    pParse->p_Const_கோவை = p;
  }
  திரும்பு regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE வெறுமை sqlite3_கோவை_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  முழு iReg,        /* First register to set to NULL */
  முழு nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  கோவை t;
  memset(&t, 0, அளவு(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_கோவை_Code_Run_Just_Once(pParse, &t, iReg);
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
முழு sqlite3_கோவை_Code_Temp(Parse *pParse, கோவை *p_கோவை, முழு *pReg){
  முழு r2;
  p_கோவை = sqlite3_கோவை_Skip_Collate_And_Likely(p_கோவை);
  எனில்( ConstFactorOk(pParse)
   && ALWAYS(p_கோவை!=0)
   && p_கோவை->op!=TK_REGISTER
   && sqlite3_கோவை_Is_Constant_Not_Join(pParse, p_கோவை)
  ){
    *pReg  = 0;
    r2 = sqlite3_கோவை_Code_Run_Just_Once(pParse, p_கோவை, -1);
  }இல்லையெனில்{
    முழு r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_கோவை_Code_Target(pParse, p_கோவை, r1);
    எனில்( r2==r1 ){
      *pReg = r1;
    }இல்லையெனில்{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  திரும்பு r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
வெறுமை sqlite3_கோவை_Code(Parse *pParse, கோவை *p_கோவை, முழு target){
  முழு inReg;

  ப்ரதிஜ்ஞா( p_கோவை==0 || !கோவை_Has_VVAProperty(p_கோவை,EP_Immutable) );
  ப்ரதிஜ்ஞா( target>0 && target<=pParse->nMem );
  ப்ரதிஜ்ஞா( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  எனில்( pParse->pVdbe==0 ) திரும்பு;
  inReg = sqlite3_கோவை_Code_Target(pParse, p_கோவை, target);
  எனில்( inReg!=target ){
    u8 op;
    கோவை *pX = sqlite3_கோவை_Skip_Collate_And_Likely(p_கோவை);
    testcase( pX!=p_கோவை );
    எனில்( ALWAYS(pX)
     && (கோவை_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }இல்லையெனில்{
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
வெறுமை sqlite3_கோவை_Code_Copy(Parse *pParse, கோவை *p_கோவை, முழு target){
  sqlite3 *db = pParse->db;
  p_கோவை = sqlite3_கோவை_Dup(db, p_கோவை, 0);
  எனில்( !db->mallocFailed ) sqlite3_கோவை_Code(pParse, p_கோவை, target);
  sqlite3_கோவை_Delete(db, p_கோவை);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
வெறுமை sqlite3_கோவை_Code_Factorable(Parse *pParse, கோவை *p_கோவை, முழு target){
  எனில்( pParse->okConstFactor && sqlite3_கோவை_Is_Constant_Not_Join(pParse,p_கோவை) ){
    sqlite3_கோவை_Code_Run_Just_Once(pParse, p_கோவை, target);
  }இல்லையெனில்{
    sqlite3_கோவை_Code_Copy(pParse, p_கோவை, target);
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
முழு sqlite3_கோவை_Code_கோவை_List(
  Parse *pParse,     /* Parsing context */
  கோவை_List *pList,   /* The expression list to be coded */
  முழு target,        /* Where to write results */
  முழு srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  கட்டமைப்பு கோவை_List_item *pItem;
  முழு i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  ப்ரதிஜ்ஞா( pList!=0 );
  ப்ரதிஜ்ஞா( target>0 );
  ப்ரதிஜ்ஞா( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_கோவை;
  எனில்( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  க்கு(pItem=pList->a, i=0; i<n; i++, pItem++){
    கோவை *p_கோவை = pItem->p_கோவை;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    எனில்( pItem->fg.bSorterRef ){
      i--;
      n--;
    }இல்லையெனில்
#endif
    எனில்( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      எனில்( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }இல்லையெனில்{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }இல்லையெனில் எனில்( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_கோவை_Is_Constant_Not_Join(pParse,p_கோவை)
    ){
      sqlite3_கோவை_Code_Run_Just_Once(pParse, p_கோவை, target+i);
    }இல்லையெனில்{
      முழு inReg = sqlite3_கோவை_Code_Target(pParse, p_கோவை, target+i);
      எனில்( inReg!=target+i ){
        VdbeOp *pOp;
        எனில்( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }இல்லையெனில்{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  திரும்பு n;
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
நிலையான வெறுமை கோவை_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  கோவை *p_கோவை,      /* The BETWEEN expression */
  முழு dest,         /* Jump destination or storage location */
  வெறுமை (*xJump)(Parse*,கோவை*,முழு,முழு), /* Action to take */
  முழு jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  கோவை கோவை_And;     /* The AND operator in  x>=y AND x<=z  */
  கோவை comp_இடது;    /* The  x>=y  term */
  கோவை comp_வலது;   /* The  x<=z  term */
  முழு regFree1 = 0; /* Temporary use register */
  கோவை *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_இடது, 0, அளவு(கோவை));
  memset(&comp_வலது, 0, அளவு(கோவை));
  memset(&கோவை_And, 0, அளவு(கோவை));

  ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
  pDel = sqlite3_கோவை_Dup(db, p_கோவை->p_இடது, 0);
  எனில்( db->mallocFailed==0 ){
    கோவை_And.op = TK_AND;
    கோவை_And.p_இடது = &comp_இடது;
    கோவை_And.p_வலது = &comp_வலது;
    comp_இடது.op = TK_GE;
    comp_இடது.p_இடது = pDel;
    comp_இடது.p_வலது = p_கோவை->x.pList->a[0].p_கோவை;
    comp_வலது.op = TK_LE;
    comp_வலது.p_இடது = pDel;
    comp_வலது.p_வலது = p_கோவை->x.pList->a[1].p_கோவை;
    sqlite3_கோவை_To_Register(pDel, கோவை_Code_Vector(pParse, pDel, &regFree1));
    எனில்( xJump ){
      xJump(pParse, &கோவை_And, dest, jumpIfNull);
    }இல்லையெனில்{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_கோவை_Code_Target(pParse, &கோவை_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3_கோவை_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_கோவை_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_கோவை_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_கோவை_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_கோவை_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_கோவை_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_கோவை_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_கோவை_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_கோவை_If_False && jumpIfNull!=0 && regFree1!=0 );
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
வெறுமை sqlite3_கோவை_If_True(Parse *pParse, கோவை *p_கோவை, முழு dest, முழு jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  முழு op = 0;
  முழு regFree1 = 0;
  முழு regFree2 = 0;
  முழு r1, r2;

  ப்ரதிஜ்ஞா( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  எனில்( NEVER(v==0) )     திரும்பு;  /* Existence of VDBE checked by caller */
  எனில்( NEVER(p_கோவை==0) ) திரும்பு;  /* No way this can happen */
  ப்ரதிஜ்ஞா( !கோவை_Has_VVAProperty(p_கோவை, EP_Immutable) );
  op = p_கோவை->op;
  தேர்வு( op ){
    நிலை TK_AND:
    நிலை TK_OR: {
      கோவை *pAlt = sqlite3_கோவை_Simplified_And_Or(p_கோவை);
      எனில்( pAlt!=p_கோவை ){
        sqlite3_கோவை_If_True(pParse, pAlt, dest, jumpIfNull);
      }இல்லையெனில்{
        கோவை *pFirst, *pSecond;
        எனில்( கோவை_Eval_Rhs_First(p_கோவை) ){
          pFirst = p_கோவை->p_வலது;
          pSecond = p_கோவை->p_இடது;
        }இல்லையெனில்{
          pFirst = p_கோவை->p_இடது;
          pSecond = p_கோவை->p_வலது;
        }
        எனில்( op==TK_AND ){
          முழு d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_கோவை_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_கோவை_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }இல்லையெனில்{
          testcase( jumpIfNull==0 );
          sqlite3_கோவை_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_கோவை_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      நிறுத்து;
    }
    நிலை TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_கோவை_If_False(pParse, p_கோவை->p_இடது, dest, jumpIfNull);
      நிறுத்து;
    }
    நிலை TK_TRUTH: {
      முழு isNot;      /* IS NOT TRUE or IS NOT FALSE */
      முழு isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_கோவை->op2==TK_ISNOT;
      isTrue = sqlite3_கோவை_Truth_மதிப்பு(p_கோவை->p_வலது);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      எனில்( isTrue ^ isNot ){
        sqlite3_கோவை_If_True(pParse, p_கோவை->p_இடது, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }இல்லையெனில்{
        sqlite3_கோவை_If_False(pParse, p_கோவை->p_இடது, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      நிறுத்து;
    }
    நிலை TK_IS:
    நிலை TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    நிலை TK_LT:
    நிலை TK_LE:
    நிலை TK_GT:
    நிலை TK_GE:
    நிலை TK_NE:
    நிலை TK_EQ: {
      முழு addrIsNull;
      எனில்( sqlite3_கோவை_Is_Vector(p_கோவை->p_இடது) ) செல் default_கோவை;
      எனில்( கோவை_Has_Property(p_கோவை, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = கோவை_Compute_Operands(pParse, p_கோவை,
                                   &r1, &r2, &regFree1, &regFree2);
      }இல்லையெனில்{
        r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
        r2 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_வலது, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_கோவை->p_இடது, p_கோவை->p_வலது, op,
                  r1, r2, dest, jumpIfNull, கோவை_Has_Property(p_கோவை,EP_Commuted));
      ப்ரதிஜ்ஞா(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ப்ரதிஜ்ஞா(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ப்ரதிஜ்ஞா(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ப்ரதிஜ்ஞா(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ப்ரதிஜ்ஞா(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      ப்ரதிஜ்ஞா(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      எனில்( addrIsNull ){
        எனில்( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }இல்லையெனில்{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      நிறுத்து;
    }
    நிலை TK_ISNULL:
    நிலை TK_NOTNULL: {
      ப்ரதிஜ்ஞா( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ப்ரதிஜ்ஞா( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
      ப்ரதிஜ்ஞா( regFree1==0 || regFree1==r1 );
      எனில்( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      நிறுத்து;
    }
    நிலை TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      கோவை_Code_Between(pParse, p_கோவை, dest, sqlite3_கோவை_If_True, jumpIfNull);
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    நிலை TK_IN: {
      முழு destIfFalse = sqlite3VdbeMakeLabel(pParse);
      முழு destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_கோவை_Code_IN(pParse, p_கோவை, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      நிறுத்து;
    }
#endif
    இயல்பு: {
    default_கோவை:
      எனில்( கோவை_Always_True(p_கோவை) ){
        sqlite3VdbeGoto(v, dest);
      }இல்லையெனில் எனில்( கோவை_Always_False(p_கோவை) ){
        /* No-op */
      }இல்லையெனில்{
        r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      நிறுத்து;
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
வெறுமை sqlite3_கோவை_If_False(Parse *pParse, கோவை *p_கோவை, முழு dest, முழு jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  முழு op = 0;
  முழு regFree1 = 0;
  முழு regFree2 = 0;
  முழு r1, r2;

  ப்ரதிஜ்ஞா( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  எனில்( NEVER(v==0) ) திரும்பு; /* Existence of VDBE checked by caller */
  எனில்( p_கோவை==0 )    திரும்பு;
  ப்ரதிஜ்ஞா( !கோவை_Has_VVAProperty(p_கோவை,EP_Immutable) );

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
  op = ((p_கோவை->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_ISNULL || op==OP_NotNull );
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_NOTNULL || op==OP_IsNull );
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_NE || op==OP_Eq );
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_EQ || op==OP_Ne );
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_LT || op==OP_Ge );
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_LE || op==OP_Gt );
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_GT || op==OP_Le );
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_GE || op==OP_Lt );

  தேர்வு( p_கோவை->op ){
    நிலை TK_AND:
    நிலை TK_OR: {
      கோவை *pAlt = sqlite3_கோவை_Simplified_And_Or(p_கோவை);
      எனில்( pAlt!=p_கோவை ){
        sqlite3_கோவை_If_False(pParse, pAlt, dest, jumpIfNull);
      }இல்லையெனில்{
        கோவை *pFirst, *pSecond;
        எனில்( கோவை_Eval_Rhs_First(p_கோவை) ){
          pFirst = p_கோவை->p_வலது;
          pSecond = p_கோவை->p_இடது;
        }இல்லையெனில்{
          pFirst = p_கோவை->p_இடது;
          pSecond = p_கோவை->p_வலது;
        }
        எனில்( p_கோவை->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_கோவை_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_கோவை_If_False(pParse, pSecond, dest, jumpIfNull);
        }இல்லையெனில்{
          முழு d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_கோவை_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_கோவை_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      நிறுத்து;
    }
    நிலை TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_கோவை_If_True(pParse, p_கோவை->p_இடது, dest, jumpIfNull);
      நிறுத்து;
    }
    நிலை TK_TRUTH: {
      முழு isNot;   /* IS NOT TRUE or IS NOT FALSE */
      முழு isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_கோவை->op2==TK_ISNOT;
      isTrue = sqlite3_கோவை_Truth_மதிப்பு(p_கோவை->p_வலது);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      எனில்( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_கோவை_If_False(pParse, p_கோவை->p_இடது, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }இல்லையெனில்{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_கோவை_If_True(pParse, p_கோவை->p_இடது, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      நிறுத்து;
    }
    நிலை TK_IS:
    நிலை TK_ISNOT:
      testcase( p_கோவை->op==TK_IS );
      testcase( p_கோவை->op==TK_ISNOT );
      op = (p_கோவை->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    நிலை TK_LT:
    நிலை TK_LE:
    நிலை TK_GT:
    நிலை TK_GE:
    நிலை TK_NE:
    நிலை TK_EQ: {
      முழு addrIsNull;
      எனில்( sqlite3_கோவை_Is_Vector(p_கோவை->p_இடது) ) செல் default_கோவை;
      எனில்( கோவை_Has_Property(p_கோவை, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = கோவை_Compute_Operands(pParse, p_கோவை,
                                   &r1, &r2, &regFree1, &regFree2);
      }இல்லையெனில்{
        r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
        r2 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_வலது, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_கோவை->p_இடது, p_கோவை->p_வலது, op,
                  r1, r2, dest, jumpIfNull,கோவை_Has_Property(p_கோவை,EP_Commuted));
      ப்ரதிஜ்ஞா(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ப்ரதிஜ்ஞா(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ப்ரதிஜ்ஞா(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ப்ரதிஜ்ஞா(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ப்ரதிஜ்ஞா(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      ப்ரதிஜ்ஞா(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      எனில்( addrIsNull ){
        எனில்( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }இல்லையெனில்{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      நிறுத்து;
    }
    நிலை TK_ISNULL:
    நிலை TK_NOTNULL: {
      r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை->p_இடது, &regFree1);
      ப்ரதிஜ்ஞா( regFree1==0 || regFree1==r1 );
      எனில்( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      நிறுத்து;
    }
    நிலை TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      கோவை_Code_Between(pParse, p_கோவை, dest, sqlite3_கோவை_If_False, jumpIfNull);
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    நிலை TK_IN: {
      எனில்( jumpIfNull ){
        sqlite3_கோவை_Code_IN(pParse, p_கோவை, dest, dest);
      }இல்லையெனில்{
        முழு destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_கோவை_Code_IN(pParse, p_கோவை, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      நிறுத்து;
    }
#endif
    இயல்பு: {
    default_கோவை:
      எனில்( கோவை_Always_False(p_கோவை) ){
        sqlite3VdbeGoto(v, dest);
      }இல்லையெனில் எனில்( கோவை_Always_True(p_கோவை) ){
        /* no-op */
      }இல்லையெனில்{
        r1 = sqlite3_கோவை_Code_Temp(pParse, p_கோவை, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      நிறுத்து;
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
வெறுமை sqlite3_கோவை_If_False_Dup(Parse *pParse, கோவை *p_கோவை, முழு dest,முழு jumpIfNull){
  sqlite3 *db = pParse->db;
  கோவை *pCopy = sqlite3_கோவை_Dup(db, p_கோவை, 0);
  எனில்( db->mallocFailed==0 ){
    sqlite3_கோவை_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_கோவை_Delete(db, pCopy);
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
நிலையான SQLITE_NOINLINE முழு கோவை_Compare_Variable(
  மாறிலி Parse *pParse,
  மாறிலி கோவை *pVar,
  மாறிலி கோவை *p_கோவை
){
  முழு res = 2;
  முழு iVar;
  sqlite3_மதிப்பு *pL, *pR = 0;
 
  எனில்( p_கோவை->op==TK_VARIABLE && pVar->iColumn==p_கோவை->iColumn ){
    திரும்பு 0;
  }
  எனில்( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) திரும்பு 2;
  sqlite3_மதிப்பு_From_கோவை(pParse->db, p_கோவை, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  எனில்( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_மதிப்பு(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    எனில்( pL ){
      எனில்( sqlite3_மதிப்பு_type(pL)==SQLITE_TEXT ){
        sqlite3_மதிப்பு_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_மதிப்பு_Free(pR);
    sqlite3_மதிப்பு_Free(pL);
  }
  திரும்பு res;
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
முழு sqlite3_கோவை_Compare(
  மாறிலி Parse *pParse,
  மாறிலி கோவை *pA,
  மாறிலி கோவை *pB,
  முழு iTab
){
  u32 combinedFlags;
  எனில்( pA==0 || pB==0 ){
    திரும்பு pB==pA ? 0 : 2;
  }
  எனில்( pParse && pA->op==TK_VARIABLE ){
    திரும்பு கோவை_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  எனில்( combinedFlags & EP_Int_மதிப்பு ){
    எனில்( (pA->flags&pB->flags&EP_Int_மதிப்பு)!=0 && pA->u.i_மதிப்பு==pB->u.i_மதிப்பு ){
      திரும்பு 0;
    }
    திரும்பு 2;
  }
  எனில்( pA->op!=pB->op || pA->op==TK_RAISE ){
    எனில்( pA->op==TK_COLLATE && sqlite3_கோவை_Compare(pParse, pA->p_இடது,pB,iTab)<2 ){
      திரும்பு 1;
    }
    எனில்( pB->op==TK_COLLATE && sqlite3_கோவை_Compare(pParse, pA,pB->p_இடது,iTab)<2 ){
      திரும்பு 1;
    }
    எனில்( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }இல்லையெனில்{
      திரும்பு 2;
    }
  }
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(pA, EP_Int_மதிப்பு) );
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(pB, EP_Int_மதிப்பு) );
  எனில்( pA->u.zToken ){
    எனில்( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      எனில்( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) திரும்பு 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      ப்ரதிஜ்ஞா( pA->op==pB->op );
      எனில்( கோவை_Has_Property(pA,EP_WinFunc)!=கோவை_Has_Property(pB,EP_WinFunc) ){
        திரும்பு 2;
      }
      எனில்( கோவை_Has_Property(pA,EP_WinFunc) ){
        எனில்( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          திரும்பு 2;
        }
      }
#endif
    }இல்லையெனில் எனில்( pA->op==TK_NULL ){
      திரும்பு 0;
    }இல்லையெனில் எனில்( pA->op==TK_COLLATE ){
      எனில்( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) திரும்பு 2;
    }இல்லையெனில்
    எனில்( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      திரும்பு 2;
    }
  }
  எனில்( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) திரும்பு 2;
  எனில்( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    எனில்( combinedFlags & EP_xIsSelect ) திரும்பு 2;
    எனில்( (combinedFlags & EP_FixedCol)==0
     && sqlite3_கோவை_Compare(pParse, pA->p_இடது, pB->p_இடது, iTab) ) திரும்பு 2;
    எனில்( sqlite3_கோவை_Compare(pParse, pA->p_வலது, pB->p_வலது, iTab) ) திரும்பு 2;
    எனில்( sqlite3_கோவை_List_Compare(pA->x.pList, pB->x.pList, iTab) ) திரும்பு 2;
    எனில்( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      எனில்( pA->iColumn!=pB->iColumn ) திரும்பு 2;
      எனில்( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) திரும்பு 2;
      எனில்( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        திரும்பு 2;
      }
    }
  }
  திரும்பு 0;
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
முழு sqlite3_கோவை_List_Compare(மாறிலி கோவை_List *pA, மாறிலி கோவை_List *pB, முழு iTab){
  முழு i;
  எனில்( pA==0 && pB==0 ) திரும்பு 0;
  எனில்( pA==0 || pB==0 ) திரும்பு 1;
  எனில்( pA->n_கோவை!=pB->n_கோவை ) திரும்பு 1;
  க்கு(i=0; i<pA->n_கோவை; i++){
    முழு res;
    கோவை *p_கோவை_A = pA->a[i].p_கோவை;
    கோவை *p_கோவை_B = pB->a[i].p_கோவை;
    எனில்( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) திரும்பு 1;
    எனில்( (res = sqlite3_கோவை_Compare(0, p_கோவை_A, p_கோவை_B, iTab)) ) திரும்பு res;
  }
  திரும்பு 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
முழு sqlite3_கோவை_Compare_Skip(கோவை *pA,கோவை *pB, முழு iTab){
  திரும்பு sqlite3_கோவை_Compare(0,
             sqlite3_கோவை_Skip_Collate(pA),
             sqlite3_கோவை_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
நிலையான முழு கோவை_Implies_Not_Null(
  மாறிலி Parse *pParse,/* Parsing context */
  மாறிலி கோவை *p,      /* The expression to be checked */
  மாறிலி கோவை *pNN,    /* The expression that is NOT NULL */
  முழு iTab,           /* Table being evaluated */
  முழு seenNot         /* Return true only if p can be any non-NULL value */
){
  ப்ரதிஜ்ஞா( p );
  ப்ரதிஜ்ஞா( pNN );
  எனில்( sqlite3_கோவை_Compare(pParse, p, pNN, iTab)==0 ){
    திரும்பு pNN->op!=TK_NULL;
  }
  தேர்வு( p->op ){
    நிலை TK_IN: {
      எனில்( seenNot && கோவை_Has_Property(p, EP_xIsSelect) ) திரும்பு 0;
      ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_கோவை>0) );
      திரும்பு கோவை_Implies_Not_Null(pParse, p->p_இடது, pNN, iTab, 1);
    }
    நிலை TK_BETWEEN: {
      கோவை_List *pList;
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p) );
      pList = p->x.pList;
      ப்ரதிஜ்ஞா( pList!=0 );
      ப்ரதிஜ்ஞா( pList->n_கோவை==2 );
      எனில்( seenNot ) திரும்பு 0;
      எனில்( கோவை_Implies_Not_Null(pParse, pList->a[0].p_கோவை, pNN, iTab, 1)
       || கோவை_Implies_Not_Null(pParse, pList->a[1].p_கோவை, pNN, iTab, 1)
      ){
        திரும்பு 1;
      }
      திரும்பு கோவை_Implies_Not_Null(pParse, p->p_இடது, pNN, iTab, 1);
    }
    நிலை TK_EQ:
    நிலை TK_NE:
    நிலை TK_LT:
    நிலை TK_LE:
    நிலை TK_GT:
    நிலை TK_GE:
    நிலை TK_PLUS:
    நிலை TK_MINUS:
    நிலை TK_BITOR:
    நிலை TK_LSHIFT:
    நிலை TK_RSHIFT:
    நிலை TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    நிலை TK_STAR:
    நிலை TK_REM:
    நிலை TK_BITAND:
    நிலை TK_SLASH: {
      எனில்( கோவை_Implies_Not_Null(pParse, p->p_வலது, pNN, iTab, seenNot) ) திரும்பு 1;
      /* no break */ deliberate_fall_through
    }
    நிலை TK_SPAN:
    நிலை TK_COLLATE:
    நிலை TK_UPLUS:
    நிலை TK_UMINUS: {
      திரும்பு கோவை_Implies_Not_Null(pParse, p->p_இடது, pNN, iTab, seenNot);
    }
    நிலை TK_TRUTH: {
      எனில்( seenNot ) திரும்பு 0;
      எனில்( p->op2!=TK_IS ) திரும்பு 0;
      திரும்பு கோவை_Implies_Not_Null(pParse, p->p_இடது, pNN, iTab, 1);
    }
    நிலை TK_BITNOT:
    நிலை TK_NOT: {
      திரும்பு கோவை_Implies_Not_Null(pParse, p->p_இடது, pNN, iTab, 1);
    }
  }
  திரும்பு 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
நிலையான முழு sqlite3_கோவை_Is_Not_True(கோவை *p_கோவை){
  முழு v;
  எனில்( p_கோவை->op==TK_NULL ) திரும்பு 1;
  எனில்( p_கோவை->op==TK_TRUEFALSE && sqlite3_கோவை_Truth_மதிப்பு(p_கோவை)==0 ) திரும்பு 1;
  v = 1;
  எனில்( sqlite3_கோவை_Is_Integer(p_கோவை, &v, 0, 0) && v==0 ) திரும்பு 1;
  திரும்பு 0;
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
நிலையான முழு sqlite3_கோவை_Is_IIF(sqlite3 *db, மாறிலி கோவை *p_கோவை){
  கோவை_List *pList;
  எனில்( p_கோவை->op==TK_FUNCTION ){
    மாறிலி எழுத்து *z = p_கோவை->u.zToken;
    FuncDef *pDef;
    எனில்( (z[0]!='i' && z[0]!='I') ) திரும்பு 0;
    எனில்( p_கோவை->x.pList==0 ) திரும்பு 0;
    pDef = sqlite3FindFunction(db, z, p_கோவை->x.pList->n_கோவை, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    எனில்( pDef==0 ) திரும்பு 0;
#இல்லையெனில்
    எனில்( NEVER(pDef==0) ) திரும்பு 0;
#endif
    எனில்( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) திரும்பு 0;
    எனில்( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) திரும்பு 0;
  }இல்லையெனில் எனில்( p_கோவை->op==TK_CASE ){
    எனில்( p_கோவை->p_இடது!=0 ) திரும்பு 0;
  }இல்லையெனில்{
    திரும்பு 0;
  }
  pList = p_கோவை->x.pList;
  ப்ரதிஜ்ஞா( pList!=0 );
  எனில்( pList->n_கோவை==2 ) திரும்பு 1;
  எனில்( pList->n_கோவை==3 && sqlite3_கோவை_Is_Not_True(pList->a[2].p_கோவை) ) திரும்பு 1;
  திரும்பு 0;
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
முழு sqlite3_கோவை_Implies_கோவை(
  மாறிலி Parse *pParse,
  மாறிலி கோவை *pE1,
  மாறிலி கோவை *pE2,
  முழு iTab
){
  எனில்( sqlite3_கோவை_Compare(pParse, pE1, pE2, iTab)==0 ){
    திரும்பு 1;
  }
  எனில்( pE2->op==TK_OR
   && (sqlite3_கோவை_Implies_கோவை(pParse, pE1, pE2->p_இடது, iTab)
             || sqlite3_கோவை_Implies_கோவை(pParse, pE1, pE2->p_வலது, iTab) )
  ){
    திரும்பு 1;
  }
  எனில்( pE2->op==TK_NOTNULL
   && கோவை_Implies_Not_Null(pParse, pE1, pE2->p_இடது, iTab, 0)
  ){
    திரும்பு 1;
  }
  எனில்( sqlite3_கோவை_Is_IIF(pParse->db, pE1) ){
    திரும்பு sqlite3_கோவை_Implies_கோவை(pParse,pE1->x.pList->a[0].p_கோவை,pE2,iTab);
  }
  திரும்பு 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
நிலையான வெறுமை bothImplyNotNullRow(Walker *pWalker, கோவை *pE1, கோவை *pE2){
  எனில்( pWalker->eCode==0 ){
    sqlite3_Walk_கோவை(pWalker, pE1);
    எனில்( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_கோவை(pWalker, pE2);
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
நிலையான முழு impliesNotNullRow(Walker *pWalker, கோவை *p_கோவை){
  testcase( p_கோவை->op==TK_AGG_COLUMN );
  testcase( p_கோவை->op==TK_AGG_FUNCTION );
  எனில்( கோவை_Has_Property(p_கோவை, EP_OuterON) ) திரும்பு WRC_Prune;
  எனில்( கோவை_Has_Property(p_கோவை, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    திரும்பு WRC_Prune;
  }
  தேர்வு( p_கோவை->op ){
    நிலை TK_ISNOT:
    நிலை TK_ISNULL:
    நிலை TK_NOTNULL:
    நிலை TK_IS:
    நிலை TK_VECTOR:
    நிலை TK_FUNCTION:
    நிலை TK_TRUTH:
    நிலை TK_CASE:
      testcase( p_கோவை->op==TK_ISNOT );
      testcase( p_கோவை->op==TK_ISNULL );
      testcase( p_கோவை->op==TK_NOTNULL );
      testcase( p_கோவை->op==TK_IS );
      testcase( p_கோவை->op==TK_VECTOR );
      testcase( p_கோவை->op==TK_FUNCTION );
      testcase( p_கோவை->op==TK_TRUTH );
      testcase( p_கோவை->op==TK_CASE );
      திரும்பு WRC_Prune;

    நிலை TK_COLUMN:
      எனில்( pWalker->u.iCur==p_கோவை->iTable ){
        pWalker->eCode = 1;
        திரும்பு WRC_Abort;
      }
      திரும்பு WRC_Prune;

    நிலை TK_OR:
    நிலை TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_கோவை->op==TK_OR );
      testcase( p_கோவை->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_கோவை->p_இடது, p_கோவை->p_வலது);
      திரும்பு WRC_Prune;
       
    நிலை TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      எனில்( கோவை_Use_XList(p_கோவை) && ALWAYS(p_கோவை->x.pList->n_கோவை>0) ){
        sqlite3_Walk_கோவை(pWalker, p_கோவை->p_இடது);
      }
      திரும்பு WRC_Prune;

    நிலை TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
      ப்ரதிஜ்ஞா( p_கோவை->x.pList->n_கோவை==2 );
      sqlite3_Walk_கோவை(pWalker, p_கோவை->p_இடது);
      bothImplyNotNullRow(pWalker, p_கோவை->x.pList->a[0].p_கோவை,
                                   p_கோவை->x.pList->a[1].p_கோவை);
      திரும்பு WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    நிலை TK_EQ:
    நிலை TK_NE:
    நிலை TK_LT:
    நிலை TK_LE:
    நிலை TK_GT:
    நிலை TK_GE: {
      கோவை *p_இடது = p_கோவை->p_இடது;
      கோவை *p_வலது = p_கோவை->p_வலது;
      testcase( p_கோவை->op==TK_EQ );
      testcase( p_கோவை->op==TK_NE );
      testcase( p_கோவை->op==TK_LT );
      testcase( p_கோவை->op==TK_LE );
      testcase( p_கோவை->op==TK_GT );
      testcase( p_கோவை->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      ப்ரதிஜ்ஞா( p_இடது->op!=TK_COLUMN || கோவை_Use_YTab(p_இடது) );
      ப்ரதிஜ்ஞா( p_வலது->op!=TK_COLUMN || கோவை_Use_YTab(p_வலது) );
      எனில்( (p_இடது->op==TK_COLUMN
           && ALWAYS(p_இடது->y.pTab!=0)
           && IsVirtual(p_இடது->y.pTab))
       || (p_வலது->op==TK_COLUMN
           && ALWAYS(p_வலது->y.pTab!=0)
           && IsVirtual(p_வலது->y.pTab))
      ){
        திரும்பு WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    இயல்பு:
      திரும்பு WRC_Continue;
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
முழு sqlite3_கோவை_Implies_Non_Null_Row(கோவை *p, முழு iTab, முழு isRJ){
  Walker w;
  p = sqlite3_கோவை_Skip_Collate_And_Likely(p);
  எனில்( p==0 ) திரும்பு 0;
  எனில்( p->op==TK_NOTNULL ){
    p = p->p_இடது;
  }இல்லையெனில்{
    போது( p->op==TK_AND ){
      எனில்( sqlite3_கோவை_Implies_Non_Null_Row(p->p_இடது, iTab, isRJ) ) திரும்பு 1;
      p = p->p_வலது;
    }
  }
  w.x_கோவை_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_கோவை(&w, p);
  திரும்பு w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
கட்டமைப்பு IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  முழு iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
நிலையான முழு கோவை_Idx_Cover(Walker *pWalker, கோவை *p_கோவை){
  எனில்( p_கோவை->op==TK_COLUMN
   && p_கோவை->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_கோவை->iColumn)<0
  ){
    pWalker->eCode = 1;
    திரும்பு WRC_Abort;
  }
  திரும்பு WRC_Continue;
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
முழு sqlite3_கோவை_Covered_By_Index(
  கோவை *p_கோவை,        /* The index to be tested */
  முழு iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  கட்டமைப்பு IdxCover xcov;
  memset(&w, 0, அளவு(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_கோவை_Callback = கோவை_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_கோவை(&w, p_கோவை);
  திரும்பு !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
கட்டமைப்பு RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  முழு *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
நிலையான முழு selectRefEnter(Walker *pWalker, Select *pSelect){
  கட்டமைப்பு RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  முழு *piNew;
  எனில்( pSrc->nSrc==0 ) திரும்பு WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*அளவு(முழு));
  எனில்( piNew==0 ){
    p->nExclude = 0;
    திரும்பு WRC_Abort;
  }இல்லையெனில்{
    p->aiExclude = piNew;
  }
  க்கு(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  திரும்பு WRC_Continue;
}
நிலையான வெறுமை selectRefLeave(Walker *pWalker, Select *pSelect){
  கட்டமைப்பு RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  எனில்( p->nExclude ){
    ப்ரதிஜ்ஞா( p->nExclude>=pSrc->nSrc );
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
நிலையான முழு கோவை_Ref_To_Src_List(Walker *pWalker, கோவை *p_கோவை){
  எனில்( p_கோவை->op==TK_COLUMN
   || p_கோவை->op==TK_AGG_COLUMN
  ){
    முழு i;
    கட்டமைப்பு RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    முழு nSrc = pSrc ? pSrc->nSrc : 0;
    க்கு(i=0; i<nSrc; i++){
      எனில்( p_கோவை->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        திரும்பு WRC_Continue;
      }
    }
    க்கு(i=0; i<p->nExclude && p->aiExclude[i]!=p_கோவை->iTable; i++){}
    எனில்( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  திரும்பு WRC_Continue;
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
முழு sqlite3ReferencesSrcList(Parse *pParse, கோவை *p_கோவை, SrcList *pSrcList){
  Walker w;
  கட்டமைப்பு RefSrcList x;
  ப்ரதிஜ்ஞா( pParse->db!=0 );
  memset(&w, 0, அளவு(w));
  memset(&x, 0, அளவு(x));
  w.x_கோவை_Callback = கோவை_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_AGG_FUNCTION );
  ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
  sqlite3_Walk_கோவை_List(&w, p_கோவை->x.pList);
  எனில்( p_கோவை->p_இடது ){
    ப்ரதிஜ்ஞா( p_கோவை->p_இடது->op==TK_ORDER );
    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை->p_இடது) );
    ப்ரதிஜ்ஞா( p_கோவை->p_இடது->x.pList!=0 );
    sqlite3_Walk_கோவை_List(&w, p_கோவை->p_இடது->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  எனில்( கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
    sqlite3_Walk_கோவை(&w, p_கோவை->y.pWin->pFilter);
  }
#endif
  எனில்( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  எனில்( w.eCode & 0x01 ){
    திரும்பு 1;
  }இல்லையெனில் எனில்( w.eCode ){
    திரும்பு 0;
  }இல்லையெனில்{
    திரும்பு -1;
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
நிலையான முழு agginfo_Persist_கோவை_Cb(Walker *pWalker, கோவை *p_கோவை){
  எனில்( ALWAYS(!கோவை_Has_Property(p_கோவை, EP_TokenOnly|EP_Reduced))
   && p_கோவை->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_கோவை->pAggInfo;
    முழு iAgg = p_கோவை->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    ப்ரதிஜ்ஞா( iAgg>=0 );
    எனில்( p_கோவை->op!=TK_AGG_FUNCTION ){
      எனில்( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_கோவை
      ){
        p_கோவை = sqlite3_கோவை_Dup(db, p_கோவை, 0);
        எனில்( p_கோவை && !sqlite3_கோவை_Deferred_Delete(pParse, p_கோவை) ){
          pAggInfo->aCol[iAgg].pCExpr = p_கோவை;
        }
      }
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( p_கோவை->op==TK_AGG_FUNCTION );
      எனில்( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_கோவை
      ){
        p_கோவை = sqlite3_கோவை_Dup(db, p_கோவை, 0);
        எனில்( p_கோவை && !sqlite3_கோவை_Deferred_Delete(pParse, p_கோவை) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_கோவை;
        }
      }
    }
  }
  திரும்பு WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
வெறுமை sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, அளவு(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_கோவை_Callback = agginfo_Persist_கோவை_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
நிலையான முழு addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  முழு i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       அளவு(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  திரும்பு i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
நிலையான முழு addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  முழு i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       அளவு(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  திரும்பு i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
நிலையான வெறுமை findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  கோவை *p_கோவை          /* Expr describing the column to find or insert */
){
  கட்டமைப்பு AggInfo_col *pCol;
  முழு k;
  முழு mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  ப்ரதிஜ்ஞா( mxTerm <= SMXV(i16) );
  ப்ரதிஜ்ஞா( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  க்கு(k=0; k<pAggInfo->nColumn; k++, pCol++){
    எனில்( pCol->pCExpr==p_கோவை ) திரும்பு;
    எனில்( pCol->iTable==p_கோவை->iTable
     && pCol->iColumn==p_கோவை->iColumn
     && p_கோவை->op!=TK_IF_NULL_ROW
    ){
      செல் fix_up_கோவை;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  எனில்( k<0 ){
    /* OOM on resize */
    ப்ரதிஜ்ஞா( pParse->db->mallocFailed );
    திரும்பு;
  }
  எனில்( k>mxTerm ){
    sqlite3_Error_செய்தி(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
  pCol->pTab = p_கோவை->y.pTab;
  pCol->iTable = p_கோவை->iTable;
  pCol->iColumn = p_கோவை->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_கோவை;
  எனில்( pAggInfo->pGroupBy && p_கோவை->op!=TK_IF_NULL_ROW ){
    முழு j, n;
    கோவை_List *pGB = pAggInfo->pGroupBy;
    கட்டமைப்பு கோவை_List_item *pTerm = pGB->a;
    n = pGB->n_கோவை;
    க்கு(j=0; j<n; j++, pTerm++){
      கோவை *pE = pTerm->p_கோவை;
      எனில்( pE->op==TK_COLUMN
       && pE->iTable==p_கோவை->iTable
       && pE->iColumn==p_கோவை->iColumn
      ){
        pCol->iSorterColumn = j;
        நிறுத்து;
      }
    }
  }
  எனில்( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_கோவை:
  கோவை_Set_VVAProperty(p_கோவை, EP_NoReduce);
  ப்ரதிஜ்ஞா( p_கோவை->pAggInfo==0 || p_கோவை->pAggInfo==pAggInfo );
  p_கோவை->pAggInfo = pAggInfo;
  எனில்( p_கோவை->op==TK_COLUMN ){
    p_கோவை->op = TK_AGG_COLUMN;
  }
  ப்ரதிஜ்ஞா( k <= SMXV(p_கோவை->iAgg) );
  p_கோவை->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
நிலையான முழு analyzeAggregate(Walker *pWalker, கோவை *p_கோவை){
  முழு i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  ப்ரதிஜ்ஞா( pNC->ncFlags & NC_UAggInfo );
  ப்ரதிஜ்ஞா( pAggInfo->iFirstReg==0 );
  தேர்வு( p_கோவை->op ){
    இயல்பு: {
      Indexed_கோவை *pIEpr;
      கோவை tmp;
      ப்ரதிஜ்ஞா( pParse->iSelfTab==0 );
      எனில்( (pNC->ncFlags & NC_InAggFunc)==0 ) நிறுத்து;
      எனில்( pParse->pIdxEpr==0 ) நிறுத்து;
      க்கு(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        முழு iDataCur = pIEpr->iDataCur;
        எனில்( iDataCur<0 ) தொடர்;
        எனில்( sqlite3_கோவை_Compare(0, p_கோவை, pIEpr->p_கோவை, iDataCur)==0 ) நிறுத்து;
      }
      எனில்( pIEpr==0 ) நிறுத்து;
      எனில்( NEVER(!கோவை_Use_YTab(p_கோவை)) ) நிறுத்து;
      க்கு(i=0; i<pSrcList->nSrc; i++){
         எனில்( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           நிறுத்து;
         }
      }
      எனில்( i>=pSrcList->nSrc ) நிறுத்து;
      எனில்( NEVER(p_கோவை->pAggInfo!=0) ) நிறுத்து; /* Resolved by outer context */
      எனில்( pParse->nErr ){ திரும்பு WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, அளவு(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      எனில்( pParse->nErr ){ திரும்பு WRC_Abort; }
      ப்ரதிஜ்ஞா( pAggInfo->aCol!=0 );
      ப்ரதிஜ்ஞா( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_கோவை;
      p_கோவை->pAggInfo = pAggInfo;
      p_கோவை->iAgg = tmp.iAgg;
      திரும்பு WRC_Prune;
    }
    நிலை TK_IF_NULL_ROW:
    நிலை TK_AGG_COLUMN:
    நிலை TK_COLUMN: {
      testcase( p_கோவை->op==TK_AGG_COLUMN );
      testcase( p_கோவை->op==TK_COLUMN );
      testcase( p_கோவை->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      எனில்( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        க்கு(i=0; i<pSrcList->nSrc; i++, pItem++){
          ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_TokenOnly|EP_Reduced) );
          எனில்( p_கோவை->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_கோவை);
            நிறுத்து;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      திரும்பு WRC_Continue;
    }
    நிலை TK_AGG_FUNCTION: {
      எனில்( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_கோவை->op2
       && p_கோவை->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        கட்டமைப்பு AggInfo_func *pItem = pAggInfo->aFunc;
        முழு mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        ப்ரதிஜ்ஞா( mxTerm <= SMXV(i16) );
        க்கு(i=0; i<pAggInfo->nFunc; i++, pItem++){
          எனில்( NEVER(pItem->pFExpr==p_கோவை) ) நிறுத்து;
          எனில்( sqlite3_கோவை_Compare(0, pItem->pFExpr, p_கோவை, -1)==0 ){
            நிறுத்து;
          }
        }
        எனில்( i>mxTerm ){
          sqlite3_Error_செய்தி(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          ப்ரதிஜ்ஞா( i<pAggInfo->nFunc );
        }இல்லையெனில் எனில்( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          எனில்( i>=0 ){
            முழு nArg;
            ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_கோவை;
            ப்ரதிஜ்ஞா( கோவை_Use_UToken(p_கோவை) );
            nArg = p_கோவை->x.pList ? p_கோவை->x.pList->n_கோவை : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_கோவை->u.zToken, nArg, enc, 0);
            ப்ரதிஜ்ஞா( pItem->bOBUnique==0 );
            எனில்( p_கோவை->p_இடது
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              கோவை_List *pOBList;
              ப்ரதிஜ்ஞா( nArg>0 );
              ப்ரதிஜ்ஞா( p_கோவை->p_இடது->op==TK_ORDER );
              ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை->p_இடது) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_கோவை->p_இடது->x.pList;
              ப்ரதிஜ்ஞா( pOBList->n_கோவை>0 );
              ப்ரதிஜ்ஞா( pItem->bOBUnique==0 );
              எனில்( pOBList->n_கோவை==1
               && nArg==1
               && sqlite3_கோவை_Compare(0,pOBList->a[0].p_கோவை,
                               p_கோவை->x.pList->a[0].p_கோவை,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = கோவை_Has_Property(p_கோவை, EP_Distinct);
              }இல்லையெனில்{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }இல்லையெனில்{
              pItem->iOBTab = -1;
            }
            எனில்( கோவை_Has_Property(p_கோவை, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }இல்லையெனில்{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_TokenOnly|EP_Reduced) );
        கோவை_Set_VVAProperty(p_கோவை, EP_NoReduce);
        ப்ரதிஜ்ஞா( i <= SMXV(p_கோவை->iAgg) );
        p_கோவை->iAgg = (i16)i;
        p_கோவை->pAggInfo = pAggInfo;
        திரும்பு WRC_Prune;
      }இல்லையெனில்{
        திரும்பு WRC_Continue;
      }
    }
  }
  திரும்பு WRC_Continue;
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
வெறுமை sqlite3_கோவை_Analyze_Aggregates(NameContext *pNC, கோவை *p_கோவை){
  Walker w;
  w.x_கோவை_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  ப்ரதிஜ்ஞா( pNC->pSrcList!=0 );
  sqlite3_Walk_கோவை(&w, p_கோவை);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
வெறுமை sqlite3_கோவை_Analyze_Agg_List(NameContext *pNC, கோவை_List *pList){
  கட்டமைப்பு கோவை_List_item *pItem;
  முழு i;
  எனில்( pList ){
    க்கு(pItem=pList->a, i=0; i<pList->n_கோவை; i++, pItem++){
      sqlite3_கோவை_Analyze_Aggregates(pNC, pItem->p_கோவை);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
முழு sqlite3GetTempReg(Parse *pParse){
  எனில்( pParse->nTempReg==0 ){
    திரும்பு ++pParse->nMem;
  }
  திரும்பு pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
வெறுமை sqlite3ReleaseTempReg(Parse *pParse, முழு iReg){
  எனில்( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    முழு kk;
    க்கு(kk=0; kk<pParse->nTempReg; kk++){
      ப்ரதிஜ்ஞா( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    எனில்( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
முழு sqlite3GetTempRange(Parse *pParse, முழு nReg){
  முழு i, n;
  எனில்( nReg==1 ) திரும்பு sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  எனில்( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }இல்லையெனில்{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  திரும்பு i;
}
வெறுமை sqlite3ReleaseTempRange(Parse *pParse, முழு iReg, முழு nReg){
  எனில்( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    திரும்பு;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  எனில்( nReg>pParse->nRangeReg ){
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
வெறுமை sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
வெறுமை sqlite3TouchRegister(Parse *pParse, முழு iReg){
  எனில்( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#எனில் defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
முழு sqlite3FirstAvailableRegister(Parse *pParse, முழு iMin){
  மாறிலி கோவை_List *pList = pParse->p_Const_கோவை;
  எனில்( pList ){
    முழு i;
    க்கு(i=0; i<pList->n_கோவை; i++){
      எனில்( pList->a[i].u.i_Const_கோவை_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_கோவை_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  திரும்பு iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
முழு sqlite3NoTempsInRange(Parse *pParse, முழு iFirst, முழு iLast){
  முழு i;
  எனில்( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     திரும்பு 0;
  }
  க்கு(i=0; i<pParse->nTempReg; i++){
    எனில்( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      திரும்பு 0;
    }
  }
  எனில்( pParse->p_Const_கோவை ){
    கோவை_List *pList = pParse->p_Const_கோவை;
    க்கு(i=0; i<pList->n_கோவை; i++){
      முழு iReg = pList->a[i].u.i_Const_கோவை_Reg;
      எனில்( iReg==0 ) தொடர்;
      எனில்( iReg>=iFirst && iReg<=iLast ) திரும்பு 0;
    }
  }
  திரும்பு 1;
}
#endif /* SQLITE_DEBUG */
