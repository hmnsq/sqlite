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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER

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
ಪೂರ್ಣಾಂಕ sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(
  Parse *pParse,                  /* Parse context to store any error in */
  ಕೋಷ್ಟಕ *p_ಪೋಷಕ,                 /* Parent table of FK constraint pFKey */
  FKey *pFKey,                    /* Foreign key to find index for */
  ಸೂಚ್ಯಂಕ **ppIdx,                  /* OUT: Unique index on parent table */
  ಪೂರ್ಣಾಂಕ **pai_ಲಂಬಸಾಲು                    /* OUT: Map of index columns in pFKey */
){
  ಸೂಚ್ಯಂಕ *pIdx = 0;                    /* Value to return via *ppIdx */
  ಪೂರ್ಣಾಂಕ *ai_ಲಂಬಸಾಲು = 0;                     /* Value to return via *paiCol */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = pFKey->n_ಲಂಬಸಾಲು;             /* Number of columns in parent key */
  ಅಕ್ಷರ *z_ಕೀಲಿ = pFKey->a_ಲಂಬಸಾಲು[0].z_ಲಂಬಸಾಲು;   /* Name of left-most parent key column */

  /* The caller is responsible for zeroing output parameters. */
  ಪ್ರತಿಜ್ಞಾ( ppIdx && *ppIdx==0 );
  ಪ್ರತಿಜ್ಞಾ( !pai_ಲಂಬಸಾಲು || *pai_ಲಂಬಸಾಲು==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse );

  /* If this is a non-composite (single column) foreign key, check if it 
  ** maps to the INTEGER PRIMARY KEY of table pParent. If so, leave *ppIdx 
  ** and *paiCol set to zero and return early. 
  **
  ** Otherwise, for a composite foreign key (more than one column), allocate
  ** space for the aiCol array (returned via output parameter *paiCol).
  ** Non-composite foreign keys do not require the aiCol array.
  */
  ಆದರೆ( n_ಲಂಬಸಾಲು==1 ){
    /* The FK maps to the IPK if any of the following are true:
    **
    **   1) There is an INTEGER PRIMARY KEY column and the FK is implicitly 
    **      mapped to the primary key of table pParent, or
    **   2) The FK is explicitly mapped to a column declared as INTEGER
    **      PRIMARY KEY.
    */
    ಆದರೆ( p_ಪೋಷಕ->iPKey>=0 ){
      ಆದರೆ( !z_ಕೀಲಿ ) ಹಿಂತಿರುಗಿ 0;
      ಆದರೆ( !sqlite3StrICmp(p_ಪೋಷಕ->a_ಲಂಬಸಾಲು[p_ಪೋಷಕ->iPKey].zCnName, z_ಕೀಲಿ) ){
        ಹಿಂತಿರುಗಿ 0;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pai_ಲಂಬಸಾಲು ){
    ಪ್ರತಿಜ್ಞಾ( n_ಲಂಬಸಾಲು>1 );
    ai_ಲಂಬಸಾಲು = (ಪೂರ್ಣಾಂಕ *)sqlite3DbMallocRawNN(pParse->db, n_ಲಂಬಸಾಲು*ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
    ಆದರೆ( !ai_ಲಂಬಸಾಲು ) ಹಿಂತಿರುಗಿ 1;
    *pai_ಲಂಬಸಾಲು = ai_ಲಂಬಸಾಲು;
  }

  ಗಾಗಿ(pIdx=p_ಪೋಷಕ->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
    ಆದರೆ( pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು==n_ಲಂಬಸಾಲು && Is_Unique_ಸೂಚ್ಯಂಕ(pIdx) && pIdx->pPartIdxWhere==0 ){ 
      /* pIdx is a UNIQUE index (or a PRIMARY KEY) and has the right number
      ** of columns. If each indexed column corresponds to a foreign key
      ** column of pFKey, then this index is a winner.  */

      ಆದರೆ( z_ಕೀಲಿ==0 ){
        /* If zKey is NULL, then this foreign key is implicitly mapped to 
        ** the PRIMARY KEY of table pParent. The PRIMARY KEY index may be 
        ** identified by the test.  */
        ಆದರೆ( Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) ){
          ಆದರೆ( ai_ಲಂಬಸಾಲು ){
            ಪೂರ್ಣಾಂಕ i;
            ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++) ai_ಲಂಬಸಾಲು[i] = pFKey->a_ಲಂಬಸಾಲು[i].iFrom;
          }
          ನಿಲ್ಲಿಸು;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* If zKey is non-NULL, then this foreign key was declared to
        ** map to an explicit list of columns in table pParent. Check if this
        ** index matches those columns. Also, check that the index uses
        ** the default collation sequences for each column. */
        ಪೂರ್ಣಾಂಕ i, j;
        ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
          i16 i_ಲಂಬಸಾಲು = pIdx->ai_ಸ್ತಂಭ[i];     /* Index of column in parent tbl */
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDfltColl;            /* Def. collation for column */
          ಅಕ್ಷರ *z_Idx_ಲಂಬಸಾಲು;                    /* Name of indexed column */

          ಆದರೆ( i_ಲಂಬಸಾಲು<0 ) ನಿಲ್ಲಿಸು; /* No foreign keys against expression indexes */

          /* If the index uses a collation sequence that is different from
          ** the default collation sequence for the column, this index is
          ** unusable. Bail out early in this case.  */
          zDfltColl = sqlite3_ಸ್ತಂಭ_Coll(&p_ಪೋಷಕ->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು]);
          ಆದರೆ( !zDfltColl ) zDfltColl = sqlite3_Str_ದ್ವಿಮಾನ;
          ಆದರೆ( sqlite3StrICmp(pIdx->azColl[i], zDfltColl) ) ನಿಲ್ಲಿಸು;

          z_Idx_ಲಂಬಸಾಲು = p_ಪೋಷಕ->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName;
          ಗಾಗಿ(j=0; j<n_ಲಂಬಸಾಲು; j++){
            ಆದರೆ( sqlite3StrICmp(pFKey->a_ಲಂಬಸಾಲು[j].z_ಲಂಬಸಾಲು, z_Idx_ಲಂಬಸಾಲು)==0 ){
              ಆದರೆ( ai_ಲಂಬಸಾಲು ) ai_ಲಂಬಸಾಲು[i] = pFKey->a_ಲಂಬಸಾಲು[j].iFrom;
              ನಿಲ್ಲಿಸು;
            }
          }
          ಆದರೆ( j==n_ಲಂಬಸಾಲು ) ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ( i==n_ಲಂಬಸಾಲು ) ನಿಲ್ಲಿಸು;      /* pIdx is usable */
      }
    }
  }

  ಆದರೆ( !pIdx ){
    ಆದರೆ( !pParse->disableTriggers ){
      sqlite3_Error_ಸಂದೇಶ(pParse,
           "foreign key mismatch - \"%w\" referencing \"%w\"",
           pFKey->pFrom->zName, pFKey->z_ಗಮ್ಯ);
    }
    sqlite3DbFree(pParse->db, ai_ಲಂಬಸಾಲು);
    ಹಿಂತಿರುಗಿ 1;
  }

  *ppIdx = pIdx;
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ fk_Lookup_ಪೋಷಕ(
  Parse *pParse,        /* Parse context */
  ಪೂರ್ಣಾಂಕ iDb,              /* Index of database housing pTab */
  ಕೋಷ್ಟಕ *pTab,          /* Parent table of FK pFKey */
  ಸೂಚ್ಯಂಕ *pIdx,          /* Unique index on parent key columns in pTab */
  FKey *pFKey,          /* Foreign key constraint */
  ಪೂರ್ಣಾಂಕ *ai_ಲಂಬಸಾಲು,           /* Map from parent key columns to child table columns */
  ಪೂರ್ಣಾಂಕ reg_ದತ್ತಾಂಶ,          /* Address of array containing child table row */
  ಪೂರ್ಣಾಂಕ nIncr,            /* Increment constraint counter by this */
  ಪೂರ್ಣಾಂಕ isIgnore          /* If true, pretend pTab contains all NULL values */
){
  ಪೂರ್ಣಾಂಕ i;                                    /* Iterator variable */
  Vdbe *v = sqlite3GetVdbe(pParse);         /* Vdbe to add code to */
  ಪೂರ್ಣಾಂಕ iCur = pParse->nTab - 1;              /* Cursor number to use */
  ಪೂರ್ಣಾಂಕ iOk = sqlite3VdbeMakeLabel(pParse);   /* jump here if parent key found */

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
  ಆದರೆ( nIncr<0 ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_FkIfZero, pFKey->isDeferred, iOk);
    VdbeCoverage(v);
  }
  ಗಾಗಿ(i=0; i<pFKey->n_ಲಂಬಸಾಲು; i++){
    ಪೂರ್ಣಾಂಕ iReg = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pFKey->pFrom,ai_ಲಂಬಸಾಲು[i]) + reg_ದತ್ತಾಂಶ + 1;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, iReg, iOk); VdbeCoverage(v);
  }

  ಆದರೆ( isIgnore==0 ){
    ಆದರೆ( pIdx==0 ){
      /* If pIdx is NULL, then the parent key is the INTEGER PRIMARY KEY
      ** column of the parent table (table pTab).  */
      ಪೂರ್ಣಾಂಕ iMustBeInt;               /* Address of MustBeInt instruction */
      ಪೂರ್ಣಾಂಕ reg_ತಾತ್ಕಾಲಿಕ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
  
      /* Invoke MustBeInt to coerce the child key value to an integer (i.e. 
      ** apply the affinity of the parent key). If this fails, then there
      ** is no matching parent key. Before using MustBeInt, make a copy of
      ** the value. Otherwise, the value inserted into the child key column
      ** will have INTEGER affinity applied to it, which may not be correct.  */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_SCopy, 
        sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pFKey->pFrom,ai_ಲಂಬಸಾಲು[0])+1+reg_ದತ್ತಾಂಶ, reg_ತಾತ್ಕಾಲಿಕ);
      iMustBeInt = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_MustBeInt, reg_ತಾತ್ಕಾಲಿಕ, 0);
      VdbeCoverage(v);
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter.  */
      ಆದರೆ( pTab==pFKey->pFrom && nIncr==1 ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Eq, reg_ದತ್ತಾಂಶ, iOk, reg_ತಾತ್ಕಾಲಿಕ); VdbeCoverage(v);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      }
  
      sqlite3_Open_ಕೋಷ್ಟಕ(pParse, iCur, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NotExists, iCur, 0, reg_ತಾತ್ಕಾಲಿಕ); VdbeCoverage(v);
      sqlite3VdbeGoto(v, iOk);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, sqlite3VdbeCurrentAddr(v)-2);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, iMustBeInt);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, reg_ತಾತ್ಕಾಲಿಕ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = pFKey->n_ಲಂಬಸಾಲು;
      ಪೂರ್ಣಾಂಕ reg_ತಾತ್ಕಾಲಿಕ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_ಲಂಬಸಾಲು);
  
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenRead, iCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pIdx);
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Copy, 
               sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pFKey->pFrom, ai_ಲಂಬಸಾಲು[i])+1+reg_ದತ್ತಾಂಶ,
               reg_ತಾತ್ಕಾಲಿಕ+i);
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
      ಆದರೆ( pTab==pFKey->pFrom && nIncr==1 ){
        ಪೂರ್ಣಾಂಕ iJump = sqlite3VdbeCurrentAddr(v) + n_ಲಂಬಸಾಲು + 1;
        ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
          ಪೂರ್ಣಾಂಕ i_ಮಗು = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pFKey->pFrom,ai_ಲಂಬಸಾಲು[i])
                              +1+reg_ದತ್ತಾಂಶ;
          ಪೂರ್ಣಾಂಕ i_ಪೋಷಕ = 1+reg_ದತ್ತಾಂಶ;
          i_ಪೋಷಕ += sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pIdx->p_ಕೋಷ್ಟಕ,
                                                 pIdx->ai_ಸ್ತಂಭ[i]);
          ಪ್ರತಿಜ್ಞಾ( pIdx->ai_ಸ್ತಂಭ[i]>=0 );
          ಪ್ರತಿಜ್ಞಾ( ai_ಲಂಬಸಾಲು[i]!=pTab->iPKey );
          ಆದರೆ( pIdx->ai_ಸ್ತಂಭ[i]==pTab->iPKey ){
            /* The parent key is a composite key that includes the IPK column */
            i_ಪೋಷಕ = reg_ದತ್ತಾಂಶ;
          }
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Ne, i_ಮಗು, iJump, i_ಪೋಷಕ); VdbeCoverage(v);
          sqlite3VdbeChangeP5(v, SQLITE_JUMPIFNULL);
        }
        sqlite3VdbeGoto(v, iOk);
      }

      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, reg_ತಾತ್ಕಾಲಿಕ, n_ಲಂಬಸಾಲು, 0,
                        sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Str(pParse->db,pIdx), n_ಲಂಬಸಾಲು);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, iCur, iOk, reg_ತಾತ್ಕಾಲಿಕ, n_ಲಂಬಸಾಲು);
      VdbeCoverage(v);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, reg_ತಾತ್ಕಾಲಿಕ, n_ಲಂಬಸಾಲು);
    }
  }

  ಆದರೆ( !pFKey->isDeferred && !(pParse->db->flags & SQLITE_DeferFKs)
   && !pParse->pToplevel 
   && !pParse->isMultiWrite 
  ){
    /* Special case: If this is an INSERT statement that will insert exactly
    ** one row into the table, raise a constraint immediately instead of
    ** incrementing a counter. This is necessary as the VM code is being
    ** generated for will not open a statement transaction.  */
    ಪ್ರತಿಜ್ಞಾ( nIncr==1 );
    sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
        OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( nIncr>0 && pFKey->isDeferred==0 ){
      sqlite3MayAbort(pParse);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
  }

  sqlite3VdbeResolveLabel(v, iOk);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Close, iCur);
}


