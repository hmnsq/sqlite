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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER

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
ਪੂਰਨ sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(
  Parse *pParse,                  /* Parse context to store any error in */
  ਸਾਰਣੀ *p_ਮਾਪਾ,                 /* Parent table of FK constraint pFKey */
  FKey *pFKey,                    /* Foreign key to find index for */
  ਸੂਚਕਾਂਕ **ppIdx,                  /* OUT: Unique index on parent table */
  ਪੂਰਨ **pai_ਖਾਨਾ                    /* OUT: Map of index columns in pFKey */
){
  ਸੂਚਕਾਂਕ *pIdx = 0;                    /* Value to return via *ppIdx */
  ਪੂਰਨ *ai_ਖਾਨਾ = 0;                     /* Value to return via *paiCol */
  ਪੂਰਨ n_ਖਾਨਾ = pFKey->n_ਖਾਨਾ;             /* Number of columns in parent key */
  ਅੱਖਰ *z_ਕੁੰਜੀ = pFKey->a_ਖਾਨਾ[0].z_ਖਾਨਾ;   /* Name of left-most parent key column */

  /* The caller is responsible for zeroing output parameters. */
  ਪ੍ਰਤਿਜ੍ਞਾ( ppIdx && *ppIdx==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !pai_ਖਾਨਾ || *pai_ਖਾਨਾ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse );

  /* If this is a non-composite (single column) foreign key, check if it 
  ** maps to the INTEGER PRIMARY KEY of table pParent. If so, leave *ppIdx 
  ** and *paiCol set to zero and return early. 
  **
  ** Otherwise, for a composite foreign key (more than one column), allocate
  ** space for the aiCol array (returned via output parameter *paiCol).
  ** Non-composite foreign keys do not require the aiCol array.
  */
  ਜੇ( n_ਖਾਨਾ==1 ){
    /* The FK maps to the IPK if any of the following are true:
    **
    **   1) There is an INTEGER PRIMARY KEY column and the FK is implicitly 
    **      mapped to the primary key of table pParent, or
    **   2) The FK is explicitly mapped to a column declared as INTEGER
    **      PRIMARY KEY.
    */
    ਜੇ( p_ਮਾਪਾ->iPKey>=0 ){
      ਜੇ( !z_ਕੁੰਜੀ ) ਵਾਪਸ 0;
      ਜੇ( !sqlite3StrICmp(p_ਮਾਪਾ->a_ਖਾਨਾ[p_ਮਾਪਾ->iPKey].zCnName, z_ਕੁੰਜੀ) ){
        ਵਾਪਸ 0;
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pai_ਖਾਨਾ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਖਾਨਾ>1 );
    ai_ਖਾਨਾ = (ਪੂਰਨ *)sqlite3DbMallocRawNN(pParse->db, n_ਖਾਨਾ*ਆਕਾਰ(ਪੂਰਨ));
    ਜੇ( !ai_ਖਾਨਾ ) ਵਾਪਸ 1;
    *pai_ਖਾਨਾ = ai_ਖਾਨਾ;
  }

  ਲਈ(pIdx=p_ਮਾਪਾ->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
    ਜੇ( pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ==n_ਖਾਨਾ && Is_Unique_ਸੂਚਕਾਂਕ(pIdx) && pIdx->pPartIdxWhere==0 ){ 
      /* pIdx is a UNIQUE index (or a PRIMARY KEY) and has the right number
      ** of columns. If each indexed column corresponds to a foreign key
      ** column of pFKey, then this index is a winner.  */

      ਜੇ( z_ਕੁੰਜੀ==0 ){
        /* If zKey is NULL, then this foreign key is implicitly mapped to 
        ** the PRIMARY KEY of table pParent. The PRIMARY KEY index may be 
        ** identified by the test.  */
        ਜੇ( Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) ){
          ਜੇ( ai_ਖਾਨਾ ){
            ਪੂਰਨ i;
            ਲਈ(i=0; i<n_ਖਾਨਾ; i++) ai_ਖਾਨਾ[i] = pFKey->a_ਖਾਨਾ[i].iFrom;
          }
          ਰੋਕੋ;
        }
      }ਨਹੀਂਤਾਂ{
        /* If zKey is non-NULL, then this foreign key was declared to
        ** map to an explicit list of columns in table pParent. Check if this
        ** index matches those columns. Also, check that the index uses
        ** the default collation sequences for each column. */
        ਪੂਰਨ i, j;
        ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
          i16 i_ਖਾਨਾ = pIdx->ai_ਕਾਲਮ[i];     /* Index of column in parent tbl */
          ਅਚਲ ਅੱਖਰ *zDfltColl;            /* Def. collation for column */
          ਅੱਖਰ *z_Idx_ਖਾਨਾ;                    /* Name of indexed column */

          ਜੇ( i_ਖਾਨਾ<0 ) ਰੋਕੋ; /* No foreign keys against expression indexes */

          /* If the index uses a collation sequence that is different from
          ** the default collation sequence for the column, this index is
          ** unusable. Bail out early in this case.  */
          zDfltColl = sqlite3_ਕਾਲਮ_Coll(&p_ਮਾਪਾ->a_ਖਾਨਾ[i_ਖਾਨਾ]);
          ਜੇ( !zDfltColl ) zDfltColl = sqlite3_Str_ਦੁਆਧਾਰੀ;
          ਜੇ( sqlite3StrICmp(pIdx->azColl[i], zDfltColl) ) ਰੋਕੋ;

          z_Idx_ਖਾਨਾ = p_ਮਾਪਾ->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName;
          ਲਈ(j=0; j<n_ਖਾਨਾ; j++){
            ਜੇ( sqlite3StrICmp(pFKey->a_ਖਾਨਾ[j].z_ਖਾਨਾ, z_Idx_ਖਾਨਾ)==0 ){
              ਜੇ( ai_ਖਾਨਾ ) ai_ਖਾਨਾ[i] = pFKey->a_ਖਾਨਾ[j].iFrom;
              ਰੋਕੋ;
            }
          }
          ਜੇ( j==n_ਖਾਨਾ ) ਰੋਕੋ;
        }
        ਜੇ( i==n_ਖਾਨਾ ) ਰੋਕੋ;      /* pIdx is usable */
      }
    }
  }

  ਜੇ( !pIdx ){
    ਜੇ( !pParse->disableTriggers ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse,
           "foreign key mismatch - \"%w\" referencing \"%w\"",
           pFKey->pFrom->zName, pFKey->z_ਮੰਜ਼ਿਲ);
    }
    sqlite3DbFree(pParse->db, ai_ਖਾਨਾ);
    ਵਾਪਸ 1;
  }

  *ppIdx = pIdx;
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਖਾਲੀ fk_Lookup_ਮਾਪਾ(
  Parse *pParse,        /* Parse context */
  ਪੂਰਨ iDb,              /* Index of database housing pTab */
  ਸਾਰਣੀ *pTab,          /* Parent table of FK pFKey */
  ਸੂਚਕਾਂਕ *pIdx,          /* Unique index on parent key columns in pTab */
  FKey *pFKey,          /* Foreign key constraint */
  ਪੂਰਨ *ai_ਖਾਨਾ,           /* Map from parent key columns to child table columns */
  ਪੂਰਨ reg_ਅੰਕੜੇ,          /* Address of array containing child table row */
  ਪੂਰਨ nIncr,            /* Increment constraint counter by this */
  ਪੂਰਨ isIgnore          /* If true, pretend pTab contains all NULL values */
){
  ਪੂਰਨ i;                                    /* Iterator variable */
  Vdbe *v = sqlite3GetVdbe(pParse);         /* Vdbe to add code to */
  ਪੂਰਨ iCur = pParse->nTab - 1;              /* Cursor number to use */
  ਪੂਰਨ iOk = sqlite3VdbeMakeLabel(pParse);   /* jump here if parent key found */

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
  ਜੇ( nIncr<0 ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_FkIfZero, pFKey->isDeferred, iOk);
    VdbeCoverage(v);
  }
  ਲਈ(i=0; i<pFKey->n_ਖਾਨਾ; i++){
    ਪੂਰਨ iReg = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pFKey->pFrom,ai_ਖਾਨਾ[i]) + reg_ਅੰਕੜੇ + 1;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, iReg, iOk); VdbeCoverage(v);
  }

  ਜੇ( isIgnore==0 ){
    ਜੇ( pIdx==0 ){
      /* If pIdx is NULL, then the parent key is the INTEGER PRIMARY KEY
      ** column of the parent table (table pTab).  */
      ਪੂਰਨ iMustBeInt;               /* Address of MustBeInt instruction */
      ਪੂਰਨ reg_ਆਰਜ਼ੀ = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
  
      /* Invoke MustBeInt to coerce the child key value to an integer (i.e. 
      ** apply the affinity of the parent key). If this fails, then there
      ** is no matching parent key. Before using MustBeInt, make a copy of
      ** the value. Otherwise, the value inserted into the child key column
      ** will have INTEGER affinity applied to it, which may not be correct.  */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_SCopy, 
        sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pFKey->pFrom,ai_ਖਾਨਾ[0])+1+reg_ਅੰਕੜੇ, reg_ਆਰਜ਼ੀ);
      iMustBeInt = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_MustBeInt, reg_ਆਰਜ਼ੀ, 0);
      VdbeCoverage(v);
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter.  */
      ਜੇ( pTab==pFKey->pFrom && nIncr==1 ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Eq, reg_ਅੰਕੜੇ, iOk, reg_ਆਰਜ਼ੀ); VdbeCoverage(v);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      }
  
      sqlite3_Open_ਸਾਰਣੀ(pParse, iCur, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NotExists, iCur, 0, reg_ਆਰਜ਼ੀ); VdbeCoverage(v);
      sqlite3VdbeGoto(v, iOk);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, sqlite3VdbeCurrentAddr(v)-2);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, iMustBeInt);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, reg_ਆਰਜ਼ੀ);
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ n_ਖਾਨਾ = pFKey->n_ਖਾਨਾ;
      ਪੂਰਨ reg_ਆਰਜ਼ੀ = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_ਖਾਨਾ);
  
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenRead, iCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pIdx);
      ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Copy, 
               sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pFKey->pFrom, ai_ਖਾਨਾ[i])+1+reg_ਅੰਕੜੇ,
               reg_ਆਰਜ਼ੀ+i);
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
      ਜੇ( pTab==pFKey->pFrom && nIncr==1 ){
        ਪੂਰਨ iJump = sqlite3VdbeCurrentAddr(v) + n_ਖਾਨਾ + 1;
        ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
          ਪੂਰਨ i_ਬੱਚਾ = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pFKey->pFrom,ai_ਖਾਨਾ[i])
                              +1+reg_ਅੰਕੜੇ;
          ਪੂਰਨ i_ਮਾਪਾ = 1+reg_ਅੰਕੜੇ;
          i_ਮਾਪਾ += sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pIdx->p_ਸਾਰਣੀ,
                                                 pIdx->ai_ਕਾਲਮ[i]);
          ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->ai_ਕਾਲਮ[i]>=0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( ai_ਖਾਨਾ[i]!=pTab->iPKey );
          ਜੇ( pIdx->ai_ਕਾਲਮ[i]==pTab->iPKey ){
            /* The parent key is a composite key that includes the IPK column */
            i_ਮਾਪਾ = reg_ਅੰਕੜੇ;
          }
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Ne, i_ਬੱਚਾ, iJump, i_ਮਾਪਾ); VdbeCoverage(v);
          sqlite3VdbeChangeP5(v, SQLITE_JUMPIFNULL);
        }
        sqlite3VdbeGoto(v, iOk);
      }

      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, reg_ਆਰਜ਼ੀ, n_ਖਾਨਾ, 0,
                        sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(pParse->db,pIdx), n_ਖਾਨਾ);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, iCur, iOk, reg_ਆਰਜ਼ੀ, n_ਖਾਨਾ);
      VdbeCoverage(v);
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, reg_ਆਰਜ਼ੀ, n_ਖਾਨਾ);
    }
  }

  ਜੇ( !pFKey->isDeferred && !(pParse->db->flags & SQLITE_DeferFKs)
   && !pParse->pToplevel 
   && !pParse->isMultiWrite 
  ){
    /* Special case: If this is an INSERT statement that will insert exactly
    ** one row into the table, raise a constraint immediately instead of
    ** incrementing a counter. This is necessary as the VM code is being
    ** generated for will not open a statement transaction.  */
    ਪ੍ਰਤਿਜ੍ਞਾ( nIncr==1 );
    sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
        OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
  }ਨਹੀਂਤਾਂ{
    ਜੇ( nIncr>0 && pFKey->isDeferred==0 ){
      sqlite3MayAbort(pParse);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
  }

  sqlite3VdbeResolveLabel(v, iOk);
  sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Close, iCur);
}


