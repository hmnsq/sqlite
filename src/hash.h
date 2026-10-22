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
#اگر_غیر_تعریف_شدہ SQLITE_HASH_H
#تعریف_کرو SQLITE_HASH_H

/* Forward declarations of structures. */
قسمتعریف ساخت Hash Hash;
قسمتعریف ساخت HashElem HashElem;

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
ساخت Hash {
  غیرعلامتی صحیح htsize;      /* Number of buckets in the hash table */
  غیرعلامتی صحیح گنتی;       /* Number of entries in this table */
  HashElem *پہلا;          /* The first element of the array */
  ساخت _ht {              /* the hash table */
    غیرعلامتی صحیح گنتی;        /* Number of entries with this hash */
    HashElem *chain;           /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
ساخت HashElem {
  HashElem *اگلا, *پچھلا;       /* Next and previous elements in the table */
  خالی *ڈیٹا;                  /* Data associated with this element */
  ثابت حرف *p_کلید;            /* Key associated with this element */
  غیرعلامتی صحیح h;              /* hash for pKey */
};

/*
** Access routines.  To delete, insert a NULL pointer.
*/
خالی sqlite3HashInit(Hash*);
خالی *sqlite3_Hash_داخل_کرو(Hash*, ثابت حرف *p_کلید, خالی *p_ڈیٹا);
خالی *sqlite3_Hash_ڈھونڈو(ثابت Hash*, ثابت حرف *p_کلید);
خالی sqlite3HashClear(Hash*);

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
#تعریف_کرو sqlite_Hash_پہلا(H)  ((H)->پہلا)
#تعریف_کرو sqlite_Hash_اگلا(E)   ((E)->اگلا)
#تعریف_کرو sqlite_Hash_ڈیٹا(E)   ((E)->ڈیٹا)
/* #define sqliteHashKey(E)    ((E)->pKey) // NOT USED */
/* #define sqliteHashKeysize(E) ((E)->nKey)  // NOT USED */

/*
** Number of entries in a hash table
*/
#تعریف_کرو sqlite_Hash_گنتی(H)  ((H)->گنتی)

#اگر_ختم /* SQLITE_HASH_H */
