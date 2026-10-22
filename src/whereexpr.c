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
स्थिर शून्य व्यञ्जकम्_Analyze(SrcList*, WhereClause*, पूर्णाङ्क);

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
स्थिर पूर्णाङ्क where_Clause_निवेशय(WhereClause *pWC, व्यञ्जकम् *p, u16 wtFlags){
  WhereTerm *pTerm;
  पूर्णाङ्क idx;
  testcase( wtFlags & TERM_VIRTUAL );
  सचे( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, परिमाण(pWC->a[0])*pWC->nSlot*2 );
    सचे( pWC->a==0 ){
      सचे( wtFlags & TERM_DYNAMIC ){
        sqlite3_व्यञ्जकम्_Delete(db, p);
      }
      pWC->a = pOld;
      पटिनिवत्त 0;
    }
    memcpy(pWC->a, pOld, परिमाण(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  सचे( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  सचे( p && व्यञ्जकम्_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }अञ्ञथा{
    pTerm->truthProb = 1;
  }
  pTerm->p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         परिमाण(WhereTerm) - offsetof(WhereTerm,eOperator));
  पटिनिवत्त idx;
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
  सचे( op>TK_GE ) पटिनिवत्त 0;
  सचे( op>=TK_EQ ) पटिनिवत्त 1;
  पटिनिवत्त op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
स्थिर u16 व्यञ्जकम्_Commute(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्){
  सचे( p_व्यञ्जकम्->p_वामः->op==TK_VECTOR
   || p_व्यञ्जकम्->p_दक्षिणः->op==TK_VECTOR
   || sqlite3_द्विमान_Compare_Coll_Seq(pParse, p_व्यञ्जकम्->p_वामः, p_व्यञ्जकम्->p_दक्षिणः) !=
      sqlite3_द्विमान_Compare_Coll_Seq(pParse, p_व्यञ्जकम्->p_दक्षिणः, p_व्यञ्जकम्->p_वामः)
  ){
    p_व्यञ्जकम्->flags ^= EP_Commuted;
  }
  SWAP(व्यञ्जकम्*,p_व्यञ्जकम्->p_दक्षिणः,p_व्यञ्जकम्->p_वामः);
  सचे( p_व्यञ्जकम्->op>=TK_GT ){
    प्रतिज्ञा( TK_LT==TK_GT+2 );
    प्रतिज्ञा( TK_GE==TK_LE+2 );
    प्रतिज्ञा( TK_GT>TK_EQ );
    प्रतिज्ञा( TK_GT<TK_LE );
    प्रतिज्ञा( p_व्यञ्जकम्->op>=TK_GT && p_व्यञ्जकम्->op<=TK_GE );
    p_व्यञ्जकम्->op = ((p_व्यञ्जकम्->op-TK_GT)^2)+TK_GT;
  }
  पटिनिवत्त 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
स्थिर u16 operatorMask(पूर्णाङ्क op){
  u16 c;
  प्रतिज्ञा( allowedOp(op) );
  सचे( op>=TK_EQ ){
    प्रतिज्ञा( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }अञ्ञथा सचे( op==TK_IN ){
    c = WO_IN;
  }अञ्ञथा सचे( op==TK_ISNULL ){
    c = WO_ISNULL;
  }अञ्ञथा{
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
  पटिनिवत्त c;
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
  व्यञ्जकम् *p_व्यञ्जकम्,      /* Test this expression */
  व्यञ्जकम् **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  पूर्णाङ्क *pisComplete, /* True if the only wildcard is % in the last character */
  पूर्णाङ्क *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ध्रुव u8 *z = 0;           /* String on RHS of LIKE operator */
  व्यञ्जकम् *p_दक्षिणः, *p_वामः;      /* Right and left size of LIKE operator */
  व्यञ्जकम्_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  पूर्णाङ्क cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_मानम् *pVal = 0;
  पूर्णाङ्क op;                    /* Opcode of pRight */
  पूर्णाङ्क rc;                    /* Result code to return */

  सचे( !sqlite3IsLikeFunction(db, p_व्यञ्जकम्, pnoCase, (वर्ण*)wc) ){
    पटिनिवत्त 0;
  }
#ifdef SQLITE_EBCDIC
  सचे( *pnoCase ) पटिनिवत्त 0;
#endif
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
  pList = p_व्यञ्जकम्->x.pList;
  p_वामः = pList->a[1].p_व्यञ्जकम्;

  p_दक्षिणः = sqlite3_व्यञ्जकम्_Skip_Collate(pList->a[0].p_व्यञ्जकम्);
  op = p_दक्षिणः->op;
  सचे( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    पूर्णाङ्क iCol = p_दक्षिणः->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_मानम्(pReprepare, iCol, SQLITE_AFF_BLOB);
    सचे( pVal && sqlite3_मानम्_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_मानम्_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    प्रतिज्ञा( p_दक्षिणः->op==TK_VARIABLE || p_दक्षिणः->op==TK_REGISTER );
  }अञ्ञथा सचे( op==TK_STRING ){
    प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_दक्षिणः, EP_Int_मानम्) );
     z = (u8*)p_दक्षिणः->u.zToken;
  }
  सचे( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    याव( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      सचे( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }अञ्ञथा सचे( c>=0x80 ){
        ध्रुव u8 *z2 = z+cnt-1;
        सचे( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          भङ्ग;
        }अञ्ञथा{
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
    सचे( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      व्यञ्जकम् *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_व्यञ्जकम्(db, TK_STRING, (वर्ण*)z);
      सचे( pPrefix ){
        पूर्णाङ्क iFrom, iTo;
        वर्ण *zNew;
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pPrefix, EP_Int_मानम्) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        अत्थाय(iFrom=iTo=0; iFrom<cnt; iFrom++){
          सचे( zNew[iFrom]==wc[3] ) iFrom++;
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
        सचे( p_वामः->op!=TK_COLUMN
         || sqlite3_व्यञ्जकम्_Affinity(p_वामः)!=SQLITE_AFF_TEXT
         || (ALWAYS( व्यञ्जकम्_Use_YTab(p_वामः) )
             && ALWAYS(p_वामः->y.pTab)
             && IsVirtual(p_वामः->y.pTab))  /* Might be numeric */
        ){
          पूर्णाङ्क is_अङ्कः;
          द्विगुण rDummy;
          प्रतिज्ञा( zNew[iTo]==0 );
          is_अङ्कः = sqlite3AtoF(zNew, &rDummy);
          सचे( is_अङ्कः<=0 ){
            सचे( iTo==1 && zNew[0]=='-' ){
              is_अङ्कः = +1;
            }अञ्ञथा{
              zNew[iTo-1]++;
              is_अङ्कः = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          सचे( is_अङ्कः>0 ){
            sqlite3_व्यञ्जकम्_Delete(db, pPrefix);
            sqlite3_मानम्_Free(pVal);
            पटिनिवत्त 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      सचे( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_दक्षिणः->iColumn, 0);
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_दक्षिणः, EP_Int_मानम्) );
        सचे( *pisComplete && p_दक्षिणः->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          पूर्णाङ्क r1 = sqlite3GetTempReg(pParse);
          sqlite3_व्यञ्जकम्_Code_Target(pParse, p_दक्षिणः, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }अञ्ञथा{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_मानम्_Free(pVal);
  पटिनिवत्त rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Like_Operator(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
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
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_FUNCTION );
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
  अत्थाय(i=0; i<ArraySize(aOp); i++){
    सचे( sqlite3StrICmp(p_व्यञ्जकम्->u.zToken, aOp[i].zOp)==0 ){
      पटिनिवत्त aOp[i].eOp;
    }
  }
  पटिनिवत्त 0;
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
  व्यञ्जकम् *p_व्यञ्जकम्,                    /* Test this expression */
  unsigned वर्ण *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  व्यञ्जकम् **pp_वामः,                  /* Column expression to left of MATCH/op2 */
  व्यञ्जकम् **pp_दक्षिणः                  /* Expression to left of MATCH/op2 */
){
  सचे( p_व्यञ्जकम्->op==TK_FUNCTION ){
    व्यञ्जकम्_List *pList;
    व्यञ्जकम् *pCol;                     /* Column reference */
    पूर्णाङ्क i;

    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
    pList = p_व्यञ्जकम्->x.pList;
    सचे( pList==0 || pList->n_व्यञ्जकम्!=2 ){
      पटिनिवत्त 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_व्यञ्जकम्;
    प्रतिज्ञा( pCol->op!=TK_COLUMN || (व्यञ्जकम्_Use_YTab(pCol) && pCol->y.pTab!=0) );
    सचे( व्यञ्जकम्_Is_Vtab(pCol) && (i = sqlite3_व्यञ्जकम्_Is_Like_Operator(p_व्यञ्जकम्))!=0 ){
      *peOp2 = i;
      *pp_दक्षिणः = pList->a[0].p_व्यञ्जकम्;
      *pp_वामः = pCol;
      पटिनिवत्त 1;
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
    pCol = pList->a[0].p_व्यञ्जकम्;
    प्रतिज्ञा( pCol->op!=TK_COLUMN || व्यञ्जकम्_Use_YTab(pCol) );
    प्रतिज्ञा( pCol->op!=TK_COLUMN || (व्यञ्जकम्_Use_YTab(pCol) && pCol->y.pTab!=0) );
    सचे( व्यञ्जकम्_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      शून्य (*xNotUsed)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**);
      शून्य *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      प्रतिज्ञा( pVtab!=0 );
      प्रतिज्ञा( pVtab->pModule!=0 );
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      pMod = (sqlite3_module *)pVtab->pModule;
      सचे( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_व्यञ्जकम्->u.zToken, &xNotUsed, &pNotUsed);
        सचे( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_दक्षिणः = pList->a[1].p_व्यञ्जकम्;
          *pp_वामः = pCol;
          पटिनिवत्त 1;
        }
      }
    }
  }अञ्ञथा सचे( p_व्यञ्जकम्->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    प्रतिज्ञा( TK_NE < TK_EQ );
    प्रतिज्ञा( TK_ISNOT < TK_EQ );
    प्रतिज्ञा( TK_NOTNULL < TK_EQ );
    पटिनिवत्त 0;
  }अञ्ञथा सचे( p_व्यञ्जकम्->op==TK_NE || p_व्यञ्जकम्->op==TK_ISNOT || p_व्यञ्जकम्->op==TK_NOTNULL ){
    पूर्णाङ्क res = 0;
    व्यञ्जकम् *p_वामः = p_व्यञ्जकम्->p_वामः;
    व्यञ्जकम् *p_दक्षिणः = p_व्यञ्जकम्->p_दक्षिणः;
    प्रतिज्ञा( p_वामः->op!=TK_COLUMN || (व्यञ्जकम्_Use_YTab(p_वामः) && p_वामः->y.pTab!=0) );
    सचे( व्यञ्जकम्_Is_Vtab(p_वामः) ){
      res++;
    }
    प्रतिज्ञा( p_दक्षिणः==0 || p_दक्षिणः->op!=TK_COLUMN
            || (व्यञ्जकम्_Use_YTab(p_दक्षिणः) && p_दक्षिणः->y.pTab!=0) );
    सचे( p_दक्षिणः && व्यञ्जकम्_Is_Vtab(p_दक्षिणः) ){
      res++;
      SWAP(व्यञ्जकम्*, p_वामः, p_दक्षिणः);
    }
    *pp_वामः = p_वामः;
    *pp_दक्षिणः = p_दक्षिणः;
    सचे( p_व्यञ्जकम्->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    सचे( p_व्यञ्जकम्->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    सचे( p_व्यञ्जकम्->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    पटिनिवत्त res;
  }
  पटिनिवत्त 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
स्थिर शून्य transferJoinMarkings(व्यञ्जकम् *pDerived, व्यञ्जकम् *pBase){
  सचे( pDerived && व्यञ्जकम्_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
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
  सचे( pTerm->eOperator!=WO_AND ){
    पटिनिवत्त N==0 ? pTerm : 0;
  }
  सचे( N<pTerm->u.pAndInfo->wc.nTerm ){
    पटिनिवत्त &pTerm->u.pAndInfo->wc.a[N];
  }
  पटिनिवत्त 0;
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
  व्यञ्जकम् *pNew;            /* New virtual expression */
  पूर्णाङ्क op;                /* Operator for the combined expression */
  पूर्णाङ्क idxNew;            /* Index in pWC of the next virtual term */
  व्यञ्जकम् *pA, *pB;         /* Expressions associated with pOne and pTwo */

  सचे( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) पटिनिवत्त;
  सचे( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) पटिनिवत्त;
  सचे( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) पटिनिवत्त;
  सचे( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) पटिनिवत्त;
  pA = pOne->p_व्यञ्जकम्;
  pB = pTwo->p_व्यञ्जकम्;
  प्रतिज्ञा( pA->p_वामः!=0 && pA->p_दक्षिणः!=0 );
  प्रतिज्ञा( pB->p_वामः!=0 && pB->p_दक्षिणः!=0 );
  सचे( sqlite3_व्यञ्जकम्_Compare(0,pA->p_वामः, pB->p_वामः, -1) )  पटिनिवत्त;
  सचे( sqlite3_व्यञ्जकम्_Compare(0,pA->p_दक्षिणः, pB->p_दक्षिणः,-1) ) पटिनिवत्त;
  सचे( व्यञ्जकम्_Has_Property(pA,EP_Commuted)!=व्यञ्जकम्_Has_Property(pB,EP_Commuted) ){
    पटिनिवत्त;
  }
  /* If we reach this point, it means the two subterms can be combined */
  सचे( (eOp & (eOp-1))!=0 ){
    सचे( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }अञ्ञथा{
      प्रतिज्ञा( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_व्यञ्जकम्_Dup(db, pA, 0);
  सचे( pNew==0 ) पटिनिवत्त;
  अत्थाय(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ प्रतिज्ञा( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_निवेशय(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  व्यञ्जकम्_Analyze(pSrc, pWC, idxNew);
}

#सचे !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
स्थिर शून्य व्यञ्जकम्_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  पूर्णाङ्क idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  व्यञ्जकम् *p_व्यञ्जकम् = pTerm->p_व्यञ्जकम्;             /* The expression of the term */
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
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, परिमाण(*pOrInfo));
  सचे( pOrInfo==0 ) पटिनिवत्त;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, परिमाण(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_व्यञ्जकम्, TK_OR);
  sqlite3_Where_व्यञ्जकम्_Analyze(pSrc, pOrWc);
  सचे( db->mallocFailed ) पटिनिवत्त;
  प्रतिज्ञा( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  अत्थाय(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    सचे( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      प्रतिज्ञा( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, परिमाण(*pAndInfo));
      सचे( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        पूर्णाङ्क j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->वामः_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, परिमाण(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_व्यञ्जकम्, TK_AND);
        sqlite3_Where_व्यञ्जकम्_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        सचे( !db->mallocFailed ){
          अत्थाय(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            प्रतिज्ञा( pAndTerm->p_व्यञ्जकम् );
            सचे( allowedOp(pAndTerm->p_व्यञ्जकम्->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->वामः_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }अञ्ञथा सचे( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }अञ्ञथा{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->वामः_Cursor);
      सचे( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->वामः_Cursor);
      }
      indexable &= b;
      सचे( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }अञ्ञथा{
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
  pTerm->वामः_Cursor = -1;
  सचे( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  सचे( indexable && pOrWc->nTerm==2 ){
    पूर्णाङ्क iOne = 0;
    WhereTerm *pOne;
    याव( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      पूर्णाङ्क iTwo = 0;
      WhereTerm *pTwo;
      याव( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  सचे( chngToIN ){
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
    अत्थाय(j=0; j<2 && !okToChngToIN; j++){
      व्यञ्जकम् *p_वामः = 0;
      pOrTerm = pOrWc->a;
      अत्थाय(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        प्रतिज्ञा( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        सचे( pOrTerm->वामः_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          प्रतिज्ञा( j==1 );
          अनुवर्त;
        }
        सचे( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->वामः_Cursor))==0 ){
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
        iColumn = pOrTerm->u.x.वामः_Column;
        iCursor = pOrTerm->वामः_Cursor;
        p_वामः = pOrTerm->p_व्यञ्जकम्->p_वामः;
        भङ्ग;
      }
      सचे( i<0 ){
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
      अत्थाय(; i>=0 && okToChngToIN; i--, pOrTerm++){
        प्रतिज्ञा( pOrTerm->eOperator & WO_EQ );
        प्रतिज्ञा( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        सचे( pOrTerm->वामः_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }अञ्ञथा सचे( pOrTerm->u.x.वामः_Column!=iColumn || (iColumn==XN_व्यञ्जकम्
               && sqlite3_व्यञ्जकम्_Compare(pParse, pOrTerm->p_व्यञ्जकम्->p_वामः, p_वामः, -1)
        )){
          okToChngToIN = 0;
        }अञ्ञथा{
          पूर्णाङ्क aff_वामः, aff_दक्षिणः;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_दक्षिणः = sqlite3_व्यञ्जकम्_Affinity(pOrTerm->p_व्यञ्जकम्->p_दक्षिणः);
          aff_वामः = sqlite3_व्यञ्जकम्_Affinity(pOrTerm->p_व्यञ्जकम्->p_वामः);
          सचे( aff_दक्षिणः!=0 && aff_दक्षिणः!=aff_वामः ){
            okToChngToIN = 0;
          }अञ्ञथा{
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
    सचे( okToChngToIN ){
      व्यञ्जकम् *pDup;            /* A transient duplicate expression */
      व्यञ्जकम्_List *pList = 0;   /* The RHS of the IN operator */
      व्यञ्जकम् *p_वामः = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      व्यञ्जकम् *pNew;            /* The complete IN operator */

      अत्थाय(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        व्यञ्जकम् *pThis;
        सचे( (pOrTerm->wtFlags & TERM_OK)==0 ) अनुवर्त;
        प्रतिज्ञा( pOrTerm->eOperator & WO_EQ );
        प्रतिज्ञा( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        प्रतिज्ञा( pOrTerm->वामः_Cursor==iCursor );
        प्रतिज्ञा( pOrTerm->u.x.वामः_Column==iColumn );
        pThis = pOrTerm->p_व्यञ्जकम्;
        pDup = sqlite3_व्यञ्जकम्_Dup(db, pThis->p_दक्षिणः, 0);
        pList = sqlite3_व्यञ्जकम्_List_Append(pWInfo->pParse, pList, pDup);
        सचे( p_वामः==0 ){
          p_वामः = pThis->p_वामः;
          pCollSeq = sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pParse, pThis);
        }अञ्ञथा{
          प्रतिज्ञा( 0==sqlite3_व्यञ्जकम्_Compare(pParse, 
                        sqlite3_व्यञ्जकम्_Skip_Collate(pThis->p_वामः),
                        sqlite3_व्यञ्जकम्_Skip_Collate(p_वामः), -1) );
          सचे( pCollSeq!=sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pParse, pThis) ){
            p_वामः = 0;  /* Collating sequence mismatch */
            भङ्ग;
          }
        }
      }
      सचे( p_वामः==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }अञ्ञथा{
        pDup = sqlite3_व्यञ्जकम्_Dup(db, p_वामः, 0);
        सचे( sqlite3_व्यञ्जकम्_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          प्रतिज्ञा( pCollSeq->zName!=0 );
          pDup = sqlite3_व्यञ्जकम्_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      सचे( pNew ){
        पूर्णाङ्क idxNew;
        transferJoinMarkings(pNew, p_व्यञ्जकम्);
        प्रतिज्ञा( व्यञ्जकम्_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_निवेशय(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        व्यञ्जकम्_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }अञ्ञथा{
        sqlite3_व्यञ्जकम्_List_Delete(db, pList);
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
स्थिर पूर्णाङ्क termIsEquivalence(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्, SrcList *pSrc){
  वर्ण aff1, aff2;
  सचे( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) पटिनिवत्त 0;  /* (1) */
  सचे( p_व्यञ्जकम्->op!=TK_EQ && p_व्यञ्जकम्->op!=TK_IS ) पटिनिवत्त 0;                 /* (2) */
  सचे( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON|EP_Collate) ) पटिनिवत्त 0;        /* (3) */
  प्रतिज्ञा( pSrc!=0 );
  सचे( p_व्यञ्जकम्->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    पटिनिवत्त 0;                                                          /* (4) */
  }
  aff1 = sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्->p_वामः);
  aff2 = sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्->p_दक्षिणः);
  सचे( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    पटिनिवत्त 0;                                                          /* (5) */
  }
  सचे( !sqlite3_व्यञ्जकम्_Coll_Seq_Match(pParse, p_व्यञ्जकम्->p_वामः, p_व्यञ्जकम्->p_दक्षिणः) ){
    पटिनिवत्त 0;                                                          /* (6) */
  }
  पटिनिवत्त 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
स्थिर Bitmask व्यञ्जकम्_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  याव( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_व्यञ्जकम्_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_व्यञ्जकम्_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_व्यञ्जकम्_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, pS->pHaving);
    सचे( ALWAYS(pSrc!=0) ){
      पूर्णाङ्क i;
      अत्थाय(i=0; i<pSrc->nSrc; i++){
        सचे( pSrc->a[i].fg.isSubquery ){
          mask |= व्यञ्जकम्_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        सचे( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        सचे( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_व्यञ्जकम्_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  पटिनिवत्त mask;
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
स्थिर SQLITE_NOINLINE पूर्णाङ्क व्यञ्जकम्_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  पूर्णाङ्क *aiCurCol,         /* Write the referenced table cursor and column here */
  व्यञ्जकम् *p_व्यञ्जकम्,           /* An operand of a comparison operator */
  पूर्णाङ्क j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  पूर्णाङ्क i;
  पूर्णाङ्क iCur;
  करोहि{
    iCur = pFrom->a[j].iCursor;
    अत्थाय(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      सचे( pIdx->a_Col_व्यञ्जकम्==0 ) अनुवर्त;
      अत्थाय(i=0; i<pIdx->n_कुञ्चिका_Col; i++){
        सचे( pIdx->aiColumn[i]!=XN_व्यञ्जकम् ) अनुवर्त;
        प्रतिज्ञा( pIdx->b_Has_व्यञ्जकम् );
        सचे( sqlite3_व्यञ्जकम्_Compare_Skip(p_व्यञ्जकम्,pIdx->a_Col_व्यञ्जकम्->a[i].p_व्यञ्जकम्,iCur)==0
         && !sqlite3_व्यञ्जकम्_Is_Constant(0,pIdx->a_Col_व्यञ्जकम्->a[i].p_व्यञ्जकम्)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_व्यञ्जकम्;
          पटिनिवत्त 1;
        }
      }
    }
  }याव( ++j < pFrom->nSrc );
  पटिनिवत्त 0;
}
स्थिर पूर्णाङ्क व्यञ्जकम्_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  पूर्णाङ्क *aiCurCol,         /* Write the referenced table cursor & column here */
  व्यञ्जकम् *p_व्यञ्जकम्,           /* An operand of a comparison operator */
  पूर्णाङ्क op                 /* The specific comparison operator */
){
  पूर्णाङ्क i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  प्रतिज्ञा( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  प्रतिज्ञा( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  प्रतिज्ञा( op<=TK_GE );
  सचे( p_व्यञ्जकम्->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
    p_व्यञ्जकम् = p_व्यञ्जकम्->x.pList->a[0].p_व्यञ्जकम्;
  }

  सचे( p_व्यञ्जकम्->op==TK_COLUMN ){
    aiCurCol[0] = p_व्यञ्जकम्->iTable;
    aiCurCol[1] = p_व्यञ्जकम्->iColumn;
    पटिनिवत्त 1;
  }

  अत्थाय(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    अत्थाय(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      सचे( pIdx->a_Col_व्यञ्जकम् ){
        पटिनिवत्त व्यञ्जकम्_Might_Be_Indexed2(pFrom,aiCurCol,p_व्यञ्जकम्,i);
      }
    }
  }
  पटिनिवत्त 0;
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
स्थिर शून्य व्यञ्जकम्_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  पूर्णाङ्क idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  व्यञ्जकम् *p_व्यञ्जकम्;                     /* The expression to be analyzed */
  Bitmask prereq_वामः;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_दक्षिणः = 0;          /* Extra dependencies on LEFT JOIN */
  व्यञ्जकम् *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  पूर्णाङ्क isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  पूर्णाङ्क noCase = 0;                  /* uppercase equivalent to lowercase */
  पूर्णाङ्क op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned वर्ण eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  पूर्णाङ्क n_वामः;                       /* Number of elements on left side vector */

  सचे( db->mallocFailed ){
    पटिनिवत्त;
  }
  प्रतिज्ञा( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_व्यञ्जकम् = pTerm->p_व्यञ्जकम्;
  प्रतिज्ञा( p_व्यञ्जकम्!=0 ); /* Because malloc() has not failed */
  प्रतिज्ञा( p_व्यञ्जकम्->op!=TK_AS && p_व्यञ्जकम्->op!=TK_COLLATE );
व्यञ्जकम्_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_वामः = sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, p_व्यञ्जकम्->p_वामः);
  op = p_व्यञ्जकम्->op;
  सचे( op==TK_IN ){
    प्रतिज्ञा( p_व्यञ्जकम्->p_दक्षिणः==0 );
    सचे( sqlite3_व्यञ्जकम्_Check_IN(pParse, p_व्यञ्जकम्) ) पटिनिवत्त;
    सचे( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
      pTerm->prereq_दक्षिणः = व्यञ्जकम्_Select_Usage(pMaskSet, p_व्यञ्जकम्->x.pSelect);
    }अञ्ञथा{
      pTerm->prereq_दक्षिणः = sqlite3_Where_व्यञ्जकम्_List_Usage(pMaskSet, p_व्यञ्जकम्->x.pList);
    }
    prereqAll = prereq_वामः | pTerm->prereq_दक्षिणः;
  }अञ्ञथा{
    pTerm->prereq_दक्षिणः = sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, p_व्यञ्जकम्->p_दक्षिणः);
    सचे( p_व्यञ्जकम्->p_वामः==0
     || व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_xIsSelect|EP_IfNullRow)
     || p_व्यञ्जकम्->x.pList!=0
    ){
      prereqAll = sqlite3_Where_व्यञ्जकम्_Usage_NN(pMaskSet, p_व्यञ्जकम्);
    }अञ्ञथा{
      prereqAll = prereq_वामः | pTerm->prereq_दक्षिणः;
    }
  }
  सचे( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  सचे( prereqAll!=sqlite3_Where_व्यञ्जकम्_Usage_NN(pMaskSet, p_व्यञ्जकम्) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_वृक्षः_View_व्यञ्जकम्(0,p_व्यञ्जकम्,0);
    प्रतिज्ञा( 0 );
  }
#endif

  सचे( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_व्यञ्जकम्->w.iJoin);
    सचे( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) ){
      prereqAll |= x;
      extra_दक्षिणः = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }अञ्ञथा सचे( (prereqAll>>1)>=x ){
      व्यञ्जकम्_Clear_Property(p_व्यञ्जकम्, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->वामः_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  सचे( allowedOp(op) ){
    पूर्णाङ्क aiCurCol[2];
    व्यञ्जकम् *p_वामः = sqlite3_व्यञ्जकम्_Skip_Collate(p_व्यञ्जकम्->p_वामः);
    व्यञ्जकम् *p_दक्षिणः = sqlite3_व्यञ्जकम्_Skip_Collate(p_व्यञ्जकम्->p_दक्षिणः);
    u16 opMask = (pTerm->prereq_दक्षिणः & prereq_वामः)==0 ? WO_ALL : WO_EQUIV;

    सचे( pTerm->u.x.iField>0 ){
      प्रतिज्ञा( op==TK_IN );
      प्रतिज्ञा( p_वामः->op==TK_VECTOR );
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_वामः) );
      p_वामः = p_वामः->x.pList->a[pTerm->u.x.iField-1].p_व्यञ्जकम्;
    }

    सचे( व्यञ्जकम्_Might_Be_Indexed(pSrc, aiCurCol, p_वामः, op) ){
      pTerm->वामः_Cursor = aiCurCol[0];
      प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.वामः_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    सचे( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    सचे( p_दक्षिणः
     && व्यञ्जकम्_Might_Be_Indexed(pSrc, aiCurCol, p_दक्षिणः, op)
     && !व्यञ्जकम्_Has_Property(p_दक्षिणः, EP_FixedCol)
    ){
      WhereTerm *pNew;
      व्यञ्जकम् *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      प्रतिज्ञा( pTerm->u.x.iField==0 );
      सचे( pTerm->वामः_Cursor>=0 ){
        पूर्णाङ्क idxNew;
        pDup = sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्, 0);
        सचे( db->mallocFailed ){
          sqlite3_व्यञ्जकम्_Delete(db, pDup);
          पटिनिवत्त;
        }
        idxNew = where_Clause_निवेशय(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        सचे( idxNew==0 ) पटिनिवत्त;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        सचे( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        प्रतिज्ञा( pWInfo->pTabList!=0 );
        सचे( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }अञ्ञथा{
        pDup = p_व्यञ्जकम्;
        pNew = pTerm;
      }
      pNew->wtFlags |= व्यञ्जकम्_Commute(pParse, pDup);
      pNew->वामः_Cursor = aiCurCol[0];
      प्रतिज्ञा( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.वामः_Column = aiCurCol[1];
      testcase( (prereq_वामः | extra_दक्षिणः) != prereq_वामः );
      pNew->prereq_दक्षिणः = prereq_वामः | extra_दक्षिणः;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }अञ्ञथा
    सचे( op==TK_ISNULL
     && !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_OuterON)
     && 0==sqlite3_व्यञ्जकम्_Can_Be_Null(p_वामः)
    ){
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      p_व्यञ्जकम्->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_व्यञ्जकम्->u.zToken = "false";
      व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, EP_IsFalse);
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
  अञ्ञथा सचे( p_व्यञ्जकम्->op==TK_BETWEEN && pWC->op==TK_AND ){
    व्यञ्जकम्_List *pList;
    पूर्णाङ्क i;
    स्थिर ध्रुव u8 ops[] = {TK_GE, TK_LE};
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
    pList = p_व्यञ्जकम्->x.pList;
    प्रतिज्ञा( pList!=0 );
    प्रतिज्ञा( pList->n_व्यञ्जकम्==2 );
    प्रतिज्ञा( pWC->a[idxTerm].nChild==0 );
    अत्थाय(i=0; i<2; i++){
      व्यञ्जकम् *p_New_व्यञ्जकम्;
      पूर्णाङ्क idxNew;
      p_New_व्यञ्जकम् = sqlite3PExpr(pParse, ops[i],
                             sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्->p_वामः, 0),
                             sqlite3_व्यञ्जकम्_Dup(db, pList->a[i].p_व्यञ्जकम्, 0));
      transferJoinMarkings(p_New_व्यञ्जकम्, p_व्यञ्जकम्);
      idxNew = where_Clause_निवेशय(pWC, p_New_व्यञ्जकम्, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      व्यञ्जकम्_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#सचे !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  अञ्ञथा सचे( p_व्यञ्जकम्->op==TK_OR && !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Collate) ){
    व्यञ्जकम् *pAlt = sqlite3_व्यञ्जकम्_Simplified_And_Or(p_व्यञ्जकम्);
    सचे( pAlt!=p_व्यञ्जकम् ){
      pTerm->p_व्यञ्जकम् = p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(pAlt);
      गच्छ व्यञ्जकम्_Analyze_restart;
    }
    प्रतिज्ञा( pWC->op==TK_AND );
    व्यञ्जकम्_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  अञ्ञथा सचे( p_व्यञ्जकम्->op==TK_NOTNULL ){
    सचे( p_व्यञ्जकम्->p_वामः->op==TK_COLUMN
     && p_व्यञ्जकम्->p_वामः->iColumn>=0
     && !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON)
    ){
      व्यञ्जकम् *p_New_व्यञ्जकम्;
      व्यञ्जकम् *p_वामः = p_व्यञ्जकम्->p_वामः;
      पूर्णाङ्क idxNew;
      WhereTerm *pNewTerm;
 
      p_New_व्यञ्जकम् = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_व्यञ्जकम्_Dup(db, p_वामः, 0),
                              sqlite3_व्यञ्जकम्_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_निवेशय(pWC, p_New_व्यञ्जकम्,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      सचे( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_दक्षिणः = 0;
        pNewTerm->वामः_Cursor = p_वामः->iTable;
        pNewTerm->u.x.वामः_Column = p_वामः->iColumn;
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
  अञ्ञथा सचे( p_व्यञ्जकम्->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_व्यञ्जकम्, &pStr1, &isComplete, &noCase)
  ){
    व्यञ्जकम् *p_वामः;       /* LHS of LIKE/GLOB operator */
    व्यञ्जकम् *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    व्यञ्जकम् *pNewExpr1;
    व्यञ्जकम् *pNewExpr2;
    पूर्णाङ्क idxNew1;
    पूर्णाङ्क idxNew2;
    ध्रुव वर्ण *zCollSeqName;     /* Name of collating sequence */
    ध्रुव u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
    p_वामः = p_व्यञ्जकम्->x.pList->a[1].p_व्यञ्जकम्;
    pStr2 = sqlite3_व्यञ्जकम्_Dup(db, pStr1, 0);
    प्रतिज्ञा( pStr1==0 || !व्यञ्जकम्_Has_Property(pStr1, EP_Int_मानम्) );
    प्रतिज्ञा( pStr2==0 || !व्यञ्जकम्_Has_Property(pStr2, EP_Int_मानम्) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    सचे( noCase && !pParse->db->mallocFailed ){
      पूर्णाङ्क i;
      वर्ण c;
      pTerm->wtFlags |= TERM_LIKE;
      अत्थाय(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    सचे( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      सचे( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        सचे( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      याव( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      प्रतिज्ञा( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_द्विमान;
    pNewExpr1 = sqlite3_व्यञ्जकम्_Dup(db, p_वामः, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_व्यञ्जकम्_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_व्यञ्जकम्);
    idxNew1 = where_Clause_निवेशय(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_व्यञ्जकम्_Dup(db, p_वामः, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_व्यञ्जकम्_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_व्यञ्जकम्);
    idxNew2 = where_Clause_निवेशय(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    व्यञ्जकम्_Analyze(pSrc, pWC, idxNew1);
    व्यञ्जकम्_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    सचे( isComplete ){
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
  सचे( (p_व्यञ्जकम्->op==TK_EQ || p_व्यञ्जकम्->op==TK_IS)
   && (n_वामः = sqlite3_व्यञ्जकम्_Vector_Size(p_व्यञ्जकम्->p_वामः))>1
   && sqlite3_व्यञ्जकम्_Vector_Size(p_व्यञ्जकम्->p_दक्षिणः)==n_वामः
   && ( (p_व्यञ्जकम्->p_वामः->flags & EP_xIsSelect)==0
     || (p_व्यञ्जकम्->p_दक्षिणः->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    पूर्णाङ्क i;
    अत्थाय(i=0; i<n_वामः; i++){
      पूर्णाङ्क idxNew;
      व्यञ्जकम् *pNew;
      व्यञ्जकम् *p_वामः = sqlite3_व्यञ्जकम्_For_Vector_Field(pParse, p_व्यञ्जकम्->p_वामः, i, n_वामः);
      व्यञ्जकम् *p_दक्षिणः = sqlite3_व्यञ्जकम्_For_Vector_Field(pParse, p_व्यञ्जकम्->p_दक्षिणः, i, n_वामः);

      pNew = sqlite3PExpr(pParse, p_व्यञ्जकम्->op, p_वामः, p_दक्षिणः);
      transferJoinMarkings(pNew, p_व्यञ्जकम्);
      idxNew = where_Clause_निवेशय(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      व्यञ्जकम्_Analyze(pSrc, pWC, idxNew);
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
  अञ्ञथा सचे( p_व्यञ्जकम्->op==TK_IN
   && pTerm->u.x.iField==0
   && p_व्यञ्जकम्->p_वामः->op==TK_VECTOR
   && ALWAYS( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) )
   && (p_व्यञ्जकम्->x.pSelect->pPrior==0 || (p_व्यञ्जकम्->x.pSelect->selFlags & SF_मानानि))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_व्यञ्जकम्->x.pSelect->pWin==0
#endif
   && (p_व्यञ्जकम्->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_व्यञ्जकम्->x.pSelect->pEList->n_व्यञ्जकम् <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    पूर्णाङ्क i;
    प्रतिज्ञा( pTerm->nChild==0 );
    अत्थाय(i=0; i<sqlite3_व्यञ्जकम्_Vector_Size(p_व्यञ्जकम्->p_वामः); i++){
      पूर्णाङ्क idxNew;
      idxNew = where_Clause_निवेशय(pWC, p_व्यञ्जकम्, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      व्यञ्जकम्_Analyze(pSrc, pWC, idxNew);
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
  अञ्ञथा सचे( pWC->op==TK_AND ){
    व्यञ्जकम् *p_दक्षिणः = 0, *p_वामः = 0;
    पूर्णाङ्क res = isAuxiliaryVtabOperator(db, p_व्यञ्जकम्, &eOp2, &p_वामः, &p_दक्षिणः);
    याव( res-- > 0 ){
      पूर्णाङ्क idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_व्यञ्जकम्;

      prereq_व्यञ्जकम् = sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, p_दक्षिणः);
      prereqColumn = sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, p_वामः);
      सचे( (prereq_व्यञ्जकम् & prereqColumn)==0 ){
        व्यञ्जकम् *p_New_व्यञ्जकम्;
        p_New_व्यञ्जकम् = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_व्यञ्जकम्_Dup(db, p_दक्षिणः, 0));
        सचे( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) && p_New_व्यञ्जकम् ){
          व्यञ्जकम्_Set_Property(p_New_व्यञ्जकम्, EP_OuterON);
          p_New_व्यञ्जकम्->w.iJoin = p_व्यञ्जकम्->w.iJoin;
        }
        idxNew = where_Clause_निवेशय(pWC, p_New_व्यञ्जकम्, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_दक्षिणः = prereq_व्यञ्जकम् | extra_दक्षिणः;
        pNewTerm->वामः_Cursor = p_वामः->iTable;
        pNewTerm->u.x.वामः_Column = p_वामः->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(व्यञ्जकम्*, p_वामः, p_दक्षिणः);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_दक्षिणः |= extra_दक्षिणः;
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
शून्य sqlite3WhereSplit(WhereClause *pWC, व्यञ्जकम् *p_व्यञ्जकम्, u8 op){
  व्यञ्जकम् *pE2 = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p_व्यञ्जकम्);
  pWC->op = op;
  प्रतिज्ञा( pE2!=0 || p_व्यञ्जकम्==0 );
  सचे( pE2==0 ) पटिनिवत्त;
  सचे( pE2->op!=op ){
    where_Clause_निवेशय(pWC, p_व्यञ्जकम्, 0);
  }अञ्ञथा{
    sqlite3WhereSplit(pWC, pE2->p_वामः, op);
    sqlite3WhereSplit(pWC, pE2->p_दक्षिणः, op);
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
स्थिर शून्य where_Add_Limit_व्यञ्जकम्(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  पूर्णाङ्क iReg,           /* Register that will hold value of the limit/offset */
  व्यञ्जकम् *p_व्यञ्जकम्,        /* Expression that defines the limit/offset */
  पूर्णाङ्क iCsr,           /* Cursor to which the constraint applies */
  पूर्णाङ्क eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  व्यञ्जकम् *pNew;
  पूर्णाङ्क iVal = 0;

  सचे( sqlite3_व्यञ्जकम्_Is_Integer(p_व्यञ्जकम्, &iVal, pParse, 0) && iVal>=0 ){
    व्यञ्जकम् *pVal = sqlite3_व्यञ्जकम्_Int32(db, iVal);
    सचे( pVal==0 ) पटिनिवत्त;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }अञ्ञथा{
    व्यञ्जकम् *pVal = sqlite3_व्यञ्जकम्_Alloc(db, TK_REGISTER, 0, 0);
    सचे( pVal==0 ) पटिनिवत्त;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  सचे( pNew ){
    WhereTerm *pTerm;
    पूर्णाङ्क idx;
    idx = where_Clause_निवेशय(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->वामः_Cursor = iCsr;
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
  सचे( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    व्यञ्जकम्_List *pOrderBy = p->pOrderBy;
    पूर्णाङ्क iCsr = p->pSrc->a[0].iCursor;
    पूर्णाङ्क ii;

    /* Check condition (4). Return early if it is not met. */
    अत्थाय(ii=0; ii<pWC->nTerm; ii++){
      सचे( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        प्रतिज्ञा( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        प्रतिज्ञा( pWC->a[ii].eOperator==WO_ROWVAL );
        अनुवर्त;
      }
      सचे( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        अनुवर्त;
      }
      सचे( pWC->a[ii].वामः_Cursor==iCsr && pWC->a[ii].prereq_दक्षिणः==0 ) अनुवर्त;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      सचे( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        सचे( pParent->वामः_Cursor==iCsr
         && pParent->prereq_दक्षिणः==0
         && pParent->nChild==1
        ){
          अनुवर्त;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      पटिनिवत्त;
    }

    /* Check condition (5). Return early if it is not met. */
    सचे( pOrderBy ){
      अत्थाय(ii=0; ii<pOrderBy->n_व्यञ्जकम्; ii++){
        व्यञ्जकम् *p_व्यञ्जकम् = pOrderBy->a[ii].p_व्यञ्जकम्;
        सचे( p_व्यञ्जकम्->op!=TK_COLUMN ) पटिनिवत्त;
        सचे( p_व्यञ्जकम्->iTable!=iCsr ) पटिनिवत्त;
        सचे( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) पटिनिवत्त;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    प्रतिज्ञा( p->pLimit->op==TK_LIMIT );
    सचे( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_व्यञ्जकम्(pWC, p->iOffset, p->pLimit->p_दक्षिणः,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    सचे( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_व्यञ्जकम्(pWC, p->iLimit, p->pLimit->p_वामः,
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
  सचे( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    पूर्णाङ्क i;
    /* Verify that every term past pWC->nBase is virtual */
    अत्थाय(i=pWC->nBase; i<pWC->nTerm; i++){
      प्रतिज्ञा( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    याव(1){
      प्रतिज्ञा( a->eMatchOp==0 || a->eOperator==WO_AUX );
      सचे( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_व्यञ्जकम्_Delete(db, a->p_व्यञ्जकम्);
      }
      सचे( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        सचे( a->wtFlags & TERM_ORINFO ){
          प्रतिज्ञा( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }अञ्ञथा{
          प्रतिज्ञा( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      सचे( a==aLast ) भङ्ग;
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
स्थिर SQLITE_NOINLINE Bitmask sqlite3_Where_व्यञ्जकम्_Usage_Full(
  WhereMaskSet *pMaskSet,
  व्यञ्जकम् *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  सचे( p->p_वामः ) mask |= sqlite3_Where_व्यञ्जकम्_Usage_NN(pMaskSet, p->p_वामः);
  सचे( p->p_दक्षिणः ){
    mask |= sqlite3_Where_व्यञ्जकम्_Usage_NN(pMaskSet, p->p_दक्षिणः);
    प्रतिज्ञा( p->x.pList==0 );
  }अञ्ञथा सचे( व्यञ्जकम्_Use_XSelect(p) ){
    सचे( व्यञ्जकम्_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= व्यञ्जकम्_Select_Usage(pMaskSet, p->x.pSelect);
  }अञ्ञथा सचे( p->x.pList ){
    mask |= sqlite3_Where_व्यञ्जकम्_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  सचे( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && व्यञ्जकम्_Use_YWin(p) ){
    प्रतिज्ञा( p->y.pWin!=0 );
    mask |= sqlite3_Where_व्यञ्जकम्_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_व्यञ्जकम्_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  पटिनिवत्त mask;
}
Bitmask sqlite3_Where_व्यञ्जकम्_Usage_NN(WhereMaskSet *pMaskSet, व्यञ्जकम् *p){
  सचे( p->op==TK_COLUMN && !व्यञ्जकम्_Has_Property(p, EP_FixedCol) ){
    पटिनिवत्त sqlite3WhereGetMask(pMaskSet, p->iTable);
  }अञ्ञथा सचे( व्यञ्जकम्_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    प्रतिज्ञा( p->op!=TK_IF_NULL_ROW );
    पटिनिवत्त 0;
  }
  पटिनिवत्त sqlite3_Where_व्यञ्जकम्_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_व्यञ्जकम्_Usage(WhereMaskSet *pMaskSet, व्यञ्जकम् *p){
  पटिनिवत्त p ? sqlite3_Where_व्यञ्जकम्_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_व्यञ्जकम्_List_Usage(WhereMaskSet *pMaskSet, व्यञ्जकम्_List *pList){
  पूर्णाङ्क i;
  Bitmask mask = 0;
  सचे( pList ){
    अत्थाय(i=0; i<pList->n_व्यञ्जकम्; i++){
      mask |= sqlite3_Where_व्यञ्जकम्_Usage(pMaskSet, pList->a[i].p_व्यञ्जकम्);
    }
  }
  पटिनिवत्त mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
शून्य sqlite3_Where_व्यञ्जकम्_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  पूर्णाङ्क i;
  अत्थाय(i=pWC->nTerm-1; i>=0; i--){
    व्यञ्जकम्_Analyze(pTabList, pWC, i);
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
  व्यञ्जकम्_List *pArgs;
  व्यञ्जकम् *pColRef;
  व्यञ्जकम् *pTerm;
  सचे( pItem->fg.isTabFunc==0 ) पटिनिवत्त;
  pTab = pItem->pSTab;
  प्रतिज्ञा( pTab!=0 );
  सचे( !IsVirtual(pTab) ){
    sqlite3_Error_सन्देशः(pParse, "'%s' is not a function", pItem->zName);
    पटिनिवत्त;
  }
  pArgs = pItem->u1.pFuncArg;
  सचे( pArgs==0 ) पटिनिवत्त;
  अत्थाय(j=k=0; j<pArgs->n_व्यञ्जकम्; j++){
    व्यञ्जकम् *pRhs;
    u32 joinType;
    याव( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    सचे( k>=pTab->nCol ){
      sqlite3_Error_सन्देशः(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      पटिनिवत्त;
    }
    pColRef = sqlite3_व्यञ्जकम्_Alloc(pParse->db, TK_COLUMN, 0, 0);
    सचे( pColRef==0 ) पटिनिवत्त;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    प्रतिज्ञा( व्यञ्जकम्_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_व्यञ्जकम्_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_व्यञ्जकम्_Dup(pParse->db, pArgs->a[j].p_व्यञ्जकम्, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    सचे( pItem->fg.jointype & (JT_वामः|JT_दक्षिणः) ){
      testcase( pItem->fg.jointype & JT_वामः );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_दक्षिणः ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }अञ्ञथा{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_व्यञ्जकम्(pTerm, pItem->iCursor, joinType);
    where_Clause_निवेशय(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
पूर्णाङ्क sqlite3WhereLoopBloomable(ध्रुव WhereLoop *pLoop){
  सचे( pLoop->wsFlags & WHERE_IPK ) पटिनिवत्त 1;
  सचे( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) पटिनिवत्त 0;
  पटिनिवत्त sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
