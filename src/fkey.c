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
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER

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
పూర్ణాంకం sqlite3_Fk_Locate_సూచీ(
  Parse *pParse,                  /* Parse context to store any error in */
  పట్టిక *p_మాతృ,                 /* Parent table of FK constraint pFKey */
  FKey *pFKey,                    /* Foreign key to find index for */
  సూచీ **ppIdx,                  /* OUT: Unique index on parent table */
  పూర్ణాంకం **pai_నిలువు                    /* OUT: Map of index columns in pFKey */
){
  సూచీ *pIdx = 0;                    /* Value to return via *ppIdx */
  పూర్ణాంకం *ai_నిలువు = 0;                     /* Value to return via *paiCol */
  పూర్ణాంకం n_నిలువు = pFKey->n_నిలువు;             /* Number of columns in parent key */
  అక్షరం *z_కీలకం = pFKey->a_నిలువు[0].z_నిలువు;   /* Name of left-most parent key column */

  /* The caller is responsible for zeroing output parameters. */
  ప్రతిజ్ఞా( ppIdx && *ppIdx==0 );
  ప్రతిజ్ఞా( !pai_నిలువు || *pai_నిలువు==0 );
  ప్రతిజ్ఞా( pParse );

  /* If this is a non-composite (single column) foreign key, check if it 
  ** maps to the INTEGER PRIMARY KEY of table pParent. If so, leave *ppIdx 
  ** and *paiCol set to zero and return early. 
  **
  ** Otherwise, for a composite foreign key (more than one column), allocate
  ** space for the aiCol array (returned via output parameter *paiCol).
  ** Non-composite foreign keys do not require the aiCol array.
  */
  ఒకవేళ( n_నిలువు==1 ){
    /* The FK maps to the IPK if any of the following are true:
    **
    **   1) There is an INTEGER PRIMARY KEY column and the FK is implicitly 
    **      mapped to the primary key of table pParent, or
    **   2) The FK is explicitly mapped to a column declared as INTEGER
    **      PRIMARY KEY.
    */
    ఒకవేళ( p_మాతృ->iPKey>=0 ){
      ఒకవేళ( !z_కీలకం ) తిరిగి 0;
      ఒకవేళ( !sqlite3StrICmp(p_మాతృ->a_నిలువు[p_మాతృ->iPKey].zCnName, z_కీలకం) ){
        తిరిగి 0;
      }
    }
  }లేకపోతే ఒకవేళ( pai_నిలువు ){
    ప్రతిజ్ఞా( n_నిలువు>1 );
    ai_నిలువు = (పూర్ణాంకం *)sqlite3DbMallocRawNN(pParse->db, n_నిలువు*పరిమాణం(పూర్ణాంకం));
    ఒకవేళ( !ai_నిలువు ) తిరిగి 1;
    *pai_నిలువు = ai_నిలువు;
  }

  కొరకు(pIdx=p_మాతృ->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
    ఒకవేళ( pIdx->n_కీలకం_నిలువు==n_నిలువు && Is_Unique_సూచీ(pIdx) && pIdx->pPartIdxWhere==0 ){ 
      /* pIdx is a UNIQUE index (or a PRIMARY KEY) and has the right number
      ** of columns. If each indexed column corresponds to a foreign key
      ** column of pFKey, then this index is a winner.  */

      ఒకవేళ( z_కీలకం==0 ){
        /* If zKey is NULL, then this foreign key is implicitly mapped to 
        ** the PRIMARY KEY of table pParent. The PRIMARY KEY index may be 
        ** identified by the test.  */
        ఒకవేళ( Is_Primary_కీలకం_సూచీ(pIdx) ){
          ఒకవేళ( ai_నిలువు ){
            పూర్ణాంకం i;
            కొరకు(i=0; i<n_నిలువు; i++) ai_నిలువు[i] = pFKey->a_నిలువు[i].iFrom;
          }
          ఆపు;
        }
      }లేకపోతే{
        /* If zKey is non-NULL, then this foreign key was declared to
        ** map to an explicit list of columns in table pParent. Check if this
        ** index matches those columns. Also, check that the index uses
        ** the default collation sequences for each column. */
        పూర్ణాంకం i, j;
        కొరకు(i=0; i<n_నిలువు; i++){
          i16 i_నిలువు = pIdx->ai_స్తంభం[i];     /* Index of column in parent tbl */
          స్థిరరాశి అక్షరం *zDfltColl;            /* Def. collation for column */
          అక్షరం *z_Idx_నిలువు;                    /* Name of indexed column */

          ఒకవేళ( i_నిలువు<0 ) ఆపు; /* No foreign keys against expression indexes */

          /* If the index uses a collation sequence that is different from
          ** the default collation sequence for the column, this index is
          ** unusable. Bail out early in this case.  */
          zDfltColl = sqlite3_స్తంభం_Coll(&p_మాతృ->a_నిలువు[i_నిలువు]);
          ఒకవేళ( !zDfltColl ) zDfltColl = sqlite3_Str_ద్విమాన;
          ఒకవేళ( sqlite3StrICmp(pIdx->azColl[i], zDfltColl) ) ఆపు;

          z_Idx_నిలువు = p_మాతృ->a_నిలువు[i_నిలువు].zCnName;
          కొరకు(j=0; j<n_నిలువు; j++){
            ఒకవేళ( sqlite3StrICmp(pFKey->a_నిలువు[j].z_నిలువు, z_Idx_నిలువు)==0 ){
              ఒకవేళ( ai_నిలువు ) ai_నిలువు[i] = pFKey->a_నిలువు[j].iFrom;
              ఆపు;
            }
          }
          ఒకవేళ( j==n_నిలువు ) ఆపు;
        }
        ఒకవేళ( i==n_నిలువు ) ఆపు;      /* pIdx is usable */
      }
    }
  }

  ఒకవేళ( !pIdx ){
    ఒకవేళ( !pParse->disableTriggers ){
      sqlite3_Error_సందేశం(pParse,
           "foreign key mismatch - \"%w\" referencing \"%w\"",
           pFKey->pFrom->zName, pFKey->z_గమ్యం);
    }
    sqlite3DbFree(pParse->db, ai_నిలువు);
    తిరిగి 1;
  }

  *ppIdx = pIdx;
  తిరిగి 0;
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
స్థిర శూన్య fk_Lookup_మాతృ(
  Parse *pParse,        /* Parse context */
  పూర్ణాంకం iDb,              /* Index of database housing pTab */
  పట్టిక *pTab,          /* Parent table of FK pFKey */
  సూచీ *pIdx,          /* Unique index on parent key columns in pTab */
  FKey *pFKey,          /* Foreign key constraint */
  పూర్ణాంకం *ai_నిలువు,           /* Map from parent key columns to child table columns */
  పూర్ణాంకం reg_దత్తాంశం,          /* Address of array containing child table row */
  పూర్ణాంకం nIncr,            /* Increment constraint counter by this */
  పూర్ణాంకం isIgnore          /* If true, pretend pTab contains all NULL values */
){
  పూర్ణాంకం i;                                    /* Iterator variable */
  Vdbe *v = sqlite3GetVdbe(pParse);         /* Vdbe to add code to */
  పూర్ణాంకం iCur = pParse->nTab - 1;              /* Cursor number to use */
  పూర్ణాంకం iOk = sqlite3VdbeMakeLabel(pParse);   /* jump here if parent key found */

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
  ఒకవేళ( nIncr<0 ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_FkIfZero, pFKey->isDeferred, iOk);
    VdbeCoverage(v);
  }
  కొరకు(i=0; i<pFKey->n_నిలువు; i++){
    పూర్ణాంకం iReg = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pFKey->pFrom,ai_నిలువు[i]) + reg_దత్తాంశం + 1;
    sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, iReg, iOk); VdbeCoverage(v);
  }

  ఒకవేళ( isIgnore==0 ){
    ఒకవేళ( pIdx==0 ){
      /* If pIdx is NULL, then the parent key is the INTEGER PRIMARY KEY
      ** column of the parent table (table pTab).  */
      పూర్ణాంకం iMustBeInt;               /* Address of MustBeInt instruction */
      పూర్ణాంకం reg_తాత్కాలిక = sqlite3_Get_తాత్కాలిక_Reg(pParse);
  
      /* Invoke MustBeInt to coerce the child key value to an integer (i.e. 
      ** apply the affinity of the parent key). If this fails, then there
      ** is no matching parent key. Before using MustBeInt, make a copy of
      ** the value. Otherwise, the value inserted into the child key column
      ** will have INTEGER affinity applied to it, which may not be correct.  */
      sqlite3_Vdbe_కూడు_Op2(v, OP_SCopy, 
        sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pFKey->pFrom,ai_నిలువు[0])+1+reg_దత్తాంశం, reg_తాత్కాలిక);
      iMustBeInt = sqlite3_Vdbe_కూడు_Op2(v, OP_MustBeInt, reg_తాత్కాలిక, 0);
      VdbeCoverage(v);
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter.  */
      ఒకవేళ( pTab==pFKey->pFrom && nIncr==1 ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_Eq, reg_దత్తాంశం, iOk, reg_తాత్కాలిక); VdbeCoverage(v);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      }
  
      sqlite3_Open_పట్టిక(pParse, iCur, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_కూడు_Op3(v, OP_NotExists, iCur, 0, reg_తాత్కాలిక); VdbeCoverage(v);
      sqlite3VdbeGoto(v, iOk);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, sqlite3VdbeCurrentAddr(v)-2);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, iMustBeInt);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, reg_తాత్కాలిక);
    }లేకపోతే{
      పూర్ణాంకం n_నిలువు = pFKey->n_నిలువు;
      పూర్ణాంకం reg_తాత్కాలిక = sqlite3_Get_తాత్కాలిక_Range(pParse, n_నిలువు);
  
      sqlite3_Vdbe_కూడు_Op3(v, OP_OpenRead, iCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pIdx);
      కొరకు(i=0; i<n_నిలువు; i++){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Copy, 
               sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pFKey->pFrom, ai_నిలువు[i])+1+reg_దత్తాంశం,
               reg_తాత్కాలిక+i);
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
      ఒకవేళ( pTab==pFKey->pFrom && nIncr==1 ){
        పూర్ణాంకం iJump = sqlite3VdbeCurrentAddr(v) + n_నిలువు + 1;
        కొరకు(i=0; i<n_నిలువు; i++){
          పూర్ణాంకం i_బిడ్డ = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pFKey->pFrom,ai_నిలువు[i])
                              +1+reg_దత్తాంశం;
          పూర్ణాంకం i_మాతృ = 1+reg_దత్తాంశం;
          i_మాతృ += sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pIdx->p_పట్టిక,
                                                 pIdx->ai_స్తంభం[i]);
          ప్రతిజ్ఞా( pIdx->ai_స్తంభం[i]>=0 );
          ప్రతిజ్ఞా( ai_నిలువు[i]!=pTab->iPKey );
          ఒకవేళ( pIdx->ai_స్తంభం[i]==pTab->iPKey ){
            /* The parent key is a composite key that includes the IPK column */
            i_మాతృ = reg_దత్తాంశం;
          }
          sqlite3_Vdbe_కూడు_Op3(v, OP_Ne, i_బిడ్డ, iJump, i_మాతృ); VdbeCoverage(v);
          sqlite3VdbeChangeP5(v, SQLITE_JUMPIFNULL);
        }
        sqlite3VdbeGoto(v, iOk);
      }

      sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, reg_తాత్కాలిక, n_నిలువు, 0,
                        sqlite3_సూచీ_Affinity_Str(pParse->db,pIdx), n_నిలువు);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, iCur, iOk, reg_తాత్కాలిక, n_నిలువు);
      VdbeCoverage(v);
      sqlite3_Release_తాత్కాలిక_Range(pParse, reg_తాత్కాలిక, n_నిలువు);
    }
  }

  ఒకవేళ( !pFKey->isDeferred && !(pParse->db->flags & SQLITE_DeferFKs)
   && !pParse->pToplevel 
   && !pParse->isMultiWrite 
  ){
    /* Special case: If this is an INSERT statement that will insert exactly
    ** one row into the table, raise a constraint immediately instead of
    ** incrementing a counter. This is necessary as the VM code is being
    ** generated for will not open a statement transaction.  */
    ప్రతిజ్ఞా( nIncr==1 );
    sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
        OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
  }లేకపోతే{
    ఒకవేళ( nIncr>0 && pFKey->isDeferred==0 ){
      sqlite3MayAbort(pParse);
    }
    sqlite3_Vdbe_కూడు_Op2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
  }

  sqlite3VdbeResolveLabel(v, iOk);
  sqlite3_Vdbe_కూడు_Op1(v, OP_Close, iCur);
}


