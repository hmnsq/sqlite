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
#ضمن "sqliteInt.h"
#ضمن "whereInt.h"

/* Forward declarations */
ساكن فراغ اظہاریہ_Analyze(SrcList*, WhereClause*, صحيح);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
ساكن فراغ whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
ساكن فراغ whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
ساكن صحيح where_Clause_داخل_کرو(WhereClause *pWC, اظہاریہ *p, u16 wtFlags){
  Where_رکن *p_رکن;
  صحيح idx;
  testcase( wtFlags & رکن_VIRTUAL );
  إذا( pWC->n_رکن>=pWC->n_تھاں ){
    Where_رکن *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, حجم(pWC->a[0])*pWC->n_تھاں*2 );
    إذا( pWC->a==0 ){
      إذا( wtFlags & رکن_DYNAMIC ){
        sqlite3_اظہاریہ_Delete(db, p);
      }
      pWC->a = pOld;
      أرجع 0;
    }
    حافظہ_نقل(pWC->a, pOld, حجم(pWC->a[0])*pWC->n_رکن);
    pWC->n_تھاں = pWC->n_تھاں*2;
  }
  p_رکن = &pWC->a[idx = pWC->n_رکن++];
  إذا( (wtFlags & رکن_VIRTUAL)==0 ) pWC->n_ادھار = pWC->n_رکن;
  إذا( p && اظہاریہ_Has_Property(p, EP_Unlikely) ){
    p_رکن->truthProb = sqlite3LogEst(p->i_جدول) - 270;
  }وإلا{
    p_رکن->truthProb = 1;
  }
  p_رکن->p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p);
  p_رکن->wtFlags = wtFlags;
  p_رکن->pWC = pWC;
  p_رکن->i_ماپا = -1;
  memset(&p_رکن->eOperator, 0,
         حجم(Where_رکن) - offsetof(Where_رکن,eOperator));
  أرجع idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
