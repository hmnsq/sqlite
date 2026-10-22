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
#که_نه_تعریف_شوی SQLITE_VDBEINT_H
#تعریف_کړه SQLITE_VDBEINT_H

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#که_نه_تعریف_شوی SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 50
#د_که_پای

/*
** VDBE_DISPLAY_P4 is true or false depending on whether or not the
** "explain" P4 display logic is enabled.
*/
#إذا !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#وإلا
# define VDBE_DISPLAY_P4 0
#د_که_پای

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
تعريفنوع بنية VdbeOp Op;

/*
** Boolean values
*/
تعريفنوع بینښه Bool;

/* Opaque type used by code in vdbesort.c */
تعريفنوع بنية VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
تعريفنوع بنية Aux_معلومات Aux_معلومات;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
تعريفنوع بنية VdbeTxtBlbCache VdbeTxtBlbCache;

/* Types of VDBE cursors */
#تعریف_کړه CURTYPE_BTREE       0
#تعریف_کړه CURTYPE_SORTER      1
#تعریف_کړه CURTYPE_VTAB        2
#تعریف_کړه CURTYPE_PSEUDO      3

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
تعريفنوع بنية VdbeCursor VdbeCursor;
بنية VdbeCursor {
  u8 eCurType;            /* One of the CURTYPE_* values above */
  i8 iDb;                 /* Index of cursor database in db->aDb[] */
  u8 null_لیکه;             /* True if pointing to a row with no data */
  u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
  u8 is_جدول;             /* True for rowid tables.  False for indexes */
#که_تعریف_شوی SQLITE_DEBUG
  u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#د_که_پای
  Bool isEphemeral:1;     /* True for an ephemeral table */
  Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
  Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
  Bool noReuse:1;         /* OpenEphemeral may not reuse this cursor */
  Bool ستون_Cache:1;        /* pCache pointer is initialized and non-NULL */
  u16 seek_موندنه;            /* See the OP_SeekHit and OP_IfNoHope opcodes */
  اتحاد {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seq_شمېر;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cache_حالت;        /* Cache is valid if this matches Vdbe.cacheCtr */
  صحيح seek_پایله;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match".
  ** For CURTYPE_PSEUDO, seekResult is the register holding the record */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  اتحاد {
    BtCursor *pCursor;          /* CURTYPE_BTREE or _PSEUDO.  Btree cursor */
    sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.              Vtab cursor */
    VdbeSorter *pSorter;        /* CURTYPE_SORTER.            Sorter object */
  } uc;
  کلی_Info *p_کلی_Info;      /* Info about index keys needed by index cursors */
  u32 i_Hdr_دننه_واټن;         /* Offset to next unparsed byte of the header */
  Pgno pgno_ریښه;          /* Root page of the open btree cursor */
  i16 n_ډګر;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 moveto_موخه;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *a_دننه_واټن;           /* Pointer to aType[nField] */
  ثابت u8 *a_لیکه;         /* Data for the current row, if all on one page */
  u32 payload_اندازه;        /* Total number of bytes in the record */
  u32 sz_لیکه;              /* Byte available in aRow */
#که_تعریف_شوی SQLITE_ENABLE_ستنه_کارول_شوی_ماسک
  u64 ماسک_کارول_شوی;           /* Mask of columns used by this cursor */
#د_که_پای
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
#تعریف_کړه SZ_VDBECURSOR(N) \
    (ROUND8(offsetof(VdbeCursor,aType)) + ((N)+1)*حجم(u64))

/* Return true if P is a null-only cursor
*/
#تعریف_کړه IsNullCursor(P) \
  ((P)->eCurType==CURTYPE_PSEUDO && (P)->null_لیکه && (P)->seek_پایله==0)

/*
** A value for VdbeCursor.cacheStatus that means the cache is always invalid.
*/
#تعریف_کړه CACHE_STALE 0

/*
** Large TEXT or BLOB values can be slow to load, so we want to avoid
** loading them more than once.  For that reason, large TEXT and BLOB values
** can be stored in a cache defined by this object, and attached to the
** VdbeCursor using the pCache field.
*/
بنية VdbeTxtBlbCache {
  حرف *pCValue;        /* A RCStr buffer to hold the value */
  i64 i_دننه_واټن;          /* File offset of the row being cached */
  صحيح i_ستون;             /* Column for which the cache is valid */
  u32 cache_حالت;      /* Vdbe.cacheCtr value */
  u32 ستون_Cache_Ctr;      /* Column cache counter */
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
تعريفنوع بنية Vdbe_چوکاټ Vdbe_چوکاټ;
بنية Vdbe_چوکاټ {
  Vdbe *v;                /* VM this frame belongs to */
  Vdbe_چوکاټ *p_پلار;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  فراغ *token;            /* Copy of SubProgram.token */
  i64 وروستی_Rowid;          /* Last insert rowid (sqlite3.lastRowid) */
  Aux_معلومات *p_Aux_معلومات;      /* Linked list of auxdata allocations */
#إذا SQLITE_DEBUG
  u32 i_چوکاټ_Magic;        /* magic number for sanity checking */
#د_که_پای
  صحيح nCursor;            /* Number of entries in apCsr */
  صحيح pc;                 /* Program Counter in parent (calling) frame */
  صحيح nOp;                /* Size of aOp array */
  صحيح nMem;               /* Number of entries in aMem */
  صحيح n_ماشوم_Mem;          /* Number of memory cells for child frame */
  صحيح n_ماشوم_Csr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#تعریف_کړه SQLITE_چوکاټ_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#تعریف_کړه Vdbe_چوکاټ_Mem(p) ((Mem *)&((u8 *)p)[ROUND8(حجم(Vdbe_چوکاټ))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
بنية sqlite3_ارزښت {
  اتحاد Mem_ارزښت {
    مزدوج r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    صحيح nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    ثابت حرف *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  حرف *z;            /* String or BLOB value */
  صحيح n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  صحيح szMalloc;       /* Size of the zMalloc allocation */
  u32 u_لنډمهاله;          /* Transient storage for serial_type in OP_MakeRecord */
  حرف *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  فراغ (*xDel)(فراغ*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#که_تعریف_شوی SQLITE_DEBUG
  Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  u16 mScopyFlags;    /* flags value immediately after the shallow copy */
  u8  bScopy;         /* The pScopyFrom of some other Mem *might* point here */
#د_که_پای
};

/*
** Size of struct Mem not including the Mem.zMalloc member or anything that
** follows.
*/
#تعریف_کړه MEMCELLSIZE offsetof(Mem,db)

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
#تعریف_کړه MEM_Undefined 0x0000   /* Value is undefined */
#تعریف_کړه MEM_Null      0x0001   /* Value is NULL (or a pointer) */
#تعریف_کړه MEM_Str       0x0002   /* Value is a string */
#تعریف_کړه MEM_Int       0x0004   /* Value is an integer */
#تعریف_کړه MEM_Real      0x0008   /* Value is a real number */
#تعریف_کړه MEM_Blob      0x0010   /* Value is a BLOB */
#تعریف_کړه MEM_IntReal   0x0020   /* MEM_Int that stringifies like MEM_Real */
#تعریف_کړه MEM_Aff_ماسک   0x003f   /* Mask of affinity bits */

/* Extra bits that modify the meanings of the core datatypes above
*/
#تعریف_کړه MEM_FromBind  0x0040   /* Value originates from sqlite3_bind() */
 /*                   0x0080   // Available */
#تعریف_کړه MEM_Cleared   0x0100   /* NULL set by OP_Null, not from data */
#تعریف_کړه MEM_غړی      0x0200   /* String in Mem.z is zero terminated */
#تعریف_کړه MEM_Zero      0x0400   /* Mem.i contains count of 0s appended to blob */
#تعریف_کړه MEM_Subtype   0x0800   /* Mem.eSubtype is valid */
#تعریف_کړه MEM_Type_ماسک  0x0dbf   /* Mask of type bits */

/* Bits that determine the storage for Mem.z for a string or blob or
** aggregate accumulator.
*/
#تعریف_کړه MEM_Dyn       0x1000   /* Need to call Mem.xDel() on Mem.z */
#تعریف_کړه MEM_Static    0x2000   /* Mem.z points to a static string */
#تعریف_کړه MEM_Ephem     0x4000   /* Mem.z points to an ephemeral string */
#تعریف_کړه MEM_Agg       0x8000   /* Mem.z points to an agg function context */

/* Return TRUE if Mem X contains dynamically allocated content - anything
** that needs to be deallocated to avoid a leak.
*/
#تعریف_کړه VdbeMemDynamic(X)  \
  (((X)->flags&(MEM_Agg|MEM_Dyn))!=0)

/*
** Clear any existing type flags from a Mem and replace them with f
*/
#تعریف_کړه Mem_سېټ_Type_Flag(p, f) \
   ((p)->flags = ((p)->flags&~(MEM_Type_ماسک|MEM_Zero))|f)

/*
** True if Mem X is a NULL-nochng type.
*/
#تعریف_کړه MemNullNochng(X) \
  (((X)->flags&MEM_Type_ماسک)==(MEM_Null|MEM_Zero) \
    && (X)->n==0 && (X)->u.nZero==0)

/*
** Return true if a memory cell has been initialized and is valid.
** is for use inside assert() statements only.
**
** A Memory cell is initialized if at least one of the
** MEM_Null, MEM_Str, MEM_Int, MEM_Real, MEM_Blob, or MEM_IntReal bits
** is set.  It is "undefined" if all those bits are zero.
*/
#که_تعریف_شوی SQLITE_DEBUG
#تعریف_کړه memIsValid(M)  ((M)->flags & MEM_Aff_ماسک)!=0
#د_که_پای

/*
** Each auxiliary data pointer stored by a user defined function
** implementation calling sqlite3_set_auxdata() is stored in an instance
** of this structure. All such structures associated with a single VM
** are stored in a linked list headed at Vdbe.pAuxData. All are destroyed
** when the VM is halted (if not before).
*/
بنية Aux_معلومات {
  صحيح iAuxOp;                     /* Instruction number of OP_Function opcode */
  صحيح iAuxArg;                    /* Index of function argument. */
  فراغ *pAux;                     /* Aux data pointer */
  فراغ (*xDeleteAux)(فراغ*);      /* Destructor for the aux data */
  Aux_معلومات *p_بل_Aux;              /* Next element in list */
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
بنية sqlite3_context {
  Mem *p_بهر;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  صحيح iOp;                /* Instruction number of OP_Function */
  صحيح isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_ارزښت *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#تعریف_کړه SZ_CONTEXT(N)  \
   (offsetof(sqlite3_context,argv)+(N)*حجم(sqlite3_ارزښت*))


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
تعريفنوع بنية لټون_حالت لټون_حالت;
بنية لټون_حالت {
  صحيح addrExplain;                /* OP_Explain for loop */
  صحيح aAddrRange[6];
  صحيح addrLoop;                   /* Address of "loops" counter */
  صحيح addr_لیدنه;                  /* Address of "rows visited" counter */
  صحيح iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  حرف *zName;                    /* Name of table or index */
};

/* The DblquoteStr object holds the text of a double-quoted
** string for a prepared statement.  A linked list of these objects
** is constructed during statement parsing and is held on Vdbe.pDblStr.
** When computing a normalized SQL statement for an SQL statement, that
** list is consulted for each double-quoted identifier to see if the
** identifier should really be a string literal.
*/
تعريفنوع بنية DblquoteStr DblquoteStr;
بنية DblquoteStr {
  DblquoteStr *p_بل_Str;   /* Next string literal in the list */
  حرف z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
بنية Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  صحيح nMem;               /* Number of memory locations currently allocated */
  صحيح nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  صحيح pc;                 /* The program counter */
  صحيح rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  صحيح iStatement;         /* Statement number (or 0 if has no opened stmt) */
  i64 i_Current_وخت;       /* Value of julianday('now') for this statement */
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
  صحيح nOp;                /* Number of instructions in the program */
  صحيح nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *a_ستون_Name;          /* Column names to return */
  Mem *p_پایله_لیکه;        /* Current output row */
  حرف *z_Err_پیغام;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#که_نه_تعریف_شوی SQLITE_OMIT_TRACE
  i64 پیل_وخت;          /* Time when query started - used for profiling */
#د_که_پای
#که_تعریف_شوی SQLITE_DEBUG
  صحيح rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  صحيح napArg;             /* Size of the apArg[] array */
#د_که_پای
  u16 n_Res_ستنه;         /* Number of columns in one row of the result set */
  u16 nResAlloc;          /* Column slots allocated to aColName[] */
  u8 errorAction;         /* Recovery action to do in case of an error */
  u8 minWriteFileFormat;  /* Minimum file format for writable database files */
  u8 prepFlags;           /* SQLITE_PREPARE_* flags */
  u8 eVdbeState;          /* On of the VDBE_*_STATE values */
  bft expired:2;          /* 1: recompile VM immediately  2: when convenient */
  bft explain:2;          /* 0: normal, 1: EXPLAIN, 2: EXPLAIN QUERY PLAN */
  bft changeCntOn:1;      /* True to update the change-counter */
  bft کاروي_Stmt_Journal:1;  /* True if uses a statement journal */
  bft readOnly:1;         /* True for statements that do not write */
  bft bIsReader:1;        /* True for statements that read */
  bft haveEqpOps:1;       /* Bytecode supports EXPLAIN QUERY PLAN */
  y_Db_ماسک btree_ماسک;      /* Bitmask of db->aDb[] entries referenced */
  y_Db_ماسک قلف_ماسک;       /* Subset of btreeMask that requires a lock */
  u32 aCounter[9];        /* Counters used by sqlite3_stmt_status() */
  حرف *zSql;             /* Text of the SQL statement that generated this */
#که_تعریف_شوی SQLITE_ENABLE_NORMALIZE
  حرف *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#د_که_پای
  فراغ *pFree;            /* Free this when deleting the vdbe */
  Vdbe_چوکاټ *p_چوکاټ;      /* Parent frame */
  Vdbe_چوکاټ *p_Del_چوکاټ;   /* List of frame objects to free on VM reset */
  صحيح n_چوکاټ;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  Sub_پروګرام *p_پروګرام;   /* Linked list of all sub-programs used by VM */
  Aux_معلومات *p_Aux_معلومات;      /* Linked list of auxdata allocations */
#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح n_لټون;              /* Entries in aScan[] */
  لټون_حالت *a_لټون;      /* Scan definitions for sqlite3_stmt_scanstatus() */
#د_که_پای
};

/*
** The following are allowed values for Vdbe.eVdbeState
*/
#تعریف_کړه VDBE_INIT_STATE     0   /* Prepared statement under construction */
#تعریف_کړه VDBE_READY_STATE    1   /* Ready to run but not yet started */
#تعریف_کړه VDBE_پرلپسې_STATE      2   /* Run in progress */
#تعریف_کړه VDBE_HALT_STATE     3   /* Finished.  Need reset() or finalize() */

/*
** Structure used to store the context required by the
** sqlite3_preupdate_*() API functions.
*/
بنية PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  صحيح op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *a_ریکارډ;                    /* old.* database record */
  کلی_Info *pKeyinfo;              /* Key information */
  Unpacked_ریکارډ *pUnpacked;      /* Unpacked version of aRecord[] */
  Unpacked_ریکارډ *pNewUnpacked;   /* Unpacked version of new.* record */
  صحيح iNewReg;                    /* Register for new.* values */
  صحيح iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  جدول *pTab;                    /* Schema object being updated */
  شاخص *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_ارزښت **apDflt;         /* Array of default values, if required */
  بنية {
    u8 keyinfoSpace[SZ_KEYINFO_0];  /* Space to hold pKeyinfo[0] content */
  } u_کلی;
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
تعريفنوع بنية ارزښت_List ارزښت_List;
بنية ارزښت_List {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_ارزښت *p_بهر;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#که_نه_تعریف_شوی SQLITE_AMALGAMATION
خارجي ثابت u8 sqlite3SmallTypeSizes[];
#د_که_پای

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#تعریف_کړه ONE_BYTE_INT(x)    ((i8)(x)[0])
#تعریف_کړه TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#تعریف_کړه THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#تعریف_کړه FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#تعریف_کړه FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#تعریف_کړه FOUR_BYTE_INT(x)   ((صحيح)FOUR_BYTE_UINT(x))
#تعریف_کړه SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
فراغ sqlite3VdbeError(Vdbe*, ثابت حرف *, ...);
فراغ sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
فراغ sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
فراغ sqlite_Vdbe_راوباسه_ډېری(Vdbe*,صحيح);
صحيح SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
صحيح SQLITE_NOINLINE sqlite3_Vdbe_پای_Moveto(VdbeCursor*);
صحيح sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#که_تعریف_شوی SQLITE_MIXED_ENDIAN_64_بټ_FLOAT
  u64 sqlite3_Float_بدلون(u64 دننه);
# define بدلون_Mixed_Endian_Float(X)  X = sqlite3_Float_بدلون(X)
#وإلا
# define بدلون_Mixed_Endian_Float(X)
#د_که_پای
فراغ sqlite3VdbeSerialGet(ثابت بینښه حرف*, u32, Mem*);
فراغ sqlite3_Vdbe_Delete_Aux_معلومات(sqlite3*, Aux_معلومات**, صحيح, صحيح);

صحيح sqlite2_Btree_کلی_Compare(BtCursor *, ثابت فراغ *, صحيح, صحيح, صحيح *);
صحيح sqlite3_Vdbe_Idx_کلی_Compare(sqlite3*,VdbeCursor*,Unpacked_ریکارډ*,صحيح*);
صحيح sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
صحيح sqlite3VdbeExec(Vdbe*);
#إذا !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
صحيح sqlite3_Vdbe_بل_Opcode(Vdbe*,Mem*,صحيح,صحيح*,صحيح*,Op**);
حرف *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#د_که_پای
#إذا defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
حرف *sqlite3VdbeDisplayComment(sqlite3*,ثابت Op*,ثابت حرف*);
#د_که_پای
#إذا !defined(SQLITE_OMIT_EXPLAIN)
صحيح sqlite3VdbeList(Vdbe*);
#د_که_پای
صحيح sqlite3VdbeHalt(Vdbe*);
صحيح sqlite3VdbeChangeEncoding(Mem *, صحيح);
صحيح sqlite3VdbeMemTooBig(Mem*);
صحيح sqlite3VdbeMemCopy(Mem*, ثابت Mem*);
فراغ sqlite3VdbeMemShallowCopy(Mem*, ثابت Mem*, صحيح);
فراغ sqlite3VdbeMemMove(Mem*, Mem*);
صحيح sqlite3VdbeMemNulTerminate(Mem*);
صحيح sqlite3_Vdbe_Mem_سېټ_Str(Mem*, ثابت حرف*, i64, u8, فراغ(*)(فراغ*));
صحيح sqlite3_Vdbe_Mem_سېټ_متن(Mem*, ثابت حرف*, i64, فراغ(*)(فراغ*));
فراغ sqlite3_Vdbe_Mem_سېټ_Int64(Mem*, i64);
#که_تعریف_شوی SQLITE_OMIT_FLOATING_ټکی
# define sqlite3_Vdbe_Mem_سېټ_Double sqlite3_Vdbe_Mem_سېټ_Int64
#وإلا
  فراغ sqlite3_Vdbe_Mem_سېټ_Double(Mem*, مزدوج);
#د_که_پای
فراغ sqlite3_Vdbe_Mem_سېټ_اشاره_ګر(Mem*, فراغ*, ثابت حرف*, فراغ(*)(فراغ*));
فراغ sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
فراغ sqlite3_Vdbe_Mem_سېټ_Null(Mem*);
#که_نه_تعریف_شوی SQLITE_OMIT_INCRBLOB
فراغ sqlite3_Vdbe_Mem_سېټ_Zero_Blob(Mem*,صحيح);
#وإلا
صحيح sqlite3_Vdbe_Mem_سېټ_Zero_Blob(Mem*,صحيح);
#د_که_پای
#که_تعریف_شوی SQLITE_DEBUG
صحيح sqlite3_Vdbe_Mem_Is_لیکه_سېټ(ثابت Mem*);
#د_که_پای
صحيح sqlite3_Vdbe_Mem_سېټ_لیکه_سېټ(Mem*);
صحيح sqlite3VdbeMemZeroTerminateIfAble(Mem*);
صحيح sqlite3VdbeMemMakeWriteable(Mem*);
صحيح sqlite3VdbeMemStringify(Mem*, u8, u8);
صحيح sqlite3IntFloatCompare(i64,مزدوج);
i64 sqlite3_Vdbe_Int_ارزښت(ثابت Mem*);
صحيح sqlite3VdbeMemIntegerify(Mem*);
مزدوج sqlite3_Vdbe_Real_ارزښت(Mem*);
صحيح sqlite3_Mem_Real_ارزښت_RC(Mem*, مزدوج*);
صحيح sqlite3_Vdbe_Boolean_ارزښت(Mem*, صحيح ifNull);
فراغ sqlite3VdbeIntegerAffinity(Mem*,صحيح b_From_متن);
صحيح sqlite3VdbeMemRealify(Mem*);
صحيح sqlite3VdbeMemNumerify(Mem*);
صحيح sqlite3VdbeMemCast(Mem*,u8,u8);
صحيح sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
صحيح sqlite3_Vdbe_Mem_From_Btree_Zero_دننه_واټن(BtCursor*,u32,Mem*);
فراغ sqlite3VdbeMemRelease(Mem *p);
فراغ sqlite3VdbeMemReleaseMalloc(Mem*p);
صحيح sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
صحيح sqlite3_Vdbe_Mem_Agg_ارزښت(Mem*, Mem*, FuncDef*);
#د_که_پای
#إذا !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
ثابت حرف *sqlite3OpcodeName(صحيح);
#د_که_پای
صحيح sqlite3VdbeMemGrow(Mem *pMem, صحيح n, صحيح preserve);
صحيح sqlite3VdbeMemClearAndResize(Mem *pMem, صحيح n);
صحيح sqlite3VdbeCloseStatement(Vdbe *, صحيح);
#که_تعریف_شوی SQLITE_DEBUG
صحيح sqlite3_Vdbe_چوکاټ_Is_Valid(Vdbe_چوکاټ*);
#د_که_پای
فراغ sqlite3_Vdbe_چوکاټ_Mem_Del(فراغ*);      /* Destructor on Mem */
فراغ sqlite3_Vdbe_چوکاټ_Delete(Vdbe_چوکاټ*); /* Actually deletes the Frame */
صحيح sqlite3_Vdbe_چوکاټ_Restore(Vdbe_چوکاټ *);
#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
فراغ sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,صحيح,ثابت حرف*,جدول*,i64,صحيح,صحيح);
#د_که_پای
صحيح sqlite3VdbeTransferError(Vdbe *p);
صحيح sqlite3_Vdbe_پیدا_کړه_شاخص_کلی(BtCursor*, شاخص*, Unpacked_ریکارډ*, صحيح*, صحيح);

صحيح sqlite3VdbeSorterInit(sqlite3 *, صحيح, VdbeCursor *);
فراغ sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
فراغ sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
صحيح sqlite3VdbeSorterRowkey(ثابت VdbeCursor *, Mem *);
صحيح sqlite3_Vdbe_Sorter_بل(sqlite3 *, ثابت VdbeCursor *);
صحيح sqlite3VdbeSorterRewind(ثابت VdbeCursor *, صحيح *);
صحيح sqlite3VdbeSorterWrite(ثابت VdbeCursor *, Mem *);
صحيح sqlite3VdbeSorterCompare(ثابت VdbeCursor *, Mem *, صحيح, صحيح *);

فراغ sqlite3_Vdbe_ارزښت_List_Free(فراغ*);

#که_تعریف_شوی SQLITE_DEBUG
  فراغ sqlite3VdbeIncrWriteCounter(Vdbe*, VdbeCursor*);
  فراغ sqlite3VdbeAssertAbortable(Vdbe*);
#وإلا
# define sqlite3VdbeIncrWriteCounter(V,C)
# define sqlite3VdbeAssertAbortable(V)
#د_که_پای

#إذا !defined(SQLITE_OMIT_SHARED_CACHE)
  فراغ sqlite3VdbeEnter(Vdbe*);
#وإلا
# define sqlite3VdbeEnter(X)
#د_که_پای

#إذا !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  فراغ sqlite3VdbeLeave(Vdbe*);
#وإلا
# define sqlite3VdbeLeave(X)
#د_که_پای

#که_تعریف_شوی SQLITE_DEBUG
فراغ sqlite3_Vdbe_Mem_About_منزل_Change(Vdbe*,Mem*);
صحيح sqlite3VdbeCheckMemInvariants(Mem*);
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_FOREIGN_کلی
صحيح sqlite3_Vdbe_Check_Fk_سمدستي(Vdbe*);
صحيح sqlite3VdbeCheckFkDeferred(Vdbe*);
#وإلا
# define sqlite3_Vdbe_Check_Fk_سمدستي(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#د_که_پای

#که_تعریف_شوی SQLITE_DEBUG
  فراغ sqlite3VdbePrintSql(Vdbe*);
  فراغ sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
  صحيح sqlite3VdbeMemTranslate(Mem*, u8);
  صحيح sqlite3VdbeMemHandleBom(Mem *pMem);
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_INCRBLOB
  صحيح sqlite3VdbeMemExpandBlob(Mem *);
  #تعریف_کړه ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#وإلا
  #تعریف_کړه sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #تعریف_کړه ExpandBlob(P) SQLITE_OK
#د_که_پای

#د_که_پای /* !defined(SQLITE_VDBEINT_H) */
