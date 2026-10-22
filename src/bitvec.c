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
#അന്തര്ഭാവ "sqliteInt.h"

/* Size of the Bitvec structure in bytes. */
#define BITVEC_SZ        512

/* Round the union size down to the nearest pointer boundary, since that's how
** it will be aligned within the Bitvec struct. */
#define BITVEC_USIZE \
    (((BITVEC_SZ-(3*വലുപ്പം(u32)))/വലുപ്പം(Bitvec*))*വലുപ്പം(Bitvec*))

/* Type of the array "element" for the bitmap representation.
** Should be a power of 2, and ideally, evenly divide into BITVEC_USIZE.
** Setting this to the "natural word" size of your CPU may improve
** performance. */
#define BITVEC_TELEM     u8
/* Size, in bits, of the bitmap element. */
#define BITVEC_SZELEM    8
/* Number of elements in a bitmap array. */
#define BITVEC_NELEM     (BITVEC_USIZE/വലുപ്പം(BITVEC_TELEM))
/* Number of bits in the bitmap array. */
#define BITVEC_NBIT      (BITVEC_NELEM*BITVEC_SZELEM)

/* Number of u32 values in hash table. */
#define BITVEC_NINT      (BITVEC_USIZE/വലുപ്പം(u32))
/* Maximum number of entries in hash table before
** sub-dividing and re-hashing. */
#define BITVEC_MXHASH    (BITVEC_NINT/2)
/* Hashing function for the aHash representation.
** Empirical testing showed that the *37 multiplier
** (an arbitrary prime)in the hash function provided
** no fewer collisions than the no-op *1. */
#define BITVEC_HASH(X)   (((X)*1)%BITVEC_NINT)

#define BITVEC_NPTR      ((u32)(BITVEC_USIZE/വലുപ്പം(Bitvec *)))


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
ഘടന Bitvec {
  u32 iSize;      /* Maximum bit index.  Max iSize is 4,294,967,296. */
  u32 nSet;       /* Number of bits that are set - only valid for aHash
                  ** element.  Max is BITVEC_NINT.  For BITVEC_SZ of 512,
                  ** this would be 125. */
  u32 i_ഹാരകം;   /* Number of bits handled by each apSub[] entry. */
                  /* Should >=0 for apSub element. */
                  /* Max iDivisor is max(u32) / BITVEC_NPTR + 1.  */
                  /* For a BITVEC_SZ of 512, this would be 34,359,739. */
  സംഘം {
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
  പ്രതിജ്ഞാ( വലുപ്പം(*p)==BITVEC_SZ );
  p = sqlite3MallocZero( വലുപ്പം(*p) );
  എങ്കിൽ( p ){
    p->iSize = iSize;
  }
  തിരികെ p;
}

/*
** Check to see if the i-th bit is set.  Return true or false.
** If p is NULL (if the bitmap has not been created) or if
** i is out of range, then return false.
*/
പൂർണ്ണസംഖ്യ sqlite3BitvecTestNotNull(Bitvec *p, u32 i){
  പ്രതിജ്ഞാ( p!=0 );
  i--;
  എങ്കിൽ( i>=p->iSize ) തിരികെ 0;
  ആകുമ്പോൾ( p->i_ഹാരകം ){
    u32 bin = i/p->i_ഹാരകം;
    i = i%p->i_ഹാരകം;
    p = p->u.apSub[bin];
    എങ്കിൽ (!p) {
      തിരികെ 0;
    }
  }
  എങ്കിൽ( p->iSize<=BITVEC_NBIT ){
    തിരികെ (p->u.aBitmap[i/BITVEC_SZELEM] & (1<<(i&(BITVEC_SZELEM-1))))!=0;
  } അല്ലെങ്കിൽ{
    u32 h = BITVEC_HASH(i++);
    ആകുമ്പോൾ( p->u.aHash[h] ){
      എങ്കിൽ( p->u.aHash[h]==i ) തിരികെ 1;
      h = (h+1) % BITVEC_NINT;
    }
    തിരികെ 0;
  }
}
പൂർണ്ണസംഖ്യ sqlite3BitvecTest(Bitvec *p, u32 i){
  തിരികെ p!=0 && sqlite3BitvecTestNotNull(p,i);
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
പൂർണ്ണസംഖ്യ sqlite3BitvecSet(Bitvec *p, u32 i){
  u32 h;
  എങ്കിൽ( p==0 ) തിരികെ SQLITE_OK;
  പ്രതിജ്ഞാ( i>0 );
  പ്രതിജ്ഞാ( i<=p->iSize );
  i--;
  ആകുമ്പോൾ((p->iSize > BITVEC_NBIT) && p->i_ഹാരകം) {
    u32 bin = i/p->i_ഹാരകം;
    i = i%p->i_ഹാരകം;
    എങ്കിൽ( p->u.apSub[bin]==0 ){
      p->u.apSub[bin] = sqlite3BitvecCreate( p->i_ഹാരകം );
      എങ്കിൽ( p->u.apSub[bin]==0 ) തിരികെ SQLITE_NOMEM_BKPT;
    }
    p = p->u.apSub[bin];
  }
  എങ്കിൽ( p->iSize<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] |= 1 << (i&(BITVEC_SZELEM-1));
    തിരികെ SQLITE_OK;
  }
  h = BITVEC_HASH(i++);
  /* if there wasn't a hash collision, and this doesn't */
  /* completely fill the hash, then just add it without */
  /* worrying about sub-dividing and re-hashing. */
  എങ്കിൽ( !p->u.aHash[h] ){
    എങ്കിൽ (p->nSet<(BITVEC_NINT-1)) {
      പോകുക bitvec_set_end;
    } അല്ലെങ്കിൽ {
      പോകുക bitvec_set_rehash;
    }
  }
  /* there was a collision, check to see if it's already */
  /* in hash, if not, try to find a spot for it */
  ചെയ്യുക {
    എങ്കിൽ( p->u.aHash[h]==i ) തിരികെ SQLITE_OK;
    h++;
    എങ്കിൽ( h>=BITVEC_NINT ) h = 0;
  } ആകുമ്പോൾ( p->u.aHash[h] );
  /* we didn't find it in the hash.  h points to the first */
  /* available free spot. check to see if this is going to */
  /* make our hash too "full".  */
bitvec_set_rehash:
  എങ്കിൽ( p->nSet>=BITVEC_MXHASH ){
    unsigned പൂർണ്ണസംഖ്യ j;
    പൂർണ്ണസംഖ്യ rc;
    u32 *ai_മൂല്യങ്ങൾ = sqlite3_അട്ടി_Alloc_Raw(0, വലുപ്പം(p->u.aHash));
    എങ്കിൽ( ai_മൂല്യങ്ങൾ==0 ){
      തിരികെ SQLITE_NOMEM_BKPT;
    }അല്ലെങ്കിൽ{
      memcpy(ai_മൂല്യങ്ങൾ, p->u.aHash, വലുപ്പം(p->u.aHash));
      memset(p->u.apSub, 0, വലുപ്പം(p->u.apSub));
      p->i_ഹാരകം = p->iSize/BITVEC_NPTR;
      എങ്കിൽ( (p->iSize%BITVEC_NPTR)!=0 ) p->i_ഹാരകം++;
      എങ്കിൽ( p->i_ഹാരകം<BITVEC_NBIT ) p->i_ഹാരകം = BITVEC_NBIT;
      rc = sqlite3BitvecSet(p, i);
      വേണ്ടി(j=0; j<BITVEC_NINT; j++){
        എങ്കിൽ( ai_മൂല്യങ്ങൾ[j] ) rc |= sqlite3BitvecSet(p, ai_മൂല്യങ്ങൾ[j]);
      }
      sqlite3_അട്ടി_Free(0, ai_മൂല്യങ്ങൾ);
      തിരികെ rc;
    }
  }
bitvec_set_end:
  p->nSet++;
  p->u.aHash[h] = i;
  തിരികെ SQLITE_OK;
}

/*
** Clear the i-th bit.
**
** pBuf must be a pointer to at least BITVEC_SZ bytes of temporary storage
** that BitvecClear can use to rebuilt its hash table.
*/
ശൂന്യം sqlite3BitvecClear(Bitvec *p, u32 i, ശൂന്യം *pBuf){
  എങ്കിൽ( p==0 ) തിരികെ;
  പ്രതിജ്ഞാ( i>0 );
  i--;
  ആകുമ്പോൾ( p->i_ഹാരകം ){
    u32 bin = i/p->i_ഹാരകം;
    i = i%p->i_ഹാരകം;
    p = p->u.apSub[bin];
    എങ്കിൽ (!p) {
      തിരികെ;
    }
  }
  എങ്കിൽ( p->iSize<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] &= ~(BITVEC_TELEM)(1<<(i&(BITVEC_SZELEM-1)));
  }അല്ലെങ്കിൽ{
    unsigned പൂർണ്ണസംഖ്യ j;
    u32 *ai_മൂല്യങ്ങൾ = pBuf;
    memcpy(ai_മൂല്യങ്ങൾ, p->u.aHash, വലുപ്പം(p->u.aHash));
    memset(p->u.aHash, 0, വലുപ്പം(p->u.aHash));
    p->nSet = 0;
    വേണ്ടി(j=0; j<BITVEC_NINT; j++){
      എങ്കിൽ( ai_മൂല്യങ്ങൾ[j] && ai_മൂല്യങ്ങൾ[j]!=(i+1) ){
        u32 h = BITVEC_HASH(ai_മൂല്യങ്ങൾ[j]-1);
        p->nSet++;
        ആകുമ്പോൾ( p->u.aHash[h] ){
          h++;
          എങ്കിൽ( h>=BITVEC_NINT ) h = 0;
        }
        p->u.aHash[h] = ai_മൂല്യങ്ങൾ[j];
      }
    }
  }
}

/*
** Destroy a bitmap object.  Reclaim all memory used.
*/
ശൂന്യം sqlite3BitvecDestroy(Bitvec *p){
  എങ്കിൽ( p==0 ) തിരികെ;
  എങ്കിൽ( p->i_ഹാരകം ){
    unsigned പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<BITVEC_NPTR; i++){
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
  തിരികെ p->iSize;
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
സ്ഥിതം ശൂന്യം showBitvec(Bitvec *p, പൂർണ്ണസംഖ്യ n, unsigned x){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( p==0 ){
    printf("NULL\n");
    തിരികെ;
  }
  printf("Bitvec 0x%p iSize=%u", p, p->iSize);
  എങ്കിൽ( p->iSize<=BITVEC_NBIT ){
    printf(" bitmap\n");
    printf("%*s   bits:", n, "");
    വേണ്ടി(i=1; i<=BITVEC_NBIT; i++){
      എങ്കിൽ( sqlite3BitvecTest(p,i) ) printf(" %u", x+(unsigned)i);
    }
    printf("\n");
  }അല്ലെങ്കിൽ എങ്കിൽ( p->i_ഹാരകം==0 ){
    printf(" hash with %u entries\n", p->nSet);
    printf("%*s   bits:", n, "");
    വേണ്ടി(i=0; i<BITVEC_NINT; i++){
      എങ്കിൽ( p->u.aHash[i] ) printf(" %u", x+(unsigned)p->u.aHash[i]);
    }
    printf("\n");
  }അല്ലെങ്കിൽ{
    printf(" sub-bitvec with iDivisor=%u\n", p->i_ഹാരകം);
    വേണ്ടി(i=0; i<BITVEC_NPTR; i++){
      എങ്കിൽ( p->u.apSub[i]==0 ) തുടരുക;
      printf("%*s   apSub[%d]=", n, "", i);
      showBitvec(p->u.apSub[i], n+4, i*p->i_ഹാരകം);
    }
  }
}
ശൂന്യം sqlite3ShowBitvec(Bitvec *p){
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
പൂർണ്ണസംഖ്യ sqlite3BitvecBuiltinTest(പൂർണ്ണസംഖ്യ sz, പൂർണ്ണസംഖ്യ *aOp){
  Bitvec *pBitvec = 0;
  unsigned അക്ഷരം *pV = 0;
  പൂർണ്ണസംഖ്യ rc = -1;
  പൂർണ്ണസംഖ്യ i, nx, pc, op;
  ശൂന്യം *pTmpSpace;

  /* Allocate the Bitvec to be tested and a linear array of
  ** bits to act as the reference */
  എങ്കിൽ( sz<=0 ){
    pBitvec = sqlite3BitvecCreate( 2*(unsigned)(-sz) );
    pV = 0;
  }അല്ലെങ്കിൽ{
    pBitvec = sqlite3BitvecCreate( sz );
    pV = sqlite3MallocZero( (7+(i64)sz)/8 + 1 );
  }
  pTmpSpace = sqlite3_malloc64(BITVEC_SZ);
  എങ്കിൽ( pBitvec==0 || pTmpSpace==0 || (pV==0 && sz>0) ) പോകുക bitvec_end;

  /* NULL pBitvec tests */
  sqlite3BitvecSet(0, 1);
  sqlite3BitvecClear(0, 1, pTmpSpace);

  /* Run the program */
  pc = i = 0;
  ആകുമ്പോൾ( (op = aOp[pc])!=0 ){
    എങ്കിൽ( op>=6 ){
#ifdef SQLITE_DEBUG
      എങ്കിൽ( op==6 ){
        sqlite3ShowBitvec(pBitvec);
      }അല്ലെങ്കിൽ എങ്കിൽ( op==7 ){
        printf("BITVEC_SZ     = %d (%d by sizeof)\n",
               BITVEC_SZ, (പൂർണ്ണസംഖ്യ)വലുപ്പം(Bitvec));
        printf("BITVEC_USIZE  = %d\n", (പൂർണ്ണസംഖ്യ)BITVEC_USIZE);
        printf("BITVEC_NELEM  = %d\n", (പൂർണ്ണസംഖ്യ)BITVEC_NELEM);
        printf("BITVEC_NBIT   = %d\n", (പൂർണ്ണസംഖ്യ)BITVEC_NBIT);
        printf("BITVEC_NINT   = %d\n", (പൂർണ്ണസംഖ്യ)BITVEC_NINT);
        printf("BITVEC_MXHASH = %d\n", (പൂർണ്ണസംഖ്യ)BITVEC_MXHASH);
        printf("BITVEC_NPTR   = %d\n", (പൂർണ്ണസംഖ്യ)BITVEC_NPTR);
      }
#endif
      pc++;
      തുടരുക;
    }
    തിരഞ്ഞെടുപ്പ്( op ){
      അവസ്ഥ 1:
      അവസ്ഥ 2:
      അവസ്ഥ 5: {
        nx = 4;
        i = aOp[pc+2] - 1;
        aOp[pc+2] += aOp[pc+3];
        നിർത്തുക;
      }
      അവസ്ഥ 3:
      അവസ്ഥ 4:
      സ്ഥിരസ്ഥിതി: {
        nx = 2;
        sqlite3_randomness(വലുപ്പം(i), &i);
        നിർത്തുക;
      }
    }
    എങ്കിൽ( (--aOp[pc+1]) > 0 ) nx = 0;
    pc += nx;
    i = (i & 0x7fffffff)%sz;
    എങ്കിൽ( (op & 1)!=0 ){
      എങ്കിൽ( pV ) SETBIT(pV, (i+1));
      എങ്കിൽ( op!=5 ){
        എങ്കിൽ( sqlite3BitvecSet(pBitvec, i+1) ) പോകുക bitvec_end;
      }
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( pV ) CLEARBIT(pV, (i+1));
      sqlite3BitvecClear(pBitvec, i+1, pTmpSpace);
    }
  }

  /* Test to make sure the linear array exactly matches the
  ** Bitvec object.  Start with the assumption that they do
  ** match (rc==0).  Change rc to non-zero if a discrepancy
  ** is found.
  */
  എങ്കിൽ( pV ){
    rc = sqlite3BitvecTest(0,0) + sqlite3BitvecTest(pBitvec, sz+1)
            + sqlite3BitvecTest(pBitvec, 0)
            + (sqlite3BitvecSize(pBitvec) - sz);
    വേണ്ടി(i=1; i<=sz; i++){
      എങ്കിൽ( (TESTBIT(pV,i))!=sqlite3BitvecTest(pBitvec,i) ){
        rc = i;
        നിർത്തുക;
      }
    }
  }അല്ലെങ്കിൽ{
    rc = 0;
  }

  /* Free allocated structure */
bitvec_end:
  sqlite3_free(pTmpSpace);
  sqlite3_free(pV);
  sqlite3BitvecDestroy(pBitvec);
  തിരികെ rc;
}
#endif /* SQLITE_UNTESTABLE */
