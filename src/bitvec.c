/*
** 2008 February 16
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements an object that represents a fixed-length
** bitmap.  Bits are numbered starting with 1.
**
** A bitmap is used to record which pages of a database file have been
** journalled during a transaction, or which pages have the "dont-write"
** property.  Usually only a few pages are meet either condition.
** So the bitmap is usually sparse and has low cardinality.
** But sometimes (for example when during a DROP of a large table) most
** or all of the pages in a database can get journalled.  In those cases,
** the bitmap becomes dense with high cardinality.  The algorithm needs
** to handle both cases well.
**
** The size of the bitmap is fixed when the object is created.
**
** All bits are clear when the bitmap is created.  Individual bits
** may be set or cleared one at a time.
**
** Test operations are about 100 times more common that set operations.
** Clear operations are exceedingly rare.  There are usually between
** 5 and 500 set operations per Bitvec object, though the number of sets can
** sometimes grow into tens of thousands or larger.  The size of the
** Bitvec object is the number of pages in the database file at the
** start of a transaction, and is thus usually less than a few thousand,
** but can be as large as 2 billion for a really big database.
*/
#ضمن "sqliteInt.h"

/* Size of the Bitvec structure in bytes. */
#تعریف_کړه BITVEC_SZ        512

/* Round the union size down to the nearest pointer boundary, since that's how
** it will be aligned within the Bitvec struct. */
#تعریف_کړه BITVEC_USIZE \
    (((BITVEC_SZ-(3*حجم(u32)))/حجم(Bitvec*))*حجم(Bitvec*))

/* Type of the array "element" for the bitmap representation.
** Should be a power of 2, and ideally, evenly divide into BITVEC_USIZE.
** Setting this to the "natural word" size of your CPU may improve
** performance. */
#تعریف_کړه BITVEC_TELEM     u8
/* Size, in bits, of the bitmap element. */
#تعریف_کړه BITVEC_SZELEM    8
/* Number of elements in a bitmap array. */
#تعریف_کړه BITVEC_NELEM     (BITVEC_USIZE/حجم(BITVEC_TELEM))
/* Number of bits in the bitmap array. */
#تعریف_کړه BITVEC_NBIT      (BITVEC_NELEM*BITVEC_SZELEM)

/* Number of u32 values in hash table. */
#تعریف_کړه BITVEC_NINT      (BITVEC_USIZE/حجم(u32))
/* Maximum number of entries in hash table before
** sub-dividing and re-hashing. */
#تعریف_کړه BITVEC_MXHASH    (BITVEC_NINT/2)
/* Hashing function for the aHash representation.
** Empirical testing showed that the *37 multiplier
** (an arbitrary prime)in the hash function provided
** no fewer collisions than the no-op *1. */
#تعریف_کړه BITVEC_HASH(X)   (((X)*1)%BITVEC_NINT)

#تعریف_کړه BITVEC_NPTR      ((u32)(BITVEC_USIZE/حجم(Bitvec *)))


/*
** A bitmap is an instance of the following structure.
**
** This bitmap records the existence of zero or more bits
** with values between 1 and iSize, inclusive.
**
** There are three possible representations of the bitmap.
** If iSize<=BITVEC_NBIT, then Bitvec.u.aBitmap[] is a straight
** bitmap.  The least significant bit is bit 1.
**
** If iSize>BITVEC_NBIT and iDivisor==0 then Bitvec.u.aHash[] is
** a hash table that will hold up to BITVEC_MXHASH distinct values.
**
** Otherwise, the value i is redirected into one of BITVEC_NPTR
** sub-bitmaps pointed to by Bitvec.u.apSub[].  Each subbitmap
** handles up to iDivisor separate values of i.  apSub[0] holds
** values between 1 and iDivisor.  apSub[1] holds values between
** iDivisor+1 and 2*iDivisor.  apSub[N] holds values between
** N*iDivisor+1 and (N+1)*iDivisor.  Each subbitmap is normalized
** to hold deal with values between 1 and iDivisor.
*/
بنية Bitvec {
  u32 i_اندازه;      /* Maximum bit index.  Max iSize is 4,294,967,296. */
  u32 n_سېټ;       /* Number of bits that are set - only valid for aHash
                  ** element.  Max is BITVEC_NINT.  For BITVEC_SZ of 512,
                  ** this would be 125. */
  u32 i_وېشونکی;   /* Number of bits handled by each apSub[] entry. */
                  /* Should >=0 for apSub element. */
                  /* Max iDivisor is max(u32) / BITVEC_NPTR + 1.  */
                  /* For a BITVEC_SZ of 512, this would be 34,359,739. */
  اتحاد {
    BITVEC_TELEM aBitmap[BITVEC_NELEM];    /* Bitmap representation */
    u32 aHash[BITVEC_NINT];      /* Hash table representation */
    Bitvec *apSub[BITVEC_NPTR];  /* Recursive representation */
  } u;
};


/*
** Create a new bitmap object able to handle bits between 0 and iSize,
** inclusive.  Return a pointer to the new object.  Return NULL if
** malloc fails.
*/
Bitvec *sqlite3BitvecCreate(u32 i_اندازه){
  Bitvec *p;
  أكد( حجم(*p)==BITVEC_SZ );
  p = sqlite3MallocZero( حجم(*p) );
  إذا( p ){
    p->i_اندازه = i_اندازه;
  }
  أرجع p;
}

/*
** Check to see if the i-th bit is set.  Return true or false.
** If p is NULL (if the bitmap has not been created) or if
** i is out of range, then return false.
*/
صحيح sqlite3BitvecTestNotNull(Bitvec *p, u32 i){
  أكد( p!=0 );
  i--;
  إذا( i>=p->i_اندازه ) أرجع 0;
  طالما( p->i_وېشونکی ){
    u32 bin = i/p->i_وېشونکی;
    i = i%p->i_وېشونکی;
    p = p->u.apSub[bin];
    إذا (!p) {
      أرجع 0;
    }
  }
  إذا( p->i_اندازه<=BITVEC_NBIT ){
    أرجع (p->u.aBitmap[i/BITVEC_SZELEM] & (1<<(i&(BITVEC_SZELEM-1))))!=0;
  } وإلا{
    u32 h = BITVEC_HASH(i++);
    طالما( p->u.aHash[h] ){
      إذا( p->u.aHash[h]==i ) أرجع 1;
      h = (h+1) % BITVEC_NINT;
    }
    أرجع 0;
  }
}
صحيح sqlite3BitvecTest(Bitvec *p, u32 i){
  أرجع p!=0 && sqlite3BitvecTestNotNull(p,i);
}

/*
** Set the i-th bit.  Return 0 on success and an error code if
** anything goes wrong.
**
** This routine might cause sub-bitmaps to be allocated.  Failing
** to get the memory needed to hold the sub-bitmap is the only
** that can go wrong with an insert, assuming p and i are valid.
**
** The calling function must ensure that p is a valid Bitvec object
** and that the value for "i" is within range of the Bitvec object.
** Otherwise the behavior is undefined.
*/
صحيح sqlite3_Bitvec_سېټ(Bitvec *p, u32 i){
  u32 h;
  إذا( p==0 ) أرجع SQLITE_OK;
  أكد( i>0 );
  أكد( i<=p->i_اندازه );
  i--;
  طالما((p->i_اندازه > BITVEC_NBIT) && p->i_وېشونکی) {
    u32 bin = i/p->i_وېشونکی;
    i = i%p->i_وېشونکی;
    إذا( p->u.apSub[bin]==0 ){
      p->u.apSub[bin] = sqlite3BitvecCreate( p->i_وېشونکی );
      إذا( p->u.apSub[bin]==0 ) أرجع SQLITE_NOMEM_BKPT;
    }
    p = p->u.apSub[bin];
  }
  إذا( p->i_اندازه<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] |= 1 << (i&(BITVEC_SZELEM-1));
    أرجع SQLITE_OK;
  }
  h = BITVEC_HASH(i++);
  /* if there wasn't a hash collision, and this doesn't */
  /* completely fill the hash, then just add it without */
  /* worrying about sub-dividing and re-hashing. */
  إذا( !p->u.aHash[h] ){
    إذا (p->n_سېټ<(BITVEC_NINT-1)) {
      اذهب bitvec_سېټ_پای_ټکی;
    } وإلا {
      اذهب bitvec_سېټ_rehash;
    }
  }
  /* there was a collision, check to see if it's already */
  /* in hash, if not, try to find a spot for it */
  افعل {
    إذا( p->u.aHash[h]==i ) أرجع SQLITE_OK;
    h++;
    إذا( h>=BITVEC_NINT ) h = 0;
  } طالما( p->u.aHash[h] );
  /* we didn't find it in the hash.  h points to the first */
  /* available free spot. check to see if this is going to */
  /* make our hash too "full".  */