/*
** Return an Expr object that refers to a memory register corresponding
** to column iCol of table pTab.
**
** regBase is the first of an array of register that contains the data
** for pTab.  regBase itself holds the rowid.  regBase+1 holds the first
** column.  regBase+2 holds the second column, and so forth.
*/
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ *ಅಭಿವ್ಯಕ್ತಿ_ಕೋಷ್ಟಕ_ರಿಜಿಸ್ಟರ್(
  Parse *pParse,     /* Parsing and code generating context */
  ಕೋಷ್ಟಕ *pTab,       /* The table whose content is at r[regBase]... */
  ಪೂರ್ಣಾಂಕ reg_ತಳ,       /* Contents of table pTab */
  i16 i_ಲಂಬಸಾಲು           /* Which column of pTab is desired */
){
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;
  ಸ್ತಂಭ *p_ಲಂಬಸಾಲು;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zColl;
  sqlite3 *db = pParse->db;

  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ರಿಜಿಸ್ಟರ್, 0);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ ){
    ಆದರೆ( i_ಲಂಬಸಾಲು>=0 && i_ಲಂಬಸಾಲು!=pTab->iPKey ){
      p_ಲಂಬಸಾಲು = &pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು];
      p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = reg_ತಳ + sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab,i_ಲಂಬಸಾಲು) + 1;
      p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = p_ಲಂಬಸಾಲು->affinity;
      zColl = sqlite3_ಸ್ತಂಭ_Coll(p_ಲಂಬಸಾಲು);
      ಆದರೆ( zColl==0 ) zColl = db->pDfltColl->zName;
      p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, zColl);
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = reg_ತಳ;
      p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_INTEGER;
    }
  }
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ;
}

/*
** Return an Expr object that refers to column iCol of table pTab which
** has cursor iCur.
*/
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ *ಅಭಿವ್ಯಕ್ತಿ_ಕೋಷ್ಟಕ_ಸ್ತಂಭ(
  sqlite3 *db,      /* The database connection */
  ಕೋಷ್ಟಕ *pTab,      /* The table whose column is desired */
  ಪೂರ್ಣಾಂಕ iCursor,      /* The open cursor on the table */
  i16 i_ಲಂಬಸಾಲು          /* The column that is wanted */
){
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ಸ್ತಂಭ, 0);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
    p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = pTab;
    p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = iCursor;
    p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = i_ಲಂಬಸಾಲು;
  }
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ;
}

/*
** Column iCol1 of table p1 is to be compared to column iCol2 of table p2.
** Return non-zero if the comparison will use an affinity compatible
** with the affinity of column iCol1 of table p1. Or zero if it will not.
**
** This is used in cases where (p1.iCol1 = p2.iCol2) must be evaluated
** using the affinity of p1.iCol1 as part of foreign key processing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fk_ಬಳಸುತ್ತದೆ_Lhs_Affinity(ಕೋಷ್ಟಕ *p1, ಪೂರ್ಣಾಂಕ iCol1, ಕೋಷ್ಟಕ *p2, ಪೂರ್ಣಾಂಕ iCol2){
  u8 aff1 = iCol1>=0 ? p1->a_ಲಂಬಸಾಲು[iCol1].affinity : SQLITE_AFF_NUMERIC;
  u8 aff2 = p2->a_ಲಂಬಸಾಲು[iCol2].affinity;

  /* If either side has a numeric affinity, then the comparison uses numeric
  ** affinity. So the affinity matches if aff1 is numeric, but not if aff1
  ** is not numeric but aff2 is.  */
  ಆದರೆ( sqlite3IsNumericAffinity(aff1) ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( sqlite3IsNumericAffinity(aff2) ) ಹಿಂತಿರುಗಿ 0;

  ಪ್ರತಿಜ್ಞಾ( aff1==SQLITE_AFF_ಪಠ್ಯ || aff1==SQLITE_AFF_BLOB );
  ಪ್ರತಿಜ್ಞಾ( aff2==SQLITE_AFF_ಪಠ್ಯ || aff2==SQLITE_AFF_BLOB );

  /* At this point, either the affinities are the same or one is TEXT and
  ** the other BLOB. In the second case the comparison will be done with
  ** BLOB affinity, which does not match if aff1 is TEXT.  */
  ಹಿಂತಿರುಗಿ (aff1==aff2 || aff1==SQLITE_AFF_BLOB);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CAST
/*
** Return non-zero if a lookup on column iCol of table pTab, using collation
** sequence zColl, can be done using either the rowid or an index. That is,
** if iCol is the INTEGER PRIMARY KEY of pTab, or if pTab has an index
** whose left-most column is iCol and whose collation sequence for that
** column is zColl. Return zero otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fk_ಮಗು_ಕೀಲಿ_Is_ಸೂಚ್ಯಂಕಿತ(ಕೋಷ್ಟಕ *pTab, ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zColl){
  ಸೂಚ್ಯಂಕ *pIdx;
  ಆದರೆ( i_ಲಂಬಸಾಲು==pTab->iPKey ) ಹಿಂತಿರುಗಿ 1;
  ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
    ಆದರೆ( pIdx->ai_ಸ್ತಂಭ[0]==i_ಲಂಬಸಾಲು
     && pIdx->pPartIdxWhere==0
     && sqlite3StrICmp(pIdx->azColl[0], zColl)==0
    ){
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ *fk_ಪೋಷಕ_ಮಗು_Eq(
  Parse *pParse,            /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ,              /* Parent key value */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ,             /* Unresolved reference to child key column */
  ಕೋಷ್ಟಕ *pTab,              /* Parent table */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು,                 /* Parent key column, or <0 for rowid */
  ಕೋಷ್ಟಕ *p_ಮಗು,            /* Child table */
  ಪೂರ್ಣಾಂಕ i_ಮಗು_ಲಂಬಸಾಲು,            /* Child key column */
  ಪೂರ್ಣಾಂಕ b_ಸಂಯುಕ್ತ            /* This is part of a composite FK */
){
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ *pEq;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CAST
  ಅಭಿವ್ಯಕ್ತಿ *p_ಒಳ = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( fk_ಬಳಸುತ್ತದೆ_Lhs_Affinity(pTab, i_ಲಂಬಸಾಲು, p_ಮಗು, i_ಮಗು_ಲಂಬಸಾಲು) ){
    ಹಿಂತಿರುಗಿ sqlite3PExpr(pParse, TK_EQ, p_ಎಡ, p_ಬಲ);
  }
  ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು>=0 );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CAST
  {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zColl = sqlite3_ಸ್ತಂಭ_Coll(&pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು]);
    ಆದರೆ( zColl==0 ) zColl = sqlite3_Str_ದ್ವಿಮಾನ;

    /* The IN term is only useful if it can drive a lookup on the child
    ** table. Omit it otherwise, so that a full scan of the child table
    ** does not have to evaluate it for every row.  */
    ಆದರೆ( b_ಸಂಯುಕ್ತ || fk_ಮಗು_ಕೀಲಿ_Is_ಸೂಚ್ಯಂಕಿತ(p_ಮಗು, i_ಮಗು_ಲಂಬಸಾಲು, zColl) ){
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Token tNumeric = { "NUMERIC", 7 };
      ಅಭಿವ್ಯಕ್ತಿ *pCast;
      ಅಭಿವ್ಯಕ್ತಿ_List *pList;

      pCast = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_CAST, &tNumeric, 0);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Attach_Subtrees(db, pCast, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0), 0);
      pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0,
          sqlite3PExpr(pParse, TK_UPLUS, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0), 0)
      );
      pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList,
          sqlite3PExpr(pParse, TK_UPLUS, pCast, 0)
      );
      p_ಒಳ = sqlite3PExpr(pParse, TK_ಒಳ, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(pParse,
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಬಲ, 0), zColl
      ), 0);
      ಆದರೆ( p_ಒಳ ){
        p_ಒಳ->x.pList = pList;
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ_And_Flags(pParse, p_ಒಳ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList);
      }
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  p_ಬಲ = sqlite3PExpr(pParse, TK_UPLUS, p_ಬಲ, 0);
  pEq = sqlite3PExpr(pParse, TK_EQ, p_ಎಡ, p_ಬಲ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CAST
  pEq = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pEq, p_ಒಳ);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ pEq;
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
ಸ್ಥಿರ ಶೂನ್ಯ fk_ಪರಿಶೀಲನೆ_Children(
  Parse *pParse,                  /* Parse context */
  SrcList *pSrc,                  /* The child table to be scanned */
  ಕೋಷ್ಟಕ *pTab,                    /* The parent table */
  ಸೂಚ್ಯಂಕ *pIdx,                    /* Index on parent covering the foreign key */
  FKey *pFKey,                    /* The foreign key linking pSrc to pTab */
  ಪೂರ್ಣಾಂಕ *ai_ಲಂಬಸಾಲು,                     /* Map from pIdx cols to child table cols */
  ಪೂರ್ಣಾಂಕ reg_ದತ್ತಾಂಶ,                    /* Parent row data starts here */
  ಪೂರ್ಣಾಂಕ nIncr                       /* Amount to increment deferred counter by */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  ಪೂರ್ಣಾಂಕ i;                          /* Iterator variable */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere = 0;               /* WHERE clause to scan with */
  NameContext sNameContext;       /* Context used to resolve WHERE clause */
  WhereInfo *pWInfo;              /* Context used by sqlite3WhereXXX() */
  ಪೂರ್ಣಾಂಕ iFkIfZero = 0;              /* Address of OP_FkIfZero */
  Vdbe *v = sqlite3GetVdbe(pParse);

  ಪ್ರತಿಜ್ಞಾ( pIdx==0 || pIdx->p_ಕೋಷ್ಟಕ==pTab );
  ಪ್ರತಿಜ್ಞಾ( pIdx==0 || pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು==pFKey->n_ಲಂಬಸಾಲು );
  ಪ್ರತಿಜ್ಞಾ( pIdx!=0 || pFKey->n_ಲಂಬಸಾಲು==1 );
  ಪ್ರತಿಜ್ಞಾ( pIdx!=0 || HasRowid(pTab) );

  ಆದರೆ( nIncr<0 ){
    iFkIfZero = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_FkIfZero, pFKey->isDeferred, 0);
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
  ಗಾಗಿ(i=0; i<pFKey->n_ಲಂಬಸಾಲು; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;                  /* Value from parent table row */
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ;                 /* Column ref to child table */
    ಅಭಿವ್ಯಕ್ತಿ *pEq;                    /* Expression (pLeft = pRight) */
    i16 i_ಲಂಬಸಾಲು;                     /* Index of column in parent table */
    i16 iCol2;                    /* Index of column in child table */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು;             /* Name of column in child table */

    i_ಲಂಬಸಾಲು = pIdx ? pIdx->ai_ಸ್ತಂಭ[i] : -1;
    p_ಎಡ = ಅಭಿವ್ಯಕ್ತಿ_ಕೋಷ್ಟಕ_ರಿಜಿಸ್ಟರ್(pParse, pTab, reg_ದತ್ತಾಂಶ, i_ಲಂಬಸಾಲು);
    iCol2 = ai_ಲಂಬಸಾಲು ? ai_ಲಂಬಸಾಲು[i] : pFKey->a_ಲಂಬಸಾಲು[0].iFrom;
    ಪ್ರತಿಜ್ಞಾ( iCol2>=0 );
    z_ಲಂಬಸಾಲು = pFKey->pFrom->a_ಲಂಬಸಾಲು[iCol2].zCnName;
    p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, z_ಲಂಬಸಾಲು);
    pEq = fk_ಪೋಷಕ_ಮಗು_Eq(pParse, p_ಎಡ, p_ಬಲ, pTab, i_ಲಂಬಸಾಲು,
                          pFKey->pFrom, iCol2, pFKey->n_ಲಂಬಸಾಲು>0);
    pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pWhere, pEq);
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
  ಆದರೆ( pTab==pFKey->pFrom && nIncr>0 ){
    ಅಭಿವ್ಯಕ್ತಿ *pNe;                    /* Expression (pLeft != pRight) */
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;                  /* Value from parent table row */
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ;                 /* Column ref to child table */
    ಆದರೆ( HasRowid(pTab) ){
      p_ಎಡ = ಅಭಿವ್ಯಕ್ತಿ_ಕೋಷ್ಟಕ_ರಿಜಿಸ್ಟರ್(pParse, pTab, reg_ದತ್ತಾಂಶ, -1);
      p_ಬಲ = ಅಭಿವ್ಯಕ್ತಿ_ಕೋಷ್ಟಕ_ಸ್ತಂಭ(db, pTab, pSrc->a[0].iCursor, -1);
      pNe = sqlite3PExpr(pParse, TK_NE, p_ಎಡ, p_ಬಲ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಭಿವ್ಯಕ್ತಿ *pEq, *pAll = 0;
      ಪ್ರತಿಜ್ಞಾ( pIdx!=0 );
      ಗಾಗಿ(i=0; i<pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
        i16 i_ಲಂಬಸಾಲು = pIdx->ai_ಸ್ತಂಭ[i];
        ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು>=0 );
        p_ಎಡ = ಅಭಿವ್ಯಕ್ತಿ_ಕೋಷ್ಟಕ_ರಿಜಿಸ್ಟರ್(pParse, pTab, reg_ದತ್ತಾಂಶ, i_ಲಂಬಸಾಲು);
        p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName);
        pEq = sqlite3PExpr(pParse, TK_IS, p_ಎಡ, p_ಬಲ);
        pAll = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pAll, pEq);
      }
      pNe = sqlite3PExpr(pParse, TK_NOT, pAll, 0);
    }
    pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pWhere, pNe);
  }

  /* Resolve the references in the WHERE clause. */
  memset(&sNameContext, 0, ಗಾತ್ರ(NameContext));
  sNameContext.pSrcList = pSrc;
  sNameContext.pParse = pParse;
  sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNameContext, pWhere);

  /* Create VDBE to loop through the entries in pSrc that match the WHERE
  ** clause. For each row found, increment either the deferred or immediate
  ** foreign key constraint counter. */
  ಆದರೆ( pParse->nErr==0 ){
    pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0, 0);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
    ಆದರೆ( pWInfo ){
      sqlite3_Where_ಕೊನೆ(pWInfo);
    }
  }

  /* Clean up the WHERE clause constructed above. */
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
  ಆದರೆ( iFkIfZero ){
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ_Or_ತೆಗೆ_Inst(v, iFkIfZero);
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
FKey *sqlite3FkReferences(ಕೋಷ್ಟಕ *pTab){
  ಹಿಂತಿರುಗಿ (FKey *)sqlite3_Hash_ಕಂಡುಹಿಡಿ(&pTab->pSchema->fkeyHash, pTab->zName);
}

/*
** The second argument is a Trigger structure allocated by the 
** fkActionTrigger() routine. This function deletes the Trigger structure
** and all of its sub-components.
**
** The Trigger structure or any of its sub-components may be allocated from
** the lookaside buffer belonging to database handle dbMem.
*/
ಸ್ಥಿರ ಶೂನ್ಯ fkTriggerDelete(sqlite3 *dbMem, Trigger *p){
  ಆದರೆ( p ){
    Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ = p->ಹೆಜ್ಜೆ_list;
    sqlite3SrcListDelete(dbMem, p_ಹೆಜ್ಜೆ->pSrc);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(dbMem, p_ಹೆಜ್ಜೆ->pWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(dbMem, p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List);
    sqlite3SelectDelete(dbMem, p_ಹೆಜ್ಜೆ->pSelect);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(dbMem, p->pWhen);
    sqlite3DbFree(dbMem, p);
  }
}

/*
** Clear the apTrigger[] cache of CASCADE triggers for all foreign keys
** in a particular database.  This needs to happen when the schema
** changes.
*/
ಶೂನ್ಯ sqlite3FkClearTriggerCache(sqlite3 *db, ಪೂರ್ಣಾಂಕ iDb){
  HashElem *k;
  Hash *pHash = &db->aDb[iDb].pSchema->tblHash;
  ಗಾಗಿ(k=sqlite_Hash_ಮೊದಲ(pHash); k; k=sqlite_Hash_ಮುಂದಿನ(k)){
    ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(k);
    FKey *pFKey;
    ಆದರೆ( !Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ) ಮುಂದುವರಿ;
    ಗಾಗಿ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_ಮುಂದಿನ_From){
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
ಶೂನ್ಯ sqlite3_Fk_Drop_ಕೋಷ್ಟಕ(Parse *pParse, SrcList *pName, ಕೋಷ್ಟಕ *pTab){
  sqlite3 *db = pParse->db;
  ಆದರೆ( (db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು) && Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ){
    ಪೂರ್ಣಾಂಕ iSkip = 0;
    Vdbe *v = sqlite3GetVdbe(pParse);

    ಪ್ರತಿಜ್ಞಾ( v );                  /* VDBE has already been allocated */
    ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
    ಆದರೆ( sqlite3FkReferences(pTab)==0 ){
      /* Search for a deferred foreign key constraint for which this table
      ** is the child table. If one cannot be found, return without 
      ** generating any VDBE code. If one can be found, then jump over
      ** the entire DELETE if there are no outstanding deferred constraints
      ** when this statement is run.  */
      FKey *p;
      ಗಾಗಿ(p=pTab->u.tab.pFKey; p; p=p->p_ಮುಂದಿನ_From){
        ಆದರೆ( p->isDeferred || (db->flags & SQLITE_DeferFKs) ) ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( !p ) ಹಿಂತಿರುಗಿ;
      iSkip = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_FkIfZero, 1, iSkip); VdbeCoverage(v);
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
    ಆದರೆ( (db->flags & SQLITE_DeferFKs)==0 ){
      sqlite3VdbeVerifyAbortable(v, OE_Abort);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_FkIfZero, 0, sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
      sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
          OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
    }

    ಆದರೆ( iSkip ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fk_ಮಗು_Is_Modified(
  ಕೋಷ್ಟಕ *pTab,                    /* Table being updated */
  FKey *p,                        /* Foreign key for which pTab is the child */
  ಪೂರ್ಣಾಂಕ *aChange,                   /* Array indicating modified columns */
  ಪೂರ್ಣಾಂಕ bChngRowid                  /* True if rowid is modified by this update */
){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<p->n_ಲಂಬಸಾಲು; i++){
    ಪೂರ್ಣಾಂಕ i_ಮಗು_ಕೀಲಿ = p->a_ಲಂಬಸಾಲು[i].iFrom;
    ಆದರೆ( aChange[i_ಮಗು_ಕೀಲಿ]>=0 ) ಹಿಂತಿರುಗಿ 1;
    ಆದರೆ( i_ಮಗು_ಕೀಲಿ==pTab->iPKey && bChngRowid ) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fk_ಪೋಷಕ_Is_Modified(
  ಕೋಷ್ಟಕ *pTab, 
  FKey *p, 
  ಪೂರ್ಣಾಂಕ *aChange, 
  ಪೂರ್ಣಾಂಕ bChngRowid
){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<p->n_ಲಂಬಸಾಲು; i++){
    ಅಕ್ಷರ *z_ಕೀಲಿ = p->a_ಲಂಬಸಾಲು[i].z_ಲಂಬಸಾಲು;
    ಪೂರ್ಣಾಂಕ i_ಕೀಲಿ;
    ಗಾಗಿ(i_ಕೀಲಿ=0; i_ಕೀಲಿ<pTab->n_ಲಂಬಸಾಲು; i_ಕೀಲಿ++){
      ಆದರೆ( aChange[i_ಕೀಲಿ]>=0 || (i_ಕೀಲಿ==pTab->iPKey && bChngRowid) ){
        ಸ್ತಂಭ *p_ಲಂಬಸಾಲು = &pTab->a_ಲಂಬಸಾಲು[i_ಕೀಲಿ];
        ಆದರೆ( z_ಕೀಲಿ ){
          ಆದರೆ( 0==sqlite3StrICmp(p_ಲಂಬಸಾಲು->zCnName, z_ಕೀಲಿ) ) ಹಿಂತಿರುಗಿ 1;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_PRIMKEY ){
          ಹಿಂತಿರುಗಿ 1;
        }
      }
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return true if the parser passed as the first argument is being
** used to code a trigger that is really a "SET NULL" action belonging
** to trigger pFKey.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_ಗಣ_Null_Action(Parse *pParse, FKey *pFKey){
  Trigger *p = pParse->pTrigCoded;
  ಆದರೆ( p ){
    ಆದರೆ( (p==pFKey->apTrigger[0] && pFKey->aAction[0]==OE_ಗಣ_Null)
     || (p==pFKey->apTrigger[1] && pFKey->aAction[1]==OE_ಗಣ_Null)
    ){
      ಪ್ರತಿಜ್ಞಾ( (pParse->db->flags & SQLITE_FkNoAction)==0 );
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಶೂನ್ಯ sqlite3FkCheck(
  Parse *pParse,                  /* Parse context */
  ಕೋಷ್ಟಕ *pTab,                    /* Row is being deleted from this table */ 
  ಪೂರ್ಣಾಂಕ regOld,                     /* Previous row data is stored here */
  ಪೂರ್ಣಾಂಕ regNew,                     /* New row data is stored here */
  ಪೂರ್ಣಾಂಕ *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  ಪೂರ್ಣಾಂಕ bChngRowid                  /* True if rowid is UPDATEd */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  FKey *pFKey;                    /* Used to iterate through FKs */
  ಪೂರ್ಣಾಂಕ iDb;                        /* Index of database containing pTab */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;                /* Name of database containing pTab */
  ಪೂರ್ಣಾಂಕ isIgnoreErrors = pParse->disableTriggers;

  /* Exactly one of regOld and regNew should be non-zero. */
  ಪ್ರತಿಜ್ಞಾ( (regOld==0)!=(regNew==0) );

  /* If foreign-keys are disabled, this function is a no-op. */
  ಆದರೆ( (db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು)==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( !Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ) ಹಿಂತಿರುಗಿ;

  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=00 && iDb<db->nDb );
  zDb = db->aDb[iDb].zDbSName;

  /* Loop through all the foreign key constraints for which pTab is the
  ** child table (the table that the foreign key definition is part of).  */
  ಗಾಗಿ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_ಮುಂದಿನ_From){
    ಕೋಷ್ಟಕ *p_ಗಮ್ಯ;                   /* Parent table of foreign key pFKey */
    ಸೂಚ್ಯಂಕ *pIdx = 0;              /* Index on key columns in pTo */
    ಪೂರ್ಣಾಂಕ *aiFree = 0;
    ಪೂರ್ಣಾಂಕ *ai_ಲಂಬಸಾಲು;
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ bIgnore = 0;

    ಆದರೆ( aChange 
     && sqlite3_stricmp(pTab->zName, pFKey->z_ಗಮ್ಯ)!=0
     && fk_ಮಗು_Is_Modified(pTab, pFKey, aChange, bChngRowid)==0 
    ){
      ಮುಂದುವರಿ;
    }

    /* Find the parent table of this foreign key. Also find a unique index 
    ** on the parent key columns in the parent table. If either of these 
    ** schema items cannot be located, set an error in pParse and return 
    ** early.  */
    ಆದರೆ( pParse->disableTriggers ){
      p_ಗಮ್ಯ = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, pFKey->z_ಗಮ್ಯ, zDb);
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಗಮ್ಯ = sqlite3_Locate_ಕೋಷ್ಟಕ(pParse, 0, pFKey->z_ಗಮ್ಯ, zDb);
    }
    ಆದರೆ( !p_ಗಮ್ಯ || sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(pParse, p_ಗಮ್ಯ, pFKey, &pIdx, &aiFree) ){
      ಪ್ರತಿಜ್ಞಾ( isIgnoreErrors==0 || (regOld!=0 && regNew==0) );
      ಆದರೆ( !isIgnoreErrors || db->mallocFailed ) ಹಿಂತಿರುಗಿ;
      ಆದರೆ( p_ಗಮ್ಯ==0 ){
        /* If isIgnoreErrors is true, then a table is being dropped. In this
        ** case SQLite runs a "DELETE FROM xxx" on the table being dropped
        ** before actually dropping it in order to check FK constraints.
        ** If the parent table of an FK constraint on the current table is
        ** missing, behave as if it is empty. i.e. decrement the relevant
        ** FK counter for each row of the current table with non-NULL keys.
        */
        Vdbe *v = sqlite3GetVdbe(pParse);
        ಪೂರ್ಣಾಂಕ iJump = sqlite3VdbeCurrentAddr(v) + pFKey->n_ಲಂಬಸಾಲು + 1;
        ಗಾಗಿ(i=0; i<pFKey->n_ಲಂಬಸಾಲು; i++){
          ಪೂರ್ಣಾಂಕ i_From_ಲಂಬಸಾಲು, iReg;
          i_From_ಲಂಬಸಾಲು = pFKey->a_ಲಂಬಸಾಲು[i].iFrom;
          iReg = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pFKey->pFrom,i_From_ಲಂಬಸಾಲು) + regOld+1;
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, iReg, iJump); VdbeCoverage(v);
        }
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_FkCounter, pFKey->isDeferred, -1);
      }
      ಮುಂದುವರಿ;
    }
    ಪ್ರತಿಜ್ಞಾ( pFKey->n_ಲಂಬಸಾಲು==1 || (aiFree && pIdx) );

    ಆದರೆ( aiFree ){
      ai_ಲಂಬಸಾಲು = aiFree;
    }ಇಲ್ಲದಿದ್ದರೆ{
      i_ಲಂಬಸಾಲು = pFKey->a_ಲಂಬಸಾಲು[0].iFrom;
      ai_ಲಂಬಸಾಲು = &i_ಲಂಬಸಾಲು;
    }
    ಗಾಗಿ(i=0; i<pFKey->n_ಲಂಬಸಾಲು; i++){
      ಆದರೆ( ai_ಲಂಬಸಾಲು[i]==pTab->iPKey ){
        ai_ಲಂಬಸಾಲು[i] = -1;
      }
      ಪ್ರತಿಜ್ಞಾ( pIdx==0 || pIdx->ai_ಸ್ತಂಭ[i]>=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
      /* Request permission to read the parent key columns. If the 
      ** authorization callback returns SQLITE_IGNORE, behave as if any
      ** values read from the parent table are NULL. */
      ಆದರೆ( db->xAuth ){
        ಪೂರ್ಣಾಂಕ rcauth;
        ಅಕ್ಷರ *z_ಲಂಬಸಾಲು = p_ಗಮ್ಯ->a_ಲಂಬಸಾಲು[pIdx ? pIdx->ai_ಸ್ತಂಭ[i] : p_ಗಮ್ಯ->iPKey].zCnName;
        rcauth = sqlite3_Auth_Read_ಲಂಬಸಾಲು(pParse, p_ಗಮ್ಯ->zName, z_ಲಂಬಸಾಲು, iDb);
        bIgnore = (rcauth==SQLITE_IGNORE);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }

    /* Take a shared-cache advisory read-lock on the parent table. Allocate 
    ** a cursor to use to search the unique index on the parent key columns 
    ** in the parent table.  */
    sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, p_ಗಮ್ಯ->tnum, 0, p_ಗಮ್ಯ->zName);
    pParse->nTab++;

    ಆದರೆ( regOld!=0 ){
      /* A row is being removed from the child table. Search for the parent.
      ** If the parent does not exist, removing the child row resolves an 
      ** outstanding foreign key constraint violation. */
      fk_Lookup_ಪೋಷಕ(pParse, iDb, p_ಗಮ್ಯ, pIdx, pFKey, ai_ಲಂಬಸಾಲು, regOld, -1, bIgnore);
    }
    ಆದರೆ( regNew!=0 && !is_ಗಣ_Null_Action(pParse, pFKey) ){
      /* A row is being added to the child table. If a parent row cannot
      ** be found, adding the child row has violated the FK constraint. 
      **
      ** If this operation is being performed as part of a trigger program
      ** that is actually a "SET NULL" action belonging to this very 
      ** foreign key, then omit this scan altogether. As all child key
      ** values are guaranteed to be NULL, it is not possible for adding
      ** this row to cause an FK violation.  */
      fk_Lookup_ಪೋಷಕ(pParse, iDb, p_ಗಮ್ಯ, pIdx, pFKey, ai_ಲಂಬಸಾಲು, regNew, +1, bIgnore);
    }

    sqlite3DbFree(db, aiFree);
  }

  /* Loop through all the foreign key constraints that refer to this table.
  ** (the "child" constraints) */
  ಗಾಗಿ(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_ಮುಂದಿನ_ಗಮ್ಯ){
    ಸೂಚ್ಯಂಕ *pIdx = 0;              /* Foreign key index for pFKey */
    SrcList *pSrc;
    ಪೂರ್ಣಾಂಕ *ai_ಲಂಬಸಾಲು = 0;

    ಆದರೆ( aChange && fk_ಪೋಷಕ_Is_Modified(pTab, pFKey, aChange, bChngRowid)==0 ){
      ಮುಂದುವರಿ;
    }

    ಆದರೆ( !pFKey->isDeferred && !(db->flags & SQLITE_DeferFKs) 
     && !pParse->pToplevel && !pParse->isMultiWrite 
    ){
      ಪ್ರತಿಜ್ಞಾ( regOld==0 && regNew!=0 );
      /* Inserting a single row into a parent table cannot cause (or fix)
      ** an immediate foreign key violation. So do nothing in this case.  */
      ಮುಂದುವರಿ;
    }

    ಆದರೆ( sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(pParse, pTab, pFKey, &pIdx, &ai_ಲಂಬಸಾಲು) ){
      ಆದರೆ( !isIgnoreErrors || db->mallocFailed ) ಹಿಂತಿರುಗಿ;
      ಮುಂದುವರಿ;
    }
    ಪ್ರತಿಜ್ಞಾ( ai_ಲಂಬಸಾಲು || pFKey->n_ಲಂಬಸಾಲು==1 );

    /* Create a SrcList structure containing the child table.  We need the
    ** child table as a SrcList for sqlite3WhereBegin() */
    pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
    ಆದರೆ( pSrc ){
      Src_ಅಂಶ *p_ಅಂಶ = pSrc->a;
      p_ಅಂಶ->pSTab = pFKey->pFrom;
      p_ಅಂಶ->zName = pFKey->pFrom->zName;
      p_ಅಂಶ->pSTab->nTabRef++;
      p_ಅಂಶ->iCursor = pParse->nTab++;
  
      ಆದರೆ( regNew!=0 ){
        fk_ಪರಿಶೀಲನೆ_Children(pParse, pSrc, pTab, pIdx, pFKey, ai_ಲಂಬಸಾಲು, regNew, -1);
      }
      ಆದರೆ( regOld!=0 ){
        ಪೂರ್ಣಾಂಕ eAction = pFKey->aAction[aChange!=0];
        ಆದರೆ( (db->flags & SQLITE_FkNoAction) ) eAction = OE_None;

        fk_ಪರಿಶೀಲನೆ_Children(pParse, pSrc, pTab, pIdx, pFKey, ai_ಲಂಬಸಾಲು, regOld, 1);
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
        ಆದರೆ( !pFKey->isDeferred && eAction!=OE_Cascade && eAction!=OE_ಗಣ_Null ){
          sqlite3MayAbort(pParse);
        }
      }
      p_ಅಂಶ->zName = 0;
      sqlite3SrcListDelete(db, pSrc);
    }
    sqlite3DbFree(db, ai_ಲಂಬಸಾಲು);
  }
}

#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ತಂಭ_ಮುಖವಾಡ(x) (((x)>31) ? 0xffffffff : ((u32)1<<(x)))

/*
** This function is called before generating code to update or delete a 
** row contained in table pTab.
*/
u32 sqlite3FkOldmask(
  Parse *pParse,                  /* Parse context */
  ಕೋಷ್ಟಕ *pTab                     /* Table being modified */
){
  u32 ಮುಖವಾಡ = 0;
  ಆದರೆ( pParse->db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು && Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ){
    FKey *p;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(p=pTab->u.tab.pFKey; p; p=p->p_ಮುಂದಿನ_From){
      ಗಾಗಿ(i=0; i<p->n_ಲಂಬಸಾಲು; i++) ಮುಖವಾಡ |= ಸ್ತಂಭ_ಮುಖವಾಡ(p->a_ಲಂಬಸಾಲು[i].iFrom);
    }
    ಗಾಗಿ(p=sqlite3FkReferences(pTab); p; p=p->p_ಮುಂದಿನ_ಗಮ್ಯ){
      ಸೂಚ್ಯಂಕ *pIdx = 0;
      sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(pParse, pTab, p, &pIdx, 0);
      ಆದರೆ( pIdx ){
        ಗಾಗಿ(i=0; i<pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
          ಪ್ರತಿಜ್ಞಾ( pIdx->ai_ಸ್ತಂಭ[i]>=0 );
          ಮುಖವಾಡ |= ಸ್ತಂಭ_ಮುಖವಾಡ(pIdx->ai_ಸ್ತಂಭ[i]);
        }
      }
    }
  }
  ಹಿಂತಿರುಗಿ ಮುಖವಾಡ;
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
ಪೂರ್ಣಾಂಕ sqlite3FkRequired(
  Parse *pParse,                  /* Parse context */
  ಕೋಷ್ಟಕ *pTab,                    /* Table being modified */
  ಪೂರ್ಣಾಂಕ *aChange,                   /* Non-NULL for UPDATE operations */
  ಪೂರ್ಣಾಂಕ chngRowid                   /* True for UPDATE that affects rowid */
){
  ಪೂರ್ಣಾಂಕ eRet = 1;                   /* Value to return if bHaveFK is true */
  ಪೂರ್ಣಾಂಕ bHaveFK = 0;                /* If FK processing is required */
  ಆದರೆ( pParse->db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು && Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ){
    ಆದರೆ( !aChange ){
      /* A DELETE operation. Foreign key processing is required if the 
      ** table in question is either the child or parent table for any 
      ** foreign key constraint.  */
      bHaveFK = (sqlite3FkReferences(pTab) || pTab->u.tab.pFKey);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* This is an UPDATE. Foreign key processing is only required if the
      ** operation modifies one or more child or parent key columns. */
      FKey *p;

      /* Check if any child key columns are being modified. */
      ಗಾಗಿ(p=pTab->u.tab.pFKey; p; p=p->p_ಮುಂದಿನ_From){
        ಆದರೆ( fk_ಮಗು_Is_Modified(pTab, p, aChange, chngRowid) ){
          ಆದರೆ( 0==sqlite3_stricmp(pTab->zName, p->z_ಗಮ್ಯ) ) eRet = 2;
          bHaveFK = 1;
        }
      }

      /* Check if any parent key columns are being modified. */
      ಗಾಗಿ(p=sqlite3FkReferences(pTab); p; p=p->p_ಮುಂದಿನ_ಗಮ್ಯ){
        ಆದರೆ( fk_ಪೋಷಕ_Is_Modified(pTab, p, aChange, chngRowid) ){
          ಆದರೆ( (pParse->db->flags & SQLITE_FkNoAction)==0 
           && p->aAction[1]!=OE_None 
          ){
            ಹಿಂತಿರುಗಿ 2;
          }
          bHaveFK = 1;
        }
      }
    }
  }
  ಹಿಂತಿರುಗಿ bHaveFK ? eRet : 0;
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
ಸ್ಥಿರ Trigger *fkActionTrigger(
  Parse *pParse,                  /* Parse context */
  ಕೋಷ್ಟಕ *pTab,                    /* Table being updated or deleted from */
  FKey *pFKey,                    /* Foreign key to get action for */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges              /* Change-list for UPDATE, NULL for DELETE */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  ಪೂರ್ಣಾಂಕ action;                     /* One of OE_None, OE_Cascade etc. */
  Trigger *pTrigger;              /* Trigger definition to return */
  ಪೂರ್ಣಾಂಕ iAction = (pChanges!=0);    /* 1 for UPDATE, 0 for DELETE */

  action = pFKey->aAction[iAction];
  ಆದರೆ( (db->flags & SQLITE_FkNoAction) ) action = OE_None;
  ಆದರೆ( action==OE_Restrict && (db->flags & SQLITE_DeferFKs) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  pTrigger = pFKey->apTrigger[iAction];

  ಆದರೆ( action!=OE_None && !pTrigger ){
    ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zFrom;            /* Name of child table */
    ಪೂರ್ಣಾಂಕ nFrom;                    /* Length in bytes of zFrom */
    ಸೂಚ್ಯಂಕ *pIdx = 0;              /* Parent key index for this FK */
    ಪೂರ್ಣಾಂಕ *ai_ಲಂಬಸಾಲು = 0;               /* child table cols -> parent key cols */
    Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ = 0;        /* First (only) step of trigger program */
    ಅಭಿವ್ಯಕ್ತಿ *pWhere = 0;             /* WHERE clause of trigger step */
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = 0;          /* Changes list if ON UPDATE CASCADE */
    Select *pSelect = 0;          /* If RESTRICT, "SELECT RAISE(...)" */
    ಪೂರ್ಣಾಂಕ i;                        /* Iterator variable */
    ಅಭಿವ್ಯಕ್ತಿ *pWhen = 0;              /* WHEN clause for the trigger */

    ಆದರೆ( sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(pParse, pTab, pFKey, &pIdx, &ai_ಲಂಬಸಾಲು) ) ಹಿಂತಿರುಗಿ 0;
    ಪ್ರತಿಜ್ಞಾ( ai_ಲಂಬಸಾಲು || pFKey->n_ಲಂಬಸಾಲು==1 );

    ಗಾಗಿ(i=0; i<pFKey->n_ಲಂಬಸಾಲು; i++){
      Token tOld = { "old", 3 };  /* Literal "old" token */
      Token tNew = { "new", 3 };  /* Literal "new" token */
      Token t_From_ಲಂಬಸಾಲು;             /* Name of column in child table */
      Token t_ಗಮ್ಯ_ಲಂಬಸಾಲು;               /* Name of column in parent table */
      ಪೂರ್ಣಾಂಕ i_From_ಲಂಬಸಾಲು;               /* Idx of column in child table */
      ಪೂರ್ಣಾಂಕ i_ಗಮ್ಯ_ಲಂಬಸಾಲು;                 /* Idx of column in parent table */
      ಅಭಿವ್ಯಕ್ತಿ *pEq;                  /* OLD.tToCol = tFromCol */
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;                /* OLD.tToCol */
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ;               /* tFromCol */

      i_From_ಲಂಬಸಾಲು = ai_ಲಂಬಸಾಲು ? ai_ಲಂಬಸಾಲು[i] : pFKey->a_ಲಂಬಸಾಲು[0].iFrom;
      ಪ್ರತಿಜ್ಞಾ( i_From_ಲಂಬಸಾಲು>=0 );
      ಪ್ರತಿಜ್ಞಾ( pIdx!=0 || (pTab->iPKey>=0 && pTab->iPKey<pTab->n_ಲಂಬಸಾಲು) );
      ಪ್ರತಿಜ್ಞಾ( pIdx==0 || pIdx->ai_ಸ್ತಂಭ[i]>=0 );
      i_ಗಮ್ಯ_ಲಂಬಸಾಲು = pIdx ? pIdx->ai_ಸ್ತಂಭ[i] : pTab->iPKey;
      sqlite3TokenInit(&t_ಗಮ್ಯ_ಲಂಬಸಾಲು, pTab->a_ಲಂಬಸಾಲು[i_ಗಮ್ಯ_ಲಂಬಸಾಲು].zCnName);
      sqlite3TokenInit(&t_From_ಲಂಬಸಾಲು, pFKey->pFrom->a_ಲಂಬಸಾಲು[i_From_ಲಂಬಸಾಲು].zCnName);

      p_ಎಡ = sqlite3PExpr(pParse, TK_DOT,
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &tOld, 0),
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &t_ಗಮ್ಯ_ಲಂಬಸಾಲು, 0)
      );
      p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &t_From_ಲಂಬಸಾಲು, 0);

      /* OLD.* columns do not normally have an affinity. So set the affinity
      ** of OLD.tToCol (the parent key) explicitly here. */
      ಆದರೆ( p_ಎಡ ) p_ಎಡ->aff_ಅಭಿವ್ಯಕ್ತಿ = pTab->a_ಲಂಬಸಾಲು[i_ಗಮ್ಯ_ಲಂಬಸಾಲು].affinity;

      /* Create the expression "OLD.tToCol = tFromCol". It is important
      ** that the "OLD.tToCol" term is on the LHS of the = operator, so
      ** that the collation sequence associated with the parent table is
      ** used for the comparison. */
      pEq = fk_ಪೋಷಕ_ಮಗು_Eq(pParse, p_ಎಡ, p_ಬಲ, pTab, i_ಗಮ್ಯ_ಲಂಬಸಾಲು,
                            pFKey->pFrom, i_From_ಲಂಬಸಾಲು, pFKey->n_ಲಂಬಸಾಲು>1);
      pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pWhere, pEq);

      /* For ON UPDATE, construct the next term of the WHEN clause.
      ** The final WHEN clause will be like this:
      **
      **    WHEN NOT(old.col1 IS new.col1 AND ... AND old.colN IS new.colN)
      */
      ಆದರೆ( pChanges ){
        pEq = sqlite3PExpr(pParse, TK_IS,
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &tOld, 0),
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &t_ಗಮ್ಯ_ಲಂಬಸಾಲು, 0)),
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &tNew, 0),
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &t_ಗಮ್ಯ_ಲಂಬಸಾಲು, 0))
            );
        pWhen = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pWhen, pEq);
      }
  
      ಆದರೆ( action!=OE_Restrict && (action!=OE_Cascade || pChanges) ){
        ಅಭಿವ್ಯಕ್ತಿ *pNew;
        ಆದರೆ( action==OE_Cascade ){
          pNew = sqlite3PExpr(pParse, TK_DOT, 
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &tNew, 0),
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ID, &t_ಗಮ್ಯ_ಲಂಬಸಾಲು, 0));
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( action==OE_ಗಣ_Dflt ){
          ಸ್ತಂಭ *p_ಲಂಬಸಾಲು = pFKey->pFrom->a_ಲಂಬಸಾಲು + i_From_ಲಂಬಸಾಲು;
          ಅಭಿವ್ಯಕ್ತಿ *pDflt;
          ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED ){
            testcase( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL );
            testcase( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_STORED );
            pDflt = 0;
          }ಇಲ್ಲದಿದ್ದರೆ{
            pDflt = sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Auth(pFKey->pFrom, p_ಲಂಬಸಾಲು, pParse);
          }
          ಆದರೆ( pDflt ){
            pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pDflt, 0);
          }ಇಲ್ಲದಿದ್ದರೆ{
            pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_NULL, 0, 0);
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_NULL, 0, 0);
        }
        pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList, pNew);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಣ_Name(pParse, pList, &t_From_ಲಂಬಸಾಲು, 0);
      }
    }
    sqlite3DbFree(db, ai_ಲಂಬಸಾಲು);

    zFrom = pFKey->pFrom->zName;
    nFrom = sqlite3Strlen30(zFrom);

    ಆದರೆ( action==OE_Restrict ){
      SrcList *pSrc;
      ಅಭಿವ್ಯಕ್ತಿ *pRaise; 

      pRaise = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_STRING, "FOREIGN KEY constraint failed"),
      pRaise = sqlite3PExpr(pParse, TK_RAISE, pRaise, 0);
      ಆದರೆ( pRaise ){
        pRaise->aff_ಅಭಿವ್ಯಕ್ತಿ = OE_Abort;
      }
      pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      ಆದರೆ( pSrc ){
        Src_ಅಂಶ *p_ಅಂಶ = &pSrc->a[0];
        p_ಅಂಶ->zName = sqlite3DbStrDup(db, zFrom);
        p_ಅಂಶ->fg.fixedSchema = 1;
        p_ಅಂಶ->u4.pSchema = pTab->pSchema;
      }
      pSelect = sqlite3SelectNew(pParse, 
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, pRaise),
          pSrc,
          pWhere,
          0, 0, 0, 0, 0
      );
      pWhere = 0;
    }

    /* Disable lookaside memory allocation */
    DisableLookaside;

    pTrigger = (Trigger *)sqlite3DbMallocZero(db, 
        ಗಾತ್ರ(Trigger) +         /* struct Trigger */
        ಗಾತ್ರ(Trigger_ಹೆಜ್ಜೆ)       /* Single step in trigger program */
    );
    ಆದರೆ( pTrigger ){
      p_ಹೆಜ್ಜೆ = pTrigger->ಹೆಜ್ಜೆ_list = (Trigger_ಹೆಜ್ಜೆ *)&pTrigger[1];
      p_ಹೆಜ್ಜೆ->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      ಆದರೆ( p_ಹೆಜ್ಜೆ->pSrc ){
        Src_ಅಂಶ *p_ಅಂಶ = &p_ಹೆಜ್ಜೆ->pSrc->a[0];
        p_ಅಂಶ->zName = sqlite3DbStrNDup(db, zFrom, nFrom);
        p_ಅಂಶ->u4.pSchema = pTab->pSchema;
        p_ಅಂಶ->fg.fixedSchema = 1;
      }
      p_ಹೆಜ್ಜೆ->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhere, EXPRDUP_REDUCE);
      p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pList, EXPRDUP_REDUCE);
      p_ಹೆಜ್ಜೆ->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
      ಆದರೆ( pWhen ){
        pWhen = sqlite3PExpr(pParse, TK_NOT, pWhen, 0);
        pTrigger->pWhen = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhen, EXPRDUP_REDUCE);
      }
    }

    /* Re-enable the lookaside buffer, if it was disabled earlier. */
    EnableLookaside;

    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhen);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList);
    sqlite3SelectDelete(db, pSelect);
    ಆದರೆ( db->mallocFailed==1 ){
      fkTriggerDelete(db, pTrigger);
      ಹಿಂತಿರುಗಿ 0;
    }
    ಪ್ರತಿಜ್ಞಾ( p_ಹೆಜ್ಜೆ!=0 );
    ಪ್ರತಿಜ್ಞಾ( pTrigger!=0 );

    ಆಯ್ಕೆ( action ){
      ಸ್ಥಿತಿ OE_Restrict:
        p_ಹೆಜ್ಜೆ->op = TK_SELECT;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ OE_Cascade: 
        ಆದರೆ( !pChanges ){ 
          p_ಹೆಜ್ಜೆ->op = TK_DELETE; 
          ನಿಲ್ಲಿಸು; 
        }
        /* no break */ deliberate_fall_through
      ಪೂರ್ವನಿಯೋಜಿತ:
        p_ಹೆಜ್ಜೆ->op = TK_UPDATE;
    }
    p_ಹೆಜ್ಜೆ->pTrig = pTrigger;
    pTrigger->pSchema = pTab->pSchema;
    pTrigger->pTabSchema = pTab->pSchema;
    pFKey->apTrigger[iAction] = pTrigger;
    pTrigger->op = (pChanges ? TK_UPDATE : TK_DELETE);
  }

  ಹಿಂತಿರುಗಿ pTrigger;
}

