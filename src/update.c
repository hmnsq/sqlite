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
#অন্তর্ভাব "sqliteInt.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
স্থির শূন্য updateVirtualTable(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  Table *pTab,         /* The virtual table */
  রাশিমালা_List *pChanges,  /* The columns to change in the UPDATE statement */
  রাশিমালা *p_Rowid_রাশিমালা,    /* Expression used to recompute the rowid */
  পূর্ণসংখ্যা *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  রাশিমালা *pWhere,        /* WHERE clause of the UPDATE statement */
  পূর্ণসংখ্যা onError          /* ON CONFLICT strategy */
);
#endif /* SQLITE_OMIT_VIRTUALTABLE */

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
স্থির SQLITE_NOINLINE শূন্য columnDefaultUncommonCase(
  Vdbe *v,          /* Byte code under construction */
  Table *pTab,      /* The table */
  Column *pCol      /* Which column of the table */
){
  sqlite3_মান *p_মান = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  প্রতিজ্ঞা( !IsView(pTab) );
  sqlite3_মান_From_রাশিমালা(sqlite3VdbeDb(v),
                       sqlite3_Column_রাশিমালা_Auth(pTab,pCol,pParse), enc,
                       pCol->affinity, &p_মান);
  যদি( p_মান ){
    sqlite3VdbeAppendP4(v, p_মান, P4_MEM);
  }
}
শূন্য sqlite3ColumnDefault(
  Vdbe *v,          /* Byte code under construction */
  Table *pTab,      /* The table */
  পূর্ণসংখ্যা i,            /* Which column of the table */
  পূর্ণসংখ্যা iReg          /* Register in which results are stored */
){
  Column *pCol;
  প্রতিজ্ঞা( pTab!=0 );
  প্রতিজ্ঞা( pTab->nCol>i );
  pCol = &pTab->aCol[i];
  যদি( pCol->iDflt ){
    columnDefaultUncommonCase(v,pTab,pCol);
  }
#ifndef SQLITE_OMIT_FLOATING_POINT
  যদি( pCol->affinity==SQLITE_AFF_REAL ){
    sqlite3VdbeAddOp1(v, OP_RealAffinity, iReg);
  }
#endif
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
স্থির পূর্ণসংখ্যা সূচক_Column_Is_Being_Updated(
  সূচক *pIdx,      /* The index to check */
  পূর্ণসংখ্যা iCol,         /* Which column of the index to check */
  পূর্ণসংখ্যা *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  পূর্ণসংখ্যা chngRowid     /* true if the rowid is being updated */
){
  i16 iIdxCol = pIdx->aiColumn[iCol];
  প্রতিজ্ঞা( iIdxCol!=XN_ROWID ); /* Cannot index rowid */
  যদি( iIdxCol>=0 ){
    ফেরত aXRef[iIdxCol]>=0;
  }
  প্রতিজ্ঞা( iIdxCol==XN_রাশিমালা );
  প্রতিজ্ঞা( pIdx->a_Col_রাশিমালা!=0 );
  প্রতিজ্ঞা( pIdx->a_Col_রাশিমালা->a[iCol].p_রাশিমালা!=0 );
  ফেরত sqlite3_রাশিমালা_References_Updated_Column(pIdx->a_Col_রাশিমালা->a[iCol].p_রাশিমালা,
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
স্থির পূর্ণসংখ্যা সূচক_Where_Clause_Might_Change(
  সূচক *pIdx,      /* The index to check */
  পূর্ণসংখ্যা *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  পূর্ণসংখ্যা chngRowid     /* true if the rowid is being updated */
){
  যদি( pIdx->pPartIdxWhere==0 ) ফেরত 0;
  ফেরত sqlite3_রাশিমালা_References_Updated_Column(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
স্থির রাশিমালা *রাশিমালা_Row_Column(Parse *pParse, পূর্ণসংখ্যা iCol){
  রাশিমালা *pRet = sqlite3PExpr(pParse, TK_ROW, 0, 0);
  যদি( pRet ) pRet->iColumn = iCol+1;
  ফেরত pRet;
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
স্থির শূন্য updateFromSelect(
  Parse *pParse,                  /* Parse context */
  পূর্ণসংখ্যা iEph,                       /* Cursor for open eph. table */
  সূচক *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  রাশিমালা_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  রাশিমালা *pWhere,                   /* WHERE clause for query */
  রাশিমালা_List *pOrderBy,             /* ORDER BY clause */
  রাশিমালা *pLimit                    /* LIMIT clause */
){
  পূর্ণসংখ্যা i;
  SelectDest dest;
  Select *pSelect = 0;
  রাশিমালা_List *pList = 0;
  রাশিমালা_List *pGrp = 0;
  রাশিমালা *pLimit2 = 0;
  রাশিমালা_List *pOrderBy2 = 0;
  sqlite3 *db = pParse->db;
  Table *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  রাশিমালা *pWhere2;
  পূর্ণসংখ্যা eDest;

#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  যদি( pOrderBy && pLimit==0 ) {
    sqlite3_Error_বার্তা(pParse, "ORDER BY without LIMIT on UPDATE");
    ফেরত;
  }
  pOrderBy2 = sqlite3_রাশিমালা_List_Dup(db, pOrderBy, 0);
  pLimit2 = sqlite3_রাশিমালা_Dup(db, pLimit, 0);
#অন্যথা
  UNUSED_PARAMETER(pOrderBy);
  UNUSED_PARAMETER(pLimit);
#endif

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_রাশিমালা_Dup(db, pWhere, 0);

  প্রতিজ্ঞা( pTabList->nSrc>1 );
  যদি( pSrc ){
    প্রতিজ্ঞা( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  যদি( pPk ){
    জন্য(i=0; i<pPk->n_চাবি_Col; i++){
      রাশিমালা *pNew = রাশিমালা_Row_Column(pParse, pPk->aiColumn[i]);
#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
      যদি( pLimit ){
        pGrp = sqlite3_রাশিমালা_List_Append(pParse, pGrp, sqlite3_রাশিমালা_Dup(db, pNew, 0));
      }
#endif
      pList = sqlite3_রাশিমালা_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_Table : SRT_Upfrom;
  }অন্যথা যদি( IsView(pTab) ){
    জন্য(i=0; i<pTab->nCol; i++){
      pList = sqlite3_রাশিমালা_List_Append(pParse, pList, রাশিমালা_Row_Column(pParse, i));
    }
    eDest = SRT_Table;
  }অন্যথা{
    eDest = IsVirtual(pTab) ? SRT_Table : SRT_Upfrom;
    pList = sqlite3_রাশিমালা_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
    যদি( pLimit ){
      pGrp = sqlite3_রাশিমালা_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
    }
#endif
  }
  প্রতিজ্ঞা( pChanges!=0 || pParse->db->mallocFailed );
  যদি( pChanges ){
    জন্য(i=0; i<pChanges->n_রাশিমালা; i++){
      pList = sqlite3_রাশিমালা_List_Append(pParse, pList,
          sqlite3_রাশিমালা_Dup(db, pChanges->a[i].p_রাশিমালা, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, pOrderBy2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  যদি( pSelect ) pSelect->selFlags |= SF_OrderByReqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_চাবি_Col : -1);
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
শূন্য sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  রাশিমালা_List *pChanges,    /* Things to be changed */
  রাশিমালা *pWhere,          /* The WHERE clause.  May be null */
  পূর্ণসংখ্যা onError,           /* How to handle constraint errors */
  রাশিমালা_List *pOrderBy,    /* ORDER BY clause. May be null */
  রাশিমালা *pLimit,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  পূর্ণসংখ্যা i, j, k;           /* Loop counters */
  Table *pTab;           /* The table to be updated */
  পূর্ণসংখ্যা addr_শীর্ষ = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  সূচক *pIdx;           /* For looping over indices */
  সূচক *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  পূর্ণসংখ্যা nIdx;              /* Number of indices that need updating */
  পূর্ণসংখ্যা nAllIdx;           /* Total number of indexes */
  পূর্ণসংখ্যা iBaseCur;          /* Base cursor number */
  পূর্ণসংখ্যা i_তথ্য_Cur;          /* Cursor for the canonical data btree */
  পূর্ণসংখ্যা iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  পূর্ণসংখ্যা *aRegIdx = 0;      /* Registers for to each index and the main table */
  পূর্ণসংখ্যা *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *aToOpen;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_চাবি;            /* Either chngPk or chngRowid */
  রাশিমালা *p_Rowid_রাশিমালা = 0;  /* Expression defining the new record number */
  পূর্ণসংখ্যা i_Rowid_রাশিমালা = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  পূর্ণসংখ্যা iDb;               /* Database containing the table being updated */
  পূর্ণসংখ্যা eOnePass;          /* ONEPASS_XXX value from where.c */
  পূর্ণসংখ্যা hasFK;             /* True if foreign key processing is required */
  পূর্ণসংখ্যা labelBreak;        /* Jump here to break out of UPDATE loop */
  পূর্ণসংখ্যা labelContinue;     /* Jump here to continue next step of UPDATE loop */
  পূর্ণসংখ্যা flags;             /* Flags for sqlite3WhereBegin() */

#ifndef SQLITE_OMIT_TRIGGER
  পূর্ণসংখ্যা isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  পূর্ণসংখ্যা tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#endif
  পূর্ণসংখ্যা newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  পূর্ণসংখ্যা iEph = 0;          /* Ephemeral table holding all primary key values */
  পূর্ণসংখ্যা n_চাবি = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  পূর্ণসংখ্যা aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  পূর্ণসংখ্যা addrOpen = 0;      /* Address of OP_OpenEphemeral */
  পূর্ণসংখ্যা iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  পূর্ণসংখ্যা bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  পূর্ণসংখ্যা bFinishSeek = 1;   /* The OP_FinishSeek opcode is needed */
  পূর্ণসংখ্যা nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  পূর্ণসংখ্যা reg_Row_গুনতি = 0;   /* A count of rows changed */
  পূর্ণসংখ্যা regOldRowid = 0;   /* The old rowid */
  পূর্ণসংখ্যা regNewRowid = 0;   /* The new rowid */
  পূর্ণসংখ্যা regNew = 0;        /* Content of the NEW.* table in triggers */
  পূর্ণসংখ্যা regOld = 0;        /* Content of OLD.* table in triggers */
  পূর্ণসংখ্যা regRowSet = 0;     /* Rowset of rows to be updated */
  পূর্ণসংখ্যা reg_চাবি = 0;        /* composite PRIMARY KEY value */

  memset(&sContext, 0, আকার(sContext));
  db = pParse->db;
  প্রতিজ্ঞা( db->pParse==pParse );
  যদি( pParse->nErr ){
    যাও update_cleanup;
  }
  প্রতিজ্ঞা( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  যদি( pTab==0 ) যাও update_cleanup;
  iDb = sqlite3_Schema_To_সূচক(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#ifndef SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  প্রতিজ্ঞা( pTrigger || tmask==0 );
#অন্যথা
# define pTrigger 0
# define isView 0
# define tmask 0
#endif
#ifdef SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#endif

#যদি TREETRACE_ENABLED
  যদি( sqlite3_বৃক্ষ_Trace & 0x10000 ){
    sqlite3_বৃক্ষ_View_সারি(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_বৃক্ষ_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, pOrderBy, pLimit, pUpsert, pTrigger);
  }
#endif

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_রাশিমালা : 0;
  প্রতিজ্ঞা( nChangeFrom==0 || pUpsert==0 );

#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  যদি( !isView && nChangeFrom==0 ){
    pWhere = sqlite3LimitWhere(
        pParse, pTabList, pWhere, pOrderBy, pLimit, "UPDATE"
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#endif

  যদি( sqlite3ViewGetColumnNames(pParse, pTab) ){
    যাও update_cleanup;
  }
  যদি( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    যাও update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  iBaseCur = i_তথ্য_Cur = pParse->nTab++;
  iIdxCur = i_তথ্য_Cur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_চাবি_সূচক(pTab);
  testcase( pPk!=0 && pPk!=pTab->p_সূচক );
  জন্য(nIdx=0, pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী, nIdx++){
    যদি( pPk==pIdx ){
      i_তথ্য_Cur = pParse->nTab;
    }
    pParse->nTab++;
  }
  যদি( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    i_তথ্য_Cur = pUpsert->i_তথ্য_Cur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = iBaseCur;
  }
  pTabList->a[0].iCursor = i_তথ্য_Cur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, আকার(পূর্ণসংখ্যা) * (pTab->nCol+nIdx+1) + nIdx+2 );
  যদি( aXRef==0 ) যাও update_cleanup;
  aRegIdx = aXRef+pTab->nCol;
  aToOpen = (u8*)(aRegIdx+nIdx+1);
  memset(aToOpen, 1, nIdx+1);
  aToOpen[nIdx+1] = 0;
  জন্য(i=0; i<pTab->nCol; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  memset(&sNC, 0, আকার(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  যদি( v==0 ) যাও update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  জন্য(i=0; i<pChanges->n_রাশিমালা; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    যদি( nChangeFrom==0 && sqlite3_Resolve_রাশিমালা_Names(&sNC, pChanges->a[i].p_রাশিমালা) ){
      যাও update_cleanup;
    }
    j = sqlite3_Column_সূচক(pTab, pChanges->a[i].zEName);
    যদি( j>=0 ){
      যদি( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_রাশিমালা = pChanges->a[i].p_রাশিমালা;
        i_Rowid_রাশিমালা = i;
      }অন্যথা যদি( pPk && (pTab->aCol[j].colFlags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
      অন্যথা যদি( pTab->aCol[j].colFlags & COLFLAG_GENERATED ){
        testcase( pTab->aCol[j].colFlags & COLFLAG_VIRTUAL );
        testcase( pTab->aCol[j].colFlags & COLFLAG_STORED );
        sqlite3_Error_বার্তা(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->aCol[j].zCnName);
        যাও update_cleanup;
      }
#endif
      aXRef[j] = i;
    }অন্যথা{
      যদি( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_রাশিমালা = pChanges->a[i].p_রাশিমালা;
        i_Rowid_রাশিমালা = i;
      }অন্যথা{
        sqlite3_Error_বার্তা(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        যাও update_cleanup;
      }
    }
#ifndef SQLITE_OMIT_AUTHORIZATION
    {
      পূর্ণসংখ্যা rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->aCol[j].zCnName,
                            db->aDb[iDb].zDbSName);
      যদি( rc!=SQLITE_OK ){
        যদি( rc==SQLITE_DENY ){
          যাও update_cleanup;
        }অন্যথা যদি( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }অন্যথা{
          aXRef[j] = -1;
        }
      }
    }
#endif
  }
  প্রতিজ্ঞা( (chngRowid & chngPk)==0 );
  প্রতিজ্ঞা( chngRowid==0 || chngRowid==1 );
  প্রতিজ্ঞা( chngPk==0 || chngPk==1 );
  chng_চাবি = chngRowid + chngPk;

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  যদি( pTab->tabFlags & TF_HasGenerated ){
    পূর্ণসংখ্যা bProgress;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    করো{
      bProgress = 0;
      জন্য(i=0; i<pTab->nCol; i++){
        যদি( aXRef[i]>=0 ) চালু;
        যদি( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 ) চালু;
        যদি( sqlite3_রাশিমালা_References_Updated_Column(
                sqlite3_Column_রাশিমালা(pTab, &pTab->aCol[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          bProgress = 1;
        }
      }
    }যখন( bProgress );
  }
#endif

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].colUsed = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_চাবি);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  যদি( onError==OE_Replace ) bReplace = 1;
  জন্য(nAllIdx=0, pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী, nAllIdx++){
    পূর্ণসংখ্যা reg;
    যদি( chng_চাবি || hasFK>1 || pIdx==pPk
     || সূচক_Where_Clause_Might_Change(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    }অন্যথা{
      reg = 0;
      জন্য(i=0; i<pIdx->n_চাবি_Col; i++){
        যদি( সূচক_Column_Is_Being_Updated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->nColumn;
          যদি( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          বিরতি;
        }
      }
    }
    যদি( reg==0 ) aToOpen[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  যদি( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    memset(aToOpen, 1, nIdx+1);
  }

  যদি( pParse->nested==0 ) sqlite3_Vdbe_গুনতি_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  যদি( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    প্রতিজ্ঞা( aRegIdx[nAllIdx]==pParse->nMem );
    regRowSet = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    যদি( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->nCol;
    }
    যদি( chng_চাবি || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->nCol;
  }

  /* Start the view context. */
  যদি( isView ){
    sqlite3_Auth_Context_ঠেলো(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#যদি !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  যদি( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, pOrderBy, pLimit, i_তথ্য_Cur
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#endif

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  যদি( nChangeFrom==0 && sqlite3_Resolve_রাশিমালা_Names(&sNC, pWhere) ){
    যাও update_cleanup;
  }

#ifndef SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  যদি( IsVirtual(pTab) ){
    updateVirtualTable(pParse, pTabList, pTab, pChanges, p_Rowid_রাশিমালা, aXRef,
                       pWhere, onError);
    যাও update_cleanup;
  }
#endif

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  যদি( (db->flags&SQLITE_গুনতি_Rows)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_Row_গুনতি = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, reg_Row_গুনতি);
  }

  যদি( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3VdbeAddOp3(v, OP_Null, 0, regRowSet, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3VdbeAddOp3(v, OP_OpenEphemeral, iEph, 0, regRowSet);
  }অন্যথা{
    প্রতিজ্ঞা( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_চাবি_Col : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_চাবি = ++pParse->nMem;
    যদি( pUpsert==0 ){
      পূর্ণসংখ্যা nEphCol = nPk + nChangeFrom + (isView ? pTab->nCol : 0);
      iEph = pParse->nTab++;
      যদি( pPk ) sqlite3VdbeAddOp3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iEph, nEphCol);
      যদি( pPk ){
        চাবি_Info *p_চাবি_Info = sqlite3_চাবি_Info_Of_সূচক(pParse, pPk);
        যদি( p_চাবি_Info ){
          p_চাবি_Info->nAllField = nEphCol;
          sqlite3VdbeAppendP4(v, p_চাবি_Info, P4_KEYINFO);
        }
      }
      যদি( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, pOrderBy, pLimit
        );
#ifndef SQLITE_OMIT_SUBQUERY
        যদি( isView ) i_তথ্য_Cur = iEph;
#endif
      }
    }
  }

  যদি( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_চাবি = nPk;
    reg_চাবি = iPk;
  }অন্যথা{
    যদি( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_রাশিমালা_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
      bFinishSeek = 0;
    }অন্যথা{
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
      যদি( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_চাবি
       && !bReplace
       && (pWhere==0 || !রাশিমালা_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      যদি( pWInfo==0 ) যাও update_cleanup;

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
      bFinishSeek = sqlite3WhereUsesDeferredSeek(pWInfo);
      যদি( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        যদি( eOnePass==ONEPASS_MULTI ){
          পূর্ণসংখ্যা iCur = aiCurOnePass[1];
          যদি( iCur>=0 && iCur!=i_তথ্য_Cur && aToOpen[iCur-iBaseCur] ){
            eOnePass = ONEPASS_OFF;
          }
          প্রতিজ্ঞা( iCur!=i_তথ্য_Cur || !HasRowid(pTab) );
        }
      }
    }

    যদি( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3VdbeAddOp2(v, OP_Rowid, i_তথ্য_Cur, regOldRowid);
      যদি( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3VdbeAddOp3(v, OP_ঢোকাও, iEph, regRowSet, regOldRowid);
      }অন্যথা{
        যদি( ALWAYS(addrOpen) ) sqlite3VdbeChangeToNoop(v, addrOpen);
      }
    }অন্যথা{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      জন্য(i=0; i<nPk; i++){
        প্রতিজ্ঞা( pPk->aiColumn[i]>=0 );
        sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, i_তথ্য_Cur,
                                        pPk->aiColumn[i], iPk+i);
      }
      যদি( eOnePass ){
        যদি( addrOpen ) sqlite3VdbeChangeToNoop(v, addrOpen);
        n_চাবি = nPk;
        reg_চাবি = iPk;
      }অন্যথা{
        sqlite3VdbeAddOp4(v, OP_MakeRecord, iPk, nPk, reg_চাবি,
                          sqlite3_সূচক_Affinity_Str(db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ঢোকাও, iEph, reg_চাবি, iPk, nPk);
      }
    }
  }

  যদি( pUpsert==0 ){
    যদি( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3WhereEnd(pWInfo);
    }

    যদি( !isView ){
      পূর্ণসংখ্যা addrOnce = 0;
      পূর্ণসংখ্যা iNotUsed1 = 0;
      পূর্ণসংখ্যা iNotUsed2 = 0;

      /* Open every index that needs updating. */
      যদি( eOnePass!=ONEPASS_OFF ){
        যদি( aiCurOnePass[0]>=0 ) aToOpen[aiCurOnePass[0]-iBaseCur] = 0;
        যদি( aiCurOnePass[1]>=0 ) aToOpen[aiCurOnePass[1]-iBaseCur] = 0;
      }

      যদি( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, 0, iBaseCur,
                                 aToOpen, &iNotUsed1, &iNotUsed2);
      যদি( addrOnce ){
        sqlite3_Vdbe_Jump_Here_Or_বের_করো_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    যদি( eOnePass!=ONEPASS_OFF ){
      যদি( aiCurOnePass[0]!=i_তথ্য_Cur
       && aiCurOnePass[1]!=i_তথ্য_Cur
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
       && !isView
#endif
      ){
        প্রতিজ্ঞা( pPk );
        sqlite3VdbeAddOp4Int(v, OP_NotFound, i_তথ্য_Cur, labelBreak, reg_চাবি,n_চাবি);
        VdbeCoverage(v);
      }
      যদি( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3VdbeAddOp2(v, OP_IsNull, pPk ? reg_চাবি : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }অন্যথা যদি( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_শীর্ষ = sqlite3VdbeCurrentAddr(v);
      যদি( nChangeFrom ){
        যদি( !isView ){
          যদি( pPk ){
            জন্য(i=0; i<nPk; i++){
              sqlite3VdbeAddOp3(v, OP_Column, iEph, i, iPk+i);
            }
            sqlite3VdbeAddOp4Int(
                v, OP_NotFound, i_তথ্য_Cur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }অন্যথা{
            sqlite3VdbeAddOp2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3VdbeAddOp3(
                v, OP_NotExists, i_তথ্য_Cur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }অন্যথা{
        sqlite3VdbeAddOp2(v, OP_Row_তথ্য, iEph, reg_চাবি);
        sqlite3VdbeAddOp4Int(v, OP_NotFound, i_তথ্য_Cur, labelContinue, reg_চাবি,0);
        VdbeCoverage(v);
      }
    }অন্যথা{
      sqlite3VdbeAddOp2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_শীর্ষ = sqlite3VdbeAddOp2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_NotExists, i_তথ্য_Cur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  প্রতিজ্ঞা( chng_চাবি || pTrigger || hasFK || regOldRowid==regNewRowid );
  যদি( chngRowid ){
    প্রতিজ্ঞা( i_Rowid_রাশিমালা>=0 );
    যদি( nChangeFrom==0 ){
      sqlite3_রাশিমালা_Code(pParse, p_Rowid_রাশিমালা, regNewRowid);
    }অন্যথা{
      sqlite3VdbeAddOp3(v, OP_Column, iEph, i_Rowid_রাশিমালা, regNewRowid);
    }
    sqlite3VdbeAddOp1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  যদি( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    জন্য(i=0; i<pTab->nCol; i++){
      u32 colFlags = pTab->aCol[i].colFlags;
      k = sqlite3TableColumnToStorage(pTab, i) + regOld;
      যদি( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (colFlags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, i_তথ্য_Cur, i, k);
      }অন্যথা{
        sqlite3VdbeAddOp2(v, OP_Null, 0, k);
      }
    }
    যদি( chngRowid==0 && pPk==0 ){
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      যদি( isView ) sqlite3VdbeAddOp2(v, OP_Null, 0, regOldRowid);
#endif
      sqlite3VdbeAddOp2(v, OP_Copy, regOldRowid, regNewRowid);
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
  জন্য(i=0, k=regNew; i<pTab->nCol; i++, k++){
    যদি( i==pTab->iPKey ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, k);
    }অন্যথা যদি( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)!=0 ){
      যদি( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
    }অন্যথা{
      j = aXRef[i];
      যদি( j>=0 ){
        যদি( nChangeFrom ){
          পূর্ণসংখ্যা nOff = (isView ? pTab->nCol : nPk);
          প্রতিজ্ঞা( eOnePass==ONEPASS_OFF );
          sqlite3VdbeAddOp3(v, OP_Column, iEph, nOff+j, k);
        }অন্যথা{
          sqlite3_রাশিমালা_Code(pParse, pChanges->a[j].p_রাশিমালা, k);
        }
      }অন্যথা যদি( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, i_তথ্য_Cur, i, k);
        bFinishSeek = 0;
      }অন্যথা{
        sqlite3VdbeAddOp2(v, OP_Null, 0, k);
      }
    }
  }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  যদি( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#endif

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  যদি( tmask&TRIGGER_BEFORE ){
    sqlite3TableAffinity(v, pTab, regNew);
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    যদি( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      যদি( pPk ){
        sqlite3VdbeAddOp4Int(v, OP_NotFound,i_তথ্য_Cur,labelContinue,reg_চাবি,n_চাবি);
        VdbeCoverage(v);
      }অন্যথা{
        sqlite3VdbeAddOp3(v, OP_NotExists, i_তথ্য_Cur, labelContinue,regOldRowid);
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
      জন্য(i=0, k=regNew; i<pTab->nCol; i++, k++){
        যদি( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
          যদি( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
        }অন্যথা যদি( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, i_তথ্য_Cur, i, k);
        }
      }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
      যদি( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#endif
    }
  }

  যদি( !isView ){
    /* Do constraint checks. */
    প্রতিজ্ঞা( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_তথ্য_Cur, iIdxCur,
        regNewRowid, regOldRowid, chng_চাবি, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    যদি( bReplace || chng_চাবি ){
      যদি( pPk ){
        sqlite3VdbeAddOp4Int(v, OP_NotFound,i_তথ্য_Cur,labelContinue,reg_চাবি,n_চাবি);
      }অন্যথা{
        sqlite3VdbeAddOp3(v, OP_NotExists, i_তথ্য_Cur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    যদি( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_চাবি);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3_Generate_Row_সূচক_Delete(pParse, pTab, i_তথ্য_Cur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    যদি( bFinishSeek ){
      sqlite3VdbeAddOp1(v, OP_FinishSeek, i_তথ্য_Cur);
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
    প্রতিজ্ঞা( regNew==regNewRowid+1 );
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeAddOp3(v, OP_Delete, i_তথ্য_Cur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_চাবি) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    যদি( eOnePass==ONEPASS_MULTI ){
      প্রতিজ্ঞা( hasFK==0 && chng_চাবি==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    যদি( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
    }
#অন্যথা
    যদি( hasFK>1 || chng_চাবি ){
      sqlite3VdbeAddOp2(v, OP_Delete, i_তথ্য_Cur, 0);
    }
#endif

    যদি( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_চাবি);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, i_তথ্য_Cur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    যদি( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_চাবি);
    }
  }

  /* Increment the row counter
  */
  যদি( reg_Row_গুনতি ){
    sqlite3VdbeAddOp2(v, OP_AddImm, reg_Row_গুনতি, 1);
  }

  যদি( pTrigger ){
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  যদি( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }অন্যথা যদি( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3WhereEnd(pWInfo);
  }অন্যথা{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3VdbeAddOp2(v, OP_পরবর্তী, iEph, addr_শীর্ষ); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  যদি( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  যদি( reg_Row_গুনতি ){
    sqlite3_Code_Change_গুনতি(v, reg_Row_গুনতি, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_বের_করো(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_রাশিমালা_List_Delete(db, pChanges);
  sqlite3_রাশিমালা_Delete(db, pWhere);
#যদি defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT)
  sqlite3_রাশিমালা_List_Delete(db, pOrderBy);
  sqlite3_রাশিমালা_Delete(db, pLimit);
#endif
  ফেরত;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ifdef isView
 #undef isView
#endif
#ifdef pTrigger
 #undef pTrigger
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
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
স্থির শূন্য updateVirtualTable(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  Table *pTab,         /* The virtual table */
  রাশিমালা_List *pChanges,  /* The columns to change in the UPDATE statement */
  রাশিমালা *pRowid,        /* Expression used to recompute the rowid */
  পূর্ণসংখ্যা *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  রাশিমালা *pWhere,        /* WHERE clause of the UPDATE statement */
  পূর্ণসংখ্যা onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  পূর্ণসংখ্যা ephemTab;             /* Table holding the result of the SELECT */
  পূর্ণসংখ্যা i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  ধ্রুবক অক্ষর *pVTab = (ধ্রুবক অক্ষর*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  পূর্ণসংখ্যা nArg = 2 + pTab->nCol;      /* Number of arguments to VUpdate */
  পূর্ণসংখ্যা regArg;                     /* First register in VUpdate arg array */
  পূর্ণসংখ্যা regRec;                     /* Register in which to assemble record */
  পূর্ণসংখ্যা regRowid;                   /* Register for ephemeral table rowid */
  পূর্ণসংখ্যা iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  পূর্ণসংখ্যা aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  পূর্ণসংখ্যা eOnePass;                   /* True to use onepass strategy */
  পূর্ণসংখ্যা addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  প্রতিজ্ঞা( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3VdbeAddOp2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  যদি( pSrc->nSrc>1 ){
    সূচক *pPk = 0;
    রাশিমালা *pRow;
    রাশিমালা_List *pList;
    যদি( HasRowid(pTab) ){
      যদি( pRowid ){
        pRow = sqlite3_রাশিমালা_Dup(db, pRowid, 0);
      }অন্যথা{
        pRow = sqlite3PExpr(pParse, TK_ROW, 0, 0);
      }
    }অন্যথা{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_চাবি_সূচক(pTab);
      প্রতিজ্ঞা( pPk!=0 );
      প্রতিজ্ঞা( pPk->n_চাবি_Col==1 );
      iPk = pPk->aiColumn[0];
      যদি( aXRef[iPk]>=0 ){
        pRow = sqlite3_রাশিমালা_Dup(db, pChanges->a[aXRef[iPk]].p_রাশিমালা, 0);
      }অন্যথা{
        pRow = রাশিমালা_Row_Column(pParse, iPk);
      }
    }
    pList = sqlite3_রাশিমালা_List_Append(pParse, 0, pRow);

    জন্য(i=0; i<pTab->nCol; i++){
      যদি( aXRef[i]>=0 ){
        pList = sqlite3_রাশিমালা_List_Append(pParse, pList,
          sqlite3_রাশিমালা_Dup(db, pChanges->a[aXRef[i]].p_রাশিমালা, 0)
        );
      }অন্যথা{
        রাশিমালা *p_Row_রাশিমালা = রাশিমালা_Row_Column(pParse, i);
        যদি( p_Row_রাশিমালা ) p_Row_রাশিমালা->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_রাশিমালা_List_Append(pParse, pList, p_Row_রাশিমালা);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_রাশিমালা_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }অন্যথা{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    যদি( pWInfo==0 ) ফেরত;

    /* Populate the argument registers. */
    জন্য(i=0; i<pTab->nCol; i++){
      প্রতিজ্ঞা( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 );
      যদি( aXRef[i]>=0 ){
        sqlite3_রাশিমালা_Code(pParse, pChanges->a[aXRef[i]].p_রাশিমালা, regArg+2+i);
      }অন্যথা{
        sqlite3VdbeAddOp3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    যদি( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCsr, regArg);
      যদি( pRowid ){
        sqlite3_রাশিমালা_Code(pParse, pRowid, regArg+1);
      }অন্যথা{
        sqlite3VdbeAddOp2(v, OP_Rowid, iCsr, regArg+1);
      }
    }অন্যথা{
      সূচক *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_চাবি_সূচক(pTab);
      প্রতিজ্ঞা( pPk!=0 );
      প্রতিজ্ঞা( pPk->n_চাবি_Col==1 );
      iPk = pPk->aiColumn[0];
      sqlite3VdbeAddOp3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3VdbeAddOp2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    প্রতিজ্ঞা( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    যদি( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3VdbeChangeToNoop(v, addr);
      sqlite3VdbeAddOp1(v, OP_Close, iCsr);
    }অন্যথা{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regArg, nArg, regRec);
#যদি defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#endif
      sqlite3VdbeAddOp2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3VdbeAddOp3(v, OP_ঢোকাও, ephemTab, regRec, regRowid);
    }
  }


  যদি( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    যদি( pSrc->nSrc==1 ){
      sqlite3WhereEnd(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    জন্য(i=0; i<nArg; i++){
      sqlite3VdbeAddOp3(v, OP_Column, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3VdbeAddOp4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  যদি( eOnePass==ONEPASS_OFF ){
    sqlite3VdbeAddOp2(v, OP_পরবর্তী, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    sqlite3VdbeAddOp2(v, OP_Close, ephemTab, 0);
  }অন্যথা{
    sqlite3WhereEnd(pWInfo);
  }
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */
