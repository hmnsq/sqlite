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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WHEREINT_H
#وصف_ڪريو SQLITE_WHEREINT_H


/* Forward references
*/
تعريفنوع بنية WhereClause WhereClause;
تعريفنوع بنية Where_نقاب_سيٽ Where_نقاب_سيٽ;
تعريفنوع بنية WhereOrInfo WhereOrInfo;
تعريفنوع بنية WhereAndInfo WhereAndInfo;
تعريفنوع بنية WhereLevel WhereLevel;
تعريفنوع بنية WhereLoop WhereLoop;
تعريفنوع بنية Where_رستو Where_رستو;
تعريفنوع بنية Where_رڪن Where_رڪن;
تعريفنوع بنية WhereLoopBuilder WhereLoopBuilder;
تعريفنوع بنية Where_جاچ Where_جاچ;
تعريفنوع بنية Where_Or_لاڳت Where_Or_لاڳت;
تعريفنوع بنية Where_Or_سيٽ Where_Or_سيٽ;
تعريفنوع بنية Where_Mem_بلاڪ Where_Mem_بلاڪ;
تعريفنوع بنية Where_ساڄو_Join Where_ساڄو_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
بنية Where_Mem_بلاڪ {
  Where_Mem_بلاڪ *p_ايندڙ;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
بنية Where_ساڄو_Join {
  صحيح i_ميلاپ;          /* Cursor used to determine prior matched rows */
  صحيح regBloom;        /* Bloom filter for iRJMatch */
  صحيح regReturn;       /* Return register for the interior subroutine */
  صحيح addrSubrtn;      /* Starting address for the interior subroutine */
  صحيح پڇاڙي_Subrtn;       /* The last opcode in the interior subroutine */
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
  صحيح i_کاٻو_Join;        /* Memory cell used to implement LEFT OUTER JOIN */
  صحيح iTabCur;          /* The VDBE cursor used to access the table */
  صحيح iIdxCur;          /* The VDBE cursor used to access pIdx */
  صحيح addrBrk;          /* Jump here to break out of the loop */
  صحيح addrHalt;         /* Abort the query due to empty table or similar */
  صحيح addrNxt;          /* Jump here to start the next IN combination */
  صحيح addrSkip;         /* Jump here for next iteration of skip-scan */
  صحيح addrCont;         /* Jump here to continue with the next loop cycle */
  صحيح addrDeleteCont;   /* Delete auto-index entry then same as addrCont */
  صحيح addr_پهريون;        /* First instruction of interior of the loop */
  صحيح addrBody;         /* Beginning of the body of this loop */
  صحيح regBignull;       /* big-null flag reg. True if a NULL-scan is needed */
  صحيح addrBignull;      /* Jump here for next part of big-null scan */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_LIKE_DOESNT_ميلاپ_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  صحيح addrLikeRep;      /* LIKE range processing address */
#جيڪڏهن_پورو
  صحيح regFilter;        /* Bloom filter */
  Where_ساڄو_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  صحيح p1, p2;           /* Operands of the opcode used to end the loop */
  اتحاد {               /* Information that depends on pWLoop->wsFlags */
    بنية {
      صحيح n_اندريون;              /* Number of entries in aInLoop[] */
      بنية اندريون_Loop {
        صحيح iCur;              /* The VDBE cursor used by this IN operator */
        صحيح addr_اندريون_چوٽي;         /* Top of the IN loop */
        صحيح i_بنياد;             /* Base register of multi-key index record */
        صحيح n_اڳياڙي;           /* Number of prior entries in the key */
        u8 e_پڇاڙي_Loop_Op;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *a_اندريون_Loop;           /* Information about each nested IN operator */
    } اندريون;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    اشاريو *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  بنية WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addr_گهمو;        /* Address at which row is visited */
#جيڪڏهن_پورو
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
  Bitmask نقاب_Self;     /* Bitmask identifying table iTab */
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  حرف cId;             /* Symbolic ID of this loop for debugging use */
#جيڪڏهن_پورو
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 i_ترتيب_ڏيو_Idx;          /* Sorting index number.  0==None */
  LogEst r_تياري;        /* One-time setup cost (ex: create transient index) */
  LogEst r_لاڳيتو;          /* Cost of running each loop */
  LogEst n_ٻاهريون;          /* Estimated number of output rows */
  اتحاد {
    بنية {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_چوٽي;              /* Size of TOP vector */
      u16 n_Distinct_کانو;      /* Index columns used to sort for DISTINCT */
      اشاريو *p_اشاريو;         /* Index used, or NULL */
      اظهار_List *p_ترتيب_By;    /* ORDER BY clause if this is really a subquery */
    } btree;
    بنية {               /* Information for virtual tables */
      صحيح idx_انگ;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 b_Omit_وٿي : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_انگ_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omit_نقاب;          /* Terms that may be omitted */
      حرف *idxStr;          /* Index identifier string */
      u32 m_Handle_اندريون;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#جيڪڏهن_وصف_ٿيل WHERETRACE_ENABLED
  LogEst r_تارو_Delta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#جيڪڏهن_پورو
  Where_رڪن **aLTerm;   /* WhereTerms used */
  WhereLoop *p_ايندڙ_Loop; /* Next WhereLoop object in the WhereClause */
  Where_رڪن *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
بنية Where_Or_لاڳت {
  Bitmask prereq;     /* Prerequisites */
  LogEst r_لاڳيتو;        /* Cost of running this subquery */
  LogEst n_ٻاهريون;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#وصف_ڪريو N_OR_لاڳت 3
بنية Where_Or_سيٽ {
  u16 n;                      /* Number of valid a[] entries */
  Where_Or_لاڳت a[N_OR_لاڳت];   /* Set of best costs */
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
بنية Where_رستو {
  Bitmask نقاب_Loop;     /* Bitmask of all WhereLoop objects in this path */
  Bitmask revLoop;      /* aLoop[]s that should be reversed for ORDER BY */
  LogEst n_صف;          /* Estimated number of rows generated by this path */
  LogEst r_لاڳت;         /* Total cost of this path */
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
بنية Where_رڪن {
  اظهار *p_اظهار;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 n_ٻار;              /* Number of children that must disable us */
  u8 e_ميلاپ_Op;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  صحيح i_والد;            /* Disable pWC->a[iParent] when this term disabled */
  صحيح کاٻو_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  صحيح i_رڪن;              /* Which WhereTerm is this, for debug purposes */
#جيڪڏهن_پورو
  اتحاد {
    بنية {
      صحيح کاٻو_ڪالم;         /* Column number of X in "X <op> <expr>" */
      صحيح i_ميدان;             /* Field in (?,?,?) IN (SELECT...) vector */
    } x;                    /* Opcode other than OP_OR or OP_AND */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  Bitmask prereq_ساڄو;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#وصف_ڪريو رڪن_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#وصف_ڪريو رڪن_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#وصف_ڪريو رڪن_CODED      0x0004 /* This term is already coded */
#وصف_ڪريو رڪن_COPIED     0x0008 /* Has a child */
#وصف_ڪريو رڪن_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#وصف_ڪريو رڪن_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#وصف_ڪريو رڪن_OK         0x0040 /* Used during OR-clause processing */
#وصف_ڪريو رڪن_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#وصف_ڪريو رڪن_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#وصف_ڪريو رڪن_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#وصف_ڪريو رڪن_LIKE       0x0400 /* The original LIKE operator */
#وصف_ڪريو رڪن_IS         0x0800 /* Term.pExpr is an IS operator */
#وصف_ڪريو رڪن_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#وصف_ڪريو رڪن_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
#  define رڪن_HIGHTRUTH  0x4000 /* Term excludes few rows */
#وإلا
#  define رڪن_HIGHTRUTH  0      /* Only used with STAT4 */
#جيڪڏهن_پورو
#وصف_ڪريو رڪن_ٽڪرو      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
بنية Where_جاچ {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  ثابت حرف *zCollName;     /* Required collating sequence, if not NULL */
  اظهار *p_Idx_اظهار;            /* Search for this index expression */
  صحيح k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 op_نقاب;                /* Acceptable operators */
  حرف idxaff;               /* Must match this affinity, if zCollName!=NULL */
  بنانشان حرف iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  بنانشان حرف nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
  صحيح aiCur[11];             /* Cursors in the equivalence class */
  i16 ai_ڪالم[11];          /* Corresponding column number in the eq-class */
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
  صحيح n_رڪن;               /* Number of terms */
  صحيح n_جڳهه;               /* Number of entries in a[] */
  صحيح n_بنياد;               /* Number of terms through the last non-Virtual */
  Where_رڪن *a;            /* Each a[] describes a term of the WHERE clause */
#إذا defined(SQLITE_SMALL_ڍير)
  Where_رڪن aStatic[1];    /* Initial static space for a[] */
#وإلا
  Where_رڪن aStatic[8];    /* Initial static space for a[] */
#جيڪڏهن_پورو
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
بنية Where_نقاب_سيٽ {
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
  Where_Or_سيٽ *p_Or_سيٽ;       /* Record best loops here, if not NULL */
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STAT4
  Unpacked_رڪارڊ *pRec;     /* Probe for stat4 (if required) */
  صحيح nRecValid;            /* Number of valid fields currently in pRec */
#جيڪڏهن_پورو
  بنانشان حرف bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  بنانشان حرف bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  بنانشان صحيح i_Plan_حد_بندي;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#وصف_ڪريو SQLITE_BLDF1_اشاريي_وارو  0x0001   /* An index is used */
#وصف_ڪريو SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#وصف_ڪريو SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_QUERY_PLANNER_حد_بندي
# define SQLITE_QUERY_PLANNER_حد_بندي 20000
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_QUERY_PLANNER_حد_بندي_INCR
# define SQLITE_QUERY_PLANNER_حد_بندي_INCR 1000
#جيڪڏهن_پورو

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
  اظهار_List *p_ترتيب_By;       /* The ORDER BY clause or NULL */
  اظهار_List *p_نتيجو_سيٽ;     /* Result set of the query */
#إذا WHERETRACE_ENABLED
  اظهار *pWhere;             /* The complete WHERE clause */
#جيڪڏهن_پورو
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  صحيح aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  صحيح iContinue;            /* Jump here to continue with next record */
  صحيح iBreak;               /* Jump here to break out of the loop */
  صحيح savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst i_حد_بندي;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  بنانشان bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  بنانشان untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  بنانشان bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  بنانشان sorted        :1;   /* True if really sorted (not just grouped) */
  بنانشان b_تارو_پورو     :1;   /* True if check for star-query is complete */
  بنانشان b_تارو_استعمال_ٿيل     :1;   /* True if star-query heuristic is used */
  LogEst n_صف_ٻاهريون;           /* Estimated number of output rows */
#جيڪڏهن_وصف_ٿيل WHERETRACE_ENABLED
  LogEst r_ڪل_لاڳت;        /* Total cost of the solution */
#جيڪڏهن_پورو
  صحيح i_چوٽي;                 /* The very beginning of the WHERE loop */
  صحيح i_پڇاڙي_Where;            /* End of the WHERE clause itself */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  Where_Mem_بلاڪ *p_Mem_منزل_Free;/* Memory to free when this object destroyed */
  Bitmask rev_نقاب;          /* Mask of ORDER BY terms that need reversing */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  Where_نقاب_سيٽ s_نقاب_سيٽ;    /* Map cursor numbers to bitmasks */
  WhereLevel a[FLEXARRAY];  /* Information about each nest loop in WHERE */
};

/*
** The size (in bytes) of a WhereInfo object that holds N WhereLevels.
*/
#وصف_ڪريو SZ_WHEREINFO(N) ROUND8(offsetof(WhereInfo,a)+(N)*حجم(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3_Where_Get_نقاب(Where_نقاب_سيٽ*,صحيح);
#جيڪڏهن_وصف_ٿيل WHERETRACE_ENABLED
فراغ sqlite3WhereClausePrint(WhereClause *pWC);
فراغ sqlite3_Where_رڪن_Print(Where_رڪن *p_رڪن, صحيح i_رڪن);
فراغ sqlite3WhereLoopPrint(ثابت WhereLoop *p, ثابت WhereClause *pWC);
#جيڪڏهن_پورو
Where_رڪن *sqlite3_Where_ڳوليو_رڪن(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  صحيح iCur,             /* Cursor number of LHS */
  صحيح i_ڪالم,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  اشاريو *pIdx           /* Must be compatible with this index, if not NULL */
);
فراغ *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
فراغ *sqlite3WhereRealloc(WhereInfo *pWInfo, فراغ *pOld, u64 nByte);

/* wherecode.c: */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_EXPLAIN
صحيح sqlite3_Where_Explain_One_جاچ(
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
فراغ sqlite3_Where_جوڙيو_Explain_متن(
  Parse *pParse,                  /* Parse context */
  صحيح addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#وإلا
# define sqlite3_Where_Explain_One_جاچ(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3_Where_جوڙيو_Explain_متن(u,v,w,x,y)
#جيڪڏهن_پورو /* SQLITE_OMIT_EXPLAIN */
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STMT_SCANSTATUS
فراغ sqlite3_Where_جوڙيو_جاچ_حالت(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  صحيح addrExplain                 /* Address of OP_Explain (or 0) */
);
#وإلا
# define sqlite3_Where_جوڙيو_جاچ_حالت(a, b, c, d) ((فراغ)d)
#جيڪڏهن_پورو
Bitmask sqlite3_Where_ڪوڊ_One_Loop_شروعات(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  صحيح iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
);
SQLITE_NOINLINE فراغ sqlite3_Where_ساڄو_Join_Loop(
  WhereInfo *pWInfo,
  صحيح iLevel,
  WhereLevel *pLevel
);

/* whereexpr.c: */
فراغ sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
فراغ sqlite3WhereClauseClear(WhereClause*);
فراغ sqlite3_Where_ورهايو(WhereClause*,اظهار*,u8);
فراغ sqlite3_Where_جوڙيو_حد_بندي(WhereClause*, Select*);
Bitmask sqlite3_Where_اظهار_Usage(Where_نقاب_سيٽ*, اظهار*);
Bitmask sqlite3_Where_اظهار_Usage_NN(Where_نقاب_سيٽ*, اظهار*);
Bitmask sqlite3_Where_اظهار_List_Usage(Where_نقاب_سيٽ*, اظهار_List*);
فراغ sqlite3_Where_اظهار_Analyze(SrcList*, WhereClause*);
فراغ sqlite3_Where_Tab_Func_دليلون(Parse*, Src_شي*, WhereClause*);
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
#وصف_ڪريو WO_اندريون     0x0001
#وصف_ڪريو WO_EQ     0x0002
#وصف_ڪريو WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#وصف_ڪريو WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#وصف_ڪريو WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#وصف_ڪريو WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#وصف_ڪريو WO_AUX    0x0040       /* Op useful to virtual tables only */
#وصف_ڪريو WO_IS     0x0080
#وصف_ڪريو WO_ISNULL 0x0100
#وصف_ڪريو WO_OR     0x0200       /* Two or more OR-connected terms */
#وصف_ڪريو WO_AND    0x0400       /* Two or more AND-connected terms */
#وصف_ڪريو WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#وصف_ڪريو WO_NOOP   0x1000       /* This term does not restrict search space */
#وصف_ڪريو WO_ROWVAL 0x2000       /* A row-value term */

#وصف_ڪريو WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#وصف_ڪريو WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#وصف_ڪريو WHERE_ڪالم_EQ    0x00000001  /* x=EXPR */
#وصف_ڪريو WHERE_ڪالم_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#وصف_ڪريو WHERE_ڪالم_اندريون    0x00000004  /* x IN (...) */
#وصف_ڪريو WHERE_ڪالم_NULL  0x00000008  /* x IS NULL */
#وصف_ڪريو WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#وصف_ڪريو WHERE_چوٽي_حد_بندي    0x00000010  /* x<EXPR or x<=EXPR constraint */
#وصف_ڪريو WHERE_BTM_حد_بندي    0x00000020  /* x>EXPR or x>=EXPR constraint */
#وصف_ڪريو WHERE_BOTH_حد_بندي   0x00000030  /* Both x>EXPR and x<EXPR */
#وصف_ڪريو WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#وصف_ڪريو WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#وصف_ڪريو WHERE_اشاريي_وارو      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#وصف_ڪريو WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#وصف_ڪريو WHERE_اندريون_ABLE      0x00000800  /* Able to support an IN operator */
#وصف_ڪريو WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#وصف_ڪريو WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#وصف_ڪريو WHERE_AUTO_اشاريو   0x00004000  /* Uses an ephemeral index */
#وصف_ڪريو WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#وصف_ڪريو WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#وصف_ڪريو WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#وصف_ڪريو WHERE_اندريون_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#وصف_ڪريو WHERE_BIGNULL_ترتيب_ڏيو 0x00080000  /* Column nEq of index is BIGNULL */
#وصف_ڪريو WHERE_اندريون_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#وصف_ڪريو WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#وصف_ڪريو WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#وصف_ڪريو WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#وصف_ڪريو WHERE_OMIT_وٿي  0x01000000  /* Set offset counter to zero */
#وصف_ڪريو WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#وصف_ڪريو WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#جيڪڏهن_پورو /* !defined(SQLITE_WHEREINT_H) */
