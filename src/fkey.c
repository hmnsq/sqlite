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

#که_نه_تعریف_شوی SQLITE_OMIT_FOREIGN_کلی
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER

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
صحيح sqlite3_Fk_Locate_شاخص(
  Parse *pParse,                  /* Parse context to store any error in */
  جدول *p_پلار,                 /* Parent table of FK constraint pFKey */
  FKey *pFKey,                    /* Foreign key to find index for */
  شاخص **ppIdx,                  /* OUT: Unique index on parent table */
  صحيح **pai_ستون                    /* OUT: Map of index columns in pFKey */
){
  شاخص *pIdx = 0;                    /* Value to return via *ppIdx */
  صحيح *ai_ستون = 0;                     /* Value to return via *paiCol */
  صحيح n_ستون = pFKey->n_ستون;             /* Number of columns in parent key */
  حرف *z_کلی = pFKey->a_ستون[0].z_ستون;   /* Name of left-most parent key column */

  /* The caller is responsible for zeroing output parameters. */
  أكد( ppIdx && *ppIdx==0 );
  أكد( !pai_ستون || *pai_ستون==0 );
  أكد( pParse );

  /* If this is a non-composite (single column) foreign key, check if it 
  ** maps to the INTEGER PRIMARY KEY of table pParent. If so, leave *ppIdx 
  ** and *paiCol set to zero and return early. 
  **
  ** Otherwise, for a composite foreign key (more than one column), allocate
  ** space for the aiCol array (returned via output parameter *paiCol).
  ** Non-composite foreign keys do not require the aiCol array.
  */
  إذا( n_ستون==1 ){
    /* The FK maps to the IPK if any of the following are true:
    **
    **   1) There is an INTEGER PRIMARY KEY column and the FK is implicitly 
    **      mapped to the primary key of table pParent, or
    **   2) The FK is explicitly mapped to a column declared as INTEGER
    **      PRIMARY KEY.
    */
    إذا( p_پلار->iPKey>=0 ){
      إذا( !z_کلی ) أرجع 0;
      إذا( !sqlite3StrICmp(p_پلار->a_ستون[p_پلار->iPKey].zCnName, z_کلی) ){
        أرجع 0;
      }
    }
  }وإلا إذا( pai_ستون ){
    أكد( n_ستون>1 );
    ai_ستون = (صحيح *)sqlite3DbMallocRawNN(pParse->db, n_ستون*حجم(صحيح));
    إذا( !ai_ستون ) أرجع 1;
    *pai_ستون = ai_ستون;
  }

  لكل(pIdx=p_پلار->p_شاخص; pIdx; pIdx=pIdx->p_بل){
    إذا( pIdx->n_کلی_ستون==n_ستون && Is_Unique_شاخص(pIdx) && pIdx->pPartIdxWhere==0 ){ 
      /* pIdx is a UNIQUE index (or a PRIMARY KEY) and has the right number
      ** of columns. If each indexed column corresponds to a foreign key
      ** column of pFKey, then this index is a winner.  */

      إذا( z_کلی==0 ){
        /* If zKey is NULL, then this foreign key is implicitly mapped to 
        ** the PRIMARY KEY of table pParent. The PRIMARY KEY index may be 
        ** identified by the test.  */
        إذا( Is_Primary_کلی_شاخص(pIdx) ){
          إذا( ai_ستون ){
            صحيح i;
            لكل(i=0; i<n_ستون; i++) ai_ستون[i] = pFKey->a_ستون[i].iFrom;
          }
          اكسر;
        }
      }وإلا{
        /* If zKey is non-NULL, then this foreign key was declared to
        ** map to an explicit list of columns in table pParent. Check if this
        ** index matches those columns. Also, check that the index uses
        ** the default collation sequences for each column. */
        صحيح i, j;
        لكل(i=0; i<n_ستون; i++){
          i16 i_ستون = pIdx->ai_ستنه[i];     /* Index of column in parent tbl */
          ثابت حرف *zDfltColl;            /* Def. collation for column */
          حرف *z_Idx_ستون;                    /* Name of indexed column */

          إذا( i_ستون<0 ) اكسر; /* No foreign keys against expression indexes */

          /* If the index uses a collation sequence that is different from
          ** the default collation sequence for the column, this index is
          ** unusable. Bail out early in this case.  */
          zDfltColl = sqlite3_ستنه_Coll(&p_پلار->a_ستون[i_ستون]);
          إذا( !zDfltColl ) zDfltColl = sqlite3_Str_دوګونی;
          إذا( sqlite3StrICmp(pIdx->azColl[i], zDfltColl) ) اكسر;

          z_Idx_ستون = p_پلار->a_ستون[i_ستون].zCnName;
          لكل(j=0; j<n_ستون; j++){
            إذا( sqlite3StrICmp(pFKey->a_ستون[j].z_ستون, z_Idx_ستون)==0 ){
              إذا( ai_ستون ) ai_ستون[i] = pFKey->a_ستون[j].iFrom;
              اكسر;
            }
          }
          إذا( j==n_ستون ) اكسر;
        }
        إذا( i==n_ستون ) اكسر;      /* pIdx is usable */
      }
    }
  }

  إذا( !pIdx ){
    إذا( !pParse->disableTriggers ){
      sqlite3_Error_پیغام(pParse,
           "foreign key mismatch - \"%w\" referencing \"%w\"",
           pFKey->pFrom->zName, pFKey->z_منزل);
    }
    sqlite3DbFree(pParse->db, ai_ستون);
    أرجع 1;
  }

  *ppIdx = pIdx;
  أرجع 0;
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
ساكن فراغ fk_Lookup_پلار(
  Parse *pParse,        /* Parse context */
  صحيح iDb,              /* Index of database housing pTab */
  جدول *pTab,          /* Parent table of FK pFKey */
  شاخص *pIdx,          /* Unique index on parent key columns in pTab */
  FKey *pFKey,          /* Foreign key constraint */
  صحيح *ai_ستون,           /* Map from parent key columns to child table columns */
  صحيح reg_معلومات,          /* Address of array containing child table row */
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
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_FkIfZero, pFKey->isDeferred, iOk);
    VdbeCoverage(v);
  }
  لكل(i=0; i<pFKey->n_ستون; i++){
    صحيح iReg = sqlite3_جدول_ستنه_منزل_Storage(pFKey->pFrom,ai_ستون[i]) + reg_معلومات + 1;
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IsNull, iReg, iOk); VdbeCoverage(v);
  }

  إذا( isIgnore==0 ){
    إذا( pIdx==0 ){
      /* If pIdx is NULL, then the parent key is the INTEGER PRIMARY KEY
      ** column of the parent table (table pTab).  */
      صحيح iMustBeInt;               /* Address of MustBeInt instruction */
      صحيح reg_لنډمهاله = sqlite3_Get_لنډمهاله_Reg(pParse);
  
      /* Invoke MustBeInt to coerce the child key value to an integer (i.e. 
      ** apply the affinity of the parent key). If this fails, then there
      ** is no matching parent key. Before using MustBeInt, make a copy of
      ** the value. Otherwise, the value inserted into the child key column
      ** will have INTEGER affinity applied to it, which may not be correct.  */
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_SCopy, 
        sqlite3_جدول_ستنه_منزل_Storage(pFKey->pFrom,ai_ستون[0])+1+reg_معلومات, reg_لنډمهاله);
      iMustBeInt = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_MustBeInt, reg_لنډمهاله, 0);
      VdbeCoverage(v);
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter.  */
      إذا( pTab==pFKey->pFrom && nIncr==1 ){
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Eq, reg_معلومات, iOk, reg_لنډمهاله); VdbeCoverage(v);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      }
  
      sqlite3_Open_جدول(pParse, iCur, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_NotExists, iCur, 0, reg_لنډمهاله); VdbeCoverage(v);
      sqlite3VdbeGoto(v, iOk);
      sqlite3_Vdbe_Jump_دلته(v, sqlite3VdbeCurrentAddr(v)-2);
      sqlite3_Vdbe_Jump_دلته(v, iMustBeInt);
      sqlite3_Release_لنډمهاله_Reg(pParse, reg_لنډمهاله);
    }وإلا{
      صحيح n_ستون = pFKey->n_ستون;
      صحيح reg_لنډمهاله = sqlite3_Get_لنډمهاله_Range(pParse, n_ستون);
  
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_OpenRead, iCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_سېټ_P4_کلی_Info(pParse, pIdx);
      لكل(i=0; i<n_ستون; i++){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Copy, 
               sqlite3_جدول_ستنه_منزل_Storage(pFKey->pFrom, ai_ستون[i])+1+reg_معلومات,
               reg_لنډمهاله+i);
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
        صحيح iJump = sqlite3VdbeCurrentAddr(v) + n_ستون + 1;
        لكل(i=0; i<n_ستون; i++){
          صحيح i_ماشوم = sqlite3_جدول_ستنه_منزل_Storage(pFKey->pFrom,ai_ستون[i])
                              +1+reg_معلومات;
          صحيح i_پلار = 1+reg_معلومات;
          i_پلار += sqlite3_جدول_ستنه_منزل_Storage(pIdx->p_جدول,
                                                 pIdx->ai_ستنه[i]);
          أكد( pIdx->ai_ستنه[i]>=0 );
          أكد( ai_ستون[i]!=pTab->iPKey );
          إذا( pIdx->ai_ستنه[i]==pTab->iPKey ){
            /* The parent key is a composite key that includes the IPK column */
            i_پلار = reg_معلومات;
          }
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Ne, i_ماشوم, iJump, i_پلار); VdbeCoverage(v);
          sqlite3VdbeChangeP5(v, SQLITE_JUMPIFNULL);
        }
        sqlite3VdbeGoto(v, iOk);
      }

      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Affinity, reg_لنډمهاله, n_ستون, 0,
                        sqlite3_شاخص_Affinity_Str(pParse->db,pIdx), n_ستون);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_وموندل, iCur, iOk, reg_لنډمهاله, n_ستون);
      VdbeCoverage(v);
      sqlite3_Release_لنډمهاله_Range(pParse, reg_لنډمهاله, n_ستون);
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
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
  }

  sqlite3VdbeResolveLabel(v, iOk);
  sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Close, iCur);
}


