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
  यावत्( elem ){
    HashElem *अग्र_elem = elem->अग्र;
    sqlite3_free(elem);
    elem = अग्र_elem;
  }
  pH->गणनम् = 0;
}

/*
** The hashing function.
*/
स्थिर अचिह्नित पूर्णाङ्क strHash(ध्रुव वर्ण *z){
  अचिह्नित पूर्णाङ्क h = 0;
  यावत्( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#यदि_परिभाषितम् SQLITE_EBCDIC
    h += 0xbf & (अचिह्नित वर्ण)*(z++);
#अन्यथा
    h += 0xdf & (अचिह्नित वर्ण)*(z++);
#यदि_अन्तः
    h *= 0x9e3779b1;
  }
  प्रत्यागम h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
स्थिर शून्य निवेशय_Element(
  Hash *pH,              /* The complete hash table */
  संरचना _ht *p_प्रविष्टिः,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *p_शीर्ष;       /* First element already in pEntry */
  यदि( p_प्रविष्टिः ){
    p_शीर्ष = p_प्रविष्टिः->गणनम् ? p_प्रविष्टिः->chain : 0;
    p_प्रविष्टिः->गणनम्++;
    p_प्रविष्टिः->chain = pNew;
  }अन्यथा{
    p_शीर्ष = 0;
  }
  यदि( p_शीर्ष ){
    pNew->अग्र = p_शीर्ष;
    pNew->पूर्व = p_शीर्ष->पूर्व;
    यदि( p_शीर्ष->पूर्व ){ p_शीर्ष->पूर्व->अग्र = pNew; }
    अन्यथा             { pH->first = pNew; }
    p_शीर्ष->पूर्व = pNew;
  }अन्यथा{
    pNew->अग्र = pH->first;
    यदि( pH->first ){ pH->first->पूर्व = pNew; }
    pNew->पूर्व = 0;
    pH->first = pNew;
  }
}


/* Resize the hash table so that it contains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
स्थिर पूर्णाङ्क rehash(Hash *pH, अचिह्नित पूर्णाङ्क new_आकार){
  संरचना _ht *new_ht;            /* The new hash table */
  HashElem *elem, *अग्र_elem;    /* For looping over existing elements */

#यदि SQLITE_MALLOC_SOFT_सीमा>0
  यदि( new_आकार*परिमाण(संरचना _ht)>SQLITE_MALLOC_SOFT_सीमा ){
    new_आकार = SQLITE_MALLOC_SOFT_सीमा/परिमाण(संरचना _ht);
  }
  यदि( new_आकार==pH->htsize ) प्रत्यागम 0;
#यदि_अन्तः

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (संरचना _ht *)sqlite3Malloc( new_आकार*परिमाण(संरचना _ht) );
  sqlite3_अन्तः_Benign_Malloc();

  यदि( new_ht==0 ) प्रत्यागम 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_आकार = sqlite3_Malloc_आकार(new_ht)/परिमाण(संरचना _ht);
  स्मृतिं_पूरय(new_ht, 0, new_आकार*परिमाण(संरचना _ht));
  प्रति(elem=pH->first, pH->first=0; elem; elem = अग्र_elem){
    अग्र_elem = elem->अग्र;
    निवेशय_Element(pH, &new_ht[elem->h % new_आकार], elem);
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
  अचिह्नित पूर्णाङ्क *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  अचिह्नित पूर्णाङ्क गणनम्;            /* Number of elements left to test */
  अचिह्नित पूर्णाङ्क h;                /* The computed hash */
  स्थिर HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_कुञ्चिका);
  यदि( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    संरचना _ht *p_प्रविष्टिः;
    p_प्रविष्टिः = &pH->ht[h % pH->htsize];
    elem = p_प्रविष्टिः->chain;
    गणनम् = p_प्रविष्टिः->गणनम्;
  }अन्यथा{
    elem = pH->first;
    गणनम् = pH->गणनम्;
  }
  यदि( pHash ) *pHash = h;
  यावत्( गणनम् ){
    प्रतिज्ञा( elem!=0 );
    यदि( h==elem->h && sqlite3StrICmp(elem->p_कुञ्चिका,p_कुञ्चिका)==0 ){ 
      प्रत्यागम elem;
    }
    elem = elem->अग्र;
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
  संरचना _ht *p_प्रविष्टिः;
  यदि( elem->पूर्व ){
    elem->पूर्व->अग्र = elem->अग्र; 
  }अन्यथा{
    pH->first = elem->अग्र;
  }
  यदि( elem->अग्र ){
    elem->अग्र->पूर्व = elem->पूर्व;
  }
  यदि( pH->ht ){
    p_प्रविष्टिः = &pH->ht[elem->h % pH->htsize];
    यदि( p_प्रविष्टिः->chain==elem ){
      p_प्रविष्टिः->chain = elem->अग्र;
    }
    प्रतिज्ञा( p_प्रविष्टिः->गणनम्>0 );
    p_प्रविष्टिः->गणनम्--;
  }
  sqlite3_free( elem );
  pH->गणनम्--;
  यदि( pH->गणनम्==0 ){
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
  प्रत्यागम findElementWithHash(pH, p_कुञ्चिका, 0)->दत्त;
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
शून्य *sqlite3_Hash_निवेशय(Hash *pH, ध्रुव वर्ण *p_कुञ्चिका, शून्य *दत्त){
  अचिह्नित पूर्णाङ्क h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  प्रतिज्ञा( pH!=0 );
  प्रतिज्ञा( p_कुञ्चिका!=0 );
  elem = findElementWithHash(pH,p_कुञ्चिका,&h);
  यदि( elem->दत्त ){
    शून्य *old_दत्त = elem->दत्त;
    यदि( दत्त==0 ){
      removeElement(pH,elem);
    }अन्यथा{
      elem->दत्त = दत्त;
      elem->p_कुञ्चिका = p_कुञ्चिका;
    }
    प्रत्यागम old_दत्त;
  }
  यदि( दत्त==0 ) प्रत्यागम 0;
  new_elem = (HashElem*)sqlite3Malloc( परिमाण(HashElem) );
  यदि( new_elem==0 ) प्रत्यागम दत्त;
  new_elem->p_कुञ्चिका = p_कुञ्चिका;
  new_elem->h = h;
  new_elem->दत्त = दत्त;
  pH->गणनम्++;
  यदि( pH->गणनम्>=5 && pH->गणनम् > 2*pH->htsize ){
    rehash(pH, pH->गणनम्*3);
  }
  निवेशय_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  प्रत्यागम 0;
}

