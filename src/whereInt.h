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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WHEREINT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_WHEREINT_H


/* Forward references
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ WhereClause WhereClause;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Where_ਨਕਾਬ_ਸੈੱਟ Where_ਨਕਾਬ_ਸੈੱਟ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ WhereOrInfo WhereOrInfo;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ WhereAndInfo WhereAndInfo;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ WhereLevel WhereLevel;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ WhereLoop WhereLoop;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Where_ਰਸਤਾ Where_ਰਸਤਾ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Where_ਪਦ Where_ਪਦ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ WhereLoopBuilder WhereLoopBuilder;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Where_ਜਾਂਚ Where_ਜਾਂਚ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Where_Or_ਲਾਗਤ Where_Or_ਲਾਗਤ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Where_Or_ਸੈੱਟ Where_Or_ਸੈੱਟ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Where_Mem_ਖੰਡ Where_Mem_ਖੰਡ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Where_ਸੱਜਾ_Join Where_ਸੱਜਾ_Join;

/*
** This object is a header on a block of allocated memory that will be
** automatically freed when its WInfo object is destructed.
*/
ਬਣਤਰ Where_Mem_ਖੰਡ {
  Where_Mem_ਖੰਡ *p_ਅਗਲਾ;      /* Next block in the chain */
  u64 sz;                    /* Bytes of space */
};

