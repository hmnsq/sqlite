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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_HASH_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_HASH_H

/* Forward declarations of structures. */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Hash Hash;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ HashElem HashElem;

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
ರಚನೆ Hash {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ htsize;      /* Number of buckets in the hash table */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;       /* Number of entries in this table */
  HashElem *ಮೊದಲ;          /* The first element of the array */
  ರಚನೆ _ht {              /* the hash table */
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;        /* Number of entries with this hash */
    HashElem *chain;           /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
ರಚನೆ HashElem {
  HashElem *ಮುಂದಿನ, *prev;       /* Next and previous elements in the table */
  ಶೂನ್ಯ *ದತ್ತಾಂಶ;                  /* Data associated with this element */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_ಕೀಲಿ;            /* Key associated with this element */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h;              /* hash for pKey */
};

/*
** Access routines.  To delete, insert a NULL pointer.
*/
ಶೂನ್ಯ sqlite3HashInit(Hash*);
ಶೂನ್ಯ *sqlite3_Hash_ಸೇರಿಸು(Hash*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_ಕೀಲಿ, ಶೂನ್ಯ *p_ದತ್ತಾಂಶ);
ಶೂನ್ಯ *sqlite3_Hash_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ Hash*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_ಕೀಲಿ);
ಶೂನ್ಯ sqlite3HashClear(Hash*);

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
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite_Hash_ಮೊದಲ(H)  ((H)->ಮೊದಲ)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite_Hash_ಮುಂದಿನ(E)   ((E)->ಮುಂದಿನ)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite_Hash_ದತ್ತಾಂಶ(E)   ((E)->ದತ್ತಾಂಶ)
/* #define sqliteHashKey(E)    ((E)->pKey) // NOT USED */
/* #define sqliteHashKeysize(E) ((E)->nKey)  // NOT USED */

/*
** Number of entries in a hash table
*/
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite_Hash_ಎಣಿಕೆ(H)  ((H)->ಎಣಿಕೆ_2)

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_HASH_H */
