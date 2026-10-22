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
#समावेश "sqliteInt.h"

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
अटल व्योम updateVirtualTable(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  Table *pTab,         /* The virtual table */
  व्यंजक_List *pChanges,  /* The columns to change in the UPDATE statement */
  व्यंजक *p_Rowid_व्यंजक,    /* Expression used to recompute the rowid */
  पूर्णांक *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  व्यंजक *pWhere,        /* WHERE clause of the UPDATE statement */
  पूर्णांक onError          /* ON CONFLICT strategy */
);
#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */

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
अटल SQLITE_NOINLINE व्योम columnDefaultUncommonCase(
  Vdbe *v,          /* Byte code under construction */
  Table *pTab,      /* The table */
  Column *pCol      /* Which column of the table */
){
  sqlite3_मूल्य *p_मूल्य = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  निश्चित( !IsView(pTab) );
  sqlite3_मूल्य_From_व्यंजक(sqlite3VdbeDb(v),
                       sqlite3_Column_व्यंजक_Auth(pTab,pCol,pParse), enc,
                       pCol->affinity, &p_मूल्य);
  अगर( p_मूल्य ){
    sqlite3VdbeAppendP4(v, p_मूल्य, P4_MEM);
  }
}
व्योम sqlite3ColumnDefault(
  Vdbe *v,          /* Byte code under construction */
  Table *pTab,      /* The table */
  पूर्णांक i,            /* Which column of the table */
  पूर्णांक iReg          /* Register in which results are stored */
){
  Column *pCol;
  निश्चित( pTab!=0 );
  निश्चित( pTab->nCol>i );
  pCol = &pTab->aCol[i];
  अगर( pCol->iDflt ){
    columnDefaultUncommonCase(v,pTab,pCol);
  }
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
  अगर( pCol->affinity==SQLITE_AFF_REAL ){
    sqlite3VdbeAddOp1(v, OP_RealAffinity, iReg);
  }
#पूर्ण_अगर
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
अटल पूर्णांक सूचकांक_Column_Is_Being_Updated(
  सूचकांक *pIdx,      /* The index to check */
  पूर्णांक iCol,         /* Which column of the index to check */
  पूर्णांक *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  पूर्णांक chngRowid     /* true if the rowid is being updated */
){
  i16 iIdxCol = pIdx->aiColumn[iCol];
  निश्चित( iIdxCol!=XN_ROWID ); /* Cannot index rowid */
  अगर( iIdxCol>=0 ){
    वापस aXRef[iIdxCol]>=0;
  }
  निश्चित( iIdxCol==XN_व्यंजक );
  निश्चित( pIdx->a_Col_व्यंजक!=0 );
  निश्चित( pIdx->a_Col_व्यंजक->a[iCol].p_व्यंजक!=0 );
  वापस sqlite3_व्यंजक_References_Updated_Column(pIdx->a_Col_व्यंजक->a[iCol].p_व्यंजक,
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
अटल पूर्णांक सूचकांक_Where_Clause_Might_Change(
  सूचकांक *pIdx,      /* The index to check */
  पूर्णांक *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  पूर्णांक chngRowid     /* true if the rowid is being updated */
){
  अगर( pIdx->pPartIdxWhere==0 ) वापस 0;
  वापस sqlite3_व्यंजक_References_Updated_Column(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
अटल व्यंजक *व्यंजक_Row_Column(Parse *pParse, पूर्णांक iCol){
  व्यंजक *pRet = sqlite3PExpr(pParse, TK_ROW, 0, 0);
  अगर( pRet ) pRet->iColumn = iCol+1;
  वापस pRet;
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
अटल व्योम updateFromSelect(
  Parse *pParse,                  /* Parse context */
  पूर्णांक iEph,                       /* Cursor for open eph. table */
  सूचकांक *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  व्यंजक_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  व्यंजक *pWhere,                   /* WHERE clause for query */
  व्यंजक_List *pOrderBy,             /* ORDER BY clause */
  व्यंजक *pLimit                    /* LIMIT clause */
){
  पूर्णांक i;
  SelectDest dest;
  Select *pSelect = 0;
  व्यंजक_List *pList = 0;
  व्यंजक_List *pGrp = 0;
  व्यंजक *pLimit2 = 0;
  व्यंजक_List *pOrderBy2 = 0;
  sqlite3 *db = pParse->db;
  Table *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  व्यंजक *pWhere2;
  पूर्णांक eDest;

#अगर_घोषित SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  अगर( pOrderBy && pLimit==0 ) {
    sqlite3_Error_संदेश(pParse, "ORDER BY without LIMIT on UPDATE");
    वापस;
  }
  pOrderBy2 = sqlite3_व्यंजक_List_Dup(db, pOrderBy, 0);
  pLimit2 = sqlite3_व्यंजक_Dup(db, pLimit, 0);
#अन्यथा
  UNUSED_PARAMETER(pOrderBy);
  UNUSED_PARAMETER(pLimit);
#पूर्ण_अगर

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_व्यंजक_Dup(db, pWhere, 0);

  निश्चित( pTabList->nSrc>1 );
  अगर( pSrc ){
    निश्चित( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  अगर( pPk ){
    क्रम(i=0; i<pPk->n_कुंजी_Col; i++){
      व्यंजक *pNew = व्यंजक_Row_Column(pParse, pPk->aiColumn[i]);
#अगर_घोषित SQLITE_ENABLE_UPDATE_DELETE_LIMIT
      अगर( pLimit ){
        pGrp = sqlite3_व्यंजक_List_Append(pParse, pGrp, sqlite3_व्यंजक_Dup(db, pNew, 0));
      }
#पूर्ण_अगर
      pList = sqlite3_व्यंजक_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_Table : SRT_Upfrom;
  }अन्यथा अगर( IsView(pTab) ){
    क्रम(i=0; i<pTab->nCol; i++){
      pList = sqlite3_व्यंजक_List_Append(pParse, pList, व्यंजक_Row_Column(pParse, i));
    }
    eDest = SRT_Table;
  }अन्यथा{
    eDest = IsVirtual(pTab) ? SRT_Table : SRT_Upfrom;
    pList = sqlite3_व्यंजक_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
#अगर_घोषित SQLITE_ENABLE_UPDATE_DELETE_LIMIT
    अगर( pLimit ){
      pGrp = sqlite3_व्यंजक_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
    }
#पूर्ण_अगर
  }
  निश्चित( pChanges!=0 || pParse->db->mallocFailed );
  अगर( pChanges ){
    क्रम(i=0; i<pChanges->n_व्यंजक; i++){
      pList = sqlite3_व्यंजक_List_Append(pParse, pList,
          sqlite3_व्यंजक_Dup(db, pChanges->a[i].p_व्यंजक, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, pOrderBy2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  अगर( pSelect ) pSelect->selFlags |= SF_OrderByReqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_कुंजी_Col : -1);
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
व्योम sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  व्यंजक_List *pChanges,    /* Things to be changed */
  व्यंजक *pWhere,          /* The WHERE clause.  May be null */
  पूर्णांक onError,           /* How to handle constraint errors */
  व्यंजक_List *pOrderBy,    /* ORDER BY clause. May be null */
  व्यंजक *pLimit,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  पूर्णांक i, j, k;           /* Loop counters */
  Table *pTab;           /* The table to be updated */
  पूर्णांक addr_शीर्ष = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  सूचकांक *pIdx;           /* For looping over indices */
  सूचकांक *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  पूर्णांक nIdx;              /* Number of indices that need updating */
  पूर्णांक nAllIdx;           /* Total number of indexes */
  पूर्णांक iBaseCur;          /* Base cursor number */
  पूर्णांक i_तथ्य_Cur;          /* Cursor for the canonical data btree */
  पूर्णांक iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  पूर्णांक *aRegIdx = 0;      /* Registers for to each index and the main table */
  पूर्णांक *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *aToOpen;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_कुंजी;            /* Either chngPk or chngRowid */
  व्यंजक *p_Rowid_व्यंजक = 0;  /* Expression defining the new record number */
  पूर्णांक i_Rowid_व्यंजक = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  पूर्णांक iDb;               /* Database containing the table being updated */
  पूर्णांक eOnePass;          /* ONEPASS_XXX value from where.c */
  पूर्णांक hasFK;             /* True if foreign key processing is required */
  पूर्णांक labelBreak;        /* Jump here to break out of UPDATE loop */
  पूर्णांक labelContinue;     /* Jump here to continue next step of UPDATE loop */
  पूर्णांक flags;             /* Flags for sqlite3WhereBegin() */

#अगर_अघोषित SQLITE_OMIT_TRIGGER
  पूर्णांक isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  पूर्णांक tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#पूर्ण_अगर
  पूर्णांक newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  पूर्णांक iEph = 0;          /* Ephemeral table holding all primary key values */
  पूर्णांक n_कुंजी = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  पूर्णांक aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  पूर्णांक addrOpen = 0;      /* Address of OP_OpenEphemeral */
  पूर्णांक iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  पूर्णांक bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  पूर्णांक bFinishSeek = 1;   /* The OP_FinishSeek opcode is needed */
  पूर्णांक nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  पूर्णांक reg_Row_गिनती = 0;   /* A count of rows changed */
  पूर्णांक regOldRowid = 0;   /* The old rowid */
  पूर्णांक regNewRowid = 0;   /* The new rowid */
  पूर्णांक regNew = 0;        /* Content of the NEW.* table in triggers */
  पूर्णांक regOld = 0;        /* Content of OLD.* table in triggers */
  पूर्णांक regRowSet = 0;     /* Rowset of rows to be updated */
  पूर्णांक reg_कुंजी = 0;        /* composite PRIMARY KEY value */

  स_रखो(&sContext, 0, माप(sContext));
  db = pParse->db;
  निश्चित( db->pParse==pParse );
  अगर( pParse->nErr ){
    जाओ update_cleanup;
  }
  निश्चित( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  अगर( pTab==0 ) जाओ update_cleanup;
  iDb = sqlite3_Schema_To_सूचकांक(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#अगर_अघोषित SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  निश्चित( pTrigger || tmask==0 );
#अन्यथा
# define pTrigger 0
# define isView 0
# define tmask 0
#पूर्ण_अगर
#अगर_घोषित SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#पूर्ण_अगर

#अगर TREETRACE_ENABLED
  अगर( sqlite3_वृक्ष_Trace & 0x10000 ){
    sqlite3_वृक्ष_View_पंक्ति(0, "In sqlite3Update() at %s:%d", __खाता__, __LINE__);
    sqlite3_वृक्ष_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, pOrderBy, pLimit, pUpsert, pTrigger);
  }
#पूर्ण_अगर

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_व्यंजक : 0;
  निश्चित( nChangeFrom==0 || pUpsert==0 );

#अगर_घोषित SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  अगर( !isView && nChangeFrom==0 ){
    pWhere = sqlite3LimitWhere(
        pParse, pTabList, pWhere, pOrderBy, pLimit, "UPDATE"
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#पूर्ण_अगर

  अगर( sqlite3ViewGetColumnNames(pParse, pTab) ){
    जाओ update_cleanup;
  }
  अगर( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    जाओ update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  iBaseCur = i_तथ्य_Cur = pParse->nTab++;
  iIdxCur = i_तथ्य_Cur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_कुंजी_सूचकांक(pTab);
  testcase( pPk!=0 && pPk!=pTab->p_सूचकांक );
  क्रम(nIdx=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, nIdx++){
    अगर( pPk==pIdx ){
      i_तथ्य_Cur = pParse->nTab;
    }
    pParse->nTab++;
  }
  अगर( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    i_तथ्य_Cur = pUpsert->i_तथ्य_Cur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = iBaseCur;
  }
  pTabList->a[0].iCursor = i_तथ्य_Cur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, माप(पूर्णांक) * (pTab->nCol+nIdx+1) + nIdx+2 );
  अगर( aXRef==0 ) जाओ update_cleanup;
  aRegIdx = aXRef+pTab->nCol;
  aToOpen = (u8*)(aRegIdx+nIdx+1);
  स_रखो(aToOpen, 1, nIdx+1);
  aToOpen[nIdx+1] = 0;
  क्रम(i=0; i<pTab->nCol; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  स_रखो(&sNC, 0, माप(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  अगर( v==0 ) जाओ update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  क्रम(i=0; i<pChanges->n_व्यंजक; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    अगर( nChangeFrom==0 && sqlite3_Resolve_व्यंजक_Names(&sNC, pChanges->a[i].p_व्यंजक) ){
      जाओ update_cleanup;
    }
    j = sqlite3_Column_सूचकांक(pTab, pChanges->a[i].zEName);
    अगर( j>=0 ){
      अगर( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_व्यंजक = pChanges->a[i].p_व्यंजक;
        i_Rowid_व्यंजक = i;
      }अन्यथा अगर( pPk && (pTab->aCol[j].colFlags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
      अन्यथा अगर( pTab->aCol[j].colFlags & COLFLAG_GENERATED ){
        testcase( pTab->aCol[j].colFlags & COLFLAG_VIRTUAL );
        testcase( pTab->aCol[j].colFlags & COLFLAG_STORED );
        sqlite3_Error_संदेश(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->aCol[j].zCnName);
        जाओ update_cleanup;
      }
#पूर्ण_अगर
      aXRef[j] = i;
    }अन्यथा{
      अगर( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_व्यंजक = pChanges->a[i].p_व्यंजक;
        i_Rowid_व्यंजक = i;
      }अन्यथा{
        sqlite3_Error_संदेश(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        जाओ update_cleanup;
      }
    }
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
    {
      पूर्णांक rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->aCol[j].zCnName,
                            db->aDb[iDb].zDbSName);
      अगर( rc!=SQLITE_OK ){
        अगर( rc==SQLITE_DENY ){
          जाओ update_cleanup;
        }अन्यथा अगर( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }अन्यथा{
          aXRef[j] = -1;
        }
      }
    }
#पूर्ण_अगर
  }
  निश्चित( (chngRowid & chngPk)==0 );
  निश्चित( chngRowid==0 || chngRowid==1 );
  निश्चित( chngPk==0 || chngPk==1 );
  chng_कुंजी = chngRowid + chngPk;

#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  अगर( pTab->tabFlags & TF_HasGenerated ){
    पूर्णांक bProgress;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    करो{
      bProgress = 0;
      क्रम(i=0; i<pTab->nCol; i++){
        अगर( aXRef[i]>=0 ) जारी;
        अगर( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 ) जारी;
        अगर( sqlite3_व्यंजक_References_Updated_Column(
                sqlite3_Column_व्यंजक(pTab, &pTab->aCol[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          bProgress = 1;
        }
      }
    }जबतक( bProgress );
  }
#पूर्ण_अगर

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].colUsed = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_कुंजी);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  अगर( onError==OE_Replace ) bReplace = 1;
  क्रम(nAllIdx=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, nAllIdx++){
    पूर्णांक reg;
    अगर( chng_कुंजी || hasFK>1 || pIdx==pPk
     || सूचकांक_Where_Clause_Might_Change(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    }अन्यथा{
      reg = 0;
      क्रम(i=0; i<pIdx->n_कुंजी_Col; i++){
        अगर( सूचकांक_Column_Is_Being_Updated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->nColumn;
          अगर( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          अवरोध;
        }
      }
    }
    अगर( reg==0 ) aToOpen[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  अगर( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    स_रखो(aToOpen, 1, nIdx+1);
  }

  अगर( pParse->nested==0 ) sqlite3_Vdbe_गिनती_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  अगर( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    निश्चित( aRegIdx[nAllIdx]==pParse->nMem );
    regRowSet = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    अगर( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->nCol;
    }
    अगर( chng_कुंजी || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->nCol;
  }

  /* Start the view context. */
  अगर( isView ){
    sqlite3_Auth_Context_धकेलो(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#अगर !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  अगर( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, pOrderBy, pLimit, i_तथ्य_Cur
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#पूर्ण_अगर

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  अगर( nChangeFrom==0 && sqlite3_Resolve_व्यंजक_Names(&sNC, pWhere) ){
    जाओ update_cleanup;
  }

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  अगर( IsVirtual(pTab) ){
    updateVirtualTable(pParse, pTabList, pTab, pChanges, p_Rowid_व्यंजक, aXRef,
                       pWhere, onError);
    जाओ update_cleanup;
  }
#पूर्ण_अगर

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  अगर( (db->flags&SQLITE_गिनती_Rows)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_Row_गिनती = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, reg_Row_गिनती);
  }

  अगर( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3VdbeAddOp3(v, OP_Null, 0, regRowSet, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3VdbeAddOp3(v, OP_OpenEphemeral, iEph, 0, regRowSet);
  }अन्यथा{
    निश्चित( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_कुंजी_Col : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_कुंजी = ++pParse->nMem;
    अगर( pUpsert==0 ){
      पूर्णांक nEphCol = nPk + nChangeFrom + (isView ? pTab->nCol : 0);
      iEph = pParse->nTab++;
      अगर( pPk ) sqlite3VdbeAddOp3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iEph, nEphCol);
      अगर( pPk ){
        कुंजी_Info *p_कुंजी_Info = sqlite3_कुंजी_Info_Of_सूचकांक(pParse, pPk);
        अगर( p_कुंजी_Info ){
          p_कुंजी_Info->nAllField = nEphCol;
          sqlite3VdbeAppendP4(v, p_कुंजी_Info, P4_KEYINFO);
        }
      }
      अगर( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, pOrderBy, pLimit
        );
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
        अगर( isView ) i_तथ्य_Cur = iEph;
#पूर्ण_अगर
      }
    }
  }

  अगर( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_कुंजी = nPk;
    reg_कुंजी = iPk;
  }अन्यथा{
    अगर( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_व्यंजक_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
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
      अगर( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_कुंजी
       && !bReplace
       && (pWhere==0 || !व्यंजक_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      अगर( pWInfo==0 ) जाओ update_cleanup;

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
      अगर( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        अगर( eOnePass==ONEPASS_MULTI ){
          पूर्णांक iCur = aiCurOnePass[1];
          अगर( iCur>=0 && iCur!=i_तथ्य_Cur && aToOpen[iCur-iBaseCur] ){
            eOnePass = ONEPASS_OFF;
          }
          निश्चित( iCur!=i_तथ्य_Cur || !HasRowid(pTab) );
        }
      }
    }

    अगर( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3VdbeAddOp2(v, OP_Rowid, i_तथ्य_Cur, regOldRowid);
      अगर( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3VdbeAddOp3(v, OP_जोड़ो, iEph, regRowSet, regOldRowid);
      }अन्यथा{
        अगर( ALWAYS(addrOpen) ) sqlite3VdbeChangeToNoop(v, addrOpen);
      }
    }अन्यथा{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      क्रम(i=0; i<nPk; i++){
        निश्चित( pPk->aiColumn[i]>=0 );
        sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, i_तथ्य_Cur,
                                        pPk->aiColumn[i], iPk+i);
      }
      अगर( eOnePass ){
        अगर( addrOpen ) sqlite3VdbeChangeToNoop(v, addrOpen);
        n_कुंजी = nPk;
        reg_कुंजी = iPk;
      }अन्यथा{
        sqlite3VdbeAddOp4(v, OP_MakeRecord, iPk, nPk, reg_कुंजी,
                          sqlite3_सूचकांक_Affinity_Str(db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, OP_Idx_जोड़ो, iEph, reg_कुंजी, iPk, nPk);
      }
    }
  }

  अगर( pUpsert==0 ){
    अगर( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3WhereEnd(pWInfo);
    }

    अगर( !isView ){
      पूर्णांक addrOnce = 0;
      पूर्णांक iNotUsed1 = 0;
      पूर्णांक iNotUsed2 = 0;

      /* Open every index that needs updating. */
      अगर( eOnePass!=ONEPASS_OFF ){
        अगर( aiCurOnePass[0]>=0 ) aToOpen[aiCurOnePass[0]-iBaseCur] = 0;
        अगर( aiCurOnePass[1]>=0 ) aToOpen[aiCurOnePass[1]-iBaseCur] = 0;
      }

      अगर( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, 0, iBaseCur,
                                 aToOpen, &iNotUsed1, &iNotUsed2);
      अगर( addrOnce ){
        sqlite3_Vdbe_Jump_Here_Or_निकालो_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    अगर( eOnePass!=ONEPASS_OFF ){
      अगर( aiCurOnePass[0]!=i_तथ्य_Cur
       && aiCurOnePass[1]!=i_तथ्य_Cur
#अगर_घोषित SQLITE_ALLOW_ROWID_IN_VIEW
       && !isView
#पूर्ण_अगर
      ){
        निश्चित( pPk );
        sqlite3VdbeAddOp4Int(v, OP_NotFound, i_तथ्य_Cur, labelBreak, reg_कुंजी,n_कुंजी);
        VdbeCoverage(v);
      }
      अगर( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3VdbeAddOp2(v, OP_IsNull, pPk ? reg_कुंजी : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }अन्यथा अगर( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_शीर्ष = sqlite3VdbeCurrentAddr(v);
      अगर( nChangeFrom ){
        अगर( !isView ){
          अगर( pPk ){
            क्रम(i=0; i<nPk; i++){
              sqlite3VdbeAddOp3(v, OP_Column, iEph, i, iPk+i);
            }
            sqlite3VdbeAddOp4Int(
                v, OP_NotFound, i_तथ्य_Cur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }अन्यथा{
            sqlite3VdbeAddOp2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3VdbeAddOp3(
                v, OP_NotExists, i_तथ्य_Cur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Row_तथ्य, iEph, reg_कुंजी);
        sqlite3VdbeAddOp4Int(v, OP_NotFound, i_तथ्य_Cur, labelContinue, reg_कुंजी,0);
        VdbeCoverage(v);
      }
    }अन्यथा{
      sqlite3VdbeAddOp2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_शीर्ष = sqlite3VdbeAddOp2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_NotExists, i_तथ्य_Cur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  निश्चित( chng_कुंजी || pTrigger || hasFK || regOldRowid==regNewRowid );
  अगर( chngRowid ){
    निश्चित( i_Rowid_व्यंजक>=0 );
    अगर( nChangeFrom==0 ){
      sqlite3_व्यंजक_Code(pParse, p_Rowid_व्यंजक, regNewRowid);
    }अन्यथा{
      sqlite3VdbeAddOp3(v, OP_Column, iEph, i_Rowid_व्यंजक, regNewRowid);
    }
    sqlite3VdbeAddOp1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  अगर( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    क्रम(i=0; i<pTab->nCol; i++){
      u32 colFlags = pTab->aCol[i].colFlags;
      k = sqlite3TableColumnToStorage(pTab, i) + regOld;
      अगर( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (colFlags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, i_तथ्य_Cur, i, k);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, k);
      }
    }
    अगर( chngRowid==0 && pPk==0 ){
#अगर_घोषित SQLITE_ALLOW_ROWID_IN_VIEW
      अगर( isView ) sqlite3VdbeAddOp2(v, OP_Null, 0, regOldRowid);
#पूर्ण_अगर
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
  क्रम(i=0, k=regNew; i<pTab->nCol; i++, k++){
    अगर( i==pTab->iPKey ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, k);
    }अन्यथा अगर( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)!=0 ){
      अगर( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
    }अन्यथा{
      j = aXRef[i];
      अगर( j>=0 ){
        अगर( nChangeFrom ){
          पूर्णांक nOff = (isView ? pTab->nCol : nPk);
          निश्चित( eOnePass==ONEPASS_OFF );
          sqlite3VdbeAddOp3(v, OP_Column, iEph, nOff+j, k);
        }अन्यथा{
          sqlite3_व्यंजक_Code(pParse, pChanges->a[j].p_व्यंजक, k);
        }
      }अन्यथा अगर( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, i_तथ्य_Cur, i, k);
        bFinishSeek = 0;
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, k);
      }
    }
  }
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
  अगर( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#पूर्ण_अगर

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  अगर( tmask&TRIGGER_BEFORE ){
    sqlite3TableAffinity(v, pTab, regNew);
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    अगर( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      अगर( pPk ){
        sqlite3VdbeAddOp4Int(v, OP_NotFound,i_तथ्य_Cur,labelContinue,reg_कुंजी,n_कुंजी);
        VdbeCoverage(v);
      }अन्यथा{
        sqlite3VdbeAddOp3(v, OP_NotExists, i_तथ्य_Cur, labelContinue,regOldRowid);
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
      क्रम(i=0, k=regNew; i<pTab->nCol; i++, k++){
        अगर( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
          अगर( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
        }अन्यथा अगर( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, i_तथ्य_Cur, i, k);
        }
      }
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
      अगर( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#पूर्ण_अगर
    }
  }

  अगर( !isView ){
    /* Do constraint checks. */
    निश्चित( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_तथ्य_Cur, iIdxCur,
        regNewRowid, regOldRowid, chng_कुंजी, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    अगर( bReplace || chng_कुंजी ){
      अगर( pPk ){
        sqlite3VdbeAddOp4Int(v, OP_NotFound,i_तथ्य_Cur,labelContinue,reg_कुंजी,n_कुंजी);
      }अन्यथा{
        sqlite3VdbeAddOp3(v, OP_NotExists, i_तथ्य_Cur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    अगर( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_कुंजी);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3_Generate_Row_सूचकांक_Delete(pParse, pTab, i_तथ्य_Cur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    अगर( bFinishSeek ){
      sqlite3VdbeAddOp1(v, OP_FinishSeek, i_तथ्य_Cur);
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
    निश्चित( regNew==regNewRowid+1 );
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeAddOp3(v, OP_Delete, i_तथ्य_Cur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_कुंजी) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    अगर( eOnePass==ONEPASS_MULTI ){
      निश्चित( hasFK==0 && chng_कुंजी==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    अगर( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
    }
#अन्यथा
    अगर( hasFK>1 || chng_कुंजी ){
      sqlite3VdbeAddOp2(v, OP_Delete, i_तथ्य_Cur, 0);
    }
#पूर्ण_अगर

    अगर( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_कुंजी);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, i_तथ्य_Cur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    अगर( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_कुंजी);
    }
  }

  /* Increment the row counter
  */
  अगर( reg_Row_गिनती ){
    sqlite3VdbeAddOp2(v, OP_AddImm, reg_Row_गिनती, 1);
  }

  अगर( pTrigger ){
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  अगर( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }अन्यथा अगर( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3WhereEnd(pWInfo);
  }अन्यथा{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3VdbeAddOp2(v, OP_अग्र, iEph, addr_शीर्ष); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  अगर( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  अगर( reg_Row_गिनती ){
    sqlite3_Code_Change_गिनती(v, reg_Row_गिनती, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_निकालो(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_व्यंजक_List_Delete(db, pChanges);
  sqlite3_व्यंजक_Delete(db, pWhere);
#अगर defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT)
  sqlite3_व्यंजक_List_Delete(db, pOrderBy);
  sqlite3_व्यंजक_Delete(db, pLimit);
#पूर्ण_अगर
  वापस;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#अगर_घोषित isView
 #अघोषित isView
#पूर्ण_अगर
#अगर_घोषित pTrigger
 #अघोषित pTrigger
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
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
अटल व्योम updateVirtualTable(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  Table *pTab,         /* The virtual table */
  व्यंजक_List *pChanges,  /* The columns to change in the UPDATE statement */
  व्यंजक *pRowid,        /* Expression used to recompute the rowid */
  पूर्णांक *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  व्यंजक *pWhere,        /* WHERE clause of the UPDATE statement */
  पूर्णांक onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  पूर्णांक ephemTab;             /* Table holding the result of the SELECT */
  पूर्णांक i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  स्थिर अक्षर *pVTab = (स्थिर अक्षर*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  पूर्णांक nArg = 2 + pTab->nCol;      /* Number of arguments to VUpdate */
  पूर्णांक regArg;                     /* First register in VUpdate arg array */
  पूर्णांक regRec;                     /* Register in which to assemble record */
  पूर्णांक regRowid;                   /* Register for ephemeral table rowid */
  पूर्णांक iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  पूर्णांक aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  पूर्णांक eOnePass;                   /* True to use onepass strategy */
  पूर्णांक addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  निश्चित( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3VdbeAddOp2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  अगर( pSrc->nSrc>1 ){
    सूचकांक *pPk = 0;
    व्यंजक *pRow;
    व्यंजक_List *pList;
    अगर( HasRowid(pTab) ){
      अगर( pRowid ){
        pRow = sqlite3_व्यंजक_Dup(db, pRowid, 0);
      }अन्यथा{
        pRow = sqlite3PExpr(pParse, TK_ROW, 0, 0);
      }
    }अन्यथा{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
      निश्चित( pPk!=0 );
      निश्चित( pPk->n_कुंजी_Col==1 );
      iPk = pPk->aiColumn[0];
      अगर( aXRef[iPk]>=0 ){
        pRow = sqlite3_व्यंजक_Dup(db, pChanges->a[aXRef[iPk]].p_व्यंजक, 0);
      }अन्यथा{
        pRow = व्यंजक_Row_Column(pParse, iPk);
      }
    }
    pList = sqlite3_व्यंजक_List_Append(pParse, 0, pRow);

    क्रम(i=0; i<pTab->nCol; i++){
      अगर( aXRef[i]>=0 ){
        pList = sqlite3_व्यंजक_List_Append(pParse, pList,
          sqlite3_व्यंजक_Dup(db, pChanges->a[aXRef[i]].p_व्यंजक, 0)
        );
      }अन्यथा{
        व्यंजक *p_Row_व्यंजक = व्यंजक_Row_Column(pParse, i);
        अगर( p_Row_व्यंजक ) p_Row_व्यंजक->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_व्यंजक_List_Append(pParse, pList, p_Row_व्यंजक);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_व्यंजक_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }अन्यथा{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    अगर( pWInfo==0 ) वापस;

    /* Populate the argument registers. */
    क्रम(i=0; i<pTab->nCol; i++){
      निश्चित( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 );
      अगर( aXRef[i]>=0 ){
        sqlite3_व्यंजक_Code(pParse, pChanges->a[aXRef[i]].p_व्यंजक, regArg+2+i);
      }अन्यथा{
        sqlite3VdbeAddOp3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    अगर( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCsr, regArg);
      अगर( pRowid ){
        sqlite3_व्यंजक_Code(pParse, pRowid, regArg+1);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Rowid, iCsr, regArg+1);
      }
    }अन्यथा{
      सूचकांक *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
      निश्चित( pPk!=0 );
      निश्चित( pPk->n_कुंजी_Col==1 );
      iPk = pPk->aiColumn[0];
      sqlite3VdbeAddOp3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3VdbeAddOp2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    निश्चित( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    अगर( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3VdbeChangeToNoop(v, addr);
      sqlite3VdbeAddOp1(v, OP_Close, iCsr);
    }अन्यथा{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regArg, nArg, regRec);
#अगर defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#पूर्ण_अगर
      sqlite3VdbeAddOp2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3VdbeAddOp3(v, OP_जोड़ो, ephemTab, regRec, regRowid);
    }
  }


  अगर( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    अगर( pSrc->nSrc==1 ){
      sqlite3WhereEnd(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    क्रम(i=0; i<nArg; i++){
      sqlite3VdbeAddOp3(v, OP_Column, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3VdbeAddOp4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  अगर( eOnePass==ONEPASS_OFF ){
    sqlite3VdbeAddOp2(v, OP_अग्र, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    sqlite3VdbeAddOp2(v, OP_Close, ephemTab, 0);
  }अन्यथा{
    sqlite3WhereEnd(pWInfo);
  }
}
#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */
