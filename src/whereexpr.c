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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "whereInt.h"

/* Forward declarations */
స్థిర శూన్య సమాసం_Analyze(SrcList*, WhereClause*, పూర్ణాంకం);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
స్థిర శూన్య whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
స్థిర శూన్య whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
స్థిర పూర్ణాంకం where_Clause_చొప్పించు(WhereClause *pWC, సమాసం *p, u16 wtFlags){
  Where_పదం *p_పదం;
  పూర్ణాంకం idx;
  testcase( wtFlags & పదం_VIRTUAL );
  ఒకవేళ( pWC->n_పదం>=pWC->n_స్థానం ){
    Where_పదం *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, పరిమాణం(pWC->a[0])*pWC->n_స్థానం*2 );
    ఒకవేళ( pWC->a==0 ){
      ఒకవేళ( wtFlags & పదం_DYNAMIC ){
        sqlite3_సమాసం_Delete(db, p);
      }
      pWC->a = pOld;
      తిరిగి 0;
    }
    జ్ఞాపకం_నకలు(pWC->a, pOld, పరిమాణం(pWC->a[0])*pWC->n_పదం);
    pWC->n_స్థానం = pWC->n_స్థానం*2;
  }
  p_పదం = &pWC->a[idx = pWC->n_పదం++];
  ఒకవేళ( (wtFlags & పదం_VIRTUAL)==0 ) pWC->n_పునాది = pWC->n_పదం;
  ఒకవేళ( p && సమాసం_Has_Property(p, EP_Unlikely) ){
    p_పదం->truthProb = sqlite3LogEst(p->i_పట్టిక) - 270;
  }లేకపోతే{
    p_పదం->truthProb = 1;
  }
  p_పదం->p_సమాసం = sqlite3_సమాసం_Skip_Collate_And_Likely(p);
  p_పదం->wtFlags = wtFlags;
  p_పదం->pWC = pWC;
  p_పదం->i_మాతృ = -1;
  memset(&p_పదం->eOperator, 0,
         పరిమాణం(Where_పదం) - offsetof(Where_పదం,eOperator));
  తిరిగి idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
స్థిర పూర్ణాంకం allowedOp(పూర్ణాంకం op){
  ప్రతిజ్ఞా( TK_GT>TK_EQ && TK_GT<TK_GE );
  ప్రతిజ్ఞా( TK_LT>TK_EQ && TK_LT<TK_GE );
  ప్రతిజ్ఞా( TK_LE>TK_EQ && TK_LE<TK_GE );
  ప్రతిజ్ఞా( TK_GE==TK_EQ+4 );
  ప్రతిజ్ఞా( TK_లోపల<TK_EQ );
  ప్రతిజ్ఞా( TK_IS<TK_EQ );
  ప్రతిజ్ఞా( TK_ISNULL<TK_EQ );
  ఒకవేళ( op>TK_GE ) తిరిగి 0;
  ఒకవేళ( op>=TK_EQ ) తిరిగి 1;
  తిరిగి op==TK_లోపల || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
స్థిర u16 సమాసం_Commute(Parse *pParse, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->p_ఎడమ->op==TK_సదిశ
   || p_సమాసం->p_కుడి->op==TK_సదిశ
   || sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, p_సమాసం->p_ఎడమ, p_సమాసం->p_కుడి) !=
      sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, p_సమాసం->p_కుడి, p_సమాసం->p_ఎడమ)
  ){
    p_సమాసం->flags ^= EP_Commuted;
  }
  మార్పిడి(సమాసం*,p_సమాసం->p_కుడి,p_సమాసం->p_ఎడమ);
  ఒకవేళ( p_సమాసం->op>=TK_GT ){
    ప్రతిజ్ఞా( TK_LT==TK_GT+2 );
    ప్రతిజ్ఞా( TK_GE==TK_LE+2 );
    ప్రతిజ్ఞా( TK_GT>TK_EQ );
    ప్రతిజ్ఞా( TK_GT<TK_LE );
    ప్రతిజ్ఞా( p_సమాసం->op>=TK_GT && p_సమాసం->op<=TK_GE );
    p_సమాసం->op = ((p_సమాసం->op-TK_GT)^2)+TK_GT;
  }
  తిరిగి 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
