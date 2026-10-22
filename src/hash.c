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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
శూన్య sqlite3HashInit(Hash *pNew){
  ప్రతిజ్ఞా( pNew!=0 );
  pNew->మొదటి = 0;
  pNew->లెక్క = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
శూన్య sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  ప్రతిజ్ఞా( pH!=0 );
  elem = pH->మొదటి;
  pH->మొదటి = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  అయితే( elem ){
    HashElem *తదుపరి_elem = elem->తదుపరి;
    sqlite3_free(elem);
    elem = తదుపరి_elem;
  }
  pH->లెక్క = 0;
}

/*
** The hashing function.
*/
స్థిర గుర్తులేని పూర్ణాంకం strHash(స్థిరరాశి అక్షరం *z){
  గుర్తులేని పూర్ణాంకం h = 0;
  అయితే( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#నిర్వచితమైతే SQLITE_EBCDIC
    h += 0xbf & (గుర్తులేని అక్షరం)*(z++);
#లేకపోతే
    h += 0xdf & (గుర్తులేని అక్షరం)*(z++);
#అయితే_ముగింపు
    h *= 0x9e3779b1;
  }
  తిరిగి h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
స్థిర శూన్య చొప్పించు_Element(
  Hash *pH,              /* The complete hash table */
  నిర్మాణం _ht *p_నమోదు,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *p_తల;       /* First element already in pEntry */
  ఒకవేళ( p_నమోదు ){
    p_తల = p_నమోదు->లెక్క ? p_నమోదు->chain : 0;
    p_నమోదు->లెక్క++;
    p_నమోదు->chain = pNew;
  }లేకపోతే{
    p_తల = 0;
  }
  ఒకవేళ( p_తల ){
    pNew->తదుపరి = p_తల;
    pNew->prev = p_తల->prev;
    ఒకవేళ( p_తల->prev ){ p_తల->prev->తదుపరి = pNew; }
    లేకపోతే             { pH->మొదటి = pNew; }
    p_తల->prev = pNew;
  }లేకపోతే{
    pNew->తదుపరి = pH->మొదటి;
    ఒకవేళ( pH->మొదటి ){ pH->మొదటి->prev = pNew; }
    pNew->prev = 0;
    pH->మొదటి = pNew;
  }
}


/* Resize the hash table so that it contains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
స్థిర పూర్ణాంకం rehash(Hash *pH, గుర్తులేని పూర్ణాంకం new_కొలత){
  నిర్మాణం _ht *new_ht;            /* The new hash table */
  HashElem *elem, *తదుపరి_elem;    /* For looping over existing elements */

#ఒకవేళ SQLITE_MALLOC_SOFT_హద్దు>0
  ఒకవేళ( new_కొలత*పరిమాణం(నిర్మాణం _ht)>SQLITE_MALLOC_SOFT_హద్దు ){
    new_కొలత = SQLITE_MALLOC_SOFT_హద్దు/పరిమాణం(నిర్మాణం _ht);
  }
  ఒకవేళ( new_కొలత==pH->htsize ) తిరిగి 0;
#అయితే_ముగింపు

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (నిర్మాణం _ht *)sqlite3Malloc( new_కొలత*పరిమాణం(నిర్మాణం _ht) );
  sqlite3_చివర_Benign_Malloc();

  ఒకవేళ( new_ht==0 ) తిరిగి 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_కొలత = sqlite3_Malloc_కొలత(new_ht)/పరిమాణం(నిర్మాణం _ht);
  memset(new_ht, 0, new_కొలత*పరిమాణం(నిర్మాణం _ht));
  కొరకు(elem=pH->మొదటి, pH->మొదటి=0; elem; elem = తదుపరి_elem){
    తదుపరి_elem = elem->తదుపరి;
    చొప్పించు_Element(pH, &new_ht[elem->h % new_కొలత], elem);
  }
  తిరిగి 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
స్థిర HashElem *కనుగొను_Element_With_Hash(
  స్థిరరాశి Hash *pH,     /* The pH to be searched */
  స్థిరరాశి అక్షరం *p_కీలకం,   /* The key we are searching for */
  గుర్తులేని పూర్ణాంకం *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  గుర్తులేని పూర్ణాంకం లెక్క;            /* Number of elements left to test */
  గుర్తులేని పూర్ణాంకం h;                /* The computed hash */
  స్థిర HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_కీలకం);
  ఒకవేళ( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    నిర్మాణం _ht *p_నమోదు;
    p_నమోదు = &pH->ht[h % pH->htsize];
    elem = p_నమోదు->chain;
    లెక్క = p_నమోదు->లెక్క;
  }లేకపోతే{
    elem = pH->మొదటి;
    లెక్క = pH->లెక్క;
  }
  ఒకవేళ( pHash ) *pHash = h;
  అయితే( లెక్క ){
    ప్రతిజ్ఞా( elem!=0 );
    ఒకవేళ( h==elem->h && sqlite3StrICmp(elem->p_కీలకం,p_కీలకం)==0 ){ 
      తిరిగి elem;
    }
    elem = elem->తదుపరి;
    లెక్క--;
  }
  తిరిగి &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
స్థిర శూన్య removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  నిర్మాణం _ht *p_నమోదు;
  ఒకవేళ( elem->prev ){
    elem->prev->తదుపరి = elem->తదుపరి; 
  }లేకపోతే{
    pH->మొదటి = elem->తదుపరి;
  }
  ఒకవేళ( elem->తదుపరి ){
    elem->తదుపరి->prev = elem->prev;
  }
  ఒకవేళ( pH->ht ){
    p_నమోదు = &pH->ht[elem->h % pH->htsize];
    ఒకవేళ( p_నమోదు->chain==elem ){
      p_నమోదు->chain = elem->తదుపరి;
    }
    ప్రతిజ్ఞా( p_నమోదు->లెక్క>0 );
    p_నమోదు->లెక్క--;
  }
  sqlite3_free( elem );
  pH->లెక్క--;
  ఒకవేళ( pH->లెక్క==0 ){
    ప్రతిజ్ఞా( pH->మొదటి==0 );
    ప్రతిజ్ఞా( pH->లెక్క==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
శూన్య *sqlite3_Hash_కనుగొను(స్థిరరాశి Hash *pH, స్థిరరాశి అక్షరం *p_కీలకం){
  ప్రతిజ్ఞా( pH!=0 );
  ప్రతిజ్ఞా( p_కీలకం!=0 );
  తిరిగి కనుగొను_Element_With_Hash(pH, p_కీలకం, 0)->దత్తాంశం;
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
శూన్య *sqlite3_Hash_చొప్పించు(Hash *pH, స్థిరరాశి అక్షరం *p_కీలకం, శూన్య *దత్తాంశం){
  గుర్తులేని పూర్ణాంకం h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  ప్రతిజ్ఞా( pH!=0 );
  ప్రతిజ్ఞా( p_కీలకం!=0 );
  elem = కనుగొను_Element_With_Hash(pH,p_కీలకం,&h);
  ఒకవేళ( elem->దత్తాంశం ){
    శూన్య *old_దత్తాంశం = elem->దత్తాంశం;
    ఒకవేళ( దత్తాంశం==0 ){
      removeElement(pH,elem);
    }లేకపోతే{
      elem->దత్తాంశం = దత్తాంశం;
      elem->p_కీలకం = p_కీలకం;
    }
    తిరిగి old_దత్తాంశం;
  }
  ఒకవేళ( దత్తాంశం==0 ) తిరిగి 0;
  new_elem = (HashElem*)sqlite3Malloc( పరిమాణం(HashElem) );
  ఒకవేళ( new_elem==0 ) తిరిగి దత్తాంశం;
  new_elem->p_కీలకం = p_కీలకం;
  new_elem->h = h;
  new_elem->దత్తాంశం = దత్తాంశం;
  pH->లెక్క++;
  ఒకవేళ( pH->లెక్క>=5 && pH->లెక్క > 2*pH->htsize ){
    rehash(pH, pH->లెక్క*3);
  }
  చొప్పించు_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  తిరిగి 0;
}

