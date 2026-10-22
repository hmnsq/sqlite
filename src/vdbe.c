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
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

/*
** High-resolution hardware timer used for debugging and testing only.
*/
#إذا defined(VDBE_PROFILE)  \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
# include "hwtime.h"
#endif

/*
** Invoke this macro on memory cells just prior to changing the
** value of the cell.  This macro verifies that shallow copies are
** not misused.  A shallow copy of a string or blob just copies a
** pointer to the string or blob, not the content.  If the original
** is changed while the copy is still in use, the string or blob might
** be changed out from under the copy.  This macro verifies that nothing
** like that ever happens.
*/
#ifdef SQLITE_DEBUG
# define memAboutToChange(P,M) sqlite3VdbeMemAboutToChange(P,M)
#وإلا
# define memAboutToChange(P,M)
#endif

/*
** The following global variable is incremented every time a cursor
** moves, either by the OP_SeekXX, OP_Next, or OP_Prev opcodes.  The test
** procedures use this information to make sure that indices are
** working correctly.  This variable has no function other than to
** help verify the correct operation of the library.
*/
#ifdef SQLITE_TEST
صحيح sqlite3_ئىزدەش_سانى = 0;
#endif

/*
** When this global variable is positive, it gets decremented once before
** each instruction in the VDBE.  When it reaches zero, the u1.isInterrupted
** field of the sqlite3 structure is set in order to simulate an interrupt.
**
** This facility is used for testing purposes only.  It does not function
** in an ordinary build.
*/
#ifdef SQLITE_TEST
صحيح sqlite3_interrupt_سانى = 0;
#endif

/*
** The next global variable is incremented each type the OP_Sort opcode
** is executed.  The test procedures use this information to make sure that
** sorting is occurring or not occurring at appropriate times.   This variable
** has no function other than to help verify the correct operation of the
** library.
*/
#ifdef SQLITE_TEST
صحيح sqlite3_sort_سانى = 0;
#endif

/*
** The next global variable records the size of the largest MEM_Blob
** or MEM_Str that has been used by a VDBE opcode.  The test procedures
** use this information to make sure that the zero-blob functionality
** is working correctly.   This variable has no function other than to
** help verify the correct operation of the library.
*/
#ifdef SQLITE_TEST
صحيح sqlite3_max_blobsize = 0;
ساكن فراغ updateMaxBlobsize(Mem *p){
  إذا( (p->flags & (MEM_Str|MEM_Blob))!=0 && p->n>sqlite3_max_blobsize ){
    sqlite3_max_blobsize = p->n;
  }
}
#endif

/*
** This macro evaluates to true if either the update hook or the preupdate
** hook are enabled for database connect DB.
*/
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
# define HAS_UPDATE_HOOK(DB) ((DB)->xPreUpdateCallback||(DB)->xUpdateCallback)
#وإلا
# define HAS_UPDATE_HOOK(DB) ((DB)->xUpdateCallback)
#endif

/*
** The next global variable is incremented each time the OP_Found opcode
** is executed. This is used to test whether or not the foreign key
** operation implemented using OP_FkIsZero is working. This variable
** has no function other than to help verify the correct operation of the
** library.
*/
#ifdef SQLITE_TEST
صحيح sqlite3_found_سانى = 0;
#endif

/*
** Test a register to see if it exceeds the current maximum blob size.
** If it does, record the new maximum blob size.
*/
#إذا defined(SQLITE_TEST) && !defined(SQLITE_UNTESTABLE)
# define UPDATE_MAX_BLOBSIZE(P)  updateMaxBlobsize(P)
#وإلا
# define UPDATE_MAX_BLOBSIZE(P)
#endif

#ifdef SQLITE_DEBUG
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
ساكن فراغ test_trace_breakpoint(صحيح pc, Op *pOp, Vdbe *v){
  ساكن u64 n = 0;
  (فراغ)pc;
  (فراغ)pOp;
  (فراغ)v;
  n++;
  إذا( n==LARGEST_UINT64 ) abort(); /* So that n is used, preventing a warning */
}
#endif

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
#إذا !defined(SQLITE_VDBE_COVERAGE)
# define VdbeBranchTaken(I,M)
#وإلا
# define VdbeBranchTaken(I,M) vdbeTakeBranch(pOp->i_Src_قۇر,I,M)
  ساكن فراغ vdbeTakeBranch(u32 i_Src_قۇر, u8 I, u8 M){
    u8 mNever;
    أكد( I<=2 );  /* 0: fall through,  1: taken,  2: alternate taken */
    أكد( M<=4 );  /* 2: two-way branch, 3: three-way branch, 4: OP_Jump */
    أكد( I<M );   /* I can only be 2 if M is 3 or 4 */
    /* Transform I from a integer [0,1,2] into a bitmask of [1,2,4] */
    I = 1<<I;
    /* The upper 8 bits of iSrcLine are flags.  The lower three bits of
    ** the flags indicate directions that the branch can never go.  If
    ** a branch really does go in one of those directions, assert right
    ** away. */
    mNever = i_Src_قۇر >> 24;
    أكد( (I & mNever)==0 );
    إذا( sqlite3GlobalConfig.xVdbeBranch==0 ) أرجع;  /*NO_TEST*/
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
    إذا( M==2 ) I |= 0x04;
    إذا( M==4 ){
      I |= 0x08;
      إذا( (mNever&0x08)!=0 && (I&0x05)!=0) I |= 0x05; /*NO_TEST*/
    }
    sqlite3GlobalConfig.xVdbeBranch(sqlite3GlobalConfig.pVdbeBranchArg,
                                    i_Src_قۇر&0xffffff, I, M);
  }
#endif

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
#define Deephemeralize(P) \
   إذا( ((P)->flags&MEM_Ephem)!=0 \
       && sqlite3VdbeMemMakeWriteable(P) ){ اذهب no_mem;}

/* Return true if the cursor was opened using the OP_OpenSorter opcode. */
#define isSorter(x) ((x)->eCurType==CURTYPE_SORTER)

/*
** Allocate VdbeCursor number iCur.  Return a pointer to it.  Return NULL
** if we run out of memory.
*/
ساكن VdbeCursor *allocateCursor(
  Vdbe *p,              /* The virtual machine */
  صحيح iCur,             /* Index of the new VdbeCursor */
  صحيح nField,           /* Number of fields in the table or index */
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
  nByte = SZ_VDBECURSOR(nField);
  أكد( ROUND8(nByte)==nByte );
  إذا( eCurType==CURTYPE_BTREE ) nByte += sqlite3BtreeCursorSize();

  أكد( iCur>=0 && iCur<p->nCursor );
  إذا( p->apCsr[iCur] ){ /*OPTIMIZATION-IF-FALSE*/
    sqlite3VdbeFreeCursorNN(p, p->apCsr[iCur]);
    p->apCsr[iCur] = 0;
  }

  /* There used to be a call to sqlite3VdbeMemClearAndResize() to make sure
  ** the pMem used to hold space for the cursor has enough storage available
  ** in pMem->zMalloc.  But for the special case of the aMem[] entries used
  ** to hold cursors, it is faster to in-line the logic. */
  أكد( pMem->flags==MEM_Undefined );
  أكد( (pMem->flags & MEM_Dyn)==0 );
  أكد( pMem->szMalloc==0 || pMem->z==pMem->zMalloc );
  إذا( pMem->szMalloc<nByte ){
    إذا( pMem->szMalloc>0 ){
      sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    }
    pMem->z = pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, nByte);
    إذا( pMem->zMalloc==0 ){
      pMem->szMalloc = 0;
      أرجع 0;
    }
    pMem->szMalloc = (صحيح)nByte;
  }

  p->apCsr[iCur] = pCx = (VdbeCursor*)pMem->zMalloc;
  memset(pCx, 0, offsetof(VdbeCursor,pAltCursor));
  pCx->eCurType = eCurType;
  pCx->nField = nField;
  pCx->aOffset = &pCx->aType[nField];
  إذا( eCurType==CURTYPE_BTREE ){
    أكد( ROUND8(SZ_VDBECURSOR(nField))==SZ_VDBECURSOR(nField) );
    pCx->uc.pCursor = (BtCursor*)&pMem->z[SZ_VDBECURSOR(nField)];
    sqlite3BtreeCursorZero(pCx->uc.pCursor);
  }
  أرجع pCx;
}

/*
** The string in pRec is known to look like an integer and to have a
** floating point value of rValue.  Return true and set *piValue to the
** integer value if the string is in range to be an integer.  Otherwise,
** return false.
*/
ساكن صحيح alsoAnInt(Mem *pRec, مزدوج r_قىممەت, i64 *pi_قىممەت){
  i64 i_قىممەت;
  i_قىممەت = sqlite3RealToI64(r_قىممەت);
  إذا( sqlite3RealSameAsInt(r_قىممەت,i_قىممەت) ){
    *pi_قىممەت = i_قىممەت;
    أرجع 1;
  }
  أرجع 0==sqlite3Atoi64(pRec->z, pi_قىممەت, pRec->n, pRec->enc);
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
ساكن فراغ applyNumericAffinity(Mem *pRec, صحيح bTryForInt){
  مزدوج r_قىممەت;
  صحيح rc;
  أكد( (pRec->flags & (MEM_Str|MEM_Int|MEM_Real|MEM_IntReal))==MEM_Str );
  rc = sqlite3_Mem_Real_قىممەت_RC(pRec, &r_قىممەت);
  إذا( rc<=0 ) أرجع;
  إذا( (rc&2)==0 && alsoAnInt(pRec, r_قىممەت, &pRec->u.i) ){
    pRec->flags |= MEM_Int;
  }وإلا{
    pRec->u.r = r_قىممەت;
    pRec->flags |= MEM_Real;
    إذا( bTryForInt ) sqlite3VdbeIntegerAffinity(pRec, 1);
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
ساكن فراغ applyAffinity(
  Mem *pRec,          /* The value to apply affinity to */
  حرف affinity,      /* The affinity to be applied */
  u8 enc              /* Use this text encoding */
){
  إذا( affinity>=SQLITE_AFF_NUMERIC ){
    أكد( affinity==SQLITE_AFF_INTEGER || affinity==SQLITE_AFF_REAL
             || affinity==SQLITE_AFF_NUMERIC || affinity==SQLITE_AFF_FLEXNUM );
    إذا( (pRec->flags & MEM_Int)==0 ){ /*OPTIMIZATION-IF-FALSE*/
      إذا( (pRec->flags & (MEM_Real|MEM_IntReal))==0 ){
        إذا( pRec->flags & MEM_Str ) applyNumericAffinity(pRec,1);
      }وإلا إذا( affinity<=SQLITE_AFF_REAL ){
        sqlite3VdbeIntegerAffinity(pRec, 0);
      }
    }
  }وإلا إذا( affinity==SQLITE_AFF_TEXT ){
    /* Only attempt the conversion to TEXT if there is an integer or real
    ** representation (blob and NULL do not get converted) but no string
    ** representation.  It would be harmless to repeat the conversion if
    ** there is already a string rep, but it is pointless to waste those
    ** CPU cycles. */
    إذا( 0==(pRec->flags&MEM_Str) ){ /*OPTIMIZATION-IF-FALSE*/
      إذا( (pRec->flags&(MEM_Real|MEM_Int|MEM_IntReal)) ){
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
صحيح sqlite3_قىممەت_numeric_type(sqlite3_قىممەت *pVal){
  صحيح eType = sqlite3_قىممەت_type(pVal);
  إذا( eType==SQLITE_TEXT ){
    Mem *pMem = (Mem*)pVal;
#إذا SQLITE_THREADSAFE>0
    sqlite3_mutex *pMutex = pMem->db ? pMem->db->mutex : 0;
#endif
    sqlite3_mutex_enter(pMutex);
    applyNumericAffinity(pMem, 0);
    sqlite3_mutex_leave(pMutex);
    eType = sqlite3_قىممەت_type(pVal);
  }
  أرجع eType;
}

/*
** Exported version of applyAffinity(). This one works on sqlite3_value*,
** not the internal Mem* type.
*/
فراغ sqlite3_قىممەت_Apply_Affinity(
  sqlite3_قىممەت *pVal,
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
ساكن u16 SQLITE_NOINLINE computeNumericType(Mem *pMem){
  صحيح rc;
  sqlite3_int64 ix;
  أكد( (pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal))==0 );
  أكد( (pMem->flags & (MEM_Str|MEM_Blob))!=0 );
  إذا( ExpandBlob(pMem) ){
    pMem->u.i = 0;
    أرجع MEM_Int;
  }
  rc = sqlite3_Mem_Real_قىممەت_RC(pMem, &pMem->u.r);
  إذا( rc<=0 ){
    إذا( (rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)<=1 ){
      pMem->u.i = ix;
      أرجع MEM_Int;
    }وإلا{
      أرجع MEM_Real;
    }
  }وإلا إذا( (rc&2)==0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc)==0 ){
    pMem->u.i = ix;
    أرجع MEM_Int;
  }
  أرجع MEM_Real;
}

/*
** Return the numeric type for pMem, either MEM_Int or MEM_Real or both or
** none. 
**
** Unlike applyNumericAffinity(), this routine does not modify pMem->flags.
** But it does set pMem->u.r and pMem->u.i appropriately.
*/
ساكن u16 numericType(Mem *pMem){
  أكد( (pMem->flags & MEM_Null)==0
       || pMem->db==0 || pMem->db->mallocFailed );
  إذا( pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null) ){
    testcase( pMem->flags & MEM_Int );
    testcase( pMem->flags & MEM_Real );
    testcase( pMem->flags & MEM_IntReal );
    أرجع pMem->flags & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Null);
  }
  أكد( pMem->flags & (MEM_Str|MEM_Blob) );
  testcase( pMem->flags & MEM_Str );
  testcase( pMem->flags & MEM_Blob );
  أرجع computeNumericType(pMem);
  أرجع 0;
}

#ifdef SQLITE_DEBUG
/*
** Write a nice string representation of the contents of cell pMem
** into buffer zBuf, length nBuf.
*/
فراغ sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr){
  صحيح f = pMem->flags;
  ساكن ثابت حرف *ثابت encnames[] = {"(X)", "(8)", "(16LE)", "(16BE)"};
  إذا( f&MEM_Blob ){
    صحيح i;
    حرف c;
    إذا( f & MEM_Dyn ){
      c = 'z';
      أكد( (f & (MEM_Static|MEM_Ephem))==0 );
    }وإلا إذا( f & MEM_Static ){
      c = 't';
      أكد( (f & (MEM_Dyn|MEM_Ephem))==0 );
    }وإلا إذا( f & MEM_Ephem ){
      c = 'e';
      أكد( (f & (MEM_Static|MEM_Dyn))==0 );
    }وإلا{
      c = 's';
    }
    sqlite3_str_appendf(pStr, "%cx[", c);
    لكل(i=0; i<25 && i<pMem->n; i++){
      sqlite3_str_appendf(pStr, "%02X", ((صحيح)pMem->z[i] & 0xFF));
    }
    sqlite3_str_appendf(pStr, "|");
    لكل(i=0; i<25 && i<pMem->n; i++){
      حرف z = pMem->z[i];
      sqlite3_str_appendchar(pStr, 1, (z<32||z>126)?'.':z);
    }
    sqlite3_str_appendf(pStr,"]");
    إذا( f & MEM_Zero ){
      sqlite3_str_appendf(pStr, "+%dz",pMem->u.nZero);
    }
  }وإلا إذا( f & MEM_Str ){
    صحيح j;
    u8 c;
    إذا( f & MEM_Dyn ){
      c = 'z';
      أكد( (f & (MEM_Static|MEM_Ephem))==0 );
    }وإلا إذا( f & MEM_Static ){
      c = 't';
      أكد( (f & (MEM_Dyn|MEM_Ephem))==0 );
    }وإلا إذا( f & MEM_Ephem ){
      c = 'e';
      أكد( (f & (MEM_Static|MEM_Dyn))==0 );
    }وإلا{
      c = 's';
    }
    sqlite3_str_appendf(pStr, " %c%d[", c, pMem->n);
    لكل(j=0; j<25 && j<pMem->n; j++){
      c = pMem->z[j];
      sqlite3_str_appendchar(pStr, 1, (c>=0x20&&c<=0x7f) ? c : '.');
    }
    sqlite3_str_appendf(pStr, "]%s", encnames[pMem->enc]);
    إذا( f & MEM_Term ){
      sqlite3_str_appendf(pStr, "(0-term)");
    }
  }
}
#endif

#ifdef SQLITE_DEBUG
/*
** Print the value of a register for tracing purposes:
*/
ساكن فراغ memTracePrint(Mem *p){
  إذا( p->flags & MEM_Undefined ){
    printf(" undefined");
  }وإلا إذا( p->flags & MEM_Null ){
    printf(p->flags & MEM_Zero ? " NULL-nochng" : " NULL");
  }وإلا إذا( (p->flags & (MEM_Int|MEM_Str))==(MEM_Int|MEM_Str) ){
    printf(" si:%lld", p->u.i);
  }وإلا إذا( (p->flags & (MEM_IntReal))!=0 ){
    printf(" ir:%lld", p->u.i);
  }وإلا إذا( p->flags & MEM_Int ){
    printf(" i:%lld", p->u.i);
#ifndef SQLITE_OMIT_FLOATING_POINT
  }وإلا إذا( p->flags & MEM_Real ){
    printf(" r:%.17g", p->u.r);
#endif
  }وإلا إذا( sqlite3VdbeMemIsRowSet(p) ){
    printf(" (rowset)");
  }وإلا{
    StrAccum acc;
    حرف zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
    sqlite3VdbeMemPrettyPrint(p, &acc);
    printf(" %s", sqlite3StrAccumFinish(&acc));
  }
  إذا( p->flags & MEM_Subtype ) printf(" subtype=0x%02x", p->eSubtype);
}
ساكن فراغ registerTrace(صحيح iReg, Mem *p){
  printf("R[%d] = ", iReg);
  memTracePrint(p);
  إذا( p->pScopyFrom ){
    أكد( p->pScopyFrom->bScopy );
    printf(" <== R[%d]", (صحيح)(p->pScopyFrom - &p[-iReg]));
  }
  printf("\n");
  sqlite3VdbeCheckMemInvariants(p);
}
/**/ فراغ sqlite3PrintMem(Mem *pMem){
  memTracePrint(pMem);
  printf("\n");
  fflush(stdout);
}
#endif

#ifdef SQLITE_DEBUG
/*
** Show the values of all registers in the virtual machine.  Used for
** interactive debugging.
*/
فراغ sqlite3VdbeRegisterDump(Vdbe *v){
  صحيح i;
  لكل(i=1; i<v->nMem; i++) registerTrace(i, v->aMem+i);
}
#endif /* SQLITE_DEBUG */


#ifdef SQLITE_DEBUG
#  define REGISTER_TRACE(R,M) إذا(db->flags&SQLITE_VdbeTrace)registerTrace(R,M)
#وإلا
#  define REGISTER_TRACE(R,M)
#endif

#ifndef NDEBUG
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
ساكن صحيح check_Savepoint_سانى(sqlite3 *db){
  صحيح n = 0;
  Savepoint *p;
  لكل(p=db->pSavepoint; p; p=p->pNext) n++;
  أكد( n==(db->nSavepoint + db->isTransactionSavepoint) );
  أرجع 1;
}
#endif

/*
** Return the register of pOp->p2 after first preparing it to be
** overwritten with an integer value.
*/
ساكن SQLITE_NOINLINE Mem *out2PrereleaseWithClear(Mem *pOut){
  sqlite3VdbeMemSetNull(pOut);
  pOut->flags = MEM_Int;
  أرجع pOut;
}
ساكن Mem *out2Prerelease(Vdbe *p, VdbeOp *pOp){
  Mem *pOut;
  أكد( pOp->p2>0 );
  أكد( pOp->p2<=(p->nMem+1 - p->nCursor) );
  pOut = &p->aMem[pOp->p2];
  memAboutToChange(p, pOut);
  إذا( VdbeMemDynamic(pOut) ){ /*OPTIMIZATION-IF-FALSE*/
    أرجع out2PrereleaseWithClear(pOut);
  }وإلا{
    pOut->flags = MEM_Int;
    أرجع pOut;
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
ساكن u64 filterHash(ثابت Mem *aMem, ثابت Op *pOp){
  صحيح i, mx;
  u64 h = 0;

  أكد( pOp->p4type==P4_INT32 );
  لكل(i=pOp->p3, mx=i+pOp->p4.i; i<mx; i++){
    ثابت Mem *p = &aMem[i];
    إذا( p->flags & (MEM_Int|MEM_IntReal) ){
      h += p->u.i;
    }وإلا إذا( p->flags & MEM_Real ){
      h += sqlite3_Vdbe_Int_قىممەت(p);
    }وإلا إذا( p->flags & MEM_Str ){
      u64 x;
      h += p->n;
      إذا( p->n >= (صحيح)حجم(x) ){
        memcpy(&x, p->z, حجم(x));
        h += x;
        memcpy(&x, p->z + p->n - حجم(x), حجم(x));
        h += x;
      }وإلا{
        x = 0;
        memcpy(&x, p->z, p->n);
        h += x;
      }
    }وإلا إذا( p->flags & MEM_Blob ){
      صحيح n = p->n;
      u64 x = 0;
      إذا( n ){
        memcpy(&x, p->z, MIN(n, (صحيح)حجم(x)));
        h += x;
      }
      h += n;
      إذا( p->flags & MEM_Zero ) h += p->u.nZero;
    }
  }
  أرجع h;
}

/*
** For OP_Column, factor out the case where content is loaded from
** overflow pages, so that the code to implement this case is separate
** the common case where all content fits on the page.  Factoring out
** the code reduces register pressure and helps the common case
** to run faster.
*/
ساكن SQLITE_NOINLINE صحيح vdbeColumnFromOverflow(
  VdbeCursor *pC,       /* The BTree cursor from which we are reading */
  صحيح iCol,             /* The column to read */
  u32 t,                /* The serial-type code for the column value */
  i64 iOffset,          /* Offset to the start of the content value */
  u32 cacheStatus,      /* Current Vdbe.cacheCtr value */
  u32 colCacheCtr,      /* Current value of the column cache counter */
  Mem *pDest            /* Store the value into this register. */
){
  صحيح rc;
  sqlite3 *db = pDest->db;
  صحيح encoding = pDest->enc;
  صحيح len = sqlite3VdbeSerialTypeLen(t);
  أكد( pC->eCurType==CURTYPE_BTREE );
  إذا( len>db->aLimit[SQLITE_LIMIT_LENGTH] ) أرجع SQLITE_TOOBIG;
  إذا( len > 4000 && pC->p_ئاچقۇچ_Info==0 ){
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
    حرف *pBuf;
    إذا( pC->colCache==0 ){
      pC->pCache = sqlite3DbMallocZero(db, حجم(VdbeTxtBlbCache) );
      إذا( pC->pCache==0 ) أرجع SQLITE_NOMEM;
      pC->colCache = 1;
    }
    pCache = pC->pCache;
    إذا( pCache->pCValue==0
     || pCache->iCol!=iCol
     || pCache->cacheStatus!=cacheStatus
     || pCache->colCacheCtr!=colCacheCtr
     || pCache->iOffset!=sqlite3BtreeOffset(pC->uc.pCursor)
    ){
      إذا( pCache->pCValue ) sqlite3RCStrUnref(pCache->pCValue);
      pBuf = pCache->pCValue = sqlite3RCStrNew( len+3 );
      إذا( pBuf==0 ) أرجع SQLITE_NOMEM;
      rc = sqlite3BtreePayload(pC->uc.pCursor, iOffset, len, pBuf);
      إذا( rc ) أرجع rc;
      pBuf[len] = 0;
      pBuf[len+1] = 0;
      pBuf[len+2] = 0;
      pCache->iCol = iCol;
      pCache->cacheStatus = cacheStatus;
      pCache->colCacheCtr = colCacheCtr;
      pCache->iOffset = sqlite3BtreeOffset(pC->uc.pCursor);
    }وإلا{
      pBuf = pCache->pCValue;
    }
    أكد( t>=12 );
    sqlite3RCStrRef(pBuf);
    إذا( t&1 ){
      /* Prevents the removal of a BOM in UTF16 text.  Otherwise
      ** it causes no harm as long as len>=0, and pDest->enc is set
      ** to the correct encoding afterwards: ----
      ** (tag-20261016-1)                        `vvvvvvvvvvv    */
      rc = sqlite3VdbeMemSetStr(pDest, pBuf, len, SQLITE_UTF8,
                                sqlite3RCStrUnref);
      pDest->flags |= MEM_Term;
    }وإلا{
      rc = sqlite3VdbeMemSetStr(pDest, pBuf, len, 0,
                                sqlite3RCStrUnref);
     }
    pDest->enc = encoding;
  }وإلا{
    rc = sqlite3VdbeMemFromBtree(pC->uc.pCursor, iOffset, len, pDest);
    إذا( rc ) أرجع rc;
    sqlite3VdbeSerialGet((ثابت u8*)pDest->z, t, pDest);
    إذا( (t&1)!=0 && encoding==SQLITE_UTF8 ){
      pDest->z[len] = 0;
      pDest->flags |= MEM_Term;
    }
  }
  pDest->flags &= ~MEM_Ephem;
  أرجع rc;
}

/*
** Memory cell pMem may contain a blob or a NULL value. Cursor pCsr is
** open on an index. If the current index entry matches the blob value in
** pMem byte-for-byte, set pMem to NULL and return 1. Otherwise, return 0.
**
** If an error occurs, set (*pRc) to an SQLite error code. Return 1 in this
** case as well.
*/
ساكن SQLITE_NOINLINE صحيح vdbe_Index_ئاچقۇچ_Compare(
  BtCursor *pCsr,                 /* Cursor to compare key to */
  Mem *pMem,
  صحيح *pRc
){
  صحيح ret = 0;
  u32 n_ئاچقۇچ = 0;

  أكد( pMem->flags & (MEM_Blob|MEM_Null) );
  n_ئاچقۇچ = sqlite3BtreePayloadSize(pCsr);
  إذا( n_ئاچقۇچ==(u32)pMem->n && ALWAYS((pMem->flags & MEM_Blob)!=0) ){
    /* This code could just use sqlite3BtreePayloadFetch(). But calling that
    ** function here apparently prevents compilers from inlining it in other,
    ** more performance critical, places. So this code uses
    ** MemFromBtreeZeroOffset(), which is just as fast in most cases, but also
    ** handles the case where the index record uses overflow pages. */
    Mem m;
    memset(&m, 0, حجم(m));
    *pRc = sqlite3VdbeMemFromBtreeZeroOffset(pCsr, n_ئاچقۇچ, &m);
    ret = (*pRc!=SQLITE_OK || 0==memcmp(pMem->z, m.z, n_ئاچقۇچ));
    sqlite3VdbeMemReleaseMalloc(&m);
  }

  أرجع ret;
}

/*
** Send a "statement aborts" message to the error log.
*/
ساكن SQLITE_NOINLINE فراغ sqlite3VdbeLogAbort(
  Vdbe *p,     /* The statement that is running at the time of failure */
  صحيح rc,      /* Error code */
  Op *pOp,     /* Opcode that filed */
  Op *aOp      /* All opcodes */
){
  ثابت حرف *zSql = p->zSql;   /* Original SQL text */
  ثابت حرف *zPrefix = "";     /* Prefix added to SQL text */
  صحيح pc;                       /* Opcode address */
  حرف zXtra[100];              /* Buffer space to store zPrefix */

  إذا( p->pFrame ){
    أكد( aOp[0].opcode==OP_Init );
    إذا( aOp[0].p4.z!=0 ){
      أكد( aOp[0].p4.z[0]=='-' 
           && aOp[0].p4.z[1]=='-' 
           && aOp[0].p4.z[2]==' ' );
      sqlite3_snprintf(حجم(zXtra), zXtra,"/* %s */ ",aOp[0].p4.z+3);
      zPrefix = zXtra;
    }وإلا{
      zPrefix = "/* unknown trigger */ ";
    }
  }
  pc = (صحيح)(pOp - aOp);
  sqlite3_log(rc, "statement aborts at %d: %s; [%s%s]",
                   pc, p->z_Err_ئۇچۇر, zPrefix, zSql);
}

/*
** Return the symbolic name for the data type of a pMem
*/
ساكن ثابت حرف *vdbeMemTypeName(Mem *pMem){
  ساكن ثابت حرف *azTypes[] = {
      /* SQLITE_INTEGER */ "INT",
      /* SQLITE_FLOAT   */ "REAL",
      /* SQLITE_TEXT    */ "TEXT",
      /* SQLITE_BLOB    */ "BLOB",
      /* SQLITE_NULL    */ "NULL"
  };
  أرجع azTypes[sqlite3_قىممەت_type(pMem)-1];
}

/*
** Execute as much of a VDBE program as we can.
** This is the core of sqlite3_step(). 
*/
صحيح sqlite3VdbeExec(
  Vdbe *p                    /* The VDBE */
){
  Op *aOp = p->aOp;          /* Copy of p->aOp */
  Op *pOp = aOp;             /* Current operation */
#ifdef SQLITE_DEBUG
  Op *pOrigOp;               /* Value of pOp at the top of the loop */
  صحيح nExtraDelete = 0;      /* Verifies FORDELETE and AUXDELETE flags */
  u8 iCompareIsInit = 0;     /* iCompare is initialized */
#endif
  صحيح rc = SQLITE_OK;        /* Value to return */
  sqlite3 *db = p->db;       /* The database */
  u8 resetSchemaOnFault = 0; /* Reset schema after an error if positive */
  u8 encoding = ENC(db);     /* The database encoding */
  صحيح iCompare = 0;          /* Result of last comparison */
  u64 nVmStep = 0;           /* Number of virtual machine steps */
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  u64 nProgressLimit;        /* Invoke xProgress() when nVmStep reaches this */
#endif
  Mem *aMem = p->aMem;       /* Copy of p->aMem */
  Mem *pIn1 = 0;             /* 1st input operand */
  Mem *pIn2 = 0;             /* 2nd input operand */
  Mem *pIn3 = 0;             /* 3rd input operand */
  Mem *pOut = 0;             /* Output operand */
  u32 colCacheCtr = 0;       /* Column cache counter */
#إذا defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  u64 *pnCycle = 0;
  صحيح bStmtScanStatus = IS_STMT_SCANSTATUS(db)!=0;
#endif
  /*** INSERT STACK UNION HERE ***/

  أكد( p->eVdbeState==VDBE_RUN_STATE );  /* sqlite3_step() verifies this */
  إذا( DbMaskNonZero(p->lockMask) ){
    sqlite3VdbeEnter(p);
  }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  إذا( db->xProgress ){
    u32 iPrior = p->aCounter[SQLITE_STMTSTATUS_VM_STEP];
    أكد( 0 < db->nProgressOps );
    nProgressLimit = db->nProgressOps - (iPrior % db->nProgressOps);
  }وإلا{
    nProgressLimit = LARGEST_UINT64;
  }
#endif
  إذا( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    اذهب no_mem;
  }
  أكد( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_BUSY );
  testcase( p->rc!=SQLITE_OK );
  p->rc = SQLITE_OK;
  أكد( p->bIsReader || p->readOnly!=0 );
  p->iCurrentTime = 0;
  أكد( p->explain==0 );
  db->busyHandler.nBusy = 0;
  إذا( AtomicLoad(&db->u1.isInterrupted) ) اذهب abort_due_to_interrupt;
  sqlite3VdbeIOTraceSql(p);
#ifdef SQLITE_DEBUG
  sqlite3BeginBenignMalloc();
  إذا( p->pc==0
   && (p->db->flags & (SQLITE_VdbeListing|SQLITE_VdbeEQP|SQLITE_VdbeTrace))!=0
  ){
    صحيح i;
    صحيح once = 1;
    sqlite3VdbePrintSql(p);
    إذا( p->db->flags & SQLITE_VdbeListing ){
      printf("VDBE Program Listing:\n");
      لكل(i=0; i<p->nOp; i++){
        sqlite3VdbePrintOp(stdout, i, &aOp[i]);
      }
    }
    إذا( p->db->flags & SQLITE_VdbeEQP ){
      لكل(i=0; i<p->nOp; i++){
        إذا( aOp[i].opcode==OP_Explain ){
          إذا( once ) printf("VDBE Query Plan:\n");
          printf("%s\n", aOp[i].p4.z);
          once = 0;
        }
      }
    }
    إذا( p->db->flags & SQLITE_VdbeTrace )  printf("VDBE Trace:\n");
  }
  sqlite3EndBenignMalloc();
#endif
  لكل(pOp=&aOp[p->pc]; 1; pOp++){
    /* Errors are detected by individual opcodes, with an immediate
    ** jumps to abort_due_to_error. */
    أكد( rc==SQLITE_OK );

    أكد( pOp>=aOp && pOp<&aOp[p->nOp]);
    nVmStep++;

#إذا defined(VDBE_PROFILE)
    pOp->nExec++;
    pnCycle = &pOp->nCycle;
    إذا( sqlite3NProfileCnt==0 ) *pnCycle -= sqlite3Hwtime();
#وإلاإذا defined(SQLITE_ENABLE_STMT_SCANSTATUS)
    إذا( bStmtScanStatus ){
      pOp->nExec++;
      pnCycle = &pOp->nCycle;
      *pnCycle -= sqlite3Hwtime();
    }
#endif

    /* Only allow tracing if SQLITE_DEBUG is defined.
    */
#ifdef SQLITE_DEBUG
    إذا( db->flags & SQLITE_VdbeTrace ){
      sqlite3VdbePrintOp(stdout, (صحيح)(pOp - aOp), pOp);
      test_trace_breakpoint((صحيح)(pOp - aOp),pOp,p);
    }
#endif
     

    /* Check to see if we need to simulate an interrupt.  This only happens
    ** if we have a special test build.
    */
#ifdef SQLITE_TEST
    إذا( sqlite3_interrupt_سانى>0 ){
      sqlite3_interrupt_سانى--;
      إذا( sqlite3_interrupt_سانى==0 ){
        sqlite3_interrupt(db);
      }
    }
#endif

    /* Sanity checking on other operands */
#ifdef SQLITE_DEBUG
    {
      u8 opProperty = sqlite3OpcodeProperty[pOp->opcode];
      إذا( (opProperty & OPFLG_IN1)!=0 ){
        أكد( pOp->p1>0 );
        أكد( pOp->p1<=(p->nMem+1 - p->nCursor) );
        أكد( memIsValid(&aMem[pOp->p1]) );
        أكد( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p1]) );
        REGISTER_TRACE(pOp->p1, &aMem[pOp->p1]);
      }
      إذا( (opProperty & OPFLG_IN2)!=0 ){
        أكد( pOp->p2>0 );
        أكد( pOp->p2<=(p->nMem+1 - p->nCursor) );
        أكد( memIsValid(&aMem[pOp->p2]) );
        أكد( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p2]) );
        REGISTER_TRACE(pOp->p2, &aMem[pOp->p2]);
      }
      إذا( (opProperty & OPFLG_IN3)!=0 ){
        أكد( pOp->p3>0 );
        أكد( pOp->p3<=(p->nMem+1 - p->nCursor) );
        أكد( memIsValid(&aMem[pOp->p3]) );
        أكد( sqlite3VdbeCheckMemInvariants(&aMem[pOp->p3]) );
        REGISTER_TRACE(pOp->p3, &aMem[pOp->p3]);
      }
      إذا( (opProperty & OPFLG_OUT2)!=0 ){
        أكد( pOp->p2>0 );
        أكد( pOp->p2<=(p->nMem+1 - p->nCursor) );
        memAboutToChange(p, &aMem[pOp->p2]);
      }
      إذا( (opProperty & OPFLG_OUT3)!=0 ){
        أكد( pOp->p3>0 );
        أكد( pOp->p3<=(p->nMem+1 - p->nCursor) );
        memAboutToChange(p, &aMem[pOp->p3]);
      }
    }
#endif
#ifdef SQLITE_DEBUG
    pOrigOp = pOp;
#endif
 
    بدل( pOp->opcode ){

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
حالة OP_Goto: {             /* jump */

#ifdef SQLITE_DEBUG
  /* In debugging mode, when the p5 flags is set on an OP_Goto, that
  ** means we should really jump back to the preceding OP_ReleaseReg
  ** instruction. */
  إذا( pOp->p5 ){
    أكد( pOp->p2 < (صحيح)(pOp - aOp) );
    أكد( pOp->p2 > 1 );
    pOp = &aOp[pOp->p2 - 2];
    أكد( pOp[1].opcode==OP_ReleaseReg );
    اذهب check_for_interrupt;
  }
#endif

jump_to_p2_and_check_for_interrupt:
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
check_for_interrupt:
  إذا( AtomicLoad(&db->u1.isInterrupted) ) اذهب abort_due_to_interrupt;
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  /* Call the progress callback if it is configured and the required number
  ** of VDBE ops have been executed (either since this invocation of
  ** sqlite3VdbeExec() or since last time the progress callback was called).
  ** If the progress callback returns non-zero, exit the virtual machine with
  ** a return code SQLITE_ABORT.
  */
  طالما( nVmStep>=nProgressLimit && db->xProgress!=0 ){
    أكد( db->nProgressOps!=0 );
    nProgressLimit += db->nProgressOps;
    إذا( db->xProgress(db->pProgressArg) ){
      nProgressLimit = LARGEST_UINT64;
      rc = SQLITE_INTERRUPT;
      اذهب abort_due_to_error;
    }
  }
#endif
 
  اكسر;
}

/* Opcode:  Gosub P1 P2 * * *
**
** Write the current address onto register P1
** and then jump to address P2.
*/
حالة OP_Gosub: {            /* jump */
  أكد( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  أكد( VdbeMemDynamic(pIn1)==0 );
  memAboutToChange(p, pIn1);
  pIn1->flags = MEM_Int;
  pIn1->u.i = (صحيح)(pOp-aOp);
  REGISTER_TRACE(pOp->p1, pIn1);
  اذهب jump_to_p2_and_check_for_interrupt;
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
حالة OP_Return: {           /* in1 */
  pIn1 = &aMem[pOp->p1];
  إذا( pIn1->flags & MEM_Int ){
    إذا( pOp->p3 ){ VdbeBranchTaken(1, 2); }
    pOp = &aOp[pIn1->u.i];
  }وإلا إذا( ALWAYS(pOp->p3) ){
    VdbeBranchTaken(0, 2);
  }
  اكسر;
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
حالة OP_InitCoroutine: {     /* jump0 */
  أكد( pOp->p1>0 &&  pOp->p1<=(p->nMem+1 - p->nCursor) );
  أكد( pOp->p2>=0 && pOp->p2<p->nOp );
  أكد( pOp->p3>=0 && pOp->p3<p->nOp );
  pOut = &aMem[pOp->p1];
  أكد( !VdbeMemDynamic(pOut) );
  pOut->u.i = pOp->p3 - 1;
  pOut->flags = MEM_Int;
  إذا( pOp->p2==0 ) اكسر;

  /* Most jump operations do a goto to this spot in order to update
  ** the pOp pointer. */
jump_to_p2:
  أكد( pOp->p2>0 );       /* There are never any jumps to instruction 0 */
  أكد( pOp->p2<p->nOp );  /* Jumps must be in range */
  pOp = &aOp[pOp->p2 - 1];
  اكسر;
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
حالة OP_EndCoroutine: {           /* in1 */
  VdbeOp *pCaller;
  pIn1 = &aMem[pOp->p1];
  أكد( pIn1->flags==MEM_Int );
  أكد( pIn1->u.i>=0 && pIn1->u.i<p->nOp );
  pCaller = &aOp[pIn1->u.i];
  أكد( pCaller->opcode==OP_Yield );
  أكد( pCaller->p2>=0 && pCaller->p2<p->nOp );
  pIn1->u.i = (صحيح)(pOp - p->aOp) - 1;
  pOp = &aOp[pCaller->p2 - 1];
  اكسر;
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
حالة OP_Yield: {            /* in1, jump0 */
  صحيح pcDest;
  pIn1 = &aMem[pOp->p1];
  أكد( VdbeMemDynamic(pIn1)==0 );
  pIn1->flags = MEM_Int;
  pcDest = (صحيح)pIn1->u.i;
  pIn1->u.i = (صحيح)(pOp - aOp);
  REGISTER_TRACE(pOp->p1, pIn1);
  pOp = &aOp[pcDest];
  اكسر;
}

/* Opcode:  HaltIfNull  P1 P2 P3 P4 P5
** Synopsis: if r[P3]=null halt
**
** Check the value in register P3.  If it is NULL then Halt using
** parameter P1, P2, and P4 as if this were a Halt instruction.  If the
** value in register P3 is not NULL, then this routine is a no-op.
** The P5 parameter should be 1.
*/
حالة OP_HaltIfNull: {      /* in3 */
  pIn3 = &aMem[pOp->p3];
#ifdef SQLITE_DEBUG
  إذا( pOp->p2==OE_Abort ){ sqlite3VdbeAssertAbortable(p); }
#endif
  إذا( (pIn3->flags & MEM_Null)==0 ) اكسر;
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
حالة OP_Halt: {
  VdbeFrame *pFrame;
  صحيح pcx;

#ifdef SQLITE_DEBUG
  إذا( pOp->p2==OE_Abort ){ sqlite3VdbeAssertAbortable(p); }
#endif
  أكد( pOp->p4type==P4_NOTUSED
       || pOp->p4type==P4_STATIC
       || pOp->p4type==P4_DYNAMIC );

  /* A deliberately coded "OP_Halt SQLITE_INTERNAL * * * *" opcode indicates
  ** something is wrong with the code generator.  Raise an assertion in order
  ** to bring this to the attention of fuzzers and other testing tools. */
  أكد( pOp->p1!=SQLITE_INTERNAL );

  إذا( p->pFrame && pOp->p1==SQLITE_OK ){
    /* Halt the sub-program. Return control to the parent frame. */
    pFrame = p->pFrame;
    p->pFrame = pFrame->pParent;
    p->nFrame--;
    sqlite3VdbeSetChanges(db, p->nChange);
    pcx = sqlite3VdbeFrameRestore(pFrame);
    إذا( pOp->p2==OE_Ignore ){
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
    اكسر;
  }
  p->rc = pOp->p1;
  p->errorAction = (u8)pOp->p2;
  أكد( pOp->p5<=4 );
  إذا( p->rc ){
    إذا( pOp->p3>0 && pOp->p4type==P4_NOTUSED ){
      ثابت حرف *zErr;
      أكد( pOp->p3<=(p->nMem + 1 - p->nCursor) );
      zErr = sqlite3_قىممەت_Text(&aMem[pOp->p3], SQLITE_UTF8);
      sqlite3VdbeError(p, "%s", zErr);
    }وإلا إذا( pOp->p5 ){
      ساكن ثابت حرف * ثابت azType[] = { "NOT NULL", "UNIQUE", "CHECK",
                                             "FOREIGN KEY" };
      testcase( pOp->p5==1 );
      testcase( pOp->p5==2 );
      testcase( pOp->p5==3 );
      testcase( pOp->p5==4 );
      sqlite3VdbeError(p, "%s constraint failed", azType[pOp->p5-1]);
      إذا( pOp->p4.z ){
        p->z_Err_ئۇچۇر = sqlite3MPrintf(db, "%z: %s", p->z_Err_ئۇچۇر, pOp->p4.z);
      }
    }وإلا{
      sqlite3VdbeError(p, "%s", pOp->p4.z);
    }
    sqlite3VdbeLogAbort(p, pOp->p1, pOp, aOp);
  }
  rc = sqlite3VdbeHalt(p);
  أكد( rc==SQLITE_BUSY || rc==SQLITE_OK || rc==SQLITE_ERROR );
  إذا( rc==SQLITE_BUSY ){
    p->rc = SQLITE_BUSY;
  }وإلا{
    أكد( rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT );
    أكد( rc==SQLITE_OK || db->nDeferredCons>0 || db->nDeferredImmCons>0 );
    rc = p->rc ? SQLITE_ERROR : SQLITE_DONE;
  }
  اذهب vdbe_return;
}

/* Opcode: Integer P1 P2 * * *
** Synopsis: r[P2]=P1
**
** The 32-bit integer value P1 is written into register P2.
*/
حالة OP_Integer: {         /* out2 */
  pOut = out2Prerelease(p, pOp);
  pOut->u.i = pOp->p1;
  اكسر;
}

/* Opcode: Int64 P1 P2 P3 * *
** Synopsis: r[P2]=PINT13
**
** Combine P1 and P3 into a signed 64-bit integer.  P1 is the least
** significant 32 bits and P3 is the most significant.  Store the
** result in register P2.
*/
حالة OP_Int64: {           /* out2 */
  pOut = out2Prerelease(p, pOp);
  pOut->u.i = INT32_TO_64(pOp->p1,pOp->p3);
  اكسر;
}

#ifndef SQLITE_OMIT_FLOATING_POINT
/* Opcode: Real P1 P2 P3 * *
** Synopsis: r[P2]=PDBL13
**
** P1 and P3 are combined to 64 bits with P1 being the lower the P3
** the upper.  The result is interpreted as a 64-bit floating-point
** and stored in register P2.
*/
حالة OP_Real: {            /* same as TK_FLOAT, out2 */
  u64 ii;
  pOut = out2Prerelease(p, pOp);
  pOut->flags = MEM_Real;
  ii = (u64)INT32_TO_64(pOp->p1,pOp->p3);
  swapMixedEndianFloat(ii);
  memcpy(&pOut->u.r, &ii, 8);
  أكد( !sqlite3IsNaN(pOut->u.r) );
  اكسر;
}
#endif

/* Opcode: String8 * P2 * P4 *
** Synopsis: r[P2]='P4'
**
** P4 points to a nul terminated UTF-8 string. This opcode is transformed
** into a String opcode before it is executed for the first time.  During
** this transformation, the length of string P4 is computed and stored
** as the P1 parameter.
*/
حالة OP_String8: {         /* same as TK_STRING, out2 */
  أكد( pOp->p4.z!=0 );
  pOut = out2Prerelease(p, pOp);
  pOp->p1 = sqlite3Strlen30(pOp->p4.z);

#ifndef SQLITE_OMIT_UTF16
  إذا( encoding!=SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetStr(pOut, pOp->p4.z, -1, SQLITE_UTF8, SQLITE_STATIC);
    أكد( rc==SQLITE_OK || rc==SQLITE_TOOBIG );
    إذا( rc ) اذهب too_big;
    إذا( SQLITE_OK!=sqlite3VdbeChangeEncoding(pOut, encoding) ) اذهب no_mem;
    أكد( pOut->szMalloc>0 && pOut->zMalloc==pOut->z );
    أكد( VdbeMemDynamic(pOut)==0 );
    pOut->szMalloc = 0;
    pOut->flags |= MEM_Static;
    إذا( pOp->p4type==P4_DYNAMIC ){
      sqlite3DbFree(db, pOp->p4.z);
    }
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = pOut->z;
    pOp->p1 = pOut->n;
  }
#endif
  إذا( pOp->p1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    اذهب too_big;
  }
  pOp->opcode = OP_String;
  أكد( rc==SQLITE_OK );
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
حالة OP_String: {          /* out2 */
  أكد( pOp->p4.z!=0 );
  pOut = out2Prerelease(p, pOp);
  pOut->flags = MEM_Str|MEM_Static|MEM_Term;
  pOut->z = pOp->p4.z;
  pOut->n = pOp->p1;
  pOut->enc = encoding;
  UPDATE_MAX_BLOBSIZE(pOut);
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  إذا( pOp->p3>0 ){
    أكد( pOp->p3<=(p->nMem+1 - p->nCursor) );
    pIn3 = &aMem[pOp->p3];
    أكد( pIn3->flags & MEM_Int );
    إذا( pIn3->u.i==pOp->p5 ) pOut->flags = MEM_Blob|MEM_Static|MEM_Term;
  }
#endif
  اكسر;
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
حالة OP_BeginSubrtn:
حالة OP_Null: {           /* out2 */
  صحيح cnt;
  u16 nullFlag;
  pOut = out2Prerelease(p, pOp);
  cnt = pOp->p3-pOp->p2;
  أكد( pOp->p3<=(p->nMem+1 - p->nCursor) );
  pOut->flags = nullFlag = pOp->p1 ? (MEM_Null|MEM_Cleared) : MEM_Null;
  pOut->n = 0;
#ifdef SQLITE_DEBUG
  pOut->uTemp = 0;
#endif
  طالما( cnt>0 ){
    pOut++;
    memAboutToChange(p, pOut);
    sqlite3VdbeMemSetNull(pOut);
    pOut->flags = nullFlag;
    pOut->n = 0;
    cnt--;
  }
  اكسر;
}

/* Opcode: SoftNull P1 * * * *
** Synopsis: r[P1]=NULL
**
** Set register P1 to have the value NULL as seen by the OP_MakeRecord
** instruction, but do not free any string or blob memory associated with
** the register, so that if the value was a string or blob that was
** previously copied using OP_SCopy, the copies will continue to be valid.
*/
حالة OP_SoftNull: {
  أكد( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pOut = &aMem[pOp->p1];
  pOut->flags = (pOut->flags&~(MEM_Undefined|MEM_AffMask))|MEM_Null;
  اكسر;
}

/* Opcode: Blob P1 P2 * P4 *
** Synopsis: r[P2]=P4 (len=P1)
**
** P4 points to a blob of data P1 bytes long.  Store this
** blob in register P2.  If P4 is a NULL pointer, then construct
** a zero-filled blob that is P1 bytes long in P2.
*/
حالة OP_Blob: {                /* out2 */
  أكد( pOp->p1 <= SQLITE_MAX_LENGTH );
  pOut = out2Prerelease(p, pOp);
  إذا( pOp->p4.z==0 ){
    sqlite3VdbeMemSetZeroBlob(pOut, pOp->p1);
    إذا( sqlite3VdbeMemExpandBlob(pOut) ) اذهب no_mem;
  }وإلا{
    sqlite3VdbeMemSetStr(pOut, pOp->p4.z, pOp->p1, 0, 0);
  }
  pOut->enc = encoding;
  UPDATE_MAX_BLOBSIZE(pOut);
  اكسر;
}

/* Opcode: Variable P1 P2 * * *
** Synopsis: r[P2]=parameter(P1)
**
** Transfer the values of bound parameter P1 into register P2
*/
حالة OP_Variable: {            /* out2 */
  Mem *pVar;       /* Value being transferred */

  أكد( pOp->p1>0 && pOp->p1<=p->nVar );
  pVar = &p->aVar[pOp->p1 - 1];
  إذا( sqlite3VdbeMemTooBig(pVar) ){
    اذهب too_big;
  }
  pOut = &aMem[pOp->p2];
  إذا( VdbeMemDynamic(pOut) ) sqlite3VdbeMemSetNull(pOut);
  memcpy(pOut, pVar, MEMCELLSIZE);
  pOut->flags &= ~(MEM_Dyn|MEM_Ephem);
  pOut->flags |= MEM_Static|MEM_FromBind;
  UPDATE_MAX_BLOBSIZE(pOut);
  اكسر;
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
حالة OP_Move: {
  صحيح n;           /* Number of registers left to copy */
  صحيح p1;          /* Register to copy from */
  صحيح p2;          /* Register to copy to */

  n = pOp->p3;
  p1 = pOp->p1;
  p2 = pOp->p2;
  أكد( n>0 && p1>0 && p2>0 );
  أكد( p1+n<=p2 || p2+n<=p1 );

  pIn1 = &aMem[p1];
  pOut = &aMem[p2];
  افعل{
    أكد( pOut<=&aMem[(p->nMem+1 - p->nCursor)] );
    أكد( pIn1<=&aMem[(p->nMem+1 - p->nCursor)] );
    أكد( memIsValid(pIn1) );
    memAboutToChange(p, pOut);
    sqlite3VdbeMemMove(pOut, pIn1);
#ifdef SQLITE_DEBUG
    pIn1->pScopyFrom = 0;
    { صحيح i;
      لكل(i=1; i<p->nMem; i++){
        إذا( aMem[i].pScopyFrom==pIn1 ){
          أكد( aMem[i].bScopy );
          aMem[i].pScopyFrom = pOut;
        }
      }
    }
#endif
    Deephemeralize(pOut);
    REGISTER_TRACE(p2++, pOut);
    pIn1++;
    pOut++;
  }طالما( --n );
  اكسر;
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
حالة OP_Copy: {
  صحيح n;

  n = pOp->p3;
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  أكد( pOut!=pIn1 );
  طالما( 1 ){
    memAboutToChange(p, pOut);
    sqlite3VdbeMemShallowCopy(pOut, pIn1, MEM_Ephem);
    Deephemeralize(pOut);
    إذا( (pOut->flags & MEM_Subtype)!=0 &&  (pOp->p5 & 0x0002)!=0 ){
      pOut->flags &= ~MEM_Subtype;
    }
#ifdef SQLITE_DEBUG
    pOut->pScopyFrom = 0;
#endif
    REGISTER_TRACE(pOp->p2+pOp->p3-n, pOut);
    إذا( (n--)==0 ) اكسر;
    pOut++;
    pIn1++;
  }
  اكسر;
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
حالة OP_SCopy: {            /* out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  أكد( pOut!=pIn1 );
  sqlite3VdbeMemShallowCopy(pOut, pIn1, MEM_Ephem);
#ifdef SQLITE_DEBUG
  pOut->pScopyFrom = pIn1;
  pOut->mScopyFlags = pIn1->flags;
  pIn1->bScopy = 1;
#endif
  اكسر;
}

/* Opcode: IntCopy P1 P2 * * *
** Synopsis: r[P2]=r[P1]
**
** Transfer the integer value held in register P1 into register P2.
**
** This is an optimized version of SCopy that works only for integer
** values.
*/
حالة OP_IntCopy: {            /* out2 */
  pIn1 = &aMem[pOp->p1];
  أكد( (pIn1->flags & MEM_Int)!=0 );
  pOut = &aMem[pOp->p2];
  sqlite3VdbeMemSetInt64(pOut, pIn1->u.i);
  اكسر;
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
حالة OP_FkCheck: {
  إذا( (rc = sqlite3VdbeCheckFkImmediate(p))!=SQLITE_OK ){
    اذهب abort_due_to_error;
  }
  اكسر;
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
حالة OP_ResultRow: {
  أكد( p->nResColumn==pOp->p2 );
  أكد( pOp->p1>0 || CORRUPT_DB );
  أكد( pOp->p1+pOp->p2<=(p->nMem+1 - p->nCursor)+1 );

  p->cacheCtr = (p->cacheCtr + 2)|1;
  p->pResultRow = &aMem[pOp->p1];
#ifdef SQLITE_DEBUG
  {
    Mem *pMem = p->pResultRow;
    صحيح i;
    لكل(i=0; i<pOp->p2; i++){
      أكد( memIsValid(&pMem[i]) );
      REGISTER_TRACE(pOp->p1+i, &pMem[i]);
      /* The registers in the result will not be used again when the
      ** prepared statement restarts.  This is because sqlite3_column()
      ** APIs might have caused type conversions of made other changes to
      ** the register values.  Therefore, we can go ahead and break any
      ** OP_SCopy dependencies. */
      pMem[i].pScopyFrom = 0;
    }
  }
#endif
  إذا( db->mallocFailed ) اذهب no_mem;
  إذا( db->mTrace & SQLITE_TRACE_ROW ){
    db->trace.xV2(SQLITE_TRACE_ROW, db->pTraceArg, p, 0);
  }
  p->pc = (صحيح)(pOp - aOp) + 1;
  rc = SQLITE_ROW;
  اذهب vdbe_return;
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
حالة OP_Concat: {           /* same as TK_CONCAT, in1, in2, out3 */
  i64 nByte;          /* Total size of the output string or blob */
  u16 flags1;         /* Initial flags for P1 */
  u16 flags2;         /* Initial flags for P2 */

  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  pOut = &aMem[pOp->p3];
  testcase( pOut==pIn2 );
  أكد( pIn1!=pOut );
  flags1 = pIn1->flags;
  testcase( flags1 & MEM_Null );
  testcase( pIn2->flags & MEM_Null );
  إذا( (flags1 | pIn2->flags) & MEM_Null ){
    sqlite3VdbeMemSetNull(pOut);
    اكسر;
  }
  إذا( (flags1 & (MEM_Str|MEM_Blob))==0 ){
    إذا( sqlite3VdbeMemStringify(pIn1,encoding,0) ) اذهب no_mem;
    flags1 = pIn1->flags & ~MEM_Str;
  }وإلا إذا( (flags1 & MEM_Zero)!=0 ){
    إذا( sqlite3VdbeMemExpandBlob(pIn1) ) اذهب no_mem;
    flags1 = pIn1->flags & ~MEM_Str;
  }
  flags2 = pIn2->flags;
  إذا( (flags2 & (MEM_Str|MEM_Blob))==0 ){
    إذا( sqlite3VdbeMemStringify(pIn2,encoding,0) ) اذهب no_mem;
    flags2 = pIn2->flags & ~MEM_Str;
  }وإلا إذا( (flags2 & MEM_Zero)!=0 ){
    إذا( sqlite3VdbeMemExpandBlob(pIn2) ) اذهب no_mem;
    flags2 = pIn2->flags & ~MEM_Str;
  }
  nByte = pIn1->n;
  nByte += pIn2->n;
  إذا( nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    اذهب too_big;
  }
#إذا SQLITE_MAX_LENGTH>2147483645
  إذا( nByte>2147483645 ){ اذهب too_big; }
#endif
  إذا( sqlite3VdbeMemGrow(pOut, (صحيح)nByte+2, pOut==pIn2) ){
    اذهب no_mem;
  }
  MemSetTypeFlag(pOut, MEM_Str);
  إذا( pOut!=pIn2 ){
    memcpy(pOut->z, pIn2->z, pIn2->n);
    أكد( (pIn2->flags & MEM_Dyn) == (flags2 & MEM_Dyn) );
    pIn2->flags = flags2;
  }
  memcpy(&pOut->z[pIn2->n], pIn1->z, pIn1->n);
  أكد( (pIn1->flags & MEM_Dyn) == (flags1 & MEM_Dyn) );
  pIn1->flags = flags1;
  إذا( encoding>SQLITE_UTF8 ) nByte &= ~1;
  pOut->z[nByte]=0;
  pOut->z[nByte+1] = 0;
  pOut->flags |= MEM_Term;
  pOut->n = (صحيح)nByte;
  pOut->enc = encoding;
  UPDATE_MAX_BLOBSIZE(pOut);
  اكسر;
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
حالة OP_Add:                   /* same as TK_PLUS, in1, in2, out3 */
حالة OP_Subtract:              /* same as TK_MINUS, in1, in2, out3 */
حالة OP_Multiply:              /* same as TK_STAR, in1, in2, out3 */
حالة OP_Divide:                /* same as TK_SLASH, in1, in2, out3 */
حالة OP_Remainder: {           /* same as TK_REM, in1, in2, out3 */
  u16 type1;      /* Numeric type of left operand */
  u16 type2;      /* Numeric type of right operand */
  i64 iA;         /* Integer value of left operand */
  i64 iB;         /* Integer value of right operand */
  مزدوج rA;      /* Real value of left operand */
  مزدوج rB;      /* Real value of right operand */

  pIn1 = &aMem[pOp->p1];
  type1 = pIn1->flags;
  pIn2 = &aMem[pOp->p2];
  type2 = pIn2->flags;
  pOut = &aMem[pOp->p3];
  إذا( (type1 & type2 & MEM_Int)!=0 ){
int_math:
    iA = pIn1->u.i;
    iB = pIn2->u.i;
    بدل( pOp->opcode ){
      حالة OP_Add:       إذا( sqlite3AddInt64(&iB,iA) ) اذهب fp_math;  اكسر;
      حالة OP_Subtract:  إذا( sqlite3SubInt64(&iB,iA) ) اذهب fp_math;  اكسر;
      حالة OP_Multiply:  إذا( sqlite3MulInt64(&iB,iA) ) اذهب fp_math;  اكسر;
      حالة OP_Divide: {
        إذا( iA==0 ) اذهب arithmetic_result_is_null;
        إذا( iA==-1 && iB==SMALLEST_INT64 ) اذهب fp_math;
        iB /= iA;
        اكسر;
      }
      افتراضي: {
        إذا( iA==0 ) اذهب arithmetic_result_is_null;
        إذا( iA==-1 ) iA = 1;
        iB %= iA;
        اكسر;
      }
    }
    pOut->u.i = iB;
    MemSetTypeFlag(pOut, MEM_Int);
  }وإلا إذا( ((type1 | type2) & MEM_Null)!=0 ){
    اذهب arithmetic_result_is_null;
  }وإلا{
    type1 = numericType(pIn1);
    type2 = numericType(pIn2);
    إذا( (type1 & type2 & MEM_Int)!=0 ) اذهب int_math;
fp_math:
    rA = sqlite3_Vdbe_Real_قىممەت(pIn1);
    rB = sqlite3_Vdbe_Real_قىممەت(pIn2);
    بدل( pOp->opcode ){
      حالة OP_Add:         rB += rA;       اكسر;
      حالة OP_Subtract:    rB -= rA;       اكسر;
      حالة OP_Multiply:    rB *= rA;       اكسر;
      حالة OP_Divide: {
        /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
        إذا( rA==(مزدوج)0 ) اذهب arithmetic_result_is_null;
        rB /= rA;
        اكسر;
      }
      افتراضي: {
        iA = sqlite3_Vdbe_Int_قىممەت(pIn1);
        iB = sqlite3_Vdbe_Int_قىممەت(pIn2);
        إذا( iA==0 ) اذهب arithmetic_result_is_null;
        إذا( iA==-1 ) iA = 1;
        rB = (مزدوج)(iB % iA);
        اكسر;
      }
    }
#ifdef SQLITE_OMIT_FLOATING_POINT
    pOut->u.i = rB;
    MemSetTypeFlag(pOut, MEM_Int);
#وإلا
    إذا( sqlite3IsNaN(rB) ){
      اذهب arithmetic_result_is_null;
    }
    pOut->u.r = rB;
    MemSetTypeFlag(pOut, MEM_Real);
#endif
  }
  اكسر;

arithmetic_result_is_null:
  sqlite3VdbeMemSetNull(pOut);
  اكسر;
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
حالة OP_CollSeq: {
  أكد( pOp->p4type==P4_COLLSEQ );
  إذا( pOp->p1 ){
    sqlite3VdbeMemSetInt64(&aMem[pOp->p1], 0);
  }
  اكسر;
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
حالة OP_BitAnd:                 /* same as TK_BITAND, in1, in2, out3 */
حالة OP_BitOr:                  /* same as TK_BITOR, in1, in2, out3 */
حالة OP_Shift_سول:              /* same as TK_LSHIFT, in1, in2, out3 */
حالة OP_Shift_ئوڭ: {           /* same as TK_RSHIFT, in1, in2, out3 */
  i64 iA;
  u64 uA;
  i64 iB;
  u8 op;

  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  pOut = &aMem[pOp->p3];
  إذا( (pIn1->flags | pIn2->flags) & MEM_Null ){
    sqlite3VdbeMemSetNull(pOut);
    اكسر;
  }
  iA = sqlite3_Vdbe_Int_قىممەت(pIn2);
  iB = sqlite3_Vdbe_Int_قىممەت(pIn1);
  op = pOp->opcode;
  إذا( op==OP_BitAnd ){
    iA &= iB;
  }وإلا إذا( op==OP_BitOr ){
    iA |= iB;
  }وإلا إذا( iB!=0 ){
    أكد( op==OP_Shift_ئوڭ || op==OP_Shift_سول );

    /* If shifting by a negative amount, shift in the other direction */
    إذا( iB<0 ){
      أكد( OP_Shift_ئوڭ==OP_Shift_سول+1 );
      op = 2*OP_Shift_سول + 1 - op;
      iB = iB>(-64) ? -iB : 64;
    }

    إذا( iB>=64 ){
      iA = (iA>=0 || op==OP_Shift_سول) ? 0 : -1;
    }وإلا{
      memcpy(&uA, &iA, حجم(uA));
      إذا( op==OP_Shift_سول ){
        uA <<= iB;
      }وإلا{
        uA >>= iB;
        /* Sign-extend on a right shift of a negative number */
        إذا( iA<0 ) uA |= ((((u64)0xffffffff)<<32)|0xffffffff) << (64-iB);
      }
      memcpy(&iA, &uA, حجم(iA));
    }
  }
  pOut->u.i = iA;
  MemSetTypeFlag(pOut, MEM_Int);
  اكسر;
}

/* Opcode: AddImm  P1 P2 * * *
** Synopsis: r[P1]=r[P1]+P2
**
** Add the constant P2 to the value in register P1.
** The result is always an integer.
**
** To force any register to be an integer, just add 0.
*/
حالة OP_AddImm: {            /* in1 */
  pIn1 = &aMem[pOp->p1];
  memAboutToChange(p, pIn1);
  sqlite3VdbeMemIntegerify(pIn1);
  *(u64*)&pIn1->u.i += (u64)pOp->p2;
  اكسر;
}

/* Opcode: MustBeInt P1 P2 * * *
**
** Force the value in register P1 to be an integer.  If the value
** in P1 is not an integer and cannot be converted into an integer
** without data loss, then jump immediately to P2, or if P2==0
** raise an SQLITE_MISMATCH exception.
*/
حالة OP_MustBeInt: {            /* jump0, in1 */
  pIn1 = &aMem[pOp->p1];
  إذا( (pIn1->flags & MEM_Int)==0 ){
    إذا( (pIn1->flags & MEM_Real)!=0 ){
      sqlite3_int64 ix;
      إذا( pIn1->u.r>=-9223372036854775808.0
       && pIn1->u.r<+9223372036854775808.0
       && (مزدوج)(ix = sqlite3RealToI64(pIn1->u.r))==pIn1->u.r
      ){
        pIn1->u.i = ix;
        MemSetTypeFlag(pIn1, MEM_Int);
      }
    }وإلا{
      applyAffinity(pIn1, SQLITE_AFF_NUMERIC, encoding);
    }
    إذا( (pIn1->flags & MEM_Int)==0 ){
      VdbeBranchTaken(1, 2);
      إذا( pOp->p2==0 ){
        rc = SQLITE_MISMATCH;
        اذهب abort_due_to_error;
      }وإلا{
        اذهب jump_to_p2;
      }
    }
  }
  VdbeBranchTaken(0, 2);
  MemSetTypeFlag(pIn1, MEM_Int);
  اكسر;
}

#ifndef SQLITE_OMIT_FLOATING_POINT
/* Opcode: RealAffinity P1 * * * *
**
** If register P1 holds an integer convert it to a real value.
**
** This opcode is used when extracting information from a column that
** has REAL affinity.  Such column values may still be stored as
** integers, for space efficiency, but after extraction we want them
** to have only a real value.
*/
حالة OP_RealAffinity: {                  /* in1 */
  pIn1 = &aMem[pOp->p1];
  إذا( pIn1->flags & (MEM_Int|MEM_IntReal) ){
    testcase( pIn1->flags & MEM_Int );
    testcase( pIn1->flags & MEM_IntReal );
    sqlite3VdbeMemRealify(pIn1);
    REGISTER_TRACE(pOp->p1, pIn1);
  }
  اكسر;
}
#endif

#إذا !defined(SQLITE_OMIT_CAST) || !defined(SQLITE_OMIT_ANALYZE)
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
حالة OP_Cast: {                  /* in1 */
  أكد( pOp->p2>=SQLITE_AFF_BLOB && pOp->p2<=SQLITE_AFF_REAL );
  testcase( pOp->p2==SQLITE_AFF_TEXT );
  testcase( pOp->p2==SQLITE_AFF_BLOB );
  testcase( pOp->p2==SQLITE_AFF_NUMERIC );
  testcase( pOp->p2==SQLITE_AFF_INTEGER );
  testcase( pOp->p2==SQLITE_AFF_REAL );
  pIn1 = &aMem[pOp->p1];
  memAboutToChange(p, pIn1);
  rc = ExpandBlob(pIn1);
  إذا( rc ) اذهب abort_due_to_error;
  rc = sqlite3VdbeMemCast(pIn1, pOp->p2, encoding);
  إذا( rc ) اذهب abort_due_to_error;
  UPDATE_MAX_BLOBSIZE(pIn1);
  REGISTER_TRACE(pOp->p1, pIn1);
  اكسر;
}
#endif /* SQLITE_OMIT_CAST */

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
حالة OP_Eq:               /* same as TK_EQ, jump, in1, in3 */
حالة OP_Ne:               /* same as TK_NE, jump, in1, in3 */
حالة OP_Lt:               /* same as TK_LT, jump, in1, in3 */
حالة OP_Le:               /* same as TK_LE, jump, in1, in3 */
حالة OP_Gt:               /* same as TK_GT, jump, in1, in3 */
حالة OP_Ge: {             /* same as TK_GE, jump, in1, in3 */
  صحيح res, res2;      /* Result of the comparison of pIn1 against pIn3 */
  حرف affinity;      /* Affinity to use for comparison */
  u16 flags1;         /* Copy of initial value of pIn1->flags */
  u16 flags3;         /* Copy of initial value of pIn3->flags */

  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  flags1 = pIn1->flags;
  flags3 = pIn3->flags;
  إذا( (flags1 & flags3 & MEM_Int)!=0 ){
    /* Common case of comparison of two integers */
    إذا( pIn3->u.i > pIn1->u.i ){
      إذا( sqlite3aGTb[pOp->opcode] ){
        VdbeBranchTaken(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        اذهب jump_to_p2;
      }
      iCompare = +1;
      VVA_ONLY( iCompareIsInit = 1; )
    }وإلا إذا( pIn3->u.i < pIn1->u.i ){
      إذا( sqlite3aLTb[pOp->opcode] ){
        VdbeBranchTaken(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        اذهب jump_to_p2;
      }
      iCompare = -1;
      VVA_ONLY( iCompareIsInit = 1; )
    }وإلا{
      إذا( sqlite3aEQb[pOp->opcode] ){
        VdbeBranchTaken(1, (pOp->p5 & SQLITE_NULLEQ)?2:3);
        اذهب jump_to_p2;
      }
      iCompare = 0;
      VVA_ONLY( iCompareIsInit = 1; )
    }
    VdbeBranchTaken(0, (pOp->p5 & SQLITE_NULLEQ)?2:3);
    اكسر;
  }
  إذا( (flags1 | flags3)&MEM_Null ){
    /* One or both operands are NULL */
    إذا( pOp->p5 & SQLITE_NULLEQ ){
      /* If SQLITE_NULLEQ is set (which will only happen if the operator is
      ** OP_Eq or OP_Ne) then take the jump or not depending on whether
      ** or not both operands are null.
      */
      أكد( (flags1 & MEM_Cleared)==0 );
      أكد( (pOp->p5 & SQLITE_JUMPIFNULL)==0 || CORRUPT_DB );
      testcase( (pOp->p5 & SQLITE_JUMPIFNULL)!=0 );
      إذا( (flags1&flags3&MEM_Null)!=0
       && (flags3&MEM_Cleared)==0
      ){
        res = 0;  /* Operands are equal */
      }وإلا{
        res = ((flags3 & MEM_Null) ? -1 : +1);  /* Operands are not equal */
      }
    }وإلا{
      /* SQLITE_NULLEQ is clear and at least one operand is NULL,
      ** then the result is always NULL.
      ** The jump is taken if the SQLITE_JUMPIFNULL bit is set.
      */
      VdbeBranchTaken(2,3);
      إذا( pOp->p5 & SQLITE_JUMPIFNULL ){
        اذهب jump_to_p2;
      }
      iCompare = 1;    /* Operands are not equal */
      VVA_ONLY( iCompareIsInit = 1; )
      اكسر;
    }
  }وإلا{
    /* Neither operand is NULL and we couldn't do the special high-speed
    ** integer comparison case.  So do a general-case comparison. */
    affinity = pOp->p5 & SQLITE_AFF_MASK;
    إذا( affinity>=SQLITE_AFF_NUMERIC ){
      إذا( (flags1 | flags3)&MEM_Str ){
        إذا( (flags1 & (MEM_Int|MEM_IntReal|MEM_Real|MEM_Str))==MEM_Str ){
          applyNumericAffinity(pIn1,0);
          أكد( flags3==pIn3->flags || CORRUPT_DB );
          flags3 = pIn3->flags;
        }
        إذا( (flags3 & (MEM_Int|MEM_IntReal|MEM_Real|MEM_Str))==MEM_Str ){
          applyNumericAffinity(pIn3,0);
        }
      }
    }وإلا إذا( affinity==SQLITE_AFF_TEXT && ((flags1 | flags3) & MEM_Str)!=0 ){
      إذا( (flags1 & MEM_Str)!=0 ){
        pIn1->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
      }وإلا إذا( (flags1&(MEM_Int|MEM_Real|MEM_IntReal))!=0 ){
        testcase( pIn1->flags & MEM_Int );
        testcase( pIn1->flags & MEM_Real );
        testcase( pIn1->flags & MEM_IntReal );
        sqlite3VdbeMemStringify(pIn1, encoding, 1);
        testcase( (flags1&MEM_Dyn) != (pIn1->flags&MEM_Dyn) );
        flags1 = (pIn1->flags & ~MEM_TypeMask) | (flags1 & MEM_TypeMask);
        إذا( NEVER(pIn1==pIn3) ) flags3 = flags1 | MEM_Str;
      }
      إذا( (flags3 & MEM_Str)!=0 ){
        pIn3->flags &= ~(MEM_Int|MEM_Real|MEM_IntReal);
      }وإلا إذا( (flags3&(MEM_Int|MEM_Real|MEM_IntReal))!=0 ){
        testcase( pIn3->flags & MEM_Int );
        testcase( pIn3->flags & MEM_Real );
        testcase( pIn3->flags & MEM_IntReal );
        sqlite3VdbeMemStringify(pIn3, encoding, 1);
        testcase( (flags3&MEM_Dyn) != (pIn3->flags&MEM_Dyn) );
        flags3 = (pIn3->flags & ~MEM_TypeMask) | (flags3 & MEM_TypeMask);
      }
    }
    أكد( pOp->p4type==P4_COLLSEQ || pOp->p4.pColl==0 );
    res = sqlite3MemCompare(pIn3, pIn1, pOp->p4.pColl);
  }

  /* At this point, res is negative, zero, or positive if reg[P1] is
  ** less than, equal to, or greater than reg[P3], respectively.  Compute
  ** the answer to this operator in res2, depending on what the comparison
  ** operator actually is.  The next block of code depends on the fact
  ** that the 6 comparison operators are consecutive integers in this
  ** order:  NE, EQ, GT, LE, LT, GE */
  أكد( OP_Eq==OP_Ne+1 ); أكد( OP_Gt==OP_Ne+2 ); أكد( OP_Le==OP_Ne+3 );
  أكد( OP_Lt==OP_Ne+4 ); أكد( OP_Ge==OP_Ne+5 );
  إذا( res<0 ){
    res2 = sqlite3aLTb[pOp->opcode];
  }وإلا إذا( res==0 ){
    res2 = sqlite3aEQb[pOp->opcode];
  }وإلا{
    res2 = sqlite3aGTb[pOp->opcode];
  }
  iCompare = res;
  VVA_ONLY( iCompareIsInit = 1; )

  /* Undo any changes made by applyAffinity() to the input registers. */
  أكد( (pIn3->flags & MEM_Dyn) == (flags3 & MEM_Dyn) );
  pIn3->flags = flags3;
  أكد( (pIn1->flags & MEM_Dyn) == (flags1 & MEM_Dyn) );
  pIn1->flags = flags1;

  VdbeBranchTaken(res2!=0, (pOp->p5 & SQLITE_NULLEQ)?2:3);
  إذا( res2 ){
    اذهب jump_to_p2;
  }
  اكسر;
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
حالة OP_ElseEq: {       /* same as TK_ESCAPE, jump */

#ifdef SQLITE_DEBUG
  /* Verify the preconditions of this opcode - that it follows an OP_Lt or
  ** OP_Gt with zero or more intervening OP_ReleaseReg opcodes */
  صحيح iAddr;
  لكل(iAddr = (صحيح)(pOp - aOp) - 1; ALWAYS(iAddr>=0); iAddr--){
    إذا( aOp[iAddr].opcode==OP_ReleaseReg ) استمر;
    أكد( aOp[iAddr].opcode==OP_Lt || aOp[iAddr].opcode==OP_Gt );
    اكسر;
  }
#endif /* SQLITE_DEBUG */
  أكد( iCompareIsInit );
  VdbeBranchTaken(iCompare==0, 2);
  إذا( iCompare==0 ) اذهب jump_to_p2;
  اكسر;
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
حالة OP_Permutation: {
  أكد( pOp->p4type==P4_INTARRAY );
  أكد( pOp->p4.ai );
  أكد( pOp[1].opcode==OP_Compare );
  أكد( pOp[1].p5 & OPFLAG_PERMUTE );
  اكسر;
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
حالة OP_Compare: {
  صحيح n;
  صحيح i;
  صحيح p1;
  صحيح p2;
  ثابت ئاچقۇچ_Info *p_ئاچقۇچ_Info;
  u32 idx;
  CollSeq *pColl;    /* Collating sequence to use on this term */
  صحيح bRev;          /* True for DESCENDING sort order */
  u32 *aPermute;     /* The permutation */

  إذا( (pOp->p5 & OPFLAG_PERMUTE)==0 ){
    aPermute = 0;
  }وإلا{
    أكد( pOp>aOp );
    أكد( pOp[-1].opcode==OP_Permutation );
    أكد( pOp[-1].p4type==P4_INTARRAY );
    aPermute = pOp[-1].p4.ai + 1;
    أكد( aPermute!=0 );
  }
  n = pOp->p3;
  p_ئاچقۇچ_Info = pOp->p4.p_ئاچقۇچ_Info;
  أكد( n>0 );
  أكد( p_ئاچقۇچ_Info!=0 );
  أكد( p_ئاچقۇچ_Info->aSortFlags!=0 );
  p1 = pOp->p1;
  p2 = pOp->p2;
#ifdef SQLITE_DEBUG
  إذا( aPermute ){
    صحيح k, mx = 0;
    لكل(k=0; k<n; k++) إذا( aPermute[k]>(u32)mx ) mx = aPermute[k];
    أكد( p1>0 && p1+mx<=(p->nMem+1 - p->nCursor)+1 );
    أكد( p2>0 && p2+mx<=(p->nMem+1 - p->nCursor)+1 );
  }وإلا{
    أكد( p1>0 && p1+n<=(p->nMem+1 - p->nCursor)+1 );
    أكد( p2>0 && p2+n<=(p->nMem+1 - p->nCursor)+1 );
  }
#endif /* SQLITE_DEBUG */
  لكل(i=0; i<n; i++){
    idx = aPermute ? aPermute[i] : (u32)i;
    أكد( memIsValid(&aMem[p1+idx]) );
    أكد( memIsValid(&aMem[p2+idx]) );
    REGISTER_TRACE(p1+idx, &aMem[p1+idx]);
    REGISTER_TRACE(p2+idx, &aMem[p2+idx]);
    أكد( i<p_ئاچقۇچ_Info->n_ئاچقۇچ_Field );
    pColl = p_ئاچقۇچ_Info->aColl[i];
    bRev = (p_ئاچقۇچ_Info->aSortFlags[i] & KEYINFO_ORDER_DESC);
    iCompare = sqlite3MemCompare(&aMem[p1+idx], &aMem[p2+idx], pColl);
    VVA_ONLY( iCompareIsInit = 1; )
    إذا( iCompare ){
      إذا( (p_ئاچقۇچ_Info->aSortFlags[i] & KEYINFO_ORDER_BIGNULL)
       && ((aMem[p1+idx].flags & MEM_Null) || (aMem[p2+idx].flags & MEM_Null))
      ){
        iCompare = -iCompare;
      }
      إذا( bRev ) iCompare = -iCompare;
      اكسر;
    }
  }
  أكد( pOp[1].opcode==OP_Jump );
  اكسر;
}

/* Opcode: Jump P1 P2 P3 * *
**
** Jump to the instruction at address P1, P2, or P3 depending on whether
** in the most recent OP_Compare instruction the P1 vector was less than,
** equal to, or greater than the P2 vector, respectively.
**
** This opcode must immediately follow an OP_Compare opcode.
*/
حالة OP_Jump: {             /* jump */
  أكد( pOp>aOp && pOp[-1].opcode==OP_Compare );
  أكد( iCompareIsInit );
  إذا( iCompare<0 ){
    VdbeBranchTaken(0,4); pOp = &aOp[pOp->p1 - 1];
  }وإلا إذا( iCompare==0 ){
    VdbeBranchTaken(1,4); pOp = &aOp[pOp->p2 - 1];
  }وإلا{
    VdbeBranchTaken(2,4); pOp = &aOp[pOp->p3 - 1];
  }
  اكسر;
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
حالة OP_And:              /* same as TK_AND, in1, in2, out3 */
حالة OP_Or: {             /* same as TK_OR, in1, in2, out3 */
  صحيح v1;    /* Left operand:  0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */
  صحيح v2;    /* Right operand: 0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */

  v1 = sqlite3_Vdbe_Boolean_قىممەت(&aMem[pOp->p1], 2);
  v2 = sqlite3_Vdbe_Boolean_قىممەت(&aMem[pOp->p2], 2);
  إذا( pOp->opcode==OP_And ){
    ساكن ثابت unsigned حرف and_logic[] = { 0, 0, 0, 0, 1, 2, 0, 2, 2 };
    v1 = and_logic[v1*3+v2];
  }وإلا{
    ساكن ثابت unsigned حرف or_logic[] = { 0, 1, 2, 1, 1, 1, 2, 1, 2 };
    v1 = or_logic[v1*3+v2];
  }
  pOut = &aMem[pOp->p3];
  إذا( v1==2 ){
    MemSetTypeFlag(pOut, MEM_Null);
  }وإلا{
    pOut->u.i = v1;
    MemSetTypeFlag(pOut, MEM_Int);
  }
  اكسر;
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
حالة OP_IsTrue: {               /* in1, out2 */
  أكد( pOp->p4type==P4_INT32 );
  أكد( pOp->p4.i==0 || pOp->p4.i==1 );
  أكد( pOp->p3==0 || pOp->p3==1 );
  sqlite3VdbeMemSetInt64(&aMem[pOp->p2],
      sqlite3_Vdbe_Boolean_قىممەت(&aMem[pOp->p1], pOp->p3) ^ pOp->p4.i);
  اكسر;
}

/* Opcode: Not P1 P2 * * *
** Synopsis: r[P2]= !r[P1]
**
** Interpret the value in register P1 as a boolean value.  Store the
** boolean complement in register P2.  If the value in register P1 is
** NULL, then a NULL is stored in P2.
*/
حالة OP_Not: {                /* same as TK_NOT, in1, out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  إذا( (pIn1->flags & MEM_Null)==0 ){
    sqlite3VdbeMemSetInt64(pOut, !sqlite3_Vdbe_Boolean_قىممەت(pIn1,0));
  }وإلا{
    sqlite3VdbeMemSetNull(pOut);
  }
  اكسر;
}

/* Opcode: BitNot P1 P2 * * *
** Synopsis: r[P2]= ~r[P1]
**
** Interpret the content of register P1 as an integer.  Store the
** ones-complement of the P1 value into register P2.  If P1 holds
** a NULL then store a NULL in P2.
*/
حالة OP_BitNot: {             /* same as TK_BITNOT, in1, out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  sqlite3VdbeMemSetNull(pOut);
  إذا( (pIn1->flags & MEM_Null)==0 ){
    pOut->flags = MEM_Int;
    pOut->u.i = ~sqlite3_Vdbe_Int_قىممەت(pIn1);
  }
  اكسر;
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
حالة OP_Once: {             /* jump */
  u32 iAddr;                /* Address of this instruction */
  أكد( p->aOp[0].opcode==OP_Init );
  إذا( p->pFrame ){
    iAddr = (صحيح)(pOp - p->aOp);
    إذا( (p->pFrame->aOnce[iAddr/8] & (1<<(iAddr & 7)))!=0 ){
      VdbeBranchTaken(1, 2);
      اذهب jump_to_p2;
    }
    p->pFrame->aOnce[iAddr/8] |= 1<<(iAddr & 7);
  }وإلا{
    إذا( p->aOp[0].p1==pOp->p1 ){
      VdbeBranchTaken(1, 2);
      اذهب jump_to_p2;
    }
  }
  VdbeBranchTaken(0, 2);
  pOp->p1 = p->aOp[0].p1;
  اكسر;
}

/* Opcode: If P1 P2 P3 * *
**
** Jump to P2 if the value in register P1 is true.  The value
** is considered true if it is numeric and non-zero.  If the value
** in P1 is NULL then take the jump if and only if P3 is non-zero.
*/
حالة OP_If:  {               /* jump, in1 */
  صحيح c;
  c = sqlite3_Vdbe_Boolean_قىممەت(&aMem[pOp->p1], pOp->p3);
  VdbeBranchTaken(c!=0, 2);
  إذا( c ) اذهب jump_to_p2;
  اكسر;
}

/* Opcode: IfNot P1 P2 P3 * *
**
** Jump to P2 if the value in register P1 is False.  The value
** is considered false if it has a numeric value of zero.  If the value
** in P1 is NULL then take the jump if and only if P3 is non-zero.
*/
حالة OP_IfNot: {            /* jump, in1 */
  صحيح c;
  c = !sqlite3_Vdbe_Boolean_قىممەت(&aMem[pOp->p1], !pOp->p3);
  VdbeBranchTaken(c!=0, 2);
  إذا( c ) اذهب jump_to_p2;
  اكسر;
}

/* Opcode: IsNull P1 P2 * * *
** Synopsis: if r[P1]==NULL goto P2
**
** Jump to P2 if the value in register P1 is NULL.
*/
حالة OP_IsNull: {            /* same as TK_ISNULL, jump, in1 */
  pIn1 = &aMem[pOp->p1];
  VdbeBranchTaken( (pIn1->flags & MEM_Null)!=0, 2);
  إذا( (pIn1->flags & MEM_Null)!=0 ){
    اذهب jump_to_p2;
  }
  اكسر;
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
حالة OP_IsType: {        /* jump */
  VdbeCursor *pC;
  u16 typeMask;
  u32 serialType;

  أكد( pOp->p1>=(-1) && pOp->p1<p->nCursor );
  أكد( pOp->p1>=0 || (pOp->p3>=0 && pOp->p3<=(p->nMem+1 - p->nCursor)) );
  إذا( pOp->p1>=0 ){
    pC = p->apCsr[pOp->p1];
    أكد( pC!=0 );
    أكد( pOp->p3>=0 );
    إذا( pOp->p3<pC->nHdrParsed ){
      serialType = pC->aType[pOp->p3];
      إذا( serialType>=12 ){
        إذا( serialType&1 ){
          typeMask = 0x04;   /* SQLITE_TEXT */
        }وإلا{
          typeMask = 0x08;   /* SQLITE_BLOB */
        }
      }وإلا{
        ساكن ثابت unsigned حرف aMask[] = {
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
        typeMask = aMask[serialType];
      }
    }وإلا{
      typeMask = 1 << (pOp->p4.i - 1);
      testcase( typeMask==0x01 );
      testcase( typeMask==0x02 );
      testcase( typeMask==0x04 );
      testcase( typeMask==0x08 );
      testcase( typeMask==0x10 );
    }
  }وإلا{
    أكد( memIsValid(&aMem[pOp->p3]) );
    typeMask = 1 << (sqlite3_قىممەت_type((sqlite3_قىممەت*)&aMem[pOp->p3])-1);
    testcase( typeMask==0x01 );
    testcase( typeMask==0x02 );
    testcase( typeMask==0x04 );
    testcase( typeMask==0x08 );
    testcase( typeMask==0x10 );
  }
  VdbeBranchTaken( (typeMask & pOp->p5)!=0, 2);
  إذا( typeMask & pOp->p5 ){
    اذهب jump_to_p2;
  }
  اكسر;
}

/* Opcode: ZeroOrNull P1 P2 P3 * *
** Synopsis: r[P2] = 0 OR NULL
**
** If both registers P1 and P3 are NOT NULL, then store a zero in
** register P2.  If either registers P1 or P3 are NULL then put
** a NULL in register P2.
*/
حالة OP_ZeroOrNull: {            /* in1, in2, out2, in3 */
  إذا( (aMem[pOp->p1].flags & MEM_Null)!=0
   || (aMem[pOp->p3].flags & MEM_Null)!=0
  ){
    sqlite3VdbeMemSetNull(aMem + pOp->p2);
  }وإلا{
    sqlite3VdbeMemSetInt64(aMem + pOp->p2, 0);
  }
  اكسر;
}

/* Opcode: NotNull P1 P2 * * *
** Synopsis: if r[P1]!=NULL goto P2
**
** Jump to P2 if the value in register P1 is not NULL. 
*/
حالة OP_NotNull: {            /* same as TK_NOTNULL, jump, in1 */
  pIn1 = &aMem[pOp->p1];
  VdbeBranchTaken( (pIn1->flags & MEM_Null)==0, 2);
  إذا( (pIn1->flags & MEM_Null)==0 ){
    اذهب jump_to_p2;
  }
  اكسر;
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
حالة OP_IfNullRow: {         /* jump */
  VdbeCursor *pC;
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  إذا( pC && pC->nullRow ){
    sqlite3VdbeMemSetNull(aMem + pOp->p3);
    اذهب jump_to_p2;
  }
  اكسر;
}

#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
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
حالة OP_Offset: {          /* out3 */
  VdbeCursor *pC;    /* The VDBE cursor */
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  pOut = &p->aMem[pOp->p3];
  إذا( pC==0 || pC->eCurType!=CURTYPE_BTREE ){
    sqlite3VdbeMemSetNull(pOut);
  }وإلا{
    إذا( pC->deferredMoveto ){
      rc = sqlite3VdbeFinishMoveto(pC);
      إذا( rc ) اذهب abort_due_to_error;
    }
    إذا( sqlite3BtreeEof(pC->uc.pCursor) ){
      sqlite3VdbeMemSetNull(pOut);
    }وإلا{
      sqlite3VdbeMemSetInt64(pOut, sqlite3BtreeOffset(pC->uc.pCursor));
    }
  }
  اكسر;
}
#endif /* SQLITE_ENABLE_OFFSET_SQL_FUNC */

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
حالة OP_Column: {            /* ncycle */
  u32 p2;            /* column number to retrieve */
  VdbeCursor *pC;    /* The VDBE cursor */
  BtCursor *pCrsr;   /* The B-Tree cursor corresponding to pC */
  u32 *aOffset;      /* aOffset[i] is offset to start of data for i-th column */
  صحيح len;           /* The length of the serialized data for the column */
  صحيح i;             /* Loop counter */
  Mem *pDest;        /* Where to write the extracted value */
  Mem sMem;          /* For storing the record being decoded */
  ثابت u8 *zData;   /* Part of the record being decoded */
  ثابت u8 *zHdr;    /* Next unparsed byte of the header */
  ثابت u8 *zEndHdr; /* Pointer to first byte after the header */
  u64 offset64;      /* 64-bit offset */
  u32 t;             /* A type code from the record header */
  Mem *pReg;         /* PseudoTable input register */

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  pC = p->apCsr[pOp->p1];
  p2 = (u32)pOp->p2;

op_column_restart:
  أكد( pC!=0 );
  أكد( p2<(u32)pC->nField
       || (pC->eCurType==CURTYPE_PSEUDO && pC->seekResult==0) );
  aOffset = pC->aOffset;
  أكد( aOffset==pC->aType+pC->nField );
  أكد( pC->eCurType!=CURTYPE_VTAB );
  أكد( pC->eCurType!=CURTYPE_PSEUDO || pC->nullRow );
  أكد( pC->eCurType!=CURTYPE_SORTER );

  إذا( pC->cacheStatus!=p->cacheCtr ){                /*OPTIMIZATION-IF-FALSE*/
    إذا( pC->nullRow ){
      إذا( pC->eCurType==CURTYPE_PSEUDO && pC->seekResult>0 ){
        /* For the special case of as pseudo-cursor, the seekResult field
        ** identifies the register that holds the record */
        pReg = &aMem[pC->seekResult];
        أكد( pReg->flags & MEM_Blob );
        أكد( memIsValid(pReg) );
        pC->payloadSize = pC->szRow = pReg->n;
        pC->aRow = (u8*)pReg->z;
      }وإلا{
        pDest = &aMem[pOp->p3];
        memAboutToChange(p, pDest);
        sqlite3VdbeMemSetNull(pDest);
        اذهب op_column_out;
      }
    }وإلا{
      pCrsr = pC->uc.pCursor;
      إذا( pC->deferredMoveto ){
        u32 iMap;
        أكد( !pC->isEphemeral );
        إذا( pC->ub.aAltMap && (iMap = pC->ub.aAltMap[1+p2])>0  ){
          pC = pC->pAltCursor;
          p2 = iMap - 1;
          اذهب op_column_restart;
        }
        rc = sqlite3VdbeFinishMoveto(pC);
        إذا( rc ) اذهب abort_due_to_error;
      }وإلا إذا( sqlite3BtreeCursorHasMoved(pCrsr) ){
        rc = sqlite3VdbeHandleMovedCursor(pC);
        إذا( rc ) اذهب abort_due_to_error;
        اذهب op_column_restart;
      }
      أكد( pC->eCurType==CURTYPE_BTREE );
      أكد( pCrsr );
      أكد( sqlite3BtreeCursorIsValid(pCrsr) );
      pC->payloadSize = sqlite3BtreePayloadSize(pCrsr);
      pC->aRow = sqlite3BtreePayloadFetch(pCrsr, &pC->szRow);
      أكد( pC->szRow<=pC->payloadSize );
      أكد( pC->szRow<=65536 );  /* Maximum page size is 64KiB */
    }
    pC->cacheStatus = p->cacheCtr;
    إذا( (aOffset[0] = pC->aRow[0])<0x80 ){
      pC->iHdrOffset = 1;
    }وإلا{
      pC->iHdrOffset = sqlite3GetVarint32(pC->aRow, aOffset);
    }
    pC->nHdrParsed = 0;

    إذا( pC->szRow<aOffset[0] ){      /*OPTIMIZATION-IF-FALSE*/
      /* pC->aRow does not have to hold the entire row, but it does at least
      ** need to cover the header of the record.  If pC->aRow does not contain
      ** the complete header, then set it to zero, forcing the header to be
      ** dynamically allocated. */
      pC->aRow = 0;
      pC->szRow = 0;

      /* Make sure a corrupt database has not given us an oversize header.
      ** Do this now to avoid an oversize memory allocation.
      **
      ** Type entries can be between 1 and 5 bytes each.  But 4 and 5 byte
      ** types use so much data space that there can only be 4096 and 32 of
      ** them, respectively.  So the maximum header length results from a
      ** 3-byte type for each of the maximum of 32768 columns plus three
      ** extra bytes for the header length itself.  32768*3 + 3 = 98307.
      */
      إذا( aOffset[0] > 98307 || aOffset[0] > pC->payloadSize ){
        اذهب op_column_corrupt;
      }
    }وإلا{
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
      zData = pC->aRow;
      أكد( pC->nHdrParsed<=p2 );         /* Conditional skipped */
      testcase( aOffset[0]==0 );
      اذهب op_column_read_header;
    }
  }وإلا إذا( sqlite3BtreeCursorHasMoved(pC->uc.pCursor) ){
    rc = sqlite3VdbeHandleMovedCursor(pC);
    إذا( rc ) اذهب abort_due_to_error;
    اذهب op_column_restart;
  }

  /* Make sure at least the first p2+1 entries of the header have been
  ** parsed and valid information is in aOffset[] and pC->aType[].
  */
  إذا( pC->nHdrParsed<=p2 ){
    /* If there is more header available for parsing in the record, try
    ** to extract additional fields up through the p2+1-th field
    */
    إذا( pC->iHdrOffset<aOffset[0] ){
      /* Make sure zData points to enough of the record to cover the header. */
      إذا( pC->aRow==0 ){
        memset(&sMem, 0, حجم(sMem));
        rc = sqlite3VdbeMemFromBtreeZeroOffset(pC->uc.pCursor,aOffset[0],&sMem);
        إذا( rc!=SQLITE_OK ) اذهب abort_due_to_error;
        zData = (u8*)sMem.z;
      }وإلا{
        zData = pC->aRow;
      }
 
      /* Fill in pC->aType[i] and aOffset[i] values through the p2-th field. */
    op_column_read_header:
      i = pC->nHdrParsed;
      offset64 = aOffset[i];
      zHdr = zData + pC->iHdrOffset;
      zEndHdr = zData + aOffset[0];
      testcase( zHdr>=zEndHdr );
      افعل{
        إذا( (pC->aType[i] = t = zHdr[0])<0x80 ){
          zHdr++;
          offset64 += sqlite3VdbeOneByteSerialTypeLen(t);
        }وإلا{
          zHdr += sqlite3GetVarint32(zHdr, &t);
          pC->aType[i] = t;
          offset64 += sqlite3VdbeSerialTypeLen(t);
        }
        aOffset[++i] = (u32)(offset64 & 0xffffffff);
      }طالما( (u32)i<=p2 && zHdr<zEndHdr );

      /* The record is corrupt if any of the following are true:
      ** (1) the bytes of the header extend past the declared header size
      ** (2) the entire header was used but not all data was used
      ** (3) the end of the data extends beyond the end of the record.
      */
      إذا( (zHdr>=zEndHdr && (zHdr>zEndHdr || offset64!=pC->payloadSize))
       || (offset64 > pC->payloadSize)
      ){
        إذا( aOffset[0]==0 ){
          i = 0;
          zHdr = zEndHdr;
        }وإلا{
          إذا( pC->aRow==0 ) sqlite3VdbeMemRelease(&sMem);
          اذهب op_column_corrupt;
        }
      }

      pC->nHdrParsed = i;
      pC->iHdrOffset = (u32)(zHdr - zData);
      إذا( pC->aRow==0 ) sqlite3VdbeMemRelease(&sMem);
    }وإلا{
      t = 0;
    }

    /* If after trying to extract new entries from the header, nHdrParsed is
    ** still not up to p2, that means that the record has fewer than p2
    ** columns.  So the result will be either the default value or a NULL.
    */
    إذا( pC->nHdrParsed<=p2 ){
      pDest = &aMem[pOp->p3];
      memAboutToChange(p, pDest);
      إذا( pOp->p4type==P4_MEM ){
        sqlite3VdbeMemShallowCopy(pDest, pOp->p4.pMem, MEM_Static);
      }وإلا{
        sqlite3VdbeMemSetNull(pDest);
      }
      اذهب op_column_out;
    }
  }وإلا{
    t = pC->aType[p2];
  }

  /* Extract the content for the p2+1-th column.  Control can only
  ** reach this point if aOffset[p2], aOffset[p2+1], and pC->aType[p2] are
  ** all valid.
  */
  أكد( p2<pC->nHdrParsed );
  أكد( rc==SQLITE_OK );
  pDest = &aMem[pOp->p3];
  memAboutToChange(p, pDest);
  أكد( sqlite3VdbeCheckMemInvariants(pDest) );
  إذا( VdbeMemDynamic(pDest) ){
    sqlite3VdbeMemSetNull(pDest);
  }
  أكد( t==pC->aType[p2] );
  إذا( pC->szRow>=aOffset[p2+1] ){
    /* This is the common case where the desired content fits on the original
    ** page - where the content is not on an overflow page.
    **
    ** The big switch() is an in-line variant of sqlite3VdbeSerialGet() that
    ** has been optimized for the OP_Column opcode.
    */
    zData = pC->aRow + aOffset[p2];
    بدل( t ){
      حالة 0:
      حالة 11:
        pDest->flags = MEM_Null;
        اكسر;
      حالة 1:
        pDest->u.i = ONE_BYTE_INT(zData);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        اكسر;
      حالة 2:
        pDest->u.i = TWO_BYTE_INT(zData);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        اكسر;
      حالة 3:
        pDest->u.i = THREE_BYTE_INT(zData);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        اكسر;
      حالة 4:
        pDest->u.i = FOUR_BYTE_INT(zData);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        اكسر;
      حالة 5:
        pDest->u.i = SIX_BYTE_INT(zData);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        اكسر;
      حالة 6: {
        pDest->u.i = (i64)sqlite3Get8byte(zData);
        pDest->flags = MEM_Int;
        testcase( pDest->u.i<0 );
        اكسر;
      }
      حالة 7: {
        u64 x = sqlite3Get8byte(zData);
        swapMixedEndianFloat(x);
        pDest->flags = IsNaN(x) ? MEM_Null : MEM_Real;
        memcpy(&pDest->u.r, &x, حجم(x));
        testcase( pDest->u.r<0 );
        اكسر;
      }
      حالة 8:
      حالة 9: {
        pDest->u.i = t-8;
        pDest->flags = MEM_Int;
        اكسر;
      }
      حالة 10:
        /* Internal use only: NULL with virtual table
        ** UPDATE no-change flag set */
        pDest->flags = MEM_Null|MEM_Zero;
        pDest->u.nZero = 0;
        pDest->n = 0;
        اكسر;
      افتراضي: {
        /* If the column value is a string or blob, we need a persistent
        ** value, not a MEM_Ephem value.  This case is a fast short-cut
        ** that is equivalent to calling sqlite3VdbeSerialGet() and
        ** sqlite3VdbeDeephemeralize().
        */
        ساكن ثابت u16 aFlag[] = { MEM_Blob, MEM_Str|MEM_Term };
        pDest->n = len = (t-12)/2;
        pDest->enc = encoding;
        إذا( pDest->szMalloc < len+2 ){
          إذا( len>db->aLimit[SQLITE_LIMIT_LENGTH] ) اذهب too_big;
          pDest->flags = MEM_Null;
          إذا( sqlite3VdbeMemGrow(pDest, len+2, 0) ) اذهب no_mem;
        }وإلا{
          pDest->z = pDest->zMalloc;
        }
        memcpy(pDest->z, zData, len);
        pDest->z[len] = 0;
        pDest->z[len+1] = 0;
        pDest->flags = aFlag[t&1];
      }
    } /* End of switch */
  }وإلا{
    u8 p5;
    pDest->enc = encoding;
    أكد( pDest->db==db );
    /* This branch happens only when content is on overflow pages */
    إذا( ((p5 = (pOp->p5 & OPFLAG_BYTELENARG))!=0
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
    }وإلا{
      rc = vdbeColumnFromOverflow(pC, p2, t, aOffset[p2],
                p->cacheCtr, colCacheCtr, pDest);
      إذا( rc ){
        إذا( rc==SQLITE_NOMEM ) اذهب no_mem;
        إذا( rc==SQLITE_TOOBIG ) اذهب too_big;
        اذهب abort_due_to_error;
      }
    }
  }

op_column_out:
  UPDATE_MAX_BLOBSIZE(pDest);
  REGISTER_TRACE(pOp->p3, pDest);
  اكسر;

op_column_corrupt:
  إذا( aOp[0].p3>0 ){
    pOp = &aOp[aOp[0].p3-1];
    اكسر;
  }وإلا{
    rc = SQLITE_CORRUPT_BKPT;
    اذهب abort_due_to_error;
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
حالة OP_TypeCheck: {
  Table *pTab;
  Column *aCol;
  صحيح i;
  صحيح nCol;

  أكد( pOp->p4type==P4_TABLE );
  pTab = pOp->p4.pTab;
  أكد( pTab->tabFlags & TF_Strict );
  أكد( pOp->p3>=0 && pOp->p3<pTab->nCol+2 );
  aCol = pTab->aCol;
  pIn1 = &aMem[pOp->p1];
  إذا( pOp->p3<2 ){
    أكد( pTab->nNVCol==pOp->p2 );
    i = 0;
    nCol = pTab->nCol;
  }وإلا{
    i = pOp->p3-2;
    nCol = i+1;
    أكد( i<pTab->nCol );
    أكد( aCol[i].colFlags & COLFLAG_VIRTUAL );
    أكد( pOp->p2==1 );
  }
  لكل(; i<nCol; i++){
    إذا( (aCol[i].colFlags & COLFLAG_GENERATED)!=0 && pOp->p3<2 ){
      إذا( (aCol[i].colFlags & COLFLAG_VIRTUAL)!=0 ) استمر;
      إذا( pOp->p3 ){ pIn1++; استمر; }
    }
    أكد( pIn1 < &aMem[pOp->p1+pOp->p2] );
    applyAffinity(pIn1, aCol[i].affinity, encoding);
    إذا( (pIn1->flags & MEM_Null)==0 ){
      بدل( aCol[i].eCType ){
        حالة COLTYPE_BLOB: {
          إذا( (pIn1->flags & MEM_Blob)==0 ) اذهب vdbe_type_error;
          اكسر;
        }
        حالة COLTYPE_INTEGER:
        حالة COLTYPE_INT: {
          إذا( (pIn1->flags & MEM_Int)==0 ) اذهب vdbe_type_error;
          اكسر;
        }
        حالة COLTYPE_TEXT: {
          إذا( (pIn1->flags & MEM_Str)==0 ) اذهب vdbe_type_error;
          اكسر;
        }
        حالة COLTYPE_REAL: {
          testcase( (pIn1->flags & (MEM_Real|MEM_IntReal))==MEM_Real );
          أكد( (pIn1->flags & MEM_IntReal)==0 );
          إذا( pIn1->flags & MEM_Int ){
            /* When applying REAL affinity, if the result is still an MEM_Int
            ** that will fit in 6 bytes, then change the type to MEM_IntReal
            ** so that we keep the high-resolution integer value but know that
            ** the type really wants to be REAL. */
            testcase( pIn1->u.i==140737488355328LL );
            testcase( pIn1->u.i==140737488355327LL );
            testcase( pIn1->u.i==-140737488355328LL );
            testcase( pIn1->u.i==-140737488355329LL );
            إذا( pIn1->u.i<=140737488355327LL && pIn1->u.i>=-140737488355328LL){
              pIn1->flags |= MEM_IntReal;
              pIn1->flags &= ~MEM_Int;
            }وإلا{
              pIn1->u.r = (مزدوج)pIn1->u.i;
              pIn1->flags |= MEM_Real;
              pIn1->flags &= ~MEM_Int;
            }
          }وإلا إذا( (pIn1->flags & (MEM_Real|MEM_IntReal))==0 ){
            اذهب vdbe_type_error;
          }
          اكسر;
        }
        افتراضي: {
          /* COLTYPE_ANY.  Accept anything. */
          اكسر;
        }
      }
    }
    REGISTER_TRACE((صحيح)(pIn1-aMem), pIn1);
    pIn1++;
  }
  أكد( pIn1 == &aMem[pOp->p1+pOp->p2] );
  اكسر;

vdbe_type_error:
  sqlite3VdbeError(p, "cannot store %s value in %s column %s.%s",
     vdbeMemTypeName(pIn1), sqlite3StdType[aCol[i].eCType-1],
     pTab->zName, aCol[i].zCnName);
  rc = SQLITE_CONSTRAINT_DATATYPE;
  اذهب abort_due_to_error;
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
حالة OP_Affinity: {
  ثابت حرف *zAffinity;   /* The affinity to be applied */

  zAffinity = pOp->p4.z;
  أكد( zAffinity!=0 );
  أكد( pOp->p2>0 );
  أكد( zAffinity[pOp->p2]==0 );
  pIn1 = &aMem[pOp->p1];
  طالما( 1 /*exit-by-break*/ ){
    أكد( pIn1 <= &p->aMem[(p->nMem+1 - p->nCursor)] );
    أكد( zAffinity[0]==SQLITE_AFF_NONE || memIsValid(pIn1) );
    applyAffinity(pIn1, zAffinity[0], encoding);
    إذا( zAffinity[0]==SQLITE_AFF_REAL && (pIn1->flags & MEM_Int)!=0 ){
      pIn1->u.r = (مزدوج)pIn1->u.i;
      pIn1->flags |= MEM_Real;
      pIn1->flags &= ~(MEM_Int|MEM_Str);
    }
    REGISTER_TRACE((صحيح)(pIn1-aMem), pIn1);
    zAffinity++;
    إذا( zAffinity[0]==0 ) اكسر;
    pIn1++;
  }
  اكسر;
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
حالة OP_MakeRecord: {
  Mem *pRec;             /* The new record */
  u64 nData;             /* Number of bytes of data space */
  صحيح nHdr;              /* Number of bytes of header space */
  i64 nByte;             /* Data space required for this record */
  i64 nZero;             /* Number of zero bytes at the end of the record */
  صحيح nVarint;           /* Number of bytes in a varint */
  u32 serial_type;       /* Type field */
  Mem *pData0;           /* First field to be combined into the record */
  Mem *pLast;            /* Last field of the record */
  صحيح nField;            /* Number of fields in the record */
  حرف *zAffinity;       /* The affinity string for the record */
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
  nData = 0;         /* Number of bytes of data space */
  nHdr = 0;          /* Number of bytes of header space */
  nZero = 0;         /* Number of zero bytes at the end of the record */
  nField = pOp->p1;
  zAffinity = pOp->p4.z;
  أكد( nField>0 && pOp->p2>0 && pOp->p2+nField<=(p->nMem+1 - p->nCursor)+1 );
  pData0 = &aMem[nField];
  nField = pOp->p2;
  pLast = &pData0[nField-1];

  /* Identify the output register */
  أكد( pOp->p3<pOp->p1 || pOp->p3>=pOp->p1+pOp->p2 );
  pOut = &aMem[pOp->p3];
  memAboutToChange(p, pOut);

  /* Apply the requested affinity to all inputs
  */
  أكد( pData0<=pLast );
  إذا( zAffinity ){
    pRec = pData0;
    افعل{
      applyAffinity(pRec, zAffinity[0], encoding);
      إذا( zAffinity[0]==SQLITE_AFF_REAL && (pRec->flags & MEM_Int) ){
        pRec->flags |= MEM_IntReal;
        pRec->flags &= ~(MEM_Int);
      }
      REGISTER_TRACE((صحيح)(pRec-aMem), pRec);
      zAffinity++;
      pRec++;
      أكد( zAffinity[0]==0 || pRec<=pLast );
    }طالما( zAffinity[0] );
  }

#ifdef SQLITE_ENABLE_NULL_TRIM
  /* NULLs can be safely trimmed from the end of the record, as long as
  ** as the schema format is 2 or more and none of the omitted columns
  ** have a non-NULL default value.  Also, the record must be left with
  ** at least one field.  If P5>0 then it will be one more than the
  ** index of the right-most column with a non-NULL default value */
  إذا( pOp->p5 ){
    طالما( (pLast->flags & MEM_Null)!=0 && nField>pOp->p5 ){
      pLast--;
      nField--;
    }
  }
#endif

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
  pRec = pLast;
  افعل{
    أكد( memIsValid(pRec) );
    إذا( pRec->flags & MEM_Null ){
      إذا( pRec->flags & MEM_Zero ){
        /* Values with MEM_Null and MEM_Zero are created by xColumn virtual
        ** table methods that never invoke sqlite3_result_xxxxx() while
        ** computing an unchanging column value in an UPDATE statement.
        ** Give such values a special internal-use-only serial-type of 10
        ** so that they can be passed through to xUpdate and have
        ** a true sqlite3_value_nochange(). */
#ifndef SQLITE_ENABLE_NULL_TRIM
        أكد( pOp->p5==OPFLAG_NOCHNG_MAGIC || CORRUPT_DB );
#endif
        pRec->uTemp = 10;
      }وإلا{
        pRec->uTemp = 0;
      }
      nHdr++;
    }وإلا إذا( pRec->flags & (MEM_Int|MEM_IntReal) ){
      /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
      i64 i = pRec->u.i;
      u64 uu;
      testcase( pRec->flags & MEM_Int );
      testcase( pRec->flags & MEM_IntReal );
      إذا( i<0 ){
        uu = ~i;
      }وإلا{
        uu = i;
      }
      nHdr++;
      testcase( uu==127 );               testcase( uu==128 );
      testcase( uu==32767 );             testcase( uu==32768 );
      testcase( uu==8388607 );           testcase( uu==8388608 );
      testcase( uu==2147483647 );        testcase( uu==2147483648LL );
      testcase( uu==140737488355327LL ); testcase( uu==140737488355328LL );
      إذا( uu<=127 ){
        إذا( (i&1)==i && p->minWriteFileFormat>=4 ){
          pRec->uTemp = 8+(u32)uu;
        }وإلا{
          nData++;
          pRec->uTemp = 1;
        }
      }وإلا إذا( uu<=32767 ){
        nData += 2;
        pRec->uTemp = 2;
      }وإلا إذا( uu<=8388607 ){
        nData += 3;
        pRec->uTemp = 3;
      }وإلا إذا( uu<=2147483647 ){
        nData += 4;
        pRec->uTemp = 4;
      }وإلا إذا( uu<=140737488355327LL ){
        nData += 6;
        pRec->uTemp = 5;
      }وإلا{
        nData += 8;
        إذا( pRec->flags & MEM_IntReal ){
          /* If the value is IntReal and is going to take up 8 bytes to store
          ** as an integer, then we might as well make it an 8-byte floating
          ** point value */
          pRec->u.r = (مزدوج)pRec->u.i;
          pRec->flags &= ~MEM_IntReal;
          pRec->flags |= MEM_Real;
          pRec->uTemp = 7;
        }وإلا{
          pRec->uTemp = 6;
        }
      }
    }وإلا إذا( pRec->flags & MEM_Real ){
      nHdr++;
      nData += 8;
      pRec->uTemp = 7;
    }وإلا{
      أكد( db->mallocFailed || pRec->flags&(MEM_Str|MEM_Blob) );
      أكد( pRec->n>=0 );
      len = (u32)pRec->n;
      serial_type = (len*2) + 12 + ((pRec->flags & MEM_Str)!=0);
      إذا( pRec->flags & MEM_Zero ){
        serial_type += (u32)pRec->u.nZero*2;
        إذا( nData ){
          إذا( sqlite3VdbeMemExpandBlob(pRec) ) اذهب no_mem;
          len += pRec->u.nZero;
        }وإلا{
          nZero += pRec->u.nZero;
        }
      }
      nData += len;
      nHdr += sqlite3VarintLen(serial_type);
      pRec->uTemp = serial_type;
    }
    إذا( pRec==pData0 ) اكسر;
    pRec--;
  }طالما(1);

  /* EVIDENCE-OF: R-22564-11647 The header begins with a single varint
  ** which determines the total number of bytes in the header. The varint
  ** value is the size of the header in bytes including the size varint
  ** itself. */
  testcase( nHdr==126 );
  testcase( nHdr==127 );
  إذا( nHdr<=126 ){
    /* The common case */
    nHdr += 1;
  }وإلا{
    /* Rare case of a really large header */
    nVarint = sqlite3VarintLen(nHdr);
    nHdr += nVarint;
    إذا( nVarint<sqlite3VarintLen(nHdr) ) nHdr++;
  }
  nByte = nHdr+nData;

  /* If we are able to put an over-run area of 7 bytes on the end of the
  ** memory allocation into which the record is being constructed, then
  ** the encoding of integer values can go faster. This is only possible
  ** if SQLITE_MAX_LENGTH is no with 7 of INT32_MAX and if the host CPU
  ** byte-order is known at compile-time.
  */
#إذا SQLITE_MAX_LENGTH<=2147483640 && SQLITE_BYTEORDER>0
# define OVERRUN 7   /* We are able to allocate an overrun of 7 bytes */
#وإلا
# define OVERRUN 0   /* No overrun will be available */
#endif

  /* Make sure the output register has a buffer large enough to store
  ** the new record. The output register (pOp->p3) is not allowed to
  ** be one of the input registers (because the following call to
  ** sqlite3VdbeMemClearAndResize() could clobber the value before it is used).
  */
  إذا( nByte+nZero<=pOut->szMalloc-OVERRUN ){
    /* The output register is already large enough to hold the record.
    ** No error checks or buffer enlargement is required */
    pOut->z = pOut->zMalloc;
  }وإلا{
    /* Need to make sure that the output is not too big and then enlarge
    ** the output register to hold the full result */
    إذا( nByte+nZero>db->aLimit[SQLITE_LIMIT_LENGTH] ){
      اذهب too_big;
    }
    إذا( sqlite3VdbeMemClearAndResize(pOut, (صحيح)nByte+OVERRUN) ){
      اذهب no_mem;
    }
  }
  pOut->n = (صحيح)nByte;
  pOut->flags = MEM_Blob;
  إذا( nZero ){
    pOut->u.nZero = nZero;
    pOut->flags |= MEM_Zero;
  }
  UPDATE_MAX_BLOBSIZE(pOut);
  zHdr = (u8 *)pOut->z;
  zPayload = zHdr + nHdr;

  /* Write the record */
  إذا( nHdr<0x80 ){
    *(zHdr++) = nHdr;
  }وإلا{
    zHdr += sqlite3PutVarint(zHdr,nHdr);
  }
  أكد( pData0<=pLast );
  pRec = pData0;
  طالما( 1 /*exit-by-break*/ ){
    serial_type = pRec->uTemp;
    /* EVIDENCE-OF: R-06529-47362 Following the size varint are one or more
    ** additional varints, one per column.
    ** EVIDENCE-OF: R-64536-51728 The values for each column in the record
    ** immediately follow the header. */
    إذا( serial_type<=7 ){
      *(zHdr++) = serial_type;
      إذا( serial_type==0 ){
        /* NULL value.  No change in zPayload */
      }وإلا{
        u64 v;
        إذا( serial_type==7 ){
          أكد( حجم(v)==حجم(pRec->u.r) );
          memcpy(&v, &pRec->u.r, حجم(v));
          swapMixedEndianFloat(v);
        }وإلا{
          v = pRec->u.i;
        }
        len = sqlite3SmallTypeSizes[serial_type];
        أكد( len>=1 && len<=8 && len!=5 && len!=7 );
#إذا SQLITE_BYTEORDER==1234
        v = sqlite3BSwap64(v);
        إذا( OVERRUN ){
          ساكن ثابت u8 aShift[] = { 0, 56, 48, 40, 32, 16, 0, 0 };
          v >>= aShift[serial_type];
          memcpy(zPayload, &v, 8);
        }وإلا{
          /* Test this limb by compiling with -DSQLITE_MAX_LENGTH=2147483647 */
          memcpy(zPayload, (u8*)&v + 8 - len, len);
        }
#وإلاإذا SQLITE_BYTEORDER==4321
        إذا( OVERRUN ){
          ساكن ثابت u8 aShift[] = { 0, 56, 48, 40, 32, 16, 0, 0 };
          v <<= aShift[serial_type];
          memcpy(zPayload, &v, 8);
        }وإلا{
          /* Test this limb by compiling with -DSQLITE_MAX_LENGTH=2147483647 */
          memcpy(zPayload, (u8*)&v + 8 - len, len);
        }
#وإلا
        /* Test this limb by compiling with -DSQLITE_BYTEORDER=0 */
        بدل( len ){
          افتراضي: zPayload[7] = (u8)(v&0xff); v >>= 8;
                   zPayload[6] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          حالة 6:  zPayload[5] = (u8)(v&0xff); v >>= 8;
                   zPayload[4] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          حالة 4:  zPayload[3] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          حالة 3:  zPayload[2] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          حالة 2:  zPayload[1] = (u8)(v&0xff); v >>= 8;
                   /* no break */ deliberate_fall_through
          حالة 1:  zPayload[0] = (u8)(v&0xff);
        }
#endif
#undef OVERRUN  /* We are done with the OVERRUN macro now */
        zPayload += len;
      }
    }وإلا إذا( serial_type<0x80 ){
      *(zHdr++) = serial_type;
      إذا( serial_type>=14 && pRec->n>0 ){
        أكد( pRec->z!=0 );
        memcpy(zPayload, pRec->z, pRec->n);
        zPayload += pRec->n;
      }
    }وإلا{
      zHdr += sqlite3PutVarint(zHdr, serial_type);
      إذا( pRec->n ){
        أكد( pRec->z!=0 );
        أكد( pRec->z!=(ثابت حرف*)sqlite3CtypeMap );
        memcpy(zPayload, pRec->z, pRec->n);
        zPayload += pRec->n;
      }
    }
    إذا( pRec==pLast ) اكسر;
    pRec++;
  }
  أكد( nHdr==(صحيح)(zHdr - (u8*)pOut->z) );
  أكد( nByte==(صحيح)(zPayload - (u8*)pOut->z) );

  أكد( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  REGISTER_TRACE(pOp->p3, pOut);
  اكسر;
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
حالة OP_سانى: {         /* out2 */
  i64 nEntry;
  BtCursor *pCrsr;

  أكد( p->apCsr[pOp->p1]->eCurType==CURTYPE_BTREE );
  pCrsr = p->apCsr[pOp->p1]->uc.pCursor;
  أكد( pCrsr );
  إذا( pOp->p3 ){
    nEntry = sqlite3_Btree_Row_سانى_Est(pCrsr);
  }وإلا{
    nEntry = 0;  /* Not needed.  Only used to silence a warning. */
    rc = sqlite3_Btree_سانى(db, pCrsr, &nEntry);
    إذا( rc ) اذهب abort_due_to_error;
  }
  pOut = out2Prerelease(p, pOp);
  pOut->u.i = nEntry;
  اذهب check_for_interrupt;
}

/* Opcode: Savepoint P1 * * P4 *
**
** Open, release or rollback the savepoint named by parameter P4, depending
** on the value of P1. To open a new savepoint set P1==0 (SAVEPOINT_BEGIN).
** To release (commit) an existing savepoint set P1==1 (SAVEPOINT_RELEASE).
** To rollback an existing savepoint set P1==2 (SAVEPOINT_ROLLBACK).
*/
حالة OP_Savepoint: {
  صحيح p1;                         /* Value of P1 operand */
  حرف *zName;                    /* Name of savepoint */
  صحيح nName;
  Savepoint *pNew;
  Savepoint *pSavepoint;
  Savepoint *pTmp;
  صحيح iSavepoint;
  صحيح ii;

  p1 = pOp->p1;
  zName = pOp->p4.z;

  /* Assert that the p1 parameter is valid. Also that if there is no open
  ** transaction, then there cannot be any savepoints.
  */
  أكد( db->pSavepoint==0 || db->autoCommit==0 );
  أكد( p1==SAVEPOINT_BEGIN||p1==SAVEPOINT_RELEASE||p1==SAVEPOINT_ROLLBACK );
  أكد( db->pSavepoint || db->isTransactionSavepoint==0 );
  أكد( check_Savepoint_سانى(db) );
  أكد( p->bIsReader );

  إذا( p1==SAVEPOINT_BEGIN ){
    إذا( db->nVdbeWrite>0 ){
      /* A new savepoint cannot be created if there are active write
      ** statements (i.e. open read/write incremental blob handles).
      */
      sqlite3VdbeError(p, "cannot open savepoint - SQL statements in progress");
      rc = SQLITE_BUSY;
    }وإلا{
      nName = sqlite3Strlen30(zName);

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* This call is Ok even if this savepoint is actually a transaction
      ** savepoint (and therefore should not prompt xSavepoint()) callbacks.
      ** If this is a transaction savepoint being opened, it is guaranteed
      ** that the db->aVTrans[] array is empty.  */
      أكد( db->autoCommit==0 || db->nVTrans==0 );
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_BEGIN,
                                db->nStatement+db->nSavepoint);
      إذا( rc!=SQLITE_OK ) اذهب abort_due_to_error;
#endif

      /* Create a new savepoint structure. */
      pNew = sqlite3DbMallocRawNN(db, حجم(Savepoint)+nName+1);
      إذا( pNew ){
        pNew->zName = (حرف *)&pNew[1];
        memcpy(pNew->zName, zName, nName+1);
   
        /* If there is no open transaction, then mark this as a special
        ** "transaction savepoint". */
        إذا( db->autoCommit ){
          db->autoCommit = 0;
          db->isTransactionSavepoint = 1;
        }وإلا{
          db->nSavepoint++;
        }

        /* Link the new savepoint into the database handle's list. */
        pNew->pNext = db->pSavepoint;
        db->pSavepoint = pNew;
        pNew->nDeferredCons = db->nDeferredCons;
        pNew->nDeferredImmCons = db->nDeferredImmCons;
      }
    }
  }وإلا{
    أكد( p1==SAVEPOINT_RELEASE || p1==SAVEPOINT_ROLLBACK );
    iSavepoint = 0;

    /* Find the named savepoint. If there is no such savepoint, then an
    ** an error is returned to the user.  */
    لكل(
      pSavepoint = db->pSavepoint;
      pSavepoint && sqlite3StrICmp(pSavepoint->zName, zName);
      pSavepoint = pSavepoint->pNext
    ){
      iSavepoint++;
    }
    إذا( !pSavepoint ){
      sqlite3VdbeError(p, "no such savepoint: %s", zName);
      rc = SQLITE_ERROR;
    }وإلا إذا( db->nVdbeWrite>0 && p1==SAVEPOINT_RELEASE ){
      /* It is not possible to release (commit) a savepoint if there are
      ** active write statements.
      */
      sqlite3VdbeError(p, "cannot release savepoint - "
                          "SQL statements in progress");
      rc = SQLITE_BUSY;
    }وإلا{

      /* Determine whether or not this is a transaction savepoint. If so,
      ** and this is a RELEASE command, then the current transaction
      ** is committed.
      */
      صحيح isTransaction = pSavepoint->pNext==0 && db->isTransactionSavepoint;
      إذا( isTransaction && p1==SAVEPOINT_RELEASE ){
        إذا( (rc = sqlite3VdbeCheckFkDeferred(p))!=SQLITE_OK ){
          اذهب vdbe_return;
        }
        db->autoCommit = 1;
        إذا( sqlite3VdbeHalt(p)==SQLITE_BUSY ){
          p->pc = (صحيح)(pOp - aOp);
          db->autoCommit = 0;
          p->rc = rc = SQLITE_BUSY;
          اذهب vdbe_return;
        }
        rc = p->rc;
        إذا( rc ){
          db->autoCommit = 0;
        }وإلا{
          db->isTransactionSavepoint = 0;
        }
      }وإلا{
        صحيح isSchemaChange;
        iSavepoint = db->nSavepoint - iSavepoint - 1;
        إذا( p1==SAVEPOINT_ROLLBACK ){
          isSchemaChange = (db->mDbFlags & DBFLAG_SchemaChange)!=0;
          لكل(ii=0; ii<db->nDb; ii++){
            rc = sqlite3BtreeTripAllCursors(db->aDb[ii].pBt,
                                       SQLITE_ABORT_ROLLBACK,
                                       isSchemaChange==0);
            إذا( rc!=SQLITE_OK ) اذهب abort_due_to_error;
          }
        }وإلا{
          أكد( p1==SAVEPOINT_RELEASE );
          isSchemaChange = 0;
        }
        لكل(ii=0; ii<db->nDb; ii++){
          rc = sqlite3BtreeSavepoint(db->aDb[ii].pBt, p1, iSavepoint);
          إذا( rc!=SQLITE_OK ){
            اذهب abort_due_to_error;
          }
        }
        إذا( isSchemaChange ){
          sqlite3ExpirePreparedStatements(db, 0);
          sqlite3ResetAllSchemasOfConnection(db);
          db->mDbFlags |= DBFLAG_SchemaChange;
        }
      }
      إذا( rc ) اذهب abort_due_to_error;
 
      /* Regardless of whether this is a RELEASE or ROLLBACK, destroy all
      ** savepoints nested inside of the savepoint being operated on. */
      طالما( db->pSavepoint!=pSavepoint ){
        pTmp = db->pSavepoint;
        db->pSavepoint = pTmp->pNext;
        sqlite3DbFree(db, pTmp);
        db->nSavepoint--;
      }

      /* If it is a RELEASE, then destroy the savepoint being operated on
      ** too. If it is a ROLLBACK TO, then set the number of deferred
      ** constraint violations present in the database to the value stored
      ** when the savepoint was created.  */
      إذا( p1==SAVEPOINT_RELEASE ){
        أكد( pSavepoint==db->pSavepoint );
        db->pSavepoint = pSavepoint->pNext;
        sqlite3DbFree(db, pSavepoint);
        إذا( !isTransaction ){
          db->nSavepoint--;
        }
      }وإلا{
        أكد( p1==SAVEPOINT_ROLLBACK );
        db->nDeferredCons = pSavepoint->nDeferredCons;
        db->nDeferredImmCons = pSavepoint->nDeferredImmCons;
      }

      إذا( !isTransaction || p1==SAVEPOINT_ROLLBACK ){
        rc = sqlite3VtabSavepoint(db, p1, iSavepoint);
        إذا( rc!=SQLITE_OK ) اذهب abort_due_to_error;
      }
    }
  }
  إذا( rc ) اذهب abort_due_to_error;
  إذا( p->eVdbeState==VDBE_HALT_STATE ){
    rc = SQLITE_DONE;
    اذهب vdbe_return;
  }
  اكسر;
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
حالة OP_AutoCommit: {
  صحيح desiredAutoCommit;
  صحيح iRollback;

  desiredAutoCommit = pOp->p1;
  iRollback = pOp->p2;
  أكد( desiredAutoCommit==1 || desiredAutoCommit==0 );
  أكد( desiredAutoCommit==1 || iRollback==0 );
  أكد( db->nVdbeActive>0 );  /* At least this one VM is active */
  أكد( p->bIsReader );

  إذا( desiredAutoCommit!=db->autoCommit ){
    إذا( iRollback ){
      أكد( desiredAutoCommit==1 );
      sqlite3RollbackAll(db, SQLITE_ABORT_ROLLBACK);
      db->autoCommit = 1;
    }وإلا إذا( desiredAutoCommit && db->nVdbeWrite>0 ){
      /* If this instruction implements a COMMIT and other VMs are writing
      ** return an error indicating that the other VMs must complete first.
      */
      sqlite3VdbeError(p, "cannot commit transaction - "
                          "SQL statements in progress");
      rc = SQLITE_BUSY;
      اذهب abort_due_to_error;
    }وإلا إذا( (rc = sqlite3VdbeCheckFkDeferred(p))!=SQLITE_OK ){
      اذهب vdbe_return;
    }وإلا{
      db->autoCommit = (u8)desiredAutoCommit;
    }
    إذا( sqlite3VdbeHalt(p)==SQLITE_BUSY ){
      p->pc = (صحيح)(pOp - aOp);
      db->autoCommit = (u8)(1-desiredAutoCommit);
      p->rc = rc = SQLITE_BUSY;
      اذهب vdbe_return;
    }
    sqlite3CloseSavepoints(db);
    إذا( p->rc==SQLITE_OK ){
      rc = SQLITE_DONE;
    }وإلا{
      rc = SQLITE_ERROR;
    }
    اذهب vdbe_return;
  }وإلا{
    sqlite3VdbeError(p,
        (!desiredAutoCommit)?"cannot start a transaction within a transaction":(
        (iRollback)?"cannot rollback - no transaction is active":
                   "cannot commit - no transaction is active"));
        
    rc = SQLITE_ERROR;
    اذهب abort_due_to_error;
  }
  /*NOTREACHED*/ أكد(0);
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
حالة OP_Transaction: {
  Btree *pBt;
  Db *pDb;
  صحيح iMeta = 0;

  أكد( p->bIsReader );
  أكد( p->readOnly==0 || pOp->p2==0 );
  أكد( pOp->p2>=0 && pOp->p2<=2 );
  أكد( pOp->p1>=0 && pOp->p1<db->nDb );
  أكد( DbMaskTest(p->btreeMask, pOp->p1) );
  أكد( rc==SQLITE_OK );
  إذا( pOp->p2 && (db->flags & (SQLITE_QueryOnly|SQLITE_CorruptRdOnly))!=0 ){
    إذا( db->flags & SQLITE_QueryOnly ){
      /* Writes prohibited by the "PRAGMA query_only=TRUE" statement */
      rc = SQLITE_READONLY;
    }وإلا{
      /* Writes prohibited due to a prior SQLITE_CORRUPT in the current
      ** transaction */
      rc = SQLITE_CORRUPT;
    }
    اذهب abort_due_to_error;
  }
  pDb = &db->aDb[pOp->p1];
  pBt = pDb->pBt;

  إذا( pBt ){
    rc = sqlite3BtreeBeginTrans(pBt, pOp->p2, &iMeta);
    testcase( rc==SQLITE_BUSY_SNAPSHOT );
    testcase( rc==SQLITE_BUSY_RECOVERY );
    إذا( rc!=SQLITE_OK ){
      إذا( (rc&0xff)==SQLITE_BUSY ){
        p->pc = (صحيح)(pOp - aOp);
        p->rc = rc;
        اذهب vdbe_return;
      }
      اذهب abort_due_to_error;
    }

    إذا( p->usesStmtJournal
     && pOp->p2
     && (db->autoCommit==0 || db->nVdbeRead>1)
    ){
      أكد( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE );
      إذا( p->iStatement==0 ){
        أكد( db->nStatement>=0 && db->nSavepoint>=0 );
        db->nStatement++;
        p->iStatement = db->nSavepoint + db->nStatement;
      }

      rc = sqlite3VtabSavepoint(db, SAVEPOINT_BEGIN, p->iStatement-1);
      إذا( rc==SQLITE_OK ){
        rc = sqlite3BtreeBeginStmt(pBt, p->iStatement);
      }

      /* Store the current value of the database handles deferred constraint
      ** counter. If the statement transaction needs to be rolled back,
      ** the value of this counter needs to be restored too.  */
      p->nStmtDefCons = db->nDeferredCons;
      p->nStmtDefImmCons = db->nDeferredImmCons;
    }
  }
  أكد( pOp->p5==0 || pOp->p4type==P4_INT32 );
  إذا( rc==SQLITE_OK
   && pOp->p5
   && (iMeta!=pOp->p3 || pDb->pSchema->iGeneration!=pOp->p4.i)
  ){
    /*
    ** IMPLEMENTATION-OF: R-03189-51135 As each SQL statement runs, the schema
    ** version is checked to ensure that the schema has not changed since the
    ** SQL statement was prepared.
    */
    sqlite3DbFree(db, p->z_Err_ئۇچۇر);
    p->z_Err_ئۇچۇر = sqlite3DbStrDup(db, "database schema has changed");
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
    إذا( db->aDb[pOp->p1].pSchema->schema_cookie!=iMeta ){
      sqlite3ResetOneSchema(db, pOp->p1);
    }
    p->expired = 1;
    rc = SQLITE_SCHEMA;

    /* Set changeCntOn to 0 to prevent the value returned by sqlite3_changes()
    ** from being modified in sqlite3VdbeHalt(). If this statement is
    ** reprepared, changeCntOn will be set again. */
    p->changeCntOn = 0;
  }
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
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
حالة OP_ReadCookie: {               /* out2 */
  صحيح iMeta;
  صحيح iDb;
  صحيح iCookie;

  أكد( p->bIsReader );
  iDb = pOp->p1;
  iCookie = pOp->p3;
  أكد( pOp->p3<SQLITE_N_BTREE_META );
  أكد( iDb>=0 && iDb<db->nDb );
  أكد( db->aDb[iDb].pBt!=0 );
  أكد( DbMaskTest(p->btreeMask, iDb) );

  sqlite3BtreeGetMeta(db->aDb[iDb].pBt, iCookie, (u32 *)&iMeta);
  pOut = out2Prerelease(p, pOp);
  pOut->u.i = iMeta;
  اكسر;
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
حالة OP_SetCookie: {
  Db *pDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  أكد( pOp->p2<SQLITE_N_BTREE_META );
  أكد( pOp->p1>=0 && pOp->p1<db->nDb );
  أكد( DbMaskTest(p->btreeMask, pOp->p1) );
  أكد( p->readOnly==0 );
  pDb = &db->aDb[pOp->p1];
  أكد( pDb->pBt!=0 );
  أكد( sqlite3SchemaMutexHeld(db, pOp->p1, 0) );
  /* See note about index shifting on OP_ReadCookie */
  rc = sqlite3BtreeUpdateMeta(pDb->pBt, pOp->p2, pOp->p3);
  إذا( pOp->p2==BTREE_SCHEMA_VERSION ){
    /* When the schema cookie changes, record the new cookie internally */
    *(u32*)&pDb->pSchema->schema_cookie = *(u32*)&pOp->p3 - pOp->p5;
    db->mDbFlags |= DBFLAG_SchemaChange;
    sqlite3FkClearTriggerCache(db, pOp->p1);
  }وإلا إذا( pOp->p2==BTREE_FILE_FORMAT ){
    /* Record changes in the file format */
    pDb->pSchema->file_format = pOp->p3;
  }
  إذا( pOp->p1==1 ){
    /* Invalidate all prepared statements whenever the TEMP database
    ** schema is changed.  Ticket #1644 */
    sqlite3ExpirePreparedStatements(db, 0);
    p->expired = 0;
  }
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
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
حالة OP_ReopenIdx: {         /* ncycle */
  صحيح nField;
  ئاچقۇچ_Info *p_ئاچقۇچ_Info;
  u32 p2;
  صحيح iDb;
  صحيح wrFlag;
  Btree *pX;
  VdbeCursor *pCur;
  Db *pDb;

  أكد( pOp->p5==0 || pOp->p5==OPFLAG_SEEKEQ );
  أكد( pOp->p4type==P4_KEYINFO );
  pCur = p->apCsr[pOp->p1];
  إذا( pCur && pCur->pgno_يىلتىز==(u32)pOp->p2 ){
    أكد( pCur->iDb==pOp->p3 );      /* Guaranteed by the code generator */
    أكد( pCur->eCurType==CURTYPE_BTREE );
    sqlite3BtreeClearCursor(pCur->uc.pCursor);
    اذهب open_cursor_set_hints;
  }
  /* If the cursor is not currently open or is open on a different
  ** index, then fall through into OP_OpenRead to force a reopen */
حالة OP_OpenRead:            /* ncycle */
حالة OP_OpenWrite:

  أكد( pOp->opcode==OP_OpenWrite || pOp->p5==0 || pOp->p5==OPFLAG_SEEKEQ );
  أكد( p->bIsReader );
  أكد( pOp->opcode==OP_OpenRead || pOp->opcode==OP_ReopenIdx
          || p->readOnly==0 );

  إذا( p->expired==1 ){
    rc = SQLITE_ABORT_ROLLBACK;
    اذهب abort_due_to_error;
  }

  nField = 0;
  p_ئاچقۇچ_Info = 0;
  p2 = (u32)pOp->p2;
  iDb = pOp->p3;
  أكد( iDb>=0 && iDb<db->nDb );
  أكد( DbMaskTest(p->btreeMask, iDb) );
  pDb = &db->aDb[iDb];
  pX = pDb->pBt;
  أكد( pX!=0 );
  إذا( pOp->opcode==OP_OpenWrite ){
    أكد( OPFLAG_FORDELETE==BTREE_FORDELETE );
    wrFlag = BTREE_WRCSR | (pOp->p5 & OPFLAG_FORDELETE);
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    إذا( pDb->pSchema->file_format < p->minWriteFileFormat ){
      p->minWriteFileFormat = pDb->pSchema->file_format;
    }
    إذا( pOp->p5 & OPFLAG_P2ISREG ){
      أكد( p2>0 );
      أكد( p2<=(u32)(p->nMem+1 - p->nCursor) );
      pIn2 = &aMem[p2];
      أكد( memIsValid(pIn2) );
      أكد( (pIn2->flags & MEM_Int)!=0 );
      sqlite3VdbeMemIntegerify(pIn2);
      p2 = (صحيح)pIn2->u.i;
      /* The p2 value always comes from a prior OP_CreateBtree opcode and
      ** that opcode will always set the p2 value to 2 or more or else fail.
      ** If there were a failure, the prepared statement would have halted
      ** before reaching this instruction. */
      أكد( p2>=2 );
    }
  }وإلا{
    wrFlag = 0;
    أكد( (pOp->p5 & OPFLAG_P2ISREG)==0 );
  }
  إذا( pOp->p4type==P4_KEYINFO ){
    p_ئاچقۇچ_Info = pOp->p4.p_ئاچقۇچ_Info;
    أكد( p_ئاچقۇچ_Info->enc==ENC(db) );
    أكد( p_ئاچقۇچ_Info->db==db );
    nField = p_ئاچقۇچ_Info->nAllField;
  }وإلا إذا( pOp->p4type==P4_INT32 ){
    nField = pOp->p4.i;
  }
  أكد( pOp->p1>=0 );
  أكد( nField>=0 );
  testcase( nField==0 );  /* Table with INTEGER PRIMARY KEY and nothing else */
  pCur = allocateCursor(p, pOp->p1, nField, CURTYPE_BTREE);
  إذا( pCur==0 ) اذهب no_mem;
  pCur->iDb = iDb;
  pCur->nullRow = 1;
  pCur->isOrdered = 1;
  pCur->pgno_يىلتىز = p2;
#ifdef SQLITE_DEBUG
  pCur->wrFlag = wrFlag;
#endif
  rc = sqlite3BtreeCursor(pX, p2, wrFlag, p_ئاچقۇچ_Info, pCur->uc.pCursor);
  pCur->p_ئاچقۇچ_Info = p_ئاچقۇچ_Info;
  /* Set the VdbeCursor.isTable variable. Previous versions of
  ** SQLite used to check if the root-page flags were sane at this point
  ** and report database corruption if they were not, but this check has
  ** since moved into the btree layer.  */ 
  pCur->isTable = pOp->p4type!=P4_KEYINFO;

open_cursor_set_hints:
  أكد( OPFLAG_BULKCSR==BTREE_BULKLOAD );
  أكد( OPFLAG_SEEKEQ==BTREE_SEEK_EQ );
  testcase( pOp->p5 & OPFLAG_BULKCSR );
  testcase( pOp->p2 & OPFLAG_SEEKEQ );
  sqlite3BtreeCursorHintFlags(pCur->uc.pCursor,
                               (pOp->p5 & (OPFLAG_BULKCSR|OPFLAG_SEEKEQ)));
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}

/* Opcode: OpenDup P1 P2 * * *
**
** Open a new cursor P1 that points to the same ephemeral table as
** cursor P2.  The P2 cursor must have been opened by a prior OP_OpenEphemeral
** opcode.  Only ephemeral cursors may be duplicated.
**
** Duplicate ephemeral cursors are used for self-joins of materialized views.
*/
حالة OP_OpenDup: {           /* ncycle */
  VdbeCursor *pOrig;    /* The original cursor to be duplicated */
  VdbeCursor *pCx;      /* The new cursor */

  pOrig = p->apCsr[pOp->p2];
  أكد( pOrig );
  أكد( pOrig->isEphemeral );  /* Only ephemeral cursors can be duplicated */

  pCx = allocateCursor(p, pOp->p1, pOrig->nField, CURTYPE_BTREE);
  إذا( pCx==0 ) اذهب no_mem;
  pCx->nullRow = 1;
  pCx->isEphemeral = 1;
  pCx->p_ئاچقۇچ_Info = pOrig->p_ئاچقۇچ_Info;
  pCx->isTable = pOrig->isTable;
  pCx->pgno_يىلتىز = pOrig->pgno_يىلتىز;
  pCx->isOrdered = pOrig->isOrdered;
  pCx->ub.pBtx = pOrig->ub.pBtx;
  pCx->noReuse = 1;
  pOrig->noReuse = 1;
  rc = sqlite3BtreeCursor(pCx->ub.pBtx, pCx->pgno_يىلتىز, BTREE_WRCSR,
                          pCx->p_ئاچقۇچ_Info, pCx->uc.pCursor);
  /* The sqlite3BtreeCursor() routine can only fail for the first cursor
  ** opened for a database.  Since there is already an open cursor when this
  ** opcode is run, the sqlite3BtreeCursor() cannot fail */
  أكد( rc==SQLITE_OK );
  اكسر;
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
حالة OP_OpenAutoindex:       /* ncycle */
حالة OP_OpenEphemeral: {     /* ncycle */
  VdbeCursor *pCx;
  ئاچقۇچ_Info *p_ئاچقۇچ_Info;

  ساكن ثابت صحيح vfsFlags =
      SQLITE_OPEN_READWRITE |
      SQLITE_OPEN_CREATE |
      SQLITE_OPEN_EXCLUSIVE |
      SQLITE_OPEN_DELETEONCLOSE |
      SQLITE_OPEN_TRANSIENT_DB;
  أكد( pOp->p1>=0 );
  أكد( pOp->p2>=0 );
  إذا( pOp->p3>0 ){
    /* Make register reg[P3] into a value that can be used as the data
    ** form sqlite3BtreeInsert() where the length of the data is zero. */
    أكد( pOp->p2==0 ); /* Only used when number of columns is zero */
    أكد( pOp->opcode==OP_OpenEphemeral );
    أكد( aMem[pOp->p3].flags & MEM_Null );
    aMem[pOp->p3].n = 0;
    aMem[pOp->p3].z = "";
  }
  pCx = p->apCsr[pOp->p1];
  إذا( pCx && !pCx->noReuse &&  ALWAYS(pOp->p2<=pCx->nField) ){
    /* If the ephemeral table is already open and has no duplicates from
    ** OP_OpenDup, then erase all existing content so that the table is
    ** empty again, rather than creating a new table. */
    أكد( pCx->isEphemeral );
    pCx->seq_سانى = 0;
    pCx->cacheStatus = CACHE_STALE;
    rc = sqlite3BtreeClearTable(pCx->ub.pBtx, pCx->pgno_يىلتىز, 0);
  }وإلا{
    pCx = allocateCursor(p, pOp->p1, pOp->p2, CURTYPE_BTREE);
    إذا( pCx==0 ) اذهب no_mem;
    pCx->isEphemeral = 1;
    rc = sqlite3BtreeOpen(db->pVfs, 0, db, &pCx->ub.pBtx,
                          BTREE_OMIT_JOURNAL | BTREE_SINGLE | pOp->p5,
                          vfsFlags);
    إذا( rc==SQLITE_OK ){
      rc = sqlite3BtreeBeginTrans(pCx->ub.pBtx, 1, 0);
      إذا( rc==SQLITE_OK ){
        /* If a transient index is required, create it by calling
        ** sqlite3BtreeCreateTable() with the BTREE_BLOBKEY flag before
        ** opening it. If a transient table is required, just use the
        ** automatically created table with root-page 1 (an BLOB_INTKEY table).
        */
        إذا( (pCx->p_ئاچقۇچ_Info = p_ئاچقۇچ_Info = pOp->p4.p_ئاچقۇچ_Info)!=0 ){
          أكد( pOp->p4type==P4_KEYINFO );
          rc = sqlite3BtreeCreateTable(pCx->ub.pBtx, &pCx->pgno_يىلتىز,
              BTREE_BLOBKEY | pOp->p5);
          إذا( rc==SQLITE_OK ){
            أكد( pCx->pgno_يىلتىز==SCHEMA_يىلتىز+1 );
            أكد( p_ئاچقۇچ_Info->db==db );
            أكد( p_ئاچقۇچ_Info->enc==ENC(db) );
            rc = sqlite3BtreeCursor(pCx->ub.pBtx, pCx->pgno_يىلتىز, BTREE_WRCSR,
                p_ئاچقۇچ_Info, pCx->uc.pCursor);
          }
          pCx->isTable = 0;
        }وإلا{
          pCx->pgno_يىلتىز = SCHEMA_يىلتىز;
          rc = sqlite3BtreeCursor(pCx->ub.pBtx, SCHEMA_يىلتىز, BTREE_WRCSR,
              0, pCx->uc.pCursor);
          pCx->isTable = 1;
        }
      }
      pCx->isOrdered = (pOp->p5!=BTREE_UNORDERED);
      أكد( p->apCsr[pOp->p1]==pCx );
      إذا( rc ){
        أكد( !sqlite3BtreeClosesWithCursor(pCx->ub.pBtx, pCx->uc.pCursor) );
        sqlite3BtreeClose(pCx->ub.pBtx);
        p->apCsr[pOp->p1] = 0;  /* Not required; helps with static analysis */
      }وإلا{
        أكد( sqlite3BtreeClosesWithCursor(pCx->ub.pBtx, pCx->uc.pCursor) );
        إذا( pOp->opcode==OP_OpenAutoindex ){
          sqlite3BtreeCursorHintFlags(pCx->uc.pCursor, BTREE_SEEK_EQ);
        }
      }
    }
  }
  إذا( rc ) اذهب abort_due_to_error;
  pCx->nullRow = 1;
  اكسر;
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
حالة OP_SorterOpen: {
  VdbeCursor *pCx;

  أكد( pOp->p1>=0 );
  أكد( pOp->p2>=0 );
  pCx = allocateCursor(p, pOp->p1, pOp->p2, CURTYPE_SORTER);
  إذا( pCx==0 ) اذهب no_mem;
  pCx->p_ئاچقۇچ_Info = pOp->p4.p_ئاچقۇچ_Info;
  أكد( pCx->p_ئاچقۇچ_Info->db==db );
  أكد( pCx->p_ئاچقۇچ_Info->enc==ENC(db) );
  rc = sqlite3VdbeSorterInit(db, pOp->p3, pCx);
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}

/* Opcode: SequenceTest P1 P2 * * *
** Synopsis: if( cursor[P1].ctr++ ) pc = P2
**
** P1 is a sorter cursor. If the sequence counter is currently zero, jump
** to P2. Regardless of whether or not the jump is taken, increment the
** the sequence value.
*/
حالة OP_SequenceTest: {
  VdbeCursor *pC;
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( isSorter(pC) );
  إذا( (pC->seq_سانى++)==0 ){
    اذهب jump_to_p2;
  }
  اكسر;
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
حالة OP_OpenPseudo: {
  VdbeCursor *pCx;

  أكد( pOp->p1>=0 );
  أكد( pOp->p3>=0 );
  pCx = allocateCursor(p, pOp->p1, pOp->p3, CURTYPE_PSEUDO);
  إذا( pCx==0 ) اذهب no_mem;
  pCx->nullRow = 1;
  pCx->seekResult = pOp->p2;
  pCx->isTable = 1;
  /* Give this pseudo-cursor a fake BtCursor pointer so that pCx
  ** can be safely passed to sqlite3VdbeCursorMoveto().  This avoids a test
  ** for pCx->eCurType==CURTYPE_BTREE inside of sqlite3VdbeCursorMoveto()
  ** which is a performance optimization */
  pCx->uc.pCursor = sqlite3BtreeFakeValidCursor();
  أكد( pOp->p5==0 );
  اكسر;
}

/* Opcode: Close P1 * * * *
**
** Close a cursor previously opened as P1.  If P1 is not
** currently open, this instruction is a no-op.
*/
حالة OP_Close: {             /* ncycle */
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  sqlite3VdbeFreeCursor(p, p->apCsr[pOp->p1]);
  p->apCsr[pOp->p1] = 0;
  اكسر;
}

#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
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
حالة OP_ColumnsUsed: {
  VdbeCursor *pC;
  pC = p->apCsr[pOp->p1];
  أكد( pC->eCurType==CURTYPE_BTREE );
  pC->maskUsed = INT32_TO_64(pOp->p2,pOp->p3);
  اكسر;
}
#endif

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
حالة OP_SeekLT:         /* jump0, in3, group, ncycle */
حالة OP_SeekLE:         /* jump0, in3, group, ncycle */
حالة OP_SeekGE:         /* jump0, in3, group, ncycle */
حالة OP_SeekGT: {       /* jump0, in3, group, ncycle */
  صحيح res;           /* Comparison result */
  صحيح oc;            /* Opcode */
  VdbeCursor *pC;    /* The cursor to seek */
  UnpackedRecord r;  /* The key to seek for */
  صحيح nField;        /* Number of columns or fields in the key */
  i64 i_ئاچقۇچ;          /* The rowid we are to seek to */
  صحيح eqOnly;        /* Only interested in == results */

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( pOp->p2!=0 );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( OP_SeekLE == OP_SeekLT+1 );
  أكد( OP_SeekGE == OP_SeekLT+2 );
  أكد( OP_SeekGT == OP_SeekLT+3 );
  أكد( pC->isOrdered );
  أكد( pC->uc.pCursor!=0 );
  oc = pOp->opcode;
  eqOnly = 0;
  pC->nullRow = 0;
#ifdef SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#endif

  pC->deferredMoveto = 0;
  pC->cacheStatus = CACHE_STALE;
  إذا( pC->isTable ){
    u16 flags3, newType;
    /* The OPFLAG_SEEKEQ/BTREE_SEEK_EQ flag is only set on index cursors */
    أكد( sqlite3BtreeCursorHasHint(pC->uc.pCursor, BTREE_SEEK_EQ)==0
              || CORRUPT_DB );

    /* The input value in P3 might be of any type: integer, real, string,
    ** blob, or NULL.  But it needs to be an integer before we can do
    ** the seek, so convert it. */
    pIn3 = &aMem[pOp->p3];
    flags3 = pIn3->flags;
    إذا( (flags3 & (MEM_Int|MEM_Real|MEM_IntReal|MEM_Str))==MEM_Str ){
      applyNumericAffinity(pIn3, 0);
    }
    i_ئاچقۇچ = sqlite3_Vdbe_Int_قىممەت(pIn3); /* Get the integer key value */
    newType = pIn3->flags; /* Record the type after applying numeric affinity */
    pIn3->flags = flags3;  /* But convert the type back to its original */

    /* If the P3 value could not be converted into an integer without
    ** loss of information, then special processing is required... */
    إذا( (newType & (MEM_Int|MEM_IntReal))==0 ){
      صحيح c;
      إذا( (newType & MEM_Real)==0 ){
        إذا( (newType & MEM_Null) || oc>=OP_SeekGE ){
          VdbeBranchTaken(1,2);
          اذهب jump_to_p2;
        }وإلا{
          rc = sqlite3BtreeLast(pC->uc.pCursor, &res);
          إذا( rc!=SQLITE_OK ) اذهب abort_due_to_error;
          اذهب seek_not_found;
        }
      }
      c = sqlite3IntFloatCompare(i_ئاچقۇچ, pIn3->u.r);

      /* If the approximation iKey is larger than the actual real search
      ** term, substitute >= for > and < for <=. e.g. if the search term
      ** is 4.9 and the integer approximation 5:
      **
      **        (x >  4.9)    ->     (x >= 5)
      **        (x <= 4.9)    ->     (x <  5)
      */
      إذا( c>0 ){
        أكد( OP_SeekGE==(OP_SeekGT-1) );
        أكد( OP_SeekLT==(OP_SeekLE-1) );
        أكد( (OP_SeekLE & 0x0001)==(OP_SeekGT & 0x0001) );
        إذا( (oc & 0x0001)==(OP_SeekGT & 0x0001) ) oc--;
      }

      /* If the approximation iKey is smaller than the actual real search
      ** term, substitute <= for < and > for >=.  */
      وإلا إذا( c<0 ){
        أكد( OP_SeekLE==(OP_SeekLT+1) );
        أكد( OP_SeekGT==(OP_SeekGE+1) );
        أكد( (OP_SeekLT & 0x0001)==(OP_SeekGE & 0x0001) );
        إذا( (oc & 0x0001)==(OP_SeekLT & 0x0001) ) oc++;
      }
    }
    rc = sqlite3BtreeTableMoveto(pC->uc.pCursor, (u64)i_ئاچقۇچ, 0, &res);
    pC->movetoTarget = i_ئاچقۇچ;  /* Used by OP_Delete */
    إذا( rc!=SQLITE_OK ){
      اذهب abort_due_to_error;
    }
  }وإلا{
    /* For a cursor with the OPFLAG_SEEKEQ/BTREE_SEEK_EQ hint, only the
    ** OP_SeekGE and OP_SeekLE opcodes are allowed, and these must be
    ** immediately followed by an OP_IdxGT or OP_IdxLT opcode, respectively,
    ** with the same key.
    */
    إذا( sqlite3BtreeCursorHasHint(pC->uc.pCursor, BTREE_SEEK_EQ) ){
      eqOnly = 1;
      أكد( pOp->opcode==OP_SeekGE || pOp->opcode==OP_SeekLE );
      أكد( pOp[1].opcode==OP_IdxLT || pOp[1].opcode==OP_IdxGT );
      أكد( pOp->opcode==OP_SeekGE || pOp[1].opcode==OP_IdxLT );
      أكد( pOp->opcode==OP_SeekLE || pOp[1].opcode==OP_IdxGT );
      أكد( pOp[1].p1==pOp[0].p1 );
      أكد( pOp[1].p2<=pOp[0].p2 );
      testcase( pOp->opcode==OP_SeekGE && pOp[1].p2<pOp[0].p2 );
      testcase( pOp->opcode==OP_SeekLE && pOp[1].p2<pOp[0].p2 );
      أكد( pOp[1].p3==pOp[0].p3 );
      أكد( pOp[1].p4.i==pOp[0].p4.i );
    }

    nField = pOp->p4.i;
    أكد( pOp->p4type==P4_INT32 );
    أكد( nField>0 );
    r.p_ئاچقۇچ_Info = pC->p_ئاچقۇچ_Info;
    r.nField = (u16)nField;

    /* The next line of code computes as follows, only faster:
    **   if( oc==OP_SeekGT || oc==OP_SeekLE ){
    **     r.default_rc = -1;
    **   }else{
    **     r.default_rc = +1;
    **   }
    */
    r.default_rc = ((1 & (oc - OP_SeekLT)) ? -1 : +1);
    أكد( oc!=OP_SeekGT || r.default_rc==-1 );
    أكد( oc!=OP_SeekLE || r.default_rc==-1 );
    أكد( oc!=OP_SeekGE || r.default_rc==+1 );
    أكد( oc!=OP_SeekLT || r.default_rc==+1 );

    r.aMem = &aMem[pOp->p3];
#ifdef SQLITE_DEBUG
    {
      صحيح i;
      لكل(i=0; i<r.nField; i++){
        أكد( memIsValid(&r.aMem[i]) );
        إذا( i>0 ) REGISTER_TRACE(pOp->p3+i, &r.aMem[i]);
      }
    }
#endif
    r.eqSeen = 0;
    rc = sqlite3BtreeIndexMoveto(pC->uc.pCursor, &r, &res);
    إذا( rc!=SQLITE_OK ){
      اذهب abort_due_to_error;
    }
    إذا( eqOnly && r.eqSeen==0 ){
      أكد( res!=0 );
      اذهب seek_not_found;
    }
  }
#ifdef SQLITE_TEST
  sqlite3_ئىزدەش_سانى++;
#endif
  إذا( oc>=OP_SeekGE ){  أكد( oc==OP_SeekGE || oc==OP_SeekGT );
    إذا( res<0 || (res==0 && oc==OP_SeekGT) ){
      res = 0;
      rc = sqlite3BtreeNext(pC->uc.pCursor, 0);
      إذا( rc!=SQLITE_OK ){
        إذا( rc==SQLITE_DONE ){
          rc = SQLITE_OK;
          res = 1;
        }وإلا{
          اذهب abort_due_to_error;
        }
      }
    }وإلا{
      res = 0;
    }
  }وإلا{
    أكد( oc==OP_SeekLT || oc==OP_SeekLE );
    إذا( res>0 || (res==0 && oc==OP_SeekLT) ){
      res = 0;
      rc = sqlite3BtreePrevious(pC->uc.pCursor, 0);
      إذا( rc!=SQLITE_OK ){
        إذا( rc==SQLITE_DONE ){
          rc = SQLITE_OK;
          res = 1;
        }وإلا{
          اذهب abort_due_to_error;
        }
      }
    }وإلا{
      /* res might be negative because the table is empty.  Check to
      ** see if this is the case.
      */
      res = sqlite3BtreeEof(pC->uc.pCursor);
    }
  }
seek_not_found:
  أكد( pOp->p2>0 );
  VdbeBranchTaken(res!=0,2);
  إذا( res ){
    اذهب jump_to_p2;
  }وإلا إذا( eqOnly ){
    أكد( pOp[1].opcode==OP_IdxLT || pOp[1].opcode==OP_IdxGT );
    pOp++; /* Skip the OP_IdxLt or OP_IdxGT that follows */
  }
  اكسر;
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
حالة OP_SeekScan: {          /* ncycle */
  VdbeCursor *pC;
  صحيح res;
  صحيح nStep;
  UnpackedRecord r;

  أكد( pOp[1].opcode==OP_SeekGE );

  /* If pOp->p5 is clear, then pOp->p2 points to the first instruction past the
  ** OP_IdxGT that follows the OP_SeekGE. Otherwise, it points to the first
  ** opcode past the OP_SeekGE itself.  */
  أكد( pOp->p2>=(صحيح)(pOp-aOp)+2 );
#ifdef SQLITE_DEBUG
  إذا( pOp->p5==0 ){
    /* There are no inequality constraints following the IN constraint. */
    أكد( pOp[1].p1==aOp[pOp->p2-1].p1 );
    أكد( pOp[1].p2==aOp[pOp->p2-1].p2 );
    أكد( pOp[1].p3==aOp[pOp->p2-1].p3 );
    أكد( aOp[pOp->p2-1].opcode==OP_IdxGT
         || aOp[pOp->p2-1].opcode==OP_IdxGE );
    testcase( aOp[pOp->p2-1].opcode==OP_IdxGE );
  }وإلا{
    /* There are inequality constraints.  */
    أكد( pOp->p2==(صحيح)(pOp-aOp)+2 );
    أكد( aOp[pOp->p2-1].opcode==OP_SeekGE );
  }
#endif

  أكد( pOp->p1>0 );
  pC = p->apCsr[pOp[1].p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( !pC->isTable );
  إذا( !sqlite3BtreeCursorIsValidNN(pC->uc.pCursor) ){
#ifdef SQLITE_DEBUG
     إذا( db->flags&SQLITE_VdbeTrace ){
       printf("... cursor not valid - fall through\n");
     }       
#endif
    اكسر;
  }
  nStep = pOp->p1;
  أكد( nStep>=1 );
  r.p_ئاچقۇچ_Info = pC->p_ئاچقۇچ_Info;
  r.nField = (u16)pOp[1].p4.i;
  r.default_rc = 0;
  r.aMem = &aMem[pOp[1].p3];
#ifdef SQLITE_DEBUG
  {
    صحيح i;
    لكل(i=0; i<r.nField; i++){
      أكد( memIsValid(&r.aMem[i]) );
      REGISTER_TRACE(pOp[1].p3+i, &aMem[pOp[1].p3+i]);
    }
  }
#endif
  res = 0;  /* Not needed.  Only used to silence a warning. */
  طالما(1){
    rc = sqlite3_Vdbe_Idx_ئاچقۇچ_Compare(db, pC, &r, &res);
    إذا( rc ) اذهب abort_due_to_error;
    إذا( res>0 && pOp->p5==0 ){
      seekscan_ئىزدەش_fail:
      /* Jump to SeekGE.P2, ending the loop */
#ifdef SQLITE_DEBUG
      إذا( db->flags&SQLITE_VdbeTrace ){
        printf("... %d steps and then skip\n", pOp->p1 - nStep);
      }       
#endif
      VdbeBranchTaken(1,3);
      pOp++;
      اذهب jump_to_p2;
    }
    إذا( res>=0 ){
      /* Jump to This.P2, bypassing the OP_SeekGE opcode */
#ifdef SQLITE_DEBUG
      إذا( db->flags&SQLITE_VdbeTrace ){
        printf("... %d steps and then success\n", pOp->p1 - nStep);
      }       
#endif
      VdbeBranchTaken(2,3);
      اذهب jump_to_p2;
      اكسر;
    }
    إذا( nStep<=0 ){
#ifdef SQLITE_DEBUG
      إذا( db->flags&SQLITE_VdbeTrace ){
        printf("... fall through after %d steps\n", pOp->p1);
      }       
#endif
      VdbeBranchTaken(0,3);
      اكسر;
    }
    nStep--;
    pC->cacheStatus = CACHE_STALE;
    rc = sqlite3BtreeNext(pC->uc.pCursor, 0);
    إذا( rc ){
      إذا( rc==SQLITE_DONE ){
        rc = SQLITE_OK;
        اذهب seekscan_ئىزدەش_fail;
      }وإلا{
        اذهب abort_due_to_error;
      }
    }
  }
 
  اكسر;
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
حالة OP_SeekHit: {           /* ncycle */
  VdbeCursor *pC;
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pOp->p3>=pOp->p2 );
  إذا( pC->seekHit<pOp->p2 ){
#ifdef SQLITE_DEBUG
    إذا( db->flags&SQLITE_VdbeTrace ){
      printf("seekHit changes from %d to %d\n", pC->seekHit, pOp->p2);
    }       
#endif
    pC->seekHit = pOp->p2;
  }وإلا إذا( pC->seekHit>pOp->p3 ){
#ifdef SQLITE_DEBUG
    إذا( db->flags&SQLITE_VdbeTrace ){
      printf("seekHit changes from %d to %d\n", pC->seekHit, pOp->p3);
    }       
#endif
    pC->seekHit = pOp->p3;
  }
  اكسر;
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
حالة OP_IfNoHope: {     /* jump, in3, ncycle */
  VdbeCursor *pC;
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
#ifdef SQLITE_DEBUG
  إذا( db->flags&SQLITE_VdbeTrace ){
    printf("seekHit is %d\n", pC->seekHit);
  }       
#endif
  إذا( pC->seekHit>=pOp->p4.i ) اكسر;
  /* Fall through into OP_NotFound */
  /* no break */ deliberate_fall_through
}
حالة OP_NoConflict:     /* jump, in3, ncycle */
حالة OP_NotFound:       /* jump, in3, ncycle */
حالة OP_Found: {        /* jump, in3, ncycle */
  صحيح alreadyExists;
  صحيح ii;
  VdbeCursor *pC;
  UnpackedRecord *p_Idx_ئاچقۇچ;
  UnpackedRecord r;

#ifdef SQLITE_TEST
  إذا( pOp->opcode!=OP_NoConflict ) sqlite3_found_سانى++;
#endif

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( pOp->p4type==P4_INT32 );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
#ifdef SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#endif
  r.aMem = &aMem[pOp->p3];
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->uc.pCursor!=0 );
  أكد( pC->isTable==0 );
  r.nField = (u16)pOp->p4.i;
  إذا( r.nField>0 ){
    /* Key values in an array of registers */
    r.p_ئاچقۇچ_Info = pC->p_ئاچقۇچ_Info;
    r.default_rc = 0;
#ifdef SQLITE_DEBUG
    (فراغ)sqlite3FaultSim(50);  /* For use by --counter in TH3 */
    لكل(ii=0; ii<r.nField; ii++){
      أكد( memIsValid(&r.aMem[ii]) );
      أكد( (r.aMem[ii].flags & MEM_Zero)==0 || r.aMem[ii].n==0 );
      إذا( ii ) REGISTER_TRACE(pOp->p3+ii, &r.aMem[ii]);
    }
#endif
    rc = sqlite3BtreeIndexMoveto(pC->uc.pCursor, &r, &pC->seekResult);
  }وإلا{
    /* Composite key generated by OP_MakeRecord */
    أكد( r.aMem->flags & MEM_Blob );
    أكد( pOp->opcode!=OP_NoConflict );
    rc = ExpandBlob(r.aMem);
    أكد( rc==SQLITE_OK || rc==SQLITE_NOMEM );
    إذا( rc ) اذهب no_mem;
    p_Idx_ئاچقۇچ = sqlite3VdbeAllocUnpackedRecord(pC->p_ئاچقۇچ_Info);
    إذا( p_Idx_ئاچقۇچ==0 ) اذهب no_mem;
    sqlite3VdbeRecordUnpack(r.aMem->n, r.aMem->z, p_Idx_ئاچقۇچ);
    p_Idx_ئاچقۇچ->default_rc = 0;
    rc = sqlite3BtreeIndexMoveto(pC->uc.pCursor, p_Idx_ئاچقۇچ, &pC->seekResult);
    sqlite3DbFreeNN(db, p_Idx_ئاچقۇچ);
  }
  إذا( rc!=SQLITE_OK ){
    اذهب abort_due_to_error;
  }
  alreadyExists = (pC->seekResult==0);
  pC->nullRow = 1-alreadyExists;
  pC->deferredMoveto = 0;
  pC->cacheStatus = CACHE_STALE;
  إذا( pOp->opcode==OP_Found ){
    VdbeBranchTaken(alreadyExists!=0,2);
    إذا( alreadyExists ) اذهب jump_to_p2;
  }وإلا{
    إذا( !alreadyExists ){
      VdbeBranchTaken(1,2);
      اذهب jump_to_p2;
    }
    إذا( pOp->opcode==OP_NoConflict ){
      /* For the OP_NoConflict opcode, take the jump if any of the
      ** input fields are NULL, since any key with a NULL will not
      ** conflict */
      لكل(ii=0; ii<r.nField; ii++){
        إذا( r.aMem[ii].flags & MEM_Null ){
          VdbeBranchTaken(1,2);
          اذهب jump_to_p2;
        }
      }
    }
    VdbeBranchTaken(0,2);
    إذا( pOp->opcode==OP_IfNoHope ){
      pC->seekHit = pOp->p4.i;
    }
  }
  اكسر;
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
حالة OP_SeekRowid: {        /* jump0, in3, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  صحيح res;
  i64 i_ئاچقۇچ;

  pIn3 = &aMem[pOp->p3];
  testcase( pIn3->flags & MEM_Int );
  testcase( pIn3->flags & MEM_IntReal );
  testcase( pIn3->flags & MEM_Real );
  testcase( (pIn3->flags & (MEM_Str|MEM_Int))==MEM_Str );
  إذا( (pIn3->flags & (MEM_Int|MEM_IntReal))==0 ){
    /* If pIn3->u.i does not contain an integer, compute iKey as the
    ** integer value of pIn3.  Jump to P2 if pIn3 cannot be converted
    ** into an integer without loss of information.  Take care to avoid
    ** changing the datatype of pIn3, however, as it is used by other
    ** parts of the prepared statement. */
    Mem x = pIn3[0];
    إذا( x.flags & MEM_Str ){
      applyNumericAffinity(&x, 1);
    }
    إذا( x.flags & MEM_Int ){
      i_ئاچقۇچ = x.u.i;
    }وإلا
    إذا( (x.flags & MEM_Real)==0
     || x.u.r < -9223372036854775808.0
     || x.u.r > 9223372036854774784.0
           /*   ^^^^^^^^^^^^^^^^^^^^^-- same value as every other double
           **   between 9223372036854774263.0 and 923372036854775295.0 */
     || (مزدوج)(i_ئاچقۇچ = sqlite3RealToI64(x.u.r))!=x.u.r
    ){
      اذهب jump_to_p2;
    }
    اذهب not_Exists_With_ئاچقۇچ;
  }
  /* Fall through into OP_NotExists */
  /* no break */ deliberate_fall_through
حالة OP_NotExists:          /* jump, in3, ncycle */
  pIn3 = &aMem[pOp->p3];
  أكد( (pIn3->flags & MEM_Int)!=0 || pOp->opcode==OP_SeekRowid );
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  i_ئاچقۇچ = pIn3->u.i;
not_Exists_With_ئاچقۇچ:
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
#ifdef SQLITE_DEBUG
  إذا( pOp->opcode==OP_SeekRowid ) pC->seekOp = OP_SeekRowid;
#endif
  أكد( pC->isTable );
  أكد( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  أكد( pCrsr!=0 );
  res = 0;
  rc = sqlite3BtreeTableMoveto(pCrsr, i_ئاچقۇچ, 0, &res);
  أكد( rc==SQLITE_OK || res==0 );
  pC->movetoTarget = i_ئاچقۇچ;  /* Used by OP_Delete */
  pC->nullRow = 0;
  pC->cacheStatus = CACHE_STALE;
  pC->deferredMoveto = 0;
  VdbeBranchTaken(res!=0,2);
  pC->seekResult = res;
  إذا( res!=0 ){
    أكد( rc==SQLITE_OK );
    إذا( pOp->p2==0 ){
      rc = SQLITE_CORRUPT_BKPT;
    }وإلا{
      اذهب jump_to_p2;
    }
  }
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}

/* Opcode: Sequence P1 P2 * * *
** Synopsis: r[P2]=cursor[P1].ctr++
**
** Find the next available sequence number for cursor P1.
** Write the sequence number into register P2.
** The sequence number on the cursor is incremented after this
** instruction. 
*/
حالة OP_Sequence: {           /* out2 */
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( p->apCsr[pOp->p1]!=0 );
  أكد( p->apCsr[pOp->p1]->eCurType!=CURTYPE_VTAB );
  pOut = out2Prerelease(p, pOp);
  pOut->u.i = p->apCsr[pOp->p1]->seq_سانى++;
  اكسر;
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
حالة OP_NewRowid: {           /* out2 */
  i64 v;                 /* The new rowid */
  VdbeCursor *pC;        /* Cursor of table to get the new rowid */
  صحيح res;               /* Result of an sqlite3BtreeLast() */
  صحيح cnt;               /* Counter to limit the number of searches */
#ifndef SQLITE_OMIT_AUTOINCREMENT
  Mem *pMem;             /* Register holding largest rowid for AUTOINCREMENT */
  VdbeFrame *pFrame;     /* Root frame of VDBE */
#endif

  v = 0;
  res = 0;
  pOut = out2Prerelease(p, pOp);
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->isTable );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->uc.pCursor!=0 );
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
    أكد( pC->isTable );

#ifdef SQLITE_32BIT_ROWID
#   define MAX_ROWID 0x7fffffff
#وإلا
    /* Some compilers complain about constants of the form 0x7fffffffffffffff.
    ** Others complain about 0x7ffffffffffffffffLL.  The following macro seems
    ** to provide the constant while making all compilers happy.
    */
#   define MAX_ROWID  (i64)( (((u64)0x7fffffff)<<32) | (u64)0xffffffff )
#endif

    إذا( !pC->useRandomRowid ){
      rc = sqlite3BtreeLast(pC->uc.pCursor, &res);
      إذا( rc!=SQLITE_OK ){
        اذهب abort_due_to_error;
      }
      إذا( res ){
        v = 1;   /* IMP: R-61914-48074 */
      }وإلا{
        أكد( sqlite3BtreeCursorIsValid(pC->uc.pCursor) );
        v = sqlite3_Btree_Integer_ئاچقۇچ(pC->uc.pCursor);
        إذا( v>=MAX_ROWID ){
          pC->useRandomRowid = 1;
        }وإلا{
          v++;   /* IMP: R-29538-34987 */
        }
      }
    }

#ifndef SQLITE_OMIT_AUTOINCREMENT
    إذا( pOp->p3 ){
      /* Assert that P3 is a valid memory cell. */
      أكد( pOp->p3>0 );
      إذا( p->pFrame ){
        لكل(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
        /* Assert that P3 is a valid memory cell. */
        أكد( pOp->p3<=pFrame->nMem );
        pMem = &pFrame->aMem[pOp->p3];
      }وإلا{
        /* Assert that P3 is a valid memory cell. */
        أكد( pOp->p3<=(p->nMem+1 - p->nCursor) );
        pMem = &aMem[pOp->p3];
        memAboutToChange(p, pMem);
      }
      أكد( memIsValid(pMem) );

      REGISTER_TRACE(pOp->p3, pMem);
      sqlite3VdbeMemIntegerify(pMem);
      أكد( (pMem->flags & MEM_Int)!=0 );  /* mem(P3) holds an integer */
      إذا( pMem->u.i==MAX_ROWID || pC->useRandomRowid ){
        rc = SQLITE_FULL;   /* IMP: R-17817-00630 */
        اذهب abort_due_to_error;
      }
      إذا( v<pMem->u.i+1 ){
        v = pMem->u.i + 1;
      }
      pMem->u.i = v;
    }
#endif
    إذا( pC->useRandomRowid ){
      /* IMPLEMENTATION-OF: R-07677-41881 If the largest ROWID is equal to the
      ** largest possible integer (9223372036854775807) then the database
      ** engine starts picking positive candidate ROWIDs at random until
      ** it finds one that is not previously used. */
      أكد( pOp->p3==0 );  /* We cannot be in random rowid mode if this is
                             ** an AUTOINCREMENT table. */
      cnt = 0;
      افعل{
        sqlite3_randomness(حجم(v), &v);
        v &= (MAX_ROWID>>1); v++;  /* Ensure that v is greater than zero */
      }طالما(  ((rc = sqlite3BtreeTableMoveto(pC->uc.pCursor, (u64)v,
                                                 0, &res))==SQLITE_OK)
            && (res==0)
            && (++cnt<100));
      إذا( rc ) اذهب abort_due_to_error;
      إذا( res==0 ){
        rc = SQLITE_FULL;   /* IMP: R-38219-53002 */
        اذهب abort_due_to_error;
      }
      أكد( v>0 );  /* EV: R-40812-03570 */
    }
    pC->deferredMoveto = 0;
    pC->cacheStatus = CACHE_STALE;
  }
  pOut->u.i = v;
  اكسر;
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
حالة OP_قىستۇرۇش: {
  Mem *pData;       /* MEM cell holding data for the record to be inserted */
  Mem *p_ئاچقۇچ;        /* MEM cell holding key  for the record */
  VdbeCursor *pC;   /* Cursor to table into which insert is written */
  صحيح seekResult;   /* Result of prior seek or 0 if no USESEEKRESULT flag */
  ثابت حرف *zDb;  /* database name - used by the update hook */
  Table *pTab;      /* Table structure - used by update and pre-update hooks */
  BtreePayload x;   /* Payload to be inserted */

  pData = &aMem[pOp->p2];
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( memIsValid(pData) );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->deferredMoveto==0 );
  أكد( pC->uc.pCursor!=0 );
  أكد( (pOp->p5 & OPFLAG_ISNOOP) || pC->isTable );
  أكد( pOp->p4type==P4_TABLE || pOp->p4type>=P4_STATIC );
  REGISTER_TRACE(pOp->p2, pData);
  sqlite3VdbeIncrWriteCounter(p, pC);

  p_ئاچقۇچ = &aMem[pOp->p3];
  أكد( p_ئاچقۇچ->flags & MEM_Int );
  أكد( memIsValid(p_ئاچقۇچ) );
  REGISTER_TRACE(pOp->p3, p_ئاچقۇچ);
  x.n_ئاچقۇچ = p_ئاچقۇچ->u.i;

  إذا( pOp->p4type==P4_TABLE && HAS_UPDATE_HOOK(db) ){
    أكد( pC->iDb>=0 );
    zDb = db->aDb[pC->iDb].zDbSName;
    pTab = pOp->p4.pTab;
    أكد( (pOp->p5 & OPFLAG_ISNOOP) || HasRowid(pTab) );
  }وإلا{
    pTab = 0;
    zDb = 0;
  }

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
  /* Invoke the pre-update hook, if any */
  إذا( pTab ){
    إذا( db->xPreUpdateCallback && !(pOp->p5 & OPFLAG_ISUPDATE) ){
      sqlite3VdbePreUpdateHook(p,pC,SQLITE_قىستۇرۇش,zDb,pTab,x.n_ئاچقۇچ,pOp->p2,-1);
    }
    إذا( db->xUpdateCallback==0 || pTab->aCol==0 ){
      /* Prevent post-update hook from running in cases when it should not */
      pTab = 0;
    }
  }
  إذا( pOp->p5 & OPFLAG_ISNOOP ) اكسر;
#endif

  أكد( (pOp->p5 & OPFLAG_LASTROWID)==0 || (pOp->p5 & OPFLAG_NCHANGE)!=0 );
  إذا( pOp->p5 & OPFLAG_NCHANGE ){
    p->nChange++;
    إذا( pOp->p5 & OPFLAG_LASTROWID ) db->lastRowid = x.n_ئاچقۇچ;
  }
  أكد( (pData->flags & (MEM_Blob|MEM_Str))!=0 || pData->n==0 );
  x.pData = pData->z;
  x.nData = pData->n;
  seekResult = ((pOp->p5 & OPFLAG_USESEEKRESULT) ? pC->seekResult : 0);
  إذا( pData->flags & MEM_Zero ){
    x.nZero = pData->u.nZero;
  }وإلا{
    x.nZero = 0;
  }
  x.p_ئاچقۇچ = 0;
  أكد( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  rc = sqlite3_Btree_قىستۇرۇش(pC->uc.pCursor, &x,
      (pOp->p5 & (OPFLAG_APPEND|OPFLAG_SAVEPOSITION|OPFLAG_PREFORMAT)),
      seekResult
  );
  pC->deferredMoveto = 0;
  pC->cacheStatus = CACHE_STALE;
  colCacheCtr++;

  /* Invoke the update-hook if required. */
  إذا( rc ) اذهب abort_due_to_error;
  إذا( pTab ){
    أكد( db->xUpdateCallback!=0 );
    أكد( pTab->aCol!=0 );
    db->xUpdateCallback(db->pUpdateArg,
           (pOp->p5 & OPFLAG_ISUPDATE) ? SQLITE_UPDATE : SQLITE_قىستۇرۇش,
           zDb, pTab->zName, x.n_ئاچقۇچ);
  }
  اكسر;
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
حالة OP_RowCell: {
  VdbeCursor *pDest;              /* Cursor to write to */
  VdbeCursor *pSrc;               /* Cursor to read from */
  i64 i_ئاچقۇچ;                       /* Rowid value to insert with */
  أكد( pOp[1].opcode==OP_قىستۇرۇش || pOp[1].opcode==OP_Idx_قىستۇرۇش );
  أكد( pOp[1].opcode==OP_قىستۇرۇش    || pOp->p3==0 );
  أكد( pOp[1].opcode==OP_Idx_قىستۇرۇش || pOp->p3>0 );
  أكد( pOp[1].p5 & OPFLAG_PREFORMAT );
  pDest = p->apCsr[pOp->p1];
  pSrc = p->apCsr[pOp->p2];
  i_ئاچقۇچ = pOp->p3 ? aMem[pOp->p3].u.i : 0;
  rc = sqlite3BtreeTransferRow(pDest->uc.pCursor, pSrc->uc.pCursor, i_ئاچقۇچ);
  إذا( rc!=SQLITE_OK ) اذهب abort_due_to_error;
  اكسر;
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
حالة OP_Delete: {
  VdbeCursor *pC;
  ثابت حرف *zDb;
  Table *pTab;
  صحيح opflags;

  opflags = pOp->p2;
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->uc.pCursor!=0 );
  أكد( pC->deferredMoveto==0 );
  sqlite3VdbeIncrWriteCounter(p, pC);

#ifdef SQLITE_DEBUG
  إذا( pOp->p4type==P4_TABLE
   && HasRowid(pOp->p4.pTab)
   && pOp->p5==0
   && sqlite3BtreeCursorIsValidNN(pC->uc.pCursor)
  ){
    /* If p5 is zero, the seek operation that positioned the cursor prior to
    ** OP_Delete will have also set the pC->movetoTarget field to the rowid of
    ** the row that is being deleted */
    i64 i_ئاچقۇچ = sqlite3_Btree_Integer_ئاچقۇچ(pC->uc.pCursor);
    أكد( CORRUPT_DB || pC->movetoTarget==i_ئاچقۇچ );
  }
#endif

  /* If the update-hook or pre-update-hook will be invoked, set zDb to
  ** the name of the db to pass as to it. Also set local pTab to a copy
  ** of p4.pTab. Finally, if p5 is true, indicating that this cursor was
  ** last moved with OP_Next or OP_Prev, not Seek or NotFound, set
  ** VdbeCursor.movetoTarget to the current rowid.  */
  إذا( pOp->p4type==P4_TABLE && HAS_UPDATE_HOOK(db) ){
    أكد( pC->iDb>=0 );
    أكد( pOp->p4.pTab!=0 );
    zDb = db->aDb[pC->iDb].zDbSName;
    pTab = pOp->p4.pTab;
    إذا( (pOp->p5 & OPFLAG_SAVEPOSITION)!=0 && pC->isTable ){
      pC->movetoTarget = sqlite3_Btree_Integer_ئاچقۇچ(pC->uc.pCursor);
    }
  }وإلا{
    zDb = 0;
    pTab = 0;
  }

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
  /* Invoke the pre-update-hook if required. */
  أكد( db->xPreUpdateCallback==0 || pTab==pOp->p4.pTab );
  إذا( db->xPreUpdateCallback && pTab ){
    أكد( !(opflags & OPFLAG_ISUPDATE)
         || HasRowid(pTab)==0
         || (aMem[pOp->p3].flags & MEM_Int)
    );
    sqlite3VdbePreUpdateHook(p, pC,
        (opflags & OPFLAG_ISUPDATE) ? SQLITE_UPDATE : SQLITE_DELETE,
        zDb, pTab, pC->movetoTarget,
        pOp->p3, -1
    );
  }
  إذا( opflags & OPFLAG_ISNOOP ) اكسر;
#endif

  /* Only flags that can be set are SAVEPOISTION and AUXDELETE */
  أكد( (pOp->p5 & ~(OPFLAG_SAVEPOSITION|OPFLAG_AUXDELETE))==0 );
  أكد( OPFLAG_SAVEPOSITION==BTREE_SAVEPOSITION );
  أكد( OPFLAG_AUXDELETE==BTREE_AUXDELETE );

#ifdef SQLITE_DEBUG
  إذا( p->pFrame==0 ){
    إذا( pC->isEphemeral==0
        && (pOp->p5 & OPFLAG_AUXDELETE)==0
        && (pC->wrFlag & OPFLAG_FORDELETE)==0
      ){
      nExtraDelete++;
    }
    إذا( pOp->p2 & OPFLAG_NCHANGE ){
      nExtraDelete--;
    }
  }
#endif

  rc = sqlite3BtreeDelete(pC->uc.pCursor, pOp->p5);
  pC->cacheStatus = CACHE_STALE;
  colCacheCtr++;
  pC->seekResult = 0;
  إذا( rc ) اذهب abort_due_to_error;

  /* Invoke the update-hook if required. */
  إذا( opflags & OPFLAG_NCHANGE ){
    p->nChange++;
    إذا( db->xUpdateCallback && ALWAYS(pTab!=0) && HasRowid(pTab) ){
      db->xUpdateCallback(db->pUpdateArg, SQLITE_DELETE, zDb, pTab->zName,
          pC->movetoTarget);
      أكد( pC->iDb>=0 );
    }
  }

  اكسر;
}
/* Opcode: ResetCount * * * * *
**
** The value of the change counter is copied to the database handle
** change counter (returned by subsequent calls to sqlite3_changes()).
** Then the VMs internal change counter resets to 0.
** This is used by trigger programs.
*/
حالة OP_Reset_سانى: {
  sqlite3VdbeSetChanges(db, p->nChange);
  p->nChange = 0;
  اكسر;
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
حالة OP_SorterCompare: {
  VdbeCursor *pC;
  صحيح res;
  صحيح n_ئاچقۇچ_Col;

  pC = p->apCsr[pOp->p1];
  أكد( isSorter(pC) );
  أكد( pOp->p4type==P4_INT32 );
  pIn3 = &aMem[pOp->p3];
  n_ئاچقۇچ_Col = pOp->p4.i;
  res = 0;
  rc = sqlite3VdbeSorterCompare(pC, pIn3, n_ئاچقۇچ_Col, &res);
  VdbeBranchTaken(res!=0,2);
  إذا( rc ) اذهب abort_due_to_error;
  إذا( res ) اذهب jump_to_p2;
  اكسر;
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
حالة OP_SorterData: {       /* ncycle */
  VdbeCursor *pC;

  pOut = &aMem[pOp->p2];
  pC = p->apCsr[pOp->p1];
  أكد( isSorter(pC) );
  rc = sqlite3VdbeSorterRowkey(pC, pOut);
  أكد( rc!=SQLITE_OK || (pOut->flags & MEM_Blob) );
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  إذا( rc ) اذهب abort_due_to_error;
  p->apCsr[pOp->p3]->cacheStatus = CACHE_STALE;
  اكسر;
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
حالة OP_RowData: {
  VdbeCursor *pC;
  BtCursor *pCrsr;
  u32 n;

  pOut = out2Prerelease(p, pOp);

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( isSorter(pC)==0 );
  أكد( pC->nullRow==0 );
  أكد( pC->uc.pCursor!=0 );
  pCrsr = pC->uc.pCursor;

  /* The OP_RowData opcodes always follow OP_NotExists or
  ** OP_SeekRowid or OP_Rewind/Op_Next with no intervening instructions
  ** that might invalidate the cursor.
  ** If this were not the case, one of the following assert()s
  ** would fail.  Should this ever change (because of changes in the code
  ** generator) then the fix would be to insert a call to
  ** sqlite3VdbeCursorMoveto().
  */
  أكد( pC->deferredMoveto==0 );
  أكد( sqlite3BtreeCursorIsValid(pCrsr) );

  n = sqlite3BtreePayloadSize(pCrsr);
  إذا( n>(u32)db->aLimit[SQLITE_LIMIT_LENGTH] ){
    اذهب too_big;
  }
  testcase( n==0 );
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCrsr, n, pOut);
  إذا( rc ) اذهب abort_due_to_error;
  إذا( !pOp->p3 ) Deephemeralize(pOut);
  UPDATE_MAX_BLOBSIZE(pOut);
  REGISTER_TRACE(pOp->p2, pOut);
  اكسر;
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
حالة OP_Rowid: {                 /* out2, ncycle */
  VdbeCursor *pC;
  i64 v;
  sqlite3_vtab *pVtab;
  ثابت sqlite3_module *pModule;

  pOut = out2Prerelease(p, pOp);
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType!=CURTYPE_PSEUDO || pC->nullRow );
  إذا( pC->nullRow ){
    pOut->flags = MEM_Null;
    اكسر;
  }وإلا إذا( pC->deferredMoveto ){
    v = pC->movetoTarget;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  }وإلا إذا( pC->eCurType==CURTYPE_VTAB ){
    أكد( pC->uc.pVCur!=0 );
    pVtab = pC->uc.pVCur->pVtab;
    pModule = pVtab->pModule;
    أكد( pModule->xRowid );
    rc = pModule->xRowid(pC->uc.pVCur, &v);
    sqlite3VtabImportErrmsg(p, pVtab);
    إذا( rc ) اذهب abort_due_to_error;
#endif /* SQLITE_OMIT_VIRTUALTABLE */
  }وإلا{
    أكد( pC->eCurType==CURTYPE_BTREE );
    أكد( pC->uc.pCursor!=0 );
    rc = sqlite3VdbeCursorRestore(pC);
    إذا( rc ) اذهب abort_due_to_error;
    إذا( pC->nullRow ){
      pOut->flags = MEM_Null;
      اكسر;
    }
    v = sqlite3_Btree_Integer_ئاچقۇچ(pC->uc.pCursor);
  }
  pOut->u.i = v;
  اكسر;
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
حالة OP_NullRow: {
  VdbeCursor *pC;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  إذا( pC==0 ){
    /* If the cursor is not already open, create a special kind of
    ** pseudo-cursor that always gives null rows. */
    pC = allocateCursor(p, pOp->p1, 1, CURTYPE_PSEUDO);
    إذا( pC==0 ) اذهب no_mem;
    pC->seekResult = 0;
    pC->isTable = 1;
    pC->noReuse = 1;
    pC->uc.pCursor = sqlite3BtreeFakeValidCursor();
  }
  pC->nullRow = (pOp->p2==0);
  pC->cacheStatus = CACHE_STALE;
  إذا( pC->eCurType==CURTYPE_BTREE ){
    أكد( pC->uc.pCursor!=0 );
    sqlite3BtreeClearCursor(pC->uc.pCursor);
  }
#ifdef SQLITE_DEBUG
  إذا( pC->seekOp==0 ) pC->seekOp = OP_NullRow;
#endif
  اكسر;
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
حالة OP_SeekEnd:             /* ncycle */
حالة OP_Last: {              /* jump0, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  صحيح res;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  res = 0;
  أكد( pCrsr!=0 );
#ifdef SQLITE_DEBUG
  pC->seekOp = pOp->opcode;
#endif
  إذا( pOp->opcode==OP_SeekEnd ){
    أكد( pOp->p2==0 );
    pC->seekResult = -1;
    إذا( sqlite3BtreeCursorIsValidNN(pCrsr) ){
      اكسر;
    }
  }
  rc = sqlite3BtreeLast(pCrsr, &res);
  pC->nullRow = (u8)res;
  pC->deferredMoveto = 0;
  pC->cacheStatus = CACHE_STALE;
  إذا( rc ) اذهب abort_due_to_error;
  إذا( pOp->p2>0 ){
    VdbeBranchTaken(res!=0,2);
    إذا( res ) اذهب jump_to_p2;
  }
  اكسر;
}

/* Opcode: IfSizeBetween P1 P2 P3 P4 *
**
** Let N be the approximate number of rows in the table or index
** with cursor P1 and let X be 10*log2(N) if N is positive or -1
** if N is zero.
**
** Jump to P2 if X is in between P3 and P4, inclusive.
*/
حالة OP_IfSizeBetween: {        /* jump */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  صحيح res;
  i64 sz;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( pOp->p4type==P4_INT32 );
  أكد( pOp->p3>=-1 && pOp->p3<=640*2 );
  أكد( pOp->p4.i>=-1 && pOp->p4.i<=640*2 );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  pCrsr = pC->uc.pCursor;
  أكد( pCrsr );
  rc = sqlite3BtreeFirst(pCrsr, &res);
  إذا( rc ) اذهب abort_due_to_error;
  إذا( res!=0 ){
    sz = -1;  /* -Infinity encoding */
  }وإلا{
    sz = sqlite3_Btree_Row_سانى_Est(pCrsr);
    أكد( sz>0 );
    sz = sqlite3LogEst((u64)sz);
  }
  res = sz>=pOp->p3 && sz<=pOp->p4.i;
  VdbeBranchTaken(res!=0,2);
  إذا( res ) اذهب jump_to_p2;
  اكسر;
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
حالة OP_SorterSort:    /* jump ncycle */
حالة OP_Sort: {        /* jump ncycle */
#ifdef SQLITE_TEST
  sqlite3_sort_سانى++;
  sqlite3_ئىزدەش_سانى--;
#endif
  p->aCounter[SQLITE_STMTSTATUS_SORT]++;
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
حالة OP_Rewind: {        /* jump0, ncycle */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  صحيح res;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( pOp->p5==0 );
  أكد( pOp->p2>=0 && pOp->p2<p->nOp );

  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( isSorter(pC)==(pOp->opcode==OP_SorterSort) );
  res = 1;
#ifdef SQLITE_DEBUG
  pC->seekOp = OP_Rewind;
#endif
  إذا( isSorter(pC) ){
    rc = sqlite3VdbeSorterRewind(pC, &res);
  }وإلا{
    أكد( pC->eCurType==CURTYPE_BTREE );
    pCrsr = pC->uc.pCursor;
    أكد( pCrsr );
    rc = sqlite3BtreeFirst(pCrsr, &res);
    pC->deferredMoveto = 0;
    pC->cacheStatus = CACHE_STALE;
  }
  إذا( rc ) اذهب abort_due_to_error;
  pC->nullRow = (u8)res;
  إذا( pOp->p2>0 ){
    VdbeBranchTaken(res!=0,2);
    إذا( res ) اذهب jump_to_p2;
  }
  اكسر;
}

/* Opcode: IfEmpty P1 P2 * * *
** Synopsis: if( empty(P1) ) goto P2
**
** Check to see if the b-tree table that cursor P1 references is empty
** and jump to P2 if it is.
*/
حالة OP_IfEmpty: {        /* jump */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  صحيح res;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( pOp->p2>=0 && pOp->p2<p->nOp );

  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  pCrsr = pC->uc.pCursor;
  أكد( pCrsr );
  rc = sqlite3BtreeIsEmpty(pCrsr, &res);
  إذا( rc ) اذهب abort_due_to_error;
  VdbeBranchTaken(res!=0,2);
  إذا( res ) اذهب jump_to_p2;
  اكسر;
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
حالة OP_SorterNext: {  /* jump */
  VdbeCursor *pC;

  pC = p->apCsr[pOp->p1];
  أكد( isSorter(pC) );
  rc = sqlite3VdbeSorterNext(db, pC);
  اذهب next_tail;

حالة OP_Prev:          /* jump, ncycle */
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( pOp->p5==0
       || pOp->p5==SQLITE_STMTSTATUS_FULLSCAN_STEP
       || pOp->p5==SQLITE_STMTSTATUS_AUTOINDEX);
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->deferredMoveto==0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->seekOp==OP_SeekLT || pC->seekOp==OP_SeekLE
       || pC->seekOp==OP_Last   || pC->seekOp==OP_IfNoHope
       || pC->seekOp==OP_NullRow);
  rc = sqlite3BtreePrevious(pC->uc.pCursor, pOp->p3);
  اذهب next_tail;

حالة OP_Next:          /* jump, ncycle */
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  أكد( pOp->p5==0
       || pOp->p5==SQLITE_STMTSTATUS_FULLSCAN_STEP
       || pOp->p5==SQLITE_STMTSTATUS_AUTOINDEX);
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->deferredMoveto==0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->seekOp==OP_SeekGT || pC->seekOp==OP_SeekGE
       || pC->seekOp==OP_Rewind || pC->seekOp==OP_Found
       || pC->seekOp==OP_NullRow|| pC->seekOp==OP_SeekRowid
       || pC->seekOp==OP_IfNoHope);
  rc = sqlite3BtreeNext(pC->uc.pCursor, pOp->p3);

next_tail:
  pC->cacheStatus = CACHE_STALE;
  VdbeBranchTaken(rc==SQLITE_OK,2);
  إذا( rc==SQLITE_OK ){
    pC->nullRow = 0;
    p->aCounter[pOp->p5]++;
#ifdef SQLITE_TEST
    sqlite3_ئىزدەش_سانى++;
#endif
    اذهب jump_to_p2_and_check_for_interrupt;
  }
  إذا( rc!=SQLITE_DONE ) اذهب abort_due_to_error;
  rc = SQLITE_OK;
  pC->nullRow = 1;
  اذهب check_for_interrupt;
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
حالة OP_Idx_قىستۇرۇش: {        /* in2 */
  VdbeCursor *pC;
  BtreePayload x;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  sqlite3VdbeIncrWriteCounter(p, pC);
  أكد( pC!=0 );
  أكد( !isSorter(pC) );
  pIn2 = &aMem[pOp->p2];
  أكد( (pIn2->flags & MEM_Blob) || (pOp->p5 & OPFLAG_PREFORMAT) );
  إذا( pOp->p5 & OPFLAG_NCHANGE ) p->nChange++;
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->isTable==0 );
  rc = ExpandBlob(pIn2);
  إذا( rc ) اذهب abort_due_to_error;
  x.n_ئاچقۇچ = pIn2->n;
  x.p_ئاچقۇچ = pIn2->z;
  x.aMem = aMem + pOp->p3;
  x.nMem = (u16)pOp->p4.i;
  rc = sqlite3_Btree_قىستۇرۇش(pC->uc.pCursor, &x,
       (pOp->p5 & (OPFLAG_APPEND|OPFLAG_SAVEPOSITION|OPFLAG_PREFORMAT)),
      ((pOp->p5 & OPFLAG_USESEEKRESULT) ? pC->seekResult : 0)
      );
  أكد( pC->deferredMoveto==0 );
  pC->cacheStatus = CACHE_STALE;
  إذا( rc) اذهب abort_due_to_error;
  اكسر;
}

/* Opcode: SorterInsert P1 P2 * * *
** Synopsis: key=r[P2]
**
** Register P2 holds an SQL index key made using the
** MakeRecord instructions.  This opcode writes that key
** into the sorter P1.  Data for the entry is nil.
*/
حالة OP_Sorter_قىستۇرۇش: {     /* in2 */
  VdbeCursor *pC;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  sqlite3VdbeIncrWriteCounter(p, pC);
  أكد( pC!=0 );
  أكد( isSorter(pC) );
  pIn2 = &aMem[pOp->p2];
  أكد( pIn2->flags & MEM_Blob );
  أكد( pC->isTable==0 );
  rc = ExpandBlob(pIn2);
  إذا( rc ) اذهب abort_due_to_error;
  rc = sqlite3VdbeSorterWrite(pC, pIn2);
  إذا( rc) اذهب abort_due_to_error;
  اكسر;
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
حالة OP_IdxDelete: {
  VdbeCursor *pC;
  BtCursor *pCrsr;
  صحيح res;
  UnpackedRecord r;

  أكد( pOp->p5>0 );
  أكد( pOp->p2>0 && pOp->p2+pOp->p5<=(p->nMem+1 - p->nCursor)+1 );
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  sqlite3VdbeIncrWriteCounter(p, pC);
  pCrsr = pC->uc.pCursor;
  أكد( pCrsr!=0 );
  r.p_ئاچقۇچ_Info = pC->p_ئاچقۇچ_Info;
  r.nField = pOp->p5;
  r.default_rc = 0;
  r.aMem = &aMem[pOp->p2];
  rc = sqlite3BtreeIndexMoveto(pCrsr, &r, &res);
  إذا( rc ) اذهب abort_due_to_error;
  إذا( res!=0 ){
    rc = sqlite3_Vdbe_Find_Index_ئاچقۇچ(pCrsr, pOp->p4.pIdx, &r, &res, 0);
    إذا( rc!=SQLITE_OK ) اذهب abort_due_to_error;
    إذا( res!=0 ){
      إذا( !sqlite3WritableSchema(db) ){
        rc = sqlite3ReportError(
            SQLITE_CORRUPT_INDEX, __LINE__, "index corruption");
        اذهب abort_due_to_error;
      }
      pC->cacheStatus = CACHE_STALE;
      pC->seekResult = 0;
      اكسر;
    }
  }

  إذا( pOp->p3 && vdbe_Index_ئاچقۇچ_Compare(pCrsr, &aMem[pOp->p3], &rc) ){
    إذا( rc ) اذهب abort_due_to_error;
    sqlite3VdbeMemSetNull(&aMem[pOp->p3]);
    اكسر;
  }

  rc = sqlite3BtreeDelete(pCrsr, BTREE_AUXDELETE);
  إذا( rc ) اذهب abort_due_to_error;
  أكد( pC->deferredMoveto==0 );
  pC->cacheStatus = CACHE_STALE;
  pC->seekResult = 0;
  اكسر;
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
حالة OP_DeferredSeek:         /* ncycle */
حالة OP_IdxRowid: {           /* out2, ncycle */
  VdbeCursor *pC;             /* The P1 index cursor */
  VdbeCursor *pTabCur;        /* The P2 table cursor (OP_DeferredSeek only) */
  i64 rowid;                  /* Rowid that P1 current points to */

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE || IsNullCursor(pC) );
  أكد( pC->uc.pCursor!=0 );
  أكد( pC->isTable==0 || IsNullCursor(pC) );
  أكد( pC->deferredMoveto==0 );
  أكد( !pC->nullRow || pOp->opcode==OP_IdxRowid );

  /* The IdxRowid and Seek opcodes are combined because of the commonality
  ** of sqlite3VdbeCursorRestore() and sqlite3VdbeIdxRowid(). */
  rc = sqlite3VdbeCursorRestore(pC);

  /* sqlite3VdbeCursorRestore() may fail if the cursor has been disturbed
  ** since it was last positioned and an error (e.g. OOM or an IO error)
  ** occurs while trying to reposition it. */
  إذا( rc!=SQLITE_OK ) اذهب abort_due_to_error;

  إذا( !pC->nullRow ){
    rowid = 0;  /* Not needed.  Only used to silence a warning. */
    rc = sqlite3VdbeIdxRowid(db, pC->uc.pCursor, &rowid);
    إذا( rc!=SQLITE_OK ){
      اذهب abort_due_to_error;
    }
    إذا( pOp->opcode==OP_DeferredSeek ){
      أكد( pOp->p3>=0 && pOp->p3<p->nCursor );
      pTabCur = p->apCsr[pOp->p3];
      أكد( pTabCur!=0 );
      أكد( pTabCur->eCurType==CURTYPE_BTREE );
      أكد( pTabCur->uc.pCursor!=0 );
      أكد( pTabCur->isTable );
#إذا defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
      أكد( 
          sqlite3BtreeCursorHintTblCsr(pC->uc.pCursor)==pTabCur->uc.pCursor 
      );
#endif
      pTabCur->nullRow = 0;
      pTabCur->movetoTarget = rowid;
      pTabCur->deferredMoveto = 1;
      pTabCur->cacheStatus = CACHE_STALE;
      أكد( pOp->p4type==P4_INTARRAY || pOp->p4.ai==0 );
      أكد( !pTabCur->isEphemeral );
      pTabCur->ub.aAltMap = pOp->p4.ai;
      أكد( !pC->isEphemeral );
      pTabCur->pAltCursor = pC;
    }وإلا{
      pOut = out2Prerelease(p, pOp);
      pOut->u.i = rowid;
    }
  }وإلا{
    أكد( pOp->opcode==OP_IdxRowid );
    sqlite3VdbeMemSetNull(&aMem[pOp->p2]);
  }
  اكسر;
}

/* Opcode: FinishSeek P1 * * * *
**
** If cursor P1 was previously moved via OP_DeferredSeek, complete that
** seek operation now, without further delay.  If the cursor seek has
** already occurred, this instruction is a no-op.
*/
حالة OP_FinishSeek: {        /* ncycle */
  VdbeCursor *pC;            /* The P1 index cursor */

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  إذا( pC->deferredMoveto ){
    rc = sqlite3VdbeFinishMoveto(pC);
    إذا( rc ) اذهب abort_due_to_error;
  }
  اكسر;
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
حالة OP_IdxLE:          /* jump, ncycle */
حالة OP_IdxGT:          /* jump, ncycle */
حالة OP_IdxLT:          /* jump, ncycle */
حالة OP_IdxGE:  {       /* jump, ncycle */
  VdbeCursor *pC;
  صحيح res;
  UnpackedRecord r;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->isOrdered );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->uc.pCursor!=0);
  أكد( pC->deferredMoveto==0 );
  أكد( pOp->p4type==P4_INT32 );
  r.p_ئاچقۇچ_Info = pC->p_ئاچقۇچ_Info;
  r.nField = (u16)pOp->p4.i;
  إذا( pOp->opcode<OP_IdxLT ){
    أكد( pOp->opcode==OP_IdxLE || pOp->opcode==OP_IdxGT );
    r.default_rc = -1;
  }وإلا{
    أكد( pOp->opcode==OP_IdxGE || pOp->opcode==OP_IdxLT );
    r.default_rc = 0;
  }
  r.aMem = &aMem[pOp->p3];
#ifdef SQLITE_DEBUG
  {
    صحيح i;
    لكل(i=0; i<r.nField; i++){
      أكد( memIsValid(&r.aMem[i]) );
      REGISTER_TRACE(pOp->p3+i, &aMem[pOp->p3+i]);
    }
  }
#endif

  /* Inlined version of sqlite3VdbeIdxKeyCompare() */
  {
    i64 n_Cell_ئاچقۇچ = 0;
    BtCursor *pCur;
    Mem m;

    أكد( pC->eCurType==CURTYPE_BTREE );
    pCur = pC->uc.pCursor;
    أكد( sqlite3BtreeCursorIsValid(pCur) );
    n_Cell_ئاچقۇچ = sqlite3BtreePayloadSize(pCur);
    /* nCellKey will always be between 0 and 0xffffffff because of the way
    ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
    إذا( n_Cell_ئاچقۇچ<=0 || n_Cell_ئاچقۇچ>0x7fffffff ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب abort_due_to_error;
    }
    sqlite3VdbeMemInit(&m, db, 0);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)n_Cell_ئاچقۇچ, &m);
    إذا( rc ) اذهب abort_due_to_error;
    res = sqlite3VdbeRecordCompareWithSkip(m.n, m.z, &r, 0);
    sqlite3VdbeMemReleaseMalloc(&m);
  }
  /* End of inlined sqlite3VdbeIdxKeyCompare() */

  أكد( (OP_IdxLE&1)==(OP_IdxLT&1) && (OP_IdxGE&1)==(OP_IdxGT&1) );
  إذا( (pOp->opcode&1)==(OP_IdxLT&1) ){
    أكد( pOp->opcode==OP_IdxLE || pOp->opcode==OP_IdxLT );
    res = -res;
  }وإلا{
    أكد( pOp->opcode==OP_IdxGE || pOp->opcode==OP_IdxGT );
    res++;
  }
  VdbeBranchTaken(res>0,2);
  أكد( rc==SQLITE_OK );
  إذا( res>0 ) اذهب jump_to_p2;
  اكسر;
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
حالة OP_Destroy: {     /* out2 */
  صحيح iMoved;
  صحيح iDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  أكد( p->readOnly==0 );
  أكد( pOp->p1>1 );
  pOut = out2Prerelease(p, pOp);
  pOut->flags = MEM_Null;
  إذا( db->nVdbeRead > db->nVDestroy+1 ){
    rc = SQLITE_LOCKED;
    p->errorAction = OE_Abort;
    اذهب abort_due_to_error;
  }وإلا{
    iDb = pOp->p3;
    أكد( DbMaskTest(p->btreeMask, iDb) );
    iMoved = 0;  /* Not needed.  Only to silence a warning. */
    rc = sqlite3BtreeDropTable(db->aDb[iDb].pBt, pOp->p1, &iMoved);
    pOut->flags = MEM_Int;
    pOut->u.i = iMoved;
    إذا( rc ) اذهب abort_due_to_error;
#ifndef SQLITE_OMIT_AUTOVACUUM
    إذا( iMoved!=0 ){
      sqlite3_يىلتىز_Page_Moved(db, iDb, iMoved, pOp->p1);
      /* All OP_Destroy operations occur on the same btree */
      أكد( resetSchemaOnFault==0 || resetSchemaOnFault==iDb+1 );
      resetSchemaOnFault = iDb+1;
    }
#endif
  }
  اكسر;
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
حالة OP_Clear: {
  i64 nChange;

  sqlite3VdbeIncrWriteCounter(p, 0);
  nChange = 0;
  أكد( p->readOnly==0 );
  أكد( DbMaskTest(p->btreeMask, pOp->p2) );
  rc = sqlite3BtreeClearTable(db->aDb[pOp->p2].pBt, (u32)pOp->p1, &nChange);
  إذا( pOp->p3 ){
    p->nChange += nChange;
    إذا( pOp->p3>0 ){
      أكد( memIsValid(&aMem[pOp->p3]) );
      memAboutToChange(p, &aMem[pOp->p3]);
      aMem[pOp->p3].u.i += nChange;
    }
  }
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}

/* Opcode: ResetSorter P1 * * * *
**
** Delete all contents from the ephemeral table or sorter
** that is open on cursor P1.
**
** This opcode only works for cursors used for sorting and
** opened with OP_OpenEphemeral or OP_SorterOpen.
*/
حالة OP_ResetSorter: {
  VdbeCursor *pC;

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  إذا( isSorter(pC) ){
    sqlite3VdbeSorterReset(db, pC->uc.pSorter);
  }وإلا{
    أكد( pC->eCurType==CURTYPE_BTREE );
    أكد( pC->isEphemeral );
    rc = sqlite3BtreeClearTableOfCursor(pC->uc.pCursor);
    إذا( rc ) اذهب abort_due_to_error;
  }
  اكسر;
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
حالة OP_CreateBtree: {          /* out2 */
  Pgno pgno;
  Db *pDb;

  sqlite3VdbeIncrWriteCounter(p, 0);
  pOut = out2Prerelease(p, pOp);
  pgno = 0;
  أكد( pOp->p3==BTREE_INTKEY || pOp->p3==BTREE_BLOBKEY );
  أكد( pOp->p1>=0 && pOp->p1<db->nDb );
  أكد( DbMaskTest(p->btreeMask, pOp->p1) );
  أكد( p->readOnly==0 );
  pDb = &db->aDb[pOp->p1];
  أكد( pDb->pBt!=0 );
  rc = sqlite3BtreeCreateTable(pDb->pBt, &pgno, pOp->p3);
  إذا( rc ) اذهب abort_due_to_error;
  pOut->u.i = pgno;
  اكسر;
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
حالة OP_SqlExec: {
  حرف *zErr;
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;
#endif
  u8 mTrace;
  صحيح savedAnalysisLimit;

  sqlite3VdbeIncrWriteCounter(p, 0);
  db->nSqlExec++;
  zErr = 0;
#ifndef SQLITE_OMIT_AUTHORIZATION
  xAuth = db->xAuth;
#endif
  mTrace = db->mTrace;
  savedAnalysisLimit = db->nAnalysisLimit;
  إذا( pOp->p1 & 0x0001 ){
#ifndef SQLITE_OMIT_AUTHORIZATION
    db->xAuth = 0;
#endif
    db->mTrace = 0;
  }
  إذا( pOp->p1 & 0x0002 ){
    db->nAnalysisLimit = pOp->p2;
  }
  rc = sqlite3_exec(db, pOp->p4.z, 0, 0, &zErr);
  db->nSqlExec--;
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  db->mTrace = mTrace;
  db->nAnalysisLimit = savedAnalysisLimit;
  إذا( zErr || rc ){
    sqlite3VdbeError(p, "%s", zErr);
    sqlite3_free(zErr);
    إذا( rc==SQLITE_NOMEM ) اذهب no_mem;
    اذهب abort_due_to_error;
  }
  اكسر;
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
حالة OP_ParseSchema: {
  صحيح iDb;
  ثابت حرف *zSchema;
  حرف *zSql;
  InitData initData;

  /* Any prepared statement that invokes this opcode will hold mutexes
  ** on every btree.  This is a prerequisite for invoking
  ** sqlite3InitCallback().
  */
#ifdef SQLITE_DEBUG
  لكل(iDb=0; iDb<db->nDb; iDb++){
    أكد( iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[iDb].pBt) );
  }
#endif

  iDb = pOp->p1;
  أكد( iDb>=0 && iDb<db->nDb );
  أكد( DbHasProperty(db, iDb, DB_SchemaLoaded)
           || db->mallocFailed
           || (CORRUPT_DB && (db->flags & SQLITE_NoSchemaError)!=0) );

#ifndef SQLITE_OMIT_ALTERTABLE
  إذا( pOp->p4.z==0 ){
    أكد( pOp->p5!=0 );
    sqlite3SchemaClear(db->aDb[iDb].pSchema);
    db->mDbFlags &= ~DBFLAG_SchemaKnownOk;
    rc = sqlite3InitOne(db, iDb, &p->z_Err_ئۇچۇر, pOp->p5);
    db->mDbFlags |= DBFLAG_SchemaChange;
    p->expired = 0;
  }وإلا
#endif
  {
    أكد( pOp->p5==0 );
    zSchema = LEGACY_SCHEMA_TABLE;
    initData.db = db;
    initData.iDb = iDb;
    initData.pz_Err_ئۇچۇر = &p->z_Err_ئۇچۇر;
    initData.mInitFlags = 0;
    initData.mxPage = sqlite3BtreeLastPage(db->aDb[iDb].pBt);
    zSql = sqlite3MPrintf(db,
       "SELECT*FROM\"%w\".%s WHERE %s ORDER BY rowid",
       db->aDb[iDb].zDbSName, zSchema, pOp->p4.z);
    إذا( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }وإلا{
      أكد( db->init.busy==0 );
      db->init.busy = 1;
      initData.rc = SQLITE_OK;
      initData.nInitRow = 0;
      أكد( !db->mallocFailed );
      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &initData, 0);
      إذا( rc==SQLITE_OK ) rc = initData.rc;
      إذا( rc==SQLITE_OK && initData.nInitRow==0 ){
        /* The OP_ParseSchema opcode with a non-NULL P4 argument should parse
        ** at least one SQL statement. Any less than that indicates that
        ** the sqlite_schema table is corrupt. */
        rc = SQLITE_CORRUPT_BKPT;
      }
      sqlite3DbFreeNN(db, zSql);
      db->init.busy = 0;
    }
  }
  إذا( rc ){
    sqlite3ResetAllSchemasOfConnection(db);
    إذا( rc==SQLITE_NOMEM ){
      اذهب no_mem;
    }
    اذهب abort_due_to_error;
  }
  اكسر; 
}

#إذا !defined(SQLITE_OMIT_ANALYZE)
/* Opcode: LoadAnalysis P1 * * * *
**
** Read the sqlite_stat1 table for database P1 and load the content
** of that table into the internal index hash table.  This will cause
** the analysis to be used when preparing all subsequent queries.
*/
حالة OP_LoadAnalysis: {
  أكد( pOp->p1>=0 && pOp->p1<db->nDb );
  rc = sqlite3AnalysisLoad(db, pOp->p1);
  إذا( rc ) اذهب abort_due_to_error;
  اكسر; 
}
#endif /* !defined(SQLITE_OMIT_ANALYZE) */

/* Opcode: DropTable P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the table named P4 in database P1.  This is called after a table
** is dropped from disk (using the Destroy opcode) in order to keep
** the internal representation of the
** schema consistent with what is on disk.
*/
حالة OP_DropTable: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3UnlinkAndDeleteTable(db, pOp->p1, pOp->p4.z);
  اكسر;
}

/* Opcode: DropIndex P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the index named P4 in database P1.  This is called after an index
** is dropped from disk (using the Destroy opcode)
** in order to keep the internal representation of the
** schema consistent with what is on disk.
*/
حالة OP_DropIndex: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3UnlinkAndDeleteIndex(db, pOp->p1, pOp->p4.z);
  اكسر;
}

/* Opcode: DropTrigger P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the trigger named P4 in database P1.  This is called after a trigger
** is dropped from disk (using the Destroy opcode) in order to keep
** the internal representation of the
** schema consistent with what is on disk.
*/
حالة OP_DropTrigger: {
  sqlite3VdbeIncrWriteCounter(p, 0);
  sqlite3UnlinkAndDeleteTrigger(db, pOp->p1, pOp->p4.z);
  اكسر;
}


#ifndef SQLITE_OMIT_INTEGRITY_CHECK
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
حالة OP_IntegrityCk: {
  صحيح n_يىلتىز;      /* Number of tables to check.  (Number of root pages.) */
  Pgno *a_يىلتىز;    /* Array of rootpage numbers for tables to be checked */
  صحيح nErr;       /* Number of errors reported */
  حرف *z;        /* Text of the error report */
  Mem *pnErr;     /* Register keeping track of errors remaining */

  أكد( p->bIsReader );
  أكد( pOp->p4type==P4_INTARRAY );
  n_يىلتىز = pOp->p2;
  a_يىلتىز = pOp->p4.ai;
  أكد( n_يىلتىز>0 );
  أكد( a_يىلتىز!=0 );
  أكد( a_يىلتىز[0]==(Pgno)n_يىلتىز );
  أكد( pOp->p1>0 && (pOp->p1+1)<=(p->nMem+1 - p->nCursor) );
  pnErr = &aMem[pOp->p1];
  أكد( (pnErr->flags & MEM_Int)!=0 );
  أكد( (pnErr->flags & (MEM_Str|MEM_Blob))==0 );
  pIn1 = &aMem[pOp->p1+1];
  أكد( pOp->p5<db->nDb );
  أكد( DbMaskTest(p->btreeMask, pOp->p5) );
  rc = sqlite3BtreeIntegrityCheck(db, db->aDb[pOp->p5].pBt, &a_يىلتىز[1], 
      &aMem[pOp->p3], n_يىلتىز, (صحيح)pnErr->u.i+1, &nErr, &z);
  sqlite3VdbeMemSetNull(pIn1);
  إذا( nErr==0 ){
    أكد( z==0 );
  }وإلا إذا( rc ){
    sqlite3_free(z);
    اذهب abort_due_to_error;
  }وإلا{
    pnErr->u.i -= nErr-1;
    sqlite3VdbeMemSetStr(pIn1, z, -1, SQLITE_UTF8, sqlite3_free);
  }
  UPDATE_MAX_BLOBSIZE(pIn1);
  sqlite3VdbeChangeEncoding(pIn1, encoding);
  اذهب check_for_interrupt;
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
حالة OP_IFind_ئاچقۇچ: {     /* jump, in3 */
  VdbeCursor *pC;
  صحيح res;
  UnpackedRecord r;

  أكد( pOp[-1].opcode==OP_Found );
  أكد( pOp[-1].p1==pOp->p1 );
  أكد( pOp[-1].p3==pOp->p3 );
  pC = p->apCsr[pOp->p1];
  أكد( pOp->p4type==P4_INDEX );
  أكد( pC->eCurType==CURTYPE_BTREE );
  أكد( pC->uc.pCursor!=0 );
  أكد( pC->isTable==0 );

  memset(&r, 0, حجم(r));
  r.aMem = &aMem[pOp->p3];
  r.nField = pOp->p4.pIdx->nColumn;
  r.p_ئاچقۇچ_Info = pC->p_ئاچقۇچ_Info;

  rc = sqlite3_Vdbe_Find_Index_ئاچقۇچ(pC->uc.pCursor, pOp->p4.pIdx, &r, &res, 1);
  إذا( rc || res!=0 ){
    rc = SQLITE_OK;
    اذهب jump_to_p2;
  }
  pC->nullRow = 0;
  اكسر;
};
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

/* Opcode: RowSetAdd P1 P2 * * *
** Synopsis: rowset(P1)=r[P2]
**
** Insert the integer value held by register P2 into a RowSet object
** held in register P1.
**
** An assertion fails if P2 is not an integer.
*/
حالة OP_RowSetAdd: {       /* in1, in2 */
  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  أكد( (pIn2->flags & MEM_Int)!=0 );
  إذا( (pIn1->flags & MEM_Blob)==0 ){
    إذا( sqlite3VdbeMemSetRowSet(pIn1) ) اذهب no_mem;
  }
  أكد( sqlite3VdbeMemIsRowSet(pIn1) );
  sqlite3_Row_Set_قىستۇرۇش((RowSet*)pIn1->z, pIn2->u.i);
  اكسر;
}

/* Opcode: RowSetRead P1 P2 P3 * *
** Synopsis: r[P3]=rowset(P1)
**
** Extract the smallest value from the RowSet object in P1
** and put that value into register P3.
** Or, if RowSet object P1 is initially empty, leave P3
** unchanged and jump to instruction P2.
*/
حالة OP_RowSetRead: {       /* jump, in1, out3 */
  i64 val;

  pIn1 = &aMem[pOp->p1];
  أكد( (pIn1->flags & MEM_Blob)==0 || sqlite3VdbeMemIsRowSet(pIn1) );
  إذا( (pIn1->flags & MEM_Blob)==0
   || sqlite3RowSetNext((RowSet*)pIn1->z, &val)==0
  ){
    /* The boolean index is empty */
    sqlite3VdbeMemSetNull(pIn1);
    VdbeBranchTaken(1,2);
    اذهب jump_to_p2_and_check_for_interrupt;
  }وإلا{
    /* A value was pulled from the index */
    VdbeBranchTaken(0,2);
    sqlite3VdbeMemSetInt64(&aMem[pOp->p3], val);
  }
  اذهب check_for_interrupt;
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
حالة OP_RowSetTest: {                     /* jump, in1, in3 */
  صحيح iSet;
  صحيح exists;

  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  iSet = pOp->p4.i;
  أكد( pIn3->flags&MEM_Int );

  /* If there is anything other than a rowset object in memory cell P1,
  ** delete it now and initialize P1 with an empty rowset
  */
  إذا( (pIn1->flags & MEM_Blob)==0 ){
    إذا( sqlite3VdbeMemSetRowSet(pIn1) ) اذهب no_mem;
  }
  أكد( sqlite3VdbeMemIsRowSet(pIn1) );
  أكد( pOp->p4type==P4_INT32 );
  أكد( iSet==-1 || iSet>=0 );
  إذا( iSet ){
    exists = sqlite3RowSetTest((RowSet*)pIn1->z, iSet, pIn3->u.i);
    VdbeBranchTaken(exists!=0,2);
    إذا( exists ) اذهب jump_to_p2;
  }
  إذا( iSet>=0 ){
    sqlite3_Row_Set_قىستۇرۇش((RowSet*)pIn1->z, pIn3->u.i);
  }
  اكسر;
}


#ifndef SQLITE_OMIT_TRIGGER

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
حالة OP_Program: {        /* jump0 */
  صحيح nMem;               /* Number of memory registers for sub-program */
  i64 nByte;              /* Bytes of runtime space required for sub-program */
  Mem *pRt;               /* Register to allocate runtime space */
  Mem *pMem;              /* Used to iterate through memory cells */
  Mem *pEnd;              /* Last memory cell in new array */
  VdbeFrame *pFrame;      /* New vdbe frame to execute in */
  SubProgram *pProgram;   /* Sub-program to execute */
  فراغ *t;                /* Token identifying trigger */

  pProgram = pOp->p4.pProgram;
  pRt = &aMem[pOp->p3];
  أكد( pProgram->nOp>0 );
 
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
  إذا( pOp->p5 ){
    t = pProgram->token;
    لكل(pFrame=p->pFrame; pFrame && pFrame->token!=t; pFrame=pFrame->pParent);
    إذا( pFrame ) اكسر;
  }

  إذا( p->nFrame>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    rc = SQLITE_ERROR;
    sqlite3VdbeError(p, "triggers nested too deep");
    اذهب abort_due_to_error;
  }

  /* Register pRt is used to store the memory required to save the state
  ** of the current program, and the memory required at runtime to execute
  ** the trigger program. If this trigger has been fired before, then pRt
  ** is already allocated. Otherwise, it must be initialized.  */
  إذا( (pRt->flags&MEM_Blob)==0 ){
    /* SubProgram.nMem is set to the number of memory cells used by the
    ** program stored in SubProgram.aOp. As well as these, one memory
    ** cell is required for each cursor used by the program. Set local
    ** variable nMem (and later, VdbeFrame.nChildMem) to this value.
    */
    nMem = pProgram->nMem + pProgram->nCsr;
    أكد( nMem>0 );
    إذا( pProgram->nCsr==0 ) nMem++;
    nByte = ROUND8(حجم(VdbeFrame))
              + nMem * حجم(Mem)
              + pProgram->nCsr * حجم(VdbeCursor*)
              + (7 + (i64)pProgram->nOp)/8;
    pFrame = sqlite3DbMallocZero(db, nByte);
    إذا( !pFrame ){
      اذهب no_mem;
    }
    sqlite3VdbeMemRelease(pRt);
    pRt->flags = MEM_Blob|MEM_Dyn;
    pRt->z = (حرف*)pFrame;
    pRt->n = (صحيح)nByte;
    pRt->xDel = sqlite3VdbeFrameMemDel;

    pFrame->v = p;
    pFrame->nChildMem = nMem;
    pFrame->nChildCsr = pProgram->nCsr;
    pFrame->pc = (صحيح)(pOp - aOp);
    pFrame->aMem = p->aMem;
    pFrame->nMem = p->nMem;
    pFrame->apCsr = p->apCsr;
    pFrame->nCursor = p->nCursor;
    pFrame->aOp = p->aOp;
    pFrame->nOp = p->nOp;
    pFrame->token = pProgram->token;
#ifdef SQLITE_DEBUG
    pFrame->iFrameMagic = SQLITE_FRAME_MAGIC;
#endif

    pEnd = &VdbeFrameMem(pFrame)[pFrame->nChildMem];
    لكل(pMem=VdbeFrameMem(pFrame); pMem!=pEnd; pMem++){
      pMem->flags = MEM_Undefined;
      pMem->db = db;
    }
  }وإلا{
    pFrame = (VdbeFrame*)pRt->z;
    أكد( pRt->xDel==sqlite3VdbeFrameMemDel );
    أكد( pProgram->nMem+pProgram->nCsr==pFrame->nChildMem
        || (pProgram->nCsr==0 && pProgram->nMem+1==pFrame->nChildMem) );
    أكد( pProgram->nCsr==pFrame->nChildCsr );
    أكد( (صحيح)(pOp - aOp)==pFrame->pc );
  }

  p->nFrame++;
  pFrame->pParent = p->pFrame;
  pFrame->lastRowid = db->lastRowid;
  pFrame->nChange = p->nChange;
  pFrame->nDbChange = p->db->nChange;
  أكد( pFrame->pAuxData==0 );
  pFrame->pAuxData = p->pAuxData;
  p->pAuxData = 0;
  p->nChange = 0;
  p->pFrame = pFrame;
  p->aMem = aMem = VdbeFrameMem(pFrame);
  p->nMem = pFrame->nChildMem;
  p->nCursor = (u16)pFrame->nChildCsr;
  p->apCsr = (VdbeCursor **)&aMem[p->nMem];
  pFrame->aOnce = (u8*)&p->apCsr[pProgram->nCsr];
  memset(pFrame->aOnce, 0, (pProgram->nOp + 7)/8);
  p->aOp = aOp = pProgram->aOp;
  p->nOp = pProgram->nOp;
#ifdef SQLITE_DEBUG
  /* Verify that second and subsequent executions of the same trigger do not
  ** try to reuse register values from the first use. */
  {
    صحيح i;
    لكل(i=0; i<p->nMem; i++){
      aMem[i].pScopyFrom = 0;  /* Prevent false-positive AboutToChange() errs */
      MemSetTypeFlag(&aMem[i], MEM_Undefined); /* Fault if this reg is reused */
    }
  }
#endif
  pOp = &aOp[-1];
  اذهب check_for_interrupt;
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
حالة OP_Param: {           /* out2 */
  VdbeFrame *pFrame;
  Mem *pIn;
  pOut = out2Prerelease(p, pOp);
  pFrame = p->pFrame;
  pIn = &pFrame->aMem[pOp->p1 + pFrame->aOp[pFrame->pc].p1];  
  sqlite3VdbeMemShallowCopy(pOut, pIn, MEM_Ephem);
  اكسر;
}

#endif /* #ifndef SQLITE_OMIT_TRIGGER */

#ifndef SQLITE_OMIT_FOREIGN_ئاچقۇچ
/* Opcode: FkCounter P1 P2 * * *
** Synopsis: fkctr[P1]+=P2
**
** Increment a "constraint counter" by P2 (P2 may be negative or positive).
** If P1 is non-zero, the database constraint counter is incremented
** (deferred foreign key constraints). Otherwise, if P1 is zero, the
** statement counter is incremented (immediate foreign key constraints).
*/
حالة OP_FkCounter: {
  إذا( pOp->p1 ){
    db->nDeferredCons += pOp->p2;
  }وإلا{
    إذا( db->flags & SQLITE_DeferFKs ){
      db->nDeferredImmCons += pOp->p2;
    }وإلا{
      p->nFkConstraint += pOp->p2;
    }
  }
  اكسر;
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
حالة OP_FkIfZero: {         /* jump */
  إذا( pOp->p1 ){
    VdbeBranchTaken(db->nDeferredCons==0 && db->nDeferredImmCons==0, 2);
    إذا( db->nDeferredCons==0 && db->nDeferredImmCons==0 ) اذهب jump_to_p2;
  }وإلا{
    VdbeBranchTaken(p->nFkConstraint==0 && db->nDeferredImmCons==0, 2);
    إذا( p->nFkConstraint==0 && db->nDeferredImmCons==0 ) اذهب jump_to_p2;
  }
  اكسر;
}
#endif /* #ifndef SQLITE_OMIT_FOREIGN_KEY */

#ifndef SQLITE_OMIT_AUTOINCREMENT
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
حالة OP_MemMax: {        /* in2 */
  VdbeFrame *pFrame;
  إذا( p->pFrame ){
    لكل(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    pIn1 = &pFrame->aMem[pOp->p1];
  }وإلا{
    pIn1 = &aMem[pOp->p1];
  }
  أكد( memIsValid(pIn1) );
  sqlite3VdbeMemIntegerify(pIn1);
  pIn2 = &aMem[pOp->p2];
  sqlite3VdbeMemIntegerify(pIn2);
  إذا( pIn1->u.i<pIn2->u.i){
    pIn1->u.i = pIn2->u.i;
  }
  اكسر;
}
#endif /* SQLITE_OMIT_AUTOINCREMENT */

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
حالة OP_IfPos: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  أكد( pIn1->flags&MEM_Int );
  VdbeBranchTaken( pIn1->u.i>0, 2);
  إذا( pIn1->u.i>0 ){
    pIn1->u.i -= pOp->p3;
    اذهب jump_to_p2;
  }
  اكسر;
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
حالة OP_OffsetLimit: {    /* in1, out2, in3 */
  i64 x;
  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  pOut = out2Prerelease(p, pOp);
  أكد( pIn1->flags & MEM_Int );
  أكد( pIn3->flags & MEM_Int );
  x = pIn1->u.i;
  إذا( x<=0 || sqlite3AddInt64(&x, pIn3->u.i>0?pIn3->u.i:0) ){
    /* If the LIMIT is less than or equal to zero, loop forever.  This
    ** is documented.  But also, if the LIMIT+OFFSET exceeds 2^63 then
    ** also loop forever.  This is undocumented.  In fact, one could argue
    ** that the loop should terminate.  But assuming 1 billion iterations
    ** per second (far exceeding the capabilities of any current hardware)
    ** it would take nearly 300 years to actually reach the limit.  So
    ** looping forever is a reasonable approximation. */
    pOut->u.i = -1;
  }وإلا{
    pOut->u.i = x;
  }
  اكسر;
}

/* Opcode: IfNotZero P1 P2 * * *
** Synopsis: if r[P1]!=0 then r[P1]--, goto P2
**
** Register P1 must contain an integer.  If the content of register P1 is
** initially greater than zero, then decrement the value in register P1.
** If it is non-zero (negative or positive) and then also jump to P2. 
** If register P1 is initially zero, leave it unchanged and fall through.
*/
حالة OP_IfNotZero: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  أكد( pIn1->flags&MEM_Int );
  VdbeBranchTaken(pIn1->u.i<0, 2);
  إذا( pIn1->u.i ){
     إذا( pIn1->u.i>0 ) pIn1->u.i--;
     اذهب jump_to_p2;
  }
  اكسر;
}

/* Opcode: DecrJumpZero P1 P2 * * *
** Synopsis: if (--r[P1])==0 goto P2
**
** Register P1 must hold an integer.  Decrement the value in P1
** and jump to P2 if the new value is exactly zero.
*/
حالة OP_DecrJumpZero: {      /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  أكد( pIn1->flags&MEM_Int );
  إذا( pIn1->u.i>SMALLEST_INT64 ) pIn1->u.i--;
  VdbeBranchTaken(pIn1->u.i==0, 2);
  إذا( pIn1->u.i==0 ) اذهب jump_to_p2;
  اكسر;
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
حالة OP_AggInverse:
حالة OP_AggStep: {
  صحيح n;
  sqlite3_context *pCtx;
  u64 nAlloc;

  أكد( pOp->p4type==P4_FUNCDEF );
  n = pOp->p5;
  أكد( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  أكد( n==0 || (pOp->p2>0 && pOp->p2+n<=(p->nMem+1 - p->nCursor)+1) );
  أكد( pOp->p3<pOp->p2 || pOp->p3>=pOp->p2+n );

  /* Allocate space for (a) the context object and (n-1) extra pointers
  ** to append to the sqlite3_context.argv[1] array, and (b) a memory
  ** cell in which to store the accumulation. Be careful that the memory
  ** cell is 8-byte aligned, even on platforms where a pointer is 32-bits.
  **
  ** Note: We could avoid this by using a regular memory cell from aMem[] for 
  ** the accumulator, instead of allocating one here. */
  nAlloc = ROUND8P( SZ_CONTEXT(n) );
  pCtx = sqlite3DbMallocRawNN(db, nAlloc + حجم(Mem));
  إذا( pCtx==0 ) اذهب no_mem;
  pCtx->pOut = (Mem*)((u8*)pCtx + nAlloc);
  أكد( EIGHT_BYTE_ALIGNMENT(pCtx->pOut) );

  sqlite3VdbeMemInit(pCtx->pOut, db, MEM_Null);
  pCtx->pMem = 0;
  pCtx->pFunc = pOp->p4.pFunc;
  pCtx->iOp = (صحيح)(pOp - aOp);
  pCtx->pVdbe = p;
  pCtx->skipFlag = 0;
  pCtx->isError = 0;
  pCtx->enc = encoding;
  pCtx->argc = n;
  pOp->p4type = P4_FUNCCTX;
  pOp->p4.pCtx = pCtx;

  /* OP_AggInverse must have P1==1 and OP_AggStep must have P1==0 */
  أكد( pOp->p1==(pOp->opcode==OP_AggInverse) );

  pOp->opcode = OP_AggStep1;
  /* Fall through into OP_AggStep */
  /* no break */ deliberate_fall_through
}
حالة OP_AggStep1: {
  صحيح i;
  sqlite3_context *pCtx;
  Mem *pMem;

  أكد( pOp->p4type==P4_FUNCCTX );
  pCtx = pOp->p4.pCtx;
  pMem = &aMem[pOp->p3];

#ifdef SQLITE_DEBUG
  إذا( pOp->p1 ){
    /* This is an OP_AggInverse call.  Verify that xStep has always
    ** been called at least once prior to any xInverse call. */
    أكد( pMem->uTemp==0x1122e0e3 );
  }وإلا{
    /* This is an OP_AggStep call.  Mark it as such. */
    pMem->uTemp = 0x1122e0e3;
  }
#endif

  /* If this function is inside of a trigger, the register array in aMem[]
  ** might change from one evaluation to the next.  The next block of code
  ** checks to see if the register array has changed, and if so it
  ** reinitializes the relevant parts of the sqlite3_context object */
  إذا( pCtx->pMem != pMem ){
    pCtx->pMem = pMem;
    لكل(i=pCtx->argc-1; i>=0; i--) pCtx->argv[i] = &aMem[pOp->p2+i];
  }

#ifdef SQLITE_DEBUG
  لكل(i=0; i<pCtx->argc; i++){
    أكد( memIsValid(pCtx->argv[i]) );
    REGISTER_TRACE(pOp->p2+i, pCtx->argv[i]);
  }
#endif

  pMem->n++;
  أكد( pCtx->pOut->flags==MEM_Null );
  أكد( pCtx->isError==0 );
  أكد( pCtx->skipFlag==0 );
#ifndef SQLITE_OMIT_WINDOWFUNC
  إذا( pOp->p1 ){
    (pCtx->pFunc->xInverse)(pCtx,pCtx->argc,pCtx->argv);
  }وإلا
#endif
  (pCtx->pFunc->xSFunc)(pCtx,pCtx->argc,pCtx->argv); /* IMP: R-24505-23230 */

  إذا( pCtx->isError ){
    إذا( pCtx->isError>0 ){
      sqlite3VdbeError(p, "%s", sqlite3_قىممەت_text(pCtx->pOut));
      rc = pCtx->isError;
    }
    إذا( pCtx->skipFlag ){
      أكد( pOp[-1].opcode==OP_CollSeq );
      i = pOp[-1].p1;
      إذا( i ) sqlite3VdbeMemSetInt64(&aMem[i], 1);
      pCtx->skipFlag = 0;
    }
    sqlite3VdbeMemRelease(pCtx->pOut);
    pCtx->pOut->flags = MEM_Null;
    pCtx->isError = 0;
    إذا( rc ) اذهب abort_due_to_error;
  }
  أكد( pCtx->pOut->flags==MEM_Null );
  أكد( pCtx->skipFlag==0 );
  اكسر;
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
حالة OP_Agg_قىممەت:
حالة OP_AggFinal: {
  Mem *pMem;
  أكد( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  أكد( pOp->p3==0 || pOp->opcode==OP_Agg_قىممەت );
  pMem = &aMem[pOp->p1];
  أكد( (pMem->flags & ~(MEM_Null|MEM_Agg))==0 );
#ifndef SQLITE_OMIT_WINDOWFUNC
  إذا( pOp->p3 ){
    memAboutToChange(p, &aMem[pOp->p3]);
    rc = sqlite3_Vdbe_Mem_Agg_قىممەت(pMem, &aMem[pOp->p3], pOp->p4.pFunc);
    pMem = &aMem[pOp->p3];
  }وإلا
#endif
  {
    rc = sqlite3VdbeMemFinalize(pMem, pOp->p4.pFunc);
  }
 
  إذا( rc ){
    sqlite3VdbeError(p, "%s", sqlite3_قىممەت_text(pMem));
    اذهب abort_due_to_error;
  }
  sqlite3VdbeChangeEncoding(pMem, encoding);
  UPDATE_MAX_BLOBSIZE(pMem);
  REGISTER_TRACE((صحيح)(pMem-aMem), pMem);
  اكسر;
}

#ifndef SQLITE_OMIT_WAL
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
حالة OP_Checkpoint: {
  صحيح i;                          /* Loop counter */
  صحيح aRes[3];                    /* Results */
  Mem *pMem;                      /* Write results here */

  أكد( p->readOnly==0 );
  aRes[0] = 0;
  aRes[1] = aRes[2] = -1;
  أكد( pOp->p2==SQLITE_CHECKPOINT_PASSIVE
       || pOp->p2==SQLITE_CHECKPOINT_FULL
       || pOp->p2==SQLITE_CHECKPOINT_RESTART
       || pOp->p2==SQLITE_CHECKPOINT_TRUNCATE
       || pOp->p2==SQLITE_CHECKPOINT_NOOP
  );
  rc = sqlite3Checkpoint(db, pOp->p1, pOp->p2, &aRes[1], &aRes[2]);
  إذا( rc ){
    إذا( rc!=SQLITE_BUSY ) اذهب abort_due_to_error;
    rc = SQLITE_OK;
    aRes[0] = 1;
  }
  لكل(i=0, pMem = &aMem[pOp->p3]; i<3; i++, pMem++){
    sqlite3VdbeMemSetInt64(pMem, (i64)aRes[i]);
  }   
  اكسر;
}; 
#endif

#ifndef SQLITE_OMIT_PRAGMA
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
حالة OP_JournalMode: {    /* out2 */
  Btree *pBt;                     /* Btree to change journal mode of */
  Pager *pPager;                  /* Pager associated with pBt */
  صحيح eNew;                       /* New journal mode */
  صحيح eOld;                       /* The old journal mode */
#ifndef SQLITE_OMIT_WAL
  ثابت حرف *zFilename;          /* Name of database file for pPager */
#endif

  pOut = out2Prerelease(p, pOp);
  eNew = pOp->p3;
  أكد( eNew==PAGER_JOURNALMODE_DELETE
       || eNew==PAGER_JOURNALMODE_TRUNCATE
       || eNew==PAGER_JOURNALMODE_PERSIST
       || eNew==PAGER_JOURNALMODE_OFF
       || eNew==PAGER_JOURNALMODE_MEMORY
       || eNew==PAGER_JOURNALMODE_WAL
       || eNew==PAGER_JOURNALMODE_QUERY
  );
  أكد( pOp->p1>=0 && pOp->p1<db->nDb );
  أكد( p->readOnly==0 );

  pBt = db->aDb[pOp->p1].pBt;
  pPager = sqlite3BtreePager(pBt);
  eOld = sqlite3PagerGetJournalMode(pPager);
  إذا( eNew==PAGER_JOURNALMODE_QUERY ) eNew = eOld;
  أكد( sqlite3BtreeHoldsMutex(pBt) );
  إذا( !sqlite3PagerOkToChangeJournalMode(pPager) ) eNew = eOld;

#ifndef SQLITE_OMIT_WAL
  zFilename = sqlite3PagerFilename(pPager, 1);

  /* Do not allow a transition to journal_mode=WAL for a database
  ** in temporary storage or if the VFS does not support shared memory
  */
  إذا( eNew==PAGER_JOURNALMODE_WAL
   && (sqlite3Strlen30(zFilename)==0           /* Temp file */
       || !sqlite3PagerWalSupported(pPager))   /* No shared-memory support */
  ){
    eNew = eOld;
  }

  إذا( (eNew!=eOld)
   && (eOld==PAGER_JOURNALMODE_WAL || eNew==PAGER_JOURNALMODE_WAL)
  ){
    إذا( !db->autoCommit || db->nVdbeRead>1 ){
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p,
          "cannot change %s wal mode from within a transaction",
          (eNew==PAGER_JOURNALMODE_WAL ? "into" : "out of")
      );
      اذهب abort_due_to_error;
    }وإلا{

      إذا( eOld==PAGER_JOURNALMODE_WAL ){
        /* If leaving WAL mode, close the log file. If successful, the call
        ** to PagerCloseWal() checkpoints and deletes the write-ahead-log
        ** file. An EXCLUSIVE lock may still be held on the database file
        ** after a successful return.
        */
        rc = sqlite3PagerCloseWal(pPager, db);
        إذا( rc==SQLITE_OK ){
          sqlite3PagerSetJournalMode(pPager, eNew);
        }
      }وإلا إذا( eOld==PAGER_JOURNALMODE_MEMORY ){
        /* Cannot transition directly from MEMORY to WAL.  Use mode OFF
        ** as an intermediate */
        sqlite3PagerSetJournalMode(pPager, PAGER_JOURNALMODE_OFF);
      }
 
      /* Open a transaction on the database file. Regardless of the journal
      ** mode, this transaction always uses a rollback journal.
      */
      أكد( sqlite3BtreeTxnState(pBt)!=SQLITE_TXN_WRITE );
      إذا( rc==SQLITE_OK ){
        rc = sqlite3BtreeSetVersion(pBt, (eNew==PAGER_JOURNALMODE_WAL ? 2 : 1));
      }
    }
  }
#endif /* ifndef SQLITE_OMIT_WAL */

  إذا( rc ) eNew = eOld;
  eNew = sqlite3PagerSetJournalMode(pPager, eNew);

  pOut->flags = MEM_Str|MEM_Static|MEM_Term;
  pOut->z = (حرف *)sqlite3JournalModename(eNew);
  pOut->n = sqlite3Strlen30(pOut->z);
  pOut->enc = SQLITE_UTF8;
  sqlite3VdbeChangeEncoding(pOut, encoding);
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
};
#endif /* SQLITE_OMIT_PRAGMA */

#إذا !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)
/* Opcode: Vacuum P1 P2 * * *
**
** Vacuum the entire database P1.  P1 is 0 for "main", and 2 or more
** for an attached database.  The "temp" database may not be vacuumed.
**
** If P2 is not zero, then it is a register holding a string which is
** the file into which the result of vacuum should be written.  When
** P2 is zero, the vacuum overwrites the original database.
*/
حالة OP_Vacuum: {
  أكد( p->readOnly==0 );
  rc = sqlite3RunVacuum(&p->z_Err_ئۇچۇر, db, pOp->p1,
                        pOp->p2 ? &aMem[pOp->p2] : 0);
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}
#endif

#إذا !defined(SQLITE_OMIT_AUTOVACUUM)
/* Opcode: IncrVacuum P1 P2 * * *
**
** Perform a single step of the incremental vacuum procedure on
** the P1 database. If the vacuum has finished, jump to instruction
** P2. Otherwise, fall through to the next instruction.
*/
حالة OP_IncrVacuum: {        /* jump */
  Btree *pBt;

  أكد( pOp->p1>=0 && pOp->p1<db->nDb );
  أكد( DbMaskTest(p->btreeMask, pOp->p1) );
  أكد( p->readOnly==0 );
  pBt = db->aDb[pOp->p1].pBt;
  rc = sqlite3BtreeIncrVacuum(pBt);
  VdbeBranchTaken(rc==SQLITE_DONE,2);
  إذا( rc ){
    إذا( rc!=SQLITE_DONE ) اذهب abort_due_to_error;
    rc = SQLITE_OK;
    اذهب jump_to_p2;
  }
  اكسر;
}
#endif

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
حالة OP_Expire: {
  أكد( pOp->p2==0 || pOp->p2==1 );
  إذا( !pOp->p1 ){
    sqlite3ExpirePreparedStatements(db, pOp->p2);
  }وإلا{
    p->expired = pOp->p2+1;
  }
  اكسر;
}

/* Opcode: CursorLock P1 * * * *
**
** Lock the btree to which cursor P1 is pointing so that the btree cannot be
** written by an other cursor.
*/
حالة OP_CursorLock: {
  VdbeCursor *pC;
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  sqlite3BtreeCursorPin(pC->uc.pCursor);
  اكسر;
}

/* Opcode: CursorUnlock P1 * * * *
**
** Unlock the btree to which cursor P1 is pointing so that it can be
** written by other cursors.
*/
حالة OP_CursorUnlock: {
  VdbeCursor *pC;
  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );
  pC = p->apCsr[pOp->p1];
  أكد( pC!=0 );
  أكد( pC->eCurType==CURTYPE_BTREE );
  sqlite3BtreeCursorUnpin(pC->uc.pCursor);
  اكسر;
}

#ifndef SQLITE_OMIT_SHARED_CACHE
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
حالة OP_TableLock: {
  u8 isWriteLock = (u8)pOp->p3;
  إذا( isWriteLock || 0==(db->flags&SQLITE_ReadUncommit) ){
    صحيح p1 = pOp->p1;
    أكد( p1>=0 && p1<db->nDb );
    أكد( DbMaskTest(p->btreeMask, p1) );
    أكد( isWriteLock==0 || isWriteLock==1 );
    rc = sqlite3BtreeLockTable(db->aDb[p1].pBt, pOp->p2, isWriteLock);
    إذا( rc ){
      إذا( (rc&0xFF)==SQLITE_LOCKED ){
        ثابت حرف *z = pOp->p4.z;
        sqlite3VdbeError(p, "database table is locked: %s", z);
      }
      اذهب abort_due_to_error;
    }
  }
  اكسر;
}
#endif /* SQLITE_OMIT_SHARED_CACHE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VBegin * * * P4 *
**
** P4 may be a pointer to an sqlite3_vtab structure. If so, call the
** xBegin method for that table.
**
** Also, whether or not P4 is set, check that this is not being called from
** within a callback to a virtual table xSync() method. If it is, the error
** code will be set to SQLITE_LOCKED.
*/
حالة OP_VBegin: {
  VTable *pVTab;
  pVTab = pOp->p4.pVtab;
  rc = sqlite3VtabBegin(db, pVTab);
  إذا( pVTab ) sqlite3VtabImportErrmsg(p, pVTab->pVtab);
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VCreate P1 P2 * * *
**
** P2 is a register that holds the name of a virtual table in database
** P1. Call the xCreate method for that table.
*/
حالة OP_VCreate: {
  Mem sMem;          /* For storing the record being decoded */
  ثابت حرف *zTab;  /* Name of the virtual table */

  memset(&sMem, 0, حجم(sMem));
  sMem.db = db;
  /* Because P2 is always a static string, it is impossible for the
  ** sqlite3VdbeMemCopy() to fail */
  أكد( (aMem[pOp->p2].flags & MEM_Str)!=0 );
  أكد( (aMem[pOp->p2].flags & MEM_Static)!=0 );
  rc = sqlite3VdbeMemCopy(&sMem, &aMem[pOp->p2]);
  أكد( rc==SQLITE_OK );
  zTab = (ثابت حرف*)sqlite3_قىممەت_text(&sMem);
  أكد( zTab || db->mallocFailed );
  إذا( zTab ){
    rc = sqlite3VtabCallCreate(db, pOp->p1, zTab, &p->z_Err_ئۇچۇر);
  }
  sqlite3VdbeMemRelease(&sMem);
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VDestroy P1 * * P4 *
**
** P4 is the name of a virtual table in database P1.  Call the xDestroy method
** of that table.
*/
حالة OP_VDestroy: {
  db->nVDestroy++;
  rc = sqlite3VtabCallDestroy(db, pOp->p1, pOp->p4.z);
  db->nVDestroy--;
  أكد( p->errorAction==OE_Abort && p->usesStmtJournal );
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VOpen P1 * * P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** P1 is a cursor number.  This opcode opens a cursor to the virtual
** table and stores that cursor in P1.
*/
حالة OP_VOpen: {             /* ncycle */
  VdbeCursor *pCur;
  sqlite3_vtab_cursor *pVCur;
  sqlite3_vtab *pVtab;
  ثابت sqlite3_module *pModule;

  أكد( p->bIsReader );
  pCur = p->apCsr[pOp->p1];
  إذا( pCur!=0
   && ALWAYS( pCur->eCurType==CURTYPE_VTAB )
   && ALWAYS( pCur->uc.pVCur->pVtab==pOp->p4.pVtab->pVtab )
  ){
    /* This opcode is a no-op if the cursor is already open */
    اكسر;
  }
  pVCur = 0;
  pVtab = pOp->p4.pVtab->pVtab;
  إذا( pVtab==0 || NEVER(pVtab->pModule==0) ){
    rc = SQLITE_LOCKED;
    اذهب abort_due_to_error;
  }
  pModule = pVtab->pModule;
  rc = pModule->xOpen(pVtab, &pVCur);
  sqlite3VtabImportErrmsg(p, pVtab);
  إذا( rc ) اذهب abort_due_to_error;

  /* Initialize sqlite3_vtab_cursor base class */
  pVCur->pVtab = pVtab;

  /* Initialize vdbe cursor object */
  pCur = allocateCursor(p, pOp->p1, 0, CURTYPE_VTAB);
  إذا( pCur ){
    pCur->uc.pVCur = pVCur;
    pVtab->nRef++;
  }وإلا{
    أكد( db->mallocFailed );
    pModule->xClose(pVCur);
    اذهب no_mem;
  }
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VCheck P1 P2 P3 P4 *
**
** P4 is a pointer to a Table object that is a virtual table in schema P1
** that supports the xIntegrity() method.  This opcode runs the xIntegrity()
** method for that virtual table, using P3 as the integer argument.  If
** an error is reported back, the table name is prepended to the error
** message and that message is stored in P2.  If no errors are seen,
** register P2 is set to NULL.
*/
حالة OP_VCheck: {             /* out2 */
  Table *pTab;
  sqlite3_vtab *pVtab;
  ثابت sqlite3_module *pModule;
  حرف *zErr = 0;

  pOut = &aMem[pOp->p2];
  sqlite3VdbeMemSetNull(pOut);  /* Innocent until proven guilty */
  أكد( pOp->p4type==P4_TABLEREF );
  pTab = pOp->p4.pTab;
  أكد( pTab!=0 );
  أكد( pTab->nTabRef>0 );
  أكد( IsVirtual(pTab) );
  إذا( pTab->u.vtab.p==0 ) اكسر;
  pVtab = pTab->u.vtab.p->pVtab;
  أكد( pVtab!=0 );
  pModule = pVtab->pModule;
  أكد( pModule!=0 );
  أكد( pModule->iVersion>=4 );
  أكد( pModule->xIntegrity!=0 );
  sqlite3VtabLock(pTab->u.vtab.p);
  أكد( pOp->p1>=0 && pOp->p1<db->nDb );
  rc = pModule->xIntegrity(pVtab, db->aDb[pOp->p1].zDbSName, pTab->zName,
                           pOp->p3, &zErr);
  sqlite3VtabUnlock(pTab->u.vtab.p);
  إذا( rc ){
    sqlite3_free(zErr);
    اذهب abort_due_to_error;
  }
  إذا( zErr ){
    sqlite3VdbeMemSetStr(pOut, zErr, -1, SQLITE_UTF8, sqlite3_free);
  }
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
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
حالة OP_VInitIn: {        /* out2, ncycle */
  VdbeCursor *pC;         /* The cursor containing the RHS values */
  قىممەت_List *pRhs;        /* New ValueList object to put in reg[P2] */

  pC = p->apCsr[pOp->p1];
  pRhs = sqlite3_malloc64( حجم(*pRhs) );
  إذا( pRhs==0 ) اذهب no_mem;
  pRhs->pCsr = pC->uc.pCursor;
  pRhs->pOut = &aMem[pOp->p3];
  pOut = out2Prerelease(p, pOp);
  pOut->flags = MEM_Null;
  sqlite3VdbeMemSetPointer(pOut, pRhs, "ValueList", sqlite3_Vdbe_قىممەت_List_Free);
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */


#ifndef SQLITE_OMIT_VIRTUALTABLE
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
حالة OP_VFilter: {   /* jump, ncycle */
  صحيح nArg;
  صحيح iQuery;
  ثابت sqlite3_module *pModule;
  Mem *pQuery;
  Mem *pArgc;
  sqlite3_vtab_cursor *pVCur;
  sqlite3_vtab *pVtab;
  VdbeCursor *pCur;
  صحيح res;
  صحيح i;
  Mem **apArg;

  pQuery = &aMem[pOp->p3];
  pArgc = &pQuery[1];
  pCur = p->apCsr[pOp->p1];
  أكد( memIsValid(pQuery) );
  REGISTER_TRACE(pOp->p3, pQuery);
  أكد( pCur!=0 );
  أكد( pCur->eCurType==CURTYPE_VTAB );
  pVCur = pCur->uc.pVCur;
  pVtab = pVCur->pVtab;
  pModule = pVtab->pModule;

  /* Grab the index number and argc parameters */
  أكد( (pQuery->flags&MEM_Int)!=0 && pArgc->flags==MEM_Int );
  nArg = (صحيح)pArgc->u.i;
  iQuery = (صحيح)pQuery->u.i;

  /* Invoke the xFilter method */
  apArg = p->apArg;
  أكد( nArg<=p->napArg );
  لكل(i = 0; i<nArg; i++){
    apArg[i] = &pArgc[i+1];
  }
  rc = pModule->xFilter(pVCur, iQuery, pOp->p4.z, nArg, apArg);
  sqlite3VtabImportErrmsg(p, pVtab);
  إذا( rc ) اذهب abort_due_to_error;
  res = pModule->xEof(pVCur);
  pCur->nullRow = 0;
  VdbeBranchTaken(res!=0,2);
  إذا( res ) اذهب jump_to_p2;
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
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
حالة OP_VColumn: {           /* ncycle */
  sqlite3_vtab *pVtab;
  ثابت sqlite3_module *pModule;
  Mem *pDest;
  sqlite3_context sContext;
  FuncDef nullFunc;

  VdbeCursor *pCur = p->apCsr[pOp->p1];
  أكد( pCur!=0 );
  أكد( pOp->p3>0 && pOp->p3<=(p->nMem+1 - p->nCursor) );
  pDest = &aMem[pOp->p3];
  memAboutToChange(p, pDest);
  إذا( pCur->nullRow ){
    sqlite3VdbeMemSetNull(pDest);
    اكسر;
  }
  أكد( pCur->eCurType==CURTYPE_VTAB );
  pVtab = pCur->uc.pVCur->pVtab;
  pModule = pVtab->pModule;
  أكد( pModule->xColumn );
  memset(&sContext, 0, حجم(sContext));
  sContext.pOut = pDest;
  sContext.enc = encoding;
  nullFunc.pUserData = 0;
  nullFunc.funcFlags = SQLITE_RESULT_SUBTYPE;
  sContext.pFunc = &nullFunc;
  أكد( pOp->p5==OPFLAG_NOCHNG || pOp->p5==0 );
  إذا( pOp->p5 & OPFLAG_NOCHNG ){
    sqlite3VdbeMemSetNull(pDest);
    pDest->flags = MEM_Null|MEM_Zero;
    pDest->u.nZero = 0;
  }وإلا{
    MemSetTypeFlag(pDest, MEM_Null);
  }
  rc = pModule->xColumn(pCur->uc.pVCur, &sContext, pOp->p2);
  sqlite3VtabImportErrmsg(p, pVtab);
  إذا( sContext.isError>0 ){
    sqlite3VdbeError(p, "%s", sqlite3_قىممەت_text(pDest));
    rc = sContext.isError;
  }
  sqlite3VdbeChangeEncoding(pDest, encoding);
  REGISTER_TRACE(pOp->p3, pDest);
  UPDATE_MAX_BLOBSIZE(pDest);

  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VNext P1 P2 * * *
**
** Advance virtual table P1 to the next row in its result set and
** jump to instruction P2.  Or, if the virtual table has reached
** the end of its result set, then fall through to the next instruction.
*/
حالة OP_VNext: {   /* jump, ncycle */
  sqlite3_vtab *pVtab;
  ثابت sqlite3_module *pModule;
  صحيح res;
  VdbeCursor *pCur;

  pCur = p->apCsr[pOp->p1];
  أكد( pCur!=0 );
  أكد( pCur->eCurType==CURTYPE_VTAB );
  إذا( pCur->nullRow ){
    اكسر;
  }
  pVtab = pCur->uc.pVCur->pVtab;
  pModule = pVtab->pModule;
  أكد( pModule->xNext );

  /* Invoke the xNext() method of the module. There is no way for the
  ** underlying implementation to return an error if one occurs during
  ** xNext(). Instead, if an error occurs, true is returned (indicating that
  ** data is available) and the error code returned when xColumn or
  ** some other method is next invoked on the save virtual table cursor.
  */
  rc = pModule->xNext(pCur->uc.pVCur);
  sqlite3VtabImportErrmsg(p, pVtab);
  إذا( rc ) اذهب abort_due_to_error;
  res = pModule->xEof(pCur->uc.pVCur);
  VdbeBranchTaken(!res,2);
  إذا( !res ){
    /* If there is data, jump to P2 */
    اذهب jump_to_p2_and_check_for_interrupt;
  }
  اذهب check_for_interrupt;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VRename P1 * * P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** This opcode invokes the corresponding xRename method. The value
** in register P1 is passed as the zName argument to the xRename method.
*/
حالة OP_VRename: {
  sqlite3_vtab *pVtab;
  Mem *pName;
  صحيح isLegacy;
 
  isLegacy = (db->flags & SQLITE_LegacyAlter);
  db->flags |= SQLITE_LegacyAlter;
  pVtab = pOp->p4.pVtab->pVtab;
  pName = &aMem[pOp->p1];
  أكد( pVtab->pModule->xRename );
  أكد( memIsValid(pName) );
  أكد( p->readOnly==0 );
  REGISTER_TRACE(pOp->p1, pName);
  أكد( pName->flags & MEM_Str );
  testcase( pName->enc==SQLITE_UTF8 );
  testcase( pName->enc==SQLITE_UTF16BE );
  testcase( pName->enc==SQLITE_UTF16LE );
  rc = sqlite3VdbeChangeEncoding(pName, SQLITE_UTF8);
  إذا( rc ) اذهب abort_due_to_error;
  rc = pVtab->pModule->xRename(pVtab, pName->z);
  إذا( isLegacy==0 ) db->flags &= ~(u64)SQLITE_LegacyAlter;
  sqlite3VtabImportErrmsg(p, pVtab);
  p->expired = 0;
  إذا( rc ) اذهب abort_due_to_error;
  اكسر;
}
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
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
حالة OP_VUpdate: {
  sqlite3_vtab *pVtab;
  ثابت sqlite3_module *pModule;
  صحيح nArg;
  صحيح i;
  sqlite_int64 rowid = 0;
  Mem **apArg;
  Mem *pX;

  أكد( pOp->p2==1        || pOp->p5==OE_Fail   || pOp->p5==OE_Rollback
       || pOp->p5==OE_Abort || pOp->p5==OE_Ignore || pOp->p5==OE_Replace
  );
  أكد( p->readOnly==0 );
  إذا( db->mallocFailed ) اذهب no_mem;
  sqlite3VdbeIncrWriteCounter(p, 0);
  pVtab = pOp->p4.pVtab->pVtab;
  إذا( pVtab==0 || NEVER(pVtab->pModule==0) ){
    rc = SQLITE_LOCKED;
    اذهب abort_due_to_error;
  }
  pModule = pVtab->pModule;
  nArg = pOp->p2;
  أكد( pOp->p4type==P4_VTAB );
  إذا( ALWAYS(pModule->xUpdate) ){
    u8 vtabOnConflict = db->vtabOnConflict;
    apArg = p->apArg;
    pX = &aMem[pOp->p3];
    أكد( nArg<=p->napArg );
    لكل(i=0; i<nArg; i++){
      أكد( memIsValid(pX) );
      memAboutToChange(p, pX);
      apArg[i] = pX;
      pX++;
    }
    db->vtabOnConflict = pOp->p5;
    rc = pModule->xUpdate(pVtab, nArg, apArg, &rowid);
    db->vtabOnConflict = vtabOnConflict;
    sqlite3VtabImportErrmsg(p, pVtab);
    إذا( rc==SQLITE_OK && pOp->p1 ){
      أكد( nArg>1 && apArg[0] && (apArg[0]->flags&MEM_Null) );
      db->lastRowid = rowid;
    }
    إذا( (rc&0xff)==SQLITE_CONSTRAINT && pOp->p4.pVtab->bConstraint ){
      إذا( pOp->p5==OE_Ignore ){
        rc = SQLITE_OK;
      }وإلا{
        p->errorAction = ((pOp->p5==OE_Replace) ? OE_Abort : pOp->p5);
      }
    }وإلا{
      p->nChange++;
    }
    إذا( rc ) اذهب abort_due_to_error;
  }
  اكسر;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef  SQLITE_OMIT_PAGER_PRAGMAS
/* Opcode: Pagecount P1 P2 * * *
**
** Write the current number of pages in database P1 to memory cell P2.
*/
حالة OP_Pagecount: {            /* out2 */
  pOut = out2Prerelease(p, pOp);
  pOut->u.i = sqlite3BtreeLastPage(db->aDb[pOp->p1].pBt);
  اكسر;
}
#endif


#ifndef  SQLITE_OMIT_PAGER_PRAGMAS
/* Opcode: MaxPgcnt P1 P2 P3 * *
**
** Try to set the maximum page count for database P1 to the value in P3.
** Do not let the maximum page count fall below the current page count and
** do not change the maximum page count value if P3==0.
**
** Store the maximum page count after the change in register P2.
*/
حالة OP_MaxPgcnt: {            /* out2 */
  unsigned صحيح newMax;
  Btree *pBt;

  pOut = out2Prerelease(p, pOp);
  pBt = db->aDb[pOp->p1].pBt;
  newMax = 0;
  إذا( pOp->p3 ){
    newMax = sqlite3BtreeLastPage(pBt);
    إذا( newMax < (unsigned)pOp->p3 ) newMax = (unsigned)pOp->p3;
  }
  pOut->u.i = sqlite3_Btree_Max_Page_سانى(pBt, newMax);
  اكسر;
}
#endif

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
حالة OP_PureFunc:              /* group */
حالة OP_Function: {            /* group */
  صحيح i;
  sqlite3_context *pCtx;

  أكد( pOp->p4type==P4_FUNCCTX );
  pCtx = pOp->p4.pCtx;

  /* If this function is inside of a trigger, the register array in aMem[]
  ** might change from one evaluation to the next.  The next block of code
  ** checks to see if the register array has changed, and if so it
  ** reinitializes the relevant parts of the sqlite3_context object */
  pOut = &aMem[pOp->p3];
  إذا( pCtx->pOut != pOut ){
    pCtx->pVdbe = p;
    pCtx->pOut = pOut;
    pCtx->enc = encoding;
    لكل(i=pCtx->argc-1; i>=0; i--) pCtx->argv[i] = &aMem[pOp->p2+i];
  }
  أكد( pCtx->pVdbe==p );

  memAboutToChange(p, pOut);
#ifdef SQLITE_DEBUG
  لكل(i=0; i<pCtx->argc; i++){
    أكد( memIsValid(pCtx->argv[i]) );
    REGISTER_TRACE(pOp->p2+i, pCtx->argv[i]);
  }
#endif
  MemSetTypeFlag(pOut, MEM_Null);
  أكد( pCtx->isError==0 );
  (*pCtx->pFunc->xSFunc)(pCtx, pCtx->argc, pCtx->argv);/* IMP: R-24505-23230 */

  /* If the function returned an error, throw an exception */
  إذا( pCtx->isError ){
    إذا( pCtx->isError>0 ){
      sqlite3VdbeError(p, "%s", sqlite3_قىممەت_text(pOut));
      rc = pCtx->isError;
    }
    sqlite3VdbeDeleteAuxData(db, &p->pAuxData, pCtx->iOp, pOp->p1);
    pCtx->isError = 0;
    إذا( rc ) اذهب abort_due_to_error;
  }

  أكد( (pOut->flags&MEM_Str)==0
       || pOut->enc==encoding
       || db->mallocFailed );
  أكد( !sqlite3VdbeMemTooBig(pOut) );

  REGISTER_TRACE(pOp->p3, pOut);
  UPDATE_MAX_BLOBSIZE(pOut);
  اكسر;
}

/* Opcode: ClrSubtype P1 * * * *
** Synopsis:  r[P1].subtype = 0
**
** Clear the subtype from register P1.
*/
حالة OP_ClrSubtype: {   /* in1 */
  pIn1 = &aMem[pOp->p1];
  pIn1->flags &= ~MEM_Subtype;
  اكسر;
}

/* Opcode: GetSubtype P1 P2 * * *
** Synopsis:  r[P2] = r[P1].subtype
**
** Extract the subtype value from register P1 and write that subtype
** into register P2.  If P1 has no subtype, then P1 gets a NULL.
*/
حالة OP_GetSubtype: {   /* in1 out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  إذا( pIn1->flags & MEM_Subtype ){
    sqlite3VdbeMemSetInt64(pOut, pIn1->eSubtype);
  }وإلا{
    sqlite3VdbeMemSetNull(pOut);
  }
  اكسر;
}

/* Opcode: SetSubtype P1 P2 * * *
** Synopsis:  r[P2].subtype = r[P1]
**
** Set the subtype value of register P2 to the integer from register P1.
** If P1 is NULL, clear the subtype from p2.
*/
حالة OP_SetSubtype: {   /* in1 out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  إذا( pIn1->flags & MEM_Null ){
    pOut->flags &= ~MEM_Subtype;
  }وإلا{
    أكد( pIn1->flags & MEM_Int );
    pOut->flags |= MEM_Subtype;
    pOut->eSubtype = (u8)(pIn1->u.i & 0xff);
  }
  اكسر;
}

/* Opcode: FilterAdd P1 * P3 P4 *
** Synopsis: filter(P1) += key(P3@P4)
**
** Compute a hash on the P4 registers starting with r[P3] and
** add that hash to the bloom filter contained in r[P1].
*/
حالة OP_FilterAdd: {
  u64 h;

  أكد( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  أكد( pIn1->flags & MEM_Blob );
  أكد( pIn1->n>0 );
  h = filterHash(aMem, pOp);
#ifdef SQLITE_DEBUG
  إذا( db->flags&SQLITE_VdbeTrace ){
    صحيح ii;
    لكل(ii=pOp->p3; ii<pOp->p3+pOp->p4.i; ii++){
      registerTrace(ii, &aMem[ii]);
    }
    printf("hash: %llu modulo %d -> %u\n", h, pIn1->n, (صحيح)(h%pIn1->n));
  }
#endif
  h %= (pIn1->n*8);
  pIn1->z[h/8] |= 1<<(h&7);
  اكسر;
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
حالة OP_Filter: {          /* jump */
  u64 h;

  أكد( pOp->p1>0 && pOp->p1<=(p->nMem+1 - p->nCursor) );
  pIn1 = &aMem[pOp->p1];
  أكد( (pIn1->flags & MEM_Blob)!=0 );
  أكد( pIn1->n >= 1 );
  h = filterHash(aMem, pOp);
#ifdef SQLITE_DEBUG
  إذا( db->flags&SQLITE_VdbeTrace ){
    صحيح ii;
    لكل(ii=pOp->p3; ii<pOp->p3+pOp->p4.i; ii++){
      registerTrace(ii, &aMem[ii]);
    }
    printf("hash: %llu modulo %d -> %u\n", h, pIn1->n, (صحيح)(h%pIn1->n));
  }
#endif
  h %= (pIn1->n*8);
  إذا( (pIn1->z[h/8] & (1<<(h&7)))==0 ){
    VdbeBranchTaken(1, 2);
    p->aCounter[SQLITE_STMTSTATUS_FILTER_HIT]++;
    اذهب jump_to_p2;
  }وإلا{
    p->aCounter[SQLITE_STMTSTATUS_FILTER_MISS]++;
    VdbeBranchTaken(0, 2);
  }
  اكسر;
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
حالة OP_Trace:
حالة OP_Init: {          /* jump0 */
  صحيح i;
#ifndef SQLITE_OMIT_TRACE
  حرف *zTrace;
#endif

  /* If the P4 argument is not NULL, then it must be an SQL comment string.
  ** The "--" string is broken up to prevent false-positives with srcck1.c.
  **
  ** This assert() provides evidence for:
  ** EVIDENCE-OF: R-50676-09860 The callback can compute the same text that
  ** would have been returned by the legacy sqlite3_trace() interface by
  ** using the X argument when X begins with "--" and invoking
  ** sqlite3_expanded_sql(P) otherwise.
  */
  أكد( pOp->p4.z==0 || strncmp(pOp->p4.z, "-" "- ", 3)==0 );

  /* OP_Init is always instruction 0 */
  أكد( pOp==p->aOp || pOp->opcode==OP_Trace );

#ifndef SQLITE_OMIT_TRACE
  إذا( (db->mTrace & (SQLITE_TRACE_STMT|SQLITE_TRACE_LEGACY))!=0
   && p->minWriteFileFormat!=254  /* tag-20220401a */
   && (zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql))!=0
  ){
#ifndef SQLITE_OMIT_DEPRECATED
    إذا( db->mTrace & SQLITE_TRACE_LEGACY ){
      حرف *z = sqlite3VdbeExpandSql(p, zTrace);
      db->trace.xLegacy(db->pTraceArg, z);
      sqlite3_free(z);
    }وإلا
#endif
    إذا( db->nVdbeExec>1 ){
      حرف *z = sqlite3MPrintf(db, "-- %s", zTrace);
      (فراغ)db->trace.xV2(SQLITE_TRACE_STMT, db->pTraceArg, p, z);
      sqlite3DbFree(db, z);
    }وإلا{
      (فراغ)db->trace.xV2(SQLITE_TRACE_STMT, db->pTraceArg, p, zTrace);
    }
  }
#ifdef SQLITE_USE_FCNTL_TRACE
  zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql);
  إذا( zTrace ){
    صحيح j;
    لكل(j=0; j<db->nDb; j++){
      إذا( DbMaskTest(p->btreeMask, j)==0 ) استمر;
      sqlite3_file_control(db, db->aDb[j].zDbSName, SQLITE_FCNTL_TRACE, zTrace);
    }
  }
#endif /* SQLITE_USE_FCNTL_TRACE */
#ifdef SQLITE_DEBUG
  إذا( (db->flags & SQLITE_SqlTrace)!=0
   && (zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql))!=0
  ){
    sqlite3DebugPrintf("SQL-trace: %s\n", zTrace);
  }
#endif /* SQLITE_DEBUG */
#endif /* SQLITE_OMIT_TRACE */
  أكد( pOp->p2>0 );
  إذا( pOp->p1>=sqlite3GlobalConfig.iOnceResetThreshold ){
    إذا( pOp->opcode==OP_Trace ) اكسر;
    لكل(i=1; i<p->nOp; i++){
      إذا( p->aOp[i].opcode==OP_Once ) p->aOp[i].p1 = 0;
    }
    pOp->p1 = 0;
  }
  pOp->p1++;
  p->aCounter[SQLITE_STMTSTATUS_RUN]++;
  اذهب jump_to_p2;
}

#ifdef SQLITE_ENABLE_CURSOR_HINTS
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
حالة OP_CursorHint: {
  VdbeCursor *pC;
  pC = p->apCsr[pOp->p1];

  أكد( pOp->p1>=0 && pOp->p1<p->nCursor );

  إذا( pC ){
    أكد( pC->eCurType==CURTYPE_BTREE );
    إذا( pOp->p4type==P4_ئىپادە ){
      sqlite3BtreeCursorHint(pC->uc.pCursor, BTREE_HINT_RANGE,
          pOp->p4.p_ئىپادە, aMem);
    }وإلا إذا( p->apCsr[pOp->p3] ){
      sqlite3BtreeCursorHint(
          pC->uc.pCursor, BTREE_HINT_TABLECURSOR, p->apCsr[pOp->p3]->uc.pCursor
      );
    }
  }
  اكسر;
}
#endif /* SQLITE_ENABLE_CURSOR_HINTS */

#ifdef SQLITE_DEBUG
/* Opcode:  Abortable   * * * * *
**
** Verify that an Abort can happen.  Assert if an Abort at this point
** might cause database corruption.  This opcode only appears in debugging
** builds.
**
** An Abort is safe if either there have been no writes, or if there is
** an active statement journal.
*/
حالة OP_Abortable: {
  sqlite3VdbeAssertAbortable(p);
  اكسر;
}
#endif

#ifdef SQLITE_DEBUG
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
حالة OP_ReleaseReg: {
  Mem *pMem;
  صحيح i;
  u32 constMask;
  أكد( pOp->p1>0 );
  أكد( pOp->p1+pOp->p2<=(p->nMem+1 - p->nCursor)+1 );
  pMem = &aMem[pOp->p1];
  constMask = pOp->p3;
  لكل(i=0; i<pOp->p2; i++, pMem++){
    إذا( i>=32 || (constMask & MASKBIT32(i))==0 ){
      pMem->pScopyFrom = 0;
      إذا( i<32 && pOp->p5 ) MemSetTypeFlag(pMem, MEM_Undefined);
    }
  }
  اكسر;
}
#endif

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
افتراضي: {          /* This is really OP_Noop, OP_Explain */
  أكد( pOp->opcode==OP_Noop || pOp->opcode==OP_Explain );

  اكسر;
}

/*****************************************************************************
** The cases of the switch statement above this line should all be indented
** by 6 spaces.  But the left-most 6 spaces have been removed to improve the
** readability.  From this point on down, the normal indentation rules are
** restored.
*****************************************************************************/
    }

#إذا defined(VDBE_PROFILE)
    *pnCycle += sqlite3NProfileCnt ? sqlite3NProfileCnt : sqlite3Hwtime();
    pnCycle = 0;
#وإلاإذا defined(SQLITE_ENABLE_STMT_SCANSTATUS)
    إذا( pnCycle ){
      *pnCycle += sqlite3Hwtime();
      pnCycle = 0;
    }
#endif

    /* The following code adds nothing to the actual functionality
    ** of the program.  It is only here for testing and debugging.
    ** On the other hand, it does burn CPU cycles every time through
    ** the evaluator loop.  So we can leave it out when NDEBUG is defined.
    */
#ifndef NDEBUG
    أكد( pOp>=&aOp[-1] && pOp<&aOp[p->nOp-1] );

#ifdef SQLITE_DEBUG
    إذا( db->flags & SQLITE_VdbeTrace ){
      u8 opProperty = sqlite3OpcodeProperty[pOrigOp->opcode];
      إذا( rc!=0 ) printf("rc=%d\n",rc);
      إذا( opProperty & (OPFLG_OUT2) ){
        registerTrace(pOrigOp->p2, &aMem[pOrigOp->p2]);
      }
      إذا( opProperty & OPFLG_OUT3 ){
        registerTrace(pOrigOp->p3, &aMem[pOrigOp->p3]);
      }
      إذا( opProperty==0xff ){
        /* Never happens.  This code exists to avoid a harmless linkage
        ** warning about sqlite3VdbeRegisterDump() being defined but not
        ** used. */
        sqlite3VdbeRegisterDump(p);
      }
    }
#endif  /* SQLITE_DEBUG */
#endif  /* NDEBUG */
  }  /* The end of the for(;;) loop the loops through opcodes */

  /* If we reach this point, it means that execution is finished with
  ** an error of some kind.
  */
abort_due_to_error:
  إذا( db->mallocFailed ){
    rc = SQLITE_NOMEM_BKPT;
  }وإلا إذا( rc==SQLITE_IOERR_CORRUPTFS ){
    rc = SQLITE_CORRUPT_BKPT;
  }
  أكد( rc );
#ifdef SQLITE_DEBUG
  إذا( db->flags & SQLITE_VdbeTrace ){
    ثابت حرف *zTrace = p->zSql;
    إذا( zTrace==0 ){
      إذا( aOp[0].opcode==OP_Trace ){
        zTrace = aOp[0].p4.z;
      }
      إذا( zTrace==0 ) zTrace = "???";
    }
    printf("ABORT-due-to-error (rc=%d): %s\n", rc, zTrace);
  }
#endif
  إذا( p->z_Err_ئۇچۇر==0 && rc!=SQLITE_IOERR_NOMEM ){
    sqlite3VdbeError(p, "%s", sqlite3ErrStr(rc));
  }
  p->rc = rc;
  sqlite3SystemError(db, rc);
  testcase( sqlite3GlobalConfig.xLog!=0 );
  sqlite3VdbeLogAbort(p, rc, pOp, aOp);
  إذا( p->eVdbeState==VDBE_RUN_STATE ) sqlite3VdbeHalt(p);
  إذا( rc==SQLITE_IOERR_NOMEM ) sqlite3OomFault(db);
  إذا( rc==SQLITE_CORRUPT && db->autoCommit==0 ){
    db->flags |= SQLITE_CorruptRdOnly;
  }
  rc = SQLITE_ERROR;
  إذا( resetSchemaOnFault>0 ){
    sqlite3ResetOneSchema(db, resetSchemaOnFault-1);
  }

  /* This is the only way out of this procedure.  We have to
  ** release the mutexes on btrees that were acquired at the
  ** top. */
vdbe_return:
#إذا defined(VDBE_PROFILE)
  إذا( pnCycle ){
    *pnCycle += sqlite3NProfileCnt ? sqlite3NProfileCnt : sqlite3Hwtime();
    pnCycle = 0;
  }
#وإلاإذا defined(SQLITE_ENABLE_STMT_SCANSTATUS)
  إذا( pnCycle ){
    *pnCycle += sqlite3Hwtime();
    pnCycle = 0;
  }
#endif

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  طالما( nVmStep>=nProgressLimit && db->xProgress!=0 ){
    nProgressLimit += db->nProgressOps;
    إذا( db->xProgress(db->pProgressArg) ){
      nProgressLimit = LARGEST_UINT64;
      rc = SQLITE_INTERRUPT;
      اذهب abort_due_to_error;
    }
  }
#endif
  p->aCounter[SQLITE_STMTSTATUS_VM_STEP] += (صحيح)nVmStep;
  إذا( DbMaskNonZero(p->lockMask) ){
    sqlite3VdbeLeave(p);
  }
  أكد( rc!=SQLITE_OK || nExtraDelete==0
       || sqlite3_strlike("DELETE%",p->zSql,0)!=0
  );
  أرجع rc;

  /* Jump to here if a string or blob larger than SQLITE_MAX_LENGTH
  ** is encountered.
  */
too_big:
  sqlite3VdbeError(p, "string or blob too big");
  rc = SQLITE_TOOBIG;
  اذهب abort_due_to_error;

  /* Jump to here if a malloc() fails.
  */
no_mem:
  sqlite3OomFault(db);
  sqlite3VdbeError(p, "out of memory");
  rc = SQLITE_NOMEM_BKPT;
  اذهب abort_due_to_error;

  /* Jump to here if the sqlite3_interrupt() API sets the interrupt
  ** flag.
  */
abort_due_to_interrupt:
  أكد( AtomicLoad(&db->u1.isInterrupted) );
  rc = SQLITE_INTERRUPT;
  اذهب abort_due_to_error;
}
