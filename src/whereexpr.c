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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "whereInt.h"

/* Forward declarations */
ସ୍ଥିର ଶୂନ୍ୟ ପଦସମଷ୍ଟି_Analyze(SrcList*, WhereClause*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
ସ୍ଥିର ଶୂନ୍ୟ whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
ସ୍ଥିର ଶୂନ୍ୟ whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା where_Clause_ଭର୍ତ୍ତି(WhereClause *pWC, ପଦସମଷ୍ଟି *p, u16 wtFlags){
  WhereTerm *pTerm;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;
  testcase( wtFlags & TERM_VIRTUAL );
  ଯଦି( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, ଆକାର(pWC->a[0])*pWC->nSlot*2 );
    ଯଦି( pWC->a==0 ){
      ଯଦି( wtFlags & TERM_DYNAMIC ){
        sqlite3_ପଦସମଷ୍ଟି_Delete(db, p);
      }
      pWC->a = pOld;
      ଫେରନ୍ତୁ 0;
    }
    memcpy(pWC->a, pOld, ଆକାର(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  ଯଦି( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  ଯଦି( p && ପଦସମଷ୍ଟି_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }ଅନ୍ୟଥା{
    pTerm->truthProb = 1;
  }
  pTerm->p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         ଆକାର(WhereTerm) - offsetof(WhereTerm,eOperator));
  ଫେରନ୍ତୁ idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା allowedOp(ପୂର୍ଣ୍ଣସଂଖ୍ୟା op){
  ପ୍ରତିଜ୍ଞା( TK_GT>TK_EQ && TK_GT<TK_GE );
  ପ୍ରତିଜ୍ଞା( TK_LT>TK_EQ && TK_LT<TK_GE );
  ପ୍ରତିଜ୍ଞା( TK_LE>TK_EQ && TK_LE<TK_GE );
  ପ୍ରତିଜ୍ଞା( TK_GE==TK_EQ+4 );
  ପ୍ରତିଜ୍ଞା( TK_IN<TK_EQ );
  ପ୍ରତିଜ୍ଞା( TK_IS<TK_EQ );
  ପ୍ରତିଜ୍ଞା( TK_ISNULL<TK_EQ );
  ଯଦି( op>TK_GE ) ଫେରନ୍ତୁ 0;
  ଯଦି( op>=TK_EQ ) ଫେରନ୍ତୁ 1;
  ଫେରନ୍ତୁ op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ସ୍ଥିର u16 ପଦସମଷ୍ଟି_Commute(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ->op==TK_VECTOR
   || p_ପଦସମଷ୍ଟି->p_ଡାହାଣ->op==TK_VECTOR
   || sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ) !=
      sqlite3_ଦ୍ୱିମିକ_Compare_Coll_Seq(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, p_ପଦସମଷ୍ଟି->p_ବାମ)
  ){
    p_ପଦସମଷ୍ଟି->flags ^= EP_Commuted;
  }
  SWAP(ପଦସମଷ୍ଟି*,p_ପଦସମଷ୍ଟି->p_ଡାହାଣ,p_ପଦସମଷ୍ଟି->p_ବାମ);
  ଯଦି( p_ପଦସମଷ୍ଟି->op>=TK_GT ){
    ପ୍ରତିଜ୍ଞା( TK_LT==TK_GT+2 );
    ପ୍ରତିଜ୍ଞା( TK_GE==TK_LE+2 );
    ପ୍ରତିଜ୍ଞା( TK_GT>TK_EQ );
    ପ୍ରତିଜ୍ଞା( TK_GT<TK_LE );
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op>=TK_GT && p_ପଦସମଷ୍ଟି->op<=TK_GE );
    p_ପଦସମଷ୍ଟି->op = ((p_ପଦସମଷ୍ଟି->op-TK_GT)^2)+TK_GT;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ସ୍ଥିର u16 operatorMask(ପୂର୍ଣ୍ଣସଂଖ୍ୟା op){
  u16 c;
  ପ୍ରତିଜ୍ଞା( allowedOp(op) );
  ଯଦି( op>=TK_EQ ){
    ପ୍ରତିଜ୍ଞା( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }ଅନ୍ୟଥା ଯଦି( op==TK_IN ){
    c = WO_IN;
  }ଅନ୍ୟଥା ଯଦି( op==TK_ISNULL ){
    c = WO_ISNULL;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( op==TK_IS );
    c = WO_IS;
  }
  ପ୍ରତିଜ୍ଞା( op!=TK_ISNULL || c==WO_ISNULL );
  ପ୍ରତିଜ୍ଞା( op!=TK_IN || c==WO_IN );
  ପ୍ରତିଜ୍ଞା( op!=TK_EQ || c==WO_EQ );
  ପ୍ରତିଜ୍ଞା( op!=TK_LT || c==WO_LT );
  ପ୍ରତିଜ୍ଞା( op!=TK_LE || c==WO_LE );
  ପ୍ରତିଜ୍ଞା( op!=TK_GT || c==WO_GT );
  ପ୍ରତିଜ୍ଞା( op!=TK_GE || c==WO_GE );
  ପ୍ରତିଜ୍ଞା( op!=TK_IS || c==WO_IS );
  ଫେରନ୍ତୁ c;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,      /* Test this expression */
  ପଦସମଷ୍ଟି **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pisComplete, /* True if the only wildcard is % in the last character */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ଅଚଳ u8 *z = 0;           /* String on RHS of LIKE operator */
  ପଦସମଷ୍ଟି *p_ଡାହାଣ, *p_ବାମ;      /* Right and left size of LIKE operator */
  ପଦସମଷ୍ଟି_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_ମୂଲ୍ୟ *pVal = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;                    /* Opcode of pRight */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                    /* Result code to return */

  ଯଦି( !sqlite3IsLikeFunction(db, p_ପଦସମଷ୍ଟି, pnoCase, (ଅକ୍ଷର*)wc) ){
    ଫେରନ୍ତୁ 0;
  }
#ifdef SQLITE_EBCDIC
  ଯଦି( *pnoCase ) ଫେରନ୍ତୁ 0;
#endif
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
  pList = p_ପଦସମଷ୍ଟି->x.pList;
  p_ବାମ = pList->a[1].p_ପଦସମଷ୍ଟି;

  p_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate(pList->a[0].p_ପଦସମଷ୍ଟି);
  op = p_ଡାହାଣ->op;
  ଯଦି( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = p_ଡାହାଣ->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_ମୂଲ୍ୟ(pReprepare, iCol, SQLITE_AFF_BLOB);
    ଯଦି( pVal && sqlite3_ମୂଲ୍ୟ_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_ମୂଲ୍ୟ_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    ପ୍ରତିଜ୍ଞା( p_ଡାହାଣ->op==TK_VARIABLE || p_ଡାହାଣ->op==TK_REGISTER );
  }ଅନ୍ୟଥା ଯଦି( op==TK_STRING ){
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ଡାହାଣ, EP_Int_ମୂଲ୍ୟ) );
     z = (u8*)p_ଡାହାଣ->u.zToken;
  }
  ଯଦି( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    ଯେତେବେଳେ( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      ଯଦି( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }ଅନ୍ୟଥା ଯଦି( c>=0x80 ){
        ଅଚଳ u8 *z2 = z+cnt-1;
        ଯଦି( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା{
          cnt = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(z2-z);
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
    ଯଦି( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      ପଦସମଷ୍ଟି *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_ପଦସମଷ୍ଟି(db, TK_STRING, (ଅକ୍ଷର*)z);
      ଯଦି( pPrefix ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFrom, iTo;
        ଅକ୍ଷର *zNew;
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pPrefix, EP_Int_ମୂଲ୍ୟ) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        ପାଇଁ(iFrom=iTo=0; iFrom<cnt; iFrom++){
          ଯଦି( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;
        ପ୍ରତିଜ୍ଞା( iTo>0 );

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
        ଯଦି( p_ବାମ->op!=TK_COLUMN
         || sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ବାମ)!=SQLITE_AFF_TEXT
         || (ALWAYS( ପଦସମଷ୍ଟି_Use_YTab(p_ବାମ) )
             && ALWAYS(p_ବାମ->y.pTab)
             && IsVirtual(p_ବାମ->y.pTab))  /* Might be numeric */
        ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_ଅଙ୍କ;
          ଦ୍ୱିଗୁଣ rDummy;
          ପ୍ରତିଜ୍ଞା( zNew[iTo]==0 );
          is_ଅଙ୍କ = sqlite3AtoF(zNew, &rDummy);
          ଯଦି( is_ଅଙ୍କ<=0 ){
            ଯଦି( iTo==1 && zNew[0]=='-' ){
              is_ଅଙ୍କ = +1;
            }ଅନ୍ୟଥା{
              zNew[iTo-1]++;
              is_ଅଙ୍କ = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          ଯଦି( is_ଅଙ୍କ>0 ){
            sqlite3_ପଦସମଷ୍ଟି_Delete(db, pPrefix);
            sqlite3_ମୂଲ୍ୟ_Free(pVal);
            ଫେରନ୍ତୁ 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      ଯଦି( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_ଡାହାଣ->iColumn, 0);
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ଡାହାଣ, EP_Int_ମୂଲ୍ୟ) );
        ଯଦି( *pisComplete && p_ଡାହାଣ->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = sqlite3GetTempReg(pParse);
          sqlite3_ପଦସମଷ୍ଟି_Code_Target(pParse, p_ଡାହାଣ, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }ଅନ୍ୟଥା{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_ମୂଲ୍ୟ_Free(pVal);
  ଫେରନ୍ତୁ rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Is_Like_Operator(ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ସ୍ଥିର ଅଚଳ ସଂରଚନା {
    ଅଚଳ ଅକ୍ଷର *zOp;
    unsigned ଅକ୍ଷର eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
  ପାଇଁ(i=0; i<ArraySize(aOp); i++){
    ଯଦି( sqlite3StrICmp(p_ପଦସମଷ୍ଟି->u.zToken, aOp[i].zOp)==0 ){
      ଫେରନ୍ତୁ aOp[i].eOp;
    }
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,                    /* Test this expression */
  unsigned ଅକ୍ଷର *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  ପଦସମଷ୍ଟି **pp_ବାମ,                  /* Column expression to left of MATCH/op2 */
  ପଦସମଷ୍ଟି **pp_ଡାହାଣ                  /* Expression to left of MATCH/op2 */
){
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION ){
    ପଦସମଷ୍ଟି_List *pList;
    ପଦସମଷ୍ଟି *pCol;                     /* Column reference */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
    pList = p_ପଦସମଷ୍ଟି->x.pList;
    ଯଦି( pList==0 || pList->n_ପଦସମଷ୍ଟି!=2 ){
      ଫେରନ୍ତୁ 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_ପଦସମଷ୍ଟି;
    ପ୍ରତିଜ୍ଞା( pCol->op!=TK_COLUMN || (ପଦସମଷ୍ଟି_Use_YTab(pCol) && pCol->y.pTab!=0) );
    ଯଦି( ପଦସମଷ୍ଟି_Is_Vtab(pCol) && (i = sqlite3_ପଦସମଷ୍ଟି_Is_Like_Operator(p_ପଦସମଷ୍ଟି))!=0 ){
      *peOp2 = i;
      *pp_ଡାହାଣ = pList->a[0].p_ପଦସମଷ୍ଟି;
      *pp_ବାମ = pCol;
      ଫେରନ୍ତୁ 1;
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
    pCol = pList->a[0].p_ପଦସମଷ୍ଟି;
    ପ୍ରତିଜ୍ଞା( pCol->op!=TK_COLUMN || ପଦସମଷ୍ଟି_Use_YTab(pCol) );
    ପ୍ରତିଜ୍ଞା( pCol->op!=TK_COLUMN || (ପଦସମଷ୍ଟି_Use_YTab(pCol) && pCol->y.pTab!=0) );
    ଯଦି( ପଦସମଷ୍ଟି_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      ଶୂନ୍ୟ (*xNotUsed)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**);
      ଶୂନ୍ୟ *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      ପ୍ରତିଜ୍ଞା( pVtab!=0 );
      ପ୍ରତିଜ୍ଞା( pVtab->pModule!=0 );
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      pMod = (sqlite3_module *)pVtab->pModule;
      ଯଦି( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_ପଦସମଷ୍ଟି->u.zToken, &xNotUsed, &pNotUsed);
        ଯଦି( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_ଡାହାଣ = pList->a[1].p_ପଦସମଷ୍ଟି;
          *pp_ବାମ = pCol;
          ଫେରନ୍ତୁ 1;
        }
      }
    }
  }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    ପ୍ରତିଜ୍ଞା( TK_NE < TK_EQ );
    ପ୍ରତିଜ୍ଞା( TK_ISNOT < TK_EQ );
    ପ୍ରତିଜ୍ଞା( TK_NOTNULL < TK_EQ );
    ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_NE || p_ପଦସମଷ୍ଟି->op==TK_ISNOT || p_ପଦସମଷ୍ଟି->op==TK_NOTNULL ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
    ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
    ପଦସମଷ୍ଟି *p_ଡାହାଣ = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
    ପ୍ରତିଜ୍ଞା( p_ବାମ->op!=TK_COLUMN || (ପଦସମଷ୍ଟି_Use_YTab(p_ବାମ) && p_ବାମ->y.pTab!=0) );
    ଯଦି( ପଦସମଷ୍ଟି_Is_Vtab(p_ବାମ) ){
      res++;
    }
    ପ୍ରତିଜ୍ଞା( p_ଡାହାଣ==0 || p_ଡାହାଣ->op!=TK_COLUMN
            || (ପଦସମଷ୍ଟି_Use_YTab(p_ଡାହାଣ) && p_ଡାହାଣ->y.pTab!=0) );
    ଯଦି( p_ଡାହାଣ && ପଦସମଷ୍ଟି_Is_Vtab(p_ଡାହାଣ) ){
      res++;
      SWAP(ପଦସମଷ୍ଟି*, p_ବାମ, p_ଡାହାଣ);
    }
    *pp_ବାମ = p_ବାମ;
    *pp_ଡାହାଣ = p_ଡାହାଣ;
    ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    ଫେରନ୍ତୁ res;
  }
  ଫେରନ୍ତୁ 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ସ୍ଥିର ଶୂନ୍ୟ transferJoinMarkings(ପଦସମଷ୍ଟି *pDerived, ପଦସମଷ୍ଟି *pBase){
  ଯଦି( pDerived && ପଦସମଷ୍ଟି_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ସ୍ଥିର ଶୂନ୍ୟ markTermAsChild(WhereClause *pWC, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChild, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  ପ୍ରତିଜ୍ଞା(   pWC->a[iParent].nChild <  UMXV(pWC->a[0].nChild) );
  pWC->a[iParent].nChild++;
  testcase( pWC->a[iParent].nChild == UMXV(pWC->a[0].nChild) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ସ୍ଥିର WhereTerm *whereNthSubterm(WhereTerm *pTerm, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଯଦି( pTerm->eOperator!=WO_AND ){
    ଫେରନ୍ତୁ N==0 ? pTerm : 0;
  }
  ଯଦି( N<pTerm->u.pAndInfo->wc.nTerm ){
    ଫେରନ୍ତୁ &pTerm->u.pAndInfo->wc.a[N];
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ଶୂନ୍ୟ whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  ପଦସମଷ୍ଟି *pNew;            /* New virtual expression */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;                /* Operator for the combined expression */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew;            /* Index in pWC of the next virtual term */
  ପଦସମଷ୍ଟି *pA, *pB;         /* Expressions associated with pOne and pTwo */

  ଯଦି( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) ଫେରନ୍ତୁ;
  ଯଦି( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ଫେରନ୍ତୁ;
  ଯଦି( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ଫେରନ୍ତୁ;
  ଯଦି( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) ଫେରନ୍ତୁ;
  pA = pOne->p_ପଦସମଷ୍ଟି;
  pB = pTwo->p_ପଦସମଷ୍ଟି;
  ପ୍ରତିଜ୍ଞା( pA->p_ବାମ!=0 && pA->p_ଡାହାଣ!=0 );
  ପ୍ରତିଜ୍ଞା( pB->p_ବାମ!=0 && pB->p_ଡାହାଣ!=0 );
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0,pA->p_ବାମ, pB->p_ବାମ, -1) )  ଫେରନ୍ତୁ;
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0,pA->p_ଡାହାଣ, pB->p_ଡାହାଣ,-1) ) ଫେରନ୍ତୁ;
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pA,EP_Commuted)!=ପଦସମଷ୍ଟି_Has_Property(pB,EP_Commuted) ){
    ଫେରନ୍ତୁ;
  }
  /* If we reach this point, it means the two subterms can be combined */
  ଯଦି( (eOp & (eOp-1))!=0 ){
    ଯଦି( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pA, 0);
  ଯଦି( pNew==0 ) ଫେରନ୍ତୁ;
  ପାଇଁ(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ ପ୍ରତିଜ୍ଞା( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_ଭର୍ତ୍ତି(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  ପଦସମଷ୍ଟି_Analyze(pSrc, pWC, idxNew);
}

#ଯଦି !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
ସ୍ଥିର ଶୂନ୍ୟ ପଦସମଷ୍ଟି_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pTerm->p_ପଦସମଷ୍ଟି;             /* The expression of the term */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                                  /* Loop counters */
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
  ପ୍ରତିଜ୍ଞା( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, ଆକାର(*pOrInfo));
  ଯଦି( pOrInfo==0 ) ଫେରନ୍ତୁ;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, ଆକାର(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_ପଦସମଷ୍ଟି, TK_OR);
  sqlite3_Where_ପଦସମଷ୍ଟି_Analyze(pSrc, pOrWc);
  ଯଦି( db->mallocFailed ) ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  ପାଇଁ(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    ଯଦି( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      ପ୍ରତିଜ୍ଞା( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, ଆକାର(*pAndInfo));
      ଯଦି( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->ବାମ_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, ଆକାର(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_ପଦସମଷ୍ଟି, TK_AND);
        sqlite3_Where_ପଦସମଷ୍ଟି_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        ଯଦି( !db->mallocFailed ){
          ପାଇଁ(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            ପ୍ରତିଜ୍ଞା( pAndTerm->p_ପଦସମଷ୍ଟି );
            ଯଦି( allowedOp(pAndTerm->p_ପଦସମଷ୍ଟି->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->ବାମ_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }ଅନ୍ୟଥା ଯଦି( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }ଅନ୍ୟଥା{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->ବାମ_Cursor);
      ଯଦି( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->ବାମ_Cursor);
      }
      indexable &= b;
      ଯଦି( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }ଅନ୍ୟଥା{
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
  pTerm->ବାମ_Cursor = -1;
  ଯଦି( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  ଯଦି( indexable && pOrWc->nTerm==2 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOne = 0;
    WhereTerm *pOne;
    ଯେତେବେଳେ( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTwo = 0;
      WhereTerm *pTwo;
      ଯେତେବେଳେ( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  ଯଦି( chngToIN ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା okToChngToIN = 0;     /* True if the conversion to IN is valid */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iColumn = -1;         /* Column index on lhs of IN operator */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCursor = -1;         /* Table cursor common to all terms */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    ପାଇଁ(j=0; j<2 && !okToChngToIN; j++){
      ପଦସମଷ୍ଟି *p_ବାମ = 0;
      pOrTerm = pOrWc->a;
      ପାଇଁ(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        ପ୍ରତିଜ୍ଞା( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        ଯଦି( pOrTerm->ବାମ_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          ପ୍ରତିଜ୍ଞା( j==1 );
          ଜାରି;
        }
        ଯଦି( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->ବାମ_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          ପ୍ରତିଜ୍ଞା( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          ଜାରି;
        }
        ପ୍ରତିଜ୍ଞା( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.ବାମ_Column;
        iCursor = pOrTerm->ବାମ_Cursor;
        p_ବାମ = pOrTerm->p_ପଦସମଷ୍ଟି->p_ବାମ;
        ଭାଙ୍ଗ;
      }
      ଯଦି( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        ପ୍ରତିଜ୍ଞା( j==1 );
        ପ୍ରତିଜ୍ଞା( IsPowerOfTwo(chngToIN) );
        ପ୍ରତିଜ୍ଞା( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        ଭାଙ୍ଗ;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      ପାଇଁ(; i>=0 && okToChngToIN; i--, pOrTerm++){
        ପ୍ରତିଜ୍ଞା( pOrTerm->eOperator & WO_EQ );
        ପ୍ରତିଜ୍ଞା( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        ଯଦି( pOrTerm->ବାମ_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }ଅନ୍ୟଥା ଯଦି( pOrTerm->u.x.ବାମ_Column!=iColumn || (iColumn==XN_ପଦସମଷ୍ଟି
               && sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, pOrTerm->p_ପଦସମଷ୍ଟି->p_ବାମ, p_ବାମ, -1)
        )){
          okToChngToIN = 0;
        }ଅନ୍ୟଥା{
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା aff_ବାମ, aff_ଡାହାଣ;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Affinity(pOrTerm->p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
          aff_ବାମ = sqlite3_ପଦସମଷ୍ଟି_Affinity(pOrTerm->p_ପଦସମଷ୍ଟି->p_ବାମ);
          ଯଦି( aff_ଡାହାଣ!=0 && aff_ଡାହାଣ!=aff_ବାମ ){
            okToChngToIN = 0;
          }ଅନ୍ୟଥା{
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
    ଯଦି( okToChngToIN ){
      ପଦସମଷ୍ଟି *pDup;            /* A transient duplicate expression */
      ପଦସମଷ୍ଟି_List *pList = 0;   /* The RHS of the IN operator */
      ପଦସମଷ୍ଟି *p_ବାମ = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      ପଦସମଷ୍ଟି *pNew;            /* The complete IN operator */

      ପାଇଁ(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        ପଦସମଷ୍ଟି *pThis;
        ଯଦି( (pOrTerm->wtFlags & TERM_OK)==0 ) ଜାରି;
        ପ୍ରତିଜ୍ଞା( pOrTerm->eOperator & WO_EQ );
        ପ୍ରତିଜ୍ଞା( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        ପ୍ରତିଜ୍ଞା( pOrTerm->ବାମ_Cursor==iCursor );
        ପ୍ରତିଜ୍ଞା( pOrTerm->u.x.ବାମ_Column==iColumn );
        pThis = pOrTerm->p_ପଦସମଷ୍ଟି;
        pDup = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pThis->p_ଡାହାଣ, 0);
        pList = sqlite3_ପଦସମଷ୍ଟି_List_Append(pWInfo->pParse, pList, pDup);
        ଯଦି( p_ବାମ==0 ){
          p_ବାମ = pThis->p_ବାମ;
          pCollSeq = sqlite3_ପଦସମଷ୍ଟି_Compare_Coll_Seq(pParse, pThis);
        }ଅନ୍ୟଥା{
          ପ୍ରତିଜ୍ଞା( 0==sqlite3_ପଦସମଷ୍ଟି_Compare(pParse, 
                        sqlite3_ପଦସମଷ୍ଟି_Skip_Collate(pThis->p_ବାମ),
                        sqlite3_ପଦସମଷ୍ଟି_Skip_Collate(p_ବାମ), -1) );
          ଯଦି( pCollSeq!=sqlite3_ପଦସମଷ୍ଟି_Compare_Coll_Seq(pParse, pThis) ){
            p_ବାମ = 0;  /* Collating sequence mismatch */
            ଭାଙ୍ଗ;
          }
        }
      }
      ଯଦି( p_ବାମ==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }ଅନ୍ୟଥା{
        pDup = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ବାମ, 0);
        ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          ପ୍ରତିଜ୍ଞା( pCollSeq->zName!=0 );
          pDup = sqlite3_ପଦସମଷ୍ଟି_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      ଯଦି( pNew ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew;
        transferJoinMarkings(pNew, p_ପଦସମଷ୍ଟି);
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_ଭର୍ତ୍ତି(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        ପଦସମଷ୍ଟି_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }ଅନ୍ୟଥା{
        sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pList);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା termIsEquivalence(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, SrcList *pSrc){
  ଅକ୍ଷର aff1, aff2;
  ଯଦି( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) ଫେରନ୍ତୁ 0;  /* (1) */
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_EQ && p_ପଦସମଷ୍ଟି->op!=TK_IS ) ଫେରନ୍ତୁ 0;                 /* (2) */
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON|EP_Collate) ) ଫେରନ୍ତୁ 0;        /* (3) */
  ପ୍ରତିଜ୍ଞା( pSrc!=0 );
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    ଫେରନ୍ତୁ 0;                                                          /* (4) */
  }
  aff1 = sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି->p_ବାମ);
  aff2 = sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
  ଯଦି( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    ଫେରନ୍ତୁ 0;                                                          /* (5) */
  }
  ଯଦି( !sqlite3_ପଦସମଷ୍ଟି_Coll_Seq_Match(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ) ){
    ଫେରନ୍ତୁ 0;                                                          /* (6) */
  }
  ଫେରନ୍ତୁ 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ସ୍ଥିର Bitmask ପଦସମଷ୍ଟି_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  ଯେତେବେଳେ( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, pS->pHaving);
    ଯଦି( ALWAYS(pSrc!=0) ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପାଇଁ(i=0; i<pSrc->nSrc; i++){
        ଯଦି( pSrc->a[i].fg.isSubquery ){
          mask |= ପଦସମଷ୍ଟି_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        ଯଦି( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        ଯଦି( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  ଫେରନ୍ତୁ mask;
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
ସ୍ଥିର SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aiCurCol,         /* Write the referenced table cursor and column here */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,           /* An operand of a comparison operator */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur;
  କର{
    iCur = pFrom->a[j].iCursor;
    ପାଇଁ(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ଯଦି( pIdx->a_Col_ପଦସମଷ୍ଟି==0 ) ଜାରି;
      ପାଇଁ(i=0; i<pIdx->n_ଚାବି_Col; i++){
        ଯଦି( pIdx->aiColumn[i]!=XN_ପଦସମଷ୍ଟି ) ଜାରି;
        ପ୍ରତିଜ୍ଞା( pIdx->b_Has_ପଦସମଷ୍ଟି );
        ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare_Skip(p_ପଦସମଷ୍ଟି,pIdx->a_Col_ପଦସମଷ୍ଟି->a[i].p_ପଦସମଷ୍ଟି,iCur)==0
         && !sqlite3_ପଦସମଷ୍ଟି_Is_Constant(0,pIdx->a_Col_ପଦସମଷ୍ଟି->a[i].p_ପଦସମଷ୍ଟି)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_ପଦସମଷ୍ଟି;
          ଫେରନ୍ତୁ 1;
        }
      }
    }
  }ଯେତେବେଳେ( ++j < pFrom->nSrc );
  ଫେରନ୍ତୁ 0;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aiCurCol,         /* Write the referenced table cursor & column here */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,           /* An operand of a comparison operator */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op                 /* The specific comparison operator */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  ପ୍ରତିଜ୍ଞା( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  ପ୍ରତିଜ୍ଞା( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  ପ୍ରତିଜ୍ଞା( op<=TK_GE );
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
    p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->x.pList->a[0].p_ପଦସମଷ୍ଟି;
  }

  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN ){
    aiCurCol[0] = p_ପଦସମଷ୍ଟି->iTable;
    aiCurCol[1] = p_ପଦସମଷ୍ଟି->iColumn;
    ଫେରନ୍ତୁ 1;
  }

  ପାଇଁ(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    ପାଇଁ(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ଯଦି( pIdx->a_Col_ପଦସମଷ୍ଟି ){
        ଫେରନ୍ତୁ ପଦସମଷ୍ଟି_Might_Be_Indexed2(pFrom,aiCurCol,p_ପଦସମଷ୍ଟି,i);
      }
    }
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ଶୂନ୍ୟ ପଦସମଷ୍ଟି_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି;                     /* The expression to be analyzed */
  Bitmask prereq_ବାମ;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_ଡାହାଣ = 0;          /* Extra dependencies on LEFT JOIN */
  ପଦସମଷ୍ଟି *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା noCase = 0;                  /* uppercase equivalent to lowercase */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned ଅକ୍ଷର eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ବାମ;                       /* Number of elements on left side vector */

  ଯଦି( db->mallocFailed ){
    ଫେରନ୍ତୁ;
  }
  ପ୍ରତିଜ୍ଞା( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_ପଦସମଷ୍ଟି = pTerm->p_ପଦସମଷ୍ଟି;
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 ); /* Because malloc() has not failed */
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op!=TK_AS && p_ପଦସମଷ୍ଟି->op!=TK_COLLATE );
ପଦସମଷ୍ଟି_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_ବାମ = sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, p_ପଦସମଷ୍ଟି->p_ବାମ);
  op = p_ପଦସମଷ୍ଟି->op;
  ଯଦି( op==TK_IN ){
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ଡାହାଣ==0 );
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Check_IN(pParse, p_ପଦସମଷ୍ଟି) ) ଫେରନ୍ତୁ;
    ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
      pTerm->prereq_ଡାହାଣ = ପଦସମଷ୍ଟି_Select_Usage(pMaskSet, p_ପଦସମଷ୍ଟି->x.pSelect);
    }ଅନ୍ୟଥା{
      pTerm->prereq_ଡାହାଣ = sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(pMaskSet, p_ପଦସମଷ୍ଟି->x.pList);
    }
    prereqAll = prereq_ବାମ | pTerm->prereq_ଡାହାଣ;
  }ଅନ୍ୟଥା{
    pTerm->prereq_ଡାହାଣ = sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
    ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ==0
     || ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_xIsSelect|EP_IfNullRow)
     || p_ପଦସମଷ୍ଟି->x.pList!=0
    ){
      prereqAll = sqlite3_Where_ପଦସମଷ୍ଟି_Usage_NN(pMaskSet, p_ପଦସମଷ୍ଟି);
    }ଅନ୍ୟଥା{
      prereqAll = prereq_ବାମ | pTerm->prereq_ଡାହାଣ;
    }
  }
  ଯଦି( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  ଯଦି( prereqAll!=sqlite3_Where_ପଦସମଷ୍ଟି_Usage_NN(pMaskSet, p_ପଦସମଷ୍ଟି) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(0,p_ପଦସମଷ୍ଟି,0);
    ପ୍ରତିଜ୍ଞା( 0 );
  }
#endif

  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_ପଦସମଷ୍ଟି->w.iJoin);
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) ){
      prereqAll |= x;
      extra_ଡାହାଣ = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }ଅନ୍ୟଥା ଯଦି( (prereqAll>>1)>=x ){
      ପଦସମଷ୍ଟି_Clear_Property(p_ପଦସମଷ୍ଟି, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->ବାମ_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  ଯଦି( allowedOp(op) ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା aiCurCol[2];
    ପଦସମଷ୍ଟି *p_ବାମ = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate(p_ପଦସମଷ୍ଟି->p_ବାମ);
    ପଦସମଷ୍ଟି *p_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
    u16 opMask = (pTerm->prereq_ଡାହାଣ & prereq_ବାମ)==0 ? WO_ALL : WO_EQUIV;

    ଯଦି( pTerm->u.x.iField>0 ){
      ପ୍ରତିଜ୍ଞା( op==TK_IN );
      ପ୍ରତିଜ୍ଞା( p_ବାମ->op==TK_VECTOR );
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ବାମ) );
      p_ବାମ = p_ବାମ->x.pList->a[pTerm->u.x.iField-1].p_ପଦସମଷ୍ଟି;
    }

    ଯଦି( ପଦସମଷ୍ଟି_Might_Be_Indexed(pSrc, aiCurCol, p_ବାମ, op) ){
      pTerm->ବାମ_Cursor = aiCurCol[0];
      ପ୍ରତିଜ୍ଞା( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.ବାମ_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    ଯଦି( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    ଯଦି( p_ଡାହାଣ
     && ପଦସମଷ୍ଟି_Might_Be_Indexed(pSrc, aiCurCol, p_ଡାହାଣ, op)
     && !ପଦସମଷ୍ଟି_Has_Property(p_ଡାହାଣ, EP_FixedCol)
    ){
      WhereTerm *pNew;
      ପଦସମଷ୍ଟି *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      ପ୍ରତିଜ୍ଞା( pTerm->u.x.iField==0 );
      ଯଦି( pTerm->ବାମ_Cursor>=0 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew;
        pDup = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ପଦସମଷ୍ଟି, 0);
        ଯଦି( db->mallocFailed ){
          sqlite3_ପଦସମଷ୍ଟି_Delete(db, pDup);
          ଫେରନ୍ତୁ;
        }
        idxNew = where_Clause_ଭର୍ତ୍ତି(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        ଯଦି( idxNew==0 ) ଫେରନ୍ତୁ;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        ଯଦି( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        ପ୍ରତିଜ୍ଞା( pWInfo->pTabList!=0 );
        ଯଦି( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }ଅନ୍ୟଥା{
        pDup = p_ପଦସମଷ୍ଟି;
        pNew = pTerm;
      }
      pNew->wtFlags |= ପଦସମଷ୍ଟି_Commute(pParse, pDup);
      pNew->ବାମ_Cursor = aiCurCol[0];
      ପ୍ରତିଜ୍ଞା( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.ବାମ_Column = aiCurCol[1];
      testcase( (prereq_ବାମ | extra_ଡାହାଣ) != prereq_ବାମ );
      pNew->prereq_ଡାହାଣ = prereq_ବାମ | extra_ଡାହାଣ;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }ଅନ୍ୟଥା
    ଯଦି( op==TK_ISNULL
     && !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_OuterON)
     && 0==sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(p_ବାମ)
    ){
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      p_ପଦସମଷ୍ଟି->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_ପଦସମଷ୍ଟି->u.zToken = "false";
      ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_IsFalse);
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
  ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_BETWEEN && pWC->op==TK_AND ){
    ପଦସମଷ୍ଟି_List *pList;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ସ୍ଥିର ଅଚଳ u8 ops[] = {TK_GE, TK_LE};
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
    pList = p_ପଦସମଷ୍ଟି->x.pList;
    ପ୍ରତିଜ୍ଞା( pList!=0 );
    ପ୍ରତିଜ୍ଞା( pList->n_ପଦସମଷ୍ଟି==2 );
    ପ୍ରତିଜ୍ଞା( pWC->a[idxTerm].nChild==0 );
    ପାଇଁ(i=0; i<2; i++){
      ପଦସମଷ୍ଟି *p_New_ପଦସମଷ୍ଟି;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew;
      p_New_ପଦସମଷ୍ଟି = sqlite3PExpr(pParse, ops[i],
                             sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ପଦସମଷ୍ଟି->p_ବାମ, 0),
                             sqlite3_ପଦସମଷ୍ଟି_Dup(db, pList->a[i].p_ପଦସମଷ୍ଟି, 0));
      transferJoinMarkings(p_New_ପଦସମଷ୍ଟି, p_ପଦସମଷ୍ଟି);
      idxNew = where_Clause_ଭର୍ତ୍ତି(pWC, p_New_ପଦସମଷ୍ଟି, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      ପଦସମଷ୍ଟି_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#ଯଦି !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_OR && !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Collate) ){
    ପଦସମଷ୍ଟି *pAlt = sqlite3_ପଦସମଷ୍ଟି_Simplified_And_Or(p_ପଦସମଷ୍ଟି);
    ଯଦି( pAlt!=p_ପଦସମଷ୍ଟି ){
      pTerm->p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(pAlt);
      ଯାଅ ପଦସମଷ୍ଟି_Analyze_restart;
    }
    ପ୍ରତିଜ୍ଞା( pWC->op==TK_AND );
    ପଦସମଷ୍ଟି_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_NOTNULL ){
    ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ->op==TK_COLUMN
     && p_ପଦସମଷ୍ଟି->p_ବାମ->iColumn>=0
     && !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON)
    ){
      ପଦସମଷ୍ଟି *p_New_ପଦସମଷ୍ଟି;
      ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew;
      WhereTerm *pNewTerm;
 
      p_New_ପଦସମଷ୍ଟି = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ବାମ, 0),
                              sqlite3_ପଦସମଷ୍ଟି_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_ଭର୍ତ୍ତି(pWC, p_New_ପଦସମଷ୍ଟି,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      ଯଦି( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_ଡାହାଣ = 0;
        pNewTerm->ବାମ_Cursor = p_ବାମ->iTable;
        pNewTerm->u.x.ବାମ_Column = p_ବାମ->iColumn;
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
  ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_ପଦସମଷ୍ଟି, &pStr1, &isComplete, &noCase)
  ){
    ପଦସମଷ୍ଟି *p_ବାମ;       /* LHS of LIKE/GLOB operator */
    ପଦସମଷ୍ଟି *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    ପଦସମଷ୍ଟି *pNewExpr1;
    ପଦସମଷ୍ଟି *pNewExpr2;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew1;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew2;
    ଅଚଳ ଅକ୍ଷର *zCollSeqName;     /* Name of collating sequence */
    ଅଚଳ u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
    p_ବାମ = p_ପଦସମଷ୍ଟି->x.pList->a[1].p_ପଦସମଷ୍ଟି;
    pStr2 = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pStr1, 0);
    ପ୍ରତିଜ୍ଞା( pStr1==0 || !ପଦସମଷ୍ଟି_Has_Property(pStr1, EP_Int_ମୂଲ୍ୟ) );
    ପ୍ରତିଜ୍ଞା( pStr2==0 || !ପଦସମଷ୍ଟି_Has_Property(pStr2, EP_Int_ମୂଲ୍ୟ) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    ଯଦି( noCase && !pParse->db->mallocFailed ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ଅକ୍ଷର c;
      pTerm->wtFlags |= TERM_LIKE;
      ପାଇଁ(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    ଯଦି( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      ଯଦି( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        ଯଦି( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      ଯେତେବେଳେ( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      ପ୍ରତିଜ୍ଞା( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ଦ୍ୱିମିକ;
    pNewExpr1 = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ବାମ, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_ପଦସମଷ୍ଟି_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_ପଦସମଷ୍ଟି);
    idxNew1 = where_Clause_ଭର୍ତ୍ତି(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ବାମ, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_ପଦସମଷ୍ଟି_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_ପଦସମଷ୍ଟି);
    idxNew2 = where_Clause_ଭର୍ତ୍ତି(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    ପଦସମଷ୍ଟି_Analyze(pSrc, pWC, idxNew1);
    ପଦସମଷ୍ଟି_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    ଯଦି( isComplete ){
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
  ଯଦି( (p_ପଦସମଷ୍ଟି->op==TK_EQ || p_ପଦସମଷ୍ଟି->op==TK_IS)
   && (n_ବାମ = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି->p_ବାମ))>1
   && sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ)==n_ବାମ
   && ( (p_ପଦସମଷ୍ଟି->p_ବାମ->flags & EP_xIsSelect)==0
     || (p_ପଦସମଷ୍ଟି->p_ଡାହାଣ->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<n_ବାମ; i++){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew;
      ପଦସମଷ୍ଟି *pNew;
      ପଦସମଷ୍ଟି *p_ବାମ = sqlite3_ପଦସମଷ୍ଟି_For_Vector_Field(pParse, p_ପଦସମଷ୍ଟି->p_ବାମ, i, n_ବାମ);
      ପଦସମଷ୍ଟି *p_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_For_Vector_Field(pParse, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, i, n_ବାମ);

      pNew = sqlite3PExpr(pParse, p_ପଦସମଷ୍ଟି->op, p_ବାମ, p_ଡାହାଣ);
      transferJoinMarkings(pNew, p_ପଦସମଷ୍ଟି);
      idxNew = where_Clause_ଭର୍ତ୍ତି(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      ପଦସମଷ୍ଟି_Analyze(pSrc, pWC, idxNew);
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
  ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_IN
   && pTerm->u.x.iField==0
   && p_ପଦସମଷ୍ଟି->p_ବାମ->op==TK_VECTOR
   && ALWAYS( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) )
   && (p_ପଦସମଷ୍ଟି->x.pSelect->pPrior==0 || (p_ପଦସମଷ୍ଟି->x.pSelect->selFlags & SF_ମୂଲ୍ୟଗୁଡ଼ିକ))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_ପଦସମଷ୍ଟି->x.pSelect->pWin==0
#endif
   && (p_ପଦସମଷ୍ଟି->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_ପଦସମଷ୍ଟି->x.pSelect->pEList->n_ପଦସମଷ୍ଟି <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପ୍ରତିଜ୍ଞା( pTerm->nChild==0 );
    ପାଇଁ(i=0; i<sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି->p_ବାମ); i++){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew;
      idxNew = where_Clause_ଭର୍ତ୍ତି(pWC, p_ପଦସମଷ୍ଟି, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      ପଦସମଷ୍ଟି_Analyze(pSrc, pWC, idxNew);
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
  ଅନ୍ୟଥା ଯଦି( pWC->op==TK_AND ){
    ପଦସମଷ୍ଟି *p_ଡାହାଣ = 0, *p_ବାମ = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = isAuxiliaryVtabOperator(db, p_ପଦସମଷ୍ଟି, &eOp2, &p_ବାମ, &p_ଡାହାଣ);
    ଯେତେବେଳେ( res-- > 0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_ପଦସମଷ୍ଟି;

      prereq_ପଦସମଷ୍ଟି = sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, p_ଡାହାଣ);
      prereqColumn = sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, p_ବାମ);
      ଯଦି( (prereq_ପଦସମଷ୍ଟି & prereqColumn)==0 ){
        ପଦସମଷ୍ଟି *p_New_ପଦସମଷ୍ଟି;
        p_New_ପଦସମଷ୍ଟି = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ଡାହାଣ, 0));
        ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) && p_New_ପଦସମଷ୍ଟି ){
          ପଦସମଷ୍ଟି_Set_Property(p_New_ପଦସମଷ୍ଟି, EP_OuterON);
          p_New_ପଦସମଷ୍ଟି->w.iJoin = p_ପଦସମଷ୍ଟି->w.iJoin;
        }
        idxNew = where_Clause_ଭର୍ତ୍ତି(pWC, p_New_ପଦସମଷ୍ଟି, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_ଡାହାଣ = prereq_ପଦସମଷ୍ଟି | extra_ଡାହାଣ;
        pNewTerm->ବାମ_Cursor = p_ବାମ->iTable;
        pNewTerm->u.x.ବାମ_Column = p_ବାମ->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(ପଦସମଷ୍ଟି*, p_ବାମ, p_ଡାହାଣ);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_ଡାହାଣ |= extra_ଡାହାଣ;
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
ଶୂନ୍ୟ sqlite3WhereSplit(WhereClause *pWC, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, u8 op){
  ପଦସମଷ୍ଟି *pE2 = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(p_ପଦସମଷ୍ଟି);
  pWC->op = op;
  ପ୍ରତିଜ୍ଞା( pE2!=0 || p_ପଦସମଷ୍ଟି==0 );
  ଯଦି( pE2==0 ) ଫେରନ୍ତୁ;
  ଯଦି( pE2->op!=op ){
    where_Clause_ଭର୍ତ୍ତି(pWC, p_ପଦସମଷ୍ଟି, 0);
  }ଅନ୍ୟଥା{
    sqlite3WhereSplit(pWC, pE2->p_ବାମ, op);
    sqlite3WhereSplit(pWC, pE2->p_ଡାହାଣ, op);
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
ସ୍ଥିର ଶୂନ୍ୟ where_Add_Limit_ପଦସମଷ୍ଟି(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iReg,           /* Register that will hold value of the limit/offset */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,        /* Expression that defines the limit/offset */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCsr,           /* Cursor to which the constraint applies */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  ପଦସମଷ୍ଟି *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iVal = 0;

  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Integer(p_ପଦସମଷ୍ଟି, &iVal, pParse, 0) && iVal>=0 ){
    ପଦସମଷ୍ଟି *pVal = sqlite3_ପଦସମଷ୍ଟି_Int32(db, iVal);
    ଯଦି( pVal==0 ) ଫେରନ୍ତୁ;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }ଅନ୍ୟଥା{
    ପଦସମଷ୍ଟି *pVal = sqlite3_ପଦସମଷ୍ଟି_Alloc(db, TK_REGISTER, 0, 0);
    ଯଦି( pVal==0 ) ଫେରନ୍ତୁ;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  ଯଦି( pNew ){
    WhereTerm *pTerm;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;
    idx = where_Clause_ଭର୍ତ୍ତି(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->ବାମ_Cursor = iCsr;
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
ଶୂନ୍ୟ SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  ପ୍ରତିଜ୍ଞା( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  ଯଦି( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    ପଦସମଷ୍ଟି_List *pOrderBy = p->pOrderBy;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCsr = p->pSrc->a[0].iCursor;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;

    /* Check condition (4). Return early if it is not met. */
    ପାଇଁ(ii=0; ii<pWC->nTerm; ii++){
      ଯଦି( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        ପ୍ରତିଜ୍ଞା( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        ପ୍ରତିଜ୍ଞା( pWC->a[ii].eOperator==WO_ROWVAL );
        ଜାରି;
      }
      ଯଦି( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        ଜାରି;
      }
      ଯଦି( pWC->a[ii].ବାମ_Cursor==iCsr && pWC->a[ii].prereq_ଡାହାଣ==0 ) ଜାରି;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      ଯଦି( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        ଯଦି( pParent->ବାମ_Cursor==iCsr
         && pParent->prereq_ଡାହାଣ==0
         && pParent->nChild==1
        ){
          ଜାରି;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      ଫେରନ୍ତୁ;
    }

    /* Check condition (5). Return early if it is not met. */
    ଯଦି( pOrderBy ){
      ପାଇଁ(ii=0; ii<pOrderBy->n_ପଦସମଷ୍ଟି; ii++){
        ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pOrderBy->a[ii].p_ପଦସମଷ୍ଟି;
        ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_COLUMN ) ଫେରନ୍ତୁ;
        ଯଦି( p_ପଦସମଷ୍ଟି->iTable!=iCsr ) ଫେରନ୍ତୁ;
        ଯଦି( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) ଫେରନ୍ତୁ;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    ପ୍ରତିଜ୍ଞା( p->pLimit->op==TK_LIMIT );
    ଯଦି( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_ପଦସମଷ୍ଟି(pWC, p->iOffset, p->pLimit->p_ଡାହାଣ,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    ଯଦି( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_ପଦସମଷ୍ଟି(pWC, p->iLimit, p->pLimit->p_ବାମ,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
ଶୂନ୍ୟ sqlite3WhereClauseInit(
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
ଶୂନ୍ୟ sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  ପ୍ରତିଜ୍ଞା( pWC->nTerm>=pWC->nBase );
  ଯଦି( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    /* Verify that every term past pWC->nBase is virtual */
    ପାଇଁ(i=pWC->nBase; i<pWC->nTerm; i++){
      ପ୍ରତିଜ୍ଞା( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    ଯେତେବେଳେ(1){
      ପ୍ରତିଜ୍ଞା( a->eMatchOp==0 || a->eOperator==WO_AUX );
      ଯଦି( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_ପଦସମଷ୍ଟି_Delete(db, a->p_ପଦସମଷ୍ଟି);
      }
      ଯଦି( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        ଯଦି( a->wtFlags & TERM_ORINFO ){
          ପ୍ରତିଜ୍ଞା( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }ଅନ୍ୟଥା{
          ପ୍ରତିଜ୍ଞା( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      ଯଦି( a==aLast ) ଭାଙ୍ଗ;
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
ସ୍ଥିର SQLITE_NOINLINE Bitmask sqlite3_Where_ପଦସମଷ୍ଟି_Usage_Full(
  WhereMaskSet *pMaskSet,
  ପଦସମଷ୍ଟି *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  ଯଦି( p->p_ବାମ ) mask |= sqlite3_Where_ପଦସମଷ୍ଟି_Usage_NN(pMaskSet, p->p_ବାମ);
  ଯଦି( p->p_ଡାହାଣ ){
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_Usage_NN(pMaskSet, p->p_ଡାହାଣ);
    ପ୍ରତିଜ୍ଞା( p->x.pList==0 );
  }ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p) ){
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= ପଦସମଷ୍ଟି_Select_Usage(pMaskSet, p->x.pSelect);
  }ଅନ୍ୟଥା ଯଦି( p->x.pList ){
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  ଯଦି( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && ପଦସମଷ୍ଟି_Use_YWin(p) ){
    ପ୍ରତିଜ୍ଞା( p->y.pWin!=0 );
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  ଫେରନ୍ତୁ mask;
}
Bitmask sqlite3_Where_ପଦସମଷ୍ଟି_Usage_NN(WhereMaskSet *pMaskSet, ପଦସମଷ୍ଟି *p){
  ଯଦି( p->op==TK_COLUMN && !ପଦସମଷ୍ଟି_Has_Property(p, EP_FixedCol) ){
    ଫେରନ୍ତୁ sqlite3WhereGetMask(pMaskSet, p->iTable);
  }ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    ପ୍ରତିଜ୍ଞା( p->op!=TK_IF_NULL_ROW );
    ଫେରନ୍ତୁ 0;
  }
  ଫେରନ୍ତୁ sqlite3_Where_ପଦସମଷ୍ଟି_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_ପଦସମଷ୍ଟି_Usage(WhereMaskSet *pMaskSet, ପଦସମଷ୍ଟି *p){
  ଫେରନ୍ତୁ p ? sqlite3_Where_ପଦସମଷ୍ଟି_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_ପଦସମଷ୍ଟି_List_Usage(WhereMaskSet *pMaskSet, ପଦସମଷ୍ଟି_List *pList){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Bitmask mask = 0;
  ଯଦି( pList ){
    ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
      mask |= sqlite3_Where_ପଦସମଷ୍ଟି_Usage(pMaskSet, pList->a[i].p_ପଦସମଷ୍ଟି);
    }
  }
  ଫେରନ୍ତୁ mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
ଶୂନ୍ୟ sqlite3_Where_ପଦସମଷ୍ଟି_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=pWC->nTerm-1; i>=0; i--){
    ପଦସମଷ୍ଟି_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
ଶୂନ୍ୟ sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା j, k;
  ପଦସମଷ୍ଟି_List *pArgs;
  ପଦସମଷ୍ଟି *pColRef;
  ପଦସମଷ୍ଟି *pTerm;
  ଯଦି( pItem->fg.isTabFunc==0 ) ଫେରନ୍ତୁ;
  pTab = pItem->pSTab;
  ପ୍ରତିଜ୍ଞା( pTab!=0 );
  ଯଦି( !IsVirtual(pTab) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "'%s' is not a function", pItem->zName);
    ଫେରନ୍ତୁ;
  }
  pArgs = pItem->u1.pFuncArg;
  ଯଦି( pArgs==0 ) ଫେରନ୍ତୁ;
  ପାଇଁ(j=k=0; j<pArgs->n_ପଦସମଷ୍ଟି; j++){
    ପଦସମଷ୍ଟି *pRhs;
    u32 joinType;
    ଯେତେବେଳେ( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    ଯଦି( k>=pTab->nCol ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      ଫେରନ୍ତୁ;
    }
    pColRef = sqlite3_ପଦସମଷ୍ଟି_Alloc(pParse->db, TK_COLUMN, 0, 0);
    ଯଦି( pColRef==0 ) ଫେରନ୍ତୁ;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_ପଦସମଷ୍ଟି_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_ପଦସମଷ୍ଟି_Dup(pParse->db, pArgs->a[j].p_ପଦସମଷ୍ଟି, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    ଯଦି( pItem->fg.jointype & (JT_ବାମ|JT_ଡାହାଣ) ){
      testcase( pItem->fg.jointype & JT_ବାମ );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_ଡାହାଣ ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }ଅନ୍ୟଥା{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_ପଦସମଷ୍ଟି(pTerm, pItem->iCursor, joinType);
    where_Clause_ଭର୍ତ୍ତି(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3WhereLoopBloomable(ଅଚଳ WhereLoop *pLoop){
  ଯଦି( pLoop->wsFlags & WHERE_IPK ) ଫେରନ୍ତୁ 1;
  ଯଦି( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
