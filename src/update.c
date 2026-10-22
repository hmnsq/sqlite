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
#شامل "sqliteInt.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
/* Forward declaration */
ساکن خالی update_Virtual_جدول(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  جدول *pTab,         /* The virtual table */
  اظہاریہ_List *pChanges,  /* The columns to change in the UPDATE statement */
  اظہاریہ *p_Rowid_اظہاریہ,    /* Expression used to recompute the rowid */
  صحیح *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  اظہاریہ *pWhere,        /* WHERE clause of the UPDATE statement */
  صحیح onError          /* ON CONFLICT strategy */
);
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */

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
ساکن SQLITE_NOINLINE خالی عمود_Default_Uncommon_Case(
  Vdbe *v,          /* Byte code under construction */
  جدول *pTab,      /* The table */
  عمود *p_ستون      /* Which column of the table */
){
  sqlite3_قدر *p_قدر = 0;
  u8 enc = ENC(sqlite3VdbeDb(v));
  Parse *pParse = sqlite3VdbeParser(v);
  أكد( !IsView(pTab) );
  sqlite3_قدر_From_اظہاریہ(sqlite3VdbeDb(v),
                       sqlite3_عمود_اظہاریہ_Auth(pTab,p_ستون,pParse), enc,
                       p_ستون->affinity, &p_قدر);
  اگر( p_قدر ){
    sqlite3VdbeAppendP4(v, p_قدر, P4_MEM);
  }
}
خالی sqlite3_عمود_Default(
  Vdbe *v,          /* Byte code under construction */
  جدول *pTab,      /* The table */
  صحیح i,            /* Which column of the table */
  صحیح iReg          /* Register in which results are stored */
){
  عمود *p_ستون;
  أكد( pTab!=0 );
  أكد( pTab->n_ستون>i );
  p_ستون = &pTab->a_ستون[i];
  اگر( p_ستون->iDflt ){
    عمود_Default_Uncommon_Case(v,pTab,p_ستون);
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
  اگر( p_ستون->affinity==SQLITE_AFF_REAL ){
    sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, iReg);
  }
#اگر_ختم
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
ساکن صحیح اشاریہ_عمود_Is_Being_Updated(
  اشاریہ *pIdx,      /* The index to check */
  صحیح i_ستون,         /* Which column of the index to check */
  صحیح *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  صحیح chngRowid     /* true if the rowid is being updated */
){
  i16 i_Idx_ستون = pIdx->ai_عمود[i_ستون];
  أكد( i_Idx_ستون!=XN_ROWID ); /* Cannot index rowid */
  اگر( i_Idx_ستون>=0 ){
    واپس aXRef[i_Idx_ستون]>=0;
  }
  أكد( i_Idx_ستون==XN_اظہاریہ );
  أكد( pIdx->a_ستون_اظہاریہ!=0 );
  أكد( pIdx->a_ستون_اظہاریہ->a[i_ستون].p_اظہاریہ!=0 );
  واپس sqlite3_اظہاریہ_References_Updated_عمود(pIdx->a_ستون_اظہاریہ->a[i_ستون].p_اظہاریہ,
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
ساکن صحیح اشاریہ_Where_Clause_Might_Change(
  اشاریہ *pIdx,      /* The index to check */
  صحیح *aXRef,       /* aXRef[j]>=0 if column j is being updated */
  صحیح chngRowid     /* true if the rowid is being updated */
){
  اگر( pIdx->pPartIdxWhere==0 ) واپس 0;
  واپس sqlite3_اظہاریہ_References_Updated_عمود(pIdx->pPartIdxWhere,
                                            aXRef, chngRowid);
}

/*
** Allocate and return a pointer to an expression of type TK_ROW with
** Expr.iColumn set to value (iCol+1). The resolver will modify the
** expression to be a TK_COLUMN reading column iCol of the first
** table in the source-list (pSrc->a[0]).
*/
ساکن اظہاریہ *اظہاریہ_صف_عمود(Parse *pParse, صحیح i_ستون){
  اظہاریہ *pRet = sqlite3PExpr(pParse, TK_صف, 0, 0);
  اگر( pRet ) pRet->i_عمود = i_ستون+1;
  واپس pRet;
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
ساکن خالی updateFromSelect(
  Parse *pParse,                  /* Parse context */
  صحیح iEph,                       /* Cursor for open eph. table */
  اشاریہ *pPk,                     /* PK if table 0 is WITHOUT ROWID */
  اظہاریہ_List *pChanges,             /* List of expressions to return */
  SrcList *pTabList,              /* List of tables to select from */
  اظہاریہ *pWhere,                   /* WHERE clause for query */
  اظہاریہ_List *p_ترتیب_By,             /* ORDER BY clause */
  اظہاریہ *p_حد_بندی                    /* LIMIT clause */
){
  صحیح i;
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
  صحیح eDest;

#اگر_تعریف_شدہ SQLITE_ENABLE_UPDATE_DELETE_حد_بندی
  اگر( p_ترتیب_By && p_حد_بندی==0 ) {
    sqlite3_Error_پیغام(pParse, "ORDER BY without LIMIT on UPDATE");
    واپس;
  }
  p_ترتیب_By2 = sqlite3_اظہاریہ_List_Dup(db, p_ترتیب_By, 0);
  pLimit2 = sqlite3_اظہاریہ_Dup(db, p_حد_بندی, 0);
#ورنہ
  غیر_مستعمل_PARAMETER(p_ترتیب_By);
  غیر_مستعمل_PARAMETER(p_حد_بندی);
#اگر_ختم

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3_اظہاریہ_Dup(db, pWhere, 0);

  أكد( pTabList->nSrc>1 );
  اگر( pSrc ){
    أكد( pSrc->a[0].fg.notCte );
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  اگر( pPk ){
    برائے(i=0; i<pPk->n_کلید_ستون; i++){
      اظہاریہ *pNew = اظہاریہ_صف_عمود(pParse, pPk->ai_عمود[i]);
#اگر_تعریف_شدہ SQLITE_ENABLE_UPDATE_DELETE_حد_بندی
      اگر( p_حد_بندی ){
        pGrp = sqlite3_اظہاریہ_List_Append(pParse, pGrp, sqlite3_اظہاریہ_Dup(db, pNew, 0));
      }
#اگر_ختم
      pList = sqlite3_اظہاریہ_List_Append(pParse, pList, pNew);
    }
    eDest = IsVirtual(pTab) ? SRT_جدول : SRT_Upfrom;
  }ورنہ اگر( IsView(pTab) ){
    برائے(i=0; i<pTab->n_ستون; i++){
      pList = sqlite3_اظہاریہ_List_Append(pParse, pList, اظہاریہ_صف_عمود(pParse, i));
    }
    eDest = SRT_جدول;
  }ورنہ{
    eDest = IsVirtual(pTab) ? SRT_جدول : SRT_Upfrom;
    pList = sqlite3_اظہاریہ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_صف,0,0));
#اگر_تعریف_شدہ SQLITE_ENABLE_UPDATE_DELETE_حد_بندی
    اگر( p_حد_بندی ){
      pGrp = sqlite3_اظہاریہ_List_Append(pParse, 0, sqlite3PExpr(pParse,TK_صف,0,0));
    }
#اگر_ختم
  }
  أكد( pChanges!=0 || pParse->db->mallocFailed );
  اگر( pChanges ){
    برائے(i=0; i<pChanges->n_اظہاریہ; i++){
      pList = sqlite3_اظہاریہ_List_Append(pParse, pList,
          sqlite3_اظہاریہ_Dup(db, pChanges->a[i].p_اظہاریہ, 0)
      );
    }
  }
  pSelect = sqlite3SelectNew(pParse, pList,
      pSrc, pWhere2, pGrp, 0, p_ترتیب_By2,
      SF_UFSrcCheck|SF_IncludeHidden|SF_UpdateFrom, pLimit2
  );
  اگر( pSelect ) pSelect->selFlags |= SF_ترتیب_By_Reqd;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->n_کلید_ستون : -1);
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
خالی sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  اظہاریہ_List *pChanges,    /* Things to be changed */
  اظہاریہ *pWhere,          /* The WHERE clause.  May be null */
  صحیح onError,           /* How to handle constraint errors */
  اظہاریہ_List *p_ترتیب_By,    /* ORDER BY clause. May be null */
  اظہاریہ *p_حد_بندی,          /* LIMIT clause. May be null */
  Upsert *pUpsert        /* ON CONFLICT clause, or null */
){
  صحیح i, j, k;           /* Loop counters */
  جدول *pTab;           /* The table to be updated */
  صحیح addr_چوٹی = 0;       /* VDBE instruction address of the start of the loop */
  WhereInfo *pWInfo = 0; /* Information about the WHERE clause */
  Vdbe *v;               /* The virtual database engine */
  اشاریہ *pIdx;           /* For looping over indices */
  اشاریہ *pPk;            /* The PRIMARY KEY index for WITHOUT ROWID tables */
  صحیح nIdx;              /* Number of indices that need updating */
  صحیح nAllIdx;           /* Total number of indexes */
  صحیح i_بنیاد_Cur;          /* Base cursor number */
  صحیح i_ڈیٹا_Cur;          /* Cursor for the canonical data btree */
  صحیح iIdxCur;           /* Cursor for the first index */
  sqlite3 *db;           /* The database structure */
  صحیح *aRegIdx = 0;      /* Registers for to each index and the main table */
  صحیح *aXRef = 0;        /* aXRef[i] is the index in pChanges->a[] of the
                         ** an expression for the i-th column of the table.
                         ** aXRef[i]==-1 if the i-th column is not changed. */
  u8 *a_مقصد_Open;           /* 1 for tables and indices to be opened */
  u8 chngPk;             /* PRIMARY KEY changed in a WITHOUT ROWID table */
  u8 chngRowid;          /* Rowid changed in a normal table */
  u8 chng_کلید;            /* Either chngPk or chngRowid */
  اظہاریہ *p_Rowid_اظہاریہ = 0;  /* Expression defining the new record number */
  صحیح i_Rowid_اظہاریہ = -1;   /* Index of "rowid=" (or IPK) assignment in pChanges */
  AuthContext sContext;  /* The authorization context */
  NameContext sNC;       /* The name-context to resolve expressions in */
  صحیح iDb;               /* Database containing the table being updated */
  صحیح eOnePass;          /* ONEPASS_XXX value from where.c */
  صحیح hasFK;             /* True if foreign key processing is required */
  صحیح labelBreak;        /* Jump here to break out of UPDATE loop */
  صحیح labelContinue;     /* Jump here to continue next step of UPDATE loop */
  صحیح flags;             /* Flags for sqlite3WhereBegin() */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
  صحیح isView;            /* True when updating a view (INSTEAD OF trigger) */
  Trigger *pTrigger;     /* List of triggers on pTab, if required */
  صحیح tmask;             /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
#اگر_ختم
  صحیح newmask;           /* Mask of NEW.* columns accessed by BEFORE triggers */
  صحیح iEph = 0;          /* Ephemeral table holding all primary key values */
  صحیح n_کلید = 0;          /* Number of elements in regKey for WITHOUT ROWID */
  صحیح aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  صحیح addrOpen = 0;      /* Address of OP_OpenEphemeral */
  صحیح iPk = 0;           /* First of nPk cells holding PRIMARY KEY value */
  i16 nPk = 0;           /* Number of components of the PRIMARY KEY */
  صحیح bReplace = 0;      /* True if REPLACE conflict resolution might happen */
  صحیح b_اختتام_Seek = 1;   /* The OP_FinishSeek opcode is needed */
  صحیح nChangeFrom = 0;   /* If there is a FROM, pChanges->nExpr, else 0 */

  /* Register Allocations */
  صحیح reg_صف_گنتی = 0;   /* A count of rows changed */
  صحیح regOldRowid = 0;   /* The old rowid */
  صحیح regNewRowid = 0;   /* The new rowid */
  صحیح regNew = 0;        /* Content of the NEW.* table in triggers */
  صحیح regOld = 0;        /* Content of OLD.* table in triggers */
  صحیح reg_صف_سیٹ = 0;     /* Rowset of rows to be updated */
  صحیح reg_کلید = 0;        /* composite PRIMARY KEY value */

  memset(&sContext, 0, حجم(sContext));
  db = pParse->db;
  أكد( db->pParse==pParse );
  اگر( pParse->nErr ){
    جاؤ update_cleanup;
  }
  أكد( db->mallocFailed==0 );

  /* Locate the table which we want to update.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  اگر( pTab==0 ) جاؤ update_cleanup;
  iDb = sqlite3_Schema_مقصد_اشاریہ(pParse->db, pTab->pSchema);

  /* Figure out if we have any triggers and if the table being
  ** updated is a view.
  */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_UPDATE, pChanges, &tmask);
  isView = IsView(pTab);
  أكد( pTrigger || tmask==0 );
#ورنہ
# define pTrigger 0
# define isView 0
# define tmask 0
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#اگر_ختم

#اگر TREETRACE_ENABLED
  اگر( sqlite3_درخت_Trace & 0x10000 ){
    sqlite3_درخت_View_سطر(0, "In sqlite3Update() at %s:%d", __FILE__, __LINE__);
    sqlite3_درخت_View_Update(pParse->pWith, pTabList, pChanges, pWhere,
                          onError, p_ترتیب_By, p_حد_بندی, pUpsert, pTrigger);
  }
#اگر_ختم

  /* If there was a FROM clause, set nChangeFrom to the number of expressions
  ** in the change-list. Otherwise, set it to 0. There cannot be a FROM
  ** clause if this function is being called to generate code for part of
  ** an UPSERT statement.  */
  nChangeFrom = (pTabList->nSrc>1) ? pChanges->n_اظہاریہ : 0;
  أكد( nChangeFrom==0 || pUpsert==0 );

#اگر_تعریف_شدہ SQLITE_ENABLE_UPDATE_DELETE_حد_بندی
  اگر( !isView && nChangeFrom==0 ){
    pWhere = sqlite3_حد_بندی_Where(
        pParse, pTabList, pWhere, p_ترتیب_By, p_حد_بندی, "UPDATE"
    );
    p_ترتیب_By = 0;
    p_حد_بندی = 0;
  }
#اگر_ختم

  اگر( sqlite3_View_Get_عمود_نام(pParse, pTab) ){
    جاؤ update_cleanup;
  }
  اگر( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    جاؤ update_cleanup;
  }

  /* Allocate a cursors for the main database table and for all indices.
  ** The index cursors might not be used, but if they are used they
  ** need to occur right after the database cursor.  So go ahead and
  ** allocate enough space, just in case.
  */
  i_بنیاد_Cur = i_ڈیٹا_Cur = pParse->nTab++;
  iIdxCur = i_ڈیٹا_Cur+1;
  pPk = HasRowid(pTab) ? 0 : sqlite3_Primary_کلید_اشاریہ(pTab);
  testcase( pPk!=0 && pPk!=pTab->p_اشاریہ );
  برائے(nIdx=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, nIdx++){
    اگر( pPk==pIdx ){
      i_ڈیٹا_Cur = pParse->nTab;
    }
    pParse->nTab++;
  }
  اگر( pUpsert ){
    /* On an UPSERT, reuse the same cursors already opened by INSERT */
    i_ڈیٹا_Cur = pUpsert->i_ڈیٹا_Cur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = i_بنیاد_Cur;
  }
  pTabList->a[0].iCursor = i_ڈیٹا_Cur;

  /* Allocate space for aXRef[], aRegIdx[], and aToOpen[].
  ** Initialize aXRef[] and aToOpen[] to their default values.
  */
  aXRef = sqlite3DbMallocRawNN(db, حجم(صحیح) * (pTab->n_ستون+nIdx+1) + nIdx+2 );
  اگر( aXRef==0 ) جاؤ update_cleanup;
  aRegIdx = aXRef+pTab->n_ستون;
  a_مقصد_Open = (u8*)(aRegIdx+nIdx+1);
  memset(a_مقصد_Open, 1, nIdx+1);
  a_مقصد_Open[nIdx+1] = 0;
  برائے(i=0; i<pTab->n_ستون; i++) aXRef[i] = -1;

  /* Initialize the name-context */
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = NC_UUpsert;

  /* Begin generating code. */
  v = sqlite3GetVdbe(pParse);
  اگر( v==0 ) جاؤ update_cleanup;

  /* Resolve the column names in all the expressions of the
  ** of the UPDATE statement.  Also find the column index
  ** for each column to be updated in the pChanges array.  For each
  ** column to be updated, make sure we have authorization to change
  ** that column.
  */
  chngRowid = chngPk = 0;
  برائے(i=0; i<pChanges->n_اظہاریہ; i++){
    /* If this is an UPDATE with a FROM clause, do not resolve expressions
    ** here. The call to sqlite3Select() below will do that. */
    اگر( nChangeFrom==0 && sqlite3_Resolve_اظہاریہ_نام(&sNC, pChanges->a[i].p_اظہاریہ) ){
      جاؤ update_cleanup;
    }
    j = sqlite3_عمود_اشاریہ(pTab, pChanges->a[i].zEName);
    اگر( j>=0 ){
      اگر( j==pTab->iPKey ){
        chngRowid = 1;
        p_Rowid_اظہاریہ = pChanges->a[i].p_اظہاریہ;
        i_Rowid_اظہاریہ = i;
      }ورنہ اگر( pPk && (pTab->a_ستون[j].ستون_Flags & COLFLAG_PRIMKEY)!=0 ){
        chngPk = 1;
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
      ورنہ اگر( pTab->a_ستون[j].ستون_Flags & COLFLAG_GENERATED ){
        testcase( pTab->a_ستون[j].ستون_Flags & COLFLAG_VIRTUAL );
        testcase( pTab->a_ستون[j].ستون_Flags & COLFLAG_STORED );
        sqlite3_Error_پیغام(pParse,
           "cannot UPDATE generated column \"%s\"",
           pTab->a_ستون[j].zCnName);
        جاؤ update_cleanup;
      }
#اگر_ختم
      aXRef[j] = i;
    }ورنہ{
      اگر( pPk==0 && sqlite3IsRowid(pChanges->a[i].zEName) ){
        j = -1;
        chngRowid = 1;
        p_Rowid_اظہاریہ = pChanges->a[i].p_اظہاریہ;
        i_Rowid_اظہاریہ = i;
      }ورنہ{
        sqlite3_Error_پیغام(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        جاؤ update_cleanup;
      }
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
    {
      صحیح rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName,
                            j<0 ? "ROWID" : pTab->a_ستون[j].zCnName,
                            db->aDb[iDb].zDbSName);
      اگر( rc!=SQLITE_OK ){
        اگر( rc==SQLITE_DENY ){
          جاؤ update_cleanup;
        }ورنہ اگر( j<0 || j==pTab->iPKey ){
          chngRowid = 0;
        }ورنہ{
          aXRef[j] = -1;
        }
      }
    }
#اگر_ختم
  }
  أكد( (chngRowid & chngPk)==0 );
  أكد( chngRowid==0 || chngRowid==1 );
  أكد( chngPk==0 || chngPk==1 );
  chng_کلید = chngRowid + chngPk;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
  /* Mark generated columns as changing if their generator expressions
  ** reference any changing column.  The actual aXRef[] value for
  ** generated expressions is not used, other than to check to see that it
  ** is non-negative, so the value of aXRef[] for generated columns can be
  ** set to any non-negative number.  We use 99999 so that the value is
  ** obvious when looking at aXRef[] in a symbolic debugger.
  */
  اگر( pTab->tabFlags & TF_HasGenerated ){
    صحیح b_پیش_رفت;
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    کرو{
      b_پیش_رفت = 0;
      برائے(i=0; i<pTab->n_ستون; i++){
        اگر( aXRef[i]>=0 ) جاری;
        اگر( (pTab->a_ستون[i].ستون_Flags & COLFLAG_GENERATED)==0 ) جاری;
        اگر( sqlite3_اظہاریہ_References_Updated_عمود(
                sqlite3_عمود_اظہاریہ(pTab, &pTab->a_ستون[i]),
                 aXRef, chngRowid)
        ){
          aXRef[i] = 99999;
          b_پیش_رفت = 1;
        }
      }
    }جب( b_پیش_رفت );
  }
#اگر_ختم

  /* The SET expressions are not actually used inside the WHERE loop.
  ** So reset the colUsed mask. Unless this is a virtual table. In that
  ** case, set all bits of the colUsed mask (to ensure that the virtual
  ** table implementation makes all columns available).
  */
  pTabList->a[0].ستون_مستعمل = IsVirtual(pTab) ? ALLBITS : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chng_کلید);

  /* There is one entry in the aRegIdx[] array for each index on the table
  ** being updated.  Fill in aRegIdx[] with a register number that will hold
  ** the key for accessing each index.
  */
  اگر( onError==OE_Replace ) bReplace = 1;
  برائے(nAllIdx=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, nAllIdx++){
    صحیح reg;
    اگر( chng_کلید || hasFK>1 || pIdx==pPk
     || اشاریہ_Where_Clause_Might_Change(pIdx,aXRef,chngRowid)
    ){
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->n_عمود;
    }ورنہ{
      reg = 0;
      برائے(i=0; i<pIdx->n_کلید_ستون; i++){
        اگر( اشاریہ_عمود_Is_Being_Updated(pIdx, i, aXRef, chngRowid) ){
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->n_عمود;
          اگر( onError==OE_Default && pIdx->onError==OE_Replace ){
            bReplace = 1;
          }
          توڑو;
        }
      }
    }
    اگر( reg==0 ) a_مقصد_Open[nAllIdx+1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;  /* Register storing the table record */
  اگر( bReplace ){
    /* If REPLACE conflict resolution might be invoked, open cursors on all
    ** indexes in case they are needed to delete records.  */
    memset(a_مقصد_Open, 1, nIdx+1);
  }

  اگر( pParse->nested==0 ) sqlite3_Vdbe_گنتی_Changes(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  /* Allocate required registers. */
  اگر( !IsVirtual(pTab) ){
    /* For now, regRowSet and aRegIdx[nAllIdx] share the same register.
    ** If regRowSet turns out to be needed, then aRegIdx[nAllIdx] will be
    ** reallocated.  aRegIdx[nAllIdx] is the register in which the main
    ** table record is written.  regRowSet holds the RowSet for the
    ** two-pass update algorithm. */
    أكد( aRegIdx[nAllIdx]==pParse->nMem );
    reg_صف_سیٹ = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    اگر( chngPk || pTrigger || hasFK ){
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->n_ستون;
    }
    اگر( chng_کلید || pTrigger || hasFK ){
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->n_ستون;
  }

  /* Start the view context. */
  اگر( isView ){
    sqlite3_Auth_Context_ڈالو(pParse, &sContext, pTab->zName);
  }

  /* If we are trying to update a view, realize that view into
  ** an ephemeral table.
  */
#اگر !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  اگر( nChangeFrom==0 && isView ){
    sqlite3MaterializeView(pParse, pTab,
        pWhere, p_ترتیب_By, p_حد_بندی, i_ڈیٹا_Cur
    );
    p_ترتیب_By = 0;
    p_حد_بندی = 0;
  }
#اگر_ختم

  /* Resolve the column names in all the expressions in the
  ** WHERE clause.
  */
  اگر( nChangeFrom==0 && sqlite3_Resolve_اظہاریہ_نام(&sNC, pWhere) ){
    جاؤ update_cleanup;
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  /* Virtual tables must be handled separately */
  اگر( IsVirtual(pTab) ){
    update_Virtual_جدول(pParse, pTabList, pTab, pChanges, p_Rowid_اظہاریہ, aXRef,
                       pWhere, onError);
    جاؤ update_cleanup;
  }
#اگر_ختم

  /* Jump to labelBreak to abandon further processing of this UPDATE */
  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  /* Not an UPSERT.  Normal processing.  Begin by
  ** initialize the count of updated rows */
  اگر( (db->flags&SQLITE_گنتی_صفیں)!=0
   && !pParse->pTriggerTab
   && !pParse->nested
   && !pParse->bReturning
   && pUpsert==0
  ){
    reg_صف_گنتی = ++pParse->nMem;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, reg_صف_گنتی);
  }

  اگر( nChangeFrom==0 && HasRowid(pTab) ){
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, reg_صف_سیٹ, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenEphemeral, iEph, 0, reg_صف_سیٹ);
  }ورنہ{
    أكد( pPk!=0 || HasRowid(pTab) );
    nPk = pPk ? pPk->n_کلید_ستون : 0;
    iPk = pParse->nMem+1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    reg_کلید = ++pParse->nMem;
    اگر( pUpsert==0 ){
      صحیح n_Eph_ستون = nPk + nChangeFrom + (isView ? pTab->n_ستون : 0);
      iEph = pParse->nTab++;
      اگر( pPk ) sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, iPk, iPk+nPk-1);
      addrOpen = sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, iEph, n_Eph_ستون);
      اگر( pPk ){
        کلید_Info *p_کلید_Info = sqlite3_کلید_Info_کا_اشاریہ(pParse, pPk);
        اگر( p_کلید_Info ){
          p_کلید_Info->n_All_میدان = n_Eph_ستون;
          sqlite3VdbeAppendP4(v, p_کلید_Info, P4_KEYINFO);
        }
      }
      اگر( nChangeFrom ){
        updateFromSelect(
            pParse, iEph, pPk, pChanges, pTabList, pWhere, p_ترتیب_By, p_حد_بندی
        );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
        اگر( isView ) i_ڈیٹا_Cur = iEph;
#اگر_ختم
      }
    }
  }

  اگر( nChangeFrom ){
    sqlite3MultiWrite(pParse);
    eOnePass = ONEPASS_OFF;
    n_کلید = nPk;
    reg_کلید = iPk;
  }ورنہ{
    اگر( pUpsert ){
      /* If this is an UPSERT, then all cursors have already been opened by
      ** the outer INSERT and the data cursor should be pointing at the row
      ** that is to be updated.  So bypass the code that searches for the
      ** row(s) to be updated.
      */
      pWInfo = 0;
      eOnePass = ONEPASS_SINGLE;
      sqlite3_اظہاریہ_If_False(pParse, pWhere, labelBreak, SQLITE_JUMPIFNULL);
      b_اختتام_Seek = 0;
    }ورنہ{
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
      اگر( !pParse->nested
       && !pTrigger
       && !hasFK
       && !chng_کلید
       && !bReplace
       && (pWhere==0 || !اظہاریہ_Has_Property(pWhere, EP_Subquery))
      ){
        flags |= WHERE_ONEPASS_MULTIROW;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere,0,0,0,flags,iIdxCur);
      اگر( pWInfo==0 ) جاؤ update_cleanup;

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
      b_اختتام_Seek = sqlite3_Where_استعمال_کرتا_Deferred_Seek(pWInfo);
      اگر( eOnePass!=ONEPASS_SINGLE ){
        sqlite3MultiWrite(pParse);
        اگر( eOnePass==ONEPASS_MULTI ){
          صحیح iCur = aiCurOnePass[1];
          اگر( iCur>=0 && iCur!=i_ڈیٹا_Cur && a_مقصد_Open[iCur-i_بنیاد_Cur] ){
            eOnePass = ONEPASS_OFF;
          }
          أكد( iCur!=i_ڈیٹا_Cur || !HasRowid(pTab) );
        }
      }
    }

    اگر( HasRowid(pTab) ){
      /* Read the rowid of the current row of the WHERE scan. In ONEPASS_OFF
      ** mode, write the rowid into the FIFO. In either of the one-pass modes,
      ** leave it in register regOldRowid.  */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, i_ڈیٹا_Cur, regOldRowid);
      اگر( eOnePass==ONEPASS_OFF ){
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iEph, reg_صف_سیٹ, regOldRowid);
      }ورنہ{
        اگر( ALWAYS(addrOpen) ) sqlite3_Vdbe_Change_مقصد_Noop(v, addrOpen);
      }
    }ورنہ{
      /* Read the PK of the current row into an array of registers. In
      ** ONEPASS_OFF mode, serialize the array into a record and store it in
      ** the ephemeral table. Or, in ONEPASS_SINGLE or MULTI mode, change
      ** the OP_OpenEphemeral instruction to a Noop (the ephemeral table
      ** is not required) and leave the PK fields in the array of registers.  */
      برائے(i=0; i<nPk; i++){
        أكد( pPk->ai_عمود[i]>=0 );
        sqlite3_اظہاریہ_رمز_Get_عمود_کا_جدول(v, pTab, i_ڈیٹا_Cur,
                                        pPk->ai_عمود[i], iPk+i);
      }
      اگر( eOnePass ){
        اگر( addrOpen ) sqlite3_Vdbe_Change_مقصد_Noop(v, addrOpen);
        n_کلید = nPk;
        reg_کلید = iPk;
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, iPk, nPk, reg_کلید,
                          sqlite3_اشاریہ_Affinity_Str(db, pPk), nPk);
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iEph, reg_کلید, iPk, nPk);
      }
    }
  }

  اگر( pUpsert==0 ){
    اگر( nChangeFrom==0 && eOnePass!=ONEPASS_MULTI ){
      sqlite3_Where_انتہا(pWInfo);
    }

    اگر( !isView ){
      صحیح addrOnce = 0;
      صحیح iNotUsed1 = 0;
      صحیح iNotUsed2 = 0;

      /* Open every index that needs updating. */
      اگر( eOnePass!=ONEPASS_OFF ){
        اگر( aiCurOnePass[0]>=0 ) a_مقصد_Open[aiCurOnePass[0]-i_بنیاد_Cur] = 0;
        اگر( aiCurOnePass[1]>=0 ) a_مقصد_Open[aiCurOnePass[1]-i_بنیاد_Cur] = 0;
      }

      اگر( eOnePass==ONEPASS_MULTI && (nIdx-(aiCurOnePass[1]>=0))>0 ){
        addrOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
      }
      sqlite3_Open_جدول_And_Indices(pParse, pTab, OP_OpenWrite, 0, i_بنیاد_Cur,
                                 a_مقصد_Open, &iNotUsed1, &iNotUsed2);
      اگر( addrOnce ){
        sqlite3_Vdbe_Jump_یہاں_Or_نکالو_Inst(v, addrOnce);
      }
    }

    /* Top of the update loop */
    اگر( eOnePass!=ONEPASS_OFF ){
      اگر( aiCurOnePass[0]!=i_ڈیٹا_Cur
       && aiCurOnePass[1]!=i_ڈیٹا_Cur
#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
       && !isView
#اگر_ختم
      ){
        أكد( pPk );
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_ملا, i_ڈیٹا_Cur, labelBreak, reg_کلید,n_کلید);
        VdbeCoverage(v);
      }
      اگر( eOnePass!=ONEPASS_SINGLE ){
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, pPk ? reg_کلید : regOldRowid, labelBreak);
      VdbeCoverageIf(v, pPk==0);
      VdbeCoverageIf(v, pPk!=0);
    }ورنہ اگر( pPk || nChangeFrom ){
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      addr_چوٹی = sqlite3VdbeCurrentAddr(v);
      اگر( nChangeFrom ){
        اگر( !isView ){
          اگر( pPk ){
            برائے(i=0; i<nPk; i++){
              sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iEph, i, iPk+i);
            }
            sqlite3_Vdbe_جوڑو_Op4_Int(
                v, OP_Not_ملا, i_ڈیٹا_Cur, labelContinue, iPk, nPk
            ); VdbeCoverage(v);
          }ورنہ{
            sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iEph, regOldRowid);
            sqlite3_Vdbe_جوڑو_Op3(
                v, OP_NotExists, i_ڈیٹا_Cur, labelContinue, regOldRowid
            ); VdbeCoverage(v);
          }
        }
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_صف_ڈیٹا, iEph, reg_کلید);
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_ملا, i_ڈیٹا_Cur, labelContinue, reg_کلید,0);
        VdbeCoverage(v);
      }
    }ورنہ{
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iEph, labelBreak); VdbeCoverage(v);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addr_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iEph, regOldRowid);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, i_ڈیٹا_Cur, labelContinue, regOldRowid);
      VdbeCoverage(v);
    }
  }

  /* If the rowid value will change, set register regNewRowid to
  ** contain the new value. If the rowid is not being modified,
  ** then regNewRowid is the same register as regOldRowid, which is
  ** already populated.  */
  أكد( chng_کلید || pTrigger || hasFK || regOldRowid==regNewRowid );
  اگر( chngRowid ){
    أكد( i_Rowid_اظہاریہ>=0 );
    اگر( nChangeFrom==0 ){
      sqlite3_اظہاریہ_رمز(pParse, p_Rowid_اظہاریہ, regNewRowid);
    }ورنہ{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iEph, i_Rowid_اظہاریہ, regNewRowid);
    }
    sqlite3_Vdbe_جوڑو_Op1(v, OP_MustBeInt, regNewRowid); VdbeCoverage(v);
    sqlite3MayAbort(pParse);
  }

  /* Compute the old pre-UPDATE content of the row being changed, if that
  ** information is needed */
  اگر( chngPk || hasFK || pTrigger ){
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse,
        pTrigger, pChanges, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onError
    );
    برائے(i=0; i<pTab->n_ستون; i++){
      u32 ستون_Flags = pTab->a_ستون[i].ستون_Flags;
      k = sqlite3_جدول_عمود_مقصد_Storage(pTab, i) + regOld;
      اگر( oldmask==0xffffffff
       || (i<32 && (oldmask & MASKBIT32(i))!=0)
       || (ستون_Flags & COLFLAG_PRIMKEY)!=0
      ){
        testcase(  oldmask!=0xffffffff && i==31 );
        sqlite3_اظہاریہ_رمز_Get_عمود_کا_جدول(v, pTab, i_ڈیٹا_Cur, i, k);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, k);
      }
    }
    اگر( chngRowid==0 && pPk==0 ){
#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
      اگر( isView ) sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, regOldRowid);
