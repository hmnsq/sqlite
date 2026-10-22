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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "whereInt.h"

/* Forward declarations */
स्थिर शून्य व्यंजक_Analyze(SrcList*, WhereClause*, पूर्णाङ्क);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
स्थिर शून्य whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
स्थिर शून्य whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
स्थिर पूर्णाङ्क where_Clause_जोड़ो(WhereClause *pWC, व्यंजक *p, u16 wtFlags){
  WhereTerm *pTerm;
  पूर्णाङ्क idx;
  testcase( wtFlags & TERM_VIRTUAL );
  यदि( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, परिमाण(pWC->a[0])*pWC->nSlot*2 );
    यदि( pWC->a==0 ){
      यदि( wtFlags & TERM_DYNAMIC ){
        sqlite3_व्यंजक_Delete(db, p);
      }
      pWC->a = pOld;
      प्रत्यागम 0;
    }
    memcpy(pWC->a, pOld, परिमाण(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  यदि( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  यदि( p && व्यंजक_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }अन्यथा{
    pTerm->truthProb = 1;
  }
  pTerm->p_व्यंजक = sqlite3_व्यंजक_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         परिमाण(WhereTerm) - offsetof(WhereTerm,eOperator));
  प्रत्यागम idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
स्थिर पूर्णाङ्क allowedOp(पूर्णाङ्क op){
  प्रतिज्ञा( TK_GT>TK_EQ && TK_GT<TK_GE );
  प्रतिज्ञा( TK_LT>TK_EQ && TK_LT<TK_GE );
  प्रतिज्ञा( TK_LE>TK_EQ && TK_LE<TK_GE );
  प्रतिज्ञा( TK_GE==TK_EQ+4 );
  प्रतिज्ञा( TK_IN<TK_EQ );
  प्रतिज्ञा( TK_IS<TK_EQ );
  प्रतिज्ञा( TK_ISNULL<TK_EQ );
  यदि( op>TK_GE ) प्रत्यागम 0;
  यदि( op>=TK_EQ ) प्रत्यागम 1;
  प्रत्यागम op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
स्थिर u16 व्यंजक_Commute(Parse *pParse, व्यंजक *p_व्यंजक){
  यदि( p_व्यंजक->p_बायाँ->op==TK_VECTOR
   || p_व्यंजक->p_दायाँ->op==TK_VECTOR
   || sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, p_व्यंजक->p_बायाँ, p_व्यंजक->p_दायाँ) !=
      sqlite3_द्विआधारी_Compare_Coll_Seq(pParse, p_व्यंजक->p_दायाँ, p_व्यंजक->p_बायाँ)
  ){
    p_व्यंजक->flags ^= EP_Commuted;
  }
  SWAP(व्यंजक*,p_व्यंजक->p_दायाँ,p_व्यंजक->p_बायाँ);
  यदि( p_व्यंजक->op>=TK_GT ){
    प्रतिज्ञा( TK_LT==TK_GT+2 );
    प्रतिज्ञा( TK_GE==TK_LE+2 );
    प्रतिज्ञा( TK_GT>TK_EQ );
    प्रतिज्ञा( TK_GT<TK_LE );
    प्रतिज्ञा( p_व्यंजक->op>=TK_GT && p_व्यंजक->op<=TK_GE );
    p_व्यंजक->op = ((p_व्यंजक->op-TK_GT)^2)+TK_GT;
  }
  प्रत्यागम 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
स्थिर u16 operatorMask(पूर्णाङ्क op){
  u16 c;
  प्रतिज्ञा( allowedOp(op) );
  यदि( op>=TK_EQ ){
    प्रतिज्ञा( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }अन्यथा यदि( op==TK_IN ){
    c = WO_IN;
  }अन्यथा यदि( op==TK_ISNULL ){
    c = WO_ISNULL;
  }अन्यथा{
    प्रतिज्ञा( op==TK_IS );
    c = WO_IS;
  }
  प्रतिज्ञा( op!=TK_ISNULL || c==WO_ISNULL );
  प्रतिज्ञा( op!=TK_IN || c==WO_IN );
  प्रतिज्ञा( op!=TK_EQ || c==WO_EQ );
  प्रतिज्ञा( op!=TK_LT || c==WO_LT );
  प्रतिज्ञा( op!=TK_LE || c==WO_LE );
  प्रतिज्ञा( op!=TK_GT || c==WO_GT );
  प्रतिज्ञा( op!=TK_GE || c==WO_GE );
  प्रतिज्ञा( op!=TK_IS || c==WO_IS );
  प्रत्यागम c;
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
स्थिर पूर्णाङ्क isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  व्यंजक *p_व्यंजक,      /* Test this expression */
  व्यंजक **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  पूर्णाङ्क *pisComplete, /* True if the only wildcard is % in the last character */
  पूर्णाङ्क *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ध्रुव u8 *z = 0;           /* String on RHS of LIKE operator */
  व्यंजक *p_दायाँ, *p_बायाँ;      /* Right and left size of LIKE operator */
  व्यंजक_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  पूर्णाङ्क cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_मूल्य *pVal = 0;
  पूर्णाङ्क op;                    /* Opcode of pRight */
  पूर्णाङ्क rc;                    /* Result code to return */

  यदि( !sqlite3IsLikeFunction(db, p_व्यंजक, pnoCase, (वर्ण*)wc) ){
    प्रत्यागम 0;
  }
#ifdef SQLITE_EBCDIC
  यदि( *pnoCase ) प्रत्यागम 0;
#endif
  प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
  pList = p_व्यंजक->x.pList;
  p_बायाँ = pList->a[1].p_व्यंजक;

  p_दायाँ = sqlite3_व्यंजक_Skip_Collate(pList->a[0].p_व्यंजक);
  op = p_दायाँ->op;
  यदि( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    पूर्णाङ्क iCol = p_दायाँ->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_मूल्य(pReprepare, iCol, SQLITE_AFF_BLOB);
    यदि( pVal && sqlite3_मूल्य_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_मूल्य_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    प्रतिज्ञा( p_दायाँ->op==TK_VARIABLE || p_दायाँ->op==TK_REGISTER );
  }अन्यथा यदि( op==TK_STRING ){
    प्रतिज्ञा( !व्यंजक_Has_Property(p_दायाँ, EP_Int_मूल्य) );
     z = (u8*)p_दायाँ->u.zToken;
  }
  यदि( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    यावत्( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      यदि( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }अन्यथा यदि( c>=0x80 ){
        ध्रुव u8 *z2 = z+cnt-1;
        यदि( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          भङ्ग;
        }अन्यथा{
          cnt = (पूर्णाङ्क)(z2-z);
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
    यदि( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      व्यंजक *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_व्यंजक(db, TK_STRING, (वर्ण*)z);
      यदि( pPrefix ){
        पूर्णाङ्क iFrom, iTo;
        वर्ण *zNew;
        प्रतिज्ञा( !व्यंजक_Has_Property(pPrefix, EP_Int_मूल्य) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        प्रति(iFrom=iTo=0; iFrom<cnt; iFrom++){
          यदि( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;
        प्रतिज्ञा( iTo>0 );

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
        यदि( p_बायाँ->op!=TK_COLUMN
         || sqlite3_व्यंजक_Affinity(p_बायाँ)!=SQLITE_AFF_TEXT
         || (ALWAYS( व्यंजक_Use_YTab(p_बायाँ) )
             && ALWAYS(p_बायाँ->y.pTab)
             && IsVirtual(p_बायाँ->y.pTab))  /* Might be numeric */
        ){
          पूर्णाङ्क is_अंक;
          द्विगुण rDummy;
          प्रतिज्ञा( zNew[iTo]==0 );
          is_अंक = sqlite3AtoF(zNew, &rDummy);
          यदि( is_अंक<=0 ){
            यदि( iTo==1 && zNew[0]=='-' ){
              is_अंक = +1;
            }अन्यथा{
              zNew[iTo-1]++;
              is_अंक = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          यदि( is_अंक>0 ){
            sqlite3_व्यंजक_Delete(db, pPrefix);
            sqlite3_मूल्य_Free(pVal);
            प्रत्यागम 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      यदि( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_दायाँ->iColumn, 0);
        प्रतिज्ञा( !व्यंजक_Has_Property(p_दायाँ, EP_Int_मूल्य) );
        यदि( *pisComplete && p_दायाँ->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          पूर्णाङ्क r1 = sqlite3GetTempReg(pParse);
          sqlite3_व्यंजक_Code_Target(pParse, p_दायाँ, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }अन्यथा{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_मूल्य_Free(pVal);
  प्रत्यागम rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
पूर्णाङ्क sqlite3_व्यंजक_Is_Like_Operator(ध्रुव व्यंजक *p_व्यंजक){
  स्थिर ध्रुव संरचना {
    ध्रुव वर्ण *zOp;
    unsigned वर्ण eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  पूर्णाङ्क i;
  प्रतिज्ञा( p_व्यंजक->op==TK_FUNCTION );
  प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
  प्रति(i=0; i<ArraySize(aOp); i++){
    यदि( sqlite3StrICmp(p_व्यंजक->u.zToken, aOp[i].zOp)==0 ){
      प्रत्यागम aOp[i].eOp;
    }
  }
  प्रत्यागम 0;
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
स्थिर पूर्णाङ्क isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  व्यंजक *p_व्यंजक,                    /* Test this expression */
  unsigned वर्ण *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  व्यंजक **pp_बायाँ,                  /* Column expression to left of MATCH/op2 */
  व्यंजक **pp_दायाँ                  /* Expression to left of MATCH/op2 */
){
  यदि( p_व्यंजक->op==TK_FUNCTION ){
    व्यंजक_List *pList;
    व्यंजक *pCol;                     /* Column reference */
    पूर्णाङ्क i;

    प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
    pList = p_व्यंजक->x.pList;
    यदि( pList==0 || pList->n_व्यंजक!=2 ){
      प्रत्यागम 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_व्यंजक;
    प्रतिज्ञा( pCol->op!=TK_COLUMN || (व्यंजक_Use_YTab(pCol) && pCol->y.pTab!=0) );
    यदि( व्यंजक_Is_Vtab(pCol) && (i = sqlite3_व्यंजक_Is_Like_Operator(p_व्यंजक))!=0 ){
      *peOp2 = i;
      *pp_दायाँ = pList->a[0].p_व्यंजक;
      *pp_बायाँ = pCol;
      प्रत्यागम 1;
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
    pCol = pList->a[0].p_व्यंजक;
    प्रतिज्ञा( pCol->op!=TK_COLUMN || व्यंजक_Use_YTab(pCol) );
    प्रतिज्ञा( pCol->op!=TK_COLUMN || (व्यंजक_Use_YTab(pCol) && pCol->y.pTab!=0) );
    यदि( व्यंजक_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      शून्य (*xNotUsed)(sqlite3_context*,पूर्णाङ्क,sqlite3_मूल्य**);
      शून्य *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      प्रतिज्ञा( pVtab!=0 );
      प्रतिज्ञा( pVtab->pModule!=0 );
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      pMod = (sqlite3_module *)pVtab->pModule;
      यदि( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_व्यंजक->u.zToken, &xNotUsed, &pNotUsed);
        यदि( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_दायाँ = pList->a[1].p_व्यंजक;
          *pp_बायाँ = pCol;
          प्रत्यागम 1;
        }
      }
    }
  }अन्यथा यदि( p_व्यंजक->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    प्रतिज्ञा( TK_NE < TK_EQ );
    प्रतिज्ञा( TK_ISNOT < TK_EQ );
    प्रतिज्ञा( TK_NOTNULL < TK_EQ );
    प्रत्यागम 0;
  }अन्यथा यदि( p_व्यंजक->op==TK_NE || p_व्यंजक->op==TK_ISNOT || p_व्यंजक->op==TK_NOTNULL ){
    पूर्णाङ्क res = 0;
    व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
    व्यंजक *p_दायाँ = p_व्यंजक->p_दायाँ;
    प्रतिज्ञा( p_बायाँ->op!=TK_COLUMN || (व्यंजक_Use_YTab(p_बायाँ) && p_बायाँ->y.pTab!=0) );
    यदि( व्यंजक_Is_Vtab(p_बायाँ) ){
      res++;
    }
    प्रतिज्ञा( p_दायाँ==0 || p_दायाँ->op!=TK_COLUMN
            || (व्यंजक_Use_YTab(p_दायाँ) && p_दायाँ->y.pTab!=0) );
    यदि( p_दायाँ && व्यंजक_Is_Vtab(p_दायाँ) ){
      res++;
      SWAP(व्यंजक*, p_बायाँ, p_दायाँ);
    }
    *pp_बायाँ = p_बायाँ;
    *pp_दायाँ = p_दायाँ;
    यदि( p_व्यंजक->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    यदि( p_व्यंजक->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    यदि( p_व्यंजक->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    प्रत्यागम res;
  }
  प्रत्यागम 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
स्थिर शून्य transferJoinMarkings(व्यंजक *pDerived, व्यंजक *pBase){
  यदि( pDerived && व्यंजक_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
स्थिर शून्य markTermAsChild(WhereClause *pWC, पूर्णाङ्क iChild, पूर्णाङ्क iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  प्रतिज्ञा(   pWC->a[iParent].nChild <  UMXV(pWC->a[0].nChild) );
  pWC->a[iParent].nChild++;
  testcase( pWC->a[iParent].nChild == UMXV(pWC->a[0].nChild) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
स्थिर WhereTerm *whereNthSubterm(WhereTerm *pTerm, पूर्णाङ्क N){
  यदि( pTerm->eOperator!=WO_AND ){
    प्रत्यागम N==0 ? pTerm : 0;
  }
  यदि( N<pTerm->u.pAndInfo->wc.nTerm ){
    प्रत्यागम &pTerm->u.pAndInfo->wc.a[N];
  }
  प्रत्यागम 0;
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
स्थिर शून्य whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  व्यंजक *pNew;            /* New virtual expression */
  पूर्णाङ्क op;                /* Operator for the combined expression */
  पूर्णाङ्क idxNew;            /* Index in pWC of the next virtual term */
  व्यंजक *pA, *pB;         /* Expressions associated with pOne and pTwo */

  यदि( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) प्रत्यागम;
  यदि( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) प्रत्यागम;
  यदि( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) प्रत्यागम;
  यदि( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) प्रत्यागम;
  pA = pOne->p_व्यंजक;
  pB = pTwo->p_व्यंजक;
  प्रतिज्ञा( pA->p_बायाँ!=0 && pA->p_दायाँ!=0 );
  प्रतिज्ञा( pB->p_बायाँ!=0 && pB->p_दायाँ!=0 );
  यदि( sqlite3_व्यंजक_Compare(0,pA->p_बायाँ, pB->p_बायाँ, -1) )  प्रत्यागम;
  यदि( sqlite3_व्यंजक_Compare(0,pA->p_दायाँ, pB->p_दायाँ,-1) ) प्रत्यागम;
  यदि( व्यंजक_Has_Property(pA,EP_Commuted)!=व्यंजक_Has_Property(pB,EP_Commuted) ){
    प्रत्यागम;
  }
  /* If we reach this point, it means the two subterms can be combined */
  यदि( (eOp & (eOp-1))!=0 ){
    यदि( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }अन्यथा{
      प्रतिज्ञा( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_व्यंजक_Dup(db, pA, 0);
  यदि( pNew==0 ) प्रत्यागम;
  प्रति(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ प्रतिज्ञा( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_जोड़ो(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  व्यंजक_Analyze(pSrc, pWC, idxNew);
}

#यदि !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
स्थिर शून्य व्यंजक_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  पूर्णाङ्क idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  व्यंजक *p_व्यंजक = pTerm->p_व्यंजक;             /* The expression of the term */
  पूर्णाङ्क i;                                  /* Loop counters */
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
  प्रतिज्ञा( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  प्रतिज्ञा( p_व्यंजक->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, परिमाण(*pOrInfo));
  यदि( pOrInfo==0 ) प्रत्यागम;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, परिमाण(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_व्यंजक, TK_OR);
  sqlite3_Where_व्यंजक_Analyze(pSrc, pOrWc);
  यदि( db->mallocFailed ) प्रत्यागम;
  प्रतिज्ञा( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  प्रति(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    यदि( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      प्रतिज्ञा( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, परिमाण(*pAndInfo));
      यदि( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        पूर्णाङ्क j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->बायाँ_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, परिमाण(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_व्यंजक, TK_AND);
        sqlite3_Where_व्यंजक_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        यदि( !db->mallocFailed ){
          प्रति(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            प्रतिज्ञा( pAndTerm->p_व्यंजक );
            यदि( allowedOp(pAndTerm->p_व्यंजक->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->बायाँ_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }अन्यथा यदि( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }अन्यथा{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->बायाँ_Cursor);
      यदि( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->बायाँ_Cursor);
      }
      indexable &= b;
      यदि( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }अन्यथा{
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
  pTerm->बायाँ_Cursor = -1;
  यदि( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  यदि( indexable && pOrWc->nTerm==2 ){
    पूर्णाङ्क iOne = 0;
    WhereTerm *pOne;
    यावत्( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      पूर्णाङ्क iTwo = 0;
      WhereTerm *pTwo;
      यावत्( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  यदि( chngToIN ){
    पूर्णाङ्क okToChngToIN = 0;     /* True if the conversion to IN is valid */
    पूर्णाङ्क iColumn = -1;         /* Column index on lhs of IN operator */
    पूर्णाङ्क iCursor = -1;         /* Table cursor common to all terms */
    पूर्णाङ्क j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    प्रति(j=0; j<2 && !okToChngToIN; j++){
      व्यंजक *p_बायाँ = 0;
      pOrTerm = pOrWc->a;
      प्रति(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        प्रतिज्ञा( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        यदि( pOrTerm->बायाँ_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          प्रतिज्ञा( j==1 );
          अनुवर्त;
        }
        यदि( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->बायाँ_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          प्रतिज्ञा( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          अनुवर्त;
        }
        प्रतिज्ञा( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.बायाँ_Column;
        iCursor = pOrTerm->बायाँ_Cursor;
        p_बायाँ = pOrTerm->p_व्यंजक->p_बायाँ;
        भङ्ग;
      }
      यदि( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        प्रतिज्ञा( j==1 );
        प्रतिज्ञा( IsPowerOfTwo(chngToIN) );
        प्रतिज्ञा( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        भङ्ग;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      प्रति(; i>=0 && okToChngToIN; i--, pOrTerm++){
        प्रतिज्ञा( pOrTerm->eOperator & WO_EQ );
        प्रतिज्ञा( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        यदि( pOrTerm->बायाँ_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }अन्यथा यदि( pOrTerm->u.x.बायाँ_Column!=iColumn || (iColumn==XN_व्यंजक
               && sqlite3_व्यंजक_Compare(pParse, pOrTerm->p_व्यंजक->p_बायाँ, p_बायाँ, -1)
        )){
          okToChngToIN = 0;
        }अन्यथा{
          पूर्णाङ्क aff_बायाँ, aff_दायाँ;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_दायाँ = sqlite3_व्यंजक_Affinity(pOrTerm->p_व्यंजक->p_दायाँ);
          aff_बायाँ = sqlite3_व्यंजक_Affinity(pOrTerm->p_व्यंजक->p_बायाँ);
          यदि( aff_दायाँ!=0 && aff_दायाँ!=aff_बायाँ ){
            okToChngToIN = 0;
          }अन्यथा{
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
    यदि( okToChngToIN ){
      व्यंजक *pDup;            /* A transient duplicate expression */
      व्यंजक_List *pList = 0;   /* The RHS of the IN operator */
      व्यंजक *p_बायाँ = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      व्यंजक *pNew;            /* The complete IN operator */

      प्रति(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        व्यंजक *pThis;
        यदि( (pOrTerm->wtFlags & TERM_OK)==0 ) अनुवर्त;
        प्रतिज्ञा( pOrTerm->eOperator & WO_EQ );
        प्रतिज्ञा( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        प्रतिज्ञा( pOrTerm->बायाँ_Cursor==iCursor );
        प्रतिज्ञा( pOrTerm->u.x.बायाँ_Column==iColumn );
        pThis = pOrTerm->p_व्यंजक;
        pDup = sqlite3_व्यंजक_Dup(db, pThis->p_दायाँ, 0);
        pList = sqlite3_व्यंजक_List_Append(pWInfo->pParse, pList, pDup);
        यदि( p_बायाँ==0 ){
          p_बायाँ = pThis->p_बायाँ;
          pCollSeq = sqlite3_व्यंजक_Compare_Coll_Seq(pParse, pThis);
        }अन्यथा{
          प्रतिज्ञा( 0==sqlite3_व्यंजक_Compare(pParse, 
                        sqlite3_व्यंजक_Skip_Collate(pThis->p_बायाँ),
                        sqlite3_व्यंजक_Skip_Collate(p_बायाँ), -1) );
          यदि( pCollSeq!=sqlite3_व्यंजक_Compare_Coll_Seq(pParse, pThis) ){
            p_बायाँ = 0;  /* Collating sequence mismatch */
            भङ्ग;
          }
        }
      }
      यदि( p_बायाँ==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }अन्यथा{
        pDup = sqlite3_व्यंजक_Dup(db, p_बायाँ, 0);
        यदि( sqlite3_व्यंजक_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          प्रतिज्ञा( pCollSeq->zName!=0 );
          pDup = sqlite3_व्यंजक_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      यदि( pNew ){
        पूर्णाङ्क idxNew;
        transferJoinMarkings(pNew, p_व्यंजक);
        प्रतिज्ञा( व्यंजक_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_जोड़ो(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        व्यंजक_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }अन्यथा{
        sqlite3_व्यंजक_List_Delete(db, pList);
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
स्थिर पूर्णाङ्क termIsEquivalence(Parse *pParse, व्यंजक *p_व्यंजक, SrcList *pSrc){
  वर्ण aff1, aff2;
  यदि( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) प्रत्यागम 0;  /* (1) */
  यदि( p_व्यंजक->op!=TK_EQ && p_व्यंजक->op!=TK_IS ) प्रत्यागम 0;                 /* (2) */
  यदि( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON|EP_Collate) ) प्रत्यागम 0;        /* (3) */
  प्रतिज्ञा( pSrc!=0 );
  यदि( p_व्यंजक->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    प्रत्यागम 0;                                                          /* (4) */
  }
  aff1 = sqlite3_व्यंजक_Affinity(p_व्यंजक->p_बायाँ);
  aff2 = sqlite3_व्यंजक_Affinity(p_व्यंजक->p_दायाँ);
  यदि( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    प्रत्यागम 0;                                                          /* (5) */
  }
  यदि( !sqlite3_व्यंजक_Coll_Seq_Match(pParse, p_व्यंजक->p_बायाँ, p_व्यंजक->p_दायाँ) ){
    प्रत्यागम 0;                                                          /* (6) */
  }
  प्रत्यागम 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
स्थिर Bitmask व्यंजक_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  यावत्( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_व्यंजक_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_व्यंजक_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_व्यंजक_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_व्यंजक_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_व्यंजक_Usage(pMaskSet, pS->pHaving);
    यदि( ALWAYS(pSrc!=0) ){
      पूर्णाङ्क i;
      प्रति(i=0; i<pSrc->nSrc; i++){
        यदि( pSrc->a[i].fg.isSubquery ){
          mask |= व्यंजक_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        यदि( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_व्यंजक_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        यदि( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_व्यंजक_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  प्रत्यागम mask;
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
स्थिर SQLITE_NOINLINE पूर्णाङ्क व्यंजक_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  पूर्णाङ्क *aiCurCol,         /* Write the referenced table cursor and column here */
  व्यंजक *p_व्यंजक,           /* An operand of a comparison operator */
  पूर्णाङ्क j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  पूर्णाङ्क i;
  पूर्णाङ्क iCur;
  कुरु{
    iCur = pFrom->a[j].iCursor;
    प्रति(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      यदि( pIdx->a_Col_व्यंजक==0 ) अनुवर्त;
      प्रति(i=0; i<pIdx->n_कुंजी_Col; i++){
        यदि( pIdx->aiColumn[i]!=XN_व्यंजक ) अनुवर्त;
        प्रतिज्ञा( pIdx->b_Has_व्यंजक );
        यदि( sqlite3_व्यंजक_Compare_Skip(p_व्यंजक,pIdx->a_Col_व्यंजक->a[i].p_व्यंजक,iCur)==0
         && !sqlite3_व्यंजक_Is_Constant(0,pIdx->a_Col_व्यंजक->a[i].p_व्यंजक)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_व्यंजक;
          प्रत्यागम 1;
        }
      }
    }
  }यावत्( ++j < pFrom->nSrc );
  प्रत्यागम 0;
}
स्थिर पूर्णाङ्क व्यंजक_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  पूर्णाङ्क *aiCurCol,         /* Write the referenced table cursor & column here */
  व्यंजक *p_व्यंजक,           /* An operand of a comparison operator */
  पूर्णाङ्क op                 /* The specific comparison operator */
){
  पूर्णाङ्क i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  प्रतिज्ञा( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  प्रतिज्ञा( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  प्रतिज्ञा( op<=TK_GE );
  यदि( p_व्यंजक->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
    p_व्यंजक = p_व्यंजक->x.pList->a[0].p_व्यंजक;
  }

  यदि( p_व्यंजक->op==TK_COLUMN ){
    aiCurCol[0] = p_व्यंजक->iTable;
    aiCurCol[1] = p_व्यंजक->iColumn;
    प्रत्यागम 1;
  }

  प्रति(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    प्रति(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      यदि( pIdx->a_Col_व्यंजक ){
        प्रत्यागम व्यंजक_Might_Be_Indexed2(pFrom,aiCurCol,p_व्यंजक,i);
      }
    }
  }
  प्रत्यागम 0;
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
स्थिर शून्य व्यंजक_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  पूर्णाङ्क idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  व्यंजक *p_व्यंजक;                     /* The expression to be analyzed */
  Bitmask prereq_बायाँ;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_दायाँ = 0;          /* Extra dependencies on LEFT JOIN */
  व्यंजक *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  पूर्णाङ्क isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  पूर्णाङ्क noCase = 0;                  /* uppercase equivalent to lowercase */
  पूर्णाङ्क op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned वर्ण eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  पूर्णाङ्क n_बायाँ;                       /* Number of elements on left side vector */

  यदि( db->mallocFailed ){
    प्रत्यागम;
  }
  प्रतिज्ञा( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_व्यंजक = pTerm->p_व्यंजक;
  प्रतिज्ञा( p_व्यंजक!=0 ); /* Because malloc() has not failed */
  प्रतिज्ञा( p_व्यंजक->op!=TK_AS && p_व्यंजक->op!=TK_COLLATE );
व्यंजक_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_बायाँ = sqlite3_Where_व्यंजक_Usage(pMaskSet, p_व्यंजक->p_बायाँ);
  op = p_व्यंजक->op;
  यदि( op==TK_IN ){
    प्रतिज्ञा( p_व्यंजक->p_दायाँ==0 );
    यदि( sqlite3_व्यंजक_Check_IN(pParse, p_व्यंजक) ) प्रत्यागम;
    यदि( व्यंजक_Use_XSelect(p_व्यंजक) ){
      pTerm->prereq_दायाँ = व्यंजक_Select_Usage(pMaskSet, p_व्यंजक->x.pSelect);
    }अन्यथा{
      pTerm->prereq_दायाँ = sqlite3_Where_व्यंजक_List_Usage(pMaskSet, p_व्यंजक->x.pList);
    }
    prereqAll = prereq_बायाँ | pTerm->prereq_दायाँ;
  }अन्यथा{
    pTerm->prereq_दायाँ = sqlite3_Where_व्यंजक_Usage(pMaskSet, p_व्यंजक->p_दायाँ);
    यदि( p_व्यंजक->p_बायाँ==0
     || व्यंजक_Has_Property(p_व्यंजक, EP_xIsSelect|EP_IfNullRow)
     || p_व्यंजक->x.pList!=0
    ){
      prereqAll = sqlite3_Where_व्यंजक_Usage_NN(pMaskSet, p_व्यंजक);
    }अन्यथा{
      prereqAll = prereq_बायाँ | pTerm->prereq_दायाँ;
    }
  }
  यदि( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  यदि( prereqAll!=sqlite3_Where_व्यंजक_Usage_NN(pMaskSet, p_व्यंजक) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_वृक्ष_View_व्यंजक(0,p_व्यंजक,0);
    प्रतिज्ञा( 0 );
  }
#endif

  यदि( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_व्यंजक->w.iJoin);
    यदि( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) ){
      prereqAll |= x;
      extra_दायाँ = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }अन्यथा यदि( (prereqAll>>1)>=x ){
      व्यंजक_Clear_Property(p_व्यंजक, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->बायाँ_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  यदि( allowedOp(op) ){
    पूर्णाङ्क aiCurCol[2];
    व्यंजक *p_बायाँ = sqlite3_व्यंजक_Skip_Collate(p_व्यंजक->p_बायाँ);
    व्यंजक *p_दायाँ = sqlite3_व्यंजक_Skip_Collate(p_व्यंजक->p_दायाँ);
    u16 opMask = (pTerm->prereq_दायाँ & prereq_बायाँ)==0 ? WO_ALL : WO_EQUIV;

    यदि( pTerm->u.x.iField>0 ){
      प्रतिज्ञा( op==TK_IN );
      प्रतिज्ञा( p_बायाँ->op==TK_VECTOR );
      प्रतिज्ञा( व्यंजक_Use_XList(p_बायाँ) );
      p_बायाँ = p_बायाँ->x.pList->a[pTerm->u.x.iField-1].p_व्यंजक;
    }

    यदि( व्यंजक_Might_Be_Indexed(pSrc, aiCurCol, p_बायाँ, op) ){
      pTerm->बायाँ_Cursor = aiCurCol[0];
      प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.बायाँ_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    यदि( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    यदि( p_दायाँ
     && व्यंजक_Might_Be_Indexed(pSrc, aiCurCol, p_दायाँ, op)
     && !व्यंजक_Has_Property(p_दायाँ, EP_FixedCol)
    ){
      WhereTerm *pNew;
      व्यंजक *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      प्रतिज्ञा( pTerm->u.x.iField==0 );
      यदि( pTerm->बायाँ_Cursor>=0 ){
        पूर्णाङ्क idxNew;
        pDup = sqlite3_व्यंजक_Dup(db, p_व्यंजक, 0);
        यदि( db->mallocFailed ){
          sqlite3_व्यंजक_Delete(db, pDup);
          प्रत्यागम;
        }
        idxNew = where_Clause_जोड़ो(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        यदि( idxNew==0 ) प्रत्यागम;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        यदि( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        प्रतिज्ञा( pWInfo->pTabList!=0 );
        यदि( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }अन्यथा{
        pDup = p_व्यंजक;
        pNew = pTerm;
      }
      pNew->wtFlags |= व्यंजक_Commute(pParse, pDup);
      pNew->बायाँ_Cursor = aiCurCol[0];
      प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.बायाँ_Column = aiCurCol[1];
      testcase( (prereq_बायाँ | extra_दायाँ) != prereq_बायाँ );
      pNew->prereq_दायाँ = prereq_बायाँ | extra_दायाँ;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }अन्यथा
    यदि( op==TK_ISNULL
     && !व्यंजक_Has_Property(p_व्यंजक,EP_OuterON)
     && 0==sqlite3_व्यंजक_Can_Be_Null(p_बायाँ)
    ){
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      p_व्यंजक->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_व्यंजक->u.zToken = "false";
      व्यंजक_Set_Property(p_व्यंजक, EP_IsFalse);
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
  अन्यथा यदि( p_व्यंजक->op==TK_BETWEEN && pWC->op==TK_AND ){
    व्यंजक_List *pList;
    पूर्णाङ्क i;
    स्थिर ध्रुव u8 ops[] = {TK_GE, TK_LE};
    प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
    pList = p_व्यंजक->x.pList;
    प्रतिज्ञा( pList!=0 );
    प्रतिज्ञा( pList->n_व्यंजक==2 );
    प्रतिज्ञा( pWC->a[idxTerm].nChild==0 );
    प्रति(i=0; i<2; i++){
      व्यंजक *p_New_व्यंजक;
      पूर्णाङ्क idxNew;
      p_New_व्यंजक = sqlite3PExpr(pParse, ops[i],
                             sqlite3_व्यंजक_Dup(db, p_व्यंजक->p_बायाँ, 0),
                             sqlite3_व्यंजक_Dup(db, pList->a[i].p_व्यंजक, 0));
      transferJoinMarkings(p_New_व्यंजक, p_व्यंजक);
      idxNew = where_Clause_जोड़ो(pWC, p_New_व्यंजक, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      व्यंजक_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#यदि !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  अन्यथा यदि( p_व्यंजक->op==TK_OR && !व्यंजक_Has_Property(p_व्यंजक, EP_Collate) ){
    व्यंजक *pAlt = sqlite3_व्यंजक_Simplified_And_Or(p_व्यंजक);
    यदि( pAlt!=p_व्यंजक ){
      pTerm->p_व्यंजक = p_व्यंजक = sqlite3_व्यंजक_Skip_Collate_And_Likely(pAlt);
      गच्छ व्यंजक_Analyze_restart;
    }
    प्रतिज्ञा( pWC->op==TK_AND );
    व्यंजक_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  अन्यथा यदि( p_व्यंजक->op==TK_NOTNULL ){
    यदि( p_व्यंजक->p_बायाँ->op==TK_COLUMN
     && p_व्यंजक->p_बायाँ->iColumn>=0
     && !व्यंजक_Has_Property(p_व्यंजक, EP_OuterON)
    ){
      व्यंजक *p_New_व्यंजक;
      व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
      पूर्णाङ्क idxNew;
      WhereTerm *pNewTerm;
 
      p_New_व्यंजक = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_व्यंजक_Dup(db, p_बायाँ, 0),
                              sqlite3_व्यंजक_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_जोड़ो(pWC, p_New_व्यंजक,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      यदि( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_दायाँ = 0;
        pNewTerm->बायाँ_Cursor = p_बायाँ->iTable;
        pNewTerm->u.x.बायाँ_Column = p_बायाँ->iColumn;
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
  अन्यथा यदि( p_व्यंजक->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_व्यंजक, &pStr1, &isComplete, &noCase)
  ){
    व्यंजक *p_बायाँ;       /* LHS of LIKE/GLOB operator */
    व्यंजक *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    व्यंजक *pNewExpr1;
    व्यंजक *pNewExpr2;
    पूर्णाङ्क idxNew1;
    पूर्णाङ्क idxNew2;
    ध्रुव वर्ण *zCollSeqName;     /* Name of collating sequence */
    ध्रुव u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
    p_बायाँ = p_व्यंजक->x.pList->a[1].p_व्यंजक;
    pStr2 = sqlite3_व्यंजक_Dup(db, pStr1, 0);
    प्रतिज्ञा( pStr1==0 || !व्यंजक_Has_Property(pStr1, EP_Int_मूल्य) );
    प्रतिज्ञा( pStr2==0 || !व्यंजक_Has_Property(pStr2, EP_Int_मूल्य) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    यदि( noCase && !pParse->db->mallocFailed ){
      पूर्णाङ्क i;
      वर्ण c;
      pTerm->wtFlags |= TERM_LIKE;
      प्रति(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    यदि( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      यदि( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        यदि( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      यावत्( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      प्रतिज्ञा( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_द्विआधारी;
    pNewExpr1 = sqlite3_व्यंजक_Dup(db, p_बायाँ, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_व्यंजक_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_व्यंजक);
    idxNew1 = where_Clause_जोड़ो(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_व्यंजक_Dup(db, p_बायाँ, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_व्यंजक_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_व्यंजक);
    idxNew2 = where_Clause_जोड़ो(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    व्यंजक_Analyze(pSrc, pWC, idxNew1);
    व्यंजक_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    यदि( isComplete ){
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
  यदि( (p_व्यंजक->op==TK_EQ || p_व्यंजक->op==TK_IS)
   && (n_बायाँ = sqlite3_व्यंजक_Vector_Size(p_व्यंजक->p_बायाँ))>1
   && sqlite3_व्यंजक_Vector_Size(p_व्यंजक->p_दायाँ)==n_बायाँ
   && ( (p_व्यंजक->p_बायाँ->flags & EP_xIsSelect)==0
     || (p_व्यंजक->p_दायाँ->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    पूर्णाङ्क i;
    प्रति(i=0; i<n_बायाँ; i++){
      पूर्णाङ्क idxNew;
      व्यंजक *pNew;
      व्यंजक *p_बायाँ = sqlite3_व्यंजक_For_Vector_Field(pParse, p_व्यंजक->p_बायाँ, i, n_बायाँ);
      व्यंजक *p_दायाँ = sqlite3_व्यंजक_For_Vector_Field(pParse, p_व्यंजक->p_दायाँ, i, n_बायाँ);

      pNew = sqlite3PExpr(pParse, p_व्यंजक->op, p_बायाँ, p_दायाँ);
      transferJoinMarkings(pNew, p_व्यंजक);
      idxNew = where_Clause_जोड़ो(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      व्यंजक_Analyze(pSrc, pWC, idxNew);
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
  अन्यथा यदि( p_व्यंजक->op==TK_IN
   && pTerm->u.x.iField==0
   && p_व्यंजक->p_बायाँ->op==TK_VECTOR
   && ALWAYS( व्यंजक_Use_XSelect(p_व्यंजक) )
   && (p_व्यंजक->x.pSelect->pPrior==0 || (p_व्यंजक->x.pSelect->selFlags & SF_मूल्यावली))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_व्यंजक->x.pSelect->pWin==0
#endif
   && (p_व्यंजक->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_व्यंजक->x.pSelect->pEList->n_व्यंजक <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    पूर्णाङ्क i;
    प्रतिज्ञा( pTerm->nChild==0 );
    प्रति(i=0; i<sqlite3_व्यंजक_Vector_Size(p_व्यंजक->p_बायाँ); i++){
      पूर्णाङ्क idxNew;
      idxNew = where_Clause_जोड़ो(pWC, p_व्यंजक, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      व्यंजक_Analyze(pSrc, pWC, idxNew);
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
  अन्यथा यदि( pWC->op==TK_AND ){
    व्यंजक *p_दायाँ = 0, *p_बायाँ = 0;
    पूर्णाङ्क res = isAuxiliaryVtabOperator(db, p_व्यंजक, &eOp2, &p_बायाँ, &p_दायाँ);
    यावत्( res-- > 0 ){
      पूर्णाङ्क idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_व्यंजक;

      prereq_व्यंजक = sqlite3_Where_व्यंजक_Usage(pMaskSet, p_दायाँ);
      prereqColumn = sqlite3_Where_व्यंजक_Usage(pMaskSet, p_बायाँ);
      यदि( (prereq_व्यंजक & prereqColumn)==0 ){
        व्यंजक *p_New_व्यंजक;
        p_New_व्यंजक = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_व्यंजक_Dup(db, p_दायाँ, 0));
        यदि( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) && p_New_व्यंजक ){
          व्यंजक_Set_Property(p_New_व्यंजक, EP_OuterON);
          p_New_व्यंजक->w.iJoin = p_व्यंजक->w.iJoin;
        }
        idxNew = where_Clause_जोड़ो(pWC, p_New_व्यंजक, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_दायाँ = prereq_व्यंजक | extra_दायाँ;
        pNewTerm->बायाँ_Cursor = p_बायाँ->iTable;
        pNewTerm->u.x.बायाँ_Column = p_बायाँ->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(व्यंजक*, p_बायाँ, p_दायाँ);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_दायाँ |= extra_दायाँ;
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
शून्य sqlite3WhereSplit(WhereClause *pWC, व्यंजक *p_व्यंजक, u8 op){
  व्यंजक *pE2 = sqlite3_व्यंजक_Skip_Collate_And_Likely(p_व्यंजक);
  pWC->op = op;
  प्रतिज्ञा( pE2!=0 || p_व्यंजक==0 );
  यदि( pE2==0 ) प्रत्यागम;
  यदि( pE2->op!=op ){
    where_Clause_जोड़ो(pWC, p_व्यंजक, 0);
  }अन्यथा{
    sqlite3WhereSplit(pWC, pE2->p_बायाँ, op);
    sqlite3WhereSplit(pWC, pE2->p_दायाँ, op);
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
स्थिर शून्य where_Add_Limit_व्यंजक(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  पूर्णाङ्क iReg,           /* Register that will hold value of the limit/offset */
  व्यंजक *p_व्यंजक,        /* Expression that defines the limit/offset */
  पूर्णाङ्क iCsr,           /* Cursor to which the constraint applies */
  पूर्णाङ्क eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  व्यंजक *pNew;
  पूर्णाङ्क iVal = 0;

  यदि( sqlite3_व्यंजक_Is_Integer(p_व्यंजक, &iVal, pParse, 0) && iVal>=0 ){
    व्यंजक *pVal = sqlite3_व्यंजक_Int32(db, iVal);
    यदि( pVal==0 ) प्रत्यागम;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }अन्यथा{
    व्यंजक *pVal = sqlite3_व्यंजक_Alloc(db, TK_REGISTER, 0, 0);
    यदि( pVal==0 ) प्रत्यागम;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  यदि( pNew ){
    WhereTerm *pTerm;
    पूर्णाङ्क idx;
    idx = where_Clause_जोड़ो(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->बायाँ_Cursor = iCsr;
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
शून्य SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  प्रतिज्ञा( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  यदि( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    व्यंजक_List *pOrderBy = p->pOrderBy;
    पूर्णाङ्क iCsr = p->pSrc->a[0].iCursor;
    पूर्णाङ्क ii;

    /* Check condition (4). Return early if it is not met. */
    प्रति(ii=0; ii<pWC->nTerm; ii++){
      यदि( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        प्रतिज्ञा( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        प्रतिज्ञा( pWC->a[ii].eOperator==WO_ROWVAL );
        अनुवर्त;
      }
      यदि( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        अनुवर्त;
      }
      यदि( pWC->a[ii].बायाँ_Cursor==iCsr && pWC->a[ii].prereq_दायाँ==0 ) अनुवर्त;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      यदि( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        यदि( pParent->बायाँ_Cursor==iCsr
         && pParent->prereq_दायाँ==0
         && pParent->nChild==1
        ){
          अनुवर्त;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      प्रत्यागम;
    }

    /* Check condition (5). Return early if it is not met. */
    यदि( pOrderBy ){
      प्रति(ii=0; ii<pOrderBy->n_व्यंजक; ii++){
        व्यंजक *p_व्यंजक = pOrderBy->a[ii].p_व्यंजक;
        यदि( p_व्यंजक->op!=TK_COLUMN ) प्रत्यागम;
        यदि( p_व्यंजक->iTable!=iCsr ) प्रत्यागम;
        यदि( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) प्रत्यागम;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    प्रतिज्ञा( p->pLimit->op==TK_LIMIT );
    यदि( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_व्यंजक(pWC, p->iOffset, p->pLimit->p_दायाँ,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    यदि( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_व्यंजक(pWC, p->iLimit, p->pLimit->p_बायाँ,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
शून्य sqlite3WhereClauseInit(
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
शून्य sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  प्रतिज्ञा( pWC->nTerm>=pWC->nBase );
  यदि( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    पूर्णाङ्क i;
    /* Verify that every term past pWC->nBase is virtual */
    प्रति(i=pWC->nBase; i<pWC->nTerm; i++){
      प्रतिज्ञा( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    यावत्(1){
      प्रतिज्ञा( a->eMatchOp==0 || a->eOperator==WO_AUX );
      यदि( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_व्यंजक_Delete(db, a->p_व्यंजक);
      }
      यदि( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        यदि( a->wtFlags & TERM_ORINFO ){
          प्रतिज्ञा( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }अन्यथा{
          प्रतिज्ञा( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      यदि( a==aLast ) भङ्ग;
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
स्थिर SQLITE_NOINLINE Bitmask sqlite3_Where_व्यंजक_Usage_Full(
  WhereMaskSet *pMaskSet,
  व्यंजक *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  यदि( p->p_बायाँ ) mask |= sqlite3_Where_व्यंजक_Usage_NN(pMaskSet, p->p_बायाँ);
  यदि( p->p_दायाँ ){
    mask |= sqlite3_Where_व्यंजक_Usage_NN(pMaskSet, p->p_दायाँ);
    प्रतिज्ञा( p->x.pList==0 );
  }अन्यथा यदि( व्यंजक_Use_XSelect(p) ){
    यदि( व्यंजक_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= व्यंजक_Select_Usage(pMaskSet, p->x.pSelect);
  }अन्यथा यदि( p->x.pList ){
    mask |= sqlite3_Where_व्यंजक_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  यदि( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && व्यंजक_Use_YWin(p) ){
    प्रतिज्ञा( p->y.pWin!=0 );
    mask |= sqlite3_Where_व्यंजक_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_व्यंजक_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_व्यंजक_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  प्रत्यागम mask;
}
Bitmask sqlite3_Where_व्यंजक_Usage_NN(WhereMaskSet *pMaskSet, व्यंजक *p){
  यदि( p->op==TK_COLUMN && !व्यंजक_Has_Property(p, EP_FixedCol) ){
    प्रत्यागम sqlite3WhereGetMask(pMaskSet, p->iTable);
  }अन्यथा यदि( व्यंजक_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    प्रतिज्ञा( p->op!=TK_IF_NULL_ROW );
    प्रत्यागम 0;
  }
  प्रत्यागम sqlite3_Where_व्यंजक_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_व्यंजक_Usage(WhereMaskSet *pMaskSet, व्यंजक *p){
  प्रत्यागम p ? sqlite3_Where_व्यंजक_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_व्यंजक_List_Usage(WhereMaskSet *pMaskSet, व्यंजक_List *pList){
  पूर्णाङ्क i;
  Bitmask mask = 0;
  यदि( pList ){
    प्रति(i=0; i<pList->n_व्यंजक; i++){
      mask |= sqlite3_Where_व्यंजक_Usage(pMaskSet, pList->a[i].p_व्यंजक);
    }
  }
  प्रत्यागम mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
शून्य sqlite3_Where_व्यंजक_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  पूर्णाङ्क i;
  प्रति(i=pWC->nTerm-1; i>=0; i--){
    व्यंजक_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
शून्य sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  पूर्णाङ्क j, k;
  व्यंजक_List *pArgs;
  व्यंजक *pColRef;
  व्यंजक *pTerm;
  यदि( pItem->fg.isTabFunc==0 ) प्रत्यागम;
  pTab = pItem->pSTab;
  प्रतिज्ञा( pTab!=0 );
  यदि( !IsVirtual(pTab) ){
    sqlite3_Error_संदेश(pParse, "'%s' is not a function", pItem->zName);
    प्रत्यागम;
  }
  pArgs = pItem->u1.pFuncArg;
  यदि( pArgs==0 ) प्रत्यागम;
  प्रति(j=k=0; j<pArgs->n_व्यंजक; j++){
    व्यंजक *pRhs;
    u32 joinType;
    यावत्( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    यदि( k>=pTab->nCol ){
      sqlite3_Error_संदेश(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      प्रत्यागम;
    }
    pColRef = sqlite3_व्यंजक_Alloc(pParse->db, TK_COLUMN, 0, 0);
    यदि( pColRef==0 ) प्रत्यागम;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    प्रतिज्ञा( व्यंजक_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_व्यंजक_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_व्यंजक_Dup(pParse->db, pArgs->a[j].p_व्यंजक, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    यदि( pItem->fg.jointype & (JT_बायाँ|JT_दायाँ) ){
      testcase( pItem->fg.jointype & JT_बायाँ );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_दायाँ ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }अन्यथा{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_व्यंजक(pTerm, pItem->iCursor, joinType);
    where_Clause_जोड़ो(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
पूर्णाङ्क sqlite3WhereLoopBloomable(ध्रुव WhereLoop *pLoop){
  यदि( pLoop->wsFlags & WHERE_IPK ) प्रत्यागम 1;
  यदि( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) प्रत्यागम 0;
  प्रत्यागम sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
