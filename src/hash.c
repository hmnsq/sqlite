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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
શૂન્ય sqlite3HashInit(Hash *pNew){
  પ્રતિજ્ઞા( pNew!=0 );
  pNew->first = 0;
  pNew->ગણતરી = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
શૂન્ય sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  પ્રતિજ્ઞા( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  જ્યારે( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->ગણતરી = 0;
}

/*
** The hashing function.
*/
સ્થિર unsigned પૂર્ણાંક strHash(અચલ અક્ષર *z){
  unsigned પૂર્ણાંક h = 0;
  જ્યારે( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned અક્ષર)*(z++);
#નહીંતર
    h += 0xdf & (unsigned અક્ષર)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  પાછા h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
સ્થિર શૂન્ય ઉમેરો_Element(
  Hash *pH,              /* The complete hash table */
  સંરચના _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  જો( pEntry ){
    pHead = pEntry->ગણતરી ? pEntry->chain : 0;
    pEntry->ગણતરી++;
    pEntry->chain = pNew;
  }નહીંતર{
    pHead = 0;
  }
  જો( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    જો( pHead->prev ){ pHead->prev->next = pNew; }
    નહીંતર             { pH->first = pNew; }
    pHead->prev = pNew;
  }નહીંતર{
    pNew->next = pH->first;
    જો( pH->first ){ pH->first->prev = pNew; }
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
સ્થિર પૂર્ણાંક rehash(Hash *pH, unsigned પૂર્ણાંક new_size){
  સંરચના _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#જો SQLITE_MALLOC_SOFT_LIMIT>0
  જો( new_size*કદ(સંરચના _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/કદ(સંરચના _ht);
  }
  જો( new_size==pH->htsize ) પાછા 0;
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
  new_ht = (સંરચના _ht *)sqlite3Malloc( new_size*કદ(સંરચના _ht) );
  sqlite3EndBenignMalloc();

  જો( new_ht==0 ) પાછા 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/કદ(સંરચના _ht);
  memset(new_ht, 0, new_size*કદ(સંરચના _ht));
  માટે(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    ઉમેરો_Element(pH, &new_ht[elem->h % new_size], elem);
  }
  પાછા 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
સ્થિર HashElem *findElementWithHash(
  અચલ Hash *pH,     /* The pH to be searched */
  અચલ અક્ષર *p_ચાવી,   /* The key we are searching for */
  unsigned પૂર્ણાંક *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned પૂર્ણાંક ગણતરી;            /* Number of elements left to test */
  unsigned પૂર્ણાંક h;                /* The computed hash */
  સ્થિર HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_ચાવી);
  જો( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    સંરચના _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    ગણતરી = pEntry->ગણતરી;
  }નહીંતર{
    elem = pH->first;
    ગણતરી = pH->ગણતરી;
  }
  જો( pHash ) *pHash = h;
  જ્યારે( ગણતરી ){
    પ્રતિજ્ઞા( elem!=0 );
    જો( h==elem->h && sqlite3StrICmp(elem->p_ચાવી,p_ચાવી)==0 ){ 
      પાછા elem;
    }
    elem = elem->next;
    ગણતરી--;
  }
  પાછા &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
સ્થિર શૂન્ય removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  સંરચના _ht *pEntry;
  જો( elem->prev ){
    elem->prev->next = elem->next; 
  }નહીંતર{
    pH->first = elem->next;
  }
  જો( elem->next ){
    elem->next->prev = elem->prev;
  }
  જો( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    જો( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    પ્રતિજ્ઞા( pEntry->ગણતરી>0 );
    pEntry->ગણતરી--;
  }
  sqlite3_free( elem );
  pH->ગણતરી--;
  જો( pH->ગણતરી==0 ){
    પ્રતિજ્ઞા( pH->first==0 );
    પ્રતિજ્ઞા( pH->ગણતરી==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
શૂન્ય *sqlite3HashFind(અચલ Hash *pH, અચલ અક્ષર *p_ચાવી){
  પ્રતિજ્ઞા( pH!=0 );
  પ્રતિજ્ઞા( p_ચાવી!=0 );
  પાછા findElementWithHash(pH, p_ચાવી, 0)->data;
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
શૂન્ય *sqlite3_Hash_ઉમેરો(Hash *pH, અચલ અક્ષર *p_ચાવી, શૂન્ય *data){
  unsigned પૂર્ણાંક h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  પ્રતિજ્ઞા( pH!=0 );
  પ્રતિજ્ઞા( p_ચાવી!=0 );
  elem = findElementWithHash(pH,p_ચાવી,&h);
  જો( elem->data ){
    શૂન્ય *old_data = elem->data;
    જો( data==0 ){
      removeElement(pH,elem);
    }નહીંતર{
      elem->data = data;
      elem->p_ચાવી = p_ચાવી;
    }
    પાછા old_data;
  }
  જો( data==0 ) પાછા 0;
  new_elem = (HashElem*)sqlite3Malloc( કદ(HashElem) );
  જો( new_elem==0 ) પાછા data;
  new_elem->p_ચાવી = p_ચાવી;
  new_elem->h = h;
  new_elem->data = data;
  pH->ગણતરી++;
  જો( pH->ગણતરી>=5 && pH->ગણતરી > 2*pH->htsize ){
    rehash(pH, pH->ગણતરી*3);
  }
  ઉમેરો_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  પાછા 0;
}

