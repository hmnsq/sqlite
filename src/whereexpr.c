/*
** 2015-06-08
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This module contains C code that generates VDBE code used to process
** the WHERE clause of SQL statements.
**
** This file was originally part of where.c but was split out to improve
** readability and editability.  This file contains utility routines for
** analyzing Expr objects in the WHERE clause.
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "whereInt.h"

/* Forward declarations */
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_Analyze(SrcList*, WhereClause*, ಪೂರ್ಣಾಙ್ಕ);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Add a single new WhereTerm entry to the WhereClause object pWC.
** The new WhereTerm object is constructed from Expr p and with wtFlags.
** The index in pWC->a[] of the new WhereTerm is returned on success.
** 0 is returned if the new WhereTerm could not be added due to a memory
** allocation error.  The memory allocation failure will be recorded in
** the db->mallocFailed flag so that higher-level functions can detect it.
**
** This routine will increase the size of the pWC->a[] array as necessary.
**
** If the wtFlags argument includes TERM_DYNAMIC, then responsibility
** for freeing the expression p is assumed by the WhereClause object pWC.
** This is true even if this routine fails to allocate a new WhereTerm.
**
** WARNING:  This routine might reallocate the space used to store
** WhereTerms.  All pointers to WhereTerms should be invalidated after
** calling this routine.  Such pointers may be reinitialized by referencing
** the pWC->a[] array.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ where_Clause_ಸೇರಿಸು(WhereClause *pWC, ಅಭಿವ್ಯಕ್ತಿ *p, u16 wtFlags){
  WhereTerm *pTerm;
  ಪೂರ್ಣಾಙ್ಕ idx;
  testcase( wtFlags & TERM_VIRTUAL );
  ಯದಿ( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, ಪರಿಮಾಣ(pWC->a[0])*pWC->nSlot*2 );
    ಯದಿ( pWC->a==0 ){
      ಯದಿ( wtFlags & TERM_DYNAMIC ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p);
      }
      pWC->a = pOld;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    memcpy(pWC->a, pOld, ಪರಿಮಾಣ(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  ಯದಿ( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  ಯದಿ( p && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }ಅನ್ಯಥಾ{
    pTerm->truthProb = 1;
  }
  pTerm->p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         ಪರಿಮಾಣ(WhereTerm) - offsetof(WhereTerm,eOperator));
  ಪ್ರತ್ಯಾಗಮ idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ allowedOp(ಪೂರ್ಣಾಙ್ಕ op){
  ಪ್ರತಿಜ್ಞಾ( TK_GT>TK_EQ && TK_GT<TK_GE );
  ಪ್ರತಿಜ್ಞಾ( TK_LT>TK_EQ && TK_LT<TK_GE );
  ಪ್ರತಿಜ್ಞಾ( TK_LE>TK_EQ && TK_LE<TK_GE );
  ಪ್ರತಿಜ್ಞಾ( TK_GE==TK_EQ+4 );
  ಪ್ರತಿಜ್ಞಾ( TK_IN<TK_EQ );
  ಪ್ರತಿಜ್ಞಾ( TK_IS<TK_EQ );
  ಪ್ರತಿಜ್ಞಾ( TK_ISNULL<TK_EQ );
  ಯದಿ( op>TK_GE ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( op>=TK_EQ ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತ್ಯಾಗಮ op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ಸ್ಥಿರ u16 ಅಭಿವ್ಯಕ್ತಿ_Commute(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_VECTOR
   || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ->op==TK_VECTOR
   || sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ) !=
      sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ)
  ){
    p_ಅಭಿವ್ಯಕ್ತಿ->flags ^= EP_Commuted;
  }
  SWAP(ಅಭಿವ್ಯಕ್ತಿ*,p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ,p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op>=TK_GT ){
    ಪ್ರತಿಜ್ಞಾ( TK_LT==TK_GT+2 );
    ಪ್ರತಿಜ್ಞಾ( TK_GE==TK_LE+2 );
    ಪ್ರತಿಜ್ಞಾ( TK_GT>TK_EQ );
    ಪ್ರತಿಜ್ಞಾ( TK_GT<TK_LE );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op>=TK_GT && p_ಅಭಿವ್ಯಕ್ತಿ->op<=TK_GE );
    p_ಅಭಿವ್ಯಕ್ತಿ->op = ((p_ಅಭಿವ್ಯಕ್ತಿ->op-TK_GT)^2)+TK_GT;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ಸ್ಥಿರ u16 operatorMask(ಪೂರ್ಣಾಙ್ಕ op){
  u16 c;
  ಪ್ರತಿಜ್ಞಾ( allowedOp(op) );
  ಯದಿ( op>=TK_EQ ){
    ಪ್ರತಿಜ್ಞಾ( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }ಅನ್ಯಥಾ ಯದಿ( op==TK_IN ){
    c = WO_IN;
  }ಅನ್ಯಥಾ ಯದಿ( op==TK_ISNULL ){
    c = WO_ISNULL;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( op==TK_IS );
    c = WO_IS;
  }
  ಪ್ರತಿಜ್ಞಾ( op!=TK_ISNULL || c==WO_ISNULL );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_IN || c==WO_IN );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_EQ || c==WO_EQ );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_LT || c==WO_LT );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_LE || c==WO_LE );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_GT || c==WO_GT );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_GE || c==WO_GE );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_IS || c==WO_IS );
  ಪ್ರತ್ಯಾಗಮ c;
}


