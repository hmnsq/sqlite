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
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_Analyze(SrcList*, WhereClause*, ಪೂರ್ಣಾಂಕ);

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ where_Clause_ಸೇರಿಸು(WhereClause *pWC, ಅಭಿವ್ಯಕ್ತಿ *p, u16 wtFlags){
  Where_ಪದ *p_ಪದ;
  ಪೂರ್ಣಾಂಕ idx;
  testcase( wtFlags & ಪದ_VIRTUAL );
  ಆದರೆ( pWC->n_ಪದ>=pWC->n_ಸ್ಥಾನ ){
    Where_ಪದ *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, ಗಾತ್ರ(pWC->a[0])*pWC->n_ಸ್ಥಾನ*2 );
    ಆದರೆ( pWC->a==0 ){
      ಆದರೆ( wtFlags & ಪದ_DYNAMIC ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p);
      }
      pWC->a = pOld;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಮರಣೆ_ನಕಲು(pWC->a, pOld, ಗಾತ್ರ(pWC->a[0])*pWC->n_ಪದ);
    pWC->n_ಸ್ಥಾನ = pWC->n_ಸ್ಥಾನ*2;
  }
  p_ಪದ = &pWC->a[idx = pWC->n_ಪದ++];
  ಆದರೆ( (wtFlags & ಪದ_VIRTUAL)==0 ) pWC->n_ತಳ = pWC->n_ಪದ;
  ಆದರೆ( p && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Unlikely) ){
    p_ಪದ->truthProb = sqlite3LogEst(p->i_ಕೋಷ್ಟಕ) - 270;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಪದ->truthProb = 1;
  }
  p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p);
  p_ಪದ->wtFlags = wtFlags;
  p_ಪದ->pWC = pWC;
  p_ಪದ->i_ಪೋಷಕ = -1;
  memset(&p_ಪದ->eOperator, 0,
         ಗಾತ್ರ(Where_ಪದ) - offsetof(Where_ಪದ,eOperator));
  ಹಿಂತಿರುಗಿ idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ allowedOp(ಪೂರ್ಣಾಂಕ op){
  ಪ್ರತಿಜ್ಞಾ( TK_GT>TK_EQ && TK_GT<TK_GE );
  ಪ್ರತಿಜ್ಞಾ( TK_LT>TK_EQ && TK_LT<TK_GE );
  ಪ್ರತಿಜ್ಞಾ( TK_LE>TK_EQ && TK_LE<TK_GE );
  ಪ್ರತಿಜ್ಞಾ( TK_GE==TK_EQ+4 );
  ಪ್ರತಿಜ್ಞಾ( TK_ಒಳ<TK_EQ );
  ಪ್ರತಿಜ್ಞಾ( TK_IS<TK_EQ );
  ಪ್ರತಿಜ್ಞಾ( TK_ISNULL<TK_EQ );
  ಆದರೆ( op>TK_GE ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( op>=TK_EQ ) ಹಿಂತಿರುಗಿ 1;
  ಹಿಂತಿರುಗಿ op==TK_ಒಳ || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ಸ್ಥಿರ u16 ಅಭಿವ್ಯಕ್ತಿ_Commute(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ಸದಿಶ
   || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ->op==TK_ಸದಿಶ
   || sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ) !=
      sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ)
  ){
    p_ಅಭಿವ್ಯಕ್ತಿ->flags ^= EP_Commuted;
  }
  ಅದಲುಬದಲು(ಅಭಿವ್ಯಕ್ತಿ*,p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ,p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op>=TK_GT ){
    ಪ್ರತಿಜ್ಞಾ( TK_LT==TK_GT+2 );
    ಪ್ರತಿಜ್ಞಾ( TK_GE==TK_LE+2 );
    ಪ್ರತಿಜ್ಞಾ( TK_GT>TK_EQ );
    ಪ್ರತಿಜ್ಞಾ( TK_GT<TK_LE );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op>=TK_GT && p_ಅಭಿವ್ಯಕ್ತಿ->op<=TK_GE );
    p_ಅಭಿವ್ಯಕ್ತಿ->op = ((p_ಅಭಿವ್ಯಕ್ತಿ->op-TK_GT)^2)+TK_GT;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ಸ್ಥಿರ u16 operator_ಮುಖವಾಡ(ಪೂರ್ಣಾಂಕ op){
  u16 c;
  ಪ್ರತಿಜ್ಞಾ( allowedOp(op) );
  ಆದರೆ( op>=TK_EQ ){
    ಪ್ರತಿಜ್ಞಾ( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_ಒಳ ){
    c = WO_ಒಳ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_ISNULL ){
    c = WO_ISNULL;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( op==TK_IS );
    c = WO_IS;
  }
  ಪ್ರತಿಜ್ಞಾ( op!=TK_ISNULL || c==WO_ISNULL );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_ಒಳ || c==WO_ಒಳ );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_EQ || c==WO_EQ );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_LT || c==WO_LT );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_LE || c==WO_LE );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_GT || c==WO_GT );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_GE || c==WO_GE );
  ಪ್ರತಿಜ್ಞಾ( op!=TK_IS || c==WO_IS );
  ಹಿಂತಿರುಗಿ c;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_LIKE_OPTIMIZATION
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,      /* Test this expression */
  ಅಭಿವ್ಯಕ್ತಿ **pp_ಪೂರ್ವಪ್ರತ್ಯಯ,  /* Pointer to TK_STRING expression with pattern prefix */
  ಪೂರ್ಣಾಂಕ *pisComplete, /* True if the only wildcard is % in the last character */
  ಪೂರ್ಣಾಂಕ *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ಸ್ಥಿರಾಂಕ u8 *z = 0;           /* String on RHS of LIKE operator */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ, *p_ಎಡ;      /* Right and left size of LIKE operator */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  ಪೂರ್ಣಾಂಕ cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_ಮೌಲ್ಯ *pVal = 0;
  ಪೂರ್ಣಾಂಕ op;                    /* Opcode of pRight */
  ಪೂರ್ಣಾಂಕ rc;                    /* Result code to return */

  ಆದರೆ( !sqlite3IsLikeFunction(db, p_ಅಭಿವ್ಯಕ್ತಿ, pnoCase, (ಅಕ್ಷರ*)wc) ){
    ಹಿಂತಿರುಗಿ 0;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
  ಆದರೆ( *pnoCase ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
  p_ಎಡ = pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;

  p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
  op = p_ಬಲ->op;
  ಆದರೆ( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = p_ಬಲ->i_ಸ್ತಂಭ;
    pVal = sqlite3_Vdbe_Get_Bound_ಮೌಲ್ಯ(pReprepare, i_ಲಂಬಸಾಲು, SQLITE_AFF_BLOB);
    ಆದರೆ( pVal && sqlite3_ಮೌಲ್ಯ_type(pVal)==SQLITE_ಪಠ್ಯ ){
      z = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, i_ಲಂಬಸಾಲು, 0);
    ಪ್ರತಿಜ್ಞಾ( p_ಬಲ->op==TK_VARIABLE || p_ಬಲ->op==TK_ರಿಜಿಸ್ಟರ್ );
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_STRING ){
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ, EP_Int_ಮೌಲ್ಯ) );
     z = (u8*)p_ಬಲ->u.zToken;
  }
  ಆದರೆ( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    ಆಗ( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      ಆದರೆ( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c>=0x80 ){
        ಸ್ಥಿರಾಂಕ u8 *z2 = z+cnt-1;
        ಆದರೆ( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ{
          cnt = (ಪೂರ್ಣಾಂಕ)(z2-z);
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
    ಆದರೆ( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಪೂರ್ವಪ್ರತ್ಯಯ;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      p_ಪೂರ್ವಪ್ರತ್ಯಯ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_STRING, (ಅಕ್ಷರ*)z);
      ಆದರೆ( p_ಪೂರ್ವಪ್ರತ್ಯಯ ){
        ಪೂರ್ಣಾಂಕ iFrom, i_ಗಮ್ಯ;
        ಅಕ್ಷರ *zNew;
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಪೂರ್ವಪ್ರತ್ಯಯ, EP_Int_ಮೌಲ್ಯ) );
        zNew = p_ಪೂರ್ವಪ್ರತ್ಯಯ->u.zToken;
        zNew[cnt] = 0;
        ಗಾಗಿ(iFrom=i_ಗಮ್ಯ=0; iFrom<cnt; iFrom++){
          ಆದರೆ( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[i_ಗಮ್ಯ++] = zNew[iFrom];
        }
        zNew[i_ಗಮ್ಯ] = 0;
        ಪ್ರತಿಜ್ಞಾ( i_ಗಮ್ಯ>0 );

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
        ಆದರೆ( p_ಎಡ->op!=TK_ಸ್ತಂಭ
         || sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಎಡ)!=SQLITE_AFF_ಪಠ್ಯ
         || (ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಎಡ) )
             && ALWAYS(p_ಎಡ->y.pTab)
             && IsVirtual(p_ಎಡ->y.pTab))  /* Might be numeric */
        ){
          ಪೂರ್ಣಾಂಕ is_ಅಂಕೆ;
          ದ್ವಿಗುಣ rDummy;
          ಪ್ರತಿಜ್ಞಾ( zNew[i_ಗಮ್ಯ]==0 );
          is_ಅಂಕೆ = sqlite3AtoF(zNew, &rDummy);
          ಆದರೆ( is_ಅಂಕೆ<=0 ){
            ಆದರೆ( i_ಗಮ್ಯ==1 && zNew[0]=='-' ){
              is_ಅಂಕೆ = +1;
            }ಇಲ್ಲದಿದ್ದರೆ{
              zNew[i_ಗಮ್ಯ-1]++;
              is_ಅಂಕೆ = sqlite3AtoF(zNew, &rDummy);
              zNew[i_ಗಮ್ಯ-1]--;
            }
          }
          ಆದರೆ( is_ಅಂಕೆ>0 ){
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಪೂರ್ವಪ್ರತ್ಯಯ);
            sqlite3_ಮೌಲ್ಯ_Free(pVal);
            ಹಿಂತಿರುಗಿ 0;
          }
        }
      }
      *pp_ಪೂರ್ವಪ್ರತ್ಯಯ = p_ಪೂರ್ವಪ್ರತ್ಯಯ;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      ಆದರೆ( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_ಬಲ->i_ಸ್ತಂಭ, 0);
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ, EP_Int_ಮೌಲ್ಯ) );
        ಆದರೆ( *pisComplete && p_ಬಲ->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, p_ಬಲ, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_ಮೌಲ್ಯ_Free(pVal);
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Like_Operator(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOp;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ eOp;
  } aOp[] = {
    { "match",  SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಹೊಂದಿಕೆ },
    { "glob",   SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GLOB },
    { "like",   SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LIKE },
    { "regexp", SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_REGEXP }
  };
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(aOp); i++){
    ಆದರೆ( sqlite3StrICmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, aOp[i].zOp)==0 ){
      ಹಿಂತಿರುಗಿ aOp[i].eOp;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,                    /* Test this expression */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  ಅಭಿವ್ಯಕ್ತಿ **pp_ಎಡ,                  /* Column expression to left of MATCH/op2 */
  ಅಭಿವ್ಯಕ್ತಿ **pp_ಬಲ                  /* Expression to left of MATCH/op2 */
){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಲಂಬಸಾಲು;                     /* Column reference */
    ಪೂರ್ಣಾಂಕ i;

    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
    ಆದರೆ( pList==0 || pList->n_ಅಭಿವ್ಯಕ್ತಿ!=2 ){
      ಹಿಂತಿರುಗಿ 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    p_ಲಂಬಸಾಲು = pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು->op!=TK_ಸ್ತಂಭ || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಲಂಬಸಾಲು) && p_ಲಂಬಸಾಲು->y.pTab!=0) );
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(p_ಲಂಬಸಾಲು) && (i = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Like_Operator(p_ಅಭಿವ್ಯಕ್ತಿ))!=0 ){
      *peOp2 = i;
      *pp_ಬಲ = pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      *pp_ಎಡ = p_ಲಂಬಸಾಲು;
      ಹಿಂತಿರುಗಿ 1;
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
    p_ಲಂಬಸಾಲು = pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು->op!=TK_ಸ್ತಂಭ || ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಲಂಬಸಾಲು) );
    ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು->op!=TK_ಸ್ತಂಭ || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಲಂಬಸಾಲು) && p_ಲಂಬಸಾಲು->y.pTab!=0) );
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(p_ಲಂಬಸಾಲು) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      ಶೂನ್ಯ (*x_Not_ಬಳಸಿದ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**);
      ಶೂನ್ಯ *p_Not_ಬಳಸಿದ;
      pVtab = sqlite3GetVTable(db, p_ಲಂಬಸಾಲು->y.pTab)->pVtab;
      ಪ್ರತಿಜ್ಞಾ( pVtab!=0 );
      ಪ್ರತಿಜ್ಞಾ( pVtab->pModule!=0 );
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      pMod = (sqlite3_module *)pVtab->pModule;
      ಆದರೆ( pMod->x_ಕಂಡುಹಿಡಿ_Function!=0 ){
        i = pMod->x_ಕಂಡುಹಿಡಿ_Function(pVtab,2, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, &x_Not_ಬಳಸಿದ, &p_Not_ಬಳಸಿದ);
        ಆದರೆ( i>=SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_ಬಲ = pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
          *pp_ಎಡ = p_ಲಂಬಸಾಲು;
          ಹಿಂತಿರುಗಿ 1;
        }
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    ಪ್ರತಿಜ್ಞಾ( TK_NE < TK_EQ );
    ಪ್ರತಿಜ್ಞಾ( TK_ISNOT < TK_EQ );
    ಪ್ರತಿಜ್ಞಾ( TK_NOTNULL < TK_EQ );
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL ){
    ಪೂರ್ಣಾಂಕ res = 0;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
    ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op!=TK_ಸ್ತಂಭ || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಎಡ) && p_ಎಡ->y.pTab!=0) );
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(p_ಎಡ) ){
      res++;
    }
    ಪ್ರತಿಜ್ಞಾ( p_ಬಲ==0 || p_ಬಲ->op!=TK_ಸ್ತಂಭ
            || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಬಲ) && p_ಬಲ->y.pTab!=0) );
    ಆದರೆ( p_ಬಲ && ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(p_ಬಲ) ){
      res++;
      ಅದಲುಬದಲು(ಅಭಿವ್ಯಕ್ತಿ*, p_ಎಡ, p_ಬಲ);
    }
    *pp_ಎಡ = p_ಎಡ;
    *pp_ಬಲ = p_ಬಲ;
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE ) *peOp2 = SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_NE;
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT ) *peOp2 = SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ISNOT;
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL ) *peOp2 = SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ISNOTNULL;
    ಹಿಂತಿರುಗಿ res;
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ಸ್ಥಿರ ಶೂನ್ಯ transferJoinMarkings(ಅಭಿವ್ಯಕ್ತಿ *pDerived, ಅಭಿವ್ಯಕ್ತಿ *p_ತಳ){
  ಆದರೆ( pDerived && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ತಳ, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= p_ತಳ->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = p_ತಳ->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ಸ್ಥಿರ ಶೂನ್ಯ mark_ಪದ_As_ಮಗು(WhereClause *pWC, ಪೂರ್ಣಾಂಕ i_ಮಗು, ಪೂರ್ಣಾಂಕ i_ಪೋಷಕ){
  pWC->a[i_ಮಗು].i_ಪೋಷಕ = i_ಪೋಷಕ;
  pWC->a[i_ಮಗು].truthProb = pWC->a[i_ಪೋಷಕ].truthProb;
  ಪ್ರತಿಜ್ಞಾ(   pWC->a[i_ಪೋಷಕ].n_ಮಗು <  UMXV(pWC->a[0].n_ಮಗು) );
  pWC->a[i_ಪೋಷಕ].n_ಮಗು++;
  testcase( pWC->a[i_ಪೋಷಕ].n_ಮಗು == UMXV(pWC->a[0].n_ಮಗು) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ಸ್ಥಿರ Where_ಪದ *whereNthSubterm(Where_ಪದ *p_ಪದ, ಪೂರ್ಣಾಂಕ N){
  ಆದರೆ( p_ಪದ->eOperator!=WO_AND ){
    ಹಿಂತಿರುಗಿ N==0 ? p_ಪದ : 0;
  }
  ಆದರೆ( N<p_ಪದ->u.pAndInfo->wc.n_ಪದ ){
    ಹಿಂತಿರುಗಿ &p_ಪದ->u.pAndInfo->wc.a[N];
  }
  ಹಿಂತಿರುಗಿ 0;
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
  Where_ಪದ *pOne,       /* First disjunct */
  Where_ಪದ *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  ಅಭಿವ್ಯಕ್ತಿ *pNew;            /* New virtual expression */
  ಪೂರ್ಣಾಂಕ op;                /* Operator for the combined expression */
  ಪೂರ್ಣಾಂಕ idxNew;            /* Index in pWC of the next virtual term */
  ಅಭಿವ್ಯಕ್ತಿ *pA, *pB;         /* Expressions associated with pOne and pTwo */

  ಆದರೆ( (pOne->wtFlags | pTwo->wtFlags) & ಪದ_VNULL ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) ಹಿಂತಿರುಗಿ;
  pA = pOne->p_ಅಭಿವ್ಯಕ್ತಿ;
  pB = pTwo->p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( pA->p_ಎಡ!=0 && pA->p_ಬಲ!=0 );
  ಪ್ರತಿಜ್ಞಾ( pB->p_ಎಡ!=0 && pB->p_ಬಲ!=0 );
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pA->p_ಎಡ, pB->p_ಎಡ, -1) )  ಹಿಂತಿರುಗಿ;
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pA->p_ಬಲ, pB->p_ಬಲ,-1) ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pA,EP_Commuted)!=ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pB,EP_Commuted) ){
    ಹಿಂತಿರುಗಿ;
  }
  /* If we reach this point, it means the two subterms can be combined */
  ಆದರೆ( (eOp & (eOp-1))!=0 ){
    ಆದರೆ( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pA, 0);
  ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ ಪ್ರತಿಜ್ಞಾ( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_ಸೇರಿಸು(pWC, pNew, ಪದ_VIRTUAL|ಪದ_DYNAMIC);
  ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
}

#ಆದರೆ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_Analyze_Or_ಪದ(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  ಪೂರ್ಣಾಂಕ idx_ಪದ               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  Where_ಪದ *p_ಪದ = &pWC->a[idx_ಪದ];    /* The term to be analyzed */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;             /* The expression of the term */
  ಪೂರ್ಣಾಂಕ i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  Where_ಪದ *p_Or_ಪದ;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chng_ಗಮ್ಯ_ಒಳ;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  ಪ್ರತಿಜ್ಞಾ( (p_ಪದ->wtFlags & (ಪದ_DYNAMIC|ಪದ_ORINFO|ಪದ_ANDINFO))==0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_OR );
  p_ಪದ->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, ಗಾತ್ರ(*pOrInfo));
  ಆದರೆ( pOrInfo==0 ) ಹಿಂತಿರುಗಿ;
  p_ಪದ->wtFlags |= ಪದ_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, ಗಾತ್ರ(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3_Where_ವಿಭಜಿಸು(pOrWc, p_ಅಭಿವ್ಯಕ್ತಿ, TK_OR);
  sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pOrWc);
  ಆದರೆ( db->mallocFailed ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( pOrWc->n_ಪದ>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chng_ಗಮ್ಯ_ಒಳ = ~(Bitmask)0;
  ಗಾಗಿ(i=pOrWc->n_ಪದ-1, p_Or_ಪದ=pOrWc->a; i>=0 && indexable; i--, p_Or_ಪದ++){
    ಆದರೆ( (p_Or_ಪದ->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      ಪ್ರತಿಜ್ಞಾ( (p_Or_ಪದ->wtFlags & (ಪದ_ANDINFO|ಪದ_ORINFO))==0 );
      chng_ಗಮ್ಯ_ಒಳ = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(*pAndInfo));
      ಆದರೆ( pAndInfo ){
        WhereClause *pAndWC;
        Where_ಪದ *p_And_ಪದ;
        ಪೂರ್ಣಾಂಕ j;
        Bitmask b = 0;
        p_Or_ಪದ->u.pAndInfo = pAndInfo;
        p_Or_ಪದ->wtFlags |= ಪದ_ANDINFO;
        p_Or_ಪದ->eOperator = WO_AND;
        p_Or_ಪದ->ಎಡ_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, ಗಾತ್ರ(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3_Where_ವಿಭಜಿಸು(pAndWC, p_Or_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ, TK_AND);
        sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        ಆದರೆ( !db->mallocFailed ){
          ಗಾಗಿ(j=0, p_And_ಪದ=pAndWC->a; j<pAndWC->n_ಪದ; j++, p_And_ಪದ++){
            ಪ್ರತಿಜ್ಞಾ( p_And_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ );
            ಆದರೆ( allowedOp(p_And_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->op)
             || p_And_ಪದ->eOperator==WO_AUX
            ){
              b |= sqlite3_Where_Get_ಮುಖವಾಡ(&pWInfo->s_ಮುಖವಾಡ_ಗಣ, p_And_ಪದ->ಎಡ_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_Or_ಪದ->wtFlags & ಪದ_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }ಇಲ್ಲದಿದ್ದರೆ{
      Bitmask b;
      b = sqlite3_Where_Get_ಮುಖವಾಡ(&pWInfo->s_ಮುಖವಾಡ_ಗಣ, p_Or_ಪದ->ಎಡ_Cursor);
      ಆದರೆ( p_Or_ಪದ->wtFlags & ಪದ_VIRTUAL ){
        Where_ಪದ *pOther = &pOrWc->a[p_Or_ಪದ->i_ಪೋಷಕ];
        b |= sqlite3_Where_Get_ಮುಖವಾಡ(&pWInfo->s_ಮುಖವಾಡ_ಗಣ, pOther->ಎಡ_Cursor);
      }
      indexable &= b;
      ಆದರೆ( (p_Or_ಪದ->eOperator & WO_EQ)==0 ){
        chng_ಗಮ್ಯ_ಒಳ = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        chng_ಗಮ್ಯ_ಒಳ &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  p_ಪದ->eOperator = WO_OR;
  p_ಪದ->ಎಡ_Cursor = -1;
  ಆದರೆ( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  ಆದರೆ( indexable && pOrWc->n_ಪದ==2 ){
    ಪೂರ್ಣಾಂಕ iOne = 0;
    Where_ಪದ *pOne;
    ಆಗ( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      ಪೂರ್ಣಾಂಕ iTwo = 0;
      Where_ಪದ *pTwo;
      ಆಗ( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  ಆದರೆ( chng_ಗಮ್ಯ_ಒಳ ){
    ಪೂರ್ಣಾಂಕ ok_ಗಮ್ಯ_Chng_ಗಮ್ಯ_ಒಳ = 0;     /* True if the conversion to IN is valid */
    ಪೂರ್ಣಾಂಕ i_ಸ್ತಂಭ = -1;         /* Column index on lhs of IN operator */
    ಪೂರ್ಣಾಂಕ iCursor = -1;         /* Table cursor common to all terms */
    ಪೂರ್ಣಾಂಕ j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    ಗಾಗಿ(j=0; j<2 && !ok_ಗಮ್ಯ_Chng_ಗಮ್ಯ_ಒಳ; j++){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = 0;
      p_Or_ಪದ = pOrWc->a;
      ಗಾಗಿ(i=pOrWc->n_ಪದ-1; i>=0; i--, p_Or_ಪದ++){
        ಪ್ರತಿಜ್ಞಾ( p_Or_ಪದ->eOperator & WO_EQ );
        p_Or_ಪದ->wtFlags &= ~ಪದ_OK;
        ಆದರೆ( p_Or_ಪದ->ಎಡ_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          ಪ್ರತಿಜ್ಞಾ( j==1 );
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( (chng_ಗಮ್ಯ_ಒಳ & sqlite3_Where_Get_ಮುಖವಾಡ(&pWInfo->s_ಮುಖವಾಡ_ಗಣ,
                                            p_Or_ಪದ->ಎಡ_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( p_Or_ಪದ->wtFlags & ಪದ_COPIED );
          testcase( p_Or_ಪದ->wtFlags & ಪದ_VIRTUAL );
          ಪ್ರತಿಜ್ಞಾ( p_Or_ಪದ->wtFlags & (ಪದ_COPIED|ಪದ_VIRTUAL) );
          ಮುಂದುವರಿ;
        }
        ಪ್ರತಿಜ್ಞಾ( (p_Or_ಪದ->eOperator & (WO_OR|WO_AND))==0 );
        i_ಸ್ತಂಭ = p_Or_ಪದ->u.x.ಎಡ_ಸ್ತಂಭ;
        iCursor = p_Or_ಪದ->ಎಡ_Cursor;
        p_ಎಡ = p_Or_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        ಪ್ರತಿಜ್ಞಾ( j==1 );
        ಪ್ರತಿಜ್ಞಾ( Is_Power_ದ_Two(chng_ಗಮ್ಯ_ಒಳ) );
        ಪ್ರತಿಜ್ಞಾ( chng_ಗಮ್ಯ_ಒಳ==sqlite3_Where_Get_ಮುಖವಾಡ(&pWInfo->s_ಮುಖವಾಡ_ಗಣ, iCursor) );
        ನಿಲ್ಲಿಸು;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      ok_ಗಮ್ಯ_Chng_ಗಮ್ಯ_ಒಳ = 1;
      ಗಾಗಿ(; i>=0 && ok_ಗಮ್ಯ_Chng_ಗಮ್ಯ_ಒಳ; i--, p_Or_ಪದ++){
        ಪ್ರತಿಜ್ಞಾ( p_Or_ಪದ->eOperator & WO_EQ );
        ಪ್ರತಿಜ್ಞಾ( (p_Or_ಪದ->eOperator & (WO_OR|WO_AND))==0 );
        ಆದರೆ( p_Or_ಪದ->ಎಡ_Cursor!=iCursor ){
          p_Or_ಪದ->wtFlags &= ~ಪದ_OK;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_Or_ಪದ->u.x.ಎಡ_ಸ್ತಂಭ!=i_ಸ್ತಂಭ || (i_ಸ್ತಂಭ==XN_ಅಭಿವ್ಯಕ್ತಿ
               && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, p_Or_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಎಡ, -1)
        )){
          ok_ಗಮ್ಯ_Chng_ಗಮ್ಯ_ಒಳ = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪೂರ್ಣಾಂಕ aff_ಎಡ, aff_ಬಲ;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_Or_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
          aff_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_Or_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
          ಆದರೆ( aff_ಬಲ!=0 && aff_ಬಲ!=aff_ಎಡ ){
            ok_ಗಮ್ಯ_Chng_ಗಮ್ಯ_ಒಳ = 0;
          }ಇಲ್ಲದಿದ್ದರೆ{
            p_Or_ಪದ->wtFlags |= ಪದ_OK;
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
    ಆದರೆ( ok_ಗಮ್ಯ_Chng_ಗಮ್ಯ_ಒಳ ){
      ಅಭಿವ್ಯಕ್ತಿ *pDup;            /* A transient duplicate expression */
      ಅಭಿವ್ಯಕ್ತಿ_List *pList = 0;   /* The RHS of the IN operator */
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      ಅಭಿವ್ಯಕ್ತಿ *pNew;            /* The complete IN operator */

      ಗಾಗಿ(i=pOrWc->n_ಪದ-1, p_Or_ಪದ=pOrWc->a; i>=0; i--, p_Or_ಪದ++){
        ಅಭಿವ್ಯಕ್ತಿ *pThis;
        ಆದರೆ( (p_Or_ಪದ->wtFlags & ಪದ_OK)==0 ) ಮುಂದುವರಿ;
        ಪ್ರತಿಜ್ಞಾ( p_Or_ಪದ->eOperator & WO_EQ );
        ಪ್ರತಿಜ್ಞಾ( (p_Or_ಪದ->eOperator & (WO_OR|WO_AND))==0 );
        ಪ್ರತಿಜ್ಞಾ( p_Or_ಪದ->ಎಡ_Cursor==iCursor );
        ಪ್ರತಿಜ್ಞಾ( p_Or_ಪದ->u.x.ಎಡ_ಸ್ತಂಭ==i_ಸ್ತಂಭ );
        pThis = p_Or_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
        pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pThis->p_ಬಲ, 0);
        pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pWInfo->pParse, pList, pDup);
        ಆದರೆ( p_ಎಡ==0 ){
          p_ಎಡ = pThis->p_ಎಡ;
          pCollSeq = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(pParse, pThis);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪ್ರತಿಜ್ಞಾ( 0==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, 
                        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(pThis->p_ಎಡ),
                        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(p_ಎಡ), -1) );
          ಆದರೆ( pCollSeq!=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(pParse, pThis) ){
            p_ಎಡ = 0;  /* Collating sequence mismatch */
            ನಿಲ್ಲಿಸು;
          }
        }
      }
      ಆದರೆ( p_ಎಡ==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0);
        ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          ಪ್ರತಿಜ್ಞಾ( pCollSeq->zName!=0 );
          pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_ಒಳ, pDup, 0);
      }
      ಆದರೆ( pNew ){
        ಪೂರ್ಣಾಂಕ idxNew;
        transferJoinMarkings(pNew, p_ಅಭಿವ್ಯಕ್ತಿ);
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_ಸೇರಿಸು(pWC, pNew, ಪದ_VIRTUAL|ಪದ_DYNAMIC);
        testcase( idxNew==0 );
        ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        mark_ಪದ_As_ಮಗು(pWC, idxNew, idx_ಪದ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList);
      }
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಪದ_Is_Equivalence(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, SrcList *pSrc){
  ಅಕ್ಷರ aff1, aff2;
  ಆದರೆ( !OptimizationEnabled(pParse->db, SQLITE_ಸಂಕ್ರಾಮಿಕ) ) ಹಿಂತಿರುಗಿ 0;  /* (1) */
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_EQ && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_IS ) ಹಿಂತಿರುಗಿ 0;                 /* (2) */
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_Collate) ) ಹಿಂತಿರುಗಿ 0;        /* (3) */
  ಪ್ರತಿಜ್ಞಾ( pSrc!=0 );
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    ಹಿಂತಿರುಗಿ 0;                                                          /* (4) */
  }
  aff1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  aff2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
  ಆದರೆ( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    ಹಿಂತಿರುಗಿ 0;                                                          /* (5) */
  }
  ಆದರೆ( !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq_ಹೊಂದಿಕೆ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ) ){
    ಹಿಂತಿರುಗಿ 0;                                                          /* (6) */
  }
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ಸ್ಥಿರ Bitmask ಅಭಿವ್ಯಕ್ತಿ_Select_Usage(Where_ಮುಖವಾಡ_ಗಣ *p_ಮುಖವಾಡ_ಗಣ, Select *pS){
  Bitmask ಮುಖವಾಡ = 0;
  ಆಗ( pS ){
    SrcList *pSrc = pS->pSrc;
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(p_ಮುಖವಾಡ_ಗಣ, pS->pEList);
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(p_ಮುಖವಾಡ_ಗಣ, pS->p_ಗುಂಪು_By);
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(p_ಮುಖವಾಡ_ಗಣ, pS->p_ಕ್ರಮ_By);
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, pS->pWhere);
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, pS->pHaving);
    ಆದರೆ( ALWAYS(pSrc!=0) ){
      ಪೂರ್ಣಾಂಕ i;
      ಗಾಗಿ(i=0; i<pSrc->nSrc; i++){
        ಆದರೆ( pSrc->a[i].fg.isSubquery ){
          ಮುಖವಾಡ |= ಅಭಿವ್ಯಕ್ತಿ_Select_Usage(p_ಮುಖವಾಡ_ಗಣ, pSrc->a[i].u4.pSubq->pSelect);
        }
        ಆದರೆ( pSrc->a[i].fg.isUsing==0 ){
          ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, pSrc->a[i].u3.pOn);
        }
        ಆದರೆ( pSrc->a[i].fg.isTabFunc ){
          ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(p_ಮುಖವಾಡ_ಗಣ, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->p_ಪೂರ್ವ;
  }
  ಹಿಂತಿರುಗಿ ಮುಖವಾಡ;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  ಪೂರ್ಣಾಂಕ *ai_Cur_ಲಂಬಸಾಲು,         /* Write the referenced table cursor and column here */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,           /* An operand of a comparison operator */
  ಪೂರ್ಣಾಂಕ j                  /* Start looking with the j-th pFrom entry */
){
  ಸೂಚ್ಯಂಕ *pIdx;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ iCur;
  ಮಾಡು{
    iCur = pFrom->a[j].iCursor;
    ಗಾಗಿ(pIdx=pFrom->a[j].pSTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
      ಆದರೆ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಮುಂದುವರಿ;
      ಗಾಗಿ(i=0; i<pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
        ಆದರೆ( pIdx->ai_ಸ್ತಂಭ[i]!=XN_ಅಭಿವ್ಯಕ್ತಿ ) ಮುಂದುವರಿ;
        ಪ್ರತಿಜ್ಞಾ( pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ );
        ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Skip(p_ಅಭಿವ್ಯಕ್ತಿ,pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[i].p_ಅಭಿವ್ಯಕ್ತಿ,iCur)==0
         && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(0,pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[i].p_ಅಭಿವ್ಯಕ್ತಿ)
        ){
          ai_Cur_ಲಂಬಸಾಲು[0] = iCur;
          ai_Cur_ಲಂಬಸಾಲು[1] = XN_ಅಭಿವ್ಯಕ್ತಿ;
          ಹಿಂತಿರುಗಿ 1;
        }
      }
    }
  }ಆಗ( ++j < pFrom->nSrc );
  ಹಿಂತಿರುಗಿ 0;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Might_Be_ಸೂಚ್ಯಂಕಿತ(
  SrcList *pFrom,        /* The FROM clause */
  ಪೂರ್ಣಾಂಕ *ai_Cur_ಲಂಬಸಾಲು,         /* Write the referenced table cursor & column here */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,           /* An operand of a comparison operator */
  ಪೂರ್ಣಾಂಕ op                 /* The specific comparison operator */
){
  ಪೂರ್ಣಾಂಕ i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  ಪ್ರತಿಜ್ಞಾ( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  ಪ್ರತಿಜ್ಞಾ( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_ಒಳ<TK_GE );
  ಪ್ರತಿಜ್ಞಾ( op<=TK_GE );
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸದಿಶ && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
  }

  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ ){
    ai_Cur_ಲಂಬಸಾಲು[0] = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;
    ai_Cur_ಲಂಬಸಾಲು[1] = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
    ಹಿಂತಿರುಗಿ 1;
  }

  ಗಾಗಿ(i=0; i<pFrom->nSrc; i++){
    ಸೂಚ್ಯಂಕ *pIdx;
    ಗಾಗಿ(pIdx=pFrom->a[i].pSTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
      ಆದರೆ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ ){
        ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_Might_Be_Indexed2(pFrom,ai_Cur_ಲಂಬಸಾಲು,p_ಅಭಿವ್ಯಕ್ತಿ,i);
      }
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
  ಪೂರ್ಣಾಂಕ idx_ಪದ               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  Where_ಪದ *p_ಪದ;                /* The term to be analyzed */
  Where_ಮುಖವಾಡ_ಗಣ *p_ಮುಖವಾಡ_ಗಣ;          /* Set of table index masks */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;                     /* The expression to be analyzed */
  Bitmask prereq_ಎಡ;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_ಬಲ = 0;          /* Extra dependencies on LEFT JOIN */
  ಅಭಿವ್ಯಕ್ತಿ *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  ಪೂರ್ಣಾಂಕ isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  ಪೂರ್ಣಾಂಕ noCase = 0;                  /* uppercase equivalent to lowercase */
  ಪೂರ್ಣಾಂಕ op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  ಪೂರ್ಣಾಂಕ n_ಎಡ;                       /* Number of elements on left side vector */

  ಆದರೆ( db->mallocFailed ){
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( pWC->n_ಪದ > idx_ಪದ );
  p_ಪದ = &pWC->a[idx_ಪದ];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  p_ಪದ->i_ಪದ = idx_ಪದ;
#ಆದರೆ_ಅಂತ್ಯ
  p_ಮುಖವಾಡ_ಗಣ = &pWInfo->s_ಮುಖವಾಡ_ಗಣ;
  p_ಅಭಿವ್ಯಕ್ತಿ = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 ); /* Because malloc() has not failed */
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AS && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_COLLATE );
ಅಭಿವ್ಯಕ್ತಿ_Analyze_restart:
  p_ಮುಖವಾಡ_ಗಣ->bVarSelect = 0;
  prereq_ಎಡ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಆದರೆ( op==TK_ಒಳ ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==0 );
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಒಳ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ) ಹಿಂತಿರುಗಿ;
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
      p_ಪದ->prereq_ಬಲ = ಅಭಿವ್ಯಕ್ತಿ_Select_Usage(p_ಮುಖವಾಡ_ಗಣ, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect);
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಪದ->prereq_ಬಲ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(p_ಮುಖವಾಡ_ಗಣ, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList);
    }
    prereqAll = prereq_ಎಡ | p_ಪದ->prereq_ಬಲ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಪದ->prereq_ಬಲ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0
     || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect|EP_If_Null_ಅಡ್ಡಸಾಲು)
     || p_ಅಭಿವ್ಯಕ್ತಿ->x.pList!=0
    ){
      prereqAll = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(p_ಮುಖವಾಡ_ಗಣ, p_ಅಭಿವ್ಯಕ್ತಿ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      prereqAll = prereq_ಎಡ | p_ಪದ->prereq_ಬಲ;
    }
  }
  ಆದರೆ( p_ಮುಖವಾಡ_ಗಣ->bVarSelect ) p_ಪದ->wtFlags |= ಪದ_VARSELECT;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( prereqAll!=sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(p_ಮುಖವಾಡ_ಗಣ, p_ಅಭಿವ್ಯಕ್ತಿ) ){
    ಬರೆ("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0,p_ಅಭಿವ್ಯಕ್ತಿ,0);
    ಪ್ರತಿಜ್ಞಾ( 0 );
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3_Where_Get_ಮುಖವಾಡ(p_ಮುಖವಾಡ_ಗಣ, p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin);
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ){
      prereqAll |= x;
      extra_ಬಲ = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (prereqAll>>1)>=x ){
      ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON);
    }
  }
  p_ಪದ->prereqAll = prereqAll;
  p_ಪದ->ಎಡ_Cursor = -1;
  p_ಪದ->i_ಪೋಷಕ = -1;
  p_ಪದ->eOperator = 0;
  ಆದರೆ( allowedOp(op) ){
    ಪೂರ್ಣಾಂಕ ai_Cur_ಲಂಬಸಾಲು[2];
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    u16 op_ಮುಖವಾಡ = (p_ಪದ->prereq_ಬಲ & prereq_ಎಡ)==0 ? WO_ALL : WO_EQUIV;

    ಆದರೆ( p_ಪದ->u.x.i_ಕ್ಷೇತ್ರ>0 ){
      ಪ್ರತಿಜ್ಞಾ( op==TK_ಒಳ );
      ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op==TK_ಸದಿಶ );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಎಡ) );
      p_ಎಡ = p_ಎಡ->x.pList->a[p_ಪದ->u.x.i_ಕ್ಷೇತ್ರ-1].p_ಅಭಿವ್ಯಕ್ತಿ;
    }

    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Might_Be_ಸೂಚ್ಯಂಕಿತ(pSrc, ai_Cur_ಲಂಬಸಾಲು, p_ಎಡ, op) ){
      p_ಪದ->ಎಡ_Cursor = ai_Cur_ಲಂಬಸಾಲು[0];
      ಪ್ರತಿಜ್ಞಾ( (p_ಪದ->eOperator & (WO_OR|WO_AND))==0 );
      p_ಪದ->u.x.ಎಡ_ಸ್ತಂಭ = ai_Cur_ಲಂಬಸಾಲು[1];
      p_ಪದ->eOperator = operator_ಮುಖವಾಡ(op) & op_ಮುಖವಾಡ;
    }
    ಆದರೆ( op==TK_IS ) p_ಪದ->wtFlags |= ಪದ_IS;
    ಆದರೆ( p_ಬಲ
     && ಅಭಿವ್ಯಕ್ತಿ_Might_Be_ಸೂಚ್ಯಂಕಿತ(pSrc, ai_Cur_ಲಂಬಸಾಲು, p_ಬಲ, op)
     && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ, EP_Fixed_ಲಂಬಸಾಲು)
    ){
      Where_ಪದ *pNew;
      ಅಭಿವ್ಯಕ್ತಿ *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      ಪ್ರತಿಜ್ಞಾ( p_ಪದ->u.x.i_ಕ್ಷೇತ್ರ==0 );
      ಆದರೆ( p_ಪದ->ಎಡ_Cursor>=0 ){
        ಪೂರ್ಣಾಂಕ idxNew;
        pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಆದರೆ( db->mallocFailed ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDup);
          ಹಿಂತಿರುಗಿ;
        }
        idxNew = where_Clause_ಸೇರಿಸು(pWC, pDup, ಪದ_VIRTUAL|ಪದ_DYNAMIC);
        ಆದರೆ( idxNew==0 ) ಹಿಂತಿರುಗಿ;
        pNew = &pWC->a[idxNew];
        mark_ಪದ_As_ಮಗು(pWC, idxNew, idx_ಪದ);
        ಆದರೆ( op==TK_IS ) pNew->wtFlags |= ಪದ_IS;
        p_ಪದ = &pWC->a[idx_ಪದ];
        p_ಪದ->wtFlags |= ಪದ_COPIED;
        ಪ್ರತಿಜ್ಞಾ( pWInfo->pTabList!=0 );
        ಆದರೆ( ಪದ_Is_Equivalence(pParse, pDup, pWInfo->pTabList) ){
          p_ಪದ->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDup = p_ಅಭಿವ್ಯಕ್ತಿ;
        pNew = p_ಪದ;
      }
      pNew->wtFlags |= ಅಭಿವ್ಯಕ್ತಿ_Commute(pParse, pDup);
      pNew->ಎಡ_Cursor = ai_Cur_ಲಂಬಸಾಲು[0];
      ಪ್ರತಿಜ್ಞಾ( (p_ಪದ->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.ಎಡ_ಸ್ತಂಭ = ai_Cur_ಲಂಬಸಾಲು[1];
      testcase( (prereq_ಎಡ | extra_ಬಲ) != prereq_ಎಡ );
      pNew->prereq_ಬಲ = prereq_ಎಡ | extra_ಬಲ;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operator_ಮುಖವಾಡ(pDup->op) + eExtraOp) & op_ಮುಖವಾಡ;
    }ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( op==TK_ISNULL
     && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_OuterON)
     && 0==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಎಡ)
    ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken = "false";
      ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_IsFalse);
      p_ಪದ->prereqAll = 0;
      p_ಪದ->eOperator = 0;
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_BETWEEN_OPTIMIZATION
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
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_BETWEEN && pWC->op==TK_AND ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಪೂರ್ಣಾಂಕ i;
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 ops[] = {TK_GE, TK_LE};
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
    ಪ್ರತಿಜ್ಞಾ( pList!=0 );
    ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 );
    ಪ್ರತಿಜ್ಞಾ( pWC->a[idx_ಪದ].n_ಮಗು==0 );
    ಗಾಗಿ(i=0; i<2; i++){
      ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
      ಪೂರ್ಣಾಂಕ idxNew;
      p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, ops[i],
                             sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0),
                             sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, 0));
      transferJoinMarkings(p_New_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಭಿವ್ಯಕ್ತಿ);
      idxNew = where_Clause_ಸೇರಿಸು(pWC, p_New_ಅಭಿವ್ಯಕ್ತಿ, ಪದ_VIRTUAL|ಪದ_DYNAMIC);
      testcase( idxNew==0 );
      ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
      p_ಪದ = &pWC->a[idx_ಪದ];
      mark_ಪದ_As_ಮಗು(pWC, idxNew, idx_ಪದ);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#ಆದರೆ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_OR && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Collate) ){
    ಅಭಿವ್ಯಕ್ತಿ *pAlt = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ);
    ಆದರೆ( pAlt!=p_ಅಭಿವ್ಯಕ್ತಿ ){
      p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pAlt);
      ಹೋಗು ಅಭಿವ್ಯಕ್ತಿ_Analyze_restart;
    }
    ಪ್ರತಿಜ್ಞಾ( pWC->op==TK_AND );
    ಅಭಿವ್ಯಕ್ತಿ_Analyze_Or_ಪದ(pSrc, pWC, idx_ಪದ);
    p_ಪದ = &pWC->a[idx_ಪದ];
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL ){
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ಸ್ತಂಭ
     && p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->i_ಸ್ತಂಭ>=0
     && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON)
    ){
      ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಪೂರ್ಣಾಂಕ idxNew;
      Where_ಪದ *p_New_ಪದ;
 
      p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0),
                              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_ಸೇರಿಸು(pWC, p_New_ಅಭಿವ್ಯಕ್ತಿ,
                                ಪದ_VIRTUAL|ಪದ_DYNAMIC|ಪದ_VNULL);
      ಆದರೆ( idxNew ){
        p_New_ಪದ = &pWC->a[idxNew];
        p_New_ಪದ->prereq_ಬಲ = 0;
        p_New_ಪದ->ಎಡ_Cursor = p_ಎಡ->i_ಕೋಷ್ಟಕ;
        p_New_ಪದ->u.x.ಎಡ_ಸ್ತಂಭ = p_ಎಡ->i_ಸ್ತಂಭ;
        p_New_ಪದ->eOperator = WO_GT;
        mark_ಪದ_As_ಮಗು(pWC, idxNew, idx_ಪದ);
        p_ಪದ = &pWC->a[idx_ಪದ];
        p_ಪದ->wtFlags |= ಪದ_COPIED;
        p_New_ಪದ->prereqAll = p_ಪದ->prereqAll;
      }
    }
  }


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_LIKE_OPTIMIZATION
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
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, &pStr1, &isComplete, &noCase)
  ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;       /* LHS of LIKE/GLOB operator */
    ಅಭಿವ್ಯಕ್ತಿ *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    ಅಭಿವ್ಯಕ್ತಿ *pNewExpr1;
    ಅಭಿವ್ಯಕ್ತಿ *pNewExpr2;
    ಪೂರ್ಣಾಂಕ idxNew1;
    ಪೂರ್ಣಾಂಕ idxNew2;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCollSeqName;     /* Name of collating sequence */
    ಸ್ಥಿರಾಂಕ u16 wtFlags = ಪದ_LIKEOPT | ಪದ_VIRTUAL | ಪದ_DYNAMIC;

    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
    pStr2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pStr1, 0);
    ಪ್ರತಿಜ್ಞಾ( pStr1==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pStr1, EP_Int_ಮೌಲ್ಯ) );
    ಪ್ರತಿಜ್ಞಾ( pStr2==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pStr2, EP_Int_ಮೌಲ್ಯ) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    ಆದರೆ( noCase && !pParse->db->mallocFailed ){
      ಪೂರ್ಣಾಂಕ i;
      ಅಕ್ಷರ c;
      p_ಪದ->wtFlags |= ಪದ_LIKE;
      ಗಾಗಿ(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    ಆದರೆ( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      ಆದರೆ( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        ಆದರೆ( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      ಆಗ( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      ಪ್ರತಿಜ್ಞಾ( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ದ್ವಿಮಾನ;
    pNewExpr1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_ಅಭಿವ್ಯಕ್ತಿ);
    idxNew1 = where_Clause_ಸೇರಿಸು(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_ಅಭಿವ್ಯಕ್ತಿ);
    idxNew2 = where_Clause_ಸೇರಿಸು(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew1);
    ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew2);
    p_ಪದ = &pWC->a[idx_ಪದ];
    ಆದರೆ( isComplete ){
      mark_ಪದ_As_ಮಗು(pWC, idxNew1, idx_ಪದ);
      mark_ಪದ_As_ಮಗು(pWC, idxNew2, idx_ಪದ);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LIKE_OPTIMIZATION */

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
  ಆದರೆ( (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EQ || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS)
   && (n_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ))>1
   && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ)==n_ಎಡ
   && ( (p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->flags & EP_xIsSelect)==0
     || (p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<n_ಎಡ; i++){
      ಪೂರ್ಣಾಂಕ idxNew;
      ಅಭಿವ್ಯಕ್ತಿ *pNew;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_ಸದಿಶ_ಕ್ಷೇತ್ರ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, i, n_ಎಡ);
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_ಸದಿಶ_ಕ್ಷೇತ್ರ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, i, n_ಎಡ);

      pNew = sqlite3PExpr(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->op, p_ಎಡ, p_ಬಲ);
      transferJoinMarkings(pNew, p_ಅಭಿವ್ಯಕ್ತಿ);
      idxNew = where_Clause_ಸೇರಿಸು(pWC, pNew, ಪದ_DYNAMIC|ಪದ_ತುಂಡು);
      ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
    }
    p_ಪದ = &pWC->a[idx_ಪದ];
    p_ಪದ->wtFlags |= ಪದ_CODED|ಪದ_VIRTUAL;  /* Disable the original */
    p_ಪದ->eOperator = WO_ROWVAL;
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
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಒಳ
   && p_ಪದ->u.x.i_ಕ್ಷೇತ್ರ==0
   && p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ಸದಿಶ
   && ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) )
   && (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->p_ಪೂರ್ವ==0 || (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_ಮೌಲ್ಯಗಳು))
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
   && p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pWin==0
#ಆದರೆ_ಅಂತ್ಯ
   && (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ <= UMXV(p_ಪದ->n_ಮಗು)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    ಪೂರ್ಣಾಂಕ i;
    ಪ್ರತಿಜ್ಞಾ( p_ಪದ->n_ಮಗು==0 );
    ಗಾಗಿ(i=0; i<sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ); i++){
      ಪೂರ್ಣಾಂಕ idxNew;
      idxNew = where_Clause_ಸೇರಿಸು(pWC, p_ಅಭಿವ್ಯಕ್ತಿ, ಪದ_VIRTUAL|ಪದ_ತುಂಡು);
      pWC->a[idxNew].u.x.i_ಕ್ಷೇತ್ರ = i+1;
      ಅಭಿವ್ಯಕ್ತಿ_Analyze(pSrc, pWC, idxNew);
      mark_ಪದ_As_ಮಗು(pWC, idxNew, idx_ಪದ);
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  /* Add a WO_AUX auxiliary term to the constraint set if the
  ** current expression is of the form "column OP expr" where OP
  ** is an operator that gets passed into virtual tables but which is
  ** not normally optimized for ordinary tables.  In other words, OP
  ** is one of MATCH, LIKE, GLOB, REGEXP, !=, IS, IS NOT, or NOT NULL.
  ** This information is used by the xBestIndex methods of
  ** virtual tables.  The native query optimizer does not attempt
  ** to do anything with MATCH functions.
  */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pWC->op==TK_AND ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = 0, *p_ಎಡ = 0;
    ಪೂರ್ಣಾಂಕ res = isAuxiliaryVtabOperator(db, p_ಅಭಿವ್ಯಕ್ತಿ, &eOp2, &p_ಎಡ, &p_ಬಲ);
    ಆಗ( res-- > 0 ){
      ಪೂರ್ಣಾಂಕ idxNew;
      Where_ಪದ *p_New_ಪದ;
      Bitmask prereq_ಸ್ತಂಭ, prereq_ಅಭಿವ್ಯಕ್ತಿ;

      prereq_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, p_ಬಲ);
      prereq_ಸ್ತಂಭ = sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, p_ಎಡ);
      ಆದರೆ( (prereq_ಅಭಿವ್ಯಕ್ತಿ & prereq_ಸ್ತಂಭ)==0 ){
        ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
        p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_ಹೊಂದಿಕೆ,
            0, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಬಲ, 0));
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) && p_New_ಅಭಿವ್ಯಕ್ತಿ ){
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_New_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON);
          p_New_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin = p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin;
        }
        idxNew = where_Clause_ಸೇರಿಸು(pWC, p_New_ಅಭಿವ್ಯಕ್ತಿ, ಪದ_VIRTUAL|ಪದ_DYNAMIC);
        testcase( idxNew==0 );
        p_New_ಪದ = &pWC->a[idxNew];
        p_New_ಪದ->prereq_ಬಲ = prereq_ಅಭಿವ್ಯಕ್ತಿ | extra_ಬಲ;
        p_New_ಪದ->ಎಡ_Cursor = p_ಎಡ->i_ಕೋಷ್ಟಕ;
        p_New_ಪದ->u.x.ಎಡ_ಸ್ತಂಭ = p_ಎಡ->i_ಸ್ತಂಭ;
        p_New_ಪದ->eOperator = WO_AUX;
        p_New_ಪದ->e_ಹೊಂದಿಕೆ_Op = eOp2;
        mark_ಪದ_As_ಮಗು(pWC, idxNew, idx_ಪದ);
        p_ಪದ = &pWC->a[idx_ಪದ];
        p_ಪದ->wtFlags |= ಪದ_COPIED;
        p_New_ಪದ->prereqAll = p_ಪದ->prereqAll;
      }
      ಅದಲುಬದಲು(ಅಭಿವ್ಯಕ್ತಿ*, p_ಎಡ, p_ಬಲ);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( p_ಪದ!=&pWC->a[idx_ಪದ] );
  p_ಪದ = &pWC->a[idx_ಪದ];
  p_ಪದ->prereq_ಬಲ |= extra_ಬಲ;
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
ಶೂನ್ಯ sqlite3_Where_ವಿಭಜಿಸು(WhereClause *pWC, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, u8 op){
  ಅಭಿವ್ಯಕ್ತಿ *pE2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ);
  pWC->op = op;
  ಪ್ರತಿಜ್ಞಾ( pE2!=0 || p_ಅಭಿವ್ಯಕ್ತಿ==0 );
  ಆದರೆ( pE2==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( pE2->op!=op ){
    where_Clause_ಸೇರಿಸು(pWC, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Where_ವಿಭಜಿಸು(pWC, pE2->p_ಎಡ, op);
    sqlite3_Where_ವಿಭಜಿಸು(pWC, pE2->p_ಬಲ, op);
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
ಸ್ಥಿರ ಶೂನ್ಯ where_ಕೂಡಿಸು_ಎಲ್ಲೆ_ಅಭಿವ್ಯಕ್ತಿ(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  ಪೂರ್ಣಾಂಕ iReg,           /* Register that will hold value of the limit/offset */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,        /* Expression that defines the limit/offset */
  ಪೂರ್ಣಾಂಕ iCsr,           /* Cursor to which the constraint applies */
  ಪೂರ್ಣಾಂಕ e_ಹೊಂದಿಕೆ_Op        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  ಪೂರ್ಣಾಂಕ iVal = 0;

  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p_ಅಭಿವ್ಯಕ್ತಿ, &iVal, pParse, 0) && iVal>=0 ){
    ಅಭಿವ್ಯಕ್ತಿ *pVal = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, iVal);
    ಆದರೆ( pVal==0 ) ಹಿಂತಿರುಗಿ;
    pNew = sqlite3PExpr(pParse, TK_ಹೊಂದಿಕೆ, 0, pVal);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಅಭಿವ್ಯಕ್ತಿ *pVal = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ರಿಜಿಸ್ಟರ್, 0, 0);
    ಆದರೆ( pVal==0 ) ಹಿಂತಿರುಗಿ;
    pVal->i_ಕೋಷ್ಟಕ = iReg;
    pNew = sqlite3PExpr(pParse, TK_ಹೊಂದಿಕೆ, 0, pVal);
  }
  ಆದರೆ( pNew ){
    Where_ಪದ *p_ಪದ;
    ಪೂರ್ಣಾಂಕ idx;
    idx = where_Clause_ಸೇರಿಸು(pWC, pNew, ಪದ_DYNAMIC|ಪದ_VIRTUAL);
    p_ಪದ = &pWC->a[idx];
    p_ಪದ->ಎಡ_Cursor = iCsr;
    p_ಪದ->eOperator = WO_AUX;
    p_ಪದ->e_ಹೊಂದಿಕೆ_Op = e_ಹೊಂದಿಕೆ_Op;
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
ಶೂನ್ಯ SQLITE_NOINLINE sqlite3_Where_ಕೂಡಿಸು_ಎಲ್ಲೆ(WhereClause *pWC, Select *p){
  ಪ್ರತಿಜ್ಞಾ( p!=0 && p->p_ಎಲ್ಲೆ!=0 );                 /* 1 -- checked by caller */
  ಆದರೆ( p->p_ಗುಂಪು_By==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By = p->p_ಕ್ರಮ_By;
    ಪೂರ್ಣಾಂಕ iCsr = p->pSrc->a[0].iCursor;
    ಪೂರ್ಣಾಂಕ ii;

    /* Check condition (4). Return early if it is not met. */
    ಗಾಗಿ(ii=0; ii<pWC->n_ಪದ; ii++){
      ಆದರೆ( pWC->a[ii].wtFlags & ಪದ_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        ಪ್ರತಿಜ್ಞಾ( pWC->a[ii].wtFlags & ಪದ_VIRTUAL );
        ಪ್ರತಿಜ್ಞಾ( pWC->a[ii].eOperator==WO_ROWVAL );
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( pWC->a[ii].n_ಮಗು ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( pWC->a[ii].ಎಡ_Cursor==iCsr && pWC->a[ii].prereq_ಬಲ==0 ) ಮುಂದುವರಿ;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      ಆದರೆ( pWC->a[ii].i_ಪೋಷಕ>=0 ){
        Where_ಪದ *p_ಪೋಷಕ = &pWC->a[ pWC->a[ii].i_ಪೋಷಕ ];
        ಆದರೆ( p_ಪೋಷಕ->ಎಡ_Cursor==iCsr
         && p_ಪೋಷಕ->prereq_ಬಲ==0
         && p_ಪೋಷಕ->n_ಮಗು==1
        ){
          ಮುಂದುವರಿ;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      ಹಿಂತಿರುಗಿ;
    }

    /* Check condition (5). Return early if it is not met. */
    ಆದರೆ( p_ಕ್ರಮ_By ){
      ಗಾಗಿ(ii=0; ii<p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
        ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = p_ಕ್ರಮ_By->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_ಸ್ತಂಭ ) ಹಿಂತಿರುಗಿ;
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ!=iCsr ) ಹಿಂತಿರುಗಿ;
        ಆದರೆ( p_ಕ್ರಮ_By->a[ii].fg.ವಿಂಗಡಿಸು_Flags & KEYINFO_ಕ್ರಮ_BIGNULL ) ಹಿಂತಿರುಗಿ;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    ಪ್ರತಿಜ್ಞಾ( p->p_ಎಲ್ಲೆ->op==TK_ಎಲ್ಲೆ );
    ಆದರೆ( p->i_ಸ್ಥಾನಾಂತರ!=0 && (p->selFlags & SF_Compound)==0 ){
      where_ಕೂಡಿಸು_ಎಲ್ಲೆ_ಅಭಿವ್ಯಕ್ತಿ(pWC, p->i_ಸ್ಥಾನಾಂತರ, p->p_ಎಲ್ಲೆ->p_ಬಲ,
                        iCsr, SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಸ್ಥಾನಾಂತರ);
    }
    ಆದರೆ( p->i_ಸ್ಥಾನಾಂತರ==0 || (p->selFlags & SF_Compound)==0 ){
      where_ಕೂಡಿಸು_ಎಲ್ಲೆ_ಅಭಿವ್ಯಕ್ತಿ(pWC, p->i_ಎಲ್ಲೆ, p->p_ಎಲ್ಲೆ->p_ಎಡ,
                        iCsr, SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಎಲ್ಲೆ);
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
  pWC->n_ಪದ = 0;
  pWC->n_ತಳ = 0;
  pWC->n_ಸ್ಥಾನ = Array_ಗಾತ್ರ(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
ಶೂನ್ಯ sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  ಪ್ರತಿಜ್ಞಾ( pWC->n_ಪದ>=pWC->n_ತಳ );
  ಆದರೆ( pWC->n_ಪದ>0 ){
    Where_ಪದ *a = pWC->a;
    Where_ಪದ *a_ಕೊನೆಯ = &pWC->a[pWC->n_ಪದ-1];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಪೂರ್ಣಾಂಕ i;
    /* Verify that every term past pWC->nBase is virtual */
    ಗಾಗಿ(i=pWC->n_ತಳ; i<pWC->n_ಪದ; i++){
      ಪ್ರತಿಜ್ಞಾ( (pWC->a[i].wtFlags & ಪದ_VIRTUAL)!=0 );
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆಗ(1){
      ಪ್ರತಿಜ್ಞಾ( a->e_ಹೊಂದಿಕೆ_Op==0 || a->eOperator==WO_AUX );
      ಆದರೆ( a->wtFlags & ಪದ_DYNAMIC ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, a->p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ಆದರೆ( a->wtFlags & (ಪದ_ORINFO|ಪದ_ANDINFO) ){
        ಆದರೆ( a->wtFlags & ಪದ_ORINFO ){
          ಪ್ರತಿಜ್ಞಾ( (a->wtFlags & ಪದ_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪ್ರತಿಜ್ಞಾ( (a->wtFlags & ಪದ_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      ಆದರೆ( a==a_ಕೊನೆಯ ) ನಿಲ್ಲಿಸು;
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
ಸ್ಥಿರ SQLITE_NOINLINE Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_ತುಂಬಿದ(
  Where_ಮುಖವಾಡ_ಗಣ *p_ಮುಖವಾಡ_ಗಣ,
  ಅಭಿವ್ಯಕ್ತಿ *p
){
  Bitmask ಮುಖವಾಡ;
  ಮುಖವಾಡ = (p->op==TK_IF_NULL_ಅಡ್ಡಸಾಲು) ? sqlite3_Where_Get_ಮುಖವಾಡ(p_ಮುಖವಾಡ_ಗಣ, p->i_ಕೋಷ್ಟಕ) : 0;
  ಆದರೆ( p->p_ಎಡ ) ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(p_ಮುಖವಾಡ_ಗಣ, p->p_ಎಡ);
  ಆದರೆ( p->p_ಬಲ ){
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(p_ಮುಖವಾಡ_ಗಣ, p->p_ಬಲ);
    ಪ್ರತಿಜ್ಞಾ( p->x.pList==0 );
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_VarSelect) ) p_ಮುಖವಾಡ_ಗಣ->bVarSelect = 1;
    ಮುಖವಾಡ |= ಅಭಿವ್ಯಕ್ತಿ_Select_Usage(p_ಮುಖವಾಡ_ಗಣ, p->x.pSelect);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->x.pList ){
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(p_ಮುಖವಾಡ_ಗಣ, p->x.pList);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಆದರೆ( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p) ){
    ಪ್ರತಿಜ್ಞಾ( p->y.pWin!=0 );
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(p_ಮುಖವಾಡ_ಗಣ, p->y.pWin->p_ವಿಭಜನೆ);
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(p_ಮುಖವಾಡ_ಗಣ, p->y.pWin->p_ಕ್ರಮ_By);
    ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, p->y.pWin->pFilter);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ ಮುಖವಾಡ;
}
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(Where_ಮುಖವಾಡ_ಗಣ *p_ಮುಖವಾಡ_ಗಣ, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಆದರೆ( p->op==TK_ಸ್ತಂಭ && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Fixed_ಲಂಬಸಾಲು) ){
    ಹಿಂತಿರುಗಿ sqlite3_Where_Get_ಮುಖವಾಡ(p_ಮುಖವಾಡ_ಗಣ, p->i_ಕೋಷ್ಟಕ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly|EP_ಎಲೆ) ){
    ಪ್ರತಿಜ್ಞಾ( p->op!=TK_IF_NULL_ಅಡ್ಡಸಾಲು );
    ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_ತುಂಬಿದ(p_ಮುಖವಾಡ_ಗಣ, p);
}
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(Where_ಮುಖವಾಡ_ಗಣ *p_ಮುಖವಾಡ_ಗಣ, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಹಿಂತಿರುಗಿ p ? sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(p_ಮುಖವಾಡ_ಗಣ,p) : 0;
}
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(Where_ಮುಖವಾಡ_ಗಣ *p_ಮುಖವಾಡ_ಗಣ, ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಪೂರ್ಣಾಂಕ i;
  Bitmask ಮುಖವಾಡ = 0;
  ಆದರೆ( pList ){
    ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಮುಖವಾಡ |= sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(p_ಮುಖವಾಡ_ಗಣ, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
  }
  ಹಿಂತಿರುಗಿ ಮುಖವಾಡ;
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
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=pWC->n_ಪದ-1; i>=0; i--){
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
ಶೂನ್ಯ sqlite3_Where_Tab_Func_ವಾದಗಳು(
  Parse *pParse,                    /* Parsing context */
  Src_ಅಂಶ *p_ಅಂಶ,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  ಕೋಷ್ಟಕ *pTab;
  ಪೂರ್ಣಾಂಕ j, k;
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ವಾದಗಳು;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಲಂಬಸಾಲು_Ref;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಪದ;
  ಆದರೆ( p_ಅಂಶ->fg.isTabFunc==0 ) ಹಿಂತಿರುಗಿ;
  pTab = p_ಅಂಶ->pSTab;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಆದರೆ( !IsVirtual(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "'%s' is not a function", p_ಅಂಶ->zName);
    ಹಿಂತಿರುಗಿ;
  }
  p_ವಾದಗಳು = p_ಅಂಶ->u1.pFuncArg;
  ಆದರೆ( p_ವಾದಗಳು==0 ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(j=k=0; j<p_ವಾದಗಳು->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
    ಅಭಿವ್ಯಕ್ತಿ *pRhs;
    u32 joinType;
    ಆಗ( k<pTab->n_ಲಂಬಸಾಲು && (pTab->a_ಲಂಬಸಾಲು[k].ಲಂಬಸಾಲು_Flags & COLFLAG_HIDDEN)==0 ){k++;}
    ಆದರೆ( k>=pTab->n_ಲಂಬಸಾಲು ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      ಹಿಂತಿರುಗಿ;
    }
    p_ಲಂಬಸಾಲು_Ref = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(pParse->db, TK_ಸ್ತಂಭ, 0, 0);
    ಆದರೆ( p_ಲಂಬಸಾಲು_Ref==0 ) ಹಿಂತಿರುಗಿ;
    p_ಲಂಬಸಾಲು_Ref->i_ಕೋಷ್ಟಕ = p_ಅಂಶ->iCursor;
    p_ಲಂಬಸಾಲು_Ref->i_ಸ್ತಂಭ = k++;
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಲಂಬಸಾಲು_Ref) );
    p_ಲಂಬಸಾಲು_Ref->y.pTab = pTab;
    p_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಲಂಬಸಾಲು_ಬಳಸಿದ(p_ಲಂಬಸಾಲು_Ref);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, p_ವಾದಗಳು->a[j].p_ಅಭಿವ್ಯಕ್ತಿ, 0), 0);
    p_ಪದ = sqlite3PExpr(pParse, TK_EQ, p_ಲಂಬಸಾಲು_Ref, pRhs);
    ಆದರೆ( p_ಅಂಶ->fg.jointype & (JT_ಎಡ|JT_ಬಲ) ){
      testcase( p_ಅಂಶ->fg.jointype & JT_ಎಡ );  /* testtag-20230227a */
      testcase( p_ಅಂಶ->fg.jointype & JT_ಬಲ ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }ಇಲ್ಲದಿದ್ದರೆ{
      testcase( p_ಅಂಶ->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_ಗಣ_Join_ಅಭಿವ್ಯಕ್ತಿ(p_ಪದ, p_ಅಂಶ->iCursor, joinType);
    where_Clause_ಸೇರಿಸು(pWC, p_ಪದ, ಪದ_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
ಪೂರ್ಣಾಂಕ sqlite3WhereLoopBloomable(ಸ್ಥಿರಾಂಕ WhereLoop *pLoop){
  ಆದರೆ( pLoop->wsFlags & WHERE_IPK ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( NEVER((pLoop->wsFlags & WHERE_ಸೂಚ್ಯಂಕಿತ)==0) ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ sqlite3_ಸೂಚ್ಯಂಕ_Bloomable(pLoop->u.btree.p_ಸೂಚ್ಯಂಕ, pLoop->u.btree.nEq);
}
