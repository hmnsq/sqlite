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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
ശൂന്യം sqlite3HashInit(Hash *pNew){
  പ്രതിജ്ഞാ( pNew!=0 );
  pNew->first = 0;
  pNew->എണ്ണം = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
ശൂന്യം sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  പ്രതിജ്ഞാ( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  ആകുമ്പോൾ( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->എണ്ണം = 0;
}

/*
** The hashing function.
*/
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ strHash(സ്ഥിരം അക്ഷരം *z){
  unsigned പൂർണ്ണസംഖ്യ h = 0;
  ആകുമ്പോൾ( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned അക്ഷരം)*(z++);
#അല്ലെങ്കിൽ
    h += 0xdf & (unsigned അക്ഷരം)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  തിരികെ h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
സ്ഥിതം ശൂന്യം തിരുകുക_Element(
  Hash *pH,              /* The complete hash table */
  ഘടന _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  എങ്കിൽ( pEntry ){
    pHead = pEntry->എണ്ണം ? pEntry->chain : 0;
    pEntry->എണ്ണം++;
    pEntry->chain = pNew;
  }അല്ലെങ്കിൽ{
    pHead = 0;
  }
  എങ്കിൽ( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    എങ്കിൽ( pHead->prev ){ pHead->prev->next = pNew; }
    അല്ലെങ്കിൽ             { pH->first = pNew; }
    pHead->prev = pNew;
  }അല്ലെങ്കിൽ{
    pNew->next = pH->first;
    എങ്കിൽ( pH->first ){ pH->first->prev = pNew; }
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
സ്ഥിതം പൂർണ്ണസംഖ്യ rehash(Hash *pH, unsigned പൂർണ്ണസംഖ്യ new_size){
  ഘടന _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#എങ്കിൽ SQLITE_MALLOC_SOFT_LIMIT>0
  എങ്കിൽ( new_size*വലുപ്പം(ഘടന _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/വലുപ്പം(ഘടന _ht);
  }
  എങ്കിൽ( new_size==pH->htsize ) തിരികെ 0;
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
  new_ht = (ഘടന _ht *)sqlite3Malloc( new_size*വലുപ്പം(ഘടന _ht) );
  sqlite3EndBenignMalloc();

  എങ്കിൽ( new_ht==0 ) തിരികെ 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/വലുപ്പം(ഘടന _ht);
  memset(new_ht, 0, new_size*വലുപ്പം(ഘടന _ht));
  വേണ്ടി(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    തിരുകുക_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  തിരികെ 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
സ്ഥിതം HashElem *findElementWithHash(
  സ്ഥിരം Hash *pH,     /* The pH to be searched */
  സ്ഥിരം അക്ഷരം *p_താക്കോൽ,   /* The key we are searching for */
  unsigned പൂർണ്ണസംഖ്യ *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned പൂർണ്ണസംഖ്യ എണ്ണം;            /* Number of elements left to test */
  unsigned പൂർണ്ണസംഖ്യ h;                /* The computed hash */
  സ്ഥിതം HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_താക്കോൽ);
  എങ്കിൽ( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    ഘടന _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    എണ്ണം = pEntry->എണ്ണം;
  }അല്ലെങ്കിൽ{
    elem = pH->first;
    എണ്ണം = pH->എണ്ണം;
  }
  എങ്കിൽ( pHash ) *pHash = h;
  ആകുമ്പോൾ( എണ്ണം ){
    പ്രതിജ്ഞാ( elem!=0 );
    എങ്കിൽ( h==elem->h && sqlite3StrICmp(elem->p_താക്കോൽ,p_താക്കോൽ)==0 ){ 
      തിരികെ elem;
    }
    elem = elem->next;
    എണ്ണം--;
  }
  തിരികെ &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
സ്ഥിതം ശൂന്യം removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  ഘടന _ht *pEntry;
  എങ്കിൽ( elem->prev ){
    elem->prev->next = elem->next; 
  }അല്ലെങ്കിൽ{
    pH->first = elem->next;
  }
  എങ്കിൽ( elem->next ){
    elem->next->prev = elem->prev;
  }
  എങ്കിൽ( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    എങ്കിൽ( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    പ്രതിജ്ഞാ( pEntry->എണ്ണം>0 );
    pEntry->എണ്ണം--;
  }
  sqlite3_free( elem );
  pH->എണ്ണം--;
  എങ്കിൽ( pH->എണ്ണം==0 ){
    പ്രതിജ്ഞാ( pH->first==0 );
    പ്രതിജ്ഞാ( pH->എണ്ണം==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
ശൂന്യം *sqlite3HashFind(സ്ഥിരം Hash *pH, സ്ഥിരം അക്ഷരം *p_താക്കോൽ){
  പ്രതിജ്ഞാ( pH!=0 );
  പ്രതിജ്ഞാ( p_താക്കോൽ!=0 );
  തിരികെ findElementWithHash(pH, p_താക്കോൽ, 0)->data;
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
ശൂന്യം *sqlite3_Hash_തിരുകുക(Hash *pH, സ്ഥിരം അക്ഷരം *p_താക്കോൽ, ശൂന്യം *data){
  unsigned പൂർണ്ണസംഖ്യ h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  പ്രതിജ്ഞാ( pH!=0 );
  പ്രതിജ്ഞാ( p_താക്കോൽ!=0 );
  elem = findElementWithHash(pH,p_താക്കോൽ,&h);
  എങ്കിൽ( elem->data ){
    ശൂന്യം *old_data = elem->data;
    എങ്കിൽ( data==0 ){
      removeElement(pH,elem);
    }അല്ലെങ്കിൽ{
      elem->data = data;
      elem->p_താക്കോൽ = p_താക്കോൽ;
    }
    തിരികെ old_data;
  }
  എങ്കിൽ( data==0 ) തിരികെ 0;
  new_elem = (HashElem*)sqlite3Malloc( വലുപ്പം(HashElem) );
  എങ്കിൽ( new_elem==0 ) തിരികെ data;
  new_elem->p_താക്കോൽ = p_താക്കോൽ;
  new_elem->h = h;
  new_elem->data = data;
  pH->എണ്ണം++;
  എങ്കിൽ( pH->എണ്ണം>=5 && pH->എണ്ണം > 2*pH->htsize ){
    rehash(pH, pH->എണ്ണം*3);
  }
  തിരുകുക_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  തിരികെ 0;
}

