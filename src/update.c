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
#ضمن "sqliteInt.h"

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
ساكن فراغ update_Virtual_جدول(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  جدول *pTab,         /* The virtual table */
  اظہاریہ_List *pChanges,  /* The columns to change in the UPDATE statement */
  اظہاریہ *p_Rowid_اظہاریہ,    /* Expression used to recompute the rowid */
  صحيح *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  اظہاریہ *pWhere,        /* WHERE clause of the UPDATE statement */
  صحيح onError          /* ON CONFLICT strategy */
);
#جے_انت /* SQLITE_OMIT_VIRTUALTABLE */

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
ساكن SQLITE_NOINLINE فراغ کالم_Default_Uncommon_Case(
  Vdbe *v,          /* Byte code under construction */
  جدول *pTab,      /* The table */
  کالم *p_خانہ      /* Which column of the table */
){
  sqlite3_قدر *p_قدر = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  أكد( !IsView(pTab) );
  sqlite3_قدر_From_اظہاریہ(sqlite3VdbeDb(v),
                       sqlite3_کالم_اظہاریہ_Auth(pTab,p_خانہ,pParse), enc,
                       p_خانہ->affinity, &p_قدر);
  إذا( p_قدر ){
    sqlite3VdbeAppendP4(v, p_قدر, P4_MEM);
  }
}
فراغ sqlite3_کالم_Default(
  Vdbe *v,          /* Byte code under construction */
  جدول *pTab,      /* The table */
  صحيح i,            /* Which column of the table */
  صحيح iReg          /* Register in which results are stored */
){
  کالم *p_خانہ;
  أكد( pTab!=0 );
  أكد( pTab->n_خانہ>i );
  p_خانہ = &pTab->a_خانہ[i];
  إذا( p_خانہ->iDflt ){
    کالم_Default_Uncommon_Case(v,pTab,p_خانہ);
  }
#جے_بنا_تعریف SQLITE_OMIT_FLOATING_نقطہ
  إذا( p_خانہ->affinity==SQLITE_AFF_REAL ){
    sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, iReg);
  }
