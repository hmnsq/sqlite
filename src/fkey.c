/*
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used by the compiler to add foreign key
** support to compiled SQL statements.
*/
#ضمن "sqliteInt.h"

#ifndef SQLITE_OMIT_FOREIGN_مفتاح
#ifndef SQLITE_OMIT_TRIGGER

/*
** Deferred and Immediate FKs
** --------------------------
**
** Foreign keys in SQLite come in two flavours: deferred and immediate.
** If an immediate foreign key constraint is violated,
** SQLITE_CONSTRAINT_FOREIGNKEY is returned and the current
** statement transaction rolled back. If a 
** deferred foreign key constraint is violated, no action is taken 
** immediately. However if the application attempts to commit the 
** transaction before fixing the constraint violation, the attempt fails.
**
** Deferred constraints are implemented using a simple counter associated
** with the database handle. The counter is set to zero each time a 
** database transaction is opened. Each time a statement is executed 
** that causes a foreign key violation, the counter is incremented. Each
** time a statement is executed that removes an existing violation from
** the database, the counter is decremented. When the transaction is
** committed, the commit fails if the current value of the counter is
** greater than zero. This scheme has two big drawbacks:
**
**   * When a commit fails due to a deferred foreign key constraint, 
**     there is no way to tell which foreign constraint is not satisfied,
**     or which row it is not satisfied for.
**
**   * If the database contains foreign key violations when the 
**     transaction is opened, this may cause the mechanism to malfunction.
**
** Despite these problems, this approach is adopted as it seems simpler
** than the alternatives.
**
** INSERT operations:
**
**   I.1) For each FK for which the table is the child table, search
**        the parent table for a match. If none is found increment the
**        constraint counter.
**
**   I.2) For each FK for which the table is the parent table, 
**        search the child table for rows that correspond to the new
**        row in the parent table. Decrement the counter for each row
**        found (as the constraint is now satisfied).
**
** DELETE operations:
**
**   D.1) For each FK for which the table is the child table, 
**        search the parent table for a row that corresponds to the 
**        deleted row in the child table. If such a row is not found, 
**        decrement the counter.
**
**   D.2) For each FK for which the table is the parent table, search 
**        the child table for rows that correspond to the deleted row 
**        in the parent table. For each found increment the counter.
**
** UPDATE operations:
**
**   An UPDATE command requires that all 4 steps above are taken, but only
**   for FK constraints for which the affected columns are actually 
**   modified (values must be compared at runtime).
**
** Note that I.1 and D.1 are very similar operations, as are I.2 and D.2.
** This simplifies the implementation a bit.
**
** For the purposes of immediate FK constraints, the OR REPLACE conflict
** resolution is considered to delete rows before the new row is inserted.
** If a delete caused by OR REPLACE violates an FK constraint, an exception
** is thrown, even if the FK constraint would be satisfied after the new 
** row is inserted.
**
** Immediate constraints are usually handled similarly. The only difference 
** is that the counter used is stored as part of each individual statement
** object (struct Vdbe). If, after the statement has run, its immediate
** constraint counter is greater than zero,
** it returns SQLITE_CONSTRAINT_FOREIGNKEY
** and the statement transaction is rolled back. An exception is an INSERT
** statement that inserts a single row only (no triggers). In this case,
** instead of using a counter, an exception is thrown immediately if the
** INSERT violates a foreign key constraint. This is necessary as such
** an INSERT does not open a statement transaction.
**
** TODO: How should dropping a table be handled? How should renaming a 
** table be handled?
**
**
** Query API Notes
** ---------------
**
** Before coding an UPDATE or DELETE row operation, the code-generator
** for those two operations needs to know whether or not the operation
** requires any FK processing and, if so, which columns of the original
** row are required by the FK processing VDBE code (i.e. if FKs were
** implemented using triggers, which of the old.* columns would be 
** accessed). No information is required by the code-generator before
** coding an INSERT operation. The functions used by the UPDATE/DELETE
** generation code to query for this information are:
**
**   sqlite3FkRequired() - Test to see if FK processing is required.
**   sqlite3FkOldmask()  - Query for the set of required old.* columns.
**
**
** Externally accessible module functions
** --------------------------------------
**
**   sqlite3FkCheck()    - Check for foreign key violations.
**   sqlite3FkActions()  - Code triggers for ON UPDATE/ON DELETE actions.
**   sqlite3FkDelete()   - Delete an FKey structure.
*/

/*
** VDBE Calling Convention
** -----------------------
**
** Example:
**
**   For the following INSERT statement:
**
**     CREATE TABLE t1(a, b INTEGER PRIMARY KEY, c);
**     INSERT INTO t1 VALUES(1, 2, 3.1);
**
**   Register (x):        2    (type integer)
**   Register (x+1):      1    (type integer)
**   Register (x+2):      NULL (type NULL)
**   Register (x+3):      3.1  (type real)
*/

/*
** A foreign key constraint requires that the key columns in the parent
** table are collectively subject to a UNIQUE or PRIMARY KEY constraint.
** Given that pParent is the parent table for foreign key constraint pFKey, 
** search the schema for a unique index on the parent key columns. 
**
** If successful, zero is returned. If the parent key is an INTEGER PRIMARY 
** KEY column, then output variable *ppIdx is set to NULL. Otherwise, *ppIdx 
** is set to point to the unique index. 
** 
** If the parent key consists of a single column (the foreign key constraint
** is not a composite foreign key), output variable *paiCol is set to NULL.
** Otherwise, it is set to point to an allocated array of size N, where
** N is the number of columns in the parent key. The first element of the
** array is the index of the child table column that is mapped by the FK
** constraint to the parent table column stored in the left-most column
** of index *ppIdx. The second element of the array is the index of the
** child table column that corresponds to the second left-most column of
** *ppIdx, and so on.
**
** If the required index cannot be found, either because:
**
**   1) The named parent key columns do not exist, or
**
**   2) The named parent key columns do exist, but are not subject to a
**      UNIQUE or PRIMARY KEY constraint, or
**
**   3) No parent key columns were provided explicitly as part of the
**      foreign key definition, and the parent table does not have a
**      PRIMARY KEY, or
**
**   4) No parent key columns were provided explicitly as part of the
**      foreign key definition, and the PRIMARY KEY of the parent table 
**      consists of a different number of columns to the child key in 
**      the child table.
**
** then non-zero is returned, and a "foreign key mismatch" error loaded
** into pParse. If an OOM error occurs, non-zero is returned and the
** pParse->db->mallocFailed flag is set.
*/
صحيح sqlite3FkLocateIndex(
  Parse *pParse,                  /* Parse context to store any error in */
  Table *pParent,                 /* Parent table of FK constraint pFKey */
  FKey *pFKey,                    /* Foreign key to find index for */
  Index **ppIdx,                  /* OUT: Unique index on parent table */
  صحيح **paiCol                    /* OUT: Map of index columns in pFKey */
){
  Index *pIdx = 0;                    /* Value to return via *ppIdx */
  صحيح *aiCol = 0;                     /* Value to return via *paiCol */
  صحيح nCol = pFKey->nCol;             /* Number of columns in parent key */
  حرف *z_مفتاح = pFKey->aCol[0].zCol;   /* Name of left-most parent key column */

  /* The caller is responsible for zeroing output parameters. */
  أكد( ppIdx && *ppIdx==0 );
  أكد( !paiCol || *paiCol==0 );
  أكد( pParse );

  /* If this is a non-composite (single column) foreign key, check if it 
  ** maps to the INTEGER PRIMARY KEY of table pParent. If so, leave *ppIdx 
  ** and *paiCol set to zero and return early. 
  **
  ** Otherwise, for a composite foreign key (more than one column), allocate
  ** space for the aiCol array (returned via output parameter *paiCol).
  ** Non-composite foreign keys do not require the aiCol array.
  */
  إذا( nCol==1 ){
    /* The FK maps to the IPK if any of the following are true:
    **
    **   1) There is an INTEGER PRIMARY KEY column and the FK is implicitly 
    **      mapped to the primary key of table pParent, or
    **   2) The FK is explicitly mapped to a column declared as INTEGER
    **      PRIMARY KEY.
    */
    إذا( pParent->iPKey>=0 ){
      إذا( !z_مفتاح ) إرجاع 0;
      إذا( !sqlite3StrICmp(pParent->aCol[pParent->iPKey].zCnName, z_مفتاح) ){
        إرجاع 0;
      }
    }
  }وإلا إذا( paiCol ){
    أكد( nCol>1 );
    aiCol = (صحيح *)sqlite3DbMallocRawNN(pParse->db, nCol*حجم(صحيح));
    إذا( !aiCol ) إرجاع 1;
    *paiCol = aiCol;
  }

  لكل(pIdx=pParent->pIndex; pIdx; pIdx=pIdx->pNext){
    إذا( pIdx->n_مفتاح_Col==nCol && IsUniqueIndex(pIdx) && pIdx->pPartIdxWhere==0 ){ 
      /* pIdx is a UNIQUE index (or a PRIMARY KEY) and has the right number
      ** of columns. If each indexed column corresponds to a foreign key
      ** column of pFKey, then this index is a winner.  */

      إذا( z_مفتاح==0 ){
        /* If zKey is NULL, then this foreign key is implicitly mapped to 
        ** the PRIMARY KEY of table pParent. The PRIMARY KEY index may be 
        ** identified by the test.  */
        إذا( Is_Primary_مفتاح_Index(pIdx) ){
          إذا( aiCol ){
            صحيح i;
            لكل(i=0; i<nCol; i++) aiCol[i] = pFKey->aCol[i].iFrom;
          }
          اكسر;
        }
      }وإلا{
        /* If zKey is non-NULL, then this foreign key was declared to
        ** map to an explicit list of columns in table pParent. Check if this
        ** index matches those columns. Also, check that the index uses
        ** the default collation sequences for each column. */
        صحيح i, j;
        لكل(i=0; i<nCol; i++){
          i16 iCol = pIdx->aiColumn[i];     /* Index of column in parent tbl */
          ثابت حرف *zDfltColl;            /* Def. collation for column */
          حرف *zIdxCol;                    /* Name of indexed column */

          إذا( iCol<0 ) اكسر; /* No foreign keys against expression indexes */

          /* If the index uses a collation sequence that is different from
          ** the default collation sequence for the column, this index is
          ** unusable. Bail out early in this case.  */
          zDfltColl = sqlite3ColumnColl(&pParent->aCol[iCol]);
          إذا( !zDfltColl ) zDfltColl = sqlite3_Str_ثنائي;
          إذا( sqlite3StrICmp(pIdx->azColl[i], zDfltColl) ) اكسر;

          zIdxCol = pParent->aCol[iCol].zCnName;
          لكل(j=0; j<nCol; j++){
            إذا( sqlite3StrICmp(pFKey->aCol[j].zCol, zIdxCol)==0 ){
              إذا( aiCol ) aiCol[i] = pFKey->aCol[j].iFrom;
              اكسر;
            }
          }
          إذا( j==nCol ) اكسر;
        }
        إذا( i==nCol ) اكسر;      /* pIdx is usable */
      }
    }
  }

  إذا( !pIdx ){
    إذا( !pParse->disableTriggers ){
      sqlite3_Error_رسالة(pParse,
           "foreign key mismatch - \"%w\" referencing \"%w\"",
           pFKey->pFrom->zName, pFKey->zTo);
    }
    sqlite3DbFree(pParse->db, aiCol);
    إرجاع 1;
  }

  *ppIdx = pIdx;
  إرجاع 0;
}

/*
** This function is called when a row is inserted into or deleted from the 
** child table of foreign key constraint pFKey. If an SQL UPDATE is executed 
** on the child table of pFKey, this function is invoked twice for each row
** affected - once to "delete" the old row, and then again to "insert" the
** new row.
**
** Each time it is called, this function generates VDBE code to locate the
** row in the parent table that corresponds to the row being inserted into 
** or deleted from the child table. If the parent row can be found, no 
** special action is taken. Otherwise, if the parent row can *not* be
** found in the parent table:
**
**   Operation | FK type   | Action taken
**   --------------------------------------------------------------------------
**   INSERT      immediate   Increment the "immediate constraint counter".
**
**   DELETE      immediate   Decrement the "immediate constraint counter".
**
**   INSERT      deferred    Increment the "deferred constraint counter".
**
**   DELETE      deferred    Decrement the "deferred constraint counter".
**
** These operations are identified in the comment at the top of this file 
** (fkey.c) as "I.1" and "D.1".
*/
ساكن فراغ fkLookupParent(
  Parse *pParse,        /* Parse context */
  صحيح iDb,              /* Index of database housing pTab */
  Table *pTab,          /* Parent table of FK pFKey */
  Index *pIdx,          /* Unique index on parent key columns in pTab */
  FKey *pFKey,          /* Foreign key constraint */
  صحيح *aiCol,           /* Map from parent key columns to child table columns */
  صحيح regData,          /* Address of array containing child table row */
  صحيح nIncr,            /* Increment constraint counter by this */
  صحيح isIgnore          /* If true, pretend pTab contains all NULL values */
){
  صحيح i;                                    /* Iterator variable */
  Vdbe *v = sqlite3GetVdbe(pParse);         /* Vdbe to add code to */
  صحيح iCur = pParse->nTab - 1;              /* Cursor number to use */
  صحيح iOk = sqlite3VdbeMakeLabel(pParse);   /* jump here if parent key found */

  sqlite3VdbeVerifyAbortable(v,
    (!pFKey->isDeferred
      && !(pParse->db->flags & SQLITE_DeferFKs)
      && !pParse->pToplevel 
      && !pParse->isMultiWrite) ? OE_Abort : OE_Ignore);

  /* If nIncr is less than zero, then check at runtime if there are any
  ** outstanding constraints to resolve. If there are not, there is no need
  ** to check if deleting this row resolves any outstanding violations.
  **
  ** Check if any of the key columns in the child table row are NULL. If 
  ** any are, then the constraint is considered satisfied. No need to 
  ** search for a matching row in the parent table.  */
  إذا( nIncr<0 ){
    sqlite3VdbeAddOp2(v, OP_FkIfZero, pFKey->isDeferred, iOk);
    VdbeCoverage(v);
  }
  لكل(i=0; i<pFKey->nCol; i++){
    صحيح iReg = sqlite3TableColumnToStorage(pFKey->pFrom,aiCol[i]) + regData + 1;
    sqlite3VdbeAddOp2(v, OP_IsNull, iReg, iOk); VdbeCoverage(v);
  }

  إذا( isIgnore==0 ){
    إذا( pIdx==0 ){
      /* If pIdx is NULL, then the parent key is the INTEGER PRIMARY KEY
      ** column of the parent table (table pTab).  */
      صحيح iMustBeInt;               /* Address of MustBeInt instruction */
      صحيح regTemp = sqlite3GetTempReg(pParse);
  
      /* Invoke MustBeInt to coerce the child key value to an integer (i.e. 
      ** apply the affinity of the parent key). If this fails, then there
      ** is no matching parent key. Before using MustBeInt, make a copy of
      ** the value. Otherwise, the value inserted into the child key column
      ** will have INTEGER affinity applied to it, which may not be correct.  */
      sqlite3VdbeAddOp2(v, OP_SCopy, 
        sqlite3TableColumnToStorage(pFKey->pFrom,aiCol[0])+1+regData, regTemp);
      iMustBeInt = sqlite3VdbeAddOp2(v, OP_MustBeInt, regTemp, 0);
      VdbeCoverage(v);
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter.  */
      إذا( pTab==pFKey->pFrom && nIncr==1 ){
        sqlite3VdbeAddOp3(v, OP_Eq, regData, iOk, regTemp); VdbeCoverage(v);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      }
  
      sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenRead);
      sqlite3VdbeAddOp3(v, OP_NotExists, iCur, 0, regTemp); VdbeCoverage(v);
      sqlite3VdbeGoto(v, iOk);
      sqlite3VdbeJumpHere(v, sqlite3VdbeCurrentAddr(v)-2);
      sqlite3VdbeJumpHere(v, iMustBeInt);
      sqlite3ReleaseTempReg(pParse, regTemp);
    }وإلا{
      صحيح nCol = pFKey->nCol;
      صحيح regTemp = sqlite3GetTempRange(pParse, nCol);
  
      sqlite3VdbeAddOp3(v, OP_OpenRead, iCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_Set_P4_مفتاح_Info(pParse, pIdx);
      لكل(i=0; i<nCol; i++){
        sqlite3VdbeAddOp2(v, OP_Copy, 
               sqlite3TableColumnToStorage(pFKey->pFrom, aiCol[i])+1+regData,
               regTemp+i);
      }
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter. 
      **
      ** If any of the parent-key values are NULL, then the row cannot match 
      ** itself. So set JUMPIFNULL to make sure we do the OP_Found if any
      ** of the parent-key values are NULL (at this point it is known that
      ** none of the child key values are).
      */
      إذا( pTab==pFKey->pFrom && nIncr==1 ){
        صحيح iJump = sqlite3VdbeCurrentAddr(v) + nCol + 1;
        لكل(i=0; i<nCol; i++){
          صحيح iChild = sqlite3TableColumnToStorage(pFKey->pFrom,aiCol[i])
                              +1+regData;
          صحيح iParent = 1+regData;
          iParent += sqlite3TableColumnToStorage(pIdx->pTable,
                                                 pIdx->aiColumn[i]);
          أكد( pIdx->aiColumn[i]>=0 );
          أكد( aiCol[i]!=pTab->iPKey );
          إذا( pIdx->aiColumn[i]==pTab->iPKey ){
            /* The parent key is a composite key that includes the IPK column */
            iParent = regData;
          }
          sqlite3VdbeAddOp3(v, OP_Ne, iChild, iJump, iParent); VdbeCoverage(v);
          sqlite3VdbeChangeP5(v, SQLITE_JUMPIFNULL);
        }
        sqlite3VdbeGoto(v, iOk);
      }

      sqlite3VdbeAddOp4(v, OP_Affinity, regTemp, nCol, 0,
                        sqlite3IndexAffinityStr(pParse->db,pIdx), nCol);
      sqlite3VdbeAddOp4Int(v, OP_Found, iCur, iOk, regTemp, nCol);
      VdbeCoverage(v);
      sqlite3ReleaseTempRange(pParse, regTemp, nCol);
    }
  }

  إذا( !pFKey->isDeferred && !(pParse->db->flags & SQLITE_DeferFKs)
   && !pParse->pToplevel 
   && !pParse->isMultiWrite 
  ){
    /* Special case: If this is an INSERT statement that will insert exactly
    ** one row into the table, raise a constraint immediately instead of
    ** incrementing a counter. This is necessary as the VM code is being
    ** generated for will not open a statement transaction.  */
    أكد( nIncr==1 );
    sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
        OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
  }وإلا{
    إذا( nIncr>0 && pFKey->isDeferred==0 ){
      sqlite3MayAbort(pParse);
    }
    sqlite3VdbeAddOp2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
  }

  sqlite3VdbeResolveLabel(v, iOk);
  sqlite3VdbeAddOp1(v, OP_Close, iCur);
}


/*
** Return an Expr object that refers to a memory register corresponding
** to column iCol of table pTab.
**
** regBase is the first of an array of register that contains the data
** for pTab.  regBase itself holds the rowid.  regBase+1 holds the first
** column.  regBase+2 holds the second column, and so forth.
*/
ساكن تعبير *تعبير_Table_Register(
  Parse *pParse,     /* Parsing and code generating context */
  Table *pTab,       /* The table whose content is at r[regBase]... */
  صحيح regBase,       /* Contents of table pTab */
  i16 iCol           /* Which column of pTab is desired */
){
  تعبير *p_تعبير;
  Column *pCol;
  ثابت حرف *zColl;
  sqlite3 *db = pParse->db;

  p_تعبير = sqlite3_تعبير(db, TK_REGISTER, 0);
  إذا( p_تعبير ){
    إذا( iCol>=0 && iCol!=pTab->iPKey ){
      pCol = &pTab->aCol[iCol];
      p_تعبير->iTable = regBase + sqlite3TableColumnToStorage(pTab,iCol) + 1;
      p_تعبير->aff_تعبير = pCol->affinity;
      zColl = sqlite3ColumnColl(pCol);
      إذا( zColl==0 ) zColl = db->pDfltColl->zName;
      p_تعبير = sqlite3_تعبير_Add_Collate_String(pParse, p_تعبير, zColl);
    }وإلا{
      p_تعبير->iTable = regBase;
      p_تعبير->aff_تعبير = SQLITE_AFF_INTEGER;
    }
  }
  إرجاع p_تعبير;
}

/*
** Return an Expr object that refers to column iCol of table pTab which
** has cursor iCur.
*/
ساكن تعبير *تعبير_Table_Column(
  sqlite3 *db,      /* The database connection */
  Table *pTab,      /* The table whose column is desired */
  صحيح iCursor,      /* The open cursor on the table */
  i16 iCol          /* The column that is wanted */
){
  تعبير *p_تعبير = sqlite3_تعبير(db, TK_COLUMN, 0);
  إذا( p_تعبير ){
    أكد( تعبير_Use_YTab(p_تعبير) );
    p_تعبير->y.pTab = pTab;
    p_تعبير->iTable = iCursor;
    p_تعبير->iColumn = iCol;
  }
  إرجاع p_تعبير;
}

/*
** Column iCol1 of table p1 is to be compared to column iCol2 of table p2.
** Return non-zero if the comparison will use an affinity compatible
** with the affinity of column iCol1 of table p1. Or zero if it will not.
**
** This is used in cases where (p1.iCol1 = p2.iCol2) must be evaluated
** using the affinity of p1.iCol1 as part of foreign key processing.
*/
ساكن صحيح fkUsesLhsAffinity(Table *p1, صحيح iCol1, Table *p2, صحيح iCol2){
  u8 aff1 = iCol1>=0 ? p1->aCol[iCol1].affinity : SQLITE_AFF_NUMERIC;
  u8 aff2 = p2->aCol[iCol2].affinity;

  /* If either side has a numeric affinity, then the comparison uses numeric
  ** affinity. So the affinity matches if aff1 is numeric, but not if aff1
  ** is not numeric but aff2 is.  */
  إذا( sqlite3IsNumericAffinity(aff1) ) إرجاع 1;
  إذا( sqlite3IsNumericAffinity(aff2) ) إرجاع 0;

  أكد( aff1==SQLITE_AFF_TEXT || aff1==SQLITE_AFF_BLOB );
  أكد( aff2==SQLITE_AFF_TEXT || aff2==SQLITE_AFF_BLOB );

  /* At this point, either the affinities are the same or one is TEXT and
  ** the other BLOB. In the second case the comparison will be done with
  ** BLOB affinity, which does not match if aff1 is TEXT.  */
  إرجاع (aff1==aff2 || aff1==SQLITE_AFF_BLOB);
}

#ifndef SQLITE_OMIT_CAST
/*
** Return non-zero if a lookup on column iCol of table pTab, using collation
** sequence zColl, can be done using either the rowid or an index. That is,
** if iCol is the INTEGER PRIMARY KEY of pTab, or if pTab has an index
** whose left-most column is iCol and whose collation sequence for that
** column is zColl. Return zero otherwise.
*/
ساكن صحيح fk_Child_مفتاح_Is_Indexed(Table *pTab, صحيح iCol, ثابت حرف *zColl){
  Index *pIdx;
  إذا( iCol==pTab->iPKey ) إرجاع 1;
  لكل(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
    إذا( pIdx->aiColumn[0]==iCol
     && pIdx->pPartIdxWhere==0
     && sqlite3StrICmp(pIdx->azColl[0], zColl)==0
    ){
      إرجاع 1;
    }
  }
  إرجاع 0;
}
#endif

/*
** Return an expression equivalent to (pLeft = pRight), where pLeft is a
** parent key value and pRight is an unresolved reference to column
** iChildCol of child table pChild. The comparison uses the affinity and
** collation sequence of column iCol of parent table pTab. pLeft must
** already carry that affinity and collation sequence. Ownership of
** both pLeft and pRight passes to this routine.
**
** If the simple (pLeft = pRight) does not naturally use the affinity of
** the parent column, the comparison is coded as (pLeft = +pRight). But
** the unary + prevents any index on the child column from being used.
** So in that case, if the child column is indexed, an extra term is added:
**
**     (pLeft = +pRight) AND
**     (pRight COLLATE <coll> IN (+pLeft, +CAST(pLeft AS NUMERIC)))
**
** Each child value that matches pLeft under the affinity of the parent
** column is either equal to pLeft itself or numerically equal to
** CAST(pLeft AS NUMERIC). So the IN term matches a superset of the
** child rows that the exact (pLeft = +pRight) term does, and since it
** compares using the affinity of the child column, it may be used with
** an index on the child column. The unary + operators on the RHS of the
** IN ensure that no affinity other than that of the child column is
** applied.
*/
ساكن تعبير *fkParentChildEq(
  Parse *pParse,            /* Parsing context */
  تعبير *p_يسار,              /* Parent key value */
  تعبير *p_يمين,             /* Unresolved reference to child key column */
  Table *pTab,              /* Parent table */
  صحيح iCol,                 /* Parent key column, or <0 for rowid */
  Table *pChild,            /* Child table */
  صحيح iChildCol,            /* Child key column */
  صحيح bComposite            /* This is part of a composite FK */
){
  sqlite3 *db = pParse->db;
  تعبير *pEq;
#ifndef SQLITE_OMIT_CAST
  تعبير *pIn = 0;
#endif

  إذا( fkUsesLhsAffinity(pTab, iCol, pChild, iChildCol) ){
    إرجاع sqlite3PExpr(pParse, TK_EQ, p_يسار, p_يمين);
  }
  أكد( iCol>=0 );

#ifndef SQLITE_OMIT_CAST
  {
    ثابت حرف *zColl = sqlite3ColumnColl(&pTab->aCol[iCol]);
    إذا( zColl==0 ) zColl = sqlite3_Str_ثنائي;

    /* The IN term is only useful if it can drive a lookup on the child
    ** table. Omit it otherwise, so that a full scan of the child table
    ** does not have to evaluate it for every row.  */
    إذا( bComposite || fk_Child_مفتاح_Is_Indexed(pChild, iChildCol, zColl) ){
      ساكن ثابت Token tNumeric = { "NUMERIC", 7 };
      تعبير *pCast;
      تعبير_List *pList;

      pCast = sqlite3_تعبير_Alloc(db, TK_CAST, &tNumeric, 0);
      sqlite3_تعبير_Attach_Subtrees(db, pCast, sqlite3_تعبير_Dup(db, p_يسار, 0), 0);
      pList = sqlite3_تعبير_List_Append(pParse, 0,
          sqlite3PExpr(pParse, TK_UPLUS, sqlite3_تعبير_Dup(db, p_يسار, 0), 0)
      );
      pList = sqlite3_تعبير_List_Append(pParse, pList,
          sqlite3PExpr(pParse, TK_UPLUS, pCast, 0)
      );
      pIn = sqlite3PExpr(pParse, TK_IN, sqlite3_تعبير_Add_Collate_String(pParse,
            sqlite3_تعبير_Dup(db, p_يمين, 0), zColl
      ), 0);
      إذا( pIn ){
        pIn->x.pList = pList;
        sqlite3_تعبير_Set_Height_And_Flags(pParse, pIn);
      }وإلا{
        sqlite3_تعبير_List_Delete(db, pList);
      }
    }
  }
#endif

  p_يمين = sqlite3PExpr(pParse, TK_UPLUS, p_يمين, 0);
  pEq = sqlite3PExpr(pParse, TK_EQ, p_يسار, p_يمين);
#ifndef SQLITE_OMIT_CAST
  pEq = sqlite3_تعبير_And(pParse, pEq, pIn);
#endif
  إرجاع pEq;
}

/*
** This function is called to generate code executed when a row is deleted
** from the parent table of foreign key constraint pFKey and, if pFKey is 
** deferred, when a row is inserted into the same table. When generating
** code for an SQL UPDATE operation, this function may be called twice -
** once to "delete" the old row and once to "insert" the new row.
**
** Parameter nIncr is passed -1 when inserting a row (as this may decrease
** the number of FK violations in the db) or +1 when deleting one (as this
** may increase the number of FK constraint problems).
**
** The code generated by this function scans through the rows in the child
** table that correspond to the parent table row being deleted or inserted.
** For each child row found, one of the following actions is taken:
**
**   Operation | FK type   | Action taken
**   --------------------------------------------------------------------------
**   DELETE      immediate   Increment the "immediate constraint counter".
**
**   INSERT      immediate   Decrement the "immediate constraint counter".
**
**   DELETE      deferred    Increment the "deferred constraint counter".
**
**   INSERT      deferred    Decrement the "deferred constraint counter".
**
** These operations are identified in the comment at the top of this file 
** (fkey.c) as "I.2" and "D.2".
*/
ساكن فراغ fkScanChildren(
  Parse *pParse,                  /* Parse context */
  SrcList *pSrc,                  /* The child table to be scanned */
  Table *pTab,                    /* The parent table */
  Index *pIdx,                    /* Index on parent covering the foreign key */
  FKey *pFKey,                    /* The foreign key linking pSrc to pTab */
  صحيح *aiCol,                     /* Map from pIdx cols to child table cols */
  صحيح regData,                    /* Parent row data starts here */
  صحيح nIncr                       /* Amount to increment deferred counter by */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  صحيح i;                          /* Iterator variable */
  تعبير *pWhere = 0;               /* WHERE clause to scan with */
  NameContext sNameContext;       /* Context used to resolve WHERE clause */
  WhereInfo *pWInfo;              /* Context used by sqlite3WhereXXX() */
  صحيح iFkIfZero = 0;              /* Address of OP_FkIfZero */
  Vdbe *v = sqlite3GetVdbe(pParse);

  أكد( pIdx==0 || pIdx->pTable==pTab );
  أكد( pIdx==0 || pIdx->n_مفتاح_Col==pFKey->nCol );
  أكد( pIdx!=0 || pFKey->nCol==1 );
  أكد( pIdx!=0 || HasRowid(pTab) );

  إذا( nIncr<0 ){
    iFkIfZero = sqlite3VdbeAddOp2(v, OP_FkIfZero, pFKey->isDeferred, 0);
    VdbeCoverage(v);
  }

  /* Create an Expr object representing an SQL expression like:
  **
  **   <parent-key1> = <child-key1> AND <parent-key2> = <child-key2> ...
  **
  ** The collation sequence used for the comparison should be that of
  ** the parent key columns. The affinity of the parent key column should
  ** be applied to each child key value before the comparison takes place.
  */
  لكل(i=0; i<pFKey->nCol; i++){
    تعبير *p_يسار;                  /* Value from parent table row */
    تعبير *p_يمين;                 /* Column ref to child table */
    تعبير *pEq;                    /* Expression (pLeft = pRight) */
    i16 iCol;                     /* Index of column in parent table */
    i16 iCol2;                    /* Index of column in child table */
    ثابت حرف *zCol;             /* Name of column in child table */

    iCol = pIdx ? pIdx->aiColumn[i] : -1;
    p_يسار = تعبير_Table_Register(pParse, pTab, regData, iCol);
    iCol2 = aiCol ? aiCol[i] : pFKey->aCol[0].iFrom;
    أكد( iCol2>=0 );
    zCol = pFKey->pFrom->aCol[iCol2].zCnName;
    p_يمين = sqlite3_تعبير(db, TK_ID, zCol);
    pEq = fkParentChildEq(pParse, p_يسار, p_يمين, pTab, iCol,
                          pFKey->pFrom, iCol2, pFKey->nCol>0);
    pWhere = sqlite3_تعبير_And(pParse, pWhere, pEq);
  }

  /* If the child table is the same as the parent table, then add terms
  ** to the WHERE clause that prevent this entry from being scanned.
  ** The added WHERE clause terms are like this:
  **
  **     $current_rowid!=rowid
  **     NOT( $current_a==a AND $current_b==b AND ... )
  **
  ** The first form is used for rowid tables.  The second form is used
  ** for WITHOUT ROWID tables. In the second form, the *parent* key is
  ** (a,b,...). Either the parent or primary key could be used to 
  ** uniquely identify the current row, but the parent key is more convenient
  ** as the required values have already been loaded into registers
  ** by the caller.
  */
  إذا( pTab==pFKey->pFrom && nIncr>0 ){
    تعبير *pNe;                    /* Expression (pLeft != pRight) */
    تعبير *p_يسار;                  /* Value from parent table row */
    تعبير *p_يمين;                 /* Column ref to child table */
    إذا( HasRowid(pTab) ){
      p_يسار = تعبير_Table_Register(pParse, pTab, regData, -1);
      p_يمين = تعبير_Table_Column(db, pTab, pSrc->a[0].iCursor, -1);
      pNe = sqlite3PExpr(pParse, TK_NE, p_يسار, p_يمين);
    }وإلا{
      تعبير *pEq, *pAll = 0;
      أكد( pIdx!=0 );
      لكل(i=0; i<pIdx->n_مفتاح_Col; i++){
        i16 iCol = pIdx->aiColumn[i];
        أكد( iCol>=0 );
        p_يسار = تعبير_Table_Register(pParse, pTab, regData, iCol);
        p_يمين = sqlite3_تعبير(db, TK_ID, pTab->aCol[iCol].zCnName);
        pEq = sqlite3PExpr(pParse, TK_IS, p_يسار, p_يمين);
        pAll = sqlite3_تعبير_And(pParse, pAll, pEq);
      }
      pNe = sqlite3PExpr(pParse, TK_NOT, pAll, 0);
    }
    pWhere = sqlite3_تعبير_And(pParse, pWhere, pNe);
  }

  /* Resolve the references in the WHERE clause. */
  memset(&sNameContext, 0, حجم(NameContext));
  sNameContext.pSrcList = pSrc;
  sNameContext.pParse = pParse;
  sqlite3_Resolve_تعبير_Names(&sNameContext, pWhere);

  /* Create VDBE to loop through the entries in pSrc that match the WHERE
  ** clause. For each row found, increment either the deferred or immediate
  ** foreign key constraint counter. */
  إذا( pParse->nErr==0 ){
    pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0, 0);
    sqlite3VdbeAddOp2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
    إذا( pWInfo ){
      sqlite3WhereEnd(pWInfo);
    }
  }

  /* Clean up the WHERE clause constructed above. */
  sqlite3_تعبير_Delete(db, pWhere);
  إذا( iFkIfZero ){
    sqlite3_Vdbe_Jump_Here_Or_اسحب_Inst(v, iFkIfZero);
  }
}

/*
** This function returns a linked list of FKey objects (connected by
** FKey.pNextTo) holding all children of table pTab.  For example,
** given the following schema:
**
**   CREATE TABLE t1(a PRIMARY KEY);
**   CREATE TABLE t2(b REFERENCES t1(a);
**
** Calling this function with table "t1" as an argument returns a pointer
** to the FKey structure representing the foreign key constraint on table
** "t2". Calling this function with "t2" as the argument would return a
** NULL pointer (as there are no FK constraints for which t2 is the parent
** table).
*/
FKey *sqlite3FkReferences(Table *pTab){
  إرجاع (FKey *)sqlite3HashFind(&pTab->pSchema->fkeyHash, pTab->zName);
}

/*
** The second argument is a Trigger structure allocated by the 
** fkActionTrigger() routine. This function deletes the Trigger structure
** and all of its sub-components.
**
** The Trigger structure or any of its sub-components may be allocated from
** the lookaside buffer belonging to database handle dbMem.
*/
ساكن فراغ fkTriggerDelete(sqlite3 *dbMem, Trigger *p){
  إذا( p ){
    TriggerStep *pStep = p->step_list;
    sqlite3SrcListDelete(dbMem, pStep->pSrc);
    sqlite3_تعبير_Delete(dbMem, pStep->pWhere);
    sqlite3_تعبير_List_Delete(dbMem, pStep->p_تعبير_List);
    sqlite3SelectDelete(dbMem, pStep->pSelect);
    sqlite3_تعبير_Delete(dbMem, p->pWhen);
    sqlite3DbFree(dbMem, p);
  }
}

/*
** Clear the apTrigger[] cache of CASCADE triggers for all foreign keys
** in a particular database.  This needs to happen when the schema
** changes.
*/
فراغ sqlite3FkClearTriggerCache(sqlite3 *db, صحيح iDb){
  HashElem *k;
  Hash *pHash = &db->aDb[iDb].pSchema->tblHash;
  لكل(k=sqliteHashFirst(pHash); k; k=sqliteHashNext(k)){
    Table *pTab = sqliteHashData(k);
    FKey *pFKey;
    إذا( !IsOrdinaryTable(pTab) ) استمر;
    لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
      fkTriggerDelete(db, pFKey->apTrigger[0]); pFKey->apTrigger[0] = 0;
      fkTriggerDelete(db, pFKey->apTrigger[1]); pFKey->apTrigger[1] = 0;
    }
  }
}

/*
** This function is called to generate code that runs when table pTab is
** being dropped from the database. The SrcList passed as the second argument
** to this function contains a single entry guaranteed to resolve to
** table pTab.
**
** Normally, no code is required. However, if either
**
**   (a) The table is the parent table of a FK constraint, or
**   (b) The table is the child table of a deferred FK constraint and it is
**       determined at runtime that there are outstanding deferred FK 
**       constraint violations in the database,
**
** then the equivalent of "DELETE FROM <tbl>" is executed before dropping
** the table from the database. Triggers are disabled while running this
** DELETE, but foreign key actions are not.
*/
فراغ sqlite3FkDropTable(Parse *pParse, SrcList *pName, Table *pTab){
  sqlite3 *db = pParse->db;
  إذا( (db->flags&SQLITE_Foreign_مفاتيح) && IsOrdinaryTable(pTab) ){
    صحيح iSkip = 0;
    Vdbe *v = sqlite3GetVdbe(pParse);

    أكد( v );                  /* VDBE has already been allocated */
    أكد( IsOrdinaryTable(pTab) );
    إذا( sqlite3FkReferences(pTab)==0 ){
      /* Search for a deferred foreign key constraint for which this table
      ** is the child table. If one cannot be found, return without 
      ** generating any VDBE code. If one can be found, then jump over
      ** the entire DELETE if there are no outstanding deferred constraints
      ** when this statement is run.  */
      FKey *p;
      لكل(p=pTab->u.tab.pFKey; p; p=p->pNextFrom){
        إذا( p->isDeferred || (db->flags & SQLITE_DeferFKs) ) اكسر;
      }
      إذا( !p ) إرجاع;
      iSkip = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_FkIfZero, 1, iSkip); VdbeCoverage(v);
    }

    pParse->disableTriggers = 1;
    sqlite3DeleteFrom(pParse, sqlite3SrcListDup(db, pName, 0), 0, 0, 0);
    pParse->disableTriggers = 0;

    /* If the DELETE has generated immediate foreign key constraint 
    ** violations, halt the VDBE and return an error at this point, before
    ** any modifications to the schema are made. This is because statement
    ** transactions are not able to rollback schema changes.  
    **
    ** If the SQLITE_DeferFKs flag is set, then this is not required, as
    ** the statement transaction will not be rolled back even if FK
    ** constraints are violated.
    */
    إذا( (db->flags & SQLITE_DeferFKs)==0 ){
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      sqlite3VdbeAddOp2(v, OP_FkIfZero, 0, sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
      sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
          OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
    }

    إذا( iSkip ){
      sqlite3VdbeResolveLabel(v, iSkip);
    }
  }
}


/*
** The second argument points to an FKey object representing a foreign key
** for which pTab is the child table. An UPDATE statement against pTab
** is currently being processed. For each column of the table that is 
** actually updated, the corresponding element in the aChange[] array
** is zero or greater (if a column is unmodified the corresponding element
** is set to -1). If the rowid column is modified by the UPDATE statement
** the bChngRowid argument is non-zero.
**
** This function returns true if any of the columns that are part of the
** child key for FK constraint *p are modified.
*/
ساكن صحيح fkChildIsModified(
  Table *pTab,                    /* Table being updated */
  FKey *p,                        /* Foreign key for which pTab is the child */
  صحيح *aChange,                   /* Array indicating modified columns */
  صحيح bChngRowid                  /* True if rowid is modified by this update */
){
  صحيح i;
  لكل(i=0; i<p->nCol; i++){
    صحيح i_Child_مفتاح = p->aCol[i].iFrom;
    إذا( aChange[i_Child_مفتاح]>=0 ) إرجاع 1;
    إذا( i_Child_مفتاح==pTab->iPKey && bChngRowid ) إرجاع 1;
  }
  إرجاع 0;
}

/*
** The second argument points to an FKey object representing a foreign key
** for which pTab is the parent table. An UPDATE statement against pTab
** is currently being processed. For each column of the table that is 
** actually updated, the corresponding element in the aChange[] array
** is zero or greater (if a column is unmodified the corresponding element
** is set to -1). If the rowid column is modified by the UPDATE statement
** the bChngRowid argument is non-zero.
**
** This function returns true if any of the columns that are part of the
** parent key for FK constraint *p are modified.
*/
ساكن صحيح fkParentIsModified(
  Table *pTab, 
  FKey *p, 
  صحيح *aChange, 
  صحيح bChngRowid
){
  صحيح i;
  لكل(i=0; i<p->nCol; i++){
    حرف *z_مفتاح = p->aCol[i].zCol;
    صحيح i_مفتاح;
    لكل(i_مفتاح=0; i_مفتاح<pTab->nCol; i_مفتاح++){
      إذا( aChange[i_مفتاح]>=0 || (i_مفتاح==pTab->iPKey && bChngRowid) ){
        Column *pCol = &pTab->aCol[i_مفتاح];
        إذا( z_مفتاح ){
          إذا( 0==sqlite3StrICmp(pCol->zCnName, z_مفتاح) ) إرجاع 1;
        }وإلا إذا( pCol->colFlags & COLFLAG_PRIMKEY ){
          إرجاع 1;
        }
      }
    }
  }
  إرجاع 0;
}

/*
** Return true if the parser passed as the first argument is being
** used to code a trigger that is really a "SET NULL" action belonging
** to trigger pFKey.
*/
ساكن صحيح isSetNullAction(Parse *pParse, FKey *pFKey){
  Trigger *p = pParse->pTrigCoded;
  إذا( p ){
    إذا( (p==pFKey->apTrigger[0] && pFKey->aAction[0]==OE_SetNull)
     || (p==pFKey->apTrigger[1] && pFKey->aAction[1]==OE_SetNull)
    ){
      أكد( (pParse->db->flags & SQLITE_FkNoAction)==0 );
      إرجاع 1;
    }
  }
  إرجاع 0;
}

/*
** This function is called when inserting, deleting or updating a row of
** table pTab to generate VDBE code to perform foreign key constraint 
** processing for the operation.
**
** For a DELETE operation, parameter regOld is passed the index of the
** first register in an array of (pTab->nCol+1) registers containing the
** rowid of the row being deleted, followed by each of the column values
** of the row being deleted, from left to right. Parameter regNew is passed
** zero in this case.
**
** For an INSERT operation, regOld is passed zero and regNew is passed the
** first register of an array of (pTab->nCol+1) registers containing the new
** row data.
**
** For an UPDATE operation, this function is called twice. Once before
** the original record is deleted from the table using the calling convention
** described for DELETE. Then again after the original record is deleted
** but before the new record is inserted using the INSERT convention. 
*/
فراغ sqlite3FkCheck(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Row is being deleted from this table */ 
  صحيح regOld,                     /* Previous row data is stored here */
  صحيح regNew,                     /* New row data is stored here */
  صحيح *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  صحيح bChngRowid                  /* True if rowid is UPDATEd */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  FKey *pFKey;                    /* Used to iterate through FKs */
  صحيح iDb;                        /* Index of database containing pTab */
  ثابت حرف *zDb;                /* Name of database containing pTab */
  صحيح isIgnoreErrors = pParse->disableTriggers;

  /* Exactly one of regOld and regNew should be non-zero. */
  أكد( (regOld==0)!=(regNew==0) );

  /* If foreign-keys are disabled, this function is a no-op. */
  إذا( (db->flags&SQLITE_Foreign_مفاتيح)==0 ) إرجاع;
  إذا( !IsOrdinaryTable(pTab) ) إرجاع;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  أكد( iDb>=00 && iDb<db->nDb );
  zDb = db->aDb[iDb].zDbSName;

  /* Loop through all the foreign key constraints for which pTab is the
  ** child table (the table that the foreign key definition is part of).  */
  لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
    Table *pTo;                   /* Parent table of foreign key pFKey */
    Index *pIdx = 0;              /* Index on key columns in pTo */
    صحيح *aiFree = 0;
    صحيح *aiCol;
    صحيح iCol;
    صحيح i;
    صحيح bIgnore = 0;

    إذا( aChange 
     && sqlite3_stricmp(pTab->zName, pFKey->zTo)!=0
     && fkChildIsModified(pTab, pFKey, aChange, bChngRowid)==0 
    ){
      استمر;
    }

    /* Find the parent table of this foreign key. Also find a unique index 
    ** on the parent key columns in the parent table. If either of these 
    ** schema items cannot be located, set an error in pParse and return 
    ** early.  */
    إذا( pParse->disableTriggers ){
      pTo = sqlite3FindTable(db, pFKey->zTo, zDb);
    }وإلا{
      pTo = sqlite3LocateTable(pParse, 0, pFKey->zTo, zDb);
    }
    إذا( !pTo || sqlite3FkLocateIndex(pParse, pTo, pFKey, &pIdx, &aiFree) ){
      أكد( isIgnoreErrors==0 || (regOld!=0 && regNew==0) );
      إذا( !isIgnoreErrors || db->mallocFailed ) إرجاع;
      إذا( pTo==0 ){
        /* If isIgnoreErrors is true, then a table is being dropped. In this
        ** case SQLite runs a "DELETE FROM xxx" on the table being dropped
        ** before actually dropping it in order to check FK constraints.
        ** If the parent table of an FK constraint on the current table is
        ** missing, behave as if it is empty. i.e. decrement the relevant
        ** FK counter for each row of the current table with non-NULL keys.
        */
        Vdbe *v = sqlite3GetVdbe(pParse);
        صحيح iJump = sqlite3VdbeCurrentAddr(v) + pFKey->nCol + 1;
        لكل(i=0; i<pFKey->nCol; i++){
          صحيح iFromCol, iReg;
          iFromCol = pFKey->aCol[i].iFrom;
          iReg = sqlite3TableColumnToStorage(pFKey->pFrom,iFromCol) + regOld+1;
          sqlite3VdbeAddOp2(v, OP_IsNull, iReg, iJump); VdbeCoverage(v);
        }
        sqlite3VdbeAddOp2(v, OP_FkCounter, pFKey->isDeferred, -1);
      }
      استمر;
    }
    أكد( pFKey->nCol==1 || (aiFree && pIdx) );

    إذا( aiFree ){
      aiCol = aiFree;
    }وإلا{
      iCol = pFKey->aCol[0].iFrom;
      aiCol = &iCol;
    }
    لكل(i=0; i<pFKey->nCol; i++){
      إذا( aiCol[i]==pTab->iPKey ){
        aiCol[i] = -1;
      }
      أكد( pIdx==0 || pIdx->aiColumn[i]>=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
      /* Request permission to read the parent key columns. If the 
      ** authorization callback returns SQLITE_IGNORE, behave as if any
      ** values read from the parent table are NULL. */
      إذا( db->xAuth ){
        صحيح rcauth;
        حرف *zCol = pTo->aCol[pIdx ? pIdx->aiColumn[i] : pTo->iPKey].zCnName;
        rcauth = sqlite3AuthReadCol(pParse, pTo->zName, zCol, iDb);
        bIgnore = (rcauth==SQLITE_IGNORE);
      }
#endif
    }

    /* Take a shared-cache advisory read-lock on the parent table. Allocate 
    ** a cursor to use to search the unique index on the parent key columns 
    ** in the parent table.  */
    sqlite3TableLock(pParse, iDb, pTo->tnum, 0, pTo->zName);
    pParse->nTab++;

    إذا( regOld!=0 ){
      /* A row is being removed from the child table. Search for the parent.
      ** If the parent does not exist, removing the child row resolves an 
      ** outstanding foreign key constraint violation. */
      fkLookupParent(pParse, iDb, pTo, pIdx, pFKey, aiCol, regOld, -1, bIgnore);
    }
    إذا( regNew!=0 && !isSetNullAction(pParse, pFKey) ){
      /* A row is being added to the child table. If a parent row cannot
      ** be found, adding the child row has violated the FK constraint. 
      **
      ** If this operation is being performed as part of a trigger program
      ** that is actually a "SET NULL" action belonging to this very 
      ** foreign key, then omit this scan altogether. As all child key
      ** values are guaranteed to be NULL, it is not possible for adding
      ** this row to cause an FK violation.  */
      fkLookupParent(pParse, iDb, pTo, pIdx, pFKey, aiCol, regNew, +1, bIgnore);
    }

    sqlite3DbFree(db, aiFree);
  }

  /* Loop through all the foreign key constraints that refer to this table.
  ** (the "child" constraints) */
  لكل(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->pNextTo){
    Index *pIdx = 0;              /* Foreign key index for pFKey */
    SrcList *pSrc;
    صحيح *aiCol = 0;

    إذا( aChange && fkParentIsModified(pTab, pFKey, aChange, bChngRowid)==0 ){
      استمر;
    }

    إذا( !pFKey->isDeferred && !(db->flags & SQLITE_DeferFKs) 
     && !pParse->pToplevel && !pParse->isMultiWrite 
    ){
      أكد( regOld==0 && regNew!=0 );
      /* Inserting a single row into a parent table cannot cause (or fix)
      ** an immediate foreign key violation. So do nothing in this case.  */
      استمر;
    }

    إذا( sqlite3FkLocateIndex(pParse, pTab, pFKey, &pIdx, &aiCol) ){
      إذا( !isIgnoreErrors || db->mallocFailed ) إرجاع;
      استمر;
    }
    أكد( aiCol || pFKey->nCol==1 );

    /* Create a SrcList structure containing the child table.  We need the
    ** child table as a SrcList for sqlite3WhereBegin() */
    pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
    إذا( pSrc ){
      SrcItem *pItem = pSrc->a;
      pItem->pSTab = pFKey->pFrom;
      pItem->zName = pFKey->pFrom->zName;
      pItem->pSTab->nTabRef++;
      pItem->iCursor = pParse->nTab++;
  
      إذا( regNew!=0 ){
        fkScanChildren(pParse, pSrc, pTab, pIdx, pFKey, aiCol, regNew, -1);
      }
      إذا( regOld!=0 ){
        صحيح eAction = pFKey->aAction[aChange!=0];
        إذا( (db->flags & SQLITE_FkNoAction) ) eAction = OE_None;

        fkScanChildren(pParse, pSrc, pTab, pIdx, pFKey, aiCol, regOld, 1);
        /* If this is a deferred FK constraint, or a CASCADE or SET NULL
        ** action applies, then any foreign key violations caused by
        ** removing the parent key will be rectified by the action trigger.
        ** So do not set the "may-abort" flag in this case.
        **
        ** Note 1: If the FK is declared "ON UPDATE CASCADE", then the
        ** may-abort flag will eventually be set on this statement anyway
        ** (when this function is called as part of processing the UPDATE
        ** within the action trigger).
        **
        ** Note 2: At first glance it may seem like SQLite could simply omit
        ** all OP_FkCounter related scans when either CASCADE or SET NULL
        ** applies. The trouble starts if the CASCADE or SET NULL action 
        ** trigger causes other triggers or action rules attached to the 
        ** child table to fire. In these cases the fk constraint counters
        ** might be set incorrectly if any OP_FkCounter related scans are 
        ** omitted.  */
        إذا( !pFKey->isDeferred && eAction!=OE_Cascade && eAction!=OE_SetNull ){
          sqlite3MayAbort(pParse);
        }
      }
      pItem->zName = 0;
      sqlite3SrcListDelete(db, pSrc);
    }
    sqlite3DbFree(db, aiCol);
  }
}

#define COLUMN_MASK(x) (((x)>31) ? 0xffffffff : ((u32)1<<(x)))

/*
** This function is called before generating code to update or delete a 
** row contained in table pTab.
*/
u32 sqlite3FkOldmask(
  Parse *pParse,                  /* Parse context */
  Table *pTab                     /* Table being modified */
){
  u32 mask = 0;
  إذا( pParse->db->flags&SQLITE_Foreign_مفاتيح && IsOrdinaryTable(pTab) ){
    FKey *p;
    صحيح i;
    لكل(p=pTab->u.tab.pFKey; p; p=p->pNextFrom){
      لكل(i=0; i<p->nCol; i++) mask |= COLUMN_MASK(p->aCol[i].iFrom);
    }
    لكل(p=sqlite3FkReferences(pTab); p; p=p->pNextTo){
      Index *pIdx = 0;
      sqlite3FkLocateIndex(pParse, pTab, p, &pIdx, 0);
      إذا( pIdx ){
        لكل(i=0; i<pIdx->n_مفتاح_Col; i++){
          أكد( pIdx->aiColumn[i]>=0 );
          mask |= COLUMN_MASK(pIdx->aiColumn[i]);
        }
      }
    }
  }
  إرجاع mask;
}


/*
** This function is called before generating code to update or delete a 
** row contained in table pTab. If the operation is a DELETE, then
** parameter aChange is passed a NULL value. For an UPDATE, aChange points
** to an array of size N, where N is the number of columns in table pTab.
** If the i'th column is not modified by the UPDATE, then the corresponding 
** entry in the aChange[] array is set to -1. If the column is modified,
** the value is 0 or greater. Parameter chngRowid is set to true if the
** UPDATE statement modifies the rowid fields of the table.
**
** If any foreign key processing will be required, this function returns
** non-zero. If there is no foreign key related processing, this function 
** returns zero.
**
** For an UPDATE, this function returns 2 if:
**
**   * There are any FKs for which pTab is the child and the parent table
**     and any FK processing at all is required (even of a different FK), or
**
**   * the UPDATE modifies one or more parent keys for which the action is
**     not "NO ACTION" (i.e. is CASCADE, SET DEFAULT or SET NULL).
**
** Or, assuming some other foreign key processing is required, 1.
*/
صحيح sqlite3FkRequired(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being modified */
  صحيح *aChange,                   /* Non-NULL for UPDATE operations */
  صحيح chngRowid                   /* True for UPDATE that affects rowid */
){
  صحيح eRet = 1;                   /* Value to return if bHaveFK is true */
  صحيح bHaveFK = 0;                /* If FK processing is required */
  إذا( pParse->db->flags&SQLITE_Foreign_مفاتيح && IsOrdinaryTable(pTab) ){
    إذا( !aChange ){
      /* A DELETE operation. Foreign key processing is required if the 
      ** table in question is either the child or parent table for any 
      ** foreign key constraint.  */
      bHaveFK = (sqlite3FkReferences(pTab) || pTab->u.tab.pFKey);
    }وإلا{
      /* This is an UPDATE. Foreign key processing is only required if the
      ** operation modifies one or more child or parent key columns. */
      FKey *p;

      /* Check if any child key columns are being modified. */
      لكل(p=pTab->u.tab.pFKey; p; p=p->pNextFrom){
        إذا( fkChildIsModified(pTab, p, aChange, chngRowid) ){
          إذا( 0==sqlite3_stricmp(pTab->zName, p->zTo) ) eRet = 2;
          bHaveFK = 1;
        }
      }

      /* Check if any parent key columns are being modified. */
      لكل(p=sqlite3FkReferences(pTab); p; p=p->pNextTo){
        إذا( fkParentIsModified(pTab, p, aChange, chngRowid) ){
          إذا( (pParse->db->flags & SQLITE_FkNoAction)==0 
           && p->aAction[1]!=OE_None 
          ){
            إرجاع 2;
          }
          bHaveFK = 1;
        }
      }
    }
  }
  إرجاع bHaveFK ? eRet : 0;
}

/*
** This function is called when an UPDATE or DELETE operation is being 
** compiled on table pTab, which is the parent table of foreign-key pFKey.
** If the current operation is an UPDATE, then the pChanges parameter is
** passed a pointer to the list of columns being modified. If it is a
** DELETE, pChanges is passed a NULL pointer.
**
** It returns a pointer to a Trigger structure containing a trigger
** equivalent to the ON UPDATE or ON DELETE action specified by pFKey.
** If the action is "NO ACTION" then a NULL pointer is returned (these actions
** require no special handling by the triggers sub-system, code for them is
** created by fkScanChildren()).
**
** For example, if pFKey is the foreign key and pTab is table "p" in 
** the following schema:
**
**   CREATE TABLE p(pk PRIMARY KEY);
**   CREATE TABLE c(ck REFERENCES p ON DELETE CASCADE);
**
** then the returned trigger structure is equivalent to:
**
**   CREATE TRIGGER ... DELETE ON p BEGIN
**     DELETE FROM c WHERE ck = old.pk;
**   END;
**
** The returned pointer is cached as part of the foreign key object. It
** is eventually freed along with the rest of the foreign key object by 
** sqlite3FkDelete().
*/
ساكن Trigger *fkActionTrigger(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being updated or deleted from */
  FKey *pFKey,                    /* Foreign key to get action for */
  تعبير_List *pChanges              /* Change-list for UPDATE, NULL for DELETE */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  صحيح action;                     /* One of OE_None, OE_Cascade etc. */
  Trigger *pTrigger;              /* Trigger definition to return */
  صحيح iAction = (pChanges!=0);    /* 1 for UPDATE, 0 for DELETE */

  action = pFKey->aAction[iAction];
  إذا( (db->flags & SQLITE_FkNoAction) ) action = OE_None;
  إذا( action==OE_Restrict && (db->flags & SQLITE_DeferFKs) ){
    إرجاع 0;
  }
  pTrigger = pFKey->apTrigger[iAction];

  إذا( action!=OE_None && !pTrigger ){
    حرف ثابت *zFrom;            /* Name of child table */
    صحيح nFrom;                    /* Length in bytes of zFrom */
    Index *pIdx = 0;              /* Parent key index for this FK */
    صحيح *aiCol = 0;               /* child table cols -> parent key cols */
    TriggerStep *pStep = 0;        /* First (only) step of trigger program */
    تعبير *pWhere = 0;             /* WHERE clause of trigger step */
    تعبير_List *pList = 0;          /* Changes list if ON UPDATE CASCADE */
    Select *pSelect = 0;          /* If RESTRICT, "SELECT RAISE(...)" */
    صحيح i;                        /* Iterator variable */
    تعبير *pWhen = 0;              /* WHEN clause for the trigger */

    إذا( sqlite3FkLocateIndex(pParse, pTab, pFKey, &pIdx, &aiCol) ) إرجاع 0;
    أكد( aiCol || pFKey->nCol==1 );

    لكل(i=0; i<pFKey->nCol; i++){
      Token tOld = { "old", 3 };  /* Literal "old" token */
      Token tNew = { "new", 3 };  /* Literal "new" token */
      Token tFromCol;             /* Name of column in child table */
      Token tToCol;               /* Name of column in parent table */
      صحيح iFromCol;               /* Idx of column in child table */
      صحيح iToCol;                 /* Idx of column in parent table */
      تعبير *pEq;                  /* OLD.tToCol = tFromCol */
      تعبير *p_يسار;                /* OLD.tToCol */
      تعبير *p_يمين;               /* tFromCol */

      iFromCol = aiCol ? aiCol[i] : pFKey->aCol[0].iFrom;
      أكد( iFromCol>=0 );
      أكد( pIdx!=0 || (pTab->iPKey>=0 && pTab->iPKey<pTab->nCol) );
      أكد( pIdx==0 || pIdx->aiColumn[i]>=0 );
      iToCol = pIdx ? pIdx->aiColumn[i] : pTab->iPKey;
      sqlite3TokenInit(&tToCol, pTab->aCol[iToCol].zCnName);
      sqlite3TokenInit(&tFromCol, pFKey->pFrom->aCol[iFromCol].zCnName);

      p_يسار = sqlite3PExpr(pParse, TK_DOT,
          sqlite3_تعبير_Alloc(db, TK_ID, &tOld, 0),
          sqlite3_تعبير_Alloc(db, TK_ID, &tToCol, 0)
      );
      p_يمين = sqlite3_تعبير_Alloc(db, TK_ID, &tFromCol, 0);

      /* OLD.* columns do not normally have an affinity. So set the affinity
      ** of OLD.tToCol (the parent key) explicitly here. */
      إذا( p_يسار ) p_يسار->aff_تعبير = pTab->aCol[iToCol].affinity;

      /* Create the expression "OLD.tToCol = tFromCol". It is important
      ** that the "OLD.tToCol" term is on the LHS of the = operator, so
      ** that the collation sequence associated with the parent table is
      ** used for the comparison. */
      pEq = fkParentChildEq(pParse, p_يسار, p_يمين, pTab, iToCol,
                            pFKey->pFrom, iFromCol, pFKey->nCol>1);
      pWhere = sqlite3_تعبير_And(pParse, pWhere, pEq);

      /* For ON UPDATE, construct the next term of the WHEN clause.
      ** The final WHEN clause will be like this:
      **
      **    WHEN NOT(old.col1 IS new.col1 AND ... AND old.colN IS new.colN)
      */
      إذا( pChanges ){
        pEq = sqlite3PExpr(pParse, TK_IS,
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_تعبير_Alloc(db, TK_ID, &tOld, 0),
              sqlite3_تعبير_Alloc(db, TK_ID, &tToCol, 0)),
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_تعبير_Alloc(db, TK_ID, &tNew, 0),
              sqlite3_تعبير_Alloc(db, TK_ID, &tToCol, 0))
            );
        pWhen = sqlite3_تعبير_And(pParse, pWhen, pEq);
      }
  
      إذا( action!=OE_Restrict && (action!=OE_Cascade || pChanges) ){
        تعبير *pNew;
        إذا( action==OE_Cascade ){
          pNew = sqlite3PExpr(pParse, TK_DOT, 
            sqlite3_تعبير_Alloc(db, TK_ID, &tNew, 0),
            sqlite3_تعبير_Alloc(db, TK_ID, &tToCol, 0));
        }وإلا إذا( action==OE_SetDflt ){
          Column *pCol = pFKey->pFrom->aCol + iFromCol;
          تعبير *pDflt;
          إذا( pCol->colFlags & COLFLAG_GENERATED ){
            testcase( pCol->colFlags & COLFLAG_VIRTUAL );
            testcase( pCol->colFlags & COLFLAG_STORED );
            pDflt = 0;
          }وإلا{
            pDflt = sqlite3_Column_تعبير_Auth(pFKey->pFrom, pCol, pParse);
          }
          إذا( pDflt ){
            pNew = sqlite3_تعبير_Dup(db, pDflt, 0);
          }وإلا{
            pNew = sqlite3_تعبير_Alloc(db, TK_NULL, 0, 0);
          }
        }وإلا{
          pNew = sqlite3_تعبير_Alloc(db, TK_NULL, 0, 0);
        }
        pList = sqlite3_تعبير_List_Append(pParse, pList, pNew);
        sqlite3_تعبير_List_Set_Name(pParse, pList, &tFromCol, 0);
      }
    }
    sqlite3DbFree(db, aiCol);

    zFrom = pFKey->pFrom->zName;
    nFrom = sqlite3Strlen30(zFrom);

    إذا( action==OE_Restrict ){
      SrcList *pSrc;
      تعبير *pRaise; 

      pRaise = sqlite3_تعبير(db, TK_STRING, "FOREIGN KEY constraint failed"),
      pRaise = sqlite3PExpr(pParse, TK_RAISE, pRaise, 0);
      إذا( pRaise ){
        pRaise->aff_تعبير = OE_Abort;
      }
      pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      إذا( pSrc ){
        SrcItem *pItem = &pSrc->a[0];
        pItem->zName = sqlite3DbStrDup(db, zFrom);
        pItem->fg.fixedSchema = 1;
        pItem->u4.pSchema = pTab->pSchema;
      }
      pSelect = sqlite3SelectNew(pParse, 
          sqlite3_تعبير_List_Append(pParse, 0, pRaise),
          pSrc,
          pWhere,
          0, 0, 0, 0, 0
      );
      pWhere = 0;
    }

    /* Disable lookaside memory allocation */
    DisableLookaside;

    pTrigger = (Trigger *)sqlite3DbMallocZero(db, 
        حجم(Trigger) +         /* struct Trigger */
        حجم(TriggerStep)       /* Single step in trigger program */
    );
    إذا( pTrigger ){
      pStep = pTrigger->step_list = (TriggerStep *)&pTrigger[1];
      pStep->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      إذا( pStep->pSrc ){
        SrcItem *pItem = &pStep->pSrc->a[0];
        pItem->zName = sqlite3DbStrNDup(db, zFrom, nFrom);
        pItem->u4.pSchema = pTab->pSchema;
        pItem->fg.fixedSchema = 1;
      }
      pStep->pWhere = sqlite3_تعبير_Dup(db, pWhere, EXPRDUP_REDUCE);
      pStep->p_تعبير_List = sqlite3_تعبير_List_Dup(db, pList, EXPRDUP_REDUCE);
      pStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
      إذا( pWhen ){
        pWhen = sqlite3PExpr(pParse, TK_NOT, pWhen, 0);
        pTrigger->pWhen = sqlite3_تعبير_Dup(db, pWhen, EXPRDUP_REDUCE);
      }
    }

    /* Re-enable the lookaside buffer, if it was disabled earlier. */
    EnableLookaside;

    sqlite3_تعبير_Delete(db, pWhere);
    sqlite3_تعبير_Delete(db, pWhen);
    sqlite3_تعبير_List_Delete(db, pList);
    sqlite3SelectDelete(db, pSelect);
    إذا( db->mallocFailed==1 ){
      fkTriggerDelete(db, pTrigger);
      إرجاع 0;
    }
    أكد( pStep!=0 );
    أكد( pTrigger!=0 );

    بدل( action ){
      حالة OE_Restrict:
        pStep->op = TK_SELECT;
        اكسر;
      حالة OE_Cascade: 
        إذا( !pChanges ){ 
          pStep->op = TK_DELETE; 
          اكسر; 
        }
        /* no break */ deliberate_fall_through
      افتراضي:
        pStep->op = TK_UPDATE;
    }
    pStep->pTrig = pTrigger;
    pTrigger->pSchema = pTab->pSchema;
    pTrigger->pTabSchema = pTab->pSchema;
    pFKey->apTrigger[iAction] = pTrigger;
    pTrigger->op = (pChanges ? TK_UPDATE : TK_DELETE);
  }

  إرجاع pTrigger;
}

/*
** This function is called when deleting or updating a row to implement
** any required CASCADE, SET NULL or SET DEFAULT actions.
*/
فراغ sqlite3FkActions(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being updated or deleted from */
  تعبير_List *pChanges,             /* Change-list for UPDATE, NULL for DELETE */
  صحيح regOld,                     /* Address of array containing old row */
  صحيح *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  صحيح bChngRowid                  /* True if rowid is UPDATEd */
){
  /* If foreign-key support is enabled, iterate through all FKs that 
  ** refer to table pTab. If there is an action associated with the FK 
  ** for this operation (either update or delete), invoke the associated 
  ** trigger sub-program.  */
  إذا( pParse->db->flags&SQLITE_Foreign_مفاتيح ){
    FKey *pFKey;                  /* Iterator variable */
    لكل(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->pNextTo){
      إذا( aChange==0 || fkParentIsModified(pTab, pFKey, aChange, bChngRowid) ){
        Trigger *pAct = fkActionTrigger(pParse, pTab, pFKey, pChanges);
        إذا( pAct ){
          sqlite3CodeRowTriggerDirect(pParse, pAct, pTab, regOld, OE_Abort, 0);
        }
      }
    }
  }
}

#endif /* ifndef SQLITE_OMIT_TRIGGER */

/*
** Free all memory associated with foreign key definitions attached to
** table pTab. Remove the deleted foreign keys from the Schema.fkeyHash
** hash table.
*/
فراغ sqlite3FkDelete(sqlite3 *db, Table *pTab){
  FKey *pFKey;                    /* Iterator variable */
  FKey *pNext;                    /* Copy of pFKey->pNextFrom */

  أكد( IsOrdinaryTable(pTab) );
  أكد( db!=0 );
  لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pNext){
    أكد( db==0 || sqlite3SchemaMutexHeld(db, 0, pTab->pSchema) );

    /* Remove the FK from the fkeyHash hash table. */
    إذا( db->pnBytesFreed==0 ){
      إذا( pFKey->pPrevTo ){
        pFKey->pPrevTo->pNextTo = pFKey->pNextTo;
      }وإلا{
        ثابت حرف *z = (pFKey->pNextTo ? pFKey->pNextTo->zTo : pFKey->zTo);
        sqlite3_Hash_أدرج(&pTab->pSchema->fkeyHash, z, pFKey->pNextTo);
      }
      إذا( pFKey->pNextTo ){
        pFKey->pNextTo->pPrevTo = pFKey->pPrevTo;
      }
    }

    /* EV: R-30323-21917 Each foreign key constraint in SQLite is
    ** classified as either immediate or deferred.
    */
    أكد( pFKey->isDeferred==0 || pFKey->isDeferred==1 );

    /* Delete any triggers created to implement actions for this FK. */
#ifndef SQLITE_OMIT_TRIGGER
    fkTriggerDelete(db, pFKey->apTrigger[0]);
    fkTriggerDelete(db, pFKey->apTrigger[1]);
#endif

    pNext = pFKey->pNextFrom;
    sqlite3DbFree(db, pFKey);
  }
}
#endif /* ifndef SQLITE_OMIT_FOREIGN_KEY */
