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
#અન્તર્ભાવ "sqliteInt.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
સ્થિર શૂન્ય updateVirtualTable(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  Table *pTab,         /* The virtual table */
  પદાવલિ_List *pChanges,  /* The columns to change in the UPDATE statement */
  પદાવલિ *p_Rowid_પદાવલિ,    /* Expression used to recompute the rowid */
  પૂર્ણાંક *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  પદાવલિ *pWhere,        /* WHERE clause of the UPDATE statement */
  પૂર્ણાંક onError          /* ON CONFLICT strategy */
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
સ્થિર SQLITE_NOINLINE શૂન્ય columnDefaultUncommonCase(
  Vdbe *v,          /* Byte code under construction */
  Table *pTab,      /* The table */
  Column *pCol      /* Which column of the table */
){
  sqlite3_કિંમત *p_કિંમત = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  પ્રતિજ્ઞા( !IsView(pTab) );
  sqlite3_કિંમત_From_પદાવલિ(sqlite3VdbeDb(v),
                       sqlite3_Column_પદાવલિ_Auth(pTab,pCol,pParse), enc,
                       pCol->affinity, &p_કિંમત);
  જો( p_કિંમત ){
    sqlite3VdbeAppendP4(v, p_કિંમત, P4_MEM);
  }
}
શૂન્ય sqlite3ColumnDefault(
  Vdbe *v,          /* Byte code under construction */
  Table *pTab,      /* The table */
  પૂર્ણાંક i,            /* Which column of the table */
  પૂર્ણાંક iReg          /* Register in which results are stored */
){
  Column *pCol;
  પ્રતિજ્ઞા( pTab!=0 );
  પ્રતિજ્ઞા( pTab->nCol>i );
  pCol = &pTab->aCol[i];
  જો( pCol->iDflt ){
    columnDefaultUncommonCase(v,pTab,pCol);
  }
#ifndef SQLITE_OMIT_FLOATING_POINT
  જો( pCol->affinity==SQLITE_AFF_REAL ){
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
સ્થિર પૂર્ણાંક indexColumnIsBeingUpdated(
  Index *pIdx,      /* The index to check */
  પૂર્ણાંક iCol,         /* Which column of the index to check */
  પૂર્ણાંક *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  પૂર્ણાંક chngRowid     /* true if the rowid is being updated */
){
  i16 iIdxCol = pIdx->aiColumn[iCol];
  પ્રતિજ્ઞા( iIdxCol!=XN_ROWID ); /* Cannot index rowid */
  જો( iIdxCol>=0 ){
    પાછા aXRef[iIdxCol]>=0;
  }
  પ્રતિજ્ઞા( iIdxCol==XN_પદાવલિ );
  પ્રતિજ્ઞા( pIdx->a_Col_પદાવલિ!=0 );
  પ્રતિજ્ઞા( pIdx->a_Col_પદાવલિ->a[iCol].p_પદાવલિ!=0 );
  પાછા sqlite3_પદાવલિ_References_Updated_Column(pIdx->a_Col_પદાવલિ->a[iCol].p_પદાવલિ,
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
સ્થિર પૂર્ણાંક indexWhereClauseMightChange(
  Index *pIdx,      /* The index to check */
  પૂર્ણાંક *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  પૂર્ણાંક chngRowid     /* true if the rowid is being updated */
){
  જો( pIdx->pPartIdxWhere==0 ) પાછા 0;
  પાછા sqlite3_પદાવલિ_References_Updated_Column(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
સ્થિર પદાવલિ *પદાવલિ_Row_Column(Parse *pParse, પૂર્ણાંક iCol){
  પદાવલિ *pRet = sqlite3PExpr(pParse, TK_ROW, 0, 0);
  જો( pRet ) pRet->iColumn = iCol+1;
  પાછા pRet;
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
સ્થિર શૂન્ય updateFromSelect(
  Parse *pParse,                  /* Parse context */
  પૂર્ણાંક iEph,                       /* Cursor for open eph. table */
  Index *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  પદાવલિ_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  પદાવલિ *pWhere,                   /* WHERE clause for query */
  પદાવલિ_List *pOrderBy,             /* ORDER BY clause */
  પદાવલિ *pLimit                    /* LIMIT clause */
){
  પૂર્ણાંક i;
  SelectDest dest;
  Select *pSelect = 0;
  પદાવલિ_List *pList = 0;
  પદાવલિ_List *pGrp = 0;
  પદાવલિ *pLimit2 = 0;
  પદાવલિ_List *pOrderBy2 = 0;
  sqlite3 *db = pParse->db;
  Table *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  પદાવલિ *pWhere2;
  પૂર્ણાંક eDest;

#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  જો( pOrderBy && pLimit==0 ) {
    sqlite3_Error_સંદેશ(pParse, "ORDER BY without LIMIT on UPDATE");
    પાછા;
  }
  pOrderBy2 = sqlite3_પદાવલિ_List_Dup(db, pOrderBy, 0);
  pLimit2 = sqlite3_પદાવલિ_Dup(db, pLimit, 0);
#નહીંતર
  UNUSED_PARAMETER(pOrderBy);
  UNUSED_PARAMETER(pLimit);
#endif

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_પદાવલિ_Dup(db, pWhere, 0);

  પ્રતિજ્ઞા( pTabList->nSrc>1 );
  જો( pSrc ){
    પ્રતિજ્ઞા( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  જો( pPk ){
    માટે(i=0; i<pPk->n_ચાવી_Col; i++){
      પદાવલિ *pNew = પદાવલિ_Row_Column(pParse, pPk->aiColumn[i]);
#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
      જો( pLimit ){
        pGrp = sqlite3_પદાવલિ_List_Append(pParse, pGrp, sqlite3_પદાવલિ_Dup(db, pNew, 0));
      }
#endif
      pList = sqlite3_પદાવલિ_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_Table : SRT_Upfrom;
  }નહીંતર જો( IsView(pTab) ){
    માટે(i=0; i<pTab->nCol; i++){
      pList = sqlite3_પદાવલિ_List_Append(pParse, pList, પદાવલિ_Row_Column(pParse, i));
    }
    eDest = SRT_Table;
  }નહીંતર{
    eDest = IsVirtual(pTab) ? SRT_Table : SRT_Upfrom;
    pList = sqlite3_પદાવલિ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
    જો( pLimit ){
      pGrp = sqlite3_પદાવલિ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
    }
#endif
  }
  પ્રતિજ્ઞા( pChanges!=0 || pParse->db->mallocFailed );
  જો( pChanges ){
    માટે(i=0; i<pChanges->n_પદાવલિ; i++){
      pList = sqlite3_પદાવલિ_List_Append(pParse, pList,
          sqlite3_પદાવલિ_Dup(db, pChanges->a[i].p_પદાવલિ, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, pOrderBy2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  જો( pSelect ) pSelect->selFlags |= SF_OrderByReqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_ચાવી_Col : -1);
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
શૂન્ય sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  પદાવલિ_List *pChanges,    /* Things to be changed */
  પદાવલિ *pWhere,          /* The WHERE clause.  May be null */
  પૂર્ણાંક onError,           /* How to handle constraint errors */
  પદાવલિ_List *pOrderBy,    /* ORDER BY clause. May be null */
  પદાવલિ *pLimit,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  પૂર્ણાંક i, j, k;           /* Loop counters */
  Table *pTab;           /* The table to be updated */
  પૂર્ણાંક addr_ટોચ = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  Index *pIdx;           /* For looping over indices */
  Index *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  પૂર્ણાંક nIdx;              /* Number of indices that need updating */
  પૂર્ણાંક nAllIdx;           /* Total number of indexes */
  પૂર્ણાંક iBaseCur;          /* Base cursor number */
  પૂર્ણાંક iDataCur;          /* Cursor for the canonical data btree */
  પૂર્ણાંક iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  પૂર્ણાંક *aRegIdx = 0;      /* Registers for to each index and the main table */
  પૂર્ણાંક *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *aToOpen;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_ચાવી;            /* Either chngPk or chngRowid */
  પદાવલિ *p_Rowid_પદાવલિ = 0;  /* Expression defining the new record number */
  પૂર્ણાંક i_Rowid_પદાવલિ = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  પૂર્ણાંક iDb;               /* Database containing the table being updated */
  પૂર્ણાંક eOnePass;          /* ONEPASS_XXX value from where.c */
  પૂર્ણાંક hasFK;             /* True if foreign key processing is required */
  પૂર્ણાંક labelBreak;        /* Jump here to break out of UPDATE loop */
  પૂર્ણાંક labelContinue;     /* Jump here to continue next step of UPDATE loop */
  પૂર્ણાંક flags;             /* Flags for sqlite3WhereBegin() */

#ifndef SQLITE_OMIT_TRIGGER
  પૂર્ણાંક isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  પૂર્ણાંક tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#endif
  પૂર્ણાંક newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  પૂર્ણાંક iEph = 0;          /* Ephemeral table holding all primary key values */
  પૂર્ણાંક n_ચાવી = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  પૂર્ણાંક aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  પૂર્ણાંક addrOpen = 0;      /* Address of OP_OpenEphemeral */
  પૂર્ણાંક iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  પૂર્ણાંક bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  પૂર્ણાંક bFinishSeek = 1;   /* The OP_FinishSeek opcode is needed */
  પૂર્ણાંક nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  પૂર્ણાંક reg_Row_ગણતરી = 0;   /* A count of rows changed */
  પૂર્ણાંક regOldRowid = 0;   /* The old rowid */
  પૂર્ણાંક regNewRowid = 0;   /* The new rowid */
  પૂર્ણાંક regNew = 0;        /* Content of the NEW.* table in triggers */
  પૂર્ણાંક regOld = 0;        /* Content of OLD.* table in triggers */
  પૂર્ણાંક regRowSet = 0;     /* Rowset of rows to be updated */
  પૂર્ણાંક reg_ચાવી = 0;        /* composite PRIMARY KEY value */

  memset(&sContext, 0, કદ(sContext));
  db = pParse->db;
  પ્રતિજ્ઞા( db->pParse==pParse );
  જો( pParse->nErr ){
    જાઓ update_cleanup;
  }
  પ્રતિજ્ઞા( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  જો( pTab==0 ) જાઓ update_cleanup;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#ifndef SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  પ્રતિજ્ઞા( pTrigger || tmask==0 );
#નહીંતર
# define pTrigger 0
# define isView 0
# define tmask 0
#endif
#ifdef SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#endif

#જો TREETRACE_ENABLED
  જો( sqlite3_વૃક્ષ_Trace & 0x10000 ){
    sqlite3_વૃક્ષ_View_લીટી(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_વૃક્ષ_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, pOrderBy, pLimit, pUpsert, pTrigger);
  }
#endif

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_પદાવલિ : 0;
  પ્રતિજ્ઞા( nChangeFrom==0 || pUpsert==0 );

#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  જો( !isView && nChangeFrom==0 ){
    pWhere = sqlite3LimitWhere(
        pParse, pTabList, pWhere, pOrderBy, pLimit, "UPDATE"
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#endif

  જો( sqlite3ViewGetColumnNames(pParse, pTab) ){
    જાઓ update_cleanup;
  }
  જો( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    જાઓ update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  iBaseCur = iDataCur = pParse->nTab++;
  iIdxCur = iDataCur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_ચાવી_Index(pTab);
  testcase( pPk!=0 && pPk!=pTab->pIndex );
  માટે(nIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nIdx++){
    જો( pPk==pIdx ){
      iDataCur = pParse->nTab;
    }
    pParse->nTab++;
  }
  જો( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    iDataCur = pUpsert->iDataCur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = iBaseCur;
  }
  pTabList->a[0].iCursor = iDataCur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, કદ(પૂર્ણાંક) * (pTab->nCol+nIdx+1) + nIdx+2 );
  જો( aXRef==0 ) જાઓ update_cleanup;
  aRegIdx = aXRef+pTab->nCol;
  aToOpen = (u8*)(aRegIdx+nIdx+1);
  memset(aToOpen, 1, nIdx+1);
  aToOpen[nIdx+1] = 0;
  માટે(i=0; i<pTab->nCol; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  memset(&sNC, 0, કદ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  જો( v==0 ) જાઓ update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  માટે(i=0; i<pChanges->n_પદાવલિ; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    જો( nChangeFrom==0 && sqlite3_Resolve_પદાવલિ_Names(&sNC, pChanges->a[i].p_પદાવલિ) ){
      જાઓ update_cleanup;
    }
    j = sqlite3ColumnIndex(pTab, pChanges->a[i].zEName);
    જો( j>=0 ){
      જો( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_પદાવલિ = pChanges->a[i].p_પદાવલિ;
        i_Rowid_પદાવલિ = i;
      }નહીંતર જો( pPk && (pTab->aCol[j].colFlags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
      નહીંતર જો( pTab->aCol[j].colFlags & COLFLAG_GENERATED ){
        testcase( pTab->aCol[j].colFlags & COLFLAG_VIRTUAL );
        testcase( pTab->aCol[j].colFlags & COLFLAG_STORED );
        sqlite3_Error_સંદેશ(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->aCol[j].zCnName);
        જાઓ update_cleanup;
      }
#endif
      aXRef[j] = i;
    }નહીંતર{
      જો( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_પદાવલિ = pChanges->a[i].p_પદાવલિ;
        i_Rowid_પદાવલિ = i;
      }નહીંતર{
        sqlite3_Error_સંદેશ(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        જાઓ update_cleanup;
      }
    }
#ifndef SQLITE_OMIT_AUTHORIZATION
    {
      પૂર્ણાંક rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->aCol[j].zCnName,
                            db->aDb[iDb].zDbSName);
      જો( rc!=SQLITE_OK ){
        જો( rc==SQLITE_DENY ){
          જાઓ update_cleanup;
        }નહીંતર જો( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }નહીંતર{
          aXRef[j] = -1;
        }
      }
    }
#endif
  }
  પ્રતિજ્ઞા( (chngRowid & chngPk)==0 );
  પ્રતિજ્ઞા( chngRowid==0 || chngRowid==1 );
  પ્રતિજ્ઞા( chngPk==0 || chngPk==1 );
  chng_ચાવી = chngRowid + chngPk;

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  જો( pTab->tabFlags & TF_HasGenerated ){
    પૂર્ણાંક bProgress;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    કરો{
      bProgress = 0;
      માટે(i=0; i<pTab->nCol; i++){
        જો( aXRef[i]>=0 ) ચાલુ;
        જો( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 ) ચાલુ;
        જો( sqlite3_પદાવલિ_References_Updated_Column(
                sqlite3_Column_પદાવલિ(pTab, &pTab->aCol[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          bProgress = 1;
        }
      }
    }જ્યારે( bProgress );
  }
#endif

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].colUsed = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_ચાવી);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  જો( onError==OE_Replace ) bReplace = 1;
  માટે(nAllIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nAllIdx++){
    પૂર્ણાંક reg;
    જો( chng_ચાવી || hasFK>1 || pIdx==pPk
     || indexWhereClauseMightChange(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    }નહીંતર{
      reg = 0;
      માટે(i=0; i<pIdx->n_ચાવી_Col; i++){
        જો( indexColumnIsBeingUpdated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->nColumn;
          જો( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          તોડો;
        }
      }
    }
    જો( reg==0 ) aToOpen[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  જો( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    memset(aToOpen, 1, nIdx+1);
  }

  જો( pParse->nested==0 ) sqlite3_Vdbe_ગણતરી_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  જો( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    પ્રતિજ્ઞા( aRegIdx[nAllIdx]==pParse->nMem );
    regRowSet = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    જો( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->nCol;
    }
    જો( chng_ચાવી || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->nCol;
  }

  /* Start the view context. */
  જો( isView ){
    sqlite3_Auth_Context_ધકેલો(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#જો !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  જો( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, pOrderBy, pLimit, iDataCur
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#endif

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  જો( nChangeFrom==0 && sqlite3_Resolve_પદાવલિ_Names(&sNC, pWhere) ){
    જાઓ update_cleanup;
  }

#ifndef SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  જો( IsVirtual(pTab) ){
    updateVirtualTable(pParse, pTabList, pTab, pChanges, p_Rowid_પદાવલિ, aXRef,
                       pWhere, onError);
    જાઓ update_cleanup;
  }
#endif

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  જો( (db->flags&SQLITE_ગણતરી_Rows)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_Row_ગણતરી = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, reg_Row_ગણતરી);
  }

  જો( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3VdbeAddOp3(v, OP_Null, 0, regRowSet, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3VdbeAddOp3(v, OP_OpenEphemeral, iEph, 0, regRowSet);
  }નહીંતર{
    પ્રતિજ્ઞા( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_ચાવી_Col : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_ચાવી = ++pParse->nMem;
    જો( pUpsert==0 ){
      પૂર્ણાંક nEphCol = nPk + nChangeFrom + (isView ? pTab->nCol : 0);
      iEph = pParse->nTab++;
      જો( pPk ) sqlite3VdbeAddOp3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iEph, nEphCol);
      જો( pPk ){
        ચાવી_Info *p_ચાવી_Info = sqlite3_ચાવી_Info_Of_Index(pParse, pPk);
        જો( p_ચાવી_Info ){
          p_ચાવી_Info->nAllField = nEphCol;
          sqlite3VdbeAppendP4(v, p_ચાવી_Info, P4_KEYINFO);
        }
      }
      જો( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, pOrderBy, pLimit
        );
#ifndef SQLITE_OMIT_SUBQUERY
        જો( isView ) iDataCur = iEph;
#endif
      }
    }
  }

  જો( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_ચાવી = nPk;
    reg_ચાવી = iPk;
  }નહીંતર{
    જો( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_પદાવલિ_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
      bFinishSeek = 0;
    }નહીંતર{
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
      જો( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_ચાવી
       && !bReplace
       && (pWhere==0 || !પદાવલિ_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      જો( pWInfo==0 ) જાઓ update_cleanup;

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
      જો( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        જો( eOnePass==ONEPASS_MULTI ){
          પૂર્ણાંક iCur = aiCurOnePass[1];
          જો( iCur>=0 && iCur!=iDataCur && aToOpen[iCur-iBaseCur] ){
            eOnePass = ONEPASS_OFF;
          }
          પ્રતિજ્ઞા( iCur!=iDataCur || !HasRowid(pTab) );
        }
      }
    }

    જો( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3VdbeAddOp2(v, OP_Rowid, iDataCur, regOldRowid);
      જો( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3VdbeAddOp3(v, OP_ઉમેરો, iEph, regRowSet, regOldRowid);
      }નહીંતર{
        જો( ALWAYS(addrOpen) ) sqlite3VdbeChangeToNoop(v, addrOpen);
      }
    }નહીંતર{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      માટે(i=0; i<nPk; i++){
        પ્રતિજ્ઞા( pPk->aiColumn[i]>=0 );
        sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iDataCur,
                                        pPk->aiColumn[i], iPk+i);
      }
      જો( eOnePass ){
        જો( addrOpen ) sqlite3VdbeChangeToNoop(v, addrOpen);
        n_ચાવી = nPk;
        reg_ચાવી = iPk;
      }નહીંતર{
        sqlite3VdbeAddOp4(v, OP_MakeRecord, iPk, nPk, reg_ચાવી,
                          sqlite3IndexAffinityStr(db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iEph, reg_ચાવી, iPk, nPk);
      }
    }
  }

  જો( pUpsert==0 ){
    જો( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3WhereEnd(pWInfo);
    }

    જો( !isView ){
      પૂર્ણાંક addrOnce = 0;
      પૂર્ણાંક iNotUsed1 = 0;
      પૂર્ણાંક iNotUsed2 = 0;

      /* Open every index that needs updating. */
      જો( eOnePass!=ONEPASS_OFF ){
        જો( aiCurOnePass[0]>=0 ) aToOpen[aiCurOnePass[0]-iBaseCur] = 0;
        જો( aiCurOnePass[1]>=0 ) aToOpen[aiCurOnePass[1]-iBaseCur] = 0;
      }

      જો( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, 0, iBaseCur,
                                 aToOpen, &iNotUsed1, &iNotUsed2);
      જો( addrOnce ){
        sqlite3_Vdbe_Jump_Here_Or_કાઢો_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    જો( eOnePass!=ONEPASS_OFF ){
      જો( aiCurOnePass[0]!=iDataCur
       && aiCurOnePass[1]!=iDataCur
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
       && !isView
#endif
      ){
        પ્રતિજ્ઞા( pPk );
        sqlite3VdbeAddOp4Int(v, OP_NotFound, iDataCur, labelBreak, reg_ચાવી,n_ચાવી);
        VdbeCoverage(v);
      }
      જો( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3VdbeAddOp2(v, OP_IsNull, pPk ? reg_ચાવી : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }નહીંતર જો( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_ટોચ = sqlite3VdbeCurrentAddr(v);
      જો( nChangeFrom ){
        જો( !isView ){
          જો( pPk ){
            માટે(i=0; i<nPk; i++){
              sqlite3VdbeAddOp3(v, OP_Column, iEph, i, iPk+i);
            }
            sqlite3VdbeAddOp4Int(
                v, OP_NotFound, iDataCur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }નહીંતર{
            sqlite3VdbeAddOp2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3VdbeAddOp3(
                v, OP_NotExists, iDataCur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }નહીંતર{
        sqlite3VdbeAddOp2(v, OP_RowData, iEph, reg_ચાવી);
        sqlite3VdbeAddOp4Int(v, OP_NotFound, iDataCur, labelContinue, reg_ચાવી,0);
        VdbeCoverage(v);
      }
    }નહીંતર{
      sqlite3VdbeAddOp2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_ટોચ = sqlite3VdbeAddOp2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_NotExists, iDataCur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  પ્રતિજ્ઞા( chng_ચાવી || pTrigger || hasFK || regOldRowid==regNewRowid );
  જો( chngRowid ){
    પ્રતિજ્ઞા( i_Rowid_પદાવલિ>=0 );
    જો( nChangeFrom==0 ){
      sqlite3_પદાવલિ_Code(pParse, p_Rowid_પદાવલિ, regNewRowid);
    }નહીંતર{
      sqlite3VdbeAddOp3(v, OP_Column, iEph, i_Rowid_પદાવલિ, regNewRowid);
    }
    sqlite3VdbeAddOp1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  જો( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    માટે(i=0; i<pTab->nCol; i++){
      u32 colFlags = pTab->aCol[i].colFlags;
      k = sqlite3TableColumnToStorage(pTab, i) + regOld;
      જો( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (colFlags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iDataCur, i, k);
      }નહીંતર{
        sqlite3VdbeAddOp2(v, OP_Null, 0, k);
      }
    }
    જો( chngRowid==0 && pPk==0 ){
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      જો( isView ) sqlite3VdbeAddOp2(v, OP_Null, 0, regOldRowid);
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
  માટે(i=0, k=regNew; i<pTab->nCol; i++, k++){
    જો( i==pTab->iPKey ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, k);
    }નહીંતર જો( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)!=0 ){
      જો( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
    }નહીંતર{
      j = aXRef[i];
      જો( j>=0 ){
        જો( nChangeFrom ){
          પૂર્ણાંક nOff = (isView ? pTab->nCol : nPk);
          પ્રતિજ્ઞા( eOnePass==ONEPASS_OFF );
          sqlite3VdbeAddOp3(v, OP_Column, iEph, nOff+j, k);
        }નહીંતર{
          sqlite3_પદાવલિ_Code(pParse, pChanges->a[j].p_પદાવલિ, k);
        }
      }નહીંતર જો( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iDataCur, i, k);
        bFinishSeek = 0;
      }નહીંતર{
        sqlite3VdbeAddOp2(v, OP_Null, 0, k);
      }
    }
  }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  જો( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#endif

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  જો( tmask&TRIGGER_BEFORE ){
    sqlite3TableAffinity(v, pTab, regNew);
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    જો( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      જો( pPk ){
        sqlite3VdbeAddOp4Int(v, OP_NotFound,iDataCur,labelContinue,reg_ચાવી,n_ચાવી);
        VdbeCoverage(v);
      }નહીંતર{
        sqlite3VdbeAddOp3(v, OP_NotExists, iDataCur, labelContinue,regOldRowid);
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
      માટે(i=0, k=regNew; i<pTab->nCol; i++, k++){
        જો( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
          જો( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
        }નહીંતર જો( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iDataCur, i, k);
        }
      }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
      જો( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#endif
    }
  }

  જો( !isView ){
    /* Do constraint checks. */
    પ્રતિજ્ઞા( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, iDataCur, iIdxCur,
        regNewRowid, regOldRowid, chng_ચાવી, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    જો( bReplace || chng_ચાવી ){
      જો( pPk ){
        sqlite3VdbeAddOp4Int(v, OP_NotFound,iDataCur,labelContinue,reg_ચાવી,n_ચાવી);
      }નહીંતર{
        sqlite3VdbeAddOp3(v, OP_NotExists, iDataCur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    જો( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_ચાવી);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    જો( bFinishSeek ){
      sqlite3VdbeAddOp1(v, OP_FinishSeek, iDataCur);
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
    પ્રતિજ્ઞા( regNew==regNewRowid+1 );
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeAddOp3(v, OP_Delete, iDataCur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_ચાવી) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    જો( eOnePass==ONEPASS_MULTI ){
      પ્રતિજ્ઞા( hasFK==0 && chng_ચાવી==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    જો( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
    }
#નહીંતર
    જો( hasFK>1 || chng_ચાવી ){
      sqlite3VdbeAddOp2(v, OP_Delete, iDataCur, 0);
    }
#endif

    જો( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_ચાવી);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, iDataCur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    જો( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_ચાવી);
    }
  }

  /* Increment the row counter
  */
  જો( reg_Row_ગણતરી ){
    sqlite3VdbeAddOp2(v, OP_AddImm, reg_Row_ગણતરી, 1);
  }

  જો( pTrigger ){
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  જો( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }નહીંતર જો( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3WhereEnd(pWInfo);
  }નહીંતર{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3VdbeAddOp2(v, OP_Next, iEph, addr_ટોચ); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  જો( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  જો( reg_Row_ગણતરી ){
    sqlite3_Code_Change_ગણતરી(v, reg_Row_ગણતરી, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_કાઢો(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_પદાવલિ_List_Delete(db, pChanges);
  sqlite3_પદાવલિ_Delete(db, pWhere);
#જો defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT)
  sqlite3_પદાવલિ_List_Delete(db, pOrderBy);
  sqlite3_પદાવલિ_Delete(db, pLimit);
#endif
  પાછા;
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
સ્થિર શૂન્ય updateVirtualTable(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  Table *pTab,         /* The virtual table */
  પદાવલિ_List *pChanges,  /* The columns to change in the UPDATE statement */
  પદાવલિ *pRowid,        /* Expression used to recompute the rowid */
  પૂર્ણાંક *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  પદાવલિ *pWhere,        /* WHERE clause of the UPDATE statement */
  પૂર્ણાંક onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  પૂર્ણાંક ephemTab;             /* Table holding the result of the SELECT */
  પૂર્ણાંક i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  અચલ અક્ષર *pVTab = (અચલ અક્ષર*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  પૂર્ણાંક nArg = 2 + pTab->nCol;      /* Number of arguments to VUpdate */
  પૂર્ણાંક regArg;                     /* First register in VUpdate arg array */
  પૂર્ણાંક regRec;                     /* Register in which to assemble record */
  પૂર્ણાંક regRowid;                   /* Register for ephemeral table rowid */
  પૂર્ણાંક iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  પૂર્ણાંક aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  પૂર્ણાંક eOnePass;                   /* True to use onepass strategy */
  પૂર્ણાંક addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  પ્રતિજ્ઞા( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3VdbeAddOp2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  જો( pSrc->nSrc>1 ){
    Index *pPk = 0;
    પદાવલિ *pRow;
    પદાવલિ_List *pList;
    જો( HasRowid(pTab) ){
      જો( pRowid ){
        pRow = sqlite3_પદાવલિ_Dup(db, pRowid, 0);
      }નહીંતર{
        pRow = sqlite3PExpr(pParse, TK_ROW, 0, 0);
      }
    }નહીંતર{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_ચાવી_Index(pTab);
      પ્રતિજ્ઞા( pPk!=0 );
      પ્રતિજ્ઞા( pPk->n_ચાવી_Col==1 );
      iPk = pPk->aiColumn[0];
      જો( aXRef[iPk]>=0 ){
        pRow = sqlite3_પદાવલિ_Dup(db, pChanges->a[aXRef[iPk]].p_પદાવલિ, 0);
      }નહીંતર{
        pRow = પદાવલિ_Row_Column(pParse, iPk);
      }
    }
    pList = sqlite3_પદાવલિ_List_Append(pParse, 0, pRow);

    માટે(i=0; i<pTab->nCol; i++){
      જો( aXRef[i]>=0 ){
        pList = sqlite3_પદાવલિ_List_Append(pParse, pList,
          sqlite3_પદાવલિ_Dup(db, pChanges->a[aXRef[i]].p_પદાવલિ, 0)
        );
      }નહીંતર{
        પદાવલિ *p_Row_પદાવલિ = પદાવલિ_Row_Column(pParse, i);
        જો( p_Row_પદાવલિ ) p_Row_પદાવલિ->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_પદાવલિ_List_Append(pParse, pList, p_Row_પદાવલિ);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_પદાવલિ_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }નહીંતર{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    જો( pWInfo==0 ) પાછા;

    /* Populate the argument registers. */
    માટે(i=0; i<pTab->nCol; i++){
      પ્રતિજ્ઞા( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 );
      જો( aXRef[i]>=0 ){
        sqlite3_પદાવલિ_Code(pParse, pChanges->a[aXRef[i]].p_પદાવલિ, regArg+2+i);
      }નહીંતર{
        sqlite3VdbeAddOp3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    જો( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCsr, regArg);
      જો( pRowid ){
        sqlite3_પદાવલિ_Code(pParse, pRowid, regArg+1);
      }નહીંતર{
        sqlite3VdbeAddOp2(v, OP_Rowid, iCsr, regArg+1);
      }
    }નહીંતર{
      Index *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_ચાવી_Index(pTab);
      પ્રતિજ્ઞા( pPk!=0 );
      પ્રતિજ્ઞા( pPk->n_ચાવી_Col==1 );
      iPk = pPk->aiColumn[0];
      sqlite3VdbeAddOp3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3VdbeAddOp2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    પ્રતિજ્ઞા( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    જો( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3VdbeChangeToNoop(v, addr);
      sqlite3VdbeAddOp1(v, OP_Close, iCsr);
    }નહીંતર{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regArg, nArg, regRec);
#જો defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#endif
      sqlite3VdbeAddOp2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3VdbeAddOp3(v, OP_ઉમેરો, ephemTab, regRec, regRowid);
    }
  }


  જો( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    જો( pSrc->nSrc==1 ){
      sqlite3WhereEnd(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    માટે(i=0; i<nArg; i++){
      sqlite3VdbeAddOp3(v, OP_Column, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3VdbeAddOp4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  જો( eOnePass==ONEPASS_OFF ){
    sqlite3VdbeAddOp2(v, OP_Next, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    sqlite3VdbeAddOp2(v, OP_Close, ephemTab, 0);
  }નહીંતર{
    sqlite3WhereEnd(pWInfo);
  }
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */
