/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** The code in this file implements the function that runs the
** bytecode of a prepared statement.
**
** Various scripts scan this source file in order to generate HTML
** documentation, headers files, or other derived files.  The formatting
** of the code in this file is, therefore, important.  See other comments
** in this file for details.  If in doubt, do not deviate from existing
** commenting and indentation practices when changing or adding code.
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/*
** High-resolution hardware timer used for debugging and testing only.
*/
#ಆದರೆ defined(VDBE_PROFILE)  \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
# include "hwtime.h"
#ಆದರೆ_ಅಂತ್ಯ

/*
** Invoke this macro on memory cells just prior to changing the
** value of the cell.  This macro verifies that shallow copies are
** not misused.  A shallow copy of a string or blob just copies a
** pointer to the string or blob, not the content.  If the original
** is changed while the copy is still in use, the string or blob might
** be changed out from under the copy.  This macro verifies that nothing
** like that ever happens.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
# define mem_About_ಗಮ್ಯ_Change(P,M) sqlite3_Vdbe_Mem_About_ಗಮ್ಯ_Change(P,M)
#ಇಲ್ಲದಿದ್ದರೆ
# define mem_About_ಗಮ್ಯ_Change(P,M)
#ಆದರೆ_ಅಂತ್ಯ

/*
** The following global variable is incremented every time a cursor
** moves, either by the OP_SeekXX, OP_Next, or OP_Prev opcodes.  The test
** procedures use this information to make sure that indices are
** working correctly.  This variable has no function other than to
** help verify the correct operation of the library.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3_ಹುಡುಕು_ಎಣಿಕೆ = 0;
#ಆದರೆ_ಅಂತ್ಯ

/*
** When this global variable is positive, it gets decremented once before
** each instruction in the VDBE.  When it reaches zero, the u1.isInterrupted
** field of the sqlite3 structure is set in order to simulate an interrupt.
**
** This facility is used for testing purposes only.  It does not function
** in an ordinary build.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3_ಅಡಚಣೆ_ಎಣಿಕೆ = 0;
#ಆದರೆ_ಅಂತ್ಯ

/*
** The next global variable is incremented each type the OP_Sort opcode
** is executed.  The test procedures use this information to make sure that
** sorting is occurring or not occurring at appropriate times.   This variable
** has no function other than to help verify the correct operation of the
** library.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3_ವಿಂಗಡಿಸು_ಎಣಿಕೆ = 0;
#ಆದರೆ_ಅಂತ್ಯ

/*
** The next global variable records the size of the largest MEM_Blob
** or MEM_Str that has been used by a VDBE opcode.  The test procedures
** use this information to make sure that the zero-blob functionality
** is working correctly.   This variable has no function other than to
** help verify the correct operation of the library.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3_max_blobsize = 0;
ಸ್ಥಿರ ಶೂನ್ಯ updateMaxBlobsize(Mem *p){
  ಆದರೆ( (p->flags & (MEM_Str|MEM_Blob))!=0 && p->n>sqlite3_max_blobsize ){
    sqlite3_max_blobsize = p->n;
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** This macro evaluates to true if either the update hook or the preupdate
** hook are enabled for database connect DB.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
# define HAS_UPDATE_HOOK(DB) ((DB)->xPreUpdateCallback||(DB)->xUpdateCallback)
#ಇಲ್ಲದಿದ್ದರೆ
# define HAS_UPDATE_HOOK(DB) ((DB)->xUpdateCallback)
#ಆದರೆ_ಅಂತ್ಯ

/*
** The next global variable is incremented each time the OP_Found opcode
** is executed. This is used to test whether or not the foreign key
** operation implemented using OP_FkIsZero is working. This variable
** has no function other than to help verify the correct operation of the
** library.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3_ಸಿಕ್ಕಿತು_ಎಣಿಕೆ = 0;
#ಆದರೆ_ಅಂತ್ಯ

/*
** Test a register to see if it exceeds the current maximum blob size.
** If it does, record the new maximum blob size.
*/
#ಆದರೆ defined(SQLITE_TEST) && !defined(SQLITE_UNTESTABLE)
# define UPDATE_MAX_BLOBSIZE(P)  updateMaxBlobsize(P)
#ಇಲ್ಲದಿದ್ದರೆ
# define UPDATE_MAX_BLOBSIZE(P)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/* This routine provides a convenient place to set a breakpoint during
** tracing with PRAGMA vdbe_trace=on.  The breakpoint fires right after
** each opcode is printed.  Variables "pc" (program counter) and pOp are
** available to add conditionals to the breakpoint.  GDB example:
**
**         break test_trace_breakpoint if pc=22
**
** Other useful labels for breakpoints include:
**   test_addop_breakpoint(pc,pOp)
**   sqlite3CorruptError(lineno)
**   sqlite3MisuseError(lineno)
**   sqlite3CantopenError(lineno)
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_trace_breakpoint(ಪೂರ್ಣಾಂಕ pc, Op *pOp, Vdbe *v){
  ಸ್ಥಿರ u64 n = 0;
  (ಶೂನ್ಯ)pc;
  (ಶೂನ್ಯ)pOp;
  (ಶೂನ್ಯ)v;
  n++;
  ಆದರೆ( n==ಅತಿದೊಡ್ಡ_UINT64 ) abort(); /* So that n is used, preventing a warning */
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Invoke the VDBE coverage callback, if that callback is defined.  This
** feature is used for test suite validation only and does not appear an
** production builds.
**
** M is the type of branch.  I is the direction taken for this instance of
** the branch.
**
**   M: 2 - two-way branch (I=0: fall-thru   1: jump                )
**      3 - two-way + NULL (I=0: fall-thru   1: jump      2: NULL   )
**      4 - OP_Jump        (I=0: jump p1     1: jump p2   2: jump p3)
**
** In other words, if M is 2, then I is either 0 (for fall-through) or
** 1 (for when the branch is taken).  If M is 3, the I is 0 for an
** ordinary fall-through, I is 1 if the branch was taken, and I is 2
** if the result of comparison is NULL.  For M=3, I=2 the jump may or
** may not be taken, depending on the SQLITE_JUMPIFNULL flags in p5.
** When M is 4, that means that an OP_Jump is being run.  I is 0, 1, or 2
** depending on if the operands are less than, equal, or greater than.
**
** iSrcLine is the source code line (from the __LINE__ macro) that
** generated the VDBE instruction combined with flag bits.  The source
** code line number is in the lower 24 bits of iSrcLine and the upper
** 8 bytes are flags.  The lower three bits of the flags indicate
** values for I that should never occur.  For example, if the branch is
** always taken, the flags should be 0x05 since the fall-through and
** alternate branch are never taken.  If a branch is never taken then
** flags should be 0x06 since only the fall-through approach is allowed.
**
** Bit 0x08 of the flags indicates an OP_Jump opcode that is only
** interested in equal or not-equal.  In other words, I==0 and I==2
** should be treated as equivalent
**
** Since only a line number is retained, not the filename, this macro
** only works for amalgamation builds.  But that is ok, since these macros
** should be no-ops except for special builds used to measure test coverage.
*/
#ಆದರೆ !defined(SQLITE_VDBE_COVERAGE)
# define Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(I,M)
#ಇಲ್ಲದಿದ್ದರೆ
# define Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(I,M) vdbeTakeBranch(pOp->i_Src_ಸಾಲು,I,M)
  ಸ್ಥಿರ ಶೂನ್ಯ vdbeTakeBranch(u32 i_Src_ಸಾಲು, u8 I, u8 M){
    u8 mNever;
    ಪ್ರತಿಜ್ಞಾ( I<=2 );  /* 0: fall through,  1: taken,  2: alternate taken */
    ಪ್ರತಿಜ್ಞಾ( M<=4 );  /* 2: two-way branch, 3: three-way branch, 4: OP_Jump */
    ಪ್ರತಿಜ್ಞಾ( I<M );   /* I can only be 2 if M is 3 or 4 */
    /* Transform I from a integer [0,1,2] into a bitmask of [1,2,4] */
    I = 1<<I;
    /* The upper 8 bits of iSrcLine are flags.  The lower three bits of
    ** the flags indicate directions that the branch can never go.  If
    ** a branch really does go in one of those directions, assert right
    ** away. */
    mNever = i_Src_ಸಾಲು >> 24;
    ಪ್ರತಿಜ್ಞಾ( (I & mNever)==0 );
    ಆದರೆ( sqlite3GlobalConfig.xVdbeBranch==0 ) ಹಿಂತಿರುಗಿ;  /*NO_TEST*/
    /* Invoke the branch coverage callback with three arguments:
    **    iSrcLine - the line number of the VdbeCoverage() macro, with
    **               flags removed.
    **    I        - Mask of bits 0x07 indicating which cases are are
    **               fulfilled by this instance of the jump.  0x01 means
    **               fall-thru, 0x02 means taken, 0x04 means NULL.  Any
    **               impossible cases (ex: if the comparison is never NULL)
    **               are filled in automatically so that the coverage
    **               measurement logic does not flag those impossible cases
    **               as missed coverage.
    **    M        - Type of jump.  Same as M argument above
    */
    I |= mNever;
    ಆದರೆ( M==2 ) I |= 0x04;
    ಆದರೆ( M==4 ){
      I |= 0x08;
      ಆದರೆ( (mNever&0x08)!=0 && (I&0x05)!=0) I |= 0x05; /*NO_TEST*/
    }
    sqlite3GlobalConfig.xVdbeBranch(sqlite3GlobalConfig.pVdbeBranchArg,
                                    i_Src_ಸಾಲು&0xffffff, I, M);
  }
#ಆದರೆ_ಅಂತ್ಯ

/*
** An ephemeral string value (signified by the MEM_Ephem flag) contains
** a pointer to a dynamically allocated string where some other entity
** is responsible for deallocating that string.  Because the register
** does not control the string, it might be deleted without the register
** knowing it.
**
** This routine converts an ephemeral string into a dynamically allocated
** string that the register itself controls.  In other words, it
** converts an MEM_Ephem string into a string with P.z==P.zMalloc.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು Deephemeralize(P) \
   ಆದರೆ( ((P)->flags&MEM_Ephem)!=0 \
       && sqlite3VdbeMemMakeWriteable(P) ){ ಹೋಗು no_mem;}

/* Return true if the cursor was opened using the OP_OpenSorter opcode. */
#ವ್ಯಾಖ್ಯಾನಿಸು isSorter(x) ((x)->eCurType==CURTYPE_SORTER)

/*
** Allocate VdbeCursor number iCur.  Return a pointer to it.  Return NULL
** if we run out of memory.
*/
ಸ್ಥಿರ VdbeCursor *allocateCursor(
  Vdbe *p,              /* The virtual machine */
  ಪೂರ್ಣಾಂಕ iCur,             /* Index of the new VdbeCursor */
  ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ,           /* Number of fields in the table or index */
  u8 eCurType           /* Type of the new cursor */
){
  /* Find the memory cell that will be used to store the blob of memory
  ** required for this VdbeCursor structure. It is convenient to use a
  ** vdbe memory cell to manage the memory allocation required for a
  ** VdbeCursor structure for the following reasons:
  **
  **   * Sometimes cursor numbers are used for a couple of different
  **     purposes in a vdbe program. The different uses might require
  **     different sized allocations. Memory cells provide growable
  **     allocations.
  **
  **   * When using ENABLE_MEMORY_MANAGEMENT, memory cell buffers can
  **     be freed lazily via the sqlite3_release_memory() API. This
  **     minimizes the number of malloc calls made by the system.
  **
  ** The memory cell for cursor 0 is aMem[0]. The rest are allocated from
  ** the top of the register space.  Cursor 1 is at Mem[p->nMem-1].
  ** Cursor 2 is at Mem[p->nMem-2]. And so forth.
  */
  Mem *pMem = iCur>0 ? &p->aMem[p->nMem-iCur] : p->aMem;

  i64 nByte;
  VdbeCursor *pCx = 0;
  nByte = SZ_VDBECURSOR(n_ಕ್ಷೇತ್ರ);
  ಪ್ರತಿಜ್ಞಾ( ROUND8(nByte)==nByte );
  ಆದರೆ( eCurType==CURTYPE_BTREE ) nByte += sqlite3_Btree_Cursor_ಗಾತ್ರ();

  ಪ್ರತಿಜ್ಞಾ( iCur>=0 && iCur<p->nCursor );
  ಆದರೆ( p->apCsr[iCur] ){ /*OPTIMIZATION-IF-FALSE*/
    sqlite3VdbeFreeCursorNN(p, p->apCsr[iCur]);
    p->apCsr[iCur] = 0;
  }

  /* There used to be a call to sqlite3VdbeMemClearAndResize() to make sure
  ** the pMem used to hold space for the cursor has enough storage available
  ** in pMem->zMalloc.  But for the special case of the aMem[] entries used
  ** to hold cursors, it is faster to in-line the logic. */
  ಪ್ರತಿಜ್ಞಾ( pMem->flags==MEM_Undefined );
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Dyn)==0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->szMalloc==0 || pMem->z==pMem->zMalloc );
  ಆದರೆ( pMem->szMalloc<nByte ){
    ಆದರೆ( pMem->szMalloc>0 ){
      sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    }
    pMem->z = pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, nByte);
    ಆದರೆ( pMem->zMalloc==0 ){
      pMem->szMalloc = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    pMem->szMalloc = (ಪೂರ್ಣಾಂಕ)nByte;
  }

  p->apCsr[iCur] = pCx = (VdbeCursor*)pMem->zMalloc;
  memset(pCx, 0, offsetof(VdbeCursor,pAltCursor));
  pCx->eCurType = eCurType;
  pCx->n_ಕ್ಷೇತ್ರ = n_ಕ್ಷೇತ್ರ;
  pCx->a_ಸ್ಥಾನಾಂತರ = &pCx->aType[n_ಕ್ಷೇತ್ರ];
  ಆದರೆ( eCurType==CURTYPE_BTREE ){
    ಪ್ರತಿಜ್ಞಾ( ROUND8(SZ_VDBECURSOR(n_ಕ್ಷೇತ್ರ))==SZ_VDBECURSOR(n_ಕ್ಷೇತ್ರ) );
    pCx->uc.pCursor = (BtCursor*)&pMem->z[SZ_VDBECURSOR(n_ಕ್ಷೇತ್ರ)];
    sqlite3BtreeCursorZero(pCx->uc.pCursor);
  }
  ಹಿಂತಿರುಗಿ pCx;
}

/*
** The string in pRec is known to look like an integer and to have a
** floating point value of rValue.  Return true and set *piValue to the
** integer value if the string is in range to be an integer.  Otherwise,
** return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ alsoAnInt(Mem *pRec, ದ್ವಿಗುಣ r_ಮೌಲ್ಯ, i64 *pi_ಮೌಲ್ಯ){
  i64 i_ಮೌಲ್ಯ;
  i_ಮೌಲ್ಯ = sqlite3_Real_ಗಮ್ಯ_I64(r_ಮೌಲ್ಯ);
  ಆದರೆ( sqlite3RealSameAsInt(r_ಮೌಲ್ಯ,i_ಮೌಲ್ಯ) ){
    *pi_ಮೌಲ್ಯ = i_ಮೌಲ್ಯ;
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0==sqlite3Atoi64(pRec->z, pi_ಮೌಲ್ಯ, pRec->n, pRec->enc);
}

/*
** Try to convert a value into a numeric representation if we can
** do so without loss of information.  In other words, if the string
** looks like a number, convert it into a number.  If it does not
** look like a number, leave it alone.
**
** If the bTryForInt flag is true, then extra effort is made to give
** an integer representation.  Strings that look like floating point
** values but which have no fractional component (example: '48.00')
** will have a MEM_Int representation when bTryForInt is true.
**
** If bTryForInt is false, then if the input string contains a decimal
** point or exponential notation, the result is only MEM_Real, even
** if there is an exact integer representation of the quantity.
*/
ಸ್ಥಿರ ಶೂನ್ಯ applyNumericAffinity(Mem *pRec, ಪೂರ್ಣಾಂಕ bTryForInt){
  ದ್ವಿಗುಣ r_ಮೌಲ್ಯ;
  ಪೂರ್ಣಾಂಕ rc;
  ಪ್ರತಿಜ್ಞಾ( (pRec->flags & (MEM_Str|MEM_Int|MEM_Real|MEM_IntReal))==MEM_Str );
  rc = sqlite3_Mem_Real_ಮೌಲ್ಯ_RC(pRec, &r_ಮೌಲ್ಯ);
  ಆದರೆ( rc<=0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( (rc&2)==0 && alsoAnInt(pRec, r_ಮೌಲ್ಯ, &pRec->u.i) ){
    pRec->flags |= MEM_Int;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pRec->u.r = r_ಮೌಲ್ಯ;
    pRec->flags |= MEM_Real;
    ಆದರೆ( bTryForInt ) sqlite3VdbeIntegerAffinity(pRec, 1);
  }
  /* TEXT->NUMERIC is many->one.  Hence, it is important to invalidate the
  ** string representation after computing a numeric equivalent, because the
  ** string representation might not be the canonical representation for the
  ** numeric value.  Ticket [343634942dd54ab57b7024] 2018-01-31. */
  pRec->flags &= ~MEM_Str;
}

/*
** Processing is determine by the affinity parameter:
**
** SQLITE_AFF_INTEGER:
** SQLITE_AFF_REAL:
** SQLITE_AFF_NUMERIC:
**    Try to convert pRec to an integer representation or a
**    floating-point representation if an integer representation
**    is not possible.  Note that the integer representation is
**    always preferred, even if the affinity is REAL, because
**    an integer representation is more space efficient on disk.
**
** SQLITE_AFF_FLEXNUM:
**    If the value is text, then try to convert it into a number of
**    some kind (integer or real) but do not make any other changes.
**
** SQLITE_AFF_TEXT:
**    Convert pRec to a text representation.
**
** SQLITE_AFF_BLOB:
** SQLITE_AFF_NONE:
**    No-op.  pRec is unchanged.
*/
ಸ್ಥಿರ ಶೂನ್ಯ applyAffinity(
  Mem *pRec,          /* The value to apply affinity to */
  ಅಕ್ಷರ affinity,      /* The affinity to be applied */
  u8 enc              /* Use this text encoding */
){
  ಆದರೆ( affinity>=SQLITE_AFF_NUMERIC ){
    ಪ್ರತಿಜ್ಞಾ( affinity==SQLITE_AFF_INTEGER || affinity==SQLITE_AFF_REAL
             || affinity==SQLITE_AFF_NUMERIC || affinity==SQLITE_AFF_FLEXNUM );
    ಆದರೆ( (pRec->flags & MEM_Int)==0 ){ /*OPTIMIZATION-IF-FALSE*/
      ಆದರೆ( (pRec->flags & (MEM_Real|MEM_IntReal))==0 ){
        ಆದರೆ( pRec->flags & MEM_Str ) applyNumericAffinity(pRec,1);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( affinity<=SQLITE_AFF_REAL ){
        sqlite3VdbeIntegerAffinity(pRec, 0);
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( affinity==SQLITE_AFF_ಪಠ್ಯ ){
    /* Only attempt the conversion to TEXT if there is an integer or real
    ** representation (blob and NULL do not get converted) but no string
    ** representation.  It would be harmless to repeat the conversion if
    ** there is already a string rep, but it is pointless to waste those
    ** CPU cycles. */
    ಆದರೆ( 0==(pRec->flags&MEM_Str) ){ /*OPTIMIZATION-IF-FALSE*/
      ಆದರೆ( (pRec->flags&(MEM_Real|MEM_Int|MEM_IntReal)) ){
        testcase( pRec->flags & MEM_Int );
        testcase( pRec->flags & MEM_Real );
        testcase( pRec->flags & MEM_IntReal );
        sqlite3VdbeMemStringify(pRec, enc, 1);
      }
    }
    pRec->flags &= ~(MEM_Real|MEM_Int|MEM_IntReal);
  }
}

/*
** Try to convert the type of a function argument or a result column
** into a numeric representation.  Use either INTEGER or REAL whichever
** is appropriate.  But only do the conversion if it is possible without
** loss of information and return the revised type of the argument.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_numeric_type(sqlite3_ಮೌಲ್ಯ *pVal){
  ಪೂರ್ಣಾಂಕ eType = sqlite3_ಮೌಲ್ಯ_type(pVal);
  ಆದರೆ( eType==SQLITE_ಪಠ್ಯ ){
    Mem *pMem = (Mem*)pVal;
#ಆದರೆ SQLITE_THREADSAFE>0
    sqlite3_mutex *pMutex = pMem->db ? pMem->db->mutex : 0;
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_mutex_enter(pMutex);
    applyNumericAffinity(pMem, 0);
    sqlite3_mutex_leave(pMutex);
    eType = sqlite3_ಮೌಲ್ಯ_type(pVal);
  }
  ಹಿಂತಿರುಗಿ eType;
}

/*
** Exported version of applyAffinity(). This one works on sqlite3_value*,
** not the internal Mem* type.
*/
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_Apply_Affinity(
  sqlite3_ಮೌಲ್ಯ *pVal,
  u8 affinity,
  u8 enc
){
  applyAffinity((Mem *)pVal, affinity, enc);
}

/*
** pMem currently only holds a string type (or maybe a BLOB that we can
** interpret as a string if we want to).  Compute its corresponding
** numeric type, if has one.  Set the pMem->u.r and pMem->u.i fields
** accordingly.
*/
ಸ್ಥಿರ u16 SQLITE_NOINLINE computeNumericType(Mem *pMem){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_int64 ix;
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 );
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & (MEM_Str|MEM_Blob))!=0 );
  ಆದರೆ( ExpandBlob(pMem) ){
    pMem->u.i = 0;
    ಹಿಂತಿರುಗಿ MEM_Int;
  }
  rc = sqlite3_Mem_Real_ಮೌಲ್ಯ_RC(pMem, &pMem->u.r);
  ಆದರೆ( rc<=0 ){
    ಆದರೆ( (rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<=1 ){
      pMem->u.i = ix;
      ಹಿಂತಿರುಗಿ MEM_Int;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ MEM_Real;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)==0 ){
    pMem->u.i = ix;
    ಹಿಂತಿರುಗಿ MEM_Int;
  }
  ಹಿಂತಿರುಗಿ MEM_Real;
}

/*
** Return the numeric type for pMem, either MEM_Int or MEM_Real or both or
** none. 
**
** Unlike applyNumericAffinity(), this routine does not modify pMem->flags.
** But it does set pMem->u.r and pMem->u.i appropriately.
*/
ಸ್ಥಿರ u16 numericType(Mem *pMem){
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Null)==0
       || pMem->db==0 || pMem->db->mallocFailed );
  ಆದರೆ( pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null) ){
    testcase( pMem->flags & MEM_Int );
    testcase( pMem->flags & MEM_Real );
    testcase( pMem->flags & MEM_IntReal );
    ಹಿಂತಿರುಗಿ pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null);
  }
  ಪ್ರತಿಜ್ಞಾ( pMem->flags & (MEM_Str|MEM_Blob) );
  testcase( pMem->flags & MEM_Str );
  testcase( pMem->flags & MEM_Blob );
  ಹಿಂತಿರುಗಿ computeNumericType(pMem);
  ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Write a nice string representation of the contents of cell pMem
** into buffer zBuf, length nBuf.
*/
ಶೂನ್ಯ sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr){
  ಪೂರ್ಣಾಂಕ f = pMem->flags;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ encnames[] = {"(X)", "(8)", "(16LE)", "(16BE)"};
  ಆದರೆ( f&MEM_Blob ){
    ಪೂರ್ಣಾಂಕ i;
    ಅಕ್ಷರ c;
    ಆದರೆ( f & MEM_Dyn ){
      c = 'z';
      ಪ್ರತಿಜ್ಞಾ( (f & (MEM_Static|MEM_Ephem))==0 );
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( f & MEM_Static ){
      c = 't';
      ಪ್ರತಿಜ್ಞಾ( (f & (MEM_Dyn|MEM_Ephem))==0 );
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( f & MEM_Ephem ){
      c = 'e';
      ಪ್ರತಿಜ್ಞಾ( (f & (MEM_Static|MEM_Dyn))==0 );
    }ಇಲ್ಲದಿದ್ದರೆ{
      c = 's';
    }
    sqlite3_str_appendf(pStr, "%cx[", c);
    ಗಾಗಿ(i=0; i<25 && i<pMem->n; i++){
      sqlite3_str_appendf(pStr, "%02X", ((ಪೂರ್ಣಾಂಕ)pMem->z[i] & 0xFF));
    }
    sqlite3_str_appendf(pStr, "|");
    ಗಾಗಿ(i=0; i<25 && i<pMem->n; i++){
      ಅಕ್ಷರ z = pMem->z[i];
      sqlite3_str_appendchar(pStr, 1, (z<32||z>126)?'.':z);
    }
    sqlite3_str_appendf(pStr,"]");
    ಆದರೆ( f & MEM_Zero ){
      sqlite3_str_appendf(pStr, "+%dz",pMem->u.nZero);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( f & MEM_Str ){
    ಪೂರ್ಣಾಂಕ j;
    u8 c;
    ಆದರೆ( f & MEM_Dyn ){
      c = 'z';
      ಪ್ರತಿಜ್ಞಾ( (f & (MEM_Static|MEM_Ephem))==0 );
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( f & MEM_Static ){
      c = 't';
      ಪ್ರತಿಜ್ಞಾ( (f & (MEM_Dyn|MEM_Ephem))==0 );
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( f & MEM_Ephem ){
      c = 'e';
      ಪ್ರತಿಜ್ಞಾ( (f & (MEM_Static|MEM_Dyn))==0 );
    }ಇಲ್ಲದಿದ್ದರೆ{
      c = 's';
    }
    sqlite3_str_appendf(pStr, " %c%d[", c, pMem->n);
    ಗಾಗಿ(j=0; j<25 && j<pMem->n; j++){
      c = pMem->z[j];
      sqlite3_str_appendchar(pStr, 1, (c>=0x20&&c<=0x7f) ? c : '.');
    }
    sqlite3_str_appendf(pStr, "]%s", encnames[pMem->enc]);
    ಆದರೆ( f & MEM_ಪದ ){
      sqlite3_str_appendf(pStr, "(0-term)");
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Print the value of a register for tracing purposes:
*/
ಸ್ಥಿರ ಶೂನ್ಯ memTracePrint(Mem *p){
  ಆದರೆ( p->flags & MEM_Undefined ){
    ಬರೆ(" undefined");
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->flags & MEM_Null ){
    ಬರೆ(p->flags & MEM_Zero ? " NULL-nochng" : " NULL");
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (p->flags & (MEM_Int|MEM_Str))==(MEM_Int|MEM_Str) ){
    ಬರೆ(" si:%lld", p->u.i);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (p->flags & (MEM_IntReal))!=0 ){
    ಬರೆ(" ir:%lld", p->u.i);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->flags & MEM_Int ){
    ಬರೆ(" i:%lld", p->u.i);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->flags & MEM_Real ){
    ಬರೆ(" r:%.17g", p->u.r);
#ಆದರೆ_ಅಂತ್ಯ
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(p) ){
    ಬರೆ(" (rowset)");
  }ಇಲ್ಲದಿದ್ದರೆ{
    StrAccum acc;
    ಅಕ್ಷರ zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, ಗಾತ್ರ(zBuf), 0);
    sqlite3VdbeMemPrettyPrint(p, &acc);
    ಬರೆ(" %s", sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc));
  }
  ಆದರೆ( p->flags & MEM_Subtype ) ಬರೆ(" subtype=0x%02x", p->eSubtype);
}
ಸ್ಥಿರ ಶೂನ್ಯ ರಿಜಿಸ್ಟರ್_Trace(ಪೂರ್ಣಾಂಕ iReg, Mem *p){
  ಬರೆ("R[%d] = ", iReg);
  memTracePrint(p);
  ಆದರೆ( p->pScopyFrom ){
    ಪ್ರತಿಜ್ಞಾ( p->pScopyFrom->bScopy );
    ಬರೆ(" <== R[%d]", (ಪೂರ್ಣಾಂಕ)(p->pScopyFrom - &p[-iReg]));
  }
  ಬರೆ("\n");
  sqlite3VdbeCheckMemInvariants(p);
}
/**/ ಶೂನ್ಯ sqlite3PrintMem(Mem *pMem){
  memTracePrint(pMem);
  ಬರೆ("\n");
  fflush(stdout);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Show the values of all registers in the virtual machine.  Used for
** interactive debugging.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ರಿಜಿಸ್ಟರ್_Dump(Vdbe *v){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=1; i<v->nMem; i++) ರಿಜಿಸ್ಟರ್_Trace(i, v->aMem+i);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
#  define ರಿಜಿಸ್ಟರ್_TRACE(R,M) ಆದರೆ(db->flags&SQLITE_VdbeTrace)ರಿಜಿಸ್ಟರ್_Trace(R,M)
#ಇಲ್ಲದಿದ್ದರೆ
#  define ರಿಜಿಸ್ಟರ್_TRACE(R,M)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
/*
** This function is only called from within an assert() expression. It
** checks that the sqlite3.nTransaction variable is correctly set to
** the number of non-transaction savepoints currently in the
** linked list starting at sqlite3.pSavepoint.
**
** Usage:
**
**     assert( checkSavepointCount(db) );
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_Savepoint_ಎಣಿಕೆ(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ n = 0;
  Savepoint *p;
  ಗಾಗಿ(p=db->pSavepoint; p; p=p->p_ಮುಂದಿನ) n++;
  ಪ್ರತಿಜ್ಞಾ( n==(db->nSavepoint + db->isTransactionSavepoint) );
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return the register of pOp->p2 after first preparing it to be
** overwritten with an integer value.
*/
ಸ್ಥಿರ SQLITE_NOINLINE Mem *out2PrereleaseWithClear(Mem *p_ಹೊರಹೋಗು){
  sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
  p_ಹೊರಹೋಗು->flags = MEM_Int;
  ಹಿಂತಿರುಗಿ p_ಹೊರಹೋಗು;
}
ಸ್ಥಿರ Mem *out2Prerelease(Vdbe *p, VdbeOp *pOp){
  Mem *p_ಹೊರಹೋಗು;
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2<=(p->nMem+1 - p->nCursor) );
  p_ಹೊರಹೋಗು = &p->aMem[pOp->p2];
  mem_About_ಗಮ್ಯ_Change(p, p_ಹೊರಹೋಗು);
  ಆದರೆ( VdbeMemDynamic(p_ಹೊರಹೋಗು) ){ /*OPTIMIZATION-IF-FALSE*/
    ಹಿಂತಿರುಗಿ out2PrereleaseWithClear(p_ಹೊರಹೋಗು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಹೊರಹೋಗು->flags = MEM_Int;
    ಹಿಂತಿರುಗಿ p_ಹೊರಹೋಗು;
  }
}

/*
** Compute a bloom filter hash using pOp->p4.i registers from aMem[] beginning
** with pOp->p3.  Return the hash.
**
** IMPORTANT RESTRICTION (tag-202607231411):  This hash is only valid if the
** collating sequence for TEXT is BINARY. Hence, Bloom filters that use this
** hash will not work for look-ups that use any other collating sequence.
*/
ಸ್ಥಿರ u64 filterHash(ಸ್ಥಿರಾಂಕ Mem *aMem, ಸ್ಥಿರಾಂಕ Op *pOp){
  ಪೂರ್ಣಾಂಕ i, mx;
  u64 h = 0;

  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INT32 );
  ಗಾಗಿ(i=pOp->p3, mx=i+pOp->p4.i; i<mx; i++){
    ಸ್ಥಿರಾಂಕ Mem *p = &aMem[i];
    ಆದರೆ( p->flags & (MEM_Int|MEM_IntReal) ){
      h += p->u.i;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->flags & MEM_Real ){
      h += sqlite3_Vdbe_Int_ಮೌಲ್ಯ(p);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->flags & MEM_Str ){
      u64 x;
      h += p->n;
      ಆದರೆ( p->n >= (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(x) ){
        ಸ್ಮರಣೆ_ನಕಲು(&x, p->z, ಗಾತ್ರ(x));
        h += x;
        ಸ್ಮರಣೆ_ನಕಲು(&x, p->z + p->n - ಗಾತ್ರ(x), ಗಾತ್ರ(x));
        h += x;
      }ಇಲ್ಲದಿದ್ದರೆ{
        x = 0;
        ಸ್ಮರಣೆ_ನಕಲು(&x, p->z, p->n);
        h += x;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->flags & MEM_Blob ){
      ಪೂರ್ಣಾಂಕ n = p->n;
      u64 x = 0;
      ಆದರೆ( n ){
        ಸ್ಮರಣೆ_ನಕಲು(&x, p->z, MIN(n, (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(x)));
        h += x;
      }
      h += n;
      ಆದರೆ( p->flags & MEM_Zero ) h += p->u.nZero;
    }
  }
  ಹಿಂತಿರುಗಿ h;
}

/*
** For OP_Column, factor out the case where content is loaded from
** overflow pages, so that the code to implement this case is separate
** the common case where all content fits on the page.  Factoring out
** the code reduces register pressure and helps the common case
** to run faster.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ vdbe_ಸ್ತಂಭ_From_ಅತಿಪ್ರವಾಹ(
  VdbeCursor *pC,       /* The BTree cursor from which we are reading */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು,             /* The column to read */
  u32 t,                /* The serial-type code for the column value */
  i64 i_ಸ್ಥಾನಾಂತರ,          /* Offset to the start of the content value */
  u32 cache_ಸ್ಥಿತಿ,      /* Current Vdbe.cacheCtr value */
  u32 ಲಂಬಸಾಲು_Cache_Ctr,      /* Current value of the column cache counter */
  Mem *pDest            /* Store the value into this register. */
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3 *db = pDest->db;
  ಪೂರ್ಣಾಂಕ encoding = pDest->enc;
  ಪೂರ್ಣಾಂಕ len = sqlite3VdbeSerialTypeLen(t);
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಆದರೆ( len>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ) ಹಿಂತಿರುಗಿ SQLITE_TOOBIG;
  ಆದರೆ( len > 4000 && pC->p_ಕೀಲಿ_Info==0 ){
    /* Cache large column values that are on overflow pages using
    ** an RCStr (reference counted string) so that if they are reloaded,
    ** that do not have to be copied a second time.  The overhead of
    ** creating and managing the cache is such that this is only
    ** profitable for larger TEXT and BLOB values.
    **
    ** Only do this on table-btrees so that writes to index-btrees do not
    ** need to clear the cache.  This buys performance in the common case
    ** in exchange for generality.
    */
    VdbeTxtBlbCache *pCache;
    ಅಕ್ಷರ *pBuf;
    ಆದರೆ( pC->ಲಂಬಸಾಲು_Cache==0 ){
      pC->pCache = sqlite3DbMallocZero(db, ಗಾತ್ರ(VdbeTxtBlbCache) );
      ಆದರೆ( pC->pCache==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      pC->ಲಂಬಸಾಲು_Cache = 1;
    }
    pCache = pC->pCache;
    ಆದರೆ( pCache->pCValue==0
     || pCache->i_ಲಂಬಸಾಲು!=i_ಲಂಬಸಾಲು
     || pCache->cache_ಸ್ಥಿತಿ!=cache_ಸ್ಥಿತಿ
     || pCache->ಲಂಬಸಾಲು_Cache_Ctr!=ಲಂಬಸಾಲು_Cache_Ctr
     || pCache->i_ಸ್ಥಾನಾಂತರ!=sqlite3_Btree_ಸ್ಥಾನಾಂತರ(pC->uc.pCursor)
    ){
      ಆದರೆ( pCache->pCValue ) sqlite3RCStrUnref(pCache->pCValue);
      pBuf = pCache->pCValue = sqlite3RCStrNew( len+3 );
      ಆದರೆ( pBuf==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      rc = sqlite3BtreePayload(pC->uc.pCursor, i_ಸ್ಥಾನಾಂತರ, len, pBuf);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
      pBuf[len] = 0;
      pBuf[len+1] = 0;
      pBuf[len+2] = 0;
      pCache->i_ಲಂಬಸಾಲು = i_ಲಂಬಸಾಲು;
      pCache->cache_ಸ್ಥಿತಿ = cache_ಸ್ಥಿತಿ;
      pCache->ಲಂಬಸಾಲು_Cache_Ctr = ಲಂಬಸಾಲು_Cache_Ctr;
      pCache->i_ಸ್ಥಾನಾಂತರ = sqlite3_Btree_ಸ್ಥಾನಾಂತರ(pC->uc.pCursor);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pBuf = pCache->pCValue;
    }
    ಪ್ರತಿಜ್ಞಾ( t>=12 );
    sqlite3RCStrRef(pBuf);
    ಆದರೆ( t&1 ){
      /* Prevents the removal of a BOM in UTF16 text.  Otherwise
      ** it causes no harm as long as len>=0, and pDest->enc is set
      ** to the correct encoding afterwards: ----
      ** (tag-20261016-1)                        `vvvvvvvvvvv    */
      rc = sqlite3_Vdbe_Mem_ಗಣ_Str(pDest, pBuf, len, SQLITE_UTF8,
                                sqlite3RCStrUnref);
      pDest->flags |= MEM_ಪದ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = sqlite3_Vdbe_Mem_ಗಣ_Str(pDest, pBuf, len, 0,
                                sqlite3RCStrUnref);
     }
    pDest->enc = encoding;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3VdbeMemFromBtree(pC->uc.pCursor, i_ಸ್ಥಾನಾಂತರ, len, pDest);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    sqlite3VdbeSerialGet((ಸ್ಥಿರಾಂಕ u8*)pDest->z, t, pDest);
    ಆದರೆ( (t&1)!=0 && encoding==SQLITE_UTF8 ){
      pDest->z[len] = 0;
      pDest->flags |= MEM_ಪದ;
    }
  }
  pDest->flags &= ~MEM_Ephem;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Memory cell pMem may contain a blob or a NULL value. Cursor pCsr is
** open on an index. If the current index entry matches the blob value in
** pMem byte-for-byte, set pMem to NULL and return 1. Otherwise, return 0.
**
** If an error occurs, set (*pRc) to an SQLite error code. Return 1 in this
** case as well.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ vdbe_ಸೂಚ್ಯಂಕ_ಕೀಲಿ_Compare(
  BtCursor *pCsr,                 /* Cursor to compare key to */
  Mem *pMem,
  ಪೂರ್ಣಾಂಕ *pRc
){
  ಪೂರ್ಣಾಂಕ ret = 0;
  u32 n_ಕೀಲಿ = 0;

  ಪ್ರತಿಜ್ಞಾ( pMem->flags & (MEM_Blob|MEM_Null) );
  n_ಕೀಲಿ = sqlite3_Btree_Payload_ಗಾತ್ರ(pCsr);
  ಆದರೆ( n_ಕೀಲಿ==(u32)pMem->n && ALWAYS((pMem->flags & MEM_Blob)!=0) ){
    /* This code could just use sqlite3BtreePayloadFetch(). But calling that
    ** function here apparently prevents compilers from inlining it in other,
    ** more performance critical, places. So this code uses
    ** MemFromBtreeZeroOffset(), which is just as fast in most cases, but also
    ** handles the case where the index record uses overflow pages. */
    Mem m;
    memset(&m, 0, ಗಾತ್ರ(m));
    *pRc = sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(pCsr, n_ಕೀಲಿ, &m);
    ret = (*pRc!=SQLITE_OK || 0==memcmp(pMem->z, m.z, n_ಕೀಲಿ));
    sqlite3VdbeMemReleaseMalloc(&m);
  }

  ಹಿಂತಿರುಗಿ ret;
}

/*
** Send a "statement aborts" message to the error log.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ sqlite3VdbeLogAbort(
  Vdbe *p,     /* The statement that is running at the time of failure */
  ಪೂರ್ಣಾಂಕ rc,      /* Error code */
  Op *pOp,     /* Opcode that filed */
  Op *aOp      /* All opcodes */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = p->zSql;   /* Original SQL text */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪೂರ್ವಪ್ರತ್ಯಯ = "";     /* Prefix added to SQL text */
  ಪೂರ್ಣಾಂಕ pc;                       /* Opcode address */
  ಅಕ್ಷರ zXtra[100];              /* Buffer space to store zPrefix */

  ಆದರೆ( p->p_ಚೌಕಟ್ಟು ){
    ಪ್ರತಿಜ್ಞಾ( aOp[0].opcode==OP_Init );
    ಆದರೆ( aOp[0].p4.z!=0 ){
      ಪ್ರತಿಜ್ಞಾ( aOp[0].p4.z[0]=='-' 
           && aOp[0].p4.z[1]=='-' 
           && aOp[0].p4.z[2]==' ' );
      sqlite3_snprintf(ಗಾತ್ರ(zXtra), zXtra,"/* %s */ ",aOp[0].p4.z+3);
      z_ಪೂರ್ವಪ್ರತ್ಯಯ = zXtra;
    }ಇಲ್ಲದಿದ್ದರೆ{
      z_ಪೂರ್ವಪ್ರತ್ಯಯ = "/* unknown trigger */ ";
    }
  }
  pc = (ಪೂರ್ಣಾಂಕ)(pOp - aOp);
  sqlite3_log(rc, "statement aborts at %d: %s; [%s%s]",
                   pc, p->z_Err_ಸಂದೇಶ, z_ಪೂರ್ವಪ್ರತ್ಯಯ, zSql);
}

/*
** Return the symbolic name for the data type of a pMem
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *vdbeMemTypeName(Mem *pMem){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azTypes[] = {
      /* SQLITE_INTEGER */ "INT",
      /* SQLITE_FLOAT   */ "REAL",
      /* SQLITE_TEXT    */ "TEXT",
      /* SQLITE_BLOB    */ "BLOB",
      /* SQLITE_NULL    */ "NULL"
  };
  ಹಿಂತಿರುಗಿ azTypes[sqlite3_ಮೌಲ್ಯ_type(pMem)-1];
}

/*
** Execute as much of a VDBE program as we can.
** This is the core of sqlite3_step(). 
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeExec(
  Vdbe *p                    /* The VDBE */
){
  Op *aOp = p->aOp;          /* Copy of p->aOp */
  Op *pOp = aOp;             /* Current operation */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  Op *pOrigOp;               /* Value of pOp at the top of the loop */
  ಪೂರ್ಣಾಂಕ nExtraDelete = 0;      /* Verifies FORDELETE and AUXDELETE flags */
  u8 iCompareIsInit = 0;     /* iCompare is initialized */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;        /* Value to return */
  sqlite3 *db = p->db;       /* The database */
  u8 resetSchemaOnFault = 0; /* Reset schema after an error if positive */
  u8 encoding = ENC(db);     /* The database encoding */
  ಪೂರ್ಣಾಂಕ iCompare = 0;          /* Result of last comparison */
  u64 n_Vm_ಹೆಜ್ಜೆ = 0;           /* Number of virtual machine steps */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
  u64 n_ಪ್ರಗತಿ_ಎಲ್ಲೆ;        /* Invoke xProgress() when nVmStep reaches this */
#ಆದರೆ_ಅಂತ್ಯ
  Mem *aMem = p->aMem;       /* Copy of p->aMem */
  Mem *pIn1 = 0;             /* 1st input operand */
  Mem *pIn2 = 0;             /* 2nd input operand */
  Mem *pIn3 = 0;             /* 3rd input operand */
  Mem *p_ಹೊರಹೋಗು = 0;             /* Output operand */
  u32 ಲಂಬಸಾಲು_Cache_Ctr = 0;       /* Column cache counter */
#ಆದರೆ defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  u64 *pn_ಚಕ್ರ = 0;
  ಪೂರ್ಣಾಂಕ b_Stmt_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ = IS_STMT_SCANSTATUS(db)!=0;
#ಆದರೆ_ಅಂತ್ಯ
  /*** INSERT STACK UNION HERE ***/

  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_ಸತತ_STATE );  /* sqlite3_step() verifies this */
  ಆದರೆ( Db_ಮುಖವಾಡ_Non_Zero(p->ಬೀಗ_ಮುಖವಾಡ) ){
    sqlite3VdbeEnter(p);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
  ಆದರೆ( db->x_ಪ್ರಗತಿ ){
    u32 i_ಪೂರ್ವ = p->aCounter[SQLITE_STMTSTATUS_VM_ಹೆಜ್ಜೆ];
    ಪ್ರತಿಜ್ಞಾ( 0 < db->n_ಪ್ರಗತಿ_Ops );
    n_ಪ್ರಗತಿ_ಎಲ್ಲೆ = db->n_ಪ್ರಗತಿ_Ops - (i_ಪೂರ್ವ % db->n_ಪ್ರಗತಿ_Ops);
  }ಇಲ್ಲದಿದ್ದರೆ{
    n_ಪ್ರಗತಿ_ಎಲ್ಲೆ = ಅತಿದೊಡ್ಡ_UINT64;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    ಹೋಗು no_mem;
  }
  ಪ್ರತಿಜ್ಞಾ( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_BUSY );
  testcase( p->rc!=SQLITE_OK );
  p->rc = SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( p->bIsReader || p->readOnly!=0 );
  p->i_Current_ಸಮಯ = 0;
  ಪ್ರತಿಜ್ಞಾ( p->explain==0 );
  db->busyHandler.nBusy = 0;
  ಆದರೆ( Atomic_ತುಂಬು(&db->u1.isInterrupted) ) ಹೋಗು abort_due_ಗಮ್ಯ_ಅಡಚಣೆ;
  sqlite3VdbeIOTraceSql(p);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  sqlite3BeginBenignMalloc();
  ಆದರೆ( p->pc==0
   && (p->db->flags & (SQLITE_VdbeListing|SQLITE_VdbeEQP|SQLITE_VdbeTrace))!=0
  ){
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ once = 1;
    sqlite3VdbePrintSql(p);
    ಆದರೆ( p->db->flags & SQLITE_VdbeListing ){
      ಬರೆ("VDBE Program Listing:\n");
      ಗಾಗಿ(i=0; i<p->nOp; i++){
        sqlite3VdbePrintOp(stdout, i, &aOp[i]);
      }
    }
    ಆದರೆ( p->db->flags & SQLITE_VdbeEQP ){
      ಗಾಗಿ(i=0; i<p->nOp; i++){
        ಆದರೆ( aOp[i].opcode==OP_Explain ){
          ಆದರೆ( once ) ಬರೆ("VDBE Query Plan:\n");
          ಬರೆ("%s\n", aOp[i].p4.z);
          once = 0;
        }
      }
    }
    ಆದರೆ( p->db->flags & SQLITE_VdbeTrace )  ಬರೆ("VDBE Trace:\n");
  }
  sqlite3_ಕೊನೆ_Benign_Malloc();
#ಆದರೆ_ಅಂತ್ಯ
  ಗಾಗಿ(pOp=&aOp[p->pc]; 1; pOp++){
    /* Errors are detected by individual opcodes, with an immediate
    ** jumps to abort_due_to_error. */
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );

    ಪ್ರತಿಜ್ಞಾ( pOp>=aOp && pOp<&aOp[p->nOp]);
    n_Vm_ಹೆಜ್ಜೆ++;

#ಆದರೆ defined(VDBE_PROFILE)
    pOp->nExec++;
    pn_ಚಕ್ರ = &pOp->n_ಚಕ್ರ;
    ಆದರೆ( sqlite3NProfileCnt==0 ) *pn_ಚಕ್ರ -= sqlite3Hwtime();
#ಅಥವಾಆದರೆ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
    ಆದರೆ( b_Stmt_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ ){
      pOp->nExec++;
      pn_ಚಕ್ರ = &pOp->n_ಚಕ್ರ;
      *pn_ಚಕ್ರ -= sqlite3Hwtime();
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* Only allow tracing if SQLITE_DEBUG is defined.
    */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಆದರೆ( db->flags & SQLITE_VdbeTrace ){
      sqlite3VdbePrintOp(stdout, (ಪೂರ್ಣಾಂಕ)(pOp - aOp), pOp);
      test_trace_breakpoint((ಪೂರ್ಣಾಂಕ)(pOp - aOp),pOp,p);
    }
#ಆದರೆ_ಅಂತ್ಯ
     

    /* Check to see if we need to simulate an interrupt.  This only happens
    ** if we have a special test build.
    */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
    ಆದರೆ( sqlite3_ಅಡಚಣೆ_ಎಣಿಕೆ>0 ){
      sqlite3_ಅಡಚಣೆ_ಎಣಿಕೆ--;
      ಆದರೆ( sqlite3_ಅಡಚಣೆ_ಎಣಿಕೆ==0 ){
        sqlite3_ಅಡಚಣೆ(db);
      }
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* Sanity checking on other operands */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    {
      u8 opProperty = sqlite3OpcodeProperty[pOp->opcode];
      ಆದರೆ( (opProperty & OPFLG_IN1)!=0 ){
        ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 );
        ಪ್ರತಿಜ್ಞಾ( pOp->p1<=(p->nMem+1 - p->nCursor) );
        ಪ್ರತಿಜ್ಞಾ( memIsValid(&aMem[pOp->p1]) );
        ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p1]) );
        ರಿಜಿಸ್ಟರ್_TRACE(pOp->p1, &aMem[pOp->p1]);
      }
      ಆದರೆ( (opProperty & OPFLG_IN2)!=0 ){
        ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 );
        ಪ್ರತಿಜ್ಞಾ( pOp->p2<=(p->nMem+1 - p->nCursor) );
        ಪ್ರತಿಜ್ಞಾ( memIsValid(&aMem[pOp->p2]) );
        ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p2]) );
        ರಿಜಿಸ್ಟರ್_TRACE(pOp->p2, &aMem[pOp->p2]);
      }
      ಆದರೆ( (opProperty & OPFLG_IN3)!=0 ){
        ಪ್ರತಿಜ್ಞಾ( pOp->p3>0 );
        ಪ್ರತಿಜ್ಞಾ( pOp->p3<=(p->nMem+1 - p->nCursor) );
        ಪ್ರತಿಜ್ಞಾ( memIsValid(&aMem[pOp->p3]) );
        ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p3]) );
        ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3, &aMem[pOp->p3]);
      }
      ಆದರೆ( (opProperty & OPFLG_OUT2)!=0 ){
        ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 );
        ಪ್ರತಿಜ್ಞಾ( pOp->p2<=(p->nMem+1 - p->nCursor) );
        mem_About_ಗಮ್ಯ_Change(p, &aMem[pOp->p2]);
      }
      ಆದರೆ( (opProperty & OPFLG_OUT3)!=0 ){
        ಪ್ರತಿಜ್ಞಾ( pOp->p3>0 );
        ಪ್ರತಿಜ್ಞಾ( pOp->p3<=(p->nMem+1 - p->nCursor) );
        mem_About_ಗಮ್ಯ_Change(p, &aMem[pOp->p3]);
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    pOrigOp = pOp;
#ಆದರೆ_ಅಂತ್ಯ
 
    ಆಯ್ಕೆ( pOp->opcode ){

/*****************************************************************************
** What follows is a massive switch statement where each case implements a
** separate instruction in the virtual machine.  If we follow the usual
** indentation conventions, each case should be indented by 6 spaces.  But
** that is a lot of wasted space on the left margin.  So the code within
** the switch statement will break with convention and be flush-left. Another
** big comment (similar to this one) will mark the point in the code where
** we transition back to normal indentation.
**
** The formatting of each case is important.  The makefile for SQLite
** generates two C files "opcodes.h" and "opcodes.c" by scanning this
** file looking for lines that begin with "case OP_".  The opcodes.h files
** will be filled with #defines that give unique integer values to each
** opcode and the opcodes.c file is filled with an array of strings where
** each string is the symbolic name for the corresponding opcode.  If the
** case statement is followed by a comment of the form "/# same as ... #/"
** that comment is used to determine the particular value of the opcode.
**
** Other keywords in the comment that follows each case are used to
** construct the OPFLG_INITIALIZER value that initializes opcodeProperty[].
** Keywords include: in1, in2, in3, out2, out3.  See
** the mkopcodeh.awk script for additional information.
**
** Documentation about VDBE opcodes is generated by scanning this file
** for lines of that contain "Opcode:".  That line and all subsequent
** comment lines are used in the generation of the opcode.html documentation
** file.
**
** SUMMARY:
**
**     Formatting is important to scripts that scan this file.
**     Do not deviate from the formatting style currently in use.
**
*****************************************************************************/

/* Opcode:  Goto * P2 * * *
**
** An unconditional jump to address P2.
** The next instruction executed will be
** the one at index P2 from the beginning of
** the program.
**
** The P1 parameter is not actually used by this opcode.  However, it
** is sometimes set to 1 instead of 0 as a hint to the command-line shell
** that this Goto is the bottom of a loop and that the lines from P2 down
** to the current line should be indented for EXPLAIN output.
*/
ಸ್ಥಿತಿ OP_Goto: {             /* jump */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* In debugging mode, when the p5 flags is set on an OP_Goto, that
  ** means we should really jump back to the preceding OP_ReleaseReg
  ** instruction. */
  ಆದರೆ( pOp->p5 ){
    ಪ್ರತಿಜ್ಞಾ( pOp->p2 < (ಪೂರ್ಣಾಂಕ)(pOp - aOp) );
    ಪ್ರತಿಜ್ಞಾ( pOp->p2 > 1 );
    pOp = &aOp[pOp->p2 - 2];
    ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_ReleaseReg );
    ಹೋಗು check_for_ಅಡಚಣೆ;
  }
#ಆದರೆ_ಅಂತ್ಯ

jump_ಗಮ್ಯ_p2_and_check_for_ಅಡಚಣೆ:
  pOp = &aOp[pOp->p2 - 1];

  /* Opcodes that are used as the bottom of a loop (OP_Next, OP_Prev,
  ** OP_VNext, or OP_SorterNext) all jump here upon
  ** completion.  Check to see if sqlite3_interrupt() has been called
  ** or if the progress callback needs to be invoked.
  **
  ** This code uses unstructured "goto" statements and does not look clean.
  ** But that is not due to sloppy coding habits. The code is written this
  ** way for performance, to avoid having to run the interrupt and progress
  ** checks on every opcode.  This helps sqlite3_step() to run about 1.5%
  ** faster according to "valgrind --tool=cachegrind" */
check_for_ಅಡಚಣೆ:
  ಆದರೆ( Atomic_ತುಂಬು(&db->u1.isInterrupted) ) ಹೋಗು abort_due_ಗಮ್ಯ_ಅಡಚಣೆ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
  /* Call the progress callback if it is configured and the required number
  ** of VDBE ops have been executed (either since this invocation of
  ** sqlite3VdbeExec() or since last time the progress callback was called).
  ** If the progress callback returns non-zero, exit the virtual machine with
  ** a return code SQLITE_ABORT.
  */
  ಆಗ( n_Vm_ಹೆಜ್ಜೆ>=n_ಪ್ರಗತಿ_ಎಲ್ಲೆ && db->x_ಪ್ರಗತಿ!=0 ){
    ಪ್ರತಿಜ್ಞಾ( db->n_ಪ್ರಗತಿ_Ops!=0 );
    n_ಪ್ರಗತಿ_ಎಲ್ಲೆ += db->n_ಪ್ರಗತಿ_Ops;
    ಆದರೆ( db->x_ಪ್ರಗತಿ(db->p_ಪ್ರಗತಿ_Arg) ){
      n_ಪ್ರಗತಿ_ಎಲ್ಲೆ = ಅತಿದೊಡ್ಡ_UINT64;
      rc = SQLITE_ಅಡಚಣೆ;
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
 
  ನಿಲ್ಲಿಸು;
}

/* Opcode:  Gosub P1 P2 * * *
**
** Write the current address onto register P1
** and then jump to address P2.
*/
ಸ್ಥಿತಿ OP_Gosub: {            /* jump */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( VdbeMemDynamic(pIn1)==0 );
  mem_About_ಗಮ್ಯ_Change(p, pIn1);
  pIn1->flags = MEM_Int;
  pIn1->u.i = (ಪೂರ್ಣಾಂಕ)(pOp-aOp);
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p1, pIn1);
  ಹೋಗು jump_ಗಮ್ಯ_p2_and_check_for_ಅಡಚಣೆ;
}

/* Opcode:  Return P1 P2 P3 * *
**
** Jump to the address stored in register P1.  If P1 is a return address
** register, then this accomplishes a return from a subroutine.
**
** If P3 is 1, then the jump is only taken if register P1 holds an integer
** values, otherwise execution falls through to the next opcode, and the
** OP_Return becomes a no-op. If P3 is 0, then register P1 must hold an
** integer or else an assert() is raised.  P3 should be set to 1 when
** this opcode is used in combination with OP_BeginSubrtn, and set to 0
** otherwise.
**
** The value in register P1 is unchanged by this opcode.
**
** P2 is not used by the byte-code engine.  However, if P2 is positive
** and also less than the current address, then the "EXPLAIN" output
** formatter in the CLI will indent all opcodes from the P2 opcode up
** to be not including the current Return.   P2 should be the first opcode
** in the subroutine from which this opcode is returning.  Thus the P2
** value is a byte-code indentation hint.  See tag-20220407a in
** wherecode.c and shell.c.
*/
ಸ್ಥಿತಿ OP_Return: {           /* in1 */
  pIn1 = &aMem[pOp->p1];
  ಆದರೆ( pIn1->flags & MEM_Int ){
    ಆದರೆ( pOp->p3 ){ Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1, 2); }
    pOp = &aOp[pIn1->u.i];
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ALWAYS(pOp->p3) ){
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0, 2);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: InitCoroutine P1 P2 P3 * *
**
** Set up register P1 so that it will Yield to the coroutine
** located at address P3.
**
** If P2!=0 then the coroutine implementation immediately follows
** this opcode.  So jump over the coroutine implementation to
** address P2.
**
** See also: EndCoroutine
*/
ಸ್ಥಿತಿ OP_InitCoroutine: {     /* jump0 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 &&  pOp->p1<=(p->nMem+1 - p->nCursor) );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>=0 && pOp->p2<p->nOp );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3>=0 && pOp->p3<p->nOp );
  p_ಹೊರಹೋಗು = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( !VdbeMemDynamic(p_ಹೊರಹೋಗು) );
  p_ಹೊರಹೋಗು->u.i = pOp->p3 - 1;
  p_ಹೊರಹೋಗು->flags = MEM_Int;
  ಆದರೆ( pOp->p2==0 ) ನಿಲ್ಲಿಸು;

  /* Most jump operations do a goto to this spot in order to update
  ** the pOp pointer. */
jump_ಗಮ್ಯ_p2:
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 );       /* There are never any jumps to instruction 0 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p2<p->nOp );  /* Jumps must be in range */
  pOp = &aOp[pOp->p2 - 1];
  ನಿಲ್ಲಿಸು;
}

/* Opcode:  EndCoroutine P1 * * * *
**
** The instruction at the address in register P1 is a Yield.
** Jump to the P2 parameter of that Yield.
** After the jump, the value register P1 is left with a value
** such that subsequent OP_Yields go back to the this same
** OP_EndCoroutine instruction.
**
** See also: InitCoroutine
*/
ಸ್ಥಿತಿ OP_ಕೊನೆ_Coroutine: {           /* in1 */
  VdbeOp *pCaller;
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pIn1->flags==MEM_Int );
  ಪ್ರತಿಜ್ಞಾ( pIn1->u.i>=0 && pIn1->u.i<p->nOp );
  pCaller = &aOp[pIn1->u.i];
  ಪ್ರತಿಜ್ಞಾ( pCaller->opcode==OP_Yield );
  ಪ್ರತಿಜ್ಞಾ( pCaller->p2>=0 && pCaller->p2<p->nOp );
  pIn1->u.i = (ಪೂರ್ಣಾಂಕ)(pOp - p->aOp) - 1;
  pOp = &aOp[pCaller->p2 - 1];
  ನಿಲ್ಲಿಸು;
}

/* Opcode:  Yield P1 P2 * * *
**
** Swap the program counter with the value in register P1.  This
** has the effect of yielding to a coroutine.
**
** If the coroutine that is launched by this instruction ends with
** Yield or Return then continue to the next instruction.  But if
** the coroutine launched by this instruction ends with
** EndCoroutine, then jump to P2 rather than continuing with the
** next instruction.
**
** See also: InitCoroutine
*/
ಸ್ಥಿತಿ OP_Yield: {            /* in1, jump0 */
  ಪೂರ್ಣಾಂಕ pcDest;
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( VdbeMemDynamic(pIn1)==0 );
  pIn1->flags = MEM_Int;
  pcDest = (ಪೂರ್ಣಾಂಕ)pIn1->u.i;
  pIn1->u.i = (ಪೂರ್ಣಾಂಕ)(pOp - aOp);
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p1, pIn1);
  pOp = &aOp[pcDest];
  ನಿಲ್ಲಿಸು;
}

/* Opcode:  HaltIfNull  P1 P2 P3 P4 P5
** Synopsis: if r[P3]=null halt
**
** Check the value in register P3.  If it is NULL then Halt using
** parameter P1, P2, and P4 as if this were a Halt instruction.  If the
** value in register P3 is not NULL, then this routine is a no-op.
** The P5 parameter should be 1.
*/
ಸ್ಥಿತಿ OP_HaltIfNull: {      /* in3 */
  pIn3 = &aMem[pOp->p3];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( pOp->p2==OE_Abort ){ sqlite3VdbeAssertAbortable(p); }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( (pIn3->flags & MEM_Null)==0 ) ನಿಲ್ಲಿಸು;
  /* Fall through into OP_Halt */
  /* no break */ deliberate_fall_through
}

/* Opcode:  Halt P1 P2 P3 P4 P5
**
** Exit immediately.  All open cursors, etc are closed
** automatically.
**
** P1 is the result code returned by sqlite3_exec(), sqlite3_reset(),
** or sqlite3_finalize().  For a normal halt, this should be SQLITE_OK (0).
** For errors, it can be some other value.  If P1!=0 then P2 will determine
** whether or not to rollback the current transaction.  Do not rollback
** if P2==OE_Fail. Do the rollback if P2==OE_Rollback.  If P2==OE_Abort,
** then back out all changes that have occurred during this execution of the
** VDBE, but do not rollback the transaction.
**
** If P3 is not zero and P4 is NULL, then P3 is a register that holds the
** text of an error message.
**
** If P3 is zero and P4 is not null then the error message string is held
** in P4.
**
** P5 is a value between 1 and 4, inclusive, then the P4 error message
** string is modified as follows:
**
**    1:  NOT NULL constraint failed: P4
**    2:  UNIQUE constraint failed: P4
**    3:  CHECK constraint failed: P4
**    4:  FOREIGN KEY constraint failed: P4
**
** If P3 is zero and P5 is not zero and P4 is NULL, then everything after
** the ":" is omitted.
**
** There is an implied "Halt 0 0 0" instruction inserted at the very end of
** every program.  So a jump past the last instruction of the program
** is the same as executing Halt.
*/
ಸ್ಥಿತಿ OP_Halt: {
  Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು;
  ಪೂರ್ಣಾಂಕ pcx;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( pOp->p2==OE_Abort ){ sqlite3VdbeAssertAbortable(p); }
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_NOTUSED
       || pOp->p4type==P4_STATIC
       || pOp->p4type==P4_DYNAMIC );

  /* A deliberately coded "OP_Halt SQLITE_INTERNAL * * * *" opcode indicates
  ** something is wrong with the code generator.  Raise an assertion in order
  ** to bring this to the attention of fuzzers and other testing tools. */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1!=SQLITE_ಆಂತರಿಕ );

  ಆದರೆ( p->p_ಚೌಕಟ್ಟು && pOp->p1==SQLITE_OK ){
    /* Halt the sub-program. Return control to the parent frame. */
    p_ಚೌಕಟ್ಟು = p->p_ಚೌಕಟ್ಟು;
    p->p_ಚೌಕಟ್ಟು = p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ;
    p->n_ಚೌಕಟ್ಟು--;
    sqlite3_Vdbe_ಗಣ_Changes(db, p->nChange);
    pcx = sqlite3_Vdbe_ಚೌಕಟ್ಟು_Restore(p_ಚೌಕಟ್ಟು);
    ಆದರೆ( pOp->p2==OE_Ignore ){
      /* Instruction pcx is the OP_Program that invoked the sub-program
      ** currently being halted. If the p2 instruction of this OP_Halt
      ** instruction is set to OE_Ignore, then the sub-program is throwing
      ** an IGNORE exception. In this case jump to the address specified
      ** as the p2 of the calling OP_Program.  */
      pcx = p->aOp[pcx].p2-1;
    }
    aOp = p->aOp;
    aMem = p->aMem;
    pOp = &aOp[pcx];
    ನಿಲ್ಲಿಸು;
  }
  p->rc = pOp->p1;
  p->errorAction = (u8)pOp->p2;
  ಪ್ರತಿಜ್ಞಾ( pOp->p5<=4 );
  ಆದರೆ( p->rc ){
    ಆದರೆ( pOp->p3>0 && pOp->p4type==P4_NOTUSED ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr;
      ಪ್ರತಿಜ್ಞಾ( pOp->p3<=(p->nMem + 1 - p->nCursor) );
      zErr = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(&aMem[pOp->p3], SQLITE_UTF8);
      sqlite3VdbeError(p, "%s", zErr);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pOp->p5 ){
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ azType[] = { "NOT NULL", "UNIQUE", "CHECK",
                                             "FOREIGN KEY" };
      testcase( pOp->p5==1 );
      testcase( pOp->p5==2 );
      testcase( pOp->p5==3 );
      testcase( pOp->p5==4 );
      sqlite3VdbeError(p, "%s constraint failed", azType[pOp->p5-1]);
      ಆದರೆ( pOp->p4.z ){
        p->z_Err_ಸಂದೇಶ = sqlite3MPrintf(db, "%z: %s", p->z_Err_ಸಂದೇಶ, pOp->p4.z);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3VdbeError(p, "%s", pOp->p4.z);
    }
    sqlite3VdbeLogAbort(p, pOp->p1, pOp, aOp);
  }
  rc = sqlite3VdbeHalt(p);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_BUSY || rc==SQLITE_OK || rc==SQLITE_ERROR );
  ಆದರೆ( rc==SQLITE_BUSY ){
    p->rc = SQLITE_BUSY;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT );
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || db->nDeferredCons>0 || db->nDeferredImmCons>0 );
    rc = p->rc ? SQLITE_ERROR : SQLITE_ಮುಗಿದ;
  }
  ಹೋಗು vdbe_return;
}

/* Opcode: Integer P1 P2 * * *
** Synopsis: r[P2]=P1
**
** The 32-bit integer value P1 is written into register P2.
*/
ಸ್ಥಿತಿ OP_Integer: {         /* out2 */
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->u.i = pOp->p1;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Int64 P1 P2 P3 * *
** Synopsis: r[P2]=PINT13
**
** Combine P1 and P3 into a signed 64-bit integer.  P1 is the least
** significant 32 bits and P3 is the most significant.  Store the
** result in register P2.
*/
ಸ್ಥಿತಿ OP_Int64: {           /* out2 */
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->u.i = INT32_ಗಮ್ಯ_64(pOp->p1,pOp->p3);
  ನಿಲ್ಲಿಸು;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
/* Opcode: Real P1 P2 P3 * *
** Synopsis: r[P2]=PDBL13
**
** P1 and P3 are combined to 64 bits with P1 being the lower the P3
** the upper.  The result is interpreted as a 64-bit floating-point
** and stored in register P2.
*/
ಸ್ಥಿತಿ OP_Real: {            /* same as TK_FLOAT, out2 */
  u64 ii;
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->flags = MEM_Real;
  ii = (u64)INT32_ಗಮ್ಯ_64(pOp->p1,pOp->p3);
  ಅದಲುಬದಲು_Mixed_Endian_Float(ii);
  ಸ್ಮರಣೆ_ನಕಲು(&p_ಹೊರಹೋಗು->u.r, &ii, 8);
  ಪ್ರತಿಜ್ಞಾ( !sqlite3IsNaN(p_ಹೊರಹೋಗು->u.r) );
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Opcode: String8 * P2 * P4 *
** Synopsis: r[P2]='P4'
**
** P4 points to a nul terminated UTF-8 string. This opcode is transformed
** into a String opcode before it is executed for the first time.  During
** this transformation, the length of string P4 is computed and stored
** as the P1 parameter.
*/
ಸ್ಥಿತಿ OP_String8: {         /* same as TK_STRING, out2 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p4.z!=0 );
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  pOp->p1 = sqlite3Strlen30(pOp->p4.z);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  ಆದರೆ( encoding!=SQLITE_UTF8 ){
    rc = sqlite3_Vdbe_Mem_ಗಣ_Str(p_ಹೊರಹೋಗು, pOp->p4.z, -1, SQLITE_UTF8, SQLITE_STATIC);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || rc==SQLITE_TOOBIG );
    ಆದರೆ( rc ) ಹೋಗು too_big;
    ಆದರೆ( SQLITE_OK!=sqlite3VdbeChangeEncoding(p_ಹೊರಹೋಗು, encoding) ) ಹೋಗು no_mem;
    ಪ್ರತಿಜ್ಞಾ( p_ಹೊರಹೋಗು->szMalloc>0 && p_ಹೊರಹೋಗು->zMalloc==p_ಹೊರಹೋಗು->z );
    ಪ್ರತಿಜ್ಞಾ( VdbeMemDynamic(p_ಹೊರಹೋಗು)==0 );
    p_ಹೊರಹೋಗು->szMalloc = 0;
    p_ಹೊರಹೋಗು->flags |= MEM_Static;
    ಆದರೆ( pOp->p4type==P4_DYNAMIC ){
      sqlite3DbFree(db, pOp->p4.z);
    }
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = p_ಹೊರಹೋಗು->z;
    pOp->p1 = p_ಹೊರಹೋಗು->n;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pOp->p1>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
    ಹೋಗು too_big;
  }
  pOp->opcode = OP_String;
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  /* Fall through to the next case, OP_String */
  /* no break */ deliberate_fall_through
}
 
/* Opcode: String P1 P2 P3 P4 P5
** Synopsis: r[P2]='P4' (len=P1)
**
** The string value P4 of length P1 (bytes) is stored in register P2.
**
** If P3 is not zero and the content of register P3 is equal to P5, then
** the datatype of the register P2 is converted to BLOB.  The content is
** the same sequence of bytes, it is merely interpreted as a BLOB instead
** of a string, as if it had been CAST.  In other words:
**
** if( P3!=0 and reg[P3]==P5 ) reg[P2] := CAST(reg[P2] as BLOB)
*/
ಸ್ಥಿತಿ OP_String: {          /* out2 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p4.z!=0 );
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->flags = MEM_Str|MEM_Static|MEM_ಪದ;
  p_ಹೊರಹೋಗು->z = pOp->p4.z;
  p_ಹೊರಹೋಗು->n = pOp->p1;
  p_ಹೊರಹೋಗು->enc = encoding;
  UPDATE_MAX_BLOBSIZE(p_ಹೊರಹೋಗು);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_LIKE_DOESNT_ಹೊಂದಿಕೆ_BLOBS
  ಆದರೆ( pOp->p3>0 ){
    ಪ್ರತಿಜ್ಞಾ( pOp->p3<=(p->nMem+1 - p->nCursor) );
    pIn3 = &aMem[pOp->p3];
    ಪ್ರತಿಜ್ಞಾ( pIn3->flags & MEM_Int );
    ಆದರೆ( pIn3->u.i==pOp->p5 ) p_ಹೊರಹೋಗು->flags = MEM_Blob|MEM_Static|MEM_ಪದ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ನಿಲ್ಲಿಸು;
}

/* Opcode: BeginSubrtn * P2 * * *
** Synopsis: r[P2]=NULL
**
** Mark the beginning of a subroutine that can be entered in-line
** or that can be called using OP_Gosub.  The subroutine should
** be terminated by an OP_Return instruction that has a P1 operand that
** is the same as the P2 operand to this opcode and that has P3 set to 1.
** If the subroutine is entered in-line, then the OP_Return will simply
** fall through.  But if the subroutine is entered using OP_Gosub, then
** the OP_Return will jump back to the first instruction after the OP_Gosub.
**
** This routine works by loading a NULL into the P2 register.  When the
** return address register contains a NULL, the OP_Return instruction is
** a no-op that simply falls through to the next instruction (assuming that
** the OP_Return opcode has a P3 value of 1).  Thus if the subroutine is
** entered in-line, then the OP_Return will cause in-line execution to
** continue.  But if the subroutine is entered via OP_Gosub, then the
** OP_Return will cause a return to the address following the OP_Gosub.
**
** This opcode is identical to OP_Null.  It has a different name
** only to make the byte code easier to read and verify.
*/
/* Opcode: Null P1 P2 P3 * *
** Synopsis: r[P2..P3]=NULL
**
** Write a NULL into registers P2.  If P3 greater than P2, then also write
** NULL into register P3 and every register in between P2 and P3.  If P3
** is less than P2 (typically P3 is zero) then only register P2 is
** set to NULL.
**
** If the P1 value is non-zero, then also set the MEM_Cleared flag so that
** NULL values will not compare equal even if SQLITE_NULLEQ is set on
** OP_Ne or OP_Eq.
*/
ಸ್ಥಿತಿ OP_BeginSubrtn:
ಸ್ಥಿತಿ OP_Null: {           /* out2 */
  ಪೂರ್ಣಾಂಕ cnt;
  u16 nullFlag;
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  cnt = pOp->p3-pOp->p2;
  ಪ್ರತಿಜ್ಞಾ( pOp->p3<=(p->nMem+1 - p->nCursor) );
  p_ಹೊರಹೋಗು->flags = nullFlag = pOp->p1 ? (MEM_Null|MEM_Cleared) : MEM_Null;
  p_ಹೊರಹೋಗು->n = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  p_ಹೊರಹೋಗು->u_ತಾತ್ಕಾಲಿಕ = 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಆಗ( cnt>0 ){
    p_ಹೊರಹೋಗು++;
    mem_About_ಗಮ್ಯ_Change(p, p_ಹೊರಹೋಗು);
    sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
    p_ಹೊರಹೋಗು->flags = nullFlag;
    p_ಹೊರಹೋಗು->n = 0;
    cnt--;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SoftNull P1 * * * *
** Synopsis: r[P1]=NULL
**
** Set register P1 to have the value NULL as seen by the OP_MakeRecord
** instruction, but do not free any string or blob memory associated with
** the register, so that if the value was a string or blob that was
** previously copied using OP_SCopy, the copies will continue to be valid.
*/
ಸ್ಥಿತಿ OP_SoftNull: {
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  p_ಹೊರಹೋಗು = &aMem[pOp->p1];
  p_ಹೊರಹೋಗು->flags = (p_ಹೊರಹೋಗು->flags&~(MEM_Undefined|MEM_Aff_ಮುಖವಾಡ))|MEM_Null;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Blob P1 P2 * P4 *
** Synopsis: r[P2]=P4 (len=P1)
**
** P4 points to a blob of data P1 bytes long.  Store this
** blob in register P2.  If P4 is a NULL pointer, then construct
** a zero-filled blob that is P1 bytes long in P2.
*/
ಸ್ಥಿತಿ OP_Blob: {                /* out2 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1 <= SQLITE_MAX_ಉದ್ದಳತೆ );
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  ಆದರೆ( pOp->p4.z==0 ){
    sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(p_ಹೊರಹೋಗು, pOp->p1);
    ಆದರೆ( sqlite3VdbeMemExpandBlob(p_ಹೊರಹೋಗು) ) ಹೋಗು no_mem;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Vdbe_Mem_ಗಣ_Str(p_ಹೊರಹೋಗು, pOp->p4.z, pOp->p1, 0, 0);
  }
  p_ಹೊರಹೋಗು->enc = encoding;
  UPDATE_MAX_BLOBSIZE(p_ಹೊರಹೋಗು);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Variable P1 P2 * * *
** Synopsis: r[P2]=parameter(P1)
**
** Transfer the values of bound parameter P1 into register P2
*/
ಸ್ಥಿತಿ OP_Variable: {            /* out2 */
  Mem *pVar;       /* Value being transferred */

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 && pOp->p1<=p->nVar );
  pVar = &p->aVar[pOp->p1 - 1];
  ಆದರೆ( sqlite3VdbeMemTooBig(pVar) ){
    ಹೋಗು too_big;
  }
  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  ಆದರೆ( VdbeMemDynamic(p_ಹೊರಹೋಗು) ) sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
  ಸ್ಮರಣೆ_ನಕಲು(p_ಹೊರಹೋಗು, pVar, MEMCELLSIZE);
  p_ಹೊರಹೋಗು->flags &= ~(MEM_Dyn|MEM_Ephem);
  p_ಹೊರಹೋಗು->flags |= MEM_Static|MEM_FromBind;
  UPDATE_MAX_BLOBSIZE(p_ಹೊರಹೋಗು);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Move P1 P2 P3 * *
** Synopsis: r[P2@P3]=r[P1@P3]
**
** Move the P3 values in register P1..P1+P3-1 over into
** registers P2..P2+P3-1.  Registers P1..P1+P3-1 are
** left holding a NULL.  It is an error for register ranges
** P1..P1+P3-1 and P2..P2+P3-1 to overlap.  It is an error
** for P3 to be less than 1.
*/
ಸ್ಥಿತಿ OP_Move: {
  ಪೂರ್ಣಾಂಕ n;           /* Number of registers left to copy */
  ಪೂರ್ಣಾಂಕ p1;          /* Register to copy from */
  ಪೂರ್ಣಾಂಕ p2;          /* Register to copy to */

  n = pOp->p3;
  p1 = pOp->p1;
  p2 = pOp->p2;
  ಪ್ರತಿಜ್ಞಾ( n>0 && p1>0 && p2>0 );
  ಪ್ರತಿಜ್ಞಾ( p1+n<=p2 || p2+n<=p1 );

  pIn1 = &aMem[p1];
  p_ಹೊರಹೋಗು = &aMem[p2];
  ಮಾಡು{
    ಪ್ರತಿಜ್ಞಾ( p_ಹೊರಹೋಗು<=&aMem[(p->nMem+1 - p->nCursor)] );
    ಪ್ರತಿಜ್ಞಾ( pIn1<=&aMem[(p->nMem+1 - p->nCursor)] );
    ಪ್ರತಿಜ್ಞಾ( memIsValid(pIn1) );
    mem_About_ಗಮ್ಯ_Change(p, p_ಹೊರಹೋಗು);
    sqlite3VdbeMemMove(p_ಹೊರಹೋಗು, pIn1);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    pIn1->pScopyFrom = 0;
    { ಪೂರ್ಣಾಂಕ i;
      ಗಾಗಿ(i=1; i<p->nMem; i++){
        ಆದರೆ( aMem[i].pScopyFrom==pIn1 ){
          ಪ್ರತಿಜ್ಞಾ( aMem[i].bScopy );
          aMem[i].pScopyFrom = p_ಹೊರಹೋಗು;
        }
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
    Deephemeralize(p_ಹೊರಹೋಗು);
    ರಿಜಿಸ್ಟರ್_TRACE(p2++, p_ಹೊರಹೋಗು);
    pIn1++;
    p_ಹೊರಹೋಗು++;
  }ಆಗ( --n );
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Copy P1 P2 P3 * P5
** Synopsis: r[P2@P3+1]=r[P1@P3+1]
**
** Make a copy of registers P1..P1+P3 into registers P2..P2+P3.
**
** If the 0x0002 bit of P5 is set then also clear the MEM_Subtype flag in the
** destination.  The 0x0001 bit of P5 indicates that this Copy opcode cannot
** be merged.  The 0x0001 bit is used by the query planner and does not
** come into play during query execution.
**
** This instruction makes a deep copy of the value.  A duplicate
** is made of any string or blob constant.  See also OP_SCopy.
*/
ಸ್ಥಿತಿ OP_Copy: {
  ಪೂರ್ಣಾಂಕ n;

  n = pOp->p3;
  pIn1 = &aMem[pOp->p1];
  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  ಪ್ರತಿಜ್ಞಾ( p_ಹೊರಹೋಗು!=pIn1 );
  ಆಗ( 1 ){
    mem_About_ಗಮ್ಯ_Change(p, p_ಹೊರಹೋಗು);
    sqlite3VdbeMemShallowCopy(p_ಹೊರಹೋಗು, pIn1, MEM_Ephem);
    Deephemeralize(p_ಹೊರಹೋಗು);
    ಆದರೆ( (p_ಹೊರಹೋಗು->flags & MEM_Subtype)!=0 &&  (pOp->p5 & 0x0002)!=0 ){
      p_ಹೊರಹೋಗು->flags &= ~MEM_Subtype;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    p_ಹೊರಹೋಗು->pScopyFrom = 0;
#ಆದರೆ_ಅಂತ್ಯ
    ರಿಜಿಸ್ಟರ್_TRACE(pOp->p2+pOp->p3-n, p_ಹೊರಹೋಗು);
    ಆದರೆ( (n--)==0 ) ನಿಲ್ಲಿಸು;
    p_ಹೊರಹೋಗು++;
    pIn1++;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SCopy P1 P2 * * *
** Synopsis: r[P2]=r[P1]
**
** Make a shallow copy of register P1 into register P2.
**
** This instruction makes a shallow copy of the value.  If the value
** is a string or blob, then the copy is only a pointer to the
** original and hence if the original changes so will the copy.
** Worse, if the original is deallocated, the copy becomes invalid.
** Thus the program must guarantee that the original will not change
** during the lifetime of the copy.  Use OP_Copy to make a complete
** copy.
*/
ಸ್ಥಿತಿ OP_SCopy: {            /* out2 */
  pIn1 = &aMem[pOp->p1];
  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  ಪ್ರತಿಜ್ಞಾ( p_ಹೊರಹೋಗು!=pIn1 );
  sqlite3VdbeMemShallowCopy(p_ಹೊರಹೋಗು, pIn1, MEM_Ephem);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  p_ಹೊರಹೋಗು->pScopyFrom = pIn1;
  p_ಹೊರಹೋಗು->mScopyFlags = pIn1->flags;
  pIn1->bScopy = 1;
#ಆದರೆ_ಅಂತ್ಯ
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IntCopy P1 P2 * * *
** Synopsis: r[P2]=r[P1]
**
** Transfer the integer value held in register P1 into register P2.
**
** This is an optimized version of SCopy that works only for integer
** values.
*/
ಸ್ಥಿತಿ OP_IntCopy: {            /* out2 */
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( (pIn1->flags & MEM_Int)!=0 );
  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  sqlite3_Vdbe_Mem_ಗಣ_Int64(p_ಹೊರಹೋಗು, pIn1->u.i);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: FkCheck * * * * *
**
** Halt with an SQLITE_CONSTRAINT error if there are any unresolved
** foreign key constraint violations.  If there are no foreign key
** constraint violations, this is a no-op.
**
** FK constraint violations are also checked when the prepared statement
** exits.  This opcode is used to raise foreign key constraint errors prior
** to returning results such as a row change count or the result of a
** RETURNING clause.
*/
ಸ್ಥಿತಿ OP_FkCheck: {
  ಆದರೆ( (rc = sqlite3_Vdbe_Check_Fk_ತಕ್ಷಣ(p))!=SQLITE_OK ){
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: ResultRow P1 P2 * * *
** Synopsis: output=r[P1@P2]
**
** The registers P1 through P1+P2-1 contain a single row of
** results. This opcode causes the sqlite3_step() call to terminate
** with an SQLITE_ROW return code and it sets up the sqlite3_stmt
** structure to provide access to the r(P1)..r(P1+P2-1) values as
** the result row.
*/
ಸ್ಥಿತಿ OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು: {
  ಪ್ರತಿಜ್ಞಾ( p->n_Res_ಸ್ತಂಭ==pOp->p2 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1+pOp->p2<=(p->nMem+1 - p->nCursor)+1 );

  p->cacheCtr = (p->cacheCtr + 2)|1;
  p->p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು = &aMem[pOp->p1];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  {
    Mem *pMem = p->p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pOp->p2; i++){
      ಪ್ರತಿಜ್ಞಾ( memIsValid(&pMem[i]) );
      ರಿಜಿಸ್ಟರ್_TRACE(pOp->p1+i, &pMem[i]);
      /* The registers in the result will not be used again when the
      ** prepared statement restarts.  This is because sqlite3_column()
      ** APIs might have caused type conversions of made other changes to
      ** the register values.  Therefore, we can go ahead and break any
      ** OP_SCopy dependencies. */
      pMem[i].pScopyFrom = 0;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( db->mallocFailed ) ಹೋಗು no_mem;
  ಆದರೆ( db->mTrace & SQLITE_TRACE_ಅಡ್ಡಸಾಲು ){
    db->trace.xV2(SQLITE_TRACE_ಅಡ್ಡಸಾಲು, db->pTraceArg, p, 0);
  }
  p->pc = (ಪೂರ್ಣಾಂಕ)(pOp - aOp) + 1;
  rc = SQLITE_ಅಡ್ಡಸಾಲು;
  ಹೋಗು vdbe_return;
}

/* Opcode: Concat P1 P2 P3 * *
** Synopsis: r[P3]=r[P2]+r[P1]
**
** Add the text in register P1 onto the end of the text in
** register P2 and store the result in register P3.
** If either the P1 or P2 text are NULL then store NULL in P3.
**
**   P3 = P2 || P1
**
** It is illegal for P1 and P3 to be the same register. Sometimes,
** if P3 is the same register as P2, the implementation is able
** to avoid a memcpy().
*/
ಸ್ಥಿತಿ OP_Concat: {           /* same as TK_CONCAT, in1, in2, out3 */
  i64 nByte;          /* Total size of the output string or blob */
  u16 flags1;         /* Initial flags for P1 */
  u16 flags2;         /* Initial flags for P2 */

  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  p_ಹೊರಹೋಗು = &aMem[pOp->p3];
  testcase( p_ಹೊರಹೋಗು==pIn2 );
  ಪ್ರತಿಜ್ಞಾ( pIn1!=p_ಹೊರಹೋಗು );
  flags1 = pIn1->flags;
  testcase( flags1 & MEM_Null );
  testcase( pIn2->flags & MEM_Null );
  ಆದರೆ( (flags1 | pIn2->flags) & MEM_Null ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
    ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ( (flags1 & (MEM_Str|MEM_Blob))==0 ){
    ಆದರೆ( sqlite3VdbeMemStringify(pIn1,encoding,0) ) ಹೋಗು no_mem;
    flags1 = pIn1->flags & ~MEM_Str;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags1 & MEM_Zero)!=0 ){
    ಆದರೆ( sqlite3VdbeMemExpandBlob(pIn1) ) ಹೋಗು no_mem;
    flags1 = pIn1->flags & ~MEM_Str;
  }
  flags2 = pIn2->flags;
  ಆದರೆ( (flags2 & (MEM_Str|MEM_Blob))==0 ){
    ಆದರೆ( sqlite3VdbeMemStringify(pIn2,encoding,0) ) ಹೋಗು no_mem;
    flags2 = pIn2->flags & ~MEM_Str;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags2 & MEM_Zero)!=0 ){
    ಆದರೆ( sqlite3VdbeMemExpandBlob(pIn2) ) ಹೋಗು no_mem;
    flags2 = pIn2->flags & ~MEM_Str;
  }
  nByte = pIn1->n;
  nByte += pIn2->n;
  ಆದರೆ( nByte>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
    ಹೋಗು too_big;
  }
#ಆದರೆ SQLITE_MAX_ಉದ್ದಳತೆ>2147483645
  ಆದರೆ( nByte>2147483645 ){ ಹೋಗು too_big; }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( sqlite3VdbeMemGrow(p_ಹೊರಹೋಗು, (ಪೂರ್ಣಾಂಕ)nByte+2, p_ಹೊರಹೋಗು==pIn2) ){
    ಹೋಗು no_mem;
  }
  Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Str);
  ಆದರೆ( p_ಹೊರಹೋಗು!=pIn2 ){
    ಸ್ಮರಣೆ_ನಕಲು(p_ಹೊರಹೋಗು->z, pIn2->z, pIn2->n);
    ಪ್ರತಿಜ್ಞಾ( (pIn2->flags & MEM_Dyn) == (flags2 & MEM_Dyn) );
    pIn2->flags = flags2;
  }
  ಸ್ಮರಣೆ_ನಕಲು(&p_ಹೊರಹೋಗು->z[pIn2->n], pIn1->z, pIn1->n);
  ಪ್ರತಿಜ್ಞಾ( (pIn1->flags & MEM_Dyn) == (flags1 & MEM_Dyn) );
  pIn1->flags = flags1;
  ಆದರೆ( encoding>SQLITE_UTF8 ) nByte &= ~1;
  p_ಹೊರಹೋಗು->z[nByte]=0;
  p_ಹೊರಹೋಗು->z[nByte+1] = 0;
  p_ಹೊರಹೋಗು->flags |= MEM_ಪದ;
  p_ಹೊರಹೋಗು->n = (ಪೂರ್ಣಾಂಕ)nByte;
  p_ಹೊರಹೋಗು->enc = encoding;
  UPDATE_MAX_BLOBSIZE(p_ಹೊರಹೋಗು);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Add P1 P2 P3 * *
** Synopsis: r[P3]=r[P1]+r[P2]
**
** Add the value in register P1 to the value in register P2
** and store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: Multiply P1 P2 P3 * *
** Synopsis: r[P3]=r[P1]*r[P2]
**
**
** Multiply the value in register P1 by the value in register P2
** and store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: Subtract P1 P2 P3 * *
** Synopsis: r[P3]=r[P2]-r[P1]
**
** Subtract the value in register P1 from the value in register P2
** and store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: Divide P1 P2 P3 * *
** Synopsis: r[P3]=r[P2]/r[P1]
**
** Divide the value in register P1 by the value in register P2
** and store the result in register P3 (P3=P2/P1). If the value in
** register P1 is zero, then the result is NULL. If either input is
** NULL, the result is NULL.
*/
/* Opcode: Remainder P1 P2 P3 * *
** Synopsis: r[P3]=r[P2]%r[P1]
**
** Compute the remainder after integer register P2 is divided by
** register P1 and store the result in register P3.
** If the value in register P1 is zero the result is NULL.
** If either operand is NULL, the result is NULL.
*/
ಸ್ಥಿತಿ OP_ಕೂಡಿಸು:                   /* same as TK_PLUS, in1, in2, out3 */
ಸ್ಥಿತಿ OP_ಕಳೆ:              /* same as TK_MINUS, in1, in2, out3 */
ಸ್ಥಿತಿ OP_Multiply:              /* same as TK_STAR, in1, in2, out3 */
ಸ್ಥಿತಿ OP_Divide:                /* same as TK_SLASH, in1, in2, out3 */
ಸ್ಥಿತಿ OP_Remainder: {           /* same as TK_REM, in1, in2, out3 */
  u16 type1;      /* Numeric type of left operand */
  u16 type2;      /* Numeric type of right operand */
  i64 iA;         /* Integer value of left operand */
  i64 iB;         /* Integer value of right operand */
  ದ್ವಿಗುಣ rA;      /* Real value of left operand */
  ದ್ವಿಗುಣ rB;      /* Real value of right operand */

  pIn1 = &aMem[pOp->p1];
  type1 = pIn1->flags;
  pIn2 = &aMem[pOp->p2];
  type2 = pIn2->flags;
  p_ಹೊರಹೋಗು = &aMem[pOp->p3];
  ಆದರೆ( (type1 & type2 & MEM_Int)!=0 ){
int_math:
    iA = pIn1->u.i;
    iB = pIn2->u.i;
    ಆಯ್ಕೆ( pOp->opcode ){
      ಸ್ಥಿತಿ OP_ಕೂಡಿಸು:       ಆದರೆ( sqlite3_ಕೂಡಿಸು_Int64(&iB,iA) ) ಹೋಗು fp_math;  ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ OP_ಕಳೆ:  ಆದರೆ( sqlite3SubInt64(&iB,iA) ) ಹೋಗು fp_math;  ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ OP_Multiply:  ಆದರೆ( sqlite3MulInt64(&iB,iA) ) ಹೋಗು fp_math;  ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ OP_Divide: {
        ಆದರೆ( iA==0 ) ಹೋಗು arithmetic_ಫಲಿತಾಂಶ_is_null;
        ಆದರೆ( iA==-1 && iB==ಅತಿಚಿಕ್ಕ_INT64 ) ಹೋಗು fp_math;
        iB /= iA;
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಆದರೆ( iA==0 ) ಹೋಗು arithmetic_ಫಲಿತಾಂಶ_is_null;
        ಆದರೆ( iA==-1 ) iA = 1;
        iB %= iA;
        ನಿಲ್ಲಿಸು;
      }
    }
    p_ಹೊರಹೋಗು->u.i = iB;
    Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Int);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ((type1 | type2) & MEM_Null)!=0 ){
    ಹೋಗು arithmetic_ಫಲಿತಾಂಶ_is_null;
  }ಇಲ್ಲದಿದ್ದರೆ{
    type1 = numericType(pIn1);
    type2 = numericType(pIn2);
    ಆದರೆ( (type1 & type2 & MEM_Int)!=0 ) ಹೋಗು int_math;
fp_math:
    rA = sqlite3_Vdbe_Real_ಮೌಲ್ಯ(pIn1);
    rB = sqlite3_Vdbe_Real_ಮೌಲ್ಯ(pIn2);
    ಆಯ್ಕೆ( pOp->opcode ){
      ಸ್ಥಿತಿ OP_ಕೂಡಿಸು:         rB += rA;       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ OP_ಕಳೆ:    rB -= rA;       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ OP_Multiply:    rB *= rA;       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ OP_Divide: {
        /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
        ಆದರೆ( rA==(ದ್ವಿಗುಣ)0 ) ಹೋಗು arithmetic_ಫಲಿತಾಂಶ_is_null;
        rB /= rA;
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        iA = sqlite3_Vdbe_Int_ಮೌಲ್ಯ(pIn1);
        iB = sqlite3_Vdbe_Int_ಮೌಲ್ಯ(pIn2);
        ಆದರೆ( iA==0 ) ಹೋಗು arithmetic_ಫಲಿತಾಂಶ_is_null;
        ಆದರೆ( iA==-1 ) iA = 1;
        rB = (ದ್ವಿಗುಣ)(iB % iA);
        ನಿಲ್ಲಿಸು;
      }
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
    p_ಹೊರಹೋಗು->u.i = rB;
    Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Int);
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( sqlite3IsNaN(rB) ){
      ಹೋಗು arithmetic_ಫಲಿತಾಂಶ_is_null;
    }
    p_ಹೊರಹೋಗು->u.r = rB;
    Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Real);
#ಆದರೆ_ಅಂತ್ಯ
  }
  ನಿಲ್ಲಿಸು;

arithmetic_ಫಲಿತಾಂಶ_is_null:
  sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: CollSeq P1 * * P4
**
** P4 is a pointer to a CollSeq object. If the next call to a user function
** or aggregate calls sqlite3GetFuncCollSeq(), this collation sequence will
** be returned. This is used by the built-in min(), max() and nullif()
** functions.
**
** If P1 is not zero, then it is a register that a subsequent min() or
** max() aggregate will set to 1 if the current row is not the minimum or
** maximum.  The P1 register is initialized to 0 by this instruction.
**
** The interface used by the implementation of the aforementioned functions
** to retrieve the collation sequence set by this opcode is not available
** publicly.  Only built-in functions have access to this feature.
*/
ಸ್ಥಿತಿ OP_CollSeq: {
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_COLLSEQ );
  ಆದರೆ( pOp->p1 ){
    sqlite3_Vdbe_Mem_ಗಣ_Int64(&aMem[pOp->p1], 0);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: BitAnd P1 P2 P3 * *
** Synopsis: r[P3]=r[P1]&r[P2]
**
** Take the bit-wise AND of the values in register P1 and P2 and
** store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: BitOr P1 P2 P3 * *
** Synopsis: r[P3]=r[P1]|r[P2]
**
** Take the bit-wise OR of the values in register P1 and P2 and
** store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: ShiftLeft P1 P2 P3 * *
** Synopsis: r[P3]=r[P2]<<r[P1]
**
** Shift the integer value in register P2 to the left by the
** number of bits specified by the integer in register P1.
** Store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: ShiftRight P1 P2 P3 * *
** Synopsis: r[P3]=r[P2]>>r[P1]
**
** Shift the integer value in register P2 to the right by the
** number of bits specified by the integer in register P1.
** Store the result in register P3.
** If either input is NULL, the result is NULL.
*/
ಸ್ಥಿತಿ OP_ಬಿಟ್_And:                 /* same as TK_BITAND, in1, in2, out3 */
ಸ್ಥಿತಿ OP_ಬಿಟ್_Or:                  /* same as TK_BITOR, in1, in2, out3 */
ಸ್ಥಿತಿ OP_ಸರಿಸು_ಎಡ:              /* same as TK_LSHIFT, in1, in2, out3 */
ಸ್ಥಿತಿ OP_ಸರಿಸು_ಬಲ: {           /* same as TK_RSHIFT, in1, in2, out3 */
  i64 iA;
  u64 uA;
  i64 iB;
  u8 op;

  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  p_ಹೊರಹೋಗು = &aMem[pOp->p3];
  ಆದರೆ( (pIn1->flags | pIn2->flags) & MEM_Null ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
    ನಿಲ್ಲಿಸು;
  }
  iA = sqlite3_Vdbe_Int_ಮೌಲ್ಯ(pIn2);
  iB = sqlite3_Vdbe_Int_ಮೌಲ್ಯ(pIn1);
  op = pOp->opcode;
  ಆದರೆ( op==OP_ಬಿಟ್_And ){
    iA &= iB;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==OP_ಬಿಟ್_Or ){
    iA |= iB;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iB!=0 ){
    ಪ್ರತಿಜ್ಞಾ( op==OP_ಸರಿಸು_ಬಲ || op==OP_ಸರಿಸು_ಎಡ );

    /* If shifting by a negative amount, shift in the other direction */
    ಆದರೆ( iB<0 ){
      ಪ್ರತಿಜ್ಞಾ( OP_ಸರಿಸು_ಬಲ==OP_ಸರಿಸು_ಎಡ+1 );
      op = 2*OP_ಸರಿಸು_ಎಡ + 1 - op;
      iB = iB>(-64) ? -iB : 64;
    }

    ಆದರೆ( iB>=64 ){
      iA = (iA>=0 || op==OP_ಸರಿಸು_ಎಡ) ? 0 : -1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಮರಣೆ_ನಕಲು(&uA, &iA, ಗಾತ್ರ(uA));
      ಆದರೆ( op==OP_ಸರಿಸು_ಎಡ ){
        uA <<= iB;
      }ಇಲ್ಲದಿದ್ದರೆ{
        uA >>= iB;
        /* Sign-extend on a right shift of a negative number */
        ಆದರೆ( iA<0 ) uA |= ((((u64)0xffffffff)<<32)|0xffffffff) << (64-iB);
      }
      ಸ್ಮರಣೆ_ನಕಲು(&iA, &uA, ಗಾತ್ರ(iA));
    }
  }
  p_ಹೊರಹೋಗು->u.i = iA;
  Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Int);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: AddImm  P1 P2 * * *
** Synopsis: r[P1]=r[P1]+P2
**
** Add the constant P2 to the value in register P1.
** The result is always an integer.
**
** To force any register to be an integer, just add 0.
*/
ಸ್ಥಿತಿ OP_ಕೂಡಿಸು_Imm: {            /* in1 */
  pIn1 = &aMem[pOp->p1];
  mem_About_ಗಮ್ಯ_Change(p, pIn1);
  sqlite3VdbeMemIntegerify(pIn1);
  *(u64*)&pIn1->u.i += (u64)pOp->p2;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: MustBeInt P1 P2 * * *
**
** Force the value in register P1 to be an integer.  If the value
** in P1 is not an integer and cannot be converted into an integer
** without data loss, then jump immediately to P2, or if P2==0
** raise an SQLITE_MISMATCH exception.
*/
ಸ್ಥಿತಿ OP_MustBeInt: {            /* jump0, in1 */
  pIn1 = &aMem[pOp->p1];
  ಆದರೆ( (pIn1->flags & MEM_Int)==0 ){
    ಆದರೆ( (pIn1->flags & MEM_Real)!=0 ){
      sqlite3_int64 ix;
      ಆದರೆ( pIn1->u.r>=-9223372036854775808.0
       && pIn1->u.r<+9223372036854775808.0
       && (ದ್ವಿಗುಣ)(ix = sqlite3_Real_ಗಮ್ಯ_I64(pIn1->u.r))==pIn1->u.r
      ){
        pIn1->u.i = ix;
        Mem_ಗಣ_Type_Flag(pIn1, MEM_Int);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      applyAffinity(pIn1, SQLITE_AFF_NUMERIC, encoding);
    }
    ಆದರೆ( (pIn1->flags & MEM_Int)==0 ){
      Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1, 2);
      ಆದರೆ( pOp->p2==0 ){
        rc = SQLITE_MISMATCH;
        ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹೋಗು jump_ಗಮ್ಯ_p2;
      }
    }
  }
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0, 2);
  Mem_ಗಣ_Type_Flag(pIn1, MEM_Int);
  ನಿಲ್ಲಿಸು;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
/* Opcode: RealAffinity P1 * * * *
**
** If register P1 holds an integer convert it to a real value.
**
** This opcode is used when extracting information from a column that
** has REAL affinity.  Such column values may still be stored as
** integers, for space efficiency, but after extraction we want them
** to have only a real value.
*/
ಸ್ಥಿತಿ OP_RealAffinity: {                  /* in1 */
  pIn1 = &aMem[pOp->p1];
  ಆದರೆ( pIn1->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pIn1->flags & MEM_Int );
    testcase( pIn1->flags & MEM_IntReal );
    sqlite3VdbeMemRealify(pIn1);
    ರಿಜಿಸ್ಟರ್_TRACE(pOp->p1, pIn1);
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_CAST) || !defined(SQLITE_OMIT_ANALYZE)
/* Opcode: Cast P1 P2 * * *
** Synopsis: affinity(r[P1])
**
** Force the value in register P1 to be the type defined by P2.
**
** <ul>
** <li> P2=='A' &rarr; BLOB
** <li> P2=='B' &rarr; TEXT
** <li> P2=='C' &rarr; NUMERIC
** <li> P2=='D' &rarr; INTEGER
** <li> P2=='E' &rarr; REAL
** </ul>
**
** A NULL value is not changed by this routine.  It remains NULL.
*/
ಸ್ಥಿತಿ OP_Cast: {                  /* in1 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>=SQLITE_AFF_BLOB && pOp->p2<=SQLITE_AFF_REAL );
  testcase( pOp->p2==SQLITE_AFF_ಪಠ್ಯ );
  testcase( pOp->p2==SQLITE_AFF_BLOB );
  testcase( pOp->p2==SQLITE_AFF_NUMERIC );
  testcase( pOp->p2==SQLITE_AFF_INTEGER );
  testcase( pOp->p2==SQLITE_AFF_REAL );
  pIn1 = &aMem[pOp->p1];
  mem_About_ಗಮ್ಯ_Change(p, pIn1);
  rc = ExpandBlob(pIn1);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  rc = sqlite3VdbeMemCast(pIn1, pOp->p2, encoding);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  UPDATE_MAX_BLOBSIZE(pIn1);
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p1, pIn1);
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_CAST */

/* Opcode: Eq P1 P2 P3 P4 P5
** Synopsis: IF r[P3]==r[P1]
**
** Compare the values in register P1 and P3.  If reg(P3)==reg(P1) then
** jump to address P2.
**
** The SQLITE_AFF_MASK portion of P5 must be an affinity character -
** SQLITE_AFF_TEXT, SQLITE_AFF_INTEGER, and so forth. An attempt is made
** to coerce both inputs according to this affinity before the
** comparison is made. If the SQLITE_AFF_MASK is 0x00, then numeric
** affinity is used. Note that the affinity conversions are stored
** back into the input registers P1 and P3.  So this opcode can cause
** persistent changes to registers P1 and P3.
**
** Once any conversions have taken place, and neither value is NULL,
** the values are compared. If both values are blobs then memcmp() is
** used to determine the results of the comparison.  If both values
** are text, then the appropriate collating function specified in
** P4 is used to do the comparison.  If P4 is not specified then
** memcmp() is used to compare text string.  If both values are
** numeric, then a numeric comparison is used. If the two values
** are of different types, then numbers are considered less than
** strings and strings are considered less than blobs.
**
** If SQLITE_NULLEQ is set in P5 then the result of comparison is always either
** true or false and is never NULL.  If both operands are NULL then the result
** of comparison is true.  If either operand is NULL then the result is false.
** If neither operand is NULL the result is the same as it would be if
** the SQLITE_NULLEQ flag were omitted from P5.
**
** This opcode saves the result of comparison for use by the new
** OP_Jump opcode.
*/
/* Opcode: Ne P1 P2 P3 P4 P5
** Synopsis: IF r[P3]!=r[P1]
**
** This works just like the Eq opcode except that the jump is taken if
** the operands in registers P1 and P3 are not equal.  See the Eq opcode for
** additional information.
*/
/* Opcode: Lt P1 P2 P3 P4 P5
** Synopsis: IF r[P3]<r[P1]
**
** Compare the values in register P1 and P3.  If reg(P3)<reg(P1) then
** jump to address P2.
**
** If the SQLITE_JUMPIFNULL bit of P5 is set and either reg(P1) or
** reg(P3) is NULL then the take the jump.  If the SQLITE_JUMPIFNULL
** bit is clear then fall through if either operand is NULL.
**
** The SQLITE_AFF_MASK portion of P5 must be an affinity character -
** SQLITE_AFF_TEXT, SQLITE_AFF_INTEGER, and so forth. An attempt is made
** to coerce both inputs according to this affinity before the
** comparison is made. If the SQLITE_AFF_MASK is 0x00, then numeric
** affinity is used. Note that the affinity conversions are stored
** back into the input registers P1 and P3.  So this opcode can cause
** persistent changes to registers P1 and P3.
**
** Once any conversions have taken place, and neither value is NULL,
** the values are compared. If both values are blobs then memcmp() is
** used to determine the results of the comparison.  If both values
** are text, then the appropriate collating function specified in
** P4 is  used to do the comparison.  If P4 is not specified then
** memcmp() is used to compare text string.  If both values are
** numeric, then a numeric comparison is used. If the two values
** are of different types, then numbers are considered less than
** strings and strings are considered less than blobs.
**
** This opcode saves the result of comparison for use by the new
** OP_Jump opcode.
*/
/* Opcode: Le P1 P2 P3 P4 P5
** Synopsis: IF r[P3]<=r[P1]
**
** This works just like the Lt opcode except that the jump is taken if
** the content of register P3 is less than or equal to the content of
** register P1.  See the Lt opcode for additional information.
*/
/* Opcode: Gt P1 P2 P3 P4 P5
** Synopsis: IF r[P3]>r[P1]
**
** This works just like the Lt opcode except that the jump is taken if
** the content of register P3 is greater than the content of
** register P1.  See the Lt opcode for additional information.
*/
/* Opcode: Ge P1 P2 P3 P4 P5
** Synopsis: IF r[P3]>=r[P1]
**
** This works just like the Lt opcode except that the jump is taken if
** the content of register P3 is greater than or equal to the content of
** register P1.  See the Lt opcode for additional information.
*/
ಸ್ಥಿತಿ OP_Eq:               /* same as TK_EQ, jump, in1, in3 */
ಸ್ಥಿತಿ OP_Ne:               /* same as TK_NE, jump, in1, in3 */
ಸ್ಥಿತಿ OP_Lt:               /* same as TK_LT, jump, in1, in3 */
ಸ್ಥಿತಿ OP_Le:               /* same as TK_LE, jump, in1, in3 */
ಸ್ಥಿತಿ OP_Gt:               /* same as TK_GT, jump, in1, in3 */
ಸ್ಥಿತಿ OP_Ge: {             /* same as TK_GE, jump, in1, in3 */
  ಪೂರ್ಣಾಂಕ res, res2;      /* Result of the comparison of pIn1 against pIn3 */
  ಅಕ್ಷರ affinity;      /* Affinity to use for comparison */
  u16 flags1;         /* Copy of initial value of pIn1->flags */
  u16 flags3;         /* Copy of initial value of pIn3->flags */

  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  flags1 = pIn1->flags;
  flags3 = pIn3->flags;
  ಆದರೆ( (flags1 & flags3 & MEM_Int)!=0 ){
    /* Common case of comparison of two integers */
    ಆದರೆ( pIn3->u.i > pIn1->u.i ){
      ಆದರೆ( sqlite3aGTb[pOp->opcode] ){
        Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        ಹೋಗು jump_ಗಮ್ಯ_p2;
      }
      iCompare = +1;
      VVA_ONLY( iCompareIsInit = 1; )
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pIn3->u.i < pIn1->u.i ){
      ಆದರೆ( sqlite3aLTb[pOp->opcode] ){
        Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        ಹೋಗು jump_ಗಮ್ಯ_p2;
      }
      iCompare = -1;
      VVA_ONLY( iCompareIsInit = 1; )
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( sqlite3aEQb[pOp->opcode] ){
        Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        ಹೋಗು jump_ಗಮ್ಯ_p2;
      }
      iCompare = 0;
      VVA_ONLY( iCompareIsInit = 1; )
    }
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0, (pOp->p5 & SQLITE_NULLEQ)?2:3);
    ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ( (flags1 | flags3)&MEM_Null ){
    /* One or both operands are NULL */
    ಆದರೆ( pOp->p5 & SQLITE_NULLEQ ){
      /* If SQLITE_NULLEQ is set (which will only happen if the operator is
      ** OP_Eq or OP_Ne) then take the jump or not depending on whether
      ** or not both operands are null.
      */
      ಪ್ರತಿಜ್ಞಾ( (flags1 & MEM_Cleared)==0 );
      ಪ್ರತಿಜ್ಞಾ( (pOp->p5 & SQLITE_JUMPIFNULL)==0 || CORRUPT_DB );
      testcase( (pOp->p5 & SQLITE_JUMPIFNULL)!=0 );
      ಆದರೆ( (flags1&flags3&MEM_Null)!=0
       && (flags3&MEM_Cleared)==0
      ){
        res = 0;  /* Operands are equal */
      }ಇಲ್ಲದಿದ್ದರೆ{
        res = ((flags3 & MEM_Null) ? -1 : +1);  /* Operands are not equal */
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* SQLITE_NULLEQ is clear and at least one operand is NULL,
      ** then the result is always NULL.
      ** The jump is taken if the SQLITE_JUMPIFNULL bit is set.
      */
      Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(2,3);
      ಆದರೆ( pOp->p5 & SQLITE_JUMPIFNULL ){
        ಹೋಗು jump_ಗಮ್ಯ_p2;
      }
      iCompare = 1;    /* Operands are not equal */
      VVA_ONLY( iCompareIsInit = 1; )
      ನಿಲ್ಲಿಸು;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Neither operand is NULL and we couldn't do the special high-speed
    ** integer comparison case.  So do a general-case comparison. */
    affinity = pOp->p5 & SQLITE_AFF_ಮುಖವಾಡ;
    ಆದರೆ( affinity>=SQLITE_AFF_NUMERIC ){
      ಆದರೆ( (flags1 | flags3)&MEM_Str ){
        ಆದರೆ( (flags1 & (MEM_Int|MEM_IntReal|MEM_Real|MEM_Str))==MEM_Str ){
          applyNumericAffinity(pIn1,0);
          ಪ್ರತಿಜ್ಞಾ( flags3==pIn3->flags || CORRUPT_DB );
          flags3 = pIn3->flags;
        }
        ಆದರೆ( (flags3 & (MEM_Int|MEM_IntReal|MEM_Real|MEM_Str))==MEM_Str ){
          applyNumericAffinity(pIn3,0);
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( affinity==SQLITE_AFF_ಪಠ್ಯ && ((flags1 | flags3) & MEM_Str)!=0 ){
      ಆದರೆ( (flags1 & MEM_Str)!=0 ){
        pIn1->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags1&(MEM_Int|MEM_Real|MEM_IntReal))!=0 ){
        testcase( pIn1->flags & MEM_Int );
        testcase( pIn1->flags & MEM_Real );
        testcase( pIn1->flags & MEM_IntReal );
        sqlite3VdbeMemStringify(pIn1, encoding, 1);
        testcase( (flags1&MEM_Dyn) != (pIn1->flags&MEM_Dyn) );
        flags1 = (pIn1->flags & ~MEM_Type_ಮುಖವಾಡ) | (flags1 & MEM_Type_ಮುಖವಾಡ);
        ಆದರೆ( NEVER(pIn1==pIn3) ) flags3 = flags1 | MEM_Str;
      }
      ಆದರೆ( (flags3 & MEM_Str)!=0 ){
        pIn3->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags3&(MEM_Int|MEM_Real|MEM_IntReal))!=0 ){
        testcase( pIn3->flags & MEM_Int );
        testcase( pIn3->flags & MEM_Real );
        testcase( pIn3->flags & MEM_IntReal );
        sqlite3VdbeMemStringify(pIn3, encoding, 1);
        testcase( (flags3&MEM_Dyn) != (pIn3->flags&MEM_Dyn) );
        flags3 = (pIn3->flags & ~MEM_Type_ಮುಖವಾಡ) | (flags3 & MEM_Type_ಮುಖವಾಡ);
      }
    }
    ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_COLLSEQ || pOp->p4.pColl==0 );
    res = sqlite3MemCompare(pIn3, pIn1, pOp->p4.pColl);
  }

  /* At this point, res is negative, zero, or positive if reg[P1] is
  ** less than, equal to, or greater than reg[P3], respectively.  Compute
  ** the answer to this operator in res2, depending on what the comparison
  ** operator actually is.  The next block of code depends on the fact
  ** that the 6 comparison operators are consecutive integers in this
  ** order:  NE, EQ, GT, LE, LT, GE */
  ಪ್ರತಿಜ್ಞಾ( OP_Eq==OP_Ne+1 ); ಪ್ರತಿಜ್ಞಾ( OP_Gt==OP_Ne+2 ); ಪ್ರತಿಜ್ಞಾ( OP_Le==OP_Ne+3 );
  ಪ್ರತಿಜ್ಞಾ( OP_Lt==OP_Ne+4 ); ಪ್ರತಿಜ್ಞಾ( OP_Ge==OP_Ne+5 );
  ಆದರೆ( res<0 ){
    res2 = sqlite3aLTb[pOp->opcode];
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( res==0 ){
    res2 = sqlite3aEQb[pOp->opcode];
  }ಇಲ್ಲದಿದ್ದರೆ{
    res2 = sqlite3aGTb[pOp->opcode];
  }
  iCompare = res;
  VVA_ONLY( iCompareIsInit = 1; )

  /* Undo any changes made by applyAffinity() to the input registers. */
  ಪ್ರತಿಜ್ಞಾ( (pIn3->flags & MEM_Dyn) == (flags3 & MEM_Dyn) );
  pIn3->flags = flags3;
  ಪ್ರತಿಜ್ಞಾ( (pIn1->flags & MEM_Dyn) == (flags1 & MEM_Dyn) );
  pIn1->flags = flags1;

  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res2!=0, (pOp->p5 & SQLITE_NULLEQ)?2:3);
  ಆದರೆ( res2 ){
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: ElseEq * P2 * * *
**
** This opcode must follow an OP_Lt or OP_Gt comparison operator.  There
** can be zero or more OP_ReleaseReg opcodes intervening, but no other
** opcodes are allowed to occur between this instruction and the previous
** OP_Lt or OP_Gt.
**
** If the result of an OP_Eq comparison on the same two operands as
** the prior OP_Lt or OP_Gt would have been true, then jump to P2.  If
** the result of an OP_Eq comparison on the two previous operands
** would have been false or NULL, then fall through.
*/
ಸ್ಥಿತಿ OP_ElseEq: {       /* same as TK_ESCAPE, jump */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Verify the preconditions of this opcode - that it follows an OP_Lt or
  ** OP_Gt with zero or more intervening OP_ReleaseReg opcodes */
  ಪೂರ್ಣಾಂಕ iAddr;
  ಗಾಗಿ(iAddr = (ಪೂರ್ಣಾಂಕ)(pOp - aOp) - 1; ALWAYS(iAddr>=0); iAddr--){
    ಆದರೆ( aOp[iAddr].opcode==OP_ReleaseReg ) ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ( aOp[iAddr].opcode==OP_Lt || aOp[iAddr].opcode==OP_Gt );
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */
  ಪ್ರತಿಜ್ಞಾ( iCompareIsInit );
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(iCompare==0, 2);
  ಆದರೆ( iCompare==0 ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
}


/* Opcode: Permutation * * * P4 *
**
** Set the permutation used by the OP_Compare operator in the next
** instruction.  The permutation is stored in the P4 operand.
**
** The permutation is only valid for the next opcode which must be
** an OP_Compare that has the OPFLAG_PERMUTE bit set in P5.
**
** The first integer in the P4 integer array is the length of the array
** and does not become part of the permutation.
*/
ಸ್ಥಿತಿ OP_Permutation: {
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INTARRAY );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4.ai );
  ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_Compare );
  ಪ್ರತಿಜ್ಞಾ( pOp[1].p5 & OPFLAG_ಕ್ರಮಬದಲಾವಣೆ );
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Compare P1 P2 P3 P4 P5
** Synopsis: r[P1@P3] <-> r[P2@P3]
**
** Compare two vectors of registers in reg(P1)..reg(P1+P3-1) (call this
** vector "A") and in reg(P2)..reg(P2+P3-1) ("B").  Save the result of
** the comparison for use by the next OP_Jump instruct.
**
** If P5 has the OPFLAG_PERMUTE bit set, then the order of comparison is
** determined by the most recent OP_Permutation operator.  If the
** OPFLAG_PERMUTE bit is clear, then register are compared in sequential
** order.
**
** P4 is a KeyInfo structure that defines collating sequences and sort
** orders for the comparison.  The permutation applies to registers
** only.  The KeyInfo elements are used sequentially.
**
** The comparison is a sort comparison, so NULLs compare equal,
** NULLs are less than numbers, numbers are less than strings,
** and strings are less than blobs.
**
** This opcode must be immediately followed by an OP_Jump opcode.
*/
ಸ್ಥಿತಿ OP_Compare: {
  ಪೂರ್ಣಾಂಕ n;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ p1;
  ಪೂರ್ಣಾಂಕ p2;
  ಸ್ಥಿರಾಂಕ ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
  u32 idx;
  CollSeq *pColl;    /* Collating sequence to use on this term */
  ಪೂರ್ಣಾಂಕ bRev;          /* True for DESCENDING sort order */
  u32 *a_ಕ್ರಮಬದಲಾವಣೆ;     /* The permutation */

  ಆದರೆ( (pOp->p5 & OPFLAG_ಕ್ರಮಬದಲಾವಣೆ)==0 ){
    a_ಕ್ರಮಬದಲಾವಣೆ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pOp>aOp );
    ಪ್ರತಿಜ್ಞಾ( pOp[-1].opcode==OP_Permutation );
    ಪ್ರತಿಜ್ಞಾ( pOp[-1].p4type==P4_INTARRAY );
    a_ಕ್ರಮಬದಲಾವಣೆ = pOp[-1].p4.ai + 1;
    ಪ್ರತಿಜ್ಞಾ( a_ಕ್ರಮಬದಲಾವಣೆ!=0 );
  }
  n = pOp->p3;
  p_ಕೀಲಿ_Info = pOp->p4.p_ಕೀಲಿ_Info;
  ಪ್ರತಿಜ್ಞಾ( n>0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags!=0 );
  p1 = pOp->p1;
  p2 = pOp->p2;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( a_ಕ್ರಮಬದಲಾವಣೆ ){
    ಪೂರ್ಣಾಂಕ k, mx = 0;
    ಗಾಗಿ(k=0; k<n; k++) ಆದರೆ( a_ಕ್ರಮಬದಲಾವಣೆ[k]>(u32)mx ) mx = a_ಕ್ರಮಬದಲಾವಣೆ[k];
    ಪ್ರತಿಜ್ಞಾ( p1>0 && p1+mx<=(p->nMem+1 - p->nCursor)+1 );
    ಪ್ರತಿಜ್ಞಾ( p2>0 && p2+mx<=(p->nMem+1 - p->nCursor)+1 );
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( p1>0 && p1+n<=(p->nMem+1 - p->nCursor)+1 );
    ಪ್ರತಿಜ್ಞಾ( p2>0 && p2+n<=(p->nMem+1 - p->nCursor)+1 );
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */
  ಗಾಗಿ(i=0; i<n; i++){
    idx = a_ಕ್ರಮಬದಲಾವಣೆ ? a_ಕ್ರಮಬದಲಾವಣೆ[i] : (u32)i;
    ಪ್ರತಿಜ್ಞಾ( memIsValid(&aMem[p1+idx]) );
    ಪ್ರತಿಜ್ಞಾ( memIsValid(&aMem[p2+idx]) );
    ರಿಜಿಸ್ಟರ್_TRACE(p1+idx, &aMem[p1+idx]);
    ರಿಜಿಸ್ಟರ್_TRACE(p2+idx, &aMem[p2+idx]);
    ಪ್ರತಿಜ್ಞಾ( i<p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ );
    pColl = p_ಕೀಲಿ_Info->aColl[i];
    bRev = (p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[i] & KEYINFO_ಕ್ರಮ_DESC);
    iCompare = sqlite3MemCompare(&aMem[p1+idx], &aMem[p2+idx], pColl);
    VVA_ONLY( iCompareIsInit = 1; )
    ಆದರೆ( iCompare ){
      ಆದರೆ( (p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[i] & KEYINFO_ಕ್ರಮ_BIGNULL)
       && ((aMem[p1+idx].flags & MEM_Null) || (aMem[p2+idx].flags & MEM_Null))
      ){
        iCompare = -iCompare;
      }
      ಆದರೆ( bRev ) iCompare = -iCompare;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_Jump );
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Jump P1 P2 P3 * *
**
** Jump to the instruction at address P1, P2, or P3 depending on whether
** in the most recent OP_Compare instruction the P1 vector was less than,
** equal to, or greater than the P2 vector, respectively.
**
** This opcode must immediately follow an OP_Compare opcode.
*/
ಸ್ಥಿತಿ OP_Jump: {             /* jump */
  ಪ್ರತಿಜ್ಞಾ( pOp>aOp && pOp[-1].opcode==OP_Compare );
  ಪ್ರತಿಜ್ಞಾ( iCompareIsInit );
  ಆದರೆ( iCompare<0 ){
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0,4); pOp = &aOp[pOp->p1 - 1];
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iCompare==0 ){
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1,4); pOp = &aOp[pOp->p2 - 1];
  }ಇಲ್ಲದಿದ್ದರೆ{
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(2,4); pOp = &aOp[pOp->p3 - 1];
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: And P1 P2 P3 * *
** Synopsis: r[P3]=(r[P1] && r[P2])
**
** Take the logical AND of the values in registers P1 and P2 and
** write the result into register P3.
**
** If either P1 or P2 is 0 (false) then the result is 0 even if
** the other input is NULL.  A NULL and true or two NULLs give
** a NULL output.
*/
/* Opcode: Or P1 P2 P3 * *
** Synopsis: r[P3]=(r[P1] || r[P2])
**
** Take the logical OR of the values in register P1 and P2 and
** store the answer in register P3.
**
** If either P1 or P2 is nonzero (true) then the result is 1 (true)
** even if the other input is NULL.  A NULL and false or two NULLs
** give a NULL output.
*/
ಸ್ಥಿತಿ OP_And:              /* same as TK_AND, in1, in2, out3 */
ಸ್ಥಿತಿ OP_Or: {             /* same as TK_OR, in1, in2, out3 */
  ಪೂರ್ಣಾಂಕ v1;    /* Left operand:  0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */
  ಪೂರ್ಣಾಂಕ v2;    /* Right operand: 0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */

  v1 = sqlite3_Vdbe_Boolean_ಮೌಲ್ಯ(&aMem[pOp->p1], 2);
  v2 = sqlite3_Vdbe_Boolean_ಮೌಲ್ಯ(&aMem[pOp->p2], 2);
  ಆದರೆ( pOp->opcode==OP_And ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ and_logic[] = { 0, 0, 0, 0, 1, 2, 0, 2, 2 };
    v1 = and_logic[v1*3+v2];
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ or_logic[] = { 0, 1, 2, 1, 1, 1, 2, 1, 2 };
    v1 = or_logic[v1*3+v2];
  }
  p_ಹೊರಹೋಗು = &aMem[pOp->p3];
  ಆದರೆ( v1==2 ){
    Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Null);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಹೊರಹೋಗು->u.i = v1;
    Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Int);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IsTrue P1 P2 P3 P4 *
** Synopsis: r[P2] = coalesce(r[P1]==TRUE,P3) ^ P4
**
** This opcode implements the IS TRUE, IS FALSE, IS NOT TRUE, and
** IS NOT FALSE operators.
**
** Interpret the value in register P1 as a boolean value.  Store that
** boolean (a 0 or 1) in register P2.  Or if the value in register P1 is
** NULL, then the P3 is stored in register P2.  Invert the answer if P4
** is 1.
**
** The logic is summarized like this:
**
** <ul>
** <li> If P3==0 and P4==0  then  r[P2] := r[P1] IS TRUE
** <li> If P3==1 and P4==1  then  r[P2] := r[P1] IS FALSE
** <li> If P3==0 and P4==1  then  r[P2] := r[P1] IS NOT TRUE
** <li> If P3==1 and P4==0  then  r[P2] := r[P1] IS NOT FALSE
** </ul>
*/
ಸ್ಥಿತಿ OP_IsTrue: {               /* in1, out2 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INT32 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4.i==0 || pOp->p4.i==1 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3==0 || pOp->p3==1 );
  sqlite3_Vdbe_Mem_ಗಣ_Int64(&aMem[pOp->p2],
      sqlite3_Vdbe_Boolean_ಮೌಲ್ಯ(&aMem[pOp->p1], pOp->p3) ^ pOp->p4.i);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Not P1 P2 * * *
** Synopsis: r[P2]= !r[P1]
**
** Interpret the value in register P1 as a boolean value.  Store the
** boolean complement in register P2.  If the value in register P1 is
** NULL, then a NULL is stored in P2.
*/
ಸ್ಥಿತಿ OP_Not: {                /* same as TK_NOT, in1, out2 */
  pIn1 = &aMem[pOp->p1];
  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  ಆದರೆ( (pIn1->flags & MEM_Null)==0 ){
    sqlite3_Vdbe_Mem_ಗಣ_Int64(p_ಹೊರಹೋಗು, !sqlite3_Vdbe_Boolean_ಮೌಲ್ಯ(pIn1,0));
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: BitNot P1 P2 * * *
** Synopsis: r[P2]= ~r[P1]
**
** Interpret the content of register P1 as an integer.  Store the
** ones-complement of the P1 value into register P2.  If P1 holds
** a NULL then store a NULL in P2.
*/
ಸ್ಥಿತಿ OP_ಬಿಟ್_Not: {             /* same as TK_BITNOT, in1, out2 */
  pIn1 = &aMem[pOp->p1];
  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
  ಆದರೆ( (pIn1->flags & MEM_Null)==0 ){
    p_ಹೊರಹೋಗು->flags = MEM_Int;
    p_ಹೊರಹೋಗು->u.i = ~sqlite3_Vdbe_Int_ಮೌಲ್ಯ(pIn1);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Once P1 P2 P3 * *
**
** Fall through to the next instruction the first time this opcode is
** encountered on each invocation of the byte-code program.  Jump to P2
** on the second and all subsequent encounters during the same invocation.
**
** Top-level programs determine first invocation by comparing the P1
** operand against the P1 operand on the OP_Init opcode at the beginning
** of the program.  If the P1 values differ, then fall through and make
** the P1 of this opcode equal to the P1 of OP_Init.  If P1 values are
** the same then take the jump.
**
** For subprograms, there is a bitmask in the VdbeFrame that determines
** whether or not the jump should be taken.  The bitmask is necessary
** because the self-altering code trick does not work for recursive
** triggers.
**
** The P3 operand is not used directly by this opcode.  However P3 is
** used by the code generator as follows:  If this opcode is the start
** of a subroutine and that subroutine uses a Bloom filter, then P3 will
** be the register that holds that Bloom filter.  See tag-202407032019
** in the source code for implementation details.
*/
ಸ್ಥಿತಿ OP_Once: {             /* jump */
  u32 iAddr;                /* Address of this instruction */
  ಪ್ರತಿಜ್ಞಾ( p->aOp[0].opcode==OP_Init );
  ಆದರೆ( p->p_ಚೌಕಟ್ಟು ){
    iAddr = (ಪೂರ್ಣಾಂಕ)(pOp - p->aOp);
    ಆದರೆ( (p->p_ಚೌಕಟ್ಟು->aOnce[iAddr/8] & (1<<(iAddr & 7)))!=0 ){
      Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1, 2);
      ಹೋಗು jump_ಗಮ್ಯ_p2;
    }
    p->p_ಚೌಕಟ್ಟು->aOnce[iAddr/8] |= 1<<(iAddr & 7);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( p->aOp[0].p1==pOp->p1 ){
      Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1, 2);
      ಹೋಗು jump_ಗಮ್ಯ_p2;
    }
  }
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0, 2);
  pOp->p1 = p->aOp[0].p1;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: If P1 P2 P3 * *
**
** Jump to P2 if the value in register P1 is true.  The value
** is considered true if it is numeric and non-zero.  If the value
** in P1 is NULL then take the jump if and only if P3 is non-zero.
*/
ಸ್ಥಿತಿ OP_If:  {               /* jump, in1 */
  ಪೂರ್ಣಾಂಕ c;
  c = sqlite3_Vdbe_Boolean_ಮೌಲ್ಯ(&aMem[pOp->p1], pOp->p3);
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(c!=0, 2);
  ಆದರೆ( c ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IfNot P1 P2 P3 * *
**
** Jump to P2 if the value in register P1 is False.  The value
** is considered false if it has a numeric value of zero.  If the value
** in P1 is NULL then take the jump if and only if P3 is non-zero.
*/
ಸ್ಥಿತಿ OP_IfNot: {            /* jump, in1 */
  ಪೂರ್ಣಾಂಕ c;
  c = !sqlite3_Vdbe_Boolean_ಮೌಲ್ಯ(&aMem[pOp->p1], !pOp->p3);
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(c!=0, 2);
  ಆದರೆ( c ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IsNull P1 P2 * * *
** Synopsis: if r[P1]==NULL goto P2
**
** Jump to P2 if the value in register P1 is NULL.
*/
ಸ್ಥಿತಿ OP_IsNull: {            /* same as TK_ISNULL, jump, in1 */
  pIn1 = &aMem[pOp->p1];
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ( (pIn1->flags & MEM_Null)!=0, 2);
  ಆದರೆ( (pIn1->flags & MEM_Null)!=0 ){
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IsType P1 P2 P3 P4 P5
** Synopsis: if typeof(P1.P3) in P5 goto P2
**
** Jump to P2 if the type of a column in a btree is one of the types specified
** by the P5 bitmask.
**
** P1 is normally a cursor on a btree for which the row decode cache is
** valid through at least column P3.  In other words, there should have been
** a prior OP_Column for column P3 or greater.  If the cursor is not valid,
** then this opcode might give spurious results.
** The the btree row has fewer than P3 columns, then use P4 as the
** datatype.
**
** If P1 is -1, then P3 is a register number and the datatype is taken
** from the value in that register.
**
** P5 is a bitmask of data types.  SQLITE_INTEGER is the least significant
** (0x01) bit. SQLITE_FLOAT is the 0x02 bit. SQLITE_TEXT is 0x04.
** SQLITE_BLOB is 0x08.  SQLITE_NULL is 0x10.
**
** WARNING: This opcode does not reliably distinguish between NULL and REAL
** when P1>=0.  If the database contains a NaN value, this opcode will think
** that the datatype is REAL when it should be NULL.  When P1<0 and the value
** is already stored in register P3, then this opcode does reliably
** distinguish between NULL and REAL.  The problem only arises then P1>=0.
**
** Take the jump to address P2 if and only if the datatype of the
** value determined by P1 and P3 corresponds to one of the bits in the
** P5 bitmask.
**
*/
ಸ್ಥಿತಿ OP_IsType: {        /* jump */
  VdbeCursor *pC;
  u16 type_ಮುಖವಾಡ;
  u32 serialType;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=(-1) && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 || (pOp->p3>=0 && pOp->p3<=(p->nMem+1 - p->nCursor)) );
  ಆದರೆ( pOp->p1>=0 ){
    pC = p->apCsr[pOp->p1];
    ಪ್ರತಿಜ್ಞಾ( pC!=0 );
    ಪ್ರತಿಜ್ಞಾ( pOp->p3>=0 );
    ಆದರೆ( pOp->p3<pC->nHdrParsed ){
      serialType = pC->aType[pOp->p3];
      ಆದರೆ( serialType>=12 ){
        ಆದರೆ( serialType&1 ){
          type_ಮುಖವಾಡ = 0x04;   /* SQLITE_TEXT */
        }ಇಲ್ಲದಿದ್ದರೆ{
          type_ಮುಖವಾಡ = 0x08;   /* SQLITE_BLOB */
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ a_ಮುಖವಾಡ[] = {
           0x10, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x2,
           0x01, 0x01, 0x10, 0x10
        };
        testcase( serialType==0 );
        testcase( serialType==1 );
        testcase( serialType==2 );
        testcase( serialType==3 );
        testcase( serialType==4 );
        testcase( serialType==5 );
        testcase( serialType==6 );
        testcase( serialType==7 );
        testcase( serialType==8 );
        testcase( serialType==9 );
        testcase( serialType==10 );
        testcase( serialType==11 );
        type_ಮುಖವಾಡ = a_ಮುಖವಾಡ[serialType];
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      type_ಮುಖವಾಡ = 1 << (pOp->p4.i - 1);
      testcase( type_ಮುಖವಾಡ==0x01 );
      testcase( type_ಮುಖವಾಡ==0x02 );
      testcase( type_ಮುಖವಾಡ==0x04 );
      testcase( type_ಮುಖವಾಡ==0x08 );
      testcase( type_ಮುಖವಾಡ==0x10 );
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( memIsValid(&aMem[pOp->p3]) );
    type_ಮುಖವಾಡ = 1 << (sqlite3_ಮೌಲ್ಯ_type((sqlite3_ಮೌಲ್ಯ*)&aMem[pOp->p3])-1);
    testcase( type_ಮುಖವಾಡ==0x01 );
    testcase( type_ಮುಖವಾಡ==0x02 );
    testcase( type_ಮುಖವಾಡ==0x04 );
    testcase( type_ಮುಖವಾಡ==0x08 );
    testcase( type_ಮುಖವಾಡ==0x10 );
  }
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ( (type_ಮುಖವಾಡ & pOp->p5)!=0, 2);
  ಆದರೆ( type_ಮುಖವಾಡ & pOp->p5 ){
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: ZeroOrNull P1 P2 P3 * *
** Synopsis: r[P2] = 0 OR NULL
**
** If both registers P1 and P3 are NOT NULL, then store a zero in
** register P2.  If either registers P1 or P3 are NULL then put
** a NULL in register P2.
*/
ಸ್ಥಿತಿ OP_ZeroOrNull: {            /* in1, in2, out2, in3 */
  ಆದರೆ( (aMem[pOp->p1].flags & MEM_Null)!=0
   || (aMem[pOp->p3].flags & MEM_Null)!=0
  ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(aMem + pOp->p2);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Vdbe_Mem_ಗಣ_Int64(aMem + pOp->p2, 0);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: NotNull P1 P2 * * *
** Synopsis: if r[P1]!=NULL goto P2
**
** Jump to P2 if the value in register P1 is not NULL. 
*/
ಸ್ಥಿತಿ OP_NotNull: {            /* same as TK_NOTNULL, jump, in1 */
  pIn1 = &aMem[pOp->p1];
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ( (pIn1->flags & MEM_Null)==0, 2);
  ಆದರೆ( (pIn1->flags & MEM_Null)==0 ){
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IfNullRow P1 P2 P3 * *
** Synopsis: if P1.nullRow then r[P3]=NULL, goto P2
**
** Check the cursor P1 to see if it is currently pointing at a NULL row.
** If it is, then set register P3 to NULL and jump immediately to P2.
** If P1 is not on a NULL row, then fall through without making any
** changes.
**
** If P1 is not an open cursor, then this opcode is a no-op.
*/
ಸ್ಥಿತಿ OP_If_Null_ಅಡ್ಡಸಾಲು: {         /* jump */
  VdbeCursor *pC;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಆದರೆ( pC && pC->null_ಅಡ್ಡಸಾಲು ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(aMem + pOp->p3);
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಥಾನಾಂತರ_SQL_FUNC
/* Opcode: Offset P1 P2 P3 * *
** Synopsis: r[P3] = sqlite_offset(P1)
**
** Store in register r[P3] the byte offset into the database file that is the
** start of the payload for the record at which that cursor P1 is currently
** pointing.
**
** P2 is the column number for the argument to the sqlite_offset() function.
** This opcode does not use P2 itself, but the P2 value is used by the
** code generator.  The P1, P2, and P3 operands to this opcode are the
** same as for OP_Column.
**
** This opcode is only available if SQLite is compiled with the
** -DSQLITE_ENABLE_OFFSET_SQL_FUNC option.
*/
ಸ್ಥಿತಿ OP_ಸ್ಥಾನಾಂತರ: {          /* out3 */
  VdbeCursor *pC;    /* The VDBE cursor */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  p_ಹೊರಹೋಗು = &p->aMem[pOp->p3];
  ಆದರೆ( pC==0 || pC->eCurType!=CURTYPE_BTREE ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( pC->deferredMoveto ){
      rc = sqlite3_Vdbe_ಮುಕ್ತಾಯ_Moveto(pC);
      ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }
    ಆದರೆ( sqlite3BtreeEof(pC->uc.pCursor) ){
      sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_Mem_ಗಣ_Int64(p_ಹೊರಹೋಗು, sqlite3_Btree_ಸ್ಥಾನಾಂತರ(pC->uc.pCursor));
    }
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_OFFSET_SQL_FUNC */

/* Opcode: Column P1 P2 P3 P4 P5
** Synopsis: r[P3]=PX cursor P1 column P2
**
** Interpret the data that cursor P1 points to as a structure built using
** the MakeRecord instruction.  (See the MakeRecord opcode for additional
** information about the format of the data.)  Extract the P2-th column
** from this record.  If there are less than (P2+1)
** values in the record, extract a NULL.
**
** The value extracted is stored in register P3.
**
** If the record contains fewer than P2 fields, then extract a NULL.  Or,
** if the P4 argument is a P4_MEM use the value of the P4 argument as
** the result.
**
** If the OPFLAG_LENGTHARG bit is set in P5 then the result is guaranteed
** to only be used by the length() function or the equivalent.  The content
** of large blobs is not loaded, thus saving CPU cycles.  If the
** OPFLAG_TYPEOFARG bit is set then the result will only be used by the
** typeof() function or the IS NULL or IS NOT NULL operators or the
** equivalent.  In this case, all content loading can be omitted.
*/
ಸ್ಥಿತಿ OP_ಸ್ತಂಭ: {            /* ncycle */
  u32 p2;            /* column number to retrieve */
  VdbeCursor *pC;    /* The VDBE cursor */
  BtCursor *pCrsr;   /* The B-Tree cursor corresponding to pC */
  u32 *a_ಸ್ಥಾನಾಂತರ;      /* aOffset[i] is offset to start of data for i-th column */
  ಪೂರ್ಣಾಂಕ len;           /* The length of the serialized data for the column */
  ಪೂರ್ಣಾಂಕ i;             /* Loop counter */
  Mem *pDest;        /* Where to write the extracted value */
  Mem sMem;          /* For storing the record being decoded */
  ಸ್ಥಿರಾಂಕ u8 *z_ದತ್ತಾಂಶ;   /* Part of the record being decoded */
  ಸ್ಥಿರಾಂಕ u8 *zHdr;    /* Next unparsed byte of the header */
  ಸ್ಥಿರಾಂಕ u8 *z_ಕೊನೆ_Hdr; /* Pointer to first byte after the header */
  u64 offset64;      /* 64-bit offset */
  u32 t;             /* A type code from the record header */
  Mem *pReg;         /* PseudoTable input register */

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  pC = p->apCsr[pOp->p1];
  p2 = (u32)pOp->p2;

op_ಸ್ತಂಭ_restart:
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( p2<(u32)pC->n_ಕ್ಷೇತ್ರ
       || (pC->eCurType==CURTYPE_PSEUDO && pC->seek_ಫಲಿತಾಂಶ==0) );
  a_ಸ್ಥಾನಾಂತರ = pC->a_ಸ್ಥಾನಾಂತರ;
  ಪ್ರತಿಜ್ಞಾ( a_ಸ್ಥಾನಾಂತರ==pC->aType+pC->n_ಕ್ಷೇತ್ರ );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType!=CURTYPE_VTAB );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType!=CURTYPE_PSEUDO || pC->null_ಅಡ್ಡಸಾಲು );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType!=CURTYPE_SORTER );

  ಆದರೆ( pC->cache_ಸ್ಥಿತಿ!=p->cacheCtr ){                /*OPTIMIZATION-IF-FALSE*/
    ಆದರೆ( pC->null_ಅಡ್ಡಸಾಲು ){
      ಆದರೆ( pC->eCurType==CURTYPE_PSEUDO && pC->seek_ಫಲಿತಾಂಶ>0 ){
        /* For the special case of as pseudo-cursor, the seekResult field
        ** identifies the register that holds the record */
        pReg = &aMem[pC->seek_ಫಲಿತಾಂಶ];
        ಪ್ರತಿಜ್ಞಾ( pReg->flags & MEM_Blob );
        ಪ್ರತಿಜ್ಞಾ( memIsValid(pReg) );
        pC->payload_ಗಾತ್ರ = pC->sz_ಅಡ್ಡಸಾಲು = pReg->n;
        pC->a_ಅಡ್ಡಸಾಲು = (u8*)pReg->z;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDest = &aMem[pOp->p3];
        mem_About_ಗಮ್ಯ_Change(p, pDest);
        sqlite3_Vdbe_Mem_ಗಣ_Null(pDest);
        ಹೋಗು op_ಸ್ತಂಭ_ಹೊರಹೋಗು;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      pCrsr = pC->uc.pCursor;
      ಆದರೆ( pC->deferredMoveto ){
        u32 iMap;
        ಪ್ರತಿಜ್ಞಾ( !pC->isEphemeral );
        ಆದರೆ( pC->ub.aAltMap && (iMap = pC->ub.aAltMap[1+p2])>0  ){
          pC = pC->pAltCursor;
          p2 = iMap - 1;
          ಹೋಗು op_ಸ್ತಂಭ_restart;
        }
        rc = sqlite3_Vdbe_ಮುಕ್ತಾಯ_Moveto(pC);
        ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3BtreeCursorHasMoved(pCrsr) ){
        rc = sqlite3VdbeHandleMovedCursor(pC);
        ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
        ಹೋಗು op_ಸ್ತಂಭ_restart;
      }
      ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
      ಪ್ರತಿಜ್ಞಾ( pCrsr );
      ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorIsValid(pCrsr) );
      pC->payload_ಗಾತ್ರ = sqlite3_Btree_Payload_ಗಾತ್ರ(pCrsr);
      pC->a_ಅಡ್ಡಸಾಲು = sqlite3BtreePayloadFetch(pCrsr, &pC->sz_ಅಡ್ಡಸಾಲು);
      ಪ್ರತಿಜ್ಞಾ( pC->sz_ಅಡ್ಡಸಾಲು<=pC->payload_ಗಾತ್ರ );
      ಪ್ರತಿಜ್ಞಾ( pC->sz_ಅಡ್ಡಸಾಲು<=65536 );  /* Maximum page size is 64KiB */
    }
    pC->cache_ಸ್ಥಿತಿ = p->cacheCtr;
    ಆದರೆ( (a_ಸ್ಥಾನಾಂತರ[0] = pC->a_ಅಡ್ಡಸಾಲು[0])<0x80 ){
      pC->i_Hdr_ಸ್ಥಾನಾಂತರ = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pC->i_Hdr_ಸ್ಥಾನಾಂತರ = sqlite3GetVarint32(pC->a_ಅಡ್ಡಸಾಲು, a_ಸ್ಥಾನಾಂತರ);
    }
    pC->nHdrParsed = 0;

    ಆದರೆ( pC->sz_ಅಡ್ಡಸಾಲು<a_ಸ್ಥಾನಾಂತರ[0] ){      /*OPTIMIZATION-IF-FALSE*/
      /* pC->aRow does not have to hold the entire row, but it does at least
      ** need to cover the header of the record.  If pC->aRow does not contain
      ** the complete header, then set it to zero, forcing the header to be
      ** dynamically allocated. */
      pC->a_ಅಡ್ಡಸಾಲು = 0;
      pC->sz_ಅಡ್ಡಸಾಲು = 0;

      /* Make sure a corrupt database has not given us an oversize header.
      ** Do this now to avoid an oversize memory allocation.
      **
      ** Type entries can be between 1 and 5 bytes each.  But 4 and 5 byte
      ** types use so much data space that there can only be 4096 and 32 of
      ** them, respectively.  So the maximum header length results from a
      ** 3-byte type for each of the maximum of 32768 columns plus three
      ** extra bytes for the header length itself.  32768*3 + 3 = 98307.
      */
      ಆದರೆ( a_ಸ್ಥಾನಾಂತರ[0] > 98307 || a_ಸ್ಥಾನಾಂತರ[0] > pC->payload_ಗಾತ್ರ ){
        ಹೋಗು op_ಸ್ತಂಭ_corrupt;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* This is an optimization.  By skipping over the first few tests
      ** (ex: pC->nHdrParsed<=p2) in the next section, we achieve a
      ** measurable performance gain.
      **
      ** This branch is taken even if aOffset[0]==0.  Such a record is never
      ** generated by SQLite, and could be considered corruption, but we
      ** accept it for historical reasons.  When aOffset[0]==0, the code this
      ** branch jumps to reads past the end of the record, but never more
      ** than a few bytes.  Even if the record occurs at the end of the page
      ** content area, the "page header" comes after the page content and so
      ** this overread is harmless.  Similar overreads can occur for a corrupt
      ** database file.
      */
      z_ದತ್ತಾಂಶ = pC->a_ಅಡ್ಡಸಾಲು;
      ಪ್ರತಿಜ್ಞಾ( pC->nHdrParsed<=p2 );         /* Conditional skipped */
      testcase( a_ಸ್ಥಾನಾಂತರ[0]==0 );
      ಹೋಗು op_ಸ್ತಂಭ_read_ಶೀರ್ಷಿಕೆ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3BtreeCursorHasMoved(pC->uc.pCursor) ){
    rc = sqlite3VdbeHandleMovedCursor(pC);
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    ಹೋಗು op_ಸ್ತಂಭ_restart;
  }

  /* Make sure at least the first p2+1 entries of the header have been
  ** parsed and valid information is in aOffset[] and pC->aType[].
  */
  ಆದರೆ( pC->nHdrParsed<=p2 ){
    /* If there is more header available for parsing in the record, try
    ** to extract additional fields up through the p2+1-th field
    */
    ಆದರೆ( pC->i_Hdr_ಸ್ಥಾನಾಂತರ<a_ಸ್ಥಾನಾಂತರ[0] ){
      /* Make sure zData points to enough of the record to cover the header. */
      ಆದರೆ( pC->a_ಅಡ್ಡಸಾಲು==0 ){
        memset(&sMem, 0, ಗಾತ್ರ(sMem));
        rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(pC->uc.pCursor,a_ಸ್ಥಾನಾಂತರ[0],&sMem);
        ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
        z_ದತ್ತಾಂಶ = (u8*)sMem.z;
      }ಇಲ್ಲದಿದ್ದರೆ{
        z_ದತ್ತಾಂಶ = pC->a_ಅಡ್ಡಸಾಲು;
      }
 
      /* Fill in pC->aType[i] and aOffset[i] values through the p2-th field. */
    op_ಸ್ತಂಭ_read_ಶೀರ್ಷಿಕೆ:
      i = pC->nHdrParsed;
      offset64 = a_ಸ್ಥಾನಾಂತರ[i];
      zHdr = z_ದತ್ತಾಂಶ + pC->i_Hdr_ಸ್ಥಾನಾಂತರ;
      z_ಕೊನೆ_Hdr = z_ದತ್ತಾಂಶ + a_ಸ್ಥಾನಾಂತರ[0];
      testcase( zHdr>=z_ಕೊನೆ_Hdr );
      ಮಾಡು{
        ಆದರೆ( (pC->aType[i] = t = zHdr[0])<0x80 ){
          zHdr++;
          offset64 += sqlite3VdbeOneByteSerialTypeLen(t);
        }ಇಲ್ಲದಿದ್ದರೆ{
          zHdr += sqlite3GetVarint32(zHdr, &t);
          pC->aType[i] = t;
          offset64 += sqlite3VdbeSerialTypeLen(t);
        }
        a_ಸ್ಥಾನಾಂತರ[++i] = (u32)(offset64 & 0xffffffff);
      }ಆಗ( (u32)i<=p2 && zHdr<z_ಕೊನೆ_Hdr );

      /* The record is corrupt if any of the following are true:
      ** (1) the bytes of the header extend past the declared header size
      ** (2) the entire header was used but not all data was used
      ** (3) the end of the data extends beyond the end of the record.
      */
      ಆದರೆ( (zHdr>=z_ಕೊನೆ_Hdr && (zHdr>z_ಕೊನೆ_Hdr || offset64!=pC->payload_ಗಾತ್ರ))
       || (offset64 > pC->payload_ಗಾತ್ರ)
      ){
        ಆದರೆ( a_ಸ್ಥಾನಾಂತರ[0]==0 ){
          i = 0;
          zHdr = z_ಕೊನೆ_Hdr;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಆದರೆ( pC->a_ಅಡ್ಡಸಾಲು==0 ) sqlite3VdbeMemRelease(&sMem);
          ಹೋಗು op_ಸ್ತಂಭ_corrupt;
        }
      }

      pC->nHdrParsed = i;
      pC->i_Hdr_ಸ್ಥಾನಾಂತರ = (u32)(zHdr - z_ದತ್ತಾಂಶ);
      ಆದರೆ( pC->a_ಅಡ್ಡಸಾಲು==0 ) sqlite3VdbeMemRelease(&sMem);
    }ಇಲ್ಲದಿದ್ದರೆ{
      t = 0;
    }

    /* If after trying to extract new entries from the header, nHdrParsed is
    ** still not up to p2, that means that the record has fewer than p2
    ** columns.  So the result will be either the default value or a NULL.
    */
    ಆದರೆ( pC->nHdrParsed<=p2 ){
      pDest = &aMem[pOp->p3];
      mem_About_ಗಮ್ಯ_Change(p, pDest);
      ಆದರೆ( pOp->p4type==P4_MEM ){
        sqlite3VdbeMemShallowCopy(pDest, pOp->p4.pMem, MEM_Static);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_Mem_ಗಣ_Null(pDest);
      }
      ಹೋಗು op_ಸ್ತಂಭ_ಹೊರಹೋಗು;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    t = pC->aType[p2];
  }

  /* Extract the content for the p2+1-th column.  Control can only
  ** reach this point if aOffset[p2], aOffset[p2+1], and pC->aType[p2] are
  ** all valid.
  */
  ಪ್ರತಿಜ್ಞಾ( p2<pC->nHdrParsed );
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  pDest = &aMem[pOp->p3];
  mem_About_ಗಮ್ಯ_Change(p, pDest);
  ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCheckMemInvariants(pDest) );
  ಆದರೆ( VdbeMemDynamic(pDest) ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(pDest);
  }
  ಪ್ರತಿಜ್ಞಾ( t==pC->aType[p2] );
  ಆದರೆ( pC->sz_ಅಡ್ಡಸಾಲು>=a_ಸ್ಥಾನಾಂತರ[p2+1] ){
    /* This is the common case where the desired content fits on the original
    ** page - where the content is not on an overflow page.
    **
    ** The big switch() is an in-line variant of sqlite3VdbeSerialGet() that
    ** has been optimized for the OP_Column opcode.
    */
    z_ದತ್ತಾಂಶ = pC->a_ಅಡ್ಡಸಾಲು + a_ಸ್ಥಾನಾಂತರ[p2];
    ಆಯ್ಕೆ( t ){
      ಸ್ಥಿತಿ 0:
      ಸ್ಥಿತಿ 11:
        pDest->flags = MEM_Null;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 1:
        pDest->u.i = ONE_BYTE_INT(z_ದತ್ತಾಂಶ);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 2:
        pDest->u.i = TWO_BYTE_INT(z_ದತ್ತಾಂಶ);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 3:
        pDest->u.i = THREE_BYTE_INT(z_ದತ್ತಾಂಶ);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 4:
        pDest->u.i = FOUR_BYTE_INT(z_ದತ್ತಾಂಶ);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 5:
        pDest->u.i = SIX_BYTE_INT(z_ದತ್ತಾಂಶ);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 6: {
        pDest->u.i = (i64)sqlite3Get8byte(z_ದತ್ತಾಂಶ);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 7: {
        u64 x = sqlite3Get8byte(z_ದತ್ತಾಂಶ);
        ಅದಲುಬದಲು_Mixed_Endian_Float(x);
        pDest->flags = IsNaN(x) ? MEM_Null : MEM_Real;
        ಸ್ಮರಣೆ_ನಕಲು(&pDest->u.r, &x, ಗಾತ್ರ(x));
        testcase( pDest->u.r<0 );
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 8:
      ಸ್ಥಿತಿ 9: {
        pDest->u.i = t-8;
        pDest->flags = MEM_Int;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 10:
        /* Internal use only: NULL with virtual table
        ** UPDATE no-change flag set */
        pDest->flags = MEM_Null|MEM_Zero;
        pDest->u.nZero = 0;
        pDest->n = 0;
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ: {
        /* If the column value is a string or blob, we need a persistent
        ** value, not a MEM_Ephem value.  This case is a fast short-cut
        ** that is equivalent to calling sqlite3VdbeSerialGet() and
        ** sqlite3VdbeDeephemeralize().
        */
        ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u16 aFlag[] = { MEM_Blob, MEM_Str|MEM_ಪದ };
        pDest->n = len = (t-12)/2;
        pDest->enc = encoding;
        ಆದರೆ( pDest->szMalloc < len+2 ){
          ಆದರೆ( len>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ) ಹೋಗು too_big;
          pDest->flags = MEM_Null;
          ಆದರೆ( sqlite3VdbeMemGrow(pDest, len+2, 0) ) ಹೋಗು no_mem;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pDest->z = pDest->zMalloc;
        }
        ಸ್ಮರಣೆ_ನಕಲು(pDest->z, z_ದತ್ತಾಂಶ, len);
        pDest->z[len] = 0;
        pDest->z[len+1] = 0;
        pDest->flags = aFlag[t&1];
      }
    } /* End of switch */
  }ಇಲ್ಲದಿದ್ದರೆ{
    u8 p5;
    pDest->enc = encoding;
    ಪ್ರತಿಜ್ಞಾ( pDest->db==db );
    /* This branch happens only when content is on overflow pages */
    ಆದರೆ( ((p5 = (pOp->p5 & OPFLAG_BYTELENARG))!=0
          && (p5==OPFLAG_TYPEOFARG
              || (t>=12 && ((t&1)==0 || p5==OPFLAG_BYTELENARG))
             )
        )
     || sqlite3VdbeSerialTypeLen(t)==0
    ){
      /* Content is irrelevant for
      **    1. the typeof() function,
      **    2. the length(X) function if X is a blob, and
      **    3. if the content length is zero.
      ** So we might as well use bogus content rather than reading
      ** content from disk.
      **
      ** Although sqlite3VdbeSerialGet() may read at most 8 bytes from the
      ** buffer passed to it, debugging function VdbeMemPrettyPrint() may
      ** read more.  Use the global constant sqlite3CtypeMap[] as the array,
      ** as that array is 256 bytes long (plenty for VdbeMemPrettyPrint())
      ** and it begins with a bunch of zeros.
      */
      sqlite3VdbeSerialGet((u8*)sqlite3CtypeMap, t, pDest);
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = vdbe_ಸ್ತಂಭ_From_ಅತಿಪ್ರವಾಹ(pC, p2, t, a_ಸ್ಥಾನಾಂತರ[p2],
                p->cacheCtr, ಲಂಬಸಾಲು_Cache_Ctr, pDest);
      ಆದರೆ( rc ){
        ಆದರೆ( rc==SQLITE_NOMEM ) ಹೋಗು no_mem;
        ಆದರೆ( rc==SQLITE_TOOBIG ) ಹೋಗು too_big;
        ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }
    }
  }

op_ಸ್ತಂಭ_ಹೊರಹೋಗು:
  UPDATE_MAX_BLOBSIZE(pDest);
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3, pDest);
  ನಿಲ್ಲಿಸು;

op_ಸ್ತಂಭ_corrupt:
  ಆದರೆ( aOp[0].p3>0 ){
    pOp = &aOp[aOp[0].p3-1];
    ನಿಲ್ಲಿಸು;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_CORRUPT_BKPT;
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
}

/* Opcode: TypeCheck P1 P2 P3 P4 *
** Synopsis: typecheck(r[P1@P2])
**
** Apply affinities to the range of P2 registers beginning with P1.
** Take the affinities from the Table object in P4.  If any value
** cannot be coerced into the correct type, then raise an error.
**
** If P3==0, then omit checking of VIRTUAL columns.
**
** If P3==1, then omit checking of all generated column, both VIRTUAL
** and STORED.
**
** If P3>=2, then only check column number P3-2 in the table (which will
** be a VIRTUAL column) against the value in reg[P1].  In this case,
** P2 will be 1.
**
** This opcode is similar to OP_Affinity except that this opcode
** forces the register type to the Table column type.  This is used
** to implement "strict affinity".
**
** GENERATED ALWAYS AS ... STATIC columns are only checked if P3
** is zero.  When P3 is non-zero, no type checking occurs for
** static generated columns.  Virtual columns are computed at query time
** and so they are never checked.
**
** Preconditions:
**
** <ul>
** <li> P2 should be the number of non-virtual columns in the
**      table of P4 unless P3>1, in which case P2 will be 1.
** <li> Table P4 is a STRICT table.
** </ul>
**
** If any precondition is false, an assertion fault occurs.
*/
ಸ್ಥಿತಿ OP_TypeCheck: {
  ಕೋಷ್ಟಕ *pTab;
  ಸ್ತಂಭ *a_ಲಂಬಸಾಲು;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;

  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_ಕೋಷ್ಟಕ );
  pTab = pOp->p4.pTab;
  ಪ್ರತಿಜ್ಞಾ( pTab->tabFlags & TF_Strict );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3>=0 && pOp->p3<pTab->n_ಲಂಬಸಾಲು+2 );
  a_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು;
  pIn1 = &aMem[pOp->p1];
  ಆದರೆ( pOp->p3<2 ){
    ಪ್ರತಿಜ್ಞಾ( pTab->nNVCol==pOp->p2 );
    i = 0;
    n_ಲಂಬಸಾಲು = pTab->n_ಲಂಬಸಾಲು;
  }ಇಲ್ಲದಿದ್ದರೆ{
    i = pOp->p3-2;
    n_ಲಂಬಸಾಲು = i+1;
    ಪ್ರತಿಜ್ಞಾ( i<pTab->n_ಲಂಬಸಾಲು );
    ಪ್ರತಿಜ್ಞಾ( a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL );
    ಪ್ರತಿಜ್ಞಾ( pOp->p2==1 );
  }
  ಗಾಗಿ(; i<n_ಲಂಬಸಾಲು; i++){
    ಆದರೆ( (a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)!=0 && pOp->p3<2 ){
      ಆದರೆ( (a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL)!=0 ) ಮುಂದುವರಿ;
      ಆದರೆ( pOp->p3 ){ pIn1++; ಮುಂದುವರಿ; }
    }
    ಪ್ರತಿಜ್ಞಾ( pIn1 < &aMem[pOp->p1+pOp->p2] );
    applyAffinity(pIn1, a_ಲಂಬಸಾಲು[i].affinity, encoding);
    ಆದರೆ( (pIn1->flags & MEM_Null)==0 ){
      ಆಯ್ಕೆ( a_ಲಂಬಸಾಲು[i].eCType ){
        ಸ್ಥಿತಿ COLTYPE_BLOB: {
          ಆದರೆ( (pIn1->flags & MEM_Blob)==0 ) ಹೋಗು vdbe_type_error;
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ COLTYPE_INTEGER:
        ಸ್ಥಿತಿ COLTYPE_INT: {
          ಆದರೆ( (pIn1->flags & MEM_Int)==0 ) ಹೋಗು vdbe_type_error;
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ COLTYPE_ಪಠ್ಯ: {
          ಆದರೆ( (pIn1->flags & MEM_Str)==0 ) ಹೋಗು vdbe_type_error;
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ COLTYPE_REAL: {
          testcase( (pIn1->flags & (MEM_Real|MEM_IntReal))==MEM_Real );
          ಪ್ರತಿಜ್ಞಾ( (pIn1->flags & MEM_IntReal)==0 );
          ಆದರೆ( pIn1->flags & MEM_Int ){
            /* When applying REAL affinity, if the result is still an MEM_Int
            ** that will fit in 6 bytes, then change the type to MEM_IntReal
            ** so that we keep the high-resolution integer value but know that
            ** the type really wants to be REAL. */
            testcase( pIn1->u.i==140737488355328LL );
            testcase( pIn1->u.i==140737488355327LL );
            testcase( pIn1->u.i==-140737488355328LL );
            testcase( pIn1->u.i==-140737488355329LL );
            ಆದರೆ( pIn1->u.i<=140737488355327LL && pIn1->u.i>=-140737488355328LL){
              pIn1->flags |= MEM_IntReal;
              pIn1->flags &= ~MEM_Int;
            }ಇಲ್ಲದಿದ್ದರೆ{
              pIn1->u.r = (ದ್ವಿಗುಣ)pIn1->u.i;
              pIn1->flags |= MEM_Real;
              pIn1->flags &= ~MEM_Int;
            }
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (pIn1->flags & (MEM_Real|MEM_IntReal))==0 ){
            ಹೋಗು vdbe_type_error;
          }
          ನಿಲ್ಲಿಸು;
        }
        ಪೂರ್ವನಿಯೋಜಿತ: {
          /* COLTYPE_ANY.  Accept anything. */
          ನಿಲ್ಲಿಸು;
        }
      }
    }
    ರಿಜಿಸ್ಟರ್_TRACE((ಪೂರ್ಣಾಂಕ)(pIn1-aMem), pIn1);
    pIn1++;
  }
  ಪ್ರತಿಜ್ಞಾ( pIn1 == &aMem[pOp->p1+pOp->p2] );
  ನಿಲ್ಲಿಸು;

vdbe_type_error:
  sqlite3VdbeError(p, "cannot store %s value in %s column %s.%s",
     vdbeMemTypeName(pIn1), sqlite3StdType[a_ಲಂಬಸಾಲು[i].eCType-1],
     pTab->zName, a_ಲಂಬಸಾಲು[i].zCnName);
  rc = SQLITE_CONSTRAINT_DATATYPE;
  ಹೋಗು abort_due_ಗಮ್ಯ_error;
}

/* Opcode: Affinity P1 P2 * P4 *
** Synopsis: affinity(r[P1@P2])
**
** Apply affinities to a range of P2 registers starting with P1.
**
** P4 is a string that is P2 characters long. The N-th character of the
** string indicates the column affinity that should be used for the N-th
** memory cell in the range.
*/
ಸ್ಥಿತಿ OP_Affinity: {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zAffinity;   /* The affinity to be applied */

  zAffinity = pOp->p4.z;
  ಪ್ರತಿಜ್ಞಾ( zAffinity!=0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 );
  ಪ್ರತಿಜ್ಞಾ( zAffinity[pOp->p2]==0 );
  pIn1 = &aMem[pOp->p1];
  ಆಗ( 1 /*exit-by-break*/ ){
    ಪ್ರತಿಜ್ಞಾ( pIn1 <= &p->aMem[(p->nMem+1 - p->nCursor)] );
    ಪ್ರತಿಜ್ಞಾ( zAffinity[0]==SQLITE_AFF_NONE || memIsValid(pIn1) );
    applyAffinity(pIn1, zAffinity[0], encoding);
    ಆದರೆ( zAffinity[0]==SQLITE_AFF_REAL && (pIn1->flags & MEM_Int)!=0 ){
      pIn1->u.r = (ದ್ವಿಗುಣ)pIn1->u.i;
      pIn1->flags |= MEM_Real;
      pIn1->flags &= ~(MEM_Int|MEM_Str);
    }
    ರಿಜಿಸ್ಟರ್_TRACE((ಪೂರ್ಣಾಂಕ)(pIn1-aMem), pIn1);
    zAffinity++;
    ಆದರೆ( zAffinity[0]==0 ) ನಿಲ್ಲಿಸು;
    pIn1++;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: MakeRecord P1 P2 P3 P4 *
** Synopsis: r[P3]=mkrec(r[P1@P2])
**
** Convert P2 registers beginning with P1 into the [record format]
** use as a data record in a database table or as a key
** in an index.  The OP_Column opcode can decode the record later.
**
** P4 may be a string that is P2 characters long.  The N-th character of the
** string indicates the column affinity that should be used for the N-th
** field of the index key.
**
** The mapping from character to affinity is given by the SQLITE_AFF_
** macros defined in sqliteInt.h.
**
** If P4 is NULL then all index fields have the affinity BLOB.
**
** The meaning of P5 depends on whether or not the SQLITE_ENABLE_NULL_TRIM
** compile-time option is enabled:
**
**   * If SQLITE_ENABLE_NULL_TRIM is enabled, then the P5 is the index
**     of the right-most table that can be null-trimmed.
**
**   * If SQLITE_ENABLE_NULL_TRIM is omitted, then P5 has the value
**     OPFLAG_NOCHNG_MAGIC if the OP_MakeRecord opcode is allowed to
**     accept no-change records with serial_type 10.  This value is
**     only used inside an assert() and does not affect the end result.
*/
ಸ್ಥಿತಿ OP_Make_ದಾಖಲೆ: {
  Mem *pRec;             /* The new record */
  u64 n_ದತ್ತಾಂಶ;             /* Number of bytes of data space */
  ಪೂರ್ಣಾಂಕ nHdr;              /* Number of bytes of header space */
  i64 nByte;             /* Data space required for this record */
  i64 nZero;             /* Number of zero bytes at the end of the record */
  ಪೂರ್ಣಾಂಕ nVarint;           /* Number of bytes in a varint */
  u32 serial_type;       /* Type field */
  Mem *pData0;           /* First field to be combined into the record */
  Mem *p_ಕೊನೆಯ;            /* Last field of the record */
  ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ;            /* Number of fields in the record */
  ಅಕ್ಷರ *zAffinity;       /* The affinity string for the record */
  u32 len;               /* Length of a field */
  u8 *zHdr;              /* Where to write next byte of the header */
  u8 *zPayload;          /* Where to write next byte of the payload */

  /* Assuming the record contains N fields, the record format looks
  ** like this:
  **
  ** ------------------------------------------------------------------------
  ** | hdr-size | type 0 | type 1 | ... | type N-1 | data0 | ... | data N-1 |
  ** ------------------------------------------------------------------------
  **
  ** Data(0) is taken from register P1.  Data(1) comes from register P1+1
  ** and so forth.
  **
  ** Each type field is a varint representing the serial type of the
  ** corresponding data element (see sqlite3VdbeSerialType()). The
  ** hdr-size field is also a varint which is the offset from the beginning
  ** of the record to data0.
  */
  n_ದತ್ತಾಂಶ = 0;         /* Number of bytes of data space */
  nHdr = 0;          /* Number of bytes of header space */
  nZero = 0;         /* Number of zero bytes at the end of the record */
  n_ಕ್ಷೇತ್ರ = pOp->p1;
  zAffinity = pOp->p4.z;
  ಪ್ರತಿಜ್ಞಾ( n_ಕ್ಷೇತ್ರ>0 && pOp->p2>0 && pOp->p2+n_ಕ್ಷೇತ್ರ<=(p->nMem+1 - p->nCursor)+1 );
  pData0 = &aMem[n_ಕ್ಷೇತ್ರ];
  n_ಕ್ಷೇತ್ರ = pOp->p2;
  p_ಕೊನೆಯ = &pData0[n_ಕ್ಷೇತ್ರ-1];

  /* Identify the output register */
  ಪ್ರತಿಜ್ಞಾ( pOp->p3<pOp->p1 || pOp->p3>=pOp->p1+pOp->p2 );
  p_ಹೊರಹೋಗು = &aMem[pOp->p3];
  mem_About_ಗಮ್ಯ_Change(p, p_ಹೊರಹೋಗು);

  /* Apply the requested affinity to all inputs
  */
  ಪ್ರತಿಜ್ಞಾ( pData0<=p_ಕೊನೆಯ );
  ಆದರೆ( zAffinity ){
    pRec = pData0;
    ಮಾಡು{
      applyAffinity(pRec, zAffinity[0], encoding);
      ಆದರೆ( zAffinity[0]==SQLITE_AFF_REAL && (pRec->flags & MEM_Int) ){
        pRec->flags |= MEM_IntReal;
        pRec->flags &= ~(MEM_Int);
      }
      ರಿಜಿಸ್ಟರ್_TRACE((ಪೂರ್ಣಾಂಕ)(pRec-aMem), pRec);
      zAffinity++;
      pRec++;
      ಪ್ರತಿಜ್ಞಾ( zAffinity[0]==0 || pRec<=p_ಕೊನೆಯ );
    }ಆಗ( zAffinity[0] );
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NULL_TRIM
  /* NULLs can be safely trimmed from the end of the record, as long as
  ** as the schema format is 2 or more and none of the omitted columns
  ** have a non-NULL default value.  Also, the record must be left with
  ** at least one field.  If P5>0 then it will be one more than the
  ** index of the right-most column with a non-NULL default value */
  ಆದರೆ( pOp->p5 ){
    ಆಗ( (p_ಕೊನೆಯ->flags & MEM_Null)!=0 && n_ಕ್ಷೇತ್ರ>pOp->p5 ){
      p_ಕೊನೆಯ--;
      n_ಕ್ಷೇತ್ರ--;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Loop through the elements that will make up the record to figure
  ** out how much space is required for the new record.  After this loop,
  ** the Mem.uTemp field of each term should hold the serial-type that will
  ** be used for that term in the generated record:
  **
  **   Mem.uTemp value    type
  **   ---------------    ---------------
  **      0               NULL
  **      1               1-byte signed integer
  **      2               2-byte signed integer
  **      3               3-byte signed integer
  **      4               4-byte signed integer
  **      5               6-byte signed integer
  **      6               8-byte signed integer
  **      7               IEEE float
  **      8               Integer constant 0
  **      9               Integer constant 1
  **     10,11            reserved for expansion
  **    N>=12 and even    BLOB
  **    N>=13 and odd     text
  **
  ** The following additional values are computed:
  **     nHdr        Number of bytes needed for the record header
  **     nData       Number of bytes of data space needed for the record
  **     nZero       Zero bytes at the end of the record
  */
  pRec = p_ಕೊನೆಯ;
  ಮಾಡು{
    ಪ್ರತಿಜ್ಞಾ( memIsValid(pRec) );
    ಆದರೆ( pRec->flags & MEM_Null ){
      ಆದರೆ( pRec->flags & MEM_Zero ){
        /* Values with MEM_Null and MEM_Zero are created by xColumn virtual
        ** table methods that never invoke sqlite3_result_xxxxx() while
        ** computing an unchanging column value in an UPDATE statement.
        ** Give such values a special internal-use-only serial-type of 10
        ** so that they can be passed through to xUpdate and have
        ** a true sqlite3_value_nochange(). */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_NULL_TRIM
        ಪ್ರತಿಜ್ಞಾ( pOp->p5==OPFLAG_NOCHNG_MAGIC || CORRUPT_DB );
#ಆದರೆ_ಅಂತ್ಯ
        pRec->u_ತಾತ್ಕಾಲಿಕ = 10;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pRec->u_ತಾತ್ಕಾಲಿಕ = 0;
      }
      nHdr++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pRec->flags & (MEM_Int|MEM_IntReal) ){
      /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
      i64 i = pRec->u.i;
      u64 uu;
      testcase( pRec->flags & MEM_Int );
      testcase( pRec->flags & MEM_IntReal );
      ಆದರೆ( i<0 ){
        uu = ~i;
      }ಇಲ್ಲದಿದ್ದರೆ{
        uu = i;
      }
      nHdr++;
      testcase( uu==127 );               testcase( uu==128 );
      testcase( uu==32767 );             testcase( uu==32768 );
      testcase( uu==8388607 );           testcase( uu==8388608 );
      testcase( uu==2147483647 );        testcase( uu==2147483648LL );
      testcase( uu==140737488355327LL ); testcase( uu==140737488355328LL );
      ಆದರೆ( uu<=127 ){
        ಆದರೆ( (i&1)==i && p->minWriteFileFormat>=4 ){
          pRec->u_ತಾತ್ಕಾಲಿಕ = 8+(u32)uu;
        }ಇಲ್ಲದಿದ್ದರೆ{
          n_ದತ್ತಾಂಶ++;
          pRec->u_ತಾತ್ಕಾಲಿಕ = 1;
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( uu<=32767 ){
        n_ದತ್ತಾಂಶ += 2;
        pRec->u_ತಾತ್ಕಾಲಿಕ = 2;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( uu<=8388607 ){
        n_ದತ್ತಾಂಶ += 3;
        pRec->u_ತಾತ್ಕಾಲಿಕ = 3;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( uu<=2147483647 ){
        n_ದತ್ತಾಂಶ += 4;
        pRec->u_ತಾತ್ಕಾಲಿಕ = 4;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( uu<=140737488355327LL ){
        n_ದತ್ತಾಂಶ += 6;
        pRec->u_ತಾತ್ಕಾಲಿಕ = 5;
      }ಇಲ್ಲದಿದ್ದರೆ{
        n_ದತ್ತಾಂಶ += 8;
        ಆದರೆ( pRec->flags & MEM_IntReal ){
          /* If the value is IntReal and is going to take up 8 bytes to store
          ** as an integer, then we might as well make it an 8-byte floating
          ** point value */
          pRec->u.r = (ದ್ವಿಗುಣ)pRec->u.i;
          pRec->flags &= ~MEM_IntReal;
          pRec->flags |= MEM_Real;
          pRec->u_ತಾತ್ಕಾಲಿಕ = 7;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pRec->u_ತಾತ್ಕಾಲಿಕ = 6;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pRec->flags & MEM_Real ){
      nHdr++;
      n_ದತ್ತಾಂಶ += 8;
      pRec->u_ತಾತ್ಕಾಲಿಕ = 7;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( db->mallocFailed || pRec->flags&(MEM_Str|MEM_Blob) );
      ಪ್ರತಿಜ್ಞಾ( pRec->n>=0 );
      len = (u32)pRec->n;
      serial_type = (len*2) + 12 + ((pRec->flags & MEM_Str)!=0);
      ಆದರೆ( pRec->flags & MEM_Zero ){
        serial_type += (u32)pRec->u.nZero*2;
        ಆದರೆ( n_ದತ್ತಾಂಶ ){
          ಆದರೆ( sqlite3VdbeMemExpandBlob(pRec) ) ಹೋಗು no_mem;
          len += pRec->u.nZero;
        }ಇಲ್ಲದಿದ್ದರೆ{
          nZero += pRec->u.nZero;
        }
      }
      n_ದತ್ತಾಂಶ += len;
      nHdr += sqlite3VarintLen(serial_type);
      pRec->u_ತಾತ್ಕಾಲಿಕ = serial_type;
    }
    ಆದರೆ( pRec==pData0 ) ನಿಲ್ಲಿಸು;
    pRec--;
  }ಆಗ(1);

  /* EVIDENCE-OF: R-22564-11647 The header begins with a single varint
  ** which determines the total number of bytes in the header. The varint
  ** value is the size of the header in bytes including the size varint
  ** itself. */
  testcase( nHdr==126 );
  testcase( nHdr==127 );
  ಆದರೆ( nHdr<=126 ){
    /* The common case */
    nHdr += 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Rare case of a really large header */
    nVarint = sqlite3VarintLen(nHdr);
    nHdr += nVarint;
    ಆದರೆ( nVarint<sqlite3VarintLen(nHdr) ) nHdr++;
  }
  nByte = nHdr+n_ದತ್ತಾಂಶ;

  /* If we are able to put an over-run area of 7 bytes on the end of the
  ** memory allocation into which the record is being constructed, then
  ** the encoding of integer values can go faster. This is only possible
  ** if SQLITE_MAX_LENGTH is no with 7 of INT32_MAX and if the host CPU
  ** byte-order is known at compile-time.
  */
#ಆದರೆ SQLITE_MAX_ಉದ್ದಳತೆ<=2147483640 && SQLITE_BYTEORDER>0
# define OVERRUN 7   /* We are able to allocate an overrun of 7 bytes */
#ಇಲ್ಲದಿದ್ದರೆ
# define OVERRUN 0   /* No overrun will be available */
#ಆದರೆ_ಅಂತ್ಯ

  /* Make sure the output register has a buffer large enough to store
  ** the new record. The output register (pOp->p3) is not allowed to
  ** be one of the input registers (because the following call to
  ** sqlite3VdbeMemClearAndResize() could clobber the value before it is used).
  */
  ಆದರೆ( nByte+nZero<=p_ಹೊರಹೋಗು->szMalloc-OVERRUN ){
    /* The output register is already large enough to hold the record.
    ** No error checks or buffer enlargement is required */
    p_ಹೊರಹೋಗು->z = p_ಹೊರಹೋಗು->zMalloc;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Need to make sure that the output is not too big and then enlarge
    ** the output register to hold the full result */
    ಆದರೆ( nByte+nZero>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
      ಹೋಗು too_big;
    }
    ಆದರೆ( sqlite3VdbeMemClearAndResize(p_ಹೊರಹೋಗು, (ಪೂರ್ಣಾಂಕ)nByte+OVERRUN) ){
      ಹೋಗು no_mem;
    }
  }
  p_ಹೊರಹೋಗು->n = (ಪೂರ್ಣಾಂಕ)nByte;
  p_ಹೊರಹೋಗು->flags = MEM_Blob;
  ಆದರೆ( nZero ){
    p_ಹೊರಹೋಗು->u.nZero = nZero;
    p_ಹೊರಹೋಗು->flags |= MEM_Zero;
  }
  UPDATE_MAX_BLOBSIZE(p_ಹೊರಹೋಗು);
  zHdr = (u8 *)p_ಹೊರಹೋಗು->z;
  zPayload = zHdr + nHdr;

  /* Write the record */
  ಆದರೆ( nHdr<0x80 ){
    *(zHdr++) = nHdr;
  }ಇಲ್ಲದಿದ್ದರೆ{
    zHdr += sqlite3PutVarint(zHdr,nHdr);
  }
  ಪ್ರತಿಜ್ಞಾ( pData0<=p_ಕೊನೆಯ );
  pRec = pData0;
  ಆಗ( 1 /*exit-by-break*/ ){
    serial_type = pRec->u_ತಾತ್ಕಾಲಿಕ;
    /* EVIDENCE-OF: R-06529-47362 Following the size varint are one or more
    ** additional varints, one per column.
    ** EVIDENCE-OF: R-64536-51728 The values for each column in the record
    ** immediately follow the header. */
    ಆದರೆ( serial_type<=7 ){
      *(zHdr++) = serial_type;
      ಆದರೆ( serial_type==0 ){
        /* NULL value.  No change in zPayload */
      }ಇಲ್ಲದಿದ್ದರೆ{
        u64 v;
        ಆದರೆ( serial_type==7 ){
          ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(v)==ಗಾತ್ರ(pRec->u.r) );
          ಸ್ಮರಣೆ_ನಕಲು(&v, &pRec->u.r, ಗಾತ್ರ(v));
          ಅದಲುಬದಲು_Mixed_Endian_Float(v);
        }ಇಲ್ಲದಿದ್ದರೆ{
          v = pRec->u.i;
        }
        len = sqlite3SmallTypeSizes[serial_type];
        ಪ್ರತಿಜ್ಞಾ( len>=1 && len<=8 && len!=5 && len!=7 );
#ಆದರೆ SQLITE_BYTEORDER==1234
        v = sqlite3BSwap64(v);
        ಆದರೆ( OVERRUN ){
          ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 a_ಸರಿಸು[] = { 0, 56, 48, 40, 32, 16, 0, 0 };
          v >>= a_ಸರಿಸು[serial_type];
          ಸ್ಮರಣೆ_ನಕಲು(zPayload, &v, 8);
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Test this limb by compiling with -DSQLITE_MAX_LENGTH=2147483647 */
          ಸ್ಮರಣೆ_ನಕಲು(zPayload, (u8*)&v + 8 - len, len);
        }
#ಅಥವಾಆದರೆ SQLITE_BYTEORDER==4321
        ಆದರೆ( OVERRUN ){
          ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 a_ಸರಿಸು[] = { 0, 56, 48, 40, 32, 16, 0, 0 };
          v <<= a_ಸರಿಸು[serial_type];
          ಸ್ಮರಣೆ_ನಕಲು(zPayload, &v, 8);
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Test this limb by compiling with -DSQLITE_MAX_LENGTH=2147483647 */
          ಸ್ಮರಣೆ_ನಕಲು(zPayload, (u8*)&v + 8 - len, len);
        }
#ಇಲ್ಲದಿದ್ದರೆ
        /* Test this limb by compiling with -DSQLITE_BYTEORDER=0 */
        ಆಯ್ಕೆ( len ){
          ಪೂರ್ವನಿಯೋಜಿತ: zPayload[7] = (u8)(v&0xff); v >>= 8;
                   zPayload[6] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          ಸ್ಥಿತಿ 6:  zPayload[5] = (u8)(v&0xff); v >>= 8;
                   zPayload[4] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          ಸ್ಥಿತಿ 4:  zPayload[3] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          ಸ್ಥಿತಿ 3:  zPayload[2] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          ಸ್ಥಿತಿ 2:  zPayload[1] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          ಸ್ಥಿತಿ 1:  zPayload[0] = (u8)(v&0xff);
        }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು OVERRUN  /* We are done with the OVERRUN macro now */
        zPayload += len;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( serial_type<0x80 ){
      *(zHdr++) = serial_type;
      ಆದರೆ( serial_type>=14 && pRec->n>0 ){
        ಪ್ರತಿಜ್ಞಾ( pRec->z!=0 );
        ಸ್ಮರಣೆ_ನಕಲು(zPayload, pRec->z, pRec->n);
        zPayload += pRec->n;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      zHdr += sqlite3PutVarint(zHdr, serial_type);
      ಆದರೆ( pRec->n ){
        ಪ್ರತಿಜ್ಞಾ( pRec->z!=0 );
        ಪ್ರತಿಜ್ಞಾ( pRec->z!=(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3CtypeMap );
        ಸ್ಮರಣೆ_ನಕಲು(zPayload, pRec->z, pRec->n);
        zPayload += pRec->n;
      }
    }
    ಆದರೆ( pRec==p_ಕೊನೆಯ ) ನಿಲ್ಲಿಸು;
    pRec++;
  }
  ಪ್ರತಿಜ್ಞಾ( nHdr==(ಪೂರ್ಣಾಂಕ)(zHdr - (u8*)p_ಹೊರಹೋಗು->z) );
  ಪ್ರತಿಜ್ಞಾ( nByte==(ಪೂರ್ಣಾಂಕ)(zPayload - (u8*)p_ಹೊರಹೋಗು->z) );

  ಪ್ರತಿಜ್ಞಾ( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3, p_ಹೊರಹೋಗು);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Count P1 P2 P3 * *
** Synopsis: r[P2]=count()
**
** Store the number of entries (an integer value) in the table or index
** opened by cursor P1 in register P2.
**
** If P3==0, then an exact count is obtained, which involves visiting
** every btree page of the table.  But if P3 is non-zero, an estimate
** is returned based on the current cursor position. 
*/
ಸ್ಥಿತಿ OP_ಎಣಿಕೆ: {         /* out2 */
  i64 n_ನಮೂದು;
  BtCursor *pCrsr;

  ಪ್ರತಿಜ್ಞಾ( p->apCsr[pOp->p1]->eCurType==CURTYPE_BTREE );
  pCrsr = p->apCsr[pOp->p1]->uc.pCursor;
  ಪ್ರತಿಜ್ಞಾ( pCrsr );
  ಆದರೆ( pOp->p3 ){
    n_ನಮೂದು = sqlite3_Btree_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ_Est(pCrsr);
  }ಇಲ್ಲದಿದ್ದರೆ{
    n_ನಮೂದು = 0;  /* Not needed.  Only used to silence a warning. */
    rc = sqlite3_Btree_ಎಣಿಕೆ(db, pCrsr, &n_ನಮೂದು);
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->u.i = n_ನಮೂದು;
  ಹೋಗು check_for_ಅಡಚಣೆ;
}

/* Opcode: Savepoint P1 * * P4 *
**
** Open, release or rollback the savepoint named by parameter P4, depending
** on the value of P1. To open a new savepoint set P1==0 (SAVEPOINT_BEGIN).
** To release (commit) an existing savepoint set P1==1 (SAVEPOINT_RELEASE).
** To rollback an existing savepoint set P1==2 (SAVEPOINT_ROLLBACK).
*/
ಸ್ಥಿತಿ OP_Savepoint: {
  ಪೂರ್ಣಾಂಕ p1;                         /* Value of P1 operand */
  ಅಕ್ಷರ *zName;                    /* Name of savepoint */
  ಪೂರ್ಣಾಂಕ nName;
  Savepoint *pNew;
  Savepoint *pSavepoint;
  Savepoint *pTmp;
  ಪೂರ್ಣಾಂಕ iSavepoint;
  ಪೂರ್ಣಾಂಕ ii;

  p1 = pOp->p1;
  zName = pOp->p4.z;

  /* Assert that the p1 parameter is valid. Also that if there is no open
  ** transaction, then there cannot be any savepoints.
  */
  ಪ್ರತಿಜ್ಞಾ( db->pSavepoint==0 || db->autoCommit==0 );
  ಪ್ರತಿಜ್ಞಾ( p1==SAVEPOINT_BEGIN||p1==SAVEPOINT_RELEASE||p1==SAVEPOINT_ROLLBACK );
  ಪ್ರತಿಜ್ಞಾ( db->pSavepoint || db->isTransactionSavepoint==0 );
  ಪ್ರತಿಜ್ಞಾ( check_Savepoint_ಎಣಿಕೆ(db) );
  ಪ್ರತಿಜ್ಞಾ( p->bIsReader );

  ಆದರೆ( p1==SAVEPOINT_BEGIN ){
    ಆದರೆ( db->nVdbeWrite>0 ){
      /* A new savepoint cannot be created if there are active write
      ** statements (i.e. open read/write incremental blob handles).
      */
      sqlite3VdbeError(p, "cannot open savepoint - SQL statements in progress");
      rc = SQLITE_BUSY;
    }ಇಲ್ಲದಿದ್ದರೆ{
      nName = sqlite3Strlen30(zName);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
      /* This call is Ok even if this savepoint is actually a transaction
      ** savepoint (and therefore should not prompt xSavepoint()) callbacks.
      ** If this is a transaction savepoint being opened, it is guaranteed
      ** that the db->aVTrans[] array is empty.  */
      ಪ್ರತಿಜ್ಞಾ( db->autoCommit==0 || db->nVTrans==0 );
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_BEGIN,
                                db->nStatement+db->nSavepoint);
      ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
#ಆದರೆ_ಅಂತ್ಯ

      /* Create a new savepoint structure. */
      pNew = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(Savepoint)+nName+1);
      ಆದರೆ( pNew ){
        pNew->zName = (ಅಕ್ಷರ *)&pNew[1];
        ಸ್ಮರಣೆ_ನಕಲು(pNew->zName, zName, nName+1);
   
        /* If there is no open transaction, then mark this as a special
        ** "transaction savepoint". */
        ಆದರೆ( db->autoCommit ){
          db->autoCommit = 0;
          db->isTransactionSavepoint = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          db->nSavepoint++;
        }

        /* Link the new savepoint into the database handle's list. */
        pNew->p_ಮುಂದಿನ = db->pSavepoint;
        db->pSavepoint = pNew;
        pNew->nDeferredCons = db->nDeferredCons;
        pNew->nDeferredImmCons = db->nDeferredImmCons;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( p1==SAVEPOINT_RELEASE || p1==SAVEPOINT_ROLLBACK );
    iSavepoint = 0;

    /* Find the named savepoint. If there is no such savepoint, then an
    ** an error is returned to the user.  */
    ಗಾಗಿ(
      pSavepoint = db->pSavepoint;
      pSavepoint && sqlite3StrICmp(pSavepoint->zName, zName);
      pSavepoint = pSavepoint->p_ಮುಂದಿನ
    ){
      iSavepoint++;
    }
    ಆದರೆ( !pSavepoint ){
      sqlite3VdbeError(p, "no such savepoint: %s", zName);
      rc = SQLITE_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->nVdbeWrite>0 && p1==SAVEPOINT_RELEASE ){
      /* It is not possible to release (commit) a savepoint if there are
      ** active write statements.
      */
      sqlite3VdbeError(p, "cannot release savepoint - "
                          "SQL statements in progress");
      rc = SQLITE_BUSY;
    }ಇಲ್ಲದಿದ್ದರೆ{

      /* Determine whether or not this is a transaction savepoint. If so,
      ** and this is a RELEASE command, then the current transaction
      ** is committed.
      */
      ಪೂರ್ಣಾಂಕ isTransaction = pSavepoint->p_ಮುಂದಿನ==0 && db->isTransactionSavepoint;
      ಆದರೆ( isTransaction && p1==SAVEPOINT_RELEASE ){
        ಆದರೆ( (rc = sqlite3VdbeCheckFkDeferred(p))!=SQLITE_OK ){
          ಹೋಗು vdbe_return;
        }
        db->autoCommit = 1;
        ಆದರೆ( sqlite3VdbeHalt(p)==SQLITE_BUSY ){
          p->pc = (ಪೂರ್ಣಾಂಕ)(pOp - aOp);
          db->autoCommit = 0;
          p->rc = rc = SQLITE_BUSY;
          ಹೋಗು vdbe_return;
        }
        rc = p->rc;
        ಆದರೆ( rc ){
          db->autoCommit = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          db->isTransactionSavepoint = 0;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ isSchemaChange;
        iSavepoint = db->nSavepoint - iSavepoint - 1;
        ಆದರೆ( p1==SAVEPOINT_ROLLBACK ){
          isSchemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0;
          ಗಾಗಿ(ii=0; ii<db->nDb; ii++){
            rc = sqlite3BtreeTripAllCursors(db->aDb[ii].pBt,
                                       SQLITE_ABORT_ROLLBACK,
                                       isSchemaChange==0);
            ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪ್ರತಿಜ್ಞಾ( p1==SAVEPOINT_RELEASE );
          isSchemaChange = 0;
        }
        ಗಾಗಿ(ii=0; ii<db->nDb; ii++){
          rc = sqlite3BtreeSavepoint(db->aDb[ii].pBt, p1, iSavepoint);
          ಆದರೆ( rc!=SQLITE_OK ){
            ಹೋಗು abort_due_ಗಮ್ಯ_error;
          }
        }
        ಆದರೆ( isSchemaChange ){
          sqlite3ExpirePreparedStatements(db, 0);
          sqlite3_Reset_All_Schemas_ದ_Connection(db);
          db->mDbFlags |= DBFLAG_SchemaChange;
        }
      }
      ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
 
      /* Regardless of whether this is a RELEASE or ROLLBACK, destroy all
      ** savepoints nested inside of the savepoint being operated on. */
      ಆಗ( db->pSavepoint!=pSavepoint ){
        pTmp = db->pSavepoint;
        db->pSavepoint = pTmp->p_ಮುಂದಿನ;
        sqlite3DbFree(db, pTmp);
        db->nSavepoint--;
      }

      /* If it is a RELEASE, then destroy the savepoint being operated on
      ** too. If it is a ROLLBACK TO, then set the number of deferred
      ** constraint violations present in the database to the value stored
      ** when the savepoint was created.  */
      ಆದರೆ( p1==SAVEPOINT_RELEASE ){
        ಪ್ರತಿಜ್ಞಾ( pSavepoint==db->pSavepoint );
        db->pSavepoint = pSavepoint->p_ಮುಂದಿನ;
        sqlite3DbFree(db, pSavepoint);
        ಆದರೆ( !isTransaction ){
          db->nSavepoint--;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( p1==SAVEPOINT_ROLLBACK );
        db->nDeferredCons = pSavepoint->nDeferredCons;
        db->nDeferredImmCons = pSavepoint->nDeferredImmCons;
      }

      ಆದರೆ( !isTransaction || p1==SAVEPOINT_ROLLBACK ){
        rc = sqlite3VtabSavepoint(db, p1, iSavepoint);
        ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }
    }
  }
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ಆದರೆ( p->eVdbeState==VDBE_HALT_STATE ){
    rc = SQLITE_ಮುಗಿದ;
    ಹೋಗು vdbe_return;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: AutoCommit P1 P2 * * *
**
** Set the database auto-commit flag to P1 (1 or 0). If P2 is true, roll
** back any currently active btree transactions. If there are any active
** VMs (apart from this one), then a ROLLBACK fails.  A COMMIT fails if
** there are active writing VMs or active VMs that use shared cache.
**
** This instruction causes the VM to halt.
*/
ಸ್ಥಿತಿ OP_AutoCommit: {
  ಪೂರ್ಣಾಂಕ desiredAutoCommit;
  ಪೂರ್ಣಾಂಕ iRollback;

  desiredAutoCommit = pOp->p1;
  iRollback = pOp->p2;
  ಪ್ರತಿಜ್ಞಾ( desiredAutoCommit==1 || desiredAutoCommit==0 );
  ಪ್ರತಿಜ್ಞಾ( desiredAutoCommit==1 || iRollback==0 );
  ಪ್ರತಿಜ್ಞಾ( db->n_Vdbe_ಸಕ್ರಿಯ>0 );  /* At least this one VM is active */
  ಪ್ರತಿಜ್ಞಾ( p->bIsReader );

  ಆದರೆ( desiredAutoCommit!=db->autoCommit ){
    ಆದರೆ( iRollback ){
      ಪ್ರತಿಜ್ಞಾ( desiredAutoCommit==1 );
      sqlite3RollbackAll(db, SQLITE_ABORT_ROLLBACK);
      db->autoCommit = 1;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( desiredAutoCommit && db->nVdbeWrite>0 ){
      /* If this instruction implements a COMMIT and other VMs are writing
      ** return an error indicating that the other VMs must complete first.
      */
      sqlite3VdbeError(p, "cannot commit transaction - "
                          "SQL statements in progress");
      rc = SQLITE_BUSY;
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (rc = sqlite3VdbeCheckFkDeferred(p))!=SQLITE_OK ){
      ಹೋಗು vdbe_return;
    }ಇಲ್ಲದಿದ್ದರೆ{
      db->autoCommit = (u8)desiredAutoCommit;
    }
    ಆದರೆ( sqlite3VdbeHalt(p)==SQLITE_BUSY ){
      p->pc = (ಪೂರ್ಣಾಂಕ)(pOp - aOp);
      db->autoCommit = (u8)(1-desiredAutoCommit);
      p->rc = rc = SQLITE_BUSY;
      ಹೋಗು vdbe_return;
    }
    sqlite3CloseSavepoints(db);
    ಆದರೆ( p->rc==SQLITE_OK ){
      rc = SQLITE_ಮುಗಿದ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = SQLITE_ERROR;
    }
    ಹೋಗು vdbe_return;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3VdbeError(p,
        (!desiredAutoCommit)?"cannot start a transaction within a transaction":(
        (iRollback)?"cannot rollback - no transaction is active":
                   "cannot commit - no transaction is active"));
        
    rc = SQLITE_ERROR;
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  /*NOTREACHED*/ ಪ್ರತಿಜ್ಞಾ(0);
}

/* Opcode: Transaction P1 P2 P3 P4 P5
**
** Begin a transaction on database P1 if a transaction is not already
** active.
** If P2 is non-zero, then a write-transaction is started, or if a
** read-transaction is already active, it is upgraded to a write-transaction.
** If P2 is zero, then a read-transaction is started.  If P2 is 2 or more
** then an exclusive transaction is started.
**
** P1 is the index of the database file on which the transaction is
** started.  Index 0 is the main database file and index 1 is the
** file used for temporary tables.  Indices of 2 or more are used for
** attached databases.
**
** If a write-transaction is started and the Vdbe.usesStmtJournal flag is
** true (this flag is set if the Vdbe may modify more than one row and may
** throw an ABORT exception), a statement transaction may also be opened.
** More specifically, a statement transaction is opened iff the database
** connection is currently not in autocommit mode, or if there are other
** active statements. A statement transaction allows the changes made by this
** VDBE to be rolled back after an error without having to roll back the
** entire transaction. If no error is encountered, the statement transaction
** will automatically commit when the VDBE halts.
**
** If P5!=0 then this opcode also checks the schema cookie against P3
** and the schema generation counter against P4.
** The cookie changes its value whenever the database schema changes.
** This operation is used to detect when that the cookie has changed
** and that the current process needs to reread the schema.  If the schema
** cookie in P3 differs from the schema cookie in the database header or
** if the schema generation counter in P4 differs from the current
** generation counter, then an SQLITE_SCHEMA error is raised and execution
** halts.  The sqlite3_step() wrapper function might then reprepare the
** statement and rerun it from the beginning.
*/
ಸ್ಥಿತಿ OP_Transaction: {
  Btree *pBt;
  Db *pDb;
  ಪೂರ್ಣಾಂಕ iMeta = 0;

  ಪ್ರತಿಜ್ಞಾ( p->bIsReader );
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 || pOp->p2==0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>=0 && pOp->p2<=2 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, pOp->p1) );
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  ಆದರೆ( pOp->p2 && (db->flags & (SQLITE_QueryOnly|SQLITE_CorruptRdOnly))!=0 ){
    ಆದರೆ( db->flags & SQLITE_QueryOnly ){
      /* Writes prohibited by the "PRAGMA query_only=TRUE" statement */
      rc = SQLITE_READONLY;
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Writes prohibited due to a prior SQLITE_CORRUPT in the current
      ** transaction */
      rc = SQLITE_CORRUPT;
    }
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  pDb = &db->aDb[pOp->p1];
  pBt = pDb->pBt;

  ಆದರೆ( pBt ){
    rc = sqlite3BtreeBeginTrans(pBt, pOp->p2, &iMeta);
    testcase( rc==SQLITE_BUSY_SNAPSHOT );
    testcase( rc==SQLITE_BUSY_RECOVERY );
    ಆದರೆ( rc!=SQLITE_OK ){
      ಆದರೆ( (rc&0xff)==SQLITE_BUSY ){
        p->pc = (ಪೂರ್ಣಾಂಕ)(pOp - aOp);
        p->rc = rc;
        ಹೋಗು vdbe_return;
      }
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }

    ಆದರೆ( p->ಬಳಸುತ್ತದೆ_Stmt_Journal
     && pOp->p2
     && (db->autoCommit==0 || db->nVdbeRead>1)
    ){
      ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE );
      ಆದರೆ( p->iStatement==0 ){
        ಪ್ರತಿಜ್ಞಾ( db->nStatement>=0 && db->nSavepoint>=0 );
        db->nStatement++;
        p->iStatement = db->nSavepoint + db->nStatement;
      }

      rc = sqlite3VtabSavepoint(db, SAVEPOINT_BEGIN, p->iStatement-1);
      ಆದರೆ( rc==SQLITE_OK ){
        rc = sqlite3BtreeBeginStmt(pBt, p->iStatement);
      }

      /* Store the current value of the database handles deferred constraint
      ** counter. If the statement transaction needs to be rolled back,
      ** the value of this counter needs to be restored too.  */
      p->nStmtDefCons = db->nDeferredCons;
      p->nStmtDefImmCons = db->nDeferredImmCons;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( pOp->p5==0 || pOp->p4type==P4_INT32 );
  ಆದರೆ( rc==SQLITE_OK
   && pOp->p5
   && (iMeta!=pOp->p3 || pDb->pSchema->iGeneration!=pOp->p4.i)
  ){
    /*
    ** IMPLEMENTATION-OF: R-03189-51135 As each SQL statement runs, the schema
    ** version is checked to ensure that the schema has not changed since the
    ** SQL statement was prepared.
    */
    sqlite3DbFree(db, p->z_Err_ಸಂದೇಶ);
    p->z_Err_ಸಂದೇಶ = sqlite3DbStrDup(db, "database schema has changed");
    /* If the schema-cookie from the database file matches the cookie
    ** stored with the in-memory representation of the schema, do
    ** not reload the schema from the database file.
    **
    ** If virtual-tables are in use, this is not just an optimization.
    ** Often, v-tables store their data in other SQLite tables, which
    ** are queried from within xNext() and other v-table methods using
    ** prepared queries. If such a query is out-of-date, we do not want to
    ** discard the database schema, as the user code implementing the
    ** v-table would have to be ready for the sqlite3_vtab structure itself
    ** to be invalidated whenever sqlite3_step() is called from within
    ** a v-table method.
    */
    ಆದರೆ( db->aDb[pOp->p1].pSchema->schema_cookie!=iMeta ){
      sqlite3ResetOneSchema(db, pOp->p1);
    }
    p->expired = 1;
    rc = SQLITE_SCHEMA;

    /* Set changeCntOn to 0 to prevent the value returned by sqlite3_changes()
    ** from being modified in sqlite3VdbeHalt(). If this statement is
    ** reprepared, changeCntOn will be set again. */
    p->changeCntOn = 0;
  }
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: ReadCookie P1 P2 P3 * *
**
** Read cookie number P3 from database P1 and write it into register P2.
** P3==1 is the schema version.  P3==2 is the database format.
** P3==3 is the recommended pager cache size, and so forth.  P1==0 is
** the main database file and P1==1 is the database file used to store
** temporary tables.
**
** There must be a read-lock on the database (either a transaction
** must be started or there must be an open cursor) before
** executing this instruction.
*/
ಸ್ಥಿತಿ OP_ReadCookie: {               /* out2 */
  ಪೂರ್ಣಾಂಕ iMeta;
  ಪೂರ್ಣಾಂಕ iDb;
  ಪೂರ್ಣಾಂಕ iCookie;

  ಪ್ರತಿಜ್ಞಾ( p->bIsReader );
  iDb = pOp->p1;
  iCookie = pOp->p3;
  ಪ್ರತಿಜ್ಞಾ( pOp->p3<SQLITE_N_BTREE_META );
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( db->aDb[iDb].pBt!=0 );
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, iDb) );

  sqlite3BtreeGetMeta(db->aDb[iDb].pBt, iCookie, (u32 *)&iMeta);
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->u.i = iMeta;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SetCookie P1 P2 P3 * P5
**
** Write the integer value P3 into cookie number P2 of database P1.
** P2==1 is the schema version.  P2==2 is the database format.
** P2==3 is the recommended pager cache
** size, and so forth.  P1==0 is the main database file and P1==1 is the
** database file used to store temporary tables.
**
** A transaction must be started before executing this opcode.
**
** If P2 is the SCHEMA_VERSION cookie (cookie number 1) then the internal
** schema version is set to P3-P5.  The "PRAGMA schema_version=N" statement
** has P5 set to 1, so that the internal schema version will be different
** from the database schema version, resulting in a schema reset.
*/
ಸ್ಥಿತಿ OP_ಗಣ_Cookie: {
  Db *pDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  ಪ್ರತಿಜ್ಞಾ( pOp->p2<SQLITE_N_BTREE_META );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, pOp->p1) );
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  pDb = &db->aDb[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pDb->pBt!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, pOp->p1, 0) );
  /* See note about index shifting on OP_ReadCookie */
  rc = sqlite3BtreeUpdateMeta(pDb->pBt, pOp->p2, pOp->p3);
  ಆದರೆ( pOp->p2==BTREE_SCHEMA_ಆವೃತ್ತಿ ){
    /* When the schema cookie changes, record the new cookie internally */
    *(u32*)&pDb->pSchema->schema_cookie = *(u32*)&pOp->p3 - pOp->p5;
    db->mDbFlags |= DBFLAG_SchemaChange;
    sqlite3FkClearTriggerCache(db, pOp->p1);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pOp->p2==BTREE_FILE_FORMAT ){
    /* Record changes in the file format */
    pDb->pSchema->file_format = pOp->p3;
  }
  ಆದರೆ( pOp->p1==1 ){
    /* Invalidate all prepared statements whenever the TEMP database
    ** schema is changed.  Ticket #1644 */
    sqlite3ExpirePreparedStatements(db, 0);
    p->expired = 0;
  }
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: OpenRead P1 P2 P3 P4 P5
** Synopsis: root=P2 iDb=P3
**
** Open a read-only cursor for the database table whose root page is
** P2 in a database file.  The database file is determined by P3.
** P3==0 means the main database, P3==1 means the database used for
** temporary tables, and P3>1 means used the corresponding attached
** database.  Give the new cursor an identifier of P1.  The P1
** values need not be contiguous but all P1 values should be small integers.
** It is an error for P1 to be negative.
**
** Allowed P5 bits:
** <ul>
** <li>  <b>0x02 OPFLAG_SEEKEQ</b>: This cursor will only be used for
**       equality lookups (implemented as a pair of opcodes OP_SeekGE/OP_IdxGT
**       of OP_SeekLE/OP_IdxLT)
** </ul>
**
** The P4 value may be either an integer (P4_INT32) or a pointer to
** a KeyInfo structure (P4_KEYINFO). If it is a pointer to a KeyInfo
** object, then table being opened must be an [index b-tree] where the
** KeyInfo object defines the content and collating
** sequence of that index b-tree. Otherwise, if P4 is an integer
** value, then the table being opened must be a [table b-tree] with a
** number of columns no less than the value of P4.
**
** See also: OpenWrite, ReopenIdx
*/
/* Opcode: ReopenIdx P1 P2 P3 P4 P5
** Synopsis: root=P2 iDb=P3
**
** The ReopenIdx opcode works like OP_OpenRead except that it first
** checks to see if the cursor on P1 is already open on the same
** b-tree and if it is this opcode becomes a no-op.  In other words,
** if the cursor is already open, do not reopen it.
**
** The ReopenIdx opcode may only be used with P5==0 or P5==OPFLAG_SEEKEQ
** and with P4 being a P4_KEYINFO object.  Furthermore, the P3 value must
** be the same as every other ReopenIdx or OpenRead for the same cursor
** number.
**
** Allowed P5 bits:
** <ul>
** <li>  <b>0x02 OPFLAG_SEEKEQ</b>: This cursor will only be used for
**       equality lookups (implemented as a pair of opcodes OP_SeekGE/OP_IdxGT
**       of OP_SeekLE/OP_IdxLT)
** </ul>
**
** See also: OP_OpenRead, OP_OpenWrite
*/
/* Opcode: OpenWrite P1 P2 P3 P4 P5
** Synopsis: root=P2 iDb=P3
**
** Open a read/write cursor named P1 on the table or index whose root
** page is P2 (or whose root page is held in register P2 if the
** OPFLAG_P2ISREG bit is set in P5 - see below).
**
** The P4 value may be either an integer (P4_INT32) or a pointer to
** a KeyInfo structure (P4_KEYINFO). If it is a pointer to a KeyInfo
** object, then table being opened must be an [index b-tree] where the
** KeyInfo object defines the content and collating
** sequence of that index b-tree. Otherwise, if P4 is an integer
** value, then the table being opened must be a [table b-tree] with a
** number of columns no less than the value of P4.
**
** Allowed P5 bits:
** <ul>
** <li>  <b>0x02 OPFLAG_SEEKEQ</b>: This cursor will only be used for
**       equality lookups (implemented as a pair of opcodes OP_SeekGE/OP_IdxGT
**       of OP_SeekLE/OP_IdxLT)
** <li>  <b>0x08 OPFLAG_FORDELETE</b>: This cursor is used only to seek
**       and subsequently delete entries in an index btree.  This is a
**       hint to the storage engine that the storage engine is allowed to
**       ignore.  The hint is not used by the official SQLite b*tree storage
**       engine, but is used by COMDB2.
** <li>  <b>0x10 OPFLAG_P2ISREG</b>: Use the content of register P2
**       as the root page, not the value of P2 itself.
** </ul>
**
** This instruction works like OpenRead except that it opens the cursor
** in read/write mode.
**
** See also: OP_OpenRead, OP_ReopenIdx
*/
ಸ್ಥಿತಿ OP_ReopenIdx: {         /* ncycle */
  ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ;
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
  u32 p2;
  ಪೂರ್ಣಾಂಕ iDb;
  ಪೂರ್ಣಾಂಕ wrFlag;
  Btree *pX;
  VdbeCursor *pCur;
  Db *pDb;

  ಪ್ರತಿಜ್ಞಾ( pOp->p5==0 || pOp->p5==OPFLAG_SEEKEQ );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_KEYINFO );
  pCur = p->apCsr[pOp->p1];
  ಆದರೆ( pCur && pCur->pgno_ಬೇರು==(u32)pOp->p2 ){
    ಪ್ರತಿಜ್ಞಾ( pCur->iDb==pOp->p3 );      /* Guaranteed by the code generator */
    ಪ್ರತಿಜ್ಞಾ( pCur->eCurType==CURTYPE_BTREE );
    sqlite3BtreeClearCursor(pCur->uc.pCursor);
    ಹೋಗು open_cursor_ಗಣ_hints;
  }
  /* If the cursor is not currently open or is open on a different
  ** index, then fall through into OP_OpenRead to force a reopen */
ಸ್ಥಿತಿ OP_OpenRead:            /* ncycle */
ಸ್ಥಿತಿ OP_OpenWrite:

  ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_OpenWrite || pOp->p5==0 || pOp->p5==OPFLAG_SEEKEQ );
  ಪ್ರತಿಜ್ಞಾ( p->bIsReader );
  ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_OpenRead || pOp->opcode==OP_ReopenIdx
          || p->readOnly==0 );

  ಆದರೆ( p->expired==1 ){
    rc = SQLITE_ABORT_ROLLBACK;
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }

  n_ಕ್ಷೇತ್ರ = 0;
  p_ಕೀಲಿ_Info = 0;
  p2 = (u32)pOp->p2;
  iDb = pOp->p3;
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, iDb) );
  pDb = &db->aDb[iDb];
  pX = pDb->pBt;
  ಪ್ರತಿಜ್ಞಾ( pX!=0 );
  ಆದರೆ( pOp->opcode==OP_OpenWrite ){
    ಪ್ರತಿಜ್ಞಾ( OPFLAG_FORDELETE==BTREE_FORDELETE );
    wrFlag = BTREE_WRCSR | (pOp->p5 & OPFLAG_FORDELETE);
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಆದರೆ( pDb->pSchema->file_format < p->minWriteFileFormat ){
      p->minWriteFileFormat = pDb->pSchema->file_format;
    }
    ಆದರೆ( pOp->p5 & OPFLAG_P2ISREG ){
      ಪ್ರತಿಜ್ಞಾ( p2>0 );
      ಪ್ರತಿಜ್ಞಾ( p2<=(u32)(p->nMem+1 - p->nCursor) );
      pIn2 = &aMem[p2];
      ಪ್ರತಿಜ್ಞಾ( memIsValid(pIn2) );
      ಪ್ರತಿಜ್ಞಾ( (pIn2->flags & MEM_Int)!=0 );
      sqlite3VdbeMemIntegerify(pIn2);
      p2 = (ಪೂರ್ಣಾಂಕ)pIn2->u.i;
      /* The p2 value always comes from a prior OP_CreateBtree opcode and
      ** that opcode will always set the p2 value to 2 or more or else fail.
      ** If there were a failure, the prepared statement would have halted
      ** before reaching this instruction. */
      ಪ್ರತಿಜ್ಞಾ( p2>=2 );
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    wrFlag = 0;
    ಪ್ರತಿಜ್ಞಾ( (pOp->p5 & OPFLAG_P2ISREG)==0 );
  }
  ಆದರೆ( pOp->p4type==P4_KEYINFO ){
    p_ಕೀಲಿ_Info = pOp->p4.p_ಕೀಲಿ_Info;
    ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->enc==ENC(db) );
    ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->db==db );
    n_ಕ್ಷೇತ್ರ = p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pOp->p4type==P4_INT32 ){
    n_ಕ್ಷೇತ್ರ = pOp->p4.i;
  }
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 );
  ಪ್ರತಿಜ್ಞಾ( n_ಕ್ಷೇತ್ರ>=0 );
  testcase( n_ಕ್ಷೇತ್ರ==0 );  /* Table with INTEGER PRIMARY KEY and nothing else */
  pCur = allocateCursor(p, pOp->p1, n_ಕ್ಷೇತ್ರ, CURTYPE_BTREE);
  ಆದರೆ( pCur==0 ) ಹೋಗು no_mem;
  pCur->iDb = iDb;
  pCur->null_ಅಡ್ಡಸಾಲು = 1;
  pCur->isOrdered = 1;
  pCur->pgno_ಬೇರು = p2;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  pCur->wrFlag = wrFlag;
#ಆದರೆ_ಅಂತ್ಯ
  rc = sqlite3BtreeCursor(pX, p2, wrFlag, p_ಕೀಲಿ_Info, pCur->uc.pCursor);
  pCur->p_ಕೀಲಿ_Info = p_ಕೀಲಿ_Info;
  /* Set the VdbeCursor.isTable variable. Previous versions of
  ** SQLite used to check if the root-page flags were sane at this point
  ** and report database corruption if they were not, but this check has
  ** since moved into the btree layer.  */ 
  pCur->is_ಕೋಷ್ಟಕ = pOp->p4type!=P4_KEYINFO;

open_cursor_ಗಣ_hints:
  ಪ್ರತಿಜ್ಞಾ( OPFLAG_BULKCSR==BTREE_BULKLOAD );
  ಪ್ರತಿಜ್ಞಾ( OPFLAG_SEEKEQ==BTREE_SEEK_EQ );
  testcase( pOp->p5 & OPFLAG_BULKCSR );
  testcase( pOp->p2 & OPFLAG_SEEKEQ );
  sqlite3BtreeCursorHintFlags(pCur->uc.pCursor,
                               (pOp->p5 & (OPFLAG_BULKCSR|OPFLAG_SEEKEQ)));
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: OpenDup P1 P2 * * *
**
** Open a new cursor P1 that points to the same ephemeral table as
** cursor P2.  The P2 cursor must have been opened by a prior OP_OpenEphemeral
** opcode.  Only ephemeral cursors may be duplicated.
**
** Duplicate ephemeral cursors are used for self-joins of materialized views.
*/
ಸ್ಥಿತಿ OP_OpenDup: {           /* ncycle */
  VdbeCursor *pOrig;    /* The original cursor to be duplicated */
  VdbeCursor *pCx;      /* The new cursor */

  pOrig = p->apCsr[pOp->p2];
  ಪ್ರತಿಜ್ಞಾ( pOrig );
  ಪ್ರತಿಜ್ಞಾ( pOrig->isEphemeral );  /* Only ephemeral cursors can be duplicated */

  pCx = allocateCursor(p, pOp->p1, pOrig->n_ಕ್ಷೇತ್ರ, CURTYPE_BTREE);
  ಆದರೆ( pCx==0 ) ಹೋಗು no_mem;
  pCx->null_ಅಡ್ಡಸಾಲು = 1;
  pCx->isEphemeral = 1;
  pCx->p_ಕೀಲಿ_Info = pOrig->p_ಕೀಲಿ_Info;
  pCx->is_ಕೋಷ್ಟಕ = pOrig->is_ಕೋಷ್ಟಕ;
  pCx->pgno_ಬೇರು = pOrig->pgno_ಬೇರು;
  pCx->isOrdered = pOrig->isOrdered;
  pCx->ub.pBtx = pOrig->ub.pBtx;
  pCx->noReuse = 1;
  pOrig->noReuse = 1;
  rc = sqlite3BtreeCursor(pCx->ub.pBtx, pCx->pgno_ಬೇರು, BTREE_WRCSR,
                          pCx->p_ಕೀಲಿ_Info, pCx->uc.pCursor);
  /* The sqlite3BtreeCursor() routine can only fail for the first cursor
  ** opened for a database.  Since there is already an open cursor when this
  ** opcode is run, the sqlite3BtreeCursor() cannot fail */
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  ನಿಲ್ಲಿಸು;
}


/* Opcode: OpenEphemeral P1 P2 P3 P4 P5
** Synopsis: nColumn=P2
**
** Open a new cursor P1 to a transient table.
** The cursor is always opened read/write even if
** the main database is read-only.  The ephemeral
** table is deleted automatically when the cursor is closed.
**
** If the cursor P1 is already opened on an ephemeral table, the table
** is cleared (all content is erased).
**
** P2 is the number of columns in the ephemeral table.
** The cursor points to a BTree table if P4==0 and to a BTree index
** if P4 is not 0.  If P4 is not NULL, it points to a KeyInfo structure
** that defines the format of keys in the index.
**
** The P5 parameter can be a mask of the BTREE_* flags defined
** in btree.h.  These flags control aspects of the operation of
** the btree.  The BTREE_OMIT_JOURNAL and BTREE_SINGLE flags are
** added automatically.
**
** If P3 is positive, then reg[P3] is modified slightly so that it
** can be used as zero-length data for OP_Insert.  This is an optimization
** that avoids an extra OP_Blob opcode to initialize that register.
*/
/* Opcode: OpenAutoindex P1 P2 * P4 *
** Synopsis: nColumn=P2
**
** This opcode works the same as OP_OpenEphemeral.  It has a
** different name to distinguish its use.  Tables created using
** by this opcode will be used for automatically created transient
** indices in joins.
*/
ಸ್ಥಿತಿ OP_OpenAutoindex:       /* ncycle */
ಸ್ಥಿತಿ OP_OpenEphemeral: {     /* ncycle */
  VdbeCursor *pCx;
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;

  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ vfsFlags =
      SQLITE_OPEN_READWRITE |
      SQLITE_OPEN_CREATE |
      SQLITE_OPEN_EXCLUSIVE |
      SQLITE_OPEN_DELETEONCLOSE |
      SQLITE_OPEN_TRANSIENT_DB;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>=0 );
  ಆದರೆ( pOp->p3>0 ){
    /* Make register reg[P3] into a value that can be used as the data
    ** form sqlite3BtreeInsert() where the length of the data is zero. */
    ಪ್ರತಿಜ್ಞಾ( pOp->p2==0 ); /* Only used when number of columns is zero */
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_OpenEphemeral );
    ಪ್ರತಿಜ್ಞಾ( aMem[pOp->p3].flags & MEM_Null );
    aMem[pOp->p3].n = 0;
    aMem[pOp->p3].z = "";
  }
  pCx = p->apCsr[pOp->p1];
  ಆದರೆ( pCx && !pCx->noReuse &&  ALWAYS(pOp->p2<=pCx->n_ಕ್ಷೇತ್ರ) ){
    /* If the ephemeral table is already open and has no duplicates from
    ** OP_OpenDup, then erase all existing content so that the table is
    ** empty again, rather than creating a new table. */
    ಪ್ರತಿಜ್ಞಾ( pCx->isEphemeral );
    pCx->seq_ಎಣಿಕೆ = 0;
    pCx->cache_ಸ್ಥಿತಿ = CACHE_STALE;
    rc = sqlite3_Btree_Clear_ಕೋಷ್ಟಕ(pCx->ub.pBtx, pCx->pgno_ಬೇರು, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pCx = allocateCursor(p, pOp->p1, pOp->p2, CURTYPE_BTREE);
    ಆದರೆ( pCx==0 ) ಹೋಗು no_mem;
    pCx->isEphemeral = 1;
    rc = sqlite3BtreeOpen(db->pVfs, 0, db, &pCx->ub.pBtx,
                          BTREE_OMIT_JOURNAL | BTREE_SINGLE | pOp->p5,
                          vfsFlags);
    ಆದರೆ( rc==SQLITE_OK ){
      rc = sqlite3BtreeBeginTrans(pCx->ub.pBtx, 1, 0);
      ಆದರೆ( rc==SQLITE_OK ){
        /* If a transient index is required, create it by calling
        ** sqlite3BtreeCreateTable() with the BTREE_BLOBKEY flag before
        ** opening it. If a transient table is required, just use the
        ** automatically created table with root-page 1 (an BLOB_INTKEY table).
        */
        ಆದರೆ( (pCx->p_ಕೀಲಿ_Info = p_ಕೀಲಿ_Info = pOp->p4.p_ಕೀಲಿ_Info)!=0 ){
          ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_KEYINFO );
          rc = sqlite3_Btree_Create_ಕೋಷ್ಟಕ(pCx->ub.pBtx, &pCx->pgno_ಬೇರು,
              BTREE_BLOBKEY | pOp->p5);
          ಆದರೆ( rc==SQLITE_OK ){
            ಪ್ರತಿಜ್ಞಾ( pCx->pgno_ಬೇರು==SCHEMA_ಬೇರು+1 );
            ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->db==db );
            ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->enc==ENC(db) );
            rc = sqlite3BtreeCursor(pCx->ub.pBtx, pCx->pgno_ಬೇರು, BTREE_WRCSR,
                p_ಕೀಲಿ_Info, pCx->uc.pCursor);
          }
          pCx->is_ಕೋಷ್ಟಕ = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pCx->pgno_ಬೇರು = SCHEMA_ಬೇರು;
          rc = sqlite3BtreeCursor(pCx->ub.pBtx, SCHEMA_ಬೇರು, BTREE_WRCSR,
              0, pCx->uc.pCursor);
          pCx->is_ಕೋಷ್ಟಕ = 1;
        }
      }
      pCx->isOrdered = (pOp->p5!=BTREE_UNORDERED);
      ಪ್ರತಿಜ್ಞಾ( p->apCsr[pOp->p1]==pCx );
      ಆದರೆ( rc ){
        ಪ್ರತಿಜ್ಞಾ( !sqlite3BtreeClosesWithCursor(pCx->ub.pBtx, pCx->uc.pCursor) );
        sqlite3BtreeClose(pCx->ub.pBtx);
        p->apCsr[pOp->p1] = 0;  /* Not required; helps with static analysis */
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeClosesWithCursor(pCx->ub.pBtx, pCx->uc.pCursor) );
        ಆದರೆ( pOp->opcode==OP_OpenAutoindex ){
          sqlite3BtreeCursorHintFlags(pCx->uc.pCursor, BTREE_SEEK_EQ);
        }
      }
    }
  }
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  pCx->null_ಅಡ್ಡಸಾಲು = 1;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SorterOpen P1 P2 P3 P4 *
**
** This opcode works like OP_OpenEphemeral except that it opens
** a transient index that is specifically designed to sort large
** tables using an external merge-sort algorithm.
**
** If argument P3 is non-zero, then it indicates that the sorter may
** assume that a stable sort considering the first P3 fields of each
** key is sufficient to produce the required results.
*/
ಸ್ಥಿತಿ OP_SorterOpen: {
  VdbeCursor *pCx;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>=0 );
  pCx = allocateCursor(p, pOp->p1, pOp->p2, CURTYPE_SORTER);
  ಆದರೆ( pCx==0 ) ಹೋಗು no_mem;
  pCx->p_ಕೀಲಿ_Info = pOp->p4.p_ಕೀಲಿ_Info;
  ಪ್ರತಿಜ್ಞಾ( pCx->p_ಕೀಲಿ_Info->db==db );
  ಪ್ರತಿಜ್ಞಾ( pCx->p_ಕೀಲಿ_Info->enc==ENC(db) );
  rc = sqlite3VdbeSorterInit(db, pOp->p3, pCx);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SequenceTest P1 P2 * * *
** Synopsis: if( cursor[P1].ctr++ ) pc = P2
**
** P1 is a sorter cursor. If the sequence counter is currently zero, jump
** to P2. Regardless of whether or not the jump is taken, increment the
** the sequence value.
*/
ಸ್ಥಿತಿ OP_SequenceTest: {
  VdbeCursor *pC;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( isSorter(pC) );
  ಆದರೆ( (pC->seq_ಎಣಿಕೆ++)==0 ){
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: OpenPseudo P1 P2 P3 * *
** Synopsis: P3 columns in r[P2]
**
** Open a new cursor that points to a fake table that contains a single
** row of data.  The content of that one row is the content of memory
** register P2.  In other words, cursor P1 becomes an alias for the
** MEM_Blob content contained in register P2.
**
** A pseudo-table created by this opcode is used to hold a single
** row output from the sorter so that the row can be decomposed into
** individual columns using the OP_Column opcode.  The OP_Column opcode
** is the only cursor opcode that works with a pseudo-table.
**
** P3 is the number of fields in the records that will be stored by
** the pseudo-table.  If P2 is 0 or negative then the pseudo-cursor
** will return NULL for every column.
*/
ಸ್ಥಿತಿ OP_OpenPseudo: {
  VdbeCursor *pCx;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3>=0 );
  pCx = allocateCursor(p, pOp->p1, pOp->p3, CURTYPE_PSEUDO);
  ಆದರೆ( pCx==0 ) ಹೋಗು no_mem;
  pCx->null_ಅಡ್ಡಸಾಲು = 1;
  pCx->seek_ಫಲಿತಾಂಶ = pOp->p2;
  pCx->is_ಕೋಷ್ಟಕ = 1;
  /* Give this pseudo-cursor a fake BtCursor pointer so that pCx
  ** can be safely passed to sqlite3VdbeCursorMoveto().  This avoids a test
  ** for pCx->eCurType==CURTYPE_BTREE inside of sqlite3VdbeCursorMoveto()
  ** which is a performance optimization */
  pCx->uc.pCursor = sqlite3BtreeFakeValidCursor();
  ಪ್ರತಿಜ್ಞಾ( pOp->p5==0 );
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Close P1 * * * *
**
** Close a cursor previously opened as P1.  If P1 is not
** currently open, this instruction is a no-op.
*/
ಸ್ಥಿತಿ OP_Close: {             /* ncycle */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  sqlite3VdbeFreeCursor(p, p->apCsr[pOp->p1]);
  p->apCsr[pOp->p1] = 0;
  ನಿಲ್ಲಿಸು;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_ಬಳಸಿದ_ಮುಖವಾಡ
/* Opcode: ColumnsUsed P1 P2 P3 * *
** Synopsis: Cursor P1 uses columns PHEX23
**
** This opcode (which only exists if SQLite was compiled with
** SQLITE_ENABLE_COLUMN_USED_MASK) identifies which columns of the
** table or index for cursor P1 are used.  P2 and P3 combine to give
** a 64-bit unsigned integer mask.  P2 stores the lower 32 bits and
** P3 stores the upper 32 bits.  If the high-order bit of P3 is set
** that means that and 64-th or some later column is used.
*/
ಸ್ಥಿತಿ OP_Columns_ಬಳಸಿದ: {
  VdbeCursor *pC;
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  pC->ಮುಖವಾಡ_ಬಳಸಿದ = INT32_ಗಮ್ಯ_64(pOp->p2,pOp->p3);
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Opcode: SeekGE P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** If cursor P1 refers to an SQL table (B-Tree that uses integer keys),
** use the value in register P3 as the key.  If cursor P1 refers
** to an SQL index, then P3 is the first in an array of P4 registers
** that are used as an unpacked index key.
**
** Reposition cursor P1 so that  it points to the smallest entry that
** is greater than or equal to the key value. If there are no records
** greater than or equal to the key and P2 is not zero, then jump to P2.
**
** If the cursor P1 was opened using the OPFLAG_SEEKEQ flag, then this
** opcode will either land on a record that exactly matches the key, or
** else it will cause a jump to P2.  When the cursor is OPFLAG_SEEKEQ,
** this opcode must be followed by an IdxLE opcode with the same arguments,
** except that the P2 argument (the jump destination) can be different.
** The IdxGT opcode will be skipped if this opcode succeeds, but the
** IdxGT opcode will be used on subsequent loop iterations.  The
** OPFLAG_SEEKEQ flags is a hint to the btree layer to say that this
** is an equality search.
**
** The P2 of the IdxLE might be less than the P2 of this opcode.  The P2
** of the IdxLE will be the location to jump to if there are one or more
** matching rows and is the location to continue with the next row of the
** first containing loop.  The P2 of this opcode will be the location to
** jump to if zero rows of cursor P1 match the key, and that will be the
** continuation address of the first outer loop that contributes to the
** key.  The P2 of this opcode might be greater than the P2 of the
** subsequent IdxLE if the first outer loop that influences the key is
** outer to the immediately next outer loop.
**
** This opcode leaves the cursor configured to move in forward order,
** from the beginning toward the end.  In other words, the cursor is
** configured to use Next, not Prev.
**
** See also: Found, NotFound, SeekLt, SeekGt, SeekLe
*/
/* Opcode: SeekGT P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** If cursor P1 refers to an SQL table (B-Tree that uses integer keys),
** use the value in register P3 as a key. If cursor P1 refers
** to an SQL index, then P3 is the first in an array of P4 registers
** that are used as an unpacked index key.
**
** Reposition cursor P1 so that it points to the smallest entry that
** is greater than the key value. If there are no records greater than
** the key and P2 is not zero, then jump to P2.
**
** This opcode leaves the cursor configured to move in forward order,
** from the beginning toward the end.  In other words, the cursor is
** configured to use Next, not Prev.
**
** See also: Found, NotFound, SeekLt, SeekGe, SeekLe
*/
/* Opcode: SeekLT P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** If cursor P1 refers to an SQL table (B-Tree that uses integer keys),
** use the value in register P3 as a key. If cursor P1 refers
** to an SQL index, then P3 is the first in an array of P4 registers
** that are used as an unpacked index key.
**
** Reposition cursor P1 so that  it points to the largest entry that
** is less than the key value. If there are no records less than
** the key and P2 is not zero, then jump to P2.
**
** This opcode leaves the cursor configured to move in reverse order,
** from the end toward the beginning.  In other words, the cursor is
** configured to use Prev, not Next.
**
** See also: Found, NotFound, SeekGt, SeekGe, SeekLe
*/
/* Opcode: SeekLE P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** If cursor P1 refers to an SQL table (B-Tree that uses integer keys),
** use the value in register P3 as a key. If cursor P1 refers
** to an SQL index, then P3 is the first in an array of P4 registers
** that are used as an unpacked index key.
**
** Reposition cursor P1 so that it points to the largest entry that
** is less than or equal to the key value. If there are no records
** less than or equal to the key and P2 is not zero, then jump to P2.
**
** This opcode leaves the cursor configured to move in reverse order,
** from the end toward the beginning.  In other words, the cursor is
** configured to use Prev, not Next.
**
** If the cursor P1 was opened using the OPFLAG_SEEKEQ flag, then this
** opcode will either land on a record that exactly matches the key, or
** else it will cause a jump to P2.  When the cursor is OPFLAG_SEEKEQ,
** this opcode must be followed by an IdxGE opcode with the same arguments
** except the P2 of IdxGE might be less than the P2 of this opcode.
** The IdxGE opcode will be skipped if this opcode succeeds, but the
** IdxGE opcode will be used on subsequent loop iterations.  The
** OPFLAG_SEEKEQ flags is a hint to the btree layer to say that this
** is an equality search.
**
** The P2 of the IdxGE might be less than the P2 of this opcode.  The P2
** of the IdxGE will be the location to jump to if there are one or more
** matching rows and is the location to continue with the next row of the
** first containing loop.  The P2 of this opcode will be the location to
** jump to if no rows of cursor P1 match the key, and that will be the
** continuation address of the first outer loop that contributes to the
** key. The P2 of this opcode might be greater than the P2 of the
** subsequent IdxLE if the first outer loop that influences the key is
** outer to the immediately next outer loop.
**
** See also: Found, NotFound, SeekGt, SeekGe, SeekLt
*/
ಸ್ಥಿತಿ OP_SeekLT:         /* jump0, in3, group, ncycle */
ಸ್ಥಿತಿ OP_SeekLE:         /* jump0, in3, group, ncycle */
ಸ್ಥಿತಿ OP_SeekGE:         /* jump0, in3, group, ncycle */
ಸ್ಥಿತಿ OP_SeekGT: {       /* jump0, in3, group, ncycle */
  ಪೂರ್ಣಾಂಕ res;           /* Comparison result */
  ಪೂರ್ಣಾಂಕ oc;            /* Opcode */
  VdbeCursor *pC;    /* The cursor to seek */
  Unpacked_ದಾಖಲೆ r;  /* The key to seek for */
  ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ;        /* Number of columns or fields in the key */
  i64 i_ಕೀಲಿ;          /* The rowid we are to seek to */
  ಪೂರ್ಣಾಂಕ eqOnly;        /* Only interested in == results */

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2!=0 );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( OP_SeekLE == OP_SeekLT+1 );
  ಪ್ರತಿಜ್ಞಾ( OP_SeekGE == OP_SeekLT+2 );
  ಪ್ರತಿಜ್ಞಾ( OP_SeekGT == OP_SeekLT+3 );
  ಪ್ರತಿಜ್ಞಾ( pC->isOrdered );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
  oc = pOp->opcode;
  eqOnly = 0;
  pC->null_ಅಡ್ಡಸಾಲು = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#ಆದರೆ_ಅಂತ್ಯ

  pC->deferredMoveto = 0;
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಆದರೆ( pC->is_ಕೋಷ್ಟಕ ){
    u16 flags3, newType;
    /* The OPFLAG_SEEKEQ/BTREE_SEEK_EQ flag is only set on index cursors */
    ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorHasHint(pC->uc.pCursor, BTREE_SEEK_EQ)==0
              || CORRUPT_DB );

    /* The input value in P3 might be of any type: integer, real, string,
    ** blob, or NULL.  But it needs to be an integer before we can do
    ** the seek, so convert it. */
    pIn3 = &aMem[pOp->p3];
    flags3 = pIn3->flags;
    ಆದರೆ( (flags3 & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Str))==MEM_Str ){
      applyNumericAffinity(pIn3, 0);
    }
    i_ಕೀಲಿ = sqlite3_Vdbe_Int_ಮೌಲ್ಯ(pIn3); /* Get the integer key value */
    newType = pIn3->flags; /* Record the type after applying numeric affinity */
    pIn3->flags = flags3;  /* But convert the type back to its original */

    /* If the P3 value could not be converted into an integer without
    ** loss of information, then special processing is required... */
    ಆದರೆ( (newType & (MEM_Int|MEM_IntReal))==0 ){
      ಪೂರ್ಣಾಂಕ c;
      ಆದರೆ( (newType & MEM_Real)==0 ){
        ಆದರೆ( (newType & MEM_Null) || oc>=OP_SeekGE ){
          Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1,2);
          ಹೋಗು jump_ಗಮ್ಯ_p2;
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = sqlite3_Btree_ಕೊನೆಯ(pC->uc.pCursor, &res);
          ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
          ಹೋಗು seek_not_ಸಿಕ್ಕಿತು;
        }
      }
      c = sqlite3IntFloatCompare(i_ಕೀಲಿ, pIn3->u.r);

      /* If the approximation iKey is larger than the actual real search
      ** term, substitute >= for > and < for <=. e.g. if the search term
      ** is 4.9 and the integer approximation 5:
      **
      **        (x >  4.9)    ->     (x >= 5)
      **        (x <= 4.9)    ->     (x <  5)
      */
      ಆದರೆ( c>0 ){
        ಪ್ರತಿಜ್ಞಾ( OP_SeekGE==(OP_SeekGT-1) );
        ಪ್ರತಿಜ್ಞಾ( OP_SeekLT==(OP_SeekLE-1) );
        ಪ್ರತಿಜ್ಞಾ( (OP_SeekLE & 0x0001)==(OP_SeekGT & 0x0001) );
        ಆದರೆ( (oc & 0x0001)==(OP_SeekGT & 0x0001) ) oc--;
      }

      /* If the approximation iKey is smaller than the actual real search
      ** term, substitute <= for < and > for >=.  */
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c<0 ){
        ಪ್ರತಿಜ್ಞಾ( OP_SeekLE==(OP_SeekLT+1) );
        ಪ್ರತಿಜ್ಞಾ( OP_SeekGT==(OP_SeekGE+1) );
        ಪ್ರತಿಜ್ಞಾ( (OP_SeekLT & 0x0001)==(OP_SeekGE & 0x0001) );
        ಆದರೆ( (oc & 0x0001)==(OP_SeekLT & 0x0001) ) oc++;
      }
    }
    rc = sqlite3_Btree_ಕೋಷ್ಟಕ_Moveto(pC->uc.pCursor, (u64)i_ಕೀಲಿ, 0, &res);
    pC->moveto_ಗುರಿ = i_ಕೀಲಿ;  /* Used by OP_Delete */
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* For a cursor with the OPFLAG_SEEKEQ/BTREE_SEEK_EQ hint, only the
    ** OP_SeekGE and OP_SeekLE opcodes are allowed, and these must be
    ** immediately followed by an OP_IdxGT or OP_IdxLT opcode, respectively,
    ** with the same key.
    */
    ಆದರೆ( sqlite3BtreeCursorHasHint(pC->uc.pCursor, BTREE_SEEK_EQ) ){
      eqOnly = 1;
      ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_SeekGE || pOp->opcode==OP_SeekLE );
      ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_IdxLT || pOp[1].opcode==OP_IdxGT );
      ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_SeekGE || pOp[1].opcode==OP_IdxLT );
      ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_SeekLE || pOp[1].opcode==OP_IdxGT );
      ಪ್ರತಿಜ್ಞಾ( pOp[1].p1==pOp[0].p1 );
      ಪ್ರತಿಜ್ಞಾ( pOp[1].p2<=pOp[0].p2 );
      testcase( pOp->opcode==OP_SeekGE && pOp[1].p2<pOp[0].p2 );
      testcase( pOp->opcode==OP_SeekLE && pOp[1].p2<pOp[0].p2 );
      ಪ್ರತಿಜ್ಞಾ( pOp[1].p3==pOp[0].p3 );
      ಪ್ರತಿಜ್ಞಾ( pOp[1].p4.i==pOp[0].p4.i );
    }

    n_ಕ್ಷೇತ್ರ = pOp->p4.i;
    ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INT32 );
    ಪ್ರತಿಜ್ಞಾ( n_ಕ್ಷೇತ್ರ>0 );
    r.p_ಕೀಲಿ_Info = pC->p_ಕೀಲಿ_Info;
    r.n_ಕ್ಷೇತ್ರ = (u16)n_ಕ್ಷೇತ್ರ;

    /* The next line of code computes as follows, only faster:
    **   if( oc==OP_SeekGT || oc==OP_SeekLE ){
    **     r.default_rc = -1;
    **   }else{
    **     r.default_rc = +1;
    **   }
    */
    r.default_rc = ((1 & (oc - OP_SeekLT)) ? -1 : +1);
    ಪ್ರತಿಜ್ಞಾ( oc!=OP_SeekGT || r.default_rc==-1 );
    ಪ್ರತಿಜ್ಞಾ( oc!=OP_SeekLE || r.default_rc==-1 );
    ಪ್ರತಿಜ್ಞಾ( oc!=OP_SeekGE || r.default_rc==+1 );
    ಪ್ರತಿಜ್ಞಾ( oc!=OP_SeekLT || r.default_rc==+1 );

    r.aMem = &aMem[pOp->p3];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    {
      ಪೂರ್ಣಾಂಕ i;
      ಗಾಗಿ(i=0; i<r.n_ಕ್ಷೇತ್ರ; i++){
        ಪ್ರತಿಜ್ಞಾ( memIsValid(&r.aMem[i]) );
        ಆದರೆ( i>0 ) ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3+i, &r.aMem[i]);
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
    r.eq_ನೋಡಿದ = 0;
    rc = sqlite3_Btree_ಸೂಚ್ಯಂಕ_Moveto(pC->uc.pCursor, &r, &res);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }
    ಆದರೆ( eqOnly && r.eq_ನೋಡಿದ==0 ){
      ಪ್ರತಿಜ್ಞಾ( res!=0 );
      ಹೋಗು seek_not_ಸಿಕ್ಕಿತು;
    }
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  sqlite3_ಹುಡುಕು_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( oc>=OP_SeekGE ){  ಪ್ರತಿಜ್ಞಾ( oc==OP_SeekGE || oc==OP_SeekGT );
    ಆದರೆ( res<0 || (res==0 && oc==OP_SeekGT) ){
      res = 0;
      rc = sqlite3_Btree_ಮುಂದಿನ(pC->uc.pCursor, 0);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
          rc = SQLITE_OK;
          res = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಹೋಗು abort_due_ಗಮ್ಯ_error;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      res = 0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( oc==OP_SeekLT || oc==OP_SeekLE );
    ಆದರೆ( res>0 || (res==0 && oc==OP_SeekLT) ){
      res = 0;
      rc = sqlite3_Btree_ಹಿಂದಿನ(pC->uc.pCursor, 0);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
          rc = SQLITE_OK;
          res = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಹೋಗು abort_due_ಗಮ್ಯ_error;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* res might be negative because the table is empty.  Check to
      ** see if this is the case.
      */
      res = sqlite3BtreeEof(pC->uc.pCursor);
    }
  }
seek_not_ಸಿಕ್ಕಿತು:
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 );
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res!=0,2);
  ಆದರೆ( res ){
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( eqOnly ){
    ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_IdxLT || pOp[1].opcode==OP_IdxGT );
    pOp++; /* Skip the OP_IdxLt or OP_IdxGT that follows */
  }
  ನಿಲ್ಲಿಸು;
}


/* Opcode: SeekScan  P1 P2 * * P5
** Synopsis: Scan-ahead up to P1 rows
**
** This opcode is a prefix opcode to OP_SeekGE.  In other words, this
** opcode must be immediately followed by OP_SeekGE. This constraint is
** checked by assert() statements.
**
** This opcode uses the P1 through P4 operands of the subsequent
** OP_SeekGE.  In the text that follows, the operands of the subsequent
** OP_SeekGE opcode are denoted as SeekOP.P1 through SeekOP.P4.   Only
** the P1, P2 and P5 operands of this opcode are also used, and  are called
** This.P1, This.P2 and This.P5.
**
** This opcode helps to optimize IN operators on a multi-column index
** where the IN operator is on the later terms of the index by avoiding
** unnecessary seeks on the btree, substituting steps to the next row
** of the b-tree instead.  A correct answer is obtained if this opcode
** is omitted or is a no-op.
**
** The SeekGE.P3 and SeekGE.P4 operands identify an unpacked key which
** is the desired entry that we want the cursor SeekGE.P1 to be pointing
** to.  Call this SeekGE.P3/P4 row the "target".
**
** If the SeekGE.P1 cursor is not currently pointing to a valid row,
** then this opcode is a no-op and control passes through into the OP_SeekGE.
**
** If the SeekGE.P1 cursor is pointing to a valid row, then that row
** might be the target row, or it might be near and slightly before the
** target row, or it might be after the target row.  If the cursor is
** currently before the target row, then this opcode attempts to position
** the cursor on or after the target row by invoking sqlite3BtreeStep()
** on the cursor between 1 and This.P1 times.
**
** The This.P5 parameter is a flag that indicates what to do if the
** cursor ends up pointing at a valid row that is past the target
** row.  If This.P5 is false (0) then a jump is made to SeekGE.P2.  If
** This.P5 is true (non-zero) then a jump is made to This.P2.  The P5==0
** case occurs when there are no inequality constraints to the right of
** the IN constraint.  The jump to SeekGE.P2 ends the loop.  The P5!=0 case
** occurs when there are inequality constraints to the right of the IN
** operator.  In that case, the This.P2 will point either directly to or
** to setup code prior to the OP_IdxGT or OP_IdxGE opcode that checks for
** loop terminate.
**
** Possible outcomes from this opcode:<ol>
**
** <li> If the cursor is initially not pointed to any valid row, then
**      fall through into the subsequent OP_SeekGE opcode.
**
** <li> If the cursor is left pointing to a row that is before the target
**      row, even after making as many as This.P1 calls to
**      sqlite3BtreeNext(), then also fall through into OP_SeekGE.
**
** <li> If the cursor is left pointing at the target row, either because it
**      was at the target row to begin with or because one or more
**      sqlite3BtreeNext() calls moved the cursor to the target row,
**      then jump to This.P2..,
**
** <li> If the cursor started out before the target row and a call to
**      to sqlite3BtreeNext() moved the cursor off the end of the index
**      (indicating that the target row definitely does not exist in the
**      btree) then jump to SeekGE.P2, ending the loop.
**
** <li> If the cursor ends up on a valid row that is past the target row
**      (indicating that the target row does not exist in the btree) then
**      jump to SeekOP.P2 if This.P5==0 or to This.P2 if This.P5>0.
** </ol>
*/
ಸ್ಥಿತಿ OP_Seek_ಪರಿಶೀಲನೆ: {          /* ncycle */
  VdbeCursor *pC;
  ಪೂರ್ಣಾಂಕ res;
  ಪೂರ್ಣಾಂಕ n_ಹೆಜ್ಜೆ;
  Unpacked_ದಾಖಲೆ r;

  ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_SeekGE );

  /* If pOp->p5 is clear, then pOp->p2 points to the first instruction past the
  ** OP_IdxGT that follows the OP_SeekGE. Otherwise, it points to the first
  ** opcode past the OP_SeekGE itself.  */
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>=(ಪೂರ್ಣಾಂಕ)(pOp-aOp)+2 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( pOp->p5==0 ){
    /* There are no inequality constraints following the IN constraint. */
    ಪ್ರತಿಜ್ಞಾ( pOp[1].p1==aOp[pOp->p2-1].p1 );
    ಪ್ರತಿಜ್ಞಾ( pOp[1].p2==aOp[pOp->p2-1].p2 );
    ಪ್ರತಿಜ್ಞಾ( pOp[1].p3==aOp[pOp->p2-1].p3 );
    ಪ್ರತಿಜ್ಞಾ( aOp[pOp->p2-1].opcode==OP_IdxGT
         || aOp[pOp->p2-1].opcode==OP_IdxGE );
    testcase( aOp[pOp->p2-1].opcode==OP_IdxGE );
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* There are inequality constraints.  */
    ಪ್ರತಿಜ್ಞಾ( pOp->p2==(ಪೂರ್ಣಾಂಕ)(pOp-aOp)+2 );
    ಪ್ರತಿಜ್ಞಾ( aOp[pOp->p2-1].opcode==OP_SeekGE );
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 );
  pC = p->apCsr[pOp[1].p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( !pC->is_ಕೋಷ್ಟಕ );
  ಆದರೆ( !sqlite3BtreeCursorIsValidNN(pC->uc.pCursor) ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
     ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
       ಬರೆ("... cursor not valid - fall through\n");
     }       
#ಆದರೆ_ಅಂತ್ಯ
    ನಿಲ್ಲಿಸು;
  }
  n_ಹೆಜ್ಜೆ = pOp->p1;
  ಪ್ರತಿಜ್ಞಾ( n_ಹೆಜ್ಜೆ>=1 );
  r.p_ಕೀಲಿ_Info = pC->p_ಕೀಲಿ_Info;
  r.n_ಕ್ಷೇತ್ರ = (u16)pOp[1].p4.i;
  r.default_rc = 0;
  r.aMem = &aMem[pOp[1].p3];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  {
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<r.n_ಕ್ಷೇತ್ರ; i++){
      ಪ್ರತಿಜ್ಞಾ( memIsValid(&r.aMem[i]) );
      ರಿಜಿಸ್ಟರ್_TRACE(pOp[1].p3+i, &aMem[pOp[1].p3+i]);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  res = 0;  /* Not needed.  Only used to silence a warning. */
  ಆಗ(1){
    rc = sqlite3_Vdbe_Idx_ಕೀಲಿ_Compare(db, pC, &r, &res);
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    ಆದರೆ( res>0 && pOp->p5==0 ){
      seekscan_ಹುಡುಕು_fail:
      /* Jump to SeekGE.P2, ending the loop */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
        ಬರೆ("... %d steps and then skip\n", pOp->p1 - n_ಹೆಜ್ಜೆ);
      }       
#ಆದರೆ_ಅಂತ್ಯ
      Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1,3);
      pOp++;
      ಹೋಗು jump_ಗಮ್ಯ_p2;
    }
    ಆದರೆ( res>=0 ){
      /* Jump to This.P2, bypassing the OP_SeekGE opcode */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
        ಬರೆ("... %d steps and then success\n", pOp->p1 - n_ಹೆಜ್ಜೆ);
      }       
#ಆದರೆ_ಅಂತ್ಯ
      Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(2,3);
      ಹೋಗು jump_ಗಮ್ಯ_p2;
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( n_ಹೆಜ್ಜೆ<=0 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
        ಬರೆ("... fall through after %d steps\n", pOp->p1);
      }       
#ಆದರೆ_ಅಂತ್ಯ
      Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0,3);
      ನಿಲ್ಲಿಸು;
    }
    n_ಹೆಜ್ಜೆ--;
    pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
    rc = sqlite3_Btree_ಮುಂದಿನ(pC->uc.pCursor, 0);
    ಆದರೆ( rc ){
      ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
        rc = SQLITE_OK;
        ಹೋಗು seekscan_ಹುಡುಕು_fail;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }
    }
  }
 
  ನಿಲ್ಲಿಸು;
}


/* Opcode: SeekHit P1 P2 P3 * *
** Synopsis: set P2<=seekHit<=P3
**
** Increase or decrease the seekHit value for cursor P1, if necessary,
** so that it is no less than P2 and no greater than P3.
**
** The seekHit integer represents the maximum of terms in an index for which
** there is known to be at least one match.  If the seekHit value is smaller
** than the total number of equality terms in an index lookup, then the
** OP_IfNoHope opcode might run to see if the IN loop can be abandoned
** early, thus saving work.  This is part of the IN-early-out optimization.
**
** P1 must be a valid b-tree cursor.
*/
ಸ್ಥಿತಿ OP_Seek_ದೊರಕಿತು: {           /* ncycle */
  VdbeCursor *pC;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3>=pOp->p2 );
  ಆದರೆ( pC->seek_ದೊರಕಿತು<pOp->p2 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
      ಬರೆ("seekHit changes from %d to %d\n", pC->seek_ದೊರಕಿತು, pOp->p2);
    }       
#ಆದರೆ_ಅಂತ್ಯ
    pC->seek_ದೊರಕಿತು = pOp->p2;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pC->seek_ದೊರಕಿತು>pOp->p3 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
      ಬರೆ("seekHit changes from %d to %d\n", pC->seek_ದೊರಕಿತು, pOp->p3);
    }       
#ಆದರೆ_ಅಂತ್ಯ
    pC->seek_ದೊರಕಿತು = pOp->p3;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Found P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** If P4==0 then register P3 holds a blob constructed by MakeRecord.  If
** P4>0 then register P3 is the first of P4 registers that form an unpacked
** record.
**
** Cursor P1 is on an index btree.  If the record identified by P3 and P4
** is a prefix of any entry in P1 then a jump is made to P2 and
** P1 is left pointing at the matching entry.
**
** This operation leaves the cursor in a state where it can be
** advanced in the forward direction.  The Next instruction will work,
** but not the Prev instruction.
**
** See also: NotFound, NoConflict, NotExists. SeekGe
*/
/* Opcode: NotFound P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** If P4==0 then register P3 holds a blob constructed by MakeRecord.  If
** P4>0 then register P3 is the first of P4 registers that form an unpacked
** record.
**
** Cursor P1 is on an index btree.  If the record identified by P3 and P4
** is not the prefix of any entry in P1 then a jump is made to P2.  If P1
** does contain an entry whose prefix matches the P3/P4 record then control
** falls through to the next instruction and P1 is left pointing at the
** matching entry.
**
** This operation leaves the cursor in a state where it cannot be
** advanced in either direction.  In other words, the Next and Prev
** opcodes do not work after this operation.
**
** See also: Found, NotExists, NoConflict, IfNoHope
*/
/* Opcode: IfNoHope P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** Register P3 is the first of P4 registers that form an unpacked
** record.  Cursor P1 is an index btree.  P2 is a jump destination.
** In other words, the operands to this opcode are the same as the
** operands to OP_NotFound and OP_IdxGT.
**
** This opcode is an optimization attempt only.  If this opcode always
** falls through, the correct answer is still obtained, but extra work
** is performed.
**
** A value of N in the seekHit flag of cursor P1 means that there exists
** a key P3:N that will match some record in the index.  We want to know
** if it is possible for a record P3:P4 to match some record in the
** index.  If it is not possible, we can skip some work.  So if seekHit
** is less than P4, attempt to find out if a match is possible by running
** OP_NotFound.
**
** This opcode is used in IN clause processing for a multi-column key.
** If an IN clause is attached to an element of the key other than the
** left-most element, and if there are no matches on the most recent
** seek over the whole key, then it might be that one of the key element
** to the left is prohibiting a match, and hence there is "no hope" of
** any match regardless of how many IN clause elements are checked.
** In such a case, we abandon the IN clause search early, using this
** opcode.  The opcode name comes from the fact that the
** jump is taken if there is "no hope" of achieving a match.
**
** See also: NotFound, SeekHit
*/
/* Opcode: NoConflict P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** If P4==0 then register P3 holds a blob constructed by MakeRecord.  If
** P4>0 then register P3 is the first of P4 registers that form an unpacked
** record.
**
** Cursor P1 is on an index btree.  If the record identified by P3 and P4
** contains any NULL value, jump immediately to P2.  If all terms of the
** record are not-NULL then a check is done to determine if any row in the
** P1 index btree has a matching key prefix.  If there are no matches, jump
** immediately to P2.  If there is a match, fall through and leave the P1
** cursor pointing to the matching row.
**
** This opcode is similar to OP_NotFound with the exceptions that the
** branch is always taken if any part of the search key input is NULL.
**
** This operation leaves the cursor in a state where it cannot be
** advanced in either direction.  In other words, the Next and Prev
** opcodes do not work after this operation.
**
** See also: NotFound, Found, NotExists
*/
ಸ್ಥಿತಿ OP_IfNoHope: {     /* jump, in3, ncycle */
  VdbeCursor *pC;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
    ಬರೆ("seekHit is %d\n", pC->seek_ದೊರಕಿತು);
  }       
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pC->seek_ದೊರಕಿತು>=pOp->p4.i ) ನಿಲ್ಲಿಸು;
  /* Fall through into OP_NotFound */
  /* no break */ deliberate_fall_through
}
ಸ್ಥಿತಿ OP_NoConflict:     /* jump, in3, ncycle */
ಸ್ಥಿತಿ OP_Not_ಸಿಕ್ಕಿತು:       /* jump, in3, ncycle */
ಸ್ಥಿತಿ OP_ಸಿಕ್ಕಿತು: {        /* jump, in3, ncycle */
  ಪೂರ್ಣಾಂಕ alreadyExists;
  ಪೂರ್ಣಾಂಕ ii;
  VdbeCursor *pC;
  Unpacked_ದಾಖಲೆ *p_Idx_ಕೀಲಿ;
  Unpacked_ದಾಖಲೆ r;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಆದರೆ( pOp->opcode!=OP_NoConflict ) sqlite3_ಸಿಕ್ಕಿತು_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INT32 );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#ಆದರೆ_ಅಂತ್ಯ
  r.aMem = &aMem[pOp->p3];
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->is_ಕೋಷ್ಟಕ==0 );
  r.n_ಕ್ಷೇತ್ರ = (u16)pOp->p4.i;
  ಆದರೆ( r.n_ಕ್ಷೇತ್ರ>0 ){
    /* Key values in an array of registers */
    r.p_ಕೀಲಿ_Info = pC->p_ಕೀಲಿ_Info;
    r.default_rc = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    (ಶೂನ್ಯ)sqlite3FaultSim(50);  /* For use by --counter in TH3 */
    ಗಾಗಿ(ii=0; ii<r.n_ಕ್ಷೇತ್ರ; ii++){
      ಪ್ರತಿಜ್ಞಾ( memIsValid(&r.aMem[ii]) );
      ಪ್ರತಿಜ್ಞಾ( (r.aMem[ii].flags & MEM_Zero)==0 || r.aMem[ii].n==0 );
      ಆದರೆ( ii ) ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3+ii, &r.aMem[ii]);
    }
#ಆದರೆ_ಅಂತ್ಯ
    rc = sqlite3_Btree_ಸೂಚ್ಯಂಕ_Moveto(pC->uc.pCursor, &r, &pC->seek_ಫಲಿತಾಂಶ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Composite key generated by OP_MakeRecord */
    ಪ್ರತಿಜ್ಞಾ( r.aMem->flags & MEM_Blob );
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode!=OP_NoConflict );
    rc = ExpandBlob(r.aMem);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || rc==SQLITE_NOMEM );
    ಆದರೆ( rc ) ಹೋಗು no_mem;
    p_Idx_ಕೀಲಿ = sqlite3_Vdbe_Alloc_Unpacked_ದಾಖಲೆ(pC->p_ಕೀಲಿ_Info);
    ಆದರೆ( p_Idx_ಕೀಲಿ==0 ) ಹೋಗು no_mem;
    sqlite3_Vdbe_ದಾಖಲೆ_Unpack(r.aMem->n, r.aMem->z, p_Idx_ಕೀಲಿ);
    p_Idx_ಕೀಲಿ->default_rc = 0;
    rc = sqlite3_Btree_ಸೂಚ್ಯಂಕ_Moveto(pC->uc.pCursor, p_Idx_ಕೀಲಿ, &pC->seek_ಫಲಿತಾಂಶ);
    sqlite3DbFreeNN(db, p_Idx_ಕೀಲಿ);
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  alreadyExists = (pC->seek_ಫಲಿತಾಂಶ==0);
  pC->null_ಅಡ್ಡಸಾಲು = 1-alreadyExists;
  pC->deferredMoveto = 0;
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಆದರೆ( pOp->opcode==OP_ಸಿಕ್ಕಿತು ){
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(alreadyExists!=0,2);
    ಆದರೆ( alreadyExists ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( !alreadyExists ){
      Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1,2);
      ಹೋಗು jump_ಗಮ್ಯ_p2;
    }
    ಆದರೆ( pOp->opcode==OP_NoConflict ){
      /* For the OP_NoConflict opcode, take the jump if any of the
      ** input fields are NULL, since any key with a NULL will not
      ** conflict */
      ಗಾಗಿ(ii=0; ii<r.n_ಕ್ಷೇತ್ರ; ii++){
        ಆದರೆ( r.aMem[ii].flags & MEM_Null ){
          Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1,2);
          ಹೋಗು jump_ಗಮ್ಯ_p2;
        }
      }
    }
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0,2);
    ಆದರೆ( pOp->opcode==OP_IfNoHope ){
      pC->seek_ದೊರಕಿತು = pOp->p4.i;
    }
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SeekRowid P1 P2 P3 * *
** Synopsis: intkey=r[P3]
**
** P1 is the index of a cursor open on an SQL table btree (with integer
** keys).  If register P3 does not contain an integer or if P1 does not
** contain a record with rowid P3 then jump immediately to P2. 
** Or, if P2 is 0, raise an SQLITE_CORRUPT error. If P1 does contain
** a record with rowid P3 then
** leave the cursor pointing at that record and fall through to the next
** instruction.
**
** The OP_NotExists opcode performs the same operation, but with OP_NotExists
** the P3 register must be guaranteed to contain an integer value.  With this
** opcode, register P3 might not contain an integer.
**
** The OP_NotFound opcode performs the same operation on index btrees
** (with arbitrary multi-value keys).
**
** This opcode leaves the cursor in a state where it cannot be advanced
** in either direction.  In other words, the Next and Prev opcodes will
** not work following this opcode.
**
** See also: Found, NotFound, NoConflict, SeekRowid
*/
/* Opcode: NotExists P1 P2 P3 * *
** Synopsis: intkey=r[P3]
**
** P1 is the index of a cursor open on an SQL table btree (with integer
** keys).  P3 is an integer rowid.  If P1 does not contain a record with
** rowid P3 then jump immediately to P2.  Or, if P2 is 0, raise an
** SQLITE_CORRUPT error. If P1 does contain a record with rowid P3 then
** leave the cursor pointing at that record and fall through to the next
** instruction.
**
** The OP_SeekRowid opcode performs the same operation but also allows the
** P3 register to contain a non-integer value, in which case the jump is
** always taken.  This opcode requires that P3 always contain an integer.
**
** The OP_NotFound opcode performs the same operation on index btrees
** (with arbitrary multi-value keys).
**
** This opcode leaves the cursor in a state where it cannot be advanced
** in either direction.  In other words, the Next and Prev opcodes will
** not work following this opcode.
**
** See also: Found, NotFound, NoConflict, SeekRowid
*/
ಸ್ಥಿತಿ OP_SeekRowid: {        /* jump0, in3, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  ಪೂರ್ಣಾಂಕ res;
  i64 i_ಕೀಲಿ;

  pIn3 = &aMem[pOp->p3];
  testcase( pIn3->flags & MEM_Int );
  testcase( pIn3->flags & MEM_IntReal );
  testcase( pIn3->flags & MEM_Real );
  testcase( (pIn3->flags & (MEM_Str|MEM_Int))==MEM_Str );
  ಆದರೆ( (pIn3->flags & (MEM_Int|MEM_IntReal))==0 ){
    /* If pIn3->u.i does not contain an integer, compute iKey as the
    ** integer value of pIn3.  Jump to P2 if pIn3 cannot be converted
    ** into an integer without loss of information.  Take care to avoid
    ** changing the datatype of pIn3, however, as it is used by other
    ** parts of the prepared statement. */
    Mem x = pIn3[0];
    ಆದರೆ( x.flags & MEM_Str ){
      applyNumericAffinity(&x, 1);
    }
    ಆದರೆ( x.flags & MEM_Int ){
      i_ಕೀಲಿ = x.u.i;
    }ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( (x.flags & MEM_Real)==0
     || x.u.r < -9223372036854775808.0
     || x.u.r > 9223372036854774784.0
           /*   ^^^^^^^^^^^^^^^^^^^^^-- same value as every other double
           **   between 9223372036854774263.0 and 923372036854775295.0 */
     || (ದ್ವಿಗುಣ)(i_ಕೀಲಿ = sqlite3_Real_ಗಮ್ಯ_I64(x.u.r))!=x.u.r
    ){
      ಹೋಗು jump_ಗಮ್ಯ_p2;
    }
    ಹೋಗು not_Exists_With_ಕೀಲಿ;
  }
  /* Fall through into OP_NotExists */
  /* no break */ deliberate_fall_through
ಸ್ಥಿತಿ OP_NotExists:          /* jump, in3, ncycle */
  pIn3 = &aMem[pOp->p3];
  ಪ್ರತಿಜ್ಞಾ( (pIn3->flags & MEM_Int)!=0 || pOp->opcode==OP_SeekRowid );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  i_ಕೀಲಿ = pIn3->u.i;
not_Exists_With_ಕೀಲಿ:
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( pOp->opcode==OP_SeekRowid ) pC->seekOp = OP_SeekRowid;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( pC->is_ಕೋಷ್ಟಕ );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  ಪ್ರತಿಜ್ಞಾ( pCrsr!=0 );
  res = 0;
  rc = sqlite3_Btree_ಕೋಷ್ಟಕ_Moveto(pCrsr, i_ಕೀಲಿ, 0, &res);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || res==0 );
  pC->moveto_ಗುರಿ = i_ಕೀಲಿ;  /* Used by OP_Delete */
  pC->null_ಅಡ್ಡಸಾಲು = 0;
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  pC->deferredMoveto = 0;
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res!=0,2);
  pC->seek_ಫಲಿತಾಂಶ = res;
  ಆದರೆ( res!=0 ){
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
    ಆದರೆ( pOp->p2==0 ){
      rc = SQLITE_CORRUPT_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹೋಗು jump_ಗಮ್ಯ_p2;
    }
  }
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Sequence P1 P2 * * *
** Synopsis: r[P2]=cursor[P1].ctr++
**
** Find the next available sequence number for cursor P1.
** Write the sequence number into register P2.
** The sequence number on the cursor is incremented after this
** instruction. 
*/
ಸ್ಥಿತಿ OP_Sequence: {           /* out2 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( p->apCsr[pOp->p1]!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->apCsr[pOp->p1]->eCurType!=CURTYPE_VTAB );
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->u.i = p->apCsr[pOp->p1]->seq_ಎಣಿಕೆ++;
  ನಿಲ್ಲಿಸು;
}


/* Opcode: NewRowid P1 P2 P3 * *
** Synopsis: r[P2]=rowid
**
** Get a new integer record number (a.k.a "rowid") used as the key to a table.
** The record number is not previously used as a key in the database
** table that cursor P1 points to.  The new record number is written
** written to register P2.
**
** If P3>0 then P3 is a register in the root frame of this VDBE that holds
** the largest previously generated record number. No new record numbers are
** allowed to be less than this value. When this value reaches its maximum,
** an SQLITE_FULL error is generated. The P3 register is updated with the '
** generated record number. This P3 mechanism is used to help implement the
** AUTOINCREMENT feature.
*/
ಸ್ಥಿತಿ OP_NewRowid: {           /* out2 */
  i64 v;                 /* The new rowid */
  VdbeCursor *pC;        /* Cursor of table to get the new rowid */
  ಪೂರ್ಣಾಂಕ res;               /* Result of an sqlite3BtreeLast() */
  ಪೂರ್ಣಾಂಕ cnt;               /* Counter to limit the number of searches */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINCREMENT
  Mem *pMem;             /* Register holding largest rowid for AUTOINCREMENT */
  Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು;     /* Root frame of VDBE */
#ಆದರೆ_ಅಂತ್ಯ

  v = 0;
  res = 0;
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->is_ಕೋಷ್ಟಕ );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
  {
    /* The next rowid or record number (different terms for the same
    ** thing) is obtained in a two-step algorithm.
    **
    ** First we attempt to find the largest existing rowid and add one
    ** to that.  But if the largest existing rowid is already the maximum
    ** positive integer, we have to fall through to the second
    ** probabilistic algorithm
    **
    ** The second algorithm is to select a rowid at random and see if
    ** it already exists in the table.  If it does not exist, we have
    ** succeeded.  If the random rowid does exist, we select a new one
    ** and try again, up to 100 times.
    */
    ಪ್ರತಿಜ್ಞಾ( pC->is_ಕೋಷ್ಟಕ );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_32_ಬಿಟ್_ROWID
#   define MAX_ROWID 0x7fffffff
#ಇಲ್ಲದಿದ್ದರೆ
    /* Some compilers complain about constants of the form 0x7fffffffffffffff.
    ** Others complain about 0x7ffffffffffffffffLL.  The following macro seems
    ** to provide the constant while making all compilers happy.
    */
#   define MAX_ROWID  (i64)( (((u64)0x7fffffff)<<32) | (u64)0xffffffff )
#ಆದರೆ_ಅಂತ್ಯ

    ಆದರೆ( !pC->useRandomRowid ){
      rc = sqlite3_Btree_ಕೊನೆಯ(pC->uc.pCursor, &res);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }
      ಆದರೆ( res ){
        v = 1;   /* IMP: R-61914-48074 */
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorIsValid(pC->uc.pCursor) );
        v = sqlite3_Btree_Integer_ಕೀಲಿ(pC->uc.pCursor);
        ಆದರೆ( v>=MAX_ROWID ){
          pC->useRandomRowid = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          v++;   /* IMP: R-29538-34987 */
        }
      }
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINCREMENT
    ಆದರೆ( pOp->p3 ){
      /* Assert that P3 is a valid memory cell. */
      ಪ್ರತಿಜ್ಞಾ( pOp->p3>0 );
      ಆದರೆ( p->p_ಚೌಕಟ್ಟು ){
        ಗಾಗಿ(p_ಚೌಕಟ್ಟು=p->p_ಚೌಕಟ್ಟು; p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ; p_ಚೌಕಟ್ಟು=p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ);
        /* Assert that P3 is a valid memory cell. */
        ಪ್ರತಿಜ್ಞಾ( pOp->p3<=p_ಚೌಕಟ್ಟು->nMem );
        pMem = &p_ಚೌಕಟ್ಟು->aMem[pOp->p3];
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Assert that P3 is a valid memory cell. */
        ಪ್ರತಿಜ್ಞಾ( pOp->p3<=(p->nMem+1 - p->nCursor) );
        pMem = &aMem[pOp->p3];
        mem_About_ಗಮ್ಯ_Change(p, pMem);
      }
      ಪ್ರತಿಜ್ಞಾ( memIsValid(pMem) );

      ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3, pMem);
      sqlite3VdbeMemIntegerify(pMem);
      ಪ್ರತಿಜ್ಞಾ( (pMem->flags & MEM_Int)!=0 );  /* mem(P3) holds an integer */
      ಆದರೆ( pMem->u.i==MAX_ROWID || pC->useRandomRowid ){
        rc = SQLITE_ತುಂಬಿದ;   /* IMP: R-17817-00630 */
        ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }
      ಆದರೆ( v<pMem->u.i+1 ){
        v = pMem->u.i + 1;
      }
      pMem->u.i = v;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( pC->useRandomRowid ){
      /* IMPLEMENTATION-OF: R-07677-41881 If the largest ROWID is equal to the
      ** largest possible integer (9223372036854775807) then the database
      ** engine starts picking positive candidate ROWIDs at random until
      ** it finds one that is not previously used. */
      ಪ್ರತಿಜ್ಞಾ( pOp->p3==0 );  /* We cannot be in random rowid mode if this is
                             ** an AUTOINCREMENT table. */
      cnt = 0;
      ಮಾಡು{
        sqlite3_randomness(ಗಾತ್ರ(v), &v);
        v &= (MAX_ROWID>>1); v++;  /* Ensure that v is greater than zero */
      }ಆಗ(  ((rc = sqlite3_Btree_ಕೋಷ್ಟಕ_Moveto(pC->uc.pCursor, (u64)v,
                                                 0, &res))==SQLITE_OK)
            && (res==0)
            && (++cnt<100));
      ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
      ಆದರೆ( res==0 ){
        rc = SQLITE_ತುಂಬಿದ;   /* IMP: R-38219-53002 */
        ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }
      ಪ್ರತಿಜ್ಞಾ( v>0 );  /* EV: R-40812-03570 */
    }
    pC->deferredMoveto = 0;
    pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  }
  p_ಹೊರಹೋಗು->u.i = v;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Insert P1 P2 P3 P4 P5
** Synopsis: intkey=r[P3] data=r[P2]
**
** Write an entry into the table of cursor P1.  A new entry is
** created if it doesn't already exist or the data for an existing
** entry is overwritten.  The data is the value MEM_Blob stored in register
** number P2. The key is stored in register P3. The key must
** be a MEM_Int.
**
** If the OPFLAG_NCHANGE flag of P5 is set, then the row change count is
** incremented (otherwise not).  If the OPFLAG_LASTROWID flag of P5 is set,
** then rowid is stored for subsequent return by the
** sqlite3_last_insert_rowid() function (otherwise it is unmodified).
**
** If the OPFLAG_USESEEKRESULT flag of P5 is set, the implementation might
** run faster by avoiding an unnecessary seek on cursor P1.  However,
** the OPFLAG_USESEEKRESULT flag must only be set if there have been no prior
** seeks on the cursor or if the most recent seek used a key equal to P3.
**
** If the OPFLAG_ISUPDATE flag is set, then this opcode is part of an
** UPDATE operation.  Otherwise (if the flag is clear) then this opcode
** is part of an INSERT operation.  The difference is only important to
** the update hook.
**
** Parameter P4 may point to a Table structure, or may be NULL. If it is
** not NULL, then the update-hook (sqlite3.xUpdateCallback) is invoked
** following a successful insert.
**
** (WARNING/TODO: If P1 is a pseudo-cursor and P2 is dynamically
** allocated, then ownership of P2 is transferred to the pseudo-cursor
** and register P2 becomes ephemeral.  If the cursor is changed, the
** value of register P2 will then change.  Make sure this does not
** cause any problems.)
**
** This instruction only works on tables.  The equivalent instruction
** for indices is OP_IdxInsert.
*/
ಸ್ಥಿತಿ OP_ಸೇರಿಸು: {
  Mem *p_ದತ್ತಾಂಶ;       /* MEM cell holding data for the record to be inserted */
  Mem *p_ಕೀಲಿ;        /* MEM cell holding key  for the record */
  VdbeCursor *pC;   /* Cursor to table into which insert is written */
  ಪೂರ್ಣಾಂಕ seek_ಫಲಿತಾಂಶ;   /* Result of prior seek or 0 if no USESEEKRESULT flag */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;  /* database name - used by the update hook */
  ಕೋಷ್ಟಕ *pTab;      /* Table structure - used by update and pre-update hooks */
  BtreePayload x;   /* Payload to be inserted */

  p_ದತ್ತಾಂಶ = &aMem[pOp->p2];
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( memIsValid(p_ದತ್ತಾಂಶ) );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
  ಪ್ರತಿಜ್ಞಾ( (pOp->p5 & OPFLAG_ISNOOP) || pC->is_ಕೋಷ್ಟಕ );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_ಕೋಷ್ಟಕ || pOp->p4type>=P4_STATIC );
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p2, p_ದತ್ತಾಂಶ);
  sqlite3VdbeIncrWriteCounter(p, pC);

  p_ಕೀಲಿ = &aMem[pOp->p3];
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ->flags & MEM_Int );
  ಪ್ರತಿಜ್ಞಾ( memIsValid(p_ಕೀಲಿ) );
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3, p_ಕೀಲಿ);
  x.n_ಕೀಲಿ = p_ಕೀಲಿ->u.i;

  ಆದರೆ( pOp->p4type==P4_ಕೋಷ್ಟಕ && HAS_UPDATE_HOOK(db) ){
    ಪ್ರತಿಜ್ಞಾ( pC->iDb>=0 );
    zDb = db->aDb[pC->iDb].zDbSName;
    pTab = pOp->p4.pTab;
    ಪ್ರತಿಜ್ಞಾ( (pOp->p5 & OPFLAG_ISNOOP) || HasRowid(pTab) );
  }ಇಲ್ಲದಿದ್ದರೆ{
    pTab = 0;
    zDb = 0;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
  /* Invoke the pre-update hook, if any */
  ಆದರೆ( pTab ){
    ಆದರೆ( db->xPreUpdateCallback && !(pOp->p5 & OPFLAG_ISUPDATE) ){
      sqlite3VdbePreUpdateHook(p,pC,SQLITE_ಸೇರಿಸು,zDb,pTab,x.n_ಕೀಲಿ,pOp->p2,-1);
    }
    ಆದರೆ( db->xUpdateCallback==0 || pTab->a_ಲಂಬಸಾಲು==0 ){
      /* Prevent post-update hook from running in cases when it should not */
      pTab = 0;
    }
  }
  ಆದರೆ( pOp->p5 & OPFLAG_ISNOOP ) ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( (pOp->p5 & OPFLAG_LASTROWID)==0 || (pOp->p5 & OPFLAG_NCHANGE)!=0 );
  ಆದರೆ( pOp->p5 & OPFLAG_NCHANGE ){
    p->nChange++;
    ಆದರೆ( pOp->p5 & OPFLAG_LASTROWID ) db->ಕೊನೆಯ_Rowid = x.n_ಕೀಲಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( (p_ದತ್ತಾಂಶ->flags & (MEM_Blob|MEM_Str))!=0 || p_ದತ್ತಾಂಶ->n==0 );
  x.p_ದತ್ತಾಂಶ = p_ದತ್ತಾಂಶ->z;
  x.n_ದತ್ತಾಂಶ = p_ದತ್ತಾಂಶ->n;
  seek_ಫಲಿತಾಂಶ = ((pOp->p5 & OPFLAG_USESEEKRESULT) ? pC->seek_ಫಲಿತಾಂಶ : 0);
  ಆದರೆ( p_ದತ್ತಾಂಶ->flags & MEM_Zero ){
    x.nZero = p_ದತ್ತಾಂಶ->u.nZero;
  }ಇಲ್ಲದಿದ್ದರೆ{
    x.nZero = 0;
  }
  x.p_ಕೀಲಿ = 0;
  ಪ್ರತಿಜ್ಞಾ( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  rc = sqlite3_Btree_ಸೇರಿಸು(pC->uc.pCursor, &x,
      (pOp->p5 & (OPFLAG_APPEND|OPFLAG_SAVEPOSITION|OPFLAG_PREFORMAT)),
      seek_ಫಲಿತಾಂಶ
  );
  pC->deferredMoveto = 0;
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಲಂಬಸಾಲು_Cache_Ctr++;

  /* Invoke the update-hook if required. */
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ಆದರೆ( pTab ){
    ಪ್ರತಿಜ್ಞಾ( db->xUpdateCallback!=0 );
    ಪ್ರತಿಜ್ಞಾ( pTab->a_ಲಂಬಸಾಲು!=0 );
    db->xUpdateCallback(db->pUpdateArg,
           (pOp->p5 & OPFLAG_ISUPDATE) ? SQLITE_UPDATE : SQLITE_ಸೇರಿಸು,
           zDb, pTab->zName, x.n_ಕೀಲಿ);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: RowCell P1 P2 P3 * *
**
** P1 and P2 are both open cursors. Both must be opened on the same type
** of table - intkey or index. This opcode is used as part of copying
** the current row from P2 into P1. If the cursors are opened on intkey
** tables, register P3 contains the rowid to use with the new record in
** P1. If they are opened on index tables, P3 is not used.
**
** This opcode must be followed by either an Insert or InsertIdx opcode
** with the OPFLAG_PREFORMAT flag set to complete the insert operation.
*/
ಸ್ಥಿತಿ OP_ಅಡ್ಡಸಾಲು_Cell: {
  VdbeCursor *pDest;              /* Cursor to write to */
  VdbeCursor *pSrc;               /* Cursor to read from */
  i64 i_ಕೀಲಿ;                       /* Rowid value to insert with */
  ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_ಸೇರಿಸು || pOp[1].opcode==OP_Idx_ಸೇರಿಸು );
  ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_ಸೇರಿಸು    || pOp->p3==0 );
  ಪ್ರತಿಜ್ಞಾ( pOp[1].opcode==OP_Idx_ಸೇರಿಸು || pOp->p3>0 );
  ಪ್ರತಿಜ್ಞಾ( pOp[1].p5 & OPFLAG_PREFORMAT );
  pDest = p->apCsr[pOp->p1];
  pSrc = p->apCsr[pOp->p2];
  i_ಕೀಲಿ = pOp->p3 ? aMem[pOp->p3].u.i : 0;
  rc = sqlite3_Btree_Transfer_ಅಡ್ಡಸಾಲು(pDest->uc.pCursor, pSrc->uc.pCursor, i_ಕೀಲಿ);
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
};

/* Opcode: Delete P1 P2 P3 P4 P5
**
** Delete the record at which the P1 cursor is currently pointing.
**
** If the OPFLAG_SAVEPOSITION bit of the P5 parameter is set, then
** the cursor will be left pointing at  either the next or the previous
** record in the table. If it is left pointing at the next record, then
** the next Next instruction will be a no-op. As a result, in this case
** it is ok to delete a record from within a Next loop. If
** OPFLAG_SAVEPOSITION bit of P5 is clear, then the cursor will be
** left in an undefined state.
**
** If the OPFLAG_AUXDELETE bit is set on P5, that indicates that this
** delete is one of several associated with deleting a table row and
** all its associated index entries.  Exactly one of those deletes is
** the "primary" delete.  The others are all on OPFLAG_FORDELETE
** cursors or else are marked with the AUXDELETE flag.
**
** If the OPFLAG_NCHANGE (0x01) flag of P2 (NB: P2 not P5) is set, then
** the row change count is incremented (otherwise not).
**
** If the OPFLAG_ISNOOP (0x40) flag of P2 (not P5!) is set, then the
** pre-update-hook for deletes is run, but the btree is otherwise unchanged.
** This happens when the OP_Delete is to be shortly followed by an OP_Insert
** with the same key, causing the btree entry to be overwritten.
**
** P1 must not be pseudo-table.  It has to be a real table with
** multiple rows.
**
** If P4 is not NULL then it points to a Table object. In this case either
** the update or pre-update hook, or both, may be invoked. The P1 cursor must
** have been positioned using OP_NotFound prior to invoking this opcode in
** this case. Specifically, if one is configured, the pre-update hook is
** invoked if P4 is not NULL. The update-hook is invoked if one is configured,
** P4 is not NULL, and the OPFLAG_NCHANGE flag is set in P2.
**
** If the OPFLAG_ISUPDATE flag is set in P2, then P3 contains the address
** of the memory cell that contains the value that the rowid of the row will
** be set to by the update.
*/
ಸ್ಥಿತಿ OP_Delete: {
  VdbeCursor *pC;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;
  ಕೋಷ್ಟಕ *pTab;
  ಪೂರ್ಣಾಂಕ opflags;

  opflags = pOp->p2;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  sqlite3VdbeIncrWriteCounter(p, pC);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( pOp->p4type==P4_ಕೋಷ್ಟಕ
   && HasRowid(pOp->p4.pTab)
   && pOp->p5==0
   && sqlite3BtreeCursorIsValidNN(pC->uc.pCursor)
  ){
    /* If p5 is zero, the seek operation that positioned the cursor prior to
    ** OP_Delete will have also set the pC->movetoTarget field to the rowid of
    ** the row that is being deleted */
    i64 i_ಕೀಲಿ = sqlite3_Btree_Integer_ಕೀಲಿ(pC->uc.pCursor);
    ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || pC->moveto_ಗುರಿ==i_ಕೀಲಿ );
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If the update-hook or pre-update-hook will be invoked, set zDb to
  ** the name of the db to pass as to it. Also set local pTab to a copy
  ** of p4.pTab. Finally, if p5 is true, indicating that this cursor was
  ** last moved with OP_Next or OP_Prev, not Seek or NotFound, set
  ** VdbeCursor.movetoTarget to the current rowid.  */
  ಆದರೆ( pOp->p4type==P4_ಕೋಷ್ಟಕ && HAS_UPDATE_HOOK(db) ){
    ಪ್ರತಿಜ್ಞಾ( pC->iDb>=0 );
    ಪ್ರತಿಜ್ಞಾ( pOp->p4.pTab!=0 );
    zDb = db->aDb[pC->iDb].zDbSName;
    pTab = pOp->p4.pTab;
    ಆದರೆ( (pOp->p5 & OPFLAG_SAVEPOSITION)!=0 && pC->is_ಕೋಷ್ಟಕ ){
      pC->moveto_ಗುರಿ = sqlite3_Btree_Integer_ಕೀಲಿ(pC->uc.pCursor);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    zDb = 0;
    pTab = 0;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
  /* Invoke the pre-update-hook if required. */
  ಪ್ರತಿಜ್ಞಾ( db->xPreUpdateCallback==0 || pTab==pOp->p4.pTab );
  ಆದರೆ( db->xPreUpdateCallback && pTab ){
    ಪ್ರತಿಜ್ಞಾ( !(opflags & OPFLAG_ISUPDATE)
         || HasRowid(pTab)==0
         || (aMem[pOp->p3].flags & MEM_Int)
    );
    sqlite3VdbePreUpdateHook(p, pC,
        (opflags & OPFLAG_ISUPDATE) ? SQLITE_UPDATE : SQLITE_DELETE,
        zDb, pTab, pC->moveto_ಗುರಿ,
        pOp->p3, -1
    );
  }
  ಆದರೆ( opflags & OPFLAG_ISNOOP ) ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

  /* Only flags that can be set are SAVEPOISTION and AUXDELETE */
  ಪ್ರತಿಜ್ಞಾ( (pOp->p5 & ~(OPFLAG_SAVEPOSITION|OPFLAG_AUXDELETE))==0 );
  ಪ್ರತಿಜ್ಞಾ( OPFLAG_SAVEPOSITION==BTREE_SAVEPOSITION );
  ಪ್ರತಿಜ್ಞಾ( OPFLAG_AUXDELETE==BTREE_AUXDELETE );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( p->p_ಚೌಕಟ್ಟು==0 ){
    ಆದರೆ( pC->isEphemeral==0
        && (pOp->p5 & OPFLAG_AUXDELETE)==0
        && (pC->wrFlag & OPFLAG_FORDELETE)==0
      ){
      nExtraDelete++;
    }
    ಆದರೆ( pOp->p2 & OPFLAG_NCHANGE ){
      nExtraDelete--;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  rc = sqlite3BtreeDelete(pC->uc.pCursor, pOp->p5);
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಲಂಬಸಾಲು_Cache_Ctr++;
  pC->seek_ಫಲಿತಾಂಶ = 0;
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;

  /* Invoke the update-hook if required. */
  ಆದರೆ( opflags & OPFLAG_NCHANGE ){
    p->nChange++;
    ಆದರೆ( db->xUpdateCallback && ALWAYS(pTab!=0) && HasRowid(pTab) ){
      db->xUpdateCallback(db->pUpdateArg, SQLITE_DELETE, zDb, pTab->zName,
          pC->moveto_ಗುರಿ);
      ಪ್ರತಿಜ್ಞಾ( pC->iDb>=0 );
    }
  }

  ನಿಲ್ಲಿಸು;
}
/* Opcode: ResetCount * * * * *
**
** The value of the change counter is copied to the database handle
** change counter (returned by subsequent calls to sqlite3_changes()).
** Then the VMs internal change counter resets to 0.
** This is used by trigger programs.
*/
ಸ್ಥಿತಿ OP_Reset_ಎಣಿಕೆ: {
  sqlite3_Vdbe_ಗಣ_Changes(db, p->nChange);
  p->nChange = 0;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SorterCompare P1 P2 P3 P4
** Synopsis: if key(P1)!=trim(r[P3],P4) goto P2
**
** P1 is a sorter cursor. This instruction compares a prefix of the
** record blob in register P3 against a prefix of the entry that
** the sorter cursor currently points to.  Only the first P4 fields
** of r[P3] and the sorter record are compared.
**
** If either P3 or the sorter contains a NULL in one of their significant
** fields (not counting the P4 fields at the end which are ignored) then
** the comparison is assumed to be equal.
**
** Fall through to next instruction if the two records compare equal to
** each other.  Jump to P2 if they are different.
*/
ಸ್ಥಿತಿ OP_SorterCompare: {
  VdbeCursor *pC;
  ಪೂರ್ಣಾಂಕ res;
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ_ಲಂಬಸಾಲು;

  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( isSorter(pC) );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INT32 );
  pIn3 = &aMem[pOp->p3];
  n_ಕೀಲಿ_ಲಂಬಸಾಲು = pOp->p4.i;
  res = 0;
  rc = sqlite3VdbeSorterCompare(pC, pIn3, n_ಕೀಲಿ_ಲಂಬಸಾಲು, &res);
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res!=0,2);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ಆದರೆ( res ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
};

/* Opcode: SorterData P1 P2 P3 * *
** Synopsis: r[P2]=data
**
** Write into register P2 the current sorter data for sorter cursor P1.
** Then clear the column header cache on cursor P3.
**
** This opcode is normally used to move a record out of the sorter and into
** a register that is the source for a pseudo-table cursor created using
** OpenPseudo.  That pseudo-table cursor is the one that is identified by
** parameter P3.  Clearing the P3 column cache as part of this opcode saves
** us from having to issue a separate NullRow instruction to clear that cache.
*/
ಸ್ಥಿತಿ OP_Sorter_ದತ್ತಾಂಶ: {       /* ncycle */
  VdbeCursor *pC;

  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( isSorter(pC) );
  rc = sqlite3VdbeSorterRowkey(pC, p_ಹೊರಹೋಗು);
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || (p_ಹೊರಹೋಗು->flags & MEM_Blob) );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  p->apCsr[pOp->p3]->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: RowData P1 P2 P3 * *
** Synopsis: r[P2]=data
**
** Write into register P2 the complete row content for the row at
** which cursor P1 is currently pointing.
** There is no interpretation of the data. 
** It is just copied onto the P2 register exactly as
** it is found in the database file.
**
** If cursor P1 is an index, then the content is the key of the row.
** If cursor P2 is a table, then the content extracted is the data.
**
** If the P1 cursor must be pointing to a valid row (not a NULL row)
** of a real table, not a pseudo-table.
**
** If P3!=0 then this opcode is allowed to make an ephemeral pointer
** into the database page.  That means that the content of the output
** register will be invalidated as soon as the cursor moves - including
** moves caused by other cursors that "save" the current cursors
** position in order that they can write to the same table.  If P3==0
** then a copy of the data is made into memory.  P3!=0 is faster, but
** P3==0 is safer.
**
** If P3!=0 then the content of the P2 register is unsuitable for use
** in OP_Result and any OP_Result will invalidate the P2 register content.
** The P2 register content is invalidated by opcodes like OP_Function or
** by any use of another cursor pointing to the same table.
*/
ಸ್ಥಿತಿ OP_ಅಡ್ಡಸಾಲು_ದತ್ತಾಂಶ: {
  VdbeCursor *pC;
  BtCursor *pCrsr;
  u32 n;

  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( isSorter(pC)==0 );
  ಪ್ರತಿಜ್ಞಾ( pC->null_ಅಡ್ಡಸಾಲು==0 );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
  pCrsr = pC->uc.pCursor;

  /* The OP_RowData opcodes always follow OP_NotExists or
  ** OP_SeekRowid or OP_Rewind/Op_Next with no intervening instructions
  ** that might invalidate the cursor.
  ** If this were not the case, one of the following assert()s
  ** would fail.  Should this ever change (because of changes in the code
  ** generator) then the fix would be to insert a call to
  ** sqlite3VdbeCursorMoveto().
  */
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorIsValid(pCrsr) );

  n = sqlite3_Btree_Payload_ಗಾತ್ರ(pCrsr);
  ಆದರೆ( n>(u32)db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] ){
    ಹೋಗು too_big;
  }
  testcase( n==0 );
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(pCrsr, n, p_ಹೊರಹೋಗು);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ಆದರೆ( !pOp->p3 ) Deephemeralize(p_ಹೊರಹೋಗು);
  UPDATE_MAX_BLOBSIZE(p_ಹೊರಹೋಗು);
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p2, p_ಹೊರಹೋಗು);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Rowid P1 P2 * * *
** Synopsis: r[P2]=PX rowid of P1
**
** Store in register P2 an integer which is the key of the table entry that
** P1 is currently point to.
**
** P1 can be either an ordinary table or a virtual table.  There used to
** be a separate OP_VRowid opcode for use with virtual tables, but this
** one opcode now works for both table types.
*/
ಸ್ಥಿತಿ OP_Rowid: {                 /* out2, ncycle */
  VdbeCursor *pC;
  i64 v;
  sqlite3_vtab *pVtab;
  ಸ್ಥಿರಾಂಕ sqlite3_module *pModule;

  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType!=CURTYPE_PSEUDO || pC->null_ಅಡ್ಡಸಾಲು );
  ಆದರೆ( pC->null_ಅಡ್ಡಸಾಲು ){
    p_ಹೊರಹೋಗು->flags = MEM_Null;
    ನಿಲ್ಲಿಸು;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pC->deferredMoveto ){
    v = pC->moveto_ಗುರಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pC->eCurType==CURTYPE_VTAB ){
    ಪ್ರತಿಜ್ಞಾ( pC->uc.pVCur!=0 );
    pVtab = pC->uc.pVCur->pVtab;
    pModule = pVtab->pModule;
    ಪ್ರತಿಜ್ಞಾ( pModule->xRowid );
    rc = pModule->xRowid(pC->uc.pVCur, &v);
    sqlite3VtabImportErrmsg(p, pVtab);
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
    ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
    rc = sqlite3VdbeCursorRestore(pC);
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    ಆದರೆ( pC->null_ಅಡ್ಡಸಾಲು ){
      p_ಹೊರಹೋಗು->flags = MEM_Null;
      ನಿಲ್ಲಿಸು;
    }
    v = sqlite3_Btree_Integer_ಕೀಲಿ(pC->uc.pCursor);
  }
  p_ಹೊರಹೋಗು->u.i = v;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: NullRow P1 P2 * * *
**
** Move the cursor P1 to a null row if P2 is 0.  Any OP_Column operations
** that occur while the cursor is on the null row will always
** write a NULL.
**
** If cursor P1 is not previously opened, open it now to a special
** pseudo-cursor that always returns NULL for every column.
**
** If P2 is not 0, then the nullRow flag on the pseudo-cursor is
** turned off.
*/
ಸ್ಥಿತಿ OP_Null_ಅಡ್ಡಸಾಲು: {
  VdbeCursor *pC;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಆದರೆ( pC==0 ){
    /* If the cursor is not already open, create a special kind of
    ** pseudo-cursor that always gives null rows. */
    pC = allocateCursor(p, pOp->p1, 1, CURTYPE_PSEUDO);
    ಆದರೆ( pC==0 ) ಹೋಗು no_mem;
    pC->seek_ಫಲಿತಾಂಶ = 0;
    pC->is_ಕೋಷ್ಟಕ = 1;
    pC->noReuse = 1;
    pC->uc.pCursor = sqlite3BtreeFakeValidCursor();
  }
  pC->null_ಅಡ್ಡಸಾಲು = (pOp->p2==0);
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಆದರೆ( pC->eCurType==CURTYPE_BTREE ){
    ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
    sqlite3BtreeClearCursor(pC->uc.pCursor);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( pC->seekOp==0 ) pC->seekOp = OP_Null_ಅಡ್ಡಸಾಲು;
#ಆದರೆ_ಅಂತ್ಯ
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SeekEnd P1 * * * *
**
** Position cursor P1 at the end of the btree for the purpose of
** appending a new entry onto the btree.
**
** It is assumed that the cursor is used only for appending and so
** if the cursor is valid, then the cursor must already be pointing
** at the end of the btree and so no changes are made to
** the cursor.
*/
/* Opcode: Last P1 P2 * * *
**
** The next use of the Rowid or Column or Prev instruction for P1
** will refer to the last entry in the database table or index.
** If the table or index is empty and P2>0, then jump immediately to P2.
** If P2 is 0 or if the table or index is not empty, fall through
** to the following instruction.
**
** This opcode leaves the cursor configured to move in reverse order,
** from the end toward the beginning.  In other words, the cursor is
** configured to use Prev, not Next.
*/
ಸ್ಥಿತಿ OP_Seek_ಕೊನೆ:             /* ncycle */
ಸ್ಥಿತಿ OP_ಕೊನೆಯ: {              /* jump0, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  ಪೂರ್ಣಾಂಕ res;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  res = 0;
  ಪ್ರತಿಜ್ಞಾ( pCrsr!=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pOp->opcode==OP_Seek_ಕೊನೆ ){
    ಪ್ರತಿಜ್ಞಾ( pOp->p2==0 );
    pC->seek_ಫಲಿತಾಂಶ = -1;
    ಆದರೆ( sqlite3BtreeCursorIsValidNN(pCrsr) ){
      ನಿಲ್ಲಿಸು;
    }
  }
  rc = sqlite3_Btree_ಕೊನೆಯ(pCrsr, &res);
  pC->null_ಅಡ್ಡಸಾಲು = (u8)res;
  pC->deferredMoveto = 0;
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ಆದರೆ( pOp->p2>0 ){
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res!=0,2);
    ಆದರೆ( res ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IfSizeBetween P1 P2 P3 P4 *
**
** Let N be the approximate number of rows in the table or index
** with cursor P1 and let X be 10*log2(N) if N is positive or -1
** if N is zero.
**
** Jump to P2 if X is in between P3 and P4, inclusive.
*/
ಸ್ಥಿತಿ OP_If_ಗಾತ್ರ_Between: {        /* jump */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  ಪೂರ್ಣಾಂಕ res;
  i64 sz;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INT32 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3>=-1 && pOp->p3<=640*2 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4.i>=-1 && pOp->p4.i<=640*2 );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  pCrsr = pC->uc.pCursor;
  ಪ್ರತಿಜ್ಞಾ( pCrsr );
  rc = sqlite3_Btree_ಮೊದಲ(pCrsr, &res);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ಆದರೆ( res!=0 ){
    sz = -1;  /* -Infinity encoding */
  }ಇಲ್ಲದಿದ್ದರೆ{
    sz = sqlite3_Btree_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ_Est(pCrsr);
    ಪ್ರತಿಜ್ಞಾ( sz>0 );
    sz = sqlite3LogEst((u64)sz);
  }
  res = sz>=pOp->p3 && sz<=pOp->p4.i;
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res!=0,2);
  ಆದರೆ( res ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
}


/* Opcode: SorterSort P1 P2 * * *
**
** After all records have been inserted into the Sorter object
** identified by P1, invoke this opcode to actually do the sorting.
** Jump to P2 if there are no records to be sorted.
**
** This opcode is an alias for OP_Sort and OP_Rewind that is used
** for Sorter objects.
*/
/* Opcode: Sort P1 P2 * * *
**
** This opcode does exactly the same thing as OP_Rewind except that
** it increments an undocumented global variable used for testing.
**
** Sorting is accomplished by writing records into a sorting index,
** then rewinding that index and playing it back from beginning to
** end.  We use the OP_Sort opcode instead of OP_Rewind to do the
** rewinding so that the global variable will be incremented and
** regression tests can determine whether or not the optimizer is
** correctly optimizing out sorts.
*/
ಸ್ಥಿತಿ OP_Sorter_ವಿಂಗಡಿಸು:    /* jump ncycle */
ಸ್ಥಿತಿ OP_ವಿಂಗಡಿಸು: {        /* jump ncycle */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  sqlite3_ವಿಂಗಡಿಸು_ಎಣಿಕೆ++;
  sqlite3_ಹುಡುಕು_ಎಣಿಕೆ--;
#ಆದರೆ_ಅಂತ್ಯ
  p->aCounter[SQLITE_STMTSTATUS_ವಿಂಗಡಿಸು]++;
  /* Fall through into OP_Rewind */
  /* no break */ deliberate_fall_through
}
/* Opcode: Rewind P1 P2 * * *
**
** The next use of the Rowid or Column or Next instruction for P1
** will refer to the first entry in the database table or index.
** If the table or index is empty, jump immediately to P2.
** If the table or index is not empty, fall through to the following
** instruction.
**
** If P2 is zero, that is an assertion that the P1 table is never
** empty and hence the jump will never be taken.
**
** This opcode leaves the cursor configured to move in forward order,
** from the beginning toward the end.  In other words, the cursor is
** configured to use Next, not Prev.
*/
ಸ್ಥಿತಿ OP_Rewind: {        /* jump0, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  ಪೂರ್ಣಾಂಕ res;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p5==0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>=0 && pOp->p2<p->nOp );

  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( isSorter(pC)==(pOp->opcode==OP_Sorter_ವಿಂಗಡಿಸು) );
  res = 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  pC->seekOp = OP_Rewind;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( isSorter(pC) ){
    rc = sqlite3VdbeSorterRewind(pC, &res);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
    pCrsr = pC->uc.pCursor;
    ಪ್ರತಿಜ್ಞಾ( pCrsr );
    rc = sqlite3_Btree_ಮೊದಲ(pCrsr, &res);
    pC->deferredMoveto = 0;
    pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  }
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  pC->null_ಅಡ್ಡಸಾಲು = (u8)res;
  ಆದರೆ( pOp->p2>0 ){
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res!=0,2);
    ಆದರೆ( res ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IfEmpty P1 P2 * * *
** Synopsis: if( empty(P1) ) goto P2
**
** Check to see if the b-tree table that cursor P1 references is empty
** and jump to P2 if it is.
*/
ಸ್ಥಿತಿ OP_If_ಖಾಲಿಗಳು: {        /* jump */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  ಪೂರ್ಣಾಂಕ res;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>=0 && pOp->p2<p->nOp );

  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  ಪ್ರತಿಜ್ಞಾ( pCrsr );
  rc = sqlite3_Btree_Is_ಖಾಲಿಗಳು(pCrsr, &res);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res!=0,2);
  ಆದರೆ( res ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Next P1 P2 P3 * P5
**
** Advance cursor P1 so that it points to the next key/data pair in its
** table or index.  If there are no more key/value pairs then fall through
** to the following instruction.  But if the cursor advance was successful,
** jump immediately to P2.
**
** The Next opcode is only valid following an SeekGT, SeekGE, or
** OP_Rewind opcode used to position the cursor.  Next is not allowed
** to follow SeekLT, SeekLE, or OP_Last.
**
** The P1 cursor must be for a real table, not a pseudo-table.  P1 must have
** been opened prior to this opcode or the program will segfault.
**
** The P3 value is a hint to the btree implementation. If P3==1, that
** means P1 is an SQL index and that this instruction could have been
** omitted if that index had been unique.  P3 is usually 0.  P3 is
** always either 0 or 1.
**
** If P5 is positive and the jump is taken, then event counter
** number P5-1 in the prepared statement is incremented.
**
** See also: Prev
*/
/* Opcode: Prev P1 P2 P3 * P5
**
** Back up cursor P1 so that it points to the previous key/data pair in its
** table or index.  If there is no previous key/value pairs then fall through
** to the following instruction.  But if the cursor backup was successful,
** jump immediately to P2.
**
**
** The Prev opcode is only valid following an SeekLT, SeekLE, or
** OP_Last opcode used to position the cursor.  Prev is not allowed
** to follow SeekGT, SeekGE, or OP_Rewind.
**
** The P1 cursor must be for a real table, not a pseudo-table.  If P1 is
** not open then the behavior is undefined.
**
** The P3 value is a hint to the btree implementation. If P3==1, that
** means P1 is an SQL index and that this instruction could have been
** omitted if that index had been unique.  P3 is usually 0.  P3 is
** always either 0 or 1.
**
** If P5 is positive and the jump is taken, then event counter
** number P5-1 in the prepared statement is incremented.
*/
/* Opcode: SorterNext P1 P2 * * P5
**
** This opcode works just like OP_Next except that P1 must be a
** sorter object for which the OP_SorterSort opcode has been
** invoked.  This opcode advances the cursor to the next sorted
** record, or jumps to P2 if there are no more sorted records.
*/
ಸ್ಥಿತಿ OP_Sorter_ಮುಂದಿನ: {  /* jump */
  VdbeCursor *pC;

  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( isSorter(pC) );
  rc = sqlite3_Vdbe_Sorter_ಮುಂದಿನ(db, pC);
  ಹೋಗು ಮುಂದಿನ_ಬಾಲ;

ಸ್ಥಿತಿ OP_Prev:          /* jump, ncycle */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p5==0
       || pOp->p5==SQLITE_STMTSTATUS_FULLSCAN_ಹೆಜ್ಜೆ
       || pOp->p5==SQLITE_STMTSTATUS_AUTOINDEX);
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->seekOp==OP_SeekLT || pC->seekOp==OP_SeekLE
       || pC->seekOp==OP_ಕೊನೆಯ   || pC->seekOp==OP_IfNoHope
       || pC->seekOp==OP_Null_ಅಡ್ಡಸಾಲು);
  rc = sqlite3_Btree_ಹಿಂದಿನ(pC->uc.pCursor, pOp->p3);
  ಹೋಗು ಮುಂದಿನ_ಬಾಲ;

ಸ್ಥಿತಿ OP_ಮುಂದಿನ:          /* jump, ncycle */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  ಪ್ರತಿಜ್ಞಾ( pOp->p5==0
       || pOp->p5==SQLITE_STMTSTATUS_FULLSCAN_ಹೆಜ್ಜೆ
       || pOp->p5==SQLITE_STMTSTATUS_AUTOINDEX);
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->seekOp==OP_SeekGT || pC->seekOp==OP_SeekGE
       || pC->seekOp==OP_Rewind || pC->seekOp==OP_ಸಿಕ್ಕಿತು
       || pC->seekOp==OP_Null_ಅಡ್ಡಸಾಲು|| pC->seekOp==OP_SeekRowid
       || pC->seekOp==OP_IfNoHope);
  rc = sqlite3_Btree_ಮುಂದಿನ(pC->uc.pCursor, pOp->p3);

ಮುಂದಿನ_ಬಾಲ:
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(rc==SQLITE_OK,2);
  ಆದರೆ( rc==SQLITE_OK ){
    pC->null_ಅಡ್ಡಸಾಲು = 0;
    p->aCounter[pOp->p5]++;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
    sqlite3_ಹುಡುಕು_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ
    ಹೋಗು jump_ಗಮ್ಯ_p2_and_check_for_ಅಡಚಣೆ;
  }
  ಆದರೆ( rc!=SQLITE_ಮುಗಿದ ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  rc = SQLITE_OK;
  pC->null_ಅಡ್ಡಸಾಲು = 1;
  ಹೋಗು check_for_ಅಡಚಣೆ;
}

/* Opcode: IdxInsert P1 P2 P3 P4 P5
** Synopsis: key=r[P2]
**
** Register P2 holds an SQL index key made using the
** MakeRecord instructions.  This opcode writes that key
** into the index P1.  Data for the entry is nil.
**
** If P4 is not zero, then it is the number of values in the unpacked
** key of reg(P2).  In that case, P3 is the index of the first register
** for the unpacked key.  The availability of the unpacked key can sometimes
** be an optimization.
**
** If P5 has the OPFLAG_APPEND bit set, that is a hint to the b-tree layer
** that this insert is likely to be an append.
**
** If P5 has the OPFLAG_NCHANGE bit set, then the change counter is
** incremented by this instruction.  If the OPFLAG_NCHANGE bit is clear,
** then the change counter is unchanged.
**
** If the OPFLAG_USESEEKRESULT flag of P5 is set, the implementation might
** run faster by avoiding an unnecessary seek on cursor P1.  However,
** the OPFLAG_USESEEKRESULT flag must only be set if there have been no prior
** seeks on the cursor or if the most recent seek used a key equivalent
** to P2.
**
** This instruction only works for indices.  The equivalent instruction
** for tables is OP_Insert.
*/
ಸ್ಥಿತಿ OP_Idx_ಸೇರಿಸು: {        /* in2 */
  VdbeCursor *pC;
  BtreePayload x;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  sqlite3VdbeIncrWriteCounter(p, pC);
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( !isSorter(pC) );
  pIn2 = &aMem[pOp->p2];
  ಪ್ರತಿಜ್ಞಾ( (pIn2->flags & MEM_Blob) || (pOp->p5 & OPFLAG_PREFORMAT) );
  ಆದರೆ( pOp->p5 & OPFLAG_NCHANGE ) p->nChange++;
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->is_ಕೋಷ್ಟಕ==0 );
  rc = ExpandBlob(pIn2);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  x.n_ಕೀಲಿ = pIn2->n;
  x.p_ಕೀಲಿ = pIn2->z;
  x.aMem = aMem + pOp->p3;
  x.nMem = (u16)pOp->p4.i;
  rc = sqlite3_Btree_ಸೇರಿಸು(pC->uc.pCursor, &x,
       (pOp->p5 & (OPFLAG_APPEND|OPFLAG_SAVEPOSITION|OPFLAG_PREFORMAT)),
      ((pOp->p5 & OPFLAG_USESEEKRESULT) ? pC->seek_ಫಲಿತಾಂಶ : 0)
      );
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಆದರೆ( rc) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SorterInsert P1 P2 * * *
** Synopsis: key=r[P2]
**
** Register P2 holds an SQL index key made using the
** MakeRecord instructions.  This opcode writes that key
** into the sorter P1.  Data for the entry is nil.
*/
ಸ್ಥಿತಿ OP_Sorter_ಸೇರಿಸು: {     /* in2 */
  VdbeCursor *pC;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  sqlite3VdbeIncrWriteCounter(p, pC);
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( isSorter(pC) );
  pIn2 = &aMem[pOp->p2];
  ಪ್ರತಿಜ್ಞಾ( pIn2->flags & MEM_Blob );
  ಪ್ರತಿಜ್ಞಾ( pC->is_ಕೋಷ್ಟಕ==0 );
  rc = ExpandBlob(pIn2);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  rc = sqlite3VdbeSorterWrite(pC, pIn2);
  ಆದರೆ( rc) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IdxDelete P1 P2 P3 P4 P5
** Synopsis: key=r[P2@P5]
**
** The content of P5 registers starting at register P2 form
** an unpacked index key. This opcode removes that entry from the
** index opened by cursor P1.
**
** P4 is a pointer to an Index structure.
**
** If P3 is non-zero, it is the register number of a register holding
** a record that will be inserted into this index. If that record is
** identical to the one that would be deleted by this instruction, 
** skip the delete and set register P3 to NULL.
**
** Raise an SQLITE_CORRUPT_INDEX error if no matching index entry is found
** and not in writable_schema mode.
*/
ಸ್ಥಿತಿ OP_IdxDelete: {
  VdbeCursor *pC;
  BtCursor *pCrsr;
  ಪೂರ್ಣಾಂಕ res;
  Unpacked_ದಾಖಲೆ r;

  ಪ್ರತಿಜ್ಞಾ( pOp->p5>0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 && pOp->p2+pOp->p5<=(p->nMem+1 - p->nCursor)+1 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  sqlite3VdbeIncrWriteCounter(p, pC);
  pCrsr = pC->uc.pCursor;
  ಪ್ರತಿಜ್ಞಾ( pCrsr!=0 );
  r.p_ಕೀಲಿ_Info = pC->p_ಕೀಲಿ_Info;
  r.n_ಕ್ಷೇತ್ರ = pOp->p5;
  r.default_rc = 0;
  r.aMem = &aMem[pOp->p2];
  rc = sqlite3_Btree_ಸೂಚ್ಯಂಕ_Moveto(pCrsr, &r, &res);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ಆದರೆ( res!=0 ){
    rc = sqlite3_Vdbe_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(pCrsr, pOp->p4.pIdx, &r, &res, 0);
    ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    ಆದರೆ( res!=0 ){
      ಆದರೆ( !sqlite3WritableSchema(db) ){
        rc = sqlite3ReportError(
            SQLITE_CORRUPT_ಸೂಚ್ಯಂಕ, __LINE__, "index corruption");
        ಹೋಗು abort_due_ಗಮ್ಯ_error;
      }
      pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
      pC->seek_ಫಲಿತಾಂಶ = 0;
      ನಿಲ್ಲಿಸು;
    }
  }

  ಆದರೆ( pOp->p3 && vdbe_ಸೂಚ್ಯಂಕ_ಕೀಲಿ_Compare(pCrsr, &aMem[pOp->p3], &rc) ){
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    sqlite3_Vdbe_Mem_ಗಣ_Null(&aMem[pOp->p3]);
    ನಿಲ್ಲಿಸು;
  }

  rc = sqlite3BtreeDelete(pCrsr, BTREE_AUXDELETE);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  pC->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  pC->seek_ಫಲಿತಾಂಶ = 0;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: DeferredSeek P1 * P3 P4 *
** Synopsis: Move P3 to P1.rowid if needed
**
** P1 is an open index cursor and P3 is a cursor on the corresponding
** table.  This opcode does a deferred seek of the P3 table cursor
** to the row that corresponds to the current row of P1.
**
** This is a deferred seek.  Nothing actually happens until
** the cursor is used to read a record.  That way, if no reads
** occur, no unnecessary I/O happens.
**
** P4 may be an array of integers (type P4_INTARRAY) containing
** one entry for each column in the P3 table.  If array entry a(i)
** is non-zero, then reading column a(i)-1 from cursor P3 is
** equivalent to performing the deferred seek and then reading column i
** from P1.  This information is stored in P3 and used to redirect
** reads against P3 over to P1, thus possibly avoiding the need to
** seek and read cursor P3.
*/
/* Opcode: IdxRowid P1 P2 * * *
** Synopsis: r[P2]=rowid
**
** Write into register P2 an integer which is the last entry in the record at
** the end of the index key pointed to by cursor P1.  This integer should be
** the rowid of the table entry to which this index entry points.
**
** See also: Rowid, MakeRecord.
*/
ಸ್ಥಿತಿ OP_DeferredSeek:         /* ncycle */
ಸ್ಥಿತಿ OP_IdxRowid: {           /* out2, ncycle */
  VdbeCursor *pC;             /* The P1 index cursor */
  VdbeCursor *pTabCur;        /* The P2 table cursor (OP_DeferredSeek only) */
  i64 rowid;                  /* Rowid that P1 current points to */

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE || IsNullCursor(pC) );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->is_ಕೋಷ್ಟಕ==0 || IsNullCursor(pC) );
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  ಪ್ರತಿಜ್ಞಾ( !pC->null_ಅಡ್ಡಸಾಲು || pOp->opcode==OP_IdxRowid );

  /* The IdxRowid and Seek opcodes are combined because of the commonality
  ** of sqlite3VdbeCursorRestore() and sqlite3VdbeIdxRowid(). */
  rc = sqlite3VdbeCursorRestore(pC);

  /* sqlite3VdbeCursorRestore() may fail if the cursor has been disturbed
  ** since it was last positioned and an error (e.g. OOM or an IO error)
  ** occurs while trying to reposition it. */
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abort_due_ಗಮ್ಯ_error;

  ಆದರೆ( !pC->null_ಅಡ್ಡಸಾಲು ){
    rowid = 0;  /* Not needed.  Only used to silence a warning. */
    rc = sqlite3VdbeIdxRowid(db, pC->uc.pCursor, &rowid);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }
    ಆದರೆ( pOp->opcode==OP_DeferredSeek ){
      ಪ್ರತಿಜ್ಞಾ( pOp->p3>=0 && pOp->p3<p->nCursor );
      pTabCur = p->apCsr[pOp->p3];
      ಪ್ರತಿಜ್ಞಾ( pTabCur!=0 );
      ಪ್ರತಿಜ್ಞಾ( pTabCur->eCurType==CURTYPE_BTREE );
      ಪ್ರತಿಜ್ಞಾ( pTabCur->uc.pCursor!=0 );
      ಪ್ರತಿಜ್ಞಾ( pTabCur->is_ಕೋಷ್ಟಕ );
#ಆದರೆ defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
      ಪ್ರತಿಜ್ಞಾ( 
          sqlite3BtreeCursorHintTblCsr(pC->uc.pCursor)==pTabCur->uc.pCursor 
      );
#ಆದರೆ_ಅಂತ್ಯ
      pTabCur->null_ಅಡ್ಡಸಾಲು = 0;
      pTabCur->moveto_ಗುರಿ = rowid;
      pTabCur->deferredMoveto = 1;
      pTabCur->cache_ಸ್ಥಿತಿ = CACHE_STALE;
      ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INTARRAY || pOp->p4.ai==0 );
      ಪ್ರತಿಜ್ಞಾ( !pTabCur->isEphemeral );
      pTabCur->ub.aAltMap = pOp->p4.ai;
      ಪ್ರತಿಜ್ಞಾ( !pC->isEphemeral );
      pTabCur->pAltCursor = pC;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
      p_ಹೊರಹೋಗು->u.i = rowid;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_IdxRowid );
    sqlite3_Vdbe_Mem_ಗಣ_Null(&aMem[pOp->p2]);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: FinishSeek P1 * * * *
**
** If cursor P1 was previously moved via OP_DeferredSeek, complete that
** seek operation now, without further delay.  If the cursor seek has
** already occurred, this instruction is a no-op.
*/
ಸ್ಥಿತಿ OP_ಮುಕ್ತಾಯ_Seek: {        /* ncycle */
  VdbeCursor *pC;            /* The P1 index cursor */

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಆದರೆ( pC->deferredMoveto ){
    rc = sqlite3_Vdbe_ಮುಕ್ತಾಯ_Moveto(pC);
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IdxGE P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** The P4 register values beginning with P3 form an unpacked index
** key that omits the PRIMARY KEY.  Compare this key value against the index
** that P1 is currently pointing to, ignoring the PRIMARY KEY or ROWID
** fields at the end.
**
** If the P1 index entry is greater than or equal to the key value
** then jump to P2.  Otherwise fall through to the next instruction.
*/
/* Opcode: IdxGT P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** The P4 register values beginning with P3 form an unpacked index
** key that omits the PRIMARY KEY.  Compare this key value against the index
** that P1 is currently pointing to, ignoring the PRIMARY KEY or ROWID
** fields at the end.
**
** If the P1 index entry is greater than the key value
** then jump to P2.  Otherwise fall through to the next instruction.
*/
/* Opcode: IdxLT P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** The P4 register values beginning with P3 form an unpacked index
** key that omits the PRIMARY KEY or ROWID.  Compare this key value against
** the index that P1 is currently pointing to, ignoring the PRIMARY KEY or
** ROWID on the P1 index.
**
** If the P1 index entry is less than the key value then jump to P2.
** Otherwise fall through to the next instruction.
*/
/* Opcode: IdxLE P1 P2 P3 P4 *
** Synopsis: key=r[P3@P4]
**
** The P4 register values beginning with P3 form an unpacked index
** key that omits the PRIMARY KEY or ROWID.  Compare this key value against
** the index that P1 is currently pointing to, ignoring the PRIMARY KEY or
** ROWID on the P1 index.
**
** If the P1 index entry is less than or equal to the key value then jump
** to P2. Otherwise fall through to the next instruction.
*/
ಸ್ಥಿತಿ OP_IdxLE:          /* jump, ncycle */
ಸ್ಥಿತಿ OP_IdxGT:          /* jump, ncycle */
ಸ್ಥಿತಿ OP_IdxLT:          /* jump, ncycle */
ಸ್ಥಿತಿ OP_IdxGE:  {       /* jump, ncycle */
  VdbeCursor *pC;
  ಪೂರ್ಣಾಂಕ res;
  Unpacked_ದಾಖಲೆ r;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->isOrdered );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0);
  ಪ್ರತಿಜ್ಞಾ( pC->deferredMoveto==0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INT32 );
  r.p_ಕೀಲಿ_Info = pC->p_ಕೀಲಿ_Info;
  r.n_ಕ್ಷೇತ್ರ = (u16)pOp->p4.i;
  ಆದರೆ( pOp->opcode<OP_IdxLT ){
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_IdxLE || pOp->opcode==OP_IdxGT );
    r.default_rc = -1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_IdxGE || pOp->opcode==OP_IdxLT );
    r.default_rc = 0;
  }
  r.aMem = &aMem[pOp->p3];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  {
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<r.n_ಕ್ಷೇತ್ರ; i++){
      ಪ್ರತಿಜ್ಞಾ( memIsValid(&r.aMem[i]) );
      ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3+i, &aMem[pOp->p3+i]);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Inlined version of sqlite3VdbeIdxKeyCompare() */
  {
    i64 n_Cell_ಕೀಲಿ = 0;
    BtCursor *pCur;
    Mem m;

    ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
    pCur = pC->uc.pCursor;
    ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorIsValid(pCur) );
    n_Cell_ಕೀಲಿ = sqlite3_Btree_Payload_ಗಾತ್ರ(pCur);
    /* nCellKey will always be between 0 and 0xffffffff because of the way
    ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
    ಆದರೆ( n_Cell_ಕೀಲಿ<=0 || n_Cell_ಕೀಲಿ>0x7fffffff ){
      rc = SQLITE_CORRUPT_BKPT;
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }
    sqlite3VdbeMemInit(&m, db, 0);
    rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(pCur, (u32)n_Cell_ಕೀಲಿ, &m);
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    res = sqlite3_Vdbe_ದಾಖಲೆ_Compare_With_Skip(m.n, m.z, &r, 0);
    sqlite3VdbeMemReleaseMalloc(&m);
  }
  /* End of inlined sqlite3VdbeIdxKeyCompare() */

  ಪ್ರತಿಜ್ಞಾ( (OP_IdxLE&1)==(OP_IdxLT&1) && (OP_IdxGE&1)==(OP_IdxGT&1) );
  ಆದರೆ( (pOp->opcode&1)==(OP_IdxLT&1) ){
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_IdxLE || pOp->opcode==OP_IdxLT );
    res = -res;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_IdxGE || pOp->opcode==OP_IdxGT );
    res++;
  }
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res>0,2);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  ಆದರೆ( res>0 ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Destroy P1 P2 P3 * *
**
** Delete an entire database table or index whose root page in the database
** file is given by P1.
**
** The table being destroyed is in the main database file if P3==0.  If
** P3==1 then the table to be destroyed is in the auxiliary database file
** that is used to store tables create using CREATE TEMPORARY TABLE.
**
** If AUTOVACUUM is enabled then it is possible that another root page
** might be moved into the newly deleted root page in order to keep all
** root pages contiguous at the beginning of the database.  The former
** value of the root page that moved - its value before the move occurred -
** is stored in register P2. If no page movement was required (because the
** table being dropped was already the last one in the database) then a
** zero is stored in register P2.  If AUTOVACUUM is disabled then a zero
** is stored in register P2.
**
** This opcode throws an error if there are any active reader VMs when
** it is invoked. This is done to avoid the difficulty associated with
** updating existing cursors when a root page is moved in an AUTOVACUUM
** database. This error is thrown even if the database is not an AUTOVACUUM
** db in order to avoid introducing an incompatibility between autovacuum
** and non-autovacuum modes.
**
** See also: Clear
*/
ಸ್ಥಿತಿ OP_Destroy: {     /* out2 */
  ಪೂರ್ಣಾಂಕ iMoved;
  ಪೂರ್ಣಾಂಕ iDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>1 );
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->flags = MEM_Null;
  ಆದರೆ( db->nVdbeRead > db->nVDestroy+1 ){
    rc = SQLITE_LOCKED;
    p->errorAction = OE_Abort;
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }ಇಲ್ಲದಿದ್ದರೆ{
    iDb = pOp->p3;
    ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, iDb) );
    iMoved = 0;  /* Not needed.  Only to silence a warning. */
    rc = sqlite3_Btree_Drop_ಕೋಷ್ಟಕ(db->aDb[iDb].pBt, pOp->p1, &iMoved);
    p_ಹೊರಹೋಗು->flags = MEM_Int;
    p_ಹೊರಹೋಗು->u.i = iMoved;
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( iMoved!=0 ){
      sqlite3_ಬೇರು_ಪುಟ_Moved(db, iDb, iMoved, pOp->p1);
      /* All OP_Destroy operations occur on the same btree */
      ಪ್ರತಿಜ್ಞಾ( resetSchemaOnFault==0 || resetSchemaOnFault==iDb+1 );
      resetSchemaOnFault = iDb+1;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Clear P1 P2 P3
**
** Delete all contents of the database table or index whose root page
** in the database file is given by P1.  But, unlike Destroy, do not
** remove the table or index from the database file.
**
** The table being cleared is in the main database file if P2==0.  If
** P2==1 then the table to be cleared is in the auxiliary database file
** that is used to store tables create using CREATE TEMPORARY TABLE.
**
** If the P3 value is non-zero, then the row change count is incremented
** by the number of rows in the table being cleared. If P3 is greater
** than zero, then the value stored in register P3 is also incremented
** by the number of rows in the table being cleared.
**
** See also: Destroy
*/
ಸ್ಥಿತಿ OP_Clear: {
  i64 nChange;

  sqlite3VdbeIncrWriteCounter(p, 0);
  nChange = 0;
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, pOp->p2) );
  rc = sqlite3_Btree_Clear_ಕೋಷ್ಟಕ(db->aDb[pOp->p2].pBt, (u32)pOp->p1, &nChange);
  ಆದರೆ( pOp->p3 ){
    p->nChange += nChange;
    ಆದರೆ( pOp->p3>0 ){
      ಪ್ರತಿಜ್ಞಾ( memIsValid(&aMem[pOp->p3]) );
      mem_About_ಗಮ್ಯ_Change(p, &aMem[pOp->p3]);
      aMem[pOp->p3].u.i += nChange;
    }
  }
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: ResetSorter P1 * * * *
**
** Delete all contents from the ephemeral table or sorter
** that is open on cursor P1.
**
** This opcode only works for cursors used for sorting and
** opened with OP_OpenEphemeral or OP_SorterOpen.
*/
ಸ್ಥಿತಿ OP_ResetSorter: {
  VdbeCursor *pC;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಆದರೆ( isSorter(pC) ){
    sqlite3VdbeSorterReset(db, pC->uc.pSorter);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
    ಪ್ರತಿಜ್ಞಾ( pC->isEphemeral );
    rc = sqlite3_Btree_Clear_ಕೋಷ್ಟಕ_ದ_Cursor(pC->uc.pCursor);
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: CreateBtree P1 P2 P3 * *
** Synopsis: r[P2]=root iDb=P1 flags=P3
**
** Allocate a new b-tree in the main database file if P1==0 or in the
** TEMP database file if P1==1 or in an attached database if
** P1>1.  The P3 argument must be 1 (BTREE_INTKEY) for a rowid table
** it must be 2 (BTREE_BLOBKEY) for an index or WITHOUT ROWID table.
** The root page number of the new b-tree is stored in register P2.
*/
ಸ್ಥಿತಿ OP_CreateBtree: {          /* out2 */
  Pgno pgno;
  Db *pDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  pgno = 0;
  ಪ್ರತಿಜ್ಞಾ( pOp->p3==BTREE_INTKEY || pOp->p3==BTREE_BLOBKEY );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, pOp->p1) );
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  pDb = &db->aDb[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pDb->pBt!=0 );
  rc = sqlite3_Btree_Create_ಕೋಷ್ಟಕ(pDb->pBt, &pgno, pOp->p3);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  p_ಹೊರಹೋಗು->u.i = pgno;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SqlExec P1 P2 * P4 *
**
** Run the SQL statement or statements specified in the P4 string.
**
** The P1 parameter is a bitmask of options:
**
**    0x0001     Disable Auth and Trace callbacks while the statements
**               in P4 are running.
**
**    0x0002     Set db->nAnalysisLimit to P2 while the statements in
**               P4 are running.
**
*/
ಸ್ಥಿತಿ OP_SqlExec: {
  ಅಕ್ಷರ *zErr;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;
#ಆದರೆ_ಅಂತ್ಯ
  u8 mTrace;
  ಪೂರ್ಣಾಂಕ saved_Analysis_ಎಲ್ಲೆ;

  sqlite3VdbeIncrWriteCounter(p, 0);
  db->nSqlExec++;
  zErr = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  xAuth = db->xAuth;
#ಆದರೆ_ಅಂತ್ಯ
  mTrace = db->mTrace;
  saved_Analysis_ಎಲ್ಲೆ = db->n_Analysis_ಎಲ್ಲೆ;
  ಆದರೆ( pOp->p1 & 0x0001 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
    db->xAuth = 0;
#ಆದರೆ_ಅಂತ್ಯ
    db->mTrace = 0;
  }
  ಆದರೆ( pOp->p1 & 0x0002 ){
    db->n_Analysis_ಎಲ್ಲೆ = pOp->p2;
  }
  rc = sqlite3_exec(db, pOp->p4.z, 0, 0, &zErr);
  db->nSqlExec--;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ಆದರೆ_ಅಂತ್ಯ
  db->mTrace = mTrace;
  db->n_Analysis_ಎಲ್ಲೆ = saved_Analysis_ಎಲ್ಲೆ;
  ಆದರೆ( zErr || rc ){
    sqlite3VdbeError(p, "%s", zErr);
    sqlite3_free(zErr);
    ಆದರೆ( rc==SQLITE_NOMEM ) ಹೋಗು no_mem;
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: ParseSchema P1 * * P4 P5
**
** Read and parse all entries from the schema table of database P1
** that match the WHERE clause P4.  If P4 is a NULL pointer, then the
** entire schema for P1 is reparsed.
**
** When P4 is NULL, the P5 value is used as the mFlags argument
** to sqlite3InitOne().  In other words, P5 should be a mask composed
** of INITFLAG_* values.
**
** The P4==0 case is only used by ALTER TABLE and P5!=0 for all such
** cases.  For uses other than ALTER TABLE, P4<>0 and P5==0.
**
** This opcode invokes the parser to create a new virtual machine,
** then runs the new virtual machine.  It is thus a re-entrant opcode.
*/
ಸ್ಥಿತಿ OP_ParseSchema: {
  ಪೂರ್ಣಾಂಕ iDb;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema;
  ಅಕ್ಷರ *zSql;
  Init_ದತ್ತಾಂಶ init_ದತ್ತಾಂಶ;

  /* Any prepared statement that invokes this opcode will hold mutexes
  ** on every btree.  This is a prerequisite for invoking
  ** sqlite3InitCallback().
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಗಾಗಿ(iDb=0; iDb<db->nDb; iDb++){
    ಪ್ರತಿಜ್ಞಾ( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );
  }
#ಆದರೆ_ಅಂತ್ಯ

  iDb = pOp->p1;
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( DbHasProperty(db, iDb, DB_SchemaLoaded)
           || db->mallocFailed
           || (CORRUPT_DB && (db->flags & SQLITE_NoSchemaError)!=0) );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ALTERTABLE
  ಆದರೆ( pOp->p4.z==0 ){
    ಪ್ರತಿಜ್ಞಾ( pOp->p5!=0 );
    sqlite3SchemaClear(db->aDb[iDb].pSchema);
    db->mDbFlags &= ~DBFLAG_Schema_ತಿಳಿದ_Ok;
    rc = sqlite3InitOne(db, iDb, &p->z_Err_ಸಂದೇಶ, pOp->p5);
    db->mDbFlags |= DBFLAG_SchemaChange;
    p->expired = 0;
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  {
    ಪ್ರತಿಜ್ಞಾ( pOp->p5==0 );
    zSchema = LEGACY_SCHEMA_ಕೋಷ್ಟಕ;
    init_ದತ್ತಾಂಶ.db = db;
    init_ದತ್ತಾಂಶ.iDb = iDb;
    init_ದತ್ತಾಂಶ.pz_Err_ಸಂದೇಶ = &p->z_Err_ಸಂದೇಶ;
    init_ದತ್ತಾಂಶ.mInitFlags = 0;
    init_ದತ್ತಾಂಶ.mx_ಪುಟ = sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(db->aDb[iDb].pBt);
    zSql = sqlite3MPrintf(db,
       "SELECT*FROM\"%w\".%s WHERE %s ORDER BY rowid",
       db->aDb[iDb].zDbSName, zSchema, pOp->p4.z);
    ಆದರೆ( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( db->init.busy==0 );
      db->init.busy = 1;
      init_ದತ್ತಾಂಶ.rc = SQLITE_OK;
      init_ದತ್ತಾಂಶ.n_Init_ಅಡ್ಡಸಾಲು = 0;
      ಪ್ರತಿಜ್ಞಾ( !db->mallocFailed );
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &init_ದತ್ತಾಂಶ, 0);
      ಆದರೆ( rc==SQLITE_OK ) rc = init_ದತ್ತಾಂಶ.rc;
      ಆದರೆ( rc==SQLITE_OK && init_ದತ್ತಾಂಶ.n_Init_ಅಡ್ಡಸಾಲು==0 ){
        /* The OP_ParseSchema opcode with a non-NULL P4 argument should parse
        ** at least one SQL statement. Any less than that indicates that
        ** the sqlite_schema table is corrupt. */
        rc = SQLITE_CORRUPT_BKPT;
      }
      sqlite3DbFreeNN(db, zSql);
      db->init.busy = 0;
    }
  }
  ಆದರೆ( rc ){
    sqlite3_Reset_All_Schemas_ದ_Connection(db);
    ಆದರೆ( rc==SQLITE_NOMEM ){
      ಹೋಗು no_mem;
    }
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  ನಿಲ್ಲಿಸು; 
}

#ಆದರೆ !defined(SQLITE_OMIT_ANALYZE)
/* Opcode: LoadAnalysis P1 * * * *
**
** Read the sqlite_stat1 table for database P1 and load the content
** of that table into the internal index hash table.  This will cause
** the analysis to be used when preparing all subsequent queries.
*/
ಸ್ಥಿತಿ OP_ತುಂಬು_Analysis: {
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<db->nDb );
  rc = sqlite3_Analysis_ತುಂಬು(db, pOp->p1);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು; 
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_ANALYZE) */

/* Opcode: DropTable P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the table named P4 in database P1.  This is called after a table
** is dropped from disk (using the Destroy opcode) in order to keep
** the internal representation of the
** schema consistent with what is on disk.
*/
ಸ್ಥಿತಿ OP_Drop_ಕೋಷ್ಟಕ: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3_Unlink_And_Delete_ಕೋಷ್ಟಕ(db, pOp->p1, pOp->p4.z);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: DropIndex P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the index named P4 in database P1.  This is called after an index
** is dropped from disk (using the Destroy opcode)
** in order to keep the internal representation of the
** schema consistent with what is on disk.
*/
ಸ್ಥಿತಿ OP_Drop_ಸೂಚ್ಯಂಕ: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3_Unlink_And_Delete_ಸೂಚ್ಯಂಕ(db, pOp->p1, pOp->p4.z);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: DropTrigger P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the trigger named P4 in database P1.  This is called after a trigger
** is dropped from disk (using the Destroy opcode) in order to keep
** the internal representation of the
** schema consistent with what is on disk.
*/
ಸ್ಥಿತಿ OP_DropTrigger: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3UnlinkAndDeleteTrigger(db, pOp->p1, pOp->p4.z);
  ನಿಲ್ಲಿಸು;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INTEGRITY_CHECK
/* Opcode: IntegrityCk P1 P2 P3 P4 P5
**
** Do an analysis of the currently open database.  Store in
** register (P1+1) the text of an error message describing any problems.
** If no problems are found, store a NULL in register (P1+1).
**
** The register (P1) contains one less than the maximum number of allowed
** errors.  At most reg(P1) errors will be reported.
** In other words, the analysis stops as soon as reg(P1) errors are
** seen.  Reg(P1) is updated with the number of errors remaining.
**
** The root page numbers of all tables in the database are integers
** stored in P4_INTARRAY argument.
**
** If P5 is not zero, the check is done on the auxiliary database
** file, not the main database file.
**
** This opcode is used to implement the integrity_check pragma.
*/
ಸ್ಥಿತಿ OP_IntegrityCk: {
  ಪೂರ್ಣಾಂಕ n_ಬೇರು;      /* Number of tables to check.  (Number of root pages.) */
  Pgno *a_ಬೇರು;    /* Array of rootpage numbers for tables to be checked */
  ಪೂರ್ಣಾಂಕ nErr;       /* Number of errors reported */
  ಅಕ್ಷರ *z;        /* Text of the error report */
  Mem *pnErr;     /* Register keeping track of errors remaining */

  ಪ್ರತಿಜ್ಞಾ( p->bIsReader );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INTARRAY );
  n_ಬೇರು = pOp->p2;
  a_ಬೇರು = pOp->p4.ai;
  ಪ್ರತಿಜ್ಞಾ( n_ಬೇರು>0 );
  ಪ್ರತಿಜ್ಞಾ( a_ಬೇರು!=0 );
  ಪ್ರತಿಜ್ಞಾ( a_ಬೇರು[0]==(Pgno)n_ಬೇರು );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 && (pOp->p1+1)<=(p->nMem+1 - p->nCursor) );
  pnErr = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( (pnErr->flags & MEM_Int)!=0 );
  ಪ್ರತಿಜ್ಞಾ( (pnErr->flags & (MEM_Str|MEM_Blob))==0 );
  pIn1 = &aMem[pOp->p1+1];
  ಪ್ರತಿಜ್ಞಾ( pOp->p5<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, pOp->p5) );
  rc = sqlite3BtreeIntegrityCheck(db, db->aDb[pOp->p5].pBt, &a_ಬೇರು[1], 
      &aMem[pOp->p3], n_ಬೇರು, (ಪೂರ್ಣಾಂಕ)pnErr->u.i+1, &nErr, &z);
  sqlite3_Vdbe_Mem_ಗಣ_Null(pIn1);
  ಆದರೆ( nErr==0 ){
    ಪ್ರತಿಜ್ಞಾ( z==0 );
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc ){
    sqlite3_free(z);
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pnErr->u.i -= nErr-1;
    sqlite3_Vdbe_Mem_ಗಣ_Str(pIn1, z, -1, SQLITE_UTF8, sqlite3_free);
  }
  UPDATE_MAX_BLOBSIZE(pIn1);
  sqlite3VdbeChangeEncoding(pIn1, encoding);
  ಹೋಗು check_for_ಅಡಚಣೆ;
}

/* Opcode: IFindKey P1 P2 P3 P4 *
**
** This instruction always follows an OP_Found with the same P1, P2 and P3
** values as this instruction and a non-zero P4 value. The P4 value to
** this opcode is of type P4_INDEX and contains a pointer to the Index
** object of for the index being searched.
**
** This opcode uses sqlite3VdbeFindIndexKey() to search around the current
** cursor location for an index key that exactly matches all fields that
** are not indexed expressions or references to VIRTUAL generated columns,
** and either exactly match or are real numbers that are within 2 ULPs of
** each other if the don't match.
**
** To put it another way, this opcode looks for nearby index entries that
** are very close to the search key, but which might have small differences
** in floating-point values that come via an expression.
**
** If no nearby alternative entry is found in cursor P1, then jump to P2.
** But if a close match is found, fall through.
**
** This opcode is used by PRAGMA integrity_check to help distinguish
** between truely corrupt indexes and expression indexes that are holding
** floating-point values that are off by one or two ULPs.
*/
ಸ್ಥಿತಿ OP_IFind_ಕೀಲಿ: {     /* jump, in3 */
  VdbeCursor *pC;
  ಪೂರ್ಣಾಂಕ res;
  Unpacked_ದಾಖಲೆ r;

  ಪ್ರತಿಜ್ಞಾ( pOp[-1].opcode==OP_ಸಿಕ್ಕಿತು );
  ಪ್ರತಿಜ್ಞಾ( pOp[-1].p1==pOp->p1 );
  ಪ್ರತಿಜ್ಞಾ( pOp[-1].p3==pOp->p3 );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_ಸೂಚ್ಯಂಕ );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pC->uc.pCursor!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->is_ಕೋಷ್ಟಕ==0 );

  memset(&r, 0, ಗಾತ್ರ(r));
  r.aMem = &aMem[pOp->p3];
  r.n_ಕ್ಷೇತ್ರ = pOp->p4.pIdx->n_ಸ್ತಂಭ;
  r.p_ಕೀಲಿ_Info = pC->p_ಕೀಲಿ_Info;

  rc = sqlite3_Vdbe_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(pC->uc.pCursor, pOp->p4.pIdx, &r, &res, 1);
  ಆದರೆ( rc || res!=0 ){
    rc = SQLITE_OK;
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  pC->null_ಅಡ್ಡಸಾಲು = 0;
  ನಿಲ್ಲಿಸು;
};
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INTEGRITY_CHECK */

/* Opcode: RowSetAdd P1 P2 * * *
** Synopsis: rowset(P1)=r[P2]
**
** Insert the integer value held by register P2 into a RowSet object
** held in register P1.
**
** An assertion fails if P2 is not an integer.
*/
ಸ್ಥಿತಿ OP_ಅಡ್ಡಸಾಲು_ಗಣ_ಕೂಡಿಸು: {       /* in1, in2 */
  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  ಪ್ರತಿಜ್ಞಾ( (pIn2->flags & MEM_Int)!=0 );
  ಆದರೆ( (pIn1->flags & MEM_Blob)==0 ){
    ಆದರೆ( sqlite3_Vdbe_Mem_ಗಣ_ಅಡ್ಡಸಾಲು_ಗಣ(pIn1) ) ಹೋಗು no_mem;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pIn1) );
  sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_ಸೇರಿಸು((ಅಡ್ಡಸಾಲು_ಗಣ*)pIn1->z, pIn2->u.i);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: RowSetRead P1 P2 P3 * *
** Synopsis: r[P3]=rowset(P1)
**
** Extract the smallest value from the RowSet object in P1
** and put that value into register P3.
** Or, if RowSet object P1 is initially empty, leave P3
** unchanged and jump to instruction P2.
*/
ಸ್ಥಿತಿ OP_ಅಡ್ಡಸಾಲು_ಗಣ_Read: {       /* jump, in1, out3 */
  i64 val;

  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( (pIn1->flags & MEM_Blob)==0 || sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pIn1) );
  ಆದರೆ( (pIn1->flags & MEM_Blob)==0
   || sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_ಮುಂದಿನ((ಅಡ್ಡಸಾಲು_ಗಣ*)pIn1->z, &val)==0
  ){
    /* The boolean index is empty */
    sqlite3_Vdbe_Mem_ಗಣ_Null(pIn1);
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1,2);
    ಹೋಗು jump_ಗಮ್ಯ_p2_and_check_for_ಅಡಚಣೆ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* A value was pulled from the index */
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0,2);
    sqlite3_Vdbe_Mem_ಗಣ_Int64(&aMem[pOp->p3], val);
  }
  ಹೋಗು check_for_ಅಡಚಣೆ;
}

/* Opcode: RowSetTest P1 P2 P3 P4
** Synopsis: if r[P3] in rowset(P1) goto P2
**
** Register P3 is assumed to hold a 64-bit integer value. If register P1
** contains a RowSet object and that RowSet object contains
** the value held in P3, jump to register P2. Otherwise, insert the
** integer in P3 into the RowSet and continue on to the
** next opcode.
**
** The RowSet object is optimized for the case where sets of integers
** are inserted in distinct phases, which each set contains no duplicates.
** Each set is identified by a unique P4 value. The first set
** must have P4==0, the final set must have P4==-1, and for all other sets
** must have P4>0.
**
** This allows optimizations: (a) when P4==0 there is no need to test
** the RowSet object for P3, as it is guaranteed not to contain it,
** (b) when P4==-1 there is no need to insert the value, as it will
** never be tested for, and (c) when a value that is part of set X is
** inserted, there is no need to search to see if the same value was
** previously inserted as part of set X (only if it was previously
** inserted as part of some other set).
*/
ಸ್ಥಿತಿ OP_ಅಡ್ಡಸಾಲು_ಗಣ_Test: {                     /* jump, in1, in3 */
  ಪೂರ್ಣಾಂಕ i_ಗಣ;
  ಪೂರ್ಣಾಂಕ exists;

  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  i_ಗಣ = pOp->p4.i;
  ಪ್ರತಿಜ್ಞಾ( pIn3->flags&MEM_Int );

  /* If there is anything other than a rowset object in memory cell P1,
  ** delete it now and initialize P1 with an empty rowset
  */
  ಆದರೆ( (pIn1->flags & MEM_Blob)==0 ){
    ಆದರೆ( sqlite3_Vdbe_Mem_ಗಣ_ಅಡ್ಡಸಾಲು_ಗಣ(pIn1) ) ಹೋಗು no_mem;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pIn1) );
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_INT32 );
  ಪ್ರತಿಜ್ಞಾ( i_ಗಣ==-1 || i_ಗಣ>=0 );
  ಆದರೆ( i_ಗಣ ){
    exists = sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Test((ಅಡ್ಡಸಾಲು_ಗಣ*)pIn1->z, i_ಗಣ, pIn3->u.i);
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(exists!=0,2);
    ಆದರೆ( exists ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ಆದರೆ( i_ಗಣ>=0 ){
    sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_ಸೇರಿಸು((ಅಡ್ಡಸಾಲು_ಗಣ*)pIn1->z, pIn3->u.i);
  }
  ನಿಲ್ಲಿಸು;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER

/* Opcode: Program P1 P2 P3 P4 P5
**
** Execute the trigger program passed as P4 (type P4_SUBPROGRAM).
**
** P1 contains the address of the memory cell that contains the first memory
** cell in an array of values used as arguments to the sub-program. P2
** contains the address to jump to if the sub-program throws an IGNORE
** exception using the RAISE() function. P2 might be zero, if there is
** no possibility that an IGNORE exception will be raised.
** Register P3 contains the address
** of a memory cell in this (the parent) VM that is used to allocate the
** memory required by the sub-vdbe at runtime.
**
** P4 is a pointer to the VM containing the trigger program.
**
** If P5 is non-zero, then recursive program invocation is enabled.
*/
ಸ್ಥಿತಿ OP_ಕಾರ್ಯಕ್ರಮ: {        /* jump0 */
  ಪೂರ್ಣಾಂಕ nMem;               /* Number of memory registers for sub-program */
  i64 nByte;              /* Bytes of runtime space required for sub-program */
  Mem *pRt;               /* Register to allocate runtime space */
  Mem *pMem;              /* Used to iterate through memory cells */
  Mem *p_ಕೊನೆ;              /* Last memory cell in new array */
  Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು;      /* New vdbe frame to execute in */
  Sub_ಕಾರ್ಯಕ್ರಮ *p_ಕಾರ್ಯಕ್ರಮ;   /* Sub-program to execute */
  ಶೂನ್ಯ *t;                /* Token identifying trigger */

  p_ಕಾರ್ಯಕ್ರಮ = pOp->p4.p_ಕಾರ್ಯಕ್ರಮ;
  pRt = &aMem[pOp->p3];
  ಪ್ರತಿಜ್ಞಾ( p_ಕಾರ್ಯಕ್ರಮ->nOp>0 );
 
  /* If the p5 flag is clear, then recursive invocation of triggers is
  ** disabled for backwards compatibility (p5 is set if this sub-program
  ** is really a trigger, not a foreign key action, and the flag set
  ** and cleared by the "PRAGMA recursive_triggers" command is clear).
  **
  ** It is recursive invocation of triggers, at the SQL level, that is
  ** disabled. In some cases a single trigger may generate more than one
  ** SubProgram (if the trigger may be executed with more than one different
  ** ON CONFLICT algorithm). SubProgram structures associated with a
  ** single trigger all have the same value for the SubProgram.token
  ** variable.  */
  ಆದರೆ( pOp->p5 ){
    t = p_ಕಾರ್ಯಕ್ರಮ->token;
    ಗಾಗಿ(p_ಚೌಕಟ್ಟು=p->p_ಚೌಕಟ್ಟು; p_ಚೌಕಟ್ಟು && p_ಚೌಕಟ್ಟು->token!=t; p_ಚೌಕಟ್ಟು=p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ);
    ಆದರೆ( p_ಚೌಕಟ್ಟು ) ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ( p->n_ಚೌಕಟ್ಟು>=db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_TRIGGER_ಆಳ] ){
    rc = SQLITE_ERROR;
    sqlite3VdbeError(p, "triggers nested too deep");
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }

  /* Register pRt is used to store the memory required to save the state
  ** of the current program, and the memory required at runtime to execute
  ** the trigger program. If this trigger has been fired before, then pRt
  ** is already allocated. Otherwise, it must be initialized.  */
  ಆದರೆ( (pRt->flags&MEM_Blob)==0 ){
    /* SubProgram.nMem is set to the number of memory cells used by the
    ** program stored in SubProgram.aOp. As well as these, one memory
    ** cell is required for each cursor used by the program. Set local
    ** variable nMem (and later, VdbeFrame.nChildMem) to this value.
    */
    nMem = p_ಕಾರ್ಯಕ್ರಮ->nMem + p_ಕಾರ್ಯಕ್ರಮ->nCsr;
    ಪ್ರತಿಜ್ಞಾ( nMem>0 );
    ಆದರೆ( p_ಕಾರ್ಯಕ್ರಮ->nCsr==0 ) nMem++;
    nByte = ROUND8(ಗಾತ್ರ(Vdbe_ಚೌಕಟ್ಟು))
              + nMem * ಗಾತ್ರ(Mem)
              + p_ಕಾರ್ಯಕ್ರಮ->nCsr * ಗಾತ್ರ(VdbeCursor*)
              + (7 + (i64)p_ಕಾರ್ಯಕ್ರಮ->nOp)/8;
    p_ಚೌಕಟ್ಟು = sqlite3DbMallocZero(db, nByte);
    ಆದರೆ( !p_ಚೌಕಟ್ಟು ){
      ಹೋಗು no_mem;
    }
    sqlite3VdbeMemRelease(pRt);
    pRt->flags = MEM_Blob|MEM_Dyn;
    pRt->z = (ಅಕ್ಷರ*)p_ಚೌಕಟ್ಟು;
    pRt->n = (ಪೂರ್ಣಾಂಕ)nByte;
    pRt->xDel = sqlite3_Vdbe_ಚೌಕಟ್ಟು_Mem_Del;

    p_ಚೌಕಟ್ಟು->v = p;
    p_ಚೌಕಟ್ಟು->n_ಮಗು_Mem = nMem;
    p_ಚೌಕಟ್ಟು->n_ಮಗು_Csr = p_ಕಾರ್ಯಕ್ರಮ->nCsr;
    p_ಚೌಕಟ್ಟು->pc = (ಪೂರ್ಣಾಂಕ)(pOp - aOp);
    p_ಚೌಕಟ್ಟು->aMem = p->aMem;
    p_ಚೌಕಟ್ಟು->nMem = p->nMem;
    p_ಚೌಕಟ್ಟು->apCsr = p->apCsr;
    p_ಚೌಕಟ್ಟು->nCursor = p->nCursor;
    p_ಚೌಕಟ್ಟು->aOp = p->aOp;
    p_ಚೌಕಟ್ಟು->nOp = p->nOp;
    p_ಚೌಕಟ್ಟು->token = p_ಕಾರ್ಯಕ್ರಮ->token;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    p_ಚೌಕಟ್ಟು->i_ಚೌಕಟ್ಟು_Magic = SQLITE_ಚೌಕಟ್ಟು_MAGIC;
#ಆದರೆ_ಅಂತ್ಯ

    p_ಕೊನೆ = &Vdbe_ಚೌಕಟ್ಟು_Mem(p_ಚೌಕಟ್ಟು)[p_ಚೌಕಟ್ಟು->n_ಮಗು_Mem];
    ಗಾಗಿ(pMem=Vdbe_ಚೌಕಟ್ಟು_Mem(p_ಚೌಕಟ್ಟು); pMem!=p_ಕೊನೆ; pMem++){
      pMem->flags = MEM_Undefined;
      pMem->db = db;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಚೌಕಟ್ಟು = (Vdbe_ಚೌಕಟ್ಟು*)pRt->z;
    ಪ್ರತಿಜ್ಞಾ( pRt->xDel==sqlite3_Vdbe_ಚೌಕಟ್ಟು_Mem_Del );
    ಪ್ರತಿಜ್ಞಾ( p_ಕಾರ್ಯಕ್ರಮ->nMem+p_ಕಾರ್ಯಕ್ರಮ->nCsr==p_ಚೌಕಟ್ಟು->n_ಮಗು_Mem
        || (p_ಕಾರ್ಯಕ್ರಮ->nCsr==0 && p_ಕಾರ್ಯಕ್ರಮ->nMem+1==p_ಚೌಕಟ್ಟು->n_ಮಗು_Mem) );
    ಪ್ರತಿಜ್ಞಾ( p_ಕಾರ್ಯಕ್ರಮ->nCsr==p_ಚೌಕಟ್ಟು->n_ಮಗು_Csr );
    ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಂಕ)(pOp - aOp)==p_ಚೌಕಟ್ಟು->pc );
  }

  p->n_ಚೌಕಟ್ಟು++;
  p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ = p->p_ಚೌಕಟ್ಟು;
  p_ಚೌಕಟ್ಟು->ಕೊನೆಯ_Rowid = db->ಕೊನೆಯ_Rowid;
  p_ಚೌಕಟ್ಟು->nChange = p->nChange;
  p_ಚೌಕಟ್ಟು->nDbChange = p->db->nChange;
  ಪ್ರತಿಜ್ಞಾ( p_ಚೌಕಟ್ಟು->p_Aux_ದತ್ತಾಂಶ==0 );
  p_ಚೌಕಟ್ಟು->p_Aux_ದತ್ತಾಂಶ = p->p_Aux_ದತ್ತಾಂಶ;
  p->p_Aux_ದತ್ತಾಂಶ = 0;
  p->nChange = 0;
  p->p_ಚೌಕಟ್ಟು = p_ಚೌಕಟ್ಟು;
  p->aMem = aMem = Vdbe_ಚೌಕಟ್ಟು_Mem(p_ಚೌಕಟ್ಟು);
  p->nMem = p_ಚೌಕಟ್ಟು->n_ಮಗು_Mem;
  p->nCursor = (u16)p_ಚೌಕಟ್ಟು->n_ಮಗು_Csr;
  p->apCsr = (VdbeCursor **)&aMem[p->nMem];
  p_ಚೌಕಟ್ಟು->aOnce = (u8*)&p->apCsr[p_ಕಾರ್ಯಕ್ರಮ->nCsr];
  memset(p_ಚೌಕಟ್ಟು->aOnce, 0, (p_ಕಾರ್ಯಕ್ರಮ->nOp + 7)/8);
  p->aOp = aOp = p_ಕಾರ್ಯಕ್ರಮ->aOp;
  p->nOp = p_ಕಾರ್ಯಕ್ರಮ->nOp;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Verify that second and subsequent executions of the same trigger do not
  ** try to reuse register values from the first use. */
  {
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<p->nMem; i++){
      aMem[i].pScopyFrom = 0;  /* Prevent false-positive AboutToChange() errs */
      Mem_ಗಣ_Type_Flag(&aMem[i], MEM_Undefined); /* Fault if this reg is reused */
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  pOp = &aOp[-1];
  ಹೋಗು check_for_ಅಡಚಣೆ;
}

/* Opcode: Param P1 P2 * * *
**
** This opcode is only ever present in sub-programs called via the
** OP_Program instruction. Copy a value currently stored in a memory
** cell of the calling (parent) frame to cell P2 in the current frames
** address space. This is used by trigger programs to access the new.*
** and old.* values.
**
** The address of the cell in the parent frame is determined by adding
** the value of the P1 argument to the value of the P1 argument to the
** calling OP_Program instruction.
*/
ಸ್ಥಿತಿ OP_Param: {           /* out2 */
  Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು;
  Mem *p_ಒಳ;
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಚೌಕಟ್ಟು = p->p_ಚೌಕಟ್ಟು;
  p_ಒಳ = &p_ಚೌಕಟ್ಟು->aMem[pOp->p1 + p_ಚೌಕಟ್ಟು->aOp[p_ಚೌಕಟ್ಟು->pc].p1];  
  sqlite3VdbeMemShallowCopy(p_ಹೊರಹೋಗು, p_ಒಳ, MEM_Ephem);
  ನಿಲ್ಲಿಸು;
}

#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_TRIGGER */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
/* Opcode: FkCounter P1 P2 * * *
** Synopsis: fkctr[P1]+=P2
**
** Increment a "constraint counter" by P2 (P2 may be negative or positive).
** If P1 is non-zero, the database constraint counter is incremented
** (deferred foreign key constraints). Otherwise, if P1 is zero, the
** statement counter is incremented (immediate foreign key constraints).
*/
ಸ್ಥಿತಿ OP_FkCounter: {
  ಆದರೆ( pOp->p1 ){
    db->nDeferredCons += pOp->p2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( db->flags & SQLITE_DeferFKs ){
      db->nDeferredImmCons += pOp->p2;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->nFkConstraint += pOp->p2;
    }
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: FkIfZero P1 P2 * * *
** Synopsis: if fkctr[P1]==0 goto P2
**
** This opcode tests if a foreign key constraint-counter is currently zero.
** If so, jump to instruction P2. Otherwise, fall through to the next
** instruction.
**
** If P1 is non-zero, then the jump is taken if the database constraint-counter
** is zero (the one that counts deferred constraint violations). If P1 is
** zero, the jump is taken if the statement constraint-counter is zero
** (immediate foreign key constraint violations).
*/
ಸ್ಥಿತಿ OP_FkIfZero: {         /* jump */
  ಆದರೆ( pOp->p1 ){
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(db->nDeferredCons==0 && db->nDeferredImmCons==0, 2);
    ಆದರೆ( db->nDeferredCons==0 && db->nDeferredImmCons==0 ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(p->nFkConstraint==0 && db->nDeferredImmCons==0, 2);
    ಆದರೆ( p->nFkConstraint==0 && db->nDeferredImmCons==0 ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_FOREIGN_KEY */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINCREMENT
/* Opcode: MemMax P1 P2 * * *
** Synopsis: r[P1]=max(r[P1],r[P2])
**
** P1 is a register in the root frame of this VM (the root frame is
** different from the current frame if this instruction is being executed
** within a sub-program). Set the value of register P1 to the maximum of
** its current value and the value in register P2.
**
** This instruction throws an error if the memory cell is not initially
** an integer.
*/
ಸ್ಥಿತಿ OP_MemMax: {        /* in2 */
  Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು;
  ಆದರೆ( p->p_ಚೌಕಟ್ಟು ){
    ಗಾಗಿ(p_ಚೌಕಟ್ಟು=p->p_ಚೌಕಟ್ಟು; p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ; p_ಚೌಕಟ್ಟು=p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ);
    pIn1 = &p_ಚೌಕಟ್ಟು->aMem[pOp->p1];
  }ಇಲ್ಲದಿದ್ದರೆ{
    pIn1 = &aMem[pOp->p1];
  }
  ಪ್ರತಿಜ್ಞಾ( memIsValid(pIn1) );
  sqlite3VdbeMemIntegerify(pIn1);
  pIn2 = &aMem[pOp->p2];
  sqlite3VdbeMemIntegerify(pIn2);
  ಆದರೆ( pIn1->u.i<pIn2->u.i){
    pIn1->u.i = pIn2->u.i;
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_AUTOINCREMENT */

/* Opcode: IfPos P1 P2 P3 * *
** Synopsis: if r[P1]>0 then r[P1]-=P3, goto P2
**
** Register P1 must contain an integer.
** If the value of register P1 is 1 or greater, subtract P3 from the
** value in P1 and jump to P2.
**
** If the initial value of register P1 is less than 1, then the
** value is unchanged and control passes through to the next instruction.
*/
ಸ್ಥಿತಿ OP_IfPos: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pIn1->flags&MEM_Int );
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ( pIn1->u.i>0, 2);
  ಆದರೆ( pIn1->u.i>0 ){
    pIn1->u.i -= pOp->p3;
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: OffsetLimit P1 P2 P3 * *
** Synopsis: if r[P1]>0 then r[P2]=r[P1]+max(0,r[P3]) else r[P2]=(-1)
**
** This opcode performs a commonly used computation associated with
** LIMIT and OFFSET processing.  r[P1] holds the limit counter.  r[P3]
** holds the offset counter.  The opcode computes the combined value
** of the LIMIT and OFFSET and stores that value in r[P2].  The r[P2]
** value computed is the total number of rows that will need to be
** visited in order to complete the query.
**
** If r[P3] is zero or negative, that means there is no OFFSET
** and r[P2] is set to be the value of the LIMIT, r[P1].
**
** if r[P1] is zero or negative, that means there is no LIMIT
** and r[P2] is set to -1.
**
** Otherwise, r[P2] is set to the sum of r[P1] and r[P3].
*/
ಸ್ಥಿತಿ OP_ಸ್ಥಾನಾಂತರ_ಎಲ್ಲೆ: {    /* in1, out2, in3 */
  i64 x;
  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  ಪ್ರತಿಜ್ಞಾ( pIn1->flags & MEM_Int );
  ಪ್ರತಿಜ್ಞಾ( pIn3->flags & MEM_Int );
  x = pIn1->u.i;
  ಆದರೆ( x<=0 || sqlite3_ಕೂಡಿಸು_Int64(&x, pIn3->u.i>0?pIn3->u.i:0) ){
    /* If the LIMIT is less than or equal to zero, loop forever.  This
    ** is documented.  But also, if the LIMIT+OFFSET exceeds 2^63 then
    ** also loop forever.  This is undocumented.  In fact, one could argue
    ** that the loop should terminate.  But assuming 1 billion iterations
    ** per second (far exceeding the capabilities of any current hardware)
    ** it would take nearly 300 years to actually reach the limit.  So
    ** looping forever is a reasonable approximation. */
    p_ಹೊರಹೋಗು->u.i = -1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಹೊರಹೋಗು->u.i = x;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: IfNotZero P1 P2 * * *
** Synopsis: if r[P1]!=0 then r[P1]--, goto P2
**
** Register P1 must contain an integer.  If the content of register P1 is
** initially greater than zero, then decrement the value in register P1.
** If it is non-zero (negative or positive) and then also jump to P2. 
** If register P1 is initially zero, leave it unchanged and fall through.
*/
ಸ್ಥಿತಿ OP_IfNotZero: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pIn1->flags&MEM_Int );
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(pIn1->u.i<0, 2);
  ಆದರೆ( pIn1->u.i ){
     ಆದರೆ( pIn1->u.i>0 ) pIn1->u.i--;
     ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: DecrJumpZero P1 P2 * * *
** Synopsis: if (--r[P1])==0 goto P2
**
** Register P1 must hold an integer.  Decrement the value in P1
** and jump to P2 if the new value is exactly zero.
*/
ಸ್ಥಿತಿ OP_DecrJumpZero: {      /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pIn1->flags&MEM_Int );
  ಆದರೆ( pIn1->u.i>ಅತಿಚಿಕ್ಕ_INT64 ) pIn1->u.i--;
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(pIn1->u.i==0, 2);
  ಆದರೆ( pIn1->u.i==0 ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
}


/* Opcode: AggStep * P2 P3 P4 P5
** Synopsis: accum=r[P3] step(r[P2@P5])
**
** Execute the xStep function for an aggregate.
** The function has P5 arguments.  P4 is a pointer to the
** FuncDef structure that specifies the function.  Register P3 is the
** accumulator.
**
** The P5 arguments are taken from register P2 and its
** successors.
*/
/* Opcode: AggInverse * P2 P3 P4 P5
** Synopsis: accum=r[P3] inverse(r[P2@P5])
**
** Execute the xInverse function for an aggregate.
** The function has P5 arguments.  P4 is a pointer to the
** FuncDef structure that specifies the function.  Register P3 is the
** accumulator.
**
** The P5 arguments are taken from register P2 and its
** successors.
*/
/* Opcode: AggStep1 P1 P2 P3 P4 P5
** Synopsis: accum=r[P3] step(r[P2@P5])
**
** Execute the xStep (if P1==0) or xInverse (if P1!=0) function for an
** aggregate.  The function has P5 arguments.  P4 is a pointer to the
** FuncDef structure that specifies the function.  Register P3 is the
** accumulator.
**
** The P5 arguments are taken from register P2 and its
** successors.
**
** This opcode is initially coded as OP_AggStep0.  On first evaluation,
** the FuncDef stored in P4 is converted into an sqlite3_context and
** the opcode is changed.  In this way, the initialization of the
** sqlite3_context only happens once, instead of on each call to the
** step function.
*/
ಸ್ಥಿತಿ OP_AggInverse:
ಸ್ಥಿತಿ OP_Agg_ಹೆಜ್ಜೆ: {
  ಪೂರ್ಣಾಂಕ n;
  sqlite3_context *pCtx;
  u64 nAlloc;

  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_FUNCDEF );
  n = pOp->p5;
  ಪ್ರತಿಜ್ಞಾ( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  ಪ್ರತಿಜ್ಞಾ( n==0 || (pOp->p2>0 && pOp->p2+n<=(p->nMem+1 - p->nCursor)+1) );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3<pOp->p2 || pOp->p3>=pOp->p2+n );

  /* Allocate space for (a) the context object and (n-1) extra pointers
  ** to append to the sqlite3_context.argv[1] array, and (b) a memory
  ** cell in which to store the accumulation. Be careful that the memory
  ** cell is 8-byte aligned, even on platforms where a pointer is 32-bits.
  **
  ** Note: We could avoid this by using a regular memory cell from aMem[] for 
  ** the accumulator, instead of allocating one here. */
  nAlloc = ROUND8P( SZ_CONTEXT(n) );
  pCtx = sqlite3DbMallocRawNN(db, nAlloc + ಗಾತ್ರ(Mem));
  ಆದರೆ( pCtx==0 ) ಹೋಗು no_mem;
  pCtx->p_ಹೊರಹೋಗು = (Mem*)((u8*)pCtx + nAlloc);
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pCtx->p_ಹೊರಹೋಗು) );

  sqlite3VdbeMemInit(pCtx->p_ಹೊರಹೋಗು, db, MEM_Null);
  pCtx->pMem = 0;
  pCtx->pFunc = pOp->p4.pFunc;
  pCtx->iOp = (ಪೂರ್ಣಾಂಕ)(pOp - aOp);
  pCtx->pVdbe = p;
  pCtx->skipFlag = 0;
  pCtx->isError = 0;
  pCtx->enc = encoding;
  pCtx->argc = n;
  pOp->p4type = P4_FUNCCTX;
  pOp->p4.pCtx = pCtx;

  /* OP_AggInverse must have P1==1 and OP_AggStep must have P1==0 */
  ಪ್ರತಿಜ್ಞಾ( pOp->p1==(pOp->opcode==OP_AggInverse) );

  pOp->opcode = OP_AggStep1;
  /* Fall through into OP_AggStep */
  /* no break */ deliberate_fall_through
}
ಸ್ಥಿತಿ OP_AggStep1: {
  ಪೂರ್ಣಾಂಕ i;
  sqlite3_context *pCtx;
  Mem *pMem;

  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_FUNCCTX );
  pCtx = pOp->p4.pCtx;
  pMem = &aMem[pOp->p3];

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( pOp->p1 ){
    /* This is an OP_AggInverse call.  Verify that xStep has always
    ** been called at least once prior to any xInverse call. */
    ಪ್ರತಿಜ್ಞಾ( pMem->u_ತಾತ್ಕಾಲಿಕ==0x1122e0e3 );
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* This is an OP_AggStep call.  Mark it as such. */
    pMem->u_ತಾತ್ಕಾಲಿಕ = 0x1122e0e3;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If this function is inside of a trigger, the register array in aMem[]
  ** might change from one evaluation to the next.  The next block of code
  ** checks to see if the register array has changed, and if so it
  ** reinitializes the relevant parts of the sqlite3_context object */
  ಆದರೆ( pCtx->pMem != pMem ){
    pCtx->pMem = pMem;
    ಗಾಗಿ(i=pCtx->argc-1; i>=0; i--) pCtx->argv[i] = &aMem[pOp->p2+i];
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಗಾಗಿ(i=0; i<pCtx->argc; i++){
    ಪ್ರತಿಜ್ಞಾ( memIsValid(pCtx->argv[i]) );
    ರಿಜಿಸ್ಟರ್_TRACE(pOp->p2+i, pCtx->argv[i]);
  }
#ಆದರೆ_ಅಂತ್ಯ

  pMem->n++;
  ಪ್ರತಿಜ್ಞಾ( pCtx->p_ಹೊರಹೋಗು->flags==MEM_Null );
  ಪ್ರತಿಜ್ಞಾ( pCtx->isError==0 );
  ಪ್ರತಿಜ್ಞಾ( pCtx->skipFlag==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಆದರೆ( pOp->p1 ){
    (pCtx->pFunc->xInverse)(pCtx,pCtx->argc,pCtx->argv);
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  (pCtx->pFunc->xSFunc)(pCtx,pCtx->argc,pCtx->argv); /* IMP: R-24505-23230 */

  ಆದರೆ( pCtx->isError ){
    ಆದರೆ( pCtx->isError>0 ){
      sqlite3VdbeError(p, "%s", sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pCtx->p_ಹೊರಹೋಗು));
      rc = pCtx->isError;
    }
    ಆದರೆ( pCtx->skipFlag ){
      ಪ್ರತಿಜ್ಞಾ( pOp[-1].opcode==OP_CollSeq );
      i = pOp[-1].p1;
      ಆದರೆ( i ) sqlite3_Vdbe_Mem_ಗಣ_Int64(&aMem[i], 1);
      pCtx->skipFlag = 0;
    }
    sqlite3VdbeMemRelease(pCtx->p_ಹೊರಹೋಗು);
    pCtx->p_ಹೊರಹೋಗು->flags = MEM_Null;
    pCtx->isError = 0;
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  ಪ್ರತಿಜ್ಞಾ( pCtx->p_ಹೊರಹೋಗು->flags==MEM_Null );
  ಪ್ರತಿಜ್ಞಾ( pCtx->skipFlag==0 );
  ನಿಲ್ಲಿಸು;
}

/* Opcode: AggFinal P1 P2 * P4 *
** Synopsis: accum=r[P1] N=P2
**
** P1 is the memory location that is the accumulator for an aggregate
** or window function.  Execute the finalizer function
** for an aggregate and store the result in P1.
**
** P2 is the number of arguments that the step function takes and
** P4 is a pointer to the FuncDef for this function.  The P2
** argument is not used by this opcode.  It is only there to disambiguate
** functions that can take varying numbers of arguments.  The
** P4 argument is only needed for the case where
** the step function was not previously called.
*/
/* Opcode: AggValue * P2 P3 P4 *
** Synopsis: r[P3]=value N=P2
**
** Invoke the xValue() function and store the result in register P3.
**
** P2 is the number of arguments that the step function takes and
** P4 is a pointer to the FuncDef for this function.  The P2
** argument is not used by this opcode.  It is only there to disambiguate
** functions that can take varying numbers of arguments.  The
** P4 argument is only needed for the case where
** the step function was not previously called.
*/
ಸ್ಥಿತಿ OP_Agg_ಮೌಲ್ಯ:
ಸ್ಥಿತಿ OP_AggFinal: {
  Mem *pMem;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3==0 || pOp->opcode==OP_Agg_ಮೌಲ್ಯ );
  pMem = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( (pMem->flags & ~(MEM_Null|MEM_Agg))==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಆದರೆ( pOp->p3 ){
    mem_About_ಗಮ್ಯ_Change(p, &aMem[pOp->p3]);
    rc = sqlite3_Vdbe_Mem_Agg_ಮೌಲ್ಯ(pMem, &aMem[pOp->p3], pOp->p4.pFunc);
    pMem = &aMem[pOp->p3];
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  {
    rc = sqlite3VdbeMemFinalize(pMem, pOp->p4.pFunc);
  }
 
  ಆದರೆ( rc ){
    sqlite3VdbeError(p, "%s", sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pMem));
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  sqlite3VdbeChangeEncoding(pMem, encoding);
  UPDATE_MAX_BLOBSIZE(pMem);
  ರಿಜಿಸ್ಟರ್_TRACE((ಪೂರ್ಣಾಂಕ)(pMem-aMem), pMem);
  ನಿಲ್ಲಿಸು;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
/* Opcode: Checkpoint P1 P2 P3 * *
**
** Checkpoint database P1. This is a no-op if P1 is not currently in
** WAL mode. Parameter P2 is one of SQLITE_CHECKPOINT_PASSIVE, FULL,
** RESTART, or TRUNCATE.  Write 1 or 0 into mem[P3] if the checkpoint returns
** SQLITE_BUSY or not, respectively.  Write the number of pages in the
** WAL after the checkpoint into mem[P3+1] and the number of pages
** in the WAL that have been checkpointed after the checkpoint
** completes into mem[P3+2].  However on an error, mem[P3+1] and
** mem[P3+2] are initialized to -1.
*/
ಸ್ಥಿತಿ OP_Checkpoint: {
  ಪೂರ್ಣಾಂಕ i;                          /* Loop counter */
  ಪೂರ್ಣಾಂಕ aRes[3];                    /* Results */
  Mem *pMem;                      /* Write results here */

  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  aRes[0] = 0;
  aRes[1] = aRes[2] = -1;
  ಪ್ರತಿಜ್ಞಾ( pOp->p2==SQLITE_CHECKPOINT_PASSIVE
       || pOp->p2==SQLITE_CHECKPOINT_ತುಂಬಿದ
       || pOp->p2==SQLITE_CHECKPOINT_RESTART
       || pOp->p2==SQLITE_CHECKPOINT_TRUNCATE
       || pOp->p2==SQLITE_CHECKPOINT_NOOP
  );
  rc = sqlite3Checkpoint(db, pOp->p1, pOp->p2, &aRes[1], &aRes[2]);
  ಆದರೆ( rc ){
    ಆದರೆ( rc!=SQLITE_BUSY ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    rc = SQLITE_OK;
    aRes[0] = 1;
  }
  ಗಾಗಿ(i=0, pMem = &aMem[pOp->p3]; i<3; i++, pMem++){
    sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem, (i64)aRes[i]);
  }   
  ನಿಲ್ಲಿಸು;
}; 
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_PRAGMA
/* Opcode: JournalMode P1 P2 P3 * *
**
** Change the journal mode of database P1 to P3. P3 must be one of the
** PAGER_JOURNALMODE_XXX values. If changing between the various rollback
** modes (delete, truncate, persist, off and memory), this is a simple
** operation. No IO is required.
**
** If changing into or out of WAL mode the procedure is more complicated.
**
** Write a string containing the final journal-mode to register P2.
*/
ಸ್ಥಿತಿ OP_Journal_ಬಹುಲಕ: {    /* out2 */
  Btree *pBt;                     /* Btree to change journal mode of */
  Pager *pPager;                  /* Pager associated with pBt */
  ಪೂರ್ಣಾಂಕ eNew;                       /* New journal mode */
  ಪೂರ್ಣಾಂಕ eOld;                       /* The old journal mode */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename;          /* Name of database file for pPager */
#ಆದರೆ_ಅಂತ್ಯ

  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  eNew = pOp->p3;
  ಪ್ರತಿಜ್ಞಾ( eNew==PAGER_JOURNALMODE_DELETE
       || eNew==PAGER_JOURNALMODE_TRUNCATE
       || eNew==PAGER_JOURNALMODE_PERSIST
       || eNew==PAGER_JOURNALMODE_OFF
       || eNew==PAGER_JOURNALMODE_ಸ್ಮರಣೆ
       || eNew==PAGER_JOURNALMODE_WAL
       || eNew==PAGER_JOURNALMODE_QUERY
  );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );

  pBt = db->aDb[pOp->p1].pBt;
  pPager = sqlite3BtreePager(pBt);
  eOld = sqlite3_Pager_Get_Journal_ಬಹುಲಕ(pPager);
  ಆದರೆ( eNew==PAGER_JOURNALMODE_QUERY ) eNew = eOld;
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(pBt) );
  ಆದರೆ( !sqlite3_Pager_Ok_ಗಮ್ಯ_Change_Journal_ಬಹುಲಕ(pPager) ) eNew = eOld;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  zFilename = sqlite3PagerFilename(pPager, 1);

  /* Do not allow a transition to journal_mode=WAL for a database
  ** in temporary storage or if the VFS does not support shared memory
  */
  ಆದರೆ( eNew==PAGER_JOURNALMODE_WAL
   && (sqlite3Strlen30(zFilename)==0           /* Temp file */
       || !sqlite3PagerWalSupported(pPager))   /* No shared-memory support */
  ){
    eNew = eOld;
  }

  ಆದರೆ( (eNew!=eOld)
   && (eOld==PAGER_JOURNALMODE_WAL || eNew==PAGER_JOURNALMODE_WAL)
  ){
    ಆದರೆ( !db->autoCommit || db->nVdbeRead>1 ){
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p,
          "cannot change %s wal mode from within a transaction",
          (eNew==PAGER_JOURNALMODE_WAL ? "into" : "out of")
      );
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }ಇಲ್ಲದಿದ್ದರೆ{

      ಆದರೆ( eOld==PAGER_JOURNALMODE_WAL ){
        /* If leaving WAL mode, close the log file. If successful, the call
        ** to PagerCloseWal() checkpoints and deletes the write-ahead-log
        ** file. An EXCLUSIVE lock may still be held on the database file
        ** after a successful return.
        */
        rc = sqlite3PagerCloseWal(pPager, db);
        ಆದರೆ( rc==SQLITE_OK ){
          sqlite3_Pager_ಗಣ_Journal_ಬಹುಲಕ(pPager, eNew);
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( eOld==PAGER_JOURNALMODE_ಸ್ಮರಣೆ ){
        /* Cannot transition directly from MEMORY to WAL.  Use mode OFF
        ** as an intermediate */
        sqlite3_Pager_ಗಣ_Journal_ಬಹುಲಕ(pPager, PAGER_JOURNALMODE_OFF);
      }
 
      /* Open a transaction on the database file. Regardless of the journal
      ** mode, this transaction always uses a rollback journal.
      */
      ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE );
      ಆದರೆ( rc==SQLITE_OK ){
        rc = sqlite3_Btree_ಗಣ_ಆವೃತ್ತಿ(pBt, (eNew==PAGER_JOURNALMODE_WAL ? 2 : 1));
      }
    }
  }
#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_OMIT_WAL */

  ಆದರೆ( rc ) eNew = eOld;
  eNew = sqlite3_Pager_ಗಣ_Journal_ಬಹುಲಕ(pPager, eNew);

  p_ಹೊರಹೋಗು->flags = MEM_Str|MEM_Static|MEM_ಪದ;
  p_ಹೊರಹೋಗು->z = (ಅಕ್ಷರ *)sqlite3JournalModename(eNew);
  p_ಹೊರಹೋಗು->n = sqlite3Strlen30(p_ಹೊರಹೋಗು->z);
  p_ಹೊರಹೋಗು->enc = SQLITE_UTF8;
  sqlite3VdbeChangeEncoding(p_ಹೊರಹೋಗು, encoding);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
};
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_PRAGMA */

#ಆದರೆ !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)
/* Opcode: Vacuum P1 P2 * * *
**
** Vacuum the entire database P1.  P1 is 0 for "main", and 2 or more
** for an attached database.  The "temp" database may not be vacuumed.
**
** If P2 is not zero, then it is a register holding a string which is
** the file into which the result of vacuum should be written.  When
** P2 is zero, the vacuum overwrites the original database.
*/
ಸ್ಥಿತಿ OP_Vacuum: {
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  rc = sqlite3_ಸತತ_Vacuum(&p->z_Err_ಸಂದೇಶ, db, pOp->p1,
                        pOp->p2 ? &aMem[pOp->p2] : 0);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_AUTOVACUUM)
/* Opcode: IncrVacuum P1 P2 * * *
**
** Perform a single step of the incremental vacuum procedure on
** the P1 database. If the vacuum has finished, jump to instruction
** P2. Otherwise, fall through to the next instruction.
*/
ಸ್ಥಿತಿ OP_IncrVacuum: {        /* jump */
  Btree *pBt;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, pOp->p1) );
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  pBt = db->aDb[pOp->p1].pBt;
  rc = sqlite3BtreeIncrVacuum(pBt);
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(rc==SQLITE_ಮುಗಿದ,2);
  ಆದರೆ( rc ){
    ಆದರೆ( rc!=SQLITE_ಮುಗಿದ ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
    rc = SQLITE_OK;
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Opcode: Expire P1 P2 * * *
**
** Cause precompiled statements to expire.  When an expired statement
** is executed using sqlite3_step() it will either automatically
** reprepare itself (if it was originally created using sqlite3_prepare_v2())
** or it will fail with SQLITE_SCHEMA.
**
** If P1 is 0, then all SQL statements become expired. If P1 is non-zero,
** then only the currently executing statement is expired.
**
** If P2 is 0, then SQL statements are expired immediately.  If P2 is 1,
** then running SQL statements are allowed to continue to run to completion.
** The P2==1 case occurs when a CREATE INDEX or similar schema change happens
** that might help the statement run faster but which does not affect the
** correctness of operation.
*/
ಸ್ಥಿತಿ OP_Expire: {
  ಪ್ರತಿಜ್ಞಾ( pOp->p2==0 || pOp->p2==1 );
  ಆದರೆ( !pOp->p1 ){
    sqlite3ExpirePreparedStatements(db, pOp->p2);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->expired = pOp->p2+1;
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: CursorLock P1 * * * *
**
** Lock the btree to which cursor P1 is pointing so that the btree cannot be
** written by an other cursor.
*/
ಸ್ಥಿತಿ OP_Cursor_ಬೀಗ: {
  VdbeCursor *pC;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  sqlite3BtreeCursorPin(pC->uc.pCursor);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: CursorUnlock P1 * * * *
**
** Unlock the btree to which cursor P1 is pointing so that it can be
** written by other cursors.
*/
ಸ್ಥಿತಿ OP_CursorUnlock: {
  VdbeCursor *pC;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pC!=0 );
  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  sqlite3BtreeCursorUnpin(pC->uc.pCursor);
  ನಿಲ್ಲಿಸು;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
/* Opcode: TableLock P1 P2 P3 P4 *
** Synopsis: iDb=P1 root=P2 write=P3
**
** Obtain a lock on a particular table. This instruction is only used when
** the shared-cache feature is enabled.
**
** P1 is the index of the database in sqlite3.aDb[] of the database
** on which the lock is acquired.  A readlock is obtained if P3==0 or
** a write lock if P3==1.
**
** P2 contains the root-page of the table to lock.
**
** P4 contains a pointer to the name of the table being locked. This is only
** used to generate an error message if the lock cannot be obtained.
*/
ಸ್ಥಿತಿ OP_ಕೋಷ್ಟಕ_ಬೀಗ: {
  u8 is_Write_ಬೀಗ = (u8)pOp->p3;
  ಆದರೆ( is_Write_ಬೀಗ || 0==(db->flags&SQLITE_ReadUncommit) ){
    ಪೂರ್ಣಾಂಕ p1 = pOp->p1;
    ಪ್ರತಿಜ್ಞಾ( p1>=0 && p1<db->nDb );
    ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, p1) );
    ಪ್ರತಿಜ್ಞಾ( is_Write_ಬೀಗ==0 || is_Write_ಬೀಗ==1 );
    rc = sqlite3_Btree_ಬೀಗ_ಕೋಷ್ಟಕ(db->aDb[p1].pBt, pOp->p2, is_Write_ಬೀಗ);
    ಆದರೆ( rc ){
      ಆದರೆ( (rc&0xFF)==SQLITE_LOCKED ){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = pOp->p4.z;
        sqlite3VdbeError(p, "database table is locked: %s", z);
      }
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SHARED_CACHE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VBegin * * * P4 *
**
** P4 may be a pointer to an sqlite3_vtab structure. If so, call the
** xBegin method for that table.
**
** Also, whether or not P4 is set, check that this is not being called from
** within a callback to a virtual table xSync() method. If it is, the error
** code will be set to SQLITE_LOCKED.
*/
ಸ್ಥಿತಿ OP_VBegin: {
  VTable *pVTab;
  pVTab = pOp->p4.pVtab;
  rc = sqlite3VtabBegin(db, pVTab);
  ಆದರೆ( pVTab ) sqlite3VtabImportErrmsg(p, pVTab->pVtab);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VCreate P1 P2 * * *
**
** P2 is a register that holds the name of a virtual table in database
** P1. Call the xCreate method for that table.
*/
ಸ್ಥಿತಿ OP_VCreate: {
  Mem sMem;          /* For storing the record being decoded */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab;  /* Name of the virtual table */

  memset(&sMem, 0, ಗಾತ್ರ(sMem));
  sMem.db = db;
  /* Because P2 is always a static string, it is impossible for the
  ** sqlite3VdbeMemCopy() to fail */
  ಪ್ರತಿಜ್ಞಾ( (aMem[pOp->p2].flags & MEM_Str)!=0 );
  ಪ್ರತಿಜ್ಞಾ( (aMem[pOp->p2].flags & MEM_Static)!=0 );
  rc = sqlite3VdbeMemCopy(&sMem, &aMem[pOp->p2]);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  zTab = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(&sMem);
  ಪ್ರತಿಜ್ಞಾ( zTab || db->mallocFailed );
  ಆದರೆ( zTab ){
    rc = sqlite3VtabCallCreate(db, pOp->p1, zTab, &p->z_Err_ಸಂದೇಶ);
  }
  sqlite3VdbeMemRelease(&sMem);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VDestroy P1 * * P4 *
**
** P4 is the name of a virtual table in database P1.  Call the xDestroy method
** of that table.
*/
ಸ್ಥಿತಿ OP_VDestroy: {
  db->nVDestroy++;
  rc = sqlite3VtabCallDestroy(db, pOp->p1, pOp->p4.z);
  db->nVDestroy--;
  ಪ್ರತಿಜ್ಞಾ( p->errorAction==OE_Abort && p->ಬಳಸುತ್ತದೆ_Stmt_Journal );
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VOpen P1 * * P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** P1 is a cursor number.  This opcode opens a cursor to the virtual
** table and stores that cursor in P1.
*/
ಸ್ಥಿತಿ OP_VOpen: {             /* ncycle */
  VdbeCursor *pCur;
  sqlite3_vtab_cursor *pVCur;
  sqlite3_vtab *pVtab;
  ಸ್ಥಿರಾಂಕ sqlite3_module *pModule;

  ಪ್ರತಿಜ್ಞಾ( p->bIsReader );
  pCur = p->apCsr[pOp->p1];
  ಆದರೆ( pCur!=0
   && ALWAYS( pCur->eCurType==CURTYPE_VTAB )
   && ALWAYS( pCur->uc.pVCur->pVtab==pOp->p4.pVtab->pVtab )
  ){
    /* This opcode is a no-op if the cursor is already open */
    ನಿಲ್ಲಿಸು;
  }
  pVCur = 0;
  pVtab = pOp->p4.pVtab->pVtab;
  ಆದರೆ( pVtab==0 || NEVER(pVtab->pModule==0) ){
    rc = SQLITE_LOCKED;
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  pModule = pVtab->pModule;
  rc = pModule->xOpen(pVtab, &pVCur);
  sqlite3VtabImportErrmsg(p, pVtab);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;

  /* Initialize sqlite3_vtab_cursor base class */
  pVCur->pVtab = pVtab;

  /* Initialize vdbe cursor object */
  pCur = allocateCursor(p, pOp->p1, 0, CURTYPE_VTAB);
  ಆದರೆ( pCur ){
    pCur->uc.pVCur = pVCur;
    pVtab->nRef++;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    pModule->xClose(pVCur);
    ಹೋಗು no_mem;
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VCheck P1 P2 P3 P4 *
**
** P4 is a pointer to a Table object that is a virtual table in schema P1
** that supports the xIntegrity() method.  This opcode runs the xIntegrity()
** method for that virtual table, using P3 as the integer argument.  If
** an error is reported back, the table name is prepended to the error
** message and that message is stored in P2.  If no errors are seen,
** register P2 is set to NULL.
*/
ಸ್ಥಿತಿ OP_VCheck: {             /* out2 */
  ಕೋಷ್ಟಕ *pTab;
  sqlite3_vtab *pVtab;
  ಸ್ಥಿರಾಂಕ sqlite3_module *pModule;
  ಅಕ್ಷರ *zErr = 0;

  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);  /* Innocent until proven guilty */
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_TABLEREF );
  pTab = pOp->p4.pTab;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTab->nTabRef>0 );
  ಪ್ರತಿಜ್ಞಾ( IsVirtual(pTab) );
  ಆದರೆ( pTab->u.vtab.p==0 ) ನಿಲ್ಲಿಸು;
  pVtab = pTab->u.vtab.p->pVtab;
  ಪ್ರತಿಜ್ಞಾ( pVtab!=0 );
  pModule = pVtab->pModule;
  ಪ್ರತಿಜ್ಞಾ( pModule!=0 );
  ಪ್ರತಿಜ್ಞಾ( pModule->i_ಆವೃತ್ತಿ>=4 );
  ಪ್ರತಿಜ್ಞಾ( pModule->xIntegrity!=0 );
  sqlite3_Vtab_ಬೀಗ(pTab->u.vtab.p);
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<db->nDb );
  rc = pModule->xIntegrity(pVtab, db->aDb[pOp->p1].zDbSName, pTab->zName,
                           pOp->p3, &zErr);
  sqlite3VtabUnlock(pTab->u.vtab.p);
  ಆದರೆ( rc ){
    sqlite3_free(zErr);
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  ಆದರೆ( zErr ){
    sqlite3_Vdbe_Mem_ಗಣ_Str(p_ಹೊರಹೋಗು, zErr, -1, SQLITE_UTF8, sqlite3_free);
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VInitIn P1 P2 P3 * *
** Synopsis: r[P2]=ValueList(P1,P3)
**
** Set register P2 to be a pointer to a ValueList object for cursor P1
** with cache register P3 and output register P3+1.  This ValueList object
** can be used as the first argument to sqlite3_vtab_in_first() and
** sqlite3_vtab_in_next() to extract all of the values stored in the P1
** cursor.  Register P3 is used to hold the values returned by
** sqlite3_vtab_in_first() and sqlite3_vtab_in_next().
*/
ಸ್ಥಿತಿ OP_VInit_ಒಳ: {        /* out2, ncycle */
  VdbeCursor *pC;         /* The cursor containing the RHS values */
  ಮೌಲ್ಯ_List *pRhs;        /* New ValueList object to put in reg[P2] */

  pC = p->apCsr[pOp->p1];
  pRhs = sqlite3_malloc64( ಗಾತ್ರ(*pRhs) );
  ಆದರೆ( pRhs==0 ) ಹೋಗು no_mem;
  pRhs->pCsr = pC->uc.pCursor;
  pRhs->p_ಹೊರಹೋಗು = &aMem[pOp->p3];
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->flags = MEM_Null;
  sqlite3_Vdbe_Mem_ಗಣ_ಸೂಚಕ(p_ಹೊರಹೋಗು, pRhs, "ValueList", sqlite3_Vdbe_ಮೌಲ್ಯ_List_Free);
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VFilter P1 P2 P3 P4 *
** Synopsis: iplan=r[P3] zplan='P4'
**
** P1 is a cursor opened using VOpen.  P2 is an address to jump to if
** the filtered result set is empty.
**
** P4 is either NULL or a string that was generated by the xBestIndex
** method of the module.  The interpretation of the P4 string is left
** to the module implementation.
**
** This opcode invokes the xFilter method on the virtual table specified
** by P1.  The integer query plan parameter to xFilter is stored in register
** P3. Register P3+1 stores the argc parameter to be passed to the
** xFilter method. Registers P3+2..P3+1+argc are the argc
** additional parameters which are passed to
** xFilter as argv. Register P3+2 becomes argv[0] when passed to xFilter.
**
** A jump is made to P2 if the result set after filtering would be empty.
*/
ಸ್ಥಿತಿ OP_VFilter: {   /* jump, ncycle */
  ಪೂರ್ಣಾಂಕ nArg;
  ಪೂರ್ಣಾಂಕ iQuery;
  ಸ್ಥಿರಾಂಕ sqlite3_module *pModule;
  Mem *pQuery;
  Mem *pArgc;
  sqlite3_vtab_cursor *pVCur;
  sqlite3_vtab *pVtab;
  VdbeCursor *pCur;
  ಪೂರ್ಣಾಂಕ res;
  ಪೂರ್ಣಾಂಕ i;
  Mem **apArg;

  pQuery = &aMem[pOp->p3];
  pArgc = &pQuery[1];
  pCur = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( memIsValid(pQuery) );
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3, pQuery);
  ಪ್ರತಿಜ್ಞಾ( pCur!=0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->eCurType==CURTYPE_VTAB );
  pVCur = pCur->uc.pVCur;
  pVtab = pVCur->pVtab;
  pModule = pVtab->pModule;

  /* Grab the index number and argc parameters */
  ಪ್ರತಿಜ್ಞಾ( (pQuery->flags&MEM_Int)!=0 && pArgc->flags==MEM_Int );
  nArg = (ಪೂರ್ಣಾಂಕ)pArgc->u.i;
  iQuery = (ಪೂರ್ಣಾಂಕ)pQuery->u.i;

  /* Invoke the xFilter method */
  apArg = p->apArg;
  ಪ್ರತಿಜ್ಞಾ( nArg<=p->napArg );
  ಗಾಗಿ(i = 0; i<nArg; i++){
    apArg[i] = &pArgc[i+1];
  }
  rc = pModule->xFilter(pVCur, iQuery, pOp->p4.z, nArg, apArg);
  sqlite3VtabImportErrmsg(p, pVtab);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  res = pModule->xEof(pVCur);
  pCur->null_ಅಡ್ಡಸಾಲು = 0;
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(res!=0,2);
  ಆದರೆ( res ) ಹೋಗು jump_ಗಮ್ಯ_p2;
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VColumn P1 P2 P3 * P5
** Synopsis: r[P3]=vcolumn(P2)
**
** Store in register P3 the value of the P2-th column of
** the current row of the virtual-table of cursor P1.
**
** If the VColumn opcode is being used to fetch the value of
** an unchanging column during an UPDATE operation, then the P5
** value is OPFLAG_NOCHNG.  This will cause the sqlite3_vtab_nochange()
** function to return true inside the xColumn method of the virtual
** table implementation.  The P5 column might also contain other
** bits (OPFLAG_LENGTHARG or OPFLAG_TYPEOFARG) but those bits are
** unused by OP_VColumn.
*/
ಸ್ಥಿತಿ OP_VColumn: {           /* ncycle */
  sqlite3_vtab *pVtab;
  ಸ್ಥಿರಾಂಕ sqlite3_module *pModule;
  Mem *pDest;
  sqlite3_context sContext;
  FuncDef nullFunc;

  VdbeCursor *pCur = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pCur!=0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  pDest = &aMem[pOp->p3];
  mem_About_ಗಮ್ಯ_Change(p, pDest);
  ಆದರೆ( pCur->null_ಅಡ್ಡಸಾಲು ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(pDest);
    ನಿಲ್ಲಿಸು;
  }
  ಪ್ರತಿಜ್ಞಾ( pCur->eCurType==CURTYPE_VTAB );
  pVtab = pCur->uc.pVCur->pVtab;
  pModule = pVtab->pModule;
  ಪ್ರತಿಜ್ಞಾ( pModule->x_ಸ್ತಂಭ );
  memset(&sContext, 0, ಗಾತ್ರ(sContext));
  sContext.p_ಹೊರಹೋಗು = pDest;
  sContext.enc = encoding;
  nullFunc.p_User_ದತ್ತಾಂಶ = 0;
  nullFunc.funcFlags = SQLITE_ಫಲಿತಾಂಶ_SUBTYPE;
  sContext.pFunc = &nullFunc;
  ಪ್ರತಿಜ್ಞಾ( pOp->p5==OPFLAG_NOCHNG || pOp->p5==0 );
  ಆದರೆ( pOp->p5 & OPFLAG_NOCHNG ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(pDest);
    pDest->flags = MEM_Null|MEM_Zero;
    pDest->u.nZero = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    Mem_ಗಣ_Type_Flag(pDest, MEM_Null);
  }
  rc = pModule->x_ಸ್ತಂಭ(pCur->uc.pVCur, &sContext, pOp->p2);
  sqlite3VtabImportErrmsg(p, pVtab);
  ಆದರೆ( sContext.isError>0 ){
    sqlite3VdbeError(p, "%s", sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pDest));
    rc = sContext.isError;
  }
  sqlite3VdbeChangeEncoding(pDest, encoding);
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3, pDest);
  UPDATE_MAX_BLOBSIZE(pDest);

  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VNext P1 P2 * * *
**
** Advance virtual table P1 to the next row in its result set and
** jump to instruction P2.  Or, if the virtual table has reached
** the end of its result set, then fall through to the next instruction.
*/
ಸ್ಥಿತಿ OP_VNext: {   /* jump, ncycle */
  sqlite3_vtab *pVtab;
  ಸ್ಥಿರಾಂಕ sqlite3_module *pModule;
  ಪೂರ್ಣಾಂಕ res;
  VdbeCursor *pCur;

  pCur = p->apCsr[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pCur!=0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->eCurType==CURTYPE_VTAB );
  ಆದರೆ( pCur->null_ಅಡ್ಡಸಾಲು ){
    ನಿಲ್ಲಿಸು;
  }
  pVtab = pCur->uc.pVCur->pVtab;
  pModule = pVtab->pModule;
  ಪ್ರತಿಜ್ಞಾ( pModule->x_ಮುಂದಿನ );

  /* Invoke the xNext() method of the module. There is no way for the
  ** underlying implementation to return an error if one occurs during
  ** xNext(). Instead, if an error occurs, true is returned (indicating that
  ** data is available) and the error code returned when xColumn or
  ** some other method is next invoked on the save virtual table cursor.
  */
  rc = pModule->x_ಮುಂದಿನ(pCur->uc.pVCur);
  sqlite3VtabImportErrmsg(p, pVtab);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  res = pModule->xEof(pCur->uc.pVCur);
  Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(!res,2);
  ಆದರೆ( !res ){
    /* If there is data, jump to P2 */
    ಹೋಗು jump_ಗಮ್ಯ_p2_and_check_for_ಅಡಚಣೆ;
  }
  ಹೋಗು check_for_ಅಡಚಣೆ;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VRename P1 * * P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** This opcode invokes the corresponding xRename method. The value
** in register P1 is passed as the zName argument to the xRename method.
*/
ಸ್ಥಿತಿ OP_VRename: {
  sqlite3_vtab *pVtab;
  Mem *pName;
  ಪೂರ್ಣಾಂಕ isLegacy;
 
  isLegacy = (db->flags & SQLITE_LegacyAlter);
  db->flags |= SQLITE_LegacyAlter;
  pVtab = pOp->p4.pVtab->pVtab;
  pName = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pVtab->pModule->xRename );
  ಪ್ರತಿಜ್ಞಾ( memIsValid(pName) );
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p1, pName);
  ಪ್ರತಿಜ್ಞಾ( pName->flags & MEM_Str );
  testcase( pName->enc==SQLITE_UTF8 );
  testcase( pName->enc==SQLITE_UTF16BE );
  testcase( pName->enc==SQLITE_UTF16LE );
  rc = sqlite3VdbeChangeEncoding(pName, SQLITE_UTF8);
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  rc = pVtab->pModule->xRename(pVtab, pName->z);
  ಆದರೆ( isLegacy==0 ) db->flags &= ~(u64)SQLITE_LegacyAlter;
  sqlite3VtabImportErrmsg(p, pVtab);
  p->expired = 0;
  ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VUpdate P1 P2 P3 P4 P5
** Synopsis: data=r[P3@P2]
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** This opcode invokes the corresponding xUpdate method. P2 values
** are contiguous memory cells starting at P3 to pass to the xUpdate
** invocation. The value in register (P3+P2-1) corresponds to the
** p2th element of the argv array passed to xUpdate.
**
** The xUpdate method will do a DELETE or an INSERT or both.
** The argv[0] element (which corresponds to memory cell P3)
** is the rowid of a row to delete.  If argv[0] is NULL then no
** deletion occurs.  The argv[1] element is the rowid of the new
** row.  This can be NULL to have the virtual table select the new
** rowid for itself.  The subsequent elements in the array are
** the values of columns in the new row.
**
** If P2==1 then no insert is performed.  argv[0] is the rowid of
** a row to delete.
**
** P1 is a boolean flag. If it is set to true and the xUpdate call
** is successful, then the value returned by sqlite3_last_insert_rowid()
** is set to the value of the rowid for the row just inserted.
**
** P5 is the error actions (OE_Replace, OE_Fail, OE_Ignore, etc) to
** apply in the case of a constraint failure on an insert or update.
*/
ಸ್ಥಿತಿ OP_VUpdate: {
  sqlite3_vtab *pVtab;
  ಸ್ಥಿರಾಂಕ sqlite3_module *pModule;
  ಪೂರ್ಣಾಂಕ nArg;
  ಪೂರ್ಣಾಂಕ i;
  sqlite_int64 rowid = 0;
  Mem **apArg;
  Mem *pX;

  ಪ್ರತಿಜ್ಞಾ( pOp->p2==1        || pOp->p5==OE_Fail   || pOp->p5==OE_Rollback
       || pOp->p5==OE_Abort || pOp->p5==OE_Ignore || pOp->p5==OE_Replace
  );
  ಪ್ರತಿಜ್ಞಾ( p->readOnly==0 );
  ಆದರೆ( db->mallocFailed ) ಹೋಗು no_mem;
  sqlite3VdbeIncrWriteCounter(p, 0);
  pVtab = pOp->p4.pVtab->pVtab;
  ಆದರೆ( pVtab==0 || NEVER(pVtab->pModule==0) ){
    rc = SQLITE_LOCKED;
    ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  pModule = pVtab->pModule;
  nArg = pOp->p2;
  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_VTAB );
  ಆದರೆ( ALWAYS(pModule->xUpdate) ){
    u8 vtabOnConflict = db->vtabOnConflict;
    apArg = p->apArg;
    pX = &aMem[pOp->p3];
    ಪ್ರತಿಜ್ಞಾ( nArg<=p->napArg );
    ಗಾಗಿ(i=0; i<nArg; i++){
      ಪ್ರತಿಜ್ಞಾ( memIsValid(pX) );
      mem_About_ಗಮ್ಯ_Change(p, pX);
      apArg[i] = pX;
      pX++;
    }
    db->vtabOnConflict = pOp->p5;
    rc = pModule->xUpdate(pVtab, nArg, apArg, &rowid);
    db->vtabOnConflict = vtabOnConflict;
    sqlite3VtabImportErrmsg(p, pVtab);
    ಆದರೆ( rc==SQLITE_OK && pOp->p1 ){
      ಪ್ರತಿಜ್ಞಾ( nArg>1 && apArg[0] && (apArg[0]->flags&MEM_Null) );
      db->ಕೊನೆಯ_Rowid = rowid;
    }
    ಆದರೆ( (rc&0xff)==SQLITE_CONSTRAINT && pOp->p4.pVtab->bConstraint ){
      ಆದರೆ( pOp->p5==OE_Ignore ){
        rc = SQLITE_OK;
      }ಇಲ್ಲದಿದ್ದರೆ{
        p->errorAction = ((pOp->p5==OE_Replace) ? OE_Abort : pOp->p5);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->nChange++;
    }
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ  SQLITE_OMIT_PAGER_PRAGMAS
/* Opcode: Pagecount P1 P2 * * *
**
** Write the current number of pages in database P1 to memory cell P2.
*/
ಸ್ಥಿತಿ OP_Pagecount: {            /* out2 */
  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  p_ಹೊರಹೋಗು->u.i = sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(db->aDb[pOp->p1].pBt);
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ  SQLITE_OMIT_PAGER_PRAGMAS
/* Opcode: MaxPgcnt P1 P2 P3 * *
**
** Try to set the maximum page count for database P1 to the value in P3.
** Do not let the maximum page count fall below the current page count and
** do not change the maximum page count value if P3==0.
**
** Store the maximum page count after the change in register P2.
*/
ಸ್ಥಿತಿ OP_MaxPgcnt: {            /* out2 */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ newMax;
  Btree *pBt;

  p_ಹೊರಹೋಗು = out2Prerelease(p, pOp);
  pBt = db->aDb[pOp->p1].pBt;
  newMax = 0;
  ಆದರೆ( pOp->p3 ){
    newMax = sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(pBt);
    ಆದರೆ( newMax < (ಚಿಹ್ನೆರಹಿತ)pOp->p3 ) newMax = (ಚಿಹ್ನೆರಹಿತ)pOp->p3;
  }
  p_ಹೊರಹೋಗು->u.i = sqlite3_Btree_Max_ಪುಟ_ಎಣಿಕೆ(pBt, newMax);
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Opcode: Function P1 P2 P3 P4 *
** Synopsis: r[P3]=func(r[P2@NP])
**
** Invoke a user function (P4 is a pointer to an sqlite3_context object that
** contains a pointer to the function to be run) with arguments taken
** from register P2 and successors.  The number of arguments is in
** the sqlite3_context object that P4 points to.
** The result of the function is stored
** in register P3.  Register P3 must not be one of the function inputs.
**
** P1 is a 32-bit bitmask indicating whether or not each argument to the
** function was determined to be constant at compile time. If the first
** argument was constant then bit 0 of P1 is set. This is used to determine
** whether meta data associated with a user function argument using the
** sqlite3_set_auxdata() API may be safely retained until the next
** invocation of this opcode.
**
** See also: AggStep, AggFinal, PureFunc
*/
/* Opcode: PureFunc P1 P2 P3 P4 *
** Synopsis: r[P3]=func(r[P2@NP])
**
** Invoke a user function (P4 is a pointer to an sqlite3_context object that
** contains a pointer to the function to be run) with arguments taken
** from register P2 and successors.  The number of arguments is in
** the sqlite3_context object that P4 points to.
** The result of the function is stored
** in register P3.  Register P3 must not be one of the function inputs.
**
** P1 is a 32-bit bitmask indicating whether or not each argument to the
** function was determined to be constant at compile time. If the first
** argument was constant then bit 0 of P1 is set. This is used to determine
** whether meta data associated with a user function argument using the
** sqlite3_set_auxdata() API may be safely retained until the next
** invocation of this opcode.
**
** This opcode works exactly like OP_Function.  The only difference is in
** its name.  This opcode is used in places where the function must be
** purely non-deterministic.  Some built-in date/time functions can be
** either deterministic of non-deterministic, depending on their arguments.
** When those function are used in a non-deterministic way, they will check
** to see if they were called using OP_PureFunc instead of OP_Function, and
** if they were, they throw an error.
**
** See also: AggStep, AggFinal, Function
*/
ಸ್ಥಿತಿ OP_ಶುದ್ಧ_Func:              /*ಗುಂಪು*/
ಸ್ಥಿತಿ OP_Function: {            /*ಗುಂಪು*/
  ಪೂರ್ಣಾಂಕ i;
  sqlite3_context *pCtx;

  ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_FUNCCTX );
  pCtx = pOp->p4.pCtx;

  /* If this function is inside of a trigger, the register array in aMem[]
  ** might change from one evaluation to the next.  The next block of code
  ** checks to see if the register array has changed, and if so it
  ** reinitializes the relevant parts of the sqlite3_context object */
  p_ಹೊರಹೋಗು = &aMem[pOp->p3];
  ಆದರೆ( pCtx->p_ಹೊರಹೋಗು != p_ಹೊರಹೋಗು ){
    pCtx->pVdbe = p;
    pCtx->p_ಹೊರಹೋಗು = p_ಹೊರಹೋಗು;
    pCtx->enc = encoding;
    ಗಾಗಿ(i=pCtx->argc-1; i>=0; i--) pCtx->argv[i] = &aMem[pOp->p2+i];
  }
  ಪ್ರತಿಜ್ಞಾ( pCtx->pVdbe==p );

  mem_About_ಗಮ್ಯ_Change(p, p_ಹೊರಹೋಗು);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಗಾಗಿ(i=0; i<pCtx->argc; i++){
    ಪ್ರತಿಜ್ಞಾ( memIsValid(pCtx->argv[i]) );
    ರಿಜಿಸ್ಟರ್_TRACE(pOp->p2+i, pCtx->argv[i]);
  }
#ಆದರೆ_ಅಂತ್ಯ
  Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Null);
  ಪ್ರತಿಜ್ಞಾ( pCtx->isError==0 );
  (*pCtx->pFunc->xSFunc)(pCtx, pCtx->argc, pCtx->argv);/* IMP: R-24505-23230 */

  /* If the function returned an error, throw an exception */
  ಆದರೆ( pCtx->isError ){
    ಆದರೆ( pCtx->isError>0 ){
      sqlite3VdbeError(p, "%s", sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p_ಹೊರಹೋಗು));
      rc = pCtx->isError;
    }
    sqlite3_Vdbe_Delete_Aux_ದತ್ತಾಂಶ(db, &p->p_Aux_ದತ್ತಾಂಶ, pCtx->iOp, pOp->p1);
    pCtx->isError = 0;
    ಆದರೆ( rc ) ಹೋಗು abort_due_ಗಮ್ಯ_error;
  }

  ಪ್ರತಿಜ್ಞಾ( (p_ಹೊರಹೋಗು->flags&MEM_Str)==0
       || p_ಹೊರಹೋಗು->enc==encoding
       || db->mallocFailed );
  ಪ್ರತಿಜ್ಞಾ( !sqlite3VdbeMemTooBig(p_ಹೊರಹೋಗು) );

  ರಿಜಿಸ್ಟರ್_TRACE(pOp->p3, p_ಹೊರಹೋಗು);
  UPDATE_MAX_BLOBSIZE(p_ಹೊರಹೋಗು);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: ClrSubtype P1 * * * *
** Synopsis:  r[P1].subtype = 0
**
** Clear the subtype from register P1.
*/
ಸ್ಥಿತಿ OP_ClrSubtype: {   /* in1 */
  pIn1 = &aMem[pOp->p1];
  pIn1->flags &= ~MEM_Subtype;
  ನಿಲ್ಲಿಸು;
}

/* Opcode: GetSubtype P1 P2 * * *
** Synopsis:  r[P2] = r[P1].subtype
**
** Extract the subtype value from register P1 and write that subtype
** into register P2.  If P1 has no subtype, then P1 gets a NULL.
*/
ಸ್ಥಿತಿ OP_GetSubtype: {   /* in1 out2 */
  pIn1 = &aMem[pOp->p1];
  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  ಆದರೆ( pIn1->flags & MEM_Subtype ){
    sqlite3_Vdbe_Mem_ಗಣ_Int64(p_ಹೊರಹೋಗು, pIn1->eSubtype);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Vdbe_Mem_ಗಣ_Null(p_ಹೊರಹೋಗು);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: SetSubtype P1 P2 * * *
** Synopsis:  r[P2].subtype = r[P1]
**
** Set the subtype value of register P2 to the integer from register P1.
** If P1 is NULL, clear the subtype from p2.
*/
ಸ್ಥಿತಿ OP_ಗಣ_Subtype: {   /* in1 out2 */
  pIn1 = &aMem[pOp->p1];
  p_ಹೊರಹೋಗು = &aMem[pOp->p2];
  ಆದರೆ( pIn1->flags & MEM_Null ){
    p_ಹೊರಹೋಗು->flags &= ~MEM_Subtype;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pIn1->flags & MEM_Int );
    p_ಹೊರಹೋಗು->flags |= MEM_Subtype;
    p_ಹೊರಹೋಗು->eSubtype = (u8)(pIn1->u.i & 0xff);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: FilterAdd P1 * P3 P4 *
** Synopsis: filter(P1) += key(P3@P4)
**
** Compute a hash on the P4 registers starting with r[P3] and
** add that hash to the bloom filter contained in r[P1].
*/
ಸ್ಥಿತಿ OP_Filter_ಕೂಡಿಸು: {
  u64 h;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( pIn1->flags & MEM_Blob );
  ಪ್ರತಿಜ್ಞಾ( pIn1->n>0 );
  h = filterHash(aMem, pOp);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
    ಪೂರ್ಣಾಂಕ ii;
    ಗಾಗಿ(ii=pOp->p3; ii<pOp->p3+pOp->p4.i; ii++){
      ರಿಜಿಸ್ಟರ್_Trace(ii, &aMem[ii]);
    }
    ಬರೆ("hash: %llu modulo %d -> %u\n", h, pIn1->n, (ಪೂರ್ಣಾಂಕ)(h%pIn1->n));
  }
#ಆದರೆ_ಅಂತ್ಯ
  h %= (pIn1->n*8);
  pIn1->z[h/8] |= 1<<(h&7);
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Filter P1 P2 P3 P4 *
** Synopsis: if key(P3@P4) not in filter(P1) goto P2
**
** Compute a hash on the key contained in the P4 registers starting
** with r[P3].  Check to see if that hash is found in the
** bloom filter hosted by register P1.  If it is not present then
** maybe jump to P2.  Otherwise fall through.
**
** False negatives are harmless.  It is always safe to fall through,
** even if the value is in the bloom filter.  A false negative causes
** more CPU cycles to be used, but it should still yield the correct
** answer.  However, an incorrect answer may well arise from a
** false positive - if the jump is taken when it should fall through.
*/
ಸ್ಥಿತಿ OP_Filter: {          /* jump */
  u64 h;

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  ಪ್ರತಿಜ್ಞಾ( (pIn1->flags & MEM_Blob)!=0 );
  ಪ್ರತಿಜ್ಞಾ( pIn1->n >= 1 );
  h = filterHash(aMem, pOp);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( db->flags&SQLITE_VdbeTrace ){
    ಪೂರ್ಣಾಂಕ ii;
    ಗಾಗಿ(ii=pOp->p3; ii<pOp->p3+pOp->p4.i; ii++){
      ರಿಜಿಸ್ಟರ್_Trace(ii, &aMem[ii]);
    }
    ಬರೆ("hash: %llu modulo %d -> %u\n", h, pIn1->n, (ಪೂರ್ಣಾಂಕ)(h%pIn1->n));
  }
#ಆದರೆ_ಅಂತ್ಯ
  h %= (pIn1->n*8);
  ಆದರೆ( (pIn1->z[h/8] & (1<<(h&7)))==0 ){
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(1, 2);
    p->aCounter[SQLITE_STMTSTATUS_FILTER_ದೊರಕಿತು]++;
    ಹೋಗು jump_ಗಮ್ಯ_p2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->aCounter[SQLITE_STMTSTATUS_FILTER_ತಪ್ಪಿದ]++;
    Vdbe_Branch_ತೆಗೆದುಕೊಂಡ(0, 2);
  }
  ನಿಲ್ಲಿಸು;
}

/* Opcode: Trace P1 P2 * P4 *
**
** Write P4 on the statement trace output if statement tracing is
** enabled.
**
** Operand P1 must be 0x7fffffff and P2 must positive.
*/
/* Opcode: Init P1 P2 P3 P4 *
** Synopsis: Start at P2
**
** Programs contain a single instance of this opcode as the very first
** opcode.
**
** If tracing is enabled (by the sqlite3_trace()) interface, then
** the UTF-8 string contained in P4 is emitted on the trace callback.
** Or if P4 is blank, use the string returned by sqlite3_sql().
**
** If P2 is not zero, jump to instruction P2.
**
** Increment the value of P1 so that OP_Once opcodes will jump the
** first time they are evaluated for this run.
**
** If P3 is not zero, then it is an address to jump to if an SQLITE_CORRUPT
** error is encountered.
*/
ಸ್ಥಿತಿ OP_Trace:
ಸ್ಥಿತಿ OP_Init: {          /* jump0 */
  ಪೂರ್ಣಾಂಕ i;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
  ಅಕ್ಷರ *zTrace;
#ಆದರೆ_ಅಂತ್ಯ

  /* If the P4 argument is not NULL, then it must be an SQL comment string.
  ** The "--" string is broken up to prevent false-positives with srcck1.c.
  **
  ** This assert() provides evidence for:
  ** EVIDENCE-OF: R-50676-09860 The callback can compute the same text that
  ** would have been returned by the legacy sqlite3_trace() interface by
  ** using the X argument when X begins with "--" and invoking
  ** sqlite3_expanded_sql(P) otherwise.
  */
  ಪ್ರತಿಜ್ಞಾ( pOp->p4.z==0 || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(pOp->p4.z, "-" "- ", 3)==0 );

  /* OP_Init is always instruction 0 */
  ಪ್ರತಿಜ್ಞಾ( pOp==p->aOp || pOp->opcode==OP_Trace );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
  ಆದರೆ( (db->mTrace & (SQLITE_TRACE_STMT|SQLITE_TRACE_LEGACY))!=0
   && p->minWriteFileFormat!=254  /* tag-20220401a */
   && (zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql))!=0
  ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
    ಆದರೆ( db->mTrace & SQLITE_TRACE_LEGACY ){
      ಅಕ್ಷರ *z = sqlite3VdbeExpandSql(p, zTrace);
      db->trace.xLegacy(db->pTraceArg, z);
      sqlite3_free(z);
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( db->nVdbeExec>1 ){
      ಅಕ್ಷರ *z = sqlite3MPrintf(db, "-- %s", zTrace);
      (ಶೂನ್ಯ)db->trace.xV2(SQLITE_TRACE_STMT, db->pTraceArg, p, z);
      sqlite3DbFree(db, z);
    }ಇಲ್ಲದಿದ್ದರೆ{
      (ಶೂನ್ಯ)db->trace.xV2(SQLITE_TRACE_STMT, db->pTraceArg, p, zTrace);
    }
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_USE_FCNTL_TRACE
  zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql);
  ಆದರೆ( zTrace ){
    ಪೂರ್ಣಾಂಕ j;
    ಗಾಗಿ(j=0; j<db->nDb; j++){
      ಆದರೆ( Db_ಮುಖವಾಡ_Test(p->btree_ಮುಖವಾಡ, j)==0 ) ಮುಂದುವರಿ;
      sqlite3_file_control(db, db->aDb[j].zDbSName, SQLITE_FCNTL_TRACE, zTrace);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_USE_FCNTL_TRACE */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( (db->flags & SQLITE_SqlTrace)!=0
   && (zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql))!=0
  ){
    sqlite3DebugPrintf("SQL-trace: %s\n", zTrace);
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRACE */
  ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 );
  ಆದರೆ( pOp->p1>=sqlite3GlobalConfig.i_Once_Reset_ಮಿತಿ ){
    ಆದರೆ( pOp->opcode==OP_Trace ) ನಿಲ್ಲಿಸು;
    ಗಾಗಿ(i=1; i<p->nOp; i++){
      ಆದರೆ( p->aOp[i].opcode==OP_Once ) p->aOp[i].p1 = 0;
    }
    pOp->p1 = 0;
  }
  pOp->p1++;
  p->aCounter[SQLITE_STMTSTATUS_ಸತತ]++;
  ಹೋಗು jump_ಗಮ್ಯ_p2;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_CURSOR_HINTS
/* Opcode: CursorHint P1 * P3 P4 *
**
** Provide a hint to cursor P1. 
**
** If P4 is of type P4_EXPR, then the hint is that the cursor need only return
** rows that satisfy the Expr in P4. TK_REGISTER terms in the P4 expression
** refer to values currently held in registers.  TK_COLUMN terms in the P4
** expression refer to columns in the b-tree to which cursor P1 is pointing.
** P3 is ignore in this case.
**
** Or, if P4 is P4_NOTUSED, then the hint is that cursor P1 is an index cursor
** used to drive table cursor P3. In other words, that this VM may execute
** OP_DeferredSeek instructions to lazily position P3 based on current 
** position of P1.
*/
ಸ್ಥಿತಿ OP_CursorHint: {
  VdbeCursor *pC;
  pC = p->apCsr[pOp->p1];

  ಪ್ರತಿಜ್ಞಾ( pOp->p1>=0 && pOp->p1<p->nCursor );

  ಆದರೆ( pC ){
    ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
    ಆದರೆ( pOp->p4type==P4_ಅಭಿವ್ಯಕ್ತಿ ){
      sqlite3BtreeCursorHint(pC->uc.pCursor, BTREE_HINT_RANGE,
          pOp->p4.p_ಅಭಿವ್ಯಕ್ತಿ, aMem);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->apCsr[pOp->p3] ){
      sqlite3BtreeCursorHint(
          pC->uc.pCursor, BTREE_HINT_TABLECURSOR, p->apCsr[pOp->p3]->uc.pCursor
      );
    }
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_CURSOR_HINTS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/* Opcode:  Abortable   * * * * *
**
** Verify that an Abort can happen.  Assert if an Abort at this point
** might cause database corruption.  This opcode only appears in debugging
** builds.
**
** An Abort is safe if either there have been no writes, or if there is
** an active statement journal.
*/
ಸ್ಥಿತಿ OP_Abortable: {
  sqlite3VdbeAssertAbortable(p);
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/* Opcode:  ReleaseReg   P1 P2 P3 * P5
** Synopsis: release r[P1@P2] mask P3
**
** Release registers from service.  Any content that was in the
** the registers is unreliable after this opcode completes.
**
** The registers released will be the P2 registers starting at P1,
** except if bit ii of P3 set, then do not release register P1+ii.
** In other words, P3 is a mask of registers to preserve.
**
** Releasing a register clears the Mem.pScopyFrom pointer.  That means
** that if the content of the released register was set using OP_SCopy,
** a change to the value of the source register for the OP_SCopy will no longer
** generate an assertion fault in sqlite3VdbeMemAboutToChange().
**
** If P5 is set, then all released registers have their type set
** to MEM_Undefined so that any subsequent attempt to read the released
** register (before it is reinitialized) will generate an assertion fault.
**
** P5 ought to be set on every call to this opcode.
** However, there are places in the code generator will release registers
** before their are used, under the (valid) assumption that the registers
** will not be reallocated for some other purpose before they are used and
** hence are safe to release.
**
** This opcode is only available in testing and debugging builds.  It is
** not generated for release builds.  The purpose of this opcode is to help
** validate the generated bytecode.  This opcode does not actually contribute
** to computing an answer.
*/
ಸ್ಥಿತಿ OP_ReleaseReg: {
  Mem *pMem;
  ಪೂರ್ಣಾಂಕ i;
  u32 const_ಮುಖವಾಡ;
  ಪ್ರತಿಜ್ಞಾ( pOp->p1>0 );
  ಪ್ರತಿಜ್ಞಾ( pOp->p1+pOp->p2<=(p->nMem+1 - p->nCursor)+1 );
  pMem = &aMem[pOp->p1];
  const_ಮುಖವಾಡ = pOp->p3;
  ಗಾಗಿ(i=0; i<pOp->p2; i++, pMem++){
    ಆದರೆ( i>=32 || (const_ಮುಖವಾಡ & MASKBIT32(i))==0 ){
      pMem->pScopyFrom = 0;
      ಆದರೆ( i<32 && pOp->p5 ) Mem_ಗಣ_Type_Flag(pMem, MEM_Undefined);
    }
  }
  ನಿಲ್ಲಿಸು;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Opcode: Noop * * * * *
**
** Do nothing.  Continue downward to the next opcode.
*/
/* Opcode: Explain P1 P2 P3 P4 *
**
** This is the same as OP_Noop during normal query execution.  The
** purpose of this opcode is to hold information about the query
** plan for the purpose of EXPLAIN QUERY PLAN output.
**
** The P4 value is human-readable text that describes the query plan
** element.  Something like "SCAN t1" or "SEARCH t2 USING INDEX t2x1".
**
** The P1 value is the ID of the current element and P2 is the parent
** element for the case of nested query plan elements.  If P2 is zero
** then this element is a top-level element.
**
** For loop elements, P3 is the estimated code of each invocation of this
** element.
**
** As with all opcodes, the meanings of the parameters for OP_Explain
** are subject to change from one release to the next.  Applications
** should not attempt to interpret or use any of the information
** contained in the OP_Explain opcode.  The information provided by this
** opcode is intended for testing and debugging use only.
*/
ಪೂರ್ವನಿಯೋಜಿತ: {          /* This is really OP_Noop, OP_Explain */
  ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_Noop || pOp->opcode==OP_Explain );

  ನಿಲ್ಲಿಸು;
}

/*****************************************************************************
** The cases of the switch statement above this line should all be indented
** by 6 spaces.  But the left-most 6 spaces have been removed to improve the
** readability.  From this point on down, the normal indentation rules are
** restored.
*****************************************************************************/
    }

#ಆದರೆ defined(VDBE_PROFILE)
    *pn_ಚಕ್ರ += sqlite3NProfileCnt ? sqlite3NProfileCnt : sqlite3Hwtime();
    pn_ಚಕ್ರ = 0;
#ಅಥವಾಆದರೆ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
    ಆದರೆ( pn_ಚಕ್ರ ){
      *pn_ಚಕ್ರ += sqlite3Hwtime();
      pn_ಚಕ್ರ = 0;
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* The following code adds nothing to the actual functionality
    ** of the program.  It is only here for testing and debugging.
    ** On the other hand, it does burn CPU cycles every time through
    ** the evaluator loop.  So we can leave it out when NDEBUG is defined.
    */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
    ಪ್ರತಿಜ್ಞಾ( pOp>=&aOp[-1] && pOp<&aOp[p->nOp-1] );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಆದರೆ( db->flags & SQLITE_VdbeTrace ){
      u8 opProperty = sqlite3OpcodeProperty[pOrigOp->opcode];
      ಆದರೆ( rc!=0 ) ಬರೆ("rc=%d\n",rc);
      ಆದರೆ( opProperty & (OPFLG_OUT2) ){
        ರಿಜಿಸ್ಟರ್_Trace(pOrigOp->p2, &aMem[pOrigOp->p2]);
      }
      ಆದರೆ( opProperty & OPFLG_OUT3 ){
        ರಿಜಿಸ್ಟರ್_Trace(pOrigOp->p3, &aMem[pOrigOp->p3]);
      }
      ಆದರೆ( opProperty==0xff ){
        /* Never happens.  This code exists to avoid a harmless linkage
        ** warning about sqlite3VdbeRegisterDump() being defined but not
        ** used. */
        sqlite3_Vdbe_ರಿಜಿಸ್ಟರ್_Dump(p);
      }
    }
#ಆದರೆ_ಅಂತ್ಯ  /* SQLITE_DEBUG */
#ಆದರೆ_ಅಂತ್ಯ  /* NDEBUG */
  }  /* The end of the for(;;) loop the loops through opcodes */

  /* If we reach this point, it means that execution is finished with
  ** an error of some kind.
  */
abort_due_ಗಮ್ಯ_error:
  ಆದರೆ( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==SQLITE_IOERR_CORRUPTFS ){
    rc = SQLITE_CORRUPT_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( rc );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( db->flags & SQLITE_VdbeTrace ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTrace = p->zSql;
    ಆದರೆ( zTrace==0 ){
      ಆದರೆ( aOp[0].opcode==OP_Trace ){
        zTrace = aOp[0].p4.z;
      }
      ಆದರೆ( zTrace==0 ) zTrace = "???";
    }
    ಬರೆ("ABORT-due-to-error (rc=%d): %s\n", rc, zTrace);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( p->z_Err_ಸಂದೇಶ==0 && rc!=SQLITE_IOERR_NOMEM ){
    sqlite3VdbeError(p, "%s", sqlite3ErrStr(rc));
  }
  p->rc = rc;
  sqlite3SystemError(db, rc);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  sqlite3VdbeLogAbort(p, rc, pOp, aOp);
  ಆದರೆ( p->eVdbeState==VDBE_ಸತತ_STATE ) sqlite3VdbeHalt(p);
  ಆದರೆ( rc==SQLITE_IOERR_NOMEM ) sqlite3OomFault(db);
  ಆದರೆ( rc==SQLITE_CORRUPT && db->autoCommit==0 ){
    db->flags |= SQLITE_CorruptRdOnly;
  }
  rc = SQLITE_ERROR;
  ಆದರೆ( resetSchemaOnFault>0 ){
    sqlite3ResetOneSchema(db, resetSchemaOnFault-1);
  }

  /* This is the only way out of this procedure.  We have to
  ** release the mutexes on btrees that were acquired at the
  ** top. */
vdbe_return:
#ಆದರೆ defined(VDBE_PROFILE)
  ಆದರೆ( pn_ಚಕ್ರ ){
    *pn_ಚಕ್ರ += sqlite3NProfileCnt ? sqlite3NProfileCnt : sqlite3Hwtime();
    pn_ಚಕ್ರ = 0;
  }
#ಅಥವಾಆದರೆ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
  ಆದರೆ( pn_ಚಕ್ರ ){
    *pn_ಚಕ್ರ += sqlite3Hwtime();
    pn_ಚಕ್ರ = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
  ಆಗ( n_Vm_ಹೆಜ್ಜೆ>=n_ಪ್ರಗತಿ_ಎಲ್ಲೆ && db->x_ಪ್ರಗತಿ!=0 ){
    n_ಪ್ರಗತಿ_ಎಲ್ಲೆ += db->n_ಪ್ರಗತಿ_Ops;
    ಆದರೆ( db->x_ಪ್ರಗತಿ(db->p_ಪ್ರಗತಿ_Arg) ){
      n_ಪ್ರಗತಿ_ಎಲ್ಲೆ = ಅತಿದೊಡ್ಡ_UINT64;
      rc = SQLITE_ಅಡಚಣೆ;
      ಹೋಗು abort_due_ಗಮ್ಯ_error;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  p->aCounter[SQLITE_STMTSTATUS_VM_ಹೆಜ್ಜೆ] += (ಪೂರ್ಣಾಂಕ)n_Vm_ಹೆಜ್ಜೆ;
  ಆದರೆ( Db_ಮುಖವಾಡ_Non_Zero(p->ಬೀಗ_ಮುಖವಾಡ) ){
    sqlite3VdbeLeave(p);
  }
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || nExtraDelete==0
       || sqlite3_strlike("DELETE%",p->zSql,0)!=0
  );
  ಹಿಂತಿರುಗಿ rc;

  /* Jump to here if a string or blob larger than SQLITE_MAX_LENGTH
  ** is encountered.
  */
too_big:
  sqlite3VdbeError(p, "string or blob too big");
  rc = SQLITE_TOOBIG;
  ಹೋಗು abort_due_ಗಮ್ಯ_error;

  /* Jump to here if a malloc() fails.
  */
no_mem:
  sqlite3OomFault(db);
  sqlite3VdbeError(p, "out of memory");
  rc = SQLITE_NOMEM_BKPT;
  ಹೋಗು abort_due_ಗಮ್ಯ_error;

  /* Jump to here if the sqlite3_interrupt() API sets the interrupt
  ** flag.
  */
abort_due_ಗಮ್ಯ_ಅಡಚಣೆ:
  ಪ್ರತಿಜ್ಞಾ( Atomic_ತುಂಬು(&db->u1.isInterrupted) );
  rc = SQLITE_ಅಡಚಣೆ;
  ಹೋಗು abort_due_ಗಮ್ಯ_error;
}
