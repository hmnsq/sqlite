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
#अन्तर्भाव "sqliteInt.h"

/* Size of the Bitvec structure in bytes. */
#परिभाषय BITVEC_SZ        512

/* Round the union size down to the nearest pointer boundary, since that's how
** it will be aligned within the Bitvec struct. */
#परिभाषय BITVEC_USIZE \
    (((BITVEC_SZ-(3*परिमाण(u32)))/परिमाण(Bitvec*))*परिमाण(Bitvec*))

/* Type of the array "element" for the bitmap representation.
** Should be a power of 2, and ideally, evenly divide into BITVEC_USIZE.
** Setting this to the "natural word" size of your CPU may improve
** performance. */
#परिभाषय BITVEC_TELEM     u8
/* Size, in bits, of the bitmap element. */
#परिभाषय BITVEC_SZELEM    8
/* Number of elements in a bitmap array. */
#परिभाषय BITVEC_NELEM     (BITVEC_USIZE/परिमाण(BITVEC_TELEM))
/* Number of bits in the bitmap array. */
#परिभाषय BITVEC_NBIT      (BITVEC_NELEM*BITVEC_SZELEM)

/* Number of u32 values in hash table. */
#परिभाषय BITVEC_NINT      (BITVEC_USIZE/परिमाण(u32))
/* Maximum number of entries in hash table before
** sub-dividing and re-hashing. */
#परिभाषय BITVEC_MXHASH    (BITVEC_NINT/2)
/* Hashing function for the aHash representation.
** Empirical testing showed that the *37 multiplier
** (an arbitrary prime)in the hash function provided
** no fewer collisions than the no-op *1. */
#परिभाषय BITVEC_HASH(X)   (((X)*1)%BITVEC_NINT)

#परिभाषय BITVEC_NPTR      ((u32)(BITVEC_USIZE/परिमाण(Bitvec *)))


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
संरचना Bitvec {
  u32 i_आकार;      /* Maximum bit index.  Max iSize is 4,294,967,296. */
  u32 n_समुच्चयः;       /* Number of bits that are set - only valid for aHash
                  ** element.  Max is BITVEC_NINT.  For BITVEC_SZ of 512,
                  ** this would be 125. */
  u32 i_भाजकः;   /* Number of bits handled by each apSub[] entry. */
                  /* Should >=0 for apSub element. */
                  /* Max iDivisor is max(u32) / BITVEC_NPTR + 1.  */
                  /* For a BITVEC_SZ of 512, this would be 34,359,739. */
  सङ्घ {
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
Bitvec *sqlite3BitvecCreate(u32 i_आकार){
  Bitvec *p;
  प्रतिज्ञा( परिमाण(*p)==BITVEC_SZ );
  p = sqlite3MallocZero( परिमाण(*p) );
  यदि( p ){
    p->i_आकार = i_आकार;
  }
  प्रत्यागम p;
}

/*
** Check to see if the i-th bit is set.  Return true or false.
** If p is NULL (if the bitmap has not been created) or if
** i is out of range, then return false.
*/
पूर्णाङ्क sqlite3BitvecTestNotNull(Bitvec *p, u32 i){
  प्रतिज्ञा( p!=0 );
  i--;
  यदि( i>=p->i_आकार ) प्रत्यागम 0;
  यावत्( p->i_भाजकः ){
    u32 bin = i/p->i_भाजकः;
    i = i%p->i_भाजकः;
    p = p->u.apSub[bin];
    यदि (!p) {
      प्रत्यागम 0;
    }
  }
  यदि( p->i_आकार<=BITVEC_NBIT ){
    प्रत्यागम (p->u.aBitmap[i/BITVEC_SZELEM] & (1<<(i&(BITVEC_SZELEM-1))))!=0;
  } अन्यथा{
    u32 h = BITVEC_HASH(i++);
    यावत्( p->u.aHash[h] ){
      यदि( p->u.aHash[h]==i ) प्रत्यागम 1;
      h = (h+1) % BITVEC_NINT;
    }
    प्रत्यागम 0;
  }
}
पूर्णाङ्क sqlite3BitvecTest(Bitvec *p, u32 i){
  प्रत्यागम p!=0 && sqlite3BitvecTestNotNull(p,i);
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
पूर्णाङ्क sqlite3_Bitvec_समुच्चयः(Bitvec *p, u32 i){
  u32 h;
  यदि( p==0 ) प्रत्यागम SQLITE_OK;
  प्रतिज्ञा( i>0 );
  प्रतिज्ञा( i<=p->i_आकार );
  i--;
  यावत्((p->i_आकार > BITVEC_NBIT) && p->i_भाजकः) {
    u32 bin = i/p->i_भाजकः;
    i = i%p->i_भाजकः;
    यदि( p->u.apSub[bin]==0 ){
      p->u.apSub[bin] = sqlite3BitvecCreate( p->i_भाजकः );
      यदि( p->u.apSub[bin]==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    p = p->u.apSub[bin];
  }
  यदि( p->i_आकार<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] |= 1 << (i&(BITVEC_SZELEM-1));
    प्रत्यागम SQLITE_OK;
  }
  h = BITVEC_HASH(i++);
  /* if there wasn't a hash collision, and this doesn't */
  /* completely fill the hash, then just add it without */
  /* worrying about sub-dividing and re-hashing. */
  यदि( !p->u.aHash[h] ){
    यदि (p->n_समुच्चयः<(BITVEC_NINT-1)) {
      गच्छ bitvec_समुच्चयः_अन्तः;
    } अन्यथा {
      गच्छ bitvec_समुच्चयः_rehash;
    }
  }
  /* there was a collision, check to see if it's already */
  /* in hash, if not, try to find a spot for it */
  कुरु {
    यदि( p->u.aHash[h]==i ) प्रत्यागम SQLITE_OK;
    h++;
    यदि( h>=BITVEC_NINT ) h = 0;
  } यावत्( p->u.aHash[h] );
  /* we didn't find it in the hash.  h points to the first */
  /* available free spot. check to see if this is going to */
  /* make our hash too "full".  */
bitvec_समुच्चयः_rehash:
  यदि( p->n_समुच्चयः>=BITVEC_MXHASH ){
    अचिह्नित पूर्णाङ्क j;
    पूर्णाङ्क rc;
    u32 *ai_मानानि = sqlite3_राशिः_Alloc_Raw(0, परिमाण(p->u.aHash));
    यदि( ai_मानानि==0 ){
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }अन्यथा{
      स्मृतिं_प्रतिलिख(ai_मानानि, p->u.aHash, परिमाण(p->u.aHash));
      स्मृतिं_पूरय(p->u.apSub, 0, परिमाण(p->u.apSub));
      p->i_भाजकः = p->i_आकार/BITVEC_NPTR;
      यदि( (p->i_आकार%BITVEC_NPTR)!=0 ) p->i_भाजकः++;
      यदि( p->i_भाजकः<BITVEC_NBIT ) p->i_भाजकः = BITVEC_NBIT;
      rc = sqlite3_Bitvec_समुच्चयः(p, i);
      प्रति(j=0; j<BITVEC_NINT; j++){
        यदि( ai_मानानि[j] ) rc |= sqlite3_Bitvec_समुच्चयः(p, ai_मानानि[j]);
      }
      sqlite3_राशिः_Free(0, ai_मानानि);
      प्रत्यागम rc;
    }
  }
bitvec_समुच्चयः_अन्तः:
  p->n_समुच्चयः++;
  p->u.aHash[h] = i;
  प्रत्यागम SQLITE_OK;
}

/*
** Clear the i-th bit.
**
** pBuf must be a pointer to at least BITVEC_SZ bytes of temporary storage
** that BitvecClear can use to rebuilt its hash table.
*/
शून्य sqlite3BitvecClear(Bitvec *p, u32 i, शून्य *pBuf){
  यदि( p==0 ) प्रत्यागम;
  प्रतिज्ञा( i>0 );
  i--;
  यावत्( p->i_भाजकः ){
    u32 bin = i/p->i_भाजकः;
    i = i%p->i_भाजकः;
    p = p->u.apSub[bin];
    यदि (!p) {
      प्रत्यागम;
    }
  }
  यदि( p->i_आकार<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] &= ~(BITVEC_TELEM)(1<<(i&(BITVEC_SZELEM-1)));
  }अन्यथा{
    अचिह्नित पूर्णाङ्क j;
    u32 *ai_मानानि = pBuf;
    स्मृतिं_प्रतिलिख(ai_मानानि, p->u.aHash, परिमाण(p->u.aHash));
    स्मृतिं_पूरय(p->u.aHash, 0, परिमाण(p->u.aHash));
    p->n_समुच्चयः = 0;
    प्रति(j=0; j<BITVEC_NINT; j++){
      यदि( ai_मानानि[j] && ai_मानानि[j]!=(i+1) ){
        u32 h = BITVEC_HASH(ai_मानानि[j]-1);
        p->n_समुच्चयः++;
        यावत्( p->u.aHash[h] ){
          h++;
          यदि( h>=BITVEC_NINT ) h = 0;
        }
        p->u.aHash[h] = ai_मानानि[j];
      }
    }
  }
}

/*
** Destroy a bitmap object.  Reclaim all memory used.
*/
शून्य sqlite3BitvecDestroy(Bitvec *p){
  यदि( p==0 ) प्रत्यागम;
  यदि( p->i_भाजकः ){
    अचिह्नित पूर्णाङ्क i;
    प्रति(i=0; i<BITVEC_NPTR; i++){
      sqlite3BitvecDestroy(p->u.apSub[i]);
    }
  }
  sqlite3_free(p);
}

/*
** Return the value of the iSize parameter specified when Bitvec *p
** was created.
*/
u32 sqlite3_Bitvec_आकार(Bitvec *p){
  प्रत्यागम p->i_आकार;
}

#यदि_परिभाषितम् SQLITE_DEBUG
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
स्थिर शून्य showBitvec(Bitvec *p, पूर्णाङ्क n, अचिह्नित x){
  पूर्णाङ्क i;
  यदि( p==0 ){
    प्रारूप_लिख("NULL\n");
    प्रत्यागम;
  }
  प्रारूप_लिख("Bitvec 0x%p iSize=%u", p, p->i_आकार);
  यदि( p->i_आकार<=BITVEC_NBIT ){
    प्रारूप_लिख(" bitmap\n");
    प्रारूप_लिख("%*s   bits:", n, "");
    प्रति(i=1; i<=BITVEC_NBIT; i++){
      यदि( sqlite3BitvecTest(p,i) ) प्रारूप_लिख(" %u", x+(अचिह्नित)i);
    }
    प्रारूप_लिख("\n");
  }अन्यथा यदि( p->i_भाजकः==0 ){
    प्रारूप_लिख(" hash with %u entries\n", p->n_समुच्चयः);
    प्रारूप_लिख("%*s   bits:", n, "");
    प्रति(i=0; i<BITVEC_NINT; i++){
      यदि( p->u.aHash[i] ) प्रारूप_लिख(" %u", x+(अचिह्नित)p->u.aHash[i]);
    }
    प्रारूप_लिख("\n");
  }अन्यथा{
    प्रारूप_लिख(" sub-bitvec with iDivisor=%u\n", p->i_भाजकः);
    प्रति(i=0; i<BITVEC_NPTR; i++){
      यदि( p->u.apSub[i]==0 ) अनुवर्त;
      प्रारूप_लिख("%*s   apSub[%d]=", n, "", i);
      showBitvec(p->u.apSub[i], n+4, i*p->i_भाजकः);
    }
  }
}
शून्य sqlite3ShowBitvec(Bitvec *p){
  showBitvec(p, 0, 0);
}
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_UNTESTABLE
/*
** Let V[] be an array of unsigned characters sufficient to hold
** up to N bits.  Let I be an integer between 0 and N.  0<=I<N.
** Then the following macros can be used to set, clear, or test
** individual bits within V.
*/
#परिभाषय SETBIT(V,I)      V[I>>3] |= (1<<(I&7))
#परिभाषय CLEARBIT(V,I)    V[I>>3] &= ~(BITVEC_TELEM)(1<<(I&7))
#परिभाषय TESTBIT(V,I)     (V[I>>3]&(1<<(I&7)))!=0


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
पूर्णाङ्क sqlite3BitvecBuiltinTest(पूर्णाङ्क sz, पूर्णाङ्क *aOp){
  Bitvec *pBitvec = 0;
  अचिह्नित वर्ण *pV = 0;
  पूर्णाङ्क rc = -1;
  पूर्णाङ्क i, nx, pc, op;
  शून्य *pTmpSpace;

  /* Allocate the Bitvec to be tested and a linear array of
  ** bits to act as the reference */
  यदि( sz<=0 ){
    pBitvec = sqlite3BitvecCreate( 2*(अचिह्नित)(-sz) );
    pV = 0;
  }अन्यथा{
    pBitvec = sqlite3BitvecCreate( sz );
    pV = sqlite3MallocZero( (7+(i64)sz)/8 + 1 );
  }
  pTmpSpace = sqlite3_malloc64(BITVEC_SZ);
  यदि( pBitvec==0 || pTmpSpace==0 || (pV==0 && sz>0) ) गच्छ bitvec_अन्तः;

  /* NULL pBitvec tests */
  sqlite3_Bitvec_समुच्चयः(0, 1);
  sqlite3BitvecClear(0, 1, pTmpSpace);

  /* Run the program */
  pc = i = 0;
  यावत्( (op = aOp[pc])!=0 ){
    यदि( op>=6 ){
#यदि_परिभाषितम् SQLITE_DEBUG
      यदि( op==6 ){
        sqlite3ShowBitvec(pBitvec);
      }अन्यथा यदि( op==7 ){
        प्रारूप_लिख("BITVEC_SZ     = %d (%d by sizeof)\n",
               BITVEC_SZ, (पूर्णाङ्क)परिमाण(Bitvec));
        प्रारूप_लिख("BITVEC_USIZE  = %d\n", (पूर्णाङ्क)BITVEC_USIZE);
        प्रारूप_लिख("BITVEC_NELEM  = %d\n", (पूर्णाङ्क)BITVEC_NELEM);
        प्रारूप_लिख("BITVEC_NBIT   = %d\n", (पूर्णाङ्क)BITVEC_NBIT);
        प्रारूप_लिख("BITVEC_NINT   = %d\n", (पूर्णाङ्क)BITVEC_NINT);
        प्रारूप_लिख("BITVEC_MXHASH = %d\n", (पूर्णाङ्क)BITVEC_MXHASH);
        प्रारूप_लिख("BITVEC_NPTR   = %d\n", (पूर्णाङ्क)BITVEC_NPTR);
      }
#यदि_अन्तः
      pc++;
      अनुवर्त;
    }
    विकल्प( op ){
      पक्ष 1:
      पक्ष 2:
      पक्ष 5: {
        nx = 4;
        i = aOp[pc+2] - 1;
        aOp[pc+2] += aOp[pc+3];
        भङ्ग;
      }
      पक्ष 3:
      पक्ष 4:
      स्वभाव: {
        nx = 2;
        sqlite3_randomness(परिमाण(i), &i);
        भङ्ग;
      }
    }
    यदि( (--aOp[pc+1]) > 0 ) nx = 0;
    pc += nx;
    i = (i & 0x7fffffff)%sz;
    यदि( (op & 1)!=0 ){
      यदि( pV ) SETBIT(pV, (i+1));
      यदि( op!=5 ){
        यदि( sqlite3_Bitvec_समुच्चयः(pBitvec, i+1) ) गच्छ bitvec_अन्तः;
      }
    }अन्यथा{
      यदि( pV ) CLEARBIT(pV, (i+1));
      sqlite3BitvecClear(pBitvec, i+1, pTmpSpace);
    }
  }

  /* Test to make sure the linear array exactly matches the
  ** Bitvec object.  Start with the assumption that they do
  ** match (rc==0).  Change rc to non-zero if a discrepancy
  ** is found.
  */
  यदि( pV ){
    rc = sqlite3BitvecTest(0,0) + sqlite3BitvecTest(pBitvec, sz+1)
            + sqlite3BitvecTest(pBitvec, 0)
            + (sqlite3_Bitvec_आकार(pBitvec) - sz);
    प्रति(i=1; i<=sz; i++){
      यदि( (TESTBIT(pV,i))!=sqlite3BitvecTest(pBitvec,i) ){
        rc = i;
        भङ्ग;
      }
    }
  }अन्यथा{
    rc = 0;
  }

  /* Free allocated structure */
bitvec_अन्तः:
  sqlite3_free(pTmpSpace);
  sqlite3_free(pV);
  sqlite3BitvecDestroy(pBitvec);
  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_UNTESTABLE */