/*
** Return an Expr object that refers to a memory register corresponding
** to column iCol of table pTab.
**
** regBase is the first of an array of register that contains the data
** for pTab.  regBase itself holds the rowid.  regBase+1 holds the first
** column.  regBase+2 holds the second column, and so forth.
*/
ਸਥਿਰ ਵਿਅੰਜਕ *ਵਿਅੰਜਕ_ਸਾਰਣੀ_ਰਜਿਸਟਰ(
  Parse *pParse,     /* Parsing and code generating context */
  ਸਾਰਣੀ *pTab,       /* The table whose content is at r[regBase]... */
  ਪੂਰਨ reg_ਅਧਾਰ,       /* Contents of table pTab */
  i16 i_ਖਾਨਾ           /* Which column of pTab is desired */
){
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;
  ਕਾਲਮ *p_ਖਾਨਾ;
  ਅਚਲ ਅੱਖਰ *zColl;
  sqlite3 *db = pParse->db;

  p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ(db, TK_ਰਜਿਸਟਰ, 0);
  ਜੇ( p_ਵਿਅੰਜਕ ){
    ਜੇ( i_ਖਾਨਾ>=0 && i_ਖਾਨਾ!=pTab->iPKey ){
      p_ਖਾਨਾ = &pTab->a_ਖਾਨਾ[i_ਖਾਨਾ];
      p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = reg_ਅਧਾਰ + sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab,i_ਖਾਨਾ) + 1;
      p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ = p_ਖਾਨਾ->affinity;
      zColl = sqlite3_ਕਾਲਮ_Coll(p_ਖਾਨਾ);
      ਜੇ( zColl==0 ) zColl = db->pDfltColl->zName;
      p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(pParse, p_ਵਿਅੰਜਕ, zColl);
    }ਨਹੀਂਤਾਂ{
      p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = reg_ਅਧਾਰ;
      p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ = SQLITE_AFF_INTEGER;
    }
  }
  ਵਾਪਸ p_ਵਿਅੰਜਕ;
}

/*
** Return an Expr object that refers to column iCol of table pTab which
** has cursor iCur.
*/
ਸਥਿਰ ਵਿਅੰਜਕ *ਵਿਅੰਜਕ_ਸਾਰਣੀ_ਕਾਲਮ(
  sqlite3 *db,      /* The database connection */
  ਸਾਰਣੀ *pTab,      /* The table whose column is desired */
  ਪੂਰਨ iCursor,      /* The open cursor on the table */
  i16 i_ਖਾਨਾ          /* The column that is wanted */
){
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ(db, TK_ਕਾਲਮ, 0);
  ਜੇ( p_ਵਿਅੰਜਕ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
    p_ਵਿਅੰਜਕ->y.pTab = pTab;
    p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = iCursor;
    p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = i_ਖਾਨਾ;
  }
  ਵਾਪਸ p_ਵਿਅੰਜਕ;
}

