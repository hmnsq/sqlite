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
#ifndef SQLITE_VDBEINT_H
#define SQLITE_VDBEINT_H

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#ifndef SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 50
#endif

/*
** VDBE_DISPLAY_P4 is true or false depending on whether or not the
** "explain" P4 display logic is enabled.
*/
#যদি !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#অন্যথা
# define VDBE_DISPLAY_P4 0
#endif

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
প্রকারনির্দেশ সংরচনা VdbeOp Op;

/*
** Boolean values
*/
প্রকারনির্দেশ unsigned Bool;

/* Opaque type used by code in vdbesort.c */
প্রকারনির্দেশ সংরচনা VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
প্রকারনির্দেশ সংরচনা AuxData AuxData;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
প্রকারনির্দেশ সংরচনা VdbeTxtBlbCache VdbeTxtBlbCache;

/* Types of VDBE cursors */
#define CURTYPE_BTREE       0
#define CURTYPE_SORTER      1
#define CURTYPE_VTAB        2
#define CURTYPE_PSEUDO      3

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
প্রকারনির্দেশ সংরচনা VdbeCursor VdbeCursor;
সংরচনা VdbeCursor {
  u8 eCurType;            /* One of the CURTYPE_* values above */
  i8 iDb;                 /* Index of cursor database in db->aDb[] */
  u8 nullRow;             /* True if pointing to a row with no data */
  u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
  u8 isTable;             /* True for rowid tables.  False for indexes */
#ifdef SQLITE_DEBUG
  u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#endif
  Bool isEphemeral:1;     /* True for an ephemeral table */
  Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
  Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
  Bool noReuse:1;         /* OpenEphemeral may not reuse this cursor */
  Bool colCache:1;        /* pCache pointer is initialized and non-NULL */
  u16 seekHit;            /* See the OP_SeekHit and OP_IfNoHope opcodes */
  সঙ্ঘ {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seq_গুনতি;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cacheStatus;        /* Cache is valid if this matches Vdbe.cacheCtr */
  পূর্ণাঙ্ক seekResult;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match".
  ** For CURTYPE_PSEUDO, seekResult is the register holding the record */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  সঙ্ঘ {
    BtCursor *pCursor;          /* CURTYPE_BTREE or _PSEUDO.  Btree cursor */
    sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.              Vtab cursor */
    VdbeSorter *pSorter;        /* CURTYPE_SORTER.            Sorter object */
  } uc;
  চাবি_Info *p_চাবি_Info;      /* Info about index keys needed by index cursors */
  u32 iHdrOffset;         /* Offset to next unparsed byte of the header */
  Pgno pgno_মূল;          /* Root page of the open btree cursor */
  i16 nField;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 movetoTarget;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *aOffset;           /* Pointer to aType[nField] */
  ধ্রুৱ u8 *aRow;         /* Data for the current row, if all on one page */
  u32 payloadSize;        /* Total number of bytes in the record */
  u32 szRow;              /* Byte available in aRow */
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
  u64 maskUsed;           /* Mask of columns used by this cursor */
#endif
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
#define SZ_VDBECURSOR(N) \
    (ROUND8(offsetof(VdbeCursor,aType)) + ((N)+1)*পরিমাণ(u64))

/* Return true if P is a null-only cursor
*/
#define IsNullCursor(P) \
  ((P)->eCurType==CURTYPE_PSEUDO && (P)->nullRow && (P)->seekResult==0)

/*
** A value for VdbeCursor.cacheStatus that means the cache is always invalid.
*/
#define CACHE_STALE 0

/*
** Large TEXT or BLOB values can be slow to load, so we want to avoid
** loading them more than once.  For that reason, large TEXT and BLOB values
** can be stored in a cache defined by this object, and attached to the
** VdbeCursor using the pCache field.
*/
সংরচনা VdbeTxtBlbCache {
  ৱর্ণ *pCValue;        /* A RCStr buffer to hold the value */
  i64 iOffset;          /* File offset of the row being cached */
  পূর্ণাঙ্ক iCol;             /* Column for which the cache is valid */
  u32 cacheStatus;      /* Vdbe.cacheCtr value */
  u32 colCacheCtr;      /* Column cache counter */
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
প্রকারনির্দেশ সংরচনা VdbeFrame VdbeFrame;
সংরচনা VdbeFrame {
  Vdbe *v;                /* VM this frame belongs to */
  VdbeFrame *pParent;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  শূন্য *token;            /* Copy of SubProgram.token */
  i64 lastRowid;          /* Last insert rowid (sqlite3.lastRowid) */
  AuxData *pAuxData;      /* Linked list of auxdata allocations */
#যদি SQLITE_DEBUG
  u32 iFrameMagic;        /* magic number for sanity checking */
#endif
  পূর্ণাঙ্ক nCursor;            /* Number of entries in apCsr */
  পূর্ণাঙ্ক pc;                 /* Program Counter in parent (calling) frame */
  পূর্ণাঙ্ক nOp;                /* Size of aOp array */
  পূর্ণাঙ্ক nMem;               /* Number of entries in aMem */
  পূর্ণাঙ্ক nChildMem;          /* Number of memory cells for child frame */
  পূর্ণাঙ্ক nChildCsr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#define SQLITE_FRAME_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#define VdbeFrameMem(p) ((Mem *)&((u8 *)p)[ROUND8(পরিমাণ(VdbeFrame))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
সংরচনা sqlite3_মান {
  সঙ্ঘ Mem_মান {
    দ্ৱিগুণ r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    পূর্ণাঙ্ক nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    ধ্রুৱ ৱর্ণ *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  ৱর্ণ *z;            /* String or BLOB value */
  পূর্ণাঙ্ক n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  পূর্ণাঙ্ক szMalloc;       /* Size of the zMalloc allocation */
  u32 uTemp;          /* Transient storage for serial_type in OP_MakeRecord */
  ৱর্ণ *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  শূন্য (*xDel)(শূন্য*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#ifdef SQLITE_DEBUG
  Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  u16 mScopyFlags;    /* flags value immediately after the shallow copy */
  u8  bScopy;         /* The pScopyFrom of some other Mem *might* point here */
#endif
};

/*
** Size of struct Mem not including the Mem.zMalloc member or anything that
** follows.
*/
#define MEMCELLSIZE offsetof(Mem,db)

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
#define MEM_Undefined 0x0000   /* Value is undefined */
#define MEM_Null      0x0001   /* Value is NULL (or a pointer) */
#define MEM_Str       0x0002   /* Value is a string */
#define MEM_Int       0x0004   /* Value is an integer */
#define MEM_Real      0x0008   /* Value is a real number */
#define MEM_Blob      0x0010   /* Value is a BLOB */
#define MEM_IntReal   0x0020   /* MEM_Int that stringifies like MEM_Real */
#define MEM_AffMask   0x003f   /* Mask of affinity bits */

/* Extra bits that modify the meanings of the core datatypes above
*/
#define MEM_FromBind  0x0040   /* Value originates from sqlite3_bind() */
 /*                   0x0080   // Available */
#define MEM_Cleared   0x0100   /* NULL set by OP_Null, not from data */
#define MEM_Term      0x0200   /* String in Mem.z is zero terminated */
#define MEM_Zero      0x0400   /* Mem.i contains count of 0s appended to blob */
#define MEM_Subtype   0x0800   /* Mem.eSubtype is valid */
#define MEM_TypeMask  0x0dbf   /* Mask of type bits */

/* Bits that determine the storage for Mem.z for a string or blob or
** aggregate accumulator.
*/
#define MEM_Dyn       0x1000   /* Need to call Mem.xDel() on Mem.z */
#define MEM_Static    0x2000   /* Mem.z points to a static string */
#define MEM_Ephem     0x4000   /* Mem.z points to an ephemeral string */
#define MEM_Agg       0x8000   /* Mem.z points to an agg function context */

/* Return TRUE if Mem X contains dynamically allocated content - anything
** that needs to be deallocated to avoid a leak.
*/
#define VdbeMemDynamic(X)  \
  (((X)->flags&(MEM_Agg|MEM_Dyn))!=0)

/*
** Clear any existing type flags from a Mem and replace them with f
*/
#define MemSetTypeFlag(p, f) \
   ((p)->flags = ((p)->flags&~(MEM_TypeMask|MEM_Zero))|f)

/*
** True if Mem X is a NULL-nochng type.
*/
#define MemNullNochng(X) \
  (((X)->flags&MEM_TypeMask)==(MEM_Null|MEM_Zero) \
    && (X)->n==0 && (X)->u.nZero==0)

/*
** Return true if a memory cell has been initialized and is valid.
** is for use inside assert() statements only.
**
** A Memory cell is initialized if at least one of the
** MEM_Null, MEM_Str, MEM_Int, MEM_Real, MEM_Blob, or MEM_IntReal bits
** is set.  It is "undefined" if all those bits are zero.
*/
#ifdef SQLITE_DEBUG
#define memIsValid(M)  ((M)->flags & MEM_AffMask)!=0
#endif

/*
** Each auxiliary data pointer stored by a user defined function
** implementation calling sqlite3_set_auxdata() is stored in an instance
** of this structure. All such structures associated with a single VM
** are stored in a linked list headed at Vdbe.pAuxData. All are destroyed
** when the VM is halted (if not before).
*/
সংরচনা AuxData {
  পূর্ণাঙ্ক iAuxOp;                     /* Instruction number of OP_Function opcode */
  পূর্ণাঙ্ক iAuxArg;                    /* Index of function argument. */
  শূন্য *pAux;                     /* Aux data pointer */
  শূন্য (*xDeleteAux)(শূন্য*);      /* Destructor for the aux data */
  AuxData *pNextAux;              /* Next element in list */
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
সংরচনা sqlite3_context {
  Mem *pOut;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  পূর্ণাঙ্ক iOp;                /* Instruction number of OP_Function */
  পূর্ণাঙ্ক isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_মান *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#define SZ_CONTEXT(N)  \
   (offsetof(sqlite3_context,argv)+(N)*পরিমাণ(sqlite3_মান*))


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
প্রকারনির্দেশ সংরচনা ScanStatus ScanStatus;
সংরচনা ScanStatus {
  পূর্ণাঙ্ক addrExplain;                /* OP_Explain for loop */
  পূর্ণাঙ্ক aAddrRange[6];
  পূর্ণাঙ্ক addrLoop;                   /* Address of "loops" counter */
  পূর্ণাঙ্ক addrVisit;                  /* Address of "rows visited" counter */
  পূর্ণাঙ্ক iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  ৱর্ণ *zName;                    /* Name of table or index */
};

/* The DblquoteStr object holds the text of a double-quoted
** string for a prepared statement.  A linked list of these objects
** is constructed during statement parsing and is held on Vdbe.pDblStr.
** When computing a normalized SQL statement for an SQL statement, that
** list is consulted for each double-quoted identifier to see if the
** identifier should really be a string literal.
*/
প্রকারনির্দেশ সংরচনা DblquoteStr DblquoteStr;
সংরচনা DblquoteStr {
  DblquoteStr *pNextStr;   /* Next string literal in the list */
  ৱর্ণ z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
সংরচনা Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  পূর্ণাঙ্ক nMem;               /* Number of memory locations currently allocated */
  পূর্ণাঙ্ক nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  পূর্ণাঙ্ক pc;                 /* The program counter */
  পূর্ণাঙ্ক rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  পূর্ণাঙ্ক iStatement;         /* Statement number (or 0 if has no opened stmt) */
  i64 iCurrentTime;       /* Value of julianday('now') for this statement */
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
  পূর্ণাঙ্ক nOp;                /* Number of instructions in the program */
  পূর্ণাঙ্ক nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *aColName;          /* Column names to return */
  Mem *pResultRow;        /* Current output row */
  ৱর্ণ *z_Err_বাৰ্তা;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#ifndef SQLITE_OMIT_TRACE
  i64 startTime;          /* Time when query started - used for profiling */
#endif
#ifdef SQLITE_DEBUG
  পূর্ণাঙ্ক rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  পূর্ণাঙ্ক napArg;             /* Size of the apArg[] array */
#endif
  u16 nResColumn;         /* Number of columns in one row of the result set */
  u16 nResAlloc;          /* Column slots allocated to aColName[] */
  u8 errorAction;         /* Recovery action to do in case of an error */
  u8 minWriteFileFormat;  /* Minimum file format for writable database files */
  u8 prepFlags;           /* SQLITE_PREPARE_* flags */
  u8 eVdbeState;          /* On of the VDBE_*_STATE values */
  bft expired:2;          /* 1: recompile VM immediately  2: when convenient */
  bft explain:2;          /* 0: normal, 1: EXPLAIN, 2: EXPLAIN QUERY PLAN */
  bft changeCntOn:1;      /* True to update the change-counter */
  bft usesStmtJournal:1;  /* True if uses a statement journal */
  bft readOnly:1;         /* True for statements that do not write */
  bft bIsReader:1;        /* True for statements that read */
  bft haveEqpOps:1;       /* Bytecode supports EXPLAIN QUERY PLAN */
  yDbMask btreeMask;      /* Bitmask of db->aDb[] entries referenced */
  yDbMask lockMask;       /* Subset of btreeMask that requires a lock */
  u32 aCounter[9];        /* Counters used by sqlite3_stmt_status() */
  ৱর্ণ *zSql;             /* Text of the SQL statement that generated this */
#ifdef SQLITE_ENABLE_NORMALIZE
  ৱর্ণ *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#endif
  শূন্য *pFree;            /* Free this when deleting the vdbe */
  VdbeFrame *pFrame;      /* Parent frame */
  VdbeFrame *pDelFrame;   /* List of frame objects to free on VM reset */
  পূর্ণাঙ্ক nFrame;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  SubProgram *pProgram;   /* Linked list of all sub-programs used by VM */
  AuxData *pAuxData;      /* Linked list of auxdata allocations */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  পূর্ণাঙ্ক nScan;              /* Entries in aScan[] */
  ScanStatus *aScan;      /* Scan definitions for sqlite3_stmt_scanstatus() */
#endif
};

/*
** The following are allowed values for Vdbe.eVdbeState
*/
#define VDBE_INIT_STATE     0   /* Prepared statement under construction */
#define VDBE_READY_STATE    1   /* Ready to run but not yet started */
#define VDBE_RUN_STATE      2   /* Run in progress */
#define VDBE_HALT_STATE     3   /* Finished.  Need reset() or finalize() */

/*
** Structure used to store the context required by the
** sqlite3_preupdate_*() API functions.
*/
সংরচনা PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  পূর্ণাঙ্ক op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *aRecord;                    /* old.* database record */
  চাবি_Info *pKeyinfo;              /* Key information */
  UnpackedRecord *pUnpacked;      /* Unpacked version of aRecord[] */
  UnpackedRecord *pNewUnpacked;   /* Unpacked version of new.* record */
  পূর্ণাঙ্ক iNewReg;                    /* Register for new.* values */
  পূর্ণাঙ্ক iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  Table *pTab;                    /* Schema object being updated */
  Index *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_মান **apDflt;         /* Array of default values, if required */
  সংরচনা {
    u8 keyinfoSpace[SZ_KEYINFO_0];  /* Space to hold pKeyinfo[0] content */
  } u_চাবি;
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
প্রকারনির্দেশ সংরচনা মান_List মান_List;
সংরচনা মান_List {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_মান *pOut;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#ifndef SQLITE_AMALGAMATION
বাহ্য ধ্রুৱ u8 sqlite3SmallTypeSizes[];
#endif

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#define ONE_BYTE_INT(x)    ((i8)(x)[0])
#define TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#define THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#define FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define FOUR_BYTE_INT(x)   ((পূর্ণাঙ্ক)FOUR_BYTE_UINT(x))
#define SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
শূন্য sqlite3VdbeError(Vdbe*, ধ্রুৱ ৱর্ণ *, ...);
শূন্য sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
শূন্য sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
শূন্য sqlite_Vdbe_বেৰ_কৰো_স্তূপ(Vdbe*,পূর্ণাঙ্ক);
পূর্ণাঙ্ক SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
পূর্ণাঙ্ক SQLITE_NOINLINE sqlite3VdbeFinishMoveto(VdbeCursor*);
পূর্ণাঙ্ক sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#ifdef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
  u64 sqlite3FloatSwap(u64 in);
# define swapMixedEndianFloat(X)  X = sqlite3FloatSwap(X)
#অন্যথা
# define swapMixedEndianFloat(X)
#endif
শূন্য sqlite3VdbeSerialGet(ধ্রুৱ unsigned ৱর্ণ*, u32, Mem*);
শূন্য sqlite3VdbeDeleteAuxData(sqlite3*, AuxData**, পূর্ণাঙ্ক, পূর্ণাঙ্ক);

পূর্ণাঙ্ক sqlite2_Btree_চাবি_Compare(BtCursor *, ধ্রুৱ শূন্য *, পূর্ণাঙ্ক, পূর্ণাঙ্ক, পূর্ণাঙ্ক *);
পূর্ণাঙ্ক sqlite3_Vdbe_Idx_চাবি_Compare(sqlite3*,VdbeCursor*,UnpackedRecord*,পূর্ণাঙ্ক*);
পূর্ণাঙ্ক sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
পূর্ণাঙ্ক sqlite3VdbeExec(Vdbe*);
#যদি !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
পূর্ণাঙ্ক sqlite3VdbeNextOpcode(Vdbe*,Mem*,পূর্ণাঙ্ক,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*,Op**);
ৱর্ণ *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#endif
#যদি defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
ৱর্ণ *sqlite3VdbeDisplayComment(sqlite3*,ধ্রুৱ Op*,ধ্রুৱ ৱর্ণ*);
#endif
#যদি !defined(SQLITE_OMIT_EXPLAIN)
পূর্ণাঙ্ক sqlite3VdbeList(Vdbe*);
#endif
পূর্ণাঙ্ক sqlite3VdbeHalt(Vdbe*);
পূর্ণাঙ্ক sqlite3VdbeChangeEncoding(Mem *, পূর্ণাঙ্ক);
পূর্ণাঙ্ক sqlite3VdbeMemTooBig(Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemCopy(Mem*, ধ্রুৱ Mem*);
শূন্য sqlite3VdbeMemShallowCopy(Mem*, ধ্রুৱ Mem*, পূর্ণাঙ্ক);
শূন্য sqlite3VdbeMemMove(Mem*, Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemNulTerminate(Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemSetStr(Mem*, ধ্রুৱ ৱর্ণ*, i64, u8, শূন্য(*)(শূন্য*));
পূর্ণাঙ্ক sqlite3VdbeMemSetText(Mem*, ধ্রুৱ ৱর্ণ*, i64, শূন্য(*)(শূন্য*));
শূন্য sqlite3VdbeMemSetInt64(Mem*, i64);
#ifdef SQLITE_OMIT_FLOATING_POINT
# define sqlite3VdbeMemSetDouble sqlite3VdbeMemSetInt64
#অন্যথা
  শূন্য sqlite3VdbeMemSetDouble(Mem*, দ্ৱিগুণ);
#endif
শূন্য sqlite3VdbeMemSetPointer(Mem*, শূন্য*, ধ্রুৱ ৱর্ণ*, শূন্য(*)(শূন্য*));
শূন্য sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
শূন্য sqlite3VdbeMemSetNull(Mem*);
#ifndef SQLITE_OMIT_INCRBLOB
শূন্য sqlite3VdbeMemSetZeroBlob(Mem*,পূর্ণাঙ্ক);
#অন্যথা
পূর্ণাঙ্ক sqlite3VdbeMemSetZeroBlob(Mem*,পূর্ণাঙ্ক);
#endif
#ifdef SQLITE_DEBUG
পূর্ণাঙ্ক sqlite3VdbeMemIsRowSet(ধ্রুৱ Mem*);
#endif
পূর্ণাঙ্ক sqlite3VdbeMemSetRowSet(Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemZeroTerminateIfAble(Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemMakeWriteable(Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemStringify(Mem*, u8, u8);
পূর্ণাঙ্ক sqlite3IntFloatCompare(i64,দ্ৱিগুণ);
i64 sqlite3_Vdbe_Int_মান(ধ্রুৱ Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemIntegerify(Mem*);
দ্ৱিগুণ sqlite3_Vdbe_Real_মান(Mem*);
পূর্ণাঙ্ক sqlite3_Mem_Real_মান_RC(Mem*, দ্ৱিগুণ*);
পূর্ণাঙ্ক sqlite3_Vdbe_Boolean_মান(Mem*, পূর্ণাঙ্ক ifNull);
শূন্য sqlite3VdbeIntegerAffinity(Mem*,পূর্ণাঙ্ক bFromText);
পূর্ণাঙ্ক sqlite3VdbeMemRealify(Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemNumerify(Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemCast(Mem*,u8,u8);
পূর্ণাঙ্ক sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
পূর্ণাঙ্ক sqlite3VdbeMemFromBtreeZeroOffset(BtCursor*,u32,Mem*);
শূন্য sqlite3VdbeMemRelease(Mem *p);
শূন্য sqlite3VdbeMemReleaseMalloc(Mem*p);
পূর্ণাঙ্ক sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#ifndef SQLITE_OMIT_WINDOWFUNC
পূর্ণাঙ্ক sqlite3_Vdbe_Mem_Agg_মান(Mem*, Mem*, FuncDef*);
#endif
#যদি !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
ধ্রুৱ ৱর্ণ *sqlite3OpcodeName(পূর্ণাঙ্ক);
#endif
পূর্ণাঙ্ক sqlite3VdbeMemGrow(Mem *pMem, পূর্ণাঙ্ক n, পূর্ণাঙ্ক preserve);
পূর্ণাঙ্ক sqlite3VdbeMemClearAndResize(Mem *pMem, পূর্ণাঙ্ক n);
পূর্ণাঙ্ক sqlite3VdbeCloseStatement(Vdbe *, পূর্ণাঙ্ক);
#ifdef SQLITE_DEBUG
পূর্ণাঙ্ক sqlite3VdbeFrameIsValid(VdbeFrame*);
#endif
শূন্য sqlite3VdbeFrameMemDel(শূন্য*);      /* Destructor on Mem */
শূন্য sqlite3VdbeFrameDelete(VdbeFrame*); /* Actually deletes the Frame */
পূর্ণাঙ্ক sqlite3VdbeFrameRestore(VdbeFrame *);
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
শূন্য sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ*,Table*,i64,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
#endif
পূর্ণাঙ্ক sqlite3VdbeTransferError(Vdbe *p);
পূর্ণাঙ্ক sqlite3_Vdbe_Find_Index_চাবি(BtCursor*, Index*, UnpackedRecord*, পূর্ণাঙ্ক*, পূর্ণাঙ্ক);

পূর্ণাঙ্ক sqlite3VdbeSorterInit(sqlite3 *, পূর্ণাঙ্ক, VdbeCursor *);
শূন্য sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
শূন্য sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
পূর্ণাঙ্ক sqlite3VdbeSorterRowkey(ধ্রুৱ VdbeCursor *, Mem *);
পূর্ণাঙ্ক sqlite3VdbeSorterNext(sqlite3 *, ধ্রুৱ VdbeCursor *);
পূর্ণাঙ্ক sqlite3VdbeSorterRewind(ধ্রুৱ VdbeCursor *, পূর্ণাঙ্ক *);
পূর্ণাঙ্ক sqlite3VdbeSorterWrite(ধ্রুৱ VdbeCursor *, Mem *);
পূর্ণাঙ্ক sqlite3VdbeSorterCompare(ধ্রুৱ VdbeCursor *, Mem *, পূর্ণাঙ্ক, পূর্ণাঙ্ক *);

শূন্য sqlite3_Vdbe_মান_List_Free(শূন্য*);

#ifdef SQLITE_DEBUG
  শূন্য sqlite3VdbeIncrWriteCounter(Vdbe*, VdbeCursor*);
  শূন্য sqlite3VdbeAssertAbortable(Vdbe*);
#অন্যথা
# define sqlite3VdbeIncrWriteCounter(V,C)
# define sqlite3VdbeAssertAbortable(V)
#endif

#যদি !defined(SQLITE_OMIT_SHARED_CACHE)
  শূন্য sqlite3VdbeEnter(Vdbe*);
#অন্যথা
# define sqlite3VdbeEnter(X)
#endif

#যদি !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  শূন্য sqlite3VdbeLeave(Vdbe*);
#অন্যথা
# define sqlite3VdbeLeave(X)
#endif

#ifdef SQLITE_DEBUG
শূন্য sqlite3VdbeMemAboutToChange(Vdbe*,Mem*);
পূর্ণাঙ্ক sqlite3VdbeCheckMemInvariants(Mem*);
#endif

#ifndef SQLITE_OMIT_FOREIGN_চাবি
পূর্ণাঙ্ক sqlite3VdbeCheckFkImmediate(Vdbe*);
পূর্ণাঙ্ক sqlite3VdbeCheckFkDeferred(Vdbe*);
#অন্যথা
# define sqlite3VdbeCheckFkImmediate(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#endif

#ifdef SQLITE_DEBUG
  শূন্য sqlite3VdbePrintSql(Vdbe*);
  শূন্য sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#endif
#ifndef SQLITE_OMIT_UTF16
  পূর্ণাঙ্ক sqlite3VdbeMemTranslate(Mem*, u8);
  পূর্ণাঙ্ক sqlite3VdbeMemHandleBom(Mem *pMem);
#endif

#ifndef SQLITE_OMIT_INCRBLOB
  পূর্ণাঙ্ক sqlite3VdbeMemExpandBlob(Mem *);
  #define ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#অন্যথা
  #define sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #define ExpandBlob(P) SQLITE_OK
#endif

#endif /* !defined(SQLITE_VDBEINT_H) */
