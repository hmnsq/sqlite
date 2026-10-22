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
#ifndef SQLITE_WHEREINT_H
#define SQLITE_WHEREINT_H


/* Forward references
*/
प्रकारनिर्देश संरचना WhereClause WhereClause;
प्रकारनिर्देश संरचना WhereMaskSet WhereMaskSet;
प्रकारनिर्देश संरचना WhereOrInfo WhereOrInfo;
प्रकारनिर्देश संरचना WhereAndInfo WhereAndInfo;
प्रकारनिर्देश संरचना WhereLevel WhereLevel;
प्रकारनिर्देश संरचना WhereLoop WhereLoop;
प्रकारनिर्देश संरचना WherePath WherePath;
प्रकारनिर्देश संरचना WhereTerm WhereTerm;
प्रकारनिर्देश संरचना WhereLoopBuilder WhereLoopBuilder;
प्रकारनिर्देश संरचना WhereScan WhereScan;
प्रकारनिर्देश संरचना WhereOrCost WhereOrCost;
प्रकारनिर्देश संरचना WhereOrSet WhereOrSet;
प्रकारनिर्देश संरचना WhereMemBlock WhereMemBlock;
प्रकारनिर्देश संरचना Where_दक्षिणः_Join Where_दक्षिणः_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
संरचना WhereMemBlock {
  WhereMemBlock *pNext;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
संरचना Where_दक्षिणः_Join {
  पूर्णाङ्क iMatch;          /* Cursor used to determine prior matched rows */
  पूर्णाङ्क regBloom;        /* Bloom filter for iRJMatch */
  पूर्णाङ्क regReturn;       /* Return register for the interior subroutine */
  पूर्णाङ्क addrSubrtn;      /* Starting address for the interior subroutine */
  पूर्णाङ्क endSubrtn;       /* The last opcode in the interior subroutine */
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
संरचना WhereLevel {
  पूर्णाङ्क i_वामः_Join;        /* Memory cell used to implement LEFT OUTER JOIN */
  पूर्णाङ्क iTabCur;          /* The VDBE cursor used to access the table */
  पूर्णाङ्क iIdxCur;          /* The VDBE cursor used to access pIdx */
  पूर्णाङ्क addrBrk;          /* Jump here to break out of the loop */
  पूर्णाङ्क addrHalt;         /* Abort the query due to empty table or similar */
  पूर्णाङ्क addrNxt;          /* Jump here to start the next IN combination */
  पूर्णाङ्क addrSkip;         /* Jump here for next iteration of skip-scan */
  पूर्णाङ्क addrCont;         /* Jump here to continue with the next loop cycle */
  पूर्णाङ्क addrDeleteCont;   /* Delete auto-index entry then same as addrCont */
  पूर्णाङ्क addrFirst;        /* First instruction of interior of the loop */
  पूर्णाङ्क addrBody;         /* Beginning of the body of this loop */
  पूर्णाङ्क regBignull;       /* big-null flag reg. True if a NULL-scan is needed */
  पूर्णाङ्क addrBignull;      /* Jump here for next part of big-null scan */
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  पूर्णाङ्क addrLikeRep;      /* LIKE range processing address */
#endif
  पूर्णाङ्क regFilter;        /* Bloom filter */
  Where_दक्षिणः_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  पूर्णाङ्क p1, p2;           /* Operands of the opcode used to end the loop */
  सङ्घ {               /* Information that depends on pWLoop->wsFlags */
    संरचना {
      पूर्णाङ्क nIn;              /* Number of entries in aInLoop[] */
      संरचना InLoop {
        पूर्णाङ्क iCur;              /* The VDBE cursor used by this IN operator */
        पूर्णाङ्क addr_In_शिखरम्;         /* Top of the IN loop */
        पूर्णाङ्क iBase;             /* Base register of multi-key index record */
        पूर्णाङ्क nPrefix;           /* Number of prior entries in the key */
        u8 eEndLoopOp;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *aInLoop;           /* Information about each nested IN operator */
    } in;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    Index *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  संरचना WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णाङ्क addrVisit;        /* Address at which row is visited */
#endif
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
संरचना WhereLoop {
  Bitmask prereq;       /* Bitmask of other loops that must run first */
  Bitmask maskSelf;     /* Bitmask identifying table iTab */
#ifdef SQLITE_DEBUG
  वर्ण cId;             /* Symbolic ID of this loop for debugging use */
#endif
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 iSortIdx;          /* Sorting index number.  0==None */
  LogEst rSetup;        /* One-time setup cost (ex: create transient index) */
  LogEst rRun;          /* Cost of running each loop */
  LogEst nOut;          /* Estimated number of output rows */
  सङ्घ {
    संरचना {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_शिखरम्;              /* Size of TOP vector */
      u16 nDistinctCol;      /* Index columns used to sort for DISTINCT */
      Index *pIndex;         /* Index used, or NULL */
      व्यञ्जकम्_List *pOrderBy;    /* ORDER BY clause if this is really a subquery */
    } btree;
    संरचना {               /* Information for virtual tables */
      पूर्णाङ्क idx_अङ्कः;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 bOmitOffset : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_अङ्कः_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omitMask;          /* Terms that may be omitted */
      वर्ण *idxStr;          /* Index identifier string */
      u32 mHandleIn;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#ifdef WHERETRACE_ENABLED
  LogEst rStarDelta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#endif
  WhereTerm **aLTerm;   /* WhereTerms used */
  WhereLoop *pNextLoop; /* Next WhereLoop object in the WhereClause */
  WhereTerm *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
संरचना WhereOrCost {
  Bitmask prereq;     /* Prerequisites */
  LogEst rRun;        /* Cost of running this subquery */
  LogEst nOut;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#define N_OR_COST 3
संरचना WhereOrSet {
  u16 n;                      /* Number of valid a[] entries */
  WhereOrCost a[N_OR_COST];   /* Set of best costs */
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
संरचना WherePath {
  Bitmask maskLoop;     /* Bitmask of all WhereLoop objects in this path */
  Bitmask revLoop;      /* aLoop[]s that should be reversed for ORDER BY */
  LogEst nRow;          /* Estimated number of rows generated by this path */
  LogEst rCost;         /* Total cost of this path */
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
संरचना WhereTerm {
  व्यञ्जकम् *p_व्यञ्जकम्;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 nChild;              /* Number of children that must disable us */
  u8 eMatchOp;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  पूर्णाङ्क iParent;            /* Disable pWC->a[iParent] when this term disabled */
  पूर्णाङ्क वामः_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#ifdef SQLITE_DEBUG
  पूर्णाङ्क iTerm;              /* Which WhereTerm is this, for debug purposes */
#endif
  सङ्घ {
    संरचना {
      पूर्णाङ्क वामः_Column;         /* Column number of X in "X <op> <expr>" */
      पूर्णाङ्क iField;             /* Field in (?,?,?) IN (SELECT...) vector */
    } x;                    /* Opcode other than OP_OR or OP_AND */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  Bitmask prereq_दक्षिणः;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#define TERM_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#define TERM_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#define TERM_CODED      0x0004 /* This term is already coded */
#define TERM_COPIED     0x0008 /* Has a child */
#define TERM_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#define TERM_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#define TERM_OK         0x0040 /* Used during OR-clause processing */
#define TERM_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#define TERM_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#define TERM_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#define TERM_LIKE       0x0400 /* The original LIKE operator */
#define TERM_IS         0x0800 /* Term.pExpr is an IS operator */
#define TERM_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#define TERM_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#ifdef SQLITE_ENABLE_STAT4
#  define TERM_HIGHTRUTH  0x4000 /* Term excludes few rows */
#अञ्ञथा
#  define TERM_HIGHTRUTH  0      /* Only used with STAT4 */
#endif
#define TERM_SLICE      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
संरचना WhereScan {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  ध्रुव वर्ण *zCollName;     /* Required collating sequence, if not NULL */
  व्यञ्जकम् *p_Idx_व्यञ्जकम्;            /* Search for this index expression */
  पूर्णाङ्क k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 opMask;                /* Acceptable operators */
  वर्ण idxaff;               /* Must match this affinity, if zCollName!=NULL */
  unsigned वर्ण iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  unsigned वर्ण nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
  पूर्णाङ्क aiCur[11];             /* Cursors in the equivalence class */
  i16 aiColumn[11];          /* Corresponding column number in the eq-class */
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
संरचना WhereClause {
  WhereInfo *pWInfo;       /* WHERE clause processing context */
  WhereClause *pOuter;     /* Outer conjunction */
  u8 op;                   /* Split operator.  TK_AND or TK_OR */
  u8 hasOr;                /* True if any a[].eOperator is WO_OR */
  पूर्णाङ्क nTerm;               /* Number of terms */
  पूर्णाङ्क nSlot;               /* Number of entries in a[] */
  पूर्णाङ्क nBase;               /* Number of terms through the last non-Virtual */
  WhereTerm *a;            /* Each a[] describes a term of the WHERE clause */
#सचे defined(SQLITE_SMALL_राशिः)
  WhereTerm aStatic[1];    /* Initial static space for a[] */
#अञ्ञथा
  WhereTerm aStatic[8];    /* Initial static space for a[] */
#endif
};

/*
** A WhereTerm with eOperator==WO_OR has its u.pOrInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
संरचना WhereOrInfo {
  WhereClause wc;          /* Decomposition into subterms */
  Bitmask indexable;       /* Bitmask of all indexable tables in the clause */
};

/*
** A WhereTerm with eOperator==WO_AND has its u.pAndInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
संरचना WhereAndInfo {
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
संरचना WhereMaskSet {
  पूर्णाङ्क bVarSelect;               /* Used by sqlite3WhereExprUsage() */
  पूर्णाङ्क n;                        /* Number of assigned cursor values */
  पूर्णाङ्क ix[BMS];                  /* Cursor assigned to each bit */
};

/*
** This object is a convenience wrapper holding all information needed
** to construct WhereLoop objects for a particular query.
*/
संरचना WhereLoopBuilder {
  WhereInfo *pWInfo;        /* Information about this WHERE */
  WhereClause *pWC;         /* WHERE clause terms */
  WhereLoop *pNew;          /* Template WhereLoop */
  WhereOrSet *pOrSet;       /* Record best loops here, if not NULL */
#ifdef SQLITE_ENABLE_STAT4
  UnpackedRecord *pRec;     /* Probe for stat4 (if required) */
  पूर्णाङ्क nRecValid;            /* Number of valid fields currently in pRec */
#endif
  unsigned वर्ण bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  unsigned वर्ण bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  unsigned पूर्णाङ्क iPlanLimit;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#define SQLITE_BLDF1_INDEXED  0x0001   /* An index is used */
#define SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#define SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

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
#ifndef SQLITE_QUERY_PLANNER_LIMIT
# define SQLITE_QUERY_PLANNER_LIMIT 20000
#endif
#ifndef SQLITE_QUERY_PLANNER_LIMIT_INCR
# define SQLITE_QUERY_PLANNER_LIMIT_INCR 1000
#endif

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
संरचना WhereInfo {
  Parse *pParse;            /* Parsing and code generating context */
  SrcList *pTabList;        /* List of tables in the join */
  व्यञ्जकम्_List *pOrderBy;       /* The ORDER BY clause or NULL */
  व्यञ्जकम्_List *pResultSet;     /* Result set of the query */
#सचे WHERETRACE_ENABLED
  व्यञ्जकम् *pWhere;             /* The complete WHERE clause */
#endif
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  पूर्णाङ्क aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  पूर्णाङ्क iContinue;            /* Jump here to continue with next record */
  पूर्णाङ्क iBreak;               /* Jump here to break out of the loop */
  पूर्णाङ्क savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst iLimit;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  unsigned bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  unsigned untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  unsigned bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  unsigned sorted        :1;   /* True if really sorted (not just grouped) */
  unsigned bStarDone     :1;   /* True if check for star-query is complete */
  unsigned bStarUsed     :1;   /* True if star-query heuristic is used */
  LogEst nRowOut;           /* Estimated number of output rows */
#ifdef WHERETRACE_ENABLED
  LogEst rTotalCost;        /* Total cost of the solution */
#endif
  पूर्णाङ्क i_शिखरम्;                 /* The very beginning of the WHERE loop */
  पूर्णाङ्क iEndWhere;            /* End of the WHERE clause itself */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  WhereMemBlock *pMemToFree;/* Memory to free when this object destroyed */
  Bitmask revMask;          /* Mask of ORDER BY terms that need reversing */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  WhereMaskSet sMaskSet;    /* Map cursor numbers to bitmasks */
  WhereLevel a[FLEXARRAY];  /* Information about each nest loop in WHERE */
};

/*
** The size (in bytes) of a WhereInfo object that holds N WhereLevels.
*/
#define SZ_WHEREINFO(N) ROUND8(offsetof(WhereInfo,a)+(N)*परिमाण(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3WhereGetMask(WhereMaskSet*,पूर्णाङ्क);
#ifdef WHERETRACE_ENABLED
शून्य sqlite3WhereClausePrint(WhereClause *pWC);
शून्य sqlite3WhereTermPrint(WhereTerm *pTerm, पूर्णाङ्क iTerm);
शून्य sqlite3WhereLoopPrint(ध्रुव WhereLoop *p, ध्रुव WhereClause *pWC);
#endif
WhereTerm *sqlite3WhereFindTerm(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  पूर्णाङ्क iCur,             /* Cursor number of LHS */
  पूर्णाङ्क iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  Index *pIdx           /* Must be compatible with this index, if not NULL */
);
शून्य *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
शून्य *sqlite3WhereRealloc(WhereInfo *pWInfo, शून्य *pOld, u64 nByte);

/* wherecode.c: */
#ifndef SQLITE_OMIT_EXPLAIN
पूर्णाङ्क sqlite3WhereExplainOneScan(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
पूर्णाङ्क sqlite3WhereExplainBloomFilter(
  ध्रुव Parse *pParse,            /* Parse context */
  ध्रुव WhereInfo *pWInfo,        /* WHERE clause */
  ध्रुव WhereLevel *pLevel        /* Bloom filter on this level */
);
शून्य sqlite3WhereAddExplainText(
  Parse *pParse,                  /* Parse context */
  पूर्णाङ्क addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#अञ्ञथा
# define sqlite3WhereExplainOneScan(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3WhereAddExplainText(u,v,w,x,y)
#endif /* SQLITE_OMIT_EXPLAIN */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
शून्य sqlite3WhereAddScanStatus(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  पूर्णाङ्क addrExplain                 /* Address of OP_Explain (or 0) */
);
#अञ्ञथा
# define sqlite3WhereAddScanStatus(a, b, c, d) ((शून्य)d)
#endif
Bitmask sqlite3WhereCodeOneLoopStart(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  पूर्णाङ्क iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
);
SQLITE_NOINLINE शून्य sqlite3_Where_दक्षिणः_Join_Loop(
  WhereInfo *pWInfo,
  पूर्णाङ्क iLevel,
  WhereLevel *pLevel
);

/* whereexpr.c: */
शून्य sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
शून्य sqlite3WhereClauseClear(WhereClause*);
शून्य sqlite3WhereSplit(WhereClause*,व्यञ्जकम्*,u8);
शून्य sqlite3WhereAddLimit(WhereClause*, Select*);
Bitmask sqlite3_Where_व्यञ्जकम्_Usage(WhereMaskSet*, व्यञ्जकम्*);
Bitmask sqlite3_Where_व्यञ्जकम्_Usage_NN(WhereMaskSet*, व्यञ्जकम्*);
Bitmask sqlite3_Where_व्यञ्जकम्_List_Usage(WhereMaskSet*, व्यञ्जकम्_List*);
शून्य sqlite3_Where_व्यञ्जकम्_Analyze(SrcList*, WhereClause*);
शून्य sqlite3WhereTabFuncArgs(Parse*, SrcItem*, WhereClause*);
पूर्णाङ्क sqlite3WhereLoopBloomable(ध्रुव WhereLoop*);

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
#define WO_IN     0x0001
#define WO_EQ     0x0002
#define WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#define WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#define WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#define WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#define WO_AUX    0x0040       /* Op useful to virtual tables only */
#define WO_IS     0x0080
#define WO_ISNULL 0x0100
#define WO_OR     0x0200       /* Two or more OR-connected terms */
#define WO_AND    0x0400       /* Two or more AND-connected terms */
#define WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#define WO_NOOP   0x1000       /* This term does not restrict search space */
#define WO_ROWVAL 0x2000       /* A row-value term */

#define WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#define WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#define WHERE_COLUMN_EQ    0x00000001  /* x=EXPR */
#define WHERE_COLUMN_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#define WHERE_COLUMN_IN    0x00000004  /* x IN (...) */
#define WHERE_COLUMN_NULL  0x00000008  /* x IS NULL */
#define WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#define WHERE_शिखरम्_LIMIT    0x00000010  /* x<EXPR or x<=EXPR constraint */
#define WHERE_BTM_LIMIT    0x00000020  /* x>EXPR or x>=EXPR constraint */
#define WHERE_BOTH_LIMIT   0x00000030  /* Both x>EXPR and x<EXPR */
#define WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#define WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#define WHERE_INDEXED      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#define WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#define WHERE_IN_ABLE      0x00000800  /* Able to support an IN operator */
#define WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#define WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#define WHERE_AUTO_INDEX   0x00004000  /* Uses an ephemeral index */
#define WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#define WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#define WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#define WHERE_IN_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#define WHERE_BIGNULL_SORT 0x00080000  /* Column nEq of index is BIGNULL */
#define WHERE_IN_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#define WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#define WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#define WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#define WHERE_OMIT_OFFSET  0x01000000  /* Set offset counter to zero */
#define WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#define WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#endif /* !defined(SQLITE_WHEREINT_H) */
