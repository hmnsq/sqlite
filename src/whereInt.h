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
#اگر_غیر_تعریف_شدہ SQLITE_WHEREINT_H
#تعریف_کرو SQLITE_WHEREINT_H


/* Forward references
*/
قسمتعریف ساخت WhereClause WhereClause;
قسمتعریف ساخت Where_نقاب_سیٹ Where_نقاب_سیٹ;
قسمتعریف ساخت WhereOrInfo WhereOrInfo;
قسمتعریف ساخت WhereAndInfo WhereAndInfo;
قسمتعریف ساخت WhereLevel WhereLevel;
قسمتعریف ساخت WhereLoop WhereLoop;
قسمتعریف ساخت Where_راستہ Where_راستہ;
قسمتعریف ساخت Where_رکن Where_رکن;
قسمتعریف ساخت WhereLoopBuilder WhereLoopBuilder;
قسمتعریف ساخت Where_جائزہ Where_جائزہ;
قسمتعریف ساخت Where_Or_لاگت Where_Or_لاگت;
قسمتعریف ساخت Where_Or_سیٹ Where_Or_سیٹ;
قسمتعریف ساخت Where_Mem_بلاک Where_Mem_بلاک;
قسمتعریف ساخت Where_دایاں_Join Where_دایاں_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
ساخت Where_Mem_بلاک {
  Where_Mem_بلاک *p_اگلا;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
ساخت Where_دایاں_Join {
  صحیح i_مطابقت;          /* Cursor used to determine prior matched rows */
  صحیح regBloom;        /* Bloom filter for iRJMatch */
  صحیح regReturn;       /* Return register for the interior subroutine */
  صحیح addrSubrtn;      /* Starting address for the interior subroutine */
  صحیح انتہا_Subrtn;       /* The last opcode in the interior subroutine */
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
ساخت WhereLevel {
  صحیح i_بایاں_Join;        /* Memory cell used to implement LEFT OUTER JOIN */
  صحیح iTabCur;          /* The VDBE cursor used to access the table */
  صحیح iIdxCur;          /* The VDBE cursor used to access pIdx */
  صحیح addrBrk;          /* Jump here to break out of the loop */
  صحیح addrHalt;         /* Abort the query due to empty table or similar */
  صحیح addrNxt;          /* Jump here to start the next IN combination */
  صحیح addrSkip;         /* Jump here for next iteration of skip-scan */
  صحیح addrCont;         /* Jump here to continue with the next loop cycle */
  صحیح addrDeleteCont;   /* Delete auto-index entry then same as addrCont */
  صحیح addr_پہلا;        /* First instruction of interior of the loop */
  صحیح addrBody;         /* Beginning of the body of this loop */
  صحیح regBignull;       /* big-null flag reg. True if a NULL-scan is needed */
  صحیح addrBignull;      /* Jump here for next part of big-null scan */
#اگر_غیر_تعریف_شدہ SQLITE_LIKE_DOESNT_مطابقت_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  صحیح addrLikeRep;      /* LIKE range processing address */
#اگر_ختم
  صحیح regFilter;        /* Bloom filter */
  Where_دایاں_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  صحیح p1, p2;           /* Operands of the opcode used to end the loop */
  اتحاد {               /* Information that depends on pWLoop->wsFlags */
    ساخت {
      صحیح n_اندر;              /* Number of entries in aInLoop[] */
      ساخت اندر_Loop {
        صحیح iCur;              /* The VDBE cursor used by this IN operator */
        صحیح addr_اندر_چوٹی;         /* Top of the IN loop */
        صحیح i_بنیاد;             /* Base register of multi-key index record */
        صحیح n_سابقہ;           /* Number of prior entries in the key */
        u8 e_انتہا_Loop_Op;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *a_اندر_Loop;           /* Information about each nested IN operator */
    } اندر;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    اشاریہ *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  ساخت WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  صحیح addr_دورہ;        /* Address at which row is visited */
#اگر_ختم
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
ساخت WhereLoop {
  Bitmask prereq;       /* Bitmask of other loops that must run first */
  Bitmask نقاب_Self;     /* Bitmask identifying table iTab */
#اگر_تعریف_شدہ SQLITE_DEBUG
  حرف cId;             /* Symbolic ID of this loop for debugging use */
#اگر_ختم
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 i_ترتیب_دو_Idx;          /* Sorting index number.  0==None */
  LogEst r_تیاری;        /* One-time setup cost (ex: create transient index) */
  LogEst r_تسلسل;          /* Cost of running each loop */
  LogEst n_برآمد;          /* Estimated number of output rows */
  اتحاد {
    ساخت {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_چوٹی;              /* Size of TOP vector */
      u16 n_Distinct_ستون;      /* Index columns used to sort for DISTINCT */
      اشاریہ *p_اشاریہ;         /* Index used, or NULL */
      اظہاریہ_List *p_ترتیب_By;    /* ORDER BY clause if this is really a subquery */
    } btree;
    ساخت {               /* Information for virtual tables */
      صحیح idx_عدد;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 b_Omit_انحراف : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_عدد_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omit_نقاب;          /* Terms that may be omitted */
      حرف *idxStr;          /* Index identifier string */
      u32 m_Handle_اندر;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#اگر_تعریف_شدہ WHERETRACE_ENABLED
  LogEst r_ستارہ_Delta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#اگر_ختم
  Where_رکن **aLTerm;   /* WhereTerms used */
  WhereLoop *p_اگلا_Loop; /* Next WhereLoop object in the WhereClause */
  Where_رکن *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
ساخت Where_Or_لاگت {
  Bitmask prereq;     /* Prerequisites */
  LogEst r_تسلسل;        /* Cost of running this subquery */
  LogEst n_برآمد;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#تعریف_کرو N_OR_لاگت 3
ساخت Where_Or_سیٹ {
  u16 n;                      /* Number of valid a[] entries */
  Where_Or_لاگت a[N_OR_لاگت];   /* Set of best costs */
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
ساخت Where_راستہ {
  Bitmask نقاب_Loop;     /* Bitmask of all WhereLoop objects in this path */
  Bitmask revLoop;      /* aLoop[]s that should be reversed for ORDER BY */
  LogEst n_صف;          /* Estimated number of rows generated by this path */
  LogEst r_لاگت;         /* Total cost of this path */
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
ساخت Where_رکن {
  اظہاریہ *p_اظہاریہ;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 n_بچہ;              /* Number of children that must disable us */
  u8 e_مطابقت_Op;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  صحیح i_والد;            /* Disable pWC->a[iParent] when this term disabled */
  صحیح بایاں_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#اگر_تعریف_شدہ SQLITE_DEBUG
  صحیح i_رکن;              /* Which WhereTerm is this, for debug purposes */
#اگر_ختم
  اتحاد {
    ساخت {
      صحیح بایاں_عمود;         /* Column number of X in "X <op> <expr>" */
      صحیح i_میدان;             /* Field in (?,?,?) IN (SELECT...) vector */
    } x;                    /* Opcode other than OP_OR or OP_AND */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  Bitmask prereq_دایاں;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#تعریف_کرو رکن_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#تعریف_کرو رکن_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#تعریف_کرو رکن_CODED      0x0004 /* This term is already coded */
#تعریف_کرو رکن_COPIED     0x0008 /* Has a child */
#تعریف_کرو رکن_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#تعریف_کرو رکن_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#تعریف_کرو رکن_OK         0x0040 /* Used during OR-clause processing */
#تعریف_کرو رکن_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#تعریف_کرو رکن_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#تعریف_کرو رکن_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#تعریف_کرو رکن_LIKE       0x0400 /* The original LIKE operator */
#تعریف_کرو رکن_IS         0x0800 /* Term.pExpr is an IS operator */
#تعریف_کرو رکن_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#تعریف_کرو رکن_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
#  define رکن_HIGHTRUTH  0x4000 /* Term excludes few rows */
#ورنہ
#  define رکن_HIGHTRUTH  0      /* Only used with STAT4 */
#اگر_ختم
#تعریف_کرو رکن_قاش      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
ساخت Where_جائزہ {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  ثابت حرف *zCollName;     /* Required collating sequence, if not NULL */
  اظہاریہ *p_Idx_اظہاریہ;            /* Search for this index expression */
  صحیح k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 op_نقاب;                /* Acceptable operators */
  حرف idxaff;               /* Must match this affinity, if zCollName!=NULL */
  غیرعلامتی حرف iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  غیرعلامتی حرف nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
  صحیح aiCur[11];             /* Cursors in the equivalence class */
  i16 ai_عمود[11];          /* Corresponding column number in the eq-class */
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
ساخت WhereClause {
  WhereInfo *pWInfo;       /* WHERE clause processing context */
  WhereClause *pOuter;     /* Outer conjunction */
  u8 op;                   /* Split operator.  TK_AND or TK_OR */
  u8 hasOr;                /* True if any a[].eOperator is WO_OR */
  صحیح n_رکن;               /* Number of terms */
  صحیح n_خانہ;               /* Number of entries in a[] */
  صحیح n_بنیاد;               /* Number of terms through the last non-Virtual */
  Where_رکن *a;            /* Each a[] describes a term of the WHERE clause */
#اگر defined(SQLITE_SMALL_ڈھیر)
  Where_رکن aStatic[1];    /* Initial static space for a[] */
#ورنہ
  Where_رکن aStatic[8];    /* Initial static space for a[] */
#اگر_ختم
};

/*
** A WhereTerm with eOperator==WO_OR has its u.pOrInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
ساخت WhereOrInfo {
  WhereClause wc;          /* Decomposition into subterms */
  Bitmask indexable;       /* Bitmask of all indexable tables in the clause */
};

/*
** A WhereTerm with eOperator==WO_AND has its u.pAndInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
ساخت WhereAndInfo {
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
ساخت Where_نقاب_سیٹ {
  صحیح bVarSelect;               /* Used by sqlite3WhereExprUsage() */
  صحیح n;                        /* Number of assigned cursor values */
  صحیح ix[BMS];                  /* Cursor assigned to each bit */
};

/*
** This object is a convenience wrapper holding all information needed
** to construct WhereLoop objects for a particular query.
*/
ساخت WhereLoopBuilder {
  WhereInfo *pWInfo;        /* Information about this WHERE */
  WhereClause *pWC;         /* WHERE clause terms */
  WhereLoop *pNew;          /* Template WhereLoop */
  Where_Or_سیٹ *p_Or_سیٹ;       /* Record best loops here, if not NULL */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  Unpacked_ریکارڈ *pRec;     /* Probe for stat4 (if required) */
  صحیح nRecValid;            /* Number of valid fields currently in pRec */
#اگر_ختم
  غیرعلامتی حرف bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  غیرعلامتی حرف bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  غیرعلامتی صحیح i_Plan_حد_بندی;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#تعریف_کرو SQLITE_BLDF1_اشاریہ_دار  0x0001   /* An index is used */
#تعریف_کرو SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#تعریف_کرو SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

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
#اگر_غیر_تعریف_شدہ SQLITE_QUERY_PLANNER_حد_بندی
# define SQLITE_QUERY_PLANNER_حد_بندی 20000
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_QUERY_PLANNER_حد_بندی_INCR
# define SQLITE_QUERY_PLANNER_حد_بندی_INCR 1000
#اگر_ختم

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
ساخت WhereInfo {
  Parse *pParse;            /* Parsing and code generating context */
  SrcList *pTabList;        /* List of tables in the join */
  اظہاریہ_List *p_ترتیب_By;       /* The ORDER BY clause or NULL */
  اظہاریہ_List *p_نتیجہ_سیٹ;     /* Result set of the query */
#اگر WHERETRACE_ENABLED
  اظہاریہ *pWhere;             /* The complete WHERE clause */
#اگر_ختم
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  صحیح aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  صحیح iContinue;            /* Jump here to continue with next record */
  صحیح iBreak;               /* Jump here to break out of the loop */
  صحیح savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst i_حد_بندی;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  غیرعلامتی bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  غیرعلامتی untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  غیرعلامتی bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  غیرعلامتی sorted        :1;   /* True if really sorted (not just grouped) */
  غیرعلامتی b_ستارہ_مکمل     :1;   /* True if check for star-query is complete */
  غیرعلامتی b_ستارہ_مستعمل     :1;   /* True if star-query heuristic is used */
  LogEst n_صف_برآمد;           /* Estimated number of output rows */
#اگر_تعریف_شدہ WHERETRACE_ENABLED
  LogEst r_کل_لاگت;        /* Total cost of the solution */
#اگر_ختم
  صحیح i_چوٹی;                 /* The very beginning of the WHERE loop */
  صحیح i_انتہا_Where;            /* End of the WHERE clause itself */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  Where_Mem_بلاک *p_Mem_مقصد_Free;/* Memory to free when this object destroyed */
  Bitmask rev_نقاب;          /* Mask of ORDER BY terms that need reversing */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  Where_نقاب_سیٹ s_نقاب_سیٹ;    /* Map cursor numbers to bitmasks */
  WhereLevel a[FLEXARRAY];  /* Information about each nest loop in WHERE */
};

/*
** The size (in bytes) of a WhereInfo object that holds N WhereLevels.
*/
#تعریف_کرو SZ_WHEREINFO(N) ROUND8(offsetof(WhereInfo,a)+(N)*حجم(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3_Where_Get_نقاب(Where_نقاب_سیٹ*,صحیح);
#اگر_تعریف_شدہ WHERETRACE_ENABLED
خالی sqlite3WhereClausePrint(WhereClause *pWC);
خالی sqlite3_Where_رکن_Print(Where_رکن *p_رکن, صحیح i_رکن);
خالی sqlite3WhereLoopPrint(ثابت WhereLoop *p, ثابت WhereClause *pWC);
#اگر_ختم
Where_رکن *sqlite3_Where_ڈھونڈو_رکن(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  صحیح iCur,             /* Cursor number of LHS */
  صحیح i_عمود,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  اشاریہ *pIdx           /* Must be compatible with this index, if not NULL */
);
خالی *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
خالی *sqlite3WhereRealloc(WhereInfo *pWInfo, خالی *pOld, u64 nByte);

/* wherecode.c: */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
صحیح sqlite3_Where_Explain_One_جائزہ(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
صحیح sqlite3WhereExplainBloomFilter(
  ثابت Parse *pParse,            /* Parse context */
  ثابت WhereInfo *pWInfo,        /* WHERE clause */
  ثابت WhereLevel *pLevel        /* Bloom filter on this level */
);
خالی sqlite3_Where_جوڑو_Explain_متن(
  Parse *pParse,                  /* Parse context */
  صحیح addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#ورنہ
# define sqlite3_Where_Explain_One_جائزہ(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3_Where_جوڑو_Explain_متن(u,v,w,x,y)
#اگر_ختم /* SQLITE_OMIT_EXPLAIN */
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
خالی sqlite3_Where_جوڑو_جائزہ_حالت(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  صحیح addrExplain                 /* Address of OP_Explain (or 0) */
);
#ورنہ
# define sqlite3_Where_جوڑو_جائزہ_حالت(a, b, c, d) ((خالی)d)
#اگر_ختم
Bitmask sqlite3_Where_رمز_One_Loop_آغاز(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  صحیح iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
);
SQLITE_NOINLINE خالی sqlite3_Where_دایاں_Join_Loop(
  WhereInfo *pWInfo,
  صحیح iLevel,
  WhereLevel *pLevel
);

/* whereexpr.c: */
خالی sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
خالی sqlite3WhereClauseClear(WhereClause*);
خالی sqlite3_Where_بانٹو(WhereClause*,اظہاریہ*,u8);
خالی sqlite3_Where_جوڑو_حد_بندی(WhereClause*, Select*);
Bitmask sqlite3_Where_اظہاریہ_Usage(Where_نقاب_سیٹ*, اظہاریہ*);
Bitmask sqlite3_Where_اظہاریہ_Usage_NN(Where_نقاب_سیٹ*, اظہاریہ*);
Bitmask sqlite3_Where_اظہاریہ_List_Usage(Where_نقاب_سیٹ*, اظہاریہ_List*);
خالی sqlite3_Where_اظہاریہ_Analyze(SrcList*, WhereClause*);
خالی sqlite3_Where_Tab_Func_دلائل(Parse*, Src_شے*, WhereClause*);
صحیح sqlite3WhereLoopBloomable(ثابت WhereLoop*);

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
#تعریف_کرو WO_اندر     0x0001
#تعریف_کرو WO_EQ     0x0002
#تعریف_کرو WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#تعریف_کرو WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#تعریف_کرو WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#تعریف_کرو WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#تعریف_کرو WO_AUX    0x0040       /* Op useful to virtual tables only */
#تعریف_کرو WO_IS     0x0080
#تعریف_کرو WO_ISNULL 0x0100
#تعریف_کرو WO_OR     0x0200       /* Two or more OR-connected terms */
#تعریف_کرو WO_AND    0x0400       /* Two or more AND-connected terms */
#تعریف_کرو WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#تعریف_کرو WO_NOOP   0x1000       /* This term does not restrict search space */
#تعریف_کرو WO_ROWVAL 0x2000       /* A row-value term */

#تعریف_کرو WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#تعریف_کرو WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#تعریف_کرو WHERE_عمود_EQ    0x00000001  /* x=EXPR */
#تعریف_کرو WHERE_عمود_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#تعریف_کرو WHERE_عمود_اندر    0x00000004  /* x IN (...) */
#تعریف_کرو WHERE_عمود_NULL  0x00000008  /* x IS NULL */
#تعریف_کرو WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#تعریف_کرو WHERE_چوٹی_حد_بندی    0x00000010  /* x<EXPR or x<=EXPR constraint */
#تعریف_کرو WHERE_BTM_حد_بندی    0x00000020  /* x>EXPR or x>=EXPR constraint */
#تعریف_کرو WHERE_BOTH_حد_بندی   0x00000030  /* Both x>EXPR and x<EXPR */
#تعریف_کرو WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#تعریف_کرو WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#تعریف_کرو WHERE_اشاریہ_دار      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#تعریف_کرو WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#تعریف_کرو WHERE_اندر_ABLE      0x00000800  /* Able to support an IN operator */
#تعریف_کرو WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#تعریف_کرو WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#تعریف_کرو WHERE_AUTO_اشاریہ   0x00004000  /* Uses an ephemeral index */
#تعریف_کرو WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#تعریف_کرو WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#تعریف_کرو WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#تعریف_کرو WHERE_اندر_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#تعریف_کرو WHERE_BIGNULL_ترتیب_دو 0x00080000  /* Column nEq of index is BIGNULL */
#تعریف_کرو WHERE_اندر_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#تعریف_کرو WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#تعریف_کرو WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#تعریف_کرو WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#تعریف_کرو WHERE_OMIT_انحراف  0x01000000  /* Set offset counter to zero */
#تعریف_کرو WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#تعریف_کرو WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#اگر_ختم /* !defined(SQLITE_WHEREINT_H) */
