/*
** 2015-08-12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** SQLite JSON functions.
**
** This file began as an extension in ext/misc/json1.c in 2015.  That
** extension proved so useful that it has now been moved into the core.
**
** The original design stored all JSON as pure text, canonical RFC-8259.
** Support for JSON-5 extensions was added with version 3.42.0 (2023-05-16).
** All generated JSON text still conforms strictly to RFC-8259, but text
** with JSON-5 extensions is accepted as input.
**
** Beginning with version 3.45.0 (circa 2024-01-01), these routines also
** accept BLOB values that have JSON encoded using a binary representation
** called "JSONB".  The name JSONB comes from PostgreSQL, however the on-disk
** format for SQLite-JSONB is completely different and incompatible with
** PostgreSQL-JSONB.
**
** Decoding and interpreting JSONB is still O(N) where N is the size of
** the input, the same as text JSON.  However, the constant of proportionality
** for JSONB is much smaller due to faster parsing.  The size of each
** element in JSONB is encoded in its header, so there is no need to search
** for delimiters using persnickety syntax rules.  JSONB seems to be about
** 3x faster than text JSON as a result.  JSONB is also tends to be slightly
** smaller than text JSON, by 5% or 10%, but there are corner cases where
** JSONB can be slightly larger.  So you are not far mistaken to say that
** a JSONB blob is the same size as the equivalent RFC-8259 text.
**
**
** THE JSONB ENCODING:
**
** Every JSON element is encoded in JSONB as a header and a payload.
** The header is between 1 and 9 bytes in size.  The payload is zero
** or more bytes.
**
** The lower 4 bits of the first byte of the header determines the
** element type:
**
**    0:   NULL
**    1:   TRUE
**    2:   FALSE
**    3:   INT        -- RFC-8259 integer literal
**    4:   INT5       -- JSON5 integer literal
**    5:   FLOAT      -- RFC-8259 floating point literal
**    6:   FLOAT5     -- JSON5 floating point literal
**    7:   TEXT       -- Text literal acceptable to both SQL and JSON
**    8:   TEXTJ      -- Text containing RFC-8259 escapes
**    9:   TEXT5      -- Text containing JSON5 and/or RFC-8259 escapes
**   10:   TEXTRAW    -- Text containing unescaped syntax characters
**   11:   ARRAY
**   12:   OBJECT
**
** The other three possible values (13-15) are reserved for future
** enhancements.
**
** The upper 4 bits of the first byte determine the size of the header
** and sometimes also the size of the payload.  If X is the first byte
** of the element and if X>>4 is between 0 and 11, then the payload
** will be that many bytes in size and the header is exactly one byte
** in size.  Other four values for X>>4 (12-15) indicate that the header
** is more than one byte in size and that the payload size is determined
** by the remainder of the header, interpreted as a unsigned big-endian
** integer.
**
**   Value of X>>4         Size integer        Total header size
**   -------------     --------------------    -----------------
**        12           1 byte (0-255)                2
**        13           2 byte (0-65535)              3
**        14           4 byte (0-4294967295)         5
**        15           8 byte (0-1.8e19)             9
**
** The payload size need not be expressed in its minimal form.  For example,
** if the payload size is 10, the size can be expressed in any of 5 different
** ways: (1) (X>>4)==10, (2) (X>>4)==12 following by one 0x0a byte,
** (3) (X>>4)==13 followed by 0x00 and 0x0a, (4) (X>>4)==14 followed by
** 0x00 0x00 0x00 0x0a, or (5) (X>>4)==15 followed by 7 bytes of 0x00 and
** a single byte of 0x0a.  The shorter forms are preferred, of course, but
** sometimes when generating JSONB, the payload size is not known in advance
** and it is convenient to reserve sufficient header space to cover the
** largest possible payload size and then come back later and patch up
** the size when it becomes known, resulting in a non-minimal encoding.
**
** The value (X>>4)==15 is not actually used in the current implementation
** (as SQLite is currently unable to handle BLOBs larger than about 2GB)
** but is included in the design to allow for future enhancements.
**
** The payload follows the header.  NULL, TRUE, and FALSE have no payload and
** their payload size must always be zero.  The payload for INT, INT5,
** FLOAT, FLOAT5, TEXT, TEXTJ, TEXT5, and TEXTROW is text.  Note that the
** "..." or '...' delimiters are omitted from the various text encodings.
** The payload for ARRAY and OBJECT is a list of additional elements that
** are the content for the array or object.  The payload for an OBJECT
** must be an even number of elements.  The first element of each pair is
** the label and must be of type TEXT, TEXTJ, TEXT5, or TEXTRAW.
**
** A valid JSONB blob consists of a single element, as described above.
** Usually this will be an ARRAY or OBJECT element which has many more
** elements as its content.  But the overall blob is just a single element.
**
** Input validation for JSONB blobs simply checks that the element type
** code is between 0 and 12 and that the total size of the element
** (header plus payload) is the same as the size of the BLOB.  If those
** checks are true, the BLOB is assumed to be JSONB and processing continues.
** Errors are only raised if some other miscoding is discovered during
** processing.
**
** Additional information can be found in the doc/jsonb.md file of the
** canonical SQLite source tree.
*/
#ifndef SQLITE_OMIT_JSON
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/* JSONB element types
*/
#define JSONB_NULL     0   /* "null" */
#define JSONB_TRUE     1   /* "true" */
#define JSONB_FALSE    2   /* "false" */
#define JSONB_INT      3   /* integer acceptable to JSON and SQL */
#define JSONB_INT5     4   /* integer in 0x000 notation */
#define JSONB_FLOAT    5   /* float acceptable to JSON and SQL */
#define JSONB_FLOAT5   6   /* float with JSON5 extensions */
#define JSONB_TEXT     7   /* Text compatible with both JSON and SQL */
#define JSONB_TEXTJ    8   /* Text with JSON escapes */
#define JSONB_TEXT5    9   /* Text with JSON-5 escape */
#define JSONB_TEXTRAW 10   /* SQL text that needs escaping for JSON */
#define JSONB_ARRAY   11   /* An array */
#define JSONB_OBJECT  12   /* An object */

/* Human-readable names for the JSONB values.  The index for each
** string must correspond to the JSONB_* integer above.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ jsonIsSpace[] = {
#ifdef SQLITE_ASCII
/*0  1  2  3  4  5  6  7   8  9  a  b  c  d  e  f  */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 1, 0, 0, 1, 0, 0,  /* 0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 1 */
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 2 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 3 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 4 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 5 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 6 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 7 */

  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 8 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 9 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* a */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* b */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* c */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* d */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* e */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* f */
#endif
#ifdef SQLITE_EBCDIC
/*0  1  2  3  4  5  6  7   8  9  a  b  c  d  e  f  */
  0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0, 0, 1, 0, 0,  /* 0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 1 */
  0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 2 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 3 */
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 4 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 5 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 6 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 7 */

  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 8 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 9 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* a */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* b */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* c */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* d */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* e */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* f */
#endif

};
#define jsonIsspace(x) (jsonIsSpace[(unsigned ವರ್ಣ)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ jsonIsOk[256] = {
#ifdef SQLITE_ASCII
/*0  1  2  3  4  5  6  7   8  9  a  b  c  d  e  f  */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 1 */
  1, 1, 0, 1, 1, 1, 1, 0,  1, 1, 1, 1, 1, 1, 1, 1,  /* 2 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 3 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 4 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 0, 1, 1, 1,  /* 5 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 6 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 7 */

  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 8 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 9 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* a */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* b */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* c */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* d */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* e */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1   /* f */
#endif
#ifdef SQLITE_EBCDIC
/*0  1  2  3  4  5  6  7   8  9  a  b  c  d  e  f  */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 0 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 1 */
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  /* 2 */
  1, 1, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,  /* 3 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 4 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 5 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 6 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 0, 1, 0,  /* 7 */

  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 8 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* 9 */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* a */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* b */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* c */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* d */
  0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  /* e */
  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1   /* f */
#endif
};

/* Objects */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ JsonCache JsonCache;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ JsonString JsonString;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#define JSON_CACHE_ID    (-429938)  /* Cache entry */
#define JSON_CACHE_SIZE  4          /* Max number of cache entries */

/*
** jsonUnescapeOneChar() returns this invalid code point if it encounters
** a syntax error.
*/
#define JSON_INVALID_CHAR 0x99999

/* A cache mapping JSON text into JSONB blobs.
**
** Each cache entry is a JsonParse object with the following restrictions:
**
**    *   The bReadOnly flag must be set
**
**    *   The aBlob[] array must be owned by the JsonParse object.  In other
**        words, nBlobAlloc must be non-zero.
**
**    *   eEdit and delta must be zero.
**
**    *   zJson must be an RCStr.  In other words bJsonIsRCStr must be true.
*/
ಸಂರಚನಾ JsonCache {
  sqlite3 *db;                    /* Database connection */
  ಪೂರ್ಣಾಙ್ಕ nUsed;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_SIZE];  /* One line for each cache entry */
};

/* An instance of this object represents a JSON string
** under construction.  Really, this is a generic string accumulator
** that can be and is used to create strings other than JSON.
**
** If the generated string is longer than will fit into the zSpace[] buffer,
** then it will be an RCStr string.  This aids with caching of large
** JSON strings.
*/
ಸಂರಚನಾ JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  ವರ್ಣ *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  ವರ್ಣ zSpace[100];        /* Initial static space */
};

/* Allowed values for JsonString.eErr */
#define JSTRING_OOM         0x01   /* Out of memory */
#define JSTRING_MALFORMED   0x02   /* Malformed JSONB */
#define JSTRING_TOODEEP     0x04   /* JSON nested too deep */
#define JSTRING_ERR         0x08   /* Error already sent to sqlite3_result */

/* The "subtype" set for text JSON values passed through using
** sqlite3_result_subtype() and sqlite3_value_subtype().
*/
#define JSON_SUBTYPE  74    /* Ascii for "J" */

/*
** Bit values for the flags passed into various SQL function implementations
** via the sqlite3_user_data() value.
*/
#define JSON_JSON      0x01        /* Result is always JSON */
#define JSON_SQL       0x02        /* Result is always SQL */
#define JSON_ABPATH    0x03        /* Allow abbreviated JSON path specs */
#define JSON_ISSET     0x04        /* json_set(), not json_insert() */
#define JSON_AINS      0x08        /* json_array_insert(), not json_insert() */
#define JSON_BLOB      0x10        /* Use the BLOB output format */

#define JSON_ಸೇರಿಸು_TYPE(X)  (((X)&0xC)>>2)


/* A parsed JSON value.  Lifecycle:
**
**   1.  JSON comes in and is parsed into a JSONB value in aBlob.  The
**       original text is stored in zJson.  This step is skipped if the
**       input is JSONB instead of text JSON.
**
**   2.  The aBlob[] array is searched using the JSON path notation, if needed.
**       
**   3.  Zero or more changes are made to aBlob[] (via json_remove() or
**       json_replace() or json_patch() or similar).
**
**   4.  New JSON text is generated from the aBlob[] for output.  This step
**       is skipped if the function is one of the jsonb_* functions that
**       returns JSONB instead of text JSON.
*/
ಸಂರಚನಾ JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  ವರ್ಣ *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  ಪೂರ್ಣಾಙ್ಕ nJson;         /* Length of the zJson string in bytes */
  u32 nJPRef;        /* Number of references to this object */
  u32 iErr;          /* Error location in zJson[] */
  u16 iDepth;        /* Nesting depth */
  u8 nErr;           /* Number of errors seen */
  u8 oom;            /* Set to true if out of memory */
  u8 bJsonIsRCStr;   /* True if zJson is an RCStr */
  u8 hasNonstd;      /* True if input uses non-standard features like JSON5 */
  u8 bReadOnly;      /* Do not modify. */
  /* Search and edit information.  See jsonLookupStep() */
  u8 eEdit;          /* Edit operation to apply */
  ಪೂರ್ಣಾಙ್ಕ delta;         /* Size change due to the edit */
  u32 nIns;          /* Number of bytes to insert */
  u32 iLabel;        /* Location of label if search landed on an object value */
  u8 *aIns;          /* Content to be inserted */
};

/* Allowed values for JsonParse.eEdit */
#define JEDIT_DEL   1   /* Delete if exists */
#define JEDIT_REPL  2   /* Overwrite if exists */
#define JEDIT_INS   3   /* Insert if not exists */
#define JEDIT_SET   4   /* Insert or overwrite */
#define JEDIT_AINS  5   /* array_insert() */

/*
** Maximum nesting depth of JSON for this implementation.
**
** This limit is needed to avoid a stack overflow in the recursive
** descent parser.  A depth of 1000 is far deeper than any sane JSON
** should go.  Historical note: This limit was 2000 prior to version 3.42.0
*/
#ifndef SQLITE_JSON_MAX_DEPTH
# define JSON_MAX_DEPTH  1000
#ಅನ್ಯಥಾ
# define JSON_MAX_DEPTH SQLITE_JSON_MAX_DEPTH
#endif

/*
** Allowed values for the flgs argument to jsonParseFuncArg();
*/
#define JSON_EDITABLE  0x01   /* Generate a writable JsonParse object */
#define JSON_KEEPERROR 0x02   /* Return non-NULL even if there is an error */
#define JSON_NOCACHE   0x04   /* Do not use the JSON cache */