/*
** Return an Expr object that refers to a memory register corresponding
** to column iCol of table pTab.
**
** regBase is the first of an array of register that contains the data
** for pTab.  regBase itself holds the rowid.  regBase+1 holds the first
** column.  regBase+2 holds the second column, and so forth.
*/
ساكن بیان *بیان_جدول_ثبتوونکی(
  Parse *pParse,     /* Parsing and code generating context */
  جدول *pTab,       /* The table whose content is at r[regBase]... */
  صحيح reg_بنسټ,       /* Contents of table pTab */
  i16 i_ستون           /* Which column of pTab is desired */
){
  بیان *p_بیان;
  ستنه *p_ستون;
  ثابت حرف *zColl;
  sqlite3 *db = pParse->db;

  p_بیان = sqlite3_بیان(db, TK_ثبتوونکی, 0);
  إذا( p_بیان ){
    إذا( i_ستون>=0 && i_ستون!=pTab->iPKey ){
      p_ستون = &pTab->a_ستون[i_ستون];
      p_بیان->i_جدول = reg_بنسټ + sqlite3_جدول_ستنه_منزل_Storage(pTab,i_ستون) + 1;
      p_بیان->aff_بیان = p_ستون->affinity;
      zColl = sqlite3_ستنه_Coll(p_ستون);
      إذا( zColl==0 ) zColl = db->pDfltColl->zName;
      p_بیان = sqlite3_بیان_جمع_کړه_Collate_String(pParse, p_بیان, zColl);
    }وإلا{
      p_بیان->i_جدول = reg_بنسټ;
      p_بیان->aff_بیان = SQLITE_AFF_INTEGER;
    }
  }
  أرجع p_بیان;
}

/*
** Return an Expr object that refers to column iCol of table pTab which
** has cursor iCur.
*/
ساكن بیان *بیان_جدول_ستنه(
  sqlite3 *db,      /* The database connection */
  جدول *pTab,      /* The table whose column is desired */
  صحيح iCursor,      /* The open cursor on the table */
  i16 i_ستون          /* The column that is wanted */
){
  بیان *p_بیان = sqlite3_بیان(db, TK_ستنه, 0);
  إذا( p_بیان ){
    أكد( بیان_Use_YTab(p_بیان) );
    p_بیان->y.pTab = pTab;
    p_بیان->i_جدول = iCursor;
    p_بیان->i_ستنه = i_ستون;
  }
  أرجع p_بیان;
}