/*
** Return an Expr object that refers to a memory register corresponding
** to column iCol of table pTab.
**
** regBase is the first of an array of register that contains the data
** for pTab.  regBase itself holds the rowid.  regBase+1 holds the first
** column.  regBase+2 holds the second column, and so forth.
*/
స్థిర సమాసం *సమాసం_పట్టిక_రిజిస్టరు(
  Parse *pParse,     /* Parsing and code generating context */
  పట్టిక *pTab,       /* The table whose content is at r[regBase]... */
  పూర్ణాంకం reg_పునాది,       /* Contents of table pTab */
  i16 i_నిలువు           /* Which column of pTab is desired */
){
  సమాసం *p_సమాసం;
  స్తంభం *p_నిలువు;
  స్థిరరాశి అక్షరం *zColl;
  sqlite3 *db = pParse->db;

  p_సమాసం = sqlite3_సమాసం(db, TK_రిజిస్టరు, 0);
  ఒకవేళ( p_సమాసం ){
    ఒకవేళ( i_నిలువు>=0 && i_నిలువు!=pTab->iPKey ){
      p_నిలువు = &pTab->a_నిలువు[i_నిలువు];
      p_సమాసం->i_పట్టిక = reg_పునాది + sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab,i_నిలువు) + 1;
      p_సమాసం->aff_సమాసం = p_నిలువు->affinity;
      zColl = sqlite3_స్తంభం_Coll(p_నిలువు);
      ఒకవేళ( zColl==0 ) zColl = db->pDfltColl->zName;
      p_సమాసం = sqlite3_సమాసం_కూడు_Collate_String(pParse, p_సమాసం, zColl);
    }లేకపోతే{
      p_సమాసం->i_పట్టిక = reg_పునాది;
      p_సమాసం->aff_సమాసం = SQLITE_AFF_INTEGER;
    }
  }
  తిరిగి p_సమాసం;
}

/*
** Return an Expr object that refers to column iCol of table pTab which
** has cursor iCur.
*/
స్థిర సమాసం *సమాసం_పట్టిక_స్తంభం(
  sqlite3 *db,      /* The database connection */
  పట్టిక *pTab,      /* The table whose column is desired */
  పూర్ణాంకం iCursor,      /* The open cursor on the table */
  i16 i_నిలువు          /* The column that is wanted */
){
  సమాసం *p_సమాసం = sqlite3_సమాసం(db, TK_స్తంభం, 0);
  ఒకవేళ( p_సమాసం ){
    ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
    p_సమాసం->y.pTab = pTab;
    p_సమాసం->i_పట్టిక = iCursor;
    p_సమాసం->i_స్తంభం = i_నిలువు;
  }
  తిరిగి p_సమాసం;
}