/**************************************************************************
** Forward references
**************************************************************************/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnStringAsBlob(JsonString*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonArgIsJsonb(sqlite3_ಮೌಲ್ಯ *pJson, JsonParse *p);
ಸ್ಥಿರ u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnParse(sqlite3_context*,JsonParse*);
ಸ್ಥಿರ JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_ಮೌಲ್ಯ*,u32);
ಸ್ಥಿರ ಶೂನ್ಯ jsonParseFree(JsonParse*);
ಸ್ಥಿರ u32 jsonbPayloadSize(ಧ್ರುವ JsonParse*, u32, u32*);
ಸ್ಥಿರ u32 jsonUnescapeOneChar(ಧ್ರುವ ವರ್ಣ*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonCacheDelete(JsonCache *p){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonCacheDeleteGeneric(ಶೂನ್ಯ *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ json_Cache_ಸೇರಿಸು(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  ಪ್ರತಿಜ್ಞಾ( pParse->zJson!=0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->bJsonIsRCStr );
  ಪ್ರತಿಜ್ಞಾ( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ಯದಿ( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(*p));
    ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }
  ಯದಿ( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*ಪರಿಮಾಣ(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  ಪ್ರತಿಜ್ಞಾ( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Search for a cached translation the json text supplied by pArg.  Return
** the JsonParse object if found.  Return NULL if not found.
**
** When a match if found, the matching entry is moved to become the
** most-recently used entry if it isn't so already.
**
** The JsonParse object returned still belongs to the Cache and might
** be deleted at any moment.  If the caller wants the JsonParse to
** linger, it needs to increment the nPJRef reference counter.
*/
ಸ್ಥಿರ JsonParse *json_Cache_ಹುಡುಕು(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_ಮೌಲ್ಯ *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಧ್ರುವ ವರ್ಣ *zJson;
  ಪೂರ್ಣಾಙ್ಕ nJson;

  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(pArg)!=SQLITE_TEXT ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  zJson = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pArg);
  ಯದಿ( zJson==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  nJson = sqlite3_ಮೌಲ್ಯ_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ಯದಿ( p==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿ(i=0; i<p->nUsed; i++){
    ಯದಿ( p->a[i]->zJson==zJson ) ಭಙ್ಗ;
  }
  ಯದಿ( i>=p->nUsed ){
    ಪ್ರತಿ(i=0; i<p->nUsed; i++){
      ಯದಿ( p->a[i]->nJson!=nJson ) ಅನುವರ್ತ;
      ಯದಿ( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) ಭಙ್ಗ;
    }
  }
  ಯದಿ( i<p->nUsed ){
    ಯದಿ( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*ಪರಿಮಾಣ(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    ಪ್ರತಿಜ್ಞಾ( p->a[i]->delta==0 );
    ಪ್ರತ್ಯಾಗಮ p->a[i];
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = ಪರಿಮಾಣ(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringReset(JsonString *p){
  ಯದಿ( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  ಯದಿ( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  ಪ್ರತಿಜ್ಞಾ( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  ವರ್ಣ *zNew;
  ಯದಿ( p->bStatic ){
    ಯದಿ( p->eErr ) ಪ್ರತ್ಯಾಗಮ 1;
    zNew = sqlite3RCStrNew(nTotal);
    ಯದಿ( zNew==0 ){
      jsonStringOom(p);
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }ಅನ್ಯಥಾ{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    ಯದಿ( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ jsonStringExpandAndAppend(
  JsonString *p,
  ಧ್ರುವ ವರ್ಣ *zIn,
  u32 N
){
  ಪ್ರತಿಜ್ಞಾ( N>0 );
  ಯದಿ( jsonStringGrow(p,N) ) ಪ್ರತ್ಯಾಗಮ;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendRaw(JsonString *p, ಧ್ರುವ ವರ್ಣ *zIn, u32 N){
  ಯದಿ( N==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }ಅನ್ಯಥಾ{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendRawNZ(JsonString *p, ಧ್ರುವ ವರ್ಣ *zIn, u32 N){
  ಪ್ರತಿಜ್ಞಾ( N>0 );
  ಯದಿ( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }ಅನ್ಯಥಾ{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonPrintf(ಪೂರ್ಣಾಙ್ಕ N, JsonString *p, ಧ್ರುವ ವರ್ಣ *zFormat, ...){
  va_list ap;
  ಯದಿ( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) ಪ್ರತ್ಯಾಗಮ;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (ಪೂರ್ಣಾಙ್ಕ)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ jsonAppendCharExpand(JsonString *p, ವರ್ಣ c){
  ಯದಿ( jsonStringGrow(p,1) ) ಪ್ರತ್ಯಾಗಮ;
  p->zBuf[p->nUsed++] = c;
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendChar(JsonString *p, ವರ್ಣ c){
  ಯದಿ( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }ಅನ್ಯಥಾ{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringTrimOneChar(JsonString *p){
  ಯದಿ( p->eErr==0 ){
    ಪ್ರತಿಜ್ಞಾ( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  ಪ್ರತ್ಯಾಗಮ p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendSeparator(JsonString *p){
  ವರ್ಣ c;
  ಯದಿ( p->nUsed==0 ) ಪ್ರತ್ಯಾಗಮ;
  c = p->zBuf[p->nUsed-1];
  ಯದಿ( c=='[' || c=='{' ) ಪ್ರತ್ಯಾಗಮ;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendControlChar(JsonString *p, u8 c){
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  ಪ್ರತಿಜ್ಞಾ( ಪರಿಮಾಣ(aSpecial)==32 );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\b']=='b' );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\f']=='f' );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\n']=='n' );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\r']=='r' );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\t']=='t' );
  ಪ್ರತಿಜ್ಞಾ( c>=0 && c<ಪರಿಮಾಣ(aSpecial) );
  ಪ್ರತಿಜ್ಞಾ( p->nUsed+7 <= p->nAlloc );
  ಯದಿ( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }ಅನ್ಯಥಾ{
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = 'u';
    p->zBuf[p->nUsed+2] = '0';
    p->zBuf[p->nUsed+3] = '0';
    p->zBuf[p->nUsed+4] = "0123456789abcdef"[c>>4];
    p->zBuf[p->nUsed+5] = "0123456789abcdef"[c&0xf];
    p->nUsed += 6;
  }
}

/* Append the N-byte string in zIn to the end of the JsonString string
** under construction.  Enclose the string in double-quotes ("...") and
** escape any double-quotes or backslash characters contained within the
** string.
**
** This routine is a high-runner.  There is a measurable performance
** increase associated with unwinding the jsonIsOk[] loop.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendString(JsonString *p, ಧ್ರುವ ವರ್ಣ *zIn, u32 N){
  u32 k;
  u8 c;
  ಧ್ರುವ u8 *z = (ಧ್ರುವ u8*)zIn;
  ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) ಪ್ರತ್ಯಾಗಮ;
  p->zBuf[p->nUsed++] = '"';
  ಯಾವತ್( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    ಯಾವತ್( 1 /* Exit by break */ ){
      ಯದಿ( k+3>=N ){
        ಯಾವತ್( k<N && jsonIsOk[z[k]] ){ k++; }
        ಭಙ್ಗ;
      }
      ಯದಿ( !jsonIsOk[z[k]] ){
        ಭಙ್ಗ;
      }
      ಯದಿ( !jsonIsOk[z[k+1]] ){
        k += 1;
        ಭಙ್ಗ;
      }
      ಯದಿ( !jsonIsOk[z[k+2]] ){
        k += 2;
        ಭಙ್ಗ;
      }
      ಯದಿ( !jsonIsOk[z[k+3]] ){
        k += 3;
        ಭಙ್ಗ;
      }ಅನ್ಯಥಾ{
        k += 4;
      }
    }
    ಯದಿ( k>=N ){
      ಯದಿ( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      ಭಙ್ಗ;
    }
    ಯದಿ( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    ಯದಿ( c=='"' || c=='\\' ){
      ಯದಿ( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) ಪ್ರತ್ಯಾಗಮ;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }ಅನ್ಯಥಾ ಯದಿ( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }ಅನ್ಯಥಾ{
      ಯದಿ( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) ಪ್ರತ್ಯಾಗಮ;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  ಪ್ರತಿಜ್ಞಾ( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Append_Sql_ಮೌಲ್ಯ(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ          /* Value to append */
){
  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_type(p_ಮೌಲ್ಯ) ){
    ಪಕ್ಷ SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_ಮೌಲ್ಯ_double(p_ಮೌಲ್ಯ));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_INTEGER: {
      ಧ್ರುವ ವರ್ಣ *z = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(p_ಮೌಲ್ಯ);
      u32 n = (u32)sqlite3_ಮೌಲ್ಯ_bytes(p_ಮೌಲ್ಯ);
      jsonAppendRaw(p, z, n);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_TEXT: {
      ಧ್ರುವ ವರ್ಣ *z = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(p_ಮೌಲ್ಯ);
      u32 n = (u32)sqlite3_ಮೌಲ್ಯ_bytes(p_ಮೌಲ್ಯ);
      ಯದಿ( sqlite3_ಮೌಲ್ಯ_subtype(p_ಮೌಲ್ಯ)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }ಅನ್ಯಥಾ{
        jsonAppendString(p, z, n);
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      JsonParse px;
      memset(&px, 0, ಪರಿಮಾಣ(px));
      ಯದಿ( jsonArgIsJsonb(p_ಮೌಲ್ಯ, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }ಅನ್ಯಥಾ ಯದಿ( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      ಭಙ್ಗ;
    }
  }
}

/* Make the text in p (which is probably a generated JSON text string)
** the result of the SQL function.
**
** The JsonString is reset.
**
** If pParse and ctx are both non-NULL, then the SQL string in p is
** loaded into the zJson field of the pParse object as a RCStr and the
** pParse is added to the cache.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  ಪ್ರತಿಜ್ಞಾ( (pParse!=0)==(ctx!=0) );
  ಪ್ರತಿಜ್ಞಾ( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  ಯದಿ( p->eErr==0 ){
    ಪೂರ್ಣಾಙ್ಕ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    ಯದಿ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }ಅನ್ಯಥಾ ಯದಿ( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }ಅನ್ಯಥಾ{
      ಯದಿ( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        ಪೂರ್ಣಾಙ್ಕ rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_ಸೇರಿಸು(ctx, pParse);
        ಯದಿ( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          ಪ್ರತ್ಯಾಗಮ;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }ಅನ್ಯಥಾ ಯದಿ( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }ಅನ್ಯಥಾ ಯದಿ( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }ಅನ್ಯಥಾ ಯದಿ( p->eErr & JSTRING_MALFORMED ){
    sqlite3_result_error(p->pCtx, "malformed JSON", -1);
  }
  jsonStringReset(p);
}

/**************************************************************************
** Utility routines for dealing with JsonParse objects
**************************************************************************/

/*
** Reclaim all memory allocated by a JsonParse object.  But do not
** delete the JsonParse object itself.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonParseReset(JsonParse *pParse){
  ಪ್ರತಿಜ್ಞಾ( pParse->nJPRef<=1 );
  ಯದಿ( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  ಯದಿ( pParse->nBlobAlloc ){
    sqlite3DbFree(pParse->db, pParse->aBlob);
    pParse->aBlob = 0;
    pParse->nBlob = 0;
    pParse->nBlobAlloc = 0;
  }
}

/*
** Decrement the reference count on the JsonParse object.  When the
** count reaches zero, free the object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonParseFree(JsonParse *pParse){
  ಯದಿ( pParse ){
    ಯದಿ( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }ಅನ್ಯಥಾ{
      jsonParseReset(pParse);
      sqlite3DbFree(pParse->db, pParse);
    }
  }
}

/**************************************************************************
** Utility routines for the JSON text parser
**************************************************************************/

/*
** Translate a single byte of Hex into an integer.
** This routine only gives a correct answer if h really is a valid hexadecimal
** character:  0..9a..fA..F.  But unlike sqlite3HexToInt(), it does not
** assert() if the digit is not hex.
*/
ಸ್ಥಿರ u8 jsonHexToInt(ಪೂರ್ಣಾಙ್ಕ h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  ಪ್ರತ್ಯಾಗಮ (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
ಸ್ಥಿರ u32 jsonHexToInt4(ಧ್ರುವ ವರ್ಣ *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  ಪ್ರತ್ಯಾಗಮ v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonIs2Hex(ಧ್ರುವ ವರ್ಣ *z){
  ಪ್ರತ್ಯಾಗಮ sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonIs4Hex(ಧ್ರುವ ವರ್ಣ *z){
  ಪ್ರತ್ಯಾಗಮ jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
}

/*
** Return the number of bytes of JSON5 whitespace at the beginning of
** the input string z[].
**
** JSON5 whitespace consists of any of the following characters:
**
**    Unicode  UTF-8         Name
**    U+0009   09            horizontal tab
**    U+000a   0a            line feed
**    U+000b   0b            vertical tab
**    U+000c   0c            form feed
**    U+000d   0d            carriage return
**    U+0020   20            space
**    U+00a0   c2 a0         non-breaking space
**    U+1680   e1 9a 80      ogham space mark
**    U+2000   e2 80 80      en quad
**    U+2001   e2 80 81      em quad
**    U+2002   e2 80 82      en space
**    U+2003   e2 80 83      em space
**    U+2004   e2 80 84      three-per-em space
**    U+2005   e2 80 85      four-per-em space
**    U+2006   e2 80 86      six-per-em space
**    U+2007   e2 80 87      figure space
**    U+2008   e2 80 88      punctuation space
**    U+2009   e2 80 89      thin space
**    U+200a   e2 80 8a      hair space
**    U+2028   e2 80 a8      line separator
**    U+2029   e2 80 a9      paragraph separator
**    U+202f   e2 80 af      narrow no-break space (NNBSP)
**    U+205f   e2 81 9f      medium mathematical space (MMSP)
**    U+3000   e3 80 80      ideographical space
**    U+FEFF   ef bb bf      byte order mark
**
** In addition, comments between '/', '*' and '*', '/' and
** from '/', '/' to end-of-line are also considered to be whitespace.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ json5Whitespace(ಧ್ರುವ ವರ್ಣ *zIn){
  ಪೂರ್ಣಾಙ್ಕ n = 0;
  ಧ್ರುವ u8 *z = (u8*)zIn;
  ಯಾವತ್( 1 /*exit by "goto whitespace_done"*/ ){
    ವಿಕಲ್ಪ( z[n] ){
      ಪಕ್ಷ 0x09:
      ಪಕ್ಷ 0x0a:
      ಪಕ್ಷ 0x0b:
      ಪಕ್ಷ 0x0c:
      ಪಕ್ಷ 0x0d:
      ಪಕ್ಷ 0x20: {
        n++;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ '/': {
        ಯದಿ( z[n+1]=='*' && z[n+2]!=0 ){
          ಪೂರ್ಣಾಙ್ಕ j;
          ಪ್ರತಿ(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            ಯದಿ( z[j]==0 ) ಗಚ್ಛ whitespace_done;
          }
          n = j+1;
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ ಯದಿ( z[n+1]=='/' ){
          ಪೂರ್ಣಾಙ್ಕ j;
          ವರ್ಣ c;
          ಪ್ರತಿ(j=n+2; (c = z[j])!=0; j++){
            ಯದಿ( c=='\n' || c=='\r' ) ಭಙ್ಗ;
            ಯದಿ( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              ಭಙ್ಗ;
            }
          }
          n = j;
          ಯದಿ( z[n] ) n++;
          ಭಙ್ಗ;
        }
        ಗಚ್ಛ whitespace_done;
      }
      ಪಕ್ಷ 0xc2: {
        ಯದಿ( z[n+1]==0xa0 ){
          n += 2;
          ಭಙ್ಗ;
        }
        ಗಚ್ಛ whitespace_done;
      }
      ಪಕ್ಷ 0xe1: {
        ಯದಿ( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          ಭಙ್ಗ;
        }
        ಗಚ್ಛ whitespace_done;
      }
      ಪಕ್ಷ 0xe2: {
        ಯದಿ( z[n+1]==0x80 ){
          u8 c = z[n+2];
          ಯದಿ( c<0x80 ) ಗಚ್ಛ whitespace_done;
          ಯದಿ( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            ಭಙ್ಗ;
          }
        }ಅನ್ಯಥಾ ಯದಿ( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          ಭಙ್ಗ;
        }
        ಗಚ್ಛ whitespace_done;
      }
      ಪಕ್ಷ 0xe3: {
        ಯದಿ( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          ಭಙ್ಗ;
        }
        ಗಚ್ಛ whitespace_done;
      }
      ಪಕ್ಷ 0xef: {
        ಯದಿ( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          ಭಙ್ಗ;
        }
        ಗಚ್ಛ whitespace_done;
      }
      ಸ್ವಭಾವ: {
        ಗಚ್ಛ whitespace_done;
      }
    }
  }
  whitespace_done:
  ಪ್ರತ್ಯಾಗಮ n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ NanInfName {
  ವರ್ಣ c1;
  ವರ್ಣ c2;
  ವರ್ಣ n;
  ವರ್ಣ eType;
  ವರ್ಣ nRepl;
  ವರ್ಣ *zMatch;
  ವರ್ಣ *zRepl;
} aNanInfName[] = {
  { 'i', 'I', 3, JSONB_FLOAT, 7, "inf", "9.0e999" },
  { 'i', 'I', 8, JSONB_FLOAT, 7, "infinity", "9.0e999" },
  { 'n', 'N', 3, JSONB_NULL, 4, "NaN", "null" },
  { 'q', 'Q', 4, JSONB_NULL, 4, "QNaN", "null" },
  { 's', 'S', 4, JSONB_NULL, 4, "SNaN", "null" },
};


/*
** Report the wrong number of arguments for json_insert(), json_replace()
** or json_set().
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Wrong_ಅಂಕೆ_Args(
  sqlite3_context *pCtx,
  ಧ್ರುವ ವರ್ಣ *zFuncName
){
  ವರ್ಣ *z_ಸಂದೇಶ = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, z_ಸಂದೇಶ, -1);
  sqlite3_free(z_ಸಂದೇಶ);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  ಪ್ರತಿಜ್ಞಾ( N>pParse->nBlobAlloc );
  ಯದಿ( pParse->nBlobAlloc==0 ){
    t = 100;
  }ಅನ್ಯಥಾ{
    t = pParse->nBlobAlloc*2;
  }
  ಯದಿ( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  ಯದಿ( aNew==0 ){ pParse->oom = 1; ಪ್ರತ್ಯಾಗಮ 1; }
  ಪ್ರತಿಜ್ಞಾ( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 nSize;
  ಪ್ರತಿಜ್ಞಾ( !pParse->bReadOnly );
  ಯದಿ( pParse->oom ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( pParse->nBlobAlloc>0 ) ಪ್ರತ್ಯಾಗಮ 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  ಯದಿ( jsonBlobExpand(pParse, nSize) ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  ಪ್ರತ್ಯಾಗಮ 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  ಯದಿ( pParse->oom==0 ){
    ಪ್ರತಿಜ್ಞಾ( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  ಯದಿ( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }ಅನ್ಯಥಾ{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Blob_Append_ಗಂಟು(JsonParse*,u8,u64,ಧ್ರುವ ಶೂನ್ಯ*);
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ json_Blob_Expand_And_Append_ಗಂಟು(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ಧ್ರುವ ಶೂನ್ಯ *aPayload
){
  ಯದಿ( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) ಪ್ರತ್ಯಾಗಮ;
  json_Blob_Append_ಗಂಟು(pParse, eType, szPayload, aPayload);
}


/* Append a node type byte together with the payload size and
** possibly also the payload.
**
** If aPayload is not NULL, then it is a pointer to the payload which
** is also appended.  If aPayload is NULL, the pParse->aBlob[] array
** is resized (if necessary) so that it is big enough to hold the
** payload, but the payload is not appended and pParse->nBlob is left
** pointing to where the first byte of payload will eventually be.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Blob_Append_ಗಂಟು(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  ಧ್ರುವ ಶೂನ್ಯ *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  ಯದಿ( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_ಗಂಟು(pParse,eType,szPayload,aPayload);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿಜ್ಞಾ( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  ಯದಿ( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }ಅನ್ಯಥಾ ಯದಿ( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }ಅನ್ಯಥಾ ಯದಿ( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }ಅನ್ಯಥಾ{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  ಯದಿ( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  ಪೂರ್ಣಾಙ್ಕ delta;
  ಯದಿ( pParse->oom ) ಪ್ರತ್ಯಾಗಮ 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  ಯದಿ( szType<=11 ){
    nExtra = 0;
  }ಅನ್ಯಥಾ ಯದಿ( szType==12 ){
    nExtra = 1;
  }ಅನ್ಯಥಾ ಯದಿ( szType==13 ){
    nExtra = 2;
  }ಅನ್ಯಥಾ ಯದಿ( szType==14 ){
    nExtra = 4;
  }ಅನ್ಯಥಾ{
    nExtra = 8;
  }
  ಯದಿ( szPayload<=11 ){
    nNeeded = 0;
  }ಅನ್ಯಥಾ ಯದಿ( szPayload<=0xff ){
    nNeeded = 1;
  }ಅನ್ಯಥಾ ಯದಿ( szPayload<=0xffff ){
    nNeeded = 2;
  }ಅನ್ಯಥಾ{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  ಯದಿ( delta ){
    u64 newSize = (u64)pParse->nBlob + delta;
    ಯದಿ( delta>0 ){
      ಯದಿ( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        ಪ್ರತ್ಯಾಗಮ 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }ಅನ್ಯಥಾ{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  ಯದಿ( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }ಅನ್ಯಥಾ ಯದಿ( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }ಅನ್ಯಥಾ ಯದಿ( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }ಅನ್ಯಥಾ{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  ಪ್ರತ್ಯಾಗಮ delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonIs4HexB(ಧ್ರುವ ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ *pOp){
  ಯದಿ( z[0]!='u' ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( !jsonIs4Hex(&z[1]) ) ಪ್ರತ್ಯಾಗಮ 0;
  *pOp = JSONB_TEXTJ;
  ಪ್ರತ್ಯಾಗಮ 1;
}

/*
** Check a single element of the JSONB in pParse for validity.
**
** The element to be checked starts at offset i and must end at on the
** last byte before iEnd.
**
** Return 0 if everything is correct.  Return the 1-based byte offset of the
** error if a problem is detected.  (In other words, if the error is at offset
** 0, return 1).
*/
ಸ್ಥಿರ u32 jsonbValidityCheck(
  ಧ್ರುವ JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ಧ್ರುವ u8 *z;
  u8 x;
  ಯದಿ( iDepth>JSON_MAX_DEPTH ) ಪ್ರತ್ಯಾಗಮ i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  ಯದಿ( NEVER(n==0) ) ಪ್ರತ್ಯಾಗಮ i+1;          /* Checked by caller */
  ಯದಿ( NEVER(i+n+sz!=iEnd) ) ಪ್ರತ್ಯಾಗಮ i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  ವಿಕಲ್ಪ( x ){
    ಪಕ್ಷ JSONB_NULL:
    ಪಕ್ಷ JSONB_TRUE:
    ಪಕ್ಷ JSONB_FALSE: {
      ಪ್ರತ್ಯಾಗಮ n+sz==1 ? 0 : i+1;
    }
    ಪಕ್ಷ JSONB_INT: {
      ಯದಿ( sz<1 ) ಪ್ರತ್ಯಾಗಮ i+1;
      j = i+n;
      ಯದಿ( z[j]=='-' ){
        j++;
        ಯದಿ( sz<2 ) ಪ್ರತ್ಯಾಗಮ i+1;
      }
      k = i+n+sz;
      ಯಾವತ್( j<k ){
        ಯದಿ( sqlite3Isdigit(z[j]) ){
          j++;
        }ಅನ್ಯಥಾ{
          ಪ್ರತ್ಯಾಗಮ j+1;
        }
      }
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪಕ್ಷ JSONB_INT5: {
      ಯದಿ( sz<3 ) ಪ್ರತ್ಯಾಗಮ i+1;
      j = i+n;
      ಯದಿ( z[j]=='-' ){
        ಯದಿ( sz<4 ) ಪ್ರತ್ಯಾಗಮ i+1;
        j++;
      }
      ಯದಿ( z[j]!='0' ) ಪ್ರತ್ಯಾಗಮ i+1;
      ಯದಿ( z[j+1]!='x' && z[j+1]!='X' ) ಪ್ರತ್ಯಾಗಮ j+2;
      j += 2;
      k = i+n+sz;
      ಯಾವತ್( j<k ){
        ಯದಿ( sqlite3Isxdigit(z[j]) ){
          j++;
        }ಅನ್ಯಥಾ{
          ಪ್ರತ್ಯಾಗಮ j+1;
        }
      }
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪಕ್ಷ JSONB_FLOAT:
    ಪಕ್ಷ JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      ಯದಿ( sz<2 ) ಪ್ರತ್ಯಾಗಮ i+1;
      j = i+n;
      k = j+sz;
      ಯದಿ( z[j]=='-' ){
        j++;
        ಯದಿ( sz<3 ) ಪ್ರತ್ಯಾಗಮ i+1;
      }
      ಯದಿ( z[j]=='.' ){
        ಯದಿ( x==JSONB_FLOAT ) ಪ್ರತ್ಯಾಗಮ j+1;
        ಯದಿ( !sqlite3Isdigit(z[j+1]) ) ಪ್ರತ್ಯಾಗಮ j+1;
        j += 2;
        seen = 1;
      }ಅನ್ಯಥಾ ಯದಿ( z[j]=='0' && x==JSONB_FLOAT ){
        ಯದಿ( j+3>k ) ಪ್ರತ್ಯಾಗಮ j+1;
        ಯದಿ( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) ಪ್ರತ್ಯಾಗಮ j+1;
        j++;
      }
      ಪ್ರತಿ(; j<k; j++){
        ಯದಿ( sqlite3Isdigit(z[j]) ) ಅನುವರ್ತ;
        ಯದಿ( z[j]=='.' ){
          ಯದಿ( seen>0 ) ಪ್ರತ್ಯಾಗಮ j+1;
          ಯದಿ( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            ಪ್ರತ್ಯಾಗಮ j+1;
          }
          seen = 1;
          ಅನುವರ್ತ;
        }
        ಯದಿ( z[j]=='e' || z[j]=='E' ){
          ಯದಿ( seen==2 ) ಪ್ರತ್ಯಾಗಮ j+1;
          ಯದಿ( j==k-1 ) ಪ್ರತ್ಯಾಗಮ j+1;
          ಯದಿ( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            ಯದಿ( j==k-1 ) ಪ್ರತ್ಯಾಗಮ j+1;
          }
          seen = 2;
          ಅನುವರ್ತ;
        }
        ಪ್ರತ್ಯಾಗಮ j+1;
      }
      ಯದಿ( seen==0 ) ಪ್ರತ್ಯಾಗಮ i+1;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪಕ್ಷ JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      ಯಾವತ್( j<k ){
        ಯದಿ( !jsonIsOk[z[j]] && z[j]!='\'' ) ಪ್ರತ್ಯಾಗಮ j+1;
        j++;
      }
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪಕ್ಷ JSONB_TEXTJ:
    ಪಕ್ಷ JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      ಯಾವತ್( j<k ){
        ಯದಿ( !jsonIsOk[z[j]] && z[j]!='\'' ){
          ಯದಿ( z[j]=='"' ){
            ಯದಿ( x==JSONB_TEXTJ ) ಪ್ರತ್ಯಾಗಮ j+1;
          }ಅನ್ಯಥಾ ಯದಿ( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            ಯದಿ( x==JSONB_TEXTJ ) ಪ್ರತ್ಯಾಗಮ j+1;
          }ಅನ್ಯಥಾ ಯದಿ( NEVER(z[j]!='\\') || j+1>=k ){
            ಪ್ರತ್ಯಾಗಮ j+1;
          }ಅನ್ಯಥಾ ಯದಿ( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }ಅನ್ಯಥಾ ಯದಿ( z[j+1]=='u' ){
            ಯದಿ( j+5>=k ) ಪ್ರತ್ಯಾಗಮ j+1;
            ಯದಿ( !jsonIs4Hex((ಧ್ರುವ ವರ್ಣ*)&z[j+2]) ) ಪ್ರತ್ಯಾಗಮ j+1;
            j++;
          }ಅನ್ಯಥಾ ಯದಿ( x!=JSONB_TEXT5 ){
            ಪ್ರತ್ಯಾಗಮ j+1;
          }ಅನ್ಯಥಾ{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((ಧ್ರುವ ವರ್ಣ*)&z[j], k-j, &c);
            ಯದಿ( c==JSON_INVALID_CHAR ) ಪ್ರತ್ಯಾಗಮ j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪಕ್ಷ JSONB_TEXTRAW: {
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪಕ್ಷ JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      ಯಾವತ್( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        ಯದಿ( n==0 ) ಪ್ರತ್ಯಾಗಮ j+1;
        ಯದಿ( j+n+sz>k ) ಪ್ರತ್ಯಾಗಮ j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        ಯದಿ( sub ) ಪ್ರತ್ಯಾಗಮ sub;
        j += n + sz;
      }
      ಪ್ರತಿಜ್ಞಾ( j==k );
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪಕ್ಷ JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      ಯಾವತ್( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        ಯದಿ( n==0 ) ಪ್ರತ್ಯಾಗಮ j+1;
        ಯದಿ( j+n+sz>k ) ಪ್ರತ್ಯಾಗಮ j+1;
        ಯದಿ( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          ಯದಿ( x<JSONB_TEXT || x>JSONB_TEXTRAW ) ಪ್ರತ್ಯಾಗಮ j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        ಯದಿ( sub ) ಪ್ರತ್ಯಾಗಮ sub;
        cnt++;
        j += n + sz;
      }
      ಪ್ರತಿಜ್ಞಾ( j==k );
      ಯದಿ( (cnt & 1)!=0 ) ಪ್ರತ್ಯಾಗಮ j+1;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಸ್ವಭಾವ: {
      ಪ್ರತ್ಯಾಗಮ i+1;
    }
  }
}

/*
** Translate a single element of JSON text at pParse->zJson[i] into
** its equivalent binary JSONB representation.  Append the translation into
** pParse->aBlob[] beginning at pParse->nBlob.  The size of
** pParse->aBlob[] is increased as necessary.
**
** Return the index of the first character past the end of the element parsed,
** or one of the following special result codes:
**
**      0    End of input
**     -1    Syntax error or OOM
**     -2    '}' seen   \
**     -3    ']' seen    \___  For these returns, pParse->iErr is set to
**     -4    ',' seen    /     the index in zJson[] of the seen character
**     -5    ':' seen   /
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  ವರ್ಣ c;
  u32 j;
  u32 iThis, iStart;
  ಪೂರ್ಣಾಙ್ಕ x;
  u8 t;
  ಧ್ರುವ ವರ್ಣ *z = pParse->zJson;
json_parse_restart:
  ವಿಕಲ್ಪ( (u8)z[i] ){
  ಪಕ್ಷ '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_ಗಂಟು(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    ಯದಿ( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      ಪ್ರತ್ಯಾಗಮ -1;
    }
    iStart = pParse->nBlob;
    ಪ್ರತಿ(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      ಯದಿ( x<=0 ){
        ಪೂರ್ಣಾಙ್ಕ op;
        ಯದಿ( x==(-2) ){
          j = pParse->iErr;
          ಯದಿ( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          ಭಙ್ಗ;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        ಯದಿ( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          ಪೂರ್ಣಾಙ್ಕ k = j+1;
          ಯಾವತ್( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          ಪ್ರತಿಜ್ಞಾ( iBlob==pParse->nBlob );
          json_Blob_Append_ಗಂಟು(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }ಅನ್ಯಥಾ{
          ಯದಿ( x!=-1 ) pParse->iErr = j;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
      }
      ಯದಿ( pParse->oom ) ಪ್ರತ್ಯಾಗಮ -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      ಯದಿ( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        ಪ್ರತ್ಯಾಗಮ -1;
      }
      j = x;
      ಯದಿ( z[j]==':' ){
        j++;
      }ಅನ್ಯಥಾ{
        ಯದಿ( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          ಕುರು{ j++; }ಯಾವತ್( jsonIsspace(z[j]) );
          ಯದಿ( z[j]==':' ){
            j++;
            ಗಚ್ಛ parse_object_ಮೌಲ್ಯ;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ಯದಿ( x!=(-5) ){
          ಯದಿ( x!=(-1) ) pParse->iErr = j;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_ಮೌಲ್ಯ:
      x = jsonTranslateTextToBlob(pParse, j);
      ಯದಿ( x<=0 ){
        ಯದಿ( x!=(-1) ) pParse->iErr = j;
        ಪ್ರತ್ಯಾಗಮ -1;
      }
      j = x;
      ಯದಿ( z[j]==',' ){
        ಅನುವರ್ತ;
      }ಅನ್ಯಥಾ ಯದಿ( z[j]=='}' ){
        ಭಙ್ಗ;
      }ಅನ್ಯಥಾ{
        ಯದಿ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ಯದಿ( z[j]==',' ){
            ಅನುವರ್ತ;
          }ಅನ್ಯಥಾ ಯದಿ( z[j]=='}' ){
            ಭಙ್ಗ;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ಯದಿ( x==(-4) ){
          j = pParse->iErr;
          ಅನುವರ್ತ;
        }
        ಯದಿ( x==(-2) ){
          j = pParse->iErr;
          ಭಙ್ಗ;
        }
      }
      pParse->iErr = j;
      ಪ್ರತ್ಯಾಗಮ -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    ಪ್ರತ್ಯಾಗಮ j+1;
  }
  ಪಕ್ಷ '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    ಪ್ರತಿಜ್ಞಾ( i<=(u32)pParse->nJson );
    json_Blob_Append_ಗಂಟು(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    ಯದಿ( pParse->oom ) ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತಿ(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      ಯದಿ( x<=0 ){
        ಯದಿ( x==(-3) ){
          j = pParse->iErr;
          ಯದಿ( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          ಭಙ್ಗ;
        }
        ಯದಿ( x!=(-1) ) pParse->iErr = j;
        ಪ್ರತ್ಯಾಗಮ -1;
      }
      j = x;
      ಯದಿ( z[j]==',' ){
        ಅನುವರ್ತ;
      }ಅನ್ಯಥಾ ಯದಿ( z[j]==']' ){
        ಭಙ್ಗ;
      }ಅನ್ಯಥಾ{
        ಯದಿ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ಯದಿ( z[j]==',' ){
            ಅನುವರ್ತ;
          }ಅನ್ಯಥಾ ಯದಿ( z[j]==']' ){
            ಭಙ್ಗ;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ಯದಿ( x==(-4) ){
          j = pParse->iErr;
          ಅನುವರ್ತ;
        }
        ಯದಿ( x==(-3) ){
          j = pParse->iErr;
          ಭಙ್ಗ;
        }
      }
      pParse->iErr = j;
      ಪ್ರತ್ಯಾಗಮ -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    ಪ್ರತ್ಯಾಗಮ j+1;
  }
  ಪಕ್ಷ '\'': {
    u8 opcode;
    ವರ್ಣ cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    ಗಚ್ಛ parse_string;
  ಪಕ್ಷ '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    ಯಾವತ್( 1 /*exit-by-break*/ ){
      ಯದಿ( jsonIsOk[(u8)z[j]] ){
        ಯದಿ( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }ಅನ್ಯಥಾ ಯದಿ( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }ಅನ್ಯಥಾ{
          j += 3;
          ಅನುವರ್ತ;
        }
      }
      c = z[j];
      ಯದಿ( c==cDelim ){
        ಭಙ್ಗ;
      }ಅನ್ಯಥಾ ಯದಿ( c=='\\' ){
        c = z[++j];
        ಯದಿ( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          ಯದಿ( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }ಅನ್ಯಥಾ ಯದಿ( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#ಅನ್ಯಥಾ
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ಅನ್ಯಥಾ ಯದಿ( c=='\r' ){
          ಯದಿ( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ಅನ್ಯಥಾ{
          pParse->iErr = j;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
      }ಅನ್ಯಥಾ ಯದಿ( c<=0x1f ){
        ಯದಿ( c==0 ){
          pParse->iErr = j;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }ಅನ್ಯಥಾ ಯದಿ( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_ಗಂಟು(pParse, opcode, j-1-i, &z[i+1]);
    ಪ್ರತ್ಯಾಗಮ j+1;
  }
  ಪಕ್ಷ 't': {
    ಯದಿ( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      ಪ್ರತ್ಯಾಗಮ i+4;
    }
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪಕ್ಷ 'f': {
    ಯದಿ( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      ಪ್ರತ್ಯಾಗಮ i+5;
    }
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪಕ್ಷ '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    ಗಚ್ಛ parse_ಸಂಖ್ಯೆ;
  ಪಕ್ಷ '.':
    ಯದಿ( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      ಗಚ್ಛ parse_ಸಂಖ್ಯೆ_2;
    }
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -1;
  ಪಕ್ಷ '-':
  ಪಕ್ಷ '0':
  ಪಕ್ಷ '1':
  ಪಕ್ಷ '2':
  ಪಕ್ಷ '3':
  ಪಕ್ಷ '4':
  ಪಕ್ಷ '5':
  ಪಕ್ಷ '6':
  ಪಕ್ಷ '7':
  ಪಕ್ಷ '8':
  ಪಕ್ಷ '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_ಸಂಖ್ಯೆ:
    seenE = 0;
    ಪ್ರತಿಜ್ಞಾ( '-' < '0' );
    ಪ್ರತಿಜ್ಞಾ( '+' < '0' );
    ಪ್ರತಿಜ್ಞಾ( '.' < '0' );
    c = z[i];

    ಯದಿ( c<='0' ){
      ಯದಿ( c=='0' ){
        ಯದಿ( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          ಪ್ರತಿಜ್ಞಾ( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          ಪ್ರತಿ(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          ಗಚ್ಛ parse_ಸಂಖ್ಯೆ_finish;
        }ಅನ್ಯಥಾ ಯದಿ( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
      }ಅನ್ಯಥಾ{
        ಯದಿ( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          ಯದಿ( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            ಯದಿ( z[i]=='-' ){
              json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 6, "-9e999");
            }ಅನ್ಯಥಾ{
              json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 5, "9e999");
            }
            ಪ್ರತ್ಯಾಗಮ i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          ಯದಿ( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            ಗಚ್ಛ parse_ಸಂಖ್ಯೆ_2;
          }
          pParse->iErr = i;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
        ಯದಿ( z[i+1]=='0' ){
          ಯದಿ( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            ಪ್ರತ್ಯಾಗಮ -1;
          }ಅನ್ಯಥಾ ಯದಿ( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            ಪ್ರತಿ(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            ಗಚ್ಛ parse_ಸಂಖ್ಯೆ_finish;
          }
        }
      }
    }

  parse_ಸಂಖ್ಯೆ_2:
    ಪ್ರತಿ(j=i+1;; j++){
      c = z[j];
      ಯದಿ( sqlite3Isdigit(c) ) ಅನುವರ್ತ;
      ಯದಿ( c=='.' ){
        ಯದಿ( (t & 0x02)!=0 ){
          pParse->iErr = j;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
        t |= 0x02;
        ಅನುವರ್ತ;
      }
      ಯದಿ( c=='e' || c=='E' ){
        ಯದಿ( z[j-1]<'0' ){
          ಯದಿ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }ಅನ್ಯಥಾ{
            pParse->iErr = j;
            ಪ್ರತ್ಯಾಗಮ -1;
          }
        }
        ಯದಿ( seenE ){
          pParse->iErr = j;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        ಯದಿ( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        ಯದಿ( c<'0' || c>'9' ){
          pParse->iErr = j;
          ಪ್ರತ್ಯಾಗಮ -1;
        }
        ಅನುವರ್ತ;
      }
      ಭಙ್ಗ;
    }
    ಯದಿ( z[j-1]<'0' ){
      ಯದಿ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }ಅನ್ಯಥಾ{
        pParse->iErr = j;
        ಪ್ರತ್ಯಾಗಮ -1;
      }
    }
  parse_ಸಂಖ್ಯೆ_finish:
    ಪ್ರತಿಜ್ಞಾ( JSONB_INT+0x01==JSONB_INT5 );
    ಪ್ರತಿಜ್ಞಾ( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    ಪ್ರತಿಜ್ಞಾ( JSONB_INT+0x02==JSONB_FLOAT );
    ಯದಿ( z[i]=='+' ) i++;
    json_Blob_Append_ಗಂಟು(pParse, JSONB_INT+t, j-i, &z[i]);
    ಪ್ರತ್ಯಾಗಮ j;
  }
  ಪಕ್ಷ '}': {
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -2;  /* End of {...} */
  }
  ಪಕ್ಷ ']': {
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -3;  /* End of [...] */
  }
  ಪಕ್ಷ ',': {
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -4;  /* List separator */
  }
  ಪಕ್ಷ ':': {
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -5;  /* Object label/value separator */
  }
  ಪಕ್ಷ 0: {
    ಪ್ರತ್ಯಾಗಮ 0;   /* End of file */
  }
  ಪಕ್ಷ 0x09:
  ಪಕ್ಷ 0x0a:
  ಪಕ್ಷ 0x0d:
  ಪಕ್ಷ 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    ಗಚ್ಛ json_parse_restart;
  }
  ಪಕ್ಷ 0x0b:
  ಪಕ್ಷ 0x0c:
  ಪಕ್ಷ '/':
  ಪಕ್ಷ 0xc2:
  ಪಕ್ಷ 0xe1:
  ಪಕ್ಷ 0xe2:
  ಪಕ್ಷ 0xe3:
  ಪಕ್ಷ 0xef: {
    j = json5Whitespace(&z[i]);
    ಯದಿ( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      ಗಚ್ಛ json_parse_restart;
    }
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪಕ್ಷ 'n': {
    ಯದಿ( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      ಪ್ರತ್ಯಾಗಮ i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  ಸ್ವಭಾವ: {
    u32 k;
    ಪೂರ್ಣಾಙ್ಕ nn;
    c = z[i];
    ಪ್ರತಿ(k=0; k<ಪರಿಮಾಣ(aNanInfName)/ಪರಿಮಾಣ(aNanInfName[0]); k++){
      ಯದಿ( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) ಅನುವರ್ತ;
      nn = aNanInfName[k].n;
      ಯದಿ( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        ಅನುವರ್ತ;
      }
      ಯದಿ( sqlite3Isalnum(z[i+nn]) ) ಅನುವರ್ತ;
      ಯದಿ( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 5, "9e999");
      }ಅನ್ಯಥಾ{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      ಪ್ರತ್ಯಾಗಮ i + nn;
    }
    pParse->iErr = i;
    ಪ್ರತ್ಯಾಗಮ -1;  /* Syntax error */
  }
  } /* End switch(z[i]) */
}


/*
** Parse a complete JSON string.  Return 0 on success or non-zero if there
** are any errors.  If an error occurs, free all memory held by pParse,
** but not pParse itself.
**
** pParse must be initialized to an empty parse object prior to calling
** this routine.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಧ್ರುವ ವರ್ಣ *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  ಯದಿ( pParse->oom ) i = -1;
  ಯದಿ( i>0 ){
#ifdef SQLITE_DEBUG
    ಪ್ರತಿಜ್ಞಾ( pParse->iDepth==0 );
    ಯದಿ( sqlite3Config.bJsonSelfcheck ){
      ಪ್ರತಿಜ್ಞಾ( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    ಯಾವತ್( jsonIsspace(zJson[i]) ) i++;
    ಯದಿ( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      ಯದಿ( zJson[i] ){
        ಯದಿ( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  ಯದಿ( i<=0 ){
    ಯದಿ( pCtx!=0 ){
      ಯದಿ( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }ಅನ್ಯಥಾ{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  ಪ್ರತಿಜ್ಞಾ( pStr->eErr==0 );
  memset(&px, 0, ಪರಿಮಾಣ(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (ಶೂನ್ಯ)jsonTranslateTextToBlob(&px, 0);
  ಯದಿ( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( px.nBlobAlloc>0 );
    ಪ್ರತಿಜ್ಞಾ( !px.bReadOnly );
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
ಸ್ಥಿರ u32 jsonbPayloadSizeWide(
  ಧ್ರುವ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  ಪ್ರತಿಜ್ಞಾ( i<pParse->nBlob );
  ಪ್ರತಿಜ್ಞಾ( x==(pParse->aBlob[i]>>4) );
  ಪ್ರತಿಜ್ಞಾ( x>=13 );
  ಯದಿ( x==13 ){
    ಯದಿ( i+2>=pParse->nBlob ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ಅನ್ಯಥಾ ಯದಿ( x==14 ){
    ಯದಿ( i+4>=pParse->nBlob ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ಅನ್ಯಥಾ{
    ಯದಿ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  testcase( (i64)i+sz+n > pParse->nBlob );
  testcase( (i64)i+sz+n > pParse->nBlob-pParse->delta
         && (i64)i+sz+n > pParse->nBlob );
  testcase( pParse->delta>0 );
  testcase( pParse->delta<0 );
  /* Quirks with -Os and gcov cause the NO_TEST line below to show up as
  ** false-positive coverage miss.  The testcases() macros above are
  ** sufficient to prove that the branch is in fact covered */
  ಯದಿ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  *pSz = sz;
  ಪ್ರತ್ಯಾಗಮ n;
}

/*
** This is the main routine for determining the size of a node in JSONB.
** The jsonbPayloadSizeWide() above is a helper.  The two routines
** jsonbPayloadSizeInline() and jsonbPayloadSizeSemiInline() below are
** optional optimizations.  It is important to keep all these routines in
** sync.  Agents reading this code:  Help us humans to remember that!
**
** The byte at index i is a node type-code.  This routine
** determines the payload size for that node and writes that
** payload size in to *pSz.  It returns the offset from i to the
** beginning of the payload.  Return 0 on error.
*/
ಸ್ಥಿರ u32 jsonbPayloadSize(
  ಧ್ರುವ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ಯದಿ( i>=pParse->nBlob ){
    *pSz = 0;
    ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ ಯದಿ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ಅನ್ಯಥಾ ಯದಿ( x==12 ){
    ಯದಿ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ jsonbPayloadSizeWide(pParse, i, pSz, x);
  }
  ಯದಿ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  *pSz = sz;
  ಪ್ರತ್ಯಾಗಮ n;
}

#ಯದಿ SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
ಸ್ಥಿರ SQLITE_INLINE u32 jsonbPayloadSizeInline(
  ಧ್ರುವ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ಯದಿ( i>=pParse->nBlob ){
    *pSz = 0;
    ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ ಯದಿ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ಅನ್ಯಥಾ ಯದಿ( x==12 ){
    ಯದಿ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ಅನ್ಯಥಾ ಯದಿ( x==13 ){
    ಯದಿ( i+2>=pParse->nBlob ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ಅನ್ಯಥಾ ಯದಿ( x==14 ){
    ಯದಿ( i+4>=pParse->nBlob ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ಅನ್ಯಥಾ{
    ಯದಿ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  ಯದಿ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  *pSz = sz;
  ಪ್ರತ್ಯಾಗಮ n;
}
#ಅನ್ಯಥಾ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif


#ಯದಿ SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize() that implements the more
** common paths inline but then calls out to a subroutine for the uncommon
** paths.
**
**    *   It is guaranteed that the i parameter is a valid index for
**        pParse->aBlob[].  There is no possibility of running of the end
**        of the allocation.
**
**    *   Large sizes (greater than 255) are uncommon and are handled
**        by a subroutine call to the jsonbPayloadSizeWide().
*/
ಸ್ಥಿರ SQLITE_INLINE u32 jsonbPayloadSizeSemiInline(
  ಧ್ರುವ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ಪ್ರತಿಜ್ಞಾ( i<pParse->nBlob );
  ಯದಿ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ಅನ್ಯಥಾ ಯದಿ( x==12 ){
    ಯದಿ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ jsonbPayloadSizeWide(pParse,i,pSz,x);
  }
  ಯದಿ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  *pSz = sz;
  ಪ್ರತ್ಯಾಗಮ n;
}
#ಅನ್ಯಥಾ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeSemiInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif

/*
** Translate the binary JSONB representation of JSON beginning at
** pParse->aBlob[i] into a JSON text string.  Append the JSON
** text onto the end of pOut.  Return the index in pParse->aBlob[]
** of the first byte past the end of the element that is translated.
**
** If an error is detected in the BLOB input, the pOut->eErr flag
** might get set to JSTRING_MALFORMED.  But not all BLOB input errors
** are detected.  So a malformed JSONB input might either result
** in an error, or in incorrect JSON.
**
** The pOut->eErr JSTRING_OOM flag is set on a OOM.
*/
ಸ್ಥಿರ u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSizeInline(pParse, i, &sz);
  ಯದಿ( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    ಪ್ರತ್ಯಾಗಮ pParse->nBlob+1;
  }
  ವಿಕಲ್ಪ( pParse->aBlob[i] & 0x0f ){
    ಪಕ್ಷ JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      ಪ್ರತ್ಯಾಗಮ i+1;
    }
    ಪಕ್ಷ JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      ಪ್ರತ್ಯಾಗಮ i+1;
    }
    ಪಕ್ಷ JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      ಪ್ರತ್ಯಾಗಮ i+1;
    }
    ಪಕ್ಷ JSONB_INT:
    ಪಕ್ಷ JSONB_FLOAT: {
      ಯದಿ( sz==0 ) ಗಚ್ಛ malformed_jsonb;
      jsonAppendRaw(pOut, (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n], sz);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ಧ್ರುವ ವರ್ಣ *zIn = (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n];
      ಪೂರ್ಣಾಙ್ಕ bOverflow = 0;
      ಯದಿ( sz==0 ) ಗಚ್ಛ malformed_jsonb;
      ಯದಿ( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }ಅನ್ಯಥಾ ಯದಿ( zIn[0]=='+' ){
        k++;
      }
      ಪ್ರತಿ(; k<sz; k++){
        ಯದಿ( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ ಯದಿ( (u>>60)!=0 ){
          bOverflow = 1;
        }ಅನ್ಯಥಾ{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ಧ್ರುವ ವರ್ಣ *zIn = (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n];
      ಯದಿ( sz==0 ) ಗಚ್ಛ malformed_jsonb;
      ಯದಿ( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        ಯದಿ( sz<=1 ) ಗಚ್ಛ malformed_jsonb;
        k = 1;
      }
      ಯದಿ( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      ಪ್ರತಿ(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        ಯದಿ( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_TEXT:
    ಪಕ್ಷ JSONB_TEXTJ: {
      ಯದಿ( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_TEXT5: {
      ಧ್ರುವ ವರ್ಣ *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      ಯಾವತ್( sz2>0 ){
        ಪ್ರತಿ(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        ಯದಿ( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          ಯದಿ( k>=sz2 ){
            ಭಙ್ಗ;
          }
          zIn += k;
          sz2 -= k;
        }
        ಯದಿ( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          ಅನುವರ್ತ;
        }
        ಯದಿ( zIn[0]<=0x1f ){
          ಯದಿ( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) ಭಙ್ಗ;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          ಅನುವರ್ತ;
        }
        ಪ್ರತಿಜ್ಞಾ( zIn[0]=='\\' );
        ಪ್ರತಿಜ್ಞಾ( sz2>=1 );
        ಯದಿ( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          ಭಙ್ಗ;
        }
        ವಿಕಲ್ಪ( (u8)zIn[1] ){
          ಪಕ್ಷ '\'':
            jsonAppendChar(pOut, '\'');
            ಭಙ್ಗ;
          ಪಕ್ಷ 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            ಭಙ್ಗ;
          ಪಕ್ಷ 'x':
            ಯದಿ( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ಭಙ್ಗ;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            ಭಙ್ಗ;
          ಪಕ್ಷ '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            ಭಙ್ಗ;
          ಪಕ್ಷ '\r':
            ಯದಿ( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            ಭಙ್ಗ;
          ಪಕ್ಷ '\n':
            ಭಙ್ಗ;
          ಪಕ್ಷ 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            ಯದಿ( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ಭಙ್ಗ;
            }
            zIn += 2;
            sz2 -= 2;
            ಭಙ್ಗ;
          ಸ್ವಭಾವ:
            jsonAppendRawNZ(pOut, zIn, 2);
            ಭಙ್ಗ;
        }
        ಪ್ರತಿಜ್ಞಾ( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_TEXTRAW: {
      jsonAppendString(pOut, (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n], sz);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      ಯದಿ( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      ಯಾವತ್( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      ಯದಿ( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      ಯದಿ( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_OBJECT: {
      ಪೂರ್ಣಾಙ್ಕ x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      ಯದಿ( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      ಯಾವತ್( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      ಯದಿ( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      ಯದಿ( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      ಭಙ್ಗ;
    }

    ಸ್ವಭಾವ: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ i+n+sz;
}

/* Context for recursion of json_pretty()
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ JsonPretty JsonPretty;
ಸಂರಚನಾ JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  ಧ್ರುವ ವರ್ಣ *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
ಸ್ಥಿರ ಶೂನ್ಯ jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  ಪ್ರತಿ(jj=0; jj<pPretty->nIndent; jj++){
    jsonAppendRaw(pPretty->pOut, pPretty->zIndent, pPretty->szIndent);
  }
}

/*
** Translate the binary JSONB representation of JSON beginning at
** pParse->aBlob[i] into a JSON text string.  Append the JSON
** text onto the end of pOut.  Return the index in pParse->aBlob[]
** of the first byte past the end of the element that is translated.
**
** This is a variant of jsonTranslateBlobToText() that "pretty-prints"
** the output.  Extra whitespace is inserted to make the JSON easier
** for humans to read.
**
** If an error is detected in the BLOB input, the pOut->eErr flag
** might get set to JSTRING_MALFORMED.  But not all BLOB input errors
** are detected.  So a malformed JSONB input might either result
** in an error, or in incorrect JSON.
**
** The pOut->eErr JSTRING_OOM flag is set on a OOM.
*/
ಸ್ಥಿರ u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  ಯದಿ( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    ಪ್ರತ್ಯಾಗಮ pParse->nBlob+1;
  }
  ವಿಕಲ್ಪ( pParse->aBlob[i] & 0x0f ){
    ಪಕ್ಷ JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      ಯದಿ( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        ಯದಿ( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        ಯಾವತ್( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          ಯದಿ( j>=iEnd ){
            ಯದಿ( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            ಭಙ್ಗ;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      ಯದಿ( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        ಯದಿ( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        ಯಾವತ್( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          ಯದಿ( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            ಭಙ್ಗ;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          ಯದಿ( j>=iEnd ) ಭಙ್ಗ;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
ಸ್ಥಿರ u32 jsonb_Array_ಎಣಿಕೆ(JsonParse *pParse, u32 i_ಬೇರು){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, i_ಬೇರು, &sz);
  iEnd = i_ಬೇರು+n+sz;
  ಪ್ರತಿ(i=i_ಬೇರು+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSizeSemiInline(pParse, i, &sz);
  }
  ಪ್ರತ್ಯಾಗಮ k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonAfterEditSizeAdjust(JsonParse *pParse, u32 i_ಬೇರು){
  u32 sz = 0;
  u32 nBlob;
  ಪ್ರತಿಜ್ಞಾ( pParse->delta!=0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (ಶೂನ್ಯ)jsonbPayloadSize(pParse, i_ಬೇರು, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, i_ಬೇರು, sz);
}

/*
** If the JSONB at aIns[0..nIns-1] can be expanded (by denormalizing the
** size field) by d bytes, then write the expansion into aOut[] and
** return true.  In this way, an overwrite happens without changing the
** size of the JSONB, which reduces memcpy() operations and also make it
** faster and easier to update the B-Tree entry that contains the JSONB
** in the database.
**
** If the expansion of aIns[] by d bytes cannot be (easily) accomplished
** then return false.
**
** The d parameter is guaranteed to be between 1 and 8.
**
** This routine is an optimization.  A correct answer is obtained if it
** always leaves the output unchanged and returns false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  ಧ್ರುವ u8 *aIns,           /* New content */
  u32 nIns,                 /* Bytes of new content */
  u32 d                     /* Need to expand new content by this much */
){
  u32 szPayload;       /* Bytes of payload */
  u32 i;               /* New header size, after expansion & a loop counter */
  u8 szHdr;            /* Size of header before expansion */

  /* Lookup table for finding the upper 4 bits of the first byte of the
  ** expanded aIns[], based on the size of the expanded aIns[] header:
  **
  **                             2     3  4     5  6  7  8     9 */
  ಸ್ಥಿರ ಧ್ರುವ u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  ಯದಿ( (aIns[0]&0x0f)<=2 ) ಪ್ರತ್ಯಾಗಮ 0;    /* Cannot enlarge NULL, true, false */
  ವಿಕಲ್ಪ( aIns[0]>>4 ){
    ಸ್ವಭಾವ: {                         /* aIns[] header size 1 */
      ಯದಿ( ((1<<d)&0x116)==0 ) ಪ್ರತ್ಯಾಗಮ 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 12: {                         /* aIns[] header size is 2 */
      ಯದಿ( ((1<<d)&0x8a)==0) ಪ್ರತ್ಯಾಗಮ 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 13: {                         /* aIns[] header size is 3 */
      ಯದಿ( d!=2 && d!=6 ) ಪ್ರತ್ಯಾಗಮ 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 14: {                         /* aIns[] header size is 5 */
      ಯದಿ( d!=4 ) ಪ್ರತ್ಯಾಗಮ 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 15: {                         /* aIns[] header size is 9 */
      ಪ್ರತ್ಯಾಗಮ 0;                          /* No solution */
    }
  }
  ಪ್ರತಿಜ್ಞಾ( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  ಯಾವತ್( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    ಯದಿ( i==1 ) ಭಙ್ಗ;
    szPayload >>= 8;
  }
  ಪ್ರತಿಜ್ಞಾ( (szPayload>>8)==0 );
  ಪ್ರತ್ಯಾಗಮ 1;
}

/*
** Modify the JSONB blob at pParse->aBlob by removing nDel bytes of
** content beginning at iDel, and replacing them with nIns bytes of
** content given by aIns.
**
** nDel may be zero, in which case no bytes are removed.  But iDel is
** still important as new bytes will be insert beginning at iDel.
**
** aIns may be zero, in which case space is created to hold nIns bytes
** beginning at iDel, but that space is uninitialized.
**
** Set pParse->oom if an OOM occurs.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  ಧ್ರುವ u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  ಪ್ರತಿಜ್ಞಾ( pParse->nBlob >= (u64)iDel + (u64)nDel );
  ಯದಿ( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (ಪೂರ್ಣಾಙ್ಕ)-d)
  ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( d!=0 ){
    ಯದಿ( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      ಯದಿ( pParse->oom ) ಪ್ರತ್ಯಾಗಮ;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  ಯದಿ( nIns && aIns ){
    memcpy(&pParse->aBlob[iDel], aIns, nIns);
  }
}

/*
** Return the number of escaped newlines to be ignored.
** An escaped newline is a one of the following byte sequences:
**
**    0x5c 0x0a
**    0x5c 0x0d
**    0x5c 0x0d 0x0a
**    0x5c 0xe2 0x80 0xa8
**    0x5c 0xe2 0x80 0xa9
*/
ಸ್ಥಿರ u32 jsonBytesToBypass(ಧ್ರುವ ವರ್ಣ *z, u32 n){
  u32 i = 0;
  ಯಾವತ್( i+1<n ){
    ಯದಿ( z[i]!='\\' ) ಪ್ರತ್ಯಾಗಮ i;
    ಯದಿ( z[i+1]=='\n' ){
      i += 2;
      ಅನುವರ್ತ;
    }
    ಯದಿ( z[i+1]=='\r' ){
      ಯದಿ( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }ಅನ್ಯಥಾ{
        i += 2;
      }
      ಅನುವರ್ತ;
    }
    ಯದಿ( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      ಅನುವರ್ತ;
    }
    ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ i;
}

/*
** Input z[0..n] defines JSON escape sequence including the leading '\\'.
** Decode that escape sequence into a single character.  Write that
** character into *piOut.  Return the number of bytes in the escape sequence.
**
** If there is a syntax error of some kind (for example too few characters
** after the '\\' to complete the encoding) then *piOut is set to
** JSON_INVALID_CHAR.
*/
ಸ್ಥಿರ u32 jsonUnescapeOneChar(ಧ್ರುವ ವರ್ಣ *z, u32 n, u32 *piOut){
  ಪ್ರತಿಜ್ಞಾ( n>0 );
  ಪ್ರತಿಜ್ಞಾ( z[0]=='\\' );
  ಯದಿ( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    ಪ್ರತ್ಯಾಗಮ n;
  }
  ವಿಕಲ್ಪ( (u8)z[1] ){
    ಪಕ್ಷ 'u': {
      u32 v, vlo;
      ಯದಿ( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        ಪ್ರತ್ಯಾಗಮ n;
      }
      v = jsonHexToInt4(&z[2]);
      ಯದಿ( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        ಪ್ರತ್ಯಾಗಮ 12;
      }ಅನ್ಯಥಾ{
        *piOut = v;
        ಪ್ರತ್ಯಾಗಮ 6;
      }
    }
    ಪಕ್ಷ 'b': {   *piOut = '\b';  ಪ್ರತ್ಯಾಗಮ 2; }
    ಪಕ್ಷ 'f': {   *piOut = '\f';  ಪ್ರತ್ಯಾಗಮ 2; }
    ಪಕ್ಷ 'n': {   *piOut = '\n';  ಪ್ರತ್ಯಾಗಮ 2; }
    ಪಕ್ಷ 'r': {   *piOut = '\r';  ಪ್ರತ್ಯಾಗಮ 2; }
    ಪಕ್ಷ 't': {   *piOut = '\t';  ಪ್ರತ್ಯಾಗಮ 2; }
    ಪಕ್ಷ 'v': {   *piOut = '\v';  ಪ್ರತ್ಯಾಗಮ 2; }
    ಪಕ್ಷ '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#ಅನ್ಯಥಾ
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      ಪ್ರತ್ಯಾಗಮ 2;
    }
    ಪಕ್ಷ '\'':
    ಪಕ್ಷ '"':
    ಪಕ್ಷ '/':
    ಪಕ್ಷ '\\':{   *piOut = z[1];  ಪ್ರತ್ಯಾಗಮ 2; }
    ಪಕ್ಷ 'x': {
      ಯದಿ( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        ಪ್ರತ್ಯಾಗಮ n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      ಪ್ರತ್ಯಾಗಮ 4;
    }
    ಪಕ್ಷ 0xe2:
    ಪಕ್ಷ '\r':
    ಪಕ್ಷ '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      ಯದಿ( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        ಪ್ರತ್ಯಾಗಮ n;
      }ಅನ್ಯಥಾ ಯದಿ( nSkip==n ){
        *piOut = 0;
        ಪ್ರತ್ಯಾಗಮ n;
      }ಅನ್ಯಥಾ ಯದಿ( z[nSkip]=='\\' ){
        ಪ್ರತ್ಯಾಗಮ nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        ಪ್ರತ್ಯಾಗಮ nSkip + sz;
      }
    }
    ಸ್ವಭಾವ: {
      *piOut = JSON_INVALID_CHAR;
      ಪ್ರತ್ಯಾಗಮ 2;
    }
  }
}


/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.
**
** In this version, we know that one or the other or both of the
** two comparands contains an escape sequence.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ jsonLabelCompareEscaped(
  ಧ್ರುವ ವರ್ಣ *z_ಎಡ,          /* The left label */
  u32 n_ಎಡ,                  /* Size of the left label in bytes */
  ಪೂರ್ಣಾಙ್ಕ raw_ಎಡ,                /* True if zLeft contains no escapes */
  ಧ್ರುವ ವರ್ಣ *z_ಬಲ,         /* The right label */
  u32 n_ಬಲ,                 /* Size of the right label in bytes */
  ಪೂರ್ಣಾಙ್ಕ raw_ಬಲ                /* True if zRight is escape-free */
){
  u32 c_ಎಡ, c_ಬಲ;
  ಪ್ರತಿಜ್ಞಾ( raw_ಎಡ==0 || raw_ಬಲ==0 );
  ಯಾವತ್( 1 /*exit-by-return*/ ){
    ಯದಿ( n_ಎಡ==0 ){
      c_ಎಡ = 0;
    }ಅನ್ಯಥಾ ಯದಿ( raw_ಎಡ || z_ಎಡ[0]!='\\' ){
      c_ಎಡ = ((u8*)z_ಎಡ)[0];
      ಯದಿ( c_ಎಡ>=0xc0 ){
        ಪೂರ್ಣಾಙ್ಕ sz = sqlite3Utf8ReadLimited((u8*)z_ಎಡ, n_ಎಡ, &c_ಎಡ);
        z_ಎಡ += sz;
        n_ಎಡ -= sz;
      }ಅನ್ಯಥಾ{
        z_ಎಡ++;
        n_ಎಡ--;
      }
    }ಅನ್ಯಥಾ{
      u32 n = jsonUnescapeOneChar(z_ಎಡ, n_ಎಡ, &c_ಎಡ);
      z_ಎಡ += n;
      ಪ್ರತಿಜ್ಞಾ( n<=n_ಎಡ );
      n_ಎಡ -= n;
    }
    ಯದಿ( n_ಬಲ==0 ){
      c_ಬಲ = 0;
    }ಅನ್ಯಥಾ ಯದಿ( raw_ಬಲ || z_ಬಲ[0]!='\\' ){
      c_ಬಲ = ((u8*)z_ಬಲ)[0];
      ಯದಿ( c_ಬಲ>=0xc0 ){
        ಪೂರ್ಣಾಙ್ಕ sz = sqlite3Utf8ReadLimited((u8*)z_ಬಲ, n_ಬಲ, &c_ಬಲ);
        z_ಬಲ += sz;
        n_ಬಲ -= sz;
      }ಅನ್ಯಥಾ{
        z_ಬಲ++;
        n_ಬಲ--;
      }
    }ಅನ್ಯಥಾ{
      u32 n = jsonUnescapeOneChar(z_ಬಲ, n_ಬಲ, &c_ಬಲ);
      z_ಬಲ += n;
      ಪ್ರತಿಜ್ಞಾ( n<=n_ಬಲ );
      n_ಬಲ -= n;
    }
    ಯದಿ( c_ಎಡ!=c_ಬಲ ) ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ( c_ಎಡ==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonLabelCompare(
  ಧ್ರುವ ವರ್ಣ *z_ಎಡ,          /* The left label */
  u32 n_ಎಡ,                  /* Size of the left label in bytes */
  ಪೂರ್ಣಾಙ್ಕ raw_ಎಡ,                /* True if zLeft contains no escapes */
  ಧ್ರುವ ವರ್ಣ *z_ಬಲ,         /* The right label */
  u32 n_ಬಲ,                 /* Size of the right label in bytes */
  ಪೂರ್ಣಾಙ್ಕ raw_ಬಲ                /* True if zRight is escape-free */
){
  ಯದಿ( raw_ಎಡ && raw_ಬಲ ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    ಯದಿ( n_ಎಡ!=n_ಬಲ ) ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ memcmp(z_ಎಡ, z_ಬಲ, n_ಎಡ)==0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ jsonLabelCompareEscaped(z_ಎಡ, n_ಎಡ, raw_ಎಡ,
                                   z_ಬಲ, n_ಬಲ, raw_ಬಲ);
  }
}

/*
** Error returns from jsonLookupStep()
*/
#define JSON_LOOKUP_ERROR      0xffffffff
#define JSON_LOOKUP_NOTFOUND   0xfffffffe
#define JSON_LOOKUP_NOTARRAY   0xfffffffd
#define JSON_LOOKUP_TOODEEP    0xfffffffc
#define JSON_LOOKUP_PATHERROR  0xfffffffb
#define JSON_LOOKUP_ISERROR(x) ((x)>=JSON_LOOKUP_PATHERROR)

/* Forward declaration */
ಸ್ಥಿರ u32 jsonLookupStep(JsonParse*,u32,ಧ್ರುವ ವರ್ಣ*,u32);


/* This helper routine for jsonLookupStep() populates pIns with
** binary data that is to be inserted into pParse.
**
** In the common case, pIns just points to pParse->aIns and pParse->nIns.
** But if the zPath of the original edit operation includes path elements
** that go deeper, additional substructure must be created.
**
** For example:
**
**     json_insert('{}', '$.a.b.c', 123);
**
** The search stops at '$.a'  But additional substructure must be
** created for the ".b.c" part of the patch so that the final result
** is:  {"a":{"b":{"c"::123}}}.  This routine populates pIns with
** the binary equivalent of {"b":{"c":123}} so that it can be inserted.
**
** The caller is responsible for resetting pIns when it has finished
** using the substructure.
*/
ಸ್ಥಿರ u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  ಧ್ರುವ ವರ್ಣ *zTail   /* Tail of the path that determines substructure */
){
  ಸ್ಥಿರ ಧ್ರುವ u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  ಪೂರ್ಣಾಙ್ಕ rc;
  memset(pIns, 0, ಪರಿಮಾಣ(*pIns));
  pIns->db = pParse->db;
  ಯದಿ( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }ಅನ್ಯಥಾ{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    ಯದಿ( pIns->iDepth >= JSON_MAX_DEPTH ){
      ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  ಪ್ರತ್ಯಾಗಮ rc;  /* Error code only */
}

/*
** Search along zPath to find the Json element specified.  Return an
** index into pParse->aBlob[] for the start of that element's value.
**
** If the value found by this routine is the value half of label/value pair
** within an object, then set pPath->iLabel to the start of the corresponding
** label, before returning.
**
** Return one of the JSON_LOOKUP error codes if problems are seen.
**
** This routine will also modify the blob.  If pParse->eEdit is one of
** JEDIT_DEL, JEDIT_REPL, JEDIT_INS, JEDIT_SET, or JEDIT_AINS, then changes
** might be made to the selected value. If an edit is performed, then the
** return value does not necessarily point to the select element. If an edit
** is performed, the return value is only useful for detecting error
** conditions.
*/
ಸ್ಥಿರ u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_ಬೇರು,              /* Begin the search at this element of aBlob[] */
  ಧ್ರುವ ವರ್ಣ *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_ಕೀಲಿ, sz, n, iEnd, rc;
  ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ;
  u8 x;

  ಯದಿ( zPath[0]==0 ){
    ಯದಿ( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, i_ಬೇರು, &sz);
      sz += n;
      ಯದಿ( pParse->eEdit==JEDIT_DEL ){
        ಯದಿ( iLabel>0 ){
          sz += i_ಬೇರು - iLabel;
          i_ಬೇರು = iLabel;
        }
        jsonBlobEdit(pParse, i_ಬೇರು, sz, 0, 0);
      }ಅನ್ಯಥಾ ಯದಿ( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }ಅನ್ಯಥಾ ಯದಿ( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        ಯದಿ( zPath[-1]!=']' ){
          ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_NOTARRAY;
        }ಅನ್ಯಥಾ{
          jsonBlobEdit(pParse, i_ಬೇರು, 0, pParse->aIns, pParse->nIns);
        }
      }ಅನ್ಯಥಾ{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_ಬೇರು, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    ಪ್ರತ್ಯಾಗಮ i_ಬೇರು;
  }
  ಯದಿ( zPath[0]=='.' ){
    ಪೂರ್ಣಾಙ್ಕ raw_ಕೀಲಿ = 1;
    x = pParse->aBlob[i_ಬೇರು];
    zPath++;
    ಯದಿ( zPath[0]=='"' ){
      z_ಕೀಲಿ = zPath + 1;
      ಪ್ರತಿ(i=1; zPath[i] && zPath[i]!='"'; i++){
        ಯದಿ( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_ಕೀಲಿ = i-1;
      ಯದಿ( zPath[i] ){
        i++;
      }ಅನ್ಯಥಾ{
        ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_PATHERROR;
      }
      testcase( n_ಕೀಲಿ==0 );
      raw_ಕೀಲಿ = memchr(z_ಕೀಲಿ, '\\', n_ಕೀಲಿ)==0;
    }ಅನ್ಯಥಾ{
      z_ಕೀಲಿ = zPath;
      ಪ್ರತಿ(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_ಕೀಲಿ = i;
      ಯದಿ( n_ಕೀಲಿ==0 ){
        ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_PATHERROR;
      }
    }
    ಯದಿ( (x & 0x0f)!=JSONB_OBJECT ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_ಬೇರು, &sz);
    j = i_ಬೇರು + n;  /* j is the index of a label */
    iEnd = j+sz;
    ಯಾವತ್( j<iEnd ){
      ಪೂರ್ಣಾಙ್ಕ rawLabel;
      ಧ್ರುವ ವರ್ಣ *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      ಯದಿ( x<JSONB_TEXT || x>JSONB_TEXTRAW ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ಯದಿ( n==0 ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      ಯದಿ( k+sz>=iEnd ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
      zLabel = (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      ಯದಿ( jsonLabelCompare(z_ಕೀಲಿ, n_ಕೀಲಿ, raw_ಕೀಲಿ, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        ಯದಿ( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        ಯದಿ( n==0 || v+n+sz>iEnd ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
        ಪ್ರತಿಜ್ಞಾ( j>0 );
        ಯದಿ( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        ಯದಿ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ಬೇರು);
        ಪ್ರತ್ಯಾಗಮ rc;
      }
      j = k+sz;
      ಯದಿ( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ಯದಿ( n==0 ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ಯದಿ( j>iEnd ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
    ಯದಿ( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      ಯದಿ( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, ಪರಿಮಾಣ(ix));
      ix.db = pParse->db;
      json_Blob_Append_ಗಂಟು(&ix, raw_ಕೀಲಿ?JSONB_TEXTRAW:JSONB_TEXT5, n_ಕೀಲಿ, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      ಯದಿ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_ಕೀಲಿ+v.nBlob)
      ){
        ಪ್ರತಿಜ್ಞಾ( !pParse->oom );
        nIns = ix.nBlob + n_ಕೀಲಿ + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        ಯದಿ( !pParse->oom ){
          ಪ್ರತಿಜ್ಞಾ( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          ಪ್ರತಿಜ್ಞಾ( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], z_ಕೀಲಿ, n_ಕೀಲಿ);
          k += n_ಕೀಲಿ;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          ಯದಿ( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, i_ಬೇರು);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      ಪ್ರತ್ಯಾಗಮ rc;
    }
  }ಅನ್ಯಥಾ ಯದಿ( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_ಬೇರು] & 0x0f;
    ಯದಿ( x!=JSONB_ARRAY )  ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_ಬೇರು, &sz);
    i = 1;
    ಯಾವತ್( sqlite3Isdigit(zPath[i]) ){
      ಯದಿ( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    ಯದಿ( i<2 || zPath[i]!=']' ){
      ಯದಿ( zPath[1]=='#' ){
        kk = jsonb_Array_ಎಣಿಕೆ(pParse, i_ಬೇರು);
        i = 2;
        ಯದಿ( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          ಕುರು{
            ಯದಿ( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }ಯಾವತ್( sqlite3Isdigit(zPath[i]) );
          ಯದಿ( nn>kk ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        ಯದಿ( zPath[i]!=']' ){
          ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_PATHERROR;
        }
      }ಅನ್ಯಥಾ{
        ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_ಬೇರು+n;
    iEnd = j+sz;
    ಯಾವತ್( j<iEnd ){
      ಯದಿ( kk==0 ){
        ಯದಿ( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        ಯದಿ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ಬೇರು);
        ಪ್ರತ್ಯಾಗಮ rc;
      }
      kk--;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ಯದಿ( n==0 ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ಯದಿ( j>iEnd ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_ERROR;
    ಯದಿ( kk>0 ) ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_NOTFOUND;
    ಯದಿ( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      ಯದಿ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        ಪ್ರತಿಜ್ಞಾ( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      ಯದಿ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ಬೇರು);
      ಪ್ರತ್ಯಾಗಮ rc;
    }
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_PATHERROR; 
  }
  ಪ್ರತ್ಯಾಗಮ JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  ಧ್ರುವ u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  ಯದಿ( NEVER(aBlob==0) ) ಪ್ರತ್ಯಾಗಮ;
  memset(&x, 0, ಪರಿಮಾಣ(x));
  x.aBlob = (u8*)aBlob;
  x.nBlob = nBlob;
  jsonStringInit(&s, ctx);
  jsonTranslateBlobToText(&x, 0, &s);
  jsonReturnString(&s, 0, 0);
}


/*
** Return the value of the BLOB node at index i.
**
** If the value is a primitive, return it as an SQL value.
** If the value is an array or object, return it as either
** JSON text or the BLOB encoding, depending on the eMode flag
** as follows:
**
**     eMode==0     JSONB if the JSON_B flag is set in userdata or
**                  text if the JSON_B flag is omitted from userdata.
**
**     eMode==1     Text
**
**     eMode==2     JSONB
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  ಪೂರ್ಣಾಙ್ಕ eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  ಪ್ರತಿಜ್ಞಾ( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  ಯದಿ( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ವಿಕಲ್ಪ( pParse->aBlob[i] & 0x0f ){
    ಪಕ್ಷ JSONB_NULL: {
      ಯದಿ( sz ) ಗಚ್ಛ returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_TRUE: {
      ಯದಿ( sz ) ಗಚ್ಛ returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_FALSE: {
      ಯದಿ( sz ) ಗಚ್ಛ returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_INT5:
    ಪಕ್ಷ JSONB_INT: {
      sqlite3_int64 iRes = 0;
      ವರ್ಣ *z;
      ಪೂರ್ಣಾಙ್ಕ bNeg = 0;
      ವರ್ಣ x;
      ಯದಿ( sz==0 ) ಗಚ್ಛ returnfromblob_malformed;
      x = (ವರ್ಣ)pParse->aBlob[i+n];
      ಯದಿ( x=='-' ){
        ಯದಿ( sz<2 ) ಗಚ್ಛ returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n], (ಪೂರ್ಣಾಙ್ಕ)sz);
      ಯದಿ( z==0 ) ಗಚ್ಛ returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      ಯದಿ( rc==0 ){
        ಯದಿ( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          ದ್ವಿಗುಣ r;
          r = (ದ್ವಿಗುಣ)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }ಅನ್ಯಥಾ{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }ಅನ್ಯಥಾ ಯದಿ( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }ಅನ್ಯಥಾ ಯದಿ( rc==1 ){
        ಗಚ್ಛ returnfromblob_malformed;
      }ಅನ್ಯಥಾ{
        ಯದಿ( bNeg ){ n--; sz++; }
        ಗಚ್ಛ to_double;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_FLOAT5:
    ಪಕ್ಷ JSONB_FLOAT: {
      ದ್ವಿಗುಣ r;
      ವರ್ಣ *z;
      ಯದಿ( sz==0 ) ಗಚ್ಛ returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n], (ಪೂರ್ಣಾಙ್ಕ)sz);
      ಯದಿ( z==0 ) ಗಚ್ಛ returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      ಯದಿ( rc<=0 ) ಗಚ್ಛ returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_TEXTRAW:
    ಪಕ್ಷ JSONB_TEXT: {
      sqlite3_result_text(pCtx, (ವರ್ಣ*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_TEXT5:
    ಪಕ್ಷ JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      ಧ್ರುವ ವರ್ಣ *z;
      ವರ್ಣ *zOut;
      u32 nOut = sz;
      z = (ಧ್ರುವ ವರ್ಣ*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      ಯದಿ( zOut==0 ) ಗಚ್ಛ returnfromblob_oom;
      ಪ್ರತಿ(iIn=iOut=0; iIn<sz; iIn++){
        ವರ್ಣ c = z[iIn];
        ಯದಿ( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          ಯದಿ( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }ಅನ್ಯಥಾ ಯದಿ( v<=0x7f ){
            zOut[iOut++] = (ವರ್ಣ)v;
          }ಅನ್ಯಥಾ ಯದಿ( v<=0x7ff ){
            ಪ್ರತಿಜ್ಞಾ( szEscape>=2 );
            zOut[iOut++] = (ವರ್ಣ)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }ಅನ್ಯಥಾ ಯದಿ( v<0x10000 ){
            ಪ್ರತಿಜ್ಞಾ( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }ಅನ್ಯಥಾ ಯದಿ( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }ಅನ್ಯಥಾ{
            ಪ್ರತಿಜ್ಞಾ( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }ಅನ್ಯಥಾ{
          zOut[iOut++] = c;
        }
      } /* end for() */
      ಪ್ರತಿಜ್ಞಾ( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JSONB_ARRAY:
    ಪಕ್ಷ JSONB_OBJECT: {
      ಯದಿ( eMode==0 ){
        ಯದಿ( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }ಅನ್ಯಥಾ{
          eMode = 1;
        }
      }
      ಯದಿ( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }ಅನ್ಯಥಾ{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      ಗಚ್ಛ returnfromblob_malformed;
    }
  }
  ಪ್ರತ್ಯಾಗಮ;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  ಪ್ರತ್ಯಾಗಮ;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  ಪ್ರತ್ಯಾಗಮ;
}

/*
** pArg is a function argument that might be an SQL value or a JSON
** value.  Figure out what it is and encode it as a JSONB blob.
** Return the results in pParse.
**
** pParse is uninitialized upon entry.  This routine will handle the
** initialization of pParse.  The result will be contained in
** pParse->aBlob and pParse->nBlob.  pParse->aBlob might be dynamically
** allocated (if pParse->nBlobAlloc is greater than zero) in which case
** the caller is responsible for freeing the space allocated to pParse->aBlob
** when it has finished with it.  Or pParse->aBlob might be a static string
** or a value obtained from sqlite3_value_blob(pArg).
**
** If the argument is a BLOB that is clearly not a JSONB, then this
** function might set an error message in ctx and return non-zero.
** It might also set an error message and return non-zero on an OOM error.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_ಮೌಲ್ಯ *pArg,
  JsonParse *pParse
){
  ಪೂರ್ಣಾಙ್ಕ eType = sqlite3_ಮೌಲ್ಯ_type(pArg);
  ಸ್ಥಿರ u8 aNull[] = { 0x00 };
  memset(pParse, 0, ಪರಿಮಾಣ(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  ವಿಕಲ್ಪ( eType ){
    ಸ್ವಭಾವ: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪಕ್ಷ SQLITE_BLOB: {
      ಯದಿ( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_TEXT: {
      ಧ್ರುವ ವರ್ಣ *zJson = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pArg);
      ಪೂರ್ಣಾಙ್ಕ nJson = sqlite3_ಮೌಲ್ಯ_bytes(pArg);
      ಯದಿ( zJson==0 ) ಪ್ರತ್ಯಾಗಮ 1;
      ಯದಿ( sqlite3_ಮೌಲ್ಯ_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (ವರ್ಣ*)zJson;
        pParse->nJson = nJson;
        ಯದಿ( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, ಪರಿಮಾಣ(pParse[0]));
          ಪ್ರತ್ಯಾಗಮ 1;
        }
      }ಅನ್ಯಥಾ{
        json_Blob_Append_ಗಂಟು(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_FLOAT: {
      ಯದಿ( NEVER(sqlite3IsNaN(sqlite3_ಮೌಲ್ಯ_double(pArg))) ){
        json_Blob_Append_ಗಂಟು(pParse, JSONB_NULL, 0, 0);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ n = sqlite3_ಮೌಲ್ಯ_bytes(pArg);
        ಧ್ರುವ ವರ್ಣ *z = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pArg);
        ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ 1;
        ಯದಿ( z[0]=='I' ){
          json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 5, "9e999");
        }ಅನ್ಯಥಾ ಯದಿ( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 6, "-9e999");
        }ಅನ್ಯಥಾ{
          json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, n, z);
        }
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SQLITE_INTEGER: {
      ಪೂರ್ಣಾಙ್ಕ n = sqlite3_ಮೌಲ್ಯ_bytes(pArg);
      ಧ್ರುವ ವರ್ಣ *z = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pArg);
      ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ 1;
      json_Blob_Append_ಗಂಟು(pParse, JSONB_INT, n, z);
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}

/*
** Generate a path error.
**
** The specifics of the error are determined by the rc argument.
**
**          rc                        error
**  -----------------       ----------------------
**  JSON_LOOKUP_ARRAY       "not an array"
**  JSON_LOOKUP_TOODEEP     "JSON nested too deep"
**  JSON_LOOKUP_ERROR       "malformed JSON"
**  otherwise...            "bad JSON path"
**
** If ctx is not NULL then push the error message into ctx and return NULL.
** If ctx is NULL, then return the text of the error message.
*/
ಸ್ಥಿರ ವರ್ಣ *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  ಧ್ರುವ ವರ್ಣ *zPath,        /* The path with the problem */
  ಪೂರ್ಣಾಙ್ಕ rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  ವರ್ಣ *z_ಸಂದೇಶ;
  ಯದಿ( rc==(ಪೂರ್ಣಾಙ್ಕ)JSON_LOOKUP_NOTARRAY ){
    z_ಸಂದೇಶ = sqlite3_mprintf("not an array element: %Q", zPath);
  }ಅನ್ಯಥಾ ಯದಿ( rc==(ಪೂರ್ಣಾಙ್ಕ)JSON_LOOKUP_ERROR ){
    z_ಸಂದೇಶ = sqlite3_mprintf("malformed JSON");
  }ಅನ್ಯಥಾ ಯದಿ( rc==(ಪೂರ್ಣಾಙ್ಕ)JSON_LOOKUP_TOODEEP ){
    z_ಸಂದೇಶ = sqlite3_mprintf("JSON path too deep");
  }ಅನ್ಯಥಾ{
    z_ಸಂದೇಶ = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  ಯದಿ( ctx==0 ) ಪ್ರತ್ಯಾಗಮ z_ಸಂದೇಶ;
  ಯದಿ( z_ಸಂದೇಶ ){
    sqlite3_result_error(ctx, z_ಸಂದೇಶ, -1);
    sqlite3_free(z_ಸಂದೇಶ);
  }ಅನ್ಯಥಾ{
    sqlite3_result_error_nomem(ctx);
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_ಸೇರಿಸು_Into_Blob(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv,
  ಪೂರ್ಣಾಙ್ಕ eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  u32 rc = 0;
  ಧ್ರುವ ವರ್ಣ *zPath = 0;
  ಪೂರ್ಣಾಙ್ಕ flgs;
  JsonParse *p;
  JsonParse ax;

  ಪ್ರತಿಜ್ಞಾ( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ(i=1; i<argc-1; i+=2){
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[i])==SQLITE_NULL ) ಅನುವರ್ತ;
    zPath = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
    ಯದಿ( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ( zPath[0]!='$' ) ಗಚ್ಛ json_ಸೇರಿಸು_Into_Blob_patherror;
    ಯದಿ( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ( zPath[1]==0 ){
      ಯದಿ( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }ಅನ್ಯಥಾ{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    ಯದಿ( rc==JSON_LOOKUP_NOTFOUND ) ಅನುವರ್ತ;
    ಯದಿ( JSON_LOOKUP_ISERROR(rc) ) ಗಚ್ಛ json_ಸೇರಿಸು_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ಪ್ರತ್ಯಾಗಮ;

json_ಸೇರಿಸು_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  ಪ್ರತ್ಯಾಗಮ;
}

/*
** If pArg is a blob that seems like a JSONB blob, then initialize
** p to point to that JSONB and return TRUE.  If pArg does not seem like
** a JSONB blob, then return FALSE.
**
** For small BLOBs (having no more than 7 bytes of payload) a full
** validity check is done.  So for small BLOBs this routine only returns
** true if the value is guaranteed to be a valid JSONB.  For larger BLOBs
** (8 byte or more of payload) only the size of the outermost element is
** checked to verify that the BLOB is superficially valid JSONB.
**
** A full JSONB validation is done on smaller BLOBs because those BLOBs might
** also be text JSON that has been incorrectly cast into a BLOB.
** (See tag-20240123-a and https://sqlite.org/forum/forumpost/012136abd5)
** If the BLOB is 9 bytes are larger, then it is not possible for the
** superficial size check done here to pass if the input is really text
** JSON so we do not need to look deeper in that case.
**
** Why we only need to do full JSONB validation for smaller BLOBs:
**
** The first byte of valid JSON text must be one of: '{', '[', '"', ' ', '\n',
** '\r', '\t', '-', or a digit '0' through '9'.  Of these, only a subset
** can also be the first byte of JSONB:  '{', '[', and digits '3'
** through '9'.  In every one of those cases, the payload size is 7 bytes
** or less.  So if we do full JSONB validation for every BLOB where the
** payload is less than 7 bytes, we will never get a false positive for
** JSONB on an input that is really text JSON.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonArgIsJsonb(sqlite3_ಮೌಲ್ಯ *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(pArg)!=SQLITE_BLOB ) ಪ್ರತ್ಯಾಗಮ 0;
  p->aBlob = (u8*)sqlite3_ಮೌಲ್ಯ_blob(pArg);
  p->nBlob = (u32)sqlite3_ಮೌಲ್ಯ_bytes(pArg);
  ಯದಿ( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Generate a JsonParse object, containing valid JSONB in aBlob and nBlob,
** from the SQL function argument pArg.  Return a pointer to the new
** JsonParse object.
**
** Ownership of the new JsonParse object is passed to the caller.  The
** caller should invoke jsonParseFree() on the return value when it
** has finished using it.
**
** If any errors are detected, an appropriate error messages is set
** using sqlite3_result_error() or the equivalent and this routine
** returns NULL.  This routine also returns NULL if the pArg argument
** is an SQL NULL value, but no error message is set in that case.  This
** is so that SQL functions that are given NULL arguments will return
** a NULL value.
*/
ಸ್ಥಿರ JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_ಮೌಲ್ಯ *pArg,
  u32 flgs
){
  ಪೂರ್ಣಾಙ್ಕ eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  ಪ್ರತಿಜ್ಞಾ( ctx!=0 );
  eType = sqlite3_ಮೌಲ್ಯ_type(pArg);
  ಯದಿ( eType==SQLITE_NULL ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_ಹುಡುಕು(ctx, pArg);
  }
  ಯದಿ( pFromCache ){
    pFromCache->nJPRef++;
    ಯದಿ( (flgs & JSON_EDITABLE)==0 ){
      ಪ್ರತ್ಯಾಗಮ pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(*p));
  ಯದಿ( p==0 ) ಗಚ್ಛ json_pfa_oom;
  memset(p, 0, ಪರಿಮಾಣ(*p));
  p->db = db;
  p->nJPRef = 1;
  ಯದಿ( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    ಯದಿ( p->aBlob==0 ) ಗಚ್ಛ json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    ಪ್ರತ್ಯಾಗಮ p;
  }
  ಯದಿ( eType==SQLITE_BLOB ){
    ಯದಿ( jsonArgIsJsonb(pArg,p) ){
      ಯದಿ( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        ಗಚ್ಛ json_pfa_oom;
      }
      ಪ್ರತ್ಯಾಗಮ p;
    }
    /* If the blob is not valid JSONB, fall through into trying to cast
    ** the blob into text which is then interpreted as JSON.  (tag-20240123-a)
    **
    ** This goes against all historical documentation about how the SQLite
    ** JSON functions were suppose to work.  From the beginning, blob was
    ** reserved for expansion and a blob value should have raised an error.
    ** But it did not, due to a bug.  And many applications came to depend
    ** upon this buggy behavior, especially when using the CLI and reading
    ** JSON text using readfile(), which returns a blob.  For this reason
    ** we will continue to support the bug moving forward.
    ** See for example https://sqlite.org/forum/forumpost/012136abd5292b8d
    */
  }
  p->zJson = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pArg);
  p->nJson = sqlite3_ಮೌಲ್ಯ_bytes(pArg);
  ಯದಿ( db->mallocFailed ) ಗಚ್ಛ json_pfa_oom;
  ಯದಿ( p->nJson==0 ) ಗಚ್ಛ json_pfa_malformed;
  ಪ್ರತಿಜ್ಞಾ( p->zJson!=0 );
  ಯದಿ( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    ಯದಿ( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      ಪ್ರತ್ಯಾಗಮ p;
    }ಅನ್ಯಥಾ{
      jsonParseFree(p);
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ isRCStr = sqlite3_ಮೌಲ್ಯ_Is_Of_Class(pArg, sqlite3RCStrUnref);
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಯದಿ( !isRCStr ){
      ವರ್ಣ *zNew = sqlite3RCStrNew( p->nJson );
      ಯದಿ( zNew==0 ) ಗಚ್ಛ json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }ಅನ್ಯಥಾ{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_ಸೇರಿಸು(ctx, p);
    ಯದಿ( rc==SQLITE_NOMEM ) ಗಚ್ಛ json_pfa_oom;
    ಯದಿ( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      ಗಚ್ಛ rebuild_from_cache;
    }
  }
  ಪ್ರತ್ಯಾಗಮ p;

json_pfa_malformed:
  ಯದಿ( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    ಪ್ರತ್ಯಾಗಮ p;
  }ಅನ್ಯಥಾ{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    ಪ್ರತ್ಯಾಗಮ 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  ಪೂರ್ಣಾಙ್ಕ flgs;
  ಯದಿ( p->oom ){
    sqlite3_result_error_nomem(ctx);
    ಪ್ರತ್ಯಾಗಮ;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  ಯದಿ( flgs & JSON_BLOB ){
    ಯದಿ( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }ಅನ್ಯಥಾ{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }ಅನ್ಯಥಾ{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    jsonTranslateBlobToText(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_result_subtype(ctx, JSON_SUBTYPE);
  }
}

/****************************************************************************
** SQL functions used for testing and debugging
****************************************************************************/

#ಯದಿ SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  ಪೂರ್ಣಾಙ್ಕ nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  ಯಾವತ್( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    ಪೂರ್ಣಾಙ್ಕ showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    ಯದಿ( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    ಯದಿ( nn==0 ) nn = 1;
    ಯದಿ( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    ಪ್ರತಿ(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    ಯದಿ( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      ಅನುವರ್ತ;
    }
    pParse->nBlob = savedNBlob;
    ಯದಿ( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      ಯದಿ( iEnd>pParse->nBlob ){
        ಯದಿ( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }ಅನ್ಯಥಾ{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    ವಿಕಲ್ಪ( x ){
      ಪಕ್ಷ JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_INT:      sqlite3_str_appendall(pOut,"int"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); ಭಙ್ಗ;
      ಪಕ್ಷ JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        ಭಙ್ಗ;
      }
    }
    ಯದಿ( showContent ){
      ಯದಿ( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }ಅನ್ಯಥಾ{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        ಪ್ರತಿ(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          ಯದಿ( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (ವರ್ಣ*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  ವರ್ಣ zBuf[1000];
  ಯದಿ( pParse==0 ){
    printf("NULL pointer\n");
    ಪ್ರತ್ಯಾಗಮ;
  }ಅನ್ಯಥಾ{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    ಯದಿ( pParse->nBlob==0 ) ಪ್ರತ್ಯಾಗಮ;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, ಪರಿಮಾಣ(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_ಮೌಲ್ಯ(&out));
  sqlite3_str_reset(&out);
}
#endif /* SQLITE_DEBUG */

#ifdef SQLITE_DEBUG
/*
** SQL function:   json_parse(JSON)
**
** Parse JSON using jsonParseFuncArg().  Return text that is a
** human-readable dump of the binary JSONB for the input parameter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonParseFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  ಪ್ರತಿಜ್ಞಾ( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }ಅನ್ಯಥಾ{
    jsonShowParse(p);
  }
  jsonParseFree(p);
  sqlite3_str_reset(&out);
}
#endif /* SQLITE_DEBUG */

/****************************************************************************
** Scalar SQL function implementations
****************************************************************************/

/*
** Implementation of the json_quote(VALUE) function.  Return a JSON value
** corresponding to the SQL value input.  Mostly this means putting
** double-quotes around strings and returning the unquoted string "null"
** when given a NULL input.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonQuoteFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_ಮೌಲ್ಯ(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonArrayFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  ಪ್ರತಿ(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_ಮೌಲ್ಯ(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonArrayLengthFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( argc==2 ){
    ಧ್ರುವ ವರ್ಣ *zPath = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
    ಯದಿ( zPath==0 ){
      jsonParseFree(p);
      ಪ್ರತ್ಯಾಗಮ;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    ಯದಿ( JSON_LOOKUP_ISERROR(i) ){
      ಯದಿ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ಅನ್ಯಥಾ{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }ಅನ್ಯಥಾ{
    i = 0;
  }
  ಯದಿ( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_ಎಣಿಕೆ(p, i);
  }
  ಯದಿ( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonAllAlphanum(ಧ್ರುವ ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ n){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  ಪ್ರತ್ಯಾಗಮ i==n;
}

/*
** json_extract(JSON, PATH, ...)
** "->"(JSON,PATH)
** "->>"(JSON,PATH)
**
** Return the element described by PATH.  Return NULL if that PATH element
** is not found.
**
** If JSON_JSON is set or if more that one PATH argument is supplied then
** always return a JSON representation of the result.  If JSON_SQL is set,
** then always return an SQL representation of the result.  If neither flag
** is present and argc==2, then return JSON for objects and arrays and SQL
** for all other values.
**
** When multiple PATH arguments are supplied, the result is a JSON array
** containing the result of each PATH.
**
** Abbreviated JSON path expressions are allows if JSON_ABPATH, for
** compatibility with PG.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonExtractFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p = 0;      /* The parse */
  ಪೂರ್ಣಾಙ್ಕ flags;             /* Flags associated with the function */
  ಪೂರ್ಣಾಙ್ಕ i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  ಯದಿ( argc<2 ) ಪ್ರತ್ಯಾಗಮ;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  ಯದಿ( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  ಪ್ರತಿ(i=1; i<argc; i++){
    /* With a single PATH argument */
    ಧ್ರುವ ವರ್ಣ *zPath = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
    ಪೂರ್ಣಾಙ್ಕ nPath;
    u32 j;
    ಯದಿ( zPath==0 ) ಗಚ್ಛ json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    ಯದಿ( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }ಅನ್ಯಥಾ ಯದಿ( (flags & JSON_ABPATH) ){
      /* The -> and ->> operators accept abbreviated PATH arguments.  This
      ** is mostly for compatibility with PostgreSQL, but also for
      ** convenience.
      **
      **     NUMBER   ==>  $[NUMBER]     // PG compatible
      **     LABEL    ==>  $.LABEL       // PG compatible
      **     [NUMBER] ==>  $[NUMBER]     // Not PG.  Purely for convenience
      **
      ** Updated 2024-05-27:  If the NUMBER is negative, then PG counts from
      ** the right of the array.  Hence for negative NUMBER:
      **
      **     NUMBER   ==>  $[#NUMBER]    // PG compatible
      */
      jsonStringInit(&jx, ctx);
      ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        ಯದಿ( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }ಅನ್ಯಥಾ ಯದಿ( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }ಅನ್ಯಥಾ ಯದಿ( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }ಅನ್ಯಥಾ{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }ಅನ್ಯಥಾ{
      jsonBadPathError(ctx, zPath, 0);
      ಗಚ್ಛ json_extract_error;
    }
    ಯದಿ( j<p->nBlob ){
      ಯದಿ( argc==2 ){
        ಯದಿ( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          ಪ್ರತಿಜ್ಞಾ( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }ಅನ್ಯಥಾ{
          jsonReturnFromBlob(p, j, ctx, 0);
          ಯದಿ( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }ಅನ್ಯಥಾ{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }ಅನ್ಯಥಾ ಯದಿ( j==JSON_LOOKUP_NOTFOUND ){
      ಯದಿ( argc==2 ){
        ಗಚ್ಛ json_extract_error;  /* Return NULL if not found */
      }ಅನ್ಯಥಾ{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }ಅನ್ಯಥಾ{
      jsonBadPathError(ctx, zPath, j);
      ಗಚ್ಛ json_extract_error;
    }
  }
  ಯದಿ( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    ಯದಿ( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  ಪ್ರತ್ಯಾಗಮ;
}

/*
** Return codes for jsonMergePatch()
*/
#define JSON_MERGE_OK          0     /* Success */
#define JSON_MERGE_BADTARGET   1     /* Malformed TARGET blob */
#define JSON_MERGE_BADPATCH    2     /* Malformed PATCH blob */
#define JSON_MERGE_OOM         3     /* Out-of-memory condition */
#define JSON_MERGE_TOODEEP     4     /* Nested too deep */

/*
** RFC-7396 MergePatch for two JSONB blobs.
**
** pTarget is the target. pPatch is the patch.  The target is updated
** in place.  The patch is read-only.
**
** The original RFC-7396 algorithm is this:
**
**   define MergePatch(Target, Patch):
**     if Patch is an Object:
**       if Target is not an Object:
**         Target = {} # Ignore the contents and set it to an empty Object
**     for each Name/Value pair in Patch:
**         if Value is null:
**           if Name exists in Target:
**             remove the Name/Value pair from Target
**         else:
**           Target[Name] = MergePatch(Target[Name], Value)
**       return Target
**     else:
**       return Patch
**
** Here is an equivalent algorithm restructured to show the actual
** implementation:
**
** 01   define MergePatch(Target, Patch):
** 02      if Patch is not an Object:
** 03         return Patch
** 04      else: // if Patch is an Object
** 05         if Target is not an Object:
** 06            Target = {}
** 07      for each Name/Value pair in Patch:
** 08         if Name exists in Target:
** 09            if Value is null:
** 10               remove the Name/Value pair from Target
** 11            else
** 12               Target[name] = MergePatch(Target[Name], Value)
** 13         else if Value is not NULL:
** 14            if Value is not an Object:
** 15               Target[name] = Value
** 16            else:
** 17               Target[name] = MergePatch('{}',value)
** 18      return Target
**  |
**  ^---- Line numbers referenced in comments in the implementation
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  ಧ್ರುವ JsonParse *pPatch, /* The PATCH */
  u32 iPatch,              /* Index of PATCH in pPatch->aBlob[] */
  u32 iDepth               /* Nesting depth */
){
  u8 x;             /* Type of a single node */
  u32 n, sz=0;      /* Return values from jsonbPayloadSize() */
  u32 iTCursor;     /* Cursor position while scanning the target object */
  u32 iTStart;      /* First label in the target object */
  u32 iTEndBE;      /* Original first byte past end of target, before edit */
  u32 iTEnd;        /* Current first byte past end of target */
  u8 eTLabel;       /* Node type of the target label */
  u32 iTLabel = 0;  /* Index of the label */
  u32 nTLabel = 0;  /* Header size in bytes for the target label */
  u32 szTLabel = 0; /* Size of the target label payload */
  u32 iTValue = 0;  /* Index of the target value */
  u32 nTValue = 0;  /* Header size of the target value */
  u32 szTValue = 0; /* Payload size for the target value */

  u32 iPCursor;     /* Cursor position while scanning the patch */
  u32 iPEnd;        /* First byte past the end of the patch */
  u8 ePLabel;       /* Node type of the patch label */
  u32 iPLabel;      /* Start of patch label */
  u32 nPLabel;      /* Size of header on the patch label */
  u32 szPLabel;     /* Payload size of the patch label */
  u32 iPValue;      /* Start of patch value */
  u32 nPValue;      /* Header size for the patch value */
  u32 szPValue;     /* Payload size of the patch value */

  ಪ್ರತಿಜ್ಞಾ( iTarget>=0 && iTarget<pTarget->nBlob );
  ಪ್ರತಿಜ್ಞಾ( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  ಯದಿ( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    ಪ್ರತ್ಯಾಗಮ pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  ಯದಿ( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  ಯದಿ( NEVER(n==0) ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  ಯದಿ( NEVER(n==0) ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  ಯಾವತ್( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    ಯದಿ( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    ಯದಿ( nPLabel==0 ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    ಯದಿ( iPValue>=iPEnd ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    ಯದಿ( nPValue==0 ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    ಯದಿ( iPCursor>iPEnd ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    ಯಾವತ್( iTCursor<iTEnd ){
      ಪೂರ್ಣಾಙ್ಕ isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      ಯದಿ( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      ಯದಿ( nTLabel==0 ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      ಯದಿ( iTValue>=iTEnd ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      ಯದಿ( nTValue==0 ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADTARGET;
      ಯದಿ( iTValue + nTValue + szTValue > iTEnd ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ಧ್ರುವ ವರ್ಣ*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (ಧ್ರುವ ವರ್ಣ*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      ಯದಿ( isEqual ) ಭಙ್ಗ;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    ಯದಿ( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      ಯದಿ( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        ಯದಿ( NEVER(pTarget->oom) ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_OOM;
      }ಅನ್ಯಥಾ{
        /* Algorithm line 12 */
        ಪೂರ್ಣಾಙ್ಕ rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        ಯದಿ( iDepth>=JSON_MAX_DEPTH ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
        pTarget->delta += savedDelta;
      }        
    }ಅನ್ಯಥಾ ಯದಿ( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      ಯದಿ( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        ಯದಿ( pTarget->oom ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        ಯದಿ( pTarget->oom ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        ಯದಿ( iDepth>=JSON_MAX_DEPTH ) ಪ್ರತ್ಯಾಗಮ JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  ಯದಿ( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  ಪ್ರತ್ಯಾಗಮ pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonPatchFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  ಪೂರ್ಣಾಙ್ಕ rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  ಪ್ರತಿಜ್ಞಾ( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  ಯದಿ( pTarget==0 ) ಪ್ರತ್ಯಾಗಮ;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  ಯದಿ( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    ಯದಿ( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }ಅನ್ಯಥಾ ಯದಿ( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }ಅನ್ಯಥಾ ಯದಿ( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }ಅನ್ಯಥಾ{
      sqlite3_result_error(ctx, "malformed JSON", -1);
    }
    jsonParseFree(pPatch);
  }
  jsonParseFree(pTarget);
}


/*
** Implementation of the json_object(NAME,VALUE,...) function.  Return a JSON
** object that contains all name/value given in arguments.  Or if any name
** is not a string or if any value is a BLOB, throw an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonObjectFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ i;
  JsonString jx;
  ಧ್ರುವ ವರ್ಣ *z;
  u32 n;

  ಯದಿ( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    ಪ್ರತ್ಯಾಗಮ;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  ಪ್ರತಿ(i=0; i<argc; i+=2){
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      ಪ್ರತ್ಯಾಗಮ;
    }
    jsonAppendSeparator(&jx);
    z = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
    n = sqlite3_ಮೌಲ್ಯ_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_ಮೌಲ್ಯ(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}


/*
** json_remove(JSON, PATH, ...)
**
** Remove the named elements from JSON and return the result.  malformed
** JSON or PATH arguments result in an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonRemoveFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;          /* The parse */
  ಧ್ರುವ ವರ್ಣ *zPath = 0; /* Path of element to be removed */
  ಪೂರ್ಣಾಙ್ಕ i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  ಯದಿ( argc<1 ) ಪ್ರತ್ಯಾಗಮ;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ(i=1; i<argc; i++){
    zPath = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
    ಯದಿ( zPath==0 ){
      ಗಚ್ಛ json_remove_done;
    }
    ಯದಿ( zPath[0]!='$' ){
      ಗಚ್ಛ json_remove_patherror;
    }
    ಯದಿ( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      ಗಚ್ಛ json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    ಯದಿ( JSON_LOOKUP_ISERROR(rc) ){
      ಯದಿ( rc==JSON_LOOKUP_NOTFOUND ){
        ಅನುವರ್ತ;  /* No-op */
      }ಅನ್ಯಥಾ{
        jsonBadPathError(ctx, zPath, rc);
      }
      ಗಚ್ಛ json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ಪ್ರತ್ಯಾಗಮ;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  ಪ್ರತ್ಯಾಗಮ;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReplaceFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಯದಿ( argc<1 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( (argc&1)==0 ) {
    json_Wrong_ಅಂಕೆ_Args(ctx, "replace");
    ಪ್ರತ್ಯಾಗಮ;
  }
  json_ಸೇರಿಸು_Into_Blob(ctx, argc, argv, JEDIT_REPL);
}


/*
** json_set(JSON, PATH, VALUE, ...)
**
** Set the value at PATH to VALUE.  Create the PATH if it does not already
** exist.  Overwrite existing values that do exist.
** If JSON or PATH is malformed, throw an error.
**
** json_insert(JSON, PATH, VALUE, ...)
**
** Create PATH and initialize it to VALUE.  If PATH already exists, this
** routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonSetFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  ಪೂರ್ಣಾಙ್ಕ eInsType = JSON_ಸೇರಿಸು_TYPE(flags);
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *azInsType[] = { "insert", "set", "array_insert" };
  ಸ್ಥಿರ ಧ್ರುವ u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  ಯದಿ( argc<1 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿಜ್ಞಾ( eInsType>=0 && eInsType<=2 );
  ಯದಿ( (argc&1)==0 ) {
    json_Wrong_ಅಂಕೆ_Args(ctx, azInsType[eInsType]);
    ಪ್ರತ್ಯಾಗಮ;
  }
  json_ಸೇರಿಸು_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonTypeFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;          /* The parse */
  ಧ್ರುವ ವರ್ಣ *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( argc==2 ){
    zPath = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
    ಯದಿ( zPath==0 ) ಗಚ್ಛ json_type_done;
    ಯದಿ( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      ಗಚ್ಛ json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    ಯದಿ( JSON_LOOKUP_ISERROR(i) ){
      ಯದಿ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ಅನ್ಯಥಾ{
        jsonBadPathError(ctx, zPath, i);
      }
      ಗಚ್ಛ json_type_done;
    }
  }ಅನ್ಯಥಾ{
    i = 0;
  }
  sqlite3_result_text(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
json_type_done:
  jsonParseFree(p);
}

/*
** json_pretty(JSON)
** json_pretty(JSON, INDENT)
**
** Return text that is a pretty-printed rendering of the input JSON.
** If the argument is not valid JSON, return NULL.
**
** The INDENT argument is text that is used for indentation.  If omitted,
** it defaults to four spaces (the same as PostgreSQL).
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonPrettyFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, ಪರಿಮಾಣ(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  ಯದಿ( x.pParse==0 ) ಪ್ರತ್ಯಾಗಮ;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  ಯದಿ( argc==1 || (x.zIndent = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }ಅನ್ಯಥಾ{
    x.szIndent = (u32)strlen(x.zIndent);
  }
  jsonTranslateBlobToPrettyText(&x, 0);
  jsonReturnString(&s, 0, 0);
  jsonParseFree(x.pParse);
}

/*
** json_valid(JSON)
** json_valid(JSON, FLAGS)
**
** Check the JSON argument to see if it is well-formed.  The FLAGS argument
** encodes the various constraints on what is meant by "well-formed":
**
**     0x01      Canonical RFC-8259 JSON text
**     0x02      JSON text with optional JSON-5 extensions
**     0x04      Superficially appears to be JSONB
**     0x08      Strictly well-formed JSONB
**
** If the FLAGS argument is omitted, it defaults to 1.  Useful values for
** FLAGS include:
**
**    1          Strict canonical JSON text
**    2          JSON text perhaps with JSON-5 extensions
**    4          Superficially appears to be JSONB
**    5          Canonical JSON text or superficial JSONB
**    6          JSON-5 text or superficial JSONB
**    8          Strict JSONB
**    9          Canonical JSON text or strict JSONB
**    10         JSON-5 text or strict JSONB
**
** Other flag combinations are redundant.  For example, every canonical
** JSON text is also well-formed JSON-5 text, so FLAG values 2 and 3
** are the same.  Similarly, any input that passes a strict JSONB validation
** will also pass the superficial validation so 12 through 15 are the same
** as 8 through 11 respectively.
**
** This routine runs in linear time to validate text and when doing strict
** JSONB validation.  Superficial JSONB validation is constant time,
** assuming the BLOB is already in memory.  The performance advantage
** of superficial JSONB validation is why that option is provided.
** Application developers can choose to do fast superficial validation or
** slower strict validation, according to their specific needs.
**
** Only the lower four bits of the FLAGS argument are currently used.
** Higher bits are reserved for future expansion.   To facilitate
** compatibility, the current implementation raises an error if any bit
** in FLAGS is set other than the lower four bits.
**
** The original circa 2015 implementation of the JSON routines in
** SQLite only supported canonical RFC-8259 JSON text and the json_valid()
** function only accepted one argument.  That is why the default value
** for the FLAGS argument is 1, since FLAGS=1 causes this routine to only
** recognize canonical RFC-8259 JSON text as valid.  The extra FLAGS
** argument was added when the JSON routines were extended to support
** JSON5-like extensions and binary JSONB stored in BLOBs.
**
** Return Values:
**
**   *   Raise an error if FLAGS is outside the range of 1 to 15.
**   *   Return NULL if the input is NULL
**   *   Return 1 if the input is well-formed.
**   *   Return 0 if the input is not well-formed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonValidFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  ಯದಿ( argc==2 ){
    i64 f = sqlite3_ಮೌಲ್ಯ_int64(argv[1]);
    ಯದಿ( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      ಪ್ರತ್ಯಾಗಮ;
    }
    flags = f & 0x0f;
  }
  ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_type(argv[0]) ){
    ಪಕ್ಷ SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಪಕ್ಷ SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, ಪರಿಮಾಣ(py));
      ಯದಿ( jsonArgIsJsonb(argv[0], &py) ){
        ಯದಿ( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }ಅನ್ಯಥಾ ಯದಿ( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        ಭಙ್ಗ;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    ಸ್ವಭಾವ: {
      JsonParse px;
      ಯದಿ( (flags & 0x3)==0 ) ಭಙ್ಗ;
      memset(&px, 0, ಪರಿಮಾಣ(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      ಯದಿ( p ){
        ಯದಿ( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }ಅನ್ಯಥಾ ಯದಿ( p->nErr ){
          /* no-op */
        }ಅನ್ಯಥಾ ಯದಿ( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }ಅನ್ಯಥಾ{
        sqlite3_result_error_nomem(ctx);
      }
      ಭಙ್ಗ;
    }
  }
  sqlite3_result_int(ctx, res);
}

/*
** json_error_position(JSON)
**
** If the argument is NULL, return NULL
**
** If the argument is BLOB, do a full validity check and return non-zero
** if the check fails.  The return value is the approximate 1-based offset
** to the byte of the element that contains the first error.
**
** Otherwise interpret the argument is TEXT (even if it is numeric) and
** return the 1-based character position for where the parser first recognized
** that the input was not valid JSON, or return 0 if the input text looks
** ok.  JSON-5 extensions are accepted.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonErrorFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, ಪರಿಮಾಣ(s));
  s.db = sqlite3_context_db_handle(ctx);
  ಯದಿ( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }ಅನ್ಯಥಾ{
    s.zJson = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    ಯದಿ( s.zJson==0 ) ಪ್ರತ್ಯಾಗಮ;  /* NULL input or OOM */
    s.nJson = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
    ಯದಿ( jsonConvertTextToBlob(&s,0) ){
      ಯದಿ( s.oom ){
        iErrPos = -1;
      }ಅನ್ಯಥಾ{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        ಪ್ರತಿಜ್ಞಾ( s.zJson!=0 );  /* Because s.oom is false */
        ಪ್ರತಿ(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          ಯದಿ( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  ಯದಿ( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }ಅನ್ಯಥಾ{
    sqlite3_result_int64(ctx, iErrPos);
  }
}

/****************************************************************************
** Aggregate SQL function implementations
****************************************************************************/
/*
** json_group_array(VALUE)
**
** Return a JSON array composed of all values in the aggregate.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonArrayStep(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, ಪರಿಮಾಣ(*pStr));
  ಯದಿ( pStr ){
    ಯದಿ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }ಅನ್ಯಥಾ ಯದಿ( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_ಮೌಲ್ಯ(pStr, argv[0]);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonArrayCompute(sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ isFinal){
  JsonString *pStr;
  ಪೂರ್ಣಾಙ್ಕ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ಯದಿ( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    ಯದಿ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ಪ್ರತ್ಯಾಗಮ;
    }ಅನ್ಯಥಾ ಯದಿ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ಯದಿ( isFinal ){
        ಯದಿ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ಅನ್ಯಥಾ{
        jsonStringTrimOneChar(pStr);
      }
      ಪ್ರತ್ಯಾಗಮ;
    }ಅನ್ಯಥಾ ಯದಿ( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (ಪೂರ್ಣಾಙ್ಕ)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ಅನ್ಯಥಾ{
      sqlite3_result_text(ctx, pStr->zBuf, (ಪೂರ್ಣಾಙ್ಕ)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }ಅನ್ಯಥಾ ಯದಿ( flags & JSON_BLOB ){
    ಸ್ಥಿರ ಧ್ರುವ u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }ಅನ್ಯಥಾ{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
ಸ್ಥಿರ ಶೂನ್ಯ json_Array_ಮೌಲ್ಯ(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonGroupInverse(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ inStr = 0;
  ಪೂರ್ಣಾಙ್ಕ nNest = 0;
  ವರ್ಣ *z;
  ವರ್ಣ c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  ಯದಿ( NEVER(!pStr) ) ಪ್ರತ್ಯಾಗಮ;
  z = pStr->zBuf;
  ಪ್ರತಿ(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    ಯದಿ( c=='"' ){
      inStr = !inStr;
    }ಅನ್ಯಥಾ ಯದಿ( c=='\\' ){
      i++;
    }ಅನ್ಯಥಾ ಯದಿ( !inStr ){
      ಯದಿ( c=='{' || c=='[' ) nNest++;
      ಯದಿ( c=='}' || c==']' ) nNest--;
    }
  }
  ಯದಿ( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }ಅನ್ಯಥಾ{
    pStr->nUsed = 1;
  }
}
#ಅನ್ಯಥಾ
# define jsonGroupInverse 0
#endif


/*
** json_group_obj(NAME,VALUE)
**
** Return a JSON object composed of all names and values in the aggregate.
**
** Rows for which NAME is NULL do not result in a new entry.  However, we
** do initially insert a "@" entry into the growing string for each null entry
** and change the first character of the string to "@" to signal that the
** string contains null entries.  The "@" markers are needed in order to
** correctly process xInverse() requests.  The initial "@" is converted
** back into "{" and the "@" null values are removed by jsonObjectCompute().
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonObjectStep(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonString *pStr;
  ಧ್ರುವ ವರ್ಣ *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, ಪರಿಮಾಣ(*pStr));
  ಯದಿ( pStr ){
    z = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    n = sqlite3Strlen30(z);
    ಯದಿ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }ಅನ್ಯಥಾ ಯದಿ( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    ಯದಿ( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_ಮೌಲ್ಯ(pStr, argv[1]);
    }ಅನ್ಯಥಾ{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonObjectCompute(sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ isFinal){
  JsonString *pStr;
  ಪೂರ್ಣಾಙ್ಕ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ಯದಿ( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    ಯದಿ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      ಪೂರ್ಣಾಙ್ಕ inStr = 0;
      ಯದಿ( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        ಯದಿ( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          ಪ್ರತ್ಯಾಗಮ;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      ಪ್ರತಿ(i=j=1; i<pStr->nUsed; i++){
        ವರ್ಣ c = pStr->zBuf[i];
        ಯದಿ( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }ಅನ್ಯಥಾ ಯದಿ( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }ಅನ್ಯಥಾ ಯದಿ( c=='@' && !inStr ){
          ಪ್ರತಿಜ್ಞಾ( i+1<pStr->nUsed );
          ಯದಿ( pStr->zBuf[i+1]==',' ){
            i++;
          }ಅನ್ಯಥಾ ಯದಿ( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }ಅನ್ಯಥಾ{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    ಯದಿ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ಯದಿ( isFinal ){
        ಯದಿ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ಅನ್ಯಥಾ{
        jsonStringTrimOneChar(pOgStr);
      }
    }ಅನ್ಯಥಾ ಯದಿ( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (ಪೂರ್ಣಾಙ್ಕ)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ಅನ್ಯಥಾ{
      sqlite3_result_text(ctx, pStr->zBuf, (ಪೂರ್ಣಾಙ್ಕ)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    ಯದಿ( pStr!=pOgStr ) jsonStringReset(pStr);
  }ಅನ್ಯಥಾ ಯದಿ( flags & JSON_BLOB ){
    ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }ಅನ್ಯಥಾ{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
ಸ್ಥಿರ ಶೂನ್ಯ json_Object_ಮೌಲ್ಯ(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ JsonParent JsonParent;
ಸಂರಚನಾ JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 i_ಮೌಲ್ಯ;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_ಕೀಲಿ;                  /* Key for JSONB_ARRAY */
};

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ JsonEachCursor JsonEachCursor;
ಸಂರಚನಾ JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_ಬೇರು;                 /* Size of the root path in bytes */
  u8 eType;                  /* Type of the container for element i */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u32 nParent;               /* Current nesting depth */
  u32 nParentAlloc;          /* Space allocated for aParent[] */
  JsonParent *aParent;       /* Parent elements of i */
  sqlite3 *db;               /* Database connection */
  JsonString path;           /* Current path */
  JsonParse sParse;          /* Parse of the input JSON */
};
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ JsonEachConnection JsonEachConnection;
ಸಂರಚನಾ JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  JsonEachConnection *pNew;
  ಪೂರ್ಣಾಙ್ಕ rc;

/* Column numbers */
#define JEACH_ಕೀಲಿ     0
#define JEACH_ಮೌಲ್ಯ   1
#define JEACH_TYPE    2
#define JEACH_ATOM    3
#define JEACH_ID      4
#define JEACH_PARENT  5
#define JEACH_FULLKEY 6
#define JEACH_PATH    7
/* The xBestIndex method assumes that the JSON and ROOT columns are
** the last two columns in the table.  Should this ever changes, be
** sure to update the xBestIndex method. */
#define JEACH_JSON    8
#define JEACH_ಬೇರು    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  ಯದಿ( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, ಪರಿಮಾಣ(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* destructor for json_each virtual table */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, ಪರಿಮಾಣ(*pCur));
  ಯದಿ( pCur==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
ಸ್ಥಿರ ಶೂನ್ಯ jsonEachCursorReset(JsonEachCursor *p){
  jsonParseReset(&p->sParse);
  jsonStringReset(&p->path);
  sqlite3DbFree(p->db, p->aParent);
  p->iRowid = 0;
  p->i = 0;
  p->aParent = 0;
  p->nParent = 0;
  p->nParentAlloc = 0;
  p->iEnd = 0;
  p->eType = 0;
}

/* Destructor for a jsonEachCursor object */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ಪ್ರತ್ಯಾಗಮ p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonSkipLabel(JsonEachCursor *p){
  ಯದಿ( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    ಯದಿ( sz >= p->sParse.nBlob ) sz = p->i;
    ಪ್ರತ್ಯಾಗಮ sz;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ p->i;
  }
}

/*
** Append the path name for the current element.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendPathName(JsonEachCursor *p){
  ಪ್ರತಿಜ್ಞಾ( p->nParent>0 );
  ಪ್ರತಿಜ್ಞಾ( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  ಯದಿ( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_ಕೀಲಿ);
  }ಅನ್ಯಥಾ{
    u32 n, sz = 0, k, i;
    ಧ್ರುವ ವರ್ಣ *z;
    ಪೂರ್ಣಾಙ್ಕ needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (ಧ್ರುವ ವರ್ಣ*)&p->sParse.aBlob[k];
    ಯದಿ( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿ(i=0; i<sz; i++){
        ಯದಿ( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          ಭಙ್ಗ;
        }
      }
    }
    ಯದಿ( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }ಅನ್ಯಥಾ{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ json_Each_Malformed_ಒಳಹರಿವು(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_ಸಂದೇಶ);
  cur->pVtab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  ಪ್ರತ್ಯಾಗಮ cur->pVtab->z_Err_ಸಂದೇಶ ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    ಯದಿ( n==0 )ಪ್ರತ್ಯಾಗಮ json_Each_Malformed_ಒಳಹರಿವು(cur);
    ಯದಿ( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      ಯದಿ( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, ಪರಿಮಾಣ(JsonParent)*nNew);
        ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->i_ಮೌಲ್ಯ = i;
      pParent->iEnd = i + n + sz;
      pParent->i_ಕೀಲಿ = -1;
      pParent->nPath = (u32)p->path.nUsed;
      ಯದಿ( p->eType && p->nParent ){
        jsonAppendPathName(p);
        ಯದಿ( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }ಅನ್ಯಥಾ{
      p->i = i + n + sz;
    }
    ಯಾವತ್( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    ಯದಿ( levelChange ){
      ಯದಿ( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->i_ಮೌಲ್ಯ;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }ಅನ್ಯಥಾ{
        p->eType = 0;
      }
    }
  }ಅನ್ಯಥಾ{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    ಯದಿ( n==0 )ಪ್ರತ್ಯಾಗಮ json_Each_Malformed_ಒಳಹರಿವು(cur);
    p->i = i + n + sz;
  }
  ಯದಿ( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_ಕೀಲಿ++;
  }
  p->iRowid++;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  ವರ್ಣ *z = p->path.zBuf;
  ಯದಿ( p->iRowid==0 && p->bRecursive && n>=2 ){
    ಯಾವತ್( ALWAYS(n>1) ){
      n--;
      ಯದಿ( z[n]=='[' || z[n]=='.' ){
        u32 x;
        ವರ್ಣ cSaved = z[n];
        z[n] = 0;
        ಪ್ರತಿಜ್ಞಾ( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        ಯದಿ( !JSON_LOOKUP_ISERROR(x) ) ಭಙ್ಗ;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ n;
}

/* Return the value of a column */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ಪೂರ್ಣಾಙ್ಕ iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ವಿಕಲ್ಪ( iColumn ){
    ಪಕ್ಷ JEACH_ಕೀಲಿ: {
      ಯದಿ( p->nParent==0 ){
        u32 n, j;
        ಯದಿ( p->n_ಬೇರು==1 ) ಭಙ್ಗ;
        j = jsonEachPathLength(p);
        n = p->n_ಬೇರು - j;
        ಯದಿ( n==0 ){
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ ಯದಿ( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }ಅನ್ಯಥಾ ಯದಿ( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }ಅನ್ಯಥಾ{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        ಭಙ್ಗ;
      }
      ಯದಿ( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }ಅನ್ಯಥಾ{
        ಪ್ರತಿಜ್ಞಾ( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].i_ಕೀಲಿ);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JEACH_ಮೌಲ್ಯ: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      ಯದಿ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      ಯದಿ( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JEACH_PARENT: {
      ಯದಿ( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      ಯದಿ( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      sqlite3_result_text(ctx, p->path.zBuf, p->n_ಬೇರು, SQLITE_TRANSIENT);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ JEACH_JSON: {
      ಯದಿ( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }ಅನ್ಯಥಾ{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Return the current rowid value */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  ಪೂರ್ಣಾಙ್ಕ i;                     /* Loop counter or computed array index */
  ಪೂರ್ಣಾಙ್ಕ aIdx[2];               /* Index of constraints for JSON and ROOT */
  ಪೂರ್ಣಾಙ್ಕ unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  ಪೂರ್ಣಾಙ್ಕ idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  ಧ್ರುವ ಸಂರಚನಾ sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  ಪ್ರತಿಜ್ಞಾ( JEACH_ಬೇರು == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  ಪ್ರತಿ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ಪೂರ್ಣಾಙ್ಕ iCol;
    ಪೂರ್ಣಾಙ್ಕ iMask;
    ಯದಿ( pConstraint->iColumn < JEACH_JSON ) ಅನುವರ್ತ;
    iCol = pConstraint->iColumn - JEACH_JSON;
    ಪ್ರತಿಜ್ಞಾ( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    ಯದಿ( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }ಅನ್ಯಥಾ ಯದಿ( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  ಯದಿ( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  ಯದಿ( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    ಪ್ರತ್ಯಾಗಮ SQLITE_CONSTRAINT;
  }
  ಯದಿ( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_ಅಂಕೆ = 0;
  }ಅನ್ಯಥಾ{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ಯದಿ( aIdx[1]<0 ){
      pIdxInfo->idx_ಅಂಕೆ = 1;  /* Only JSON supplied.  Plan 1 */
    }ಅನ್ಯಥಾ{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_ಅಂಕೆ = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Start a search on a new JSON string */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ಧ್ರುವ ವರ್ಣ *z_ಬೇರು = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  ಯದಿ( idx_ಅಂಕೆ==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  memset(&p->sParse, 0, ಪರಿಮಾಣ(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  ಯದಿ( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }ಅನ್ಯಥಾ{
    p->sParse.zJson = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    p->sParse.nJson = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
    ಯದಿ( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    }      
    ಯದಿ( jsonConvertTextToBlob(&p->sParse, 0) ){
      ಯದಿ( p->sParse.oom ){
        ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
      }
      ಪ್ರತ್ಯಾಗಮ json_Each_Malformed_ಒಳಹರಿವು(cur);
    }
  }
  ಯದಿ( idx_ಅಂಕೆ==3 ){
    z_ಬೇರು = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
    ಯದಿ( z_ಬೇರು==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    ಯದಿ( z_ಬೇರು[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_ಸಂದೇಶ);
      cur->pVtab->z_Err_ಸಂದೇಶ = jsonBadPathError(0, z_ಬೇರು, 0);
      jsonEachCursorReset(p);
      ಪ್ರತ್ಯಾಗಮ cur->pVtab->z_Err_ಸಂದೇಶ ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_ಬೇರು = sqlite3Strlen30(z_ಬೇರು);
    ಯದಿ( z_ಬೇರು[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }ಅನ್ಯಥಾ{
      i = jsonLookupStep(&p->sParse, 0, z_ಬೇರು+1, 0);
      ಯದಿ( JSON_LOOKUP_ISERROR(i) ){
        ಯದಿ( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_ಸಂದೇಶ);
        cur->pVtab->z_Err_ಸಂದೇಶ = jsonBadPathError(0, z_ಬೇರು, 0);
        jsonEachCursorReset(p);
        ಪ್ರತ್ಯಾಗಮ cur->pVtab->z_Err_ಸಂದೇಶ ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      ಯದಿ( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }ಅನ್ಯಥಾ{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_ಬೇರು, p->n_ಬೇರು);
  }ಅನ್ಯಥಾ{
    i = p->i = 0;
    p->eType = 0;
    p->n_ಬೇರು = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  ಯದಿ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, ಪರಿಮಾಣ(JsonParent));
    ಯದಿ( p->aParent==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_ಕೀಲಿ = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].i_ಮೌಲ್ಯ = i;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* The methods of the json_each virtual table */
ಸ್ಥಿರ sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  jsonEachBestIndex,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  jsonEachNext,              /* xNext - advance a cursor */
  jsonEachEof,               /* xEof - check for end of scan */
  jsonEachColumn,            /* xColumn - read data */
  jsonEachRowid,             /* xRowid - read data */
  0,                         /* xUpdate */
  0,                         /* xBegin */
  0,                         /* xSync */
  0,                         /* xCommit */
  0,                         /* xRollback */
  0,                         /* xFindMethod */
  0,                         /* xRename */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadowName */
  0                          /* xIntegrity */
};
#endif /* SQLITE_OMIT_VIRTUALTABLE */
#endif /* !defined(SQLITE_OMIT_JSON) */

/*
** Register JSON functions.
*/
ಶೂನ್ಯ sqlite3RegisterJsonFunctions(ಶೂನ್ಯ){
#ifndef SQLITE_OMIT_JSON
  ಸ್ಥಿರ FuncDef aJsonFunc[] = {
    /*   sqlite3_result_subtype() ----,  ,--- sqlite3_value_subtype()       */
    /*                                |  |                                  */
    /*             Uses cache ------, |  | ,---- Returns JSONB              */
    /*                              | |  | |                                */
    /*     Number of arguments ---, | |  | | ,--- Flags                     */
    /*                            | | |  | | |                              */
    JFUNCTION(json,               1,1,1, 0,0,0,          jsonRemoveFunc),
    JFUNCTION(jsonb,              1,1,0, 0,1,0,          jsonRemoveFunc),
    JFUNCTION(json_array,        -1,0,1, 1,0,0,          jsonArrayFunc),
    JFUNCTION(jsonb_array,       -1,0,1, 1,1,0,          jsonArrayFunc),
    JFUNCTION(json_array_ಸೇರಿಸು, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_ಸೇರಿಸು,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_ಸೇರಿಸು,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_ಸೇರಿಸು,      -1,1,0, 1,1,0,          jsonSetFunc),
    JFUNCTION(json_object,       -1,0,1, 1,0,0,          jsonObjectFunc),
    JFUNCTION(jsonb_object,      -1,0,1, 1,1,0,          jsonObjectFunc),
    JFUNCTION(json_patch,         2,1,1, 0,0,0,          jsonPatchFunc),
    JFUNCTION(jsonb_patch,        2,1,0, 0,1,0,          jsonPatchFunc),
    JFUNCTION(json_pretty,        1,1,0, 0,0,0,          jsonPrettyFunc),
    JFUNCTION(json_pretty,        2,1,0, 0,0,0,          jsonPrettyFunc),
    JFUNCTION(json_quote,         1,0,1, 1,0,0,          jsonQuoteFunc),
    JFUNCTION(json_remove,       -1,1,1, 0,0,0,          jsonRemoveFunc),
    JFUNCTION(jsonb_remove,      -1,1,0, 0,1,0,          jsonRemoveFunc),
    JFUNCTION(json_replace,      -1,1,1, 1,0,0,          jsonReplaceFunc),
    JFUNCTION(jsonb_replace,     -1,1,0, 1,1,0,          jsonReplaceFunc),
    JFUNCTION(json_set,          -1,1,1, 1,0,JSON_ISSET, jsonSetFunc),
    JFUNCTION(jsonb_set,         -1,1,0, 1,1,JSON_ISSET, jsonSetFunc),
    JFUNCTION(json_type,          1,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_type,          2,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_valid,         1,1,0, 0,0,0,          jsonValidFunc),
    JFUNCTION(json_valid,         2,1,0, 0,0,0,          jsonValidFunc),
#ಯದಿ SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_ಮೌಲ್ಯ, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_ಮೌಲ್ಯ, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_ಮೌಲ್ಯ, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_ಮೌಲ್ಯ, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_ಸೇರಿಸು_Builtin_Funcs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#ಯದಿ  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zName){
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  ಪ್ರತಿಜ್ಞಾ( sqlite3HashFind(&db->aModule, zName)==0 );
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(azModule)/ಪರಿಮಾಣ(azModule[0]); i++){
    ಯದಿ( sqlite3StrICmp(azModule[i],zName)==0 ){
      ಪ್ರತ್ಯಾಗಮ sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
