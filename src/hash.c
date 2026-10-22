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
  pNew->मोजणी = 0;
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
  पर्यंत( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->मोजणी = 0;
}

/*
** The hashing function.
*/
स्थिर अचिन्हित पूर्णांक strHash(स्थिरांक अक्षर *z){
  अचिन्हित पूर्णांक h = 0;
  पर्यंत( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (अचिन्हित अक्षर)*(z++);
#अन्यथा
    h += 0xdf & (अचिन्हित अक्षर)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  परत h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
स्थिर शून्य घाला_Element(
  Hash *pH,              /* The complete hash table */
  संरचना _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  जर( pEntry ){
    pHead = pEntry->मोजणी ? pEntry->chain : 0;
    pEntry->मोजणी++;
    pEntry->chain = pNew;
  }अन्यथा{
    pHead = 0;
  }
  जर( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    जर( pHead->prev ){ pHead->prev->next = pNew; }
    अन्यथा             { pH->first = pNew; }
    pHead->prev = pNew;
  }अन्यथा{
    pNew->next = pH->first;
    जर( pH->first ){ pH->first->prev = pNew; }
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
स्थिर पूर्णांक rehash(Hash *pH, अचिन्हित पूर्णांक new_size){
  संरचना _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#जर SQLITE_MALLOC_SOFT_LIMIT>0
  जर( new_size*आकार(संरचना _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/आकार(संरचना _ht);
  }
  जर( new_size==pH->htsize ) परत 0;
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
  new_ht = (संरचना _ht *)sqlite3Malloc( new_size*आकार(संरचना _ht) );
  sqlite3EndBenignMalloc();

  जर( new_ht==0 ) परत 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/आकार(संरचना _ht);
  स_रखो(new_ht, 0, new_size*आकार(संरचना _ht));
  साठी(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    घाला_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  परत 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
स्थिर HashElem *findElementWithHash(
  स्थिरांक Hash *pH,     /* The pH to be searched */
  स्थिरांक अक्षर *p_किल्ली,   /* The key we are searching for */
  अचिन्हित पूर्णांक *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  अचिन्हित पूर्णांक मोजणी;            /* Number of elements left to test */
  अचिन्हित पूर्णांक h;                /* The computed hash */
  स्थिर HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_किल्ली);
  जर( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    संरचना _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    मोजणी = pEntry->मोजणी;
  }अन्यथा{
    elem = pH->first;
    मोजणी = pH->मोजणी;
  }
  जर( pHash ) *pHash = h;
  पर्यंत( मोजणी ){
    प्रतिज्ञा( elem!=0 );
    जर( h==elem->h && sqlite3StrICmp(elem->p_किल्ली,p_किल्ली)==0 ){ 
      परत elem;
    }
    elem = elem->next;
    मोजणी--;
  }
  परत &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
स्थिर शून्य removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  संरचना _ht *pEntry;
  जर( elem->prev ){
    elem->prev->next = elem->next; 
  }अन्यथा{
    pH->first = elem->next;
  }
  जर( elem->next ){
    elem->next->prev = elem->prev;
  }
  जर( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    जर( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    प्रतिज्ञा( pEntry->मोजणी>0 );
    pEntry->मोजणी--;
  }
  sqlite3_free( elem );
  pH->मोजणी--;
  जर( pH->मोजणी==0 ){
    प्रतिज्ञा( pH->first==0 );
    प्रतिज्ञा( pH->मोजणी==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
शून्य *sqlite3HashFind(स्थिरांक Hash *pH, स्थिरांक अक्षर *p_किल्ली){
  प्रतिज्ञा( pH!=0 );
  प्रतिज्ञा( p_किल्ली!=0 );
  परत findElementWithHash(pH, p_किल्ली, 0)->data;
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
शून्य *sqlite3_Hash_घाला(Hash *pH, स्थिरांक अक्षर *p_किल्ली, शून्य *data){
  अचिन्हित पूर्णांक h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  प्रतिज्ञा( pH!=0 );
  प्रतिज्ञा( p_किल्ली!=0 );
  elem = findElementWithHash(pH,p_किल्ली,&h);
  जर( elem->data ){
    शून्य *old_data = elem->data;
    जर( data==0 ){
      removeElement(pH,elem);
    }अन्यथा{
      elem->data = data;
      elem->p_किल्ली = p_किल्ली;
    }
    परत old_data;
  }
  जर( data==0 ) परत 0;
  new_elem = (HashElem*)sqlite3Malloc( आकार(HashElem) );
  जर( new_elem==0 ) परत data;
  new_elem->p_किल्ली = p_किल्ली;
  new_elem->h = h;
  new_elem->data = data;
  pH->मोजणी++;
  जर( pH->मोजणी>=5 && pH->मोजणी > 2*pH->htsize ){
    rehash(pH, pH->मोजणी*3);
  }
  घाला_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  परत 0;
}

