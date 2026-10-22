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
#अगर_अघोषित SQLITE_WHEREINT_H
#घोषणा SQLITE_WHEREINT_H


/* Forward references
*/
प्रकार काष्ठा WhereClause WhereClause;
प्रकार काष्ठा WhereMaskSet WhereMaskSet;
प्रकार काष्ठा WhereOrInfo WhereOrInfo;
प्रकार काष्ठा WhereAndInfo WhereAndInfo;
प्रकार काष्ठा WhereLevel WhereLevel;
प्रकार काष्ठा WhereLoop WhereLoop;
प्रकार काष्ठा WherePath WherePath;
प्रकार काष्ठा WhereTerm WhereTerm;
प्रकार काष्ठा WhereLoopBuilder WhereLoopBuilder;
प्रकार काष्ठा WhereScan WhereScan;
प्रकार काष्ठा WhereOrCost WhereOrCost;
प्रकार काष्ठा WhereOrSet WhereOrSet;
प्रकार काष्ठा WhereMemBlock WhereMemBlock;
प्रकार काष्ठा Where_दायाँ_Join Where_दायाँ_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
काष्ठा WhereMemBlock {
  WhereMemBlock *p_अग्र;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
काष्ठा Where_दायाँ_Join {
  पूर्णांक iMatch;          /* Cursor used to determine prior matched rows */
  पूर्णांक regBloom;        /* Bloom filter for iRJMatch */
  पूर्णांक regReturn;       /* Return register for the interior subroutine */
  पूर्णांक addrSubrtn;      /* Starting address for the interior subroutine */
  पूर्णांक endSubrtn;       /* The last opcode in the interior subroutine */
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
काष्ठा WhereLevel {
  पूर्णांक i_बायाँ_Join;        /* Memory cell used to implement LEFT OUTER JOIN */
  पूर्णांक iTabCur;          /* The VDBE cursor used to access the table */
  पूर्णांक iIdxCur;          /* The VDBE cursor used to access pIdx */
  पूर्णांक addrBrk;          /* Jump here to break out of the loop */
  पूर्णांक addrHalt;         /* Abort the query due to empty table or similar */
  पूर्णांक addrNxt;          /* Jump here to start the next IN combination */
  पूर्णांक addrSkip;         /* Jump here for next iteration of skip-scan */
  पूर्णांक addrCont;         /* Jump here to continue with the next loop cycle */
  पूर्णांक addrDeleteCont;   /* Delete auto-index entry then same as addrCont */
  पूर्णांक addrFirst;        /* First instruction of interior of the loop */
  पूर्णांक addrBody;         /* Beginning of the body of this loop */
  पूर्णांक regBignull;       /* big-null flag reg. True if a NULL-scan is needed */
  पूर्णांक addrBignull;      /* Jump here for next part of big-null scan */
#अगर_अघोषित SQLITE_LIKE_DOESNT_MATCH_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  पूर्णांक addrLikeRep;      /* LIKE range processing address */
#पूर्ण_अगर
  पूर्णांक regFilter;        /* Bloom filter */
  Where_दायाँ_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  पूर्णांक p1, p2;           /* Operands of the opcode used to end the loop */
  जोड़ {               /* Information that depends on pWLoop->wsFlags */
    काष्ठा {
      पूर्णांक nIn;              /* Number of entries in aInLoop[] */
      काष्ठा InLoop {
        पूर्णांक iCur;              /* The VDBE cursor used by this IN operator */
        पूर्णांक addr_In_शीर्ष;         /* Top of the IN loop */
        पूर्णांक iBase;             /* Base register of multi-key index record */
        पूर्णांक nPrefix;           /* Number of prior entries in the key */
        u8 eEndLoopOp;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *aInLoop;           /* Information about each nested IN operator */
    } in;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    सूचकांक *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  काष्ठा WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णांक addrVisit;        /* Address at which row is visited */
#पूर्ण_अगर
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
काष्ठा WhereLoop {
  Bitmask prereq;       /* Bitmask of other loops that must run first */
  Bitmask maskSelf;     /* Bitmask identifying table iTab */
#अगर_घोषित SQLITE_DEBUG
  अक्षर cId;             /* Symbolic ID of this loop for debugging use */
#पूर्ण_अगर
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 iSortIdx;          /* Sorting index number.  0==None */
  LogEst rSetup;        /* One-time setup cost (ex: create transient index) */
  LogEst rRun;          /* Cost of running each loop */
  LogEst nOut;          /* Estimated number of output rows */
  जोड़ {
    काष्ठा {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_शीर्ष;              /* Size of TOP vector */
      u16 nDistinctCol;      /* Index columns used to sort for DISTINCT */
      सूचकांक *p_सूचकांक;         /* Index used, or NULL */
      व्यंजक_List *pOrderBy;    /* ORDER BY clause if this is really a subquery */
    } btree;
    काष्ठा {               /* Information for virtual tables */
      पूर्णांक idx_अंक;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 bOmitOffset : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_अंक_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omitMask;          /* Terms that may be omitted */
      अक्षर *idxStr;          /* Index identifier string */
      u32 mHandleIn;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ दुरत्व(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#अगर_घोषित WHERETRACE_ENABLED
  LogEst rStarDelta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#पूर्ण_अगर
  WhereTerm **aLTerm;   /* WhereTerms used */
  WhereLoop *p_अग्र_Loop; /* Next WhereLoop object in the WhereClause */
  WhereTerm *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
काष्ठा WhereOrCost {
  Bitmask prereq;     /* Prerequisites */
  LogEst rRun;        /* Cost of running this subquery */
  LogEst nOut;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#घोषणा N_OR_COST 3
काष्ठा WhereOrSet {
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
काष्ठा WherePath {
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
काष्ठा WhereTerm {
  व्यंजक *p_व्यंजक;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 nChild;              /* Number of children that must disable us */
  u8 eMatchOp;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  पूर्णांक iParent;            /* Disable pWC->a[iParent] when this term disabled */
  पूर्णांक बायाँ_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#अगर_घोषित SQLITE_DEBUG
  पूर्णांक iTerm;              /* Which WhereTerm is this, for debug purposes */
#पूर्ण_अगर
  जोड़ {
    काष्ठा {
      पूर्णांक बायाँ_Column;         /* Column number of X in "X <op> <expr>" */
      पूर्णांक iField;             /* Field in (?,?,?) IN (SELECT...) vector */
    } x;                    /* Opcode other than OP_OR or OP_AND */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  Bitmask prereq_दायाँ;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#घोषणा TERM_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#घोषणा TERM_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#घोषणा TERM_CODED      0x0004 /* This term is already coded */
#घोषणा TERM_COPIED     0x0008 /* Has a child */
#घोषणा TERM_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#घोषणा TERM_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#घोषणा TERM_OK         0x0040 /* Used during OR-clause processing */
#घोषणा TERM_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#घोषणा TERM_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#घोषणा TERM_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#घोषणा TERM_LIKE       0x0400 /* The original LIKE operator */
#घोषणा TERM_IS         0x0800 /* Term.pExpr is an IS operator */
#घोषणा TERM_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#घोषणा TERM_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#अगर_घोषित SQLITE_ENABLE_STAT4
#  define TERM_HIGHTRUTH  0x4000 /* Term excludes few rows */
#अन्यथा
#  define TERM_HIGHTRUTH  0      /* Only used with STAT4 */
#पूर्ण_अगर
#घोषणा TERM_SLICE      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
काष्ठा WhereScan {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  स्थिर अक्षर *zCollName;     /* Required collating sequence, if not NULL */
  व्यंजक *p_Idx_व्यंजक;            /* Search for this index expression */
  पूर्णांक k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 opMask;                /* Acceptable operators */
  अक्षर idxaff;               /* Must match this affinity, if zCollName!=NULL */
  अचिन्हित अक्षर iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  अचिन्हित अक्षर nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
  पूर्णांक aiCur[11];             /* Cursors in the equivalence class */
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
काष्ठा WhereClause {
  WhereInfo *pWInfo;       /* WHERE clause processing context */
  WhereClause *pOuter;     /* Outer conjunction */
  u8 op;                   /* Split operator.  TK_AND or TK_OR */
  u8 hasOr;                /* True if any a[].eOperator is WO_OR */
  पूर्णांक nTerm;               /* Number of terms */
  पूर्णांक nSlot;               /* Number of entries in a[] */
  पूर्णांक nBase;               /* Number of terms through the last non-Virtual */
  WhereTerm *a;            /* Each a[] describes a term of the WHERE clause */
#अगर defined(SQLITE_SMALL_ढेर)
  WhereTerm aStatic[1];    /* Initial static space for a[] */
#अन्यथा
  WhereTerm aStatic[8];    /* Initial static space for a[] */
#पूर्ण_अगर
};

/*
** A WhereTerm with eOperator==WO_OR has its u.pOrInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
काष्ठा WhereOrInfo {
  WhereClause wc;          /* Decomposition into subterms */
  Bitmask indexable;       /* Bitmask of all indexable tables in the clause */
};

/*
** A WhereTerm with eOperator==WO_AND has its u.pAndInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
काष्ठा WhereAndInfo {
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
काष्ठा WhereMaskSet {
  पूर्णांक bVarSelect;               /* Used by sqlite3WhereExprUsage() */
  पूर्णांक n;                        /* Number of assigned cursor values */
  पूर्णांक ix[BMS];                  /* Cursor assigned to each bit */
};

/*
** This object is a convenience wrapper holding all information needed
** to construct WhereLoop objects for a particular query.
*/
काष्ठा WhereLoopBuilder {
  WhereInfo *pWInfo;        /* Information about this WHERE */
  WhereClause *pWC;         /* WHERE clause terms */
  WhereLoop *pNew;          /* Template WhereLoop */
  WhereOrSet *pOrSet;       /* Record best loops here, if not NULL */
#अगर_घोषित SQLITE_ENABLE_STAT4
  UnpackedRecord *pRec;     /* Probe for stat4 (if required) */
  पूर्णांक nRecValid;            /* Number of valid fields currently in pRec */
#पूर्ण_अगर
  अचिन्हित अक्षर bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  अचिन्हित अक्षर bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  अचिन्हित पूर्णांक iPlanLimit;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#घोषणा SQLITE_BLDF1_INDEXED  0x0001   /* An index is used */
#घोषणा SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#घोषणा SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

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
#अगर_अघोषित SQLITE_QUERY_PLANNER_LIMIT
# define SQLITE_QUERY_PLANNER_LIMIT 20000
#पूर्ण_अगर
#अगर_अघोषित SQLITE_QUERY_PLANNER_LIMIT_INCR
# define SQLITE_QUERY_PLANNER_LIMIT_INCR 1000
#पूर्ण_अगर

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
काष्ठा WhereInfo {
  Parse *pParse;            /* Parsing and code generating context */
  SrcList *pTabList;        /* List of tables in the join */
  व्यंजक_List *pOrderBy;       /* The ORDER BY clause or NULL */
  व्यंजक_List *p_परिणाम_Set;     /* Result set of the query */
#अगर WHERETRACE_ENABLED
  व्यंजक *pWhere;             /* The complete WHERE clause */
#पूर्ण_अगर
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  पूर्णांक aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  पूर्णांक iContinue;            /* Jump here to continue with next record */
  पूर्णांक iBreak;               /* Jump here to break out of the loop */
  पूर्णांक savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst iLimit;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  अचिन्हित bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  अचिन्हित untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  अचिन्हित bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  अचिन्हित sorted        :1;   /* True if really sorted (not just grouped) */
  अचिन्हित bStarDone     :1;   /* True if check for star-query is complete */
  अचिन्हित bStarUsed     :1;   /* True if star-query heuristic is used */
  LogEst nRowOut;           /* Estimated number of output rows */
#अगर_घोषित WHERETRACE_ENABLED
  LogEst r_योग_Cost;        /* Total cost of the solution */
#पूर्ण_अगर
  पूर्णांक i_शीर्ष_2;                 /* The very beginning of the WHERE loop */
  पूर्णांक iEndWhere;            /* End of the WHERE clause itself */
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
#घोषणा SZ_WHEREINFO(N) ROUND8(दुरत्व(WhereInfo,a)+(N)*माप(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3WhereGetMask(WhereMaskSet*,पूर्णांक);
#अगर_घोषित WHERETRACE_ENABLED
व्योम sqlite3WhereClausePrint(WhereClause *pWC);
व्योम sqlite3WhereTermPrint(WhereTerm *pTerm, पूर्णांक iTerm);
व्योम sqlite3WhereLoopPrint(स्थिर WhereLoop *p, स्थिर WhereClause *pWC);
#पूर्ण_अगर
WhereTerm *sqlite3WhereFindTerm(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  पूर्णांक iCur,             /* Cursor number of LHS */
  पूर्णांक iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  सूचकांक *pIdx           /* Must be compatible with this index, if not NULL */
);
व्योम *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
व्योम *sqlite3WhereRealloc(WhereInfo *pWInfo, व्योम *pOld, u64 nByte);

/* wherecode.c: */
#अगर_अघोषित SQLITE_OMIT_EXPLAIN
पूर्णांक sqlite3WhereExplainOneScan(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
पूर्णांक sqlite3WhereExplainBloomFilter(
  स्थिर Parse *pParse,            /* Parse context */
  स्थिर WhereInfo *pWInfo,        /* WHERE clause */
  स्थिर WhereLevel *pLevel        /* Bloom filter on this level */
);
व्योम sqlite3WhereAddExplainText(
  Parse *pParse,                  /* Parse context */
  पूर्णांक addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#अन्यथा
# define sqlite3WhereExplainOneScan(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3WhereAddExplainText(u,v,w,x,y)
#पूर्ण_अगर /* SQLITE_OMIT_EXPLAIN */
#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
व्योम sqlite3WhereAddScanStatus(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  पूर्णांक addrExplain                 /* Address of OP_Explain (or 0) */
);
#अन्यथा
# define sqlite3WhereAddScanStatus(a, b, c, d) ((व्योम)d)
#पूर्ण_अगर
Bitmask sqlite3WhereCodeOneLoopStart(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  पूर्णांक iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
);
SQLITE_NOINLINE व्योम sqlite3_Where_दायाँ_Join_Loop(
  WhereInfo *pWInfo,
  पूर्णांक iLevel,
  WhereLevel *pLevel
);

/* whereexpr.c: */
व्योम sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
व्योम sqlite3WhereClauseClear(WhereClause*);
व्योम sqlite3WhereSplit(WhereClause*,व्यंजक*,u8);
व्योम sqlite3WhereAddLimit(WhereClause*, Select*);
Bitmask sqlite3_Where_व्यंजक_Usage(WhereMaskSet*, व्यंजक*);
Bitmask sqlite3_Where_व्यंजक_Usage_NN(WhereMaskSet*, व्यंजक*);
Bitmask sqlite3_Where_व्यंजक_List_Usage(WhereMaskSet*, व्यंजक_List*);
व्योम sqlite3_Where_व्यंजक_Analyze(SrcList*, WhereClause*);
व्योम sqlite3WhereTabFuncArgs(Parse*, SrcItem*, WhereClause*);
पूर्णांक sqlite3WhereLoopBloomable(स्थिर WhereLoop*);

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
#घोषणा WO_IN     0x0001
#घोषणा WO_EQ     0x0002
#घोषणा WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#घोषणा WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#घोषणा WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#घोषणा WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#घोषणा WO_AUX    0x0040       /* Op useful to virtual tables only */
#घोषणा WO_IS     0x0080
#घोषणा WO_ISNULL 0x0100
#घोषणा WO_OR     0x0200       /* Two or more OR-connected terms */
#घोषणा WO_AND    0x0400       /* Two or more AND-connected terms */
#घोषणा WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#घोषणा WO_NOOP   0x1000       /* This term does not restrict search space */
#घोषणा WO_ROWVAL 0x2000       /* A row-value term */

#घोषणा WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#घोषणा WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#घोषणा WHERE_COLUMN_EQ    0x00000001  /* x=EXPR */
#घोषणा WHERE_COLUMN_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#घोषणा WHERE_COLUMN_IN    0x00000004  /* x IN (...) */
#घोषणा WHERE_COLUMN_NULL  0x00000008  /* x IS NULL */
#घोषणा WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#घोषणा WHERE_शीर्ष_LIMIT    0x00000010  /* x<EXPR or x<=EXPR constraint */
#घोषणा WHERE_BTM_LIMIT    0x00000020  /* x>EXPR or x>=EXPR constraint */
#घोषणा WHERE_BOTH_LIMIT   0x00000030  /* Both x>EXPR and x<EXPR */
#घोषणा WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#घोषणा WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#घोषणा WHERE_INDEXED      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#घोषणा WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#घोषणा WHERE_IN_ABLE      0x00000800  /* Able to support an IN operator */
#घोषणा WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#घोषणा WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#घोषणा WHERE_AUTO_सूचकांक   0x00004000  /* Uses an ephemeral index */
#घोषणा WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#घोषणा WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#घोषणा WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#घोषणा WHERE_IN_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#घोषणा WHERE_BIGNULL_SORT 0x00080000  /* Column nEq of index is BIGNULL */
#घोषणा WHERE_IN_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#घोषणा WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#घोषणा WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#घोषणा WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#घोषणा WHERE_OMIT_OFFSET  0x01000000  /* Set offset counter to zero */
#घोषणा WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#घोषणा WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#पूर्ण_अगर /* !defined(SQLITE_WHEREINT_H) */
