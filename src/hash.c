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
  pNew->پهريون = 0;
  pNew->ڳڻپ = 0;
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
  elem = pH->پهريون;
  pH->پهريون = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  طالما( elem ){
    HashElem *ايندڙ_elem = elem->ايندڙ;
    sqlite3_free(elem);
    elem = ايندڙ_elem;
  }
  pH->ڳڻپ = 0;
}

/*
** The hashing function.
*/
ساكن بنانشان صحيح strHash(ثابت حرف *z){
  بنانشان صحيح h = 0;
  طالما( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#جيڪڏهن_وصف_ٿيل SQLITE_EBCDIC
    h += 0xbf & (بنانشان حرف)*(z++);
#وإلا
    h += 0xdf & (بنانشان حرف)*(z++);
#جيڪڏهن_پورو
    h *= 0x9e3779b1;
  }
  أرجع h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ساكن فراغ وجهو_Element(
  Hash *pH,              /* The complete hash table */
  بنية _ht *p_داخلا,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *p_سر;       /* First element already in pEntry */
  إذا( p_داخلا ){
    p_سر = p_داخلا->ڳڻپ ? p_داخلا->chain : 0;
    p_داخلا->ڳڻپ++;
    p_داخلا->chain = pNew;
  }وإلا{
    p_سر = 0;
  }
  إذا( p_سر ){
    pNew->ايندڙ = p_سر;
    pNew->prev = p_سر->prev;
    إذا( p_سر->prev ){ p_سر->prev->ايندڙ = pNew; }
    وإلا             { pH->پهريون = pNew; }
    p_سر->prev = pNew;
  }وإلا{
    pNew->ايندڙ = pH->پهريون;
    إذا( pH->پهريون ){ pH->پهريون->prev = pNew; }
    pNew->prev = 0;
    pH->پهريون = pNew;
  }
}


/* Resize the hash table so that it contains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
ساكن صحيح rehash(Hash *pH, بنانشان صحيح new_سائيز){
  بنية _ht *new_ht;            /* The new hash table */
  HashElem *elem, *ايندڙ_elem;    /* For looping over existing elements */

#إذا SQLITE_MALLOC_SOFT_حد_بندي>0
  إذا( new_سائيز*حجم(بنية _ht)>SQLITE_MALLOC_SOFT_حد_بندي ){
    new_سائيز = SQLITE_MALLOC_SOFT_حد_بندي/حجم(بنية _ht);
  }
  إذا( new_سائيز==pH->htsize ) أرجع 0;
#جيڪڏهن_پورو

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (بنية _ht *)sqlite3Malloc( new_سائيز*حجم(بنية _ht) );
  sqlite3_پڇاڙي_Benign_Malloc();

  إذا( new_ht==0 ) أرجع 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_سائيز = sqlite3_Malloc_سائيز(new_ht)/حجم(بنية _ht);
  memset(new_ht, 0, new_سائيز*حجم(بنية _ht));
  لكل(elem=pH->پهريون, pH->پهريون=0; elem; elem = ايندڙ_elem){
    ايندڙ_elem = elem->ايندڙ;
    وجهو_Element(pH, &new_ht[elem->h % new_سائيز], elem);
  }
  أرجع 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ساكن HashElem *ڳوليو_Element_With_Hash(
  ثابت Hash *pH,     /* The pH to be searched */
  ثابت حرف *p_ڪنجي,   /* The key we are searching for */
  بنانشان صحيح *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  بنانشان صحيح ڳڻپ;            /* Number of elements left to test */
  بنانشان صحيح h;                /* The computed hash */
  ساكن HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_ڪنجي);
  إذا( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    بنية _ht *p_داخلا;
    p_داخلا = &pH->ht[h % pH->htsize];
    elem = p_داخلا->chain;
    ڳڻپ = p_داخلا->ڳڻپ;
  }وإلا{
    elem = pH->پهريون;
    ڳڻپ = pH->ڳڻپ;
  }
  إذا( pHash ) *pHash = h;
  طالما( ڳڻپ ){
    أكد( elem!=0 );
    إذا( h==elem->h && sqlite3StrICmp(elem->p_ڪنجي,p_ڪنجي)==0 ){ 
      أرجع elem;
    }
    elem = elem->ايندڙ;
    ڳڻپ--;
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
  بنية _ht *p_داخلا;
  إذا( elem->prev ){
    elem->prev->ايندڙ = elem->ايندڙ; 
  }وإلا{
    pH->پهريون = elem->ايندڙ;
  }
  إذا( elem->ايندڙ ){
    elem->ايندڙ->prev = elem->prev;
  }
  إذا( pH->ht ){
    p_داخلا = &pH->ht[elem->h % pH->htsize];
    إذا( p_داخلا->chain==elem ){
      p_داخلا->chain = elem->ايندڙ;
    }
    أكد( p_داخلا->ڳڻپ>0 );
    p_داخلا->ڳڻپ--;
  }
  sqlite3_free( elem );
  pH->ڳڻپ--;
  إذا( pH->ڳڻپ==0 ){
    أكد( pH->پهريون==0 );
    أكد( pH->ڳڻپ==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
فراغ *sqlite3_Hash_ڳوليو(ثابت Hash *pH, ثابت حرف *p_ڪنجي){
  أكد( pH!=0 );
  أكد( p_ڪنجي!=0 );
  أرجع ڳوليو_Element_With_Hash(pH, p_ڪنجي, 0)->معلومات;
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
فراغ *sqlite3_Hash_وجهو(Hash *pH, ثابت حرف *p_ڪنجي, فراغ *معلومات){
  بنانشان صحيح h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  أكد( pH!=0 );
  أكد( p_ڪنجي!=0 );
  elem = ڳوليو_Element_With_Hash(pH,p_ڪنجي,&h);
  إذا( elem->معلومات ){
    فراغ *old_معلومات = elem->معلومات;
    إذا( معلومات==0 ){
      removeElement(pH,elem);
    }وإلا{
      elem->معلومات = معلومات;
      elem->p_ڪنجي = p_ڪنجي;
    }
    أرجع old_معلومات;
  }
  إذا( معلومات==0 ) أرجع 0;
  new_elem = (HashElem*)sqlite3Malloc( حجم(HashElem) );
  إذا( new_elem==0 ) أرجع معلومات;
  new_elem->p_ڪنجي = p_ڪنجي;
  new_elem->h = h;
  new_elem->معلومات = معلومات;
  pH->ڳڻپ++;
  إذا( pH->ڳڻپ>=5 && pH->ڳڻپ > 2*pH->htsize ){
    rehash(pH, pH->ڳڻپ*3);
  }
  وجهو_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  أرجع 0;
}

