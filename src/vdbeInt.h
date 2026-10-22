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
#अगर_अघोषित SQLITE_VDBEINT_H
#घोषणा SQLITE_VDBEINT_H

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#अगर_अघोषित SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 50
#पूर्ण_अगर

/*
** VDBE_DISPLAY_P4 is true or false depending on whether or not the
** "explain" P4 display logic is enabled.
*/
#अगर !defined(SQLITE_OMIT_EXPLAIN) || !defined(न_संशोधन) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG) \
     || defined(SQLITE_ENABLE_BYTECODE_VTAB)
# define VDBE_DISPLAY_P4 1
#अन्यथा
# define VDBE_DISPLAY_P4 0
#पूर्ण_अगर

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
प्रकार काष्ठा VdbeOp Op;

/*
** Boolean values
*/
प्रकार अचिन्हित Bool;

/* Opaque type used by code in vdbesort.c */
प्रकार काष्ठा VdbeSorter VdbeSorter;

/* Elements of the linked list at Vdbe.pAuxData */
प्रकार काष्ठा Aux_तथ्य Aux_तथ्य;

/* A cache of large TEXT or BLOB values in a VdbeCursor */
प्रकार काष्ठा VdbeTxtBlbCache VdbeTxtBlbCache;

/* Types of VDBE cursors */
#घोषणा CURTYPE_BTREE       0
#घोषणा CURTYPE_SORTER      1
#घोषणा CURTYPE_VTAB        2
#घोषणा CURTYPE_PSEUDO      3

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
प्रकार काष्ठा VdbeCursor VdbeCursor;
काष्ठा VdbeCursor {
  u8 eCurType;            /* One of the CURTYPE_* values above */
  i8 iDb;                 /* Index of cursor database in db->aDb[] */
  u8 nullRow;             /* True if pointing to a row with no data */
  u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
  u8 isTable;             /* True for rowid tables.  False for indexes */
#अगर_घोषित SQLITE_DEBUG
  u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#पूर्ण_अगर
  Bool isEphemeral:1;     /* True for an ephemeral table */
  Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
  Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
  Bool noReuse:1;         /* OpenEphemeral may not reuse this cursor */
  Bool colCache:1;        /* pCache pointer is initialized and non-NULL */
  u16 seekHit;            /* See the OP_SeekHit and OP_IfNoHope opcodes */
  जोड़ {                 /* pBtx for isEphermeral.  pAltMap otherwise */
    Btree *pBtx;            /* Separate file holding temporary table */
    u32 *aAltMap;           /* Mapping from table to index column numbers */
  } ub;
  i64 seq_गिनती;           /* Sequence counter */

  /* Cached OP_Column parse information is only valid if cacheStatus matches
  ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
  ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
  ** the cache is out of date. */
  u32 cacheStatus;        /* Cache is valid if this matches Vdbe.cacheCtr */
  पूर्णांक seek_परिणाम;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
  /* seekResult does not distinguish between "no seeks have ever occurred
  ** on this cursor" and "the most recent seek was an exact match".
  ** For CURTYPE_PSEUDO, seekResult is the register holding the record */

  /* When a new VdbeCursor is allocated, only the fields above are zeroed.
  ** The fields that follow are uninitialized, and must be individually
  ** initialized prior to first use. */
  VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
  जोड़ {
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
  स्थिर u8 *aRow;         /* Data for the current row, if all on one page */
  u32 payload_आकार;        /* Total number of bytes in the record */
  u32 szRow;              /* Byte available in aRow */
#अगर_घोषित SQLITE_ENABLE_COLUMN_USED_MASK
  u64 maskUsed;           /* Mask of columns used by this cursor */
#पूर्ण_अगर
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
#घोषणा SZ_VDBECURSOR(N) \
    (ROUND8(दुरत्व(VdbeCursor,aType)) + ((N)+1)*माप(u64))

/* Return true if P is a null-only cursor
*/
#घोषणा IsNullCursor(P) \
  ((P)->eCurType==CURTYPE_PSEUDO && (P)->nullRow && (P)->seek_परिणाम==0)

/*
** A value for VdbeCursor.cacheStatus that means the cache is always invalid.
*/
#घोषणा CACHE_STALE 0

/*
** Large TEXT or BLOB values can be slow to load, so we want to avoid
** loading them more than once.  For that reason, large TEXT and BLOB values
** can be stored in a cache defined by this object, and attached to the
** VdbeCursor using the pCache field.
*/
काष्ठा VdbeTxtBlbCache {
  अक्षर *pCValue;        /* A RCStr buffer to hold the value */
  i64 iOffset;          /* File offset of the row being cached */
  पूर्णांक iCol;             /* Column for which the cache is valid */
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
प्रकार काष्ठा VdbeFrame VdbeFrame;
काष्ठा VdbeFrame {
  Vdbe *v;                /* VM this frame belongs to */
  VdbeFrame *pParent;     /* Parent of this frame, or NULL if parent is main */
  Op *aOp;                /* Program instructions for parent frame */
  Mem *aMem;              /* Array of memory cells for parent frame */
  VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
  u8 *aOnce;              /* Bitmask used by OP_Once */
  व्योम *token;            /* Copy of SubProgram.token */
  i64 lastRowid;          /* Last insert rowid (sqlite3.lastRowid) */
  Aux_तथ्य *p_Aux_तथ्य;      /* Linked list of auxdata allocations */
#अगर SQLITE_DEBUG
  u32 iFrameMagic;        /* magic number for sanity checking */
#पूर्ण_अगर
  पूर्णांक nCursor;            /* Number of entries in apCsr */
  पूर्णांक pc;                 /* Program Counter in parent (calling) frame */
  पूर्णांक nOp;                /* Size of aOp array */
  पूर्णांक nMem;               /* Number of entries in aMem */
  पूर्णांक nChildMem;          /* Number of memory cells for child frame */
  पूर्णांक nChildCsr;          /* Number of cursors for child frame */
  i64 nChange;            /* Statement changes (Vdbe.nChange)     */
  i64 nDbChange;          /* Value of db->nChange */
};

/* Magic number for sanity checking on VdbeFrame objects */
#घोषणा SQLITE_FRAME_MAGIC 0x879fb71e

/*
** Return a pointer to the array of registers allocated for use
** by a VdbeFrame.
*/
#घोषणा VdbeFrameMem(p) ((Mem *)&((u8 *)p)[ROUND8(माप(VdbeFrame))])

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
काष्ठा sqlite3_मूल्य {
  जोड़ Mem_मूल्य {
    द्विगुन r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    पूर्णांक nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    स्थिर अक्षर *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  अक्षर *z;            /* String or BLOB value */
  पूर्णांक n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  sqlite3 *db;        /* The associated database connection */
  पूर्णांक szMalloc;       /* Size of the zMalloc allocation */
  u32 u_अस्थायी;          /* Transient storage for serial_type in OP_MakeRecord */
  अक्षर *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  व्योम (*xDel)(व्योम*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#अगर_घोषित SQLITE_DEBUG
  Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  u16 mScopyFlags;    /* flags value immediately after the shallow copy */
  u8  bScopy;         /* The pScopyFrom of some other Mem *might* point here */
#पूर्ण_अगर
};

/*
** Size of struct Mem not including the Mem.zMalloc member or anything that
** follows.
*/
#घोषणा MEMCELLSIZE दुरत्व(Mem,db)

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
#घोषणा MEM_Undefined 0x0000   /* Value is undefined */
#घोषणा MEM_Null      0x0001   /* Value is NULL (or a pointer) */
#घोषणा MEM_Str       0x0002   /* Value is a string */
#घोषणा MEM_Int       0x0004   /* Value is an integer */
#घोषणा MEM_Real      0x0008   /* Value is a real number */
#घोषणा MEM_Blob      0x0010   /* Value is a BLOB */
#घोषणा MEM_IntReal   0x0020   /* MEM_Int that stringifies like MEM_Real */
#घोषणा MEM_AffMask   0x003f   /* Mask of affinity bits */

/* Extra bits that modify the meanings of the core datatypes above
*/
#घोषणा MEM_FromBind  0x0040   /* Value originates from sqlite3_bind() */
 /*                   0x0080   // Available */
#घोषणा MEM_Cleared   0x0100   /* NULL set by OP_Null, not from data */
#घोषणा MEM_Term      0x0200   /* String in Mem.z is zero terminated */
#घोषणा MEM_Zero      0x0400   /* Mem.i contains count of 0s appended to blob */
#घोषणा MEM_Subtype   0x0800   /* Mem.eSubtype is valid */
#घोषणा MEM_TypeMask  0x0dbf   /* Mask of type bits */

/* Bits that determine the storage for Mem.z for a string or blob or
** aggregate accumulator.
*/
#घोषणा MEM_Dyn       0x1000   /* Need to call Mem.xDel() on Mem.z */
#घोषणा MEM_Static    0x2000   /* Mem.z points to a static string */
#घोषणा MEM_Ephem     0x4000   /* Mem.z points to an ephemeral string */
#घोषणा MEM_Agg       0x8000   /* Mem.z points to an agg function context */

/* Return TRUE if Mem X contains dynamically allocated content - anything
** that needs to be deallocated to avoid a leak.
*/
#घोषणा VdbeMemDynamic(X)  \
  (((X)->flags&(MEM_Agg|MEM_Dyn))!=0)

/*
** Clear any existing type flags from a Mem and replace them with f
*/
#घोषणा MemSetTypeFlag(p, f) \
   ((p)->flags = ((p)->flags&~(MEM_TypeMask|MEM_Zero))|f)

/*
** True if Mem X is a NULL-nochng type.
*/
#घोषणा MemNullNochng(X) \
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
#अगर_घोषित SQLITE_DEBUG
#घोषणा memIsValid(M)  ((M)->flags & MEM_AffMask)!=0
#पूर्ण_अगर

/*
** Each auxiliary data pointer stored by a user defined function
** implementation calling sqlite3_set_auxdata() is stored in an instance
** of this structure. All such structures associated with a single VM
** are stored in a linked list headed at Vdbe.pAuxData. All are destroyed
** when the VM is halted (if not before).
*/
काष्ठा Aux_तथ्य {
  पूर्णांक iAuxOp;                     /* Instruction number of OP_Function opcode */
  पूर्णांक iAuxArg;                    /* Index of function argument. */
  व्योम *pAux;                     /* Aux data pointer */
  व्योम (*xDeleteAux)(व्योम*);      /* Destructor for the aux data */
  Aux_तथ्य *p_अग्र_Aux;              /* Next element in list */
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
काष्ठा sqlite3_context {
  Mem *pOut;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  पूर्णांक iOp;                /* Instruction number of OP_Function */
  पूर्णांक isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u16 argc;               /* Number of arguments */
  sqlite3_मूल्य *argv[FLEXARRAY]; /* Argument set */
};

/*
** The size (in bytes) of an sqlite3_context object that holds N
** argv[] arguments.
*/
#घोषणा SZ_CONTEXT(N)  \
   (दुरत्व(sqlite3_context,argv)+(N)*माप(sqlite3_मूल्य*))


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
प्रकार काष्ठा ScanStatus ScanStatus;
काष्ठा ScanStatus {
  पूर्णांक addrExplain;                /* OP_Explain for loop */
  पूर्णांक aAddrRange[6];
  पूर्णांक addrLoop;                   /* Address of "loops" counter */
  पूर्णांक addrVisit;                  /* Address of "rows visited" counter */
  पूर्णांक iSelectID;                  /* The "Select-ID" for this loop */
  LogEst nEst;                    /* Estimated output rows per loop */
  अक्षर *zName;                    /* Name of table or index */
};

/* The DblquoteStr object holds the text of a double-quoted
** string for a prepared statement.  A linked list of these objects
** is constructed during statement parsing and is held on Vdbe.pDblStr.
** When computing a normalized SQL statement for an SQL statement, that
** list is consulted for each double-quoted identifier to see if the
** identifier should really be a string literal.
*/
प्रकार काष्ठा DblquoteStr DblquoteStr;
काष्ठा DblquoteStr {
  DblquoteStr *p_अग्र_Str;   /* Next string literal in the list */
  अक्षर z[8];               /* Dequoted value for the string */
};

/*
** An instance of the virtual machine.  This structure contains the complete
** state of the virtual machine.
**
** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
** is really a pointer to an instance of this structure.
*/
काष्ठा Vdbe {
  sqlite3 *db;            /* The database connection that owns this statement */
  Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
  Parse *pParse;          /* Parsing context used to create this Vdbe */
  ynVar nVar;             /* Number of entries in aVar[] */
  पूर्णांक nMem;               /* Number of memory locations currently allocated */
  पूर्णांक nCursor;            /* Number of slots in apCsr[] */
  u32 cacheCtr;           /* VdbeCursor row cache generation counter */
  पूर्णांक pc;                 /* The program counter */
  पूर्णांक rc;                 /* Value to return */
  i64 nChange;            /* Number of db changes made since last reset */
  पूर्णांक iStatement;         /* Statement number (or 0 if has no opened stmt) */
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
  पूर्णांक nOp;                /* Number of instructions in the program */
  पूर्णांक nOpAlloc;           /* Slots allocated for aOp[] */
  Mem *aColName;          /* Column names to return */
  Mem *p_परिणाम_Row;        /* Current output row */
  अक्षर *z_Err_संदेश;          /* Error message written here */
  VList *pVList;          /* Name of variables */
#अगर_अघोषित SQLITE_OMIT_TRACE
  i64 startTime;          /* Time when query started - used for profiling */
#पूर्ण_अगर
#अगर_घोषित SQLITE_DEBUG
  पूर्णांक rcApp;              /* errcode set by sqlite3_result_error_code() */
  u32 nWrite;             /* Number of write operations that have occurred */
  पूर्णांक napArg;             /* Size of the apArg[] array */
#पूर्ण_अगर
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
  u32 a_गणक[9];        /* Counters used by sqlite3_stmt_status() */
  अक्षर *zSql;             /* Text of the SQL statement that generated this */
#अगर_घोषित SQLITE_ENABLE_NORMALIZE
  अक्षर *zNormSql;         /* Normalization of the associated SQL statement */
  DblquoteStr *pDblStr;   /* List of double-quoted string literals */
#पूर्ण_अगर
  व्योम *pFree;            /* Free this when deleting the vdbe */
  VdbeFrame *pFrame;      /* Parent frame */
  VdbeFrame *pDelFrame;   /* List of frame objects to free on VM reset */
  पूर्णांक nFrame;             /* Number of frames in pFrame list */
  u32 expmask;            /* Binding to these vars might invalidate VM */
  u32 smimask;            /* Only invalid if changing to/from small integer */
                          /* Note: smimask is always a subset of expmask */
  SubProgram *pProgram;   /* Linked list of all sub-programs used by VM */
  Aux_तथ्य *p_Aux_तथ्य;      /* Linked list of auxdata allocations */
#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णांक nScan;              /* Entries in aScan[] */
  ScanStatus *aScan;      /* Scan definitions for sqlite3_stmt_scanstatus() */
#पूर्ण_अगर
};

/*
** The following are allowed values for Vdbe.eVdbeState
*/
#घोषणा VDBE_INIT_STATE     0   /* Prepared statement under construction */
#घोषणा VDBE_READY_STATE    1   /* Ready to run but not yet started */
#घोषणा VDBE_RUN_STATE      2   /* Run in progress */
#घोषणा VDBE_HALT_STATE     3   /* Finished.  Need reset() or finalize() */

/*
** Structure used to store the context required by the
** sqlite3_preupdate_*() API functions.
*/
काष्ठा PreUpdate {
  Vdbe *v;
  VdbeCursor *pCsr;               /* Cursor to read old values from */
  पूर्णांक op;                         /* One of SQLITE_INSERT, UPDATE, DELETE */
  u8 *aRecord;                    /* old.* database record */
  कुंजी_Info *pKeyinfo;              /* Key information */
  UnpackedRecord *pUnpacked;      /* Unpacked version of aRecord[] */
  UnpackedRecord *pNewUnpacked;   /* Unpacked version of new.* record */
  पूर्णांक iNewReg;                    /* Register for new.* values */
  पूर्णांक iBlobWrite;                 /* Value returned by preupdate_blobwrite() */
  i64 iKey1;                      /* First key value passed to hook */
  i64 iKey2;                      /* Second key value passed to hook */
  Mem oldipk;                     /* Memory cell holding "old" IPK value */
  Mem *aNew;                      /* Array of new.* values */
  Table *pTab;                    /* Schema object being updated */
  सूचकांक *pPk;                     /* PK index if pTab is WITHOUT ROWID */
  sqlite3_मूल्य **apDflt;         /* Array of default values, if required */
  काष्ठा {
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
प्रकार काष्ठा मूल्य_List मूल्य_List;
काष्ठा मूल्य_List {
  BtCursor *pCsr;          /* An ephemeral table holding all values */
  sqlite3_मूल्य *pOut;     /* Register to hold each decoded output value */
};

/* Size of content associated with serial types that fit into a
** single-byte varint.
*/
#अगर_अघोषित SQLITE_AMALGAMATION
बाह्य स्थिर u8 sqlite3SmallTypeSizes[];
#पूर्ण_अगर

/* Input "x" is a sequence of unsigned characters that represent a
** big-endian integer.  Return the equivalent native integer
*/
#घोषणा ONE_BYTE_INT(x)    ((i8)(x)[0])
#घोषणा TWO_BYTE_INT(x)    (256*(i8)((x)[0])|(x)[1])
#घोषणा THREE_BYTE_INT(x)  (65536*(i8)((x)[0])|((x)[1]<<8)|(x)[2])
#घोषणा FOUR_BYTE_UINT(x)  (((u32)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#घोषणा FOUR_BYTE_U64(x)   (((u64)(x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#घोषणा FOUR_BYTE_INT(x)   ((पूर्णांक)FOUR_BYTE_UINT(x))
#घोषणा SIX_BYTE_INT(x)    (FOUR_BYTE_UINT(x+2)+4294967296LL*TWO_BYTE_INT(x))

/*
** Function prototypes
*/
व्योम sqlite3VdbeError(Vdbe*, स्थिर अक्षर *, ...);
व्योम sqlite3VdbeFreeCursor(Vdbe *, VdbeCursor*);
व्योम sqlite3VdbeFreeCursorNN(Vdbe*,VdbeCursor*);
व्योम sqlite_Vdbe_निकालो_ढेर(Vdbe*,पूर्णांक);
पूर्णांक SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p);
पूर्णांक SQLITE_NOINLINE sqlite3VdbeFinishMoveto(VdbeCursor*);
पूर्णांक sqlite3VdbeCursorRestore(VdbeCursor*);
u32 sqlite3VdbeSerialTypeLen(u32);
u8 sqlite3VdbeOneByteSerialTypeLen(u8);
#अगर_घोषित SQLITE_MIXED_ENDIAN_64BIT_FLOAT
  u64 sqlite3FloatSwap(u64 in);
# define swapMixedEndianFloat(X)  X = sqlite3FloatSwap(X)
#अन्यथा
# define swapMixedEndianFloat(X)
#पूर्ण_अगर
व्योम sqlite3VdbeSerialGet(स्थिर अचिन्हित अक्षर*, u32, Mem*);
व्योम sqlite3_Vdbe_Delete_Aux_तथ्य(sqlite3*, Aux_तथ्य**, पूर्णांक, पूर्णांक);

पूर्णांक sqlite2_Btree_कुंजी_Compare(BtCursor *, स्थिर व्योम *, पूर्णांक, पूर्णांक, पूर्णांक *);
पूर्णांक sqlite3_Vdbe_Idx_कुंजी_Compare(sqlite3*,VdbeCursor*,UnpackedRecord*,पूर्णांक*);
पूर्णांक sqlite3VdbeIdxRowid(sqlite3*, BtCursor*, i64*);
पूर्णांक sqlite3VdbeExec(Vdbe*);
#अगर !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
पूर्णांक sqlite3_Vdbe_अग्र_Opcode(Vdbe*,Mem*,पूर्णांक,पूर्णांक*,पूर्णांक*,Op**);
अक्षर *sqlite3VdbeDisplayP4(sqlite3*,Op*);
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
अक्षर *sqlite3VdbeDisplayComment(sqlite3*,स्थिर Op*,स्थिर अक्षर*);
#पूर्ण_अगर
#अगर !defined(SQLITE_OMIT_EXPLAIN)
पूर्णांक sqlite3VdbeList(Vdbe*);
#पूर्ण_अगर
पूर्णांक sqlite3VdbeHalt(Vdbe*);
पूर्णांक sqlite3VdbeChangeEncoding(Mem *, पूर्णांक);
पूर्णांक sqlite3VdbeMemTooBig(Mem*);
पूर्णांक sqlite3VdbeMemCopy(Mem*, स्थिर Mem*);
व्योम sqlite3VdbeMemShallowCopy(Mem*, स्थिर Mem*, पूर्णांक);
व्योम sqlite3VdbeMemMove(Mem*, Mem*);
पूर्णांक sqlite3VdbeMemNulTerminate(Mem*);
पूर्णांक sqlite3VdbeMemSetStr(Mem*, स्थिर अक्षर*, i64, u8, व्योम(*)(व्योम*));
पूर्णांक sqlite3VdbeMemSetText(Mem*, स्थिर अक्षर*, i64, व्योम(*)(व्योम*));
व्योम sqlite3VdbeMemSetInt64(Mem*, i64);
#अगर_घोषित SQLITE_OMIT_FLOATING_POINT
# define sqlite3VdbeMemSetDouble sqlite3VdbeMemSetInt64
#अन्यथा
  व्योम sqlite3VdbeMemSetDouble(Mem*, द्विगुन);
#पूर्ण_अगर
व्योम sqlite3VdbeMemSetPointer(Mem*, व्योम*, स्थिर अक्षर*, व्योम(*)(व्योम*));
व्योम sqlite3VdbeMemInit(Mem*,sqlite3*,u16);
व्योम sqlite3VdbeMemSetNull(Mem*);
#अगर_अघोषित SQLITE_OMIT_INCRBLOB
व्योम sqlite3VdbeMemSetZeroBlob(Mem*,पूर्णांक);
#अन्यथा
पूर्णांक sqlite3VdbeMemSetZeroBlob(Mem*,पूर्णांक);
#पूर्ण_अगर
#अगर_घोषित SQLITE_DEBUG
पूर्णांक sqlite3VdbeMemIsRowSet(स्थिर Mem*);
#पूर्ण_अगर
पूर्णांक sqlite3VdbeMemSetRowSet(Mem*);
पूर्णांक sqlite3VdbeMemZeroTerminateIfAble(Mem*);
पूर्णांक sqlite3VdbeMemMakeWriteable(Mem*);
पूर्णांक sqlite3VdbeMemStringify(Mem*, u8, u8);
पूर्णांक sqlite3IntFloatCompare(i64,द्विगुन);
i64 sqlite3_Vdbe_Int_मूल्य(स्थिर Mem*);
पूर्णांक sqlite3VdbeMemIntegerify(Mem*);
द्विगुन sqlite3_Vdbe_Real_मूल्य(Mem*);
पूर्णांक sqlite3_Mem_Real_मूल्य_RC(Mem*, द्विगुन*);
पूर्णांक sqlite3_Vdbe_Boolean_मूल्य(Mem*, पूर्णांक ifNull);
व्योम sqlite3VdbeIntegerAffinity(Mem*,पूर्णांक bFromText);
पूर्णांक sqlite3VdbeMemRealify(Mem*);
पूर्णांक sqlite3VdbeMemNumerify(Mem*);
पूर्णांक sqlite3VdbeMemCast(Mem*,u8,u8);
पूर्णांक sqlite3VdbeMemFromBtree(BtCursor*,u32,u32,Mem*);
पूर्णांक sqlite3VdbeMemFromBtreeZeroOffset(BtCursor*,u32,Mem*);
व्योम sqlite3VdbeMemRelease(Mem *p);
व्योम sqlite3VdbeMemReleaseMalloc(Mem*p);
पूर्णांक sqlite3VdbeMemFinalize(Mem*, FuncDef*);
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
पूर्णांक sqlite3_Vdbe_Mem_Agg_मूल्य(Mem*, Mem*, FuncDef*);
#पूर्ण_अगर
#अगर !defined(SQLITE_OMIT_EXPLAIN) || defined(SQLITE_ENABLE_BYTECODE_VTAB)
स्थिर अक्षर *sqlite3OpcodeName(पूर्णांक);
#पूर्ण_अगर
पूर्णांक sqlite3VdbeMemGrow(Mem *pMem, पूर्णांक n, पूर्णांक preserve);
पूर्णांक sqlite3VdbeMemClearAndResize(Mem *pMem, पूर्णांक n);
पूर्णांक sqlite3VdbeCloseStatement(Vdbe *, पूर्णांक);
#अगर_घोषित SQLITE_DEBUG
पूर्णांक sqlite3VdbeFrameIsValid(VdbeFrame*);
#पूर्ण_अगर
व्योम sqlite3VdbeFrameMemDel(व्योम*);      /* Destructor on Mem */
व्योम sqlite3VdbeFrameDelete(VdbeFrame*); /* Actually deletes the Frame */
पूर्णांक sqlite3VdbeFrameRestore(VdbeFrame *);
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
व्योम sqlite3VdbePreUpdateHook(
    Vdbe*,VdbeCursor*,पूर्णांक,स्थिर अक्षर*,Table*,i64,पूर्णांक,पूर्णांक);
#पूर्ण_अगर
पूर्णांक sqlite3VdbeTransferError(Vdbe *p);
पूर्णांक sqlite3_Vdbe_Find_सूचकांक_कुंजी(BtCursor*, सूचकांक*, UnpackedRecord*, पूर्णांक*, पूर्णांक);

पूर्णांक sqlite3VdbeSorterInit(sqlite3 *, पूर्णांक, VdbeCursor *);
व्योम sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
व्योम sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
पूर्णांक sqlite3VdbeSorterRowkey(स्थिर VdbeCursor *, Mem *);
पूर्णांक sqlite3_Vdbe_Sorter_अग्र(sqlite3 *, स्थिर VdbeCursor *);
पूर्णांक sqlite3VdbeSorterRewind(स्थिर VdbeCursor *, पूर्णांक *);
पूर्णांक sqlite3VdbeSorterWrite(स्थिर VdbeCursor *, Mem *);
पूर्णांक sqlite3VdbeSorterCompare(स्थिर VdbeCursor *, Mem *, पूर्णांक, पूर्णांक *);

व्योम sqlite3_Vdbe_मूल्य_List_Free(व्योम*);

#अगर_घोषित SQLITE_DEBUG
  व्योम sqlite3_Vdbe_Incr_Write_गणक(Vdbe*, VdbeCursor*);
  व्योम sqlite3VdbeAssertAbortable(Vdbe*);
#अन्यथा
# define sqlite3_Vdbe_Incr_Write_गणक(V,C)
# define sqlite3VdbeAssertAbortable(V)
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_SHARED_CACHE)
  व्योम sqlite3VdbeEnter(Vdbe*);
#अन्यथा
# define sqlite3VdbeEnter(X)
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
  व्योम sqlite3VdbeLeave(Vdbe*);
#अन्यथा
# define sqlite3VdbeLeave(X)
#पूर्ण_अगर

#अगर_घोषित SQLITE_DEBUG
व्योम sqlite3VdbeMemAboutToChange(Vdbe*,Mem*);
पूर्णांक sqlite3VdbeCheckMemInvariants(Mem*);
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
पूर्णांक sqlite3VdbeCheckFkImmediate(Vdbe*);
पूर्णांक sqlite3VdbeCheckFkDeferred(Vdbe*);
#अन्यथा
# define sqlite3VdbeCheckFkImmediate(p) 0
# define sqlite3VdbeCheckFkDeferred(p) 0
#पूर्ण_अगर

#अगर_घोषित SQLITE_DEBUG
  व्योम sqlite3VdbePrintSql(Vdbe*);
  व्योम sqlite3VdbeMemPrettyPrint(Mem *pMem, StrAccum *pStr);
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_UTF16
  पूर्णांक sqlite3VdbeMemTranslate(Mem*, u8);
  पूर्णांक sqlite3VdbeMemHandleBom(Mem *pMem);
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_INCRBLOB
  पूर्णांक sqlite3VdbeMemExpandBlob(Mem *);
  #घोषणा ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)
#अन्यथा
  #घोषणा sqlite3VdbeMemExpandBlob(x) SQLITE_OK
  #घोषणा ExpandBlob(P) SQLITE_OK
#पूर्ण_अगर

#पूर्ण_अगर /* !defined(SQLITE_VDBEINT_H) */
