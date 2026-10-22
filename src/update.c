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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
ಸ್ಥಿರ ಶೂನ್ಯ update_Virtual_ಕೋಷ್ಟಕ(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  ಕೋಷ್ಟಕ *pTab,         /* The virtual table */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,  /* The columns to change in the UPDATE statement */
  ಅಭಿವ್ಯಕ್ತಿ *p_Rowid_ಅಭಿವ್ಯಕ್ತಿ,    /* Expression used to recompute the rowid */
  ಪೂರ್ಣಾಂಕ *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,        /* WHERE clause of the UPDATE statement */
  ಪೂರ್ಣಾಂಕ onError          /* ON CONFLICT strategy */
);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

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
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ ಸ್ತಂಭ_Default_Uncommon_Case(
  Vdbe *v,          /* Byte code under construction */
  ಕೋಷ್ಟಕ *pTab,      /* The table */
  ಸ್ತಂಭ *p_ಲಂಬಸಾಲು      /* Which column of the table */
){
  sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  ಪ್ರತಿಜ್ಞಾ( !IsView(pTab) );
  sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(sqlite3VdbeDb(v),
                       sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Auth(pTab,p_ಲಂಬಸಾಲು,pParse), enc,
                       p_ಲಂಬಸಾಲು->affinity, &p_ಮೌಲ್ಯ);
  ಆದರೆ( p_ಮೌಲ್ಯ ){
    sqlite3VdbeAppendP4(v, p_ಮೌಲ್ಯ, P4_MEM);
  }
}
ಶೂನ್ಯ sqlite3_ಸ್ತಂಭ_Default(
  Vdbe *v,          /* Byte code under construction */
  ಕೋಷ್ಟಕ *pTab,      /* The table */
  ಪೂರ್ಣಾಂಕ i,            /* Which column of the table */
  ಪೂರ್ಣಾಂಕ iReg          /* Register in which results are stored */
){
  ಸ್ತಂಭ *p_ಲಂಬಸಾಲು;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTab->n_ಲಂಬಸಾಲು>i );
  p_ಲಂಬಸಾಲು = &pTab->a_ಲಂಬಸಾಲು[i];
  ಆದರೆ( p_ಲಂಬಸಾಲು->iDflt ){
    ಸ್ತಂಭ_Default_Uncommon_Case(v,pTab,p_ಲಂಬಸಾಲು);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
  ಆದರೆ( p_ಲಂಬಸಾಲು->affinity==SQLITE_AFF_REAL ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_RealAffinity, iReg);
  }
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Is_Being_Updated(
  ಸೂಚ್ಯಂಕ *pIdx,      /* The index to check */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು,         /* Which column of the index to check */
  ಪೂರ್ಣಾಂಕ *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  ಪೂರ್ಣಾಂಕ chngRowid     /* true if the rowid is being updated */
){
  i16 i_Idx_ಲಂಬಸಾಲು = pIdx->ai_ಸ್ತಂಭ[i_ಲಂಬಸಾಲು];
  ಪ್ರತಿಜ್ಞಾ( i_Idx_ಲಂಬಸಾಲು!=XN_ROWID ); /* Cannot index rowid */
  ಆದರೆ( i_Idx_ಲಂಬಸಾಲು>=0 ){
    ಹಿಂತಿರುಗಿ aXRef[i_Idx_ಲಂಬಸಾಲು]>=0;
  }
  ಪ್ರತಿಜ್ಞಾ( i_Idx_ಲಂಬಸಾಲು==XN_ಅಭಿವ್ಯಕ್ತಿ );
  ಪ್ರತಿಜ್ಞಾ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[i_ಲಂಬಸಾಲು].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_References_Updated_ಸ್ತಂಭ(pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[i_ಲಂಬಸಾಲು].p_ಅಭಿವ್ಯಕ್ತಿ,
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ_Where_Clause_Might_Change(
  ಸೂಚ್ಯಂಕ *pIdx,      /* The index to check */
  ಪೂರ್ಣಾಂಕ *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  ಪೂರ್ಣಾಂಕ chngRowid     /* true if the rowid is being updated */
){
  ಆದರೆ( pIdx->pPartIdxWhere==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_References_Updated_ಸ್ತಂಭ(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ *ಅಭಿವ್ಯಕ್ತಿ_ಅಡ್ಡಸಾಲು_ಸ್ತಂಭ(Parse *pParse, ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು){
  ಅಭಿವ್ಯಕ್ತಿ *pRet = sqlite3PExpr(pParse, TK_ಅಡ್ಡಸಾಲು, 0, 0);
  ಆದರೆ( pRet ) pRet->i_ಸ್ತಂಭ = i_ಲಂಬಸಾಲು+1;
  ಹಿಂತಿರುಗಿ pRet;
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
ಸ್ಥಿರ ಶೂನ್ಯ updateFromSelect(
  Parse *pParse,                  /* Parse context */
  ಪೂರ್ಣಾಂಕ iEph,                       /* Cursor for open eph. table */
  ಸೂಚ್ಯಂಕ *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,                   /* WHERE clause for query */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,             /* ORDER BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ                    /* LIMIT clause */
){
  ಪೂರ್ಣಾಂಕ i;
  SelectDest dest;
  Select *pSelect = 0;
  ಅಭಿವ್ಯಕ್ತಿ_List *pList = 0;
  ಅಭಿವ್ಯಕ್ತಿ_List *pGrp = 0;
  ಅಭಿವ್ಯಕ್ತಿ *pLimit2 = 0;
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By2 = 0;
  sqlite3 *db = pParse->db;
  ಕೋಷ್ಟಕ *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  ಅಭಿವ್ಯಕ್ತಿ *pWhere2;
  ಪೂರ್ಣಾಂಕ eDest;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ
  ಆದರೆ( p_ಕ್ರಮ_By && p_ಎಲ್ಲೆ==0 ) {
    sqlite3_Error_ಸಂದೇಶ(pParse, "ORDER BY without LIMIT on UPDATE");
    ಹಿಂತಿರುಗಿ;
  }
  p_ಕ್ರಮ_By2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p_ಕ್ರಮ_By, 0);
  pLimit2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಲ್ಲೆ, 0);
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(p_ಕ್ರಮ_By);
  ಬಳಸದ_PARAMETER(p_ಎಲ್ಲೆ);
#ಆದರೆ_ಅಂತ್ಯ

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhere, 0);

  ಪ್ರತಿಜ್ಞಾ( pTabList->nSrc>1 );
  ಆದರೆ( pSrc ){
    ಪ್ರತಿಜ್ಞಾ( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  ಆದರೆ( pPk ){
    ಗಾಗಿ(i=0; i<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
      ಅಭಿವ್ಯಕ್ತಿ *pNew = ಅಭಿವ್ಯಕ್ತಿ_ಅಡ್ಡಸಾಲು_ಸ್ತಂಭ(pParse, pPk->ai_ಸ್ತಂಭ[i]);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ
      ಆದರೆ( p_ಎಲ್ಲೆ ){
        pGrp = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pGrp, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pNew, 0));
      }
#ಆದರೆ_ಅಂತ್ಯ
      pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_ಕೋಷ್ಟಕ : SRT_Upfrom;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( IsView(pTab) ){
    ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList, ಅಭಿವ್ಯಕ್ತಿ_ಅಡ್ಡಸಾಲು_ಸ್ತಂಭ(pParse, i));
    }
    eDest = SRT_ಕೋಷ್ಟಕ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    eDest = IsVirtual(pTab) ? SRT_ಕೋಷ್ಟಕ : SRT_Upfrom;
    pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ಅಡ್ಡಸಾಲು,0,0));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ
    ಆದರೆ( p_ಎಲ್ಲೆ ){
      pGrp = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_ಅಡ್ಡಸಾಲು,0,0));
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಪ್ರತಿಜ್ಞಾ( pChanges!=0 || pParse->db->mallocFailed );
  ಆದರೆ( pChanges ){
    ಗಾಗಿ(i=0; i<pChanges->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList,
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pChanges->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, p_ಕ್ರಮ_By2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  ಆದರೆ( pSelect ) pSelect->selFlags |= SF_ಕ್ರಮ_By_Reqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು : -1);
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
ಶೂನ್ಯ sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,    /* Things to be changed */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,          /* The WHERE clause.  May be null */
  ಪೂರ್ಣಾಂಕ onError,           /* How to handle constraint errors */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,    /* ORDER BY clause. May be null */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  ಪೂರ್ಣಾಂಕ i, j, k;           /* Loop counters */
  ಕೋಷ್ಟಕ *pTab;           /* The table to be updated */
  ಪೂರ್ಣಾಂಕ addr_ತುದಿ = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  ಸೂಚ್ಯಂಕ *pIdx;           /* For looping over indices */
  ಸೂಚ್ಯಂಕ *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  ಪೂರ್ಣಾಂಕ nIdx;              /* Number of indices that need updating */
  ಪೂರ್ಣಾಂಕ nAllIdx;           /* Total number of indexes */
  ಪೂರ್ಣಾಂಕ i_ತಳ_Cur;          /* Base cursor number */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur;          /* Cursor for the canonical data btree */
  ಪೂರ್ಣಾಂಕ iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  ಪೂರ್ಣಾಂಕ *aRegIdx = 0;      /* Registers for to each index and the main table */
  ಪೂರ್ಣಾಂಕ *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *a_ಗಮ್ಯ_Open;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_ಕೀಲಿ;            /* Either chngPk or chngRowid */
  ಅಭಿವ್ಯಕ್ತಿ *p_Rowid_ಅಭಿವ್ಯಕ್ತಿ = 0;  /* Expression defining the new record number */
  ಪೂರ್ಣಾಂಕ i_Rowid_ಅಭಿವ್ಯಕ್ತಿ = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  ಪೂರ್ಣಾಂಕ iDb;               /* Database containing the table being updated */
  ಪೂರ್ಣಾಂಕ eOnePass;          /* ONEPASS_XXX value from where.c */
  ಪೂರ್ಣಾಂಕ hasFK;             /* True if foreign key processing is required */
  ಪೂರ್ಣಾಂಕ labelBreak;        /* Jump here to break out of UPDATE loop */
  ಪೂರ್ಣಾಂಕ labelContinue;     /* Jump here to continue next step of UPDATE loop */
  ಪೂರ್ಣಾಂಕ flags;             /* Flags for sqlite3WhereBegin() */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  ಪೂರ್ಣಾಂಕ isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  ಪೂರ್ಣಾಂಕ tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  ಪೂರ್ಣಾಂಕ iEph = 0;          /* Ephemeral table holding all primary key values */
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  ಪೂರ್ಣಾಂಕ aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  ಪೂರ್ಣಾಂಕ addrOpen = 0;      /* Address of OP_OpenEphemeral */
  ಪೂರ್ಣಾಂಕ iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  ಪೂರ್ಣಾಂಕ bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  ಪೂರ್ಣಾಂಕ b_ಮುಕ್ತಾಯ_Seek = 1;   /* The OP_FinishSeek opcode is needed */
  ಪೂರ್ಣಾಂಕ nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  ಪೂರ್ಣಾಂಕ reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ = 0;   /* A count of rows changed */
  ಪೂರ್ಣಾಂಕ regOldRowid = 0;   /* The old rowid */
  ಪೂರ್ಣಾಂಕ regNewRowid = 0;   /* The new rowid */
  ಪೂರ್ಣಾಂಕ regNew = 0;        /* Content of the NEW.* table in triggers */
  ಪೂರ್ಣಾಂಕ regOld = 0;        /* Content of OLD.* table in triggers */
  ಪೂರ್ಣಾಂಕ reg_ಅಡ್ಡಸಾಲು_ಗಣ = 0;     /* Rowset of rows to be updated */
  ಪೂರ್ಣಾಂಕ reg_ಕೀಲಿ = 0;        /* composite PRIMARY KEY value */

  memset(&sContext, 0, ಗಾತ್ರ(sContext));
  db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಆದರೆ( pParse->nErr ){
    ಹೋಗು update_cleanup;
  }
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ಆದರೆ( pTab==0 ) ಹೋಗು update_cleanup;
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  ಪ್ರತಿಜ್ಞಾ( pTrigger || tmask==0 );
#ಇಲ್ಲದಿದ್ದರೆ
# define pTrigger 0
# define isView 0
# define tmask 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x10000 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_ವೃಕ್ಷ_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, p_ಕ್ರಮ_By, p_ಎಲ್ಲೆ, pUpsert, pTrigger);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  ಪ್ರತಿಜ್ಞಾ( nChangeFrom==0 || pUpsert==0 );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ
  ಆದರೆ( !isView && nChangeFrom==0 ){
    pWhere = sqlite3_ಎಲ್ಲೆ_Where(
        pParse, pTabList, pWhere, p_ಕ್ರಮ_By, p_ಎಲ್ಲೆ, "UPDATE"
    );
    p_ಕ್ರಮ_By = 0;
    p_ಎಲ್ಲೆ = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, pTab) ){
    ಹೋಗು update_cleanup;
  }
  ಆದರೆ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    ಹೋಗು update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  i_ತಳ_Cur = i_ದತ್ತಾಂಶ_Cur = pParse->nTab++;
  iIdxCur = i_ದತ್ತಾಂಶ_Cur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
  testcase( pPk!=0 && pPk!=pTab->p_ಸೂಚ್ಯಂಕ );
  ಗಾಗಿ(nIdx=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, nIdx++){
    ಆದರೆ( pPk==pIdx ){
      i_ದತ್ತಾಂಶ_Cur = pParse->nTab;
    }
    pParse->nTab++;
  }
  ಆದರೆ( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    i_ದತ್ತಾಂಶ_Cur = pUpsert->i_ದತ್ತಾಂಶ_Cur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = i_ತಳ_Cur;
  }
  pTabList->a[0].iCursor = i_ದತ್ತಾಂಶ_Cur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) * (pTab->n_ಲಂಬಸಾಲು+nIdx+1) + nIdx+2 );
  ಆದರೆ( aXRef==0 ) ಹೋಗು update_cleanup;
  aRegIdx = aXRef+pTab->n_ಲಂಬಸಾಲು;
  a_ಗಮ್ಯ_Open = (u8*)(aRegIdx+nIdx+1);
  memset(a_ಗಮ್ಯ_Open, 1, nIdx+1);
  a_ಗಮ್ಯ_Open[nIdx+1] = 0;
  ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  memset(&sNC, 0, ಗಾತ್ರ(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  ಆದರೆ( v==0 ) ಹೋಗು update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  ಗಾಗಿ(i=0; i<pChanges->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    ಆದರೆ( nChangeFrom==0 && sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, pChanges->a[i].p_ಅಭಿವ್ಯಕ್ತಿ) ){
      ಹೋಗು update_cleanup;
    }
    j = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, pChanges->a[i].zEName);
    ಆದರೆ( j>=0 ){
      ಆದರೆ( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_ಅಭಿವ್ಯಕ್ತಿ = pChanges->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
        i_Rowid_ಅಭಿವ್ಯಕ್ತಿ = i;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pPk && (pTab->a_ಲಂಬಸಾಲು[j].ಲಂಬಸಾಲು_Flags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[j].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED ){
        testcase( pTab->a_ಲಂಬಸಾಲು[j].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL );
        testcase( pTab->a_ಲಂಬಸಾಲು[j].ಲಂಬಸಾಲು_Flags & COLFLAG_STORED );
        sqlite3_Error_ಸಂದೇಶ(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->a_ಲಂಬಸಾಲು[j].zCnName);
        ಹೋಗು update_cleanup;
      }
#ಆದರೆ_ಅಂತ್ಯ
      aXRef[j] = i;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_ಅಭಿವ್ಯಕ್ತಿ = pChanges->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
        i_Rowid_ಅಭಿವ್ಯಕ್ತಿ = i;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Error_ಸಂದೇಶ(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        ಹೋಗು update_cleanup;
      }
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
    {
      ಪೂರ್ಣಾಂಕ rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->a_ಲಂಬಸಾಲು[j].zCnName,
                            db->aDb[iDb].zDbSName);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಆದರೆ( rc==SQLITE_DENY ){
          ಹೋಗು update_cleanup;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          aXRef[j] = -1;
        }
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಪ್ರತಿಜ್ಞಾ( (chngRowid & chngPk)==0 );
  ಪ್ರತಿಜ್ಞಾ( chngRowid==0 || chngRowid==1 );
  ಪ್ರತಿಜ್ಞಾ( chngPk==0 || chngPk==1 );
  chng_ಕೀಲಿ = chngRowid + chngPk;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  ಆದರೆ( pTab->tabFlags & TF_HasGenerated ){
    ಪೂರ್ಣಾಂಕ b_ಪ್ರಗತಿ;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    ಮಾಡು{
      b_ಪ್ರಗತಿ = 0;
      ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
        ಆದರೆ( aXRef[i]>=0 ) ಮುಂದುವರಿ;
        ಆದರೆ( (pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)==0 ) ಮುಂದುವರಿ;
        ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_References_Updated_ಸ್ತಂಭ(
                sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pTab, &pTab->a_ಲಂಬಸಾಲು[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          b_ಪ್ರಗತಿ = 1;
        }
      }
    }ಆಗ( b_ಪ್ರಗತಿ );
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].ಲಂಬಸಾಲು_ಬಳಸಿದ = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_ಕೀಲಿ);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  ಆದರೆ( onError==OE_Replace ) bReplace = 1;
  ಗಾಗಿ(nAllIdx=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, nAllIdx++){
    ಪೂರ್ಣಾಂಕ reg;
    ಆದರೆ( chng_ಕೀಲಿ || hasFK>1 || pIdx==pPk
     || ಸೂಚ್ಯಂಕ_Where_Clause_Might_Change(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->n_ಸ್ತಂಭ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      reg = 0;
      ಗಾಗಿ(i=0; i<pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
        ಆದರೆ( ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Is_Being_Updated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->n_ಸ್ತಂಭ;
          ಆದರೆ( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          ನಿಲ್ಲಿಸು;
        }
      }
    }
    ಆದರೆ( reg==0 ) a_ಗಮ್ಯ_Open[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  ಆದರೆ( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    memset(a_ಗಮ್ಯ_Open, 1, nIdx+1);
  }

  ಆದರೆ( pParse->nested==0 ) sqlite3_Vdbe_ಎಣಿಕೆ_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  ಆದರೆ( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    ಪ್ರತಿಜ್ಞಾ( aRegIdx[nAllIdx]==pParse->nMem );
    reg_ಅಡ್ಡಸಾಲು_ಗಣ = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    ಆದರೆ( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->n_ಲಂಬಸಾಲು;
    }
    ಆದರೆ( chng_ಕೀಲಿ || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->n_ಲಂಬಸಾಲು;
  }

  /* Start the view context. */
  ಆದರೆ( isView ){
    sqlite3_Auth_Context_ತಳ್ಳು(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#ಆದರೆ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  ಆದರೆ( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_ಕ್ರಮ_By, p_ಎಲ್ಲೆ, i_ದತ್ತಾಂಶ_Cur
    );
    p_ಕ್ರಮ_By = 0;
    p_ಎಲ್ಲೆ = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  ಆದರೆ( nChangeFrom==0 && sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, pWhere) ){
    ಹೋಗು update_cleanup;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  ಆದರೆ( IsVirtual(pTab) ){
    update_Virtual_ಕೋಷ್ಟಕ(pParse, pTabList, pTab, pChanges, p_Rowid_ಅಭಿವ್ಯಕ್ತಿ, aXRef,
                       pWhere, onError);
    ಹೋಗು update_cleanup;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  ಆದರೆ( (db->flags&SQLITE_ಎಣಿಕೆ_ಅಡ್ಡಸಾಲುಗಳು)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ = ++pParse->nMem;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ);
  }

  ಆದರೆ( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 0, reg_ಅಡ್ಡಸಾಲು_ಗಣ, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenEphemeral, iEph, 0, reg_ಅಡ್ಡಸಾಲು_ಗಣ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_ಕೀಲಿ = ++pParse->nMem;
    ಆದರೆ( pUpsert==0 ){
      ಪೂರ್ಣಾಂಕ n_Eph_ಲಂಬಸಾಲು = nPk + nChangeFrom + (isView ? pTab->n_ಲಂಬಸಾಲು : 0);
      iEph = pParse->nTab++;
      ಆದರೆ( pPk ) sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, iEph, n_Eph_ಲಂಬಸಾಲು);
      ಆದರೆ( pPk ){
        ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_ದ_ಸೂಚ್ಯಂಕ(pParse, pPk);
        ಆದರೆ( p_ಕೀಲಿ_Info ){
          p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ = n_Eph_ಲಂಬಸಾಲು;
          sqlite3VdbeAppendP4(v, p_ಕೀಲಿ_Info, P4_KEYINFO);
        }
      }
      ಆದರೆ( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, p_ಕ್ರಮ_By, p_ಎಲ್ಲೆ
        );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
        ಆದರೆ( isView ) i_ದತ್ತಾಂಶ_Cur = iEph;
#ಆದರೆ_ಅಂತ್ಯ
      }
    }
  }

  ಆದರೆ( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_ಕೀಲಿ = nPk;
    reg_ಕೀಲಿ = iPk;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
      b_ಮುಕ್ತಾಯ_Seek = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
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
      ಆದರೆ( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_ಕೀಲಿ
       && !bReplace
       && (pWhere==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      ಆದರೆ( pWInfo==0 ) ಹೋಗು update_cleanup;

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
      b_ಮುಕ್ತಾಯ_Seek = sqlite3_Where_ಬಳಸುತ್ತದೆ_Deferred_Seek(pWInfo);
      ಆದರೆ( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        ಆದರೆ( eOnePass==ONEPASS_MULTI ){
          ಪೂರ್ಣಾಂಕ iCur = aiCurOnePass[1];
          ಆದರೆ( iCur>=0 && iCur!=i_ದತ್ತಾಂಶ_Cur && a_ಗಮ್ಯ_Open[iCur-i_ತಳ_Cur] ){
            eOnePass = ONEPASS_OFF;
          }
          ಪ್ರತಿಜ್ಞಾ( iCur!=i_ದತ್ತಾಂಶ_Cur || !HasRowid(pTab) );
        }
      }
    }

    ಆದರೆ( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, i_ದತ್ತಾಂಶ_Cur, regOldRowid);
      ಆದರೆ( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iEph, reg_ಅಡ್ಡಸಾಲು_ಗಣ, regOldRowid);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( ALWAYS(addrOpen) ) sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, addrOpen);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      ಗಾಗಿ(i=0; i<nPk; i++){
        ಪ್ರತಿಜ್ಞಾ( pPk->ai_ಸ್ತಂಭ[i]>=0 );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, i_ದತ್ತಾಂಶ_Cur,
                                        pPk->ai_ಸ್ತಂಭ[i], iPk+i);
      }
      ಆದರೆ( eOnePass ){
        ಆದರೆ( addrOpen ) sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, addrOpen);
        n_ಕೀಲಿ = nPk;
        reg_ಕೀಲಿ = iPk;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Make_ದಾಖಲೆ, iPk, nPk, reg_ಕೀಲಿ,
                          sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Str(db, pPk), nPk);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iEph, reg_ಕೀಲಿ, iPk, nPk);
      }
    }
  }

  ಆದರೆ( pUpsert==0 ){
    ಆದರೆ( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3_Where_ಕೊನೆ(pWInfo);
    }

    ಆದರೆ( !isView ){
      ಪೂರ್ಣಾಂಕ addrOnce = 0;
      ಪೂರ್ಣಾಂಕ iNotUsed1 = 0;
      ಪೂರ್ಣಾಂಕ iNotUsed2 = 0;

      /* Open every index that needs updating. */
      ಆದರೆ( eOnePass!=ONEPASS_OFF ){
        ಆದರೆ( aiCurOnePass[0]>=0 ) a_ಗಮ್ಯ_Open[aiCurOnePass[0]-i_ತಳ_Cur] = 0;
        ಆದರೆ( aiCurOnePass[1]>=0 ) a_ಗಮ್ಯ_Open[aiCurOnePass[1]-i_ತಳ_Cur] = 0;
      }

      ಆದರೆ( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3_Open_ಕೋಷ್ಟಕ_And_Indices(pParse, pTab, OP_OpenWrite, 0, i_ತಳ_Cur,
                                 a_ಗಮ್ಯ_Open, &iNotUsed1, &iNotUsed2);
      ಆದರೆ( addrOnce ){
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ_Or_ತೆಗೆ_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    ಆದರೆ( eOnePass!=ONEPASS_OFF ){
      ಆದರೆ( aiCurOnePass[0]!=i_ದತ್ತಾಂಶ_Cur
       && aiCurOnePass[1]!=i_ದತ್ತಾಂಶ_Cur
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
       && !isView
#ಆದರೆ_ಅಂತ್ಯ
      ){
        ಪ್ರತಿಜ್ಞಾ( pPk );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Not_ಸಿಕ್ಕಿತು, i_ದತ್ತಾಂಶ_Cur, labelBreak, reg_ಕೀಲಿ,n_ಕೀಲಿ);
        VdbeCoverage(v);
      }
      ಆದರೆ( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, pPk ? reg_ಕೀಲಿ : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_ತುದಿ = sqlite3VdbeCurrentAddr(v);
      ಆದರೆ( nChangeFrom ){
        ಆದರೆ( !isView ){
          ಆದರೆ( pPk ){
            ಗಾಗಿ(i=0; i<nPk; i++){
              sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iEph, i, iPk+i);
            }
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(
                v, OP_Not_ಸಿಕ್ಕಿತು, i_ದತ್ತಾಂಶ_Cur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(
                v, OP_NotExists, i_ದತ್ತಾಂಶ_Cur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಅಡ್ಡಸಾಲು_ದತ್ತಾಂಶ, iEph, reg_ಕೀಲಿ);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Not_ಸಿಕ್ಕಿತು, i_ದತ್ತಾಂಶ_Cur, labelContinue, reg_ಕೀಲಿ,0);
        VdbeCoverage(v);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NotExists, i_ದತ್ತಾಂಶ_Cur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  ಪ್ರತಿಜ್ಞಾ( chng_ಕೀಲಿ || pTrigger || hasFK || regOldRowid==regNewRowid );
  ಆದರೆ( chngRowid ){
    ಪ್ರತಿಜ್ಞಾ( i_Rowid_ಅಭಿವ್ಯಕ್ತಿ>=0 );
    ಆದರೆ( nChangeFrom==0 ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_Rowid_ಅಭಿವ್ಯಕ್ತಿ, regNewRowid);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iEph, i_Rowid_ಅಭಿವ್ಯಕ್ತಿ, regNewRowid);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  ಆದರೆ( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      u32 ಲಂಬಸಾಲು_Flags = pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags;
      k = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i) + regOld;
      ಆದರೆ( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (ಲಂಬಸಾಲು_Flags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, i_ದತ್ತಾಂಶ_Cur, i, k);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, k);
      }
    }
    ಆದರೆ( chngRowid==0 && pPk==0 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
      ಆದರೆ( isView ) sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, regOldRowid);
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Copy, regOldRowid, regNewRowid);
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
  ಗಾಗಿ(i=0, k=regNew; i<pTab->n_ಲಂಬಸಾಲು; i++, k++){
    ಆದರೆ( i==pTab->iPKey ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, k);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)!=0 ){
      ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL ) k--;
    }ಇಲ್ಲದಿದ್ದರೆ{
      j = aXRef[i];
      ಆದರೆ( j>=0 ){
        ಆದರೆ( nChangeFrom ){
          ಪೂರ್ಣಾಂಕ nOff = (isView ? pTab->n_ಲಂಬಸಾಲು : nPk);
          ಪ್ರತಿಜ್ಞಾ( eOnePass==ONEPASS_OFF );
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iEph, nOff+j, k);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pChanges->a[j].p_ಅಭಿವ್ಯಕ್ತಿ, k);
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, i_ದತ್ತಾಂಶ_Cur, i, k);
        b_ಮುಕ್ತಾಯ_Seek = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, k);
      }
    }
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
  ಆದರೆ( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  ಆದರೆ( tmask&TRIGGER_BEFORE ){
    sqlite3_ಕೋಷ್ಟಕ_Affinity(v, pTab, regNew);
    sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    ಆದರೆ( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      ಆದರೆ( pPk ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Not_ಸಿಕ್ಕಿತು,i_ದತ್ತಾಂಶ_Cur,labelContinue,reg_ಕೀಲಿ,n_ಕೀಲಿ);
        VdbeCoverage(v);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NotExists, i_ದತ್ತಾಂಶ_Cur, labelContinue,regOldRowid);
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
      ಗಾಗಿ(i=0, k=regNew; i<pTab->n_ಲಂಬಸಾಲು; i++, k++){
        ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED ){
          ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL ) k--;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, i_ದತ್ತಾಂಶ_Cur, i, k);
        }
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
      ಆದರೆ( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
  }

  ಆದರೆ( !isView ){
    /* Do constraint checks. */
    ಪ್ರತಿಜ್ಞಾ( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_ದತ್ತಾಂಶ_Cur, iIdxCur,
        regNewRowid, regOldRowid, chng_ಕೀಲಿ, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    ಆದರೆ( bReplace || chng_ಕೀಲಿ ){
      ಆದರೆ( pPk ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Not_ಸಿಕ್ಕಿತು,i_ದತ್ತಾಂಶ_Cur,labelContinue,reg_ಕೀಲಿ,n_ಕೀಲಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NotExists, i_ದತ್ತಾಂಶ_Cur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    ಆದರೆ( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_ಕೀಲಿ);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3_Generate_ಅಡ್ಡಸಾಲು_ಸೂಚ್ಯಂಕ_Delete(pParse, pTab, i_ದತ್ತಾಂಶ_Cur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    ಆದರೆ( b_ಮುಕ್ತಾಯ_Seek ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ಮುಕ್ತಾಯ_Seek, i_ದತ್ತಾಂಶ_Cur);
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
    ಪ್ರತಿಜ್ಞಾ( regNew==regNewRowid+1 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Delete, i_ದತ್ತಾಂಶ_Cur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_ಕೀಲಿ) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    ಆದರೆ( eOnePass==ONEPASS_MULTI ){
      ಪ್ರತಿಜ್ಞಾ( hasFK==0 && chng_ಕೀಲಿ==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    ಆದರೆ( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_ಕೋಷ್ಟಕ);
    }
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( hasFK>1 || chng_ಕೀಲಿ ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Delete, i_ದತ್ತಾಂಶ_Cur, 0);
    }
#ಆದರೆ_ಅಂತ್ಯ

    ಆದರೆ( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_ಕೀಲಿ);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, i_ದತ್ತಾಂಶ_Cur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    ಆದರೆ( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_ಕೀಲಿ);
    }
  }

  /* Increment the row counter
  */
  ಆದರೆ( reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ, 1);
  }

  ಆದರೆ( pTrigger ){
    sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  ಆದರೆ( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Where_ಕೊನೆ(pWInfo);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iEph, addr_ತುದಿ); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ಆದರೆ( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3_Autoincrement_ಕೊನೆ(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  ಆದರೆ( reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ ){
    sqlite3_ಸಂಕೇತ_Change_ಎಣಿಕೆ(v, reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_ತೆಗೆ(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pChanges);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
#ಆದರೆ defined(SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ)
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಕ್ರಮ_By);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಎಲ್ಲೆ);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ isView
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು isView
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ pTrigger
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು pTrigger
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
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
ಸ್ಥಿರ ಶೂನ್ಯ update_Virtual_ಕೋಷ್ಟಕ(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  ಕೋಷ್ಟಕ *pTab,         /* The virtual table */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,  /* The columns to change in the UPDATE statement */
  ಅಭಿವ್ಯಕ್ತಿ *pRowid,        /* Expression used to recompute the rowid */
  ಪೂರ್ಣಾಂಕ *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,        /* WHERE clause of the UPDATE statement */
  ಪೂರ್ಣಾಂಕ onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  ಪೂರ್ಣಾಂಕ ephemTab;             /* Table holding the result of the SELECT */
  ಪೂರ್ಣಾಂಕ i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pVTab = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  ಪೂರ್ಣಾಂಕ nArg = 2 + pTab->n_ಲಂಬಸಾಲು;      /* Number of arguments to VUpdate */
  ಪೂರ್ಣಾಂಕ regArg;                     /* First register in VUpdate arg array */
  ಪೂರ್ಣಾಂಕ regRec;                     /* Register in which to assemble record */
  ಪೂರ್ಣಾಂಕ regRowid;                   /* Register for ephemeral table rowid */
  ಪೂರ್ಣಾಂಕ iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  ಪೂರ್ಣಾಂಕ aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  ಪೂರ್ಣಾಂಕ eOnePass;                   /* True to use onepass strategy */
  ಪೂರ್ಣಾಂಕ addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  ಪ್ರತಿಜ್ಞಾ( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  ಆದರೆ( pSrc->nSrc>1 ){
    ಸೂಚ್ಯಂಕ *pPk = 0;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಡ್ಡಸಾಲು;
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಆದರೆ( HasRowid(pTab) ){
      ಆದರೆ( pRowid ){
        p_ಅಡ್ಡಸಾಲು = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pRowid, 0);
      }ಇಲ್ಲದಿದ್ದರೆ{
        p_ಅಡ್ಡಸಾಲು = sqlite3PExpr(pParse, TK_ಅಡ್ಡಸಾಲು, 0, 0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      ಪ್ರತಿಜ್ಞಾ( pPk!=0 );
      ಪ್ರತಿಜ್ಞಾ( pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು==1 );
      iPk = pPk->ai_ಸ್ತಂಭ[0];
      ಆದರೆ( aXRef[iPk]>=0 ){
        p_ಅಡ್ಡಸಾಲು = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pChanges->a[aXRef[iPk]].p_ಅಭಿವ್ಯಕ್ತಿ, 0);
      }ಇಲ್ಲದಿದ್ದರೆ{
        p_ಅಡ್ಡಸಾಲು = ಅಭಿವ್ಯಕ್ತಿ_ಅಡ್ಡಸಾಲು_ಸ್ತಂಭ(pParse, iPk);
      }
    }
    pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, p_ಅಡ್ಡಸಾಲು);

    ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      ಆದರೆ( aXRef[i]>=0 ){
        pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList,
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pChanges->a[aXRef[i]].p_ಅಭಿವ್ಯಕ್ತಿ, 0)
        );
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಅಭಿವ್ಯಕ್ತಿ *p_ಅಡ್ಡಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ = ಅಭಿವ್ಯಕ್ತಿ_ಅಡ್ಡಸಾಲು_ಸ್ತಂಭ(pParse, i);
        ಆದರೆ( p_ಅಡ್ಡಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ ) p_ಅಡ್ಡಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList, p_ಅಡ್ಡಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }ಇಲ್ಲದಿದ್ದರೆ{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    ಆದರೆ( pWInfo==0 ) ಹಿಂತಿರುಗಿ;

    /* Populate the argument registers. */
    ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      ಪ್ರತಿಜ್ಞಾ( (pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)==0 );
      ಆದರೆ( aXRef[i]>=0 ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pChanges->a[aXRef[i]].p_ಅಭಿವ್ಯಕ್ತಿ, regArg+2+i);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    ಆದರೆ( HasRowid(pTab) ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iCsr, regArg);
      ಆದರೆ( pRowid ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pRowid, regArg+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iCsr, regArg+1);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸೂಚ್ಯಂಕ *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      ಪ್ರತಿಜ್ಞಾ( pPk!=0 );
      ಪ್ರತಿಜ್ಞಾ( pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು==1 );
      iPk = pPk->ai_ಸ್ತಂಭ[0];
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    ಪ್ರತಿಜ್ಞಾ( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    ಆದರೆ( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, addr);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Close, iCsr);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, regArg, nArg, regRec);
#ಆದರೆ defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, ephemTab, regRec, regRowid);
    }
  }


  ಆದರೆ( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    ಆದರೆ( pSrc->nSrc==1 ){
      sqlite3_Where_ಕೊನೆ(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    ಗಾಗಿ(i=0; i<nArg; i++){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  ಆದರೆ( eOnePass==ONEPASS_OFF ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Close, ephemTab, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Where_ಕೊನೆ(pWInfo);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */
