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
#అన్తర్భావ "sqliteInt.h"

/* Size of the Bitvec structure in bytes. */
#నిర్వచించు BITVEC_SZ        512

/* Round the union size down to the nearest pointer boundary, since that's how
** it will be aligned within the Bitvec struct. */
#నిర్వచించు BITVEC_USIZE \
    (((BITVEC_SZ-(3*పరిమాణం(u32)))/పరిమాణం(Bitvec*))*పరిమాణం(Bitvec*))

/* Type of the array "element" for the bitmap representation.
** Should be a power of 2, and ideally, evenly divide into BITVEC_USIZE.
** Setting this to the "natural word" size of your CPU may improve
** performance. */
#నిర్వచించు BITVEC_TELEM     u8
/* Size, in bits, of the bitmap element. */
#నిర్వచించు BITVEC_SZELEM    8
/* Number of elements in a bitmap array. */
#నిర్వచించు BITVEC_NELEM     (BITVEC_USIZE/పరిమాణం(BITVEC_TELEM))
/* Number of bits in the bitmap array. */
#నిర్వచించు BITVEC_NBIT      (BITVEC_NELEM*BITVEC_SZELEM)

/* Number of u32 values in hash table. */
#నిర్వచించు BITVEC_NINT      (BITVEC_USIZE/పరిమాణం(u32))
/* Maximum number of entries in hash table before
** sub-dividing and re-hashing. */
#నిర్వచించు BITVEC_MXHASH    (BITVEC_NINT/2)
/* Hashing function for the aHash representation.
** Empirical testing showed that the *37 multiplier
** (an arbitrary prime)in the hash function provided
** no fewer collisions than the no-op *1. */
#నిర్వచించు BITVEC_HASH(X)   (((X)*1)%BITVEC_NINT)

#నిర్వచించు BITVEC_NPTR      ((u32)(BITVEC_USIZE/పరిమాణం(Bitvec *)))


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
నిర్మాణం Bitvec {
  u32 i_కొలత;      /* Maximum bit index.  Max iSize is 4,294,967,296. */
  u32 n_సమితి;       /* Number of bits that are set - only valid for aHash
                  ** element.  Max is BITVEC_NINT.  For BITVEC_SZ of 512,
                  ** this would be 125. */
  u32 i_భాజకం;   /* Number of bits handled by each apSub[] entry. */
                  /* Should >=0 for apSub element. */
                  /* Max iDivisor is max(u32) / BITVEC_NPTR + 1.  */
                  /* For a BITVEC_SZ of 512, this would be 34,359,739. */
  సంఘం {
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
Bitvec *sqlite3BitvecCreate(u32 i_కొలత){
  Bitvec *p;
  ప్రతిజ్ఞా( పరిమాణం(*p)==BITVEC_SZ );
  p = sqlite3MallocZero( పరిమాణం(*p) );
  ఒకవేళ( p ){
    p->i_కొలత = i_కొలత;
  }
  తిరిగి p;
}

/*
** Check to see if the i-th bit is set.  Return true or false.
** If p is NULL (if the bitmap has not been created) or if
** i is out of range, then return false.
*/
పూర్ణాంకం sqlite3BitvecTestNotNull(Bitvec *p, u32 i){
  ప్రతిజ్ఞా( p!=0 );
  i--;
  ఒకవేళ( i>=p->i_కొలత ) తిరిగి 0;
  అయితే( p->i_భాజకం ){
    u32 bin = i/p->i_భాజకం;
    i = i%p->i_భాజకం;
    p = p->u.apSub[bin];
    ఒకవేళ (!p) {
      తిరిగి 0;
    }
  }
  ఒకవేళ( p->i_కొలత<=BITVEC_NBIT ){
    తిరిగి (p->u.aBitmap[i/BITVEC_SZELEM] & (1<<(i&(BITVEC_SZELEM-1))))!=0;
  } లేకపోతే{
    u32 h = BITVEC_HASH(i++);
    అయితే( p->u.aHash[h] ){
      ఒకవేళ( p->u.aHash[h]==i ) తిరిగి 1;
      h = (h+1) % BITVEC_NINT;
    }
    తిరిగి 0;
  }
}
పూర్ణాంకం sqlite3BitvecTest(Bitvec *p, u32 i){
  తిరిగి p!=0 && sqlite3BitvecTestNotNull(p,i);
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
పూర్ణాంకం sqlite3_Bitvec_సమితి(Bitvec *p, u32 i){
  u32 h;
  ఒకవేళ( p==0 ) తిరిగి SQLITE_OK;
  ప్రతిజ్ఞా( i>0 );
  ప్రతిజ్ఞా( i<=p->i_కొలత );
  i--;
  అయితే((p->i_కొలత > BITVEC_NBIT) && p->i_భాజకం) {
    u32 bin = i/p->i_భాజకం;
    i = i%p->i_భాజకం;
    ఒకవేళ( p->u.apSub[bin]==0 ){
      p->u.apSub[bin] = sqlite3BitvecCreate( p->i_భాజకం );
      ఒకవేళ( p->u.apSub[bin]==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    }
    p = p->u.apSub[bin];
  }
  ఒకవేళ( p->i_కొలత<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] |= 1 << (i&(BITVEC_SZELEM-1));
    తిరిగి SQLITE_OK;
  }
  h = BITVEC_HASH(i++);
  /* if there wasn't a hash collision, and this doesn't */
  /* completely fill the hash, then just add it without */
  /* worrying about sub-dividing and re-hashing. */
  ఒకవేళ( !p->u.aHash[h] ){
    ఒకవేళ (p->n_సమితి<(BITVEC_NINT-1)) {
      వెళ్ళు bitvec_సమితి_చివర;
    } లేకపోతే {
      వెళ్ళు bitvec_సమితి_rehash;
    }
  }
  /* there was a collision, check to see if it's already */
  /* in hash, if not, try to find a spot for it */
  చేయి {
    ఒకవేళ( p->u.aHash[h]==i ) తిరిగి SQLITE_OK;
    h++;
    ఒకవేళ( h>=BITVEC_NINT ) h = 0;
  } అయితే( p->u.aHash[h] );
  /* we didn't find it in the hash.  h points to the first */
  /* available free spot. check to see if this is going to */
  /* make our hash too "full".  */
bitvec_సమితి_rehash:
  ఒకవేళ( p->n_సమితి>=BITVEC_MXHASH ){
    గుర్తులేని పూర్ణాంకం j;
    పూర్ణాంకం rc;
    u32 *ai_విలువలు = sqlite3_దొంతర_Alloc_Raw(0, పరిమాణం(p->u.aHash));
    ఒకవేళ( ai_విలువలు==0 ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }లేకపోతే{
      జ్ఞాపకం_నకలు(ai_విలువలు, p->u.aHash, పరిమాణం(p->u.aHash));
      memset(p->u.apSub, 0, పరిమాణం(p->u.apSub));
      p->i_భాజకం = p->i_కొలత/BITVEC_NPTR;
      ఒకవేళ( (p->i_కొలత%BITVEC_NPTR)!=0 ) p->i_భాజకం++;
      ఒకవేళ( p->i_భాజకం<BITVEC_NBIT ) p->i_భాజకం = BITVEC_NBIT;
      rc = sqlite3_Bitvec_సమితి(p, i);
      కొరకు(j=0; j<BITVEC_NINT; j++){
        ఒకవేళ( ai_విలువలు[j] ) rc |= sqlite3_Bitvec_సమితి(p, ai_విలువలు[j]);
      }
      sqlite3_దొంతర_Free(0, ai_విలువలు);
      తిరిగి rc;
    }
  }
bitvec_సమితి_చివర:
  p->n_సమితి++;
  p->u.aHash[h] = i;
  తిరిగి SQLITE_OK;
}

/*
** Clear the i-th bit.
**
** pBuf must be a pointer to at least BITVEC_SZ bytes of temporary storage
** that BitvecClear can use to rebuilt its hash table.
*/
శూన్య sqlite3BitvecClear(Bitvec *p, u32 i, శూన్య *pBuf){
  ఒకవేళ( p==0 ) తిరిగి;
  ప్రతిజ్ఞా( i>0 );
  i--;
  అయితే( p->i_భాజకం ){
    u32 bin = i/p->i_భాజకం;
    i = i%p->i_భాజకం;
    p = p->u.apSub[bin];
    ఒకవేళ (!p) {
      తిరిగి;
    }
  }
  ఒకవేళ( p->i_కొలత<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] &= ~(BITVEC_TELEM)(1<<(i&(BITVEC_SZELEM-1)));
  }లేకపోతే{
    గుర్తులేని పూర్ణాంకం j;
    u32 *ai_విలువలు = pBuf;
    జ్ఞాపకం_నకలు(ai_విలువలు, p->u.aHash, పరిమాణం(p->u.aHash));
    memset(p->u.aHash, 0, పరిమాణం(p->u.aHash));
    p->n_సమితి = 0;
    కొరకు(j=0; j<BITVEC_NINT; j++){
      ఒకవేళ( ai_విలువలు[j] && ai_విలువలు[j]!=(i+1) ){
        u32 h = BITVEC_HASH(ai_విలువలు[j]-1);
        p->n_సమితి++;
        అయితే( p->u.aHash[h] ){
          h++;
          ఒకవేళ( h>=BITVEC_NINT ) h = 0;
        }
        p->u.aHash[h] = ai_విలువలు[j];
      }
    }
  }
}

/*
** Destroy a bitmap object.  Reclaim all memory used.
*/
శూన్య sqlite3BitvecDestroy(Bitvec *p){
  ఒకవేళ( p==0 ) తిరిగి;
  ఒకవేళ( p->i_భాజకం ){
    గుర్తులేని పూర్ణాంకం i;
    కొరకు(i=0; i<BITVEC_NPTR; i++){
      sqlite3BitvecDestroy(p->u.apSub[i]);
    }
  }
  sqlite3_free(p);
}

/*
** Return the value of the iSize parameter specified when Bitvec *p
** was created.
*/
u32 sqlite3_Bitvec_కొలత(Bitvec *p){
  తిరిగి p->i_కొలత;
}

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర శూన్య showBitvec(Bitvec *p, పూర్ణాంకం n, గుర్తులేని x){
  పూర్ణాంకం i;
  ఒకవేళ( p==0 ){
    రాయి("NULL\n");
    తిరిగి;
  }
  రాయి("Bitvec 0x%p iSize=%u", p, p->i_కొలత);
  ఒకవేళ( p->i_కొలత<=BITVEC_NBIT ){
    రాయి(" bitmap\n");
    రాయి("%*s   bits:", n, "");
    కొరకు(i=1; i<=BITVEC_NBIT; i++){
      ఒకవేళ( sqlite3BitvecTest(p,i) ) రాయి(" %u", x+(గుర్తులేని)i);
    }
    రాయి("\n");
  }లేకపోతే ఒకవేళ( p->i_భాజకం==0 ){
    రాయి(" hash with %u entries\n", p->n_సమితి);
    రాయి("%*s   bits:", n, "");
    కొరకు(i=0; i<BITVEC_NINT; i++){
      ఒకవేళ( p->u.aHash[i] ) రాయి(" %u", x+(గుర్తులేని)p->u.aHash[i]);
    }
    రాయి("\n");
  }లేకపోతే{
    రాయి(" sub-bitvec with iDivisor=%u\n", p->i_భాజకం);
    కొరకు(i=0; i<BITVEC_NPTR; i++){
      ఒకవేళ( p->u.apSub[i]==0 ) కొనసాగు;
      రాయి("%*s   apSub[%d]=", n, "", i);
      showBitvec(p->u.apSub[i], n+4, i*p->i_భాజకం);
    }
  }
}
శూన్య sqlite3ShowBitvec(Bitvec *p){
  showBitvec(p, 0, 0);
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
/*
** Let V[] be an array of unsigned characters sufficient to hold
** up to N bits.  Let I be an integer between 0 and N.  0<=I<N.
** Then the following macros can be used to set, clear, or test
** individual bits within V.
*/
#నిర్వచించు SETBIT(V,I)      V[I>>3] |= (1<<(I&7))
#నిర్వచించు CLEARBIT(V,I)    V[I>>3] &= ~(BITVEC_TELEM)(1<<(I&7))
#నిర్వచించు TESTBIT(V,I)     (V[I>>3]&(1<<(I&7)))!=0


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
పూర్ణాంకం sqlite3BitvecBuiltinTest(పూర్ణాంకం sz, పూర్ణాంకం *aOp){
  Bitvec *pBitvec = 0;
  గుర్తులేని అక్షరం *pV = 0;
  పూర్ణాంకం rc = -1;
  పూర్ణాంకం i, nx, pc, op;
  శూన్య *pTmpSpace;

  /* Allocate the Bitvec to be tested and a linear array of
  ** bits to act as the reference */
  ఒకవేళ( sz<=0 ){
    pBitvec = sqlite3BitvecCreate( 2*(గుర్తులేని)(-sz) );
    pV = 0;
  }లేకపోతే{
    pBitvec = sqlite3BitvecCreate( sz );
    pV = sqlite3MallocZero( (7+(i64)sz)/8 + 1 );
  }
  pTmpSpace = sqlite3_malloc64(BITVEC_SZ);
  ఒకవేళ( pBitvec==0 || pTmpSpace==0 || (pV==0 && sz>0) ) వెళ్ళు bitvec_చివర;

  /* NULL pBitvec tests */
  sqlite3_Bitvec_సమితి(0, 1);
  sqlite3BitvecClear(0, 1, pTmpSpace);

  /* Run the program */
  pc = i = 0;
  అయితే( (op = aOp[pc])!=0 ){
    ఒకవేళ( op>=6 ){
#నిర్వచితమైతే SQLITE_DEBUG
      ఒకవేళ( op==6 ){
        sqlite3ShowBitvec(pBitvec);
      }లేకపోతే ఒకవేళ( op==7 ){
        రాయి("BITVEC_SZ     = %d (%d by sizeof)\n",
               BITVEC_SZ, (పూర్ణాంకం)పరిమాణం(Bitvec));
        రాయి("BITVEC_USIZE  = %d\n", (పూర్ణాంకం)BITVEC_USIZE);
        రాయి("BITVEC_NELEM  = %d\n", (పూర్ణాంకం)BITVEC_NELEM);
        రాయి("BITVEC_NBIT   = %d\n", (పూర్ణాంకం)BITVEC_NBIT);
        రాయి("BITVEC_NINT   = %d\n", (పూర్ణాంకం)BITVEC_NINT);
        రాయి("BITVEC_MXHASH = %d\n", (పూర్ణాంకం)BITVEC_MXHASH);
        రాయి("BITVEC_NPTR   = %d\n", (పూర్ణాంకం)BITVEC_NPTR);
      }
#అయితే_ముగింపు
      pc++;
      కొనసాగు;
    }
    ఎంపిక( op ){
      స్థితి 1:
      స్థితి 2:
      స్థితి 5: {
        nx = 4;
        i = aOp[pc+2] - 1;
        aOp[pc+2] += aOp[pc+3];
        ఆపు;
      }
      స్థితి 3:
      స్థితి 4:
      అప్రమేయ: {
        nx = 2;
        sqlite3_randomness(పరిమాణం(i), &i);
        ఆపు;
      }
    }
    ఒకవేళ( (--aOp[pc+1]) > 0 ) nx = 0;
    pc += nx;
    i = (i & 0x7fffffff)%sz;
    ఒకవేళ( (op & 1)!=0 ){
      ఒకవేళ( pV ) SETBIT(pV, (i+1));
      ఒకవేళ( op!=5 ){
        ఒకవేళ( sqlite3_Bitvec_సమితి(pBitvec, i+1) ) వెళ్ళు bitvec_చివర;
      }
    }లేకపోతే{
      ఒకవేళ( pV ) CLEARBIT(pV, (i+1));
      sqlite3BitvecClear(pBitvec, i+1, pTmpSpace);
    }
  }

  /* Test to make sure the linear array exactly matches the
  ** Bitvec object.  Start with the assumption that they do
  ** match (rc==0).  Change rc to non-zero if a discrepancy
  ** is found.
  */
  ఒకవేళ( pV ){
    rc = sqlite3BitvecTest(0,0) + sqlite3BitvecTest(pBitvec, sz+1)
            + sqlite3BitvecTest(pBitvec, 0)
            + (sqlite3_Bitvec_కొలత(pBitvec) - sz);
    కొరకు(i=1; i<=sz; i++){
      ఒకవేళ( (TESTBIT(pV,i))!=sqlite3BitvecTest(pBitvec,i) ){
        rc = i;
        ఆపు;
      }
    }
  }లేకపోతే{
    rc = 0;
  }

  /* Free allocated structure */
bitvec_చివర:
  sqlite3_free(pTmpSpace);
  sqlite3_free(pV);
  sqlite3BitvecDestroy(pBitvec);
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_UNTESTABLE */
