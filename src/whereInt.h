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
#నిర్వచితం_కాకపోతే SQLITE_WHEREINT_H
#నిర్వచించు SQLITE_WHEREINT_H


/* Forward references
*/
రకనిర్వచనం నిర్మాణం WhereClause WhereClause;
రకనిర్వచనం నిర్మాణం Where_ముసుగు_సమితి Where_ముసుగు_సమితి;
రకనిర్వచనం నిర్మాణం WhereOrInfo WhereOrInfo;
రకనిర్వచనం నిర్మాణం WhereAndInfo WhereAndInfo;
రకనిర్వచనం నిర్మాణం WhereLevel WhereLevel;
రకనిర్వచనం నిర్మాణం WhereLoop WhereLoop;
రకనిర్వచనం నిర్మాణం Where_మార్గం Where_మార్గం;
రకనిర్వచనం నిర్మాణం Where_పదం Where_పదం;
రకనిర్వచనం నిర్మాణం WhereLoopBuilder WhereLoopBuilder;
రకనిర్వచనం నిర్మాణం Where_పరిశీలన Where_పరిశీలన;
రకనిర్వచనం నిర్మాణం Where_Or_ఖర్చు Where_Or_ఖర్చు;
రకనిర్వచనం నిర్మాణం Where_Or_సమితి Where_Or_సమితి;
రకనిర్వచనం నిర్మాణం Where_Mem_ఖండం Where_Mem_ఖండం;
రకనిర్వచనం నిర్మాణం Where_కుడి_Join Where_కుడి_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
నిర్మాణం Where_Mem_ఖండం {
  Where_Mem_ఖండం *p_తదుపరి;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
నిర్మాణం Where_కుడి_Join {
  పూర్ణాంకం i_సరిపోలిక;          /* Cursor used to determine prior matched rows */
  పూర్ణాంకం regBloom;        /* Bloom filter for iRJMatch */
  పూర్ణాంకం regReturn;       /* Return register for the interior subroutine */
  పూర్ణాంకం addrSubrtn;      /* Starting address for the interior subroutine */
  పూర్ణాంకం చివర_Subrtn;       /* The last opcode in the interior subroutine */
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
నిర్మాణం WhereLevel {
  పూర్ణాంకం i_ఎడమ_Join;        /* Memory cell used to implement LEFT OUTER JOIN */
  పూర్ణాంకం iTabCur;          /* The VDBE cursor used to access the table */
  పూర్ణాంకం iIdxCur;          /* The VDBE cursor used to access pIdx */
  పూర్ణాంకం addrBrk;          /* Jump here to break out of the loop */
  పూర్ణాంకం addrHalt;         /* Abort the query due to empty table or similar */
  పూర్ణాంకం addrNxt;          /* Jump here to start the next IN combination */
  పూర్ణాంకం addrSkip;         /* Jump here for next iteration of skip-scan */
  పూర్ణాంకం addrCont;         /* Jump here to continue with the next loop cycle */
  పూర్ణాంకం addrDeleteCont;   /* Delete auto-index entry then same as addrCont */
  పూర్ణాంకం addr_మొదటి;        /* First instruction of interior of the loop */
  పూర్ణాంకం addrBody;         /* Beginning of the body of this loop */
  పూర్ణాంకం regBignull;       /* big-null flag reg. True if a NULL-scan is needed */
  పూర్ణాంకం addrBignull;      /* Jump here for next part of big-null scan */
#నిర్వచితం_కాకపోతే SQLITE_LIKE_DOESNT_సరిపోలిక_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  పూర్ణాంకం addrLikeRep;      /* LIKE range processing address */
#అయితే_ముగింపు
  పూర్ణాంకం regFilter;        /* Bloom filter */
  Where_కుడి_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  పూర్ణాంకం p1, p2;           /* Operands of the opcode used to end the loop */
  సంఘం {               /* Information that depends on pWLoop->wsFlags */
    నిర్మాణం {
      పూర్ణాంకం n_లోపల;              /* Number of entries in aInLoop[] */
      నిర్మాణం లోపల_Loop {
        పూర్ణాంకం iCur;              /* The VDBE cursor used by this IN operator */
        పూర్ణాంకం addr_లోపల_శిఖరం;         /* Top of the IN loop */
        పూర్ణాంకం i_పునాది;             /* Base register of multi-key index record */
        పూర్ణాంకం n_ఉపసర్గ;           /* Number of prior entries in the key */
        u8 e_చివర_Loop_Op;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *a_లోపల_Loop;           /* Information about each nested IN operator */
    } లోపల;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    సూచీ *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  నిర్మాణం WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  పూర్ణాంకం addr_సందర్శించు;        /* Address at which row is visited */
#అయితే_ముగింపు
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
నిర్మాణం WhereLoop {
  Bitmask prereq;       /* Bitmask of other loops that must run first */
  Bitmask ముసుగు_Self;     /* Bitmask identifying table iTab */
#నిర్వచితమైతే SQLITE_DEBUG
  అక్షరం cId;             /* Symbolic ID of this loop for debugging use */
#అయితే_ముగింపు
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 i_క్రమీకరించు_Idx;          /* Sorting index number.  0==None */
  LogEst r_అమరిక;        /* One-time setup cost (ex: create transient index) */
  LogEst r_నిరంతరం;          /* Cost of running each loop */
  LogEst n_బహిర్గతం;          /* Estimated number of output rows */
  సంఘం {
    నిర్మాణం {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_శిఖరం;              /* Size of TOP vector */
      u16 n_Distinct_నిలువు;      /* Index columns used to sort for DISTINCT */
      సూచీ *p_సూచీ;         /* Index used, or NULL */
      సమాసం_List *p_క్రమం_By;    /* ORDER BY clause if this is really a subquery */
    } btree;
    నిర్మాణం {               /* Information for virtual tables */
      పూర్ణాంకం idx_అంకె;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 b_Omit_స్థానభ్రంశం : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_అంకె_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omit_ముసుగు;          /* Terms that may be omitted */
      అక్షరం *idxStr;          /* Index identifier string */
      u32 m_Handle_లోపల;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#నిర్వచితమైతే WHERETRACE_ENABLED
  LogEst r_నక్షత్రం_Delta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#అయితే_ముగింపు
  Where_పదం **aLTerm;   /* WhereTerms used */
  WhereLoop *p_తదుపరి_Loop; /* Next WhereLoop object in the WhereClause */
  Where_పదం *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
నిర్మాణం Where_Or_ఖర్చు {
  Bitmask prereq;     /* Prerequisites */
  LogEst r_నిరంతరం;        /* Cost of running this subquery */
  LogEst n_బహిర్గతం;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#నిర్వచించు N_OR_ఖర్చు 3
నిర్మాణం Where_Or_సమితి {
  u16 n;                      /* Number of valid a[] entries */
  Where_Or_ఖర్చు a[N_OR_ఖర్చు];   /* Set of best costs */
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
నిర్మాణం Where_మార్గం {
  Bitmask ముసుగు_Loop;     /* Bitmask of all WhereLoop objects in this path */
  Bitmask revLoop;      /* aLoop[]s that should be reversed for ORDER BY */
  LogEst n_అడ్డువరుస;          /* Estimated number of rows generated by this path */
  LogEst r_ఖర్చు;         /* Total cost of this path */
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
నిర్మాణం Where_పదం {
  సమాసం *p_సమాసం;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 n_బిడ్డ;              /* Number of children that must disable us */
  u8 e_సరిపోలిక_Op;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  పూర్ణాంకం i_మాతృ;            /* Disable pWC->a[iParent] when this term disabled */
  పూర్ణాంకం ఎడమ_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#నిర్వచితమైతే SQLITE_DEBUG
  పూర్ణాంకం i_పదం;              /* Which WhereTerm is this, for debug purposes */
#అయితే_ముగింపు
  సంఘం {
    నిర్మాణం {
      పూర్ణాంకం ఎడమ_స్తంభం;         /* Column number of X in "X <op> <expr>" */
      పూర్ణాంకం i_క్షేత్రం;             /* Field in (?,?,?) IN (SELECT...) vector */
    } x;                    /* Opcode other than OP_OR or OP_AND */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  Bitmask prereq_కుడి;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#నిర్వచించు పదం_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#నిర్వచించు పదం_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#నిర్వచించు పదం_CODED      0x0004 /* This term is already coded */
#నిర్వచించు పదం_COPIED     0x0008 /* Has a child */
#నిర్వచించు పదం_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#నిర్వచించు పదం_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#నిర్వచించు పదం_OK         0x0040 /* Used during OR-clause processing */
#నిర్వచించు పదం_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#నిర్వచించు పదం_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#నిర్వచించు పదం_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#నిర్వచించు పదం_LIKE       0x0400 /* The original LIKE operator */
#నిర్వచించు పదం_IS         0x0800 /* Term.pExpr is an IS operator */
#నిర్వచించు పదం_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#నిర్వచించు పదం_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
#  define పదం_HIGHTRUTH  0x4000 /* Term excludes few rows */
#లేకపోతే
#  define పదం_HIGHTRUTH  0      /* Only used with STAT4 */
#అయితే_ముగింపు
#నిర్వచించు పదం_ముక్క      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
నిర్మాణం Where_పరిశీలన {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  స్థిరరాశి అక్షరం *zCollName;     /* Required collating sequence, if not NULL */
  సమాసం *p_Idx_సమాసం;            /* Search for this index expression */
  పూర్ణాంకం k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 op_ముసుగు;                /* Acceptable operators */
  అక్షరం idxaff;               /* Must match this affinity, if zCollName!=NULL */
  గుర్తులేని అక్షరం iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  గుర్తులేని అక్షరం nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
  పూర్ణాంకం aiCur[11];             /* Cursors in the equivalence class */
  i16 ai_స్తంభం[11];          /* Corresponding column number in the eq-class */
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
నిర్మాణం WhereClause {
  WhereInfo *pWInfo;       /* WHERE clause processing context */
  WhereClause *pOuter;     /* Outer conjunction */
  u8 op;                   /* Split operator.  TK_AND or TK_OR */
  u8 hasOr;                /* True if any a[].eOperator is WO_OR */
  పూర్ణాంకం n_పదం;               /* Number of terms */
  పూర్ణాంకం n_స్థానం;               /* Number of entries in a[] */
  పూర్ణాంకం n_పునాది;               /* Number of terms through the last non-Virtual */
  Where_పదం *a;            /* Each a[] describes a term of the WHERE clause */
#ఒకవేళ defined(SQLITE_SMALL_దొంతర)
  Where_పదం aStatic[1];    /* Initial static space for a[] */
#లేకపోతే
  Where_పదం aStatic[8];    /* Initial static space for a[] */
#అయితే_ముగింపు
};

/*
** A WhereTerm with eOperator==WO_OR has its u.pOrInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
నిర్మాణం WhereOrInfo {
  WhereClause wc;          /* Decomposition into subterms */
  Bitmask indexable;       /* Bitmask of all indexable tables in the clause */
};

/*
** A WhereTerm with eOperator==WO_AND has its u.pAndInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
నిర్మాణం WhereAndInfo {
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
నిర్మాణం Where_ముసుగు_సమితి {
  పూర్ణాంకం bVarSelect;               /* Used by sqlite3WhereExprUsage() */
  పూర్ణాంకం n;                        /* Number of assigned cursor values */
  పూర్ణాంకం ix[BMS];                  /* Cursor assigned to each bit */
};

/*
** This object is a convenience wrapper holding all information needed
** to construct WhereLoop objects for a particular query.
*/
నిర్మాణం WhereLoopBuilder {
  WhereInfo *pWInfo;        /* Information about this WHERE */
  WhereClause *pWC;         /* WHERE clause terms */
  WhereLoop *pNew;          /* Template WhereLoop */
  Where_Or_సమితి *p_Or_సమితి;       /* Record best loops here, if not NULL */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  Unpacked_రికార్డు *pRec;     /* Probe for stat4 (if required) */
  పూర్ణాంకం nRecValid;            /* Number of valid fields currently in pRec */
#అయితే_ముగింపు
  గుర్తులేని అక్షరం bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  గుర్తులేని అక్షరం bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  గుర్తులేని పూర్ణాంకం i_Plan_హద్దు;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#నిర్వచించు SQLITE_BLDF1_సూచీకృత  0x0001   /* An index is used */
#నిర్వచించు SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#నిర్వచించు SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

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
#నిర్వచితం_కాకపోతే SQLITE_QUERY_PLANNER_హద్దు
# define SQLITE_QUERY_PLANNER_హద్దు 20000
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_QUERY_PLANNER_హద్దు_INCR
# define SQLITE_QUERY_PLANNER_హద్దు_INCR 1000
#అయితే_ముగింపు

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
నిర్మాణం WhereInfo {
  Parse *pParse;            /* Parsing and code generating context */
  SrcList *pTabList;        /* List of tables in the join */
  సమాసం_List *p_క్రమం_By;       /* The ORDER BY clause or NULL */
  సమాసం_List *p_ఫలితం_సమితి;     /* Result set of the query */
#ఒకవేళ WHERETRACE_ENABLED
  సమాసం *pWhere;             /* The complete WHERE clause */
#అయితే_ముగింపు
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  పూర్ణాంకం aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  పూర్ణాంకం iContinue;            /* Jump here to continue with next record */
  పూర్ణాంకం iBreak;               /* Jump here to break out of the loop */
  పూర్ణాంకం savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst i_హద్దు;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  గుర్తులేని bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  గుర్తులేని untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  గుర్తులేని bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  గుర్తులేని sorted        :1;   /* True if really sorted (not just grouped) */
  గుర్తులేని b_నక్షత్రం_పూర్తి     :1;   /* True if check for star-query is complete */
  గుర్తులేని b_నక్షత్రం_వాడిన     :1;   /* True if star-query heuristic is used */
  LogEst n_అడ్డువరుస_బహిర్గతం;           /* Estimated number of output rows */
#నిర్వచితమైతే WHERETRACE_ENABLED
  LogEst r_మొత్తం_ఖర్చు;        /* Total cost of the solution */
#అయితే_ముగింపు
  పూర్ణాంకం i_శిఖరం;                 /* The very beginning of the WHERE loop */
  పూర్ణాంకం i_చివర_Where;            /* End of the WHERE clause itself */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  Where_Mem_ఖండం *p_Mem_గమ్యం_Free;/* Memory to free when this object destroyed */
  Bitmask rev_ముసుగు;          /* Mask of ORDER BY terms that need reversing */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  Where_ముసుగు_సమితి s_ముసుగు_సమితి;    /* Map cursor numbers to bitmasks */
  WhereLevel a[FLEXARRAY];  /* Information about each nest loop in WHERE */
};

/*
** The size (in bytes) of a WhereInfo object that holds N WhereLevels.
*/
#నిర్వచించు SZ_WHEREINFO(N) ROUND8(offsetof(WhereInfo,a)+(N)*పరిమాణం(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3_Where_Get_ముసుగు(Where_ముసుగు_సమితి*,పూర్ణాంకం);
#నిర్వచితమైతే WHERETRACE_ENABLED
శూన్య sqlite3WhereClausePrint(WhereClause *pWC);
శూన్య sqlite3_Where_పదం_Print(Where_పదం *p_పదం, పూర్ణాంకం i_పదం);
శూన్య sqlite3WhereLoopPrint(స్థిరరాశి WhereLoop *p, స్థిరరాశి WhereClause *pWC);
#అయితే_ముగింపు
Where_పదం *sqlite3_Where_కనుగొను_పదం(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  పూర్ణాంకం iCur,             /* Cursor number of LHS */
  పూర్ణాంకం i_స్తంభం,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  సూచీ *pIdx           /* Must be compatible with this index, if not NULL */
);
శూన్య *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
శూన్య *sqlite3WhereRealloc(WhereInfo *pWInfo, శూన్య *pOld, u64 nByte);

/* wherecode.c: */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN
పూర్ణాంకం sqlite3_Where_Explain_One_పరిశీలన(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
పూర్ణాంకం sqlite3WhereExplainBloomFilter(
  స్థిరరాశి Parse *pParse,            /* Parse context */
  స్థిరరాశి WhereInfo *pWInfo,        /* WHERE clause */
  స్థిరరాశి WhereLevel *pLevel        /* Bloom filter on this level */
);
శూన్య sqlite3_Where_కూడు_Explain_పాఠ్యం(
  Parse *pParse,                  /* Parse context */
  పూర్ణాంకం addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#లేకపోతే
# define sqlite3_Where_Explain_One_పరిశీలన(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3_Where_కూడు_Explain_పాఠ్యం(u,v,w,x,y)
#అయితే_ముగింపు /* SQLITE_OMIT_EXPLAIN */
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
శూన్య sqlite3_Where_కూడు_పరిశీలన_స్థితి(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  పూర్ణాంకం addrExplain                 /* Address of OP_Explain (or 0) */
);
#లేకపోతే
# define sqlite3_Where_కూడు_పరిశీలన_స్థితి(a, b, c, d) ((శూన్య)d)
#అయితే_ముగింపు
Bitmask sqlite3_Where_సంకేతం_One_Loop_ప్రారంభం(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  పూర్ణాంకం iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
);
SQLITE_NOINLINE శూన్య sqlite3_Where_కుడి_Join_Loop(
  WhereInfo *pWInfo,
  పూర్ణాంకం iLevel,
  WhereLevel *pLevel
);

/* whereexpr.c: */
శూన్య sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
శూన్య sqlite3WhereClauseClear(WhereClause*);
శూన్య sqlite3_Where_విభజించు(WhereClause*,సమాసం*,u8);
శూన్య sqlite3_Where_కూడు_హద్దు(WhereClause*, Select*);
Bitmask sqlite3_Where_సమాసం_Usage(Where_ముసుగు_సమితి*, సమాసం*);
Bitmask sqlite3_Where_సమాసం_Usage_NN(Where_ముసుగు_సమితి*, సమాసం*);
Bitmask sqlite3_Where_సమాసం_List_Usage(Where_ముసుగు_సమితి*, సమాసం_List*);
శూన్య sqlite3_Where_సమాసం_Analyze(SrcList*, WhereClause*);
శూన్య sqlite3_Where_Tab_Func_వాదనలు(Parse*, Src_అంశం*, WhereClause*);
పూర్ణాంకం sqlite3WhereLoopBloomable(స్థిరరాశి WhereLoop*);

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
#నిర్వచించు WO_లోపల     0x0001
#నిర్వచించు WO_EQ     0x0002
#నిర్వచించు WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#నిర్వచించు WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#నిర్వచించు WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#నిర్వచించు WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#నిర్వచించు WO_AUX    0x0040       /* Op useful to virtual tables only */
#నిర్వచించు WO_IS     0x0080
#నిర్వచించు WO_ISNULL 0x0100
#నిర్వచించు WO_OR     0x0200       /* Two or more OR-connected terms */
#నిర్వచించు WO_AND    0x0400       /* Two or more AND-connected terms */
#నిర్వచించు WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#నిర్వచించు WO_NOOP   0x1000       /* This term does not restrict search space */
#నిర్వచించు WO_ROWVAL 0x2000       /* A row-value term */

#నిర్వచించు WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#నిర్వచించు WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#నిర్వచించు WHERE_స్తంభం_EQ    0x00000001  /* x=EXPR */
#నిర్వచించు WHERE_స్తంభం_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#నిర్వచించు WHERE_స్తంభం_లోపల    0x00000004  /* x IN (...) */
#నిర్వచించు WHERE_స్తంభం_NULL  0x00000008  /* x IS NULL */
#నిర్వచించు WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#నిర్వచించు WHERE_శిఖరం_హద్దు    0x00000010  /* x<EXPR or x<=EXPR constraint */
#నిర్వచించు WHERE_BTM_హద్దు    0x00000020  /* x>EXPR or x>=EXPR constraint */
#నిర్వచించు WHERE_BOTH_హద్దు   0x00000030  /* Both x>EXPR and x<EXPR */
#నిర్వచించు WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#నిర్వచించు WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#నిర్వచించు WHERE_సూచీకృత      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#నిర్వచించు WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#నిర్వచించు WHERE_లోపల_ABLE      0x00000800  /* Able to support an IN operator */
#నిర్వచించు WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#నిర్వచించు WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#నిర్వచించు WHERE_AUTO_సూచీ   0x00004000  /* Uses an ephemeral index */
#నిర్వచించు WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#నిర్వచించు WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#నిర్వచించు WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#నిర్వచించు WHERE_లోపల_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#నిర్వచించు WHERE_BIGNULL_క్రమీకరించు 0x00080000  /* Column nEq of index is BIGNULL */
#నిర్వచించు WHERE_లోపల_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#నిర్వచించు WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#నిర్వచించు WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#నిర్వచించు WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#నిర్వచించు WHERE_OMIT_స్థానభ్రంశం  0x01000000  /* Set offset counter to zero */
#నిర్వచించు WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#నిర్వచించు WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#అయితే_ముగింపు /* !defined(SQLITE_WHEREINT_H) */
