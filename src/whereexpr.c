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
#شامل "sqliteInt.h"
#شامل "whereInt.h"

/* Forward declarations */
ساکن خالی اظہاریہ_Analyze(SrcList*, WhereClause*, صحیح);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
ساکن خالی whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
ساکن خالی whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
ساکن صحیح where_Clause_داخل_کرو(WhereClause *pWC, اظہاریہ *p, u16 wtFlags){
  Where_رکن *p_رکن;
  صحیح idx;
  testcase( wtFlags & رکن_VIRTUAL );
  اگر( pWC->n_رکن>=pWC->n_خانہ ){
    Where_رکن *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, حجم(pWC->a[0])*pWC->n_خانہ*2 );
    اگر( pWC->a==0 ){
      اگر( wtFlags & رکن_DYNAMIC ){
        sqlite3_اظہاریہ_Delete(db, p);
      }
      pWC->a = pOld;
      واپس 0;
    }
    حافظہ_نقل(pWC->a, pOld, حجم(pWC->a[0])*pWC->n_رکن);
    pWC->n_خانہ = pWC->n_خانہ*2;
  }
  p_رکن = &pWC->a[idx = pWC->n_رکن++];
  اگر( (wtFlags & رکن_VIRTUAL)==0 ) pWC->n_بنیاد = pWC->n_رکن;
  اگر( p && اظہاریہ_Has_Property(p, EP_Unlikely) ){
    p_رکن->truthProb = sqlite3LogEst(p->i_جدول) - 270;
  }ورنہ{
    p_رکن->truthProb = 1;
  }
  p_رکن->p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p);
  p_رکن->wtFlags = wtFlags;
  p_رکن->pWC = pWC;
  p_رکن->i_والد = -1;
  memset(&p_رکن->eOperator, 0,
         حجم(Where_رکن) - offsetof(Where_رکن,eOperator));
  واپس idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
