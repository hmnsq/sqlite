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
#ifndef SQLITE_HASH_H
#define SQLITE_HASH_H

/* Forward declarations of structures. */
প্রকারলেখ কাঠামো Hash Hash;
প্রকারলেখ কাঠামো HashElem HashElem;

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
কাঠামো Hash {
  unsigned পূর্ণসংখ্যা htsize;      /* Number of buckets in the hash table */
  unsigned পূর্ণসংখ্যা গুনতি;       /* Number of entries in this table */
  HashElem *first;          /* The first element of the array */
  কাঠামো _ht {              /* the hash table */
    unsigned পূর্ণসংখ্যা গুনতি;        /* Number of entries with this hash */
    HashElem *chain;           /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
কাঠামো HashElem {
  HashElem *পরবর্তী, *পূর্ব;       /* Next and previous elements in the table */
  শূন্য *তথ্য;                  /* Data associated with this element */
  ধ্রুবক অক্ষর *p_চাবি;            /* Key associated with this element */
  unsigned পূর্ণসংখ্যা h;              /* hash for pKey */
};

/*
** Access routines.  To delete, insert a NULL pointer.
*/
শূন্য sqlite3HashInit(Hash*);
শূন্য *sqlite3_Hash_ঢোকাও(Hash*, ধ্রুবক অক্ষর *p_চাবি, শূন্য *p_তথ্য);
শূন্য *sqlite3HashFind(ধ্রুবক Hash*, ধ্রুবক অক্ষর *p_চাবি);
শূন্য sqlite3HashClear(Hash*);

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
#define sqliteHashFirst(H)  ((H)->first)
#define sqlite_Hash_পরবর্তী(E)   ((E)->পরবর্তী)
#define sqlite_Hash_তথ্য(E)   ((E)->তথ্য)
/* #define sqliteHashKey(E)    ((E)->pKey) // NOT USED */
/* #define sqliteHashKeysize(E) ((E)->nKey)  // NOT USED */

/*
** Number of entries in a hash table
*/
#define sqlite_Hash_গুনতি(H)  ((H)->গুনতি)

#endif /* SQLITE_HASH_H */
