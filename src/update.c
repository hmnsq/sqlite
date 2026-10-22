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
** This file contains C code routines that are called by the parser
** to handle UPDATE statements.
*/
#अन्तर्भाव "sqliteInt.h"

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
स्थिर शून्य update_Virtual_सारणी(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  सारणी *pTab,         /* The virtual table */
  व्यञ्जकम्_List *pChanges,  /* The columns to change in the UPDATE statement */
  व्यञ्जकम् *p_Rowid_व्यञ्जकम्,    /* Expression used to recompute the rowid */
  पूर्णाङ्क *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  व्यञ्जकम् *pWhere,        /* WHERE clause of the UPDATE statement */
  पूर्णाङ्क onError          /* ON CONFLICT strategy */
);
#यदि_अन्तः /* SQLITE_OMIT_VIRTUALTABLE */

/*
** The most recently coded instruction was an OP_Column to retrieve the
** i-th column of table pTab. This routine sets the P4 parameter of the
** OP_Column to the default value, if any.
**
** The default value of a column is specified by a DEFAULT clause in the
** column definition. This was either supplied by the user when the table
** was created, or added later to the table definition by an ALTER TABLE
** command. If the latter, then the row-records in the table btree on disk
** may not contain a value for the column and the default value, taken
** from the P4 parameter of the OP_Column instruction, is returned instead.
** If the former, then all row-records are guaranteed to include a value
** for the column and the P4 value is not required.
**
** Column definitions created by an ALTER TABLE command may only have
** literal default values specified: a number, null or a string. (If a more
** complicated default expression value was provided, it is evaluated
** when the ALTER TABLE is executed and one of the literal values written
** into the sqlite_schema table.)
**
** Therefore, the P4 parameter is only required if the default value for
** the column is a literal number, string or null. The sqlite3ValueFromExpr()
** function is capable of transforming these types of expressions into
** sqlite3_value objects.
**
** If column as REAL affinity and the table is an ordinary b-tree table
** (not a virtual table) then the value might have been stored as an
** integer.  In that case, add an OP_RealAffinity opcode to make sure
** it has been converted into REAL.
*/
स्थिर SQLITE_NOINLINE शून्य columnDefaultUncommonCase(
  Vdbe *v,          /* Byte code under construction */
  सारणी *pTab,      /* The table */
  Column *pCol      /* Which column of the table */
){
  sqlite3_मानम् *p_मानम् = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  प्रतिज्ञा( !IsView(pTab) );
  sqlite3_मानम्_From_व्यञ्जकम्(sqlite3VdbeDb(v),
                       sqlite3_Column_व्यञ्जकम्_Auth(pTab,pCol,pParse), enc,
                       pCol->affinity, &p_मानम्);
  यदि( p_मानम् ){
    sqlite3VdbeAppendP4(v, p_मानम्, P4_MEM);
  }
}
शून्य sqlite3ColumnDefault(
  Vdbe *v,          /* Byte code under construction */
  सारणी *pTab,      /* The table */
  पूर्णाङ्क i,            /* Which column of the table */
  पूर्णाङ्क iReg          /* Register in which results are stored */
){
  Column *pCol;
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( pTab->nCol>i );
  pCol = &pTab->aCol[i];
  यदि( pCol->iDflt ){
    columnDefaultUncommonCase(v,pTab,pCol);
  }
#यदि_अपरिभाषितम् SQLITE_OMIT_FLOATING_POINT
  यदि( pCol->affinity==SQLITE_AFF_REAL ){
    sqlite3_Vdbe_सङ्कलय_Op1(v, OP_RealAffinity, iReg);
  }
#यदि_अन्तः
}

/*
** Check to see if column iCol of index pIdx references any of the
** columns defined by aXRef and chngRowid.  Return true if it does
** and false if not.  This is an optimization.  False-positives are a
** performance degradation, but false-negatives can result in a corrupt
** index and incorrect answers.
**
** aXRef[j] will be non-negative if column j of the original table is
** being updated.  chngRowid will be true if the rowid of the table is
** being updated.
*/
स्थिर पूर्णाङ्क सूचकांक_Column_Is_Being_Updated(
  सूचकांक *pIdx,      /* The index to check */
  पूर्णाङ्क iCol,         /* Which column of the index to check */
  पूर्णाङ्क *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  पूर्णाङ्क chngRowid     /* true if the rowid is being updated */
){
  i16 iIdxCol = pIdx->aiColumn[iCol];
  प्रतिज्ञा( iIdxCol!=XN_ROWID ); /* Cannot index rowid */
  यदि( iIdxCol>=0 ){
    प्रत्यागम aXRef[iIdxCol]>=0;
  }
  प्रतिज्ञा( iIdxCol==XN_व्यञ्जकम् );
  प्रतिज्ञा( pIdx->a_Col_व्यञ्जकम्!=0 );
  प्रतिज्ञा( pIdx->a_Col_व्यञ्जकम्->a[iCol].p_व्यञ्जकम्!=0 );
  प्रत्यागम sqlite3_व्यञ्जकम्_References_Updated_Column(pIdx->a_Col_व्यञ्जकम्->a[iCol].p_व्यञ्जकम्,
                                            aXRef,chngRowid);
}

