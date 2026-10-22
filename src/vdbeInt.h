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
#నిర్వచితం_కాకపోతే SQLITE_VDBEINT_H
#నిర్వచించు SQLITE_VDBEINT_H

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#నిర్వచితం_కాకపోతే SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 50
#అయితే_ముగింపు

/*
** VDBE_DISPLAY_P4 is true or false depending on whether or not the
** "explain" P4 display logic is enabled.
*/
#ఒకవేళ !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#లేకపోతే
# define VDBE_DISPLAY_P4 0
#అయితే_ముగింపు

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
రకనిర్వచనం నిర్మాణం VdbeOp Op;

/*
** Boolean values
*/
రకనిర్వచనం గుర్తులేని Bool;

/* Opaque type used by code in vdbesort.c */
రకనిర్వచనం నిర్మాణం VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
రకనిర్వచనం నిర్మాణం Aux_దత్తాంశం Aux_దత్తాంశం;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
రకనిర్వచనం నిర్మాణం VdbeTxtBlbCache VdbeTxtBlbCache;

/* Types of VDBE cursors */
#నిర్వచించు CURTYPE_BTREE       0
#నిర్వచించు CURTYPE_SORTER      1
#నిర్వచించు CURTYPE_VTAB        2
#నిర్వచించు CURTYPE_PSEUDO      3

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
రకనిర్వచనం నిర్మాణం VdbeCursor VdbeCursor;
నిర్మాణం VdbeCursor {
  u8 eCurType;            /* One of the CURTYPE_* values above */
  i8 iDb;                 /* Index of cursor database in db->aDb[] */
  u8 null_అడ్డువరుస;             /* True if pointing to a row with no data */
  u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
  u8 is_పట్టిక;             /* True for rowid tables.  False for indexes */
#నిర్వచితమైతే SQLITE_DEBUG
  u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#అయితే_ముగింపు
  Bool isEphemeral:1;     /* True for an ephemeral table */
  Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
  Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
  Bool noReuse:1;         /* OpenEphemeral may not reuse this cursor */
  Bool నిలువు_Cache:1;        /* pCache pointer is initialized and non-NULL */
  u16 seek_తగిలింది;            /* See the OP_SeekHit and OP_IfNoHope opcodes */
  సంఘం {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seq_లెక్క;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cache_స్థితి;        /* Cache is valid if this matches Vdbe.cacheCtr */
  పూర్ణాంకం seek_ఫలితం;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match".
  ** For CURTYPE_PSEUDO, seekResult is the register holding the record */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  సంఘం {
    BtCursor *pCursor;          /* CURTYPE_BTREE or _PSEUDO.  Btree cursor */
    sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.              Vtab cursor */
    VdbeSorter *pSorter;        /* CURTYPE_SORTER.            Sorter object */
  } uc;
  కీలకం_Info *p_కీలకం_Info;      /* Info about index keys needed by index cursors */
  u32 i_Hdr_స్థానభ్రంశం;         /* Offset to next unparsed byte of the header */
  Pgno pgno_మూలం;          /* Root page of the open btree cursor */
  i16 n_క్షేత్రం;             /* Number of fields in the header */
  u16 nHdrParsed;         /* Number of header fields parsed so far */
  i64 moveto_లక్ష్యం;       /* Argument to the deferred sqlite3BtreeMoveto() */
  u32 *a_స్థానభ్రంశం;           /* Pointer to aType[nField] */
  స్థిరరాశి u8 *a_అడ్డువరుస;         /* Data for the current row, if all on one page */
  u32 payload_కొలత;        /* Total number of bytes in the record */
  u32 sz_అడ్డువరుస;              /* Byte available in aRow */
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_వాడిన_ముసుగు
  u64 ముసుగు_వాడిన;           /* Mask of columns used by this cursor */
#అయితే_ముగింపు
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
#నిర్వచించు SZ_VDBECURSOR(N) \
    (ROUND8(offsetof(VdbeCursor,aType)) + ((N)+1)*పరిమాణం(u64))

/* Return true if P is a null-only cursor
*/
#నిర్వచించు IsNullCursor(P) \
  ((P)->eCurType==CURTYPE_PSEUDO && (P)->null_అడ్డువరుస && (P)->seek_ఫలితం==0)

/*
** A value for VdbeCursor.cacheStatus that means the cache is always invalid.
*/
#నిర్వచించు CACHE_STALE 0

/*
** Large TEXT or BLOB values can be slow to load, so we want to avoid
** loading them more than once.  For that reason, large TEXT and BLOB values
** can be stored in a cache defined by this object, and attached to the
** VdbeCursor using the pCache field.
*/
నిర్మాణం VdbeTxtBlbCache {
  అక్షరం *pCValue;        /* A RCStr buffer to hold the value */
  i64 i_స్థానభ్రంశం;          /* File offset of the row being cached */
  పూర్ణాంకం i_నిలువు;             /* Column for which the cache is valid */
  u32 cache_స్థితి;      /* Vdbe.cacheCtr value */
  u32 నిలువు_Cache_Ctr;      /* Column cache counter */
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
రకనిర్వచనం నిర్మాణం Vdbe_చట్రం Vdbe_చట్రం;
నిర్మాణం Vdbe_చట్రం {
  Vdbe *v;                /* VM this frame belongs to */
  Vdbe_చట్రం *p_మాతృ;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  శూన్య *token;            /* Copy of SubProgram.token */
  i64 చివరి_Rowid;          /* Last insert rowid (sqlite3.lastRowid) */
  Aux_దత్తాంశం *p_Aux_దత్తాంశం;      /* Linked list of auxdata allocations */
#ఒకవేళ SQLITE_DEBUG
  u32 i_చట్రం_Magic;        /* magic number for sanity checking */
#అయితే_ముగింపు
  పూర్ణాంకం nCursor;            /* Number of entries in apCsr */
  పూర్ణాంకం pc;                 /* Program Counter in parent (calling) frame */
  పూర్ణాంకం nOp;                /* Size of aOp array */
  పూర్ణాంకం nMem;               /* Number of entries in aMem */
  పూర్ణాంకం n_బిడ్డ_Mem;          /* Number of memory cells for child frame */
  పూర్ణాంకం n_బిడ్డ_Csr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#నిర్వచించు SQLITE_చట్రం_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#నిర్వచించు Vdbe_చట్రం_Mem(p) ((Mem *)&((u8 *)p)[ROUND8(పరిమాణం(Vdbe_చట్రం))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
నిర్మాణం sqlite3_విలువ {
  సంఘం Mem_విలువ {
    ద్విగుణ r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    పూర్ణాంకం nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    స్థిరరాశి అక్షరం *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  అక్షరం *z;            /* String or BLOB value */
  పూర్ణాంకం n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  పూర్ణాంకం szMalloc;       /* Size of the zMalloc allocation */
  u32 u_తాత్కాలిక;          /* Transient storage for serial_type in OP_MakeRecord */
  అక్షరం *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  శూన్య (*xDel)(శూన్య*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#నిర్వచితమైతే SQLITE_DEBUG
  Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  u16 mScopyFlags;    /* flags value immediately after the shallow copy */
  u8  bScopy;         /* The pScopyFrom of some other Mem *might* point here */
#అయితే_ముగింపు
};

/*
** Size of struct Mem not including the Mem.zMalloc member or anything that
** follows.
*/
#నిర్వచించు MEMCELLSIZE offsetof(Mem,db)

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
#నిర్వచించు MEM_Undefined 0x0000   /* Value is undefined */
#నిర్వచించు MEM_Null      0x0001   /* Value is NULL (or a pointer) */
#నిర్వచించు MEM_Str       0x0002   /* Value is a string */
#నిర్వచించు MEM_Int       0x0004   /* Value is an integer */
#నిర్వచించు MEM_Real      0x0008   /* Value is a real number */
#నిర్వచించు MEM_Blob      0x0010   /* Value is a BLOB */
#నిర్వచించు MEM_IntReal   0x0020   /* MEM_Int that stringifies like MEM_Real */
#నిర్వచించు MEM_Aff_ముసుగు   0x003f   /* Mask of affinity bits */

/* Extra bits that modify the meanings of the core datatypes above
*/
#నిర్వచించు MEM_FromBind  0x0040   /* Value originates from sqlite3_bind() */
 /*                   0x0080   // Available */
#నిర్వచించు MEM_Cleared   0x0100   /* NULL set by OP_Null, not from data */
#నిర్వచించు MEM_పదం      0x0200   /* String in Mem.z is zero terminated */
#నిర్వచించు MEM_Zero      0x0400   /* Mem.i contains count of 0s appended to blob */
#నిర్వచించు MEM_Subtype   0x0800   /* Mem.eSubtype is valid */
#నిర్వచించు MEM_Type_ముసుగు  0x0dbf   /* Mask of type bits */

/* Bits that determine the storage for Mem.z for a string or blob or
** aggregate accumulator.
*/
#నిర్వచించు MEM_Dyn       0x1000   /* Need to call Mem.xDel() on Mem.z */
#నిర్వచించు MEM_Static    0x2000   /* Mem.z points to a static string */
#నిర్వచించు MEM_Ephem     0x4000   /* Mem.z points to an ephemeral string */
#నిర్వచించు MEM_Agg       0x8000   /* Mem.z points to an agg function context */

/* Return TRUE if Mem X contains dynamically allocated content - anything
** that needs to be deallocated to avoid a leak.
*/
#నిర్వచించు VdbeMemDynamic(X)  \
  (((X)->flags&(MEM_Agg|MEM_Dyn))!=0)

/*
** Clear any existing type flags from a Mem and replace them with f
*/
#నిర్వచించు Mem_సమితి_Type_Flag(p, f) \
   ((p)->flags = ((p)->flags&~(MEM_Type_ముసుగు|MEM_Zero))|f)

/*
** True if Mem X is a NULL-nochng type.
*/
#నిర్వచించు MemNullNochng(X) \
  (((X)->flags&MEM_Type_ముసుగు)==(MEM_Null|MEM_Zero) \
    && (X)->n==0 && (X)->u.nZero==0)

/*
** Return true if a memory cell has been initialized and is valid.
** is for use inside assert() statements only.
**
** A Memory cell is initialized if at least one of the
** MEM_Null, MEM_Str, MEM_Int, MEM_Real, MEM_Blob, or MEM_IntReal bits
** is set.  It is "undefined" if all those bits are zero.
*/
#నిర్వచితమైతే SQLITE_DEBUG
#నిర్వచించు memIsValid(M)  ((M)->flags & MEM_Aff_ముసుగు)!=0
#అయితే_ముగింపు

/*
** Each auxiliary data pointer stored by a user defined function
** implementation calling sqlite3_set_auxdata() is stored in an instance
** of this structure. All such structures associated with a single VM
** are stored in a linked list headed at Vdbe.pAuxData. All are destroyed
** when the VM is halted (if not before).
*/
నిర్మాణం Aux_దత్తాంశం {
  పూర్ణాంకం iAuxOp;                     /* Instruction number of OP_Function opcode */
  పూర్ణాంకం iAuxArg;                    /* Index of function argument. */
  శూన్య *pAux;                     /* Aux data pointer */
  శూన్య (*xDeleteAux)(శూన్య*);      /* Destructor for the aux data */
  Aux_దత్తాంశం *p_తదుపరి_Aux;              /* Next element in list */
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
నిర్మాణం sqlite3_context {
  Mem *p_బహిర్గతం;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  పూర్ణాంకం iOp;                /* Instruction number of OP_Function */
  పూర్ణాంకం isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_విలువ *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#నిర్వచించు SZ_CONTEXT(N)  \
   (offsetof(sqlite3_context,argv)+(N)*పరిమాణం(sqlite3_విలువ*))


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
రకనిర్వచనం నిర్మాణం పరిశీలన_స్థితి పరిశీలన_స్థితి;
నిర్మాణం పరిశీలన_స్థితి {
  పూర్ణాంకం addrExplain;                /* OP_Explain for loop */
  పూర్ణాంకం aAddrRange[6];
  పూర్ణాంకం addrLoop;                   /* Address of "loops" counter */
  పూర్ణాంకం addr_సందర్శించు;                  /* Address of "rows visited" counter */
  పూర్ణాంకం iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  అక్షరం *zName;                    /* Name of table or index */
};

/* The DblquoteStr object holds the text of a double-quoted
** string for a prepared statement.  A linked list of these objects
** is constructed during statement parsing and is held on Vdbe.pDblStr.
** When computing a normalized SQL statement for an SQL statement, that
** list is consulted for each double-quoted identifier to see if the
** identifier should really be a string literal.
*/
రకనిర్వచనం నిర్మాణం DblquoteStr DblquoteStr;
నిర్మాణం DblquoteStr {
  DblquoteStr *p_తదుపరి_Str;   /* Next string literal in the list */
  అక్షరం z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
నిర్మాణం Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  పూర్ణాంకం nMem;               /* Number of memory locations currently allocated */
  పూర్ణాంకం nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  పూర్ణాంకం pc;                 /* The program counter */
  పూర్ణాంకం rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  పూర్ణాంకం iStatement;         /* Statement number (or 0 if has no opened stmt) */
  i64 i_Current_సమయం;       /* Value of julianday('now') for this statement */
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
  పూర్ణాంకం nOp;                /* Number of instructions in the program */
  పూర్ణాంకం nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *a_నిలువు_Name;          /* Column names to return */
  Mem *p_ఫలితం_అడ్డువరుస;        /* Current output row */
  అక్షరం *z_Err_సందేశం;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
  i64 ప్రారంభం_సమయం;          /* Time when query started - used for profiling */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
  పూర్ణాంకం rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  పూర్ణాంకం napArg;             /* Size of the apArg[] array */
#అయితే_ముగింపు
  u16 n_Res_స్తంభం;         /* Number of columns in one row of the result set */
  u16 nResAlloc;          /* Column slots allocated to aColName[] */
  u8 errorAction;         /* Recovery action to do in case of an error */
  u8 minWriteFileFormat;  /* Minimum file format for writable database files */
  u8 prepFlags;           /* SQLITE_PREPARE_* flags */
  u8 eVdbeState;          /* On of the VDBE_*_STATE values */
  bft expired:2;          /* 1: recompile VM immediately  2: when convenient */
  bft explain:2;          /* 0: normal, 1: EXPLAIN, 2: EXPLAIN QUERY PLAN */
  bft changeCntOn:1;      /* True to update the change-counter */
  bft ఉపయోగిస్తుంది_Stmt_Journal:1;  /* True if uses a statement journal */
  bft readOnly:1;         /* True for statements that do not write */
  bft bIsReader:1;        /* True for statements that read */
  bft haveEqpOps:1;       /* Bytecode supports EXPLAIN QUERY PLAN */
  y_Db_ముసుగు btree_ముసుగు;      /* Bitmask of db->aDb[] entries referenced */
  y_Db_ముసుగు తాళం_ముసుగు;       /* Subset of btreeMask that requires a lock */
  u32 aCounter[9];        /* Counters used by sqlite3_stmt_status() */
  అక్షరం *zSql;             /* Text of the SQL statement that generated this */
#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
  అక్షరం *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#అయితే_ముగింపు
  శూన్య *pFree;            /* Free this when deleting the vdbe */
  Vdbe_చట్రం *p_చట్రం;      /* Parent frame */
  Vdbe_చట్రం *p_Del_చట్రం;   /* List of frame objects to free on VM reset */
  పూర్ణాంకం n_చట్రం;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  Sub_కార్యక్రమం *p_కార్యక్రమం;   /* Linked list of all sub-programs used by VM */
  Aux_దత్తాంశం *p_Aux_దత్తాంశం;      /* Linked list of auxdata allocations */
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  పూర్ణాంకం n_పరిశీలన;              /* Entries in aScan[] */
  పరిశీలన_స్థితి *a_పరిశీలన;      /* Scan definitions for sqlite3_stmt_scanstatus() */
#అయితే_ముగింపు
};

/*
** The following are allowed values for Vdbe.eVdbeState
*/
#నిర్వచించు VDBE_INIT_STATE     0   /* Prepared statement under construction */
#నిర్వచించు VDBE_READY_STATE    1   /* Ready to run but not yet started */
#నిర్వచించు VDBE_నిరంతరం_STATE      2   /* Run in progress */
#నిర్వచించు VDBE_HALT_STATE     3   /* Finished.  Need reset() or finalize() */

/*
** Structure used to store the context required by the
** sqlite3_preupdate_*() API functions.
*/
నిర్మాణం PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  పూర్ణాంకం op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *a_రికార్డు;                    /* old.* database record */
  కీలకం_Info *pKeyinfo;              /* Key information */
  Unpacked_రికార్డు *pUnpacked;      /* Unpacked version of aRecord[] */
  Unpacked_రికార్డు *pNewUnpacked;   /* Unpacked version of new.* record */
  పూర్ణాంకం iNewReg;                    /* Register for new.* values */
  పూర్ణాంకం iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  పట్టిక *pTab;                    /* Schema object being updated */
  సూచీ *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_విలువ **apDflt;         /* Array of default values, if required */
  నిర్మాణం {
    u8 keyinfoSpace[SZ_KEYINFO_0];  /* Space to hold pKeyinfo[0] content */
  } u_కీలకం;
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
రకనిర్వచనం నిర్మాణం విలువ_List విలువ_List;
నిర్మాణం విలువ_List {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_విలువ *p_బహిర్గతం;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#నిర్వచితం_కాకపోతే SQLITE_AMALGAMATION
బాహ్య స్థిరరాశి u8 sqlite3SmallTypeSizes[];
#అయితే_ముగింపు

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#నిర్వచించు ONE_BYTE_INT(x)    ((i8)(x)[0])
#నిర్వచించు TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#నిర్వచించు THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#నిర్వచించు FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#నిర్వచించు FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#నిర్వచించు FOUR_BYTE_INT(x)   ((పూర్ణాంకం)FOUR_BYTE_UINT(x))
#నిర్వచించు SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
శూన్య sqlite3VdbeError(Vdbe*, స్థిరరాశి అక్షరం *, ...);
శూన్య sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
శూన్య sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
శూన్య sqlite_Vdbe_తీయి_దొంతర(Vdbe*,పూర్ణాంకం);
పూర్ణాంకం SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
పూర్ణాంకం SQLITE_NOINLINE sqlite3_Vdbe_ముగింపు_Moveto(VdbeCursor*);
పూర్ణాంకం sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#నిర్వచితమైతే SQLITE_MIXED_ENDIAN_64_బిట్టు_FLOAT
  u64 sqlite3_Float_మార్పిడి(u64 లోపల);
# define మార్పిడి_Mixed_Endian_Float(X)  X = sqlite3_Float_మార్పిడి(X)
#లేకపోతే
# define మార్పిడి_Mixed_Endian_Float(X)
#అయితే_ముగింపు
శూన్య sqlite3VdbeSerialGet(స్థిరరాశి గుర్తులేని అక్షరం*, u32, Mem*);
శూన్య sqlite3_Vdbe_Delete_Aux_దత్తాంశం(sqlite3*, Aux_దత్తాంశం**, పూర్ణాంకం, పూర్ణాంకం);

పూర్ణాంకం sqlite2_Btree_కీలకం_Compare(BtCursor *, స్థిరరాశి శూన్య *, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం *);
పూర్ణాంకం sqlite3_Vdbe_Idx_కీలకం_Compare(sqlite3*,VdbeCursor*,Unpacked_రికార్డు*,పూర్ణాంకం*);
పూర్ణాంకం sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
పూర్ణాంకం sqlite3VdbeExec(Vdbe*);
#ఒకవేళ !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
పూర్ణాంకం sqlite3_Vdbe_తదుపరి_Opcode(Vdbe*,Mem*,పూర్ణాంకం,పూర్ణాంకం*,పూర్ణాంకం*,Op**);
అక్షరం *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
అక్షరం *sqlite3VdbeDisplayComment(sqlite3*,స్థిరరాశి Op*,స్థిరరాశి అక్షరం*);
#అయితే_ముగింపు
#ఒకవేళ !defined(SQLITE_OMIT_EXPLAIN)
పూర్ణాంకం sqlite3VdbeList(Vdbe*);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3VdbeHalt(Vdbe*);
పూర్ణాంకం sqlite3VdbeChangeEncoding(Mem *, పూర్ణాంకం);
పూర్ణాంకం sqlite3VdbeMemTooBig(Mem*);
పూర్ణాంకం sqlite3VdbeMemCopy(Mem*, స్థిరరాశి Mem*);
శూన్య sqlite3VdbeMemShallowCopy(Mem*, స్థిరరాశి Mem*, పూర్ణాంకం);
శూన్య sqlite3VdbeMemMove(Mem*, Mem*);
పూర్ణాంకం sqlite3VdbeMemNulTerminate(Mem*);
పూర్ణాంకం sqlite3_Vdbe_Mem_సమితి_Str(Mem*, స్థిరరాశి అక్షరం*, i64, u8, శూన్య(*)(శూన్య*));
పూర్ణాంకం sqlite3_Vdbe_Mem_సమితి_పాఠ్యం(Mem*, స్థిరరాశి అక్షరం*, i64, శూన్య(*)(శూన్య*));
శూన్య sqlite3_Vdbe_Mem_సమితి_Int64(Mem*, i64);
#నిర్వచితమైతే SQLITE_OMIT_FLOATING_బిందువు
# define sqlite3_Vdbe_Mem_సమితి_Double sqlite3_Vdbe_Mem_సమితి_Int64
#లేకపోతే
  శూన్య sqlite3_Vdbe_Mem_సమితి_Double(Mem*, ద్విగుణ);
#అయితే_ముగింపు
శూన్య sqlite3_Vdbe_Mem_సమితి_సూచిక(Mem*, శూన్య*, స్థిరరాశి అక్షరం*, శూన్య(*)(శూన్య*));
శూన్య sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
శూన్య sqlite3_Vdbe_Mem_సమితి_Null(Mem*);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
శూన్య sqlite3_Vdbe_Mem_సమితి_Zero_Blob(Mem*,పూర్ణాంకం);
#లేకపోతే
పూర్ణాంకం sqlite3_Vdbe_Mem_సమితి_Zero_Blob(Mem*,పూర్ణాంకం);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
పూర్ణాంకం sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(స్థిరరాశి Mem*);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3_Vdbe_Mem_సమితి_అడ్డువరుస_సమితి(Mem*);
పూర్ణాంకం sqlite3VdbeMemZeroTerminateIfAble(Mem*);
పూర్ణాంకం sqlite3VdbeMemMakeWriteable(Mem*);
పూర్ణాంకం sqlite3VdbeMemStringify(Mem*, u8, u8);
పూర్ణాంకం sqlite3IntFloatCompare(i64,ద్విగుణ);
i64 sqlite3_Vdbe_Int_విలువ(స్థిరరాశి Mem*);
పూర్ణాంకం sqlite3VdbeMemIntegerify(Mem*);
ద్విగుణ sqlite3_Vdbe_Real_విలువ(Mem*);
పూర్ణాంకం sqlite3_Mem_Real_విలువ_RC(Mem*, ద్విగుణ*);
పూర్ణాంకం sqlite3_Vdbe_Boolean_విలువ(Mem*, పూర్ణాంకం ifNull);
శూన్య sqlite3VdbeIntegerAffinity(Mem*,పూర్ణాంకం b_From_పాఠ్యం);
పూర్ణాంకం sqlite3VdbeMemRealify(Mem*);
పూర్ణాంకం sqlite3VdbeMemNumerify(Mem*);
పూర్ణాంకం sqlite3VdbeMemCast(Mem*,u8,u8);
పూర్ణాంకం sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
పూర్ణాంకం sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(BtCursor*,u32,Mem*);
శూన్య sqlite3VdbeMemRelease(Mem *p);
శూన్య sqlite3VdbeMemReleaseMalloc(Mem*p);
పూర్ణాంకం sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
పూర్ణాంకం sqlite3_Vdbe_Mem_Agg_విలువ(Mem*, Mem*, FuncDef*);
#అయితే_ముగింపు
#ఒకవేళ !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
స్థిరరాశి అక్షరం *sqlite3OpcodeName(పూర్ణాంకం);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3VdbeMemGrow(Mem *pMem, పూర్ణాంకం n, పూర్ణాంకం preserve);
పూర్ణాంకం sqlite3VdbeMemClearAndResize(Mem *pMem, పూర్ణాంకం n);
పూర్ణాంకం sqlite3VdbeCloseStatement(Vdbe *, పూర్ణాంకం);
#నిర్వచితమైతే SQLITE_DEBUG
పూర్ణాంకం sqlite3_Vdbe_చట్రం_Is_Valid(Vdbe_చట్రం*);
#అయితే_ముగింపు
శూన్య sqlite3_Vdbe_చట్రం_Mem_Del(శూన్య*);      /* Destructor on Mem */
శూన్య sqlite3_Vdbe_చట్రం_Delete(Vdbe_చట్రం*); /* Actually deletes the Frame */
పూర్ణాంకం sqlite3_Vdbe_చట్రం_Restore(Vdbe_చట్రం *);
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
శూన్య sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,పూర్ణాంకం,స్థిరరాశి అక్షరం*,పట్టిక*,i64,పూర్ణాంకం,పూర్ణాంకం);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3VdbeTransferError(Vdbe *p);
పూర్ణాంకం sqlite3_Vdbe_కనుగొను_సూచీ_కీలకం(BtCursor*, సూచీ*, Unpacked_రికార్డు*, పూర్ణాంకం*, పూర్ణాంకం);

పూర్ణాంకం sqlite3VdbeSorterInit(sqlite3 *, పూర్ణాంకం, VdbeCursor *);
శూన్య sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
శూన్య sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
పూర్ణాంకం sqlite3VdbeSorterRowkey(స్థిరరాశి VdbeCursor *, Mem *);
పూర్ణాంకం sqlite3_Vdbe_Sorter_తదుపరి(sqlite3 *, స్థిరరాశి VdbeCursor *);
పూర్ణాంకం sqlite3VdbeSorterRewind(స్థిరరాశి VdbeCursor *, పూర్ణాంకం *);
పూర్ణాంకం sqlite3VdbeSorterWrite(స్థిరరాశి VdbeCursor *, Mem *);
పూర్ణాంకం sqlite3VdbeSorterCompare(స్థిరరాశి VdbeCursor *, Mem *, పూర్ణాంకం, పూర్ణాంకం *);

శూన్య sqlite3_Vdbe_విలువ_List_Free(శూన్య*);

#నిర్వచితమైతే SQLITE_DEBUG
  శూన్య sqlite3VdbeIncrWriteCounter(Vdbe*, VdbeCursor*);
  శూన్య sqlite3VdbeAssertAbortable(Vdbe*);
#లేకపోతే
# define sqlite3VdbeIncrWriteCounter(V,C)
# define sqlite3VdbeAssertAbortable(V)
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_SHARED_CACHE)
  శూన్య sqlite3VdbeEnter(Vdbe*);
#లేకపోతే
# define sqlite3VdbeEnter(X)
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  శూన్య sqlite3VdbeLeave(Vdbe*);
#లేకపోతే
# define sqlite3VdbeLeave(X)
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
శూన్య sqlite3_Vdbe_Mem_About_గమ్యం_Change(Vdbe*,Mem*);
పూర్ణాంకం sqlite3VdbeCheckMemInvariants(Mem*);
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
పూర్ణాంకం sqlite3_Vdbe_Check_Fk_తక్షణ(Vdbe*);
పూర్ణాంకం sqlite3VdbeCheckFkDeferred(Vdbe*);
#లేకపోతే
# define sqlite3_Vdbe_Check_Fk_తక్షణ(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
  శూన్య sqlite3VdbePrintSql(Vdbe*);
  శూన్య sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  పూర్ణాంకం sqlite3VdbeMemTranslate(Mem*, u8);
  పూర్ణాంకం sqlite3VdbeMemHandleBom(Mem *pMem);
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
  పూర్ణాంకం sqlite3VdbeMemExpandBlob(Mem *);
  #నిర్వచించు ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#లేకపోతే
  #నిర్వచించు sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #నిర్వచించు ExpandBlob(P) SQLITE_OK
#అయితే_ముగింపు

#అయితే_ముగింపు /* !defined(SQLITE_VDBEINT_H) */
