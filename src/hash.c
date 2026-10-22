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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
வெறுமை sqlite3HashInit(Hash *pNew){
  ப்ரதிஜ்ஞா( pNew!=0 );
  pNew->first = 0;
  pNew->மொத்தம் = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
வெறுமை sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  ப்ரதிஜ்ஞா( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  போது( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->மொத்தம் = 0;
}

/*
** The hashing function.
*/
நிலையான unsigned முழு strHash(மாறிலி எழுத்து *z){
  unsigned முழு h = 0;
  போது( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned எழுத்து)*(z++);
#இல்லையெனில்
    h += 0xdf & (unsigned எழுத்து)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  திரும்பு h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
நிலையான வெறுமை செருகு_Element(
  Hash *pH,              /* The complete hash table */
  கட்டமைப்பு _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  எனில்( pEntry ){
    pHead = pEntry->மொத்தம் ? pEntry->chain : 0;
    pEntry->மொத்தம்++;
    pEntry->chain = pNew;
  }இல்லையெனில்{
    pHead = 0;
  }
  எனில்( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    எனில்( pHead->prev ){ pHead->prev->next = pNew; }
    இல்லையெனில்             { pH->first = pNew; }
    pHead->prev = pNew;
  }இல்லையெனில்{
    pNew->next = pH->first;
    எனில்( pH->first ){ pH->first->prev = pNew; }
    pNew->prev = 0;
    pH->first = pNew;
  }
}


/* Resize the hash table so that it contains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
நிலையான முழு rehash(Hash *pH, unsigned முழு new_size){
  கட்டமைப்பு _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#எனில் SQLITE_MALLOC_SOFT_LIMIT>0
  எனில்( new_size*அளவு(கட்டமைப்பு _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/அளவு(கட்டமைப்பு _ht);
  }
  எனில்( new_size==pH->htsize ) திரும்பு 0;
#endif

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (கட்டமைப்பு _ht *)sqlite3Malloc( new_size*அளவு(கட்டமைப்பு _ht) );
  sqlite3EndBenignMalloc();

  எனில்( new_ht==0 ) திரும்பு 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/அளவு(கட்டமைப்பு _ht);
  memset(new_ht, 0, new_size*அளவு(கட்டமைப்பு _ht));
  க்கு(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    செருகு_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  திரும்பு 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
நிலையான HashElem *findElementWithHash(
  மாறிலி Hash *pH,     /* The pH to be searched */
  மாறிலி எழுத்து *p_சாவி,   /* The key we are searching for */
  unsigned முழு *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned முழு மொத்தம்;            /* Number of elements left to test */
  unsigned முழு h;                /* The computed hash */
  நிலையான HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_சாவி);
  எனில்( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    கட்டமைப்பு _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    மொத்தம் = pEntry->மொத்தம்;
  }இல்லையெனில்{
    elem = pH->first;
    மொத்தம் = pH->மொத்தம்;
  }
  எனில்( pHash ) *pHash = h;
  போது( மொத்தம் ){
    ப்ரதிஜ்ஞா( elem!=0 );
    எனில்( h==elem->h && sqlite3StrICmp(elem->p_சாவி,p_சாவி)==0 ){ 
      திரும்பு elem;
    }
    elem = elem->next;
    மொத்தம்--;
  }
  திரும்பு &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
நிலையான வெறுமை removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  கட்டமைப்பு _ht *pEntry;
  எனில்( elem->prev ){
    elem->prev->next = elem->next; 
  }இல்லையெனில்{
    pH->first = elem->next;
  }
  எனில்( elem->next ){
    elem->next->prev = elem->prev;
  }
  எனில்( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    எனில்( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    ப்ரதிஜ்ஞா( pEntry->மொத்தம்>0 );
    pEntry->மொத்தம்--;
  }
  sqlite3_free( elem );
  pH->மொத்தம்--;
  எனில்( pH->மொத்தம்==0 ){
    ப்ரதிஜ்ஞா( pH->first==0 );
    ப்ரதிஜ்ஞா( pH->மொத்தம்==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
வெறுமை *sqlite3HashFind(மாறிலி Hash *pH, மாறிலி எழுத்து *p_சாவி){
  ப்ரதிஜ்ஞா( pH!=0 );
  ப்ரதிஜ்ஞா( p_சாவி!=0 );
  திரும்பு findElementWithHash(pH, p_சாவி, 0)->data;
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
வெறுமை *sqlite3_Hash_செருகு(Hash *pH, மாறிலி எழுத்து *p_சாவி, வெறுமை *data){
  unsigned முழு h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  ப்ரதிஜ்ஞா( pH!=0 );
  ப்ரதிஜ்ஞா( p_சாவி!=0 );
  elem = findElementWithHash(pH,p_சாவி,&h);
  எனில்( elem->data ){
    வெறுமை *old_data = elem->data;
    எனில்( data==0 ){
      removeElement(pH,elem);
    }இல்லையெனில்{
      elem->data = data;
      elem->p_சாவி = p_சாவி;
    }
    திரும்பு old_data;
  }
  எனில்( data==0 ) திரும்பு 0;
  new_elem = (HashElem*)sqlite3Malloc( அளவு(HashElem) );
  எனில்( new_elem==0 ) திரும்பு data;
  new_elem->p_சாவி = p_சாவி;
  new_elem->h = h;
  new_elem->data = data;
  pH->மொத்தம்++;
  எனில்( pH->மொத்தம்>=5 && pH->மொத்தம் > 2*pH->htsize ){
    rehash(pH, pH->மொத்தம்*3);
  }
  செருகு_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  திரும்பு 0;
}

