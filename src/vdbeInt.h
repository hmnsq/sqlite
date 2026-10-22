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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBEINT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_VDBEINT_H

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 50
#ਜੇ_ਅੰਤ

/*
** VDBE_DISPLAY_P4 is true or false depending on whether or not the
** "explain" P4 display logic is enabled.
*/
#ਜੇ !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#ਨਹੀਂਤਾਂ
# define VDBE_DISPLAY_P4 0
#ਜੇ_ਅੰਤ

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VdbeOp Op;

/*
** Boolean values
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ Bool;

/* Opaque type used by code in vdbesort.c */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Aux_ਅੰਕੜੇ Aux_ਅੰਕੜੇ;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VdbeTxtBlbCache VdbeTxtBlbCache;

/* Types of VDBE cursors */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CURTYPE_BTREE       0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CURTYPE_SORTER      1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CURTYPE_VTAB        2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CURTYPE_PSEUDO      3

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VdbeCursor VdbeCursor;
ਬਣਤਰ VdbeCursor {
  u8 eCurType;            /* One of the CURTYPE_* values above */
  i8 iDb;                 /* Index of cursor database in db->aDb[] */
  u8 null_ਪੰਕਤੀ;             /* True if pointing to a row with no data */
  u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
  u8 is_ਸਾਰਣੀ;             /* True for rowid tables.  False for indexes */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#ਜੇ_ਅੰਤ
  Bool isEphemeral:1;     /* True for an ephemeral table */
  Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
  Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
  Bool noReuse:1;         /* OpenEphemeral may not reuse this cursor */
  Bool ਖਾਨਾ_Cache:1;        /* pCache pointer is initialized and non-NULL */
  u16 seek_ਪ੍ਰਾਪਤੀ;            /* See the OP_SeekHit and OP_IfNoHope opcodes */
  ਸੰਘ {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seq_ਗਿਣਤੀ;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cache_ਸਥਿਤੀ;        /* Cache is valid if this matches Vdbe.cacheCtr */
  ਪੂਰਨ seek_ਨਤੀਜਾ;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match".
  ** For CURTYPE_PSEUDO, seekResult is the register holding the record */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  ਸੰਘ {
    BtCursor *pCursor;          /* CURTYPE_BTREE or _PSEUDO.  Btree cursor */
    sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.              Vtab cursor */
    VdbeSorter *pSorter;        /* CURTYPE_SORTER.            Sorter object */
  } uc;
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;      /* Info about index keys needed by index cursors */
  u32 i_Hdr_ਵਿੱਥ;         /* Offset to next unparsed byte of the header */
  Pgno pgno_ਜੜ੍ਹ;          /* Root page of the open btree cursor */
  i16 n_ਖੇਤਰ;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 moveto_ਨਿਸ਼ਾਨਾ;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *a_ਵਿੱਥ;           /* Pointer to aType[nField] */
  ਅਚਲ u8 *a_ਪੰਕਤੀ;         /* Data for the current row, if all on one page */
  u32 payload_ਮਾਪ;        /* Total number of bytes in the record */
  u32 sz_ਪੰਕਤੀ;              /* Byte available in aRow */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_ਵਰਤਿਆ_ਨਕਾਬ
  u64 ਨਕਾਬ_ਵਰਤਿਆ;           /* Mask of columns used by this cursor */
#ਜੇ_ਅੰਤ
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_VDBECURSOR(N) \
    (ROUND8(offsetof(VdbeCursor,aType)) + ((N)+1)*ਆਕਾਰ(u64))

/* Return true if P is a null-only cursor
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IsNullCursor(P) \
  ((P)->eCurType==CURTYPE_PSEUDO && (P)->null_ਪੰਕਤੀ && (P)->seek_ਨਤੀਜਾ==0)

/*
** A value for VdbeCursor.cacheStatus that means the cache is always invalid.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CACHE_STALE 0

/*
** Large TEXT or BLOB values can be slow to load, so we want to avoid
** loading them more than once.  For that reason, large TEXT and BLOB values
** can be stored in a cache defined by this object, and attached to the
** VdbeCursor using the pCache field.
*/
ਬਣਤਰ VdbeTxtBlbCache {
  ਅੱਖਰ *pCValue;        /* A RCStr buffer to hold the value */
  i64 i_ਵਿੱਥ;          /* File offset of the row being cached */
  ਪੂਰਨ i_ਖਾਨਾ;             /* Column for which the cache is valid */
  u32 cache_ਸਥਿਤੀ;      /* Vdbe.cacheCtr value */
  u32 ਖਾਨਾ_Cache_Ctr;      /* Column cache counter */
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Vdbe_ਚੌਖਟਾ Vdbe_ਚੌਖਟਾ;
ਬਣਤਰ Vdbe_ਚੌਖਟਾ {
  Vdbe *v;                /* VM this frame belongs to */
  Vdbe_ਚੌਖਟਾ *p_ਮਾਪਾ;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  ਖਾਲੀ *token;            /* Copy of SubProgram.token */
  i64 ਆਖਰੀ_Rowid;          /* Last insert rowid (sqlite3.lastRowid) */
  Aux_ਅੰਕੜੇ *p_Aux_ਅੰਕੜੇ;      /* Linked list of auxdata allocations */
#ਜੇ SQLITE_DEBUG
  u32 i_ਚੌਖਟਾ_Magic;        /* magic number for sanity checking */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ nCursor;            /* Number of entries in apCsr */
  ਪੂਰਨ pc;                 /* Program Counter in parent (calling) frame */
  ਪੂਰਨ nOp;                /* Size of aOp array */
  ਪੂਰਨ nMem;               /* Number of entries in aMem */
  ਪੂਰਨ n_ਬੱਚਾ_Mem;          /* Number of memory cells for child frame */
  ਪੂਰਨ n_ਬੱਚਾ_Csr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਚੌਖਟਾ_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Vdbe_ਚੌਖਟਾ_Mem(p) ((Mem *)&((u8 *)p)[ROUND8(ਆਕਾਰ(Vdbe_ਚੌਖਟਾ))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
ਬਣਤਰ sqlite3_ਮੁੱਲ {
  ਸੰਘ Mem_ਮੁੱਲ {
    ਦੁੱਗਣਾ r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    ਪੂਰਨ nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    ਅਚਲ ਅੱਖਰ *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  ਅੱਖਰ *z;            /* String or BLOB value */
  ਪੂਰਨ n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  ਪੂਰਨ szMalloc;       /* Size of the zMalloc allocation */
  u32 u_ਆਰਜ਼ੀ;          /* Transient storage for serial_type in OP_MakeRecord */
  ਅੱਖਰ *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  u16 mScopyFlags;    /* flags value immediately after the shallow copy */
  u8  bScopy;         /* The pScopyFrom of some other Mem *might* point here */
#ਜੇ_ਅੰਤ
};

/*
** Size of struct Mem not including the Mem.zMalloc member or anything that
** follows.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEMCELLSIZE offsetof(Mem,db)

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Undefined 0x0000   /* Value is undefined */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Null      0x0001   /* Value is NULL (or a pointer) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Str       0x0002   /* Value is a string */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Int       0x0004   /* Value is an integer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Real      0x0008   /* Value is a real number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Blob      0x0010   /* Value is a BLOB */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_IntReal   0x0020   /* MEM_Int that stringifies like MEM_Real */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Aff_ਨਕਾਬ   0x003f   /* Mask of affinity bits */

/* Extra bits that modify the meanings of the core datatypes above
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_FromBind  0x0040   /* Value originates from sqlite3_bind() */
 /*                   0x0080   // Available */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Cleared   0x0100   /* NULL set by OP_Null, not from data */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_ਪਦ      0x0200   /* String in Mem.z is zero terminated */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Zero      0x0400   /* Mem.i contains count of 0s appended to blob */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Subtype   0x0800   /* Mem.eSubtype is valid */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Type_ਨਕਾਬ  0x0dbf   /* Mask of type bits */

/* Bits that determine the storage for Mem.z for a string or blob or
** aggregate accumulator.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Dyn       0x1000   /* Need to call Mem.xDel() on Mem.z */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Static    0x2000   /* Mem.z points to a static string */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Ephem     0x4000   /* Mem.z points to an ephemeral string */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_Agg       0x8000   /* Mem.z points to an agg function context */

/* Return TRUE if Mem X contains dynamically allocated content - anything
** that needs to be deallocated to avoid a leak.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VdbeMemDynamic(X)  \
  (((X)->flags&(MEM_Agg|MEM_Dyn))!=0)

/*
** Clear any existing type flags from a Mem and replace them with f
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Mem_ਸੈੱਟ_Type_Flag(p, f) \
   ((p)->flags = ((p)->flags&~(MEM_Type_ਨਕਾਬ|MEM_Zero))|f)

/*
** True if Mem X is a NULL-nochng type.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MemNullNochng(X) \
  (((X)->flags&MEM_Type_ਨਕਾਬ)==(MEM_Null|MEM_Zero) \
    && (X)->n==0 && (X)->u.nZero==0)

/*
** Return true if a memory cell has been initialized and is valid.
** is for use inside assert() statements only.
**
** A Memory cell is initialized if at least one of the
** MEM_Null, MEM_Str, MEM_Int, MEM_Real, MEM_Blob, or MEM_IntReal bits
** is set.  It is "undefined" if all those bits are zero.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ memIsValid(M)  ((M)->flags & MEM_Aff_ਨਕਾਬ)!=0
#ਜੇ_ਅੰਤ

/*
** Each auxiliary data pointer stored by a user defined function
** implementation calling sqlite3_set_auxdata() is stored in an instance
** of this structure. All such structures associated with a single VM
** are stored in a linked list headed at Vdbe.pAuxData. All are destroyed
** when the VM is halted (if not before).
*/
ਬਣਤਰ Aux_ਅੰਕੜੇ {
  ਪੂਰਨ iAuxOp;                     /* Instruction number of OP_Function opcode */
  ਪੂਰਨ iAuxArg;                    /* Index of function argument. */
  ਖਾਲੀ *pAux;                     /* Aux data pointer */
  ਖਾਲੀ (*xDeleteAux)(ਖਾਲੀ*);      /* Destructor for the aux data */
  Aux_ਅੰਕੜੇ *p_ਅਗਲਾ_Aux;              /* Next element in list */
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
ਬਣਤਰ sqlite3_context {
  Mem *p_ਬਾਹਰਲਾ;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  ਪੂਰਨ iOp;                /* Instruction number of OP_Function */
  ਪੂਰਨ isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_ਮੁੱਲ *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_CONTEXT(N)  \
   (offsetof(sqlite3_context,argv)+(N)*ਆਕਾਰ(sqlite3_ਮੁੱਲ*))


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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਜਾਂਚ_ਸਥਿਤੀ ਜਾਂਚ_ਸਥਿਤੀ;
ਬਣਤਰ ਜਾਂਚ_ਸਥਿਤੀ {
  ਪੂਰਨ addrExplain;                /* OP_Explain for loop */
  ਪੂਰਨ aAddrRange[6];
  ਪੂਰਨ addrLoop;                   /* Address of "loops" counter */
  ਪੂਰਨ addr_ਦੇਖੋ;                  /* Address of "rows visited" counter */
  ਪੂਰਨ iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  ਅੱਖਰ *zName;                    /* Name of table or index */
};

/* The DblquoteStr object holds the text of a double-quoted
** string for a prepared statement.  A linked list of these objects
** is constructed during statement parsing and is held on Vdbe.pDblStr.
** When computing a normalized SQL statement for an SQL statement, that
** list is consulted for each double-quoted identifier to see if the
** identifier should really be a string literal.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ DblquoteStr DblquoteStr;
ਬਣਤਰ DblquoteStr {
  DblquoteStr *p_ਅਗਲਾ_Str;   /* Next string literal in the list */
  ਅੱਖਰ z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
ਬਣਤਰ Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  ਪੂਰਨ nMem;               /* Number of memory locations currently allocated */
  ਪੂਰਨ nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  ਪੂਰਨ pc;                 /* The program counter */
  ਪੂਰਨ rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  ਪੂਰਨ iStatement;         /* Statement number (or 0 if has no opened stmt) */
  i64 i_Current_ਸਮਾਂ;       /* Value of julianday('now') for this statement */
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
  ਪੂਰਨ nOp;                /* Number of instructions in the program */
  ਪੂਰਨ nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *a_ਖਾਨਾ_Name;          /* Column names to return */
  Mem *p_ਨਤੀਜਾ_ਪੰਕਤੀ;        /* Current output row */
  ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
  i64 ਸ਼ੁਰੂ_ਸਮਾਂ;          /* Time when query started - used for profiling */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪੂਰਨ rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  ਪੂਰਨ napArg;             /* Size of the apArg[] array */
#ਜੇ_ਅੰਤ
  u16 n_Res_ਕਾਲਮ;         /* Number of columns in one row of the result set */
  u16 nResAlloc;          /* Column slots allocated to aColName[] */
  u8 errorAction;         /* Recovery action to do in case of an error */
  u8 minWriteFileFormat;  /* Minimum file format for writable database files */
  u8 prepFlags;           /* SQLITE_PREPARE_* flags */
  u8 eVdbeState;          /* On of the VDBE_*_STATE values */
  bft expired:2;          /* 1: recompile VM immediately  2: when convenient */
  bft explain:2;          /* 0: normal, 1: EXPLAIN, 2: EXPLAIN QUERY PLAN */
  bft changeCntOn:1;      /* True to update the change-counter */
  bft ਵਰਤਦਾ_Stmt_Journal:1;  /* True if uses a statement journal */
  bft readOnly:1;         /* True for statements that do not write */
  bft bIsReader:1;        /* True for statements that read */
  bft haveEqpOps:1;       /* Bytecode supports EXPLAIN QUERY PLAN */
  y_Db_ਨਕਾਬ btree_ਨਕਾਬ;      /* Bitmask of db->aDb[] entries referenced */
  y_Db_ਨਕਾਬ ਤਾਲਾ_ਨਕਾਬ;       /* Subset of btreeMask that requires a lock */
  u32 aCounter[9];        /* Counters used by sqlite3_stmt_status() */
  ਅੱਖਰ *zSql;             /* Text of the SQL statement that generated this */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
  ਅੱਖਰ *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#ਜੇ_ਅੰਤ
  ਖਾਲੀ *pFree;            /* Free this when deleting the vdbe */
  Vdbe_ਚੌਖਟਾ *p_ਚੌਖਟਾ;      /* Parent frame */
  Vdbe_ਚੌਖਟਾ *p_Del_ਚੌਖਟਾ;   /* List of frame objects to free on VM reset */
  ਪੂਰਨ n_ਚੌਖਟਾ;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  Sub_ਪ੍ਰੋਗਰਾਮ *p_ਪ੍ਰੋਗਰਾਮ;   /* Linked list of all sub-programs used by VM */
  Aux_ਅੰਕੜੇ *p_Aux_ਅੰਕੜੇ;      /* Linked list of auxdata allocations */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  ਪੂਰਨ n_ਜਾਂਚ;              /* Entries in aScan[] */
  ਜਾਂਚ_ਸਥਿਤੀ *a_ਜਾਂਚ;      /* Scan definitions for sqlite3_stmt_scanstatus() */
#ਜੇ_ਅੰਤ
};

/*
** The following are allowed values for Vdbe.eVdbeState
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VDBE_INIT_STATE     0   /* Prepared statement under construction */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VDBE_READY_STATE    1   /* Ready to run but not yet started */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VDBE_ਲੜੀਵਾਰ_STATE      2   /* Run in progress */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VDBE_HALT_STATE     3   /* Finished.  Need reset() or finalize() */

/*
** Structure used to store the context required by the
** sqlite3_preupdate_*() API functions.
*/
ਬਣਤਰ PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  ਪੂਰਨ op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *a_ਰਿਕਾਰਡ;                    /* old.* database record */
  ਕੁੰਜੀ_Info *pKeyinfo;              /* Key information */
  Unpacked_ਰਿਕਾਰਡ *pUnpacked;      /* Unpacked version of aRecord[] */
  Unpacked_ਰਿਕਾਰਡ *pNewUnpacked;   /* Unpacked version of new.* record */
  ਪੂਰਨ iNewReg;                    /* Register for new.* values */
  ਪੂਰਨ iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  ਸਾਰਣੀ *pTab;                    /* Schema object being updated */
  ਸੂਚਕਾਂਕ *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_ਮੁੱਲ **apDflt;         /* Array of default values, if required */
  ਬਣਤਰ {
    u8 keyinfoSpace[SZ_KEYINFO_0];  /* Space to hold pKeyinfo[0] content */
  } u_ਕੁੰਜੀ;
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਮੁੱਲ_List ਮੁੱਲ_List;
ਬਣਤਰ ਮੁੱਲ_List {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_ਮੁੱਲ *p_ਬਾਹਰਲਾ;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_AMALGAMATION
ਬਾਹਰੀ ਅਚਲ u8 sqlite3SmallTypeSizes[];
#ਜੇ_ਅੰਤ

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONE_BYTE_INT(x)    ((i8)(x)[0])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOUR_BYTE_INT(x)   ((ਪੂਰਨ)FOUR_BYTE_UINT(x))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
ਖਾਲੀ sqlite3VdbeError(Vdbe*, ਅਚਲ ਅੱਖਰ *, ...);
ਖਾਲੀ sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
ਖਾਲੀ sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
ਖਾਲੀ sqlite_Vdbe_ਕੱਢੋ_ਢੇਰ(Vdbe*,ਪੂਰਨ);
ਪੂਰਨ SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
ਪੂਰਨ SQLITE_NOINLINE sqlite3_Vdbe_ਅੰਤ_Moveto(VdbeCursor*);
ਪੂਰਨ sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MIXED_ENDIAN_64_ਦੋਆਧਾਰੀ_ਅੰਕ_FLOAT
  u64 sqlite3_Float_ਅਦਲਾ_ਬਦਲੀ(u64 ਵਿੱਚ);
# define ਅਦਲਾ_ਬਦਲੀ_Mixed_Endian_Float(X)  X = sqlite3_Float_ਅਦਲਾ_ਬਦਲੀ(X)
#ਨਹੀਂਤਾਂ
# define ਅਦਲਾ_ਬਦਲੀ_Mixed_Endian_Float(X)
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3VdbeSerialGet(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*, u32, Mem*);
ਖਾਲੀ sqlite3_Vdbe_Delete_Aux_ਅੰਕੜੇ(sqlite3*, Aux_ਅੰਕੜੇ**, ਪੂਰਨ, ਪੂਰਨ);

ਪੂਰਨ sqlite2_Btree_ਕੁੰਜੀ_Compare(BtCursor *, ਅਚਲ ਖਾਲੀ *, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ *);
ਪੂਰਨ sqlite3_Vdbe_Idx_ਕੁੰਜੀ_Compare(sqlite3*,VdbeCursor*,Unpacked_ਰਿਕਾਰਡ*,ਪੂਰਨ*);
ਪੂਰਨ sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
ਪੂਰਨ sqlite3VdbeExec(Vdbe*);
#ਜੇ !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
ਪੂਰਨ sqlite3_Vdbe_ਅਗਲਾ_Opcode(Vdbe*,Mem*,ਪੂਰਨ,ਪੂਰਨ*,ਪੂਰਨ*,Op**);
ਅੱਖਰ *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
ਅੱਖਰ *sqlite3VdbeDisplayComment(sqlite3*,ਅਚਲ Op*,ਅਚਲ ਅੱਖਰ*);
#ਜੇ_ਅੰਤ
#ਜੇ !defined(SQLITE_OMIT_EXPLAIN)
ਪੂਰਨ sqlite3VdbeList(Vdbe*);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3VdbeHalt(Vdbe*);
ਪੂਰਨ sqlite3VdbeChangeEncoding(Mem *, ਪੂਰਨ);
ਪੂਰਨ sqlite3VdbeMemTooBig(Mem*);
ਪੂਰਨ sqlite3VdbeMemCopy(Mem*, ਅਚਲ Mem*);
ਖਾਲੀ sqlite3VdbeMemShallowCopy(Mem*, ਅਚਲ Mem*, ਪੂਰਨ);
ਖਾਲੀ sqlite3VdbeMemMove(Mem*, Mem*);
ਪੂਰਨ sqlite3VdbeMemNulTerminate(Mem*);
ਪੂਰਨ sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(Mem*, ਅਚਲ ਅੱਖਰ*, i64, u8, ਖਾਲੀ(*)(ਖਾਲੀ*));
ਪੂਰਨ sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪਾਠ(Mem*, ਅਚਲ ਅੱਖਰ*, i64, ਖਾਲੀ(*)(ਖਾਲੀ*));
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(Mem*, i64);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
# define sqlite3_Vdbe_Mem_ਸੈੱਟ_Double sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64
#ਨਹੀਂਤਾਂ
  ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_Double(Mem*, ਦੁੱਗਣਾ);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_ਸੂਚਕ(Mem*, ਖਾਲੀ*, ਅਚਲ ਅੱਖਰ*, ਖਾਲੀ(*)(ਖਾਲੀ*));
ਖਾਲੀ sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(Mem*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
ਖਾਲੀ sqlite3_Vdbe_Mem_ਸੈੱਟ_Zero_Blob(Mem*,ਪੂਰਨ);
#ਨਹੀਂਤਾਂ
ਪੂਰਨ sqlite3_Vdbe_Mem_ਸੈੱਟ_Zero_Blob(Mem*,ਪੂਰਨ);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(ਅਚਲ Mem*);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪੰਕਤੀ_ਸੈੱਟ(Mem*);
ਪੂਰਨ sqlite3VdbeMemZeroTerminateIfAble(Mem*);
ਪੂਰਨ sqlite3VdbeMemMakeWriteable(Mem*);
ਪੂਰਨ sqlite3VdbeMemStringify(Mem*, u8, u8);
ਪੂਰਨ sqlite3IntFloatCompare(i64,ਦੁੱਗਣਾ);
i64 sqlite3_Vdbe_Int_ਮੁੱਲ(ਅਚਲ Mem*);
ਪੂਰਨ sqlite3VdbeMemIntegerify(Mem*);
ਦੁੱਗਣਾ sqlite3_Vdbe_Real_ਮੁੱਲ(Mem*);
ਪੂਰਨ sqlite3_Mem_Real_ਮੁੱਲ_RC(Mem*, ਦੁੱਗਣਾ*);
ਪੂਰਨ sqlite3_Vdbe_Boolean_ਮੁੱਲ(Mem*, ਪੂਰਨ ifNull);
ਖਾਲੀ sqlite3VdbeIntegerAffinity(Mem*,ਪੂਰਨ b_From_ਪਾਠ);
ਪੂਰਨ sqlite3VdbeMemRealify(Mem*);
ਪੂਰਨ sqlite3VdbeMemNumerify(Mem*);
ਪੂਰਨ sqlite3VdbeMemCast(Mem*,u8,u8);
ਪੂਰਨ sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
ਪੂਰਨ sqlite3_Vdbe_Mem_From_Btree_Zero_ਵਿੱਥ(BtCursor*,u32,Mem*);
ਖਾਲੀ sqlite3VdbeMemRelease(Mem *p);
ਖਾਲੀ sqlite3VdbeMemReleaseMalloc(Mem*p);
ਪੂਰਨ sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਪੂਰਨ sqlite3_Vdbe_Mem_Agg_ਮੁੱਲ(Mem*, Mem*, FuncDef*);
#ਜੇ_ਅੰਤ
#ਜੇ !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
ਅਚਲ ਅੱਖਰ *sqlite3OpcodeName(ਪੂਰਨ);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3VdbeMemGrow(Mem *pMem, ਪੂਰਨ n, ਪੂਰਨ preserve);
ਪੂਰਨ sqlite3VdbeMemClearAndResize(Mem *pMem, ਪੂਰਨ n);
ਪੂਰਨ sqlite3VdbeCloseStatement(Vdbe *, ਪੂਰਨ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ sqlite3_Vdbe_ਚੌਖਟਾ_Is_Valid(Vdbe_ਚੌਖਟਾ*);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_Vdbe_ਚੌਖਟਾ_Mem_Del(ਖਾਲੀ*);      /* Destructor on Mem */
ਖਾਲੀ sqlite3_Vdbe_ਚੌਖਟਾ_Delete(Vdbe_ਚੌਖਟਾ*); /* Actually deletes the Frame */
ਪੂਰਨ sqlite3_Vdbe_ਚੌਖਟਾ_Restore(Vdbe_ਚੌਖਟਾ *);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
ਖਾਲੀ sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*,ਸਾਰਣੀ*,i64,ਪੂਰਨ,ਪੂਰਨ);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3VdbeTransferError(Vdbe *p);
ਪੂਰਨ sqlite3_Vdbe_ਲੱਭੋ_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(BtCursor*, ਸੂਚਕਾਂਕ*, Unpacked_ਰਿਕਾਰਡ*, ਪੂਰਨ*, ਪੂਰਨ);

ਪੂਰਨ sqlite3VdbeSorterInit(sqlite3 *, ਪੂਰਨ, VdbeCursor *);
ਖਾਲੀ sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
ਖਾਲੀ sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
ਪੂਰਨ sqlite3VdbeSorterRowkey(ਅਚਲ VdbeCursor *, Mem *);
ਪੂਰਨ sqlite3_Vdbe_Sorter_ਅਗਲਾ(sqlite3 *, ਅਚਲ VdbeCursor *);
ਪੂਰਨ sqlite3VdbeSorterRewind(ਅਚਲ VdbeCursor *, ਪੂਰਨ *);
ਪੂਰਨ sqlite3VdbeSorterWrite(ਅਚਲ VdbeCursor *, Mem *);
ਪੂਰਨ sqlite3VdbeSorterCompare(ਅਚਲ VdbeCursor *, Mem *, ਪੂਰਨ, ਪੂਰਨ *);

ਖਾਲੀ sqlite3_Vdbe_ਮੁੱਲ_List_Free(ਖਾਲੀ*);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਖਾਲੀ sqlite3VdbeIncrWriteCounter(Vdbe*, VdbeCursor*);
  ਖਾਲੀ sqlite3VdbeAssertAbortable(Vdbe*);
#ਨਹੀਂਤਾਂ
# define sqlite3VdbeIncrWriteCounter(V,C)
# define sqlite3VdbeAssertAbortable(V)
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE)
  ਖਾਲੀ sqlite3VdbeEnter(Vdbe*);
#ਨਹੀਂਤਾਂ
# define sqlite3VdbeEnter(X)
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  ਖਾਲੀ sqlite3VdbeLeave(Vdbe*);
#ਨਹੀਂਤਾਂ
# define sqlite3VdbeLeave(X)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਖਾਲੀ sqlite3_Vdbe_Mem_About_ਮੰਜ਼ਿਲ_Change(Vdbe*,Mem*);
ਪੂਰਨ sqlite3VdbeCheckMemInvariants(Mem*);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
ਪੂਰਨ sqlite3_Vdbe_Check_Fk_ਤੁਰੰਤ(Vdbe*);
ਪੂਰਨ sqlite3VdbeCheckFkDeferred(Vdbe*);
#ਨਹੀਂਤਾਂ
# define sqlite3_Vdbe_Check_Fk_ਤੁਰੰਤ(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਖਾਲੀ sqlite3VdbePrintSql(Vdbe*);
  ਖਾਲੀ sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  ਪੂਰਨ sqlite3VdbeMemTranslate(Mem*, u8);
  ਪੂਰਨ sqlite3VdbeMemHandleBom(Mem *pMem);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
  ਪੂਰਨ sqlite3VdbeMemExpandBlob(Mem *);
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ExpandBlob(P) SQLITE_OK
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* !defined(SQLITE_VDBEINT_H) */