/*
** Column iCol1 of table p1 is to be compared to column iCol2 of table p2.
** Return non-zero if the comparison will use an affinity compatible
** with the affinity of column iCol1 of table p1. Or zero if it will not.
**
** This is used in cases where (p1.iCol1 = p2.iCol2) must be evaluated
** using the affinity of p1.iCol1 as part of foreign key processing.
*/
ਸਥਿਰ ਪੂਰਨ fk_ਵਰਤਦਾ_Lhs_Affinity(ਸਾਰਣੀ *p1, ਪੂਰਨ iCol1, ਸਾਰਣੀ *p2, ਪੂਰਨ iCol2){
  u8 aff1 = iCol1>=0 ? p1->a_ਖਾਨਾ[iCol1].affinity : SQLITE_AFF_NUMERIC;
  u8 aff2 = p2->a_ਖਾਨਾ[iCol2].affinity;

  /* If either side has a numeric affinity, then the comparison uses numeric
  ** affinity. So the affinity matches if aff1 is numeric, but not if aff1
  ** is not numeric but aff2 is.  */
  ਜੇ( sqlite3IsNumericAffinity(aff1) ) ਵਾਪਸ 1;
  ਜੇ( sqlite3IsNumericAffinity(aff2) ) ਵਾਪਸ 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( aff1==SQLITE_AFF_ਪਾਠ || aff1==SQLITE_AFF_BLOB );
  ਪ੍ਰਤਿਜ੍ਞਾ( aff2==SQLITE_AFF_ਪਾਠ || aff2==SQLITE_AFF_BLOB );

  /* At this point, either the affinities are the same or one is TEXT and
  ** the other BLOB. In the second case the comparison will be done with
  ** BLOB affinity, which does not match if aff1 is TEXT.  */
  ਵਾਪਸ (aff1==aff2 || aff1==SQLITE_AFF_BLOB);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CAST
/*
** Return non-zero if a lookup on column iCol of table pTab, using collation
** sequence zColl, can be done using either the rowid or an index. That is,
** if iCol is the INTEGER PRIMARY KEY of pTab, or if pTab has an index
** whose left-most column is iCol and whose collation sequence for that
** column is zColl. Return zero otherwise.
*/
ਸਥਿਰ ਪੂਰਨ fk_ਬੱਚਾ_ਕੁੰਜੀ_Is_ਸੂਚਕਾਂਕਿਤ(ਸਾਰਣੀ *pTab, ਪੂਰਨ i_ਖਾਨਾ, ਅਚਲ ਅੱਖਰ *zColl){
  ਸੂਚਕਾਂਕ *pIdx;
  ਜੇ( i_ਖਾਨਾ==pTab->iPKey ) ਵਾਪਸ 1;
  ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
    ਜੇ( pIdx->ai_ਕਾਲਮ[0]==i_ਖਾਨਾ
     && pIdx->pPartIdxWhere==0
     && sqlite3StrICmp(pIdx->azColl[0], zColl)==0
    ){
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਵਿਅੰਜਕ *fk_ਮਾਪਾ_ਬੱਚਾ_Eq(
  Parse *pParse,            /* Parsing context */
  ਵਿਅੰਜਕ *p_ਖੱਬਾ,              /* Parent key value */
  ਵਿਅੰਜਕ *p_ਸੱਜਾ,             /* Unresolved reference to child key column */
  ਸਾਰਣੀ *pTab,              /* Parent table */
  ਪੂਰਨ i_ਖਾਨਾ,                 /* Parent key column, or <0 for rowid */
  ਸਾਰਣੀ *p_ਬੱਚਾ,            /* Child table */
  ਪੂਰਨ i_ਬੱਚਾ_ਖਾਨਾ,            /* Child key column */
  ਪੂਰਨ b_ਸੰਯੁਕਤ            /* This is part of a composite FK */
){
  sqlite3 *db = pParse->db;
  ਵਿਅੰਜਕ *pEq;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CAST
  ਵਿਅੰਜਕ *p_ਵਿੱਚ = 0;
#ਜੇ_ਅੰਤ

  ਜੇ( fk_ਵਰਤਦਾ_Lhs_Affinity(pTab, i_ਖਾਨਾ, p_ਬੱਚਾ, i_ਬੱਚਾ_ਖਾਨਾ) ){
    ਵਾਪਸ sqlite3PExpr(pParse, TK_EQ, p_ਖੱਬਾ, p_ਸੱਜਾ);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=0 );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CAST
  {
    ਅਚਲ ਅੱਖਰ *zColl = sqlite3_ਕਾਲਮ_Coll(&pTab->a_ਖਾਨਾ[i_ਖਾਨਾ]);
    ਜੇ( zColl==0 ) zColl = sqlite3_Str_ਦੁਆਧਾਰੀ;

    /* The IN term is only useful if it can drive a lookup on the child
    ** table. Omit it otherwise, so that a full scan of the child table
    ** does not have to evaluate it for every row.  */
    ਜੇ( b_ਸੰਯੁਕਤ || fk_ਬੱਚਾ_ਕੁੰਜੀ_Is_ਸੂਚਕਾਂਕਿਤ(p_ਬੱਚਾ, i_ਬੱਚਾ_ਖਾਨਾ, zColl) ){
      ਸਥਿਰ ਅਚਲ Token tNumeric = { "NUMERIC", 7 };
      ਵਿਅੰਜਕ *pCast;
      ਵਿਅੰਜਕ_List *pList;

      pCast = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_CAST, &tNumeric, 0);
      sqlite3_ਵਿਅੰਜਕ_Attach_Subtrees(db, pCast, sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਖੱਬਾ, 0), 0);
      pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0,
          sqlite3PExpr(pParse, TK_UPLUS, sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਖੱਬਾ, 0), 0)
      );
      pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pList,
          sqlite3PExpr(pParse, TK_UPLUS, pCast, 0)
      );
      p_ਵਿੱਚ = sqlite3PExpr(pParse, TK_ਵਿੱਚ, sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(pParse,
            sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਸੱਜਾ, 0), zColl
      ), 0);
      ਜੇ( p_ਵਿੱਚ ){
        p_ਵਿੱਚ->x.pList = pList;
        sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ_And_Flags(pParse, p_ਵਿੱਚ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pList);
      }
    }
  }
#ਜੇ_ਅੰਤ

  p_ਸੱਜਾ = sqlite3PExpr(pParse, TK_UPLUS, p_ਸੱਜਾ, 0);
  pEq = sqlite3PExpr(pParse, TK_EQ, p_ਖੱਬਾ, p_ਸੱਜਾ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CAST
  pEq = sqlite3_ਵਿਅੰਜਕ_And(pParse, pEq, p_ਵਿੱਚ);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ pEq;
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
ਸਥਿਰ ਖਾਲੀ fk_ਜਾਂਚ_Children(
  Parse *pParse,                  /* Parse context */
  SrcList *pSrc,                  /* The child table to be scanned */
  ਸਾਰਣੀ *pTab,                    /* The parent table */
  ਸੂਚਕਾਂਕ *pIdx,                    /* Index on parent covering the foreign key */
  FKey *pFKey,                    /* The foreign key linking pSrc to pTab */
  ਪੂਰਨ *ai_ਖਾਨਾ,                     /* Map from pIdx cols to child table cols */
  ਪੂਰਨ reg_ਅੰਕੜੇ,                    /* Parent row data starts here */
  ਪੂਰਨ nIncr                       /* Amount to increment deferred counter by */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  ਪੂਰਨ i;                          /* Iterator variable */
  ਵਿਅੰਜਕ *pWhere = 0;               /* WHERE clause to scan with */
  NameContext sNameContext;       /* Context used to resolve WHERE clause */
  WhereInfo *pWInfo;              /* Context used by sqlite3WhereXXX() */
  ਪੂਰਨ iFkIfZero = 0;              /* Address of OP_FkIfZero */
  Vdbe *v = sqlite3GetVdbe(pParse);

  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx==0 || pIdx->p_ਸਾਰਣੀ==pTab );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx==0 || pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ==pFKey->n_ਖਾਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 || pFKey->n_ਖਾਨਾ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 || HasRowid(pTab) );

  ਜੇ( nIncr<0 ){
    iFkIfZero = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_FkIfZero, pFKey->isDeferred, 0);
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
  ਲਈ(i=0; i<pFKey->n_ਖਾਨਾ; i++){
    ਵਿਅੰਜਕ *p_ਖੱਬਾ;                  /* Value from parent table row */
    ਵਿਅੰਜਕ *p_ਸੱਜਾ;                 /* Column ref to child table */
    ਵਿਅੰਜਕ *pEq;                    /* Expression (pLeft = pRight) */
    i16 i_ਖਾਨਾ;                     /* Index of column in parent table */
    i16 iCol2;                    /* Index of column in child table */
    ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ;             /* Name of column in child table */

    i_ਖਾਨਾ = pIdx ? pIdx->ai_ਕਾਲਮ[i] : -1;
    p_ਖੱਬਾ = ਵਿਅੰਜਕ_ਸਾਰਣੀ_ਰਜਿਸਟਰ(pParse, pTab, reg_ਅੰਕੜੇ, i_ਖਾਨਾ);
    iCol2 = ai_ਖਾਨਾ ? ai_ਖਾਨਾ[i] : pFKey->a_ਖਾਨਾ[0].iFrom;
    ਪ੍ਰਤਿਜ੍ਞਾ( iCol2>=0 );
    z_ਖਾਨਾ = pFKey->pFrom->a_ਖਾਨਾ[iCol2].zCnName;
    p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, z_ਖਾਨਾ);
    pEq = fk_ਮਾਪਾ_ਬੱਚਾ_Eq(pParse, p_ਖੱਬਾ, p_ਸੱਜਾ, pTab, i_ਖਾਨਾ,
                          pFKey->pFrom, iCol2, pFKey->n_ਖਾਨਾ>0);
    pWhere = sqlite3_ਵਿਅੰਜਕ_And(pParse, pWhere, pEq);
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
  ਜੇ( pTab==pFKey->pFrom && nIncr>0 ){
    ਵਿਅੰਜਕ *pNe;                    /* Expression (pLeft != pRight) */
    ਵਿਅੰਜਕ *p_ਖੱਬਾ;                  /* Value from parent table row */
    ਵਿਅੰਜਕ *p_ਸੱਜਾ;                 /* Column ref to child table */
    ਜੇ( HasRowid(pTab) ){
      p_ਖੱਬਾ = ਵਿਅੰਜਕ_ਸਾਰਣੀ_ਰਜਿਸਟਰ(pParse, pTab, reg_ਅੰਕੜੇ, -1);
      p_ਸੱਜਾ = ਵਿਅੰਜਕ_ਸਾਰਣੀ_ਕਾਲਮ(db, pTab, pSrc->a[0].iCursor, -1);
      pNe = sqlite3PExpr(pParse, TK_NE, p_ਖੱਬਾ, p_ਸੱਜਾ);
    }ਨਹੀਂਤਾਂ{
      ਵਿਅੰਜਕ *pEq, *pAll = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 );
      ਲਈ(i=0; i<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
        i16 i_ਖਾਨਾ = pIdx->ai_ਕਾਲਮ[i];
        ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=0 );
        p_ਖੱਬਾ = ਵਿਅੰਜਕ_ਸਾਰਣੀ_ਰਜਿਸਟਰ(pParse, pTab, reg_ਅੰਕੜੇ, i_ਖਾਨਾ);
        p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName);
        pEq = sqlite3PExpr(pParse, TK_IS, p_ਖੱਬਾ, p_ਸੱਜਾ);
        pAll = sqlite3_ਵਿਅੰਜਕ_And(pParse, pAll, pEq);
      }
      pNe = sqlite3PExpr(pParse, TK_NOT, pAll, 0);
    }
    pWhere = sqlite3_ਵਿਅੰਜਕ_And(pParse, pWhere, pNe);
  }

  /* Resolve the references in the WHERE clause. */
  memset(&sNameContext, 0, ਆਕਾਰ(NameContext));
  sNameContext.pSrcList = pSrc;
  sNameContext.pParse = pParse;
  sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNameContext, pWhere);

  /* Create VDBE to loop through the entries in pSrc that match the WHERE
  ** clause. For each row found, increment either the deferred or immediate
  ** foreign key constraint counter. */
  ਜੇ( pParse->nErr==0 ){
    pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0, 0);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
    ਜੇ( pWInfo ){
      sqlite3_Where_ਸਿਰਾ(pWInfo);
    }
  }

  /* Clean up the WHERE clause constructed above. */
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhere);
  ਜੇ( iFkIfZero ){
    sqlite3_Vdbe_Jump_ਇੱਥੇ_Or_ਕੱਢੋ_Inst(v, iFkIfZero);
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
FKey *sqlite3FkReferences(ਸਾਰਣੀ *pTab){
  ਵਾਪਸ (FKey *)sqlite3_Hash_ਲੱਭੋ(&pTab->pSchema->fkeyHash, pTab->zName);
}

/*
** The second argument is a Trigger structure allocated by the 
** fkActionTrigger() routine. This function deletes the Trigger structure
** and all of its sub-components.
**
** The Trigger structure or any of its sub-components may be allocated from
** the lookaside buffer belonging to database handle dbMem.
*/
ਸਥਿਰ ਖਾਲੀ fkTriggerDelete(sqlite3 *dbMem, Trigger *p){
  ਜੇ( p ){
    Trigger_ਕਦਮ *p_ਕਦਮ = p->ਕਦਮ_list;
    sqlite3SrcListDelete(dbMem, p_ਕਦਮ->pSrc);
    sqlite3_ਵਿਅੰਜਕ_Delete(dbMem, p_ਕਦਮ->pWhere);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(dbMem, p_ਕਦਮ->p_ਵਿਅੰਜਕ_List);
    sqlite3SelectDelete(dbMem, p_ਕਦਮ->pSelect);
    sqlite3_ਵਿਅੰਜਕ_Delete(dbMem, p->pWhen);
    sqlite3DbFree(dbMem, p);
  }
}

/*
** Clear the apTrigger[] cache of CASCADE triggers for all foreign keys
** in a particular database.  This needs to happen when the schema
** changes.
*/
ਖਾਲੀ sqlite3FkClearTriggerCache(sqlite3 *db, ਪੂਰਨ iDb){
  HashElem *k;
  Hash *pHash = &db->aDb[iDb].pSchema->tblHash;
  ਲਈ(k=sqlite_Hash_ਪਹਿਲਾ(pHash); k; k=sqlite_Hash_ਅਗਲਾ(k)){
    ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(k);
    FKey *pFKey;
    ਜੇ( !Is_Ordinary_ਸਾਰਣੀ(pTab) ) ਜਾਰੀ;
    ਲਈ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_ਅਗਲਾ_From){
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
ਖਾਲੀ sqlite3_Fk_Drop_ਸਾਰਣੀ(Parse *pParse, SrcList *pName, ਸਾਰਣੀ *pTab){
  sqlite3 *db = pParse->db;
  ਜੇ( (db->flags&SQLITE_Foreign_ਕੁੰਜੀਆਂ) && Is_Ordinary_ਸਾਰਣੀ(pTab) ){
    ਪੂਰਨ iSkip = 0;
    Vdbe *v = sqlite3GetVdbe(pParse);

    ਪ੍ਰਤਿਜ੍ਞਾ( v );                  /* VDBE has already been allocated */
    ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
    ਜੇ( sqlite3FkReferences(pTab)==0 ){
      /* Search for a deferred foreign key constraint for which this table
      ** is the child table. If one cannot be found, return without 
      ** generating any VDBE code. If one can be found, then jump over
      ** the entire DELETE if there are no outstanding deferred constraints
      ** when this statement is run.  */
      FKey *p;
      ਲਈ(p=pTab->u.tab.pFKey; p; p=p->p_ਅਗਲਾ_From){
        ਜੇ( p->isDeferred || (db->flags & SQLITE_DeferFKs) ) ਰੋਕੋ;
      }
      ਜੇ( !p ) ਵਾਪਸ;
      iSkip = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_FkIfZero, 1, iSkip); VdbeCoverage(v);
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
    ਜੇ( (db->flags & SQLITE_DeferFKs)==0 ){
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_FkIfZero, 0, sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
      sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
          OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
    }

    ਜੇ( iSkip ){
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
ਸਥਿਰ ਪੂਰਨ fk_ਬੱਚਾ_Is_Modified(
  ਸਾਰਣੀ *pTab,                    /* Table being updated */
  FKey *p,                        /* Foreign key for which pTab is the child */
  ਪੂਰਨ *aChange,                   /* Array indicating modified columns */
  ਪੂਰਨ bChngRowid                  /* True if rowid is modified by this update */
){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<p->n_ਖਾਨਾ; i++){
    ਪੂਰਨ i_ਬੱਚਾ_ਕੁੰਜੀ = p->a_ਖਾਨਾ[i].iFrom;
    ਜੇ( aChange[i_ਬੱਚਾ_ਕੁੰਜੀ]>=0 ) ਵਾਪਸ 1;
    ਜੇ( i_ਬੱਚਾ_ਕੁੰਜੀ==pTab->iPKey && bChngRowid ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ fk_ਮਾਪਾ_Is_Modified(
  ਸਾਰਣੀ *pTab, 
  FKey *p, 
  ਪੂਰਨ *aChange, 
  ਪੂਰਨ bChngRowid
){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<p->n_ਖਾਨਾ; i++){
    ਅੱਖਰ *z_ਕੁੰਜੀ = p->a_ਖਾਨਾ[i].z_ਖਾਨਾ;
    ਪੂਰਨ i_ਕੁੰਜੀ;
    ਲਈ(i_ਕੁੰਜੀ=0; i_ਕੁੰਜੀ<pTab->n_ਖਾਨਾ; i_ਕੁੰਜੀ++){
      ਜੇ( aChange[i_ਕੁੰਜੀ]>=0 || (i_ਕੁੰਜੀ==pTab->iPKey && bChngRowid) ){
        ਕਾਲਮ *p_ਖਾਨਾ = &pTab->a_ਖਾਨਾ[i_ਕੁੰਜੀ];
        ਜੇ( z_ਕੁੰਜੀ ){
          ਜੇ( 0==sqlite3StrICmp(p_ਖਾਨਾ->zCnName, z_ਕੁੰਜੀ) ) ਵਾਪਸ 1;
        }ਨਹੀਂਤਾਂ ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_PRIMKEY ){
          ਵਾਪਸ 1;
        }
      }
    }
  }
  ਵਾਪਸ 0;
}

/*
** Return true if the parser passed as the first argument is being
** used to code a trigger that is really a "SET NULL" action belonging
** to trigger pFKey.
*/
ਸਥਿਰ ਪੂਰਨ is_ਸੈੱਟ_Null_Action(Parse *pParse, FKey *pFKey){
  Trigger *p = pParse->pTrigCoded;
  ਜੇ( p ){
    ਜੇ( (p==pFKey->apTrigger[0] && pFKey->aAction[0]==OE_ਸੈੱਟ_Null)
     || (p==pFKey->apTrigger[1] && pFKey->aAction[1]==OE_ਸੈੱਟ_Null)
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( (pParse->db->flags & SQLITE_FkNoAction)==0 );
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
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
ਖਾਲੀ sqlite3FkCheck(
  Parse *pParse,                  /* Parse context */
  ਸਾਰਣੀ *pTab,                    /* Row is being deleted from this table */ 
  ਪੂਰਨ regOld,                     /* Previous row data is stored here */
  ਪੂਰਨ regNew,                     /* New row data is stored here */
  ਪੂਰਨ *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  ਪੂਰਨ bChngRowid                  /* True if rowid is UPDATEd */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  FKey *pFKey;                    /* Used to iterate through FKs */
  ਪੂਰਨ iDb;                        /* Index of database containing pTab */
  ਅਚਲ ਅੱਖਰ *zDb;                /* Name of database containing pTab */
  ਪੂਰਨ isIgnoreErrors = pParse->disableTriggers;

  /* Exactly one of regOld and regNew should be non-zero. */
  ਪ੍ਰਤਿਜ੍ਞਾ( (regOld==0)!=(regNew==0) );

  /* If foreign-keys are disabled, this function is a no-op. */
  ਜੇ( (db->flags&SQLITE_Foreign_ਕੁੰਜੀਆਂ)==0 ) ਵਾਪਸ;
  ਜੇ( !Is_Ordinary_ਸਾਰਣੀ(pTab) ) ਵਾਪਸ;

  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=00 && iDb<db->nDb );
  zDb = db->aDb[iDb].zDbSName;

  /* Loop through all the foreign key constraints for which pTab is the
  ** child table (the table that the foreign key definition is part of).  */
  ਲਈ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_ਅਗਲਾ_From){
    ਸਾਰਣੀ *p_ਮੰਜ਼ਿਲ;                   /* Parent table of foreign key pFKey */
    ਸੂਚਕਾਂਕ *pIdx = 0;              /* Index on key columns in pTo */
    ਪੂਰਨ *aiFree = 0;
    ਪੂਰਨ *ai_ਖਾਨਾ;
    ਪੂਰਨ i_ਖਾਨਾ;
    ਪੂਰਨ i;
    ਪੂਰਨ bIgnore = 0;

    ਜੇ( aChange 
     && sqlite3_stricmp(pTab->zName, pFKey->z_ਮੰਜ਼ਿਲ)!=0
     && fk_ਬੱਚਾ_Is_Modified(pTab, pFKey, aChange, bChngRowid)==0 
    ){
      ਜਾਰੀ;
    }

    /* Find the parent table of this foreign key. Also find a unique index 
    ** on the parent key columns in the parent table. If either of these 
    ** schema items cannot be located, set an error in pParse and return 
    ** early.  */
    ਜੇ( pParse->disableTriggers ){
      p_ਮੰਜ਼ਿਲ = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, pFKey->z_ਮੰਜ਼ਿਲ, zDb);
    }ਨਹੀਂਤਾਂ{
      p_ਮੰਜ਼ਿਲ = sqlite3_Locate_ਸਾਰਣੀ(pParse, 0, pFKey->z_ਮੰਜ਼ਿਲ, zDb);
    }
    ਜੇ( !p_ਮੰਜ਼ਿਲ || sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(pParse, p_ਮੰਜ਼ਿਲ, pFKey, &pIdx, &aiFree) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( isIgnoreErrors==0 || (regOld!=0 && regNew==0) );
      ਜੇ( !isIgnoreErrors || db->mallocFailed ) ਵਾਪਸ;
      ਜੇ( p_ਮੰਜ਼ਿਲ==0 ){
        /* If isIgnoreErrors is true, then a table is being dropped. In this
        ** case SQLite runs a "DELETE FROM xxx" on the table being dropped
        ** before actually dropping it in order to check FK constraints.
        ** If the parent table of an FK constraint on the current table is
        ** missing, behave as if it is empty. i.e. decrement the relevant
        ** FK counter for each row of the current table with non-NULL keys.
        */
        Vdbe *v = sqlite3GetVdbe(pParse);
        ਪੂਰਨ iJump = sqlite3VdbeCurrentAddr(v) + pFKey->n_ਖਾਨਾ + 1;
        ਲਈ(i=0; i<pFKey->n_ਖਾਨਾ; i++){
          ਪੂਰਨ i_From_ਖਾਨਾ, iReg;
          i_From_ਖਾਨਾ = pFKey->a_ਖਾਨਾ[i].iFrom;
          iReg = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pFKey->pFrom,i_From_ਖਾਨਾ) + regOld+1;
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, iReg, iJump); VdbeCoverage(v);
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_FkCounter, pFKey->isDeferred, -1);
      }
      ਜਾਰੀ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pFKey->n_ਖਾਨਾ==1 || (aiFree && pIdx) );

    ਜੇ( aiFree ){
      ai_ਖਾਨਾ = aiFree;
    }ਨਹੀਂਤਾਂ{
      i_ਖਾਨਾ = pFKey->a_ਖਾਨਾ[0].iFrom;
      ai_ਖਾਨਾ = &i_ਖਾਨਾ;
    }
    ਲਈ(i=0; i<pFKey->n_ਖਾਨਾ; i++){
      ਜੇ( ai_ਖਾਨਾ[i]==pTab->iPKey ){
        ai_ਖਾਨਾ[i] = -1;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx==0 || pIdx->ai_ਕਾਲਮ[i]>=0 );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
      /* Request permission to read the parent key columns. If the 
      ** authorization callback returns SQLITE_IGNORE, behave as if any
      ** values read from the parent table are NULL. */
      ਜੇ( db->xAuth ){
        ਪੂਰਨ rcauth;
        ਅੱਖਰ *z_ਖਾਨਾ = p_ਮੰਜ਼ਿਲ->a_ਖਾਨਾ[pIdx ? pIdx->ai_ਕਾਲਮ[i] : p_ਮੰਜ਼ਿਲ->iPKey].zCnName;
        rcauth = sqlite3_Auth_Read_ਖਾਨਾ(pParse, p_ਮੰਜ਼ਿਲ->zName, z_ਖਾਨਾ, iDb);
        bIgnore = (rcauth==SQLITE_IGNORE);
      }
#ਜੇ_ਅੰਤ
    }

    /* Take a shared-cache advisory read-lock on the parent table. Allocate 
    ** a cursor to use to search the unique index on the parent key columns 
    ** in the parent table.  */
    sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, p_ਮੰਜ਼ਿਲ->tnum, 0, p_ਮੰਜ਼ਿਲ->zName);
    pParse->nTab++;

    ਜੇ( regOld!=0 ){
      /* A row is being removed from the child table. Search for the parent.
      ** If the parent does not exist, removing the child row resolves an 
      ** outstanding foreign key constraint violation. */
      fk_Lookup_ਮਾਪਾ(pParse, iDb, p_ਮੰਜ਼ਿਲ, pIdx, pFKey, ai_ਖਾਨਾ, regOld, -1, bIgnore);
    }
    ਜੇ( regNew!=0 && !is_ਸੈੱਟ_Null_Action(pParse, pFKey) ){
      /* A row is being added to the child table. If a parent row cannot
      ** be found, adding the child row has violated the FK constraint. 
      **
      ** If this operation is being performed as part of a trigger program
      ** that is actually a "SET NULL" action belonging to this very 
      ** foreign key, then omit this scan altogether. As all child key
      ** values are guaranteed to be NULL, it is not possible for adding
      ** this row to cause an FK violation.  */
      fk_Lookup_ਮਾਪਾ(pParse, iDb, p_ਮੰਜ਼ਿਲ, pIdx, pFKey, ai_ਖਾਨਾ, regNew, +1, bIgnore);
    }

    sqlite3DbFree(db, aiFree);
  }

  /* Loop through all the foreign key constraints that refer to this table.
  ** (the "child" constraints) */
  ਲਈ(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ){
    ਸੂਚਕਾਂਕ *pIdx = 0;              /* Foreign key index for pFKey */
    SrcList *pSrc;
    ਪੂਰਨ *ai_ਖਾਨਾ = 0;

    ਜੇ( aChange && fk_ਮਾਪਾ_Is_Modified(pTab, pFKey, aChange, bChngRowid)==0 ){
      ਜਾਰੀ;
    }

    ਜੇ( !pFKey->isDeferred && !(db->flags & SQLITE_DeferFKs) 
     && !pParse->pToplevel && !pParse->isMultiWrite 
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( regOld==0 && regNew!=0 );
      /* Inserting a single row into a parent table cannot cause (or fix)
      ** an immediate foreign key violation. So do nothing in this case.  */
      ਜਾਰੀ;
    }

    ਜੇ( sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(pParse, pTab, pFKey, &pIdx, &ai_ਖਾਨਾ) ){
      ਜੇ( !isIgnoreErrors || db->mallocFailed ) ਵਾਪਸ;
      ਜਾਰੀ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( ai_ਖਾਨਾ || pFKey->n_ਖਾਨਾ==1 );

    /* Create a SrcList structure containing the child table.  We need the
    ** child table as a SrcList for sqlite3WhereBegin() */
    pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
    ਜੇ( pSrc ){
      Src_ਵਸਤੂ *p_ਵਸਤੂ = pSrc->a;
      p_ਵਸਤੂ->pSTab = pFKey->pFrom;
      p_ਵਸਤੂ->zName = pFKey->pFrom->zName;
      p_ਵਸਤੂ->pSTab->nTabRef++;
      p_ਵਸਤੂ->iCursor = pParse->nTab++;
  
      ਜੇ( regNew!=0 ){
        fk_ਜਾਂਚ_Children(pParse, pSrc, pTab, pIdx, pFKey, ai_ਖਾਨਾ, regNew, -1);
      }
      ਜੇ( regOld!=0 ){
        ਪੂਰਨ eAction = pFKey->aAction[aChange!=0];
        ਜੇ( (db->flags & SQLITE_FkNoAction) ) eAction = OE_None;

        fk_ਜਾਂਚ_Children(pParse, pSrc, pTab, pIdx, pFKey, ai_ਖਾਨਾ, regOld, 1);
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
        ਜੇ( !pFKey->isDeferred && eAction!=OE_Cascade && eAction!=OE_ਸੈੱਟ_Null ){
          sqlite3MayAbort(pParse);
        }
      }
      p_ਵਸਤੂ->zName = 0;
      sqlite3SrcListDelete(db, pSrc);
    }
    sqlite3DbFree(db, ai_ਖਾਨਾ);
  }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਕਾਲਮ_ਨਕਾਬ(x) (((x)>31) ? 0xffffffff : ((u32)1<<(x)))

/*
** This function is called before generating code to update or delete a 
** row contained in table pTab.
*/
u32 sqlite3FkOldmask(
  Parse *pParse,                  /* Parse context */
  ਸਾਰਣੀ *pTab                     /* Table being modified */
){
  u32 ਨਕਾਬ = 0;
  ਜੇ( pParse->db->flags&SQLITE_Foreign_ਕੁੰਜੀਆਂ && Is_Ordinary_ਸਾਰਣੀ(pTab) ){
    FKey *p;
    ਪੂਰਨ i;
    ਲਈ(p=pTab->u.tab.pFKey; p; p=p->p_ਅਗਲਾ_From){
      ਲਈ(i=0; i<p->n_ਖਾਨਾ; i++) ਨਕਾਬ |= ਕਾਲਮ_ਨਕਾਬ(p->a_ਖਾਨਾ[i].iFrom);
    }
    ਲਈ(p=sqlite3FkReferences(pTab); p; p=p->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ){
      ਸੂਚਕਾਂਕ *pIdx = 0;
      sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(pParse, pTab, p, &pIdx, 0);
      ਜੇ( pIdx ){
        ਲਈ(i=0; i<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
          ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->ai_ਕਾਲਮ[i]>=0 );
          ਨਕਾਬ |= ਕਾਲਮ_ਨਕਾਬ(pIdx->ai_ਕਾਲਮ[i]);
        }
      }
    }
  }
  ਵਾਪਸ ਨਕਾਬ;
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
ਪੂਰਨ sqlite3FkRequired(
  Parse *pParse,                  /* Parse context */
  ਸਾਰਣੀ *pTab,                    /* Table being modified */
  ਪੂਰਨ *aChange,                   /* Non-NULL for UPDATE operations */
  ਪੂਰਨ chngRowid                   /* True for UPDATE that affects rowid */
){
  ਪੂਰਨ eRet = 1;                   /* Value to return if bHaveFK is true */
  ਪੂਰਨ bHaveFK = 0;                /* If FK processing is required */
  ਜੇ( pParse->db->flags&SQLITE_Foreign_ਕੁੰਜੀਆਂ && Is_Ordinary_ਸਾਰਣੀ(pTab) ){
    ਜੇ( !aChange ){
      /* A DELETE operation. Foreign key processing is required if the 
      ** table in question is either the child or parent table for any 
      ** foreign key constraint.  */
      bHaveFK = (sqlite3FkReferences(pTab) || pTab->u.tab.pFKey);
    }ਨਹੀਂਤਾਂ{
      /* This is an UPDATE. Foreign key processing is only required if the
      ** operation modifies one or more child or parent key columns. */
      FKey *p;

      /* Check if any child key columns are being modified. */
      ਲਈ(p=pTab->u.tab.pFKey; p; p=p->p_ਅਗਲਾ_From){
        ਜੇ( fk_ਬੱਚਾ_Is_Modified(pTab, p, aChange, chngRowid) ){
          ਜੇ( 0==sqlite3_stricmp(pTab->zName, p->z_ਮੰਜ਼ਿਲ) ) eRet = 2;
          bHaveFK = 1;
        }
      }

      /* Check if any parent key columns are being modified. */
      ਲਈ(p=sqlite3FkReferences(pTab); p; p=p->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ){
        ਜੇ( fk_ਮਾਪਾ_Is_Modified(pTab, p, aChange, chngRowid) ){
          ਜੇ( (pParse->db->flags & SQLITE_FkNoAction)==0 
           && p->aAction[1]!=OE_None 
          ){
            ਵਾਪਸ 2;
          }
          bHaveFK = 1;
        }
      }
    }
  }
  ਵਾਪਸ bHaveFK ? eRet : 0;
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
ਸਥਿਰ Trigger *fkActionTrigger(
  Parse *pParse,                  /* Parse context */
  ਸਾਰਣੀ *pTab,                    /* Table being updated or deleted from */
  FKey *pFKey,                    /* Foreign key to get action for */
  ਵਿਅੰਜਕ_List *pChanges              /* Change-list for UPDATE, NULL for DELETE */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  ਪੂਰਨ action;                     /* One of OE_None, OE_Cascade etc. */
  Trigger *pTrigger;              /* Trigger definition to return */
  ਪੂਰਨ iAction = (pChanges!=0);    /* 1 for UPDATE, 0 for DELETE */

  action = pFKey->aAction[iAction];
  ਜੇ( (db->flags & SQLITE_FkNoAction) ) action = OE_None;
  ਜੇ( action==OE_Restrict && (db->flags & SQLITE_DeferFKs) ){
    ਵਾਪਸ 0;
  }
  pTrigger = pFKey->apTrigger[iAction];

  ਜੇ( action!=OE_None && !pTrigger ){
    ਅੱਖਰ ਅਚਲ *zFrom;            /* Name of child table */
    ਪੂਰਨ nFrom;                    /* Length in bytes of zFrom */
    ਸੂਚਕਾਂਕ *pIdx = 0;              /* Parent key index for this FK */
    ਪੂਰਨ *ai_ਖਾਨਾ = 0;               /* child table cols -> parent key cols */
    Trigger_ਕਦਮ *p_ਕਦਮ = 0;        /* First (only) step of trigger program */
    ਵਿਅੰਜਕ *pWhere = 0;             /* WHERE clause of trigger step */
    ਵਿਅੰਜਕ_List *pList = 0;          /* Changes list if ON UPDATE CASCADE */
    Select *pSelect = 0;          /* If RESTRICT, "SELECT RAISE(...)" */
    ਪੂਰਨ i;                        /* Iterator variable */
    ਵਿਅੰਜਕ *pWhen = 0;              /* WHEN clause for the trigger */

    ਜੇ( sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(pParse, pTab, pFKey, &pIdx, &ai_ਖਾਨਾ) ) ਵਾਪਸ 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( ai_ਖਾਨਾ || pFKey->n_ਖਾਨਾ==1 );

    ਲਈ(i=0; i<pFKey->n_ਖਾਨਾ; i++){
      Token tOld = { "old", 3 };  /* Literal "old" token */
      Token tNew = { "new", 3 };  /* Literal "new" token */
      Token t_From_ਖਾਨਾ;             /* Name of column in child table */
      Token t_ਮੰਜ਼ਿਲ_ਖਾਨਾ;               /* Name of column in parent table */
      ਪੂਰਨ i_From_ਖਾਨਾ;               /* Idx of column in child table */
      ਪੂਰਨ i_ਮੰਜ਼ਿਲ_ਖਾਨਾ;                 /* Idx of column in parent table */
      ਵਿਅੰਜਕ *pEq;                  /* OLD.tToCol = tFromCol */
      ਵਿਅੰਜਕ *p_ਖੱਬਾ;                /* OLD.tToCol */
      ਵਿਅੰਜਕ *p_ਸੱਜਾ;               /* tFromCol */

      i_From_ਖਾਨਾ = ai_ਖਾਨਾ ? ai_ਖਾਨਾ[i] : pFKey->a_ਖਾਨਾ[0].iFrom;
      ਪ੍ਰਤਿਜ੍ਞਾ( i_From_ਖਾਨਾ>=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 || (pTab->iPKey>=0 && pTab->iPKey<pTab->n_ਖਾਨਾ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx==0 || pIdx->ai_ਕਾਲਮ[i]>=0 );
      i_ਮੰਜ਼ਿਲ_ਖਾਨਾ = pIdx ? pIdx->ai_ਕਾਲਮ[i] : pTab->iPKey;
      sqlite3TokenInit(&t_ਮੰਜ਼ਿਲ_ਖਾਨਾ, pTab->a_ਖਾਨਾ[i_ਮੰਜ਼ਿਲ_ਖਾਨਾ].zCnName);
      sqlite3TokenInit(&t_From_ਖਾਨਾ, pFKey->pFrom->a_ਖਾਨਾ[i_From_ਖਾਨਾ].zCnName);

      p_ਖੱਬਾ = sqlite3PExpr(pParse, TK_DOT,
          sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &tOld, 0),
          sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &t_ਮੰਜ਼ਿਲ_ਖਾਨਾ, 0)
      );
      p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &t_From_ਖਾਨਾ, 0);

      /* OLD.* columns do not normally have an affinity. So set the affinity
      ** of OLD.tToCol (the parent key) explicitly here. */
      ਜੇ( p_ਖੱਬਾ ) p_ਖੱਬਾ->aff_ਵਿਅੰਜਕ = pTab->a_ਖਾਨਾ[i_ਮੰਜ਼ਿਲ_ਖਾਨਾ].affinity;

      /* Create the expression "OLD.tToCol = tFromCol". It is important
      ** that the "OLD.tToCol" term is on the LHS of the = operator, so
      ** that the collation sequence associated with the parent table is
      ** used for the comparison. */
      pEq = fk_ਮਾਪਾ_ਬੱਚਾ_Eq(pParse, p_ਖੱਬਾ, p_ਸੱਜਾ, pTab, i_ਮੰਜ਼ਿਲ_ਖਾਨਾ,
                            pFKey->pFrom, i_From_ਖਾਨਾ, pFKey->n_ਖਾਨਾ>1);
      pWhere = sqlite3_ਵਿਅੰਜਕ_And(pParse, pWhere, pEq);

      /* For ON UPDATE, construct the next term of the WHEN clause.
      ** The final WHEN clause will be like this:
      **
      **    WHEN NOT(old.col1 IS new.col1 AND ... AND old.colN IS new.colN)
      */
      ਜੇ( pChanges ){
        pEq = sqlite3PExpr(pParse, TK_IS,
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &tOld, 0),
              sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &t_ਮੰਜ਼ਿਲ_ਖਾਨਾ, 0)),
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &tNew, 0),
              sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &t_ਮੰਜ਼ਿਲ_ਖਾਨਾ, 0))
            );
        pWhen = sqlite3_ਵਿਅੰਜਕ_And(pParse, pWhen, pEq);
      }
  
      ਜੇ( action!=OE_Restrict && (action!=OE_Cascade || pChanges) ){
        ਵਿਅੰਜਕ *pNew;
        ਜੇ( action==OE_Cascade ){
          pNew = sqlite3PExpr(pParse, TK_DOT, 
            sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &tNew, 0),
            sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ID, &t_ਮੰਜ਼ਿਲ_ਖਾਨਾ, 0));
        }ਨਹੀਂਤਾਂ ਜੇ( action==OE_ਸੈੱਟ_Dflt ){
          ਕਾਲਮ *p_ਖਾਨਾ = pFKey->pFrom->a_ਖਾਨਾ + i_From_ਖਾਨਾ;
          ਵਿਅੰਜਕ *pDflt;
          ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED ){
            testcase( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_VIRTUAL );
            testcase( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_STORED );
            pDflt = 0;
          }ਨਹੀਂਤਾਂ{
            pDflt = sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ_Auth(pFKey->pFrom, p_ਖਾਨਾ, pParse);
          }
          ਜੇ( pDflt ){
            pNew = sqlite3_ਵਿਅੰਜਕ_Dup(db, pDflt, 0);
          }ਨਹੀਂਤਾਂ{
            pNew = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_NULL, 0, 0);
          }
        }ਨਹੀਂਤਾਂ{
          pNew = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_NULL, 0, 0);
        }
        pList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pList, pNew);
        sqlite3_ਵਿਅੰਜਕ_List_ਸੈੱਟ_Name(pParse, pList, &t_From_ਖਾਨਾ, 0);
      }
    }
    sqlite3DbFree(db, ai_ਖਾਨਾ);

    zFrom = pFKey->pFrom->zName;
    nFrom = sqlite3Strlen30(zFrom);

    ਜੇ( action==OE_Restrict ){
      SrcList *pSrc;
      ਵਿਅੰਜਕ *pRaise; 

      pRaise = sqlite3_ਵਿਅੰਜਕ(db, TK_STRING, "FOREIGN KEY constraint failed"),
      pRaise = sqlite3PExpr(pParse, TK_RAISE, pRaise, 0);
      ਜੇ( pRaise ){
        pRaise->aff_ਵਿਅੰਜਕ = OE_Abort;
      }
      pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      ਜੇ( pSrc ){
        Src_ਵਸਤੂ *p_ਵਸਤੂ = &pSrc->a[0];
        p_ਵਸਤੂ->zName = sqlite3DbStrDup(db, zFrom);
        p_ਵਸਤੂ->fg.fixedSchema = 1;
        p_ਵਸਤੂ->u4.pSchema = pTab->pSchema;
      }
      pSelect = sqlite3SelectNew(pParse, 
          sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0, pRaise),
          pSrc,
          pWhere,
          0, 0, 0, 0, 0
      );
      pWhere = 0;
    }

    /* Disable lookaside memory allocation */
    DisableLookaside;

    pTrigger = (Trigger *)sqlite3DbMallocZero(db, 
        ਆਕਾਰ(Trigger) +         /* struct Trigger */
        ਆਕਾਰ(Trigger_ਕਦਮ)       /* Single step in trigger program */
    );
    ਜੇ( pTrigger ){
      p_ਕਦਮ = pTrigger->ਕਦਮ_list = (Trigger_ਕਦਮ *)&pTrigger[1];
      p_ਕਦਮ->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      ਜੇ( p_ਕਦਮ->pSrc ){
        Src_ਵਸਤੂ *p_ਵਸਤੂ = &p_ਕਦਮ->pSrc->a[0];
        p_ਵਸਤੂ->zName = sqlite3DbStrNDup(db, zFrom, nFrom);
        p_ਵਸਤੂ->u4.pSchema = pTab->pSchema;
        p_ਵਸਤੂ->fg.fixedSchema = 1;
      }
      p_ਕਦਮ->pWhere = sqlite3_ਵਿਅੰਜਕ_Dup(db, pWhere, EXPRDUP_REDUCE);
      p_ਕਦਮ->p_ਵਿਅੰਜਕ_List = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, pList, EXPRDUP_REDUCE);
      p_ਕਦਮ->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
      ਜੇ( pWhen ){
        pWhen = sqlite3PExpr(pParse, TK_NOT, pWhen, 0);
        pTrigger->pWhen = sqlite3_ਵਿਅੰਜਕ_Dup(db, pWhen, EXPRDUP_REDUCE);
      }
    }

    /* Re-enable the lookaside buffer, if it was disabled earlier. */
    EnableLookaside;

    sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhere);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhen);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pList);
    sqlite3SelectDelete(db, pSelect);
    ਜੇ( db->mallocFailed==1 ){
      fkTriggerDelete(db, pTrigger);
      ਵਾਪਸ 0;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕਦਮ!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pTrigger!=0 );

    ਚੋਣ( action ){
      ਹਾਲਤ OE_Restrict:
        p_ਕਦਮ->op = TK_SELECT;
        ਰੋਕੋ;
      ਹਾਲਤ OE_Cascade: 
        ਜੇ( !pChanges ){ 
          p_ਕਦਮ->op = TK_DELETE; 
          ਰੋਕੋ; 
        }
        /* no break */ deliberate_fall_through
      ਮੂਲ:
        p_ਕਦਮ->op = TK_UPDATE;
    }
    p_ਕਦਮ->pTrig = pTrigger;
    pTrigger->pSchema = pTab->pSchema;
    pTrigger->pTabSchema = pTab->pSchema;
    pFKey->apTrigger[iAction] = pTrigger;
    pTrigger->op = (pChanges ? TK_UPDATE : TK_DELETE);
  }

  ਵਾਪਸ pTrigger;
}

