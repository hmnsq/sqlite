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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "whereInt.h"

/* Forward declarations */
ਸਥਿਰ ਖਾਲੀ ਵਿਅੰਜਕ_Analyze(SrcList*, WhereClause*, ਪੂਰਨ);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
ਸਥਿਰ ਖਾਲੀ whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
ਸਥਿਰ ਖਾਲੀ whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
ਸਥਿਰ ਪੂਰਨ where_Clause_ਪਾਓ(WhereClause *pWC, ਵਿਅੰਜਕ *p, u16 wtFlags){
  Where_ਪਦ *p_ਪਦ;
  ਪੂਰਨ idx;
  testcase( wtFlags & ਪਦ_VIRTUAL );
  ਜੇ( pWC->n_ਪਦ>=pWC->n_ਥਾਂ ){
    Where_ਪਦ *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, ਆਕਾਰ(pWC->a[0])*pWC->n_ਥਾਂ*2 );
    ਜੇ( pWC->a==0 ){
      ਜੇ( wtFlags & ਪਦ_DYNAMIC ){
        sqlite3_ਵਿਅੰਜਕ_Delete(db, p);
      }
      pWC->a = pOld;
      ਵਾਪਸ 0;
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pWC->a, pOld, ਆਕਾਰ(pWC->a[0])*pWC->n_ਪਦ);
    pWC->n_ਥਾਂ = pWC->n_ਥਾਂ*2;
  }
  p_ਪਦ = &pWC->a[idx = pWC->n_ਪਦ++];
  ਜੇ( (wtFlags & ਪਦ_VIRTUAL)==0 ) pWC->n_ਅਧਾਰ = pWC->n_ਪਦ;
  ਜੇ( p && ਵਿਅੰਜਕ_Has_Property(p, EP_Unlikely) ){
    p_ਪਦ->truthProb = sqlite3LogEst(p->i_ਸਾਰਣੀ) - 270;
  }ਨਹੀਂਤਾਂ{
    p_ਪਦ->truthProb = 1;
  }
  p_ਪਦ->p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p);
  p_ਪਦ->wtFlags = wtFlags;
  p_ਪਦ->pWC = pWC;
  p_ਪਦ->i_ਮਾਪਾ = -1;
  memset(&p_ਪਦ->eOperator, 0,
         ਆਕਾਰ(Where_ਪਦ) - offsetof(Where_ਪਦ,eOperator));
  ਵਾਪਸ idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