ساكن صحيح allowedOp(صحيح op){
  أكد( TK_GT>TK_EQ && TK_GT<TK_GE );
  أكد( TK_LT>TK_EQ && TK_LT<TK_GE );
  أكد( TK_LE>TK_EQ && TK_LE<TK_GE );
  أكد( TK_GE==TK_EQ+4 );
  أكد( TK_وچ<TK_EQ );
  أكد( TK_IS<TK_EQ );
  أكد( TK_ISNULL<TK_EQ );
  إذا( op>TK_GE ) أرجع 0;
  إذا( op>=TK_EQ ) أرجع 1;
  أرجع op==TK_وچ || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ساكن u16 اظہاریہ_Commute(Parse *pParse, اظہاریہ *p_اظہاریہ){
  إذا( p_اظہاریہ->p_بایاں->op==TK_سمتیہ
   || p_اظہاریہ->p_دایاں->op==TK_سمتیہ
   || sqlite3_ثنائی_Compare_Coll_Seq(pParse, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں) !=
      sqlite3_ثنائی_Compare_Coll_Seq(pParse, p_اظہاریہ->p_دایاں, p_اظہاریہ->p_بایاں)
  ){
    p_اظہاریہ->flags ^= EP_Commuted;
  }
  وٹاندرا(اظہاریہ*,p_اظہاریہ->p_دایاں,p_اظہاریہ->p_بایاں);
  إذا( p_اظہاریہ->op>=TK_GT ){
    أكد( TK_LT==TK_GT+2 );
    أكد( TK_GE==TK_LE+2 );
    أكد( TK_GT>TK_EQ );
    أكد( TK_GT<TK_LE );
    أكد( p_اظہاریہ->op>=TK_GT && p_اظہاریہ->op<=TK_GE );
    p_اظہاریہ->op = ((p_اظہاریہ->op-TK_GT)^2)+TK_GT;
  }
  أرجع 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ساكن u16 operator_نقاب(صحيح op){
  u16 c;
  أكد( allowedOp(op) );
  إذا( op>=TK_EQ ){
    أكد( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }وإلا إذا( op==TK_وچ ){
    c = WO_وچ;
  }وإلا إذا( op==TK_ISNULL ){
    c = WO_ISNULL;
  }وإلا{
    أكد( op==TK_IS );
    c = WO_IS;
  }
  أكد( op!=TK_ISNULL || c==WO_ISNULL );
  أكد( op!=TK_وچ || c==WO_وچ );
  أكد( op!=TK_EQ || c==WO_EQ );
  أكد( op!=TK_LT || c==WO_LT );
  أكد( op!=TK_LE || c==WO_LE );
  أكد( op!=TK_GT || c==WO_GT );
  أكد( op!=TK_GE || c==WO_GE );
  أكد( op!=TK_IS || c==WO_IS );
  أرجع c;
}


#جے_بنا_تعریف SQLITE_OMIT_LIKE_OPTIMIZATION
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
ساكن صحيح isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  اظہاریہ *p_اظہاریہ,      /* Test this expression */
  اظہاریہ **pp_اگیتر,  /* Pointer to TK_STRING expression with pattern prefix */
  صحيح *pisComplete, /* True if the only wildcard is % in the last character */
  صحيح *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ثابت u8 *z = 0;           /* String on RHS of LIKE operator */
  اظہاریہ *p_دایاں, *p_بایاں;      /* Right and left size of LIKE operator */
  اظہاریہ_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  صحيح cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_قدر *pVal = 0;
  صحيح op;                    /* Opcode of pRight */
  صحيح rc;                    /* Result code to return */

  إذا( !sqlite3IsLikeFunction(db, p_اظہاریہ, pnoCase, (حرف*)wc) ){
    أرجع 0;
  }
#جے_تعریف_شدہ SQLITE_EBCDIC
  إذا( *pnoCase ) أرجع 0;
#جے_انت
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  pList = p_اظہاریہ->x.pList;
  p_بایاں = pList->a[1].p_اظہاریہ;

  p_دایاں = sqlite3_اظہاریہ_Skip_Collate(pList->a[0].p_اظہاریہ);
  op = p_دایاں->op;
  إذا( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    صحيح i_خانہ = p_دایاں->i_کالم;
    pVal = sqlite3_Vdbe_Get_Bound_قدر(pReprepare, i_خانہ, SQLITE_AFF_BLOB);
    إذا( pVal && sqlite3_قدر_type(pVal)==SQLITE_متن ){
      z = sqlite3_قدر_متن(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, i_خانہ, 0);
    أكد( p_دایاں->op==TK_VARIABLE || p_دایاں->op==TK_رجسٹر );
  }وإلا إذا( op==TK_STRING ){
    أكد( !اظہاریہ_Has_Property(p_دایاں, EP_Int_قدر) );
     z = (u8*)p_دایاں->u.zToken;
  }
  إذا( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    طالما( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      إذا( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }وإلا إذا( c>=0x80 ){
        ثابت u8 *z2 = z+cnt-1;
        إذا( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          اكسر;
        }وإلا{
          cnt = (صحيح)(z2-z);
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
    إذا( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      اظہاریہ *p_اگیتر;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      p_اگیتر = sqlite3_اظہاریہ(db, TK_STRING, (حرف*)z);
      إذا( p_اگیتر ){
        صحيح iFrom, i_منزل;
        حرف *zNew;
        أكد( !اظہاریہ_Has_Property(p_اگیتر, EP_Int_قدر) );
        zNew = p_اگیتر->u.zToken;
        zNew[cnt] = 0;
        لكل(iFrom=i_منزل=0; iFrom<cnt; iFrom++){
          إذا( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[i_منزل++] = zNew[iFrom];
        }
        zNew[i_منزل] = 0;
        أكد( i_منزل>0 );

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
        إذا( p_بایاں->op!=TK_کالم
         || sqlite3_اظہاریہ_Affinity(p_بایاں)!=SQLITE_AFF_متن
         || (ALWAYS( اظہاریہ_Use_YTab(p_بایاں) )
             && ALWAYS(p_بایاں->y.pTab)
             && IsVirtual(p_بایاں->y.pTab))  /* Might be numeric */
        ){
          صحيح is_عدد;
          مزدوج rDummy;
          أكد( zNew[i_منزل]==0 );
          is_عدد = sqlite3AtoF(zNew, &rDummy);
          إذا( is_عدد<=0 ){
            إذا( i_منزل==1 && zNew[0]=='-' ){
              is_عدد = +1;
            }وإلا{
              zNew[i_منزل-1]++;
              is_عدد = sqlite3AtoF(zNew, &rDummy);
              zNew[i_منزل-1]--;
            }
          }
          إذا( is_عدد>0 ){
            sqlite3_اظہاریہ_Delete(db, p_اگیتر);
            sqlite3_قدر_Free(pVal);
            أرجع 0;
          }
        }
      }
      *pp_اگیتر = p_اگیتر;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      إذا( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_دایاں->i_کالم, 0);
        أكد( !اظہاریہ_Has_Property(p_دایاں, EP_Int_قدر) );
        إذا( *pisComplete && p_دایاں->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          صحيح r1 = sqlite3_Get_عارضی_Reg(pParse);
          sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, p_دایاں, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3_Release_عارضی_Reg(pParse, r1);
        }
      }
    }وإلا{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_قدر_Free(pVal);
  أرجع rc;
}
#جے_انت /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
صحيح sqlite3_اظہاریہ_Is_Like_Operator(ثابت اظہاریہ *p_اظہاریہ){
  ساكن ثابت بنية {
    ثابت حرف *zOp;
    بنانشانی حرف eOp;
  } aOp[] = {
    { "match",  SQLITE_اشاریہ_CONSTRAINT_میل },
    { "glob",   SQLITE_اشاریہ_CONSTRAINT_GLOB },
    { "like",   SQLITE_اشاریہ_CONSTRAINT_LIKE },
    { "regexp", SQLITE_اشاریہ_CONSTRAINT_REGEXP }
  };
  صحيح i;
  أكد( p_اظہاریہ->op==TK_FUNCTION );
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
  لكل(i=0; i<Array_ناپ(aOp); i++){
    إذا( sqlite3StrICmp(p_اظہاریہ->u.zToken, aOp[i].zOp)==0 ){
      أرجع aOp[i].eOp;
    }
  }
  أرجع 0;
}


#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
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
ساكن صحيح isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  اظہاریہ *p_اظہاریہ,                    /* Test this expression */
  بنانشانی حرف *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  اظہاریہ **pp_بایاں,                  /* Column expression to left of MATCH/op2 */
  اظہاریہ **pp_دایاں                  /* Expression to left of MATCH/op2 */
){
  إذا( p_اظہاریہ->op==TK_FUNCTION ){
    اظہاریہ_List *pList;
    اظہاریہ *p_خانہ;                     /* Column reference */
    صحيح i;

    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    pList = p_اظہاریہ->x.pList;
    إذا( pList==0 || pList->n_اظہاریہ!=2 ){
      أرجع 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    p_خانہ = pList->a[1].p_اظہاریہ;
    أكد( p_خانہ->op!=TK_کالم || (اظہاریہ_Use_YTab(p_خانہ) && p_خانہ->y.pTab!=0) );
    إذا( اظہاریہ_Is_Vtab(p_خانہ) && (i = sqlite3_اظہاریہ_Is_Like_Operator(p_اظہاریہ))!=0 ){
      *peOp2 = i;
      *pp_دایاں = pList->a[0].p_اظہاریہ;
      *pp_بایاں = p_خانہ;
      أرجع 1;
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
    p_خانہ = pList->a[0].p_اظہاریہ;
    أكد( p_خانہ->op!=TK_کالم || اظہاریہ_Use_YTab(p_خانہ) );
    أكد( p_خانہ->op!=TK_کالم || (اظہاریہ_Use_YTab(p_خانہ) && p_خانہ->y.pTab!=0) );
    إذا( اظہاریہ_Is_Vtab(p_خانہ) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      فراغ (*x_Not_ورتیا)(sqlite3_context*,صحيح,sqlite3_قدر**);
      فراغ *p_Not_ورتیا;
      pVtab = sqlite3GetVTable(db, p_خانہ->y.pTab)->pVtab;
      أكد( pVtab!=0 );
      أكد( pVtab->pModule!=0 );
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      pMod = (sqlite3_module *)pVtab->pModule;
      إذا( pMod->x_لبھو_Function!=0 ){
        i = pMod->x_لبھو_Function(pVtab,2, p_اظہاریہ->u.zToken, &x_Not_ورتیا, &p_Not_ورتیا);
        إذا( i>=SQLITE_اشاریہ_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_دایاں = pList->a[1].p_اظہاریہ;
          *pp_بایاں = p_خانہ;
          أرجع 1;
        }
      }
    }
  }وإلا إذا( p_اظہاریہ->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    أكد( TK_NE < TK_EQ );
    أكد( TK_ISNOT < TK_EQ );
    أكد( TK_NOTNULL < TK_EQ );
    أرجع 0;
  }وإلا إذا( p_اظہاریہ->op==TK_NE || p_اظہاریہ->op==TK_ISNOT || p_اظہاریہ->op==TK_NOTNULL ){
    صحيح res = 0;
    اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
    اظہاریہ *p_دایاں = p_اظہاریہ->p_دایاں;
    أكد( p_بایاں->op!=TK_کالم || (اظہاریہ_Use_YTab(p_بایاں) && p_بایاں->y.pTab!=0) );
    إذا( اظہاریہ_Is_Vtab(p_بایاں) ){
      res++;
    }
    أكد( p_دایاں==0 || p_دایاں->op!=TK_کالم
            || (اظہاریہ_Use_YTab(p_دایاں) && p_دایاں->y.pTab!=0) );
    إذا( p_دایاں && اظہاریہ_Is_Vtab(p_دایاں) ){
      res++;
      وٹاندرا(اظہاریہ*, p_بایاں, p_دایاں);
    }
    *pp_بایاں = p_بایاں;
    *pp_دایاں = p_دایاں;
    إذا( p_اظہاریہ->op==TK_NE ) *peOp2 = SQLITE_اشاریہ_CONSTRAINT_NE;
    إذا( p_اظہاریہ->op==TK_ISNOT ) *peOp2 = SQLITE_اشاریہ_CONSTRAINT_ISNOT;
    إذا( p_اظہاریہ->op==TK_NOTNULL ) *peOp2 = SQLITE_اشاریہ_CONSTRAINT_ISNOTNULL;
    أرجع res;
  }
  أرجع 0;
}
#جے_انت /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ساكن فراغ transferJoinMarkings(اظہاریہ *pDerived, اظہاریہ *p_ادھار){
  إذا( pDerived && اظہاریہ_Has_Property(p_ادھار, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= p_ادھار->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = p_ادھار->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ساكن فراغ mark_رکن_As_بچہ(WhereClause *pWC, صحيح i_بچہ, صحيح i_ماپا){
  pWC->a[i_بچہ].i_ماپا = i_ماپا;
  pWC->a[i_بچہ].truthProb = pWC->a[i_ماپا].truthProb;
  أكد(   pWC->a[i_ماپا].n_بچہ <  UMXV(pWC->a[0].n_بچہ) );
  pWC->a[i_ماپا].n_بچہ++;
  testcase( pWC->a[i_ماپا].n_بچہ == UMXV(pWC->a[0].n_بچہ) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ساكن Where_رکن *whereNthSubterm(Where_رکن *p_رکن, صحيح N){
  إذا( p_رکن->eOperator!=WO_AND ){
    أرجع N==0 ? p_رکن : 0;
  }
  إذا( N<p_رکن->u.pAndInfo->wc.n_رکن ){
    أرجع &p_رکن->u.pAndInfo->wc.a[N];
  }
  أرجع 0;
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
ساكن فراغ whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  Where_رکن *pOne,       /* First disjunct */
  Where_رکن *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  اظہاریہ *pNew;            /* New virtual expression */
  صحيح op;                /* Operator for the combined expression */
  صحيح idxNew;            /* Index in pWC of the next virtual term */
  اظہاریہ *pA, *pB;         /* Expressions associated with pOne and pTwo */

  إذا( (pOne->wtFlags | pTwo->wtFlags) & رکن_VNULL ) أرجع;
  إذا( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) أرجع;
  إذا( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) أرجع;
  إذا( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) أرجع;
  pA = pOne->p_اظہاریہ;
  pB = pTwo->p_اظہاریہ;
  أكد( pA->p_بایاں!=0 && pA->p_دایاں!=0 );
  أكد( pB->p_بایاں!=0 && pB->p_دایاں!=0 );
  إذا( sqlite3_اظہاریہ_Compare(0,pA->p_بایاں, pB->p_بایاں, -1) )  أرجع;
  إذا( sqlite3_اظہاریہ_Compare(0,pA->p_دایاں, pB->p_دایاں,-1) ) أرجع;
  إذا( اظہاریہ_Has_Property(pA,EP_Commuted)!=اظہاریہ_Has_Property(pB,EP_Commuted) ){
    أرجع;
  }
  /* If we reach this point, it means the two subterms can be combined */
  إذا( (eOp & (eOp-1))!=0 ){
    إذا( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }وإلا{
      أكد( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_اظہاریہ_Dup(db, pA, 0);
  إذا( pNew==0 ) أرجع;
  لكل(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ أكد( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_داخل_کرو(pWC, pNew, رکن_VIRTUAL|رکن_DYNAMIC);
  اظہاریہ_Analyze(pSrc, pWC, idxNew);
}

#إذا !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
ساكن فراغ اظہاریہ_Analyze_Or_رکن(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  صحيح idx_رکن               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  Where_رکن *p_رکن = &pWC->a[idx_رکن];    /* The term to be analyzed */
  اظہاریہ *p_اظہاریہ = p_رکن->p_اظہاریہ;             /* The expression of the term */
  صحيح i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  Where_رکن *p_Or_رکن;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chng_منزل_وچ;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  أكد( (p_رکن->wtFlags & (رکن_DYNAMIC|رکن_ORINFO|رکن_ANDINFO))==0 );
  أكد( p_اظہاریہ->op==TK_OR );
  p_رکن->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, حجم(*pOrInfo));
  إذا( pOrInfo==0 ) أرجع;
  p_رکن->wtFlags |= رکن_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, حجم(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3_Where_ونڈو(pOrWc, p_اظہاریہ, TK_OR);
  sqlite3_Where_اظہاریہ_Analyze(pSrc, pOrWc);
  إذا( db->mallocFailed ) أرجع;
  أكد( pOrWc->n_رکن>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chng_منزل_وچ = ~(Bitmask)0;
  لكل(i=pOrWc->n_رکن-1, p_Or_رکن=pOrWc->a; i>=0 && indexable; i--, p_Or_رکن++){
    إذا( (p_Or_رکن->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      أكد( (p_Or_رکن->wtFlags & (رکن_ANDINFO|رکن_ORINFO))==0 );
      chng_منزل_وچ = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, حجم(*pAndInfo));
      إذا( pAndInfo ){
        WhereClause *pAndWC;
        Where_رکن *p_And_رکن;
        صحيح j;
        Bitmask b = 0;
        p_Or_رکن->u.pAndInfo = pAndInfo;
        p_Or_رکن->wtFlags |= رکن_ANDINFO;
        p_Or_رکن->eOperator = WO_AND;
        p_Or_رکن->بایاں_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, حجم(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3_Where_ونڈو(pAndWC, p_Or_رکن->p_اظہاریہ, TK_AND);
        sqlite3_Where_اظہاریہ_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        إذا( !db->mallocFailed ){
          لكل(j=0, p_And_رکن=pAndWC->a; j<pAndWC->n_رکن; j++, p_And_رکن++){
            أكد( p_And_رکن->p_اظہاریہ );
            إذا( allowedOp(p_And_رکن->p_اظہاریہ->op)
             || p_And_رکن->eOperator==WO_AUX
            ){
              b |= sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, p_And_رکن->بایاں_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }وإلا إذا( p_Or_رکن->wtFlags & رکن_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }وإلا{
      Bitmask b;
      b = sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, p_Or_رکن->بایاں_Cursor);
      إذا( p_Or_رکن->wtFlags & رکن_VIRTUAL ){
        Where_رکن *pOther = &pOrWc->a[p_Or_رکن->i_ماپا];
        b |= sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, pOther->بایاں_Cursor);
      }
      indexable &= b;
      إذا( (p_Or_رکن->eOperator & WO_EQ)==0 ){
        chng_منزل_وچ = 0;
      }وإلا{
        chng_منزل_وچ &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  p_رکن->eOperator = WO_OR;
  p_رکن->بایاں_Cursor = -1;
  إذا( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  إذا( indexable && pOrWc->n_رکن==2 ){
    صحيح iOne = 0;
    Where_رکن *pOne;
    طالما( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      صحيح iTwo = 0;
      Where_رکن *pTwo;
      طالما( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  إذا( chng_منزل_وچ ){
    صحيح ok_منزل_Chng_منزل_وچ = 0;     /* True if the conversion to IN is valid */
    صحيح i_کالم = -1;         /* Column index on lhs of IN operator */
    صحيح iCursor = -1;         /* Table cursor common to all terms */
    صحيح j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    لكل(j=0; j<2 && !ok_منزل_Chng_منزل_وچ; j++){
      اظہاریہ *p_بایاں = 0;
      p_Or_رکن = pOrWc->a;
      لكل(i=pOrWc->n_رکن-1; i>=0; i--, p_Or_رکن++){
        أكد( p_Or_رکن->eOperator & WO_EQ );
        p_Or_رکن->wtFlags &= ~رکن_OK;
        إذا( p_Or_رکن->بایاں_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          أكد( j==1 );
          استمر;
        }
        إذا( (chng_منزل_وچ & sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ,
                                            p_Or_رکن->بایاں_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( p_Or_رکن->wtFlags & رکن_COPIED );
          testcase( p_Or_رکن->wtFlags & رکن_VIRTUAL );
          أكد( p_Or_رکن->wtFlags & (رکن_COPIED|رکن_VIRTUAL) );
          استمر;
        }
        أكد( (p_Or_رکن->eOperator & (WO_OR|WO_AND))==0 );
        i_کالم = p_Or_رکن->u.x.بایاں_کالم;
        iCursor = p_Or_رکن->بایاں_Cursor;
        p_بایاں = p_Or_رکن->p_اظہاریہ->p_بایاں;
        اكسر;
      }
      إذا( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        أكد( j==1 );
        أكد( Is_Power_دا_Two(chng_منزل_وچ) );
        أكد( chng_منزل_وچ==sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, iCursor) );
        اكسر;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      ok_منزل_Chng_منزل_وچ = 1;
      لكل(; i>=0 && ok_منزل_Chng_منزل_وچ; i--, p_Or_رکن++){
        أكد( p_Or_رکن->eOperator & WO_EQ );
        أكد( (p_Or_رکن->eOperator & (WO_OR|WO_AND))==0 );
        إذا( p_Or_رکن->بایاں_Cursor!=iCursor ){
          p_Or_رکن->wtFlags &= ~رکن_OK;
        }وإلا إذا( p_Or_رکن->u.x.بایاں_کالم!=i_کالم || (i_کالم==XN_اظہاریہ
               && sqlite3_اظہاریہ_Compare(pParse, p_Or_رکن->p_اظہاریہ->p_بایاں, p_بایاں, -1)
        )){
          ok_منزل_Chng_منزل_وچ = 0;
        }وإلا{
          صحيح aff_بایاں, aff_دایاں;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_دایاں = sqlite3_اظہاریہ_Affinity(p_Or_رکن->p_اظہاریہ->p_دایاں);
          aff_بایاں = sqlite3_اظہاریہ_Affinity(p_Or_رکن->p_اظہاریہ->p_بایاں);
          إذا( aff_دایاں!=0 && aff_دایاں!=aff_بایاں ){
            ok_منزل_Chng_منزل_وچ = 0;
          }وإلا{
            p_Or_رکن->wtFlags |= رکن_OK;
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
    إذا( ok_منزل_Chng_منزل_وچ ){
      اظہاریہ *pDup;            /* A transient duplicate expression */
      اظہاریہ_List *pList = 0;   /* The RHS of the IN operator */
      اظہاریہ *p_بایاں = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      اظہاریہ *pNew;            /* The complete IN operator */

      لكل(i=pOrWc->n_رکن-1, p_Or_رکن=pOrWc->a; i>=0; i--, p_Or_رکن++){
        اظہاریہ *pThis;
        إذا( (p_Or_رکن->wtFlags & رکن_OK)==0 ) استمر;
        أكد( p_Or_رکن->eOperator & WO_EQ );
        أكد( (p_Or_رکن->eOperator & (WO_OR|WO_AND))==0 );
        أكد( p_Or_رکن->بایاں_Cursor==iCursor );
        أكد( p_Or_رکن->u.x.بایاں_کالم==i_کالم );
        pThis = p_Or_رکن->p_اظہاریہ;
        pDup = sqlite3_اظہاریہ_Dup(db, pThis->p_دایاں, 0);
        pList = sqlite3_اظہاریہ_List_Append(pWInfo->pParse, pList, pDup);
        إذا( p_بایاں==0 ){
          p_بایاں = pThis->p_بایاں;
          pCollSeq = sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pThis);
        }وإلا{
          أكد( 0==sqlite3_اظہاریہ_Compare(pParse, 
                        sqlite3_اظہاریہ_Skip_Collate(pThis->p_بایاں),
                        sqlite3_اظہاریہ_Skip_Collate(p_بایاں), -1) );
          إذا( pCollSeq!=sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pThis) ){
            p_بایاں = 0;  /* Collating sequence mismatch */
            اكسر;
          }
        }
      }
      إذا( p_بایاں==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }وإلا{
        pDup = sqlite3_اظہاریہ_Dup(db, p_بایاں, 0);
        إذا( sqlite3_اظہاریہ_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          أكد( pCollSeq->zName!=0 );
          pDup = sqlite3_اظہاریہ_جوڑو_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_وچ, pDup, 0);
      }
      إذا( pNew ){
        صحيح idxNew;
        transferJoinMarkings(pNew, p_اظہاریہ);
        أكد( اظہاریہ_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_داخل_کرو(pWC, pNew, رکن_VIRTUAL|رکن_DYNAMIC);
        testcase( idxNew==0 );
        اظہاریہ_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
      }وإلا{
        sqlite3_اظہاریہ_List_Delete(db, pList);
      }
    }
  }
}
#جے_انت /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

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
ساكن صحيح رکن_Is_Equivalence(Parse *pParse, اظہاریہ *p_اظہاریہ, SrcList *pSrc){
  حرف aff1, aff2;
  إذا( !OptimizationEnabled(pParse->db, SQLITE_متعدی) ) أرجع 0;  /* (1) */
  إذا( p_اظہاریہ->op!=TK_EQ && p_اظہاریہ->op!=TK_IS ) أرجع 0;                 /* (2) */
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON|EP_Collate) ) أرجع 0;        /* (3) */
  أكد( pSrc!=0 );
  إذا( p_اظہاریہ->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    أرجع 0;                                                          /* (4) */
  }
  aff1 = sqlite3_اظہاریہ_Affinity(p_اظہاریہ->p_بایاں);
  aff2 = sqlite3_اظہاریہ_Affinity(p_اظہاریہ->p_دایاں);
  إذا( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    أرجع 0;                                                          /* (5) */
  }
  إذا( !sqlite3_اظہاریہ_Coll_Seq_میل(pParse, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں) ){
    أرجع 0;                                                          /* (6) */
  }
  أرجع 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ساكن Bitmask اظہاریہ_Select_Usage(Where_نقاب_سیٹ *p_نقاب_سیٹ, Select *pS){
  Bitmask نقاب = 0;
  طالما( pS ){
    SrcList *pSrc = pS->pSrc;
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, pS->pEList);
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, pS->p_گروہ_By);
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, pS->p_ترتیب_By);
    نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, pS->pWhere);
    نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, pS->pHaving);
    إذا( ALWAYS(pSrc!=0) ){
      صحيح i;
      لكل(i=0; i<pSrc->nSrc; i++){
        إذا( pSrc->a[i].fg.isSubquery ){
          نقاب |= اظہاریہ_Select_Usage(p_نقاب_سیٹ, pSrc->a[i].u4.pSubq->pSelect);
        }
        إذا( pSrc->a[i].fg.isUsing==0 ){
          نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, pSrc->a[i].u3.pOn);
        }
        إذا( pSrc->a[i].fg.isTabFunc ){
          نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->p_پہلوں;
  }
  أرجع نقاب;
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
ساكن SQLITE_NOINLINE صحيح اظہاریہ_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *ai_Cur_خانہ,         /* Write the referenced table cursor and column here */
  اظہاریہ *p_اظہاریہ,           /* An operand of a comparison operator */
  صحيح j                  /* Start looking with the j-th pFrom entry */
){
  اشاریہ *pIdx;
  صحيح i;
  صحيح iCur;
  افعل{
    iCur = pFrom->a[j].iCursor;
    لكل(pIdx=pFrom->a[j].pSTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
      إذا( pIdx->a_خانہ_اظہاریہ==0 ) استمر;
      لكل(i=0; i<pIdx->n_کلید_خانہ; i++){
        إذا( pIdx->ai_کالم[i]!=XN_اظہاریہ ) استمر;
        أكد( pIdx->b_Has_اظہاریہ );
        إذا( sqlite3_اظہاریہ_Compare_Skip(p_اظہاریہ,pIdx->a_خانہ_اظہاریہ->a[i].p_اظہاریہ,iCur)==0
         && !sqlite3_اظہاریہ_Is_Constant(0,pIdx->a_خانہ_اظہاریہ->a[i].p_اظہاریہ)
        ){
          ai_Cur_خانہ[0] = iCur;
          ai_Cur_خانہ[1] = XN_اظہاریہ;
          أرجع 1;
        }
      }
    }
  }طالما( ++j < pFrom->nSrc );
  أرجع 0;
}
ساكن صحيح اظہاریہ_Might_Be_اشاریہ_دار(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *ai_Cur_خانہ,         /* Write the referenced table cursor & column here */
  اظہاریہ *p_اظہاریہ,           /* An operand of a comparison operator */
  صحيح op                 /* The specific comparison operator */
){
  صحيح i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  أكد( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  أكد( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_وچ<TK_GE );
  أكد( op<=TK_GE );
  إذا( p_اظہاریہ->op==TK_سمتیہ && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    p_اظہاریہ = p_اظہاریہ->x.pList->a[0].p_اظہاریہ;
  }

  إذا( p_اظہاریہ->op==TK_کالم ){
    ai_Cur_خانہ[0] = p_اظہاریہ->i_جدول;
    ai_Cur_خانہ[1] = p_اظہاریہ->i_کالم;
    أرجع 1;
  }

  لكل(i=0; i<pFrom->nSrc; i++){
    اشاریہ *pIdx;
    لكل(pIdx=pFrom->a[i].pSTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
      إذا( pIdx->a_خانہ_اظہاریہ ){
        أرجع اظہاریہ_Might_Be_Indexed2(pFrom,ai_Cur_خانہ,p_اظہاریہ,i);
      }
    }
  }
  أرجع 0;
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
ساكن فراغ اظہاریہ_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  صحيح idx_رکن               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  Where_رکن *p_رکن;                /* The term to be analyzed */
  Where_نقاب_سیٹ *p_نقاب_سیٹ;          /* Set of table index masks */
  اظہاریہ *p_اظہاریہ;                     /* The expression to be analyzed */
  Bitmask prereq_بایاں;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_دایاں = 0;          /* Extra dependencies on LEFT JOIN */
  اظہاریہ *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  صحيح isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  صحيح noCase = 0;                  /* uppercase equivalent to lowercase */
  صحيح op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  بنانشانی حرف eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  صحيح n_بایاں;                       /* Number of elements on left side vector */

  إذا( db->mallocFailed ){
    أرجع;
  }
  أكد( pWC->n_رکن > idx_رکن );
  p_رکن = &pWC->a[idx_رکن];
#جے_تعریف_شدہ SQLITE_DEBUG
  p_رکن->i_رکن = idx_رکن;
#جے_انت
  p_نقاب_سیٹ = &pWInfo->s_نقاب_سیٹ;
  p_اظہاریہ = p_رکن->p_اظہاریہ;
  أكد( p_اظہاریہ!=0 ); /* Because malloc() has not failed */
  أكد( p_اظہاریہ->op!=TK_AS && p_اظہاریہ->op!=TK_COLLATE );
اظہاریہ_Analyze_restart:
  p_نقاب_سیٹ->bVarSelect = 0;
  prereq_بایاں = sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p_اظہاریہ->p_بایاں);
  op = p_اظہاریہ->op;
  إذا( op==TK_وچ ){
    أكد( p_اظہاریہ->p_دایاں==0 );
    إذا( sqlite3_اظہاریہ_Check_وچ(pParse, p_اظہاریہ) ) أرجع;
    إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
      p_رکن->prereq_دایاں = اظہاریہ_Select_Usage(p_نقاب_سیٹ, p_اظہاریہ->x.pSelect);
    }وإلا{
      p_رکن->prereq_دایاں = sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, p_اظہاریہ->x.pList);
    }
    prereqAll = prereq_بایاں | p_رکن->prereq_دایاں;
  }وإلا{
    p_رکن->prereq_دایاں = sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p_اظہاریہ->p_دایاں);
    إذا( p_اظہاریہ->p_بایاں==0
     || اظہاریہ_Has_Property(p_اظہاریہ, EP_xIsSelect|EP_If_Null_صف)
     || p_اظہاریہ->x.pList!=0
    ){
      prereqAll = sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ, p_اظہاریہ);
    }وإلا{
      prereqAll = prereq_بایاں | p_رکن->prereq_دایاں;
    }
  }
  إذا( p_نقاب_سیٹ->bVarSelect ) p_رکن->wtFlags |= رکن_VARSELECT;

#جے_تعریف_شدہ SQLITE_DEBUG
  إذا( prereqAll!=sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ, p_اظہاریہ) ){
    لکھو("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_درخت_View_اظہاریہ(0,p_اظہاریہ,0);
    أكد( 0 );
  }
#جے_انت

  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3_Where_Get_نقاب(p_نقاب_سیٹ, p_اظہاریہ->w.iJoin);
    إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ){
      prereqAll |= x;
      extra_دایاں = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }وإلا إذا( (prereqAll>>1)>=x ){
      اظہاریہ_Clear_Property(p_اظہاریہ, EP_InnerON);
    }
  }
  p_رکن->prereqAll = prereqAll;
  p_رکن->بایاں_Cursor = -1;
  p_رکن->i_ماپا = -1;
  p_رکن->eOperator = 0;
  إذا( allowedOp(op) ){
    صحيح ai_Cur_خانہ[2];
    اظہاریہ *p_بایاں = sqlite3_اظہاریہ_Skip_Collate(p_اظہاریہ->p_بایاں);
    اظہاریہ *p_دایاں = sqlite3_اظہاریہ_Skip_Collate(p_اظہاریہ->p_دایاں);
    u16 op_نقاب = (p_رکن->prereq_دایاں & prereq_بایاں)==0 ? WO_ALL : WO_EQUIV;

    إذا( p_رکن->u.x.i_میدان>0 ){
      أكد( op==TK_وچ );
      أكد( p_بایاں->op==TK_سمتیہ );
      أكد( اظہاریہ_Use_XList(p_بایاں) );
      p_بایاں = p_بایاں->x.pList->a[p_رکن->u.x.i_میدان-1].p_اظہاریہ;
    }

    إذا( اظہاریہ_Might_Be_اشاریہ_دار(pSrc, ai_Cur_خانہ, p_بایاں, op) ){
      p_رکن->بایاں_Cursor = ai_Cur_خانہ[0];
      أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
      p_رکن->u.x.بایاں_کالم = ai_Cur_خانہ[1];
      p_رکن->eOperator = operator_نقاب(op) & op_نقاب;
    }
    إذا( op==TK_IS ) p_رکن->wtFlags |= رکن_IS;
    إذا( p_دایاں
     && اظہاریہ_Might_Be_اشاریہ_دار(pSrc, ai_Cur_خانہ, p_دایاں, op)
     && !اظہاریہ_Has_Property(p_دایاں, EP_Fixed_خانہ)
    ){
      Where_رکن *pNew;
      اظہاریہ *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      أكد( p_رکن->u.x.i_میدان==0 );
      إذا( p_رکن->بایاں_Cursor>=0 ){
        صحيح idxNew;
        pDup = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
        إذا( db->mallocFailed ){
          sqlite3_اظہاریہ_Delete(db, pDup);
          أرجع;
        }
        idxNew = where_Clause_داخل_کرو(pWC, pDup, رکن_VIRTUAL|رکن_DYNAMIC);
        إذا( idxNew==0 ) أرجع;
        pNew = &pWC->a[idxNew];
        mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
        إذا( op==TK_IS ) pNew->wtFlags |= رکن_IS;
        p_رکن = &pWC->a[idx_رکن];
        p_رکن->wtFlags |= رکن_COPIED;
        أكد( pWInfo->pTabList!=0 );
        إذا( رکن_Is_Equivalence(pParse, pDup, pWInfo->pTabList) ){
          p_رکن->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }وإلا{
        pDup = p_اظہاریہ;
        pNew = p_رکن;
      }
      pNew->wtFlags |= اظہاریہ_Commute(pParse, pDup);
      pNew->بایاں_Cursor = ai_Cur_خانہ[0];
      أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.بایاں_کالم = ai_Cur_خانہ[1];
      testcase( (prereq_بایاں | extra_دایاں) != prereq_بایاں );
      pNew->prereq_دایاں = prereq_بایاں | extra_دایاں;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operator_نقاب(pDup->op) + eExtraOp) & op_نقاب;
    }وإلا
    إذا( op==TK_ISNULL
     && !اظہاریہ_Has_Property(p_اظہاریہ,EP_OuterON)
     && 0==sqlite3_اظہاریہ_Can_Be_Null(p_بایاں)
    ){
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      p_اظہاریہ->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_اظہاریہ->u.zToken = "false";
      اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_IsFalse);
      p_رکن->prereqAll = 0;
      p_رکن->eOperator = 0;
    }
  }

#جے_بنا_تعریف SQLITE_OMIT_BETWEEN_OPTIMIZATION
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
  وإلا إذا( p_اظہاریہ->op==TK_BETWEEN && pWC->op==TK_AND ){
    اظہاریہ_List *pList;
    صحيح i;
    ساكن ثابت u8 ops[] = {TK_GE, TK_LE};
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    pList = p_اظہاریہ->x.pList;
    أكد( pList!=0 );
    أكد( pList->n_اظہاریہ==2 );
    أكد( pWC->a[idx_رکن].n_بچہ==0 );
    لكل(i=0; i<2; i++){
      اظہاریہ *p_New_اظہاریہ;
      صحيح idxNew;
      p_New_اظہاریہ = sqlite3PExpr(pParse, ops[i],
                             sqlite3_اظہاریہ_Dup(db, p_اظہاریہ->p_بایاں, 0),
                             sqlite3_اظہاریہ_Dup(db, pList->a[i].p_اظہاریہ, 0));
      transferJoinMarkings(p_New_اظہاریہ, p_اظہاریہ);
      idxNew = where_Clause_داخل_کرو(pWC, p_New_اظہاریہ, رکن_VIRTUAL|رکن_DYNAMIC);
      testcase( idxNew==0 );
      اظہاریہ_Analyze(pSrc, pWC, idxNew);
      p_رکن = &pWC->a[idx_رکن];
      mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
    }
  }
#جے_انت /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#إذا !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  وإلا إذا( p_اظہاریہ->op==TK_OR && !اظہاریہ_Has_Property(p_اظہاریہ, EP_Collate) ){
    اظہاریہ *pAlt = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ);
    إذا( pAlt!=p_اظہاریہ ){
      p_رکن->p_اظہاریہ = p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pAlt);
      اذهب اظہاریہ_Analyze_restart;
    }
    أكد( pWC->op==TK_AND );
    اظہاریہ_Analyze_Or_رکن(pSrc, pWC, idx_رکن);
    p_رکن = &pWC->a[idx_رکن];
  }
#جے_انت /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  وإلا إذا( p_اظہاریہ->op==TK_NOTNULL ){
    إذا( p_اظہاریہ->p_بایاں->op==TK_کالم
     && p_اظہاریہ->p_بایاں->i_کالم>=0
     && !اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON)
    ){
      اظہاریہ *p_New_اظہاریہ;
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      صحيح idxNew;
      Where_رکن *p_New_رکن;
 
      p_New_اظہاریہ = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_اظہاریہ_Dup(db, p_بایاں, 0),
                              sqlite3_اظہاریہ_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_داخل_کرو(pWC, p_New_اظہاریہ,
                                رکن_VIRTUAL|رکن_DYNAMIC|رکن_VNULL);
      إذا( idxNew ){
        p_New_رکن = &pWC->a[idxNew];
        p_New_رکن->prereq_دایاں = 0;
        p_New_رکن->بایاں_Cursor = p_بایاں->i_جدول;
        p_New_رکن->u.x.بایاں_کالم = p_بایاں->i_کالم;
        p_New_رکن->eOperator = WO_GT;
        mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
        p_رکن = &pWC->a[idx_رکن];
        p_رکن->wtFlags |= رکن_COPIED;
        p_New_رکن->prereqAll = p_رکن->prereqAll;
      }
    }
  }


#جے_بنا_تعریف SQLITE_OMIT_LIKE_OPTIMIZATION
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
  وإلا إذا( p_اظہاریہ->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_اظہاریہ, &pStr1, &isComplete, &noCase)
  ){
    اظہاریہ *p_بایاں;       /* LHS of LIKE/GLOB operator */
    اظہاریہ *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    اظہاریہ *pNewExpr1;
    اظہاریہ *pNewExpr2;
    صحيح idxNew1;
    صحيح idxNew2;
    ثابت حرف *zCollSeqName;     /* Name of collating sequence */
    ثابت u16 wtFlags = رکن_LIKEOPT | رکن_VIRTUAL | رکن_DYNAMIC;

    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    p_بایاں = p_اظہاریہ->x.pList->a[1].p_اظہاریہ;
    pStr2 = sqlite3_اظہاریہ_Dup(db, pStr1, 0);
    أكد( pStr1==0 || !اظہاریہ_Has_Property(pStr1, EP_Int_قدر) );
    أكد( pStr2==0 || !اظہاریہ_Has_Property(pStr2, EP_Int_قدر) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    إذا( noCase && !pParse->db->mallocFailed ){
      صحيح i;
      حرف c;
      p_رکن->wtFlags |= رکن_LIKE;
      لكل(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    إذا( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      إذا( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        إذا( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3_اتلا_منزل_ہیٹھلا[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      طالما( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      أكد( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ثنائی;
    pNewExpr1 = sqlite3_اظہاریہ_Dup(db, p_بایاں, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_اظہاریہ_جوڑو_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_اظہاریہ);
    idxNew1 = where_Clause_داخل_کرو(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_اظہاریہ_Dup(db, p_بایاں, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_اظہاریہ_جوڑو_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_اظہاریہ);
    idxNew2 = where_Clause_داخل_کرو(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    اظہاریہ_Analyze(pSrc, pWC, idxNew1);
    اظہاریہ_Analyze(pSrc, pWC, idxNew2);
    p_رکن = &pWC->a[idx_رکن];
    إذا( isComplete ){
      mark_رکن_As_بچہ(pWC, idxNew1, idx_رکن);
      mark_رکن_As_بچہ(pWC, idxNew2, idx_رکن);
    }
  }
#جے_انت /* SQLITE_OMIT_LIKE_OPTIMIZATION */

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
  إذا( (p_اظہاریہ->op==TK_EQ || p_اظہاریہ->op==TK_IS)
   && (n_بایاں = sqlite3_اظہاریہ_سمتیہ_ناپ(p_اظہاریہ->p_بایاں))>1
   && sqlite3_اظہاریہ_سمتیہ_ناپ(p_اظہاریہ->p_دایاں)==n_بایاں
   && ( (p_اظہاریہ->p_بایاں->flags & EP_xIsSelect)==0
     || (p_اظہاریہ->p_دایاں->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    صحيح i;
    لكل(i=0; i<n_بایاں; i++){
      صحيح idxNew;
      اظہاریہ *pNew;
      اظہاریہ *p_بایاں = sqlite3_اظہاریہ_For_سمتیہ_میدان(pParse, p_اظہاریہ->p_بایاں, i, n_بایاں);
      اظہاریہ *p_دایاں = sqlite3_اظہاریہ_For_سمتیہ_میدان(pParse, p_اظہاریہ->p_دایاں, i, n_بایاں);

      pNew = sqlite3PExpr(pParse, p_اظہاریہ->op, p_بایاں, p_دایاں);
      transferJoinMarkings(pNew, p_اظہاریہ);
      idxNew = where_Clause_داخل_کرو(pWC, pNew, رکن_DYNAMIC|رکن_ٹکڑا);
      اظہاریہ_Analyze(pSrc, pWC, idxNew);
    }
    p_رکن = &pWC->a[idx_رکن];
    p_رکن->wtFlags |= رکن_CODED|رکن_VIRTUAL;  /* Disable the original */
    p_رکن->eOperator = WO_ROWVAL;
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
  وإلا إذا( p_اظہاریہ->op==TK_وچ
   && p_رکن->u.x.i_میدان==0
   && p_اظہاریہ->p_بایاں->op==TK_سمتیہ
   && ALWAYS( اظہاریہ_Use_XSelect(p_اظہاریہ) )
   && (p_اظہاریہ->x.pSelect->p_پہلوں==0 || (p_اظہاریہ->x.pSelect->selFlags & SF_اقدار))
#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
   && p_اظہاریہ->x.pSelect->pWin==0
#جے_انت
   && (p_اظہاریہ->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_اظہاریہ->x.pSelect->pEList->n_اظہاریہ <= UMXV(p_رکن->n_بچہ)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    صحيح i;
    أكد( p_رکن->n_بچہ==0 );
    لكل(i=0; i<sqlite3_اظہاریہ_سمتیہ_ناپ(p_اظہاریہ->p_بایاں); i++){
      صحيح idxNew;
      idxNew = where_Clause_داخل_کرو(pWC, p_اظہاریہ, رکن_VIRTUAL|رکن_ٹکڑا);
      pWC->a[idxNew].u.x.i_میدان = i+1;
      اظہاریہ_Analyze(pSrc, pWC, idxNew);
      mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
    }
  }

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  /* Add a WO_AUX auxiliary term to the constraint set if the
  ** current expression is of the form "column OP expr" where OP
  ** is an operator that gets passed into virtual tables but which is
  ** not normally optimized for ordinary tables.  In other words, OP
  ** is one of MATCH, LIKE, GLOB, REGEXP, !=, IS, IS NOT, or NOT NULL.
  ** This information is used by the xBestIndex methods of
  ** virtual tables.  The native query optimizer does not attempt
  ** to do anything with MATCH functions.
  */
  وإلا إذا( pWC->op==TK_AND ){
    اظہاریہ *p_دایاں = 0, *p_بایاں = 0;
    صحيح res = isAuxiliaryVtabOperator(db, p_اظہاریہ, &eOp2, &p_بایاں, &p_دایاں);
    طالما( res-- > 0 ){
      صحيح idxNew;
      Where_رکن *p_New_رکن;
      Bitmask prereq_کالم, prereq_اظہاریہ;

      prereq_اظہاریہ = sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p_دایاں);
      prereq_کالم = sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p_بایاں);
      إذا( (prereq_اظہاریہ & prereq_کالم)==0 ){
        اظہاریہ *p_New_اظہاریہ;
        p_New_اظہاریہ = sqlite3PExpr(pParse, TK_میل,
            0, sqlite3_اظہاریہ_Dup(db, p_دایاں, 0));
        إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) && p_New_اظہاریہ ){
          اظہاریہ_سیٹ_Property(p_New_اظہاریہ, EP_OuterON);
          p_New_اظہاریہ->w.iJoin = p_اظہاریہ->w.iJoin;
        }
        idxNew = where_Clause_داخل_کرو(pWC, p_New_اظہاریہ, رکن_VIRTUAL|رکن_DYNAMIC);
        testcase( idxNew==0 );
        p_New_رکن = &pWC->a[idxNew];
        p_New_رکن->prereq_دایاں = prereq_اظہاریہ | extra_دایاں;
        p_New_رکن->بایاں_Cursor = p_بایاں->i_جدول;
        p_New_رکن->u.x.بایاں_کالم = p_بایاں->i_کالم;
        p_New_رکن->eOperator = WO_AUX;
        p_New_رکن->e_میل_Op = eOp2;
        mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
        p_رکن = &pWC->a[idx_رکن];
        p_رکن->wtFlags |= رکن_COPIED;
        p_New_رکن->prereqAll = p_رکن->prereqAll;
      }
      وٹاندرا(اظہاریہ*, p_بایاں, p_دایاں);
    }
  }
#جے_انت /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( p_رکن!=&pWC->a[idx_رکن] );
  p_رکن = &pWC->a[idx_رکن];
  p_رکن->prereq_دایاں |= extra_دایاں;
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
فراغ sqlite3_Where_ونڈو(WhereClause *pWC, اظہاریہ *p_اظہاریہ, u8 op){
  اظہاریہ *pE2 = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ);
  pWC->op = op;
  أكد( pE2!=0 || p_اظہاریہ==0 );
  إذا( pE2==0 ) أرجع;
  إذا( pE2->op!=op ){
    where_Clause_داخل_کرو(pWC, p_اظہاریہ, 0);
  }وإلا{
    sqlite3_Where_ونڈو(pWC, pE2->p_بایاں, op);
    sqlite3_Where_ونڈو(pWC, pE2->p_دایاں, op);
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
ساكن فراغ where_جوڑو_حد_بندی_اظہاریہ(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  صحيح iReg,           /* Register that will hold value of the limit/offset */
  اظہاریہ *p_اظہاریہ,        /* Expression that defines the limit/offset */
  صحيح iCsr,           /* Cursor to which the constraint applies */
  صحيح e_میل_Op        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  اظہاریہ *pNew;
  صحيح iVal = 0;

  إذا( sqlite3_اظہاریہ_Is_Integer(p_اظہاریہ, &iVal, pParse, 0) && iVal>=0 ){
    اظہاریہ *pVal = sqlite3_اظہاریہ_Int32(db, iVal);
    إذا( pVal==0 ) أرجع;
    pNew = sqlite3PExpr(pParse, TK_میل, 0, pVal);
  }وإلا{
    اظہاریہ *pVal = sqlite3_اظہاریہ_Alloc(db, TK_رجسٹر, 0, 0);
    إذا( pVal==0 ) أرجع;
    pVal->i_جدول = iReg;
    pNew = sqlite3PExpr(pParse, TK_میل, 0, pVal);
  }
  إذا( pNew ){
    Where_رکن *p_رکن;
    صحيح idx;
    idx = where_Clause_داخل_کرو(pWC, pNew, رکن_DYNAMIC|رکن_VIRTUAL);
    p_رکن = &pWC->a[idx];
    p_رکن->بایاں_Cursor = iCsr;
    p_رکن->eOperator = WO_AUX;
    p_رکن->e_میل_Op = e_میل_Op;
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
فراغ SQLITE_NOINLINE sqlite3_Where_جوڑو_حد_بندی(WhereClause *pWC, Select *p){
  أكد( p!=0 && p->p_حد_بندی!=0 );                 /* 1 -- checked by caller */
  إذا( p->p_گروہ_By==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    اظہاریہ_List *p_ترتیب_By = p->p_ترتیب_By;
    صحيح iCsr = p->pSrc->a[0].iCursor;
    صحيح ii;

    /* Check condition (4). Return early if it is not met. */
    لكل(ii=0; ii<pWC->n_رکن; ii++){
      إذا( pWC->a[ii].wtFlags & رکن_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        أكد( pWC->a[ii].wtFlags & رکن_VIRTUAL );
        أكد( pWC->a[ii].eOperator==WO_ROWVAL );
        استمر;
      }
      إذا( pWC->a[ii].n_بچہ ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        استمر;
      }
      إذا( pWC->a[ii].بایاں_Cursor==iCsr && pWC->a[ii].prereq_دایاں==0 ) استمر;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      إذا( pWC->a[ii].i_ماپا>=0 ){
        Where_رکن *p_ماپا = &pWC->a[ pWC->a[ii].i_ماپا ];
        إذا( p_ماپا->بایاں_Cursor==iCsr
         && p_ماپا->prereq_دایاں==0
         && p_ماپا->n_بچہ==1
        ){
          استمر;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      أرجع;
    }

    /* Check condition (5). Return early if it is not met. */
    إذا( p_ترتیب_By ){
      لكل(ii=0; ii<p_ترتیب_By->n_اظہاریہ; ii++){
        اظہاریہ *p_اظہاریہ = p_ترتیب_By->a[ii].p_اظہاریہ;
        إذا( p_اظہاریہ->op!=TK_کالم ) أرجع;
        إذا( p_اظہاریہ->i_جدول!=iCsr ) أرجع;
        إذا( p_ترتیب_By->a[ii].fg.ترتیب_دیو_Flags & KEYINFO_ترتیب_BIGNULL ) أرجع;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    أكد( p->p_حد_بندی->op==TK_حد_بندی );
    إذا( p->i_وتھ!=0 && (p->selFlags & SF_Compound)==0 ){
      where_جوڑو_حد_بندی_اظہاریہ(pWC, p->i_وتھ, p->p_حد_بندی->p_دایاں,
                        iCsr, SQLITE_اشاریہ_CONSTRAINT_وتھ);
    }
    إذا( p->i_وتھ==0 || (p->selFlags & SF_Compound)==0 ){
      where_جوڑو_حد_بندی_اظہاریہ(pWC, p->i_حد_بندی, p->p_حد_بندی->p_بایاں,
                        iCsr, SQLITE_اشاریہ_CONSTRAINT_حد_بندی);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
فراغ sqlite3WhereClauseInit(
  WhereClause *pWC,        /* The WhereClause to be initialized */
  WhereInfo *pWInfo        /* The WHERE processing context */
){
  pWC->pWInfo = pWInfo;
  pWC->hasOr = 0;
  pWC->pOuter = 0;
  pWC->n_رکن = 0;
  pWC->n_ادھار = 0;
  pWC->n_تھاں = Array_ناپ(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
فراغ sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  أكد( pWC->n_رکن>=pWC->n_ادھار );
  إذا( pWC->n_رکن>0 ){
    Where_رکن *a = pWC->a;
    Where_رکن *a_آخری = &pWC->a[pWC->n_رکن-1];
#جے_تعریف_شدہ SQLITE_DEBUG
    صحيح i;
    /* Verify that every term past pWC->nBase is virtual */
    لكل(i=pWC->n_ادھار; i<pWC->n_رکن; i++){
      أكد( (pWC->a[i].wtFlags & رکن_VIRTUAL)!=0 );
    }
#جے_انت
    طالما(1){
      أكد( a->e_میل_Op==0 || a->eOperator==WO_AUX );
      إذا( a->wtFlags & رکن_DYNAMIC ){
        sqlite3_اظہاریہ_Delete(db, a->p_اظہاریہ);
      }
      إذا( a->wtFlags & (رکن_ORINFO|رکن_ANDINFO) ){
        إذا( a->wtFlags & رکن_ORINFO ){
          أكد( (a->wtFlags & رکن_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }وإلا{
          أكد( (a->wtFlags & رکن_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      إذا( a==a_آخری ) اكسر;
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
ساكن SQLITE_NOINLINE Bitmask sqlite3_Where_اظہاریہ_Usage_بھریا(
  Where_نقاب_سیٹ *p_نقاب_سیٹ,
  اظہاریہ *p
){
  Bitmask نقاب;
  نقاب = (p->op==TK_IF_NULL_صف) ? sqlite3_Where_Get_نقاب(p_نقاب_سیٹ, p->i_جدول) : 0;
  إذا( p->p_بایاں ) نقاب |= sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ, p->p_بایاں);
  إذا( p->p_دایاں ){
    نقاب |= sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ, p->p_دایاں);
    أكد( p->x.pList==0 );
  }وإلا إذا( اظہاریہ_Use_XSelect(p) ){
    إذا( اظہاریہ_Has_Property(p, EP_VarSelect) ) p_نقاب_سیٹ->bVarSelect = 1;
    نقاب |= اظہاریہ_Select_Usage(p_نقاب_سیٹ, p->x.pSelect);
  }وإلا إذا( p->x.pList ){
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, p->x.pList);
  }
#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
  إذا( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && اظہاریہ_Use_YWin(p) ){
    أكد( p->y.pWin!=0 );
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, p->y.pWin->p_حصے_کرو);
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, p->y.pWin->p_ترتیب_By);
    نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p->y.pWin->pFilter);
  }
#جے_انت
  أرجع نقاب;
}
Bitmask sqlite3_Where_اظہاریہ_Usage_NN(Where_نقاب_سیٹ *p_نقاب_سیٹ, اظہاریہ *p){
  إذا( p->op==TK_کالم && !اظہاریہ_Has_Property(p, EP_Fixed_خانہ) ){
    أرجع sqlite3_Where_Get_نقاب(p_نقاب_سیٹ, p->i_جدول);
  }وإلا إذا( اظہاریہ_Has_Property(p, EP_TokenOnly|EP_پتی) ){
    أكد( p->op!=TK_IF_NULL_صف );
    أرجع 0;
  }
  أرجع sqlite3_Where_اظہاریہ_Usage_بھریا(p_نقاب_سیٹ, p);
}
Bitmask sqlite3_Where_اظہاریہ_Usage(Where_نقاب_سیٹ *p_نقاب_سیٹ, اظہاریہ *p){
  أرجع p ? sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ,p) : 0;
}
Bitmask sqlite3_Where_اظہاریہ_List_Usage(Where_نقاب_سیٹ *p_نقاب_سیٹ, اظہاریہ_List *pList){
  صحيح i;
  Bitmask نقاب = 0;
  إذا( pList ){
    لكل(i=0; i<pList->n_اظہاریہ; i++){
      نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, pList->a[i].p_اظہاریہ);
    }
  }
  أرجع نقاب;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
فراغ sqlite3_Where_اظہاریہ_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  صحيح i;
  لكل(i=pWC->n_رکن-1; i>=0; i--){
    اظہاریہ_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
فراغ sqlite3_Where_Tab_Func_دلیلاں(
  Parse *pParse,                    /* Parsing context */
  Src_شے *p_شے,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  جدول *pTab;
  صحيح j, k;
  اظہاریہ_List *p_دلیلاں;
  اظہاریہ *p_خانہ_Ref;
  اظہاریہ *p_رکن;
  إذا( p_شے->fg.isTabFunc==0 ) أرجع;
  pTab = p_شے->pSTab;
  أكد( pTab!=0 );
  إذا( !IsVirtual(pTab) ){
    sqlite3_Error_پیغام(pParse, "'%s' is not a function", p_شے->zName);
    أرجع;
  }
  p_دلیلاں = p_شے->u1.pFuncArg;
  إذا( p_دلیلاں==0 ) أرجع;
  لكل(j=k=0; j<p_دلیلاں->n_اظہاریہ; j++){
    اظہاریہ *pRhs;
    u32 joinType;
    طالما( k<pTab->n_خانہ && (pTab->a_خانہ[k].خانہ_Flags & COLFLAG_HIDDEN)==0 ){k++;}
    إذا( k>=pTab->n_خانہ ){
      sqlite3_Error_پیغام(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      أرجع;
    }
    p_خانہ_Ref = sqlite3_اظہاریہ_Alloc(pParse->db, TK_کالم, 0, 0);
    إذا( p_خانہ_Ref==0 ) أرجع;
    p_خانہ_Ref->i_جدول = p_شے->iCursor;
    p_خانہ_Ref->i_کالم = k++;
    أكد( اظہاریہ_Use_YTab(p_خانہ_Ref) );
    p_خانہ_Ref->y.pTab = pTab;
    p_شے->خانہ_ورتیا |= sqlite3_اظہاریہ_خانہ_ورتیا(p_خانہ_Ref);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_اظہاریہ_Dup(pParse->db, p_دلیلاں->a[j].p_اظہاریہ, 0), 0);
    p_رکن = sqlite3PExpr(pParse, TK_EQ, p_خانہ_Ref, pRhs);
    إذا( p_شے->fg.jointype & (JT_بایاں|JT_دایاں) ){
      testcase( p_شے->fg.jointype & JT_بایاں );  /* testtag-20230227a */
      testcase( p_شے->fg.jointype & JT_دایاں ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }وإلا{
      testcase( p_شے->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_سیٹ_Join_اظہاریہ(p_رکن, p_شے->iCursor, joinType);
    where_Clause_داخل_کرو(pWC, p_رکن, رکن_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
صحيح sqlite3WhereLoopBloomable(ثابت WhereLoop *pLoop){
  إذا( pLoop->wsFlags & WHERE_IPK ) أرجع 1;
  إذا( NEVER((pLoop->wsFlags & WHERE_اشاریہ_دار)==0) ) أرجع 0;
  أرجع sqlite3_اشاریہ_Bloomable(pLoop->u.btree.p_اشاریہ, pLoop->u.btree.nEq);
}