/*
** Column iCol1 of table p1 is to be compared to column iCol2 of table p2.
** Return non-zero if the comparison will use an affinity compatible
** with the affinity of column iCol1 of table p1. Or zero if it will not.
**
** This is used in cases where (p1.iCol1 = p2.iCol2) must be evaluated
** using the affinity of p1.iCol1 as part of foreign key processing.
*/
ساكن صحيح fk_کاروي_Lhs_Affinity(جدول *p1, صحيح iCol1, جدول *p2, صحيح iCol2){
  u8 aff1 = iCol1>=0 ? p1->a_ستون[iCol1].affinity : SQLITE_AFF_NUMERIC;
  u8 aff2 = p2->a_ستون[iCol2].affinity;

  /* If either side has a numeric affinity, then the comparison uses numeric
  ** affinity. So the affinity matches if aff1 is numeric, but not if aff1
  ** is not numeric but aff2 is.  */
  إذا( sqlite3IsNumericAffinity(aff1) ) أرجع 1;
  إذا( sqlite3IsNumericAffinity(aff2) ) أرجع 0;

  أكد( aff1==SQLITE_AFF_متن || aff1==SQLITE_AFF_BLOB );
  أكد( aff2==SQLITE_AFF_متن || aff2==SQLITE_AFF_BLOB );

  /* At this point, either the affinities are the same or one is TEXT and
  ** the other BLOB. In the second case the comparison will be done with
  ** BLOB affinity, which does not match if aff1 is TEXT.  */
  أرجع (aff1==aff2 || aff1==SQLITE_AFF_BLOB);
}

