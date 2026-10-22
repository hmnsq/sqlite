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
#define BITVEC_SZ        512

/* Round the union size down to the nearest pointer boundary, since that's how
** it will be aligned within the Bitvec struct. */
#define BITVEC_USIZE \
    (((BITVEC_SZ-(3*ಪರಿಮಾಣ(u32)))/ಪರಿಮಾಣ(Bitvec*))*ಪರಿಮಾಣ(Bitvec*))

/* Type of the array "element" for the bitmap representation.
** Should be a power of 2, and ideally, evenly divide into BITVEC_USIZE.
** Setting this to the "natural word" size of your CPU may improve
** performance. */
#define BITVEC_TELEM     u8
/* Size, in bits, of the bitmap element. */
#define BITVEC_SZELEM    8
/* Number of elements in a bitmap array. */
#define BITVEC_NELEM     (BITVEC_USIZE/ಪರಿಮಾಣ(BITVEC_TELEM))
/* Number of bits in the bitmap array. */
#define BITVEC_NBIT      (BITVEC_NELEM*BITVEC_SZELEM)

/* Number of u32 values in hash table. */
#define BITVEC_NINT      (BITVEC_USIZE/ಪರಿಮಾಣ(u32))
/* Maximum number of entries in hash table before
** sub-dividing and re-hashing. */
#define BITVEC_MXHASH    (BITVEC_NINT/2)
/* Hashing function for the aHash representation.
** Empirical testing showed that the *37 multiplier
** (an arbitrary prime)in the hash function provided
** no fewer collisions than the no-op *1. */
#define BITVEC_HASH(X)   (((X)*1)%BITVEC_NINT)

#define BITVEC_NPTR      ((u32)(BITVEC_USIZE/ಪರಿಮಾಣ(Bitvec *)))


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
ಸಂರಚನಾ Bitvec {
  u32 iSize;      /* Maximum bit index.  Max iSize is 4,294,967,296. */
  u32 nSet;       /* Number of bits that are set - only valid for aHash
                  ** element.  Max is BITVEC_NINT.  For BITVEC_SZ of 512,
                  ** this would be 125. */
  u32 i_ಭಾಜಕ;   /* Number of bits handled by each apSub[] entry. */
                  /* Should >=0 for apSub element. */
                  /* Max iDivisor is max(u32) / BITVEC_NPTR + 1.  */
                  /* For a BITVEC_SZ of 512, this would be 34,359,739. */
  ಸಙ್ಘ {
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
Bitvec *sqlite3BitvecCreate(u32 iSize){
  Bitvec *p;
  ಪ್ರತಿಜ್ಞಾ( ಪರಿಮಾಣ(*p)==BITVEC_SZ );
  p = sqlite3MallocZero( ಪರಿಮಾಣ(*p) );
  ಯದಿ( p ){
    p->iSize = iSize;
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Check to see if the i-th bit is set.  Return true or false.
** If p is NULL (if the bitmap has not been created) or if
** i is out of range, then return false.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3BitvecTestNotNull(Bitvec *p, u32 i){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  i--;
  ಯದಿ( i>=p->iSize ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯಾವತ್( p->i_ಭಾಜಕ ){
    u32 bin = i/p->i_ಭಾಜಕ;
    i = i%p->i_ಭಾಜಕ;
    p = p->u.apSub[bin];
    ಯದಿ (!p) {
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  }
  ಯದಿ( p->iSize<=BITVEC_NBIT ){
    ಪ್ರತ್ಯಾಗಮ (p->u.aBitmap[i/BITVEC_SZELEM] & (1<<(i&(BITVEC_SZELEM-1))))!=0;
  } ಅನ್ಯಥಾ{
    u32 h = BITVEC_HASH(i++);
    ಯಾವತ್( p->u.aHash[h] ){
      ಯದಿ( p->u.aHash[h]==i ) ಪ್ರತ್ಯಾಗಮ 1;
      h = (h+1) % BITVEC_NINT;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}
ಪೂರ್ಣಾಙ್ಕ sqlite3BitvecTest(Bitvec *p, u32 i){
  ಪ್ರತ್ಯಾಗಮ p!=0 && sqlite3BitvecTestNotNull(p,i);
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
ಪೂರ್ಣಾಙ್ಕ sqlite3BitvecSet(Bitvec *p, u32 i){
  u32 h;
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( i>0 );
  ಪ್ರತಿಜ್ಞಾ( i<=p->iSize );
  i--;
  ಯಾವತ್((p->iSize > BITVEC_NBIT) && p->i_ಭಾಜಕ) {
    u32 bin = i/p->i_ಭಾಜಕ;
    i = i%p->i_ಭಾಜಕ;
    ಯದಿ( p->u.apSub[bin]==0 ){
      p->u.apSub[bin] = sqlite3BitvecCreate( p->i_ಭಾಜಕ );
      ಯದಿ( p->u.apSub[bin]==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    }
    p = p->u.apSub[bin];
  }
  ಯದಿ( p->iSize<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] |= 1 << (i&(BITVEC_SZELEM-1));
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }
  h = BITVEC_HASH(i++);
  /* if there wasn't a hash collision, and this doesn't */
  /* completely fill the hash, then just add it without */
  /* worrying about sub-dividing and re-hashing. */
  ಯದಿ( !p->u.aHash[h] ){
    ಯದಿ (p->nSet<(BITVEC_NINT-1)) {
      ಗಚ್ಛ bitvec_set_end;
    } ಅನ್ಯಥಾ {
      ಗಚ್ಛ bitvec_set_rehash;
    }
  }
  /* there was a collision, check to see if it's already */
  /* in hash, if not, try to find a spot for it */
  ಕುರು {
    ಯದಿ( p->u.aHash[h]==i ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    h++;
    ಯದಿ( h>=BITVEC_NINT ) h = 0;
  } ಯಾವತ್( p->u.aHash[h] );
  /* we didn't find it in the hash.  h points to the first */
  /* available free spot. check to see if this is going to */
  /* make our hash too "full".  */
bitvec_set_rehash:
  ಯದಿ( p->nSet>=BITVEC_MXHASH ){
    unsigned ಪೂರ್ಣಾಙ್ಕ j;
    ಪೂರ್ಣಾಙ್ಕ rc;
    u32 *ai_ಮೌಲ್ಯಗಳು = sqlite3_ರಾಶಿ_Alloc_Raw(0, ಪರಿಮಾಣ(p->u.aHash));
    ಯದಿ( ai_ಮೌಲ್ಯಗಳು==0 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    }ಅನ್ಯಥಾ{
      memcpy(ai_ಮೌಲ್ಯಗಳು, p->u.aHash, ಪರಿಮಾಣ(p->u.aHash));
      memset(p->u.apSub, 0, ಪರಿಮಾಣ(p->u.apSub));
      p->i_ಭಾಜಕ = p->iSize/BITVEC_NPTR;
      ಯದಿ( (p->iSize%BITVEC_NPTR)!=0 ) p->i_ಭಾಜಕ++;
      ಯದಿ( p->i_ಭಾಜಕ<BITVEC_NBIT ) p->i_ಭಾಜಕ = BITVEC_NBIT;
      rc = sqlite3BitvecSet(p, i);
      ಪ್ರತಿ(j=0; j<BITVEC_NINT; j++){
        ಯದಿ( ai_ಮೌಲ್ಯಗಳು[j] ) rc |= sqlite3BitvecSet(p, ai_ಮೌಲ್ಯಗಳು[j]);
      }
      sqlite3_ರಾಶಿ_Free(0, ai_ಮೌಲ್ಯಗಳು);
      ಪ್ರತ್ಯಾಗಮ rc;
    }
  }
bitvec_set_end:
  p->nSet++;
  p->u.aHash[h] = i;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Clear the i-th bit.
**
** pBuf must be a pointer to at least BITVEC_SZ bytes of temporary storage
** that BitvecClear can use to rebuilt its hash table.
*/
ಶೂನ್ಯ sqlite3BitvecClear(Bitvec *p, u32 i, ಶೂನ್ಯ *pBuf){
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿಜ್ಞಾ( i>0 );
  i--;
  ಯಾವತ್( p->i_ಭಾಜಕ ){
    u32 bin = i/p->i_ಭಾಜಕ;
    i = i%p->i_ಭಾಜಕ;
    p = p->u.apSub[bin];
    ಯದಿ (!p) {
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
  ಯದಿ( p->iSize<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] &= ~(BITVEC_TELEM)(1<<(i&(BITVEC_SZELEM-1)));
  }ಅನ್ಯಥಾ{
    unsigned ಪೂರ್ಣಾಙ್ಕ j;
    u32 *ai_ಮೌಲ್ಯಗಳು = pBuf;
    memcpy(ai_ಮೌಲ್ಯಗಳು, p->u.aHash, ಪರಿಮಾಣ(p->u.aHash));
    memset(p->u.aHash, 0, ಪರಿಮಾಣ(p->u.aHash));
    p->nSet = 0;
    ಪ್ರತಿ(j=0; j<BITVEC_NINT; j++){
      ಯದಿ( ai_ಮೌಲ್ಯಗಳು[j] && ai_ಮೌಲ್ಯಗಳು[j]!=(i+1) ){
        u32 h = BITVEC_HASH(ai_ಮೌಲ್ಯಗಳು[j]-1);
        p->nSet++;
        ಯಾವತ್( p->u.aHash[h] ){
          h++;
          ಯದಿ( h>=BITVEC_NINT ) h = 0;
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
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( p->i_ಭಾಜಕ ){
    unsigned ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<BITVEC_NPTR; i++){
      sqlite3BitvecDestroy(p->u.apSub[i]);
    }
  }
  sqlite3_free(p);
}

/*
** Return the value of the iSize parameter specified when Bitvec *p
** was created.
*/
u32 sqlite3BitvecSize(Bitvec *p){
  ಪ್ರತ್ಯಾಗಮ p->iSize;
}

#ifdef SQLITE_DEBUG
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
ಸ್ಥಿರ ಶೂನ್ಯ showBitvec(Bitvec *p, ಪೂರ್ಣಾಙ್ಕ n, unsigned x){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( p==0 ){
    printf("NULL\n");
    ಪ್ರತ್ಯಾಗಮ;
  }
  printf("Bitvec 0x%p iSize=%u", p, p->iSize);
  ಯದಿ( p->iSize<=BITVEC_NBIT ){
    printf(" bitmap\n");
    printf("%*s   bits:", n, "");
    ಪ್ರತಿ(i=1; i<=BITVEC_NBIT; i++){
      ಯದಿ( sqlite3BitvecTest(p,i) ) printf(" %u", x+(unsigned)i);
    }
    printf("\n");
  }ಅನ್ಯಥಾ ಯದಿ( p->i_ಭಾಜಕ==0 ){
    printf(" hash with %u entries\n", p->nSet);
    printf("%*s   bits:", n, "");
    ಪ್ರತಿ(i=0; i<BITVEC_NINT; i++){
      ಯದಿ( p->u.aHash[i] ) printf(" %u", x+(unsigned)p->u.aHash[i]);
    }
    printf("\n");
  }ಅನ್ಯಥಾ{
    printf(" sub-bitvec with iDivisor=%u\n", p->i_ಭಾಜಕ);
    ಪ್ರತಿ(i=0; i<BITVEC_NPTR; i++){
      ಯದಿ( p->u.apSub[i]==0 ) ಅನುವರ್ತ;
      printf("%*s   apSub[%d]=", n, "", i);
      showBitvec(p->u.apSub[i], n+4, i*p->i_ಭಾಜಕ);
    }
  }
}
ಶೂನ್ಯ sqlite3ShowBitvec(Bitvec *p){
  showBitvec(p, 0, 0);
}
#endif

#ifndef SQLITE_UNTESTABLE
/*
** Let V[] be an array of unsigned characters sufficient to hold
** up to N bits.  Let I be an integer between 0 and N.  0<=I<N.
** Then the following macros can be used to set, clear, or test
** individual bits within V.
*/
#define SETBIT(V,I)      V[I>>3] |= (1<<(I&7))
#define CLEARBIT(V,I)    V[I>>3] &= ~(BITVEC_TELEM)(1<<(I&7))
#define TESTBIT(V,I)     (V[I>>3]&(1<<(I&7)))!=0


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
ಪೂರ್ಣಾಙ್ಕ sqlite3BitvecBuiltinTest(ಪೂರ್ಣಾಙ್ಕ sz, ಪೂರ್ಣಾಙ್ಕ *aOp){
  Bitvec *pBitvec = 0;
  unsigned ವರ್ಣ *pV = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = -1;
  ಪೂರ್ಣಾಙ್ಕ i, nx, pc, op;
  ಶೂನ್ಯ *pTmpSpace;

  /* Allocate the Bitvec to be tested and a linear array of
  ** bits to act as the reference */
  ಯದಿ( sz<=0 ){
    pBitvec = sqlite3BitvecCreate( 2*(unsigned)(-sz) );
    pV = 0;
  }ಅನ್ಯಥಾ{
    pBitvec = sqlite3BitvecCreate( sz );
    pV = sqlite3MallocZero( (7+(i64)sz)/8 + 1 );
  }
  pTmpSpace = sqlite3_malloc64(BITVEC_SZ);
  ಯದಿ( pBitvec==0 || pTmpSpace==0 || (pV==0 && sz>0) ) ಗಚ್ಛ bitvec_end;

  /* NULL pBitvec tests */
  sqlite3BitvecSet(0, 1);
  sqlite3BitvecClear(0, 1, pTmpSpace);

  /* Run the program */
  pc = i = 0;
  ಯಾವತ್( (op = aOp[pc])!=0 ){
    ಯದಿ( op>=6 ){
#ifdef SQLITE_DEBUG
      ಯದಿ( op==6 ){
        sqlite3ShowBitvec(pBitvec);
      }ಅನ್ಯಥಾ ಯದಿ( op==7 ){
        printf("BITVEC_SZ     = %d (%d by sizeof)\n",
               BITVEC_SZ, (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(Bitvec));
        printf("BITVEC_USIZE  = %d\n", (ಪೂರ್ಣಾಙ್ಕ)BITVEC_USIZE);
        printf("BITVEC_NELEM  = %d\n", (ಪೂರ್ಣಾಙ್ಕ)BITVEC_NELEM);
        printf("BITVEC_NBIT   = %d\n", (ಪೂರ್ಣಾಙ್ಕ)BITVEC_NBIT);
        printf("BITVEC_NINT   = %d\n", (ಪೂರ್ಣಾಙ್ಕ)BITVEC_NINT);
        printf("BITVEC_MXHASH = %d\n", (ಪೂರ್ಣಾಙ್ಕ)BITVEC_MXHASH);
        printf("BITVEC_NPTR   = %d\n", (ಪೂರ್ಣಾಙ್ಕ)BITVEC_NPTR);
      }
#endif
      pc++;
      ಅನುವರ್ತ;
    }
    ವಿಕಲ್ಪ( op ){
      ಪಕ್ಷ 1:
      ಪಕ್ಷ 2:
      ಪಕ್ಷ 5: {
        nx = 4;
        i = aOp[pc+2] - 1;
        aOp[pc+2] += aOp[pc+3];
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 3:
      ಪಕ್ಷ 4:
      ಸ್ವಭಾವ: {
        nx = 2;
        sqlite3_randomness(ಪರಿಮಾಣ(i), &i);
        ಭಙ್ಗ;
      }
    }
    ಯದಿ( (--aOp[pc+1]) > 0 ) nx = 0;
    pc += nx;
    i = (i & 0x7fffffff)%sz;
    ಯದಿ( (op & 1)!=0 ){
      ಯದಿ( pV ) SETBIT(pV, (i+1));
      ಯದಿ( op!=5 ){
        ಯದಿ( sqlite3BitvecSet(pBitvec, i+1) ) ಗಚ್ಛ bitvec_end;
      }
    }ಅನ್ಯಥಾ{
      ಯದಿ( pV ) CLEARBIT(pV, (i+1));
      sqlite3BitvecClear(pBitvec, i+1, pTmpSpace);
    }
  }

  /* Test to make sure the linear array exactly matches the
  ** Bitvec object.  Start with the assumption that they do
  ** match (rc==0).  Change rc to non-zero if a discrepancy
  ** is found.
  */
  ಯದಿ( pV ){
    rc = sqlite3BitvecTest(0,0) + sqlite3BitvecTest(pBitvec, sz+1)
            + sqlite3BitvecTest(pBitvec, 0)
            + (sqlite3BitvecSize(pBitvec) - sz);
    ಪ್ರತಿ(i=1; i<=sz; i++){
      ಯದಿ( (TESTBIT(pV,i))!=sqlite3BitvecTest(pBitvec,i) ){
        rc = i;
        ಭಙ್ಗ;
      }
    }
  }ಅನ್ಯಥಾ{
    rc = 0;
  }

  /* Free allocated structure */
bitvec_end:
  sqlite3_free(pTmpSpace);
  sqlite3_free(pV);
  sqlite3BitvecDestroy(pBitvec);
  ಪ್ರತ್ಯಾಗಮ rc;
}
#endif /* SQLITE_UNTESTABLE */
