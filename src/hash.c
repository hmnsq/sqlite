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
#include "sqliteInt.h"
#include <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
void sqlite3HashInit(Hash *pNew){
  assert( pNew!=0 );
  pNew->first = 0;
  pNew->count = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
void sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  assert( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  ながら( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->count = 0;
}

/*
** The hashing function.
*/
static unsigned 整数 strHash(const char *z){
  unsigned 整数 h = 0;
  ながら( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#ifdef SQLITE_EBCDIC
    h += 0xbf & (unsigned char)*(z++);
#否则
    h += 0xdf & (unsigned char)*(z++);
#endif
    h *= 0x9e3779b1;
  }
  戻る h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
static void insertElement(
  Hash *pH,              /* The complete hash table */
  struct _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  もし( pEntry ){
    pHead = pEntry->count ? pEntry->chain : 0;
    pEntry->count++;
    pEntry->chain = pNew;
  }否则{
    pHead = 0;
  }
  もし( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    もし( pHead->prev ){ pHead->prev->next = pNew; }
    否则             { pH->first = pNew; }
    pHead->prev = pNew;
  }否则{
    pNew->next = pH->first;
    もし( pH->first ){ pH->first->prev = pNew; }
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
static 整数 rehash(Hash *pH, unsigned 整数 new_size){
  struct _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#もし SQLITE_MALLOC_SOFT_LIMIT>0
  もし( new_size*sizeof(struct _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/sizeof(struct _ht);
  }
  もし( new_size==pH->htsize ) 戻る 0;
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
  new_ht = (struct _ht *)sqlite3Malloc( new_size*sizeof(struct _ht) );
  sqlite3EndBenignMalloc();

  もし( new_ht==0 ) 戻る 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/sizeof(struct _ht);
  memset(new_ht, 0, new_size*sizeof(struct _ht));
  ために(elem=pH->first, pH->first=0; elem; elem = next_elem){
    next_elem = elem->next;
    insertElement(pH, &new_ht[elem->h % new_size], elem);
  }
  戻る 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
static HashElem *findElementWithHash(
  const Hash *pH,     /* The pH to be searched */
  const char *pKey,   /* The key we are searching for */
  unsigned 整数 *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  unsigned 整数 count;            /* Number of elements left to test */
  unsigned 整数 h;                /* The computed hash */
  static HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(pKey);
  もし( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    struct _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    count = pEntry->count;
  }否则{
    elem = pH->first;
    count = pH->count;
  }
  もし( pHash ) *pHash = h;
  ながら( count ){
    assert( elem!=0 );
    もし( h==elem->h && sqlite3StrICmp(elem->pKey,pKey)==0 ){ 
      戻る elem;
    }
    elem = elem->next;
    count--;
  }
  戻る &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
static void removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  struct _ht *pEntry;
  もし( elem->prev ){
    elem->prev->next = elem->next; 
  }否则{
    pH->first = elem->next;
  }
  もし( elem->next ){
    elem->next->prev = elem->prev;
  }
  もし( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    もし( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    assert( pEntry->count>0 );
    pEntry->count--;
  }
  sqlite3_free( elem );
  pH->count--;
  もし( pH->count==0 ){
    assert( pH->first==0 );
    assert( pH->count==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
void *sqlite3HashFind(const Hash *pH, const char *pKey){
  assert( pH!=0 );
  assert( pKey!=0 );
  戻る findElementWithHash(pH, pKey, 0)->data;
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
void *sqlite3HashInsert(Hash *pH, const char *pKey, void *data){
  unsigned 整数 h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  assert( pH!=0 );
  assert( pKey!=0 );
  elem = findElementWithHash(pH,pKey,&h);
  もし( elem->data ){
    void *old_data = elem->data;
    もし( data==0 ){
      removeElement(pH,elem);
    }否则{
      elem->data = data;
      elem->pKey = pKey;
    }
    戻る old_data;
  }
  もし( data==0 ) 戻る 0;
  new_elem = (HashElem*)sqlite3Malloc( sizeof(HashElem) );
  もし( new_elem==0 ) 戻る data;
  new_elem->pKey = pKey;
  new_elem->h = h;
  new_elem->data = data;
  pH->count++;
  もし( pH->count>=5 && pH->count > 2*pH->htsize ){
    rehash(pH, pH->count*3);
  }
  insertElement(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  戻る 0;
}

