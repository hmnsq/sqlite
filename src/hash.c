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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
शून्य sqlite3HashInit(Hash *pNew){
  प्रतिज्ञा( pNew!=0 );
  pNew->first = 0;
  pNew->गणनम् = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
शून्य sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  प्रतिज्ञा( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  जाव( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->गणनम् = 0;
}

/*
** The hashing function.
*/
स्थिर unsigned पूर्णाङ्क strHash(ध्रुव वर्ण *z){
  unsigned पूर्णाङ्क h = 0;
  जाव( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned वर्ण)*(z++);
#अण्णहा
    h += 0xdf & (unsigned वर्ण)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  प्रत्यागम h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
स्थिर शून्य निवेशय_Element(
  Hash *pH,              /* The complete hash table */
  संरचना _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  जइ( pEntry ){
    pHead = pEntry->गणनम् ? pEntry->chain : 0;
    pEntry->गणनम्++;
    pEntry->chain = pNew;
  }अण्णहा{
    pHead = 0;
  }
  जइ( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    जइ( pHead->prev ){ pHead->prev->next = pNew; }
    अण्णहा             { pH->first = pNew; }
    pHead->prev = pNew;
  }अण्णहा{
    pNew->next = pH->first;
    जइ( pH->first ){ pH->first->prev = pNew; }
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
स्थिर पूर्णाङ्क rehash(Hash *pH, unsigned पूर्णाङ्क new_size){
  संरचना _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#जइ SQLITE_MALLOC_SOFT_LIMIT>0
  जइ( new_size*परिमाण(संरचना _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/परिमाण(संरचना _ht);
  }
  जइ( new_size==pH->htsize ) प्रत्यागम 0;
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
  new_ht = (संरचना _ht *)sqlite3Malloc( new_size*परिमाण(संरचना _ht) );
  sqlite3EndBenignMalloc();

  जइ( new_ht==0 ) प्रत्यागम 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/परिमाण(संरचना _ht);
  memset(new_ht, 0, new_size*परिमाण(संरचना _ht));
  प्रति(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    निवेशय_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  प्रत्यागम 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
स्थिर HashElem *findElementWithHash(
  ध्रुव Hash *pH,     /* The pH to be searched */
  ध्रुव वर्ण *p_कुञ्चिका,   /* The key we are searching for */
  unsigned पूर्णाङ्क *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned पूर्णाङ्क गणनम्;            /* Number of elements left to test */
  unsigned पूर्णाङ्क h;                /* The computed hash */
  स्थिर HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_कुञ्चिका);
  जइ( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    संरचना _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    गणनम् = pEntry->गणनम्;
  }अण्णहा{
    elem = pH->first;
    गणनम् = pH->गणनम्;
  }
  जइ( pHash ) *pHash = h;
  जाव( गणनम् ){
    प्रतिज्ञा( elem!=0 );
    जइ( h==elem->h && sqlite3StrICmp(elem->p_कुञ्चिका,p_कुञ्चिका)==0 ){ 
      प्रत्यागम elem;
    }
    elem = elem->next;
    गणनम्--;
  }
  प्रत्यागम &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
स्थिर शून्य removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  संरचना _ht *pEntry;
  जइ( elem->prev ){
    elem->prev->next = elem->next; 
  }अण्णहा{
    pH->first = elem->next;
  }
  जइ( elem->next ){
    elem->next->prev = elem->prev;
  }
  जइ( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    जइ( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    प्रतिज्ञा( pEntry->गणनम्>0 );
    pEntry->गणनम्--;
  }
  sqlite3_free( elem );
  pH->गणनम्--;
  जइ( pH->गणनम्==0 ){
    प्रतिज्ञा( pH->first==0 );
    प्रतिज्ञा( pH->गणनम्==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
शून्य *sqlite3HashFind(ध्रुव Hash *pH, ध्रुव वर्ण *p_कुञ्चिका){
  प्रतिज्ञा( pH!=0 );
  प्रतिज्ञा( p_कुञ्चिका!=0 );
  प्रत्यागम findElementWithHash(pH, p_कुञ्चिका, 0)->data;
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
शून्य *sqlite3_Hash_निवेशय(Hash *pH, ध्रुव वर्ण *p_कुञ्चिका, शून्य *data){
  unsigned पूर्णाङ्क h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  प्रतिज्ञा( pH!=0 );
  प्रतिज्ञा( p_कुञ्चिका!=0 );
  elem = findElementWithHash(pH,p_कुञ्चिका,&h);
  जइ( elem->data ){
    शून्य *old_data = elem->data;
    जइ( data==0 ){
      removeElement(pH,elem);
    }अण्णहा{
      elem->data = data;
      elem->p_कुञ्चिका = p_कुञ्चिका;
    }
    प्रत्यागम old_data;
  }
  जइ( data==0 ) प्रत्यागम 0;
  new_elem = (HashElem*)sqlite3Malloc( परिमाण(HashElem) );
  जइ( new_elem==0 ) प्रत्यागम data;
  new_elem->p_कुञ्चिका = p_कुञ्चिका;
  new_elem->h = h;
  new_elem->data = data;
  pH->गणनम्++;
  जइ( pH->गणनम्>=5 && pH->गणनम् > 2*pH->htsize ){
    rehash(pH, pH->गणनम्*3);
  }
  निवेशय_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  प्रत्यागम 0;
}