#ifndef SQLITE_OMIT_LIKE_OPTIMIZATION
/*
** Check to see if the given expression is a LIKE or GLOB operator that
** can be optimized using inequality constraints.  Return TRUE if it is
** so and false if not.
**
** In order for the operator to be optimizible, the RHS must be a string
** literal that does not begin with a wildcard.  The LHS must be a column
** that may only be NULL, a string, or a BLOB, never a number. (This means
** that virtual tables cannot participate in the LIKE optimization.)  The
** collating sequence for the column on the LHS must be appropriate for
** the operator.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,      /* Test this expression */
  ಅಭಿವ್ಯಕ್ತಿ **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  ಪೂರ್ಣಾಙ್ಕ *pisComplete, /* True if the only wildcard is % in the last character */
  ಪೂರ್ಣಾಙ್ಕ *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ಧ್ರುವ u8 *z = 0;           /* String on RHS of LIKE operator */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ, *p_ಎಡ;      /* Right and left size of LIKE operator */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  ಪೂರ್ಣಾಙ್ಕ cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_ಮೌಲ್ಯ *pVal = 0;
  ಪೂರ್ಣಾಙ್ಕ op;                    /* Opcode of pRight */
  ಪೂರ್ಣಾಙ್ಕ rc;                    /* Result code to return */

  ಯದಿ( !sqlite3IsLikeFunction(db, p_ಅಭಿವ್ಯಕ್ತಿ, pnoCase, (ವರ್ಣ*)wc) ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#ifdef SQLITE_EBCDIC
  ಯದಿ( *pnoCase ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
  p_ಎಡ = pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;

  p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
  op = p_ಬಲ->op;
  ಯದಿ( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    ಪೂರ್ಣಾಙ್ಕ iCol = p_ಬಲ->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_ಮೌಲ್ಯ(pReprepare, iCol, SQLITE_AFF_BLOB);
    ಯದಿ( pVal && sqlite3_ಮೌಲ್ಯ_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_ಮೌಲ್ಯ_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    ಪ್ರತಿಜ್ಞಾ( p_ಬಲ->op==TK_VARIABLE || p_ಬಲ->op==TK_REGISTER );
  }ಅನ್ಯಥಾ ಯದಿ( op==TK_STRING ){
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ, EP_Int_ಮೌಲ್ಯ) );
     z = (u8*)p_ಬಲ->u.zToken;
  }
  ಯದಿ( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    ಯಾವತ್( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      ಯದಿ( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }ಅನ್ಯಥಾ ಯದಿ( c>=0x80 ){
        ಧ್ರುವ u8 *z2 = z+cnt-1;
        ಯದಿ( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ{
          cnt = (ಪೂರ್ಣಾಙ್ಕ)(z2-z);
        }
      }
    }

    /* The optimization is possible only if (1) the pattern does not begin
    ** with a wildcard and if (2) the non-wildcard prefix does not end with
    ** an (illegal 0xff) character, or (3) the pattern does not consist of
    ** a single escape character. The second condition is necessary so
    ** that we can increment the prefix key to find an upper bound for the
    ** range search. The third is because the caller assumes that the pattern
    ** consists of at least one character after all escapes have been
    ** removed.  */
    ಯದಿ( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      ಅಭಿವ್ಯಕ್ತಿ *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_STRING, (ವರ್ಣ*)z);
      ಯದಿ( pPrefix ){
        ಪೂರ್ಣಾಙ್ಕ iFrom, iTo;
        ವರ್ಣ *zNew;
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pPrefix, EP_Int_ಮೌಲ್ಯ) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        ಪ್ರತಿ(iFrom=iTo=0; iFrom<cnt; iFrom++){
          ಯದಿ( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;
        ಪ್ರತಿಜ್ಞಾ( iTo>0 );

        /* If the LHS is not an ordinary column with TEXT affinity, then the
        ** pattern prefix boundaries (both the start and end boundaries) must
        ** not look like a number.  Otherwise the pattern might be treated as
        ** a number, which will invalidate the LIKE optimization.
        **
        ** Getting this right has been a persistent source of bugs in the
        ** LIKE optimization.  See, for example:
        **    2018-09-10 https://sqlite.org/src/info/c94369cae9b561b1
        **    2019-05-02 https://sqlite.org/src/info/b043a54c3de54b28
        **    2019-06-10 https://sqlite.org/src/info/fd76310a5e843e07
        **    2019-06-14 https://sqlite.org/src/info/ce8717f0885af975
        **    2019-09-03 https://sqlite.org/src/info/0f0428096f17252a
        */
        ಯದಿ( p_ಎಡ->op!=TK_COLUMN
         || sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಎಡ)!=SQLITE_AFF_TEXT
         || (ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಎಡ) )
             && ALWAYS(p_ಎಡ->y.pTab)
             && IsVirtual(p_ಎಡ->y.pTab))  /* Might be numeric */
        ){
          ಪೂರ್ಣಾಙ್ಕ is_ಅಂಕೆ;
          ದ್ವಿಗುಣ rDummy;
          ಪ್ರತಿಜ್ಞಾ( zNew[iTo]==0 );
          is_ಅಂಕೆ = sqlite3AtoF(zNew, &rDummy);
          ಯದಿ( is_ಅಂಕೆ<=0 ){
            ಯದಿ( iTo==1 && zNew[0]=='-' ){
              is_ಅಂಕೆ = +1;
            }ಅನ್ಯಥಾ{
              zNew[iTo-1]++;
              is_ಅಂಕೆ = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          ಯದಿ( is_ಅಂಕೆ>0 ){
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pPrefix);
            sqlite3_ಮೌಲ್ಯ_Free(pVal);
            ಪ್ರತ್ಯಾಗಮ 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      ಯದಿ( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_ಬಲ->iColumn, 0);
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ, EP_Int_ಮೌಲ್ಯ) );
        ಯದಿ( *pisComplete && p_ಬಲ->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempReg(pParse);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Target(pParse, p_ಬಲ, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }ಅನ್ಯಥಾ{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_ಮೌಲ್ಯ_Free(pVal);
  ಪ್ರತ್ಯಾಗಮ rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Like_Operator(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
    ಧ್ರುವ ವರ್ಣ *zOp;
    unsigned ವರ್ಣ eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
  ಪ್ರತಿ(i=0; i<ArraySize(aOp); i++){
    ಯದಿ( sqlite3StrICmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, aOp[i].zOp)==0 ){
      ಪ್ರತ್ಯಾಗಮ aOp[i].eOp;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Check to see if the pExpr expression is a form that needs to be passed
** to the xBestIndex method of virtual tables.  Forms of interest include:
**
**          Expression                   Virtual Table Operator
**          -----------------------      ---------------------------------
**      1.  column MATCH expr            SQLITE_INDEX_CONSTRAINT_MATCH
**      2.  column GLOB expr             SQLITE_INDEX_CONSTRAINT_GLOB
**      3.  column LIKE expr             SQLITE_INDEX_CONSTRAINT_LIKE
**      4.  column REGEXP expr           SQLITE_INDEX_CONSTRAINT_REGEXP
**      5.  column != expr               SQLITE_INDEX_CONSTRAINT_NE
**      6.  expr != column               SQLITE_INDEX_CONSTRAINT_NE
**      7.  column IS NOT expr           SQLITE_INDEX_CONSTRAINT_ISNOT
**      8.  expr IS NOT column           SQLITE_INDEX_CONSTRAINT_ISNOT
**      9.  column IS NOT NULL           SQLITE_INDEX_CONSTRAINT_ISNOTNULL
**
** In every case, "column" must be a column of a virtual table.  If there
** is a match, set *ppLeft to the "column" expression, set *ppRight to the
** "expr" expression (even though in forms (6) and (8) the column is on the
** right and the expression is on the left).  Also set *peOp2 to the
** appropriate virtual table operator.  The return value is 1 or 2 if there
** is a match.  The usual return is 1, but if the RHS is also a column
** of virtual table in forms (5) or (7) then return 2.
**
** If the expression matches none of the patterns above, return 0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,                    /* Test this expression */
  unsigned ವರ್ಣ *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  ಅಭಿವ್ಯಕ್ತಿ **pp_ಎಡ,                  /* Column expression to left of MATCH/op2 */
  ಅಭಿವ್ಯಕ್ತಿ **pp_ಬಲ                  /* Expression to left of MATCH/op2 */
){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಅಭಿವ್ಯಕ್ತಿ *pCol;                     /* Column reference */
    ಪೂರ್ಣಾಙ್ಕ i;

    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
    ಯದಿ( pList==0 || pList->n_ಅಭಿವ್ಯಕ್ತಿ!=2 ){
      ಪ್ರತ್ಯಾಗಮ 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( pCol->op!=TK_COLUMN || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(pCol) && pCol->y.pTab!=0) );
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(pCol) && (i = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Like_Operator(p_ಅಭಿವ್ಯಕ್ತಿ))!=0 ){
      *peOp2 = i;
      *pp_ಬಲ = pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      *pp_ಎಡ = pCol;
      ಪ್ರತ್ಯಾಗಮ 1;
    }

    /* We can also match against the first column of overloaded
    ** functions where xFindFunction returns a value of at least
    ** SQLITE_INDEX_CONSTRAINT_FUNCTION.
    **
    **      OVERLOADED(vtab_column,expression)
    **
    ** Historically, xFindFunction expected to see lower-case function
    ** names.  But for this use case, xFindFunction is expected to deal
    ** with function names in an arbitrary case.
    */
    pCol = pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( pCol->op!=TK_COLUMN || ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(pCol) );
    ಪ್ರತಿಜ್ಞಾ( pCol->op!=TK_COLUMN || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(pCol) && pCol->y.pTab!=0) );
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      ಶೂನ್ಯ (*xNotUsed)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**);
      ಶೂನ್ಯ *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      ಪ್ರತಿಜ್ಞಾ( pVtab!=0 );
      ಪ್ರತಿಜ್ಞಾ( pVtab->pModule!=0 );
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      pMod = (sqlite3_module *)pVtab->pModule;
      ಯದಿ( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, &xNotUsed, &pNotUsed);
        ಯದಿ( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_ಬಲ = pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
          *pp_ಎಡ = pCol;
          ಪ್ರತ್ಯಾಗಮ 1;
        }
      }
    }
  }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    ಪ್ರತಿಜ್ಞಾ( TK_NE < TK_EQ );
    ಪ್ರತಿಜ್ಞಾ( TK_ISNOT < TK_EQ );
    ಪ್ರತಿಜ್ಞಾ( TK_NOTNULL < TK_EQ );
    ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL ){
    ಪೂರ್ಣಾಙ್ಕ res = 0;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
    ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op!=TK_COLUMN || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಎಡ) && p_ಎಡ->y.pTab!=0) );
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(p_ಎಡ) ){
      res++;
    }
    ಪ್ರತಿಜ್ಞಾ( p_ಬಲ==0 || p_ಬಲ->op!=TK_COLUMN
            || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಬಲ) && p_ಬಲ->y.pTab!=0) );
    ಯದಿ( p_ಬಲ && ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(p_ಬಲ) ){
      res++;
      SWAP(ಅಭಿವ್ಯಕ್ತಿ*, p_ಎಡ, p_ಬಲ);
    }
    *pp_ಎಡ = p_ಎಡ;
    *pp_ಬಲ = p_ಬಲ;
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    ಪ್ರತ್ಯಾಗಮ res;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ಸ್ಥಿರ ಶೂನ್ಯ transferJoinMarkings(ಅಭಿವ್ಯಕ್ತಿ *pDerived, ಅಭಿವ್ಯಕ್ತಿ *pBase){
  ಯದಿ( pDerived && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ಸ್ಥಿರ ಶೂನ್ಯ markTermAsChild(WhereClause *pWC, ಪೂರ್ಣಾಙ್ಕ iChild, ಪೂರ್ಣಾಙ್ಕ iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  ಪ್ರತಿಜ್ಞಾ(   pWC->a[iParent].nChild <  UMXV(pWC->a[0].nChild) );
  pWC->a[iParent].nChild++;
  testcase( pWC->a[iParent].nChild == UMXV(pWC->a[0].nChild) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ಸ್ಥಿರ WhereTerm *whereNthSubterm(WhereTerm *pTerm, ಪೂರ್ಣಾಙ್ಕ N){
  ಯದಿ( pTerm->eOperator!=WO_AND ){
    ಪ್ರತ್ಯಾಗಮ N==0 ? pTerm : 0;
  }
  ಯದಿ( N<pTerm->u.pAndInfo->wc.nTerm ){
    ಪ್ರತ್ಯಾಗಮ &pTerm->u.pAndInfo->wc.a[N];
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Subterms pOne and pTwo are contained within WHERE clause pWC.  The
** two subterms are in disjunction - they are OR-ed together.
**
** If these two terms are both of the form:  "A op B" with the same
** A and B values but different operators and if the operators are
** compatible (if one is = and the other is <, for example) then
** add a new virtual AND term to pWC that is the combination of the
** two.
**
** Some examples:
**
**    x<y OR x=y    -->     x<=y
**    x=y OR x=y    -->     x=y
**    x<=y OR x<y   -->     x<=y
**
** The following is NOT generated:
**
**    x<y OR x>y    -->     x!=y    
*/
ಸ್ಥಿರ ಶೂನ್ಯ whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  ಅಭಿವ್ಯಕ್ತಿ *pNew;            /* New virtual expression */
  ಪೂರ್ಣಾಙ್ಕ op;                /* Operator for the combined expression */
  ಪೂರ್ಣಾಙ್ಕ idxNew;            /* Index in pWC of the next virtual term */
  ಅಭಿವ್ಯಕ್ತಿ *pA, *pB;         /* Expressions associated with pOne and pTwo */

  ಯದಿ( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) ಪ್ರತ್ಯಾಗಮ;
  pA = pOne->p_ಅಭಿವ್ಯಕ್ತಿ;
  pB = pTwo->p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( pA->p_ಎಡ!=0 && pA->p_ಬಲ!=0 );
  ಪ್ರತಿಜ್ಞಾ( pB->p_ಎಡ!=0 && pB->p_ಬಲ!=0 );
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pA->p_ಎಡ, pB->p_ಎಡ, -1) )  ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pA->p_ಬಲ, pB->p_ಬಲ,-1) ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pA,EP_Commuted)!=ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pB,EP_Commuted) ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  /* If we reach this point, it means the two subterms can be combined */
  ಯದಿ( (eOp & (eOp-1))!=0 ){
    ಯದಿ( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pA, 0);
  ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ ಪ್ರತಿಜ್ಞಾ( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_ಸೇರಿಸು(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
}

#ಯದಿ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Analyze a term that consists of two or more OR-connected
** subterms.  So in:
**
**     ... WHERE  (a=5) AND (b=7 OR c=9 OR d=13) AND (d=13)
**                          ^^^^^^^^^^^^^^^^^^^^
**
** This routine analyzes terms such as the middle term in the above example.
** A WhereOrTerm object is computed and attached to the term under
** analysis, regardless of the outcome of the analysis.  Hence:
**
**     WhereTerm.wtFlags   |=  TERM_ORINFO
**     WhereTerm.u.pOrInfo  =  a dynamically allocated WhereOrTerm object
**
** The term being analyzed must have two or more of OR-connected subterms.
** A single subterm might be a set of AND-connected sub-subterms.
** Examples of terms under analysis:
**
**     (A)     t1.x=t2.y OR t1.x=t2.z OR t1.y=15 OR t1.z=t3.a+5
**     (B)     x=expr1 OR expr2=x OR x=expr3
**     (C)     t1.x=t2.y OR (t1.x=t2.z AND t1.y=15)
**     (D)     x=expr1 OR (y>11 AND y<22 AND z LIKE '*hello*')
**     (E)     (p.a=1 AND q.b=2 AND r.c=3) OR (p.x=4 AND q.y=5 AND r.z=6)
**     (F)     x>A OR (x=A AND y>=B)
**
** CASE 1:
**
** If all subterms are of the form T.C=expr for some single column of C and
** a single table T (as shown in example B above) then create a new virtual
** term that is an equivalent IN expression.  In other words, if the term
** being analyzed is:
**
**      x = expr1  OR  expr2 = x  OR  x = expr3
**
** then create a new virtual term like this:
**
**      x IN (expr1,expr2,expr3)
**
** CASE 2:
**
** If there are exactly two disjuncts and one side has x>A and the other side
** has x=A (for the same x and A) then add a new virtual conjunct term to the
** WHERE clause of the form "x>=A".  Example:
**
**      x>A OR (x=A AND y>B)    adds:    x>=A
**
** The added conjunct can sometimes be helpful in query planning.
**
** CASE 3:
**
** If all subterms are indexable by a single table T, then set
**
**     WhereTerm.eOperator              =  WO_OR
**     WhereTerm.u.pOrInfo->indexable  |=  the cursor number for table T
**
** A subterm is "indexable" if it is of the form
** "T.C <op> <expr>" where C is any column of table T and
** <op> is one of "=", "<", "<=", ">", ">=", "IS NULL", or "IN".
** A subterm is also indexable if it is an AND of two or more
** subsubterms at least one of which is indexable.  Indexable AND
** subterms have their eOperator set to WO_AND and they have
** u.pAndInfo set to a dynamically allocated WhereAndTerm object.
**
** From another point of view, "indexable" means that the subterm could
** potentially be used with an index if an appropriate index exists.
** This analysis does not consider whether or not the index exists; that
** is decided elsewhere.  This analysis only looks at whether subterms
** appropriate for indexing exist.
**
** All examples A through E above satisfy case 3.  But if a term
** also satisfies case 1 (such as B) we know that the optimizer will
** always prefer case 1, so in that case we pretend that case 3 is not
** satisfied.
**
** It might be the case that multiple tables are indexable.  For example,
** (E) above is indexable on tables P, Q, and R.
**
** Terms that satisfy case 3 are candidates for lookup by using
** separate indices to find rowids for each subterm and composing
** the union of all rowids using a RowSet object.  This is similar
** to "bitmap indices" in other database engines.
**
** OTHERWISE:
**
** If none of cases 1, 2, or 3 apply, then leave the eOperator set to
** zero.  This term is not useful for search.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  ಪೂರ್ಣಾಙ್ಕ idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pTerm->p_ಅಭಿವ್ಯಕ್ತಿ;             /* The expression of the term */
  ಪೂರ್ಣಾಙ್ಕ i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  WhereTerm *pOrTerm;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chngToIN;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  ಪ್ರತಿಜ್ಞಾ( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(*pOrInfo));
  ಯದಿ( pOrInfo==0 ) ಪ್ರತ್ಯಾಗಮ;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, ಪರಿಮಾಣ(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_ಅಭಿವ್ಯಕ್ತಿ, TK_OR);
  sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pOrWc);
  ಯದಿ( db->mallocFailed ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿಜ್ಞಾ( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  ಪ್ರತಿ(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    ಯದಿ( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      ಪ್ರತಿಜ್ಞಾ( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, ಪರಿಮಾಣ(*pAndInfo));
      ಯದಿ( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        ಪೂರ್ಣಾಙ್ಕ j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->ಎಡ_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, ಪರಿಮಾಣ(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_ಅಭಿವ್ಯಕ್ತಿ, TK_AND);
        sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        ಯದಿ( !db->mallocFailed ){
          ಪ್ರತಿ(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            ಪ್ರತಿಜ್ಞಾ( pAndTerm->p_ಅಭಿವ್ಯಕ್ತಿ );
            ಯದಿ( allowedOp(pAndTerm->p_ಅಭಿವ್ಯಕ್ತಿ->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->ಎಡ_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }ಅನ್ಯಥಾ ಯದಿ( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }ಅನ್ಯಥಾ{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->ಎಡ_Cursor);
      ಯದಿ( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->ಎಡ_Cursor);
      }
      indexable &= b;
      ಯದಿ( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }ಅನ್ಯಥಾ{
        chngToIN &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  pTerm->eOperator = WO_OR;
  pTerm->ಎಡ_Cursor = -1;
  ಯದಿ( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  ಯದಿ( indexable && pOrWc->nTerm==2 ){
    ಪೂರ್ಣಾಙ್ಕ iOne = 0;
    WhereTerm *pOne;
    ಯಾವತ್( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      ಪೂರ್ಣಾಙ್ಕ iTwo = 0;
      WhereTerm *pTwo;
      ಯಾವತ್( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
        whereCombineDisjuncts(pSrc, pWC, pOne, pTwo);
      }
    }
  }

  /*
  ** chngToIN holds a set of tables that *might* satisfy case 1.  But
  ** we have to do some additional checking to see if case 1 really
  ** is satisfied.
  **
  ** chngToIN will hold either 0, 1, or 2 bits.  The 0-bit case means
  ** that there is no possibility of transforming the OR clause into an
  ** IN operator because one or more terms in the OR clause contain
  ** something other than == on a column in the single table.  The 1-bit
  ** case means that every term of the OR clause is of the form
  ** "table.column=expr" for some single table.  The one bit that is set
  ** will correspond to the common table.  We still need to check to make
  ** sure the same column is used on all terms.  The 2-bit case is when
  ** the all terms are of the form "table1.column=table2.column".  It
  ** might be possible to form an IN operator with either table1.column
  ** or table2.column as the LHS if either is common to every term of
  ** the OR clause.
  **
  ** Note that terms of the form "table.column1=table.column2" (the
  ** same table on both sizes of the ==) cannot be optimized.
  */
  ಯದಿ( chngToIN ){
    ಪೂರ್ಣಾಙ್ಕ okToChngToIN = 0;     /* True if the conversion to IN is valid */
    ಪೂರ್ಣಾಙ್ಕ iColumn = -1;         /* Column index on lhs of IN operator */
    ಪೂರ್ಣಾಙ್ಕ iCursor = -1;         /* Table cursor common to all terms */
    ಪೂರ್ಣಾಙ್ಕ j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    ಪ್ರತಿ(j=0; j<2 && !okToChngToIN; j++){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = 0;
      pOrTerm = pOrWc->a;
      ಪ್ರತಿ(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        ಪ್ರತಿಜ್ಞಾ( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        ಯದಿ( pOrTerm->ಎಡ_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          ಪ್ರತಿಜ್ಞಾ( j==1 );
          ಅನುವರ್ತ;
        }
        ಯದಿ( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->ಎಡ_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          ಪ್ರತಿಜ್ಞಾ( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          ಅನುವರ್ತ;
        }
        ಪ್ರತಿಜ್ಞಾ( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.ಎಡ_Column;
        iCursor = pOrTerm->ಎಡ_Cursor;
        p_ಎಡ = pOrTerm->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        ಭಙ್ಗ;
      }
      ಯದಿ( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        ಪ್ರತಿಜ್ಞಾ( j==1 );
        ಪ್ರತಿಜ್ಞಾ( IsPowerOfTwo(chngToIN) );
        ಪ್ರತಿಜ್ಞಾ( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        ಭಙ್ಗ;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      ಪ್ರತಿ(; i>=0 && okToChngToIN; i--, pOrTerm++){
        ಪ್ರತಿಜ್ಞಾ( pOrTerm->eOperator & WO_EQ );
        ಪ್ರತಿಜ್ಞಾ( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        ಯದಿ( pOrTerm->ಎಡ_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }ಅನ್ಯಥಾ ಯದಿ( pOrTerm->u.x.ಎಡ_Column!=iColumn || (iColumn==XN_ಅಭಿವ್ಯಕ್ತಿ
               && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pOrTerm->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಎಡ, -1)
        )){
          okToChngToIN = 0;
        }ಅನ್ಯಥಾ{
          ಪೂರ್ಣಾಙ್ಕ aff_ಎಡ, aff_ಬಲ;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pOrTerm->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
          aff_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pOrTerm->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
          ಯದಿ( aff_ಬಲ!=0 && aff_ಬಲ!=aff_ಎಡ ){
            okToChngToIN = 0;
          }ಅನ್ಯಥಾ{
            pOrTerm->wtFlags |= TERM_OK;
          }
        }
      }
    }

    /* At this point, okToChngToIN is true if original pTerm is a
    ** candidate to satisfy case 1, though we are not yet certain that
    ** the collating sequences are all compatible.  Try to construct a
    ** new virtual term that is pTerm converted from an OR operator
    ** into an IN operator.
    **
    ** During construction, verify that the collating sequences on all
    ** subterms of the OR are compatible.  Omit the construction of the
    ** new IN operator if there are any collating sequence mismatches.
    */
    ಯದಿ( okToChngToIN ){
      ಅಭಿವ್ಯಕ್ತಿ *pDup;            /* A transient duplicate expression */
      ಅಭಿವ್ಯಕ್ತಿ_List *pList = 0;   /* The RHS of the IN operator */
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      ಅಭಿವ್ಯಕ್ತಿ *pNew;            /* The complete IN operator */

      ಪ್ರತಿ(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        ಅಭಿವ್ಯಕ್ತಿ *pThis;
        ಯದಿ( (pOrTerm->wtFlags & TERM_OK)==0 ) ಅನುವರ್ತ;
        ಪ್ರತಿಜ್ಞಾ( pOrTerm->eOperator & WO_EQ );
        ಪ್ರತಿಜ್ಞಾ( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        ಪ್ರತಿಜ್ಞಾ( pOrTerm->ಎಡ_Cursor==iCursor );
        ಪ್ರತಿಜ್ಞಾ( pOrTerm->u.x.ಎಡ_Column==iColumn );
        pThis = pOrTerm->p_ಅಭಿವ್ಯಕ್ತಿ;
        pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pThis->p_ಬಲ, 0);
        pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pWInfo->pParse, pList, pDup);
        ಯದಿ( p_ಎಡ==0 ){
          p_ಎಡ = pThis->p_ಎಡ;
          pCollSeq = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(pParse, pThis);
        }ಅನ್ಯಥಾ{
          ಪ್ರತಿಜ್ಞಾ( 0==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, 
                        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(pThis->p_ಎಡ),
                        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(p_ಎಡ), -1) );
          ಯದಿ( pCollSeq!=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(pParse, pThis) ){
            p_ಎಡ = 0;  /* Collating sequence mismatch */
            ಭಙ್ಗ;
          }
        }
      }
      ಯದಿ( p_ಎಡ==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }ಅನ್ಯಥಾ{
        pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0);
        ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          ಪ್ರತಿಜ್ಞಾ( pCollSeq->zName!=0 );
          pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      ಯದಿ( pNew ){
        ಪೂರ್ಣಾಙ್ಕ idxNew;
        transferJoinMarkings(pNew, p_ಅಭಿವ್ಯಕ್ತಿ);
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_ಸೇರಿಸು(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }ಅನ್ಯಥಾ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList);
      }
    }
  }
}
#endif /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

/*
** We already know that pExpr is a binary operator where both operands are
** column references.  This routine checks to see if pExpr is an equivalence
** relation:
**   1.  The SQLITE_Transitive optimization must be enabled
**   2.  Must be either an == or an IS operator
**   3.  Not originating in the ON clause of an OUTER JOIN
**   4.  The operator is not IS or else the query does not contain RIGHT JOIN
**   5.  The affinities of A and B must be compatible
**   6.  Both operands use the same collating sequence, and they must not
**       use explicit COLLATE clauses.
** If this routine returns TRUE, that means that the RHS can be substituted
** for the LHS anyplace else in the WHERE clause where the LHS column occurs.
** This is an optimization.  No harm comes from returning 0.  But if 1 is
** returned when it should not be, then incorrect answers might result.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ termIsEquivalence(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, SrcList *pSrc){
  ವರ್ಣ aff1, aff2;
  ಯದಿ( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) ಪ್ರತ್ಯಾಗಮ 0;  /* (1) */
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_EQ && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_IS ) ಪ್ರತ್ಯಾಗಮ 0;                 /* (2) */
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_Collate) ) ಪ್ರತ್ಯಾಗಮ 0;        /* (3) */
  ಪ್ರತಿಜ್ಞಾ( pSrc!=0 );
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    ಪ್ರತ್ಯಾಗಮ 0;                                                          /* (4) */
  }
  aff1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  aff2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
  ಯದಿ( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    ಪ್ರತ್ಯಾಗಮ 0;                                                          /* (5) */
  }
  ಯದಿ( !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq_Match(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ) ){
    ಪ್ರತ್ಯಾಗಮ 0;                                                          /* (6) */
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ಸ್ಥಿರ Bitmask ಅಭಿವ್ಯಕ್ತಿ_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  ಯಾವತ್( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, pS->pHaving);
    ಯದಿ( ALWAYS(pSrc!=0) ){
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++){
        ಯದಿ( pSrc->a[i].fg.isSubquery ){
          mask |= ಅಭಿವ್ಯಕ್ತಿ_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        ಯದಿ( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        ಯದಿ( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  ಪ್ರತ್ಯಾಗಮ mask;
}

/*
** Expression pExpr is one operand of a comparison operator that might
** be useful for indexing.  This routine checks to see if pExpr appears
** in any index.  Return TRUE (1) if pExpr is an indexed term and return
** FALSE (0) if not.  If TRUE is returned, also set aiCurCol[0] to the cursor
** number of the table that is indexed and aiCurCol[1] to the column number
** of the column that is indexed, or XN_EXPR (-2) if an expression is being
** indexed.
**
** If pExpr is a TK_COLUMN column reference, then this routine always returns
** true even if that particular column is not indexed, because the column
** might be added to an automatic index later.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  ಪೂರ್ಣಾಙ್ಕ *aiCurCol,         /* Write the referenced table cursor and column here */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,           /* An operand of a comparison operator */
  ಪೂರ್ಣಾಙ್ಕ j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ iCur;
  ಕುರು{
    iCur = pFrom->a[j].iCursor;
    ಪ್ರತಿ(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ಯದಿ( pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಅನುವರ್ತ;
      ಪ್ರತಿ(i=0; i<pIdx->n_ಕೀಲಿ_Col; i++){
        ಯದಿ( pIdx->aiColumn[i]!=XN_ಅಭಿವ್ಯಕ್ತಿ ) ಅನುವರ್ತ;
        ಪ್ರತಿಜ್ಞಾ( pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ );
        ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Skip(p_ಅಭಿವ್ಯಕ್ತಿ,pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ->a[i].p_ಅಭಿವ್ಯಕ್ತಿ,iCur)==0
         && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(0,pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ->a[i].p_ಅಭಿವ್ಯಕ್ತಿ)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_ಅಭಿವ್ಯಕ್ತಿ;
          ಪ್ರತ್ಯಾಗಮ 1;
        }
      }
    }
  }ಯಾವತ್( ++j < pFrom->nSrc );
  ಪ್ರತ್ಯಾಗಮ 0;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  ಪೂರ್ಣಾಙ್ಕ *aiCurCol,         /* Write the referenced table cursor & column here */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,           /* An operand of a comparison operator */
  ಪೂರ್ಣಾಙ್ಕ op                 /* The specific comparison operator */
){
  ಪೂರ್ಣಾಙ್ಕ i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  ಪ್ರತಿಜ್ಞಾ( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  ಪ್ರತಿಜ್ಞಾ( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  ಪ್ರತಿಜ್ಞಾ( op<=TK_GE );
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
  }

  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN ){
    aiCurCol[0] = p_ಅಭಿವ್ಯಕ್ತಿ->iTable;
    aiCurCol[1] = p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
    ಪ್ರತ್ಯಾಗಮ 1;
  }

  ಪ್ರತಿ(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    ಪ್ರತಿ(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ಯದಿ( pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ ){
        ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ_Might_Be_Indexed2(pFrom,aiCurCol,p_ಅಭಿವ್ಯಕ್ತಿ,i);
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


/*
** The input to this routine is an WhereTerm structure with only the
** "pExpr" field filled in.  The job of this routine is to analyze the
** subexpression and populate all the other fields of the WhereTerm
** structure.
**
** If the expression is of the form "<expr> <op> X" it gets commuted
** to the standard form of "X <op> <expr>".
**
** If the expression is of the form "X <op> Y" where both X and Y are
** columns, then the original expression is unchanged and a new virtual
** term of the form "Y <op> X" is added to the WHERE clause and
** analyzed separately.  The original term is marked with TERM_COPIED
** and the new term is marked with TERM_DYNAMIC (because it's pExpr
** needs to be freed with the WhereClause) and TERM_VIRTUAL (because it
** is a commuted copy of a prior term.)  The original term has nChild=1
** and the copy has idxParent set to the index of the original term.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  ಪೂರ್ಣಾಙ್ಕ idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;                     /* The expression to be analyzed */
  Bitmask prereq_ಎಡ;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_ಬಲ = 0;          /* Extra dependencies on LEFT JOIN */
  ಅಭಿವ್ಯಕ್ತಿ *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  ಪೂರ್ಣಾಙ್ಕ isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  ಪೂರ್ಣಾಙ್ಕ noCase = 0;                  /* uppercase equivalent to lowercase */
  ಪೂರ್ಣಾಙ್ಕ op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned ವರ್ಣ eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  ಪೂರ್ಣಾಙ್ಕ n_ಎಡ;                       /* Number of elements on left side vector */

  ಯದಿ( db->mallocFailed ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿಜ್ಞಾ( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_ಅಭಿವ್ಯಕ್ತಿ = pTerm->p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 ); /* Because malloc() has not failed */
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AS && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_COLLATE );
ಅಭಿವ್ಯಕ್ತಿ_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_ಎಡ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಯದಿ( op==TK_IN ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==0 );
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_IN(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ) ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
      pTerm->prereq_ಬಲ = ಅಭಿವ್ಯಕ್ತಿ_Select_Usage(pMaskSet, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect);
    }ಅನ್ಯಥಾ{
      pTerm->prereq_ಬಲ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(pMaskSet, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList);
    }
    prereqAll = prereq_ಎಡ | pTerm->prereq_ಬಲ;
  }ಅನ್ಯಥಾ{
    pTerm->prereq_ಬಲ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0
     || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect|EP_IfNullRow)
     || p_ಅಭಿವ್ಯಕ್ತಿ->x.pList!=0
    ){
      prereqAll = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(pMaskSet, p_ಅಭಿವ್ಯಕ್ತಿ);
    }ಅನ್ಯಥಾ{
      prereqAll = prereq_ಎಡ | pTerm->prereq_ಬಲ;
    }
  }
  ಯದಿ( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  ಯದಿ( prereqAll!=sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(pMaskSet, p_ಅಭಿವ್ಯಕ್ತಿ) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0,p_ಅಭಿವ್ಯಕ್ತಿ,0);
    ಪ್ರತಿಜ್ಞಾ( 0 );
  }
#endif

  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin);
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ){
      prereqAll |= x;
      extra_ಬಲ = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }ಅನ್ಯಥಾ ಯದಿ( (prereqAll>>1)>=x ){
      ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->ಎಡ_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  ಯದಿ( allowedOp(op) ){
    ಪೂರ್ಣಾಙ್ಕ aiCurCol[2];
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    u16 opMask = (pTerm->prereq_ಬಲ & prereq_ಎಡ)==0 ? WO_ALL : WO_EQUIV;

    ಯದಿ( pTerm->u.x.iField>0 ){
      ಪ್ರತಿಜ್ಞಾ( op==TK_IN );
      ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op==TK_VECTOR );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಎಡ) );
      p_ಎಡ = p_ಎಡ->x.pList->a[pTerm->u.x.iField-1].p_ಅಭಿವ್ಯಕ್ತಿ;
    }

    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Might_Be_Indexed(pSrc, aiCurCol, p_ಎಡ, op) ){
      pTerm->ಎಡ_Cursor = aiCurCol[0];
      ಪ್ರತಿಜ್ಞಾ( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.ಎಡ_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    ಯದಿ( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    ಯದಿ( p_ಬಲ
     && ಅಭಿವ್ಯಕ್ತಿ_Might_Be_Indexed(pSrc, aiCurCol, p_ಬಲ, op)
     && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ, EP_FixedCol)
    ){
      WhereTerm *pNew;
      ಅಭಿವ್ಯಕ್ತಿ *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      ಪ್ರತಿಜ್ಞಾ( pTerm->u.x.iField==0 );
      ಯದಿ( pTerm->ಎಡ_Cursor>=0 ){
        ಪೂರ್ಣಾಙ್ಕ idxNew;
        pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಯದಿ( db->mallocFailed ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDup);
          ಪ್ರತ್ಯಾಗಮ;
        }
        idxNew = where_Clause_ಸೇರಿಸು(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        ಯದಿ( idxNew==0 ) ಪ್ರತ್ಯಾಗಮ;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        ಯದಿ( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        ಪ್ರತಿಜ್ಞಾ( pWInfo->pTabList!=0 );
        ಯದಿ( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }ಅನ್ಯಥಾ{
        pDup = p_ಅಭಿವ್ಯಕ್ತಿ;
        pNew = pTerm;
      }
      pNew->wtFlags |= ಅಭಿವ್ಯಕ್ತಿ_Commute(pParse, pDup);
      pNew->ಎಡ_Cursor = aiCurCol[0];
      ಪ್ರತಿಜ್ಞಾ( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.ಎಡ_Column = aiCurCol[1];
      testcase( (prereq_ಎಡ | extra_ಬಲ) != prereq_ಎಡ );
      pNew->prereq_ಬಲ = prereq_ಎಡ | extra_ಬಲ;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }ಅನ್ಯಥಾ
    ಯದಿ( op==TK_ISNULL
     && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_OuterON)
     && 0==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಎಡ)
    ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken = "false";
      ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_IsFalse);
      pTerm->prereqAll = 0;
      pTerm->eOperator = 0;
    }
  }

#ifndef SQLITE_OMIT_BETWEEN_OPTIMIZATION
  /* If a term is the BETWEEN operator, create two new virtual terms
  ** that define the range that the BETWEEN implements.  For example:
  **
  **      a BETWEEN b AND c
  **
  ** is converted into:
  **
  **      (a BETWEEN b AND c) AND (a>=b) AND (a<=c)
  **
  ** The two new terms are added onto the end of the WhereClause object.
  ** The new terms are "dynamic" and are children of the original BETWEEN
  ** term.  That means that if the BETWEEN term is coded, the children are
  ** skipped.  Or, if the children are satisfied by an index, the original
  ** BETWEEN term is skipped.
  */
  ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_BETWEEN && pWC->op==TK_AND ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಸ್ಥಿರ ಧ್ರುವ u8 ops[] = {TK_GE, TK_LE};
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
    ಪ್ರತಿಜ್ಞಾ( pList!=0 );
    ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 );
    ಪ್ರತಿಜ್ಞಾ( pWC->a[idxTerm].nChild==0 );
    ಪ್ರತಿ(i=0; i<2; i++){
      ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
      ಪೂರ್ಣಾಙ್ಕ idxNew;
      p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, ops[i],
                             sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0),
                             sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, 0));
      transferJoinMarkings(p_New_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಭಿವ್ಯಕ್ತಿ);
      idxNew = where_Clause_ಸೇರಿಸು(pWC, p_New_ಅಭಿವ್ಯಕ್ತಿ, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#ಯದಿ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_OR && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Collate) ){
    ಅಭಿವ್ಯಕ್ತಿ *pAlt = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ);
    ಯದಿ( pAlt!=p_ಅಭಿವ್ಯಕ್ತಿ ){
      pTerm->p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pAlt);
      ಗಚ್ಛ ಅಭಿವ್ಯಕ್ತಿ_Analyze_restart;
    }
    ಪ್ರತಿಜ್ಞಾ( pWC->op==TK_AND );
    ಅಭಿವ್ಯಕ್ತಿ_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL ){
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_COLUMN
     && p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->iColumn>=0
     && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON)
    ){
      ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಪೂರ್ಣಾಙ್ಕ idxNew;
      WhereTerm *pNewTerm;
 
      p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0),
                              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_ಸೇರಿಸು(pWC, p_New_ಅಭಿವ್ಯಕ್ತಿ,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      ಯದಿ( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_ಬಲ = 0;
        pNewTerm->ಎಡ_Cursor = p_ಎಡ->iTable;
        pNewTerm->u.x.ಎಡ_Column = p_ಎಡ->iColumn;
        pNewTerm->eOperator = WO_GT;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
    }
  }


#ifndef SQLITE_OMIT_LIKE_OPTIMIZATION
  /* Add constraints to reduce the search space on a LIKE or GLOB
  ** operator.
  **
  ** A like pattern of the form "x LIKE 'aBc%'" is changed into constraints
  **
  **          x>='ABC' AND x<'abd' AND x LIKE 'aBc%'
  **
  ** The last character of the prefix "abc" is incremented to form the
  ** termination condition "abd".  If case is not significant (the default
  ** for LIKE) then the lower-bound is made all uppercase and the upper-
  ** bound is made all lowercase so that the bounds also work when comparing
  ** BLOBs.
  */
  ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, &pStr1, &isComplete, &noCase)
  ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;       /* LHS of LIKE/GLOB operator */
    ಅಭಿವ್ಯಕ್ತಿ *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    ಅಭಿವ್ಯಕ್ತಿ *pNewExpr1;
    ಅಭಿವ್ಯಕ್ತಿ *pNewExpr2;
    ಪೂರ್ಣಾಙ್ಕ idxNew1;
    ಪೂರ್ಣಾಙ್ಕ idxNew2;
    ಧ್ರುವ ವರ್ಣ *zCollSeqName;     /* Name of collating sequence */
    ಧ್ರುವ u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
    pStr2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pStr1, 0);
    ಪ್ರತಿಜ್ಞಾ( pStr1==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pStr1, EP_Int_ಮೌಲ್ಯ) );
    ಪ್ರತಿಜ್ಞಾ( pStr2==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pStr2, EP_Int_ಮೌಲ್ಯ) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    ಯದಿ( noCase && !pParse->db->mallocFailed ){
      ಪೂರ್ಣಾಙ್ಕ i;
      ವರ್ಣ c;
      pTerm->wtFlags |= TERM_LIKE;
      ಪ್ರತಿ(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    ಯದಿ( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      ಯದಿ( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        ಯದಿ( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      ಯಾವತ್( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      ಪ್ರತಿಜ್ಞಾ( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ದ್ವಿಮಾನ;
    pNewExpr1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_ಅಭಿವ್ಯಕ್ತಿ);
    idxNew1 = where_Clause_ಸೇರಿಸು(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_ಅಭಿವ್ಯಕ್ತಿ);
    idxNew2 = where_Clause_ಸೇರಿಸು(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew1);
    ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    ಯದಿ( isComplete ){
      markTermAsChild(pWC, idxNew1, idxTerm);
      markTermAsChild(pWC, idxNew2, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

  /* If there is a vector == or IS term - e.g. "(a, b) == (?, ?)" - create
  ** new terms for each component comparison - "a = ?" and "b = ?".  The
  ** new terms completely replace the original vector comparison, which is
  ** no longer used.
  **
  ** This is only required if at least one side of the comparison operation
  ** is not a sub-select.
  **
  ** tag-20220128a
  */
  ಯದಿ( (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EQ || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS)
   && (n_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ))>1
   && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ)==n_ಎಡ
   && ( (p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->flags & EP_xIsSelect)==0
     || (p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<n_ಎಡ; i++){
      ಪೂರ್ಣಾಙ್ಕ idxNew;
      ಅಭಿವ್ಯಕ್ತಿ *pNew;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_Vector_Field(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, i, n_ಎಡ);
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_Vector_Field(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, i, n_ಎಡ);

      pNew = sqlite3PExpr(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->op, p_ಎಡ, p_ಬಲ);
      transferJoinMarkings(pNew, p_ಅಭಿವ್ಯಕ್ತಿ);
      idxNew = where_Clause_ಸೇರಿಸು(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
    }
    pTerm = &pWC->a[idxTerm];
    pTerm->wtFlags |= TERM_CODED|TERM_VIRTUAL;  /* Disable the original */
    pTerm->eOperator = WO_ROWVAL;
  }

  /* If there is a vector IN term - e.g. "(a, b) IN (SELECT ...)" - create
  ** a virtual term for each vector component. The expression object
  ** used by each such virtual term is pExpr (the full vector IN(...)
  ** expression). The WhereTerm.u.x.iField variable identifies the index within
  ** the vector on the LHS that the virtual term represents.
  **
  ** This only works if the RHS is a simple SELECT (not a compound) that does
  ** not use window functions.
  */
  ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IN
   && pTerm->u.x.iField==0
   && p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_VECTOR
   && ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) )
   && (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pPrior==0 || (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_ಮೌಲ್ಯಗಳು))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pWin==0
#endif
   && (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿಜ್ಞಾ( pTerm->nChild==0 );
    ಪ್ರತಿ(i=0; i<sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ); i++){
      ಪೂರ್ಣಾಙ್ಕ idxNew;
      idxNew = where_Clause_ಸೇರಿಸು(pWC, p_ಅಭಿವ್ಯಕ್ತಿ, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }

#ifndef SQLITE_OMIT_VIRTUALTABLE
  /* Add a WO_AUX auxiliary term to the constraint set if the
  ** current expression is of the form "column OP expr" where OP
  ** is an operator that gets passed into virtual tables but which is
  ** not normally optimized for ordinary tables.  In other words, OP
  ** is one of MATCH, LIKE, GLOB, REGEXP, !=, IS, IS NOT, or NOT NULL.
  ** This information is used by the xBestIndex methods of
  ** virtual tables.  The native query optimizer does not attempt
  ** to do anything with MATCH functions.
  */
  ಅನ್ಯಥಾ ಯದಿ( pWC->op==TK_AND ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = 0, *p_ಎಡ = 0;
    ಪೂರ್ಣಾಙ್ಕ res = isAuxiliaryVtabOperator(db, p_ಅಭಿವ್ಯಕ್ತಿ, &eOp2, &p_ಎಡ, &p_ಬಲ);
    ಯಾವತ್( res-- > 0 ){
      ಪೂರ್ಣಾಙ್ಕ idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_ಅಭಿವ್ಯಕ್ತಿ;

      prereq_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, p_ಬಲ);
      prereqColumn = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, p_ಎಡ);
      ಯದಿ( (prereq_ಅಭಿವ್ಯಕ್ತಿ & prereqColumn)==0 ){
        ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
        p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಬಲ, 0));
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) && p_New_ಅಭಿವ್ಯಕ್ತಿ ){
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_New_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON);
          p_New_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin = p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin;
        }
        idxNew = where_Clause_ಸೇರಿಸು(pWC, p_New_ಅಭಿವ್ಯಕ್ತಿ, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_ಬಲ = prereq_ಅಭಿವ್ಯಕ್ತಿ | extra_ಬಲ;
        pNewTerm->ಎಡ_Cursor = p_ಎಡ->iTable;
        pNewTerm->u.x.ಎಡ_Column = p_ಎಡ->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(ಅಭಿವ್ಯಕ್ತಿ*, p_ಎಡ, p_ಬಲ);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_ಬಲ |= extra_ಬಲ;
}

/***************************************************************************
** Routines with file scope above.  Interface to the rest of the where.c
** subsystem follows.
***************************************************************************/

/*
** This routine identifies subexpressions in the WHERE clause where
** each subexpression is separated by the AND operator or some other
** operator specified in the op parameter.  The WhereClause structure
** is filled with pointers to subexpressions.  For example:
**
**    WHERE  a=='hello' AND coalesce(b,11)<10 AND (c+12!=d OR c==22)
**           \________/     \_______________/     \________________/
**            slot[0]            slot[1]               slot[2]
**
** The original WHERE clause in pExpr is unaltered.  All this routine
** does is make slot[] entries point to substructure within pExpr.
**
** In the previous sentence and in the diagram, "slot[]" refers to
** the WhereClause.a[] array.  The slot[] array grows as needed to contain
** all terms of the WHERE clause.
*/
ಶೂನ್ಯ sqlite3WhereSplit(WhereClause *pWC, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, u8 op){
  ಅಭಿವ್ಯಕ್ತಿ *pE2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ);
  pWC->op = op;
  ಪ್ರತಿಜ್ಞಾ( pE2!=0 || p_ಅಭಿವ್ಯಕ್ತಿ==0 );
  ಯದಿ( pE2==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( pE2->op!=op ){
    where_Clause_ಸೇರಿಸು(pWC, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  }ಅನ್ಯಥಾ{
    sqlite3WhereSplit(pWC, pE2->p_ಎಡ, op);
    sqlite3WhereSplit(pWC, pE2->p_ಬಲ, op);
  }
}

/*
** Add either a LIMIT (if eMatchOp==SQLITE_INDEX_CONSTRAINT_LIMIT) or
** OFFSET (if eMatchOp==SQLITE_INDEX_CONSTRAINT_OFFSET) term to the
** where-clause passed as the first argument. The value for the term
** is found in register iReg.
**
** In the common case where the value is a simple integer
** (example: "LIMIT 5 OFFSET 10") then the expression codes as a
** TK_INTEGER so that it will be available to sqlite3_vtab_rhs_value().
** If not, then it codes as a TK_REGISTER expression.
*/
ಸ್ಥಿರ ಶೂನ್ಯ where_Add_Limit_ಅಭಿವ್ಯಕ್ತಿ(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  ಪೂರ್ಣಾಙ್ಕ iReg,           /* Register that will hold value of the limit/offset */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,        /* Expression that defines the limit/offset */
  ಪೂರ್ಣಾಙ್ಕ iCsr,           /* Cursor to which the constraint applies */
  ಪೂರ್ಣಾಙ್ಕ eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  ಪೂರ್ಣಾಙ್ಕ iVal = 0;

  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p_ಅಭಿವ್ಯಕ್ತಿ, &iVal, pParse, 0) && iVal>=0 ){
    ಅಭಿವ್ಯಕ್ತಿ *pVal = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, iVal);
    ಯದಿ( pVal==0 ) ಪ್ರತ್ಯಾಗಮ;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }ಅನ್ಯಥಾ{
    ಅಭಿವ್ಯಕ್ತಿ *pVal = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_REGISTER, 0, 0);
    ಯದಿ( pVal==0 ) ಪ್ರತ್ಯಾಗಮ;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  ಯದಿ( pNew ){
    WhereTerm *pTerm;
    ಪೂರ್ಣಾಙ್ಕ idx;
    idx = where_Clause_ಸೇರಿಸು(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->ಎಡ_Cursor = iCsr;
    pTerm->eOperator = WO_AUX;
    pTerm->eMatchOp = eMatchOp;
  }
}

/*
** Possibly add terms corresponding to the LIMIT and OFFSET clauses of the
** SELECT statement passed as the second argument. These terms are only
** added if:
**
**   1. The SELECT statement has a LIMIT clause, and
**   2. The SELECT statement is not an aggregate or DISTINCT query, and
**   3. The SELECT statement has exactly one object in its FROM clause, and
**      that object is a virtual table, and
**   4. There are no terms in the WHERE clause that will not be passed
**      to the virtual table xBestIndex method.
**   5. The ORDER BY clause, if any, will be made available to the xBestIndex
**      method.
**
** LIMIT and OFFSET terms are ignored by most of the planner code. They
** exist only so that they may be passed to the xBestIndex method of the
** single virtual table in the FROM clause of the SELECT.
*/
ಶೂನ್ಯ SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  ಪ್ರತಿಜ್ಞಾ( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  ಯದಿ( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy = p->pOrderBy;
    ಪೂರ್ಣಾಙ್ಕ iCsr = p->pSrc->a[0].iCursor;
    ಪೂರ್ಣಾಙ್ಕ ii;

    /* Check condition (4). Return early if it is not met. */
    ಪ್ರತಿ(ii=0; ii<pWC->nTerm; ii++){
      ಯದಿ( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        ಪ್ರತಿಜ್ಞಾ( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        ಪ್ರತಿಜ್ಞಾ( pWC->a[ii].eOperator==WO_ROWVAL );
        ಅನುವರ್ತ;
      }
      ಯದಿ( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        ಅನುವರ್ತ;
      }
      ಯದಿ( pWC->a[ii].ಎಡ_Cursor==iCsr && pWC->a[ii].prereq_ಬಲ==0 ) ಅನುವರ್ತ;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      ಯದಿ( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        ಯದಿ( pParent->ಎಡ_Cursor==iCsr
         && pParent->prereq_ಬಲ==0
         && pParent->nChild==1
        ){
          ಅನುವರ್ತ;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      ಪ್ರತ್ಯಾಗಮ;
    }

    /* Check condition (5). Return early if it is not met. */
    ಯದಿ( pOrderBy ){
      ಪ್ರತಿ(ii=0; ii<pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
        ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pOrderBy->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_COLUMN ) ಪ್ರತ್ಯಾಗಮ;
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable!=iCsr ) ಪ್ರತ್ಯಾಗಮ;
        ಯದಿ( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) ಪ್ರತ್ಯಾಗಮ;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    ಪ್ರತಿಜ್ಞಾ( p->pLimit->op==TK_LIMIT );
    ಯದಿ( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_ಅಭಿವ್ಯಕ್ತಿ(pWC, p->iOffset, p->pLimit->p_ಬಲ,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    ಯದಿ( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_ಅಭಿವ್ಯಕ್ತಿ(pWC, p->iLimit, p->pLimit->p_ಎಡ,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
ಶೂನ್ಯ sqlite3WhereClauseInit(
  WhereClause *pWC,        /* The WhereClause to be initialized */
  WhereInfo *pWInfo        /* The WHERE processing context */
){
  pWC->pWInfo = pWInfo;
  pWC->hasOr = 0;
  pWC->pOuter = 0;
  pWC->nTerm = 0;
  pWC->nBase = 0;
  pWC->nSlot = ArraySize(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
ಶೂನ್ಯ sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  ಪ್ರತಿಜ್ಞಾ( pWC->nTerm>=pWC->nBase );
  ಯದಿ( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    ಪೂರ್ಣಾಙ್ಕ i;
    /* Verify that every term past pWC->nBase is virtual */
    ಪ್ರತಿ(i=pWC->nBase; i<pWC->nTerm; i++){
      ಪ್ರತಿಜ್ಞಾ( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    ಯಾವತ್(1){
      ಪ್ರತಿಜ್ಞಾ( a->eMatchOp==0 || a->eOperator==WO_AUX );
      ಯದಿ( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, a->p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ಯದಿ( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        ಯದಿ( a->wtFlags & TERM_ORINFO ){
          ಪ್ರತಿಜ್ಞಾ( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }ಅನ್ಯಥಾ{
          ಪ್ರತಿಜ್ಞಾ( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      ಯದಿ( a==aLast ) ಭಙ್ಗ;
      a++;
    }
  }
}


/*
** These routines walk (recursively) an expression tree and generate
** a bitmask indicating which tables are used in that expression
** tree.
**
** sqlite3WhereExprUsage(MaskSet, Expr) ->
**
**       Return a Bitmask of all tables referenced by Expr.  Expr can be
**       be NULL, in which case 0 is returned.
**
** sqlite3WhereExprUsageNN(MaskSet, Expr) ->
**
**       Same as sqlite3WhereExprUsage() except that Expr must not be
**       NULL.  The "NN" suffix on the name stands for "Not Null".
**
** sqlite3WhereExprListUsage(MaskSet, ExprList) ->
**
**       Return a Bitmask of all tables referenced by every expression
**       in the expression list ExprList.  ExprList can be NULL, in which
**       case 0 is returned.
**
** sqlite3WhereExprUsageFull(MaskSet, ExprList) ->
**
**       Internal use only.  Called only by sqlite3WhereExprUsageNN() for
**       complex expressions that require pushing register values onto
**       the stack.  Many calls to sqlite3WhereExprUsageNN() do not need
**       the more complex analysis done by this routine.  Hence, the
**       computations done by this routine are broken out into a separate
**       "no-inline" function to avoid the stack push overhead in the
**       common case where it is not needed.
*/
ಸ್ಥಿರ SQLITE_NOINLINE Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_Full(
  WhereMaskSet *pMaskSet,
  ಅಭಿವ್ಯಕ್ತಿ *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  ಯದಿ( p->p_ಎಡ ) mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(pMaskSet, p->p_ಎಡ);
  ಯದಿ( p->p_ಬಲ ){
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(pMaskSet, p->p_ಬಲ);
    ಪ್ರತಿಜ್ಞಾ( p->x.pList==0 );
  }ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= ಅಭಿವ್ಯಕ್ತಿ_Select_Usage(pMaskSet, p->x.pSelect);
  }ಅನ್ಯಥಾ ಯದಿ( p->x.pList ){
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  ಯದಿ( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p) ){
    ಪ್ರತಿಜ್ಞಾ( p->y.pWin!=0 );
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ mask;
}
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(WhereMaskSet *pMaskSet, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಯದಿ( p->op==TK_COLUMN && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_FixedCol) ){
    ಪ್ರತ್ಯಾಗಮ sqlite3WhereGetMask(pMaskSet, p->iTable);
  }ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    ಪ್ರತಿಜ್ಞಾ( p->op!=TK_IF_NULL_ROW );
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(WhereMaskSet *pMaskSet, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಪ್ರತ್ಯಾಗಮ p ? sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(WhereMaskSet *pMaskSet, ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಪೂರ್ಣಾಙ್ಕ i;
  Bitmask mask = 0;
  ಯದಿ( pList ){
    ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      mask |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(pMaskSet, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
  }
  ಪ್ರತ್ಯಾಗಮ mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
ಶೂನ್ಯ sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=pWC->nTerm-1; i>=0; i--){
    ಅಭಿವ್ಯಕ್ತಿ_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
ಶೂನ್ಯ sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  ಪೂರ್ಣಾಙ್ಕ j, k;
  ಅಭಿವ್ಯಕ್ತಿ_List *pArgs;
  ಅಭಿವ್ಯಕ್ತಿ *pColRef;
  ಅಭಿವ್ಯಕ್ತಿ *pTerm;
  ಯದಿ( pItem->fg.isTabFunc==0 ) ಪ್ರತ್ಯಾಗಮ;
  pTab = pItem->pSTab;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಯದಿ( !IsVirtual(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "'%s' is not a function", pItem->zName);
    ಪ್ರತ್ಯಾಗಮ;
  }
  pArgs = pItem->u1.pFuncArg;
  ಯದಿ( pArgs==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ(j=k=0; j<pArgs->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
    ಅಭಿವ್ಯಕ್ತಿ *pRhs;
    u32 joinType;
    ಯಾವತ್( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    ಯದಿ( k>=pTab->nCol ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      ಪ್ರತ್ಯಾಗಮ;
    }
    pColRef = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(pParse->db, TK_COLUMN, 0, 0);
    ಯದಿ( pColRef==0 ) ಪ್ರತ್ಯಾಗಮ;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, pArgs->a[j].p_ಅಭಿವ್ಯಕ್ತಿ, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    ಯದಿ( pItem->fg.jointype & (JT_ಎಡ|JT_ಬಲ) ){
      testcase( pItem->fg.jointype & JT_ಎಡ );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_ಬಲ ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }ಅನ್ಯಥಾ{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_ಅಭಿವ್ಯಕ್ತಿ(pTerm, pItem->iCursor, joinType);
    where_Clause_ಸೇರಿಸು(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3WhereLoopBloomable(ಧ್ರುವ WhereLoop *pLoop){
  ಯದಿ( pLoop->wsFlags & WHERE_IPK ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯದಿ( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
