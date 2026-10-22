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
#నిర్వచితం_కాకపోతే SQLITE_HASH_H
#నిర్వచించు SQLITE_HASH_H

/* Forward declarations of structures. */
రకనిర్వచనం నిర్మాణం Hash Hash;
రకనిర్వచనం నిర్మాణం HashElem HashElem;

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
నిర్మాణం Hash {
  గుర్తులేని పూర్ణాంకం htsize;      /* Number of buckets in the hash table */
  గుర్తులేని పూర్ణాంకం లెక్క;       /* Number of entries in this table */
  HashElem *మొదటి;          /* The first element of the array */
  నిర్మాణం _ht {              /* the hash table */
    గుర్తులేని పూర్ణాంకం లెక్క;        /* Number of entries with this hash */
    HashElem *chain;           /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
నిర్మాణం HashElem {
  HashElem *తదుపరి, *prev;       /* Next and previous elements in the table */
  శూన్య *దత్తాంశం;                  /* Data associated with this element */
  స్థిరరాశి అక్షరం *p_కీలకం;            /* Key associated with this element */
  గుర్తులేని పూర్ణాంకం h;              /* hash for pKey */
};

/*
** Access routines.  To delete, insert a NULL pointer.
*/
శూన్య sqlite3HashInit(Hash*);
శూన్య *sqlite3_Hash_చొప్పించు(Hash*, స్థిరరాశి అక్షరం *p_కీలకం, శూన్య *p_దత్తాంశం);
శూన్య *sqlite3_Hash_కనుగొను(స్థిరరాశి Hash*, స్థిరరాశి అక్షరం *p_కీలకం);
శూన్య sqlite3HashClear(Hash*);

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
#నిర్వచించు sqlite_Hash_మొదటి(H)  ((H)->మొదటి)
#నిర్వచించు sqlite_Hash_తదుపరి(E)   ((E)->తదుపరి)
#నిర్వచించు sqlite_Hash_దత్తాంశం(E)   ((E)->దత్తాంశం)
/* #define sqliteHashKey(E)    ((E)->pKey) // NOT USED */
/* #define sqliteHashKeysize(E) ((E)->nKey)  // NOT USED */

/*
** Number of entries in a hash table
*/
#నిర్వచించు sqlite_Hash_లెక్క(H)  ((H)->లెక్క)

#అయితే_ముగింపు /* SQLITE_HASH_H */