/*
** This function is called when deleting or updating a row to implement
** any required CASCADE, SET NULL or SET DEFAULT actions.
*/
ಶೂನ್ಯ sqlite3FkActions(
  Parse *pParse,                  /* Parse context */
  ಕೋಷ್ಟಕ *pTab,                    /* Table being updated or deleted from */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,             /* Change-list for UPDATE, NULL for DELETE */
  ಪೂರ್ಣಾಂಕ regOld,                     /* Address of array containing old row */
  ಪೂರ್ಣಾಂಕ *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  ಪೂರ್ಣಾಂಕ bChngRowid                  /* True if rowid is UPDATEd */
){
  /* If foreign-key support is enabled, iterate through all FKs that 
  ** refer to table pTab. If there is an action associated with the FK 
  ** for this operation (either update or delete), invoke the associated 
  ** trigger sub-program.  */
  ಆದರೆ( pParse->db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು ){
    FKey *pFKey;                  /* Iterator variable */
    ಗಾಗಿ(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->p_ಮುಂದಿನ_ಗಮ್ಯ){
      ಆದರೆ( aChange==0 || fk_ಪೋಷಕ_Is_Modified(pTab, pFKey, aChange, bChngRowid) ){
        Trigger *pAct = fkActionTrigger(pParse, pTab, pFKey, pChanges);
        ಆದರೆ( pAct ){
          sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger_ನೇರ(pParse, pAct, pTab, regOld, OE_Abort, 0);
        }
      }
    }
  }
}