/*
** Check to see if index pIdx is a partial index whose conditional
** expression might change values due to an UPDATE.  Return true if
** the index is subject to change and false if the index is guaranteed
** to be unchanged.  This is an optimization.  False-positives are a
** performance degradation, but false-negatives can result in a corrupt
** index and incorrect answers.
**
** aXRef[j] will be non-negative if column j of the original table is
** being updated.  chngRowid will be true if the rowid of the table is
** being updated.
*/
स्थिर पूर्णाङ्क सूचकांक_Where_Clause_Might_Change(
  सूचकांक *pIdx,      /* The index to check */
  पूर्णाङ्क *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  पूर्णाङ्क chngRowid     /* true if the rowid is being updated */
){
  यदि( pIdx->pPartIdxWhere==0 ) प्रत्यागम 0;
  प्रत्यागम sqlite3_व्यञ्जकम्_References_Updated_Column(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
स्थिर व्यञ्जकम् *व्यञ्जकम्_Row_Column(Parse *pParse, पूर्णाङ्क iCol){
  व्यञ्जकम् *pRet = sqlite3PExpr(pParse, TK_ROW, 0, 0);
  यदि( pRet ) pRet->iColumn = iCol+1;
  प्रत्यागम pRet;
}

/*
** Assuming both the pLimit and pOrderBy parameters are NULL, this function
** generates VM code to run the query:
**
**   SELECT <other-columns>, pChanges FROM pTabList WHERE pWhere
**
** and write the results to the ephemeral table already opened as cursor
** iEph. None of pChanges, pTabList or pWhere are modified or consumed by
** this function, they must be deleted by the caller.
**
** Or, if pLimit and pOrderBy are not NULL, and pTab is not a view:
**
**   SELECT <other-columns>, pChanges FROM pTabList
**   WHERE pWhere
**   GROUP BY <other-columns>
**   ORDER BY pOrderBy LIMIT pLimit
**
** If pTab is a view, the GROUP BY clause is omitted.
**
** Exactly how results are written to table iEph, and exactly what
** the <other-columns> in the query above are is determined by the type
** of table pTabList->a[0].pTab.
**
** If the table is a WITHOUT ROWID table, then argument pPk must be its
** PRIMARY KEY. In this case <other-columns> are the primary key columns
** of the table, in order. The results of the query are written to ephemeral
** table iEph as index keys, using OP_IdxInsert.
**
** If the table is actually a view, then <other-columns> are all columns of
** the view. The results are written to the ephemeral table iEph as records
** with automatically assigned integer keys.
**
** If the table is a virtual or ordinary intkey table, then <other-columns>
** is its rowid. For a virtual table, the results are written to iEph as
** records with automatically assigned integer keys For intkey tables, the
** rowid value in <other-columns> is used as the integer key, and the
** remaining fields make up the table record.
*/
स्थिर शून्य updateFromSelect(
  Parse *pParse,                  /* Parse context */
  पूर्णाङ्क iEph,                       /* Cursor for open eph. table */
  सूचकांक *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  व्यञ्जकम्_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  व्यञ्जकम् *pWhere,                   /* WHERE clause for query */
  व्यञ्जकम्_List *p_क्रमः_By,             /* ORDER BY clause */
  व्यञ्जकम् *p_सीमा                    /* LIMIT clause */
){
  पूर्णाङ्क i;
  SelectDest dest;
  Select *pSelect = 0;
  व्यञ्जकम्_List *pList = 0;
  व्यञ्जकम्_List *pGrp = 0;
  व्यञ्जकम् *pLimit2 = 0;
  व्यञ्जकम्_List *p_क्रमः_By2 = 0;
  sqlite3 *db = pParse->db;
  सारणी *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  व्यञ्जकम् *pWhere2;
  पूर्णाङ्क eDest;

#यदि_परिभाषितम् SQLITE_ENABLE_UPDATE_DELETE_सीमा
  यदि( p_क्रमः_By && p_सीमा==0 ) {
    sqlite3_Error_सन्देशः(pParse, "ORDER BY without LIMIT on UPDATE");
    प्रत्यागम;
  }
  p_क्रमः_By2 = sqlite3_व्यञ्जकम्_List_Dup(db, p_क्रमः_By, 0);
  pLimit2 = sqlite3_व्यञ्जकम्_Dup(db, p_सीमा, 0);
#अन्यथा
  अप्रयुक्तम्_PARAMETER(p_क्रमः_By);
  अप्रयुक्तम्_PARAMETER(p_सीमा);
#यदि_अन्तः

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_व्यञ्जकम्_Dup(db, pWhere, 0);

  प्रतिज्ञा( pTabList->nSrc>1 );
  यदि( pSrc ){
    प्रतिज्ञा( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  यदि( pPk ){
    प्रति(i=0; i<pPk->n_कुञ्चिका_Col; i++){
      व्यञ्जकम् *pNew = व्यञ्जकम्_Row_Column(pParse, pPk->aiColumn[i]);
#यदि_परिभाषितम् SQLITE_ENABLE_UPDATE_DELETE_सीमा
      यदि( p_सीमा ){
        pGrp = sqlite3_व्यञ्जकम्_List_Append(pParse, pGrp, sqlite3_व्यञ्जकम्_Dup(db, pNew, 0));
      }
#यदि_अन्तः
      pList = sqlite3_व्यञ्जकम्_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_सारणी : SRT_Upfrom;
  }अन्यथा यदि( IsView(pTab) ){
    प्रति(i=0; i<pTab->nCol; i++){
      pList = sqlite3_व्यञ्जकम्_List_Append(pParse, pList, व्यञ्जकम्_Row_Column(pParse, i));
    }
    eDest = SRT_सारणी;
  }अन्यथा{
    eDest = IsVirtual(pTab) ? SRT_सारणी : SRT_Upfrom;
    pList = sqlite3_व्यञ्जकम्_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
#यदि_परिभाषितम् SQLITE_ENABLE_UPDATE_DELETE_सीमा
    यदि( p_सीमा ){
      pGrp = sqlite3_व्यञ्जकम्_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
    }
#यदि_अन्तः
  }
  प्रतिज्ञा( pChanges!=0 || pParse->db->mallocFailed );
  यदि( pChanges ){
    प्रति(i=0; i<pChanges->n_व्यञ्जकम्; i++){
      pList = sqlite3_व्यञ्जकम्_List_Append(pParse, pList,
          sqlite3_व्यञ्जकम्_Dup(db, pChanges->a[i].p_व्यञ्जकम्, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, p_क्रमः_By2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  यदि( pSelect ) pSelect->selFlags |= SF_क्रमः_By_Reqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_कुञ्चिका_Col : -1);
  sqlite3Select(pParse, pSelect, &dest);
  sqlite3SelectDelete(db, pSelect);
}

/*
** Process an UPDATE statement.
**
**   UPDATE OR IGNORE tbl SET a=b, c=d FROM tbl2... WHERE e<5 AND f NOT NULL;
**          \_______/ \_/     \______/      \_____/       \________________/
**           onError   |      pChanges         |                pWhere
**                     \_______________________/
**                               pTabList
*/
शून्य sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  व्यञ्जकम्_List *pChanges,    /* Things to be changed */
  व्यञ्जकम् *pWhere,          /* The WHERE clause.  May be null */
  पूर्णाङ्क onError,           /* How to handle constraint errors */
  व्यञ्जकम्_List *p_क्रमः_By,    /* ORDER BY clause. May be null */
  व्यञ्जकम् *p_सीमा,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  पूर्णाङ्क i, j, k;           /* Loop counters */
  सारणी *pTab;           /* The table to be updated */
  पूर्णाङ्क addr_शिखरम् = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  सूचकांक *pIdx;           /* For looping over indices */
  सूचकांक *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  पूर्णाङ्क nIdx;              /* Number of indices that need updating */
  पूर्णाङ्क nAllIdx;           /* Total number of indexes */
  पूर्णाङ्क i_आधारः_Cur;          /* Base cursor number */
  पूर्णाङ्क i_दत्त_Cur;          /* Cursor for the canonical data btree */
  पूर्णाङ्क iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  पूर्णाङ्क *aRegIdx = 0;      /* Registers for to each index and the main table */
  पूर्णाङ्क *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *a_गन्तव्यम्_Open;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_कुञ्चिका;            /* Either chngPk or chngRowid */
  व्यञ्जकम् *p_Rowid_व्यञ्जकम् = 0;  /* Expression defining the new record number */
  पूर्णाङ्क i_Rowid_व्यञ्जकम् = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  पूर्णाङ्क iDb;               /* Database containing the table being updated */
  पूर्णाङ्क eOnePass;          /* ONEPASS_XXX value from where.c */
  पूर्णाङ्क hasFK;             /* True if foreign key processing is required */
  पूर्णाङ्क labelBreak;        /* Jump here to break out of UPDATE loop */
  पूर्णाङ्क labelContinue;     /* Jump here to continue next step of UPDATE loop */
  पूर्णाङ्क flags;             /* Flags for sqlite3WhereBegin() */

#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
  पूर्णाङ्क isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  पूर्णाङ्क tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#यदि_अन्तः
  पूर्णाङ्क newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  पूर्णाङ्क iEph = 0;          /* Ephemeral table holding all primary key values */
  पूर्णाङ्क n_कुञ्चिका = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  पूर्णाङ्क aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  पूर्णाङ्क addrOpen = 0;      /* Address of OP_OpenEphemeral */
  पूर्णाङ्क iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  पूर्णाङ्क bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  पूर्णाङ्क bFinishSeek = 1;   /* The OP_FinishSeek opcode is needed */
  पूर्णाङ्क nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  पूर्णाङ्क reg_Row_गणनम् = 0;   /* A count of rows changed */
  पूर्णाङ्क regOldRowid = 0;   /* The old rowid */
  पूर्णाङ्क regNewRowid = 0;   /* The new rowid */
  पूर्णाङ्क regNew = 0;        /* Content of the NEW.* table in triggers */
  पूर्णाङ्क regOld = 0;        /* Content of OLD.* table in triggers */
  पूर्णाङ्क reg_Row_समुच्चयः = 0;     /* Rowset of rows to be updated */
  पूर्णाङ्क reg_कुञ्चिका = 0;        /* composite PRIMARY KEY value */

  स्मृतिं_पूरय(&sContext, 0, परिमाण(sContext));
  db = pParse->db;
  प्रतिज्ञा( db->pParse==pParse );
  यदि( pParse->nErr ){
    गच्छ update_cleanup;
  }
  प्रतिज्ञा( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  यदि( pTab==0 ) गच्छ update_cleanup;
  iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  प्रतिज्ञा( pTrigger || tmask==0 );
#अन्यथा
# define pTrigger 0
# define isView 0
# define tmask 0
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#यदि_अन्तः

#यदि TREETRACE_ENABLED
  यदि( sqlite3_वृक्षः_Trace & 0x10000 ){
    sqlite3_वृक्षः_View_पङ्क्तिः(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_वृक्षः_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, p_क्रमः_By, p_सीमा, pUpsert, pTrigger);
  }
#यदि_अन्तः

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_व्यञ्जकम् : 0;
  प्रतिज्ञा( nChangeFrom==0 || pUpsert==0 );

#यदि_परिभाषितम् SQLITE_ENABLE_UPDATE_DELETE_सीमा
  यदि( !isView && nChangeFrom==0 ){
    pWhere = sqlite3_सीमा_Where(
        pParse, pTabList, pWhere, p_क्रमः_By, p_सीमा, "UPDATE"
    );
    p_क्रमः_By = 0;
    p_सीमा = 0;
  }
#यदि_अन्तः

  यदि( sqlite3_View_Get_Column_नामानि(pParse, pTab) ){
    गच्छ update_cleanup;
  }
  यदि( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    गच्छ update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  i_आधारः_Cur = i_दत्त_Cur = pParse->nTab++;
  iIdxCur = i_दत्त_Cur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
  testcase( pPk!=0 && pPk!=pTab->p_सूचकांक );
  प्रति(nIdx=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, nIdx++){
    यदि( pPk==pIdx ){
      i_दत्त_Cur = pParse->nTab;
    }
    pParse->nTab++;
  }
  यदि( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    i_दत्त_Cur = pUpsert->i_दत्त_Cur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = i_आधारः_Cur;
  }
  pTabList->a[0].iCursor = i_दत्त_Cur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, परिमाण(पूर्णाङ्क) * (pTab->nCol+nIdx+1) + nIdx+2 );
  यदि( aXRef==0 ) गच्छ update_cleanup;
  aRegIdx = aXRef+pTab->nCol;
  a_गन्तव्यम्_Open = (u8*)(aRegIdx+nIdx+1);
  स्मृतिं_पूरय(a_गन्तव्यम्_Open, 1, nIdx+1);
  a_गन्तव्यम्_Open[nIdx+1] = 0;
  प्रति(i=0; i<pTab->nCol; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  स्मृतिं_पूरय(&sNC, 0, परिमाण(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  यदि( v==0 ) गच्छ update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  प्रति(i=0; i<pChanges->n_व्यञ्जकम्; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    यदि( nChangeFrom==0 && sqlite3_Resolve_व्यञ्जकम्_नामानि(&sNC, pChanges->a[i].p_व्यञ्जकम्) ){
      गच्छ update_cleanup;
    }
    j = sqlite3_Column_सूचकांक(pTab, pChanges->a[i].zEName);
    यदि( j>=0 ){
      यदि( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_व्यञ्जकम् = pChanges->a[i].p_व्यञ्जकम्;
        i_Rowid_व्यञ्जकम् = i;
      }अन्यथा यदि( pPk && (pTab->aCol[j].colFlags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#यदि_अपरिभाषितम् SQLITE_OMIT_GENERATED_COLUMNS
      अन्यथा यदि( pTab->aCol[j].colFlags & COLFLAG_GENERATED ){
        testcase( pTab->aCol[j].colFlags & COLFLAG_VIRTUAL );
        testcase( pTab->aCol[j].colFlags & COLFLAG_STORED );
        sqlite3_Error_सन्देशः(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->aCol[j].zCnName);
        गच्छ update_cleanup;
      }
#यदि_अन्तः
      aXRef[j] = i;
    }अन्यथा{
      यदि( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_व्यञ्जकम् = pChanges->a[i].p_व्यञ्जकम्;
        i_Rowid_व्यञ्जकम् = i;
      }अन्यथा{
        sqlite3_Error_सन्देशः(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        गच्छ update_cleanup;
      }
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTHORIZATION
    {
      पूर्णाङ्क rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->aCol[j].zCnName,
                            db->aDb[iDb].zDbSName);
      यदि( rc!=SQLITE_OK ){
        यदि( rc==SQLITE_DENY ){
          गच्छ update_cleanup;
        }अन्यथा यदि( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }अन्यथा{
          aXRef[j] = -1;
        }
      }
    }
#यदि_अन्तः
  }
  प्रतिज्ञा( (chngRowid & chngPk)==0 );
  प्रतिज्ञा( chngRowid==0 || chngRowid==1 );
  प्रतिज्ञा( chngPk==0 || chngPk==1 );
  chng_कुञ्चिका = chngRowid + chngPk;

#यदि_अपरिभाषितम् SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  यदि( pTab->tabFlags & TF_HasGenerated ){
    पूर्णाङ्क b_प्रगतिः;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    कुरु{
      b_प्रगतिः = 0;
      प्रति(i=0; i<pTab->nCol; i++){
        यदि( aXRef[i]>=0 ) अनुवर्त;
        यदि( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 ) अनुवर्त;
        यदि( sqlite3_व्यञ्जकम्_References_Updated_Column(
                sqlite3_Column_व्यञ्जकम्(pTab, &pTab->aCol[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          b_प्रगतिः = 1;
        }
      }
    }यावत्( b_प्रगतिः );
  }
#यदि_अन्तः

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].col_प्रयुक्तम् = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_कुञ्चिका);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  यदि( onError==OE_Replace ) bReplace = 1;
  प्रति(nAllIdx=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, nAllIdx++){
    पूर्णाङ्क reg;
    यदि( chng_कुञ्चिका || hasFK>1 || pIdx==pPk
     || सूचकांक_Where_Clause_Might_Change(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    }अन्यथा{
      reg = 0;
      प्रति(i=0; i<pIdx->n_कुञ्चिका_Col; i++){
        यदि( सूचकांक_Column_Is_Being_Updated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->nColumn;
          यदि( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          भङ्ग;
        }
      }
    }
    यदि( reg==0 ) a_गन्तव्यम्_Open[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  यदि( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    स्मृतिं_पूरय(a_गन्तव्यम्_Open, 1, nIdx+1);
  }

  यदि( pParse->nested==0 ) sqlite3_Vdbe_गणनम्_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  यदि( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    प्रतिज्ञा( aRegIdx[nAllIdx]==pParse->nMem );
    reg_Row_समुच्चयः = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    यदि( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->nCol;
    }
    यदि( chng_कुञ्चिका || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->nCol;
  }

  /* Start the view context. */
  यदि( isView ){
    sqlite3_Auth_Context_प्रक्षिप(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#यदि !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  यदि( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_क्रमः_By, p_सीमा, i_दत्त_Cur
    );
    p_क्रमः_By = 0;
    p_सीमा = 0;
  }
#यदि_अन्तः

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  यदि( nChangeFrom==0 && sqlite3_Resolve_व्यञ्जकम्_नामानि(&sNC, pWhere) ){
    गच्छ update_cleanup;
  }

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  यदि( IsVirtual(pTab) ){
    update_Virtual_सारणी(pParse, pTabList, pTab, pChanges, p_Rowid_व्यञ्जकम्, aXRef,
                       pWhere, onError);
    गच्छ update_cleanup;
  }
#यदि_अन्तः

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  यदि( (db->flags&SQLITE_गणनम्_पङ्क्तिकाः)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_Row_गणनम् = ++pParse->nMem;
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, reg_Row_गणनम्);
  }

  यदि( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Null, 0, reg_Row_समुच्चयः, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3_Vdbe_सङ्कलय_Op3(v, OP_OpenEphemeral, iEph, 0, reg_Row_समुच्चयः);
  }अन्यथा{
    प्रतिज्ञा( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_कुञ्चिका_Col : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_कुञ्चिका = ++pParse->nMem;
    यदि( pUpsert==0 ){
      पूर्णाङ्क nEphCol = nPk + nChangeFrom + (isView ? pTab->nCol : 0);
      iEph = pParse->nTab++;
      यदि( pPk ) sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, iEph, nEphCol);
      यदि( pPk ){
        कुञ्चिका_Info *p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_सम्बन्धि_सूचकांक(pParse, pPk);
        यदि( p_कुञ्चिका_Info ){
          p_कुञ्चिका_Info->n_All_क्षेत्रम् = nEphCol;
          sqlite3VdbeAppendP4(v, p_कुञ्चिका_Info, P4_KEYINFO);
        }
      }
      यदि( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, p_क्रमः_By, p_सीमा
        );
#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
        यदि( isView ) i_दत्त_Cur = iEph;
#यदि_अन्तः
      }
    }
  }

  यदि( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_कुञ्चिका = nPk;
    reg_कुञ्चिका = iPk;
  }अन्यथा{
    यदि( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_व्यञ्जकम्_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
      bFinishSeek = 0;
    }अन्यथा{
      /* Begin the database scan.
      **
      ** Do not consider a single-pass strategy for a multi-row update if
      ** there is anything that might disrupt the cursor being used to do
      ** the UPDATE:
      **   (1) This is a nested UPDATE
      **   (2) There are triggers
      **   (3) There are FOREIGN KEY constraints
      **   (4) There are REPLACE conflict handlers
      **   (5) There are subqueries in the WHERE clause
      */
      flags = WHERE_ONEPASS_DESIRED;
      यदि( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_कुञ्चिका
       && !bReplace
       && (pWhere==0 || !व्यञ्जकम्_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      यदि( pWInfo==0 ) गच्छ update_cleanup;

      /* A one-pass strategy that might update more than one row may not
      ** be used if any column of the index used for the scan is being
      ** updated. Otherwise, if there is an index on "b", statements like
      ** the following could create an infinite loop:
      **
      **   UPDATE t1 SET b=b+1 WHERE b>?
      **
      ** Fall back to ONEPASS_OFF if where.c has selected a ONEPASS_MULTI
      ** strategy that uses an index for which one or more columns are being
      ** updated.  */
      eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
      bFinishSeek = sqlite3_Where_उपयुङ्क्ते_Deferred_Seek(pWInfo);
      यदि( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        यदि( eOnePass==ONEPASS_MULTI ){
          पूर्णाङ्क iCur = aiCurOnePass[1];
          यदि( iCur>=0 && iCur!=i_दत्त_Cur && a_गन्तव्यम्_Open[iCur-i_आधारः_Cur] ){
            eOnePass = ONEPASS_OFF;
          }
          प्रतिज्ञा( iCur!=i_दत्त_Cur || !HasRowid(pTab) );
        }
      }
    }

    यदि( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, i_दत्त_Cur, regOldRowid);
      यदि( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_निवेशय, iEph, reg_Row_समुच्चयः, regOldRowid);
      }अन्यथा{
        यदि( ALWAYS(addrOpen) ) sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, addrOpen);
      }
    }अन्यथा{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      प्रति(i=0; i<nPk; i++){
        प्रतिज्ञा( pPk->aiColumn[i]>=0 );
        sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, i_दत्त_Cur,
                                        pPk->aiColumn[i], iPk+i);
      }
      यदि( eOnePass ){
        यदि( addrOpen ) sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, addrOpen);
        n_कुञ्चिका = nPk;
        reg_कुञ्चिका = iPk;
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Make_लेखः, iPk, nPk, reg_कुञ्चिका,
                          sqlite3_सूचकांक_Affinity_Str(db, pPk), nPk);
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iEph, reg_कुञ्चिका, iPk, nPk);
      }
    }
  }

  यदि( pUpsert==0 ){
    यदि( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3_Where_अन्तः(pWInfo);
    }

    यदि( !isView ){
      पूर्णाङ्क addrOnce = 0;
      पूर्णाङ्क iNotUsed1 = 0;
      पूर्णाङ्क iNotUsed2 = 0;

      /* Open every index that needs updating. */
      यदि( eOnePass!=ONEPASS_OFF ){
        यदि( aiCurOnePass[0]>=0 ) a_गन्तव्यम्_Open[aiCurOnePass[0]-i_आधारः_Cur] = 0;
        यदि( aiCurOnePass[1]>=0 ) a_गन्तव्यम्_Open[aiCurOnePass[1]-i_आधारः_Cur] = 0;
      }

      यदि( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3_Open_सारणी_And_Indices(pParse, pTab, OP_OpenWrite, 0, i_आधारः_Cur,
                                 a_गन्तव्यम्_Open, &iNotUsed1, &iNotUsed2);
      यदि( addrOnce ){
        sqlite3_Vdbe_Jump_अत्र_Or_निष्कासय_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    यदि( eOnePass!=ONEPASS_OFF ){
      यदि( aiCurOnePass[0]!=i_दत्त_Cur
       && aiCurOnePass[1]!=i_दत्त_Cur
#यदि_परिभाषितम् SQLITE_ALLOW_ROWID_अन्तर्_VIEW
       && !isView
#यदि_अन्तः
      ){
        प्रतिज्ञा( pPk );
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Not_प्राप्तम्, i_दत्त_Cur, labelBreak, reg_कुञ्चिका,n_कुञ्चिका);
        VdbeCoverage(v);
      }
      यदि( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IsNull, pPk ? reg_कुञ्चिका : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }अन्यथा यदि( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_शिखरम् = sqlite3VdbeCurrentAddr(v);
      यदि( nChangeFrom ){
        यदि( !isView ){
          यदि( pPk ){
            प्रति(i=0; i<nPk; i++){
              sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iEph, i, iPk+i);
            }
            sqlite3_Vdbe_सङ्कलय_Op4_Int(
                v, OP_Not_प्राप्तम्, i_दत्त_Cur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }अन्यथा{
            sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3_Vdbe_सङ्कलय_Op3(
                v, OP_NotExists, i_दत्त_Cur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Row_दत्त, iEph, reg_कुञ्चिका);
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Not_प्राप्तम्, i_दत्त_Cur, labelContinue, reg_कुञ्चिका,0);
        VdbeCoverage(v);
      }
    }अन्यथा{
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_NotExists, i_दत्त_Cur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  प्रतिज्ञा( chng_कुञ्चिका || pTrigger || hasFK || regOldRowid==regNewRowid );
  यदि( chngRowid ){
    प्रतिज्ञा( i_Rowid_व्यञ्जकम्>=0 );
    यदि( nChangeFrom==0 ){
      sqlite3_व्यञ्जकम्_संहिता(pParse, p_Rowid_व्यञ्जकम्, regNewRowid);
    }अन्यथा{
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iEph, i_Rowid_व्यञ्जकम्, regNewRowid);
    }
    sqlite3_Vdbe_सङ्कलय_Op1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  यदि( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    प्रति(i=0; i<pTab->nCol; i++){
      u32 colFlags = pTab->aCol[i].colFlags;
      k = sqlite3_सारणी_Column_गन्तव्यम्_Storage(pTab, i) + regOld;
      यदि( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (colFlags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, i_दत्त_Cur, i, k);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, k);
      }
    }
    यदि( chngRowid==0 && pPk==0 ){
#यदि_परिभाषितम् SQLITE_ALLOW_ROWID_अन्तर्_VIEW
      यदि( isView ) sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, regOldRowid);
#यदि_अन्तः
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Copy, regOldRowid, regNewRowid);
    }
  }

  /* Populate the array of registers beginning at regNew with the new
  ** row data. This array is used to check constants, create the new
  ** table and index records, and as the values for any new.* references
  ** made by triggers.
  **
  ** If there are one or more BEFORE triggers, then do not populate the
  ** registers associated with columns that are (a) not modified by
  ** this UPDATE statement and (b) not accessed by new.* references. The
  ** values for registers not modified by the UPDATE must be reloaded from
  ** the database after the BEFORE triggers are fired anyway (as the trigger
  ** may have modified them). So not loading those that are not going to
  ** be used eliminates some redundant opcodes.
  */
  newmask = sqlite3TriggerColmask(
      pParse, pTrigger, pChanges, 1, TRIGGER_BEFORE, pTab, onError
  );
  प्रति(i=0, k=regNew; i<pTab->nCol; i++, k++){
    यदि( i==pTab->iPKey ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, k);
    }अन्यथा यदि( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)!=0 ){
      यदि( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
    }अन्यथा{
      j = aXRef[i];
      यदि( j>=0 ){
        यदि( nChangeFrom ){
          पूर्णाङ्क nOff = (isView ? pTab->nCol : nPk);
          प्रतिज्ञा( eOnePass==ONEPASS_OFF );
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iEph, nOff+j, k);
        }अन्यथा{
          sqlite3_व्यञ्जकम्_संहिता(pParse, pChanges->a[j].p_व्यञ्जकम्, k);
        }
      }अन्यथा यदि( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, i_दत्त_Cur, i, k);
        bFinishSeek = 0;
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, k);
      }
    }
  }
#यदि_अपरिभाषितम् SQLITE_OMIT_GENERATED_COLUMNS
  यदि( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#यदि_अन्तः

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  यदि( tmask&TRIGGER_BEFORE ){
    sqlite3_सारणी_Affinity(v, pTab, regNew);
    sqlite3_संहिता_Row_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    यदि( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      यदि( pPk ){
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Not_प्राप्तम्,i_दत्त_Cur,labelContinue,reg_कुञ्चिका,n_कुञ्चिका);
        VdbeCoverage(v);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_NotExists, i_दत्त_Cur, labelContinue,regOldRowid);
        VdbeCoverage(v);
      }

      /* After-BEFORE-trigger-reload-loop:
      ** If it did not delete it, the BEFORE trigger may still have modified
      ** some of the columns of the row being updated. Load the values for
      ** all columns not modified by the update statement into their registers
      ** in case this has happened. Only unmodified columns are reloaded.
      ** The values computed for modified columns use the values before the
      ** BEFORE trigger runs.  See test case trigger1-18.0 (added 2018-04-26)
      ** for an example.
      */
      प्रति(i=0, k=regNew; i<pTab->nCol; i++, k++){
        यदि( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
          यदि( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
        }अन्यथा यदि( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, i_दत्त_Cur, i, k);
        }
      }
#यदि_अपरिभाषितम् SQLITE_OMIT_GENERATED_COLUMNS
      यदि( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#यदि_अन्तः
    }
  }

  यदि( !isView ){
    /* Do constraint checks. */
    प्रतिज्ञा( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_दत्त_Cur, iIdxCur,
        regNewRowid, regOldRowid, chng_कुञ्चिका, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    यदि( bReplace || chng_कुञ्चिका ){
      यदि( pPk ){
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Not_प्राप्तम्,i_दत्त_Cur,labelContinue,reg_कुञ्चिका,n_कुञ्चिका);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_NotExists, i_दत्त_Cur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    यदि( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_कुञ्चिका);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3_Generate_Row_सूचकांक_Delete(pParse, pTab, i_दत्त_Cur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    यदि( bFinishSeek ){
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_FinishSeek, i_दत्त_Cur);
    }

    /* If changing the rowid value, or if there are foreign key constraints
    ** to process, delete the old record. Otherwise, add a noop OP_Delete
    ** to invoke the pre-update hook.
    **
    ** That (regNew==regnewRowid+1) is true is also important for the
    ** pre-update hook. If the caller invokes preupdate_new(), the returned
    ** value is copied from memory cell (regNewRowid+1+iCol), where iCol
    ** is the column index supplied by the user.
    */
    प्रतिज्ञा( regNew==regNewRowid+1 );
#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Delete, i_दत्त_Cur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_कुञ्चिका) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    यदि( eOnePass==ONEPASS_MULTI ){
      प्रतिज्ञा( hasFK==0 && chng_कुञ्चिका==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    यदि( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_सारणी);
    }
#अन्यथा
    यदि( hasFK>1 || chng_कुञ्चिका ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Delete, i_दत्त_Cur, 0);
    }
#यदि_अन्तः

    यदि( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_कुञ्चिका);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, i_दत्त_Cur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    यदि( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_कुञ्चिका);
    }
  }

  /* Increment the row counter
  */
  यदि( reg_Row_गणनम् ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_सङ्कलय_Imm, reg_Row_गणनम्, 1);
  }

  यदि( pTrigger ){
    sqlite3_संहिता_Row_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  यदि( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }अन्यथा यदि( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Where_अन्तः(pWInfo);
  }अन्यथा{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, iEph, addr_शिखरम्); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  यदि( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3_Autoincrement_अन्तः(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  यदि( reg_Row_गणनम् ){
    sqlite3_संहिता_Change_गणनम्(v, reg_Row_गणनम्, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_निष्कासय(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_व्यञ्जकम्_List_Delete(db, pChanges);
  sqlite3_व्यञ्जकम्_Delete(db, pWhere);
#यदि defined(SQLITE_ENABLE_UPDATE_DELETE_सीमा)
  sqlite3_व्यञ्जकम्_List_Delete(db, p_क्रमः_By);
  sqlite3_व्यञ्जकम्_Delete(db, p_सीमा);
#यदि_अन्तः
  प्रत्यागम;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#यदि_परिभाषितम् isView
 #अपरिभाषय isView
#यदि_अन्तः
#यदि_परिभाषितम् pTrigger
 #अपरिभाषय pTrigger
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
/*
** Generate code for an UPDATE of a virtual table.
**
** There are two possible strategies - the default and the special
** "onepass" strategy. Onepass is only used if the virtual table
** implementation indicates that pWhere may match at most one row.
**
** The default strategy is to create an ephemeral table that contains
** for each row to be changed:
**
**   (A)  The original rowid of that row.
**   (B)  The revised rowid for the row.
**   (C)  The content of every column in the row.
**
** Then loop through the contents of this ephemeral table executing a
** VUpdate for each row. When finished, drop the ephemeral table.
**
** The "onepass" strategy does not use an ephemeral table. Instead, it
** stores the same values (A, B and C above) in a register array and
** makes a single invocation of VUpdate.
*/
स्थिर शून्य update_Virtual_सारणी(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  सारणी *pTab,         /* The virtual table */
  व्यञ्जकम्_List *pChanges,  /* The columns to change in the UPDATE statement */
  व्यञ्जकम् *pRowid,        /* Expression used to recompute the rowid */
  पूर्णाङ्क *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  व्यञ्जकम् *pWhere,        /* WHERE clause of the UPDATE statement */
  पूर्णाङ्क onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  पूर्णाङ्क ephemTab;             /* Table holding the result of the SELECT */
  पूर्णाङ्क i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  ध्रुव वर्ण *pVTab = (ध्रुव वर्ण*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  पूर्णाङ्क nArg = 2 + pTab->nCol;      /* Number of arguments to VUpdate */
  पूर्णाङ्क regArg;                     /* First register in VUpdate arg array */
  पूर्णाङ्क regRec;                     /* Register in which to assemble record */
  पूर्णाङ्क regRowid;                   /* Register for ephemeral table rowid */
  पूर्णाङ्क iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  पूर्णाङ्क aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  पूर्णाङ्क eOnePass;                   /* True to use onepass strategy */
  पूर्णाङ्क addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  प्रतिज्ञा( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  यदि( pSrc->nSrc>1 ){
    सूचकांक *pPk = 0;
    व्यञ्जकम् *pRow;
    व्यञ्जकम्_List *pList;
    यदि( HasRowid(pTab) ){
      यदि( pRowid ){
        pRow = sqlite3_व्यञ्जकम्_Dup(db, pRowid, 0);
      }अन्यथा{
        pRow = sqlite3PExpr(pParse, TK_ROW, 0, 0);
      }
    }अन्यथा{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
      प्रतिज्ञा( pPk!=0 );
      प्रतिज्ञा( pPk->n_कुञ्चिका_Col==1 );
      iPk = pPk->aiColumn[0];
      यदि( aXRef[iPk]>=0 ){
        pRow = sqlite3_व्यञ्जकम्_Dup(db, pChanges->a[aXRef[iPk]].p_व्यञ्जकम्, 0);
      }अन्यथा{
        pRow = व्यञ्जकम्_Row_Column(pParse, iPk);
      }
    }
    pList = sqlite3_व्यञ्जकम्_List_Append(pParse, 0, pRow);

    प्रति(i=0; i<pTab->nCol; i++){
      यदि( aXRef[i]>=0 ){
        pList = sqlite3_व्यञ्जकम्_List_Append(pParse, pList,
          sqlite3_व्यञ्जकम्_Dup(db, pChanges->a[aXRef[i]].p_व्यञ्जकम्, 0)
        );
      }अन्यथा{
        व्यञ्जकम् *p_Row_व्यञ्जकम् = व्यञ्जकम्_Row_Column(pParse, i);
        यदि( p_Row_व्यञ्जकम् ) p_Row_व्यञ्जकम्->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_व्यञ्जकम्_List_Append(pParse, pList, p_Row_व्यञ्जकम्);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_व्यञ्जकम्_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }अन्यथा{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    यदि( pWInfo==0 ) प्रत्यागम;

    /* Populate the argument registers. */
    प्रति(i=0; i<pTab->nCol; i++){
      प्रतिज्ञा( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 );
      यदि( aXRef[i]>=0 ){
        sqlite3_व्यञ्जकम्_संहिता(pParse, pChanges->a[aXRef[i]].p_व्यञ्जकम्, regArg+2+i);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    यदि( HasRowid(pTab) ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, iCsr, regArg);
      यदि( pRowid ){
        sqlite3_व्यञ्जकम्_संहिता(pParse, pRowid, regArg+1);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, iCsr, regArg+1);
      }
    }अन्यथा{
      सूचकांक *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
      प्रतिज्ञा( pPk!=0 );
      प्रतिज्ञा( pPk->n_कुञ्चिका_Col==1 );
      iPk = pPk->aiColumn[0];
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    प्रतिज्ञा( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    यदि( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, addr);
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Close, iCsr);
    }अन्यथा{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, regArg, nArg, regRec);
#यदि defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#यदि_अन्तः
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_निवेशय, ephemTab, regRec, regRowid);
    }
  }


  यदि( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    यदि( pSrc->nSrc==1 ){
      sqlite3_Where_अन्तः(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    प्रति(i=0; i<nArg; i++){
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3_Vdbe_सङ्कलय_Op4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  यदि( eOnePass==ONEPASS_OFF ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_अत्र(v, addr);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Close, ephemTab, 0);
  }अन्यथा{
    sqlite3_Where_अन्तः(pWInfo);
  }
}
#यदि_अन्तः /* SQLITE_OMIT_VIRTUALTABLE */
