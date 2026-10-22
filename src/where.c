/*
** 2001 September 15
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
** the WHERE clause of SQL statements.  This module is responsible for
** generating the code that loops through a table looking for applicable
** rows.  Indices are selected and used to speed the search when doing
** so is applicable.  Because this module is responsible for selecting
** indices, you might also think of this module as the "query optimizer".
*/
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "whereInt.h"

/*
** Extra information appended to the end of sqlite3_index_info but not
** visible to the xBestIndex function, at least not directly.  The
** sqlite3_vtab_collation() interface knows how to reach it, however.
**
** This object is not an API and can be changed from one release to the
** next.  As long as allocateIndexInfo() and sqlite3_vtab_collation()
** agree on the structure, all will be well.
*/
प्रकारनिर्देश संरचना Hidden_सूचकांक_Info Hidden_सूचकांक_Info;
संरचना Hidden_सूचकांक_Info {
  WhereClause *pWC;        /* The Where clause being analyzed */
  Parse *pParse;           /* The parsing context */
  पूर्णाङ्क eDistinct;           /* Value to return from sqlite3_vtab_distinct() */
  u32 m_अन्तर्;                 /* Mask of terms that are <col> IN (...) */
  u32 m_Handle_अन्तर्;           /* Terms that vtab will handle as <col> IN (...) */
  sqlite3_मानम् *aRhs[FLEXARRAY];  /* RHS values for constraints. MUST BE LAST
                                   ** Extra space is allocated to hold up
                                   ** to nTerm such values */
};

/* Size (in bytes) of a HiddenIndeInfo object sufficient to hold as
** many as N constraints */
#परिभाषय SZ_HIDDENINDEXINFO(N) \
                  (offsetof(Hidden_सूचकांक_Info,aRhs) + (N)*परिमाण(sqlite3_मानम्*))

/* Forward declaration of methods */
स्थिर पूर्णाङ्क whereLoopResize(sqlite3*, WhereLoop*, पूर्णाङ्क);

/*
** Return the estimated number of output rows from a WHERE clause
*/
LogEst sqlite3_Where_Output_Row_गणनम्(WhereInfo *pWInfo){
  प्रत्यागम pWInfo->n_Row_बहिर्गमनम्;
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
पूर्णाङ्क sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  प्रत्यागम pWInfo->eDistinct;
}

/*
** Return the number of ORDER BY terms that are satisfied by the
** WHERE clause.  A return of 0 means that the output must be
** completely sorted.  A return equal to the number of ORDER BY
** terms means that no sorting is needed at all.  A return that
** is positive but less than the number of ORDER BY terms means that
** block sorting is required.
*/
पूर्णाङ्क sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  प्रत्यागम pWInfo->nOBSat<0 ? 0 : pWInfo->nOBSat;
}

/*
** In the ORDER BY LIMIT optimization, if the inner-most loop is known
** to emit rows in increasing order, and if the last row emitted by the
** inner-most loop did not fit within the sorter, then we can skip all
** subsequent rows for the current iteration of the inner loop (because they
** will not fit in the sorter either) and continue with the second inner
** loop - the loop immediately outside the inner-most.
**
** When a row does not fit in the sorter (because the sorter already
** holds LIMIT+OFFSET rows that are smaller), then a jump is made to the
** label returned by this function.
**
** If the ORDER BY LIMIT optimization applies, the jump destination should
** be the continuation for the second-inner-most loop.  If the ORDER BY
** LIMIT optimization does not apply, then the jump destination should
** be the continuation for the inner-most loop.
**
** It is always safe for this routine to return the continuation of the
** inner-most loop, in the sense that a correct answer will result. 
** Returning the continuation the second inner loop is an optimization
** that might make the code run a little faster, but should not change
** the final answer.
*/
पूर्णाङ्क sqlite3_Where_क्रमः_By_सीमा_Opt_Label(WhereInfo *pWInfo){
  WhereLevel *pInner;
  यदि( !pWInfo->bOrderedInnerLoop ){
    /* The ORDER BY LIMIT optimization does not apply.  Jump to the
    ** continuation of the inner-most loop. */
    प्रत्यागम pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel-1];
  प्रतिज्ञा( pInner->addrNxt!=0 );
  प्रत्यागम pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
}

/*
** While generating code for the min/max optimization, after handling
** the aggregate-step call to min() or max(), check to see if any
** additional looping is required.  If the output order is such that
** we are certain that the correct answer has already been found, then
** code an OP_Goto to by pass subsequent processing.
**
** Any extra OP_Goto that is coded here is an optimization.  The
** correct answer should be obtained regardless.  This OP_Goto just
** makes the answer appear faster.
*/
शून्य sqlite3_Where_Min_Max_Opt_Early_बहिर्गमनम्(Vdbe *v, WhereInfo *pWInfo){
  WhereLevel *pInner;
  पूर्णाङ्क i;
  यदि( !pWInfo->bOrderedInnerLoop ) प्रत्यागम;
  यदि( pWInfo->nOBSat==0 ) प्रत्यागम;
  प्रति(i=pWInfo->nLevel-1; i>=0; i--){
    pInner = &pWInfo->a[i];
    यदि( (pInner->pWLoop->wsFlags & WHERE_COLUMN_अन्तर्)!=0 ){
      sqlite3VdbeGoto(v, pInner->addrNxt);
      प्रत्यागम;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
पूर्णाङ्क sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  प्रतिज्ञा( pWInfo->iContinue!=0 );
  प्रत्यागम pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
पूर्णाङ्क sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  प्रत्यागम pWInfo->iBreak;
}

/*
** Return ONEPASS_OFF (0) if an UPDATE or DELETE statement is unable to
** operate directly on the rowids returned by a WHERE clause.  Return
** ONEPASS_SINGLE (1) if the statement can operation directly because only
** a single row is to be changed.  Return ONEPASS_MULTI (2) if the one-pass
** optimization can be used on multiple
**
** If the ONEPASS optimization is used (if this routine returns true)
** then also write the indices of open cursors used by ONEPASS
** into aiCur[0] and aiCur[1].  iaCur[0] gets the cursor of the data
** table and aiCur[1] gets the cursor used by an auxiliary index.
** Either value may be -1, indicating that cursor is not used.
** Any cursors returned will have been opened for writing.
**
** aiCur[0] and aiCur[1] both get -1 if the where-clause logic is
** unable to use the ONEPASS optimization.
*/
पूर्णाङ्क sqlite3WhereOkOnePass(WhereInfo *pWInfo, पूर्णाङ्क *aiCur){
  स्मृतिं_प्रतिलिख(aiCur, pWInfo->aiCurOnePass, परिमाण(पूर्णाङ्क)*2);
#यदि_परिभाषितम् WHERETRACE_ENABLED
  यदि( sqlite3WhereTrace && pWInfo->eOnePass!=ONEPASS_OFF ){
    sqlite3DebugPrintf("%s cursors: %d %d\n",
         pWInfo->eOnePass==ONEPASS_SINGLE ? "ONEPASS_SINGLE" : "ONEPASS_MULTI",
         aiCur[0], aiCur[1]);
  }
#यदि_अन्तः
  प्रत्यागम pWInfo->eOnePass;
}

/*
** Return TRUE if the WHERE loop uses the OP_DeferredSeek opcode to move
** the data cursor to the row selected by the index cursor.
*/
पूर्णाङ्क sqlite3_Where_उपयुङ्क्ते_Deferred_Seek(WhereInfo *pWInfo){
  प्रत्यागम pWInfo->bDeferredSeek;
}

/*
** Move the content of pSrc into pDest
*/
स्थिर शून्य whereOrMove(Where_Or_समुच्चयः *pDest, Where_Or_समुच्चयः *pSrc){
  pDest->n = pSrc->n;
  स्मृतिं_प्रतिलिख(pDest->a, pSrc->a, pDest->n*परिमाण(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
स्थिर पूर्णाङ्क where_Or_निवेशय(
  Where_Or_समुच्चयः *p_समुच्चयः,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst r_सततम्,           /* Run-cost of the new entry */
  LogEst n_बहिर्गमनम्            /* Number of outputs for the new entry */
){
  u16 i;
  WhereOrCost *p;
  प्रति(i=p_समुच्चयः->n, p=p_समुच्चयः->a; i>0; i--, p++){
    यदि( r_सततम्<=p->r_सततम् && (prereq & p->prereq)==prereq ){
      गच्छ where_Or_निवेशय_समाप्तम्;
    }
    यदि( p->r_सततम्<=r_सततम् && (p->prereq & prereq)==p->prereq ){
      प्रत्यागम 0;
    }
  }
  यदि( p_समुच्चयः->n<N_OR_COST ){
    p = &p_समुच्चयः->a[p_समुच्चयः->n++];
    p->n_बहिर्गमनम् = n_बहिर्गमनम्;
  }अन्यथा{
    p = p_समुच्चयः->a;
    प्रति(i=1; i<p_समुच्चयः->n; i++){
      यदि( p->r_सततम्>p_समुच्चयः->a[i].r_सततम् ) p = p_समुच्चयः->a + i;
    }
    यदि( p->r_सततम्<=r_सततम् ) प्रत्यागम 0;
  }
where_Or_निवेशय_समाप्तम्:
  p->prereq = prereq;
  p->r_सततम् = r_सततम्;
  यदि( p->n_बहिर्गमनम्>n_बहिर्गमनम् ) p->n_बहिर्गमनम् = n_बहिर्गमनम्;
  प्रत्यागम 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
Bitmask sqlite3_Where_Get_आवरणम्(Where_आवरणम्_समुच्चयः *p_आवरणम्_समुच्चयः, पूर्णाङ्क iCursor){
  पूर्णाङ्क i;
  प्रतिज्ञा( p_आवरणम्_समुच्चयः->n<=(पूर्णाङ्क)परिमाण(Bitmask)*8 );
  प्रतिज्ञा( p_आवरणम्_समुच्चयः->n>0 || p_आवरणम्_समुच्चयः->ix[0]<0 );
  प्रतिज्ञा( iCursor>=-1 );
  यदि( p_आवरणम्_समुच्चयः->ix[0]==iCursor ){
    प्रत्यागम 1;
  }
  प्रति(i=1; i<p_आवरणम्_समुच्चयः->n; i++){
    यदि( p_आवरणम्_समुच्चयः->ix[i]==iCursor ){
      प्रत्यागम MASKBIT(i);
    }
  }
  प्रत्यागम 0;
}

/* Allocate memory that is automatically freed when pWInfo is freed.
*/
शून्य *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte){
  Where_Mem_खण्डम् *p_खण्डम्;
  p_खण्डम् = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte+परिमाण(*p_खण्डम्));
  यदि( p_खण्डम् ){
    p_खण्डम्->p_अग्र = pWInfo->p_Mem_गन्तव्यम्_Free;
    p_खण्डम्->sz = nByte;
    pWInfo->p_Mem_गन्तव्यम्_Free = p_खण्डम्;
    p_खण्डम्++;
  }
  प्रत्यागम (शून्य*)p_खण्डम्;
}
शून्य *sqlite3WhereRealloc(WhereInfo *pWInfo, शून्य *pOld, u64 nByte){
  शून्य *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  यदि( pNew && pOld ){
    Where_Mem_खण्डम् *pOldBlk = (Where_Mem_खण्डम्*)pOld;
    pOldBlk--;
    प्रतिज्ञा( pOldBlk->sz<nByte );
    स्मृतिं_प्रतिलिख(pNew, pOld, pOldBlk->sz);
  }
  प्रत्यागम pNew;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
स्थिर शून्य create_आवरणम्(Where_आवरणम्_समुच्चयः *p_आवरणम्_समुच्चयः, पूर्णाङ्क iCursor){
  प्रतिज्ञा( p_आवरणम्_समुच्चयः->n < Array_आकार(p_आवरणम्_समुच्चयः->ix) );
  p_आवरणम्_समुच्चयः->ix[p_आवरणम्_समुच्चयः->n++] = iCursor;
}

/*
** If the right-hand branch of the expression is a TK_COLUMN, then return
** a pointer to the right-hand branch.  Otherwise, return NULL.
*/
स्थिर व्यञ्जकम् *where_दक्षिणः_Subexpr_Is_Column(व्यञ्जकम् *p){
  p = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p->p_दक्षिणः);
  यदि( ALWAYS(p!=0) && p->op==TK_COLUMN && !व्यञ्जकम्_Has_Property(p, EP_FixedCol) ){
    प्रत्यागम p;
  }
  प्रत्यागम 0;
}

/*
** Term pTerm is guaranteed to be a WO_IN term. It may be a component term
** of a vector IN expression of the form "(x, y, ...) IN (SELECT ...)".
** This function checks to see if the term is compatible with an index
** column with affinity idxaff (one of the SQLITE_AFF_XYZ values). If so,
** it returns a pointer to the name of the collation sequence (e.g. "BINARY"
** or "NOCASE") used by the comparison in pTerm. If it is not compatible
** with affinity idxaff, NULL is returned.
*/
स्थिर SQLITE_NOINLINE ध्रुव वर्ण *सूचकांक_अन्तर्_Affinity_Ok(
  Parse *pParse, 
  Where_पदांशः *p_पदांशः, 
  u8 idxaff
){
  व्यञ्जकम् *pX = p_पदांशः->p_व्यञ्जकम्;
  व्यञ्जकम् inexpr;

  प्रतिज्ञा( p_पदांशः->eOperator & WO_अन्तर् );

  यदि( व्यञ्जकम्_Use_XSelect(pX) ){
    पूर्णाङ्क i_क्षेत्रम् = 0;
    inexpr.flags = 0;
    inexpr.op = TK_EQ;
    यदि( sqlite3_व्यञ्जकम्_Is_सदिशम्(pX->p_वामः) ){
      i_क्षेत्रम् = p_पदांशः->u.x.i_क्षेत्रम् - 1;
      inexpr.p_वामः = pX->p_वामः->x.pList->a[i_क्षेत्रम्].p_व्यञ्जकम्;
    }अन्यथा{
      inexpr.p_वामः = pX->p_वामः;
    }
    inexpr.p_दक्षिणः = pX->x.pSelect->pEList->a[i_क्षेत्रम्].p_व्यञ्जकम्;
    pX = &inexpr;
  }

  यदि( sqlite3_सूचकांक_Affinity_Ok(pX, idxaff) ){
    CollSeq *pRet = sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pParse, pX);
    प्रत्यागम pRet ? pRet->zName : sqlite3_Str_द्विमान;
  }
  प्रत्यागम 0;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
स्थिर Where_पदांशः *where_अवलोकनम्_अग्र(Where_अवलोकनम् *p_अवलोकनम्){
  पूर्णाङ्क iCur;            /* The cursor on the LHS of the term */
  i16 iColumn;         /* The column on the LHS of the term.  -1 for IPK */
  व्यञ्जकम् *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  Where_पदांशः *p_पदांशः;    /* The term being tested */
  पूर्णाङ्क k = p_अवलोकनम्->k;    /* Where to start scanning */

  प्रतिज्ञा( p_अवलोकनम्->iEquiv<=p_अवलोकनम्->nEquiv );
  pWC = p_अवलोकनम्->pWC;
  यावत्(1){
    iColumn = p_अवलोकनम्->aiColumn[p_अवलोकनम्->iEquiv-1];
    iCur = p_अवलोकनम्->aiCur[p_अवलोकनम्->iEquiv-1];
    प्रतिज्ञा( pWC!=0 );
    प्रतिज्ञा( iCur>=0 );
    कुरु{
      प्रति(p_पदांशः=pWC->a+k; k<pWC->n_पदांशः; k++, p_पदांशः++){
        प्रतिज्ञा( (p_पदांशः->eOperator & (WO_OR|WO_AND))==0 || p_पदांशः->वामः_Cursor<0 );
        यदि( p_पदांशः->वामः_Cursor==iCur
         && p_पदांशः->u.x.वामः_Column==iColumn
         && (iColumn!=XN_व्यञ्जकम्
             || sqlite3_व्यञ्जकम्_Compare_Skip(p_पदांशः->p_व्यञ्जकम्->p_वामः,
                                       p_अवलोकनम्->p_Idx_व्यञ्जकम्,iCur)==0)
         && (p_अवलोकनम्->iEquiv<=1 || !व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON))
        ){
          यदि( (p_पदांशः->eOperator & WO_EQUIV)!=0
           && p_अवलोकनम्->nEquiv<Array_आकार(p_अवलोकनम्->aiCur)
           && (pX = where_दक्षिणः_Subexpr_Is_Column(p_पदांशः->p_व्यञ्जकम्))!=0
          ){
            पूर्णाङ्क j;
            प्रति(j=0; j<p_अवलोकनम्->nEquiv; j++){
              यदि( p_अवलोकनम्->aiCur[j]==pX->i_सारणी
               && p_अवलोकनम्->aiColumn[j]==pX->iColumn ){
                  भङ्ग;
              }
            }
            यदि( j==p_अवलोकनम्->nEquiv ){
              p_अवलोकनम्->aiCur[j] = pX->i_सारणी;
              p_अवलोकनम्->aiColumn[j] = pX->iColumn;
              p_अवलोकनम्->nEquiv++;
            }
          }
          यदि( (p_पदांशः->eOperator & p_अवलोकनम्->op_आवरणम्)!=0 ){
            /* Verify the affinity and collating sequence match */
            यदि( p_अवलोकनम्->zCollName && (p_पदांशः->eOperator & WO_ISNULL)==0 ){
              ध्रुव वर्ण *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = p_पदांशः->p_व्यञ्जकम्;

              यदि( (p_पदांशः->eOperator & WO_अन्तर्) ){
                zCollName = सूचकांक_अन्तर्_Affinity_Ok(pParse, p_पदांशः, p_अवलोकनम्->idxaff);
                यदि( !zCollName ) अनुवर्त;
              }अन्यथा{
                CollSeq *pColl;
                यदि( !sqlite3_सूचकांक_Affinity_Ok(pX, p_अवलोकनम्->idxaff) ){
                  अनुवर्त;
                }
                प्रतिज्ञा(pX->p_वामः);
                pColl = sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3_Str_द्विमान;
              }

              यदि( sqlite3StrICmp(zCollName, p_अवलोकनम्->zCollName) ){
                अनुवर्त;
              }
            }
            यदि( (p_पदांशः->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = p_पदांशः->p_व्यञ्जकम्->p_दक्षिणः, ALWAYS(pX!=0))
             && pX->op==TK_COLUMN
             && pX->i_सारणी==p_अवलोकनम्->aiCur[0]
             && pX->iColumn==p_अवलोकनम्->aiColumn[0]
            ){
              testcase( p_पदांशः->eOperator & WO_IS );
              अनुवर्त;
            }
            p_अवलोकनम्->pWC = pWC;
            p_अवलोकनम्->k = k+1;
#यदि_परिभाषितम् WHERETRACE_ENABLED
            यदि( (sqlite3WhereTrace & 0x20000)!=0 && p_अवलोकनम्->nEquiv>1 ){
              पूर्णाङ्क ii;
              sqlite3DebugPrintf("EQUIVALENT TO {%d:%d} (due to TERM-%d):",
                 p_अवलोकनम्->aiCur[0], p_अवलोकनम्->aiColumn[0], p_पदांशः->i_पदांशः);
              प्रति(ii=1; ii<p_अवलोकनम्->nEquiv; ii++){
                sqlite3DebugPrintf(" {%d:%d}",
                   p_अवलोकनम्->aiCur[ii], p_अवलोकनम्->aiColumn[ii]);
              }
              sqlite3DebugPrintf("\n");
            }
#यदि_अन्तः
            प्रत्यागम p_पदांशः;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    }यावत्( pWC!=0 );
    यदि( p_अवलोकनम्->iEquiv>=p_अवलोकनम्->nEquiv ) भङ्ग;
    pWC = p_अवलोकनम्->pOrigWC;
    k = 0;
    p_अवलोकनम्->iEquiv++;
  }
  प्रत्यागम 0;
}

/*
** This is whereScanInit() for the case of an index on an expression.
** It is factored out into a separate tail-recursion subroutine so that
** the normal whereScanInit() routine, which is a high-runner, does not
** need to push registers onto the stack as part of its prologue.
*/
स्थिर SQLITE_NOINLINE Where_पदांशः *where_अवलोकनम्_Init_सूचकांक_व्यञ्जकम्(Where_अवलोकनम् *p_अवलोकनम्){
  p_अवलोकनम्->idxaff = sqlite3_व्यञ्जकम्_Affinity(p_अवलोकनम्->p_Idx_व्यञ्जकम्);
  प्रत्यागम where_अवलोकनम्_अग्र(p_अवलोकनम्);
}

/*
** Initialize a WHERE clause scanner object.  Return a pointer to the
** first match.  Return NULL if there are no matches.
**
** The scanner will be searching the WHERE clause pWC.  It will look
** for terms of the form "X <op> <expr>" where X is column iColumn of table
** iCur.   Or if pIdx!=0 then X is column iColumn of index pIdx.  pIdx
** must be one of the indexes of table iCur.
**
** The <op> must be one of the operators described by opMask.
**
** If the search is for X and the WHERE clause contains terms of the
** form X=Y then this routine might also return terms of the form
** "Y <op> <expr>".  The number of levels of transitivity is limited,
** but is enough to handle most commonly occurring SQL statements.
**
** If X is not the INTEGER PRIMARY KEY then X must be compatible with
** index pIdx.
*/
स्थिर Where_पदांशः *where_अवलोकनम्_Init(
  Where_अवलोकनम् *p_अवलोकनम्,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  पूर्णाङ्क iCur,               /* Cursor to scan for */
  पूर्णाङ्क iColumn,            /* Column to scan for */
  u32 op_आवरणम्,             /* Operator(s) to scan for */
  सूचकांक *pIdx             /* Must be compatible with this index */
){
  p_अवलोकनम्->pOrigWC = pWC;
  p_अवलोकनम्->pWC = pWC;
  p_अवलोकनम्->p_Idx_व्यञ्जकम् = 0;
  p_अवलोकनम्->idxaff = 0;
  p_अवलोकनम्->zCollName = 0;
  p_अवलोकनम्->op_आवरणम् = op_आवरणम्;
  p_अवलोकनम्->k = 0;
  p_अवलोकनम्->aiCur[0] = iCur;
  p_अवलोकनम्->nEquiv = 1;
  p_अवलोकनम्->iEquiv = 1;
  यदि( pIdx ){
    पूर्णाङ्क j = iColumn;
    iColumn = pIdx->aiColumn[j];
    यदि( iColumn==pIdx->p_सारणी->iPKey ){
      iColumn = XN_ROWID;
    }अन्यथा यदि( iColumn>=0 ){
      p_अवलोकनम्->idxaff = pIdx->p_सारणी->aCol[iColumn].affinity;
      p_अवलोकनम्->zCollName = pIdx->azColl[j];
    }अन्यथा यदि( iColumn==XN_व्यञ्जकम् ){
      p_अवलोकनम्->p_Idx_व्यञ्जकम् = pIdx->a_Col_व्यञ्जकम्->a[j].p_व्यञ्जकम्;
      p_अवलोकनम्->zCollName = pIdx->azColl[j];
      p_अवलोकनम्->aiColumn[0] = XN_व्यञ्जकम्;
      प्रत्यागम where_अवलोकनम्_Init_सूचकांक_व्यञ्जकम्(p_अवलोकनम्);
    }
  }अन्यथा यदि( iColumn==XN_व्यञ्जकम् ){
    प्रत्यागम 0;
  }
  p_अवलोकनम्->aiColumn[0] = iColumn;
  प्रत्यागम where_अवलोकनम्_अग्र(p_अवलोकनम्);
}

/*
** Search for a term in the WHERE clause that is of the form "X <op> <expr>"
** where X is a reference to the iColumn of table iCur or of index pIdx
** if pIdx!=0 and <op> is one of the WO_xx operator codes specified by
** the op parameter.  Return a pointer to the term.  Return 0 if not found.
**
** If pIdx!=0 then it must be one of the indexes of table iCur. 
** Search for terms matching the iColumn-th column of pIdx
** rather than the iColumn-th column of table iCur.
**
** The term returned might by Y=<expr> if there is another constraint in
** the WHERE clause that specifies that X=Y.  Any such constraints will be
** identified by the WO_EQUIV bit in the pTerm->eOperator field.  The
** aiCur[]/iaColumn[] arrays hold X and all its equivalents. There are 11
** slots in aiCur[]/aiColumn[] so that means we can look for X plus up to 10
** other equivalent values.  Hence a search for X will return <expr> if X=A1
** and A1=A2 and A2=A3 and ... and A9=A10 and A10=<expr>.
**
** If there are multiple terms in the WHERE clause of the form "X <op> <expr>"
** then try for the one with no dependencies on <expr> - in other words where
** <expr> is a constant expression of some kind.  Only return entries of
** the form "X <op> Y" where Y is a column in another table if no terms of
** the form "X <op> <const-expr>" exist.   If no terms with a constant RHS
** exist, try to return a term that does not use WO_EQUIV.
*/
Where_पदांशः *sqlite3_Where_Find_पदांशः(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  पूर्णाङ्क iCur,             /* Cursor number of LHS */
  पूर्णाङ्क iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  सूचकांक *pIdx           /* Must be compatible with this index, if not NULL */
){
  Where_पदांशः *p_परिणाम = 0;
  Where_पदांशः *p;
  Where_अवलोकनम् अवलोकनम्;

  p = where_अवलोकनम्_Init(&अवलोकनम्, pWC, iCur, iColumn, op, pIdx);
  op &= WO_EQ|WO_IS;
  यावत्( p ){
    यदि( (p->prereq_दक्षिणः & notReady)==0 ){
      यदि( p->prereq_दक्षिणः==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        प्रत्यागम p;
      }
      यदि( p_परिणाम==0 ) p_परिणाम = p;
    }
    p = where_अवलोकनम्_अग्र(&अवलोकनम्);
  }
  प्रत्यागम p_परिणाम;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
स्थिर पूर्णाङ्क find_सूचकांक_Col(
  Parse *pParse,                  /* Parse context */
  व्यञ्जकम्_List *pList,                /* Expression list to search */
  पूर्णाङ्क i_आधारः,                      /* Cursor for table associated with pIdx */
  सूचकांक *pIdx,                    /* Index to match column of */
  पूर्णाङ्क iCol                        /* Column of index to match */
){
  पूर्णाङ्क i;
  ध्रुव वर्ण *zColl = pIdx->azColl[iCol];

  प्रति(i=0; i<pList->n_व्यञ्जकम्; i++){
    व्यञ्जकम् *p = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(pList->a[i].p_व्यञ्जकम्);
    यदि( ALWAYS(p!=0)
     && (p->op==TK_COLUMN || p->op==TK_AGG_COLUMN)
     && p->iColumn==pIdx->aiColumn[iCol]
     && p->i_सारणी==i_आधारः
    ){
      CollSeq *pColl = sqlite3_व्यञ्जकम्_NNColl_Seq(pParse, pList->a[i].p_व्यञ्जकम्);
      यदि( 0==sqlite3StrICmp(pColl->zName, zColl) ){
        प्रत्यागम i;
      }
    }
  }

  प्रत्यागम -1;
}

/*
** Return TRUE if the iCol-th column of index pIdx is NOT NULL
*/
स्थिर पूर्णाङ्क सूचकांक_Column_Not_Null(सूचकांक *pIdx, पूर्णाङ्क iCol){
  पूर्णाङ्क j;
  प्रतिज्ञा( pIdx!=0 );
  प्रतिज्ञा( iCol>=0 && iCol<pIdx->nColumn );
  j = pIdx->aiColumn[iCol];
  यदि( j>=0 ){
    प्रत्यागम pIdx->p_सारणी->aCol[j].notNull;
  }अन्यथा यदि( j==(-1) ){
    प्रत्यागम 1;
  }अन्यथा{
    प्रतिज्ञा( j==(-2) );
    प्रत्यागम 0;  /* Assume an indexed expression can always yield a NULL */

  }
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
स्थिर पूर्णाङ्क isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  व्यञ्जकम्_List *pDistinct       /* The result set that needs to be DISTINCT */
){
  सारणी *pTab;
  सूचकांक *pIdx;
  पूर्णाङ्क i;                         
  पूर्णाङ्क i_आधारः;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT
  ** clause is redundant. */
  यदि( pTabList->nSrc!=1 ) प्रत्यागम 0;
  i_आधारः = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  /* If any of the expressions is an IPK column on table iBase, then return
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  प्रति(i=0; i<pDistinct->n_व्यञ्जकम्; i++){
    व्यञ्जकम् *p = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(pDistinct->a[i].p_व्यञ्जकम्);
    यदि( NEVER(p==0) ) अनुवर्त;
    यदि( p->op!=TK_COLUMN && p->op!=TK_AGG_COLUMN ) अनुवर्त;
    यदि( p->i_सारणी==i_आधारः && p->iColumn<0 ) प्रत्यागम 1;
  }

  /* Loop through all indices on the table, checking each to see if it makes
  ** the DISTINCT qualifier redundant. It does so if:
  **
  **   1. The index is itself UNIQUE, and
  **
  **   2. All of the columns in the index are either part of the pDistinct
  **      list, or else the WHERE clause contains a term of the form "col=X",
  **      where X is a constant value. The collation sequences of the
  **      comparison and select-list expressions must match those of the index.
  **
  **   3. All of those index columns for which the WHERE clause does not
  **      contain a "col=X" term are subject to a NOT NULL constraint.
  */
  प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
    यदि( !Is_Unique_सूचकांक(pIdx) ) अनुवर्त;
    यदि( pIdx->pPartIdxWhere ) अनुवर्त;
    प्रति(i=0; i<pIdx->n_कुञ्चिका_Col; i++){
      यदि( 0==sqlite3_Where_Find_पदांशः(pWC, i_आधारः, i, ~(Bitmask)0, WO_EQ, pIdx) ){
        यदि( find_सूचकांक_Col(pParse, pDistinct, i_आधारः, pIdx, i)<0 ) भङ्ग;
        यदि( सूचकांक_Column_Not_Null(pIdx, i)==0 ) भङ्ग;
      }
    }
    यदि( i==pIdx->n_कुञ्चिका_Col ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      प्रत्यागम 1;
    }
  }

  प्रत्यागम 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
स्थिर LogEst estLog(LogEst N){
  प्रत्यागम N<=10 ? 0 : sqlite3LogEst(N) - 33;
}

/*
** Convert OP_Column opcodes to OP_Copy in previously generated code.
**
** This routine runs over generated VDBE code and translates OP_Column
** opcodes into OP_Copy when the table is being accessed via co-routine
** instead of via table lookup.
**
** If the iAutoidxCur is not zero, then any OP_Rowid instructions on
** cursor iTabCur are transformed into OP_Sequence opcode for the
** iAutoidxCur cursor, in order to generate unique rowids for the
** automatic index being generated.
*/
स्थिर शून्य translate_Column_गन्तव्यम्_Copy(
  Parse *pParse,      /* Parsing context */
  पूर्णाङ्क iStart,         /* Translate from this opcode to the end */
  पूर्णाङ्क iTabCur,        /* OP_Column/OP_Rowid references to this table */
  पूर्णाङ्क i_पञ्जिकाकोशः,      /* The first column is in this register */
  पूर्णाङ्क iAutoidxCur     /* If non-zero, cursor of autoindex being generated */
){
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, iStart);
  पूर्णाङ्क i_अन्तः = sqlite3VdbeCurrentAddr(v);
  यदि( pParse->db->mallocFailed ) प्रत्यागम;
#यदि_परिभाषितम् SQLITE_DEBUG
  यदि( pParse->db->flags & SQLITE_VdbeAddopTrace ){
    प्रारूप_लिख("CHECKING for column-to-copy on cursor %d for %d..%d\n",
            iTabCur, iStart, i_अन्तः);
  }
#यदि_अन्तः
  प्रति(; iStart<i_अन्तः; iStart++, pOp++){
    यदि( pOp->p1!=iTabCur ) अनुवर्त;
    यदि( pOp->opcode==OP_Column ){
#यदि_परिभाषितम् SQLITE_DEBUG
      यदि( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        प्रारूप_लिख("TRANSLATE OP_Column to OP_Copy at %d\n", iStart);
      }
#यदि_अन्तः
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + i_पञ्जिकाकोशः;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;  /* Cause the MEM_Subtype flag to be cleared */
    }अन्यथा यदि( pOp->opcode==OP_Rowid ){
#यदि_परिभाषितम् SQLITE_DEBUG
      यदि( pParse->db->flags & SQLITE_VdbeAddopTrace ){
        प्रारूप_लिख("TRANSLATE OP_Rowid to OP_Sequence at %d\n", iStart);
      }
#यदि_अन्तः
      pOp->opcode = OP_Sequence;
      pOp->p1 = iAutoidxCur;
#यदि_परिभाषितम् SQLITE_ALLOW_ROWID_अन्तर्_VIEW
      यदि( iAutoidxCur==0 ){
        pOp->opcode = OP_Null;
        pOp->p3 = 0;
      }
#यदि_अन्तः
    }
  }
}

/*
** Two routines for printing the content of an sqlite3_index_info
** structure.  Used for testing and debugging only.  If neither
** SQLITE_TEST or SQLITE_DEBUG are defined, then these routines
** are no-ops.
*/
#यदि !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
स्थिर शून्य where_Trace_सूचकांक_Info_Inputs(
  sqlite3_सूचकांक_info *p,   /* The IndexInfo object */
  सारणी *pTab              /* The TABLE that is the virtual table */
){
  पूर्णाङ्क i;
  यदि( (sqlite3WhereTrace & 0x10)==0 ) प्रत्यागम;
  sqlite3DebugPrintf("sqlite3_index_info inputs for %s:\n", pTab->zName);
  प्रति(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf(
       "  constraint[%d]: col=%d termid=%d op=%d usabled=%d collseq=%s\n",
       i,
       p->aConstraint[i].iColumn,
       p->aConstraint[i].i_पदांशः_विस्थापनम्,
       p->aConstraint[i].op,
       p->aConstraint[i].usable,
       sqlite3_vtab_collation(p,i));
  }
  प्रति(i=0; i<p->n_क्रमः_By; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->a_क्रमः_By[i].iColumn,
       p->a_क्रमः_By[i].desc);
  }
}
स्थिर शून्य where_Trace_सूचकांक_Info_Outputs(
  sqlite3_सूचकांक_info *p,   /* The IndexInfo object */
  सारणी *pTab              /* The TABLE that is the virtual table */
){
  पूर्णाङ्क i;
  यदि( (sqlite3WhereTrace & 0x10)==0 ) प्रत्यागम;
  sqlite3DebugPrintf("sqlite3_index_info outputs for %s:\n", pTab->zName);
  प्रति(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argv_सूचकांक,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idx_अङ्कः);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->क्रमः_By_Consumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimatedCost);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimated_पङ्क्तिकाः);
}
#अन्यथा
#परिभाषय where_Trace_सूचकांक_Info_Inputs(A,B)
#परिभाषय where_Trace_सूचकांक_Info_Outputs(A,B)
#यदि_अन्तः

/*
** We know that pSrc is an operand of an outer join.  Return true if
** pTerm is a constraint that is compatible with that join.
**
** pTerm must be EP_OuterON if pSrc is the right operand of an
** outer join.  pTerm can be either EP_OuterON or EP_InnerON if pSrc
** is the left operand of a RIGHT join.
**
** See https://sqlite.org/forum/forumpost/206d99a16dd9212f
** for an example of a WHERE clause constraints that may not be used on
** the right table of a RIGHT JOIN because the constraint implies a
** not-NULL condition on the left table of the RIGHT JOIN.
*/
स्थिर पूर्णाङ्क constraintCompatibleWithOuterJoin(
  ध्रुव Where_पदांशः *p_पदांशः,       /* WHERE clause term to check */
  ध्रुव SrcItem *pSrc           /* Table we are trying to access */
){
  प्रतिज्ञा( (pSrc->fg.jointype&(JT_वामः|JT_LTORJ|JT_दक्षिणः))!=0 ); /* By caller */
  testcase( (pSrc->fg.jointype & (JT_वामः|JT_LTORJ|JT_दक्षिणः))==JT_वामः );
  testcase( (pSrc->fg.jointype & (JT_वामः|JT_LTORJ|JT_दक्षिणः))==JT_LTORJ );
  testcase( व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON) )
  testcase( व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_InnerON) );
  यदि( !व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON|EP_InnerON)
   || p_पदांशः->p_व्यञ्जकम्->w.iJoin != pSrc->iCursor
  ){
    प्रत्यागम 0;
  }
  यदि( (pSrc->fg.jointype & (JT_वामः|JT_दक्षिणः))!=0
   && NEVER(व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_InnerON))
  ){
    प्रत्यागम 0;
  }
  प्रत्यागम 1;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOMATIC_सूचकांक
/*
** Return true if column iCol of table pTab seem like it might be a
** good column to use as part of a query-time index.
**
** Current algorithm (subject to improvement!):
**
**   1.   If iCol is already the left-most column of some other index,
**        then return false.
**
**   2.   If iCol is part of an existing index that has an aiRowLogEst of
**        more than 20, then return false.
**
**   3.   If no disqualifying conditions above are found, return true.
**
** 2025-01-03: I experimented with a new rule that returns false if the
** the datatype of the column is "BOOLEAN". This did not improve
** performance on any queries at hand, but it did burn CPU cycles, so the
** idea was not committed.
*/
स्थिर SQLITE_NOINLINE पूर्णाङ्क column_Is_Good_सूचकांक_Candidate(
  ध्रुव सारणी *pTab,
  पूर्णाङ्क iCol
){
  ध्रुव सूचकांक *pIdx;
  प्रति(pIdx = pTab->p_सूचकांक; pIdx!=0; pIdx=pIdx->p_अग्र){
    पूर्णाङ्क j;
    प्रति(j=0; j<pIdx->n_कुञ्चिका_Col; j++){
       यदि( pIdx->aiColumn[j]==iCol ){
         यदि( j==0 ) प्रत्यागम 0;
         यदि( pIdx->hasStat1 && pIdx->aiRowLogEst[j+1]>20 ) प्रत्यागम 0;
         भङ्ग;
       }
    }
  }
  प्रत्यागम 1;
}
#यदि_अन्तः /* SQLITE_OMIT_AUTOMATIC_INDEX */



#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOMATIC_सूचकांक
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
स्थिर पूर्णाङ्क पदांशः_Can_Drive_सूचकांक(
  ध्रुव Where_पदांशः *p_पदांशः,        /* WHERE clause term to check */
  ध्रुव SrcItem *pSrc,           /* Table we are trying to access */
  ध्रुव Bitmask notReady         /* Tables in outer loops of the join */
){
  वर्ण aff;
  पूर्णाङ्क वामः_Col;
  
  यदि( p_पदांशः->वामः_Cursor!=pSrc->iCursor ) प्रत्यागम 0;
  यदि( (p_पदांशः->eOperator & (WO_EQ|WO_IS))==0 ) प्रत्यागम 0;
  प्रतिज्ञा( (pSrc->fg.jointype & JT_दक्षिणः)==0 );
  यदि( (pSrc->fg.jointype & (JT_वामः|JT_LTORJ|JT_दक्षिणः))!=0
   && !constraintCompatibleWithOuterJoin(p_पदांशः,pSrc)
  ){
    प्रत्यागम 0;  /* See https://sqlite.org/forum/forumpost/51e6959f61 */
  }
  यदि( (p_पदांशः->prereq_दक्षिणः & notReady)!=0 ) प्रत्यागम 0;
  प्रतिज्ञा( (p_पदांशः->eOperator & (WO_OR|WO_AND))==0 );
  वामः_Col = p_पदांशः->u.x.वामः_Column;
  यदि( वामः_Col<0 ) प्रत्यागम 0;
  aff = pSrc->pSTab->aCol[वामः_Col].affinity;
  यदि( !sqlite3_सूचकांक_Affinity_Ok(p_पदांशः->p_व्यञ्जकम्, aff) ) प्रत्यागम 0;
  testcase( p_पदांशः->p_व्यञ्जकम्->op==TK_IS );
  प्रत्यागम column_Is_Good_सूचकांक_Candidate(pSrc->pSTab, वामः_Col);
}
#यदि_अन्तः


#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOMATIC_सूचकांक

#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Argument pIdx represents an automatic index that the current statement
** will create and populate. Add an OP_Explain with text of the form:
**
**     CREATE AUTOMATIC INDEX ON <table>(<cols>) [WHERE <expr>]
**
** This is only required if sqlite3_stmt_scanstatus() is enabled, to
** associate an SQLITE_SCANSTAT_NCYCLE and SQLITE_SCANSTAT_NLOOP
** values with. In order to avoid breaking legacy code and test cases,
** the OP_Explain is not added if this is an EXPLAIN QUERY PLAN command.
*/
स्थिर शून्य explain_Automatic_सूचकांक(
  Parse *pParse,
  सूचकांक *pIdx,                    /* Automatic index to explain */
  पूर्णाङ्क bPartial,                   /* True if pIdx is a partial index */
  पूर्णाङ्क *pAddrExplain               /* OUT: Address of OP_Explain */
){
  यदि( IS_STMT_SCANSTATUS(pParse->db) && pParse->explain!=2 ){
    सारणी *pTab = pIdx->p_सारणी;
    ध्रुव वर्ण *zSep = "";
    वर्ण *zText = 0;
    पूर्णाङ्क ii = 0;
    sqlite3_str *pStr = sqlite3_str_new(pParse->db);
    sqlite3_str_appendf(pStr,"CREATE AUTOMATIC INDEX ON %s(", pTab->zName);
    प्रतिज्ञा( pIdx->nColumn>1 );
    प्रतिज्ञा( pIdx->aiColumn[pIdx->nColumn-1]==XN_ROWID || !HasRowid(pTab) );
    प्रति(ii=0; ii<(pIdx->nColumn-1); ii++){
      ध्रुव वर्ण *zName = 0;
      पूर्णाङ्क iCol = pIdx->aiColumn[ii];

      zName = pTab->aCol[iCol].zCnName;
      sqlite3_str_appendf(pStr, "%s%s", zSep, zName);
      zSep = ", ";
    }
    zText = sqlite3_str_finish(pStr);
    यदि( zText==0 ){
      sqlite3OomFault(pParse->db);
    }अन्यथा{
      *pAddrExplain = sqlite3VdbeExplain(
          pParse, 0, "%s)%s", zText, (bPartial ? " WHERE <expr>" : "")
      );
      sqlite3_free(zText);
    }
  }
}
#अन्यथा
# define explain_Automatic_सूचकांक(a,b,c,d)
#यदि_अन्तः

/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
स्थिर SQLITE_NOINLINE शून्य construct_Automatic_सूचकांक(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  ध्रुव Bitmask notReady,     /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  पूर्णाङ्क n_कुञ्चिका_Col;                /* Number of columns in the constructed index */
  Where_पदांशः *p_पदांशः;           /* A single term of the WHERE clause */
  Where_पदांशः *pWCEnd;          /* End of pWC->a[] */
  सूचकांक *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  पूर्णाङ्क addrInit;               /* Address of the initialization bypass jump */
  सारणी *p_सारणी;              /* The table being indexed */
  पूर्णाङ्क addr_शिखरम्;                /* Top of the index fill loop */
  पूर्णाङ्क reg_लेखः;              /* Register holding an index record */
  पूर्णाङ्क n;                      /* Column counter */
  पूर्णाङ्क i;                      /* Loop counter */
  पूर्णाङ्क mxBitCol;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  वर्ण *z_Not_प्रयुक्तम्;             /* Extra space on the end of pIdx */
  Bitmask idx_स्तम्भसङ्ख्या;            /* Bitmap of columns used for indexing */
  Bitmask extra_स्तम्भसङ्ख्या;          /* Bitmap of additional columns */
  u8 प्रेषितम्_Warning = 0;         /* True if a warning has been issued */
  u8 useBloomFilter = 1;      /* True to also add a Bloom filter */
  व्यञ्जकम् *pPartial = 0;         /* Partial Index Expression */
  पूर्णाङ्क iContinue = 0;          /* Jump here to skip excluded rows */
  SrcList *pTabList;          /* The complete FROM clause */
  SrcItem *pSrc;              /* The FROM clause term to get the next index */
  पूर्णाङ्क addr_गणक = 0;        /* Address where integer counter is initialized */
  पूर्णाङ्क reg_आधारः;                /* Array of registers where record is assembled */
#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णाङ्क addrExp = 0;            /* Address of OP_Explain */
#यदि_अन्तः

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  प्रतिज्ञा( v!=0 );
  addrInit = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Once); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  n_कुञ्चिका_Col = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  p_सारणी = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->n_पदांशः];
  pLoop = pLevel->pWLoop;
  idx_स्तम्भसङ्ख्या = 0;
  प्रति(p_पदांशः=pWC->a; p_पदांशः<pWCEnd; p_पदांशः++){
    व्यञ्जकम् *p_व्यञ्जकम् = p_पदांशः->p_व्यञ्जकम्;
    /* Make the automatic index a partial index if there are terms in the
    ** WHERE clause (or the ON clause of a LEFT join) that constrain which
    ** rows of the target table (pSrc) that can be used. */
    यदि( (p_पदांशः->wtFlags & पदांशः_VIRTUAL)==0
     && sqlite3_व्यञ्जकम्_Is_Single_सारणी_Constraint(p_व्यञ्जकम्, pTabList, pLevel->iFrom, 0)
    ){
      pPartial = sqlite3_व्यञ्जकम्_And(pParse, pPartial,
                                sqlite3_व्यञ्जकम्_Dup(pParse->db, p_व्यञ्जकम्, 0));
    }
    यदि( पदांशः_Can_Drive_सूचकांक(p_पदांशः, pSrc, notReady) ){
      पूर्णाङ्क iCol;
      Bitmask c_आवरणम्;
      प्रतिज्ञा( (p_पदांशः->eOperator & (WO_OR|WO_AND))==0 );
      iCol = p_पदांशः->u.x.वामः_Column;
      c_आवरणम् = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS );
      testcase( iCol==BMS-1 );
      यदि( !प्रेषितम्_Warning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", p_सारणी->zName,
            p_सारणी->aCol[iCol].zCnName);
        प्रेषितम्_Warning = 1;
      }
      यदि( (idx_स्तम्भसङ्ख्या & c_आवरणम्)==0 ){
        यदि( whereLoopResize(pParse->db, pLoop, n_कुञ्चिका_Col+1) ){
          गच्छ अन्तः_auto_सूचकांक_create;
        }
        pLoop->aLTerm[n_कुञ्चिका_Col++] = p_पदांशः;
        idx_स्तम्भसङ्ख्या |= c_आवरणम्;
      }
    }
  }
  प्रतिज्ञा( n_कुञ्चिका_Col>0 || pParse->db->mallocFailed );
  pLoop->u.btree.nEq = pLoop->nLTerm = n_कुञ्चिका_Col;
  pLoop->wsFlags = WHERE_COLUMN_EQ | WHERE_IDX_ONLY | WHERE_अनुक्रमितम्
                     | WHERE_AUTO_सूचकांक;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  यदि( IsView(p_सारणी) ){
    extra_स्तम्भसङ्ख्या = ALLBITS & ~idx_स्तम्भसङ्ख्या;
  }अन्यथा{
    extra_स्तम्भसङ्ख्या = pSrc->col_प्रयुक्तम् & (~idx_स्तम्भसङ्ख्या | MASKBIT(BMS-1));
  }
  यदि( !HasRowid(p_सारणी) ){
    /* For WITHOUT ROWID tables, ensure that all PRIMARY KEY columns are
    ** either in the idxCols mask or in the extraCols mask */
    प्रति(i=0; i<p_सारणी->nCol; i++){
      यदि( (p_सारणी->aCol[i].colFlags & COLFLAG_PRIMKEY)==0 ) अनुवर्त;
      यदि( i>=BMS-1 ){
        extra_स्तम्भसङ्ख्या |= MASKBIT(BMS-1);
        भङ्ग;
      }
      यदि( idx_स्तम्भसङ्ख्या & MASKBIT(i) ) अनुवर्त;
      extra_स्तम्भसङ्ख्या |= MASKBIT(i);
    }
  }
  mxBitCol = MIN(BMS-1,p_सारणी->nCol);
  testcase( p_सारणी->nCol==BMS-1 );
  testcase( p_सारणी->nCol==BMS-2 );
  प्रति(i=0; i<mxBitCol; i++){
    यदि( extra_स्तम्भसङ्ख्या & MASKBIT(i) ) n_कुञ्चिका_Col++;
  }
  यदि( pSrc->col_प्रयुक्तम् & MASKBIT(BMS-1) ){
    n_कुञ्चिका_Col += p_सारणी->nCol - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  प्रतिज्ञा( n_कुञ्चिका_Col <= p_सारणी->nCol + MAX(0, p_सारणी->nCol - BMS + 1) );
  /* ^-- This guarantees that the number of index columns will fit in the u16 */
  pIdx = sqlite3_Allocate_सूचकांक_Object(pParse->db, n_कुञ्चिका_Col+HasRowid(p_सारणी),
                                    0, &z_Not_प्रयुक्तम्);
  यदि( pIdx==0 ) गच्छ अन्तः_auto_सूचकांक_create;
  pLoop->u.btree.p_सूचकांक = pIdx;
  pIdx->zName = "auto-index";
  pIdx->p_सारणी = p_सारणी;
  n = 0;
  idx_स्तम्भसङ्ख्या = 0;
  प्रति(p_पदांशः=pWC->a; p_पदांशः<pWCEnd; p_पदांशः++){
    यदि( पदांशः_Can_Drive_सूचकांक(p_पदांशः, pSrc, notReady) ){
      पूर्णाङ्क iCol;
      Bitmask c_आवरणम्;
      प्रतिज्ञा( (p_पदांशः->eOperator & (WO_OR|WO_AND))==0 );
      iCol = p_पदांशः->u.x.वामः_Column;
      c_आवरणम् = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS-1 );
      testcase( iCol==BMS );
      यदि( (idx_स्तम्भसङ्ख्या & c_आवरणम्)==0 ){
        व्यञ्जकम् *pX = p_पदांशः->p_व्यञ्जकम्;
        idx_स्तम्भसङ्ख्या |= c_आवरणम्;
        pIdx->aiColumn[n] = p_पदांशः->u.x.वामः_Column;
        pColl = sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pParse, pX);
        प्रतिज्ञा( pColl!=0 || pParse->nErr>0 ); /* TH3 collate01.800 */
        यदि( !sqlite3_Is_द्विमान(pColl) ){
          /* Disallow the use of a Bloom filter if any non-BINARY collating
          ** sequence is involved.  tag-202607231411 */
          useBloomFilter = 0;
        }
        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3_Str_द्विमान;
        n++;
      }
    }
  }
  प्रतिज्ञा( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  प्रति(i=0; i<mxBitCol; i++){
    यदि( extra_स्तम्भसङ्ख्या & MASKBIT(i) ){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_द्विमान;
      n++;
    }
  }
  यदि( pSrc->col_प्रयुक्तम् & MASKBIT(BMS-1) ){
    प्रति(i=BMS-1; i<p_सारणी->nCol; i++){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3_Str_द्विमान;
      n++;
    }
  }
  प्रतिज्ञा( n==n_कुञ्चिका_Col );
  यदि( HasRowid(p_सारणी) ){
    pIdx->aiColumn[n] = XN_ROWID;
    pIdx->azColl[n] = sqlite3_Str_द्विमान;
  }

  /* Create the automatic index */
  explain_Automatic_सूचकांक(pParse, pIdx, pPartial!=0, &addrExp);
  प्रतिज्ञा( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenAutoindex, pLevel->iIdxCur, n_कुञ्चिका_Col+1);
  sqlite3_Vdbe_समुच्चयः_P4_कुञ्चिका_Info(pParse, pIdx);
  VdbeComment((v, "for %s", p_सारणी->zName));
  यदि( OptimizationEnabled(pParse->db, SQLITE_BloomFilter) && useBloomFilter ){
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Blob, 10000, pLevel->regFilter);
  }

  /* Fill the automatic index with content */
  प्रतिज्ञा( pSrc == &pWC->pWInfo->pTabList->a[pLevel->iFrom] );
  यदि( pSrc->fg.द्वारा_Coroutine ){
    पूर्णाङ्क regYield;
    Subquery *pSubq;
    प्रतिज्ञा( pSrc->fg.isSubquery );
    pSubq = pSrc->u4.pSubq;
    प्रतिज्ञा( pSubq!=0 );
    regYield = pSubq->regReturn;
    addr_गणक = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, 0);
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    addr_शिखरम् =  sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pSrc->pSTab->zName));
  }अन्यथा{
    प्रतिज्ञा( pLevel->addrHalt );
    addr_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rewind,pLevel->iTabCur,pLevel->addrHalt);
    VdbeCoverage(v);
  }
  यदि( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3_व्यञ्जकम्_If_False(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  reg_लेखः = sqlite3_Get_अस्थायी_Reg(pParse);
  reg_आधारः = sqlite3_Generate_सूचकांक_कुञ्चिका(
      pParse, pIdx, pLevel->iTabCur, reg_लेखः, 0, 0, 0, 0
  );
  यदि( pLevel->regFilter ){
    प्रतिज्ञा( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter_सङ्कलय, pLevel->regFilter, 0,
                         reg_आधारः, pLoop->u.btree.nEq);
  }
  sqlite3_Vdbe_अवलोकनम्_स्थितिः_Counters(v, addrExp, addrExp, sqlite3VdbeCurrentAddr(v));
  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Idx_निवेशय, pLevel->iIdxCur, reg_लेखः);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  यदि( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  यदि( pSrc->fg.द्वारा_Coroutine ){
    प्रतिज्ञा( pSrc->fg.isSubquery && pSrc->u4.pSubq!=0 );
    sqlite3VdbeChangeP2(v, addr_गणक, reg_आधारः+n);
    testcase( pParse->db->mallocFailed );
    प्रतिज्ञा( pLevel->iIdxCur>0 );
    translate_Column_गन्तव्यम्_Copy(pParse, addr_शिखरम्, pLevel->iTabCur,
                          pSrc->u4.pSubq->reg_परिणाम, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addr_शिखरम्);
    pSrc->fg.द्वारा_Coroutine = 0;
    sqlite3_Vdbe_Jump_अत्र(v, addr_शिखरम्);
  }अन्यथा{
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, pLevel->iTabCur, addr_शिखरम्+1); VdbeCoverage(v);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    यदि( (pSrc->fg.jointype & JT_वामः)!=0 ){
      sqlite3_Vdbe_Jump_अत्र(v, addr_शिखरम्);
    }
  }
  sqlite3_Release_अस्थायी_Reg(pParse, reg_लेखः);
 
  /* Jump here when skipping the initialization */
  sqlite3_Vdbe_Jump_अत्र(v, addrInit);
  sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExp, addrExp, -1);

अन्तः_auto_सूचकांक_create:
  sqlite3_व्यञ्जकम्_Delete(pParse->db, pPartial);
}
#यदि_अन्तः /* SQLITE_OMIT_AUTOMATIC_INDEX */

/*
** Generate bytecode that will initialize a Bloom filter that is appropriate
** for pLevel.
**
** If there are inner loops within pLevel that have the WHERE_BLOOMFILTER
** flag set, initialize a Bloomfilter for them as well.
**
** When the Bloom filter is initialized, the WHERE_BLOOMFILTER flag is cleared
** from the loop, but the regFilter value is set to a register that implements
** the Bloom filter.  When regFilter is positive, the
** sqlite3WhereCodeOneLoopStart() will generate code to test the Bloom filter
** and skip the subsequence B-Tree seek if the Bloom filter indicates that
** no matching rows exist.
**
** This routine may only be called if it has previously been determined that
** the loop would benefit from a Bloom filter, and the WHERE_BLOOMFILTER bit
** is set.
*/
स्थिर SQLITE_NOINLINE शून्य sqlite3ConstructBloomFilter(
  WhereInfo *pWInfo,    /* The WHERE clause */
  पूर्णाङ्क iLevel,           /* Index in pWInfo->a[] that is pLevel */
  WhereLevel *pLevel,   /* Make a Bloom filter for this FROM term */
  Bitmask notReady      /* Loops that are not ready */
){
  पूर्णाङ्क addrOnce;                        /* Address of opening OP_Once */
  पूर्णाङ्क addr_शिखरम्;                         /* Address of OP_Rewind */
  पूर्णाङ्क addrCont;                        /* Jump here to skip a row */
  ध्रुव Where_पदांशः *p_पदांशः;              /* For looping over WHERE clause terms */
  ध्रुव Where_पदांशः *pWCEnd;             /* Last WHERE clause term */
  Parse *pParse = pWInfo->pParse;      /* Parsing context */
  Vdbe *v = pParse->pVdbe;             /* VDBE under construction */
  WhereLoop *pLoop = pLevel->pWLoop;   /* The loop being coded */
  पूर्णाङ्क iCur;                            /* Cursor for table getting the filter */
  अनुक्रमितम्_व्यञ्जकम् *saved_pIdxEpr;          /* saved copy of Parse.pIdxEpr */
  अनुक्रमितम्_व्यञ्जकम् *saved_p_Idx_Part_व्यञ्जकम्;     /* saved copy of Parse.pIdxPartExpr */

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_p_Idx_Part_व्यञ्जकम् = pParse->p_Idx_Part_व्यञ्जकम्;
  pParse->pIdxEpr = 0;
  pParse->p_Idx_Part_व्यञ्जकम् = 0;

  प्रतिज्ञा( pLoop!=0 );
  प्रतिज्ञा( v!=0 );
  प्रतिज्ञा( pLoop->wsFlags & WHERE_BLOOMFILTER );
  प्रतिज्ञा( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 );

  addrOnce = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Once); VdbeCoverage(v);
  कुरु{
    ध्रुव SrcList *pTabList;
    ध्रुव SrcItem *pItem;
    ध्रुव सारणी *pTab;
    u64 sz;
    पूर्णाङ्क iSrc;
    sqlite3WhereExplainBloomFilter(pParse, pWInfo, pLevel);
    addrCont = sqlite3VdbeMakeLabel(pParse);
    iCur = pLevel->iTabCur;
    pLevel->regFilter = ++pParse->nMem;

    /* The Bloom filter is a Blob held in a register.  Initialize it
    ** to zero-filled blob of at least 80K bits, but maybe more if the
    ** estimated size of the table is larger.  We could actually
    ** measure the size of the table at run-time using OP_Count with
    ** P3==1 and use that value to initialize the blob.  But that makes
    ** testing complicated.  By basing the blob size on the value in the
    ** sqlite_stat1 table, testing is much easier.
    */
    pTabList = pWInfo->pTabList;
    iSrc = pLevel->iFrom;
    pItem = &pTabList->a[iSrc];
    प्रतिज्ञा( pItem!=0 );
    pTab = pItem->pSTab;
    प्रतिज्ञा( pTab!=0 );
    sz = sqlite3_Log_Est_गन्तव्यम्_Int(pTab->nRowLogEst);
    यदि( sz<10000 ){
      sz = 10000;
    }अन्यथा यदि( sz>10000000 ){
      sz = 10000000;
    }
    प्रतिज्ञा( sqlite3WhereLoopBloomable(pLoop) );
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Blob, (पूर्णाङ्क)sz, pLevel->regFilter);

    addr_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.n_पदांशः];
    प्रति(p_पदांशः=pWInfo->sWC.a; p_पदांशः<pWCEnd; p_पदांशः++){
      व्यञ्जकम् *p_व्यञ्जकम् = p_पदांशः->p_व्यञ्जकम्;
      यदि( (p_पदांशः->wtFlags & पदांशः_VIRTUAL)==0
       && sqlite3_व्यञ्जकम्_Is_Single_सारणी_Constraint(p_व्यञ्जकम्, pTabList, iSrc, 0)
      ){
        sqlite3_व्यञ्जकम्_If_False(pParse, p_पदांशः->p_व्यञ्जकम्, addrCont, SQLITE_JUMPIFNULL);
      }
    }
    यदि( pLoop->wsFlags & WHERE_IPK ){
      पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, iCur, r1);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter_सङ्कलय, pLevel->regFilter, 0, r1, 1);
      sqlite3_Release_अस्थायी_Reg(pParse, r1);
    }अन्यथा{
      सूचकांक *pIdx = pLoop->u.btree.p_सूचकांक;
      पूर्णाङ्क n = pLoop->u.btree.nEq;
      पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Range(pParse, n);
      पूर्णाङ्क jj;
      प्रतिज्ञा( pIdx!=0 );
      प्रति(jj=0; jj<n; jj++){
        प्रतिज्ञा( pIdx->p_सारणी==pItem->pSTab );
        sqlite3_व्यञ्जकम्_संहिता_आरोपय_सूचकांक_Column(pParse, pIdx, iCur, jj, r1+jj);
      }
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter_सङ्कलय, pLevel->regFilter, 0, r1, n);
      sqlite3_Release_अस्थायी_Range(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, pLevel->iTabCur, addr_शिखरम्+1);
    VdbeCoverage(v);
    sqlite3_Vdbe_Jump_अत्र(v, addr_शिखरम्);
    pLoop->wsFlags &= ~WHERE_BLOOMFILTER;
    यावत्( ++iLevel < pWInfo->nLevel ){
      ध्रुव SrcItem *pTabItem;
      pLevel = &pWInfo->a[iLevel];
      pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
      यदि( pTabItem->fg.jointype & (JT_वामः|JT_LTORJ) ) अनुवर्त;
      pLoop = pLevel->pWLoop;
      यदि( NEVER(pLoop==0) ) अनुवर्त;
      यदि( pLoop->prereq & notReady ) अनुवर्त;
      यदि( (pLoop->wsFlags & (WHERE_BLOOMFILTER|WHERE_COLUMN_अन्तर्))
                 ==WHERE_BLOOMFILTER
      ){
        /* This is a candidate for bloom-filter pull-down (early evaluation).
        ** The test that WHERE_COLUMN_IN is omitted is important, as we are
        ** not able to do early evaluation of bloom filters that make use of
        ** the IN operator */
        भङ्ग;
      }
    }
  }यावत्( iLevel < pWInfo->nLevel );
  sqlite3_Vdbe_Jump_अत्र(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->p_Idx_Part_व्यञ्जकम् = saved_p_Idx_Part_व्यञ्जकम्;
}


#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
/*
** Return term iTerm of the WhereClause passed as the first argument. Terms
** are numbered from 0 upwards, starting with the terms in pWC->a[], then
** those in pWC->pOuter->a[] (if any), and so on.
*/
स्थिर Where_पदांशः *पदांशः_From_Where_Clause(WhereClause *pWC, पूर्णाङ्क i_पदांशः){
  WhereClause *p;
  प्रति(p=pWC; p; p=p->pOuter){
    यदि( i_पदांशः<p->n_पदांशः ) प्रत्यागम &p->a[i_पदांशः];
    i_पदांशः -= p->n_पदांशः;
  }
  प्रत्यागम 0;
}

/*
** Allocate and populate an sqlite3_index_info structure. It is the
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to freeIndexInfo().
*/
स्थिर sqlite3_सूचकांक_info *allocate_सूचकांक_Info(
  WhereInfo *pWInfo,              /* The WHERE clause */
  WhereClause *pWC,               /* The WHERE clause being analyzed */
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  SrcItem *pSrc,                  /* The FROM clause term that is the vtab */
  u16 *pmNoOmit                   /* Mask of terms not to omit */
){
  पूर्णाङ्क i, j;
  पूर्णाङ्क n_पदांशः;
  Parse *pParse = pWInfo->pParse;
  संरचना sqlite3_सूचकांक_constraint *pIdxCons;
  संरचना sqlite3_सूचकांक_orderby *p_Idx_क्रमः_By;
  संरचना sqlite3_सूचकांक_constraint_usage *pUsage;
  संरचना Hidden_सूचकांक_Info *pHidden;
  Where_पदांशः *p_पदांशः;
  पूर्णाङ्क n_क्रमः_By;
  sqlite3_सूचकांक_info *pIdxInfo;
  u16 mNoOmit = 0;
  ध्रुव सारणी *pTab;
  पूर्णाङ्क eDistinct = 0;
  व्यञ्जकम्_List *p_क्रमः_By = pWInfo->p_क्रमः_By;
  WhereClause *p;

  प्रतिज्ञा( pSrc!=0 );
  pTab = pSrc->pSTab;
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( IsVirtual(pTab) );

  /* Find all WHERE clause constraints referring to this virtual table.
  ** Mark each term with the TERM_OK flag.  Set nTerm to the number of
  ** terms found.
  */
  प्रति(p=pWC, n_पदांशः=0; p; p=p->pOuter){
    प्रति(i=0, p_पदांशः=p->a; i<p->n_पदांशः; i++, p_पदांशः++){
      p_पदांशः->wtFlags &= ~पदांशः_OK;
      यदि( p_पदांशः->वामः_Cursor != pSrc->iCursor ) अनुवर्त;
      यदि( p_पदांशः->prereq_दक्षिणः & mUnusable ) अनुवर्त;
      प्रतिज्ञा( Is_Power_सम्बन्धि_Two(p_पदांशः->eOperator & ~WO_EQUIV) );
      testcase( p_पदांशः->eOperator & WO_अन्तर् );
      testcase( p_पदांशः->eOperator & WO_ISNULL );
      testcase( p_पदांशः->eOperator & WO_IS );
      testcase( p_पदांशः->eOperator & WO_ALL );
      यदि( (p_पदांशः->eOperator & ~(WO_EQUIV))==0 ) अनुवर्त;
      यदि( p_पदांशः->wtFlags & पदांशः_VNULL ) अनुवर्त;

      प्रतिज्ञा( (p_पदांशः->eOperator & (WO_OR|WO_AND))==0 );
      प्रतिज्ञा( p_पदांशः->u.x.वामः_Column>=XN_ROWID );
      प्रतिज्ञा( p_पदांशः->u.x.वामः_Column<pTab->nCol );
      यदि( (pSrc->fg.jointype & (JT_वामः|JT_LTORJ|JT_दक्षिणः))!=0
          && !constraintCompatibleWithOuterJoin(p_पदांशः,pSrc)
        ){
        अनुवर्त;
      }
      n_पदांशः++;
      p_पदांशः->wtFlags |= पदांशः_OK;
    }
  }

  /* If the ORDER BY clause contains only columns in the current
  ** virtual table then allocate space for the aOrderBy part of
  ** the sqlite3_index_info structure.
  */
  n_क्रमः_By = 0;
  यदि( p_क्रमः_By ){
    पूर्णाङ्क n = p_क्रमः_By->n_व्यञ्जकम्;
    प्रति(i=0; i<n; i++){
      व्यञ्जकम् *p_व्यञ्जकम् = p_क्रमः_By->a[i].p_व्यञ्जकम्;
      व्यञ्जकम् *pE2;

      /* Skip over constant terms in the ORDER BY clause */
      यदि( sqlite3_व्यञ्जकम्_Is_Constant(0, p_व्यञ्जकम्) ){
        अनुवर्त;
      }

      /* Virtual tables are unable to deal with NULLS FIRST */
      यदि( p_क्रमः_By->a[i].fg.क्रमय_Flags & KEYINFO_क्रमः_BIGNULL ) भङ्ग;

      /* First case - a direct column references without a COLLATE operator */
      यदि( p_व्यञ्जकम्->op==TK_COLUMN && p_व्यञ्जकम्->i_सारणी==pSrc->iCursor ){
        प्रतिज्ञा( p_व्यञ्जकम्->iColumn>=XN_ROWID && p_व्यञ्जकम्->iColumn<pTab->nCol );
        अनुवर्त;
      }

      /* 2nd case - a column reference with a COLLATE operator.  Only match
      ** of the COLLATE operator matches the collation of the column. */
      यदि( p_व्यञ्जकम्->op==TK_COLLATE
       && (pE2 = p_व्यञ्जकम्->p_वामः)->op==TK_COLUMN
       && pE2->i_सारणी==pSrc->iCursor
      ){
        ध्रुव वर्ण *zColl;  /* The collating sequence name */
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
        प्रतिज्ञा( p_व्यञ्जकम्->u.zToken!=0 );
        प्रतिज्ञा( pE2->iColumn>=XN_ROWID && pE2->iColumn<pTab->nCol );
        p_व्यञ्जकम्->iColumn = pE2->iColumn;
        यदि( pE2->iColumn<0 ) अनुवर्त;  /* Collseq does not matter for rowid */
        zColl = sqlite3ColumnColl(&pTab->aCol[pE2->iColumn]);
        यदि( zColl==0 ) zColl = sqlite3_Str_द्विमान;
        यदि( sqlite3_stricmp(p_व्यञ्जकम्->u.zToken, zColl)==0 ) अनुवर्त;
      }

      /* No matches cause a break out of the loop */
      भङ्ग;
    }
    यदि( i==n ){
      पूर्णाङ्क b_क्रमय_By_गणः = (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)!=0;
      n_क्रमः_By = n;
      यदि( (pWInfo->wctrlFlags & WHERE_DISTINCTBY) && !pSrc->fg.rowid_प्रयुक्तम् ){
        eDistinct = 2 + b_क्रमय_By_गणः;
      }अन्यथा यदि( pWInfo->wctrlFlags & WHERE_GROUPBY ){
        eDistinct = 1 - b_क्रमय_By_गणः;
      }अन्यथा यदि( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
        eDistinct = 3;
      }
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, परिमाण(*pIdxInfo)
                           + (परिमाण(*pIdxCons) + परिमाण(*pUsage))*n_पदांशः
                           + परिमाण(*p_Idx_क्रमः_By)*n_क्रमः_By
                           + SZ_HIDDENINDEXINFO(n_पदांशः) );
  यदि( pIdxInfo==0 ){
    sqlite3_Error_सन्देशः(pParse, "out of memory");
    प्रत्यागम 0;
  }
  pHidden = (संरचना Hidden_सूचकांक_Info*)&pIdxInfo[1];
  pIdxCons = (संरचना sqlite3_सूचकांक_constraint*)&pHidden->aRhs[n_पदांशः];
  p_Idx_क्रमः_By = (संरचना sqlite3_सूचकांक_orderby*)&pIdxCons[n_पदांशः];
  pUsage = (संरचना sqlite3_सूचकांक_constraint_usage*)&p_Idx_क्रमः_By[n_क्रमः_By];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->a_क्रमः_By = p_Idx_क्रमः_By;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->col_प्रयुक्तम् = (sqlite3_int64)pSrc->col_प्रयुक्तम्;
  यदि( HasRowid(pTab)==0 ){
    /* Ensure that all bits associated with PK columns are set. This is to
    ** ensure they are available for cases like RIGHT joins or OR loops. */
    सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक((सारणी*)pTab);
    प्रतिज्ञा( pPk!=0 );
    प्रति(i=0; i<pPk->n_कुञ्चिका_Col; i++){
      पूर्णाङ्क iCol = pPk->aiColumn[i];
      प्रतिज्ञा( iCol>=0 );
      यदि( iCol>=BMS-1 ) iCol = BMS-1;
      pIdxInfo->col_प्रयुक्तम् |= MASKBIT(iCol);
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->m_अन्तर् = 0;
  प्रति(p=pWC, i=j=0; p; p=p->pOuter){
    पूर्णाङ्क n_चरमः = i+p->n_पदांशः;;
    प्रति(p_पदांशः=p->a; i<n_चरमः; i++, p_पदांशः++){
      u16 op;
      यदि( (p_पदांशः->wtFlags & पदांशः_OK)==0 ) अनुवर्त;
      pIdxCons[j].iColumn = p_पदांशः->u.x.वामः_Column;
      pIdxCons[j].i_पदांशः_विस्थापनम् = i;
      op = p_पदांशः->eOperator & WO_ALL;
      यदि( op==WO_अन्तर् ){
        यदि( (p_पदांशः->wtFlags & पदांशः_खण्डिका)==0 ){
          pHidden->m_अन्तर् |= SMASKBIT32(j);
        }
        op = WO_EQ;
      }
      यदि( op==WO_AUX ){
        pIdxCons[j].op = p_पदांशः->e_मेलनम्_Op;
      }अन्यथा यदि( op & (WO_ISNULL|WO_IS) ){
        यदि( op==WO_ISNULL ){
          pIdxCons[j].op = SQLITE_सूचकांक_CONSTRAINT_ISNULL;
        }अन्यथा{
          pIdxCons[j].op = SQLITE_सूचकांक_CONSTRAINT_IS;
        }
      }अन्यथा{
        pIdxCons[j].op = (u8)op;
        /* The direct assignment in the previous line is possible only because
        ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
        ** following asserts verify this fact. */
        प्रतिज्ञा( WO_EQ==SQLITE_सूचकांक_CONSTRAINT_EQ );
        प्रतिज्ञा( WO_LT==SQLITE_सूचकांक_CONSTRAINT_LT );
        प्रतिज्ञा( WO_LE==SQLITE_सूचकांक_CONSTRAINT_LE );
        प्रतिज्ञा( WO_GT==SQLITE_सूचकांक_CONSTRAINT_GT );
        प्रतिज्ञा( WO_GE==SQLITE_सूचकांक_CONSTRAINT_GE );
        प्रतिज्ञा( p_पदांशः->eOperator&(WO_अन्तर्|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_AUX) );

        यदि( op & (WO_LT|WO_LE|WO_GT|WO_GE)
            && sqlite3_व्यञ्जकम्_Is_सदिशम्(p_पदांशः->p_व्यञ्जकम्->p_दक्षिणः)
          ){
          testcase( j!=i );
          यदि( j<16 ) mNoOmit |= (1 << j);
          यदि( op==WO_LT ) pIdxCons[j].op = WO_LE;
          यदि( op==WO_GT ) pIdxCons[j].op = WO_GE;
        }
      }

      j++;
    }
  }
  प्रतिज्ञा( j==n_पदांशः );
  pIdxInfo->nConstraint = j;
  प्रति(i=j=0; i<n_क्रमः_By; i++){
    व्यञ्जकम् *p_व्यञ्जकम् = p_क्रमः_By->a[i].p_व्यञ्जकम्;
    यदि( sqlite3_व्यञ्जकम्_Is_Constant(0, p_व्यञ्जकम्) ) अनुवर्त;
    प्रतिज्ञा( p_व्यञ्जकम्->op==TK_COLUMN
         || (p_व्यञ्जकम्->op==TK_COLLATE && p_व्यञ्जकम्->p_वामः->op==TK_COLUMN
              && p_व्यञ्जकम्->iColumn==p_व्यञ्जकम्->p_वामः->iColumn) );
    p_Idx_क्रमः_By[j].iColumn = p_व्यञ्जकम्->iColumn;
    p_Idx_क्रमः_By[j].desc = p_क्रमः_By->a[i].fg.क्रमय_Flags & KEYINFO_क्रमः_DESC;
    j++;
  }
  pIdxInfo->n_क्रमः_By = j;

  *pmNoOmit = mNoOmit;
  प्रत्यागम pIdxInfo;
}

/*
** Free and zero the sqlite3_index_info.idxStr value if needed.
*/
स्थिर शून्य freeIdxStr(sqlite3_सूचकांक_info *pIdxInfo){
  यदि( pIdxInfo->need_गन्तव्यम्_Free_Idx_Str ){
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->need_गन्तव्यम्_Free_Idx_Str = 0;
  }
}  

/*
** Free an sqlite3_index_info structure allocated by allocateIndexInfo()
** and possibly modified by xBestIndex methods.
*/
स्थिर शून्य free_सूचकांक_Info(sqlite3 *db, sqlite3_सूचकांक_info *pIdxInfo){
  Hidden_सूचकांक_Info *pHidden;
  पूर्णाङ्क i;
  प्रतिज्ञा( pIdxInfo!=0 );
  pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  प्रतिज्ञा( pHidden->pParse!=0 );
  प्रतिज्ञा( pHidden->pParse->db==db );
  प्रति(i=0; i<pIdxInfo->nConstraint; i++){
    sqlite3_मानम्_Free(pHidden->aRhs[i]); /* IMP: R-14553-25174 */
    pHidden->aRhs[i] = 0;
  }
  freeIdxStr(pIdxInfo);
  sqlite3DbFree(db, pIdxInfo);
}

/*
** The table object reference passed as the second argument to this function
** must represent a virtual table. This function invokes the xBestIndex()
** method of the virtual table with the sqlite3_index_info object that
** comes in as the 3rd argument to this function.
**
** If an error occurs, pParse is populated with an error message and an
** appropriate error code is returned.  A return of SQLITE_CONSTRAINT from
** xBestIndex is not considered an error.  SQLITE_CONSTRAINT indicates that
** the current configuration of "unusable" flags in sqlite3_index_info can
** not result in a valid plan.
**
** Whether or not an error is returned, it is the responsibility of the
** caller to eventually free p->idxStr if p->needToFreeIdxStr indicates
** that this is required.
*/
स्थिर पूर्णाङ्क vtab_Best_सूचकांक(Parse *pParse, सारणी *pTab, sqlite3_सूचकांक_info *p){
  पूर्णाङ्क rc;
  sqlite3_vtab *pVtab;

  प्रतिज्ञा( IsVirtual(pTab) );
  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  where_Trace_सूचकांक_Info_Inputs(p, pTab);
  pParse->db->n_Schema_तालकम्++;
  rc = pVtab->pModule->x_Best_सूचकांक(pVtab, p);
  pParse->db->n_Schema_तालकम्--;
  where_Trace_सूचकांक_Info_Outputs(p, pTab);

  यदि( rc!=SQLITE_OK && rc!=SQLITE_CONSTRAINT ){
    यदि( rc==SQLITE_NOMEM ){
      sqlite3OomFault(pParse->db);
    }अन्यथा यदि( !pVtab->z_Err_सन्देशः ){
      sqlite3_Error_सन्देशः(pParse, "%s", sqlite3ErrStr(rc));
    }अन्यथा{
      sqlite3_Error_सन्देशः(pParse, "%s", pVtab->z_Err_सन्देशः);
    }
  }
  यदि( pTab->u.vtab.p->bAllSchemas ){
    sqlite3_Vtab_उपयुङ्क्ते_All_Schemas(pParse);
  }
  sqlite3_free(pVtab->z_Err_सन्देशः);
  pVtab->z_Err_सन्देशः = 0;
  प्रत्यागम rc;
}
#यदि_अन्तः /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
/*
** Estimate the location of a particular key among all keys in an
** index.  Store the results in aStat as follows:
**
**    aStat[0]      Est. number of rows less than pRec
**    aStat[1]      Est. number of rows equal to pRec
**
** Return the index of the sample that is the smallest sample that
** is greater than or equal to pRec. Note that this index is not an index
** into the aSample[] array - it is an index into a virtual set of samples
** based on the contents of aSample[] and the number of fields in record
** pRec.
*/
स्थिर पूर्णाङ्क where_कुञ्चिका_Stats(
  Parse *pParse,              /* Database connection */
  सूचकांक *pIdx,                /* Index to consider domain of */
  Unpacked_लेखः *pRec,       /* Vector of values to consider */
  पूर्णाङ्क आवर्तनम्_ऊर्ध्वम्,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  सूचकांक_Sample *aSample = pIdx->aSample;
  पूर्णाङ्क iCol;                   /* Index of required stats in anEq[] etc. */
  पूर्णाङ्क i;                      /* Index of first sample >= pRec */
  पूर्णाङ्क iSample;                /* Smallest sample larger than or equal to pRec */
  पूर्णाङ्क iMin = 0;               /* Smallest sample not yet tested */
  पूर्णाङ्क iTest;                  /* Next sample to test */
  पूर्णाङ्क res;                    /* Result of comparison operation */
  पूर्णाङ्क n_क्षेत्रम्;                 /* Number of fields in pRec */
  tRowcnt i_अधस्तनम् = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#यदि_अपरिभाषितम् SQLITE_DEBUG
  अप्रयुक्तम्_PARAMETER( pParse );
#यदि_अन्तः
  प्रतिज्ञा( pRec!=0 );
  प्रतिज्ञा( pIdx->nSample>0 );
  प्रतिज्ञा( pRec->n_क्षेत्रम्>0 );


  /* Do a binary search to find the first sample greater than or equal
  ** to pRec. If pRec contains a single field, the set of samples to search
  ** is simply the aSample[] array. If the samples in aSample[] contain more
  ** than one fields, all fields following the first are ignored.
  **
  ** If pRec contains N fields, where N is more than one, then as well as the
  ** samples in aSample[] (truncated to N fields), the search also has to
  ** consider prefixes of those samples. For example, if the set of samples
  ** in aSample is:
  **
  **     aSample[0] = (a, 5)
  **     aSample[1] = (a, 10)
  **     aSample[2] = (b, 5)
  **     aSample[3] = (c, 100)
  **     aSample[4] = (c, 105)
  **
  ** Then the search space should ideally be the samples above and the
  ** unique prefixes [a], [b] and [c]. But since that is hard to organize,
  ** the code actually searches this set:
  **
  **     0: (a)
  **     1: (a, 5)
  **     2: (a, 10)
  **     3: (a, 10)
  **     4: (b)
  **     5: (b, 5)
  **     6: (c)
  **     7: (c, 100)
  **     8: (c, 105)
  **     9: (c, 105)
  **
  ** For each sample in the aSample[] array, N samples are present in the
  ** effective sample array. In the above, samples 0 and 1 are based on
  ** sample aSample[0]. Samples 2 and 3 on aSample[1] etc.
  **
  ** Often, sample i of each block of N effective samples has (i+1) fields.
  ** Except, each sample may be extended to ensure that it is greater than or
  ** equal to the previous sample in the array. For example, in the above,
  ** sample 2 is the first sample of a block of N samples, so at first it
  ** appears that it should be 1 field in size. However, that would make it
  ** smaller than sample 1, so the binary search would not work. As a result,
  ** it is extended to two fields. The duplicates that this creates do not
  ** cause any problems.
  */
  यदि( !HasRowid(pIdx->p_सारणी) && Is_Primary_कुञ्चिका_सूचकांक(pIdx) ){
    n_क्षेत्रम् = pIdx->n_कुञ्चिका_Col;
  }अन्यथा{
    n_क्षेत्रम् = pIdx->nColumn;
  }
  n_क्षेत्रम् = MIN(pRec->n_क्षेत्रम्, n_क्षेत्रम्);
  iCol = 0;
  iSample = pIdx->nSample * n_क्षेत्रम्;
  कुरु{
    पूर्णाङ्क iSamp;                    /* Index in aSample[] of test sample */
    पूर्णाङ्क n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / n_क्षेत्रम्;
    यदि( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField)
      ** fields that is greater than the previous effective sample.  */
      प्रति(n=(iTest % n_क्षेत्रम्) + 1; n<n_क्षेत्रम्; n++){
        यदि( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) भङ्ग;
      }
    }अन्यथा{
      n = iTest + 1;
    }

    pRec->n_क्षेत्रम् = n;
    res = sqlite3_Vdbe_लेखः_Compare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    यदि( res<0 ){
      i_अधस्तनम् = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }अन्यथा यदि( res==0 && n<n_क्षेत्रम् ){
      i_अधस्तनम् = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }अन्यथा{
      iSample = iTest;
      iCol = n-1;
    }
  }यावत्( res && iMin<iSample );
  i = iSample / n_क्षेत्रम्;

#यदि_परिभाषितम् SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  यदि( pParse->db->mallocFailed==0 ){
    यदि( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      प्रतिज्ञा( i<pIdx->nSample );
      प्रतिज्ञा( iCol==n_क्षेत्रम्-1 );
      pRec->n_क्षेत्रम् = n_क्षेत्रम्;
      प्रतिज्ञा( 0==sqlite3_Vdbe_लेखः_Compare(aSample[i].n, aSample[i].p, pRec)
           || pParse->db->mallocFailed
      );
    }अन्यथा{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      प्रतिज्ञा( i<=pIdx->nSample && i>=0 );
      pRec->n_क्षेत्रम् = iCol+1;
      प्रतिज्ञा( i==pIdx->nSample
           || sqlite3_Vdbe_लेखः_Compare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      यदि( iCol>0 ){
        pRec->n_क्षेत्रम् = iCol;
        प्रतिज्ञा( sqlite3_Vdbe_लेखः_Compare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
      यदि( i>0 ){
        pRec->n_क्षेत्रम् = n_क्षेत्रम्;
        प्रतिज्ञा( sqlite3_Vdbe_लेखः_Compare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed || CORRUPT_DB );
      }
    }
  }
#यदि_अन्तः /* ifdef SQLITE_DEBUG */

  यदि( res==0 ){
    /* Record pRec is equal to sample i */
    प्रतिज्ञा( iCol==n_क्षेत्रम्-1 );
    aStat[0] = aSample[i].anLt[iCol];
    aStat[1] = aSample[i].anEq[iCol];
  }अन्यथा{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt i_ऊर्ध्वस्थम्, iGap;
    यदि( i>=pIdx->nSample ){
      i_ऊर्ध्वस्थम् = pIdx->nRowEst0;
    }अन्यथा{
      i_ऊर्ध्वस्थम् = aSample[i].anLt[iCol];
    }

    यदि( i_अधस्तनम्>=i_ऊर्ध्वस्थम् ){
      iGap = 0;
    }अन्यथा{
      iGap = i_ऊर्ध्वस्थम् - i_अधस्तनम्;
    }
    यदि( आवर्तनम्_ऊर्ध्वम् ){
      iGap = (iGap*2)/3;
    }अन्यथा{
      iGap = iGap/3;
    }
    aStat[0] = i_अधस्तनम् + iGap;
    aStat[1] = pIdx->aAvgEq[n_क्षेत्रम्-1];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->n_क्षेत्रम् = n_क्षेत्रम्;
  प्रत्यागम i;
}
#यदि_अन्तः /* SQLITE_ENABLE_STAT4 */

/*
** If it is not NULL, pTerm is a term that provides an upper or lower
** bound on a range scan. Without considering pTerm, it is estimated
** that the scan will visit nNew rows. This function returns the number
** estimated to be visited after taking pTerm into account.
**
** If the user explicitly specified a likelihood() value for this term,
** then the return value is the likelihood multiplied by the number of
** input rows. Otherwise, this function assumes that an "IS NOT NULL" term
** has a likelihood of 0.50, and any other term a likelihood of 0.25.
*/
स्थिर LogEst whereRangeAdjust(Where_पदांशः *p_पदांशः, LogEst nNew){
  LogEst nRet = nNew;
  यदि( p_पदांशः ){
    यदि( p_पदांशः->truthProb<=0 ){
      nRet += p_पदांशः->truthProb;
    }अन्यथा यदि( (p_पदांशः->wtFlags & पदांशः_VNULL)==0 ){
      nRet -= 20;        प्रतिज्ञा( 20==sqlite3LogEst(4) );
    }
  }
  प्रत्यागम nRet;
}


#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
/*
** Return the affinity for a single column of an index.
*/
वर्ण sqlite3_सूचकांक_Column_Affinity(sqlite3 *db, सूचकांक *pIdx, पूर्णाङ्क iCol){
  प्रतिज्ञा( iCol>=0 && iCol<pIdx->nColumn );
  यदि( !pIdx->zColAff ){
    यदि( sqlite3_सूचकांक_Affinity_Str(db, pIdx)==0 ) प्रत्यागम SQLITE_AFF_BLOB;
  }
  प्रतिज्ञा( pIdx->zColAff[iCol]!=0 );
  प्रत्यागम pIdx->zColAff[iCol];
}
#यदि_अन्तः


#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
/*
** This function is called to estimate the number of rows visited by a
** range-scan on a skip-scan index. For example:
**
**   CREATE INDEX i1 ON t1(a, b, c);
**   SELECT * FROM t1 WHERE a=? AND c BETWEEN ? AND ?;
**
** Value pLoop->nOut is currently set to the estimated number of rows
** visited for scanning (a=? AND b=?). This function reduces that estimate
** by some factor to account for the (c BETWEEN ? AND ?) expression based
** on the stat4 data for the index. this scan will be performed multiple
** times (once for each (a,b) combination that matches a=?) is dealt with
** by the caller.
**
** It does this by scanning through all stat4 samples, comparing values
** extracted from pLower and pUpper with the corresponding column in each
** sample. If L and U are the number of samples found to be less than or
** equal to the values extracted from pLower and pUpper respectively, and
** N is the total number of samples, the pLoop->nOut value is adjusted
** as follows:
**
**   nOut = nOut * ( min(U - L, 1) / N )
**
** If pLower is NULL, or a value cannot be extracted from the term, L is
** set to zero. If pUpper is NULL, or a value cannot be extracted from it,
** U is set to N.
**
** Normally, this function sets *pbDone to 1 before returning. However,
** if no value can be extracted from either pLower or pUpper (and so the
** estimate of the number of rows delivered remains unchanged), *pbDone
** is left as is.
**
** If an error occurs, an SQLite error code is returned. Otherwise,
** SQLITE_OK.
*/
स्थिर पूर्णाङ्क where_Range_Skip_अवलोकनम्_Est(
  Parse *pParse,       /* Parsing & code generating context */
  Where_पदांशः *p_अधस्तनम्,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_पदांशः *p_ऊर्ध्वस्थम्,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  पूर्णाङ्क *pb_समाप्तम्          /* Set to true if at least one expr. value extracted */
){
  सूचकांक *p = pLoop->u.btree.p_सूचकांक;
  पूर्णाङ्क nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क n_अधस्तनम् = -1;
  पूर्णाङ्क n_ऊर्ध्वस्थम् = p->nSample+1;
  पूर्णाङ्क rc = SQLITE_OK;
  u8 aff = sqlite3_सूचकांक_Column_Affinity(db, p, nEq);
  CollSeq *pColl;
 
  sqlite3_मानम् *p1 = 0;          /* Value extracted from pLower */
  sqlite3_मानम् *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_मानम् *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  यदि( p_अधस्तनम् ){
    rc = sqlite3_Stat4_मानम्_From_व्यञ्जकम्(pParse, p_अधस्तनम्->p_व्यञ्जकम्->p_दक्षिणः, aff, &p1);
    n_अधस्तनम् = 0;
  }
  यदि( p_ऊर्ध्वस्थम् && rc==SQLITE_OK ){
    rc = sqlite3_Stat4_मानम्_From_व्यञ्जकम्(pParse, p_ऊर्ध्वस्थम्->p_व्यञ्जकम्->p_दक्षिणः, aff, &p2);
    n_ऊर्ध्वस्थम् = p2 ? 0 : p->nSample;
  }

  यदि( p1 || p2 ){
    पूर्णाङ्क i;
    पूर्णाङ्क nDiff;
    प्रति(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3Stat4Column(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      यदि( rc==SQLITE_OK && p1 ){
        पूर्णाङ्क res = sqlite3MemCompare(p1, pVal, pColl);
        यदि( res>=0 ) n_अधस्तनम्++;
      }
      यदि( rc==SQLITE_OK && p2 ){
        पूर्णाङ्क res = sqlite3MemCompare(p2, pVal, pColl);
        यदि( res>=0 ) n_ऊर्ध्वस्थम्++;
      }
    }
    nDiff = (n_ऊर्ध्वस्थम् - n_अधस्तनम्);
    यदि( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    यदि( nDiff!=1 || p_ऊर्ध्वस्थम्==0 || p_अधस्तनम्==0 ){
      पूर्णाङ्क nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->n_बहिर्गमनम् -= nAdjust;
      *pb_समाप्तम् = 1;
      WHERETRACE(0x20, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           n_अधस्तनम्, n_ऊर्ध्वस्थम्, nAdjust*-1, pLoop->n_बहिर्गमनम्));
    }

  }अन्यथा{
    प्रतिज्ञा( *pb_समाप्तम्==0 );
  }

  sqlite3_मानम्_Free(p1);
  sqlite3_मानम्_Free(p2);
  sqlite3_मानम्_Free(pVal);

  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_ENABLE_STAT4 */

/*
** This function is used to estimate the number of rows that will be visited
** by scanning an index for a range of values. The range may have an upper
** bound, a lower bound, or both. The WHERE clause terms that set the upper
** and lower bounds are represented by pLower and pUpper respectively. For
** example, assuming that index p is on t1(a):
**
**   ... FROM t1 WHERE a > ? AND a < ? ...
**                    |_____|   |_____|
**                       |         |
**                     pLower    pUpper
**
** If either of the upper or lower bound is not present, then NULL is passed in
** place of the corresponding WhereTerm.
**
** The value in (pBuilder->pNew->u.btree.nEq) is the number of the index
** column subject to the range constraint. Or, equivalently, the number of
** equality constraints optimized by the proposed index scan. For example,
** assuming index p is on t1(a, b), and the SQL query is:
**
**   ... FROM t1 WHERE a = ? AND b > ? AND b < ? ...
**
** then nEq is set to 1 (as the range restricted column, b, is the second
** left-most column of the index). Or, if the query is:
**
**   ... FROM t1 WHERE a > ? AND a < ? ...
**
** then nEq is set to 0.
**
** When this function is called, *pnOut is set to the sqlite3LogEst() of the
** number of rows that the index scan is expected to visit without
** considering the range constraints. If nEq is 0, then *pnOut is the number of
** rows in the index. Assuming no error occurs, *pnOut is adjusted (reduced)
** to account for the range constraints pLower and pUpper.
**
** In the absence of sqlite_stat4 ANALYZE data, or if such data cannot be
** used, a single range inequality reduces the search space by a factor of 4.
** and a pair of constraints (x>? AND x<?) reduces the expected number of
** rows visited by a factor of 64.
*/
स्थिर पूर्णाङ्क where_Range_अवलोकनम्_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  Where_पदांशः *p_अधस्तनम्,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  Where_पदांशः *p_ऊर्ध्वस्थम्,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क n_बहिर्गमनम् = pLoop->n_बहिर्गमनम्;
  LogEst nNew;

#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
  सूचकांक *p = pLoop->u.btree.p_सूचकांक;
  पूर्णाङ्क nEq = pLoop->u.btree.nEq;

  यदि( p->nSample>0 && ALWAYS(nEq<p->nSampleCol)
   && OptimizationEnabled(pParse->db, SQLITE_Stat4)
  ){
    यदि( nEq==pBuilder->nRecValid ){
      Unpacked_लेखः *pRec = pBuilder->pRec;
      tRowcnt a[2];
      पूर्णाङ्क nBtm = pLoop->u.btree.nBtm;
      पूर्णाङ्क n_शिखरम् = pLoop->u.btree.n_शिखरम्;

      /* Variable iLower will be set to the estimate of the number of rows in
      ** the index that are less than the lower bound of the range query. The
      ** lower bound being the concatenation of $P and $L, where $P is the
      ** key-prefix formed by the nEq values matched against the nEq left-most
      ** columns of the index, and $L is the value in pLower.
      **
      ** Or, if pLower is NULL or $L cannot be extracted from it (because it
      ** is not a simple variable or literal value), the lower bound of the
      ** range is $P. Due to a quirk in the way whereKeyStats() works, even
      ** if $L is available, whereKeyStats() is called for both ($P) and
      ** ($P:$L) and the larger of the two returned values is used.
      **
      ** Similarly, iUpper is to be set to the estimate of the number of rows
      ** less than the upper bound of the range query. Where the upper bound
      ** is either ($P) or ($P:$U). Again, even if $U is available, both values
      ** of iUpper are requested of whereKeyStats() and the smaller used.
      **
      ** The number of rows between the two bounds is then just iUpper-iLower.
      */
      tRowcnt i_अधस्तनम्;     /* Rows less than the lower bound */
      tRowcnt i_ऊर्ध्वस्थम्;     /* Rows less than the upper bound */
      पूर्णाङ्क iLwrIdx = -2;   /* aSample[] for the lower bound */
      पूर्णाङ्क iUprIdx = -1;   /* aSample[] for the upper bound */

      यदि( pRec ){
        testcase( pRec->n_क्षेत्रम्!=pBuilder->nRecValid );
        pRec->n_क्षेत्रम् = pBuilder->nRecValid;
      }
      /* Determine iLower and iUpper using ($P) only. */
      यदि( nEq==0 ){
        i_अधस्तनम् = 0;
        i_ऊर्ध्वस्थम् = p->nRowEst0;
      }अन्यथा{
        /* Note: this call could be optimized away - since the same values must
        ** have been requested when testing key $P in whereEqualScanEst().  */
        where_कुञ्चिका_Stats(pParse, p, pRec, 0, a);
        i_अधस्तनम् = a[0];
        i_ऊर्ध्वस्थम् = a[0] + a[1];
      }

      प्रतिज्ञा( p_अधस्तनम्==0 || (p_अधस्तनम्->eOperator & (WO_GT|WO_GE))!=0 );
      प्रतिज्ञा( p_ऊर्ध्वस्थम्==0 || (p_ऊर्ध्वस्थम्->eOperator & (WO_LT|WO_LE))!=0 );
      प्रतिज्ञा( p->a_क्रमय_क्रमः!=0 );
      यदि( p->a_क्रमय_क्रमः[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        SWAP(Where_पदांशः*, p_अधस्तनम्, p_ऊर्ध्वस्थम्);
        SWAP(पूर्णाङ्क, nBtm, n_शिखरम्);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      यदि( p_अधस्तनम् ){
        पूर्णाङ्क n;                    /* Values extracted from pExpr */
        व्यञ्जकम् *p_व्यञ्जकम् = p_अधस्तनम्->p_व्यञ्जकम्->p_दक्षिणः;
        rc = sqlite3_Stat4_Probe_समुच्चयः_मानम्(pParse, p, &pRec, p_व्यञ्जकम्, nBtm, nEq, &n);
        यदि( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 आवरणम् = WO_GT|WO_LE;
          यदि( sqlite3_व्यञ्जकम्_सदिशम्_आकार(p_व्यञ्जकम्)>n ) आवरणम् = (WO_LE|WO_LT);
          iLwrIdx = where_कुञ्चिका_Stats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((p_अधस्तनम्->eOperator & आवरणम्) ? a[1] : 0);
          यदि( iNew>i_अधस्तनम् ) i_अधस्तनम् = iNew;
          n_बहिर्गमनम्--;
          p_अधस्तनम् = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      यदि( p_ऊर्ध्वस्थम् ){
        पूर्णाङ्क n;                    /* Values extracted from pExpr */
        व्यञ्जकम् *p_व्यञ्जकम् = p_ऊर्ध्वस्थम्->p_व्यञ्जकम्->p_दक्षिणः;
        rc = sqlite3_Stat4_Probe_समुच्चयः_मानम्(pParse, p, &pRec, p_व्यञ्जकम्, n_शिखरम्, nEq, &n);
        यदि( rc==SQLITE_OK && n ){
          tRowcnt iNew;
          u16 आवरणम् = WO_GT|WO_LE;
          यदि( sqlite3_व्यञ्जकम्_सदिशम्_आकार(p_व्यञ्जकम्)>n ) आवरणम् = (WO_LE|WO_LT);
          iUprIdx = where_कुञ्चिका_Stats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((p_ऊर्ध्वस्थम्->eOperator & आवरणम्) ? a[1] : 0);
          यदि( iNew<i_ऊर्ध्वस्थम् ) i_ऊर्ध्वस्थम् = iNew;
          n_बहिर्गमनम्--;
          p_ऊर्ध्वस्थम् = 0;
        }
      }

      pBuilder->pRec = pRec;
      यदि( rc==SQLITE_OK ){
        यदि( i_ऊर्ध्वस्थम्>i_अधस्तनम् ){
          nNew = sqlite3LogEst(i_ऊर्ध्वस्थम् - i_अधस्तनम्);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT4 tables. */
          यदि( iLwrIdx==iUprIdx ){ nNew -= 20; }
          प्रतिज्ञा( 20==sqlite3LogEst(4) );
        }अन्यथा{
          nNew = 10;        प्रतिज्ञा( 10==sqlite3LogEst(2) );
        }
        यदि( nNew<n_बहिर्गमनम् ){
          n_बहिर्गमनम् = nNew;
        }
        WHERETRACE(0x20, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)i_अधस्तनम्, (u32)i_ऊर्ध्वस्थम्, n_बहिर्गमनम्));
      }
    }अन्यथा{
      पूर्णाङ्क b_समाप्तम् = 0;
      rc = where_Range_Skip_अवलोकनम्_Est(pParse, p_अधस्तनम्, p_ऊर्ध्वस्थम्, pLoop, &b_समाप्तम्);
      यदि( b_समाप्तम् ) प्रत्यागम rc;
    }
  }
#अन्यथा
  अप्रयुक्तम्_PARAMETER(pParse);
  अप्रयुक्तम्_PARAMETER(pBuilder);
  प्रतिज्ञा( p_अधस्तनम् || p_ऊर्ध्वस्थम् );
#यदि_अन्तः
  प्रतिज्ञा( p_ऊर्ध्वस्थम्==0 || (p_ऊर्ध्वस्थम्->wtFlags & पदांशः_VNULL)==0 || pParse->nErr>0 );
  nNew = whereRangeAdjust(p_अधस्तनम्, n_बहिर्गमनम्);
  nNew = whereRangeAdjust(p_ऊर्ध्वस्थम्, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */
  यदि( p_अधस्तनम् && p_अधस्तनम्->truthProb>0 && p_ऊर्ध्वस्थम् && p_ऊर्ध्वस्थम्->truthProb>0 ){
    nNew -= 20;
  }

  n_बहिर्गमनम् -= (p_अधस्तनम्!=0) + (p_ऊर्ध्वस्थम्!=0);
  यदि( nNew<10 ) nNew = 10;
  यदि( nNew<n_बहिर्गमनम् ) n_बहिर्गमनम् = nNew;
#यदि defined(WHERETRACE_ENABLED)
  यदि( pLoop->n_बहिर्गमनम्>n_बहिर्गमनम् ){
    WHERETRACE(0x20,("Range scan lowers nOut from %d to %d\n",
                    pLoop->n_बहिर्गमनम्, n_बहिर्गमनम्));
  }
#यदि_अन्तः
  pLoop->n_बहिर्गमनम् = (LogEst)n_बहिर्गमनम्;
  प्रत्यागम rc;
}

#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
/*
** Estimate the number of rows that will be returned based on
** an equality constraint x=VALUE and where that VALUE occurs in
** the histogram data.  This only works when x is the left-most
** column of an index and sqlite_stat4 histogram data is available
** for that index.  When pExpr==NULL that means the constraint is
** "x IS NULL" instead of "x=VALUE".
**
** Write the estimated row count into *pnRow and return SQLITE_OK.
** If unable to make an estimate, leave *pnRow unchanged and return
** non-zero.
**
** This routine can fail if it is unable to load a collating sequence
** required for string comparison, or if unable to allocate memory
** for a UTF conversion required for comparison.  The error is stored
** in the pParse structure.
*/
स्थिर पूर्णाङ्क where_Equal_अवलोकनम्_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  व्यञ्जकम् *p_व्यञ्जकम्,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  सूचकांक *p = pBuilder->pNew->u.btree.p_सूचकांक;
  पूर्णाङ्क nEq = pBuilder->pNew->u.btree.nEq;
  Unpacked_लेखः *pRec = pBuilder->pRec;
  पूर्णाङ्क rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  पूर्णाङ्क bOk;

  प्रतिज्ञा( nEq>=1 );
  प्रतिज्ञा( nEq<=p->nColumn );
  प्रतिज्ञा( p->aSample!=0 );
  प्रतिज्ञा( p->nSample>0 );
  प्रतिज्ञा( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  यदि( pBuilder->nRecValid<(nEq-1) ){
    प्रत्यागम SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  यदि( nEq>=p->nColumn ){
    *pnRow = 1;
    प्रत्यागम SQLITE_OK;
  }

  rc = sqlite3_Stat4_Probe_समुच्चयः_मानम्(pParse, p, &pRec, p_व्यञ्जकम्, 1, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
  यदि( bOk==0 ) प्रत्यागम SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  where_कुञ्चिका_Stats(pParse, p, pRec, 0, a);
  WHERETRACE(0x20,("equality scan regions %s(%d): %d\n",
                   p->zName, nEq-1, (पूर्णाङ्क)a[1]));
  *pnRow = a[1];
 
  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_ENABLE_STAT4 */

#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
/*
** Estimate the number of rows that will be returned based on
** an IN constraint where the right-hand side of the IN operator
** is a list of values.  Example:
**
**        WHERE x IN (1,2,3,4)
**
** Write the estimated row count into *pnRow and return SQLITE_OK.
** If unable to make an estimate, leave *pnRow unchanged and return
** non-zero.
**
** This routine can fail if it is unable to load a collating sequence
** required for string comparison, or if unable to allocate memory
** for a UTF conversion required for comparison.  The error is stored
** in the pParse structure.
*/
स्थिर पूर्णाङ्क where_अन्तर्_अवलोकनम्_Est(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  व्यञ्जकम्_List *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  सूचकांक *p = pBuilder->pNew->u.btree.p_सूचकांक;
  i64 nRow0 = sqlite3_Log_Est_गन्तव्यम्_Int(p->aiRowLogEst[0]);
  पूर्णाङ्क nRecValid = pBuilder->nRecValid;
  पूर्णाङ्क rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt nRowEst = 0;    /* New estimate of the number of rows */
  पूर्णाङ्क i;                  /* Loop counter */

  प्रतिज्ञा( p->aSample!=0 );
  प्रति(i=0; rc==SQLITE_OK && i<pList->n_व्यञ्जकम्; i++){
    nEst = nRow0;
    rc = where_Equal_अवलोकनम्_Est(pParse, pBuilder, pList->a[i].p_व्यञ्जकम्, &nEst);
    nRowEst += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  यदि( rc==SQLITE_OK ){
    यदि( nRowEst > (tRowcnt)nRow0 ) nRowEst = nRow0;
    *pnRow = nRowEst;
    WHERETRACE(0x20,("IN row estimate: est=%d\n", nRowEst));
  }
  प्रतिज्ञा( pBuilder->nRecValid==nRecValid );
  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_ENABLE_STAT4 */


#यदि defined(WHERETRACE_ENABLED) || defined(SQLITE_DEBUG)
/*
** Print the content of a WhereTerm object
*/
शून्य sqlite3_Where_पदांशः_Print(Where_पदांशः *p_पदांशः, पूर्णाङ्क i_पदांशः){
  यदि( p_पदांशः==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", i_पदांशः);
  }अन्यथा{
    वर्ण zType[8];
    वर्ण z_वामः[50];
    स्मृतिं_प्रतिलिख(zType, "....", 5);
    यदि( p_पदांशः->wtFlags & पदांशः_VIRTUAL ) zType[0] = 'V';
    यदि( p_पदांशः->eOperator & WO_EQUIV  ) zType[1] = 'E';
    यदि( व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON) ) zType[2] = 'L';
    यदि( p_पदांशः->wtFlags & पदांशः_CODED  ) zType[3] = 'C';
    यदि( p_पदांशः->eOperator & WO_SINGLE ){
      प्रतिज्ञा( (p_पदांशः->eOperator & (WO_OR|WO_AND))==0 );
      sqlite3_snprintf(परिमाण(z_वामः),z_वामः,"left={%d:%d}",
                       p_पदांशः->वामः_Cursor, p_पदांशः->u.x.वामः_Column);
    }अन्यथा यदि( (p_पदांशः->eOperator & WO_OR)!=0 && p_पदांशः->u.pOrInfo!=0 ){
      sqlite3_snprintf(परिमाण(z_वामः),z_वामः,"indexable=0x%llx",
                       p_पदांशः->u.pOrInfo->indexable);
    }अन्यथा{
      sqlite3_snprintf(परिमाण(z_वामः),z_वामः,"left=%d", p_पदांशः->वामः_Cursor);
    }
    i_पदांशः = p_पदांशः->i_पदांशः = MAX(i_पदांशः,p_पदांशः->i_पदांशः);
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s %-12s op=%03x wtFlags=%04x",
       i_पदांशः, p_पदांशः, zType, z_वामः, p_पदांशः->eOperator, p_पदांशः->wtFlags);
    /* The 0x10000 .wheretrace flag causes extra information to be
    ** shown about each Term */
    यदि( sqlite3WhereTrace & 0x10000 ){
      sqlite3DebugPrintf(" prob=%-3d prereq=%llx,%llx",
        p_पदांशः->truthProb, (u64)p_पदांशः->prereqAll, (u64)p_पदांशः->prereq_दक्षिणः);
    }
    यदि( (p_पदांशः->eOperator & (WO_OR|WO_AND))==0 && p_पदांशः->u.x.i_क्षेत्रम् ){
      sqlite3DebugPrintf(" iField=%d", p_पदांशः->u.x.i_क्षेत्रम्);
    }
    यदि( p_पदांशः->iParent>=0 ){
      sqlite3DebugPrintf(" iParent=%d", p_पदांशः->iParent);
    }
    sqlite3DebugPrintf("\n");
    sqlite3_वृक्षः_View_व्यञ्जकम्(0, p_पदांशः->p_व्यञ्जकम्, 0);
  }
}
शून्य sqlite3_Show_Where_पदांशः(Where_पदांशः *p_पदांशः){
  sqlite3_Where_पदांशः_Print(p_पदांशः, 0);
}
#यदि_अन्तः

#यदि_परिभाषितम् WHERETRACE_ENABLED
/*
** Show the complete content of a WhereClause
*/
शून्य sqlite3WhereClausePrint(WhereClause *pWC){
  पूर्णाङ्क i;
  प्रति(i=0; i<pWC->n_पदांशः; i++){
    sqlite3_Where_पदांशः_Print(&pWC->a[i], i);
  }
}
#यदि_अन्तः

#यदि_परिभाषितम् WHERETRACE_ENABLED
/*
** Print a WhereLoop object for debugging purposes
**
** Format example:
**
**     .--- Position in WHERE clause           rSetup, rRun, nOut ---.
**     |                                                             |
**     |  .--- selfMask                       nTerm ------.          |
**     |  |                                               |          |
**     |  |   .-- prereq    Idx          wsFlags----.     |          |
**     |  |   |             Name                    |     |          |
**     |  |   |           __|__        nEq ---.  ___|__   |        __|__
**     | / \ / \         /     \              | /      \ / \      /     \
**     1.002.001         t2.t2xy              2 f 010241 N 2 cost 0,56,31
*/
शून्य sqlite3WhereLoopPrint(ध्रुव WhereLoop *p, ध्रुव WhereClause *pWC){
  WhereInfo *pWInfo;
  यदि( pWC ){
    पूर्णाङ्क nb;
    SrcItem *pItem;
    सारणी *pTab;
    Bitmask mAll;

    pWInfo = pWC->pWInfo;
    nb = 1+(pWInfo->pTabList->nSrc+3)/4;
    pItem = pWInfo->pTabList->a + p->iTab;
    pTab = pItem->pSTab;
    mAll = (((Bitmask)1)<<(nb*4)) - 1;
    sqlite3DebugPrintf("%c%2d.%0*llx.%0*llx", p->cId,
                       p->iTab, nb, p->आवरणम्_Self, nb, p->prereq & mAll);
    sqlite3DebugPrintf(" %12s",
                       pItem->zAlias ? pItem->zAlias : pTab->zName);
  }अन्यथा{
    pWInfo = 0;
    sqlite3DebugPrintf("%c%2d.%03llx.%03llx %c%d",
         p->cId, p->iTab, p->आवरणम्_Self, p->prereq & 0xfff, p->cId, p->iTab);
  }
  यदि( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    ध्रुव वर्ण *zName;
    यदि( p->u.btree.p_सूचकांक && (zName = p->u.btree.p_सूचकांक->zName)!=0 ){
      यदि( वाक्ये_तुलय(zName, "sqlite_autoindex_", 17)==0 ){
        पूर्णाङ्क i = sqlite3Strlen30(zName) - 1;
        यावत्( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }अन्यथा{
      sqlite3DebugPrintf("%20s","");
    }
  }अन्यथा{
    वर्ण *z;
    यदि( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%#x)",
                p->u.vtab.idx_अङ्कः, p->u.vtab.idxStr, p->u.vtab.omit_आवरणम्);
    }अन्यथा{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idx_अङ्कः, p->u.vtab.omit_आवरणम्);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  यदि( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %06x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }अन्यथा{
    sqlite3DebugPrintf(" f %06x N %d", p->wsFlags, p->nLTerm);
  }
  यदि( pWInfo && pWInfo->b_तारकम्_प्रयुक्तम् && p->r_तारकम्_Delta!=0 ){
    sqlite3DebugPrintf(" cost %d,%d,%d delta=%d\n",
                       p->r_सज्जता, p->r_सततम्, p->n_बहिर्गमनम्, p->r_तारकम्_Delta);
  }अन्यथा{
    sqlite3DebugPrintf(" cost %d,%d,%d\n", p->r_सज्जता, p->r_सततम्, p->n_बहिर्गमनम्);
  }
  यदि( p->nLTerm && (sqlite3WhereTrace & 0x4000)!=0 ){
    पूर्णाङ्क i;
    प्रति(i=0; i<p->nLTerm; i++){
      sqlite3_Where_पदांशः_Print(p->aLTerm[i], i);
    }
  }
}
शून्य sqlite3ShowWhereLoop(ध्रुव WhereLoop *p){
  यदि( p ) sqlite3WhereLoopPrint(p, 0);
}
शून्य sqlite3ShowWhereLoopList(ध्रुव WhereLoop *p){
  यावत्( p ){
    sqlite3ShowWhereLoop(p);
    p = p->p_अग्र_Loop;
  }
}
#यदि_अन्तः

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
स्थिर शून्य whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = Array_आकार(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
स्थिर शून्य whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  यदि( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_सूचकांक) ){
    यदि( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }अन्यथा यदि( (p->wsFlags & WHERE_AUTO_सूचकांक)!=0 && p->u.btree.p_सूचकांक!=0 ){
      sqlite3DbFree(db, p->u.btree.p_सूचकांक->zColAff);
      sqlite3DbFreeNN(db, p->u.btree.p_सूचकांक);
      p->u.btree.p_सूचकांक = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object.  Leave the
** object in an initialized state, as if it had been newly allocated.
*/
स्थिर शून्य whereLoopClear(sqlite3 *db, WhereLoop *p){
  यदि( p->aLTerm!=p->aLTermSpace ){
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = Array_आकार(p->aLTermSpace);
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
स्थिर पूर्णाङ्क whereLoopResize(sqlite3 *db, WhereLoop *p, पूर्णाङ्क n){
  Where_पदांशः **paNew;
  यदि( p->nLSlot>=n ) प्रत्यागम SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRawNN(db, परिमाण(p->aLTerm[0])*n);
  यदि( paNew==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
  स्मृतिं_प्रतिलिख(paNew, p->aLTerm, परिमाण(p->aLTerm[0])*p->nLSlot);
  यदि( p->aLTerm!=p->aLTermSpace ) sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  प्रत्यागम SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
स्थिर पूर्णाङ्क whereLoopXfer(sqlite3 *db, WhereLoop *p_गन्तव्यम्, WhereLoop *pFrom){
  whereLoopClearUnion(db, p_गन्तव्यम्);
  यदि( pFrom->nLTerm > p_गन्तव्यम्->nLSlot
   && whereLoopResize(db, p_गन्तव्यम्, pFrom->nLTerm)
  ){
    स्मृतिं_पूरय(p_गन्तव्यम्, 0, WHERE_LOOP_XFER_SZ);
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  स्मृतिं_प्रतिलिख(p_गन्तव्यम्, pFrom, WHERE_LOOP_XFER_SZ);
  स्मृतिं_प्रतिलिख(p_गन्तव्यम्->aLTerm, pFrom->aLTerm, p_गन्तव्यम्->nLTerm*परिमाण(p_गन्तव्यम्->aLTerm[0]));
  यदि( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }अन्यथा यदि( (pFrom->wsFlags & WHERE_AUTO_सूचकांक)!=0 ){
    pFrom->u.btree.p_सूचकांक = 0;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
स्थिर शून्य whereLoopDelete(sqlite3 *db, WhereLoop *p){
  प्रतिज्ञा( db!=0 );
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

/*
** Free a WhereInfo structure
*/
स्थिर शून्य whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  प्रतिज्ञा( pWInfo!=0 );
  प्रतिज्ञा( db!=0 );
  sqlite3WhereClauseClear(&pWInfo->sWC);
  यावत्( pWInfo->pLoops ){
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->p_अग्र_Loop;
    whereLoopDelete(db, p);
  }
  यावत्( pWInfo->p_Mem_गन्तव्यम्_Free ){
    Where_Mem_खण्डम् *p_अग्र = pWInfo->p_Mem_गन्तव्यम्_Free->p_अग्र;
    sqlite3DbNNFreeNN(db, pWInfo->p_Mem_गन्तव्यम्_Free);
    pWInfo->p_Mem_गन्तव्यम्_Free = p_अग्र;
  }
  sqlite3DbNNFreeNN(db, pWInfo);
}

/*
** Return TRUE if X is a proper subset of Y but is of equal or less cost.
** In other words, return true if all constraints of X are also part of Y
** and Y has additional constraints that might speed the search that X lacks
** but the cost of running X is not more than the cost of running Y.
**
** In other words, return true if the cost relationship between X and Y
** is inverted and needs to be adjusted.
**
** Case 1:
**
**   (1a)  X and Y use the same index.
**   (1b)  X has fewer == terms than Y
**   (1c)  Neither X nor Y use skip-scan
**   (1d)  X does not have a a greater cost than Y
**
** Case 2:
**
**   (2a)  X has the same or lower cost, or returns the same or fewer rows,
**         than Y.
**   (2b)  X uses fewer WHERE clause terms than Y
**   (2c)  Every WHERE clause term used by X is also used by Y
**   (2d)  X skips at least as many columns as Y
**   (2e)  If X is a covering index, than Y is too
*/
स्थिर पूर्णाङ्क whereLoopCheaperProperSubset(
  ध्रुव WhereLoop *pX,       /* First WhereLoop to compare */
  ध्रुव WhereLoop *pY        /* Compare against this WhereLoop */
){
  पूर्णाङ्क i, j;
  यदि( pX->r_सततम्>pY->r_सततम् && pX->n_बहिर्गमनम्>pY->n_बहिर्गमनम् ) प्रत्यागम 0; /* (1d) and (2a) */
  प्रतिज्ञा( (pX->wsFlags & WHERE_VIRTUALTABLE)==0 );
  प्रतिज्ञा( (pY->wsFlags & WHERE_VIRTUALTABLE)==0 );
  यदि( pX->u.btree.nEq < pY->u.btree.nEq                  /* (1b) */
   && pX->u.btree.p_सूचकांक==pY->u.btree.p_सूचकांक             /* (1a) */
   && pX->nSkip==0 && pY->nSkip==0                       /* (1c) */
  ){
    प्रत्यागम 1;  /* Case 1 is true */
  }
  यदि( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    प्रत्यागम 0;                                            /* (2b) */
  }
  यदि( pY->nSkip > pX->nSkip ) प्रत्यागम 0;                  /* (2d) */
  प्रति(i=pX->nLTerm-1; i>=0; i--){
    यदि( pX->aLTerm[i]==0 ) अनुवर्त;
    प्रति(j=pY->nLTerm-1; j>=0; j--){
      यदि( pY->aLTerm[j]==pX->aLTerm[i] ) भङ्ग;
    }
    यदि( j<0 ) प्रत्यागम 0;                                  /* (2c) */
  }
  यदि( (pX->wsFlags&WHERE_IDX_ONLY)!=0
   && (pY->wsFlags&WHERE_IDX_ONLY)==0 ){
    प्रत्यागम 0;                                            /* (2e) */
  }
  प्रत्यागम 1;  /* Case 2 is true */
}

/*
** Try to adjust the cost and number of output rows of WhereLoop pTemplate
** upwards or downwards so that:
**
**   (1) pTemplate costs less than any other WhereLoops that are a proper
**       subset of pTemplate
**
**   (2) pTemplate costs more than any other WhereLoops for which pTemplate
**       is a proper subset.
**
** To say "WhereLoop X is a proper subset of Y" means that X uses fewer
** WHERE clause terms than Y and that every WHERE clause term used by X is
** also used by Y.
*/
स्थिर शून्य whereLoopAdjustCost(ध्रुव WhereLoop *p, WhereLoop *pTemplate){
  यदि( (pTemplate->wsFlags & WHERE_अनुक्रमितम्)==0 ) प्रत्यागम;
  प्रति(; p; p=p->p_अग्र_Loop){
    यदि( p->iTab!=pTemplate->iTab ) अनुवर्त;
    यदि( (p->wsFlags & WHERE_अनुक्रमितम्)==0 ) अनुवर्त;
    यदि( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_सततम्, pTemplate->n_बहिर्गमनम्,
                       MIN(p->r_सततम्, pTemplate->r_सततम्),
                       MIN(p->n_बहिर्गमनम् - 1, pTemplate->n_बहिर्गमनम्)));
      pTemplate->r_सततम् = MIN(p->r_सततम्, pTemplate->r_सततम्);
      pTemplate->n_बहिर्गमनम् = MIN(p->n_बहिर्गमनम् - 1, pTemplate->n_बहिर्गमनम्);
    }अन्यथा यदि( whereLoopCheaperProperSubset(pTemplate, p) ){
      /* Adjust pTemplate cost upward so that it is costlier than p since
      ** pTemplate is a proper subset of p */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->r_सततम्, pTemplate->n_बहिर्गमनम्,
                       MAX(p->r_सततम्, pTemplate->r_सततम्),
                       MAX(p->n_बहिर्गमनम् + 1, pTemplate->n_बहिर्गमनम्)));
      pTemplate->r_सततम् = MAX(p->r_सततम्, pTemplate->r_सततम्);
      pTemplate->n_बहिर्गमनम् = MAX(p->n_बहिर्गमनम् + 1, pTemplate->n_बहिर्गमनम्);
    }
  }
}

/*
** Search the list of WhereLoops in *ppPrev looking for one that can be
** replaced by pTemplate.
**
** Return NULL if pTemplate does not belong on the WhereLoop list.
** In other words if pTemplate ought to be dropped from further consideration.
**
** If pX is a WhereLoop that pTemplate can replace, then return the
** link that points to pX.
**
** If pTemplate cannot replace any existing element of the list but needs
** to be added to the list as a new entry, then return a pointer to the
** tail of the list.
*/
स्थिर WhereLoop **whereLoopFindLesser(
  WhereLoop **pp_पूर्व,
  ध्रुव WhereLoop *pTemplate
){
  WhereLoop *p;
  प्रति(p=(*pp_पूर्व); p; pp_पूर्व=&p->p_अग्र_Loop, p=*pp_पूर्व){
    यदि( p->iTab!=pTemplate->iTab || p->i_क्रमय_Idx!=pTemplate->i_क्रमय_Idx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      अनुवर्त;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    प्रतिज्ञा( p->r_सज्जता==0 || pTemplate->r_सज्जता==0
                 || p->r_सज्जता==pTemplate->r_सज्जता );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    प्रतिज्ञा( p->r_सज्जता>=pTemplate->r_सज्जता );

    /* Any loop using an application-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    यदि( (p->wsFlags & WHERE_AUTO_सूचकांक)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_अनुक्रमितम्)!=0
     && (pTemplate->wsFlags & WHERE_COLUMN_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      भङ्ग;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    यदि( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->r_सज्जता<=pTemplate->r_सज्जता                  /* (2a) */
     && p->r_सततम्<=pTemplate->r_सततम्                      /* (2b) */
     && p->n_बहिर्गमनम्<=pTemplate->n_बहिर्गमनम्                      /* (2c) */
    ){
      प्रत्यागम 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependencies than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    यदि( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->r_सततम्>=pTemplate->r_सततम्                             /* (2a) */
     && p->n_बहिर्गमनम्>=pTemplate->n_बहिर्गमनम्                             /* (2b) */
    ){
      प्रतिज्ञा( p->r_सज्जता>=pTemplate->r_सज्जता ); /* SETUP-INVARIANT above */
      भङ्ग;   /* Cause p to be overwritten by pTemplate */
    }
  }
  प्रत्यागम pp_पूर्व;
}

/*
** Insert or replace a WhereLoop entry using the template supplied.
**
** An existing WhereLoop entry might be overwritten if the new template
** is better and has fewer dependencies.  Or the template will be ignored
** and no insert will occur if an existing WhereLoop is faster and has
** fewer dependencies than the template.  Otherwise a new WhereLoop is
** added based on the template.
**
** If pBuilder->pOrSet is not NULL then we care about only the
** prerequisites and rRun and nOut costs of the N best loops.  That
** information is gathered in the pBuilder->pOrSet object.  This special
** processing mode is used only for OR clause processing.
**
** When accumulating multiple loops (when pBuilder->pOrSet is NULL) we
** still might overwrite similar loops with the new template if the
** new template is better.  Loops may be overwritten if the following
** conditions are met:
**
**    (1)  They have the same iTab.
**    (2)  They have the same iSortIdx.
**    (3)  The template has same or fewer dependencies than the current loop
**    (4)  The template has the same or lower cost than the current loop
*/
स्थिर पूर्णाङ्क where_Loop_निवेशय(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **pp_पूर्व, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  पूर्णाङ्क rc;

  /* Stop the search once we hit the query planner search limit */
  यदि( pBuilder->i_Plan_सीमा==0 ){
    WHERETRACE(0xffffffff,("=== query planner search limit reached ===\n"));
    यदि( pBuilder->p_Or_समुच्चयः ) pBuilder->p_Or_समुच्चयः->n = 0;
    प्रत्यागम SQLITE_समाप्तम्;
  }
  pBuilder->i_Plan_सीमा--;

  whereLoopAdjustCost(pWInfo->pLoops, pTemplate);

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  यदि( pBuilder->p_Or_समुच्चयः!=0 ){
    यदि( pTemplate->nLTerm ){
#यदि WHERETRACE_ENABLED
      u16 n = pBuilder->p_Or_समुच्चयः->n;
      पूर्णाङ्क x =
#यदि_अन्तः
      where_Or_निवेशय(pBuilder->p_Or_समुच्चयः, pTemplate->prereq, pTemplate->r_सततम्,
                                    pTemplate->n_बहिर्गमनम्);
#यदि WHERETRACE_ENABLED /* 0x8 */
      यदि( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
        sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
      }
#यदि_अन्तः
    }
    प्रत्यागम SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  pp_पूर्व = whereLoopFindLesser(&pWInfo->pLoops, pTemplate);

  यदि( pp_पूर्व==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#यदि WHERETRACE_ENABLED /* 0x8 */
    यदि( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
    }
#यदि_अन्तः
    प्रत्यागम SQLITE_OK; 
  }अन्यथा{
    p = *pp_पूर्व;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#यदि WHERETRACE_ENABLED /* 0x8 */
  यदि( sqlite3WhereTrace & 0x8 ){
    यदि( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      sqlite3WhereLoopPrint(p, pBuilder->pWC);
      sqlite3DebugPrintf("   with: ");
    }अन्यथा{
      sqlite3DebugPrintf("    add: ");
    }
    sqlite3WhereLoopPrint(pTemplate, pBuilder->pWC);
  }
#यदि_अन्तः
  यदि( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *pp_पूर्व = p = sqlite3DbMallocRawNN(db, परिमाण(WhereLoop));
    यदि( p==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
    whereLoopInit(p);
    p->p_अग्र_Loop = 0;
  }अन्यथा{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplanted by pTemplate */
    WhereLoop **pp_पुच्छ = &p->p_अग्र_Loop;
    WhereLoop *p_गन्तव्यम्_Del;
    यावत्( *pp_पुच्छ ){
      pp_पुच्छ = whereLoopFindLesser(pp_पुच्छ, pTemplate);
      यदि( pp_पुच्छ==0 ) भङ्ग;
      p_गन्तव्यम्_Del = *pp_पुच्छ;
      यदि( p_गन्तव्यम्_Del==0 ) भङ्ग;
      *pp_पुच्छ = p_गन्तव्यम्_Del->p_अग्र_Loop;
#यदि WHERETRACE_ENABLED /* 0x8 */
      यदि( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        sqlite3WhereLoopPrint(p_गन्तव्यम्_Del, pBuilder->pWC);
      }
#यदि_अन्तः
      whereLoopDelete(db, p_गन्तव्यम्_Del);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  यदि( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    सूचकांक *p_सूचकांक = p->u.btree.p_सूचकांक;
    यदि( p_सूचकांक && p_सूचकांक->idxType==SQLITE_IDXTYPE_IPK ){
      p->u.btree.p_सूचकांक = 0;
    }
  }
  प्रत्यागम rc;
}

/*
** Callback for estLikePatternLength().
**
** If this node is a string literal that is longer pWalker->sz, then set
** pWalker->sz to the byte length of that string literal.
**
** pWalker->eCode indicates how to count characters:
**
**    eCode==0     Count as a GLOB pattern
**    eCode==1     Count as a LIKE pattern
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_ग्रन्थिः_आदर्शः_दीर्घता_Est(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  यदि( p_व्यञ्जकम्->op==TK_STRING ){
    पूर्णाङ्क sz = 0;                    /* Pattern size in bytes */
    u8 *z = (u8*)p_व्यञ्जकम्->u.zToken;  /* The pattern */
    u8 c;                          /* Next character of the pattern */
    u8 c1, c2, c3;                 /* Wildcards */
    यदि( pWalker->e_संहिता ){
      c1 = '%';
      c2 = '_';
      c3 = 0;
    }अन्यथा{
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    यावत्( (c = *(z++))!=0 ){
      यदि( c==c3 ){
        यदि( *z ) z++;
        यावत्( *z && *z!=']' ) z++;
      }अन्यथा यदि( c!=c1 && c!=c2 ){
        sz++;
      }
    }
    यदि( sz>pWalker->u.sz ) pWalker->u.sz = sz;
  }
  प्रत्यागम WRC_Continue;
}

/*
** Return the length of the longest string literal in the given
** expression.
**
** eCode indicates how to count characters:
**
**    eCode==0     Count as a GLOB pattern
**    eCode==1     Count as a LIKE pattern
*/
स्थिर पूर्णाङ्क est_Like_आदर्शः_दीर्घता(व्यञ्जकम् *p, u16 e_संहिता){
  Walker w;
  w.u.sz = 0;
  w.e_संहिता = e_संहिता;
  w.x_व्यञ्जकम्_Callback = व्यञ्जकम्_ग्रन्थिः_आदर्शः_दीर्घता_Est;
  w.xSelectCallback = sqlite3SelectWalkFail;
#यदि_परिभाषितम् SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#यदि_अन्तः
  sqlite3_Walk_व्यञ्जकम्(&w, p);
  प्रत्यागम w.u.sz;
}

/*
** Adjust the WhereLoop.nOut value downward to account for terms of the
** WHERE clause that reference the loop but which are not used by an
** index.
*
** For every WHERE clause term that is not used by the index
** and which has a truth probability assigned by one of the likelihood(),
** likely(), or unlikely() SQL functions, reduce the estimated number
** of output rows by the probability specified.
**
** TUNING:  For every WHERE clause term that is not used by the index
** and which does not have an assigned truth probability, heuristics
** described below are used to try to estimate the truth probability.
** TODO --> Perhaps this is something that could be improved by better
** table statistics.
**
** Heuristic 1:  Estimate the truth probability as 93.75%.  The 93.75%
** value corresponds to -1 in LogEst notation, so this means decrement
** the WhereLoop.nOut field for every such WHERE clause term.
**
** Heuristic 2:  If there exists one or more WHERE clause terms of the
** form "x==EXPR" and EXPR is not a constant 0 or 1, then make sure the
** final output row estimate is no greater than 1/4 of the total number
** of rows in the table.  In other words, assume that x==EXPR will filter
** out at least 3 out of 4 rows.  If EXPR is -1 or 0 or 1, then maybe the
** "x" column is boolean or else -1 or 0 or 1 is a common default value
** on the "x" column and so in that case only cap the output row estimate
** at 1/2 instead of 1/4.
**
** Heuristic 3:  If there is a LIKE or GLOB (or REGEXP or MATCH) operator
** with a large constant pattern, then reduce the size of the search
** space according to the length of the pattern, under the theory that
** longer patterns are less likely to match.  This heuristic was added
** to give better output-row count estimates when preparing queries for
** the Join-Order Benchmarks.  See forum thread 2026-01-30T09:57:54z
*/
स्थिर शून्य whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst nRow            /* Number of rows in the entire table */
){
  Where_पदांशः *p_पदांशः, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->आवरणम्_Self);
  पूर्णाङ्क i, j;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  /* Skip all this if the FROM clause of the query is a single table and
  ** there is no ORDER BY. In this case it doesn't matter how accurate
  ** the WhereLoop.nOut values are.  */
  यदि( pWC->pWInfo->pTabList->nSrc<=1 && pWC->pWInfo->p_क्रमः_By==0 ){
    प्रत्यागम;
  }

  प्रतिज्ञा( (pLoop->wsFlags & WHERE_AUTO_सूचकांक)==0 );
  प्रति(i=pWC->n_आधारः, p_पदांशः=pWC->a; i>0; i--, p_पदांशः++){
    प्रतिज्ञा( p_पदांशः!=0 );
    यदि( (p_पदांशः->prereqAll & notAllowed)!=0 ) अनुवर्त;
    यदि( (p_पदांशः->prereqAll & pLoop->आवरणम्_Self)==0 ) अनुवर्त;
    यदि( (p_पदांशः->wtFlags & पदांशः_VIRTUAL)!=0 ) अनुवर्त;
    प्रति(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      यदि( pX==0 ) अनुवर्त;
      यदि( pX==p_पदांशः ) भङ्ग;
      यदि( pX->iParent>=0 && (&pWC->a[pX->iParent])==p_पदांशः ) भङ्ग;
    }
    यदि( j<0 ){
      sqlite3_प्रगतिः_Check(pWC->pWInfo->pParse);
      यदि( pLoop->आवरणम्_Self==p_पदांशः->prereqAll ){
        /* If there are extra terms in the WHERE clause not used by an index
        ** that depend only on the table being scanned, and that will tend to
        ** cause many rows to be omitted, then mark that table as
        ** "self-culling".
        **
        ** 2022-03-24:  Self-culling only applies if either the extra terms
        ** are straight comparison operators that are non-true with NULL
        ** operand, or if the loop is not an OUTER JOIN.
        */
        यदि( (p_पदांशः->eOperator & 0x3f)!=0
         || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype
                  & (JT_वामः|JT_LTORJ))==0
        ){
          pLoop->wsFlags |= WHERE_SELFCULL;
        }
      }
      यदि( p_पदांशः->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->n_बहिर्गमनम् += p_पदांशः->truthProb;
      }अन्यथा{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        व्यञ्जकम् *p_Op_व्यञ्जकम् = p_पदांशः->p_व्यञ्जकम्;
        pLoop->n_बहिर्गमनम्--;
        यदि( (p_पदांशः->eOperator&(WO_EQ|WO_IS))!=0
         && (p_पदांशः->wtFlags & पदांशः_HIGHTRUTH)==0  /* tag-20200224-1 */
        ){
          व्यञ्जकम् *p_दक्षिणः = p_Op_व्यञ्जकम्->p_दक्षिणः;
          Parse *pParse = pWC->pWInfo->pParse;
          पूर्णाङ्क k = 0;
          testcase( p_Op_व्यञ्जकम्->op==TK_IS );
          यदि( sqlite3_व्यञ्जकम्_Is_Integer(p_दक्षिणः, &k, pParse, 1) && k>=(-1) && k<=1 ){
            k = 10;
          }अन्यथा{
            k = 20;
          }
          यदि( iReduce<k ){
            p_पदांशः->wtFlags |= पदांशः_HEURTRUTH;
            iReduce = k;
          }
        }अन्यथा
        यदि( व्यञ्जकम्_Has_Property(p_Op_व्यञ्जकम्, EP_InfixFunc) 
         && p_Op_व्यञ्जकम्->op==TK_FUNCTION
        ){
          पूर्णाङ्क eOp;
          प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_Op_व्यञ्जकम्) );
          प्रतिज्ञा( p_Op_व्यञ्जकम्->x.pList->n_व्यञ्जकम्>=2 );
          eOp = sqlite3_व्यञ्जकम्_Is_Like_Operator(p_Op_व्यञ्जकम्);
          यदि( ALWAYS(eOp>0) ){
            पूर्णाङ्क sz_आदर्शः;
            व्यञ्जकम् *pRHS = p_Op_व्यञ्जकम्->x.pList->a[0].p_व्यञ्जकम्;
            eOp = eOp==SQLITE_सूचकांक_CONSTRAINT_LIKE;
            sz_आदर्शः = est_Like_आदर्शः_दीर्घता(pRHS, eOp);
            यदि( sz_आदर्शः>0 ){
              pLoop->n_बहिर्गमनम् -= sz_आदर्शः*2;
            }
          }
        }
      }
    }
  }
  यदि( pLoop->n_बहिर्गमनम् > nRow-iReduce ){
    pLoop->n_बहिर्गमनम् = nRow - iReduce;
  }
}

/*
** Term pTerm is a vector range comparison operation. The first comparison
** in the vector can be optimized using column nEq of the index. This
** function returns the total number of vector elements that can be used
** as part of the range comparison.
**
** For example, if the query is:
**
**   WHERE a = ? AND (b, c, d) > (?, ?, ?)
**
** and the index:
**
**   CREATE INDEX ... ON (a, b, c, d, e)
**
** then this function would be invoked with nEq=1. The value returned in
** this case is 3.
*/
स्थिर पूर्णाङ्क where_Range_सदिशम्_Len(
  Parse *pParse,       /* Parsing context */
  पूर्णाङ्क iCur,            /* Cursor open on pIdx */
  सूचकांक *pIdx,         /* The index to be used for a inequality constraint */
  पूर्णाङ्क nEq,             /* Number of prior equality constraints on same index */
  Where_पदांशः *p_पदांशः     /* The vector inequality constraint */
){
  पूर्णाङ्क nCmp = sqlite3_व्यञ्जकम्_सदिशम्_आकार(p_पदांशः->p_व्यञ्जकम्->p_वामः);
  पूर्णाङ्क i;

  nCmp = MIN(nCmp, (pIdx->nColumn - nEq));
  प्रति(i=1; i<nCmp; i++){
    /* Test if comparison i of pTerm is compatible with column (i+nEq)
    ** of the index. If not, exit the loop.  */
    वर्ण aff;                     /* Comparison affinity */
    वर्ण idxaff = 0;              /* Indexed columns affinity */
    CollSeq *pColl;               /* Comparison collation sequence */
    व्यञ्जकम् *pLhs, *pRhs;

    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_पदांशः->p_व्यञ्जकम्->p_वामः) );
    pLhs = p_पदांशः->p_व्यञ्जकम्->p_वामः->x.pList->a[i].p_व्यञ्जकम्;
    pRhs = p_पदांशः->p_व्यञ्जकम्->p_दक्षिणः;
    यदि( व्यञ्जकम्_Use_XSelect(pRhs) ){
      pRhs = pRhs->x.pSelect->pEList->a[i].p_व्यञ्जकम्;
    }अन्यथा{
      pRhs = pRhs->x.pList->a[i].p_व्यञ्जकम्;
    }

    /* Check that the LHS of the comparison is a column reference to
    ** the right column of the right source table. And that the sort
    ** order of the index column is the same as the sort order of the
    ** leftmost index column.  */
    यदि( pLhs->op!=TK_COLUMN
     || pLhs->i_सारणी!=iCur
     || pLhs->iColumn!=pIdx->aiColumn[i+nEq]
     || pIdx->a_क्रमय_क्रमः[i+nEq]!=pIdx->a_क्रमय_क्रमः[nEq]
    ){
      भङ्ग;
    }

    testcase( pLhs->iColumn==XN_ROWID );
    aff = sqlite3CompareAffinity(pRhs, sqlite3_व्यञ्जकम्_Affinity(pLhs));
    idxaff = sqlite3_सारणी_Column_Affinity(pIdx->p_सारणी, pLhs->iColumn);
    यदि( aff!=idxaff ) भङ्ग;

    यदि( व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_Commuted) ) SWAP(व्यञ्जकम्*, pRhs, pLhs);
    pColl = sqlite3_द्विमान_Compare_Coll_Seq(pParse, pLhs, pRhs);
    यदि( pColl==0 ) भङ्ग;
    यदि( sqlite3StrICmp(pColl->zName, pIdx->azColl[i+nEq]) ) भङ्ग;
  }
  प्रत्यागम i;
}

/*
** Adjust the cost C by the costMult factor T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#यदि_परिभाषितम् SQLITE_ENABLE_COSTMULT
# define ApplyCostMultiplier(C,T)  C += T
#अन्यथा
# define ApplyCostMultiplier(C,T)
#यदि_अन्तः

/*
** We have so far matched pBuilder->pNew->u.btree.nEq terms of the
** index pIndex. Try to match one more.
**
** When this function is called, pBuilder->pNew->nOut contains the
** number of rows expected to be visited by filtering using the nEq
** terms only. If it is modified, this value is restored before this
** function returns.
**
** If pProbe->idxType==SQLITE_IDXTYPE_IPK, that means pIndex is
** a fake index used for the INTEGER PRIMARY KEY.
*/
स्थिर पूर्णाङ्क where_Loop_सङ्कलय_Btree_सूचकांक(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  SrcItem *pSrc,                  /* FROM clause term being analyzed */
  सूचकांक *pProbe,                  /* An index on pSrc */
  LogEst n_अन्तर्_Mul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyze context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  Where_पदांशः *p_पदांशः;               /* A WhereTerm under consideration */
  पूर्णाङ्क op_आवरणम्;                     /* Valid operators for constraints */
  Where_अवलोकनम् अवलोकनम्;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nBtm;                 /* Original value of pNew->u.btree.nBtm */
  u16 saved_n_शिखरम्;                 /* Original value of pNew->u.btree.nTop */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_n_बहिर्गमनम्;              /* Original value of pNew->nOut */
  पूर्णाङ्क rc = SQLITE_OK;             /* Return code */
  LogEst r_आकार;                   /* Number of rows in the table */
  LogEst r_Log_आकार;                /* Logarithm of table size */
  Where_पदांशः *p_शिखरम् = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  प्रतिज्ञा( db->mallocFailed==0 || pParse->nErr>0 );
  यदि( pParse->nErr ){
    प्रत्यागम pParse->rc;
  }
  WHERETRACE(0x800, ("BEGIN %s.addBtreeIdx(%s), nEq=%d, nSkip=%d, rRun=%d\n",
                     pProbe->p_सारणी->zName,pProbe->zName,
                     pNew->u.btree.nEq, pNew->nSkip, pNew->r_सततम्));

  प्रतिज्ञा( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  प्रतिज्ञा( (pNew->wsFlags & WHERE_शिखरम्_सीमा)==0 );
  यदि( pNew->wsFlags & WHERE_BTM_सीमा ){
    op_आवरणम् = WO_LT|WO_LE;
  }अन्यथा{
    प्रतिज्ञा( pNew->u.btree.nBtm==0 );
    op_आवरणम् = WO_EQ|WO_अन्तर्|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  यदि( pProbe->bUnordered ){
    op_आवरणम् &= ~(WO_GT|WO_GE|WO_LT|WO_LE);
  }

  प्रतिज्ञा( pNew->u.btree.nEq<pProbe->nColumn );
  प्रतिज्ञा( pNew->u.btree.nEq<pProbe->n_कुञ्चिका_Col
       || pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY );

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_n_शिखरम् = pNew->u.btree.n_शिखरम्;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_n_बहिर्गमनम् = pNew->n_बहिर्गमनम्;
  p_पदांशः = where_अवलोकनम्_Init(&अवलोकनम्, pBuilder->pWC, pSrc->iCursor, saved_nEq,
                        op_आवरणम्, pProbe);
  pNew->r_सज्जता = 0;
  r_आकार = pProbe->aiRowLogEst[0];
  r_Log_आकार = estLog(r_आकार);
  प्रति(; rc==SQLITE_OK && p_पदांशः!=0; p_पदांशः = where_अवलोकनम्_अग्र(&अवलोकनम्)){
    u16 eOp = p_पदांशः->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst rCostIdx;
    LogEst n_बहिर्गमनम्_Unadjusted;        /* nOut before IN() and WHERE adjustments */
    पूर्णाङ्क n_अन्तर् = 0;
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
    पूर्णाङ्क nRecValid = pBuilder->nRecValid;
#यदि_अन्तः
    यदि( (eOp==WO_ISNULL || (p_पदांशः->wtFlags&पदांशः_VNULL)!=0)
     && सूचकांक_Column_Not_Null(pProbe, saved_nEq)
    ){
      अनुवर्त; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    यदि( p_पदांशः->prereq_दक्षिणः & pNew->आवरणम्_Self ) अनुवर्त;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    यदि( p_पदांशः->wtFlags & पदांशः_LIKEOPT && p_पदांशः->eOperator==WO_LT ) अनुवर्त;

    यदि( (pSrc->fg.jointype & (JT_वामः|JT_LTORJ|JT_दक्षिणः))!=0
     && !constraintCompatibleWithOuterJoin(p_पदांशः,pSrc)
    ){
      अनुवर्त;
    }
    यदि( Is_Unique_सूचकांक(pProbe) && saved_nEq==pProbe->n_कुञ्चिका_Col-1 ){
      pBuilder->bldFlags1 |= SQLITE_BLDF1_UNIQUE;
    }अन्यथा{
      pBuilder->bldFlags1 |= SQLITE_BLDF1_अनुक्रमितम्;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.n_शिखरम् = saved_n_शिखरम्;
    pNew->nLTerm = saved_nLTerm;
    यदि( pNew->nLTerm>=pNew->nLSlot
     && whereLoopResize(db, pNew, pNew->nLTerm+1)
    ){
       भङ्ग; /* OOM while trying to enlarge the pNew->aLTerm array */
    }
    pNew->aLTerm[pNew->nLTerm++] = p_पदांशः;
    pNew->prereq = (saved_prereq | p_पदांशः->prereq_दक्षिणः) & ~pNew->आवरणम्_Self;

    प्रतिज्ञा( n_अन्तर्_Mul==0
        || (pNew->wsFlags & WHERE_COLUMN_NULL)!=0
        || (pNew->wsFlags & WHERE_COLUMN_अन्तर्)!=0
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0
    );

    यदि( eOp & WO_अन्तर् ){
      व्यञ्जकम् *p_व्यञ्जकम् = p_पदांशः->p_व्यञ्जकम्;
      यदि( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        पूर्णाङ्क i;
        पूर्णाङ्क bRedundant = 0;
        n_अन्तर् = 46;  प्रतिज्ञा( 46==sqlite3LogEst(25) );

        /* The expression may actually be of the form (x, y) IN (SELECT...).
        ** In this case there is a separate term for each of (x) and (y).
        ** However, the nIn multiplier should only be applied once, not once
        ** for each such term. The following loop checks that pTerm is the
        ** first such term in use, and sets nIn back to 0 if it is not. */
        प्रति(i=0; i<pNew->nLTerm-1; i++){
          यदि( pNew->aLTerm[i] && pNew->aLTerm[i]->p_व्यञ्जकम्==p_व्यञ्जकम् ){
            n_अन्तर् = 0;
            यदि( pNew->aLTerm[i]->u.x.i_क्षेत्रम् == p_पदांशः->u.x.i_क्षेत्रम् ){
              /* Detect when two or more columns of an index match the same 
              ** column of a vector IN operater, and avoid adding the column
              ** to the WhereLoop more than once.  See tag-20250707-01
              ** in test/rowvalue.test */
              bRedundant = 1;
            }
          }
        }
        यदि( bRedundant ){
          pNew->nLTerm--;
          अनुवर्त;
        }
      }अन्यथा यदि( ALWAYS(p_व्यञ्जकम्->x.pList && p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्) ){
        /* "x IN (value, value, ...)" */
        n_अन्तर् = sqlite3LogEst(p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्);
      }
      यदि( pProbe->hasStat1 && r_Log_आकार>=10 ){
        LogEst M, logK, x;
        /* Let:
        **   N = the total number of rows in the table
        **   K = the number of entries on the RHS of the IN operator
        **   M = the number of rows in the table that match terms to the
        **       to the left in the same index.  If the IN operator is on
        **       the left-most index column, M==N.
        **
        ** Given the definitions above, it is better to omit the IN operator
        ** from the index lookup and instead do a scan of the M elements,
        ** testing each scanned row against the IN operator separately, if:
        **
        **        M*log(K) < K*log(N)
        **
        ** Our estimates for M, K, and N might be inaccurate, so we build in
        ** a safety margin of 2 (LogEst: 10) that favors using the IN operator
        ** with the index, as using an index has better worst-case behavior.
        ** If we do not have real sqlite_stat1 data, always prefer to use
        ** the index.  Do not bother with this optimization on very small
        ** tables (less than 2 rows) as it is pointless in that case.
        */
        M = pProbe->aiRowLogEst[saved_nEq];
        logK = estLog(n_अन्तर्);
        /* TUNING      v-----  10 to bias toward indexed IN */
        x = M + logK + 10 - (n_अन्तर् + r_Log_आकार);
        यदि( x>=0 ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d) "
             "prefers indexed lookup\n",
             saved_nEq, M, logK, n_अन्तर्, r_Log_आकार, x));
        }अन्यथा यदि( n_अन्तर्_Mul<2 && OptimizationEnabled(db, SQLITE_Seek_अवलोकनम्) ){
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers skip-scan\n",
             saved_nEq, M, logK, n_अन्तर्, r_Log_आकार, x, n_अन्तर्_Mul));
          pNew->wsFlags |= WHERE_अन्तर्_SEEKSCAN;
        }अन्यथा{
          WHERETRACE(0x40,
            ("IN operator (N=%d M=%d logK=%d nIn=%d rLogSize=%d x=%d"
             " nInMul=%d) prefers normal scan\n",
             saved_nEq, M, logK, n_अन्तर्, r_Log_आकार, x, n_अन्तर्_Mul));
          अनुवर्त;
        }
      }
      pNew->wsFlags |= WHERE_COLUMN_अन्तर्;
    }अन्यथा यदि( eOp & (WO_EQ|WO_IS) ){
      पूर्णाङ्क iCol = pProbe->aiColumn[saved_nEq];
      pNew->wsFlags |= WHERE_COLUMN_EQ;
      प्रतिज्ञा( saved_nEq==pNew->u.btree.nEq );
      यदि( iCol==XN_ROWID
       || (iCol>=0 && n_अन्तर्_Mul==0 && saved_nEq==pProbe->n_कुञ्चिका_Col-1)
      ){
        यदि( iCol==XN_ROWID || pProbe->uniqNotNull
         || (pProbe->n_कुञ्चिका_Col==1 && pProbe->onError && (eOp & WO_EQ))
        ){
          pNew->wsFlags |= WHERE_ONEROW;
        }अन्यथा{
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }
      }
      यदि( अवलोकनम्.iEquiv>1 ) pNew->wsFlags |= WHERE_TRANSCONS;
    }अन्यथा यदि( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_COLUMN_NULL;
    }अन्यथा{
      पूर्णाङ्क nVecLen = where_Range_सदिशम्_Len(
          pParse, pSrc->iCursor, pProbe, saved_nEq, p_पदांशः
      );
      यदि( eOp & (WO_GT|WO_GE) ){
        testcase( eOp & WO_GT );
        testcase( eOp & WO_GE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_BTM_सीमा;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = p_पदांशः;
        p_शिखरम् = 0;
        यदि( p_पदांशः->wtFlags & पदांशः_LIKEOPT ){
          /* Range constraints that come from the LIKE optimization are
          ** always used in pairs. */
          p_शिखरम् = &p_पदांशः[1];
          प्रतिज्ञा( (p_शिखरम्-(p_पदांशः->pWC->a))<p_पदांशः->pWC->n_पदांशः );
          प्रतिज्ञा( p_शिखरम्->wtFlags & पदांशः_LIKEOPT );
          प्रतिज्ञा( p_शिखरम्->eOperator==WO_LT );
          यदि( whereLoopResize(db, pNew, pNew->nLTerm+1) ) भङ्ग; /* OOM */
          pNew->aLTerm[pNew->nLTerm++] = p_शिखरम्;
          pNew->wsFlags |= WHERE_शिखरम्_सीमा;
          pNew->u.btree.n_शिखरम् = 1;
        }
      }अन्यथा{
        प्रतिज्ञा( eOp & (WO_LT|WO_LE) );
        testcase( eOp & WO_LT );
        testcase( eOp & WO_LE );
        pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_शिखरम्_सीमा;
        pNew->u.btree.n_शिखरम् = nVecLen;
        p_शिखरम् = p_पदांशः;
        pBtm = (pNew->wsFlags & WHERE_BTM_सीमा)!=0 ?
                       pNew->aLTerm[pNew->nLTerm-2] : 0;
      }
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    प्रतिज्ञा( pNew->n_बहिर्गमनम्==saved_n_बहिर्गमनम् );
    यदि( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      /* Adjust nOut using stat4 data. Or, if there is no stat4
      ** data, using some other estimate.  */
      where_Range_अवलोकनम्_Est(pParse, pBuilder, pBtm, p_शिखरम्, pNew);
    }अन्यथा{
      पूर्णाङ्क nEq = ++pNew->u.btree.nEq;
      प्रतिज्ञा( eOp & (WO_ISNULL|WO_EQ|WO_अन्तर्|WO_IS) );

      प्रतिज्ञा( pNew->n_बहिर्गमनम्==saved_n_बहिर्गमनम् );
      यदि( p_पदांशः->truthProb<=0 && pProbe->aiColumn[saved_nEq]>=0 ){
        प्रतिज्ञा( (eOp & WO_अन्तर्) || n_अन्तर्==0 );
        testcase( eOp & WO_अन्तर् );
        pNew->n_बहिर्गमनम् += p_पदांशः->truthProb;
        pNew->n_बहिर्गमनम् -= n_अन्तर्;
      }अन्यथा{
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
        tRowcnt n_बहिर्गमनम् = 0;
        यदि( n_अन्तर्_Mul==0
         && pProbe->nSample
         && ALWAYS(pNew->u.btree.nEq<=pProbe->nSampleCol)
         && ((eOp & WO_अन्तर्)==0 || व्यञ्जकम्_Use_XList(p_पदांशः->p_व्यञ्जकम्))
         && OptimizationEnabled(db, SQLITE_Stat4)
        ){
          व्यञ्जकम् *p_व्यञ्जकम् = p_पदांशः->p_व्यञ्जकम्;
          यदि( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = where_Equal_अवलोकनम्_Est(pParse, pBuilder, p_व्यञ्जकम्->p_दक्षिणः, &n_बहिर्गमनम्);
          }अन्यथा{
            rc = where_अन्तर्_अवलोकनम्_Est(pParse, pBuilder, p_व्यञ्जकम्->x.pList, &n_बहिर्गमनम्);
          }
          यदि( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          यदि( rc!=SQLITE_OK ) भङ्ग;          /* Jump out of the pTerm loop */
          यदि( n_बहिर्गमनम् ){
            pNew->n_बहिर्गमनम् = sqlite3LogEst(n_बहिर्गमनम्);
            यदि( nEq==1
             /* TUNING: Mark terms as "low selectivity" if they seem likely
             ** to be true for half or more of the rows in the table.
             ** See tag-202002240-1 */
             && pNew->n_बहिर्गमनम्+10 > pProbe->aiRowLogEst[0]
            ){
#यदि WHERETRACE_ENABLED /* 0x01 */
              यदि( sqlite3WhereTrace & 0x20 ){
                sqlite3DebugPrintf(
                   "STAT4 determines term has low selectivity:\n");
                sqlite3_Where_पदांशः_Print(p_पदांशः, 999);
              }
#यदि_अन्तः
              p_पदांशः->wtFlags |= पदांशः_HIGHTRUTH;
              यदि( p_पदांशः->wtFlags & पदांशः_HEURTRUTH ){
                /* If the term has previously been used with an assumption of
                ** higher selectivity, then set the flag to rerun the
                ** loop computations. */
                pBuilder->bldFlags2 |= SQLITE_BLDF2_2NDPASS;
              }
            }
            यदि( pNew->n_बहिर्गमनम्>saved_n_बहिर्गमनम् ) pNew->n_बहिर्गमनम् = saved_n_बहिर्गमनम्;
            pNew->n_बहिर्गमनम् -= n_अन्तर्;
          }
        }
        यदि( n_बहिर्गमनम्==0 )
#यदि_अन्तः
        {
          pNew->n_बहिर्गमनम् += (pProbe->aiRowLogEst[nEq] - pProbe->aiRowLogEst[nEq-1]);
          यदि( eOp & WO_ISNULL ){
            /* TUNING: If there is no likelihood() value, assume that a
            ** "col IS NULL" expression matches twice as many rows
            ** as (col=?). */
            pNew->n_बहिर्गमनम् += 10;
          }
        }
      }
    }

    /* Set rCostIdx to the estimated cost of visiting selected rows in the
    ** index.  The estimate is the sum of two values:
    **   1.  The cost of doing one search-by-key to find the first matching
    **       entry
    **   2.  Stepping forward in the index pNew->nOut times to find all
    **       additional matching entries.
    */
    प्रतिज्ञा( pSrc->pSTab->szTabRow>0 );
    यदि( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* The pProbe->szIdxRow is low for an IPK table since the interior
      ** pages are small.  Thus szIdxRow gives a good estimate of seek cost.
      ** But the leaf pages are full-size, so pProbe->szIdxRow would badly
      ** under-estimate the scanning cost. */
      rCostIdx = pNew->n_बहिर्गमनम् + 16;
    }अन्यथा{
      rCostIdx = pNew->n_बहिर्गमनम् + 1 + (15*pProbe->szIdxRow)/pSrc->pSTab->szTabRow;
    }
    rCostIdx = sqlite3_Log_Est_सङ्कलय(r_Log_आकार, rCostIdx);

    /* Estimate the cost of running the loop.  If all data is coming
    ** from the index, then this is just the cost of doing the index
    ** lookup and scan.  But if some data is coming out of the main table,
    ** we also have to add in the cost of doing pNew->nOut searches to
    ** locate the row in the main table that corresponds to the index entry.
    */
    pNew->r_सततम् = rCostIdx;
    यदि( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK|WHERE_EXPRIDX))==0 ){
      pNew->r_सततम् = sqlite3_Log_Est_सङ्कलय(pNew->r_सततम्, pNew->n_बहिर्गमनम् + 16);
    }
    ApplyCostMultiplier(pNew->r_सततम्, pProbe->p_सारणी->costMult);

    n_बहिर्गमनम्_Unadjusted = pNew->n_बहिर्गमनम्;
    pNew->r_सततम् += n_अन्तर्_Mul + n_अन्तर्;
    pNew->n_बहिर्गमनम् += n_अन्तर्_Mul + n_अन्तर्;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, r_आकार);
    यदि( pSrc->fg.fromExists ) pNew->n_बहिर्गमनम् = 0;
    rc = where_Loop_निवेशय(pBuilder, pNew);

    यदि( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      pNew->n_बहिर्गमनम् = saved_n_बहिर्गमनम्;
    }अन्यथा{
      pNew->n_बहिर्गमनम् = n_बहिर्गमनम्_Unadjusted;
    }

    यदि( (pNew->wsFlags & WHERE_शिखरम्_सीमा)==0
     && pNew->u.btree.nEq<pProbe->nColumn
     && (pNew->u.btree.nEq<pProbe->n_कुञ्चिका_Col ||
          pProbe->idxType!=SQLITE_IDXTYPE_PRIMARYKEY)
    ){
      यदि( pNew->u.btree.nEq>3 ){
        sqlite3_प्रगतिः_Check(pParse);
      }
      where_Loop_सङ्कलय_Btree_सूचकांक(pBuilder, pSrc, pProbe, n_अन्तर्_Mul+n_अन्तर्);
    }
    pNew->n_बहिर्गमनम् = saved_n_बहिर्गमनम्;
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
    pBuilder->nRecValid = nRecValid;
#यदि_अन्तः
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.n_शिखरम् = saved_n_शिखरम्;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->n_बहिर्गमनम् = saved_n_बहिर्गमनम्;
  pNew->nLTerm = saved_nLTerm;

  /* Consider using a skip-scan if there are no WHERE clause constraints
  ** available for the left-most terms of the index, and if the average
  ** number of repeats in the left-most terms is at least 18.
  **
  ** The magic number 18 is selected on the basis that scanning 17 rows
  ** is almost always quicker than an index seek (even though if the index
  ** contains fewer than 2^17 rows we assume otherwise in other parts of
  ** the code). And, even if it is not, it should not be too much slower.
  ** On the other hand, the extra seeks could end up being significantly
  ** more expensive.  */
  प्रतिज्ञा( 42==sqlite3LogEst(18) );
  यदि( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->n_कुञ्चिका_Col
   && saved_nEq==pNew->nLTerm
   && pProbe->no_Skip_अवलोकनम्==0
   && pProbe->hasStat1!=0
   && OptimizationEnabled(db, SQLITE_Skip_अवलोकनम्)
   && pProbe->aiRowLogEst[saved_nEq+1]>=42  /* TUNING: Minimum for skip-scan */
   && pSrc->fg.fromExists==0
   && (rc = whereLoopResize(db, pNew, pNew->nLTerm+1))==SQLITE_OK
  ){
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= WHERE_SKIPSCAN;
    nIter = pProbe->aiRowLogEst[saved_nEq] - pProbe->aiRowLogEst[saved_nEq+1];
    pNew->n_बहिर्गमनम् -= nIter;
    /* TUNING:  Because uncertainties in the estimates for skip-scan queries,
    ** add a 1.375 fudge factor to make skip-scan slightly less likely. */
    nIter += 5;
    where_Loop_सङ्कलय_Btree_सूचकांक(pBuilder, pSrc, pProbe, nIter + n_अन्तर्_Mul);
    pNew->n_बहिर्गमनम् = saved_n_बहिर्गमनम्;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  WHERETRACE(0x800, ("END %s.addBtreeIdx(%s), nEq=%d, rc=%d\n",
                      pProbe->p_सारणी->zName, pProbe->zName, saved_nEq, rc));
  प्रत्यागम rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
स्थिर पूर्णाङ्क सूचकांक_Might_Help_With_क्रमः_By(
  WhereLoopBuilder *pBuilder,
  सूचकांक *p_सूचकांक,
  पूर्णाङ्क iCursor
){
  व्यञ्जकम्_List *pOB;
  व्यञ्जकम्_List *a_Col_व्यञ्जकम्;
  पूर्णाङ्क ii, jj;

  यदि( p_सूचकांक->bUnordered ) प्रत्यागम 0;
  यदि( (pOB = pBuilder->pWInfo->p_क्रमः_By)==0 ) प्रत्यागम 0;
  प्रति(ii=0; ii<pOB->n_व्यञ्जकम्; ii++){
    व्यञ्जकम् *p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(pOB->a[ii].p_व्यञ्जकम्);
    यदि( NEVER(p_व्यञ्जकम्==0) ) अनुवर्त;
    यदि( (p_व्यञ्जकम्->op==TK_COLUMN || p_व्यञ्जकम्->op==TK_AGG_COLUMN) 
     && p_व्यञ्जकम्->i_सारणी==iCursor 
    ){
      यदि( p_व्यञ्जकम्->iColumn<0 ) प्रत्यागम 1;
      प्रति(jj=0; jj<p_सूचकांक->n_कुञ्चिका_Col; jj++){
        यदि( p_व्यञ्जकम्->iColumn==p_सूचकांक->aiColumn[jj] ) प्रत्यागम 1;
      }
    }अन्यथा यदि( (a_Col_व्यञ्जकम् = p_सूचकांक->a_Col_व्यञ्जकम्)!=0 ){
      प्रति(jj=0; jj<p_सूचकांक->n_कुञ्चिका_Col; jj++){
        यदि( p_सूचकांक->aiColumn[jj]!=XN_व्यञ्जकम् ) अनुवर्त;
        यदि( sqlite3_व्यञ्जकम्_Compare_Skip(p_व्यञ्जकम्,a_Col_व्यञ्जकम्->a[jj].p_व्यञ्जकम्,iCursor)==0 ){
          प्रत्यागम 1;
        }
      }
    }
  }
  प्रत्यागम 0;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
स्थिर पूर्णाङ्क where_Usable_Partial_सूचकांक(
  पूर्णाङ्क iTab,             /* The table for which we want an index */
  u8 jointype,          /* The JT_* flags on the join */
  WhereClause *pWC,     /* The WHERE clause of the query */
  व्यञ्जकम् *pWhere          /* The WHERE clause from the partial index */
){
  पूर्णाङ्क i;
  Where_पदांशः *p_पदांशः;
  Parse *pParse;

  यदि( jointype & JT_LTORJ ) प्रत्यागम 0;
  pParse = pWC->pWInfo->pParse;
  यावत्( pWhere->op==TK_AND ){
    यदि( !where_Usable_Partial_सूचकांक(iTab,jointype,pWC,pWhere->p_वामः) ) प्रत्यागम 0;
    pWhere = pWhere->p_दक्षिणः;
  }
  प्रति(i=0, p_पदांशः=pWC->a; i<pWC->n_पदांशः; i++, p_पदांशः++){
    व्यञ्जकम् *p_व्यञ्जकम्;
    p_व्यञ्जकम् = p_पदांशः->p_व्यञ्जकम्;
    यदि( (!व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) || p_व्यञ्जकम्->w.iJoin==iTab)
     && ((jointype & JT_OUTER)==0 || व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON))
     && sqlite3_व्यञ्जकम्_उपपादयति_व्यञ्जकम्(pParse, p_व्यञ्जकम्, pWhere, iTab)
     && !sqlite3_व्यञ्जकम्_उपपादयति_व्यञ्जकम्(pParse, p_व्यञ्जकम्, pWhere, -1)
     && (p_पदांशः->wtFlags & पदांशः_VNULL)==0
    ){
      प्रत्यागम 1;
    }
  }
  प्रत्यागम 0;
}

/*
** pIdx is an index containing expressions.  Check it see if any of the
** expressions in the index match the pExpr expression.
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Is_Covered_By_सूचकांक(
  ध्रुव व्यञ्जकम् *p_व्यञ्जकम्,
  ध्रुव सूचकांक *pIdx,
  पूर्णाङ्क iTabCur
){
  पूर्णाङ्क i;
  प्रति(i=0; i<pIdx->nColumn; i++){
    यदि( pIdx->aiColumn[i]==XN_व्यञ्जकम्
     && sqlite3_व्यञ्जकम्_Compare(0, p_व्यञ्जकम्, pIdx->a_Col_व्यञ्जकम्->a[i].p_व्यञ्जकम्, iTabCur)==0
    ){
      प्रत्यागम 1;
    }
  }
  प्रत्यागम 0;
}

/*
** Structure passed to the whereIsCoveringIndex Walker callback.
*/
प्रकारनिर्देश संरचना Covering_सूचकांक_Check Covering_सूचकांक_Check;
संरचना Covering_सूचकांक_Check {
  सूचकांक *pIdx;       /* The index */
  पूर्णाङ्क iTabCur;       /* Cursor number for the corresponding table */
  u8 b_व्यञ्जकम्;          /* Uses an indexed expression */
  u8 bUnidx;         /* Uses an unindexed column not within an indexed expr */
};

/*
** Information passed in is pWalk->u.pCovIdxCk.  Call it pCk.
**
** If the Expr node references the table with cursor pCk->iTabCur, then
** make sure that column is covered by the index pCk->pIdx.  We know that
** all columns less than 63 (really BMS-1) are covered, so we don't need
** to check them.  But we do need to check any column at 63 or greater.
**
** If the index does not cover the column, then set pWalk->eCode to
** non-zero and return WRC_Abort to stop the search.
**
** If this node does not disprove that the index can be a covering index,
** then just return WRC_Continue, to continue the search.
**
** If pCk->pIdx contains indexed expressions and one of those expressions
** matches pExpr, then prune the search.
*/
स्थिर पूर्णाङ्क where_Is_Covering_सूचकांक_Walk_Callback(Walker *pWalk, व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क i;                    /* Loop counter */
  ध्रुव सूचकांक *pIdx;        /* The index of interest */
  ध्रुव i16 *aiColumn;      /* Columns contained in the index */
  u16 nColumn;              /* Number of columns in the index */
  Covering_सूचकांक_Check *pCk;  /* Info about this search */

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  यदि( (p_व्यञ्जकम्->op==TK_COLUMN || p_व्यञ्जकम्->op==TK_AGG_COLUMN) ){
    /* if( pExpr->iColumn<(BMS-1) && pIdx->bHasExpr==0 ) return WRC_Continue;*/
    यदि( p_व्यञ्जकम्->i_सारणी!=pCk->iTabCur ) प्रत्यागम WRC_Continue;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    aiColumn = pIdx->aiColumn;
    nColumn = pIdx->nColumn;
    प्रति(i=0; i<nColumn; i++){
      यदि( aiColumn[i]==p_व्यञ्जकम्->iColumn ) प्रत्यागम WRC_Continue;
    }
    pCk->bUnidx = 1;
    प्रत्यागम WRC_Abort;
  }अन्यथा यदि( pIdx->b_Has_व्यञ्जकम्
         && व्यञ्जकम्_Is_Covered_By_सूचकांक(p_व्यञ्जकम्, pIdx, pWalk->u.pCovIdxCk->iTabCur) ){
    pCk->b_व्यञ्जकम् = 1;
    प्रत्यागम WRC_Prune;
  }
  प्रत्यागम WRC_Continue;
}


/*
** pIdx is an index that covers all of the low-number columns used by
** pWInfo->pSelect (columns from 0 through 62) or an index that has
** expressions terms.  Hence, we cannot determine whether or not it is
** a covering index by using the colUsed bitmasks.  We have to do a search
** to see if the index is covering.  This routine does that search.
**
** The return value is one of these:
**
**      0                The index is definitely not a covering index
**
**      WHERE_IDX_ONLY   The index is definitely a covering index
**
**      WHERE_EXPRIDX    The index is likely a covering index, but it is
**                       difficult to determine precisely because of the
**                       expressions that are indexed.  Score it as a
**                       covering index, but still keep the main table open
**                       just in case we need it.
**
** This routine is an optimization.  It is always safe to return zero.
** But returning one of the other two values when zero should have been
** returned can lead to incorrect bytecode and assertion faults.
*/
स्थिर SQLITE_NOINLINE u32 where_Is_Covering_सूचकांक(
  WhereInfo *pWInfo,     /* The WHERE clause context */
  सूचकांक *pIdx,           /* Index that is being tested */
  पूर्णाङ्क iTabCur            /* Cursor for the table being indexed */
){
  पूर्णाङ्क i, rc;
  संरचना Covering_सूचकांक_Check ck;
  Walker w;
  यदि( pWInfo->pSelect==0 ){
    /* We don't have access to the full query, so we cannot check to see
    ** if pIdx is covering.  Assume it is not. */
    प्रत्यागम 0;
  }
  यदि( pIdx->b_Has_व्यञ्जकम्==0 ){
    प्रति(i=0; i<pIdx->nColumn; i++){
      यदि( pIdx->aiColumn[i]>=BMS-1 ) भङ्ग;
    }
    यदि( i>=pIdx->nColumn ){
      /* pIdx does not index any columns greater than 62, but we know from
      ** colMask that columns greater than 62 are used, so this is not a
      ** covering index */
      प्रत्यागम 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.b_व्यञ्जकम् = 0;
  ck.bUnidx = 0;
  स्मृतिं_पूरय(&w, 0, परिमाण(w));
  w.x_व्यञ्जकम्_Callback = where_Is_Covering_सूचकांक_Walk_Callback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  यदि( ck.bUnidx ){
    rc = 0;
  }अन्यथा यदि( ck.b_व्यञ्जकम् ){
    rc = WHERE_EXPRIDX;
  }अन्यथा{
    rc = WHERE_IDX_ONLY;
  }
  प्रत्यागम rc;
}

/*
** This is an sqlite3ParserAddCleanup() callback that is invoked to
** free the Parse->pIdxEpr list when the Parse object is destroyed.
*/
स्थिर शून्य where_अनुक्रमितम्_व्यञ्जकम्_Cleanup(sqlite3 *db, शून्य *pObject){
  अनुक्रमितम्_व्यञ्जकम् **pp = (अनुक्रमितम्_व्यञ्जकम्**)pObject;
  यावत्( *pp!=0 ){
    अनुक्रमितम्_व्यञ्जकम् *p = *pp;
    *pp = p->pIENext;
    sqlite3_व्यञ्जकम्_Delete(db, p->p_व्यञ्जकम्);
    sqlite3DbFreeNN(db, p);
  }
}

/*
** This function is called for a partial index - one with a WHERE clause - in 
** two scenarios. In both cases, it determines whether or not the WHERE 
** clause on the index implies that a column of the table may be safely
** replaced by a constant expression. For example, in the following 
** SELECT:
**
**   CREATE INDEX i1 ON t1(b, c) WHERE a=<expr>;
**   SELECT a, b, c FROM t1 WHERE a=<expr> AND b=?;
**
** The "a" in the select-list may be replaced by <expr>, iff:
**
**    (a) <expr> is a constant expression, and
**    (b) The (a=<expr>) comparison uses the BINARY collation sequence, and
**    (c) Column "a" has an affinity other than NONE or BLOB.
**
** If argument pItem is NULL, then pMask must not be NULL. In this case this 
** function is being called as part of determining whether or not pIdx
** is a covering index. This function clears any bits in (*pMask) 
** corresponding to columns that may be replaced by constants as described
** above.
**
** Otherwise, if pItem is not NULL, then this function is being called
** as part of coding a loop that uses index pIdx. In this case, add entries
** to the Parse.pIdxPartExpr list for each column that can be replaced
** by a constant.
*/
स्थिर शून्य where_Part_Idx_व्यञ्जकम्(
  Parse *pParse,                  /* Parse context */
  सूचकांक *pIdx,                    /* Partial index being processed */
  व्यञ्जकम् *pPart,                    /* WHERE clause being processed */
  Bitmask *p_आवरणम्,                 /* Mask to clear bits in */
  पूर्णाङ्क iIdxCur,                    /* Cursor number for index */
  SrcItem *pItem                  /* The FROM clause entry for the table */
){
  प्रतिज्ञा( pItem==0 || (pItem->fg.jointype & JT_दक्षिणः)==0 );
  प्रतिज्ञा( (pItem==0 || p_आवरणम्==0) && (p_आवरणम्!=0 || pItem!=0) );

  यदि( pPart->op==TK_AND ){
    where_Part_Idx_व्यञ्जकम्(pParse, pIdx, pPart->p_दक्षिणः, p_आवरणम्, iIdxCur, pItem);
    pPart = pPart->p_वामः;
  }

  यदि( (pPart->op==TK_EQ || pPart->op==TK_IS) ){
    व्यञ्जकम् *p_वामः = pPart->p_वामः;
    व्यञ्जकम् *p_दक्षिणः = pPart->p_दक्षिणः;
    सारणी *p_वामः_Tab;            /* Table of the left operand */
    Column *p_वामः_Col;           /* Column of the left operand */
    u8 aff_दक्षिणः, aff_वामः;

    यदि( p_वामः->op!=TK_COLUMN ) प्रत्यागम;
    यदि( !sqlite3_व्यञ्जकम्_Is_Constant(0, p_दक्षिणः) ) प्रत्यागम;
    यदि( !sqlite3_Is_द्विमान(sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pParse, pPart)) ) प्रत्यागम;
    यदि( p_वामः->iColumn<0 ) प्रत्यागम;
    p_वामः_Tab = pIdx->p_सारणी;
    p_वामः_Col = &p_वामः_Tab->aCol[p_वामः->iColumn];
    aff_वामः = p_वामः_Col->affinity;
    प्रतिज्ञा( SQLITE_AFF_BLOB<SQLITE_AFF_TEXT );
    यदि( aff_वामः<SQLITE_AFF_TEXT ) प्रत्यागम;
    aff_दक्षिणः = sqlite3_व्यञ्जकम्_Affinity(p_दक्षिणः);
    यदि( aff_दक्षिणः!=0 && aff_दक्षिणः!=aff_वामः ) प्रत्यागम;
    यदि( pItem ){
      sqlite3 *db = pParse->db;
      अनुक्रमितम्_व्यञ्जकम् *p = (अनुक्रमितम्_व्यञ्जकम्*)sqlite3DbMallocRaw(db, परिमाण(*p));
      यदि( p ){
        पूर्णाङ्क bNullRow = (pItem->fg.jointype&(JT_वामः|JT_LTORJ))!=0;
        p_दक्षिणः = sqlite3_व्यञ्जकम्_Dup(db, p_दक्षिणः, 0);
        यदि( (p_वामः_Tab->tabFlags & TF_Strict)!=0 ){
          Token t;
          t.z = sqlite3ColumnType(p_वामः_Col, 0);
          प्रतिज्ञा( t.z!=0 );  /* Because affLeft>=SQLITE_AFF_TEXT */
          t.n = (पूर्णाङ्क)वाक्यदैर्घ्यम्(t.z);
          p->p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Alloc(db, TK_CAST, &t, 1);
          sqlite3_व्यञ्जकम्_Attach_Subtrees(db, p->p_व्यञ्जकम्, p_दक्षिणः, 0);
        }अन्यथा{
          p->p_व्यञ्जकम् = p_दक्षिणः;
        }
        p->i_दत्त_Cur = pItem->iCursor;
        p->iIdxCur = iIdxCur;
        p->iIdxCol = p_वामः->iColumn;
        p->bMaybeNullRow = bNullRow;
        p->pIENext = pParse->p_Idx_Part_व्यञ्जकम्;
        p->aff = aff_वामः;
        pParse->p_Idx_Part_व्यञ्जकम् = p;
        यदि( p->pIENext==0 ){
          शून्य *pArg = (शून्य*)&pParse->p_Idx_Part_व्यञ्जकम्;
          sqlite3_Parser_सङ्कलय_Cleanup(pParse, where_अनुक्रमितम्_व्यञ्जकम्_Cleanup, pArg);
        }
      }
    }अन्यथा यदि( p_वामः->iColumn<(BMS-1) ){
      *p_आवरणम् &= ~((Bitmask)1 << p_वामः->iColumn);
    }
  }
}


/*
** Add all WhereLoop objects for a single table of the join where the table
** is identified by pBuilder->pNew->iTab.  That table is guaranteed to be
** a b-tree table, not a virtual table.
**
** The costs (WhereLoop.rRun) of the b-tree loops added by this function
** are calculated as follows:
**
** For a full scan, assuming the table (or index) contains nRow rows:
**
**     cost = nRow * 3.0                    // full-table scan
**     cost = nRow * K                      // scan of covering index
**     cost = nRow * (K+3.0)                // scan of non-covering index
**
** where K is a value between 1.1 and 3.0 set based on the relative
** estimated average size of the index and table records.
**
** For an index scan, where nVisit is the number of index rows visited
** by the scan, and nSeek is the number of seek operations required on
** the index b-tree:
**
**     cost = nSeek * (log(nRow) + K * nVisit)          // covering index
**     cost = nSeek * (log(nRow) + (K+3.0) * nVisit)    // non-covering index
**
** Normally, nSeek is 1. nSeek values greater than 1 come about if the
** WHERE clause includes "x IN (....)" terms used in place of "x=?". Or when
** implicit "x IN (SELECT x FROM tbl)" terms are added for skip-scans.
**
** The estimated values (nRow, nVisit, nSeek) often contain a large amount
** of uncertainty.  For this reason, scoring is designed to pick plans that
** "do the least harm" if the estimates are inaccurate.  For example, a
** log(nRow) factor is omitted from a non-covering index scan in order to
** bias the scoring in favor of using an index, since the worst-case
** performance of using an index is far better than the worst-case performance
** of a full table scan.
*/
स्थिर पूर्णाङ्क where_Loop_सङ्कलय_Btree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mPrereq             /* Extra prerequisites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  सूचकांक *pProbe;              /* An index we are evaluating */
  सूचकांक sPk;                  /* A fake index object for the primary key */
  LogEst aiRowEstPk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 aiColumnPk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  SrcItem *pSrc;              /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  पूर्णाङ्क rc = SQLITE_OK;         /* Return code */
  पूर्णाङ्क i_क्रमय_Idx;               /* Index number */
  पूर्णाङ्क b;                      /* A boolean value */
  LogEst r_आकार;               /* number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  सारणी *pTab;                /* Table being queried */
 
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;
  प्रतिज्ञा( !IsVirtual(pSrc->pSTab) );

  यदि( pSrc->fg.is_अनुक्रमितम्_By ){
    प्रतिज्ञा( pSrc->fg.isCte==0 );
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->u2.pIBIndex;
  }अन्यथा यदि( !HasRowid(pTab) ){
    यदि( pSrc->fg.not_अनुक्रमितम् ){
      pProbe = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
    }अन्यथा{
      pProbe = pTab->p_सूचकांक;
    }
  }अन्यथा{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    सूचकांक *pFirst;                  /* First of real indices on the table */
    स्मृतिं_पूरय(&sPk, 0, परिमाण(सूचकांक));
    sPk.n_कुञ्चिका_Col = 1;
    sPk.nColumn = 1;
    sPk.aiColumn = &aiColumnPk;
    sPk.aiRowLogEst = aiRowEstPk;
    sPk.onError = OE_Replace;
    sPk.p_सारणी = pTab;
    sPk.szIdxRow = 3;  /* TUNING: Interior rows of IPK table are very small */
    sPk.idxType = SQLITE_IDXTYPE_IPK;
    aiRowEstPk[0] = pTab->nRowLogEst;
    aiRowEstPk[1] = 0;
    pFirst = pSrc->pSTab->p_सूचकांक;
    sPk.p_अग्र = pFirst;
    pProbe = &sPk;
  }
  r_आकार = pTab->nRowLogEst;

#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOMATIC_सूचकांक
  /* Automatic indexes */
  यदि( !pBuilder->p_Or_समुच्चयः      /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & (WHERE_दक्षिणः_JOIN|WHERE_OR_SUBCLAUSE))==0
   && (pWInfo->pParse->db->flags & SQLITE_Auto_सूचकांक)!=0
   && !pSrc->fg.is_अनुक्रमितम्_By  /* Has no INDEXED BY clause */
   && !pSrc->fg.not_अनुक्रमितम्   /* Has no NOT INDEXED clause */
   && !pSrc->fg.isCorrelated /* Not a correlated subquery */
   && !pSrc->fg.isRecursive  /* Not a recursive common table expression. */
   && (pSrc->fg.jointype & JT_दक्षिणः)==0 /* Not the right tab of a RIGHT JOIN */
  ){
    /* Generate auto-index WhereLoops */
    LogEst r_Log_आकार;         /* Logarithm of the number of rows in the table */
    Where_पदांशः *p_पदांशः;
    Where_पदांशः *pWCEnd = pWC->a + pWC->n_पदांशः;
    r_Log_आकार = estLog(r_आकार);
    प्रति(p_पदांशः=pWC->a; rc==SQLITE_OK && p_पदांशः<pWCEnd; p_पदांशः++){
      यदि( p_पदांशः->prereq_दक्षिणः & pNew->आवरणम्_Self ) अनुवर्त;
      यदि( पदांशः_Can_Drive_सूचकांक(p_पदांशः, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.p_सूचकांक = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = p_पदांशः;
        /* TUNING: One-time cost for computing the automatic index is
        ** estimated to be X*N*log2(N) where N is the number of rows in
        ** the table being indexed and where X is 7 (LogEst=28) for normal
        ** tables or 0.5 (LogEst=-10) for views and subqueries.  The value
        ** of X is smaller for views and subqueries so that the query planner
        ** will be more aggressive about generating automatic indexes for
        ** those objects, since there is no opportunity to add schema
        ** indexes on subqueries and views. */
        pNew->r_सज्जता = r_Log_आकार + r_आकार;
        यदि( !IsView(pTab) && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->r_सज्जता += 28;
        }अन्यथा{
          pNew->r_सज्जता -= 25;  /* Greatly reduced setup cost for auto indexes
                               ** on ephemeral materializations of views */
        }
        ApplyCostMultiplier(pNew->r_सज्जता, pTab->costMult);
        यदि( pNew->r_सज्जता<0 ) pNew->r_सज्जता = 0;
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->n_बहिर्गमनम् = 43;  प्रतिज्ञा( 43==sqlite3LogEst(20) );
        pNew->r_सततम् = sqlite3_Log_Est_सङ्कलय(r_Log_आकार,pNew->n_बहिर्गमनम्);
        pNew->wsFlags = WHERE_AUTO_सूचकांक;
        pNew->prereq = mPrereq | p_पदांशः->prereq_दक्षिणः;
        rc = where_Loop_निवेशय(pBuilder, pNew);
      }
    }
  }
#यदि_अन्तः /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices. If there was an INDEXED BY clause, then only
  ** consider index pProbe.  */
  प्रति(i_क्रमय_Idx=1;
      rc==SQLITE_OK && pProbe;
      pProbe=(pSrc->fg.is_अनुक्रमितम्_By||pSrc->fg.not_अनुक्रमितम्)?0:pProbe->p_अग्र,
         i_क्रमय_Idx++
  ){
    यदि( pProbe->pPartIdxWhere!=0
     && !where_Usable_Partial_सूचकांक(pSrc->iCursor, pSrc->fg.jointype, pWC,
                                 pProbe->pPartIdxWhere)
    ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      अनुवर्त;  /* Partial index inappropriate for this query */
    }
    यदि( pProbe->bNoQuery ) अनुवर्त;
    r_आकार = pProbe->aiRowLogEst[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.n_शिखरम् = 0;
    pNew->u.btree.nDistinctCol = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->i_क्रमय_Idx = 0;
    pNew->r_सज्जता = 0;
    pNew->prereq = mPrereq;
    pNew->n_बहिर्गमनम् = r_आकार;
    pNew->u.btree.p_सूचकांक = pProbe;
    pNew->u.btree.p_क्रमः_By = 0;
    b = सूचकांक_Might_Help_With_क्रमः_By(pBuilder, pProbe, pSrc->iCursor);

    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    प्रतिज्ञा( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    यदि( pProbe->idxType==SQLITE_IDXTYPE_IPK ){
      /* Integer primary key index */
      pNew->wsFlags = WHERE_IPK;

      /* Full table scan */
      pNew->i_क्रमय_Idx = b ? i_क्रमय_Idx : 0;
      /* TUNING: Cost of full table scan is 3.0*N.  The 3.0 factor is an
      ** extra cost designed to discourage the use of full table scans,
      ** since index lookups have better worst-case performance if our
      ** stat guesses are wrong.  Reduce the 3.0 penalty slightly
      ** (to 2.75) if we have valid STAT4 information for the table.
      ** At 2.75, a full table scan is preferred over using an index on
      ** a column with just two distinct values where each value has about
      ** an equal number of appearances.  Without STAT4 data, we still want
      ** to use an index in that case, since the constraint might be for
      ** the scarcer of the two values, and in that case an index lookup is
      ** better.
      */
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
      pNew->r_सततम् = r_आकार + 16 - 2*((pTab->tabFlags & TF_HasStat4)!=0);
#अन्यथा
      pNew->r_सततम् = r_आकार + 16;
#यदि_अन्तः
      ApplyCostMultiplier(pNew->r_सततम्, pTab->costMult);
      whereLoopOutputAdjust(pWC, pNew, r_आकार);
      यदि( pSrc->fg.isSubquery ){
        यदि( pSrc->fg.द्वारा_Coroutine ) pNew->wsFlags |= WHERE_COROUTINE;
        /* Do not set btree.pOrderBy for a recursive CTE. In this case
        ** the ORDER BY clause does not determine the overall order that
        ** rows are emitted from the CTE in.  */
        यदि( (pSrc->u4.pSubq->pSelect->selFlags & SF_Recursive)==0 ){
          pNew->u.btree.p_क्रमः_By = pSrc->u4.pSubq->pSelect->p_क्रमः_By;
        }
      }अन्यथा यदि( pSrc->fg.fromExists ){
        pNew->n_बहिर्गमनम् = 0;
      }
      rc = where_Loop_निवेशय(pBuilder, pNew);
      pNew->n_बहिर्गमनम् = r_आकार;
      यदि( rc ) भङ्ग;
    }अन्यथा{
      Bitmask m;
      यदि( pProbe->isCovering ){
        m = 0;
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_अनुक्रमितम्;
      }अन्यथा{
        m = pSrc->col_प्रयुक्तम् & pProbe->colNotIdxed;
        यदि( pProbe->pPartIdxWhere 
         && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)==0
        ){
          where_Part_Idx_व्यञ्जकम्(
              pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0
          );
        }
        pNew->wsFlags = WHERE_अनुक्रमितम्;
        यदि( m==TOPBIT || (pProbe->b_Has_व्यञ्जकम् && !pProbe->bHasVCol && m!=0) ){
          u32 isCov = where_Is_Covering_सूचकांक(pWInfo, pProbe, pSrc->iCursor);
          यदि( isCov==0 ){
            WHERETRACE(0x200,
               ("-> %s is not a covering index"
                " according to whereIsCoveringIndex()\n", pProbe->zName));
            प्रतिज्ञा( m!=0 );
          }अन्यथा{
            m = 0;
            pNew->wsFlags |= isCov;
            यदि( isCov & WHERE_IDX_ONLY ){
              WHERETRACE(0x200,
                 ("-> %s is a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }अन्यथा{
              प्रतिज्ञा( isCov==WHERE_EXPRIDX );
              WHERETRACE(0x200,
                 ("-> %s might be a covering expression index"
                  " according to whereIsCoveringIndex()\n", pProbe->zName));
            }
          }
        }अन्यथा यदि( m==0 
           && (HasRowid(pTab) || pWInfo->pSelect!=0 || sqlite3FaultSim(700))
        ){
          WHERETRACE(0x200,
             ("-> %s is a covering index according to bitmasks\n",
             pProbe->zName, m==0 ? "is" : "is not"));
          pNew->wsFlags = WHERE_IDX_ONLY | WHERE_अनुक्रमितम्;
        }
      }

      /* Full scan via index */
      यदि( b
       || !HasRowid(pTab)
       || pProbe->pPartIdxWhere!=0
       || pSrc->fg.is_अनुक्रमितम्_By
       || ( m==0
         && pProbe->bUnordered==0
         && (pProbe->szIdxRow<pTab->szTabRow)
         && (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0
         && sqlite3GlobalConfig.bUseCis
         && OptimizationEnabled(pWInfo->pParse->db, SQLITE_Cover_Idx_अवलोकनम्)
          )
      ){
        pNew->i_क्रमय_Idx = b ? i_क्रमय_Idx : 0;

        /* The cost of visiting the index rows is N*K, where K is
        ** between 1.1 and 3.0, depending on the relative sizes of the
        ** index and table rows. */
        pNew->r_सततम् = r_आकार + 1 + (15*pProbe->szIdxRow)/pTab->szTabRow;
        यदि( m!=0 ){
          /* If this is a non-covering index scan, add in the cost of
          ** doing table lookups.  The cost will be 3x the number of
          ** lookups.  Take into account WHERE clause terms that can be
          ** satisfied using just the index, and that do not require a
          ** table lookup. */
          LogEst nLookup = r_आकार + 16;  /* Base cost:  N*3 */
          पूर्णाङ्क ii;
          पूर्णाङ्क iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          प्रति(ii=0; ii<pWC2->n_पदांशः; ii++){
            Where_पदांशः *p_पदांशः = &pWC2->a[ii];
            यदि( !sqlite3_व्यञ्जकम्_Covered_By_सूचकांक(p_पदांशः->p_व्यञ्जकम्, iCur, pProbe) ){
              भङ्ग;
            }
            /* pTerm can be evaluated using just the index.  So reduce
            ** the expected number of table lookups accordingly */
            यदि( p_पदांशः->truthProb<=0 ){
              nLookup += p_पदांशः->truthProb;
            }अन्यथा{
              nLookup--;
              यदि( p_पदांशः->eOperator & (WO_EQ|WO_IS) ) nLookup -= 19;
            }
          }
         
          pNew->r_सततम् = sqlite3_Log_Est_सङ्कलय(pNew->r_सततम्, nLookup);
        }
        ApplyCostMultiplier(pNew->r_सततम्, pTab->costMult);
        whereLoopOutputAdjust(pWC, pNew, r_आकार);
        यदि( (pSrc->fg.jointype & JT_दक्षिणः)!=0 && pProbe->a_Col_व्यञ्जकम् ){
          /* Do not do an SCAN of a index-on-expression in a RIGHT JOIN
          ** because the cursor used to access the index might not be
          ** positioned to the correct row during the right-join no-match
          ** loop. */
        }अन्यथा{
          यदि( pSrc->fg.fromExists ) pNew->n_बहिर्गमनम् = 0;
          rc = where_Loop_निवेशय(pBuilder, pNew);
        }
        pNew->n_बहिर्गमनम् = r_आकार;
        यदि( rc ) भङ्ग;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = where_Loop_सङ्कलय_Btree_सूचकांक(pBuilder, pSrc, pProbe, 0);
    यदि( pBuilder->bldFlags1==SQLITE_BLDF1_अनुक्रमितम् ){
      /* If a non-unique index is used, or if a prefix of the key for
      ** unique index is used (making the index functionally non-unique)
      ** then the sqlite_stat1 data becomes important for scoring the
      ** plan */
      pTab->tabFlags |= TF_MaybeReanalyze;
    }
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#यदि_अन्तः
  }
  प्रत्यागम rc;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE

/*
** Return true if pTerm is a virtual table LIMIT or OFFSET term.
*/
स्थिर पूर्णाङ्क is_सीमा_पदांशः(Where_पदांशः *p_पदांशः){
  प्रतिज्ञा( p_पदांशः->eOperator==WO_AUX || p_पदांशः->e_मेलनम्_Op==0 );
  प्रत्यागम p_पदांशः->e_मेलनम्_Op>=SQLITE_सूचकांक_CONSTRAINT_सीमा
      && p_पदांशः->e_मेलनम्_Op<=SQLITE_सूचकांक_CONSTRAINT_विस्थापनम्;
}

/*
** Return true if the first nCons constraints in the pUsage array are
** marked as in-use (have argvIndex>0). False otherwise.
*/
स्थिर पूर्णाङ्क all_Constraints_प्रयुक्तम्(
  संरचना sqlite3_सूचकांक_constraint_usage *aUsage, 
  पूर्णाङ्क nCons
){
  पूर्णाङ्क ii;
  प्रति(ii=0; ii<nCons; ii++){
    यदि( aUsage[ii].argv_सूचकांक<=0 ) प्रत्यागम 0;
  }
  प्रत्यागम 1;
}

/*
** Argument pIdxInfo is already populated with all constraints that may
** be used by the virtual table identified by pBuilder->pNew->iTab. This
** function marks a subset of those constraints usable, invokes the
** xBestIndex method and adds the returned plan to pBuilder.
**
** A constraint is marked usable if:
**
**   * Argument mUsable indicates that its prerequisites are available, and
**
**   * It is not one of the operators specified in the mExclude mask passed
**     as the fourth argument (which in practice is either WO_IN or 0).
**
** Argument mPrereq is a mask of tables that must be scanned before the
** virtual table in question. These are added to the plans prerequisites
** before it is added to pBuilder.
**
** Output parameter *pbIn is set to true if the plan added to pBuilder
** uses one or more WO_IN terms, or false otherwise.
*/
स्थिर पूर्णाङ्क where_Loop_सङ्कलय_Virtual_One(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,                /* Mask of tables that must be used. */
  Bitmask mUsable,                /* Mask of usable tables */
  u16 mExclude,                   /* Exclude terms using these operators */
  sqlite3_सूचकांक_info *pIdxInfo,   /* Populated object for xBestIndex */
  u16 mNoOmit,                    /* Do not omit these constraints */
  पूर्णाङ्क *pb_अन्तर्,                      /* OUT: True if plan uses an IN(...) op */
  पूर्णाङ्क *pb_Retry_सीमा               /* OUT: Retry without LIMIT/OFFSET */
){
  WhereClause *pWC = pBuilder->pWC;
  Hidden_सूचकांक_Info *pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  संरचना sqlite3_सूचकांक_constraint *pIdxCons;
  संरचना sqlite3_सूचकांक_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  पूर्णाङ्क i;
  पूर्णाङ्क mx_पदांशः;
  पूर्णाङ्क rc = SQLITE_OK;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  SrcItem *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  पूर्णाङ्क nConstraint = pIdxInfo->nConstraint;

  प्रतिज्ञा( (mUsable & mPrereq)==mPrereq );
  *pb_अन्तर् = 0;
  pNew->prereq = mPrereq;

  /* Set the usable flag on the subset of constraints identified by
  ** arguments mUsable and mExclude. */
  pIdxCons = *(संरचना sqlite3_सूचकांक_constraint**)&pIdxInfo->aConstraint;
  प्रति(i=0; i<nConstraint; i++, pIdxCons++){
    Where_पदांशः *p_पदांशः = पदांशः_From_Where_Clause(pWC, pIdxCons->i_पदांशः_विस्थापनम्);
    pIdxCons->usable = 0;
    यदि( (p_पदांशः->prereq_दक्षिणः & mUsable)==p_पदांशः->prereq_दक्षिणः
     && (p_पदांशः->eOperator & mExclude)==0
     && (pb_Retry_सीमा || !is_सीमा_पदांशः(p_पदांशः))
    ){
      pIdxCons->usable = 1;
    }
  }

  /* Initialize the output fields of the sqlite3_index_info structure */
  स्मृतिं_पूरय(pUsage, 0, परिमाण(pUsage[0])*nConstraint);
  प्रतिज्ञा( pIdxInfo->need_गन्तव्यम्_Free_Idx_Str==0 );
  pIdxInfo->idxStr = 0;
  pIdxInfo->idx_अङ्कः = 0;
  pIdxInfo->क्रमः_By_Consumed = 0;
  pIdxInfo->estimatedCost = SQLITE_BIG_DBL / (द्विगुण)2;
  pIdxInfo->estimated_पङ्क्तिकाः = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->m_Handle_अन्तर् = 0;

  /* Invoke the virtual table xBestIndex() method */
  rc = vtab_Best_सूचकांक(pParse, pSrc->pSTab, pIdxInfo);
  यदि( rc ){
    यदि( rc==SQLITE_CONSTRAINT ){
      /* If the xBestIndex method returns SQLITE_CONSTRAINT, that means
      ** that the particular combination of parameters provided is unusable.
      ** Make no entries in the loop table.
      */
      WHERETRACE(0xffffffff, ("  ^^^^--- non-viable plan rejected!\n"));
      freeIdxStr(pIdxInfo);
      प्रत्यागम SQLITE_OK;
    }
    प्रत्यागम rc;
  }

  mx_पदांशः = -1;
  प्रतिज्ञा( pNew->nLSlot>=nConstraint );
  स्मृतिं_पूरय(pNew->aLTerm, 0, परिमाण(pNew->aLTerm[0])*nConstraint );
  स्मृतिं_पूरय(&pNew->u.vtab, 0, परिमाण(pNew->u.vtab));
  pIdxCons = *(संरचना sqlite3_सूचकांक_constraint**)&pIdxInfo->aConstraint;
  प्रति(i=0; i<nConstraint; i++, pIdxCons++){
    पूर्णाङ्क i_पदांशः;
    यदि( (i_पदांशः = pUsage[i].argv_सूचकांक - 1)>=0 ){
      Where_पदांशः *p_पदांशः;
      पूर्णाङ्क j = pIdxCons->i_पदांशः_विस्थापनम्;
      यदि( i_पदांशः>=nConstraint
       || j<0
       || (p_पदांशः = पदांशः_From_Where_Clause(pWC, j))==0
       || pNew->aLTerm[i_पदांशः]!=0
       || pIdxCons->usable==0
      ){
        sqlite3_Error_सन्देशः(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        प्रत्यागम SQLITE_ERROR;
      }
      testcase( i_पदांशः==nConstraint-1 );
      testcase( j==0 );
      testcase( j==pWC->n_पदांशः-1 );
      pNew->prereq |= p_पदांशः->prereq_दक्षिणः;
      प्रतिज्ञा( i_पदांशः<pNew->nLSlot );
      pNew->aLTerm[i_पदांशः] = p_पदांशः;
      यदि( i_पदांशः>mx_पदांशः ) mx_पदांशः = i_पदांशः;
      testcase( i_पदांशः==15 );
      testcase( i_पदांशः==16 );
      यदि( pUsage[i].omit ){
        यदि( i<16 && ((1<<i)&mNoOmit)==0 ){
          testcase( i!=i_पदांशः );
          pNew->u.vtab.omit_आवरणम् |= 1<<i_पदांशः;
        }अन्यथा{
          testcase( i!=i_पदांशः );
        }
        यदि( p_पदांशः->e_मेलनम्_Op==SQLITE_सूचकांक_CONSTRAINT_विस्थापनम् ){
          pNew->u.vtab.b_Omit_विस्थापनम् = 1;
        }
      }
      यदि( SMASKBIT32(i) & pHidden->m_Handle_अन्तर् ){
        pNew->u.vtab.m_Handle_अन्तर् |= MASKBIT32(i_पदांशः);
      }अन्यथा यदि( (p_पदांशः->eOperator & WO_अन्तर्)!=0 ){
        /* A virtual table that is constrained by an IN clause may not
        ** consume the ORDER BY clause because (1) the order of IN terms
        ** is not necessarily related to the order of output terms and
        ** (2) Multiple outputs from a single IN value will not merge
        ** together.  */
        pIdxInfo->क्रमः_By_Consumed = 0;
        pIdxInfo->idxFlags &= ~SQLITE_सूचकांक_अवलोकनम्_UNIQUE;
        *pb_अन्तर् = 1; प्रतिज्ञा( (mExclude & WO_अन्तर्)==0 );
      }

      /* Unless pbRetryLimit is non-NULL, there should be no LIMIT/OFFSET
      ** terms. And if there are any, they should follow all other terms. */
      प्रतिज्ञा( pb_Retry_सीमा || !is_सीमा_पदांशः(p_पदांशः) );
      प्रतिज्ञा( !is_सीमा_पदांशः(p_पदांशः) || i>=nConstraint-2 );
      प्रतिज्ञा( !is_सीमा_पदांशः(p_पदांशः) || i==nConstraint-1 || is_सीमा_पदांशः(p_पदांशः+1) );

      यदि( is_सीमा_पदांशः(p_पदांशः) && (*pb_अन्तर् || !all_Constraints_प्रयुक्तम्(pUsage, i)) ){
        /* If there is an IN(...) term handled as an == (separate call to
        ** xFilter for each value on the RHS of the IN) and a LIMIT or
        ** OFFSET term handled as well, the plan is unusable. Similarly,
        ** if there is a LIMIT/OFFSET and there are other unused terms,
        ** the plan cannot be used. In these cases set variable *pbRetryLimit
        ** to true to tell the caller to retry with LIMIT and OFFSET 
        ** disabled. */
        freeIdxStr(pIdxInfo);
        *pb_Retry_सीमा = 1;
        प्रत्यागम SQLITE_OK;
      }
    }
  }

  pNew->nLTerm = mx_पदांशः+1;
  प्रति(i=0; i<=mx_पदांशः; i++){
    यदि( pNew->aLTerm[i]==0 ){
      /* The non-zero argvIdx values must be contiguous.  Raise an
      ** error if they are not */
      sqlite3_Error_सन्देशः(pParse,"%s.xBestIndex malfunction",pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      प्रत्यागम SQLITE_ERROR;
    }
  }
  प्रतिज्ञा( pNew->nLTerm<=pNew->nLSlot );
  pNew->u.vtab.idx_अङ्कः = pIdxInfo->idx_अङ्कः;
  pNew->u.vtab.needFree = pIdxInfo->need_गन्तव्यम्_Free_Idx_Str;
  pIdxInfo->need_गन्तव्यम्_Free_Idx_Str = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->क्रमः_By_Consumed ?
      pIdxInfo->n_क्रमः_By : 0);
  pNew->u.vtab.b_Idx_अङ्कः_Hex = (pIdxInfo->idxFlags&SQLITE_सूचकांक_अवलोकनम्_HEX)!=0;
  pNew->r_सज्जता = 0;
  pNew->r_सततम् = sqlite3LogEstFromDouble(pIdxInfo->estimatedCost);
  pNew->n_बहिर्गमनम् = sqlite3LogEst(pIdxInfo->estimated_पङ्क्तिकाः);

  /* Set the WHERE_ONEROW flag if the xBestIndex() method indicated
  ** that the scan will visit at most one row. Clear it otherwise. */
  यदि( pIdxInfo->idxFlags & SQLITE_सूचकांक_अवलोकनम्_UNIQUE ){
    pNew->wsFlags |= WHERE_ONEROW;
  }अन्यथा{
    pNew->wsFlags &= ~WHERE_ONEROW;
  }
  rc = where_Loop_निवेशय(pBuilder, pNew);
  यदि( pNew->u.vtab.needFree ){
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }
  WHERETRACE(0xffffffff, ("  bIn=%d prereqIn=%04llx prereqOut=%04llx\n",
                      *pb_अन्तर्, (sqlite3_uint64)mPrereq,
                      (sqlite3_uint64)(pNew->prereq & ~mPrereq)));

  प्रत्यागम rc;
}

/*
** Return the collating sequence for a constraint passed into xBestIndex.
**
** pIdxInfo must be an sqlite3_index_info structure passed into xBestIndex.
** This routine depends on there being a HiddenIndexInfo structure immediately
** following the sqlite3_index_info structure.
**
** Return a pointer to the collation name:
**
**    1. If there is an explicit COLLATE operator on the constraint, return it.
**
**    2. Else, if the column has an alternative collation, return that.
**
**    3. Otherwise, return "BINARY".
*/
ध्रुव वर्ण *sqlite3_vtab_collation(sqlite3_सूचकांक_info *pIdxInfo, पूर्णाङ्क iCons){
  Hidden_सूचकांक_Info *pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  ध्रुव वर्ण *zRet = 0;
  यदि( iCons>=0 && iCons<pIdxInfo->nConstraint ){
    CollSeq *pC = 0;
    पूर्णाङ्क i_पदांशः = pIdxInfo->aConstraint[iCons].i_पदांशः_विस्थापनम्;
    व्यञ्जकम् *pX = पदांशः_From_Where_Clause(pHidden->pWC, i_पदांशः)->p_व्यञ्जकम्;
    यदि( pX->p_वामः ){
      pC = sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3_Str_द्विमान);
  }
  प्रत्यागम zRet;
}

/*
** Return true if constraint iCons is really an IN(...) constraint, or
** false otherwise. If iCons is an IN(...) constraint, set (if bHandle!=0)
** or clear (if bHandle==0) the flag to handle it using an iterator.
*/
पूर्णाङ्क sqlite3_vtab_अन्तर्(sqlite3_सूचकांक_info *pIdxInfo, पूर्णाङ्क iCons, पूर्णाङ्क bHandle){
  Hidden_सूचकांक_Info *pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  u32 m = SMASKBIT32(iCons);
  यदि( m & pHidden->m_अन्तर् ){
    यदि( bHandle==0 ){
      pHidden->m_Handle_अन्तर् &= ~m;
    }अन्यथा यदि( bHandle>0 ){
      pHidden->m_Handle_अन्तर् |= m;
    }
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

/*
** This interface is callable from within the xBestIndex callback only.
**
** If possible, set (*ppVal) to point to an object containing the value
** on the right-hand-side of constraint iCons.
*/
पूर्णाङ्क sqlite3_vtab_rhs_मानम्(
  sqlite3_सूचकांक_info *pIdxInfo,   /* Copy of first argument to xBestIndex */
  पूर्णाङ्क iCons,                      /* Constraint for which RHS is wanted */
  sqlite3_मानम् **ppVal           /* Write value extracted here */
){
  Hidden_सूचकांक_Info *pH = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  sqlite3_मानम् *pVal = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  यदि( iCons<0 || iCons>=pIdxInfo->nConstraint ){
    rc = SQLITE_MISUSE_BKPT; /* EV: R-30545-25046 */
  }अन्यथा{
    यदि( pH->aRhs[iCons]==0 ){
      Where_पदांशः *p_पदांशः = पदांशः_From_Where_Clause(
          pH->pWC, pIdxInfo->aConstraint[iCons].i_पदांशः_विस्थापनम्
      );
      rc = sqlite3_मानम्_From_व्यञ्जकम्(
          pH->pParse->db, p_पदांशः->p_व्यञ्जकम्->p_दक्षिणः, ENC(pH->pParse->db),
          SQLITE_AFF_BLOB, &pH->aRhs[iCons]
      );
      testcase( rc!=SQLITE_OK );
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  यदि( rc==SQLITE_OK && pVal==0 ){  /* IMP: R-19933-32160 */
    rc = SQLITE_NOTFOUND;          /* IMP: R-36424-56542 */
  }

  प्रत्यागम rc;
}

/*
** Return true if ORDER BY clause may be handled as DISTINCT.
*/
पूर्णाङ्क sqlite3_vtab_distinct(sqlite3_सूचकांक_info *pIdxInfo){
  Hidden_सूचकांक_Info *pHidden = (Hidden_सूचकांक_Info*)&pIdxInfo[1];
  प्रतिज्ञा( pHidden->eDistinct>=0 && pHidden->eDistinct<=3 );
  प्रत्यागम pHidden->eDistinct;
}

/*
** Cause the prepared statement that is associated with a call to
** xBestIndex to potentially use all schemas.  If the statement being
** prepared is read-only, then just start read transactions on all
** schemas.  But if this is a write operation, start writes on all
** schemas.
**
** This is used by the (built-in) sqlite_dbpage virtual table.
*/
शून्य sqlite3_Vtab_उपयुङ्क्ते_All_Schemas(Parse *pParse){
  पूर्णाङ्क nDb = pParse->db->nDb;
  पूर्णाङ्क i;
  प्रति(i=0; i<nDb; i++){
    sqlite3_संहिता_Verify_Schema(pParse, i);
  }
  यदि( Db_आवरणम्_Non_Zero(pParse->write_आवरणम्) ){
    प्रति(i=0; i<nDb; i++){
      sqlite3BeginWriteOperation(pParse, 0, i);
    }
  }
}

/*
** Add all WhereLoop objects for a table of the join identified by
** pBuilder->pNew->iTab.  That table is guaranteed to be a virtual table.
**
** If there are no LEFT or CROSS JOIN joins in the query, both mPrereq and
** mUnusable are set to 0. Otherwise, mPrereq is a mask of all FROM clause
** entries that occur before the virtual table in the FROM clause and are
** separated from it by at least one LEFT or CROSS JOIN. Similarly, the
** mUnusable mask contains all FROM clause entries that occur after the
** virtual table and are separated from it by at least one LEFT or
** CROSS JOIN.
**
** For example, if the query were:
**
**   ... FROM t1, t2 LEFT JOIN t3, t4, vt CROSS JOIN t5, t6;
**
** then mPrereq corresponds to (t1, t2) and mUnusable to (t5, t6).
**
** All the tables in mPrereq must be scanned before the current virtual
** table. So any terms for which all prerequisites are satisfied by
** mPrereq may be specified as "usable" in all calls to xBestIndex.
** Conversely, all tables in mUnusable must be scanned after the current
** virtual table, so any terms for which the prerequisites overlap with
** mUnusable should always be configured as "not-usable" for xBestIndex.
*/
स्थिर पूर्णाङ्क where_Loop_सङ्कलय_Virtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mPrereq,             /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  पूर्णाङ्क rc = SQLITE_OK;          /* Return code */
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  SrcItem *pSrc;               /* The FROM clause term to search */
  sqlite3_सूचकांक_info *p;       /* Object to pass to xBestIndex() */
  पूर्णाङ्क nConstraint;             /* Number of constraints in p */
  पूर्णाङ्क b_अन्तर्;                     /* True if plan uses IN(...) operator */
  WhereLoop *pNew;
  Bitmask mBest;               /* Tables used by best possible plan */
  u16 mNoOmit;
  पूर्णाङ्क bRetry = 0;              /* True to retry with LIMIT/OFFSET disabled */

  प्रतिज्ञा( (mPrereq & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  प्रतिज्ञा( IsVirtual(pSrc->pSTab) );
  p = allocate_सूचकांक_Info(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  यदि( p==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
  pNew->r_सज्जता = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  यदि( whereLoopResize(pParse->db, pNew, nConstraint) ){
    free_सूचकांक_Info(pParse->db, p);
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }

  /* First call xBestIndex() with all constraints usable. */
  WHERETRACE(0x800, ("BEGIN %s.addVirtual()\n", pSrc->pSTab->zName));
  WHERETRACE(0x800, ("  VirtualOne: all usable\n"));
  rc = where_Loop_सङ्कलय_Virtual_One(
      pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_अन्तर्, &bRetry
  );
  यदि( bRetry ){
    प्रतिज्ञा( rc==SQLITE_OK );
    rc = where_Loop_सङ्कलय_Virtual_One(
        pBuilder, mPrereq, ALLBITS, 0, p, mNoOmit, &b_अन्तर्, 0
    );
  }

  /* If the call to xBestIndex() with all terms enabled produced a plan
  ** that does not require any source tables (IOW: a plan with mBest==0)
  ** and does not use an IN(...) operator, then there is no point in making
  ** any further calls to xBestIndex() since they will all return the same
  ** result (if the xBestIndex() implementation is sane). */
  यदि( rc==SQLITE_OK && ((mBest = (pNew->prereq & ~mPrereq))!=0 || b_अन्तर्) ){
    पूर्णाङ्क आलोकितम्_Zero = 0;             /* True if a plan with no prereqs seen */
    पूर्णाङ्क आलोकितम्_Zero_No_अन्तर् = 0;         /* Plan with no prereqs and no IN(...) seen */
    Bitmask m_पूर्व = 0;
    Bitmask m_Best_No_अन्तर् = 0;

    /* If the plan produced by the earlier call uses an IN(...) term, call
    ** xBestIndex again, this time with IN(...) terms disabled. */
    यदि( b_अन्तर् ){
      WHERETRACE(0x800, ("  VirtualOne: all usable w/o IN\n"));
      rc = where_Loop_सङ्कलय_Virtual_One(
          pBuilder, mPrereq, ALLBITS, WO_अन्तर्, p, mNoOmit, &b_अन्तर्, 0);
      प्रतिज्ञा( b_अन्तर्==0 );
      m_Best_No_अन्तर् = pNew->prereq & ~mPrereq;
      यदि( m_Best_No_अन्तर्==0 ){
        आलोकितम्_Zero = 1;
        आलोकितम्_Zero_No_अन्तर् = 1;
      }
    }

    /* Call xBestIndex once for each distinct value of (prereqRight & ~mPrereq)
    ** in the set of terms that apply to the current virtual table.  */
    यावत्( rc==SQLITE_OK ){
      पूर्णाङ्क i;
      Bitmask m_अग्र = ALLBITS;
      प्रतिज्ञा( m_अग्र>0 );
      प्रति(i=0; i<nConstraint; i++){
        पूर्णाङ्क i_पदांशः = p->aConstraint[i].i_पदांशः_विस्थापनम्;
        Bitmask mThis = पदांशः_From_Where_Clause(pWC, i_पदांशः)->prereq_दक्षिणः & ~mPrereq;
        यदि( mThis>m_पूर्व && mThis<m_अग्र ) m_अग्र = mThis;
      }
      m_पूर्व = m_अग्र;
      यदि( m_अग्र==ALLBITS ) भङ्ग;
      यदि( m_अग्र==mBest || m_अग्र==m_Best_No_अन्तर् ) अनुवर्त;
      WHERETRACE(0x800, ("  VirtualOne: mPrev=%04llx mNext=%04llx\n",
                       (sqlite3_uint64)m_पूर्व, (sqlite3_uint64)m_अग्र));
      rc = where_Loop_सङ्कलय_Virtual_One(
          pBuilder, mPrereq, m_अग्र|mPrereq, 0, p, mNoOmit, &b_अन्तर्, 0);
      यदि( pNew->prereq==mPrereq ){
        आलोकितम्_Zero = 1;
        यदि( b_अन्तर्==0 ) आलोकितम्_Zero_No_अन्तर् = 1;
      }
    }

    /* If the calls to xBestIndex() in the above loop did not find a plan
    ** that requires no source tables at all (i.e. one guaranteed to be
    ** usable), make a call here with all source tables disabled */
    यदि( rc==SQLITE_OK && आलोकितम्_Zero==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled\n"));
      rc = where_Loop_सङ्कलय_Virtual_One(
          pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &b_अन्तर्, 0);
      यदि( b_अन्तर्==0 ) आलोकितम्_Zero_No_अन्तर् = 1;
    }

    /* If the calls to xBestIndex() have so far failed to find a plan
    ** that requires no source tables at all and does not use an IN(...)
    ** operator, make a final call to obtain one here.  */
    यदि( rc==SQLITE_OK && आलोकितम्_Zero_No_अन्तर्==0 ){
      WHERETRACE(0x800, ("  VirtualOne: all disabled and w/o IN\n"));
      rc = where_Loop_सङ्कलय_Virtual_One(
          pBuilder, mPrereq, mPrereq, WO_अन्तर्, p, mNoOmit, &b_अन्तर्, 0);
    }
  }

  free_सूचकांक_Info(pParse->db, p);
  WHERETRACE(0x800, ("END %s.addVirtual(), rc=%d\n", pSrc->pSTab->zName, rc));
  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
स्थिर पूर्णाङ्क where_Loop_सङ्कलय_Or(
  WhereLoopBuilder *pBuilder,
  Bitmask mPrereq,
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  Where_पदांशः *p_पदांशः, *pWCEnd;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क iCur;
  WhereClause अस्थायी_WC;
  WhereLoopBuilder sSubBuild;
  Where_Or_समुच्चयः s_योग, sCur;
  SrcItem *pItem;
 
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->n_पदांशः;
  pNew = pBuilder->pNew;
  स्मृतिं_पूरय(&s_योग, 0, परिमाण(s_योग));
  pItem = pWInfo->pTabList->a + pNew->iTab;
  iCur = pItem->iCursor;

  /* The multi-index OR optimization does not work for RIGHT and FULL JOIN */
  यदि( pItem->fg.jointype & JT_दक्षिणः ) प्रत्यागम SQLITE_OK;

  प्रति(p_पदांशः=pWC->a; p_पदांशः<pWCEnd && rc==SQLITE_OK; p_पदांशः++){
    यदि( (p_पदांशः->eOperator & WO_OR)!=0
     && (p_पदांशः->u.pOrInfo->indexable & pNew->आवरणम्_Self)!=0
     && (!व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON)
          || p_पदांशः->p_व्यञ्जकम्->w.iJoin==iCur)
    ){
      WhereClause * ध्रुव pOrWC = &p_पदांशः->u.pOrInfo->wc;
      Where_पदांशः * ध्रुव pOrWCEnd = &pOrWC->a[pOrWC->n_पदांशः];
      Where_पदांशः *p_Or_पदांशः;
      पूर्णाङ्क once = 1;
      पूर्णाङ्क i, j;
   
      sSubBuild = *pBuilder;
      sSubBuild.p_Or_समुच्चयः = &sCur;

      WHERETRACE(0x400, ("Begin processing OR-clause %p\n", p_पदांशः));
      प्रति(p_Or_पदांशः=pOrWC->a; p_Or_पदांशः<pOrWCEnd; p_Or_पदांशः++){
        यदि( (p_Or_पदांशः->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &p_Or_पदांशः->u.pAndInfo->wc;
        }अन्यथा यदि( p_Or_पदांशः->वामः_Cursor==iCur ){
          अस्थायी_WC.pWInfo = pWC->pWInfo;
          अस्थायी_WC.pOuter = pWC;
          अस्थायी_WC.op = TK_AND;
          अस्थायी_WC.n_पदांशः = 1;
          अस्थायी_WC.n_आधारः = 1;
          अस्थायी_WC.a = p_Or_पदांशः;
          sSubBuild.pWC = &अस्थायी_WC;
        }अन्यथा{
          अनुवर्त;
        }
        sCur.n = 0;
#यदि_परिभाषितम् WHERETRACE_ENABLED
        WHERETRACE(0x400, ("OR-term %d of %p has %d subterms:\n",
                   (पूर्णाङ्क)(p_Or_पदांशः-pOrWC->a), p_पदांशः, sSubBuild.pWC->n_पदांशः));
        यदि( sqlite3WhereTrace & 0x20000 ){
          sqlite3WhereClausePrint(sSubBuild.pWC);
        }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
        यदि( IsVirtual(pItem->pSTab) ){
          rc = where_Loop_सङ्कलय_Virtual(&sSubBuild, mPrereq, mUnusable);
        }अन्यथा
#यदि_अन्तः
        {
          rc = where_Loop_सङ्कलय_Btree(&sSubBuild, mPrereq);
        }
        यदि( rc==SQLITE_OK ){
          rc = where_Loop_सङ्कलय_Or(&sSubBuild, mPrereq, mUnusable);
        }
        testcase( rc==SQLITE_NOMEM && sCur.n>0 );
        testcase( rc==SQLITE_समाप्तम् );
        यदि( sCur.n==0 ){
          s_योग.n = 0;
          भङ्ग;
        }अन्यथा यदि( once ){
          whereOrMove(&s_योग, &sCur);
          once = 0;
        }अन्यथा{
          Where_Or_समुच्चयः s_पूर्व;
          whereOrMove(&s_पूर्व, &s_योग);
          s_योग.n = 0;
          प्रति(i=0; i<s_पूर्व.n; i++){
            प्रति(j=0; j<sCur.n; j++){
              where_Or_निवेशय(&s_योग, s_पूर्व.a[i].prereq | sCur.a[j].prereq,
                            sqlite3_Log_Est_सङ्कलय(s_पूर्व.a[i].r_सततम्, sCur.a[j].r_सततम्),
                            sqlite3_Log_Est_सङ्कलय(s_पूर्व.a[i].n_बहिर्गमनम्, sCur.a[j].n_बहिर्गमनम्));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = p_पदांशः;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->r_सज्जता = 0;
      pNew->i_क्रमय_Idx = 0;
      स्मृतिं_पूरय(&pNew->u, 0, परिमाण(pNew->u));
      प्रति(i=0; rc==SQLITE_OK && i<s_योग.n; i++){
        /* TUNING: Currently sSum.a[i].rRun is set to the sum of the costs
        ** of all sub-scans required by the OR-scan. However, due to rounding
        ** errors, it may be that the cost of the OR-scan is equal to its
        ** most expensive sub-scan. Add the smallest possible penalty
        ** (equivalent to multiplying the cost by 1.07) to ensure that
        ** this does not happen. Otherwise, for WHERE clauses such as the
        ** following where there is an index on "y":
        **
        **     WHERE likelihood(x=?, 0.99) OR y=?
        **
        ** the planner may elect to "OR" together a full-table scan and an
        ** index lookup. And other similarly odd results.  */
        pNew->r_सततम् = s_योग.a[i].r_सततम् + 1;
        pNew->n_बहिर्गमनम् = s_योग.a[i].n_बहिर्गमनम्;
        pNew->prereq = s_योग.a[i].prereq;
        rc = where_Loop_निवेशय(pBuilder, pNew);
      }
      WHERETRACE(0x400, ("End processing OR-clause %p\n", p_पदांशः));
    }
  }
  प्रत्यागम rc;
}

/*
** Add all WhereLoop objects for all tables
*/
स्थिर पूर्णाङ्क where_Loop_सङ्कलय_All(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask m_प्राक् = 0;
  पूर्णाङ्क iTab;
  SrcList *pTabList = pWInfo->pTabList;
  SrcItem *pItem;
  SrcItem *p_अन्तः = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क bFirstPastRJ = 0;
  पूर्णाङ्क has_दक्षिणः_Cross_Join = 0;
  WhereLoop *pNew;


  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;

  /* Verify that pNew has already been initialized */
  प्रतिज्ञा( pNew->nLTerm==0 );
  प्रतिज्ञा( pNew->wsFlags==0 );
  प्रतिज्ञा( pNew->nLSlot>=Array_आकार(pNew->aLTermSpace) );
  प्रतिज्ञा( pNew->aLTerm!=0 );

  pBuilder->i_Plan_सीमा = SQLITE_QUERY_PLANNER_सीमा;
  प्रति(iTab=0, pItem=pTabList->a; pItem<p_अन्तः; iTab++, pItem++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->i_Plan_सीमा += SQLITE_QUERY_PLANNER_सीमा_INCR;
    pNew->आवरणम्_Self = sqlite3_Where_Get_आवरणम्(&pWInfo->s_आवरणम्_समुच्चयः, pItem->iCursor);
    यदि( bFirstPastRJ
     || (pItem->fg.jointype & (JT_OUTER|JT_CROSS|JT_LTORJ))!=0
    ){
      /* Add prerequisites to prevent reordering of FROM clause terms
      ** across CROSS joins and outer joins.  The bFirstPastRJ boolean
      ** prevents the right operand of a RIGHT JOIN from being swapped with
      ** other elements even further to the right.
      **
      ** The hasRightCrossJoin flag prevent FROM-clause terms from moving
      ** from the right side of a LEFT JOIN or CROSS JOIN over to the
      ** left side of that same join.  This is a required restriction in
      ** the case of LEFT JOIN - an incorrect answer may results if it is
      ** not enforced.  This restriction is not required for CROSS JOIN.
      ** It is provided merely as a means of controlling join order, under
      ** the theory that no real-world queries that care about performance
      ** actually use the CROSS JOIN syntax.
      */
      यदि( pItem->fg.jointype & (JT_LTORJ|JT_CROSS) ){
        testcase( pItem->fg.jointype & JT_LTORJ );
        testcase( pItem->fg.jointype & JT_CROSS );
        has_दक्षिणः_Cross_Join = 1;
      }
      mPrereq |= m_प्राक्;
      bFirstPastRJ = (pItem->fg.jointype & JT_दक्षिणः)!=0;
    }अन्यथा यदि( pItem->fg.fromExists ){
      /* joins that result from the EXISTS-to-JOIN optimization should not
      ** be moved to the left of any of their dependencies */
      WhereClause *pWC = &pWInfo->sWC;
      Where_पदांशः *p_पदांशः;
      पूर्णाङ्क i;
      प्रति(i=pWC->n_आधारः, p_पदांशः=pWC->a; i>0; i--, p_पदांशः++){
        यदि( (pNew->आवरणम्_Self & p_पदांशः->prereqAll)!=0 ){
          mPrereq |= (p_पदांशः->prereqAll & (pNew->आवरणम्_Self-1));
        }
      }
    }अन्यथा यदि( !has_दक्षिणः_Cross_Join ){
      mPrereq = 0;
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
    यदि( IsVirtual(pItem->pSTab) ){
      SrcItem *p;
      प्रति(p=&pItem[1]; p<p_अन्तः; p++){
        यदि( (p->fg.jointype & (JT_OUTER|JT_CROSS)) ){
          mUnusable |= sqlite3_Where_Get_आवरणम्(&pWInfo->s_आवरणम्_समुच्चयः, p->iCursor);
        }
      }
      rc = where_Loop_सङ्कलय_Virtual(pBuilder, mPrereq, mUnusable);
    }अन्यथा
#यदि_अन्तः /* SQLITE_OMIT_VIRTUALTABLE */
    {
      rc = where_Loop_सङ्कलय_Btree(pBuilder, mPrereq);
    }
    यदि( rc==SQLITE_OK && pBuilder->pWC->hasOr ){
      rc = where_Loop_सङ्कलय_Or(pBuilder, mPrereq, mUnusable);
    }
    m_प्राक् |= pNew->आवरणम्_Self;
    यदि( rc || db->mallocFailed ){
      यदि( rc==SQLITE_समाप्तम् ){
        /* We hit the query planner search limit set by iPlanLimit */
        sqlite3_log(SQLITE_WARNING, "abbreviated query algorithm search");
        rc = SQLITE_OK;
      }अन्यथा{
        भङ्ग;
      }
    }
  }

  whereLoopClear(db, pNew);
  प्रत्यागम rc;
}

/* Implementation of the order-by-subquery optimization:
**
** WhereLoop pLoop, which the iLoop-th term of the nested loop, is really
** a subquery or CTE that has an ORDER BY clause.  See if any of the terms
** in the subquery ORDER BY clause will satisfy pOrderBy from the outer
** query.  Mark off all satisfied terms (by setting bits in *pOBSat) and
** return TRUE if they do.  If not, return false.
**
** Example:
**
**    CREATE TABLE t1(a,b,c, PRIMARY KEY(a,b));
**    CREATE TABLE t2(x,y);
**    WITH t3(p,q) AS MATERIALIZED (SELECT x+y, x-y FROM t2 ORDER BY x+y)
**       SELECT * FROM t3 JOIN t1 ON a=q ORDER BY p, b;
**
** The CTE named "t3" comes out in the natural order of "p", so the first
** first them of "ORDER BY p,b" is satisfied by a sequential scan of "t3"
** and sorting only needs to occur on the second term "b".
**
** Limitations:
**
** (1)  The optimization is not applied if the outer ORDER BY contains
**      a COLLATE clause.  The optimization might be applied if the
**      outer ORDER BY uses NULLS FIRST, NULLS LAST, ASC, and/or DESC as
**      long as the subquery ORDER BY does the same.  But if the
**      outer ORDER BY uses COLLATE, even a redundant COLLATE, the
**      optimization is bypassed.
**
** (2)  The subquery ORDER BY terms must exactly match subquery result
**      columns, including any COLLATE annotations.  This routine relies
**      on iOrderByCol to do matching between order by terms and result
**      columns, and iOrderByCol will not be set if the result column
**      and ORDER BY collations differ.
**
** (3)  The subquery and outer ORDER BY can be in opposite directions as
**      long as  the subquery is materialized.  If the subquery is
**      implemented as a co-routine, the sort orders must be in the same
**      direction because there is no way to run a co-routine backwards.
*/
स्थिर SQLITE_NOINLINE पूर्णाङ्क where_मार्गः_मेलनम्_Subquery_OB(
  WhereLoop *pLoop,       /* The nested loop term that is a subquery */
  पूर्णाङ्क iLoop,              /* Which level of the nested loop.  0==outermost */
  पूर्णाङ्क iCur,               /* Cursor used by the this loop */
  व्यञ्जकम्_List *p_क्रमः_By,     /* The ORDER BY clause on the whole query */
  Bitmask *p_Rev_आवरणम्,      /* When loops need to go in reverse order */
  Bitmask *pOBSat,        /* Which terms of pOrderBy are satisfied so far */
  u16 wctrlFlags          /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
){
  पूर्णाङ्क iOB;                /* Index into pOrderBy->a[] */
  पूर्णाङ्क jSub;               /* Index into pSubOB->a[] */
  u8 rev = 0;             /* True if iOB and jSub sort in opposite directions */
  u8 revIdx = 0;          /* Sort direction for jSub */
  व्यञ्जकम् *pOBExpr;          /* Current term of outer ORDER BY */
  व्यञ्जकम्_List *pSubOB;       /* Complete ORDER BY on the subquery */

  pSubOB = pLoop->u.btree.p_क्रमः_By;
  प्रतिज्ञा( pSubOB!=0 );
  प्रति(iOB=0; (MASKBIT(iOB) & *pOBSat)!=0; iOB++){}
  प्रति(jSub=0; jSub<pSubOB->n_व्यञ्जकम् && iOB<p_क्रमः_By->n_व्यञ्जकम्; jSub++, iOB++){
    यदि( pSubOB->a[jSub].u.x.i_क्रमः_By_Col==0 ) भङ्ग;
    pOBExpr = p_क्रमः_By->a[iOB].p_व्यञ्जकम्;
    यदि( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) भङ्ग;
    यदि( pOBExpr->i_सारणी!=iCur ) भङ्ग;
    यदि( pOBExpr->iColumn!=pSubOB->a[jSub].u.x.i_क्रमः_By_Col-1 ) भङ्ग;
    यदि( (wctrlFlags & WHERE_GROUPBY)==0 ){
      u8 sfOB = p_क्रमः_By->a[iOB].fg.क्रमय_Flags;   /* sortFlags for iOB */
      u8 sfSub = pSubOB->a[jSub].fg.क्रमय_Flags;   /* sortFlags for jSub */
      यदि( (sfSub & KEYINFO_क्रमः_BIGNULL) != (sfOB & KEYINFO_क्रमः_BIGNULL) ){
        भङ्ग;
      }
      revIdx = sfSub & KEYINFO_क्रमः_DESC;
      यदि( jSub>0 ){
        यदि( (rev^revIdx)!=(sfOB & KEYINFO_क्रमः_DESC) ){
          भङ्ग;
        }
      }अन्यथा{
        rev = revIdx ^ (sfOB & KEYINFO_क्रमः_DESC);
        यदि( rev ){
          यदि( (pLoop->wsFlags & WHERE_COROUTINE)!=0 ){
            /* Cannot run a co-routine in reverse order */
            भङ्ग;
          }
          *p_Rev_आवरणम् |= MASKBIT(iLoop);
        }
      }
    }
    *pOBSat |= MASKBIT(iOB);
  }
  प्रत्यागम jSub>0;
}

/*
** Examine a WherePath (with the addition of the extra WhereLoop of the 6th
** parameters) to see if it outputs rows in the requested ORDER BY
** (or GROUP BY) without requiring a separate sort operation.  Return N:
**
**   N>0:   N terms of the ORDER BY clause are satisfied
**   N==0:  No terms of the ORDER BY clause are satisfied
**   N<0:   Unknown yet how many terms of ORDER BY might be satisfied.  
**
** Note that processing for WHERE_GROUPBY and WHERE_DISTINCTBY is not as
** strict.  With GROUP BY and DISTINCT the only requirement is that
** equivalent rows appear immediately adjacent to one another.  GROUP BY
** and DISTINCT do not require rows to appear in any particular order as long
** as equivalent rows are grouped together.  Thus for GROUP BY and DISTINCT
** the pOrderBy terms can be matched in any order.  With ORDER BY, the
** pOrderBy terms must be matched in strict left-to-right order.
*/
स्थिर i8 where_मार्गः_Satisfies_क्रमः_By(
  WhereInfo *pWInfo,    /* The WHERE clause */
  व्यञ्जकम्_List *p_क्रमः_By,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
  Where_मार्गः *p_मार्गः,     /* The WherePath to check */
  u16 wctrlFlags,       /* WHERE_GROUPBY or _DISTINCTBY or _ORDERBY_LIMIT */
  u16 nLoop,            /* Number of entries in pPath->aLoop[] */
  WhereLoop *p_चरमः,     /* Add this WhereLoop to the end of pPath->aLoop[] */
  Bitmask *p_Rev_आवरणम्     /* OUT: Mask of WhereLoops to run in reverse order */
){
  u8 rev_समुच्चयः;            /* True if rev is known */
  u8 rev;               /* Composite sort order */
  u8 revIdx;            /* Index sort order */
  u8 is_क्रमः_Distinct;   /* All prior WhereLoops are order-distinct */
  u8 distinctColumns;   /* True if the loop has UNIQUE NOT NULL columns */
  u8 is_मेलनम्;           /* iColumn matches a term of the ORDER BY clause */
  u16 eq_Op_आवरणम्;         /* Allowed equality operators */
  u16 n_कुञ्चिका_Col;          /* Number of key columns in pIndex */
  u16 nColumn;          /* Total number of ordered columns in the index */
  u16 n_क्रमः_By;         /* Number terms in the ORDER BY clause */
  पूर्णाङ्क iLoop;            /* Index of WhereLoop in pPath being processed */
  पूर्णाङ्क i, j;             /* Loop counters */
  पूर्णाङ्क iCur;             /* Cursor number for current WhereLoop */
  पूर्णाङ्क iColumn;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  Where_पदांशः *p_पदांशः;     /* A single term of the WHERE clause */
  व्यञ्जकम् *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  सूचकांक *p_सूचकांक;        /* The index associated with pLoop */
  sqlite3 *db = pWInfo->pParse->db;  /* Database connection */
  Bitmask obSat = 0;    /* Mask of ORDER BY terms satisfied so far */
  Bitmask ob_समाप्तम्;       /* Mask of all ORDER BY terms */
  Bitmask क्रमः_Distinct_आवरणम्;  /* Mask of all well-ordered loops */
  Bitmask ready;              /* Mask of inner loops */

  /*
  ** We say the WhereLoop is "one-row" if it generates no more than one
  ** row of output.  A WhereLoop is one-row if all of the following are true:
  **  (a) All index columns match with WHERE_COLUMN_EQ.
  **  (b) The index is unique
  ** Any WhereLoop with an WHERE_COLUMN_EQ constraint on the rowid is one-row.
  ** Every one-row WhereLoop will have the WHERE_ONEROW bit set in wsFlags.
  **
  ** We say the WhereLoop is "order-distinct" if the set of columns from
  ** that WhereLoop that are in the ORDER BY clause are different for every
  ** row of the WhereLoop.  Every one-row WhereLoop is automatically
  ** order-distinct.   A WhereLoop that has no columns in the ORDER BY clause
  ** is not order-distinct. To be order-distinct is not quite the same as being
  ** UNIQUE since a UNIQUE column or index can have multiple rows that
  ** are NULL and NULL values are equivalent for the purpose of order-distinct.
  ** To be order-distinct, the columns must be UNIQUE and NOT NULL.
  **
  ** The rowid for a table is always UNIQUE and NOT NULL so whenever the
  ** rowid appears in the ORDER BY clause, the corresponding WhereLoop is
  ** automatically order-distinct.
  */

  प्रतिज्ञा( p_क्रमः_By!=0 );
  यदि( nLoop && OptimizationDisabled(db, SQLITE_क्रमः_By_Idx_Join) ) प्रत्यागम 0;

  n_क्रमः_By = p_क्रमः_By->n_व्यञ्जकम्;
  testcase( n_क्रमः_By==BMS-1 );
  यदि( n_क्रमः_By>BMS-1 ) प्रत्यागम 0;  /* Cannot optimize overly large ORDER BYs */
  is_क्रमः_Distinct = 1;
  ob_समाप्तम् = MASKBIT(n_क्रमः_By)-1;
  क्रमः_Distinct_आवरणम् = 0;
  ready = 0;
  eq_Op_आवरणम् = WO_EQ | WO_IS | WO_ISNULL;
  यदि( wctrlFlags & (WHERE_ORDERBY_सीमा|WHERE_ORDERBY_MAX|WHERE_ORDERBY_MIN) ){
    eq_Op_आवरणम् |= WO_अन्तर्;
  }
  प्रति(iLoop=0; is_क्रमः_Distinct && obSat<ob_समाप्तम् && iLoop<=nLoop; iLoop++){
    यदि( iLoop>0 ) ready |= pLoop->आवरणम्_Self;
    यदि( iLoop<nLoop ){
      pLoop = p_मार्गः->aLoop[iLoop];
      यदि( wctrlFlags & WHERE_ORDERBY_सीमा ) अनुवर्त;
    }अन्यथा{
      pLoop = p_चरमः;
    }
    यदि( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      यदि( pLoop->u.vtab.isOrdered && pWInfo->p_क्रमः_By==p_क्रमः_By ){
        obSat = ob_समाप्तम्;
      }अन्यथा{
        /* No further ORDER BY terms may be matched. So this call should
        ** return >=0, not -1. Clear isOrderDistinct to ensure it does so. */
        is_क्रमः_Distinct = 0;
      }
      भङ्ग;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    प्रति(i=0; i<n_क्रमः_By; i++){
      यदि( MASKBIT(i) & obSat ) अनुवर्त;
      pOBExpr = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p_क्रमः_By->a[i].p_व्यञ्जकम्);
      यदि( NEVER(pOBExpr==0) ) अनुवर्त;
      यदि( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) अनुवर्त;
      यदि( pOBExpr->i_सारणी!=iCur ) अनुवर्त;
      p_पदांशः = sqlite3_Where_Find_पदांशः(&pWInfo->sWC, iCur, pOBExpr->iColumn,
                       ~ready, eq_Op_आवरणम्, 0);
      यदि( p_पदांशः==0 ) अनुवर्त;
      यदि( p_पदांशः->eOperator==WO_अन्तर् ){
        /* IN terms are only valid for sorting in the ORDER BY LIMIT
        ** optimization, and then only if they are actually used
        ** by the query plan */
        प्रतिज्ञा( wctrlFlags &
               (WHERE_ORDERBY_सीमा|WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX) );
        प्रति(j=0; j<pLoop->nLTerm && p_पदांशः!=pLoop->aLTerm[j]; j++){}
        यदि( j>=pLoop->nLTerm ) अनुवर्त;
      }
      यदि( (p_पदांशः->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->iColumn>=0 ){
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3_व्यञ्जकम्_NNColl_Seq(pParse, p_क्रमः_By->a[i].p_व्यञ्जकम्);
        ध्रुव व्यञ्जकम् *pCExpr = p_पदांशः->p_व्यञ्जकम्;
        CollSeq *pColl2 = sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pParse, pCExpr);
        वर्ण aff_दक्षिणः;
        प्रतिज्ञा( pColl1 );
        यदि( pColl2==0 || sqlite3StrICmp(pColl1->zName, pColl2->zName) ){
          अनुवर्त;
        }
        aff_दक्षिणः = sqlite3_व्यञ्जकम्_Affinity(pCExpr->p_दक्षिणः);
        /* Left operand must be a column, hence always has affinity */
        प्रतिज्ञा( sqlite3_व्यञ्जकम्_Affinity(pCExpr->p_वामः)!=0 );
        यदि( aff_दक्षिणः!=0 && aff_दक्षिणः!=sqlite3_व्यञ्जकम्_Affinity(pCExpr->p_वामः) ){
          अनुवर्त;  /* An affinity conversion would be required */
        }
        testcase( pCExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    यदि( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      यदि( pLoop->wsFlags & WHERE_IPK ){
        यदि( pLoop->u.btree.p_क्रमः_By
         && OptimizationEnabled(db, SQLITE_क्रमः_By_Subq)
         &&  where_मार्गः_मेलनम्_Subquery_OB(pLoop,iLoop,iCur,
                                     p_क्रमः_By,p_Rev_आवरणम्, &obSat, wctrlFlags)
        ){
          nColumn = 0;
          is_क्रमः_Distinct = 0;
        }अन्यथा{
          nColumn = 1;
        }
        p_सूचकांक = 0;
        n_कुञ्चिका_Col = 1;
      }अन्यथा यदि( (p_सूचकांक = pLoop->u.btree.p_सूचकांक)==0 || p_सूचकांक->bUnordered ){
        प्रत्यागम 0;
      }अन्यथा{
        n_कुञ्चिका_Col = p_सूचकांक->n_कुञ्चिका_Col;
        nColumn = p_सूचकांक->nColumn;
        प्रतिज्ञा( nColumn==n_कुञ्चिका_Col+1 || !HasRowid(p_सूचकांक->p_सारणी) );
        प्रतिज्ञा( p_सूचकांक->aiColumn[nColumn-1]==XN_ROWID
                          || !HasRowid(p_सूचकांक->p_सारणी));
        /* All relevant terms of the index must also be non-NULL in order
        ** for isOrderDistinct to be true.  So the isOrderDistinct value
        ** computed here might be a false positive.  Corrections will be
        ** made at tag-20210426-1 below */
        is_क्रमः_Distinct = Is_Unique_सूचकांक(p_सूचकांक)
                          && (pLoop->wsFlags & WHERE_SKIPSCAN)==0;
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = rev_समुच्चयः = 0;
      distinctColumns = 0;
      प्रति(j=0; j<nColumn; j++){
        u8 bOnce = 1; /* True to run the ORDER BY search loop */

        प्रतिज्ञा( j>=pLoop->u.btree.nEq
            || (pLoop->aLTerm[j]==0)==(j<pLoop->nSkip)
        );
        यदि( j<pLoop->u.btree.nEq && j>=pLoop->nSkip ){
          u16 eOp = pLoop->aLTerm[j]->eOperator;

          /* Skip over == and IS and ISNULL terms.  (Also skip IN terms when
          ** doing WHERE_ORDERBY_LIMIT processing).  Except, IS and ISNULL
          ** terms imply that the index is not UNIQUE NOT NULL in which case
          ** the loop need to be marked as not order-distinct because it can
          ** have repeated NULL rows.
          **
          ** If the current term is a column of an ((?,?) IN (SELECT...))
          ** expression for which the SELECT returns more than one column,
          ** check that it is the only column used by this loop. Otherwise,
          ** if it is one of two or more, none of the columns can be
          ** considered to match an ORDER BY term.
          */
          यदि( (eOp & eq_Op_आवरणम्)!=0 ){
            यदि( eOp & (WO_ISNULL|WO_IS) ){
              testcase( eOp & WO_ISNULL );
              testcase( eOp & WO_IS );
              testcase( is_क्रमः_Distinct );
              is_क्रमः_Distinct = 0;
            }
            अनुवर्त; 
          }अन्यथा यदि( ALWAYS(eOp & WO_अन्तर्) ){
            /* ALWAYS() justification: eOp is an equality operator due to the
            ** j<pLoop->u.btree.nEq constraint above.  Any equality other
            ** than WO_IN is captured by the previous "if".  So this one
            ** always has to be WO_IN. */
            व्यञ्जकम् *pX = pLoop->aLTerm[j]->p_व्यञ्जकम्;
            प्रति(i=j+1; i<pLoop->u.btree.nEq; i++){
              यदि( pLoop->aLTerm[i]->p_व्यञ्जकम्==pX ){
                प्रतिज्ञा( (pLoop->aLTerm[i]->eOperator & WO_अन्तर्) );
                bOnce = 0;
                भङ्ग;
              }
            }
          }
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        यदि( p_सूचकांक ){
          iColumn = p_सूचकांक->aiColumn[j];
          revIdx = p_सूचकांक->a_क्रमय_क्रमः[j] & KEYINFO_क्रमः_DESC;
          यदि( iColumn==p_सूचकांक->p_सारणी->iPKey ) iColumn = XN_ROWID;
        }अन्यथा{
          iColumn = XN_ROWID;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered.  tag-20210426-1
        */
        यदि( is_क्रमः_Distinct ){
          यदि( iColumn>=0
           && j>=pLoop->u.btree.nEq
           && p_सूचकांक->p_सारणी->aCol[iColumn].notNull==0
          ){
            is_क्रमः_Distinct = 0;
          }
          यदि( iColumn==XN_व्यञ्जकम् ){
            is_क्रमः_Distinct = 0;
          }
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term having been satisfied.
        */
        is_मेलनम् = 0;
        प्रति(i=0; bOnce && i<n_क्रमः_By; i++){
          यदि( MASKBIT(i) & obSat ) अनुवर्त;
          pOBExpr = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p_क्रमः_By->a[i].p_व्यञ्जकम्);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          यदि( NEVER(pOBExpr==0) ) अनुवर्त;
          यदि( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          यदि( iColumn>=XN_ROWID ){
            यदि( pOBExpr->op!=TK_COLUMN && pOBExpr->op!=TK_AGG_COLUMN ) अनुवर्त;
            यदि( pOBExpr->i_सारणी!=iCur ) अनुवर्त;
            यदि( pOBExpr->iColumn!=iColumn ) अनुवर्त;
          }अन्यथा{
            व्यञ्जकम् *p_Ix_व्यञ्जकम् = p_सूचकांक->a_Col_व्यञ्जकम्->a[j].p_व्यञ्जकम्;
            यदि( sqlite3_व्यञ्जकम्_Compare_Skip(pOBExpr, p_Ix_व्यञ्जकम्, iCur) ){
              अनुवर्त;
            }
          }
          यदि( iColumn!=XN_ROWID ){
            pColl = sqlite3_व्यञ्जकम्_NNColl_Seq(pWInfo->pParse, p_क्रमः_By->a[i].p_व्यञ्जकम्);
            यदि( sqlite3StrICmp(pColl->zName, p_सूचकांक->azColl[j])!=0 ) अनुवर्त;
          }
          यदि( wctrlFlags & WHERE_DISTINCTBY ){
            pLoop->u.btree.nDistinctCol = j+1;
          }
          is_मेलनम् = 1;
          भङ्ग;
        }
        यदि( is_मेलनम् && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          यदि( rev_समुच्चयः ){
            यदि( (rev ^ revIdx)
                           != (p_क्रमः_By->a[i].fg.क्रमय_Flags&KEYINFO_क्रमः_DESC)
            ){
              is_मेलनम् = 0;
            }
          }अन्यथा{
            rev = revIdx ^ (p_क्रमः_By->a[i].fg.क्रमय_Flags & KEYINFO_क्रमः_DESC);
            यदि( rev ) *p_Rev_आवरणम् |= MASKBIT(iLoop);
            rev_समुच्चयः = 1;
          }
        }
        यदि( is_मेलनम् && (p_क्रमः_By->a[i].fg.क्रमय_Flags & KEYINFO_क्रमः_BIGNULL) ){
          यदि( j==pLoop->u.btree.nEq 
           && (j<n_कुञ्चिका_Col||HasRowid(p_सूचकांक->p_सारणी)||!Is_Primary_कुञ्चिका_सूचकांक(p_सूचकांक))
          ){
            /* If this is the PK of a WITHOUT ROWID table, do not use any
            ** field past nKeyCol for a NULLS LAST sort. If ALTER TABLE has
            ** been used to add columns to the table, this field may not
            ** be present in all records, which causes problems for the seeks
            ** used by NULLS LAST sorts.  */
            pLoop->wsFlags |= WHERE_BIGNULL_क्रमय;
          }अन्यथा{
            is_मेलनम् = 0;
          }
        }
        यदि( is_मेलनम् ){
          यदि( iColumn==XN_ROWID ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }अन्यथा{
          /* No match found */
          प्रतिज्ञा( n_कुञ्चिका_Col>0 );
          यदि( j<n_कुञ्चिका_Col ){
            testcase( is_क्रमः_Distinct!=0 );
            is_क्रमः_Distinct = 0;
          }
          भङ्ग;
        }
      } /* end Loop over all index columns */
      यदि( distinctColumns ){
        testcase( is_क्रमः_Distinct==0 );
        is_क्रमः_Distinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    यदि( is_क्रमः_Distinct ){
      क्रमः_Distinct_आवरणम् |= pLoop->आवरणम्_Self;
      प्रति(i=0; i<n_क्रमः_By; i++){
        व्यञ्जकम् *p;
        Bitmask m_पदांशः;
        यदि( MASKBIT(i) & obSat ) अनुवर्त;
        p = p_क्रमः_By->a[i].p_व्यञ्जकम्;
        m_पदांशः = sqlite3_Where_व्यञ्जकम्_Usage(&pWInfo->s_आवरणम्_समुच्चयः,p);
        यदि( m_पदांशः==0 && !sqlite3_व्यञ्जकम्_Is_Constant(0,p) ) अनुवर्त;
        यदि( (m_पदांशः&~क्रमः_Distinct_आवरणम्)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  यदि( obSat==ob_समाप्तम् ) प्रत्यागम (i8)n_क्रमः_By;
  यदि( !is_क्रमः_Distinct ){
    प्रति(i=n_क्रमः_By-1; i>0; i--){
      Bitmask m = ALWAYS(i<BMS) ? MASKBIT(i) - 1 : 0;
      यदि( (obSat&m)==m ) प्रत्यागम i;
    }
    प्रत्यागम 0;
  }
  प्रत्यागम -1;
}


/*
** If the WHERE_GROUPBY flag is set in the mask passed to sqlite3WhereBegin(),
** the planner assumes that the specified pOrderBy list is actually a GROUP
** BY clause - and so any order that groups rows as required satisfies the
** request.
**
** Normally, in this case it is not possible for the caller to determine
** whether or not the rows are really being delivered in sorted order, or
** just in some other order that provides the required grouping. However,
** if the WHERE_SORTBYGROUP flag is also passed to sqlite3WhereBegin(), then
** this function may be called on the returned WhereInfo object. It returns
** true if the rows really will be sorted in the specified order, or false
** otherwise.
**
** For example, assuming:
**
**   CREATE INDEX i1 ON t1(x, Y);
**
** then
**
**   SELECT * FROM t1 GROUP BY x,y ORDER BY x,y;   -- IsSorted()==1
**   SELECT * FROM t1 GROUP BY y,x ORDER BY y,x;   -- IsSorted()==0
*/
पूर्णाङ्क sqlite3WhereIsSorted(WhereInfo *pWInfo){
  प्रतिज्ञा( pWInfo->wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY) );
  प्रतिज्ञा( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  प्रत्यागम pWInfo->sorted;
}

#यदि_परिभाषितम् WHERETRACE_ENABLED
/* For debugging use only: */
स्थिर ध्रुव वर्ण *where_मार्गः_Name(Where_मार्गः *p_मार्गः, पूर्णाङ्क nLoop, WhereLoop *p_चरमः){
  स्थिर वर्ण zName[65];
  पूर्णाङ्क i;
  प्रति(i=0; i<nLoop; i++){ zName[i] = p_मार्गः->aLoop[i]->cId; }
  यदि( p_चरमः ) zName[i++] = p_चरमः->cId;
  zName[i] = 0;
  प्रत्यागम zName;
}
#यदि_अन्तः

/*
** Return the cost of sorting nRow rows, assuming that the keys have
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
स्थिर LogEst where_क्रमणम्_Cost(
  WhereInfo *pWInfo, /* Query planning context */
  LogEst nRow,       /* Estimated number of rows to sort */
  पूर्णाङ्क n_क्रमः_By,      /* Number of ORDER BY clause terms */
  पूर्णाङ्क nSorted        /* Number of initial ORDER BY terms naturally in order */
){
  /* Estimated cost of a full external sort, where N is
  ** the number of rows to sort is:
  **
  **   cost = (K * N * log(N)).
  **
  ** Or, if the order-by clause has X terms but only the last Y
  ** terms are out of order, then block-sorting will reduce the
  ** sorting cost to:
  **
  **   cost = (K * N * log(N)) * (Y/X)
  **
  ** The constant K is at least 2.0 but will be larger if there are a
  ** large number of columns to be sorted, as the sorting time is
  ** proportional to the amount of content to be sorted.  The algorithm
  ** does not currently distinguish between fat columns (BLOBs and TEXTs)
  ** and skinny columns (INTs).  It just uses the number of columns as
  ** an approximation for the row width.
  **
  ** And extra factor of 2.0 or 3.0 is added to the sorting cost if the sort
  ** is built using OP_IdxInsert and OP_Sort rather than with OP_SorterInsert.
  */
  LogEst r_क्रमय_Cost, nCol;
  प्रतिज्ञा( pWInfo->pSelect!=0 );
  प्रतिज्ञा( pWInfo->pSelect->pEList!=0 );
  /* TUNING: sorting cost proportional to the number of output columns: */
  nCol = sqlite3LogEst((pWInfo->pSelect->pEList->n_व्यञ्जकम्+59)/30);
  r_क्रमय_Cost = nRow + nCol;
  यदि( nSorted>0 ){
    /* Scale the result by (Y/X) */
    r_क्रमय_Cost += sqlite3LogEst((n_क्रमः_By-nSorted)*100/n_क्रमः_By) - 66;
  }

  /* Multiple by log(M) where M is the number of output rows.
  ** Use the LIMIT for M if it is smaller.  Or if this sort is for
  ** a DISTINCT operator, M will be the number of distinct output
  ** rows, so fudge it downwards a bit.
  */
  यदि( (pWInfo->wctrlFlags & WHERE_USE_सीमा)!=0 ){
    r_क्रमय_Cost += 10;       /* TUNING: Extra 2.0x if using LIMIT */
    यदि( nSorted!=0 ){
      r_क्रमय_Cost += 6;      /* TUNING: Extra 1.5x if also using partial sort */
    }
    यदि( pWInfo->i_सीमा<nRow ){
      nRow = pWInfo->i_सीमा;
    }
  }अन्यथा यदि( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT) ){
    /* TUNING: In the sort for a DISTINCT operator, assume that the DISTINCT
    ** reduces the number of output rows by a factor of 2 */
    यदि( nRow>10 ){ nRow -= 10;  प्रतिज्ञा( 10==sqlite3LogEst(2) ); }
  }
  r_क्रमय_Cost += estLog(nRow);
  प्रत्यागम r_क्रमय_Cost;
}

/*
** Compute the maximum number of paths in the solver algorithm, for
** queries that have three or more terms in the FROM clause.  Queries with
** two or fewer FROM clause terms are handled by the caller.
**
** Query planning is NP-hard.  We must limit the number of paths at
** each step of the solver search algorithm to avoid exponential behavior.
**
** The value returned is a tuning parameter.  Currently the value is:
**
**     18    for star queries
**     12    otherwise
**
** For the purposes of this heuristic, a star-query is defined as a query
** with a central "fact" table that is joined against multiple
** "dimension" tables, subject to the following constraints:
**
**   (aa)  Only a five-way or larger join is considered for this
**         optimization.  If there are fewer than four terms in the FROM
**         clause, this heuristic does not apply.
**
**   (bb)  The join between the fact table and the dimension tables must
**         be an INNER join.  CROSS and OUTER JOINs do not qualify.
**
**   (cc)  A table must have 3 or more dimension tables in order to be
**         considered a fact table. (Was 4 prior to 2026-02-10.)
**
**   (dd)  A table that is a self-join cannot be a dimension table.
**         Dimension tables are joined against fact tables.
**
** SIDE EFFECT:  (and really the whole point of this subroutine)
**
** If pWInfo describes a star-query, then the cost for SCANs of dimension
** WhereLoops is increased to be slightly larger than the cost of a SCAN
** in the fact table.  Only SCAN costs are increased.  SEARCH costs are
** unchanged. This heuristic helps keep fact tables in outer loops. Without
** this heuristic, paths with fact tables in outer loops tend to get pruned
** by the mxChoice limit on the number of paths, resulting in poor query
** plans.  See the starschema1.test test module for examples of queries
** that need this heuristic to find good query plans.
**
** This heuristic can be completely disabled, so that no query is
** considered a star-query, using SQLITE_TESTCTRL_OPTIMIZATION to
** disable the SQLITE_StarQuery optimization.  In the CLI, the command
** to do that is:  ".testctrl opt -starquery".
**
** HISTORICAL NOTES:
**
** This optimization was first added on 2024-05-09 by check-in 38db9b5c83d.
** The original optimization reduced the cost and output size estimate for
** fact tables to help them move to outer loops.  But months later (as people
** started upgrading) performance regression reports started caming in,
** including:
**
**    forum post b18ef983e68d06d1 (2024-12-21)
**    forum post 0025389d0860af82 (2025-01-14)
**    forum post d87570a145599033 (2025-01-17)
**
** To address these, the criteria for a star-query was tightened to exclude
** cases where the fact and dimensions are separated by an outer join, and
** the affect of star-schema detection was changed to increase the rRun cost
** on just full table scans of dimension tables, rather than reducing costs
** in the all access methods of the fact table.
*/
स्थिर पूर्णाङ्क computeMxChoice(WhereInfo *pWInfo){
  पूर्णाङ्क nLoop = pWInfo->nLevel;    /* Number of terms in the join */
  WhereLoop *pWLoop;             /* For looping over WhereLoops */

#यदि_परिभाषितम् SQLITE_DEBUG
  /* The star-query detection code below makes use of the following
  ** properties of the WhereLoop list, so verify them before
  ** continuing:
  **    (1)  .maskSelf is the bitmask corresponding to .iTab
  **    (2)  The WhereLoop list is in ascending .iTab order
  */
  प्रति(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
    प्रतिज्ञा( pWLoop->आवरणम्_Self==MASKBIT(pWLoop->iTab) );
    प्रतिज्ञा( pWLoop->p_अग्र_Loop==0 || pWLoop->iTab<=pWLoop->p_अग्र_Loop->iTab );
  }
#यदि_अन्तः /* SQLITE_DEBUG */

  यदि( nLoop>=4  /* Constraint (aa) */
   && !pWInfo->b_तारकम्_समाप्तम्
   && OptimizationEnabled(pWInfo->pParse->db, SQLITE_तारकम्_Query)
  ){
    SrcItem *aFromTabs;    /* All terms of the FROM clause */
    पूर्णाङ्क iFromIdx;          /* Term of FROM clause is the candidate fact-table */
    Bitmask m;             /* Bitmask for candidate fact-table */
    Bitmask mSelfJoin = 0; /* Tables that cannot be dimension tables */       
    WhereLoop *pStart;     /* Where to start searching for dimension-tables */

    pWInfo->b_तारकम्_समाप्तम् = 1; /* Only do this computation once */

    /* Look for fact tables with three or more dimensions where the
    ** dimension tables are not separately from the fact tables by an outer
    ** or cross join.  Adjust cost weights if found.
    */
    प्रतिज्ञा( !pWInfo->b_तारकम्_प्रयुक्तम् );
    aFromTabs = pWInfo->pTabList->a;
    pStart = pWInfo->pLoops;
    प्रति(iFromIdx=0, m=1; iFromIdx<nLoop; iFromIdx++, m<<=1){
      पूर्णाङ्क nDep = 0;             /* Number of dimension tables */
      LogEst mx_सततम्;             /* Maximum SCAN cost of a fact table */
      Bitmask m_आलोकितम् = 0;        /* Mask of dimension tables */
      SrcItem *pFactTab;        /* The candidate fact table */

      pFactTab = aFromTabs + iFromIdx;
      यदि( (pFactTab->fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
        /* If the candidate fact-table is the right table of an outer join
        ** restrict the search for dimension-tables to be tables to the right
        ** of the fact-table.  Constraint (bb) */
        यदि( iFromIdx+3 > nLoop ){
          भङ्ग;  /* ^-- Impossible to reach nDep>=2 - Constraint (cc) */
        }
        यावत्( pStart && pStart->iTab<=iFromIdx ){
          pStart = pStart->p_अग्र_Loop;
        }
      }
      प्रति(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        यदि( (aFromTabs[pWLoop->iTab].fg.jointype & (JT_OUTER|JT_CROSS))!=0 ){
          भङ्ग; /* Constraint (bb) */
        }
        यदि( (pWLoop->prereq & m)!=0        /* pWInfo depends on iFromIdx */
         && (pWLoop->आवरणम्_Self & m_आलोकितम्)==0  /* pWInfo not already a dependency */
         && (pWLoop->आवरणम्_Self & mSelfJoin)==0 /* Not a self-join */
        ){
          यदि( aFromTabs[pWLoop->iTab].pSTab==pFactTab->pSTab ){
            mSelfJoin |= m;
          }अन्यथा{
            nDep++;
            m_आलोकितम् |= pWLoop->आवरणम्_Self;
          }
        }
      }
      यदि( nDep<=2 ){
        अनुवर्त; /* Constraint (cc) */
      }

      /* If we reach this point, it means that pFactTab is a fact table
      ** with four or more dimensions connected by inner joins.  Proceed
      ** to make cost adjustments. */
     
#यदि_परिभाषितम् WHERETRACE_ENABLED
      /* Make sure rStarDelta values are initialized */
      यदि( !pWInfo->b_तारकम्_प्रयुक्तम् ){
        प्रति(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
          pWLoop->r_तारकम्_Delta = 0;
        }
      }
#यदि_अन्तः
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0x80000 */
      यदि( sqlite3WhereTrace & 0x80000 ){
        Bitmask mShow = m_आलोकितम्;
        sqlite3DebugPrintf("Fact table %s(%d), dimensions:",
            pFactTab->zAlias ? pFactTab->zAlias : pFactTab->pSTab->zName,
            iFromIdx);
        प्रति(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
          यदि( mShow & pWLoop->आवरणम्_Self ){
            SrcItem *pDim = aFromTabs + pWLoop->iTab;
            mShow &= ~pWLoop->आवरणम्_Self;
            sqlite3DebugPrintf(" %s(%d)",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, pWLoop->iTab);
          }
        }
        sqlite3DebugPrintf("\n");
      }
#यदि_अन्तः
      pWInfo->b_तारकम्_प्रयुक्तम् = 1;

      /* Compute the maximum cost of any WhereLoop for the
      ** fact table plus one epsilon */
      mx_सततम् = LOGEST_MIN;
      प्रति(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        यदि( pWLoop->iTab<iFromIdx ) अनुवर्त;
        यदि( pWLoop->iTab>iFromIdx ) भङ्ग;
        यदि( pWLoop->r_सततम्>mx_सततम् ) mx_सततम् = pWLoop->r_सततम्;
      }
      यदि( ALWAYS(mx_सततम्<LOGEST_MAX) ) mx_सततम्++;

      /* Increase the cost of table scans for dimension tables to be
      ** slightly more than the maximum cost of the fact table */
      प्रति(pWLoop=pStart; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        यदि( (pWLoop->आवरणम्_Self & m_आलोकितम्)==0 ) अनुवर्त;
        यदि( pWLoop->nLTerm ) अनुवर्त;
        यदि( pWLoop->r_सततम्<mx_सततम् ){
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0x80000 */
          यदि( sqlite3WhereTrace & 0x80000 ){
            SrcItem *pDim = aFromTabs + pWLoop->iTab;
            sqlite3DebugPrintf(
              "Increase SCAN cost of %s to %d\n",
              pDim->zAlias ? pDim->zAlias: pDim->pSTab->zName, mx_सततम्
            );
          }
          pWLoop->r_तारकम्_Delta = mx_सततम् - pWLoop->r_सततम्;
#यदि_अन्तः /* WHERETRACE_ENABLED */
          pWLoop->r_सततम् = mx_सततम्;
        }
      }
    }
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0x80000 */
    यदि( (sqlite3WhereTrace & 0x80000)!=0 && pWInfo->b_तारकम्_प्रयुक्तम् ){
      sqlite3DebugPrintf("WhereLoops changed by star-query heuristic:\n");
      प्रति(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        यदि( pWLoop->r_तारकम्_Delta ){
          sqlite3WhereLoopPrint(pWLoop, &pWInfo->sWC);
        }
      }
    }
#यदि_अन्तः
  }
  प्रत्यागम pWInfo->b_तारकम्_प्रयुक्तम् ? 18 : 12;
}

/*
** Two WhereLoop objects, pCandidate and pBaseline, are known to have the
** same cost.  Look deep into each to see if pCandidate is even slightly
** better than pBaseline.  Return false if it is, if pCandidate is is preferred.
** Return true if pBaseline is preferred or if we cannot tell the difference.
**
**    Result       Meaning
**    --------     ----------------------------------------------------------
**    true         We cannot tell the difference in pCandidate and pBaseline
**    false        pCandidate seems like a better choice than pBaseline
*/
स्थिर SQLITE_NOINLINE पूर्णाङ्क whereLoopIsNoBetter(
  ध्रुव WhereLoop *pCandidate,
  ध्रुव WhereLoop *pBaseline
){
  यदि( (pCandidate->wsFlags & WHERE_अनुक्रमितम्)==0 ) प्रत्यागम 1;
  यदि( (pBaseline->wsFlags & WHERE_अनुक्रमितम्)==0 ) प्रत्यागम 1;
  यदि( pCandidate->u.btree.p_सूचकांक->szIdxRow <
        pBaseline->u.btree.p_सूचकांक->szIdxRow ) प्रत्यागम 0;
  प्रत्यागम 1;
}

/*
** Given the list of WhereLoop objects at pWInfo->pLoops, this routine
** attempts to find the lowest cost path that visits each WhereLoop
** once.  This path is then loaded into the pWInfo->a[].pWLoop fields.
**
** Assume that the total number of output rows that will need to be sorted
** will be nRowEst (in the 10*log2 representation).  Or, ignore sorting
** costs if nRowEst==0.
**
** Return SQLITE_OK on success or SQLITE_NOMEM of a memory allocation
** error occurs.
*/
स्थिर पूर्णाङ्क where_मार्गः_Solver(WhereInfo *pWInfo, LogEst nRowEst){
  पूर्णाङ्क mxChoice;             /* Maximum number of simultaneous paths tracked */
  पूर्णाङ्क nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  पूर्णाङ्क iLoop;                /* Loop counter over the terms of the join */
  पूर्णाङ्क ii, jj;               /* Loop counters */
  पूर्णाङ्क mxI = 0;              /* Index of next entry to replace */
  पूर्णाङ्क n_क्रमः_By;             /* Number of ORDER BY clause terms */
  LogEst mxCost = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsort = 0;      /* Maximum unsorted cost of a set of path */
  पूर्णाङ्क n_गन्तव्यम्, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  Where_मार्गः *aFrom;         /* All nFrom paths at the previous level */
  Where_मार्गः *a_गन्तव्यम्;           /* The nTo best paths at the current level */
  Where_मार्गः *pFrom;         /* An element of aFrom[] that we are working on */
  Where_मार्गः *p_गन्तव्यम्;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *a_क्रमय_Cost = 0;    /* Sorting and partial sorting costs */
  वर्ण *pSpace;             /* Temporary memory used by this routine */
  पूर्णाङ्क nSpace;               /* Bytes of space allocated at pSpace */

  pParse = pWInfo->pParse;
  nLoop = pWInfo->nLevel;
  WHERETRACE(0x002, ("---- begin solver.  (nRowEst=%d, nQueryLoop=%d)\n",
                     nRowEst, pParse->nQueryLoop));
  /* TUNING: mxChoice is the maximum number of possible paths to preserve
  ** at each step.  Based on the number of loops in the FROM clause:
  **
  **     nLoop      mxChoice
  **     -----      --------
  **       1            1            // the most common case
  **       2            5
  **       3+        12 or 18        // see computeMxChoice()
  */
  यदि( nLoop<=1 ){
    mxChoice = 1;
  }अन्यथा यदि( nLoop==2 ){
    mxChoice = 5;
  }अन्यथा यदि( pParse->nErr ){
    mxChoice = 1;
  }अन्यथा{
    mxChoice = computeMxChoice(pWInfo);
  }
  प्रतिज्ञा( nLoop<=pWInfo->pTabList->nSrc );

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  यदि( pWInfo->p_क्रमः_By==0 || nRowEst==0 ){
    n_क्रमः_By = 0;
  }अन्यथा{
    n_क्रमः_By = pWInfo->p_क्रमः_By->n_व्यञ्जकम्;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (परिमाण(Where_मार्गः)+परिमाण(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += परिमाण(LogEst) * n_क्रमः_By;
  pSpace = sqlite3_राशिः_Alloc_Raw_NN(pParse->db, nSpace);
  यदि( pSpace==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
  a_गन्तव्यम् = (Where_मार्गः*)pSpace;
  aFrom = a_गन्तव्यम्+mxChoice;
  स्मृतिं_पूरय(aFrom, 0, परिमाण(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  प्रति(ii=mxChoice*2, pFrom=a_गन्तव्यम्; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  यदि( n_क्रमः_By ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array
    ** index.  */
    a_क्रमय_Cost = (LogEst*)pX;
    स्मृतिं_पूरय(a_क्रमय_Cost, 0, परिमाण(LogEst) * n_क्रमः_By);
  }
  प्रतिज्ञा( a_क्रमय_Cost==0 || &pSpace[nSpace]==(वर्ण*)&a_क्रमय_Cost[n_क्रमः_By] );
  प्रतिज्ञा( a_क्रमय_Cost!=0 || &pSpace[nSpace]==(वर्ण*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].nRow = MIN(pParse->nQueryLoop, 48);  प्रतिज्ञा( 48==sqlite3LogEst(28) );
  nFrom = 1;
  प्रतिज्ञा( aFrom[0].isOrdered==0 );
  यदि( n_क्रमः_By ){
    /* If nLoop is zero, then there are no FROM terms in the query. Since
    ** in this case the query may return a maximum of one row, the results
    ** are already in the requested order. Set isOrdered to nOrderBy to
    ** indicate this. Or, if nLoop is greater than zero, set isOrdered to
    ** -1, indicating that the result set may or may not be ordered,
    ** depending on the loops added to the current plan.  */
    aFrom[0].isOrdered = nLoop>0 ? -1 : n_क्रमः_By;
  }

  /* Compute successively longer WherePaths using the previous generation
  ** of WherePaths as the basis for the next.  Keep track of the mxChoice
  ** best paths at each generation */
  प्रति(iLoop=0; iLoop<nLoop; iLoop++){
    n_गन्तव्यम् = 0;
    प्रति(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      प्रति(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->p_अग्र_Loop){
        LogEst n_बहिर्गमनम्;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst rCost;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsort;                   /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered;                     /* isOrdered for (pFrom+pWLoop) */
        Bitmask आवरणम्_New;                  /* Mask of src visited by (..) */
        Bitmask rev_आवरणम्;                  /* Mask of rev-order loops for (..) */

        यदि( (pWLoop->prereq & ~pFrom->आवरणम्_Loop)!=0 ) अनुवर्त;
        यदि( (pWLoop->आवरणम्_Self & pFrom->आवरणम्_Loop)!=0 ) अनुवर्त;
        यदि( (pWLoop->wsFlags & WHERE_AUTO_सूचकांक)!=0 && pFrom->nRow<3 ){
          /* Do not use an automatic index if the this loop is expected
          ** to run less than 1.25 times.  It is tempting to also exclude
          ** automatic index usage on an outer loop, but sometimes an automatic
          ** index is useful in the outer loop of a correlated subquery. */
          प्रतिज्ञा( 10==sqlite3LogEst(2) );
          अनुवर्त;
        }

        /* At this point, pWLoop is a candidate to be the next loop.
        ** Compute its cost */
        rUnsort = pWLoop->r_सततम् + pFrom->nRow;
        यदि( pWLoop->r_सज्जता ){
          rUnsort = sqlite3_Log_Est_सङ्कलय(pWLoop->r_सज्जता, rUnsort);
        }
        rUnsort = sqlite3_Log_Est_सङ्कलय(rUnsort, pFrom->rUnsort);
        n_बहिर्गमनम् = pFrom->nRow + pWLoop->n_बहिर्गमनम्;
        आवरणम्_New = pFrom->आवरणम्_Loop | pWLoop->आवरणम्_Self;
        isOrdered = pFrom->isOrdered;
        यदि( isOrdered<0 ){
          rev_आवरणम् = 0;
          isOrdered = where_मार्गः_Satisfies_क्रमः_By(pWInfo,
                       pWInfo->p_क्रमः_By, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &rev_आवरणम्);
        }अन्यथा{
          rev_आवरणम् = pFrom->revLoop;
        }
        यदि( isOrdered>=0 && isOrdered<n_क्रमः_By ){
          यदि( a_क्रमय_Cost[isOrdered]==0 ){
            a_क्रमय_Cost[isOrdered] = where_क्रमणम्_Cost(
                pWInfo, nRowEst, n_क्रमः_By, isOrdered
            );
          }
          /* TUNING:  Add a small extra penalty (3) to sorting as an
          ** extra encouragement to the query planner to select a plan
          ** where the rows emerge in the correct order without any sorting
          ** required. */
          rCost = sqlite3_Log_Est_सङ्कलय(rUnsort, a_क्रमय_Cost[isOrdered]) + 3;

          WHERETRACE(0x002,
              ("---- sort cost=%-3d (%d/%d) increases cost %3d to %-3d\n",
               a_क्रमय_Cost[isOrdered], (n_क्रमः_By-isOrdered), n_क्रमः_By,
               rUnsort, rCost));
        }अन्यथा{
          rCost = rUnsort;
          rUnsort -= 2;  /* TUNING:  Slight bias in favor of no-sort plans */
        }

        /* Check to see if pWLoop should be added to the set of
        ** mxChoice best-so-far paths.
        **
        ** First look for an existing path among best-so-far paths
        ** that:
        **     (1) covers the same set of loops, and
        **     (2) has a compatible isOrdered value.
        **
        ** "Compatible isOrdered value" means either
        **     (A) both have isOrdered==-1, or
        **     (B) both have isOrder>=0, or
        **     (C) ordering does not matter because this is the last round
        **         of the solver.
        **
        ** The term "((pTo->isOrdered^isOrdered)&0x80)==0" is equivalent
        ** to (pTo->isOrdered==(-1))==(isOrdered==(-1))" for the range
        ** of legal values for isOrdered, -1..64.
        */
        testcase( n_गन्तव्यम्==0 );
        प्रति(jj=0, p_गन्तव्यम्=a_गन्तव्यम्; jj<n_गन्तव्यम्; jj++, p_गन्तव्यम्++){
          यदि( p_गन्तव्यम्->आवरणम्_Loop==आवरणम्_New
           && ( ((p_गन्तव्यम्->isOrdered^isOrdered)&0x80)==0 || iLoop==nLoop-1 )
          ){
            testcase( jj==n_गन्तव्यम्-1 );
            भङ्ग;
          }
        }
        यदि( jj>=n_गन्तव्यम् ){
          /* None of the existing best-so-far paths match the candidate. */
          यदि( n_गन्तव्यम्>=mxChoice
           && (rCost>mxCost || (rCost==mxCost && rUnsort>=mxUnsort))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0x4 */
            यदि( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d,%3d order=%c\n",
                  where_मार्गः_Name(pFrom, iLoop, pWLoop), rCost, n_बहिर्गमनम्, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#यदि_अन्तः
            अनुवर्त;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          यदि( n_गन्तव्यम्<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = n_गन्तव्यम्++;
          }अन्यथा{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          p_गन्तव्यम् = &a_गन्तव्यम्[jj];
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0x4 */
          यदि( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d,%3d order=%c\n",
                where_मार्गः_Name(pFrom, iLoop, pWLoop), rCost, n_बहिर्गमनम्, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#यदि_अन्तः
        }अन्यथा{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the same isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped.
          **
          ** The conditional is an expanded vector comparison equivalent to:
          **   (pTo->rCost,pTo->nRow,pTo->rUnsort) <= (rCost,nOut,rUnsort)
          */
          यदि( (p_गन्तव्यम्->rCost<rCost)
           || (p_गन्तव्यम्->rCost==rCost && p_गन्तव्यम्->nRow<n_बहिर्गमनम्)
           || (p_गन्तव्यम्->rCost==rCost && p_गन्तव्यम्->nRow==n_बहिर्गमनम् && p_गन्तव्यम्->rUnsort<rUnsort)
           || (p_गन्तव्यम्->rCost==rCost && p_गन्तव्यम्->nRow==n_बहिर्गमनम् && p_गन्तव्यम्->rUnsort==rUnsort
                  && whereLoopIsNoBetter(pWLoop, p_गन्तव्यम्->aLoop[iLoop]) )
          ){
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0x4 */
            यदि( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d,%3d order=%c",
                  where_मार्गः_Name(pFrom, iLoop, pWLoop), rCost, n_बहिर्गमनम्, rUnsort,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%3d,%3d order=%c\n",
                  where_मार्गः_Name(p_गन्तव्यम्, iLoop+1, 0), p_गन्तव्यम्->rCost, p_गन्तव्यम्->nRow,
                  p_गन्तव्यम्->rUnsort, p_गन्तव्यम्->isOrdered>=0 ? p_गन्तव्यम्->isOrdered+'0' : '?');
            }
#यदि_अन्तः
            /* Discard the candidate path from further consideration */
            testcase( p_गन्तव्यम्->rCost==rCost );
            अनुवर्त;
          }
          testcase( p_गन्तव्यम्->rCost==rCost+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0x4 */
          यदि( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d,%3d order=%c",
                where_मार्गः_Name(pFrom, iLoop, pWLoop), rCost, n_बहिर्गमनम्, rUnsort,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d,%3d order=%c\n",
                where_मार्गः_Name(p_गन्तव्यम्, iLoop+1, 0), p_गन्तव्यम्->rCost, p_गन्तव्यम्->nRow,
                p_गन्तव्यम्->rUnsort, p_गन्तव्यम्->isOrdered>=0 ? p_गन्तव्यम्->isOrdered+'0' : '?');
          }
#यदि_अन्तः
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        p_गन्तव्यम्->आवरणम्_Loop = pFrom->आवरणम्_Loop | pWLoop->आवरणम्_Self;
        p_गन्तव्यम्->revLoop = rev_आवरणम्;
        p_गन्तव्यम्->nRow = n_बहिर्गमनम्;
        p_गन्तव्यम्->rCost = rCost;
        p_गन्तव्यम्->rUnsort = rUnsort;
        p_गन्तव्यम्->isOrdered = isOrdered;
        स्मृतिं_प्रतिलिख(p_गन्तव्यम्->aLoop, pFrom->aLoop, परिमाण(WhereLoop*)*iLoop);
        p_गन्तव्यम्->aLoop[iLoop] = pWLoop;
        यदि( n_गन्तव्यम्>=mxChoice ){
          mxI = 0;
          mxCost = a_गन्तव्यम्[0].rCost;
          mxUnsort = a_गन्तव्यम्[0].nRow;
          प्रति(jj=1, p_गन्तव्यम्=&a_गन्तव्यम्[1]; jj<mxChoice; jj++, p_गन्तव्यम्++){
            यदि( p_गन्तव्यम्->rCost>mxCost
             || (p_गन्तव्यम्->rCost==mxCost && p_गन्तव्यम्->rUnsort>mxUnsort)
            ){
              mxCost = p_गन्तव्यम्->rCost;
              mxUnsort = p_गन्तव्यम्->rUnsort;
              mxI = jj;
            }
          }
        }
      }
    }

#यदि_परिभाषितम् WHERETRACE_ENABLED  /* >=2 */
    यदि( sqlite3WhereTrace & 0x02 ){
      LogEst rMin, rFloor = 0;
      पूर्णाङ्क n_समाप्तम् = 0;
      पूर्णाङ्क n_प्रगतिः;
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      कुरु{
        n_प्रगतिः = 0;
        rMin = 0x7fff;
        प्रति(ii=0, p_गन्तव्यम्=a_गन्तव्यम्; ii<n_गन्तव्यम्; ii++, p_गन्तव्यम्++){
          यदि( p_गन्तव्यम्->rCost>rFloor && p_गन्तव्यम्->rCost<rMin ) rMin = p_गन्तव्यम्->rCost;
        }
        प्रति(ii=0, p_गन्तव्यम्=a_गन्तव्यम्; ii<n_गन्तव्यम्; ii++, p_गन्तव्यम्++){
          यदि( p_गन्तव्यम्->rCost==rMin ){
            sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
               where_मार्गः_Name(p_गन्तव्यम्, iLoop+1, 0), p_गन्तव्यम्->rCost, p_गन्तव्यम्->nRow,
               p_गन्तव्यम्->isOrdered>=0 ? (p_गन्तव्यम्->isOrdered+'0') : '?');
            यदि( p_गन्तव्यम्->isOrdered>0 ){
              sqlite3DebugPrintf(" rev=0x%llx\n", p_गन्तव्यम्->revLoop);
            }अन्यथा{
              sqlite3DebugPrintf("\n");
            }
            n_समाप्तम्++;
            n_प्रगतिः++;
          }
        }
        rFloor = rMin;
      }यावत्( n_समाप्तम्<n_गन्तव्यम् && n_प्रगतिः>0 );
    }
#यदि_अन्तः

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = a_गन्तव्यम्;
    a_गन्तव्यम् = aFrom;
    aFrom = pFrom;
    nFrom = n_गन्तव्यम्;
  }

  यदि( nFrom==0 ){
    sqlite3_Error_सन्देशः(pParse, "no query solution");
    sqlite3_राशिः_Free_NN(pParse->db, pSpace);
    प्रत्यागम SQLITE_ERROR;
  }
 
  /* Only one path is available, which is the best path */
  प्रतिज्ञा( nFrom==1 );
  pFrom = aFrom;

  प्रतिज्ञा( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  प्रति(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  यदि( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && nRowEst
  ){
    Bitmask not_प्रयुक्तम्;
    पूर्णाङ्क rc = where_मार्गः_Satisfies_क्रमः_By(pWInfo, pWInfo->p_परिणाम_समुच्चयः, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &not_प्रयुक्तम्);
    यदि( rc==pWInfo->p_परिणाम_समुच्चयः->n_व्यञ्जकम् ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  यदि( pWInfo->p_क्रमः_By ){
    pWInfo->nOBSat = pFrom->isOrdered;
    यदि( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      यदि( pFrom->isOrdered==pWInfo->p_क्रमः_By->n_व्यञ्जकम् ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
      /* vvv--- See check-in [12ad822d9b827777] on 2023-03-16 ---vvv */
      प्रतिज्ञा( pWInfo->pSelect->p_क्रमः_By==0
           || pWInfo->nOBSat <= pWInfo->pSelect->p_क्रमः_By->n_व्यञ्जकम् );
    }अन्यथा{
      pWInfo->rev_आवरणम् = pFrom->revLoop;
      यदि( pWInfo->nOBSat<=0 ){
        pWInfo->nOBSat = 0;
        यदि( nLoop>0 ){
          u32 wsFlags = pFrom->aLoop[nLoop-1]->wsFlags;
          यदि( (wsFlags & WHERE_ONEROW)==0
           && (wsFlags&(WHERE_IPK|WHERE_COLUMN_अन्तर्))!=(WHERE_IPK|WHERE_COLUMN_अन्तर्)
          ){
            Bitmask m = 0;
            पूर्णाङ्क rc = where_मार्गः_Satisfies_क्रमः_By(pWInfo, pWInfo->p_क्रमः_By, pFrom,
                      WHERE_ORDERBY_सीमा, nLoop-1, pFrom->aLoop[nLoop-1], &m);
            testcase( wsFlags & WHERE_IPK );
            testcase( wsFlags & WHERE_COLUMN_अन्तर् );
            यदि( rc==pWInfo->p_क्रमः_By->n_व्यञ्जकम् ){
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->rev_आवरणम् = m;
            }
          }
        }
      }अन्यथा यदि( nLoop
            && pWInfo->nOBSat==1
            && (pWInfo->wctrlFlags & (WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX))!=0
            ){
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    यदि( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->p_क्रमः_By->n_व्यञ्जकम् && nLoop>0
    ){
      Bitmask rev_आवरणम् = 0;
      पूर्णाङ्क n_क्रमः = where_मार्गः_Satisfies_क्रमः_By(pWInfo, pWInfo->p_क्रमः_By,
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &rev_आवरणम्
      );
      प्रतिज्ञा( pWInfo->sorted==0 );
      यदि( n_क्रमः==pWInfo->p_क्रमः_By->n_व्यञ्जकम् ){
        pWInfo->sorted = 1;
        pWInfo->rev_आवरणम् = rev_आवरणम्;
      }
    }
  }

  pWInfo->n_Row_बहिर्गमनम् = pFrom->nRow;
#यदि_परिभाषितम् WHERETRACE_ENABLED
  pWInfo->r_योग_Cost = pFrom->rCost;
#यदि_अन्तः

  /* Free temporary memory and return success */
  sqlite3_राशिः_Free_NN(pParse->db, pSpace);
  प्रत्यागम SQLITE_OK;
}

/*
** This routine implements a heuristic designed to improve query planning.
** This routine is called in between the first and second call to
** wherePathSolver().  Hence the name "Interstage" "Heuristic".
**
** The first call to wherePathSolver() (hereafter just "solver()") computes
** the best path without regard to the order of the outputs.  The second call
** to the solver() builds upon the first call to try to find an alternative
** path that satisfies the ORDER BY clause.
**
** This routine looks at the results of the first solver() run, and for
** every FROM clause term in the resulting query plan that uses an equality
** constraint against an index, disable other WhereLoops for that same
** FROM clause term that would try to do a full-table scan.  This prevents
** an index search from being converted into a full-table scan in order to
** satisfy an ORDER BY clause, since even though we might get slightly better
** performance using the full-scan without sorting if the output size
** estimates are very precise, we might also get severe performance
** degradation using the full-scan if the output size estimate is too large.
** It is better to err on the side of caution.
**
** Except, if the first solver() call generated a full-table scan in an outer
** loop then stop this analysis at the first full-scan, since the second
** solver() run might try to swap that full-scan for another in order to
** get the output into the correct order.  In other words, we allow a
** rewrite like this:
**
**     First Solver()                      Second Solver()
**       |-- SCAN t1                         |-- SCAN t2
**       |-- SEARCH t2                       `-- SEARCH t1
**       `-- SORT USING B-TREE
**
** The purpose of this routine is to disallow rewrites such as:
**
**     First Solver()                      Second Solver()
**       |-- SEARCH t1                       |-- SCAN t2     <--- bad!
**       |-- SEARCH t2                       `-- SEARCH t1
**       `-- SORT USING B-TREE
**
** See test cases in test/whereN.test for the real-world query that
** originally provoked this heuristic.
*/
स्थिर SQLITE_NOINLINE शून्य whereInterstageHeuristic(WhereInfo *pWInfo){
  पूर्णाङ्क i;
#यदि_परिभाषितम् WHERETRACE_ENABLED
  पूर्णाङ्क once = 0;
#यदि_अन्तः
  प्रति(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *p = pWInfo->a[i].pWLoop;
    यदि( p==0 ) भङ्ग;
    यदि( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      /* Treat a vtab scan as similar to a full-table scan */
      भङ्ग;
    }
    यदि( (p->wsFlags & (WHERE_COLUMN_EQ|WHERE_COLUMN_NULL|WHERE_COLUMN_अन्तर्))!=0 ){
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      प्रति(pLoop=pWInfo->pLoops; pLoop; pLoop=pLoop->p_अग्र_Loop){
        यदि( pLoop->iTab!=iTab ) अनुवर्त;
        यदि( (pLoop->wsFlags & (WHERE_CONSTRAINT|WHERE_AUTO_सूचकांक))!=0 ){
          /* Auto-index and index-constrained loops allowed to remain */
          अनुवर्त;
        }
#यदि_परिभाषितम् WHERETRACE_ENABLED
        यदि( sqlite3WhereTrace & 0x80 ){
          यदि( once==0 ){
            sqlite3DebugPrintf("Loops disabled by interstage heuristic:\n");
            once = 1;
          }
          sqlite3WhereLoopPrint(pLoop, &pWInfo->sWC);
        }
#यदि_अन्तः /* WHERETRACE_ENABLED */
        pLoop->prereq = ALLBITS;  /* Prevent 2nd solver() from using this one */
      }
    }अन्यथा{
      भङ्ग;
    }
  }
}

/*
** Most queries use only a single table (they are not joins) and have
** simple == constraints against indexed fields.  This routine attempts
** to plan those simple cases using much less ceremony than the
** general-purpose query planner, and thereby yield faster sqlite3_prepare()
** times for the common case.
**
** Return non-zero on success, if this query can be handled by this
** no-frills query planner.  Return zero if this query needs the
** general-purpose query planner.
*/
स्थिर पूर्णाङ्क whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  SrcItem *pItem;
  WhereClause *pWC;
  Where_पदांशः *p_पदांशः;
  WhereLoop *pLoop;
  पूर्णाङ्क iCur;
  पूर्णाङ्क j;
  सारणी *pTab;
  सूचकांक *pIdx;
  Where_अवलोकनम् अवलोकनम्;

  pWInfo = pBuilder->pWInfo;
  यदि( pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE ) प्रत्यागम 0;
  प्रतिज्ञा( pWInfo->pTabList->nSrc>=1 );
  pItem = pWInfo->pTabList->a;
  pTab = pItem->pSTab;
  यदि( IsVirtual(pTab) ) प्रत्यागम 0;
  यदि( pItem->fg.is_अनुक्रमितम्_By || pItem->fg.not_अनुक्रमितम् ){
    testcase( pItem->fg.is_अनुक्रमितम्_By );
    testcase( pItem->fg.not_अनुक्रमितम् );
    प्रत्यागम 0;
  }
  iCur = pItem->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  p_पदांशः = where_अवलोकनम्_Init(&अवलोकनम्, pWC, iCur, -1, WO_EQ|WO_IS, 0);
  यावत्( p_पदांशः && p_पदांशः->prereq_दक्षिणः ) p_पदांशः = where_अवलोकनम्_अग्र(&अवलोकनम्);
  यदि( p_पदांशः ){
    testcase( p_पदांशः->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = p_पदांशः;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->r_सततम् = 33;  /* 33==sqlite3LogEst(10) */
  }अन्यथा{
    प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
      पूर्णाङ्क op_आवरणम्;
      प्रतिज्ञा( pLoop->aLTermSpace==pLoop->aLTerm );
      यदि( !Is_Unique_सूचकांक(pIdx)
       || pIdx->pPartIdxWhere!=0
       || pIdx->n_कुञ्चिका_Col>Array_आकार(pLoop->aLTermSpace)
      ) अनुवर्त;
      op_आवरणम् = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      प्रति(j=0; j<pIdx->n_कुञ्चिका_Col; j++){
        p_पदांशः = where_अवलोकनम्_Init(&अवलोकनम्, pWC, iCur, j, op_आवरणम्, pIdx);
        यावत्( p_पदांशः && p_पदांशः->prereq_दक्षिणः ) p_पदांशः = where_अवलोकनम्_अग्र(&अवलोकनम्);
        यदि( p_पदांशः==0 ) भङ्ग;
        testcase( p_पदांशः->eOperator & WO_IS );
        pLoop->aLTerm[j] = p_पदांशः;
      }
      यदि( j!=pIdx->n_कुञ्चिका_Col ) अनुवर्त;
      pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_ONEROW|WHERE_अनुक्रमितम्;
      यदि( pIdx->isCovering || (pItem->col_प्रयुक्तम् & pIdx->colNotIdxed)==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.p_सूचकांक = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->r_सततम् = 39;  /* 39==sqlite3LogEst(15) */
      भङ्ग;
    }
  }
  यदि( pLoop->wsFlags ){
    pLoop->n_बहिर्गमनम् = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    प्रतिज्ञा( pWInfo->s_आवरणम्_समुच्चयः.n==1 && iCur==pWInfo->s_आवरणम्_समुच्चयः.ix[0] );
    pLoop->आवरणम्_Self = 1; /* sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur); */
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->n_Row_बहिर्गमनम् = 1;
    यदि( pWInfo->p_क्रमः_By ) pWInfo->nOBSat =  pWInfo->p_क्रमः_By->n_व्यञ्जकम्;
    यदि( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    यदि( अवलोकनम्.iEquiv>1 ) pLoop->wsFlags |= WHERE_TRANSCONS;
#यदि_परिभाषितम् SQLITE_DEBUG
    pLoop->cId = '0';
#यदि_अन्तः
#यदि_परिभाषितम् WHERETRACE_ENABLED
    यदि( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("whereShortCut() used to compute solution\n");
    }
#यदि_अन्तः
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

/*
** Helper function for exprIsDeterministic().
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_ग्रन्थिः_Is_Deterministic(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  यदि( p_व्यञ्जकम्->op==TK_FUNCTION && व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_ConstFunc)==0 ){
    pWalker->e_संहिता = 0;
    प्रत्यागम WRC_Abort;
  }
  प्रत्यागम WRC_Continue;
}

/*
** Return true if the expression contains no non-deterministic SQL
** functions. Do not consider non-deterministic SQL functions that are
** part of sub-select statements.
*/
स्थिर पूर्णाङ्क व्यञ्जकम्_Is_Deterministic(व्यञ्जकम् *p){
  Walker w;
  स्मृतिं_पूरय(&w, 0, परिमाण(w));
  w.e_संहिता = 1;
  w.x_व्यञ्जकम्_Callback = व्यञ्जकम्_ग्रन्थिः_Is_Deterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3_Walk_व्यञ्जकम्(&w, p);
  प्रत्यागम w.e_संहिता;
}

 
#यदि_परिभाषितम् WHERETRACE_ENABLED
/*
** Display all WhereLoops in pWInfo
*/
स्थिर शून्य showAllWhereLoops(WhereInfo *pWInfo, WhereClause *pWC){
  यदि( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
    WhereLoop *p;
    पूर्णाङ्क i;
    स्थिर ध्रुव वर्ण zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
    प्रति(p=pWInfo->pLoops, i=0; p; p=p->p_अग्र_Loop, i++){
      p->cId = zLabel[i%(परिमाण(zLabel)-1)];
      sqlite3WhereLoopPrint(p, pWC);
    }
  }
}
# define WHERETRACE_ALL_LOOPS(W,C) showAllWhereLoops(W,C)
#अन्यथा
# define WHERETRACE_ALL_LOOPS(W,C)
#यदि_अन्तः

/* Attempt to omit tables from a join that do not affect the result.
** For a table to not affect the result, the following must be true:
**
**   1) The query must not be an aggregate.
**   2) The table must be the RHS of a LEFT JOIN.
**   3) Either the query must be DISTINCT, or else the ON or USING clause
**      must contain a constraint that limits the scan of the table to
**      at most a single row.
**   4) The table must not be referenced by any part of the query apart
**      from its own USING or ON clause.
**   5) The table must not have an inner-join ON or USING clause if there is
**      a RIGHT JOIN anywhere in the query.  Otherwise the ON/USING clause
**      might move from the right side to the left side of the RIGHT JOIN.
**      Note: Due to (2), this condition can only arise if the table is
**      the right-most table of a subquery that was flattened into the
**      main query and that subquery was the right-hand operand of an
**      inner join that held an ON or USING clause.
**   6) The ORDER BY clause has 63 or fewer terms
**   7) The omit-noop-join optimization is enabled.
**
** Items (1), (6), and (7) are checked by the caller.
**
** For example, given:
**
**     CREATE TABLE t1(ipk INTEGER PRIMARY KEY, v1);
**     CREATE TABLE t2(ipk INTEGER PRIMARY KEY, v2);
**     CREATE TABLE t3(ipk INTEGER PRIMARY KEY, v3);
**
** then table t2 can be omitted from the following:
**
**     SELECT v1, v3 FROM t1
**       LEFT JOIN t2 ON (t1.ipk=t2.ipk)
**       LEFT JOIN t3 ON (t1.ipk=t3.ipk)
**
** or from:
**
**     SELECT DISTINCT v1, v3 FROM t1
**       LEFT JOIN t2
**       LEFT JOIN t3 ON (t1.ipk=t3.ipk)
*/
स्थिर SQLITE_NOINLINE Bitmask whereOmitNoopJoin(
  WhereInfo *pWInfo,
  Bitmask notReady
){
  पूर्णाङ्क i;
  Bitmask tab_प्रयुक्तम्;
  पूर्णाङ्क has_दक्षिणः_Join;

  /* Preconditions checked by the caller */
  प्रतिज्ञा( pWInfo->nLevel>=2 );
  प्रतिज्ञा( OptimizationEnabled(pWInfo->pParse->db, SQLITE_OmitNoopJoin) );

  /* These two preconditions checked by the caller combine to guarantee
  ** condition (1) of the header comment */
  प्रतिज्ञा( pWInfo->p_परिणाम_समुच्चयः!=0 );
  प्रतिज्ञा( 0==(pWInfo->wctrlFlags & WHERE_AGG_DISTINCT) );

  tab_प्रयुक्तम् = sqlite3_Where_व्यञ्जकम्_List_Usage(&pWInfo->s_आवरणम्_समुच्चयः, pWInfo->p_परिणाम_समुच्चयः);
  यदि( pWInfo->p_क्रमः_By ){
    tab_प्रयुक्तम् |= sqlite3_Where_व्यञ्जकम्_List_Usage(&pWInfo->s_आवरणम्_समुच्चयः, pWInfo->p_क्रमः_By);
  }
  has_दक्षिणः_Join = (pWInfo->pTabList->a[0].fg.jointype & JT_LTORJ)!=0;
  प्रति(i=pWInfo->nLevel-1; i>=1; i--){
    Where_पदांशः *p_पदांशः, *p_अन्तः;
    SrcItem *pItem;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    pItem = &pWInfo->pTabList->a[pLoop->iTab];
    यदि( (pItem->fg.jointype & (JT_वामः|JT_दक्षिणः))!=JT_वामः ) अनुवर्त;
    यदि( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)==0
     && (pLoop->wsFlags & WHERE_ONEROW)==0
    ){
      अनुवर्त;
    }
    यदि( (tab_प्रयुक्तम् & pLoop->आवरणम्_Self)!=0 ) अनुवर्त;
    p_अन्तः = pWInfo->sWC.a + pWInfo->sWC.n_पदांशः;
    प्रति(p_पदांशः=pWInfo->sWC.a; p_पदांशः<p_अन्तः; p_पदांशः++){
      यदि( (p_पदांशः->prereqAll & pLoop->आवरणम्_Self)!=0 ){
        यदि( !व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON)
         || p_पदांशः->p_व्यञ्जकम्->w.iJoin!=pItem->iCursor
        ){
          भङ्ग;
        }
      }
      यदि( has_दक्षिणः_Join
       && व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_InnerON)
       && NEVER(p_पदांशः->p_व्यञ्जकम्->w.iJoin==pItem->iCursor)
      ){
        भङ्ग;  /* restriction (5) */
      }
    }
    यदि( p_पदांशः<p_अन्तः ) अनुवर्त;
    WHERETRACE(0xffffffff,("-> omit unused FROM-clause term %c\n",pLoop->cId));
    m1 = MASKBIT(i)-1;
    testcase( ((pWInfo->rev_आवरणम्>>1) & ~m1)!=0 );
    pWInfo->rev_आवरणम् = (m1 & pWInfo->rev_आवरणम्) | ((pWInfo->rev_आवरणम्>>1) & ~m1);
    notReady &= ~pLoop->आवरणम्_Self;
    प्रति(p_पदांशः=pWInfo->sWC.a; p_पदांशः<p_अन्तः; p_पदांशः++){
      यदि( (p_पदांशः->prereqAll & pLoop->आवरणम्_Self)!=0 ){
        p_पदांशः->wtFlags |= पदांशः_CODED;
        p_पदांशः->prereqAll = 0;
      }
    }
    यदि( i!=pWInfo->nLevel-1 ){
      पूर्णाङ्क nByte = (pWInfo->nLevel-1-i) * परिमाण(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i+1], nByte);
    }
    pWInfo->nLevel--;
    प्रतिज्ञा( pWInfo->nLevel>0 );
  }
  प्रत्यागम notReady;
}

/*
** Check to see if there are any SEARCH loops that might benefit from
** using a Bloom filter.  Consider a Bloom filter if:
**
**   (1)  The SEARCH happens more than N times where N is the number
**        of rows in the table that is being considered for the Bloom
**        filter.
**   (2)  Some searches are expected to find zero rows.  (This is determined
**        by the WHERE_SELFCULL flag on the term.)
**   (3)  Bloom-filter processing is not disabled.  (Checked by the
**        caller.)
**   (4)  The size of the table being searched is known by ANALYZE.
**
** This block of code merely checks to see if a Bloom filter would be
** appropriate, and if so sets the WHERE_BLOOMFILTER flag on the
** WhereLoop.  The implementation of the Bloom filter comes further
** down where the code for each WhereLoop is generated.
*/
स्थिर SQLITE_NOINLINE शून्य whereCheckIfBloomFilterIsUseful(
  ध्रुव WhereInfo *pWInfo
){
  पूर्णाङ्क i;
  LogEst n_अन्वेषणम् = 0;

  प्रतिज्ञा( pWInfo->nLevel>=2 );
  प्रतिज्ञा( OptimizationEnabled(pWInfo->pParse->db, SQLITE_BloomFilter) );
  प्रति(i=0; i<pWInfo->nLevel; i++){
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    ध्रुव अचिह्नित पूर्णाङ्क reqFlags = (WHERE_SELFCULL|WHERE_COLUMN_EQ);
    SrcItem *pItem = &pWInfo->pTabList->a[pLoop->iTab];
    सारणी *pTab = pItem->pSTab;
    यदि( (pTab->tabFlags & TF_HasStat1)==0 ) भङ्ग;
    pTab->tabFlags |= TF_MaybeReanalyze;
    यदि( i>=1
     && (pLoop->wsFlags & reqFlags)==reqFlags
     && sqlite3WhereLoopBloomable(pLoop)
     && n_अन्वेषणम् > pTab->nRowLogEst
    ){
      testcase( pItem->fg.jointype & JT_वामः );
      pLoop->wsFlags |= WHERE_BLOOMFILTER;
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      WHERETRACE(0xffffffff, (
         "-> use Bloom-filter on loop %c because there are ~%.1e "
         "lookups into %s which has only ~%.1e rows\n",
         pLoop->cId, (द्विगुण)sqlite3_Log_Est_गन्तव्यम्_Int(n_अन्वेषणम्), pTab->zName,
         (द्विगुण)sqlite3_Log_Est_गन्तव्यम्_Int(pTab->nRowLogEst)));
    }
    n_अन्वेषणम् += pLoop->n_बहिर्गमनम्;
  }
}

/*
** The index pIdx is used by a query and contains one or more expressions.
** In other words pIdx is an index on an expression.  iIdxCur is the cursor
** number for the index and iDataCur is the cursor number for the corresponding
** table.
**
** This routine adds IndexedExpr entries to the Parse->pIdxEpr field for
** each of the expressions in the index so that the expression code generator
** will know to replace occurrences of the indexed expression with
** references to the corresponding column of the index.
*/
स्थिर SQLITE_NOINLINE शून्य where_सङ्कलय_अनुक्रमितम्_व्यञ्जकम्(
  Parse *pParse,     /* Add IndexedExpr entries to pParse->pIdxEpr */
  सूचकांक *pIdx,       /* The index-on-expression that contains the expressions */
  पूर्णाङ्क iIdxCur,       /* Cursor number for pIdx */
  SrcItem *pTabItem  /* The FROM clause entry for the table */
){
  पूर्णाङ्क i;
  अनुक्रमितम्_व्यञ्जकम् *p;
  सारणी *pTab;
  प्रतिज्ञा( pIdx->b_Has_व्यञ्जकम् );
  pTab = pIdx->p_सारणी;
  प्रति(i=0; i<pIdx->nColumn; i++){
    व्यञ्जकम् *p_व्यञ्जकम्;
    पूर्णाङ्क j = pIdx->aiColumn[i];
    यदि( j==XN_व्यञ्जकम् ){
      p_व्यञ्जकम् = pIdx->a_Col_व्यञ्जकम्->a[i].p_व्यञ्जकम्;
    }अन्यथा यदि( j>=0 && (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)!=0 ){
      p_व्यञ्जकम् = sqlite3_Column_व्यञ्जकम्(pTab, &pTab->aCol[j]);
    }अन्यथा{
      अनुवर्त;
    }
    यदि( sqlite3_व्यञ्जकम्_Is_Constant(0,p_व्यञ्जकम्) ) अनुवर्त;
    p = sqlite3DbMallocRaw(pParse->db,  परिमाण(अनुक्रमितम्_व्यञ्जकम्));
    यदि( p==0 ) भङ्ग;
    p->pIENext = pParse->pIdxEpr;
#यदि_परिभाषितम् WHERETRACE_ENABLED
    यदि( sqlite3WhereTrace & 0x200 ){
      sqlite3DebugPrintf("New pParse->pIdxEpr term {%d,%d}\n", iIdxCur, i);
      यदि( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_व्यञ्जकम्(p_व्यञ्जकम्);
    }
#यदि_अन्तः
    p->p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(pParse->db, p_व्यञ्जकम्, 0);
    p->i_दत्त_Cur = pTabItem->iCursor;
    p->iIdxCur = iIdxCur;
    p->iIdxCol = i;
    p->bMaybeNullRow = (pTabItem->fg.jointype & (JT_वामः|JT_LTORJ|JT_दक्षिणः))!=0;
    यदि( sqlite3_सूचकांक_Affinity_Str(pParse->db, pIdx) ){
      p->aff = pIdx->zColAff[i];
    }
#यदि_परिभाषितम् SQLITE_ENABLE_EXPLAIN_COMMENTS
    p->zIdxName = pIdx->zName;
#यदि_अन्तः
    pParse->pIdxEpr = p;
    यदि( p->pIENext==0 ){
      शून्य *pArg = (शून्य*)&pParse->pIdxEpr;
      sqlite3_Parser_सङ्कलय_Cleanup(pParse, where_अनुक्रमितम्_व्यञ्जकम्_Cleanup, pArg);
    }
  }
}

/*
** Set the reverse-scan order mask to one for all tables in the query
** with the exception of MATERIALIZED common table expressions that have
** their own internal ORDER BY clauses.
**
** This implements the PRAGMA reverse_unordered_selects=ON setting.
** (Also SQLITE_DBCONFIG_REVERSE_SCANORDER).
*/
स्थिर SQLITE_NOINLINE शून्य where_Reverse_अवलोकनम्_क्रमः(WhereInfo *pWInfo){
  पूर्णाङ्क ii;
  प्रति(ii=0; ii<pWInfo->pTabList->nSrc; ii++){
    SrcItem *pItem = &pWInfo->pTabList->a[ii];
    यदि( !pItem->fg.isCte
     || pItem->u2.pCteUse->eM10d!=M10d_Yes
     || NEVER(pItem->fg.isSubquery==0)
     || pItem->u4.pSubq->pSelect->p_क्रमः_By==0
    ){
      pWInfo->rev_आवरणम् |= MASKBIT(ii);
    }
  }
}

/*
** Generate the beginning of the loop used for WHERE clause processing.
** The return value is a pointer to an opaque structure that contains
** information needed to terminate the loop.  Later, the calling routine
** should invoke sqlite3WhereEnd() with the return value of this function
** in order to complete the WHERE clause processing.
**
** If an error occurs, this routine returns NULL.
**
** The basic idea is to do a nested loop, one loop for each table in
** the FROM clause of a select.  (INSERT and UPDATE statements are the
** same as a SELECT with only a single table in the FROM clause.)  For
** example, if the SQL is this:
**
**       SELECT * FROM t1, t2, t3 WHERE ...;
**
** Then the code generated is conceptually like the following:
**
**      foreach row1 in t1 do       \    Code generated
**        foreach row2 in t2 do      |-- by sqlite3WhereBegin()
**          foreach row3 in t3 do   /
**            ...
**          end                     \    Code generated
**        end                        |-- by sqlite3WhereEnd()
**      end                         /
**
** Note that the loops might not be nested in the order in which they
** appear in the FROM clause if a different order is better able to make
** use of indices.  Note also that when the IN operator appears in
** the WHERE clause, it might result in additional nested loops for
** scanning through all values on the right-hand side of the IN.
**
** There are Btree cursors associated with each table.  t1 uses cursor
** number pTabList->a[0].iCursor.  t2 uses the cursor pTabList->a[1].iCursor.
** And so forth.  This routine generates code to open those VDBE cursors
** and sqlite3WhereEnd() generates the code to close them.
**
** The code that sqlite3WhereBegin() generates leaves the cursors named
** in pTabList pointing at their appropriate entries.  The [...] code
** can use OP_Column and OP_Rowid opcodes on these cursors to extract
** data from the various tables of the loop.
**
** If the WHERE clause is empty, the foreach loops must each scan their
** entire tables.  Thus a three-way join is an O(N^3) operation.  But if
** the tables have indices and there are terms in the WHERE clause that
** refer to those indices, a complete table scan can be avoided and the
** code will run much faster.  Most of the work of this routine is checking
** to see if there are indices that can be used to speed up the loop.
**
** Terms of the WHERE clause are also used to limit which rows actually
** make it to the "..." in the middle of the loop.  After each "foreach",
** terms of the WHERE clause that use only terms in that loop and outer
** loops are evaluated and if false a jump is made around all subsequent
** inner loops (or around the "..." if the test occurs within the inner-
** most loop)
**
** OUTER JOINS
**
** An outer join of tables t1 and t2 is conceptually coded as follows:
**
**    foreach row1 in t1 do
**      flag = 0
**      foreach row2 in t2 do
**        start:
**          ...
**          flag = 1
**      end
**      if flag==0 then
**        move the row2 cursor to a null row
**        goto start
**      fi
**    end
**
** ORDER BY CLAUSE PROCESSING
**
** pOrderBy is a pointer to the ORDER BY clause (or the GROUP BY clause
** if the WHERE_GROUPBY flag is set in wctrlFlags) of a SELECT statement
** if there is one.  If there is no ORDER BY clause or if this routine
** is called from an UPDATE or DELETE statement, then pOrderBy is NULL.
**
** The iIdxCur parameter is the cursor number of an index.  If
** WHERE_OR_SUBCLAUSE is set, iIdxCur is the cursor number of an index
** to use for OR clause processing.  The WHERE clause should use this
** specific cursor.  If WHERE_ONEPASS_DESIRED is set, then iIdxCur is
** the first cursor in an array of cursors for all indices.  iIdxCur should
** be used to compute the appropriate cursor depending on which index is
** used.
*/
WhereInfo *sqlite3WhereBegin(
  Parse *pParse,          /* The parser context */
  SrcList *pTabList,      /* FROM clause: A list of all tables to be scanned */
  व्यञ्जकम् *pWhere,           /* The WHERE clause */
  व्यञ्जकम्_List *p_क्रमः_By,     /* An ORDER BY (or GROUP BY) clause, or NULL */
  व्यञ्जकम्_List *p_परिणाम_समुच्चयः,   /* Query result set.  Req'd for DISTINCT */
  Select *pSelect,        /* The entire SELECT statement */
  u16 wctrlFlags,         /* The WHERE_* flags defined in sqliteInt.h */
  पूर्णाङ्क iAuxArg             /* If WHERE_OR_SUBCLAUSE is set, index cursor number
                          ** If WHERE_USE_LIMIT, then the limit amount */
){
  पूर्णाङ्क nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  पूर्णाङ्क nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  Where_आवरणम्_समुच्चयः *p_आवरणम्_समुच्चयः;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  पूर्णाङ्क ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  पूर्णाङ्क rc;                    /* Return code */
  u8 bFordelete = 0;         /* OPFLAG_FORDELETE or zero, as appropriate */

  प्रतिज्ञा( (wctrlFlags & WHERE_ONEPASS_MULTIROW)==0 || (
        (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
  ));

  /* Only one of WHERE_OR_SUBCLAUSE or WHERE_USE_LIMIT */
  प्रतिज्ञा( (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
            || (wctrlFlags & WHERE_USE_सीमा)==0 );

  /* Variable initialization */
  db = pParse->db;
  स्मृतिं_पूरय(&sWLB, 0, परिमाण(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( p_क्रमः_By && p_क्रमः_By->n_व्यञ्जकम्==BMS-1 );
  यदि( p_क्रमः_By && p_क्रमः_By->n_व्यञ्जकम्>=BMS ){
    p_क्रमः_By = 0;
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
    wctrlFlags |= WHERE_KEEP_ALL_JOINS; /* Disable omit-noop-join opt */
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask
  */
  testcase( pTabList->nSrc==BMS );
  यदि( pTabList->nSrc>BMS ){
    sqlite3_Error_सन्देशः(pParse, "at most %d tables in a join", BMS);
    प्रत्यागम 0;
  }

  /* This function normally generates a nested loop for all tables in
  ** pTabList.  But if the WHERE_OR_SUBCLAUSE flag is set, then we should
  ** only generate code for the first table in pTabList and assume that
  ** any cursors associated with subsequent tables are uninitialized.
  */
  nTabList = (wctrlFlags & WHERE_OR_SUBCLAUSE) ? 1 : pTabList->nSrc;

  /* Allocate and initialize the WhereInfo structure that will become the
  ** return value. A single allocation is used to store the WhereInfo
  ** struct, the contents of WhereInfo.a[], the WhereClause structure
  ** and the WhereMaskSet structure. Since WhereClause contains an 8-byte
  ** field (type Bitmask) it must be aligned on an 8-byte boundary on
  ** some architectures. Hence the ROUND8() below.
  */
  nByteWInfo = SZ_WHEREINFO(nTabList);
  pWInfo = sqlite3DbMallocRawNN(db, nByteWInfo + परिमाण(WhereLoop));
  यदि( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    गच्छ whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->p_क्रमः_By = p_क्रमः_By;
#यदि WHERETRACE_ENABLED
  pWInfo->pWhere = pWhere;
#यदि_अन्तः
  pWInfo->p_परिणाम_समुच्चयः = p_परिणाम_समुच्चयः;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->i_सीमा = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  स्मृतिं_पूरय(&pWInfo->nOBSat, 0,
         offsetof(WhereInfo,sWC) - offsetof(WhereInfo,nOBSat));
  स्मृतिं_पूरय(&pWInfo->a[0], 0, परिमाण(WhereLoop)+nTabList*परिमाण(WhereLevel));
  प्रतिज्ञा( pWInfo->eOnePass==ONEPASS_OFF );  /* ONEPASS defaults to OFF */
  p_आवरणम्_समुच्चयः = &pWInfo->s_आवरणम्_समुच्चयः;
  p_आवरणम्_समुच्चयः->n = 0;
  p_आवरणम्_समुच्चयः->ix[0] = -99; /* Initialize ix[0] to a value that can never be
                         ** a valid cursor number, to avoid an initial
                         ** test for pMaskSet->n==0 in sqlite3WhereGetMask() */
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((वर्ण*)pWInfo)+nByteWInfo);
  प्रतिज्ञा( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#यदि_परिभाषितम् SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#यदि_अन्तः

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3_Where_विभज(&pWInfo->sWC, pWhere, TK_AND);
   
  /* Special case: No FROM clause
  */
  यदि( nTabList==0 ){
    यदि( p_क्रमः_By ) pWInfo->nOBSat = p_क्रमः_By->n_व्यञ्जकम्;
    यदि( (wctrlFlags & WHERE_WANT_DISTINCT)!=0
     && OptimizationEnabled(db, SQLITE_DistinctOpt)
    ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
    यदि( ALWAYS(pWInfo->pSelect)
     && (pWInfo->pSelect->selFlags & SF_Multi_मानम्)==0
    ){
      ExplainQueryPlan((pParse, 0, "SCAN CONSTANT ROW"));
    }
  }अन्यथा{
    /* Assign a bit from the bitmask to every term in the FROM clause.
    **
    ** The N-th term of the FROM clause is assigned a bitmask of 1<<N.
    **
    ** The rule of the previous sentence ensures that if X is the bitmask for
    ** a table T, then X-1 is the bitmask for all other tables to the left of T.
    ** Knowing the bitmask for all tables to the left of a left join is
    ** important.  Ticket #3015.
    **
    ** Note that bitmasks are created for all pTabList->nSrc tables in
    ** pTabList, not just the first nTabList tables.  nTabList is normally
    ** equal to pTabList->nSrc but might be shortened to 1 if the
    ** WHERE_OR_SUBCLAUSE flag is set.
    */
    ii = 0;
    कुरु{
      create_आवरणम्(p_आवरणम्_समुच्चयः, pTabList->a[ii].iCursor);
      sqlite3_Where_Tab_Func_तर्काः(pParse, &pTabList->a[ii], &pWInfo->sWC);
    }यावत्( (++ii)<pTabList->nSrc );
  #यदि_परिभाषितम् SQLITE_DEBUG
    {
      Bitmask mx = 0;
      प्रति(ii=0; ii<pTabList->nSrc; ii++){
        Bitmask m = sqlite3_Where_Get_आवरणम्(p_आवरणम्_समुच्चयः, pTabList->a[ii].iCursor);
        प्रतिज्ञा( m>=mx );
        mx = m;
      }
    }
  #यदि_अन्तः
  }
 
  /* Analyze all of the subexpressions. */
  sqlite3_Where_व्यञ्जकम्_Analyze(pTabList, &pWInfo->sWC);
  यदि( pSelect && pSelect->p_सीमा ){
    sqlite3_Where_सङ्कलय_सीमा(&pWInfo->sWC, pSelect);
  }
  यदि( pParse->nErr ) गच्छ whereBeginError;

  /* The False-WHERE-Term-Bypass optimization:
  **
  ** If there are WHERE terms that are false, then no rows will be output,
  ** so skip over all of the code generated here.
  **
  ** Conditions:
  **
  **   (1)  The WHERE term must not refer to any tables in the join.
  **   (2)  The term must not come from an ON clause on the
  **        right-hand side of a LEFT or FULL JOIN.
  **   (3)  The term must not come from an ON clause, or there must be
  **        no RIGHT or FULL OUTER joins in pTabList.
  **   (4)  If the expression contains non-deterministic functions
  **        that are not within a sub-select. This is not required
  **        for correctness but rather to preserves SQLite's legacy
  **        behaviour in the following two cases:
  **
  **          WHERE random()>0;           -- eval random() once per row
  **          WHERE (SELECT random())>0;  -- eval random() just once overall
  **
  ** Note that the Where term need not be a constant in order for this
  ** optimization to apply, though it does need to be constant relative to
  ** the current subquery (condition 1).  The term might include variables
  ** from outer queries so that the value of the term changes from one
  ** invocation of the current subquery to the next.
  */
  प्रति(ii=0; ii<sWLB.pWC->n_आधारः; ii++){
    Where_पदांशः *pT = &sWLB.pWC->a[ii];  /* A term of the WHERE clause */
    व्यञ्जकम् *pX;                          /* The expression of pT */
    यदि( pT->wtFlags & पदांशः_VIRTUAL ) अनुवर्त;
    pX = pT->p_व्यञ्जकम्;
    प्रतिज्ञा( pX!=0 );
    प्रतिज्ञा( pT->prereqAll!=0 || !व्यञ्जकम्_Has_Property(pX, EP_OuterON) );
    यदि( pT->prereqAll==0                           /* Conditions (1) and (2) */
     && (nTabList==0 || व्यञ्जकम्_Is_Deterministic(pX))   /* Condition (4) */
     && !(व्यञ्जकम्_Has_Property(pX, EP_InnerON)          /* Condition (3) */
          && (pTabList->a[0].fg.jointype & JT_LTORJ)!=0 )
    ){
      sqlite3_व्यञ्जकम्_If_False(pParse, pX, pWInfo->iBreak, SQLITE_JUMPIFNULL);
      pT->wtFlags |= पदांशः_CODED;
    }
  }

  यदि( wctrlFlags & WHERE_WANT_DISTINCT ){
    यदि( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
      /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
      ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
      wctrlFlags &= ~WHERE_WANT_DISTINCT;
      pWInfo->wctrlFlags &= ~WHERE_WANT_DISTINCT;
    }अन्यथा यदि( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, p_परिणाम_समुच्चयः) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }अन्यथा यदि( p_क्रमः_By==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->p_क्रमः_By = p_परिणाम_समुच्चयः;
    }
  }

  /* Construct the WhereLoop objects */
#यदि defined(WHERETRACE_ENABLED)
  यदि( sqlite3WhereTrace & 0xffffffff ){
    sqlite3DebugPrintf("*** Optimizer Start *** (wctrlFlags: 0x%x",wctrlFlags);
    यदि( wctrlFlags & WHERE_USE_सीमा ){
      sqlite3DebugPrintf(", limit: %d", iAuxArg);
    }
    sqlite3DebugPrintf(")\n");
    यदि( sqlite3WhereTrace & 0x8000 ){
      Select sSelect;
      स्मृतिं_पूरय(&sSelect, 0, परिमाण(sSelect));
      sSelect.selFlags = SF_WhereBegin;
      sSelect.pSrc = pTabList;
      sSelect.pWhere = pWhere;
      sSelect.p_क्रमः_By = p_क्रमः_By;
      sSelect.pEList = p_परिणाम_समुच्चयः;
      sqlite3_वृक्षः_View_Select(0, &sSelect, 0);
    }
    यदि( sqlite3WhereTrace & 0x4000 ){ /* Display all WHERE clause terms */
      sqlite3DebugPrintf("---- WHERE clause at start of analysis:\n");
      sqlite3WhereClausePrint(sWLB.pWC);
    }
  }
#यदि_अन्तः

  यदि( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = where_Loop_सङ्कलय_All(&sWLB);
    यदि( rc ) गच्छ whereBeginError;

#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
    /* If one or more WhereTerm.truthProb values were used in estimating
    ** loop parameters, but then those truthProb values were subsequently
    ** changed based on STAT4 information while computing subsequent loops,
    ** then we need to rerun the whole loop building process so that all
    ** loops will be built using the revised truthProb values. */
    यदि( sWLB.bldFlags2 & SQLITE_BLDF2_2NDPASS ){
      WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
      WHERETRACE(0xffffffff,
           ("**** Redo all loop computations due to"
            " TERM_HIGHTRUTH changes ****\n"));
      यावत्( pWInfo->pLoops ){
        WhereLoop *p = pWInfo->pLoops;
        pWInfo->pLoops = p->p_अग्र_Loop;
        whereLoopDelete(db, p);
      }
      rc = where_Loop_सङ्कलय_All(&sWLB);
      यदि( rc ) गच्छ whereBeginError;
    }
#यदि_अन्तः
    WHERETRACE_ALL_LOOPS(pWInfo, sWLB.pWC);
 
    where_मार्गः_Solver(pWInfo, 0);
    यदि( db->mallocFailed ) गच्छ whereBeginError;
    यदि( pWInfo->p_क्रमः_By ){
       whereInterstageHeuristic(pWInfo);
       where_मार्गः_Solver(pWInfo, pWInfo->n_Row_बहिर्गमनम्<0 ? 1 : pWInfo->n_Row_बहिर्गमनम्+1);
       यदि( db->mallocFailed ) गच्छ whereBeginError;
    }

    /* TUNING:  Assume that a DISTINCT clause on a subquery reduces
    ** the output size by a factor of 8 (LogEst -30).  Search for
    ** tag-20250414a to see other cases.
    */
    यदि( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0 ){
      WHERETRACE(0x0080,("nRowOut reduced from %d to %d due to DISTINCT\n",
                         pWInfo->n_Row_बहिर्गमनम्, pWInfo->n_Row_बहिर्गमनम्-30));
      pWInfo->n_Row_बहिर्गमनम् -= 30;
    }

  }
  प्रतिज्ञा( pWInfo->pTabList!=0 );
  यदि( pWInfo->p_क्रमः_By==0 && (db->flags & SQLITE_Reverse_क्रमः)!=0 ){
    where_Reverse_अवलोकनम्_क्रमः(pWInfo);
  }
  यदि( pParse->nErr ){
    गच्छ whereBeginError;
  }
  प्रतिज्ञा( db->mallocFailed==0 );
#यदि_परिभाषितम् WHERETRACE_ENABLED
  यदि( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution cost=%d, nRow=%d",
                       pWInfo->r_योग_Cost, pWInfo->n_Row_बहिर्गमनम्);
    यदि( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->rev_आवरणम्);
    }
    विकल्प( pWInfo->eDistinct ){
      पक्ष WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        भङ्ग;
      }
      पक्ष WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        भङ्ग;
      }
      पक्ष WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        भङ्ग;
      }
    }
    sqlite3DebugPrintf("\n");
    प्रति(ii=0; ii<pWInfo->nLevel; ii++){
      sqlite3WhereLoopPrint(pWInfo->a[ii].pWLoop, sWLB.pWC);
    }
  }
#यदि_अन्तः

  /* Attempt to omit tables from a join that do not affect the result.
  ** See the comment on whereOmitNoopJoin() for further information.
  **
  ** This query optimization is factored out into a separate "no-inline"
  ** procedure to keep the sqlite3WhereBegin() procedure from becoming
  ** too large.  If sqlite3WhereBegin() becomes too large, that prevents
  ** some C-compiler optimizers from in-lining the
  ** sqlite3WhereCodeOneLoopStart() procedure, and it is important to
  ** in-line sqlite3WhereCodeOneLoopStart() for performance reasons.
  */
  notReady = ~(Bitmask)0;
  यदि( pWInfo->nLevel>=2       /* Must be a join, or this opt8n is pointless */
   && p_परिणाम_समुच्चयः!=0           /* Condition (1) */
   && 0==(wctrlFlags & (WHERE_AGG_DISTINCT|WHERE_KEEP_ALL_JOINS)) /* (1),(6) */
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)                /* (7) */
  ){
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
    प्रतिज्ञा( nTabList>0 );
  }

  /* Check to see if there are any SEARCH loops that might benefit from
  ** using a Bloom filter.
  */
  यदि( pWInfo->nLevel>=2
   && OptimizationEnabled(db, SQLITE_BloomFilter)
  ){
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

#यदि defined(WHERETRACE_ENABLED)
  यदि( sqlite3WhereTrace & 0x4000 ){ /* Display all terms of the WHERE clause */
    sqlite3DebugPrintf("---- WHERE clause at end of analysis:\n");
    sqlite3WhereClausePrint(sWLB.pWC);
  }
  WHERETRACE(0xffffffff,("*** Optimizer Finished ***\n"));
#यदि_अन्तः
  pWInfo->pParse->nQueryLoop += pWInfo->n_Row_बहिर्गमनम्;

  /* If the caller is an UPDATE or DELETE statement that is requesting
  ** to use a one-pass algorithm, determine if this is appropriate.
  **
  ** A one-pass approach can be used if the caller has requested one
  ** and either (a) the scan visits at most one row or (b) each
  ** of the following are true:
  **
  **   * the caller has indicated that a one-pass approach can be used
  **     with multiple rows (by setting WHERE_ONEPASS_MULTIROW), and
  **   * the table is not a virtual table, and
  **   * either the scan does not use the OR optimization or the caller
  **     is a DELETE operation (WHERE_DUPLICATES_OK is only specified
  **     for DELETE).
  **
  ** The last qualification is because an UPDATE statement uses
  ** WhereInfo.aiCurOnePass[1] to determine whether or not it really can
  ** use a one-pass approach, and this is not set accurately for scans
  ** that use the OR optimization.
  */
  प्रतिज्ञा( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  यदि( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 ){
    पूर्णाङ्क wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    पूर्णाङ्क bOnerow = (wsFlags & WHERE_ONEROW)!=0;
    प्रतिज्ञा( !(wsFlags&WHERE_VIRTUALTABLE) || IsVirtual(pTabList->a[0].pSTab) );
    यदि( bOnerow || (
        0!=(wctrlFlags & WHERE_ONEPASS_MULTIROW)
     && !IsVirtual(pTabList->a[0].pSTab)
     && (0==(wsFlags & WHERE_MULTI_OR) || (wctrlFlags & WHERE_DUPLICATES_OK))
     && OptimizationEnabled(db, SQLITE_OnePass)
    )){
      pWInfo->eOnePass = bOnerow ? ONEPASS_SINGLE : ONEPASS_MULTI;
      यदि( HasRowid(pTabList->a[0].pSTab) && (wsFlags & WHERE_IDX_ONLY) ){
        यदि( wctrlFlags & WHERE_ONEPASS_MULTIROW ){
          bFordelete = OPFLAG_FORDELETE;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~WHERE_IDX_ONLY);
      }
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  प्रति(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    सारणी *pTab;     /* Table to open */
    पूर्णाङ्क iDb;         /* Index of database containing table/index */
    SrcItem *pTabItem;

    pTabItem = &pTabList->a[pLevel->iFrom];
    pTab = pTabItem->pSTab;
    iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    यदि( ii==0 || (pTabItem[0].fg.jointype & JT_वामः)!=0 ){
      pLevel->addrHalt = pLevel->addrBrk;
    }अन्यथा यदि( pWInfo->a[ii-1].pRJ ){
      pLevel->addrHalt = pWInfo->a[ii-1].addrBrk;
    }अन्यथा{
      pLevel->addrHalt = pWInfo->a[ii-1].addrHalt;
    }
    यदि( (pTab->tabFlags & TF_Ephemeral)!=0 || IsView(pTab) ){
      /* Do nothing */
    }अन्यथा
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
    यदि( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      ध्रुव वर्ण *pVTab = (ध्रुव वर्ण *)sqlite3GetVTable(db, pTab);
      पूर्णाङ्क iCur = pTabItem->iCursor;
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }अन्यथा यदि( IsVirtual(pTab) ){
      /* noop */
    }अन्यथा
#यदि_अन्तः
    यदि( ((pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0)
     || (pTabItem->fg.jointype & (JT_LTORJ|JT_दक्षिणः))!=0
    ){
      पूर्णाङ्क op = OP_OpenRead;
      यदि( pWInfo->eOnePass!=ONEPASS_OFF ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = pTabItem->iCursor;
      };
      sqlite3_Open_सारणी(pParse, pTabItem->iCursor, iDb, pTab, op);
      प्रतिज्ञा( pTabItem->iCursor==pLevel->iTabCur );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS-1 );
      testcase( pWInfo->eOnePass==ONEPASS_OFF && pTab->nCol==BMS );
      यदि( pWInfo->eOnePass==ONEPASS_OFF
       && pTab->nCol<BMS
       && (pTab->tabFlags & (TF_HasGenerated|TF_WithoutRowid))==0
       && (pLoop->wsFlags & (WHERE_AUTO_सूचकांक|WHERE_BLOOMFILTER))==0
      ){
        /* If we know that only a prefix of the record will be used,
        ** it is advantageous to reduce the "column count" field in
        ** the P4 operand of the OP_OpenRead/Write opcode. */
        Bitmask b = pTabItem->col_प्रयुक्तम्;
        पूर्णाङ्क n = 0;
        प्रति(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, -1, SQLITE_INT_गन्तव्यम्_PTR(n), P4_INT32);
        प्रतिज्ञा( n<=pTab->nCol );
      }
#यदि_परिभाषितम् SQLITE_ENABLE_CURSOR_HINTS
      यदि( pLoop->u.btree.p_सूचकांक!=0 && (pTab->tabFlags & TF_WithoutRowid)==0 ){
        sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ|bFordelete);
      }अन्यथा
#यदि_अन्तः
      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }
#यदि_परिभाषितम् SQLITE_ENABLE_COLUMN_प्रयुक्तम्_आवरणम्
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Columns_प्रयुक्तम्, pTabItem->iCursor,
                    LOWER32(pTabItem->col_प्रयुक्तम्), UPPER32(pTabItem->col_प्रयुक्तम्));
#यदि_अन्तः
      यदि( ii>=2
       && (pTabItem[0].fg.jointype & (JT_LTORJ|JT_वामः))==0 
       && pLevel->addrHalt==pWInfo->a[0].addrHalt
      ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_If_रिक्तानि, pTabItem->iCursor, pWInfo->iBreak);
        VdbeCoverage(v);
      }
    }अन्यथा{
      sqlite3_सारणी_तालकम्(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    यदि( pLoop->wsFlags & WHERE_अनुक्रमितम् ){
      सूचकांक *pIx = pLoop->u.btree.p_सूचकांक;
      पूर्णाङ्क i_सूचकांक_Cur;
      पूर्णाङ्क op = OP_OpenRead;
      /* iAuxArg is always set to a positive value if ONEPASS is possible */
      प्रतिज्ञा( iAuxArg!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      यदि( !HasRowid(pTab) && Is_Primary_कुञ्चिका_सूचकांक(pIx)
       && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        i_सूचकांक_Cur = pLevel->iTabCur;
        op = 0;
      }अन्यथा यदि( pWInfo->eOnePass!=ONEPASS_OFF ){
        सूचकांक *pJ = pTabItem->pSTab->p_सूचकांक;
        i_सूचकांक_Cur = iAuxArg;
        प्रतिज्ञा( wctrlFlags & WHERE_ONEPASS_DESIRED );
        यावत्( ALWAYS(pJ) && pJ!=pIx ){
          i_सूचकांक_Cur++;
          pJ = pJ->p_अग्र;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = i_सूचकांक_Cur;
      }अन्यथा यदि( iAuxArg && (wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 ){
        i_सूचकांक_Cur = iAuxArg;
        op = OP_ReopenIdx;
      }अन्यथा{
        i_सूचकांक_Cur = pParse->nTab++;
        यदि( pIx->b_Has_व्यञ्जकम् && OptimizationEnabled(db, SQLITE_अनुक्रमितम्_व्यञ्जकम्) ){
          where_सङ्कलय_अनुक्रमितम्_व्यञ्जकम्(pParse, pIx, i_सूचकांक_Cur, pTabItem);
        }
        यदि( pIx->pPartIdxWhere && (pTabItem->fg.jointype & JT_दक्षिणः)==0 ){
          where_Part_Idx_व्यञ्जकम्(
              pParse, pIx, pIx->pPartIdxWhere, 0, i_सूचकांक_Cur, pTabItem
          );
        }
      }
      pLevel->iIdxCur = i_सूचकांक_Cur;
      प्रतिज्ञा( pIx!=0 );
      प्रतिज्ञा( pIx->pSchema==pTab->pSchema );
      प्रतिज्ञा( i_सूचकांक_Cur>=0 );
      यदि( op ){
        sqlite3_Vdbe_सङ्कलय_Op3(v, op, i_सूचकांक_Cur, pIx->tnum, iDb);
        sqlite3_Vdbe_समुच्चयः_P4_कुञ्चिका_Info(pParse, pIx);
        यदि( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_COLUMN_RANGE|WHERE_SKIPSCAN))==0
         && (pLoop->wsFlags & WHERE_BIGNULL_क्रमय)==0
         && (pLoop->wsFlags & WHERE_अन्तर्_SEEKSCAN)==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
         && pWInfo->eDistinct!=WHERE_DISTINCT_ORDERED
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ);
        }
        VdbeComment((v, "%s", pIx->zName));
#यदि_परिभाषितम् SQLITE_ENABLE_COLUMN_प्रयुक्तम्_आवरणम्
        {
          u64 col_प्रयुक्तम् = 0;
          पूर्णाङ्क ii, jj;
          प्रति(ii=0; ii<pIx->nColumn; ii++){
            jj = pIx->aiColumn[ii];
            यदि( jj<0 ) अनुवर्त;
            यदि( jj>63 ) jj = 63;
            यदि( (pTabItem->col_प्रयुक्तम् & MASKBIT(jj))==0 ) अनुवर्त;
            col_प्रयुक्तम् |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Columns_प्रयुक्तम्, i_सूचकांक_Cur,
                            LOWER32(col_प्रयुक्तम्),UPPER32(col_प्रयुक्तम्));
        }
#यदि_अन्तः /* SQLITE_ENABLE_COLUMN_USED_MASK */
#यदि_परिभाषितम् SQLITE_ENABLE_CURSOR_HINTS
        यदि( HasRowid(pTab) ){
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_CursorHint, i_सूचकांक_Cur, 0, pTabItem->iCursor);
        }
#यदि_अन्तः
      }
    }
    यदि( iDb>=0 ) sqlite3_संहिता_Verify_Schema(pParse, iDb);
    यदि( (pTabItem->fg.jointype & JT_दक्षिणः)!=0
     && (pLevel->pRJ = sqlite3WhereMalloc(pWInfo, परिमाण(Where_दक्षिणः_Join)))!=0
    ){
      Where_दक्षिणः_Join *pRJ = pLevel->pRJ;
      पूर्णाङ्क bBloomable = 0;
      pRJ->i_मेलनम् = pParse->nTab++;
      pRJ->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, pRJ->regReturn);
      प्रतिज्ञा( pTab==pTabItem->pSTab );
      यदि( HasRowid(pTab) ){
        कुञ्चिका_Info *pInfo;
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, pRJ->i_मेलनम्, 1);
        bBloomable = 1;
        pInfo = sqlite3_कुञ्चिका_Info_Alloc(pParse->db, 1, 0);
        यदि( pInfo ){
          pInfo->aColl[0] = 0;
          pInfo->a_क्रमय_Flags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, P4_KEYINFO);
        }
      }अन्यथा{
        सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, pRJ->i_मेलनम्, pPk->n_कुञ्चिका_Col);
        sqlite3_Vdbe_समुच्चयः_P4_कुञ्चिका_Info(pParse, pPk);
        bBloomable = sqlite3_सूचकांक_Bloomable(pPk,pPk->n_कुञ्चिका_Col);
      }
      यदि( bBloomable ){
        pRJ->regBloom = ++pParse->nMem;
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Blob, 65536, pRJ->regBloom);
      }अन्यथा{
        pRJ->regBloom = 0;
      }
      pLoop->wsFlags &= ~WHERE_IDX_ONLY;
      /* The nature of RIGHT JOIN processing is such that it messes up
      ** the output order.  So omit any ORDER BY/GROUP BY elimination
      ** optimizations.  We need to do an actual sort for RIGHT JOIN. */
      pWInfo->nOBSat = 0;
      pWInfo->eDistinct = WHERE_DISTINCT_UNORDERED;
    }
  }
  pWInfo->i_शिखरम् = sqlite3VdbeCurrentAddr(v);
  यदि( db->mallocFailed ) गच्छ whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  प्रति(ii=0; ii<nTabList; ii++){
    पूर्णाङ्क addrExplain;
    पूर्णाङ्क wsFlags;
    SrcItem *pSrc;
    यदि( pParse->nErr ) गच्छ whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    यदि( pSrc->fg.isMaterialized ){
      Subquery *pSubq;
      पूर्णाङ्क iOnce = 0;
      प्रतिज्ञा( pSrc->fg.isSubquery );
      pSubq = pSrc->u4.pSubq;
      यदि( pSrc->fg.isCorrelated==0 ){
        iOnce = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Once); VdbeCoverage(v);
      }अन्यथा{
        iOnce = 0;
      }
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, pSubq->regReturn, pSubq->addrFillSub);
      VdbeComment((v, "materialize %!S", pSrc));
      यदि( iOnce )  sqlite3_Vdbe_Jump_अत्र(v, iOnce);
    }
    प्रतिज्ञा( pTabList == pWInfo->pTabList );
    यदि( (wsFlags & (WHERE_AUTO_सूचकांक|WHERE_BLOOMFILTER))!=0 ){
      यदि( (wsFlags & WHERE_AUTO_सूचकांक)!=0 ){
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOMATIC_सूचकांक
        construct_Automatic_सूचकांक(pParse, &pWInfo->sWC, notReady, pLevel);
#यदि_अन्तः
      }अन्यथा{
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      यदि( db->mallocFailed ) गच्छ whereBeginError;
    }
    addrExplain = sqlite3_Where_Explain_One_अवलोकनम्(
        pParse, pTabList, pLevel, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3_Where_संहिता_One_Loop_Start(pParse,v,pWInfo,ii,pLevel,notReady);
    pWInfo->iContinue = pLevel->addrCont;
    यदि( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_OR_SUBCLAUSE)==0 ){
      sqlite3_Where_सङ्कलय_अवलोकनम्_स्थितिः(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  pWInfo->i_अन्तः_Where = sqlite3VdbeCurrentAddr(v);
  प्रत्यागम pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  यदि( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
#यदि_परिभाषितम् WHERETRACE_ENABLED
  /* Prevent harmless compiler warnings about debugging routines
  ** being declared but never used */
  sqlite3ShowWhereLoopList(0);
#यदि_अन्तः /* WHERETRACE_ENABLED */
  प्रत्यागम 0;
}

/*
** Part of sqlite3WhereEnd() will rewrite opcodes to reference the
** index rather than the main table.  In SQLITE_DEBUG mode, we want
** to trace those changes if PRAGMA vdbe_addoptrace=on.  This routine
** does that.
*/
#यदि_अपरिभाषितम् SQLITE_DEBUG
# define OpcodeRewriteTrace(D,K,P) /* no-op */
#अन्यथा
# define OpcodeRewriteTrace(D,K,P) sqlite3WhereOpcodeRewriteTrace(D,K,P)
  स्थिर शून्य sqlite3WhereOpcodeRewriteTrace(
    sqlite3 *db,
    पूर्णाङ्क pc,
    VdbeOp *pOp
  ){
    यदि( (db->flags & SQLITE_VdbeAddopTrace)==0 ) प्रत्यागम;
    sqlite3VdbePrintOp(0, pc, pOp);
    sqlite3_Show_Where_पदांशः(0); /* So compiler won't complain about unused func */
  }
#यदि_अन्तः

/*
** Generate the end of the WHERE loop.  See comments on
** sqlite3WhereBegin() for additional information.
*/
शून्य sqlite3_Where_अन्तः(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क i_अन्तः = sqlite3VdbeCurrentAddr(v);
  पूर्णाङ्क nRJ = 0;
#यदि_अपरिभाषितम् SQLITE_DISABLE_SKIPAHEAD_DISTINCT
  पूर्णाङ्क addrSeek = 0;
#यदि_अन्तः

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  प्रति(i=pWInfo->nLevel-1; i>=0; i--){
    पूर्णाङ्क addr;
    pLevel = &pWInfo->a[i];
    यदि( pLevel->pRJ ){
      /* Terminate the subroutine that forms the interior of the loop of
      ** the RIGHT JOIN table */
      Where_दक्षिणः_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeResolveLabel(v, pLevel->addrCont);
      /* Replace addrCont with a new label that will never be used, just so
      ** the subsequent call to resolve pLevel->addrCont will have something
      ** to resolve. */
      pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
      pRJ->अन्तः_Subrtn = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Return, pRJ->regReturn, pRJ->addrSubrtn, 1);
      VdbeCoverage(v);
      nRJ++;
    }
    pLoop = pLevel->pWLoop;
    यदि( pLevel->op!=OP_Noop ){
#यदि_अपरिभाषितम् SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      सूचकांक *pIdx;
      पूर्णाङ्क n;
      यदि( pWInfo->eDistinct==WHERE_DISTINCT_ORDERED
       && i==pWInfo->nLevel-1  /* Ticket [ef9318757b152e3] 2017-10-21 */
       && (pLoop->wsFlags & WHERE_अनुक्रमितम्)!=0
       && (pIdx = pLoop->u.btree.p_सूचकांक)->hasStat1
       && (n = pLoop->u.btree.nDistinctCol)>0
       && pIdx->aiRowLogEst[n]>=36
      ){
        पूर्णाङ्क r1 = pParse->nMem+1;
        पूर्णाङ्क j, op;
        पूर्णाङ्क addrIfNull = 0; /* Init to avoid false-positive compiler warning */
        यदि( pLevel->i_वामः_Join ){
          addrIfNull = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IfNullRow, pLevel->iIdxCur, r1);
        }
        प्रति(j=0; j<n; j++){
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, pLevel->iIdxCur, j, r1+j);
        }
        pParse->nMem += n+1;
        op = pLevel->op==OP_पूर्व ? OP_SeekLT : OP_SeekGT;
        addrSeek = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, op, pLevel->iIdxCur, 0, r1, n);
        VdbeCoverageIf(v, op==OP_SeekLT);
        VdbeCoverageIf(v, op==OP_SeekGT);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Goto, 1, pLevel->p2);
        यदि( pLevel->i_वामः_Join ){
          sqlite3_Vdbe_Jump_अत्र(v, addrIfNull);
        }
      }
#यदि_अन्तः /* SQLITE_DISABLE_SKIPAHEAD_DISTINCT */
    }
    यदि( pTabList->a[pLevel->iFrom].fg.fromExists ){
      /* This is an EXISTS-to-JOIN optimization loop. If this loop sees a 
      ** successful row, it should break out of itself. */
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Goto, 0, pLevel->addrBrk);
      VdbeComment((v, "EXISTS break %d", i));
    }अन्यथा यदि( pLevel->addrDeleteCont ){
      /* An inner loop matched no rows on this auto-index driven loop and
      ** there are outer loops.  So delete the auto-index entry to avoid
      ** future unnecessary work. */
      पूर्णाङ्क iBypass = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Goto);
      प्रतिज्ञा( pLoop->wsFlags & WHERE_AUTO_सूचकांक );
      sqlite3VdbeResolveLabel(v, pLevel->addrDeleteCont);
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Delete, pLevel->iIdxCur);
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
      VdbeComment((v, "delete useless auto-index entry"));
      sqlite3_Vdbe_Jump_अत्र(v, iBypass);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    यदि( pLevel->op!=OP_Noop ){
      sqlite3_Vdbe_सङ्कलय_Op3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_अग्र);
      VdbeCoverageIf(v, pLevel->op==OP_पूर्व);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
#यदि_अपरिभाषितम् SQLITE_DISABLE_SKIPAHEAD_DISTINCT
      यदि( addrSeek ){
        sqlite3_Vdbe_Jump_अत्र(v, addrSeek);
        addrSeek = 0;
      }
#यदि_अन्तः
      यदि( pLevel->regBignull ){
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_DecrJumpZero, pLevel->regBignull, pLevel->p2-1);
        VdbeCoverage(v);
      }
    }
    यदि( pLevel->addrNxt!=pLevel->addrBrk ){
      /* This occurs in three cases - (a) for a LIKE/GLOB scan that must visit
      ** ranges of text and blob values separately, and (b) when there is
      ** an IN(...) constraint driving the index, and (c) when a prior error
      ** such as an OOM has left us in an unusual state. This is asserted()
      ** in the next block, inside the #ifndef. */
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
    }
#यदि_अपरिभाषितम् SQLITE_LIKE_DOESNT_मेलनम्_BLOBS
    प्रतिज्ञा( (pLevel->addrNxt==pLevel->addrBrk)
         || (pLevel->addrLikeRep)                                    /* (a) */
         || ((pLoop->wsFlags & WHERE_अन्तर्_ABLE) && pLevel->u.अन्तर्.n_अन्तर्>0) /* (b) */
         || pWInfo->pParse->nErr>0                                   /* (c) */
    );
    यदि( pLevel->addrLikeRep ){
      /* The LIKE optimization loop is nested inside any IN(...) and
      ** skip-scan loops, so it is closed here, before either of those. */
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_DecrJumpZero, (पूर्णाङ्क)(pLevel->iLikeRepCntr>>1),
                        pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
#यदि_अन्तः
    यदि( (pLoop->wsFlags & WHERE_अन्तर्_ABLE)!=0 && pLevel->u.अन्तर्.n_अन्तर्>0 ){
      संरचना अन्तर्_Loop *p_अन्तर्;
      पूर्णाङ्क j;
      प्रति(j=pLevel->u.अन्तर्.n_अन्तर्, p_अन्तर्=&pLevel->u.अन्तर्.a_अन्तर्_Loop[j-1]; j>0; j--, p_अन्तर्--){
        प्रतिज्ञा( sqlite3VdbeGetOp(v, p_अन्तर्->addr_अन्तर्_शिखरम्+1)->opcode==OP_IsNull
                 || pParse->db->mallocFailed );
        sqlite3_Vdbe_Jump_अत्र(v, p_अन्तर्->addr_अन्तर्_शिखरम्+1);
        यदि( p_अन्तर्->e_अन्तः_Loop_Op!=OP_Noop ){
          यदि( p_अन्तर्->n_उपसर्गः ){
            पूर्णाङ्क b_Early_बहिर्गमनम् =
                (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
                 && (pLoop->wsFlags & WHERE_अन्तर्_EARLYOUT)!=0;
            यदि( pLevel->i_वामः_Join ){
              /* For LEFT JOIN queries, cursor pIn->iCur might have been
              ** disabled by OP_NullRow.  This occurs for WHERE clauses such
              ** as "a = ? AND b IN (...)", where the index is on (a, b).
              ** When the OP_IfNullRow jump is taken, the pIn->iBase register
              ** is NULLed out as a side effect.  But as that register is not
              ** used when the jump is taken, the side-effect is harmless. */
              sqlite3_Vdbe_सङ्कलय_Op3(v, OP_IfNullRow, p_अन्तर्->iCur,
                  sqlite3VdbeCurrentAddr(v) + 2 + b_Early_बहिर्गमनम्, p_अन्तर्->i_आधारः);
              VdbeCoverage(v);
            }
            यदि( b_Early_बहिर्गमनम् ){
              sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IfNoHope, pLevel->iIdxCur,
                  sqlite3VdbeCurrentAddr(v)+2,
                  p_अन्तर्->i_आधारः, p_अन्तर्->n_उपसर्गः);
              VdbeCoverage(v);
              /* Retarget the OP_IsNull against the left operand of IN so
              ** it jumps past the OP_IfNoHope.  This is because the
              ** OP_IsNull also bypasses the OP_Affinity opcode that is
              ** required by OP_IfNoHope. */
              sqlite3_Vdbe_Jump_अत्र(v, p_अन्तर्->addr_अन्तर्_शिखरम्+1);
            }
          }
          sqlite3_Vdbe_सङ्कलय_Op2(v, p_अन्तर्->e_अन्तः_Loop_Op, p_अन्तर्->iCur, p_अन्तर्->addr_अन्तर्_शिखरम्);
          VdbeCoverage(v);
          VdbeCoverageIf(v, p_अन्तर्->e_अन्तः_Loop_Op==OP_पूर्व);
          VdbeCoverageIf(v, p_अन्तर्->e_अन्तः_Loop_Op==OP_अग्र);
        }
        sqlite3_Vdbe_Jump_अत्र(v, p_अन्तर्->addr_अन्तर्_शिखरम्-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    यदि( pLevel->pRJ ){
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Return, pLevel->pRJ->regReturn, 0, 1);
      VdbeCoverage(v);
    }
    यदि( pLevel->addrSkip ){
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.p_सूचकांक->zName));
      sqlite3_Vdbe_Jump_अत्र(v, pLevel->addrSkip);
      sqlite3_Vdbe_Jump_अत्र(v, pLevel->addrSkip-2);
    }
    यदि( pLevel->i_वामः_Join ){
      पूर्णाङ्क ws = pLoop->wsFlags;
      addr = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IfPos, pLevel->i_वामः_Join); VdbeCoverage(v);
      प्रतिज्ञा( (ws & WHERE_IDX_ONLY)==0 || (ws & WHERE_अनुक्रमितम्)!=0 );
      यदि( (ws & WHERE_IDX_ONLY)==0 ){
        SrcItem *pSrc = &pTabList->a[pLevel->iFrom];
        प्रतिज्ञा( pLevel->iTabCur==pSrc->iCursor );
        यदि( pSrc->fg.द्वारा_Coroutine ){
          पूर्णाङ्क m, n;
          प्रतिज्ञा( pSrc->fg.isSubquery );
          n = pSrc->u4.pSubq->reg_परिणाम;
          प्रतिज्ञा( pSrc->pSTab!=0 );
          m = pSrc->pSTab->nCol;
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Null, 0, n, n+m-1);
        }
        sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, pLevel->iTabCur);
      }
      यदि( (ws & WHERE_अनुक्रमितम्)
       || ((ws & WHERE_MULTI_OR) && pLevel->u.pCoveringIdx)
      ){
        यदि( ws & WHERE_MULTI_OR ){
          सूचकांक *pIx = pLevel->u.pCoveringIdx;
          पूर्णाङ्क iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pIx->pSchema);
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_ReopenIdx, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3_Vdbe_समुच्चयः_P4_कुञ्चिका_Info(pParse, pIx);
        }
        sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, pLevel->iIdxCur);
      }
      यदि( ws & WHERE_अन्तर्_ABLE ){
        पूर्णाङ्क m;
        प्रति(m=0; m<pLevel->u.अन्तर्.n_अन्तर्; m++){
          संरचना अन्तर्_Loop *p_अन्तर् = &pLevel->u.अन्तर्.a_अन्तर्_Loop[m];
          यदि( p_अन्तर्->e_अन्तः_Loop_Op!=OP_Noop ){
            sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, p_अन्तर्->iCur);
          }
        }
      }
      यदि( pLevel->op==OP_Return ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, pLevel->p1, pLevel->addrFirst);
      }अन्यथा{
        sqlite3VdbeGoto(v, pLevel->addrFirst);
      }
      sqlite3_Vdbe_Jump_अत्र(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pSTab->zName));
  }

  प्रतिज्ञा( pWInfo->nLevel<=pTabList->nSrc );
  प्रति(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    पूर्णाङ्क k, चरमः_2;
    VdbeOp *pOp, *p_चरमः_Op;
    सूचकांक *pIdx = 0;
    SrcItem *pTabItem = &pTabList->a[pLevel->iFrom];
    सारणी *pTab = pTabItem->pSTab;
    प्रतिज्ञा( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* Do RIGHT JOIN processing.  Generate code that will output the
    ** unmatched rows of the right operand of the RIGHT JOIN with
    ** all of the columns of the left operand set to NULL.
    */
    यदि( pLevel->pRJ ){
      sqlite3_Where_दक्षिणः_Join_Loop(pWInfo, i, pLevel);
      अनुवर्त;
    }

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    यदि( pTabItem->fg.द्वारा_Coroutine ){
      testcase( pParse->db->mallocFailed );
      प्रतिज्ञा( pTabItem->fg.isSubquery );
      प्रतिज्ञा( pTabItem->u4.pSubq->reg_परिणाम>=0 );
      translate_Column_गन्तव्यम्_Copy(pParse, pLevel->addrBody, pLevel->iTabCur,
                            pTabItem->u4.pSubq->reg_परिणाम, 0);
      अनुवर्त;
    }

    /* If this scan uses an index, make VDBE code substitutions to read data
    ** from the index instead of from the table where possible.  In some cases
    ** this optimization prevents the table from ever being read, which can
    ** yield a significant performance boost.
    **
    ** Calls to the code generator in between sqlite3WhereBegin and
    ** sqlite3WhereEnd will have created code that references the table
    ** directly.  This loop scans all that code looking for opcodes
    ** that reference the table and converts them into opcodes that
    ** reference the index.
    */
    यदि( pLoop->wsFlags & (WHERE_अनुक्रमितम्|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.p_सूचकांक;
    }अन्यथा यदि( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCoveringIdx;
    }
    यदि( pIdx
     && !db->mallocFailed
    ){
      यदि( pWInfo->eOnePass==ONEPASS_OFF || !HasRowid(pIdx->p_सारणी) ){
        चरमः_2 = i_अन्तः;
      }अन्यथा{
        चरमः_2 = pWInfo->i_अन्तः_Where;
      }
      यदि( pIdx->b_Has_व्यञ्जकम् ){
        अनुक्रमितम्_व्यञ्जकम् *p = pParse->pIdxEpr;
        यावत्( p ){
          यदि( p->iIdxCur==pLevel->iIdxCur ){
#यदि_परिभाषितम् WHERETRACE_ENABLED
            यदि( sqlite3WhereTrace & 0x200 ){
              sqlite3DebugPrintf("Disable pParse->pIdxEpr term {%d,%d}\n",
                                  p->iIdxCur, p->iIdxCol);
              यदि( sqlite3WhereTrace & 0x5000 ) sqlite3_Show_व्यञ्जकम्(p->p_व्यञ्जकम्);
            }
#यदि_अन्तः
            p->i_दत्त_Cur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;
#यदि_परिभाषितम् SQLITE_DEBUG
      यदि( db->flags & SQLITE_VdbeAddopTrace ){
        प्रारूप_लिख("TRANSLATE cursor %d->%d in opcode range %d..%d\n",
                pLevel->iTabCur, pLevel->iIdxCur, k, चरमः_2-1);
      }
      /* Proof that the "+1" on the k value above is safe */
      pOp = sqlite3VdbeGetOp(v, k - 1);
      प्रतिज्ञा( pOp->opcode!=OP_Column || pOp->p1!=pLevel->iTabCur );
      प्रतिज्ञा( pOp->opcode!=OP_Rowid  || pOp->p1!=pLevel->iTabCur );
      प्रतिज्ञा( pOp->opcode!=OP_IfNullRow || pOp->p1!=pLevel->iTabCur );
#यदि_अन्तः
      pOp = sqlite3VdbeGetOp(v, k);
      p_चरमः_Op = pOp + (चरमः_2 - k);
      प्रतिज्ञा( pOp<=p_चरमः_Op );
      कुरु{
        यदि( pOp->p1!=pLevel->iTabCur ){
          /* no-op */
        }अन्यथा यदि( pOp->opcode==OP_Column
#यदि_परिभाषितम् SQLITE_ENABLE_विस्थापनम्_SQL_FUNC
         || pOp->opcode==OP_विस्थापनम्
#यदि_अन्तः
        ){
          पूर्णाङ्क x = pOp->p2;
          प्रतिज्ञा( pIdx->p_सारणी==pTab );
#यदि_परिभाषितम् SQLITE_ENABLE_विस्थापनम्_SQL_FUNC
          यदि( pOp->opcode==OP_विस्थापनम् ){
            /* Do not need to translate the column number */
          }अन्यथा
#यदि_अन्तः
          यदि( !HasRowid(pTab) ){
            सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
            x = pPk->aiColumn[x];
            प्रतिज्ञा( x>=0 );
          }अन्यथा{
            testcase( x!=sqlite3_Storage_Column_गन्तव्यम्_सारणी(pTab,x) );
            x = sqlite3_Storage_Column_गन्तव्यम्_सारणी(pTab,x);
          }
          x = sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(pIdx, x);
          यदि( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
            OpcodeRewriteTrace(db, k, pOp);
          }अन्यथा यदि( pLoop->wsFlags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
            यदि( pLoop->wsFlags & WHERE_IDX_ONLY ){
              /* An error. pLoop is supposed to be a covering index loop,
              ** and yet the VM code refers to a column of the table that 
              ** is not part of the index.  */
              sqlite3_Error_सन्देशः(pParse, "internal query planner error");
              pParse->rc = SQLITE_आन्तरिकम्;
            }अन्यथा{
              /* The WHERE_EXPRIDX flag is set by the planner when it is likely
              ** that pLoop is a covering index loop, but it is not possible
              ** to be 100% sure. In this case, any OP_Explain opcode
              ** corresponding to this loop describes the index as a "COVERING
              ** INDEX". But, pOp proves that pLoop is not actually a covering 
              ** index loop. So clear the WHERE_EXPRIDX flag and rewrite the
              ** text that accompanies the OP_Explain opcode, if any.  */
              pLoop->wsFlags &= ~WHERE_EXPRIDX;
              sqlite3_Where_सङ्कलय_Explain_Text(pParse,
                  pLevel->addrBody-1,
                  pTabList,
                  pLevel,
                  pWInfo->wctrlFlags
              );
            }
          }
        }अन्यथा यदि( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
          OpcodeRewriteTrace(db, k, pOp);
        }अन्यथा यदि( pOp->opcode==OP_IfNullRow ){
          pOp->p1 = pLevel->iIdxCur;
          OpcodeRewriteTrace(db, k, pOp);
        }
#यदि_परिभाषितम् SQLITE_DEBUG
        k++;
#यदि_अन्तः
      }यावत्( (++pOp)<p_चरमः_Op );
#यदि_परिभाषितम् SQLITE_DEBUG
      यदि( db->flags & SQLITE_VdbeAddopTrace ) प्रारूप_लिख("TRANSLATE complete\n");
#यदि_अन्तः
    }
  }

  /* The "break" point is here, just past the end of the outer loop.
  ** Set it.
  */
  sqlite3VdbeResolveLabel(v, pWInfo->iBreak);

  /* Final cleanup
  */
  pParse->nQueryLoop = pWInfo->savedNQueryLoop;
  whereInfoFree(db, pWInfo);
  pParse->withinRJSubrtn -= nRJ;
  प्रत्यागम;
}