bitvec_سېټ_rehash:
  إذا( p->n_سېټ>=BITVEC_MXHASH ){
    بینښه صحيح j;
    صحيح rc;
    u32 *ai_ارزښتونه = sqlite3_ډېری_Alloc_Raw(0, حجم(p->u.aHash));
    إذا( ai_ارزښتونه==0 ){
      أرجع SQLITE_NOMEM_BKPT;
    }وإلا{
      حافظه_نقل(ai_ارزښتونه, p->u.aHash, حجم(p->u.aHash));
      memset(p->u.apSub, 0, حجم(p->u.apSub));
      p->i_وېشونکی = p->i_اندازه/BITVEC_NPTR;
      إذا( (p->i_اندازه%BITVEC_NPTR)!=0 ) p->i_وېشونکی++;
      إذا( p->i_وېشونکی<BITVEC_NBIT ) p->i_وېشونکی = BITVEC_NBIT;
      rc = sqlite3_Bitvec_سېټ(p, i);
      لكل(j=0; j<BITVEC_NINT; j++){
        إذا( ai_ارزښتونه[j] ) rc |= sqlite3_Bitvec_سېټ(p, ai_ارزښتونه[j]);
      }
      sqlite3_ډېری_Free(0, ai_ارزښتونه);
      أرجع rc;
    }
  }
bitvec_سېټ_پای_ټکی:
  p->n_سېټ++;
  p->u.aHash[h] = i;
  أرجع SQLITE_OK;
}

/*
** Clear the i-th bit.
**
** pBuf must be a pointer to at least BITVEC_SZ bytes of temporary storage
** that BitvecClear can use to rebuilt its hash table.
*/
فراغ sqlite3BitvecClear(Bitvec *p, u32 i, فراغ *pBuf){
  إذا( p==0 ) أرجع;
  أكد( i>0 );
  i--;
  طالما( p->i_وېشونکی ){
    u32 bin = i/p->i_وېشونکی;
    i = i%p->i_وېشونکی;
    p = p->u.apSub[bin];
    إذا (!p) {
      أرجع;
    }
  }
  إذا( p->i_اندازه<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] &= ~(BITVEC_TELEM)(1<<(i&(BITVEC_SZELEM-1)));
  }وإلا{
    بینښه صحيح j;
    u32 *ai_ارزښتونه = pBuf;
    حافظه_نقل(ai_ارزښتونه, p->u.aHash, حجم(p->u.aHash));
    memset(p->u.aHash, 0, حجم(p->u.aHash));
    p->n_سېټ = 0;
    لكل(j=0; j<BITVEC_NINT; j++){
      إذا( ai_ارزښتونه[j] && ai_ارزښتونه[j]!=(i+1) ){
        u32 h = BITVEC_HASH(ai_ارزښتونه[j]-1);
        p->n_سېټ++;
        طالما( p->u.aHash[h] ){
          h++;
          إذا( h>=BITVEC_NINT ) h = 0;
        }
        p->u.aHash[h] = ai_ارزښتونه[j];
      }
    }
  }
}

/*
** Destroy a bitmap object.  Reclaim all memory used.
*/
فراغ sqlite3BitvecDestroy(Bitvec *p){
  إذا( p==0 ) أرجع;
  إذا( p->i_وېشونکی ){
    بینښه صحيح i;
    لكل(i=0; i<BITVEC_NPTR; i++){
      sqlite3BitvecDestroy(p->u.apSub[i]);
    }
  }
  sqlite3_free(p);
}

/*
** Return the value of the iSize parameter specified when Bitvec *p
** was created.
*/
u32 sqlite3_Bitvec_اندازه(Bitvec *p){
  أرجع p->i_اندازه;
}

#که_تعریف_شوی SQLITE_DEBUG
/*
** Show the content of a Bitvec option and its children.  Indent
** everything by n spaces.  Add x to each bitvec value.
**
** From a debugger such as gdb, one can type:
**
**    call sqlite3ShowBitvec(p)
**
** For some Bitvec p and see a recursive view of the Bitvec's content.
*/
ساكن فراغ showBitvec(Bitvec *p, صحيح n, بینښه x){
  صحيح i;
  إذا( p==0 ){
    چاپ_کړه("NULL\n");
    أرجع;
  }
  چاپ_کړه("Bitvec 0x%p iSize=%u", p, p->i_اندازه);
  إذا( p->i_اندازه<=BITVEC_NBIT ){
    چاپ_کړه(" bitmap\n");
    چاپ_کړه("%*s   bits:", n, "");
    لكل(i=1; i<=BITVEC_NBIT; i++){
      إذا( sqlite3BitvecTest(p,i) ) چاپ_کړه(" %u", x+(بینښه)i);
    }
    چاپ_کړه("\n");
  }وإلا إذا( p->i_وېشونکی==0 ){
    چاپ_کړه(" hash with %u entries\n", p->n_سېټ);
    چاپ_کړه("%*s   bits:", n, "");
    لكل(i=0; i<BITVEC_NINT; i++){
      إذا( p->u.aHash[i] ) چاپ_کړه(" %u", x+(بینښه)p->u.aHash[i]);
    }
    چاپ_کړه("\n");
  }وإلا{
    چاپ_کړه(" sub-bitvec with iDivisor=%u\n", p->i_وېشونکی);
    لكل(i=0; i<BITVEC_NPTR; i++){
      إذا( p->u.apSub[i]==0 ) استمر;
      چاپ_کړه("%*s   apSub[%d]=", n, "", i);
      showBitvec(p->u.apSub[i], n+4, i*p->i_وېشونکی);
    }
  }
}
فراغ sqlite3ShowBitvec(Bitvec *p){
  showBitvec(p, 0, 0);
}
#د_که_پای

#که_نه_تعریف_شوی SQLITE_UNTESTABLE
/*
** Let V[] be an array of unsigned characters sufficient to hold
** up to N bits.  Let I be an integer between 0 and N.  0<=I<N.
** Then the following macros can be used to set, clear, or test
** individual bits within V.
*/
#تعریف_کړه SETBIT(V,I)      V[I>>3] |= (1<<(I&7))
#تعریف_کړه CLEARBIT(V,I)    V[I>>3] &= ~(BITVEC_TELEM)(1<<(I&7))
#تعریف_کړه TESTBIT(V,I)     (V[I>>3]&(1<<(I&7)))!=0


/*
** This routine runs an extensive test of the Bitvec code.
**
** The input is an array of integers that acts as a program
** to test the Bitvec.  The integers are opcodes followed
** by 0, 1, or 3 operands, depending on the opcode.  Another
** opcode follows immediately after the last operand.
**
** There are opcodes numbered starting with 0.  0 is the
** "halt" opcode and causes the test to end.
**
**    0          Halt and return the number of errors
**    1 N S X    Set N bits beginning with S and incrementing by X
**    2 N S X    Clear N bits beginning with S and incrementing by X
**    3 N        Set N randomly chosen bits
**    4 N        Clear N randomly chosen bits
**    5 N S X    Set N bits from S increment X in array only, not in bitvec
**    6          Invoice sqlite3ShowBitvec() on the Bitvec object so far
**    7 X        Show compile-time parameters and the hash of X         
**
** The opcodes 1 through 4 perform set and clear operations are performed
** on both a Bitvec object and on a linear array of bits obtained from malloc.
** Opcode 5 works on the linear array only, not on the Bitvec.
** Opcode 5 is used to deliberately induce a fault in order to
** confirm that error detection works.  Opcodes 6 and greater are
** state output opcodes.  Opcodes 6 and greater are no-ops unless
** SQLite has been compiled with SQLITE_DEBUG.
**
** At the conclusion of the test the linear array is compared
** against the Bitvec object.  If there are any differences,
** an error is returned.  If they are the same, zero is returned.
**
** If a memory allocation error occurs, return -1.
**
** sz is the size of the Bitvec.  Or if sz is negative, make the size
** 2*(unsigned)(-sz) and disabled the linear vector check.
*/
صحيح sqlite3BitvecBuiltinTest(صحيح sz, صحيح *aOp){
  Bitvec *pBitvec = 0;
  بینښه حرف *pV = 0;
  صحيح rc = -1;
  صحيح i, nx, pc, op;
  فراغ *pTmpSpace;

  /* Allocate the Bitvec to be tested and a linear array of
  ** bits to act as the reference */
  إذا( sz<=0 ){
    pBitvec = sqlite3BitvecCreate( 2*(بینښه)(-sz) );
    pV = 0;
  }وإلا{
    pBitvec = sqlite3BitvecCreate( sz );
    pV = sqlite3MallocZero( (7+(i64)sz)/8 + 1 );
  }
  pTmpSpace = sqlite3_malloc64(BITVEC_SZ);
  إذا( pBitvec==0 || pTmpSpace==0 || (pV==0 && sz>0) ) اذهب bitvec_پای_ټکی;

  /* NULL pBitvec tests */
  sqlite3_Bitvec_سېټ(0, 1);
  sqlite3BitvecClear(0, 1, pTmpSpace);

  /* Run the program */
  pc = i = 0;
  طالما( (op = aOp[pc])!=0 ){
    إذا( op>=6 ){
#که_تعریف_شوی SQLITE_DEBUG
      إذا( op==6 ){
        sqlite3ShowBitvec(pBitvec);
      }وإلا إذا( op==7 ){
        چاپ_کړه("BITVEC_SZ     = %d (%d by sizeof)\n",
               BITVEC_SZ, (صحيح)حجم(Bitvec));
        چاپ_کړه("BITVEC_USIZE  = %d\n", (صحيح)BITVEC_USIZE);
        چاپ_کړه("BITVEC_NELEM  = %d\n", (صحيح)BITVEC_NELEM);
        چاپ_کړه("BITVEC_NBIT   = %d\n", (صحيح)BITVEC_NBIT);
        چاپ_کړه("BITVEC_NINT   = %d\n", (صحيح)BITVEC_NINT);
        چاپ_کړه("BITVEC_MXHASH = %d\n", (صحيح)BITVEC_MXHASH);
        چاپ_کړه("BITVEC_NPTR   = %d\n", (صحيح)BITVEC_NPTR);
      }
#د_که_پای
      pc++;
      استمر;
    }
    بدل( op ){
      حالة 1:
      حالة 2:
      حالة 5: {
        nx = 4;
        i = aOp[pc+2] - 1;
        aOp[pc+2] += aOp[pc+3];
        اكسر;
      }
      حالة 3:
      حالة 4:
      افتراضي: {
        nx = 2;
        sqlite3_randomness(حجم(i), &i);
        اكسر;
      }
    }
    إذا( (--aOp[pc+1]) > 0 ) nx = 0;
    pc += nx;
    i = (i & 0x7fffffff)%sz;
    إذا( (op & 1)!=0 ){
      إذا( pV ) SETBIT(pV, (i+1));
      إذا( op!=5 ){
        إذا( sqlite3_Bitvec_سېټ(pBitvec, i+1) ) اذهب bitvec_پای_ټکی;
      }
    }وإلا{
      إذا( pV ) CLEARBIT(pV, (i+1));
      sqlite3BitvecClear(pBitvec, i+1, pTmpSpace);
    }
  }

  /* Test to make sure the linear array exactly matches the
  ** Bitvec object.  Start with the assumption that they do
  ** match (rc==0).  Change rc to non-zero if a discrepancy
  ** is found.
  */
  إذا( pV ){
    rc = sqlite3BitvecTest(0,0) + sqlite3BitvecTest(pBitvec, sz+1)
            + sqlite3BitvecTest(pBitvec, 0)
            + (sqlite3_Bitvec_اندازه(pBitvec) - sz);
    لكل(i=1; i<=sz; i++){
      إذا( (TESTBIT(pV,i))!=sqlite3BitvecTest(pBitvec,i) ){
        rc = i;
        اكسر;
      }
    }
  }وإلا{
    rc = 0;
  }

  /* Free allocated structure */
bitvec_پای_ټکی:
  sqlite3_free(pTmpSpace);
  sqlite3_free(pV);
  sqlite3BitvecDestroy(pBitvec);
  أرجع rc;
}
#د_که_پای /* SQLITE_UNTESTABLE */
