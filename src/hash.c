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
#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
শূন্য sqlite3HashInit(Hash *pNew){
  প্রতিজ্ঞা( pNew!=0 );
  pNew->first = 0;
  pNew->গুনতি = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
শূন্য sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  প্রতিজ্ঞা( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  যেতিয়ালৈ( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->গুনতি = 0;
}

/*
** The hashing function.
*/
স্থির unsigned পূর্ণাঙ্ক strHash(ধ্রুৱ ৱর্ণ *z){
  unsigned পূর্ণাঙ্ক h = 0;
  যেতিয়ালৈ( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned ৱর্ণ)*(z++);
#অন্যথা
    h += 0xdf & (unsigned ৱর্ণ)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  ঘূৰাই h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
স্থির শূন্য ঢোকাও_Element(
  Hash *pH,              /* The complete hash table */
  সংরচনা _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  যদি( pEntry ){
    pHead = pEntry->গুনতি ? pEntry->chain : 0;
    pEntry->গুনতি++;
    pEntry->chain = pNew;
  }অন্যথা{
    pHead = 0;
  }
  যদি( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    যদি( pHead->prev ){ pHead->prev->next = pNew; }
    অন্যথা             { pH->first = pNew; }
    pHead->prev = pNew;
  }অন্যথা{
    pNew->next = pH->first;
    যদি( pH->first ){ pH->first->prev = pNew; }
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
স্থির পূর্ণাঙ্ক rehash(Hash *pH, unsigned পূর্ণাঙ্ক new_size){
  সংরচনা _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#যদি SQLITE_MALLOC_SOFT_LIMIT>0
  যদি( new_size*পরিমাণ(সংরচনা _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/পরিমাণ(সংরচনা _ht);
  }
  যদি( new_size==pH->htsize ) ঘূৰাই 0;
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
  new_ht = (সংরচনা _ht *)sqlite3Malloc( new_size*পরিমাণ(সংরচনা _ht) );
  sqlite3EndBenignMalloc();

  যদি( new_ht==0 ) ঘূৰাই 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/পরিমাণ(সংরচনা _ht);
  memset(new_ht, 0, new_size*পরিমাণ(সংরচনা _ht));
  বাবে(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    ঢোকাও_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  ঘূৰাই 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
স্থির HashElem *findElementWithHash(
  ধ্রুৱ Hash *pH,     /* The pH to be searched */
  ধ্রুৱ ৱর্ণ *p_চাবি,   /* The key we are searching for */
  unsigned পূর্ণাঙ্ক *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned পূর্ণাঙ্ক গুনতি;            /* Number of elements left to test */
  unsigned পূর্ণাঙ্ক h;                /* The computed hash */
  স্থির HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_চাবি);
  যদি( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    সংরচনা _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    গুনতি = pEntry->গুনতি;
  }অন্যথা{
    elem = pH->first;
    গুনতি = pH->গুনতি;
  }
  যদি( pHash ) *pHash = h;
  যেতিয়ালৈ( গুনতি ){
    প্রতিজ্ঞা( elem!=0 );
    যদি( h==elem->h && sqlite3StrICmp(elem->p_চাবি,p_চাবি)==0 ){ 
      ঘূৰাই elem;
    }
    elem = elem->next;
    গুনতি--;
  }
  ঘূৰাই &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
স্থির শূন্য removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  সংরচনা _ht *pEntry;
  যদি( elem->prev ){
    elem->prev->next = elem->next; 
  }অন্যথা{
    pH->first = elem->next;
  }
  যদি( elem->next ){
    elem->next->prev = elem->prev;
  }
  যদি( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    যদি( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    প্রতিজ্ঞা( pEntry->গুনতি>0 );
    pEntry->গুনতি--;
  }
  sqlite3_free( elem );
  pH->গুনতি--;
  যদি( pH->গুনতি==0 ){
    প্রতিজ্ঞা( pH->first==0 );
    প্রতিজ্ঞা( pH->গুনতি==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
শূন্য *sqlite3HashFind(ধ্রুৱ Hash *pH, ধ্রুৱ ৱর্ণ *p_চাবি){
  প্রতিজ্ঞা( pH!=0 );
  প্রতিজ্ঞা( p_চাবি!=0 );
  ঘূৰাই findElementWithHash(pH, p_চাবি, 0)->data;
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
শূন্য *sqlite3_Hash_ঢোকাও(Hash *pH, ধ্রুৱ ৱর্ণ *p_চাবি, শূন্য *data){
  unsigned পূর্ণাঙ্ক h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  প্রতিজ্ঞা( pH!=0 );
  প্রতিজ্ঞা( p_চাবি!=0 );
  elem = findElementWithHash(pH,p_চাবি,&h);
  যদি( elem->data ){
    শূন্য *old_data = elem->data;
    যদি( data==0 ){
      removeElement(pH,elem);
    }অন্যথা{
      elem->data = data;
      elem->p_চাবি = p_চাবি;
    }
    ঘূৰাই old_data;
  }
  যদি( data==0 ) ঘূৰাই 0;
  new_elem = (HashElem*)sqlite3Malloc( পরিমাণ(HashElem) );
  যদি( new_elem==0 ) ঘূৰাই data;
  new_elem->p_চাবি = p_চাবি;
  new_elem->h = h;
  new_elem->data = data;
  pH->গুনতি++;
  যদি( pH->গুনতি>=5 && pH->গুনতি > 2*pH->htsize ){
    rehash(pH, pH->গুনতি*3);
  }
  ঢোকাও_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  ঘূৰাই 0;
}

