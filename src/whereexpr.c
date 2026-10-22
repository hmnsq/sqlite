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
ساكن فراغ اظهار_Analyze(SrcList*, WhereClause*, صحيح);

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
ساكن صحيح where_Clause_وجهو(WhereClause *pWC, اظهار *p, u16 wtFlags){
  Where_رڪن *p_رڪن;
  صحيح idx;
  testcase( wtFlags & رڪن_VIRTUAL );
  إذا( pWC->n_رڪن>=pWC->n_جڳهه ){
    Where_رڪن *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, حجم(pWC->a[0])*pWC->n_جڳهه*2 );
    إذا( pWC->a==0 ){
      إذا( wtFlags & رڪن_DYNAMIC ){
        sqlite3_اظهار_Delete(db, p);
      }
      pWC->a = pOld;
      أرجع 0;
    }
    يادگيري_نقل(pWC->a, pOld, حجم(pWC->a[0])*pWC->n_رڪن);
    pWC->n_جڳهه = pWC->n_جڳهه*2;
  }
  p_رڪن = &pWC->a[idx = pWC->n_رڪن++];
  إذا( (wtFlags & رڪن_VIRTUAL)==0 ) pWC->n_بنياد = pWC->n_رڪن;
  إذا( p && اظهار_Has_Property(p, EP_Unlikely) ){
    p_رڪن->truthProb = sqlite3LogEst(p->i_جدول) - 270;
  }وإلا{
    p_رڪن->truthProb = 1;
  }
  p_رڪن->p_اظهار = sqlite3_اظهار_Skip_Collate_And_Likely(p);
  p_رڪن->wtFlags = wtFlags;
  p_رڪن->pWC = pWC;
  p_رڪن->i_والد = -1;
  memset(&p_رڪن->eOperator, 0,
         حجم(Where_رڪن) - offsetof(Where_رڪن,eOperator));
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
  أكد( TK_اندريون<TK_EQ );
  أكد( TK_IS<TK_EQ );
  أكد( TK_ISNULL<TK_EQ );
  إذا( op>TK_GE ) أرجع 0;
  إذا( op>=TK_EQ ) أرجع 1;
  أرجع op==TK_اندريون || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ساكن u16 اظهار_Commute(Parse *pParse, اظهار *p_اظهار){
  إذا( p_اظهار->p_کاٻو->op==TK_ويڪٽر
   || p_اظهار->p_ساڄو->op==TK_ويڪٽر
   || sqlite3_ٻٽو_Compare_Coll_Seq(pParse, p_اظهار->p_کاٻو, p_اظهار->p_ساڄو) !=
      sqlite3_ٻٽو_Compare_Coll_Seq(pParse, p_اظهار->p_ساڄو, p_اظهار->p_کاٻو)
  ){
    p_اظهار->flags ^= EP_Commuted;
  }
  مٽا_سٽا(اظهار*,p_اظهار->p_ساڄو,p_اظهار->p_کاٻو);
  إذا( p_اظهار->op>=TK_GT ){
    أكد( TK_LT==TK_GT+2 );
    أكد( TK_GE==TK_LE+2 );
    أكد( TK_GT>TK_EQ );
    أكد( TK_GT<TK_LE );
    أكد( p_اظهار->op>=TK_GT && p_اظهار->op<=TK_GE );
    p_اظهار->op = ((p_اظهار->op-TK_GT)^2)+TK_GT;
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
  }وإلا إذا( op==TK_اندريون ){
    c = WO_اندريون;
  }وإلا إذا( op==TK_ISNULL ){
    c = WO_ISNULL;
  }وإلا{
    أكد( op==TK_IS );
    c = WO_IS;
  }
  أكد( op!=TK_ISNULL || c==WO_ISNULL );
  أكد( op!=TK_اندريون || c==WO_اندريون );
  أكد( op!=TK_EQ || c==WO_EQ );
  أكد( op!=TK_LT || c==WO_LT );
  أكد( op!=TK_LE || c==WO_LE );
  أكد( op!=TK_GT || c==WO_GT );
  أكد( op!=TK_GE || c==WO_GE );
  أكد( op!=TK_IS || c==WO_IS );
  أرجع c;
}


#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_LIKE_OPTIMIZATION
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
  اظهار *p_اظهار,      /* Test this expression */
  اظهار **pp_اڳياڙي,  /* Pointer to TK_STRING expression with pattern prefix */
  صحيح *pisComplete, /* True if the only wildcard is % in the last character */
  صحيح *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ثابت u8 *z = 0;           /* String on RHS of LIKE operator */
  اظهار *p_ساڄو, *p_کاٻو;      /* Right and left size of LIKE operator */
  اظهار_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  صحيح cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_قدر *pVal = 0;
  صحيح op;                    /* Opcode of pRight */
  صحيح rc;                    /* Result code to return */

  إذا( !sqlite3IsLikeFunction(db, p_اظهار, pnoCase, (حرف*)wc) ){
    أرجع 0;
  }
#جيڪڏهن_وصف_ٿيل SQLITE_EBCDIC
  إذا( *pnoCase ) أرجع 0;
