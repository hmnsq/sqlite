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
  pNew->first = 0;
  pNew->ಎಣಿಕೆ = 0;
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
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  ಯಾವತ್( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->ಎಣಿಕೆ = 0;
}

/*
** The hashing function.
*/
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ strHash(ಧ್ರುವ ವರ್ಣ *z){
  unsigned ಪೂರ್ಣಾಙ್ಕ h = 0;
  ಯಾವತ್( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned ವರ್ಣ)*(z++);
#ಅನ್ಯಥಾ
    h += 0xdf & (unsigned ವರ್ಣ)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  ಪ್ರತ್ಯಾಗಮ h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_Element(
  Hash *pH,              /* The complete hash table */
  ಸಂರಚನಾ _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  ಯದಿ( pEntry ){
    pHead = pEntry->ಎಣಿಕೆ ? pEntry->chain : 0;
    pEntry->ಎಣಿಕೆ++;
    pEntry->chain = pNew;
  }ಅನ್ಯಥಾ{
    pHead = 0;
  }
  ಯದಿ( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    ಯದಿ( pHead->prev ){ pHead->prev->next = pNew; }
    ಅನ್ಯಥಾ             { pH->first = pNew; }
    pHead->prev = pNew;
  }ಅನ್ಯಥಾ{
    pNew->next = pH->first;
    ಯದಿ( pH->first ){ pH->first->prev = pNew; }
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ rehash(Hash *pH, unsigned ಪೂರ್ಣಾಙ್ಕ new_size){
  ಸಂರಚನಾ _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#ಯದಿ SQLITE_MALLOC_SOFT_LIMIT>0
  ಯದಿ( new_size*ಪರಿಮಾಣ(ಸಂರಚನಾ _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/ಪರಿಮಾಣ(ಸಂರಚನಾ _ht);
  }
  ಯದಿ( new_size==pH->htsize ) ಪ್ರತ್ಯಾಗಮ 0;
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
  new_ht = (ಸಂರಚನಾ _ht *)sqlite3Malloc( new_size*ಪರಿಮಾಣ(ಸಂರಚನಾ _ht) );
  sqlite3EndBenignMalloc();

  ಯದಿ( new_ht==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/ಪರಿಮಾಣ(ಸಂರಚನಾ _ht);
  memset(new_ht, 0, new_size*ಪರಿಮಾಣ(ಸಂರಚನಾ _ht));
  ಪ್ರತಿ(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    ಸೇರಿಸು_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ಸ್ಥಿರ HashElem *findElementWithHash(
  ಧ್ರುವ Hash *pH,     /* The pH to be searched */
  ಧ್ರುವ ವರ್ಣ *p_ಕೀಲಿ,   /* The key we are searching for */
  unsigned ಪೂರ್ಣಾಙ್ಕ *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;            /* Number of elements left to test */
  unsigned ಪೂರ್ಣಾಙ್ಕ h;                /* The computed hash */
  ಸ್ಥಿರ HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_ಕೀಲಿ);
  ಯದಿ( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    ಸಂರಚನಾ _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    ಎಣಿಕೆ = pEntry->ಎಣಿಕೆ;
  }ಅನ್ಯಥಾ{
    elem = pH->first;
    ಎಣಿಕೆ = pH->ಎಣಿಕೆ;
  }
  ಯದಿ( pHash ) *pHash = h;
  ಯಾವತ್( ಎಣಿಕೆ ){
    ಪ್ರತಿಜ್ಞಾ( elem!=0 );
    ಯದಿ( h==elem->h && sqlite3StrICmp(elem->p_ಕೀಲಿ,p_ಕೀಲಿ)==0 ){ 
      ಪ್ರತ್ಯಾಗಮ elem;
    }
    elem = elem->next;
    ಎಣಿಕೆ--;
  }
  ಪ್ರತ್ಯಾಗಮ &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
ಸ್ಥಿರ ಶೂನ್ಯ removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  ಸಂರಚನಾ _ht *pEntry;
  ಯದಿ( elem->prev ){
    elem->prev->next = elem->next; 
  }ಅನ್ಯಥಾ{
    pH->first = elem->next;
  }
  ಯದಿ( elem->next ){
    elem->next->prev = elem->prev;
  }
  ಯದಿ( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    ಯದಿ( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    ಪ್ರತಿಜ್ಞಾ( pEntry->ಎಣಿಕೆ>0 );
    pEntry->ಎಣಿಕೆ--;
  }
  sqlite3_free( elem );
  pH->ಎಣಿಕೆ--;
  ಯದಿ( pH->ಎಣಿಕೆ==0 ){
    ಪ್ರತಿಜ್ಞಾ( pH->first==0 );
    ಪ್ರತಿಜ್ಞಾ( pH->ಎಣಿಕೆ==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
ಶೂನ್ಯ *sqlite3HashFind(ಧ್ರುವ Hash *pH, ಧ್ರುವ ವರ್ಣ *p_ಕೀಲಿ){
  ಪ್ರತಿಜ್ಞಾ( pH!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ!=0 );
  ಪ್ರತ್ಯಾಗಮ findElementWithHash(pH, p_ಕೀಲಿ, 0)->data;
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
ಶೂನ್ಯ *sqlite3_Hash_ಸೇರಿಸು(Hash *pH, ಧ್ರುವ ವರ್ಣ *p_ಕೀಲಿ, ಶೂನ್ಯ *data){
  unsigned ಪೂರ್ಣಾಙ್ಕ h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  ಪ್ರತಿಜ್ಞಾ( pH!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ!=0 );
  elem = findElementWithHash(pH,p_ಕೀಲಿ,&h);
  ಯದಿ( elem->data ){
    ಶೂನ್ಯ *old_data = elem->data;
    ಯದಿ( data==0 ){
      removeElement(pH,elem);
    }ಅನ್ಯಥಾ{
      elem->data = data;
      elem->p_ಕೀಲಿ = p_ಕೀಲಿ;
    }
    ಪ್ರತ್ಯಾಗಮ old_data;
  }
  ಯದಿ( data==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  new_elem = (HashElem*)sqlite3Malloc( ಪರಿಮಾಣ(HashElem) );
  ಯದಿ( new_elem==0 ) ಪ್ರತ್ಯಾಗಮ data;
  new_elem->p_ಕೀಲಿ = p_ಕೀಲಿ;
  new_elem->h = h;
  new_elem->data = data;
  pH->ಎಣಿಕೆ++;
  ಯದಿ( pH->ಎಣಿಕೆ>=5 && pH->ಎಣಿಕೆ > 2*pH->htsize ){
    rehash(pH, pH->ಎಣಿಕೆ*3);
  }
  ಸೇರಿಸು_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  ಪ್ರತ್ಯಾಗಮ 0;
}