#جے_انت
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
ساكن صحيح اشاریہ_کالم_Is_Being_Updated(
  اشاریہ *pIdx,      /* The index to check */
  صحيح i_خانہ,         /* Which column of the index to check */
  صحيح *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  صحيح chngRowid     /* true if the rowid is being updated */
){
  i16 i_Idx_خانہ = pIdx->ai_کالم[i_خانہ];
  أكد( i_Idx_خانہ!=XN_ROWID ); /* Cannot index rowid */
  إذا( i_Idx_خانہ>=0 ){
    أرجع aXRef[i_Idx_خانہ]>=0;
  }
  أكد( i_Idx_خانہ==XN_اظہاریہ );
  أكد( pIdx->a_خانہ_اظہاریہ!=0 );
  أكد( pIdx->a_خانہ_اظہاریہ->a[i_خانہ].p_اظہاریہ!=0 );
  أرجع sqlite3_اظہاریہ_References_Updated_کالم(pIdx->a_خانہ_اظہاریہ->a[i_خانہ].p_اظہاریہ,
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
ساكن صحيح اشاریہ_Where_Clause_Might_Change(
  اشاریہ *pIdx,      /* The index to check */
  صحيح *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  صحيح chngRowid     /* true if the rowid is being updated */
){
  إذا( pIdx->pPartIdxWhere==0 ) أرجع 0;
  أرجع sqlite3_اظہاریہ_References_Updated_کالم(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
ساكن اظہاریہ *اظہاریہ_صف_کالم(Parse *pParse, صحيح i_خانہ){
  اظہاریہ *pRet = sqlite3PExpr(pParse, TK_صف, 0, 0);
  إذا( pRet ) pRet->i_کالم = i_خانہ+1;
  أرجع pRet;
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
ساكن فراغ updateFromSelect(
  Parse *pParse,                  /* Parse context */
  صحيح iEph,                       /* Cursor for open eph. table */
  اشاریہ *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  اظہاریہ_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  اظہاریہ *pWhere,                   /* WHERE clause for query */
  اظہاریہ_List *p_ترتیب_By,             /* ORDER BY clause */
  اظہاریہ *p_حد_بندی                    /* LIMIT clause */
){
  صحيح i;
  SelectDest dest;
  Select *pSelect = 0;
  اظہاریہ_List *pList = 0;
  اظہاریہ_List *pGrp = 0;
  اظہاریہ *pLimit2 = 0;
  اظہاریہ_List *p_ترتیب_By2 = 0;
  sqlite3 *db = pParse->db;
  جدول *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  اظہاریہ *pWhere2;
  صحيح eDest;

#جے_تعریف_شدہ SQLITE_ENABLE_UPDATE_DELETE_حد_بندی
  إذا( p_ترتیب_By && p_حد_بندی==0 ) {
    sqlite3_Error_پیغام(pParse, "ORDER BY without LIMIT on UPDATE");
    أرجع;
  }
  p_ترتیب_By2 = sqlite3_اظہاریہ_List_Dup(db, p_ترتیب_By, 0);
  pLimit2 = sqlite3_اظہاریہ_Dup(db, p_حد_بندی, 0);
#وإلا
  بن_ورتیا_PARAMETER(p_ترتیب_By);
  بن_ورتیا_PARAMETER(p_حد_بندی);
#جے_انت

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_اظہاریہ_Dup(db, pWhere, 0);

  أكد( pTabList->nSrc>1 );
  إذا( pSrc ){
    أكد( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  إذا( pPk ){
    لكل(i=0; i<pPk->n_کلید_خانہ; i++){
      اظہاریہ *pNew = اظہاریہ_صف_کالم(pParse, pPk->ai_کالم[i]);
#جے_تعریف_شدہ SQLITE_ENABLE_UPDATE_DELETE_حد_بندی
      إذا( p_حد_بندی ){
        pGrp = sqlite3_اظہاریہ_List_Append(pParse, pGrp, sqlite3_اظہاریہ_Dup(db, pNew, 0));
      }
#جے_انت
      pList = sqlite3_اظہاریہ_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_جدول : SRT_Upfrom;
  }وإلا إذا( IsView(pTab) ){
    لكل(i=0; i<pTab->n_خانہ; i++){
      pList = sqlite3_اظہاریہ_List_Append(pParse, pList, اظہاریہ_صف_کالم(pParse, i));
    }
    eDest = SRT_جدول;
  }وإلا{
    eDest = IsVirtual(pTab) ? SRT_جدول : SRT_Upfrom;
    pList = sqlite3_اظہاریہ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_صف,0,0));
#جے_تعریف_شدہ SQLITE_ENABLE_UPDATE_DELETE_حد_بندی
    إذا( p_حد_بندی ){
      pGrp = sqlite3_اظہاریہ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_صف,0,0));
    }
#جے_انت
  }
  أكد( pChanges!=0 || pParse->db->mallocFailed );
  إذا( pChanges ){
    لكل(i=0; i<pChanges->n_اظہاریہ; i++){
      pList = sqlite3_اظہاریہ_List_Append(pParse, pList,
          sqlite3_اظہاریہ_Dup(db, pChanges->a[i].p_اظہاریہ, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, p_ترتیب_By2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  إذا( pSelect ) pSelect->selFlags |= SF_ترتیب_By_Reqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_کلید_خانہ : -1);
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
فراغ sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  اظہاریہ_List *pChanges,    /* Things to be changed */
  اظہاریہ *pWhere,          /* The WHERE clause.  May be null */
  صحيح onError,           /* How to handle constraint errors */
  اظہاریہ_List *p_ترتیب_By,    /* ORDER BY clause. May be null */
  اظہاریہ *p_حد_بندی,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  صحيح i, j, k;           /* Loop counters */
  جدول *pTab;           /* The table to be updated */
  صحيح addr_چوٹی = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  اشاریہ *pIdx;           /* For looping over indices */
  اشاریہ *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  صحيح nIdx;              /* Number of indices that need updating */
  صحيح nAllIdx;           /* Total number of indexes */
  صحيح i_ادھار_Cur;          /* Base cursor number */
  صحيح i_انکڑے_Cur;          /* Cursor for the canonical data btree */
  صحيح iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  صحيح *aRegIdx = 0;      /* Registers for to each index and the main table */
  صحيح *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *a_منزل_Open;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_کلید;            /* Either chngPk or chngRowid */
  اظہاریہ *p_Rowid_اظہاریہ = 0;  /* Expression defining the new record number */
  صحيح i_Rowid_اظہاریہ = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  صحيح iDb;               /* Database containing the table being updated */
  صحيح eOnePass;          /* ONEPASS_XXX value from where.c */
  صحيح hasFK;             /* True if foreign key processing is required */
  صحيح labelBreak;        /* Jump here to break out of UPDATE loop */
  صحيح labelContinue;     /* Jump here to continue next step of UPDATE loop */
  صحيح flags;             /* Flags for sqlite3WhereBegin() */

#جے_بنا_تعریف SQLITE_OMIT_TRIGGER
  صحيح isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  صحيح tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#جے_انت
  صحيح newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  صحيح iEph = 0;          /* Ephemeral table holding all primary key values */
  صحيح n_کلید = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  صحيح aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  صحيح addrOpen = 0;      /* Address of OP_OpenEphemeral */
  صحيح iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  صحيح bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  صحيح b_انت_Seek = 1;   /* The OP_FinishSeek opcode is needed */
  صحيح nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  صحيح reg_صف_گنتی = 0;   /* A count of rows changed */
  صحيح regOldRowid = 0;   /* The old rowid */
  صحيح regNewRowid = 0;   /* The new rowid */
  صحيح regNew = 0;        /* Content of the NEW.* table in triggers */
  صحيح regOld = 0;        /* Content of OLD.* table in triggers */
  صحيح reg_صف_سیٹ = 0;     /* Rowset of rows to be updated */
  صحيح reg_کلید = 0;        /* composite PRIMARY KEY value */

  memset(&sContext, 0, حجم(sContext));
  db = pParse->db;
  أكد( db->pParse==pParse );
  إذا( pParse->nErr ){
    اذهب update_cleanup;
  }
  أكد( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  إذا( pTab==0 ) اذهب update_cleanup;
  iDb = sqlite3_Schema_منزل_اشاریہ(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#جے_بنا_تعریف SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  أكد( pTrigger || tmask==0 );
#وإلا
# define pTrigger 0
# define isView 0
# define tmask 0
#جے_انت
#جے_تعریف_شدہ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#جے_انت

#إذا TREETRACE_ENABLED
  إذا( sqlite3_درخت_Trace & 0x10000 ){
    sqlite3_درخت_View_سطر(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_درخت_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, p_ترتیب_By, p_حد_بندی, pUpsert, pTrigger);
  }
#جے_انت

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_اظہاریہ : 0;
  أكد( nChangeFrom==0 || pUpsert==0 );

#جے_تعریف_شدہ SQLITE_ENABLE_UPDATE_DELETE_حد_بندی
  إذا( !isView && nChangeFrom==0 ){
    pWhere = sqlite3_حد_بندی_Where(
        pParse, pTabList, pWhere, p_ترتیب_By, p_حد_بندی, "UPDATE"
    );
    p_ترتیب_By = 0;
    p_حد_بندی = 0;
  }
#جے_انت

  إذا( sqlite3_View_Get_کالم_ناں(pParse, pTab) ){
    اذهب update_cleanup;
  }
  إذا( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    اذهب update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  i_ادھار_Cur = i_انکڑے_Cur = pParse->nTab++;
  iIdxCur = i_انکڑے_Cur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_کلید_اشاریہ(pTab);
  testcase( pPk!=0 && pPk!=pTab->p_اشاریہ );
  لكل(nIdx=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, nIdx++){
    إذا( pPk==pIdx ){
      i_انکڑے_Cur = pParse->nTab;
    }
    pParse->nTab++;
  }
  إذا( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    i_انکڑے_Cur = pUpsert->i_انکڑے_Cur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = i_ادھار_Cur;
  }
  pTabList->a[0].iCursor = i_انکڑے_Cur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, حجم(صحيح) * (pTab->n_خانہ+nIdx+1) + nIdx+2 );
  إذا( aXRef==0 ) اذهب update_cleanup;
  aRegIdx = aXRef+pTab->n_خانہ;
  a_منزل_Open = (u8*)(aRegIdx+nIdx+1);
  memset(a_منزل_Open, 1, nIdx+1);
  a_منزل_Open[nIdx+1] = 0;
  لكل(i=0; i<pTab->n_خانہ; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  إذا( v==0 ) اذهب update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  لكل(i=0; i<pChanges->n_اظہاریہ; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    إذا( nChangeFrom==0 && sqlite3_Resolve_اظہاریہ_ناں(&sNC, pChanges->a[i].p_اظہاریہ) ){
      اذهب update_cleanup;
    }
    j = sqlite3_کالم_اشاریہ(pTab, pChanges->a[i].zEName);
    إذا( j>=0 ){
      إذا( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_اظہاریہ = pChanges->a[i].p_اظہاریہ;
        i_Rowid_اظہاریہ = i;
      }وإلا إذا( pPk && (pTab->a_خانہ[j].خانہ_Flags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
      وإلا إذا( pTab->a_خانہ[j].خانہ_Flags & COLFLAG_GENERATED ){
        testcase( pTab->a_خانہ[j].خانہ_Flags & COLFLAG_VIRTUAL );
        testcase( pTab->a_خانہ[j].خانہ_Flags & COLFLAG_STORED );
        sqlite3_Error_پیغام(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->a_خانہ[j].zCnName);
        اذهب update_cleanup;
      }
#جے_انت
      aXRef[j] = i;
    }وإلا{
      إذا( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_اظہاریہ = pChanges->a[i].p_اظہاریہ;
        i_Rowid_اظہاریہ = i;
      }وإلا{
        sqlite3_Error_پیغام(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        اذهب update_cleanup;
      }
    }
#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
    {
      صحيح rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->a_خانہ[j].zCnName,
                            db->aDb[iDb].zDbSName);
      إذا( rc!=SQLITE_OK ){
        إذا( rc==SQLITE_DENY ){
          اذهب update_cleanup;
        }وإلا إذا( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }وإلا{
          aXRef[j] = -1;
        }
      }
    }
#جے_انت
  }
  أكد( (chngRowid & chngPk)==0 );
  أكد( chngRowid==0 || chngRowid==1 );
  أكد( chngPk==0 || chngPk==1 );
  chng_کلید = chngRowid + chngPk;

#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  إذا( pTab->tabFlags & TF_HasGenerated ){
    صحيح b_ترقی;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    افعل{
      b_ترقی = 0;
      لكل(i=0; i<pTab->n_خانہ; i++){
        إذا( aXRef[i]>=0 ) استمر;
        إذا( (pTab->a_خانہ[i].خانہ_Flags & COLFLAG_GENERATED)==0 ) استمر;
        إذا( sqlite3_اظہاریہ_References_Updated_کالم(
                sqlite3_کالم_اظہاریہ(pTab, &pTab->a_خانہ[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          b_ترقی = 1;
        }
      }
    }طالما( b_ترقی );
  }
#جے_انت

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].خانہ_ورتیا = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_کلید);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  إذا( onError==OE_Replace ) bReplace = 1;
  لكل(nAllIdx=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, nAllIdx++){
    صحيح reg;
    إذا( chng_کلید || hasFK>1 || pIdx==pPk
     || اشاریہ_Where_Clause_Might_Change(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->n_کالم;
    }وإلا{
      reg = 0;
      لكل(i=0; i<pIdx->n_کلید_خانہ; i++){
        إذا( اشاریہ_کالم_Is_Being_Updated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->n_کالم;
          إذا( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          اكسر;
        }
      }
    }
    إذا( reg==0 ) a_منزل_Open[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  إذا( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    memset(a_منزل_Open, 1, nIdx+1);
  }

  إذا( pParse->nested==0 ) sqlite3_Vdbe_گنتی_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  إذا( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    أكد( aRegIdx[nAllIdx]==pParse->nMem );
    reg_صف_سیٹ = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    إذا( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->n_خانہ;
    }
    إذا( chng_کلید || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->n_خانہ;
  }

  /* Start the view context. */
  إذا( isView ){
    sqlite3_Auth_Context_ڈالو(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#إذا !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  إذا( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_ترتیب_By, p_حد_بندی, i_انکڑے_Cur
    );
    p_ترتیب_By = 0;
    p_حد_بندی = 0;
  }
#جے_انت

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  إذا( nChangeFrom==0 && sqlite3_Resolve_اظہاریہ_ناں(&sNC, pWhere) ){
    اذهب update_cleanup;
  }

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  إذا( IsVirtual(pTab) ){
    update_Virtual_جدول(pParse, pTabList, pTab, pChanges, p_Rowid_اظہاریہ, aXRef,
                       pWhere, onError);
    اذهب update_cleanup;
  }
#جے_انت

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  إذا( (db->flags&SQLITE_گنتی_صفاں)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_صف_گنتی = ++pParse->nMem;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, reg_صف_گنتی);
  }

  إذا( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, reg_صف_سیٹ, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenEphemeral, iEph, 0, reg_صف_سیٹ);
  }وإلا{
    أكد( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_کلید_خانہ : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_کلید = ++pParse->nMem;
    إذا( pUpsert==0 ){
      صحيح n_Eph_خانہ = nPk + nChangeFrom + (isView ? pTab->n_خانہ : 0);
      iEph = pParse->nTab++;
      إذا( pPk ) sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, iEph, n_Eph_خانہ);
      إذا( pPk ){
        کلید_Info *p_کلید_Info = sqlite3_کلید_Info_دا_اشاریہ(pParse, pPk);
        إذا( p_کلید_Info ){
          p_کلید_Info->n_All_میدان = n_Eph_خانہ;
          sqlite3VdbeAppendP4(v, p_کلید_Info, P4_KEYINFO);
        }
      }
      إذا( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, p_ترتیب_By, p_حد_بندی
        );
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
        إذا( isView ) i_انکڑے_Cur = iEph;
#جے_انت
      }
    }
  }

  إذا( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_کلید = nPk;
    reg_کلید = iPk;
  }وإلا{
    إذا( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_اظہاریہ_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
      b_انت_Seek = 0;
    }وإلا{
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
      إذا( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_کلید
       && !bReplace
       && (pWhere==0 || !اظہاریہ_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      إذا( pWInfo==0 ) اذهب update_cleanup;

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
      b_انت_Seek = sqlite3_Where_ورتدا_Deferred_Seek(pWInfo);
      إذا( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        إذا( eOnePass==ONEPASS_MULTI ){
          صحيح iCur = aiCurOnePass[1];
          إذا( iCur>=0 && iCur!=i_انکڑے_Cur && a_منزل_Open[iCur-i_ادھار_Cur] ){
            eOnePass = ONEPASS_OFF;
          }
          أكد( iCur!=i_انکڑے_Cur || !HasRowid(pTab) );
        }
      }
    }

    إذا( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, i_انکڑے_Cur, regOldRowid);
      إذا( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iEph, reg_صف_سیٹ, regOldRowid);
      }وإلا{
        إذا( ALWAYS(addrOpen) ) sqlite3_Vdbe_Change_منزل_Noop(v, addrOpen);
      }
    }وإلا{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      لكل(i=0; i<nPk; i++){
        أكد( pPk->ai_کالم[i]>=0 );
        sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(v, pTab, i_انکڑے_Cur,
                                        pPk->ai_کالم[i], iPk+i);
      }
      إذا( eOnePass ){
        إذا( addrOpen ) sqlite3_Vdbe_Change_منزل_Noop(v, addrOpen);
        n_کلید = nPk;
        reg_کلید = iPk;
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, iPk, nPk, reg_کلید,
                          sqlite3_اشاریہ_Affinity_Str(db, pPk), nPk);
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iEph, reg_کلید, iPk, nPk);
      }
    }
  }

  إذا( pUpsert==0 ){
    إذا( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3_Where_سرا(pWInfo);
    }

    إذا( !isView ){
      صحيح addrOnce = 0;
      صحيح iNotUsed1 = 0;
      صحيح iNotUsed2 = 0;

      /* Open every index that needs updating. */
      إذا( eOnePass!=ONEPASS_OFF ){
        إذا( aiCurOnePass[0]>=0 ) a_منزل_Open[aiCurOnePass[0]-i_ادھار_Cur] = 0;
        إذا( aiCurOnePass[1]>=0 ) a_منزل_Open[aiCurOnePass[1]-i_ادھار_Cur] = 0;
      }

      إذا( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3_Open_جدول_And_Indices(pParse, pTab, OP_OpenWrite, 0, i_ادھار_Cur,
                                 a_منزل_Open, &iNotUsed1, &iNotUsed2);
      إذا( addrOnce ){
        sqlite3_Vdbe_Jump_ایتھے_Or_نکالو_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    إذا( eOnePass!=ONEPASS_OFF ){
      إذا( aiCurOnePass[0]!=i_انکڑے_Cur
       && aiCurOnePass[1]!=i_انکڑے_Cur
#جے_تعریف_شدہ SQLITE_ALLOW_ROWID_وچ_VIEW
       && !isView
#جے_انت
      ){
        أكد( pPk );
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_لبھیا, i_انکڑے_Cur, labelBreak, reg_کلید,n_کلید);
        VdbeCoverage(v);
      }
      إذا( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, pPk ? reg_کلید : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }وإلا إذا( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_چوٹی = sqlite3VdbeCurrentAddr(v);
      إذا( nChangeFrom ){
        إذا( !isView ){
          إذا( pPk ){
            لكل(i=0; i<nPk; i++){
              sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iEph, i, iPk+i);
            }
            sqlite3_Vdbe_جوڑو_Op4_Int(
                v, OP_Not_لبھیا, i_انکڑے_Cur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }وإلا{
            sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3_Vdbe_جوڑو_Op3(
                v, OP_NotExists, i_انکڑے_Cur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_صف_انکڑے, iEph, reg_کلید);
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_لبھیا, i_انکڑے_Cur, labelContinue, reg_کلید,0);
        VdbeCoverage(v);
      }
    }وإلا{
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, i_انکڑے_Cur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  أكد( chng_کلید || pTrigger || hasFK || regOldRowid==regNewRowid );
  إذا( chngRowid ){
    أكد( i_Rowid_اظہاریہ>=0 );
    إذا( nChangeFrom==0 ){
      sqlite3_اظہاریہ_کوڈ(pParse, p_Rowid_اظہاریہ, regNewRowid);
    }وإلا{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iEph, i_Rowid_اظہاریہ, regNewRowid);
    }
    sqlite3_Vdbe_جوڑو_Op1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  إذا( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    لكل(i=0; i<pTab->n_خانہ; i++){
      u32 خانہ_Flags = pTab->a_خانہ[i].خانہ_Flags;
      k = sqlite3_جدول_کالم_منزل_Storage(pTab, i) + regOld;
      إذا( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (خانہ_Flags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(v, pTab, i_انکڑے_Cur, i, k);
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, k);
      }
    }
    إذا( chngRowid==0 && pPk==0 ){
#جے_تعریف_شدہ SQLITE_ALLOW_ROWID_وچ_VIEW
      إذا( isView ) sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, regOldRowid);
#جے_انت
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Copy, regOldRowid, regNewRowid);
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
  لكل(i=0, k=regNew; i<pTab->n_خانہ; i++, k++){
    إذا( i==pTab->iPKey ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, k);
    }وإلا إذا( (pTab->a_خانہ[i].خانہ_Flags & COLFLAG_GENERATED)!=0 ){
      إذا( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_VIRTUAL ) k--;
    }وإلا{
      j = aXRef[i];
      إذا( j>=0 ){
        إذا( nChangeFrom ){
          صحيح nOff = (isView ? pTab->n_خانہ : nPk);
          أكد( eOnePass==ONEPASS_OFF );
          sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iEph, nOff+j, k);
        }وإلا{
          sqlite3_اظہاریہ_کوڈ(pParse, pChanges->a[j].p_اظہاریہ, k);
        }
      }وإلا إذا( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(v, pTab, i_انکڑے_Cur, i, k);
        b_انت_Seek = 0;
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, k);
      }
    }
  }
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
  إذا( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#جے_انت

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  إذا( tmask&TRIGGER_BEFORE ){
    sqlite3_جدول_Affinity(v, pTab, regNew);
    sqlite3_کوڈ_صف_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    إذا( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      إذا( pPk ){
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_لبھیا,i_انکڑے_Cur,labelContinue,reg_کلید,n_کلید);
        VdbeCoverage(v);
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, i_انکڑے_Cur, labelContinue,regOldRowid);
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
      لكل(i=0, k=regNew; i<pTab->n_خانہ; i++, k++){
        إذا( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_GENERATED ){
          إذا( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_VIRTUAL ) k--;
        }وإلا إذا( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(v, pTab, i_انکڑے_Cur, i, k);
        }
      }
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
      إذا( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#جے_انت
    }
  }

  إذا( !isView ){
    /* Do constraint checks. */
    أكد( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_انکڑے_Cur, iIdxCur,
        regNewRowid, regOldRowid, chng_کلید, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    إذا( bReplace || chng_کلید ){
      إذا( pPk ){
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_لبھیا,i_انکڑے_Cur,labelContinue,reg_کلید,n_کلید);
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, i_انکڑے_Cur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    إذا( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_کلید);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3_Generate_صف_اشاریہ_Delete(pParse, pTab, i_انکڑے_Cur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    إذا( b_انت_Seek ){
      sqlite3_Vdbe_جوڑو_Op1(v, OP_انت_Seek, i_انکڑے_Cur);
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
    أكد( regNew==regNewRowid+1 );
#جے_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Delete, i_انکڑے_Cur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_کلید) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    إذا( eOnePass==ONEPASS_MULTI ){
      أكد( hasFK==0 && chng_کلید==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    إذا( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_جدول);
    }
#وإلا
    إذا( hasFK>1 || chng_کلید ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Delete, i_انکڑے_Cur, 0);
    }
#جے_انت

    إذا( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_کلید);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, i_انکڑے_Cur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    إذا( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_کلید);
    }
  }

  /* Increment the row counter
  */
  إذا( reg_صف_گنتی ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, reg_صف_گنتی, 1);
  }

  إذا( pTrigger ){
    sqlite3_کوڈ_صف_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  إذا( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }وإلا إذا( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Where_سرا(pWInfo);
  }وإلا{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iEph, addr_چوٹی); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  إذا( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3_Autoincrement_سرا(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  إذا( reg_صف_گنتی ){
    sqlite3_کوڈ_Change_گنتی(v, reg_صف_گنتی, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_نکالو(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_اظہاریہ_List_Delete(db, pChanges);
  sqlite3_اظہاریہ_Delete(db, pWhere);
#إذا defined(SQLITE_ENABLE_UPDATE_DELETE_حد_بندی)
  sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
  sqlite3_اظہاریہ_Delete(db, p_حد_بندی);
#جے_انت
  أرجع;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#جے_تعریف_شدہ isView
 #تعریف_رد isView
#جے_انت
#جے_تعریف_شدہ pTrigger
 #تعریف_رد pTrigger
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
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
ساكن فراغ update_Virtual_جدول(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  جدول *pTab,         /* The virtual table */
  اظہاریہ_List *pChanges,  /* The columns to change in the UPDATE statement */
  اظہاریہ *pRowid,        /* Expression used to recompute the rowid */
  صحيح *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  اظہاریہ *pWhere,        /* WHERE clause of the UPDATE statement */
  صحيح onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  صحيح ephemTab;             /* Table holding the result of the SELECT */
  صحيح i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  ثابت حرف *pVTab = (ثابت حرف*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  صحيح nArg = 2 + pTab->n_خانہ;      /* Number of arguments to VUpdate */
  صحيح regArg;                     /* First register in VUpdate arg array */
  صحيح regRec;                     /* Register in which to assemble record */
  صحيح regRowid;                   /* Register for ephemeral table rowid */
  صحيح iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  صحيح aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  صحيح eOnePass;                   /* True to use onepass strategy */
  صحيح addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  أكد( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  إذا( pSrc->nSrc>1 ){
    اشاریہ *pPk = 0;
    اظہاریہ *p_صف;
    اظہاریہ_List *pList;
    إذا( HasRowid(pTab) ){
      إذا( pRowid ){
        p_صف = sqlite3_اظہاریہ_Dup(db, pRowid, 0);
      }وإلا{
        p_صف = sqlite3PExpr(pParse, TK_صف, 0, 0);
      }
    }وإلا{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      أكد( pPk!=0 );
      أكد( pPk->n_کلید_خانہ==1 );
      iPk = pPk->ai_کالم[0];
      إذا( aXRef[iPk]>=0 ){
        p_صف = sqlite3_اظہاریہ_Dup(db, pChanges->a[aXRef[iPk]].p_اظہاریہ, 0);
      }وإلا{
        p_صف = اظہاریہ_صف_کالم(pParse, iPk);
      }
    }
    pList = sqlite3_اظہاریہ_List_Append(pParse, 0, p_صف);

    لكل(i=0; i<pTab->n_خانہ; i++){
      إذا( aXRef[i]>=0 ){
        pList = sqlite3_اظہاریہ_List_Append(pParse, pList,
          sqlite3_اظہاریہ_Dup(db, pChanges->a[aXRef[i]].p_اظہاریہ, 0)
        );
      }وإلا{
        اظہاریہ *p_صف_اظہاریہ = اظہاریہ_صف_کالم(pParse, i);
        إذا( p_صف_اظہاریہ ) p_صف_اظہاریہ->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_اظہاریہ_List_Append(pParse, pList, p_صف_اظہاریہ);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_اظہاریہ_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }وإلا{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    إذا( pWInfo==0 ) أرجع;

    /* Populate the argument registers. */
    لكل(i=0; i<pTab->n_خانہ; i++){
      أكد( (pTab->a_خانہ[i].خانہ_Flags & COLFLAG_GENERATED)==0 );
      إذا( aXRef[i]>=0 ){
        sqlite3_اظہاریہ_کوڈ(pParse, pChanges->a[aXRef[i]].p_اظہاریہ, regArg+2+i);
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    إذا( HasRowid(pTab) ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iCsr, regArg);
      إذا( pRowid ){
        sqlite3_اظہاریہ_کوڈ(pParse, pRowid, regArg+1);
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iCsr, regArg+1);
      }
    }وإلا{
      اشاریہ *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      أكد( pPk!=0 );
      أكد( pPk->n_کلید_خانہ==1 );
      iPk = pPk->ai_کالم[0];
      sqlite3_Vdbe_جوڑو_Op3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    أكد( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    إذا( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3_Vdbe_Change_منزل_Noop(v, addr);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Close, iCsr);
    }وإلا{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, regArg, nArg, regRec);
#إذا defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#جے_انت
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, ephemTab, regRec, regRowid);
    }
  }


  إذا( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    إذا( pSrc->nSrc==1 ){
      sqlite3_Where_سرا(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    لكل(i=0; i<nArg; i++){
      sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3_Vdbe_جوڑو_Op4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  إذا( eOnePass==ONEPASS_OFF ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ایتھے(v, addr);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Close, ephemTab, 0);
  }وإلا{
    sqlite3_Where_سرا(pWInfo);
  }
}
#جے_انت /* SQLITE_OMIT_VIRTUALTABLE */
