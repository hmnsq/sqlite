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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

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
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର * ଅଚଳ jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର jsonIsSpace[] = {
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
#define jsonIsspace(x) (jsonIsSpace[(unsigned ଅକ୍ଷର)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର jsonIsOk[256] = {
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା JsonCache JsonCache;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା JsonString JsonString;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା JsonParse JsonParse;

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
ସଂରଚନା JsonCache {
  sqlite3 *db;                    /* Database connection */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nUsed;                      /* Number of active entries in the cache */
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
ସଂରଚନା JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  ଅକ୍ଷର *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  ଅକ୍ଷର zSpace[100];        /* Initial static space */
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

#define JSON_ଭର୍ତ୍ତି_TYPE(X)  (((X)&0xC)>>2)


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
ସଂରଚନା JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  ଅକ୍ଷର *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nJson;         /* Length of the zJson string in bytes */
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା delta;         /* Size change due to the edit */
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
#ଅନ୍ୟଥା
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
ସ୍ଥିର ଶୂନ୍ୟ jsonReturnStringAsBlob(JsonString*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonArgIsJsonb(sqlite3_ମୂଲ୍ୟ *pJson, JsonParse *p);
ସ୍ଥିର u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
ସ୍ଥିର ଶୂନ୍ୟ jsonReturnParse(sqlite3_context*,JsonParse*);
ସ୍ଥିର JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_ମୂଲ୍ୟ*,u32);
ସ୍ଥିର ଶୂନ୍ୟ jsonParseFree(JsonParse*);
ସ୍ଥିର u32 jsonbPayloadSize(ଅଚଳ JsonParse*, u32, u32*);
ସ୍ଥିର u32 jsonUnescapeOneChar(ଅଚଳ ଅକ୍ଷର*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonCacheDelete(JsonCache *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
ସ୍ଥିର ଶୂନ୍ୟ jsonCacheDeleteGeneric(ଶୂନ୍ୟ *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା json_Cache_ଭର୍ତ୍ତି(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  ପ୍ରତିଜ୍ଞା( pParse->zJson!=0 );
  ପ୍ରତିଜ୍ଞା( pParse->bJsonIsRCStr );
  ପ୍ରତିଜ୍ଞା( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ଯଦି( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, ଆକାର(*p));
    ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  }
  ଯଦି( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*ଆକାର(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  ପ୍ରତିଜ୍ଞା( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର JsonParse *json_Cache_ସନ୍ଧାନ(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_ମୂଲ୍ୟ *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅଚଳ ଅକ୍ଷର *zJson;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nJson;

  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(pArg)!=SQLITE_TEXT ){
    ଫେରନ୍ତୁ 0;
  }
  zJson = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pArg);
  ଯଦି( zJson==0 ) ଫେରନ୍ତୁ 0;
  nJson = sqlite3_ମୂଲ୍ୟ_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ଯଦି( p==0 ){
    ଫେରନ୍ତୁ 0;
  }
  ପାଇଁ(i=0; i<p->nUsed; i++){
    ଯଦି( p->a[i]->zJson==zJson ) ଭାଙ୍ଗ;
  }
  ଯଦି( i>=p->nUsed ){
    ପାଇଁ(i=0; i<p->nUsed; i++){
      ଯଦି( p->a[i]->nJson!=nJson ) ଜାରି;
      ଯଦି( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) ଭାଙ୍ଗ;
    }
  }
  ଯଦି( i<p->nUsed ){
    ଯଦି( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*ଆକାର(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    ପ୍ରତିଜ୍ଞା( p->a[i]->delta==0 );
    ଫେରନ୍ତୁ p->a[i];
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = ଆକାର(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonStringReset(JsonString *p){
  ଯଦି( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  ଯଦି( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  ପ୍ରତିଜ୍ଞା( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  ଅକ୍ଷର *zNew;
  ଯଦି( p->bStatic ){
    ଯଦି( p->eErr ) ଫେରନ୍ତୁ 1;
    zNew = sqlite3RCStrNew(nTotal);
    ଯଦି( zNew==0 ){
      jsonStringOom(p);
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }ଅନ୍ୟଥା{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    ଯଦି( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ jsonStringExpandAndAppend(
  JsonString *p,
  ଅଚଳ ଅକ୍ଷର *zIn,
  u32 N
){
  ପ୍ରତିଜ୍ଞା( N>0 );
  ଯଦି( jsonStringGrow(p,N) ) ଫେରନ୍ତୁ;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
ସ୍ଥିର ଶୂନ୍ୟ jsonAppendRaw(JsonString *p, ଅଚଳ ଅକ୍ଷର *zIn, u32 N){
  ଯଦି( N==0 ) ଫେରନ୍ତୁ;
  ଯଦି( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }ଅନ୍ୟଥା{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
ସ୍ଥିର ଶୂନ୍ୟ jsonAppendRawNZ(JsonString *p, ଅଚଳ ଅକ୍ଷର *zIn, u32 N){
  ପ୍ରତିଜ୍ଞା( N>0 );
  ଯଦି( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }ଅନ୍ୟଥା{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonPrintf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା N, JsonString *p, ଅଚଳ ଅକ୍ଷର *zFormat, ...){
  va_list ap;
  ଯଦି( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) ଫେରନ୍ତୁ;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ jsonAppendCharExpand(JsonString *p, ଅକ୍ଷର c){
  ଯଦି( jsonStringGrow(p,1) ) ଫେରନ୍ତୁ;
  p->zBuf[p->nUsed++] = c;
}
ସ୍ଥିର ଶୂନ୍ୟ jsonAppendChar(JsonString *p, ଅକ୍ଷର c){
  ଯଦି( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }ଅନ୍ୟଥା{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonStringTrimOneChar(JsonString *p){
  ଯଦି( p->eErr==0 ){
    ପ୍ରତିଜ୍ଞା( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  ଫେରନ୍ତୁ p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonAppendSeparator(JsonString *p){
  ଅକ୍ଷର c;
  ଯଦି( p->nUsed==0 ) ଫେରନ୍ତୁ;
  c = p->zBuf[p->nUsed-1];
  ଯଦି( c=='[' || c=='{' ) ଫେରନ୍ତୁ;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonAppendControlChar(JsonString *p, u8 c){
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  ପ୍ରତିଜ୍ଞା( ଆକାର(aSpecial)==32 );
  ପ୍ରତିଜ୍ଞା( aSpecial['\b']=='b' );
  ପ୍ରତିଜ୍ଞା( aSpecial['\f']=='f' );
  ପ୍ରତିଜ୍ଞା( aSpecial['\n']=='n' );
  ପ୍ରତିଜ୍ଞା( aSpecial['\r']=='r' );
  ପ୍ରତିଜ୍ଞା( aSpecial['\t']=='t' );
  ପ୍ରତିଜ୍ଞା( c>=0 && c<ଆକାର(aSpecial) );
  ପ୍ରତିଜ୍ଞା( p->nUsed+7 <= p->nAlloc );
  ଯଦି( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ jsonAppendString(JsonString *p, ଅଚଳ ଅକ୍ଷର *zIn, u32 N){
  u32 k;
  u8 c;
  ଅଚଳ u8 *z = (ଅଚଳ u8*)zIn;
  ଯଦି( z==0 ) ଫେରନ୍ତୁ;
  ଯଦି( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) ଫେରନ୍ତୁ;
  p->zBuf[p->nUsed++] = '"';
  ଯେତେବେଳେ( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    ଯେତେବେଳେ( 1 /* Exit by break */ ){
      ଯଦି( k+3>=N ){
        ଯେତେବେଳେ( k<N && jsonIsOk[z[k]] ){ k++; }
        ଭାଙ୍ଗ;
      }
      ଯଦି( !jsonIsOk[z[k]] ){
        ଭାଙ୍ଗ;
      }
      ଯଦି( !jsonIsOk[z[k+1]] ){
        k += 1;
        ଭାଙ୍ଗ;
      }
      ଯଦି( !jsonIsOk[z[k+2]] ){
        k += 2;
        ଭାଙ୍ଗ;
      }
      ଯଦି( !jsonIsOk[z[k+3]] ){
        k += 3;
        ଭାଙ୍ଗ;
      }ଅନ୍ୟଥା{
        k += 4;
      }
    }
    ଯଦି( k>=N ){
      ଯଦି( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      ଭାଙ୍ଗ;
    }
    ଯଦି( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    ଯଦି( c=='"' || c=='\\' ){
      ଯଦି( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) ଫେରନ୍ତୁ;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }ଅନ୍ୟଥା ଯଦି( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }ଅନ୍ୟଥା{
      ଯଦି( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) ଫେରନ୍ତୁ;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  ପ୍ରତିଜ୍ଞା( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
ସ୍ଥିର ଶୂନ୍ୟ json_Append_Sql_ମୂଲ୍ୟ(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_ମୂଲ୍ୟ *p_ମୂଲ୍ୟ          /* Value to append */
){
  ବାଛ( sqlite3_ମୂଲ୍ୟ_type(p_ମୂଲ୍ୟ) ){
    ସ୍ଥିତି SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_ମୂଲ୍ୟ_double(p_ମୂଲ୍ୟ));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_INTEGER: {
      ଅଚଳ ଅକ୍ଷର *z = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(p_ମୂଲ୍ୟ);
      u32 n = (u32)sqlite3_ମୂଲ୍ୟ_bytes(p_ମୂଲ୍ୟ);
      jsonAppendRaw(p, z, n);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_TEXT: {
      ଅଚଳ ଅକ୍ଷର *z = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(p_ମୂଲ୍ୟ);
      u32 n = (u32)sqlite3_ମୂଲ୍ୟ_bytes(p_ମୂଲ୍ୟ);
      ଯଦି( sqlite3_ମୂଲ୍ୟ_subtype(p_ମୂଲ୍ୟ)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }ଅନ୍ୟଥା{
        jsonAppendString(p, z, n);
      }
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      JsonParse px;
      memset(&px, 0, ଆକାର(px));
      ଯଦି( jsonArgIsJsonb(p_ମୂଲ୍ୟ, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }ଅନ୍ୟଥା ଯଦି( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      ଭାଙ୍ଗ;
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
ସ୍ଥିର ଶୂନ୍ୟ jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  ପ୍ରତିଜ୍ଞା( (pParse!=0)==(ctx!=0) );
  ପ୍ରତିଜ୍ଞା( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  ଯଦି( p->eErr==0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    ଯଦି( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }ଅନ୍ୟଥା ଯଦି( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }ଅନ୍ୟଥା{
      ଯଦି( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_ଭର୍ତ୍ତି(ctx, pParse);
        ଯଦି( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          ଫେରନ୍ତୁ;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }ଅନ୍ୟଥା ଯଦି( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }ଅନ୍ୟଥା ଯଦି( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }ଅନ୍ୟଥା ଯଦି( p->eErr & JSTRING_MALFORMED ){
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
ସ୍ଥିର ଶୂନ୍ୟ jsonParseReset(JsonParse *pParse){
  ପ୍ରତିଜ୍ଞା( pParse->nJPRef<=1 );
  ଯଦି( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  ଯଦି( pParse->nBlobAlloc ){
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
ସ୍ଥିର ଶୂନ୍ୟ jsonParseFree(JsonParse *pParse){
  ଯଦି( pParse ){
    ଯଦି( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }ଅନ୍ୟଥା{
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
ସ୍ଥିର u8 jsonHexToInt(ପୂର୍ଣ୍ଣସଂଖ୍ୟା h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  ଫେରନ୍ତୁ (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
ସ୍ଥିର u32 jsonHexToInt4(ଅଚଳ ଅକ୍ଷର *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  ଫେରନ୍ତୁ v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonIs2Hex(ଅଚଳ ଅକ୍ଷର *z){
  ଫେରନ୍ତୁ sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonIs4Hex(ଅଚଳ ଅକ୍ଷର *z){
  ଫେରନ୍ତୁ jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା json5Whitespace(ଅଚଳ ଅକ୍ଷର *zIn){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
  ଅଚଳ u8 *z = (u8*)zIn;
  ଯେତେବେଳେ( 1 /*exit by "goto whitespace_done"*/ ){
    ବାଛ( z[n] ){
      ସ୍ଥିତି 0x09:
      ସ୍ଥିତି 0x0a:
      ସ୍ଥିତି 0x0b:
      ସ୍ଥିତି 0x0c:
      ସ୍ଥିତି 0x0d:
      ସ୍ଥିତି 0x20: {
        n++;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '/': {
        ଯଦି( z[n+1]=='*' && z[n+2]!=0 ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
          ପାଇଁ(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            ଯଦି( z[j]==0 ) ଯାଅ whitespace_done;
          }
          n = j+1;
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା ଯଦି( z[n+1]=='/' ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
          ଅକ୍ଷର c;
          ପାଇଁ(j=n+2; (c = z[j])!=0; j++){
            ଯଦି( c=='\n' || c=='\r' ) ଭାଙ୍ଗ;
            ଯଦି( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              ଭାଙ୍ଗ;
            }
          }
          n = j;
          ଯଦି( z[n] ) n++;
          ଭାଙ୍ଗ;
        }
        ଯାଅ whitespace_done;
      }
      ସ୍ଥିତି 0xc2: {
        ଯଦି( z[n+1]==0xa0 ){
          n += 2;
          ଭାଙ୍ଗ;
        }
        ଯାଅ whitespace_done;
      }
      ସ୍ଥିତି 0xe1: {
        ଯଦି( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          ଭାଙ୍ଗ;
        }
        ଯାଅ whitespace_done;
      }
      ସ୍ଥିତି 0xe2: {
        ଯଦି( z[n+1]==0x80 ){
          u8 c = z[n+2];
          ଯଦି( c<0x80 ) ଯାଅ whitespace_done;
          ଯଦି( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            ଭାଙ୍ଗ;
          }
        }ଅନ୍ୟଥା ଯଦି( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          ଭାଙ୍ଗ;
        }
        ଯାଅ whitespace_done;
      }
      ସ୍ଥିତି 0xe3: {
        ଯଦି( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          ଭାଙ୍ଗ;
        }
        ଯାଅ whitespace_done;
      }
      ସ୍ଥିତି 0xef: {
        ଯଦି( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          ଭାଙ୍ଗ;
        }
        ଯାଅ whitespace_done;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        ଯାଅ whitespace_done;
      }
    }
  }
  whitespace_done:
  ଫେରନ୍ତୁ n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
ସ୍ଥିର ଅଚଳ ସଂରଚନା NanInfName {
  ଅକ୍ଷର c1;
  ଅକ୍ଷର c2;
  ଅକ୍ଷର n;
  ଅକ୍ଷର eType;
  ଅକ୍ଷର nRepl;
  ଅକ୍ଷର *zMatch;
  ଅକ୍ଷର *zRepl;
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
ସ୍ଥିର ଶୂନ୍ୟ json_Wrong_ଅଙ୍କ_Args(
  sqlite3_context *pCtx,
  ଅଚଳ ଅକ୍ଷର *zFuncName
){
  ଅକ୍ଷର *z_ସନ୍ଦେଶ = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, z_ସନ୍ଦେଶ, -1);
  sqlite3_free(z_ସନ୍ଦେଶ);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  ପ୍ରତିଜ୍ଞା( N>pParse->nBlobAlloc );
  ଯଦି( pParse->nBlobAlloc==0 ){
    t = 100;
  }ଅନ୍ୟଥା{
    t = pParse->nBlobAlloc*2;
  }
  ଯଦି( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  ଯଦି( aNew==0 ){ pParse->oom = 1; ଫେରନ୍ତୁ 1; }
  ପ୍ରତିଜ୍ଞା( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  ଫେରନ୍ତୁ 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 nSize;
  ପ୍ରତିଜ୍ଞା( !pParse->bReadOnly );
  ଯଦି( pParse->oom ) ଫେରନ୍ତୁ 0;
  ଯଦି( pParse->nBlobAlloc>0 ) ଫେରନ୍ତୁ 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  ଯଦି( jsonBlobExpand(pParse, nSize) ){
    ଫେରନ୍ତୁ 0;
  }
  ପ୍ରତିଜ୍ଞା( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  ଫେରନ୍ତୁ 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  ଯଦି( pParse->oom==0 ){
    ପ୍ରତିଜ୍ଞା( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  ଯଦି( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }ଅନ୍ୟଥା{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
ସ୍ଥିର ଶୂନ୍ୟ json_Blob_Append_ଗଣ୍ଠି(JsonParse*,u8,u64,ଅଚଳ ଶୂନ୍ୟ*);
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ json_Blob_Expand_And_Append_ଗଣ୍ଠି(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ଅଚଳ ଶୂନ୍ୟ *aPayload
){
  ଯଦି( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) ଫେରନ୍ତୁ;
  json_Blob_Append_ଗଣ୍ଠି(pParse, eType, szPayload, aPayload);
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
ସ୍ଥିର ଶୂନ୍ୟ json_Blob_Append_ଗଣ୍ଠି(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  ଅଚଳ ଶୂନ୍ୟ *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  ଯଦି( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_ଗଣ୍ଠି(pParse,eType,szPayload,aPayload);
    ଫେରନ୍ତୁ;
  }
  ପ୍ରତିଜ୍ଞା( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  ଯଦି( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }ଅନ୍ୟଥା ଯଦି( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }ଅନ୍ୟଥା ଯଦି( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }ଅନ୍ୟଥା{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  ଯଦି( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା delta;
  ଯଦି( pParse->oom ) ଫେରନ୍ତୁ 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  ଯଦି( szType<=11 ){
    nExtra = 0;
  }ଅନ୍ୟଥା ଯଦି( szType==12 ){
    nExtra = 1;
  }ଅନ୍ୟଥା ଯଦି( szType==13 ){
    nExtra = 2;
  }ଅନ୍ୟଥା ଯଦି( szType==14 ){
    nExtra = 4;
  }ଅନ୍ୟଥା{
    nExtra = 8;
  }
  ଯଦି( szPayload<=11 ){
    nNeeded = 0;
  }ଅନ୍ୟଥା ଯଦି( szPayload<=0xff ){
    nNeeded = 1;
  }ଅନ୍ୟଥା ଯଦି( szPayload<=0xffff ){
    nNeeded = 2;
  }ଅନ୍ୟଥା{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  ଯଦି( delta ){
    u64 newSize = (u64)pParse->nBlob + delta;
    ଯଦି( delta>0 ){
      ଯଦି( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        ଫେରନ୍ତୁ 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }ଅନ୍ୟଥା{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  ଯଦି( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }ଅନ୍ୟଥା ଯଦି( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }ଅନ୍ୟଥା ଯଦି( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }ଅନ୍ୟଥା{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  ଫେରନ୍ତୁ delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonIs4HexB(ଅଚଳ ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOp){
  ଯଦି( z[0]!='u' ) ଫେରନ୍ତୁ 0;
  ଯଦି( !jsonIs4Hex(&z[1]) ) ଫେରନ୍ତୁ 0;
  *pOp = JSONB_TEXTJ;
  ଫେରନ୍ତୁ 1;
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
ସ୍ଥିର u32 jsonbValidityCheck(
  ଅଚଳ JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ଅଚଳ u8 *z;
  u8 x;
  ଯଦି( iDepth>JSON_MAX_DEPTH ) ଫେରନ୍ତୁ i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  ଯଦି( NEVER(n==0) ) ଫେରନ୍ତୁ i+1;          /* Checked by caller */
  ଯଦି( NEVER(i+n+sz!=iEnd) ) ଫେରନ୍ତୁ i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  ବାଛ( x ){
    ସ୍ଥିତି JSONB_NULL:
    ସ୍ଥିତି JSONB_TRUE:
    ସ୍ଥିତି JSONB_FALSE: {
      ଫେରନ୍ତୁ n+sz==1 ? 0 : i+1;
    }
    ସ୍ଥିତି JSONB_INT: {
      ଯଦି( sz<1 ) ଫେରନ୍ତୁ i+1;
      j = i+n;
      ଯଦି( z[j]=='-' ){
        j++;
        ଯଦି( sz<2 ) ଫେରନ୍ତୁ i+1;
      }
      k = i+n+sz;
      ଯେତେବେଳେ( j<k ){
        ଯଦି( sqlite3Isdigit(z[j]) ){
          j++;
        }ଅନ୍ୟଥା{
          ଫେରନ୍ତୁ j+1;
        }
      }
      ଫେରନ୍ତୁ 0;
    }
    ସ୍ଥିତି JSONB_INT5: {
      ଯଦି( sz<3 ) ଫେରନ୍ତୁ i+1;
      j = i+n;
      ଯଦି( z[j]=='-' ){
        ଯଦି( sz<4 ) ଫେରନ୍ତୁ i+1;
        j++;
      }
      ଯଦି( z[j]!='0' ) ଫେରନ୍ତୁ i+1;
      ଯଦି( z[j+1]!='x' && z[j+1]!='X' ) ଫେରନ୍ତୁ j+2;
      j += 2;
      k = i+n+sz;
      ଯେତେବେଳେ( j<k ){
        ଯଦି( sqlite3Isxdigit(z[j]) ){
          j++;
        }ଅନ୍ୟଥା{
          ଫେରନ୍ତୁ j+1;
        }
      }
      ଫେରନ୍ତୁ 0;
    }
    ସ୍ଥିତି JSONB_FLOAT:
    ସ୍ଥିତି JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      ଯଦି( sz<2 ) ଫେରନ୍ତୁ i+1;
      j = i+n;
      k = j+sz;
      ଯଦି( z[j]=='-' ){
        j++;
        ଯଦି( sz<3 ) ଫେରନ୍ତୁ i+1;
      }
      ଯଦି( z[j]=='.' ){
        ଯଦି( x==JSONB_FLOAT ) ଫେରନ୍ତୁ j+1;
        ଯଦି( !sqlite3Isdigit(z[j+1]) ) ଫେରନ୍ତୁ j+1;
        j += 2;
        seen = 1;
      }ଅନ୍ୟଥା ଯଦି( z[j]=='0' && x==JSONB_FLOAT ){
        ଯଦି( j+3>k ) ଫେରନ୍ତୁ j+1;
        ଯଦି( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) ଫେରନ୍ତୁ j+1;
        j++;
      }
      ପାଇଁ(; j<k; j++){
        ଯଦି( sqlite3Isdigit(z[j]) ) ଜାରି;
        ଯଦି( z[j]=='.' ){
          ଯଦି( seen>0 ) ଫେରନ୍ତୁ j+1;
          ଯଦି( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            ଫେରନ୍ତୁ j+1;
          }
          seen = 1;
          ଜାରି;
        }
        ଯଦି( z[j]=='e' || z[j]=='E' ){
          ଯଦି( seen==2 ) ଫେରନ୍ତୁ j+1;
          ଯଦି( j==k-1 ) ଫେରନ୍ତୁ j+1;
          ଯଦି( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            ଯଦି( j==k-1 ) ଫେରନ୍ତୁ j+1;
          }
          seen = 2;
          ଜାରି;
        }
        ଫେରନ୍ତୁ j+1;
      }
      ଯଦି( seen==0 ) ଫେରନ୍ତୁ i+1;
      ଫେରନ୍ତୁ 0;
    }
    ସ୍ଥିତି JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      ଯେତେବେଳେ( j<k ){
        ଯଦି( !jsonIsOk[z[j]] && z[j]!='\'' ) ଫେରନ୍ତୁ j+1;
        j++;
      }
      ଫେରନ୍ତୁ 0;
    }
    ସ୍ଥିତି JSONB_TEXTJ:
    ସ୍ଥିତି JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      ଯେତେବେଳେ( j<k ){
        ଯଦି( !jsonIsOk[z[j]] && z[j]!='\'' ){
          ଯଦି( z[j]=='"' ){
            ଯଦି( x==JSONB_TEXTJ ) ଫେରନ୍ତୁ j+1;
          }ଅନ୍ୟଥା ଯଦି( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            ଯଦି( x==JSONB_TEXTJ ) ଫେରନ୍ତୁ j+1;
          }ଅନ୍ୟଥା ଯଦି( NEVER(z[j]!='\\') || j+1>=k ){
            ଫେରନ୍ତୁ j+1;
          }ଅନ୍ୟଥା ଯଦି( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }ଅନ୍ୟଥା ଯଦି( z[j+1]=='u' ){
            ଯଦି( j+5>=k ) ଫେରନ୍ତୁ j+1;
            ଯଦି( !jsonIs4Hex((ଅଚଳ ଅକ୍ଷର*)&z[j+2]) ) ଫେରନ୍ତୁ j+1;
            j++;
          }ଅନ୍ୟଥା ଯଦି( x!=JSONB_TEXT5 ){
            ଫେରନ୍ତୁ j+1;
          }ଅନ୍ୟଥା{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((ଅଚଳ ଅକ୍ଷର*)&z[j], k-j, &c);
            ଯଦି( c==JSON_INVALID_CHAR ) ଫେରନ୍ତୁ j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      ଫେରନ୍ତୁ 0;
    }
    ସ୍ଥିତି JSONB_TEXTRAW: {
      ଫେରନ୍ତୁ 0;
    }
    ସ୍ଥିତି JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      ଯେତେବେଳେ( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        ଯଦି( n==0 ) ଫେରନ୍ତୁ j+1;
        ଯଦି( j+n+sz>k ) ଫେରନ୍ତୁ j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        ଯଦି( sub ) ଫେରନ୍ତୁ sub;
        j += n + sz;
      }
      ପ୍ରତିଜ୍ଞା( j==k );
      ଫେରନ୍ତୁ 0;
    }
    ସ୍ଥିତି JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      ଯେତେବେଳେ( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        ଯଦି( n==0 ) ଫେରନ୍ତୁ j+1;
        ଯଦି( j+n+sz>k ) ଫେରନ୍ତୁ j+1;
        ଯଦି( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          ଯଦି( x<JSONB_TEXT || x>JSONB_TEXTRAW ) ଫେରନ୍ତୁ j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        ଯଦି( sub ) ଫେରନ୍ତୁ sub;
        cnt++;
        j += n + sz;
      }
      ପ୍ରତିଜ୍ଞା( j==k );
      ଯଦି( (cnt & 1)!=0 ) ଫେରନ୍ତୁ j+1;
      ଫେରନ୍ତୁ 0;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ଫେରନ୍ତୁ i+1;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  ଅକ୍ଷର c;
  u32 j;
  u32 iThis, iStart;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
  u8 t;
  ଅଚଳ ଅକ୍ଷର *z = pParse->zJson;
json_parse_restart:
  ବାଛ( (u8)z[i] ){
  ସ୍ଥିତି '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    ଯଦି( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      ଫେରନ୍ତୁ -1;
    }
    iStart = pParse->nBlob;
    ପାଇଁ(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      ଯଦି( x<=0 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
        ଯଦି( x==(-2) ){
          j = pParse->iErr;
          ଯଦି( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          ଭାଙ୍ଗ;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        ଯଦି( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = j+1;
          ଯେତେବେଳେ( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          ପ୍ରତିଜ୍ଞା( iBlob==pParse->nBlob );
          json_Blob_Append_ଗଣ୍ଠି(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }ଅନ୍ୟଥା{
          ଯଦି( x!=-1 ) pParse->iErr = j;
          ଫେରନ୍ତୁ -1;
        }
      }
      ଯଦି( pParse->oom ) ଫେରନ୍ତୁ -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      ଯଦି( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        ଫେରନ୍ତୁ -1;
      }
      j = x;
      ଯଦି( z[j]==':' ){
        j++;
      }ଅନ୍ୟଥା{
        ଯଦି( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          କର{ j++; }ଯେତେବେଳେ( jsonIsspace(z[j]) );
          ଯଦି( z[j]==':' ){
            j++;
            ଯାଅ parse_object_ମୂଲ୍ୟ;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ଯଦି( x!=(-5) ){
          ଯଦି( x!=(-1) ) pParse->iErr = j;
          ଫେରନ୍ତୁ -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_ମୂଲ୍ୟ:
      x = jsonTranslateTextToBlob(pParse, j);
      ଯଦି( x<=0 ){
        ଯଦି( x!=(-1) ) pParse->iErr = j;
        ଫେରନ୍ତୁ -1;
      }
      j = x;
      ଯଦି( z[j]==',' ){
        ଜାରି;
      }ଅନ୍ୟଥା ଯଦି( z[j]=='}' ){
        ଭାଙ୍ଗ;
      }ଅନ୍ୟଥା{
        ଯଦି( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ଯଦି( z[j]==',' ){
            ଜାରି;
          }ଅନ୍ୟଥା ଯଦି( z[j]=='}' ){
            ଭାଙ୍ଗ;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ଯଦି( x==(-4) ){
          j = pParse->iErr;
          ଜାରି;
        }
        ଯଦି( x==(-2) ){
          j = pParse->iErr;
          ଭାଙ୍ଗ;
        }
      }
      pParse->iErr = j;
      ଫେରନ୍ତୁ -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    ଫେରନ୍ତୁ j+1;
  }
  ସ୍ଥିତି '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    ପ୍ରତିଜ୍ଞା( i<=(u32)pParse->nJson );
    json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    ଯଦି( pParse->oom ) ଫେରନ୍ତୁ -1;
    ଯଦି( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      ଫେରନ୍ତୁ -1;
    }
    ପାଇଁ(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      ଯଦି( x<=0 ){
        ଯଦି( x==(-3) ){
          j = pParse->iErr;
          ଯଦି( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          ଭାଙ୍ଗ;
        }
        ଯଦି( x!=(-1) ) pParse->iErr = j;
        ଫେରନ୍ତୁ -1;
      }
      j = x;
      ଯଦି( z[j]==',' ){
        ଜାରି;
      }ଅନ୍ୟଥା ଯଦି( z[j]==']' ){
        ଭାଙ୍ଗ;
      }ଅନ୍ୟଥା{
        ଯଦି( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ଯଦି( z[j]==',' ){
            ଜାରି;
          }ଅନ୍ୟଥା ଯଦି( z[j]==']' ){
            ଭାଙ୍ଗ;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ଯଦି( x==(-4) ){
          j = pParse->iErr;
          ଜାରି;
        }
        ଯଦି( x==(-3) ){
          j = pParse->iErr;
          ଭାଙ୍ଗ;
        }
      }
      pParse->iErr = j;
      ଫେରନ୍ତୁ -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    ଫେରନ୍ତୁ j+1;
  }
  ସ୍ଥିତି '\'': {
    u8 opcode;
    ଅକ୍ଷର cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    ଯାଅ parse_string;
  ସ୍ଥିତି '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    ଯେତେବେଳେ( 1 /*exit-by-break*/ ){
      ଯଦି( jsonIsOk[(u8)z[j]] ){
        ଯଦି( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }ଅନ୍ୟଥା ଯଦି( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }ଅନ୍ୟଥା{
          j += 3;
          ଜାରି;
        }
      }
      c = z[j];
      ଯଦି( c==cDelim ){
        ଭାଙ୍ଗ;
      }ଅନ୍ୟଥା ଯଦି( c=='\\' ){
        c = z[++j];
        ଯଦି( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          ଯଦି( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }ଅନ୍ୟଥା ଯଦି( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#ଅନ୍ୟଥା
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ଅନ୍ୟଥା ଯଦି( c=='\r' ){
          ଯଦି( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ଅନ୍ୟଥା{
          pParse->iErr = j;
          ଫେରନ୍ତୁ -1;
        }
      }ଅନ୍ୟଥା ଯଦି( c<=0x1f ){
        ଯଦି( c==0 ){
          pParse->iErr = j;
          ଫେରନ୍ତୁ -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }ଅନ୍ୟଥା ଯଦି( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_ଗଣ୍ଠି(pParse, opcode, j-1-i, &z[i+1]);
    ଫେରନ୍ତୁ j+1;
  }
  ସ୍ଥିତି 't': {
    ଯଦି( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      ଫେରନ୍ତୁ i+4;
    }
    pParse->iErr = i;
    ଫେରନ୍ତୁ -1;
  }
  ସ୍ଥିତି 'f': {
    ଯଦି( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      ଫେରନ୍ତୁ i+5;
    }
    pParse->iErr = i;
    ଫେରନ୍ତୁ -1;
  }
  ସ୍ଥିତି '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    ଯାଅ parse_ସଂଖ୍ୟା;
  ସ୍ଥିତି '.':
    ଯଦି( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      ଯାଅ parse_ସଂଖ୍ୟା_2;
    }
    pParse->iErr = i;
    ଫେରନ୍ତୁ -1;
  ସ୍ଥିତି '-':
  ସ୍ଥିତି '0':
  ସ୍ଥିତି '1':
  ସ୍ଥିତି '2':
  ସ୍ଥିତି '3':
  ସ୍ଥିତି '4':
  ସ୍ଥିତି '5':
  ସ୍ଥିତି '6':
  ସ୍ଥିତି '7':
  ସ୍ଥିତି '8':
  ସ୍ଥିତି '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_ସଂଖ୍ୟା:
    seenE = 0;
    ପ୍ରତିଜ୍ଞା( '-' < '0' );
    ପ୍ରତିଜ୍ଞା( '+' < '0' );
    ପ୍ରତିଜ୍ଞା( '.' < '0' );
    c = z[i];

    ଯଦି( c<='0' ){
      ଯଦି( c=='0' ){
        ଯଦି( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          ପ୍ରତିଜ୍ଞା( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          ପାଇଁ(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          ଯାଅ parse_ସଂଖ୍ୟା_finish;
        }ଅନ୍ୟଥା ଯଦି( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          ଫେରନ୍ତୁ -1;
        }
      }ଅନ୍ୟଥା{
        ଯଦି( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          ଯଦି( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            ଯଦି( z[i]=='-' ){
              json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_FLOAT, 6, "-9e999");
            }ଅନ୍ୟଥା{
              json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_FLOAT, 5, "9e999");
            }
            ଫେରନ୍ତୁ i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          ଯଦି( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            ଯାଅ parse_ସଂଖ୍ୟା_2;
          }
          pParse->iErr = i;
          ଫେରନ୍ତୁ -1;
        }
        ଯଦି( z[i+1]=='0' ){
          ଯଦି( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            ଫେରନ୍ତୁ -1;
          }ଅନ୍ୟଥା ଯଦି( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            ପାଇଁ(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            ଯାଅ parse_ସଂଖ୍ୟା_finish;
          }
        }
      }
    }

  parse_ସଂଖ୍ୟା_2:
    ପାଇଁ(j=i+1;; j++){
      c = z[j];
      ଯଦି( sqlite3Isdigit(c) ) ଜାରି;
      ଯଦି( c=='.' ){
        ଯଦି( (t & 0x02)!=0 ){
          pParse->iErr = j;
          ଫେରନ୍ତୁ -1;
        }
        t |= 0x02;
        ଜାରି;
      }
      ଯଦି( c=='e' || c=='E' ){
        ଯଦି( z[j-1]<'0' ){
          ଯଦି( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }ଅନ୍ୟଥା{
            pParse->iErr = j;
            ଫେରନ୍ତୁ -1;
          }
        }
        ଯଦି( seenE ){
          pParse->iErr = j;
          ଫେରନ୍ତୁ -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        ଯଦି( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        ଯଦି( c<'0' || c>'9' ){
          pParse->iErr = j;
          ଫେରନ୍ତୁ -1;
        }
        ଜାରି;
      }
      ଭାଙ୍ଗ;
    }
    ଯଦି( z[j-1]<'0' ){
      ଯଦି( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }ଅନ୍ୟଥା{
        pParse->iErr = j;
        ଫେରନ୍ତୁ -1;
      }
    }
  parse_ସଂଖ୍ୟା_finish:
    ପ୍ରତିଜ୍ଞା( JSONB_INT+0x01==JSONB_INT5 );
    ପ୍ରତିଜ୍ଞା( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    ପ୍ରତିଜ୍ଞା( JSONB_INT+0x02==JSONB_FLOAT );
    ଯଦି( z[i]=='+' ) i++;
    json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_INT+t, j-i, &z[i]);
    ଫେରନ୍ତୁ j;
  }
  ସ୍ଥିତି '}': {
    pParse->iErr = i;
    ଫେରନ୍ତୁ -2;  /* End of {...} */
  }
  ସ୍ଥିତି ']': {
    pParse->iErr = i;
    ଫେରନ୍ତୁ -3;  /* End of [...] */
  }
  ସ୍ଥିତି ',': {
    pParse->iErr = i;
    ଫେରନ୍ତୁ -4;  /* List separator */
  }
  ସ୍ଥିତି ':': {
    pParse->iErr = i;
    ଫେରନ୍ତୁ -5;  /* Object label/value separator */
  }
  ସ୍ଥିତି 0: {
    ଫେରନ୍ତୁ 0;   /* End of file */
  }
  ସ୍ଥିତି 0x09:
  ସ୍ଥିତି 0x0a:
  ସ୍ଥିତି 0x0d:
  ସ୍ଥିତି 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    ଯାଅ json_parse_restart;
  }
  ସ୍ଥିତି 0x0b:
  ସ୍ଥିତି 0x0c:
  ସ୍ଥିତି '/':
  ସ୍ଥିତି 0xc2:
  ସ୍ଥିତି 0xe1:
  ସ୍ଥିତି 0xe2:
  ସ୍ଥିତି 0xe3:
  ସ୍ଥିତି 0xef: {
    j = json5Whitespace(&z[i]);
    ଯଦି( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      ଯାଅ json_parse_restart;
    }
    pParse->iErr = i;
    ଫେରନ୍ତୁ -1;
  }
  ସ୍ଥିତି 'n': {
    ଯଦି( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      ଫେରନ୍ତୁ i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
    u32 k;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nn;
    c = z[i];
    ପାଇଁ(k=0; k<ଆକାର(aNanInfName)/ଆକାର(aNanInfName[0]); k++){
      ଯଦି( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) ଜାରି;
      nn = aNanInfName[k].n;
      ଯଦି( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        ଜାରି;
      }
      ଯଦି( sqlite3Isalnum(z[i+nn]) ) ଜାରି;
      ଯଦି( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_FLOAT, 5, "9e999");
      }ଅନ୍ୟଥା{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      ଫେରନ୍ତୁ i + nn;
    }
    pParse->iErr = i;
    ଫେରନ୍ତୁ -1;  /* Syntax error */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅଚଳ ଅକ୍ଷର *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  ଯଦି( pParse->oom ) i = -1;
  ଯଦି( i>0 ){
#ifdef SQLITE_DEBUG
    ପ୍ରତିଜ୍ଞା( pParse->iDepth==0 );
    ଯଦି( sqlite3Config.bJsonSelfcheck ){
      ପ୍ରତିଜ୍ଞା( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    ଯେତେବେଳେ( jsonIsspace(zJson[i]) ) i++;
    ଯଦି( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      ଯଦି( zJson[i] ){
        ଯଦି( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        ଫେରନ୍ତୁ 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  ଯଦି( i<=0 ){
    ଯଦି( pCtx!=0 ){
      ଯଦି( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }ଅନ୍ୟଥା{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  ପ୍ରତିଜ୍ଞା( pStr->eErr==0 );
  memset(&px, 0, ଆକାର(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (ଶୂନ୍ୟ)jsonTranslateTextToBlob(&px, 0);
  ଯଦି( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( px.nBlobAlloc>0 );
    ପ୍ରତିଜ୍ଞା( !px.bReadOnly );
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
ସ୍ଥିର u32 jsonbPayloadSizeWide(
  ଅଚଳ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  ପ୍ରତିଜ୍ଞା( i<pParse->nBlob );
  ପ୍ରତିଜ୍ଞା( x==(pParse->aBlob[i]>>4) );
  ପ୍ରତିଜ୍ଞା( x>=13 );
  ଯଦି( x==13 ){
    ଯଦି( i+2>=pParse->nBlob ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ଅନ୍ୟଥା ଯଦି( x==14 ){
    ଯଦି( i+4>=pParse->nBlob ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ଅନ୍ୟଥା{
    ଯଦି( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
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
  ଯଦି( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    ଫେରନ୍ତୁ 0;
  }
  *pSz = sz;
  ଫେରନ୍ତୁ n;
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
ସ୍ଥିର u32 jsonbPayloadSize(
  ଅଚଳ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ଯଦି( i>=pParse->nBlob ){
    *pSz = 0;
    ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା ଯଦି( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ଅନ୍ୟଥା ଯଦି( x==12 ){
    ଯଦି( i+1>=pParse->nBlob ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ jsonbPayloadSizeWide(pParse, i, pSz, x);
  }
  ଯଦି( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ଫେରନ୍ତୁ 0;
  }
  *pSz = sz;
  ଫେରନ୍ତୁ n;
}

#ଯଦି SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
ସ୍ଥିର SQLITE_INLINE u32 jsonbPayloadSizeInline(
  ଅଚଳ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ଯଦି( i>=pParse->nBlob ){
    *pSz = 0;
    ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା ଯଦି( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ଅନ୍ୟଥା ଯଦି( x==12 ){
    ଯଦି( i+1>=pParse->nBlob ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ଅନ୍ୟଥା ଯଦି( x==13 ){
    ଯଦି( i+2>=pParse->nBlob ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ଅନ୍ୟଥା ଯଦି( x==14 ){
    ଯଦି( i+4>=pParse->nBlob ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ଅନ୍ୟଥା{
    ଯଦି( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  ଯଦି( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ଫେରନ୍ତୁ 0;
  }
  *pSz = sz;
  ଫେରନ୍ତୁ n;
}
#ଅନ୍ୟଥା /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif


#ଯଦି SQLITE_USES_INLINE
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
ସ୍ଥିର SQLITE_INLINE u32 jsonbPayloadSizeSemiInline(
  ଅଚଳ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ପ୍ରତିଜ୍ଞା( i<pParse->nBlob );
  ଯଦି( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ଅନ୍ୟଥା ଯଦି( x==12 ){
    ଯଦି( i+1>=pParse->nBlob ){
      *pSz = 0;
      ଫେରନ୍ତୁ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ jsonbPayloadSizeWide(pParse,i,pSz,x);
  }
  ଯଦି( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ଫେରନ୍ତୁ 0;
  }
  *pSz = sz;
  ଫେରନ୍ତୁ n;
}
#ଅନ୍ୟଥା /* if !SQLITE_USES_INLINE */
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
ସ୍ଥିର u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSizeInline(pParse, i, &sz);
  ଯଦି( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    ଫେରନ୍ତୁ pParse->nBlob+1;
  }
  ବାଛ( pParse->aBlob[i] & 0x0f ){
    ସ୍ଥିତି JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      ଫେରନ୍ତୁ i+1;
    }
    ସ୍ଥିତି JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      ଫେରନ୍ତୁ i+1;
    }
    ସ୍ଥିତି JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      ଫେରନ୍ତୁ i+1;
    }
    ସ୍ଥିତି JSONB_INT:
    ସ୍ଥିତି JSONB_FLOAT: {
      ଯଦି( sz==0 ) ଯାଅ malformed_jsonb;
      jsonAppendRaw(pOut, (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n], sz);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ଅଚଳ ଅକ୍ଷର *zIn = (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bOverflow = 0;
      ଯଦି( sz==0 ) ଯାଅ malformed_jsonb;
      ଯଦି( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }ଅନ୍ୟଥା ଯଦି( zIn[0]=='+' ){
        k++;
      }
      ପାଇଁ(; k<sz; k++){
        ଯଦି( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା ଯଦି( (u>>60)!=0 ){
          bOverflow = 1;
        }ଅନ୍ୟଥା{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ଅଚଳ ଅକ୍ଷର *zIn = (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n];
      ଯଦି( sz==0 ) ଯାଅ malformed_jsonb;
      ଯଦି( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        ଯଦି( sz<=1 ) ଯାଅ malformed_jsonb;
        k = 1;
      }
      ଯଦି( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      ପାଇଁ(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        ଯଦି( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_TEXT:
    ସ୍ଥିତି JSONB_TEXTJ: {
      ଯଦି( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_TEXT5: {
      ଅଚଳ ଅକ୍ଷର *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      ଯେତେବେଳେ( sz2>0 ){
        ପାଇଁ(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        ଯଦି( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          ଯଦି( k>=sz2 ){
            ଭାଙ୍ଗ;
          }
          zIn += k;
          sz2 -= k;
        }
        ଯଦି( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          ଜାରି;
        }
        ଯଦି( zIn[0]<=0x1f ){
          ଯଦି( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) ଭାଙ୍ଗ;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          ଜାରି;
        }
        ପ୍ରତିଜ୍ଞା( zIn[0]=='\\' );
        ପ୍ରତିଜ୍ଞା( sz2>=1 );
        ଯଦି( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          ଭାଙ୍ଗ;
        }
        ବାଛ( (u8)zIn[1] ){
          ସ୍ଥିତି '\'':
            jsonAppendChar(pOut, '\'');
            ଭାଙ୍ଗ;
          ସ୍ଥିତି 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            ଭାଙ୍ଗ;
          ସ୍ଥିତି 'x':
            ଯଦି( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ଭାଙ୍ଗ;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            ଭାଙ୍ଗ;
          ସ୍ଥିତି '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            ଭାଙ୍ଗ;
          ସ୍ଥିତି '\r':
            ଯଦି( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            ଭାଙ୍ଗ;
          ସ୍ଥିତି '\n':
            ଭାଙ୍ଗ;
          ସ୍ଥିତି 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            ଯଦି( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ଭାଙ୍ଗ;
            }
            zIn += 2;
            sz2 -= 2;
            ଭାଙ୍ଗ;
          ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            jsonAppendRawNZ(pOut, zIn, 2);
            ଭାଙ୍ଗ;
        }
        ପ୍ରତିଜ୍ଞା( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_TEXTRAW: {
      jsonAppendString(pOut, (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n], sz);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      ଯଦି( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      ଯେତେବେଳେ( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      ଯଦି( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      ଯଦି( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_OBJECT: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      ଯଦି( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      ଯେତେବେଳେ( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      ଯଦି( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      ଯଦି( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      ଭାଙ୍ଗ;
    }

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ i+n+sz;
}

/* Context for recursion of json_pretty()
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା JsonPretty JsonPretty;
ସଂରଚନା JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  ଅଚଳ ଅକ୍ଷର *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
ସ୍ଥିର ଶୂନ୍ୟ jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  ପାଇଁ(jj=0; jj<pPretty->nIndent; jj++){
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
ସ୍ଥିର u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  ଯଦି( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    ଫେରନ୍ତୁ pParse->nBlob+1;
  }
  ବାଛ( pParse->aBlob[i] & 0x0f ){
    ସ୍ଥିତି JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      ଯଦି( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        ଯଦି( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        ଯେତେବେଳେ( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          ଯଦି( j>=iEnd ){
            ଯଦି( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            ଭାଙ୍ଗ;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      ଯଦି( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        ଯଦି( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        ଯେତେବେଳେ( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          ଯଦି( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            ଭାଙ୍ଗ;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          ଯଦି( j>=iEnd ) ଭାଙ୍ଗ;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
ସ୍ଥିର u32 jsonb_Array_ଗଣନା(JsonParse *pParse, u32 i_ମୂଳ){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, i_ମୂଳ, &sz);
  iEnd = i_ମୂଳ+n+sz;
  ପାଇଁ(i=i_ମୂଳ+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSizeSemiInline(pParse, i, &sz);
  }
  ଫେରନ୍ତୁ k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonAfterEditSizeAdjust(JsonParse *pParse, u32 i_ମୂଳ){
  u32 sz = 0;
  u32 nBlob;
  ପ୍ରତିଜ୍ଞା( pParse->delta!=0 );
  ପ୍ରତିଜ୍ଞା( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (ଶୂନ୍ୟ)jsonbPayloadSize(pParse, i_ମୂଳ, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, i_ମୂଳ, sz);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  ଅଚଳ u8 *aIns,           /* New content */
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
  ସ୍ଥିର ଅଚଳ u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  ଯଦି( (aIns[0]&0x0f)<=2 ) ଫେରନ୍ତୁ 0;    /* Cannot enlarge NULL, true, false */
  ବାଛ( aIns[0]>>4 ){
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {                         /* aIns[] header size 1 */
      ଯଦି( ((1<<d)&0x116)==0 ) ଫେରନ୍ତୁ 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 12: {                         /* aIns[] header size is 2 */
      ଯଦି( ((1<<d)&0x8a)==0) ଫେରନ୍ତୁ 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 13: {                         /* aIns[] header size is 3 */
      ଯଦି( d!=2 && d!=6 ) ଫେରନ୍ତୁ 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 14: {                         /* aIns[] header size is 5 */
      ଯଦି( d!=4 ) ଫେରନ୍ତୁ 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 15: {                         /* aIns[] header size is 9 */
      ଫେରନ୍ତୁ 0;                          /* No solution */
    }
  }
  ପ୍ରତିଜ୍ଞା( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  ଯେତେବେଳେ( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    ଯଦି( i==1 ) ଭାଙ୍ଗ;
    szPayload >>= 8;
  }
  ପ୍ରତିଜ୍ଞା( (szPayload>>8)==0 );
  ଫେରନ୍ତୁ 1;
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
ସ୍ଥିର ଶୂନ୍ୟ jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  ଅଚଳ u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  ପ୍ରତିଜ୍ଞା( pParse->nBlob >= (u64)iDel + (u64)nDel );
  ଯଦି( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)-d)
  ){
    ଫେରନ୍ତୁ;
  }
  ଯଦି( d!=0 ){
    ଯଦି( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      ଯଦି( pParse->oom ) ଫେରନ୍ତୁ;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  ଯଦି( nIns && aIns ){
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
ସ୍ଥିର u32 jsonBytesToBypass(ଅଚଳ ଅକ୍ଷର *z, u32 n){
  u32 i = 0;
  ଯେତେବେଳେ( i+1<n ){
    ଯଦି( z[i]!='\\' ) ଫେରନ୍ତୁ i;
    ଯଦି( z[i+1]=='\n' ){
      i += 2;
      ଜାରି;
    }
    ଯଦି( z[i+1]=='\r' ){
      ଯଦି( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }ଅନ୍ୟଥା{
        i += 2;
      }
      ଜାରି;
    }
    ଯଦି( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      ଜାରି;
    }
    ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ i;
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
ସ୍ଥିର u32 jsonUnescapeOneChar(ଅଚଳ ଅକ୍ଷର *z, u32 n, u32 *piOut){
  ପ୍ରତିଜ୍ଞା( n>0 );
  ପ୍ରତିଜ୍ଞା( z[0]=='\\' );
  ଯଦି( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    ଫେରନ୍ତୁ n;
  }
  ବାଛ( (u8)z[1] ){
    ସ୍ଥିତି 'u': {
      u32 v, vlo;
      ଯଦି( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        ଫେରନ୍ତୁ n;
      }
      v = jsonHexToInt4(&z[2]);
      ଯଦି( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        ଫେରନ୍ତୁ 12;
      }ଅନ୍ୟଥା{
        *piOut = v;
        ଫେରନ୍ତୁ 6;
      }
    }
    ସ୍ଥିତି 'b': {   *piOut = '\b';  ଫେରନ୍ତୁ 2; }
    ସ୍ଥିତି 'f': {   *piOut = '\f';  ଫେରନ୍ତୁ 2; }
    ସ୍ଥିତି 'n': {   *piOut = '\n';  ଫେରନ୍ତୁ 2; }
    ସ୍ଥିତି 'r': {   *piOut = '\r';  ଫେରନ୍ତୁ 2; }
    ସ୍ଥିତି 't': {   *piOut = '\t';  ଫେରନ୍ତୁ 2; }
    ସ୍ଥିତି 'v': {   *piOut = '\v';  ଫେରନ୍ତୁ 2; }
    ସ୍ଥିତି '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#ଅନ୍ୟଥା
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      ଫେରନ୍ତୁ 2;
    }
    ସ୍ଥିତି '\'':
    ସ୍ଥିତି '"':
    ସ୍ଥିତି '/':
    ସ୍ଥିତି '\\':{   *piOut = z[1];  ଫେରନ୍ତୁ 2; }
    ସ୍ଥିତି 'x': {
      ଯଦି( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        ଫେରନ୍ତୁ n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      ଫେରନ୍ତୁ 4;
    }
    ସ୍ଥିତି 0xe2:
    ସ୍ଥିତି '\r':
    ସ୍ଥିତି '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      ଯଦି( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        ଫେରନ୍ତୁ n;
      }ଅନ୍ୟଥା ଯଦି( nSkip==n ){
        *piOut = 0;
        ଫେରନ୍ତୁ n;
      }ଅନ୍ୟଥା ଯଦି( z[nSkip]=='\\' ){
        ଫେରନ୍ତୁ nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        ଫେରନ୍ତୁ nSkip + sz;
      }
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      *piOut = JSON_INVALID_CHAR;
      ଫେରନ୍ତୁ 2;
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
ସ୍ଥିର SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonLabelCompareEscaped(
  ଅଚଳ ଅକ୍ଷର *z_ବାମ,          /* The left label */
  u32 n_ବାମ,                  /* Size of the left label in bytes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw_ବାମ,                /* True if zLeft contains no escapes */
  ଅଚଳ ଅକ୍ଷର *z_ଡାହାଣ,         /* The right label */
  u32 n_ଡାହାଣ,                 /* Size of the right label in bytes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw_ଡାହାଣ                /* True if zRight is escape-free */
){
  u32 c_ବାମ, c_ଡାହାଣ;
  ପ୍ରତିଜ୍ଞା( raw_ବାମ==0 || raw_ଡାହାଣ==0 );
  ଯେତେବେଳେ( 1 /*exit-by-return*/ ){
    ଯଦି( n_ବାମ==0 ){
      c_ବାମ = 0;
    }ଅନ୍ୟଥା ଯଦି( raw_ବାମ || z_ବାମ[0]!='\\' ){
      c_ବାମ = ((u8*)z_ବାମ)[0];
      ଯଦି( c_ବାମ>=0xc0 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = sqlite3Utf8ReadLimited((u8*)z_ବାମ, n_ବାମ, &c_ବାମ);
        z_ବାମ += sz;
        n_ବାମ -= sz;
      }ଅନ୍ୟଥା{
        z_ବାମ++;
        n_ବାମ--;
      }
    }ଅନ୍ୟଥା{
      u32 n = jsonUnescapeOneChar(z_ବାମ, n_ବାମ, &c_ବାମ);
      z_ବାମ += n;
      ପ୍ରତିଜ୍ଞା( n<=n_ବାମ );
      n_ବାମ -= n;
    }
    ଯଦି( n_ଡାହାଣ==0 ){
      c_ଡାହାଣ = 0;
    }ଅନ୍ୟଥା ଯଦି( raw_ଡାହାଣ || z_ଡାହାଣ[0]!='\\' ){
      c_ଡାହାଣ = ((u8*)z_ଡାହାଣ)[0];
      ଯଦି( c_ଡାହାଣ>=0xc0 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = sqlite3Utf8ReadLimited((u8*)z_ଡାହାଣ, n_ଡାହାଣ, &c_ଡାହାଣ);
        z_ଡାହାଣ += sz;
        n_ଡାହାଣ -= sz;
      }ଅନ୍ୟଥା{
        z_ଡାହାଣ++;
        n_ଡାହାଣ--;
      }
    }ଅନ୍ୟଥା{
      u32 n = jsonUnescapeOneChar(z_ଡାହାଣ, n_ଡାହାଣ, &c_ଡାହାଣ);
      z_ଡାହାଣ += n;
      ପ୍ରତିଜ୍ଞା( n<=n_ଡାହାଣ );
      n_ଡାହାଣ -= n;
    }
    ଯଦି( c_ବାମ!=c_ଡାହାଣ ) ଫେରନ୍ତୁ 0;
    ଯଦି( c_ବାମ==0 ) ଫେରନ୍ତୁ 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonLabelCompare(
  ଅଚଳ ଅକ୍ଷର *z_ବାମ,          /* The left label */
  u32 n_ବାମ,                  /* Size of the left label in bytes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw_ବାମ,                /* True if zLeft contains no escapes */
  ଅଚଳ ଅକ୍ଷର *z_ଡାହାଣ,         /* The right label */
  u32 n_ଡାହାଣ,                 /* Size of the right label in bytes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw_ଡାହାଣ                /* True if zRight is escape-free */
){
  ଯଦି( raw_ବାମ && raw_ଡାହାଣ ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    ଯଦି( n_ବାମ!=n_ଡାହାଣ ) ଫେରନ୍ତୁ 0;
    ଫେରନ୍ତୁ memcmp(z_ବାମ, z_ଡାହାଣ, n_ବାମ)==0;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ jsonLabelCompareEscaped(z_ବାମ, n_ବାମ, raw_ବାମ,
                                   z_ଡାହାଣ, n_ଡାହାଣ, raw_ଡାହାଣ);
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
ସ୍ଥିର u32 jsonLookupStep(JsonParse*,u32,ଅଚଳ ଅକ୍ଷର*,u32);


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
ସ୍ଥିର u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  ଅଚଳ ଅକ୍ଷର *zTail   /* Tail of the path that determines substructure */
){
  ସ୍ଥିର ଅଚଳ u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  memset(pIns, 0, ଆକାର(*pIns));
  pIns->db = pParse->db;
  ଯଦି( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }ଅନ୍ୟଥା{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    ଯଦି( pIns->iDepth >= JSON_MAX_DEPTH ){
      ଫେରନ୍ତୁ JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  ଫେରନ୍ତୁ rc;  /* Error code only */
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
ସ୍ଥିର u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_ମୂଳ,              /* Begin the search at this element of aBlob[] */
  ଅଚଳ ଅକ୍ଷର *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_ଚାବି, sz, n, iEnd, rc;
  ଅଚଳ ଅକ୍ଷର *z_ଚାବି;
  u8 x;

  ଯଦି( zPath[0]==0 ){
    ଯଦି( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, i_ମୂଳ, &sz);
      sz += n;
      ଯଦି( pParse->eEdit==JEDIT_DEL ){
        ଯଦି( iLabel>0 ){
          sz += i_ମୂଳ - iLabel;
          i_ମୂଳ = iLabel;
        }
        jsonBlobEdit(pParse, i_ମୂଳ, sz, 0, 0);
      }ଅନ୍ୟଥା ଯଦି( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }ଅନ୍ୟଥା ଯଦି( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        ଯଦି( zPath[-1]!=']' ){
          ଫେରନ୍ତୁ JSON_LOOKUP_NOTARRAY;
        }ଅନ୍ୟଥା{
          jsonBlobEdit(pParse, i_ମୂଳ, 0, pParse->aIns, pParse->nIns);
        }
      }ଅନ୍ୟଥା{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_ମୂଳ, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    ଫେରନ୍ତୁ i_ମୂଳ;
  }
  ଯଦି( zPath[0]=='.' ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw_ଚାବି = 1;
    x = pParse->aBlob[i_ମୂଳ];
    zPath++;
    ଯଦି( zPath[0]=='"' ){
      z_ଚାବି = zPath + 1;
      ପାଇଁ(i=1; zPath[i] && zPath[i]!='"'; i++){
        ଯଦି( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_ଚାବି = i-1;
      ଯଦି( zPath[i] ){
        i++;
      }ଅନ୍ୟଥା{
        ଫେରନ୍ତୁ JSON_LOOKUP_PATHERROR;
      }
      testcase( n_ଚାବି==0 );
      raw_ଚାବି = memchr(z_ଚାବି, '\\', n_ଚାବି)==0;
    }ଅନ୍ୟଥା{
      z_ଚାବି = zPath;
      ପାଇଁ(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_ଚାବି = i;
      ଯଦି( n_ଚାବି==0 ){
        ଫେରନ୍ତୁ JSON_LOOKUP_PATHERROR;
      }
    }
    ଯଦି( (x & 0x0f)!=JSONB_OBJECT ) ଫେରନ୍ତୁ JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_ମୂଳ, &sz);
    j = i_ମୂଳ + n;  /* j is the index of a label */
    iEnd = j+sz;
    ଯେତେବେଳେ( j<iEnd ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rawLabel;
      ଅଚଳ ଅକ୍ଷର *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      ଯଦି( x<JSONB_TEXT || x>JSONB_TEXTRAW ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ଯଦି( n==0 ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      ଯଦି( k+sz>=iEnd ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
      zLabel = (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      ଯଦି( jsonLabelCompare(z_ଚାବି, n_ଚାବି, raw_ଚାବି, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        ଯଦି( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        ଯଦି( n==0 || v+n+sz>iEnd ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
        ପ୍ରତିଜ୍ଞା( j>0 );
        ଯଦି( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          ଫେରନ୍ତୁ JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        ଯଦି( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ମୂଳ);
        ଫେରନ୍ତୁ rc;
      }
      j = k+sz;
      ଯଦି( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ଯଦି( n==0 ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ଯଦି( j>iEnd ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
    ଯଦି( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      ଯଦି( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        ଫେରନ୍ତୁ JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, ଆକାର(ix));
      ix.db = pParse->db;
      json_Blob_Append_ଗଣ୍ଠି(&ix, raw_ଚାବି?JSONB_TEXTRAW:JSONB_TEXT5, n_ଚାବି, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      ଯଦି( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_ଚାବି+v.nBlob)
      ){
        ପ୍ରତିଜ୍ଞା( !pParse->oom );
        nIns = ix.nBlob + n_ଚାବି + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        ଯଦି( !pParse->oom ){
          ପ୍ରତିଜ୍ଞା( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          ପ୍ରତିଜ୍ଞା( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], z_ଚାବି, n_ଚାବି);
          k += n_ଚାବି;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          ଯଦି( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, i_ମୂଳ);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      ଫେରନ୍ତୁ rc;
    }
  }ଅନ୍ୟଥା ଯଦି( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_ମୂଳ] & 0x0f;
    ଯଦି( x!=JSONB_ARRAY )  ଫେରନ୍ତୁ JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_ମୂଳ, &sz);
    i = 1;
    ଯେତେବେଳେ( sqlite3Isdigit(zPath[i]) ){
      ଯଦି( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    ଯଦି( i<2 || zPath[i]!=']' ){
      ଯଦି( zPath[1]=='#' ){
        kk = jsonb_Array_ଗଣନା(pParse, i_ମୂଳ);
        i = 2;
        ଯଦି( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          କର{
            ଯଦି( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }ଯେତେବେଳେ( sqlite3Isdigit(zPath[i]) );
          ଯଦି( nn>kk ) ଫେରନ୍ତୁ JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        ଯଦି( zPath[i]!=']' ){
          ଫେରନ୍ତୁ JSON_LOOKUP_PATHERROR;
        }
      }ଅନ୍ୟଥା{
        ଫେରନ୍ତୁ JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_ମୂଳ+n;
    iEnd = j+sz;
    ଯେତେବେଳେ( j<iEnd ){
      ଯଦି( kk==0 ){
        ଯଦି( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          ଫେରନ୍ତୁ JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        ଯଦି( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ମୂଳ);
        ଫେରନ୍ତୁ rc;
      }
      kk--;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ଯଦି( n==0 ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ଯଦି( j>iEnd ) ଫେରନ୍ତୁ JSON_LOOKUP_ERROR;
    ଯଦି( kk>0 ) ଫେରନ୍ତୁ JSON_LOOKUP_NOTFOUND;
    ଯଦି( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      ଯଦି( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        ପ୍ରତିଜ୍ଞା( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      ଯଦି( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ମୂଳ);
      ଫେରନ୍ତୁ rc;
    }
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ JSON_LOOKUP_PATHERROR; 
  }
  ଫେରନ୍ତୁ JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  ଅଚଳ u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  ଯଦି( NEVER(aBlob==0) ) ଫେରନ୍ତୁ;
  memset(&x, 0, ଆକାର(x));
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
ସ୍ଥିର ଶୂନ୍ୟ jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  ପ୍ରତିଜ୍ଞା( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  ଯଦି( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    ଫେରନ୍ତୁ;
  }
  ବାଛ( pParse->aBlob[i] & 0x0f ){
    ସ୍ଥିତି JSONB_NULL: {
      ଯଦି( sz ) ଯାଅ returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_TRUE: {
      ଯଦି( sz ) ଯାଅ returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_FALSE: {
      ଯଦି( sz ) ଯାଅ returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_INT5:
    ସ୍ଥିତି JSONB_INT: {
      sqlite3_int64 iRes = 0;
      ଅକ୍ଷର *z;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bNeg = 0;
      ଅକ୍ଷର x;
      ଯଦି( sz==0 ) ଯାଅ returnfromblob_malformed;
      x = (ଅକ୍ଷର)pParse->aBlob[i+n];
      ଯଦି( x=='-' ){
        ଯଦି( sz<2 ) ଯାଅ returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n], (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sz);
      ଯଦି( z==0 ) ଯାଅ returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      ଯଦି( rc==0 ){
        ଯଦି( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          ଦ୍ୱିଗୁଣ r;
          r = (ଦ୍ୱିଗୁଣ)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }ଅନ୍ୟଥା{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }ଅନ୍ୟଥା ଯଦି( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }ଅନ୍ୟଥା ଯଦି( rc==1 ){
        ଯାଅ returnfromblob_malformed;
      }ଅନ୍ୟଥା{
        ଯଦି( bNeg ){ n--; sz++; }
        ଯାଅ to_double;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_FLOAT5:
    ସ୍ଥିତି JSONB_FLOAT: {
      ଦ୍ୱିଗୁଣ r;
      ଅକ୍ଷର *z;
      ଯଦି( sz==0 ) ଯାଅ returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n], (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sz);
      ଯଦି( z==0 ) ଯାଅ returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      ଯଦି( rc<=0 ) ଯାଅ returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_TEXTRAW:
    ସ୍ଥିତି JSONB_TEXT: {
      sqlite3_result_text(pCtx, (ଅକ୍ଷର*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_TEXT5:
    ସ୍ଥିତି JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      ଅଚଳ ଅକ୍ଷର *z;
      ଅକ୍ଷର *zOut;
      u32 nOut = sz;
      z = (ଅଚଳ ଅକ୍ଷର*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      ଯଦି( zOut==0 ) ଯାଅ returnfromblob_oom;
      ପାଇଁ(iIn=iOut=0; iIn<sz; iIn++){
        ଅକ୍ଷର c = z[iIn];
        ଯଦି( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          ଯଦି( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }ଅନ୍ୟଥା ଯଦି( v<=0x7f ){
            zOut[iOut++] = (ଅକ୍ଷର)v;
          }ଅନ୍ୟଥା ଯଦି( v<=0x7ff ){
            ପ୍ରତିଜ୍ଞା( szEscape>=2 );
            zOut[iOut++] = (ଅକ୍ଷର)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }ଅନ୍ୟଥା ଯଦି( v<0x10000 ){
            ପ୍ରତିଜ୍ଞା( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }ଅନ୍ୟଥା ଯଦି( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }ଅନ୍ୟଥା{
            ପ୍ରତିଜ୍ଞା( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }ଅନ୍ୟଥା{
          zOut[iOut++] = c;
        }
      } /* end for() */
      ପ୍ରତିଜ୍ଞା( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JSONB_ARRAY:
    ସ୍ଥିତି JSONB_OBJECT: {
      ଯଦି( eMode==0 ){
        ଯଦି( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }ଅନ୍ୟଥା{
          eMode = 1;
        }
      }
      ଯଦି( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }ଅନ୍ୟଥା{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ଯାଅ returnfromblob_malformed;
    }
  }
  ଫେରନ୍ତୁ;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  ଫେରନ୍ତୁ;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  ଫେରନ୍ତୁ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_ମୂଲ୍ୟ *pArg,
  JsonParse *pParse
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType = sqlite3_ମୂଲ୍ୟ_type(pArg);
  ସ୍ଥିର u8 aNull[] = { 0x00 };
  memset(pParse, 0, ଆକାର(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  ବାଛ( eType ){
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      ଫେରନ୍ତୁ 0;
    }
    ସ୍ଥିତି SQLITE_BLOB: {
      ଯଦି( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        ଫେରନ୍ତୁ 1;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_TEXT: {
      ଅଚଳ ଅକ୍ଷର *zJson = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pArg);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nJson = sqlite3_ମୂଲ୍ୟ_bytes(pArg);
      ଯଦି( zJson==0 ) ଫେରନ୍ତୁ 1;
      ଯଦି( sqlite3_ମୂଲ୍ୟ_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (ଅକ୍ଷର*)zJson;
        pParse->nJson = nJson;
        ଯଦି( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, ଆକାର(pParse[0]));
          ଫେରନ୍ତୁ 1;
        }
      }ଅନ୍ୟଥା{
        json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_FLOAT: {
      ଯଦି( NEVER(sqlite3IsNaN(sqlite3_ମୂଲ୍ୟ_double(pArg))) ){
        json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_NULL, 0, 0);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = sqlite3_ମୂଲ୍ୟ_bytes(pArg);
        ଅଚଳ ଅକ୍ଷର *z = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pArg);
        ଯଦି( z==0 ) ଫେରନ୍ତୁ 1;
        ଯଦି( z[0]=='I' ){
          json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_FLOAT, 5, "9e999");
        }ଅନ୍ୟଥା ଯଦି( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_FLOAT, 6, "-9e999");
        }ଅନ୍ୟଥା{
          json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_FLOAT, n, z);
        }
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_INTEGER: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = sqlite3_ମୂଲ୍ୟ_bytes(pArg);
      ଅଚଳ ଅକ୍ଷର *z = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pArg);
      ଯଦି( z==0 ) ଫେରନ୍ତୁ 1;
      json_Blob_Append_ଗଣ୍ଠି(pParse, JSONB_INT, n, z);
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    ଫେରନ୍ତୁ 1;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ଅକ୍ଷର *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  ଅଚଳ ଅକ୍ଷର *zPath,        /* The path with the problem */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  ଅକ୍ଷର *z_ସନ୍ଦେଶ;
  ଯଦି( rc==(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)JSON_LOOKUP_NOTARRAY ){
    z_ସନ୍ଦେଶ = sqlite3_mprintf("not an array element: %Q", zPath);
  }ଅନ୍ୟଥା ଯଦି( rc==(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)JSON_LOOKUP_ERROR ){
    z_ସନ୍ଦେଶ = sqlite3_mprintf("malformed JSON");
  }ଅନ୍ୟଥା ଯଦି( rc==(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)JSON_LOOKUP_TOODEEP ){
    z_ସନ୍ଦେଶ = sqlite3_mprintf("JSON path too deep");
  }ଅନ୍ୟଥା{
    z_ସନ୍ଦେଶ = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  ଯଦି( ctx==0 ) ଫେରନ୍ତୁ z_ସନ୍ଦେଶ;
  ଯଦି( z_ସନ୍ଦେଶ ){
    sqlite3_result_error(ctx, z_ସନ୍ଦେଶ, -1);
    sqlite3_free(z_ସନ୍ଦେଶ);
  }ଅନ୍ୟଥା{
    sqlite3_result_error_nomem(ctx);
  }
  ଫେରନ୍ତୁ 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
ସ୍ଥିର ଶୂନ୍ୟ json_ଭର୍ତ୍ତି_Into_Blob(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  u32 rc = 0;
  ଅଚଳ ଅକ୍ଷର *zPath = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flgs;
  JsonParse *p;
  JsonParse ax;

  ପ୍ରତିଜ୍ଞା( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  ପାଇଁ(i=1; i<argc-1; i+=2){
    ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[i])==SQLITE_NULL ) ଜାରି;
    zPath = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
    ଯଦି( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      ଫେରନ୍ତୁ;
    }
    ଯଦି( zPath[0]!='$' ) ଯାଅ json_ଭର୍ତ୍ତି_Into_Blob_patherror;
    ଯଦି( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      ଫେରନ୍ତୁ;
    }
    ଯଦି( zPath[1]==0 ){
      ଯଦି( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }ଅନ୍ୟଥା{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    ଯଦି( rc==JSON_LOOKUP_NOTFOUND ) ଜାରି;
    ଯଦି( JSON_LOOKUP_ISERROR(rc) ) ଯାଅ json_ଭର୍ତ୍ତି_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ଫେରନ୍ତୁ;

json_ଭର୍ତ୍ତି_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  ଫେରନ୍ତୁ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonArgIsJsonb(sqlite3_ମୂଲ୍ୟ *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(pArg)!=SQLITE_BLOB ) ଫେରନ୍ତୁ 0;
  p->aBlob = (u8*)sqlite3_ମୂଲ୍ୟ_blob(pArg);
  p->nBlob = (u32)sqlite3_ମୂଲ୍ୟ_bytes(pArg);
  ଯଦି( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    ଫେରନ୍ତୁ 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_ମୂଲ୍ୟ *pArg,
  u32 flgs
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  ପ୍ରତିଜ୍ଞା( ctx!=0 );
  eType = sqlite3_ମୂଲ୍ୟ_type(pArg);
  ଯଦି( eType==SQLITE_NULL ){
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_ସନ୍ଧାନ(ctx, pArg);
  }
  ଯଦି( pFromCache ){
    pFromCache->nJPRef++;
    ଯଦି( (flgs & JSON_EDITABLE)==0 ){
      ଫେରନ୍ତୁ pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, ଆକାର(*p));
  ଯଦି( p==0 ) ଯାଅ json_pfa_oom;
  memset(p, 0, ଆକାର(*p));
  p->db = db;
  p->nJPRef = 1;
  ଯଦି( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    ଯଦି( p->aBlob==0 ) ଯାଅ json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    ଫେରନ୍ତୁ p;
  }
  ଯଦି( eType==SQLITE_BLOB ){
    ଯଦି( jsonArgIsJsonb(pArg,p) ){
      ଯଦି( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        ଯାଅ json_pfa_oom;
      }
      ଫେରନ୍ତୁ p;
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
  p->zJson = (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pArg);
  p->nJson = sqlite3_ମୂଲ୍ୟ_bytes(pArg);
  ଯଦି( db->mallocFailed ) ଯାଅ json_pfa_oom;
  ଯଦି( p->nJson==0 ) ଯାଅ json_pfa_malformed;
  ପ୍ରତିଜ୍ଞା( p->zJson!=0 );
  ଯଦି( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    ଯଦି( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      ଫେରନ୍ତୁ p;
    }ଅନ୍ୟଥା{
      jsonParseFree(p);
      ଫେରନ୍ତୁ 0;
    }
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isRCStr = sqlite3_ମୂଲ୍ୟ_Is_Of_Class(pArg, sqlite3RCStrUnref);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ଯଦି( !isRCStr ){
      ଅକ୍ଷର *zNew = sqlite3RCStrNew( p->nJson );
      ଯଦି( zNew==0 ) ଯାଅ json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }ଅନ୍ୟଥା{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_ଭର୍ତ୍ତି(ctx, p);
    ଯଦି( rc==SQLITE_NOMEM ) ଯାଅ json_pfa_oom;
    ଯଦି( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      ଯାଅ rebuild_from_cache;
    }
  }
  ଫେରନ୍ତୁ p;

json_pfa_malformed:
  ଯଦି( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    ଫେରନ୍ତୁ p;
  }ଅନ୍ୟଥା{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    ଫେରନ୍ତୁ 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  ଫେରନ୍ତୁ 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flgs;
  ଯଦି( p->oom ){
    sqlite3_result_error_nomem(ctx);
    ଫେରନ୍ତୁ;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  ଯଦି( flgs & JSON_BLOB ){
    ଯଦି( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }ଅନ୍ୟଥା{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }ଅନ୍ୟଥା{
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

#ଯଦି SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  ଯେତେବେଳେ( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    ଯଦି( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    ଯଦି( nn==0 ) nn = 1;
    ଯଦି( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    ପାଇଁ(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    ଯଦି( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      ଜାରି;
    }
    pParse->nBlob = savedNBlob;
    ଯଦି( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      ଯଦି( iEnd>pParse->nBlob ){
        ଯଦି( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }ଅନ୍ୟଥା{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    ବାଛ( x ){
      ସ୍ଥିତି JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_INT:      sqlite3_str_appendall(pOut,"int"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); ଭାଙ୍ଗ;
      ସ୍ଥିତି JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        ଭାଙ୍ଗ;
      }
    }
    ଯଦି( showContent ){
      ଯଦି( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }ଅନ୍ୟଥା{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        ପାଇଁ(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          ଯଦି( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (ଅକ୍ଷର*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
ସ୍ଥିର ଶୂନ୍ୟ jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  ଅକ୍ଷର zBuf[1000];
  ଯଦି( pParse==0 ){
    printf("NULL pointer\n");
    ଫେରନ୍ତୁ;
  }ଅନ୍ୟଥା{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    ଯଦି( pParse->nBlob==0 ) ଫେରନ୍ତୁ;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, ଆକାର(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_ମୂଲ୍ୟ(&out));
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
ସ୍ଥିର ଶୂନ୍ୟ jsonParseFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  ପ୍ରତିଜ୍ଞା( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  ଯଦି( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ jsonQuoteFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_ମୂଲ୍ୟ(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonArrayFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  ପାଇଁ(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_ମୂଲ୍ୟ(&jx, argv[i]);
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
ସ୍ଥିର ଶୂନ୍ୟ jsonArrayLengthFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  ଯଦି( argc==2 ){
    ଅଚଳ ଅକ୍ଷର *zPath = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]);
    ଯଦି( zPath==0 ){
      jsonParseFree(p);
      ଫେରନ୍ତୁ;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    ଯଦି( JSON_LOOKUP_ISERROR(i) ){
      ଯଦି( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ଅନ୍ୟଥା{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }ଅନ୍ୟଥା{
    i = 0;
  }
  ଯଦି( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_ଗଣନା(p, i);
  }
  ଯଦି( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonAllAlphanum(ଅଚଳ ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  ଫେରନ୍ତୁ i==n;
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
ସ୍ଥିର ଶୂନ୍ୟ jsonExtractFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonParse *p = 0;      /* The parse */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;             /* Flags associated with the function */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  ଯଦି( argc<2 ) ଫେରନ୍ତୁ;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  ଯଦି( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  ପାଇଁ(i=1; i<argc; i++){
    /* With a single PATH argument */
    ଅଚଳ ଅକ୍ଷର *zPath = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPath;
    u32 j;
    ଯଦି( zPath==0 ) ଯାଅ json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    ଯଦି( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }ଅନ୍ୟଥା ଯଦି( (flags & JSON_ABPATH) ){
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
      ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        ଯଦି( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }ଅନ୍ୟଥା ଯଦି( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }ଅନ୍ୟଥା ଯଦି( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }ଅନ୍ୟଥା{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }ଅନ୍ୟଥା{
      jsonBadPathError(ctx, zPath, 0);
      ଯାଅ json_extract_error;
    }
    ଯଦି( j<p->nBlob ){
      ଯଦି( argc==2 ){
        ଯଦି( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          ପ୍ରତିଜ୍ଞା( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }ଅନ୍ୟଥା{
          jsonReturnFromBlob(p, j, ctx, 0);
          ଯଦି( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }ଅନ୍ୟଥା{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }ଅନ୍ୟଥା ଯଦି( j==JSON_LOOKUP_NOTFOUND ){
      ଯଦି( argc==2 ){
        ଯାଅ json_extract_error;  /* Return NULL if not found */
      }ଅନ୍ୟଥା{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }ଅନ୍ୟଥା{
      jsonBadPathError(ctx, zPath, j);
      ଯାଅ json_extract_error;
    }
  }
  ଯଦି( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    ଯଦି( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  ଫେରନ୍ତୁ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  ଅଚଳ JsonParse *pPatch, /* The PATCH */
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

  ପ୍ରତିଜ୍ଞା( iTarget>=0 && iTarget<pTarget->nBlob );
  ପ୍ରତିଜ୍ଞା( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  ଯଦି( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    ଫେରନ୍ତୁ pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  ଯଦି( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  ଯଦି( NEVER(n==0) ) ଫେରନ୍ତୁ JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  ଯଦି( NEVER(n==0) ) ଫେରନ୍ତୁ JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  ଯେତେବେଳେ( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    ଯଦି( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      ଫେରନ୍ତୁ JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    ଯଦି( nPLabel==0 ) ଫେରନ୍ତୁ JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    ଯଦି( iPValue>=iPEnd ) ଫେରନ୍ତୁ JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    ଯଦି( nPValue==0 ) ଫେରନ୍ତୁ JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    ଯଦି( iPCursor>iPEnd ) ଫେରନ୍ତୁ JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    ଯେତେବେଳେ( iTCursor<iTEnd ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      ଯଦି( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        ଫେରନ୍ତୁ JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      ଯଦି( nTLabel==0 ) ଫେରନ୍ତୁ JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      ଯଦି( iTValue>=iTEnd ) ଫେରନ୍ତୁ JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      ଯଦି( nTValue==0 ) ଫେରନ୍ତୁ JSON_MERGE_BADTARGET;
      ଯଦି( iTValue + nTValue + szTValue > iTEnd ) ଫେରନ୍ତୁ JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ଅଚଳ ଅକ୍ଷର*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (ଅଚଳ ଅକ୍ଷର*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      ଯଦି( isEqual ) ଭାଙ୍ଗ;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    ଯଦି( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      ଯଦି( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        ଯଦି( NEVER(pTarget->oom) ) ଫେରନ୍ତୁ JSON_MERGE_OOM;
      }ଅନ୍ୟଥା{
        /* Algorithm line 12 */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        ଯଦି( iDepth>=JSON_MAX_DEPTH ) ଫେରନ୍ତୁ JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        ଯଦି( rc ) ଫେରନ୍ତୁ rc;
        pTarget->delta += savedDelta;
      }        
    }ଅନ୍ୟଥା ଯଦି( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      ଯଦି( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        ଯଦି( pTarget->oom ) ଫେରନ୍ତୁ JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        ଯଦି( pTarget->oom ) ଫେରନ୍ତୁ JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        ଯଦି( iDepth>=JSON_MAX_DEPTH ) ଫେରନ୍ତୁ JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        ଯଦି( rc ) ଫେରନ୍ତୁ rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  ଯଦି( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  ଫେରନ୍ତୁ pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonPatchFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  ପ୍ରତିଜ୍ଞା( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  ଯଦି( pTarget==0 ) ଫେରନ୍ତୁ;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  ଯଦି( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    ଯଦି( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }ଅନ୍ୟଥା ଯଦି( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }ଅନ୍ୟଥା ଯଦି( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ jsonObjectFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  JsonString jx;
  ଅଚଳ ଅକ୍ଷର *z;
  u32 n;

  ଯଦି( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    ଫେରନ୍ତୁ;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  ପାଇଁ(i=0; i<argc; i+=2){
    ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      ଫେରନ୍ତୁ;
    }
    jsonAppendSeparator(&jx);
    z = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
    n = sqlite3_ମୂଲ୍ୟ_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_ମୂଲ୍ୟ(&jx, argv[i+1]);
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
ସ୍ଥିର ଶୂନ୍ୟ jsonRemoveFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonParse *p;          /* The parse */
  ଅଚଳ ଅକ୍ଷର *zPath = 0; /* Path of element to be removed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  ଯଦି( argc<1 ) ଫେରନ୍ତୁ;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  ପାଇଁ(i=1; i<argc; i++){
    zPath = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
    ଯଦି( zPath==0 ){
      ଯାଅ json_remove_done;
    }
    ଯଦି( zPath[0]!='$' ){
      ଯାଅ json_remove_patherror;
    }
    ଯଦି( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      ଯାଅ json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    ଯଦି( JSON_LOOKUP_ISERROR(rc) ){
      ଯଦି( rc==JSON_LOOKUP_NOTFOUND ){
        ଜାରି;  /* No-op */
      }ଅନ୍ୟଥା{
        jsonBadPathError(ctx, zPath, rc);
      }
      ଯାଅ json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ଫେରନ୍ତୁ;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  ଫେରନ୍ତୁ;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonReplaceFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଯଦି( argc<1 ) ଫେରନ୍ତୁ;
  ଯଦି( (argc&1)==0 ) {
    json_Wrong_ଅଙ୍କ_Args(ctx, "replace");
    ଫେରନ୍ତୁ;
  }
  json_ଭର୍ତ୍ତି_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
ସ୍ଥିର ଶୂନ୍ୟ jsonSetFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eInsType = JSON_ଭର୍ତ୍ତି_TYPE(flags);
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azInsType[] = { "insert", "set", "array_insert" };
  ସ୍ଥିର ଅଚଳ u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  ଯଦି( argc<1 ) ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( eInsType>=0 && eInsType<=2 );
  ଯଦି( (argc&1)==0 ) {
    json_Wrong_ଅଙ୍କ_Args(ctx, azInsType[eInsType]);
    ଫେରନ୍ତୁ;
  }
  json_ଭର୍ତ୍ତି_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonTypeFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonParse *p;          /* The parse */
  ଅଚଳ ଅକ୍ଷର *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  ଯଦି( argc==2 ){
    zPath = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]);
    ଯଦି( zPath==0 ) ଯାଅ json_type_done;
    ଯଦି( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      ଯାଅ json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    ଯଦି( JSON_LOOKUP_ISERROR(i) ){
      ଯଦି( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ଅନ୍ୟଥା{
        jsonBadPathError(ctx, zPath, i);
      }
      ଯାଅ json_type_done;
    }
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ jsonPrettyFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, ଆକାର(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  ଯଦି( x.pParse==0 ) ଫେରନ୍ତୁ;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  ଯଦି( argc==1 || (x.zIndent = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ jsonValidFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  ଯଦି( argc==2 ){
    i64 f = sqlite3_ମୂଲ୍ୟ_int64(argv[1]);
    ଯଦି( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      ଫେରନ୍ତୁ;
    }
    flags = f & 0x0f;
  }
  ବାଛ( sqlite3_ମୂଲ୍ୟ_type(argv[0]) ){
    ସ୍ଥିତି SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      ଫେରନ୍ତୁ;
    }
    ସ୍ଥିତି SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, ଆକାର(py));
      ଯଦି( jsonArgIsJsonb(argv[0], &py) ){
        ଯଦି( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }ଅନ୍ୟଥା ଯଦି( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        ଭାଙ୍ଗ;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      JsonParse px;
      ଯଦି( (flags & 0x3)==0 ) ଭାଙ୍ଗ;
      memset(&px, 0, ଆକାର(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      ଯଦି( p ){
        ଯଦି( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }ଅନ୍ୟଥା ଯଦି( p->nErr ){
          /* no-op */
        }ଅନ୍ୟଥା ଯଦି( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }ଅନ୍ୟଥା{
        sqlite3_result_error_nomem(ctx);
      }
      ଭାଙ୍ଗ;
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
ସ୍ଥିର ଶୂନ୍ୟ jsonErrorFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  ପ୍ରତିଜ୍ଞା( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, ଆକାର(s));
  s.db = sqlite3_context_db_handle(ctx);
  ଯଦି( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }ଅନ୍ୟଥା{
    s.zJson = (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    ଯଦି( s.zJson==0 ) ଫେରନ୍ତୁ;  /* NULL input or OOM */
    s.nJson = sqlite3_ମୂଲ୍ୟ_bytes(argv[0]);
    ଯଦି( jsonConvertTextToBlob(&s,0) ){
      ଯଦି( s.oom ){
        iErrPos = -1;
      }ଅନ୍ୟଥା{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        ପ୍ରତିଜ୍ଞା( s.zJson!=0 );  /* Because s.oom is false */
        ପାଇଁ(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          ଯଦି( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  ଯଦି( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ jsonArrayStep(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, ଆକାର(*pStr));
  ଯଦି( pStr ){
    ଯଦି( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }ଅନ୍ୟଥା ଯଦି( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_ମୂଲ୍ୟ(pStr, argv[0]);
  }
}
ସ୍ଥିର ଶୂନ୍ୟ jsonArrayCompute(sqlite3_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isFinal){
  JsonString *pStr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ଯଦି( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    ଯଦି( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ଫେରନ୍ତୁ;
    }ଅନ୍ୟଥା ଯଦି( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ଯଦି( isFinal ){
        ଯଦି( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ଅନ୍ୟଥା{
        jsonStringTrimOneChar(pStr);
      }
      ଫେରନ୍ତୁ;
    }ଅନ୍ୟଥା ଯଦି( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ଅନ୍ୟଥା{
      sqlite3_result_text(ctx, pStr->zBuf, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }ଅନ୍ୟଥା ଯଦି( flags & JSON_BLOB ){
    ସ୍ଥିର ଅଚଳ u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }ଅନ୍ୟଥା{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
ସ୍ଥିର ଶୂନ୍ୟ json_Array_ମୂଲ୍ୟ(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
ସ୍ଥିର ଶୂନ୍ୟ jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonGroupInverse(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା inStr = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nNest = 0;
  ଅକ୍ଷର *z;
  ଅକ୍ଷର c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  ଯଦି( NEVER(!pStr) ) ଫେରନ୍ତୁ;
  z = pStr->zBuf;
  ପାଇଁ(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    ଯଦି( c=='"' ){
      inStr = !inStr;
    }ଅନ୍ୟଥା ଯଦି( c=='\\' ){
      i++;
    }ଅନ୍ୟଥା ଯଦି( !inStr ){
      ଯଦି( c=='{' || c=='[' ) nNest++;
      ଯଦି( c=='}' || c==']' ) nNest--;
    }
  }
  ଯଦି( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }ଅନ୍ୟଥା{
    pStr->nUsed = 1;
  }
}
#ଅନ୍ୟଥା
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
ସ୍ଥିର ଶୂନ୍ୟ jsonObjectStep(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  JsonString *pStr;
  ଅଚଳ ଅକ୍ଷର *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, ଆକାର(*pStr));
  ଯଦି( pStr ){
    z = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    n = sqlite3Strlen30(z);
    ଯଦି( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }ଅନ୍ୟଥା ଯଦି( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    ଯଦି( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_ମୂଲ୍ୟ(pStr, argv[1]);
    }ଅନ୍ୟଥା{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
ସ୍ଥିର ଶୂନ୍ୟ jsonObjectCompute(sqlite3_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isFinal){
  JsonString *pStr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ଯଦି( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    ଯଦି( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ଫେରନ୍ତୁ;
    }
    ଯଦି( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା inStr = 0;
      ଯଦି( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        ଯଦି( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          ଫେରନ୍ତୁ;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      ପାଇଁ(i=j=1; i<pStr->nUsed; i++){
        ଅକ୍ଷର c = pStr->zBuf[i];
        ଯଦି( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }ଅନ୍ୟଥା ଯଦି( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }ଅନ୍ୟଥା ଯଦି( c=='@' && !inStr ){
          ପ୍ରତିଜ୍ଞା( i+1<pStr->nUsed );
          ଯଦି( pStr->zBuf[i+1]==',' ){
            i++;
          }ଅନ୍ୟଥା ଯଦି( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }ଅନ୍ୟଥା{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    ଯଦି( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ଯଦି( isFinal ){
        ଯଦି( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ଅନ୍ୟଥା{
        jsonStringTrimOneChar(pOgStr);
      }
    }ଅନ୍ୟଥା ଯଦି( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ଅନ୍ୟଥା{
      sqlite3_result_text(ctx, pStr->zBuf, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    ଯଦି( pStr!=pOgStr ) jsonStringReset(pStr);
  }ଅନ୍ୟଥା ଯଦି( flags & JSON_BLOB ){
    ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }ଅନ୍ୟଥା{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
ସ୍ଥିର ଶୂନ୍ୟ json_Object_ମୂଲ୍ୟ(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
ସ୍ଥିର ଶୂନ୍ୟ jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା JsonParent JsonParent;
ସଂରଚନା JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 i_ମୂଲ୍ୟ;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_ଚାବି;                  /* Key for JSONB_ARRAY */
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା JsonEachCursor JsonEachCursor;
ସଂରଚନା JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_ମୂଳ;                 /* Size of the root path in bytes */
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା JsonEachConnection JsonEachConnection;
ସଂରଚନା JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  JsonEachConnection *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

/* Column numbers */
#define JEACH_ଚାବି     0
#define JEACH_ମୂଲ୍ୟ   1
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
#define JEACH_ମୂଳ    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  ଯଦି( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, ଆକାର(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    ଯଦି( pNew==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  ଫେରନ୍ତୁ rc;
}

/* destructor for json_each virtual table */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, ଆକାର(*pCur));
  ଯଦି( pCur==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
ସ୍ଥିର ଶୂନ୍ୟ jsonEachCursorReset(JsonEachCursor *p){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ଫେରନ୍ତୁ p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonSkipLabel(JsonEachCursor *p){
  ଯଦି( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    ଯଦି( sz >= p->sParse.nBlob ) sz = p->i;
    ଫେରନ୍ତୁ sz;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ p->i;
  }
}

/*
** Append the path name for the current element.
*/
ସ୍ଥିର ଶୂନ୍ୟ jsonAppendPathName(JsonEachCursor *p){
  ପ୍ରତିଜ୍ଞା( p->nParent>0 );
  ପ୍ରତିଜ୍ଞା( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  ଯଦି( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_ଚାବି);
  }ଅନ୍ୟଥା{
    u32 n, sz = 0, k, i;
    ଅଚଳ ଅକ୍ଷର *z;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (ଅଚଳ ଅକ୍ଷର*)&p->sParse.aBlob[k];
    ଯଦି( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }ଅନ୍ୟଥା{
      ପାଇଁ(i=0; i<sz; i++){
        ଯଦି( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          ଭାଙ୍ଗ;
        }
      }
    }
    ଯଦି( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }ଅନ୍ୟଥା{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା json_Each_Malformed_ନିବେଶ(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_ସନ୍ଦେଶ);
  cur->pVtab->z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  ଫେରନ୍ତୁ cur->pVtab->z_Err_ସନ୍ଦେଶ ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଯଦି( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    ଯଦି( n==0 )ଫେରନ୍ତୁ json_Each_Malformed_ନିବେଶ(cur);
    ଯଦି( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      ଯଦି( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, ଆକାର(JsonParent)*nNew);
        ଯଦି( pNew==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->i_ମୂଲ୍ୟ = i;
      pParent->iEnd = i + n + sz;
      pParent->i_ଚାବି = -1;
      pParent->nPath = (u32)p->path.nUsed;
      ଯଦି( p->eType && p->nParent ){
        jsonAppendPathName(p);
        ଯଦି( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }ଅନ୍ୟଥା{
      p->i = i + n + sz;
    }
    ଯେତେବେଳେ( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    ଯଦି( levelChange ){
      ଯଦି( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->i_ମୂଲ୍ୟ;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }ଅନ୍ୟଥା{
        p->eType = 0;
      }
    }
  }ଅନ୍ୟଥା{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    ଯଦି( n==0 )ଫେରନ୍ତୁ json_Each_Malformed_ନିବେଶ(cur);
    p->i = i + n + sz;
  }
  ଯଦି( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_ଚାବି++;
  }
  p->iRowid++;
  ଫେରନ୍ତୁ rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  ଅକ୍ଷର *z = p->path.zBuf;
  ଯଦି( p->iRowid==0 && p->bRecursive && n>=2 ){
    ଯେତେବେଳେ( ALWAYS(n>1) ){
      n--;
      ଯଦି( z[n]=='[' || z[n]=='.' ){
        u32 x;
        ଅକ୍ଷର cSaved = z[n];
        z[n] = 0;
        ପ୍ରତିଜ୍ଞା( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        ଯଦି( !JSON_LOOKUP_ISERROR(x) ) ଭାଙ୍ଗ;
      }
    }
  }
  ଫେରନ୍ତୁ n;
}

/* Return the value of a column */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ବାଛ( iColumn ){
    ସ୍ଥିତି JEACH_ଚାବି: {
      ଯଦି( p->nParent==0 ){
        u32 n, j;
        ଯଦି( p->n_ମୂଳ==1 ) ଭାଙ୍ଗ;
        j = jsonEachPathLength(p);
        n = p->n_ମୂଳ - j;
        ଯଦି( n==0 ){
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା ଯଦି( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }ଅନ୍ୟଥା ଯଦି( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }ଅନ୍ୟଥା{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        ଭାଙ୍ଗ;
      }
      ଯଦି( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }ଅନ୍ୟଥା{
        ପ୍ରତିଜ୍ଞା( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].i_ଚାବି);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JEACH_ମୂଲ୍ୟ: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      ଯଦି( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      ଯଦି( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JEACH_PARENT: {
      ଯଦି( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      ଯଦି( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      sqlite3_result_text(ctx, p->path.zBuf, p->n_ମୂଳ, SQLITE_TRANSIENT);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି JEACH_JSON: {
      ଯଦି( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }ଅନ୍ୟଥା{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Return the current rowid value */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                     /* Loop counter or computed array index */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା aIdx[2];               /* Index of constraints for JSON and ROOT */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  ଅଚଳ ସଂରଚନା sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  ପ୍ରତିଜ୍ଞା( JEACH_ମୂଳ == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  ପାଇଁ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iMask;
    ଯଦି( pConstraint->iColumn < JEACH_JSON ) ଜାରି;
    iCol = pConstraint->iColumn - JEACH_JSON;
    ପ୍ରତିଜ୍ଞା( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    ଯଦି( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }ଅନ୍ୟଥା ଯଦି( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  ଯଦି( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  ଯଦି( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    ଫେରନ୍ତୁ SQLITE_CONSTRAINT;
  }
  ଯଦି( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_ଅଙ୍କ = 0;
  }ଅନ୍ୟଥା{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ଯଦି( aIdx[1]<0 ){
      pIdxInfo->idx_ଅଙ୍କ = 1;  /* Only JSON supplied.  Plan 1 */
    }ଅନ୍ୟଥା{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_ଅଙ୍କ = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Start a search on a new JSON string */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ଅଚଳ ଅକ୍ଷର *z_ମୂଳ = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  ଯଦି( idx_ଅଙ୍କ==0 ) ଫେରନ୍ତୁ SQLITE_OK;
  memset(&p->sParse, 0, ଆକାର(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  ଯଦି( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }ଅନ୍ୟଥା{
    p->sParse.zJson = (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    p->sParse.nJson = sqlite3_ମୂଲ୍ୟ_bytes(argv[0]);
    ଯଦି( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      ଫେରନ୍ତୁ SQLITE_OK;
    }      
    ଯଦି( jsonConvertTextToBlob(&p->sParse, 0) ){
      ଯଦି( p->sParse.oom ){
        ଫେରନ୍ତୁ SQLITE_NOMEM;
      }
      ଫେରନ୍ତୁ json_Each_Malformed_ନିବେଶ(cur);
    }
  }
  ଯଦି( idx_ଅଙ୍କ==3 ){
    z_ମୂଳ = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]);
    ଯଦି( z_ମୂଳ==0 ) ଫେରନ୍ତୁ SQLITE_OK;
    ଯଦି( z_ମୂଳ[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_ସନ୍ଦେଶ);
      cur->pVtab->z_Err_ସନ୍ଦେଶ = jsonBadPathError(0, z_ମୂଳ, 0);
      jsonEachCursorReset(p);
      ଫେରନ୍ତୁ cur->pVtab->z_Err_ସନ୍ଦେଶ ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_ମୂଳ = sqlite3Strlen30(z_ମୂଳ);
    ଯଦି( z_ମୂଳ[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }ଅନ୍ୟଥା{
      i = jsonLookupStep(&p->sParse, 0, z_ମୂଳ+1, 0);
      ଯଦି( JSON_LOOKUP_ISERROR(i) ){
        ଯଦି( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          ଫେରନ୍ତୁ SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_ସନ୍ଦେଶ);
        cur->pVtab->z_Err_ସନ୍ଦେଶ = jsonBadPathError(0, z_ମୂଳ, 0);
        jsonEachCursorReset(p);
        ଫେରନ୍ତୁ cur->pVtab->z_Err_ସନ୍ଦେଶ ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      ଯଦି( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }ଅନ୍ୟଥା{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_ମୂଳ, p->n_ମୂଳ);
  }ଅନ୍ୟଥା{
    i = p->i = 0;
    p->eType = 0;
    p->n_ମୂଳ = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  ଯଦି( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, ଆକାର(JsonParent));
    ଯଦି( p->aParent==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_ଚାବି = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].i_ମୂଲ୍ୟ = i;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* The methods of the json_each virtual table */
ସ୍ଥିର sqlite3_module jsonEachModule = {
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
ଶୂନ୍ୟ sqlite3RegisterJsonFunctions(ଶୂନ୍ୟ){
#ifndef SQLITE_OMIT_JSON
  ସ୍ଥିର FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_ଭର୍ତ୍ତି, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_ଭର୍ତ୍ତି,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_ଭର୍ତ୍ତି,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_ଭର୍ତ୍ତି,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#ଯଦି SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_ମୂଲ୍ୟ, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_ମୂଲ୍ୟ, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_ମୂଲ୍ୟ, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_ମୂଲ୍ୟ, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_ଭର୍ତ୍ତି_Builtin_Funcs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#ଯଦି  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *zName){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  ପ୍ରତିଜ୍ଞା( sqlite3HashFind(&db->aModule, zName)==0 );
  ପାଇଁ(i=0; i<ଆକାର(azModule)/ଆକାର(azModule[0]); i++){
    ଯଦି( sqlite3StrICmp(azModule[i],zName)==0 ){
      ଫେରନ୍ତୁ sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  ଫେରନ୍ତୁ 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
