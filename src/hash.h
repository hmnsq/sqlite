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
#که_نه_تعریف_شوی SQLITE_HASH_H
#تعریف_کړه SQLITE_HASH_H

/* Forward declarations of structures. */
تعريفنوع بنية Hash Hash;
تعريفنوع بنية HashElem HashElem;

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
بنية Hash {
  بینښه صحيح htsize;      /* Number of buckets in the hash table */
  بینښه صحيح شمېر;       /* Number of entries in this table */
  HashElem *لومړی;          /* The first element of the array */
  بنية _ht {              /* the hash table */
    بینښه صحيح شمېر;        /* Number of entries with this hash */
    HashElem *chain;           /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
بنية HashElem {
  HashElem *بل, *prev;       /* Next and previous elements in the table */
  فراغ *معلومات;                  /* Data associated with this element */
  ثابت حرف *p_کلی;            /* Key associated with this element */
  بینښه صحيح h;              /* hash for pKey */
};

/*
** Access routines.  To delete, insert a NULL pointer.
*/
فراغ sqlite3HashInit(Hash*);
فراغ *sqlite3_Hash_ورننباسه(Hash*, ثابت حرف *p_کلی, فراغ *p_معلومات);
فراغ *sqlite3_Hash_پیدا_کړه(ثابت Hash*, ثابت حرف *p_کلی);
فراغ sqlite3HashClear(Hash*);

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
#تعریف_کړه sqlite_Hash_لومړی(H)  ((H)->لومړی)
#تعریف_کړه sqlite_Hash_بل(E)   ((E)->بل)
#تعریف_کړه sqlite_Hash_معلومات(E)   ((E)->معلومات)
/* #define sqliteHashKey(E)    ((E)->pKey) // NOT USED */
/* #define sqliteHashKeysize(E) ((E)->nKey)  // NOT USED */

/*
** Number of entries in a hash table
*/
#تعریف_کړه sqlite_Hash_شمېر(H)  ((H)->شمېر)

#د_که_پای /* SQLITE_HASH_H */
