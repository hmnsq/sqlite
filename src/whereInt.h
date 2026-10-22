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
#यदि_अपरिभाषितम् SQLITE_WHEREINT_H
#परिभाषय SQLITE_WHEREINT_H


/* Forward references
*/
प्रकारनिर्देश संरचना WhereClause WhereClause;
प्रकारनिर्देश संरचना Where_आवरणम्_समुच्चयः Where_आवरणम्_समुच्चयः;
प्रकारनिर्देश संरचना WhereOrInfo WhereOrInfo;
प्रकारनिर्देश संरचना WhereAndInfo WhereAndInfo;
प्रकारनिर्देश संरचना WhereLevel WhereLevel;
प्रकारनिर्देश संरचना WhereLoop WhereLoop;
प्रकारनिर्देश संरचना Where_मार्गः Where_मार्गः;
प्रकारनिर्देश संरचना Where_पदांशः Where_पदांशः;
प्रकारनिर्देश संरचना WhereLoopBuilder WhereLoopBuilder;
प्रकारनिर्देश संरचना Where_अवलोकनम् Where_अवलोकनम्;
प्रकारनिर्देश संरचना WhereOrCost WhereOrCost;
प्रकारनिर्देश संरचना Where_Or_समुच्चयः Where_Or_समुच्चयः;
प्रकारनिर्देश संरचना Where_Mem_खण्डम् Where_Mem_खण्डम्;
प्रकारनिर्देश संरचना Where_दक्षिणः_Join Where_दक्षिणः_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
संरचना Where_Mem_खण्डम् {
  Where_Mem_खण्डम् *p_अग्र;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
संरचना Where_दक्षिणः_Join {
  पूर्णाङ्क i_मेलनम्;          /* Cursor used to determine prior matched rows */
  पूर्णाङ्क regBloom;        /* Bloom filter for iRJMatch */
  पूर्णाङ्क regReturn;       /* Return register for the interior subroutine */
  पूर्णाङ्क addrSubrtn;      /* Starting address for the interior subroutine */
  पूर्णाङ्क अन्तः_Subrtn;       /* The last opcode in the interior subroutine */
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
#यदि_अपरिभाषितम् SQLITE_LIKE_DOESNT_मेलनम्_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  पूर्णाङ्क addrLikeRep;      /* LIKE range processing address */
#यदि_अन्तः
  पूर्णाङ्क regFilter;        /* Bloom filter */
  Where_दक्षिणः_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  पूर्णाङ्क p1, p2;           /* Operands of the opcode used to end the loop */
  सङ्घ {               /* Information that depends on pWLoop->wsFlags */
    संरचना {
      पूर्णाङ्क n_अन्तर्;              /* Number of entries in aInLoop[] */
      संरचना अन्तर्_Loop {
        पूर्णाङ्क iCur;              /* The VDBE cursor used by this IN operator */
        पूर्णाङ्क addr_अन्तर्_शिखरम्;         /* Top of the IN loop */
        पूर्णाङ्क i_आधारः;             /* Base register of multi-key index record */
        पूर्णाङ्क n_उपसर्गः;           /* Number of prior entries in the key */
        u8 e_अन्तः_Loop_Op;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *a_अन्तर्_Loop;           /* Information about each nested IN operator */
    } अन्तर्;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    सूचकांक *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  संरचना WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णाङ्क addr_दर्शय;        /* Address at which row is visited */
#यदि_अन्तः
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
  Bitmask आवरणम्_Self;     /* Bitmask identifying table iTab */
#यदि_परिभाषितम् SQLITE_DEBUG
  वर्ण cId;             /* Symbolic ID of this loop for debugging use */
#यदि_अन्तः
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 i_क्रमय_Idx;          /* Sorting index number.  0==None */
  LogEst r_सज्जता;        /* One-time setup cost (ex: create transient index) */
  LogEst r_सततम्;          /* Cost of running each loop */
  LogEst n_बहिर्गमनम्;          /* Estimated number of output rows */
  सङ्घ {
    संरचना {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_शिखरम्;              /* Size of TOP vector */
      u16 nDistinctCol;      /* Index columns used to sort for DISTINCT */
      सूचकांक *p_सूचकांक;         /* Index used, or NULL */
      व्यञ्जकम्_List *p_क्रमः_By;    /* ORDER BY clause if this is really a subquery */
    } btree;
    संरचना {               /* Information for virtual tables */
      पूर्णाङ्क idx_अङ्कः;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 b_Omit_विस्थापनम् : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_अङ्कः_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omit_आवरणम्;          /* Terms that may be omitted */
      वर्ण *idxStr;          /* Index identifier string */
      u32 m_Handle_अन्तर्;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#यदि_परिभाषितम् WHERETRACE_ENABLED
  LogEst r_तारकम्_Delta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#यदि_अन्तः
  Where_पदांशः **aLTerm;   /* WhereTerms used */
  WhereLoop *p_अग्र_Loop; /* Next WhereLoop object in the WhereClause */
  Where_पदांशः *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
संरचना WhereOrCost {
  Bitmask prereq;     /* Prerequisites */
  LogEst r_सततम्;        /* Cost of running this subquery */
  LogEst n_बहिर्गमनम्;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#परिभाषय N_OR_COST 3
संरचना Where_Or_समुच्चयः {
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
संरचना Where_मार्गः {
  Bitmask आवरणम्_Loop;     /* Bitmask of all WhereLoop objects in this path */
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
संरचना Where_पदांशः {
  व्यञ्जकम् *p_व्यञ्जकम्;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 nChild;              /* Number of children that must disable us */
  u8 e_मेलनम्_Op;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  पूर्णाङ्क iParent;            /* Disable pWC->a[iParent] when this term disabled */
  पूर्णाङ्क वामः_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#यदि_परिभाषितम् SQLITE_DEBUG
  पूर्णाङ्क i_पदांशः;              /* Which WhereTerm is this, for debug purposes */
#यदि_अन्तः
  सङ्घ {
    संरचना {
      पूर्णाङ्क वामः_Column;         /* Column number of X in "X <op> <expr>" */
      पूर्णाङ्क i_क्षेत्रम्;             /* Field in (?,?,?) IN (SELECT...) vector */
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
#परिभाषय पदांशः_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#परिभाषय पदांशः_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#परिभाषय पदांशः_CODED      0x0004 /* This term is already coded */
#परिभाषय पदांशः_COPIED     0x0008 /* Has a child */
#परिभाषय पदांशः_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#परिभाषय पदांशः_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#परिभाषय पदांशः_OK         0x0040 /* Used during OR-clause processing */
#परिभाषय पदांशः_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#परिभाषय पदांशः_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#परिभाषय पदांशः_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#परिभाषय पदांशः_LIKE       0x0400 /* The original LIKE operator */
#परिभाषय पदांशः_IS         0x0800 /* Term.pExpr is an IS operator */
#परिभाषय पदांशः_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#परिभाषय पदांशः_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
#  define पदांशः_HIGHTRUTH  0x4000 /* Term excludes few rows */
#अन्यथा
#  define पदांशः_HIGHTRUTH  0      /* Only used with STAT4 */
#यदि_अन्तः
#परिभाषय पदांशः_खण्डिका      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
संरचना Where_अवलोकनम् {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  ध्रुव वर्ण *zCollName;     /* Required collating sequence, if not NULL */
  व्यञ्जकम् *p_Idx_व्यञ्जकम्;            /* Search for this index expression */
  पूर्णाङ्क k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 op_आवरणम्;                /* Acceptable operators */
  वर्ण idxaff;               /* Must match this affinity, if zCollName!=NULL */
  अचिह्नित वर्ण iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  अचिह्नित वर्ण nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
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
  पूर्णाङ्क n_पदांशः;               /* Number of terms */
  पूर्णाङ्क nSlot;               /* Number of entries in a[] */
  पूर्णाङ्क n_आधारः;               /* Number of terms through the last non-Virtual */
  Where_पदांशः *a;            /* Each a[] describes a term of the WHERE clause */
#यदि defined(SQLITE_SMALL_राशिः)
  Where_पदांशः aStatic[1];    /* Initial static space for a[] */
#अन्यथा
  Where_पदांशः aStatic[8];    /* Initial static space for a[] */
#यदि_अन्तः
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
संरचना Where_आवरणम्_समुच्चयः {
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
  Where_Or_समुच्चयः *p_Or_समुच्चयः;       /* Record best loops here, if not NULL */
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
  Unpacked_लेखः *pRec;     /* Probe for stat4 (if required) */
  पूर्णाङ्क nRecValid;            /* Number of valid fields currently in pRec */
#यदि_अन्तः
  अचिह्नित वर्ण bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  अचिह्नित वर्ण bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  अचिह्नित पूर्णाङ्क i_Plan_सीमा;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#परिभाषय SQLITE_BLDF1_अनुक्रमितम्  0x0001   /* An index is used */
#परिभाषय SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#परिभाषय SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

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
#यदि_अपरिभाषितम् SQLITE_QUERY_PLANNER_सीमा
# define SQLITE_QUERY_PLANNER_सीमा 20000
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_QUERY_PLANNER_सीमा_INCR
# define SQLITE_QUERY_PLANNER_सीमा_INCR 1000
#यदि_अन्तः

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
  व्यञ्जकम्_List *p_क्रमः_By;       /* The ORDER BY clause or NULL */
  व्यञ्जकम्_List *p_परिणाम_समुच्चयः;     /* Result set of the query */
#यदि WHERETRACE_ENABLED
  व्यञ्जकम् *pWhere;             /* The complete WHERE clause */
#यदि_अन्तः
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  पूर्णाङ्क aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  पूर्णाङ्क iContinue;            /* Jump here to continue with next record */
  पूर्णाङ्क iBreak;               /* Jump here to break out of the loop */
  पूर्णाङ्क savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst i_सीमा;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  अचिह्नित bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  अचिह्नित untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  अचिह्नित bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  अचिह्नित sorted        :1;   /* True if really sorted (not just grouped) */
  अचिह्नित b_तारकम्_समाप्तम्     :1;   /* True if check for star-query is complete */
  अचिह्नित b_तारकम्_प्रयुक्तम्     :1;   /* True if star-query heuristic is used */
  LogEst n_Row_बहिर्गमनम्;           /* Estimated number of output rows */
#यदि_परिभाषितम् WHERETRACE_ENABLED
  LogEst r_योग_Cost;        /* Total cost of the solution */
#यदि_अन्तः
  पूर्णाङ्क i_शिखरम्;                 /* The very beginning of the WHERE loop */
  पूर्णाङ्क i_अन्तः_Where;            /* End of the WHERE clause itself */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  Where_Mem_खण्डम् *p_Mem_गन्तव्यम्_Free;/* Memory to free when this object destroyed */
  Bitmask rev_आवरणम्;          /* Mask of ORDER BY terms that need reversing */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  Where_आवरणम्_समुच्चयः s_आवरणम्_समुच्चयः;    /* Map cursor numbers to bitmasks */
  WhereLevel a[FLEXARRAY];  /* Information about each nest loop in WHERE */
};

/*
** The size (in bytes) of a WhereInfo object that holds N WhereLevels.
*/
#परिभाषय SZ_WHEREINFO(N) ROUND8(offsetof(WhereInfo,a)+(N)*परिमाण(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3_Where_Get_आवरणम्(Where_आवरणम्_समुच्चयः*,पूर्णाङ्क);
#यदि_परिभाषितम् WHERETRACE_ENABLED
शून्य sqlite3WhereClausePrint(WhereClause *pWC);
शून्य sqlite3_Where_पदांशः_Print(Where_पदांशः *p_पदांशः, पूर्णाङ्क i_पदांशः);
शून्य sqlite3WhereLoopPrint(ध्रुव WhereLoop *p, ध्रुव WhereClause *pWC);
#यदि_अन्तः
Where_पदांशः *sqlite3_Where_Find_पदांशः(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  पूर्णाङ्क iCur,             /* Cursor number of LHS */
  पूर्णाङ्क iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  सूचकांक *pIdx           /* Must be compatible with this index, if not NULL */
);
शून्य *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
शून्य *sqlite3WhereRealloc(WhereInfo *pWInfo, शून्य *pOld, u64 nByte);

/* wherecode.c: */
#यदि_अपरिभाषितम् SQLITE_OMIT_EXPLAIN
पूर्णाङ्क sqlite3_Where_Explain_One_अवलोकनम्(
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
शून्य sqlite3_Where_सङ्कलय_Explain_Text(
  Parse *pParse,                  /* Parse context */
  पूर्णाङ्क addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#अन्यथा
# define sqlite3_Where_Explain_One_अवलोकनम्(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3_Where_सङ्कलय_Explain_Text(u,v,w,x,y)
#यदि_अन्तः /* SQLITE_OMIT_EXPLAIN */
#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
शून्य sqlite3_Where_सङ्कलय_अवलोकनम्_स्थितिः(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  पूर्णाङ्क addrExplain                 /* Address of OP_Explain (or 0) */
);
#अन्यथा
# define sqlite3_Where_सङ्कलय_अवलोकनम्_स्थितिः(a, b, c, d) ((शून्य)d)
#यदि_अन्तः
Bitmask sqlite3_Where_संहिता_One_Loop_Start(
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
शून्य sqlite3_Where_विभज(WhereClause*,व्यञ्जकम्*,u8);
शून्य sqlite3_Where_सङ्कलय_सीमा(WhereClause*, Select*);
Bitmask sqlite3_Where_व्यञ्जकम्_Usage(Where_आवरणम्_समुच्चयः*, व्यञ्जकम्*);
Bitmask sqlite3_Where_व्यञ्जकम्_Usage_NN(Where_आवरणम्_समुच्चयः*, व्यञ्जकम्*);
Bitmask sqlite3_Where_व्यञ्जकम्_List_Usage(Where_आवरणम्_समुच्चयः*, व्यञ्जकम्_List*);
शून्य sqlite3_Where_व्यञ्जकम्_Analyze(SrcList*, WhereClause*);
शून्य sqlite3_Where_Tab_Func_तर्काः(Parse*, SrcItem*, WhereClause*);
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
#परिभाषय WO_अन्तर्     0x0001
#परिभाषय WO_EQ     0x0002
#परिभाषय WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#परिभाषय WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#परिभाषय WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#परिभाषय WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#परिभाषय WO_AUX    0x0040       /* Op useful to virtual tables only */
#परिभाषय WO_IS     0x0080
#परिभाषय WO_ISNULL 0x0100
#परिभाषय WO_OR     0x0200       /* Two or more OR-connected terms */
#परिभाषय WO_AND    0x0400       /* Two or more AND-connected terms */
#परिभाषय WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#परिभाषय WO_NOOP   0x1000       /* This term does not restrict search space */
#परिभाषय WO_ROWVAL 0x2000       /* A row-value term */

#परिभाषय WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#परिभाषय WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#परिभाषय WHERE_COLUMN_EQ    0x00000001  /* x=EXPR */
#परिभाषय WHERE_COLUMN_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#परिभाषय WHERE_COLUMN_अन्तर्    0x00000004  /* x IN (...) */
#परिभाषय WHERE_COLUMN_NULL  0x00000008  /* x IS NULL */
#परिभाषय WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#परिभाषय WHERE_शिखरम्_सीमा    0x00000010  /* x<EXPR or x<=EXPR constraint */
#परिभाषय WHERE_BTM_सीमा    0x00000020  /* x>EXPR or x>=EXPR constraint */
#परिभाषय WHERE_BOTH_सीमा   0x00000030  /* Both x>EXPR and x<EXPR */
#परिभाषय WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#परिभाषय WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#परिभाषय WHERE_अनुक्रमितम्      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#परिभाषय WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#परिभाषय WHERE_अन्तर्_ABLE      0x00000800  /* Able to support an IN operator */
#परिभाषय WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#परिभाषय WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#परिभाषय WHERE_AUTO_सूचकांक   0x00004000  /* Uses an ephemeral index */
#परिभाषय WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#परिभाषय WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#परिभाषय WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#परिभाषय WHERE_अन्तर्_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#परिभाषय WHERE_BIGNULL_क्रमय 0x00080000  /* Column nEq of index is BIGNULL */
#परिभाषय WHERE_अन्तर्_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#परिभाषय WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#परिभाषय WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#परिभाषय WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#परिभाषय WHERE_OMIT_विस्थापनम्  0x01000000  /* Set offset counter to zero */
#परिभाषय WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#परिभाषय WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#यदि_अन्तः /* !defined(SQLITE_WHEREINT_H) */