స్థిర u16 operator_ముసుగు(పూర్ణాంకం op){
  u16 c;
  ప్రతిజ్ఞా( allowedOp(op) );
  ఒకవేళ( op>=TK_EQ ){
    ప్రతిజ్ఞా( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }లేకపోతే ఒకవేళ( op==TK_లోపల ){
    c = WO_లోపల;
  }లేకపోతే ఒకవేళ( op==TK_ISNULL ){
    c = WO_ISNULL;
  }లేకపోతే{
    ప్రతిజ్ఞా( op==TK_IS );
    c = WO_IS;
  }
  ప్రతిజ్ఞా( op!=TK_ISNULL || c==WO_ISNULL );
  ప్రతిజ్ఞా( op!=TK_లోపల || c==WO_లోపల );
  ప్రతిజ్ఞా( op!=TK_EQ || c==WO_EQ );
  ప్రతిజ్ఞా( op!=TK_LT || c==WO_LT );
  ప్రతిజ్ఞా( op!=TK_LE || c==WO_LE );
  ప్రతిజ్ఞా( op!=TK_GT || c==WO_GT );
  ప్రతిజ్ఞా( op!=TK_GE || c==WO_GE );
  ప్రతిజ్ఞా( op!=TK_IS || c==WO_IS );
  తిరిగి c;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_LIKE_OPTIMIZATION
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
స్థిర పూర్ణాంకం isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  సమాసం *p_సమాసం,      /* Test this expression */
  సమాసం **pp_ఉపసర్గ,  /* Pointer to TK_STRING expression with pattern prefix */
  పూర్ణాంకం *pisComplete, /* True if the only wildcard is % in the last character */
  పూర్ణాంకం *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  స్థిరరాశి u8 *z = 0;           /* String on RHS of LIKE operator */
  సమాసం *p_కుడి, *p_ఎడమ;      /* Right and left size of LIKE operator */
  సమాసం_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  పూర్ణాంకం cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_విలువ *pVal = 0;
  పూర్ణాంకం op;                    /* Opcode of pRight */
  పూర్ణాంకం rc;                    /* Result code to return */

  ఒకవేళ( !sqlite3IsLikeFunction(db, p_సమాసం, pnoCase, (అక్షరం*)wc) ){
    తిరిగి 0;
  }
#నిర్వచితమైతే SQLITE_EBCDIC
  ఒకవేళ( *pnoCase ) తిరిగి 0;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
  pList = p_సమాసం->x.pList;
  p_ఎడమ = pList->a[1].p_సమాసం;

  p_కుడి = sqlite3_సమాసం_Skip_Collate(pList->a[0].p_సమాసం);
  op = p_కుడి->op;
  ఒకవేళ( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    పూర్ణాంకం i_నిలువు = p_కుడి->i_స్తంభం;
    pVal = sqlite3_Vdbe_Get_Bound_విలువ(pReprepare, i_నిలువు, SQLITE_AFF_BLOB);
    ఒకవేళ( pVal && sqlite3_విలువ_type(pVal)==SQLITE_పాఠ్యం ){
      z = sqlite3_విలువ_పాఠ్యం(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, i_నిలువు, 0);
    ప్రతిజ్ఞా( p_కుడి->op==TK_VARIABLE || p_కుడి->op==TK_రిజిస్టరు );
  }లేకపోతే ఒకవేళ( op==TK_STRING ){
    ప్రతిజ్ఞా( !సమాసం_Has_Property(p_కుడి, EP_Int_విలువ) );
     z = (u8*)p_కుడి->u.zToken;
  }
  ఒకవేళ( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    అయితే( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      ఒకవేళ( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }లేకపోతే ఒకవేళ( c>=0x80 ){
        స్థిరరాశి u8 *z2 = z+cnt-1;
        ఒకవేళ( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          ఆపు;
        }లేకపోతే{
          cnt = (పూర్ణాంకం)(z2-z);
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
    ఒకవేళ( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      సమాసం *p_ఉపసర్గ;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      p_ఉపసర్గ = sqlite3_సమాసం(db, TK_STRING, (అక్షరం*)z);
      ఒకవేళ( p_ఉపసర్గ ){
        పూర్ణాంకం iFrom, i_గమ్యం;
        అక్షరం *zNew;
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_ఉపసర్గ, EP_Int_విలువ) );
        zNew = p_ఉపసర్గ->u.zToken;
        zNew[cnt] = 0;
        కొరకు(iFrom=i_గమ్యం=0; iFrom<cnt; iFrom++){
          ఒకవేళ( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[i_గమ్యం++] = zNew[iFrom];
        }
        zNew[i_గమ్యం] = 0;
        ప్రతిజ్ఞా( i_గమ్యం>0 );

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
        ఒకవేళ( p_ఎడమ->op!=TK_స్తంభం
         || sqlite3_సమాసం_Affinity(p_ఎడమ)!=SQLITE_AFF_పాఠ్యం
         || (ALWAYS( సమాసం_Use_YTab(p_ఎడమ) )
             && ALWAYS(p_ఎడమ->y.pTab)
             && IsVirtual(p_ఎడమ->y.pTab))  /* Might be numeric */
        ){
          పూర్ణాంకం is_అంకె;
          ద్విగుణ rDummy;
          ప్రతిజ్ఞా( zNew[i_గమ్యం]==0 );
          is_అంకె = sqlite3AtoF(zNew, &rDummy);
          ఒకవేళ( is_అంకె<=0 ){
            ఒకవేళ( i_గమ్యం==1 && zNew[0]=='-' ){
              is_అంకె = +1;
            }లేకపోతే{
              zNew[i_గమ్యం-1]++;
              is_అంకె = sqlite3AtoF(zNew, &rDummy);
              zNew[i_గమ్యం-1]--;
            }
          }
          ఒకవేళ( is_అంకె>0 ){
            sqlite3_సమాసం_Delete(db, p_ఉపసర్గ);
            sqlite3_విలువ_Free(pVal);
            తిరిగి 0;
          }
        }
      }
      *pp_ఉపసర్గ = p_ఉపసర్గ;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      ఒకవేళ( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_కుడి->i_స్తంభం, 0);
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_కుడి, EP_Int_విలువ) );
        ఒకవేళ( *pisComplete && p_కుడి->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
          sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, p_కుడి, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
        }
      }
    }లేకపోతే{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_విలువ_Free(pVal);
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
పూర్ణాంకం sqlite3_సమాసం_Is_Like_Operator(స్థిరరాశి సమాసం *p_సమాసం){
  స్థిర స్థిరరాశి నిర్మాణం {
    స్థిరరాశి అక్షరం *zOp;
    గుర్తులేని అక్షరం eOp;
  } aOp[] = {
    { "match",  SQLITE_సూచీ_CONSTRAINT_సరిపోలిక },
    { "glob",   SQLITE_సూచీ_CONSTRAINT_GLOB },
    { "like",   SQLITE_సూచీ_CONSTRAINT_LIKE },
    { "regexp", SQLITE_సూచీ_CONSTRAINT_REGEXP }
  };
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( p_సమాసం->op==TK_FUNCTION );
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
  కొరకు(i=0; i<Array_కొలత(aOp); i++){
    ఒకవేళ( sqlite3StrICmp(p_సమాసం->u.zToken, aOp[i].zOp)==0 ){
      తిరిగి aOp[i].eOp;
    }
  }
  తిరిగి 0;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
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
స్థిర పూర్ణాంకం isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  సమాసం *p_సమాసం,                    /* Test this expression */
  గుర్తులేని అక్షరం *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  సమాసం **pp_ఎడమ,                  /* Column expression to left of MATCH/op2 */
  సమాసం **pp_కుడి                  /* Expression to left of MATCH/op2 */
){
  ఒకవేళ( p_సమాసం->op==TK_FUNCTION ){
    సమాసం_List *pList;
    సమాసం *p_నిలువు;                     /* Column reference */
    పూర్ణాంకం i;

    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
    pList = p_సమాసం->x.pList;
    ఒకవేళ( pList==0 || pList->n_సమాసం!=2 ){
      తిరిగి 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    p_నిలువు = pList->a[1].p_సమాసం;
    ప్రతిజ్ఞా( p_నిలువు->op!=TK_స్తంభం || (సమాసం_Use_YTab(p_నిలువు) && p_నిలువు->y.pTab!=0) );
    ఒకవేళ( సమాసం_Is_Vtab(p_నిలువు) && (i = sqlite3_సమాసం_Is_Like_Operator(p_సమాసం))!=0 ){
      *peOp2 = i;
      *pp_కుడి = pList->a[0].p_సమాసం;
      *pp_ఎడమ = p_నిలువు;
      తిరిగి 1;
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
    p_నిలువు = pList->a[0].p_సమాసం;
    ప్రతిజ్ఞా( p_నిలువు->op!=TK_స్తంభం || సమాసం_Use_YTab(p_నిలువు) );
    ప్రతిజ్ఞా( p_నిలువు->op!=TK_స్తంభం || (సమాసం_Use_YTab(p_నిలువు) && p_నిలువు->y.pTab!=0) );
    ఒకవేళ( సమాసం_Is_Vtab(p_నిలువు) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      శూన్య (*x_Not_వాడిన)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**);
      శూన్య *p_Not_వాడిన;
      pVtab = sqlite3GetVTable(db, p_నిలువు->y.pTab)->pVtab;
      ప్రతిజ్ఞా( pVtab!=0 );
      ప్రతిజ్ఞా( pVtab->pModule!=0 );
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      pMod = (sqlite3_module *)pVtab->pModule;
      ఒకవేళ( pMod->x_కనుగొను_Function!=0 ){
        i = pMod->x_కనుగొను_Function(pVtab,2, p_సమాసం->u.zToken, &x_Not_వాడిన, &p_Not_వాడిన);
        ఒకవేళ( i>=SQLITE_సూచీ_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_కుడి = pList->a[1].p_సమాసం;
          *pp_ఎడమ = p_నిలువు;
          తిరిగి 1;
        }
      }
    }
  }లేకపోతే ఒకవేళ( p_సమాసం->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    ప్రతిజ్ఞా( TK_NE < TK_EQ );
    ప్రతిజ్ఞా( TK_ISNOT < TK_EQ );
    ప్రతిజ్ఞా( TK_NOTNULL < TK_EQ );
    తిరిగి 0;
  }లేకపోతే ఒకవేళ( p_సమాసం->op==TK_NE || p_సమాసం->op==TK_ISNOT || p_సమాసం->op==TK_NOTNULL ){
    పూర్ణాంకం res = 0;
    సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
    సమాసం *p_కుడి = p_సమాసం->p_కుడి;
    ప్రతిజ్ఞా( p_ఎడమ->op!=TK_స్తంభం || (సమాసం_Use_YTab(p_ఎడమ) && p_ఎడమ->y.pTab!=0) );
    ఒకవేళ( సమాసం_Is_Vtab(p_ఎడమ) ){
      res++;
    }
    ప్రతిజ్ఞా( p_కుడి==0 || p_కుడి->op!=TK_స్తంభం
            || (సమాసం_Use_YTab(p_కుడి) && p_కుడి->y.pTab!=0) );
    ఒకవేళ( p_కుడి && సమాసం_Is_Vtab(p_కుడి) ){
      res++;
      మార్పిడి(సమాసం*, p_ఎడమ, p_కుడి);
    }
    *pp_ఎడమ = p_ఎడమ;
    *pp_కుడి = p_కుడి;
    ఒకవేళ( p_సమాసం->op==TK_NE ) *peOp2 = SQLITE_సూచీ_CONSTRAINT_NE;
    ఒకవేళ( p_సమాసం->op==TK_ISNOT ) *peOp2 = SQLITE_సూచీ_CONSTRAINT_ISNOT;
    ఒకవేళ( p_సమాసం->op==TK_NOTNULL ) *peOp2 = SQLITE_సూచీ_CONSTRAINT_ISNOTNULL;
    తిరిగి res;
  }
  తిరిగి 0;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
స్థిర శూన్య transferJoinMarkings(సమాసం *pDerived, సమాసం *p_పునాది){
  ఒకవేళ( pDerived && సమాసం_Has_Property(p_పునాది, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= p_పునాది->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = p_పునాది->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
స్థిర శూన్య mark_పదం_As_బిడ్డ(WhereClause *pWC, పూర్ణాంకం i_బిడ్డ, పూర్ణాంకం i_మాతృ){
  pWC->a[i_బిడ్డ].i_మాతృ = i_మాతృ;
  pWC->a[i_బిడ్డ].truthProb = pWC->a[i_మాతృ].truthProb;
  ప్రతిజ్ఞా(   pWC->a[i_మాతృ].n_బిడ్డ <  UMXV(pWC->a[0].n_బిడ్డ) );
  pWC->a[i_మాతృ].n_బిడ్డ++;
  testcase( pWC->a[i_మాతృ].n_బిడ్డ == UMXV(pWC->a[0].n_బిడ్డ) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
స్థిర Where_పదం *whereNthSubterm(Where_పదం *p_పదం, పూర్ణాంకం N){
  ఒకవేళ( p_పదం->eOperator!=WO_AND ){
    తిరిగి N==0 ? p_పదం : 0;
  }
  ఒకవేళ( N<p_పదం->u.pAndInfo->wc.n_పదం ){
    తిరిగి &p_పదం->u.pAndInfo->wc.a[N];
  }
  తిరిగి 0;
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
స్థిర శూన్య whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  Where_పదం *pOne,       /* First disjunct */
  Where_పదం *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  సమాసం *pNew;            /* New virtual expression */
  పూర్ణాంకం op;                /* Operator for the combined expression */
  పూర్ణాంకం idxNew;            /* Index in pWC of the next virtual term */
  సమాసం *pA, *pB;         /* Expressions associated with pOne and pTwo */

  ఒకవేళ( (pOne->wtFlags | pTwo->wtFlags) & పదం_VNULL ) తిరిగి;
  ఒకవేళ( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) తిరిగి;
  ఒకవేళ( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) తిరిగి;
  ఒకవేళ( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) తిరిగి;
  pA = pOne->p_సమాసం;
  pB = pTwo->p_సమాసం;
  ప్రతిజ్ఞా( pA->p_ఎడమ!=0 && pA->p_కుడి!=0 );
  ప్రతిజ్ఞా( pB->p_ఎడమ!=0 && pB->p_కుడి!=0 );
  ఒకవేళ( sqlite3_సమాసం_Compare(0,pA->p_ఎడమ, pB->p_ఎడమ, -1) )  తిరిగి;
  ఒకవేళ( sqlite3_సమాసం_Compare(0,pA->p_కుడి, pB->p_కుడి,-1) ) తిరిగి;
  ఒకవేళ( సమాసం_Has_Property(pA,EP_Commuted)!=సమాసం_Has_Property(pB,EP_Commuted) ){
    తిరిగి;
  }
  /* If we reach this point, it means the two subterms can be combined */
  ఒకవేళ( (eOp & (eOp-1))!=0 ){
    ఒకవేళ( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }లేకపోతే{
      ప్రతిజ్ఞా( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_సమాసం_Dup(db, pA, 0);
  ఒకవేళ( pNew==0 ) తిరిగి;
  కొరకు(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ ప్రతిజ్ఞా( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_చొప్పించు(pWC, pNew, పదం_VIRTUAL|పదం_DYNAMIC);
  సమాసం_Analyze(pSrc, pWC, idxNew);
}

#ఒకవేళ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
స్థిర శూన్య సమాసం_Analyze_Or_పదం(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  పూర్ణాంకం idx_పదం               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  Where_పదం *p_పదం = &pWC->a[idx_పదం];    /* The term to be analyzed */
  సమాసం *p_సమాసం = p_పదం->p_సమాసం;             /* The expression of the term */
  పూర్ణాంకం i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  Where_పదం *p_Or_పదం;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chng_గమ్యం_లోపల;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  ప్రతిజ్ఞా( (p_పదం->wtFlags & (పదం_DYNAMIC|పదం_ORINFO|పదం_ANDINFO))==0 );
  ప్రతిజ్ఞా( p_సమాసం->op==TK_OR );
  p_పదం->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, పరిమాణం(*pOrInfo));
  ఒకవేళ( pOrInfo==0 ) తిరిగి;
  p_పదం->wtFlags |= పదం_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, పరిమాణం(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3_Where_విభజించు(pOrWc, p_సమాసం, TK_OR);
  sqlite3_Where_సమాసం_Analyze(pSrc, pOrWc);
  ఒకవేళ( db->mallocFailed ) తిరిగి;
  ప్రతిజ్ఞా( pOrWc->n_పదం>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chng_గమ్యం_లోపల = ~(Bitmask)0;
  కొరకు(i=pOrWc->n_పదం-1, p_Or_పదం=pOrWc->a; i>=0 && indexable; i--, p_Or_పదం++){
    ఒకవేళ( (p_Or_పదం->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      ప్రతిజ్ఞా( (p_Or_పదం->wtFlags & (పదం_ANDINFO|పదం_ORINFO))==0 );
      chng_గమ్యం_లోపల = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, పరిమాణం(*pAndInfo));
      ఒకవేళ( pAndInfo ){
        WhereClause *pAndWC;
        Where_పదం *p_And_పదం;
        పూర్ణాంకం j;
        Bitmask b = 0;
        p_Or_పదం->u.pAndInfo = pAndInfo;
        p_Or_పదం->wtFlags |= పదం_ANDINFO;
        p_Or_పదం->eOperator = WO_AND;
        p_Or_పదం->ఎడమ_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, పరిమాణం(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3_Where_విభజించు(pAndWC, p_Or_పదం->p_సమాసం, TK_AND);
        sqlite3_Where_సమాసం_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        ఒకవేళ( !db->mallocFailed ){
          కొరకు(j=0, p_And_పదం=pAndWC->a; j<pAndWC->n_పదం; j++, p_And_పదం++){
            ప్రతిజ్ఞా( p_And_పదం->p_సమాసం );
            ఒకవేళ( allowedOp(p_And_పదం->p_సమాసం->op)
             || p_And_పదం->eOperator==WO_AUX
            ){
              b |= sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి, p_And_పదం->ఎడమ_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }లేకపోతే ఒకవేళ( p_Or_పదం->wtFlags & పదం_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }లేకపోతే{
      Bitmask b;
      b = sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి, p_Or_పదం->ఎడమ_Cursor);
      ఒకవేళ( p_Or_పదం->wtFlags & పదం_VIRTUAL ){
        Where_పదం *pOther = &pOrWc->a[p_Or_పదం->i_మాతృ];
        b |= sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి, pOther->ఎడమ_Cursor);
      }
      indexable &= b;
      ఒకవేళ( (p_Or_పదం->eOperator & WO_EQ)==0 ){
        chng_గమ్యం_లోపల = 0;
      }లేకపోతే{
        chng_గమ్యం_లోపల &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  p_పదం->eOperator = WO_OR;
  p_పదం->ఎడమ_Cursor = -1;
  ఒకవేళ( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  ఒకవేళ( indexable && pOrWc->n_పదం==2 ){
    పూర్ణాంకం iOne = 0;
    Where_పదం *pOne;
    అయితే( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      పూర్ణాంకం iTwo = 0;
      Where_పదం *pTwo;
      అయితే( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  ఒకవేళ( chng_గమ్యం_లోపల ){
    పూర్ణాంకం ok_గమ్యం_Chng_గమ్యం_లోపల = 0;     /* True if the conversion to IN is valid */
    పూర్ణాంకం i_స్తంభం = -1;         /* Column index on lhs of IN operator */
    పూర్ణాంకం iCursor = -1;         /* Table cursor common to all terms */
    పూర్ణాంకం j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    కొరకు(j=0; j<2 && !ok_గమ్యం_Chng_గమ్యం_లోపల; j++){
      సమాసం *p_ఎడమ = 0;
      p_Or_పదం = pOrWc->a;
      కొరకు(i=pOrWc->n_పదం-1; i>=0; i--, p_Or_పదం++){
        ప్రతిజ్ఞా( p_Or_పదం->eOperator & WO_EQ );
        p_Or_పదం->wtFlags &= ~పదం_OK;
        ఒకవేళ( p_Or_పదం->ఎడమ_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          ప్రతిజ్ఞా( j==1 );
          కొనసాగు;
        }
        ఒకవేళ( (chng_గమ్యం_లోపల & sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి,
                                            p_Or_పదం->ఎడమ_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( p_Or_పదం->wtFlags & పదం_COPIED );
          testcase( p_Or_పదం->wtFlags & పదం_VIRTUAL );
          ప్రతిజ్ఞా( p_Or_పదం->wtFlags & (పదం_COPIED|పదం_VIRTUAL) );
          కొనసాగు;
        }
        ప్రతిజ్ఞా( (p_Or_పదం->eOperator & (WO_OR|WO_AND))==0 );
        i_స్తంభం = p_Or_పదం->u.x.ఎడమ_స్తంభం;
        iCursor = p_Or_పదం->ఎడమ_Cursor;
        p_ఎడమ = p_Or_పదం->p_సమాసం->p_ఎడమ;
        ఆపు;
      }
      ఒకవేళ( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        ప్రతిజ్ఞా( j==1 );
        ప్రతిజ్ఞా( Is_Power_యొక్క_Two(chng_గమ్యం_లోపల) );
        ప్రతిజ్ఞా( chng_గమ్యం_లోపల==sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి, iCursor) );
        ఆపు;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      ok_గమ్యం_Chng_గమ్యం_లోపల = 1;
      కొరకు(; i>=0 && ok_గమ్యం_Chng_గమ్యం_లోపల; i--, p_Or_పదం++){
        ప్రతిజ్ఞా( p_Or_పదం->eOperator & WO_EQ );
        ప్రతిజ్ఞా( (p_Or_పదం->eOperator & (WO_OR|WO_AND))==0 );
        ఒకవేళ( p_Or_పదం->ఎడమ_Cursor!=iCursor ){
          p_Or_పదం->wtFlags &= ~పదం_OK;
        }లేకపోతే ఒకవేళ( p_Or_పదం->u.x.ఎడమ_స్తంభం!=i_స్తంభం || (i_స్తంభం==XN_సమాసం
               && sqlite3_సమాసం_Compare(pParse, p_Or_పదం->p_సమాసం->p_ఎడమ, p_ఎడమ, -1)
        )){
          ok_గమ్యం_Chng_గమ్యం_లోపల = 0;
        }లేకపోతే{
          పూర్ణాంకం aff_ఎడమ, aff_కుడి;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_కుడి = sqlite3_సమాసం_Affinity(p_Or_పదం->p_సమాసం->p_కుడి);
          aff_ఎడమ = sqlite3_సమాసం_Affinity(p_Or_పదం->p_సమాసం->p_ఎడమ);
          ఒకవేళ( aff_కుడి!=0 && aff_కుడి!=aff_ఎడమ ){
            ok_గమ్యం_Chng_గమ్యం_లోపల = 0;
          }లేకపోతే{
            p_Or_పదం->wtFlags |= పదం_OK;
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
    ఒకవేళ( ok_గమ్యం_Chng_గమ్యం_లోపల ){
      సమాసం *pDup;            /* A transient duplicate expression */
      సమాసం_List *pList = 0;   /* The RHS of the IN operator */
      సమాసం *p_ఎడమ = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      సమాసం *pNew;            /* The complete IN operator */

      కొరకు(i=pOrWc->n_పదం-1, p_Or_పదం=pOrWc->a; i>=0; i--, p_Or_పదం++){
        సమాసం *pThis;
        ఒకవేళ( (p_Or_పదం->wtFlags & పదం_OK)==0 ) కొనసాగు;
        ప్రతిజ్ఞా( p_Or_పదం->eOperator & WO_EQ );
        ప్రతిజ్ఞా( (p_Or_పదం->eOperator & (WO_OR|WO_AND))==0 );
        ప్రతిజ్ఞా( p_Or_పదం->ఎడమ_Cursor==iCursor );
        ప్రతిజ్ఞా( p_Or_పదం->u.x.ఎడమ_స్తంభం==i_స్తంభం );
        pThis = p_Or_పదం->p_సమాసం;
        pDup = sqlite3_సమాసం_Dup(db, pThis->p_కుడి, 0);
        pList = sqlite3_సమాసం_List_Append(pWInfo->pParse, pList, pDup);
        ఒకవేళ( p_ఎడమ==0 ){
          p_ఎడమ = pThis->p_ఎడమ;
          pCollSeq = sqlite3_సమాసం_Compare_Coll_Seq(pParse, pThis);
        }లేకపోతే{
          ప్రతిజ్ఞా( 0==sqlite3_సమాసం_Compare(pParse, 
                        sqlite3_సమాసం_Skip_Collate(pThis->p_ఎడమ),
                        sqlite3_సమాసం_Skip_Collate(p_ఎడమ), -1) );
          ఒకవేళ( pCollSeq!=sqlite3_సమాసం_Compare_Coll_Seq(pParse, pThis) ){
            p_ఎడమ = 0;  /* Collating sequence mismatch */
            ఆపు;
          }
        }
      }
      ఒకవేళ( p_ఎడమ==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }లేకపోతే{
        pDup = sqlite3_సమాసం_Dup(db, p_ఎడమ, 0);
        ఒకవేళ( sqlite3_సమాసం_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          ప్రతిజ్ఞా( pCollSeq->zName!=0 );
          pDup = sqlite3_సమాసం_కూడు_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_లోపల, pDup, 0);
      }
      ఒకవేళ( pNew ){
        పూర్ణాంకం idxNew;
        transferJoinMarkings(pNew, p_సమాసం);
        ప్రతిజ్ఞా( సమాసం_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_చొప్పించు(pWC, pNew, పదం_VIRTUAL|పదం_DYNAMIC);
        testcase( idxNew==0 );
        సమాసం_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        mark_పదం_As_బిడ్డ(pWC, idxNew, idx_పదం);
      }లేకపోతే{
        sqlite3_సమాసం_List_Delete(db, pList);
      }
    }
  }
}
#అయితే_ముగింపు /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

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
స్థిర పూర్ణాంకం పదం_Is_Equivalence(Parse *pParse, సమాసం *p_సమాసం, SrcList *pSrc){
  అక్షరం aff1, aff2;
  ఒకవేళ( !OptimizationEnabled(pParse->db, SQLITE_సంక్రమణ) ) తిరిగి 0;  /* (1) */
  ఒకవేళ( p_సమాసం->op!=TK_EQ && p_సమాసం->op!=TK_IS ) తిరిగి 0;                 /* (2) */
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON|EP_Collate) ) తిరిగి 0;        /* (3) */
  ప్రతిజ్ఞా( pSrc!=0 );
  ఒకవేళ( p_సమాసం->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    తిరిగి 0;                                                          /* (4) */
  }
  aff1 = sqlite3_సమాసం_Affinity(p_సమాసం->p_ఎడమ);
  aff2 = sqlite3_సమాసం_Affinity(p_సమాసం->p_కుడి);
  ఒకవేళ( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    తిరిగి 0;                                                          /* (5) */
  }
  ఒకవేళ( !sqlite3_సమాసం_Coll_Seq_సరిపోలిక(pParse, p_సమాసం->p_ఎడమ, p_సమాసం->p_కుడి) ){
    తిరిగి 0;                                                          /* (6) */
  }
  తిరిగి 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
స్థిర Bitmask సమాసం_Select_Usage(Where_ముసుగు_సమితి *p_ముసుగు_సమితి, Select *pS){
  Bitmask ముసుగు = 0;
  అయితే( pS ){
    SrcList *pSrc = pS->pSrc;
    ముసుగు |= sqlite3_Where_సమాసం_List_Usage(p_ముసుగు_సమితి, pS->pEList);
    ముసుగు |= sqlite3_Where_సమాసం_List_Usage(p_ముసుగు_సమితి, pS->p_సమూహం_By);
    ముసుగు |= sqlite3_Where_సమాసం_List_Usage(p_ముసుగు_సమితి, pS->p_క్రమం_By);
    ముసుగు |= sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, pS->pWhere);
    ముసుగు |= sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, pS->pHaving);
    ఒకవేళ( ALWAYS(pSrc!=0) ){
      పూర్ణాంకం i;
      కొరకు(i=0; i<pSrc->nSrc; i++){
        ఒకవేళ( pSrc->a[i].fg.isSubquery ){
          ముసుగు |= సమాసం_Select_Usage(p_ముసుగు_సమితి, pSrc->a[i].u4.pSubq->pSelect);
        }
        ఒకవేళ( pSrc->a[i].fg.isUsing==0 ){
          ముసుగు |= sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, pSrc->a[i].u3.pOn);
        }
        ఒకవేళ( pSrc->a[i].fg.isTabFunc ){
          ముసుగు |= sqlite3_Where_సమాసం_List_Usage(p_ముసుగు_సమితి, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->p_పూర్వ;
  }
  తిరిగి ముసుగు;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం సమాసం_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  పూర్ణాంకం *ai_Cur_నిలువు,         /* Write the referenced table cursor and column here */
  సమాసం *p_సమాసం,           /* An operand of a comparison operator */
  పూర్ణాంకం j                  /* Start looking with the j-th pFrom entry */
){
  సూచీ *pIdx;
  పూర్ణాంకం i;
  పూర్ణాంకం iCur;
  చేయి{
    iCur = pFrom->a[j].iCursor;
    కొరకు(pIdx=pFrom->a[j].pSTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
      ఒకవేళ( pIdx->a_నిలువు_సమాసం==0 ) కొనసాగు;
      కొరకు(i=0; i<pIdx->n_కీలకం_నిలువు; i++){
        ఒకవేళ( pIdx->ai_స్తంభం[i]!=XN_సమాసం ) కొనసాగు;
        ప్రతిజ్ఞా( pIdx->b_Has_సమాసం );
        ఒకవేళ( sqlite3_సమాసం_Compare_Skip(p_సమాసం,pIdx->a_నిలువు_సమాసం->a[i].p_సమాసం,iCur)==0
         && !sqlite3_సమాసం_Is_Constant(0,pIdx->a_నిలువు_సమాసం->a[i].p_సమాసం)
        ){
          ai_Cur_నిలువు[0] = iCur;
          ai_Cur_నిలువు[1] = XN_సమాసం;
          తిరిగి 1;
        }
      }
    }
  }అయితే( ++j < pFrom->nSrc );
  తిరిగి 0;
}
స్థిర పూర్ణాంకం సమాసం_Might_Be_సూచీకృత(
  SrcList *pFrom,        /* The FROM clause */
  పూర్ణాంకం *ai_Cur_నిలువు,         /* Write the referenced table cursor & column here */
  సమాసం *p_సమాసం,           /* An operand of a comparison operator */
  పూర్ణాంకం op                 /* The specific comparison operator */
){
  పూర్ణాంకం i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  ప్రతిజ్ఞా( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  ప్రతిజ్ఞా( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_లోపల<TK_GE );
  ప్రతిజ్ఞా( op<=TK_GE );
  ఒకవేళ( p_సమాసం->op==TK_సదిశ && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
    p_సమాసం = p_సమాసం->x.pList->a[0].p_సమాసం;
  }

  ఒకవేళ( p_సమాసం->op==TK_స్తంభం ){
    ai_Cur_నిలువు[0] = p_సమాసం->i_పట్టిక;
    ai_Cur_నిలువు[1] = p_సమాసం->i_స్తంభం;
    తిరిగి 1;
  }

  కొరకు(i=0; i<pFrom->nSrc; i++){
    సూచీ *pIdx;
    కొరకు(pIdx=pFrom->a[i].pSTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
      ఒకవేళ( pIdx->a_నిలువు_సమాసం ){
        తిరిగి సమాసం_Might_Be_Indexed2(pFrom,ai_Cur_నిలువు,p_సమాసం,i);
      }
    }
  }
  తిరిగి 0;
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
స్థిర శూన్య సమాసం_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  పూర్ణాంకం idx_పదం               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  Where_పదం *p_పదం;                /* The term to be analyzed */
  Where_ముసుగు_సమితి *p_ముసుగు_సమితి;          /* Set of table index masks */
  సమాసం *p_సమాసం;                     /* The expression to be analyzed */
  Bitmask prereq_ఎడమ;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_కుడి = 0;          /* Extra dependencies on LEFT JOIN */
  సమాసం *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  పూర్ణాంకం isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  పూర్ణాంకం noCase = 0;                  /* uppercase equivalent to lowercase */
  పూర్ణాంకం op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  గుర్తులేని అక్షరం eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  పూర్ణాంకం n_ఎడమ;                       /* Number of elements on left side vector */

  ఒకవేళ( db->mallocFailed ){
    తిరిగి;
  }
  ప్రతిజ్ఞా( pWC->n_పదం > idx_పదం );
  p_పదం = &pWC->a[idx_పదం];
#నిర్వచితమైతే SQLITE_DEBUG
  p_పదం->i_పదం = idx_పదం;
#అయితే_ముగింపు
  p_ముసుగు_సమితి = &pWInfo->s_ముసుగు_సమితి;
  p_సమాసం = p_పదం->p_సమాసం;
  ప్రతిజ్ఞా( p_సమాసం!=0 ); /* Because malloc() has not failed */
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_AS && p_సమాసం->op!=TK_COLLATE );
సమాసం_Analyze_restart:
  p_ముసుగు_సమితి->bVarSelect = 0;
  prereq_ఎడమ = sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, p_సమాసం->p_ఎడమ);
  op = p_సమాసం->op;
  ఒకవేళ( op==TK_లోపల ){
    ప్రతిజ్ఞా( p_సమాసం->p_కుడి==0 );
    ఒకవేళ( sqlite3_సమాసం_Check_లోపల(pParse, p_సమాసం) ) తిరిగి;
    ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
      p_పదం->prereq_కుడి = సమాసం_Select_Usage(p_ముసుగు_సమితి, p_సమాసం->x.pSelect);
    }లేకపోతే{
      p_పదం->prereq_కుడి = sqlite3_Where_సమాసం_List_Usage(p_ముసుగు_సమితి, p_సమాసం->x.pList);
    }
    prereqAll = prereq_ఎడమ | p_పదం->prereq_కుడి;
  }లేకపోతే{
    p_పదం->prereq_కుడి = sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, p_సమాసం->p_కుడి);
    ఒకవేళ( p_సమాసం->p_ఎడమ==0
     || సమాసం_Has_Property(p_సమాసం, EP_xIsSelect|EP_If_Null_అడ్డువరుస)
     || p_సమాసం->x.pList!=0
    ){
      prereqAll = sqlite3_Where_సమాసం_Usage_NN(p_ముసుగు_సమితి, p_సమాసం);
    }లేకపోతే{
      prereqAll = prereq_ఎడమ | p_పదం->prereq_కుడి;
    }
  }
  ఒకవేళ( p_ముసుగు_సమితి->bVarSelect ) p_పదం->wtFlags |= పదం_VARSELECT;

#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( prereqAll!=sqlite3_Where_సమాసం_Usage_NN(p_ముసుగు_సమితి, p_సమాసం) ){
    రాయి("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_వృక్షం_View_సమాసం(0,p_సమాసం,0);
    ప్రతిజ్ఞా( 0 );
  }
#అయితే_ముగింపు

  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3_Where_Get_ముసుగు(p_ముసుగు_సమితి, p_సమాసం->w.iJoin);
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON) ){
      prereqAll |= x;
      extra_కుడి = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }లేకపోతే ఒకవేళ( (prereqAll>>1)>=x ){
      సమాసం_Clear_Property(p_సమాసం, EP_InnerON);
    }
  }
  p_పదం->prereqAll = prereqAll;
  p_పదం->ఎడమ_Cursor = -1;
  p_పదం->i_మాతృ = -1;
  p_పదం->eOperator = 0;
  ఒకవేళ( allowedOp(op) ){
    పూర్ణాంకం ai_Cur_నిలువు[2];
    సమాసం *p_ఎడమ = sqlite3_సమాసం_Skip_Collate(p_సమాసం->p_ఎడమ);
    సమాసం *p_కుడి = sqlite3_సమాసం_Skip_Collate(p_సమాసం->p_కుడి);
    u16 op_ముసుగు = (p_పదం->prereq_కుడి & prereq_ఎడమ)==0 ? WO_ALL : WO_EQUIV;

    ఒకవేళ( p_పదం->u.x.i_క్షేత్రం>0 ){
      ప్రతిజ్ఞా( op==TK_లోపల );
      ప్రతిజ్ఞా( p_ఎడమ->op==TK_సదిశ );
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_ఎడమ) );
      p_ఎడమ = p_ఎడమ->x.pList->a[p_పదం->u.x.i_క్షేత్రం-1].p_సమాసం;
    }

    ఒకవేళ( సమాసం_Might_Be_సూచీకృత(pSrc, ai_Cur_నిలువు, p_ఎడమ, op) ){
      p_పదం->ఎడమ_Cursor = ai_Cur_నిలువు[0];
      ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 );
      p_పదం->u.x.ఎడమ_స్తంభం = ai_Cur_నిలువు[1];
      p_పదం->eOperator = operator_ముసుగు(op) & op_ముసుగు;
    }
    ఒకవేళ( op==TK_IS ) p_పదం->wtFlags |= పదం_IS;
    ఒకవేళ( p_కుడి
     && సమాసం_Might_Be_సూచీకృత(pSrc, ai_Cur_నిలువు, p_కుడి, op)
     && !సమాసం_Has_Property(p_కుడి, EP_Fixed_నిలువు)
    ){
      Where_పదం *pNew;
      సమాసం *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      ప్రతిజ్ఞా( p_పదం->u.x.i_క్షేత్రం==0 );
      ఒకవేళ( p_పదం->ఎడమ_Cursor>=0 ){
        పూర్ణాంకం idxNew;
        pDup = sqlite3_సమాసం_Dup(db, p_సమాసం, 0);
        ఒకవేళ( db->mallocFailed ){
          sqlite3_సమాసం_Delete(db, pDup);
          తిరిగి;
        }
        idxNew = where_Clause_చొప్పించు(pWC, pDup, పదం_VIRTUAL|పదం_DYNAMIC);
        ఒకవేళ( idxNew==0 ) తిరిగి;
        pNew = &pWC->a[idxNew];
        mark_పదం_As_బిడ్డ(pWC, idxNew, idx_పదం);
        ఒకవేళ( op==TK_IS ) pNew->wtFlags |= పదం_IS;
        p_పదం = &pWC->a[idx_పదం];
        p_పదం->wtFlags |= పదం_COPIED;
        ప్రతిజ్ఞా( pWInfo->pTabList!=0 );
        ఒకవేళ( పదం_Is_Equivalence(pParse, pDup, pWInfo->pTabList) ){
          p_పదం->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }లేకపోతే{
        pDup = p_సమాసం;
        pNew = p_పదం;
      }
      pNew->wtFlags |= సమాసం_Commute(pParse, pDup);
      pNew->ఎడమ_Cursor = ai_Cur_నిలువు[0];
      ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.ఎడమ_స్తంభం = ai_Cur_నిలువు[1];
      testcase( (prereq_ఎడమ | extra_కుడి) != prereq_ఎడమ );
      pNew->prereq_కుడి = prereq_ఎడమ | extra_కుడి;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operator_ముసుగు(pDup->op) + eExtraOp) & op_ముసుగు;
    }లేకపోతే
    ఒకవేళ( op==TK_ISNULL
     && !సమాసం_Has_Property(p_సమాసం,EP_OuterON)
     && 0==sqlite3_సమాసం_Can_Be_Null(p_ఎడమ)
    ){
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      p_సమాసం->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_సమాసం->u.zToken = "false";
      సమాసం_సమితి_Property(p_సమాసం, EP_IsFalse);
      p_పదం->prereqAll = 0;
      p_పదం->eOperator = 0;
    }
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_BETWEEN_OPTIMIZATION
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
  లేకపోతే ఒకవేళ( p_సమాసం->op==TK_BETWEEN && pWC->op==TK_AND ){
    సమాసం_List *pList;
    పూర్ణాంకం i;
    స్థిర స్థిరరాశి u8 ops[] = {TK_GE, TK_LE};
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
    pList = p_సమాసం->x.pList;
    ప్రతిజ్ఞా( pList!=0 );
    ప్రతిజ్ఞా( pList->n_సమాసం==2 );
    ప్రతిజ్ఞా( pWC->a[idx_పదం].n_బిడ్డ==0 );
    కొరకు(i=0; i<2; i++){
      సమాసం *p_New_సమాసం;
      పూర్ణాంకం idxNew;
      p_New_సమాసం = sqlite3PExpr(pParse, ops[i],
                             sqlite3_సమాసం_Dup(db, p_సమాసం->p_ఎడమ, 0),
                             sqlite3_సమాసం_Dup(db, pList->a[i].p_సమాసం, 0));
      transferJoinMarkings(p_New_సమాసం, p_సమాసం);
      idxNew = where_Clause_చొప్పించు(pWC, p_New_సమాసం, పదం_VIRTUAL|పదం_DYNAMIC);
      testcase( idxNew==0 );
      సమాసం_Analyze(pSrc, pWC, idxNew);
      p_పదం = &pWC->a[idx_పదం];
      mark_పదం_As_బిడ్డ(pWC, idxNew, idx_పదం);
    }
  }
#అయితే_ముగింపు /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#ఒకవేళ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  లేకపోతే ఒకవేళ( p_సమాసం->op==TK_OR && !సమాసం_Has_Property(p_సమాసం, EP_Collate) ){
    సమాసం *pAlt = sqlite3_సమాసం_Simplified_And_Or(p_సమాసం);
    ఒకవేళ( pAlt!=p_సమాసం ){
      p_పదం->p_సమాసం = p_సమాసం = sqlite3_సమాసం_Skip_Collate_And_Likely(pAlt);
      వెళ్ళు సమాసం_Analyze_restart;
    }
    ప్రతిజ్ఞా( pWC->op==TK_AND );
    సమాసం_Analyze_Or_పదం(pSrc, pWC, idx_పదం);
    p_పదం = &pWC->a[idx_పదం];
  }
#అయితే_ముగింపు /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  లేకపోతే ఒకవేళ( p_సమాసం->op==TK_NOTNULL ){
    ఒకవేళ( p_సమాసం->p_ఎడమ->op==TK_స్తంభం
     && p_సమాసం->p_ఎడమ->i_స్తంభం>=0
     && !సమాసం_Has_Property(p_సమాసం, EP_OuterON)
    ){
      సమాసం *p_New_సమాసం;
      సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
      పూర్ణాంకం idxNew;
      Where_పదం *p_New_పదం;
 
      p_New_సమాసం = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_సమాసం_Dup(db, p_ఎడమ, 0),
                              sqlite3_సమాసం_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_చొప్పించు(pWC, p_New_సమాసం,
                                పదం_VIRTUAL|పదం_DYNAMIC|పదం_VNULL);
      ఒకవేళ( idxNew ){
        p_New_పదం = &pWC->a[idxNew];
        p_New_పదం->prereq_కుడి = 0;
        p_New_పదం->ఎడమ_Cursor = p_ఎడమ->i_పట్టిక;
        p_New_పదం->u.x.ఎడమ_స్తంభం = p_ఎడమ->i_స్తంభం;
        p_New_పదం->eOperator = WO_GT;
        mark_పదం_As_బిడ్డ(pWC, idxNew, idx_పదం);
        p_పదం = &pWC->a[idx_పదం];
        p_పదం->wtFlags |= పదం_COPIED;
        p_New_పదం->prereqAll = p_పదం->prereqAll;
      }
    }
  }


#నిర్వచితం_కాకపోతే SQLITE_OMIT_LIKE_OPTIMIZATION
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
  లేకపోతే ఒకవేళ( p_సమాసం->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_సమాసం, &pStr1, &isComplete, &noCase)
  ){
    సమాసం *p_ఎడమ;       /* LHS of LIKE/GLOB operator */
    సమాసం *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    సమాసం *pNewExpr1;
    సమాసం *pNewExpr2;
    పూర్ణాంకం idxNew1;
    పూర్ణాంకం idxNew2;
    స్థిరరాశి అక్షరం *zCollSeqName;     /* Name of collating sequence */
    స్థిరరాశి u16 wtFlags = పదం_LIKEOPT | పదం_VIRTUAL | పదం_DYNAMIC;

    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
    p_ఎడమ = p_సమాసం->x.pList->a[1].p_సమాసం;
    pStr2 = sqlite3_సమాసం_Dup(db, pStr1, 0);
    ప్రతిజ్ఞా( pStr1==0 || !సమాసం_Has_Property(pStr1, EP_Int_విలువ) );
    ప్రతిజ్ఞా( pStr2==0 || !సమాసం_Has_Property(pStr2, EP_Int_విలువ) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    ఒకవేళ( noCase && !pParse->db->mallocFailed ){
      పూర్ణాంకం i;
      అక్షరం c;
      p_పదం->wtFlags |= పదం_LIKE;
      కొరకు(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    ఒకవేళ( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      ఒకవేళ( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        ఒకవేళ( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3_పై_గమ్యం_క్రింది[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      అయితే( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      ప్రతిజ్ఞా( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ద్విమాన;
    pNewExpr1 = sqlite3_సమాసం_Dup(db, p_ఎడమ, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_సమాసం_కూడు_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_సమాసం);
    idxNew1 = where_Clause_చొప్పించు(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_సమాసం_Dup(db, p_ఎడమ, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_సమాసం_కూడు_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_సమాసం);
    idxNew2 = where_Clause_చొప్పించు(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    సమాసం_Analyze(pSrc, pWC, idxNew1);
    సమాసం_Analyze(pSrc, pWC, idxNew2);
    p_పదం = &pWC->a[idx_పదం];
    ఒకవేళ( isComplete ){
      mark_పదం_As_బిడ్డ(pWC, idxNew1, idx_పదం);
      mark_పదం_As_బిడ్డ(pWC, idxNew2, idx_పదం);
    }
  }
#అయితే_ముగింపు /* SQLITE_OMIT_LIKE_OPTIMIZATION */

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
  ఒకవేళ( (p_సమాసం->op==TK_EQ || p_సమాసం->op==TK_IS)
   && (n_ఎడమ = sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం->p_ఎడమ))>1
   && sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం->p_కుడి)==n_ఎడమ
   && ( (p_సమాసం->p_ఎడమ->flags & EP_xIsSelect)==0
     || (p_సమాసం->p_కుడి->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<n_ఎడమ; i++){
      పూర్ణాంకం idxNew;
      సమాసం *pNew;
      సమాసం *p_ఎడమ = sqlite3_సమాసం_For_సదిశ_క్షేత్రం(pParse, p_సమాసం->p_ఎడమ, i, n_ఎడమ);
      సమాసం *p_కుడి = sqlite3_సమాసం_For_సదిశ_క్షేత్రం(pParse, p_సమాసం->p_కుడి, i, n_ఎడమ);

      pNew = sqlite3PExpr(pParse, p_సమాసం->op, p_ఎడమ, p_కుడి);
      transferJoinMarkings(pNew, p_సమాసం);
      idxNew = where_Clause_చొప్పించు(pWC, pNew, పదం_DYNAMIC|పదం_ముక్క);
      సమాసం_Analyze(pSrc, pWC, idxNew);
    }
    p_పదం = &pWC->a[idx_పదం];
    p_పదం->wtFlags |= పదం_CODED|పదం_VIRTUAL;  /* Disable the original */
    p_పదం->eOperator = WO_ROWVAL;
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
  లేకపోతే ఒకవేళ( p_సమాసం->op==TK_లోపల
   && p_పదం->u.x.i_క్షేత్రం==0
   && p_సమాసం->p_ఎడమ->op==TK_సదిశ
   && ALWAYS( సమాసం_Use_XSelect(p_సమాసం) )
   && (p_సమాసం->x.pSelect->p_పూర్వ==0 || (p_సమాసం->x.pSelect->selFlags & SF_విలువలు))
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
   && p_సమాసం->x.pSelect->pWin==0
#అయితే_ముగింపు
   && (p_సమాసం->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_సమాసం->x.pSelect->pEList->n_సమాసం <= UMXV(p_పదం->n_బిడ్డ)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    పూర్ణాంకం i;
    ప్రతిజ్ఞా( p_పదం->n_బిడ్డ==0 );
    కొరకు(i=0; i<sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం->p_ఎడమ); i++){
      పూర్ణాంకం idxNew;
      idxNew = where_Clause_చొప్పించు(pWC, p_సమాసం, పదం_VIRTUAL|పదం_ముక్క);
      pWC->a[idxNew].u.x.i_క్షేత్రం = i+1;
      సమాసం_Analyze(pSrc, pWC, idxNew);
      mark_పదం_As_బిడ్డ(pWC, idxNew, idx_పదం);
    }
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  /* Add a WO_AUX auxiliary term to the constraint set if the
  ** current expression is of the form "column OP expr" where OP
  ** is an operator that gets passed into virtual tables but which is
  ** not normally optimized for ordinary tables.  In other words, OP
  ** is one of MATCH, LIKE, GLOB, REGEXP, !=, IS, IS NOT, or NOT NULL.
  ** This information is used by the xBestIndex methods of
  ** virtual tables.  The native query optimizer does not attempt
  ** to do anything with MATCH functions.
  */
  లేకపోతే ఒకవేళ( pWC->op==TK_AND ){
    సమాసం *p_కుడి = 0, *p_ఎడమ = 0;
    పూర్ణాంకం res = isAuxiliaryVtabOperator(db, p_సమాసం, &eOp2, &p_ఎడమ, &p_కుడి);
    అయితే( res-- > 0 ){
      పూర్ణాంకం idxNew;
      Where_పదం *p_New_పదం;
      Bitmask prereq_స్తంభం, prereq_సమాసం;

      prereq_సమాసం = sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, p_కుడి);
      prereq_స్తంభం = sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, p_ఎడమ);
      ఒకవేళ( (prereq_సమాసం & prereq_స్తంభం)==0 ){
        సమాసం *p_New_సమాసం;
        p_New_సమాసం = sqlite3PExpr(pParse, TK_సరిపోలిక,
            0, sqlite3_సమాసం_Dup(db, p_కుడి, 0));
        ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON) && p_New_సమాసం ){
          సమాసం_సమితి_Property(p_New_సమాసం, EP_OuterON);
          p_New_సమాసం->w.iJoin = p_సమాసం->w.iJoin;
        }
        idxNew = where_Clause_చొప్పించు(pWC, p_New_సమాసం, పదం_VIRTUAL|పదం_DYNAMIC);
        testcase( idxNew==0 );
        p_New_పదం = &pWC->a[idxNew];
        p_New_పదం->prereq_కుడి = prereq_సమాసం | extra_కుడి;
        p_New_పదం->ఎడమ_Cursor = p_ఎడమ->i_పట్టిక;
        p_New_పదం->u.x.ఎడమ_స్తంభం = p_ఎడమ->i_స్తంభం;
        p_New_పదం->eOperator = WO_AUX;
        p_New_పదం->e_సరిపోలిక_Op = eOp2;
        mark_పదం_As_బిడ్డ(pWC, idxNew, idx_పదం);
        p_పదం = &pWC->a[idx_పదం];
        p_పదం->wtFlags |= పదం_COPIED;
        p_New_పదం->prereqAll = p_పదం->prereqAll;
      }
      మార్పిడి(సమాసం*, p_ఎడమ, p_కుడి);
    }
  }
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( p_పదం!=&pWC->a[idx_పదం] );
  p_పదం = &pWC->a[idx_పదం];
  p_పదం->prereq_కుడి |= extra_కుడి;
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
శూన్య sqlite3_Where_విభజించు(WhereClause *pWC, సమాసం *p_సమాసం, u8 op){
  సమాసం *pE2 = sqlite3_సమాసం_Skip_Collate_And_Likely(p_సమాసం);
  pWC->op = op;
  ప్రతిజ్ఞా( pE2!=0 || p_సమాసం==0 );
  ఒకవేళ( pE2==0 ) తిరిగి;
  ఒకవేళ( pE2->op!=op ){
    where_Clause_చొప్పించు(pWC, p_సమాసం, 0);
  }లేకపోతే{
    sqlite3_Where_విభజించు(pWC, pE2->p_ఎడమ, op);
    sqlite3_Where_విభజించు(pWC, pE2->p_కుడి, op);
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
స్థిర శూన్య where_కూడు_హద్దు_సమాసం(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  పూర్ణాంకం iReg,           /* Register that will hold value of the limit/offset */
  సమాసం *p_సమాసం,        /* Expression that defines the limit/offset */
  పూర్ణాంకం iCsr,           /* Cursor to which the constraint applies */
  పూర్ణాంకం e_సరిపోలిక_Op        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  సమాసం *pNew;
  పూర్ణాంకం iVal = 0;

  ఒకవేళ( sqlite3_సమాసం_Is_Integer(p_సమాసం, &iVal, pParse, 0) && iVal>=0 ){
    సమాసం *pVal = sqlite3_సమాసం_Int32(db, iVal);
    ఒకవేళ( pVal==0 ) తిరిగి;
    pNew = sqlite3PExpr(pParse, TK_సరిపోలిక, 0, pVal);
  }లేకపోతే{
    సమాసం *pVal = sqlite3_సమాసం_Alloc(db, TK_రిజిస్టరు, 0, 0);
    ఒకవేళ( pVal==0 ) తిరిగి;
    pVal->i_పట్టిక = iReg;
    pNew = sqlite3PExpr(pParse, TK_సరిపోలిక, 0, pVal);
  }
  ఒకవేళ( pNew ){
    Where_పదం *p_పదం;
    పూర్ణాంకం idx;
    idx = where_Clause_చొప్పించు(pWC, pNew, పదం_DYNAMIC|పదం_VIRTUAL);
    p_పదం = &pWC->a[idx];
    p_పదం->ఎడమ_Cursor = iCsr;
    p_పదం->eOperator = WO_AUX;
    p_పదం->e_సరిపోలిక_Op = e_సరిపోలిక_Op;
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
శూన్య SQLITE_NOINLINE sqlite3_Where_కూడు_హద్దు(WhereClause *pWC, Select *p){
  ప్రతిజ్ఞా( p!=0 && p->p_హద్దు!=0 );                 /* 1 -- checked by caller */
  ఒకవేళ( p->p_సమూహం_By==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    సమాసం_List *p_క్రమం_By = p->p_క్రమం_By;
    పూర్ణాంకం iCsr = p->pSrc->a[0].iCursor;
    పూర్ణాంకం ii;

    /* Check condition (4). Return early if it is not met. */
    కొరకు(ii=0; ii<pWC->n_పదం; ii++){
      ఒకవేళ( pWC->a[ii].wtFlags & పదం_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        ప్రతిజ్ఞా( pWC->a[ii].wtFlags & పదం_VIRTUAL );
        ప్రతిజ్ఞా( pWC->a[ii].eOperator==WO_ROWVAL );
        కొనసాగు;
      }
      ఒకవేళ( pWC->a[ii].n_బిడ్డ ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        కొనసాగు;
      }
      ఒకవేళ( pWC->a[ii].ఎడమ_Cursor==iCsr && pWC->a[ii].prereq_కుడి==0 ) కొనసాగు;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      ఒకవేళ( pWC->a[ii].i_మాతృ>=0 ){
        Where_పదం *p_మాతృ = &pWC->a[ pWC->a[ii].i_మాతృ ];
        ఒకవేళ( p_మాతృ->ఎడమ_Cursor==iCsr
         && p_మాతృ->prereq_కుడి==0
         && p_మాతృ->n_బిడ్డ==1
        ){
          కొనసాగు;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      తిరిగి;
    }

    /* Check condition (5). Return early if it is not met. */
    ఒకవేళ( p_క్రమం_By ){
      కొరకు(ii=0; ii<p_క్రమం_By->n_సమాసం; ii++){
        సమాసం *p_సమాసం = p_క్రమం_By->a[ii].p_సమాసం;
        ఒకవేళ( p_సమాసం->op!=TK_స్తంభం ) తిరిగి;
        ఒకవేళ( p_సమాసం->i_పట్టిక!=iCsr ) తిరిగి;
        ఒకవేళ( p_క్రమం_By->a[ii].fg.క్రమీకరించు_Flags & KEYINFO_క్రమం_BIGNULL ) తిరిగి;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    ప్రతిజ్ఞా( p->p_హద్దు->op==TK_హద్దు );
    ఒకవేళ( p->i_స్థానభ్రంశం!=0 && (p->selFlags & SF_Compound)==0 ){
      where_కూడు_హద్దు_సమాసం(pWC, p->i_స్థానభ్రంశం, p->p_హద్దు->p_కుడి,
                        iCsr, SQLITE_సూచీ_CONSTRAINT_స్థానభ్రంశం);
    }
    ఒకవేళ( p->i_స్థానభ్రంశం==0 || (p->selFlags & SF_Compound)==0 ){
      where_కూడు_హద్దు_సమాసం(pWC, p->i_హద్దు, p->p_హద్దు->p_ఎడమ,
                        iCsr, SQLITE_సూచీ_CONSTRAINT_హద్దు);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
శూన్య sqlite3WhereClauseInit(
  WhereClause *pWC,        /* The WhereClause to be initialized */
  WhereInfo *pWInfo        /* The WHERE processing context */
){
  pWC->pWInfo = pWInfo;
  pWC->hasOr = 0;
  pWC->pOuter = 0;
  pWC->n_పదం = 0;
  pWC->n_పునాది = 0;
  pWC->n_స్థానం = Array_కొలత(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
శూన్య sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  ప్రతిజ్ఞా( pWC->n_పదం>=pWC->n_పునాది );
  ఒకవేళ( pWC->n_పదం>0 ){
    Where_పదం *a = pWC->a;
    Where_పదం *a_చివరి = &pWC->a[pWC->n_పదం-1];
#నిర్వచితమైతే SQLITE_DEBUG
    పూర్ణాంకం i;
    /* Verify that every term past pWC->nBase is virtual */
    కొరకు(i=pWC->n_పునాది; i<pWC->n_పదం; i++){
      ప్రతిజ్ఞా( (pWC->a[i].wtFlags & పదం_VIRTUAL)!=0 );
    }
#అయితే_ముగింపు
    అయితే(1){
      ప్రతిజ్ఞా( a->e_సరిపోలిక_Op==0 || a->eOperator==WO_AUX );
      ఒకవేళ( a->wtFlags & పదం_DYNAMIC ){
        sqlite3_సమాసం_Delete(db, a->p_సమాసం);
      }
      ఒకవేళ( a->wtFlags & (పదం_ORINFO|పదం_ANDINFO) ){
        ఒకవేళ( a->wtFlags & పదం_ORINFO ){
          ప్రతిజ్ఞా( (a->wtFlags & పదం_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }లేకపోతే{
          ప్రతిజ్ఞా( (a->wtFlags & పదం_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      ఒకవేళ( a==a_చివరి ) ఆపు;
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
స్థిర SQLITE_NOINLINE Bitmask sqlite3_Where_సమాసం_Usage_నిండిన(
  Where_ముసుగు_సమితి *p_ముసుగు_సమితి,
  సమాసం *p
){
  Bitmask ముసుగు;
  ముసుగు = (p->op==TK_IF_NULL_అడ్డువరుస) ? sqlite3_Where_Get_ముసుగు(p_ముసుగు_సమితి, p->i_పట్టిక) : 0;
  ఒకవేళ( p->p_ఎడమ ) ముసుగు |= sqlite3_Where_సమాసం_Usage_NN(p_ముసుగు_సమితి, p->p_ఎడమ);
  ఒకవేళ( p->p_కుడి ){
    ముసుగు |= sqlite3_Where_సమాసం_Usage_NN(p_ముసుగు_సమితి, p->p_కుడి);
    ప్రతిజ్ఞా( p->x.pList==0 );
  }లేకపోతే ఒకవేళ( సమాసం_Use_XSelect(p) ){
    ఒకవేళ( సమాసం_Has_Property(p, EP_VarSelect) ) p_ముసుగు_సమితి->bVarSelect = 1;
    ముసుగు |= సమాసం_Select_Usage(p_ముసుగు_సమితి, p->x.pSelect);
  }లేకపోతే ఒకవేళ( p->x.pList ){
    ముసుగు |= sqlite3_Where_సమాసం_List_Usage(p_ముసుగు_సమితి, p->x.pList);
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  ఒకవేళ( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && సమాసం_Use_YWin(p) ){
    ప్రతిజ్ఞా( p->y.pWin!=0 );
    ముసుగు |= sqlite3_Where_సమాసం_List_Usage(p_ముసుగు_సమితి, p->y.pWin->p_విభజన);
    ముసుగు |= sqlite3_Where_సమాసం_List_Usage(p_ముసుగు_సమితి, p->y.pWin->p_క్రమం_By);
    ముసుగు |= sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, p->y.pWin->pFilter);
  }
#అయితే_ముగింపు
  తిరిగి ముసుగు;
}
Bitmask sqlite3_Where_సమాసం_Usage_NN(Where_ముసుగు_సమితి *p_ముసుగు_సమితి, సమాసం *p){
  ఒకవేళ( p->op==TK_స్తంభం && !సమాసం_Has_Property(p, EP_Fixed_నిలువు) ){
    తిరిగి sqlite3_Where_Get_ముసుగు(p_ముసుగు_సమితి, p->i_పట్టిక);
  }లేకపోతే ఒకవేళ( సమాసం_Has_Property(p, EP_TokenOnly|EP_ఆకు) ){
    ప్రతిజ్ఞా( p->op!=TK_IF_NULL_అడ్డువరుస );
    తిరిగి 0;
  }
  తిరిగి sqlite3_Where_సమాసం_Usage_నిండిన(p_ముసుగు_సమితి, p);
}
Bitmask sqlite3_Where_సమాసం_Usage(Where_ముసుగు_సమితి *p_ముసుగు_సమితి, సమాసం *p){
  తిరిగి p ? sqlite3_Where_సమాసం_Usage_NN(p_ముసుగు_సమితి,p) : 0;
}
Bitmask sqlite3_Where_సమాసం_List_Usage(Where_ముసుగు_సమితి *p_ముసుగు_సమితి, సమాసం_List *pList){
  పూర్ణాంకం i;
  Bitmask ముసుగు = 0;
  ఒకవేళ( pList ){
    కొరకు(i=0; i<pList->n_సమాసం; i++){
      ముసుగు |= sqlite3_Where_సమాసం_Usage(p_ముసుగు_సమితి, pList->a[i].p_సమాసం);
    }
  }
  తిరిగి ముసుగు;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
శూన్య sqlite3_Where_సమాసం_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  పూర్ణాంకం i;
  కొరకు(i=pWC->n_పదం-1; i>=0; i--){
    సమాసం_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
శూన్య sqlite3_Where_Tab_Func_వాదనలు(
  Parse *pParse,                    /* Parsing context */
  Src_అంశం *p_అంశం,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  పట్టిక *pTab;
  పూర్ణాంకం j, k;
  సమాసం_List *p_వాదనలు;
  సమాసం *p_నిలువు_Ref;
  సమాసం *p_పదం;
  ఒకవేళ( p_అంశం->fg.isTabFunc==0 ) తిరిగి;
  pTab = p_అంశం->pSTab;
  ప్రతిజ్ఞా( pTab!=0 );
  ఒకవేళ( !IsVirtual(pTab) ){
    sqlite3_Error_సందేశం(pParse, "'%s' is not a function", p_అంశం->zName);
    తిరిగి;
  }
  p_వాదనలు = p_అంశం->u1.pFuncArg;
  ఒకవేళ( p_వాదనలు==0 ) తిరిగి;
  కొరకు(j=k=0; j<p_వాదనలు->n_సమాసం; j++){
    సమాసం *pRhs;
    u32 joinType;
    అయితే( k<pTab->n_నిలువు && (pTab->a_నిలువు[k].నిలువు_Flags & COLFLAG_HIDDEN)==0 ){k++;}
    ఒకవేళ( k>=pTab->n_నిలువు ){
      sqlite3_Error_సందేశం(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      తిరిగి;
    }
    p_నిలువు_Ref = sqlite3_సమాసం_Alloc(pParse->db, TK_స్తంభం, 0, 0);
    ఒకవేళ( p_నిలువు_Ref==0 ) తిరిగి;
    p_నిలువు_Ref->i_పట్టిక = p_అంశం->iCursor;
    p_నిలువు_Ref->i_స్తంభం = k++;
    ప్రతిజ్ఞా( సమాసం_Use_YTab(p_నిలువు_Ref) );
    p_నిలువు_Ref->y.pTab = pTab;
    p_అంశం->నిలువు_వాడిన |= sqlite3_సమాసం_నిలువు_వాడిన(p_నిలువు_Ref);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_సమాసం_Dup(pParse->db, p_వాదనలు->a[j].p_సమాసం, 0), 0);
    p_పదం = sqlite3PExpr(pParse, TK_EQ, p_నిలువు_Ref, pRhs);
    ఒకవేళ( p_అంశం->fg.jointype & (JT_ఎడమ|JT_కుడి) ){
      testcase( p_అంశం->fg.jointype & JT_ఎడమ );  /* testtag-20230227a */
      testcase( p_అంశం->fg.jointype & JT_కుడి ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }లేకపోతే{
      testcase( p_అంశం->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_సమితి_Join_సమాసం(p_పదం, p_అంశం->iCursor, joinType);
    where_Clause_చొప్పించు(pWC, p_పదం, పదం_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
పూర్ణాంకం sqlite3WhereLoopBloomable(స్థిరరాశి WhereLoop *pLoop){
  ఒకవేళ( pLoop->wsFlags & WHERE_IPK ) తిరిగి 1;
  ఒకవేళ( NEVER((pLoop->wsFlags & WHERE_సూచీకృత)==0) ) తిరిగి 0;
  తిరిగి sqlite3_సూచీ_Bloomable(pLoop->u.btree.p_సూచీ, pLoop->u.btree.nEq);
}