#اگر_ختم
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
  برائے(i=0, k=regNew; i<pTab->n_ستون; i++, k++){
    اگر( i==pTab->iPKey ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, k);
    }ورنہ اگر( (pTab->a_ستون[i].ستون_Flags & COLFLAG_GENERATED)!=0 ){
      اگر( pTab->a_ستون[i].ستون_Flags & COLFLAG_VIRTUAL ) k--;
    }ورنہ{
      j = aXRef[i];
      اگر( j>=0 ){
        اگر( nChangeFrom ){
          صحیح nOff = (isView ? pTab->n_ستون : nPk);
          أكد( eOnePass==ONEPASS_OFF );
          sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iEph, nOff+j, k);
        }ورنہ{
          sqlite3_اظہاریہ_رمز(pParse, pChanges->a[j].p_اظہاریہ, k);
        }
      }ورنہ اگر( 0==(tmask&TRIGGER_BEFORE) || i>31 || (newmask & MASKBIT32(i)) ){
        /* This branch loads the value of a column that will not be changed
        ** into a register. This is done if there are no BEFORE triggers, or
        ** if there are one or more BEFORE triggers that use this value via
        ** a new.* reference in a trigger program.
        */
        testcase( i==31 );
        testcase( i==32 );
        sqlite3_اظہاریہ_رمز_Get_عمود_کا_جدول(v, pTab, i_ڈیٹا_Cur, i, k);
        b_اختتام_Seek = 0;
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, k);
      }
    }
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
  اگر( pTab->tabFlags & TF_HasGenerated ){
    testcase( pTab->tabFlags & TF_HasVirtual );
    testcase( pTab->tabFlags & TF_HasStored );
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }
#اگر_ختم

  /* Fire any BEFORE UPDATE triggers. This happens before constraints are
  ** verified. One could argue that this is wrong.
  */
  اگر( tmask&TRIGGER_BEFORE ){
    sqlite3_جدول_Affinity(v, pTab, regNew);
    sqlite3_رمز_صف_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_BEFORE, pTab, regOldRowid, onError, labelContinue);

    اگر( !isView ){
      /* The row-trigger may have deleted the row being updated. In this
      ** case, jump to the next row. No updates or AFTER triggers are
      ** required. This behavior - what happens when the row being updated
      ** is deleted or renamed by a BEFORE trigger - is left undefined in the
      ** documentation.
      */
      اگر( pPk ){
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_ملا,i_ڈیٹا_Cur,labelContinue,reg_کلید,n_کلید);
        VdbeCoverage(v);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, i_ڈیٹا_Cur, labelContinue,regOldRowid);
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
      برائے(i=0, k=regNew; i<pTab->n_ستون; i++, k++){
        اگر( pTab->a_ستون[i].ستون_Flags & COLFLAG_GENERATED ){
          اگر( pTab->a_ستون[i].ستون_Flags & COLFLAG_VIRTUAL ) k--;
        }ورنہ اگر( aXRef[i]<0 && i!=pTab->iPKey ){
          sqlite3_اظہاریہ_رمز_Get_عمود_کا_جدول(v, pTab, i_ڈیٹا_Cur, i, k);
        }
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
      اگر( pTab->tabFlags & TF_HasGenerated ){
        testcase( pTab->tabFlags & TF_HasVirtual );
        testcase( pTab->tabFlags & TF_HasStored );
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
#اگر_ختم
    }
  }

  اگر( !isView ){
    /* Do constraint checks. */
    أكد( regOldRowid>0 );
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_ڈیٹا_Cur, iIdxCur,
        regNewRowid, regOldRowid, chng_کلید, onError, labelContinue, &bReplace,
        aXRef, 0);

    /* If REPLACE conflict handling may have been used, or if the PK of the
    ** row is changing, then the GenerateConstraintChecks() above may have
    ** moved cursor iDataCur. Reseek it. */
    اگر( bReplace || chng_کلید ){
      اگر( pPk ){
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_ملا,i_ڈیٹا_Cur,labelContinue,reg_کلید,n_کلید);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, i_ڈیٹا_Cur, labelContinue,regOldRowid);
      }
      VdbeCoverage(v);
    }

    /* Do FK constraint checks. */
    اگر( hasFK ){
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chng_کلید);
    }

    /* Delete the index entries associated with the current record.  */
    sqlite3_Generate_صف_اشاریہ_Delete(pParse, pTab, i_ڈیٹا_Cur, iIdxCur, aRegIdx, -1);

    /* We must run the OP_FinishSeek opcode to resolve a prior
    ** OP_DeferredSeek if there is any possibility that there have been
    ** no OP_Column opcodes since the OP_DeferredSeek was issued.  But
    ** we want to avoid the OP_FinishSeek if possible, as running it
    ** costs CPU cycles. */
    اگر( b_اختتام_Seek ){
      sqlite3_Vdbe_جوڑو_Op1(v, OP_اختتام_Seek, i_ڈیٹا_Cur);
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
#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Delete, i_ڈیٹا_Cur,
        OPFLAG_ISUPDATE | ((hasFK>1 || chng_کلید) ? 0 : OPFLAG_ISNOOP),
        regNewRowid
    );
    اگر( eOnePass==ONEPASS_MULTI ){
      أكد( hasFK==0 && chng_کلید==0 );
      sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);
    }
    اگر( !pParse->nested ){
      sqlite3VdbeAppendP4(v, pTab, P4_جدول);
    }