#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_OMIT_TRIGGER */

/*
** Free all memory associated with foreign key definitions attached to
** table pTab. Remove the deleted foreign keys from the Schema.fkeyHash
** hash table.
*/
ಶೂನ್ಯ sqlite3FkDelete(sqlite3 *db, ಕೋಷ್ಟಕ *pTab){
  FKey *pFKey;                    /* Iterator variable */
  FKey *p_ಮುಂದಿನ;                    /* Copy of pFKey->pNextFrom */

  ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಗಾಗಿ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=p_ಮುಂದಿನ){
    ಪ್ರತಿಜ್ಞಾ( db==0 || sqlite3SchemaMutexHeld(db, 0, pTab->pSchema) );

    /* Remove the FK from the fkeyHash hash table. */
    ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed==0 ){
      ಆದರೆ( pFKey->p_Prev_ಗಮ್ಯ ){
        pFKey->p_Prev_ಗಮ್ಯ->p_ಮುಂದಿನ_ಗಮ್ಯ = pFKey->p_ಮುಂದಿನ_ಗಮ್ಯ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = (pFKey->p_ಮುಂದಿನ_ಗಮ್ಯ ? pFKey->p_ಮುಂದಿನ_ಗಮ್ಯ->z_ಗಮ್ಯ : pFKey->z_ಗಮ್ಯ);
        sqlite3_Hash_ಸೇರಿಸು(&pTab->pSchema->fkeyHash, z, pFKey->p_ಮುಂದಿನ_ಗಮ್ಯ);
      }
      ಆದರೆ( pFKey->p_ಮುಂದಿನ_ಗಮ್ಯ ){
        pFKey->p_ಮುಂದಿನ_ಗಮ್ಯ->p_Prev_ಗಮ್ಯ = pFKey->p_Prev_ಗಮ್ಯ;
      }
    }

    /* EV: R-30323-21917 Each foreign key constraint in SQLite is
    ** classified as either immediate or deferred.
    */
    ಪ್ರತಿಜ್ಞಾ( pFKey->isDeferred==0 || pFKey->isDeferred==1 );

    /* Delete any triggers created to implement actions for this FK. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
    fkTriggerDelete(db, pFKey->apTrigger[0]);
    fkTriggerDelete(db, pFKey->apTrigger[1]);
#ಆದರೆ_ಅಂತ್ಯ

    p_ಮುಂದಿನ = pFKey->p_ಮುಂದಿನ_From;
    sqlite3DbFree(db, pFKey);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_OMIT_FOREIGN_KEY */