ਸਥਿਰ ਪੂਰਨ allowedOp(ਪੂਰਨ op){
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_GT>TK_EQ && TK_GT<TK_GE );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_LT>TK_EQ && TK_LT<TK_GE );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_LE>TK_EQ && TK_LE<TK_GE );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_GE==TK_EQ+4 );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_ਵਿੱਚ<TK_EQ );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_IS<TK_EQ );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_ISNULL<TK_EQ );
  ਜੇ( op>TK_GE ) ਵਾਪਸ 0;
  ਜੇ( op>=TK_EQ ) ਵਾਪਸ 1;
  ਵਾਪਸ op==TK_ਵਿੱਚ || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ਸਥਿਰ u16 ਵਿਅੰਜਕ_Commute(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਸਦਿਸ਼
   || p_ਵਿਅੰਜਕ->p_ਸੱਜਾ->op==TK_ਸਦਿਸ਼
   || sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ) !=
      sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ)
  ){
    p_ਵਿਅੰਜਕ->flags ^= EP_Commuted;
  }
  ਅਦਲਾ_ਬਦਲੀ(ਵਿਅੰਜਕ*,p_ਵਿਅੰਜਕ->p_ਸੱਜਾ,p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
  ਜੇ( p_ਵਿਅੰਜਕ->op>=TK_GT ){
    ਪ੍ਰਤਿਜ੍ਞਾ( TK_LT==TK_GT+2 );
    ਪ੍ਰਤਿਜ੍ਞਾ( TK_GE==TK_LE+2 );
    ਪ੍ਰਤਿਜ੍ਞਾ( TK_GT>TK_EQ );
    ਪ੍ਰਤਿਜ੍ਞਾ( TK_GT<TK_LE );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op>=TK_GT && p_ਵਿਅੰਜਕ->op<=TK_GE );
    p_ਵਿਅੰਜਕ->op = ((p_ਵਿਅੰਜਕ->op-TK_GT)^2)+TK_GT;
  }
  ਵਾਪਸ 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ਸਥਿਰ u16 operator_ਨਕਾਬ(ਪੂਰਨ op){
  u16 c;
  ਪ੍ਰਤਿਜ੍ਞਾ( allowedOp(op) );
  ਜੇ( op>=TK_EQ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }ਨਹੀਂਤਾਂ ਜੇ( op==TK_ਵਿੱਚ ){
    c = WO_ਵਿੱਚ;
  }ਨਹੀਂਤਾਂ ਜੇ( op==TK_ISNULL ){
    c = WO_ISNULL;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_IS );
    c = WO_IS;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_ISNULL || c==WO_ISNULL );
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_ਵਿੱਚ || c==WO_ਵਿੱਚ );
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_EQ || c==WO_EQ );
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_LT || c==WO_LT );
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_LE || c==WO_LE );
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_GT || c==WO_GT );
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_GE || c==WO_GE );
  ਪ੍ਰਤਿਜ੍ਞਾ( op!=TK_IS || c==WO_IS );
  ਵਾਪਸ c;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_LIKE_OPTIMIZATION
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
ਸਥਿਰ ਪੂਰਨ isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,      /* Test this expression */
  ਵਿਅੰਜਕ **pp_ਅਗੇਤਰ,  /* Pointer to TK_STRING expression with pattern prefix */
  ਪੂਰਨ *pisComplete, /* True if the only wildcard is % in the last character */
  ਪੂਰਨ *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ਅਚਲ u8 *z = 0;           /* String on RHS of LIKE operator */
  ਵਿਅੰਜਕ *p_ਸੱਜਾ, *p_ਖੱਬਾ;      /* Right and left size of LIKE operator */
  ਵਿਅੰਜਕ_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  ਪੂਰਨ cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_ਮੁੱਲ *pVal = 0;
  ਪੂਰਨ op;                    /* Opcode of pRight */
  ਪੂਰਨ rc;                    /* Result code to return */

  ਜੇ( !sqlite3IsLikeFunction(db, p_ਵਿਅੰਜਕ, pnoCase, (ਅੱਖਰ*)wc) ){
    ਵਾਪਸ 0;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
  ਜੇ( *pnoCase ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
  pList = p_ਵਿਅੰਜਕ->x.pList;
  p_ਖੱਬਾ = pList->a[1].p_ਵਿਅੰਜਕ;

  p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate(pList->a[0].p_ਵਿਅੰਜਕ);
  op = p_ਸੱਜਾ->op;
  ਜੇ( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    ਪੂਰਨ i_ਖਾਨਾ = p_ਸੱਜਾ->i_ਕਾਲਮ;
    pVal = sqlite3_Vdbe_Get_Bound_ਮੁੱਲ(pReprepare, i_ਖਾਨਾ, SQLITE_AFF_BLOB);
    ਜੇ( pVal && sqlite3_ਮੁੱਲ_type(pVal)==SQLITE_ਪਾਠ ){
      z = sqlite3_ਮੁੱਲ_ਪਾਠ(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, i_ਖਾਨਾ, 0);
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸੱਜਾ->op==TK_VARIABLE || p_ਸੱਜਾ->op==TK_ਰਜਿਸਟਰ );
  }ਨਹੀਂਤਾਂ ਜੇ( op==TK_STRING ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਸੱਜਾ, EP_Int_ਮੁੱਲ) );
     z = (u8*)p_ਸੱਜਾ->u.zToken;
  }
  ਜੇ( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    ਜਦੋਂ( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      ਜੇ( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }ਨਹੀਂਤਾਂ ਜੇ( c>=0x80 ){
        ਅਚਲ u8 *z2 = z+cnt-1;
        ਜੇ( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ{
          cnt = (ਪੂਰਨ)(z2-z);
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
    ਜੇ( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      ਵਿਅੰਜਕ *p_ਅਗੇਤਰ;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      p_ਅਗੇਤਰ = sqlite3_ਵਿਅੰਜਕ(db, TK_STRING, (ਅੱਖਰ*)z);
      ਜੇ( p_ਅਗੇਤਰ ){
        ਪੂਰਨ iFrom, i_ਮੰਜ਼ਿਲ;
        ਅੱਖਰ *zNew;
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਅਗੇਤਰ, EP_Int_ਮੁੱਲ) );
        zNew = p_ਅਗੇਤਰ->u.zToken;
        zNew[cnt] = 0;
        ਲਈ(iFrom=i_ਮੰਜ਼ਿਲ=0; iFrom<cnt; iFrom++){
          ਜੇ( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[i_ਮੰਜ਼ਿਲ++] = zNew[iFrom];
        }
        zNew[i_ਮੰਜ਼ਿਲ] = 0;
        ਪ੍ਰਤਿਜ੍ਞਾ( i_ਮੰਜ਼ਿਲ>0 );

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
        ਜੇ( p_ਖੱਬਾ->op!=TK_ਕਾਲਮ
         || sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਖੱਬਾ)!=SQLITE_AFF_ਪਾਠ
         || (ALWAYS( ਵਿਅੰਜਕ_Use_YTab(p_ਖੱਬਾ) )
             && ALWAYS(p_ਖੱਬਾ->y.pTab)
             && IsVirtual(p_ਖੱਬਾ->y.pTab))  /* Might be numeric */
        ){
          ਪੂਰਨ is_ਅੰਕ;
          ਦੁੱਗਣਾ rDummy;
          ਪ੍ਰਤਿਜ੍ਞਾ( zNew[i_ਮੰਜ਼ਿਲ]==0 );
          is_ਅੰਕ = sqlite3AtoF(zNew, &rDummy);
          ਜੇ( is_ਅੰਕ<=0 ){
            ਜੇ( i_ਮੰਜ਼ਿਲ==1 && zNew[0]=='-' ){
              is_ਅੰਕ = +1;
            }ਨਹੀਂਤਾਂ{
              zNew[i_ਮੰਜ਼ਿਲ-1]++;
              is_ਅੰਕ = sqlite3AtoF(zNew, &rDummy);
              zNew[i_ਮੰਜ਼ਿਲ-1]--;
            }
          }
          ਜੇ( is_ਅੰਕ>0 ){
            sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਅਗੇਤਰ);
            sqlite3_ਮੁੱਲ_Free(pVal);
            ਵਾਪਸ 0;
          }
        }
      }
      *pp_ਅਗੇਤਰ = p_ਅਗੇਤਰ;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      ਜੇ( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_ਸੱਜਾ->i_ਕਾਲਮ, 0);
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਸੱਜਾ, EP_Int_ਮੁੱਲ) );
        ਜੇ( *pisComplete && p_ਸੱਜਾ->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
          sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, p_ਸੱਜਾ, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
        }
      }
    }ਨਹੀਂਤਾਂ{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_ਮੁੱਲ_Free(pVal);
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Like_Operator(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
    ਅਚਲ ਅੱਖਰ *zOp;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ eOp;
  } aOp[] = {
    { "match",  SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਮੇਲ },
    { "glob",   SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GLOB },
    { "like",   SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LIKE },
    { "regexp", SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_REGEXP }
  };
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
  ਲਈ(i=0; i<Array_ਮਾਪ(aOp); i++){
    ਜੇ( sqlite3StrICmp(p_ਵਿਅੰਜਕ->u.zToken, aOp[i].zOp)==0 ){
      ਵਾਪਸ aOp[i].eOp;
    }
  }
  ਵਾਪਸ 0;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
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
ਸਥਿਰ ਪੂਰਨ isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,                    /* Test this expression */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  ਵਿਅੰਜਕ **pp_ਖੱਬਾ,                  /* Column expression to left of MATCH/op2 */
  ਵਿਅੰਜਕ **pp_ਸੱਜਾ                  /* Expression to left of MATCH/op2 */
){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION ){
    ਵਿਅੰਜਕ_List *pList;
    ਵਿਅੰਜਕ *p_ਖਾਨਾ;                     /* Column reference */
    ਪੂਰਨ i;

    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
    pList = p_ਵਿਅੰਜਕ->x.pList;
    ਜੇ( pList==0 || pList->n_ਵਿਅੰਜਕ!=2 ){
      ਵਾਪਸ 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    p_ਖਾਨਾ = pList->a[1].p_ਵਿਅੰਜਕ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ->op!=TK_ਕਾਲਮ || (ਵਿਅੰਜਕ_Use_YTab(p_ਖਾਨਾ) && p_ਖਾਨਾ->y.pTab!=0) );
    ਜੇ( ਵਿਅੰਜਕ_Is_Vtab(p_ਖਾਨਾ) && (i = sqlite3_ਵਿਅੰਜਕ_Is_Like_Operator(p_ਵਿਅੰਜਕ))!=0 ){
      *peOp2 = i;
      *pp_ਸੱਜਾ = pList->a[0].p_ਵਿਅੰਜਕ;
      *pp_ਖੱਬਾ = p_ਖਾਨਾ;
      ਵਾਪਸ 1;
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
    p_ਖਾਨਾ = pList->a[0].p_ਵਿਅੰਜਕ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ->op!=TK_ਕਾਲਮ || ਵਿਅੰਜਕ_Use_YTab(p_ਖਾਨਾ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ->op!=TK_ਕਾਲਮ || (ਵਿਅੰਜਕ_Use_YTab(p_ਖਾਨਾ) && p_ਖਾਨਾ->y.pTab!=0) );
    ਜੇ( ਵਿਅੰਜਕ_Is_Vtab(p_ਖਾਨਾ) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      ਖਾਲੀ (*x_Not_ਵਰਤਿਆ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**);
      ਖਾਲੀ *p_Not_ਵਰਤਿਆ;
      pVtab = sqlite3GetVTable(db, p_ਖਾਨਾ->y.pTab)->pVtab;
      ਪ੍ਰਤਿਜ੍ਞਾ( pVtab!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pVtab->pModule!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      pMod = (sqlite3_module *)pVtab->pModule;
      ਜੇ( pMod->x_ਲੱਭੋ_Function!=0 ){
        i = pMod->x_ਲੱਭੋ_Function(pVtab,2, p_ਵਿਅੰਜਕ->u.zToken, &x_Not_ਵਰਤਿਆ, &p_Not_ਵਰਤਿਆ);
        ਜੇ( i>=SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_ਸੱਜਾ = pList->a[1].p_ਵਿਅੰਜਕ;
          *pp_ਖੱਬਾ = p_ਖਾਨਾ;
          ਵਾਪਸ 1;
        }
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    ਪ੍ਰਤਿਜ੍ਞਾ( TK_NE < TK_EQ );
    ਪ੍ਰਤਿਜ੍ਞਾ( TK_ISNOT < TK_EQ );
    ਪ੍ਰਤਿਜ੍ਞਾ( TK_NOTNULL < TK_EQ );
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_NE || p_ਵਿਅੰਜਕ->op==TK_ISNOT || p_ਵਿਅੰਜਕ->op==TK_NOTNULL ){
    ਪੂਰਨ res = 0;
    ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
    ਵਿਅੰਜਕ *p_ਸੱਜਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ->op!=TK_ਕਾਲਮ || (ਵਿਅੰਜਕ_Use_YTab(p_ਖੱਬਾ) && p_ਖੱਬਾ->y.pTab!=0) );
    ਜੇ( ਵਿਅੰਜਕ_Is_Vtab(p_ਖੱਬਾ) ){
      res++;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸੱਜਾ==0 || p_ਸੱਜਾ->op!=TK_ਕਾਲਮ
            || (ਵਿਅੰਜਕ_Use_YTab(p_ਸੱਜਾ) && p_ਸੱਜਾ->y.pTab!=0) );
    ਜੇ( p_ਸੱਜਾ && ਵਿਅੰਜਕ_Is_Vtab(p_ਸੱਜਾ) ){
      res++;
      ਅਦਲਾ_ਬਦਲੀ(ਵਿਅੰਜਕ*, p_ਖੱਬਾ, p_ਸੱਜਾ);
    }
    *pp_ਖੱਬਾ = p_ਖੱਬਾ;
    *pp_ਸੱਜਾ = p_ਸੱਜਾ;
    ਜੇ( p_ਵਿਅੰਜਕ->op==TK_NE ) *peOp2 = SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_NE;
    ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ISNOT ) *peOp2 = SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ISNOT;
    ਜੇ( p_ਵਿਅੰਜਕ->op==TK_NOTNULL ) *peOp2 = SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ISNOTNULL;
    ਵਾਪਸ res;
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ਸਥਿਰ ਖਾਲੀ transferJoinMarkings(ਵਿਅੰਜਕ *pDerived, ਵਿਅੰਜਕ *p_ਅਧਾਰ){
  ਜੇ( pDerived && ਵਿਅੰਜਕ_Has_Property(p_ਅਧਾਰ, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= p_ਅਧਾਰ->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = p_ਅਧਾਰ->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ਸਥਿਰ ਖਾਲੀ mark_ਪਦ_As_ਬੱਚਾ(WhereClause *pWC, ਪੂਰਨ i_ਬੱਚਾ, ਪੂਰਨ i_ਮਾਪਾ){
  pWC->a[i_ਬੱਚਾ].i_ਮਾਪਾ = i_ਮਾਪਾ;
  pWC->a[i_ਬੱਚਾ].truthProb = pWC->a[i_ਮਾਪਾ].truthProb;
  ਪ੍ਰਤਿਜ੍ਞਾ(   pWC->a[i_ਮਾਪਾ].n_ਬੱਚਾ <  UMXV(pWC->a[0].n_ਬੱਚਾ) );
  pWC->a[i_ਮਾਪਾ].n_ਬੱਚਾ++;
  testcase( pWC->a[i_ਮਾਪਾ].n_ਬੱਚਾ == UMXV(pWC->a[0].n_ਬੱਚਾ) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ਸਥਿਰ Where_ਪਦ *whereNthSubterm(Where_ਪਦ *p_ਪਦ, ਪੂਰਨ N){
  ਜੇ( p_ਪਦ->eOperator!=WO_AND ){
    ਵਾਪਸ N==0 ? p_ਪਦ : 0;
  }
  ਜੇ( N<p_ਪਦ->u.pAndInfo->wc.n_ਪਦ ){
    ਵਾਪਸ &p_ਪਦ->u.pAndInfo->wc.a[N];
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਖਾਲੀ whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  Where_ਪਦ *pOne,       /* First disjunct */
  Where_ਪਦ *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  ਵਿਅੰਜਕ *pNew;            /* New virtual expression */
  ਪੂਰਨ op;                /* Operator for the combined expression */
  ਪੂਰਨ idxNew;            /* Index in pWC of the next virtual term */
  ਵਿਅੰਜਕ *pA, *pB;         /* Expressions associated with pOne and pTwo */

  ਜੇ( (pOne->wtFlags | pTwo->wtFlags) & ਪਦ_VNULL ) ਵਾਪਸ;
  ਜੇ( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ਵਾਪਸ;
  ਜੇ( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ਵਾਪਸ;
  ਜੇ( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) ਵਾਪਸ;
  pA = pOne->p_ਵਿਅੰਜਕ;
  pB = pTwo->p_ਵਿਅੰਜਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pA->p_ਖੱਬਾ!=0 && pA->p_ਸੱਜਾ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pB->p_ਖੱਬਾ!=0 && pB->p_ਸੱਜਾ!=0 );
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0,pA->p_ਖੱਬਾ, pB->p_ਖੱਬਾ, -1) )  ਵਾਪਸ;
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0,pA->p_ਸੱਜਾ, pB->p_ਸੱਜਾ,-1) ) ਵਾਪਸ;
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(pA,EP_Commuted)!=ਵਿਅੰਜਕ_Has_Property(pB,EP_Commuted) ){
    ਵਾਪਸ;
  }
  /* If we reach this point, it means the two subterms can be combined */
  ਜੇ( (eOp & (eOp-1))!=0 ){
    ਜੇ( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_ਵਿਅੰਜਕ_Dup(db, pA, 0);
  ਜੇ( pNew==0 ) ਵਾਪਸ;
  ਲਈ(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ ਪ੍ਰਤਿਜ੍ਞਾ( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_ਪਾਓ(pWC, pNew, ਪਦ_VIRTUAL|ਪਦ_DYNAMIC);
  ਵਿਅੰਜਕ_Analyze(pSrc, pWC, idxNew);
}

#ਜੇ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
ਸਥਿਰ ਖਾਲੀ ਵਿਅੰਜਕ_Analyze_Or_ਪਦ(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  ਪੂਰਨ idx_ਪਦ               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  Where_ਪਦ *p_ਪਦ = &pWC->a[idx_ਪਦ];    /* The term to be analyzed */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;             /* The expression of the term */
  ਪੂਰਨ i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  Where_ਪਦ *p_Or_ਪਦ;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->wtFlags & (ਪਦ_DYNAMIC|ਪਦ_ORINFO|ਪਦ_ANDINFO))==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_OR );
  p_ਪਦ->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, ਆਕਾਰ(*pOrInfo));
  ਜੇ( pOrInfo==0 ) ਵਾਪਸ;
  p_ਪਦ->wtFlags |= ਪਦ_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, ਆਕਾਰ(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3_Where_ਵੰਡੋ(pOrWc, p_ਵਿਅੰਜਕ, TK_OR);
  sqlite3_Where_ਵਿਅੰਜਕ_Analyze(pSrc, pOrWc);
  ਜੇ( db->mallocFailed ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pOrWc->n_ਪਦ>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ = ~(Bitmask)0;
  ਲਈ(i=pOrWc->n_ਪਦ-1, p_Or_ਪਦ=pOrWc->a; i>=0 && indexable; i--, p_Or_ਪਦ++){
    ਜੇ( (p_Or_ਪਦ->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_Or_ਪਦ->wtFlags & (ਪਦ_ANDINFO|ਪਦ_ORINFO))==0 );
      chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, ਆਕਾਰ(*pAndInfo));
      ਜੇ( pAndInfo ){
        WhereClause *pAndWC;
        Where_ਪਦ *p_And_ਪਦ;
        ਪੂਰਨ j;
        Bitmask b = 0;
        p_Or_ਪਦ->u.pAndInfo = pAndInfo;
        p_Or_ਪਦ->wtFlags |= ਪਦ_ANDINFO;
        p_Or_ਪਦ->eOperator = WO_AND;
        p_Or_ਪਦ->ਖੱਬਾ_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, ਆਕਾਰ(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3_Where_ਵੰਡੋ(pAndWC, p_Or_ਪਦ->p_ਵਿਅੰਜਕ, TK_AND);
        sqlite3_Where_ਵਿਅੰਜਕ_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        ਜੇ( !db->mallocFailed ){
          ਲਈ(j=0, p_And_ਪਦ=pAndWC->a; j<pAndWC->n_ਪਦ; j++, p_And_ਪਦ++){
            ਪ੍ਰਤਿਜ੍ਞਾ( p_And_ਪਦ->p_ਵਿਅੰਜਕ );
            ਜੇ( allowedOp(p_And_ਪਦ->p_ਵਿਅੰਜਕ->op)
             || p_And_ਪਦ->eOperator==WO_AUX
            ){
              b |= sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, p_And_ਪਦ->ਖੱਬਾ_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( p_Or_ਪਦ->wtFlags & ਪਦ_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }ਨਹੀਂਤਾਂ{
      Bitmask b;
      b = sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, p_Or_ਪਦ->ਖੱਬਾ_Cursor);
      ਜੇ( p_Or_ਪਦ->wtFlags & ਪਦ_VIRTUAL ){
        Where_ਪਦ *pOther = &pOrWc->a[p_Or_ਪਦ->i_ਮਾਪਾ];
        b |= sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, pOther->ਖੱਬਾ_Cursor);
      }
      indexable &= b;
      ਜੇ( (p_Or_ਪਦ->eOperator & WO_EQ)==0 ){
        chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ = 0;
      }ਨਹੀਂਤਾਂ{
        chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  p_ਪਦ->eOperator = WO_OR;
  p_ਪਦ->ਖੱਬਾ_Cursor = -1;
  ਜੇ( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  ਜੇ( indexable && pOrWc->n_ਪਦ==2 ){
    ਪੂਰਨ iOne = 0;
    Where_ਪਦ *pOne;
    ਜਦੋਂ( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      ਪੂਰਨ iTwo = 0;
      Where_ਪਦ *pTwo;
      ਜਦੋਂ( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  ਜੇ( chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ ){
    ਪੂਰਨ ok_ਮੰਜ਼ਿਲ_Chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ = 0;     /* True if the conversion to IN is valid */
    ਪੂਰਨ i_ਕਾਲਮ = -1;         /* Column index on lhs of IN operator */
    ਪੂਰਨ iCursor = -1;         /* Table cursor common to all terms */
    ਪੂਰਨ j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    ਲਈ(j=0; j<2 && !ok_ਮੰਜ਼ਿਲ_Chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ; j++){
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = 0;
      p_Or_ਪਦ = pOrWc->a;
      ਲਈ(i=pOrWc->n_ਪਦ-1; i>=0; i--, p_Or_ਪਦ++){
        ਪ੍ਰਤਿਜ੍ਞਾ( p_Or_ਪਦ->eOperator & WO_EQ );
        p_Or_ਪਦ->wtFlags &= ~ਪਦ_OK;
        ਜੇ( p_Or_ਪਦ->ਖੱਬਾ_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          ਪ੍ਰਤਿਜ੍ਞਾ( j==1 );
          ਜਾਰੀ;
        }
        ਜੇ( (chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ & sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ,
                                            p_Or_ਪਦ->ਖੱਬਾ_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( p_Or_ਪਦ->wtFlags & ਪਦ_COPIED );
          testcase( p_Or_ਪਦ->wtFlags & ਪਦ_VIRTUAL );
          ਪ੍ਰਤਿਜ੍ਞਾ( p_Or_ਪਦ->wtFlags & (ਪਦ_COPIED|ਪਦ_VIRTUAL) );
          ਜਾਰੀ;
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( (p_Or_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
        i_ਕਾਲਮ = p_Or_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ;
        iCursor = p_Or_ਪਦ->ਖੱਬਾ_Cursor;
        p_ਖੱਬਾ = p_Or_ਪਦ->p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
        ਰੋਕੋ;
      }
      ਜੇ( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        ਪ੍ਰਤਿਜ੍ਞਾ( j==1 );
        ਪ੍ਰਤਿਜ੍ਞਾ( Is_Power_ਦਾ_Two(chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ) );
        ਪ੍ਰਤਿਜ੍ਞਾ( chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ==sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, iCursor) );
        ਰੋਕੋ;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      ok_ਮੰਜ਼ਿਲ_Chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ = 1;
      ਲਈ(; i>=0 && ok_ਮੰਜ਼ਿਲ_Chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ; i--, p_Or_ਪਦ++){
        ਪ੍ਰਤਿਜ੍ਞਾ( p_Or_ਪਦ->eOperator & WO_EQ );
        ਪ੍ਰਤਿਜ੍ਞਾ( (p_Or_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
        ਜੇ( p_Or_ਪਦ->ਖੱਬਾ_Cursor!=iCursor ){
          p_Or_ਪਦ->wtFlags &= ~ਪਦ_OK;
        }ਨਹੀਂਤਾਂ ਜੇ( p_Or_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ!=i_ਕਾਲਮ || (i_ਕਾਲਮ==XN_ਵਿਅੰਜਕ
               && sqlite3_ਵਿਅੰਜਕ_Compare(pParse, p_Or_ਪਦ->p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, p_ਖੱਬਾ, -1)
        )){
          ok_ਮੰਜ਼ਿਲ_Chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ = 0;
        }ਨਹੀਂਤਾਂ{
          ਪੂਰਨ aff_ਖੱਬਾ, aff_ਸੱਜਾ;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Affinity(p_Or_ਪਦ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
          aff_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_Affinity(p_Or_ਪਦ->p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
          ਜੇ( aff_ਸੱਜਾ!=0 && aff_ਸੱਜਾ!=aff_ਖੱਬਾ ){
            ok_ਮੰਜ਼ਿਲ_Chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ = 0;
          }ਨਹੀਂਤਾਂ{
            p_Or_ਪਦ->wtFlags |= ਪਦ_OK;
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
    ਜੇ( ok_ਮੰਜ਼ਿਲ_Chng_ਮੰਜ਼ਿਲ_ਵਿੱਚ ){
      ਵਿਅੰਜਕ *pDup;            /* A transient duplicate expression */
      ਵਿਅੰਜਕ_List *pList = 0;   /* The RHS of the IN operator */
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      ਵਿਅੰਜਕ *pNew;            /* The complete IN operator */

      ਲਈ(i=pOrWc->n_ਪਦ-1, p_Or_ਪਦ=pOrWc->a; i>=0; i--, p_Or_ਪਦ++){
        ਵਿਅੰਜਕ *pThis;
        ਜੇ( (p_Or_ਪਦ->wtFlags & ਪਦ_OK)==0 ) ਜਾਰੀ;
        ਪ੍ਰਤਿਜ੍ਞਾ( p_Or_ਪਦ->eOperator & WO_EQ );
        ਪ੍ਰਤਿਜ੍ਞਾ( (p_Or_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_Or_ਪਦ->ਖੱਬਾ_Cursor==iCursor );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_Or_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ==i_ਕਾਲਮ );
        pThis = p_Or_ਪਦ->p_ਵਿਅੰਜਕ;
        pDup = sqlite3_ਵਿਅੰਜਕ_Dup(db, pThis->p_ਸੱਜਾ, 0);
        pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pWInfo->pParse, pList, pDup);
        ਜੇ( p_ਖੱਬਾ==0 ){
          p_ਖੱਬਾ = pThis->p_ਖੱਬਾ;
          pCollSeq = sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pParse, pThis);
        }ਨਹੀਂਤਾਂ{
          ਪ੍ਰਤਿਜ੍ਞਾ( 0==sqlite3_ਵਿਅੰਜਕ_Compare(pParse, 
                        sqlite3_ਵਿਅੰਜਕ_Skip_Collate(pThis->p_ਖੱਬਾ),
                        sqlite3_ਵਿਅੰਜਕ_Skip_Collate(p_ਖੱਬਾ), -1) );
          ਜੇ( pCollSeq!=sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pParse, pThis) ){
            p_ਖੱਬਾ = 0;  /* Collating sequence mismatch */
            ਰੋਕੋ;
          }
        }
      }
      ਜੇ( p_ਖੱਬਾ==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }ਨਹੀਂਤਾਂ{
        pDup = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਖੱਬਾ, 0);
        ਜੇ( sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pCollSeq->zName!=0 );
          pDup = sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_ਵਿੱਚ, pDup, 0);
      }
      ਜੇ( pNew ){
        ਪੂਰਨ idxNew;
        transferJoinMarkings(pNew, p_ਵਿਅੰਜਕ);
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_ਪਾਓ(pWC, pNew, ਪਦ_VIRTUAL|ਪਦ_DYNAMIC);
        testcase( idxNew==0 );
        ਵਿਅੰਜਕ_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        mark_ਪਦ_As_ਬੱਚਾ(pWC, idxNew, idx_ਪਦ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pList);
      }
    }
  }
}
#ਜੇ_ਅੰਤ /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

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
ਸਥਿਰ ਪੂਰਨ ਪਦ_Is_Equivalence(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, SrcList *pSrc){
  ਅੱਖਰ aff1, aff2;
  ਜੇ( !OptimizationEnabled(pParse->db, SQLITE_ਸੰਕ੍ਰਾਮਕ) ) ਵਾਪਸ 0;  /* (1) */
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_EQ && p_ਵਿਅੰਜਕ->op!=TK_IS ) ਵਾਪਸ 0;                 /* (2) */
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON|EP_Collate) ) ਵਾਪਸ 0;        /* (3) */
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc!=0 );
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    ਵਾਪਸ 0;                                                          /* (4) */
  }
  aff1 = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
  aff2 = sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
  ਜੇ( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    ਵਾਪਸ 0;                                                          /* (5) */
  }
  ਜੇ( !sqlite3_ਵਿਅੰਜਕ_Coll_Seq_ਮੇਲ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ) ){
    ਵਾਪਸ 0;                                                          /* (6) */
  }
  ਵਾਪਸ 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ਸਥਿਰ Bitmask ਵਿਅੰਜਕ_Select_Usage(Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ, Select *pS){
  Bitmask ਨਕਾਬ = 0;
  ਜਦੋਂ( pS ){
    SrcList *pSrc = pS->pSrc;
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pS->pEList);
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pS->p_ਸਮੂਹ_By);
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pS->p_ਕ੍ਰਮ_By);
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pS->pWhere);
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pS->pHaving);
    ਜੇ( ALWAYS(pSrc!=0) ){
      ਪੂਰਨ i;
      ਲਈ(i=0; i<pSrc->nSrc; i++){
        ਜੇ( pSrc->a[i].fg.isSubquery ){
          ਨਕਾਬ |= ਵਿਅੰਜਕ_Select_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pSrc->a[i].u4.pSubq->pSelect);
        }
        ਜੇ( pSrc->a[i].fg.isUsing==0 ){
          ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pSrc->a[i].u3.pOn);
        }
        ਜੇ( pSrc->a[i].fg.isTabFunc ){
          ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->p_ਪੂਰਵ;
  }
  ਵਾਪਸ ਨਕਾਬ;
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ ਵਿਅੰਜਕ_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  ਪੂਰਨ *ai_Cur_ਖਾਨਾ,         /* Write the referenced table cursor and column here */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,           /* An operand of a comparison operator */
  ਪੂਰਨ j                  /* Start looking with the j-th pFrom entry */
){
  ਸੂਚਕਾਂਕ *pIdx;
  ਪੂਰਨ i;
  ਪੂਰਨ iCur;
  ਕਰੋ{
    iCur = pFrom->a[j].iCursor;
    ਲਈ(pIdx=pFrom->a[j].pSTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
      ਜੇ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ==0 ) ਜਾਰੀ;
      ਲਈ(i=0; i<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
        ਜੇ( pIdx->ai_ਕਾਲਮ[i]!=XN_ਵਿਅੰਜਕ ) ਜਾਰੀ;
        ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->b_Has_ਵਿਅੰਜਕ );
        ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare_Skip(p_ਵਿਅੰਜਕ,pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i].p_ਵਿਅੰਜਕ,iCur)==0
         && !sqlite3_ਵਿਅੰਜਕ_Is_Constant(0,pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i].p_ਵਿਅੰਜਕ)
        ){
          ai_Cur_ਖਾਨਾ[0] = iCur;
          ai_Cur_ਖਾਨਾ[1] = XN_ਵਿਅੰਜਕ;
          ਵਾਪਸ 1;
        }
      }
    }
  }ਜਦੋਂ( ++j < pFrom->nSrc );
  ਵਾਪਸ 0;
}
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Might_Be_ਸੂਚਕਾਂਕਿਤ(
  SrcList *pFrom,        /* The FROM clause */
  ਪੂਰਨ *ai_Cur_ਖਾਨਾ,         /* Write the referenced table cursor & column here */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,           /* An operand of a comparison operator */
  ਪੂਰਨ op                 /* The specific comparison operator */
){
  ਪੂਰਨ i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_ਵਿੱਚ<TK_GE );
  ਪ੍ਰਤਿਜ੍ਞਾ( op<=TK_GE );
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਸਦਿਸ਼ && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
    p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ;
  }

  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ ){
    ai_Cur_ਖਾਨਾ[0] = p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;
    ai_Cur_ਖਾਨਾ[1] = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
    ਵਾਪਸ 1;
  }

  ਲਈ(i=0; i<pFrom->nSrc; i++){
    ਸੂਚਕਾਂਕ *pIdx;
    ਲਈ(pIdx=pFrom->a[i].pSTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
      ਜੇ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ ){
        ਵਾਪਸ ਵਿਅੰਜਕ_Might_Be_Indexed2(pFrom,ai_Cur_ਖਾਨਾ,p_ਵਿਅੰਜਕ,i);
      }
    }
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਖਾਲੀ ਵਿਅੰਜਕ_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  ਪੂਰਨ idx_ਪਦ               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  Where_ਪਦ *p_ਪਦ;                /* The term to be analyzed */
  Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ;          /* Set of table index masks */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;                     /* The expression to be analyzed */
  Bitmask prereq_ਖੱਬਾ;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_ਸੱਜਾ = 0;          /* Extra dependencies on LEFT JOIN */
  ਵਿਅੰਜਕ *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  ਪੂਰਨ isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  ਪੂਰਨ noCase = 0;                  /* uppercase equivalent to lowercase */
  ਪੂਰਨ op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  ਪੂਰਨ n_ਖੱਬਾ;                       /* Number of elements on left side vector */

  ਜੇ( db->mallocFailed ){
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pWC->n_ਪਦ > idx_ਪਦ );
  p_ਪਦ = &pWC->a[idx_ਪਦ];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  p_ਪਦ->i_ਪਦ = idx_ਪਦ;
#ਜੇ_ਅੰਤ
  p_ਨਕਾਬ_ਸੈੱਟ = &pWInfo->s_ਨਕਾਬ_ਸੈੱਟ;
  p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 ); /* Because malloc() has not failed */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_AS && p_ਵਿਅੰਜਕ->op!=TK_COLLATE );
ਵਿਅੰਜਕ_Analyze_restart:
  p_ਨਕਾਬ_ਸੈੱਟ->bVarSelect = 0;
  prereq_ਖੱਬਾ = sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
  op = p_ਵਿਅੰਜਕ->op;
  ਜੇ( op==TK_ਵਿੱਚ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਸੱਜਾ==0 );
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Check_ਵਿੱਚ(pParse, p_ਵਿਅੰਜਕ) ) ਵਾਪਸ;
    ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
      p_ਪਦ->prereq_ਸੱਜਾ = ਵਿਅੰਜਕ_Select_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p_ਵਿਅੰਜਕ->x.pSelect);
    }ਨਹੀਂਤਾਂ{
      p_ਪਦ->prereq_ਸੱਜਾ = sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p_ਵਿਅੰਜਕ->x.pList);
    }
    prereqAll = prereq_ਖੱਬਾ | p_ਪਦ->prereq_ਸੱਜਾ;
  }ਨਹੀਂਤਾਂ{
    p_ਪਦ->prereq_ਸੱਜਾ = sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
    ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ==0
     || ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_xIsSelect|EP_If_Null_ਪੰਕਤੀ)
     || p_ਵਿਅੰਜਕ->x.pList!=0
    ){
      prereqAll = sqlite3_Where_ਵਿਅੰਜਕ_Usage_NN(p_ਨਕਾਬ_ਸੈੱਟ, p_ਵਿਅੰਜਕ);
    }ਨਹੀਂਤਾਂ{
      prereqAll = prereq_ਖੱਬਾ | p_ਪਦ->prereq_ਸੱਜਾ;
    }
  }
  ਜੇ( p_ਨਕਾਬ_ਸੈੱਟ->bVarSelect ) p_ਪਦ->wtFlags |= ਪਦ_VARSELECT;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( prereqAll!=sqlite3_Where_ਵਿਅੰਜਕ_Usage_NN(p_ਨਕਾਬ_ਸੈੱਟ, p_ਵਿਅੰਜਕ) ){
    ਲਿਖੋ("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0,p_ਵਿਅੰਜਕ,0);
    ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
  }
#ਜੇ_ਅੰਤ

  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3_Where_Get_ਨਕਾਬ(p_ਨਕਾਬ_ਸੈੱਟ, p_ਵਿਅੰਜਕ->w.iJoin);
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) ){
      prereqAll |= x;
      extra_ਸੱਜਾ = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }ਨਹੀਂਤਾਂ ਜੇ( (prereqAll>>1)>=x ){
      ਵਿਅੰਜਕ_Clear_Property(p_ਵਿਅੰਜਕ, EP_InnerON);
    }
  }
  p_ਪਦ->prereqAll = prereqAll;
  p_ਪਦ->ਖੱਬਾ_Cursor = -1;
  p_ਪਦ->i_ਮਾਪਾ = -1;
  p_ਪਦ->eOperator = 0;
  ਜੇ( allowedOp(op) ){
    ਪੂਰਨ ai_Cur_ਖਾਨਾ[2];
    ਵਿਅੰਜਕ *p_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
    ਵਿਅੰਜਕ *p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
    u16 op_ਨਕਾਬ = (p_ਪਦ->prereq_ਸੱਜਾ & prereq_ਖੱਬਾ)==0 ? WO_ALL : WO_EQUIV;

    ਜੇ( p_ਪਦ->u.x.i_ਖੇਤਰ>0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_ਵਿੱਚ );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ->op==TK_ਸਦਿਸ਼ );
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਖੱਬਾ) );
      p_ਖੱਬਾ = p_ਖੱਬਾ->x.pList->a[p_ਪਦ->u.x.i_ਖੇਤਰ-1].p_ਵਿਅੰਜਕ;
    }

    ਜੇ( ਵਿਅੰਜਕ_Might_Be_ਸੂਚਕਾਂਕਿਤ(pSrc, ai_Cur_ਖਾਨਾ, p_ਖੱਬਾ, op) ){
      p_ਪਦ->ਖੱਬਾ_Cursor = ai_Cur_ਖਾਨਾ[0];
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
      p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ = ai_Cur_ਖਾਨਾ[1];
      p_ਪਦ->eOperator = operator_ਨਕਾਬ(op) & op_ਨਕਾਬ;
    }
    ਜੇ( op==TK_IS ) p_ਪਦ->wtFlags |= ਪਦ_IS;
    ਜੇ( p_ਸੱਜਾ
     && ਵਿਅੰਜਕ_Might_Be_ਸੂਚਕਾਂਕਿਤ(pSrc, ai_Cur_ਖਾਨਾ, p_ਸੱਜਾ, op)
     && !ਵਿਅੰਜਕ_Has_Property(p_ਸੱਜਾ, EP_Fixed_ਖਾਨਾ)
    ){
      Where_ਪਦ *pNew;
      ਵਿਅੰਜਕ *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->u.x.i_ਖੇਤਰ==0 );
      ਜੇ( p_ਪਦ->ਖੱਬਾ_Cursor>=0 ){
        ਪੂਰਨ idxNew;
        pDup = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ, 0);
        ਜੇ( db->mallocFailed ){
          sqlite3_ਵਿਅੰਜਕ_Delete(db, pDup);
          ਵਾਪਸ;
        }
        idxNew = where_Clause_ਪਾਓ(pWC, pDup, ਪਦ_VIRTUAL|ਪਦ_DYNAMIC);
        ਜੇ( idxNew==0 ) ਵਾਪਸ;
        pNew = &pWC->a[idxNew];
        mark_ਪਦ_As_ਬੱਚਾ(pWC, idxNew, idx_ਪਦ);
        ਜੇ( op==TK_IS ) pNew->wtFlags |= ਪਦ_IS;
        p_ਪਦ = &pWC->a[idx_ਪਦ];
        p_ਪਦ->wtFlags |= ਪਦ_COPIED;
        ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->pTabList!=0 );
        ਜੇ( ਪਦ_Is_Equivalence(pParse, pDup, pWInfo->pTabList) ){
          p_ਪਦ->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }ਨਹੀਂਤਾਂ{
        pDup = p_ਵਿਅੰਜਕ;
        pNew = p_ਪਦ;
      }
      pNew->wtFlags |= ਵਿਅੰਜਕ_Commute(pParse, pDup);
      pNew->ਖੱਬਾ_Cursor = ai_Cur_ਖਾਨਾ[0];
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.ਖੱਬਾ_ਕਾਲਮ = ai_Cur_ਖਾਨਾ[1];
      testcase( (prereq_ਖੱਬਾ | extra_ਸੱਜਾ) != prereq_ਖੱਬਾ );
      pNew->prereq_ਸੱਜਾ = prereq_ਖੱਬਾ | extra_ਸੱਜਾ;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operator_ਨਕਾਬ(pDup->op) + eExtraOp) & op_ਨਕਾਬ;
    }ਨਹੀਂਤਾਂ
    ਜੇ( op==TK_ISNULL
     && !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_OuterON)
     && 0==sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(p_ਖੱਬਾ)
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      p_ਵਿਅੰਜਕ->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_ਵਿਅੰਜਕ->u.zToken = "false";
      ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_IsFalse);
      p_ਪਦ->prereqAll = 0;
      p_ਪਦ->eOperator = 0;
    }
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_BETWEEN_OPTIMIZATION
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
  ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_BETWEEN && pWC->op==TK_AND ){
    ਵਿਅੰਜਕ_List *pList;
    ਪੂਰਨ i;
    ਸਥਿਰ ਅਚਲ u8 ops[] = {TK_GE, TK_LE};
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
    pList = p_ਵਿਅੰਜਕ->x.pList;
    ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pList->n_ਵਿਅੰਜਕ==2 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pWC->a[idx_ਪਦ].n_ਬੱਚਾ==0 );
    ਲਈ(i=0; i<2; i++){
      ਵਿਅੰਜਕ *p_New_ਵਿਅੰਜਕ;
      ਪੂਰਨ idxNew;
      p_New_ਵਿਅੰਜਕ = sqlite3PExpr(pParse, ops[i],
                             sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0),
                             sqlite3_ਵਿਅੰਜਕ_Dup(db, pList->a[i].p_ਵਿਅੰਜਕ, 0));
      transferJoinMarkings(p_New_ਵਿਅੰਜਕ, p_ਵਿਅੰਜਕ);
      idxNew = where_Clause_ਪਾਓ(pWC, p_New_ਵਿਅੰਜਕ, ਪਦ_VIRTUAL|ਪਦ_DYNAMIC);
      testcase( idxNew==0 );
      ਵਿਅੰਜਕ_Analyze(pSrc, pWC, idxNew);
      p_ਪਦ = &pWC->a[idx_ਪਦ];
      mark_ਪਦ_As_ਬੱਚਾ(pWC, idxNew, idx_ਪਦ);
    }
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#ਜੇ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_OR && !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Collate) ){
    ਵਿਅੰਜਕ *pAlt = sqlite3_ਵਿਅੰਜਕ_Simplified_And_Or(p_ਵਿਅੰਜਕ);
    ਜੇ( pAlt!=p_ਵਿਅੰਜਕ ){
      p_ਪਦ->p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(pAlt);
      ਜਾਓ ਵਿਅੰਜਕ_Analyze_restart;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pWC->op==TK_AND );
    ਵਿਅੰਜਕ_Analyze_Or_ਪਦ(pSrc, pWC, idx_ਪਦ);
    p_ਪਦ = &pWC->a[idx_ਪਦ];
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_NOTNULL ){
    ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਕਾਲਮ
     && p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->i_ਕਾਲਮ>=0
     && !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON)
    ){
      ਵਿਅੰਜਕ *p_New_ਵਿਅੰਜਕ;
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
      ਪੂਰਨ idxNew;
      Where_ਪਦ *p_New_ਪਦ;
 
      p_New_ਵਿਅੰਜਕ = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਖੱਬਾ, 0),
                              sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_ਪਾਓ(pWC, p_New_ਵਿਅੰਜਕ,
                                ਪਦ_VIRTUAL|ਪਦ_DYNAMIC|ਪਦ_VNULL);
      ਜੇ( idxNew ){
        p_New_ਪਦ = &pWC->a[idxNew];
        p_New_ਪਦ->prereq_ਸੱਜਾ = 0;
        p_New_ਪਦ->ਖੱਬਾ_Cursor = p_ਖੱਬਾ->i_ਸਾਰਣੀ;
        p_New_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ = p_ਖੱਬਾ->i_ਕਾਲਮ;
        p_New_ਪਦ->eOperator = WO_GT;
        mark_ਪਦ_As_ਬੱਚਾ(pWC, idxNew, idx_ਪਦ);
        p_ਪਦ = &pWC->a[idx_ਪਦ];
        p_ਪਦ->wtFlags |= ਪਦ_COPIED;
        p_New_ਪਦ->prereqAll = p_ਪਦ->prereqAll;
      }
    }
  }


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_LIKE_OPTIMIZATION
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
  ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_ਵਿਅੰਜਕ, &pStr1, &isComplete, &noCase)
  ){
    ਵਿਅੰਜਕ *p_ਖੱਬਾ;       /* LHS of LIKE/GLOB operator */
    ਵਿਅੰਜਕ *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    ਵਿਅੰਜਕ *pNewExpr1;
    ਵਿਅੰਜਕ *pNewExpr2;
    ਪੂਰਨ idxNew1;
    ਪੂਰਨ idxNew2;
    ਅਚਲ ਅੱਖਰ *zCollSeqName;     /* Name of collating sequence */
    ਅਚਲ u16 wtFlags = ਪਦ_LIKEOPT | ਪਦ_VIRTUAL | ਪਦ_DYNAMIC;

    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
    p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->x.pList->a[1].p_ਵਿਅੰਜਕ;
    pStr2 = sqlite3_ਵਿਅੰਜਕ_Dup(db, pStr1, 0);
    ਪ੍ਰਤਿਜ੍ਞਾ( pStr1==0 || !ਵਿਅੰਜਕ_Has_Property(pStr1, EP_Int_ਮੁੱਲ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( pStr2==0 || !ਵਿਅੰਜਕ_Has_Property(pStr2, EP_Int_ਮੁੱਲ) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    ਜੇ( noCase && !pParse->db->mallocFailed ){
      ਪੂਰਨ i;
      ਅੱਖਰ c;
      p_ਪਦ->wtFlags |= ਪਦ_LIKE;
      ਲਈ(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    ਜੇ( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      ਜੇ( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        ਜੇ( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      ਜਦੋਂ( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ਦੁਆਧਾਰੀ;
    pNewExpr1 = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਖੱਬਾ, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_ਵਿਅੰਜਕ);
    idxNew1 = where_Clause_ਪਾਓ(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਖੱਬਾ, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_ਵਿਅੰਜਕ);
    idxNew2 = where_Clause_ਪਾਓ(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    ਵਿਅੰਜਕ_Analyze(pSrc, pWC, idxNew1);
    ਵਿਅੰਜਕ_Analyze(pSrc, pWC, idxNew2);
    p_ਪਦ = &pWC->a[idx_ਪਦ];
    ਜੇ( isComplete ){
      mark_ਪਦ_As_ਬੱਚਾ(pWC, idxNew1, idx_ਪਦ);
      mark_ਪਦ_As_ਬੱਚਾ(pWC, idxNew2, idx_ਪਦ);
    }
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LIKE_OPTIMIZATION */

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
  ਜੇ( (p_ਵਿਅੰਜਕ->op==TK_EQ || p_ਵਿਅੰਜਕ->op==TK_IS)
   && (n_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ))>1
   && sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ)==n_ਖੱਬਾ
   && ( (p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->flags & EP_xIsSelect)==0
     || (p_ਵਿਅੰਜਕ->p_ਸੱਜਾ->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<n_ਖੱਬਾ; i++){
      ਪੂਰਨ idxNew;
      ਵਿਅੰਜਕ *pNew;
      ਵਿਅੰਜਕ *p_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_For_ਸਦਿਸ਼_ਖੇਤਰ(pParse, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, i, n_ਖੱਬਾ);
      ਵਿਅੰਜਕ *p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_For_ਸਦਿਸ਼_ਖੇਤਰ(pParse, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, i, n_ਖੱਬਾ);

      pNew = sqlite3PExpr(pParse, p_ਵਿਅੰਜਕ->op, p_ਖੱਬਾ, p_ਸੱਜਾ);
      transferJoinMarkings(pNew, p_ਵਿਅੰਜਕ);
      idxNew = where_Clause_ਪਾਓ(pWC, pNew, ਪਦ_DYNAMIC|ਪਦ_ਟੁਕੜਾ);
      ਵਿਅੰਜਕ_Analyze(pSrc, pWC, idxNew);
    }
    p_ਪਦ = &pWC->a[idx_ਪਦ];
    p_ਪਦ->wtFlags |= ਪਦ_CODED|ਪਦ_VIRTUAL;  /* Disable the original */
    p_ਪਦ->eOperator = WO_ROWVAL;
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
  ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਵਿੱਚ
   && p_ਪਦ->u.x.i_ਖੇਤਰ==0
   && p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਸਦਿਸ਼
   && ALWAYS( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) )
   && (p_ਵਿਅੰਜਕ->x.pSelect->p_ਪੂਰਵ==0 || (p_ਵਿਅੰਜਕ->x.pSelect->selFlags & SF_ਮੁੱਲਾਂ))
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
   && p_ਵਿਅੰਜਕ->x.pSelect->pWin==0
#ਜੇ_ਅੰਤ
   && (p_ਵਿਅੰਜਕ->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_ਵਿਅੰਜਕ->x.pSelect->pEList->n_ਵਿਅੰਜਕ <= UMXV(p_ਪਦ->n_ਬੱਚਾ)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    ਪੂਰਨ i;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->n_ਬੱਚਾ==0 );
    ਲਈ(i=0; i<sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ); i++){
      ਪੂਰਨ idxNew;
      idxNew = where_Clause_ਪਾਓ(pWC, p_ਵਿਅੰਜਕ, ਪਦ_VIRTUAL|ਪਦ_ਟੁਕੜਾ);
      pWC->a[idxNew].u.x.i_ਖੇਤਰ = i+1;
      ਵਿਅੰਜਕ_Analyze(pSrc, pWC, idxNew);
      mark_ਪਦ_As_ਬੱਚਾ(pWC, idxNew, idx_ਪਦ);
    }
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  /* Add a WO_AUX auxiliary term to the constraint set if the
  ** current expression is of the form "column OP expr" where OP
  ** is an operator that gets passed into virtual tables but which is
  ** not normally optimized for ordinary tables.  In other words, OP
  ** is one of MATCH, LIKE, GLOB, REGEXP, !=, IS, IS NOT, or NOT NULL.
  ** This information is used by the xBestIndex methods of
  ** virtual tables.  The native query optimizer does not attempt
  ** to do anything with MATCH functions.
  */
  ਨਹੀਂਤਾਂ ਜੇ( pWC->op==TK_AND ){
    ਵਿਅੰਜਕ *p_ਸੱਜਾ = 0, *p_ਖੱਬਾ = 0;
    ਪੂਰਨ res = isAuxiliaryVtabOperator(db, p_ਵਿਅੰਜਕ, &eOp2, &p_ਖੱਬਾ, &p_ਸੱਜਾ);
    ਜਦੋਂ( res-- > 0 ){
      ਪੂਰਨ idxNew;
      Where_ਪਦ *p_New_ਪਦ;
      Bitmask prereq_ਕਾਲਮ, prereq_ਵਿਅੰਜਕ;

      prereq_ਵਿਅੰਜਕ = sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p_ਸੱਜਾ);
      prereq_ਕਾਲਮ = sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p_ਖੱਬਾ);
      ਜੇ( (prereq_ਵਿਅੰਜਕ & prereq_ਕਾਲਮ)==0 ){
        ਵਿਅੰਜਕ *p_New_ਵਿਅੰਜਕ;
        p_New_ਵਿਅੰਜਕ = sqlite3PExpr(pParse, TK_ਮੇਲ,
            0, sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਸੱਜਾ, 0));
        ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) && p_New_ਵਿਅੰਜਕ ){
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_New_ਵਿਅੰਜਕ, EP_OuterON);
          p_New_ਵਿਅੰਜਕ->w.iJoin = p_ਵਿਅੰਜਕ->w.iJoin;
        }
        idxNew = where_Clause_ਪਾਓ(pWC, p_New_ਵਿਅੰਜਕ, ਪਦ_VIRTUAL|ਪਦ_DYNAMIC);
        testcase( idxNew==0 );
        p_New_ਪਦ = &pWC->a[idxNew];
        p_New_ਪਦ->prereq_ਸੱਜਾ = prereq_ਵਿਅੰਜਕ | extra_ਸੱਜਾ;
        p_New_ਪਦ->ਖੱਬਾ_Cursor = p_ਖੱਬਾ->i_ਸਾਰਣੀ;
        p_New_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ = p_ਖੱਬਾ->i_ਕਾਲਮ;
        p_New_ਪਦ->eOperator = WO_AUX;
        p_New_ਪਦ->e_ਮੇਲ_Op = eOp2;
        mark_ਪਦ_As_ਬੱਚਾ(pWC, idxNew, idx_ਪਦ);
        p_ਪਦ = &pWC->a[idx_ਪਦ];
        p_ਪਦ->wtFlags |= ਪਦ_COPIED;
        p_New_ਪਦ->prereqAll = p_ਪਦ->prereqAll;
      }
      ਅਦਲਾ_ਬਦਲੀ(ਵਿਅੰਜਕ*, p_ਖੱਬਾ, p_ਸੱਜਾ);
    }
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( p_ਪਦ!=&pWC->a[idx_ਪਦ] );
  p_ਪਦ = &pWC->a[idx_ਪਦ];
  p_ਪਦ->prereq_ਸੱਜਾ |= extra_ਸੱਜਾ;
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
ਖਾਲੀ sqlite3_Where_ਵੰਡੋ(WhereClause *pWC, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, u8 op){
  ਵਿਅੰਜਕ *pE2 = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਵਿਅੰਜਕ);
  pWC->op = op;
  ਪ੍ਰਤਿਜ੍ਞਾ( pE2!=0 || p_ਵਿਅੰਜਕ==0 );
  ਜੇ( pE2==0 ) ਵਾਪਸ;
  ਜੇ( pE2->op!=op ){
    where_Clause_ਪਾਓ(pWC, p_ਵਿਅੰਜਕ, 0);
  }ਨਹੀਂਤਾਂ{
    sqlite3_Where_ਵੰਡੋ(pWC, pE2->p_ਖੱਬਾ, op);
    sqlite3_Where_ਵੰਡੋ(pWC, pE2->p_ਸੱਜਾ, op);
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
ਸਥਿਰ ਖਾਲੀ where_ਜੋੜੋ_ਹੱਦਬੰਦੀ_ਵਿਅੰਜਕ(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  ਪੂਰਨ iReg,           /* Register that will hold value of the limit/offset */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,        /* Expression that defines the limit/offset */
  ਪੂਰਨ iCsr,           /* Cursor to which the constraint applies */
  ਪੂਰਨ e_ਮੇਲ_Op        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  ਵਿਅੰਜਕ *pNew;
  ਪੂਰਨ iVal = 0;

  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Integer(p_ਵਿਅੰਜਕ, &iVal, pParse, 0) && iVal>=0 ){
    ਵਿਅੰਜਕ *pVal = sqlite3_ਵਿਅੰਜਕ_Int32(db, iVal);
    ਜੇ( pVal==0 ) ਵਾਪਸ;
    pNew = sqlite3PExpr(pParse, TK_ਮੇਲ, 0, pVal);
  }ਨਹੀਂਤਾਂ{
    ਵਿਅੰਜਕ *pVal = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ਰਜਿਸਟਰ, 0, 0);
    ਜੇ( pVal==0 ) ਵਾਪਸ;
    pVal->i_ਸਾਰਣੀ = iReg;
    pNew = sqlite3PExpr(pParse, TK_ਮੇਲ, 0, pVal);
  }
  ਜੇ( pNew ){
    Where_ਪਦ *p_ਪਦ;
    ਪੂਰਨ idx;
    idx = where_Clause_ਪਾਓ(pWC, pNew, ਪਦ_DYNAMIC|ਪਦ_VIRTUAL);
    p_ਪਦ = &pWC->a[idx];
    p_ਪਦ->ਖੱਬਾ_Cursor = iCsr;
    p_ਪਦ->eOperator = WO_AUX;
    p_ਪਦ->e_ਮੇਲ_Op = e_ਮੇਲ_Op;
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
ਖਾਲੀ SQLITE_NOINLINE sqlite3_Where_ਜੋੜੋ_ਹੱਦਬੰਦੀ(WhereClause *pWC, Select *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 && p->p_ਹੱਦਬੰਦੀ!=0 );                 /* 1 -- checked by caller */
  ਜੇ( p->p_ਸਮੂਹ_By==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By = p->p_ਕ੍ਰਮ_By;
    ਪੂਰਨ iCsr = p->pSrc->a[0].iCursor;
    ਪੂਰਨ ii;

    /* Check condition (4). Return early if it is not met. */
    ਲਈ(ii=0; ii<pWC->n_ਪਦ; ii++){
      ਜੇ( pWC->a[ii].wtFlags & ਪਦ_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        ਪ੍ਰਤਿਜ੍ਞਾ( pWC->a[ii].wtFlags & ਪਦ_VIRTUAL );
        ਪ੍ਰਤਿਜ੍ਞਾ( pWC->a[ii].eOperator==WO_ROWVAL );
        ਜਾਰੀ;
      }
      ਜੇ( pWC->a[ii].n_ਬੱਚਾ ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        ਜਾਰੀ;
      }
      ਜੇ( pWC->a[ii].ਖੱਬਾ_Cursor==iCsr && pWC->a[ii].prereq_ਸੱਜਾ==0 ) ਜਾਰੀ;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      ਜੇ( pWC->a[ii].i_ਮਾਪਾ>=0 ){
        Where_ਪਦ *p_ਮਾਪਾ = &pWC->a[ pWC->a[ii].i_ਮਾਪਾ ];
        ਜੇ( p_ਮਾਪਾ->ਖੱਬਾ_Cursor==iCsr
         && p_ਮਾਪਾ->prereq_ਸੱਜਾ==0
         && p_ਮਾਪਾ->n_ਬੱਚਾ==1
        ){
          ਜਾਰੀ;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      ਵਾਪਸ;
    }

    /* Check condition (5). Return early if it is not met. */
    ਜੇ( p_ਕ੍ਰਮ_By ){
      ਲਈ(ii=0; ii<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; ii++){
        ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਕ੍ਰਮ_By->a[ii].p_ਵਿਅੰਜਕ;
        ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_ਕਾਲਮ ) ਵਾਪਸ;
        ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ!=iCsr ) ਵਾਪਸ;
        ਜੇ( p_ਕ੍ਰਮ_By->a[ii].fg.ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_BIGNULL ) ਵਾਪਸ;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਹੱਦਬੰਦੀ->op==TK_ਹੱਦਬੰਦੀ );
    ਜੇ( p->i_ਵਿੱਥ!=0 && (p->selFlags & SF_Compound)==0 ){
      where_ਜੋੜੋ_ਹੱਦਬੰਦੀ_ਵਿਅੰਜਕ(pWC, p->i_ਵਿੱਥ, p->p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ,
                        iCsr, SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਵਿੱਥ);
    }
    ਜੇ( p->i_ਵਿੱਥ==0 || (p->selFlags & SF_Compound)==0 ){
      where_ਜੋੜੋ_ਹੱਦਬੰਦੀ_ਵਿਅੰਜਕ(pWC, p->i_ਹੱਦਬੰਦੀ, p->p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ,
                        iCsr, SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਹੱਦਬੰਦੀ);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
ਖਾਲੀ sqlite3WhereClauseInit(
  WhereClause *pWC,        /* The WhereClause to be initialized */
  WhereInfo *pWInfo        /* The WHERE processing context */
){
  pWC->pWInfo = pWInfo;
  pWC->hasOr = 0;
  pWC->pOuter = 0;
  pWC->n_ਪਦ = 0;
  pWC->n_ਅਧਾਰ = 0;
  pWC->n_ਥਾਂ = Array_ਮਾਪ(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
ਖਾਲੀ sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( pWC->n_ਪਦ>=pWC->n_ਅਧਾਰ );
  ਜੇ( pWC->n_ਪਦ>0 ){
    Where_ਪਦ *a = pWC->a;
    Where_ਪਦ *a_ਆਖਰੀ = &pWC->a[pWC->n_ਪਦ-1];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    ਪੂਰਨ i;
    /* Verify that every term past pWC->nBase is virtual */
    ਲਈ(i=pWC->n_ਅਧਾਰ; i<pWC->n_ਪਦ; i++){
      ਪ੍ਰਤਿਜ੍ਞਾ( (pWC->a[i].wtFlags & ਪਦ_VIRTUAL)!=0 );
    }
#ਜੇ_ਅੰਤ
    ਜਦੋਂ(1){
      ਪ੍ਰਤਿਜ੍ਞਾ( a->e_ਮੇਲ_Op==0 || a->eOperator==WO_AUX );
      ਜੇ( a->wtFlags & ਪਦ_DYNAMIC ){
        sqlite3_ਵਿਅੰਜਕ_Delete(db, a->p_ਵਿਅੰਜਕ);
      }
      ਜੇ( a->wtFlags & (ਪਦ_ORINFO|ਪਦ_ANDINFO) ){
        ਜੇ( a->wtFlags & ਪਦ_ORINFO ){
          ਪ੍ਰਤਿਜ੍ਞਾ( (a->wtFlags & ਪਦ_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }ਨਹੀਂਤਾਂ{
          ਪ੍ਰਤਿਜ੍ਞਾ( (a->wtFlags & ਪਦ_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      ਜੇ( a==a_ਆਖਰੀ ) ਰੋਕੋ;
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
ਸਥਿਰ SQLITE_NOINLINE Bitmask sqlite3_Where_ਵਿਅੰਜਕ_Usage_ਭਰਿਆ(
  Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ,
  ਵਿਅੰਜਕ *p
){
  Bitmask ਨਕਾਬ;
  ਨਕਾਬ = (p->op==TK_IF_NULL_ਪੰਕਤੀ) ? sqlite3_Where_Get_ਨਕਾਬ(p_ਨਕਾਬ_ਸੈੱਟ, p->i_ਸਾਰਣੀ) : 0;
  ਜੇ( p->p_ਖੱਬਾ ) ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_Usage_NN(p_ਨਕਾਬ_ਸੈੱਟ, p->p_ਖੱਬਾ);
  ਜੇ( p->p_ਸੱਜਾ ){
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_Usage_NN(p_ਨਕਾਬ_ਸੈੱਟ, p->p_ਸੱਜਾ);
    ਪ੍ਰਤਿਜ੍ਞਾ( p->x.pList==0 );
  }ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p) ){
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p, EP_VarSelect) ) p_ਨਕਾਬ_ਸੈੱਟ->bVarSelect = 1;
    ਨਕਾਬ |= ਵਿਅੰਜਕ_Select_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p->x.pSelect);
  }ਨਹੀਂਤਾਂ ਜੇ( p->x.pList ){
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p->x.pList);
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਜੇ( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && ਵਿਅੰਜਕ_Use_YWin(p) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->y.pWin!=0 );
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p->y.pWin->p_ਹਿੱਸੇ_ਕਰੋ);
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p->y.pWin->p_ਕ੍ਰਮ_By);
    ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, p->y.pWin->pFilter);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ ਨਕਾਬ;
}
Bitmask sqlite3_Where_ਵਿਅੰਜਕ_Usage_NN(Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ, ਵਿਅੰਜਕ *p){
  ਜੇ( p->op==TK_ਕਾਲਮ && !ਵਿਅੰਜਕ_Has_Property(p, EP_Fixed_ਖਾਨਾ) ){
    ਵਾਪਸ sqlite3_Where_Get_ਨਕਾਬ(p_ਨਕਾਬ_ਸੈੱਟ, p->i_ਸਾਰਣੀ);
  }ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Has_Property(p, EP_TokenOnly|EP_ਪੱਤੀ) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->op!=TK_IF_NULL_ਪੰਕਤੀ );
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ sqlite3_Where_ਵਿਅੰਜਕ_Usage_ਭਰਿਆ(p_ਨਕਾਬ_ਸੈੱਟ, p);
}
Bitmask sqlite3_Where_ਵਿਅੰਜਕ_Usage(Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ, ਵਿਅੰਜਕ *p){
  ਵਾਪਸ p ? sqlite3_Where_ਵਿਅੰਜਕ_Usage_NN(p_ਨਕਾਬ_ਸੈੱਟ,p) : 0;
}
Bitmask sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(Where_ਨਕਾਬ_ਸੈੱਟ *p_ਨਕਾਬ_ਸੈੱਟ, ਵਿਅੰਜਕ_List *pList){
  ਪੂਰਨ i;
  Bitmask ਨਕਾਬ = 0;
  ਜੇ( pList ){
    ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
      ਨਕਾਬ |= sqlite3_Where_ਵਿਅੰਜਕ_Usage(p_ਨਕਾਬ_ਸੈੱਟ, pList->a[i].p_ਵਿਅੰਜਕ);
    }
  }
  ਵਾਪਸ ਨਕਾਬ;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
ਖਾਲੀ sqlite3_Where_ਵਿਅੰਜਕ_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  ਪੂਰਨ i;
  ਲਈ(i=pWC->n_ਪਦ-1; i>=0; i--){
    ਵਿਅੰਜਕ_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
ਖਾਲੀ sqlite3_Where_Tab_Func_ਦਲੀਲਾਂ(
  Parse *pParse,                    /* Parsing context */
  Src_ਵਸਤੂ *p_ਵਸਤੂ,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  ਸਾਰਣੀ *pTab;
  ਪੂਰਨ j, k;
  ਵਿਅੰਜਕ_List *p_ਦਲੀਲਾਂ;
  ਵਿਅੰਜਕ *p_ਖਾਨਾ_Ref;
  ਵਿਅੰਜਕ *p_ਪਦ;
  ਜੇ( p_ਵਸਤੂ->fg.isTabFunc==0 ) ਵਾਪਸ;
  pTab = p_ਵਸਤੂ->pSTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਜੇ( !IsVirtual(pTab) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "'%s' is not a function", p_ਵਸਤੂ->zName);
    ਵਾਪਸ;
  }
  p_ਦਲੀਲਾਂ = p_ਵਸਤੂ->u1.pFuncArg;
  ਜੇ( p_ਦਲੀਲਾਂ==0 ) ਵਾਪਸ;
  ਲਈ(j=k=0; j<p_ਦਲੀਲਾਂ->n_ਵਿਅੰਜਕ; j++){
    ਵਿਅੰਜਕ *pRhs;
    u32 joinType;
    ਜਦੋਂ( k<pTab->n_ਖਾਨਾ && (pTab->a_ਖਾਨਾ[k].ਖਾਨਾ_Flags & COLFLAG_HIDDEN)==0 ){k++;}
    ਜੇ( k>=pTab->n_ਖਾਨਾ ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      ਵਾਪਸ;
    }
    p_ਖਾਨਾ_Ref = sqlite3_ਵਿਅੰਜਕ_Alloc(pParse->db, TK_ਕਾਲਮ, 0, 0);
    ਜੇ( p_ਖਾਨਾ_Ref==0 ) ਵਾਪਸ;
    p_ਖਾਨਾ_Ref->i_ਸਾਰਣੀ = p_ਵਸਤੂ->iCursor;
    p_ਖਾਨਾ_Ref->i_ਕਾਲਮ = k++;
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਖਾਨਾ_Ref) );
    p_ਖਾਨਾ_Ref->y.pTab = pTab;
    p_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ |= sqlite3_ਵਿਅੰਜਕ_ਖਾਨਾ_ਵਰਤਿਆ(p_ਖਾਨਾ_Ref);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_ਵਿਅੰਜਕ_Dup(pParse->db, p_ਦਲੀਲਾਂ->a[j].p_ਵਿਅੰਜਕ, 0), 0);
    p_ਪਦ = sqlite3PExpr(pParse, TK_EQ, p_ਖਾਨਾ_Ref, pRhs);
    ਜੇ( p_ਵਸਤੂ->fg.jointype & (JT_ਖੱਬਾ|JT_ਸੱਜਾ) ){
      testcase( p_ਵਸਤੂ->fg.jointype & JT_ਖੱਬਾ );  /* testtag-20230227a */
      testcase( p_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }ਨਹੀਂਤਾਂ{
      testcase( p_ਵਸਤੂ->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_ਸੈੱਟ_Join_ਵਿਅੰਜਕ(p_ਪਦ, p_ਵਸਤੂ->iCursor, joinType);
    where_Clause_ਪਾਓ(pWC, p_ਪਦ, ਪਦ_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
ਪੂਰਨ sqlite3WhereLoopBloomable(ਅਚਲ WhereLoop *pLoop){
  ਜੇ( pLoop->wsFlags & WHERE_IPK ) ਵਾਪਸ 1;
  ਜੇ( NEVER((pLoop->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ)==0) ) ਵਾਪਸ 0;
  ਵਾਪਸ sqlite3_ਸੂਚਕਾਂਕ_Bloomable(pLoop->u.btree.p_ਸੂਚਕਾਂਕ, pLoop->u.btree.nEq);
}