#ورنہ
    اگر( hasFK>1 || chng_کلید ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Delete, i_ڈیٹا_Cur, 0);
    }
#اگر_ختم

    اگر( hasFK ){
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chng_کلید);
    }

    /* Insert the new index entries and the new record. */
    sqlite3CompleteInsertion(
        pParse, pTab, i_ڈیٹا_Cur, iIdxCur, regNewRowid, aRegIdx,
        OPFLAG_ISUPDATE | (eOnePass==ONEPASS_MULTI ? OPFLAG_SAVEPOSITION : 0),
        0, 0
    );

    /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
    ** handle rows (possibly in other tables) that refer via a foreign key
    ** to the row just updated. */
    اگر( hasFK ){
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chng_کلید);
    }
  }

  /* Increment the row counter
  */
  اگر( reg_صف_گنتی ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, reg_صف_گنتی, 1);
  }

  اگر( pTrigger ){
    sqlite3_رمز_صف_Trigger(pParse, pTrigger, TK_UPDATE, pChanges,
        TRIGGER_AFTER, pTab, regOldRowid, onError, labelContinue);
  }

  /* Repeat the above with the next record to be updated, until
  ** all record selected by the WHERE clause have been updated.
  */
  اگر( eOnePass==ONEPASS_SINGLE ){
    /* Nothing to do at end-of-loop for a single-pass */
  }ورنہ اگر( eOnePass==ONEPASS_MULTI ){
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Where_انتہا(pWInfo);
  }ورنہ{
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iEph, addr_چوٹی); VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  اگر( pParse->nested==0 && pParse->pTriggerTab==0 && pUpsert==0 ){
    sqlite3_Autoincrement_انتہا(pParse);
  }

  /*
  ** Return the number of rows that were changed, if we are tracking
  ** that information.
  */
  اگر( reg_صف_گنتی ){
    sqlite3_رمز_Change_گنتی(v, reg_صف_گنتی, "rows updated");
  }

