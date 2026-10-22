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
ساكن فراغ عبارت_Analyze(SrcList*, WhereClause*, صحيح);

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
ساكن صحيح where_Clause_درج(WhereClause *pWC, عبارت *p, u16 wtFlags){
  Where_جمله *p_جمله;
  صحيح idx;
  testcase( wtFlags & جمله_VIRTUAL );
  إذا( pWC->n_جمله>=pWC->n_جایگاه ){
    Where_جمله *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, حجم(pWC->a[0])*pWC->n_جایگاه*2 );
    إذا( pWC->a==0 ){
      إذا( wtFlags & جمله_DYNAMIC ){
        sqlite3_عبارت_Delete(db, p);
      }
      pWC->a = pOld;
      أرجع 0;
    }
    کپی_حافظه(pWC->a, pOld, حجم(pWC->a[0])*pWC->n_جمله);
    pWC->n_جایگاه = pWC->n_جایگاه*2;
  }
  p_جمله = &pWC->a[idx = pWC->n_جمله++];
  إذا( (wtFlags & جمله_VIRTUAL)==0 ) pWC->n_پایه = pWC->n_جمله;
  إذا( p && عبارت_Has_Property(p, EP_Unlikely) ){
    p_جمله->truthProb = sqlite3LogEst(p->i_جدول) - 270;
  }وإلا{
    p_جمله->truthProb = 1;
  }
  p_جمله->p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely(p);
  p_جمله->wtFlags = wtFlags;
  p_جمله->pWC = pWC;
  p_جمله->i_والد = -1;
  memset(&p_جمله->eOperator, 0,
         حجم(Where_جمله) - offsetof(Where_جمله,eOperator));
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
  أكد( TK_درون<TK_EQ );
  أكد( TK_IS<TK_EQ );
  أكد( TK_ISNULL<TK_EQ );
  إذا( op>TK_GE ) أرجع 0;
  إذا( op>=TK_EQ ) أرجع 1;
  أرجع op==TK_درون || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ساكن u16 عبارت_Commute(Parse *pParse, عبارت *p_عبارت){
  إذا( p_عبارت->p_چپ->op==TK_وکتور
   || p_عبارت->p_راست->op==TK_وکتور
   || sqlite3_دودویی_Compare_Coll_Seq(pParse, p_عبارت->p_چپ, p_عبارت->p_راست) !=
      sqlite3_دودویی_Compare_Coll_Seq(pParse, p_عبارت->p_راست, p_عبارت->p_چپ)
  ){
    p_عبارت->flags ^= EP_Commuted;
  }
  جابجا(عبارت*,p_عبارت->p_راست,p_عبارت->p_چپ);
  إذا( p_عبارت->op>=TK_GT ){
    أكد( TK_LT==TK_GT+2 );
    أكد( TK_GE==TK_LE+2 );
    أكد( TK_GT>TK_EQ );
    أكد( TK_GT<TK_LE );
    أكد( p_عبارت->op>=TK_GT && p_عبارت->op<=TK_GE );
    p_عبارت->op = ((p_عبارت->op-TK_GT)^2)+TK_GT;
  }
  أرجع 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ساكن u16 operator_ماسک(صحيح op){
  u16 c;
  أكد( allowedOp(op) );
  إذا( op>=TK_EQ ){
    أكد( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }وإلا إذا( op==TK_درون ){
    c = WO_درون;
  }وإلا إذا( op==TK_ISNULL ){
    c = WO_ISNULL;
  }وإلا{
    أكد( op==TK_IS );
    c = WO_IS;
  }
  أكد( op!=TK_ISNULL || c==WO_ISNULL );
  أكد( op!=TK_درون || c==WO_درون );
  أكد( op!=TK_EQ || c==WO_EQ );
  أكد( op!=TK_LT || c==WO_LT );
  أكد( op!=TK_LE || c==WO_LE );
  أكد( op!=TK_GT || c==WO_GT );
  أكد( op!=TK_GE || c==WO_GE );
  أكد( op!=TK_IS || c==WO_IS );
  أرجع c;
}


#اگر_تعریف_نشده SQLITE_OMIT_LIKE_OPTIMIZATION
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
  عبارت *p_عبارت,      /* Test this expression */
  عبارت **pp_پیشوند,  /* Pointer to TK_STRING expression with pattern prefix */
  صحيح *pisComplete, /* True if the only wildcard is % in the last character */
  صحيح *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ثابت u8 *z = 0;           /* String on RHS of LIKE operator */
  عبارت *p_راست, *p_چپ;      /* Right and left size of LIKE operator */
  عبارت_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  صحيح cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_مقدار *pVal = 0;
  صحيح op;                    /* Opcode of pRight */
  صحيح rc;                    /* Result code to return */

  إذا( !sqlite3IsLikeFunction(db, p_عبارت, pnoCase, (حرف*)wc) ){
    أرجع 0;
  }
#اگر_تعریف_شده SQLITE_EBCDIC
  إذا( *pnoCase ) أرجع 0;
#پایان_اگر
  أكد( عبارت_Use_XList(p_عبارت) );
  pList = p_عبارت->x.pList;
  p_چپ = pList->a[1].p_عبارت;

  p_راست = sqlite3_عبارت_Skip_Collate(pList->a[0].p_عبارت);
  op = p_راست->op;
  إذا( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    صحيح i_ستون = p_راست->i_ستونی;
    pVal = sqlite3_Vdbe_Get_Bound_مقدار(pReprepare, i_ستون, SQLITE_AFF_BLOB);
    إذا( pVal && sqlite3_مقدار_type(pVal)==SQLITE_متن ){
      z = sqlite3_مقدار_متن(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, i_ستون, 0);
    أكد( p_راست->op==TK_VARIABLE || p_راست->op==TK_ثبات );
  }وإلا إذا( op==TK_STRING ){
    أكد( !عبارت_Has_Property(p_راست, EP_Int_مقدار) );
     z = (u8*)p_راست->u.zToken;
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
      عبارت *p_پیشوند;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      p_پیشوند = sqlite3_عبارت(db, TK_STRING, (حرف*)z);
      إذا( p_پیشوند ){
        صحيح iFrom, i_مقصد;
        حرف *zNew;
        أكد( !عبارت_Has_Property(p_پیشوند, EP_Int_مقدار) );
        zNew = p_پیشوند->u.zToken;
        zNew[cnt] = 0;
        لكل(iFrom=i_مقصد=0; iFrom<cnt; iFrom++){
          إذا( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[i_مقصد++] = zNew[iFrom];
        }
        zNew[i_مقصد] = 0;
        أكد( i_مقصد>0 );

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
        إذا( p_چپ->op!=TK_ستونی
         || sqlite3_عبارت_Affinity(p_چپ)!=SQLITE_AFF_متن
         || (ALWAYS( عبارت_Use_YTab(p_چپ) )
             && ALWAYS(p_چپ->y.pTab)
             && IsVirtual(p_چپ->y.pTab))  /* Might be numeric */
        ){
          صحيح is_رقم;
          مزدوج rDummy;
          أكد( zNew[i_مقصد]==0 );
          is_رقم = sqlite3AtoF(zNew, &rDummy);
          إذا( is_رقم<=0 ){
            إذا( i_مقصد==1 && zNew[0]=='-' ){
              is_رقم = +1;
            }وإلا{
              zNew[i_مقصد-1]++;
              is_رقم = sqlite3AtoF(zNew, &rDummy);
              zNew[i_مقصد-1]--;
            }
          }
          إذا( is_رقم>0 ){
            sqlite3_عبارت_Delete(db, p_پیشوند);
            sqlite3_مقدار_Free(pVal);
            أرجع 0;
          }
        }
      }
      *pp_پیشوند = p_پیشوند;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      إذا( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_راست->i_ستونی, 0);
        أكد( !عبارت_Has_Property(p_راست, EP_Int_مقدار) );
        إذا( *pisComplete && p_راست->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          صحيح r1 = sqlite3_Get_موقت_Reg(pParse);
          sqlite3_عبارت_کد_هدف(pParse, p_راست, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3_Release_موقت_Reg(pParse, r1);
        }
      }
    }وإلا{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_مقدار_Free(pVal);
  أرجع rc;
}
#پایان_اگر /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
صحيح sqlite3_عبارت_Is_Like_Operator(ثابت عبارت *p_عبارت){
  ساكن ثابت بنية {
    ثابت حرف *zOp;
    بیعلامت حرف eOp;
  } aOp[] = {
    { "match",  SQLITE_شاخص_CONSTRAINT_تطابق },
    { "glob",   SQLITE_شاخص_CONSTRAINT_GLOB },
    { "like",   SQLITE_شاخص_CONSTRAINT_LIKE },
    { "regexp", SQLITE_شاخص_CONSTRAINT_REGEXP }
  };
  صحيح i;
  أكد( p_عبارت->op==TK_FUNCTION );
  أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
  لكل(i=0; i<Array_اندازه(aOp); i++){
    إذا( sqlite3StrICmp(p_عبارت->u.zToken, aOp[i].zOp)==0 ){
      أرجع aOp[i].eOp;
    }
  }
  أرجع 0;
}


#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
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
  عبارت *p_عبارت,                    /* Test this expression */
  بیعلامت حرف *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  عبارت **pp_چپ,                  /* Column expression to left of MATCH/op2 */
  عبارت **pp_راست                  /* Expression to left of MATCH/op2 */
){
  إذا( p_عبارت->op==TK_FUNCTION ){
    عبارت_List *pList;
    عبارت *p_ستون;                     /* Column reference */
    صحيح i;

    أكد( عبارت_Use_XList(p_عبارت) );
    pList = p_عبارت->x.pList;
    إذا( pList==0 || pList->n_عبارت!=2 ){
      أرجع 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    p_ستون = pList->a[1].p_عبارت;
    أكد( p_ستون->op!=TK_ستونی || (عبارت_Use_YTab(p_ستون) && p_ستون->y.pTab!=0) );
    إذا( عبارت_Is_Vtab(p_ستون) && (i = sqlite3_عبارت_Is_Like_Operator(p_عبارت))!=0 ){
      *peOp2 = i;
      *pp_راست = pList->a[0].p_عبارت;
      *pp_چپ = p_ستون;
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
    p_ستون = pList->a[0].p_عبارت;
    أكد( p_ستون->op!=TK_ستونی || عبارت_Use_YTab(p_ستون) );
    أكد( p_ستون->op!=TK_ستونی || (عبارت_Use_YTab(p_ستون) && p_ستون->y.pTab!=0) );
    إذا( عبارت_Is_Vtab(p_ستون) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      فراغ (*x_Not_استفاده_شده)(sqlite3_context*,صحيح,sqlite3_مقدار**);
      فراغ *p_Not_استفاده_شده;
      pVtab = sqlite3GetVTable(db, p_ستون->y.pTab)->pVtab;
      أكد( pVtab!=0 );
      أكد( pVtab->pModule!=0 );
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      pMod = (sqlite3_module *)pVtab->pModule;
      إذا( pMod->x_پیدا_کن_Function!=0 ){
        i = pMod->x_پیدا_کن_Function(pVtab,2, p_عبارت->u.zToken, &x_Not_استفاده_شده, &p_Not_استفاده_شده);
        إذا( i>=SQLITE_شاخص_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_راست = pList->a[1].p_عبارت;
          *pp_چپ = p_ستون;
          أرجع 1;
        }
      }
    }
  }وإلا إذا( p_عبارت->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    أكد( TK_NE < TK_EQ );
    أكد( TK_ISNOT < TK_EQ );
    أكد( TK_NOTNULL < TK_EQ );
    أرجع 0;
  }وإلا إذا( p_عبارت->op==TK_NE || p_عبارت->op==TK_ISNOT || p_عبارت->op==TK_NOTNULL ){
    صحيح res = 0;
    عبارت *p_چپ = p_عبارت->p_چپ;
    عبارت *p_راست = p_عبارت->p_راست;
    أكد( p_چپ->op!=TK_ستونی || (عبارت_Use_YTab(p_چپ) && p_چپ->y.pTab!=0) );
    إذا( عبارت_Is_Vtab(p_چپ) ){
      res++;
    }
    أكد( p_راست==0 || p_راست->op!=TK_ستونی
            || (عبارت_Use_YTab(p_راست) && p_راست->y.pTab!=0) );
    إذا( p_راست && عبارت_Is_Vtab(p_راست) ){
      res++;
      جابجا(عبارت*, p_چپ, p_راست);
    }
    *pp_چپ = p_چپ;
    *pp_راست = p_راست;
    إذا( p_عبارت->op==TK_NE ) *peOp2 = SQLITE_شاخص_CONSTRAINT_NE;
    إذا( p_عبارت->op==TK_ISNOT ) *peOp2 = SQLITE_شاخص_CONSTRAINT_ISNOT;
    إذا( p_عبارت->op==TK_NOTNULL ) *peOp2 = SQLITE_شاخص_CONSTRAINT_ISNOTNULL;
    أرجع res;
  }
  أرجع 0;
}
#پایان_اگر /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ساكن فراغ transferJoinMarkings(عبارت *pDerived, عبارت *p_پایه){
  إذا( pDerived && عبارت_Has_Property(p_پایه, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= p_پایه->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = p_پایه->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ساكن فراغ mark_جمله_As_فرزند(WhereClause *pWC, صحيح i_فرزند, صحيح i_والد){
  pWC->a[i_فرزند].i_والد = i_والد;
  pWC->a[i_فرزند].truthProb = pWC->a[i_والد].truthProb;
  أكد(   pWC->a[i_والد].n_فرزند <  UMXV(pWC->a[0].n_فرزند) );
  pWC->a[i_والد].n_فرزند++;
  testcase( pWC->a[i_والد].n_فرزند == UMXV(pWC->a[0].n_فرزند) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ساكن Where_جمله *whereNthSubterm(Where_جمله *p_جمله, صحيح N){
  إذا( p_جمله->eOperator!=WO_AND ){
    أرجع N==0 ? p_جمله : 0;
  }
  إذا( N<p_جمله->u.pAndInfo->wc.n_جمله ){
    أرجع &p_جمله->u.pAndInfo->wc.a[N];
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
  Where_جمله *pOne,       /* First disjunct */
  Where_جمله *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  عبارت *pNew;            /* New virtual expression */
  صحيح op;                /* Operator for the combined expression */
  صحيح idxNew;            /* Index in pWC of the next virtual term */
  عبارت *pA, *pB;         /* Expressions associated with pOne and pTwo */

  إذا( (pOne->wtFlags | pTwo->wtFlags) & جمله_VNULL ) أرجع;
  إذا( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) أرجع;
  إذا( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) أرجع;
  إذا( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) أرجع;
  pA = pOne->p_عبارت;
  pB = pTwo->p_عبارت;
  أكد( pA->p_چپ!=0 && pA->p_راست!=0 );
  أكد( pB->p_چپ!=0 && pB->p_راست!=0 );
  إذا( sqlite3_عبارت_Compare(0,pA->p_چپ, pB->p_چپ, -1) )  أرجع;
  إذا( sqlite3_عبارت_Compare(0,pA->p_راست, pB->p_راست,-1) ) أرجع;
  إذا( عبارت_Has_Property(pA,EP_Commuted)!=عبارت_Has_Property(pB,EP_Commuted) ){
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
  pNew = sqlite3_عبارت_Dup(db, pA, 0);
  إذا( pNew==0 ) أرجع;
  لكل(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ أكد( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_درج(pWC, pNew, جمله_VIRTUAL|جمله_DYNAMIC);
  عبارت_Analyze(pSrc, pWC, idxNew);
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
ساكن فراغ عبارت_Analyze_Or_جمله(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  صحيح idx_جمله               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  Where_جمله *p_جمله = &pWC->a[idx_جمله];    /* The term to be analyzed */
  عبارت *p_عبارت = p_جمله->p_عبارت;             /* The expression of the term */
  صحيح i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  Where_جمله *p_Or_جمله;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chng_مقصد_درون;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  أكد( (p_جمله->wtFlags & (جمله_DYNAMIC|جمله_ORINFO|جمله_ANDINFO))==0 );
  أكد( p_عبارت->op==TK_OR );
  p_جمله->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, حجم(*pOrInfo));
  إذا( pOrInfo==0 ) أرجع;
  p_جمله->wtFlags |= جمله_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, حجم(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3_Where_تقسیم(pOrWc, p_عبارت, TK_OR);
  sqlite3_Where_عبارت_Analyze(pSrc, pOrWc);
  إذا( db->mallocFailed ) أرجع;
  أكد( pOrWc->n_جمله>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chng_مقصد_درون = ~(Bitmask)0;
  لكل(i=pOrWc->n_جمله-1, p_Or_جمله=pOrWc->a; i>=0 && indexable; i--, p_Or_جمله++){
    إذا( (p_Or_جمله->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      أكد( (p_Or_جمله->wtFlags & (جمله_ANDINFO|جمله_ORINFO))==0 );
      chng_مقصد_درون = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, حجم(*pAndInfo));
      إذا( pAndInfo ){
        WhereClause *pAndWC;
        Where_جمله *p_And_جمله;
        صحيح j;
        Bitmask b = 0;
        p_Or_جمله->u.pAndInfo = pAndInfo;
        p_Or_جمله->wtFlags |= جمله_ANDINFO;
        p_Or_جمله->eOperator = WO_AND;
        p_Or_جمله->چپ_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, حجم(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3_Where_تقسیم(pAndWC, p_Or_جمله->p_عبارت, TK_AND);
        sqlite3_Where_عبارت_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        إذا( !db->mallocFailed ){
          لكل(j=0, p_And_جمله=pAndWC->a; j<pAndWC->n_جمله; j++, p_And_جمله++){
            أكد( p_And_جمله->p_عبارت );
            إذا( allowedOp(p_And_جمله->p_عبارت->op)
             || p_And_جمله->eOperator==WO_AUX
            ){
              b |= sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه, p_And_جمله->چپ_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }وإلا إذا( p_Or_جمله->wtFlags & جمله_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }وإلا{
      Bitmask b;
      b = sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه, p_Or_جمله->چپ_Cursor);
      إذا( p_Or_جمله->wtFlags & جمله_VIRTUAL ){
        Where_جمله *pOther = &pOrWc->a[p_Or_جمله->i_والد];
        b |= sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه, pOther->چپ_Cursor);
      }
      indexable &= b;
      إذا( (p_Or_جمله->eOperator & WO_EQ)==0 ){
        chng_مقصد_درون = 0;
      }وإلا{
        chng_مقصد_درون &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  p_جمله->eOperator = WO_OR;
  p_جمله->چپ_Cursor = -1;
  إذا( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  إذا( indexable && pOrWc->n_جمله==2 ){
    صحيح iOne = 0;
    Where_جمله *pOne;
    طالما( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      صحيح iTwo = 0;
      Where_جمله *pTwo;
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
  إذا( chng_مقصد_درون ){
    صحيح ok_مقصد_Chng_مقصد_درون = 0;     /* True if the conversion to IN is valid */
    صحيح i_ستونی = -1;         /* Column index on lhs of IN operator */
    صحيح iCursor = -1;         /* Table cursor common to all terms */
    صحيح j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    لكل(j=0; j<2 && !ok_مقصد_Chng_مقصد_درون; j++){
      عبارت *p_چپ = 0;
      p_Or_جمله = pOrWc->a;
      لكل(i=pOrWc->n_جمله-1; i>=0; i--, p_Or_جمله++){
        أكد( p_Or_جمله->eOperator & WO_EQ );
        p_Or_جمله->wtFlags &= ~جمله_OK;
        إذا( p_Or_جمله->چپ_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          أكد( j==1 );
          استمر;
        }
        إذا( (chng_مقصد_درون & sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه,
                                            p_Or_جمله->چپ_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( p_Or_جمله->wtFlags & جمله_COPIED );
          testcase( p_Or_جمله->wtFlags & جمله_VIRTUAL );
          أكد( p_Or_جمله->wtFlags & (جمله_COPIED|جمله_VIRTUAL) );
          استمر;
        }
        أكد( (p_Or_جمله->eOperator & (WO_OR|WO_AND))==0 );
        i_ستونی = p_Or_جمله->u.x.چپ_ستونی;
        iCursor = p_Or_جمله->چپ_Cursor;
        p_چپ = p_Or_جمله->p_عبارت->p_چپ;
        اكسر;
      }
      إذا( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        أكد( j==1 );
        أكد( Is_Power_از_Two(chng_مقصد_درون) );
        أكد( chng_مقصد_درون==sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه, iCursor) );
        اكسر;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      ok_مقصد_Chng_مقصد_درون = 1;
      لكل(; i>=0 && ok_مقصد_Chng_مقصد_درون; i--, p_Or_جمله++){
        أكد( p_Or_جمله->eOperator & WO_EQ );
        أكد( (p_Or_جمله->eOperator & (WO_OR|WO_AND))==0 );
        إذا( p_Or_جمله->چپ_Cursor!=iCursor ){
          p_Or_جمله->wtFlags &= ~جمله_OK;
        }وإلا إذا( p_Or_جمله->u.x.چپ_ستونی!=i_ستونی || (i_ستونی==XN_عبارت
               && sqlite3_عبارت_Compare(pParse, p_Or_جمله->p_عبارت->p_چپ, p_چپ, -1)
        )){
          ok_مقصد_Chng_مقصد_درون = 0;
        }وإلا{
          صحيح aff_چپ, aff_راست;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_راست = sqlite3_عبارت_Affinity(p_Or_جمله->p_عبارت->p_راست);
          aff_چپ = sqlite3_عبارت_Affinity(p_Or_جمله->p_عبارت->p_چپ);
          إذا( aff_راست!=0 && aff_راست!=aff_چپ ){
            ok_مقصد_Chng_مقصد_درون = 0;
          }وإلا{
            p_Or_جمله->wtFlags |= جمله_OK;
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
    إذا( ok_مقصد_Chng_مقصد_درون ){
      عبارت *pDup;            /* A transient duplicate expression */
      عبارت_List *pList = 0;   /* The RHS of the IN operator */
      عبارت *p_چپ = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      عبارت *pNew;            /* The complete IN operator */

      لكل(i=pOrWc->n_جمله-1, p_Or_جمله=pOrWc->a; i>=0; i--, p_Or_جمله++){
        عبارت *pThis;
        إذا( (p_Or_جمله->wtFlags & جمله_OK)==0 ) استمر;
        أكد( p_Or_جمله->eOperator & WO_EQ );
        أكد( (p_Or_جمله->eOperator & (WO_OR|WO_AND))==0 );
        أكد( p_Or_جمله->چپ_Cursor==iCursor );
        أكد( p_Or_جمله->u.x.چپ_ستونی==i_ستونی );
        pThis = p_Or_جمله->p_عبارت;
        pDup = sqlite3_عبارت_Dup(db, pThis->p_راست, 0);
        pList = sqlite3_عبارت_List_Append(pWInfo->pParse, pList, pDup);
        إذا( p_چپ==0 ){
          p_چپ = pThis->p_چپ;
          pCollSeq = sqlite3_عبارت_Compare_Coll_Seq(pParse, pThis);
        }وإلا{
          أكد( 0==sqlite3_عبارت_Compare(pParse, 
                        sqlite3_عبارت_Skip_Collate(pThis->p_چپ),
                        sqlite3_عبارت_Skip_Collate(p_چپ), -1) );
          إذا( pCollSeq!=sqlite3_عبارت_Compare_Coll_Seq(pParse, pThis) ){
            p_چپ = 0;  /* Collating sequence mismatch */
            اكسر;
          }
        }
      }
      إذا( p_چپ==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }وإلا{
        pDup = sqlite3_عبارت_Dup(db, p_چپ, 0);
        إذا( sqlite3_عبارت_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          أكد( pCollSeq->zName!=0 );
          pDup = sqlite3_عبارت_جمع_کن_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_درون, pDup, 0);
      }
      إذا( pNew ){
        صحيح idxNew;
        transferJoinMarkings(pNew, p_عبارت);
        أكد( عبارت_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_درج(pWC, pNew, جمله_VIRTUAL|جمله_DYNAMIC);
        testcase( idxNew==0 );
        عبارت_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        mark_جمله_As_فرزند(pWC, idxNew, idx_جمله);
      }وإلا{
        sqlite3_عبارت_List_Delete(db, pList);
      }
    }
  }
}
#پایان_اگر /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

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
ساكن صحيح جمله_Is_Equivalence(Parse *pParse, عبارت *p_عبارت, SrcList *pSrc){
  حرف aff1, aff2;
  إذا( !OptimizationEnabled(pParse->db, SQLITE_متعدی) ) أرجع 0;  /* (1) */
  إذا( p_عبارت->op!=TK_EQ && p_عبارت->op!=TK_IS ) أرجع 0;                 /* (2) */
  إذا( عبارت_Has_Property(p_عبارت, EP_OuterON|EP_Collate) ) أرجع 0;        /* (3) */
  أكد( pSrc!=0 );
  إذا( p_عبارت->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    أرجع 0;                                                          /* (4) */
  }
  aff1 = sqlite3_عبارت_Affinity(p_عبارت->p_چپ);
  aff2 = sqlite3_عبارت_Affinity(p_عبارت->p_راست);
  إذا( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    أرجع 0;                                                          /* (5) */
  }
  إذا( !sqlite3_عبارت_Coll_Seq_تطابق(pParse, p_عبارت->p_چپ, p_عبارت->p_راست) ){
    أرجع 0;                                                          /* (6) */
  }
  أرجع 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ساكن Bitmask عبارت_Select_Usage(Where_ماسک_مجموعه *p_ماسک_مجموعه, Select *pS){
  Bitmask ماسک = 0;
  طالما( pS ){
    SrcList *pSrc = pS->pSrc;
    ماسک |= sqlite3_Where_عبارت_List_Usage(p_ماسک_مجموعه, pS->pEList);
    ماسک |= sqlite3_Where_عبارت_List_Usage(p_ماسک_مجموعه, pS->p_گروه_By);
    ماسک |= sqlite3_Where_عبارت_List_Usage(p_ماسک_مجموعه, pS->p_ترتیب_By);
    ماسک |= sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, pS->pWhere);
    ماسک |= sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, pS->pHaving);
    إذا( ALWAYS(pSrc!=0) ){
      صحيح i;
      لكل(i=0; i<pSrc->nSrc; i++){
        إذا( pSrc->a[i].fg.isSubquery ){
          ماسک |= عبارت_Select_Usage(p_ماسک_مجموعه, pSrc->a[i].u4.pSubq->pSelect);
        }
        إذا( pSrc->a[i].fg.isUsing==0 ){
          ماسک |= sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, pSrc->a[i].u3.pOn);
        }
        إذا( pSrc->a[i].fg.isTabFunc ){
          ماسک |= sqlite3_Where_عبارت_List_Usage(p_ماسک_مجموعه, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->p_پیشین;
  }
  أرجع ماسک;
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
ساكن SQLITE_NOINLINE صحيح عبارت_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *ai_Cur_ستون,         /* Write the referenced table cursor and column here */
  عبارت *p_عبارت,           /* An operand of a comparison operator */
  صحيح j                  /* Start looking with the j-th pFrom entry */
){
  شاخص *pIdx;
  صحيح i;
  صحيح iCur;
  افعل{
    iCur = pFrom->a[j].iCursor;
    لكل(pIdx=pFrom->a[j].pSTab->p_شاخص; pIdx; pIdx=pIdx->p_بعدی){
      إذا( pIdx->a_ستون_عبارت==0 ) استمر;
      لكل(i=0; i<pIdx->n_کلید_ستون; i++){
        إذا( pIdx->ai_ستونی[i]!=XN_عبارت ) استمر;
        أكد( pIdx->b_Has_عبارت );
        إذا( sqlite3_عبارت_Compare_Skip(p_عبارت,pIdx->a_ستون_عبارت->a[i].p_عبارت,iCur)==0
         && !sqlite3_عبارت_Is_Constant(0,pIdx->a_ستون_عبارت->a[i].p_عبارت)
        ){
          ai_Cur_ستون[0] = iCur;
          ai_Cur_ستون[1] = XN_عبارت;
          أرجع 1;
        }
      }
    }
  }طالما( ++j < pFrom->nSrc );
  أرجع 0;
}
ساكن صحيح عبارت_Might_Be_نمایهای(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *ai_Cur_ستون,         /* Write the referenced table cursor & column here */
  عبارت *p_عبارت,           /* An operand of a comparison operator */
  صحيح op                 /* The specific comparison operator */
){
  صحيح i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  أكد( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  أكد( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_درون<TK_GE );
  أكد( op<=TK_GE );
  إذا( p_عبارت->op==TK_وکتور && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    أكد( عبارت_Use_XList(p_عبارت) );
    p_عبارت = p_عبارت->x.pList->a[0].p_عبارت;
  }

  إذا( p_عبارت->op==TK_ستونی ){
    ai_Cur_ستون[0] = p_عبارت->i_جدول;
    ai_Cur_ستون[1] = p_عبارت->i_ستونی;
    أرجع 1;
  }

  لكل(i=0; i<pFrom->nSrc; i++){
    شاخص *pIdx;
    لكل(pIdx=pFrom->a[i].pSTab->p_شاخص; pIdx; pIdx=pIdx->p_بعدی){
      إذا( pIdx->a_ستون_عبارت ){
        أرجع عبارت_Might_Be_Indexed2(pFrom,ai_Cur_ستون,p_عبارت,i);
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
ساكن فراغ عبارت_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  صحيح idx_جمله               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  Where_جمله *p_جمله;                /* The term to be analyzed */
  Where_ماسک_مجموعه *p_ماسک_مجموعه;          /* Set of table index masks */
  عبارت *p_عبارت;                     /* The expression to be analyzed */
  Bitmask prereq_چپ;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_راست = 0;          /* Extra dependencies on LEFT JOIN */
  عبارت *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  صحيح isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  صحيح noCase = 0;                  /* uppercase equivalent to lowercase */
  صحيح op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  بیعلامت حرف eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  صحيح n_چپ;                       /* Number of elements on left side vector */

  إذا( db->mallocFailed ){
    أرجع;
  }
  أكد( pWC->n_جمله > idx_جمله );
  p_جمله = &pWC->a[idx_جمله];
#اگر_تعریف_شده SQLITE_DEBUG
  p_جمله->i_جمله = idx_جمله;
#پایان_اگر
  p_ماسک_مجموعه = &pWInfo->s_ماسک_مجموعه;
  p_عبارت = p_جمله->p_عبارت;
  أكد( p_عبارت!=0 ); /* Because malloc() has not failed */
  أكد( p_عبارت->op!=TK_AS && p_عبارت->op!=TK_COLLATE );
عبارت_Analyze_restart:
  p_ماسک_مجموعه->bVarSelect = 0;
  prereq_چپ = sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, p_عبارت->p_چپ);
  op = p_عبارت->op;
  إذا( op==TK_درون ){
    أكد( p_عبارت->p_راست==0 );
    إذا( sqlite3_عبارت_Check_درون(pParse, p_عبارت) ) أرجع;
    إذا( عبارت_Use_XSelect(p_عبارت) ){
      p_جمله->prereq_راست = عبارت_Select_Usage(p_ماسک_مجموعه, p_عبارت->x.pSelect);
    }وإلا{
      p_جمله->prereq_راست = sqlite3_Where_عبارت_List_Usage(p_ماسک_مجموعه, p_عبارت->x.pList);
    }
    prereqAll = prereq_چپ | p_جمله->prereq_راست;
  }وإلا{
    p_جمله->prereq_راست = sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, p_عبارت->p_راست);
    إذا( p_عبارت->p_چپ==0
     || عبارت_Has_Property(p_عبارت, EP_xIsSelect|EP_If_Null_ردیف)
     || p_عبارت->x.pList!=0
    ){
      prereqAll = sqlite3_Where_عبارت_Usage_NN(p_ماسک_مجموعه, p_عبارت);
    }وإلا{
      prereqAll = prereq_چپ | p_جمله->prereq_راست;
    }
  }
  إذا( p_ماسک_مجموعه->bVarSelect ) p_جمله->wtFlags |= جمله_VARSELECT;

#اگر_تعریف_شده SQLITE_DEBUG
  إذا( prereqAll!=sqlite3_Where_عبارت_Usage_NN(p_ماسک_مجموعه, p_عبارت) ){
    چاپ_کن("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_درخت_View_عبارت(0,p_عبارت,0);
    أكد( 0 );
  }
#پایان_اگر

  إذا( عبارت_Has_Property(p_عبارت, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3_Where_Get_ماسک(p_ماسک_مجموعه, p_عبارت->w.iJoin);
    إذا( عبارت_Has_Property(p_عبارت, EP_OuterON) ){
      prereqAll |= x;
      extra_راست = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }وإلا إذا( (prereqAll>>1)>=x ){
      عبارت_Clear_Property(p_عبارت, EP_InnerON);
    }
  }
  p_جمله->prereqAll = prereqAll;
  p_جمله->چپ_Cursor = -1;
  p_جمله->i_والد = -1;
  p_جمله->eOperator = 0;
  إذا( allowedOp(op) ){
    صحيح ai_Cur_ستون[2];
    عبارت *p_چپ = sqlite3_عبارت_Skip_Collate(p_عبارت->p_چپ);
    عبارت *p_راست = sqlite3_عبارت_Skip_Collate(p_عبارت->p_راست);
    u16 op_ماسک = (p_جمله->prereq_راست & prereq_چپ)==0 ? WO_ALL : WO_EQUIV;

    إذا( p_جمله->u.x.i_فیلد>0 ){
      أكد( op==TK_درون );
      أكد( p_چپ->op==TK_وکتور );
      أكد( عبارت_Use_XList(p_چپ) );
      p_چپ = p_چپ->x.pList->a[p_جمله->u.x.i_فیلد-1].p_عبارت;
    }

    إذا( عبارت_Might_Be_نمایهای(pSrc, ai_Cur_ستون, p_چپ, op) ){
      p_جمله->چپ_Cursor = ai_Cur_ستون[0];
      أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 );
      p_جمله->u.x.چپ_ستونی = ai_Cur_ستون[1];
      p_جمله->eOperator = operator_ماسک(op) & op_ماسک;
    }
    إذا( op==TK_IS ) p_جمله->wtFlags |= جمله_IS;
    إذا( p_راست
     && عبارت_Might_Be_نمایهای(pSrc, ai_Cur_ستون, p_راست, op)
     && !عبارت_Has_Property(p_راست, EP_Fixed_ستون)
    ){
      Where_جمله *pNew;
      عبارت *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      أكد( p_جمله->u.x.i_فیلد==0 );
      إذا( p_جمله->چپ_Cursor>=0 ){
        صحيح idxNew;
        pDup = sqlite3_عبارت_Dup(db, p_عبارت, 0);
        إذا( db->mallocFailed ){
          sqlite3_عبارت_Delete(db, pDup);
          أرجع;
        }
        idxNew = where_Clause_درج(pWC, pDup, جمله_VIRTUAL|جمله_DYNAMIC);
        إذا( idxNew==0 ) أرجع;
        pNew = &pWC->a[idxNew];
        mark_جمله_As_فرزند(pWC, idxNew, idx_جمله);
        إذا( op==TK_IS ) pNew->wtFlags |= جمله_IS;
        p_جمله = &pWC->a[idx_جمله];
        p_جمله->wtFlags |= جمله_COPIED;
        أكد( pWInfo->pTabList!=0 );
        إذا( جمله_Is_Equivalence(pParse, pDup, pWInfo->pTabList) ){
          p_جمله->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }وإلا{
        pDup = p_عبارت;
        pNew = p_جمله;
      }
      pNew->wtFlags |= عبارت_Commute(pParse, pDup);
      pNew->چپ_Cursor = ai_Cur_ستون[0];
      أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.چپ_ستونی = ai_Cur_ستون[1];
      testcase( (prereq_چپ | extra_راست) != prereq_چپ );
      pNew->prereq_راست = prereq_چپ | extra_راست;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operator_ماسک(pDup->op) + eExtraOp) & op_ماسک;
    }وإلا
    إذا( op==TK_ISNULL
     && !عبارت_Has_Property(p_عبارت,EP_OuterON)
     && 0==sqlite3_عبارت_Can_Be_Null(p_چپ)
    ){
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      p_عبارت->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_عبارت->u.zToken = "false";
      عبارت_مجموعه_Property(p_عبارت, EP_IsFalse);
      p_جمله->prereqAll = 0;
      p_جمله->eOperator = 0;
    }
  }

#اگر_تعریف_نشده SQLITE_OMIT_BETWEEN_OPTIMIZATION
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
  وإلا إذا( p_عبارت->op==TK_BETWEEN && pWC->op==TK_AND ){
    عبارت_List *pList;
    صحيح i;
    ساكن ثابت u8 ops[] = {TK_GE, TK_LE};
    أكد( عبارت_Use_XList(p_عبارت) );
    pList = p_عبارت->x.pList;
    أكد( pList!=0 );
    أكد( pList->n_عبارت==2 );
    أكد( pWC->a[idx_جمله].n_فرزند==0 );
    لكل(i=0; i<2; i++){
      عبارت *p_New_عبارت;
      صحيح idxNew;
      p_New_عبارت = sqlite3PExpr(pParse, ops[i],
                             sqlite3_عبارت_Dup(db, p_عبارت->p_چپ, 0),
                             sqlite3_عبارت_Dup(db, pList->a[i].p_عبارت, 0));
      transferJoinMarkings(p_New_عبارت, p_عبارت);
      idxNew = where_Clause_درج(pWC, p_New_عبارت, جمله_VIRTUAL|جمله_DYNAMIC);
      testcase( idxNew==0 );
      عبارت_Analyze(pSrc, pWC, idxNew);
      p_جمله = &pWC->a[idx_جمله];
      mark_جمله_As_فرزند(pWC, idxNew, idx_جمله);
    }
  }
#پایان_اگر /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#إذا !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  وإلا إذا( p_عبارت->op==TK_OR && !عبارت_Has_Property(p_عبارت, EP_Collate) ){
    عبارت *pAlt = sqlite3_عبارت_Simplified_And_Or(p_عبارت);
    إذا( pAlt!=p_عبارت ){
      p_جمله->p_عبارت = p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely(pAlt);
      اذهب عبارت_Analyze_restart;
    }
    أكد( pWC->op==TK_AND );
    عبارت_Analyze_Or_جمله(pSrc, pWC, idx_جمله);
    p_جمله = &pWC->a[idx_جمله];
  }
#پایان_اگر /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  وإلا إذا( p_عبارت->op==TK_NOTNULL ){
    إذا( p_عبارت->p_چپ->op==TK_ستونی
     && p_عبارت->p_چپ->i_ستونی>=0
     && !عبارت_Has_Property(p_عبارت, EP_OuterON)
    ){
      عبارت *p_New_عبارت;
      عبارت *p_چپ = p_عبارت->p_چپ;
      صحيح idxNew;
      Where_جمله *p_New_جمله;
 
      p_New_عبارت = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_عبارت_Dup(db, p_چپ, 0),
                              sqlite3_عبارت_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_درج(pWC, p_New_عبارت,
                                جمله_VIRTUAL|جمله_DYNAMIC|جمله_VNULL);
      إذا( idxNew ){
        p_New_جمله = &pWC->a[idxNew];
        p_New_جمله->prereq_راست = 0;
        p_New_جمله->چپ_Cursor = p_چپ->i_جدول;
        p_New_جمله->u.x.چپ_ستونی = p_چپ->i_ستونی;
        p_New_جمله->eOperator = WO_GT;
        mark_جمله_As_فرزند(pWC, idxNew, idx_جمله);
        p_جمله = &pWC->a[idx_جمله];
        p_جمله->wtFlags |= جمله_COPIED;
        p_New_جمله->prereqAll = p_جمله->prereqAll;
      }
    }
  }


#اگر_تعریف_نشده SQLITE_OMIT_LIKE_OPTIMIZATION
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
  وإلا إذا( p_عبارت->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_عبارت, &pStr1, &isComplete, &noCase)
  ){
    عبارت *p_چپ;       /* LHS of LIKE/GLOB operator */
    عبارت *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    عبارت *pNewExpr1;
    عبارت *pNewExpr2;
    صحيح idxNew1;
    صحيح idxNew2;
    ثابت حرف *zCollSeqName;     /* Name of collating sequence */
    ثابت u16 wtFlags = جمله_LIKEOPT | جمله_VIRTUAL | جمله_DYNAMIC;

    أكد( عبارت_Use_XList(p_عبارت) );
    p_چپ = p_عبارت->x.pList->a[1].p_عبارت;
    pStr2 = sqlite3_عبارت_Dup(db, pStr1, 0);
    أكد( pStr1==0 || !عبارت_Has_Property(pStr1, EP_Int_مقدار) );
    أكد( pStr2==0 || !عبارت_Has_Property(pStr2, EP_Int_مقدار) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    إذا( noCase && !pParse->db->mallocFailed ){
      صحيح i;
      حرف c;
      p_جمله->wtFlags |= جمله_LIKE;
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
        *pC = sqlite3_فوقانی_مقصد_پایینی[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      طالما( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      أكد( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_دودویی;
    pNewExpr1 = sqlite3_عبارت_Dup(db, p_چپ, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_عبارت_جمع_کن_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_عبارت);
    idxNew1 = where_Clause_درج(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_عبارت_Dup(db, p_چپ, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_عبارت_جمع_کن_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_عبارت);
    idxNew2 = where_Clause_درج(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    عبارت_Analyze(pSrc, pWC, idxNew1);
    عبارت_Analyze(pSrc, pWC, idxNew2);
    p_جمله = &pWC->a[idx_جمله];
    إذا( isComplete ){
      mark_جمله_As_فرزند(pWC, idxNew1, idx_جمله);
      mark_جمله_As_فرزند(pWC, idxNew2, idx_جمله);
    }
  }
#پایان_اگر /* SQLITE_OMIT_LIKE_OPTIMIZATION */

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
  إذا( (p_عبارت->op==TK_EQ || p_عبارت->op==TK_IS)
   && (n_چپ = sqlite3_عبارت_وکتور_اندازه(p_عبارت->p_چپ))>1
   && sqlite3_عبارت_وکتور_اندازه(p_عبارت->p_راست)==n_چپ
   && ( (p_عبارت->p_چپ->flags & EP_xIsSelect)==0
     || (p_عبارت->p_راست->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    صحيح i;
    لكل(i=0; i<n_چپ; i++){
      صحيح idxNew;
      عبارت *pNew;
      عبارت *p_چپ = sqlite3_عبارت_For_وکتور_فیلد(pParse, p_عبارت->p_چپ, i, n_چپ);
      عبارت *p_راست = sqlite3_عبارت_For_وکتور_فیلد(pParse, p_عبارت->p_راست, i, n_چپ);

      pNew = sqlite3PExpr(pParse, p_عبارت->op, p_چپ, p_راست);
      transferJoinMarkings(pNew, p_عبارت);
      idxNew = where_Clause_درج(pWC, pNew, جمله_DYNAMIC|جمله_برش);
      عبارت_Analyze(pSrc, pWC, idxNew);
    }
    p_جمله = &pWC->a[idx_جمله];
    p_جمله->wtFlags |= جمله_CODED|جمله_VIRTUAL;  /* Disable the original */
    p_جمله->eOperator = WO_ROWVAL;
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
  وإلا إذا( p_عبارت->op==TK_درون
   && p_جمله->u.x.i_فیلد==0
   && p_عبارت->p_چپ->op==TK_وکتور
   && ALWAYS( عبارت_Use_XSelect(p_عبارت) )
   && (p_عبارت->x.pSelect->p_پیشین==0 || (p_عبارت->x.pSelect->selFlags & SF_مقادیر))
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
   && p_عبارت->x.pSelect->pWin==0
#پایان_اگر
   && (p_عبارت->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_عبارت->x.pSelect->pEList->n_عبارت <= UMXV(p_جمله->n_فرزند)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    صحيح i;
    أكد( p_جمله->n_فرزند==0 );
    لكل(i=0; i<sqlite3_عبارت_وکتور_اندازه(p_عبارت->p_چپ); i++){
      صحيح idxNew;
      idxNew = where_Clause_درج(pWC, p_عبارت, جمله_VIRTUAL|جمله_برش);
      pWC->a[idxNew].u.x.i_فیلد = i+1;
      عبارت_Analyze(pSrc, pWC, idxNew);
      mark_جمله_As_فرزند(pWC, idxNew, idx_جمله);
    }
  }

#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
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
    عبارت *p_راست = 0, *p_چپ = 0;
    صحيح res = isAuxiliaryVtabOperator(db, p_عبارت, &eOp2, &p_چپ, &p_راست);
    طالما( res-- > 0 ){
      صحيح idxNew;
      Where_جمله *p_New_جمله;
      Bitmask prereq_ستونی, prereq_عبارت;

      prereq_عبارت = sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, p_راست);
      prereq_ستونی = sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, p_چپ);
      إذا( (prereq_عبارت & prereq_ستونی)==0 ){
        عبارت *p_New_عبارت;
        p_New_عبارت = sqlite3PExpr(pParse, TK_تطابق,
            0, sqlite3_عبارت_Dup(db, p_راست, 0));
        إذا( عبارت_Has_Property(p_عبارت, EP_OuterON) && p_New_عبارت ){
          عبارت_مجموعه_Property(p_New_عبارت, EP_OuterON);
          p_New_عبارت->w.iJoin = p_عبارت->w.iJoin;
        }
        idxNew = where_Clause_درج(pWC, p_New_عبارت, جمله_VIRTUAL|جمله_DYNAMIC);
        testcase( idxNew==0 );
        p_New_جمله = &pWC->a[idxNew];
        p_New_جمله->prereq_راست = prereq_عبارت | extra_راست;
        p_New_جمله->چپ_Cursor = p_چپ->i_جدول;
        p_New_جمله->u.x.چپ_ستونی = p_چپ->i_ستونی;
        p_New_جمله->eOperator = WO_AUX;
        p_New_جمله->e_تطابق_Op = eOp2;
        mark_جمله_As_فرزند(pWC, idxNew, idx_جمله);
        p_جمله = &pWC->a[idx_جمله];
        p_جمله->wtFlags |= جمله_COPIED;
        p_New_جمله->prereqAll = p_جمله->prereqAll;
      }
      جابجا(عبارت*, p_چپ, p_راست);
    }
  }
#پایان_اگر /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( p_جمله!=&pWC->a[idx_جمله] );
  p_جمله = &pWC->a[idx_جمله];
  p_جمله->prereq_راست |= extra_راست;
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
فراغ sqlite3_Where_تقسیم(WhereClause *pWC, عبارت *p_عبارت, u8 op){
  عبارت *pE2 = sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت);
  pWC->op = op;
  أكد( pE2!=0 || p_عبارت==0 );
  إذا( pE2==0 ) أرجع;
  إذا( pE2->op!=op ){
    where_Clause_درج(pWC, p_عبارت, 0);
  }وإلا{
    sqlite3_Where_تقسیم(pWC, pE2->p_چپ, op);
    sqlite3_Where_تقسیم(pWC, pE2->p_راست, op);
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
ساكن فراغ where_جمع_کن_حد_عبارت(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  صحيح iReg,           /* Register that will hold value of the limit/offset */
  عبارت *p_عبارت,        /* Expression that defines the limit/offset */
  صحيح iCsr,           /* Cursor to which the constraint applies */
  صحيح e_تطابق_Op        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  عبارت *pNew;
  صحيح iVal = 0;

  إذا( sqlite3_عبارت_Is_Integer(p_عبارت, &iVal, pParse, 0) && iVal>=0 ){
    عبارت *pVal = sqlite3_عبارت_Int32(db, iVal);
    إذا( pVal==0 ) أرجع;
    pNew = sqlite3PExpr(pParse, TK_تطابق, 0, pVal);
  }وإلا{
    عبارت *pVal = sqlite3_عبارت_Alloc(db, TK_ثبات, 0, 0);
    إذا( pVal==0 ) أرجع;
    pVal->i_جدول = iReg;
    pNew = sqlite3PExpr(pParse, TK_تطابق, 0, pVal);
  }
  إذا( pNew ){
    Where_جمله *p_جمله;
    صحيح idx;
    idx = where_Clause_درج(pWC, pNew, جمله_DYNAMIC|جمله_VIRTUAL);
    p_جمله = &pWC->a[idx];
    p_جمله->چپ_Cursor = iCsr;
    p_جمله->eOperator = WO_AUX;
    p_جمله->e_تطابق_Op = e_تطابق_Op;
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
فراغ SQLITE_NOINLINE sqlite3_Where_جمع_کن_حد(WhereClause *pWC, Select *p){
  أكد( p!=0 && p->p_حد!=0 );                 /* 1 -- checked by caller */
  إذا( p->p_گروه_By==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    عبارت_List *p_ترتیب_By = p->p_ترتیب_By;
    صحيح iCsr = p->pSrc->a[0].iCursor;
    صحيح ii;

    /* Check condition (4). Return early if it is not met. */
    لكل(ii=0; ii<pWC->n_جمله; ii++){
      إذا( pWC->a[ii].wtFlags & جمله_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        أكد( pWC->a[ii].wtFlags & جمله_VIRTUAL );
        أكد( pWC->a[ii].eOperator==WO_ROWVAL );
        استمر;
      }
      إذا( pWC->a[ii].n_فرزند ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        استمر;
      }
      إذا( pWC->a[ii].چپ_Cursor==iCsr && pWC->a[ii].prereq_راست==0 ) استمر;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      إذا( pWC->a[ii].i_والد>=0 ){
        Where_جمله *p_والد = &pWC->a[ pWC->a[ii].i_والد ];
        إذا( p_والد->چپ_Cursor==iCsr
         && p_والد->prereq_راست==0
         && p_والد->n_فرزند==1
        ){
          استمر;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      أرجع;
    }

    /* Check condition (5). Return early if it is not met. */
    إذا( p_ترتیب_By ){
      لكل(ii=0; ii<p_ترتیب_By->n_عبارت; ii++){
        عبارت *p_عبارت = p_ترتیب_By->a[ii].p_عبارت;
        إذا( p_عبارت->op!=TK_ستونی ) أرجع;
        إذا( p_عبارت->i_جدول!=iCsr ) أرجع;
        إذا( p_ترتیب_By->a[ii].fg.مرتب_سازی_Flags & KEYINFO_ترتیب_BIGNULL ) أرجع;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    أكد( p->p_حد->op==TK_حد );
    إذا( p->i_جابه_جایی!=0 && (p->selFlags & SF_Compound)==0 ){
      where_جمع_کن_حد_عبارت(pWC, p->i_جابه_جایی, p->p_حد->p_راست,
                        iCsr, SQLITE_شاخص_CONSTRAINT_جابه_جایی);
    }
    إذا( p->i_جابه_جایی==0 || (p->selFlags & SF_Compound)==0 ){
      where_جمع_کن_حد_عبارت(pWC, p->i_حد, p->p_حد->p_چپ,
                        iCsr, SQLITE_شاخص_CONSTRAINT_حد);
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
  pWC->n_جمله = 0;
  pWC->n_پایه = 0;
  pWC->n_جایگاه = Array_اندازه(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
فراغ sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  أكد( pWC->n_جمله>=pWC->n_پایه );
  إذا( pWC->n_جمله>0 ){
    Where_جمله *a = pWC->a;
    Where_جمله *a_آخرین = &pWC->a[pWC->n_جمله-1];
#اگر_تعریف_شده SQLITE_DEBUG
    صحيح i;
    /* Verify that every term past pWC->nBase is virtual */
    لكل(i=pWC->n_پایه; i<pWC->n_جمله; i++){
      أكد( (pWC->a[i].wtFlags & جمله_VIRTUAL)!=0 );
    }
#پایان_اگر
    طالما(1){
      أكد( a->e_تطابق_Op==0 || a->eOperator==WO_AUX );
      إذا( a->wtFlags & جمله_DYNAMIC ){
        sqlite3_عبارت_Delete(db, a->p_عبارت);
      }
      إذا( a->wtFlags & (جمله_ORINFO|جمله_ANDINFO) ){
        إذا( a->wtFlags & جمله_ORINFO ){
          أكد( (a->wtFlags & جمله_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }وإلا{
          أكد( (a->wtFlags & جمله_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      إذا( a==a_آخرین ) اكسر;
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
ساكن SQLITE_NOINLINE Bitmask sqlite3_Where_عبارت_Usage_پر(
  Where_ماسک_مجموعه *p_ماسک_مجموعه,
  عبارت *p
){
  Bitmask ماسک;
  ماسک = (p->op==TK_IF_NULL_ردیف) ? sqlite3_Where_Get_ماسک(p_ماسک_مجموعه, p->i_جدول) : 0;
  إذا( p->p_چپ ) ماسک |= sqlite3_Where_عبارت_Usage_NN(p_ماسک_مجموعه, p->p_چپ);
  إذا( p->p_راست ){
    ماسک |= sqlite3_Where_عبارت_Usage_NN(p_ماسک_مجموعه, p->p_راست);
    أكد( p->x.pList==0 );
  }وإلا إذا( عبارت_Use_XSelect(p) ){
    إذا( عبارت_Has_Property(p, EP_VarSelect) ) p_ماسک_مجموعه->bVarSelect = 1;
    ماسک |= عبارت_Select_Usage(p_ماسک_مجموعه, p->x.pSelect);
  }وإلا إذا( p->x.pList ){
    ماسک |= sqlite3_Where_عبارت_List_Usage(p_ماسک_مجموعه, p->x.pList);
  }
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
  إذا( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && عبارت_Use_YWin(p) ){
    أكد( p->y.pWin!=0 );
    ماسک |= sqlite3_Where_عبارت_List_Usage(p_ماسک_مجموعه, p->y.pWin->p_افراز);
    ماسک |= sqlite3_Where_عبارت_List_Usage(p_ماسک_مجموعه, p->y.pWin->p_ترتیب_By);
    ماسک |= sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, p->y.pWin->pFilter);
  }
#پایان_اگر
  أرجع ماسک;
}
Bitmask sqlite3_Where_عبارت_Usage_NN(Where_ماسک_مجموعه *p_ماسک_مجموعه, عبارت *p){
  إذا( p->op==TK_ستونی && !عبارت_Has_Property(p, EP_Fixed_ستون) ){
    أرجع sqlite3_Where_Get_ماسک(p_ماسک_مجموعه, p->i_جدول);
  }وإلا إذا( عبارت_Has_Property(p, EP_TokenOnly|EP_برگ) ){
    أكد( p->op!=TK_IF_NULL_ردیف );
    أرجع 0;
  }
  أرجع sqlite3_Where_عبارت_Usage_پر(p_ماسک_مجموعه, p);
}
Bitmask sqlite3_Where_عبارت_Usage(Where_ماسک_مجموعه *p_ماسک_مجموعه, عبارت *p){
  أرجع p ? sqlite3_Where_عبارت_Usage_NN(p_ماسک_مجموعه,p) : 0;
}
Bitmask sqlite3_Where_عبارت_List_Usage(Where_ماسک_مجموعه *p_ماسک_مجموعه, عبارت_List *pList){
  صحيح i;
  Bitmask ماسک = 0;
  إذا( pList ){
    لكل(i=0; i<pList->n_عبارت; i++){
      ماسک |= sqlite3_Where_عبارت_Usage(p_ماسک_مجموعه, pList->a[i].p_عبارت);
    }
  }
  أرجع ماسک;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
فراغ sqlite3_Where_عبارت_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  صحيح i;
  لكل(i=pWC->n_جمله-1; i>=0; i--){
    عبارت_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
فراغ sqlite3_Where_Tab_Func_آرگومانها(
  Parse *pParse,                    /* Parsing context */
  Src_قلم *p_قلم,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  جدول *pTab;
  صحيح j, k;
  عبارت_List *p_آرگومانها;
  عبارت *p_ستون_Ref;
  عبارت *p_جمله;
  إذا( p_قلم->fg.isTabFunc==0 ) أرجع;
  pTab = p_قلم->pSTab;
  أكد( pTab!=0 );
  إذا( !IsVirtual(pTab) ){
    sqlite3_Error_پیام(pParse, "'%s' is not a function", p_قلم->zName);
    أرجع;
  }
  p_آرگومانها = p_قلم->u1.pFuncArg;
  إذا( p_آرگومانها==0 ) أرجع;
  لكل(j=k=0; j<p_آرگومانها->n_عبارت; j++){
    عبارت *pRhs;
    u32 joinType;
    طالما( k<pTab->n_ستون && (pTab->a_ستون[k].ستون_Flags & COLFLAG_HIDDEN)==0 ){k++;}
    إذا( k>=pTab->n_ستون ){
      sqlite3_Error_پیام(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      أرجع;
    }
    p_ستون_Ref = sqlite3_عبارت_Alloc(pParse->db, TK_ستونی, 0, 0);
    إذا( p_ستون_Ref==0 ) أرجع;
    p_ستون_Ref->i_جدول = p_قلم->iCursor;
    p_ستون_Ref->i_ستونی = k++;
    أكد( عبارت_Use_YTab(p_ستون_Ref) );
    p_ستون_Ref->y.pTab = pTab;
    p_قلم->ستون_استفاده_شده |= sqlite3_عبارت_ستون_استفاده_شده(p_ستون_Ref);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_عبارت_Dup(pParse->db, p_آرگومانها->a[j].p_عبارت, 0), 0);
    p_جمله = sqlite3PExpr(pParse, TK_EQ, p_ستون_Ref, pRhs);
    إذا( p_قلم->fg.jointype & (JT_چپ|JT_راست) ){
      testcase( p_قلم->fg.jointype & JT_چپ );  /* testtag-20230227a */
      testcase( p_قلم->fg.jointype & JT_راست ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }وإلا{
      testcase( p_قلم->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_مجموعه_Join_عبارت(p_جمله, p_قلم->iCursor, joinType);
    where_Clause_درج(pWC, p_جمله, جمله_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
صحيح sqlite3WhereLoopBloomable(ثابت WhereLoop *pLoop){
  إذا( pLoop->wsFlags & WHERE_IPK ) أرجع 1;
  إذا( NEVER((pLoop->wsFlags & WHERE_نمایهای)==0) ) أرجع 0;
  أرجع sqlite3_شاخص_Bloomable(pLoop->u.btree.p_شاخص, pLoop->u.btree.nEq);
}