/*
** This function is called when deleting or updating a row to implement
** any required CASCADE, SET NULL or SET DEFAULT actions.
*/
ਖਾਲੀ sqlite3FkActions(
  Parse *pParse,                  /* Parse context */
  ਸਾਰਣੀ *pTab,                    /* Table being updated or deleted from */
  ਵਿਅੰਜਕ_List *pChanges,             /* Change-list for UPDATE, NULL for DELETE */
  ਪੂਰਨ regOld,                     /* Address of array containing old row */
  ਪੂਰਨ *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  ਪੂਰਨ bChngRowid                  /* True if rowid is UPDATEd */
){
  /* If foreign-key support is enabled, iterate through all FKs that 
  ** refer to table pTab. If there is an action associated with the FK 
  ** for this operation (either update or delete), invoke the associated 
  ** trigger sub-program.  */
  ਜੇ( pParse->db->flags&SQLITE_Foreign_ਕੁੰਜੀਆਂ ){
    FKey *pFKey;                  /* Iterator variable */
    ਲਈ(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ){
      ਜੇ( aChange==0 || fk_ਮਾਪਾ_Is_Modified(pTab, pFKey, aChange, bChngRowid) ){
        Trigger *pAct = fkActionTrigger(pParse, pTab, pFKey, pChanges);
        ਜੇ( pAct ){
          sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger_ਸਿੱਧਾ(pParse, pAct, pTab, regOld, OE_Abort, 0);
        }
      }
    }
  }
}