update_cleanup:
  sqlite3_Auth_Context_نکالو(&sContext);
  sqlite3DbFree(db, aXRef); /* Also frees aRegIdx[] and aToOpen[] */
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_اظہاریہ_List_Delete(db, pChanges);
  sqlite3_اظہاریہ_Delete(db, pWhere);
#اگر defined(SQLITE_ENABLE_UPDATE_DELETE_حد_بندی)
  sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
  sqlite3_اظہاریہ_Delete(db, p_حد_بندی);
#اگر_ختم
  واپس;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#اگر_تعریف_شدہ isView
 #تعریف_منسوخ isView
#اگر_ختم
#اگر_تعریف_شدہ pTrigger
 #تعریف_منسوخ pTrigger
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
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
ساکن خالی update_Virtual_جدول(
  Parse *pParse,       /* The parsing context */
  SrcList *pSrc,       /* The virtual table to be modified */
  جدول *pTab,         /* The virtual table */
  اظہاریہ_List *pChanges,  /* The columns to change in the UPDATE statement */
  اظہاریہ *pRowid,        /* Expression used to recompute the rowid */
  صحیح *aXRef,          /* Mapping from columns of pTab to entries in pChanges */
  اظہاریہ *pWhere,        /* WHERE clause of the UPDATE statement */
  صحیح onError          /* ON CONFLICT strategy */
){
  Vdbe *v = pParse->pVdbe;  /* Virtual machine under construction */
  صحیح ephemTab;             /* Table holding the result of the SELECT */
  صحیح i;                    /* Loop counter */
  sqlite3 *db = pParse->db; /* Database connection */
  ثابت حرف *pVTab = (ثابت حرف*)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  صحیح nArg = 2 + pTab->n_ستون;      /* Number of arguments to VUpdate */
  صحیح regArg;                     /* First register in VUpdate arg array */
  صحیح regRec;                     /* Register in which to assemble record */
  صحیح regRowid;                   /* Register for ephemeral table rowid */
  صحیح iCsr = pSrc->a[0].iCursor;  /* Cursor used for virtual table scan */
  صحیح aDummy[2];                  /* Unused arg for sqlite3WhereOkOnePass() */
  صحیح eOnePass;                   /* True to use onepass strategy */
  صحیح addr;                       /* Address of OP_OpenEphemeral */

  /* Allocate nArg registers in which to gather the arguments for VUpdate. Then
  ** create and open the ephemeral table in which the records created from
  ** these arguments will be temporarily stored. */
  أكد( v );
  ephemTab = pParse->nTab++;
  addr= sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  اگر( pSrc->nSrc>1 ){
    اشاریہ *pPk = 0;
    اظہاریہ *p_صف;
    اظہاریہ_List *pList;
    اگر( HasRowid(pTab) ){
      اگر( pRowid ){
        p_صف = sqlite3_اظہاریہ_Dup(db, pRowid, 0);
      }ورنہ{
        p_صف = sqlite3PExpr(pParse, TK_صف, 0, 0);
      }
    }ورنہ{
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      أكد( pPk!=0 );
      أكد( pPk->n_کلید_ستون==1 );
      iPk = pPk->ai_عمود[0];
      اگر( aXRef[iPk]>=0 ){
        p_صف = sqlite3_اظہاریہ_Dup(db, pChanges->a[aXRef[iPk]].p_اظہاریہ, 0);
      }ورنہ{
        p_صف = اظہاریہ_صف_عمود(pParse, iPk);
      }
    }
    pList = sqlite3_اظہاریہ_List_Append(pParse, 0, p_صف);

    برائے(i=0; i<pTab->n_ستون; i++){
      اگر( aXRef[i]>=0 ){
        pList = sqlite3_اظہاریہ_List_Append(pParse, pList,
          sqlite3_اظہاریہ_Dup(db, pChanges->a[aXRef[i]].p_اظہاریہ, 0)
        );
      }ورنہ{
        اظہاریہ *p_صف_اظہاریہ = اظہاریہ_صف_عمود(pParse, i);
        اگر( p_صف_اظہاریہ ) p_صف_اظہاریہ->op2 = OPFLAG_NOCHNG;
        pList = sqlite3_اظہاریہ_List_Append(pParse, pList, p_صف_اظہاریہ);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3_اظہاریہ_List_Delete(db, pList);
    eOnePass = ONEPASS_OFF;
  }ورنہ{
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    /* Start scanning the virtual table */
    pWInfo = sqlite3WhereBegin(
        pParse, pSrc, pWhere, 0, 0, 0, WHERE_ONEPASS_DESIRED, 0
    );
    اگر( pWInfo==0 ) واپس;

    /* Populate the argument registers. */
    برائے(i=0; i<pTab->n_ستون; i++){
      أكد( (pTab->a_ستون[i].ستون_Flags & COLFLAG_GENERATED)==0 );
      اگر( aXRef[i]>=0 ){
        sqlite3_اظہاریہ_رمز(pParse, pChanges->a[aXRef[i]].p_اظہاریہ, regArg+2+i);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op3(v, OP_VColumn, iCsr, i, regArg+2+i);
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG);/* For sqlite3_vtab_nochange() */
      }
    }
    اگر( HasRowid(pTab) ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iCsr, regArg);
      اگر( pRowid ){
        sqlite3_اظہاریہ_رمز(pParse, pRowid, regArg+1);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iCsr, regArg+1);
      }
    }ورنہ{
      اشاریہ *pPk;   /* PRIMARY KEY index */
      i16 iPk;      /* PRIMARY KEY column */
      pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      أكد( pPk!=0 );
      أكد( pPk->n_کلید_ستون==1 );
      iPk = pPk->ai_عمود[0];
      sqlite3_Vdbe_جوڑو_Op3(v, OP_VColumn, iCsr, iPk, regArg);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_SCopy, regArg+2+iPk, regArg+1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    /* There is no ONEPASS_MULTI on virtual tables */
    أكد( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );

    اگر( eOnePass ){
      /* If using the onepass strategy, no-op out the OP_OpenEphemeral coded
      ** above. */
      sqlite3_Vdbe_Change_مقصد_Noop(v, addr);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Close, iCsr);
    }ورنہ{
      /* Create a record from the argument register contents and insert it into
      ** the ephemeral table. */
      sqlite3MultiWrite(pParse);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, regArg, nArg, regRec);
#اگر defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_NULL_TRIM)
      /* Signal an assert() within OP_MakeRecord that it is allowed to
      ** accept no-change records with serial_type 10 */
      sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
#اگر_ختم
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, ephemTab, regRowid);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, ephemTab, regRec, regRowid);
    }
  }


  اگر( eOnePass==ONEPASS_OFF ){
    /* End the virtual table scan */
    اگر( pSrc->nSrc==1 ){
      sqlite3_Where_انتہا(pWInfo);
    }

    /* Begin scanning through the ephemeral table. */
    addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, ephemTab); VdbeCoverage(v);

    /* Extract arguments from the current row of the ephemeral table and
    ** invoke the VUpdate method.  */
    برائے(i=0; i<nArg; i++){
      sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, ephemTab, i, regArg+i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3_Vdbe_جوڑو_Op4(v, OP_VUpdate, 0, nArg, regArg, pVTab, P4_VTAB);
  sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
  sqlite3MayAbort(pParse);

  /* End of the ephemeral table scan. Or, if using the onepass strategy,
  ** jump to here if the scan visited zero rows. */
  اگر( eOnePass==ONEPASS_OFF ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, ephemTab, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_یہاں(v, addr);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Close, ephemTab, 0);
  }ورنہ{
    sqlite3_Where_انتہا(pWInfo);
  }
}
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */
