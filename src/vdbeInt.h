/*
** 2003 September 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This is the header file for information that is private to the
** VDBE.  This information used to all be at the top of the single
** source code file "vdbe.c".  When that file became too big (over
** 6000 lines long) it was split up into several smaller files and
** this header information was factored out.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_VDBEINT_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_VDBEINT_H

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 50
#ಆದರೆ_ಅಂತ್ಯ

/*
** VDBE_DISPLAY_P4 is true or false depending on whether or not the
** "explain" P4 display logic is enabled.
*/
#ಆದರೆ !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#ಇಲ್ಲದಿದ್ದರೆ
# define VDBE_DISPLAY_P4 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VdbeOp Op;

/*
** Boolean values
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ Bool;

/* Opaque type used by code in vdbesort.c */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Aux_ದತ್ತಾಂಶ Aux_ದತ್ತಾಂಶ;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VdbeTxtBlbCache VdbeTxtBlbCache;

/* Types of VDBE cursors */
#ವ್ಯಾಖ್ಯಾನಿಸು CURTYPE_BTREE       0
#ವ್ಯಾಖ್ಯಾನಿಸು CURTYPE_SORTER      1
#ವ್ಯಾಖ್ಯಾನಿಸು CURTYPE_VTAB        2
#ವ್ಯಾಖ್ಯಾನಿಸು CURTYPE_PSEUDO      3

/*
** A VdbeCursor is an superclass (a wrapper) for various cursor objects:
**
**      * A b-tree cursor
**          -  In the main database or in an ephemeral database
**          -  On either an index or a table
**      * A sorter
**      * A virtual table
**      * A one-row "pseudotable" stored in a single register
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VdbeCursor VdbeCursor;
ರಚನೆ VdbeCursor {
  u8 eCurType;            /* One of the CURTYPE_* values above */
  i8 iDb;                 /* Index of cursor database in db->aDb[] */
  u8 null_ಅಡ್ಡಸಾಲು;             /* True if pointing to a row with no data */
  u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
  u8 is_ಕೋಷ್ಟಕ;             /* True for rowid tables.  False for indexes */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#ಆದರೆ_ಅಂತ್ಯ
  Bool isEphemeral:1;     /* True for an ephemeral table */
  Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
  Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
  Bool noReuse:1;         /* OpenEphemeral may not reuse this cursor */
  Bool ಲಂಬಸಾಲು_Cache:1;        /* pCache pointer is initialized and non-NULL */
  u16 seek_ದೊರಕಿತು;            /* See the OP_SeekHit and OP_IfNoHope opcodes */
  ಒಕ್ಕೂಟ {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seq_ಎಣಿಕೆ;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cache_ಸ್ಥಿತಿ;        /* Cache is valid if this matches Vdbe.cacheCtr */
  ಪೂರ್ಣಾಂಕ seek_ಫಲಿತಾಂಶ;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match".
  ** For CURTYPE_PSEUDO, seekResult is the register holding the record */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  ಒಕ್ಕೂಟ {
    BtCursor *pCursor;          /* CURTYPE_BTREE or _PSEUDO.  Btree cursor */
    sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.              Vtab cursor */
    VdbeSorter *pSorter;        /* CURTYPE_SORTER.            Sorter object */
  } uc;
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;      /* Info about index keys needed by index cursors */
  u32 i_Hdr_ಸ್ಥಾನಾಂತರ;         /* Offset to next unparsed byte of the header */
  Pgno pgno_ಬೇರು;          /* Root page of the open btree cursor */
  i16 n_ಕ್ಷೇತ್ರ;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 moveto_ಗುರಿ;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *a_ಸ್ಥಾನಾಂತರ;           /* Pointer to aType[nField] */
  ಸ್ಥಿರಾಂಕ u8 *a_ಅಡ್ಡಸಾಲು;         /* Data for the current row, if all on one page */
  u32 payload_ಗಾತ್ರ;        /* Total number of bytes in the record */
  u32 sz_ಅಡ್ಡಸಾಲು;              /* Byte available in aRow */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_ಬಳಸಿದ_ಮುಖವಾಡ
  u64 ಮುಖವಾಡ_ಬಳಸಿದ;           /* Mask of columns used by this cursor */
#ಆದರೆ_ಅಂತ್ಯ
  VdbeTxtBlbCache *pCache; /* Cache of large TEXT or BLOB values */

  /* Space is allocated for aType to hold at least 2*nField+1 entries:
  ** nField slots for aType[] and nField+1 array slots for aOffset[] */
  u32 aType[FLEXARRAY];    /* Type values record decode.  MUST BE LAST */
};

/*
** The size (in bytes) of a VdbeCursor object that has an nField value of N
** or less.  The value of SZ_VDBECURSOR(n) is guaranteed to be a multiple
** of 8.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_VDBECURSOR(N) \
    (ROUND8(offsetof(VdbeCursor,aType)) + ((N)+1)*ಗಾತ್ರ(u64))

/* Return true if P is a null-only cursor
*/
#ವ್ಯಾಖ್ಯಾನಿಸು IsNullCursor(P) \
  ((P)->eCurType==CURTYPE_PSEUDO && (P)->null_ಅಡ್ಡಸಾಲು && (P)->seek_ಫಲಿತಾಂಶ==0)

/*
** A value for VdbeCursor.cacheStatus that means the cache is always invalid.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು CACHE_STALE 0

/*
** Large TEXT or BLOB values can be slow to load, so we want to avoid
** loading them more than once.  For that reason, large TEXT and BLOB values
** can be stored in a cache defined by this object, and attached to the
** VdbeCursor using the pCache field.
*/
ರಚನೆ VdbeTxtBlbCache {
  ಅಕ್ಷರ *pCValue;        /* A RCStr buffer to hold the value */
  i64 i_ಸ್ಥಾನಾಂತರ;          /* File offset of the row being cached */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;             /* Column for which the cache is valid */
  u32 cache_ಸ್ಥಿತಿ;      /* Vdbe.cacheCtr value */
  u32 ಲಂಬಸಾಲು_Cache_Ctr;      /* Column cache counter */
};

/*
** When a sub-program is executed (OP_Program), a structure of this type
** is allocated to store the current value of the program counter, as
** well as the current memory cell array and various other frame specific
** values stored in the Vdbe struct. When the sub-program is finished,
** these values are copied back to the Vdbe from the VdbeFrame structure,
** restoring the state of the VM to as it was before the sub-program
** began executing.
**
** The memory for a VdbeFrame object is allocated and managed by a memory
** cell in the parent (calling) frame. When the memory cell is deleted or
** overwritten, the VdbeFrame object is not freed immediately. Instead, it
** is linked into the Vdbe.pDelFrame list. The contents of the Vdbe.pDelFrame
** list is deleted when the VM is reset in VdbeHalt(). The reason for doing
** this instead of deleting the VdbeFrame immediately is to avoid recursive
** calls to sqlite3VdbeMemRelease() when the memory cells belonging to the
** child frame are released.
**
** The currently executing frame is stored in Vdbe.pFrame. Vdbe.pFrame is
** set to NULL if the currently executing frame is the main program.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Vdbe_ಚೌಕಟ್ಟು Vdbe_ಚೌಕಟ್ಟು;
ರಚನೆ Vdbe_ಚೌಕಟ್ಟು {
  Vdbe *v;                /* VM this frame belongs to */
  Vdbe_ಚೌಕಟ್ಟು *p_ಪೋಷಕ;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  ಶೂನ್ಯ *token;            /* Copy of SubProgram.token */
  i64 ಕೊನೆಯ_Rowid;          /* Last insert rowid (sqlite3.lastRowid) */
  Aux_ದತ್ತಾಂಶ *p_Aux_ದತ್ತಾಂಶ;      /* Linked list of auxdata allocations */
#ಆದರೆ SQLITE_DEBUG
  u32 i_ಚೌಕಟ್ಟು_Magic;        /* magic number for sanity checking */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ nCursor;            /* Number of entries in apCsr */
  ಪೂರ್ಣಾಂಕ pc;                 /* Program Counter in parent (calling) frame */
  ಪೂರ್ಣಾಂಕ nOp;                /* Size of aOp array */
  ಪೂರ್ಣಾಂಕ nMem;               /* Number of entries in aMem */
  ಪೂರ್ಣಾಂಕ n_ಮಗು_Mem;          /* Number of memory cells for child frame */
  ಪೂರ್ಣಾಂಕ n_ಮಗು_Csr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಚೌಕಟ್ಟು_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು Vdbe_ಚೌಕಟ್ಟು_Mem(p) ((Mem *)&((u8 *)p)[ROUND8(ಗಾತ್ರ(Vdbe_ಚೌಕಟ್ಟು))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
ರಚನೆ sqlite3_ಮೌಲ್ಯ {
  ಒಕ್ಕೂಟ Mem_ಮೌಲ್ಯ {
    ದ್ವಿಗುಣ r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    ಪೂರ್ಣಾಂಕ nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  ಅಕ್ಷರ *z;            /* String or BLOB value */
  ಪೂರ್ಣಾಂಕ n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  ಪೂರ್ಣಾಂಕ szMalloc;       /* Size of the zMalloc allocation */
  u32 u_ತಾತ್ಕಾಲಿಕ;          /* Transient storage for serial_type in OP_MakeRecord */
  ಅಕ್ಷರ *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  u16 mScopyFlags;    /* flags value immediately after the shallow copy */
  u8  bScopy;         /* The pScopyFrom of some other Mem *might* point here */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** Size of struct Mem not including the Mem.zMalloc member or anything that
** follows.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MEMCELLSIZE offsetof(Mem,db)

/* One or more of the following flags are set to indicate the
** representations of the value stored in the Mem struct.
**
**  *  MEM_Null                An SQL NULL value
**
**  *  MEM_Null|MEM_Zero       An SQL NULL with the virtual table
**                             UPDATE no-change flag set
**
**  *  MEM_Null|MEM_Term|      An SQL NULL, but also contains a
**        MEM_Subtype          pointer accessible using
**                             sqlite3_value_pointer().
**
**  *  MEM_Null|MEM_Cleared    Special SQL NULL that compares non-equal
**                             to other NULLs even using the IS operator.
**
**  *  MEM_Str                 A string, stored in Mem.z with
**                             length Mem.n.  Zero-terminated if
**                             MEM_Term is set.  This flag is
**                             incompatible with MEM_Blob and
**                             MEM_Null, but can appear with MEM_Int,
**                             MEM_Real, and MEM_IntReal.
**
**  *  MEM_Blob                A blob, stored in Mem.z length Mem.n.
**                             Incompatible with MEM_Str, MEM_Null,
**                             MEM_Int, MEM_Real, and MEM_IntReal.
**
**  *  MEM_Blob|MEM_Zero       A blob in Mem.z of length Mem.n plus
**                             Mem.u.nZero extra 0x00 bytes at the end.
**
**  *  MEM_Int                 Integer stored in Mem.u.i.
**
**  *  MEM_Real                Real stored in Mem.u.r.
**
**  *  MEM_IntReal             Real stored as an integer in Mem.u.i.
**
** If the MEM_Null flag is set, then the value is an SQL NULL value.
** For a pointer type created using sqlite3_bind_pointer() or
** sqlite3_result_pointer() the MEM_Term and MEM_Subtype flags are also set.
**
** If the MEM_Str flag is set then Mem.z points at a string representation.
** Usually this is encoded in the same unicode encoding as the main
** database (see below for exceptions). If the MEM_Term flag is also
** set, then the string is nul terminated. The MEM_Int and MEM_Real
** flags may coexist with the MEM_Str flag.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Undefined 0x0000   /* Value is undefined */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Null      0x0001   /* Value is NULL (or a pointer) */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Str       0x0002   /* Value is a string */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Int       0x0004   /* Value is an integer */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Real      0x0008   /* Value is a real number */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Blob      0x0010   /* Value is a BLOB */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_IntReal   0x0020   /* MEM_Int that stringifies like MEM_Real */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Aff_ಮುಖವಾಡ   0x003f   /* Mask of affinity bits */

/* Extra bits that modify the meanings of the core datatypes above
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_FromBind  0x0040   /* Value originates from sqlite3_bind() */
 /*                   0x0080   // Available */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Cleared   0x0100   /* NULL set by OP_Null, not from data */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_ಪದ      0x0200   /* String in Mem.z is zero terminated */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Zero      0x0400   /* Mem.i contains count of 0s appended to blob */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Subtype   0x0800   /* Mem.eSubtype is valid */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Type_ಮುಖವಾಡ  0x0dbf   /* Mask of type bits */

/* Bits that determine the storage for Mem.z for a string or blob or
** aggregate accumulator.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Dyn       0x1000   /* Need to call Mem.xDel() on Mem.z */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Static    0x2000   /* Mem.z points to a static string */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Ephem     0x4000   /* Mem.z points to an ephemeral string */
#ವ್ಯಾಖ್ಯಾನಿಸು MEM_Agg       0x8000   /* Mem.z points to an agg function context */

/* Return TRUE if Mem X contains dynamically allocated content - anything
** that needs to be deallocated to avoid a leak.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು VdbeMemDynamic(X)  \
  (((X)->flags&(MEM_Agg|MEM_Dyn))!=0)

/*
** Clear any existing type flags from a Mem and replace them with f
*/
#ವ್ಯಾಖ್ಯಾನಿಸು Mem_ಗಣ_Type_Flag(p, f) \
   ((p)->flags = ((p)->flags&~(MEM_Type_ಮುಖವಾಡ|MEM_Zero))|f)

/*
** True if Mem X is a NULL-nochng type.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MemNullNochng(X) \
  (((X)->flags&MEM_Type_ಮುಖವಾಡ)==(MEM_Null|MEM_Zero) \
    && (X)->n==0 && (X)->u.nZero==0)

/*
** Return true if a memory cell has been initialized and is valid.
** is for use inside assert() statements only.
**
** A Memory cell is initialized if at least one of the
** MEM_Null, MEM_Str, MEM_Int, MEM_Real, MEM_Blob, or MEM_IntReal bits
** is set.  It is "undefined" if all those bits are zero.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
#ವ್ಯಾಖ್ಯಾನಿಸು memIsValid(M)  ((M)->flags & MEM_Aff_ಮುಖವಾಡ)!=0
#ಆದರೆ_ಅಂತ್ಯ

/*
** Each auxiliary data pointer stored by a user defined function
** implementation calling sqlite3_set_auxdata() is stored in an instance
** of this structure. All such structures associated with a single VM
** are stored in a linked list headed at Vdbe.pAuxData. All are destroyed
** when the VM is halted (if not before).
*/
ರಚನೆ Aux_ದತ್ತಾಂಶ {
  ಪೂರ್ಣಾಂಕ iAuxOp;                     /* Instruction number of OP_Function opcode */
  ಪೂರ್ಣಾಂಕ iAuxArg;                    /* Index of function argument. */
  ಶೂನ್ಯ *pAux;                     /* Aux data pointer */
  ಶೂನ್ಯ (*xDeleteAux)(ಶೂನ್ಯ*);      /* Destructor for the aux data */
  Aux_ದತ್ತಾಂಶ *p_ಮುಂದಿನ_Aux;              /* Next element in list */
};

/*
** The "context" argument for an installable function.  A pointer to an
** instance of this structure is the first argument to the routines used
** implement the SQL functions.
**
** There is a typedef for this structure in sqlite.h.  So all routines,
** even the public interface to SQLite, can use a pointer to this structure.
** But this file is the only place where the internal details of this
** structure are known.
**
** This structure is defined inside of vdbeInt.h because it uses substructures
** (Mem) which are only defined there.
*/
ರಚನೆ sqlite3_context {
  Mem *p_ಹೊರಹೋಗು;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  ಪೂರ್ಣಾಂಕ iOp;                /* Instruction number of OP_Function */
  ಪೂರ್ಣಾಂಕ isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_ಮೌಲ್ಯ *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_CONTEXT(N)  \
   (offsetof(sqlite3_context,argv)+(N)*ಗಾತ್ರ(sqlite3_ಮೌಲ್ಯ*))


/* The ScanStatus object holds a single value for the
** sqlite3_stmt_scanstatus() interface.
**
** aAddrRange[]:
**   This array is used by ScanStatus elements associated with EQP
**   notes that make an SQLITE_SCANSTAT_NCYCLE value available. It is
**   an array of up to 3 ranges of VM addresses for which the Vdbe.anCycle[]
**   values should be summed to calculate the NCYCLE value. Each pair of
**   integer addresses is a start and end address (both inclusive) for a range
**   instructions. A start value of 0 indicates an empty range.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ;
ರಚನೆ ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ {
  ಪೂರ್ಣಾಂಕ addrExplain;                /* OP_Explain for loop */
  ಪೂರ್ಣಾಂಕ aAddrRange[6];
  ಪೂರ್ಣಾಂಕ addrLoop;                   /* Address of "loops" counter */
  ಪೂರ್ಣಾಂಕ addr_ಭೇಟಿ;                  /* Address of "rows visited" counter */
  ಪೂರ್ಣಾಂಕ iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  ಅಕ್ಷರ *zName;                    /* Name of table or index */
};

/* The DblquoteStr object holds the text of a double-quoted
** string for a prepared statement.  A linked list of these objects
** is constructed during statement parsing and is held on Vdbe.pDblStr.
** When computing a normalized SQL statement for an SQL statement, that
** list is consulted for each double-quoted identifier to see if the
** identifier should really be a string literal.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ DblquoteStr DblquoteStr;
ರಚನೆ DblquoteStr {
  DblquoteStr *p_ಮುಂದಿನ_Str;   /* Next string literal in the list */
  ಅಕ್ಷರ z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
ರಚನೆ Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  ಪೂರ್ಣಾಂಕ nMem;               /* Number of memory locations currently allocated */
  ಪೂರ್ಣಾಂಕ nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  ಪೂರ್ಣಾಂಕ pc;                 /* The program counter */
  ಪೂರ್ಣಾಂಕ rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  ಪೂರ್ಣಾಂಕ iStatement;         /* Statement number (or 0 if has no opened stmt) */
  i64 i_Current_ಸಮಯ;       /* Value of julianday('now') for this statement */
  i64 nFkConstraint;      /* Number of imm. FK constraints this VM */
  i64 nStmtDefCons;       /* Number of def. constraints when stmt started */
  i64 nStmtDefImmCons;    /* Number of def. imm constraints when stmt started */
  Mem *aMem;              /* The memory locations */
  Mem **apArg;            /* Arguments xUpdate and xFilter vtab methods */
  VdbeCursor **apCsr;     /* One element of this array for each open cursor */
  Mem *aVar;              /* Values for the OP_Variable opcode. */

  /* When allocating a new Vdbe object, all of the fields below should be
  ** initialized to zero or NULL */

  Op *aOp;                /* Space to hold the virtual machine's program */
  ಪೂರ್ಣಾಂಕ nOp;                /* Number of instructions in the program */
  ಪೂರ್ಣಾಂಕ nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *a_ಲಂಬಸಾಲು_Name;          /* Column names to return */
  Mem *p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು;        /* Current output row */
  ಅಕ್ಷರ *z_Err_ಸಂದೇಶ;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
  i64 ಆರಂಭ_ಸಮಯ;          /* Time when query started - used for profiling */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪೂರ್ಣಾಂಕ rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  ಪೂರ್ಣಾಂಕ napArg;             /* Size of the apArg[] array */
#ಆದರೆ_ಅಂತ್ಯ
  u16 n_Res_ಸ್ತಂಭ;         /* Number of columns in one row of the result set */
  u16 nResAlloc;          /* Column slots allocated to aColName[] */
  u8 errorAction;         /* Recovery action to do in case of an error */
  u8 minWriteFileFormat;  /* Minimum file format for writable database files */
  u8 prepFlags;           /* SQLITE_PREPARE_* flags */
  u8 eVdbeState;          /* On of the VDBE_*_STATE values */
  bft expired:2;          /* 1: recompile VM immediately  2: when convenient */
  bft explain:2;          /* 0: normal, 1: EXPLAIN, 2: EXPLAIN QUERY PLAN */
  bft changeCntOn:1;      /* True to update the change-counter */
  bft ಬಳಸುತ್ತದೆ_Stmt_Journal:1;  /* True if uses a statement journal */
  bft readOnly:1;         /* True for statements that do not write */
  bft bIsReader:1;        /* True for statements that read */
  bft haveEqpOps:1;       /* Bytecode supports EXPLAIN QUERY PLAN */
  y_Db_ಮುಖವಾಡ btree_ಮುಖವಾಡ;      /* Bitmask of db->aDb[] entries referenced */
  y_Db_ಮುಖವಾಡ ಬೀಗ_ಮುಖವಾಡ;       /* Subset of btreeMask that requires a lock */
  u32 aCounter[9];        /* Counters used by sqlite3_stmt_status() */
  ಅಕ್ಷರ *zSql;             /* Text of the SQL statement that generated this */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
  ಅಕ್ಷರ *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#ಆದರೆ_ಅಂತ್ಯ
  ಶೂನ್ಯ *pFree;            /* Free this when deleting the vdbe */
  Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು;      /* Parent frame */
  Vdbe_ಚೌಕಟ್ಟು *p_Del_ಚೌಕಟ್ಟು;   /* List of frame objects to free on VM reset */
  ಪೂರ್ಣಾಂಕ n_ಚೌಕಟ್ಟು;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  Sub_ಕಾರ್ಯಕ್ರಮ *p_ಕಾರ್ಯಕ್ರಮ;   /* Linked list of all sub-programs used by VM */
  Aux_ದತ್ತಾಂಶ *p_Aux_ದತ್ತಾಂಶ;      /* Linked list of auxdata allocations */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  ಪೂರ್ಣಾಂಕ n_ಪರಿಶೀಲನೆ;              /* Entries in aScan[] */
  ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ *a_ಪರಿಶೀಲನೆ;      /* Scan definitions for sqlite3_stmt_scanstatus() */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** The following are allowed values for Vdbe.eVdbeState
*/
#ವ್ಯಾಖ್ಯಾನಿಸು VDBE_INIT_STATE     0   /* Prepared statement under construction */
#ವ್ಯಾಖ್ಯಾನಿಸು VDBE_READY_STATE    1   /* Ready to run but not yet started */
#ವ್ಯಾಖ್ಯಾನಿಸು VDBE_ಸತತ_STATE      2   /* Run in progress */
#ವ್ಯಾಖ್ಯಾನಿಸು VDBE_HALT_STATE     3   /* Finished.  Need reset() or finalize() */

/*
** Structure used to store the context required by the
** sqlite3_preupdate_*() API functions.
*/
ರಚನೆ PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  ಪೂರ್ಣಾಂಕ op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *a_ದಾಖಲೆ;                    /* old.* database record */
  ಕೀಲಿ_Info *pKeyinfo;              /* Key information */
  Unpacked_ದಾಖಲೆ *pUnpacked;      /* Unpacked version of aRecord[] */
  Unpacked_ದಾಖಲೆ *pNewUnpacked;   /* Unpacked version of new.* record */
  ಪೂರ್ಣಾಂಕ iNewReg;                    /* Register for new.* values */
  ಪೂರ್ಣಾಂಕ iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  ಕೋಷ್ಟಕ *pTab;                    /* Schema object being updated */
  ಸೂಚ್ಯಂಕ *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_ಮೌಲ್ಯ **apDflt;         /* Array of default values, if required */
  ರಚನೆ {
    u8 keyinfoSpace[SZ_KEYINFO_0];  /* Space to hold pKeyinfo[0] content */
  } u_ಕೀಲಿ;
};

/*
** An instance of this object is used to pass an vector of values into
** OP_VFilter, the xFilter method of a virtual table.  The vector is the
** set of values on the right-hand side of an IN constraint.
**
** The value as passed into xFilter is an sqlite3_value with a "pointer"
** type, such as is generated by sqlite3_result_pointer() and read by
** sqlite3_value_pointer.  Such values have MEM_Term|MEM_Subtype|MEM_Null
** and a subtype of 'p'.  The sqlite3_vtab_in_first() and _next() interfaces
** know how to use this object to step through all the values in the
** right operand of the IN constraint.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಮೌಲ್ಯ_List ಮೌಲ್ಯ_List;
ರಚನೆ ಮೌಲ್ಯ_List {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_ಮೌಲ್ಯ *p_ಹೊರಹೋಗು;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_AMALGAMATION
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ u8 sqlite3SmallTypeSizes[];
#ಆದರೆ_ಅಂತ್ಯ

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ONE_BYTE_INT(x)    ((i8)(x)[0])
#ವ್ಯಾಖ್ಯಾನಿಸು TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#ವ್ಯಾಖ್ಯಾನಿಸು THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#ವ್ಯಾಖ್ಯಾನಿಸು FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#ವ್ಯಾಖ್ಯಾನಿಸು FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#ವ್ಯಾಖ್ಯಾನಿಸು FOUR_BYTE_INT(x)   ((ಪೂರ್ಣಾಂಕ)FOUR_BYTE_UINT(x))
#ವ್ಯಾಖ್ಯಾನಿಸು SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
ಶೂನ್ಯ sqlite3VdbeError(Vdbe*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ...);
ಶೂನ್ಯ sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
ಶೂನ್ಯ sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
ಶೂನ್ಯ sqlite_Vdbe_ತೆಗೆ_ರಾಶಿ(Vdbe*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE sqlite3_Vdbe_ಮುಕ್ತಾಯ_Moveto(VdbeCursor*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MIXED_ENDIAN_64_ಬಿಟ್_FLOAT
  u64 sqlite3_Float_ಅದಲುಬದಲು(u64 ಒಳ);
# define ಅದಲುಬದಲು_Mixed_Endian_Float(X)  X = sqlite3_Float_ಅದಲುಬದಲು(X)
#ಇಲ್ಲದಿದ್ದರೆ
# define ಅದಲುಬದಲು_Mixed_Endian_Float(X)
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3VdbeSerialGet(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*, u32, Mem*);
ಶೂನ್ಯ sqlite3_Vdbe_Delete_Aux_ದತ್ತಾಂಶ(sqlite3*, Aux_ದತ್ತಾಂಶ**, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);

ಪೂರ್ಣಾಂಕ sqlite2_Btree_ಕೀಲಿ_Compare(BtCursor *, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ *);
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Idx_ಕೀಲಿ_Compare(sqlite3*,VdbeCursor*,Unpacked_ದಾಖಲೆ*,ಪೂರ್ಣಾಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeExec(Vdbe*);
#ಆದರೆ !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಮುಂದಿನ_Opcode(Vdbe*,Mem*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*,Op**);
ಅಕ್ಷರ *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
ಅಕ್ಷರ *sqlite3VdbeDisplayComment(sqlite3*,ಸ್ಥಿರಾಂಕ Op*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(SQLITE_OMIT_EXPLAIN)
ಪೂರ್ಣಾಂಕ sqlite3VdbeList(Vdbe*);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3VdbeHalt(Vdbe*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeChangeEncoding(Mem *, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemTooBig(Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemCopy(Mem*, ಸ್ಥಿರಾಂಕ Mem*);
ಶೂನ್ಯ sqlite3VdbeMemShallowCopy(Mem*, ಸ್ಥಿರಾಂಕ Mem*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3VdbeMemMove(Mem*, Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemNulTerminate(Mem*);
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_ಗಣ_Str(Mem*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, i64, u8, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_ಗಣ_ಪಠ್ಯ(Mem*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, i64, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_Int64(Mem*, i64);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
# define sqlite3_Vdbe_Mem_ಗಣ_Double sqlite3_Vdbe_Mem_ಗಣ_Int64
#ಇಲ್ಲದಿದ್ದರೆ
  ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_Double(Mem*, ದ್ವಿಗುಣ);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_ಸೂಚಕ(Mem*, ಶೂನ್ಯ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
ಶೂನ್ಯ sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_Null(Mem*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
ಶೂನ್ಯ sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(Mem*,ಪೂರ್ಣಾಂಕ);
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_ಗಣ_Zero_Blob(Mem*,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(ಸ್ಥಿರಾಂಕ Mem*);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_ಗಣ_ಅಡ್ಡಸಾಲು_ಗಣ(Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemZeroTerminateIfAble(Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemMakeWriteable(Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemStringify(Mem*, u8, u8);
ಪೂರ್ಣಾಂಕ sqlite3IntFloatCompare(i64,ದ್ವಿಗುಣ);
i64 sqlite3_Vdbe_Int_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemIntegerify(Mem*);
ದ್ವಿಗುಣ sqlite3_Vdbe_Real_ಮೌಲ್ಯ(Mem*);
ಪೂರ್ಣಾಂಕ sqlite3_Mem_Real_ಮೌಲ್ಯ_RC(Mem*, ದ್ವಿಗುಣ*);
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Boolean_ಮೌಲ್ಯ(Mem*, ಪೂರ್ಣಾಂಕ ifNull);
ಶೂನ್ಯ sqlite3VdbeIntegerAffinity(Mem*,ಪೂರ್ಣಾಂಕ b_From_ಪಠ್ಯ);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemRealify(Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemNumerify(Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemCast(Mem*,u8,u8);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(BtCursor*,u32,Mem*);
ಶೂನ್ಯ sqlite3VdbeMemRelease(Mem *p);
ಶೂನ್ಯ sqlite3VdbeMemReleaseMalloc(Mem*p);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Mem_Agg_ಮೌಲ್ಯ(Mem*, Mem*, FuncDef*);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3OpcodeName(ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemGrow(Mem *pMem, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ preserve);
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemClearAndResize(Mem *pMem, ಪೂರ್ಣಾಂಕ n);
ಪೂರ್ಣಾಂಕ sqlite3VdbeCloseStatement(Vdbe *, ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಚೌಕಟ್ಟು_Is_Valid(Vdbe_ಚೌಕಟ್ಟು*);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_Vdbe_ಚೌಕಟ್ಟು_Mem_Del(ಶೂನ್ಯ*);      /* Destructor on Mem */
ಶೂನ್ಯ sqlite3_Vdbe_ಚೌಕಟ್ಟು_Delete(Vdbe_ಚೌಕಟ್ಟು*); /* Actually deletes the Frame */
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಚೌಕಟ್ಟು_Restore(Vdbe_ಚೌಕಟ್ಟು *);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
ಶೂನ್ಯ sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಕೋಷ್ಟಕ*,i64,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3VdbeTransferError(Vdbe *p);
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(BtCursor*, ಸೂಚ್ಯಂಕ*, Unpacked_ದಾಖಲೆ*, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ);

ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterInit(sqlite3 *, ಪೂರ್ಣಾಂಕ, VdbeCursor *);
ಶೂನ್ಯ sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
ಶೂನ್ಯ sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterRowkey(ಸ್ಥಿರಾಂಕ VdbeCursor *, Mem *);
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Sorter_ಮುಂದಿನ(sqlite3 *, ಸ್ಥಿರಾಂಕ VdbeCursor *);
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterRewind(ಸ್ಥಿರಾಂಕ VdbeCursor *, ಪೂರ್ಣಾಂಕ *);
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterWrite(ಸ್ಥಿರಾಂಕ VdbeCursor *, Mem *);
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterCompare(ಸ್ಥಿರಾಂಕ VdbeCursor *, Mem *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ *);

ಶೂನ್ಯ sqlite3_Vdbe_ಮೌಲ್ಯ_List_Free(ಶೂನ್ಯ*);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಶೂನ್ಯ sqlite3VdbeIncrWriteCounter(Vdbe*, VdbeCursor*);
  ಶೂನ್ಯ sqlite3VdbeAssertAbortable(Vdbe*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3VdbeIncrWriteCounter(V,C)
# define sqlite3VdbeAssertAbortable(V)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE)
  ಶೂನ್ಯ sqlite3VdbeEnter(Vdbe*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3VdbeEnter(X)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  ಶೂನ್ಯ sqlite3VdbeLeave(Vdbe*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3VdbeLeave(X)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಶೂನ್ಯ sqlite3_Vdbe_Mem_About_ಗಮ್ಯ_Change(Vdbe*,Mem*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeCheckMemInvariants(Mem*);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Check_Fk_ತಕ್ಷಣ(Vdbe*);
ಪೂರ್ಣಾಂಕ sqlite3VdbeCheckFkDeferred(Vdbe*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_Vdbe_Check_Fk_ತಕ್ಷಣ(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಶೂನ್ಯ sqlite3VdbePrintSql(Vdbe*);
  ಶೂನ್ಯ sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  ಪೂರ್ಣಾಂಕ sqlite3VdbeMemTranslate(Mem*, u8);
  ಪೂರ್ಣಾಂಕ sqlite3VdbeMemHandleBom(Mem *pMem);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
  ಪೂರ್ಣಾಂಕ sqlite3VdbeMemExpandBlob(Mem *);
  #ವ್ಯಾಖ್ಯಾನಿಸು ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #ವ್ಯಾಖ್ಯಾನಿಸು ExpandBlob(P) SQLITE_OK
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_VDBEINT_H) */