#که_نه_تعریف_شوی SQLITE_OMIT_CAST
/*
** Return non-zero if a lookup on column iCol of table pTab, using collation
** sequence zColl, can be done using either the rowid or an index. That is,
** if iCol is the INTEGER PRIMARY KEY of pTab, or if pTab has an index
** whose left-most column is iCol and whose collation sequence for that
** column is zColl. Return zero otherwise.
*/
ساكن صحيح fk_ماشوم_کلی_Is_شاخصي(جدول *pTab, صحيح i_ستون, ثابت حرف *zColl){
  شاخص *pIdx;
  إذا( i_ستون==pTab->iPKey ) أرجع 1;
  لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بل){
    إذا( pIdx->ai_ستنه[0]==i_ستون
     && pIdx->pPartIdxWhere==0
     && sqlite3StrICmp(pIdx->azColl[0], zColl)==0
    ){
      أرجع 1;
    }
  }
  أرجع 0;
}
#د_که_پای

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
ساكن بیان *fk_پلار_ماشوم_Eq(
  Parse *pParse,            /* Parsing context */
  بیان *p_کیڼ,              /* Parent key value */
  بیان *p_ښی,             /* Unresolved reference to child key column */
  جدول *pTab,              /* Parent table */
  صحيح i_ستون,                 /* Parent key column, or <0 for rowid */
  جدول *p_ماشوم,            /* Child table */
  صحيح i_ماشوم_ستون,            /* Child key column */
  صحيح b_مرکب            /* This is part of a composite FK */
){
  sqlite3 *db = pParse->db;
  بیان *pEq;
#که_نه_تعریف_شوی SQLITE_OMIT_CAST
  بیان *p_دننه = 0;
#د_که_پای

  إذا( fk_کاروي_Lhs_Affinity(pTab, i_ستون, p_ماشوم, i_ماشوم_ستون) ){
    أرجع sqlite3PExpr(pParse, TK_EQ, p_کیڼ, p_ښی);
  }
  أكد( i_ستون>=0 );

#که_نه_تعریف_شوی SQLITE_OMIT_CAST
  {
    ثابت حرف *zColl = sqlite3_ستنه_Coll(&pTab->a_ستون[i_ستون]);
    إذا( zColl==0 ) zColl = sqlite3_Str_دوګونی;

    /* The IN term is only useful if it can drive a lookup on the child
    ** table. Omit it otherwise, so that a full scan of the child table
    ** does not have to evaluate it for every row.  */
    إذا( b_مرکب || fk_ماشوم_کلی_Is_شاخصي(p_ماشوم, i_ماشوم_ستون, zColl) ){
      ساكن ثابت Token tNumeric = { "NUMERIC", 7 };
      بیان *pCast;
      بیان_List *pList;

      pCast = sqlite3_بیان_Alloc(db, TK_CAST, &tNumeric, 0);
      sqlite3_بیان_Attach_Subtrees(db, pCast, sqlite3_بیان_Dup(db, p_کیڼ, 0), 0);
      pList = sqlite3_بیان_List_Append(pParse, 0,
          sqlite3PExpr(pParse, TK_UPLUS, sqlite3_بیان_Dup(db, p_کیڼ, 0), 0)
      );
      pList = sqlite3_بیان_List_Append(pParse, pList,
          sqlite3PExpr(pParse, TK_UPLUS, pCast, 0)
      );
      p_دننه = sqlite3PExpr(pParse, TK_دننه, sqlite3_بیان_جمع_کړه_Collate_String(pParse,
            sqlite3_بیان_Dup(db, p_ښی, 0), zColl
      ), 0);
      إذا( p_دننه ){
        p_دننه->x.pList = pList;
        sqlite3_بیان_سېټ_لوړوالی_And_Flags(pParse, p_دننه);
      }وإلا{
        sqlite3_بیان_List_Delete(db, pList);
      }
    }
  }
#د_که_پای

  p_ښی = sqlite3PExpr(pParse, TK_UPLUS, p_ښی, 0);
  pEq = sqlite3PExpr(pParse, TK_EQ, p_کیڼ, p_ښی);
#که_نه_تعریف_شوی SQLITE_OMIT_CAST
  pEq = sqlite3_بیان_And(pParse, pEq, p_دننه);
#د_که_پای
  أرجع pEq;
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
ساكن فراغ fk_لټون_Children(
  Parse *pParse,                  /* Parse context */
  SrcList *pSrc,                  /* The child table to be scanned */
  جدول *pTab,                    /* The parent table */
  شاخص *pIdx,                    /* Index on parent covering the foreign key */
  FKey *pFKey,                    /* The foreign key linking pSrc to pTab */
  صحيح *ai_ستون,                     /* Map from pIdx cols to child table cols */
  صحيح reg_معلومات,                    /* Parent row data starts here */
  صحيح nIncr                       /* Amount to increment deferred counter by */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  صحيح i;                          /* Iterator variable */
  بیان *pWhere = 0;               /* WHERE clause to scan with */
  NameContext sNameContext;       /* Context used to resolve WHERE clause */
  WhereInfo *pWInfo;              /* Context used by sqlite3WhereXXX() */
  صحيح iFkIfZero = 0;              /* Address of OP_FkIfZero */
  Vdbe *v = sqlite3GetVdbe(pParse);

  أكد( pIdx==0 || pIdx->p_جدول==pTab );
  أكد( pIdx==0 || pIdx->n_کلی_ستون==pFKey->n_ستون );
  أكد( pIdx!=0 || pFKey->n_ستون==1 );
  أكد( pIdx!=0 || HasRowid(pTab) );

  إذا( nIncr<0 ){
    iFkIfZero = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_FkIfZero, pFKey->isDeferred, 0);
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
  لكل(i=0; i<pFKey->n_ستون; i++){
    بیان *p_کیڼ;                  /* Value from parent table row */
    بیان *p_ښی;                 /* Column ref to child table */
    بیان *pEq;                    /* Expression (pLeft = pRight) */
    i16 i_ستون;                     /* Index of column in parent table */
    i16 iCol2;                    /* Index of column in child table */
    ثابت حرف *z_ستون;             /* Name of column in child table */

    i_ستون = pIdx ? pIdx->ai_ستنه[i] : -1;
    p_کیڼ = بیان_جدول_ثبتوونکی(pParse, pTab, reg_معلومات, i_ستون);
    iCol2 = ai_ستون ? ai_ستون[i] : pFKey->a_ستون[0].iFrom;
    أكد( iCol2>=0 );
    z_ستون = pFKey->pFrom->a_ستون[iCol2].zCnName;
    p_ښی = sqlite3_بیان(db, TK_ID, z_ستون);
    pEq = fk_پلار_ماشوم_Eq(pParse, p_کیڼ, p_ښی, pTab, i_ستون,
                          pFKey->pFrom, iCol2, pFKey->n_ستون>0);
    pWhere = sqlite3_بیان_And(pParse, pWhere, pEq);
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
    بیان *pNe;                    /* Expression (pLeft != pRight) */
    بیان *p_کیڼ;                  /* Value from parent table row */
    بیان *p_ښی;                 /* Column ref to child table */
    إذا( HasRowid(pTab) ){
      p_کیڼ = بیان_جدول_ثبتوونکی(pParse, pTab, reg_معلومات, -1);
      p_ښی = بیان_جدول_ستنه(db, pTab, pSrc->a[0].iCursor, -1);
      pNe = sqlite3PExpr(pParse, TK_NE, p_کیڼ, p_ښی);
    }وإلا{
      بیان *pEq, *pAll = 0;
      أكد( pIdx!=0 );
      لكل(i=0; i<pIdx->n_کلی_ستون; i++){
        i16 i_ستون = pIdx->ai_ستنه[i];
        أكد( i_ستون>=0 );
        p_کیڼ = بیان_جدول_ثبتوونکی(pParse, pTab, reg_معلومات, i_ستون);
        p_ښی = sqlite3_بیان(db, TK_ID, pTab->a_ستون[i_ستون].zCnName);
        pEq = sqlite3PExpr(pParse, TK_IS, p_کیڼ, p_ښی);
        pAll = sqlite3_بیان_And(pParse, pAll, pEq);
      }
      pNe = sqlite3PExpr(pParse, TK_NOT, pAll, 0);
    }
    pWhere = sqlite3_بیان_And(pParse, pWhere, pNe);
  }

  /* Resolve the references in the WHERE clause. */
  memset(&sNameContext, 0, حجم(NameContext));
  sNameContext.pSrcList = pSrc;
  sNameContext.pParse = pParse;
  sqlite3_Resolve_بیان_نومونه(&sNameContext, pWhere);

  /* Create VDBE to loop through the entries in pSrc that match the WHERE
  ** clause. For each row found, increment either the deferred or immediate
  ** foreign key constraint counter. */
  إذا( pParse->nErr==0 ){
    pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0, 0);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
    إذا( pWInfo ){
      sqlite3_Where_پای_ټکی(pWInfo);
    }
  }

  /* Clean up the WHERE clause constructed above. */
  sqlite3_بیان_Delete(db, pWhere);
  إذا( iFkIfZero ){
    sqlite3_Vdbe_Jump_دلته_Or_راوباسه_Inst(v, iFkIfZero);
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
FKey *sqlite3FkReferences(جدول *pTab){
  أرجع (FKey *)sqlite3_Hash_پیدا_کړه(&pTab->pSchema->fkeyHash, pTab->zName);
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
    Trigger_ګام *p_ګام = p->ګام_list;
    sqlite3SrcListDelete(dbMem, p_ګام->pSrc);
    sqlite3_بیان_Delete(dbMem, p_ګام->pWhere);
    sqlite3_بیان_List_Delete(dbMem, p_ګام->p_بیان_List);
    sqlite3SelectDelete(dbMem, p_ګام->pSelect);
    sqlite3_بیان_Delete(dbMem, p->pWhen);
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
  لكل(k=sqlite_Hash_لومړی(pHash); k; k=sqlite_Hash_بل(k)){
    جدول *pTab = sqlite_Hash_معلومات(k);
    FKey *pFKey;
    إذا( !Is_Ordinary_جدول(pTab) ) استمر;
    لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_بل_From){
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
فراغ sqlite3_Fk_Drop_جدول(Parse *pParse, SrcList *pName, جدول *pTab){
  sqlite3 *db = pParse->db;
  إذا( (db->flags&SQLITE_Foreign_کلیګانې) && Is_Ordinary_جدول(pTab) ){
    صحيح iSkip = 0;
    Vdbe *v = sqlite3GetVdbe(pParse);

    أكد( v );                  /* VDBE has already been allocated */
    أكد( Is_Ordinary_جدول(pTab) );
    إذا( sqlite3FkReferences(pTab)==0 ){
      /* Search for a deferred foreign key constraint for which this table
      ** is the child table. If one cannot be found, return without 
      ** generating any VDBE code. If one can be found, then jump over
      ** the entire DELETE if there are no outstanding deferred constraints
      ** when this statement is run.  */
      FKey *p;
      لكل(p=pTab->u.tab.pFKey; p; p=p->p_بل_From){
        إذا( p->isDeferred || (db->flags & SQLITE_DeferFKs) ) اكسر;
      }
      إذا( !p ) أرجع;
      iSkip = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_FkIfZero, 1, iSkip); VdbeCoverage(v);
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
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_FkIfZero, 0, sqlite3VdbeCurrentAddr(v)+2);
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
ساكن صحيح fk_ماشوم_Is_Modified(
  جدول *pTab,                    /* Table being updated */
  FKey *p,                        /* Foreign key for which pTab is the child */
  صحيح *aChange,                   /* Array indicating modified columns */
  صحيح bChngRowid                  /* True if rowid is modified by this update */
){
  صحيح i;
  لكل(i=0; i<p->n_ستون; i++){
    صحيح i_ماشوم_کلی = p->a_ستون[i].iFrom;
    إذا( aChange[i_ماشوم_کلی]>=0 ) أرجع 1;
    إذا( i_ماشوم_کلی==pTab->iPKey && bChngRowid ) أرجع 1;
  }
  أرجع 0;
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
ساكن صحيح fk_پلار_Is_Modified(
  جدول *pTab, 
  FKey *p, 
  صحيح *aChange, 
  صحيح bChngRowid
){
  صحيح i;
  لكل(i=0; i<p->n_ستون; i++){
    حرف *z_کلی = p->a_ستون[i].z_ستون;
    صحيح i_کلی;
    لكل(i_کلی=0; i_کلی<pTab->n_ستون; i_کلی++){
      إذا( aChange[i_کلی]>=0 || (i_کلی==pTab->iPKey && bChngRowid) ){
        ستنه *p_ستون = &pTab->a_ستون[i_کلی];
        إذا( z_کلی ){
          إذا( 0==sqlite3StrICmp(p_ستون->zCnName, z_کلی) ) أرجع 1;
        }وإلا إذا( p_ستون->ستون_Flags & COLFLAG_PRIMKEY ){
          أرجع 1;
        }
      }
    }
  }
  أرجع 0;
}

/*
** Return true if the parser passed as the first argument is being
** used to code a trigger that is really a "SET NULL" action belonging
** to trigger pFKey.
*/
ساكن صحيح is_سېټ_Null_Action(Parse *pParse, FKey *pFKey){
  Trigger *p = pParse->pTrigCoded;
  إذا( p ){
    إذا( (p==pFKey->apTrigger[0] && pFKey->aAction[0]==OE_سېټ_Null)
     || (p==pFKey->apTrigger[1] && pFKey->aAction[1]==OE_سېټ_Null)
    ){
      أكد( (pParse->db->flags & SQLITE_FkNoAction)==0 );
      أرجع 1;
    }
  }
  أرجع 0;
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
  جدول *pTab,                    /* Row is being deleted from this table */ 
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
  إذا( (db->flags&SQLITE_Foreign_کلیګانې)==0 ) أرجع;
  إذا( !Is_Ordinary_جدول(pTab) ) أرجع;

  iDb = sqlite3_Schema_منزل_شاخص(db, pTab->pSchema);
  أكد( iDb>=00 && iDb<db->nDb );
  zDb = db->aDb[iDb].zDbSName;

  /* Loop through all the foreign key constraints for which pTab is the
  ** child table (the table that the foreign key definition is part of).  */
  لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_بل_From){
    جدول *p_منزل;                   /* Parent table of foreign key pFKey */
    شاخص *pIdx = 0;              /* Index on key columns in pTo */
    صحيح *aiFree = 0;
    صحيح *ai_ستون;
    صحيح i_ستون;
    صحيح i;
    صحيح bIgnore = 0;

    إذا( aChange 
     && sqlite3_stricmp(pTab->zName, pFKey->z_منزل)!=0
     && fk_ماشوم_Is_Modified(pTab, pFKey, aChange, bChngRowid)==0 
    ){
      استمر;
    }

    /* Find the parent table of this foreign key. Also find a unique index 
    ** on the parent key columns in the parent table. If either of these 
    ** schema items cannot be located, set an error in pParse and return 
    ** early.  */
    إذا( pParse->disableTriggers ){
      p_منزل = sqlite3_پیدا_کړه_جدول(db, pFKey->z_منزل, zDb);
    }وإلا{
      p_منزل = sqlite3_Locate_جدول(pParse, 0, pFKey->z_منزل, zDb);
    }
    إذا( !p_منزل || sqlite3_Fk_Locate_شاخص(pParse, p_منزل, pFKey, &pIdx, &aiFree) ){
      أكد( isIgnoreErrors==0 || (regOld!=0 && regNew==0) );
      إذا( !isIgnoreErrors || db->mallocFailed ) أرجع;
      إذا( p_منزل==0 ){
        /* If isIgnoreErrors is true, then a table is being dropped. In this
        ** case SQLite runs a "DELETE FROM xxx" on the table being dropped
        ** before actually dropping it in order to check FK constraints.
        ** If the parent table of an FK constraint on the current table is
        ** missing, behave as if it is empty. i.e. decrement the relevant
        ** FK counter for each row of the current table with non-NULL keys.
        */
        Vdbe *v = sqlite3GetVdbe(pParse);
        صحيح iJump = sqlite3VdbeCurrentAddr(v) + pFKey->n_ستون + 1;
        لكل(i=0; i<pFKey->n_ستون; i++){
          صحيح i_From_ستون, iReg;
          i_From_ستون = pFKey->a_ستون[i].iFrom;
          iReg = sqlite3_جدول_ستنه_منزل_Storage(pFKey->pFrom,i_From_ستون) + regOld+1;
          sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IsNull, iReg, iJump); VdbeCoverage(v);
        }
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_FkCounter, pFKey->isDeferred, -1);
      }
      استمر;
    }
    أكد( pFKey->n_ستون==1 || (aiFree && pIdx) );

    إذا( aiFree ){
      ai_ستون = aiFree;
    }وإلا{
      i_ستون = pFKey->a_ستون[0].iFrom;
      ai_ستون = &i_ستون;
    }
    لكل(i=0; i<pFKey->n_ستون; i++){
      إذا( ai_ستون[i]==pTab->iPKey ){
        ai_ستون[i] = -1;
      }
      أكد( pIdx==0 || pIdx->ai_ستنه[i]>=0 );
#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
      /* Request permission to read the parent key columns. If the 
      ** authorization callback returns SQLITE_IGNORE, behave as if any
      ** values read from the parent table are NULL. */
      إذا( db->xAuth ){
        صحيح rcauth;
        حرف *z_ستون = p_منزل->a_ستون[pIdx ? pIdx->ai_ستنه[i] : p_منزل->iPKey].zCnName;
        rcauth = sqlite3_Auth_Read_ستون(pParse, p_منزل->zName, z_ستون, iDb);
        bIgnore = (rcauth==SQLITE_IGNORE);
      }
#د_که_پای
    }

    /* Take a shared-cache advisory read-lock on the parent table. Allocate 
    ** a cursor to use to search the unique index on the parent key columns 
    ** in the parent table.  */
    sqlite3_جدول_قلف(pParse, iDb, p_منزل->tnum, 0, p_منزل->zName);
    pParse->nTab++;

    إذا( regOld!=0 ){
      /* A row is being removed from the child table. Search for the parent.
      ** If the parent does not exist, removing the child row resolves an 
      ** outstanding foreign key constraint violation. */
      fk_Lookup_پلار(pParse, iDb, p_منزل, pIdx, pFKey, ai_ستون, regOld, -1, bIgnore);
    }
    إذا( regNew!=0 && !is_سېټ_Null_Action(pParse, pFKey) ){
      /* A row is being added to the child table. If a parent row cannot
      ** be found, adding the child row has violated the FK constraint. 
      **
      ** If this operation is being performed as part of a trigger program
      ** that is actually a "SET NULL" action belonging to this very 
      ** foreign key, then omit this scan altogether. As all child key
      ** values are guaranteed to be NULL, it is not possible for adding
      ** this row to cause an FK violation.  */
      fk_Lookup_پلار(pParse, iDb, p_منزل, pIdx, pFKey, ai_ستون, regNew, +1, bIgnore);
    }

    sqlite3DbFree(db, aiFree);
  }

  /* Loop through all the foreign key constraints that refer to this table.
  ** (the "child" constraints) */
  لكل(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_بل_منزل){
    شاخص *pIdx = 0;              /* Foreign key index for pFKey */
    SrcList *pSrc;
    صحيح *ai_ستون = 0;

    إذا( aChange && fk_پلار_Is_Modified(pTab, pFKey, aChange, bChngRowid)==0 ){
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

    إذا( sqlite3_Fk_Locate_شاخص(pParse, pTab, pFKey, &pIdx, &ai_ستون) ){
      إذا( !isIgnoreErrors || db->mallocFailed ) أرجع;
      استمر;
    }
    أكد( ai_ستون || pFKey->n_ستون==1 );

    /* Create a SrcList structure containing the child table.  We need the
    ** child table as a SrcList for sqlite3WhereBegin() */
    pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
    إذا( pSrc ){
      Src_توکی *p_توکی = pSrc->a;
      p_توکی->pSTab = pFKey->pFrom;
      p_توکی->zName = pFKey->pFrom->zName;
      p_توکی->pSTab->nTabRef++;
      p_توکی->iCursor = pParse->nTab++;
  
      إذا( regNew!=0 ){
        fk_لټون_Children(pParse, pSrc, pTab, pIdx, pFKey, ai_ستون, regNew, -1);
      }
      إذا( regOld!=0 ){
        صحيح eAction = pFKey->aAction[aChange!=0];
        إذا( (db->flags & SQLITE_FkNoAction) ) eAction = OE_None;

        fk_لټون_Children(pParse, pSrc, pTab, pIdx, pFKey, ai_ستون, regOld, 1);
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
        إذا( !pFKey->isDeferred && eAction!=OE_Cascade && eAction!=OE_سېټ_Null ){
          sqlite3MayAbort(pParse);
        }
      }
      p_توکی->zName = 0;
      sqlite3SrcListDelete(db, pSrc);
    }
    sqlite3DbFree(db, ai_ستون);
  }
}

#تعریف_کړه ستنه_ماسک(x) (((x)>31) ? 0xffffffff : ((u32)1<<(x)))

/*
** This function is called before generating code to update or delete a 
** row contained in table pTab.
*/
u32 sqlite3FkOldmask(
  Parse *pParse,                  /* Parse context */
  جدول *pTab                     /* Table being modified */
){
  u32 ماسک = 0;
  إذا( pParse->db->flags&SQLITE_Foreign_کلیګانې && Is_Ordinary_جدول(pTab) ){
    FKey *p;
    صحيح i;
    لكل(p=pTab->u.tab.pFKey; p; p=p->p_بل_From){
      لكل(i=0; i<p->n_ستون; i++) ماسک |= ستنه_ماسک(p->a_ستون[i].iFrom);
    }
    لكل(p=sqlite3FkReferences(pTab); p; p=p->p_بل_منزل){
      شاخص *pIdx = 0;
      sqlite3_Fk_Locate_شاخص(pParse, pTab, p, &pIdx, 0);
      إذا( pIdx ){
        لكل(i=0; i<pIdx->n_کلی_ستون; i++){
          أكد( pIdx->ai_ستنه[i]>=0 );
          ماسک |= ستنه_ماسک(pIdx->ai_ستنه[i]);
        }
      }
    }
  }
  أرجع ماسک;
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
  جدول *pTab,                    /* Table being modified */
  صحيح *aChange,                   /* Non-NULL for UPDATE operations */
  صحيح chngRowid                   /* True for UPDATE that affects rowid */
){
  صحيح eRet = 1;                   /* Value to return if bHaveFK is true */
  صحيح bHaveFK = 0;                /* If FK processing is required */
  إذا( pParse->db->flags&SQLITE_Foreign_کلیګانې && Is_Ordinary_جدول(pTab) ){
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
      لكل(p=pTab->u.tab.pFKey; p; p=p->p_بل_From){
        إذا( fk_ماشوم_Is_Modified(pTab, p, aChange, chngRowid) ){
          إذا( 0==sqlite3_stricmp(pTab->zName, p->z_منزل) ) eRet = 2;
          bHaveFK = 1;
        }
      }

      /* Check if any parent key columns are being modified. */
      لكل(p=sqlite3FkReferences(pTab); p; p=p->p_بل_منزل){
        إذا( fk_پلار_Is_Modified(pTab, p, aChange, chngRowid) ){
          إذا( (pParse->db->flags & SQLITE_FkNoAction)==0 
           && p->aAction[1]!=OE_None 
          ){
            أرجع 2;
          }
          bHaveFK = 1;
        }
      }
    }
  }
  أرجع bHaveFK ? eRet : 0;
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
  جدول *pTab,                    /* Table being updated or deleted from */
  FKey *pFKey,                    /* Foreign key to get action for */
  بیان_List *pChanges              /* Change-list for UPDATE, NULL for DELETE */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  صحيح action;                     /* One of OE_None, OE_Cascade etc. */
  Trigger *pTrigger;              /* Trigger definition to return */
  صحيح iAction = (pChanges!=0);    /* 1 for UPDATE, 0 for DELETE */

  action = pFKey->aAction[iAction];
  إذا( (db->flags & SQLITE_FkNoAction) ) action = OE_None;
  إذا( action==OE_Restrict && (db->flags & SQLITE_DeferFKs) ){
    أرجع 0;
  }
  pTrigger = pFKey->apTrigger[iAction];

  إذا( action!=OE_None && !pTrigger ){
    حرف ثابت *zFrom;            /* Name of child table */
    صحيح nFrom;                    /* Length in bytes of zFrom */
    شاخص *pIdx = 0;              /* Parent key index for this FK */
    صحيح *ai_ستون = 0;               /* child table cols -> parent key cols */
    Trigger_ګام *p_ګام = 0;        /* First (only) step of trigger program */
    بیان *pWhere = 0;             /* WHERE clause of trigger step */
    بیان_List *pList = 0;          /* Changes list if ON UPDATE CASCADE */
    Select *pSelect = 0;          /* If RESTRICT, "SELECT RAISE(...)" */
    صحيح i;                        /* Iterator variable */
    بیان *pWhen = 0;              /* WHEN clause for the trigger */

    إذا( sqlite3_Fk_Locate_شاخص(pParse, pTab, pFKey, &pIdx, &ai_ستون) ) أرجع 0;
    أكد( ai_ستون || pFKey->n_ستون==1 );

    لكل(i=0; i<pFKey->n_ستون; i++){
      Token tOld = { "old", 3 };  /* Literal "old" token */
      Token tNew = { "new", 3 };  /* Literal "new" token */
      Token t_From_ستون;             /* Name of column in child table */
      Token t_منزل_ستون;               /* Name of column in parent table */
      صحيح i_From_ستون;               /* Idx of column in child table */
      صحيح i_منزل_ستون;                 /* Idx of column in parent table */
      بیان *pEq;                  /* OLD.tToCol = tFromCol */
      بیان *p_کیڼ;                /* OLD.tToCol */
      بیان *p_ښی;               /* tFromCol */

      i_From_ستون = ai_ستون ? ai_ستون[i] : pFKey->a_ستون[0].iFrom;
      أكد( i_From_ستون>=0 );
      أكد( pIdx!=0 || (pTab->iPKey>=0 && pTab->iPKey<pTab->n_ستون) );
      أكد( pIdx==0 || pIdx->ai_ستنه[i]>=0 );
      i_منزل_ستون = pIdx ? pIdx->ai_ستنه[i] : pTab->iPKey;
      sqlite3TokenInit(&t_منزل_ستون, pTab->a_ستون[i_منزل_ستون].zCnName);
      sqlite3TokenInit(&t_From_ستون, pFKey->pFrom->a_ستون[i_From_ستون].zCnName);

      p_کیڼ = sqlite3PExpr(pParse, TK_DOT,
          sqlite3_بیان_Alloc(db, TK_ID, &tOld, 0),
          sqlite3_بیان_Alloc(db, TK_ID, &t_منزل_ستون, 0)
      );
      p_ښی = sqlite3_بیان_Alloc(db, TK_ID, &t_From_ستون, 0);

      /* OLD.* columns do not normally have an affinity. So set the affinity
      ** of OLD.tToCol (the parent key) explicitly here. */
      إذا( p_کیڼ ) p_کیڼ->aff_بیان = pTab->a_ستون[i_منزل_ستون].affinity;

      /* Create the expression "OLD.tToCol = tFromCol". It is important
      ** that the "OLD.tToCol" term is on the LHS of the = operator, so
      ** that the collation sequence associated with the parent table is
      ** used for the comparison. */
      pEq = fk_پلار_ماشوم_Eq(pParse, p_کیڼ, p_ښی, pTab, i_منزل_ستون,
                            pFKey->pFrom, i_From_ستون, pFKey->n_ستون>1);
      pWhere = sqlite3_بیان_And(pParse, pWhere, pEq);

      /* For ON UPDATE, construct the next term of the WHEN clause.
      ** The final WHEN clause will be like this:
      **
      **    WHEN NOT(old.col1 IS new.col1 AND ... AND old.colN IS new.colN)
      */
      إذا( pChanges ){
        pEq = sqlite3PExpr(pParse, TK_IS,
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_بیان_Alloc(db, TK_ID, &tOld, 0),
              sqlite3_بیان_Alloc(db, TK_ID, &t_منزل_ستون, 0)),
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_بیان_Alloc(db, TK_ID, &tNew, 0),
              sqlite3_بیان_Alloc(db, TK_ID, &t_منزل_ستون, 0))
            );
        pWhen = sqlite3_بیان_And(pParse, pWhen, pEq);
      }
  
      إذا( action!=OE_Restrict && (action!=OE_Cascade || pChanges) ){
        بیان *pNew;
        إذا( action==OE_Cascade ){
          pNew = sqlite3PExpr(pParse, TK_DOT, 
            sqlite3_بیان_Alloc(db, TK_ID, &tNew, 0),
            sqlite3_بیان_Alloc(db, TK_ID, &t_منزل_ستون, 0));
        }وإلا إذا( action==OE_سېټ_Dflt ){
          ستنه *p_ستون = pFKey->pFrom->a_ستون + i_From_ستون;
          بیان *pDflt;
          إذا( p_ستون->ستون_Flags & COLFLAG_GENERATED ){
            testcase( p_ستون->ستون_Flags & COLFLAG_VIRTUAL );
            testcase( p_ستون->ستون_Flags & COLFLAG_STORED );
            pDflt = 0;
          }وإلا{
            pDflt = sqlite3_ستنه_بیان_Auth(pFKey->pFrom, p_ستون, pParse);
          }
          إذا( pDflt ){
            pNew = sqlite3_بیان_Dup(db, pDflt, 0);
          }وإلا{
            pNew = sqlite3_بیان_Alloc(db, TK_NULL, 0, 0);
          }
        }وإلا{
          pNew = sqlite3_بیان_Alloc(db, TK_NULL, 0, 0);
        }
        pList = sqlite3_بیان_List_Append(pParse, pList, pNew);
        sqlite3_بیان_List_سېټ_Name(pParse, pList, &t_From_ستون, 0);
      }
    }
    sqlite3DbFree(db, ai_ستون);

    zFrom = pFKey->pFrom->zName;
    nFrom = sqlite3Strlen30(zFrom);

    إذا( action==OE_Restrict ){
      SrcList *pSrc;
      بیان *pRaise; 

      pRaise = sqlite3_بیان(db, TK_STRING, "FOREIGN KEY constraint failed"),
      pRaise = sqlite3PExpr(pParse, TK_RAISE, pRaise, 0);
      إذا( pRaise ){
        pRaise->aff_بیان = OE_Abort;
      }
      pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      إذا( pSrc ){
        Src_توکی *p_توکی = &pSrc->a[0];
        p_توکی->zName = sqlite3DbStrDup(db, zFrom);
        p_توکی->fg.fixedSchema = 1;
        p_توکی->u4.pSchema = pTab->pSchema;
      }
      pSelect = sqlite3SelectNew(pParse, 
          sqlite3_بیان_List_Append(pParse, 0, pRaise),
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
        حجم(Trigger_ګام)       /* Single step in trigger program */
    );
    إذا( pTrigger ){
      p_ګام = pTrigger->ګام_list = (Trigger_ګام *)&pTrigger[1];
      p_ګام->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      إذا( p_ګام->pSrc ){
        Src_توکی *p_توکی = &p_ګام->pSrc->a[0];
        p_توکی->zName = sqlite3DbStrNDup(db, zFrom, nFrom);
        p_توکی->u4.pSchema = pTab->pSchema;
        p_توکی->fg.fixedSchema = 1;
      }
      p_ګام->pWhere = sqlite3_بیان_Dup(db, pWhere, EXPRDUP_REDUCE);
      p_ګام->p_بیان_List = sqlite3_بیان_List_Dup(db, pList, EXPRDUP_REDUCE);
      p_ګام->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
      إذا( pWhen ){
        pWhen = sqlite3PExpr(pParse, TK_NOT, pWhen, 0);
        pTrigger->pWhen = sqlite3_بیان_Dup(db, pWhen, EXPRDUP_REDUCE);
      }
    }

    /* Re-enable the lookaside buffer, if it was disabled earlier. */
    EnableLookaside;

    sqlite3_بیان_Delete(db, pWhere);
    sqlite3_بیان_Delete(db, pWhen);
    sqlite3_بیان_List_Delete(db, pList);
    sqlite3SelectDelete(db, pSelect);
    إذا( db->mallocFailed==1 ){
      fkTriggerDelete(db, pTrigger);
      أرجع 0;
    }
    أكد( p_ګام!=0 );
    أكد( pTrigger!=0 );

    بدل( action ){
      حالة OE_Restrict:
        p_ګام->op = TK_SELECT;
        اكسر;
      حالة OE_Cascade: 
        إذا( !pChanges ){ 
          p_ګام->op = TK_DELETE; 
          اكسر; 
        }
        /* no break */ deliberate_fall_through
      افتراضي:
        p_ګام->op = TK_UPDATE;
    }
    p_ګام->pTrig = pTrigger;
    pTrigger->pSchema = pTab->pSchema;
    pTrigger->pTabSchema = pTab->pSchema;
    pFKey->apTrigger[iAction] = pTrigger;
    pTrigger->op = (pChanges ? TK_UPDATE : TK_DELETE);
  }

  أرجع pTrigger;
}

/*
** This function is called when deleting or updating a row to implement
** any required CASCADE, SET NULL or SET DEFAULT actions.
*/
فراغ sqlite3FkActions(
  Parse *pParse,                  /* Parse context */
  جدول *pTab,                    /* Table being updated or deleted from */
  بیان_List *pChanges,             /* Change-list for UPDATE, NULL for DELETE */
  صحيح regOld,                     /* Address of array containing old row */
  صحيح *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  صحيح bChngRowid                  /* True if rowid is UPDATEd */
){
  /* If foreign-key support is enabled, iterate through all FKs that 
  ** refer to table pTab. If there is an action associated with the FK 
  ** for this operation (either update or delete), invoke the associated 
  ** trigger sub-program.  */
  إذا( pParse->db->flags&SQLITE_Foreign_کلیګانې ){
    FKey *pFKey;                  /* Iterator variable */
    لكل(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_بل_منزل){
      إذا( aChange==0 || fk_پلار_Is_Modified(pTab, pFKey, aChange, bChngRowid) ){
        Trigger *pAct = fkActionTrigger(pParse, pTab, pFKey, pChanges);
        إذا( pAct ){
          sqlite3_کوډ_لیکه_Trigger_مستقیم(pParse, pAct, pTab, regOld, OE_Abort, 0);
        }
      }
    }
  }
}

#د_که_پای /* ifndef SQLITE_OMIT_TRIGGER */

/*
** Free all memory associated with foreign key definitions attached to
** table pTab. Remove the deleted foreign keys from the Schema.fkeyHash
** hash table.
*/
فراغ sqlite3FkDelete(sqlite3 *db, جدول *pTab){
  FKey *pFKey;                    /* Iterator variable */
  FKey *p_بل;                    /* Copy of pFKey->pNextFrom */

  أكد( Is_Ordinary_جدول(pTab) );
  أكد( db!=0 );
  لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=p_بل){
    أكد( db==0 || sqlite3SchemaMutexHeld(db, 0, pTab->pSchema) );

    /* Remove the FK from the fkeyHash hash table. */
    إذا( db->pn_بایټونه_Freed==0 ){
      إذا( pFKey->p_Prev_منزل ){
        pFKey->p_Prev_منزل->p_بل_منزل = pFKey->p_بل_منزل;
      }وإلا{
        ثابت حرف *z = (pFKey->p_بل_منزل ? pFKey->p_بل_منزل->z_منزل : pFKey->z_منزل);
        sqlite3_Hash_ورننباسه(&pTab->pSchema->fkeyHash, z, pFKey->p_بل_منزل);
      }
      إذا( pFKey->p_بل_منزل ){
        pFKey->p_بل_منزل->p_Prev_منزل = pFKey->p_Prev_منزل;
      }
    }

    /* EV: R-30323-21917 Each foreign key constraint in SQLite is
    ** classified as either immediate or deferred.
    */
    أكد( pFKey->isDeferred==0 || pFKey->isDeferred==1 );

    /* Delete any triggers created to implement actions for this FK. */
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
    fkTriggerDelete(db, pFKey->apTrigger[0]);
    fkTriggerDelete(db, pFKey->apTrigger[1]);
#د_که_پای

    p_بل = pFKey->p_بل_From;
    sqlite3DbFree(db, pFKey);
  }
}
#د_که_پای /* ifndef SQLITE_OMIT_FOREIGN_KEY */