/*
** Column iCol1 of table p1 is to be compared to column iCol2 of table p2.
** Return non-zero if the comparison will use an affinity compatible
** with the affinity of column iCol1 of table p1. Or zero if it will not.
**
** This is used in cases where (p1.iCol1 = p2.iCol2) must be evaluated
** using the affinity of p1.iCol1 as part of foreign key processing.
*/
స్థిర పూర్ణాంకం fk_ఉపయోగిస్తుంది_Lhs_Affinity(పట్టిక *p1, పూర్ణాంకం iCol1, పట్టిక *p2, పూర్ణాంకం iCol2){
  u8 aff1 = iCol1>=0 ? p1->a_నిలువు[iCol1].affinity : SQLITE_AFF_NUMERIC;
  u8 aff2 = p2->a_నిలువు[iCol2].affinity;

  /* If either side has a numeric affinity, then the comparison uses numeric
  ** affinity. So the affinity matches if aff1 is numeric, but not if aff1
  ** is not numeric but aff2 is.  */
  ఒకవేళ( sqlite3IsNumericAffinity(aff1) ) తిరిగి 1;
  ఒకవేళ( sqlite3IsNumericAffinity(aff2) ) తిరిగి 0;

  ప్రతిజ్ఞా( aff1==SQLITE_AFF_పాఠ్యం || aff1==SQLITE_AFF_BLOB );
  ప్రతిజ్ఞా( aff2==SQLITE_AFF_పాఠ్యం || aff2==SQLITE_AFF_BLOB );

  /* At this point, either the affinities are the same or one is TEXT and
  ** the other BLOB. In the second case the comparison will be done with
  ** BLOB affinity, which does not match if aff1 is TEXT.  */
  తిరిగి (aff1==aff2 || aff1==SQLITE_AFF_BLOB);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CAST
/*
** Return non-zero if a lookup on column iCol of table pTab, using collation
** sequence zColl, can be done using either the rowid or an index. That is,
** if iCol is the INTEGER PRIMARY KEY of pTab, or if pTab has an index
** whose left-most column is iCol and whose collation sequence for that
** column is zColl. Return zero otherwise.
*/
స్థిర పూర్ణాంకం fk_బిడ్డ_కీలకం_Is_సూచీకృత(పట్టిక *pTab, పూర్ణాంకం i_నిలువు, స్థిరరాశి అక్షరం *zColl){
  సూచీ *pIdx;
  ఒకవేళ( i_నిలువు==pTab->iPKey ) తిరిగి 1;
  కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
    ఒకవేళ( pIdx->ai_స్తంభం[0]==i_నిలువు
     && pIdx->pPartIdxWhere==0
     && sqlite3StrICmp(pIdx->azColl[0], zColl)==0
    ){
      తిరిగి 1;
    }
  }
  తిరిగి 0;
}
#అయితే_ముగింపు

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
స్థిర సమాసం *fk_మాతృ_బిడ్డ_Eq(
  Parse *pParse,            /* Parsing context */
  సమాసం *p_ఎడమ,              /* Parent key value */
  సమాసం *p_కుడి,             /* Unresolved reference to child key column */
  పట్టిక *pTab,              /* Parent table */
  పూర్ణాంకం i_నిలువు,                 /* Parent key column, or <0 for rowid */
  పట్టిక *p_బిడ్డ,            /* Child table */
  పూర్ణాంకం i_బిడ్డ_నిలువు,            /* Child key column */
  పూర్ణాంకం b_సంయుక్త            /* This is part of a composite FK */
){
  sqlite3 *db = pParse->db;
  సమాసం *pEq;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CAST
  సమాసం *p_లోపల = 0;
#అయితే_ముగింపు

  ఒకవేళ( fk_ఉపయోగిస్తుంది_Lhs_Affinity(pTab, i_నిలువు, p_బిడ్డ, i_బిడ్డ_నిలువు) ){
    తిరిగి sqlite3PExpr(pParse, TK_EQ, p_ఎడమ, p_కుడి);
  }
  ప్రతిజ్ఞా( i_నిలువు>=0 );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CAST
  {
    స్థిరరాశి అక్షరం *zColl = sqlite3_స్తంభం_Coll(&pTab->a_నిలువు[i_నిలువు]);
    ఒకవేళ( zColl==0 ) zColl = sqlite3_Str_ద్విమాన;

    /* The IN term is only useful if it can drive a lookup on the child
    ** table. Omit it otherwise, so that a full scan of the child table
    ** does not have to evaluate it for every row.  */
    ఒకవేళ( b_సంయుక్త || fk_బిడ్డ_కీలకం_Is_సూచీకృత(p_బిడ్డ, i_బిడ్డ_నిలువు, zColl) ){
      స్థిర స్థిరరాశి Token tNumeric = { "NUMERIC", 7 };
      సమాసం *pCast;
      సమాసం_List *pList;

      pCast = sqlite3_సమాసం_Alloc(db, TK_CAST, &tNumeric, 0);
      sqlite3_సమాసం_Attach_Subtrees(db, pCast, sqlite3_సమాసం_Dup(db, p_ఎడమ, 0), 0);
      pList = sqlite3_సమాసం_List_Append(pParse, 0,
          sqlite3PExpr(pParse, TK_UPLUS, sqlite3_సమాసం_Dup(db, p_ఎడమ, 0), 0)
      );
      pList = sqlite3_సమాసం_List_Append(pParse, pList,
          sqlite3PExpr(pParse, TK_UPLUS, pCast, 0)
      );
      p_లోపల = sqlite3PExpr(pParse, TK_లోపల, sqlite3_సమాసం_కూడు_Collate_String(pParse,
            sqlite3_సమాసం_Dup(db, p_కుడి, 0), zColl
      ), 0);
      ఒకవేళ( p_లోపల ){
        p_లోపల->x.pList = pList;
        sqlite3_సమాసం_సమితి_ఎత్తు_And_Flags(pParse, p_లోపల);
      }లేకపోతే{
        sqlite3_సమాసం_List_Delete(db, pList);
      }
    }
  }
#అయితే_ముగింపు

  p_కుడి = sqlite3PExpr(pParse, TK_UPLUS, p_కుడి, 0);
  pEq = sqlite3PExpr(pParse, TK_EQ, p_ఎడమ, p_కుడి);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CAST
  pEq = sqlite3_సమాసం_And(pParse, pEq, p_లోపల);
#అయితే_ముగింపు
  తిరిగి pEq;
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
స్థిర శూన్య fk_పరిశీలన_Children(
  Parse *pParse,                  /* Parse context */
  SrcList *pSrc,                  /* The child table to be scanned */
  పట్టిక *pTab,                    /* The parent table */
  సూచీ *pIdx,                    /* Index on parent covering the foreign key */
  FKey *pFKey,                    /* The foreign key linking pSrc to pTab */
  పూర్ణాంకం *ai_నిలువు,                     /* Map from pIdx cols to child table cols */
  పూర్ణాంకం reg_దత్తాంశం,                    /* Parent row data starts here */
  పూర్ణాంకం nIncr                       /* Amount to increment deferred counter by */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  పూర్ణాంకం i;                          /* Iterator variable */
  సమాసం *pWhere = 0;               /* WHERE clause to scan with */
  NameContext sNameContext;       /* Context used to resolve WHERE clause */
  WhereInfo *pWInfo;              /* Context used by sqlite3WhereXXX() */
  పూర్ణాంకం iFkIfZero = 0;              /* Address of OP_FkIfZero */
  Vdbe *v = sqlite3GetVdbe(pParse);

  ప్రతిజ్ఞా( pIdx==0 || pIdx->p_పట్టిక==pTab );
  ప్రతిజ్ఞా( pIdx==0 || pIdx->n_కీలకం_నిలువు==pFKey->n_నిలువు );
  ప్రతిజ్ఞా( pIdx!=0 || pFKey->n_నిలువు==1 );
  ప్రతిజ్ఞా( pIdx!=0 || HasRowid(pTab) );

  ఒకవేళ( nIncr<0 ){
    iFkIfZero = sqlite3_Vdbe_కూడు_Op2(v, OP_FkIfZero, pFKey->isDeferred, 0);
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
  కొరకు(i=0; i<pFKey->n_నిలువు; i++){
    సమాసం *p_ఎడమ;                  /* Value from parent table row */
    సమాసం *p_కుడి;                 /* Column ref to child table */
    సమాసం *pEq;                    /* Expression (pLeft = pRight) */
    i16 i_నిలువు;                     /* Index of column in parent table */
    i16 iCol2;                    /* Index of column in child table */
    స్థిరరాశి అక్షరం *z_నిలువు;             /* Name of column in child table */

    i_నిలువు = pIdx ? pIdx->ai_స్తంభం[i] : -1;
    p_ఎడమ = సమాసం_పట్టిక_రిజిస్టరు(pParse, pTab, reg_దత్తాంశం, i_నిలువు);
    iCol2 = ai_నిలువు ? ai_నిలువు[i] : pFKey->a_నిలువు[0].iFrom;
    ప్రతిజ్ఞా( iCol2>=0 );
    z_నిలువు = pFKey->pFrom->a_నిలువు[iCol2].zCnName;
    p_కుడి = sqlite3_సమాసం(db, TK_ID, z_నిలువు);
    pEq = fk_మాతృ_బిడ్డ_Eq(pParse, p_ఎడమ, p_కుడి, pTab, i_నిలువు,
                          pFKey->pFrom, iCol2, pFKey->n_నిలువు>0);
    pWhere = sqlite3_సమాసం_And(pParse, pWhere, pEq);
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
  ఒకవేళ( pTab==pFKey->pFrom && nIncr>0 ){
    సమాసం *pNe;                    /* Expression (pLeft != pRight) */
    సమాసం *p_ఎడమ;                  /* Value from parent table row */
    సమాసం *p_కుడి;                 /* Column ref to child table */
    ఒకవేళ( HasRowid(pTab) ){
      p_ఎడమ = సమాసం_పట్టిక_రిజిస్టరు(pParse, pTab, reg_దత్తాంశం, -1);
      p_కుడి = సమాసం_పట్టిక_స్తంభం(db, pTab, pSrc->a[0].iCursor, -1);
      pNe = sqlite3PExpr(pParse, TK_NE, p_ఎడమ, p_కుడి);
    }లేకపోతే{
      సమాసం *pEq, *pAll = 0;
      ప్రతిజ్ఞా( pIdx!=0 );
      కొరకు(i=0; i<pIdx->n_కీలకం_నిలువు; i++){
        i16 i_నిలువు = pIdx->ai_స్తంభం[i];
        ప్రతిజ్ఞా( i_నిలువు>=0 );
        p_ఎడమ = సమాసం_పట్టిక_రిజిస్టరు(pParse, pTab, reg_దత్తాంశం, i_నిలువు);
        p_కుడి = sqlite3_సమాసం(db, TK_ID, pTab->a_నిలువు[i_నిలువు].zCnName);
        pEq = sqlite3PExpr(pParse, TK_IS, p_ఎడమ, p_కుడి);
        pAll = sqlite3_సమాసం_And(pParse, pAll, pEq);
      }
      pNe = sqlite3PExpr(pParse, TK_NOT, pAll, 0);
    }
    pWhere = sqlite3_సమాసం_And(pParse, pWhere, pNe);
  }

  /* Resolve the references in the WHERE clause. */
  memset(&sNameContext, 0, పరిమాణం(NameContext));
  sNameContext.pSrcList = pSrc;
  sNameContext.pParse = pParse;
  sqlite3_Resolve_సమాసం_పేర్లు(&sNameContext, pWhere);

  /* Create VDBE to loop through the entries in pSrc that match the WHERE
  ** clause. For each row found, increment either the deferred or immediate
  ** foreign key constraint counter. */
  ఒకవేళ( pParse->nErr==0 ){
    pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0, 0);
    sqlite3_Vdbe_కూడు_Op2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
    ఒకవేళ( pWInfo ){
      sqlite3_Where_చివర(pWInfo);
    }
  }

  /* Clean up the WHERE clause constructed above. */
  sqlite3_సమాసం_Delete(db, pWhere);
  ఒకవేళ( iFkIfZero ){
    sqlite3_Vdbe_Jump_ఇక్కడ_Or_తీయి_Inst(v, iFkIfZero);
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
FKey *sqlite3FkReferences(పట్టిక *pTab){
  తిరిగి (FKey *)sqlite3_Hash_కనుగొను(&pTab->pSchema->fkeyHash, pTab->zName);
}

/*
** The second argument is a Trigger structure allocated by the 
** fkActionTrigger() routine. This function deletes the Trigger structure
** and all of its sub-components.
**
** The Trigger structure or any of its sub-components may be allocated from
** the lookaside buffer belonging to database handle dbMem.
*/
స్థిర శూన్య fkTriggerDelete(sqlite3 *dbMem, Trigger *p){
  ఒకవేళ( p ){
    Trigger_అడుగు *p_అడుగు = p->అడుగు_list;
    sqlite3SrcListDelete(dbMem, p_అడుగు->pSrc);
    sqlite3_సమాసం_Delete(dbMem, p_అడుగు->pWhere);
    sqlite3_సమాసం_List_Delete(dbMem, p_అడుగు->p_సమాసం_List);
    sqlite3SelectDelete(dbMem, p_అడుగు->pSelect);
    sqlite3_సమాసం_Delete(dbMem, p->pWhen);
    sqlite3DbFree(dbMem, p);
  }
}

/*
** Clear the apTrigger[] cache of CASCADE triggers for all foreign keys
** in a particular database.  This needs to happen when the schema
** changes.
*/
శూన్య sqlite3FkClearTriggerCache(sqlite3 *db, పూర్ణాంకం iDb){
  HashElem *k;
  Hash *pHash = &db->aDb[iDb].pSchema->tblHash;
  కొరకు(k=sqlite_Hash_మొదటి(pHash); k; k=sqlite_Hash_తదుపరి(k)){
    పట్టిక *pTab = sqlite_Hash_దత్తాంశం(k);
    FKey *pFKey;
    ఒకవేళ( !Is_Ordinary_పట్టిక(pTab) ) కొనసాగు;
    కొరకు(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_తదుపరి_From){
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
శూన్య sqlite3_Fk_Drop_పట్టిక(Parse *pParse, SrcList *pName, పట్టిక *pTab){
  sqlite3 *db = pParse->db;
  ఒకవేళ( (db->flags&SQLITE_Foreign_కీలకాలు) && Is_Ordinary_పట్టిక(pTab) ){
    పూర్ణాంకం iSkip = 0;
    Vdbe *v = sqlite3GetVdbe(pParse);

    ప్రతిజ్ఞా( v );                  /* VDBE has already been allocated */
    ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
    ఒకవేళ( sqlite3FkReferences(pTab)==0 ){
      /* Search for a deferred foreign key constraint for which this table
      ** is the child table. If one cannot be found, return without 
      ** generating any VDBE code. If one can be found, then jump over
      ** the entire DELETE if there are no outstanding deferred constraints
      ** when this statement is run.  */
      FKey *p;
      కొరకు(p=pTab->u.tab.pFKey; p; p=p->p_తదుపరి_From){
        ఒకవేళ( p->isDeferred || (db->flags & SQLITE_DeferFKs) ) ఆపు;
      }
      ఒకవేళ( !p ) తిరిగి;
      iSkip = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_కూడు_Op2(v, OP_FkIfZero, 1, iSkip); VdbeCoverage(v);
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
    ఒకవేళ( (db->flags & SQLITE_DeferFKs)==0 ){
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      sqlite3_Vdbe_కూడు_Op2(v, OP_FkIfZero, 0, sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
      sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
          OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
    }

    ఒకవేళ( iSkip ){
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
స్థిర పూర్ణాంకం fk_బిడ్డ_Is_Modified(
  పట్టిక *pTab,                    /* Table being updated */
  FKey *p,                        /* Foreign key for which pTab is the child */
  పూర్ణాంకం *aChange,                   /* Array indicating modified columns */
  పూర్ణాంకం bChngRowid                  /* True if rowid is modified by this update */
){
  పూర్ణాంకం i;
  కొరకు(i=0; i<p->n_నిలువు; i++){
    పూర్ణాంకం i_బిడ్డ_కీలకం = p->a_నిలువు[i].iFrom;
    ఒకవేళ( aChange[i_బిడ్డ_కీలకం]>=0 ) తిరిగి 1;
    ఒకవేళ( i_బిడ్డ_కీలకం==pTab->iPKey && bChngRowid ) తిరిగి 1;
  }
  తిరిగి 0;
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
స్థిర పూర్ణాంకం fk_మాతృ_Is_Modified(
  పట్టిక *pTab, 
  FKey *p, 
  పూర్ణాంకం *aChange, 
  పూర్ణాంకం bChngRowid
){
  పూర్ణాంకం i;
  కొరకు(i=0; i<p->n_నిలువు; i++){
    అక్షరం *z_కీలకం = p->a_నిలువు[i].z_నిలువు;
    పూర్ణాంకం i_కీలకం;
    కొరకు(i_కీలకం=0; i_కీలకం<pTab->n_నిలువు; i_కీలకం++){
      ఒకవేళ( aChange[i_కీలకం]>=0 || (i_కీలకం==pTab->iPKey && bChngRowid) ){
        స్తంభం *p_నిలువు = &pTab->a_నిలువు[i_కీలకం];
        ఒకవేళ( z_కీలకం ){
          ఒకవేళ( 0==sqlite3StrICmp(p_నిలువు->zCnName, z_కీలకం) ) తిరిగి 1;
        }లేకపోతే ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_PRIMKEY ){
          తిరిగి 1;
        }
      }
    }
  }
  తిరిగి 0;
}

/*
** Return true if the parser passed as the first argument is being
** used to code a trigger that is really a "SET NULL" action belonging
** to trigger pFKey.
*/
స్థిర పూర్ణాంకం is_సమితి_Null_Action(Parse *pParse, FKey *pFKey){
  Trigger *p = pParse->pTrigCoded;
  ఒకవేళ( p ){
    ఒకవేళ( (p==pFKey->apTrigger[0] && pFKey->aAction[0]==OE_సమితి_Null)
     || (p==pFKey->apTrigger[1] && pFKey->aAction[1]==OE_సమితి_Null)
    ){
      ప్రతిజ్ఞా( (pParse->db->flags & SQLITE_FkNoAction)==0 );
      తిరిగి 1;
    }
  }
  తిరిగి 0;
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
శూన్య sqlite3FkCheck(
  Parse *pParse,                  /* Parse context */
  పట్టిక *pTab,                    /* Row is being deleted from this table */ 
  పూర్ణాంకం regOld,                     /* Previous row data is stored here */
  పూర్ణాంకం regNew,                     /* New row data is stored here */
  పూర్ణాంకం *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  పూర్ణాంకం bChngRowid                  /* True if rowid is UPDATEd */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  FKey *pFKey;                    /* Used to iterate through FKs */
  పూర్ణాంకం iDb;                        /* Index of database containing pTab */
  స్థిరరాశి అక్షరం *zDb;                /* Name of database containing pTab */
  పూర్ణాంకం isIgnoreErrors = pParse->disableTriggers;

  /* Exactly one of regOld and regNew should be non-zero. */
  ప్రతిజ్ఞా( (regOld==0)!=(regNew==0) );

  /* If foreign-keys are disabled, this function is a no-op. */
  ఒకవేళ( (db->flags&SQLITE_Foreign_కీలకాలు)==0 ) తిరిగి;
  ఒకవేళ( !Is_Ordinary_పట్టిక(pTab) ) తిరిగి;

  iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
  ప్రతిజ్ఞా( iDb>=00 && iDb<db->nDb );
  zDb = db->aDb[iDb].zDbSName;

  /* Loop through all the foreign key constraints for which pTab is the
  ** child table (the table that the foreign key definition is part of).  */
  కొరకు(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_తదుపరి_From){
    పట్టిక *p_గమ్యం;                   /* Parent table of foreign key pFKey */
    సూచీ *pIdx = 0;              /* Index on key columns in pTo */
    పూర్ణాంకం *aiFree = 0;
    పూర్ణాంకం *ai_నిలువు;
    పూర్ణాంకం i_నిలువు;
    పూర్ణాంకం i;
    పూర్ణాంకం bIgnore = 0;

    ఒకవేళ( aChange 
     && sqlite3_stricmp(pTab->zName, pFKey->z_గమ్యం)!=0
     && fk_బిడ్డ_Is_Modified(pTab, pFKey, aChange, bChngRowid)==0 
    ){
      కొనసాగు;
    }

    /* Find the parent table of this foreign key. Also find a unique index 
    ** on the parent key columns in the parent table. If either of these 
    ** schema items cannot be located, set an error in pParse and return 
    ** early.  */
    ఒకవేళ( pParse->disableTriggers ){
      p_గమ్యం = sqlite3_కనుగొను_పట్టిక(db, pFKey->z_గమ్యం, zDb);
    }లేకపోతే{
      p_గమ్యం = sqlite3_Locate_పట్టిక(pParse, 0, pFKey->z_గమ్యం, zDb);
    }
    ఒకవేళ( !p_గమ్యం || sqlite3_Fk_Locate_సూచీ(pParse, p_గమ్యం, pFKey, &pIdx, &aiFree) ){
      ప్రతిజ్ఞా( isIgnoreErrors==0 || (regOld!=0 && regNew==0) );
      ఒకవేళ( !isIgnoreErrors || db->mallocFailed ) తిరిగి;
      ఒకవేళ( p_గమ్యం==0 ){
        /* If isIgnoreErrors is true, then a table is being dropped. In this
        ** case SQLite runs a "DELETE FROM xxx" on the table being dropped
        ** before actually dropping it in order to check FK constraints.
        ** If the parent table of an FK constraint on the current table is
        ** missing, behave as if it is empty. i.e. decrement the relevant
        ** FK counter for each row of the current table with non-NULL keys.
        */
        Vdbe *v = sqlite3GetVdbe(pParse);
        పూర్ణాంకం iJump = sqlite3VdbeCurrentAddr(v) + pFKey->n_నిలువు + 1;
        కొరకు(i=0; i<pFKey->n_నిలువు; i++){
          పూర్ణాంకం i_From_నిలువు, iReg;
          i_From_నిలువు = pFKey->a_నిలువు[i].iFrom;
          iReg = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pFKey->pFrom,i_From_నిలువు) + regOld+1;
          sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, iReg, iJump); VdbeCoverage(v);
        }
        sqlite3_Vdbe_కూడు_Op2(v, OP_FkCounter, pFKey->isDeferred, -1);
      }
      కొనసాగు;
    }
    ప్రతిజ్ఞా( pFKey->n_నిలువు==1 || (aiFree && pIdx) );

    ఒకవేళ( aiFree ){
      ai_నిలువు = aiFree;
    }లేకపోతే{
      i_నిలువు = pFKey->a_నిలువు[0].iFrom;
      ai_నిలువు = &i_నిలువు;
    }
    కొరకు(i=0; i<pFKey->n_నిలువు; i++){
      ఒకవేళ( ai_నిలువు[i]==pTab->iPKey ){
        ai_నిలువు[i] = -1;
      }
      ప్రతిజ్ఞా( pIdx==0 || pIdx->ai_స్తంభం[i]>=0 );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
      /* Request permission to read the parent key columns. If the 
      ** authorization callback returns SQLITE_IGNORE, behave as if any
      ** values read from the parent table are NULL. */
      ఒకవేళ( db->xAuth ){
        పూర్ణాంకం rcauth;
        అక్షరం *z_నిలువు = p_గమ్యం->a_నిలువు[pIdx ? pIdx->ai_స్తంభం[i] : p_గమ్యం->iPKey].zCnName;
        rcauth = sqlite3_Auth_Read_నిలువు(pParse, p_గమ్యం->zName, z_నిలువు, iDb);
        bIgnore = (rcauth==SQLITE_IGNORE);
      }
#అయితే_ముగింపు
    }

    /* Take a shared-cache advisory read-lock on the parent table. Allocate 
    ** a cursor to use to search the unique index on the parent key columns 
    ** in the parent table.  */
    sqlite3_పట్టిక_తాళం(pParse, iDb, p_గమ్యం->tnum, 0, p_గమ్యం->zName);
    pParse->nTab++;

    ఒకవేళ( regOld!=0 ){
      /* A row is being removed from the child table. Search for the parent.
      ** If the parent does not exist, removing the child row resolves an 
      ** outstanding foreign key constraint violation. */
      fk_Lookup_మాతృ(pParse, iDb, p_గమ్యం, pIdx, pFKey, ai_నిలువు, regOld, -1, bIgnore);
    }
    ఒకవేళ( regNew!=0 && !is_సమితి_Null_Action(pParse, pFKey) ){
      /* A row is being added to the child table. If a parent row cannot
      ** be found, adding the child row has violated the FK constraint. 
      **
      ** If this operation is being performed as part of a trigger program
      ** that is actually a "SET NULL" action belonging to this very 
      ** foreign key, then omit this scan altogether. As all child key
      ** values are guaranteed to be NULL, it is not possible for adding
      ** this row to cause an FK violation.  */
      fk_Lookup_మాతృ(pParse, iDb, p_గమ్యం, pIdx, pFKey, ai_నిలువు, regNew, +1, bIgnore);
    }

    sqlite3DbFree(db, aiFree);
  }

  /* Loop through all the foreign key constraints that refer to this table.
  ** (the "child" constraints) */
  కొరకు(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_తదుపరి_గమ్యం){
    సూచీ *pIdx = 0;              /* Foreign key index for pFKey */
    SrcList *pSrc;
    పూర్ణాంకం *ai_నిలువు = 0;

    ఒకవేళ( aChange && fk_మాతృ_Is_Modified(pTab, pFKey, aChange, bChngRowid)==0 ){
      కొనసాగు;
    }

    ఒకవేళ( !pFKey->isDeferred && !(db->flags & SQLITE_DeferFKs) 
     && !pParse->pToplevel && !pParse->isMultiWrite 
    ){
      ప్రతిజ్ఞా( regOld==0 && regNew!=0 );
      /* Inserting a single row into a parent table cannot cause (or fix)
      ** an immediate foreign key violation. So do nothing in this case.  */
      కొనసాగు;
    }

    ఒకవేళ( sqlite3_Fk_Locate_సూచీ(pParse, pTab, pFKey, &pIdx, &ai_నిలువు) ){
      ఒకవేళ( !isIgnoreErrors || db->mallocFailed ) తిరిగి;
      కొనసాగు;
    }
    ప్రతిజ్ఞా( ai_నిలువు || pFKey->n_నిలువు==1 );

    /* Create a SrcList structure containing the child table.  We need the
    ** child table as a SrcList for sqlite3WhereBegin() */
    pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
    ఒకవేళ( pSrc ){
      Src_అంశం *p_అంశం = pSrc->a;
      p_అంశం->pSTab = pFKey->pFrom;
      p_అంశం->zName = pFKey->pFrom->zName;
      p_అంశం->pSTab->nTabRef++;
      p_అంశం->iCursor = pParse->nTab++;
  
      ఒకవేళ( regNew!=0 ){
        fk_పరిశీలన_Children(pParse, pSrc, pTab, pIdx, pFKey, ai_నిలువు, regNew, -1);
      }
      ఒకవేళ( regOld!=0 ){
        పూర్ణాంకం eAction = pFKey->aAction[aChange!=0];
        ఒకవేళ( (db->flags & SQLITE_FkNoAction) ) eAction = OE_None;

        fk_పరిశీలన_Children(pParse, pSrc, pTab, pIdx, pFKey, ai_నిలువు, regOld, 1);
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
        ఒకవేళ( !pFKey->isDeferred && eAction!=OE_Cascade && eAction!=OE_సమితి_Null ){
          sqlite3MayAbort(pParse);
        }
      }
      p_అంశం->zName = 0;
      sqlite3SrcListDelete(db, pSrc);
    }
    sqlite3DbFree(db, ai_నిలువు);
  }
}

#నిర్వచించు స్తంభం_ముసుగు(x) (((x)>31) ? 0xffffffff : ((u32)1<<(x)))

/*
** This function is called before generating code to update or delete a 
** row contained in table pTab.
*/
u32 sqlite3FkOldmask(
  Parse *pParse,                  /* Parse context */
  పట్టిక *pTab                     /* Table being modified */
){
  u32 ముసుగు = 0;
  ఒకవేళ( pParse->db->flags&SQLITE_Foreign_కీలకాలు && Is_Ordinary_పట్టిక(pTab) ){
    FKey *p;
    పూర్ణాంకం i;
    కొరకు(p=pTab->u.tab.pFKey; p; p=p->p_తదుపరి_From){
      కొరకు(i=0; i<p->n_నిలువు; i++) ముసుగు |= స్తంభం_ముసుగు(p->a_నిలువు[i].iFrom);
    }
    కొరకు(p=sqlite3FkReferences(pTab); p; p=p->p_తదుపరి_గమ్యం){
      సూచీ *pIdx = 0;
      sqlite3_Fk_Locate_సూచీ(pParse, pTab, p, &pIdx, 0);
      ఒకవేళ( pIdx ){
        కొరకు(i=0; i<pIdx->n_కీలకం_నిలువు; i++){
          ప్రతిజ్ఞా( pIdx->ai_స్తంభం[i]>=0 );
          ముసుగు |= స్తంభం_ముసుగు(pIdx->ai_స్తంభం[i]);
        }
      }
    }
  }
  తిరిగి ముసుగు;
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
పూర్ణాంకం sqlite3FkRequired(
  Parse *pParse,                  /* Parse context */
  పట్టిక *pTab,                    /* Table being modified */
  పూర్ణాంకం *aChange,                   /* Non-NULL for UPDATE operations */
  పూర్ణాంకం chngRowid                   /* True for UPDATE that affects rowid */
){
  పూర్ణాంకం eRet = 1;                   /* Value to return if bHaveFK is true */
  పూర్ణాంకం bHaveFK = 0;                /* If FK processing is required */
  ఒకవేళ( pParse->db->flags&SQLITE_Foreign_కీలకాలు && Is_Ordinary_పట్టిక(pTab) ){
    ఒకవేళ( !aChange ){
      /* A DELETE operation. Foreign key processing is required if the 
      ** table in question is either the child or parent table for any 
      ** foreign key constraint.  */
      bHaveFK = (sqlite3FkReferences(pTab) || pTab->u.tab.pFKey);
    }లేకపోతే{
      /* This is an UPDATE. Foreign key processing is only required if the
      ** operation modifies one or more child or parent key columns. */
      FKey *p;

      /* Check if any child key columns are being modified. */
      కొరకు(p=pTab->u.tab.pFKey; p; p=p->p_తదుపరి_From){
        ఒకవేళ( fk_బిడ్డ_Is_Modified(pTab, p, aChange, chngRowid) ){
          ఒకవేళ( 0==sqlite3_stricmp(pTab->zName, p->z_గమ్యం) ) eRet = 2;
          bHaveFK = 1;
        }
      }

      /* Check if any parent key columns are being modified. */
      కొరకు(p=sqlite3FkReferences(pTab); p; p=p->p_తదుపరి_గమ్యం){
        ఒకవేళ( fk_మాతృ_Is_Modified(pTab, p, aChange, chngRowid) ){
          ఒకవేళ( (pParse->db->flags & SQLITE_FkNoAction)==0 
           && p->aAction[1]!=OE_None 
          ){
            తిరిగి 2;
          }
          bHaveFK = 1;
        }
      }
    }
  }
  తిరిగి bHaveFK ? eRet : 0;
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
స్థిర Trigger *fkActionTrigger(
  Parse *pParse,                  /* Parse context */
  పట్టిక *pTab,                    /* Table being updated or deleted from */
  FKey *pFKey,                    /* Foreign key to get action for */
  సమాసం_List *pChanges              /* Change-list for UPDATE, NULL for DELETE */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  పూర్ణాంకం action;                     /* One of OE_None, OE_Cascade etc. */
  Trigger *pTrigger;              /* Trigger definition to return */
  పూర్ణాంకం iAction = (pChanges!=0);    /* 1 for UPDATE, 0 for DELETE */

  action = pFKey->aAction[iAction];
  ఒకవేళ( (db->flags & SQLITE_FkNoAction) ) action = OE_None;
  ఒకవేళ( action==OE_Restrict && (db->flags & SQLITE_DeferFKs) ){
    తిరిగి 0;
  }
  pTrigger = pFKey->apTrigger[iAction];

  ఒకవేళ( action!=OE_None && !pTrigger ){
    అక్షరం స్థిరరాశి *zFrom;            /* Name of child table */
    పూర్ణాంకం nFrom;                    /* Length in bytes of zFrom */
    సూచీ *pIdx = 0;              /* Parent key index for this FK */
    పూర్ణాంకం *ai_నిలువు = 0;               /* child table cols -> parent key cols */
    Trigger_అడుగు *p_అడుగు = 0;        /* First (only) step of trigger program */
    సమాసం *pWhere = 0;             /* WHERE clause of trigger step */
    సమాసం_List *pList = 0;          /* Changes list if ON UPDATE CASCADE */
    Select *pSelect = 0;          /* If RESTRICT, "SELECT RAISE(...)" */
    పూర్ణాంకం i;                        /* Iterator variable */
    సమాసం *pWhen = 0;              /* WHEN clause for the trigger */

    ఒకవేళ( sqlite3_Fk_Locate_సూచీ(pParse, pTab, pFKey, &pIdx, &ai_నిలువు) ) తిరిగి 0;
    ప్రతిజ్ఞా( ai_నిలువు || pFKey->n_నిలువు==1 );

    కొరకు(i=0; i<pFKey->n_నిలువు; i++){
      Token tOld = { "old", 3 };  /* Literal "old" token */
      Token tNew = { "new", 3 };  /* Literal "new" token */
      Token t_From_నిలువు;             /* Name of column in child table */
      Token t_గమ్యం_నిలువు;               /* Name of column in parent table */
      పూర్ణాంకం i_From_నిలువు;               /* Idx of column in child table */
      పూర్ణాంకం i_గమ్యం_నిలువు;                 /* Idx of column in parent table */
      సమాసం *pEq;                  /* OLD.tToCol = tFromCol */
      సమాసం *p_ఎడమ;                /* OLD.tToCol */
      సమాసం *p_కుడి;               /* tFromCol */

      i_From_నిలువు = ai_నిలువు ? ai_నిలువు[i] : pFKey->a_నిలువు[0].iFrom;
      ప్రతిజ్ఞా( i_From_నిలువు>=0 );
      ప్రతిజ్ఞా( pIdx!=0 || (pTab->iPKey>=0 && pTab->iPKey<pTab->n_నిలువు) );
      ప్రతిజ్ఞా( pIdx==0 || pIdx->ai_స్తంభం[i]>=0 );
      i_గమ్యం_నిలువు = pIdx ? pIdx->ai_స్తంభం[i] : pTab->iPKey;
      sqlite3TokenInit(&t_గమ్యం_నిలువు, pTab->a_నిలువు[i_గమ్యం_నిలువు].zCnName);
      sqlite3TokenInit(&t_From_నిలువు, pFKey->pFrom->a_నిలువు[i_From_నిలువు].zCnName);

      p_ఎడమ = sqlite3PExpr(pParse, TK_DOT,
          sqlite3_సమాసం_Alloc(db, TK_ID, &tOld, 0),
          sqlite3_సమాసం_Alloc(db, TK_ID, &t_గమ్యం_నిలువు, 0)
      );
      p_కుడి = sqlite3_సమాసం_Alloc(db, TK_ID, &t_From_నిలువు, 0);

      /* OLD.* columns do not normally have an affinity. So set the affinity
      ** of OLD.tToCol (the parent key) explicitly here. */
      ఒకవేళ( p_ఎడమ ) p_ఎడమ->aff_సమాసం = pTab->a_నిలువు[i_గమ్యం_నిలువు].affinity;

      /* Create the expression "OLD.tToCol = tFromCol". It is important
      ** that the "OLD.tToCol" term is on the LHS of the = operator, so
      ** that the collation sequence associated with the parent table is
      ** used for the comparison. */
      pEq = fk_మాతృ_బిడ్డ_Eq(pParse, p_ఎడమ, p_కుడి, pTab, i_గమ్యం_నిలువు,
                            pFKey->pFrom, i_From_నిలువు, pFKey->n_నిలువు>1);
      pWhere = sqlite3_సమాసం_And(pParse, pWhere, pEq);

      /* For ON UPDATE, construct the next term of the WHEN clause.
      ** The final WHEN clause will be like this:
      **
      **    WHEN NOT(old.col1 IS new.col1 AND ... AND old.colN IS new.colN)
      */
      ఒకవేళ( pChanges ){
        pEq = sqlite3PExpr(pParse, TK_IS,
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_సమాసం_Alloc(db, TK_ID, &tOld, 0),
              sqlite3_సమాసం_Alloc(db, TK_ID, &t_గమ్యం_నిలువు, 0)),
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_సమాసం_Alloc(db, TK_ID, &tNew, 0),
              sqlite3_సమాసం_Alloc(db, TK_ID, &t_గమ్యం_నిలువు, 0))
            );
        pWhen = sqlite3_సమాసం_And(pParse, pWhen, pEq);
      }
  
      ఒకవేళ( action!=OE_Restrict && (action!=OE_Cascade || pChanges) ){
        సమాసం *pNew;
        ఒకవేళ( action==OE_Cascade ){
          pNew = sqlite3PExpr(pParse, TK_DOT, 
            sqlite3_సమాసం_Alloc(db, TK_ID, &tNew, 0),
            sqlite3_సమాసం_Alloc(db, TK_ID, &t_గమ్యం_నిలువు, 0));
        }లేకపోతే ఒకవేళ( action==OE_సమితి_Dflt ){
          స్తంభం *p_నిలువు = pFKey->pFrom->a_నిలువు + i_From_నిలువు;
          సమాసం *pDflt;
          ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_GENERATED ){
            testcase( p_నిలువు->నిలువు_Flags & COLFLAG_VIRTUAL );
            testcase( p_నిలువు->నిలువు_Flags & COLFLAG_STORED );
            pDflt = 0;
          }లేకపోతే{
            pDflt = sqlite3_స్తంభం_సమాసం_Auth(pFKey->pFrom, p_నిలువు, pParse);
          }
          ఒకవేళ( pDflt ){
            pNew = sqlite3_సమాసం_Dup(db, pDflt, 0);
          }లేకపోతే{
            pNew = sqlite3_సమాసం_Alloc(db, TK_NULL, 0, 0);
          }
        }లేకపోతే{
          pNew = sqlite3_సమాసం_Alloc(db, TK_NULL, 0, 0);
        }
        pList = sqlite3_సమాసం_List_Append(pParse, pList, pNew);
        sqlite3_సమాసం_List_సమితి_Name(pParse, pList, &t_From_నిలువు, 0);
      }
    }
    sqlite3DbFree(db, ai_నిలువు);

    zFrom = pFKey->pFrom->zName;
    nFrom = sqlite3Strlen30(zFrom);

    ఒకవేళ( action==OE_Restrict ){
      SrcList *pSrc;
      సమాసం *pRaise; 

      pRaise = sqlite3_సమాసం(db, TK_STRING, "FOREIGN KEY constraint failed"),
      pRaise = sqlite3PExpr(pParse, TK_RAISE, pRaise, 0);
      ఒకవేళ( pRaise ){
        pRaise->aff_సమాసం = OE_Abort;
      }
      pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      ఒకవేళ( pSrc ){
        Src_అంశం *p_అంశం = &pSrc->a[0];
        p_అంశం->zName = sqlite3DbStrDup(db, zFrom);
        p_అంశం->fg.fixedSchema = 1;
        p_అంశం->u4.pSchema = pTab->pSchema;
      }
      pSelect = sqlite3SelectNew(pParse, 
          sqlite3_సమాసం_List_Append(pParse, 0, pRaise),
          pSrc,
          pWhere,
          0, 0, 0, 0, 0
      );
      pWhere = 0;
    }

    /* Disable lookaside memory allocation */
    DisableLookaside;

    pTrigger = (Trigger *)sqlite3DbMallocZero(db, 
        పరిమాణం(Trigger) +         /* struct Trigger */
        పరిమాణం(Trigger_అడుగు)       /* Single step in trigger program */
    );
    ఒకవేళ( pTrigger ){
      p_అడుగు = pTrigger->అడుగు_list = (Trigger_అడుగు *)&pTrigger[1];
      p_అడుగు->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      ఒకవేళ( p_అడుగు->pSrc ){
        Src_అంశం *p_అంశం = &p_అడుగు->pSrc->a[0];
        p_అంశం->zName = sqlite3DbStrNDup(db, zFrom, nFrom);
        p_అంశం->u4.pSchema = pTab->pSchema;
        p_అంశం->fg.fixedSchema = 1;
      }
      p_అడుగు->pWhere = sqlite3_సమాసం_Dup(db, pWhere, EXPRDUP_REDUCE);
      p_అడుగు->p_సమాసం_List = sqlite3_సమాసం_List_Dup(db, pList, EXPRDUP_REDUCE);
      p_అడుగు->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
      ఒకవేళ( pWhen ){
        pWhen = sqlite3PExpr(pParse, TK_NOT, pWhen, 0);
        pTrigger->pWhen = sqlite3_సమాసం_Dup(db, pWhen, EXPRDUP_REDUCE);
      }
    }

    /* Re-enable the lookaside buffer, if it was disabled earlier. */
    EnableLookaside;

    sqlite3_సమాసం_Delete(db, pWhere);
    sqlite3_సమాసం_Delete(db, pWhen);
    sqlite3_సమాసం_List_Delete(db, pList);
    sqlite3SelectDelete(db, pSelect);
    ఒకవేళ( db->mallocFailed==1 ){
      fkTriggerDelete(db, pTrigger);
      తిరిగి 0;
    }
    ప్రతిజ్ఞా( p_అడుగు!=0 );
    ప్రతిజ్ఞా( pTrigger!=0 );

    ఎంపిక( action ){
      స్థితి OE_Restrict:
        p_అడుగు->op = TK_SELECT;
        ఆపు;
      స్థితి OE_Cascade: 
        ఒకవేళ( !pChanges ){ 
          p_అడుగు->op = TK_DELETE; 
          ఆపు; 
        }
        /* no break */ deliberate_fall_through
      అప్రమేయ:
        p_అడుగు->op = TK_UPDATE;
    }
    p_అడుగు->pTrig = pTrigger;
    pTrigger->pSchema = pTab->pSchema;
    pTrigger->pTabSchema = pTab->pSchema;
    pFKey->apTrigger[iAction] = pTrigger;
    pTrigger->op = (pChanges ? TK_UPDATE : TK_DELETE);
  }

  తిరిగి pTrigger;
}

/*
** This function is called when deleting or updating a row to implement
** any required CASCADE, SET NULL or SET DEFAULT actions.
*/
శూన్య sqlite3FkActions(
  Parse *pParse,                  /* Parse context */
  పట్టిక *pTab,                    /* Table being updated or deleted from */
  సమాసం_List *pChanges,             /* Change-list for UPDATE, NULL for DELETE */
  పూర్ణాంకం regOld,                     /* Address of array containing old row */
  పూర్ణాంకం *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  పూర్ణాంకం bChngRowid                  /* True if rowid is UPDATEd */
){
  /* If foreign-key support is enabled, iterate through all FKs that 
  ** refer to table pTab. If there is an action associated with the FK 
  ** for this operation (either update or delete), invoke the associated 
  ** trigger sub-program.  */
  ఒకవేళ( pParse->db->flags&SQLITE_Foreign_కీలకాలు ){
    FKey *pFKey;                  /* Iterator variable */
    కొరకు(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_తదుపరి_గమ్యం){
      ఒకవేళ( aChange==0 || fk_మాతృ_Is_Modified(pTab, pFKey, aChange, bChngRowid) ){
        Trigger *pAct = fkActionTrigger(pParse, pTab, pFKey, pChanges);
        ఒకవేళ( pAct ){
          sqlite3_సంకేతం_అడ్డువరుస_Trigger_ప్రత్యక్ష(pParse, pAct, pTab, regOld, OE_Abort, 0);
        }
      }
    }
  }
}

#అయితే_ముగింపు /* ifndef SQLITE_OMIT_TRIGGER */

/*
** Free all memory associated with foreign key definitions attached to
** table pTab. Remove the deleted foreign keys from the Schema.fkeyHash
** hash table.
*/
శూన్య sqlite3FkDelete(sqlite3 *db, పట్టిక *pTab){
  FKey *pFKey;                    /* Iterator variable */
  FKey *p_తదుపరి;                    /* Copy of pFKey->pNextFrom */

  ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
  ప్రతిజ్ఞా( db!=0 );
  కొరకు(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=p_తదుపరి){
    ప్రతిజ్ఞా( db==0 || sqlite3SchemaMutexHeld(db, 0, pTab->pSchema) );

    /* Remove the FK from the fkeyHash hash table. */
    ఒకవేళ( db->pn_బైట్లు_Freed==0 ){
      ఒకవేళ( pFKey->p_Prev_గమ్యం ){
        pFKey->p_Prev_గమ్యం->p_తదుపరి_గమ్యం = pFKey->p_తదుపరి_గమ్యం;
      }లేకపోతే{
        స్థిరరాశి అక్షరం *z = (pFKey->p_తదుపరి_గమ్యం ? pFKey->p_తదుపరి_గమ్యం->z_గమ్యం : pFKey->z_గమ్యం);
        sqlite3_Hash_చొప్పించు(&pTab->pSchema->fkeyHash, z, pFKey->p_తదుపరి_గమ్యం);
      }
      ఒకవేళ( pFKey->p_తదుపరి_గమ్యం ){
        pFKey->p_తదుపరి_గమ్యం->p_Prev_గమ్యం = pFKey->p_Prev_గమ్యం;
      }
    }

    /* EV: R-30323-21917 Each foreign key constraint in SQLite is
    ** classified as either immediate or deferred.
    */
    ప్రతిజ్ఞా( pFKey->isDeferred==0 || pFKey->isDeferred==1 );

    /* Delete any triggers created to implement actions for this FK. */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
    fkTriggerDelete(db, pFKey->apTrigger[0]);
    fkTriggerDelete(db, pFKey->apTrigger[1]);
#అయితే_ముగింపు

    p_తదుపరి = pFKey->p_తదుపరి_From;
    sqlite3DbFree(db, pFKey);
  }
}
#అయితే_ముగింపు /* ifndef SQLITE_OMIT_FOREIGN_KEY */
