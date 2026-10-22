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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

/*
** High-resolution hardware timer used for debugging and testing only.
*/
#ఒకవేళ defined(VDBE_PROFILE)  \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
# include "hwtime.h"
#అయితే_ముగింపు

/*
** Invoke this macro on memory cells just prior to changing the
** value of the cell.  This macro verifies that shallow copies are
** not misused.  A shallow copy of a string or blob just copies a
** pointer to the string or blob, not the content.  If the original
** is changed while the copy is still in use, the string or blob might
** be changed out from under the copy.  This macro verifies that nothing
** like that ever happens.
*/
#నిర్వచితమైతే SQLITE_DEBUG
# define mem_About_గమ్యం_Change(P,M) sqlite3_Vdbe_Mem_About_గమ్యం_Change(P,M)
#లేకపోతే
# define mem_About_గమ్యం_Change(P,M)
#అయితే_ముగింపు

/*
** The following global variable is incremented every time a cursor
** moves, either by the OP_SeekXX, OP_Next, or OP_Prev opcodes.  The test
** procedures use this information to make sure that indices are
** working correctly.  This variable has no function other than to
** help verify the correct operation of the library.
*/
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3_శోధన_లెక్క = 0;
#అయితే_ముగింపు

/*
** When this global variable is positive, it gets decremented once before
** each instruction in the VDBE.  When it reaches zero, the u1.isInterrupted
** field of the sqlite3 structure is set in order to simulate an interrupt.
**
** This facility is used for testing purposes only.  It does not function
** in an ordinary build.
*/
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3_అంతరాయం_లెక్క = 0;
#అయితే_ముగింపు

/*
** The next global variable is incremented each type the OP_Sort opcode
** is executed.  The test procedures use this information to make sure that
** sorting is occurring or not occurring at appropriate times.   This variable
** has no function other than to help verify the correct operation of the
** library.
*/
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3_క్రమీకరించు_లెక్క = 0;
#అయితే_ముగింపు

/*
** The next global variable records the size of the largest MEM_Blob
** or MEM_Str that has been used by a VDBE opcode.  The test procedures
** use this information to make sure that the zero-blob functionality
** is working correctly.   This variable has no function other than to
** help verify the correct operation of the library.
*/
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3_max_blobsize = 0;
స్థిర శూన్య updateMaxBlobsize(Mem *p){
  ఒకవేళ( (p->flags & (MEM_Str|MEM_Blob))!=0 && p->n>sqlite3_max_blobsize ){
    sqlite3_max_blobsize = p->n;
  }
}
#అయితే_ముగింపు

/*
** This macro evaluates to true if either the update hook or the preupdate
** hook are enabled for database connect DB.
*/
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
# define HAS_UPDATE_HOOK(DB) ((DB)->xPreUpdateCallback||(DB)->xUpdateCallback)
#లేకపోతే
# define HAS_UPDATE_HOOK(DB) ((DB)->xUpdateCallback)
#అయితే_ముగింపు

/*
** The next global variable is incremented each time the OP_Found opcode
** is executed. This is used to test whether or not the foreign key
** operation implemented using OP_FkIsZero is working. This variable
** has no function other than to help verify the correct operation of the
** library.
*/
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3_దొరికింది_లెక్క = 0;
#అయితే_ముగింపు

/*
** Test a register to see if it exceeds the current maximum blob size.
** If it does, record the new maximum blob size.
*/
#ఒకవేళ defined(SQLITE_TEST) && !defined(SQLITE_UNTESTABLE)
# define UPDATE_MAX_BLOBSIZE(P)  updateMaxBlobsize(P)
#లేకపోతే
# define UPDATE_MAX_BLOBSIZE(P)
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర శూన్య test_trace_breakpoint(పూర్ణాంకం pc, Op *pOp, Vdbe *v){
  స్థిర u64 n = 0;
  (శూన్య)pc;
  (శూన్య)pOp;
  (శూన్య)v;
  n++;
  ఒకవేళ( n==అతిపెద్ద_UINT64 ) abort(); /* So that n is used, preventing a warning */
}
#అయితే_ముగింపు

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
#ఒకవేళ !defined(SQLITE_VDBE_COVERAGE)
# define Vdbe_Branch_తీసుకున్న(I,M)
#లేకపోతే
# define Vdbe_Branch_తీసుకున్న(I,M) vdbeTakeBranch(pOp->i_Src_పంక్తి,I,M)
  స్థిర శూన్య vdbeTakeBranch(u32 i_Src_పంక్తి, u8 I, u8 M){
    u8 mNever;
    ప్రతిజ్ఞా( I<=2 );  /* 0: fall through,  1: taken,  2: alternate taken */
    ప్రతిజ్ఞా( M<=4 );  /* 2: two-way branch, 3: three-way branch, 4: OP_Jump */
    ప్రతిజ్ఞా( I<M );   /* I can only be 2 if M is 3 or 4 */
    /* Transform I from a integer [0,1,2] into a bitmask of [1,2,4] */
    I = 1<<I;
    /* The upper 8 bits of iSrcLine are flags.  The lower three bits of
    ** the flags indicate directions that the branch can never go.  If
    ** a branch really does go in one of those directions, assert right
    ** away. */
    mNever = i_Src_పంక్తి >> 24;
    ప్రతిజ్ఞా( (I & mNever)==0 );
    ఒకవేళ( sqlite3GlobalConfig.xVdbeBranch==0 ) తిరిగి;  /*NO_TEST*/
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
    ఒకవేళ( M==2 ) I |= 0x04;
    ఒకవేళ( M==4 ){
      I |= 0x08;
      ఒకవేళ( (mNever&0x08)!=0 && (I&0x05)!=0) I |= 0x05; /*NO_TEST*/
    }
    sqlite3GlobalConfig.xVdbeBranch(sqlite3GlobalConfig.pVdbeBranchArg,
                                    i_Src_పంక్తి&0xffffff, I, M);
  }
#అయితే_ముగింపు

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
#నిర్వచించు Deephemeralize(P) \
   ఒకవేళ( ((P)->flags&MEM_Ephem)!=0 \
       && sqlite3VdbeMemMakeWriteable(P) ){ వెళ్ళు no_mem;}

/* Return true if the cursor was opened using the OP_OpenSorter opcode. */
#నిర్వచించు isSorter(x) ((x)->eCurType==CURTYPE_SORTER)

/*
** Allocate VdbeCursor number iCur.  Return a pointer to it.  Return NULL
** if we run out of memory.
*/
స్థిర VdbeCursor *allocateCursor(
  Vdbe *p,              /* The virtual machine */
  పూర్ణాంకం iCur,             /* Index of the new VdbeCursor */
  పూర్ణాంకం n_క్షేత్రం,           /* Number of fields in the table or index */
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
  nByte = SZ_VDBECURSOR(n_క్షేత్రం);
  ప్రతిజ్ఞా( ROUND8(nByte)==nByte );
  ఒకవేళ( eCurType==CURTYPE_BTREE ) nByte += sqlite3_Btree_Cursor_కొలత();

  ప్రతిజ్ఞా( iCur>=0 && iCur<p->nCursor );
  ఒకవేళ( p->apCsr[iCur] ){ /*OPTIMIZATION-IF-FALSE*/
    sqlite3VdbeFreeCursorNN(p, p->apCsr[iCur]);
    p->apCsr[iCur] = 0;
  }

  /* There used to be a call to sqlite3VdbeMemClearAndResize() to make sure
  ** the pMem used to hold space for the cursor has enough storage available
  ** in pMem->zMalloc.  But for the special case of the aMem[] entries used
  ** to hold cursors, it is faster to in-line the logic. */
  ప్రతిజ్ఞా( pMem->flags==MEM_Undefined );
  ప్రతిజ్ఞా( (pMem->flags & MEM_Dyn)==0 );
  ప్రతిజ్ఞా( pMem->szMalloc==0 || pMem->z==pMem->zMalloc );
  ఒకవేళ( pMem->szMalloc<nByte ){
    ఒకవేళ( pMem->szMalloc>0 ){
      sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    }
    pMem->z = pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, nByte);
    ఒకవేళ( pMem->zMalloc==0 ){
      pMem->szMalloc = 0;
      తిరిగి 0;
    }
    pMem->szMalloc = (పూర్ణాంకం)nByte;
  }

  p->apCsr[iCur] = pCx = (VdbeCursor*)pMem->zMalloc;
  memset(pCx, 0, offsetof(VdbeCursor,pAltCursor));
  pCx->eCurType = eCurType;
  pCx->n_క్షేత్రం = n_క్షేత్రం;
  pCx->a_స్థానభ్రంశం = &pCx->aType[n_క్షేత్రం];
  ఒకవేళ( eCurType==CURTYPE_BTREE ){
    ప్రతిజ్ఞా( ROUND8(SZ_VDBECURSOR(n_క్షేత్రం))==SZ_VDBECURSOR(n_క్షేత్రం) );
    pCx->uc.pCursor = (BtCursor*)&pMem->z[SZ_VDBECURSOR(n_క్షేత్రం)];
    sqlite3BtreeCursorZero(pCx->uc.pCursor);
  }
  తిరిగి pCx;
}

/*
** The string in pRec is known to look like an integer and to have a
** floating point value of rValue.  Return true and set *piValue to the
** integer value if the string is in range to be an integer.  Otherwise,
** return false.
*/
స్థిర పూర్ణాంకం alsoAnInt(Mem *pRec, ద్విగుణ r_విలువ, i64 *pi_విలువ){
  i64 i_విలువ;
  i_విలువ = sqlite3_Real_గమ్యం_I64(r_విలువ);
  ఒకవేళ( sqlite3RealSameAsInt(r_విలువ,i_విలువ) ){
    *pi_విలువ = i_విలువ;
    తిరిగి 1;
  }
  తిరిగి 0==sqlite3Atoi64(pRec->z, pi_విలువ, pRec->n, pRec->enc);
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
స్థిర శూన్య applyNumericAffinity(Mem *pRec, పూర్ణాంకం bTryForInt){
  ద్విగుణ r_విలువ;
  పూర్ణాంకం rc;
  ప్రతిజ్ఞా( (pRec->flags & (MEM_Str|MEM_Int|MEM_Real|MEM_IntReal))==MEM_Str );
  rc = sqlite3_Mem_Real_విలువ_RC(pRec, &r_విలువ);
  ఒకవేళ( rc<=0 ) తిరిగి;
  ఒకవేళ( (rc&2)==0 && alsoAnInt(pRec, r_విలువ, &pRec->u.i) ){
    pRec->flags |= MEM_Int;
  }లేకపోతే{
    pRec->u.r = r_విలువ;
    pRec->flags |= MEM_Real;
    ఒకవేళ( bTryForInt ) sqlite3VdbeIntegerAffinity(pRec, 1);
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
స్థిర శూన్య applyAffinity(
  Mem *pRec,          /* The value to apply affinity to */
  అక్షరం affinity,      /* The affinity to be applied */
  u8 enc              /* Use this text encoding */
){
  ఒకవేళ( affinity>=SQLITE_AFF_NUMERIC ){
    ప్రతిజ్ఞా( affinity==SQLITE_AFF_INTEGER || affinity==SQLITE_AFF_REAL
             || affinity==SQLITE_AFF_NUMERIC || affinity==SQLITE_AFF_FLEXNUM );
    ఒకవేళ( (pRec->flags & MEM_Int)==0 ){ /*OPTIMIZATION-IF-FALSE*/
      ఒకవేళ( (pRec->flags & (MEM_Real|MEM_IntReal))==0 ){
        ఒకవేళ( pRec->flags & MEM_Str ) applyNumericAffinity(pRec,1);
      }లేకపోతే ఒకవేళ( affinity<=SQLITE_AFF_REAL ){
        sqlite3VdbeIntegerAffinity(pRec, 0);
      }
    }
  }లేకపోతే ఒకవేళ( affinity==SQLITE_AFF_పాఠ్యం ){
    /* Only attempt the conversion to TEXT if there is an integer or real
    ** representation (blob and NULL do not get converted) but no string
    ** representation.  It would be harmless to repeat the conversion if
    ** there is already a string rep, but it is pointless to waste those
    ** CPU cycles. */
    ఒకవేళ( 0==(pRec->flags&MEM_Str) ){ /*OPTIMIZATION-IF-FALSE*/
      ఒకవేళ( (pRec->flags&(MEM_Real|MEM_Int|MEM_IntReal)) ){
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
పూర్ణాంకం sqlite3_విలువ_numeric_type(sqlite3_విలువ *pVal){
  పూర్ణాంకం eType = sqlite3_విలువ_type(pVal);
  ఒకవేళ( eType==SQLITE_పాఠ్యం ){
    Mem *pMem = (Mem*)pVal;
#ఒకవేళ SQLITE_THREADSAFE>0
    sqlite3_mutex *pMutex = pMem->db ? pMem->db->mutex : 0;
#అయితే_ముగింపు
    sqlite3_mutex_enter(pMutex);
    applyNumericAffinity(pMem, 0);
    sqlite3_mutex_leave(pMutex);
    eType = sqlite3_విలువ_type(pVal);
  }
  తిరిగి eType;
}

/*
** Exported version of applyAffinity(). This one works on sqlite3_value*,
** not the internal Mem* type.
*/
శూన్య sqlite3_విలువ_Apply_Affinity(
  sqlite3_విలువ *pVal,
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
స్థిర u16 SQLITE_NOINLINE computeNumericType(Mem *pMem){
  పూర్ణాంకం rc;
  sqlite3_int64 ix;
  ప్రతిజ్ఞా( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 );
  ప్రతిజ్ఞా( (pMem->flags & (MEM_Str|MEM_Blob))!=0 );
  ఒకవేళ( ExpandBlob(pMem) ){
    pMem->u.i = 0;
    తిరిగి MEM_Int;
  }
  rc = sqlite3_Mem_Real_విలువ_RC(pMem, &pMem->u.r);
  ఒకవేళ( rc<=0 ){
    ఒకవేళ( (rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<=1 ){
      pMem->u.i = ix;
      తిరిగి MEM_Int;
    }లేకపోతే{
      తిరిగి MEM_Real;
    }
  }లేకపోతే ఒకవేళ( (rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)==0 ){
    pMem->u.i = ix;
    తిరిగి MEM_Int;
  }
  తిరిగి MEM_Real;
}

/*
** Return the numeric type for pMem, either MEM_Int or MEM_Real or both or
** none. 
**
** Unlike applyNumericAffinity(), this routine does not modify pMem->flags.
** But it does set pMem->u.r and pMem->u.i appropriately.
*/
స్థిర u16 numericType(Mem *pMem){
  ప్రతిజ్ఞా( (pMem->flags & MEM_Null)==0
       || pMem->db==0 || pMem->db->mallocFailed );
  ఒకవేళ( pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null) ){
    testcase( pMem->flags & MEM_Int );
    testcase( pMem->flags & MEM_Real );
    testcase( pMem->flags & MEM_IntReal );
    తిరిగి pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null);
  }
  ప్రతిజ్ఞా( pMem->flags & (MEM_Str|MEM_Blob) );
  testcase( pMem->flags & MEM_Str );
  testcase( pMem->flags & MEM_Blob );
  తిరిగి computeNumericType(pMem);
  తిరిగి 0;
}

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Write a nice string representation of the contents of cell pMem
** into buffer zBuf, length nBuf.
*/
శూన్య sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr){
  పూర్ణాంకం f = pMem->flags;
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి encnames[] = {"(X)", "(8)", "(16LE)", "(16BE)"};
  ఒకవేళ( f&MEM_Blob ){
    పూర్ణాంకం i;
    అక్షరం c;
    ఒకవేళ( f & MEM_Dyn ){
      c = 'z';
      ప్రతిజ్ఞా( (f & (MEM_Static|MEM_Ephem))==0 );
    }లేకపోతే ఒకవేళ( f & MEM_Static ){
      c = 't';
      ప్రతిజ్ఞా( (f & (MEM_Dyn|MEM_Ephem))==0 );
    }లేకపోతే ఒకవేళ( f & MEM_Ephem ){
      c = 'e';
      ప్రతిజ్ఞా( (f & (MEM_Static|MEM_Dyn))==0 );
    }లేకపోతే{
      c = 's';
    }
    sqlite3_str_appendf(pStr, "%cx[", c);
    కొరకు(i=0; i<25 && i<pMem->n; i++){
      sqlite3_str_appendf(pStr, "%02X", ((పూర్ణాంకం)pMem->z[i] & 0xFF));
    }
    sqlite3_str_appendf(pStr, "|");
    కొరకు(i=0; i<25 && i<pMem->n; i++){
      అక్షరం z = pMem->z[i];
      sqlite3_str_appendchar(pStr, 1, (z<32||z>126)?'.':z);
    }
    sqlite3_str_appendf(pStr,"]");
    ఒకవేళ( f & MEM_Zero ){
      sqlite3_str_appendf(pStr, "+%dz",pMem->u.nZero);
    }
  }లేకపోతే ఒకవేళ( f & MEM_Str ){
    పూర్ణాంకం j;
    u8 c;
    ఒకవేళ( f & MEM_Dyn ){
      c = 'z';
      ప్రతిజ్ఞా( (f & (MEM_Static|MEM_Ephem))==0 );
    }లేకపోతే ఒకవేళ( f & MEM_Static ){
      c = 't';
      ప్రతిజ్ఞా( (f & (MEM_Dyn|MEM_Ephem))==0 );
    }లేకపోతే ఒకవేళ( f & MEM_Ephem ){
      c = 'e';
      ప్రతిజ్ఞా( (f & (MEM_Static|MEM_Dyn))==0 );
    }లేకపోతే{
      c = 's';
    }
    sqlite3_str_appendf(pStr, " %c%d[", c, pMem->n);
    కొరకు(j=0; j<25 && j<pMem->n; j++){
      c = pMem->z[j];
      sqlite3_str_appendchar(pStr, 1, (c>=0x20&&c<=0x7f) ? c : '.');
    }
    sqlite3_str_appendf(pStr, "]%s", encnames[pMem->enc]);
    ఒకవేళ( f & MEM_పదం ){
      sqlite3_str_appendf(pStr, "(0-term)");
    }
  }
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Print the value of a register for tracing purposes:
*/
స్థిర శూన్య memTracePrint(Mem *p){
  ఒకవేళ( p->flags & MEM_Undefined ){
    రాయి(" undefined");
  }లేకపోతే ఒకవేళ( p->flags & MEM_Null ){
    రాయి(p->flags & MEM_Zero ? " NULL-nochng" : " NULL");
  }లేకపోతే ఒకవేళ( (p->flags & (MEM_Int|MEM_Str))==(MEM_Int|MEM_Str) ){
    రాయి(" si:%lld", p->u.i);
  }లేకపోతే ఒకవేళ( (p->flags & (MEM_IntReal))!=0 ){
    రాయి(" ir:%lld", p->u.i);
  }లేకపోతే ఒకవేళ( p->flags & MEM_Int ){
    రాయి(" i:%lld", p->u.i);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
  }లేకపోతే ఒకవేళ( p->flags & MEM_Real ){
    రాయి(" r:%.17g", p->u.r);
#అయితే_ముగింపు
  }లేకపోతే ఒకవేళ( sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(p) ){
    రాయి(" (rowset)");
  }లేకపోతే{
    StrAccum acc;
    అక్షరం zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, పరిమాణం(zBuf), 0);
    sqlite3VdbeMemPrettyPrint(p, &acc);
    రాయి(" %s", sqlite3_Str_Accum_ముగింపు(&acc));
  }
  ఒకవేళ( p->flags & MEM_Subtype ) రాయి(" subtype=0x%02x", p->eSubtype);
}
స్థిర శూన్య రిజిస్టరు_Trace(పూర్ణాంకం iReg, Mem *p){
  రాయి("R[%d] = ", iReg);
  memTracePrint(p);
  ఒకవేళ( p->pScopyFrom ){
    ప్రతిజ్ఞా( p->pScopyFrom->bScopy );
    రాయి(" <== R[%d]", (పూర్ణాంకం)(p->pScopyFrom - &p[-iReg]));
  }
  రాయి("\n");
  sqlite3VdbeCheckMemInvariants(p);
}
/**/ శూన్య sqlite3PrintMem(Mem *pMem){
  memTracePrint(pMem);
  రాయి("\n");
  fflush(stdout);
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Show the values of all registers in the virtual machine.  Used for
** interactive debugging.
*/
శూన్య sqlite3_Vdbe_రిజిస్టరు_Dump(Vdbe *v){
  పూర్ణాంకం i;
  కొరకు(i=1; i<v->nMem; i++) రిజిస్టరు_Trace(i, v->aMem+i);
}
#అయితే_ముగింపు /* SQLITE_DEBUG */


#నిర్వచితమైతే SQLITE_DEBUG
#  define రిజిస్టరు_TRACE(R,M) ఒకవేళ(db->flags&SQLITE_VdbeTrace)రిజిస్టరు_Trace(R,M)
#లేకపోతే
#  define రిజిస్టరు_TRACE(R,M)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే NDEBUG
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
స్థిర పూర్ణాంకం check_Savepoint_లెక్క(sqlite3 *db){
  పూర్ణాంకం n = 0;
  Savepoint *p;
  కొరకు(p=db->pSavepoint; p; p=p->p_తదుపరి) n++;
  ప్రతిజ్ఞా( n==(db->nSavepoint + db->isTransactionSavepoint) );
  తిరిగి 1;
}
#అయితే_ముగింపు

/*
** Return the register of pOp->p2 after first preparing it to be
** overwritten with an integer value.
*/
స్థిర SQLITE_NOINLINE Mem *out2PrereleaseWithClear(Mem *p_బహిర్గతం){
  sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
  p_బహిర్గతం->flags = MEM_Int;
  తిరిగి p_బహిర్గతం;
}
స్థిర Mem *out2Prerelease(Vdbe *p, VdbeOp *pOp){
  Mem *p_బహిర్గతం;
  ప్రతిజ్ఞా( pOp->p2>0 );
  ప్రతిజ్ఞా( pOp->p2<=(p->nMem+1 - p->nCursor) );
  p_బహిర్గతం = &p->aMem[pOp->p2];
  mem_About_గమ్యం_Change(p, p_బహిర్గతం);
  ఒకవేళ( VdbeMemDynamic(p_బహిర్గతం) ){ /*OPTIMIZATION-IF-FALSE*/
    తిరిగి out2PrereleaseWithClear(p_బహిర్గతం);
  }లేకపోతే{
    p_బహిర్గతం->flags = MEM_Int;
    తిరిగి p_బహిర్గతం;
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
స్థిర u64 filterHash(స్థిరరాశి Mem *aMem, స్థిరరాశి Op *pOp){
  పూర్ణాంకం i, mx;
  u64 h = 0;

  ప్రతిజ్ఞా( pOp->p4type==P4_INT32 );
  కొరకు(i=pOp->p3, mx=i+pOp->p4.i; i<mx; i++){
    స్థిరరాశి Mem *p = &aMem[i];
    ఒకవేళ( p->flags & (MEM_Int|MEM_IntReal) ){
      h += p->u.i;
    }లేకపోతే ఒకవేళ( p->flags & MEM_Real ){
      h += sqlite3_Vdbe_Int_విలువ(p);
    }లేకపోతే ఒకవేళ( p->flags & MEM_Str ){
      u64 x;
      h += p->n;
      ఒకవేళ( p->n >= (పూర్ణాంకం)పరిమాణం(x) ){
        జ్ఞాపకం_నకలు(&x, p->z, పరిమాణం(x));
        h += x;
        జ్ఞాపకం_నకలు(&x, p->z + p->n - పరిమాణం(x), పరిమాణం(x));
        h += x;
      }లేకపోతే{
        x = 0;
        జ్ఞాపకం_నకలు(&x, p->z, p->n);
        h += x;
      }
    }లేకపోతే ఒకవేళ( p->flags & MEM_Blob ){
      పూర్ణాంకం n = p->n;
      u64 x = 0;
      ఒకవేళ( n ){
        జ్ఞాపకం_నకలు(&x, p->z, MIN(n, (పూర్ణాంకం)పరిమాణం(x)));
        h += x;
      }
      h += n;
      ఒకవేళ( p->flags & MEM_Zero ) h += p->u.nZero;
    }
  }
  తిరిగి h;
}

/*
** For OP_Column, factor out the case where content is loaded from
** overflow pages, so that the code to implement this case is separate
** the common case where all content fits on the page.  Factoring out
** the code reduces register pressure and helps the common case
** to run faster.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం vdbe_స్తంభం_From_అధికప్రవాహం(
  VdbeCursor *pC,       /* The BTree cursor from which we are reading */
  పూర్ణాంకం i_నిలువు,             /* The column to read */
  u32 t,                /* The serial-type code for the column value */
  i64 i_స్థానభ్రంశం,          /* Offset to the start of the content value */
  u32 cache_స్థితి,      /* Current Vdbe.cacheCtr value */
  u32 నిలువు_Cache_Ctr,      /* Current value of the column cache counter */
  Mem *pDest            /* Store the value into this register. */
){
  పూర్ణాంకం rc;
  sqlite3 *db = pDest->db;
  పూర్ణాంకం encoding = pDest->enc;
  పూర్ణాంకం len = sqlite3VdbeSerialTypeLen(t);
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ఒకవేళ( len>db->a_హద్దు[SQLITE_హద్దు_నిడివి] ) తిరిగి SQLITE_TOOBIG;
  ఒకవేళ( len > 4000 && pC->p_కీలకం_Info==0 ){
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
    అక్షరం *pBuf;
    ఒకవేళ( pC->నిలువు_Cache==0 ){
      pC->pCache = sqlite3DbMallocZero(db, పరిమాణం(VdbeTxtBlbCache) );
      ఒకవేళ( pC->pCache==0 ) తిరిగి SQLITE_NOMEM;
      pC->నిలువు_Cache = 1;
    }
    pCache = pC->pCache;
    ఒకవేళ( pCache->pCValue==0
     || pCache->i_నిలువు!=i_నిలువు
     || pCache->cache_స్థితి!=cache_స్థితి
     || pCache->నిలువు_Cache_Ctr!=నిలువు_Cache_Ctr
     || pCache->i_స్థానభ్రంశం!=sqlite3_Btree_స్థానభ్రంశం(pC->uc.pCursor)
    ){
      ఒకవేళ( pCache->pCValue ) sqlite3RCStrUnref(pCache->pCValue);
      pBuf = pCache->pCValue = sqlite3RCStrNew( len+3 );
      ఒకవేళ( pBuf==0 ) తిరిగి SQLITE_NOMEM;
      rc = sqlite3BtreePayload(pC->uc.pCursor, i_స్థానభ్రంశం, len, pBuf);
      ఒకవేళ( rc ) తిరిగి rc;
      pBuf[len] = 0;
      pBuf[len+1] = 0;
      pBuf[len+2] = 0;
      pCache->i_నిలువు = i_నిలువు;
      pCache->cache_స్థితి = cache_స్థితి;
      pCache->నిలువు_Cache_Ctr = నిలువు_Cache_Ctr;
      pCache->i_స్థానభ్రంశం = sqlite3_Btree_స్థానభ్రంశం(pC->uc.pCursor);
    }లేకపోతే{
      pBuf = pCache->pCValue;
    }
    ప్రతిజ్ఞా( t>=12 );
    sqlite3RCStrRef(pBuf);
    ఒకవేళ( t&1 ){
      /* Prevents the removal of a BOM in UTF16 text.  Otherwise
      ** it causes no harm as long as len>=0, and pDest->enc is set
      ** to the correct encoding afterwards: ----
      ** (tag-20261016-1)                        `vvvvvvvvvvv    */
      rc = sqlite3_Vdbe_Mem_సమితి_Str(pDest, pBuf, len, SQLITE_UTF8,
                                sqlite3RCStrUnref);
      pDest->flags |= MEM_పదం;
    }లేకపోతే{
      rc = sqlite3_Vdbe_Mem_సమితి_Str(pDest, pBuf, len, 0,
                                sqlite3RCStrUnref);
     }
    pDest->enc = encoding;
  }లేకపోతే{
    rc = sqlite3VdbeMemFromBtree(pC->uc.pCursor, i_స్థానభ్రంశం, len, pDest);
    ఒకవేళ( rc ) తిరిగి rc;
    sqlite3VdbeSerialGet((స్థిరరాశి u8*)pDest->z, t, pDest);
    ఒకవేళ( (t&1)!=0 && encoding==SQLITE_UTF8 ){
      pDest->z[len] = 0;
      pDest->flags |= MEM_పదం;
    }
  }
  pDest->flags &= ~MEM_Ephem;
  తిరిగి rc;
}

/*
** Memory cell pMem may contain a blob or a NULL value. Cursor pCsr is
** open on an index. If the current index entry matches the blob value in
** pMem byte-for-byte, set pMem to NULL and return 1. Otherwise, return 0.
**
** If an error occurs, set (*pRc) to an SQLite error code. Return 1 in this
** case as well.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం vdbe_సూచీ_కీలకం_Compare(
  BtCursor *pCsr,                 /* Cursor to compare key to */
  Mem *pMem,
  పూర్ణాంకం *pRc
){
  పూర్ణాంకం ret = 0;
  u32 n_కీలకం = 0;

  ప్రతిజ్ఞా( pMem->flags & (MEM_Blob|MEM_Null) );
  n_కీలకం = sqlite3_Btree_Payload_కొలత(pCsr);
  ఒకవేళ( n_కీలకం==(u32)pMem->n && ALWAYS((pMem->flags & MEM_Blob)!=0) ){
    /* This code could just use sqlite3BtreePayloadFetch(). But calling that
    ** function here apparently prevents compilers from inlining it in other,
    ** more performance critical, places. So this code uses
    ** MemFromBtreeZeroOffset(), which is just as fast in most cases, but also
    ** handles the case where the index record uses overflow pages. */
    Mem m;
    memset(&m, 0, పరిమాణం(m));
    *pRc = sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(pCsr, n_కీలకం, &m);
    ret = (*pRc!=SQLITE_OK || 0==memcmp(pMem->z, m.z, n_కీలకం));
    sqlite3VdbeMemReleaseMalloc(&m);
  }

  తిరిగి ret;
}

/*
** Send a "statement aborts" message to the error log.
*/
స్థిర SQLITE_NOINLINE శూన్య sqlite3VdbeLogAbort(
  Vdbe *p,     /* The statement that is running at the time of failure */
  పూర్ణాంకం rc,      /* Error code */
  Op *pOp,     /* Opcode that filed */
  Op *aOp      /* All opcodes */
){
  స్థిరరాశి అక్షరం *zSql = p->zSql;   /* Original SQL text */
  స్థిరరాశి అక్షరం *z_ఉపసర్గ = "";     /* Prefix added to SQL text */
  పూర్ణాంకం pc;                       /* Opcode address */
  అక్షరం zXtra[100];              /* Buffer space to store zPrefix */

  ఒకవేళ( p->p_చట్రం ){
    ప్రతిజ్ఞా( aOp[0].opcode==OP_Init );
    ఒకవేళ( aOp[0].p4.z!=0 ){
      ప్రతిజ్ఞా( aOp[0].p4.z[0]=='-' 
           && aOp[0].p4.z[1]=='-' 
           && aOp[0].p4.z[2]==' ' );
      sqlite3_snprintf(పరిమాణం(zXtra), zXtra,"/* %s */ ",aOp[0].p4.z+3);
      z_ఉపసర్గ = zXtra;
    }లేకపోతే{
      z_ఉపసర్గ = "/* unknown trigger */ ";
    }
  }
  pc = (పూర్ణాంకం)(pOp - aOp);
  sqlite3_log(rc, "statement aborts at %d: %s; [%s%s]",
                   pc, p->z_Err_సందేశం, z_ఉపసర్గ, zSql);
}

/*
** Return the symbolic name for the data type of a pMem
*/
స్థిర స్థిరరాశి అక్షరం *vdbeMemTypeName(Mem *pMem){
  స్థిర స్థిరరాశి అక్షరం *azTypes[] = {
      /* SQLITE_INTEGER */ "INT",
      /* SQLITE_FLOAT   */ "REAL",
      /* SQLITE_TEXT    */ "TEXT",
      /* SQLITE_BLOB    */ "BLOB",
      /* SQLITE_NULL    */ "NULL"
  };
  తిరిగి azTypes[sqlite3_విలువ_type(pMem)-1];
}

/*
** Execute as much of a VDBE program as we can.
** This is the core of sqlite3_step(). 
*/
పూర్ణాంకం sqlite3VdbeExec(
  Vdbe *p                    /* The VDBE */
){
  Op *aOp = p->aOp;          /* Copy of p->aOp */
  Op *pOp = aOp;             /* Current operation */
#నిర్వచితమైతే SQLITE_DEBUG
  Op *pOrigOp;               /* Value of pOp at the top of the loop */
  పూర్ణాంకం nExtraDelete = 0;      /* Verifies FORDELETE and AUXDELETE flags */
  u8 iCompareIsInit = 0;     /* iCompare is initialized */
#అయితే_ముగింపు
  పూర్ణాంకం rc = SQLITE_OK;        /* Value to return */
  sqlite3 *db = p->db;       /* The database */
  u8 resetSchemaOnFault = 0; /* Reset schema after an error if positive */
  u8 encoding = ENC(db);     /* The database encoding */
  పూర్ణాంకం iCompare = 0;          /* Result of last comparison */
  u64 n_Vm_అడుగు = 0;           /* Number of virtual machine steps */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
  u64 n_పురోగతి_హద్దు;        /* Invoke xProgress() when nVmStep reaches this */
#అయితే_ముగింపు
  Mem *aMem = p->aMem;       /* Copy of p->aMem */
  Mem *pIn1 = 0;             /* 1st input operand */
  Mem *pIn2 = 0;             /* 2nd input operand */
  Mem *pIn3 = 0;             /* 3rd input operand */
  Mem *p_బహిర్గతం = 0;             /* Output operand */
  u32 నిలువు_Cache_Ctr = 0;       /* Column cache counter */
#ఒకవేళ defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  u64 *pn_చక్రం = 0;
  పూర్ణాంకం b_Stmt_పరిశీలన_స్థితి = IS_STMT_SCANSTATUS(db)!=0;
#అయితే_ముగింపు
  /*** INSERT STACK UNION HERE ***/

  ప్రతిజ్ఞా( p->eVdbeState==VDBE_నిరంతరం_STATE );  /* sqlite3_step() verifies this */
  ఒకవేళ( Db_ముసుగు_Non_Zero(p->తాళం_ముసుగు) ){
    sqlite3VdbeEnter(p);
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
  ఒకవేళ( db->x_పురోగతి ){
    u32 i_పూర్వ = p->aCounter[SQLITE_STMTSTATUS_VM_అడుగు];
    ప్రతిజ్ఞా( 0 < db->n_పురోగతి_Ops );
    n_పురోగతి_హద్దు = db->n_పురోగతి_Ops - (i_పూర్వ % db->n_పురోగతి_Ops);
  }లేకపోతే{
    n_పురోగతి_హద్దు = అతిపెద్ద_UINT64;
  }
#అయితే_ముగింపు
  ఒకవేళ( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    వెళ్ళు no_mem;
  }
  ప్రతిజ్ఞా( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_BUSY );
  testcase( p->rc!=SQLITE_OK );
  p->rc = SQLITE_OK;
  ప్రతిజ్ఞా( p->bIsReader || p->readOnly!=0 );
  p->i_Current_సమయం = 0;
  ప్రతిజ్ఞా( p->explain==0 );
  db->busyHandler.nBusy = 0;
  ఒకవేళ( Atomic_ఎక్కించు(&db->u1.isInterrupted) ) వెళ్ళు abort_due_గమ్యం_అంతరాయం;
  sqlite3VdbeIOTraceSql(p);
#నిర్వచితమైతే SQLITE_DEBUG
  sqlite3BeginBenignMalloc();
  ఒకవేళ( p->pc==0
   && (p->db->flags & (SQLITE_VdbeListing|SQLITE_VdbeEQP|SQLITE_VdbeTrace))!=0
  ){
    పూర్ణాంకం i;
    పూర్ణాంకం once = 1;
    sqlite3VdbePrintSql(p);
    ఒకవేళ( p->db->flags & SQLITE_VdbeListing ){
      రాయి("VDBE Program Listing:\n");
      కొరకు(i=0; i<p->nOp; i++){
        sqlite3VdbePrintOp(stdout, i, &aOp[i]);
      }
    }
    ఒకవేళ( p->db->flags & SQLITE_VdbeEQP ){
      కొరకు(i=0; i<p->nOp; i++){
        ఒకవేళ( aOp[i].opcode==OP_Explain ){
          ఒకవేళ( once ) రాయి("VDBE Query Plan:\n");
          రాయి("%s\n", aOp[i].p4.z);
          once = 0;
        }
      }
    }
    ఒకవేళ( p->db->flags & SQLITE_VdbeTrace )  రాయి("VDBE Trace:\n");
  }
  sqlite3_చివర_Benign_Malloc();
#అయితే_ముగింపు
  కొరకు(pOp=&aOp[p->pc]; 1; pOp++){
    /* Errors are detected by individual opcodes, with an immediate
    ** jumps to abort_due_to_error. */
    ప్రతిజ్ఞా( rc==SQLITE_OK );

    ప్రతిజ్ఞా( pOp>=aOp && pOp<&aOp[p->nOp]);
    n_Vm_అడుగు++;

#ఒకవేళ defined(VDBE_PROFILE)
    pOp->nExec++;
    pn_చక్రం = &pOp->n_చక్రం;
    ఒకవేళ( sqlite3NProfileCnt==0 ) *pn_చక్రం -= sqlite3Hwtime();
#లేదావేళ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
    ఒకవేళ( b_Stmt_పరిశీలన_స్థితి ){
      pOp->nExec++;
      pn_చక్రం = &pOp->n_చక్రం;
      *pn_చక్రం -= sqlite3Hwtime();
    }
#అయితే_ముగింపు

    /* Only allow tracing if SQLITE_DEBUG is defined.
    */
#నిర్వచితమైతే SQLITE_DEBUG
    ఒకవేళ( db->flags & SQLITE_VdbeTrace ){
      sqlite3VdbePrintOp(stdout, (పూర్ణాంకం)(pOp - aOp), pOp);
      test_trace_breakpoint((పూర్ణాంకం)(pOp - aOp),pOp,p);
    }
#అయితే_ముగింపు
     

    /* Check to see if we need to simulate an interrupt.  This only happens
    ** if we have a special test build.
    */
#నిర్వచితమైతే SQLITE_TEST
    ఒకవేళ( sqlite3_అంతరాయం_లెక్క>0 ){
      sqlite3_అంతరాయం_లెక్క--;
      ఒకవేళ( sqlite3_అంతరాయం_లెక్క==0 ){
        sqlite3_అంతరాయం(db);
      }
    }
#అయితే_ముగింపు

    /* Sanity checking on other operands */
#నిర్వచితమైతే SQLITE_DEBUG
    {
      u8 opProperty = sqlite3OpcodeProperty[pOp->opcode];
      ఒకవేళ( (opProperty & OPFLG_IN1)!=0 ){
        ప్రతిజ్ఞా( pOp->p1>0 );
        ప్రతిజ్ఞా( pOp->p1<=(p->nMem+1 - p->nCursor) );
        ప్రతిజ్ఞా( memIsValid(&aMem[pOp->p1]) );
        ప్రతిజ్ఞా( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p1]) );
        రిజిస్టరు_TRACE(pOp->p1, &aMem[pOp->p1]);
      }
      ఒకవేళ( (opProperty & OPFLG_IN2)!=0 ){
        ప్రతిజ్ఞా( pOp->p2>0 );
        ప్రతిజ్ఞా( pOp->p2<=(p->nMem+1 - p->nCursor) );
        ప్రతిజ్ఞా( memIsValid(&aMem[pOp->p2]) );
        ప్రతిజ్ఞా( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p2]) );
        రిజిస్టరు_TRACE(pOp->p2, &aMem[pOp->p2]);
      }
      ఒకవేళ( (opProperty & OPFLG_IN3)!=0 ){
        ప్రతిజ్ఞా( pOp->p3>0 );
        ప్రతిజ్ఞా( pOp->p3<=(p->nMem+1 - p->nCursor) );
        ప్రతిజ్ఞా( memIsValid(&aMem[pOp->p3]) );
        ప్రతిజ్ఞా( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p3]) );
        రిజిస్టరు_TRACE(pOp->p3, &aMem[pOp->p3]);
      }
      ఒకవేళ( (opProperty & OPFLG_OUT2)!=0 ){
        ప్రతిజ్ఞా( pOp->p2>0 );
        ప్రతిజ్ఞా( pOp->p2<=(p->nMem+1 - p->nCursor) );
        mem_About_గమ్యం_Change(p, &aMem[pOp->p2]);
      }
      ఒకవేళ( (opProperty & OPFLG_OUT3)!=0 ){
        ప్రతిజ్ఞా( pOp->p3>0 );
        ప్రతిజ్ఞా( pOp->p3<=(p->nMem+1 - p->nCursor) );
        mem_About_గమ్యం_Change(p, &aMem[pOp->p3]);
      }
    }
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
    pOrigOp = pOp;
#అయితే_ముగింపు
 
    ఎంపిక( pOp->opcode ){

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
స్థితి OP_Goto: {             /* jump */

#నిర్వచితమైతే SQLITE_DEBUG
  /* In debugging mode, when the p5 flags is set on an OP_Goto, that
  ** means we should really jump back to the preceding OP_ReleaseReg
  ** instruction. */
  ఒకవేళ( pOp->p5 ){
    ప్రతిజ్ఞా( pOp->p2 < (పూర్ణాంకం)(pOp - aOp) );
    ప్రతిజ్ఞా( pOp->p2 > 1 );
    pOp = &aOp[pOp->p2 - 2];
    ప్రతిజ్ఞా( pOp[1].opcode==OP_ReleaseReg );
    వెళ్ళు check_for_అంతరాయం;
  }
#అయితే_ముగింపు

jump_గమ్యం_p2_and_check_for_అంతరాయం:
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
check_for_అంతరాయం:
  ఒకవేళ( Atomic_ఎక్కించు(&db->u1.isInterrupted) ) వెళ్ళు abort_due_గమ్యం_అంతరాయం;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
  /* Call the progress callback if it is configured and the required number
  ** of VDBE ops have been executed (either since this invocation of
  ** sqlite3VdbeExec() or since last time the progress callback was called).
  ** If the progress callback returns non-zero, exit the virtual machine with
  ** a return code SQLITE_ABORT.
  */
  అయితే( n_Vm_అడుగు>=n_పురోగతి_హద్దు && db->x_పురోగతి!=0 ){
    ప్రతిజ్ఞా( db->n_పురోగతి_Ops!=0 );
    n_పురోగతి_హద్దు += db->n_పురోగతి_Ops;
    ఒకవేళ( db->x_పురోగతి(db->p_పురోగతి_Arg) ){
      n_పురోగతి_హద్దు = అతిపెద్ద_UINT64;
      rc = SQLITE_అంతరాయం;
      వెళ్ళు abort_due_గమ్యం_error;
    }
  }
#అయితే_ముగింపు
 
  ఆపు;
}

/* Opcode:  Gosub P1 P2 * * *
**
** Write the current address onto register P1
** and then jump to address P2.
*/
స్థితి OP_Gosub: {            /* jump */
  ప్రతిజ్ఞా( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( VdbeMemDynamic(pIn1)==0 );
  mem_About_గమ్యం_Change(p, pIn1);
  pIn1->flags = MEM_Int;
  pIn1->u.i = (పూర్ణాంకం)(pOp-aOp);
  రిజిస్టరు_TRACE(pOp->p1, pIn1);
  వెళ్ళు jump_గమ్యం_p2_and_check_for_అంతరాయం;
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
స్థితి OP_Return: {           /* in1 */
  pIn1 = &aMem[pOp->p1];
  ఒకవేళ( pIn1->flags & MEM_Int ){
    ఒకవేళ( pOp->p3 ){ Vdbe_Branch_తీసుకున్న(1, 2); }
    pOp = &aOp[pIn1->u.i];
  }లేకపోతే ఒకవేళ( ALWAYS(pOp->p3) ){
    Vdbe_Branch_తీసుకున్న(0, 2);
  }
  ఆపు;
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
స్థితి OP_InitCoroutine: {     /* jump0 */
  ప్రతిజ్ఞా( pOp->p1>0 &&  pOp->p1<=(p->nMem+1 - p->nCursor) );
  ప్రతిజ్ఞా( pOp->p2>=0 && pOp->p2<p->nOp );
  ప్రతిజ్ఞా( pOp->p3>=0 && pOp->p3<p->nOp );
  p_బహిర్గతం = &aMem[pOp->p1];
  ప్రతిజ్ఞా( !VdbeMemDynamic(p_బహిర్గతం) );
  p_బహిర్గతం->u.i = pOp->p3 - 1;
  p_బహిర్గతం->flags = MEM_Int;
  ఒకవేళ( pOp->p2==0 ) ఆపు;

  /* Most jump operations do a goto to this spot in order to update
  ** the pOp pointer. */
jump_గమ్యం_p2:
  ప్రతిజ్ఞా( pOp->p2>0 );       /* There are never any jumps to instruction 0 */
  ప్రతిజ్ఞా( pOp->p2<p->nOp );  /* Jumps must be in range */
  pOp = &aOp[pOp->p2 - 1];
  ఆపు;
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
స్థితి OP_చివర_Coroutine: {           /* in1 */
  VdbeOp *pCaller;
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( pIn1->flags==MEM_Int );
  ప్రతిజ్ఞా( pIn1->u.i>=0 && pIn1->u.i<p->nOp );
  pCaller = &aOp[pIn1->u.i];
  ప్రతిజ్ఞా( pCaller->opcode==OP_Yield );
  ప్రతిజ్ఞా( pCaller->p2>=0 && pCaller->p2<p->nOp );
  pIn1->u.i = (పూర్ణాంకం)(pOp - p->aOp) - 1;
  pOp = &aOp[pCaller->p2 - 1];
  ఆపు;
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
స్థితి OP_Yield: {            /* in1, jump0 */
  పూర్ణాంకం pcDest;
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( VdbeMemDynamic(pIn1)==0 );
  pIn1->flags = MEM_Int;
  pcDest = (పూర్ణాంకం)pIn1->u.i;
  pIn1->u.i = (పూర్ణాంకం)(pOp - aOp);
  రిజిస్టరు_TRACE(pOp->p1, pIn1);
  pOp = &aOp[pcDest];
  ఆపు;
}

/* Opcode:  HaltIfNull  P1 P2 P3 P4 P5
** Synopsis: if r[P3]=null halt
**
** Check the value in register P3.  If it is NULL then Halt using
** parameter P1, P2, and P4 as if this were a Halt instruction.  If the
** value in register P3 is not NULL, then this routine is a no-op.
** The P5 parameter should be 1.
*/
స్థితి OP_HaltIfNull: {      /* in3 */
  pIn3 = &aMem[pOp->p3];
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pOp->p2==OE_Abort ){ sqlite3VdbeAssertAbortable(p); }
#అయితే_ముగింపు
  ఒకవేళ( (pIn3->flags & MEM_Null)==0 ) ఆపు;
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
స్థితి OP_Halt: {
  Vdbe_చట్రం *p_చట్రం;
  పూర్ణాంకం pcx;

#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pOp->p2==OE_Abort ){ sqlite3VdbeAssertAbortable(p); }
#అయితే_ముగింపు
  ప్రతిజ్ఞా( pOp->p4type==P4_NOTUSED
       || pOp->p4type==P4_STATIC
       || pOp->p4type==P4_DYNAMIC );

  /* A deliberately coded "OP_Halt SQLITE_INTERNAL * * * *" opcode indicates
  ** something is wrong with the code generator.  Raise an assertion in order
  ** to bring this to the attention of fuzzers and other testing tools. */
  ప్రతిజ్ఞా( pOp->p1!=SQLITE_అంతర్గత );

  ఒకవేళ( p->p_చట్రం && pOp->p1==SQLITE_OK ){
    /* Halt the sub-program. Return control to the parent frame. */
    p_చట్రం = p->p_చట్రం;
    p->p_చట్రం = p_చట్రం->p_మాతృ;
    p->n_చట్రం--;
    sqlite3_Vdbe_సమితి_Changes(db, p->nChange);
    pcx = sqlite3_Vdbe_చట్రం_Restore(p_చట్రం);
    ఒకవేళ( pOp->p2==OE_Ignore ){
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
    ఆపు;
  }
  p->rc = pOp->p1;
  p->errorAction = (u8)pOp->p2;
  ప్రతిజ్ఞా( pOp->p5<=4 );
  ఒకవేళ( p->rc ){
    ఒకవేళ( pOp->p3>0 && pOp->p4type==P4_NOTUSED ){
      స్థిరరాశి అక్షరం *zErr;
      ప్రతిజ్ఞా( pOp->p3<=(p->nMem + 1 - p->nCursor) );
      zErr = sqlite3_విలువ_పాఠ్యం_2(&aMem[pOp->p3], SQLITE_UTF8);
      sqlite3VdbeError(p, "%s", zErr);
    }లేకపోతే ఒకవేళ( pOp->p5 ){
      స్థిర స్థిరరాశి అక్షరం * స్థిరరాశి azType[] = { "NOT NULL", "UNIQUE", "CHECK",
                                             "FOREIGN KEY" };
      testcase( pOp->p5==1 );
      testcase( pOp->p5==2 );
      testcase( pOp->p5==3 );
      testcase( pOp->p5==4 );
      sqlite3VdbeError(p, "%s constraint failed", azType[pOp->p5-1]);
      ఒకవేళ( pOp->p4.z ){
        p->z_Err_సందేశం = sqlite3MPrintf(db, "%z: %s", p->z_Err_సందేశం, pOp->p4.z);
      }
    }లేకపోతే{
      sqlite3VdbeError(p, "%s", pOp->p4.z);
    }
    sqlite3VdbeLogAbort(p, pOp->p1, pOp, aOp);
  }
  rc = sqlite3VdbeHalt(p);
  ప్రతిజ్ఞా( rc==SQLITE_BUSY || rc==SQLITE_OK || rc==SQLITE_ERROR );
  ఒకవేళ( rc==SQLITE_BUSY ){
    p->rc = SQLITE_BUSY;
  }లేకపోతే{
    ప్రతిజ్ఞా( rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT );
    ప్రతిజ్ఞా( rc==SQLITE_OK || db->nDeferredCons>0 || db->nDeferredImmCons>0 );
    rc = p->rc ? SQLITE_ERROR : SQLITE_పూర్తి;
  }
  వెళ్ళు vdbe_return;
}

/* Opcode: Integer P1 P2 * * *
** Synopsis: r[P2]=P1
**
** The 32-bit integer value P1 is written into register P2.
*/
స్థితి OP_Integer: {         /* out2 */
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->u.i = pOp->p1;
  ఆపు;
}

/* Opcode: Int64 P1 P2 P3 * *
** Synopsis: r[P2]=PINT13
**
** Combine P1 and P3 into a signed 64-bit integer.  P1 is the least
** significant 32 bits and P3 is the most significant.  Store the
** result in register P2.
*/
స్థితి OP_Int64: {           /* out2 */
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->u.i = INT32_గమ్యం_64(pOp->p1,pOp->p3);
  ఆపు;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
/* Opcode: Real P1 P2 P3 * *
** Synopsis: r[P2]=PDBL13
**
** P1 and P3 are combined to 64 bits with P1 being the lower the P3
** the upper.  The result is interpreted as a 64-bit floating-point
** and stored in register P2.
*/
స్థితి OP_Real: {            /* same as TK_FLOAT, out2 */
  u64 ii;
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->flags = MEM_Real;
  ii = (u64)INT32_గమ్యం_64(pOp->p1,pOp->p3);
  మార్పిడి_Mixed_Endian_Float(ii);
  జ్ఞాపకం_నకలు(&p_బహిర్గతం->u.r, &ii, 8);
  ప్రతిజ్ఞా( !sqlite3IsNaN(p_బహిర్గతం->u.r) );
  ఆపు;
}
#అయితే_ముగింపు

/* Opcode: String8 * P2 * P4 *
** Synopsis: r[P2]='P4'
**
** P4 points to a nul terminated UTF-8 string. This opcode is transformed
** into a String opcode before it is executed for the first time.  During
** this transformation, the length of string P4 is computed and stored
** as the P1 parameter.
*/
స్థితి OP_String8: {         /* same as TK_STRING, out2 */
  ప్రతిజ్ఞా( pOp->p4.z!=0 );
  p_బహిర్గతం = out2Prerelease(p, pOp);
  pOp->p1 = sqlite3Strlen30(pOp->p4.z);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  ఒకవేళ( encoding!=SQLITE_UTF8 ){
    rc = sqlite3_Vdbe_Mem_సమితి_Str(p_బహిర్గతం, pOp->p4.z, -1, SQLITE_UTF8, SQLITE_STATIC);
    ప్రతిజ్ఞా( rc==SQLITE_OK || rc==SQLITE_TOOBIG );
    ఒకవేళ( rc ) వెళ్ళు too_big;
    ఒకవేళ( SQLITE_OK!=sqlite3VdbeChangeEncoding(p_బహిర్గతం, encoding) ) వెళ్ళు no_mem;
    ప్రతిజ్ఞా( p_బహిర్గతం->szMalloc>0 && p_బహిర్గతం->zMalloc==p_బహిర్గతం->z );
    ప్రతిజ్ఞా( VdbeMemDynamic(p_బహిర్గతం)==0 );
    p_బహిర్గతం->szMalloc = 0;
    p_బహిర్గతం->flags |= MEM_Static;
    ఒకవేళ( pOp->p4type==P4_DYNAMIC ){
      sqlite3DbFree(db, pOp->p4.z);
    }
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = p_బహిర్గతం->z;
    pOp->p1 = p_బహిర్గతం->n;
  }
#అయితే_ముగింపు
  ఒకవేళ( pOp->p1>db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
    వెళ్ళు too_big;
  }
  pOp->opcode = OP_String;
  ప్రతిజ్ఞా( rc==SQLITE_OK );
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
స్థితి OP_String: {          /* out2 */
  ప్రతిజ్ఞా( pOp->p4.z!=0 );
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->flags = MEM_Str|MEM_Static|MEM_పదం;
  p_బహిర్గతం->z = pOp->p4.z;
  p_బహిర్గతం->n = pOp->p1;
  p_బహిర్గతం->enc = encoding;
  UPDATE_MAX_BLOBSIZE(p_బహిర్గతం);
#నిర్వచితం_కాకపోతే SQLITE_LIKE_DOESNT_సరిపోలిక_BLOBS
  ఒకవేళ( pOp->p3>0 ){
    ప్రతిజ్ఞా( pOp->p3<=(p->nMem+1 - p->nCursor) );
    pIn3 = &aMem[pOp->p3];
    ప్రతిజ్ఞా( pIn3->flags & MEM_Int );
    ఒకవేళ( pIn3->u.i==pOp->p5 ) p_బహిర్గతం->flags = MEM_Blob|MEM_Static|MEM_పదం;
  }
#అయితే_ముగింపు
  ఆపు;
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
స్థితి OP_BeginSubrtn:
స్థితి OP_Null: {           /* out2 */
  పూర్ణాంకం cnt;
  u16 nullFlag;
  p_బహిర్గతం = out2Prerelease(p, pOp);
  cnt = pOp->p3-pOp->p2;
  ప్రతిజ్ఞా( pOp->p3<=(p->nMem+1 - p->nCursor) );
  p_బహిర్గతం->flags = nullFlag = pOp->p1 ? (MEM_Null|MEM_Cleared) : MEM_Null;
  p_బహిర్గతం->n = 0;
#నిర్వచితమైతే SQLITE_DEBUG
  p_బహిర్గతం->u_తాత్కాలిక = 0;
#అయితే_ముగింపు
  అయితే( cnt>0 ){
    p_బహిర్గతం++;
    mem_About_గమ్యం_Change(p, p_బహిర్గతం);
    sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
    p_బహిర్గతం->flags = nullFlag;
    p_బహిర్గతం->n = 0;
    cnt--;
  }
  ఆపు;
}

/* Opcode: SoftNull P1 * * * *
** Synopsis: r[P1]=NULL
**
** Set register P1 to have the value NULL as seen by the OP_MakeRecord
** instruction, but do not free any string or blob memory associated with
** the register, so that if the value was a string or blob that was
** previously copied using OP_SCopy, the copies will continue to be valid.
*/
స్థితి OP_SoftNull: {
  ప్రతిజ్ఞా( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  p_బహిర్గతం = &aMem[pOp->p1];
  p_బహిర్గతం->flags = (p_బహిర్గతం->flags&~(MEM_Undefined|MEM_Aff_ముసుగు))|MEM_Null;
  ఆపు;
}

/* Opcode: Blob P1 P2 * P4 *
** Synopsis: r[P2]=P4 (len=P1)
**
** P4 points to a blob of data P1 bytes long.  Store this
** blob in register P2.  If P4 is a NULL pointer, then construct
** a zero-filled blob that is P1 bytes long in P2.
*/
స్థితి OP_Blob: {                /* out2 */
  ప్రతిజ్ఞా( pOp->p1 <= SQLITE_MAX_నిడివి );
  p_బహిర్గతం = out2Prerelease(p, pOp);
  ఒకవేళ( pOp->p4.z==0 ){
    sqlite3_Vdbe_Mem_సమితి_Zero_Blob(p_బహిర్గతం, pOp->p1);
    ఒకవేళ( sqlite3VdbeMemExpandBlob(p_బహిర్గతం) ) వెళ్ళు no_mem;
  }లేకపోతే{
    sqlite3_Vdbe_Mem_సమితి_Str(p_బహిర్గతం, pOp->p4.z, pOp->p1, 0, 0);
  }
  p_బహిర్గతం->enc = encoding;
  UPDATE_MAX_BLOBSIZE(p_బహిర్గతం);
  ఆపు;
}

/* Opcode: Variable P1 P2 * * *
** Synopsis: r[P2]=parameter(P1)
**
** Transfer the values of bound parameter P1 into register P2
*/
స్థితి OP_Variable: {            /* out2 */
  Mem *pVar;       /* Value being transferred */

  ప్రతిజ్ఞా( pOp->p1>0 && pOp->p1<=p->nVar );
  pVar = &p->aVar[pOp->p1 - 1];
  ఒకవేళ( sqlite3VdbeMemTooBig(pVar) ){
    వెళ్ళు too_big;
  }
  p_బహిర్గతం = &aMem[pOp->p2];
  ఒకవేళ( VdbeMemDynamic(p_బహిర్గతం) ) sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
  జ్ఞాపకం_నకలు(p_బహిర్గతం, pVar, MEMCELLSIZE);
  p_బహిర్గతం->flags &= ~(MEM_Dyn|MEM_Ephem);
  p_బహిర్గతం->flags |= MEM_Static|MEM_FromBind;
  UPDATE_MAX_BLOBSIZE(p_బహిర్గతం);
  ఆపు;
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
స్థితి OP_Move: {
  పూర్ణాంకం n;           /* Number of registers left to copy */
  పూర్ణాంకం p1;          /* Register to copy from */
  పూర్ణాంకం p2;          /* Register to copy to */

  n = pOp->p3;
  p1 = pOp->p1;
  p2 = pOp->p2;
  ప్రతిజ్ఞా( n>0 && p1>0 && p2>0 );
  ప్రతిజ్ఞా( p1+n<=p2 || p2+n<=p1 );

  pIn1 = &aMem[p1];
  p_బహిర్గతం = &aMem[p2];
  చేయి{
    ప్రతిజ్ఞా( p_బహిర్గతం<=&aMem[(p->nMem+1 - p->nCursor)] );
    ప్రతిజ్ఞా( pIn1<=&aMem[(p->nMem+1 - p->nCursor)] );
    ప్రతిజ్ఞా( memIsValid(pIn1) );
    mem_About_గమ్యం_Change(p, p_బహిర్గతం);
    sqlite3VdbeMemMove(p_బహిర్గతం, pIn1);
#నిర్వచితమైతే SQLITE_DEBUG
    pIn1->pScopyFrom = 0;
    { పూర్ణాంకం i;
      కొరకు(i=1; i<p->nMem; i++){
        ఒకవేళ( aMem[i].pScopyFrom==pIn1 ){
          ప్రతిజ్ఞా( aMem[i].bScopy );
          aMem[i].pScopyFrom = p_బహిర్గతం;
        }
      }
    }
#అయితే_ముగింపు
    Deephemeralize(p_బహిర్గతం);
    రిజిస్టరు_TRACE(p2++, p_బహిర్గతం);
    pIn1++;
    p_బహిర్గతం++;
  }అయితే( --n );
  ఆపు;
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
స్థితి OP_Copy: {
  పూర్ణాంకం n;

  n = pOp->p3;
  pIn1 = &aMem[pOp->p1];
  p_బహిర్గతం = &aMem[pOp->p2];
  ప్రతిజ్ఞా( p_బహిర్గతం!=pIn1 );
  అయితే( 1 ){
    mem_About_గమ్యం_Change(p, p_బహిర్గతం);
    sqlite3VdbeMemShallowCopy(p_బహిర్గతం, pIn1, MEM_Ephem);
    Deephemeralize(p_బహిర్గతం);
    ఒకవేళ( (p_బహిర్గతం->flags & MEM_Subtype)!=0 &&  (pOp->p5 & 0x0002)!=0 ){
      p_బహిర్గతం->flags &= ~MEM_Subtype;
    }
#నిర్వచితమైతే SQLITE_DEBUG
    p_బహిర్గతం->pScopyFrom = 0;
#అయితే_ముగింపు
    రిజిస్టరు_TRACE(pOp->p2+pOp->p3-n, p_బహిర్గతం);
    ఒకవేళ( (n--)==0 ) ఆపు;
    p_బహిర్గతం++;
    pIn1++;
  }
  ఆపు;
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
స్థితి OP_SCopy: {            /* out2 */
  pIn1 = &aMem[pOp->p1];
  p_బహిర్గతం = &aMem[pOp->p2];
  ప్రతిజ్ఞా( p_బహిర్గతం!=pIn1 );
  sqlite3VdbeMemShallowCopy(p_బహిర్గతం, pIn1, MEM_Ephem);
#నిర్వచితమైతే SQLITE_DEBUG
  p_బహిర్గతం->pScopyFrom = pIn1;
  p_బహిర్గతం->mScopyFlags = pIn1->flags;
  pIn1->bScopy = 1;
#అయితే_ముగింపు
  ఆపు;
}

/* Opcode: IntCopy P1 P2 * * *
** Synopsis: r[P2]=r[P1]
**
** Transfer the integer value held in register P1 into register P2.
**
** This is an optimized version of SCopy that works only for integer
** values.
*/
స్థితి OP_IntCopy: {            /* out2 */
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( (pIn1->flags & MEM_Int)!=0 );
  p_బహిర్గతం = &aMem[pOp->p2];
  sqlite3_Vdbe_Mem_సమితి_Int64(p_బహిర్గతం, pIn1->u.i);
  ఆపు;
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
స్థితి OP_FkCheck: {
  ఒకవేళ( (rc = sqlite3_Vdbe_Check_Fk_తక్షణ(p))!=SQLITE_OK ){
    వెళ్ళు abort_due_గమ్యం_error;
  }
  ఆపు;
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
స్థితి OP_ఫలితం_అడ్డువరుస: {
  ప్రతిజ్ఞా( p->n_Res_స్తంభం==pOp->p2 );
  ప్రతిజ్ఞా( pOp->p1>0 || CORRUPT_DB );
  ప్రతిజ్ఞా( pOp->p1+pOp->p2<=(p->nMem+1 - p->nCursor)+1 );

  p->cacheCtr = (p->cacheCtr + 2)|1;
  p->p_ఫలితం_అడ్డువరుస = &aMem[pOp->p1];
#నిర్వచితమైతే SQLITE_DEBUG
  {
    Mem *pMem = p->p_ఫలితం_అడ్డువరుస;
    పూర్ణాంకం i;
    కొరకు(i=0; i<pOp->p2; i++){
      ప్రతిజ్ఞా( memIsValid(&pMem[i]) );
      రిజిస్టరు_TRACE(pOp->p1+i, &pMem[i]);
      /* The registers in the result will not be used again when the
      ** prepared statement restarts.  This is because sqlite3_column()
      ** APIs might have caused type conversions of made other changes to
      ** the register values.  Therefore, we can go ahead and break any
      ** OP_SCopy dependencies. */
      pMem[i].pScopyFrom = 0;
    }
  }
#అయితే_ముగింపు
  ఒకవేళ( db->mallocFailed ) వెళ్ళు no_mem;
  ఒకవేళ( db->mTrace & SQLITE_TRACE_అడ్డువరుస ){
    db->trace.xV2(SQLITE_TRACE_అడ్డువరుస, db->pTraceArg, p, 0);
  }
  p->pc = (పూర్ణాంకం)(pOp - aOp) + 1;
  rc = SQLITE_అడ్డువరుస;
  వెళ్ళు vdbe_return;
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
స్థితి OP_Concat: {           /* same as TK_CONCAT, in1, in2, out3 */
  i64 nByte;          /* Total size of the output string or blob */
  u16 flags1;         /* Initial flags for P1 */
  u16 flags2;         /* Initial flags for P2 */

  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  p_బహిర్గతం = &aMem[pOp->p3];
  testcase( p_బహిర్గతం==pIn2 );
  ప్రతిజ్ఞా( pIn1!=p_బహిర్గతం );
  flags1 = pIn1->flags;
  testcase( flags1 & MEM_Null );
  testcase( pIn2->flags & MEM_Null );
  ఒకవేళ( (flags1 | pIn2->flags) & MEM_Null ){
    sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
    ఆపు;
  }
  ఒకవేళ( (flags1 & (MEM_Str|MEM_Blob))==0 ){
    ఒకవేళ( sqlite3VdbeMemStringify(pIn1,encoding,0) ) వెళ్ళు no_mem;
    flags1 = pIn1->flags & ~MEM_Str;
  }లేకపోతే ఒకవేళ( (flags1 & MEM_Zero)!=0 ){
    ఒకవేళ( sqlite3VdbeMemExpandBlob(pIn1) ) వెళ్ళు no_mem;
    flags1 = pIn1->flags & ~MEM_Str;
  }
  flags2 = pIn2->flags;
  ఒకవేళ( (flags2 & (MEM_Str|MEM_Blob))==0 ){
    ఒకవేళ( sqlite3VdbeMemStringify(pIn2,encoding,0) ) వెళ్ళు no_mem;
    flags2 = pIn2->flags & ~MEM_Str;
  }లేకపోతే ఒకవేళ( (flags2 & MEM_Zero)!=0 ){
    ఒకవేళ( sqlite3VdbeMemExpandBlob(pIn2) ) వెళ్ళు no_mem;
    flags2 = pIn2->flags & ~MEM_Str;
  }
  nByte = pIn1->n;
  nByte += pIn2->n;
  ఒకవేళ( nByte>db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
    వెళ్ళు too_big;
  }
#ఒకవేళ SQLITE_MAX_నిడివి>2147483645
  ఒకవేళ( nByte>2147483645 ){ వెళ్ళు too_big; }
#అయితే_ముగింపు
  ఒకవేళ( sqlite3VdbeMemGrow(p_బహిర్గతం, (పూర్ణాంకం)nByte+2, p_బహిర్గతం==pIn2) ){
    వెళ్ళు no_mem;
  }
  Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Str);
  ఒకవేళ( p_బహిర్గతం!=pIn2 ){
    జ్ఞాపకం_నకలు(p_బహిర్గతం->z, pIn2->z, pIn2->n);
    ప్రతిజ్ఞా( (pIn2->flags & MEM_Dyn) == (flags2 & MEM_Dyn) );
    pIn2->flags = flags2;
  }
  జ్ఞాపకం_నకలు(&p_బహిర్గతం->z[pIn2->n], pIn1->z, pIn1->n);
  ప్రతిజ్ఞా( (pIn1->flags & MEM_Dyn) == (flags1 & MEM_Dyn) );
  pIn1->flags = flags1;
  ఒకవేళ( encoding>SQLITE_UTF8 ) nByte &= ~1;
  p_బహిర్గతం->z[nByte]=0;
  p_బహిర్గతం->z[nByte+1] = 0;
  p_బహిర్గతం->flags |= MEM_పదం;
  p_బహిర్గతం->n = (పూర్ణాంకం)nByte;
  p_బహిర్గతం->enc = encoding;
  UPDATE_MAX_BLOBSIZE(p_బహిర్గతం);
  ఆపు;
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
స్థితి OP_కూడు:                   /* same as TK_PLUS, in1, in2, out3 */
స్థితి OP_వ్యవకలనం:              /* same as TK_MINUS, in1, in2, out3 */
స్థితి OP_Multiply:              /* same as TK_STAR, in1, in2, out3 */
స్థితి OP_Divide:                /* same as TK_SLASH, in1, in2, out3 */
స్థితి OP_Remainder: {           /* same as TK_REM, in1, in2, out3 */
  u16 type1;      /* Numeric type of left operand */
  u16 type2;      /* Numeric type of right operand */
  i64 iA;         /* Integer value of left operand */
  i64 iB;         /* Integer value of right operand */
  ద్విగుణ rA;      /* Real value of left operand */
  ద్విగుణ rB;      /* Real value of right operand */

  pIn1 = &aMem[pOp->p1];
  type1 = pIn1->flags;
  pIn2 = &aMem[pOp->p2];
  type2 = pIn2->flags;
  p_బహిర్గతం = &aMem[pOp->p3];
  ఒకవేళ( (type1 & type2 & MEM_Int)!=0 ){
int_math:
    iA = pIn1->u.i;
    iB = pIn2->u.i;
    ఎంపిక( pOp->opcode ){
      స్థితి OP_కూడు:       ఒకవేళ( sqlite3_కూడు_Int64(&iB,iA) ) వెళ్ళు fp_math;  ఆపు;
      స్థితి OP_వ్యవకలనం:  ఒకవేళ( sqlite3SubInt64(&iB,iA) ) వెళ్ళు fp_math;  ఆపు;
      స్థితి OP_Multiply:  ఒకవేళ( sqlite3MulInt64(&iB,iA) ) వెళ్ళు fp_math;  ఆపు;
      స్థితి OP_Divide: {
        ఒకవేళ( iA==0 ) వెళ్ళు arithmetic_ఫలితం_is_null;
        ఒకవేళ( iA==-1 && iB==అతిచిన్న_INT64 ) వెళ్ళు fp_math;
        iB /= iA;
        ఆపు;
      }
      అప్రమేయ: {
        ఒకవేళ( iA==0 ) వెళ్ళు arithmetic_ఫలితం_is_null;
        ఒకవేళ( iA==-1 ) iA = 1;
        iB %= iA;
        ఆపు;
      }
    }
    p_బహిర్గతం->u.i = iB;
    Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Int);
  }లేకపోతే ఒకవేళ( ((type1 | type2) & MEM_Null)!=0 ){
    వెళ్ళు arithmetic_ఫలితం_is_null;
  }లేకపోతే{
    type1 = numericType(pIn1);
    type2 = numericType(pIn2);
    ఒకవేళ( (type1 & type2 & MEM_Int)!=0 ) వెళ్ళు int_math;
fp_math:
    rA = sqlite3_Vdbe_Real_విలువ(pIn1);
    rB = sqlite3_Vdbe_Real_విలువ(pIn2);
    ఎంపిక( pOp->opcode ){
      స్థితి OP_కూడు:         rB += rA;       ఆపు;
      స్థితి OP_వ్యవకలనం:    rB -= rA;       ఆపు;
      స్థితి OP_Multiply:    rB *= rA;       ఆపు;
      స్థితి OP_Divide: {
        /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
        ఒకవేళ( rA==(ద్విగుణ)0 ) వెళ్ళు arithmetic_ఫలితం_is_null;
        rB /= rA;
        ఆపు;
      }
      అప్రమేయ: {
        iA = sqlite3_Vdbe_Int_విలువ(pIn1);
        iB = sqlite3_Vdbe_Int_విలువ(pIn2);
        ఒకవేళ( iA==0 ) వెళ్ళు arithmetic_ఫలితం_is_null;
        ఒకవేళ( iA==-1 ) iA = 1;
        rB = (ద్విగుణ)(iB % iA);
        ఆపు;
      }
    }
#నిర్వచితమైతే SQLITE_OMIT_FLOATING_బిందువు
    p_బహిర్గతం->u.i = rB;
    Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Int);
#లేకపోతే
    ఒకవేళ( sqlite3IsNaN(rB) ){
      వెళ్ళు arithmetic_ఫలితం_is_null;
    }
    p_బహిర్గతం->u.r = rB;
    Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Real);
#అయితే_ముగింపు
  }
  ఆపు;

arithmetic_ఫలితం_is_null:
  sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
  ఆపు;
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
స్థితి OP_CollSeq: {
  ప్రతిజ్ఞా( pOp->p4type==P4_COLLSEQ );
  ఒకవేళ( pOp->p1 ){
    sqlite3_Vdbe_Mem_సమితి_Int64(&aMem[pOp->p1], 0);
  }
  ఆపు;
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
స్థితి OP_బిట్టు_And:                 /* same as TK_BITAND, in1, in2, out3 */
స్థితి OP_బిట్టు_Or:                  /* same as TK_BITOR, in1, in2, out3 */
స్థితి OP_జరుపు_ఎడమ:              /* same as TK_LSHIFT, in1, in2, out3 */
స్థితి OP_జరుపు_కుడి: {           /* same as TK_RSHIFT, in1, in2, out3 */
  i64 iA;
  u64 uA;
  i64 iB;
  u8 op;

  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  p_బహిర్గతం = &aMem[pOp->p3];
  ఒకవేళ( (pIn1->flags | pIn2->flags) & MEM_Null ){
    sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
    ఆపు;
  }
  iA = sqlite3_Vdbe_Int_విలువ(pIn2);
  iB = sqlite3_Vdbe_Int_విలువ(pIn1);
  op = pOp->opcode;
  ఒకవేళ( op==OP_బిట్టు_And ){
    iA &= iB;
  }లేకపోతే ఒకవేళ( op==OP_బిట్టు_Or ){
    iA |= iB;
  }లేకపోతే ఒకవేళ( iB!=0 ){
    ప్రతిజ్ఞా( op==OP_జరుపు_కుడి || op==OP_జరుపు_ఎడమ );

    /* If shifting by a negative amount, shift in the other direction */
    ఒకవేళ( iB<0 ){
      ప్రతిజ్ఞా( OP_జరుపు_కుడి==OP_జరుపు_ఎడమ+1 );
      op = 2*OP_జరుపు_ఎడమ + 1 - op;
      iB = iB>(-64) ? -iB : 64;
    }

    ఒకవేళ( iB>=64 ){
      iA = (iA>=0 || op==OP_జరుపు_ఎడమ) ? 0 : -1;
    }లేకపోతే{
      జ్ఞాపకం_నకలు(&uA, &iA, పరిమాణం(uA));
      ఒకవేళ( op==OP_జరుపు_ఎడమ ){
        uA <<= iB;
      }లేకపోతే{
        uA >>= iB;
        /* Sign-extend on a right shift of a negative number */
        ఒకవేళ( iA<0 ) uA |= ((((u64)0xffffffff)<<32)|0xffffffff) << (64-iB);
      }
      జ్ఞాపకం_నకలు(&iA, &uA, పరిమాణం(iA));
    }
  }
  p_బహిర్గతం->u.i = iA;
  Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Int);
  ఆపు;
}

/* Opcode: AddImm  P1 P2 * * *
** Synopsis: r[P1]=r[P1]+P2
**
** Add the constant P2 to the value in register P1.
** The result is always an integer.
**
** To force any register to be an integer, just add 0.
*/
స్థితి OP_కూడు_Imm: {            /* in1 */
  pIn1 = &aMem[pOp->p1];
  mem_About_గమ్యం_Change(p, pIn1);
  sqlite3VdbeMemIntegerify(pIn1);
  *(u64*)&pIn1->u.i += (u64)pOp->p2;
  ఆపు;
}

/* Opcode: MustBeInt P1 P2 * * *
**
** Force the value in register P1 to be an integer.  If the value
** in P1 is not an integer and cannot be converted into an integer
** without data loss, then jump immediately to P2, or if P2==0
** raise an SQLITE_MISMATCH exception.
*/
స్థితి OP_MustBeInt: {            /* jump0, in1 */
  pIn1 = &aMem[pOp->p1];
  ఒకవేళ( (pIn1->flags & MEM_Int)==0 ){
    ఒకవేళ( (pIn1->flags & MEM_Real)!=0 ){
      sqlite3_int64 ix;
      ఒకవేళ( pIn1->u.r>=-9223372036854775808.0
       && pIn1->u.r<+9223372036854775808.0
       && (ద్విగుణ)(ix = sqlite3_Real_గమ్యం_I64(pIn1->u.r))==pIn1->u.r
      ){
        pIn1->u.i = ix;
        Mem_సమితి_Type_Flag(pIn1, MEM_Int);
      }
    }లేకపోతే{
      applyAffinity(pIn1, SQLITE_AFF_NUMERIC, encoding);
    }
    ఒకవేళ( (pIn1->flags & MEM_Int)==0 ){
      Vdbe_Branch_తీసుకున్న(1, 2);
      ఒకవేళ( pOp->p2==0 ){
        rc = SQLITE_MISMATCH;
        వెళ్ళు abort_due_గమ్యం_error;
      }లేకపోతే{
        వెళ్ళు jump_గమ్యం_p2;
      }
    }
  }
  Vdbe_Branch_తీసుకున్న(0, 2);
  Mem_సమితి_Type_Flag(pIn1, MEM_Int);
  ఆపు;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
/* Opcode: RealAffinity P1 * * * *
**
** If register P1 holds an integer convert it to a real value.
**
** This opcode is used when extracting information from a column that
** has REAL affinity.  Such column values may still be stored as
** integers, for space efficiency, but after extraction we want them
** to have only a real value.
*/
స్థితి OP_RealAffinity: {                  /* in1 */
  pIn1 = &aMem[pOp->p1];
  ఒకవేళ( pIn1->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pIn1->flags & MEM_Int );
    testcase( pIn1->flags & MEM_IntReal );
    sqlite3VdbeMemRealify(pIn1);
    రిజిస్టరు_TRACE(pOp->p1, pIn1);
  }
  ఆపు;
}
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_CAST) || !defined(SQLITE_OMIT_ANALYZE)
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
స్థితి OP_Cast: {                  /* in1 */
  ప్రతిజ్ఞా( pOp->p2>=SQLITE_AFF_BLOB && pOp->p2<=SQLITE_AFF_REAL );
  testcase( pOp->p2==SQLITE_AFF_పాఠ్యం );
  testcase( pOp->p2==SQLITE_AFF_BLOB );
  testcase( pOp->p2==SQLITE_AFF_NUMERIC );
  testcase( pOp->p2==SQLITE_AFF_INTEGER );
  testcase( pOp->p2==SQLITE_AFF_REAL );
  pIn1 = &aMem[pOp->p1];
  mem_About_గమ్యం_Change(p, pIn1);
  rc = ExpandBlob(pIn1);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  rc = sqlite3VdbeMemCast(pIn1, pOp->p2, encoding);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  UPDATE_MAX_BLOBSIZE(pIn1);
  రిజిస్టరు_TRACE(pOp->p1, pIn1);
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_CAST */

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
స్థితి OP_Eq:               /* same as TK_EQ, jump, in1, in3 */
స్థితి OP_Ne:               /* same as TK_NE, jump, in1, in3 */
స్థితి OP_Lt:               /* same as TK_LT, jump, in1, in3 */
స్థితి OP_Le:               /* same as TK_LE, jump, in1, in3 */
స్థితి OP_Gt:               /* same as TK_GT, jump, in1, in3 */
స్థితి OP_Ge: {             /* same as TK_GE, jump, in1, in3 */
  పూర్ణాంకం res, res2;      /* Result of the comparison of pIn1 against pIn3 */
  అక్షరం affinity;      /* Affinity to use for comparison */
  u16 flags1;         /* Copy of initial value of pIn1->flags */
  u16 flags3;         /* Copy of initial value of pIn3->flags */

  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  flags1 = pIn1->flags;
  flags3 = pIn3->flags;
  ఒకవేళ( (flags1 & flags3 & MEM_Int)!=0 ){
    /* Common case of comparison of two integers */
    ఒకవేళ( pIn3->u.i > pIn1->u.i ){
      ఒకవేళ( sqlite3aGTb[pOp->opcode] ){
        Vdbe_Branch_తీసుకున్న(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        వెళ్ళు jump_గమ్యం_p2;
      }
      iCompare = +1;
      VVA_ONLY( iCompareIsInit = 1; )
    }లేకపోతే ఒకవేళ( pIn3->u.i < pIn1->u.i ){
      ఒకవేళ( sqlite3aLTb[pOp->opcode] ){
        Vdbe_Branch_తీసుకున్న(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        వెళ్ళు jump_గమ్యం_p2;
      }
      iCompare = -1;
      VVA_ONLY( iCompareIsInit = 1; )
    }లేకపోతే{
      ఒకవేళ( sqlite3aEQb[pOp->opcode] ){
        Vdbe_Branch_తీసుకున్న(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        వెళ్ళు jump_గమ్యం_p2;
      }
      iCompare = 0;
      VVA_ONLY( iCompareIsInit = 1; )
    }
    Vdbe_Branch_తీసుకున్న(0, (pOp->p5 & SQLITE_NULLEQ)?2:3);
    ఆపు;
  }
  ఒకవేళ( (flags1 | flags3)&MEM_Null ){
    /* One or both operands are NULL */
    ఒకవేళ( pOp->p5 & SQLITE_NULLEQ ){
      /* If SQLITE_NULLEQ is set (which will only happen if the operator is
      ** OP_Eq or OP_Ne) then take the jump or not depending on whether
      ** or not both operands are null.
      */
      ప్రతిజ్ఞా( (flags1 & MEM_Cleared)==0 );
      ప్రతిజ్ఞా( (pOp->p5 & SQLITE_JUMPIFNULL)==0 || CORRUPT_DB );
      testcase( (pOp->p5 & SQLITE_JUMPIFNULL)!=0 );
      ఒకవేళ( (flags1&flags3&MEM_Null)!=0
       && (flags3&MEM_Cleared)==0
      ){
        res = 0;  /* Operands are equal */
      }లేకపోతే{
        res = ((flags3 & MEM_Null) ? -1 : +1);  /* Operands are not equal */
      }
    }లేకపోతే{
      /* SQLITE_NULLEQ is clear and at least one operand is NULL,
      ** then the result is always NULL.
      ** The jump is taken if the SQLITE_JUMPIFNULL bit is set.
      */
      Vdbe_Branch_తీసుకున్న(2,3);
      ఒకవేళ( pOp->p5 & SQLITE_JUMPIFNULL ){
        వెళ్ళు jump_గమ్యం_p2;
      }
      iCompare = 1;    /* Operands are not equal */
      VVA_ONLY( iCompareIsInit = 1; )
      ఆపు;
    }
  }లేకపోతే{
    /* Neither operand is NULL and we couldn't do the special high-speed
    ** integer comparison case.  So do a general-case comparison. */
    affinity = pOp->p5 & SQLITE_AFF_ముసుగు;
    ఒకవేళ( affinity>=SQLITE_AFF_NUMERIC ){
      ఒకవేళ( (flags1 | flags3)&MEM_Str ){
        ఒకవేళ( (flags1 & (MEM_Int|MEM_IntReal|MEM_Real|MEM_Str))==MEM_Str ){
          applyNumericAffinity(pIn1,0);
          ప్రతిజ్ఞా( flags3==pIn3->flags || CORRUPT_DB );
          flags3 = pIn3->flags;
        }
        ఒకవేళ( (flags3 & (MEM_Int|MEM_IntReal|MEM_Real|MEM_Str))==MEM_Str ){
          applyNumericAffinity(pIn3,0);
        }
      }
    }లేకపోతే ఒకవేళ( affinity==SQLITE_AFF_పాఠ్యం && ((flags1 | flags3) & MEM_Str)!=0 ){
      ఒకవేళ( (flags1 & MEM_Str)!=0 ){
        pIn1->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
      }లేకపోతే ఒకవేళ( (flags1&(MEM_Int|MEM_Real|MEM_IntReal))!=0 ){
        testcase( pIn1->flags & MEM_Int );
        testcase( pIn1->flags & MEM_Real );
        testcase( pIn1->flags & MEM_IntReal );
        sqlite3VdbeMemStringify(pIn1, encoding, 1);
        testcase( (flags1&MEM_Dyn) != (pIn1->flags&MEM_Dyn) );
        flags1 = (pIn1->flags & ~MEM_Type_ముసుగు) | (flags1 & MEM_Type_ముసుగు);
        ఒకవేళ( NEVER(pIn1==pIn3) ) flags3 = flags1 | MEM_Str;
      }
      ఒకవేళ( (flags3 & MEM_Str)!=0 ){
        pIn3->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
      }లేకపోతే ఒకవేళ( (flags3&(MEM_Int|MEM_Real|MEM_IntReal))!=0 ){
        testcase( pIn3->flags & MEM_Int );
        testcase( pIn3->flags & MEM_Real );
        testcase( pIn3->flags & MEM_IntReal );
        sqlite3VdbeMemStringify(pIn3, encoding, 1);
        testcase( (flags3&MEM_Dyn) != (pIn3->flags&MEM_Dyn) );
        flags3 = (pIn3->flags & ~MEM_Type_ముసుగు) | (flags3 & MEM_Type_ముసుగు);
      }
    }
    ప్రతిజ్ఞా( pOp->p4type==P4_COLLSEQ || pOp->p4.pColl==0 );
    res = sqlite3MemCompare(pIn3, pIn1, pOp->p4.pColl);
  }

  /* At this point, res is negative, zero, or positive if reg[P1] is
  ** less than, equal to, or greater than reg[P3], respectively.  Compute
  ** the answer to this operator in res2, depending on what the comparison
  ** operator actually is.  The next block of code depends on the fact
  ** that the 6 comparison operators are consecutive integers in this
  ** order:  NE, EQ, GT, LE, LT, GE */
  ప్రతిజ్ఞా( OP_Eq==OP_Ne+1 ); ప్రతిజ్ఞా( OP_Gt==OP_Ne+2 ); ప్రతిజ్ఞా( OP_Le==OP_Ne+3 );
  ప్రతిజ్ఞా( OP_Lt==OP_Ne+4 ); ప్రతిజ్ఞా( OP_Ge==OP_Ne+5 );
  ఒకవేళ( res<0 ){
    res2 = sqlite3aLTb[pOp->opcode];
  }లేకపోతే ఒకవేళ( res==0 ){
    res2 = sqlite3aEQb[pOp->opcode];
  }లేకపోతే{
    res2 = sqlite3aGTb[pOp->opcode];
  }
  iCompare = res;
  VVA_ONLY( iCompareIsInit = 1; )

  /* Undo any changes made by applyAffinity() to the input registers. */
  ప్రతిజ్ఞా( (pIn3->flags & MEM_Dyn) == (flags3 & MEM_Dyn) );
  pIn3->flags = flags3;
  ప్రతిజ్ఞా( (pIn1->flags & MEM_Dyn) == (flags1 & MEM_Dyn) );
  pIn1->flags = flags1;

  Vdbe_Branch_తీసుకున్న(res2!=0, (pOp->p5 & SQLITE_NULLEQ)?2:3);
  ఒకవేళ( res2 ){
    వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
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
స్థితి OP_ElseEq: {       /* same as TK_ESCAPE, jump */

#నిర్వచితమైతే SQLITE_DEBUG
  /* Verify the preconditions of this opcode - that it follows an OP_Lt or
  ** OP_Gt with zero or more intervening OP_ReleaseReg opcodes */
  పూర్ణాంకం iAddr;
  కొరకు(iAddr = (పూర్ణాంకం)(pOp - aOp) - 1; ALWAYS(iAddr>=0); iAddr--){
    ఒకవేళ( aOp[iAddr].opcode==OP_ReleaseReg ) కొనసాగు;
    ప్రతిజ్ఞా( aOp[iAddr].opcode==OP_Lt || aOp[iAddr].opcode==OP_Gt );
    ఆపు;
  }
#అయితే_ముగింపు /* SQLITE_DEBUG */
  ప్రతిజ్ఞా( iCompareIsInit );
  Vdbe_Branch_తీసుకున్న(iCompare==0, 2);
  ఒకవేళ( iCompare==0 ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
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
స్థితి OP_Permutation: {
  ప్రతిజ్ఞా( pOp->p4type==P4_INTARRAY );
  ప్రతిజ్ఞా( pOp->p4.ai );
  ప్రతిజ్ఞా( pOp[1].opcode==OP_Compare );
  ప్రతిజ్ఞా( pOp[1].p5 & OPFLAG_క్రమమార్పు );
  ఆపు;
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
స్థితి OP_Compare: {
  పూర్ణాంకం n;
  పూర్ణాంకం i;
  పూర్ణాంకం p1;
  పూర్ణాంకం p2;
  స్థిరరాశి కీలకం_Info *p_కీలకం_Info;
  u32 idx;
  CollSeq *pColl;    /* Collating sequence to use on this term */
  పూర్ణాంకం bRev;          /* True for DESCENDING sort order */
  u32 *a_క్రమమార్పు;     /* The permutation */

  ఒకవేళ( (pOp->p5 & OPFLAG_క్రమమార్పు)==0 ){
    a_క్రమమార్పు = 0;
  }లేకపోతే{
    ప్రతిజ్ఞా( pOp>aOp );
    ప్రతిజ్ఞా( pOp[-1].opcode==OP_Permutation );
    ప్రతిజ్ఞా( pOp[-1].p4type==P4_INTARRAY );
    a_క్రమమార్పు = pOp[-1].p4.ai + 1;
    ప్రతిజ్ఞా( a_క్రమమార్పు!=0 );
  }
  n = pOp->p3;
  p_కీలకం_Info = pOp->p4.p_కీలకం_Info;
  ప్రతిజ్ఞా( n>0 );
  ప్రతిజ్ఞా( p_కీలకం_Info!=0 );
  ప్రతిజ్ఞా( p_కీలకం_Info->a_క్రమీకరించు_Flags!=0 );
  p1 = pOp->p1;
  p2 = pOp->p2;
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( a_క్రమమార్పు ){
    పూర్ణాంకం k, mx = 0;
    కొరకు(k=0; k<n; k++) ఒకవేళ( a_క్రమమార్పు[k]>(u32)mx ) mx = a_క్రమమార్పు[k];
    ప్రతిజ్ఞా( p1>0 && p1+mx<=(p->nMem+1 - p->nCursor)+1 );
    ప్రతిజ్ఞా( p2>0 && p2+mx<=(p->nMem+1 - p->nCursor)+1 );
  }లేకపోతే{
    ప్రతిజ్ఞా( p1>0 && p1+n<=(p->nMem+1 - p->nCursor)+1 );
    ప్రతిజ్ఞా( p2>0 && p2+n<=(p->nMem+1 - p->nCursor)+1 );
  }
#అయితే_ముగింపు /* SQLITE_DEBUG */
  కొరకు(i=0; i<n; i++){
    idx = a_క్రమమార్పు ? a_క్రమమార్పు[i] : (u32)i;
    ప్రతిజ్ఞా( memIsValid(&aMem[p1+idx]) );
    ప్రతిజ్ఞా( memIsValid(&aMem[p2+idx]) );
    రిజిస్టరు_TRACE(p1+idx, &aMem[p1+idx]);
    రిజిస్టరు_TRACE(p2+idx, &aMem[p2+idx]);
    ప్రతిజ్ఞా( i<p_కీలకం_Info->n_కీలకం_క్షేత్రం );
    pColl = p_కీలకం_Info->aColl[i];
    bRev = (p_కీలకం_Info->a_క్రమీకరించు_Flags[i] & KEYINFO_క్రమం_DESC);
    iCompare = sqlite3MemCompare(&aMem[p1+idx], &aMem[p2+idx], pColl);
    VVA_ONLY( iCompareIsInit = 1; )
    ఒకవేళ( iCompare ){
      ఒకవేళ( (p_కీలకం_Info->a_క్రమీకరించు_Flags[i] & KEYINFO_క్రమం_BIGNULL)
       && ((aMem[p1+idx].flags & MEM_Null) || (aMem[p2+idx].flags & MEM_Null))
      ){
        iCompare = -iCompare;
      }
      ఒకవేళ( bRev ) iCompare = -iCompare;
      ఆపు;
    }
  }
  ప్రతిజ్ఞా( pOp[1].opcode==OP_Jump );
  ఆపు;
}

/* Opcode: Jump P1 P2 P3 * *
**
** Jump to the instruction at address P1, P2, or P3 depending on whether
** in the most recent OP_Compare instruction the P1 vector was less than,
** equal to, or greater than the P2 vector, respectively.
**
** This opcode must immediately follow an OP_Compare opcode.
*/
స్థితి OP_Jump: {             /* jump */
  ప్రతిజ్ఞా( pOp>aOp && pOp[-1].opcode==OP_Compare );
  ప్రతిజ్ఞా( iCompareIsInit );
  ఒకవేళ( iCompare<0 ){
    Vdbe_Branch_తీసుకున్న(0,4); pOp = &aOp[pOp->p1 - 1];
  }లేకపోతే ఒకవేళ( iCompare==0 ){
    Vdbe_Branch_తీసుకున్న(1,4); pOp = &aOp[pOp->p2 - 1];
  }లేకపోతే{
    Vdbe_Branch_తీసుకున్న(2,4); pOp = &aOp[pOp->p3 - 1];
  }
  ఆపు;
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
స్థితి OP_And:              /* same as TK_AND, in1, in2, out3 */
స్థితి OP_Or: {             /* same as TK_OR, in1, in2, out3 */
  పూర్ణాంకం v1;    /* Left operand:  0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */
  పూర్ణాంకం v2;    /* Right operand: 0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */

  v1 = sqlite3_Vdbe_Boolean_విలువ(&aMem[pOp->p1], 2);
  v2 = sqlite3_Vdbe_Boolean_విలువ(&aMem[pOp->p2], 2);
  ఒకవేళ( pOp->opcode==OP_And ){
    స్థిర స్థిరరాశి గుర్తులేని అక్షరం and_logic[] = { 0, 0, 0, 0, 1, 2, 0, 2, 2 };
    v1 = and_logic[v1*3+v2];
  }లేకపోతే{
    స్థిర స్థిరరాశి గుర్తులేని అక్షరం or_logic[] = { 0, 1, 2, 1, 1, 1, 2, 1, 2 };
    v1 = or_logic[v1*3+v2];
  }
  p_బహిర్గతం = &aMem[pOp->p3];
  ఒకవేళ( v1==2 ){
    Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Null);
  }లేకపోతే{
    p_బహిర్గతం->u.i = v1;
    Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Int);
  }
  ఆపు;
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
స్థితి OP_IsTrue: {               /* in1, out2 */
  ప్రతిజ్ఞా( pOp->p4type==P4_INT32 );
  ప్రతిజ్ఞా( pOp->p4.i==0 || pOp->p4.i==1 );
  ప్రతిజ్ఞా( pOp->p3==0 || pOp->p3==1 );
  sqlite3_Vdbe_Mem_సమితి_Int64(&aMem[pOp->p2],
      sqlite3_Vdbe_Boolean_విలువ(&aMem[pOp->p1], pOp->p3) ^ pOp->p4.i);
  ఆపు;
}

/* Opcode: Not P1 P2 * * *
** Synopsis: r[P2]= !r[P1]
**
** Interpret the value in register P1 as a boolean value.  Store the
** boolean complement in register P2.  If the value in register P1 is
** NULL, then a NULL is stored in P2.
*/
స్థితి OP_Not: {                /* same as TK_NOT, in1, out2 */
  pIn1 = &aMem[pOp->p1];
  p_బహిర్గతం = &aMem[pOp->p2];
  ఒకవేళ( (pIn1->flags & MEM_Null)==0 ){
    sqlite3_Vdbe_Mem_సమితి_Int64(p_బహిర్గతం, !sqlite3_Vdbe_Boolean_విలువ(pIn1,0));
  }లేకపోతే{
    sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
  }
  ఆపు;
}

/* Opcode: BitNot P1 P2 * * *
** Synopsis: r[P2]= ~r[P1]
**
** Interpret the content of register P1 as an integer.  Store the
** ones-complement of the P1 value into register P2.  If P1 holds
** a NULL then store a NULL in P2.
*/
స్థితి OP_బిట్టు_Not: {             /* same as TK_BITNOT, in1, out2 */
  pIn1 = &aMem[pOp->p1];
  p_బహిర్గతం = &aMem[pOp->p2];
  sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
  ఒకవేళ( (pIn1->flags & MEM_Null)==0 ){
    p_బహిర్గతం->flags = MEM_Int;
    p_బహిర్గతం->u.i = ~sqlite3_Vdbe_Int_విలువ(pIn1);
  }
  ఆపు;
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
స్థితి OP_Once: {             /* jump */
  u32 iAddr;                /* Address of this instruction */
  ప్రతిజ్ఞా( p->aOp[0].opcode==OP_Init );
  ఒకవేళ( p->p_చట్రం ){
    iAddr = (పూర్ణాంకం)(pOp - p->aOp);
    ఒకవేళ( (p->p_చట్రం->aOnce[iAddr/8] & (1<<(iAddr & 7)))!=0 ){
      Vdbe_Branch_తీసుకున్న(1, 2);
      వెళ్ళు jump_గమ్యం_p2;
    }
    p->p_చట్రం->aOnce[iAddr/8] |= 1<<(iAddr & 7);
  }లేకపోతే{
    ఒకవేళ( p->aOp[0].p1==pOp->p1 ){
      Vdbe_Branch_తీసుకున్న(1, 2);
      వెళ్ళు jump_గమ్యం_p2;
    }
  }
  Vdbe_Branch_తీసుకున్న(0, 2);
  pOp->p1 = p->aOp[0].p1;
  ఆపు;
}

/* Opcode: If P1 P2 P3 * *
**
** Jump to P2 if the value in register P1 is true.  The value
** is considered true if it is numeric and non-zero.  If the value
** in P1 is NULL then take the jump if and only if P3 is non-zero.
*/
స్థితి OP_If:  {               /* jump, in1 */
  పూర్ణాంకం c;
  c = sqlite3_Vdbe_Boolean_విలువ(&aMem[pOp->p1], pOp->p3);
  Vdbe_Branch_తీసుకున్న(c!=0, 2);
  ఒకవేళ( c ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
}

/* Opcode: IfNot P1 P2 P3 * *
**
** Jump to P2 if the value in register P1 is False.  The value
** is considered false if it has a numeric value of zero.  If the value
** in P1 is NULL then take the jump if and only if P3 is non-zero.
*/
స్థితి OP_IfNot: {            /* jump, in1 */
  పూర్ణాంకం c;
  c = !sqlite3_Vdbe_Boolean_విలువ(&aMem[pOp->p1], !pOp->p3);
  Vdbe_Branch_తీసుకున్న(c!=0, 2);
  ఒకవేళ( c ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
}

/* Opcode: IsNull P1 P2 * * *
** Synopsis: if r[P1]==NULL goto P2
**
** Jump to P2 if the value in register P1 is NULL.
*/
స్థితి OP_IsNull: {            /* same as TK_ISNULL, jump, in1 */
  pIn1 = &aMem[pOp->p1];
  Vdbe_Branch_తీసుకున్న( (pIn1->flags & MEM_Null)!=0, 2);
  ఒకవేళ( (pIn1->flags & MEM_Null)!=0 ){
    వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
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
స్థితి OP_IsType: {        /* jump */
  VdbeCursor *pC;
  u16 type_ముసుగు;
  u32 serialType;

  ప్రతిజ్ఞా( pOp->p1>=(-1) && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p1>=0 || (pOp->p3>=0 && pOp->p3<=(p->nMem+1 - p->nCursor)) );
  ఒకవేళ( pOp->p1>=0 ){
    pC = p->apCsr[pOp->p1];
    ప్రతిజ్ఞా( pC!=0 );
    ప్రతిజ్ఞా( pOp->p3>=0 );
    ఒకవేళ( pOp->p3<pC->nHdrParsed ){
      serialType = pC->aType[pOp->p3];
      ఒకవేళ( serialType>=12 ){
        ఒకవేళ( serialType&1 ){
          type_ముసుగు = 0x04;   /* SQLITE_TEXT */
        }లేకపోతే{
          type_ముసుగు = 0x08;   /* SQLITE_BLOB */
        }
      }లేకపోతే{
        స్థిర స్థిరరాశి గుర్తులేని అక్షరం a_ముసుగు[] = {
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
        type_ముసుగు = a_ముసుగు[serialType];
      }
    }లేకపోతే{
      type_ముసుగు = 1 << (pOp->p4.i - 1);
      testcase( type_ముసుగు==0x01 );
      testcase( type_ముసుగు==0x02 );
      testcase( type_ముసుగు==0x04 );
      testcase( type_ముసుగు==0x08 );
      testcase( type_ముసుగు==0x10 );
    }
  }లేకపోతే{
    ప్రతిజ్ఞా( memIsValid(&aMem[pOp->p3]) );
    type_ముసుగు = 1 << (sqlite3_విలువ_type((sqlite3_విలువ*)&aMem[pOp->p3])-1);
    testcase( type_ముసుగు==0x01 );
    testcase( type_ముసుగు==0x02 );
    testcase( type_ముసుగు==0x04 );
    testcase( type_ముసుగు==0x08 );
    testcase( type_ముసుగు==0x10 );
  }
  Vdbe_Branch_తీసుకున్న( (type_ముసుగు & pOp->p5)!=0, 2);
  ఒకవేళ( type_ముసుగు & pOp->p5 ){
    వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
}

/* Opcode: ZeroOrNull P1 P2 P3 * *
** Synopsis: r[P2] = 0 OR NULL
**
** If both registers P1 and P3 are NOT NULL, then store a zero in
** register P2.  If either registers P1 or P3 are NULL then put
** a NULL in register P2.
*/
స్థితి OP_ZeroOrNull: {            /* in1, in2, out2, in3 */
  ఒకవేళ( (aMem[pOp->p1].flags & MEM_Null)!=0
   || (aMem[pOp->p3].flags & MEM_Null)!=0
  ){
    sqlite3_Vdbe_Mem_సమితి_Null(aMem + pOp->p2);
  }లేకపోతే{
    sqlite3_Vdbe_Mem_సమితి_Int64(aMem + pOp->p2, 0);
  }
  ఆపు;
}

/* Opcode: NotNull P1 P2 * * *
** Synopsis: if r[P1]!=NULL goto P2
**
** Jump to P2 if the value in register P1 is not NULL. 
*/
స్థితి OP_NotNull: {            /* same as TK_NOTNULL, jump, in1 */
  pIn1 = &aMem[pOp->p1];
  Vdbe_Branch_తీసుకున్న( (pIn1->flags & MEM_Null)==0, 2);
  ఒకవేళ( (pIn1->flags & MEM_Null)==0 ){
    వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
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
స్థితి OP_If_Null_అడ్డువరుస: {         /* jump */
  VdbeCursor *pC;
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ఒకవేళ( pC && pC->null_అడ్డువరుస ){
    sqlite3_Vdbe_Mem_సమితి_Null(aMem + pOp->p3);
    వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
}

#నిర్వచితమైతే SQLITE_ENABLE_స్థానభ్రంశం_SQL_FUNC
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
స్థితి OP_స్థానభ్రంశం: {          /* out3 */
  VdbeCursor *pC;    /* The VDBE cursor */
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  p_బహిర్గతం = &p->aMem[pOp->p3];
  ఒకవేళ( pC==0 || pC->eCurType!=CURTYPE_BTREE ){
    sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
  }లేకపోతే{
    ఒకవేళ( pC->deferredMoveto ){
      rc = sqlite3_Vdbe_ముగింపు_Moveto(pC);
      ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
    }
    ఒకవేళ( sqlite3BtreeEof(pC->uc.pCursor) ){
      sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
    }లేకపోతే{
      sqlite3_Vdbe_Mem_సమితి_Int64(p_బహిర్గతం, sqlite3_Btree_స్థానభ్రంశం(pC->uc.pCursor));
    }
  }
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_OFFSET_SQL_FUNC */

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
స్థితి OP_స్తంభం: {            /* ncycle */
  u32 p2;            /* column number to retrieve */
  VdbeCursor *pC;    /* The VDBE cursor */
  BtCursor *pCrsr;   /* The B-Tree cursor corresponding to pC */
  u32 *a_స్థానభ్రంశం;      /* aOffset[i] is offset to start of data for i-th column */
  పూర్ణాంకం len;           /* The length of the serialized data for the column */
  పూర్ణాంకం i;             /* Loop counter */
  Mem *pDest;        /* Where to write the extracted value */
  Mem sMem;          /* For storing the record being decoded */
  స్థిరరాశి u8 *z_దత్తాంశం;   /* Part of the record being decoded */
  స్థిరరాశి u8 *zHdr;    /* Next unparsed byte of the header */
  స్థిరరాశి u8 *z_చివర_Hdr; /* Pointer to first byte after the header */
  u64 offset64;      /* 64-bit offset */
  u32 t;             /* A type code from the record header */
  Mem *pReg;         /* PseudoTable input register */

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  pC = p->apCsr[pOp->p1];
  p2 = (u32)pOp->p2;

op_స్తంభం_restart:
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( p2<(u32)pC->n_క్షేత్రం
       || (pC->eCurType==CURTYPE_PSEUDO && pC->seek_ఫలితం==0) );
  a_స్థానభ్రంశం = pC->a_స్థానభ్రంశం;
  ప్రతిజ్ఞా( a_స్థానభ్రంశం==pC->aType+pC->n_క్షేత్రం );
  ప్రతిజ్ఞా( pC->eCurType!=CURTYPE_VTAB );
  ప్రతిజ్ఞా( pC->eCurType!=CURTYPE_PSEUDO || pC->null_అడ్డువరుస );
  ప్రతిజ్ఞా( pC->eCurType!=CURTYPE_SORTER );

  ఒకవేళ( pC->cache_స్థితి!=p->cacheCtr ){                /*OPTIMIZATION-IF-FALSE*/
    ఒకవేళ( pC->null_అడ్డువరుస ){
      ఒకవేళ( pC->eCurType==CURTYPE_PSEUDO && pC->seek_ఫలితం>0 ){
        /* For the special case of as pseudo-cursor, the seekResult field
        ** identifies the register that holds the record */
        pReg = &aMem[pC->seek_ఫలితం];
        ప్రతిజ్ఞా( pReg->flags & MEM_Blob );
        ప్రతిజ్ఞా( memIsValid(pReg) );
        pC->payload_కొలత = pC->sz_అడ్డువరుస = pReg->n;
        pC->a_అడ్డువరుస = (u8*)pReg->z;
      }లేకపోతే{
        pDest = &aMem[pOp->p3];
        mem_About_గమ్యం_Change(p, pDest);
        sqlite3_Vdbe_Mem_సమితి_Null(pDest);
        వెళ్ళు op_స్తంభం_బహిర్గతం;
      }
    }లేకపోతే{
      pCrsr = pC->uc.pCursor;
      ఒకవేళ( pC->deferredMoveto ){
        u32 iMap;
        ప్రతిజ్ఞా( !pC->isEphemeral );
        ఒకవేళ( pC->ub.aAltMap && (iMap = pC->ub.aAltMap[1+p2])>0  ){
          pC = pC->pAltCursor;
          p2 = iMap - 1;
          వెళ్ళు op_స్తంభం_restart;
        }
        rc = sqlite3_Vdbe_ముగింపు_Moveto(pC);
        ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
      }లేకపోతే ఒకవేళ( sqlite3BtreeCursorHasMoved(pCrsr) ){
        rc = sqlite3VdbeHandleMovedCursor(pC);
        ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
        వెళ్ళు op_స్తంభం_restart;
      }
      ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
      ప్రతిజ్ఞా( pCrsr );
      ప్రతిజ్ఞా( sqlite3BtreeCursorIsValid(pCrsr) );
      pC->payload_కొలత = sqlite3_Btree_Payload_కొలత(pCrsr);
      pC->a_అడ్డువరుస = sqlite3BtreePayloadFetch(pCrsr, &pC->sz_అడ్డువరుస);
      ప్రతిజ్ఞా( pC->sz_అడ్డువరుస<=pC->payload_కొలత );
      ప్రతిజ్ఞా( pC->sz_అడ్డువరుస<=65536 );  /* Maximum page size is 64KiB */
    }
    pC->cache_స్థితి = p->cacheCtr;
    ఒకవేళ( (a_స్థానభ్రంశం[0] = pC->a_అడ్డువరుస[0])<0x80 ){
      pC->i_Hdr_స్థానభ్రంశం = 1;
    }లేకపోతే{
      pC->i_Hdr_స్థానభ్రంశం = sqlite3GetVarint32(pC->a_అడ్డువరుస, a_స్థానభ్రంశం);
    }
    pC->nHdrParsed = 0;

    ఒకవేళ( pC->sz_అడ్డువరుస<a_స్థానభ్రంశం[0] ){      /*OPTIMIZATION-IF-FALSE*/
      /* pC->aRow does not have to hold the entire row, but it does at least
      ** need to cover the header of the record.  If pC->aRow does not contain
      ** the complete header, then set it to zero, forcing the header to be
      ** dynamically allocated. */
      pC->a_అడ్డువరుస = 0;
      pC->sz_అడ్డువరుస = 0;

      /* Make sure a corrupt database has not given us an oversize header.
      ** Do this now to avoid an oversize memory allocation.
      **
      ** Type entries can be between 1 and 5 bytes each.  But 4 and 5 byte
      ** types use so much data space that there can only be 4096 and 32 of
      ** them, respectively.  So the maximum header length results from a
      ** 3-byte type for each of the maximum of 32768 columns plus three
      ** extra bytes for the header length itself.  32768*3 + 3 = 98307.
      */
      ఒకవేళ( a_స్థానభ్రంశం[0] > 98307 || a_స్థానభ్రంశం[0] > pC->payload_కొలత ){
        వెళ్ళు op_స్తంభం_corrupt;
      }
    }లేకపోతే{
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
      z_దత్తాంశం = pC->a_అడ్డువరుస;
      ప్రతిజ్ఞా( pC->nHdrParsed<=p2 );         /* Conditional skipped */
      testcase( a_స్థానభ్రంశం[0]==0 );
      వెళ్ళు op_స్తంభం_read_శీర్షిక;
    }
  }లేకపోతే ఒకవేళ( sqlite3BtreeCursorHasMoved(pC->uc.pCursor) ){
    rc = sqlite3VdbeHandleMovedCursor(pC);
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
    వెళ్ళు op_స్తంభం_restart;
  }

  /* Make sure at least the first p2+1 entries of the header have been
  ** parsed and valid information is in aOffset[] and pC->aType[].
  */
  ఒకవేళ( pC->nHdrParsed<=p2 ){
    /* If there is more header available for parsing in the record, try
    ** to extract additional fields up through the p2+1-th field
    */
    ఒకవేళ( pC->i_Hdr_స్థానభ్రంశం<a_స్థానభ్రంశం[0] ){
      /* Make sure zData points to enough of the record to cover the header. */
      ఒకవేళ( pC->a_అడ్డువరుస==0 ){
        memset(&sMem, 0, పరిమాణం(sMem));
        rc = sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(pC->uc.pCursor,a_స్థానభ్రంశం[0],&sMem);
        ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abort_due_గమ్యం_error;
        z_దత్తాంశం = (u8*)sMem.z;
      }లేకపోతే{
        z_దత్తాంశం = pC->a_అడ్డువరుస;
      }
 
      /* Fill in pC->aType[i] and aOffset[i] values through the p2-th field. */
    op_స్తంభం_read_శీర్షిక:
      i = pC->nHdrParsed;
      offset64 = a_స్థానభ్రంశం[i];
      zHdr = z_దత్తాంశం + pC->i_Hdr_స్థానభ్రంశం;
      z_చివర_Hdr = z_దత్తాంశం + a_స్థానభ్రంశం[0];
      testcase( zHdr>=z_చివర_Hdr );
      చేయి{
        ఒకవేళ( (pC->aType[i] = t = zHdr[0])<0x80 ){
          zHdr++;
          offset64 += sqlite3VdbeOneByteSerialTypeLen(t);
        }లేకపోతే{
          zHdr += sqlite3GetVarint32(zHdr, &t);
          pC->aType[i] = t;
          offset64 += sqlite3VdbeSerialTypeLen(t);
        }
        a_స్థానభ్రంశం[++i] = (u32)(offset64 & 0xffffffff);
      }అయితే( (u32)i<=p2 && zHdr<z_చివర_Hdr );

      /* The record is corrupt if any of the following are true:
      ** (1) the bytes of the header extend past the declared header size
      ** (2) the entire header was used but not all data was used
      ** (3) the end of the data extends beyond the end of the record.
      */
      ఒకవేళ( (zHdr>=z_చివర_Hdr && (zHdr>z_చివర_Hdr || offset64!=pC->payload_కొలత))
       || (offset64 > pC->payload_కొలత)
      ){
        ఒకవేళ( a_స్థానభ్రంశం[0]==0 ){
          i = 0;
          zHdr = z_చివర_Hdr;
        }లేకపోతే{
          ఒకవేళ( pC->a_అడ్డువరుస==0 ) sqlite3VdbeMemRelease(&sMem);
          వెళ్ళు op_స్తంభం_corrupt;
        }
      }

      pC->nHdrParsed = i;
      pC->i_Hdr_స్థానభ్రంశం = (u32)(zHdr - z_దత్తాంశం);
      ఒకవేళ( pC->a_అడ్డువరుస==0 ) sqlite3VdbeMemRelease(&sMem);
    }లేకపోతే{
      t = 0;
    }

    /* If after trying to extract new entries from the header, nHdrParsed is
    ** still not up to p2, that means that the record has fewer than p2
    ** columns.  So the result will be either the default value or a NULL.
    */
    ఒకవేళ( pC->nHdrParsed<=p2 ){
      pDest = &aMem[pOp->p3];
      mem_About_గమ్యం_Change(p, pDest);
      ఒకవేళ( pOp->p4type==P4_MEM ){
        sqlite3VdbeMemShallowCopy(pDest, pOp->p4.pMem, MEM_Static);
      }లేకపోతే{
        sqlite3_Vdbe_Mem_సమితి_Null(pDest);
      }
      వెళ్ళు op_స్తంభం_బహిర్గతం;
    }
  }లేకపోతే{
    t = pC->aType[p2];
  }

  /* Extract the content for the p2+1-th column.  Control can only
  ** reach this point if aOffset[p2], aOffset[p2+1], and pC->aType[p2] are
  ** all valid.
  */
  ప్రతిజ్ఞా( p2<pC->nHdrParsed );
  ప్రతిజ్ఞా( rc==SQLITE_OK );
  pDest = &aMem[pOp->p3];
  mem_About_గమ్యం_Change(p, pDest);
  ప్రతిజ్ఞా( sqlite3VdbeCheckMemInvariants(pDest) );
  ఒకవేళ( VdbeMemDynamic(pDest) ){
    sqlite3_Vdbe_Mem_సమితి_Null(pDest);
  }
  ప్రతిజ్ఞా( t==pC->aType[p2] );
  ఒకవేళ( pC->sz_అడ్డువరుస>=a_స్థానభ్రంశం[p2+1] ){
    /* This is the common case where the desired content fits on the original
    ** page - where the content is not on an overflow page.
    **
    ** The big switch() is an in-line variant of sqlite3VdbeSerialGet() that
    ** has been optimized for the OP_Column opcode.
    */
    z_దత్తాంశం = pC->a_అడ్డువరుస + a_స్థానభ్రంశం[p2];
    ఎంపిక( t ){
      స్థితి 0:
      స్థితి 11:
        pDest->flags = MEM_Null;
        ఆపు;
      స్థితి 1:
        pDest->u.i = ONE_BYTE_INT(z_దత్తాంశం);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ఆపు;
      స్థితి 2:
        pDest->u.i = TWO_BYTE_INT(z_దత్తాంశం);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ఆపు;
      స్థితి 3:
        pDest->u.i = THREE_BYTE_INT(z_దత్తాంశం);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ఆపు;
      స్థితి 4:
        pDest->u.i = FOUR_BYTE_INT(z_దత్తాంశం);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ఆపు;
      స్థితి 5:
        pDest->u.i = SIX_BYTE_INT(z_దత్తాంశం);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ఆపు;
      స్థితి 6: {
        pDest->u.i = (i64)sqlite3Get8byte(z_దత్తాంశం);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        ఆపు;
      }
      స్థితి 7: {
        u64 x = sqlite3Get8byte(z_దత్తాంశం);
        మార్పిడి_Mixed_Endian_Float(x);
        pDest->flags = IsNaN(x) ? MEM_Null : MEM_Real;
        జ్ఞాపకం_నకలు(&pDest->u.r, &x, పరిమాణం(x));
        testcase( pDest->u.r<0 );
        ఆపు;
      }
      స్థితి 8:
      స్థితి 9: {
        pDest->u.i = t-8;
        pDest->flags = MEM_Int;
        ఆపు;
      }
      స్థితి 10:
        /* Internal use only: NULL with virtual table
        ** UPDATE no-change flag set */
        pDest->flags = MEM_Null|MEM_Zero;
        pDest->u.nZero = 0;
        pDest->n = 0;
        ఆపు;
      అప్రమేయ: {
        /* If the column value is a string or blob, we need a persistent
        ** value, not a MEM_Ephem value.  This case is a fast short-cut
        ** that is equivalent to calling sqlite3VdbeSerialGet() and
        ** sqlite3VdbeDeephemeralize().
        */
        స్థిర స్థిరరాశి u16 aFlag[] = { MEM_Blob, MEM_Str|MEM_పదం };
        pDest->n = len = (t-12)/2;
        pDest->enc = encoding;
        ఒకవేళ( pDest->szMalloc < len+2 ){
          ఒకవేళ( len>db->a_హద్దు[SQLITE_హద్దు_నిడివి] ) వెళ్ళు too_big;
          pDest->flags = MEM_Null;
          ఒకవేళ( sqlite3VdbeMemGrow(pDest, len+2, 0) ) వెళ్ళు no_mem;
        }లేకపోతే{
          pDest->z = pDest->zMalloc;
        }
        జ్ఞాపకం_నకలు(pDest->z, z_దత్తాంశం, len);
        pDest->z[len] = 0;
        pDest->z[len+1] = 0;
        pDest->flags = aFlag[t&1];
      }
    } /* End of switch */
  }లేకపోతే{
    u8 p5;
    pDest->enc = encoding;
    ప్రతిజ్ఞా( pDest->db==db );
    /* This branch happens only when content is on overflow pages */
    ఒకవేళ( ((p5 = (pOp->p5 & OPFLAG_BYTELENARG))!=0
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
    }లేకపోతే{
      rc = vdbe_స్తంభం_From_అధికప్రవాహం(pC, p2, t, a_స్థానభ్రంశం[p2],
                p->cacheCtr, నిలువు_Cache_Ctr, pDest);
      ఒకవేళ( rc ){
        ఒకవేళ( rc==SQLITE_NOMEM ) వెళ్ళు no_mem;
        ఒకవేళ( rc==SQLITE_TOOBIG ) వెళ్ళు too_big;
        వెళ్ళు abort_due_గమ్యం_error;
      }
    }
  }

op_స్తంభం_బహిర్గతం:
  UPDATE_MAX_BLOBSIZE(pDest);
  రిజిస్టరు_TRACE(pOp->p3, pDest);
  ఆపు;

op_స్తంభం_corrupt:
  ఒకవేళ( aOp[0].p3>0 ){
    pOp = &aOp[aOp[0].p3-1];
    ఆపు;
  }లేకపోతే{
    rc = SQLITE_CORRUPT_BKPT;
    వెళ్ళు abort_due_గమ్యం_error;
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
స్థితి OP_TypeCheck: {
  పట్టిక *pTab;
  స్తంభం *a_నిలువు;
  పూర్ణాంకం i;
  పూర్ణాంకం n_నిలువు;

  ప్రతిజ్ఞా( pOp->p4type==P4_పట్టిక );
  pTab = pOp->p4.pTab;
  ప్రతిజ్ఞా( pTab->tabFlags & TF_Strict );
  ప్రతిజ్ఞా( pOp->p3>=0 && pOp->p3<pTab->n_నిలువు+2 );
  a_నిలువు = pTab->a_నిలువు;
  pIn1 = &aMem[pOp->p1];
  ఒకవేళ( pOp->p3<2 ){
    ప్రతిజ్ఞా( pTab->nNVCol==pOp->p2 );
    i = 0;
    n_నిలువు = pTab->n_నిలువు;
  }లేకపోతే{
    i = pOp->p3-2;
    n_నిలువు = i+1;
    ప్రతిజ్ఞా( i<pTab->n_నిలువు );
    ప్రతిజ్ఞా( a_నిలువు[i].నిలువు_Flags & COLFLAG_VIRTUAL );
    ప్రతిజ్ఞా( pOp->p2==1 );
  }
  కొరకు(; i<n_నిలువు; i++){
    ఒకవేళ( (a_నిలువు[i].నిలువు_Flags & COLFLAG_GENERATED)!=0 && pOp->p3<2 ){
      ఒకవేళ( (a_నిలువు[i].నిలువు_Flags & COLFLAG_VIRTUAL)!=0 ) కొనసాగు;
      ఒకవేళ( pOp->p3 ){ pIn1++; కొనసాగు; }
    }
    ప్రతిజ్ఞా( pIn1 < &aMem[pOp->p1+pOp->p2] );
    applyAffinity(pIn1, a_నిలువు[i].affinity, encoding);
    ఒకవేళ( (pIn1->flags & MEM_Null)==0 ){
      ఎంపిక( a_నిలువు[i].eCType ){
        స్థితి COLTYPE_BLOB: {
          ఒకవేళ( (pIn1->flags & MEM_Blob)==0 ) వెళ్ళు vdbe_type_error;
          ఆపు;
        }
        స్థితి COLTYPE_INTEGER:
        స్థితి COLTYPE_INT: {
          ఒకవేళ( (pIn1->flags & MEM_Int)==0 ) వెళ్ళు vdbe_type_error;
          ఆపు;
        }
        స్థితి COLTYPE_పాఠ్యం: {
          ఒకవేళ( (pIn1->flags & MEM_Str)==0 ) వెళ్ళు vdbe_type_error;
          ఆపు;
        }
        స్థితి COLTYPE_REAL: {
          testcase( (pIn1->flags & (MEM_Real|MEM_IntReal))==MEM_Real );
          ప్రతిజ్ఞా( (pIn1->flags & MEM_IntReal)==0 );
          ఒకవేళ( pIn1->flags & MEM_Int ){
            /* When applying REAL affinity, if the result is still an MEM_Int
            ** that will fit in 6 bytes, then change the type to MEM_IntReal
            ** so that we keep the high-resolution integer value but know that
            ** the type really wants to be REAL. */
            testcase( pIn1->u.i==140737488355328LL );
            testcase( pIn1->u.i==140737488355327LL );
            testcase( pIn1->u.i==-140737488355328LL );
            testcase( pIn1->u.i==-140737488355329LL );
            ఒకవేళ( pIn1->u.i<=140737488355327LL && pIn1->u.i>=-140737488355328LL){
              pIn1->flags |= MEM_IntReal;
              pIn1->flags &= ~MEM_Int;
            }లేకపోతే{
              pIn1->u.r = (ద్విగుణ)pIn1->u.i;
              pIn1->flags |= MEM_Real;
              pIn1->flags &= ~MEM_Int;
            }
          }లేకపోతే ఒకవేళ( (pIn1->flags & (MEM_Real|MEM_IntReal))==0 ){
            వెళ్ళు vdbe_type_error;
          }
          ఆపు;
        }
        అప్రమేయ: {
          /* COLTYPE_ANY.  Accept anything. */
          ఆపు;
        }
      }
    }
    రిజిస్టరు_TRACE((పూర్ణాంకం)(pIn1-aMem), pIn1);
    pIn1++;
  }
  ప్రతిజ్ఞా( pIn1 == &aMem[pOp->p1+pOp->p2] );
  ఆపు;

vdbe_type_error:
  sqlite3VdbeError(p, "cannot store %s value in %s column %s.%s",
     vdbeMemTypeName(pIn1), sqlite3StdType[a_నిలువు[i].eCType-1],
     pTab->zName, a_నిలువు[i].zCnName);
  rc = SQLITE_CONSTRAINT_DATATYPE;
  వెళ్ళు abort_due_గమ్యం_error;
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
స్థితి OP_Affinity: {
  స్థిరరాశి అక్షరం *zAffinity;   /* The affinity to be applied */

  zAffinity = pOp->p4.z;
  ప్రతిజ్ఞా( zAffinity!=0 );
  ప్రతిజ్ఞా( pOp->p2>0 );
  ప్రతిజ్ఞా( zAffinity[pOp->p2]==0 );
  pIn1 = &aMem[pOp->p1];
  అయితే( 1 /*exit-by-break*/ ){
    ప్రతిజ్ఞా( pIn1 <= &p->aMem[(p->nMem+1 - p->nCursor)] );
    ప్రతిజ్ఞా( zAffinity[0]==SQLITE_AFF_NONE || memIsValid(pIn1) );
    applyAffinity(pIn1, zAffinity[0], encoding);
    ఒకవేళ( zAffinity[0]==SQLITE_AFF_REAL && (pIn1->flags & MEM_Int)!=0 ){
      pIn1->u.r = (ద్విగుణ)pIn1->u.i;
      pIn1->flags |= MEM_Real;
      pIn1->flags &= ~(MEM_Int|MEM_Str);
    }
    రిజిస్టరు_TRACE((పూర్ణాంకం)(pIn1-aMem), pIn1);
    zAffinity++;
    ఒకవేళ( zAffinity[0]==0 ) ఆపు;
    pIn1++;
  }
  ఆపు;
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
స్థితి OP_Make_రికార్డు: {
  Mem *pRec;             /* The new record */
  u64 n_దత్తాంశం;             /* Number of bytes of data space */
  పూర్ణాంకం nHdr;              /* Number of bytes of header space */
  i64 nByte;             /* Data space required for this record */
  i64 nZero;             /* Number of zero bytes at the end of the record */
  పూర్ణాంకం nVarint;           /* Number of bytes in a varint */
  u32 serial_type;       /* Type field */
  Mem *pData0;           /* First field to be combined into the record */
  Mem *p_చివరి;            /* Last field of the record */
  పూర్ణాంకం n_క్షేత్రం;            /* Number of fields in the record */
  అక్షరం *zAffinity;       /* The affinity string for the record */
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
  n_దత్తాంశం = 0;         /* Number of bytes of data space */
  nHdr = 0;          /* Number of bytes of header space */
  nZero = 0;         /* Number of zero bytes at the end of the record */
  n_క్షేత్రం = pOp->p1;
  zAffinity = pOp->p4.z;
  ప్రతిజ్ఞా( n_క్షేత్రం>0 && pOp->p2>0 && pOp->p2+n_క్షేత్రం<=(p->nMem+1 - p->nCursor)+1 );
  pData0 = &aMem[n_క్షేత్రం];
  n_క్షేత్రం = pOp->p2;
  p_చివరి = &pData0[n_క్షేత్రం-1];

  /* Identify the output register */
  ప్రతిజ్ఞా( pOp->p3<pOp->p1 || pOp->p3>=pOp->p1+pOp->p2 );
  p_బహిర్గతం = &aMem[pOp->p3];
  mem_About_గమ్యం_Change(p, p_బహిర్గతం);

  /* Apply the requested affinity to all inputs
  */
  ప్రతిజ్ఞా( pData0<=p_చివరి );
  ఒకవేళ( zAffinity ){
    pRec = pData0;
    చేయి{
      applyAffinity(pRec, zAffinity[0], encoding);
      ఒకవేళ( zAffinity[0]==SQLITE_AFF_REAL && (pRec->flags & MEM_Int) ){
        pRec->flags |= MEM_IntReal;
        pRec->flags &= ~(MEM_Int);
      }
      రిజిస్టరు_TRACE((పూర్ణాంకం)(pRec-aMem), pRec);
      zAffinity++;
      pRec++;
      ప్రతిజ్ఞా( zAffinity[0]==0 || pRec<=p_చివరి );
    }అయితే( zAffinity[0] );
  }

#నిర్వచితమైతే SQLITE_ENABLE_NULL_TRIM
  /* NULLs can be safely trimmed from the end of the record, as long as
  ** as the schema format is 2 or more and none of the omitted columns
  ** have a non-NULL default value.  Also, the record must be left with
  ** at least one field.  If P5>0 then it will be one more than the
  ** index of the right-most column with a non-NULL default value */
  ఒకవేళ( pOp->p5 ){
    అయితే( (p_చివరి->flags & MEM_Null)!=0 && n_క్షేత్రం>pOp->p5 ){
      p_చివరి--;
      n_క్షేత్రం--;
    }
  }
#అయితే_ముగింపు

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
  pRec = p_చివరి;
  చేయి{
    ప్రతిజ్ఞా( memIsValid(pRec) );
    ఒకవేళ( pRec->flags & MEM_Null ){
      ఒకవేళ( pRec->flags & MEM_Zero ){
        /* Values with MEM_Null and MEM_Zero are created by xColumn virtual
        ** table methods that never invoke sqlite3_result_xxxxx() while
        ** computing an unchanging column value in an UPDATE statement.
        ** Give such values a special internal-use-only serial-type of 10
        ** so that they can be passed through to xUpdate and have
        ** a true sqlite3_value_nochange(). */
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_NULL_TRIM
        ప్రతిజ్ఞా( pOp->p5==OPFLAG_NOCHNG_MAGIC || CORRUPT_DB );
#అయితే_ముగింపు
        pRec->u_తాత్కాలిక = 10;
      }లేకపోతే{
        pRec->u_తాత్కాలిక = 0;
      }
      nHdr++;
    }లేకపోతే ఒకవేళ( pRec->flags & (MEM_Int|MEM_IntReal) ){
      /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
      i64 i = pRec->u.i;
      u64 uu;
      testcase( pRec->flags & MEM_Int );
      testcase( pRec->flags & MEM_IntReal );
      ఒకవేళ( i<0 ){
        uu = ~i;
      }లేకపోతే{
        uu = i;
      }
      nHdr++;
      testcase( uu==127 );               testcase( uu==128 );
      testcase( uu==32767 );             testcase( uu==32768 );
      testcase( uu==8388607 );           testcase( uu==8388608 );
      testcase( uu==2147483647 );        testcase( uu==2147483648LL );
      testcase( uu==140737488355327LL ); testcase( uu==140737488355328LL );
      ఒకవేళ( uu<=127 ){
        ఒకవేళ( (i&1)==i && p->minWriteFileFormat>=4 ){
          pRec->u_తాత్కాలిక = 8+(u32)uu;
        }లేకపోతే{
          n_దత్తాంశం++;
          pRec->u_తాత్కాలిక = 1;
        }
      }లేకపోతే ఒకవేళ( uu<=32767 ){
        n_దత్తాంశం += 2;
        pRec->u_తాత్కాలిక = 2;
      }లేకపోతే ఒకవేళ( uu<=8388607 ){
        n_దత్తాంశం += 3;
        pRec->u_తాత్కాలిక = 3;
      }లేకపోతే ఒకవేళ( uu<=2147483647 ){
        n_దత్తాంశం += 4;
        pRec->u_తాత్కాలిక = 4;
      }లేకపోతే ఒకవేళ( uu<=140737488355327LL ){
        n_దత్తాంశం += 6;
        pRec->u_తాత్కాలిక = 5;
      }లేకపోతే{
        n_దత్తాంశం += 8;
        ఒకవేళ( pRec->flags & MEM_IntReal ){
          /* If the value is IntReal and is going to take up 8 bytes to store
          ** as an integer, then we might as well make it an 8-byte floating
          ** point value */
          pRec->u.r = (ద్విగుణ)pRec->u.i;
          pRec->flags &= ~MEM_IntReal;
          pRec->flags |= MEM_Real;
          pRec->u_తాత్కాలిక = 7;
        }లేకపోతే{
          pRec->u_తాత్కాలిక = 6;
        }
      }
    }లేకపోతే ఒకవేళ( pRec->flags & MEM_Real ){
      nHdr++;
      n_దత్తాంశం += 8;
      pRec->u_తాత్కాలిక = 7;
    }లేకపోతే{
      ప్రతిజ్ఞా( db->mallocFailed || pRec->flags&(MEM_Str|MEM_Blob) );
      ప్రతిజ్ఞా( pRec->n>=0 );
      len = (u32)pRec->n;
      serial_type = (len*2) + 12 + ((pRec->flags & MEM_Str)!=0);
      ఒకవేళ( pRec->flags & MEM_Zero ){
        serial_type += (u32)pRec->u.nZero*2;
        ఒకవేళ( n_దత్తాంశం ){
          ఒకవేళ( sqlite3VdbeMemExpandBlob(pRec) ) వెళ్ళు no_mem;
          len += pRec->u.nZero;
        }లేకపోతే{
          nZero += pRec->u.nZero;
        }
      }
      n_దత్తాంశం += len;
      nHdr += sqlite3VarintLen(serial_type);
      pRec->u_తాత్కాలిక = serial_type;
    }
    ఒకవేళ( pRec==pData0 ) ఆపు;
    pRec--;
  }అయితే(1);

  /* EVIDENCE-OF: R-22564-11647 The header begins with a single varint
  ** which determines the total number of bytes in the header. The varint
  ** value is the size of the header in bytes including the size varint
  ** itself. */
  testcase( nHdr==126 );
  testcase( nHdr==127 );
  ఒకవేళ( nHdr<=126 ){
    /* The common case */
    nHdr += 1;
  }లేకపోతే{
    /* Rare case of a really large header */
    nVarint = sqlite3VarintLen(nHdr);
    nHdr += nVarint;
    ఒకవేళ( nVarint<sqlite3VarintLen(nHdr) ) nHdr++;
  }
  nByte = nHdr+n_దత్తాంశం;

  /* If we are able to put an over-run area of 7 bytes on the end of the
  ** memory allocation into which the record is being constructed, then
  ** the encoding of integer values can go faster. This is only possible
  ** if SQLITE_MAX_LENGTH is no with 7 of INT32_MAX and if the host CPU
  ** byte-order is known at compile-time.
  */
#ఒకవేళ SQLITE_MAX_నిడివి<=2147483640 && SQLITE_BYTEORDER>0
# define OVERRUN 7   /* We are able to allocate an overrun of 7 bytes */
#లేకపోతే
# define OVERRUN 0   /* No overrun will be available */
#అయితే_ముగింపు

  /* Make sure the output register has a buffer large enough to store
  ** the new record. The output register (pOp->p3) is not allowed to
  ** be one of the input registers (because the following call to
  ** sqlite3VdbeMemClearAndResize() could clobber the value before it is used).
  */
  ఒకవేళ( nByte+nZero<=p_బహిర్గతం->szMalloc-OVERRUN ){
    /* The output register is already large enough to hold the record.
    ** No error checks or buffer enlargement is required */
    p_బహిర్గతం->z = p_బహిర్గతం->zMalloc;
  }లేకపోతే{
    /* Need to make sure that the output is not too big and then enlarge
    ** the output register to hold the full result */
    ఒకవేళ( nByte+nZero>db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
      వెళ్ళు too_big;
    }
    ఒకవేళ( sqlite3VdbeMemClearAndResize(p_బహిర్గతం, (పూర్ణాంకం)nByte+OVERRUN) ){
      వెళ్ళు no_mem;
    }
  }
  p_బహిర్గతం->n = (పూర్ణాంకం)nByte;
  p_బహిర్గతం->flags = MEM_Blob;
  ఒకవేళ( nZero ){
    p_బహిర్గతం->u.nZero = nZero;
    p_బహిర్గతం->flags |= MEM_Zero;
  }
  UPDATE_MAX_BLOBSIZE(p_బహిర్గతం);
  zHdr = (u8 *)p_బహిర్గతం->z;
  zPayload = zHdr + nHdr;

  /* Write the record */
  ఒకవేళ( nHdr<0x80 ){
    *(zHdr++) = nHdr;
  }లేకపోతే{
    zHdr += sqlite3PutVarint(zHdr,nHdr);
  }
  ప్రతిజ్ఞా( pData0<=p_చివరి );
  pRec = pData0;
  అయితే( 1 /*exit-by-break*/ ){
    serial_type = pRec->u_తాత్కాలిక;
    /* EVIDENCE-OF: R-06529-47362 Following the size varint are one or more
    ** additional varints, one per column.
    ** EVIDENCE-OF: R-64536-51728 The values for each column in the record
    ** immediately follow the header. */
    ఒకవేళ( serial_type<=7 ){
      *(zHdr++) = serial_type;
      ఒకవేళ( serial_type==0 ){
        /* NULL value.  No change in zPayload */
      }లేకపోతే{
        u64 v;
        ఒకవేళ( serial_type==7 ){
          ప్రతిజ్ఞా( పరిమాణం(v)==పరిమాణం(pRec->u.r) );
          జ్ఞాపకం_నకలు(&v, &pRec->u.r, పరిమాణం(v));
          మార్పిడి_Mixed_Endian_Float(v);
        }లేకపోతే{
          v = pRec->u.i;
        }
        len = sqlite3SmallTypeSizes[serial_type];
        ప్రతిజ్ఞా( len>=1 && len<=8 && len!=5 && len!=7 );
#ఒకవేళ SQLITE_BYTEORDER==1234
        v = sqlite3BSwap64(v);
        ఒకవేళ( OVERRUN ){
          స్థిర స్థిరరాశి u8 a_జరుపు[] = { 0, 56, 48, 40, 32, 16, 0, 0 };
          v >>= a_జరుపు[serial_type];
          జ్ఞాపకం_నకలు(zPayload, &v, 8);
        }లేకపోతే{
          /* Test this limb by compiling with -DSQLITE_MAX_LENGTH=2147483647 */
          జ్ఞాపకం_నకలు(zPayload, (u8*)&v + 8 - len, len);
        }
#లేదావేళ SQLITE_BYTEORDER==4321
        ఒకవేళ( OVERRUN ){
          స్థిర స్థిరరాశి u8 a_జరుపు[] = { 0, 56, 48, 40, 32, 16, 0, 0 };
          v <<= a_జరుపు[serial_type];
          జ్ఞాపకం_నకలు(zPayload, &v, 8);
        }లేకపోతే{
          /* Test this limb by compiling with -DSQLITE_MAX_LENGTH=2147483647 */
          జ్ఞాపకం_నకలు(zPayload, (u8*)&v + 8 - len, len);
        }
#లేకపోతే
        /* Test this limb by compiling with -DSQLITE_BYTEORDER=0 */
        ఎంపిక( len ){
          అప్రమేయ: zPayload[7] = (u8)(v&0xff); v >>= 8;
                   zPayload[6] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          స్థితి 6:  zPayload[5] = (u8)(v&0xff); v >>= 8;
                   zPayload[4] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          స్థితి 4:  zPayload[3] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          స్థితి 3:  zPayload[2] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          స్థితి 2:  zPayload[1] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          స్థితి 1:  zPayload[0] = (u8)(v&0xff);
        }
#అయితే_ముగింపు
#నిర్వచనం_రద్దు OVERRUN  /* We are done with the OVERRUN macro now */
        zPayload += len;
      }
    }లేకపోతే ఒకవేళ( serial_type<0x80 ){
      *(zHdr++) = serial_type;
      ఒకవేళ( serial_type>=14 && pRec->n>0 ){
        ప్రతిజ్ఞా( pRec->z!=0 );
        జ్ఞాపకం_నకలు(zPayload, pRec->z, pRec->n);
        zPayload += pRec->n;
      }
    }లేకపోతే{
      zHdr += sqlite3PutVarint(zHdr, serial_type);
      ఒకవేళ( pRec->n ){
        ప్రతిజ్ఞా( pRec->z!=0 );
        ప్రతిజ్ఞా( pRec->z!=(స్థిరరాశి అక్షరం*)sqlite3CtypeMap );
        జ్ఞాపకం_నకలు(zPayload, pRec->z, pRec->n);
        zPayload += pRec->n;
      }
    }
    ఒకవేళ( pRec==p_చివరి ) ఆపు;
    pRec++;
  }
  ప్రతిజ్ఞా( nHdr==(పూర్ణాంకం)(zHdr - (u8*)p_బహిర్గతం->z) );
  ప్రతిజ్ఞా( nByte==(పూర్ణాంకం)(zPayload - (u8*)p_బహిర్గతం->z) );

  ప్రతిజ్ఞా( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  రిజిస్టరు_TRACE(pOp->p3, p_బహిర్గతం);
  ఆపు;
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
స్థితి OP_లెక్క: {         /* out2 */
  i64 n_నమోదు;
  BtCursor *pCrsr;

  ప్రతిజ్ఞా( p->apCsr[pOp->p1]->eCurType==CURTYPE_BTREE );
  pCrsr = p->apCsr[pOp->p1]->uc.pCursor;
  ప్రతిజ్ఞా( pCrsr );
  ఒకవేళ( pOp->p3 ){
    n_నమోదు = sqlite3_Btree_అడ్డువరుస_లెక్క_Est(pCrsr);
  }లేకపోతే{
    n_నమోదు = 0;  /* Not needed.  Only used to silence a warning. */
    rc = sqlite3_Btree_లెక్క(db, pCrsr, &n_నమోదు);
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  }
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->u.i = n_నమోదు;
  వెళ్ళు check_for_అంతరాయం;
}

/* Opcode: Savepoint P1 * * P4 *
**
** Open, release or rollback the savepoint named by parameter P4, depending
** on the value of P1. To open a new savepoint set P1==0 (SAVEPOINT_BEGIN).
** To release (commit) an existing savepoint set P1==1 (SAVEPOINT_RELEASE).
** To rollback an existing savepoint set P1==2 (SAVEPOINT_ROLLBACK).
*/
స్థితి OP_Savepoint: {
  పూర్ణాంకం p1;                         /* Value of P1 operand */
  అక్షరం *zName;                    /* Name of savepoint */
  పూర్ణాంకం nName;
  Savepoint *pNew;
  Savepoint *pSavepoint;
  Savepoint *pTmp;
  పూర్ణాంకం iSavepoint;
  పూర్ణాంకం ii;

  p1 = pOp->p1;
  zName = pOp->p4.z;

  /* Assert that the p1 parameter is valid. Also that if there is no open
  ** transaction, then there cannot be any savepoints.
  */
  ప్రతిజ్ఞా( db->pSavepoint==0 || db->autoCommit==0 );
  ప్రతిజ్ఞా( p1==SAVEPOINT_BEGIN||p1==SAVEPOINT_RELEASE||p1==SAVEPOINT_ROLLBACK );
  ప్రతిజ్ఞా( db->pSavepoint || db->isTransactionSavepoint==0 );
  ప్రతిజ్ఞా( check_Savepoint_లెక్క(db) );
  ప్రతిజ్ఞా( p->bIsReader );

  ఒకవేళ( p1==SAVEPOINT_BEGIN ){
    ఒకవేళ( db->nVdbeWrite>0 ){
      /* A new savepoint cannot be created if there are active write
      ** statements (i.e. open read/write incremental blob handles).
      */
      sqlite3VdbeError(p, "cannot open savepoint - SQL statements in progress");
      rc = SQLITE_BUSY;
    }లేకపోతే{
      nName = sqlite3Strlen30(zName);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
      /* This call is Ok even if this savepoint is actually a transaction
      ** savepoint (and therefore should not prompt xSavepoint()) callbacks.
      ** If this is a transaction savepoint being opened, it is guaranteed
      ** that the db->aVTrans[] array is empty.  */
      ప్రతిజ్ఞా( db->autoCommit==0 || db->nVTrans==0 );
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_BEGIN,
                                db->nStatement+db->nSavepoint);
      ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abort_due_గమ్యం_error;
#అయితే_ముగింపు

      /* Create a new savepoint structure. */
      pNew = sqlite3DbMallocRawNN(db, పరిమాణం(Savepoint)+nName+1);
      ఒకవేళ( pNew ){
        pNew->zName = (అక్షరం *)&pNew[1];
        జ్ఞాపకం_నకలు(pNew->zName, zName, nName+1);
   
        /* If there is no open transaction, then mark this as a special
        ** "transaction savepoint". */
        ఒకవేళ( db->autoCommit ){
          db->autoCommit = 0;
          db->isTransactionSavepoint = 1;
        }లేకపోతే{
          db->nSavepoint++;
        }

        /* Link the new savepoint into the database handle's list. */
        pNew->p_తదుపరి = db->pSavepoint;
        db->pSavepoint = pNew;
        pNew->nDeferredCons = db->nDeferredCons;
        pNew->nDeferredImmCons = db->nDeferredImmCons;
      }
    }
  }లేకపోతే{
    ప్రతిజ్ఞా( p1==SAVEPOINT_RELEASE || p1==SAVEPOINT_ROLLBACK );
    iSavepoint = 0;

    /* Find the named savepoint. If there is no such savepoint, then an
    ** an error is returned to the user.  */
    కొరకు(
      pSavepoint = db->pSavepoint;
      pSavepoint && sqlite3StrICmp(pSavepoint->zName, zName);
      pSavepoint = pSavepoint->p_తదుపరి
    ){
      iSavepoint++;
    }
    ఒకవేళ( !pSavepoint ){
      sqlite3VdbeError(p, "no such savepoint: %s", zName);
      rc = SQLITE_ERROR;
    }లేకపోతే ఒకవేళ( db->nVdbeWrite>0 && p1==SAVEPOINT_RELEASE ){
      /* It is not possible to release (commit) a savepoint if there are
      ** active write statements.
      */
      sqlite3VdbeError(p, "cannot release savepoint - "
                          "SQL statements in progress");
      rc = SQLITE_BUSY;
    }లేకపోతే{

      /* Determine whether or not this is a transaction savepoint. If so,
      ** and this is a RELEASE command, then the current transaction
      ** is committed.
      */
      పూర్ణాంకం isTransaction = pSavepoint->p_తదుపరి==0 && db->isTransactionSavepoint;
      ఒకవేళ( isTransaction && p1==SAVEPOINT_RELEASE ){
        ఒకవేళ( (rc = sqlite3VdbeCheckFkDeferred(p))!=SQLITE_OK ){
          వెళ్ళు vdbe_return;
        }
        db->autoCommit = 1;
        ఒకవేళ( sqlite3VdbeHalt(p)==SQLITE_BUSY ){
          p->pc = (పూర్ణాంకం)(pOp - aOp);
          db->autoCommit = 0;
          p->rc = rc = SQLITE_BUSY;
          వెళ్ళు vdbe_return;
        }
        rc = p->rc;
        ఒకవేళ( rc ){
          db->autoCommit = 0;
        }లేకపోతే{
          db->isTransactionSavepoint = 0;
        }
      }లేకపోతే{
        పూర్ణాంకం isSchemaChange;
        iSavepoint = db->nSavepoint - iSavepoint - 1;
        ఒకవేళ( p1==SAVEPOINT_ROLLBACK ){
          isSchemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0;
          కొరకు(ii=0; ii<db->nDb; ii++){
            rc = sqlite3BtreeTripAllCursors(db->aDb[ii].pBt,
                                       SQLITE_ABORT_ROLLBACK,
                                       isSchemaChange==0);
            ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abort_due_గమ్యం_error;
          }
        }లేకపోతే{
          ప్రతిజ్ఞా( p1==SAVEPOINT_RELEASE );
          isSchemaChange = 0;
        }
        కొరకు(ii=0; ii<db->nDb; ii++){
          rc = sqlite3BtreeSavepoint(db->aDb[ii].pBt, p1, iSavepoint);
          ఒకవేళ( rc!=SQLITE_OK ){
            వెళ్ళు abort_due_గమ్యం_error;
          }
        }
        ఒకవేళ( isSchemaChange ){
          sqlite3ExpirePreparedStatements(db, 0);
          sqlite3_Reset_All_Schemas_యొక్క_Connection(db);
          db->mDbFlags |= DBFLAG_SchemaChange;
        }
      }
      ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
 
      /* Regardless of whether this is a RELEASE or ROLLBACK, destroy all
      ** savepoints nested inside of the savepoint being operated on. */
      అయితే( db->pSavepoint!=pSavepoint ){
        pTmp = db->pSavepoint;
        db->pSavepoint = pTmp->p_తదుపరి;
        sqlite3DbFree(db, pTmp);
        db->nSavepoint--;
      }

      /* If it is a RELEASE, then destroy the savepoint being operated on
      ** too. If it is a ROLLBACK TO, then set the number of deferred
      ** constraint violations present in the database to the value stored
      ** when the savepoint was created.  */
      ఒకవేళ( p1==SAVEPOINT_RELEASE ){
        ప్రతిజ్ఞా( pSavepoint==db->pSavepoint );
        db->pSavepoint = pSavepoint->p_తదుపరి;
        sqlite3DbFree(db, pSavepoint);
        ఒకవేళ( !isTransaction ){
          db->nSavepoint--;
        }
      }లేకపోతే{
        ప్రతిజ్ఞా( p1==SAVEPOINT_ROLLBACK );
        db->nDeferredCons = pSavepoint->nDeferredCons;
        db->nDeferredImmCons = pSavepoint->nDeferredImmCons;
      }

      ఒకవేళ( !isTransaction || p1==SAVEPOINT_ROLLBACK ){
        rc = sqlite3VtabSavepoint(db, p1, iSavepoint);
        ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abort_due_గమ్యం_error;
      }
    }
  }
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఒకవేళ( p->eVdbeState==VDBE_HALT_STATE ){
    rc = SQLITE_పూర్తి;
    వెళ్ళు vdbe_return;
  }
  ఆపు;
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
స్థితి OP_AutoCommit: {
  పూర్ణాంకం desiredAutoCommit;
  పూర్ణాంకం iRollback;

  desiredAutoCommit = pOp->p1;
  iRollback = pOp->p2;
  ప్రతిజ్ఞా( desiredAutoCommit==1 || desiredAutoCommit==0 );
  ప్రతిజ్ఞా( desiredAutoCommit==1 || iRollback==0 );
  ప్రతిజ్ఞా( db->n_Vdbe_చురుకైన>0 );  /* At least this one VM is active */
  ప్రతిజ్ఞా( p->bIsReader );

  ఒకవేళ( desiredAutoCommit!=db->autoCommit ){
    ఒకవేళ( iRollback ){
      ప్రతిజ్ఞా( desiredAutoCommit==1 );
      sqlite3RollbackAll(db, SQLITE_ABORT_ROLLBACK);
      db->autoCommit = 1;
    }లేకపోతే ఒకవేళ( desiredAutoCommit && db->nVdbeWrite>0 ){
      /* If this instruction implements a COMMIT and other VMs are writing
      ** return an error indicating that the other VMs must complete first.
      */
      sqlite3VdbeError(p, "cannot commit transaction - "
                          "SQL statements in progress");
      rc = SQLITE_BUSY;
      వెళ్ళు abort_due_గమ్యం_error;
    }లేకపోతే ఒకవేళ( (rc = sqlite3VdbeCheckFkDeferred(p))!=SQLITE_OK ){
      వెళ్ళు vdbe_return;
    }లేకపోతే{
      db->autoCommit = (u8)desiredAutoCommit;
    }
    ఒకవేళ( sqlite3VdbeHalt(p)==SQLITE_BUSY ){
      p->pc = (పూర్ణాంకం)(pOp - aOp);
      db->autoCommit = (u8)(1-desiredAutoCommit);
      p->rc = rc = SQLITE_BUSY;
      వెళ్ళు vdbe_return;
    }
    sqlite3CloseSavepoints(db);
    ఒకవేళ( p->rc==SQLITE_OK ){
      rc = SQLITE_పూర్తి;
    }లేకపోతే{
      rc = SQLITE_ERROR;
    }
    వెళ్ళు vdbe_return;
  }లేకపోతే{
    sqlite3VdbeError(p,
        (!desiredAutoCommit)?"cannot start a transaction within a transaction":(
        (iRollback)?"cannot rollback - no transaction is active":
                   "cannot commit - no transaction is active"));
        
    rc = SQLITE_ERROR;
    వెళ్ళు abort_due_గమ్యం_error;
  }
  /*NOTREACHED*/ ప్రతిజ్ఞా(0);
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
స్థితి OP_Transaction: {
  Btree *pBt;
  Db *pDb;
  పూర్ణాంకం iMeta = 0;

  ప్రతిజ్ఞా( p->bIsReader );
  ప్రతిజ్ఞా( p->readOnly==0 || pOp->p2==0 );
  ప్రతిజ్ఞా( pOp->p2>=0 && pOp->p2<=2 );
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<db->nDb );
  ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, pOp->p1) );
  ప్రతిజ్ఞా( rc==SQLITE_OK );
  ఒకవేళ( pOp->p2 && (db->flags & (SQLITE_QueryOnly|SQLITE_CorruptRdOnly))!=0 ){
    ఒకవేళ( db->flags & SQLITE_QueryOnly ){
      /* Writes prohibited by the "PRAGMA query_only=TRUE" statement */
      rc = SQLITE_READONLY;
    }లేకపోతే{
      /* Writes prohibited due to a prior SQLITE_CORRUPT in the current
      ** transaction */
      rc = SQLITE_CORRUPT;
    }
    వెళ్ళు abort_due_గమ్యం_error;
  }
  pDb = &db->aDb[pOp->p1];
  pBt = pDb->pBt;

  ఒకవేళ( pBt ){
    rc = sqlite3BtreeBeginTrans(pBt, pOp->p2, &iMeta);
    testcase( rc==SQLITE_BUSY_SNAPSHOT );
    testcase( rc==SQLITE_BUSY_RECOVERY );
    ఒకవేళ( rc!=SQLITE_OK ){
      ఒకవేళ( (rc&0xff)==SQLITE_BUSY ){
        p->pc = (పూర్ణాంకం)(pOp - aOp);
        p->rc = rc;
        వెళ్ళు vdbe_return;
      }
      వెళ్ళు abort_due_గమ్యం_error;
    }

    ఒకవేళ( p->ఉపయోగిస్తుంది_Stmt_Journal
     && pOp->p2
     && (db->autoCommit==0 || db->nVdbeRead>1)
    ){
      ప్రతిజ్ఞా( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE );
      ఒకవేళ( p->iStatement==0 ){
        ప్రతిజ్ఞా( db->nStatement>=0 && db->nSavepoint>=0 );
        db->nStatement++;
        p->iStatement = db->nSavepoint + db->nStatement;
      }

      rc = sqlite3VtabSavepoint(db, SAVEPOINT_BEGIN, p->iStatement-1);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = sqlite3BtreeBeginStmt(pBt, p->iStatement);
      }

      /* Store the current value of the database handles deferred constraint
      ** counter. If the statement transaction needs to be rolled back,
      ** the value of this counter needs to be restored too.  */
      p->nStmtDefCons = db->nDeferredCons;
      p->nStmtDefImmCons = db->nDeferredImmCons;
    }
  }
  ప్రతిజ్ఞా( pOp->p5==0 || pOp->p4type==P4_INT32 );
  ఒకవేళ( rc==SQLITE_OK
   && pOp->p5
   && (iMeta!=pOp->p3 || pDb->pSchema->iGeneration!=pOp->p4.i)
  ){
    /*
    ** IMPLEMENTATION-OF: R-03189-51135 As each SQL statement runs, the schema
    ** version is checked to ensure that the schema has not changed since the
    ** SQL statement was prepared.
    */
    sqlite3DbFree(db, p->z_Err_సందేశం);
    p->z_Err_సందేశం = sqlite3DbStrDup(db, "database schema has changed");
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
    ఒకవేళ( db->aDb[pOp->p1].pSchema->schema_cookie!=iMeta ){
      sqlite3ResetOneSchema(db, pOp->p1);
    }
    p->expired = 1;
    rc = SQLITE_SCHEMA;

    /* Set changeCntOn to 0 to prevent the value returned by sqlite3_changes()
    ** from being modified in sqlite3VdbeHalt(). If this statement is
    ** reprepared, changeCntOn will be set again. */
    p->changeCntOn = 0;
  }
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
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
స్థితి OP_ReadCookie: {               /* out2 */
  పూర్ణాంకం iMeta;
  పూర్ణాంకం iDb;
  పూర్ణాంకం iCookie;

  ప్రతిజ్ఞా( p->bIsReader );
  iDb = pOp->p1;
  iCookie = pOp->p3;
  ప్రతిజ్ఞా( pOp->p3<SQLITE_N_BTREE_META );
  ప్రతిజ్ఞా( iDb>=0 && iDb<db->nDb );
  ప్రతిజ్ఞా( db->aDb[iDb].pBt!=0 );
  ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, iDb) );

  sqlite3BtreeGetMeta(db->aDb[iDb].pBt, iCookie, (u32 *)&iMeta);
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->u.i = iMeta;
  ఆపు;
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
స్థితి OP_సమితి_Cookie: {
  Db *pDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  ప్రతిజ్ఞా( pOp->p2<SQLITE_N_BTREE_META );
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<db->nDb );
  ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, pOp->p1) );
  ప్రతిజ్ఞా( p->readOnly==0 );
  pDb = &db->aDb[pOp->p1];
  ప్రతిజ్ఞా( pDb->pBt!=0 );
  ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, pOp->p1, 0) );
  /* See note about index shifting on OP_ReadCookie */
  rc = sqlite3BtreeUpdateMeta(pDb->pBt, pOp->p2, pOp->p3);
  ఒకవేళ( pOp->p2==BTREE_SCHEMA_సంస్కరణ ){
    /* When the schema cookie changes, record the new cookie internally */
    *(u32*)&pDb->pSchema->schema_cookie = *(u32*)&pOp->p3 - pOp->p5;
    db->mDbFlags |= DBFLAG_SchemaChange;
    sqlite3FkClearTriggerCache(db, pOp->p1);
  }లేకపోతే ఒకవేళ( pOp->p2==BTREE_FILE_FORMAT ){
    /* Record changes in the file format */
    pDb->pSchema->file_format = pOp->p3;
  }
  ఒకవేళ( pOp->p1==1 ){
    /* Invalidate all prepared statements whenever the TEMP database
    ** schema is changed.  Ticket #1644 */
    sqlite3ExpirePreparedStatements(db, 0);
    p->expired = 0;
  }
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
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
స్థితి OP_ReopenIdx: {         /* ncycle */
  పూర్ణాంకం n_క్షేత్రం;
  కీలకం_Info *p_కీలకం_Info;
  u32 p2;
  పూర్ణాంకం iDb;
  పూర్ణాంకం wrFlag;
  Btree *pX;
  VdbeCursor *pCur;
  Db *pDb;

  ప్రతిజ్ఞా( pOp->p5==0 || pOp->p5==OPFLAG_SEEKEQ );
  ప్రతిజ్ఞా( pOp->p4type==P4_KEYINFO );
  pCur = p->apCsr[pOp->p1];
  ఒకవేళ( pCur && pCur->pgno_మూలం==(u32)pOp->p2 ){
    ప్రతిజ్ఞా( pCur->iDb==pOp->p3 );      /* Guaranteed by the code generator */
    ప్రతిజ్ఞా( pCur->eCurType==CURTYPE_BTREE );
    sqlite3BtreeClearCursor(pCur->uc.pCursor);
    వెళ్ళు open_cursor_సమితి_hints;
  }
  /* If the cursor is not currently open or is open on a different
  ** index, then fall through into OP_OpenRead to force a reopen */
స్థితి OP_OpenRead:            /* ncycle */
స్థితి OP_OpenWrite:

  ప్రతిజ్ఞా( pOp->opcode==OP_OpenWrite || pOp->p5==0 || pOp->p5==OPFLAG_SEEKEQ );
  ప్రతిజ్ఞా( p->bIsReader );
  ప్రతిజ్ఞా( pOp->opcode==OP_OpenRead || pOp->opcode==OP_ReopenIdx
          || p->readOnly==0 );

  ఒకవేళ( p->expired==1 ){
    rc = SQLITE_ABORT_ROLLBACK;
    వెళ్ళు abort_due_గమ్యం_error;
  }

  n_క్షేత్రం = 0;
  p_కీలకం_Info = 0;
  p2 = (u32)pOp->p2;
  iDb = pOp->p3;
  ప్రతిజ్ఞా( iDb>=0 && iDb<db->nDb );
  ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, iDb) );
  pDb = &db->aDb[iDb];
  pX = pDb->pBt;
  ప్రతిజ్ఞా( pX!=0 );
  ఒకవేళ( pOp->opcode==OP_OpenWrite ){
    ప్రతిజ్ఞా( OPFLAG_FORDELETE==BTREE_FORDELETE );
    wrFlag = BTREE_WRCSR | (pOp->p5 & OPFLAG_FORDELETE);
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ఒకవేళ( pDb->pSchema->file_format < p->minWriteFileFormat ){
      p->minWriteFileFormat = pDb->pSchema->file_format;
    }
    ఒకవేళ( pOp->p5 & OPFLAG_P2ISREG ){
      ప్రతిజ్ఞా( p2>0 );
      ప్రతిజ్ఞా( p2<=(u32)(p->nMem+1 - p->nCursor) );
      pIn2 = &aMem[p2];
      ప్రతిజ్ఞా( memIsValid(pIn2) );
      ప్రతిజ్ఞా( (pIn2->flags & MEM_Int)!=0 );
      sqlite3VdbeMemIntegerify(pIn2);
      p2 = (పూర్ణాంకం)pIn2->u.i;
      /* The p2 value always comes from a prior OP_CreateBtree opcode and
      ** that opcode will always set the p2 value to 2 or more or else fail.
      ** If there were a failure, the prepared statement would have halted
      ** before reaching this instruction. */
      ప్రతిజ్ఞా( p2>=2 );
    }
  }లేకపోతే{
    wrFlag = 0;
    ప్రతిజ్ఞా( (pOp->p5 & OPFLAG_P2ISREG)==0 );
  }
  ఒకవేళ( pOp->p4type==P4_KEYINFO ){
    p_కీలకం_Info = pOp->p4.p_కీలకం_Info;
    ప్రతిజ్ఞా( p_కీలకం_Info->enc==ENC(db) );
    ప్రతిజ్ఞా( p_కీలకం_Info->db==db );
    n_క్షేత్రం = p_కీలకం_Info->n_All_క్షేత్రం;
  }లేకపోతే ఒకవేళ( pOp->p4type==P4_INT32 ){
    n_క్షేత్రం = pOp->p4.i;
  }
  ప్రతిజ్ఞా( pOp->p1>=0 );
  ప్రతిజ్ఞా( n_క్షేత్రం>=0 );
  testcase( n_క్షేత్రం==0 );  /* Table with INTEGER PRIMARY KEY and nothing else */
  pCur = allocateCursor(p, pOp->p1, n_క్షేత్రం, CURTYPE_BTREE);
  ఒకవేళ( pCur==0 ) వెళ్ళు no_mem;
  pCur->iDb = iDb;
  pCur->null_అడ్డువరుస = 1;
  pCur->isOrdered = 1;
  pCur->pgno_మూలం = p2;
#నిర్వచితమైతే SQLITE_DEBUG
  pCur->wrFlag = wrFlag;
#అయితే_ముగింపు
  rc = sqlite3BtreeCursor(pX, p2, wrFlag, p_కీలకం_Info, pCur->uc.pCursor);
  pCur->p_కీలకం_Info = p_కీలకం_Info;
  /* Set the VdbeCursor.isTable variable. Previous versions of
  ** SQLite used to check if the root-page flags were sane at this point
  ** and report database corruption if they were not, but this check has
  ** since moved into the btree layer.  */ 
  pCur->is_పట్టిక = pOp->p4type!=P4_KEYINFO;

open_cursor_సమితి_hints:
  ప్రతిజ్ఞా( OPFLAG_BULKCSR==BTREE_BULKLOAD );
  ప్రతిజ్ఞా( OPFLAG_SEEKEQ==BTREE_SEEK_EQ );
  testcase( pOp->p5 & OPFLAG_BULKCSR );
  testcase( pOp->p2 & OPFLAG_SEEKEQ );
  sqlite3BtreeCursorHintFlags(pCur->uc.pCursor,
                               (pOp->p5 & (OPFLAG_BULKCSR|OPFLAG_SEEKEQ)));
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}

/* Opcode: OpenDup P1 P2 * * *
**
** Open a new cursor P1 that points to the same ephemeral table as
** cursor P2.  The P2 cursor must have been opened by a prior OP_OpenEphemeral
** opcode.  Only ephemeral cursors may be duplicated.
**
** Duplicate ephemeral cursors are used for self-joins of materialized views.
*/
స్థితి OP_OpenDup: {           /* ncycle */
  VdbeCursor *pOrig;    /* The original cursor to be duplicated */
  VdbeCursor *pCx;      /* The new cursor */

  pOrig = p->apCsr[pOp->p2];
  ప్రతిజ్ఞా( pOrig );
  ప్రతిజ్ఞా( pOrig->isEphemeral );  /* Only ephemeral cursors can be duplicated */

  pCx = allocateCursor(p, pOp->p1, pOrig->n_క్షేత్రం, CURTYPE_BTREE);
  ఒకవేళ( pCx==0 ) వెళ్ళు no_mem;
  pCx->null_అడ్డువరుస = 1;
  pCx->isEphemeral = 1;
  pCx->p_కీలకం_Info = pOrig->p_కీలకం_Info;
  pCx->is_పట్టిక = pOrig->is_పట్టిక;
  pCx->pgno_మూలం = pOrig->pgno_మూలం;
  pCx->isOrdered = pOrig->isOrdered;
  pCx->ub.pBtx = pOrig->ub.pBtx;
  pCx->noReuse = 1;
  pOrig->noReuse = 1;
  rc = sqlite3BtreeCursor(pCx->ub.pBtx, pCx->pgno_మూలం, BTREE_WRCSR,
                          pCx->p_కీలకం_Info, pCx->uc.pCursor);
  /* The sqlite3BtreeCursor() routine can only fail for the first cursor
  ** opened for a database.  Since there is already an open cursor when this
  ** opcode is run, the sqlite3BtreeCursor() cannot fail */
  ప్రతిజ్ఞా( rc==SQLITE_OK );
  ఆపు;
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
స్థితి OP_OpenAutoindex:       /* ncycle */
స్థితి OP_OpenEphemeral: {     /* ncycle */
  VdbeCursor *pCx;
  కీలకం_Info *p_కీలకం_Info;

  స్థిర స్థిరరాశి పూర్ణాంకం vfsFlags =
      SQLITE_OPEN_READWRITE |
      SQLITE_OPEN_CREATE |
      SQLITE_OPEN_EXCLUSIVE |
      SQLITE_OPEN_DELETEONCLOSE |
      SQLITE_OPEN_TRANSIENT_DB;
  ప్రతిజ్ఞా( pOp->p1>=0 );
  ప్రతిజ్ఞా( pOp->p2>=0 );
  ఒకవేళ( pOp->p3>0 ){
    /* Make register reg[P3] into a value that can be used as the data
    ** form sqlite3BtreeInsert() where the length of the data is zero. */
    ప్రతిజ్ఞా( pOp->p2==0 ); /* Only used when number of columns is zero */
    ప్రతిజ్ఞా( pOp->opcode==OP_OpenEphemeral );
    ప్రతిజ్ఞా( aMem[pOp->p3].flags & MEM_Null );
    aMem[pOp->p3].n = 0;
    aMem[pOp->p3].z = "";
  }
  pCx = p->apCsr[pOp->p1];
  ఒకవేళ( pCx && !pCx->noReuse &&  ALWAYS(pOp->p2<=pCx->n_క్షేత్రం) ){
    /* If the ephemeral table is already open and has no duplicates from
    ** OP_OpenDup, then erase all existing content so that the table is
    ** empty again, rather than creating a new table. */
    ప్రతిజ్ఞా( pCx->isEphemeral );
    pCx->seq_లెక్క = 0;
    pCx->cache_స్థితి = CACHE_STALE;
    rc = sqlite3_Btree_Clear_పట్టిక(pCx->ub.pBtx, pCx->pgno_మూలం, 0);
  }లేకపోతే{
    pCx = allocateCursor(p, pOp->p1, pOp->p2, CURTYPE_BTREE);
    ఒకవేళ( pCx==0 ) వెళ్ళు no_mem;
    pCx->isEphemeral = 1;
    rc = sqlite3BtreeOpen(db->pVfs, 0, db, &pCx->ub.pBtx,
                          BTREE_OMIT_JOURNAL | BTREE_SINGLE | pOp->p5,
                          vfsFlags);
    ఒకవేళ( rc==SQLITE_OK ){
      rc = sqlite3BtreeBeginTrans(pCx->ub.pBtx, 1, 0);
      ఒకవేళ( rc==SQLITE_OK ){
        /* If a transient index is required, create it by calling
        ** sqlite3BtreeCreateTable() with the BTREE_BLOBKEY flag before
        ** opening it. If a transient table is required, just use the
        ** automatically created table with root-page 1 (an BLOB_INTKEY table).
        */
        ఒకవేళ( (pCx->p_కీలకం_Info = p_కీలకం_Info = pOp->p4.p_కీలకం_Info)!=0 ){
          ప్రతిజ్ఞా( pOp->p4type==P4_KEYINFO );
          rc = sqlite3_Btree_Create_పట్టిక(pCx->ub.pBtx, &pCx->pgno_మూలం,
              BTREE_BLOBKEY | pOp->p5);
          ఒకవేళ( rc==SQLITE_OK ){
            ప్రతిజ్ఞా( pCx->pgno_మూలం==SCHEMA_మూలం+1 );
            ప్రతిజ్ఞా( p_కీలకం_Info->db==db );
            ప్రతిజ్ఞా( p_కీలకం_Info->enc==ENC(db) );
            rc = sqlite3BtreeCursor(pCx->ub.pBtx, pCx->pgno_మూలం, BTREE_WRCSR,
                p_కీలకం_Info, pCx->uc.pCursor);
          }
          pCx->is_పట్టిక = 0;
        }లేకపోతే{
          pCx->pgno_మూలం = SCHEMA_మూలం;
          rc = sqlite3BtreeCursor(pCx->ub.pBtx, SCHEMA_మూలం, BTREE_WRCSR,
              0, pCx->uc.pCursor);
          pCx->is_పట్టిక = 1;
        }
      }
      pCx->isOrdered = (pOp->p5!=BTREE_UNORDERED);
      ప్రతిజ్ఞా( p->apCsr[pOp->p1]==pCx );
      ఒకవేళ( rc ){
        ప్రతిజ్ఞా( !sqlite3BtreeClosesWithCursor(pCx->ub.pBtx, pCx->uc.pCursor) );
        sqlite3BtreeClose(pCx->ub.pBtx);
        p->apCsr[pOp->p1] = 0;  /* Not required; helps with static analysis */
      }లేకపోతే{
        ప్రతిజ్ఞా( sqlite3BtreeClosesWithCursor(pCx->ub.pBtx, pCx->uc.pCursor) );
        ఒకవేళ( pOp->opcode==OP_OpenAutoindex ){
          sqlite3BtreeCursorHintFlags(pCx->uc.pCursor, BTREE_SEEK_EQ);
        }
      }
    }
  }
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  pCx->null_అడ్డువరుస = 1;
  ఆపు;
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
స్థితి OP_SorterOpen: {
  VdbeCursor *pCx;

  ప్రతిజ్ఞా( pOp->p1>=0 );
  ప్రతిజ్ఞా( pOp->p2>=0 );
  pCx = allocateCursor(p, pOp->p1, pOp->p2, CURTYPE_SORTER);
  ఒకవేళ( pCx==0 ) వెళ్ళు no_mem;
  pCx->p_కీలకం_Info = pOp->p4.p_కీలకం_Info;
  ప్రతిజ్ఞా( pCx->p_కీలకం_Info->db==db );
  ప్రతిజ్ఞా( pCx->p_కీలకం_Info->enc==ENC(db) );
  rc = sqlite3VdbeSorterInit(db, pOp->p3, pCx);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}

/* Opcode: SequenceTest P1 P2 * * *
** Synopsis: if( cursor[P1].ctr++ ) pc = P2
**
** P1 is a sorter cursor. If the sequence counter is currently zero, jump
** to P2. Regardless of whether or not the jump is taken, increment the
** the sequence value.
*/
స్థితి OP_SequenceTest: {
  VdbeCursor *pC;
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( isSorter(pC) );
  ఒకవేళ( (pC->seq_లెక్క++)==0 ){
    వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
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
స్థితి OP_OpenPseudo: {
  VdbeCursor *pCx;

  ప్రతిజ్ఞా( pOp->p1>=0 );
  ప్రతిజ్ఞా( pOp->p3>=0 );
  pCx = allocateCursor(p, pOp->p1, pOp->p3, CURTYPE_PSEUDO);
  ఒకవేళ( pCx==0 ) వెళ్ళు no_mem;
  pCx->null_అడ్డువరుస = 1;
  pCx->seek_ఫలితం = pOp->p2;
  pCx->is_పట్టిక = 1;
  /* Give this pseudo-cursor a fake BtCursor pointer so that pCx
  ** can be safely passed to sqlite3VdbeCursorMoveto().  This avoids a test
  ** for pCx->eCurType==CURTYPE_BTREE inside of sqlite3VdbeCursorMoveto()
  ** which is a performance optimization */
  pCx->uc.pCursor = sqlite3BtreeFakeValidCursor();
  ప్రతిజ్ఞా( pOp->p5==0 );
  ఆపు;
}

/* Opcode: Close P1 * * * *
**
** Close a cursor previously opened as P1.  If P1 is not
** currently open, this instruction is a no-op.
*/
స్థితి OP_Close: {             /* ncycle */
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  sqlite3VdbeFreeCursor(p, p->apCsr[pOp->p1]);
  p->apCsr[pOp->p1] = 0;
  ఆపు;
}

#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_వాడిన_ముసుగు
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
స్థితి OP_Columns_వాడిన: {
  VdbeCursor *pC;
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  pC->ముసుగు_వాడిన = INT32_గమ్యం_64(pOp->p2,pOp->p3);
  ఆపు;
}
#అయితే_ముగింపు

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
స్థితి OP_SeekLT:         /* jump0, in3, group, ncycle */
స్థితి OP_SeekLE:         /* jump0, in3, group, ncycle */
స్థితి OP_SeekGE:         /* jump0, in3, group, ncycle */
స్థితి OP_SeekGT: {       /* jump0, in3, group, ncycle */
  పూర్ణాంకం res;           /* Comparison result */
  పూర్ణాంకం oc;            /* Opcode */
  VdbeCursor *pC;    /* The cursor to seek */
  Unpacked_రికార్డు r;  /* The key to seek for */
  పూర్ణాంకం n_క్షేత్రం;        /* Number of columns or fields in the key */
  i64 i_కీలకం;          /* The rowid we are to seek to */
  పూర్ణాంకం eqOnly;        /* Only interested in == results */

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p2!=0 );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( OP_SeekLE == OP_SeekLT+1 );
  ప్రతిజ్ఞా( OP_SeekGE == OP_SeekLT+2 );
  ప్రతిజ్ఞా( OP_SeekGT == OP_SeekLT+3 );
  ప్రతిజ్ఞా( pC->isOrdered );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
  oc = pOp->opcode;
  eqOnly = 0;
  pC->null_అడ్డువరుస = 0;
#నిర్వచితమైతే SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#అయితే_ముగింపు

  pC->deferredMoveto = 0;
  pC->cache_స్థితి = CACHE_STALE;
  ఒకవేళ( pC->is_పట్టిక ){
    u16 flags3, newType;
    /* The OPFLAG_SEEKEQ/BTREE_SEEK_EQ flag is only set on index cursors */
    ప్రతిజ్ఞా( sqlite3BtreeCursorHasHint(pC->uc.pCursor, BTREE_SEEK_EQ)==0
              || CORRUPT_DB );

    /* The input value in P3 might be of any type: integer, real, string,
    ** blob, or NULL.  But it needs to be an integer before we can do
    ** the seek, so convert it. */
    pIn3 = &aMem[pOp->p3];
    flags3 = pIn3->flags;
    ఒకవేళ( (flags3 & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Str))==MEM_Str ){
      applyNumericAffinity(pIn3, 0);
    }
    i_కీలకం = sqlite3_Vdbe_Int_విలువ(pIn3); /* Get the integer key value */
    newType = pIn3->flags; /* Record the type after applying numeric affinity */
    pIn3->flags = flags3;  /* But convert the type back to its original */

    /* If the P3 value could not be converted into an integer without
    ** loss of information, then special processing is required... */
    ఒకవేళ( (newType & (MEM_Int|MEM_IntReal))==0 ){
      పూర్ణాంకం c;
      ఒకవేళ( (newType & MEM_Real)==0 ){
        ఒకవేళ( (newType & MEM_Null) || oc>=OP_SeekGE ){
          Vdbe_Branch_తీసుకున్న(1,2);
          వెళ్ళు jump_గమ్యం_p2;
        }లేకపోతే{
          rc = sqlite3_Btree_చివరి(pC->uc.pCursor, &res);
          ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abort_due_గమ్యం_error;
          వెళ్ళు seek_not_దొరికింది;
        }
      }
      c = sqlite3IntFloatCompare(i_కీలకం, pIn3->u.r);

      /* If the approximation iKey is larger than the actual real search
      ** term, substitute >= for > and < for <=. e.g. if the search term
      ** is 4.9 and the integer approximation 5:
      **
      **        (x >  4.9)    ->     (x >= 5)
      **        (x <= 4.9)    ->     (x <  5)
      */
      ఒకవేళ( c>0 ){
        ప్రతిజ్ఞా( OP_SeekGE==(OP_SeekGT-1) );
        ప్రతిజ్ఞా( OP_SeekLT==(OP_SeekLE-1) );
        ప్రతిజ్ఞా( (OP_SeekLE & 0x0001)==(OP_SeekGT & 0x0001) );
        ఒకవేళ( (oc & 0x0001)==(OP_SeekGT & 0x0001) ) oc--;
      }

      /* If the approximation iKey is smaller than the actual real search
      ** term, substitute <= for < and > for >=.  */
      లేకపోతే ఒకవేళ( c<0 ){
        ప్రతిజ్ఞా( OP_SeekLE==(OP_SeekLT+1) );
        ప్రతిజ్ఞా( OP_SeekGT==(OP_SeekGE+1) );
        ప్రతిజ్ఞా( (OP_SeekLT & 0x0001)==(OP_SeekGE & 0x0001) );
        ఒకవేళ( (oc & 0x0001)==(OP_SeekLT & 0x0001) ) oc++;
      }
    }
    rc = sqlite3_Btree_పట్టిక_Moveto(pC->uc.pCursor, (u64)i_కీలకం, 0, &res);
    pC->moveto_లక్ష్యం = i_కీలకం;  /* Used by OP_Delete */
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు abort_due_గమ్యం_error;
    }
  }లేకపోతే{
    /* For a cursor with the OPFLAG_SEEKEQ/BTREE_SEEK_EQ hint, only the
    ** OP_SeekGE and OP_SeekLE opcodes are allowed, and these must be
    ** immediately followed by an OP_IdxGT or OP_IdxLT opcode, respectively,
    ** with the same key.
    */
    ఒకవేళ( sqlite3BtreeCursorHasHint(pC->uc.pCursor, BTREE_SEEK_EQ) ){
      eqOnly = 1;
      ప్రతిజ్ఞా( pOp->opcode==OP_SeekGE || pOp->opcode==OP_SeekLE );
      ప్రతిజ్ఞా( pOp[1].opcode==OP_IdxLT || pOp[1].opcode==OP_IdxGT );
      ప్రతిజ్ఞా( pOp->opcode==OP_SeekGE || pOp[1].opcode==OP_IdxLT );
      ప్రతిజ్ఞా( pOp->opcode==OP_SeekLE || pOp[1].opcode==OP_IdxGT );
      ప్రతిజ్ఞా( pOp[1].p1==pOp[0].p1 );
      ప్రతిజ్ఞా( pOp[1].p2<=pOp[0].p2 );
      testcase( pOp->opcode==OP_SeekGE && pOp[1].p2<pOp[0].p2 );
      testcase( pOp->opcode==OP_SeekLE && pOp[1].p2<pOp[0].p2 );
      ప్రతిజ్ఞా( pOp[1].p3==pOp[0].p3 );
      ప్రతిజ్ఞా( pOp[1].p4.i==pOp[0].p4.i );
    }

    n_క్షేత్రం = pOp->p4.i;
    ప్రతిజ్ఞా( pOp->p4type==P4_INT32 );
    ప్రతిజ్ఞా( n_క్షేత్రం>0 );
    r.p_కీలకం_Info = pC->p_కీలకం_Info;
    r.n_క్షేత్రం = (u16)n_క్షేత్రం;

    /* The next line of code computes as follows, only faster:
    **   if( oc==OP_SeekGT || oc==OP_SeekLE ){
    **     r.default_rc = -1;
    **   }else{
    **     r.default_rc = +1;
    **   }
    */
    r.default_rc = ((1 & (oc - OP_SeekLT)) ? -1 : +1);
    ప్రతిజ్ఞా( oc!=OP_SeekGT || r.default_rc==-1 );
    ప్రతిజ్ఞా( oc!=OP_SeekLE || r.default_rc==-1 );
    ప్రతిజ్ఞా( oc!=OP_SeekGE || r.default_rc==+1 );
    ప్రతిజ్ఞా( oc!=OP_SeekLT || r.default_rc==+1 );

    r.aMem = &aMem[pOp->p3];
#నిర్వచితమైతే SQLITE_DEBUG
    {
      పూర్ణాంకం i;
      కొరకు(i=0; i<r.n_క్షేత్రం; i++){
        ప్రతిజ్ఞా( memIsValid(&r.aMem[i]) );
        ఒకవేళ( i>0 ) రిజిస్టరు_TRACE(pOp->p3+i, &r.aMem[i]);
      }
    }
#అయితే_ముగింపు
    r.eq_చూసిన = 0;
    rc = sqlite3_Btree_సూచీ_Moveto(pC->uc.pCursor, &r, &res);
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు abort_due_గమ్యం_error;
    }
    ఒకవేళ( eqOnly && r.eq_చూసిన==0 ){
      ప్రతిజ్ఞా( res!=0 );
      వెళ్ళు seek_not_దొరికింది;
    }
  }
#నిర్వచితమైతే SQLITE_TEST
  sqlite3_శోధన_లెక్క++;
#అయితే_ముగింపు
  ఒకవేళ( oc>=OP_SeekGE ){  ప్రతిజ్ఞా( oc==OP_SeekGE || oc==OP_SeekGT );
    ఒకవేళ( res<0 || (res==0 && oc==OP_SeekGT) ){
      res = 0;
      rc = sqlite3_Btree_తదుపరి(pC->uc.pCursor, 0);
      ఒకవేళ( rc!=SQLITE_OK ){
        ఒకవేళ( rc==SQLITE_పూర్తి ){
          rc = SQLITE_OK;
          res = 1;
        }లేకపోతే{
          వెళ్ళు abort_due_గమ్యం_error;
        }
      }
    }లేకపోతే{
      res = 0;
    }
  }లేకపోతే{
    ప్రతిజ్ఞా( oc==OP_SeekLT || oc==OP_SeekLE );
    ఒకవేళ( res>0 || (res==0 && oc==OP_SeekLT) ){
      res = 0;
      rc = sqlite3_Btree_మునుపటి(pC->uc.pCursor, 0);
      ఒకవేళ( rc!=SQLITE_OK ){
        ఒకవేళ( rc==SQLITE_పూర్తి ){
          rc = SQLITE_OK;
          res = 1;
        }లేకపోతే{
          వెళ్ళు abort_due_గమ్యం_error;
        }
      }
    }లేకపోతే{
      /* res might be negative because the table is empty.  Check to
      ** see if this is the case.
      */
      res = sqlite3BtreeEof(pC->uc.pCursor);
    }
  }
seek_not_దొరికింది:
  ప్రతిజ్ఞా( pOp->p2>0 );
  Vdbe_Branch_తీసుకున్న(res!=0,2);
  ఒకవేళ( res ){
    వెళ్ళు jump_గమ్యం_p2;
  }లేకపోతే ఒకవేళ( eqOnly ){
    ప్రతిజ్ఞా( pOp[1].opcode==OP_IdxLT || pOp[1].opcode==OP_IdxGT );
    pOp++; /* Skip the OP_IdxLt or OP_IdxGT that follows */
  }
  ఆపు;
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
స్థితి OP_Seek_పరిశీలన: {          /* ncycle */
  VdbeCursor *pC;
  పూర్ణాంకం res;
  పూర్ణాంకం n_అడుగు;
  Unpacked_రికార్డు r;

  ప్రతిజ్ఞా( pOp[1].opcode==OP_SeekGE );

  /* If pOp->p5 is clear, then pOp->p2 points to the first instruction past the
  ** OP_IdxGT that follows the OP_SeekGE. Otherwise, it points to the first
  ** opcode past the OP_SeekGE itself.  */
  ప్రతిజ్ఞా( pOp->p2>=(పూర్ణాంకం)(pOp-aOp)+2 );
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pOp->p5==0 ){
    /* There are no inequality constraints following the IN constraint. */
    ప్రతిజ్ఞా( pOp[1].p1==aOp[pOp->p2-1].p1 );
    ప్రతిజ్ఞా( pOp[1].p2==aOp[pOp->p2-1].p2 );
    ప్రతిజ్ఞా( pOp[1].p3==aOp[pOp->p2-1].p3 );
    ప్రతిజ్ఞా( aOp[pOp->p2-1].opcode==OP_IdxGT
         || aOp[pOp->p2-1].opcode==OP_IdxGE );
    testcase( aOp[pOp->p2-1].opcode==OP_IdxGE );
  }లేకపోతే{
    /* There are inequality constraints.  */
    ప్రతిజ్ఞా( pOp->p2==(పూర్ణాంకం)(pOp-aOp)+2 );
    ప్రతిజ్ఞా( aOp[pOp->p2-1].opcode==OP_SeekGE );
  }
#అయితే_ముగింపు

  ప్రతిజ్ఞా( pOp->p1>0 );
  pC = p->apCsr[pOp[1].p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( !pC->is_పట్టిక );
  ఒకవేళ( !sqlite3BtreeCursorIsValidNN(pC->uc.pCursor) ){
#నిర్వచితమైతే SQLITE_DEBUG
     ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
       రాయి("... cursor not valid - fall through\n");
     }       
#అయితే_ముగింపు
    ఆపు;
  }
  n_అడుగు = pOp->p1;
  ప్రతిజ్ఞా( n_అడుగు>=1 );
  r.p_కీలకం_Info = pC->p_కీలకం_Info;
  r.n_క్షేత్రం = (u16)pOp[1].p4.i;
  r.default_rc = 0;
  r.aMem = &aMem[pOp[1].p3];
#నిర్వచితమైతే SQLITE_DEBUG
  {
    పూర్ణాంకం i;
    కొరకు(i=0; i<r.n_క్షేత్రం; i++){
      ప్రతిజ్ఞా( memIsValid(&r.aMem[i]) );
      రిజిస్టరు_TRACE(pOp[1].p3+i, &aMem[pOp[1].p3+i]);
    }
  }
#అయితే_ముగింపు
  res = 0;  /* Not needed.  Only used to silence a warning. */
  అయితే(1){
    rc = sqlite3_Vdbe_Idx_కీలకం_Compare(db, pC, &r, &res);
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
    ఒకవేళ( res>0 && pOp->p5==0 ){
      seekscan_శోధన_fail:
      /* Jump to SeekGE.P2, ending the loop */
#నిర్వచితమైతే SQLITE_DEBUG
      ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
        రాయి("... %d steps and then skip\n", pOp->p1 - n_అడుగు);
      }       
#అయితే_ముగింపు
      Vdbe_Branch_తీసుకున్న(1,3);
      pOp++;
      వెళ్ళు jump_గమ్యం_p2;
    }
    ఒకవేళ( res>=0 ){
      /* Jump to This.P2, bypassing the OP_SeekGE opcode */
#నిర్వచితమైతే SQLITE_DEBUG
      ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
        రాయి("... %d steps and then success\n", pOp->p1 - n_అడుగు);
      }       
#అయితే_ముగింపు
      Vdbe_Branch_తీసుకున్న(2,3);
      వెళ్ళు jump_గమ్యం_p2;
      ఆపు;
    }
    ఒకవేళ( n_అడుగు<=0 ){
#నిర్వచితమైతే SQLITE_DEBUG
      ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
        రాయి("... fall through after %d steps\n", pOp->p1);
      }       
#అయితే_ముగింపు
      Vdbe_Branch_తీసుకున్న(0,3);
      ఆపు;
    }
    n_అడుగు--;
    pC->cache_స్థితి = CACHE_STALE;
    rc = sqlite3_Btree_తదుపరి(pC->uc.pCursor, 0);
    ఒకవేళ( rc ){
      ఒకవేళ( rc==SQLITE_పూర్తి ){
        rc = SQLITE_OK;
        వెళ్ళు seekscan_శోధన_fail;
      }లేకపోతే{
        వెళ్ళు abort_due_గమ్యం_error;
      }
    }
  }
 
  ఆపు;
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
స్థితి OP_Seek_తగిలింది: {           /* ncycle */
  VdbeCursor *pC;
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pOp->p3>=pOp->p2 );
  ఒకవేళ( pC->seek_తగిలింది<pOp->p2 ){
#నిర్వచితమైతే SQLITE_DEBUG
    ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
      రాయి("seekHit changes from %d to %d\n", pC->seek_తగిలింది, pOp->p2);
    }       
#అయితే_ముగింపు
    pC->seek_తగిలింది = pOp->p2;
  }లేకపోతే ఒకవేళ( pC->seek_తగిలింది>pOp->p3 ){
#నిర్వచితమైతే SQLITE_DEBUG
    ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
      రాయి("seekHit changes from %d to %d\n", pC->seek_తగిలింది, pOp->p3);
    }       
#అయితే_ముగింపు
    pC->seek_తగిలింది = pOp->p3;
  }
  ఆపు;
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
స్థితి OP_IfNoHope: {     /* jump, in3, ncycle */
  VdbeCursor *pC;
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
    రాయి("seekHit is %d\n", pC->seek_తగిలింది);
  }       
#అయితే_ముగింపు
  ఒకవేళ( pC->seek_తగిలింది>=pOp->p4.i ) ఆపు;
  /* Fall through into OP_NotFound */
  /* no break */ deliberate_fall_through
}
స్థితి OP_NoConflict:     /* jump, in3, ncycle */
స్థితి OP_Not_దొరికింది:       /* jump, in3, ncycle */
స్థితి OP_దొరికింది: {        /* jump, in3, ncycle */
  పూర్ణాంకం alreadyExists;
  పూర్ణాంకం ii;
  VdbeCursor *pC;
  Unpacked_రికార్డు *p_Idx_కీలకం;
  Unpacked_రికార్డు r;

#నిర్వచితమైతే SQLITE_TEST
  ఒకవేళ( pOp->opcode!=OP_NoConflict ) sqlite3_దొరికింది_లెక్క++;
#అయితే_ముగింపు

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p4type==P4_INT32 );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
#నిర్వచితమైతే SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#అయితే_ముగింపు
  r.aMem = &aMem[pOp->p3];
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
  ప్రతిజ్ఞా( pC->is_పట్టిక==0 );
  r.n_క్షేత్రం = (u16)pOp->p4.i;
  ఒకవేళ( r.n_క్షేత్రం>0 ){
    /* Key values in an array of registers */
    r.p_కీలకం_Info = pC->p_కీలకం_Info;
    r.default_rc = 0;
#నిర్వచితమైతే SQLITE_DEBUG
    (శూన్య)sqlite3FaultSim(50);  /* For use by --counter in TH3 */
    కొరకు(ii=0; ii<r.n_క్షేత్రం; ii++){
      ప్రతిజ్ఞా( memIsValid(&r.aMem[ii]) );
      ప్రతిజ్ఞా( (r.aMem[ii].flags & MEM_Zero)==0 || r.aMem[ii].n==0 );
      ఒకవేళ( ii ) రిజిస్టరు_TRACE(pOp->p3+ii, &r.aMem[ii]);
    }
#అయితే_ముగింపు
    rc = sqlite3_Btree_సూచీ_Moveto(pC->uc.pCursor, &r, &pC->seek_ఫలితం);
  }లేకపోతే{
    /* Composite key generated by OP_MakeRecord */
    ప్రతిజ్ఞా( r.aMem->flags & MEM_Blob );
    ప్రతిజ్ఞా( pOp->opcode!=OP_NoConflict );
    rc = ExpandBlob(r.aMem);
    ప్రతిజ్ఞా( rc==SQLITE_OK || rc==SQLITE_NOMEM );
    ఒకవేళ( rc ) వెళ్ళు no_mem;
    p_Idx_కీలకం = sqlite3_Vdbe_Alloc_Unpacked_రికార్డు(pC->p_కీలకం_Info);
    ఒకవేళ( p_Idx_కీలకం==0 ) వెళ్ళు no_mem;
    sqlite3_Vdbe_రికార్డు_Unpack(r.aMem->n, r.aMem->z, p_Idx_కీలకం);
    p_Idx_కీలకం->default_rc = 0;
    rc = sqlite3_Btree_సూచీ_Moveto(pC->uc.pCursor, p_Idx_కీలకం, &pC->seek_ఫలితం);
    sqlite3DbFreeNN(db, p_Idx_కీలకం);
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    వెళ్ళు abort_due_గమ్యం_error;
  }
  alreadyExists = (pC->seek_ఫలితం==0);
  pC->null_అడ్డువరుస = 1-alreadyExists;
  pC->deferredMoveto = 0;
  pC->cache_స్థితి = CACHE_STALE;
  ఒకవేళ( pOp->opcode==OP_దొరికింది ){
    Vdbe_Branch_తీసుకున్న(alreadyExists!=0,2);
    ఒకవేళ( alreadyExists ) వెళ్ళు jump_గమ్యం_p2;
  }లేకపోతే{
    ఒకవేళ( !alreadyExists ){
      Vdbe_Branch_తీసుకున్న(1,2);
      వెళ్ళు jump_గమ్యం_p2;
    }
    ఒకవేళ( pOp->opcode==OP_NoConflict ){
      /* For the OP_NoConflict opcode, take the jump if any of the
      ** input fields are NULL, since any key with a NULL will not
      ** conflict */
      కొరకు(ii=0; ii<r.n_క్షేత్రం; ii++){
        ఒకవేళ( r.aMem[ii].flags & MEM_Null ){
          Vdbe_Branch_తీసుకున్న(1,2);
          వెళ్ళు jump_గమ్యం_p2;
        }
      }
    }
    Vdbe_Branch_తీసుకున్న(0,2);
    ఒకవేళ( pOp->opcode==OP_IfNoHope ){
      pC->seek_తగిలింది = pOp->p4.i;
    }
  }
  ఆపు;
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
స్థితి OP_SeekRowid: {        /* jump0, in3, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  పూర్ణాంకం res;
  i64 i_కీలకం;

  pIn3 = &aMem[pOp->p3];
  testcase( pIn3->flags & MEM_Int );
  testcase( pIn3->flags & MEM_IntReal );
  testcase( pIn3->flags & MEM_Real );
  testcase( (pIn3->flags & (MEM_Str|MEM_Int))==MEM_Str );
  ఒకవేళ( (pIn3->flags & (MEM_Int|MEM_IntReal))==0 ){
    /* If pIn3->u.i does not contain an integer, compute iKey as the
    ** integer value of pIn3.  Jump to P2 if pIn3 cannot be converted
    ** into an integer without loss of information.  Take care to avoid
    ** changing the datatype of pIn3, however, as it is used by other
    ** parts of the prepared statement. */
    Mem x = pIn3[0];
    ఒకవేళ( x.flags & MEM_Str ){
      applyNumericAffinity(&x, 1);
    }
    ఒకవేళ( x.flags & MEM_Int ){
      i_కీలకం = x.u.i;
    }లేకపోతే
    ఒకవేళ( (x.flags & MEM_Real)==0
     || x.u.r < -9223372036854775808.0
     || x.u.r > 9223372036854774784.0
           /*   ^^^^^^^^^^^^^^^^^^^^^-- same value as every other double
           **   between 9223372036854774263.0 and 923372036854775295.0 */
     || (ద్విగుణ)(i_కీలకం = sqlite3_Real_గమ్యం_I64(x.u.r))!=x.u.r
    ){
      వెళ్ళు jump_గమ్యం_p2;
    }
    వెళ్ళు not_Exists_With_కీలకం;
  }
  /* Fall through into OP_NotExists */
  /* no break */ deliberate_fall_through
స్థితి OP_NotExists:          /* jump, in3, ncycle */
  pIn3 = &aMem[pOp->p3];
  ప్రతిజ్ఞా( (pIn3->flags & MEM_Int)!=0 || pOp->opcode==OP_SeekRowid );
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  i_కీలకం = pIn3->u.i;
not_Exists_With_కీలకం:
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pOp->opcode==OP_SeekRowid ) pC->seekOp = OP_SeekRowid;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( pC->is_పట్టిక );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  ప్రతిజ్ఞా( pCrsr!=0 );
  res = 0;
  rc = sqlite3_Btree_పట్టిక_Moveto(pCrsr, i_కీలకం, 0, &res);
  ప్రతిజ్ఞా( rc==SQLITE_OK || res==0 );
  pC->moveto_లక్ష్యం = i_కీలకం;  /* Used by OP_Delete */
  pC->null_అడ్డువరుస = 0;
  pC->cache_స్థితి = CACHE_STALE;
  pC->deferredMoveto = 0;
  Vdbe_Branch_తీసుకున్న(res!=0,2);
  pC->seek_ఫలితం = res;
  ఒకవేళ( res!=0 ){
    ప్రతిజ్ఞా( rc==SQLITE_OK );
    ఒకవేళ( pOp->p2==0 ){
      rc = SQLITE_CORRUPT_BKPT;
    }లేకపోతే{
      వెళ్ళు jump_గమ్యం_p2;
    }
  }
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}

/* Opcode: Sequence P1 P2 * * *
** Synopsis: r[P2]=cursor[P1].ctr++
**
** Find the next available sequence number for cursor P1.
** Write the sequence number into register P2.
** The sequence number on the cursor is incremented after this
** instruction. 
*/
స్థితి OP_Sequence: {           /* out2 */
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( p->apCsr[pOp->p1]!=0 );
  ప్రతిజ్ఞా( p->apCsr[pOp->p1]->eCurType!=CURTYPE_VTAB );
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->u.i = p->apCsr[pOp->p1]->seq_లెక్క++;
  ఆపు;
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
స్థితి OP_NewRowid: {           /* out2 */
  i64 v;                 /* The new rowid */
  VdbeCursor *pC;        /* Cursor of table to get the new rowid */
  పూర్ణాంకం res;               /* Result of an sqlite3BtreeLast() */
  పూర్ణాంకం cnt;               /* Counter to limit the number of searches */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINCREMENT
  Mem *pMem;             /* Register holding largest rowid for AUTOINCREMENT */
  Vdbe_చట్రం *p_చట్రం;     /* Root frame of VDBE */
#అయితే_ముగింపు

  v = 0;
  res = 0;
  p_బహిర్గతం = out2Prerelease(p, pOp);
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->is_పట్టిక );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
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
    ప్రతిజ్ఞా( pC->is_పట్టిక );

#నిర్వచితమైతే SQLITE_32_బిట్టు_ROWID
#   define MAX_ROWID 0x7fffffff
#లేకపోతే
    /* Some compilers complain about constants of the form 0x7fffffffffffffff.
    ** Others complain about 0x7ffffffffffffffffLL.  The following macro seems
    ** to provide the constant while making all compilers happy.
    */
#   define MAX_ROWID  (i64)( (((u64)0x7fffffff)<<32) | (u64)0xffffffff )
#అయితే_ముగింపు

    ఒకవేళ( !pC->useRandomRowid ){
      rc = sqlite3_Btree_చివరి(pC->uc.pCursor, &res);
      ఒకవేళ( rc!=SQLITE_OK ){
        వెళ్ళు abort_due_గమ్యం_error;
      }
      ఒకవేళ( res ){
        v = 1;   /* IMP: R-61914-48074 */
      }లేకపోతే{
        ప్రతిజ్ఞా( sqlite3BtreeCursorIsValid(pC->uc.pCursor) );
        v = sqlite3_Btree_Integer_కీలకం(pC->uc.pCursor);
        ఒకవేళ( v>=MAX_ROWID ){
          pC->useRandomRowid = 1;
        }లేకపోతే{
          v++;   /* IMP: R-29538-34987 */
        }
      }
    }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINCREMENT
    ఒకవేళ( pOp->p3 ){
      /* Assert that P3 is a valid memory cell. */
      ప్రతిజ్ఞా( pOp->p3>0 );
      ఒకవేళ( p->p_చట్రం ){
        కొరకు(p_చట్రం=p->p_చట్రం; p_చట్రం->p_మాతృ; p_చట్రం=p_చట్రం->p_మాతృ);
        /* Assert that P3 is a valid memory cell. */
        ప్రతిజ్ఞా( pOp->p3<=p_చట్రం->nMem );
        pMem = &p_చట్రం->aMem[pOp->p3];
      }లేకపోతే{
        /* Assert that P3 is a valid memory cell. */
        ప్రతిజ్ఞా( pOp->p3<=(p->nMem+1 - p->nCursor) );
        pMem = &aMem[pOp->p3];
        mem_About_గమ్యం_Change(p, pMem);
      }
      ప్రతిజ్ఞా( memIsValid(pMem) );

      రిజిస్టరు_TRACE(pOp->p3, pMem);
      sqlite3VdbeMemIntegerify(pMem);
      ప్రతిజ్ఞా( (pMem->flags & MEM_Int)!=0 );  /* mem(P3) holds an integer */
      ఒకవేళ( pMem->u.i==MAX_ROWID || pC->useRandomRowid ){
        rc = SQLITE_నిండిన;   /* IMP: R-17817-00630 */
        వెళ్ళు abort_due_గమ్యం_error;
      }
      ఒకవేళ( v<pMem->u.i+1 ){
        v = pMem->u.i + 1;
      }
      pMem->u.i = v;
    }
#అయితే_ముగింపు
    ఒకవేళ( pC->useRandomRowid ){
      /* IMPLEMENTATION-OF: R-07677-41881 If the largest ROWID is equal to the
      ** largest possible integer (9223372036854775807) then the database
      ** engine starts picking positive candidate ROWIDs at random until
      ** it finds one that is not previously used. */
      ప్రతిజ్ఞా( pOp->p3==0 );  /* We cannot be in random rowid mode if this is
                             ** an AUTOINCREMENT table. */
      cnt = 0;
      చేయి{
        sqlite3_randomness(పరిమాణం(v), &v);
        v &= (MAX_ROWID>>1); v++;  /* Ensure that v is greater than zero */
      }అయితే(  ((rc = sqlite3_Btree_పట్టిక_Moveto(pC->uc.pCursor, (u64)v,
                                                 0, &res))==SQLITE_OK)
            && (res==0)
            && (++cnt<100));
      ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
      ఒకవేళ( res==0 ){
        rc = SQLITE_నిండిన;   /* IMP: R-38219-53002 */
        వెళ్ళు abort_due_గమ్యం_error;
      }
      ప్రతిజ్ఞా( v>0 );  /* EV: R-40812-03570 */
    }
    pC->deferredMoveto = 0;
    pC->cache_స్థితి = CACHE_STALE;
  }
  p_బహిర్గతం->u.i = v;
  ఆపు;
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
స్థితి OP_చొప్పించు: {
  Mem *p_దత్తాంశం;       /* MEM cell holding data for the record to be inserted */
  Mem *p_కీలకం;        /* MEM cell holding key  for the record */
  VdbeCursor *pC;   /* Cursor to table into which insert is written */
  పూర్ణాంకం seek_ఫలితం;   /* Result of prior seek or 0 if no USESEEKRESULT flag */
  స్థిరరాశి అక్షరం *zDb;  /* database name - used by the update hook */
  పట్టిక *pTab;      /* Table structure - used by update and pre-update hooks */
  BtreePayload x;   /* Payload to be inserted */

  p_దత్తాంశం = &aMem[pOp->p2];
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( memIsValid(p_దత్తాంశం) );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
  ప్రతిజ్ఞా( (pOp->p5 & OPFLAG_ISNOOP) || pC->is_పట్టిక );
  ప్రతిజ్ఞా( pOp->p4type==P4_పట్టిక || pOp->p4type>=P4_STATIC );
  రిజిస్టరు_TRACE(pOp->p2, p_దత్తాంశం);
  sqlite3VdbeIncrWriteCounter(p, pC);

  p_కీలకం = &aMem[pOp->p3];
  ప్రతిజ్ఞా( p_కీలకం->flags & MEM_Int );
  ప్రతిజ్ఞా( memIsValid(p_కీలకం) );
  రిజిస్టరు_TRACE(pOp->p3, p_కీలకం);
  x.n_కీలకం = p_కీలకం->u.i;

  ఒకవేళ( pOp->p4type==P4_పట్టిక && HAS_UPDATE_HOOK(db) ){
    ప్రతిజ్ఞా( pC->iDb>=0 );
    zDb = db->aDb[pC->iDb].zDbSName;
    pTab = pOp->p4.pTab;
    ప్రతిజ్ఞా( (pOp->p5 & OPFLAG_ISNOOP) || HasRowid(pTab) );
  }లేకపోతే{
    pTab = 0;
    zDb = 0;
  }

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
  /* Invoke the pre-update hook, if any */
  ఒకవేళ( pTab ){
    ఒకవేళ( db->xPreUpdateCallback && !(pOp->p5 & OPFLAG_ISUPDATE) ){
      sqlite3VdbePreUpdateHook(p,pC,SQLITE_చొప్పించు,zDb,pTab,x.n_కీలకం,pOp->p2,-1);
    }
    ఒకవేళ( db->xUpdateCallback==0 || pTab->a_నిలువు==0 ){
      /* Prevent post-update hook from running in cases when it should not */
      pTab = 0;
    }
  }
  ఒకవేళ( pOp->p5 & OPFLAG_ISNOOP ) ఆపు;
#అయితే_ముగింపు

  ప్రతిజ్ఞా( (pOp->p5 & OPFLAG_LASTROWID)==0 || (pOp->p5 & OPFLAG_NCHANGE)!=0 );
  ఒకవేళ( pOp->p5 & OPFLAG_NCHANGE ){
    p->nChange++;
    ఒకవేళ( pOp->p5 & OPFLAG_LASTROWID ) db->చివరి_Rowid = x.n_కీలకం;
  }
  ప్రతిజ్ఞా( (p_దత్తాంశం->flags & (MEM_Blob|MEM_Str))!=0 || p_దత్తాంశం->n==0 );
  x.p_దత్తాంశం = p_దత్తాంశం->z;
  x.n_దత్తాంశం = p_దత్తాంశం->n;
  seek_ఫలితం = ((pOp->p5 & OPFLAG_USESEEKRESULT) ? pC->seek_ఫలితం : 0);
  ఒకవేళ( p_దత్తాంశం->flags & MEM_Zero ){
    x.nZero = p_దత్తాంశం->u.nZero;
  }లేకపోతే{
    x.nZero = 0;
  }
  x.p_కీలకం = 0;
  ప్రతిజ్ఞా( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  rc = sqlite3_Btree_చొప్పించు(pC->uc.pCursor, &x,
      (pOp->p5 & (OPFLAG_APPEND|OPFLAG_SAVEPOSITION|OPFLAG_PREFORMAT)),
      seek_ఫలితం
  );
  pC->deferredMoveto = 0;
  pC->cache_స్థితి = CACHE_STALE;
  నిలువు_Cache_Ctr++;

  /* Invoke the update-hook if required. */
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఒకవేళ( pTab ){
    ప్రతిజ్ఞా( db->xUpdateCallback!=0 );
    ప్రతిజ్ఞా( pTab->a_నిలువు!=0 );
    db->xUpdateCallback(db->pUpdateArg,
           (pOp->p5 & OPFLAG_ISUPDATE) ? SQLITE_UPDATE : SQLITE_చొప్పించు,
           zDb, pTab->zName, x.n_కీలకం);
  }
  ఆపు;
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
స్థితి OP_అడ్డువరుస_Cell: {
  VdbeCursor *pDest;              /* Cursor to write to */
  VdbeCursor *pSrc;               /* Cursor to read from */
  i64 i_కీలకం;                       /* Rowid value to insert with */
  ప్రతిజ్ఞా( pOp[1].opcode==OP_చొప్పించు || pOp[1].opcode==OP_Idx_చొప్పించు );
  ప్రతిజ్ఞా( pOp[1].opcode==OP_చొప్పించు    || pOp->p3==0 );
  ప్రతిజ్ఞా( pOp[1].opcode==OP_Idx_చొప్పించు || pOp->p3>0 );
  ప్రతిజ్ఞా( pOp[1].p5 & OPFLAG_PREFORMAT );
  pDest = p->apCsr[pOp->p1];
  pSrc = p->apCsr[pOp->p2];
  i_కీలకం = pOp->p3 ? aMem[pOp->p3].u.i : 0;
  rc = sqlite3_Btree_Transfer_అడ్డువరుస(pDest->uc.pCursor, pSrc->uc.pCursor, i_కీలకం);
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
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
స్థితి OP_Delete: {
  VdbeCursor *pC;
  స్థిరరాశి అక్షరం *zDb;
  పట్టిక *pTab;
  పూర్ణాంకం opflags;

  opflags = pOp->p2;
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  sqlite3VdbeIncrWriteCounter(p, pC);

#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pOp->p4type==P4_పట్టిక
   && HasRowid(pOp->p4.pTab)
   && pOp->p5==0
   && sqlite3BtreeCursorIsValidNN(pC->uc.pCursor)
  ){
    /* If p5 is zero, the seek operation that positioned the cursor prior to
    ** OP_Delete will have also set the pC->movetoTarget field to the rowid of
    ** the row that is being deleted */
    i64 i_కీలకం = sqlite3_Btree_Integer_కీలకం(pC->uc.pCursor);
    ప్రతిజ్ఞా( CORRUPT_DB || pC->moveto_లక్ష్యం==i_కీలకం );
  }
#అయితే_ముగింపు

  /* If the update-hook or pre-update-hook will be invoked, set zDb to
  ** the name of the db to pass as to it. Also set local pTab to a copy
  ** of p4.pTab. Finally, if p5 is true, indicating that this cursor was
  ** last moved with OP_Next or OP_Prev, not Seek or NotFound, set
  ** VdbeCursor.movetoTarget to the current rowid.  */
  ఒకవేళ( pOp->p4type==P4_పట్టిక && HAS_UPDATE_HOOK(db) ){
    ప్రతిజ్ఞా( pC->iDb>=0 );
    ప్రతిజ్ఞా( pOp->p4.pTab!=0 );
    zDb = db->aDb[pC->iDb].zDbSName;
    pTab = pOp->p4.pTab;
    ఒకవేళ( (pOp->p5 & OPFLAG_SAVEPOSITION)!=0 && pC->is_పట్టిక ){
      pC->moveto_లక్ష్యం = sqlite3_Btree_Integer_కీలకం(pC->uc.pCursor);
    }
  }లేకపోతే{
    zDb = 0;
    pTab = 0;
  }

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
  /* Invoke the pre-update-hook if required. */
  ప్రతిజ్ఞా( db->xPreUpdateCallback==0 || pTab==pOp->p4.pTab );
  ఒకవేళ( db->xPreUpdateCallback && pTab ){
    ప్రతిజ్ఞా( !(opflags & OPFLAG_ISUPDATE)
         || HasRowid(pTab)==0
         || (aMem[pOp->p3].flags & MEM_Int)
    );
    sqlite3VdbePreUpdateHook(p, pC,
        (opflags & OPFLAG_ISUPDATE) ? SQLITE_UPDATE : SQLITE_DELETE,
        zDb, pTab, pC->moveto_లక్ష్యం,
        pOp->p3, -1
    );
  }
  ఒకవేళ( opflags & OPFLAG_ISNOOP ) ఆపు;
#అయితే_ముగింపు

  /* Only flags that can be set are SAVEPOISTION and AUXDELETE */
  ప్రతిజ్ఞా( (pOp->p5 & ~(OPFLAG_SAVEPOSITION|OPFLAG_AUXDELETE))==0 );
  ప్రతిజ్ఞా( OPFLAG_SAVEPOSITION==BTREE_SAVEPOSITION );
  ప్రతిజ్ఞా( OPFLAG_AUXDELETE==BTREE_AUXDELETE );

#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( p->p_చట్రం==0 ){
    ఒకవేళ( pC->isEphemeral==0
        && (pOp->p5 & OPFLAG_AUXDELETE)==0
        && (pC->wrFlag & OPFLAG_FORDELETE)==0
      ){
      nExtraDelete++;
    }
    ఒకవేళ( pOp->p2 & OPFLAG_NCHANGE ){
      nExtraDelete--;
    }
  }
#అయితే_ముగింపు

  rc = sqlite3BtreeDelete(pC->uc.pCursor, pOp->p5);
  pC->cache_స్థితి = CACHE_STALE;
  నిలువు_Cache_Ctr++;
  pC->seek_ఫలితం = 0;
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;

  /* Invoke the update-hook if required. */
  ఒకవేళ( opflags & OPFLAG_NCHANGE ){
    p->nChange++;
    ఒకవేళ( db->xUpdateCallback && ALWAYS(pTab!=0) && HasRowid(pTab) ){
      db->xUpdateCallback(db->pUpdateArg, SQLITE_DELETE, zDb, pTab->zName,
          pC->moveto_లక్ష్యం);
      ప్రతిజ్ఞా( pC->iDb>=0 );
    }
  }

  ఆపు;
}
/* Opcode: ResetCount * * * * *
**
** The value of the change counter is copied to the database handle
** change counter (returned by subsequent calls to sqlite3_changes()).
** Then the VMs internal change counter resets to 0.
** This is used by trigger programs.
*/
స్థితి OP_Reset_లెక్క: {
  sqlite3_Vdbe_సమితి_Changes(db, p->nChange);
  p->nChange = 0;
  ఆపు;
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
స్థితి OP_SorterCompare: {
  VdbeCursor *pC;
  పూర్ణాంకం res;
  పూర్ణాంకం n_కీలకం_నిలువు;

  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( isSorter(pC) );
  ప్రతిజ్ఞా( pOp->p4type==P4_INT32 );
  pIn3 = &aMem[pOp->p3];
  n_కీలకం_నిలువు = pOp->p4.i;
  res = 0;
  rc = sqlite3VdbeSorterCompare(pC, pIn3, n_కీలకం_నిలువు, &res);
  Vdbe_Branch_తీసుకున్న(res!=0,2);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఒకవేళ( res ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
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
స్థితి OP_Sorter_దత్తాంశం: {       /* ncycle */
  VdbeCursor *pC;

  p_బహిర్గతం = &aMem[pOp->p2];
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( isSorter(pC) );
  rc = sqlite3VdbeSorterRowkey(pC, p_బహిర్గతం);
  ప్రతిజ్ఞా( rc!=SQLITE_OK || (p_బహిర్గతం->flags & MEM_Blob) );
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  p->apCsr[pOp->p3]->cache_స్థితి = CACHE_STALE;
  ఆపు;
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
స్థితి OP_అడ్డువరుస_దత్తాంశం: {
  VdbeCursor *pC;
  BtCursor *pCrsr;
  u32 n;

  p_బహిర్గతం = out2Prerelease(p, pOp);

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( isSorter(pC)==0 );
  ప్రతిజ్ఞా( pC->null_అడ్డువరుస==0 );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
  pCrsr = pC->uc.pCursor;

  /* The OP_RowData opcodes always follow OP_NotExists or
  ** OP_SeekRowid or OP_Rewind/Op_Next with no intervening instructions
  ** that might invalidate the cursor.
  ** If this were not the case, one of the following assert()s
  ** would fail.  Should this ever change (because of changes in the code
  ** generator) then the fix would be to insert a call to
  ** sqlite3VdbeCursorMoveto().
  */
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  ప్రతిజ్ఞా( sqlite3BtreeCursorIsValid(pCrsr) );

  n = sqlite3_Btree_Payload_కొలత(pCrsr);
  ఒకవేళ( n>(u32)db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
    వెళ్ళు too_big;
  }
  testcase( n==0 );
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(pCrsr, n, p_బహిర్గతం);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఒకవేళ( !pOp->p3 ) Deephemeralize(p_బహిర్గతం);
  UPDATE_MAX_BLOBSIZE(p_బహిర్గతం);
  రిజిస్టరు_TRACE(pOp->p2, p_బహిర్గతం);
  ఆపు;
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
స్థితి OP_Rowid: {                 /* out2, ncycle */
  VdbeCursor *pC;
  i64 v;
  sqlite3_vtab *pVtab;
  స్థిరరాశి sqlite3_module *pModule;

  p_బహిర్గతం = out2Prerelease(p, pOp);
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType!=CURTYPE_PSEUDO || pC->null_అడ్డువరుస );
  ఒకవేళ( pC->null_అడ్డువరుస ){
    p_బహిర్గతం->flags = MEM_Null;
    ఆపు;
  }లేకపోతే ఒకవేళ( pC->deferredMoveto ){
    v = pC->moveto_లక్ష్యం;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  }లేకపోతే ఒకవేళ( pC->eCurType==CURTYPE_VTAB ){
    ప్రతిజ్ఞా( pC->uc.pVCur!=0 );
    pVtab = pC->uc.pVCur->pVtab;
    pModule = pVtab->pModule;
    ప్రతిజ్ఞా( pModule->xRowid );
    rc = pModule->xRowid(pC->uc.pVCur, &v);
    sqlite3VtabImportErrmsg(p, pVtab);
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */
  }లేకపోతే{
    ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
    ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
    rc = sqlite3VdbeCursorRestore(pC);
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
    ఒకవేళ( pC->null_అడ్డువరుస ){
      p_బహిర్గతం->flags = MEM_Null;
      ఆపు;
    }
    v = sqlite3_Btree_Integer_కీలకం(pC->uc.pCursor);
  }
  p_బహిర్గతం->u.i = v;
  ఆపు;
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
స్థితి OP_Null_అడ్డువరుస: {
  VdbeCursor *pC;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ఒకవేళ( pC==0 ){
    /* If the cursor is not already open, create a special kind of
    ** pseudo-cursor that always gives null rows. */
    pC = allocateCursor(p, pOp->p1, 1, CURTYPE_PSEUDO);
    ఒకవేళ( pC==0 ) వెళ్ళు no_mem;
    pC->seek_ఫలితం = 0;
    pC->is_పట్టిక = 1;
    pC->noReuse = 1;
    pC->uc.pCursor = sqlite3BtreeFakeValidCursor();
  }
  pC->null_అడ్డువరుస = (pOp->p2==0);
  pC->cache_స్థితి = CACHE_STALE;
  ఒకవేళ( pC->eCurType==CURTYPE_BTREE ){
    ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
    sqlite3BtreeClearCursor(pC->uc.pCursor);
  }
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pC->seekOp==0 ) pC->seekOp = OP_Null_అడ్డువరుస;
#అయితే_ముగింపు
  ఆపు;
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
స్థితి OP_Seek_చివర:             /* ncycle */
స్థితి OP_చివరి: {              /* jump0, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  పూర్ణాంకం res;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  res = 0;
  ప్రతిజ్ఞా( pCrsr!=0 );
#నిర్వచితమైతే SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#అయితే_ముగింపు
  ఒకవేళ( pOp->opcode==OP_Seek_చివర ){
    ప్రతిజ్ఞా( pOp->p2==0 );
    pC->seek_ఫలితం = -1;
    ఒకవేళ( sqlite3BtreeCursorIsValidNN(pCrsr) ){
      ఆపు;
    }
  }
  rc = sqlite3_Btree_చివరి(pCrsr, &res);
  pC->null_అడ్డువరుస = (u8)res;
  pC->deferredMoveto = 0;
  pC->cache_స్థితి = CACHE_STALE;
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఒకవేళ( pOp->p2>0 ){
    Vdbe_Branch_తీసుకున్న(res!=0,2);
    ఒకవేళ( res ) వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
}

/* Opcode: IfSizeBetween P1 P2 P3 P4 *
**
** Let N be the approximate number of rows in the table or index
** with cursor P1 and let X be 10*log2(N) if N is positive or -1
** if N is zero.
**
** Jump to P2 if X is in between P3 and P4, inclusive.
*/
స్థితి OP_If_కొలత_Between: {        /* jump */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  పూర్ణాంకం res;
  i64 sz;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p4type==P4_INT32 );
  ప్రతిజ్ఞా( pOp->p3>=-1 && pOp->p3<=640*2 );
  ప్రతిజ్ఞా( pOp->p4.i>=-1 && pOp->p4.i<=640*2 );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  pCrsr = pC->uc.pCursor;
  ప్రతిజ్ఞా( pCrsr );
  rc = sqlite3_Btree_మొదటి(pCrsr, &res);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఒకవేళ( res!=0 ){
    sz = -1;  /* -Infinity encoding */
  }లేకపోతే{
    sz = sqlite3_Btree_అడ్డువరుస_లెక్క_Est(pCrsr);
    ప్రతిజ్ఞా( sz>0 );
    sz = sqlite3LogEst((u64)sz);
  }
  res = sz>=pOp->p3 && sz<=pOp->p4.i;
  Vdbe_Branch_తీసుకున్న(res!=0,2);
  ఒకవేళ( res ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
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
స్థితి OP_Sorter_క్రమీకరించు:    /* jump ncycle */
స్థితి OP_క్రమీకరించు: {        /* jump ncycle */
#నిర్వచితమైతే SQLITE_TEST
  sqlite3_క్రమీకరించు_లెక్క++;
  sqlite3_శోధన_లెక్క--;
#అయితే_ముగింపు
  p->aCounter[SQLITE_STMTSTATUS_క్రమీకరించు]++;
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
స్థితి OP_Rewind: {        /* jump0, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  పూర్ణాంకం res;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p5==0 );
  ప్రతిజ్ఞా( pOp->p2>=0 && pOp->p2<p->nOp );

  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( isSorter(pC)==(pOp->opcode==OP_Sorter_క్రమీకరించు) );
  res = 1;
#నిర్వచితమైతే SQLITE_DEBUG
  pC->seekOp = OP_Rewind;
#అయితే_ముగింపు
  ఒకవేళ( isSorter(pC) ){
    rc = sqlite3VdbeSorterRewind(pC, &res);
  }లేకపోతే{
    ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
    pCrsr = pC->uc.pCursor;
    ప్రతిజ్ఞా( pCrsr );
    rc = sqlite3_Btree_మొదటి(pCrsr, &res);
    pC->deferredMoveto = 0;
    pC->cache_స్థితి = CACHE_STALE;
  }
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  pC->null_అడ్డువరుస = (u8)res;
  ఒకవేళ( pOp->p2>0 ){
    Vdbe_Branch_తీసుకున్న(res!=0,2);
    ఒకవేళ( res ) వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
}

/* Opcode: IfEmpty P1 P2 * * *
** Synopsis: if( empty(P1) ) goto P2
**
** Check to see if the b-tree table that cursor P1 references is empty
** and jump to P2 if it is.
*/
స్థితి OP_If_ఖాళీలు: {        /* jump */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  పూర్ణాంకం res;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p2>=0 && pOp->p2<p->nOp );

  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  ప్రతిజ్ఞా( pCrsr );
  rc = sqlite3_Btree_Is_ఖాళీలు(pCrsr, &res);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  Vdbe_Branch_తీసుకున్న(res!=0,2);
  ఒకవేళ( res ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
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
స్థితి OP_Sorter_తదుపరి: {  /* jump */
  VdbeCursor *pC;

  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( isSorter(pC) );
  rc = sqlite3_Vdbe_Sorter_తదుపరి(db, pC);
  వెళ్ళు తదుపరి_తోక;

స్థితి OP_Prev:          /* jump, ncycle */
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p5==0
       || pOp->p5==SQLITE_STMTSTATUS_FULLSCAN_అడుగు
       || pOp->p5==SQLITE_STMTSTATUS_AUTOINDEX);
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->seekOp==OP_SeekLT || pC->seekOp==OP_SeekLE
       || pC->seekOp==OP_చివరి   || pC->seekOp==OP_IfNoHope
       || pC->seekOp==OP_Null_అడ్డువరుస);
  rc = sqlite3_Btree_మునుపటి(pC->uc.pCursor, pOp->p3);
  వెళ్ళు తదుపరి_తోక;

స్థితి OP_తదుపరి:          /* jump, ncycle */
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  ప్రతిజ్ఞా( pOp->p5==0
       || pOp->p5==SQLITE_STMTSTATUS_FULLSCAN_అడుగు
       || pOp->p5==SQLITE_STMTSTATUS_AUTOINDEX);
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->seekOp==OP_SeekGT || pC->seekOp==OP_SeekGE
       || pC->seekOp==OP_Rewind || pC->seekOp==OP_దొరికింది
       || pC->seekOp==OP_Null_అడ్డువరుస|| pC->seekOp==OP_SeekRowid
       || pC->seekOp==OP_IfNoHope);
  rc = sqlite3_Btree_తదుపరి(pC->uc.pCursor, pOp->p3);

తదుపరి_తోక:
  pC->cache_స్థితి = CACHE_STALE;
  Vdbe_Branch_తీసుకున్న(rc==SQLITE_OK,2);
  ఒకవేళ( rc==SQLITE_OK ){
    pC->null_అడ్డువరుస = 0;
    p->aCounter[pOp->p5]++;
#నిర్వచితమైతే SQLITE_TEST
    sqlite3_శోధన_లెక్క++;
#అయితే_ముగింపు
    వెళ్ళు jump_గమ్యం_p2_and_check_for_అంతరాయం;
  }
  ఒకవేళ( rc!=SQLITE_పూర్తి ) వెళ్ళు abort_due_గమ్యం_error;
  rc = SQLITE_OK;
  pC->null_అడ్డువరుస = 1;
  వెళ్ళు check_for_అంతరాయం;
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
స్థితి OP_Idx_చొప్పించు: {        /* in2 */
  VdbeCursor *pC;
  BtreePayload x;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  sqlite3VdbeIncrWriteCounter(p, pC);
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( !isSorter(pC) );
  pIn2 = &aMem[pOp->p2];
  ప్రతిజ్ఞా( (pIn2->flags & MEM_Blob) || (pOp->p5 & OPFLAG_PREFORMAT) );
  ఒకవేళ( pOp->p5 & OPFLAG_NCHANGE ) p->nChange++;
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->is_పట్టిక==0 );
  rc = ExpandBlob(pIn2);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  x.n_కీలకం = pIn2->n;
  x.p_కీలకం = pIn2->z;
  x.aMem = aMem + pOp->p3;
  x.nMem = (u16)pOp->p4.i;
  rc = sqlite3_Btree_చొప్పించు(pC->uc.pCursor, &x,
       (pOp->p5 & (OPFLAG_APPEND|OPFLAG_SAVEPOSITION|OPFLAG_PREFORMAT)),
      ((pOp->p5 & OPFLAG_USESEEKRESULT) ? pC->seek_ఫలితం : 0)
      );
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  pC->cache_స్థితి = CACHE_STALE;
  ఒకవేళ( rc) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}

/* Opcode: SorterInsert P1 P2 * * *
** Synopsis: key=r[P2]
**
** Register P2 holds an SQL index key made using the
** MakeRecord instructions.  This opcode writes that key
** into the sorter P1.  Data for the entry is nil.
*/
స్థితి OP_Sorter_చొప్పించు: {     /* in2 */
  VdbeCursor *pC;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  sqlite3VdbeIncrWriteCounter(p, pC);
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( isSorter(pC) );
  pIn2 = &aMem[pOp->p2];
  ప్రతిజ్ఞా( pIn2->flags & MEM_Blob );
  ప్రతిజ్ఞా( pC->is_పట్టిక==0 );
  rc = ExpandBlob(pIn2);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  rc = sqlite3VdbeSorterWrite(pC, pIn2);
  ఒకవేళ( rc) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
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
స్థితి OP_IdxDelete: {
  VdbeCursor *pC;
  BtCursor *pCrsr;
  పూర్ణాంకం res;
  Unpacked_రికార్డు r;

  ప్రతిజ్ఞా( pOp->p5>0 );
  ప్రతిజ్ఞా( pOp->p2>0 && pOp->p2+pOp->p5<=(p->nMem+1 - p->nCursor)+1 );
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  sqlite3VdbeIncrWriteCounter(p, pC);
  pCrsr = pC->uc.pCursor;
  ప్రతిజ్ఞా( pCrsr!=0 );
  r.p_కీలకం_Info = pC->p_కీలకం_Info;
  r.n_క్షేత్రం = pOp->p5;
  r.default_rc = 0;
  r.aMem = &aMem[pOp->p2];
  rc = sqlite3_Btree_సూచీ_Moveto(pCrsr, &r, &res);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఒకవేళ( res!=0 ){
    rc = sqlite3_Vdbe_కనుగొను_సూచీ_కీలకం(pCrsr, pOp->p4.pIdx, &r, &res, 0);
    ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abort_due_గమ్యం_error;
    ఒకవేళ( res!=0 ){
      ఒకవేళ( !sqlite3WritableSchema(db) ){
        rc = sqlite3ReportError(
            SQLITE_CORRUPT_సూచీ, __LINE__, "index corruption");
        వెళ్ళు abort_due_గమ్యం_error;
      }
      pC->cache_స్థితి = CACHE_STALE;
      pC->seek_ఫలితం = 0;
      ఆపు;
    }
  }

  ఒకవేళ( pOp->p3 && vdbe_సూచీ_కీలకం_Compare(pCrsr, &aMem[pOp->p3], &rc) ){
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
    sqlite3_Vdbe_Mem_సమితి_Null(&aMem[pOp->p3]);
    ఆపు;
  }

  rc = sqlite3BtreeDelete(pCrsr, BTREE_AUXDELETE);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  pC->cache_స్థితి = CACHE_STALE;
  pC->seek_ఫలితం = 0;
  ఆపు;
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
స్థితి OP_DeferredSeek:         /* ncycle */
స్థితి OP_IdxRowid: {           /* out2, ncycle */
  VdbeCursor *pC;             /* The P1 index cursor */
  VdbeCursor *pTabCur;        /* The P2 table cursor (OP_DeferredSeek only) */
  i64 rowid;                  /* Rowid that P1 current points to */

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE || IsNullCursor(pC) );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
  ప్రతిజ్ఞా( pC->is_పట్టిక==0 || IsNullCursor(pC) );
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  ప్రతిజ్ఞా( !pC->null_అడ్డువరుస || pOp->opcode==OP_IdxRowid );

  /* The IdxRowid and Seek opcodes are combined because of the commonality
  ** of sqlite3VdbeCursorRestore() and sqlite3VdbeIdxRowid(). */
  rc = sqlite3VdbeCursorRestore(pC);

  /* sqlite3VdbeCursorRestore() may fail if the cursor has been disturbed
  ** since it was last positioned and an error (e.g. OOM or an IO error)
  ** occurs while trying to reposition it. */
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abort_due_గమ్యం_error;

  ఒకవేళ( !pC->null_అడ్డువరుస ){
    rowid = 0;  /* Not needed.  Only used to silence a warning. */
    rc = sqlite3VdbeIdxRowid(db, pC->uc.pCursor, &rowid);
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు abort_due_గమ్యం_error;
    }
    ఒకవేళ( pOp->opcode==OP_DeferredSeek ){
      ప్రతిజ్ఞా( pOp->p3>=0 && pOp->p3<p->nCursor );
      pTabCur = p->apCsr[pOp->p3];
      ప్రతిజ్ఞా( pTabCur!=0 );
      ప్రతిజ్ఞా( pTabCur->eCurType==CURTYPE_BTREE );
      ప్రతిజ్ఞా( pTabCur->uc.pCursor!=0 );
      ప్రతిజ్ఞా( pTabCur->is_పట్టిక );
#ఒకవేళ defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
      ప్రతిజ్ఞా( 
          sqlite3BtreeCursorHintTblCsr(pC->uc.pCursor)==pTabCur->uc.pCursor 
      );
#అయితే_ముగింపు
      pTabCur->null_అడ్డువరుస = 0;
      pTabCur->moveto_లక్ష్యం = rowid;
      pTabCur->deferredMoveto = 1;
      pTabCur->cache_స్థితి = CACHE_STALE;
      ప్రతిజ్ఞా( pOp->p4type==P4_INTARRAY || pOp->p4.ai==0 );
      ప్రతిజ్ఞా( !pTabCur->isEphemeral );
      pTabCur->ub.aAltMap = pOp->p4.ai;
      ప్రతిజ్ఞా( !pC->isEphemeral );
      pTabCur->pAltCursor = pC;
    }లేకపోతే{
      p_బహిర్గతం = out2Prerelease(p, pOp);
      p_బహిర్గతం->u.i = rowid;
    }
  }లేకపోతే{
    ప్రతిజ్ఞా( pOp->opcode==OP_IdxRowid );
    sqlite3_Vdbe_Mem_సమితి_Null(&aMem[pOp->p2]);
  }
  ఆపు;
}

/* Opcode: FinishSeek P1 * * * *
**
** If cursor P1 was previously moved via OP_DeferredSeek, complete that
** seek operation now, without further delay.  If the cursor seek has
** already occurred, this instruction is a no-op.
*/
స్థితి OP_ముగింపు_Seek: {        /* ncycle */
  VdbeCursor *pC;            /* The P1 index cursor */

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ఒకవేళ( pC->deferredMoveto ){
    rc = sqlite3_Vdbe_ముగింపు_Moveto(pC);
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  }
  ఆపు;
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
స్థితి OP_IdxLE:          /* jump, ncycle */
స్థితి OP_IdxGT:          /* jump, ncycle */
స్థితి OP_IdxLT:          /* jump, ncycle */
స్థితి OP_IdxGE:  {       /* jump, ncycle */
  VdbeCursor *pC;
  పూర్ణాంకం res;
  Unpacked_రికార్డు r;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->isOrdered );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0);
  ప్రతిజ్ఞా( pC->deferredMoveto==0 );
  ప్రతిజ్ఞా( pOp->p4type==P4_INT32 );
  r.p_కీలకం_Info = pC->p_కీలకం_Info;
  r.n_క్షేత్రం = (u16)pOp->p4.i;
  ఒకవేళ( pOp->opcode<OP_IdxLT ){
    ప్రతిజ్ఞా( pOp->opcode==OP_IdxLE || pOp->opcode==OP_IdxGT );
    r.default_rc = -1;
  }లేకపోతే{
    ప్రతిజ్ఞా( pOp->opcode==OP_IdxGE || pOp->opcode==OP_IdxLT );
    r.default_rc = 0;
  }
  r.aMem = &aMem[pOp->p3];
#నిర్వచితమైతే SQLITE_DEBUG
  {
    పూర్ణాంకం i;
    కొరకు(i=0; i<r.n_క్షేత్రం; i++){
      ప్రతిజ్ఞా( memIsValid(&r.aMem[i]) );
      రిజిస్టరు_TRACE(pOp->p3+i, &aMem[pOp->p3+i]);
    }
  }
#అయితే_ముగింపు

  /* Inlined version of sqlite3VdbeIdxKeyCompare() */
  {
    i64 n_Cell_కీలకం = 0;
    BtCursor *pCur;
    Mem m;

    ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
    pCur = pC->uc.pCursor;
    ప్రతిజ్ఞా( sqlite3BtreeCursorIsValid(pCur) );
    n_Cell_కీలకం = sqlite3_Btree_Payload_కొలత(pCur);
    /* nCellKey will always be between 0 and 0xffffffff because of the way
    ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
    ఒకవేళ( n_Cell_కీలకం<=0 || n_Cell_కీలకం>0x7fffffff ){
      rc = SQLITE_CORRUPT_BKPT;
      వెళ్ళు abort_due_గమ్యం_error;
    }
    sqlite3VdbeMemInit(&m, db, 0);
    rc = sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(pCur, (u32)n_Cell_కీలకం, &m);
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
    res = sqlite3_Vdbe_రికార్డు_Compare_With_Skip(m.n, m.z, &r, 0);
    sqlite3VdbeMemReleaseMalloc(&m);
  }
  /* End of inlined sqlite3VdbeIdxKeyCompare() */

  ప్రతిజ్ఞా( (OP_IdxLE&1)==(OP_IdxLT&1) && (OP_IdxGE&1)==(OP_IdxGT&1) );
  ఒకవేళ( (pOp->opcode&1)==(OP_IdxLT&1) ){
    ప్రతిజ్ఞా( pOp->opcode==OP_IdxLE || pOp->opcode==OP_IdxLT );
    res = -res;
  }లేకపోతే{
    ప్రతిజ్ఞా( pOp->opcode==OP_IdxGE || pOp->opcode==OP_IdxGT );
    res++;
  }
  Vdbe_Branch_తీసుకున్న(res>0,2);
  ప్రతిజ్ఞా( rc==SQLITE_OK );
  ఒకవేళ( res>0 ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
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
స్థితి OP_Destroy: {     /* out2 */
  పూర్ణాంకం iMoved;
  పూర్ణాంకం iDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  ప్రతిజ్ఞా( p->readOnly==0 );
  ప్రతిజ్ఞా( pOp->p1>1 );
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->flags = MEM_Null;
  ఒకవేళ( db->nVdbeRead > db->nVDestroy+1 ){
    rc = SQLITE_LOCKED;
    p->errorAction = OE_Abort;
    వెళ్ళు abort_due_గమ్యం_error;
  }లేకపోతే{
    iDb = pOp->p3;
    ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, iDb) );
    iMoved = 0;  /* Not needed.  Only to silence a warning. */
    rc = sqlite3_Btree_Drop_పట్టిక(db->aDb[iDb].pBt, pOp->p1, &iMoved);
    p_బహిర్గతం->flags = MEM_Int;
    p_బహిర్గతం->u.i = iMoved;
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( iMoved!=0 ){
      sqlite3_మూలం_పుట_Moved(db, iDb, iMoved, pOp->p1);
      /* All OP_Destroy operations occur on the same btree */
      ప్రతిజ్ఞా( resetSchemaOnFault==0 || resetSchemaOnFault==iDb+1 );
      resetSchemaOnFault = iDb+1;
    }
#అయితే_ముగింపు
  }
  ఆపు;
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
స్థితి OP_Clear: {
  i64 nChange;

  sqlite3VdbeIncrWriteCounter(p, 0);
  nChange = 0;
  ప్రతిజ్ఞా( p->readOnly==0 );
  ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, pOp->p2) );
  rc = sqlite3_Btree_Clear_పట్టిక(db->aDb[pOp->p2].pBt, (u32)pOp->p1, &nChange);
  ఒకవేళ( pOp->p3 ){
    p->nChange += nChange;
    ఒకవేళ( pOp->p3>0 ){
      ప్రతిజ్ఞా( memIsValid(&aMem[pOp->p3]) );
      mem_About_గమ్యం_Change(p, &aMem[pOp->p3]);
      aMem[pOp->p3].u.i += nChange;
    }
  }
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}

/* Opcode: ResetSorter P1 * * * *
**
** Delete all contents from the ephemeral table or sorter
** that is open on cursor P1.
**
** This opcode only works for cursors used for sorting and
** opened with OP_OpenEphemeral or OP_SorterOpen.
*/
స్థితి OP_ResetSorter: {
  VdbeCursor *pC;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ఒకవేళ( isSorter(pC) ){
    sqlite3VdbeSorterReset(db, pC->uc.pSorter);
  }లేకపోతే{
    ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
    ప్రతిజ్ఞా( pC->isEphemeral );
    rc = sqlite3_Btree_Clear_పట్టిక_యొక్క_Cursor(pC->uc.pCursor);
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  }
  ఆపు;
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
స్థితి OP_CreateBtree: {          /* out2 */
  Pgno pgno;
  Db *pDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  p_బహిర్గతం = out2Prerelease(p, pOp);
  pgno = 0;
  ప్రతిజ్ఞా( pOp->p3==BTREE_INTKEY || pOp->p3==BTREE_BLOBKEY );
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<db->nDb );
  ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, pOp->p1) );
  ప్రతిజ్ఞా( p->readOnly==0 );
  pDb = &db->aDb[pOp->p1];
  ప్రతిజ్ఞా( pDb->pBt!=0 );
  rc = sqlite3_Btree_Create_పట్టిక(pDb->pBt, &pgno, pOp->p3);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  p_బహిర్గతం->u.i = pgno;
  ఆపు;
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
స్థితి OP_SqlExec: {
  అక్షరం *zErr;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;
#అయితే_ముగింపు
  u8 mTrace;
  పూర్ణాంకం saved_Analysis_హద్దు;

  sqlite3VdbeIncrWriteCounter(p, 0);
  db->nSqlExec++;
  zErr = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  xAuth = db->xAuth;
#అయితే_ముగింపు
  mTrace = db->mTrace;
  saved_Analysis_హద్దు = db->n_Analysis_హద్దు;
  ఒకవేళ( pOp->p1 & 0x0001 ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
    db->xAuth = 0;
#అయితే_ముగింపు
    db->mTrace = 0;
  }
  ఒకవేళ( pOp->p1 & 0x0002 ){
    db->n_Analysis_హద్దు = pOp->p2;
  }
  rc = sqlite3_exec(db, pOp->p4.z, 0, 0, &zErr);
  db->nSqlExec--;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#అయితే_ముగింపు
  db->mTrace = mTrace;
  db->n_Analysis_హద్దు = saved_Analysis_హద్దు;
  ఒకవేళ( zErr || rc ){
    sqlite3VdbeError(p, "%s", zErr);
    sqlite3_free(zErr);
    ఒకవేళ( rc==SQLITE_NOMEM ) వెళ్ళు no_mem;
    వెళ్ళు abort_due_గమ్యం_error;
  }
  ఆపు;
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
స్థితి OP_ParseSchema: {
  పూర్ణాంకం iDb;
  స్థిరరాశి అక్షరం *zSchema;
  అక్షరం *zSql;
  Init_దత్తాంశం init_దత్తాంశం;

  /* Any prepared statement that invokes this opcode will hold mutexes
  ** on every btree.  This is a prerequisite for invoking
  ** sqlite3InitCallback().
  */
#నిర్వచితమైతే SQLITE_DEBUG
  కొరకు(iDb=0; iDb<db->nDb; iDb++){
    ప్రతిజ్ఞా( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );
  }
#అయితే_ముగింపు

  iDb = pOp->p1;
  ప్రతిజ్ఞా( iDb>=0 && iDb<db->nDb );
  ప్రతిజ్ఞా( DbHasProperty(db, iDb, DB_SchemaLoaded)
           || db->mallocFailed
           || (CORRUPT_DB && (db->flags & SQLITE_NoSchemaError)!=0) );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ALTERTABLE
  ఒకవేళ( pOp->p4.z==0 ){
    ప్రతిజ్ఞా( pOp->p5!=0 );
    sqlite3SchemaClear(db->aDb[iDb].pSchema);
    db->mDbFlags &= ~DBFLAG_Schema_తెలిసిన_Ok;
    rc = sqlite3InitOne(db, iDb, &p->z_Err_సందేశం, pOp->p5);
    db->mDbFlags |= DBFLAG_SchemaChange;
    p->expired = 0;
  }లేకపోతే
#అయితే_ముగింపు
  {
    ప్రతిజ్ఞా( pOp->p5==0 );
    zSchema = LEGACY_SCHEMA_పట్టిక;
    init_దత్తాంశం.db = db;
    init_దత్తాంశం.iDb = iDb;
    init_దత్తాంశం.pz_Err_సందేశం = &p->z_Err_సందేశం;
    init_దత్తాంశం.mInitFlags = 0;
    init_దత్తాంశం.mx_పుట = sqlite3_Btree_చివరి_పుట(db->aDb[iDb].pBt);
    zSql = sqlite3MPrintf(db,
       "SELECT*FROM\"%w\".%s WHERE %s ORDER BY rowid",
       db->aDb[iDb].zDbSName, zSchema, pOp->p4.z);
    ఒకవేళ( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }లేకపోతే{
      ప్రతిజ్ఞా( db->init.busy==0 );
      db->init.busy = 1;
      init_దత్తాంశం.rc = SQLITE_OK;
      init_దత్తాంశం.n_Init_అడ్డువరుస = 0;
      ప్రతిజ్ఞా( !db->mallocFailed );
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &init_దత్తాంశం, 0);
      ఒకవేళ( rc==SQLITE_OK ) rc = init_దత్తాంశం.rc;
      ఒకవేళ( rc==SQLITE_OK && init_దత్తాంశం.n_Init_అడ్డువరుస==0 ){
        /* The OP_ParseSchema opcode with a non-NULL P4 argument should parse
        ** at least one SQL statement. Any less than that indicates that
        ** the sqlite_schema table is corrupt. */
        rc = SQLITE_CORRUPT_BKPT;
      }
      sqlite3DbFreeNN(db, zSql);
      db->init.busy = 0;
    }
  }
  ఒకవేళ( rc ){
    sqlite3_Reset_All_Schemas_యొక్క_Connection(db);
    ఒకవేళ( rc==SQLITE_NOMEM ){
      వెళ్ళు no_mem;
    }
    వెళ్ళు abort_due_గమ్యం_error;
  }
  ఆపు; 
}

#ఒకవేళ !defined(SQLITE_OMIT_ANALYZE)
/* Opcode: LoadAnalysis P1 * * * *
**
** Read the sqlite_stat1 table for database P1 and load the content
** of that table into the internal index hash table.  This will cause
** the analysis to be used when preparing all subsequent queries.
*/
స్థితి OP_ఎక్కించు_Analysis: {
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<db->nDb );
  rc = sqlite3_Analysis_ఎక్కించు(db, pOp->p1);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు; 
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_ANALYZE) */

/* Opcode: DropTable P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the table named P4 in database P1.  This is called after a table
** is dropped from disk (using the Destroy opcode) in order to keep
** the internal representation of the
** schema consistent with what is on disk.
*/
స్థితి OP_Drop_పట్టిక: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3_Unlink_And_Delete_పట్టిక(db, pOp->p1, pOp->p4.z);
  ఆపు;
}

/* Opcode: DropIndex P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the index named P4 in database P1.  This is called after an index
** is dropped from disk (using the Destroy opcode)
** in order to keep the internal representation of the
** schema consistent with what is on disk.
*/
స్థితి OP_Drop_సూచీ: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3_Unlink_And_Delete_సూచీ(db, pOp->p1, pOp->p4.z);
  ఆపు;
}

/* Opcode: DropTrigger P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the trigger named P4 in database P1.  This is called after a trigger
** is dropped from disk (using the Destroy opcode) in order to keep
** the internal representation of the
** schema consistent with what is on disk.
*/
స్థితి OP_DropTrigger: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3UnlinkAndDeleteTrigger(db, pOp->p1, pOp->p4.z);
  ఆపు;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_INTEGRITY_CHECK
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
స్థితి OP_IntegrityCk: {
  పూర్ణాంకం n_మూలం;      /* Number of tables to check.  (Number of root pages.) */
  Pgno *a_మూలం;    /* Array of rootpage numbers for tables to be checked */
  పూర్ణాంకం nErr;       /* Number of errors reported */
  అక్షరం *z;        /* Text of the error report */
  Mem *pnErr;     /* Register keeping track of errors remaining */

  ప్రతిజ్ఞా( p->bIsReader );
  ప్రతిజ్ఞా( pOp->p4type==P4_INTARRAY );
  n_మూలం = pOp->p2;
  a_మూలం = pOp->p4.ai;
  ప్రతిజ్ఞా( n_మూలం>0 );
  ప్రతిజ్ఞా( a_మూలం!=0 );
  ప్రతిజ్ఞా( a_మూలం[0]==(Pgno)n_మూలం );
  ప్రతిజ్ఞా( pOp->p1>0 && (pOp->p1+1)<=(p->nMem+1 - p->nCursor) );
  pnErr = &aMem[pOp->p1];
  ప్రతిజ్ఞా( (pnErr->flags & MEM_Int)!=0 );
  ప్రతిజ్ఞా( (pnErr->flags & (MEM_Str|MEM_Blob))==0 );
  pIn1 = &aMem[pOp->p1+1];
  ప్రతిజ్ఞా( pOp->p5<db->nDb );
  ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, pOp->p5) );
  rc = sqlite3BtreeIntegrityCheck(db, db->aDb[pOp->p5].pBt, &a_మూలం[1], 
      &aMem[pOp->p3], n_మూలం, (పూర్ణాంకం)pnErr->u.i+1, &nErr, &z);
  sqlite3_Vdbe_Mem_సమితి_Null(pIn1);
  ఒకవేళ( nErr==0 ){
    ప్రతిజ్ఞా( z==0 );
  }లేకపోతే ఒకవేళ( rc ){
    sqlite3_free(z);
    వెళ్ళు abort_due_గమ్యం_error;
  }లేకపోతే{
    pnErr->u.i -= nErr-1;
    sqlite3_Vdbe_Mem_సమితి_Str(pIn1, z, -1, SQLITE_UTF8, sqlite3_free);
  }
  UPDATE_MAX_BLOBSIZE(pIn1);
  sqlite3VdbeChangeEncoding(pIn1, encoding);
  వెళ్ళు check_for_అంతరాయం;
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
స్థితి OP_IFind_కీలకం: {     /* jump, in3 */
  VdbeCursor *pC;
  పూర్ణాంకం res;
  Unpacked_రికార్డు r;

  ప్రతిజ్ఞా( pOp[-1].opcode==OP_దొరికింది );
  ప్రతిజ్ఞా( pOp[-1].p1==pOp->p1 );
  ప్రతిజ్ఞా( pOp[-1].p3==pOp->p3 );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pOp->p4type==P4_సూచీ );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pC->uc.pCursor!=0 );
  ప్రతిజ్ఞా( pC->is_పట్టిక==0 );

  memset(&r, 0, పరిమాణం(r));
  r.aMem = &aMem[pOp->p3];
  r.n_క్షేత్రం = pOp->p4.pIdx->n_స్తంభం;
  r.p_కీలకం_Info = pC->p_కీలకం_Info;

  rc = sqlite3_Vdbe_కనుగొను_సూచీ_కీలకం(pC->uc.pCursor, pOp->p4.pIdx, &r, &res, 1);
  ఒకవేళ( rc || res!=0 ){
    rc = SQLITE_OK;
    వెళ్ళు jump_గమ్యం_p2;
  }
  pC->null_అడ్డువరుస = 0;
  ఆపు;
};
#అయితే_ముగింపు /* SQLITE_OMIT_INTEGRITY_CHECK */

/* Opcode: RowSetAdd P1 P2 * * *
** Synopsis: rowset(P1)=r[P2]
**
** Insert the integer value held by register P2 into a RowSet object
** held in register P1.
**
** An assertion fails if P2 is not an integer.
*/
స్థితి OP_అడ్డువరుస_సమితి_కూడు: {       /* in1, in2 */
  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  ప్రతిజ్ఞా( (pIn2->flags & MEM_Int)!=0 );
  ఒకవేళ( (pIn1->flags & MEM_Blob)==0 ){
    ఒకవేళ( sqlite3_Vdbe_Mem_సమితి_అడ్డువరుస_సమితి(pIn1) ) వెళ్ళు no_mem;
  }
  ప్రతిజ్ఞా( sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pIn1) );
  sqlite3_అడ్డువరుస_సమితి_చొప్పించు((అడ్డువరుస_సమితి*)pIn1->z, pIn2->u.i);
  ఆపు;
}

/* Opcode: RowSetRead P1 P2 P3 * *
** Synopsis: r[P3]=rowset(P1)
**
** Extract the smallest value from the RowSet object in P1
** and put that value into register P3.
** Or, if RowSet object P1 is initially empty, leave P3
** unchanged and jump to instruction P2.
*/
స్థితి OP_అడ్డువరుస_సమితి_Read: {       /* jump, in1, out3 */
  i64 val;

  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( (pIn1->flags & MEM_Blob)==0 || sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pIn1) );
  ఒకవేళ( (pIn1->flags & MEM_Blob)==0
   || sqlite3_అడ్డువరుస_సమితి_తదుపరి((అడ్డువరుస_సమితి*)pIn1->z, &val)==0
  ){
    /* The boolean index is empty */
    sqlite3_Vdbe_Mem_సమితి_Null(pIn1);
    Vdbe_Branch_తీసుకున్న(1,2);
    వెళ్ళు jump_గమ్యం_p2_and_check_for_అంతరాయం;
  }లేకపోతే{
    /* A value was pulled from the index */
    Vdbe_Branch_తీసుకున్న(0,2);
    sqlite3_Vdbe_Mem_సమితి_Int64(&aMem[pOp->p3], val);
  }
  వెళ్ళు check_for_అంతరాయం;
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
స్థితి OP_అడ్డువరుస_సమితి_Test: {                     /* jump, in1, in3 */
  పూర్ణాంకం i_సమితి;
  పూర్ణాంకం exists;

  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  i_సమితి = pOp->p4.i;
  ప్రతిజ్ఞా( pIn3->flags&MEM_Int );

  /* If there is anything other than a rowset object in memory cell P1,
  ** delete it now and initialize P1 with an empty rowset
  */
  ఒకవేళ( (pIn1->flags & MEM_Blob)==0 ){
    ఒకవేళ( sqlite3_Vdbe_Mem_సమితి_అడ్డువరుస_సమితి(pIn1) ) వెళ్ళు no_mem;
  }
  ప్రతిజ్ఞా( sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pIn1) );
  ప్రతిజ్ఞా( pOp->p4type==P4_INT32 );
  ప్రతిజ్ఞా( i_సమితి==-1 || i_సమితి>=0 );
  ఒకవేళ( i_సమితి ){
    exists = sqlite3_అడ్డువరుస_సమితి_Test((అడ్డువరుస_సమితి*)pIn1->z, i_సమితి, pIn3->u.i);
    Vdbe_Branch_తీసుకున్న(exists!=0,2);
    ఒకవేళ( exists ) వెళ్ళు jump_గమ్యం_p2;
  }
  ఒకవేళ( i_సమితి>=0 ){
    sqlite3_అడ్డువరుస_సమితి_చొప్పించు((అడ్డువరుస_సమితి*)pIn1->z, pIn3->u.i);
  }
  ఆపు;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER

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
స్థితి OP_కార్యక్రమం: {        /* jump0 */
  పూర్ణాంకం nMem;               /* Number of memory registers for sub-program */
  i64 nByte;              /* Bytes of runtime space required for sub-program */
  Mem *pRt;               /* Register to allocate runtime space */
  Mem *pMem;              /* Used to iterate through memory cells */
  Mem *p_చివర;              /* Last memory cell in new array */
  Vdbe_చట్రం *p_చట్రం;      /* New vdbe frame to execute in */
  Sub_కార్యక్రమం *p_కార్యక్రమం;   /* Sub-program to execute */
  శూన్య *t;                /* Token identifying trigger */

  p_కార్యక్రమం = pOp->p4.p_కార్యక్రమం;
  pRt = &aMem[pOp->p3];
  ప్రతిజ్ఞా( p_కార్యక్రమం->nOp>0 );
 
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
  ఒకవేళ( pOp->p5 ){
    t = p_కార్యక్రమం->token;
    కొరకు(p_చట్రం=p->p_చట్రం; p_చట్రం && p_చట్రం->token!=t; p_చట్రం=p_చట్రం->p_మాతృ);
    ఒకవేళ( p_చట్రం ) ఆపు;
  }

  ఒకవేళ( p->n_చట్రం>=db->a_హద్దు[SQLITE_హద్దు_TRIGGER_లోతు] ){
    rc = SQLITE_ERROR;
    sqlite3VdbeError(p, "triggers nested too deep");
    వెళ్ళు abort_due_గమ్యం_error;
  }

  /* Register pRt is used to store the memory required to save the state
  ** of the current program, and the memory required at runtime to execute
  ** the trigger program. If this trigger has been fired before, then pRt
  ** is already allocated. Otherwise, it must be initialized.  */
  ఒకవేళ( (pRt->flags&MEM_Blob)==0 ){
    /* SubProgram.nMem is set to the number of memory cells used by the
    ** program stored in SubProgram.aOp. As well as these, one memory
    ** cell is required for each cursor used by the program. Set local
    ** variable nMem (and later, VdbeFrame.nChildMem) to this value.
    */
    nMem = p_కార్యక్రమం->nMem + p_కార్యక్రమం->nCsr;
    ప్రతిజ్ఞా( nMem>0 );
    ఒకవేళ( p_కార్యక్రమం->nCsr==0 ) nMem++;
    nByte = ROUND8(పరిమాణం(Vdbe_చట్రం))
              + nMem * పరిమాణం(Mem)
              + p_కార్యక్రమం->nCsr * పరిమాణం(VdbeCursor*)
              + (7 + (i64)p_కార్యక్రమం->nOp)/8;
    p_చట్రం = sqlite3DbMallocZero(db, nByte);
    ఒకవేళ( !p_చట్రం ){
      వెళ్ళు no_mem;
    }
    sqlite3VdbeMemRelease(pRt);
    pRt->flags = MEM_Blob|MEM_Dyn;
    pRt->z = (అక్షరం*)p_చట్రం;
    pRt->n = (పూర్ణాంకం)nByte;
    pRt->xDel = sqlite3_Vdbe_చట్రం_Mem_Del;

    p_చట్రం->v = p;
    p_చట్రం->n_బిడ్డ_Mem = nMem;
    p_చట్రం->n_బిడ్డ_Csr = p_కార్యక్రమం->nCsr;
    p_చట్రం->pc = (పూర్ణాంకం)(pOp - aOp);
    p_చట్రం->aMem = p->aMem;
    p_చట్రం->nMem = p->nMem;
    p_చట్రం->apCsr = p->apCsr;
    p_చట్రం->nCursor = p->nCursor;
    p_చట్రం->aOp = p->aOp;
    p_చట్రం->nOp = p->nOp;
    p_చట్రం->token = p_కార్యక్రమం->token;
#నిర్వచితమైతే SQLITE_DEBUG
    p_చట్రం->i_చట్రం_Magic = SQLITE_చట్రం_MAGIC;
#అయితే_ముగింపు

    p_చివర = &Vdbe_చట్రం_Mem(p_చట్రం)[p_చట్రం->n_బిడ్డ_Mem];
    కొరకు(pMem=Vdbe_చట్రం_Mem(p_చట్రం); pMem!=p_చివర; pMem++){
      pMem->flags = MEM_Undefined;
      pMem->db = db;
    }
  }లేకపోతే{
    p_చట్రం = (Vdbe_చట్రం*)pRt->z;
    ప్రతిజ్ఞా( pRt->xDel==sqlite3_Vdbe_చట్రం_Mem_Del );
    ప్రతిజ్ఞా( p_కార్యక్రమం->nMem+p_కార్యక్రమం->nCsr==p_చట్రం->n_బిడ్డ_Mem
        || (p_కార్యక్రమం->nCsr==0 && p_కార్యక్రమం->nMem+1==p_చట్రం->n_బిడ్డ_Mem) );
    ప్రతిజ్ఞా( p_కార్యక్రమం->nCsr==p_చట్రం->n_బిడ్డ_Csr );
    ప్రతిజ్ఞా( (పూర్ణాంకం)(pOp - aOp)==p_చట్రం->pc );
  }

  p->n_చట్రం++;
  p_చట్రం->p_మాతృ = p->p_చట్రం;
  p_చట్రం->చివరి_Rowid = db->చివరి_Rowid;
  p_చట్రం->nChange = p->nChange;
  p_చట్రం->nDbChange = p->db->nChange;
  ప్రతిజ్ఞా( p_చట్రం->p_Aux_దత్తాంశం==0 );
  p_చట్రం->p_Aux_దత్తాంశం = p->p_Aux_దత్తాంశం;
  p->p_Aux_దత్తాంశం = 0;
  p->nChange = 0;
  p->p_చట్రం = p_చట్రం;
  p->aMem = aMem = Vdbe_చట్రం_Mem(p_చట్రం);
  p->nMem = p_చట్రం->n_బిడ్డ_Mem;
  p->nCursor = (u16)p_చట్రం->n_బిడ్డ_Csr;
  p->apCsr = (VdbeCursor **)&aMem[p->nMem];
  p_చట్రం->aOnce = (u8*)&p->apCsr[p_కార్యక్రమం->nCsr];
  memset(p_చట్రం->aOnce, 0, (p_కార్యక్రమం->nOp + 7)/8);
  p->aOp = aOp = p_కార్యక్రమం->aOp;
  p->nOp = p_కార్యక్రమం->nOp;
#నిర్వచితమైతే SQLITE_DEBUG
  /* Verify that second and subsequent executions of the same trigger do not
  ** try to reuse register values from the first use. */
  {
    పూర్ణాంకం i;
    కొరకు(i=0; i<p->nMem; i++){
      aMem[i].pScopyFrom = 0;  /* Prevent false-positive AboutToChange() errs */
      Mem_సమితి_Type_Flag(&aMem[i], MEM_Undefined); /* Fault if this reg is reused */
    }
  }
#అయితే_ముగింపు
  pOp = &aOp[-1];
  వెళ్ళు check_for_అంతరాయం;
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
స్థితి OP_Param: {           /* out2 */
  Vdbe_చట్రం *p_చట్రం;
  Mem *p_లోపల;
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_చట్రం = p->p_చట్రం;
  p_లోపల = &p_చట్రం->aMem[pOp->p1 + p_చట్రం->aOp[p_చట్రం->pc].p1];  
  sqlite3VdbeMemShallowCopy(p_బహిర్గతం, p_లోపల, MEM_Ephem);
  ఆపు;
}

#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_TRIGGER */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
/* Opcode: FkCounter P1 P2 * * *
** Synopsis: fkctr[P1]+=P2
**
** Increment a "constraint counter" by P2 (P2 may be negative or positive).
** If P1 is non-zero, the database constraint counter is incremented
** (deferred foreign key constraints). Otherwise, if P1 is zero, the
** statement counter is incremented (immediate foreign key constraints).
*/
స్థితి OP_FkCounter: {
  ఒకవేళ( pOp->p1 ){
    db->nDeferredCons += pOp->p2;
  }లేకపోతే{
    ఒకవేళ( db->flags & SQLITE_DeferFKs ){
      db->nDeferredImmCons += pOp->p2;
    }లేకపోతే{
      p->nFkConstraint += pOp->p2;
    }
  }
  ఆపు;
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
స్థితి OP_FkIfZero: {         /* jump */
  ఒకవేళ( pOp->p1 ){
    Vdbe_Branch_తీసుకున్న(db->nDeferredCons==0 && db->nDeferredImmCons==0, 2);
    ఒకవేళ( db->nDeferredCons==0 && db->nDeferredImmCons==0 ) వెళ్ళు jump_గమ్యం_p2;
  }లేకపోతే{
    Vdbe_Branch_తీసుకున్న(p->nFkConstraint==0 && db->nDeferredImmCons==0, 2);
    ఒకవేళ( p->nFkConstraint==0 && db->nDeferredImmCons==0 ) వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
}
#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_FOREIGN_KEY */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINCREMENT
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
స్థితి OP_MemMax: {        /* in2 */
  Vdbe_చట్రం *p_చట్రం;
  ఒకవేళ( p->p_చట్రం ){
    కొరకు(p_చట్రం=p->p_చట్రం; p_చట్రం->p_మాతృ; p_చట్రం=p_చట్రం->p_మాతృ);
    pIn1 = &p_చట్రం->aMem[pOp->p1];
  }లేకపోతే{
    pIn1 = &aMem[pOp->p1];
  }
  ప్రతిజ్ఞా( memIsValid(pIn1) );
  sqlite3VdbeMemIntegerify(pIn1);
  pIn2 = &aMem[pOp->p2];
  sqlite3VdbeMemIntegerify(pIn2);
  ఒకవేళ( pIn1->u.i<pIn2->u.i){
    pIn1->u.i = pIn2->u.i;
  }
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_AUTOINCREMENT */

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
స్థితి OP_IfPos: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( pIn1->flags&MEM_Int );
  Vdbe_Branch_తీసుకున్న( pIn1->u.i>0, 2);
  ఒకవేళ( pIn1->u.i>0 ){
    pIn1->u.i -= pOp->p3;
    వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
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
స్థితి OP_స్థానభ్రంశం_హద్దు: {    /* in1, out2, in3 */
  i64 x;
  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  p_బహిర్గతం = out2Prerelease(p, pOp);
  ప్రతిజ్ఞా( pIn1->flags & MEM_Int );
  ప్రతిజ్ఞా( pIn3->flags & MEM_Int );
  x = pIn1->u.i;
  ఒకవేళ( x<=0 || sqlite3_కూడు_Int64(&x, pIn3->u.i>0?pIn3->u.i:0) ){
    /* If the LIMIT is less than or equal to zero, loop forever.  This
    ** is documented.  But also, if the LIMIT+OFFSET exceeds 2^63 then
    ** also loop forever.  This is undocumented.  In fact, one could argue
    ** that the loop should terminate.  But assuming 1 billion iterations
    ** per second (far exceeding the capabilities of any current hardware)
    ** it would take nearly 300 years to actually reach the limit.  So
    ** looping forever is a reasonable approximation. */
    p_బహిర్గతం->u.i = -1;
  }లేకపోతే{
    p_బహిర్గతం->u.i = x;
  }
  ఆపు;
}

/* Opcode: IfNotZero P1 P2 * * *
** Synopsis: if r[P1]!=0 then r[P1]--, goto P2
**
** Register P1 must contain an integer.  If the content of register P1 is
** initially greater than zero, then decrement the value in register P1.
** If it is non-zero (negative or positive) and then also jump to P2. 
** If register P1 is initially zero, leave it unchanged and fall through.
*/
స్థితి OP_IfNotZero: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( pIn1->flags&MEM_Int );
  Vdbe_Branch_తీసుకున్న(pIn1->u.i<0, 2);
  ఒకవేళ( pIn1->u.i ){
     ఒకవేళ( pIn1->u.i>0 ) pIn1->u.i--;
     వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
}

/* Opcode: DecrJumpZero P1 P2 * * *
** Synopsis: if (--r[P1])==0 goto P2
**
** Register P1 must hold an integer.  Decrement the value in P1
** and jump to P2 if the new value is exactly zero.
*/
స్థితి OP_DecrJumpZero: {      /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( pIn1->flags&MEM_Int );
  ఒకవేళ( pIn1->u.i>అతిచిన్న_INT64 ) pIn1->u.i--;
  Vdbe_Branch_తీసుకున్న(pIn1->u.i==0, 2);
  ఒకవేళ( pIn1->u.i==0 ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
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
స్థితి OP_AggInverse:
స్థితి OP_Agg_అడుగు: {
  పూర్ణాంకం n;
  sqlite3_context *pCtx;
  u64 nAlloc;

  ప్రతిజ్ఞా( pOp->p4type==P4_FUNCDEF );
  n = pOp->p5;
  ప్రతిజ్ఞా( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  ప్రతిజ్ఞా( n==0 || (pOp->p2>0 && pOp->p2+n<=(p->nMem+1 - p->nCursor)+1) );
  ప్రతిజ్ఞా( pOp->p3<pOp->p2 || pOp->p3>=pOp->p2+n );

  /* Allocate space for (a) the context object and (n-1) extra pointers
  ** to append to the sqlite3_context.argv[1] array, and (b) a memory
  ** cell in which to store the accumulation. Be careful that the memory
  ** cell is 8-byte aligned, even on platforms where a pointer is 32-bits.
  **
  ** Note: We could avoid this by using a regular memory cell from aMem[] for 
  ** the accumulator, instead of allocating one here. */
  nAlloc = ROUND8P( SZ_CONTEXT(n) );
  pCtx = sqlite3DbMallocRawNN(db, nAlloc + పరిమాణం(Mem));
  ఒకవేళ( pCtx==0 ) వెళ్ళు no_mem;
  pCtx->p_బహిర్గతం = (Mem*)((u8*)pCtx + nAlloc);
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pCtx->p_బహిర్గతం) );

  sqlite3VdbeMemInit(pCtx->p_బహిర్గతం, db, MEM_Null);
  pCtx->pMem = 0;
  pCtx->pFunc = pOp->p4.pFunc;
  pCtx->iOp = (పూర్ణాంకం)(pOp - aOp);
  pCtx->pVdbe = p;
  pCtx->skipFlag = 0;
  pCtx->isError = 0;
  pCtx->enc = encoding;
  pCtx->argc = n;
  pOp->p4type = P4_FUNCCTX;
  pOp->p4.pCtx = pCtx;

  /* OP_AggInverse must have P1==1 and OP_AggStep must have P1==0 */
  ప్రతిజ్ఞా( pOp->p1==(pOp->opcode==OP_AggInverse) );

  pOp->opcode = OP_AggStep1;
  /* Fall through into OP_AggStep */
  /* no break */ deliberate_fall_through
}
స్థితి OP_AggStep1: {
  పూర్ణాంకం i;
  sqlite3_context *pCtx;
  Mem *pMem;

  ప్రతిజ్ఞా( pOp->p4type==P4_FUNCCTX );
  pCtx = pOp->p4.pCtx;
  pMem = &aMem[pOp->p3];

#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( pOp->p1 ){
    /* This is an OP_AggInverse call.  Verify that xStep has always
    ** been called at least once prior to any xInverse call. */
    ప్రతిజ్ఞా( pMem->u_తాత్కాలిక==0x1122e0e3 );
  }లేకపోతే{
    /* This is an OP_AggStep call.  Mark it as such. */
    pMem->u_తాత్కాలిక = 0x1122e0e3;
  }
#అయితే_ముగింపు

  /* If this function is inside of a trigger, the register array in aMem[]
  ** might change from one evaluation to the next.  The next block of code
  ** checks to see if the register array has changed, and if so it
  ** reinitializes the relevant parts of the sqlite3_context object */
  ఒకవేళ( pCtx->pMem != pMem ){
    pCtx->pMem = pMem;
    కొరకు(i=pCtx->argc-1; i>=0; i--) pCtx->argv[i] = &aMem[pOp->p2+i];
  }

#నిర్వచితమైతే SQLITE_DEBUG
  కొరకు(i=0; i<pCtx->argc; i++){
    ప్రతిజ్ఞా( memIsValid(pCtx->argv[i]) );
    రిజిస్టరు_TRACE(pOp->p2+i, pCtx->argv[i]);
  }
#అయితే_ముగింపు

  pMem->n++;
  ప్రతిజ్ఞా( pCtx->p_బహిర్గతం->flags==MEM_Null );
  ప్రతిజ్ఞా( pCtx->isError==0 );
  ప్రతిజ్ఞా( pCtx->skipFlag==0 );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  ఒకవేళ( pOp->p1 ){
    (pCtx->pFunc->xInverse)(pCtx,pCtx->argc,pCtx->argv);
  }లేకపోతే
#అయితే_ముగింపు
  (pCtx->pFunc->xSFunc)(pCtx,pCtx->argc,pCtx->argv); /* IMP: R-24505-23230 */

  ఒకవేళ( pCtx->isError ){
    ఒకవేళ( pCtx->isError>0 ){
      sqlite3VdbeError(p, "%s", sqlite3_విలువ_పాఠ్యం(pCtx->p_బహిర్గతం));
      rc = pCtx->isError;
    }
    ఒకవేళ( pCtx->skipFlag ){
      ప్రతిజ్ఞా( pOp[-1].opcode==OP_CollSeq );
      i = pOp[-1].p1;
      ఒకవేళ( i ) sqlite3_Vdbe_Mem_సమితి_Int64(&aMem[i], 1);
      pCtx->skipFlag = 0;
    }
    sqlite3VdbeMemRelease(pCtx->p_బహిర్గతం);
    pCtx->p_బహిర్గతం->flags = MEM_Null;
    pCtx->isError = 0;
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  }
  ప్రతిజ్ఞా( pCtx->p_బహిర్గతం->flags==MEM_Null );
  ప్రతిజ్ఞా( pCtx->skipFlag==0 );
  ఆపు;
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
స్థితి OP_Agg_విలువ:
స్థితి OP_AggFinal: {
  Mem *pMem;
  ప్రతిజ్ఞా( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  ప్రతిజ్ఞా( pOp->p3==0 || pOp->opcode==OP_Agg_విలువ );
  pMem = &aMem[pOp->p1];
  ప్రతిజ్ఞా( (pMem->flags & ~(MEM_Null|MEM_Agg))==0 );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  ఒకవేళ( pOp->p3 ){
    mem_About_గమ్యం_Change(p, &aMem[pOp->p3]);
    rc = sqlite3_Vdbe_Mem_Agg_విలువ(pMem, &aMem[pOp->p3], pOp->p4.pFunc);
    pMem = &aMem[pOp->p3];
  }లేకపోతే
#అయితే_ముగింపు
  {
    rc = sqlite3VdbeMemFinalize(pMem, pOp->p4.pFunc);
  }
 
  ఒకవేళ( rc ){
    sqlite3VdbeError(p, "%s", sqlite3_విలువ_పాఠ్యం(pMem));
    వెళ్ళు abort_due_గమ్యం_error;
  }
  sqlite3VdbeChangeEncoding(pMem, encoding);
  UPDATE_MAX_BLOBSIZE(pMem);
  రిజిస్టరు_TRACE((పూర్ణాంకం)(pMem-aMem), pMem);
  ఆపు;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
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
స్థితి OP_Checkpoint: {
  పూర్ణాంకం i;                          /* Loop counter */
  పూర్ణాంకం aRes[3];                    /* Results */
  Mem *pMem;                      /* Write results here */

  ప్రతిజ్ఞా( p->readOnly==0 );
  aRes[0] = 0;
  aRes[1] = aRes[2] = -1;
  ప్రతిజ్ఞా( pOp->p2==SQLITE_CHECKPOINT_PASSIVE
       || pOp->p2==SQLITE_CHECKPOINT_నిండిన
       || pOp->p2==SQLITE_CHECKPOINT_RESTART
       || pOp->p2==SQLITE_CHECKPOINT_TRUNCATE
       || pOp->p2==SQLITE_CHECKPOINT_NOOP
  );
  rc = sqlite3Checkpoint(db, pOp->p1, pOp->p2, &aRes[1], &aRes[2]);
  ఒకవేళ( rc ){
    ఒకవేళ( rc!=SQLITE_BUSY ) వెళ్ళు abort_due_గమ్యం_error;
    rc = SQLITE_OK;
    aRes[0] = 1;
  }
  కొరకు(i=0, pMem = &aMem[pOp->p3]; i<3; i++, pMem++){
    sqlite3_Vdbe_Mem_సమితి_Int64(pMem, (i64)aRes[i]);
  }   
  ఆపు;
}; 
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_PRAGMA
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
స్థితి OP_Journal_బహుళకం: {    /* out2 */
  Btree *pBt;                     /* Btree to change journal mode of */
  Pager *pPager;                  /* Pager associated with pBt */
  పూర్ణాంకం eNew;                       /* New journal mode */
  పూర్ణాంకం eOld;                       /* The old journal mode */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  స్థిరరాశి అక్షరం *zFilename;          /* Name of database file for pPager */
#అయితే_ముగింపు

  p_బహిర్గతం = out2Prerelease(p, pOp);
  eNew = pOp->p3;
  ప్రతిజ్ఞా( eNew==PAGER_JOURNALMODE_DELETE
       || eNew==PAGER_JOURNALMODE_TRUNCATE
       || eNew==PAGER_JOURNALMODE_PERSIST
       || eNew==PAGER_JOURNALMODE_OFF
       || eNew==PAGER_JOURNALMODE_జ్ఞాపకం
       || eNew==PAGER_JOURNALMODE_WAL
       || eNew==PAGER_JOURNALMODE_QUERY
  );
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<db->nDb );
  ప్రతిజ్ఞా( p->readOnly==0 );

  pBt = db->aDb[pOp->p1].pBt;
  pPager = sqlite3BtreePager(pBt);
  eOld = sqlite3_Pager_Get_Journal_బహుళకం(pPager);
  ఒకవేళ( eNew==PAGER_JOURNALMODE_QUERY ) eNew = eOld;
  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(pBt) );
  ఒకవేళ( !sqlite3_Pager_Ok_గమ్యం_Change_Journal_బహుళకం(pPager) ) eNew = eOld;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  zFilename = sqlite3PagerFilename(pPager, 1);

  /* Do not allow a transition to journal_mode=WAL for a database
  ** in temporary storage or if the VFS does not support shared memory
  */
  ఒకవేళ( eNew==PAGER_JOURNALMODE_WAL
   && (sqlite3Strlen30(zFilename)==0           /* Temp file */
       || !sqlite3PagerWalSupported(pPager))   /* No shared-memory support */
  ){
    eNew = eOld;
  }

  ఒకవేళ( (eNew!=eOld)
   && (eOld==PAGER_JOURNALMODE_WAL || eNew==PAGER_JOURNALMODE_WAL)
  ){
    ఒకవేళ( !db->autoCommit || db->nVdbeRead>1 ){
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p,
          "cannot change %s wal mode from within a transaction",
          (eNew==PAGER_JOURNALMODE_WAL ? "into" : "out of")
      );
      వెళ్ళు abort_due_గమ్యం_error;
    }లేకపోతే{

      ఒకవేళ( eOld==PAGER_JOURNALMODE_WAL ){
        /* If leaving WAL mode, close the log file. If successful, the call
        ** to PagerCloseWal() checkpoints and deletes the write-ahead-log
        ** file. An EXCLUSIVE lock may still be held on the database file
        ** after a successful return.
        */
        rc = sqlite3PagerCloseWal(pPager, db);
        ఒకవేళ( rc==SQLITE_OK ){
          sqlite3_Pager_సమితి_Journal_బహుళకం(pPager, eNew);
        }
      }లేకపోతే ఒకవేళ( eOld==PAGER_JOURNALMODE_జ్ఞాపకం ){
        /* Cannot transition directly from MEMORY to WAL.  Use mode OFF
        ** as an intermediate */
        sqlite3_Pager_సమితి_Journal_బహుళకం(pPager, PAGER_JOURNALMODE_OFF);
      }
 
      /* Open a transaction on the database file. Regardless of the journal
      ** mode, this transaction always uses a rollback journal.
      */
      ప్రతిజ్ఞా( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE );
      ఒకవేళ( rc==SQLITE_OK ){
        rc = sqlite3_Btree_సమితి_సంస్కరణ(pBt, (eNew==PAGER_JOURNALMODE_WAL ? 2 : 1));
      }
    }
  }
#అయితే_ముగింపు /* ifndef SQLITE_OMIT_WAL */

  ఒకవేళ( rc ) eNew = eOld;
  eNew = sqlite3_Pager_సమితి_Journal_బహుళకం(pPager, eNew);

  p_బహిర్గతం->flags = MEM_Str|MEM_Static|MEM_పదం;
  p_బహిర్గతం->z = (అక్షరం *)sqlite3JournalModename(eNew);
  p_బహిర్గతం->n = sqlite3Strlen30(p_బహిర్గతం->z);
  p_బహిర్గతం->enc = SQLITE_UTF8;
  sqlite3VdbeChangeEncoding(p_బహిర్గతం, encoding);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
};
#అయితే_ముగింపు /* SQLITE_OMIT_PRAGMA */

#ఒకవేళ !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)
/* Opcode: Vacuum P1 P2 * * *
**
** Vacuum the entire database P1.  P1 is 0 for "main", and 2 or more
** for an attached database.  The "temp" database may not be vacuumed.
**
** If P2 is not zero, then it is a register holding a string which is
** the file into which the result of vacuum should be written.  When
** P2 is zero, the vacuum overwrites the original database.
*/
స్థితి OP_Vacuum: {
  ప్రతిజ్ఞా( p->readOnly==0 );
  rc = sqlite3_నిరంతరం_Vacuum(&p->z_Err_సందేశం, db, pOp->p1,
                        pOp->p2 ? &aMem[pOp->p2] : 0);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_AUTOVACUUM)
/* Opcode: IncrVacuum P1 P2 * * *
**
** Perform a single step of the incremental vacuum procedure on
** the P1 database. If the vacuum has finished, jump to instruction
** P2. Otherwise, fall through to the next instruction.
*/
స్థితి OP_IncrVacuum: {        /* jump */
  Btree *pBt;

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<db->nDb );
  ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, pOp->p1) );
  ప్రతిజ్ఞా( p->readOnly==0 );
  pBt = db->aDb[pOp->p1].pBt;
  rc = sqlite3BtreeIncrVacuum(pBt);
  Vdbe_Branch_తీసుకున్న(rc==SQLITE_పూర్తి,2);
  ఒకవేళ( rc ){
    ఒకవేళ( rc!=SQLITE_పూర్తి ) వెళ్ళు abort_due_గమ్యం_error;
    rc = SQLITE_OK;
    వెళ్ళు jump_గమ్యం_p2;
  }
  ఆపు;
}
#అయితే_ముగింపు

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
స్థితి OP_Expire: {
  ప్రతిజ్ఞా( pOp->p2==0 || pOp->p2==1 );
  ఒకవేళ( !pOp->p1 ){
    sqlite3ExpirePreparedStatements(db, pOp->p2);
  }లేకపోతే{
    p->expired = pOp->p2+1;
  }
  ఆపు;
}

/* Opcode: CursorLock P1 * * * *
**
** Lock the btree to which cursor P1 is pointing so that the btree cannot be
** written by an other cursor.
*/
స్థితి OP_Cursor_తాళం: {
  VdbeCursor *pC;
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  sqlite3BtreeCursorPin(pC->uc.pCursor);
  ఆపు;
}

/* Opcode: CursorUnlock P1 * * * *
**
** Unlock the btree to which cursor P1 is pointing so that it can be
** written by other cursors.
*/
స్థితి OP_CursorUnlock: {
  VdbeCursor *pC;
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pC!=0 );
  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  sqlite3BtreeCursorUnpin(pC->uc.pCursor);
  ఆపు;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
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
స్థితి OP_పట్టిక_తాళం: {
  u8 is_Write_తాళం = (u8)pOp->p3;
  ఒకవేళ( is_Write_తాళం || 0==(db->flags&SQLITE_ReadUncommit) ){
    పూర్ణాంకం p1 = pOp->p1;
    ప్రతిజ్ఞా( p1>=0 && p1<db->nDb );
    ప్రతిజ్ఞా( Db_ముసుగు_Test(p->btree_ముసుగు, p1) );
    ప్రతిజ్ఞా( is_Write_తాళం==0 || is_Write_తాళం==1 );
    rc = sqlite3_Btree_తాళం_పట్టిక(db->aDb[p1].pBt, pOp->p2, is_Write_తాళం);
    ఒకవేళ( rc ){
      ఒకవేళ( (rc&0xFF)==SQLITE_LOCKED ){
        స్థిరరాశి అక్షరం *z = pOp->p4.z;
        sqlite3VdbeError(p, "database table is locked: %s", z);
      }
      వెళ్ళు abort_due_గమ్యం_error;
    }
  }
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_SHARED_CACHE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VBegin * * * P4 *
**
** P4 may be a pointer to an sqlite3_vtab structure. If so, call the
** xBegin method for that table.
**
** Also, whether or not P4 is set, check that this is not being called from
** within a callback to a virtual table xSync() method. If it is, the error
** code will be set to SQLITE_LOCKED.
*/
స్థితి OP_VBegin: {
  VTable *pVTab;
  pVTab = pOp->p4.pVtab;
  rc = sqlite3VtabBegin(db, pVTab);
  ఒకవేళ( pVTab ) sqlite3VtabImportErrmsg(p, pVTab->pVtab);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VCreate P1 P2 * * *
**
** P2 is a register that holds the name of a virtual table in database
** P1. Call the xCreate method for that table.
*/
స్థితి OP_VCreate: {
  Mem sMem;          /* For storing the record being decoded */
  స్థిరరాశి అక్షరం *zTab;  /* Name of the virtual table */

  memset(&sMem, 0, పరిమాణం(sMem));
  sMem.db = db;
  /* Because P2 is always a static string, it is impossible for the
  ** sqlite3VdbeMemCopy() to fail */
  ప్రతిజ్ఞా( (aMem[pOp->p2].flags & MEM_Str)!=0 );
  ప్రతిజ్ఞా( (aMem[pOp->p2].flags & MEM_Static)!=0 );
  rc = sqlite3VdbeMemCopy(&sMem, &aMem[pOp->p2]);
  ప్రతిజ్ఞా( rc==SQLITE_OK );
  zTab = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(&sMem);
  ప్రతిజ్ఞా( zTab || db->mallocFailed );
  ఒకవేళ( zTab ){
    rc = sqlite3VtabCallCreate(db, pOp->p1, zTab, &p->z_Err_సందేశం);
  }
  sqlite3VdbeMemRelease(&sMem);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VDestroy P1 * * P4 *
**
** P4 is the name of a virtual table in database P1.  Call the xDestroy method
** of that table.
*/
స్థితి OP_VDestroy: {
  db->nVDestroy++;
  rc = sqlite3VtabCallDestroy(db, pOp->p1, pOp->p4.z);
  db->nVDestroy--;
  ప్రతిజ్ఞా( p->errorAction==OE_Abort && p->ఉపయోగిస్తుంది_Stmt_Journal );
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VOpen P1 * * P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** P1 is a cursor number.  This opcode opens a cursor to the virtual
** table and stores that cursor in P1.
*/
స్థితి OP_VOpen: {             /* ncycle */
  VdbeCursor *pCur;
  sqlite3_vtab_cursor *pVCur;
  sqlite3_vtab *pVtab;
  స్థిరరాశి sqlite3_module *pModule;

  ప్రతిజ్ఞా( p->bIsReader );
  pCur = p->apCsr[pOp->p1];
  ఒకవేళ( pCur!=0
   && ALWAYS( pCur->eCurType==CURTYPE_VTAB )
   && ALWAYS( pCur->uc.pVCur->pVtab==pOp->p4.pVtab->pVtab )
  ){
    /* This opcode is a no-op if the cursor is already open */
    ఆపు;
  }
  pVCur = 0;
  pVtab = pOp->p4.pVtab->pVtab;
  ఒకవేళ( pVtab==0 || NEVER(pVtab->pModule==0) ){
    rc = SQLITE_LOCKED;
    వెళ్ళు abort_due_గమ్యం_error;
  }
  pModule = pVtab->pModule;
  rc = pModule->xOpen(pVtab, &pVCur);
  sqlite3VtabImportErrmsg(p, pVtab);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;

  /* Initialize sqlite3_vtab_cursor base class */
  pVCur->pVtab = pVtab;

  /* Initialize vdbe cursor object */
  pCur = allocateCursor(p, pOp->p1, 0, CURTYPE_VTAB);
  ఒకవేళ( pCur ){
    pCur->uc.pVCur = pVCur;
    pVtab->nRef++;
  }లేకపోతే{
    ప్రతిజ్ఞా( db->mallocFailed );
    pModule->xClose(pVCur);
    వెళ్ళు no_mem;
  }
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VCheck P1 P2 P3 P4 *
**
** P4 is a pointer to a Table object that is a virtual table in schema P1
** that supports the xIntegrity() method.  This opcode runs the xIntegrity()
** method for that virtual table, using P3 as the integer argument.  If
** an error is reported back, the table name is prepended to the error
** message and that message is stored in P2.  If no errors are seen,
** register P2 is set to NULL.
*/
స్థితి OP_VCheck: {             /* out2 */
  పట్టిక *pTab;
  sqlite3_vtab *pVtab;
  స్థిరరాశి sqlite3_module *pModule;
  అక్షరం *zErr = 0;

  p_బహిర్గతం = &aMem[pOp->p2];
  sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);  /* Innocent until proven guilty */
  ప్రతిజ్ఞా( pOp->p4type==P4_TABLEREF );
  pTab = pOp->p4.pTab;
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( pTab->nTabRef>0 );
  ప్రతిజ్ఞా( IsVirtual(pTab) );
  ఒకవేళ( pTab->u.vtab.p==0 ) ఆపు;
  pVtab = pTab->u.vtab.p->pVtab;
  ప్రతిజ్ఞా( pVtab!=0 );
  pModule = pVtab->pModule;
  ప్రతిజ్ఞా( pModule!=0 );
  ప్రతిజ్ఞా( pModule->i_సంస్కరణ>=4 );
  ప్రతిజ్ఞా( pModule->xIntegrity!=0 );
  sqlite3_Vtab_తాళం(pTab->u.vtab.p);
  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<db->nDb );
  rc = pModule->xIntegrity(pVtab, db->aDb[pOp->p1].zDbSName, pTab->zName,
                           pOp->p3, &zErr);
  sqlite3VtabUnlock(pTab->u.vtab.p);
  ఒకవేళ( rc ){
    sqlite3_free(zErr);
    వెళ్ళు abort_due_గమ్యం_error;
  }
  ఒకవేళ( zErr ){
    sqlite3_Vdbe_Mem_సమితి_Str(p_బహిర్గతం, zErr, -1, SQLITE_UTF8, sqlite3_free);
  }
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
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
స్థితి OP_VInit_లోపల: {        /* out2, ncycle */
  VdbeCursor *pC;         /* The cursor containing the RHS values */
  విలువ_List *pRhs;        /* New ValueList object to put in reg[P2] */

  pC = p->apCsr[pOp->p1];
  pRhs = sqlite3_malloc64( పరిమాణం(*pRhs) );
  ఒకవేళ( pRhs==0 ) వెళ్ళు no_mem;
  pRhs->pCsr = pC->uc.pCursor;
  pRhs->p_బహిర్గతం = &aMem[pOp->p3];
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->flags = MEM_Null;
  sqlite3_Vdbe_Mem_సమితి_సూచిక(p_బహిర్గతం, pRhs, "ValueList", sqlite3_Vdbe_విలువ_List_Free);
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */


#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
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
స్థితి OP_VFilter: {   /* jump, ncycle */
  పూర్ణాంకం nArg;
  పూర్ణాంకం iQuery;
  స్థిరరాశి sqlite3_module *pModule;
  Mem *pQuery;
  Mem *pArgc;
  sqlite3_vtab_cursor *pVCur;
  sqlite3_vtab *pVtab;
  VdbeCursor *pCur;
  పూర్ణాంకం res;
  పూర్ణాంకం i;
  Mem **apArg;

  pQuery = &aMem[pOp->p3];
  pArgc = &pQuery[1];
  pCur = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( memIsValid(pQuery) );
  రిజిస్టరు_TRACE(pOp->p3, pQuery);
  ప్రతిజ్ఞా( pCur!=0 );
  ప్రతిజ్ఞా( pCur->eCurType==CURTYPE_VTAB );
  pVCur = pCur->uc.pVCur;
  pVtab = pVCur->pVtab;
  pModule = pVtab->pModule;

  /* Grab the index number and argc parameters */
  ప్రతిజ్ఞా( (pQuery->flags&MEM_Int)!=0 && pArgc->flags==MEM_Int );
  nArg = (పూర్ణాంకం)pArgc->u.i;
  iQuery = (పూర్ణాంకం)pQuery->u.i;

  /* Invoke the xFilter method */
  apArg = p->apArg;
  ప్రతిజ్ఞా( nArg<=p->napArg );
  కొరకు(i = 0; i<nArg; i++){
    apArg[i] = &pArgc[i+1];
  }
  rc = pModule->xFilter(pVCur, iQuery, pOp->p4.z, nArg, apArg);
  sqlite3VtabImportErrmsg(p, pVtab);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  res = pModule->xEof(pVCur);
  pCur->null_అడ్డువరుస = 0;
  Vdbe_Branch_తీసుకున్న(res!=0,2);
  ఒకవేళ( res ) వెళ్ళు jump_గమ్యం_p2;
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
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
స్థితి OP_VColumn: {           /* ncycle */
  sqlite3_vtab *pVtab;
  స్థిరరాశి sqlite3_module *pModule;
  Mem *pDest;
  sqlite3_context sContext;
  FuncDef nullFunc;

  VdbeCursor *pCur = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pCur!=0 );
  ప్రతిజ్ఞా( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  pDest = &aMem[pOp->p3];
  mem_About_గమ్యం_Change(p, pDest);
  ఒకవేళ( pCur->null_అడ్డువరుస ){
    sqlite3_Vdbe_Mem_సమితి_Null(pDest);
    ఆపు;
  }
  ప్రతిజ్ఞా( pCur->eCurType==CURTYPE_VTAB );
  pVtab = pCur->uc.pVCur->pVtab;
  pModule = pVtab->pModule;
  ప్రతిజ్ఞా( pModule->x_స్తంభం );
  memset(&sContext, 0, పరిమాణం(sContext));
  sContext.p_బహిర్గతం = pDest;
  sContext.enc = encoding;
  nullFunc.p_User_దత్తాంశం = 0;
  nullFunc.funcFlags = SQLITE_ఫలితం_SUBTYPE;
  sContext.pFunc = &nullFunc;
  ప్రతిజ్ఞా( pOp->p5==OPFLAG_NOCHNG || pOp->p5==0 );
  ఒకవేళ( pOp->p5 & OPFLAG_NOCHNG ){
    sqlite3_Vdbe_Mem_సమితి_Null(pDest);
    pDest->flags = MEM_Null|MEM_Zero;
    pDest->u.nZero = 0;
  }లేకపోతే{
    Mem_సమితి_Type_Flag(pDest, MEM_Null);
  }
  rc = pModule->x_స్తంభం(pCur->uc.pVCur, &sContext, pOp->p2);
  sqlite3VtabImportErrmsg(p, pVtab);
  ఒకవేళ( sContext.isError>0 ){
    sqlite3VdbeError(p, "%s", sqlite3_విలువ_పాఠ్యం(pDest));
    rc = sContext.isError;
  }
  sqlite3VdbeChangeEncoding(pDest, encoding);
  రిజిస్టరు_TRACE(pOp->p3, pDest);
  UPDATE_MAX_BLOBSIZE(pDest);

  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VNext P1 P2 * * *
**
** Advance virtual table P1 to the next row in its result set and
** jump to instruction P2.  Or, if the virtual table has reached
** the end of its result set, then fall through to the next instruction.
*/
స్థితి OP_VNext: {   /* jump, ncycle */
  sqlite3_vtab *pVtab;
  స్థిరరాశి sqlite3_module *pModule;
  పూర్ణాంకం res;
  VdbeCursor *pCur;

  pCur = p->apCsr[pOp->p1];
  ప్రతిజ్ఞా( pCur!=0 );
  ప్రతిజ్ఞా( pCur->eCurType==CURTYPE_VTAB );
  ఒకవేళ( pCur->null_అడ్డువరుస ){
    ఆపు;
  }
  pVtab = pCur->uc.pVCur->pVtab;
  pModule = pVtab->pModule;
  ప్రతిజ్ఞా( pModule->x_తదుపరి );

  /* Invoke the xNext() method of the module. There is no way for the
  ** underlying implementation to return an error if one occurs during
  ** xNext(). Instead, if an error occurs, true is returned (indicating that
  ** data is available) and the error code returned when xColumn or
  ** some other method is next invoked on the save virtual table cursor.
  */
  rc = pModule->x_తదుపరి(pCur->uc.pVCur);
  sqlite3VtabImportErrmsg(p, pVtab);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  res = pModule->xEof(pCur->uc.pVCur);
  Vdbe_Branch_తీసుకున్న(!res,2);
  ఒకవేళ( !res ){
    /* If there is data, jump to P2 */
    వెళ్ళు jump_గమ్యం_p2_and_check_for_అంతరాయం;
  }
  వెళ్ళు check_for_అంతరాయం;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VRename P1 * * P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** This opcode invokes the corresponding xRename method. The value
** in register P1 is passed as the zName argument to the xRename method.
*/
స్థితి OP_VRename: {
  sqlite3_vtab *pVtab;
  Mem *pName;
  పూర్ణాంకం isLegacy;
 
  isLegacy = (db->flags & SQLITE_LegacyAlter);
  db->flags |= SQLITE_LegacyAlter;
  pVtab = pOp->p4.pVtab->pVtab;
  pName = &aMem[pOp->p1];
  ప్రతిజ్ఞా( pVtab->pModule->xRename );
  ప్రతిజ్ఞా( memIsValid(pName) );
  ప్రతిజ్ఞా( p->readOnly==0 );
  రిజిస్టరు_TRACE(pOp->p1, pName);
  ప్రతిజ్ఞా( pName->flags & MEM_Str );
  testcase( pName->enc==SQLITE_UTF8 );
  testcase( pName->enc==SQLITE_UTF16BE );
  testcase( pName->enc==SQLITE_UTF16LE );
  rc = sqlite3VdbeChangeEncoding(pName, SQLITE_UTF8);
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  rc = pVtab->pModule->xRename(pVtab, pName->z);
  ఒకవేళ( isLegacy==0 ) db->flags &= ~(u64)SQLITE_LegacyAlter;
  sqlite3VtabImportErrmsg(p, pVtab);
  p->expired = 0;
  ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  ఆపు;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
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
స్థితి OP_VUpdate: {
  sqlite3_vtab *pVtab;
  స్థిరరాశి sqlite3_module *pModule;
  పూర్ణాంకం nArg;
  పూర్ణాంకం i;
  sqlite_int64 rowid = 0;
  Mem **apArg;
  Mem *pX;

  ప్రతిజ్ఞా( pOp->p2==1        || pOp->p5==OE_Fail   || pOp->p5==OE_Rollback
       || pOp->p5==OE_Abort || pOp->p5==OE_Ignore || pOp->p5==OE_Replace
  );
  ప్రతిజ్ఞా( p->readOnly==0 );
  ఒకవేళ( db->mallocFailed ) వెళ్ళు no_mem;
  sqlite3VdbeIncrWriteCounter(p, 0);
  pVtab = pOp->p4.pVtab->pVtab;
  ఒకవేళ( pVtab==0 || NEVER(pVtab->pModule==0) ){
    rc = SQLITE_LOCKED;
    వెళ్ళు abort_due_గమ్యం_error;
  }
  pModule = pVtab->pModule;
  nArg = pOp->p2;
  ప్రతిజ్ఞా( pOp->p4type==P4_VTAB );
  ఒకవేళ( ALWAYS(pModule->xUpdate) ){
    u8 vtabOnConflict = db->vtabOnConflict;
    apArg = p->apArg;
    pX = &aMem[pOp->p3];
    ప్రతిజ్ఞా( nArg<=p->napArg );
    కొరకు(i=0; i<nArg; i++){
      ప్రతిజ్ఞా( memIsValid(pX) );
      mem_About_గమ్యం_Change(p, pX);
      apArg[i] = pX;
      pX++;
    }
    db->vtabOnConflict = pOp->p5;
    rc = pModule->xUpdate(pVtab, nArg, apArg, &rowid);
    db->vtabOnConflict = vtabOnConflict;
    sqlite3VtabImportErrmsg(p, pVtab);
    ఒకవేళ( rc==SQLITE_OK && pOp->p1 ){
      ప్రతిజ్ఞా( nArg>1 && apArg[0] && (apArg[0]->flags&MEM_Null) );
      db->చివరి_Rowid = rowid;
    }
    ఒకవేళ( (rc&0xff)==SQLITE_CONSTRAINT && pOp->p4.pVtab->bConstraint ){
      ఒకవేళ( pOp->p5==OE_Ignore ){
        rc = SQLITE_OK;
      }లేకపోతే{
        p->errorAction = ((pOp->p5==OE_Replace) ? OE_Abort : pOp->p5);
      }
    }లేకపోతే{
      p->nChange++;
    }
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  }
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితం_కాకపోతే  SQLITE_OMIT_PAGER_PRAGMAS
/* Opcode: Pagecount P1 P2 * * *
**
** Write the current number of pages in database P1 to memory cell P2.
*/
స్థితి OP_Pagecount: {            /* out2 */
  p_బహిర్గతం = out2Prerelease(p, pOp);
  p_బహిర్గతం->u.i = sqlite3_Btree_చివరి_పుట(db->aDb[pOp->p1].pBt);
  ఆపు;
}
#అయితే_ముగింపు


#నిర్వచితం_కాకపోతే  SQLITE_OMIT_PAGER_PRAGMAS
/* Opcode: MaxPgcnt P1 P2 P3 * *
**
** Try to set the maximum page count for database P1 to the value in P3.
** Do not let the maximum page count fall below the current page count and
** do not change the maximum page count value if P3==0.
**
** Store the maximum page count after the change in register P2.
*/
స్థితి OP_MaxPgcnt: {            /* out2 */
  గుర్తులేని పూర్ణాంకం newMax;
  Btree *pBt;

  p_బహిర్గతం = out2Prerelease(p, pOp);
  pBt = db->aDb[pOp->p1].pBt;
  newMax = 0;
  ఒకవేళ( pOp->p3 ){
    newMax = sqlite3_Btree_చివరి_పుట(pBt);
    ఒకవేళ( newMax < (గుర్తులేని)pOp->p3 ) newMax = (గుర్తులేని)pOp->p3;
  }
  p_బహిర్గతం->u.i = sqlite3_Btree_Max_పుట_లెక్క(pBt, newMax);
  ఆపు;
}
#అయితే_ముగింపు

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
స్థితి OP_శుద్ధ_Func:              /*సమూహం*/
స్థితి OP_Function: {            /*సమూహం*/
  పూర్ణాంకం i;
  sqlite3_context *pCtx;

  ప్రతిజ్ఞా( pOp->p4type==P4_FUNCCTX );
  pCtx = pOp->p4.pCtx;

  /* If this function is inside of a trigger, the register array in aMem[]
  ** might change from one evaluation to the next.  The next block of code
  ** checks to see if the register array has changed, and if so it
  ** reinitializes the relevant parts of the sqlite3_context object */
  p_బహిర్గతం = &aMem[pOp->p3];
  ఒకవేళ( pCtx->p_బహిర్గతం != p_బహిర్గతం ){
    pCtx->pVdbe = p;
    pCtx->p_బహిర్గతం = p_బహిర్గతం;
    pCtx->enc = encoding;
    కొరకు(i=pCtx->argc-1; i>=0; i--) pCtx->argv[i] = &aMem[pOp->p2+i];
  }
  ప్రతిజ్ఞా( pCtx->pVdbe==p );

  mem_About_గమ్యం_Change(p, p_బహిర్గతం);
#నిర్వచితమైతే SQLITE_DEBUG
  కొరకు(i=0; i<pCtx->argc; i++){
    ప్రతిజ్ఞా( memIsValid(pCtx->argv[i]) );
    రిజిస్టరు_TRACE(pOp->p2+i, pCtx->argv[i]);
  }
#అయితే_ముగింపు
  Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Null);
  ప్రతిజ్ఞా( pCtx->isError==0 );
  (*pCtx->pFunc->xSFunc)(pCtx, pCtx->argc, pCtx->argv);/* IMP: R-24505-23230 */

  /* If the function returned an error, throw an exception */
  ఒకవేళ( pCtx->isError ){
    ఒకవేళ( pCtx->isError>0 ){
      sqlite3VdbeError(p, "%s", sqlite3_విలువ_పాఠ్యం(p_బహిర్గతం));
      rc = pCtx->isError;
    }
    sqlite3_Vdbe_Delete_Aux_దత్తాంశం(db, &p->p_Aux_దత్తాంశం, pCtx->iOp, pOp->p1);
    pCtx->isError = 0;
    ఒకవేళ( rc ) వెళ్ళు abort_due_గమ్యం_error;
  }

  ప్రతిజ్ఞా( (p_బహిర్గతం->flags&MEM_Str)==0
       || p_బహిర్గతం->enc==encoding
       || db->mallocFailed );
  ప్రతిజ్ఞా( !sqlite3VdbeMemTooBig(p_బహిర్గతం) );

  రిజిస్టరు_TRACE(pOp->p3, p_బహిర్గతం);
  UPDATE_MAX_BLOBSIZE(p_బహిర్గతం);
  ఆపు;
}

/* Opcode: ClrSubtype P1 * * * *
** Synopsis:  r[P1].subtype = 0
**
** Clear the subtype from register P1.
*/
స్థితి OP_ClrSubtype: {   /* in1 */
  pIn1 = &aMem[pOp->p1];
  pIn1->flags &= ~MEM_Subtype;
  ఆపు;
}

/* Opcode: GetSubtype P1 P2 * * *
** Synopsis:  r[P2] = r[P1].subtype
**
** Extract the subtype value from register P1 and write that subtype
** into register P2.  If P1 has no subtype, then P1 gets a NULL.
*/
స్థితి OP_GetSubtype: {   /* in1 out2 */
  pIn1 = &aMem[pOp->p1];
  p_బహిర్గతం = &aMem[pOp->p2];
  ఒకవేళ( pIn1->flags & MEM_Subtype ){
    sqlite3_Vdbe_Mem_సమితి_Int64(p_బహిర్గతం, pIn1->eSubtype);
  }లేకపోతే{
    sqlite3_Vdbe_Mem_సమితి_Null(p_బహిర్గతం);
  }
  ఆపు;
}

/* Opcode: SetSubtype P1 P2 * * *
** Synopsis:  r[P2].subtype = r[P1]
**
** Set the subtype value of register P2 to the integer from register P1.
** If P1 is NULL, clear the subtype from p2.
*/
స్థితి OP_సమితి_Subtype: {   /* in1 out2 */
  pIn1 = &aMem[pOp->p1];
  p_బహిర్గతం = &aMem[pOp->p2];
  ఒకవేళ( pIn1->flags & MEM_Null ){
    p_బహిర్గతం->flags &= ~MEM_Subtype;
  }లేకపోతే{
    ప్రతిజ్ఞా( pIn1->flags & MEM_Int );
    p_బహిర్గతం->flags |= MEM_Subtype;
    p_బహిర్గతం->eSubtype = (u8)(pIn1->u.i & 0xff);
  }
  ఆపు;
}

/* Opcode: FilterAdd P1 * P3 P4 *
** Synopsis: filter(P1) += key(P3@P4)
**
** Compute a hash on the P4 registers starting with r[P3] and
** add that hash to the bloom filter contained in r[P1].
*/
స్థితి OP_Filter_కూడు: {
  u64 h;

  ప్రతిజ్ఞా( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( pIn1->flags & MEM_Blob );
  ప్రతిజ్ఞా( pIn1->n>0 );
  h = filterHash(aMem, pOp);
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
    పూర్ణాంకం ii;
    కొరకు(ii=pOp->p3; ii<pOp->p3+pOp->p4.i; ii++){
      రిజిస్టరు_Trace(ii, &aMem[ii]);
    }
    రాయి("hash: %llu modulo %d -> %u\n", h, pIn1->n, (పూర్ణాంకం)(h%pIn1->n));
  }
#అయితే_ముగింపు
  h %= (pIn1->n*8);
  pIn1->z[h/8] |= 1<<(h&7);
  ఆపు;
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
స్థితి OP_Filter: {          /* jump */
  u64 h;

  ప్రతిజ్ఞా( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  ప్రతిజ్ఞా( (pIn1->flags & MEM_Blob)!=0 );
  ప్రతిజ్ఞా( pIn1->n >= 1 );
  h = filterHash(aMem, pOp);
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( db->flags&SQLITE_VdbeTrace ){
    పూర్ణాంకం ii;
    కొరకు(ii=pOp->p3; ii<pOp->p3+pOp->p4.i; ii++){
      రిజిస్టరు_Trace(ii, &aMem[ii]);
    }
    రాయి("hash: %llu modulo %d -> %u\n", h, pIn1->n, (పూర్ణాంకం)(h%pIn1->n));
  }
#అయితే_ముగింపు
  h %= (pIn1->n*8);
  ఒకవేళ( (pIn1->z[h/8] & (1<<(h&7)))==0 ){
    Vdbe_Branch_తీసుకున్న(1, 2);
    p->aCounter[SQLITE_STMTSTATUS_FILTER_తగిలింది]++;
    వెళ్ళు jump_గమ్యం_p2;
  }లేకపోతే{
    p->aCounter[SQLITE_STMTSTATUS_FILTER_తప్పిపోవు]++;
    Vdbe_Branch_తీసుకున్న(0, 2);
  }
  ఆపు;
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
స్థితి OP_Trace:
స్థితి OP_Init: {          /* jump0 */
  పూర్ణాంకం i;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
  అక్షరం *zTrace;
#అయితే_ముగింపు

  /* If the P4 argument is not NULL, then it must be an SQL comment string.
  ** The "--" string is broken up to prevent false-positives with srcck1.c.
  **
  ** This assert() provides evidence for:
  ** EVIDENCE-OF: R-50676-09860 The callback can compute the same text that
  ** would have been returned by the legacy sqlite3_trace() interface by
  ** using the X argument when X begins with "--" and invoking
  ** sqlite3_expanded_sql(P) otherwise.
  */
  ప్రతిజ్ఞా( pOp->p4.z==0 || అక్షరశ్రేణి_పోలిక(pOp->p4.z, "-" "- ", 3)==0 );

  /* OP_Init is always instruction 0 */
  ప్రతిజ్ఞా( pOp==p->aOp || pOp->opcode==OP_Trace );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
  ఒకవేళ( (db->mTrace & (SQLITE_TRACE_STMT|SQLITE_TRACE_LEGACY))!=0
   && p->minWriteFileFormat!=254  /* tag-20220401a */
   && (zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql))!=0
  ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
    ఒకవేళ( db->mTrace & SQLITE_TRACE_LEGACY ){
      అక్షరం *z = sqlite3VdbeExpandSql(p, zTrace);
      db->trace.xLegacy(db->pTraceArg, z);
      sqlite3_free(z);
    }లేకపోతే
#అయితే_ముగింపు
    ఒకవేళ( db->nVdbeExec>1 ){
      అక్షరం *z = sqlite3MPrintf(db, "-- %s", zTrace);
      (శూన్య)db->trace.xV2(SQLITE_TRACE_STMT, db->pTraceArg, p, z);
      sqlite3DbFree(db, z);
    }లేకపోతే{
      (శూన్య)db->trace.xV2(SQLITE_TRACE_STMT, db->pTraceArg, p, zTrace);
    }
  }
#నిర్వచితమైతే SQLITE_USE_FCNTL_TRACE
  zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql);
  ఒకవేళ( zTrace ){
    పూర్ణాంకం j;
    కొరకు(j=0; j<db->nDb; j++){
      ఒకవేళ( Db_ముసుగు_Test(p->btree_ముసుగు, j)==0 ) కొనసాగు;
      sqlite3_file_control(db, db->aDb[j].zDbSName, SQLITE_FCNTL_TRACE, zTrace);
    }
  }
#అయితే_ముగింపు /* SQLITE_USE_FCNTL_TRACE */
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( (db->flags & SQLITE_SqlTrace)!=0
   && (zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql))!=0
  ){
    sqlite3DebugPrintf("SQL-trace: %s\n", zTrace);
  }
#అయితే_ముగింపు /* SQLITE_DEBUG */
#అయితే_ముగింపు /* SQLITE_OMIT_TRACE */
  ప్రతిజ్ఞా( pOp->p2>0 );
  ఒకవేళ( pOp->p1>=sqlite3GlobalConfig.i_Once_Reset_పరిమితి ){
    ఒకవేళ( pOp->opcode==OP_Trace ) ఆపు;
    కొరకు(i=1; i<p->nOp; i++){
      ఒకవేళ( p->aOp[i].opcode==OP_Once ) p->aOp[i].p1 = 0;
    }
    pOp->p1 = 0;
  }
  pOp->p1++;
  p->aCounter[SQLITE_STMTSTATUS_నిరంతరం]++;
  వెళ్ళు jump_గమ్యం_p2;
}

#నిర్వచితమైతే SQLITE_ENABLE_CURSOR_HINTS
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
స్థితి OP_CursorHint: {
  VdbeCursor *pC;
  pC = p->apCsr[pOp->p1];

  ప్రతిజ్ఞా( pOp->p1>=0 && pOp->p1<p->nCursor );

  ఒకవేళ( pC ){
    ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
    ఒకవేళ( pOp->p4type==P4_సమాసం ){
      sqlite3BtreeCursorHint(pC->uc.pCursor, BTREE_HINT_RANGE,
          pOp->p4.p_సమాసం, aMem);
    }లేకపోతే ఒకవేళ( p->apCsr[pOp->p3] ){
      sqlite3BtreeCursorHint(
          pC->uc.pCursor, BTREE_HINT_TABLECURSOR, p->apCsr[pOp->p3]->uc.pCursor
      );
    }
  }
  ఆపు;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_CURSOR_HINTS */

#నిర్వచితమైతే SQLITE_DEBUG
/* Opcode:  Abortable   * * * * *
**
** Verify that an Abort can happen.  Assert if an Abort at this point
** might cause database corruption.  This opcode only appears in debugging
** builds.
**
** An Abort is safe if either there have been no writes, or if there is
** an active statement journal.
*/
స్థితి OP_Abortable: {
  sqlite3VdbeAssertAbortable(p);
  ఆపు;
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థితి OP_ReleaseReg: {
  Mem *pMem;
  పూర్ణాంకం i;
  u32 const_ముసుగు;
  ప్రతిజ్ఞా( pOp->p1>0 );
  ప్రతిజ్ఞా( pOp->p1+pOp->p2<=(p->nMem+1 - p->nCursor)+1 );
  pMem = &aMem[pOp->p1];
  const_ముసుగు = pOp->p3;
  కొరకు(i=0; i<pOp->p2; i++, pMem++){
    ఒకవేళ( i>=32 || (const_ముసుగు & MASKBIT32(i))==0 ){
      pMem->pScopyFrom = 0;
      ఒకవేళ( i<32 && pOp->p5 ) Mem_సమితి_Type_Flag(pMem, MEM_Undefined);
    }
  }
  ఆపు;
}
#అయితే_ముగింపు

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
అప్రమేయ: {          /* This is really OP_Noop, OP_Explain */
  ప్రతిజ్ఞా( pOp->opcode==OP_Noop || pOp->opcode==OP_Explain );

  ఆపు;
}

/*****************************************************************************
** The cases of the switch statement above this line should all be indented
** by 6 spaces.  But the left-most 6 spaces have been removed to improve the
** readability.  From this point on down, the normal indentation rules are
** restored.
*****************************************************************************/
    }

#ఒకవేళ defined(VDBE_PROFILE)
    *pn_చక్రం += sqlite3NProfileCnt ? sqlite3NProfileCnt : sqlite3Hwtime();
    pn_చక్రం = 0;
#లేదావేళ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
    ఒకవేళ( pn_చక్రం ){
      *pn_చక్రం += sqlite3Hwtime();
      pn_చక్రం = 0;
    }
#అయితే_ముగింపు

    /* The following code adds nothing to the actual functionality
    ** of the program.  It is only here for testing and debugging.
    ** On the other hand, it does burn CPU cycles every time through
    ** the evaluator loop.  So we can leave it out when NDEBUG is defined.
    */
#నిర్వచితం_కాకపోతే NDEBUG
    ప్రతిజ్ఞా( pOp>=&aOp[-1] && pOp<&aOp[p->nOp-1] );

#నిర్వచితమైతే SQLITE_DEBUG
    ఒకవేళ( db->flags & SQLITE_VdbeTrace ){
      u8 opProperty = sqlite3OpcodeProperty[pOrigOp->opcode];
      ఒకవేళ( rc!=0 ) రాయి("rc=%d\n",rc);
      ఒకవేళ( opProperty & (OPFLG_OUT2) ){
        రిజిస్టరు_Trace(pOrigOp->p2, &aMem[pOrigOp->p2]);
      }
      ఒకవేళ( opProperty & OPFLG_OUT3 ){
        రిజిస్టరు_Trace(pOrigOp->p3, &aMem[pOrigOp->p3]);
      }
      ఒకవేళ( opProperty==0xff ){
        /* Never happens.  This code exists to avoid a harmless linkage
        ** warning about sqlite3VdbeRegisterDump() being defined but not
        ** used. */
        sqlite3_Vdbe_రిజిస్టరు_Dump(p);
      }
    }
#అయితే_ముగింపు  /* SQLITE_DEBUG */
#అయితే_ముగింపు  /* NDEBUG */
  }  /* The end of the for(;;) loop the loops through opcodes */

  /* If we reach this point, it means that execution is finished with
  ** an error of some kind.
  */
abort_due_గమ్యం_error:
  ఒకవేళ( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
  }లేకపోతే ఒకవేళ( rc==SQLITE_IOERR_CORRUPTFS ){
    rc = SQLITE_CORRUPT_BKPT;
  }
  ప్రతిజ్ఞా( rc );
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( db->flags & SQLITE_VdbeTrace ){
    స్థిరరాశి అక్షరం *zTrace = p->zSql;
    ఒకవేళ( zTrace==0 ){
      ఒకవేళ( aOp[0].opcode==OP_Trace ){
        zTrace = aOp[0].p4.z;
      }
      ఒకవేళ( zTrace==0 ) zTrace = "???";
    }
    రాయి("ABORT-due-to-error (rc=%d): %s\n", rc, zTrace);
  }
#అయితే_ముగింపు
  ఒకవేళ( p->z_Err_సందేశం==0 && rc!=SQLITE_IOERR_NOMEM ){
    sqlite3VdbeError(p, "%s", sqlite3ErrStr(rc));
  }
  p->rc = rc;
  sqlite3SystemError(db, rc);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  sqlite3VdbeLogAbort(p, rc, pOp, aOp);
  ఒకవేళ( p->eVdbeState==VDBE_నిరంతరం_STATE ) sqlite3VdbeHalt(p);
  ఒకవేళ( rc==SQLITE_IOERR_NOMEM ) sqlite3OomFault(db);
  ఒకవేళ( rc==SQLITE_CORRUPT && db->autoCommit==0 ){
    db->flags |= SQLITE_CorruptRdOnly;
  }
  rc = SQLITE_ERROR;
  ఒకవేళ( resetSchemaOnFault>0 ){
    sqlite3ResetOneSchema(db, resetSchemaOnFault-1);
  }

  /* This is the only way out of this procedure.  We have to
  ** release the mutexes on btrees that were acquired at the
  ** top. */
vdbe_return:
#ఒకవేళ defined(VDBE_PROFILE)
  ఒకవేళ( pn_చక్రం ){
    *pn_చక్రం += sqlite3NProfileCnt ? sqlite3NProfileCnt : sqlite3Hwtime();
    pn_చక్రం = 0;
  }
#లేదావేళ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
  ఒకవేళ( pn_చక్రం ){
    *pn_చక్రం += sqlite3Hwtime();
    pn_చక్రం = 0;
  }
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
  అయితే( n_Vm_అడుగు>=n_పురోగతి_హద్దు && db->x_పురోగతి!=0 ){
    n_పురోగతి_హద్దు += db->n_పురోగతి_Ops;
    ఒకవేళ( db->x_పురోగతి(db->p_పురోగతి_Arg) ){
      n_పురోగతి_హద్దు = అతిపెద్ద_UINT64;
      rc = SQLITE_అంతరాయం;
      వెళ్ళు abort_due_గమ్యం_error;
    }
  }
#అయితే_ముగింపు
  p->aCounter[SQLITE_STMTSTATUS_VM_అడుగు] += (పూర్ణాంకం)n_Vm_అడుగు;
  ఒకవేళ( Db_ముసుగు_Non_Zero(p->తాళం_ముసుగు) ){
    sqlite3VdbeLeave(p);
  }
  ప్రతిజ్ఞా( rc!=SQLITE_OK || nExtraDelete==0
       || sqlite3_strlike("DELETE%",p->zSql,0)!=0
  );
  తిరిగి rc;

  /* Jump to here if a string or blob larger than SQLITE_MAX_LENGTH
  ** is encountered.
  */
too_big:
  sqlite3VdbeError(p, "string or blob too big");
  rc = SQLITE_TOOBIG;
  వెళ్ళు abort_due_గమ్యం_error;

  /* Jump to here if a malloc() fails.
  */
no_mem:
  sqlite3OomFault(db);
  sqlite3VdbeError(p, "out of memory");
  rc = SQLITE_NOMEM_BKPT;
  వెళ్ళు abort_due_గమ్యం_error;

  /* Jump to here if the sqlite3_interrupt() API sets the interrupt
  ** flag.
  */
abort_due_గమ్యం_అంతరాయం:
  ప్రతిజ్ఞా( Atomic_ఎక్కించు(&db->u1.isInterrupted) );
  rc = SQLITE_అంతరాయం;
  వెళ్ళు abort_due_గమ్యం_error;
}
