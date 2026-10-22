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
#شامل "sqliteInt.h"
#شامل <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
تهی sqlite3HashInit(Hash *pNew){
  أكد( pNew!=0 );
  pNew->first = 0;
  pNew->شمار = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
تهی sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  أكد( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  وقتی( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->شمار = 0;
}

/*
** The hashing function.
*/
ایستا unsigned صحیح strHash(ثابت نویسه *z){
  unsigned صحیح h = 0;
  وقتی( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned نویسه)*(z++);
#وگرنه
    h += 0xdf & (unsigned نویسه)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  بازگشت h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ایستا تهی درج_Element(
  Hash *pH,              /* The complete hash table */
  ساختار _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  اگر( pEntry ){
    pHead = pEntry->شمار ? pEntry->chain : 0;
    pEntry->شمار++;
    pEntry->chain = pNew;
  }وگرنه{
    pHead = 0;
  }
  اگر( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    اگر( pHead->prev ){ pHead->prev->next = pNew; }
    وگرنه             { pH->first = pNew; }
    pHead->prev = pNew;
  }وگرنه{
    pNew->next = pH->first;
    اگر( pH->first ){ pH->first->prev = pNew; }
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
ایستا صحیح rehash(Hash *pH, unsigned صحیح new_size){
  ساختار _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#اگر SQLITE_MALLOC_SOFT_LIMIT>0
  اگر( new_size*اندازه(ساختار _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/اندازه(ساختار _ht);
  }
  اگر( new_size==pH->htsize ) بازگشت 0;
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
  new_ht = (ساختار _ht *)sqlite3Malloc( new_size*اندازه(ساختار _ht) );
  sqlite3EndBenignMalloc();

  اگر( new_ht==0 ) بازگشت 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/اندازه(ساختار _ht);
  memset(new_ht, 0, new_size*اندازه(ساختار _ht));
  برای(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    درج_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  بازگشت 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ایستا HashElem *findElementWithHash(
  ثابت Hash *pH,     /* The pH to be searched */
  ثابت نویسه *p_کلید,   /* The key we are searching for */
  unsigned صحیح *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned صحیح شمار;            /* Number of elements left to test */
  unsigned صحیح h;                /* The computed hash */
  ایستا HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_کلید);
  اگر( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    ساختار _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    شمار = pEntry->شمار;
  }وگرنه{
    elem = pH->first;
    شمار = pH->شمار;
  }
  اگر( pHash ) *pHash = h;
  وقتی( شمار ){
    أكد( elem!=0 );
    اگر( h==elem->h && sqlite3StrICmp(elem->p_کلید,p_کلید)==0 ){ 
      بازگشت elem;
    }
    elem = elem->next;
    شمار--;
  }
  بازگشت &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
ایستا تهی removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  ساختار _ht *pEntry;
  اگر( elem->prev ){
    elem->prev->next = elem->next; 
  }وگرنه{
    pH->first = elem->next;
  }
  اگر( elem->next ){
    elem->next->prev = elem->prev;
  }
  اگر( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    اگر( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    أكد( pEntry->شمار>0 );
    pEntry->شمار--;
  }
  sqlite3_free( elem );
  pH->شمار--;
  اگر( pH->شمار==0 ){
    أكد( pH->first==0 );
    أكد( pH->شمار==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
تهی *sqlite3HashFind(ثابت Hash *pH, ثابت نویسه *p_کلید){
  أكد( pH!=0 );
  أكد( p_کلید!=0 );
  بازگشت findElementWithHash(pH, p_کلید, 0)->data;
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
تهی *sqlite3_Hash_درج(Hash *pH, ثابت نویسه *p_کلید, تهی *data){
  unsigned صحیح h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  أكد( pH!=0 );
  أكد( p_کلید!=0 );
  elem = findElementWithHash(pH,p_کلید,&h);
  اگر( elem->data ){
    تهی *old_data = elem->data;
    اگر( data==0 ){
      removeElement(pH,elem);
    }وگرنه{
      elem->data = data;
      elem->p_کلید = p_کلید;
    }
    بازگشت old_data;
  }
  اگر( data==0 ) بازگشت 0;
  new_elem = (HashElem*)sqlite3Malloc( اندازه(HashElem) );
  اگر( new_elem==0 ) بازگشت data;
  new_elem->p_کلید = p_کلید;
  new_elem->h = h;
  new_elem->data = data;
  pH->شمار++;
  اگر( pH->شمار>=5 && pH->شمار > 2*pH->htsize ){
    rehash(pH, pH->شمار*3);
  }
  درج_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  بازگشت 0;
}

