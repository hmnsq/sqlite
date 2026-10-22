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
  pNew->پہلا = 0;
  pNew->گنتی = 0;
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
  elem = pH->پہلا;
  pH->پہلا = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  طالما( elem ){
    HashElem *اگلا_elem = elem->اگلا;
    sqlite3_free(elem);
    elem = اگلا_elem;
  }
  pH->گنتی = 0;
}

/*
** The hashing function.
*/
ساكن بنانشانی صحيح strHash(ثابت حرف *z){
  بنانشانی صحيح h = 0;
  طالما( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#جے_تعریف_شدہ SQLITE_EBCDIC
    h += 0xbf & (بنانشانی حرف)*(z++);
#وإلا
    h += 0xdf & (بنانشانی حرف)*(z++);
#جے_انت
    h *= 0x9e3779b1;
  }
  أرجع h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
ساكن فراغ داخل_کرو_Element(
  Hash *pH,              /* The complete hash table */
  بنية _ht *p_اندراج,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *p_سر;       /* First element already in pEntry */
  إذا( p_اندراج ){
    p_سر = p_اندراج->گنتی ? p_اندراج->chain : 0;
    p_اندراج->گنتی++;
    p_اندراج->chain = pNew;
  }وإلا{
    p_سر = 0;
  }
  إذا( p_سر ){
    pNew->اگلا = p_سر;
    pNew->prev = p_سر->prev;
    إذا( p_سر->prev ){ p_سر->prev->اگلا = pNew; }
    وإلا             { pH->پہلا = pNew; }
    p_سر->prev = pNew;
  }وإلا{
    pNew->اگلا = pH->پہلا;
    إذا( pH->پہلا ){ pH->پہلا->prev = pNew; }
    pNew->prev = 0;
    pH->پہلا = pNew;
  }
}


/* Resize the hash table so that it contains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
ساكن صحيح rehash(Hash *pH, بنانشانی صحيح new_ناپ){
  بنية _ht *new_ht;            /* The new hash table */
  HashElem *elem, *اگلا_elem;    /* For looping over existing elements */

#إذا SQLITE_MALLOC_SOFT_حد_بندی>0
  إذا( new_ناپ*حجم(بنية _ht)>SQLITE_MALLOC_SOFT_حد_بندی ){
    new_ناپ = SQLITE_MALLOC_SOFT_حد_بندی/حجم(بنية _ht);
  }
  إذا( new_ناپ==pH->htsize ) أرجع 0;
#جے_انت

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (بنية _ht *)sqlite3Malloc( new_ناپ*حجم(بنية _ht) );
  sqlite3_سرا_Benign_Malloc();

  إذا( new_ht==0 ) أرجع 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_ناپ = sqlite3_Malloc_ناپ(new_ht)/حجم(بنية _ht);
  memset(new_ht, 0, new_ناپ*حجم(بنية _ht));
  لكل(elem=pH->پہلا, pH->پہلا=0; elem; elem = اگلا_elem){
    اگلا_elem = elem->اگلا;
    داخل_کرو_Element(pH, &new_ht[elem->h % new_ناپ], elem);
  }
  أرجع 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
ساكن HashElem *لبھو_Element_With_Hash(
  ثابت Hash *pH,     /* The pH to be searched */
  ثابت حرف *p_کلید,   /* The key we are searching for */
  بنانشانی صحيح *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  بنانشانی صحيح گنتی;            /* Number of elements left to test */
  بنانشانی صحيح h;                /* The computed hash */
  ساكن HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_کلید);
  إذا( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    بنية _ht *p_اندراج;
    p_اندراج = &pH->ht[h % pH->htsize];
    elem = p_اندراج->chain;
    گنتی = p_اندراج->گنتی;
  }وإلا{
    elem = pH->پہلا;
    گنتی = pH->گنتی;
  }
  إذا( pHash ) *pHash = h;
  طالما( گنتی ){
    أكد( elem!=0 );
    إذا( h==elem->h && sqlite3StrICmp(elem->p_کلید,p_کلید)==0 ){ 
      أرجع elem;
    }
    elem = elem->اگلا;
    گنتی--;
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
  بنية _ht *p_اندراج;
  إذا( elem->prev ){
    elem->prev->اگلا = elem->اگلا; 
  }وإلا{
    pH->پہلا = elem->اگلا;
  }
  إذا( elem->اگلا ){
    elem->اگلا->prev = elem->prev;
  }
  إذا( pH->ht ){
    p_اندراج = &pH->ht[elem->h % pH->htsize];
    إذا( p_اندراج->chain==elem ){
      p_اندراج->chain = elem->اگلا;
    }
    أكد( p_اندراج->گنتی>0 );
    p_اندراج->گنتی--;
  }
  sqlite3_free( elem );
  pH->گنتی--;
  إذا( pH->گنتی==0 ){
    أكد( pH->پہلا==0 );
    أكد( pH->گنتی==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
فراغ *sqlite3_Hash_لبھو(ثابت Hash *pH, ثابت حرف *p_کلید){
  أكد( pH!=0 );
  أكد( p_کلید!=0 );
  أرجع لبھو_Element_With_Hash(pH, p_کلید, 0)->انکڑے;
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
فراغ *sqlite3_Hash_داخل_کرو(Hash *pH, ثابت حرف *p_کلید, فراغ *انکڑے){
  بنانشانی صحيح h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  أكد( pH!=0 );
  أكد( p_کلید!=0 );
  elem = لبھو_Element_With_Hash(pH,p_کلید,&h);
  إذا( elem->انکڑے ){
    فراغ *old_انکڑے = elem->انکڑے;
    إذا( انکڑے==0 ){
      removeElement(pH,elem);
    }وإلا{
      elem->انکڑے = انکڑے;
      elem->p_کلید = p_کلید;
    }
    أرجع old_انکڑے;
  }
  إذا( انکڑے==0 ) أرجع 0;
  new_elem = (HashElem*)sqlite3Malloc( حجم(HashElem) );
  إذا( new_elem==0 ) أرجع انکڑے;
  new_elem->p_کلید = p_کلید;
  new_elem->h = h;
  new_elem->انکڑے = انکڑے;
  pH->گنتی++;
  إذا( pH->گنتی>=5 && pH->گنتی > 2*pH->htsize ){
    rehash(pH, pH->گنتی*3);
  }
  داخل_کرو_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  أرجع 0;
}