#جيڪڏهن_پورو
  أكد( اظهار_Use_XList(p_اظهار) );
  pList = p_اظهار->x.pList;
  p_کاٻو = pList->a[1].p_اظهار;

  p_ساڄو = sqlite3_اظهار_Skip_Collate(pList->a[0].p_اظهار);
  op = p_ساڄو->op;
  إذا( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    صحيح i_کانو = p_ساڄو->i_ڪالم;
    pVal = sqlite3_Vdbe_Get_Bound_قدر(pReprepare, i_کانو, SQLITE_AFF_BLOB);
    إذا( pVal && sqlite3_قدر_type(pVal)==SQLITE_متن ){
      z = sqlite3_قدر_متن(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, i_کانو, 0);
    أكد( p_ساڄو->op==TK_VARIABLE || p_ساڄو->op==TK_رجسٽر );
  }وإلا إذا( op==TK_STRING ){
    أكد( !اظهار_Has_Property(p_ساڄو, EP_Int_قدر) );
     z = (u8*)p_ساڄو->u.zToken;
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
      اظهار *p_اڳياڙي;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      p_اڳياڙي = sqlite3_اظهار(db, TK_STRING, (حرف*)z);
      إذا( p_اڳياڙي ){
        صحيح iFrom, i_منزل;
        حرف *zNew;
        أكد( !اظهار_Has_Property(p_اڳياڙي, EP_Int_قدر) );
        zNew = p_اڳياڙي->u.zToken;
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
        إذا( p_کاٻو->op!=TK_ڪالم
         || sqlite3_اظهار_Affinity(p_کاٻو)!=SQLITE_AFF_متن
         || (ALWAYS( اظهار_Use_YTab(p_کاٻو) )
             && ALWAYS(p_کاٻو->y.pTab)
             && IsVirtual(p_کاٻو->y.pTab))  /* Might be numeric */
        ){
          صحيح is_انگ;
          مزدوج rDummy;
          أكد( zNew[i_منزل]==0 );
          is_انگ = sqlite3AtoF(zNew, &rDummy);
          إذا( is_انگ<=0 ){
            إذا( i_منزل==1 && zNew[0]=='-' ){
              is_انگ = +1;
            }وإلا{
              zNew[i_منزل-1]++;
              is_انگ = sqlite3AtoF(zNew, &rDummy);
              zNew[i_منزل-1]--;
            }
          }
          إذا( is_انگ>0 ){
            sqlite3_اظهار_Delete(db, p_اڳياڙي);
            sqlite3_قدر_Free(pVal);
            أرجع 0;
          }
        }
      }
      *pp_اڳياڙي = p_اڳياڙي;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      إذا( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_ساڄو->i_ڪالم, 0);
        أكد( !اظهار_Has_Property(p_ساڄو, EP_Int_قدر) );
        إذا( *pisComplete && p_ساڄو->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          صحيح r1 = sqlite3_Get_عارضي_Reg(pParse);
          sqlite3_اظهار_ڪوڊ_هدف(pParse, p_ساڄو, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3_Release_عارضي_Reg(pParse, r1);
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
#جيڪڏهن_پورو /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
صحيح sqlite3_اظهار_Is_Like_Operator(ثابت اظهار *p_اظهار){
  ساكن ثابت بنية {
    ثابت حرف *zOp;
    بنانشان حرف eOp;
  } aOp[] = {
    { "match",  SQLITE_اشاريو_CONSTRAINT_ميلاپ },
    { "glob",   SQLITE_اشاريو_CONSTRAINT_GLOB },
    { "like",   SQLITE_اشاريو_CONSTRAINT_LIKE },
    { "regexp", SQLITE_اشاريو_CONSTRAINT_REGEXP }
  };
  صحيح i;
  أكد( p_اظهار->op==TK_FUNCTION );
  أكد( !اظهار_Has_Property(p_اظهار, EP_Int_قدر) );
  لكل(i=0; i<Array_سائيز(aOp); i++){
    إذا( sqlite3StrICmp(p_اظهار->u.zToken, aOp[i].zOp)==0 ){
      أرجع aOp[i].eOp;
    }
  }
  أرجع 0;
}


#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
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
  اظهار *p_اظهار,                    /* Test this expression */
  بنانشان حرف *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  اظهار **pp_کاٻو,                  /* Column expression to left of MATCH/op2 */
  اظهار **pp_ساڄو                  /* Expression to left of MATCH/op2 */
){
  إذا( p_اظهار->op==TK_FUNCTION ){
    اظهار_List *pList;
    اظهار *p_کانو;                     /* Column reference */
    صحيح i;

    أكد( اظهار_Use_XList(p_اظهار) );
    pList = p_اظهار->x.pList;
    إذا( pList==0 || pList->n_اظهار!=2 ){
      أرجع 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    p_کانو = pList->a[1].p_اظهار;
    أكد( p_کانو->op!=TK_ڪالم || (اظهار_Use_YTab(p_کانو) && p_کانو->y.pTab!=0) );
    إذا( اظهار_Is_Vtab(p_کانو) && (i = sqlite3_اظهار_Is_Like_Operator(p_اظهار))!=0 ){
      *peOp2 = i;
      *pp_ساڄو = pList->a[0].p_اظهار;
      *pp_کاٻو = p_کانو;
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
    p_کانو = pList->a[0].p_اظهار;
    أكد( p_کانو->op!=TK_ڪالم || اظهار_Use_YTab(p_کانو) );
    أكد( p_کانو->op!=TK_ڪالم || (اظهار_Use_YTab(p_کانو) && p_کانو->y.pTab!=0) );
    إذا( اظهار_Is_Vtab(p_کانو) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      فراغ (*x_Not_استعمال_ٿيل)(sqlite3_context*,صحيح,sqlite3_قدر**);
      فراغ *p_Not_استعمال_ٿيل;
      pVtab = sqlite3GetVTable(db, p_کانو->y.pTab)->pVtab;
      أكد( pVtab!=0 );
      أكد( pVtab->pModule!=0 );
      أكد( !اظهار_Has_Property(p_اظهار, EP_Int_قدر) );
      pMod = (sqlite3_module *)pVtab->pModule;
      إذا( pMod->x_ڳوليو_Function!=0 ){
        i = pMod->x_ڳوليو_Function(pVtab,2, p_اظهار->u.zToken, &x_Not_استعمال_ٿيل, &p_Not_استعمال_ٿيل);
        إذا( i>=SQLITE_اشاريو_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_ساڄو = pList->a[1].p_اظهار;
          *pp_کاٻو = p_کانو;
          أرجع 1;
        }
      }
    }
  }وإلا إذا( p_اظهار->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    أكد( TK_NE < TK_EQ );
    أكد( TK_ISNOT < TK_EQ );
    أكد( TK_NOTNULL < TK_EQ );
    أرجع 0;
  }وإلا إذا( p_اظهار->op==TK_NE || p_اظهار->op==TK_ISNOT || p_اظهار->op==TK_NOTNULL ){
    صحيح res = 0;
    اظهار *p_کاٻو = p_اظهار->p_کاٻو;
    اظهار *p_ساڄو = p_اظهار->p_ساڄو;
    أكد( p_کاٻو->op!=TK_ڪالم || (اظهار_Use_YTab(p_کاٻو) && p_کاٻو->y.pTab!=0) );
    إذا( اظهار_Is_Vtab(p_کاٻو) ){
      res++;
    }
    أكد( p_ساڄو==0 || p_ساڄو->op!=TK_ڪالم
            || (اظهار_Use_YTab(p_ساڄو) && p_ساڄو->y.pTab!=0) );
    إذا( p_ساڄو && اظهار_Is_Vtab(p_ساڄو) ){
      res++;
      مٽا_سٽا(اظهار*, p_کاٻو, p_ساڄو);
    }
    *pp_کاٻو = p_کاٻو;
    *pp_ساڄو = p_ساڄو;
    إذا( p_اظهار->op==TK_NE ) *peOp2 = SQLITE_اشاريو_CONSTRAINT_NE;
    إذا( p_اظهار->op==TK_ISNOT ) *peOp2 = SQLITE_اشاريو_CONSTRAINT_ISNOT;
    إذا( p_اظهار->op==TK_NOTNULL ) *peOp2 = SQLITE_اشاريو_CONSTRAINT_ISNOTNULL;
    أرجع res;
  }
  أرجع 0;
}
#جيڪڏهن_پورو /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ساكن فراغ transferJoinMarkings(اظهار *pDerived, اظهار *p_بنياد){
  إذا( pDerived && اظهار_Has_Property(p_بنياد, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= p_بنياد->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = p_بنياد->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ساكن فراغ mark_رڪن_As_ٻار(WhereClause *pWC, صحيح i_ٻار, صحيح i_والد){
  pWC->a[i_ٻار].i_والد = i_والد;
  pWC->a[i_ٻار].truthProb = pWC->a[i_والد].truthProb;
  أكد(   pWC->a[i_والد].n_ٻار <  UMXV(pWC->a[0].n_ٻار) );
  pWC->a[i_والد].n_ٻار++;
  testcase( pWC->a[i_والد].n_ٻار == UMXV(pWC->a[0].n_ٻار) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ساكن Where_رڪن *whereNthSubterm(Where_رڪن *p_رڪن, صحيح N){
  إذا( p_رڪن->eOperator!=WO_AND ){
    أرجع N==0 ? p_رڪن : 0;
  }
  إذا( N<p_رڪن->u.pAndInfo->wc.n_رڪن ){
    أرجع &p_رڪن->u.pAndInfo->wc.a[N];
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
  Where_رڪن *pOne,       /* First disjunct */
  Where_رڪن *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  اظهار *pNew;            /* New virtual expression */
  صحيح op;                /* Operator for the combined expression */
  صحيح idxNew;            /* Index in pWC of the next virtual term */
  اظهار *pA, *pB;         /* Expressions associated with pOne and pTwo */

  إذا( (pOne->wtFlags | pTwo->wtFlags) & رڪن_VNULL ) أرجع;
  إذا( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) أرجع;
  إذا( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) أرجع;
  إذا( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) أرجع;
  pA = pOne->p_اظهار;
  pB = pTwo->p_اظهار;
  أكد( pA->p_کاٻو!=0 && pA->p_ساڄو!=0 );
  أكد( pB->p_کاٻو!=0 && pB->p_ساڄو!=0 );
  إذا( sqlite3_اظهار_Compare(0,pA->p_کاٻو, pB->p_کاٻو, -1) )  أرجع;
  إذا( sqlite3_اظهار_Compare(0,pA->p_ساڄو, pB->p_ساڄو,-1) ) أرجع;
  إذا( اظهار_Has_Property(pA,EP_Commuted)!=اظهار_Has_Property(pB,EP_Commuted) ){
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
  pNew = sqlite3_اظهار_Dup(db, pA, 0);
  إذا( pNew==0 ) أرجع;
  لكل(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ أكد( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_وجهو(pWC, pNew, رڪن_VIRTUAL|رڪن_DYNAMIC);
  اظهار_Analyze(pSrc, pWC, idxNew);
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
ساكن فراغ اظهار_Analyze_Or_رڪن(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  صحيح idx_رڪن               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  Where_رڪن *p_رڪن = &pWC->a[idx_رڪن];    /* The term to be analyzed */
  اظهار *p_اظهار = p_رڪن->p_اظهار;             /* The expression of the term */
  صحيح i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  Where_رڪن *p_Or_رڪن;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chng_منزل_اندريون;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  أكد( (p_رڪن->wtFlags & (رڪن_DYNAMIC|رڪن_ORINFO|رڪن_ANDINFO))==0 );
  أكد( p_اظهار->op==TK_OR );
  p_رڪن->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, حجم(*pOrInfo));
  إذا( pOrInfo==0 ) أرجع;
  p_رڪن->wtFlags |= رڪن_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, حجم(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3_Where_ورهايو(pOrWc, p_اظهار, TK_OR);
  sqlite3_Where_اظهار_Analyze(pSrc, pOrWc);
  إذا( db->mallocFailed ) أرجع;
  أكد( pOrWc->n_رڪن>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chng_منزل_اندريون = ~(Bitmask)0;
  لكل(i=pOrWc->n_رڪن-1, p_Or_رڪن=pOrWc->a; i>=0 && indexable; i--, p_Or_رڪن++){
    إذا( (p_Or_رڪن->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      أكد( (p_Or_رڪن->wtFlags & (رڪن_ANDINFO|رڪن_ORINFO))==0 );
      chng_منزل_اندريون = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, حجم(*pAndInfo));
      إذا( pAndInfo ){
        WhereClause *pAndWC;
        Where_رڪن *p_And_رڪن;
        صحيح j;
        Bitmask b = 0;
        p_Or_رڪن->u.pAndInfo = pAndInfo;
        p_Or_رڪن->wtFlags |= رڪن_ANDINFO;
        p_Or_رڪن->eOperator = WO_AND;
        p_Or_رڪن->کاٻو_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, حجم(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3_Where_ورهايو(pAndWC, p_Or_رڪن->p_اظهار, TK_AND);
        sqlite3_Where_اظهار_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        إذا( !db->mallocFailed ){
          لكل(j=0, p_And_رڪن=pAndWC->a; j<pAndWC->n_رڪن; j++, p_And_رڪن++){
            أكد( p_And_رڪن->p_اظهار );
            إذا( allowedOp(p_And_رڪن->p_اظهار->op)
             || p_And_رڪن->eOperator==WO_AUX
            ){
              b |= sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سيٽ, p_And_رڪن->کاٻو_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }وإلا إذا( p_Or_رڪن->wtFlags & رڪن_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }وإلا{
      Bitmask b;
      b = sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سيٽ, p_Or_رڪن->کاٻو_Cursor);
      إذا( p_Or_رڪن->wtFlags & رڪن_VIRTUAL ){
        Where_رڪن *pOther = &pOrWc->a[p_Or_رڪن->i_والد];
        b |= sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سيٽ, pOther->کاٻو_Cursor);
      }
      indexable &= b;
      إذا( (p_Or_رڪن->eOperator & WO_EQ)==0 ){
        chng_منزل_اندريون = 0;
      }وإلا{
        chng_منزل_اندريون &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  p_رڪن->eOperator = WO_OR;
  p_رڪن->کاٻو_Cursor = -1;
  إذا( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  إذا( indexable && pOrWc->n_رڪن==2 ){
    صحيح iOne = 0;
    Where_رڪن *pOne;
    طالما( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      صحيح iTwo = 0;
      Where_رڪن *pTwo;
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
  إذا( chng_منزل_اندريون ){
    صحيح ok_منزل_Chng_منزل_اندريون = 0;     /* True if the conversion to IN is valid */
    صحيح i_ڪالم = -1;         /* Column index on lhs of IN operator */
    صحيح iCursor = -1;         /* Table cursor common to all terms */
    صحيح j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    لكل(j=0; j<2 && !ok_منزل_Chng_منزل_اندريون; j++){
      اظهار *p_کاٻو = 0;
      p_Or_رڪن = pOrWc->a;
      لكل(i=pOrWc->n_رڪن-1; i>=0; i--, p_Or_رڪن++){
        أكد( p_Or_رڪن->eOperator & WO_EQ );
        p_Or_رڪن->wtFlags &= ~رڪن_OK;
        إذا( p_Or_رڪن->کاٻو_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          أكد( j==1 );
          استمر;
        }
        إذا( (chng_منزل_اندريون & sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سيٽ,
                                            p_Or_رڪن->کاٻو_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( p_Or_رڪن->wtFlags & رڪن_COPIED );
          testcase( p_Or_رڪن->wtFlags & رڪن_VIRTUAL );
          أكد( p_Or_رڪن->wtFlags & (رڪن_COPIED|رڪن_VIRTUAL) );
          استمر;
        }
        أكد( (p_Or_رڪن->eOperator & (WO_OR|WO_AND))==0 );
        i_ڪالم = p_Or_رڪن->u.x.کاٻو_ڪالم;
        iCursor = p_Or_رڪن->کاٻو_Cursor;
        p_کاٻو = p_Or_رڪن->p_اظهار->p_کاٻو;
        اكسر;
      }
      إذا( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        أكد( j==1 );
        أكد( Is_Power_جو_Two(chng_منزل_اندريون) );
        أكد( chng_منزل_اندريون==sqlite3_Where_Get_نقاب(&pWInfo->s_نقاب_سيٽ, iCursor) );
        اكسر;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      ok_منزل_Chng_منزل_اندريون = 1;
      لكل(; i>=0 && ok_منزل_Chng_منزل_اندريون; i--, p_Or_رڪن++){
        أكد( p_Or_رڪن->eOperator & WO_EQ );
        أكد( (p_Or_رڪن->eOperator & (WO_OR|WO_AND))==0 );
        إذا( p_Or_رڪن->کاٻو_Cursor!=iCursor ){
          p_Or_رڪن->wtFlags &= ~رڪن_OK;
        }وإلا إذا( p_Or_رڪن->u.x.کاٻو_ڪالم!=i_ڪالم || (i_ڪالم==XN_اظهار
               && sqlite3_اظهار_Compare(pParse, p_Or_رڪن->p_اظهار->p_کاٻو, p_کاٻو, -1)
        )){
          ok_منزل_Chng_منزل_اندريون = 0;
        }وإلا{
          صحيح aff_کاٻو, aff_ساڄو;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_ساڄو = sqlite3_اظهار_Affinity(p_Or_رڪن->p_اظهار->p_ساڄو);
          aff_کاٻو = sqlite3_اظهار_Affinity(p_Or_رڪن->p_اظهار->p_کاٻو);
          إذا( aff_ساڄو!=0 && aff_ساڄو!=aff_کاٻو ){
            ok_منزل_Chng_منزل_اندريون = 0;
          }وإلا{
            p_Or_رڪن->wtFlags |= رڪن_OK;
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
    إذا( ok_منزل_Chng_منزل_اندريون ){
      اظهار *pDup;            /* A transient duplicate expression */
      اظهار_List *pList = 0;   /* The RHS of the IN operator */
      اظهار *p_کاٻو = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      اظهار *pNew;            /* The complete IN operator */

      لكل(i=pOrWc->n_رڪن-1, p_Or_رڪن=pOrWc->a; i>=0; i--, p_Or_رڪن++){
        اظهار *pThis;
        إذا( (p_Or_رڪن->wtFlags & رڪن_OK)==0 ) استمر;
        أكد( p_Or_رڪن->eOperator & WO_EQ );
        أكد( (p_Or_رڪن->eOperator & (WO_OR|WO_AND))==0 );
        أكد( p_Or_رڪن->کاٻو_Cursor==iCursor );
        أكد( p_Or_رڪن->u.x.کاٻو_ڪالم==i_ڪالم );
        pThis = p_Or_رڪن->p_اظهار;
        pDup = sqlite3_اظهار_Dup(db, pThis->p_ساڄو, 0);
        pList = sqlite3_اظهار_List_Append(pWInfo->pParse, pList, pDup);
        إذا( p_کاٻو==0 ){
          p_کاٻو = pThis->p_کاٻو;
          pCollSeq = sqlite3_اظهار_Compare_Coll_Seq(pParse, pThis);
        }وإلا{
          أكد( 0==sqlite3_اظهار_Compare(pParse, 
                        sqlite3_اظهار_Skip_Collate(pThis->p_کاٻو),
                        sqlite3_اظهار_Skip_Collate(p_کاٻو), -1) );
          إذا( pCollSeq!=sqlite3_اظهار_Compare_Coll_Seq(pParse, pThis) ){
            p_کاٻو = 0;  /* Collating sequence mismatch */
            اكسر;
          }
        }
      }
      إذا( p_کاٻو==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }وإلا{
        pDup = sqlite3_اظهار_Dup(db, p_کاٻو, 0);
        إذا( sqlite3_اظهار_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          أكد( pCollSeq->zName!=0 );
          pDup = sqlite3_اظهار_جوڙيو_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_اندريون, pDup, 0);
      }
      إذا( pNew ){
        صحيح idxNew;
        transferJoinMarkings(pNew, p_اظهار);
        أكد( اظهار_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_وجهو(pWC, pNew, رڪن_VIRTUAL|رڪن_DYNAMIC);
        testcase( idxNew==0 );
        اظهار_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        mark_رڪن_As_ٻار(pWC, idxNew, idx_رڪن);
      }وإلا{
        sqlite3_اظهار_List_Delete(db, pList);
      }
    }
  }
}
#جيڪڏهن_پورو /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

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
ساكن صحيح رڪن_Is_Equivalence(Parse *pParse, اظهار *p_اظهار, SrcList *pSrc){
  حرف aff1, aff2;
  إذا( !OptimizationEnabled(pParse->db, SQLITE_متعدي) ) أرجع 0;  /* (1) */
  إذا( p_اظهار->op!=TK_EQ && p_اظهار->op!=TK_IS ) أرجع 0;                 /* (2) */
  إذا( اظهار_Has_Property(p_اظهار, EP_OuterON|EP_Collate) ) أرجع 0;        /* (3) */
  أكد( pSrc!=0 );
  إذا( p_اظهار->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    أرجع 0;                                                          /* (4) */
  }
  aff1 = sqlite3_اظهار_Affinity(p_اظهار->p_کاٻو);
  aff2 = sqlite3_اظهار_Affinity(p_اظهار->p_ساڄو);
  إذا( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    أرجع 0;                                                          /* (5) */
  }
  إذا( !sqlite3_اظهار_Coll_Seq_ميلاپ(pParse, p_اظهار->p_کاٻو, p_اظهار->p_ساڄو) ){
    أرجع 0;                                                          /* (6) */
  }
  أرجع 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ساكن Bitmask اظهار_Select_Usage(Where_نقاب_سيٽ *p_نقاب_سيٽ, Select *pS){
  Bitmask نقاب = 0;
  طالما( pS ){
    SrcList *pSrc = pS->pSrc;
    نقاب |= sqlite3_Where_اظهار_List_Usage(p_نقاب_سيٽ, pS->pEList);
    نقاب |= sqlite3_Where_اظهار_List_Usage(p_نقاب_سيٽ, pS->p_گروهه_By);
    نقاب |= sqlite3_Where_اظهار_List_Usage(p_نقاب_سيٽ, pS->p_ترتيب_By);
    نقاب |= sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, pS->pWhere);
    نقاب |= sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, pS->pHaving);
    إذا( ALWAYS(pSrc!=0) ){
      صحيح i;
      لكل(i=0; i<pSrc->nSrc; i++){
        إذا( pSrc->a[i].fg.isSubquery ){
          نقاب |= اظهار_Select_Usage(p_نقاب_سيٽ, pSrc->a[i].u4.pSubq->pSelect);
        }
        إذا( pSrc->a[i].fg.isUsing==0 ){
          نقاب |= sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, pSrc->a[i].u3.pOn);
        }
        إذا( pSrc->a[i].fg.isTabFunc ){
          نقاب |= sqlite3_Where_اظهار_List_Usage(p_نقاب_سيٽ, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->p_اڳواٽ;
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
ساكن SQLITE_NOINLINE صحيح اظهار_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *ai_Cur_کانو,         /* Write the referenced table cursor and column here */
  اظهار *p_اظهار,           /* An operand of a comparison operator */
  صحيح j                  /* Start looking with the j-th pFrom entry */
){
  اشاريو *pIdx;
  صحيح i;
  صحيح iCur;
  افعل{
    iCur = pFrom->a[j].iCursor;
    لكل(pIdx=pFrom->a[j].pSTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){
      إذا( pIdx->a_کانو_اظهار==0 ) استمر;
      لكل(i=0; i<pIdx->n_ڪنجي_کانو; i++){
        إذا( pIdx->ai_ڪالم[i]!=XN_اظهار ) استمر;
        أكد( pIdx->b_Has_اظهار );
        إذا( sqlite3_اظهار_Compare_Skip(p_اظهار,pIdx->a_کانو_اظهار->a[i].p_اظهار,iCur)==0
         && !sqlite3_اظهار_Is_Constant(0,pIdx->a_کانو_اظهار->a[i].p_اظهار)
        ){
          ai_Cur_کانو[0] = iCur;
          ai_Cur_کانو[1] = XN_اظهار;
          أرجع 1;
        }
      }
    }
  }طالما( ++j < pFrom->nSrc );
  أرجع 0;
}
ساكن صحيح اظهار_Might_Be_اشاريي_وارو(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *ai_Cur_کانو,         /* Write the referenced table cursor & column here */
  اظهار *p_اظهار,           /* An operand of a comparison operator */
  صحيح op                 /* The specific comparison operator */
){
  صحيح i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  أكد( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  أكد( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_اندريون<TK_GE );
  أكد( op<=TK_GE );
  إذا( p_اظهار->op==TK_ويڪٽر && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    أكد( اظهار_Use_XList(p_اظهار) );
    p_اظهار = p_اظهار->x.pList->a[0].p_اظهار;
  }

  إذا( p_اظهار->op==TK_ڪالم ){
    ai_Cur_کانو[0] = p_اظهار->i_جدول;
    ai_Cur_کانو[1] = p_اظهار->i_ڪالم;
    أرجع 1;
  }

  لكل(i=0; i<pFrom->nSrc; i++){
    اشاريو *pIdx;
    لكل(pIdx=pFrom->a[i].pSTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){
      إذا( pIdx->a_کانو_اظهار ){
        أرجع اظهار_Might_Be_Indexed2(pFrom,ai_Cur_کانو,p_اظهار,i);
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
ساكن فراغ اظهار_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  صحيح idx_رڪن               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  Where_رڪن *p_رڪن;                /* The term to be analyzed */
  Where_نقاب_سيٽ *p_نقاب_سيٽ;          /* Set of table index masks */
  اظهار *p_اظهار;                     /* The expression to be analyzed */
  Bitmask prereq_کاٻو;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_ساڄو = 0;          /* Extra dependencies on LEFT JOIN */
  اظهار *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  صحيح isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  صحيح noCase = 0;                  /* uppercase equivalent to lowercase */
  صحيح op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  بنانشان حرف eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  صحيح n_کاٻو;                       /* Number of elements on left side vector */

  إذا( db->mallocFailed ){
    أرجع;
  }
  أكد( pWC->n_رڪن > idx_رڪن );
  p_رڪن = &pWC->a[idx_رڪن];
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  p_رڪن->i_رڪن = idx_رڪن;
#جيڪڏهن_پورو
  p_نقاب_سيٽ = &pWInfo->s_نقاب_سيٽ;
  p_اظهار = p_رڪن->p_اظهار;
  أكد( p_اظهار!=0 ); /* Because malloc() has not failed */
  أكد( p_اظهار->op!=TK_AS && p_اظهار->op!=TK_COLLATE );
اظهار_Analyze_restart:
  p_نقاب_سيٽ->bVarSelect = 0;
  prereq_کاٻو = sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, p_اظهار->p_کاٻو);
  op = p_اظهار->op;
  إذا( op==TK_اندريون ){
    أكد( p_اظهار->p_ساڄو==0 );
    إذا( sqlite3_اظهار_Check_اندريون(pParse, p_اظهار) ) أرجع;
    إذا( اظهار_Use_XSelect(p_اظهار) ){
      p_رڪن->prereq_ساڄو = اظهار_Select_Usage(p_نقاب_سيٽ, p_اظهار->x.pSelect);
    }وإلا{
      p_رڪن->prereq_ساڄو = sqlite3_Where_اظهار_List_Usage(p_نقاب_سيٽ, p_اظهار->x.pList);
    }
    prereqAll = prereq_کاٻو | p_رڪن->prereq_ساڄو;
  }وإلا{
    p_رڪن->prereq_ساڄو = sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, p_اظهار->p_ساڄو);
    إذا( p_اظهار->p_کاٻو==0
     || اظهار_Has_Property(p_اظهار, EP_xIsSelect|EP_If_Null_صف)
     || p_اظهار->x.pList!=0
    ){
      prereqAll = sqlite3_Where_اظهار_Usage_NN(p_نقاب_سيٽ, p_اظهار);
    }وإلا{
      prereqAll = prereq_کاٻو | p_رڪن->prereq_ساڄو;
    }
  }
  إذا( p_نقاب_سيٽ->bVarSelect ) p_رڪن->wtFlags |= رڪن_VARSELECT;

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  إذا( prereqAll!=sqlite3_Where_اظهار_Usage_NN(p_نقاب_سيٽ, p_اظهار) ){
    لکو("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_وڻ_View_اظهار(0,p_اظهار,0);
    أكد( 0 );
  }
#جيڪڏهن_پورو

  إذا( اظهار_Has_Property(p_اظهار, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3_Where_Get_نقاب(p_نقاب_سيٽ, p_اظهار->w.iJoin);
    إذا( اظهار_Has_Property(p_اظهار, EP_OuterON) ){
      prereqAll |= x;
      extra_ساڄو = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }وإلا إذا( (prereqAll>>1)>=x ){
      اظهار_Clear_Property(p_اظهار, EP_InnerON);
    }
  }
  p_رڪن->prereqAll = prereqAll;
  p_رڪن->کاٻو_Cursor = -1;
  p_رڪن->i_والد = -1;
  p_رڪن->eOperator = 0;
  إذا( allowedOp(op) ){
    صحيح ai_Cur_کانو[2];
    اظهار *p_کاٻو = sqlite3_اظهار_Skip_Collate(p_اظهار->p_کاٻو);
    اظهار *p_ساڄو = sqlite3_اظهار_Skip_Collate(p_اظهار->p_ساڄو);
    u16 op_نقاب = (p_رڪن->prereq_ساڄو & prereq_کاٻو)==0 ? WO_ALL : WO_EQUIV;

    إذا( p_رڪن->u.x.i_ميدان>0 ){
      أكد( op==TK_اندريون );
      أكد( p_کاٻو->op==TK_ويڪٽر );
      أكد( اظهار_Use_XList(p_کاٻو) );
      p_کاٻو = p_کاٻو->x.pList->a[p_رڪن->u.x.i_ميدان-1].p_اظهار;
    }

    إذا( اظهار_Might_Be_اشاريي_وارو(pSrc, ai_Cur_کانو, p_کاٻو, op) ){
      p_رڪن->کاٻو_Cursor = ai_Cur_کانو[0];
      أكد( (p_رڪن->eOperator & (WO_OR|WO_AND))==0 );
      p_رڪن->u.x.کاٻو_ڪالم = ai_Cur_کانو[1];
      p_رڪن->eOperator = operator_نقاب(op) & op_نقاب;
    }
    إذا( op==TK_IS ) p_رڪن->wtFlags |= رڪن_IS;
    إذا( p_ساڄو
     && اظهار_Might_Be_اشاريي_وارو(pSrc, ai_Cur_کانو, p_ساڄو, op)
     && !اظهار_Has_Property(p_ساڄو, EP_Fixed_کانو)
    ){
      Where_رڪن *pNew;
      اظهار *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      أكد( p_رڪن->u.x.i_ميدان==0 );
      إذا( p_رڪن->کاٻو_Cursor>=0 ){
        صحيح idxNew;
        pDup = sqlite3_اظهار_Dup(db, p_اظهار, 0);
        إذا( db->mallocFailed ){
          sqlite3_اظهار_Delete(db, pDup);
          أرجع;
        }
        idxNew = where_Clause_وجهو(pWC, pDup, رڪن_VIRTUAL|رڪن_DYNAMIC);
        إذا( idxNew==0 ) أرجع;
        pNew = &pWC->a[idxNew];
        mark_رڪن_As_ٻار(pWC, idxNew, idx_رڪن);
        إذا( op==TK_IS ) pNew->wtFlags |= رڪن_IS;
        p_رڪن = &pWC->a[idx_رڪن];
        p_رڪن->wtFlags |= رڪن_COPIED;
        أكد( pWInfo->pTabList!=0 );
        إذا( رڪن_Is_Equivalence(pParse, pDup, pWInfo->pTabList) ){
          p_رڪن->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }وإلا{
        pDup = p_اظهار;
        pNew = p_رڪن;
      }
      pNew->wtFlags |= اظهار_Commute(pParse, pDup);
      pNew->کاٻو_Cursor = ai_Cur_کانو[0];
      أكد( (p_رڪن->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.کاٻو_ڪالم = ai_Cur_کانو[1];
      testcase( (prereq_کاٻو | extra_ساڄو) != prereq_کاٻو );
      pNew->prereq_ساڄو = prereq_کاٻو | extra_ساڄو;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operator_نقاب(pDup->op) + eExtraOp) & op_نقاب;
    }وإلا
    إذا( op==TK_ISNULL
     && !اظهار_Has_Property(p_اظهار,EP_OuterON)
     && 0==sqlite3_اظهار_Can_Be_Null(p_کاٻو)
    ){
      أكد( !اظهار_Has_Property(p_اظهار, EP_Int_قدر) );
      p_اظهار->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_اظهار->u.zToken = "false";
      اظهار_سيٽ_Property(p_اظهار, EP_IsFalse);
      p_رڪن->prereqAll = 0;
      p_رڪن->eOperator = 0;
    }
  }

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_BETWEEN_OPTIMIZATION
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
  وإلا إذا( p_اظهار->op==TK_BETWEEN && pWC->op==TK_AND ){
    اظهار_List *pList;
    صحيح i;
    ساكن ثابت u8 ops[] = {TK_GE, TK_LE};
    أكد( اظهار_Use_XList(p_اظهار) );
    pList = p_اظهار->x.pList;
    أكد( pList!=0 );
    أكد( pList->n_اظهار==2 );
    أكد( pWC->a[idx_رڪن].n_ٻار==0 );
    لكل(i=0; i<2; i++){
      اظهار *p_New_اظهار;
      صحيح idxNew;
      p_New_اظهار = sqlite3PExpr(pParse, ops[i],
                             sqlite3_اظهار_Dup(db, p_اظهار->p_کاٻو, 0),
                             sqlite3_اظهار_Dup(db, pList->a[i].p_اظهار, 0));
      transferJoinMarkings(p_New_اظهار, p_اظهار);
      idxNew = where_Clause_وجهو(pWC, p_New_اظهار, رڪن_VIRTUAL|رڪن_DYNAMIC);
      testcase( idxNew==0 );
      اظهار_Analyze(pSrc, pWC, idxNew);
      p_رڪن = &pWC->a[idx_رڪن];
      mark_رڪن_As_ٻار(pWC, idxNew, idx_رڪن);
    }
  }
#جيڪڏهن_پورو /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#إذا !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  وإلا إذا( p_اظهار->op==TK_OR && !اظهار_Has_Property(p_اظهار, EP_Collate) ){
    اظهار *pAlt = sqlite3_اظهار_Simplified_And_Or(p_اظهار);
    إذا( pAlt!=p_اظهار ){
      p_رڪن->p_اظهار = p_اظهار = sqlite3_اظهار_Skip_Collate_And_Likely(pAlt);
      اذهب اظهار_Analyze_restart;
    }
    أكد( pWC->op==TK_AND );
    اظهار_Analyze_Or_رڪن(pSrc, pWC, idx_رڪن);
    p_رڪن = &pWC->a[idx_رڪن];
  }
#جيڪڏهن_پورو /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  وإلا إذا( p_اظهار->op==TK_NOTNULL ){
    إذا( p_اظهار->p_کاٻو->op==TK_ڪالم
     && p_اظهار->p_کاٻو->i_ڪالم>=0
     && !اظهار_Has_Property(p_اظهار, EP_OuterON)
    ){
      اظهار *p_New_اظهار;
      اظهار *p_کاٻو = p_اظهار->p_کاٻو;
      صحيح idxNew;
      Where_رڪن *p_New_رڪن;
 
      p_New_اظهار = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_اظهار_Dup(db, p_کاٻو, 0),
                              sqlite3_اظهار_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_وجهو(pWC, p_New_اظهار,
                                رڪن_VIRTUAL|رڪن_DYNAMIC|رڪن_VNULL);
      إذا( idxNew ){
        p_New_رڪن = &pWC->a[idxNew];
        p_New_رڪن->prereq_ساڄو = 0;
        p_New_رڪن->کاٻو_Cursor = p_کاٻو->i_جدول;
        p_New_رڪن->u.x.کاٻو_ڪالم = p_کاٻو->i_ڪالم;
        p_New_رڪن->eOperator = WO_GT;
        mark_رڪن_As_ٻار(pWC, idxNew, idx_رڪن);
        p_رڪن = &pWC->a[idx_رڪن];
        p_رڪن->wtFlags |= رڪن_COPIED;
        p_New_رڪن->prereqAll = p_رڪن->prereqAll;
      }
    }
  }


#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_LIKE_OPTIMIZATION
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
  وإلا إذا( p_اظهار->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_اظهار, &pStr1, &isComplete, &noCase)
  ){
    اظهار *p_کاٻو;       /* LHS of LIKE/GLOB operator */
    اظهار *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    اظهار *pNewExpr1;
    اظهار *pNewExpr2;
    صحيح idxNew1;
    صحيح idxNew2;
    ثابت حرف *zCollSeqName;     /* Name of collating sequence */
    ثابت u16 wtFlags = رڪن_LIKEOPT | رڪن_VIRTUAL | رڪن_DYNAMIC;

    أكد( اظهار_Use_XList(p_اظهار) );
    p_کاٻو = p_اظهار->x.pList->a[1].p_اظهار;
    pStr2 = sqlite3_اظهار_Dup(db, pStr1, 0);
    أكد( pStr1==0 || !اظهار_Has_Property(pStr1, EP_Int_قدر) );
    أكد( pStr2==0 || !اظهار_Has_Property(pStr2, EP_Int_قدر) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    إذا( noCase && !pParse->db->mallocFailed ){
      صحيح i;
      حرف c;
      p_رڪن->wtFlags |= رڪن_LIKE;
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
        *pC = sqlite3_مٿانهون_منزل_هيٺاهون[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      طالما( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      أكد( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ٻٽو;
    pNewExpr1 = sqlite3_اظهار_Dup(db, p_کاٻو, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_اظهار_جوڙيو_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_اظهار);
    idxNew1 = where_Clause_وجهو(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_اظهار_Dup(db, p_کاٻو, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_اظهار_جوڙيو_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_اظهار);
    idxNew2 = where_Clause_وجهو(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    اظهار_Analyze(pSrc, pWC, idxNew1);
    اظهار_Analyze(pSrc, pWC, idxNew2);
    p_رڪن = &pWC->a[idx_رڪن];
    إذا( isComplete ){
      mark_رڪن_As_ٻار(pWC, idxNew1, idx_رڪن);
      mark_رڪن_As_ٻار(pWC, idxNew2, idx_رڪن);
    }
  }
#جيڪڏهن_پورو /* SQLITE_OMIT_LIKE_OPTIMIZATION */

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
  إذا( (p_اظهار->op==TK_EQ || p_اظهار->op==TK_IS)
   && (n_کاٻو = sqlite3_اظهار_ويڪٽر_سائيز(p_اظهار->p_کاٻو))>1
   && sqlite3_اظهار_ويڪٽر_سائيز(p_اظهار->p_ساڄو)==n_کاٻو
   && ( (p_اظهار->p_کاٻو->flags & EP_xIsSelect)==0
     || (p_اظهار->p_ساڄو->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    صحيح i;
    لكل(i=0; i<n_کاٻو; i++){
      صحيح idxNew;
      اظهار *pNew;
      اظهار *p_کاٻو = sqlite3_اظهار_For_ويڪٽر_ميدان(pParse, p_اظهار->p_کاٻو, i, n_کاٻو);
      اظهار *p_ساڄو = sqlite3_اظهار_For_ويڪٽر_ميدان(pParse, p_اظهار->p_ساڄو, i, n_کاٻو);

      pNew = sqlite3PExpr(pParse, p_اظهار->op, p_کاٻو, p_ساڄو);
      transferJoinMarkings(pNew, p_اظهار);
      idxNew = where_Clause_وجهو(pWC, pNew, رڪن_DYNAMIC|رڪن_ٽڪرو);
      اظهار_Analyze(pSrc, pWC, idxNew);
    }
    p_رڪن = &pWC->a[idx_رڪن];
    p_رڪن->wtFlags |= رڪن_CODED|رڪن_VIRTUAL;  /* Disable the original */
    p_رڪن->eOperator = WO_ROWVAL;
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
  وإلا إذا( p_اظهار->op==TK_اندريون
   && p_رڪن->u.x.i_ميدان==0
   && p_اظهار->p_کاٻو->op==TK_ويڪٽر
   && ALWAYS( اظهار_Use_XSelect(p_اظهار) )
   && (p_اظهار->x.pSelect->p_اڳواٽ==0 || (p_اظهار->x.pSelect->selFlags & SF_قدرون))
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
   && p_اظهار->x.pSelect->pWin==0
#جيڪڏهن_پورو
   && (p_اظهار->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_اظهار->x.pSelect->pEList->n_اظهار <= UMXV(p_رڪن->n_ٻار)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    صحيح i;
    أكد( p_رڪن->n_ٻار==0 );
    لكل(i=0; i<sqlite3_اظهار_ويڪٽر_سائيز(p_اظهار->p_کاٻو); i++){
      صحيح idxNew;
      idxNew = where_Clause_وجهو(pWC, p_اظهار, رڪن_VIRTUAL|رڪن_ٽڪرو);
      pWC->a[idxNew].u.x.i_ميدان = i+1;
      اظهار_Analyze(pSrc, pWC, idxNew);
      mark_رڪن_As_ٻار(pWC, idxNew, idx_رڪن);
    }
  }

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
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
    اظهار *p_ساڄو = 0, *p_کاٻو = 0;
    صحيح res = isAuxiliaryVtabOperator(db, p_اظهار, &eOp2, &p_کاٻو, &p_ساڄو);
    طالما( res-- > 0 ){
      صحيح idxNew;
      Where_رڪن *p_New_رڪن;
      Bitmask prereq_ڪالم, prereq_اظهار;

      prereq_اظهار = sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, p_ساڄو);
      prereq_ڪالم = sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, p_کاٻو);
      إذا( (prereq_اظهار & prereq_ڪالم)==0 ){
        اظهار *p_New_اظهار;
        p_New_اظهار = sqlite3PExpr(pParse, TK_ميلاپ,
            0, sqlite3_اظهار_Dup(db, p_ساڄو, 0));
        إذا( اظهار_Has_Property(p_اظهار, EP_OuterON) && p_New_اظهار ){
          اظهار_سيٽ_Property(p_New_اظهار, EP_OuterON);
          p_New_اظهار->w.iJoin = p_اظهار->w.iJoin;
        }
        idxNew = where_Clause_وجهو(pWC, p_New_اظهار, رڪن_VIRTUAL|رڪن_DYNAMIC);
        testcase( idxNew==0 );
        p_New_رڪن = &pWC->a[idxNew];
        p_New_رڪن->prereq_ساڄو = prereq_اظهار | extra_ساڄو;
        p_New_رڪن->کاٻو_Cursor = p_کاٻو->i_جدول;
        p_New_رڪن->u.x.کاٻو_ڪالم = p_کاٻو->i_ڪالم;
        p_New_رڪن->eOperator = WO_AUX;
        p_New_رڪن->e_ميلاپ_Op = eOp2;
        mark_رڪن_As_ٻار(pWC, idxNew, idx_رڪن);
        p_رڪن = &pWC->a[idx_رڪن];
        p_رڪن->wtFlags |= رڪن_COPIED;
        p_New_رڪن->prereqAll = p_رڪن->prereqAll;
      }
      مٽا_سٽا(اظهار*, p_کاٻو, p_ساڄو);
    }
  }
#جيڪڏهن_پورو /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( p_رڪن!=&pWC->a[idx_رڪن] );
  p_رڪن = &pWC->a[idx_رڪن];
  p_رڪن->prereq_ساڄو |= extra_ساڄو;
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
فراغ sqlite3_Where_ورهايو(WhereClause *pWC, اظهار *p_اظهار, u8 op){
  اظهار *pE2 = sqlite3_اظهار_Skip_Collate_And_Likely(p_اظهار);
  pWC->op = op;
  أكد( pE2!=0 || p_اظهار==0 );
  إذا( pE2==0 ) أرجع;
  إذا( pE2->op!=op ){
    where_Clause_وجهو(pWC, p_اظهار, 0);
  }وإلا{
    sqlite3_Where_ورهايو(pWC, pE2->p_کاٻو, op);
    sqlite3_Where_ورهايو(pWC, pE2->p_ساڄو, op);
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
ساكن فراغ where_جوڙيو_حد_بندي_اظهار(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  صحيح iReg,           /* Register that will hold value of the limit/offset */
  اظهار *p_اظهار,        /* Expression that defines the limit/offset */
  صحيح iCsr,           /* Cursor to which the constraint applies */
  صحيح e_ميلاپ_Op        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  اظهار *pNew;
  صحيح iVal = 0;

  إذا( sqlite3_اظهار_Is_Integer(p_اظهار, &iVal, pParse, 0) && iVal>=0 ){
    اظهار *pVal = sqlite3_اظهار_Int32(db, iVal);
    إذا( pVal==0 ) أرجع;
    pNew = sqlite3PExpr(pParse, TK_ميلاپ, 0, pVal);
  }وإلا{
    اظهار *pVal = sqlite3_اظهار_Alloc(db, TK_رجسٽر, 0, 0);
    إذا( pVal==0 ) أرجع;
    pVal->i_جدول = iReg;
    pNew = sqlite3PExpr(pParse, TK_ميلاپ, 0, pVal);
  }
  إذا( pNew ){
    Where_رڪن *p_رڪن;
    صحيح idx;
    idx = where_Clause_وجهو(pWC, pNew, رڪن_DYNAMIC|رڪن_VIRTUAL);
    p_رڪن = &pWC->a[idx];
    p_رڪن->کاٻو_Cursor = iCsr;
    p_رڪن->eOperator = WO_AUX;
    p_رڪن->e_ميلاپ_Op = e_ميلاپ_Op;
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
فراغ SQLITE_NOINLINE sqlite3_Where_جوڙيو_حد_بندي(WhereClause *pWC, Select *p){
  أكد( p!=0 && p->p_حد_بندي!=0 );                 /* 1 -- checked by caller */
  إذا( p->p_گروهه_By==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    اظهار_List *p_ترتيب_By = p->p_ترتيب_By;
    صحيح iCsr = p->pSrc->a[0].iCursor;
    صحيح ii;

    /* Check condition (4). Return early if it is not met. */
    لكل(ii=0; ii<pWC->n_رڪن; ii++){
      إذا( pWC->a[ii].wtFlags & رڪن_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        أكد( pWC->a[ii].wtFlags & رڪن_VIRTUAL );
        أكد( pWC->a[ii].eOperator==WO_ROWVAL );
        استمر;
      }
      إذا( pWC->a[ii].n_ٻار ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        استمر;
      }
      إذا( pWC->a[ii].کاٻو_Cursor==iCsr && pWC->a[ii].prereq_ساڄو==0 ) استمر;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      إذا( pWC->a[ii].i_والد>=0 ){
        Where_رڪن *p_والد = &pWC->a[ pWC->a[ii].i_والد ];
        إذا( p_والد->کاٻو_Cursor==iCsr
         && p_والد->prereq_ساڄو==0
         && p_والد->n_ٻار==1
        ){
          استمر;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      أرجع;
    }

    /* Check condition (5). Return early if it is not met. */
    إذا( p_ترتيب_By ){
      لكل(ii=0; ii<p_ترتيب_By->n_اظهار; ii++){
        اظهار *p_اظهار = p_ترتيب_By->a[ii].p_اظهار;
        إذا( p_اظهار->op!=TK_ڪالم ) أرجع;
        إذا( p_اظهار->i_جدول!=iCsr ) أرجع;
        إذا( p_ترتيب_By->a[ii].fg.ترتيب_ڏيو_Flags & KEYINFO_ترتيب_BIGNULL ) أرجع;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    أكد( p->p_حد_بندي->op==TK_حد_بندي );
    إذا( p->i_وٿي!=0 && (p->selFlags & SF_Compound)==0 ){
      where_جوڙيو_حد_بندي_اظهار(pWC, p->i_وٿي, p->p_حد_بندي->p_ساڄو,
                        iCsr, SQLITE_اشاريو_CONSTRAINT_وٿي);
    }
    إذا( p->i_وٿي==0 || (p->selFlags & SF_Compound)==0 ){
      where_جوڙيو_حد_بندي_اظهار(pWC, p->i_حد_بندي, p->p_حد_بندي->p_کاٻو,
                        iCsr, SQLITE_اشاريو_CONSTRAINT_حد_بندي);
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
  pWC->n_رڪن = 0;
  pWC->n_بنياد = 0;
  pWC->n_جڳهه = Array_سائيز(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
فراغ sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  أكد( pWC->n_رڪن>=pWC->n_بنياد );
  إذا( pWC->n_رڪن>0 ){
    Where_رڪن *a = pWC->a;
    Where_رڪن *a_آخري = &pWC->a[pWC->n_رڪن-1];
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
    صحيح i;
    /* Verify that every term past pWC->nBase is virtual */
    لكل(i=pWC->n_بنياد; i<pWC->n_رڪن; i++){
      أكد( (pWC->a[i].wtFlags & رڪن_VIRTUAL)!=0 );
    }
#جيڪڏهن_پورو
    طالما(1){
      أكد( a->e_ميلاپ_Op==0 || a->eOperator==WO_AUX );
      إذا( a->wtFlags & رڪن_DYNAMIC ){
        sqlite3_اظهار_Delete(db, a->p_اظهار);
      }
      إذا( a->wtFlags & (رڪن_ORINFO|رڪن_ANDINFO) ){
        إذا( a->wtFlags & رڪن_ORINFO ){
          أكد( (a->wtFlags & رڪن_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }وإلا{
          أكد( (a->wtFlags & رڪن_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      إذا( a==a_آخري ) اكسر;
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
ساكن SQLITE_NOINLINE Bitmask sqlite3_Where_اظهار_Usage_ڀريل(
  Where_نقاب_سيٽ *p_نقاب_سيٽ,
  اظهار *p
){
  Bitmask نقاب;
  نقاب = (p->op==TK_IF_NULL_صف) ? sqlite3_Where_Get_نقاب(p_نقاب_سيٽ, p->i_جدول) : 0;
  إذا( p->p_کاٻو ) نقاب |= sqlite3_Where_اظهار_Usage_NN(p_نقاب_سيٽ, p->p_کاٻو);
  إذا( p->p_ساڄو ){
    نقاب |= sqlite3_Where_اظهار_Usage_NN(p_نقاب_سيٽ, p->p_ساڄو);
    أكد( p->x.pList==0 );
  }وإلا إذا( اظهار_Use_XSelect(p) ){
    إذا( اظهار_Has_Property(p, EP_VarSelect) ) p_نقاب_سيٽ->bVarSelect = 1;
    نقاب |= اظهار_Select_Usage(p_نقاب_سيٽ, p->x.pSelect);
  }وإلا إذا( p->x.pList ){
    نقاب |= sqlite3_Where_اظهار_List_Usage(p_نقاب_سيٽ, p->x.pList);
  }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
  إذا( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && اظهار_Use_YWin(p) ){
    أكد( p->y.pWin!=0 );
    نقاب |= sqlite3_Where_اظهار_List_Usage(p_نقاب_سيٽ, p->y.pWin->p_حصا_ڪريو);
    نقاب |= sqlite3_Where_اظهار_List_Usage(p_نقاب_سيٽ, p->y.pWin->p_ترتيب_By);
    نقاب |= sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, p->y.pWin->pFilter);
  }
#جيڪڏهن_پورو
  أرجع نقاب;
}
Bitmask sqlite3_Where_اظهار_Usage_NN(Where_نقاب_سيٽ *p_نقاب_سيٽ, اظهار *p){
  إذا( p->op==TK_ڪالم && !اظهار_Has_Property(p, EP_Fixed_کانو) ){
    أرجع sqlite3_Where_Get_نقاب(p_نقاب_سيٽ, p->i_جدول);
  }وإلا إذا( اظهار_Has_Property(p, EP_TokenOnly|EP_پن) ){
    أكد( p->op!=TK_IF_NULL_صف );
    أرجع 0;
  }
  أرجع sqlite3_Where_اظهار_Usage_ڀريل(p_نقاب_سيٽ, p);
}
Bitmask sqlite3_Where_اظهار_Usage(Where_نقاب_سيٽ *p_نقاب_سيٽ, اظهار *p){
  أرجع p ? sqlite3_Where_اظهار_Usage_NN(p_نقاب_سيٽ,p) : 0;
}
Bitmask sqlite3_Where_اظهار_List_Usage(Where_نقاب_سيٽ *p_نقاب_سيٽ, اظهار_List *pList){
  صحيح i;
  Bitmask نقاب = 0;
  إذا( pList ){
    لكل(i=0; i<pList->n_اظهار; i++){
      نقاب |= sqlite3_Where_اظهار_Usage(p_نقاب_سيٽ, pList->a[i].p_اظهار);
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
فراغ sqlite3_Where_اظهار_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  صحيح i;
  لكل(i=pWC->n_رڪن-1; i>=0; i--){
    اظهار_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
فراغ sqlite3_Where_Tab_Func_دليلون(
  Parse *pParse,                    /* Parsing context */
  Src_شي *p_شي,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  جدول *pTab;
  صحيح j, k;
  اظهار_List *p_دليلون;
  اظهار *p_کانو_Ref;
  اظهار *p_رڪن;
  إذا( p_شي->fg.isTabFunc==0 ) أرجع;
  pTab = p_شي->pSTab;
  أكد( pTab!=0 );
  إذا( !IsVirtual(pTab) ){
    sqlite3_Error_نياپو(pParse, "'%s' is not a function", p_شي->zName);
    أرجع;
  }
  p_دليلون = p_شي->u1.pFuncArg;
  إذا( p_دليلون==0 ) أرجع;
  لكل(j=k=0; j<p_دليلون->n_اظهار; j++){
    اظهار *pRhs;
    u32 joinType;
    طالما( k<pTab->n_کانو && (pTab->a_کانو[k].کانو_Flags & COLFLAG_HIDDEN)==0 ){k++;}
    إذا( k>=pTab->n_کانو ){
      sqlite3_Error_نياپو(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      أرجع;
    }
    p_کانو_Ref = sqlite3_اظهار_Alloc(pParse->db, TK_ڪالم, 0, 0);
    إذا( p_کانو_Ref==0 ) أرجع;
    p_کانو_Ref->i_جدول = p_شي->iCursor;
    p_کانو_Ref->i_ڪالم = k++;
    أكد( اظهار_Use_YTab(p_کانو_Ref) );
    p_کانو_Ref->y.pTab = pTab;
    p_شي->کانو_استعمال_ٿيل |= sqlite3_اظهار_کانو_استعمال_ٿيل(p_کانو_Ref);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_اظهار_Dup(pParse->db, p_دليلون->a[j].p_اظهار, 0), 0);
    p_رڪن = sqlite3PExpr(pParse, TK_EQ, p_کانو_Ref, pRhs);
    إذا( p_شي->fg.jointype & (JT_کاٻو|JT_ساڄو) ){
      testcase( p_شي->fg.jointype & JT_کاٻو );  /* testtag-20230227a */
      testcase( p_شي->fg.jointype & JT_ساڄو ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }وإلا{
      testcase( p_شي->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_سيٽ_Join_اظهار(p_رڪن, p_شي->iCursor, joinType);
    where_Clause_وجهو(pWC, p_رڪن, رڪن_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
صحيح sqlite3WhereLoopBloomable(ثابت WhereLoop *pLoop){
  إذا( pLoop->wsFlags & WHERE_IPK ) أرجع 1;
  إذا( NEVER((pLoop->wsFlags & WHERE_اشاريي_وارو)==0) ) أرجع 0;
  أرجع sqlite3_اشاريو_Bloomable(pLoop->u.btree.p_اشاريو, pLoop->u.btree.nEq);
}
