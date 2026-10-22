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
#اگر_غیر_تعریف_شدہ SQLITE_VDBEINT_H
#تعریف_کرو SQLITE_VDBEINT_H

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#اگر_غیر_تعریف_شدہ SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 50
#اگر_ختم

/*
** VDBE_DISPLAY_P4 is true or false depending on whether or not the
** "explain" P4 display logic is enabled.
*/
#اگر !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#ورنہ
# define VDBE_DISPLAY_P4 0
#اگر_ختم

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
قسمتعریف ساخت VdbeOp Op;

/*
** Boolean values
*/
قسمتعریف غیرعلامتی Bool;

/* Opaque type used by code in vdbesort.c */
قسمتعریف ساخت VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
قسمتعریف ساخت Aux_ڈیٹا Aux_ڈیٹا;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
قسمتعریف ساخت VdbeTxtBlbCache VdbeTxtBlbCache;

/* Types of VDBE cursors */
#تعریف_کرو CURTYPE_BTREE       0
#تعریف_کرو CURTYPE_SORTER      1
#تعریف_کرو CURTYPE_VTAB        2
#تعریف_کرو CURTYPE_PSEUDO      3

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
قسمتعریف ساخت VdbeCursor VdbeCursor;
ساخت VdbeCursor {
  u8 eCurType;            /* One of the CURTYPE_* values above */
  i8 iDb;                 /* Index of cursor database in db->aDb[] */
  u8 null_صف;             /* True if pointing to a row with no data */
  u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
  u8 is_جدول;             /* True for rowid tables.  False for indexes */
#اگر_تعریف_شدہ SQLITE_DEBUG
  u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#اگر_ختم
  Bool isEphemeral:1;     /* True for an ephemeral table */
  Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
  Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
  Bool noReuse:1;         /* OpenEphemeral may not reuse this cursor */
  Bool ستون_Cache:1;        /* pCache pointer is initialized and non-NULL */
  u16 seek_کامیابی;            /* See the OP_SeekHit and OP_IfNoHope opcodes */
  اتحاد {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seq_گنتی;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cache_حالت;        /* Cache is valid if this matches Vdbe.cacheCtr */
  صحیح seek_نتیجہ;         /* Result of previous sqlite3BtreeMoveto() or 0
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
  کلید_Info *p_کلید_Info;      /* Info about index keys needed by index cursors */
  u32 i_Hdr_انحراف;         /* Offset to next unparsed byte of the header */
  Pgno pgno_جڑ;          /* Root page of the open btree cursor */
  i16 n_میدان;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 moveto_ہدف;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *a_انحراف;           /* Pointer to aType[nField] */
  ثابت u8 *a_صف;         /* Data for the current row, if all on one page */
  u32 payload_سائز;        /* Total number of bytes in the record */
  u32 sz_صف;              /* Byte available in aRow */
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_مستعمل_نقاب
  u64 نقاب_مستعمل;           /* Mask of columns used by this cursor */
#اگر_ختم
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
#تعریف_کرو SZ_VDBECURSOR(N) \
    (ROUND8(offsetof(VdbeCursor,aType)) + ((N)+1)*حجم(u64))

/* Return true if P is a null-only cursor
*/
#تعریف_کرو IsNullCursor(P) \
  ((P)->eCurType==CURTYPE_PSEUDO && (P)->null_صف && (P)->seek_نتیجہ==0)

/*
** A value for VdbeCursor.cacheStatus that means the cache is always invalid.
*/
#تعریف_کرو CACHE_STALE 0

/*
** Large TEXT or BLOB values can be slow to load, so we want to avoid
** loading them more than once.  For that reason, large TEXT and BLOB values
** can be stored in a cache defined by this object, and attached to the
** VdbeCursor using the pCache field.
*/
ساخت VdbeTxtBlbCache {
  حرف *pCValue;        /* A RCStr buffer to hold the value */
  i64 i_انحراف;          /* File offset of the row being cached */
  صحیح i_ستون;             /* Column for which the cache is valid */
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
قسمتعریف ساخت Vdbe_چوکھٹا Vdbe_چوکھٹا;
ساخت Vdbe_چوکھٹا {
  Vdbe *v;                /* VM this frame belongs to */
  Vdbe_چوکھٹا *p_والد;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  خالی *token;            /* Copy of SubProgram.token */
  i64 آخری_Rowid;          /* Last insert rowid (sqlite3.lastRowid) */
  Aux_ڈیٹا *p_Aux_ڈیٹا;      /* Linked list of auxdata allocations */
#اگر SQLITE_DEBUG
  u32 i_چوکھٹا_Magic;        /* magic number for sanity checking */
#اگر_ختم
  صحیح nCursor;            /* Number of entries in apCsr */
  صحیح pc;                 /* Program Counter in parent (calling) frame */
  صحیح nOp;                /* Size of aOp array */
  صحیح nMem;               /* Number of entries in aMem */
  صحیح n_بچہ_Mem;          /* Number of memory cells for child frame */
  صحیح n_بچہ_Csr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#تعریف_کرو SQLITE_چوکھٹا_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#تعریف_کرو Vdbe_چوکھٹا_Mem(p) ((Mem *)&((u8 *)p)[ROUND8(حجم(Vdbe_چوکھٹا))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
ساخت sqlite3_قدر {
  اتحاد Mem_قدر {
    دوہرا r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    صحیح nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    ثابت حرف *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  حرف *z;            /* String or BLOB value */
  صحیح n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  صحیح szMalloc;       /* Size of the zMalloc allocation */
  u32 u_عارضی;          /* Transient storage for serial_type in OP_MakeRecord */
  حرف *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  خالی (*xDel)(خالی*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#اگر_تعریف_شدہ SQLITE_DEBUG
  Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  u16 mScopyFlags;    /* flags value immediately after the shallow copy */
  u8  bScopy;         /* The pScopyFrom of some other Mem *might* point here */
#اگر_ختم
};

/*
** Size of struct Mem not including the Mem.zMalloc member or anything that
** follows.
*/
#تعریف_کرو MEMCELLSIZE offsetof(Mem,db)

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
#تعریف_کرو MEM_Undefined 0x0000   /* Value is undefined */
#تعریف_کرو MEM_Null      0x0001   /* Value is NULL (or a pointer) */
#تعریف_کرو MEM_Str       0x0002   /* Value is a string */
#تعریف_کرو MEM_Int       0x0004   /* Value is an integer */
#تعریف_کرو MEM_Real      0x0008   /* Value is a real number */
#تعریف_کرو MEM_Blob      0x0010   /* Value is a BLOB */
#تعریف_کرو MEM_IntReal   0x0020   /* MEM_Int that stringifies like MEM_Real */
#تعریف_کرو MEM_Aff_نقاب   0x003f   /* Mask of affinity bits */

/* Extra bits that modify the meanings of the core datatypes above
*/
#تعریف_کرو MEM_FromBind  0x0040   /* Value originates from sqlite3_bind() */
 /*                   0x0080   // Available */
#تعریف_کرو MEM_Cleared   0x0100   /* NULL set by OP_Null, not from data */
#تعریف_کرو MEM_رکن      0x0200   /* String in Mem.z is zero terminated */
#تعریف_کرو MEM_Zero      0x0400   /* Mem.i contains count of 0s appended to blob */
#تعریف_کرو MEM_Subtype   0x0800   /* Mem.eSubtype is valid */
#تعریف_کرو MEM_Type_نقاب  0x0dbf   /* Mask of type bits */

/* Bits that determine the storage for Mem.z for a string or blob or
** aggregate accumulator.
*/
#تعریف_کرو MEM_Dyn       0x1000   /* Need to call Mem.xDel() on Mem.z */
#تعریف_کرو MEM_Static    0x2000   /* Mem.z points to a static string */
#تعریف_کرو MEM_Ephem     0x4000   /* Mem.z points to an ephemeral string */
#تعریف_کرو MEM_Agg       0x8000   /* Mem.z points to an agg function context */

/* Return TRUE if Mem X contains dynamically allocated content - anything
** that needs to be deallocated to avoid a leak.
*/
#تعریف_کرو VdbeMemDynamic(X)  \
  (((X)->flags&(MEM_Agg|MEM_Dyn))!=0)

/*
** Clear any existing type flags from a Mem and replace them with f
*/
#تعریف_کرو Mem_سیٹ_Type_Flag(p, f) \
   ((p)->flags = ((p)->flags&~(MEM_Type_نقاب|MEM_Zero))|f)

/*
** True if Mem X is a NULL-nochng type.
*/
#تعریف_کرو MemNullNochng(X) \
  (((X)->flags&MEM_Type_نقاب)==(MEM_Null|MEM_Zero) \
    && (X)->n==0 && (X)->u.nZero==0)

/*
** Return true if a memory cell has been initialized and is valid.
** is for use inside assert() statements only.
**
** A Memory cell is initialized if at least one of the
** MEM_Null, MEM_Str, MEM_Int, MEM_Real, MEM_Blob, or MEM_IntReal bits
** is set.  It is "undefined" if all those bits are zero.
*/
#اگر_تعریف_شدہ SQLITE_DEBUG
#تعریف_کرو memIsValid(M)  ((M)->flags & MEM_Aff_نقاب)!=0
#اگر_ختم

/*
** Each auxiliary data pointer stored by a user defined function
** implementation calling sqlite3_set_auxdata() is stored in an instance
** of this structure. All such structures associated with a single VM
** are stored in a linked list headed at Vdbe.pAuxData. All are destroyed
** when the VM is halted (if not before).
*/
ساخت Aux_ڈیٹا {
  صحیح iAuxOp;                     /* Instruction number of OP_Function opcode */
  صحیح iAuxArg;                    /* Index of function argument. */
  خالی *pAux;                     /* Aux data pointer */
  خالی (*xDeleteAux)(خالی*);      /* Destructor for the aux data */
  Aux_ڈیٹا *p_اگلا_Aux;              /* Next element in list */
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
ساخت sqlite3_context {
  Mem *p_برآمد;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  صحیح iOp;                /* Instruction number of OP_Function */
  صحیح isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_قدر *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#تعریف_کرو SZ_CONTEXT(N)  \
   (offsetof(sqlite3_context,argv)+(N)*حجم(sqlite3_قدر*))


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
قسمتعریف ساخت جائزہ_حالت جائزہ_حالت;
ساخت جائزہ_حالت {
  صحیح addrExplain;                /* OP_Explain for loop */
  صحیح aAddrRange[6];
  صحیح addrLoop;                   /* Address of "loops" counter */
  صحیح addr_دورہ;                  /* Address of "rows visited" counter */
  صحیح iSelectID;                  /* The "Select-ID" for this loop */
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
قسمتعریف ساخت DblquoteStr DblquoteStr;
ساخت DblquoteStr {
  DblquoteStr *p_اگلا_Str;   /* Next string literal in the list */
  حرف z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
ساخت Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  صحیح nMem;               /* Number of memory locations currently allocated */
  صحیح nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  صحیح pc;                 /* The program counter */
  صحیح rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  صحیح iStatement;         /* Statement number (or 0 if has no opened stmt) */
  i64 i_Current_وقت;       /* Value of julianday('now') for this statement */
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
  صحیح nOp;                /* Number of instructions in the program */
  صحیح nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *a_ستون_Name;          /* Column names to return */
  Mem *p_نتیجہ_صف;        /* Current output row */
  حرف *z_Err_پیغام;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE
  i64 آغاز_وقت;          /* Time when query started - used for profiling */
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DEBUG
  صحیح rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  صحیح napArg;             /* Size of the apArg[] array */
#اگر_ختم
  u16 n_Res_عمود;         /* Number of columns in one row of the result set */
  u16 nResAlloc;          /* Column slots allocated to aColName[] */
  u8 errorAction;         /* Recovery action to do in case of an error */
  u8 minWriteFileFormat;  /* Minimum file format for writable database files */
  u8 prepFlags;           /* SQLITE_PREPARE_* flags */
  u8 eVdbeState;          /* On of the VDBE_*_STATE values */
  bft expired:2;          /* 1: recompile VM immediately  2: when convenient */
  bft explain:2;          /* 0: normal, 1: EXPLAIN, 2: EXPLAIN QUERY PLAN */
  bft changeCntOn:1;      /* True to update the change-counter */
  bft استعمال_کرتا_Stmt_Journal:1;  /* True if uses a statement journal */
  bft readOnly:1;         /* True for statements that do not write */
  bft bIsReader:1;        /* True for statements that read */
  bft haveEqpOps:1;       /* Bytecode supports EXPLAIN QUERY PLAN */
  y_Db_نقاب btree_نقاب;      /* Bitmask of db->aDb[] entries referenced */
  y_Db_نقاب تالا_نقاب;       /* Subset of btreeMask that requires a lock */
  u32 a_شمار[9];        /* Counters used by sqlite3_stmt_status() */
  حرف *zSql;             /* Text of the SQL statement that generated this */
#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
  حرف *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#اگر_ختم
  خالی *pFree;            /* Free this when deleting the vdbe */
  Vdbe_چوکھٹا *p_چوکھٹا;      /* Parent frame */
  Vdbe_چوکھٹا *p_Del_چوکھٹا;   /* List of frame objects to free on VM reset */
  صحیح n_چوکھٹا;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  Sub_پروگرام *p_پروگرام;   /* Linked list of all sub-programs used by VM */
  Aux_ڈیٹا *p_Aux_ڈیٹا;      /* Linked list of auxdata allocations */
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  صحیح n_جائزہ;              /* Entries in aScan[] */
  جائزہ_حالت *a_جائزہ;      /* Scan definitions for sqlite3_stmt_scanstatus() */
#اگر_ختم
};

/*
** The following are allowed values for Vdbe.eVdbeState
*/
#تعریف_کرو VDBE_INIT_STATE     0   /* Prepared statement under construction */
#تعریف_کرو VDBE_READY_STATE    1   /* Ready to run but not yet started */
#تعریف_کرو VDBE_تسلسل_STATE      2   /* Run in progress */
#تعریف_کرو VDBE_HALT_STATE     3   /* Finished.  Need reset() or finalize() */

/*
** Structure used to store the context required by the
** sqlite3_preupdate_*() API functions.
*/
ساخت PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  صحیح op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *a_ریکارڈ;                    /* old.* database record */
  کلید_Info *pKeyinfo;              /* Key information */
  Unpacked_ریکارڈ *pUnpacked;      /* Unpacked version of aRecord[] */
  Unpacked_ریکارڈ *pNewUnpacked;   /* Unpacked version of new.* record */
  صحیح iNewReg;                    /* Register for new.* values */
  صحیح iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  جدول *pTab;                    /* Schema object being updated */
  اشاریہ *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_قدر **apDflt;         /* Array of default values, if required */
  ساخت {
    u8 keyinfoSpace[SZ_KEYINFO_0];  /* Space to hold pKeyinfo[0] content */
  } u_کلید;
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
قسمتعریف ساخت قدر_List قدر_List;
ساخت قدر_List {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_قدر *p_برآمد;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#اگر_غیر_تعریف_شدہ SQLITE_AMALGAMATION
خارجی ثابت u8 sqlite3SmallTypeSizes[];
#اگر_ختم

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#تعریف_کرو ONE_BYTE_INT(x)    ((i8)(x)[0])
#تعریف_کرو TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#تعریف_کرو THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#تعریف_کرو FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#تعریف_کرو FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#تعریف_کرو FOUR_BYTE_INT(x)   ((صحیح)FOUR_BYTE_UINT(x))
#تعریف_کرو SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
خالی sqlite3VdbeError(Vdbe*, ثابت حرف *, ...);
خالی sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
خالی sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
خالی sqlite_Vdbe_نکالو_ڈھیر(Vdbe*,صحیح);
صحیح SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
صحیح SQLITE_NOINLINE sqlite3_Vdbe_اختتام_Moveto(VdbeCursor*);
صحیح sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#اگر_تعریف_شدہ SQLITE_MIXED_ENDIAN_64_ثنائی_ہندسہ_FLOAT
  u64 sqlite3_Float_تبادلہ(u64 اندر);
# define تبادلہ_Mixed_Endian_Float(X)  X = sqlite3_Float_تبادلہ(X)
#ورنہ
# define تبادلہ_Mixed_Endian_Float(X)
#اگر_ختم
خالی sqlite3VdbeSerialGet(ثابت غیرعلامتی حرف*, u32, Mem*);
خالی sqlite3_Vdbe_Delete_Aux_ڈیٹا(sqlite3*, Aux_ڈیٹا**, صحیح, صحیح);

صحیح sqlite2_Btree_کلید_Compare(BtCursor *, ثابت خالی *, صحیح, صحیح, صحیح *);
صحیح sqlite3_Vdbe_Idx_کلید_Compare(sqlite3*,VdbeCursor*,Unpacked_ریکارڈ*,صحیح*);
صحیح sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
صحیح sqlite3VdbeExec(Vdbe*);
#اگر !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
صحیح sqlite3_Vdbe_اگلا_Opcode(Vdbe*,Mem*,صحیح,صحیح*,صحیح*,Op**);
حرف *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#اگر_ختم
#اگر defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
حرف *sqlite3VdbeDisplayComment(sqlite3*,ثابت Op*,ثابت حرف*);
#اگر_ختم
#اگر !defined(SQLITE_OMIT_EXPLAIN)
صحیح sqlite3VdbeList(Vdbe*);
#اگر_ختم
صحیح sqlite3VdbeHalt(Vdbe*);
صحیح sqlite3VdbeChangeEncoding(Mem *, صحیح);
صحیح sqlite3VdbeMemTooBig(Mem*);
صحیح sqlite3VdbeMemCopy(Mem*, ثابت Mem*);
خالی sqlite3VdbeMemShallowCopy(Mem*, ثابت Mem*, صحیح);
خالی sqlite3VdbeMemMove(Mem*, Mem*);
صحیح sqlite3VdbeMemNulTerminate(Mem*);
صحیح sqlite3_Vdbe_Mem_سیٹ_Str(Mem*, ثابت حرف*, i64, u8, خالی(*)(خالی*));
صحیح sqlite3_Vdbe_Mem_سیٹ_متن(Mem*, ثابت حرف*, i64, خالی(*)(خالی*));
خالی sqlite3_Vdbe_Mem_سیٹ_Int64(Mem*, i64);
#اگر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
# define sqlite3_Vdbe_Mem_سیٹ_Double sqlite3_Vdbe_Mem_سیٹ_Int64
#ورنہ
  خالی sqlite3_Vdbe_Mem_سیٹ_Double(Mem*, دوہرا);
#اگر_ختم
خالی sqlite3_Vdbe_Mem_سیٹ_اشارہ_گر(Mem*, خالی*, ثابت حرف*, خالی(*)(خالی*));
خالی sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
خالی sqlite3_Vdbe_Mem_سیٹ_Null(Mem*);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
خالی sqlite3_Vdbe_Mem_سیٹ_Zero_Blob(Mem*,صحیح);
#ورنہ
صحیح sqlite3_Vdbe_Mem_سیٹ_Zero_Blob(Mem*,صحیح);
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DEBUG
صحیح sqlite3_Vdbe_Mem_Is_صف_سیٹ(ثابت Mem*);
#اگر_ختم
صحیح sqlite3_Vdbe_Mem_سیٹ_صف_سیٹ(Mem*);
صحیح sqlite3VdbeMemZeroTerminateIfAble(Mem*);
صحیح sqlite3VdbeMemMakeWriteable(Mem*);
صحیح sqlite3VdbeMemStringify(Mem*, u8, u8);
صحیح sqlite3IntFloatCompare(i64,دوہرا);
i64 sqlite3_Vdbe_Int_قدر(ثابت Mem*);
صحیح sqlite3VdbeMemIntegerify(Mem*);
دوہرا sqlite3_Vdbe_Real_قدر(Mem*);
صحیح sqlite3_Mem_Real_قدر_RC(Mem*, دوہرا*);
صحیح sqlite3_Vdbe_Boolean_قدر(Mem*, صحیح ifNull);
خالی sqlite3VdbeIntegerAffinity(Mem*,صحیح b_From_متن);
صحیح sqlite3VdbeMemRealify(Mem*);
صحیح sqlite3VdbeMemNumerify(Mem*);
صحیح sqlite3VdbeMemCast(Mem*,u8,u8);
صحیح sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
صحیح sqlite3_Vdbe_Mem_From_Btree_Zero_انحراف(BtCursor*,u32,Mem*);
خالی sqlite3VdbeMemRelease(Mem *p);
خالی sqlite3VdbeMemReleaseMalloc(Mem*p);
صحیح sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
صحیح sqlite3_Vdbe_Mem_Agg_قدر(Mem*, Mem*, FuncDef*);
#اگر_ختم
#اگر !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
ثابت حرف *sqlite3OpcodeName(صحیح);
#اگر_ختم
صحیح sqlite3VdbeMemGrow(Mem *pMem, صحیح n, صحیح preserve);
صحیح sqlite3VdbeMemClearAndResize(Mem *pMem, صحیح n);
صحیح sqlite3VdbeCloseStatement(Vdbe *, صحیح);
#اگر_تعریف_شدہ SQLITE_DEBUG
صحیح sqlite3_Vdbe_چوکھٹا_Is_Valid(Vdbe_چوکھٹا*);
#اگر_ختم
خالی sqlite3_Vdbe_چوکھٹا_Mem_Del(خالی*);      /* Destructor on Mem */
خالی sqlite3_Vdbe_چوکھٹا_Delete(Vdbe_چوکھٹا*); /* Actually deletes the Frame */
صحیح sqlite3_Vdbe_چوکھٹا_Restore(Vdbe_چوکھٹا *);
#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
خالی sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,صحیح,ثابت حرف*,جدول*,i64,صحیح,صحیح);
#اگر_ختم
صحیح sqlite3VdbeTransferError(Vdbe *p);
صحیح sqlite3_Vdbe_ڈھونڈو_اشاریہ_کلید(BtCursor*, اشاریہ*, Unpacked_ریکارڈ*, صحیح*, صحیح);

صحیح sqlite3VdbeSorterInit(sqlite3 *, صحیح, VdbeCursor *);
خالی sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
خالی sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
صحیح sqlite3VdbeSorterRowkey(ثابت VdbeCursor *, Mem *);
صحیح sqlite3_Vdbe_Sorter_اگلا(sqlite3 *, ثابت VdbeCursor *);
صحیح sqlite3VdbeSorterRewind(ثابت VdbeCursor *, صحیح *);
صحیح sqlite3VdbeSorterWrite(ثابت VdbeCursor *, Mem *);
صحیح sqlite3VdbeSorterCompare(ثابت VdbeCursor *, Mem *, صحیح, صحیح *);

خالی sqlite3_Vdbe_قدر_List_Free(خالی*);

#اگر_تعریف_شدہ SQLITE_DEBUG
  خالی sqlite3_Vdbe_Incr_Write_شمار(Vdbe*, VdbeCursor*);
  خالی sqlite3VdbeAssertAbortable(Vdbe*);
#ورنہ
# define sqlite3_Vdbe_Incr_Write_شمار(V,C)
# define sqlite3VdbeAssertAbortable(V)
#اگر_ختم

#اگر !defined(SQLITE_OMIT_SHARED_CACHE)
  خالی sqlite3VdbeEnter(Vdbe*);
#ورنہ
# define sqlite3VdbeEnter(X)
#اگر_ختم

#اگر !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  خالی sqlite3VdbeLeave(Vdbe*);
#ورنہ
# define sqlite3VdbeLeave(X)
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
خالی sqlite3_Vdbe_Mem_About_مقصد_Change(Vdbe*,Mem*);
صحیح sqlite3VdbeCheckMemInvariants(Mem*);
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FOREIGN_کلید
صحیح sqlite3_Vdbe_Check_Fk_فوری(Vdbe*);
صحیح sqlite3VdbeCheckFkDeferred(Vdbe*);
#ورنہ
# define sqlite3_Vdbe_Check_Fk_فوری(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
  خالی sqlite3VdbePrintSql(Vdbe*);
  خالی sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  صحیح sqlite3VdbeMemTranslate(Mem*, u8);
  صحیح sqlite3VdbeMemHandleBom(Mem *pMem);
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
  صحیح sqlite3VdbeMemExpandBlob(Mem *);
  #تعریف_کرو ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#ورنہ
  #تعریف_کرو sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #تعریف_کرو ExpandBlob(P) SQLITE_OK
#اگر_ختم

#اگر_ختم /* !defined(SQLITE_VDBEINT_H) */
