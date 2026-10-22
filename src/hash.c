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
#समावेश "sqliteInt.h"
#समावेश <assert.h>

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
व्योम sqlite3HashInit(Hash *pNew){
  निश्चित( pNew!=0 );
  pNew->first = 0;
  pNew->गिनती = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
व्योम sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  निश्चित( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  जबतक( elem ){
    HashElem *अग्र_elem = elem->अग्र;
    sqlite3_free(elem);
    elem = अग्र_elem;
  }
  pH->गिनती = 0;
}

/*
** The hashing function.
*/
अटल अचिन्हित पूर्णांक strHash(स्थिर अक्षर *z){
  अचिन्हित पूर्णांक h = 0;
  जबतक( z[0] ){     /*OPTIMIZATION-IF-TRUE*/
    /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
    ** 0x9e3779b1 is 2654435761 which is the closest prime number to
    ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2.
    **
    ** Only bits 0xdf for ASCII and bits 0xbf for EBCDIC each octet are
    ** hashed since the omitted bits determine the upper/lower case difference.
    */
#अगर_घोषित SQLITE_EBCDIC
    h += 0xbf & (अचिन्हित अक्षर)*(z++);
#अन्यथा
    h += 0xdf & (अचिन्हित अक्षर)*(z++);
#पूर्ण_अगर
    h *= 0x9e3779b1;
  }
  वापस h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
अटल व्योम जोड़ो_Element(
  Hash *pH,              /* The complete hash table */
  काष्ठा _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *p_शीर्ष;       /* First element already in pEntry */
  अगर( pEntry ){
    p_शीर्ष = pEntry->गिनती ? pEntry->chain : 0;
    pEntry->गिनती++;
    pEntry->chain = pNew;
  }अन्यथा{
    p_शीर्ष = 0;
  }
  अगर( p_शीर्ष ){
    pNew->अग्र = p_शीर्ष;
    pNew->पूर्व = p_शीर्ष->पूर्व;
    अगर( p_शीर्ष->पूर्व ){ p_शीर्ष->पूर्व->अग्र = pNew; }
    अन्यथा             { pH->first = pNew; }
    p_शीर्ष->पूर्व = pNew;
  }अन्यथा{
    pNew->अग्र = pH->first;
    अगर( pH->first ){ pH->first->पूर्व = pNew; }
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
अटल पूर्णांक rehash(Hash *pH, अचिन्हित पूर्णांक new_आकार){
  काष्ठा _ht *new_ht;            /* The new hash table */
  HashElem *elem, *अग्र_elem;    /* For looping over existing elements */

#अगर SQLITE_MALLOC_SOFT_LIMIT>0
  अगर( new_आकार*माप(काष्ठा _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_आकार = SQLITE_MALLOC_SOFT_LIMIT/माप(काष्ठा _ht);
  }
  अगर( new_आकार==pH->htsize ) वापस 0;
#पूर्ण_अगर

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (काष्ठा _ht *)sqlite3Malloc( new_आकार*माप(काष्ठा _ht) );
  sqlite3EndBenignMalloc();

  अगर( new_ht==0 ) वापस 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_आकार = sqlite3_Malloc_आकार(new_ht)/माप(काष्ठा _ht);
  स_रखो(new_ht, 0, new_आकार*माप(काष्ठा _ht));
  क्रम(elem=pH->first, pH->first=0; elem; elem = अग्र_elem){
    अग्र_elem = elem->अग्र;
    जोड़ो_Element(pH, &new_ht[elem->h % new_आकार], elem);
  }
  वापस 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  If no element is found,
** a pointer to a static null element with HashElem.data==0 is returned.
** If pH is not NULL, then the hash for this key is written to *pH.
*/
अटल HashElem *findElementWithHash(
  स्थिर Hash *pH,     /* The pH to be searched */
  स्थिर अक्षर *p_कुंजी,   /* The key we are searching for */
  अचिन्हित पूर्णांक *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  अचिन्हित पूर्णांक गिनती;            /* Number of elements left to test */
  अचिन्हित पूर्णांक h;                /* The computed hash */
  अटल HashElem nullElement = { 0, 0, 0, 0, 0 };

  h = strHash(p_कुंजी);
  अगर( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    काष्ठा _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    गिनती = pEntry->गिनती;
  }अन्यथा{
    elem = pH->first;
    गिनती = pH->गिनती;
  }
  अगर( pHash ) *pHash = h;
  जबतक( गिनती ){
    निश्चित( elem!=0 );
    अगर( h==elem->h && sqlite3StrICmp(elem->p_कुंजी,p_कुंजी)==0 ){ 
      वापस elem;
    }
    elem = elem->अग्र;
    गिनती--;
  }
  वापस &nullElement;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
अटल व्योम removeElement(
  Hash *pH,         /* The pH containing "elem" */
  HashElem *elem    /* The element to be removed from the pH */
){
  काष्ठा _ht *pEntry;
  अगर( elem->पूर्व ){
    elem->पूर्व->अग्र = elem->अग्र; 
  }अन्यथा{
    pH->first = elem->अग्र;
  }
  अगर( elem->अग्र ){
    elem->अग्र->पूर्व = elem->पूर्व;
  }
  अगर( pH->ht ){
    pEntry = &pH->ht[elem->h % pH->htsize];
    अगर( pEntry->chain==elem ){
      pEntry->chain = elem->अग्र;
    }
    निश्चित( pEntry->गिनती>0 );
    pEntry->गिनती--;
  }
  sqlite3_free( elem );
  pH->गिनती--;
  अगर( pH->गिनती==0 ){
    निश्चित( pH->first==0 );
    निश्चित( pH->गिनती==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
व्योम *sqlite3HashFind(स्थिर Hash *pH, स्थिर अक्षर *p_कुंजी){
  निश्चित( pH!=0 );
  निश्चित( p_कुंजी!=0 );
  वापस findElementWithHash(pH, p_कुंजी, 0)->तथ्य;
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
व्योम *sqlite3_Hash_जोड़ो(Hash *pH, स्थिर अक्षर *p_कुंजी, व्योम *तथ्य){
  अचिन्हित पूर्णांक h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  निश्चित( pH!=0 );
  निश्चित( p_कुंजी!=0 );
  elem = findElementWithHash(pH,p_कुंजी,&h);
  अगर( elem->तथ्य ){
    व्योम *old_तथ्य = elem->तथ्य;
    अगर( तथ्य==0 ){
      removeElement(pH,elem);
    }अन्यथा{
      elem->तथ्य = तथ्य;
      elem->p_कुंजी = p_कुंजी;
    }
    वापस old_तथ्य;
  }
  अगर( तथ्य==0 ) वापस 0;
  new_elem = (HashElem*)sqlite3Malloc( माप(HashElem) );
  अगर( new_elem==0 ) वापस तथ्य;
  new_elem->p_कुंजी = p_कुंजी;
  new_elem->h = h;
  new_elem->तथ्य = तथ्य;
  pH->गिनती++;
  अगर( pH->गिनती>=5 && pH->गिनती > 2*pH->htsize ){
    rehash(pH, pH->गिनती*3);
  }
  जोड़ो_Element(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  वापस 0;
}

