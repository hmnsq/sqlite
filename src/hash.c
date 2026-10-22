/*
** 2001 September 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This is the implementation of generic hash-tables
** used in SQLite.
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
ಶೂನ್ಯ sqlite3HashInit(Hash *pNew){
  ಪ್ರತಿಜ್ಞಾ( pNew!=0 );
  pNew->ಮೊದಲ = 0;
  pNew->ಎಣಿಕೆ_2 = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
ಶೂನ್ಯ sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  ಪ್ರತಿಜ್ಞಾ( pH!=0 );
  elem = pH->ಮೊದಲ;
  pH->ಮೊದಲ = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  ಆಗ( elem ){
    HashElem *ಮುಂದಿನ_elem = elem->ಮುಂದಿನ;
    sqlite3_free(elem);
    elem = ಮುಂದಿನ_elem;
  }
  pH->ಎಣಿಕೆ_2 = 0;
}

/*
** The hashing function.
*/
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ strHash(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h = 0;
  ಆಗ( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
    h += 0xbf & (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*(z++);
#ಇಲ್ಲದಿದ್ದರೆ
    h += 0xdf & (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*(z++);
#ಆದರೆ_ಅಂತ್ಯ
    h *= 0x9e3779b1;
  }
  ಹಿಂತಿರುಗಿ h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_Element(
  Hash *pH,              /* The complete hash table */
  ರಚನೆ _ht *p_ನಮೂದು,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *p_ತಲೆ;       /* First element already in pEntry */
  ಆದರೆ( p_ನಮೂದು ){
    p_ತಲೆ = p_ನಮೂದು->ಎಣಿಕೆ_2 ? p_ನಮೂದು->chain : 0;
    p_ನಮೂದು->ಎಣಿಕೆ_2++;
    p_ನಮೂದು->chain = pNew;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ತಲೆ = 0;
  }
  ಆದರೆ( p_ತಲೆ ){
    pNew->ಮುಂದಿನ = p_ತಲೆ;
    pNew->prev = p_ತಲೆ->prev;
    ಆದರೆ( p_ತಲೆ->prev ){ p_ತಲೆ->prev->ಮುಂದಿನ = pNew; }
    ಇಲ್ಲದಿದ್ದರೆ             { pH->ಮೊದಲ = pNew; }
    p_ತಲೆ->prev = pNew;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pNew->ಮುಂದಿನ = pH->ಮೊದಲ;
    ಆದರೆ( pH->ಮೊದಲ ){ pH->ಮೊದಲ->prev = pNew; }
    pNew->prev = 0;
    pH->ಮೊದಲ = pNew;
  }
}


/* Resize the hash table so that it contains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rehash(Hash *pH, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ new_ಗಾತ್ರ){
  ರಚನೆ _ht *new_ht;            /* The new hash table */
  HashElem *elem, *ಮುಂದಿನ_elem;    /* For looping over existing elements */

#ಆದರೆ SQLITE_MALLOC_SOFT_ಎಲ್ಲೆ>0
  ಆದರೆ( new_ಗಾತ್ರ*ಗಾತ್ರ(ರಚನೆ _ht)>SQLITE_MALLOC_SOFT_ಎಲ್ಲೆ ){
    new_ಗಾತ್ರ = SQLITE_MALLOC_SOFT_ಎಲ್ಲೆ/ಗಾತ್ರ(ರಚನೆ _ht);
  }
  ಆದರೆ( new_ಗಾತ್ರ==pH->htsize ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (ರಚನೆ _ht *)sqlite3Malloc( new_ಗಾತ್ರ*ಗಾತ್ರ(ರಚನೆ _ht) );
  sqlite3_ಕೊನೆ_Benign_Malloc();

  ಆದರೆ( new_ht==0 ) ಹಿಂತಿರುಗಿ 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_ಗಾತ್ರ = sqlite3_Malloc_ಗಾತ್ರ(new_ht)/ಗಾತ್ರ(ರಚನೆ _ht);
  memset(new_ht, 0, new_ಗಾತ್ರ*ಗಾತ್ರ(ರಚನೆ _ht));
  ಗಾಗಿ(elem=pH->ಮೊದಲ, pH->ಮೊದಲ=0; elem; elem = ಮುಂದಿನ_elem){
    ಮುಂದಿನ_elem = elem->ಮುಂದಿನ;
    ಸೇರಿಸು_Element(pH, &new_ht[elem->h % new_ಗಾತ್ರ], elem);
  }
  ಹಿಂತಿರುಗಿ 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ಸ್ಥಿರ HashElem *ಕಂಡುಹಿಡಿ_Element_With_Hash(
  ಸ್ಥಿರಾಂಕ Hash *pH,     /* The pH to be searched */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_ಕೀಲಿ,   /* The key we are searching for */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;            /* Number of elements left to test */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h;                /* The computed hash */
  ಸ್ಥಿರ HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_ಕೀಲಿ);
  ಆದರೆ( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    ರಚನೆ _ht *p_ನಮೂದು;
    p_ನಮೂದು = &pH->ht[h % pH->htsize];
    elem = p_ನಮೂದು->chain;
    ಎಣಿಕೆ_2 = p_ನಮೂದು->ಎಣಿಕೆ_2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    elem = pH->ಮೊದಲ;
    ಎಣಿಕೆ_2 = pH->ಎಣಿಕೆ_2;
  }
  ಆದರೆ( pHash ) *pHash = h;
  ಆಗ( ಎಣಿಕೆ_2 ){
    ಪ್ರತಿಜ್ಞಾ( elem!=0 );
    ಆದರೆ( h==elem->h && sqlite3StrICmp(elem->p_ಕೀಲಿ,p_ಕೀಲಿ)==0 ){ 
      ಹಿಂತಿರುಗಿ elem;
    }
    elem = elem->ಮುಂದಿನ;
    ಎಣಿಕೆ_2--;
  }
  ಹಿಂತಿರುಗಿ &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
ಸ್ಥಿರ ಶೂನ್ಯ removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  ರಚನೆ _ht *p_ನಮೂದು;
  ಆದರೆ( elem->prev ){
    elem->prev->ಮುಂದಿನ = elem->ಮುಂದಿನ; 
  }ಇಲ್ಲದಿದ್ದರೆ{
    pH->ಮೊದಲ = elem->ಮುಂದಿನ;
  }
  ಆದರೆ( elem->ಮುಂದಿನ ){
    elem->ಮುಂದಿನ->prev = elem->prev;
  }
  ಆದರೆ( pH->ht ){
    p_ನಮೂದು = &pH->ht[elem->h % pH->htsize];
    ಆದರೆ( p_ನಮೂದು->chain==elem ){
      p_ನಮೂದು->chain = elem->ಮುಂದಿನ;
    }
    ಪ್ರತಿಜ್ಞಾ( p_ನಮೂದು->ಎಣಿಕೆ_2>0 );
    p_ನಮೂದು->ಎಣಿಕೆ_2--;
  }
  sqlite3_free( elem );
  pH->ಎಣಿಕೆ_2--;
  ಆದರೆ( pH->ಎಣಿಕೆ_2==0 ){
    ಪ್ರತಿಜ್ಞಾ( pH->ಮೊದಲ==0 );
    ಪ್ರತಿಜ್ಞಾ( pH->ಎಣಿಕೆ_2==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
ಶೂನ್ಯ *sqlite3_Hash_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ Hash *pH, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_ಕೀಲಿ){
  ಪ್ರತಿಜ್ಞಾ( pH!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ!=0 );
  ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_Element_With_Hash(pH, p_ಕೀಲಿ, 0)->ದತ್ತಾಂಶ;
}

/* Insert an element into the hash table pH.  The key is pKey
** and the data is "data".
**
** If no element exists with a matching key, then a new
** element is created and NULL is returned.
**
** If another element already exists with the same key, then the
** new data replaces the old data and the old data is returned.
** The key is not copied in this instance.  If a malloc fails, then
** the new data is returned and the hash table is unchanged.
**
** If the "data" parameter to this function is NULL, then the
** element corresponding to "key" is removed from the hash table.
*/
ಶೂನ್ಯ *sqlite3_Hash_ಸೇರಿಸು(Hash *pH, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_ಕೀಲಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  ಪ್ರತಿಜ್ಞಾ( pH!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ!=0 );
  elem = ಕಂಡುಹಿಡಿ_Element_With_Hash(pH,p_ಕೀಲಿ,&h);
  ಆದರೆ( elem->ದತ್ತಾಂಶ ){
    ಶೂನ್ಯ *old_ದತ್ತಾಂಶ = elem->ದತ್ತಾಂಶ;
    ಆದರೆ( ದತ್ತಾಂಶ==0 ){
      removeElement(pH,elem);
    }ಇಲ್ಲದಿದ್ದರೆ{
      elem->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
      elem->p_ಕೀಲಿ = p_ಕೀಲಿ;
    }
    ಹಿಂತಿರುಗಿ old_ದತ್ತಾಂಶ;
  }
  ಆದರೆ( ದತ್ತಾಂಶ==0 ) ಹಿಂತಿರುಗಿ 0;
  new_elem = (HashElem*)sqlite3Malloc( ಗಾತ್ರ(HashElem) );
  ಆದರೆ( new_elem==0 ) ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
  new_elem->p_ಕೀಲಿ = p_ಕೀಲಿ;
  new_elem->h = h;
  new_elem->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
  pH->ಎಣಿಕೆ_2++;
  ಆದರೆ( pH->ಎಣಿಕೆ_2>=5 && pH->ಎಣಿಕೆ_2 > 2*pH->htsize ){
    rehash(pH, pH->ಎಣಿಕೆ_2*3);
  }
  ಸೇರಿಸು_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  ಹಿಂತಿರುಗಿ 0;
}

