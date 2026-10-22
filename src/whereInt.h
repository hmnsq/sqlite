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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WHEREINT_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_WHEREINT_H


/* Forward references
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ WhereClause WhereClause;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Where_ಮುಖವಾಡ_ಗಣ Where_ಮುಖವಾಡ_ಗಣ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ WhereOrInfo WhereOrInfo;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ WhereAndInfo WhereAndInfo;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ WhereLevel WhereLevel;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ WhereLoop WhereLoop;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Where_ಮಾರ್ಗ Where_ಮಾರ್ಗ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Where_ಪದ Where_ಪದ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ WhereLoopBuilder WhereLoopBuilder;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Where_ಪರಿಶೀಲನೆ Where_ಪರಿಶೀಲನೆ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Where_Or_ವೆಚ್ಚ Where_Or_ವೆಚ್ಚ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Where_Or_ಗಣ Where_Or_ಗಣ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Where_Mem_ಖಂಡ Where_Mem_ಖಂಡ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Where_ಬಲ_Join Where_ಬಲ_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
ರಚನೆ Where_Mem_ಖಂಡ {
  Where_Mem_ಖಂಡ *p_ಮುಂದಿನ;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
ರಚನೆ Where_ಬಲ_Join {
  ಪೂರ್ಣಾಂಕ i_ಹೊಂದಿಕೆ;          /* Cursor used to determine prior matched rows */
  ಪೂರ್ಣಾಂಕ regBloom;        /* Bloom filter for iRJMatch */
  ಪೂರ್ಣಾಂಕ regReturn;       /* Return register for the interior subroutine */
  ಪೂರ್ಣಾಂಕ addrSubrtn;      /* Starting address for the interior subroutine */
  ಪೂರ್ಣಾಂಕ ಕೊನೆ_Subrtn;       /* The last opcode in the interior subroutine */
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
ರಚನೆ WhereLevel {
  ಪೂರ್ಣಾಂಕ i_ಎಡ_Join;        /* Memory cell used to implement LEFT OUTER JOIN */
  ಪೂರ್ಣಾಂಕ iTabCur;          /* The VDBE cursor used to access the table */
  ಪೂರ್ಣಾಂಕ iIdxCur;          /* The VDBE cursor used to access pIdx */
  ಪೂರ್ಣಾಂಕ addrBrk;          /* Jump here to break out of the loop */
  ಪೂರ್ಣಾಂಕ addrHalt;         /* Abort the query due to empty table or similar */
  ಪೂರ್ಣಾಂಕ addrNxt;          /* Jump here to start the next IN combination */
  ಪೂರ್ಣಾಂಕ addrSkip;         /* Jump here for next iteration of skip-scan */
  ಪೂರ್ಣಾಂಕ addrCont;         /* Jump here to continue with the next loop cycle */
  ಪೂರ್ಣಾಂಕ addrDeleteCont;   /* Delete auto-index entry then same as addrCont */
  ಪೂರ್ಣಾಂಕ addr_ಮೊದಲ;        /* First instruction of interior of the loop */
  ಪೂರ್ಣಾಂಕ addrBody;         /* Beginning of the body of this loop */
  ಪೂರ್ಣಾಂಕ regBignull;       /* big-null flag reg. True if a NULL-scan is needed */
  ಪೂರ್ಣಾಂಕ addrBignull;      /* Jump here for next part of big-null scan */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_LIKE_DOESNT_ಹೊಂದಿಕೆ_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  ಪೂರ್ಣಾಂಕ addrLikeRep;      /* LIKE range processing address */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ regFilter;        /* Bloom filter */
  Where_ಬಲ_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  ಪೂರ್ಣಾಂಕ p1, p2;           /* Operands of the opcode used to end the loop */
  ಒಕ್ಕೂಟ {               /* Information that depends on pWLoop->wsFlags */
    ರಚನೆ {
      ಪೂರ್ಣಾಂಕ n_ಒಳ;              /* Number of entries in aInLoop[] */
      ರಚನೆ ಒಳ_Loop {
        ಪೂರ್ಣಾಂಕ iCur;              /* The VDBE cursor used by this IN operator */
        ಪೂರ್ಣಾಂಕ addr_ಒಳ_ತುದಿ;         /* Top of the IN loop */
        ಪೂರ್ಣಾಂಕ i_ತಳ;             /* Base register of multi-key index record */
        ಪೂರ್ಣಾಂಕ n_ಪೂರ್ವಪ್ರತ್ಯಯ;           /* Number of prior entries in the key */
        u8 e_ಕೊನೆ_Loop_Op;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *a_ಒಳ_Loop;           /* Information about each nested IN operator */
    } ಒಳ;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    ಸೂಚ್ಯಂಕ *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  ರಚನೆ WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  ಪೂರ್ಣಾಂಕ addr_ಭೇಟಿ;        /* Address at which row is visited */
#ಆದರೆ_ಅಂತ್ಯ
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
ರಚನೆ WhereLoop {
  Bitmask prereq;       /* Bitmask of other loops that must run first */
  Bitmask ಮುಖವಾಡ_Self;     /* Bitmask identifying table iTab */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಅಕ್ಷರ cId;             /* Symbolic ID of this loop for debugging use */
#ಆದರೆ_ಅಂತ್ಯ
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 i_ವಿಂಗಡಿಸು_Idx;          /* Sorting index number.  0==None */
  LogEst r_ಸಿದ್ಧತೆ;        /* One-time setup cost (ex: create transient index) */
  LogEst r_ಸತತ;          /* Cost of running each loop */
  LogEst n_ಹೊರಹೋಗು;          /* Estimated number of output rows */
  ಒಕ್ಕೂಟ {
    ರಚನೆ {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_ತುದಿ;              /* Size of TOP vector */
      u16 n_Distinct_ಲಂಬಸಾಲು;      /* Index columns used to sort for DISTINCT */
      ಸೂಚ್ಯಂಕ *p_ಸೂಚ್ಯಂಕ;         /* Index used, or NULL */
      ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;    /* ORDER BY clause if this is really a subquery */
    } btree;
    ರಚನೆ {               /* Information for virtual tables */
      ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 b_Omit_ಸ್ಥಾನಾಂತರ : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_ಅಂಕೆ_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omit_ಮುಖವಾಡ;          /* Terms that may be omitted */
      ಅಕ್ಷರ *idxStr;          /* Index identifier string */
      u32 m_Handle_ಒಳ;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WHERETRACE_ENABLED
  LogEst r_ನಕ್ಷತ್ರ_Delta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#ಆದರೆ_ಅಂತ್ಯ
  Where_ಪದ **aLTerm;   /* WhereTerms used */
  WhereLoop *p_ಮುಂದಿನ_Loop; /* Next WhereLoop object in the WhereClause */
  Where_ಪದ *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
ರಚನೆ Where_Or_ವೆಚ್ಚ {
  Bitmask prereq;     /* Prerequisites */
  LogEst r_ಸತತ;        /* Cost of running this subquery */
  LogEst n_ಹೊರಹೋಗು;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು N_OR_ವೆಚ್ಚ 3
ರಚನೆ Where_Or_ಗಣ {
  u16 n;                      /* Number of valid a[] entries */
  Where_Or_ವೆಚ್ಚ a[N_OR_ವೆಚ್ಚ];   /* Set of best costs */
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
ರಚನೆ Where_ಮಾರ್ಗ {
  Bitmask ಮುಖವಾಡ_Loop;     /* Bitmask of all WhereLoop objects in this path */
  Bitmask revLoop;      /* aLoop[]s that should be reversed for ORDER BY */
  LogEst n_ಅಡ್ಡಸಾಲು;          /* Estimated number of rows generated by this path */
  LogEst r_ವೆಚ್ಚ;         /* Total cost of this path */
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
ರಚನೆ Where_ಪದ {
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 n_ಮಗು;              /* Number of children that must disable us */
  u8 e_ಹೊಂದಿಕೆ_Op;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  ಪೂರ್ಣಾಂಕ i_ಪೋಷಕ;            /* Disable pWC->a[iParent] when this term disabled */
  ಪೂರ್ಣಾಂಕ ಎಡ_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪೂರ್ಣಾಂಕ i_ಪದ;              /* Which WhereTerm is this, for debug purposes */
#ಆದರೆ_ಅಂತ್ಯ
  ಒಕ್ಕೂಟ {
    ರಚನೆ {
      ಪೂರ್ಣಾಂಕ ಎಡ_ಸ್ತಂಭ;         /* Column number of X in "X <op> <expr>" */
      ಪೂರ್ಣಾಂಕ i_ಕ್ಷೇತ್ರ;             /* Field in (?,?,?) IN (SELECT...) vector */
    } x;                    /* Opcode other than OP_OR or OP_AND */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  Bitmask prereq_ಬಲ;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_CODED      0x0004 /* This term is already coded */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_COPIED     0x0008 /* Has a child */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_OK         0x0040 /* Used during OR-clause processing */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_LIKE       0x0400 /* The original LIKE operator */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_IS         0x0800 /* Term.pExpr is an IS operator */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
#  define ಪದ_HIGHTRUTH  0x4000 /* Term excludes few rows */
#ಇಲ್ಲದಿದ್ದರೆ
#  define ಪದ_HIGHTRUTH  0      /* Only used with STAT4 */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು ಪದ_ತುಂಡು      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
ರಚನೆ Where_ಪರಿಶೀಲನೆ {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCollName;     /* Required collating sequence, if not NULL */
  ಅಭಿವ್ಯಕ್ತಿ *p_Idx_ಅಭಿವ್ಯಕ್ತಿ;            /* Search for this index expression */
  ಪೂರ್ಣಾಂಕ k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 op_ಮುಖವಾಡ;                /* Acceptable operators */
  ಅಕ್ಷರ idxaff;               /* Must match this affinity, if zCollName!=NULL */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
  ಪೂರ್ಣಾಂಕ aiCur[11];             /* Cursors in the equivalence class */
  i16 ai_ಸ್ತಂಭ[11];          /* Corresponding column number in the eq-class */
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
ರಚನೆ WhereClause {
  WhereInfo *pWInfo;       /* WHERE clause processing context */
  WhereClause *pOuter;     /* Outer conjunction */
  u8 op;                   /* Split operator.  TK_AND or TK_OR */
  u8 hasOr;                /* True if any a[].eOperator is WO_OR */
  ಪೂರ್ಣಾಂಕ n_ಪದ;               /* Number of terms */
  ಪೂರ್ಣಾಂಕ n_ಸ್ಥಾನ;               /* Number of entries in a[] */
  ಪೂರ್ಣಾಂಕ n_ತಳ;               /* Number of terms through the last non-Virtual */
  Where_ಪದ *a;            /* Each a[] describes a term of the WHERE clause */
#ಆದರೆ defined(SQLITE_SMALL_ರಾಶಿ)
  Where_ಪದ aStatic[1];    /* Initial static space for a[] */
#ಇಲ್ಲದಿದ್ದರೆ
  Where_ಪದ aStatic[8];    /* Initial static space for a[] */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** A WhereTerm with eOperator==WO_OR has its u.pOrInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
ರಚನೆ WhereOrInfo {
  WhereClause wc;          /* Decomposition into subterms */
  Bitmask indexable;       /* Bitmask of all indexable tables in the clause */
};

/*
** A WhereTerm with eOperator==WO_AND has its u.pAndInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
ರಚನೆ WhereAndInfo {
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
ರಚನೆ Where_ಮುಖವಾಡ_ಗಣ {
  ಪೂರ್ಣಾಂಕ bVarSelect;               /* Used by sqlite3WhereExprUsage() */
  ಪೂರ್ಣಾಂಕ n;                        /* Number of assigned cursor values */
  ಪೂರ್ಣಾಂಕ ix[BMS];                  /* Cursor assigned to each bit */
};

/*
** This object is a convenience wrapper holding all information needed
** to construct WhereLoop objects for a particular query.
*/
ರಚನೆ WhereLoopBuilder {
  WhereInfo *pWInfo;        /* Information about this WHERE */
  WhereClause *pWC;         /* WHERE clause terms */
  WhereLoop *pNew;          /* Template WhereLoop */
  Where_Or_ಗಣ *p_Or_ಗಣ;       /* Record best loops here, if not NULL */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  Unpacked_ದಾಖಲೆ *pRec;     /* Probe for stat4 (if required) */
  ಪೂರ್ಣಾಂಕ nRecValid;            /* Number of valid fields currently in pRec */
#ಆದರೆ_ಅಂತ್ಯ
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_Plan_ಎಲ್ಲೆ;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_BLDF1_ಸೂಚ್ಯಂಕಿತ  0x0001   /* An index is used */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_QUERY_PLANNER_ಎಲ್ಲೆ
# define SQLITE_QUERY_PLANNER_ಎಲ್ಲೆ 20000
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_QUERY_PLANNER_ಎಲ್ಲೆ_INCR
# define SQLITE_QUERY_PLANNER_ಎಲ್ಲೆ_INCR 1000
#ಆದರೆ_ಅಂತ್ಯ

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
ರಚನೆ WhereInfo {
  Parse *pParse;            /* Parsing and code generating context */
  SrcList *pTabList;        /* List of tables in the join */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;       /* The ORDER BY clause or NULL */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಫಲಿತಾಂಶ_ಗಣ;     /* Result set of the query */
#ಆದರೆ WHERETRACE_ENABLED
  ಅಭಿವ್ಯಕ್ತಿ *pWhere;             /* The complete WHERE clause */
#ಆದರೆ_ಅಂತ್ಯ
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  ಪೂರ್ಣಾಂಕ aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  ಪೂರ್ಣಾಂಕ iContinue;            /* Jump here to continue with next record */
  ಪೂರ್ಣಾಂಕ iBreak;               /* Jump here to break out of the loop */
  ಪೂರ್ಣಾಂಕ savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst i_ಎಲ್ಲೆ;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  ಚಿಹ್ನೆರಹಿತ bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  ಚಿಹ್ನೆರಹಿತ untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  ಚಿಹ್ನೆರಹಿತ bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  ಚಿಹ್ನೆರಹಿತ sorted        :1;   /* True if really sorted (not just grouped) */
  ಚಿಹ್ನೆರಹಿತ b_ನಕ್ಷತ್ರ_ಮುಗಿದ     :1;   /* True if check for star-query is complete */
  ಚಿಹ್ನೆರಹಿತ b_ನಕ್ಷತ್ರ_ಬಳಸಿದ     :1;   /* True if star-query heuristic is used */
  LogEst n_ಅಡ್ಡಸಾಲು_ಹೊರಹೋಗು;           /* Estimated number of output rows */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WHERETRACE_ENABLED
  LogEst r_ಒಟ್ಟು_ವೆಚ್ಚ;        /* Total cost of the solution */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ i_ತುದಿ;                 /* The very beginning of the WHERE loop */
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ_Where;            /* End of the WHERE clause itself */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  Where_Mem_ಖಂಡ *p_Mem_ಗಮ್ಯ_Free;/* Memory to free when this object destroyed */
  Bitmask rev_ಮುಖವಾಡ;          /* Mask of ORDER BY terms that need reversing */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  Where_ಮುಖವಾಡ_ಗಣ s_ಮುಖವಾಡ_ಗಣ;    /* Map cursor numbers to bitmasks */
  WhereLevel a[FLEXARRAY];  /* Information about each nest loop in WHERE */
};

/*
** The size (in bytes) of a WhereInfo object that holds N WhereLevels.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_WHEREINFO(N) ROUND8(offsetof(WhereInfo,a)+(N)*ಗಾತ್ರ(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3_Where_Get_ಮುಖವಾಡ(Where_ಮುಖವಾಡ_ಗಣ*,ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WHERETRACE_ENABLED
ಶೂನ್ಯ sqlite3WhereClausePrint(WhereClause *pWC);
ಶೂನ್ಯ sqlite3_Where_ಪದ_Print(Where_ಪದ *p_ಪದ, ಪೂರ್ಣಾಂಕ i_ಪದ);
ಶೂನ್ಯ sqlite3WhereLoopPrint(ಸ್ಥಿರಾಂಕ WhereLoop *p, ಸ್ಥಿರಾಂಕ WhereClause *pWC);
#ಆದರೆ_ಅಂತ್ಯ
Where_ಪದ *sqlite3_Where_ಕಂಡುಹಿಡಿ_ಪದ(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  ಪೂರ್ಣಾಂಕ iCur,             /* Cursor number of LHS */
  ಪೂರ್ಣಾಂಕ i_ಸ್ತಂಭ,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  ಸೂಚ್ಯಂಕ *pIdx           /* Must be compatible with this index, if not NULL */
);
ಶೂನ್ಯ *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
ಶೂನ್ಯ *sqlite3WhereRealloc(WhereInfo *pWInfo, ಶೂನ್ಯ *pOld, u64 nByte);

/* wherecode.c: */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
ಪೂರ್ಣಾಂಕ sqlite3_Where_Explain_One_ಪರಿಶೀಲನೆ(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
ಪೂರ್ಣಾಂಕ sqlite3WhereExplainBloomFilter(
  ಸ್ಥಿರಾಂಕ Parse *pParse,            /* Parse context */
  ಸ್ಥಿರಾಂಕ WhereInfo *pWInfo,        /* WHERE clause */
  ಸ್ಥಿರಾಂಕ WhereLevel *pLevel        /* Bloom filter on this level */
);
ಶೂನ್ಯ sqlite3_Where_ಕೂಡಿಸು_Explain_ಪಠ್ಯ(
  Parse *pParse,                  /* Parse context */
  ಪೂರ್ಣಾಂಕ addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_Where_Explain_One_ಪರಿಶೀಲನೆ(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3_Where_ಕೂಡಿಸು_Explain_ಪಠ್ಯ(u,v,w,x,y)
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_EXPLAIN */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
ಶೂನ್ಯ sqlite3_Where_ಕೂಡಿಸು_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  ಪೂರ್ಣಾಂಕ addrExplain                 /* Address of OP_Explain (or 0) */
);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_Where_ಕೂಡಿಸು_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ(a, b, c, d) ((ಶೂನ್ಯ)d)
#ಆದರೆ_ಅಂತ್ಯ
Bitmask sqlite3_Where_ಸಂಕೇತ_One_Loop_ಆರಂಭ(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  ಪೂರ್ಣಾಂಕ iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
);
SQLITE_NOINLINE ಶೂನ್ಯ sqlite3_Where_ಬಲ_Join_Loop(
  WhereInfo *pWInfo,
  ಪೂರ್ಣಾಂಕ iLevel,
  WhereLevel *pLevel
);

/* whereexpr.c: */
ಶೂನ್ಯ sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
ಶೂನ್ಯ sqlite3WhereClauseClear(WhereClause*);
ಶೂನ್ಯ sqlite3_Where_ವಿಭಜಿಸು(WhereClause*,ಅಭಿವ್ಯಕ್ತಿ*,u8);
ಶೂನ್ಯ sqlite3_Where_ಕೂಡಿಸು_ಎಲ್ಲೆ(WhereClause*, Select*);
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage(Where_ಮುಖವಾಡ_ಗಣ*, ಅಭಿವ್ಯಕ್ತಿ*);
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Usage_NN(Where_ಮುಖವಾಡ_ಗಣ*, ಅಭಿವ್ಯಕ್ತಿ*);
Bitmask sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_List_Usage(Where_ಮುಖವಾಡ_ಗಣ*, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಶೂನ್ಯ sqlite3_Where_ಅಭಿವ್ಯಕ್ತಿ_Analyze(SrcList*, WhereClause*);
ಶೂನ್ಯ sqlite3_Where_Tab_Func_ವಾದಗಳು(Parse*, Src_ಅಂಶ*, WhereClause*);
ಪೂರ್ಣಾಂಕ sqlite3WhereLoopBloomable(ಸ್ಥಿರಾಂಕ WhereLoop*);

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
#ವ್ಯಾಖ್ಯಾನಿಸು WO_ಒಳ     0x0001
#ವ್ಯಾಖ್ಯಾನಿಸು WO_EQ     0x0002
#ವ್ಯಾಖ್ಯಾನಿಸು WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#ವ್ಯಾಖ್ಯಾನಿಸು WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#ವ್ಯಾಖ್ಯಾನಿಸು WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#ವ್ಯಾಖ್ಯಾನಿಸು WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#ವ್ಯಾಖ್ಯಾನಿಸು WO_AUX    0x0040       /* Op useful to virtual tables only */
#ವ್ಯಾಖ್ಯಾನಿಸು WO_IS     0x0080
#ವ್ಯಾಖ್ಯಾನಿಸು WO_ISNULL 0x0100
#ವ್ಯಾಖ್ಯಾನಿಸು WO_OR     0x0200       /* Two or more OR-connected terms */
#ವ್ಯಾಖ್ಯಾನಿಸು WO_AND    0x0400       /* Two or more AND-connected terms */
#ವ್ಯಾಖ್ಯಾನಿಸು WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#ವ್ಯಾಖ್ಯಾನಿಸು WO_NOOP   0x1000       /* This term does not restrict search space */
#ವ್ಯಾಖ್ಯಾನಿಸು WO_ROWVAL 0x2000       /* A row-value term */

#ವ್ಯಾಖ್ಯಾನಿಸು WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#ವ್ಯಾಖ್ಯಾನಿಸು WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಸ್ತಂಭ_EQ    0x00000001  /* x=EXPR */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಸ್ತಂಭ_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಸ್ತಂಭ_ಒಳ    0x00000004  /* x IN (...) */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಸ್ತಂಭ_NULL  0x00000008  /* x IS NULL */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ತುದಿ_ಎಲ್ಲೆ    0x00000010  /* x<EXPR or x<=EXPR constraint */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_BTM_ಎಲ್ಲೆ    0x00000020  /* x>EXPR or x>=EXPR constraint */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_BOTH_ಎಲ್ಲೆ   0x00000030  /* Both x>EXPR and x<EXPR */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಸೂಚ್ಯಂಕಿತ      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಒಳ_ABLE      0x00000800  /* Able to support an IN operator */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_AUTO_ಸೂಚ್ಯಂಕ   0x00004000  /* Uses an ephemeral index */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಒಳ_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_BIGNULL_ವಿಂಗಡಿಸು 0x00080000  /* Column nEq of index is BIGNULL */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಒಳ_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_OMIT_ಸ್ಥಾನಾಂತರ  0x01000000  /* Set offset counter to zero */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_WHEREINT_H) */
