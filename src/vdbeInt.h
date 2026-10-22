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
#यदि !defined(SQLITE_OMIT_EXPLAIN) || !defined(न_संशोधन) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#अन्यथा
# define VDBE_DISPLAY_P4 0
#endif

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
प्रकारनिर्देश संरचना VdbeOp Op;

/*
** Boolean values
*/
प्रकारनिर्देश अचिन्हित Bool;

/* Opaque type used by code in vdbesort.c */
प्रकारनिर्देश संरचना VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
प्रकारनिर्देश संरचना AuxData AuxData;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
प्रकारनिर्देश संरचना VdbeTxtBlbCache VdbeTxtBlbCache;

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
प्रकारनिर्देश संरचना VdbeCursor VdbeCursor;
संरचना VdbeCursor {
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
  सङ्घ {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seq_गिनती;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cacheStatus;        /* Cache is valid if this matches Vdbe.cacheCtr */
  पूर्णाङ्क seekResult;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match".
  ** For CURTYPE_PSEUDO, seekResult is the register holding the record */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  सङ्घ {
    BtCursor *pCursor;          /* CURTYPE_BTREE or _PSEUDO.  Btree cursor */
    sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.              Vtab cursor */
    VdbeSorter *pSorter;        /* CURTYPE_SORTER.            Sorter object */
  } uc;
  कुंजी_Info *p_कुंजी_Info;      /* Info about index keys needed by index cursors */
  u32 iHdrOffset;         /* Offset to next unparsed byte of the header */
  Pgno pgno_मूल;          /* Root page of the open btree cursor */
  i16 nField;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 movetoTarget;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *aOffset;           /* Pointer to aType[nField] */
  ध्रुव u8 *aRow;         /* Data for the current row, if all on one page */
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
    (ROUND8(दुरत्व(VdbeCursor,aType)) + ((N)+1)*परिमाण(u64))

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
संरचना VdbeTxtBlbCache {
  वर्ण *pCValue;        /* A RCStr buffer to hold the value */
  i64 iOffset;          /* File offset of the row being cached */
  पूर्णाङ्क iCol;             /* Column for which the cache is valid */
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
प्रकारनिर्देश संरचना VdbeFrame VdbeFrame;
संरचना VdbeFrame {
  Vdbe *v;                /* VM this frame belongs to */
  VdbeFrame *pParent;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  शून्य *token;            /* Copy of SubProgram.token */
  i64 lastRowid;          /* Last insert rowid (sqlite3.lastRowid) */
  AuxData *pAuxData;      /* Linked list of auxdata allocations */
#यदि SQLITE_DEBUG
  u32 iFrameMagic;        /* magic number for sanity checking */
#endif
  पूर्णाङ्क nCursor;            /* Number of entries in apCsr */
  पूर्णाङ्क pc;                 /* Program Counter in parent (calling) frame */
  पूर्णाङ्क nOp;                /* Size of aOp array */
  पूर्णाङ्क nMem;               /* Number of entries in aMem */
  पूर्णाङ्क nChildMem;          /* Number of memory cells for child frame */
  पूर्णाङ्क nChildCsr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#define SQLITE_FRAME_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#define VdbeFrameMem(p) ((Mem *)&((u8 *)p)[ROUND8(परिमाण(VdbeFrame))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
संरचना sqlite3_मूल्य {
  सङ्घ Mem_मूल्य {
    द्विगुण r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    पूर्णाङ्क nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    ध्रुव वर्ण *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  वर्ण *z;            /* String or BLOB value */
  पूर्णाङ्क n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  पूर्णाङ्क szMalloc;       /* Size of the zMalloc allocation */
  u32 uTemp;          /* Transient storage for serial_type in OP_MakeRecord */
  वर्ण *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  शून्य (*xDel)(शून्य*);/* Destructor for Mem.z - only valid if MEM_Dyn */
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
#define MEMCELLSIZE दुरत्व(Mem,db)

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
संरचना AuxData {
  पूर्णाङ्क iAuxOp;                     /* Instruction number of OP_Function opcode */
  पूर्णाङ्क iAuxArg;                    /* Index of function argument. */
  शून्य *pAux;                     /* Aux data pointer */
  शून्य (*xDeleteAux)(शून्य*);      /* Destructor for the aux data */
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
संरचना sqlite3_context {
  Mem *pOut;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  पूर्णाङ्क iOp;                /* Instruction number of OP_Function */
  पूर्णाङ्क isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_मूल्य *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#define SZ_CONTEXT(N)  \
   (दुरत्व(sqlite3_context,argv)+(N)*परिमाण(sqlite3_मूल्य*))


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
प्रकारनिर्देश संरचना ScanStatus ScanStatus;
संरचना ScanStatus {
  पूर्णाङ्क addrExplain;                /* OP_Explain for loop */
  पूर्णाङ्क aAddrRange[6];
  पूर्णाङ्क addrLoop;                   /* Address of "loops" counter */
  पूर्णाङ्क addrVisit;                  /* Address of "rows visited" counter */
  पूर्णाङ्क iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  वर्ण *zName;                    /* Name of table or index */
};

/* The DblquoteStr object holds the text of a double-quoted
** string for a prepared statement.  A linked list of these objects
** is constructed during statement parsing and is held on Vdbe.pDblStr.
** When computing a normalized SQL statement for an SQL statement, that
** list is consulted for each double-quoted identifier to see if the
** identifier should really be a string literal.
*/
प्रकारनिर्देश संरचना DblquoteStr DblquoteStr;
संरचना DblquoteStr {
  DblquoteStr *pNextStr;   /* Next string literal in the list */
  वर्ण z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
संरचना Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  पूर्णाङ्क nMem;               /* Number of memory locations currently allocated */
  पूर्णाङ्क nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  पूर्णाङ्क pc;                 /* The program counter */
  पूर्णाङ्क rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  पूर्णाङ्क iStatement;         /* Statement number (or 0 if has no opened stmt) */
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
  पूर्णाङ्क nOp;                /* Number of instructions in the program */
  पूर्णाङ्क nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *aColName;          /* Column names to return */
  Mem *pResultRow;        /* Current output row */
  वर्ण *z_Err_संदेश;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#ifndef SQLITE_OMIT_TRACE
  i64 startTime;          /* Time when query started - used for profiling */
#endif
#ifdef SQLITE_DEBUG
  पूर्णाङ्क rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  पूर्णाङ्क napArg;             /* Size of the apArg[] array */
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
  वर्ण *zSql;             /* Text of the SQL statement that generated this */
#ifdef SQLITE_ENABLE_NORMALIZE
  वर्ण *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#endif
  शून्य *pFree;            /* Free this when deleting the vdbe */
  VdbeFrame *pFrame;      /* Parent frame */
  VdbeFrame *pDelFrame;   /* List of frame objects to free on VM reset */
  पूर्णाङ्क nFrame;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  SubProgram *pProgram;   /* Linked list of all sub-programs used by VM */
  AuxData *pAuxData;      /* Linked list of auxdata allocations */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णाङ्क nScan;              /* Entries in aScan[] */
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
संरचना PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  पूर्णाङ्क op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *aRecord;                    /* old.* database record */
  कुंजी_Info *pKeyinfo;              /* Key information */
  UnpackedRecord *pUnpacked;      /* Unpacked version of aRecord[] */
  UnpackedRecord *pNewUnpacked;   /* Unpacked version of new.* record */
  पूर्णाङ्क iNewReg;                    /* Register for new.* values */
  पूर्णाङ्क iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  Table *pTab;                    /* Schema object being updated */
  Index *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_मूल्य **apDflt;         /* Array of default values, if required */
  संरचना {
    u8 keyinfoSpace[SZ_KEYINFO_0];  /* Space to hold pKeyinfo[0] content */
  } u_कुंजी;
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
प्रकारनिर्देश संरचना मूल्य_List मूल्य_List;
संरचना मूल्य_List {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_मूल्य *pOut;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#ifndef SQLITE_AMALGAMATION
बाह्य ध्रुव u8 sqlite3SmallTypeSizes[];
#endif

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#define ONE_BYTE_INT(x)    ((i8)(x)[0])
#define TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#define THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#define FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define FOUR_BYTE_INT(x)   ((पूर्णाङ्क)FOUR_BYTE_UINT(x))
#define SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
शून्य sqlite3VdbeError(Vdbe*, ध्रुव वर्ण *, ...);
शून्य sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
शून्य sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
शून्य sqlite_Vdbe_निकालो_ढेर(Vdbe*,पूर्णाङ्क);
पूर्णाङ्क SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
पूर्णाङ्क SQLITE_NOINLINE sqlite3VdbeFinishMoveto(VdbeCursor*);
पूर्णाङ्क sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#ifdef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
  u64 sqlite3FloatSwap(u64 in);
# define swapMixedEndianFloat(X)  X = sqlite3FloatSwap(X)
#अन्यथा
# define swapMixedEndianFloat(X)
#endif
शून्य sqlite3VdbeSerialGet(ध्रुव अचिन्हित वर्ण*, u32, Mem*);
शून्य sqlite3VdbeDeleteAuxData(sqlite3*, AuxData**, पूर्णाङ्क, पूर्णाङ्क);

पूर्णाङ्क sqlite2_Btree_कुंजी_Compare(BtCursor *, ध्रुव शून्य *, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क *);
पूर्णाङ्क sqlite3_Vdbe_Idx_कुंजी_Compare(sqlite3*,VdbeCursor*,UnpackedRecord*,पूर्णाङ्क*);
पूर्णाङ्क sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
पूर्णाङ्क sqlite3VdbeExec(Vdbe*);
#यदि !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
पूर्णाङ्क sqlite3VdbeNextOpcode(Vdbe*,Mem*,पूर्णाङ्क,पूर्णाङ्क*,पूर्णाङ्क*,Op**);
वर्ण *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#endif
#यदि defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
वर्ण *sqlite3VdbeDisplayComment(sqlite3*,ध्रुव Op*,ध्रुव वर्ण*);
#endif
#यदि !defined(SQLITE_OMIT_EXPLAIN)
पूर्णाङ्क sqlite3VdbeList(Vdbe*);
#endif
पूर्णाङ्क sqlite3VdbeHalt(Vdbe*);
पूर्णाङ्क sqlite3VdbeChangeEncoding(Mem *, पूर्णाङ्क);
पूर्णाङ्क sqlite3VdbeMemTooBig(Mem*);
पूर्णाङ्क sqlite3VdbeMemCopy(Mem*, ध्रुव Mem*);
शून्य sqlite3VdbeMemShallowCopy(Mem*, ध्रुव Mem*, पूर्णाङ्क);
शून्य sqlite3VdbeMemMove(Mem*, Mem*);
पूर्णाङ्क sqlite3VdbeMemNulTerminate(Mem*);
पूर्णाङ्क sqlite3VdbeMemSetStr(Mem*, ध्रुव वर्ण*, i64, u8, शून्य(*)(शून्य*));
पूर्णाङ्क sqlite3VdbeMemSetText(Mem*, ध्रुव वर्ण*, i64, शून्य(*)(शून्य*));
शून्य sqlite3VdbeMemSetInt64(Mem*, i64);
#ifdef SQLITE_OMIT_FLOATING_POINT
# define sqlite3VdbeMemSetDouble sqlite3VdbeMemSetInt64
#अन्यथा
  शून्य sqlite3VdbeMemSetDouble(Mem*, द्विगुण);
#endif
शून्य sqlite3VdbeMemSetPointer(Mem*, शून्य*, ध्रुव वर्ण*, शून्य(*)(शून्य*));
शून्य sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
शून्य sqlite3VdbeMemSetNull(Mem*);
#ifndef SQLITE_OMIT_INCRBLOB
शून्य sqlite3VdbeMemSetZeroBlob(Mem*,पूर्णाङ्क);
#अन्यथा
पूर्णाङ्क sqlite3VdbeMemSetZeroBlob(Mem*,पूर्णाङ्क);
#endif
#ifdef SQLITE_DEBUG
पूर्णाङ्क sqlite3VdbeMemIsRowSet(ध्रुव Mem*);
#endif
पूर्णाङ्क sqlite3VdbeMemSetRowSet(Mem*);
पूर्णाङ्क sqlite3VdbeMemZeroTerminateIfAble(Mem*);
पूर्णाङ्क sqlite3VdbeMemMakeWriteable(Mem*);
पूर्णाङ्क sqlite3VdbeMemStringify(Mem*, u8, u8);
पूर्णाङ्क sqlite3IntFloatCompare(i64,द्विगुण);
i64 sqlite3_Vdbe_Int_मूल्य(ध्रुव Mem*);
पूर्णाङ्क sqlite3VdbeMemIntegerify(Mem*);
द्विगुण sqlite3_Vdbe_Real_मूल्य(Mem*);
पूर्णाङ्क sqlite3_Mem_Real_मूल्य_RC(Mem*, द्विगुण*);
पूर्णाङ्क sqlite3_Vdbe_Boolean_मूल्य(Mem*, पूर्णाङ्क ifNull);
शून्य sqlite3VdbeIntegerAffinity(Mem*,पूर्णाङ्क bFromText);
पूर्णाङ्क sqlite3VdbeMemRealify(Mem*);
पूर्णाङ्क sqlite3VdbeMemNumerify(Mem*);
पूर्णाङ्क sqlite3VdbeMemCast(Mem*,u8,u8);
पूर्णाङ्क sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
पूर्णाङ्क sqlite3VdbeMemFromBtreeZeroOffset(BtCursor*,u32,Mem*);
शून्य sqlite3VdbeMemRelease(Mem *p);
शून्य sqlite3VdbeMemReleaseMalloc(Mem*p);
पूर्णाङ्क sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#ifndef SQLITE_OMIT_WINDOWFUNC
पूर्णाङ्क sqlite3_Vdbe_Mem_Agg_मूल्य(Mem*, Mem*, FuncDef*);
#endif
#यदि !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
ध्रुव वर्ण *sqlite3OpcodeName(पूर्णाङ्क);
#endif
पूर्णाङ्क sqlite3VdbeMemGrow(Mem *pMem, पूर्णाङ्क n, पूर्णाङ्क preserve);
पूर्णाङ्क sqlite3VdbeMemClearAndResize(Mem *pMem, पूर्णाङ्क n);
पूर्णाङ्क sqlite3VdbeCloseStatement(Vdbe *, पूर्णाङ्क);
#ifdef SQLITE_DEBUG
पूर्णाङ्क sqlite3VdbeFrameIsValid(VdbeFrame*);
#endif
शून्य sqlite3VdbeFrameMemDel(शून्य*);      /* Destructor on Mem */
शून्य sqlite3VdbeFrameDelete(VdbeFrame*); /* Actually deletes the Frame */
पूर्णाङ्क sqlite3VdbeFrameRestore(VdbeFrame *);
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
शून्य sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,पूर्णाङ्क,ध्रुव वर्ण*,Table*,i64,पूर्णाङ्क,पूर्णाङ्क);
#endif
पूर्णाङ्क sqlite3VdbeTransferError(Vdbe *p);
पूर्णाङ्क sqlite3_Vdbe_Find_Index_कुंजी(BtCursor*, Index*, UnpackedRecord*, पूर्णाङ्क*, पूर्णाङ्क);

पूर्णाङ्क sqlite3VdbeSorterInit(sqlite3 *, पूर्णाङ्क, VdbeCursor *);
शून्य sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
शून्य sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
पूर्णाङ्क sqlite3VdbeSorterRowkey(ध्रुव VdbeCursor *, Mem *);
पूर्णाङ्क sqlite3VdbeSorterNext(sqlite3 *, ध्रुव VdbeCursor *);
पूर्णाङ्क sqlite3VdbeSorterRewind(ध्रुव VdbeCursor *, पूर्णाङ्क *);
पूर्णाङ्क sqlite3VdbeSorterWrite(ध्रुव VdbeCursor *, Mem *);
पूर्णाङ्क sqlite3VdbeSorterCompare(ध्रुव VdbeCursor *, Mem *, पूर्णाङ्क, पूर्णाङ्क *);

शून्य sqlite3_Vdbe_मूल्य_List_Free(शून्य*);

#ifdef SQLITE_DEBUG
  शून्य sqlite3VdbeIncrWriteCounter(Vdbe*, VdbeCursor*);
  शून्य sqlite3VdbeAssertAbortable(Vdbe*);
#अन्यथा
# define sqlite3VdbeIncrWriteCounter(V,C)
# define sqlite3VdbeAssertAbortable(V)
#endif

#यदि !defined(SQLITE_OMIT_SHARED_CACHE)
  शून्य sqlite3VdbeEnter(Vdbe*);
#अन्यथा
# define sqlite3VdbeEnter(X)
#endif

#यदि !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  शून्य sqlite3VdbeLeave(Vdbe*);
#अन्यथा
# define sqlite3VdbeLeave(X)
#endif

#ifdef SQLITE_DEBUG
शून्य sqlite3VdbeMemAboutToChange(Vdbe*,Mem*);
पूर्णाङ्क sqlite3VdbeCheckMemInvariants(Mem*);
#endif

#ifndef SQLITE_OMIT_FOREIGN_कुंजी
पूर्णाङ्क sqlite3VdbeCheckFkImmediate(Vdbe*);
पूर्णाङ्क sqlite3VdbeCheckFkDeferred(Vdbe*);
#अन्यथा
# define sqlite3VdbeCheckFkImmediate(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#endif

#ifdef SQLITE_DEBUG
  शून्य sqlite3VdbePrintSql(Vdbe*);
  शून्य sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#endif
#ifndef SQLITE_OMIT_UTF16
  पूर्णाङ्क sqlite3VdbeMemTranslate(Mem*, u8);
  पूर्णाङ्क sqlite3VdbeMemHandleBom(Mem *pMem);
#endif

#ifndef SQLITE_OMIT_INCRBLOB
  पूर्णाङ्क sqlite3VdbeMemExpandBlob(Mem *);
  #define ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#अन्यथा
  #define sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #define ExpandBlob(P) SQLITE_OK
#endif

#endif /* !defined(SQLITE_VDBEINT_H) */
