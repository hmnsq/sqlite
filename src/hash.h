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
** This is the header file for the generic hash-table implementation
** used in SQLite.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_HASH_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_HASH_H

/* Forward declarations of structures. */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Hash Hash;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ HashElem HashElem;

/* A complete hash table is an instance of the following structure.
** The internals of this structure are intended to be opaque -- client
** code should not attempt to access or modify the fields of this structure
** directly.  Change this structure only by using the routines below.
** However, some of the "procedures" and "functions" for modifying and
** accessing this structure are really macros, so we can't really make
** this structure opaque.
**
** All elements of the hash table are on a single doubly-linked list.
** Hash.first points to the head of this list.
**
** There are Hash.htsize buckets.  Each bucket points to a spot in
** the global doubly-linked list.  The contents of the bucket are the
** element pointed to plus the next _ht.count-1 elements in the list.
**
** Hash.htsize and Hash.ht may be zero.  In that case lookup is done
** by a linear search of the global list.  For small tables, the 
** Hash.ht table is never allocated because if there are few elements
** in the table, it is faster to do a linear search than to manage
** the hash table.
*/
ਬਣਤਰ Hash {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ htsize;      /* Number of buckets in the hash table */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਗਿਣਤੀ_2;       /* Number of entries in this table */
  HashElem *ਪਹਿਲਾ;          /* The first element of the array */
  ਬਣਤਰ _ht {              /* the hash table */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਗਿਣਤੀ_2;        /* Number of entries with this hash */
    HashElem *chain;           /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
ਬਣਤਰ HashElem {
  HashElem *ਅਗਲਾ, *prev;       /* Next and previous elements in the table */
  ਖਾਲੀ *ਅੰਕੜੇ;                  /* Data associated with this element */
  ਅਚਲ ਅੱਖਰ *p_ਕੁੰਜੀ;            /* Key associated with this element */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h;              /* hash for pKey */
};

/*
** Access routines.  To delete, insert a NULL pointer.
*/
ਖਾਲੀ sqlite3HashInit(Hash*);
ਖਾਲੀ *sqlite3_Hash_ਪਾਓ(Hash*, ਅਚਲ ਅੱਖਰ *p_ਕੁੰਜੀ, ਖਾਲੀ *p_ਅੰਕੜੇ);
ਖਾਲੀ *sqlite3_Hash_ਲੱਭੋ(ਅਚਲ Hash*, ਅਚਲ ਅੱਖਰ *p_ਕੁੰਜੀ);
ਖਾਲੀ sqlite3HashClear(Hash*);

/*
** Macros for looping over all elements of a hash table.  The idiom is
** like this:
**
**   Hash h;
**   HashElem *p;
**   ...
**   for(p=sqliteHashFirst(&h); p; p=sqliteHashNext(p)){
**     SomeStructure *pData = sqliteHashData(p);
**     // do something with pData
**   }
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite_Hash_ਪਹਿਲਾ(H)  ((H)->ਪਹਿਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite_Hash_ਅਗਲਾ(E)   ((E)->ਅਗਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite_Hash_ਅੰਕੜੇ(E)   ((E)->ਅੰਕੜੇ)
/* #define sqliteHashKey(E)    ((E)->pKey) // NOT USED */
/* #define sqliteHashKeysize(E) ((E)->nKey)  // NOT USED */

/*
** Number of entries in a hash table
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite_Hash_ਗਿਣਤੀ(H)  ((H)->ਗਿਣਤੀ_2)

#ਜੇ_ਅੰਤ /* SQLITE_HASH_H */
