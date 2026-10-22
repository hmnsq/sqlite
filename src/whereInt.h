/*
** 2013-11-12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains structure and macro definitions for the query
** planner logic in "where.c".  These definitions are broken out into
** a separate source file for easier editing.
*/
#اگر_تعریف_نشده SQLITE_WHEREINT_H
#تعریف_کن SQLITE_WHEREINT_H


/* Forward references
*/
تعريفنوع بنية WhereClause WhereClause;
تعريفنوع بنية Where_ماسک_مجموعه Where_ماسک_مجموعه;
تعريفنوع بنية WhereOrInfo WhereOrInfo;
تعريفنوع بنية WhereAndInfo WhereAndInfo;
تعريفنوع بنية WhereLevel WhereLevel;
تعريفنوع بنية WhereLoop WhereLoop;
تعريفنوع بنية Where_مسیر Where_مسیر;
تعريفنوع بنية Where_جمله Where_جمله;
تعريفنوع بنية WhereLoopBuilder WhereLoopBuilder;
تعريفنوع بنية Where_پیمایش Where_پیمایش;
تعريفنوع بنية Where_Or_هزینه Where_Or_هزینه;
تعريفنوع بنية Where_Or_مجموعه Where_Or_مجموعه;
تعريفنوع بنية Where_Mem_بلوک Where_Mem_بلوک;
تعريفنوع بنية Where_راست_Join Where_راست_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
بنية Where_Mem_بلوک {
  Where_Mem_بلوک *p_بعدی;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
بنية Where_راست_Join {
  صحيح i_تطابق;          /* Cursor used to determine prior matched rows */
  صحيح regBloom;        /* Bloom filter for iRJMatch */
  صحيح regReturn;       /* Return register for the interior subroutine */
  صحيح addrSubrtn;      /* Starting address for the interior subroutine */
  صحيح انتها_Subrtn;       /* The last opcode in the interior subroutine */
};

/*
** This object contains information needed to implement a single nested
** loop in WHERE clause.
**
** Contrast this object with WhereLoop.  This object describes the
** implementation of the loop.  WhereLoop describes the algorithm.
** This object contains a pointer to the WhereLoop algorithm as one of
** its elements.
**
** The WhereInfo object contains a single instance of this object for
** each term in the FROM clause (which is to say, for each of the
** nested loops as implemented).  The order of WhereLevel objects determines
** the loop nested order, with WhereInfo.a[0] being the outer loop and
** WhereInfo.a[WhereInfo.nLevel-1] being the inner loop.
*/
بنية WhereLevel {
  صحيح i_چپ_Join;        /* Memory cell used to implement LEFT OUTER JOIN */
  صحيح iTabCur;          /* The VDBE cursor used to access the table */
  صحيح iIdxCur;          /* The VDBE cursor used to access pIdx */
  صحيح addrBrk;          /* Jump here to break out of the loop */
  صحيح addrHalt;         /* Abort the query due to empty table or similar */
  صحيح addrNxt;          /* Jump here to start the next IN combination */
  صحيح addrSkip;         /* Jump here for next iteration of skip-scan */
  صحيح addrCont;         /* Jump here to continue with the next loop cycle */
  صحيح addrDeleteCont;   /* Delete auto-index entry then same as addrCont */
  صحيح addr_اولی;        /* First instruction of interior of the loop */
  صحيح addrBody;         /* Beginning of the body of this loop */
  صحيح regBignull;       /* big-null flag reg. True if a NULL-scan is needed */
  صحيح addrBignull;      /* Jump here for next part of big-null scan */
#اگر_تعریف_نشده SQLITE_LIKE_DOESNT_تطابق_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  صحيح addrLikeRep;      /* LIKE range processing address */
#پایان_اگر
  صحيح regFilter;        /* Bloom filter */
  Where_راست_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  صحيح p1, p2;           /* Operands of the opcode used to end the loop */
  اتحاد {               /* Information that depends on pWLoop->wsFlags */
    بنية {
      صحيح n_درون;              /* Number of entries in aInLoop[] */
      بنية درون_Loop {
        صحيح iCur;              /* The VDBE cursor used by this IN operator */
        صحيح addr_درون_بالاترین;         /* Top of the IN loop */
        صحيح i_پایه;             /* Base register of multi-key index record */
        صحيح n_پیشوند;           /* Number of prior entries in the key */
        u8 e_انتها_Loop_Op;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *a_درون_Loop;           /* Information about each nested IN operator */
    } درون;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    شاخص *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  بنية WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#اگر_تعریف_شده SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addr_بازدید;        /* Address at which row is visited */
#پایان_اگر
};

/*
** Each instance of this object represents an algorithm for evaluating one
** term of a join.  Every term of the FROM clause will have at least
** one corresponding WhereLoop object (unless INDEXED BY constraints
** prevent a query solution - which is an error) and many terms of the
** FROM clause will have multiple WhereLoop objects, each describing a
** potential way of implementing that FROM-clause term, together with
** dependencies and cost estimates for using the chosen algorithm.
**
** Query planning consists of building up a collection of these WhereLoop
** objects, then computing a particular sequence of WhereLoop objects, with
** one WhereLoop object per FROM clause term, that satisfy all dependencies
** and that minimize the overall cost.
*/
بنية WhereLoop {
  Bitmask prereq;       /* Bitmask of other loops that must run first */
  Bitmask ماسک_Self;     /* Bitmask identifying table iTab */
#اگر_تعریف_شده SQLITE_DEBUG
  حرف cId;             /* Symbolic ID of this loop for debugging use */
#پایان_اگر
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 i_مرتب_سازی_Idx;          /* Sorting index number.  0==None */
  LogEst r_راهاندازی;        /* One-time setup cost (ex: create transient index) */
  LogEst r_پیاپی;          /* Cost of running each loop */
  LogEst n_بیرون;          /* Estimated number of output rows */
  اتحاد {
    بنية {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_بالاترین;              /* Size of TOP vector */
      u16 n_Distinct_ستون;      /* Index columns used to sort for DISTINCT */
      شاخص *p_شاخص;         /* Index used, or NULL */
      عبارت_List *p_ترتیب_By;    /* ORDER BY clause if this is really a subquery */
    } btree;
    بنية {               /* Information for virtual tables */
      صحيح idx_رقم;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 b_Omit_جابه_جایی : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_رقم_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omit_ماسک;          /* Terms that may be omitted */
      حرف *idxStr;          /* Index identifier string */
      u32 m_Handle_درون;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#اگر_تعریف_شده WHERETRACE_ENABLED
  LogEst r_ستاره_Delta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#پایان_اگر
  Where_جمله **aLTerm;   /* WhereTerms used */
  WhereLoop *p_بعدی_Loop; /* Next WhereLoop object in the WhereClause */
  Where_جمله *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
بنية Where_Or_هزینه {
  Bitmask prereq;     /* Prerequisites */
  LogEst r_پیاپی;        /* Cost of running this subquery */
  LogEst n_بیرون;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#تعریف_کن N_OR_هزینه 3
بنية Where_Or_مجموعه {
  u16 n;                      /* Number of valid a[] entries */
  Where_Or_هزینه a[N_OR_هزینه];   /* Set of best costs */
};

/*
** Each instance of this object holds a sequence of WhereLoop objects
** that implement some or all of a query plan.
**
** Think of each WhereLoop object as a node in a graph with arcs
** showing dependencies and costs for travelling between nodes.  (That is
** not a completely accurate description because WhereLoop costs are a
** vector, not a scalar, and because dependencies are many-to-one, not
** one-to-one as are graph nodes.  But it is a useful visualization aid.)
** Then a WherePath object is a path through the graph that visits some
** or all of the WhereLoop objects once.
**
** The "solver" works by creating the N best WherePath objects of length
** 1.  Then using those as a basis to compute the N best WherePath objects
** of length 2.  And so forth until the length of WherePaths equals the
** number of nodes in the FROM clause.  The best (lowest cost) WherePath
** at the end is the chosen query plan.
*/
بنية Where_مسیر {
  Bitmask ماسک_Loop;     /* Bitmask of all WhereLoop objects in this path */
  Bitmask revLoop;      /* aLoop[]s that should be reversed for ORDER BY */
  LogEst n_ردیف;          /* Estimated number of rows generated by this path */
  LogEst r_هزینه;         /* Total cost of this path */
  LogEst rUnsort;       /* Total cost of this path ignoring sorting costs */
  i8 isOrdered;         /* No. of ORDER BY terms satisfied. -1 for unknown */
  WhereLoop **aLoop;    /* Array of WhereLoop objects implementing this path */
};

/*
** The query generator uses an array of instances of this structure to
** help it analyze the subexpressions of the WHERE clause.  Each WHERE
** clause subexpression is separated from the others by AND operators,
** usually, or sometimes subexpressions separated by OR.
**
** All WhereTerms are collected into a single WhereClause structure. 
** The following identity holds:
**
**        WhereTerm.pWC->a[WhereTerm.idx] == WhereTerm
**
** When a term is of the form:
**
**              X <op> <expr>
**
** where X is a column name and <op> is one of certain operators,
** then WhereTerm.leftCursor and WhereTerm.u.leftColumn record the
** cursor number and column number for X.  WhereTerm.eOperator records
** the <op> using a bitmask encoding defined by WO_xxx below.  The
** use of a bitmask encoding for the operator allows us to search
** quickly for terms that match any of several different operators.
**
** A WhereTerm might also be two or more subterms connected by OR:
**
**         (t1.X <op> <expr>) OR (t1.Y <op> <expr>) OR ....
**
** In this second case, wtFlag has the TERM_ORINFO bit set and eOperator==WO_OR
** and the WhereTerm.u.pOrInfo field points to auxiliary information that
** is collected about the OR clause.
**
** If a term in the WHERE clause does not match either of the two previous
** categories, then eOperator==0.  The WhereTerm.pExpr field is still set
** to the original subexpression content and wtFlags is set up appropriately
** but no other fields in the WhereTerm object are meaningful.
**
** When eOperator!=0, prereqRight and prereqAll record sets of cursor numbers,
** but they do so indirectly.  A single WhereMaskSet structure translates
** cursor number into bits and the translated bit is stored in the prereq
** fields.  The translation is used in order to maximize the number of
** bits that will fit in a Bitmask.  The VDBE cursor numbers might be
** spread out over the non-negative integers.  For example, the cursor
** numbers might be 3, 8, 9, 10, 20, 23, 41, and 45.  The WhereMaskSet
** translates these sparse cursor numbers into consecutive integers
** beginning with 0 in order to make the best possible use of the available
** bits in the Bitmask.  So, in the example above, the cursor numbers
** would be mapped into integers 0 through 7.
**
** The number of terms in a join is limited by the number of bits
** in prereqRight and prereqAll.  The default is 64 bits, hence SQLite
** is only able to process joins with 64 or fewer tables.
*/
بنية Where_جمله {
  عبارت *p_عبارت;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 n_فرزند;              /* Number of children that must disable us */
  u8 e_تطابق_Op;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  صحيح i_والد;            /* Disable pWC->a[iParent] when this term disabled */
  صحيح چپ_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#اگر_تعریف_شده SQLITE_DEBUG
  صحيح i_جمله;              /* Which WhereTerm is this, for debug purposes */
#پایان_اگر
  اتحاد {
    بنية {
      صحيح چپ_ستونی;         /* Column number of X in "X <op> <expr>" */
      صحيح i_فیلد;             /* Field in (?,?,?) IN (SELECT...) vector */
    } x;                    /* Opcode other than OP_OR or OP_AND */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  Bitmask prereq_راست;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#تعریف_کن جمله_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#تعریف_کن جمله_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#تعریف_کن جمله_CODED      0x0004 /* This term is already coded */
#تعریف_کن جمله_COPIED     0x0008 /* Has a child */
#تعریف_کن جمله_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#تعریف_کن جمله_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#تعریف_کن جمله_OK         0x0040 /* Used during OR-clause processing */
#تعریف_کن جمله_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#تعریف_کن جمله_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#تعریف_کن جمله_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#تعریف_کن جمله_LIKE       0x0400 /* The original LIKE operator */
#تعریف_کن جمله_IS         0x0800 /* Term.pExpr is an IS operator */
#تعریف_کن جمله_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#تعریف_کن جمله_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#اگر_تعریف_شده SQLITE_ENABLE_STAT4
#  define جمله_HIGHTRUTH  0x4000 /* Term excludes few rows */
#وإلا
#  define جمله_HIGHTRUTH  0      /* Only used with STAT4 */
#پایان_اگر
#تعریف_کن جمله_برش      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
بنية Where_پیمایش {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  ثابت حرف *zCollName;     /* Required collating sequence, if not NULL */
  عبارت *p_Idx_عبارت;            /* Search for this index expression */
  صحيح k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 op_ماسک;                /* Acceptable operators */
  حرف idxaff;               /* Must match this affinity, if zCollName!=NULL */
  بیعلامت حرف iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  بیعلامت حرف nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
  صحيح aiCur[11];             /* Cursors in the equivalence class */
  i16 ai_ستونی[11];          /* Corresponding column number in the eq-class */
};

/*
** An instance of the following structure holds all information about a
** WHERE clause.  Mostly this is a container for one or more WhereTerms.
**
** Explanation of pOuter:  For a WHERE clause of the form
**
**           a AND ((b AND c) OR (d AND e)) AND f
**
** There are separate WhereClause objects for the whole clause and for
** the subclauses "(b AND c)" and "(d AND e)".  The pOuter field of the
** subclauses points to the WhereClause object for the whole clause.
*/
بنية WhereClause {
  WhereInfo *pWInfo;       /* WHERE clause processing context */
  WhereClause *pOuter;     /* Outer conjunction */
  u8 op;                   /* Split operator.  TK_AND or TK_OR */
  u8 hasOr;                /* True if any a[].eOperator is WO_OR */
  صحيح n_جمله;               /* Number of terms */
  صحيح n_جایگاه;               /* Number of entries in a[] */
  صحيح n_پایه;               /* Number of terms through the last non-Virtual */
  Where_جمله *a;            /* Each a[] describes a term of the WHERE clause */
#إذا defined(SQLITE_SMALL_پشته)
  Where_جمله aStatic[1];    /* Initial static space for a[] */
#وإلا
  Where_جمله aStatic[8];    /* Initial static space for a[] */
#پایان_اگر
};

/*
** A WhereTerm with eOperator==WO_OR has its u.pOrInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
بنية WhereOrInfo {
  WhereClause wc;          /* Decomposition into subterms */
  Bitmask indexable;       /* Bitmask of all indexable tables in the clause */
};

/*
** A WhereTerm with eOperator==WO_AND has its u.pAndInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
بنية WhereAndInfo {
  WhereClause wc;          /* The subexpression broken out */
};

/*
** An instance of the following structure keeps track of a mapping
** between VDBE cursor numbers and bits of the bitmasks in WhereTerm.
**
** The VDBE cursor numbers are small integers contained in
** SrcItem.iCursor and Expr.iTable fields.  For any given WHERE
** clause, the cursor numbers might not begin with 0 and they might
** contain gaps in the numbering sequence.  But we want to make maximum
** use of the bits in our bitmasks.  This structure provides a mapping
** from the sparse cursor numbers into consecutive integers beginning
** with 0.
**
** If WhereMaskSet.ix[A]==B it means that The A-th bit of a Bitmask
** corresponds VDBE cursor number B.  The A-th bit of a bitmask is 1<<A.
**
** For example, if the WHERE clause expression used these VDBE
** cursors:  4, 5, 8, 29, 57, 73.  Then the  WhereMaskSet structure
** would map those cursor numbers into bits 0 through 5.
**
** Note that the mapping is not necessarily ordered.  In the example
** above, the mapping might go like this:  4->3, 5->1, 8->2, 29->0,
** 57->5, 73->4.  Or one of 719 other combinations might be used. It
** does not really matter.  What is important is that sparse cursor
** numbers all get mapped into bit numbers that begin with 0 and contain
** no gaps.
*/
بنية Where_ماسک_مجموعه {
  صحيح bVarSelect;               /* Used by sqlite3WhereExprUsage() */
  صحيح n;                        /* Number of assigned cursor values */
  صحيح ix[BMS];                  /* Cursor assigned to each bit */
};

/*
** This object is a convenience wrapper holding all information needed
** to construct WhereLoop objects for a particular query.
*/
بنية WhereLoopBuilder {
  WhereInfo *pWInfo;        /* Information about this WHERE */
  WhereClause *pWC;         /* WHERE clause terms */
  WhereLoop *pNew;          /* Template WhereLoop */
  Where_Or_مجموعه *p_Or_مجموعه;       /* Record best loops here, if not NULL */
#اگر_تعریف_شده SQLITE_ENABLE_STAT4
  Unpacked_رکورد *pRec;     /* Probe for stat4 (if required) */
  صحيح nRecValid;            /* Number of valid fields currently in pRec */
#پایان_اگر
  بیعلامت حرف bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  بیعلامت حرف bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  بیعلامت صحيح i_Plan_حد;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#تعریف_کن SQLITE_BLDF1_نمایهای  0x0001   /* An index is used */
#تعریف_کن SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#تعریف_کن SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

/* The WhereLoopBuilder.iPlanLimit is used to limit the number of
** index+constraint combinations the query planner will consider for a
** particular query.  If this parameter is unlimited, then certain
** pathological queries can spend excess time in the sqlite3WhereBegin()
** routine.  The limit is high enough that is should not impact real-world
** queries.
**
** SQLITE_QUERY_PLANNER_LIMIT is the baseline limit.  The limit is
** increased by SQLITE_QUERY_PLANNER_LIMIT_INCR before each term of the FROM
** clause is processed, so that every table in a join is guaranteed to be
** able to propose a some index+constraint combinations even if the initial
** baseline limit was exhausted by prior tables of the join.
*/
#اگر_تعریف_نشده SQLITE_QUERY_PLANNER_حد
# define SQLITE_QUERY_PLANNER_حد 20000
#پایان_اگر
#اگر_تعریف_نشده SQLITE_QUERY_PLANNER_حد_INCR
# define SQLITE_QUERY_PLANNER_حد_INCR 1000
#پایان_اگر

/*
** The WHERE clause processing routine has two halves.  The
** first part does the start of the WHERE loop and the second
** half does the tail of the WHERE loop.  An instance of
** this structure is returned by the first half and passed
** into the second half to give some continuity.
**
** An instance of this object holds the complete state of the query
** planner.
*/
بنية WhereInfo {
  Parse *pParse;            /* Parsing and code generating context */
  SrcList *pTabList;        /* List of tables in the join */
  عبارت_List *p_ترتیب_By;       /* The ORDER BY clause or NULL */
  عبارت_List *p_نتیجه_مجموعه;     /* Result set of the query */
#إذا WHERETRACE_ENABLED
  عبارت *pWhere;             /* The complete WHERE clause */
#پایان_اگر
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  صحيح aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  صحيح iContinue;            /* Jump here to continue with next record */
  صحيح iBreak;               /* Jump here to break out of the loop */
  صحيح savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst i_حد;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  بیعلامت bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  بیعلامت untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  بیعلامت bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  بیعلامت sorted        :1;   /* True if really sorted (not just grouped) */
  بیعلامت b_ستاره_انجامشده     :1;   /* True if check for star-query is complete */
  بیعلامت b_ستاره_استفاده_شده     :1;   /* True if star-query heuristic is used */
  LogEst n_ردیف_بیرون;           /* Estimated number of output rows */
#اگر_تعریف_شده WHERETRACE_ENABLED
  LogEst r_مجموع_هزینه;        /* Total cost of the solution */
#پایان_اگر
  صحيح i_بالاترین;                 /* The very beginning of the WHERE loop */
  صحيح i_انتها_Where;            /* End of the WHERE clause itself */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  Where_Mem_بلوک *p_Mem_مقصد_Free;/* Memory to free when this object destroyed */
  Bitmask rev_ماسک;          /* Mask of ORDER BY terms that need reversing */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  Where_ماسک_مجموعه s_ماسک_مجموعه;    /* Map cursor numbers to bitmasks */
  WhereLevel a[FLEXARRAY];  /* Information about each nest loop in WHERE */
};

/*
** The size (in bytes) of a WhereInfo object that holds N WhereLevels.
*/
#تعریف_کن SZ_WHEREINFO(N) ROUND8(offsetof(WhereInfo,a)+(N)*حجم(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3_Where_Get_ماسک(Where_ماسک_مجموعه*,صحيح);
#اگر_تعریف_شده WHERETRACE_ENABLED
فراغ sqlite3WhereClausePrint(WhereClause *pWC);
فراغ sqlite3_Where_جمله_Print(Where_جمله *p_جمله, صحيح i_جمله);
فراغ sqlite3WhereLoopPrint(ثابت WhereLoop *p, ثابت WhereClause *pWC);
#پایان_اگر
Where_جمله *sqlite3_Where_پیدا_کن_جمله(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  صحيح iCur,             /* Cursor number of LHS */
  صحيح i_ستونی,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  شاخص *pIdx           /* Must be compatible with this index, if not NULL */
);
فراغ *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
فراغ *sqlite3WhereRealloc(WhereInfo *pWInfo, فراغ *pOld, u64 nByte);

/* wherecode.c: */
#اگر_تعریف_نشده SQLITE_OMIT_EXPLAIN
صحيح sqlite3_Where_Explain_One_پیمایش(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
صحيح sqlite3WhereExplainBloomFilter(
  ثابت Parse *pParse,            /* Parse context */
  ثابت WhereInfo *pWInfo,        /* WHERE clause */
  ثابت WhereLevel *pLevel        /* Bloom filter on this level */
);
فراغ sqlite3_Where_جمع_کن_Explain_متن(
  Parse *pParse,                  /* Parse context */
  صحيح addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#وإلا
# define sqlite3_Where_Explain_One_پیمایش(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3_Where_جمع_کن_Explain_متن(u,v,w,x,y)
#پایان_اگر /* SQLITE_OMIT_EXPLAIN */
#اگر_تعریف_شده SQLITE_ENABLE_STMT_SCANSTATUS
فراغ sqlite3_Where_جمع_کن_پیمایش_وضعیت(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  صحيح addrExplain                 /* Address of OP_Explain (or 0) */
);
#وإلا
# define sqlite3_Where_جمع_کن_پیمایش_وضعیت(a, b, c, d) ((فراغ)d)
#پایان_اگر
Bitmask sqlite3_Where_کد_One_Loop_شروع(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  صحيح iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
);
SQLITE_NOINLINE فراغ sqlite3_Where_راست_Join_Loop(
  WhereInfo *pWInfo,
  صحيح iLevel,
  WhereLevel *pLevel
);

/* whereexpr.c: */
فراغ sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
فراغ sqlite3WhereClauseClear(WhereClause*);
فراغ sqlite3_Where_تقسیم(WhereClause*,عبارت*,u8);
فراغ sqlite3_Where_جمع_کن_حد(WhereClause*, Select*);
Bitmask sqlite3_Where_عبارت_Usage(Where_ماسک_مجموعه*, عبارت*);
Bitmask sqlite3_Where_عبارت_Usage_NN(Where_ماسک_مجموعه*, عبارت*);
Bitmask sqlite3_Where_عبارت_List_Usage(Where_ماسک_مجموعه*, عبارت_List*);
فراغ sqlite3_Where_عبارت_Analyze(SrcList*, WhereClause*);
فراغ sqlite3_Where_Tab_Func_آرگومانها(Parse*, Src_قلم*, WhereClause*);
صحيح sqlite3WhereLoopBloomable(ثابت WhereLoop*);

/*
** Bitmasks for the operators on WhereTerm objects.  These are all
** operators that are of interest to the query planner.  An
** OR-ed combination of these values can be used when searching for
** particular WhereTerms within a WhereClause.
**
** Value constraints:
**     WO_EQ    == SQLITE_INDEX_CONSTRAINT_EQ
**     WO_LT    == SQLITE_INDEX_CONSTRAINT_LT
**     WO_LE    == SQLITE_INDEX_CONSTRAINT_LE
**     WO_GT    == SQLITE_INDEX_CONSTRAINT_GT
**     WO_GE    == SQLITE_INDEX_CONSTRAINT_GE
*/
#تعریف_کن WO_درون     0x0001
#تعریف_کن WO_EQ     0x0002
#تعریف_کن WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#تعریف_کن WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#تعریف_کن WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#تعریف_کن WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#تعریف_کن WO_AUX    0x0040       /* Op useful to virtual tables only */
#تعریف_کن WO_IS     0x0080
#تعریف_کن WO_ISNULL 0x0100
#تعریف_کن WO_OR     0x0200       /* Two or more OR-connected terms */
#تعریف_کن WO_AND    0x0400       /* Two or more AND-connected terms */
#تعریف_کن WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#تعریف_کن WO_NOOP   0x1000       /* This term does not restrict search space */
#تعریف_کن WO_ROWVAL 0x2000       /* A row-value term */

#تعریف_کن WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#تعریف_کن WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#تعریف_کن WHERE_ستونی_EQ    0x00000001  /* x=EXPR */
#تعریف_کن WHERE_ستونی_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#تعریف_کن WHERE_ستونی_درون    0x00000004  /* x IN (...) */
#تعریف_کن WHERE_ستونی_NULL  0x00000008  /* x IS NULL */
#تعریف_کن WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#تعریف_کن WHERE_بالاترین_حد    0x00000010  /* x<EXPR or x<=EXPR constraint */
#تعریف_کن WHERE_BTM_حد    0x00000020  /* x>EXPR or x>=EXPR constraint */
#تعریف_کن WHERE_BOTH_حد   0x00000030  /* Both x>EXPR and x<EXPR */
#تعریف_کن WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#تعریف_کن WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#تعریف_کن WHERE_نمایهای      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#تعریف_کن WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#تعریف_کن WHERE_درون_ABLE      0x00000800  /* Able to support an IN operator */
#تعریف_کن WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#تعریف_کن WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#تعریف_کن WHERE_AUTO_شاخص   0x00004000  /* Uses an ephemeral index */
#تعریف_کن WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#تعریف_کن WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#تعریف_کن WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#تعریف_کن WHERE_درون_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#تعریف_کن WHERE_BIGNULL_مرتب_سازی 0x00080000  /* Column nEq of index is BIGNULL */
#تعریف_کن WHERE_درون_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#تعریف_کن WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#تعریف_کن WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#تعریف_کن WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#تعریف_کن WHERE_OMIT_جابه_جایی  0x01000000  /* Set offset counter to zero */
#تعریف_کن WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#تعریف_کن WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#پایان_اگر /* !defined(SQLITE_WHEREINT_H) */