/*
** Extra information attached to a WhereLevel that is a RIGHT JOIN.
*/
ਬਣਤਰ Where_ਸੱਜਾ_Join {
  ਪੂਰਨ i_ਮੇਲ;          /* Cursor used to determine prior matched rows */
  ਪੂਰਨ regBloom;        /* Bloom filter for iRJMatch */
  ਪੂਰਨ regReturn;       /* Return register for the interior subroutine */
  ਪੂਰਨ addrSubrtn;      /* Starting address for the interior subroutine */
  ਪੂਰਨ ਸਿਰਾ_Subrtn;       /* The last opcode in the interior subroutine */
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
ਬਣਤਰ WhereLevel {
  ਪੂਰਨ i_ਖੱਬਾ_Join;        /* Memory cell used to implement LEFT OUTER JOIN */
  ਪੂਰਨ iTabCur;          /* The VDBE cursor used to access the table */
  ਪੂਰਨ iIdxCur;          /* The VDBE cursor used to access pIdx */
  ਪੂਰਨ addrBrk;          /* Jump here to break out of the loop */
  ਪੂਰਨ addrHalt;         /* Abort the query due to empty table or similar */
  ਪੂਰਨ addrNxt;          /* Jump here to start the next IN combination */
  ਪੂਰਨ addrSkip;         /* Jump here for next iteration of skip-scan */
  ਪੂਰਨ addrCont;         /* Jump here to continue with the next loop cycle */
  ਪੂਰਨ addrDeleteCont;   /* Delete auto-index entry then same as addrCont */
  ਪੂਰਨ addr_ਪਹਿਲਾ;        /* First instruction of interior of the loop */
  ਪੂਰਨ addrBody;         /* Beginning of the body of this loop */
  ਪੂਰਨ regBignull;       /* big-null flag reg. True if a NULL-scan is needed */
  ਪੂਰਨ addrBignull;      /* Jump here for next part of big-null scan */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_LIKE_DOESNT_ਮੇਲ_BLOBS
  u32 iLikeRepCntr;     /* LIKE range processing counter register (times 2) */
  ਪੂਰਨ addrLikeRep;      /* LIKE range processing address */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ regFilter;        /* Bloom filter */
  Where_ਸੱਜਾ_Join *pRJ;  /* Extra information for RIGHT JOIN */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  ਪੂਰਨ p1, p2;           /* Operands of the opcode used to end the loop */
  ਸੰਘ {               /* Information that depends on pWLoop->wsFlags */
    ਬਣਤਰ {
      ਪੂਰਨ n_ਵਿੱਚ;              /* Number of entries in aInLoop[] */
      ਬਣਤਰ ਵਿੱਚ_Loop {
        ਪੂਰਨ iCur;              /* The VDBE cursor used by this IN operator */
        ਪੂਰਨ addr_ਵਿੱਚ_ਸਿਖਰ;         /* Top of the IN loop */
        ਪੂਰਨ i_ਅਧਾਰ;             /* Base register of multi-key index record */
        ਪੂਰਨ n_ਅਗੇਤਰ;           /* Number of prior entries in the key */
        u8 e_ਸਿਰਾ_Loop_Op;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *a_ਵਿੱਚ_Loop;           /* Information about each nested IN operator */
    } ਵਿੱਚ;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    ਸੂਚਕਾਂਕ *pCoveringIdx;  /* Possible covering index for WHERE_MULTI_OR */
  } u;
  ਬਣਤਰ WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  ਪੂਰਨ addr_ਦੇਖੋ;        /* Address at which row is visited */
#ਜੇ_ਅੰਤ
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
ਬਣਤਰ WhereLoop {
  Bitmask prereq;       /* Bitmask of other loops that must run first */
  Bitmask ਨਕਾਬ_Self;     /* Bitmask identifying table iTab */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਅੱਖਰ cId;             /* Symbolic ID of this loop for debugging use */
#ਜੇ_ਅੰਤ
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 i_ਛਾਂਟੋ_Idx;          /* Sorting index number.  0==None */
  LogEst r_ਤਿਆਰੀ;        /* One-time setup cost (ex: create transient index) */
  LogEst r_ਲੜੀਵਾਰ;          /* Cost of running each loop */
  LogEst n_ਬਾਹਰਲਾ;          /* Estimated number of output rows */
  ਸੰਘ {
    ਬਣਤਰ {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      u16 nBtm;              /* Size of BTM vector */
      u16 n_ਸਿਖਰ;              /* Size of TOP vector */
      u16 n_Distinct_ਖਾਨਾ;      /* Index columns used to sort for DISTINCT */
      ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ;         /* Index used, or NULL */
      ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;    /* ORDER BY clause if this is really a subquery */
    } btree;
    ਬਣਤਰ {               /* Information for virtual tables */
      ਪੂਰਨ idx_ਅੰਕ;            /* Index number */
      u32 needFree : 1;      /* True if sqlite3_free(idxStr) is needed */
      u32 b_Omit_ਵਿੱਥ : 1;   /* True to let virtual table handle offset */
      u32 b_Idx_ਅੰਕ_Hex : 1;    /* Show idxNum as hex in EXPLAIN QUERY PLAN */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omit_ਨਕਾਬ;          /* Terms that may be omitted */
      ਅੱਖਰ *idxStr;          /* Index identifier string */
      u32 m_Handle_ਵਿੱਚ;         /* Terms to handle as IN(...) instead of == */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
  LogEst r_ਤਾਰਾ_Delta;    /* Cost delta due to star-schema heuristic.  Not
                        ** initialized unless pWInfo->bStarUsed */
#ਜੇ_ਅੰਤ
  Where_ਪਦ **aLTerm;   /* WhereTerms used */
  WhereLoop *p_ਅਗਲਾ_Loop; /* Next WhereLoop object in the WhereClause */
  Where_ਪਦ *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information
*/
ਬਣਤਰ Where_Or_ਲਾਗਤ {
  Bitmask prereq;     /* Prerequisites */
  LogEst r_ਲੜੀਵਾਰ;        /* Cost of running this subquery */
  LogEst n_ਬਾਹਰਲਾ;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ N_OR_ਲਾਗਤ 3
ਬਣਤਰ Where_Or_ਸੈੱਟ {
  u16 n;                      /* Number of valid a[] entries */
  Where_Or_ਲਾਗਤ a[N_OR_ਲਾਗਤ];   /* Set of best costs */
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
ਬਣਤਰ Where_ਰਸਤਾ {
  Bitmask ਨਕਾਬ_Loop;     /* Bitmask of all WhereLoop objects in this path */
  Bitmask revLoop;      /* aLoop[]s that should be reversed for ORDER BY */
  LogEst n_ਪੰਕਤੀ;          /* Estimated number of rows generated by this path */
  LogEst r_ਲਾਗਤ;         /* Total cost of this path */
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
ਬਣਤਰ Where_ਪਦ {
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;            /* Pointer to the subexpression that is this term */
  WhereClause *pWC;       /* The clause this term is part of */
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 n_ਬੱਚਾ;              /* Number of children that must disable us */
  u8 e_ਮੇਲ_Op;            /* Op for vtab MATCH/LIKE/GLOB/REGEXP terms */
  ਪੂਰਨ i_ਮਾਪਾ;            /* Disable pWC->a[iParent] when this term disabled */
  ਪੂਰਨ ਖੱਬਾ_Cursor;         /* Cursor number of X in "X <op> <expr>" */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪੂਰਨ i_ਪਦ;              /* Which WhereTerm is this, for debug purposes */
#ਜੇ_ਅੰਤ
  ਸੰਘ {
    ਬਣਤਰ {
      ਪੂਰਨ ਖੱਬਾ_ਕਾਲਮ;         /* Column number of X in "X <op> <expr>" */
      ਪੂਰਨ i_ਖੇਤਰ;             /* Field in (?,?,?) IN (SELECT...) vector */
    } x;                    /* Opcode other than OP_OR or OP_AND */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  Bitmask prereq_ਸੱਜਾ;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_DYNAMIC    0x0001 /* Need to call sqlite3ExprDelete(db, pExpr) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_VIRTUAL    0x0002 /* Added by the optimizer.  Do not code */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_CODED      0x0004 /* This term is already coded */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_COPIED     0x0008 /* Has a child */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_ORINFO     0x0010 /* Need to free the WhereTerm.u.pOrInfo object */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_ANDINFO    0x0020 /* Need to free the WhereTerm.u.pAndInfo obj */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_OK         0x0040 /* Used during OR-clause processing */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_VNULL      0x0080 /* Manufactured x>NULL or x<=NULL term */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_LIKEOPT    0x0100 /* Virtual terms from the LIKE optimization */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_LIKECOND   0x0200 /* Conditionally this LIKE operator term */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_LIKE       0x0400 /* The original LIKE operator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_IS         0x0800 /* Term.pExpr is an IS operator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_VARSELECT  0x1000 /* Term.pExpr contains a correlated sub-query */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_HEURTRUTH  0x2000 /* Heuristic truthProb used */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
#  define ਪਦ_HIGHTRUTH  0x4000 /* Term excludes few rows */
#ਨਹੀਂਤਾਂ
#  define ਪਦ_HIGHTRUTH  0      /* Only used with STAT4 */
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਦ_ਟੁਕੜਾ      0x8000 /* One slice of a row-value/vector comparison */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
ਬਣਤਰ Where_ਜਾਂਚ {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  ਅਚਲ ਅੱਖਰ *zCollName;     /* Required collating sequence, if not NULL */
  ਵਿਅੰਜਕ *p_Idx_ਵਿਅੰਜਕ;            /* Search for this index expression */
  ਪੂਰਨ k;                     /* Resume scanning at this->pWC->a[this->k] */
  u32 op_ਨਕਾਬ;                /* Acceptable operators */
  ਅੱਖਰ idxaff;               /* Must match this affinity, if zCollName!=NULL */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ iEquiv;      /* Current slot in aiCur[] and aiColumn[] */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ nEquiv;      /* Number of entries in aiCur[] and aiColumn[] */
  ਪੂਰਨ aiCur[11];             /* Cursors in the equivalence class */
  i16 ai_ਕਾਲਮ[11];          /* Corresponding column number in the eq-class */
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
ਬਣਤਰ WhereClause {
  WhereInfo *pWInfo;       /* WHERE clause processing context */
  WhereClause *pOuter;     /* Outer conjunction */
  u8 op;                   /* Split operator.  TK_AND or TK_OR */
  u8 hasOr;                /* True if any a[].eOperator is WO_OR */
  ਪੂਰਨ n_ਪਦ;               /* Number of terms */
  ਪੂਰਨ n_ਥਾਂ;               /* Number of entries in a[] */
  ਪੂਰਨ n_ਅਧਾਰ;               /* Number of terms through the last non-Virtual */
  Where_ਪਦ *a;            /* Each a[] describes a term of the WHERE clause */
#ਜੇ defined(SQLITE_SMALL_ਢੇਰ)
  Where_ਪਦ aStatic[1];    /* Initial static space for a[] */
#ਨਹੀਂਤਾਂ
  Where_ਪਦ aStatic[8];    /* Initial static space for a[] */
#ਜੇ_ਅੰਤ
};

/*
** A WhereTerm with eOperator==WO_OR has its u.pOrInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
ਬਣਤਰ WhereOrInfo {
  WhereClause wc;          /* Decomposition into subterms */
  Bitmask indexable;       /* Bitmask of all indexable tables in the clause */
};

/*
** A WhereTerm with eOperator==WO_AND has its u.pAndInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
ਬਣਤਰ WhereAndInfo {
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
ਬਣਤਰ Where_ਨਕਾਬ_ਸੈੱਟ {
  ਪੂਰਨ bVarSelect;               /* Used by sqlite3WhereExprUsage() */
  ਪੂਰਨ n;                        /* Number of assigned cursor values */
  ਪੂਰਨ ix[BMS];                  /* Cursor assigned to each bit */
};

/*
** This object is a convenience wrapper holding all information needed
** to construct WhereLoop objects for a particular query.
*/
ਬਣਤਰ WhereLoopBuilder {
  WhereInfo *pWInfo;        /* Information about this WHERE */
  WhereClause *pWC;         /* WHERE clause terms */
  WhereLoop *pNew;          /* Template WhereLoop */
  Where_Or_ਸੈੱਟ *p_Or_ਸੈੱਟ;       /* Record best loops here, if not NULL */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  Unpacked_ਰਿਕਾਰਡ *pRec;     /* Probe for stat4 (if required) */
  ਪੂਰਨ nRecValid;            /* Number of valid fields currently in pRec */
#ਜੇ_ਅੰਤ
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ bldFlags1;  /* First set of SQLITE_BLDF_* flags */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ bldFlags2;  /* Second set of SQLITE_BLDF_* flags */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_Plan_ਹੱਦਬੰਦੀ;  /* Search limiter */
};

/* Allowed values for WhereLoopBuider.bldFlags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_BLDF1_ਸੂਚਕਾਂਕਿਤ  0x0001   /* An index is used */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_BLDF1_UNIQUE   0x0002   /* All keys of a UNIQUE index used */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_BLDF2_2NDPASS  0x0004   /* Second builder pass needed */

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_QUERY_PLANNER_ਹੱਦਬੰਦੀ
# define SQLITE_QUERY_PLANNER_ਹੱਦਬੰਦੀ 20000
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_QUERY_PLANNER_ਹੱਦਬੰਦੀ_INCR
# define SQLITE_QUERY_PLANNER_ਹੱਦਬੰਦੀ_INCR 1000
#ਜੇ_ਅੰਤ

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
ਬਣਤਰ WhereInfo {
  Parse *pParse;            /* Parsing and code generating context */
  SrcList *pTabList;        /* List of tables in the join */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;       /* The ORDER BY clause or NULL */
  ਵਿਅੰਜਕ_List *p_ਨਤੀਜਾ_ਸੈੱਟ;     /* Result set of the query */
#ਜੇ WHERETRACE_ENABLED
  ਵਿਅੰਜਕ *pWhere;             /* The complete WHERE clause */
#ਜੇ_ਅੰਤ
  Select *pSelect;          /* The entire SELECT statement containing WHERE */
  ਪੂਰਨ aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  ਪੂਰਨ iContinue;            /* Jump here to continue with next record */
  ਪੂਰਨ iBreak;               /* Jump here to break out of the loop */
  ਪੂਰਨ savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  LogEst i_ਹੱਦਬੰਦੀ;            /* LIMIT if wctrlFlags has WHERE_USE_LIMIT */
  u8 nLevel;                /* Number of nested loop */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 eOnePass;              /* ONEPASS_OFF, or _SINGLE, or _MULTI */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values */
  ਬਿਨਾਨਿਸ਼ਾਨੀ bDeferredSeek :1;   /* Uses OP_DeferredSeek */
  ਬਿਨਾਨਿਸ਼ਾਨੀ untestedTerms :1;   /* Not all WHERE terms resolved by outer loop */
  ਬਿਨਾਨਿਸ਼ਾਨੀ bOrderedInnerLoop:1;/* True if only the inner-most loop is ordered */
  ਬਿਨਾਨਿਸ਼ਾਨੀ sorted        :1;   /* True if really sorted (not just grouped) */
  ਬਿਨਾਨਿਸ਼ਾਨੀ b_ਤਾਰਾ_ਪੂਰਾ     :1;   /* True if check for star-query is complete */
  ਬਿਨਾਨਿਸ਼ਾਨੀ b_ਤਾਰਾ_ਵਰਤਿਆ     :1;   /* True if star-query heuristic is used */
  LogEst n_ਪੰਕਤੀ_ਬਾਹਰਲਾ;           /* Estimated number of output rows */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
  LogEst r_ਕੁੱਲ_ਲਾਗਤ;        /* Total cost of the solution */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ i_ਸਿਖਰ;                 /* The very beginning of the WHERE loop */
  ਪੂਰਨ i_ਸਿਰਾ_Where;            /* End of the WHERE clause itself */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  Where_Mem_ਖੰਡ *p_Mem_ਮੰਜ਼ਿਲ_Free;/* Memory to free when this object destroyed */
  Bitmask rev_ਨਕਾਬ;          /* Mask of ORDER BY terms that need reversing */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  Where_ਨਕਾਬ_ਸੈੱਟ s_ਨਕਾਬ_ਸੈੱਟ;    /* Map cursor numbers to bitmasks */
  WhereLevel a[FLEXARRAY];  /* Information about each nest loop in WHERE */
};

/*
** The size (in bytes) of a WhereInfo object that holds N WhereLevels.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_WHEREINFO(N) ROUND8(offsetof(WhereInfo,a)+(N)*ਆਕਾਰ(WhereLevel))

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
Bitmask sqlite3_Where_Get_ਨਕਾਬ(Where_ਨਕਾਬ_ਸੈੱਟ*,ਪੂਰਨ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
ਖਾਲੀ sqlite3WhereClausePrint(WhereClause *pWC);
ਖਾਲੀ sqlite3_Where_ਪਦ_Print(Where_ਪਦ *p_ਪਦ, ਪੂਰਨ i_ਪਦ);
ਖਾਲੀ sqlite3WhereLoopPrint(ਅਚਲ WhereLoop *p, ਅਚਲ WhereClause *pWC);
#ਜੇ_ਅੰਤ
Where_ਪਦ *sqlite3_Where_ਲੱਭੋ_ਪਦ(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  ਪੂਰਨ iCur,             /* Cursor number of LHS */
  ਪੂਰਨ i_ਕਾਲਮ,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  ਸੂਚਕਾਂਕ *pIdx           /* Must be compatible with this index, if not NULL */
);
ਖਾਲੀ *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte);
ਖਾਲੀ *sqlite3WhereRealloc(WhereInfo *pWInfo, ਖਾਲੀ *pOld, u64 nByte);

/* wherecode.c: */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
ਪੂਰਨ sqlite3_Where_Explain_One_ਜਾਂਚ(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
ਪੂਰਨ sqlite3WhereExplainBloomFilter(
  ਅਚਲ Parse *pParse,            /* Parse context */
  ਅਚਲ WhereInfo *pWInfo,        /* WHERE clause */
  ਅਚਲ WhereLevel *pLevel        /* Bloom filter on this level */
);
ਖਾਲੀ sqlite3_Where_ਜੋੜੋ_Explain_ਪਾਠ(
  Parse *pParse,                  /* Parse context */
  ਪੂਰਨ addr, 
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#ਨਹੀਂਤਾਂ
# define sqlite3_Where_Explain_One_ਜਾਂਚ(u,v,w,x) 0
# define sqlite3WhereExplainBloomFilter(u,v,w) 0
# define  sqlite3_Where_ਜੋੜੋ_Explain_ਪਾਠ(u,v,w,x,y)
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_EXPLAIN */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
ਖਾਲੀ sqlite3_Where_ਜੋੜੋ_ਜਾਂਚ_ਸਥਿਤੀ(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  ਪੂਰਨ addrExplain                 /* Address of OP_Explain (or 0) */
);
#ਨਹੀਂਤਾਂ
# define sqlite3_Where_ਜੋੜੋ_ਜਾਂਚ_ਸਥਿਤੀ(a, b, c, d) ((ਖਾਲੀ)d)
#ਜੇ_ਅੰਤ
Bitmask sqlite3_Where_ਕੋਡ_One_Loop_ਸ਼ੁਰੂ(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  ਪੂਰਨ iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
);
SQLITE_NOINLINE ਖਾਲੀ sqlite3_Where_ਸੱਜਾ_Join_Loop(
  WhereInfo *pWInfo,
  ਪੂਰਨ iLevel,
  WhereLevel *pLevel
);

/* whereexpr.c: */
ਖਾਲੀ sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
ਖਾਲੀ sqlite3WhereClauseClear(WhereClause*);
ਖਾਲੀ sqlite3_Where_ਵੰਡੋ(WhereClause*,ਵਿਅੰਜਕ*,u8);
ਖਾਲੀ sqlite3_Where_ਜੋੜੋ_ਹੱਦਬੰਦੀ(WhereClause*, Select*);
Bitmask sqlite3_Where_ਵਿਅੰਜਕ_Usage(Where_ਨਕਾਬ_ਸੈੱਟ*, ਵਿਅੰਜਕ*);
Bitmask sqlite3_Where_ਵਿਅੰਜਕ_Usage_NN(Where_ਨਕਾਬ_ਸੈੱਟ*, ਵਿਅੰਜਕ*);
Bitmask sqlite3_Where_ਵਿਅੰਜਕ_List_Usage(Where_ਨਕਾਬ_ਸੈੱਟ*, ਵਿਅੰਜਕ_List*);
ਖਾਲੀ sqlite3_Where_ਵਿਅੰਜਕ_Analyze(SrcList*, WhereClause*);
ਖਾਲੀ sqlite3_Where_Tab_Func_ਦਲੀਲਾਂ(Parse*, Src_ਵਸਤੂ*, WhereClause*);
ਪੂਰਨ sqlite3WhereLoopBloomable(ਅਚਲ WhereLoop*);

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_ਵਿੱਚ     0x0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_EQ     0x0002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_AUX    0x0040       /* Op useful to virtual tables only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_IS     0x0080
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_ISNULL 0x0100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_OR     0x0200       /* Two or more OR-connected terms */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_AND    0x0400       /* Two or more AND-connected terms */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_NOOP   0x1000       /* This term does not restrict search space */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_ROWVAL 0x2000       /* A row-value term */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_ALL    0x3fff       /* Mask of all possible WO_* values */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਕਾਲਮ_EQ    0x00000001  /* x=EXPR */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਕਾਲਮ_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਕਾਲਮ_ਵਿੱਚ    0x00000004  /* x IN (...) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਕਾਲਮ_NULL  0x00000008  /* x IS NULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ    0x00000010  /* x<EXPR or x<=EXPR constraint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_BTM_ਹੱਦਬੰਦੀ    0x00000020  /* x>EXPR or x>=EXPR constraint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_BOTH_ਹੱਦਬੰਦੀ   0x00000030  /* Both x>EXPR and x<EXPR */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਸੂਚਕਾਂਕਿਤ      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਵਿੱਚ_ABLE      0x00000800  /* Able to support an IN operator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_AUTO_ਸੂਚਕਾਂਕ   0x00004000  /* Uses an ephemeral index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਵਿੱਚ_EARLYOUT  0x00040000  /* Perhaps quit IN loops early */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_BIGNULL_ਛਾਂਟੋ 0x00080000  /* Column nEq of index is BIGNULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਵਿੱਚ_SEEKSCAN  0x00100000  /* Seek-scan optimization for IN */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_TRANSCONS    0x00200000  /* Uses a transitive constraint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_BLOOMFILTER  0x00400000  /* Consider using a Bloom-filter */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_SELFCULL     0x00800000  /* nOut reduced by extra WHERE terms */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_OMIT_ਵਿੱਥ  0x01000000  /* Set offset counter to zero */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_COROUTINE    0x02000000  /* Implemented by co-routine.
                                       ** NB: False-negatives are possible */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_EXPRIDX      0x04000000  /* Uses an index-on-expressions */

#ਜੇ_ਅੰਤ /* !defined(SQLITE_WHEREINT_H) */
