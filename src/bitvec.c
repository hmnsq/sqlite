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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/* Size of the Bitvec structure in bytes. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_SZ        512

/* Round the union size down to the nearest pointer boundary, since that's how
** it will be aligned within the Bitvec struct. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_USIZE \
    (((BITVEC_SZ-(3*ಗಾತ್ರ(u32)))/ಗಾತ್ರ(Bitvec*))*ಗಾತ್ರ(Bitvec*))

/* Type of the array "element" for the bitmap representation.
** Should be a power of 2, and ideally, evenly divide into BITVEC_USIZE.
** Setting this to the "natural word" size of your CPU may improve
** performance. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_TELEM     u8
/* Size, in bits, of the bitmap element. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_SZELEM    8
/* Number of elements in a bitmap array. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_NELEM     (BITVEC_USIZE/ಗಾತ್ರ(BITVEC_TELEM))
/* Number of bits in the bitmap array. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_NBIT      (BITVEC_NELEM*BITVEC_SZELEM)

/* Number of u32 values in hash table. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_NINT      (BITVEC_USIZE/ಗಾತ್ರ(u32))
/* Maximum number of entries in hash table before
** sub-dividing and re-hashing. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_MXHASH    (BITVEC_NINT/2)
/* Hashing function for the aHash representation.
** Empirical testing showed that the *37 multiplier
** (an arbitrary prime)in the hash function provided
** no fewer collisions than the no-op *1. */
#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_HASH(X)   (((X)*1)%BITVEC_NINT)

#ವ್ಯಾಖ್ಯಾನಿಸು BITVEC_NPTR      ((u32)(BITVEC_USIZE/ಗಾತ್ರ(Bitvec *)))


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
ರಚನೆ Bitvec {
  u32 i_ಗಾತ್ರ;      /* Maximum bit index.  Max iSize is 4,294,967,296. */
  u32 n_ಗಣ;       /* Number of bits that are set - only valid for aHash
                  ** element.  Max is BITVEC_NINT.  For BITVEC_SZ of 512,
                  ** this would be 125. */
  u32 i_ಭಾಜಕ;   /* Number of bits handled by each apSub[] entry. */
                  /* Should >=0 for apSub element. */
                  /* Max iDivisor is max(u32) / BITVEC_NPTR + 1.  */
                  /* For a BITVEC_SZ of 512, this would be 34,359,739. */
  ಒಕ್ಕೂಟ {
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
Bitvec *sqlite3BitvecCreate(u32 i_ಗಾತ್ರ){
  Bitvec *p;
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(*p)==BITVEC_SZ );
  p = sqlite3MallocZero( ಗಾತ್ರ(*p) );
  ಆದರೆ( p ){
    p->i_ಗಾತ್ರ = i_ಗಾತ್ರ;
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Check to see if the i-th bit is set.  Return true or false.
** If p is NULL (if the bitmap has not been created) or if
** i is out of range, then return false.
*/
ಪೂರ್ಣಾಂಕ sqlite3BitvecTestNotNull(Bitvec *p, u32 i){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  i--;
  ಆದರೆ( i>=p->i_ಗಾತ್ರ ) ಹಿಂತಿರುಗಿ 0;
  ಆಗ( p->i_ಭಾಜಕ ){
    u32 bin = i/p->i_ಭಾಜಕ;
    i = i%p->i_ಭಾಜಕ;
    p = p->u.apSub[bin];
    ಆದರೆ (!p) {
      ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಆದರೆ( p->i_ಗಾತ್ರ<=BITVEC_NBIT ){
    ಹಿಂತಿರುಗಿ (p->u.aBitmap[i/BITVEC_SZELEM] & (1<<(i&(BITVEC_SZELEM-1))))!=0;
  } ಇಲ್ಲದಿದ್ದರೆ{
    u32 h = BITVEC_HASH(i++);
    ಆಗ( p->u.aHash[h] ){
      ಆದರೆ( p->u.aHash[h]==i ) ಹಿಂತಿರುಗಿ 1;
      h = (h+1) % BITVEC_NINT;
    }
    ಹಿಂತಿರುಗಿ 0;
  }
}
ಪೂರ್ಣಾಂಕ sqlite3BitvecTest(Bitvec *p, u32 i){
  ಹಿಂತಿರುಗಿ p!=0 && sqlite3BitvecTestNotNull(p,i);
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
ಪೂರ್ಣಾಂಕ sqlite3_Bitvec_ಗಣ(Bitvec *p, u32 i){
  u32 h;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( i>0 );
  ಪ್ರತಿಜ್ಞಾ( i<=p->i_ಗಾತ್ರ );
  i--;
  ಆಗ((p->i_ಗಾತ್ರ > BITVEC_NBIT) && p->i_ಭಾಜಕ) {
    u32 bin = i/p->i_ಭಾಜಕ;
    i = i%p->i_ಭಾಜಕ;
    ಆದರೆ( p->u.apSub[bin]==0 ){
      p->u.apSub[bin] = sqlite3BitvecCreate( p->i_ಭಾಜಕ );
      ಆದರೆ( p->u.apSub[bin]==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    p = p->u.apSub[bin];
  }
  ಆದರೆ( p->i_ಗಾತ್ರ<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] |= 1 << (i&(BITVEC_SZELEM-1));
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  h = BITVEC_HASH(i++);
  /* if there wasn't a hash collision, and this doesn't */
  /* completely fill the hash, then just add it without */
  /* worrying about sub-dividing and re-hashing. */
  ಆದರೆ( !p->u.aHash[h] ){
    ಆದರೆ (p->n_ಗಣ<(BITVEC_NINT-1)) {
      ಹೋಗು bitvec_ಗಣ_ಕೊನೆ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಹೋಗು bitvec_ಗಣ_rehash;
    }
  }
  /* there was a collision, check to see if it's already */
  /* in hash, if not, try to find a spot for it */
  ಮಾಡು {
    ಆದರೆ( p->u.aHash[h]==i ) ಹಿಂತಿರುಗಿ SQLITE_OK;
    h++;
    ಆದರೆ( h>=BITVEC_NINT ) h = 0;
  } ಆಗ( p->u.aHash[h] );
  /* we didn't find it in the hash.  h points to the first */
  /* available free spot. check to see if this is going to */
  /* make our hash too "full".  */
bitvec_ಗಣ_rehash:
  ಆದರೆ( p->n_ಗಣ>=BITVEC_MXHASH ){
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;
    ಪೂರ್ಣಾಂಕ rc;
    u32 *ai_ಮೌಲ್ಯಗಳು = sqlite3_ರಾಶಿ_Alloc_Raw(0, ಗಾತ್ರ(p->u.aHash));
    ಆದರೆ( ai_ಮೌಲ್ಯಗಳು==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಮರಣೆ_ನಕಲು(ai_ಮೌಲ್ಯಗಳು, p->u.aHash, ಗಾತ್ರ(p->u.aHash));
      memset(p->u.apSub, 0, ಗಾತ್ರ(p->u.apSub));
      p->i_ಭಾಜಕ = p->i_ಗಾತ್ರ/BITVEC_NPTR;
      ಆದರೆ( (p->i_ಗಾತ್ರ%BITVEC_NPTR)!=0 ) p->i_ಭಾಜಕ++;
      ಆದರೆ( p->i_ಭಾಜಕ<BITVEC_NBIT ) p->i_ಭಾಜಕ = BITVEC_NBIT;
      rc = sqlite3_Bitvec_ಗಣ(p, i);
      ಗಾಗಿ(j=0; j<BITVEC_NINT; j++){
        ಆದರೆ( ai_ಮೌಲ್ಯಗಳು[j] ) rc |= sqlite3_Bitvec_ಗಣ(p, ai_ಮೌಲ್ಯಗಳು[j]);
      }
      sqlite3_ರಾಶಿ_Free(0, ai_ಮೌಲ್ಯಗಳು);
      ಹಿಂತಿರುಗಿ rc;
    }
  }
bitvec_ಗಣ_ಕೊನೆ:
  p->n_ಗಣ++;
  p->u.aHash[h] = i;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Clear the i-th bit.
**
** pBuf must be a pointer to at least BITVEC_SZ bytes of temporary storage
** that BitvecClear can use to rebuilt its hash table.
*/
ಶೂನ್ಯ sqlite3BitvecClear(Bitvec *p, u32 i, ಶೂನ್ಯ *pBuf){
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( i>0 );
  i--;
  ಆಗ( p->i_ಭಾಜಕ ){
    u32 bin = i/p->i_ಭಾಜಕ;
    i = i%p->i_ಭಾಜಕ;
    p = p->u.apSub[bin];
    ಆದರೆ (!p) {
      ಹಿಂತಿರುಗಿ;
    }
  }
  ಆದರೆ( p->i_ಗಾತ್ರ<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] &= ~(BITVEC_TELEM)(1<<(i&(BITVEC_SZELEM-1)));
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;
    u32 *ai_ಮೌಲ್ಯಗಳು = pBuf;
    ಸ್ಮರಣೆ_ನಕಲು(ai_ಮೌಲ್ಯಗಳು, p->u.aHash, ಗಾತ್ರ(p->u.aHash));
    memset(p->u.aHash, 0, ಗಾತ್ರ(p->u.aHash));
    p->n_ಗಣ = 0;
    ಗಾಗಿ(j=0; j<BITVEC_NINT; j++){
      ಆದರೆ( ai_ಮೌಲ್ಯಗಳು[j] && ai_ಮೌಲ್ಯಗಳು[j]!=(i+1) ){
        u32 h = BITVEC_HASH(ai_ಮೌಲ್ಯಗಳು[j]-1);
        p->n_ಗಣ++;
        ಆಗ( p->u.aHash[h] ){
          h++;
          ಆದರೆ( h>=BITVEC_NINT ) h = 0;
        }
        p->u.aHash[h] = ai_ಮೌಲ್ಯಗಳು[j];
      }
    }
  }
}

/*
** Destroy a bitmap object.  Reclaim all memory used.
*/
ಶೂನ್ಯ sqlite3BitvecDestroy(Bitvec *p){
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( p->i_ಭಾಜಕ ){
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<BITVEC_NPTR; i++){
      sqlite3BitvecDestroy(p->u.apSub[i]);
    }
  }
  sqlite3_free(p);
}

/*
** Return the value of the iSize parameter specified when Bitvec *p
** was created.
*/
u32 sqlite3_Bitvec_ಗಾತ್ರ(Bitvec *p){
  ಹಿಂತಿರುಗಿ p->i_ಗಾತ್ರ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
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
ಸ್ಥಿರ ಶೂನ್ಯ showBitvec(Bitvec *p, ಪೂರ್ಣಾಂಕ n, ಚಿಹ್ನೆರಹಿತ x){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( p==0 ){
    ಬರೆ("NULL\n");
    ಹಿಂತಿರುಗಿ;
  }
  ಬರೆ("Bitvec 0x%p iSize=%u", p, p->i_ಗಾತ್ರ);
  ಆದರೆ( p->i_ಗಾತ್ರ<=BITVEC_NBIT ){
    ಬರೆ(" bitmap\n");
    ಬರೆ("%*s   bits:", n, "");
    ಗಾಗಿ(i=1; i<=BITVEC_NBIT; i++){
      ಆದರೆ( sqlite3BitvecTest(p,i) ) ಬರೆ(" %u", x+(ಚಿಹ್ನೆರಹಿತ)i);
    }
    ಬರೆ("\n");
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->i_ಭಾಜಕ==0 ){
    ಬರೆ(" hash with %u entries\n", p->n_ಗಣ);
    ಬರೆ("%*s   bits:", n, "");
    ಗಾಗಿ(i=0; i<BITVEC_NINT; i++){
      ಆದರೆ( p->u.aHash[i] ) ಬರೆ(" %u", x+(ಚಿಹ್ನೆರಹಿತ)p->u.aHash[i]);
    }
    ಬರೆ("\n");
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಬರೆ(" sub-bitvec with iDivisor=%u\n", p->i_ಭಾಜಕ);
    ಗಾಗಿ(i=0; i<BITVEC_NPTR; i++){
      ಆದರೆ( p->u.apSub[i]==0 ) ಮುಂದುವರಿ;
      ಬರೆ("%*s   apSub[%d]=", n, "", i);
      showBitvec(p->u.apSub[i], n+4, i*p->i_ಭಾಜಕ);
    }
  }
}
ಶೂನ್ಯ sqlite3ShowBitvec(Bitvec *p){
  showBitvec(p, 0, 0);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
/*
** Let V[] be an array of unsigned characters sufficient to hold
** up to N bits.  Let I be an integer between 0 and N.  0<=I<N.
** Then the following macros can be used to set, clear, or test
** individual bits within V.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SETBIT(V,I)      V[I>>3] |= (1<<(I&7))
#ವ್ಯಾಖ್ಯಾನಿಸು CLEARBIT(V,I)    V[I>>3] &= ~(BITVEC_TELEM)(1<<(I&7))
#ವ್ಯಾಖ್ಯಾನಿಸು TESTBIT(V,I)     (V[I>>3]&(1<<(I&7)))!=0


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
ಪೂರ್ಣಾಂಕ sqlite3BitvecBuiltinTest(ಪೂರ್ಣಾಂಕ sz, ಪೂರ್ಣಾಂಕ *aOp){
  Bitvec *pBitvec = 0;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pV = 0;
  ಪೂರ್ಣಾಂಕ rc = -1;
  ಪೂರ್ಣಾಂಕ i, nx, pc, op;
  ಶೂನ್ಯ *pTmpSpace;

  /* Allocate the Bitvec to be tested and a linear array of
  ** bits to act as the reference */
  ಆದರೆ( sz<=0 ){
    pBitvec = sqlite3BitvecCreate( 2*(ಚಿಹ್ನೆರಹಿತ)(-sz) );
    pV = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pBitvec = sqlite3BitvecCreate( sz );
    pV = sqlite3MallocZero( (7+(i64)sz)/8 + 1 );
  }
  pTmpSpace = sqlite3_malloc64(BITVEC_SZ);
  ಆದರೆ( pBitvec==0 || pTmpSpace==0 || (pV==0 && sz>0) ) ಹೋಗು bitvec_ಕೊನೆ;

  /* NULL pBitvec tests */
  sqlite3_Bitvec_ಗಣ(0, 1);
  sqlite3BitvecClear(0, 1, pTmpSpace);

  /* Run the program */
  pc = i = 0;
  ಆಗ( (op = aOp[pc])!=0 ){
    ಆದರೆ( op>=6 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      ಆದರೆ( op==6 ){
        sqlite3ShowBitvec(pBitvec);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==7 ){
        ಬರೆ("BITVEC_SZ     = %d (%d by sizeof)\n",
               BITVEC_SZ, (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(Bitvec));
        ಬರೆ("BITVEC_USIZE  = %d\n", (ಪೂರ್ಣಾಂಕ)BITVEC_USIZE);
        ಬರೆ("BITVEC_NELEM  = %d\n", (ಪೂರ್ಣಾಂಕ)BITVEC_NELEM);
        ಬರೆ("BITVEC_NBIT   = %d\n", (ಪೂರ್ಣಾಂಕ)BITVEC_NBIT);
        ಬರೆ("BITVEC_NINT   = %d\n", (ಪೂರ್ಣಾಂಕ)BITVEC_NINT);
        ಬರೆ("BITVEC_MXHASH = %d\n", (ಪೂರ್ಣಾಂಕ)BITVEC_MXHASH);
        ಬರೆ("BITVEC_NPTR   = %d\n", (ಪೂರ್ಣಾಂಕ)BITVEC_NPTR);
      }
#ಆದರೆ_ಅಂತ್ಯ
      pc++;
      ಮುಂದುವರಿ;
    }
    ಆಯ್ಕೆ( op ){
      ಸ್ಥಿತಿ 1:
      ಸ್ಥಿತಿ 2:
      ಸ್ಥಿತಿ 5: {
        nx = 4;
        i = aOp[pc+2] - 1;
        aOp[pc+2] += aOp[pc+3];
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 3:
      ಸ್ಥಿತಿ 4:
      ಪೂರ್ವನಿಯೋಜಿತ: {
        nx = 2;
        sqlite3_randomness(ಗಾತ್ರ(i), &i);
        ನಿಲ್ಲಿಸು;
      }
    }
    ಆದರೆ( (--aOp[pc+1]) > 0 ) nx = 0;
    pc += nx;
    i = (i & 0x7fffffff)%sz;
    ಆದರೆ( (op & 1)!=0 ){
      ಆದರೆ( pV ) SETBIT(pV, (i+1));
      ಆದರೆ( op!=5 ){
        ಆದರೆ( sqlite3_Bitvec_ಗಣ(pBitvec, i+1) ) ಹೋಗು bitvec_ಕೊನೆ;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( pV ) CLEARBIT(pV, (i+1));
      sqlite3BitvecClear(pBitvec, i+1, pTmpSpace);
    }
  }

  /* Test to make sure the linear array exactly matches the
  ** Bitvec object.  Start with the assumption that they do
  ** match (rc==0).  Change rc to non-zero if a discrepancy
  ** is found.
  */
  ಆದರೆ( pV ){
    rc = sqlite3BitvecTest(0,0) + sqlite3BitvecTest(pBitvec, sz+1)
            + sqlite3BitvecTest(pBitvec, 0)
            + (sqlite3_Bitvec_ಗಾತ್ರ(pBitvec) - sz);
    ಗಾಗಿ(i=1; i<=sz; i++){
      ಆದರೆ( (TESTBIT(pV,i))!=sqlite3BitvecTest(pBitvec,i) ){
        rc = i;
        ನಿಲ್ಲಿಸು;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = 0;
  }

  /* Free allocated structure */
bitvec_ಕೊನೆ:
  sqlite3_free(pTmpSpace);
  sqlite3_free(pV);
  sqlite3BitvecDestroy(pBitvec);
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_UNTESTABLE */
