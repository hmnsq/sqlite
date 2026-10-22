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
#nếu !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#không_thì
# define VDBE_DISPLAY_P4 0
#endif

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
typedef struct VdbeOp Op;

/*
** Boolean values
*/
typedef unsigned Bool;

/* Opaque type used by code in vdbesort.c */
typedef struct VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
typedef struct AuxData AuxData;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
typedef struct VdbeTxtBlbCache VdbeTxtBlbCache;

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
typedef struct VdbeCursor VdbeCursor;
struct VdbeCursor {
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
  union {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seqCount;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cacheStatus;        /* Cache is valid if this matches Vdbe.cacheCtr */
  số_nguyên seekResult;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match".
  ** For CURTYPE_PSEUDO, seekResult is the register holding the record */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  union {
    BtCursor *pCursor;          /* CURTYPE_BTREE or _PSEUDO.  Btree cursor */
    sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.              Vtab cursor */
    VdbeSorter *pSorter;        /* CURTYPE_SORTER.            Sorter object */
  } uc;
  KeyInfo *pKeyInfo;      /* Info about index keys needed by index cursors */
  u32 iHdrOffset;         /* Offset to next unparsed byte of the header */
  Pgno pgnoRoot;          /* Root page of the open btree cursor */
  i16 nField;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 movetoTarget;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *aOffset;           /* Pointer to aType[nField] */
  const u8 *aRow;         /* Data for the current row, if all on one page */
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
    (ROUND8(offsetof(VdbeCursor,aType)) + ((N)+1)*sizeof(u64))

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
struct VdbeTxtBlbCache {
  char *pCValue;        /* A RCStr buffer to hold the value */
  i64 iOffset;          /* File offset of the row being cached */
  số_nguyên iCol;             /* Column for which the cache is valid */
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
typedef struct VdbeFrame VdbeFrame;
struct VdbeFrame {
  Vdbe *v;                /* VM this frame belongs to */
  VdbeFrame *pParent;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  void *token;            /* Copy of SubProgram.token */
  i64 lastRowid;          /* Last insert rowid (sqlite3.lastRowid) */
  AuxData *pAuxData;      /* Linked list of auxdata allocations */
#nếu SQLITE_DEBUG
  u32 iFrameMagic;        /* magic number for sanity checking */
#endif
  số_nguyên nCursor;            /* Number of entries in apCsr */
  số_nguyên pc;                 /* Program Counter in parent (calling) frame */
  số_nguyên nOp;                /* Size of aOp array */
  số_nguyên nMem;               /* Number of entries in aMem */
  số_nguyên nChildMem;          /* Number of memory cells for child frame */
  số_nguyên nChildCsr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#define SQLITE_FRAME_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#define VdbeFrameMem(p) ((Mem *)&((u8 *)p)[ROUND8(sizeof(VdbeFrame))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
struct sqlite3_value {
  union MemValue {
    double r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    số_nguyên nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    const char *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  char *z;            /* String or BLOB value */
  số_nguyên n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  số_nguyên szMalloc;       /* Size of the zMalloc allocation */
  u32 uTemp;          /* Transient storage for serial_type in OP_MakeRecord */
  char *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  void (*xDel)(void*);/* Destructor for Mem.z - only valid if MEM_Dyn */
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
struct AuxData {
  số_nguyên iAuxOp;                     /* Instruction number of OP_Function opcode */
  số_nguyên iAuxArg;                    /* Index of function argument. */
  void *pAux;                     /* Aux data pointer */
  void (*xDeleteAux)(void*);      /* Destructor for the aux data */
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
struct sqlite3_context {
  Mem *pOut;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  số_nguyên iOp;                /* Instruction number of OP_Function */
  số_nguyên isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_value *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#define SZ_CONTEXT(N)  \
   (offsetof(sqlite3_context,argv)+(N)*sizeof(sqlite3_value*))


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
typedef struct ScanStatus ScanStatus;
struct ScanStatus {
  số_nguyên addrExplain;                /* OP_Explain for loop */
  số_nguyên aAddrRange[6];
  số_nguyên addrLoop;                   /* Address of "loops" counter */
  số_nguyên addrVisit;                  /* Address of "rows visited" counter */
  số_nguyên iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  char *zName;                    /* Name of table or index */
};

/* The DblquoteStr object holds the text of a double-quoted
** string for a prepared statement.  A linked list of these objects
** is constructed during statement parsing and is held on Vdbe.pDblStr.
** When computing a normalized SQL statement for an SQL statement, that
** list is consulted for each double-quoted identifier to see if the
** identifier should really be a string literal.
*/
typedef struct DblquoteStr DblquoteStr;
struct DblquoteStr {
  DblquoteStr *pNextStr;   /* Next string literal in the list */
  char z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
struct Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  số_nguyên nMem;               /* Number of memory locations currently allocated */
  số_nguyên nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  số_nguyên pc;                 /* The program counter */
  số_nguyên rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  số_nguyên iStatement;         /* Statement number (or 0 if has no opened stmt) */
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
  số_nguyên nOp;                /* Number of instructions in the program */
  số_nguyên nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *aColName;          /* Column names to return */
  Mem *pResultRow;        /* Current output row */
  char *zErrMsg;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#ifndef SQLITE_OMIT_TRACE
  i64 startTime;          /* Time when query started - used for profiling */
#endif
#ifdef SQLITE_DEBUG
  số_nguyên rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  số_nguyên napArg;             /* Size of the apArg[] array */
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
  char *zSql;             /* Text of the SQL statement that generated this */
#ifdef SQLITE_ENABLE_NORMALIZE
  char *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#endif
  void *pFree;            /* Free this when deleting the vdbe */
  VdbeFrame *pFrame;      /* Parent frame */
  VdbeFrame *pDelFrame;   /* List of frame objects to free on VM reset */
  số_nguyên nFrame;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  SubProgram *pProgram;   /* Linked list of all sub-programs used by VM */
  AuxData *pAuxData;      /* Linked list of auxdata allocations */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  số_nguyên nScan;              /* Entries in aScan[] */
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
struct PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  số_nguyên op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *aRecord;                    /* old.* database record */
  KeyInfo *pKeyinfo;              /* Key information */
  UnpackedRecord *pUnpacked;      /* Unpacked version of aRecord[] */
  UnpackedRecord *pNewUnpacked;   /* Unpacked version of new.* record */
  số_nguyên iNewReg;                    /* Register for new.* values */
  số_nguyên iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  Table *pTab;                    /* Schema object being updated */
  Index *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_value **apDflt;         /* Array of default values, if required */
  struct {
    u8 keyinfoSpace[SZ_KEYINFO_0];  /* Space to hold pKeyinfo[0] content */
  } uKey;
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
typedef struct ValueList ValueList;
struct ValueList {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_value *pOut;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#ifndef SQLITE_AMALGAMATION
extern const u8 sqlite3SmallTypeSizes[];
#endif

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#define ONE_BYTE_INT(x)    ((i8)(x)[0])
#define TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#define THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#define FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define FOUR_BYTE_INT(x)   ((số_nguyên)FOUR_BYTE_UINT(x))
#define SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
void sqlite3VdbeError(Vdbe*, const char *, ...);
void sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
void sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
void sqliteVdbePopStack(Vdbe*,số_nguyên);
số_nguyên SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
số_nguyên SQLITE_NOINLINE sqlite3VdbeFinishMoveto(VdbeCursor*);
số_nguyên sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#ifdef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
  u64 sqlite3FloatSwap(u64 in);
# define swapMixedEndianFloat(X)  X = sqlite3FloatSwap(X)
#không_thì
# define swapMixedEndianFloat(X)
#endif
void sqlite3VdbeSerialGet(const unsigned char*, u32, Mem*);
void sqlite3VdbeDeleteAuxData(sqlite3*, AuxData**, số_nguyên, số_nguyên);

số_nguyên sqlite2BtreeKeyCompare(BtCursor *, const void *, số_nguyên, số_nguyên, số_nguyên *);
số_nguyên sqlite3VdbeIdxKeyCompare(sqlite3*,VdbeCursor*,UnpackedRecord*,số_nguyên*);
số_nguyên sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
số_nguyên sqlite3VdbeExec(Vdbe*);
#nếu !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
số_nguyên sqlite3VdbeNextOpcode(Vdbe*,Mem*,số_nguyên,số_nguyên*,số_nguyên*,Op**);
char *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#endif
#nếu defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
char *sqlite3VdbeDisplayComment(sqlite3*,const Op*,const char*);
#endif
#nếu !defined(SQLITE_OMIT_EXPLAIN)
số_nguyên sqlite3VdbeList(Vdbe*);
#endif
số_nguyên sqlite3VdbeHalt(Vdbe*);
số_nguyên sqlite3VdbeChangeEncoding(Mem *, số_nguyên);
số_nguyên sqlite3VdbeMemTooBig(Mem*);
số_nguyên sqlite3VdbeMemCopy(Mem*, const Mem*);
void sqlite3VdbeMemShallowCopy(Mem*, const Mem*, số_nguyên);
void sqlite3VdbeMemMove(Mem*, Mem*);
số_nguyên sqlite3VdbeMemNulTerminate(Mem*);
số_nguyên sqlite3VdbeMemSetStr(Mem*, const char*, i64, u8, void(*)(void*));
số_nguyên sqlite3VdbeMemSetText(Mem*, const char*, i64, void(*)(void*));
void sqlite3VdbeMemSetInt64(Mem*, i64);
#ifdef SQLITE_OMIT_FLOATING_POINT
# define sqlite3VdbeMemSetDouble sqlite3VdbeMemSetInt64
#không_thì
  void sqlite3VdbeMemSetDouble(Mem*, double);
#endif
void sqlite3VdbeMemSetPointer(Mem*, void*, const char*, void(*)(void*));
void sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
void sqlite3VdbeMemSetNull(Mem*);
#ifndef SQLITE_OMIT_INCRBLOB
void sqlite3VdbeMemSetZeroBlob(Mem*,số_nguyên);
#không_thì
số_nguyên sqlite3VdbeMemSetZeroBlob(Mem*,số_nguyên);
#endif
#ifdef SQLITE_DEBUG
số_nguyên sqlite3VdbeMemIsRowSet(const Mem*);
#endif
số_nguyên sqlite3VdbeMemSetRowSet(Mem*);
số_nguyên sqlite3VdbeMemZeroTerminateIfAble(Mem*);
số_nguyên sqlite3VdbeMemMakeWriteable(Mem*);
số_nguyên sqlite3VdbeMemStringify(Mem*, u8, u8);
số_nguyên sqlite3IntFloatCompare(i64,double);
i64 sqlite3VdbeIntValue(const Mem*);
số_nguyên sqlite3VdbeMemIntegerify(Mem*);
double sqlite3VdbeRealValue(Mem*);
số_nguyên sqlite3MemRealValueRC(Mem*, double*);
số_nguyên sqlite3VdbeBooleanValue(Mem*, số_nguyên ifNull);
void sqlite3VdbeIntegerAffinity(Mem*,số_nguyên bFromText);
số_nguyên sqlite3VdbeMemRealify(Mem*);
số_nguyên sqlite3VdbeMemNumerify(Mem*);
số_nguyên sqlite3VdbeMemCast(Mem*,u8,u8);
số_nguyên sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
số_nguyên sqlite3VdbeMemFromBtreeZeroOffset(BtCursor*,u32,Mem*);
void sqlite3VdbeMemRelease(Mem *p);
void sqlite3VdbeMemReleaseMalloc(Mem*p);
số_nguyên sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#ifndef SQLITE_OMIT_WINDOWFUNC
số_nguyên sqlite3VdbeMemAggValue(Mem*, Mem*, FuncDef*);
#endif
#nếu !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
const char *sqlite3OpcodeName(số_nguyên);
#endif
số_nguyên sqlite3VdbeMemGrow(Mem *pMem, số_nguyên n, số_nguyên preserve);
số_nguyên sqlite3VdbeMemClearAndResize(Mem *pMem, số_nguyên n);
số_nguyên sqlite3VdbeCloseStatement(Vdbe *, số_nguyên);
#ifdef SQLITE_DEBUG
số_nguyên sqlite3VdbeFrameIsValid(VdbeFrame*);
#endif
void sqlite3VdbeFrameMemDel(void*);      /* Destructor on Mem */
void sqlite3VdbeFrameDelete(VdbeFrame*); /* Actually deletes the Frame */
số_nguyên sqlite3VdbeFrameRestore(VdbeFrame *);
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
void sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,số_nguyên,const char*,Table*,i64,số_nguyên,số_nguyên);
#endif
số_nguyên sqlite3VdbeTransferError(Vdbe *p);
số_nguyên sqlite3VdbeFindIndexKey(BtCursor*, Index*, UnpackedRecord*, số_nguyên*, số_nguyên);

số_nguyên sqlite3VdbeSorterInit(sqlite3 *, số_nguyên, VdbeCursor *);
void sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
void sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
số_nguyên sqlite3VdbeSorterRowkey(const VdbeCursor *, Mem *);
số_nguyên sqlite3VdbeSorterNext(sqlite3 *, const VdbeCursor *);
số_nguyên sqlite3VdbeSorterRewind(const VdbeCursor *, số_nguyên *);
số_nguyên sqlite3VdbeSorterWrite(const VdbeCursor *, Mem *);
số_nguyên sqlite3VdbeSorterCompare(const VdbeCursor *, Mem *, số_nguyên, số_nguyên *);

void sqlite3VdbeValueListFree(void*);

#ifdef SQLITE_DEBUG
  void sqlite3VdbeIncrWriteCounter(Vdbe*, VdbeCursor*);
  void sqlite3VdbeAssertAbortable(Vdbe*);
#không_thì
# define sqlite3VdbeIncrWriteCounter(V,C)
# define sqlite3VdbeAssertAbortable(V)
#endif

#nếu !defined(SQLITE_OMIT_SHARED_CACHE)
  void sqlite3VdbeEnter(Vdbe*);
#không_thì
# define sqlite3VdbeEnter(X)
#endif

#nếu !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  void sqlite3VdbeLeave(Vdbe*);
#không_thì
# define sqlite3VdbeLeave(X)
#endif

#ifdef SQLITE_DEBUG
void sqlite3VdbeMemAboutToChange(Vdbe*,Mem*);
số_nguyên sqlite3VdbeCheckMemInvariants(Mem*);
#endif

#ifndef SQLITE_OMIT_FOREIGN_KEY
số_nguyên sqlite3VdbeCheckFkImmediate(Vdbe*);
số_nguyên sqlite3VdbeCheckFkDeferred(Vdbe*);
#không_thì
# define sqlite3VdbeCheckFkImmediate(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#endif

#ifdef SQLITE_DEBUG
  void sqlite3VdbePrintSql(Vdbe*);
  void sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#endif
#ifndef SQLITE_OMIT_UTF16
  số_nguyên sqlite3VdbeMemTranslate(Mem*, u8);
  số_nguyên sqlite3VdbeMemHandleBom(Mem *pMem);
#endif

#ifndef SQLITE_OMIT_INCRBLOB
  số_nguyên sqlite3VdbeMemExpandBlob(Mem *);
  #define ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#không_thì
  #define sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #define ExpandBlob(P) SQLITE_OK
#endif

#endif /* !defined(SQLITE_VDBEINT_H) */
