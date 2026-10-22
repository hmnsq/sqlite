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
خالی sqlite3HashInit(Hash *pNew){
  أكد( pNew!=0 );
  pNew->پہلا = 0;
  pNew->گنتی = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
خالی sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  أكد( pH!=0 );
  elem = pH->پہلا;
  pH->پہلا = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  جب( elem ){
    HashElem *اگلا_elem = elem->اگلا;
    sqlite3_free(elem);
    elem = اگلا_elem;
  }
  pH->گنتی = 0;
}

/*
** The hashing function.
*/
ساکن غیرعلامتی صحیح strHash(ثابت حرف *z){
  غیرعلامتی صحیح h = 0;
  جب( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#اگر_تعریف_شدہ SQLITE_EBCDIC
    h += 0xbf & (غیرعلامتی حرف)*(z++);
#ورنہ
    h += 0xdf & (غیرعلامتی حرف)*(z++);
#اگر_ختم
    h *= 0x9e3779b1;
  }
  واپس h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ساکن خالی داخل_کرو_Element(
  Hash *pH,              /* The complete hash table */
  ساخت _ht *p_اندراج,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *p_سر;       /* First element already in pEntry */
  اگر( p_اندراج ){
    p_سر = p_اندراج->گنتی ? p_اندراج->chain : 0;
    p_اندراج->گنتی++;
    p_اندراج->chain = pNew;
  }ورنہ{
    p_سر = 0;
  }
  اگر( p_سر ){
    pNew->اگلا = p_سر;
    pNew->پچھلا = p_سر->پچھلا;
    اگر( p_سر->پچھلا ){ p_سر->پچھلا->اگلا = pNew; }
    ورنہ             { pH->پہلا = pNew; }
    p_سر->پچھلا = pNew;
  }ورنہ{
    pNew->اگلا = pH->پہلا;
    اگر( pH->پہلا ){ pH->پہلا->پچھلا = pNew; }
    pNew->پچھلا = 0;
    pH->پہلا = pNew;
  }
}


/* Resize the hash table so that it contains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
ساکن صحیح rehash(Hash *pH, غیرعلامتی صحیح new_سائز){
  ساخت _ht *new_ht;            /* The new hash table */
  HashElem *elem, *اگلا_elem;    /* For looping over existing elements */

#اگر SQLITE_MALLOC_SOFT_حد_بندی>0
  اگر( new_سائز*حجم(ساخت _ht)>SQLITE_MALLOC_SOFT_حد_بندی ){
    new_سائز = SQLITE_MALLOC_SOFT_حد_بندی/حجم(ساخت _ht);
  }
  اگر( new_سائز==pH->htsize ) واپس 0;
#اگر_ختم

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (ساخت _ht *)sqlite3Malloc( new_سائز*حجم(ساخت _ht) );
  sqlite3_انتہا_Benign_Malloc();

  اگر( new_ht==0 ) واپس 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_سائز = sqlite3_Malloc_سائز(new_ht)/حجم(ساخت _ht);
  memset(new_ht, 0, new_سائز*حجم(ساخت _ht));
  برائے(elem=pH->پہلا, pH->پہلا=0; elem; elem = اگلا_elem){
    اگلا_elem = elem->اگلا;
    داخل_کرو_Element(pH, &new_ht[elem->h % new_سائز], elem);
  }
  واپس 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ساکن HashElem *ڈھونڈو_Element_With_Hash(
  ثابت Hash *pH,     /* The pH to be searched */
  ثابت حرف *p_کلید,   /* The key we are searching for */
  غیرعلامتی صحیح *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  غیرعلامتی صحیح گنتی;            /* Number of elements left to test */
  غیرعلامتی صحیح h;                /* The computed hash */
  ساکن HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_کلید);
  اگر( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    ساخت _ht *p_اندراج;
    p_اندراج = &pH->ht[h % pH->htsize];
    elem = p_اندراج->chain;
    گنتی = p_اندراج->گنتی;
  }ورنہ{
    elem = pH->پہلا;
    گنتی = pH->گنتی;
  }
  اگر( pHash ) *pHash = h;
  جب( گنتی ){
    أكد( elem!=0 );
    اگر( h==elem->h && sqlite3StrICmp(elem->p_کلید,p_کلید)==0 ){ 
      واپس elem;
    }
    elem = elem->اگلا;
    گنتی--;
  }
  واپس &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
ساکن خالی removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  ساخت _ht *p_اندراج;
  اگر( elem->پچھلا ){
    elem->پچھلا->اگلا = elem->اگلا; 
  }ورنہ{
    pH->پہلا = elem->اگلا;
  }
  اگر( elem->اگلا ){
    elem->اگلا->پچھلا = elem->پچھلا;
  }
  اگر( pH->ht ){
    p_اندراج = &pH->ht[elem->h % pH->htsize];
    اگر( p_اندراج->chain==elem ){
      p_اندراج->chain = elem->اگلا;
    }
    أكد( p_اندراج->گنتی>0 );
    p_اندراج->گنتی--;
  }
  sqlite3_free( elem );
  pH->گنتی--;
  اگر( pH->گنتی==0 ){
    أكد( pH->پہلا==0 );
    أكد( pH->گنتی==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
خالی *sqlite3_Hash_ڈھونڈو(ثابت Hash *pH, ثابت حرف *p_کلید){
  أكد( pH!=0 );
  أكد( p_کلید!=0 );
  واپس ڈھونڈو_Element_With_Hash(pH, p_کلید, 0)->ڈیٹا;
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
خالی *sqlite3_Hash_داخل_کرو(Hash *pH, ثابت حرف *p_کلید, خالی *ڈیٹا){
  غیرعلامتی صحیح h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  أكد( pH!=0 );
  أكد( p_کلید!=0 );
  elem = ڈھونڈو_Element_With_Hash(pH,p_کلید,&h);
  اگر( elem->ڈیٹا ){
    خالی *old_ڈیٹا = elem->ڈیٹا;
    اگر( ڈیٹا==0 ){
      removeElement(pH,elem);
    }ورنہ{
      elem->ڈیٹا = ڈیٹا;
      elem->p_کلید = p_کلید;
    }
    واپس old_ڈیٹا;
  }
  اگر( ڈیٹا==0 ) واپس 0;
  new_elem = (HashElem*)sqlite3Malloc( حجم(HashElem) );
  اگر( new_elem==0 ) واپس ڈیٹا;
  new_elem->p_کلید = p_کلید;
  new_elem->h = h;
  new_elem->ڈیٹا = ڈیٹا;
  pH->گنتی++;
  اگر( pH->گنتی>=5 && pH->گنتی > 2*pH->htsize ){
    rehash(pH, pH->گنتی*3);
  }
  داخل_کرو_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  واپس 0;
}

