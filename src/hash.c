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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
ଶୂନ୍ୟ sqlite3HashInit(Hash *pNew){
  ପ୍ରତିଜ୍ଞା( pNew!=0 );
  pNew->first = 0;
  pNew->ଗଣନା_2 = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
ଶୂନ୍ୟ sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  ପ୍ରତିଜ୍ଞା( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  ଯେତେବେଳେ( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->ଗଣନା_2 = 0;
}

/*
** The hashing function.
*/
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା strHash(ଅଚଳ ଅକ୍ଷର *z){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = 0;
  ଯେତେବେଳେ( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned ଅକ୍ଷର)*(z++);
#ଅନ୍ୟଥା
    h += 0xdf & (unsigned ଅକ୍ଷର)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  ଫେରନ୍ତୁ h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ସ୍ଥିର ଶୂନ୍ୟ ଭର୍ତ୍ତି_Element(
  Hash *pH,              /* The complete hash table */
  ସଂରଚନା _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  ଯଦି( pEntry ){
    pHead = pEntry->ଗଣନା_2 ? pEntry->chain : 0;
    pEntry->ଗଣନା_2++;
    pEntry->chain = pNew;
  }ଅନ୍ୟଥା{
    pHead = 0;
  }
  ଯଦି( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    ଯଦି( pHead->prev ){ pHead->prev->next = pNew; }
    ଅନ୍ୟଥା             { pH->first = pNew; }
    pHead->prev = pNew;
  }ଅନ୍ୟଥା{
    pNew->next = pH->first;
    ଯଦି( pH->first ){ pH->first->prev = pNew; }
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା rehash(Hash *pH, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_size){
  ସଂରଚନା _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#ଯଦି SQLITE_MALLOC_SOFT_LIMIT>0
  ଯଦି( new_size*ଆକାର(ସଂରଚନା _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/ଆକାର(ସଂରଚନା _ht);
  }
  ଯଦି( new_size==pH->htsize ) ଫେରନ୍ତୁ 0;
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
  new_ht = (ସଂରଚନା _ht *)sqlite3Malloc( new_size*ଆକାର(ସଂରଚନା _ht) );
  sqlite3EndBenignMalloc();

  ଯଦି( new_ht==0 ) ଫେରନ୍ତୁ 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/ଆକାର(ସଂରଚନା _ht);
  memset(new_ht, 0, new_size*ଆକାର(ସଂରଚନା _ht));
  ପାଇଁ(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    ଭର୍ତ୍ତି_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  ଫେରନ୍ତୁ 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ସ୍ଥିର HashElem *findElementWithHash(
  ଅଚଳ Hash *pH,     /* The pH to be searched */
  ଅଚଳ ଅକ୍ଷର *p_ଚାବି,   /* The key we are searching for */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;            /* Number of elements left to test */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା h;                /* The computed hash */
  ସ୍ଥିର HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_ଚାବି);
  ଯଦି( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    ସଂରଚନା _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    ଗଣନା_2 = pEntry->ଗଣନା_2;
  }ଅନ୍ୟଥା{
    elem = pH->first;
    ଗଣନା_2 = pH->ଗଣନା_2;
  }
  ଯଦି( pHash ) *pHash = h;
  ଯେତେବେଳେ( ଗଣନା_2 ){
    ପ୍ରତିଜ୍ଞା( elem!=0 );
    ଯଦି( h==elem->h && sqlite3StrICmp(elem->p_ଚାବି,p_ଚାବି)==0 ){ 
      ଫେରନ୍ତୁ elem;
    }
    elem = elem->next;
    ଗଣନା_2--;
  }
  ଫେରନ୍ତୁ &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
ସ୍ଥିର ଶୂନ୍ୟ removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  ସଂରଚନା _ht *pEntry;
  ଯଦି( elem->prev ){
    elem->prev->next = elem->next; 
  }ଅନ୍ୟଥା{
    pH->first = elem->next;
  }
  ଯଦି( elem->next ){
    elem->next->prev = elem->prev;
  }
  ଯଦି( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    ଯଦି( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    ପ୍ରତିଜ୍ଞା( pEntry->ଗଣନା_2>0 );
    pEntry->ଗଣନା_2--;
  }
  sqlite3_free( elem );
  pH->ଗଣନା_2--;
  ଯଦି( pH->ଗଣନା_2==0 ){
    ପ୍ରତିଜ୍ଞା( pH->first==0 );
    ପ୍ରତିଜ୍ଞା( pH->ଗଣନା_2==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
ଶୂନ୍ୟ *sqlite3HashFind(ଅଚଳ Hash *pH, ଅଚଳ ଅକ୍ଷର *p_ଚାବି){
  ପ୍ରତିଜ୍ଞା( pH!=0 );
  ପ୍ରତିଜ୍ଞା( p_ଚାବି!=0 );
  ଫେରନ୍ତୁ findElementWithHash(pH, p_ଚାବି, 0)->data;
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
ଶୂନ୍ୟ *sqlite3_Hash_ଭର୍ତ୍ତି(Hash *pH, ଅଚଳ ଅକ୍ଷର *p_ଚାବି, ଶୂନ୍ୟ *data){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  ପ୍ରତିଜ୍ଞା( pH!=0 );
  ପ୍ରତିଜ୍ଞା( p_ଚାବି!=0 );
  elem = findElementWithHash(pH,p_ଚାବି,&h);
  ଯଦି( elem->data ){
    ଶୂନ୍ୟ *old_data = elem->data;
    ଯଦି( data==0 ){
      removeElement(pH,elem);
    }ଅନ୍ୟଥା{
      elem->data = data;
      elem->p_ଚାବି = p_ଚାବି;
    }
    ଫେରନ୍ତୁ old_data;
  }
  ଯଦି( data==0 ) ଫେରନ୍ତୁ 0;
  new_elem = (HashElem*)sqlite3Malloc( ଆକାର(HashElem) );
  ଯଦି( new_elem==0 ) ଫେରନ୍ତୁ data;
  new_elem->p_ଚାବି = p_ଚାବି;
  new_elem->h = h;
  new_elem->data = data;
  pH->ଗଣନା_2++;
  ଯଦି( pH->ଗଣନା_2>=5 && pH->ଗଣନା_2 > 2*pH->htsize ){
    rehash(pH, pH->ଗଣନା_2*3);
  }
  ଭର୍ତ୍ତି_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  ଫେରନ୍ତୁ 0;
}

