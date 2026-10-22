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
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
స్థిర శూన్య update_Virtual_పట్టిక(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  పట్టిక *pTab,         /* The virtual table */
  సమాసం_List *pChanges,  /* The columns to change in the UPDATE statement */
  సమాసం *p_Rowid_సమాసం,    /* Expression used to recompute the rowid */
  పూర్ణాంకం *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  సమాసం *pWhere,        /* WHERE clause of the UPDATE statement */
  పూర్ణాంకం onError          /* ON CONFLICT strategy */
);
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

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
స్థిర SQLITE_NOINLINE శూన్య స్తంభం_Default_Uncommon_Case(
  Vdbe *v,          /* Byte code under construction */
  పట్టిక *pTab,      /* The table */
  స్తంభం *p_నిలువు      /* Which column of the table */
){
  sqlite3_విలువ *p_విలువ = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  ప్రతిజ్ఞా( !IsView(pTab) );
  sqlite3_విలువ_From_సమాసం(sqlite3VdbeDb(v),
                       sqlite3_స్తంభం_సమాసం_Auth(pTab,p_నిలువు,pParse), enc,
                       p_నిలువు->affinity, &p_విలువ);
  ఒకవేళ( p_విలువ ){
    sqlite3VdbeAppendP4(v, p_విలువ, P4_MEM);
  }
}
శూన్య sqlite3_స్తంభం_Default(
  Vdbe *v,          /* Byte code under construction */
  పట్టిక *pTab,      /* The table */
  పూర్ణాంకం i,            /* Which column of the table */
  పూర్ణాంకం iReg          /* Register in which results are stored */
){
  స్తంభం *p_నిలువు;
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( pTab->n_నిలువు>i );
  p_నిలువు = &pTab->a_నిలువు[i];
  ఒకవేళ( p_నిలువు->iDflt ){
    స్తంభం_Default_Uncommon_Case(v,pTab,p_నిలువు);
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
  ఒకవేళ( p_నిలువు->affinity==SQLITE_AFF_REAL ){
    sqlite3_Vdbe_కూడు_Op1(v, OP_RealAffinity, iReg);
  }
#అయితే_ముగింపు
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
స్థిర పూర్ణాంకం సూచీ_స్తంభం_Is_Being_Updated(
  సూచీ *pIdx,      /* The index to check */
  పూర్ణాంకం i_నిలువు,         /* Which column of the index to check */
  పూర్ణాంకం *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  పూర్ణాంకం chngRowid     /* true if the rowid is being updated */
){
  i16 i_Idx_నిలువు = pIdx->ai_స్తంభం[i_నిలువు];
  ప్రతిజ్ఞా( i_Idx_నిలువు!=XN_ROWID ); /* Cannot index rowid */
  ఒకవేళ( i_Idx_నిలువు>=0 ){
    తిరిగి aXRef[i_Idx_నిలువు]>=0;
  }
  ప్రతిజ్ఞా( i_Idx_నిలువు==XN_సమాసం );
  ప్రతిజ్ఞా( pIdx->a_నిలువు_సమాసం!=0 );
  ప్రతిజ్ఞా( pIdx->a_నిలువు_సమాసం->a[i_నిలువు].p_సమాసం!=0 );
  తిరిగి sqlite3_సమాసం_References_Updated_స్తంభం(pIdx->a_నిలువు_సమాసం->a[i_నిలువు].p_సమాసం,
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
స్థిర పూర్ణాంకం సూచీ_Where_Clause_Might_Change(
  సూచీ *pIdx,      /* The index to check */
  పూర్ణాంకం *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  పూర్ణాంకం chngRowid     /* true if the rowid is being updated */
){
  ఒకవేళ( pIdx->pPartIdxWhere==0 ) తిరిగి 0;
  తిరిగి sqlite3_సమాసం_References_Updated_స్తంభం(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
స్థిర సమాసం *సమాసం_అడ్డువరుస_స్తంభం(Parse *pParse, పూర్ణాంకం i_నిలువు){
  సమాసం *pRet = sqlite3PExpr(pParse, TK_అడ్డువరుస, 0, 0);
  ఒకవేళ( pRet ) pRet->i_స్తంభం = i_నిలువు+1;
  తిరిగి pRet;
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
స్థిర శూన్య updateFromSelect(
  Parse *pParse,                  /* Parse context */
  పూర్ణాంకం iEph,                       /* Cursor for open eph. table */
  సూచీ *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  సమాసం_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  సమాసం *pWhere,                   /* WHERE clause for query */
  సమాసం_List *p_క్రమం_By,             /* ORDER BY clause */
  సమాసం *p_హద్దు                    /* LIMIT clause */
){
  పూర్ణాంకం i;
  SelectDest dest;
  Select *pSelect = 0;
  సమాసం_List *pList = 0;
  సమాసం_List *pGrp = 0;
  సమాసం *pLimit2 = 0;
  సమాసం_List *p_క్రమం_By2 = 0;
  sqlite3 *db = pParse->db;
  పట్టిక *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  సమాసం *pWhere2;
  పూర్ణాంకం eDest;

#నిర్వచితమైతే SQLITE_ENABLE_UPDATE_DELETE_హద్దు
  ఒకవేళ( p_క్రమం_By && p_హద్దు==0 ) {
    sqlite3_Error_సందేశం(pParse, "ORDER BY without LIMIT on UPDATE");
    తిరిగి;
  }
  p_క్రమం_By2 = sqlite3_సమాసం_List_Dup(db, p_క్రమం_By, 0);
  pLimit2 = sqlite3_సమాసం_Dup(db, p_హద్దు, 0);
#లేకపోతే
  వాడని_PARAMETER(p_క్రమం_By);
  వాడని_PARAMETER(p_హద్దు);
#అయితే_ముగింపు

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_సమాసం_Dup(db, pWhere, 0);

  ప్రతిజ్ఞా( pTabList->nSrc>1 );
  ఒకవేళ( pSrc ){
    ప్రతిజ్ఞా( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  ఒకవేళ( pPk ){
    కొరకు(i=0; i<pPk->n_కీలకం_నిలువు; i++){
      సమాసం *pNew = సమాసం_అడ్డువరుస_స్తంభం(pParse, pPk->ai_స్తంభం[i]);
#నిర్వచితమైతే SQLITE_ENABLE_UPDATE_DELETE_హద్దు
      ఒకవేళ( p_హద్దు ){
        pGrp = sqlite3_సమాసం_List_Append(pParse, pGrp, sqlite3_సమాసం_Dup(db, pNew, 0));
      }
#అయితే_ముగింపు
      pList = sqlite3_సమాసం_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_పట్టిక : SRT_Upfrom;
  }లేకపోతే ఒకవేళ( IsView(pTab) ){
    కొరకు(i=0; i<pTab->n_నిలువు; i++){
      pList = sqlite3_సమాసం_List_Append(pParse, pList, సమాసం_అడ్డువరుస_స్తంభం(pParse, i));
    }
    eDest = SRT_పట్టిక;
  }లేకపోతే{
    eDest = IsVirtual(pTab) ? SRT_పట్టిక : SRT_Upfrom;
    pList = sqlite3_సమాసం_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_అడ్డువరుస,0,0));
#నిర్వచితమైతే SQLITE_ENABLE_UPDATE_DELETE_హద్దు
    ఒకవేళ( p_హద్దు ){
      pGrp = sqlite3_సమాసం_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_అడ్డువరుస,0,0));
    }
#అయితే_ముగింపు
  }
  ప్రతిజ్ఞా( pChanges!=0 || pParse->db->mallocFailed );
  ఒకవేళ( pChanges ){
    కొరకు(i=0; i<pChanges->n_సమాసం; i++){
      pList = sqlite3_సమాసం_List_Append(pParse, pList,
          sqlite3_సమాసం_Dup(db, pChanges->a[i].p_సమాసం, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, p_క్రమం_By2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  ఒకవేళ( pSelect ) pSelect->selFlags |= SF_క్రమం_By_Reqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_కీలకం_నిలువు : -1);
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
శూన్య sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  సమాసం_List *pChanges,    /* Things to be changed */
  సమాసం *pWhere,          /* The WHERE clause.  May be null */
  పూర్ణాంకం onError,           /* How to handle constraint errors */
  సమాసం_List *p_క్రమం_By,    /* ORDER BY clause. May be null */
  సమాసం *p_హద్దు,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  పూర్ణాంకం i, j, k;           /* Loop counters */
  పట్టిక *pTab;           /* The table to be updated */
  పూర్ణాంకం addr_శిఖరం = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  సూచీ *pIdx;           /* For looping over indices */
  సూచీ *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  పూర్ణాంకం nIdx;              /* Number of indices that need updating */
  పూర్ణాంకం nAllIdx;           /* Total number of indexes */
  పూర్ణాంకం i_పునాది_Cur;          /* Base cursor number */
  పూర్ణాంకం i_దత్తాంశం_Cur;          /* Cursor for the canonical data btree */
  పూర్ణాంకం iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  పూర్ణాంకం *aRegIdx = 0;      /* Registers for to each index and the main table */
  పూర్ణాంకం *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *a_గమ్యం_Open;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_కీలకం;            /* Either chngPk or chngRowid */
  సమాసం *p_Rowid_సమాసం = 0;  /* Expression defining the new record number */
  పూర్ణాంకం i_Rowid_సమాసం = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  పూర్ణాంకం iDb;               /* Database containing the table being updated */
  పూర్ణాంకం eOnePass;          /* ONEPASS_XXX value from where.c */
  పూర్ణాంకం hasFK;             /* True if foreign key processing is required */
  పూర్ణాంకం labelBreak;        /* Jump here to break out of UPDATE loop */
  పూర్ణాంకం labelContinue;     /* Jump here to continue next step of UPDATE loop */
  పూర్ణాంకం flags;             /* Flags for sqlite3WhereBegin() */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  పూర్ణాంకం isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  పూర్ణాంకం tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#అయితే_ముగింపు
  పూర్ణాంకం newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  పూర్ణాంకం iEph = 0;          /* Ephemeral table holding all primary key values */
  పూర్ణాంకం n_కీలకం = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  పూర్ణాంకం aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  పూర్ణాంకం addrOpen = 0;      /* Address of OP_OpenEphemeral */
  పూర్ణాంకం iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  పూర్ణాంకం bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  పూర్ణాంకం b_ముగింపు_Seek = 1;   /* The OP_FinishSeek opcode is needed */
  పూర్ణాంకం nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  పూర్ణాంకం reg_అడ్డువరుస_లెక్క = 0;   /* A count of rows changed */
  పూర్ణాంకం regOldRowid = 0;   /* The old rowid */
  పూర్ణాంకం regNewRowid = 0;   /* The new rowid */
  పూర్ణాంకం regNew = 0;        /* Content of the NEW.* table in triggers */
  పూర్ణాంకం regOld = 0;        /* Content of OLD.* table in triggers */
  పూర్ణాంకం reg_అడ్డువరుస_సమితి = 0;     /* Rowset of rows to be updated */
  పూర్ణాంకం reg_కీలకం = 0;        /* composite PRIMARY KEY value */

  memset(&sContext, 0, పరిమాణం(sContext));
  db = pParse->db;
  ప్రతిజ్ఞా( db->pParse==pParse );
  ఒకవేళ( pParse->nErr ){
    వెళ్ళు update_cleanup;
  }
  ప్రతిజ్ఞా( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ఒకవేళ( pTab==0 ) వెళ్ళు update_cleanup;
  iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  ప్రతిజ్ఞా( pTrigger || tmask==0 );
#లేకపోతే
# define pTrigger 0
# define isView 0
# define tmask 0
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#అయితే_ముగింపు

#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x10000 ){
    sqlite3_వృక్షం_View_పంక్తి(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_వృక్షం_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, p_క్రమం_By, p_హద్దు, pUpsert, pTrigger);
  }
#అయితే_ముగింపు

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_సమాసం : 0;
  ప్రతిజ్ఞా( nChangeFrom==0 || pUpsert==0 );

#నిర్వచితమైతే SQLITE_ENABLE_UPDATE_DELETE_హద్దు
  ఒకవేళ( !isView && nChangeFrom==0 ){
    pWhere = sqlite3_హద్దు_Where(
        pParse, pTabList, pWhere, p_క్రమం_By, p_హద్దు, "UPDATE"
    );
    p_క్రమం_By = 0;
    p_హద్దు = 0;
  }
#అయితే_ముగింపు

  ఒకవేళ( sqlite3_View_Get_స్తంభం_పేర్లు(pParse, pTab) ){
    వెళ్ళు update_cleanup;
  }
  ఒకవేళ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    వెళ్ళు update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  i_పునాది_Cur = i_దత్తాంశం_Cur = pParse->nTab++;
  iIdxCur = i_దత్తాంశం_Cur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_కీలకం_సూచీ(pTab);
  testcase( pPk!=0 && pPk!=pTab->p_సూచీ );
  కొరకు(nIdx=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, nIdx++){
    ఒకవేళ( pPk==pIdx ){
      i_దత్తాంశం_Cur = pParse->nTab;
    }
    pParse->nTab++;
  }
  ఒకవేళ( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    i_దత్తాంశం_Cur = pUpsert->i_దత్తాంశం_Cur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = i_పునాది_Cur;
  }
  pTabList->a[0].iCursor = i_దత్తాంశం_Cur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, పరిమాణం(పూర్ణాంకం) * (pTab->n_నిలువు+nIdx+1) + nIdx+2 );
  ఒకవేళ( aXRef==0 ) వెళ్ళు update_cleanup;
  aRegIdx = aXRef+pTab->n_నిలువు;
  a_గమ్యం_Open = (u8*)(aRegIdx+nIdx+1);
  memset(a_గమ్యం_Open, 1, nIdx+1);
  a_గమ్యం_Open[nIdx+1] = 0;
  కొరకు(i=0; i<pTab->n_నిలువు; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  memset(&sNC, 0, పరిమాణం(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  ఒకవేళ( v==0 ) వెళ్ళు update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  కొరకు(i=0; i<pChanges->n_సమాసం; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    ఒకవేళ( nChangeFrom==0 && sqlite3_Resolve_సమాసం_పేర్లు(&sNC, pChanges->a[i].p_సమాసం) ){
      వెళ్ళు update_cleanup;
    }
    j = sqlite3_స్తంభం_సూచీ(pTab, pChanges->a[i].zEName);
    ఒకవేళ( j>=0 ){
      ఒకవేళ( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_సమాసం = pChanges->a[i].p_సమాసం;
        i_Rowid_సమాసం = i;
      }లేకపోతే ఒకవేళ( pPk && (pTab->a_నిలువు[j].నిలువు_Flags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
      లేకపోతే ఒకవేళ( pTab->a_నిలువు[j].నిలువు_Flags & COLFLAG_GENERATED ){
        testcase( pTab->a_నిలువు[j].నిలువు_Flags & COLFLAG_VIRTUAL );
        testcase( pTab->a_నిలువు[j].నిలువు_Flags & COLFLAG_STORED );
        sqlite3_Error_సందేశం(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->a_నిలువు[j].zCnName);
        వెళ్ళు update_cleanup;
      }
#అయితే_ముగింపు
      aXRef[j] = i;
    }లేకపోతే{
      ఒకవేళ( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_సమాసం = pChanges->a[i].p_సమాసం;
        i_Rowid_సమాసం = i;
      }లేకపోతే{
        sqlite3_Error_సందేశం(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        వెళ్ళు update_cleanup;
      }
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
    {
      పూర్ణాంకం rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->a_నిలువు[j].zCnName,
                            db->aDb[iDb].zDbSName);
      ఒకవేళ( rc!=SQLITE_OK ){
        ఒకవేళ( rc==SQLITE_DENY ){
          వెళ్ళు update_cleanup;
        }లేకపోతే ఒకవేళ( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }లేకపోతే{
          aXRef[j] = -1;
        }
      }
    }
#అయితే_ముగింపు
  }
  ప్రతిజ్ఞా( (chngRowid & chngPk)==0 );
  ప్రతిజ్ఞా( chngRowid==0 || chngRowid==1 );
  ప్రతిజ్ఞా( chngPk==0 || chngPk==1 );
  chng_కీలకం = chngRowid + chngPk;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  ఒకవేళ( pTab->tabFlags & TF_HasGenerated ){
    పూర్ణాంకం b_పురోగతి;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    చేయి{
      b_పురోగతి = 0;
      కొరకు(i=0; i<pTab->n_నిలువు; i++){
        ఒకవేళ( aXRef[i]>=0 ) కొనసాగు;
        ఒకవేళ( (pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_GENERATED)==0 ) కొనసాగు;
        ఒకవేళ( sqlite3_సమాసం_References_Updated_స్తంభం(
                sqlite3_స్తంభం_సమాసం(pTab, &pTab->a_నిలువు[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          b_పురోగతి = 1;
        }
      }
    }అయితే( b_పురోగతి );
  }
#అయితే_ముగింపు

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].నిలువు_వాడిన = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_కీలకం);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  ఒకవేళ( onError==OE_Replace ) bReplace = 1;
  కొరకు(nAllIdx=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, nAllIdx++){
    పూర్ణాంకం reg;
    ఒకవేళ( chng_కీలకం || hasFK>1 || pIdx==pPk
     || సూచీ_Where_Clause_Might_Change(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->n_స్తంభం;
    }లేకపోతే{
      reg = 0;
      కొరకు(i=0; i<pIdx->n_కీలకం_నిలువు; i++){
        ఒకవేళ( సూచీ_స్తంభం_Is_Being_Updated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->n_స్తంభం;
          ఒకవేళ( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          ఆపు;
        }
      }
    }
    ఒకవేళ( reg==0 ) a_గమ్యం_Open[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  ఒకవేళ( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    memset(a_గమ్యం_Open, 1, nIdx+1);
  }

  ఒకవేళ( pParse->nested==0 ) sqlite3_Vdbe_లెక్క_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  ఒకవేళ( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    ప్రతిజ్ఞా( aRegIdx[nAllIdx]==pParse->nMem );
    reg_అడ్డువరుస_సమితి = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    ఒకవేళ( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->n_నిలువు;
    }
    ఒకవేళ( chng_కీలకం || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->n_నిలువు;
  }

  /* Start the view context. */
  ఒకవేళ( isView ){
    sqlite3_Auth_Context_నెట్టు(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#ఒకవేళ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  ఒకవేళ( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_క్రమం_By, p_హద్దు, i_దత్తాంశం_Cur
    );
    p_క్రమం_By = 0;
    p_హద్దు = 0;
  }
#అయితే_ముగింపు

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  ఒకవేళ( nChangeFrom==0 && sqlite3_Resolve_సమాసం_పేర్లు(&sNC, pWhere) ){
    వెళ్ళు update_cleanup;
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  ఒకవేళ( IsVirtual(pTab) ){
    update_Virtual_పట్టిక(pParse, pTabList, pTab, pChanges, p_Rowid_సమాసం, aXRef,
                       pWhere, onError);
    వెళ్ళు update_cleanup;
  }
#అయితే_ముగింపు

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  ఒకవేళ( (db->flags&SQLITE_లెక్క_అడ్డువరుసలు)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_అడ్డువరుస_లెక్క = ++pParse->nMem;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, reg_అడ్డువరుస_లెక్క);
  }

  ఒకవేళ( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, reg_అడ్డువరుస_సమితి, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3_Vdbe_కూడు_Op3(v, OP_OpenEphemeral, iEph, 0, reg_అడ్డువరుస_సమితి);
  }లేకపోతే{
    ప్రతిజ్ఞా( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_కీలకం_నిలువు : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_కీలకం = ++pParse->nMem;
    ఒకవేళ( pUpsert==0 ){
      పూర్ణాంకం n_Eph_నిలువు = nPk + nChangeFrom + (isView ? pTab->n_నిలువు : 0);
      iEph = pParse->nTab++;
      ఒకవేళ( pPk ) sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, iEph, n_Eph_నిలువు);
      ఒకవేళ( pPk ){
        కీలకం_Info *p_కీలకం_Info = sqlite3_కీలకం_Info_యొక్క_సూచీ(pParse, pPk);
        ఒకవేళ( p_కీలకం_Info ){
          p_కీలకం_Info->n_All_క్షేత్రం = n_Eph_నిలువు;
          sqlite3VdbeAppendP4(v, p_కీలకం_Info, P4_KEYINFO);
        }
      }
      ఒకవేళ( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, p_క్రమం_By, p_హద్దు
        );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
        ఒకవేళ( isView ) i_దత్తాంశం_Cur = iEph;
#అయితే_ముగింపు
      }
    }
  }

  ఒకవేళ( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_కీలకం = nPk;
    reg_కీలకం = iPk;
  }లేకపోతే{
    ఒకవేళ( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_సమాసం_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
      b_ముగింపు_Seek = 0;
    }లేకపోతే{
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
      ఒకవేళ( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_కీలకం
       && !bReplace
       && (pWhere==0 || !సమాసం_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      ఒకవేళ( pWInfo==0 ) వెళ్ళు update_cleanup;

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
      b_ముగింపు_Seek = sqlite3_Where_ఉపయోగిస్తుంది_Deferred_Seek(pWInfo);
      ఒకవేళ( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        ఒకవేళ( eOnePass==ONEPASS_MULTI ){
          పూర్ణాంకం iCur = aiCurOnePass[1];
          ఒకవేళ( iCur>=0 && iCur!=i_దత్తాంశం_Cur && a_గమ్యం_Open[iCur-i_పునాది_Cur] ){
            eOnePass = ONEPASS_OFF;
          }
          ప్రతిజ్ఞా( iCur!=i_దత్తాంశం_Cur || !HasRowid(pTab) );
        }
      }
    }

    ఒకవేళ( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, i_దత్తాంశం_Cur, regOldRowid);
      ఒకవేళ( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iEph, reg_అడ్డువరుస_సమితి, regOldRowid);
      }లేకపోతే{
        ఒకవేళ( ALWAYS(addrOpen) ) sqlite3_Vdbe_Change_గమ్యం_Noop(v, addrOpen);
      }
    }లేకపోతే{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      కొరకు(i=0; i<nPk; i++){
        ప్రతిజ్ఞా( pPk->ai_స్తంభం[i]>=0 );
        sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, i_దత్తాంశం_Cur,
                                        pPk->ai_స్తంభం[i], iPk+i);
      }
      ఒకవేళ( eOnePass ){
        ఒకవేళ( addrOpen ) sqlite3_Vdbe_Change_గమ్యం_Noop(v, addrOpen);
        n_కీలకం = nPk;
        reg_కీలకం = iPk;
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op4(v, OP_Make_రికార్డు, iPk, nPk, reg_కీలకం,
                          sqlite3_సూచీ_Affinity_Str(db, pPk), nPk);
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iEph, reg_కీలకం, iPk, nPk);
      }
    }
  }

  ఒకవేళ( pUpsert==0 ){
    ఒకవేళ( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3_Where_చివర(pWInfo);
    }

    ఒకవేళ( !isView ){
      పూర్ణాంకం addrOnce = 0;
      పూర్ణాంకం iNotUsed1 = 0;
      పూర్ణాంకం iNotUsed2 = 0;

      /* Open every index that needs updating. */
      ఒకవేళ( eOnePass!=ONEPASS_OFF ){
        ఒకవేళ( aiCurOnePass[0]>=0 ) a_గమ్యం_Open[aiCurOnePass[0]-i_పునాది_Cur] = 0;
        ఒకవేళ( aiCurOnePass[1]>=0 ) a_గమ్యం_Open[aiCurOnePass[1]-i_పునాది_Cur] = 0;
      }

      ఒకవేళ( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3_Open_పట్టిక_And_Indices(pParse, pTab, OP_OpenWrite, 0, i_పునాది_Cur,
                                 a_గమ్యం_Open, &iNotUsed1, &iNotUsed2);
      ఒకవేళ( addrOnce ){
        sqlite3_Vdbe_Jump_ఇక్కడ_Or_తీయి_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    ఒకవేళ( eOnePass!=ONEPASS_OFF ){
      ఒకవేళ( aiCurOnePass[0]!=i_దత్తాంశం_Cur
       && aiCurOnePass[1]!=i_దత్తాంశం_Cur
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
       && !isView
#అయితే_ముగింపు
      ){
        ప్రతిజ్ఞా( pPk );
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Not_దొరికింది, i_దత్తాంశం_Cur, labelBreak, reg_కీలకం,n_కీలకం);
        VdbeCoverage(v);
      }
      ఒకవేళ( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, pPk ? reg_కీలకం : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }లేకపోతే ఒకవేళ( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_శిఖరం = sqlite3VdbeCurrentAddr(v);
      ఒకవేళ( nChangeFrom ){
        ఒకవేళ( !isView ){
          ఒకవేళ( pPk ){
            కొరకు(i=0; i<nPk; i++){
              sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iEph, i, iPk+i);
            }
            sqlite3_Vdbe_కూడు_Op4_Int(
                v, OP_Not_దొరికింది, i_దత్తాంశం_Cur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }లేకపోతే{
            sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3_Vdbe_కూడు_Op3(
                v, OP_NotExists, i_దత్తాంశం_Cur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_అడ్డువరుస_దత్తాంశం, iEph, reg_కీలకం);
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Not_దొరికింది, i_దత్తాంశం_Cur, labelContinue, reg_కీలకం,0);
        VdbeCoverage(v);
      }
    }లేకపోతే{
      sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_శిఖరం = sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3_Vdbe_కూడు_Op3(v, OP_NotExists, i_దత్తాంశం_Cur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  ప్రతిజ్ఞా( chng_కీలకం || pTrigger || hasFK || regOldRowid==regNewRowid );
  ఒకవేళ( chngRowid ){
    ప్రతిజ్ఞా( i_Rowid_సమాసం>=0 );
    ఒకవేళ( nChangeFrom==0 ){
      sqlite3_సమాసం_సంకేతం(pParse, p_Rowid_సమాసం, regNewRowid);
    }లేకపోతే{
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iEph, i_Rowid_సమాసం, regNewRowid);
    }
    sqlite3_Vdbe_కూడు_Op1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  ఒకవేళ( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    కొరకు(i=0; i<pTab->n_నిలువు; i++){
      u32 నిలువు_Flags = pTab->a_నిలువు[i].నిలువు_Flags;
      k = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i) + regOld;
      ఒకవేళ( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (నిలువు_Flags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, i_దత్తాంశం_Cur, i, k);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, k);
      }
    }
    ఒకవేళ( chngRowid==0 && pPk==0 ){
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
      ఒకవేళ( isView ) sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, regOldRowid);
#అయితే_ముగింపు
      sqlite3_Vdbe_కూడు_Op2(v, OP_Copy, regOldRowid, regNewRowid);
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
  కొరకు(i=0, k=regNew; i<pTab->n_నిలువు; i++, k++){
    ఒకవేళ( i==pTab->iPKey ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, k);
    }లేకపోతే ఒకవేళ( (pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_GENERATED)!=0 ){
      ఒకవేళ( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_VIRTUAL ) k--;
    }లేకపోతే{
      j = aXRef[i];
      ఒకవేళ( j>=0 ){
        ఒకవేళ( nChangeFrom ){
          పూర్ణాంకం nOff = (isView ? pTab->n_నిలువు : nPk);
          ప్రతిజ్ఞా( eOnePass==ONEPASS_OFF );
          sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iEph, nOff+j, k);
        }లేకపోతే{
          sqlite3_సమాసం_సంకేతం(pParse, pChanges->a[j].p_సమాసం, k);
        }
      }లేకపోతే ఒకవేళ( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, i_దత్తాంశం_Cur, i, k);
        b_ముగింపు_Seek = 0;
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, k);
      }
    }
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
  ఒకవేళ( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#అయితే_ముగింపు

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  ఒకవేళ( tmask&TRIGGER_BEFORE ){
    sqlite3_పట్టిక_Affinity(v, pTab, regNew);
    sqlite3_సంకేతం_అడ్డువరుస_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    ఒకవేళ( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      ఒకవేళ( pPk ){
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Not_దొరికింది,i_దత్తాంశం_Cur,labelContinue,reg_కీలకం,n_కీలకం);
        VdbeCoverage(v);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op3(v, OP_NotExists, i_దత్తాంశం_Cur, labelContinue,regOldRowid);
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
      కొరకు(i=0, k=regNew; i<pTab->n_నిలువు; i++, k++){
        ఒకవేళ( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_GENERATED ){
          ఒకవేళ( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_VIRTUAL ) k--;
        }లేకపోతే ఒకవేళ( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, i_దత్తాంశం_Cur, i, k);
        }
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
      ఒకవేళ( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#అయితే_ముగింపు
    }
  }

  ఒకవేళ( !isView ){
    /* Do constraint checks. */
    ప్రతిజ్ఞా( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_దత్తాంశం_Cur, iIdxCur,
        regNewRowid, regOldRowid, chng_కీలకం, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    ఒకవేళ( bReplace || chng_కీలకం ){
      ఒకవేళ( pPk ){
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Not_దొరికింది,i_దత్తాంశం_Cur,labelContinue,reg_కీలకం,n_కీలకం);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op3(v, OP_NotExists, i_దత్తాంశం_Cur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    ఒకవేళ( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_కీలకం);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3_Generate_అడ్డువరుస_సూచీ_Delete(pParse, pTab, i_దత్తాంశం_Cur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    ఒకవేళ( b_ముగింపు_Seek ){
      sqlite3_Vdbe_కూడు_Op1(v, OP_ముగింపు_Seek, i_దత్తాంశం_Cur);
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
    ప్రతిజ్ఞా( regNew==regNewRowid+1 );
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3_Vdbe_కూడు_Op3(v, OP_Delete, i_దత్తాంశం_Cur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_కీలకం) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    ఒకవేళ( eOnePass==ONEPASS_MULTI ){
      ప్రతిజ్ఞా( hasFK==0 && chng_కీలకం==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    ఒకవేళ( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_పట్టిక);
    }
#లేకపోతే
    ఒకవేళ( hasFK>1 || chng_కీలకం ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Delete, i_దత్తాంశం_Cur, 0);
    }
#అయితే_ముగింపు

    ఒకవేళ( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_కీలకం);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, i_దత్తాంశం_Cur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    ఒకవేళ( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_కీలకం);
    }
  }

  /* Increment the row counter
  */
  ఒకవేళ( reg_అడ్డువరుస_లెక్క ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, reg_అడ్డువరుస_లెక్క, 1);
  }

  ఒకవేళ( pTrigger ){
    sqlite3_సంకేతం_అడ్డువరుస_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  ఒకవేళ( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }లేకపోతే ఒకవేళ( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Where_చివర(pWInfo);
  }లేకపోతే{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iEph, addr_శిఖరం); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ఒకవేళ( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3_Autoincrement_చివర(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  ఒకవేళ( reg_అడ్డువరుస_లెక్క ){
    sqlite3_సంకేతం_Change_లెక్క(v, reg_అడ్డువరుస_లెక్క, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_తీయి(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_సమాసం_List_Delete(db, pChanges);
  sqlite3_సమాసం_Delete(db, pWhere);
#ఒకవేళ defined(SQLITE_ENABLE_UPDATE_DELETE_హద్దు)
  sqlite3_సమాసం_List_Delete(db, p_క్రమం_By);
  sqlite3_సమాసం_Delete(db, p_హద్దు);
#అయితే_ముగింపు
  తిరిగి;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#నిర్వచితమైతే isView
 #నిర్వచనం_రద్దు isView
#అయితే_ముగింపు
#నిర్వచితమైతే pTrigger
 #నిర్వచనం_రద్దు pTrigger
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
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
స్థిర శూన్య update_Virtual_పట్టిక(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  పట్టిక *pTab,         /* The virtual table */
  సమాసం_List *pChanges,  /* The columns to change in the UPDATE statement */
  సమాసం *pRowid,        /* Expression used to recompute the rowid */
  పూర్ణాంకం *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  సమాసం *pWhere,        /* WHERE clause of the UPDATE statement */
  పూర్ణాంకం onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  పూర్ణాంకం ephemTab;             /* Table holding the result of the SELECT */
  పూర్ణాంకం i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  స్థిరరాశి అక్షరం *pVTab = (స్థిరరాశి అక్షరం*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  పూర్ణాంకం nArg = 2 + pTab->n_నిలువు;      /* Number of arguments to VUpdate */
  పూర్ణాంకం regArg;                     /* First register in VUpdate arg array */
  పూర్ణాంకం regRec;                     /* Register in which to assemble record */
  పూర్ణాంకం regRowid;                   /* Register for ephemeral table rowid */
  పూర్ణాంకం iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  పూర్ణాంకం aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  పూర్ణాంకం eOnePass;                   /* True to use onepass strategy */
  పూర్ణాంకం addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  ప్రతిజ్ఞా( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  ఒకవేళ( pSrc->nSrc>1 ){
    సూచీ *pPk = 0;
    సమాసం *p_అడ్డువరుస;
    సమాసం_List *pList;
    ఒకవేళ( HasRowid(pTab) ){
      ఒకవేళ( pRowid ){
        p_అడ్డువరుస = sqlite3_సమాసం_Dup(db, pRowid, 0);
      }లేకపోతే{
        p_అడ్డువరుస = sqlite3PExpr(pParse, TK_అడ్డువరుస, 0, 0);
      }
    }లేకపోతే{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
      ప్రతిజ్ఞా( pPk!=0 );
      ప్రతిజ్ఞా( pPk->n_కీలకం_నిలువు==1 );
      iPk = pPk->ai_స్తంభం[0];
      ఒకవేళ( aXRef[iPk]>=0 ){
        p_అడ్డువరుస = sqlite3_సమాసం_Dup(db, pChanges->a[aXRef[iPk]].p_సమాసం, 0);
      }లేకపోతే{
        p_అడ్డువరుస = సమాసం_అడ్డువరుస_స్తంభం(pParse, iPk);
      }
    }
    pList = sqlite3_సమాసం_List_Append(pParse, 0, p_అడ్డువరుస);

    కొరకు(i=0; i<pTab->n_నిలువు; i++){
      ఒకవేళ( aXRef[i]>=0 ){
        pList = sqlite3_సమాసం_List_Append(pParse, pList,
          sqlite3_సమాసం_Dup(db, pChanges->a[aXRef[i]].p_సమాసం, 0)
        );
      }లేకపోతే{
        సమాసం *p_అడ్డువరుస_సమాసం = సమాసం_అడ్డువరుస_స్తంభం(pParse, i);
        ఒకవేళ( p_అడ్డువరుస_సమాసం ) p_అడ్డువరుస_సమాసం->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_సమాసం_List_Append(pParse, pList, p_అడ్డువరుస_సమాసం);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_సమాసం_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }లేకపోతే{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    ఒకవేళ( pWInfo==0 ) తిరిగి;

    /* Populate the argument registers. */
    కొరకు(i=0; i<pTab->n_నిలువు; i++){
      ప్రతిజ్ఞా( (pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_GENERATED)==0 );
      ఒకవేళ( aXRef[i]>=0 ){
        sqlite3_సమాసం_సంకేతం(pParse, pChanges->a[aXRef[i]].p_సమాసం, regArg+2+i);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    ఒకవేళ( HasRowid(pTab) ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iCsr, regArg);
      ఒకవేళ( pRowid ){
        sqlite3_సమాసం_సంకేతం(pParse, pRowid, regArg+1);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iCsr, regArg+1);
      }
    }లేకపోతే{
      సూచీ *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
      ప్రతిజ్ఞా( pPk!=0 );
      ప్రతిజ్ఞా( pPk->n_కీలకం_నిలువు==1 );
      iPk = pPk->ai_స్తంభం[0];
      sqlite3_Vdbe_కూడు_Op3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3_Vdbe_కూడు_Op2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    ప్రతిజ్ఞా( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    ఒకవేళ( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3_Vdbe_Change_గమ్యం_Noop(v, addr);
      sqlite3_Vdbe_కూడు_Op1(v, OP_Close, iCsr);
    }లేకపోతే{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, regArg, nArg, regRec);
#ఒకవేళ defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#అయితే_ముగింపు
      sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, ephemTab, regRec, regRowid);
    }
  }


  ఒకవేళ( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    ఒకవేళ( pSrc->nSrc==1 ){
      sqlite3_Where_చివర(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3_Vdbe_కూడు_Op1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    కొరకు(i=0; i<nArg; i++){
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3_Vdbe_కూడు_Op4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  ఒకవేళ( eOnePass==ONEPASS_OFF ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Close, ephemTab, 0);
  }లేకపోతే{
    sqlite3_Where_చివర(pWInfo);
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */
