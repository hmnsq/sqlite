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
#অন্তর্ভাব "sqliteInt.h"

#ifndef SQLITE_OMIT_FOREIGN_চাবি
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
পূর্ণসংখ্যা sqlite3_Fk_Locate_সূচক(
  Parse *pParse,                  /* Parse context to store any error in */
  Table *pParent,                 /* Parent table of FK constraint pFKey */
  FKey *pFKey,                    /* Foreign key to find index for */
  সূচক **ppIdx,                  /* OUT: Unique index on parent table */
  পূর্ণসংখ্যা **paiCol                    /* OUT: Map of index columns in pFKey */
){
  সূচক *pIdx = 0;                    /* Value to return via *ppIdx */
  পূর্ণসংখ্যা *aiCol = 0;                     /* Value to return via *paiCol */
  পূর্ণসংখ্যা nCol = pFKey->nCol;             /* Number of columns in parent key */
  অক্ষর *z_চাবি = pFKey->aCol[0].zCol;   /* Name of left-most parent key column */

  /* The caller is responsible for zeroing output parameters. */
  প্রতিজ্ঞা( ppIdx && *ppIdx==0 );
  প্রতিজ্ঞা( !paiCol || *paiCol==0 );
  প্রতিজ্ঞা( pParse );

  /* If this is a non-composite (single column) foreign key, check if it 
  ** maps to the INTEGER PRIMARY KEY of table pParent. If so, leave *ppIdx 
  ** and *paiCol set to zero and return early. 
  **
  ** Otherwise, for a composite foreign key (more than one column), allocate
  ** space for the aiCol array (returned via output parameter *paiCol).
  ** Non-composite foreign keys do not require the aiCol array.
  */
  যদি( nCol==1 ){
    /* The FK maps to the IPK if any of the following are true:
    **
    **   1) There is an INTEGER PRIMARY KEY column and the FK is implicitly 
    **      mapped to the primary key of table pParent, or
    **   2) The FK is explicitly mapped to a column declared as INTEGER
    **      PRIMARY KEY.
    */
    যদি( pParent->iPKey>=0 ){
      যদি( !z_চাবি ) ফেরত 0;
      যদি( !sqlite3StrICmp(pParent->aCol[pParent->iPKey].zCnName, z_চাবি) ){
        ফেরত 0;
      }
    }
  }অন্যথা যদি( paiCol ){
    প্রতিজ্ঞা( nCol>1 );
    aiCol = (পূর্ণসংখ্যা *)sqlite3DbMallocRawNN(pParse->db, nCol*আকার(পূর্ণসংখ্যা));
    যদি( !aiCol ) ফেরত 1;
    *paiCol = aiCol;
  }

  জন্য(pIdx=pParent->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
    যদি( pIdx->n_চাবি_Col==nCol && Is_Unique_সূচক(pIdx) && pIdx->pPartIdxWhere==0 ){ 
      /* pIdx is a UNIQUE index (or a PRIMARY KEY) and has the right number
      ** of columns. If each indexed column corresponds to a foreign key
      ** column of pFKey, then this index is a winner.  */

      যদি( z_চাবি==0 ){
        /* If zKey is NULL, then this foreign key is implicitly mapped to 
        ** the PRIMARY KEY of table pParent. The PRIMARY KEY index may be 
        ** identified by the test.  */
        যদি( Is_Primary_চাবি_সূচক(pIdx) ){
          যদি( aiCol ){
            পূর্ণসংখ্যা i;
            জন্য(i=0; i<nCol; i++) aiCol[i] = pFKey->aCol[i].iFrom;
          }
          বিরতি;
        }
      }অন্যথা{
        /* If zKey is non-NULL, then this foreign key was declared to
        ** map to an explicit list of columns in table pParent. Check if this
        ** index matches those columns. Also, check that the index uses
        ** the default collation sequences for each column. */
        পূর্ণসংখ্যা i, j;
        জন্য(i=0; i<nCol; i++){
          i16 iCol = pIdx->aiColumn[i];     /* Index of column in parent tbl */
          ধ্রুবক অক্ষর *zDfltColl;            /* Def. collation for column */
          অক্ষর *zIdxCol;                    /* Name of indexed column */

          যদি( iCol<0 ) বিরতি; /* No foreign keys against expression indexes */

          /* If the index uses a collation sequence that is different from
          ** the default collation sequence for the column, this index is
          ** unusable. Bail out early in this case.  */
          zDfltColl = sqlite3ColumnColl(&pParent->aCol[iCol]);
          যদি( !zDfltColl ) zDfltColl = sqlite3_Str_দ্বিমিক;
          যদি( sqlite3StrICmp(pIdx->azColl[i], zDfltColl) ) বিরতি;

          zIdxCol = pParent->aCol[iCol].zCnName;
          জন্য(j=0; j<nCol; j++){
            যদি( sqlite3StrICmp(pFKey->aCol[j].zCol, zIdxCol)==0 ){
              যদি( aiCol ) aiCol[i] = pFKey->aCol[j].iFrom;
              বিরতি;
            }
          }
          যদি( j==nCol ) বিরতি;
        }
        যদি( i==nCol ) বিরতি;      /* pIdx is usable */
      }
    }
  }

  যদি( !pIdx ){
    যদি( !pParse->disableTriggers ){
      sqlite3_Error_বার্তা(pParse,
           "foreign key mismatch - \"%w\" referencing \"%w\"",
           pFKey->pFrom->zName, pFKey->zTo);
    }
    sqlite3DbFree(pParse->db, aiCol);
    ফেরত 1;
  }

  *ppIdx = pIdx;
  ফেরত 0;
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
স্থির শূন্য fkLookupParent(
  Parse *pParse,        /* Parse context */
  পূর্ণসংখ্যা iDb,              /* Index of database housing pTab */
  Table *pTab,          /* Parent table of FK pFKey */
  সূচক *pIdx,          /* Unique index on parent key columns in pTab */
  FKey *pFKey,          /* Foreign key constraint */
  পূর্ণসংখ্যা *aiCol,           /* Map from parent key columns to child table columns */
  পূর্ণসংখ্যা reg_তথ্য,          /* Address of array containing child table row */
  পূর্ণসংখ্যা nIncr,            /* Increment constraint counter by this */
  পূর্ণসংখ্যা isIgnore          /* If true, pretend pTab contains all NULL values */
){
  পূর্ণসংখ্যা i;                                    /* Iterator variable */
  Vdbe *v = sqlite3GetVdbe(pParse);         /* Vdbe to add code to */
  পূর্ণসংখ্যা iCur = pParse->nTab - 1;              /* Cursor number to use */
  পূর্ণসংখ্যা iOk = sqlite3VdbeMakeLabel(pParse);   /* jump here if parent key found */

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
  যদি( nIncr<0 ){
    sqlite3VdbeAddOp2(v, OP_FkIfZero, pFKey->isDeferred, iOk);
    VdbeCoverage(v);
  }
  জন্য(i=0; i<pFKey->nCol; i++){
    পূর্ণসংখ্যা iReg = sqlite3TableColumnToStorage(pFKey->pFrom,aiCol[i]) + reg_তথ্য + 1;
    sqlite3VdbeAddOp2(v, OP_IsNull, iReg, iOk); VdbeCoverage(v);
  }

  যদি( isIgnore==0 ){
    যদি( pIdx==0 ){
      /* If pIdx is NULL, then the parent key is the INTEGER PRIMARY KEY
      ** column of the parent table (table pTab).  */
      পূর্ণসংখ্যা iMustBeInt;               /* Address of MustBeInt instruction */
      পূর্ণসংখ্যা reg_অস্থায়ী = sqlite3_Get_অস্থায়ী_Reg(pParse);
  
      /* Invoke MustBeInt to coerce the child key value to an integer (i.e. 
      ** apply the affinity of the parent key). If this fails, then there
      ** is no matching parent key. Before using MustBeInt, make a copy of
      ** the value. Otherwise, the value inserted into the child key column
      ** will have INTEGER affinity applied to it, which may not be correct.  */
      sqlite3VdbeAddOp2(v, OP_SCopy, 
        sqlite3TableColumnToStorage(pFKey->pFrom,aiCol[0])+1+reg_তথ্য, reg_অস্থায়ী);
      iMustBeInt = sqlite3VdbeAddOp2(v, OP_MustBeInt, reg_অস্থায়ী, 0);
      VdbeCoverage(v);
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter.  */
      যদি( pTab==pFKey->pFrom && nIncr==1 ){
        sqlite3VdbeAddOp3(v, OP_Eq, reg_তথ্য, iOk, reg_অস্থায়ী); VdbeCoverage(v);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      }
  
      sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenRead);
      sqlite3VdbeAddOp3(v, OP_NotExists, iCur, 0, reg_অস্থায়ী); VdbeCoverage(v);
      sqlite3VdbeGoto(v, iOk);
      sqlite3VdbeJumpHere(v, sqlite3VdbeCurrentAddr(v)-2);
      sqlite3VdbeJumpHere(v, iMustBeInt);
      sqlite3_Release_অস্থায়ী_Reg(pParse, reg_অস্থায়ী);
    }অন্যথা{
      পূর্ণসংখ্যা nCol = pFKey->nCol;
      পূর্ণসংখ্যা reg_অস্থায়ী = sqlite3_Get_অস্থায়ী_Range(pParse, nCol);
  
      sqlite3VdbeAddOp3(v, OP_OpenRead, iCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_Set_P4_চাবি_Info(pParse, pIdx);
      জন্য(i=0; i<nCol; i++){
        sqlite3VdbeAddOp2(v, OP_Copy, 
               sqlite3TableColumnToStorage(pFKey->pFrom, aiCol[i])+1+reg_তথ্য,
               reg_অস্থায়ী+i);
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
      যদি( pTab==pFKey->pFrom && nIncr==1 ){
        পূর্ণসংখ্যা iJump = sqlite3VdbeCurrentAddr(v) + nCol + 1;
        জন্য(i=0; i<nCol; i++){
          পূর্ণসংখ্যা iChild = sqlite3TableColumnToStorage(pFKey->pFrom,aiCol[i])
                              +1+reg_তথ্য;
          পূর্ণসংখ্যা iParent = 1+reg_তথ্য;
          iParent += sqlite3TableColumnToStorage(pIdx->pTable,
                                                 pIdx->aiColumn[i]);
          প্রতিজ্ঞা( pIdx->aiColumn[i]>=0 );
          প্রতিজ্ঞা( aiCol[i]!=pTab->iPKey );
          যদি( pIdx->aiColumn[i]==pTab->iPKey ){
            /* The parent key is a composite key that includes the IPK column */
            iParent = reg_তথ্য;
          }
          sqlite3VdbeAddOp3(v, OP_Ne, iChild, iJump, iParent); VdbeCoverage(v);
          sqlite3VdbeChangeP5(v, SQLITE_JUMPIFNULL);
        }
        sqlite3VdbeGoto(v, iOk);
      }

      sqlite3VdbeAddOp4(v, OP_Affinity, reg_অস্থায়ী, nCol, 0,
                        sqlite3_সূচক_Affinity_Str(pParse->db,pIdx), nCol);
      sqlite3VdbeAddOp4Int(v, OP_Found, iCur, iOk, reg_অস্থায়ী, nCol);
      VdbeCoverage(v);
      sqlite3_Release_অস্থায়ী_Range(pParse, reg_অস্থায়ী, nCol);
    }
  }

  যদি( !pFKey->isDeferred && !(pParse->db->flags & SQLITE_DeferFKs)
   && !pParse->pToplevel 
   && !pParse->isMultiWrite 
  ){
    /* Special case: If this is an INSERT statement that will insert exactly
    ** one row into the table, raise a constraint immediately instead of
    ** incrementing a counter. This is necessary as the VM code is being
    ** generated for will not open a statement transaction.  */
    প্রতিজ্ঞা( nIncr==1 );
    sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
        OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
  }অন্যথা{
    যদি( nIncr>0 && pFKey->isDeferred==0 ){
      sqlite3MayAbort(pParse);
    }
    sqlite3VdbeAddOp2(v, OP_Fk_গণক, pFKey->isDeferred, nIncr);
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
স্থির রাশিমালা *রাশিমালা_Table_Register(
  Parse *pParse,     /* Parsing and code generating context */
  Table *pTab,       /* The table whose content is at r[regBase]... */
  পূর্ণসংখ্যা regBase,       /* Contents of table pTab */
  i16 iCol           /* Which column of pTab is desired */
){
  রাশিমালা *p_রাশিমালা;
  Column *pCol;
  ধ্রুবক অক্ষর *zColl;
  sqlite3 *db = pParse->db;

  p_রাশিমালা = sqlite3_রাশিমালা(db, TK_REGISTER, 0);
  যদি( p_রাশিমালা ){
    যদি( iCol>=0 && iCol!=pTab->iPKey ){
      pCol = &pTab->aCol[iCol];
      p_রাশিমালা->iTable = regBase + sqlite3TableColumnToStorage(pTab,iCol) + 1;
      p_রাশিমালা->aff_রাশিমালা = pCol->affinity;
      zColl = sqlite3ColumnColl(pCol);
      যদি( zColl==0 ) zColl = db->pDfltColl->zName;
      p_রাশিমালা = sqlite3_রাশিমালা_Add_Collate_String(pParse, p_রাশিমালা, zColl);
    }অন্যথা{
      p_রাশিমালা->iTable = regBase;
      p_রাশিমালা->aff_রাশিমালা = SQLITE_AFF_INTEGER;
    }
  }
  ফেরত p_রাশিমালা;
}

/*
** Return an Expr object that refers to column iCol of table pTab which
** has cursor iCur.
*/
স্থির রাশিমালা *রাশিমালা_Table_Column(
  sqlite3 *db,      /* The database connection */
  Table *pTab,      /* The table whose column is desired */
  পূর্ণসংখ্যা iCursor,      /* The open cursor on the table */
  i16 iCol          /* The column that is wanted */
){
  রাশিমালা *p_রাশিমালা = sqlite3_রাশিমালা(db, TK_COLUMN, 0);
  যদি( p_রাশিমালা ){
    প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
    p_রাশিমালা->y.pTab = pTab;
    p_রাশিমালা->iTable = iCursor;
    p_রাশিমালা->iColumn = iCol;
  }
  ফেরত p_রাশিমালা;
}

/*
** Column iCol1 of table p1 is to be compared to column iCol2 of table p2.
** Return non-zero if the comparison will use an affinity compatible
** with the affinity of column iCol1 of table p1. Or zero if it will not.
**
** This is used in cases where (p1.iCol1 = p2.iCol2) must be evaluated
** using the affinity of p1.iCol1 as part of foreign key processing.
*/
স্থির পূর্ণসংখ্যা fkUsesLhsAffinity(Table *p1, পূর্ণসংখ্যা iCol1, Table *p2, পূর্ণসংখ্যা iCol2){
  u8 aff1 = iCol1>=0 ? p1->aCol[iCol1].affinity : SQLITE_AFF_NUMERIC;
  u8 aff2 = p2->aCol[iCol2].affinity;

  /* If either side has a numeric affinity, then the comparison uses numeric
  ** affinity. So the affinity matches if aff1 is numeric, but not if aff1
  ** is not numeric but aff2 is.  */
  যদি( sqlite3IsNumericAffinity(aff1) ) ফেরত 1;
  যদি( sqlite3IsNumericAffinity(aff2) ) ফেরত 0;

  প্রতিজ্ঞা( aff1==SQLITE_AFF_TEXT || aff1==SQLITE_AFF_BLOB );
  প্রতিজ্ঞা( aff2==SQLITE_AFF_TEXT || aff2==SQLITE_AFF_BLOB );

  /* At this point, either the affinities are the same or one is TEXT and
  ** the other BLOB. In the second case the comparison will be done with
  ** BLOB affinity, which does not match if aff1 is TEXT.  */
  ফেরত (aff1==aff2 || aff1==SQLITE_AFF_BLOB);
}

#ifndef SQLITE_OMIT_CAST
/*
** Return non-zero if a lookup on column iCol of table pTab, using collation
** sequence zColl, can be done using either the rowid or an index. That is,
** if iCol is the INTEGER PRIMARY KEY of pTab, or if pTab has an index
** whose left-most column is iCol and whose collation sequence for that
** column is zColl. Return zero otherwise.
*/
স্থির পূর্ণসংখ্যা fk_Child_চাবি_Is_Indexed(Table *pTab, পূর্ণসংখ্যা iCol, ধ্রুবক অক্ষর *zColl){
  সূচক *pIdx;
  যদি( iCol==pTab->iPKey ) ফেরত 1;
  জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
    যদি( pIdx->aiColumn[0]==iCol
     && pIdx->pPartIdxWhere==0
     && sqlite3StrICmp(pIdx->azColl[0], zColl)==0
    ){
      ফেরত 1;
    }
  }
  ফেরত 0;
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
স্থির রাশিমালা *fkParentChildEq(
  Parse *pParse,            /* Parsing context */
  রাশিমালা *p_বাম,              /* Parent key value */
  রাশিমালা *p_ডান,             /* Unresolved reference to child key column */
  Table *pTab,              /* Parent table */
  পূর্ণসংখ্যা iCol,                 /* Parent key column, or <0 for rowid */
  Table *pChild,            /* Child table */
  পূর্ণসংখ্যা iChildCol,            /* Child key column */
  পূর্ণসংখ্যা bComposite            /* This is part of a composite FK */
){
  sqlite3 *db = pParse->db;
  রাশিমালা *pEq;
#ifndef SQLITE_OMIT_CAST
  রাশিমালা *pIn = 0;
#endif

  যদি( fkUsesLhsAffinity(pTab, iCol, pChild, iChildCol) ){
    ফেরত sqlite3PExpr(pParse, TK_EQ, p_বাম, p_ডান);
  }
  প্রতিজ্ঞা( iCol>=0 );

#ifndef SQLITE_OMIT_CAST
  {
    ধ্রুবক অক্ষর *zColl = sqlite3ColumnColl(&pTab->aCol[iCol]);
    যদি( zColl==0 ) zColl = sqlite3_Str_দ্বিমিক;

    /* The IN term is only useful if it can drive a lookup on the child
    ** table. Omit it otherwise, so that a full scan of the child table
    ** does not have to evaluate it for every row.  */
    যদি( bComposite || fk_Child_চাবি_Is_Indexed(pChild, iChildCol, zColl) ){
      স্থির ধ্রুবক Token tNumeric = { "NUMERIC", 7 };
      রাশিমালা *pCast;
      রাশিমালা_List *pList;

      pCast = sqlite3_রাশিমালা_Alloc(db, TK_CAST, &tNumeric, 0);
      sqlite3_রাশিমালা_Attach_Subtrees(db, pCast, sqlite3_রাশিমালা_Dup(db, p_বাম, 0), 0);
      pList = sqlite3_রাশিমালা_List_Append(pParse, 0,
          sqlite3PExpr(pParse, TK_UPLUS, sqlite3_রাশিমালা_Dup(db, p_বাম, 0), 0)
      );
      pList = sqlite3_রাশিমালা_List_Append(pParse, pList,
          sqlite3PExpr(pParse, TK_UPLUS, pCast, 0)
      );
      pIn = sqlite3PExpr(pParse, TK_IN, sqlite3_রাশিমালা_Add_Collate_String(pParse,
            sqlite3_রাশিমালা_Dup(db, p_ডান, 0), zColl
      ), 0);
      যদি( pIn ){
        pIn->x.pList = pList;
        sqlite3_রাশিমালা_Set_Height_And_Flags(pParse, pIn);
      }অন্যথা{
        sqlite3_রাশিমালা_List_Delete(db, pList);
      }
    }
  }
#endif

  p_ডান = sqlite3PExpr(pParse, TK_UPLUS, p_ডান, 0);
  pEq = sqlite3PExpr(pParse, TK_EQ, p_বাম, p_ডান);
#ifndef SQLITE_OMIT_CAST
  pEq = sqlite3_রাশিমালা_And(pParse, pEq, pIn);
#endif
  ফেরত pEq;
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
স্থির শূন্য fkScanChildren(
  Parse *pParse,                  /* Parse context */
  SrcList *pSrc,                  /* The child table to be scanned */
  Table *pTab,                    /* The parent table */
  সূচক *pIdx,                    /* Index on parent covering the foreign key */
  FKey *pFKey,                    /* The foreign key linking pSrc to pTab */
  পূর্ণসংখ্যা *aiCol,                     /* Map from pIdx cols to child table cols */
  পূর্ণসংখ্যা reg_তথ্য,                    /* Parent row data starts here */
  পূর্ণসংখ্যা nIncr                       /* Amount to increment deferred counter by */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  পূর্ণসংখ্যা i;                          /* Iterator variable */
  রাশিমালা *pWhere = 0;               /* WHERE clause to scan with */
  NameContext sNameContext;       /* Context used to resolve WHERE clause */
  WhereInfo *pWInfo;              /* Context used by sqlite3WhereXXX() */
  পূর্ণসংখ্যা iFkIfZero = 0;              /* Address of OP_FkIfZero */
  Vdbe *v = sqlite3GetVdbe(pParse);

  প্রতিজ্ঞা( pIdx==0 || pIdx->pTable==pTab );
  প্রতিজ্ঞা( pIdx==0 || pIdx->n_চাবি_Col==pFKey->nCol );
  প্রতিজ্ঞা( pIdx!=0 || pFKey->nCol==1 );
  প্রতিজ্ঞা( pIdx!=0 || HasRowid(pTab) );

  যদি( nIncr<0 ){
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
  জন্য(i=0; i<pFKey->nCol; i++){
    রাশিমালা *p_বাম;                  /* Value from parent table row */
    রাশিমালা *p_ডান;                 /* Column ref to child table */
    রাশিমালা *pEq;                    /* Expression (pLeft = pRight) */
    i16 iCol;                     /* Index of column in parent table */
    i16 iCol2;                    /* Index of column in child table */
    ধ্রুবক অক্ষর *zCol;             /* Name of column in child table */

    iCol = pIdx ? pIdx->aiColumn[i] : -1;
    p_বাম = রাশিমালা_Table_Register(pParse, pTab, reg_তথ্য, iCol);
    iCol2 = aiCol ? aiCol[i] : pFKey->aCol[0].iFrom;
    প্রতিজ্ঞা( iCol2>=0 );
    zCol = pFKey->pFrom->aCol[iCol2].zCnName;
    p_ডান = sqlite3_রাশিমালা(db, TK_ID, zCol);
    pEq = fkParentChildEq(pParse, p_বাম, p_ডান, pTab, iCol,
                          pFKey->pFrom, iCol2, pFKey->nCol>0);
    pWhere = sqlite3_রাশিমালা_And(pParse, pWhere, pEq);
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
  যদি( pTab==pFKey->pFrom && nIncr>0 ){
    রাশিমালা *pNe;                    /* Expression (pLeft != pRight) */
    রাশিমালা *p_বাম;                  /* Value from parent table row */
    রাশিমালা *p_ডান;                 /* Column ref to child table */
    যদি( HasRowid(pTab) ){
      p_বাম = রাশিমালা_Table_Register(pParse, pTab, reg_তথ্য, -1);
      p_ডান = রাশিমালা_Table_Column(db, pTab, pSrc->a[0].iCursor, -1);
      pNe = sqlite3PExpr(pParse, TK_NE, p_বাম, p_ডান);
    }অন্যথা{
      রাশিমালা *pEq, *pAll = 0;
      প্রতিজ্ঞা( pIdx!=0 );
      জন্য(i=0; i<pIdx->n_চাবি_Col; i++){
        i16 iCol = pIdx->aiColumn[i];
        প্রতিজ্ঞা( iCol>=0 );
        p_বাম = রাশিমালা_Table_Register(pParse, pTab, reg_তথ্য, iCol);
        p_ডান = sqlite3_রাশিমালা(db, TK_ID, pTab->aCol[iCol].zCnName);
        pEq = sqlite3PExpr(pParse, TK_IS, p_বাম, p_ডান);
        pAll = sqlite3_রাশিমালা_And(pParse, pAll, pEq);
      }
      pNe = sqlite3PExpr(pParse, TK_NOT, pAll, 0);
    }
    pWhere = sqlite3_রাশিমালা_And(pParse, pWhere, pNe);
  }

  /* Resolve the references in the WHERE clause. */
  memset(&sNameContext, 0, আকার(NameContext));
  sNameContext.pSrcList = pSrc;
  sNameContext.pParse = pParse;
  sqlite3_Resolve_রাশিমালা_Names(&sNameContext, pWhere);

  /* Create VDBE to loop through the entries in pSrc that match the WHERE
  ** clause. For each row found, increment either the deferred or immediate
  ** foreign key constraint counter. */
  যদি( pParse->nErr==0 ){
    pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0, 0);
    sqlite3VdbeAddOp2(v, OP_Fk_গণক, pFKey->isDeferred, nIncr);
    যদি( pWInfo ){
      sqlite3WhereEnd(pWInfo);
    }
  }

  /* Clean up the WHERE clause constructed above. */
  sqlite3_রাশিমালা_Delete(db, pWhere);
  যদি( iFkIfZero ){
    sqlite3_Vdbe_Jump_Here_Or_বের_করো_Inst(v, iFkIfZero);
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
  ফেরত (FKey *)sqlite3HashFind(&pTab->pSchema->fkeyHash, pTab->zName);
}

/*
** The second argument is a Trigger structure allocated by the 
** fkActionTrigger() routine. This function deletes the Trigger structure
** and all of its sub-components.
**
** The Trigger structure or any of its sub-components may be allocated from
** the lookaside buffer belonging to database handle dbMem.
*/
স্থির শূন্য fkTriggerDelete(sqlite3 *dbMem, Trigger *p){
  যদি( p ){
    TriggerStep *pStep = p->step_list;
    sqlite3SrcListDelete(dbMem, pStep->pSrc);
    sqlite3_রাশিমালা_Delete(dbMem, pStep->pWhere);
    sqlite3_রাশিমালা_List_Delete(dbMem, pStep->p_রাশিমালা_List);
    sqlite3SelectDelete(dbMem, pStep->pSelect);
    sqlite3_রাশিমালা_Delete(dbMem, p->pWhen);
    sqlite3DbFree(dbMem, p);
  }
}

/*
** Clear the apTrigger[] cache of CASCADE triggers for all foreign keys
** in a particular database.  This needs to happen when the schema
** changes.
*/
শূন্য sqlite3FkClearTriggerCache(sqlite3 *db, পূর্ণসংখ্যা iDb){
  HashElem *k;
  Hash *pHash = &db->aDb[iDb].pSchema->tblHash;
  জন্য(k=sqliteHashFirst(pHash); k; k=sqlite_Hash_পরবর্তী(k)){
    Table *pTab = sqlite_Hash_তথ্য(k);
    FKey *pFKey;
    যদি( !IsOrdinaryTable(pTab) ) চালু;
    জন্য(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_পরবর্তী_From){
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
শূন্য sqlite3FkDropTable(Parse *pParse, SrcList *pName, Table *pTab){
  sqlite3 *db = pParse->db;
  যদি( (db->flags&SQLITE_Foreign_চাবিগুলি) && IsOrdinaryTable(pTab) ){
    পূর্ণসংখ্যা iSkip = 0;
    Vdbe *v = sqlite3GetVdbe(pParse);

    প্রতিজ্ঞা( v );                  /* VDBE has already been allocated */
    প্রতিজ্ঞা( IsOrdinaryTable(pTab) );
    যদি( sqlite3FkReferences(pTab)==0 ){
      /* Search for a deferred foreign key constraint for which this table
      ** is the child table. If one cannot be found, return without 
      ** generating any VDBE code. If one can be found, then jump over
      ** the entire DELETE if there are no outstanding deferred constraints
      ** when this statement is run.  */
      FKey *p;
      জন্য(p=pTab->u.tab.pFKey; p; p=p->p_পরবর্তী_From){
        যদি( p->isDeferred || (db->flags & SQLITE_DeferFKs) ) বিরতি;
      }
      যদি( !p ) ফেরত;
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
    যদি( (db->flags & SQLITE_DeferFKs)==0 ){
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      sqlite3VdbeAddOp2(v, OP_FkIfZero, 0, sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
      sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
          OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
    }

    যদি( iSkip ){
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
স্থির পূর্ণসংখ্যা fkChildIsModified(
  Table *pTab,                    /* Table being updated */
  FKey *p,                        /* Foreign key for which pTab is the child */
  পূর্ণসংখ্যা *aChange,                   /* Array indicating modified columns */
  পূর্ণসংখ্যা bChngRowid                  /* True if rowid is modified by this update */
){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<p->nCol; i++){
    পূর্ণসংখ্যা i_Child_চাবি = p->aCol[i].iFrom;
    যদি( aChange[i_Child_চাবি]>=0 ) ফেরত 1;
    যদি( i_Child_চাবি==pTab->iPKey && bChngRowid ) ফেরত 1;
  }
  ফেরত 0;
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
স্থির পূর্ণসংখ্যা fkParentIsModified(
  Table *pTab, 
  FKey *p, 
  পূর্ণসংখ্যা *aChange, 
  পূর্ণসংখ্যা bChngRowid
){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<p->nCol; i++){
    অক্ষর *z_চাবি = p->aCol[i].zCol;
    পূর্ণসংখ্যা i_চাবি;
    জন্য(i_চাবি=0; i_চাবি<pTab->nCol; i_চাবি++){
      যদি( aChange[i_চাবি]>=0 || (i_চাবি==pTab->iPKey && bChngRowid) ){
        Column *pCol = &pTab->aCol[i_চাবি];
        যদি( z_চাবি ){
          যদি( 0==sqlite3StrICmp(pCol->zCnName, z_চাবি) ) ফেরত 1;
        }অন্যথা যদি( pCol->colFlags & COLFLAG_PRIMKEY ){
          ফেরত 1;
        }
      }
    }
  }
  ফেরত 0;
}

/*
** Return true if the parser passed as the first argument is being
** used to code a trigger that is really a "SET NULL" action belonging
** to trigger pFKey.
*/
স্থির পূর্ণসংখ্যা isSetNullAction(Parse *pParse, FKey *pFKey){
  Trigger *p = pParse->pTrigCoded;
  যদি( p ){
    যদি( (p==pFKey->apTrigger[0] && pFKey->aAction[0]==OE_SetNull)
     || (p==pFKey->apTrigger[1] && pFKey->aAction[1]==OE_SetNull)
    ){
      প্রতিজ্ঞা( (pParse->db->flags & SQLITE_FkNoAction)==0 );
      ফেরত 1;
    }
  }
  ফেরত 0;
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
শূন্য sqlite3FkCheck(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Row is being deleted from this table */ 
  পূর্ণসংখ্যা regOld,                     /* Previous row data is stored here */
  পূর্ণসংখ্যা regNew,                     /* New row data is stored here */
  পূর্ণসংখ্যা *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  পূর্ণসংখ্যা bChngRowid                  /* True if rowid is UPDATEd */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  FKey *pFKey;                    /* Used to iterate through FKs */
  পূর্ণসংখ্যা iDb;                        /* Index of database containing pTab */
  ধ্রুবক অক্ষর *zDb;                /* Name of database containing pTab */
  পূর্ণসংখ্যা isIgnoreErrors = pParse->disableTriggers;

  /* Exactly one of regOld and regNew should be non-zero. */
  প্রতিজ্ঞা( (regOld==0)!=(regNew==0) );

  /* If foreign-keys are disabled, this function is a no-op. */
  যদি( (db->flags&SQLITE_Foreign_চাবিগুলি)==0 ) ফেরত;
  যদি( !IsOrdinaryTable(pTab) ) ফেরত;

  iDb = sqlite3_Schema_To_সূচক(db, pTab->pSchema);
  প্রতিজ্ঞা( iDb>=00 && iDb<db->nDb );
  zDb = db->aDb[iDb].zDbSName;

  /* Loop through all the foreign key constraints for which pTab is the
  ** child table (the table that the foreign key definition is part of).  */
  জন্য(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_পরবর্তী_From){
    Table *pTo;                   /* Parent table of foreign key pFKey */
    সূচক *pIdx = 0;              /* Index on key columns in pTo */
    পূর্ণসংখ্যা *aiFree = 0;
    পূর্ণসংখ্যা *aiCol;
    পূর্ণসংখ্যা iCol;
    পূর্ণসংখ্যা i;
    পূর্ণসংখ্যা bIgnore = 0;

    যদি( aChange 
     && sqlite3_stricmp(pTab->zName, pFKey->zTo)!=0
     && fkChildIsModified(pTab, pFKey, aChange, bChngRowid)==0 
    ){
      চালু;
    }

    /* Find the parent table of this foreign key. Also find a unique index 
    ** on the parent key columns in the parent table. If either of these 
    ** schema items cannot be located, set an error in pParse and return 
    ** early.  */
    যদি( pParse->disableTriggers ){
      pTo = sqlite3FindTable(db, pFKey->zTo, zDb);
    }অন্যথা{
      pTo = sqlite3LocateTable(pParse, 0, pFKey->zTo, zDb);
    }
    যদি( !pTo || sqlite3_Fk_Locate_সূচক(pParse, pTo, pFKey, &pIdx, &aiFree) ){
      প্রতিজ্ঞা( isIgnoreErrors==0 || (regOld!=0 && regNew==0) );
      যদি( !isIgnoreErrors || db->mallocFailed ) ফেরত;
      যদি( pTo==0 ){
        /* If isIgnoreErrors is true, then a table is being dropped. In this
        ** case SQLite runs a "DELETE FROM xxx" on the table being dropped
        ** before actually dropping it in order to check FK constraints.
        ** If the parent table of an FK constraint on the current table is
        ** missing, behave as if it is empty. i.e. decrement the relevant
        ** FK counter for each row of the current table with non-NULL keys.
        */
        Vdbe *v = sqlite3GetVdbe(pParse);
        পূর্ণসংখ্যা iJump = sqlite3VdbeCurrentAddr(v) + pFKey->nCol + 1;
        জন্য(i=0; i<pFKey->nCol; i++){
          পূর্ণসংখ্যা iFromCol, iReg;
          iFromCol = pFKey->aCol[i].iFrom;
          iReg = sqlite3TableColumnToStorage(pFKey->pFrom,iFromCol) + regOld+1;
          sqlite3VdbeAddOp2(v, OP_IsNull, iReg, iJump); VdbeCoverage(v);
        }
        sqlite3VdbeAddOp2(v, OP_Fk_গণক, pFKey->isDeferred, -1);
      }
      চালু;
    }
    প্রতিজ্ঞা( pFKey->nCol==1 || (aiFree && pIdx) );

    যদি( aiFree ){
      aiCol = aiFree;
    }অন্যথা{
      iCol = pFKey->aCol[0].iFrom;
      aiCol = &iCol;
    }
    জন্য(i=0; i<pFKey->nCol; i++){
      যদি( aiCol[i]==pTab->iPKey ){
        aiCol[i] = -1;
      }
      প্রতিজ্ঞা( pIdx==0 || pIdx->aiColumn[i]>=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
      /* Request permission to read the parent key columns. If the 
      ** authorization callback returns SQLITE_IGNORE, behave as if any
      ** values read from the parent table are NULL. */
      যদি( db->xAuth ){
        পূর্ণসংখ্যা rcauth;
        অক্ষর *zCol = pTo->aCol[pIdx ? pIdx->aiColumn[i] : pTo->iPKey].zCnName;
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

    যদি( regOld!=0 ){
      /* A row is being removed from the child table. Search for the parent.
      ** If the parent does not exist, removing the child row resolves an 
      ** outstanding foreign key constraint violation. */
      fkLookupParent(pParse, iDb, pTo, pIdx, pFKey, aiCol, regOld, -1, bIgnore);
    }
    যদি( regNew!=0 && !isSetNullAction(pParse, pFKey) ){
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
  জন্য(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_পরবর্তী_To){
    সূচক *pIdx = 0;              /* Foreign key index for pFKey */
    SrcList *pSrc;
    পূর্ণসংখ্যা *aiCol = 0;

    যদি( aChange && fkParentIsModified(pTab, pFKey, aChange, bChngRowid)==0 ){
      চালু;
    }

    যদি( !pFKey->isDeferred && !(db->flags & SQLITE_DeferFKs) 
     && !pParse->pToplevel && !pParse->isMultiWrite 
    ){
      প্রতিজ্ঞা( regOld==0 && regNew!=0 );
      /* Inserting a single row into a parent table cannot cause (or fix)
      ** an immediate foreign key violation. So do nothing in this case.  */
      চালু;
    }

    যদি( sqlite3_Fk_Locate_সূচক(pParse, pTab, pFKey, &pIdx, &aiCol) ){
      যদি( !isIgnoreErrors || db->mallocFailed ) ফেরত;
      চালু;
    }
    প্রতিজ্ঞা( aiCol || pFKey->nCol==1 );

    /* Create a SrcList structure containing the child table.  We need the
    ** child table as a SrcList for sqlite3WhereBegin() */
    pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
    যদি( pSrc ){
      SrcItem *pItem = pSrc->a;
      pItem->pSTab = pFKey->pFrom;
      pItem->zName = pFKey->pFrom->zName;
      pItem->pSTab->nTabRef++;
      pItem->iCursor = pParse->nTab++;
  
      যদি( regNew!=0 ){
        fkScanChildren(pParse, pSrc, pTab, pIdx, pFKey, aiCol, regNew, -1);
      }
      যদি( regOld!=0 ){
        পূর্ণসংখ্যা eAction = pFKey->aAction[aChange!=0];
        যদি( (db->flags & SQLITE_FkNoAction) ) eAction = OE_None;

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
        যদি( !pFKey->isDeferred && eAction!=OE_Cascade && eAction!=OE_SetNull ){
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
  যদি( pParse->db->flags&SQLITE_Foreign_চাবিগুলি && IsOrdinaryTable(pTab) ){
    FKey *p;
    পূর্ণসংখ্যা i;
    জন্য(p=pTab->u.tab.pFKey; p; p=p->p_পরবর্তী_From){
      জন্য(i=0; i<p->nCol; i++) mask |= COLUMN_MASK(p->aCol[i].iFrom);
    }
    জন্য(p=sqlite3FkReferences(pTab); p; p=p->p_পরবর্তী_To){
      সূচক *pIdx = 0;
      sqlite3_Fk_Locate_সূচক(pParse, pTab, p, &pIdx, 0);
      যদি( pIdx ){
        জন্য(i=0; i<pIdx->n_চাবি_Col; i++){
          প্রতিজ্ঞা( pIdx->aiColumn[i]>=0 );
          mask |= COLUMN_MASK(pIdx->aiColumn[i]);
        }
      }
    }
  }
  ফেরত mask;
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
পূর্ণসংখ্যা sqlite3FkRequired(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being modified */
  পূর্ণসংখ্যা *aChange,                   /* Non-NULL for UPDATE operations */
  পূর্ণসংখ্যা chngRowid                   /* True for UPDATE that affects rowid */
){
  পূর্ণসংখ্যা eRet = 1;                   /* Value to return if bHaveFK is true */
  পূর্ণসংখ্যা bHaveFK = 0;                /* If FK processing is required */
  যদি( pParse->db->flags&SQLITE_Foreign_চাবিগুলি && IsOrdinaryTable(pTab) ){
    যদি( !aChange ){
      /* A DELETE operation. Foreign key processing is required if the 
      ** table in question is either the child or parent table for any 
      ** foreign key constraint.  */
      bHaveFK = (sqlite3FkReferences(pTab) || pTab->u.tab.pFKey);
    }অন্যথা{
      /* This is an UPDATE. Foreign key processing is only required if the
      ** operation modifies one or more child or parent key columns. */
      FKey *p;

      /* Check if any child key columns are being modified. */
      জন্য(p=pTab->u.tab.pFKey; p; p=p->p_পরবর্তী_From){
        যদি( fkChildIsModified(pTab, p, aChange, chngRowid) ){
          যদি( 0==sqlite3_stricmp(pTab->zName, p->zTo) ) eRet = 2;
          bHaveFK = 1;
        }
      }

      /* Check if any parent key columns are being modified. */
      জন্য(p=sqlite3FkReferences(pTab); p; p=p->p_পরবর্তী_To){
        যদি( fkParentIsModified(pTab, p, aChange, chngRowid) ){
          যদি( (pParse->db->flags & SQLITE_FkNoAction)==0 
           && p->aAction[1]!=OE_None 
          ){
            ফেরত 2;
          }
          bHaveFK = 1;
        }
      }
    }
  }
  ফেরত bHaveFK ? eRet : 0;
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
স্থির Trigger *fkActionTrigger(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being updated or deleted from */
  FKey *pFKey,                    /* Foreign key to get action for */
  রাশিমালা_List *pChanges              /* Change-list for UPDATE, NULL for DELETE */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  পূর্ণসংখ্যা action;                     /* One of OE_None, OE_Cascade etc. */
  Trigger *pTrigger;              /* Trigger definition to return */
  পূর্ণসংখ্যা iAction = (pChanges!=0);    /* 1 for UPDATE, 0 for DELETE */

  action = pFKey->aAction[iAction];
  যদি( (db->flags & SQLITE_FkNoAction) ) action = OE_None;
  যদি( action==OE_Restrict && (db->flags & SQLITE_DeferFKs) ){
    ফেরত 0;
  }
  pTrigger = pFKey->apTrigger[iAction];

  যদি( action!=OE_None && !pTrigger ){
    অক্ষর ধ্রুবক *zFrom;            /* Name of child table */
    পূর্ণসংখ্যা nFrom;                    /* Length in bytes of zFrom */
    সূচক *pIdx = 0;              /* Parent key index for this FK */
    পূর্ণসংখ্যা *aiCol = 0;               /* child table cols -> parent key cols */
    TriggerStep *pStep = 0;        /* First (only) step of trigger program */
    রাশিমালা *pWhere = 0;             /* WHERE clause of trigger step */
    রাশিমালা_List *pList = 0;          /* Changes list if ON UPDATE CASCADE */
    Select *pSelect = 0;          /* If RESTRICT, "SELECT RAISE(...)" */
    পূর্ণসংখ্যা i;                        /* Iterator variable */
    রাশিমালা *pWhen = 0;              /* WHEN clause for the trigger */

    যদি( sqlite3_Fk_Locate_সূচক(pParse, pTab, pFKey, &pIdx, &aiCol) ) ফেরত 0;
    প্রতিজ্ঞা( aiCol || pFKey->nCol==1 );

    জন্য(i=0; i<pFKey->nCol; i++){
      Token tOld = { "old", 3 };  /* Literal "old" token */
      Token tNew = { "new", 3 };  /* Literal "new" token */
      Token tFromCol;             /* Name of column in child table */
      Token tToCol;               /* Name of column in parent table */
      পূর্ণসংখ্যা iFromCol;               /* Idx of column in child table */
      পূর্ণসংখ্যা iToCol;                 /* Idx of column in parent table */
      রাশিমালা *pEq;                  /* OLD.tToCol = tFromCol */
      রাশিমালা *p_বাম;                /* OLD.tToCol */
      রাশিমালা *p_ডান;               /* tFromCol */

      iFromCol = aiCol ? aiCol[i] : pFKey->aCol[0].iFrom;
      প্রতিজ্ঞা( iFromCol>=0 );
      প্রতিজ্ঞা( pIdx!=0 || (pTab->iPKey>=0 && pTab->iPKey<pTab->nCol) );
      প্রতিজ্ঞা( pIdx==0 || pIdx->aiColumn[i]>=0 );
      iToCol = pIdx ? pIdx->aiColumn[i] : pTab->iPKey;
      sqlite3TokenInit(&tToCol, pTab->aCol[iToCol].zCnName);
      sqlite3TokenInit(&tFromCol, pFKey->pFrom->aCol[iFromCol].zCnName);

      p_বাম = sqlite3PExpr(pParse, TK_DOT,
          sqlite3_রাশিমালা_Alloc(db, TK_ID, &tOld, 0),
          sqlite3_রাশিমালা_Alloc(db, TK_ID, &tToCol, 0)
      );
      p_ডান = sqlite3_রাশিমালা_Alloc(db, TK_ID, &tFromCol, 0);

      /* OLD.* columns do not normally have an affinity. So set the affinity
      ** of OLD.tToCol (the parent key) explicitly here. */
      যদি( p_বাম ) p_বাম->aff_রাশিমালা = pTab->aCol[iToCol].affinity;

      /* Create the expression "OLD.tToCol = tFromCol". It is important
      ** that the "OLD.tToCol" term is on the LHS of the = operator, so
      ** that the collation sequence associated with the parent table is
      ** used for the comparison. */
      pEq = fkParentChildEq(pParse, p_বাম, p_ডান, pTab, iToCol,
                            pFKey->pFrom, iFromCol, pFKey->nCol>1);
      pWhere = sqlite3_রাশিমালা_And(pParse, pWhere, pEq);

      /* For ON UPDATE, construct the next term of the WHEN clause.
      ** The final WHEN clause will be like this:
      **
      **    WHEN NOT(old.col1 IS new.col1 AND ... AND old.colN IS new.colN)
      */
      যদি( pChanges ){
        pEq = sqlite3PExpr(pParse, TK_IS,
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_রাশিমালা_Alloc(db, TK_ID, &tOld, 0),
              sqlite3_রাশিমালা_Alloc(db, TK_ID, &tToCol, 0)),
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_রাশিমালা_Alloc(db, TK_ID, &tNew, 0),
              sqlite3_রাশিমালা_Alloc(db, TK_ID, &tToCol, 0))
            );
        pWhen = sqlite3_রাশিমালা_And(pParse, pWhen, pEq);
      }
  
      যদি( action!=OE_Restrict && (action!=OE_Cascade || pChanges) ){
        রাশিমালা *pNew;
        যদি( action==OE_Cascade ){
          pNew = sqlite3PExpr(pParse, TK_DOT, 
            sqlite3_রাশিমালা_Alloc(db, TK_ID, &tNew, 0),
            sqlite3_রাশিমালা_Alloc(db, TK_ID, &tToCol, 0));
        }অন্যথা যদি( action==OE_SetDflt ){
          Column *pCol = pFKey->pFrom->aCol + iFromCol;
          রাশিমালা *pDflt;
          যদি( pCol->colFlags & COLFLAG_GENERATED ){
            testcase( pCol->colFlags & COLFLAG_VIRTUAL );
            testcase( pCol->colFlags & COLFLAG_STORED );
            pDflt = 0;
          }অন্যথা{
            pDflt = sqlite3_Column_রাশিমালা_Auth(pFKey->pFrom, pCol, pParse);
          }
          যদি( pDflt ){
            pNew = sqlite3_রাশিমালা_Dup(db, pDflt, 0);
          }অন্যথা{
            pNew = sqlite3_রাশিমালা_Alloc(db, TK_NULL, 0, 0);
          }
        }অন্যথা{
          pNew = sqlite3_রাশিমালা_Alloc(db, TK_NULL, 0, 0);
        }
        pList = sqlite3_রাশিমালা_List_Append(pParse, pList, pNew);
        sqlite3_রাশিমালা_List_Set_Name(pParse, pList, &tFromCol, 0);
      }
    }
    sqlite3DbFree(db, aiCol);

    zFrom = pFKey->pFrom->zName;
    nFrom = sqlite3Strlen30(zFrom);

    যদি( action==OE_Restrict ){
      SrcList *pSrc;
      রাশিমালা *pRaise; 

      pRaise = sqlite3_রাশিমালা(db, TK_STRING, "FOREIGN KEY constraint failed"),
      pRaise = sqlite3PExpr(pParse, TK_RAISE, pRaise, 0);
      যদি( pRaise ){
        pRaise->aff_রাশিমালা = OE_Abort;
      }
      pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      যদি( pSrc ){
        SrcItem *pItem = &pSrc->a[0];
        pItem->zName = sqlite3DbStrDup(db, zFrom);
        pItem->fg.fixedSchema = 1;
        pItem->u4.pSchema = pTab->pSchema;
      }
      pSelect = sqlite3SelectNew(pParse, 
          sqlite3_রাশিমালা_List_Append(pParse, 0, pRaise),
          pSrc,
          pWhere,
          0, 0, 0, 0, 0
      );
      pWhere = 0;
    }

    /* Disable lookaside memory allocation */
    DisableLookaside;

    pTrigger = (Trigger *)sqlite3DbMallocZero(db, 
        আকার(Trigger) +         /* struct Trigger */
        আকার(TriggerStep)       /* Single step in trigger program */
    );
    যদি( pTrigger ){
      pStep = pTrigger->step_list = (TriggerStep *)&pTrigger[1];
      pStep->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      যদি( pStep->pSrc ){
        SrcItem *pItem = &pStep->pSrc->a[0];
        pItem->zName = sqlite3DbStrNDup(db, zFrom, nFrom);
        pItem->u4.pSchema = pTab->pSchema;
        pItem->fg.fixedSchema = 1;
      }
      pStep->pWhere = sqlite3_রাশিমালা_Dup(db, pWhere, EXPRDUP_REDUCE);
      pStep->p_রাশিমালা_List = sqlite3_রাশিমালা_List_Dup(db, pList, EXPRDUP_REDUCE);
      pStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
      যদি( pWhen ){
        pWhen = sqlite3PExpr(pParse, TK_NOT, pWhen, 0);
        pTrigger->pWhen = sqlite3_রাশিমালা_Dup(db, pWhen, EXPRDUP_REDUCE);
      }
    }

    /* Re-enable the lookaside buffer, if it was disabled earlier. */
    EnableLookaside;

    sqlite3_রাশিমালা_Delete(db, pWhere);
    sqlite3_রাশিমালা_Delete(db, pWhen);
    sqlite3_রাশিমালা_List_Delete(db, pList);
    sqlite3SelectDelete(db, pSelect);
    যদি( db->mallocFailed==1 ){
      fkTriggerDelete(db, pTrigger);
      ফেরত 0;
    }
    প্রতিজ্ঞা( pStep!=0 );
    প্রতিজ্ঞা( pTrigger!=0 );

    নির্বাচন( action ){
      ক্ষেত্র OE_Restrict:
        pStep->op = TK_SELECT;
        বিরতি;
      ক্ষেত্র OE_Cascade: 
        যদি( !pChanges ){ 
          pStep->op = TK_DELETE; 
          বিরতি; 
        }
        /* no break */ deliberate_fall_through
      পূর্বনির্ধারিত:
        pStep->op = TK_UPDATE;
    }
    pStep->pTrig = pTrigger;
    pTrigger->pSchema = pTab->pSchema;
    pTrigger->pTabSchema = pTab->pSchema;
    pFKey->apTrigger[iAction] = pTrigger;
    pTrigger->op = (pChanges ? TK_UPDATE : TK_DELETE);
  }

  ফেরত pTrigger;
}

/*
** This function is called when deleting or updating a row to implement
** any required CASCADE, SET NULL or SET DEFAULT actions.
*/
শূন্য sqlite3FkActions(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being updated or deleted from */
  রাশিমালা_List *pChanges,             /* Change-list for UPDATE, NULL for DELETE */
  পূর্ণসংখ্যা regOld,                     /* Address of array containing old row */
  পূর্ণসংখ্যা *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  পূর্ণসংখ্যা bChngRowid                  /* True if rowid is UPDATEd */
){
  /* If foreign-key support is enabled, iterate through all FKs that 
  ** refer to table pTab. If there is an action associated with the FK 
  ** for this operation (either update or delete), invoke the associated 
  ** trigger sub-program.  */
  যদি( pParse->db->flags&SQLITE_Foreign_চাবিগুলি ){
    FKey *pFKey;                  /* Iterator variable */
    জন্য(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_পরবর্তী_To){
      যদি( aChange==0 || fkParentIsModified(pTab, pFKey, aChange, bChngRowid) ){
        Trigger *pAct = fkActionTrigger(pParse, pTab, pFKey, pChanges);
        যদি( pAct ){
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
শূন্য sqlite3FkDelete(sqlite3 *db, Table *pTab){
  FKey *pFKey;                    /* Iterator variable */
  FKey *p_পরবর্তী;                    /* Copy of pFKey->pNextFrom */

  প্রতিজ্ঞা( IsOrdinaryTable(pTab) );
  প্রতিজ্ঞা( db!=0 );
  জন্য(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=p_পরবর্তী){
    প্রতিজ্ঞা( db==0 || sqlite3SchemaMutexHeld(db, 0, pTab->pSchema) );

    /* Remove the FK from the fkeyHash hash table. */
    যদি( db->pnBytesFreed==0 ){
      যদি( pFKey->p_পূর্ব_To ){
        pFKey->p_পূর্ব_To->p_পরবর্তী_To = pFKey->p_পরবর্তী_To;
      }অন্যথা{
        ধ্রুবক অক্ষর *z = (pFKey->p_পরবর্তী_To ? pFKey->p_পরবর্তী_To->zTo : pFKey->zTo);
        sqlite3_Hash_ঢোকাও(&pTab->pSchema->fkeyHash, z, pFKey->p_পরবর্তী_To);
      }
      যদি( pFKey->p_পরবর্তী_To ){
        pFKey->p_পরবর্তী_To->p_পূর্ব_To = pFKey->p_পূর্ব_To;
      }
    }

    /* EV: R-30323-21917 Each foreign key constraint in SQLite is
    ** classified as either immediate or deferred.
    */
    প্রতিজ্ঞা( pFKey->isDeferred==0 || pFKey->isDeferred==1 );

    /* Delete any triggers created to implement actions for this FK. */
#ifndef SQLITE_OMIT_TRIGGER
    fkTriggerDelete(db, pFKey->apTrigger[0]);
    fkTriggerDelete(db, pFKey->apTrigger[1]);
#endif

    p_পরবর্তী = pFKey->p_পরবর্তী_From;
    sqlite3DbFree(db, pFKey);
  }
}
#endif /* ifndef SQLITE_OMIT_FOREIGN_KEY */
