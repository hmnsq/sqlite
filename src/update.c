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

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
स्थिर शून्य updateVirtualTable(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  Table *pTab,         /* The virtual table */
  अभिव्यक्ति_List *pChanges,  /* The columns to change in the UPDATE statement */
  अभिव्यक्ति *p_Rowid_अभिव्यक्ति,    /* Expression used to recompute the rowid */
  पूर्णाङ्क *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  अभिव्यक्ति *pWhere,        /* WHERE clause of the UPDATE statement */
  पूर्णाङ्क onError          /* ON CONFLICT strategy */
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
स्थिर SQLITE_NOINLINE शून्य columnDefaultUncommonCase(
  Vdbe *v,          /* Byte code under construction */
  Table *pTab,      /* The table */
  Column *pCol      /* Which column of the table */
){
  sqlite3_मान *p_मान = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  प्रतिज्ञा( !IsView(pTab) );
  sqlite3_मान_From_अभिव्यक्ति(sqlite3VdbeDb(v),
                       sqlite3_Column_अभिव्यक्ति_Auth(pTab,pCol,pParse), enc,
                       pCol->affinity, &p_मान);
  यदि( p_मान ){
    sqlite3VdbeAppendP4(v, p_मान, P4_MEM);
  }
}
शून्य sqlite3ColumnDefault(
  Vdbe *v,          /* Byte code under construction */
  Table *pTab,      /* The table */
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
#ifndef SQLITE_OMIT_FLOATING_POINT
  यदि( pCol->affinity==SQLITE_AFF_REAL ){
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
स्थिर पूर्णाङ्क indexColumnIsBeingUpdated(
  Index *pIdx,      /* The index to check */
  पूर्णाङ्क iCol,         /* Which column of the index to check */
  पूर्णाङ्क *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  पूर्णाङ्क chngRowid     /* true if the rowid is being updated */
){
  i16 iIdxCol = pIdx->aiColumn[iCol];
  प्रतिज्ञा( iIdxCol!=XN_ROWID ); /* Cannot index rowid */
  यदि( iIdxCol>=0 ){
    फर्काऊ aXRef[iIdxCol]>=0;
  }
  प्रतिज्ञा( iIdxCol==XN_अभिव्यक्ति );
  प्रतिज्ञा( pIdx->a_Col_अभिव्यक्ति!=0 );
  प्रतिज्ञा( pIdx->a_Col_अभिव्यक्ति->a[iCol].p_अभिव्यक्ति!=0 );
  फर्काऊ sqlite3_अभिव्यक्ति_References_Updated_Column(pIdx->a_Col_अभिव्यक्ति->a[iCol].p_अभिव्यक्ति,
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
स्थिर पूर्णाङ्क indexWhereClauseMightChange(
  Index *pIdx,      /* The index to check */
  पूर्णाङ्क *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  पूर्णाङ्क chngRowid     /* true if the rowid is being updated */
){
  यदि( pIdx->pPartIdxWhere==0 ) फर्काऊ 0;
  फर्काऊ sqlite3_अभिव्यक्ति_References_Updated_Column(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
स्थिर अभिव्यक्ति *अभिव्यक्ति_Row_Column(Parse *pParse, पूर्णाङ्क iCol){
  अभिव्यक्ति *pRet = sqlite3PExpr(pParse, TK_ROW, 0, 0);
  यदि( pRet ) pRet->iColumn = iCol+1;
  फर्काऊ pRet;
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
  Index *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  अभिव्यक्ति_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  अभिव्यक्ति *pWhere,                   /* WHERE clause for query */
  अभिव्यक्ति_List *pOrderBy,             /* ORDER BY clause */
  अभिव्यक्ति *pLimit                    /* LIMIT clause */
){
  पूर्णाङ्क i;
  SelectDest dest;
  Select *pSelect = 0;
  अभिव्यक्ति_List *pList = 0;
  अभिव्यक्ति_List *pGrp = 0;
  अभिव्यक्ति *pLimit2 = 0;
  अभिव्यक्ति_List *pOrderBy2 = 0;
  sqlite3 *db = pParse->db;
  Table *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  अभिव्यक्ति *pWhere2;
  पूर्णाङ्क eDest;

#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  यदि( pOrderBy && pLimit==0 ) {
    sqlite3_Error_सन्देश(pParse, "ORDER BY without LIMIT on UPDATE");
    फर्काऊ;
  }
  pOrderBy2 = sqlite3_अभिव्यक्ति_List_Dup(db, pOrderBy, 0);
  pLimit2 = sqlite3_अभिव्यक्ति_Dup(db, pLimit, 0);
#अन्यथा
  UNUSED_PARAMETER(pOrderBy);
  UNUSED_PARAMETER(pLimit);
#endif

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_अभिव्यक्ति_Dup(db, pWhere, 0);

  प्रतिज्ञा( pTabList->nSrc>1 );
  यदि( pSrc ){
    प्रतिज्ञा( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  यदि( pPk ){
    लागि(i=0; i<pPk->n_साँचो_Col; i++){
      अभिव्यक्ति *pNew = अभिव्यक्ति_Row_Column(pParse, pPk->aiColumn[i]);
#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
      यदि( pLimit ){
        pGrp = sqlite3_अभिव्यक्ति_List_Append(pParse, pGrp, sqlite3_अभिव्यक्ति_Dup(db, pNew, 0));
      }
#endif
      pList = sqlite3_अभिव्यक्ति_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_Table : SRT_Upfrom;
  }अन्यथा यदि( IsView(pTab) ){
    लागि(i=0; i<pTab->nCol; i++){
      pList = sqlite3_अभिव्यक्ति_List_Append(pParse, pList, अभिव्यक्ति_Row_Column(pParse, i));
    }
    eDest = SRT_Table;
  }अन्यथा{
    eDest = IsVirtual(pTab) ? SRT_Table : SRT_Upfrom;
    pList = sqlite3_अभिव्यक्ति_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
    यदि( pLimit ){
      pGrp = sqlite3_अभिव्यक्ति_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ROW,0,0));
    }
#endif
  }
  प्रतिज्ञा( pChanges!=0 || pParse->db->mallocFailed );
  यदि( pChanges ){
    लागि(i=0; i<pChanges->n_अभिव्यक्ति; i++){
      pList = sqlite3_अभिव्यक्ति_List_Append(pParse, pList,
          sqlite3_अभिव्यक्ति_Dup(db, pChanges->a[i].p_अभिव्यक्ति, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, pOrderBy2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  यदि( pSelect ) pSelect->selFlags |= SF_OrderByReqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_साँचो_Col : -1);
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
  अभिव्यक्ति_List *pChanges,    /* Things to be changed */
  अभिव्यक्ति *pWhere,          /* The WHERE clause.  May be null */
  पूर्णाङ्क onError,           /* How to handle constraint errors */
  अभिव्यक्ति_List *pOrderBy,    /* ORDER BY clause. May be null */
  अभिव्यक्ति *pLimit,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  पूर्णाङ्क i, j, k;           /* Loop counters */
  Table *pTab;           /* The table to be updated */
  पूर्णाङ्क addr_शीर्ष = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  Index *pIdx;           /* For looping over indices */
  Index *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  पूर्णाङ्क nIdx;              /* Number of indices that need updating */
  पूर्णाङ्क nAllIdx;           /* Total number of indexes */
  पूर्णाङ्क iBaseCur;          /* Base cursor number */
  पूर्णाङ्क iDataCur;          /* Cursor for the canonical data btree */
  पूर्णाङ्क iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  पूर्णाङ्क *aRegIdx = 0;      /* Registers for to each index and the main table */
  पूर्णाङ्क *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *aToOpen;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_साँचो;            /* Either chngPk or chngRowid */
  अभिव्यक्ति *p_Rowid_अभिव्यक्ति = 0;  /* Expression defining the new record number */
  पूर्णाङ्क i_Rowid_अभिव्यक्ति = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  पूर्णाङ्क iDb;               /* Database containing the table being updated */
  पूर्णाङ्क eOnePass;          /* ONEPASS_XXX value from where.c */
  पूर्णाङ्क hasFK;             /* True if foreign key processing is required */
  पूर्णाङ्क labelBreak;        /* Jump here to break out of UPDATE loop */
  पूर्णाङ्क labelContinue;     /* Jump here to continue next step of UPDATE loop */
  पूर्णाङ्क flags;             /* Flags for sqlite3WhereBegin() */

#ifndef SQLITE_OMIT_TRIGGER
  पूर्णाङ्क isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  पूर्णाङ्क tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#endif
  पूर्णाङ्क newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  पूर्णाङ्क iEph = 0;          /* Ephemeral table holding all primary key values */
  पूर्णाङ्क n_साँचो = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  पूर्णाङ्क aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  पूर्णाङ्क addrOpen = 0;      /* Address of OP_OpenEphemeral */
  पूर्णाङ्क iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  पूर्णाङ्क bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  पूर्णाङ्क bFinishSeek = 1;   /* The OP_FinishSeek opcode is needed */
  पूर्णाङ्क nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  पूर्णाङ्क reg_Row_गन्ती = 0;   /* A count of rows changed */
  पूर्णाङ्क regOldRowid = 0;   /* The old rowid */
  पूर्णाङ्क regNewRowid = 0;   /* The new rowid */
  पूर्णाङ्क regNew = 0;        /* Content of the NEW.* table in triggers */
  पूर्णाङ्क regOld = 0;        /* Content of OLD.* table in triggers */
  पूर्णाङ्क regRowSet = 0;     /* Rowset of rows to be updated */
  पूर्णाङ्क reg_साँचो = 0;        /* composite PRIMARY KEY value */

  स_रखो(&sContext, 0, परिमाण(sContext));
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
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#ifndef SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  प्रतिज्ञा( pTrigger || tmask==0 );
#अन्यथा
# define pTrigger 0
# define isView 0
# define tmask 0
#endif
#ifdef SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#endif

#यदि TREETRACE_ENABLED
  यदि( sqlite3_रूख_Trace & 0x10000 ){
    sqlite3_रूख_View_पङ्क्ति(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_रूख_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, pOrderBy, pLimit, pUpsert, pTrigger);
  }
#endif

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_अभिव्यक्ति : 0;
  प्रतिज्ञा( nChangeFrom==0 || pUpsert==0 );

#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  यदि( !isView && nChangeFrom==0 ){
    pWhere = sqlite3LimitWhere(
        pParse, pTabList, pWhere, pOrderBy, pLimit, "UPDATE"
    );
    pOrderBy = 0;
    pLimit = 0;
  }
#endif

  यदि( sqlite3ViewGetColumnNames(pParse, pTab) ){
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
  iBaseCur = iDataCur = pParse->nTab++;
  iIdxCur = iDataCur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_साँचो_Index(pTab);
  testcase( pPk!=0 && pPk!=pTab->pIndex );
  लागि(nIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nIdx++){
    यदि( pPk==pIdx ){
      iDataCur = pParse->nTab;
    }
    pParse->nTab++;
  }
  यदि( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    iDataCur = pUpsert->iDataCur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = iBaseCur;
  }
  pTabList->a[0].iCursor = iDataCur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, परिमाण(पूर्णाङ्क) * (pTab->nCol+nIdx+1) + nIdx+2 );
  यदि( aXRef==0 ) गच्छ update_cleanup;
  aRegIdx = aXRef+pTab->nCol;
  aToOpen = (u8*)(aRegIdx+nIdx+1);
  स_रखो(aToOpen, 1, nIdx+1);
  aToOpen[nIdx+1] = 0;
  लागि(i=0; i<pTab->nCol; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  स_रखो(&sNC, 0, परिमाण(sNC));
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
  लागि(i=0; i<pChanges->n_अभिव्यक्ति; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    यदि( nChangeFrom==0 && sqlite3_Resolve_अभिव्यक्ति_Names(&sNC, pChanges->a[i].p_अभिव्यक्ति) ){
      गच्छ update_cleanup;
    }
    j = sqlite3ColumnIndex(pTab, pChanges->a[i].zEName);
    यदि( j>=0 ){
      यदि( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_अभिव्यक्ति = pChanges->a[i].p_अभिव्यक्ति;
        i_Rowid_अभिव्यक्ति = i;
      }अन्यथा यदि( pPk && (pTab->aCol[j].colFlags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
      अन्यथा यदि( pTab->aCol[j].colFlags & COLFLAG_GENERATED ){
        testcase( pTab->aCol[j].colFlags & COLFLAG_VIRTUAL );
        testcase( pTab->aCol[j].colFlags & COLFLAG_STORED );
        sqlite3_Error_सन्देश(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->aCol[j].zCnName);
        गच्छ update_cleanup;
      }
#endif
      aXRef[j] = i;
    }अन्यथा{
      यदि( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_अभिव्यक्ति = pChanges->a[i].p_अभिव्यक्ति;
        i_Rowid_अभिव्यक्ति = i;
      }अन्यथा{
        sqlite3_Error_सन्देश(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        गच्छ update_cleanup;
      }
    }
#ifndef SQLITE_OMIT_AUTHORIZATION
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
#endif
  }
  प्रतिज्ञा( (chngRowid & chngPk)==0 );
  प्रतिज्ञा( chngRowid==0 || chngRowid==1 );
  प्रतिज्ञा( chngPk==0 || chngPk==1 );
  chng_साँचो = chngRowid + chngPk;

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  यदि( pTab->tabFlags & TF_HasGenerated ){
    पूर्णाङ्क bProgress;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    गर{
      bProgress = 0;
      लागि(i=0; i<pTab->nCol; i++){
        यदि( aXRef[i]>=0 ) जारी;
        यदि( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 ) जारी;
        यदि( sqlite3_अभिव्यक्ति_References_Updated_Column(
                sqlite3_Column_अभिव्यक्ति(pTab, &pTab->aCol[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          bProgress = 1;
        }
      }
    }जबसम्म( bProgress );
  }
#endif

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].colUsed = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_साँचो);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  यदि( onError==OE_Replace ) bReplace = 1;
  लागि(nAllIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nAllIdx++){
    पूर्णाङ्क reg;
    यदि( chng_साँचो || hasFK>1 || pIdx==pPk
     || indexWhereClauseMightChange(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    }अन्यथा{
      reg = 0;
      लागि(i=0; i<pIdx->n_साँचो_Col; i++){
        यदि( indexColumnIsBeingUpdated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->nColumn;
          यदि( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          रोक;
        }
      }
    }
    यदि( reg==0 ) aToOpen[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  यदि( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    स_रखो(aToOpen, 1, nIdx+1);
  }

  यदि( pParse->nested==0 ) sqlite3_Vdbe_गन्ती_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  यदि( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    प्रतिज्ञा( aRegIdx[nAllIdx]==pParse->nMem );
    regRowSet = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    यदि( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->nCol;
    }
    यदि( chng_साँचो || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->nCol;
  }

  /* Start the view context. */
  यदि( isView ){
    sqlite3_Auth_Context_थप(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#यदि !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  यदि( nChangeFrom==0 && isView ){
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
  यदि( nChangeFrom==0 && sqlite3_Resolve_अभिव्यक्ति_Names(&sNC, pWhere) ){
    गच्छ update_cleanup;
  }

#ifndef SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  यदि( IsVirtual(pTab) ){
    updateVirtualTable(pParse, pTabList, pTab, pChanges, p_Rowid_अभिव्यक्ति, aXRef,
                       pWhere, onError);
    गच्छ update_cleanup;
  }
#endif

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  यदि( (db->flags&SQLITE_गन्ती_Rows)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_Row_गन्ती = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, reg_Row_गन्ती);
  }

  यदि( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3VdbeAddOp3(v, OP_Null, 0, regRowSet, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3VdbeAddOp3(v, OP_OpenEphemeral, iEph, 0, regRowSet);
  }अन्यथा{
    प्रतिज्ञा( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_साँचो_Col : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_साँचो = ++pParse->nMem;
    यदि( pUpsert==0 ){
      पूर्णाङ्क nEphCol = nPk + nChangeFrom + (isView ? pTab->nCol : 0);
      iEph = pParse->nTab++;
      यदि( pPk ) sqlite3VdbeAddOp3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iEph, nEphCol);
      यदि( pPk ){
        साँचो_Info *p_साँचो_Info = sqlite3_साँचो_Info_Of_Index(pParse, pPk);
        यदि( p_साँचो_Info ){
          p_साँचो_Info->nAllField = nEphCol;
          sqlite3VdbeAppendP4(v, p_साँचो_Info, P4_KEYINFO);
        }
      }
      यदि( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, pOrderBy, pLimit
        );
#ifndef SQLITE_OMIT_SUBQUERY
        यदि( isView ) iDataCur = iEph;
#endif
      }
    }
  }

  यदि( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_साँचो = nPk;
    reg_साँचो = iPk;
  }अन्यथा{
    यदि( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_अभिव्यक्ति_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
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
       && !chng_साँचो
       && !bReplace
       && (pWhere==0 || !अभिव्यक्ति_Has_Property(pWhere, EP_Subquery))
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
      bFinishSeek = sqlite3WhereUsesDeferredSeek(pWInfo);
      यदि( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        यदि( eOnePass==ONEPASS_MULTI ){
          पूर्णाङ्क iCur = aiCurOnePass[1];
          यदि( iCur>=0 && iCur!=iDataCur && aToOpen[iCur-iBaseCur] ){
            eOnePass = ONEPASS_OFF;
          }
          प्रतिज्ञा( iCur!=iDataCur || !HasRowid(pTab) );
        }
      }
    }

    यदि( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3VdbeAddOp2(v, OP_Rowid, iDataCur, regOldRowid);
      यदि( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3VdbeAddOp3(v, OP_घुसाउ, iEph, regRowSet, regOldRowid);
      }अन्यथा{
        यदि( ALWAYS(addrOpen) ) sqlite3VdbeChangeToNoop(v, addrOpen);
      }
    }अन्यथा{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      लागि(i=0; i<nPk; i++){
        प्रतिज्ञा( pPk->aiColumn[i]>=0 );
        sqlite3_अभिव्यक्ति_Code_Get_Column_Of_Table(v, pTab, iDataCur,
                                        pPk->aiColumn[i], iPk+i);
      }
      यदि( eOnePass ){
        यदि( addrOpen ) sqlite3VdbeChangeToNoop(v, addrOpen);
        n_साँचो = nPk;
        reg_साँचो = iPk;
      }अन्यथा{
        sqlite3VdbeAddOp4(v, OP_MakeRecord, iPk, nPk, reg_साँचो,
                          sqlite3IndexAffinityStr(db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, OP_Idx_घुसाउ, iEph, reg_साँचो, iPk, nPk);
      }
    }
  }

  यदि( pUpsert==0 ){
    यदि( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3WhereEnd(pWInfo);
    }

    यदि( !isView ){
      पूर्णाङ्क addrOnce = 0;
      पूर्णाङ्क iNotUsed1 = 0;
      पूर्णाङ्क iNotUsed2 = 0;

      /* Open every index that needs updating. */
      यदि( eOnePass!=ONEPASS_OFF ){
        यदि( aiCurOnePass[0]>=0 ) aToOpen[aiCurOnePass[0]-iBaseCur] = 0;
        यदि( aiCurOnePass[1]>=0 ) aToOpen[aiCurOnePass[1]-iBaseCur] = 0;
      }

      यदि( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, 0, iBaseCur,
                                 aToOpen, &iNotUsed1, &iNotUsed2);
      यदि( addrOnce ){
        sqlite3_Vdbe_Jump_Here_Or_झिक_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    यदि( eOnePass!=ONEPASS_OFF ){
      यदि( aiCurOnePass[0]!=iDataCur
       && aiCurOnePass[1]!=iDataCur
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
       && !isView
#endif
      ){
        प्रतिज्ञा( pPk );
        sqlite3VdbeAddOp4Int(v, OP_NotFound, iDataCur, labelBreak, reg_साँचो,n_साँचो);
        VdbeCoverage(v);
      }
      यदि( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3VdbeAddOp2(v, OP_IsNull, pPk ? reg_साँचो : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }अन्यथा यदि( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_शीर्ष = sqlite3VdbeCurrentAddr(v);
      यदि( nChangeFrom ){
        यदि( !isView ){
          यदि( pPk ){
            लागि(i=0; i<nPk; i++){
              sqlite3VdbeAddOp3(v, OP_Column, iEph, i, iPk+i);
            }
            sqlite3VdbeAddOp4Int(
                v, OP_NotFound, iDataCur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }अन्यथा{
            sqlite3VdbeAddOp2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3VdbeAddOp3(
                v, OP_NotExists, iDataCur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_RowData, iEph, reg_साँचो);
        sqlite3VdbeAddOp4Int(v, OP_NotFound, iDataCur, labelContinue, reg_साँचो,0);
        VdbeCoverage(v);
      }
    }अन्यथा{
      sqlite3VdbeAddOp2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_शीर्ष = sqlite3VdbeAddOp2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_NotExists, iDataCur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  प्रतिज्ञा( chng_साँचो || pTrigger || hasFK || regOldRowid==regNewRowid );
  यदि( chngRowid ){
    प्रतिज्ञा( i_Rowid_अभिव्यक्ति>=0 );
    यदि( nChangeFrom==0 ){
      sqlite3_अभिव्यक्ति_Code(pParse, p_Rowid_अभिव्यक्ति, regNewRowid);
    }अन्यथा{
      sqlite3VdbeAddOp3(v, OP_Column, iEph, i_Rowid_अभिव्यक्ति, regNewRowid);
    }
    sqlite3VdbeAddOp1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  यदि( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    लागि(i=0; i<pTab->nCol; i++){
      u32 colFlags = pTab->aCol[i].colFlags;
      k = sqlite3TableColumnToStorage(pTab, i) + regOld;
      यदि( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (colFlags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_अभिव्यक्ति_Code_Get_Column_Of_Table(v, pTab, iDataCur, i, k);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, k);
      }
    }
    यदि( chngRowid==0 && pPk==0 ){
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
      यदि( isView ) sqlite3VdbeAddOp2(v, OP_Null, 0, regOldRowid);
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
  लागि(i=0, k=regNew; i<pTab->nCol; i++, k++){
    यदि( i==pTab->iPKey ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, k);
    }अन्यथा यदि( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)!=0 ){
      यदि( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
    }अन्यथा{
      j = aXRef[i];
      यदि( j>=0 ){
        यदि( nChangeFrom ){
          पूर्णाङ्क nOff = (isView ? pTab->nCol : nPk);
          प्रतिज्ञा( eOnePass==ONEPASS_OFF );
          sqlite3VdbeAddOp3(v, OP_Column, iEph, nOff+j, k);
        }अन्यथा{
          sqlite3_अभिव्यक्ति_Code(pParse, pChanges->a[j].p_अभिव्यक्ति, k);
        }
      }अन्यथा यदि( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_अभिव्यक्ति_Code_Get_Column_Of_Table(v, pTab, iDataCur, i, k);
        bFinishSeek = 0;
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Null, 0, k);
      }
    }
  }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  यदि( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#endif

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  यदि( tmask&TRIGGER_BEFORE ){
    sqlite3TableAffinity(v, pTab, regNew);
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    यदि( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      यदि( pPk ){
        sqlite3VdbeAddOp4Int(v, OP_NotFound,iDataCur,labelContinue,reg_साँचो,n_साँचो);
        VdbeCoverage(v);
      }अन्यथा{
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
      लागि(i=0, k=regNew; i<pTab->nCol; i++, k++){
        यदि( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
          यदि( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL ) k--;
        }अन्यथा यदि( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_अभिव्यक्ति_Code_Get_Column_Of_Table(v, pTab, iDataCur, i, k);
        }
      }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
      यदि( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#endif
    }
  }

  यदि( !isView ){
    /* Do constraint checks. */
    प्रतिज्ञा( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, iDataCur, iIdxCur,
        regNewRowid, regOldRowid, chng_साँचो, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    यदि( bReplace || chng_साँचो ){
      यदि( pPk ){
        sqlite3VdbeAddOp4Int(v, OP_NotFound,iDataCur,labelContinue,reg_साँचो,n_साँचो);
      }अन्यथा{
        sqlite3VdbeAddOp3(v, OP_NotExists, iDataCur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    यदि( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_साँचो);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    यदि( bFinishSeek ){
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
    प्रतिज्ञा( regNew==regNewRowid+1 );
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeAddOp3(v, OP_Delete, iDataCur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_साँचो) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    यदि( eOnePass==ONEPASS_MULTI ){
      प्रतिज्ञा( hasFK==0 && chng_साँचो==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    यदि( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
    }
#अन्यथा
    यदि( hasFK>1 || chng_साँचो ){
      sqlite3VdbeAddOp2(v, OP_Delete, iDataCur, 0);
    }
#endif

    यदि( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_साँचो);
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
    यदि( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_साँचो);
    }
  }

  /* Increment the row counter
  */
  यदि( reg_Row_गन्ती ){
    sqlite3VdbeAddOp2(v, OP_AddImm, reg_Row_गन्ती, 1);
  }

  यदि( pTrigger ){
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  यदि( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }अन्यथा यदि( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3WhereEnd(pWInfo);
  }अन्यथा{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3VdbeAddOp2(v, OP_Next, iEph, addr_शीर्ष); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  यदि( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  यदि( reg_Row_गन्ती ){
    sqlite3_Code_Change_गन्ती(v, reg_Row_गन्ती, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_झिक(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_अभिव्यक्ति_List_Delete(db, pChanges);
  sqlite3_अभिव्यक्ति_Delete(db, pWhere);
#यदि defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT)
  sqlite3_अभिव्यक्ति_List_Delete(db, pOrderBy);
  sqlite3_अभिव्यक्ति_Delete(db, pLimit);
#endif
  फर्काऊ;
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
स्थिर शून्य updateVirtualTable(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  Table *pTab,         /* The virtual table */
  अभिव्यक्ति_List *pChanges,  /* The columns to change in the UPDATE statement */
  अभिव्यक्ति *pRowid,        /* Expression used to recompute the rowid */
  पूर्णाङ्क *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  अभिव्यक्ति *pWhere,        /* WHERE clause of the UPDATE statement */
  पूर्णाङ्क onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  पूर्णाङ्क ephemTab;             /* Table holding the result of the SELECT */
  पूर्णाङ्क i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  अचर वर्ण *pVTab = (अचर वर्ण*)sqlite3GetVTable(db, pTab);
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
  addr= sqlite3VdbeAddOp2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  यदि( pSrc->nSrc>1 ){
    Index *pPk = 0;
    अभिव्यक्ति *pRow;
    अभिव्यक्ति_List *pList;
    यदि( HasRowid(pTab) ){
      यदि( pRowid ){
        pRow = sqlite3_अभिव्यक्ति_Dup(db, pRowid, 0);
      }अन्यथा{
        pRow = sqlite3PExpr(pParse, TK_ROW, 0, 0);
      }
    }अन्यथा{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_साँचो_Index(pTab);
      प्रतिज्ञा( pPk!=0 );
      प्रतिज्ञा( pPk->n_साँचो_Col==1 );
      iPk = pPk->aiColumn[0];
      यदि( aXRef[iPk]>=0 ){
        pRow = sqlite3_अभिव्यक्ति_Dup(db, pChanges->a[aXRef[iPk]].p_अभिव्यक्ति, 0);
      }अन्यथा{
        pRow = अभिव्यक्ति_Row_Column(pParse, iPk);
      }
    }
    pList = sqlite3_अभिव्यक्ति_List_Append(pParse, 0, pRow);

    लागि(i=0; i<pTab->nCol; i++){
      यदि( aXRef[i]>=0 ){
        pList = sqlite3_अभिव्यक्ति_List_Append(pParse, pList,
          sqlite3_अभिव्यक्ति_Dup(db, pChanges->a[aXRef[i]].p_अभिव्यक्ति, 0)
        );
      }अन्यथा{
        अभिव्यक्ति *p_Row_अभिव्यक्ति = अभिव्यक्ति_Row_Column(pParse, i);
        यदि( p_Row_अभिव्यक्ति ) p_Row_अभिव्यक्ति->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_अभिव्यक्ति_List_Append(pParse, pList, p_Row_अभिव्यक्ति);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_अभिव्यक्ति_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }अन्यथा{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    यदि( pWInfo==0 ) फर्काऊ;

    /* Populate the argument registers. */
    लागि(i=0; i<pTab->nCol; i++){
      प्रतिज्ञा( (pTab->aCol[i].colFlags & COLFLAG_GENERATED)==0 );
      यदि( aXRef[i]>=0 ){
        sqlite3_अभिव्यक्ति_Code(pParse, pChanges->a[aXRef[i]].p_अभिव्यक्ति, regArg+2+i);
      }अन्यथा{
        sqlite3VdbeAddOp3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    यदि( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCsr, regArg);
      यदि( pRowid ){
        sqlite3_अभिव्यक्ति_Code(pParse, pRowid, regArg+1);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_Rowid, iCsr, regArg+1);
      }
    }अन्यथा{
      Index *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_साँचो_Index(pTab);
      प्रतिज्ञा( pPk!=0 );
      प्रतिज्ञा( pPk->n_साँचो_Col==1 );
      iPk = pPk->aiColumn[0];
      sqlite3VdbeAddOp3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3VdbeAddOp2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    प्रतिज्ञा( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    यदि( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3VdbeChangeToNoop(v, addr);
      sqlite3VdbeAddOp1(v, OP_Close, iCsr);
    }अन्यथा{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regArg, nArg, regRec);
#यदि defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#endif
      sqlite3VdbeAddOp2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3VdbeAddOp3(v, OP_घुसाउ, ephemTab, regRec, regRowid);
    }
  }


  यदि( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    यदि( pSrc->nSrc==1 ){
      sqlite3WhereEnd(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    लागि(i=0; i<nArg; i++){
      sqlite3VdbeAddOp3(v, OP_Column, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3VdbeAddOp4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  यदि( eOnePass==ONEPASS_OFF ){
    sqlite3VdbeAddOp2(v, OP_Next, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    sqlite3VdbeAddOp2(v, OP_Close, ephemTab, 0);
  }अन्यथा{
    sqlite3WhereEnd(pWInfo);
  }
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */
