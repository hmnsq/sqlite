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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
ਖਾਲੀ sqlite3HashInit(Hash *pNew){
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew!=0 );
  pNew->ਪਹਿਲਾ = 0;
  pNew->ਗਿਣਤੀ_2 = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
ਖਾਲੀ sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  ਪ੍ਰਤਿਜ੍ਞਾ( pH!=0 );
  elem = pH->ਪਹਿਲਾ;
  pH->ਪਹਿਲਾ = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  ਜਦੋਂ( elem ){
    HashElem *ਅਗਲਾ_elem = elem->ਅਗਲਾ;
    sqlite3_free(elem);
    elem = ਅਗਲਾ_elem;
  }
  pH->ਗਿਣਤੀ_2 = 0;
}

/*
** The hashing function.
*/
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ strHash(ਅਚਲ ਅੱਖਰ *z){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h = 0;
  ਜਦੋਂ( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
    h += 0xbf & (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*(z++);
#ਨਹੀਂਤਾਂ
    h += 0xdf & (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*(z++);
#ਜੇ_ਅੰਤ
    h *= 0x9e3779b1;
  }
  ਵਾਪਸ h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ਸਥਿਰ ਖਾਲੀ ਪਾਓ_Element(
  Hash *pH,              /* The complete hash table */
  ਬਣਤਰ _ht *p_ਇੰਦਰਾਜ,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *p_ਸਿਰ;       /* First element already in pEntry */
  ਜੇ( p_ਇੰਦਰਾਜ ){
    p_ਸਿਰ = p_ਇੰਦਰਾਜ->ਗਿਣਤੀ_2 ? p_ਇੰਦਰਾਜ->chain : 0;
    p_ਇੰਦਰਾਜ->ਗਿਣਤੀ_2++;
    p_ਇੰਦਰਾਜ->chain = pNew;
  }ਨਹੀਂਤਾਂ{
    p_ਸਿਰ = 0;
  }
  ਜੇ( p_ਸਿਰ ){
    pNew->ਅਗਲਾ = p_ਸਿਰ;
    pNew->prev = p_ਸਿਰ->prev;
    ਜੇ( p_ਸਿਰ->prev ){ p_ਸਿਰ->prev->ਅਗਲਾ = pNew; }
    ਨਹੀਂਤਾਂ             { pH->ਪਹਿਲਾ = pNew; }
    p_ਸਿਰ->prev = pNew;
  }ਨਹੀਂਤਾਂ{
    pNew->ਅਗਲਾ = pH->ਪਹਿਲਾ;
    ਜੇ( pH->ਪਹਿਲਾ ){ pH->ਪਹਿਲਾ->prev = pNew; }
    pNew->prev = 0;
    pH->ਪਹਿਲਾ = pNew;
  }
}


/* Resize the hash table so that it contains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
ਸਥਿਰ ਪੂਰਨ rehash(Hash *pH, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ new_ਮਾਪ){
  ਬਣਤਰ _ht *new_ht;            /* The new hash table */
  HashElem *elem, *ਅਗਲਾ_elem;    /* For looping over existing elements */

#ਜੇ SQLITE_MALLOC_SOFT_ਹੱਦਬੰਦੀ>0
  ਜੇ( new_ਮਾਪ*ਆਕਾਰ(ਬਣਤਰ _ht)>SQLITE_MALLOC_SOFT_ਹੱਦਬੰਦੀ ){
    new_ਮਾਪ = SQLITE_MALLOC_SOFT_ਹੱਦਬੰਦੀ/ਆਕਾਰ(ਬਣਤਰ _ht);
  }
  ਜੇ( new_ਮਾਪ==pH->htsize ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (ਬਣਤਰ _ht *)sqlite3Malloc( new_ਮਾਪ*ਆਕਾਰ(ਬਣਤਰ _ht) );
  sqlite3_ਸਿਰਾ_Benign_Malloc();

  ਜੇ( new_ht==0 ) ਵਾਪਸ 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_ਮਾਪ = sqlite3_Malloc_ਮਾਪ(new_ht)/ਆਕਾਰ(ਬਣਤਰ _ht);
  memset(new_ht, 0, new_ਮਾਪ*ਆਕਾਰ(ਬਣਤਰ _ht));
  ਲਈ(elem=pH->ਪਹਿਲਾ, pH->ਪਹਿਲਾ=0; elem; elem = ਅਗਲਾ_elem){
    ਅਗਲਾ_elem = elem->ਅਗਲਾ;
    ਪਾਓ_Element(pH, &new_ht[elem->h % new_ਮਾਪ], elem);
  }
  ਵਾਪਸ 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ਸਥਿਰ HashElem *ਲੱਭੋ_Element_With_Hash(
  ਅਚਲ Hash *pH,     /* The pH to be searched */
  ਅਚਲ ਅੱਖਰ *p_ਕੁੰਜੀ,   /* The key we are searching for */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਗਿਣਤੀ_2;            /* Number of elements left to test */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h;                /* The computed hash */
  ਸਥਿਰ HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_ਕੁੰਜੀ);
  ਜੇ( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    ਬਣਤਰ _ht *p_ਇੰਦਰਾਜ;
    p_ਇੰਦਰਾਜ = &pH->ht[h % pH->htsize];
    elem = p_ਇੰਦਰਾਜ->chain;
    ਗਿਣਤੀ_2 = p_ਇੰਦਰਾਜ->ਗਿਣਤੀ_2;
  }ਨਹੀਂਤਾਂ{
    elem = pH->ਪਹਿਲਾ;
    ਗਿਣਤੀ_2 = pH->ਗਿਣਤੀ_2;
  }
  ਜੇ( pHash ) *pHash = h;
  ਜਦੋਂ( ਗਿਣਤੀ_2 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( elem!=0 );
    ਜੇ( h==elem->h && sqlite3StrICmp(elem->p_ਕੁੰਜੀ,p_ਕੁੰਜੀ)==0 ){ 
      ਵਾਪਸ elem;
    }
    elem = elem->ਅਗਲਾ;
    ਗਿਣਤੀ_2--;
  }
  ਵਾਪਸ &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
ਸਥਿਰ ਖਾਲੀ removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  ਬਣਤਰ _ht *p_ਇੰਦਰਾਜ;
  ਜੇ( elem->prev ){
    elem->prev->ਅਗਲਾ = elem->ਅਗਲਾ; 
  }ਨਹੀਂਤਾਂ{
    pH->ਪਹਿਲਾ = elem->ਅਗਲਾ;
  }
  ਜੇ( elem->ਅਗਲਾ ){
    elem->ਅਗਲਾ->prev = elem->prev;
  }
  ਜੇ( pH->ht ){
    p_ਇੰਦਰਾਜ = &pH->ht[elem->h % pH->htsize];
    ਜੇ( p_ਇੰਦਰਾਜ->chain==elem ){
      p_ਇੰਦਰਾਜ->chain = elem->ਅਗਲਾ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਇੰਦਰਾਜ->ਗਿਣਤੀ_2>0 );
    p_ਇੰਦਰਾਜ->ਗਿਣਤੀ_2--;
  }
  sqlite3_free( elem );
  pH->ਗਿਣਤੀ_2--;
  ਜੇ( pH->ਗਿਣਤੀ_2==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pH->ਪਹਿਲਾ==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pH->ਗਿਣਤੀ_2==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
ਖਾਲੀ *sqlite3_Hash_ਲੱਭੋ(ਅਚਲ Hash *pH, ਅਚਲ ਅੱਖਰ *p_ਕੁੰਜੀ){
  ਪ੍ਰਤਿਜ੍ਞਾ( pH!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ!=0 );
  ਵਾਪਸ ਲੱਭੋ_Element_With_Hash(pH, p_ਕੁੰਜੀ, 0)->ਅੰਕੜੇ;
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
ਖਾਲੀ *sqlite3_Hash_ਪਾਓ(Hash *pH, ਅਚਲ ਅੱਖਰ *p_ਕੁੰਜੀ, ਖਾਲੀ *ਅੰਕੜੇ){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  ਪ੍ਰਤਿਜ੍ਞਾ( pH!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ!=0 );
  elem = ਲੱਭੋ_Element_With_Hash(pH,p_ਕੁੰਜੀ,&h);
  ਜੇ( elem->ਅੰਕੜੇ ){
    ਖਾਲੀ *old_ਅੰਕੜੇ = elem->ਅੰਕੜੇ;
    ਜੇ( ਅੰਕੜੇ==0 ){
      removeElement(pH,elem);
    }ਨਹੀਂਤਾਂ{
      elem->ਅੰਕੜੇ = ਅੰਕੜੇ;
      elem->p_ਕੁੰਜੀ = p_ਕੁੰਜੀ;
    }
    ਵਾਪਸ old_ਅੰਕੜੇ;
  }
  ਜੇ( ਅੰਕੜੇ==0 ) ਵਾਪਸ 0;
  new_elem = (HashElem*)sqlite3Malloc( ਆਕਾਰ(HashElem) );
  ਜੇ( new_elem==0 ) ਵਾਪਸ ਅੰਕੜੇ;
  new_elem->p_ਕੁੰਜੀ = p_ਕੁੰਜੀ;
  new_elem->h = h;
  new_elem->ਅੰਕੜੇ = ਅੰਕੜੇ;
  pH->ਗਿਣਤੀ_2++;
  ਜੇ( pH->ਗਿਣਤੀ_2>=5 && pH->ਗਿਣਤੀ_2 > 2*pH->htsize ){
    rehash(pH, pH->ਗਿਣਤੀ_2*3);
  }
  ਪਾਓ_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  ਵਾਪਸ 0;
}