ساکن صحیح allowedOp(صحیح op){
  أكد( TK_GT>TK_EQ && TK_GT<TK_GE );
  أكد( TK_LT>TK_EQ && TK_LT<TK_GE );
  أكد( TK_LE>TK_EQ && TK_LE<TK_GE );
  أكد( TK_GE==TK_EQ+4 );
  أكد( TK_اندر<TK_EQ );
  أكد( TK_IS<TK_EQ );
  أكد( TK_ISNULL<TK_EQ );
  اگر( op>TK_GE ) واپس 0;
  اگر( op>=TK_EQ ) واپس 1;
  واپس op==TK_اندر || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ساکن u16 اظہاریہ_Commute(Parse *pParse, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->p_بایاں->op==TK_سمتیہ
   || p_اظہاریہ->p_دایاں->op==TK_سمتیہ
   || sqlite3_ثنائی_Compare_Coll_Seq(pParse, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں) !=
      sqlite3_ثنائی_Compare_Coll_Seq(pParse, p_اظہاریہ->p_دایاں, p_اظہاریہ->p_بایاں)
  ){
    p_اظہاریہ->flags ^= EP_Commuted;
  }
  تبادلہ(اظہاریہ*,p_اظہاریہ->p_دایاں,p_اظہاریہ->p_بایاں);
  اگر( p_اظہاریہ->op>=TK_GT ){
    أكد( TK_LT==TK_GT+2 );
    أكد( TK_GE==TK_LE+2 );
    أكد( TK_GT>TK_EQ );
    أكد( TK_GT<TK_LE );
    أكد( p_اظہاریہ->op>=TK_GT && p_اظہاریہ->op<=TK_GE );
    p_اظہاریہ->op = ((p_اظہاریہ->op-TK_GT)^2)+TK_GT;
  }
  واپس 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ساکن u16 operator_نقاب(صحیح op){
  u16 c;
  أكد( allowedOp(op) );
  اگر( op>=TK_EQ ){
    أكد( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }ورنہ اگر( op==TK_اندر ){
    c = WO_اندر;
  }ورنہ اگر( op==TK_ISNULL ){
    c = WO_ISNULL;
  }ورنہ{
    أكد( op==TK_IS );
    c = WO_IS;
  }
  أكد( op!=TK_ISNULL || c==WO_ISNULL );
  أكد( op!=TK_اندر || c==WO_اندر );
  أكد( op!=TK_EQ || c==WO_EQ );
  أكد( op!=TK_LT || c==WO_LT );
  أكد( op!=TK_LE || c==WO_LE );
  أكد( op!=TK_GT || c==WO_GT );
  أكد( op!=TK_GE || c==WO_GE );
  أكد( op!=TK_IS || c==WO_IS );
  واپس c;
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LIKE_OPTIMIZATION
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
ساکن صحیح isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  اظہاریہ *p_اظہاریہ,      /* Test this expression */
  اظہاریہ **pp_سابقہ,  /* Pointer to TK_STRING expression with pattern prefix */
  صحیح *pisComplete, /* True if the only wildcard is % in the last character */
  صحیح *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ثابت u8 *z = 0;           /* String on RHS of LIKE operator */
  اظہاریہ *p_دایاں, *p_بایاں;      /* Right and left size of LIKE operator */
  اظہاریہ_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  صحیح cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_قدر *pVal = 0;
  صحیح op;                    /* Opcode of pRight */
  صحیح rc;                    /* Result code to return */

  اگر( !sqlite3IsLikeFunction(db, p_اظہاریہ, pnoCase, (حرف*)wc) ){
    واپس 0;
  }
#اگر_تعریف_شدہ SQLITE_EBCDIC
  اگر( *pnoCase ) واپس 0;
#اگر_ختم
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  pList = p_اظہاریہ->x.pList;
  p_بایاں = pList->a[1].p_اظہاریہ;

  p_دایاں = sqlite3_اظہاریہ_Skip_Collate(pList->a[0].p_اظہاریہ);
  op = p_دایاں->op;
  اگر( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    صحیح i_ستون = p_دایاں->i_عمود;
    pVal = sqlite3_Vdbe_Get_Bound_قدر(pReprepare, i_ستون, SQLITE_AFF_BLOB);
    اگر( pVal && sqlite3_قدر_type(pVal)==SQLITE_متن ){
      z = sqlite3_قدر_متن(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, i_ستون, 0);
    أكد( p_دایاں->op==TK_VARIABLE || p_دایاں->op==TK_رجسٹر );
  }ورنہ اگر( op==TK_STRING ){
    أكد( !اظہاریہ_Has_Property(p_دایاں, EP_Int_قدر) );
     z = (u8*)p_دایاں->u.zToken;
  }
  اگر( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    جب( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      اگر( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }ورنہ اگر( c>=0x80 ){
        ثابت u8 *z2 = z+cnt-1;
        اگر( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          توڑو;
        }ورنہ{
          cnt = (صحیح)(z2-z);
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
    اگر( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      اظہاریہ *p_سابقہ;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      p_سابقہ = sqlite3_اظہاریہ(db, TK_STRING, (حرف*)z);
      اگر( p_سابقہ ){
        صحیح iFrom, i_مقصد;
        حرف *zNew;
        أكد( !اظہاریہ_Has_Property(p_سابقہ, EP_Int_قدر) );
        zNew = p_سابقہ->u.zToken;
        zNew[cnt] = 0;
        برائے(iFrom=i_مقصد=0; iFrom<cnt; iFrom++){
          اگر( zNew[iFrom]==wc[3] ) iFrom++;
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
        اگر( p_بایاں->op!=TK_عمود
         || sqlite3_اظہاریہ_Affinity(p_بایاں)!=SQLITE_AFF_متن
         || (ALWAYS( اظہاریہ_Use_YTab(p_بایاں) )
             && ALWAYS(p_بایاں->y.pTab)
             && IsVirtual(p_بایاں->y.pTab))  /* Might be numeric */
        ){
          صحیح is_عدد;
          دوہرا rDummy;
          أكد( zNew[i_مقصد]==0 );
          is_عدد = sqlite3AtoF(zNew, &rDummy);
          اگر( is_عدد<=0 ){
            اگر( i_مقصد==1 && zNew[0]=='-' ){
              is_عدد = +1;
            }ورنہ{
              zNew[i_مقصد-1]++;
              is_عدد = sqlite3AtoF(zNew, &rDummy);
              zNew[i_مقصد-1]--;
            }
          }
          اگر( is_عدد>0 ){
            sqlite3_اظہاریہ_Delete(db, p_سابقہ);
            sqlite3_قدر_Free(pVal);
            واپس 0;
          }
        }
      }
      *pp_سابقہ = p_سابقہ;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      اگر( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_دایاں->i_عمود, 0);
        أكد( !اظہاریہ_Has_Property(p_دایاں, EP_Int_قدر) );
        اگر( *pisComplete && p_دایاں->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          صحیح r1 = sqlite3_Get_عارضی_Reg(pParse);
          sqlite3_اظہاریہ_رمز_ہدف(pParse, p_دایاں, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3_Release_عارضی_Reg(pParse, r1);
        }
      }
    }ورنہ{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_قدر_Free(pVal);
  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
صحیح sqlite3_اظہاریہ_Is_Like_Operator(ثابت اظہاریہ *p_اظہاریہ){
  ساکن ثابت ساخت {
    ثابت حرف *zOp;
    غیرعلامتی حرف eOp;
  } aOp[] = {
    { "match",  SQLITE_اشاریہ_CONSTRAINT_مطابقت },
    { "glob",   SQLITE_اشاریہ_CONSTRAINT_GLOB },
    { "like",   SQLITE_اشاریہ_CONSTRAINT_LIKE },
    { "regexp", SQLITE_اشاریہ_CONSTRAINT_REGEXP }
  };
  صحیح i;
  أكد( p_اظہاریہ->op==TK_FUNCTION );
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
  برائے(i=0; i<Array_سائز(aOp); i++){
    اگر( sqlite3StrICmp(p_اظہاریہ->u.zToken, aOp[i].zOp)==0 ){
      واپس aOp[i].eOp;
    }
  }
  واپس 0;
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
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
ساکن صحیح isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  اظہاریہ *p_اظہاریہ,                    /* Test this expression */
  غیرعلامتی حرف *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  اظہاریہ **pp_بایاں,                  /* Column expression to left of MATCH/op2 */
  اظہاریہ **pp_دایاں                  /* Expression to left of MATCH/op2 */
){
  اگر( p_اظہاریہ->op==TK_FUNCTION ){
    اظہاریہ_List *pList;
    اظہاریہ *p_ستون;                     /* Column reference */
    صحیح i;

    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    pList = p_اظہاریہ->x.pList;
    اگر( pList==0 || pList->n_اظہاریہ!=2 ){
      واپس 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    p_ستون = pList->a[1].p_اظہاریہ;
    أكد( p_ستون->op!=TK_عمود || (اظہاریہ_Use_YTab(p_ستون) && p_ستون->y.pTab!=0) );
    اگر( اظہاریہ_Is_Vtab(p_ستون) && (i = sqlite3_اظہاریہ_Is_Like_Operator(p_اظہاریہ))!=0 ){
      *peOp2 = i;
      *pp_دایاں = pList->a[0].p_اظہاریہ;
      *pp_بایاں = p_ستون;
      واپس 1;
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
    p_ستون = pList->a[0].p_اظہاریہ;
    أكد( p_ستون->op!=TK_عمود || اظہاریہ_Use_YTab(p_ستون) );
    أكد( p_ستون->op!=TK_عمود || (اظہاریہ_Use_YTab(p_ستون) && p_ستون->y.pTab!=0) );
    اگر( اظہاریہ_Is_Vtab(p_ستون) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      خالی (*x_Not_مستعمل)(sqlite3_context*,صحیح,sqlite3_قدر**);
      خالی *p_Not_مستعمل;
      pVtab = sqlite3GetVTable(db, p_ستون->y.pTab)->pVtab;
      أكد( pVtab!=0 );
      أكد( pVtab->pModule!=0 );
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      pMod = (sqlite3_module *)pVtab->pModule;
      اگر( pMod->x_ڈھونڈو_Function!=0 ){
        i = pMod->x_ڈھونڈو_Function(pVtab,2, p_اظہاریہ->u.zToken, &x_Not_مستعمل, &p_Not_مستعمل);
        اگر( i>=SQLITE_اشاریہ_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_دایاں = pList->a[1].p_اظہاریہ;
          *pp_بایاں = p_ستون;
          واپس 1;
        }
      }
    }
  }ورنہ اگر( p_اظہاریہ->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    أكد( TK_NE < TK_EQ );
    أكد( TK_ISNOT < TK_EQ );
    أكد( TK_NOTNULL < TK_EQ );
    واپس 0;
  }ورنہ اگر( p_اظہاریہ->op==TK_NE || p_اظہاریہ->op==TK_ISNOT || p_اظہاریہ->op==TK_NOTNULL ){
    صحیح res = 0;
    اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
    اظہاریہ *p_دایاں = p_اظہاریہ->p_دایاں;
    أكد( p_بایاں->op!=TK_عمود || (اظہاریہ_Use_YTab(p_بایاں) && p_بایاں->y.pTab!=0) );
    اگر( اظہاریہ_Is_Vtab(p_بایاں) ){
      res++;
    }
    أكد( p_دایاں==0 || p_دایاں->op!=TK_عمود
            || (اظہاریہ_Use_YTab(p_دایاں) && p_دایاں->y.pTab!=0) );
    اگر( p_دایاں && اظہاریہ_Is_Vtab(p_دایاں) ){
      res++;
      تبادلہ(اظہاریہ*, p_بایاں, p_دایاں);
    }
    *pp_بایاں = p_بایاں;
    *pp_دایاں = p_دایاں;
    اگر( p_اظہاریہ->op==TK_NE ) *peOp2 = SQLITE_اشاریہ_CONSTRAINT_NE;
    اگر( p_اظہاریہ->op==TK_ISNOT ) *peOp2 = SQLITE_اشاریہ_CONSTRAINT_ISNOT;
    اگر( p_اظہاریہ->op==TK_NOTNULL ) *peOp2 = SQLITE_اشاریہ_CONSTRAINT_ISNOTNULL;
    واپس res;
  }
  واپس 0;
}
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ساکن خالی transferJoinMarkings(اظہاریہ *pDerived, اظہاریہ *p_بنیاد){
  اگر( pDerived && اظہاریہ_Has_Property(p_بنیاد, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= p_بنیاد->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = p_بنیاد->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ساکن خالی mark_رکن_As_بچہ(WhereClause *pWC, صحیح i_بچہ, صحیح i_والد){
  pWC->a[i_بچہ].i_والد = i_والد;
  pWC->a[i_بچہ].truthProb = pWC->a[i_والد].truthProb;
  أكد(   pWC->a[i_والد].n_بچہ <  UMXV(pWC->a[0].n_بچہ) );
  pWC->a[i_والد].n_بچہ++;
  testcase( pWC->a[i_والد].n_بچہ == UMXV(pWC->a[0].n_بچہ) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ساکن Where_رکن *whereNthSubterm(Where_رکن *p_رکن, صحیح N){
  اگر( p_رکن->eOperator!=WO_AND ){
    واپس N==0 ? p_رکن : 0;
  }
  اگر( N<p_رکن->u.pAndInfo->wc.n_رکن ){
    واپس &p_رکن->u.pAndInfo->wc.a[N];
  }
  واپس 0;
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
ساکن خالی whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  Where_رکن *pOne,       /* First disjunct */
  Where_رکن *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  اظہاریہ *pNew;            /* New virtual expression */
  صحیح op;                /* Operator for the combined expression */
  صحیح idxNew;            /* Index in pWC of the next virtual term */
  اظہاریہ *pA, *pB;         /* Expressions associated with pOne and pTwo */

  اگر( (pOne->wtFlags | pTwo->wtFlags) & رکن_VNULL ) واپس;
  اگر( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) واپس;
  اگر( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) واپس;
  اگر( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) واپس;
  pA = pOne->p_اظہاریہ;
  pB = pTwo->p_اظہاریہ;
  أكد( pA->p_بایاں!=0 && pA->p_دایاں!=0 );
  أكد( pB->p_بایاں!=0 && pB->p_دایاں!=0 );
  اگر( sqlite3_اظہاریہ_Compare(0,pA->p_بایاں, pB->p_بایاں, -1) )  واپس;
  اگر( sqlite3_اظہاریہ_Compare(0,pA->p_دایاں, pB->p_دایاں,-1) ) واپس;
  اگر( اظہاریہ_Has_Property(pA,EP_Commuted)!=اظہاریہ_Has_Property(pB,EP_Commuted) ){
    واپس;
  }
  /* If we reach this point, it means the two subterms can be combined */
  اگر( (eOp & (eOp-1))!=0 ){
    اگر( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }ورنہ{
      أكد( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_اظہاریہ_Dup(db, pA, 0);
  اگر( pNew==0 ) واپس;
  برائے(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ أكد( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_داخل_کرو(pWC, pNew, رکن_VIRTUAL|رکن_DYNAMIC);
  اظہاریہ_Analyze(pSrc, pWC, idxNew);
}

#اگر !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
ساکن خالی اظہاریہ_Analyze_Or_رکن(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  صحیح idx_رکن               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  Where_رکن *p_رکن = &pWC->a[idx_رکن];    /* The term to be analyzed */
  اظہاریہ *p_اظہاریہ = p_رکن->p_اظہاریہ;             /* The expression of the term */
  صحیح i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  Where_رکن *p_Or_رکن;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chng_مقصد_اندر;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  أكد( (p_رکن->wtFlags & (رکن_DYNAMIC|رکن_ORINFO|رکن_ANDINFO))==0 );
  أكد( p_اظہاریہ->op==TK_OR );
  p_رکن->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, حجم(*pOrInfo));
  اگر( pOrInfo==0 ) واپس;
  p_رکن->wtFlags |= رکن_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, حجم(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3_Where_بانٹو(pOrWc, p_اظہاریہ, TK_OR);
  sqlite3_Where_اظہاریہ_Analyze(pSrc, pOrWc);
  اگر( db->mallocFailed ) واپس;
  أكد( pOrWc->n_رکن>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chng_مقصد_اندر = ~(Bitmask)0;
  برائے(i=pOrWc->n_رکن-1, p_Or_رکن=pOrWc->a; i>=0 && indexable; i--, p_Or_رکن++){
    اگر( (p_Or_رکن->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      أكد( (p_Or_رکن->wtFlags & (رکن_ANDINFO|رکن_ORINFO))==0 );
      chng_مقصد_اندر = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, حجم(*pAndInfo));
      اگر( pAndInfo ){
        WhereClause *pAndWC;
        Where_رکن *p_And_رکن;
        صحیح j;
        Bitmask b = 0;
        p_Or_رکن->u.pAndInfo = pAndInfo;
        p_Or_رکن->wtFlags |= رکن_ANDINFO;
        p_Or_رکن->eOperator = WO_AND;
        p_Or_رکن->بایاں_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, حجم(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3_Where_بانٹو(pAndWC, p_Or_رکن->p_اظہاریہ, TK_AND);
        sqlite3_Where_اظہاریہ_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        اگر( !db->mallocFailed ){
          برائے(j=0, p_And_رکن=pAndWC->a; j<pAndWC->n_رکن; j++, p_And_رکن++){
            أكد( p_And_رکن->p_اظہاریہ );
            اگر( allowedOp(p_And_رکن->p_اظہاریہ->op)
             || p_And_رکن->eOperator==WO_AUX
            ){
              b |= sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, p_And_رکن->بایاں_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }ورنہ اگر( p_Or_رکن->wtFlags & رکن_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }ورنہ{
      Bitmask b;
      b = sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, p_Or_رکن->بایاں_Cursor);
      اگر( p_Or_رکن->wtFlags & رکن_VIRTUAL ){
        Where_رکن *pOther = &pOrWc->a[p_Or_رکن->i_والد];
        b |= sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, pOther->بایاں_Cursor);
      }
      indexable &= b;
      اگر( (p_Or_رکن->eOperator & WO_EQ)==0 ){
        chng_مقصد_اندر = 0;
      }ورنہ{
        chng_مقصد_اندر &= b;
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
  اگر( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  اگر( indexable && pOrWc->n_رکن==2 ){
    صحیح iOne = 0;
    Where_رکن *pOne;
    جب( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      صحیح iTwo = 0;
      Where_رکن *pTwo;
      جب( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  اگر( chng_مقصد_اندر ){
    صحیح ok_مقصد_Chng_مقصد_اندر = 0;     /* True if the conversion to IN is valid */
    صحیح i_عمود = -1;         /* Column index on lhs of IN operator */
    صحیح iCursor = -1;         /* Table cursor common to all terms */
    صحیح j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    برائے(j=0; j<2 && !ok_مقصد_Chng_مقصد_اندر; j++){
      اظہاریہ *p_بایاں = 0;
      p_Or_رکن = pOrWc->a;
      برائے(i=pOrWc->n_رکن-1; i>=0; i--, p_Or_رکن++){
        أكد( p_Or_رکن->eOperator & WO_EQ );
        p_Or_رکن->wtFlags &= ~رکن_OK;
        اگر( p_Or_رکن->بایاں_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          أكد( j==1 );
          جاری;
        }
        اگر( (chng_مقصد_اندر & sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ,
                                            p_Or_رکن->بایاں_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( p_Or_رکن->wtFlags & رکن_COPIED );
          testcase( p_Or_رکن->wtFlags & رکن_VIRTUAL );
          أكد( p_Or_رکن->wtFlags & (رکن_COPIED|رکن_VIRTUAL) );
          جاری;
        }
        أكد( (p_Or_رکن->eOperator & (WO_OR|WO_AND))==0 );
        i_عمود = p_Or_رکن->u.x.بایاں_عمود;
        iCursor = p_Or_رکن->بایاں_Cursor;
        p_بایاں = p_Or_رکن->p_اظہاریہ->p_بایاں;
        توڑو;
      }
      اگر( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        أكد( j==1 );
        أكد( Is_Power_کا_Two(chng_مقصد_اندر) );
        أكد( chng_مقصد_اندر==sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سیٹ, iCursor) );
        توڑو;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      ok_مقصد_Chng_مقصد_اندر = 1;
      برائے(; i>=0 && ok_مقصد_Chng_مقصد_اندر; i--, p_Or_رکن++){
        أكد( p_Or_رکن->eOperator & WO_EQ );
        أكد( (p_Or_رکن->eOperator & (WO_OR|WO_AND))==0 );
        اگر( p_Or_رکن->بایاں_Cursor!=iCursor ){
          p_Or_رکن->wtFlags &= ~رکن_OK;
        }ورنہ اگر( p_Or_رکن->u.x.بایاں_عمود!=i_عمود || (i_عمود==XN_اظہاریہ
               && sqlite3_اظہاریہ_Compare(pParse, p_Or_رکن->p_اظہاریہ->p_بایاں, p_بایاں, -1)
        )){
          ok_مقصد_Chng_مقصد_اندر = 0;
        }ورنہ{
          صحیح aff_بایاں, aff_دایاں;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_دایاں = sqlite3_اظہاریہ_Affinity(p_Or_رکن->p_اظہاریہ->p_دایاں);
          aff_بایاں = sqlite3_اظہاریہ_Affinity(p_Or_رکن->p_اظہاریہ->p_بایاں);
          اگر( aff_دایاں!=0 && aff_دایاں!=aff_بایاں ){
            ok_مقصد_Chng_مقصد_اندر = 0;
          }ورنہ{
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
    اگر( ok_مقصد_Chng_مقصد_اندر ){
      اظہاریہ *pDup;            /* A transient duplicate expression */
      اظہاریہ_List *pList = 0;   /* The RHS of the IN operator */
      اظہاریہ *p_بایاں = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      اظہاریہ *pNew;            /* The complete IN operator */

      برائے(i=pOrWc->n_رکن-1, p_Or_رکن=pOrWc->a; i>=0; i--, p_Or_رکن++){
        اظہاریہ *pThis;
        اگر( (p_Or_رکن->wtFlags & رکن_OK)==0 ) جاری;
        أكد( p_Or_رکن->eOperator & WO_EQ );
        أكد( (p_Or_رکن->eOperator & (WO_OR|WO_AND))==0 );
        أكد( p_Or_رکن->بایاں_Cursor==iCursor );
        أكد( p_Or_رکن->u.x.بایاں_عمود==i_عمود );
        pThis = p_Or_رکن->p_اظہاریہ;
        pDup = sqlite3_اظہاریہ_Dup(db, pThis->p_دایاں, 0);
        pList = sqlite3_اظہاریہ_List_Append(pWInfo->pParse, pList, pDup);
        اگر( p_بایاں==0 ){
          p_بایاں = pThis->p_بایاں;
          pCollSeq = sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pThis);
        }ورنہ{
          أكد( 0==sqlite3_اظہاریہ_Compare(pParse, 
                        sqlite3_اظہاریہ_Skip_Collate(pThis->p_بایاں),
                        sqlite3_اظہاریہ_Skip_Collate(p_بایاں), -1) );
          اگر( pCollSeq!=sqlite3_اظہاریہ_Compare_Coll_Seq(pParse, pThis) ){
            p_بایاں = 0;  /* Collating sequence mismatch */
            توڑو;
          }
        }
      }
      اگر( p_بایاں==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }ورنہ{
        pDup = sqlite3_اظہاریہ_Dup(db, p_بایاں, 0);
        اگر( sqlite3_اظہاریہ_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          أكد( pCollSeq->zName!=0 );
          pDup = sqlite3_اظہاریہ_جوڑو_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_اندر, pDup, 0);
      }
      اگر( pNew ){
        صحیح idxNew;
        transferJoinMarkings(pNew, p_اظہاریہ);
        أكد( اظہاریہ_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_داخل_کرو(pWC, pNew, رکن_VIRTUAL|رکن_DYNAMIC);
        testcase( idxNew==0 );
        اظہاریہ_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
      }ورنہ{
        sqlite3_اظہاریہ_List_Delete(db, pList);
      }
    }
  }
}
#اگر_ختم /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

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
ساکن صحیح رکن_Is_Equivalence(Parse *pParse, اظہاریہ *p_اظہاریہ, SrcList *pSrc){
  حرف aff1, aff2;
  اگر( !OptimizationEnabled(pParse->db, SQLITE_متعدی) ) واپس 0;  /* (1) */
  اگر( p_اظہاریہ->op!=TK_EQ && p_اظہاریہ->op!=TK_IS ) واپس 0;                 /* (2) */
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON|EP_Collate) ) واپس 0;        /* (3) */
  أكد( pSrc!=0 );
  اگر( p_اظہاریہ->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    واپس 0;                                                          /* (4) */
  }
  aff1 = sqlite3_اظہاریہ_Affinity(p_اظہاریہ->p_بایاں);
  aff2 = sqlite3_اظہاریہ_Affinity(p_اظہاریہ->p_دایاں);
  اگر( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    واپس 0;                                                          /* (5) */
  }
  اگر( !sqlite3_اظہاریہ_Coll_Seq_مطابقت(pParse, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں) ){
    واپس 0;                                                          /* (6) */
  }
  واپس 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ساکن Bitmask اظہاریہ_Select_Usage(Where_نقاب_سیٹ *p_نقاب_سیٹ, Select *pS){
  Bitmask نقاب = 0;
  جب( pS ){
    SrcList *pSrc = pS->pSrc;
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, pS->pEList);
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, pS->p_گروہ_By);
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, pS->p_ترتیب_By);
    نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, pS->pWhere);
    نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, pS->pHaving);
    اگر( ALWAYS(pSrc!=0) ){
      صحیح i;
      برائے(i=0; i<pSrc->nSrc; i++){
        اگر( pSrc->a[i].fg.isSubquery ){
          نقاب |= اظہاریہ_Select_Usage(p_نقاب_سیٹ, pSrc->a[i].u4.pSubq->pSelect);
        }
        اگر( pSrc->a[i].fg.isUsing==0 ){
          نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, pSrc->a[i].u3.pOn);
        }
        اگر( pSrc->a[i].fg.isTabFunc ){
          نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->p_پیشگی;
  }
  واپس نقاب;
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
ساکن SQLITE_NOINLINE صحیح اظہاریہ_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  صحیح *ai_Cur_ستون,         /* Write the referenced table cursor and column here */
  اظہاریہ *p_اظہاریہ,           /* An operand of a comparison operator */
  صحیح j                  /* Start looking with the j-th pFrom entry */
){
  اشاریہ *pIdx;
  صحیح i;
  صحیح iCur;
  کرو{
    iCur = pFrom->a[j].iCursor;
    برائے(pIdx=pFrom->a[j].pSTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
      اگر( pIdx->a_ستون_اظہاریہ==0 ) جاری;
      برائے(i=0; i<pIdx->n_کلید_ستون; i++){
        اگر( pIdx->ai_عمود[i]!=XN_اظہاریہ ) جاری;
        أكد( pIdx->b_Has_اظہاریہ );
        اگر( sqlite3_اظہاریہ_Compare_Skip(p_اظہاریہ,pIdx->a_ستون_اظہاریہ->a[i].p_اظہاریہ,iCur)==0
         && !sqlite3_اظہاریہ_Is_Constant(0,pIdx->a_ستون_اظہاریہ->a[i].p_اظہاریہ)
        ){
          ai_Cur_ستون[0] = iCur;
          ai_Cur_ستون[1] = XN_اظہاریہ;
          واپس 1;
        }
      }
    }
  }جب( ++j < pFrom->nSrc );
  واپس 0;
}
ساکن صحیح اظہاریہ_Might_Be_اشاریہ_دار(
  SrcList *pFrom,        /* The FROM clause */
  صحیح *ai_Cur_ستون,         /* Write the referenced table cursor & column here */
  اظہاریہ *p_اظہاریہ,           /* An operand of a comparison operator */
  صحیح op                 /* The specific comparison operator */
){
  صحیح i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  أكد( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  أكد( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_اندر<TK_GE );
  أكد( op<=TK_GE );
  اگر( p_اظہاریہ->op==TK_سمتیہ && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    p_اظہاریہ = p_اظہاریہ->x.pList->a[0].p_اظہاریہ;
  }

  اگر( p_اظہاریہ->op==TK_عمود ){
    ai_Cur_ستون[0] = p_اظہاریہ->i_جدول;
    ai_Cur_ستون[1] = p_اظہاریہ->i_عمود;
    واپس 1;
  }

  برائے(i=0; i<pFrom->nSrc; i++){
    اشاریہ *pIdx;
    برائے(pIdx=pFrom->a[i].pSTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
      اگر( pIdx->a_ستون_اظہاریہ ){
        واپس اظہاریہ_Might_Be_Indexed2(pFrom,ai_Cur_ستون,p_اظہاریہ,i);
      }
    }
  }
  واپس 0;
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
ساکن خالی اظہاریہ_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  صحیح idx_رکن               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  Where_رکن *p_رکن;                /* The term to be analyzed */
  Where_نقاب_سیٹ *p_نقاب_سیٹ;          /* Set of table index masks */
  اظہاریہ *p_اظہاریہ;                     /* The expression to be analyzed */
  Bitmask prereq_بایاں;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_دایاں = 0;          /* Extra dependencies on LEFT JOIN */
  اظہاریہ *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  صحیح isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  صحیح noCase = 0;                  /* uppercase equivalent to lowercase */
  صحیح op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  غیرعلامتی حرف eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  صحیح n_بایاں;                       /* Number of elements on left side vector */

  اگر( db->mallocFailed ){
    واپس;
  }
  أكد( pWC->n_رکن > idx_رکن );
  p_رکن = &pWC->a[idx_رکن];
#اگر_تعریف_شدہ SQLITE_DEBUG
  p_رکن->i_رکن = idx_رکن;
#اگر_ختم
  p_نقاب_سیٹ = &pWInfo->s_نقاب_سیٹ;
  p_اظہاریہ = p_رکن->p_اظہاریہ;
  أكد( p_اظہاریہ!=0 ); /* Because malloc() has not failed */
  أكد( p_اظہاریہ->op!=TK_AS && p_اظہاریہ->op!=TK_COLLATE );
اظہاریہ_Analyze_restart:
  p_نقاب_سیٹ->bVarSelect = 0;
  prereq_بایاں = sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p_اظہاریہ->p_بایاں);
  op = p_اظہاریہ->op;
  اگر( op==TK_اندر ){
    أكد( p_اظہاریہ->p_دایاں==0 );
    اگر( sqlite3_اظہاریہ_Check_اندر(pParse, p_اظہاریہ) ) واپس;
    اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
      p_رکن->prereq_دایاں = اظہاریہ_Select_Usage(p_نقاب_سیٹ, p_اظہاریہ->x.pSelect);
    }ورنہ{
      p_رکن->prereq_دایاں = sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, p_اظہاریہ->x.pList);
    }
    prereqAll = prereq_بایاں | p_رکن->prereq_دایاں;
  }ورنہ{
    p_رکن->prereq_دایاں = sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p_اظہاریہ->p_دایاں);
    اگر( p_اظہاریہ->p_بایاں==0
     || اظہاریہ_Has_Property(p_اظہاریہ, EP_xIsSelect|EP_If_Null_صف)
     || p_اظہاریہ->x.pList!=0
    ){
      prereqAll = sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ, p_اظہاریہ);
    }ورنہ{
      prereqAll = prereq_بایاں | p_رکن->prereq_دایاں;
    }
  }
  اگر( p_نقاب_سیٹ->bVarSelect ) p_رکن->wtFlags |= رکن_VARSELECT;

#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( prereqAll!=sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ, p_اظہاریہ) ){
    لکھو("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_درخت_View_اظہاریہ(0,p_اظہاریہ,0);
    أكد( 0 );
  }
#اگر_ختم

  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3_Where_Get_نقاب(p_نقاب_سیٹ, p_اظہاریہ->w.iJoin);
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ){
      prereqAll |= x;
      extra_دایاں = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }ورنہ اگر( (prereqAll>>1)>=x ){
      اظہاریہ_Clear_Property(p_اظہاریہ, EP_InnerON);
    }
  }
  p_رکن->prereqAll = prereqAll;
  p_رکن->بایاں_Cursor = -1;
  p_رکن->i_والد = -1;
  p_رکن->eOperator = 0;
  اگر( allowedOp(op) ){
    صحیح ai_Cur_ستون[2];
    اظہاریہ *p_بایاں = sqlite3_اظہاریہ_Skip_Collate(p_اظہاریہ->p_بایاں);
    اظہاریہ *p_دایاں = sqlite3_اظہاریہ_Skip_Collate(p_اظہاریہ->p_دایاں);
    u16 op_نقاب = (p_رکن->prereq_دایاں & prereq_بایاں)==0 ? WO_ALL : WO_EQUIV;

    اگر( p_رکن->u.x.i_میدان>0 ){
      أكد( op==TK_اندر );
      أكد( p_بایاں->op==TK_سمتیہ );
      أكد( اظہاریہ_Use_XList(p_بایاں) );
      p_بایاں = p_بایاں->x.pList->a[p_رکن->u.x.i_میدان-1].p_اظہاریہ;
    }

    اگر( اظہاریہ_Might_Be_اشاریہ_دار(pSrc, ai_Cur_ستون, p_بایاں, op) ){
      p_رکن->بایاں_Cursor = ai_Cur_ستون[0];
      أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
      p_رکن->u.x.بایاں_عمود = ai_Cur_ستون[1];
      p_رکن->eOperator = operator_نقاب(op) & op_نقاب;
    }
    اگر( op==TK_IS ) p_رکن->wtFlags |= رکن_IS;
    اگر( p_دایاں
     && اظہاریہ_Might_Be_اشاریہ_دار(pSrc, ai_Cur_ستون, p_دایاں, op)
     && !اظہاریہ_Has_Property(p_دایاں, EP_Fixed_ستون)
    ){
      Where_رکن *pNew;
      اظہاریہ *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      أكد( p_رکن->u.x.i_میدان==0 );
      اگر( p_رکن->بایاں_Cursor>=0 ){
        صحیح idxNew;
        pDup = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
        اگر( db->mallocFailed ){
          sqlite3_اظہاریہ_Delete(db, pDup);
          واپس;
        }
        idxNew = where_Clause_داخل_کرو(pWC, pDup, رکن_VIRTUAL|رکن_DYNAMIC);
        اگر( idxNew==0 ) واپس;
        pNew = &pWC->a[idxNew];
        mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
        اگر( op==TK_IS ) pNew->wtFlags |= رکن_IS;
        p_رکن = &pWC->a[idx_رکن];
        p_رکن->wtFlags |= رکن_COPIED;
        أكد( pWInfo->pTabList!=0 );
        اگر( رکن_Is_Equivalence(pParse, pDup, pWInfo->pTabList) ){
          p_رکن->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }ورنہ{
        pDup = p_اظہاریہ;
        pNew = p_رکن;
      }
      pNew->wtFlags |= اظہاریہ_Commute(pParse, pDup);
      pNew->بایاں_Cursor = ai_Cur_ستون[0];
      أكد( (p_رکن->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.بایاں_عمود = ai_Cur_ستون[1];
      testcase( (prereq_بایاں | extra_دایاں) != prereq_بایاں );
      pNew->prereq_دایاں = prereq_بایاں | extra_دایاں;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operator_نقاب(pDup->op) + eExtraOp) & op_نقاب;
    }ورنہ
    اگر( op==TK_ISNULL
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

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_BETWEEN_OPTIMIZATION
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
  ورنہ اگر( p_اظہاریہ->op==TK_BETWEEN && pWC->op==TK_AND ){
    اظہاریہ_List *pList;
    صحیح i;
    ساکن ثابت u8 ops[] = {TK_GE, TK_LE};
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    pList = p_اظہاریہ->x.pList;
    أكد( pList!=0 );
    أكد( pList->n_اظہاریہ==2 );
    أكد( pWC->a[idx_رکن].n_بچہ==0 );
    برائے(i=0; i<2; i++){
      اظہاریہ *p_New_اظہاریہ;
      صحیح idxNew;
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
#اگر_ختم /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#اگر !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  ورنہ اگر( p_اظہاریہ->op==TK_OR && !اظہاریہ_Has_Property(p_اظہاریہ, EP_Collate) ){
    اظہاریہ *pAlt = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ);
    اگر( pAlt!=p_اظہاریہ ){
      p_رکن->p_اظہاریہ = p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pAlt);
      جاؤ اظہاریہ_Analyze_restart;
    }
    أكد( pWC->op==TK_AND );
    اظہاریہ_Analyze_Or_رکن(pSrc, pWC, idx_رکن);
    p_رکن = &pWC->a[idx_رکن];
  }
#اگر_ختم /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  ورنہ اگر( p_اظہاریہ->op==TK_NOTNULL ){
    اگر( p_اظہاریہ->p_بایاں->op==TK_عمود
     && p_اظہاریہ->p_بایاں->i_عمود>=0
     && !اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON)
    ){
      اظہاریہ *p_New_اظہاریہ;
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      صحیح idxNew;
      Where_رکن *p_New_رکن;
 
      p_New_اظہاریہ = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_اظہاریہ_Dup(db, p_بایاں, 0),
                              sqlite3_اظہاریہ_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_داخل_کرو(pWC, p_New_اظہاریہ,
                                رکن_VIRTUAL|رکن_DYNAMIC|رکن_VNULL);
      اگر( idxNew ){
        p_New_رکن = &pWC->a[idxNew];
        p_New_رکن->prereq_دایاں = 0;
        p_New_رکن->بایاں_Cursor = p_بایاں->i_جدول;
        p_New_رکن->u.x.بایاں_عمود = p_بایاں->i_عمود;
        p_New_رکن->eOperator = WO_GT;
        mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
        p_رکن = &pWC->a[idx_رکن];
        p_رکن->wtFlags |= رکن_COPIED;
        p_New_رکن->prereqAll = p_رکن->prereqAll;
      }
    }
  }


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LIKE_OPTIMIZATION
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
  ورنہ اگر( p_اظہاریہ->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_اظہاریہ, &pStr1, &isComplete, &noCase)
  ){
    اظہاریہ *p_بایاں;       /* LHS of LIKE/GLOB operator */
    اظہاریہ *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    اظہاریہ *pNewExpr1;
    اظہاریہ *pNewExpr2;
    صحیح idxNew1;
    صحیح idxNew2;
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
    اگر( noCase && !pParse->db->mallocFailed ){
      صحیح i;
      حرف c;
      p_رکن->wtFlags |= رکن_LIKE;
      برائے(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    اگر( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      اگر( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        اگر( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3_بالائی_مقصد_زیریں[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      جب( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
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
    اگر( isComplete ){
      mark_رکن_As_بچہ(pWC, idxNew1, idx_رکن);
      mark_رکن_As_بچہ(pWC, idxNew2, idx_رکن);
    }
  }
#اگر_ختم /* SQLITE_OMIT_LIKE_OPTIMIZATION */

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
  اگر( (p_اظہاریہ->op==TK_EQ || p_اظہاریہ->op==TK_IS)
   && (n_بایاں = sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ->p_بایاں))>1
   && sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ->p_دایاں)==n_بایاں
   && ( (p_اظہاریہ->p_بایاں->flags & EP_xIsSelect)==0
     || (p_اظہاریہ->p_دایاں->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    صحیح i;
    برائے(i=0; i<n_بایاں; i++){
      صحیح idxNew;
      اظہاریہ *pNew;
      اظہاریہ *p_بایاں = sqlite3_اظہاریہ_For_سمتیہ_میدان(pParse, p_اظہاریہ->p_بایاں, i, n_بایاں);
      اظہاریہ *p_دایاں = sqlite3_اظہاریہ_For_سمتیہ_میدان(pParse, p_اظہاریہ->p_دایاں, i, n_بایاں);

      pNew = sqlite3PExpr(pParse, p_اظہاریہ->op, p_بایاں, p_دایاں);
      transferJoinMarkings(pNew, p_اظہاریہ);
      idxNew = where_Clause_داخل_کرو(pWC, pNew, رکن_DYNAMIC|رکن_قاش);
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
  ورنہ اگر( p_اظہاریہ->op==TK_اندر
   && p_رکن->u.x.i_میدان==0
   && p_اظہاریہ->p_بایاں->op==TK_سمتیہ
   && ALWAYS( اظہاریہ_Use_XSelect(p_اظہاریہ) )
   && (p_اظہاریہ->x.pSelect->p_پیشگی==0 || (p_اظہاریہ->x.pSelect->selFlags & SF_اقدار))
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
   && p_اظہاریہ->x.pSelect->pWin==0
#اگر_ختم
   && (p_اظہاریہ->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_اظہاریہ->x.pSelect->pEList->n_اظہاریہ <= UMXV(p_رکن->n_بچہ)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    صحیح i;
    أكد( p_رکن->n_بچہ==0 );
    برائے(i=0; i<sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ->p_بایاں); i++){
      صحیح idxNew;
      idxNew = where_Clause_داخل_کرو(pWC, p_اظہاریہ, رکن_VIRTUAL|رکن_قاش);
      pWC->a[idxNew].u.x.i_میدان = i+1;
      اظہاریہ_Analyze(pSrc, pWC, idxNew);
      mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
    }
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  /* Add a WO_AUX auxiliary term to the constraint set if the
  ** current expression is of the form "column OP expr" where OP
  ** is an operator that gets passed into virtual tables but which is
  ** not normally optimized for ordinary tables.  In other words, OP
  ** is one of MATCH, LIKE, GLOB, REGEXP, !=, IS, IS NOT, or NOT NULL.
  ** This information is used by the xBestIndex methods of
  ** virtual tables.  The native query optimizer does not attempt
  ** to do anything with MATCH functions.
  */
  ورنہ اگر( pWC->op==TK_AND ){
    اظہاریہ *p_دایاں = 0, *p_بایاں = 0;
    صحیح res = isAuxiliaryVtabOperator(db, p_اظہاریہ, &eOp2, &p_بایاں, &p_دایاں);
    جب( res-- > 0 ){
      صحیح idxNew;
      Where_رکن *p_New_رکن;
      Bitmask prereq_عمود, prereq_اظہاریہ;

      prereq_اظہاریہ = sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p_دایاں);
      prereq_عمود = sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p_بایاں);
      اگر( (prereq_اظہاریہ & prereq_عمود)==0 ){
        اظہاریہ *p_New_اظہاریہ;
        p_New_اظہاریہ = sqlite3PExpr(pParse, TK_مطابقت,
            0, sqlite3_اظہاریہ_Dup(db, p_دایاں, 0));
        اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) && p_New_اظہاریہ ){
          اظہاریہ_سیٹ_Property(p_New_اظہاریہ, EP_OuterON);
          p_New_اظہاریہ->w.iJoin = p_اظہاریہ->w.iJoin;
        }
        idxNew = where_Clause_داخل_کرو(pWC, p_New_اظہاریہ, رکن_VIRTUAL|رکن_DYNAMIC);
        testcase( idxNew==0 );
        p_New_رکن = &pWC->a[idxNew];
        p_New_رکن->prereq_دایاں = prereq_اظہاریہ | extra_دایاں;
        p_New_رکن->بایاں_Cursor = p_بایاں->i_جدول;
        p_New_رکن->u.x.بایاں_عمود = p_بایاں->i_عمود;
        p_New_رکن->eOperator = WO_AUX;
        p_New_رکن->e_مطابقت_Op = eOp2;
        mark_رکن_As_بچہ(pWC, idxNew, idx_رکن);
        p_رکن = &pWC->a[idx_رکن];
        p_رکن->wtFlags |= رکن_COPIED;
        p_New_رکن->prereqAll = p_رکن->prereqAll;
      }
      تبادلہ(اظہاریہ*, p_بایاں, p_دایاں);
    }
  }
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */

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
خالی sqlite3_Where_بانٹو(WhereClause *pWC, اظہاریہ *p_اظہاریہ, u8 op){
  اظہاریہ *pE2 = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ);
  pWC->op = op;
  أكد( pE2!=0 || p_اظہاریہ==0 );
  اگر( pE2==0 ) واپس;
  اگر( pE2->op!=op ){
    where_Clause_داخل_کرو(pWC, p_اظہاریہ, 0);
  }ورنہ{
    sqlite3_Where_بانٹو(pWC, pE2->p_بایاں, op);
    sqlite3_Where_بانٹو(pWC, pE2->p_دایاں, op);
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
ساکن خالی where_جوڑو_حد_بندی_اظہاریہ(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  صحیح iReg,           /* Register that will hold value of the limit/offset */
  اظہاریہ *p_اظہاریہ,        /* Expression that defines the limit/offset */
  صحیح iCsr,           /* Cursor to which the constraint applies */
  صحیح e_مطابقت_Op        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  اظہاریہ *pNew;
  صحیح iVal = 0;

  اگر( sqlite3_اظہاریہ_Is_Integer(p_اظہاریہ, &iVal, pParse, 0) && iVal>=0 ){
    اظہاریہ *pVal = sqlite3_اظہاریہ_Int32(db, iVal);
    اگر( pVal==0 ) واپس;
    pNew = sqlite3PExpr(pParse, TK_مطابقت, 0, pVal);
  }ورنہ{
    اظہاریہ *pVal = sqlite3_اظہاریہ_Alloc(db, TK_رجسٹر, 0, 0);
    اگر( pVal==0 ) واپس;
    pVal->i_جدول = iReg;
    pNew = sqlite3PExpr(pParse, TK_مطابقت, 0, pVal);
  }
  اگر( pNew ){
    Where_رکن *p_رکن;
    صحیح idx;
    idx = where_Clause_داخل_کرو(pWC, pNew, رکن_DYNAMIC|رکن_VIRTUAL);
    p_رکن = &pWC->a[idx];
    p_رکن->بایاں_Cursor = iCsr;
    p_رکن->eOperator = WO_AUX;
    p_رکن->e_مطابقت_Op = e_مطابقت_Op;
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
خالی SQLITE_NOINLINE sqlite3_Where_جوڑو_حد_بندی(WhereClause *pWC, Select *p){
  أكد( p!=0 && p->p_حد_بندی!=0 );                 /* 1 -- checked by caller */
  اگر( p->p_گروہ_By==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    اظہاریہ_List *p_ترتیب_By = p->p_ترتیب_By;
    صحیح iCsr = p->pSrc->a[0].iCursor;
    صحیح ii;

    /* Check condition (4). Return early if it is not met. */
    برائے(ii=0; ii<pWC->n_رکن; ii++){
      اگر( pWC->a[ii].wtFlags & رکن_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        أكد( pWC->a[ii].wtFlags & رکن_VIRTUAL );
        أكد( pWC->a[ii].eOperator==WO_ROWVAL );
        جاری;
      }
      اگر( pWC->a[ii].n_بچہ ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        جاری;
      }
      اگر( pWC->a[ii].بایاں_Cursor==iCsr && pWC->a[ii].prereq_دایاں==0 ) جاری;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      اگر( pWC->a[ii].i_والد>=0 ){
        Where_رکن *p_والد = &pWC->a[ pWC->a[ii].i_والد ];
        اگر( p_والد->بایاں_Cursor==iCsr
         && p_والد->prereq_دایاں==0
         && p_والد->n_بچہ==1
        ){
          جاری;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      واپس;
    }

    /* Check condition (5). Return early if it is not met. */
    اگر( p_ترتیب_By ){
      برائے(ii=0; ii<p_ترتیب_By->n_اظہاریہ; ii++){
        اظہاریہ *p_اظہاریہ = p_ترتیب_By->a[ii].p_اظہاریہ;
        اگر( p_اظہاریہ->op!=TK_عمود ) واپس;
        اگر( p_اظہاریہ->i_جدول!=iCsr ) واپس;
        اگر( p_ترتیب_By->a[ii].fg.ترتیب_دو_Flags & KEYINFO_ترتیب_BIGNULL ) واپس;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    أكد( p->p_حد_بندی->op==TK_حد_بندی );
    اگر( p->i_انحراف!=0 && (p->selFlags & SF_Compound)==0 ){
      where_جوڑو_حد_بندی_اظہاریہ(pWC, p->i_انحراف, p->p_حد_بندی->p_دایاں,
                        iCsr, SQLITE_اشاریہ_CONSTRAINT_انحراف);
    }
    اگر( p->i_انحراف==0 || (p->selFlags & SF_Compound)==0 ){
      where_جوڑو_حد_بندی_اظہاریہ(pWC, p->i_حد_بندی, p->p_حد_بندی->p_بایاں,
                        iCsr, SQLITE_اشاریہ_CONSTRAINT_حد_بندی);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
خالی sqlite3WhereClauseInit(
  WhereClause *pWC,        /* The WhereClause to be initialized */
  WhereInfo *pWInfo        /* The WHERE processing context */
){
  pWC->pWInfo = pWInfo;
  pWC->hasOr = 0;
  pWC->pOuter = 0;
  pWC->n_رکن = 0;
  pWC->n_بنیاد = 0;
  pWC->n_خانہ = Array_سائز(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
خالی sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  أكد( pWC->n_رکن>=pWC->n_بنیاد );
  اگر( pWC->n_رکن>0 ){
    Where_رکن *a = pWC->a;
    Where_رکن *a_آخری = &pWC->a[pWC->n_رکن-1];
#اگر_تعریف_شدہ SQLITE_DEBUG
    صحیح i;
    /* Verify that every term past pWC->nBase is virtual */
    برائے(i=pWC->n_بنیاد; i<pWC->n_رکن; i++){
      أكد( (pWC->a[i].wtFlags & رکن_VIRTUAL)!=0 );
    }
#اگر_ختم
    جب(1){
      أكد( a->e_مطابقت_Op==0 || a->eOperator==WO_AUX );
      اگر( a->wtFlags & رکن_DYNAMIC ){
        sqlite3_اظہاریہ_Delete(db, a->p_اظہاریہ);
      }
      اگر( a->wtFlags & (رکن_ORINFO|رکن_ANDINFO) ){
        اگر( a->wtFlags & رکن_ORINFO ){
          أكد( (a->wtFlags & رکن_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }ورنہ{
          أكد( (a->wtFlags & رکن_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      اگر( a==a_آخری ) توڑو;
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
ساکن SQLITE_NOINLINE Bitmask sqlite3_Where_اظہاریہ_Usage_بھرا(
  Where_نقاب_سیٹ *p_نقاب_سیٹ,
  اظہاریہ *p
){
  Bitmask نقاب;
  نقاب = (p->op==TK_IF_NULL_صف) ? sqlite3_Where_Get_نقاب(p_نقاب_سیٹ, p->i_جدول) : 0;
  اگر( p->p_بایاں ) نقاب |= sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ, p->p_بایاں);
  اگر( p->p_دایاں ){
    نقاب |= sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ, p->p_دایاں);
    أكد( p->x.pList==0 );
  }ورنہ اگر( اظہاریہ_Use_XSelect(p) ){
    اگر( اظہاریہ_Has_Property(p, EP_VarSelect) ) p_نقاب_سیٹ->bVarSelect = 1;
    نقاب |= اظہاریہ_Select_Usage(p_نقاب_سیٹ, p->x.pSelect);
  }ورنہ اگر( p->x.pList ){
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, p->x.pList);
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  اگر( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && اظہاریہ_Use_YWin(p) ){
    أكد( p->y.pWin!=0 );
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, p->y.pWin->p_تقسیم);
    نقاب |= sqlite3_Where_اظہاریہ_List_Usage(p_نقاب_سیٹ, p->y.pWin->p_ترتیب_By);
    نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, p->y.pWin->pFilter);
  }
#اگر_ختم
  واپس نقاب;
}
Bitmask sqlite3_Where_اظہاریہ_Usage_NN(Where_نقاب_سیٹ *p_نقاب_سیٹ, اظہاریہ *p){
  اگر( p->op==TK_عمود && !اظہاریہ_Has_Property(p, EP_Fixed_ستون) ){
    واپس sqlite3_Where_Get_نقاب(p_نقاب_سیٹ, p->i_جدول);
  }ورنہ اگر( اظہاریہ_Has_Property(p, EP_TokenOnly|EP_برگ) ){
    أكد( p->op!=TK_IF_NULL_صف );
    واپس 0;
  }
  واپس sqlite3_Where_اظہاریہ_Usage_بھرا(p_نقاب_سیٹ, p);
}
Bitmask sqlite3_Where_اظہاریہ_Usage(Where_نقاب_سیٹ *p_نقاب_سیٹ, اظہاریہ *p){
  واپس p ? sqlite3_Where_اظہاریہ_Usage_NN(p_نقاب_سیٹ,p) : 0;
}
Bitmask sqlite3_Where_اظہاریہ_List_Usage(Where_نقاب_سیٹ *p_نقاب_سیٹ, اظہاریہ_List *pList){
  صحیح i;
  Bitmask نقاب = 0;
  اگر( pList ){
    برائے(i=0; i<pList->n_اظہاریہ; i++){
      نقاب |= sqlite3_Where_اظہاریہ_Usage(p_نقاب_سیٹ, pList->a[i].p_اظہاریہ);
    }
  }
  واپس نقاب;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
خالی sqlite3_Where_اظہاریہ_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  صحیح i;
  برائے(i=pWC->n_رکن-1; i>=0; i--){
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
خالی sqlite3_Where_Tab_Func_دلائل(
  Parse *pParse,                    /* Parsing context */
  Src_شے *p_شے,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  جدول *pTab;
  صحیح j, k;
  اظہاریہ_List *p_دلائل;
  اظہاریہ *p_ستون_Ref;
  اظہاریہ *p_رکن;
  اگر( p_شے->fg.isTabFunc==0 ) واپس;
  pTab = p_شے->pSTab;
  أكد( pTab!=0 );
  اگر( !IsVirtual(pTab) ){
    sqlite3_Error_پیغام(pParse, "'%s' is not a function", p_شے->zName);
    واپس;
  }
  p_دلائل = p_شے->u1.pFuncArg;
  اگر( p_دلائل==0 ) واپس;
  برائے(j=k=0; j<p_دلائل->n_اظہاریہ; j++){
    اظہاریہ *pRhs;
    u32 joinType;
    جب( k<pTab->n_ستون && (pTab->a_ستون[k].ستون_Flags & COLFLAG_HIDDEN)==0 ){k++;}
    اگر( k>=pTab->n_ستون ){
      sqlite3_Error_پیغام(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      واپس;
    }
    p_ستون_Ref = sqlite3_اظہاریہ_Alloc(pParse->db, TK_عمود, 0, 0);
    اگر( p_ستون_Ref==0 ) واپس;
    p_ستون_Ref->i_جدول = p_شے->iCursor;
    p_ستون_Ref->i_عمود = k++;
    أكد( اظہاریہ_Use_YTab(p_ستون_Ref) );
    p_ستون_Ref->y.pTab = pTab;
    p_شے->ستون_مستعمل |= sqlite3_اظہاریہ_ستون_مستعمل(p_ستون_Ref);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_اظہاریہ_Dup(pParse->db, p_دلائل->a[j].p_اظہاریہ, 0), 0);
    p_رکن = sqlite3PExpr(pParse, TK_EQ, p_ستون_Ref, pRhs);
    اگر( p_شے->fg.jointype & (JT_بایاں|JT_دایاں) ){
      testcase( p_شے->fg.jointype & JT_بایاں );  /* testtag-20230227a */
      testcase( p_شے->fg.jointype & JT_دایاں ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }ورنہ{
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
صحیح sqlite3WhereLoopBloomable(ثابت WhereLoop *pLoop){
  اگر( pLoop->wsFlags & WHERE_IPK ) واپس 1;
  اگر( NEVER((pLoop->wsFlags & WHERE_اشاریہ_دار)==0) ) واپس 0;
  واپس sqlite3_اشاریہ_Bloomable(pLoop->u.btree.p_اشاریہ, pLoop->u.btree.nEq);
}
