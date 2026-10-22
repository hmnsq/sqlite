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
#ضمن "sqliteInt.h"
#ضمن <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
فراغ sqlite3HashInit(Hash *pNew){
  أكد( pNew!=0 );
  pNew->first = 0;
  pNew->كدي = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
فراغ sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  أكد( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  طالما( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->كدي = 0;
}

/*
** The hashing function.
*/
ساكن unsigned صحيح strHash(ثابت حرف *z){
  unsigned صحيح h = 0;
  طالما( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned حرف)*(z++);
#وإلا
    h += 0xdf & (unsigned حرف)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  أرجع h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ساكن فراغ سك_Element(
  Hash *pH,              /* The complete hash table */
  بنية _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  إذا( pEntry ){
    pHead = pEntry->كدي ? pEntry->chain : 0;
    pEntry->كدي++;
    pEntry->chain = pNew;
  }وإلا{
    pHead = 0;
  }
  إذا( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    إذا( pHead->prev ){ pHead->prev->next = pNew; }
    وإلا             { pH->first = pNew; }
    pHead->prev = pNew;
  }وإلا{
    pNew->next = pH->first;
    إذا( pH->first ){ pH->first->prev = pNew; }
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
ساكن صحيح rehash(Hash *pH, unsigned صحيح new_size){
  بنية _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#إذا SQLITE_MALLOC_SOFT_LIMIT>0
  إذا( new_size*حجم(بنية _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/حجم(بنية _ht);
  }
  إذا( new_size==pH->htsize ) أرجع 0;
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
  new_ht = (بنية _ht *)sqlite3Malloc( new_size*حجم(بنية _ht) );
  sqlite3EndBenignMalloc();

  إذا( new_ht==0 ) أرجع 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/حجم(بنية _ht);
  memset(new_ht, 0, new_size*حجم(بنية _ht));
  لكل(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    سك_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  أرجع 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ساكن HashElem *findElementWithHash(
  ثابت Hash *pH,     /* The pH to be searched */
  ثابت حرف *p_مبودي,   /* The key we are searching for */
  unsigned صحيح *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned صحيح كدي;            /* Number of elements left to test */
  unsigned صحيح h;                /* The computed hash */
  ساكن HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_مبودي);
  إذا( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    بنية _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    كدي = pEntry->كدي;
  }وإلا{
    elem = pH->first;
    كدي = pH->كدي;
  }
  إذا( pHash ) *pHash = h;
  طالما( كدي ){
    أكد( elem!=0 );
    إذا( h==elem->h && sqlite3StrICmp(elem->p_مبودي,p_مبودي)==0 ){ 
      أرجع elem;
    }
    elem = elem->next;
    كدي--;
  }
  أرجع &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
ساكن فراغ removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  بنية _ht *pEntry;
  إذا( elem->prev ){
    elem->prev->next = elem->next; 
  }وإلا{
    pH->first = elem->next;
  }
  إذا( elem->next ){
    elem->next->prev = elem->prev;
  }
  إذا( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    إذا( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    أكد( pEntry->كدي>0 );
    pEntry->كدي--;
  }
  sqlite3_free( elem );
  pH->كدي--;
  إذا( pH->كدي==0 ){
    أكد( pH->first==0 );
    أكد( pH->كدي==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
فراغ *sqlite3HashFind(ثابت Hash *pH, ثابت حرف *p_مبودي){
  أكد( pH!=0 );
  أكد( p_مبودي!=0 );
  أرجع findElementWithHash(pH, p_مبودي, 0)->data;
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
فراغ *sqlite3_Hash_سك(Hash *pH, ثابت حرف *p_مبودي, فراغ *data){
  unsigned صحيح h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  أكد( pH!=0 );
  أكد( p_مبودي!=0 );
  elem = findElementWithHash(pH,p_مبودي,&h);
  إذا( elem->data ){
    فراغ *old_data = elem->data;
    إذا( data==0 ){
      removeElement(pH,elem);
    }وإلا{
      elem->data = data;
      elem->p_مبودي = p_مبودي;
    }
    أرجع old_data;
  }
  إذا( data==0 ) أرجع 0;
  new_elem = (HashElem*)sqlite3Malloc( حجم(HashElem) );
  إذا( new_elem==0 ) أرجع data;
  new_elem->p_مبودي = p_مبودي;
  new_elem->h = h;
  new_elem->data = data;
  pH->كدي++;
  إذا( pH->كدي>=5 && pH->كدي > 2*pH->htsize ){
    rehash(pH, pH->كدي*3);
  }
  سك_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  أرجع 0;
}