#ਜੇ_ਅੰਤ /* ifndef SQLITE_OMIT_TRIGGER */

/*
** Free all memory associated with foreign key definitions attached to
** table pTab. Remove the deleted foreign keys from the Schema.fkeyHash
** hash table.
*/
ਖਾਲੀ sqlite3FkDelete(sqlite3 *db, ਸਾਰਣੀ *pTab){
  FKey *pFKey;                    /* Iterator variable */
  FKey *p_ਅਗਲਾ;                    /* Copy of pFKey->pNextFrom */

  ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਲਈ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=p_ਅਗਲਾ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db==0 || sqlite3SchemaMutexHeld(db, 0, pTab->pSchema) );

    /* Remove the FK from the fkeyHash hash table. */
    ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed==0 ){
      ਜੇ( pFKey->p_Prev_ਮੰਜ਼ਿਲ ){
        pFKey->p_Prev_ਮੰਜ਼ਿਲ->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ = pFKey->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ;
      }ਨਹੀਂਤਾਂ{
        ਅਚਲ ਅੱਖਰ *z = (pFKey->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ ? pFKey->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ->z_ਮੰਜ਼ਿਲ : pFKey->z_ਮੰਜ਼ਿਲ);
        sqlite3_Hash_ਪਾਓ(&pTab->pSchema->fkeyHash, z, pFKey->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ);
      }
      ਜੇ( pFKey->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ ){
        pFKey->p_ਅਗਲਾ_ਮੰਜ਼ਿਲ->p_Prev_ਮੰਜ਼ਿਲ = pFKey->p_Prev_ਮੰਜ਼ਿਲ;
      }
    }

    /* EV: R-30323-21917 Each foreign key constraint in SQLite is
    ** classified as either immediate or deferred.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( pFKey->isDeferred==0 || pFKey->isDeferred==1 );

    /* Delete any triggers created to implement actions for this FK. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
    fkTriggerDelete(db, pFKey->apTrigger[0]);
    fkTriggerDelete(db, pFKey->apTrigger[1]);
#ਜੇ_ਅੰਤ

    p_ਅਗਲਾ = pFKey->p_ਅਗਲਾ_From;
    sqlite3DbFree(db, pFKey);
  }
}
#ਜੇ_ਅੰਤ /* ifndef SQLITE_OMIT_FOREIGN_KEY */
