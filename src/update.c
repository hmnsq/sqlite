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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
ਸਥਿਰ ਖਾਲੀ update_Virtual_ਸਾਰਣੀ(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  ਸਾਰਣੀ *pTab,         /* The virtual table */
  ਵਿਅੰਜਕ_List *pChanges,  /* The columns to change in the UPDATE statement */
  ਵਿਅੰਜਕ *p_Rowid_ਵਿਅੰਜਕ,    /* Expression used to recompute the rowid */
  ਪੂਰਨ *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  ਵਿਅੰਜਕ *pWhere,        /* WHERE clause of the UPDATE statement */
  ਪੂਰਨ onError          /* ON CONFLICT strategy */
);
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ ਕਾਲਮ_Default_Uncommon_Case(
  Vdbe *v,          /* Byte code under construction */
  ਸਾਰਣੀ *pTab,      /* The table */
  ਕਾਲਮ *p_ਖਾਨਾ      /* Which column of the table */
){
  sqlite3_ਮੁੱਲ *p_ਮੁੱਲ = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsView(pTab) );
  sqlite3_ਮੁੱਲ_From_ਵਿਅੰਜਕ(sqlite3VdbeDb(v),
                       sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ_Auth(pTab,p_ਖਾਨਾ,pParse), enc,
                       p_ਖਾਨਾ->affinity, &p_ਮੁੱਲ);
  ਜੇ( p_ਮੁੱਲ ){
    sqlite3VdbeAppendP4(v, p_ਮੁੱਲ, P4_MEM);
  }
}
ਖਾਲੀ sqlite3_ਕਾਲਮ_Default(
  Vdbe *v,          /* Byte code under construction */
  ਸਾਰਣੀ *pTab,      /* The table */
  ਪੂਰਨ i,            /* Which column of the table */
  ਪੂਰਨ iReg          /* Register in which results are stored */
){
  ਕਾਲਮ *p_ਖਾਨਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab->n_ਖਾਨਾ>i );
  p_ਖਾਨਾ = &pTab->a_ਖਾਨਾ[i];
  ਜੇ( p_ਖਾਨਾ->iDflt ){
    ਕਾਲਮ_Default_Uncommon_Case(v,pTab,p_ਖਾਨਾ);
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
  ਜੇ( p_ਖਾਨਾ->affinity==SQLITE_AFF_REAL ){
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_RealAffinity, iReg);
  }
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਪੂਰਨ ਸੂਚਕਾਂਕ_ਕਾਲਮ_Is_Being_Updated(
  ਸੂਚਕਾਂਕ *pIdx,      /* The index to check */
  ਪੂਰਨ i_ਖਾਨਾ,         /* Which column of the index to check */
  ਪੂਰਨ *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  ਪੂਰਨ chngRowid     /* true if the rowid is being updated */
){
  i16 i_Idx_ਖਾਨਾ = pIdx->ai_ਕਾਲਮ[i_ਖਾਨਾ];
  ਪ੍ਰਤਿਜ੍ਞਾ( i_Idx_ਖਾਨਾ!=XN_ROWID ); /* Cannot index rowid */
  ਜੇ( i_Idx_ਖਾਨਾ>=0 ){
    ਵਾਪਸ aXRef[i_Idx_ਖਾਨਾ]>=0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( i_Idx_ਖਾਨਾ==XN_ਵਿਅੰਜਕ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i_ਖਾਨਾ].p_ਵਿਅੰਜਕ!=0 );
  ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_References_Updated_ਕਾਲਮ(pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i_ਖਾਨਾ].p_ਵਿਅੰਜਕ,
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
ਸਥਿਰ ਪੂਰਨ ਸੂਚਕਾਂਕ_Where_Clause_Might_Change(
  ਸੂਚਕਾਂਕ *pIdx,      /* The index to check */
  ਪੂਰਨ *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  ਪੂਰਨ chngRowid     /* true if the rowid is being updated */
){
  ਜੇ( pIdx->pPartIdxWhere==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_References_Updated_ਕਾਲਮ(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
ਸਥਿਰ ਵਿਅੰਜਕ *ਵਿਅੰਜਕ_ਪੰਕਤੀ_ਕਾਲਮ(Parse *pParse, ਪੂਰਨ i_ਖਾਨਾ){
  ਵਿਅੰਜਕ *pRet = sqlite3PExpr(pParse, TK_ਪੰਕਤੀ, 0, 0);
  ਜੇ( pRet ) pRet->i_ਕਾਲਮ = i_ਖਾਨਾ+1;
  ਵਾਪਸ pRet;
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
ਸਥਿਰ ਖਾਲੀ updateFromSelect(
  Parse *pParse,                  /* Parse context */
  ਪੂਰਨ iEph,                       /* Cursor for open eph. table */
  ਸੂਚਕਾਂਕ *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  ਵਿਅੰਜਕ_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  ਵਿਅੰਜਕ *pWhere,                   /* WHERE clause for query */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,             /* ORDER BY clause */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ                    /* LIMIT clause */
){
  ਪੂਰਨ i;
  SelectDest dest;
  Select *pSelect = 0;
  ਵਿਅੰਜਕ_List *pList = 0;
  ਵਿਅੰਜਕ_List *pGrp = 0;
  ਵਿਅੰਜਕ *pLimit2 = 0;
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By2 = 0;
  sqlite3 *db = pParse->db;
  ਸਾਰਣੀ *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  ਵਿਅੰਜਕ *pWhere2;
  ਪੂਰਨ eDest;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ
  ਜੇ( p_ਕ੍ਰਮ_By && p_ਹੱਦਬੰਦੀ==0 ) {
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "ORDER BY without LIMIT on UPDATE");
    ਵਾਪਸ;
  }
  p_ਕ੍ਰਮ_By2 = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p_ਕ੍ਰਮ_By, 0);
  pLimit2 = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਹੱਦਬੰਦੀ, 0);
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(p_ਕ੍ਰਮ_By);
  ਅਣਵਰਤਿਆ_PARAMETER(p_ਹੱਦਬੰਦੀ);
#ਜੇ_ਅੰਤ

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_ਵਿਅੰਜਕ_Dup(db, pWhere, 0);

  ਪ੍ਰਤਿਜ੍ਞਾ( pTabList->nSrc>1 );
  ਜੇ( pSrc ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  ਜੇ( pPk ){
    ਲਈ(i=0; i<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
      ਵਿਅੰਜਕ *pNew = ਵਿਅੰਜਕ_ਪੰਕਤੀ_ਕਾਲਮ(pParse, pPk->ai_ਕਾਲਮ[i]);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ
      ਜੇ( p_ਹੱਦਬੰਦੀ ){
        pGrp = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pGrp, sqlite3_ਵਿਅੰਜਕ_Dup(db, pNew, 0));
      }
#ਜੇ_ਅੰਤ
      pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_ਸਾਰਣੀ : SRT_Upfrom;
  }ਨਹੀਂਤਾਂ ਜੇ( IsView(pTab) ){
    ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
      pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pList, ਵਿਅੰਜਕ_ਪੰਕਤੀ_ਕਾਲਮ(pParse, i));
    }
    eDest = SRT_ਸਾਰਣੀ;
  }ਨਹੀਂਤਾਂ{
    eDest = IsVirtual(pTab) ? SRT_ਸਾਰਣੀ : SRT_Upfrom;
    pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ਪੰਕਤੀ,0,0));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ
    ਜੇ( p_ਹੱਦਬੰਦੀ ){
      pGrp = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ਪੰਕਤੀ,0,0));
    }
#ਜੇ_ਅੰਤ
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pChanges!=0 || pParse->db->mallocFailed );
  ਜੇ( pChanges ){
    ਲਈ(i=0; i<pChanges->n_ਵਿਅੰਜਕ; i++){
      pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pList,
          sqlite3_ਵਿਅੰਜਕ_Dup(db, pChanges->a[i].p_ਵਿਅੰਜਕ, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, p_ਕ੍ਰਮ_By2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  ਜੇ( pSelect ) pSelect->selFlags |= SF_ਕ੍ਰਮ_By_Reqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_ਕੁੰਜੀ_ਖਾਨਾ : -1);
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
ਖਾਲੀ sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  ਵਿਅੰਜਕ_List *pChanges,    /* Things to be changed */
  ਵਿਅੰਜਕ *pWhere,          /* The WHERE clause.  May be null */
  ਪੂਰਨ onError,           /* How to handle constraint errors */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,    /* ORDER BY clause. May be null */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  ਪੂਰਨ i, j, k;           /* Loop counters */
  ਸਾਰਣੀ *pTab;           /* The table to be updated */
  ਪੂਰਨ addr_ਸਿਖਰ = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  ਸੂਚਕਾਂਕ *pIdx;           /* For looping over indices */
  ਸੂਚਕਾਂਕ *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  ਪੂਰਨ nIdx;              /* Number of indices that need updating */
  ਪੂਰਨ nAllIdx;           /* Total number of indexes */
  ਪੂਰਨ i_ਅਧਾਰ_Cur;          /* Base cursor number */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur;          /* Cursor for the canonical data btree */
  ਪੂਰਨ iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  ਪੂਰਨ *aRegIdx = 0;      /* Registers for to each index and the main table */
  ਪੂਰਨ *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *a_ਮੰਜ਼ਿਲ_Open;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_ਕੁੰਜੀ;            /* Either chngPk or chngRowid */
  ਵਿਅੰਜਕ *p_Rowid_ਵਿਅੰਜਕ = 0;  /* Expression defining the new record number */
  ਪੂਰਨ i_Rowid_ਵਿਅੰਜਕ = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  ਪੂਰਨ iDb;               /* Database containing the table being updated */
  ਪੂਰਨ eOnePass;          /* ONEPASS_XXX value from where.c */
  ਪੂਰਨ hasFK;             /* True if foreign key processing is required */
  ਪੂਰਨ labelBreak;        /* Jump here to break out of UPDATE loop */
  ਪੂਰਨ labelContinue;     /* Jump here to continue next step of UPDATE loop */
  ਪੂਰਨ flags;             /* Flags for sqlite3WhereBegin() */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  ਪੂਰਨ isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  ਪੂਰਨ tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  ਪੂਰਨ iEph = 0;          /* Ephemeral table holding all primary key values */
  ਪੂਰਨ n_ਕੁੰਜੀ = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  ਪੂਰਨ aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  ਪੂਰਨ addrOpen = 0;      /* Address of OP_OpenEphemeral */
  ਪੂਰਨ iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  ਪੂਰਨ bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  ਪੂਰਨ b_ਅੰਤ_Seek = 1;   /* The OP_FinishSeek opcode is needed */
  ਪੂਰਨ nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  ਪੂਰਨ reg_ਪੰਕਤੀ_ਗਿਣਤੀ = 0;   /* A count of rows changed */
  ਪੂਰਨ regOldRowid = 0;   /* The old rowid */
  ਪੂਰਨ regNewRowid = 0;   /* The new rowid */
  ਪੂਰਨ regNew = 0;        /* Content of the NEW.* table in triggers */
  ਪੂਰਨ regOld = 0;        /* Content of OLD.* table in triggers */
  ਪੂਰਨ reg_ਪੰਕਤੀ_ਸੈੱਟ = 0;     /* Rowset of rows to be updated */
  ਪੂਰਨ reg_ਕੁੰਜੀ = 0;        /* composite PRIMARY KEY value */

  memset(&sContext, 0, ਆਕਾਰ(sContext));
  db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse==pParse );
  ਜੇ( pParse->nErr ){
    ਜਾਓ update_cleanup;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ਜੇ( pTab==0 ) ਜਾਓ update_cleanup;
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  ਪ੍ਰਤਿਜ੍ਞਾ( pTrigger || tmask==0 );
#ਨਹੀਂਤਾਂ
# define pTrigger 0
# define isView 0
# define tmask 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#ਜੇ_ਅੰਤ

#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x10000 ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_ਰੁੱਖ_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, p_ਕ੍ਰਮ_By, p_ਹੱਦਬੰਦੀ, pUpsert, pTrigger);
  }
#ਜੇ_ਅੰਤ

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_ਵਿਅੰਜਕ : 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( nChangeFrom==0 || pUpsert==0 );

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ
  ਜੇ( !isView && nChangeFrom==0 ){
    pWhere = sqlite3_ਹੱਦਬੰਦੀ_Where(
        pParse, pTabList, pWhere, p_ਕ੍ਰਮ_By, p_ਹੱਦਬੰਦੀ, "UPDATE"
    );
    p_ਕ੍ਰਮ_By = 0;
    p_ਹੱਦਬੰਦੀ = 0;
  }
#ਜੇ_ਅੰਤ

  ਜੇ( sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(pParse, pTab) ){
    ਜਾਓ update_cleanup;
  }
  ਜੇ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    ਜਾਓ update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  i_ਅਧਾਰ_Cur = i_ਅੰਕੜੇ_Cur = pParse->nTab++;
  iIdxCur = i_ਅੰਕੜੇ_Cur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
  testcase( pPk!=0 && pPk!=pTab->p_ਸੂਚਕਾਂਕ );
  ਲਈ(nIdx=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, nIdx++){
    ਜੇ( pPk==pIdx ){
      i_ਅੰਕੜੇ_Cur = pParse->nTab;
    }
    pParse->nTab++;
  }
  ਜੇ( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    i_ਅੰਕੜੇ_Cur = pUpsert->i_ਅੰਕੜੇ_Cur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = i_ਅਧਾਰ_Cur;
  }
  pTabList->a[0].iCursor = i_ਅੰਕੜੇ_Cur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, ਆਕਾਰ(ਪੂਰਨ) * (pTab->n_ਖਾਨਾ+nIdx+1) + nIdx+2 );
  ਜੇ( aXRef==0 ) ਜਾਓ update_cleanup;
  aRegIdx = aXRef+pTab->n_ਖਾਨਾ;
  a_ਮੰਜ਼ਿਲ_Open = (u8*)(aRegIdx+nIdx+1);
  memset(a_ਮੰਜ਼ਿਲ_Open, 1, nIdx+1);
  a_ਮੰਜ਼ਿਲ_Open[nIdx+1] = 0;
  ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  memset(&sNC, 0, ਆਕਾਰ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  ਜੇ( v==0 ) ਜਾਓ update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  ਲਈ(i=0; i<pChanges->n_ਵਿਅੰਜਕ; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    ਜੇ( nChangeFrom==0 && sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, pChanges->a[i].p_ਵਿਅੰਜਕ) ){
      ਜਾਓ update_cleanup;
    }
    j = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, pChanges->a[i].zEName);
    ਜੇ( j>=0 ){
      ਜੇ( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_ਵਿਅੰਜਕ = pChanges->a[i].p_ਵਿਅੰਜਕ;
        i_Rowid_ਵਿਅੰਜਕ = i;
      }ਨਹੀਂਤਾਂ ਜੇ( pPk && (pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
      ਨਹੀਂਤਾਂ ਜੇ( pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & COLFLAG_GENERATED ){
        testcase( pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL );
        testcase( pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & COLFLAG_STORED );
        sqlite3_Error_ਸੁਨੇਹਾ(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->a_ਖਾਨਾ[j].zCnName);
        ਜਾਓ update_cleanup;
      }
#ਜੇ_ਅੰਤ
      aXRef[j] = i;
    }ਨਹੀਂਤਾਂ{
      ਜੇ( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_ਵਿਅੰਜਕ = pChanges->a[i].p_ਵਿਅੰਜਕ;
        i_Rowid_ਵਿਅੰਜਕ = i;
      }ਨਹੀਂਤਾਂ{
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        ਜਾਓ update_cleanup;
      }
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
    {
      ਪੂਰਨ rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->a_ਖਾਨਾ[j].zCnName,
                            db->aDb[iDb].zDbSName);
      ਜੇ( rc!=SQLITE_OK ){
        ਜੇ( rc==SQLITE_DENY ){
          ਜਾਓ update_cleanup;
        }ਨਹੀਂਤਾਂ ਜੇ( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }ਨਹੀਂਤਾਂ{
          aXRef[j] = -1;
        }
      }
    }
#ਜੇ_ਅੰਤ
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (chngRowid & chngPk)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( chngRowid==0 || chngRowid==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( chngPk==0 || chngPk==1 );
  chng_ਕੁੰਜੀ = chngRowid + chngPk;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  ਜੇ( pTab->tabFlags & TF_HasGenerated ){
    ਪੂਰਨ b_ਤਰੱਕੀ;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    ਕਰੋ{
      b_ਤਰੱਕੀ = 0;
      ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
        ਜੇ( aXRef[i]>=0 ) ਜਾਰੀ;
        ਜੇ( (pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_GENERATED)==0 ) ਜਾਰੀ;
        ਜੇ( sqlite3_ਵਿਅੰਜਕ_References_Updated_ਕਾਲਮ(
                sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pTab, &pTab->a_ਖਾਨਾ[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          b_ਤਰੱਕੀ = 1;
        }
      }
    }ਜਦੋਂ( b_ਤਰੱਕੀ );
  }
#ਜੇ_ਅੰਤ

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].ਖਾਨਾ_ਵਰਤਿਆ = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_ਕੁੰਜੀ);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  ਜੇ( onError==OE_Replace ) bReplace = 1;
  ਲਈ(nAllIdx=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, nAllIdx++){
    ਪੂਰਨ reg;
    ਜੇ( chng_ਕੁੰਜੀ || hasFK>1 || pIdx==pPk
     || ਸੂਚਕਾਂਕ_Where_Clause_Might_Change(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->n_ਕਾਲਮ;
    }ਨਹੀਂਤਾਂ{
      reg = 0;
      ਲਈ(i=0; i<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
        ਜੇ( ਸੂਚਕਾਂਕ_ਕਾਲਮ_Is_Being_Updated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->n_ਕਾਲਮ;
          ਜੇ( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          ਰੋਕੋ;
        }
      }
    }
    ਜੇ( reg==0 ) a_ਮੰਜ਼ਿਲ_Open[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  ਜੇ( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    memset(a_ਮੰਜ਼ਿਲ_Open, 1, nIdx+1);
  }

  ਜੇ( pParse->nested==0 ) sqlite3_Vdbe_ਗਿਣਤੀ_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  ਜੇ( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    ਪ੍ਰਤਿਜ੍ਞਾ( aRegIdx[nAllIdx]==pParse->nMem );
    reg_ਪੰਕਤੀ_ਸੈੱਟ = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    ਜੇ( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->n_ਖਾਨਾ;
    }
    ਜੇ( chng_ਕੁੰਜੀ || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->n_ਖਾਨਾ;
  }

  /* Start the view context. */
  ਜੇ( isView ){
    sqlite3_Auth_Context_ਧੱਕੋ(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#ਜੇ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  ਜੇ( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_ਕ੍ਰਮ_By, p_ਹੱਦਬੰਦੀ, i_ਅੰਕੜੇ_Cur
    );
    p_ਕ੍ਰਮ_By = 0;
    p_ਹੱਦਬੰਦੀ = 0;
  }
#ਜੇ_ਅੰਤ

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  ਜੇ( nChangeFrom==0 && sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, pWhere) ){
    ਜਾਓ update_cleanup;
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  ਜੇ( IsVirtual(pTab) ){
    update_Virtual_ਸਾਰਣੀ(pParse, pTabList, pTab, pChanges, p_Rowid_ਵਿਅੰਜਕ, aXRef,
                       pWhere, onError);
    ਜਾਓ update_cleanup;
  }
#ਜੇ_ਅੰਤ

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  ਜੇ( (db->flags&SQLITE_ਗਿਣਤੀ_ਪੰਕਤੀਆਂ)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_ਪੰਕਤੀ_ਗਿਣਤੀ = ++pParse->nMem;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, reg_ਪੰਕਤੀ_ਗਿਣਤੀ);
  }

  ਜੇ( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, reg_ਪੰਕਤੀ_ਸੈੱਟ, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenEphemeral, iEph, 0, reg_ਪੰਕਤੀ_ਸੈੱਟ);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_ਕੁੰਜੀ_ਖਾਨਾ : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_ਕੁੰਜੀ = ++pParse->nMem;
    ਜੇ( pUpsert==0 ){
      ਪੂਰਨ n_Eph_ਖਾਨਾ = nPk + nChangeFrom + (isView ? pTab->n_ਖਾਨਾ : 0);
      iEph = pParse->nTab++;
      ਜੇ( pPk ) sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, iEph, n_Eph_ਖਾਨਾ);
      ਜੇ( pPk ){
        ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_ਦਾ_ਸੂਚਕਾਂਕ(pParse, pPk);
        ਜੇ( p_ਕੁੰਜੀ_Info ){
          p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ = n_Eph_ਖਾਨਾ;
          sqlite3VdbeAppendP4(v, p_ਕੁੰਜੀ_Info, P4_KEYINFO);
        }
      }
      ਜੇ( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, p_ਕ੍ਰਮ_By, p_ਹੱਦਬੰਦੀ
        );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
        ਜੇ( isView ) i_ਅੰਕੜੇ_Cur = iEph;
#ਜੇ_ਅੰਤ
      }
    }
  }

  ਜੇ( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_ਕੁੰਜੀ = nPk;
    reg_ਕੁੰਜੀ = iPk;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
      b_ਅੰਤ_Seek = 0;
    }ਨਹੀਂਤਾਂ{
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
      ਜੇ( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_ਕੁੰਜੀ
       && !bReplace
       && (pWhere==0 || !ਵਿਅੰਜਕ_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      ਜੇ( pWInfo==0 ) ਜਾਓ update_cleanup;

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
      b_ਅੰਤ_Seek = sqlite3_Where_ਵਰਤਦਾ_Deferred_Seek(pWInfo);
      ਜੇ( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        ਜੇ( eOnePass==ONEPASS_MULTI ){
          ਪੂਰਨ iCur = aiCurOnePass[1];
          ਜੇ( iCur>=0 && iCur!=i_ਅੰਕੜੇ_Cur && a_ਮੰਜ਼ਿਲ_Open[iCur-i_ਅਧਾਰ_Cur] ){
            eOnePass = ONEPASS_OFF;
          }
          ਪ੍ਰਤਿਜ੍ਞਾ( iCur!=i_ਅੰਕੜੇ_Cur || !HasRowid(pTab) );
        }
      }
    }

    ਜੇ( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, i_ਅੰਕੜੇ_Cur, regOldRowid);
      ਜੇ( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iEph, reg_ਪੰਕਤੀ_ਸੈੱਟ, regOldRowid);
      }ਨਹੀਂਤਾਂ{
        ਜੇ( ALWAYS(addrOpen) ) sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, addrOpen);
      }
    }ਨਹੀਂਤਾਂ{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      ਲਈ(i=0; i<nPk; i++){
        ਪ੍ਰਤਿਜ੍ਞਾ( pPk->ai_ਕਾਲਮ[i]>=0 );
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, i_ਅੰਕੜੇ_Cur,
                                        pPk->ai_ਕਾਲਮ[i], iPk+i);
      }
      ਜੇ( eOnePass ){
        ਜੇ( addrOpen ) sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, addrOpen);
        n_ਕੁੰਜੀ = nPk;
        reg_ਕੁੰਜੀ = iPk;
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Make_ਰਿਕਾਰਡ, iPk, nPk, reg_ਕੁੰਜੀ,
                          sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(db, pPk), nPk);
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iEph, reg_ਕੁੰਜੀ, iPk, nPk);
      }
    }
  }

  ਜੇ( pUpsert==0 ){
    ਜੇ( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3_Where_ਸਿਰਾ(pWInfo);
    }

    ਜੇ( !isView ){
      ਪੂਰਨ addrOnce = 0;
      ਪੂਰਨ iNotUsed1 = 0;
      ਪੂਰਨ iNotUsed2 = 0;

      /* Open every index that needs updating. */
      ਜੇ( eOnePass!=ONEPASS_OFF ){
        ਜੇ( aiCurOnePass[0]>=0 ) a_ਮੰਜ਼ਿਲ_Open[aiCurOnePass[0]-i_ਅਧਾਰ_Cur] = 0;
        ਜੇ( aiCurOnePass[1]>=0 ) a_ਮੰਜ਼ਿਲ_Open[aiCurOnePass[1]-i_ਅਧਾਰ_Cur] = 0;
      }

      ਜੇ( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3_Open_ਸਾਰਣੀ_And_Indices(pParse, pTab, OP_OpenWrite, 0, i_ਅਧਾਰ_Cur,
                                 a_ਮੰਜ਼ਿਲ_Open, &iNotUsed1, &iNotUsed2);
      ਜੇ( addrOnce ){
        sqlite3_Vdbe_Jump_ਇੱਥੇ_Or_ਕੱਢੋ_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    ਜੇ( eOnePass!=ONEPASS_OFF ){
      ਜੇ( aiCurOnePass[0]!=i_ਅੰਕੜੇ_Cur
       && aiCurOnePass[1]!=i_ਅੰਕੜੇ_Cur
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
       && !isView
#ਜੇ_ਅੰਤ
      ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pPk );
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Not_ਮਿਲਿਆ, i_ਅੰਕੜੇ_Cur, labelBreak, reg_ਕੁੰਜੀ,n_ਕੁੰਜੀ);
        VdbeCoverage(v);
      }
      ਜੇ( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, pPk ? reg_ਕੁੰਜੀ : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }ਨਹੀਂਤਾਂ ਜੇ( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_ਸਿਖਰ = sqlite3VdbeCurrentAddr(v);
      ਜੇ( nChangeFrom ){
        ਜੇ( !isView ){
          ਜੇ( pPk ){
            ਲਈ(i=0; i<nPk; i++){
              sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iEph, i, iPk+i);
            }
            sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(
                v, OP_Not_ਮਿਲਿਆ, i_ਅੰਕੜੇ_Cur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }ਨਹੀਂਤਾਂ{
            sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3_Vdbe_ਜੋੜੋ_Op3(
                v, OP_NotExists, i_ਅੰਕੜੇ_Cur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਪੰਕਤੀ_ਅੰਕੜੇ, iEph, reg_ਕੁੰਜੀ);
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Not_ਮਿਲਿਆ, i_ਅੰਕੜੇ_Cur, labelContinue, reg_ਕੁੰਜੀ,0);
        VdbeCoverage(v);
      }
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NotExists, i_ਅੰਕੜੇ_Cur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( chng_ਕੁੰਜੀ || pTrigger || hasFK || regOldRowid==regNewRowid );
  ਜੇ( chngRowid ){
    ਪ੍ਰਤਿਜ੍ਞਾ( i_Rowid_ਵਿਅੰਜਕ>=0 );
    ਜੇ( nChangeFrom==0 ){
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_Rowid_ਵਿਅੰਜਕ, regNewRowid);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iEph, i_Rowid_ਵਿਅੰਜਕ, regNewRowid);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  ਜੇ( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
      u32 ਖਾਨਾ_Flags = pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags;
      k = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i) + regOld;
      ਜੇ( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (ਖਾਨਾ_Flags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, i_ਅੰਕੜੇ_Cur, i, k);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, k);
      }
    }
    ਜੇ( chngRowid==0 && pPk==0 ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
      ਜੇ( isView ) sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, regOldRowid);
#ਜੇ_ਅੰਤ
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Copy, regOldRowid, regNewRowid);
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
  ਲਈ(i=0, k=regNew; i<pTab->n_ਖਾਨਾ; i++, k++){
    ਜੇ( i==pTab->iPKey ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, k);
    }ਨਹੀਂਤਾਂ ਜੇ( (pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_GENERATED)!=0 ){
      ਜੇ( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL ) k--;
    }ਨਹੀਂਤਾਂ{
      j = aXRef[i];
      ਜੇ( j>=0 ){
        ਜੇ( nChangeFrom ){
          ਪੂਰਨ nOff = (isView ? pTab->n_ਖਾਨਾ : nPk);
          ਪ੍ਰਤਿਜ੍ਞਾ( eOnePass==ONEPASS_OFF );
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iEph, nOff+j, k);
        }ਨਹੀਂਤਾਂ{
          sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pChanges->a[j].p_ਵਿਅੰਜਕ, k);
        }
      }ਨਹੀਂਤਾਂ ਜੇ( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, i_ਅੰਕੜੇ_Cur, i, k);
        b_ਅੰਤ_Seek = 0;
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, k);
      }
    }
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
  ਜੇ( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#ਜੇ_ਅੰਤ

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  ਜੇ( tmask&TRIGGER_BEFORE ){
    sqlite3_ਸਾਰਣੀ_Affinity(v, pTab, regNew);
    sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    ਜੇ( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      ਜੇ( pPk ){
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Not_ਮਿਲਿਆ,i_ਅੰਕੜੇ_Cur,labelContinue,reg_ਕੁੰਜੀ,n_ਕੁੰਜੀ);
        VdbeCoverage(v);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NotExists, i_ਅੰਕੜੇ_Cur, labelContinue,regOldRowid);
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
      ਲਈ(i=0, k=regNew; i<pTab->n_ਖਾਨਾ; i++, k++){
        ਜੇ( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_GENERATED ){
          ਜੇ( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL ) k--;
        }ਨਹੀਂਤਾਂ ਜੇ( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, i_ਅੰਕੜੇ_Cur, i, k);
        }
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
      ਜੇ( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#ਜੇ_ਅੰਤ
    }
  }

  ਜੇ( !isView ){
    /* Do constraint checks. */
    ਪ੍ਰਤਿਜ੍ਞਾ( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_ਅੰਕੜੇ_Cur, iIdxCur,
        regNewRowid, regOldRowid, chng_ਕੁੰਜੀ, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    ਜੇ( bReplace || chng_ਕੁੰਜੀ ){
      ਜੇ( pPk ){
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Not_ਮਿਲਿਆ,i_ਅੰਕੜੇ_Cur,labelContinue,reg_ਕੁੰਜੀ,n_ਕੁੰਜੀ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NotExists, i_ਅੰਕੜੇ_Cur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    ਜੇ( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_ਕੁੰਜੀ);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3_Generate_ਪੰਕਤੀ_ਸੂਚਕਾਂਕ_Delete(pParse, pTab, i_ਅੰਕੜੇ_Cur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    ਜੇ( b_ਅੰਤ_Seek ){
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ਅੰਤ_Seek, i_ਅੰਕੜੇ_Cur);
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
    ਪ੍ਰਤਿਜ੍ਞਾ( regNew==regNewRowid+1 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Delete, i_ਅੰਕੜੇ_Cur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_ਕੁੰਜੀ) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    ਜੇ( eOnePass==ONEPASS_MULTI ){
      ਪ੍ਰਤਿਜ੍ਞਾ( hasFK==0 && chng_ਕੁੰਜੀ==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    ਜੇ( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_ਸਾਰਣੀ);
    }
#ਨਹੀਂਤਾਂ
    ਜੇ( hasFK>1 || chng_ਕੁੰਜੀ ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Delete, i_ਅੰਕੜੇ_Cur, 0);
    }
#ਜੇ_ਅੰਤ

    ਜੇ( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_ਕੁੰਜੀ);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, i_ਅੰਕੜੇ_Cur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    ਜੇ( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_ਕੁੰਜੀ);
    }
  }

  /* Increment the row counter
  */
  ਜੇ( reg_ਪੰਕਤੀ_ਗਿਣਤੀ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, reg_ਪੰਕਤੀ_ਗਿਣਤੀ, 1);
  }

  ਜੇ( pTrigger ){
    sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  ਜੇ( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }ਨਹੀਂਤਾਂ ਜੇ( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Where_ਸਿਰਾ(pWInfo);
  }ਨਹੀਂਤਾਂ{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iEph, addr_ਸਿਖਰ); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ਜੇ( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3_Autoincrement_ਸਿਰਾ(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  ਜੇ( reg_ਪੰਕਤੀ_ਗਿਣਤੀ ){
    sqlite3_ਕੋਡ_Change_ਗਿਣਤੀ(v, reg_ਪੰਕਤੀ_ਗਿਣਤੀ, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_ਕੱਢੋ(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pChanges);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhere);
#ਜੇ defined(SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ)
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਕ੍ਰਮ_By);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਹੱਦਬੰਦੀ);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ isView
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ isView
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ pTrigger
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ pTrigger
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
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
ਸਥਿਰ ਖਾਲੀ update_Virtual_ਸਾਰਣੀ(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  ਸਾਰਣੀ *pTab,         /* The virtual table */
  ਵਿਅੰਜਕ_List *pChanges,  /* The columns to change in the UPDATE statement */
  ਵਿਅੰਜਕ *pRowid,        /* Expression used to recompute the rowid */
  ਪੂਰਨ *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  ਵਿਅੰਜਕ *pWhere,        /* WHERE clause of the UPDATE statement */
  ਪੂਰਨ onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  ਪੂਰਨ ephemTab;             /* Table holding the result of the SELECT */
  ਪੂਰਨ i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  ਅਚਲ ਅੱਖਰ *pVTab = (ਅਚਲ ਅੱਖਰ*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  ਪੂਰਨ nArg = 2 + pTab->n_ਖਾਨਾ;      /* Number of arguments to VUpdate */
  ਪੂਰਨ regArg;                     /* First register in VUpdate arg array */
  ਪੂਰਨ regRec;                     /* Register in which to assemble record */
  ਪੂਰਨ regRowid;                   /* Register for ephemeral table rowid */
  ਪੂਰਨ iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  ਪੂਰਨ aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  ਪੂਰਨ eOnePass;                   /* True to use onepass strategy */
  ਪੂਰਨ addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  ਪ੍ਰਤਿਜ੍ਞਾ( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  ਜੇ( pSrc->nSrc>1 ){
    ਸੂਚਕਾਂਕ *pPk = 0;
    ਵਿਅੰਜਕ *p_ਪੰਕਤੀ;
    ਵਿਅੰਜਕ_List *pList;
    ਜੇ( HasRowid(pTab) ){
      ਜੇ( pRowid ){
        p_ਪੰਕਤੀ = sqlite3_ਵਿਅੰਜਕ_Dup(db, pRowid, 0);
      }ਨਹੀਂਤਾਂ{
        p_ਪੰਕਤੀ = sqlite3PExpr(pParse, TK_ਪੰਕਤੀ, 0, 0);
      }
    }ਨਹੀਂਤਾਂ{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      ਪ੍ਰਤਿਜ੍ਞਾ( pPk!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pPk->n_ਕੁੰਜੀ_ਖਾਨਾ==1 );
      iPk = pPk->ai_ਕਾਲਮ[0];
      ਜੇ( aXRef[iPk]>=0 ){
        p_ਪੰਕਤੀ = sqlite3_ਵਿਅੰਜਕ_Dup(db, pChanges->a[aXRef[iPk]].p_ਵਿਅੰਜਕ, 0);
      }ਨਹੀਂਤਾਂ{
        p_ਪੰਕਤੀ = ਵਿਅੰਜਕ_ਪੰਕਤੀ_ਕਾਲਮ(pParse, iPk);
      }
    }
    pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0, p_ਪੰਕਤੀ);

    ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
      ਜੇ( aXRef[i]>=0 ){
        pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pList,
          sqlite3_ਵਿਅੰਜਕ_Dup(db, pChanges->a[aXRef[i]].p_ਵਿਅੰਜਕ, 0)
        );
      }ਨਹੀਂਤਾਂ{
        ਵਿਅੰਜਕ *p_ਪੰਕਤੀ_ਵਿਅੰਜਕ = ਵਿਅੰਜਕ_ਪੰਕਤੀ_ਕਾਲਮ(pParse, i);
        ਜੇ( p_ਪੰਕਤੀ_ਵਿਅੰਜਕ ) p_ਪੰਕਤੀ_ਵਿਅੰਜਕ->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pList, p_ਪੰਕਤੀ_ਵਿਅੰਜਕ);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }ਨਹੀਂਤਾਂ{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    ਜੇ( pWInfo==0 ) ਵਾਪਸ;

    /* Populate the argument registers. */
    ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
      ਪ੍ਰਤਿਜ੍ਞਾ( (pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_GENERATED)==0 );
      ਜੇ( aXRef[i]>=0 ){
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pChanges->a[aXRef[i]].p_ਵਿਅੰਜਕ, regArg+2+i);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    ਜੇ( HasRowid(pTab) ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iCsr, regArg);
      ਜੇ( pRowid ){
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pRowid, regArg+1);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iCsr, regArg+1);
      }
    }ਨਹੀਂਤਾਂ{
      ਸੂਚਕਾਂਕ *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      ਪ੍ਰਤਿਜ੍ਞਾ( pPk!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pPk->n_ਕੁੰਜੀ_ਖਾਨਾ==1 );
      iPk = pPk->ai_ਕਾਲਮ[0];
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    ਪ੍ਰਤਿਜ੍ਞਾ( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    ਜੇ( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, addr);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Close, iCsr);
    }ਨਹੀਂਤਾਂ{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, regArg, nArg, regRec);
#ਜੇ defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#ਜੇ_ਅੰਤ
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, ephemTab, regRec, regRowid);
    }
  }


  ਜੇ( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    ਜੇ( pSrc->nSrc==1 ){
      sqlite3_Where_ਸਿਰਾ(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    ਲਈ(i=0; i<nArg; i++){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  ਜੇ( eOnePass==ONEPASS_OFF ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Close, ephemTab, 0);
  }ਨਹੀਂਤਾਂ{
    sqlite3_Where_ਸਿਰਾ(pWInfo);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */
