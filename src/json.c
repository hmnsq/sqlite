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
#include "sqliteInt.h"

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
static const var_na * const jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
static const var_na jsonIsSpace[] = {
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
#define jsonIsspace(x) (jsonIsSpace[(unsigned var_na)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
static const var_na jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
static const var_na jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
static const var_na jsonIsOk[256] = {
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
typedef sa_nracanaa JsonCache JsonCache;
typedef sa_nracanaa JsonString JsonString;
typedef sa_nracanaa JsonParse JsonParse;

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
sa_nracanaa JsonCache {
  sqlite3 *db;                    /* Database connection */
  ចំនួនគត់ nUsed;                      /* Number of active entries in the cache */
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
sa_nracanaa JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  var_na *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  var_na zSpace[100];        /* Initial static space */
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

#define JSON_INSERT_TYPE(X)  (((X)&0xC)>>2)


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
sa_nracanaa JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  var_na *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  ចំនួនគត់ nJson;         /* Length of the zJson string in bytes */
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
  ចំនួនគត់ delta;         /* Size change due to the edit */
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
#បើមិនដូច្នេះ
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
static void jsonReturnStringAsBlob(JsonString*);
static ចំនួនគត់ jsonArgIsJsonb(sqlite3_value *pJson, JsonParse *p);
static u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
static void jsonReturnParse(sqlite3_context*,JsonParse*);
static JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_value*,u32);
static void jsonParseFree(JsonParse*);
static u32 jsonbPayloadSize(const JsonParse*, u32, u32*);
static u32 jsonUnescapeOneChar(const var_na*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
static void jsonCacheDelete(JsonCache *p){
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
static void jsonCacheDeleteGeneric(void *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
static ចំនួនគត់ jsonCacheInsert(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  assert( pParse->zJson!=0 );
  assert( pParse->bJsonIsRCStr );
  assert( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ប្រសិនបើ( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, parimaa_na(*p));
    ប្រសិនបើ( p==0 ) ត្រឡប់ SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    ប្រសិនបើ( p==0 ) ត្រឡប់ SQLITE_NOMEM;
  }
  ប្រសិនបើ( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*parimaa_na(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  assert( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  ត្រឡប់ SQLITE_OK;
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
static JsonParse *jsonCacheSearch(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_value *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  ចំនួនគត់ i;
  const var_na *zJson;
  ចំនួនគត់ nJson;

  ប្រសិនបើ( sqlite3_value_type(pArg)!=SQLITE_TEXT ){
    ត្រឡប់ 0;
  }
  zJson = (const var_na*)sqlite3_value_text(pArg);
  ប្រសិនបើ( zJson==0 ) ត្រឡប់ 0;
  nJson = sqlite3_value_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ប្រសិនបើ( p==0 ){
    ត្រឡប់ 0;
  }
  សម្រាប់(i=0; i<p->nUsed; i++){
    ប្រសិនបើ( p->a[i]->zJson==zJson ) bha_nga;
  }
  ប្រសិនបើ( i>=p->nUsed ){
    សម្រាប់(i=0; i<p->nUsed; i++){
      ប្រសិនបើ( p->a[i]->nJson!=nJson ) continue;
      ប្រសិនបើ( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) bha_nga;
    }
  }
  ប្រសិនបើ( i<p->nUsed ){
    ប្រសិនបើ( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*parimaa_na(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    assert( p->a[i]->delta==0 );
    ត្រឡប់ p->a[i];
  }បើមិនដូច្នេះ{
    ត្រឡប់ 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
static void jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = parimaa_na(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
static void jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
static void jsonStringReset(JsonString *p){
  ប្រសិនបើ( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
static void jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  ប្រសិនបើ( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
static void jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  assert( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
static ចំនួនគត់ jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  var_na *zNew;
  ប្រសិនបើ( p->bStatic ){
    ប្រសិនបើ( p->eErr ) ត្រឡប់ 1;
    zNew = sqlite3RCStrNew(nTotal);
    ប្រសិនបើ( zNew==0 ){
      jsonStringOom(p);
      ត្រឡប់ SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }បើមិនដូច្នេះ{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    ប្រសិនបើ( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      ត្រឡប់ SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  ត្រឡប់ SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
static SQLITE_NOINLINE void jsonStringExpandAndAppend(
  JsonString *p,
  const var_na *zIn,
  u32 N
){
  assert( N>0 );
  ប្រសិនបើ( jsonStringGrow(p,N) ) ត្រឡប់;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
static void jsonAppendRaw(JsonString *p, const var_na *zIn, u32 N){
  ប្រសិនបើ( N==0 ) ត្រឡប់;
  ប្រសិនបើ( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }បើមិនដូច្នេះ{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
static void jsonAppendRawNZ(JsonString *p, const var_na *zIn, u32 N){
  assert( N>0 );
  ប្រសិនបើ( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }បើមិនដូច្នេះ{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
static void jsonPrintf(ចំនួនគត់ N, JsonString *p, const var_na *zFormat, ...){
  va_list ap;
  ប្រសិនបើ( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) ត្រឡប់;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (ចំនួនគត់)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
static SQLITE_NOINLINE void jsonAppendCharExpand(JsonString *p, var_na c){
  ប្រសិនបើ( jsonStringGrow(p,1) ) ត្រឡប់;
  p->zBuf[p->nUsed++] = c;
}
static void jsonAppendChar(JsonString *p, var_na c){
  ប្រសិនបើ( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }បើមិនដូច្នេះ{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
static void jsonStringTrimOneChar(JsonString *p){
  ប្រសិនបើ( p->eErr==0 ){
    assert( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
static ចំនួនគត់ jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  ត្រឡប់ p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
static void jsonAppendSeparator(JsonString *p){
  var_na c;
  ប្រសិនបើ( p->nUsed==0 ) ត្រឡប់;
  c = p->zBuf[p->nUsed-1];
  ប្រសិនបើ( c=='[' || c=='{' ) ត្រឡប់;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
static void jsonAppendControlChar(JsonString *p, u8 c){
  static const var_na aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  assert( parimaa_na(aSpecial)==32 );
  assert( aSpecial['\b']=='b' );
  assert( aSpecial['\f']=='f' );
  assert( aSpecial['\n']=='n' );
  assert( aSpecial['\r']=='r' );
  assert( aSpecial['\t']=='t' );
  assert( c>=0 && c<parimaa_na(aSpecial) );
  assert( p->nUsed+7 <= p->nAlloc );
  ប្រសិនបើ( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }បើមិនដូច្នេះ{
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
static void jsonAppendString(JsonString *p, const var_na *zIn, u32 N){
  u32 k;
  u8 c;
  const u8 *z = (const u8*)zIn;
  ប្រសិនបើ( z==0 ) ត្រឡប់;
  ប្រសិនបើ( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) ត្រឡប់;
  p->zBuf[p->nUsed++] = '"';
  ខណៈពេល( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    ខណៈពេល( 1 /* Exit by break */ ){
      ប្រសិនបើ( k+3>=N ){
        ខណៈពេល( k<N && jsonIsOk[z[k]] ){ k++; }
        bha_nga;
      }
      ប្រសិនបើ( !jsonIsOk[z[k]] ){
        bha_nga;
      }
      ប្រសិនបើ( !jsonIsOk[z[k+1]] ){
        k += 1;
        bha_nga;
      }
      ប្រសិនបើ( !jsonIsOk[z[k+2]] ){
        k += 2;
        bha_nga;
      }
      ប្រសិនបើ( !jsonIsOk[z[k+3]] ){
        k += 3;
        bha_nga;
      }បើមិនដូច្នេះ{
        k += 4;
      }
    }
    ប្រសិនបើ( k>=N ){
      ប្រសិនបើ( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      bha_nga;
    }
    ប្រសិនបើ( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    ប្រសិនបើ( c=='"' || c=='\\' ){
      ប្រសិនបើ( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) ត្រឡប់;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }បើមិនដូច្នេះ ប្រសិនបើ( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }បើមិនដូច្នេះ{
      ប្រសិនបើ( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) ត្រឡប់;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  assert( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
static void jsonAppendSqlValue(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_value *pValue          /* Value to append */
){
  switch( sqlite3_value_type(pValue) ){
    pak_sa SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      bha_nga;
    }
    pak_sa SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_value_double(pValue));
      bha_nga;
    }
    pak_sa SQLITE_INTEGER: {
      const var_na *z = (const var_na*)sqlite3_value_text(pValue);
      u32 n = (u32)sqlite3_value_bytes(pValue);
      jsonAppendRaw(p, z, n);
      bha_nga;
    }
    pak_sa SQLITE_TEXT: {
      const var_na *z = (const var_na*)sqlite3_value_text(pValue);
      u32 n = (u32)sqlite3_value_bytes(pValue);
      ប្រសិនបើ( sqlite3_value_subtype(pValue)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }បើមិនដូច្នេះ{
        jsonAppendString(p, z, n);
      }
      bha_nga;
    }
    default: {
      JsonParse px;
      memset(&px, 0, parimaa_na(px));
      ប្រសិនបើ( jsonArgIsJsonb(pValue, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }បើមិនដូច្នេះ ប្រសិនបើ( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      bha_nga;
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
static void jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  assert( (pParse!=0)==(ctx!=0) );
  assert( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  ប្រសិនបើ( p->eErr==0 ){
    ចំនួនគត់ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    ប្រសិនបើ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }បើមិនដូច្នេះ ប្រសិនបើ( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }បើមិនដូច្នេះ{
      ប្រសិនបើ( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        ចំនួនគត់ rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = jsonCacheInsert(ctx, pParse);
        ប្រសិនបើ( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          ត្រឡប់;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }បើមិនដូច្នេះ ប្រសិនបើ( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }បើមិនដូច្នេះ ប្រសិនបើ( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }បើមិនដូច្នេះ ប្រសិនបើ( p->eErr & JSTRING_MALFORMED ){
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
static void jsonParseReset(JsonParse *pParse){
  assert( pParse->nJPRef<=1 );
  ប្រសិនបើ( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  ប្រសិនបើ( pParse->nBlobAlloc ){
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
static void jsonParseFree(JsonParse *pParse){
  ប្រសិនបើ( pParse ){
    ប្រសិនបើ( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }បើមិនដូច្នេះ{
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
static u8 jsonHexToInt(ចំនួនគត់ h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  ត្រឡប់ (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
static u32 jsonHexToInt4(const var_na *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  ត្រឡប់ v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
static ចំនួនគត់ jsonIs2Hex(const var_na *z){
  ត្រឡប់ sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
static ចំនួនគត់ jsonIs4Hex(const var_na *z){
  ត្រឡប់ jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
static ចំនួនគត់ json5Whitespace(const var_na *zIn){
  ចំនួនគត់ n = 0;
  const u8 *z = (u8*)zIn;
  ខណៈពេល( 1 /*exit by "goto whitespace_done"*/ ){
    switch( z[n] ){
      pak_sa 0x09:
      pak_sa 0x0a:
      pak_sa 0x0b:
      pak_sa 0x0c:
      pak_sa 0x0d:
      pak_sa 0x20: {
        n++;
        bha_nga;
      }
      pak_sa '/': {
        ប្រសិនបើ( z[n+1]=='*' && z[n+2]!=0 ){
          ចំនួនគត់ j;
          សម្រាប់(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            ប្រសិនបើ( z[j]==0 ) goto whitespace_done;
          }
          n = j+1;
          bha_nga;
        }បើមិនដូច្នេះ ប្រសិនបើ( z[n+1]=='/' ){
          ចំនួនគត់ j;
          var_na c;
          សម្រាប់(j=n+2; (c = z[j])!=0; j++){
            ប្រសិនបើ( c=='\n' || c=='\r' ) bha_nga;
            ប្រសិនបើ( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              bha_nga;
            }
          }
          n = j;
          ប្រសិនបើ( z[n] ) n++;
          bha_nga;
        }
        goto whitespace_done;
      }
      pak_sa 0xc2: {
        ប្រសិនបើ( z[n+1]==0xa0 ){
          n += 2;
          bha_nga;
        }
        goto whitespace_done;
      }
      pak_sa 0xe1: {
        ប្រសិនបើ( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          bha_nga;
        }
        goto whitespace_done;
      }
      pak_sa 0xe2: {
        ប្រសិនបើ( z[n+1]==0x80 ){
          u8 c = z[n+2];
          ប្រសិនបើ( c<0x80 ) goto whitespace_done;
          ប្រសិនបើ( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            bha_nga;
          }
        }បើមិនដូច្នេះ ប្រសិនបើ( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          bha_nga;
        }
        goto whitespace_done;
      }
      pak_sa 0xe3: {
        ប្រសិនបើ( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          bha_nga;
        }
        goto whitespace_done;
      }
      pak_sa 0xef: {
        ប្រសិនបើ( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          bha_nga;
        }
        goto whitespace_done;
      }
      default: {
        goto whitespace_done;
      }
    }
  }
  whitespace_done:
  ត្រឡប់ n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
static const sa_nracanaa NanInfName {
  var_na c1;
  var_na c2;
  var_na n;
  var_na eType;
  var_na nRepl;
  var_na *zMatch;
  var_na *zRepl;
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
static void jsonWrongNumArgs(
  sqlite3_context *pCtx,
  const var_na *zFuncName
){
  var_na *zMsg = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, zMsg, -1);
  sqlite3_free(zMsg);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
static ចំនួនគត់ jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  assert( N>pParse->nBlobAlloc );
  ប្រសិនបើ( pParse->nBlobAlloc==0 ){
    t = 100;
  }បើមិនដូច្នេះ{
    t = pParse->nBlobAlloc*2;
  }
  ប្រសិនបើ( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  ប្រសិនបើ( aNew==0 ){ pParse->oom = 1; ត្រឡប់ 1; }
  assert( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  ត្រឡប់ 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
static ចំនួនគត់ jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 nSize;
  assert( !pParse->bReadOnly );
  ប្រសិនបើ( pParse->oom ) ត្រឡប់ 0;
  ប្រសិនបើ( pParse->nBlobAlloc>0 ) ត្រឡប់ 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  ប្រសិនបើ( jsonBlobExpand(pParse, nSize) ){
    ត្រឡប់ 0;
  }
  assert( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  ត្រឡប់ 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
static SQLITE_NOINLINE void jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  ប្រសិនបើ( pParse->oom==0 ){
    assert( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
static void jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  ប្រសិនបើ( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }បើមិនដូច្នេះ{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
static void jsonBlobAppendNode(JsonParse*,u8,u64,const void*);
static SQLITE_NOINLINE void jsonBlobExpandAndAppendNode(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  const void *aPayload
){
  ប្រសិនបើ( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) ត្រឡប់;
  jsonBlobAppendNode(pParse, eType, szPayload, aPayload);
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
static void jsonBlobAppendNode(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  const void *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  ប្រសិនបើ( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendNode(pParse,eType,szPayload,aPayload);
    ត្រឡប់;
  }
  assert( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  ប្រសិនបើ( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }បើមិនដូច្នេះ ប្រសិនបើ( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }បើមិនដូច្នេះ ប្រសិនបើ( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }បើមិនដូច្នេះ{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  ប្រសិនបើ( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
static ចំនួនគត់ jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  ចំនួនគត់ delta;
  ប្រសិនបើ( pParse->oom ) ត្រឡប់ 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  ប្រសិនបើ( szType<=11 ){
    nExtra = 0;
  }បើមិនដូច្នេះ ប្រសិនបើ( szType==12 ){
    nExtra = 1;
  }បើមិនដូច្នេះ ប្រសិនបើ( szType==13 ){
    nExtra = 2;
  }បើមិនដូច្នេះ ប្រសិនបើ( szType==14 ){
    nExtra = 4;
  }បើមិនដូច្នេះ{
    nExtra = 8;
  }
  ប្រសិនបើ( szPayload<=11 ){
    nNeeded = 0;
  }បើមិនដូច្នេះ ប្រសិនបើ( szPayload<=0xff ){
    nNeeded = 1;
  }បើមិនដូច្នេះ ប្រសិនបើ( szPayload<=0xffff ){
    nNeeded = 2;
  }បើមិនដូច្នេះ{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  ប្រសិនបើ( delta ){
    u64 newSize = (u64)pParse->nBlob + delta;
    ប្រសិនបើ( delta>0 ){
      ប្រសិនបើ( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        ត្រឡប់ 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }បើមិនដូច្នេះ{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  ប្រសិនបើ( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }បើមិនដូច្នេះ ប្រសិនបើ( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }បើមិនដូច្នេះ ប្រសិនបើ( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }បើមិនដូច្នេះ{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  ត្រឡប់ delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
static ចំនួនគត់ jsonIs4HexB(const var_na *z, ចំនួនគត់ *pOp){
  ប្រសិនបើ( z[0]!='u' ) ត្រឡប់ 0;
  ប្រសិនបើ( !jsonIs4Hex(&z[1]) ) ត្រឡប់ 0;
  *pOp = JSONB_TEXTJ;
  ត្រឡប់ 1;
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
static u32 jsonbValidityCheck(
  const JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  const u8 *z;
  u8 x;
  ប្រសិនបើ( iDepth>JSON_MAX_DEPTH ) ត្រឡប់ i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  ប្រសិនបើ( NEVER(n==0) ) ត្រឡប់ i+1;          /* Checked by caller */
  ប្រសិនបើ( NEVER(i+n+sz!=iEnd) ) ត្រឡប់ i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  switch( x ){
    pak_sa JSONB_NULL:
    pak_sa JSONB_TRUE:
    pak_sa JSONB_FALSE: {
      ត្រឡប់ n+sz==1 ? 0 : i+1;
    }
    pak_sa JSONB_INT: {
      ប្រសិនបើ( sz<1 ) ត្រឡប់ i+1;
      j = i+n;
      ប្រសិនបើ( z[j]=='-' ){
        j++;
        ប្រសិនបើ( sz<2 ) ត្រឡប់ i+1;
      }
      k = i+n+sz;
      ខណៈពេល( j<k ){
        ប្រសិនបើ( sqlite3Isdigit(z[j]) ){
          j++;
        }បើមិនដូច្នេះ{
          ត្រឡប់ j+1;
        }
      }
      ត្រឡប់ 0;
    }
    pak_sa JSONB_INT5: {
      ប្រសិនបើ( sz<3 ) ត្រឡប់ i+1;
      j = i+n;
      ប្រសិនបើ( z[j]=='-' ){
        ប្រសិនបើ( sz<4 ) ត្រឡប់ i+1;
        j++;
      }
      ប្រសិនបើ( z[j]!='0' ) ត្រឡប់ i+1;
      ប្រសិនបើ( z[j+1]!='x' && z[j+1]!='X' ) ត្រឡប់ j+2;
      j += 2;
      k = i+n+sz;
      ខណៈពេល( j<k ){
        ប្រសិនបើ( sqlite3Isxdigit(z[j]) ){
          j++;
        }បើមិនដូច្នេះ{
          ត្រឡប់ j+1;
        }
      }
      ត្រឡប់ 0;
    }
    pak_sa JSONB_FLOAT:
    pak_sa JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      ប្រសិនបើ( sz<2 ) ត្រឡប់ i+1;
      j = i+n;
      k = j+sz;
      ប្រសិនបើ( z[j]=='-' ){
        j++;
        ប្រសិនបើ( sz<3 ) ត្រឡប់ i+1;
      }
      ប្រសិនបើ( z[j]=='.' ){
        ប្រសិនបើ( x==JSONB_FLOAT ) ត្រឡប់ j+1;
        ប្រសិនបើ( !sqlite3Isdigit(z[j+1]) ) ត្រឡប់ j+1;
        j += 2;
        seen = 1;
      }បើមិនដូច្នេះ ប្រសិនបើ( z[j]=='0' && x==JSONB_FLOAT ){
        ប្រសិនបើ( j+3>k ) ត្រឡប់ j+1;
        ប្រសិនបើ( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) ត្រឡប់ j+1;
        j++;
      }
      សម្រាប់(; j<k; j++){
        ប្រសិនបើ( sqlite3Isdigit(z[j]) ) continue;
        ប្រសិនបើ( z[j]=='.' ){
          ប្រសិនបើ( seen>0 ) ត្រឡប់ j+1;
          ប្រសិនបើ( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            ត្រឡប់ j+1;
          }
          seen = 1;
          continue;
        }
        ប្រសិនបើ( z[j]=='e' || z[j]=='E' ){
          ប្រសិនបើ( seen==2 ) ត្រឡប់ j+1;
          ប្រសិនបើ( j==k-1 ) ត្រឡប់ j+1;
          ប្រសិនបើ( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            ប្រសិនបើ( j==k-1 ) ត្រឡប់ j+1;
          }
          seen = 2;
          continue;
        }
        ត្រឡប់ j+1;
      }
      ប្រសិនបើ( seen==0 ) ត្រឡប់ i+1;
      ត្រឡប់ 0;
    }
    pak_sa JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      ខណៈពេល( j<k ){
        ប្រសិនបើ( !jsonIsOk[z[j]] && z[j]!='\'' ) ត្រឡប់ j+1;
        j++;
      }
      ត្រឡប់ 0;
    }
    pak_sa JSONB_TEXTJ:
    pak_sa JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      ខណៈពេល( j<k ){
        ប្រសិនបើ( !jsonIsOk[z[j]] && z[j]!='\'' ){
          ប្រសិនបើ( z[j]=='"' ){
            ប្រសិនបើ( x==JSONB_TEXTJ ) ត្រឡប់ j+1;
          }បើមិនដូច្នេះ ប្រសិនបើ( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            ប្រសិនបើ( x==JSONB_TEXTJ ) ត្រឡប់ j+1;
          }បើមិនដូច្នេះ ប្រសិនបើ( NEVER(z[j]!='\\') || j+1>=k ){
            ត្រឡប់ j+1;
          }បើមិនដូច្នេះ ប្រសិនបើ( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }បើមិនដូច្នេះ ប្រសិនបើ( z[j+1]=='u' ){
            ប្រសិនបើ( j+5>=k ) ត្រឡប់ j+1;
            ប្រសិនបើ( !jsonIs4Hex((const var_na*)&z[j+2]) ) ត្រឡប់ j+1;
            j++;
          }បើមិនដូច្នេះ ប្រសិនបើ( x!=JSONB_TEXT5 ){
            ត្រឡប់ j+1;
          }បើមិនដូច្នេះ{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((const var_na*)&z[j], k-j, &c);
            ប្រសិនបើ( c==JSON_INVALID_CHAR ) ត្រឡប់ j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      ត្រឡប់ 0;
    }
    pak_sa JSONB_TEXTRAW: {
      ត្រឡប់ 0;
    }
    pak_sa JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      ខណៈពេល( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        ប្រសិនបើ( n==0 ) ត្រឡប់ j+1;
        ប្រសិនបើ( j+n+sz>k ) ត្រឡប់ j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        ប្រសិនបើ( sub ) ត្រឡប់ sub;
        j += n + sz;
      }
      assert( j==k );
      ត្រឡប់ 0;
    }
    pak_sa JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      ខណៈពេល( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        ប្រសិនបើ( n==0 ) ត្រឡប់ j+1;
        ប្រសិនបើ( j+n+sz>k ) ត្រឡប់ j+1;
        ប្រសិនបើ( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          ប្រសិនបើ( x<JSONB_TEXT || x>JSONB_TEXTRAW ) ត្រឡប់ j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        ប្រសិនបើ( sub ) ត្រឡប់ sub;
        cnt++;
        j += n + sz;
      }
      assert( j==k );
      ប្រសិនបើ( (cnt & 1)!=0 ) ត្រឡប់ j+1;
      ត្រឡប់ 0;
    }
    default: {
      ត្រឡប់ i+1;
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
static ចំនួនគត់ jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  var_na c;
  u32 j;
  u32 iThis, iStart;
  ចំនួនគត់ x;
  u8 t;
  const var_na *z = pParse->zJson;
json_parse_restart:
  switch( (u8)z[i] ){
  pak_sa '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    jsonBlobAppendNode(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    ប្រសិនបើ( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      ត្រឡប់ -1;
    }
    iStart = pParse->nBlob;
    សម្រាប់(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      ប្រសិនបើ( x<=0 ){
        ចំនួនគត់ op;
        ប្រសិនបើ( x==(-2) ){
          j = pParse->iErr;
          ប្រសិនបើ( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          bha_nga;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        ប្រសិនបើ( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          ចំនួនគត់ k = j+1;
          ខណៈពេល( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          assert( iBlob==pParse->nBlob );
          jsonBlobAppendNode(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }បើមិនដូច្នេះ{
          ប្រសិនបើ( x!=-1 ) pParse->iErr = j;
          ត្រឡប់ -1;
        }
      }
      ប្រសិនបើ( pParse->oom ) ត្រឡប់ -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      ប្រសិនបើ( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        ត្រឡប់ -1;
      }
      j = x;
      ប្រសិនបើ( z[j]==':' ){
        j++;
      }បើមិនដូច្នេះ{
        ប្រសិនបើ( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          do{ j++; }ខណៈពេល( jsonIsspace(z[j]) );
          ប្រសិនបើ( z[j]==':' ){
            j++;
            goto parse_object_value;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ប្រសិនបើ( x!=(-5) ){
          ប្រសិនបើ( x!=(-1) ) pParse->iErr = j;
          ត្រឡប់ -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_value:
      x = jsonTranslateTextToBlob(pParse, j);
      ប្រសិនបើ( x<=0 ){
        ប្រសិនបើ( x!=(-1) ) pParse->iErr = j;
        ត្រឡប់ -1;
      }
      j = x;
      ប្រសិនបើ( z[j]==',' ){
        continue;
      }បើមិនដូច្នេះ ប្រសិនបើ( z[j]=='}' ){
        bha_nga;
      }បើមិនដូច្នេះ{
        ប្រសិនបើ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ប្រសិនបើ( z[j]==',' ){
            continue;
          }បើមិនដូច្នេះ ប្រសិនបើ( z[j]=='}' ){
            bha_nga;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ប្រសិនបើ( x==(-4) ){
          j = pParse->iErr;
          continue;
        }
        ប្រសិនបើ( x==(-2) ){
          j = pParse->iErr;
          bha_nga;
        }
      }
      pParse->iErr = j;
      ត្រឡប់ -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    ត្រឡប់ j+1;
  }
  pak_sa '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    assert( i<=(u32)pParse->nJson );
    jsonBlobAppendNode(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    ប្រសិនបើ( pParse->oom ) ត្រឡប់ -1;
    ប្រសិនបើ( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      ត្រឡប់ -1;
    }
    សម្រាប់(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      ប្រសិនបើ( x<=0 ){
        ប្រសិនបើ( x==(-3) ){
          j = pParse->iErr;
          ប្រសិនបើ( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          bha_nga;
        }
        ប្រសិនបើ( x!=(-1) ) pParse->iErr = j;
        ត្រឡប់ -1;
      }
      j = x;
      ប្រសិនបើ( z[j]==',' ){
        continue;
      }បើមិនដូច្នេះ ប្រសិនបើ( z[j]==']' ){
        bha_nga;
      }បើមិនដូច្នេះ{
        ប្រសិនបើ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ប្រសិនបើ( z[j]==',' ){
            continue;
          }បើមិនដូច្នេះ ប្រសិនបើ( z[j]==']' ){
            bha_nga;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        ប្រសិនបើ( x==(-4) ){
          j = pParse->iErr;
          continue;
        }
        ប្រសិនបើ( x==(-3) ){
          j = pParse->iErr;
          bha_nga;
        }
      }
      pParse->iErr = j;
      ត្រឡប់ -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    ត្រឡប់ j+1;
  }
  pak_sa '\'': {
    u8 opcode;
    var_na cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    goto parse_string;
  pak_sa '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    ខណៈពេល( 1 /*exit-by-break*/ ){
      ប្រសិនបើ( jsonIsOk[(u8)z[j]] ){
        ប្រសិនបើ( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }បើមិនដូច្នេះ ប្រសិនបើ( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }បើមិនដូច្នេះ{
          j += 3;
          continue;
        }
      }
      c = z[j];
      ប្រសិនបើ( c==cDelim ){
        bha_nga;
      }បើមិនដូច្នេះ ប្រសិនបើ( c=='\\' ){
        c = z[++j];
        ប្រសិនបើ( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          ប្រសិនបើ( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }បើមិនដូច្នេះ ប្រសិនបើ( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#បើមិនដូច្នេះ
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }បើមិនដូច្នេះ ប្រសិនបើ( c=='\r' ){
          ប្រសិនបើ( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }បើមិនដូច្នេះ{
          pParse->iErr = j;
          ត្រឡប់ -1;
        }
      }បើមិនដូច្នេះ ប្រសិនបើ( c<=0x1f ){
        ប្រសិនបើ( c==0 ){
          pParse->iErr = j;
          ត្រឡប់ -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }បើមិនដូច្នេះ ប្រសិនបើ( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    jsonBlobAppendNode(pParse, opcode, j-1-i, &z[i+1]);
    ត្រឡប់ j+1;
  }
  pak_sa 't': {
    ប្រសិនបើ( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      ត្រឡប់ i+4;
    }
    pParse->iErr = i;
    ត្រឡប់ -1;
  }
  pak_sa 'f': {
    ប្រសិនបើ( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      ត្រឡប់ i+5;
    }
    pParse->iErr = i;
    ត្រឡប់ -1;
  }
  pak_sa '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    goto parse_number;
  pak_sa '.':
    ប្រសិនបើ( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      goto parse_number_2;
    }
    pParse->iErr = i;
    ត្រឡប់ -1;
  pak_sa '-':
  pak_sa '0':
  pak_sa '1':
  pak_sa '2':
  pak_sa '3':
  pak_sa '4':
  pak_sa '5':
  pak_sa '6':
  pak_sa '7':
  pak_sa '8':
  pak_sa '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_number:
    seenE = 0;
    assert( '-' < '0' );
    assert( '+' < '0' );
    assert( '.' < '0' );
    c = z[i];

    ប្រសិនបើ( c<='0' ){
      ប្រសិនបើ( c=='0' ){
        ប្រសិនបើ( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          assert( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          សម្រាប់(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          goto parse_number_finish;
        }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          ត្រឡប់ -1;
        }
      }បើមិនដូច្នេះ{
        ប្រសិនបើ( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          ប្រសិនបើ( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            ប្រសិនបើ( z[i]=='-' ){
              jsonBlobAppendNode(pParse, JSONB_FLOAT, 6, "-9e999");
            }បើមិនដូច្នេះ{
              jsonBlobAppendNode(pParse, JSONB_FLOAT, 5, "9e999");
            }
            ត្រឡប់ i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          ប្រសិនបើ( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            goto parse_number_2;
          }
          pParse->iErr = i;
          ត្រឡប់ -1;
        }
        ប្រសិនបើ( z[i+1]=='0' ){
          ប្រសិនបើ( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            ត្រឡប់ -1;
          }បើមិនដូច្នេះ ប្រសិនបើ( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            សម្រាប់(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            goto parse_number_finish;
          }
        }
      }
    }

  parse_number_2:
    សម្រាប់(j=i+1;; j++){
      c = z[j];
      ប្រសិនបើ( sqlite3Isdigit(c) ) continue;
      ប្រសិនបើ( c=='.' ){
        ប្រសិនបើ( (t & 0x02)!=0 ){
          pParse->iErr = j;
          ត្រឡប់ -1;
        }
        t |= 0x02;
        continue;
      }
      ប្រសិនបើ( c=='e' || c=='E' ){
        ប្រសិនបើ( z[j-1]<'0' ){
          ប្រសិនបើ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }បើមិនដូច្នេះ{
            pParse->iErr = j;
            ត្រឡប់ -1;
          }
        }
        ប្រសិនបើ( seenE ){
          pParse->iErr = j;
          ត្រឡប់ -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        ប្រសិនបើ( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        ប្រសិនបើ( c<'0' || c>'9' ){
          pParse->iErr = j;
          ត្រឡប់ -1;
        }
        continue;
      }
      bha_nga;
    }
    ប្រសិនបើ( z[j-1]<'0' ){
      ប្រសិនបើ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }បើមិនដូច្នេះ{
        pParse->iErr = j;
        ត្រឡប់ -1;
      }
    }
  parse_number_finish:
    assert( JSONB_INT+0x01==JSONB_INT5 );
    assert( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    assert( JSONB_INT+0x02==JSONB_FLOAT );
    ប្រសិនបើ( z[i]=='+' ) i++;
    jsonBlobAppendNode(pParse, JSONB_INT+t, j-i, &z[i]);
    ត្រឡប់ j;
  }
  pak_sa '}': {
    pParse->iErr = i;
    ត្រឡប់ -2;  /* End of {...} */
  }
  pak_sa ']': {
    pParse->iErr = i;
    ត្រឡប់ -3;  /* End of [...] */
  }
  pak_sa ',': {
    pParse->iErr = i;
    ត្រឡប់ -4;  /* List separator */
  }
  pak_sa ':': {
    pParse->iErr = i;
    ត្រឡប់ -5;  /* Object label/value separator */
  }
  pak_sa 0: {
    ត្រឡប់ 0;   /* End of file */
  }
  pak_sa 0x09:
  pak_sa 0x0a:
  pak_sa 0x0d:
  pak_sa 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    goto json_parse_restart;
  }
  pak_sa 0x0b:
  pak_sa 0x0c:
  pak_sa '/':
  pak_sa 0xc2:
  pak_sa 0xe1:
  pak_sa 0xe2:
  pak_sa 0xe3:
  pak_sa 0xef: {
    j = json5Whitespace(&z[i]);
    ប្រសិនបើ( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      goto json_parse_restart;
    }
    pParse->iErr = i;
    ត្រឡប់ -1;
  }
  pak_sa 'n': {
    ប្រសិនបើ( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      ត្រឡប់ i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  default: {
    u32 k;
    ចំនួនគត់ nn;
    c = z[i];
    សម្រាប់(k=0; k<parimaa_na(aNanInfName)/parimaa_na(aNanInfName[0]); k++){
      ប្រសិនបើ( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) continue;
      nn = aNanInfName[k].n;
      ប្រសិនបើ( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        continue;
      }
      ប្រសិនបើ( sqlite3Isalnum(z[i+nn]) ) continue;
      ប្រសិនបើ( aNanInfName[k].eType==JSONB_FLOAT ){
        jsonBlobAppendNode(pParse, JSONB_FLOAT, 5, "9e999");
      }បើមិនដូច្នេះ{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      ត្រឡប់ i + nn;
    }
    pParse->iErr = i;
    ត្រឡប់ -1;  /* Syntax error */
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
static ចំនួនគត់ jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  ចំនួនគត់ i;
  const var_na *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  ប្រសិនបើ( pParse->oom ) i = -1;
  ប្រសិនបើ( i>0 ){
#ifdef SQLITE_DEBUG
    assert( pParse->iDepth==0 );
    ប្រសិនបើ( sqlite3Config.bJsonSelfcheck ){
      assert( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    ខណៈពេល( jsonIsspace(zJson[i]) ) i++;
    ប្រសិនបើ( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      ប្រសិនបើ( zJson[i] ){
        ប្រសិនបើ( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        ត្រឡប់ 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  ប្រសិនបើ( i<=0 ){
    ប្រសិនបើ( pCtx!=0 ){
      ប្រសិនបើ( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }បើមិនដូច្នេះ{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    ត្រឡប់ 1;
  }
  ត្រឡប់ 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
static void jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  assert( pStr->eErr==0 );
  memset(&px, 0, parimaa_na(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (void)jsonTranslateTextToBlob(&px, 0);
  ប្រសិនបើ( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }បើមិនដូច្នេះ{
    assert( px.nBlobAlloc>0 );
    assert( !px.bReadOnly );
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
static u32 jsonbPayloadSizeWide(
  const JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  assert( i<pParse->nBlob );
  assert( x==(pParse->aBlob[i]>>4) );
  assert( x>=13 );
  ប្រសិនបើ( x==13 ){
    ប្រសិនបើ( i+2>=pParse->nBlob ){
      *pSz = 0;
      ត្រឡប់ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }បើមិនដូច្នេះ ប្រសិនបើ( x==14 ){
    ប្រសិនបើ( i+4>=pParse->nBlob ){
      *pSz = 0;
      ត្រឡប់ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }បើមិនដូច្នេះ{
    ប្រសិនបើ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ត្រឡប់ 0;
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
  ប្រសិនបើ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    ត្រឡប់ 0;
  }
  *pSz = sz;
  ត្រឡប់ n;
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
static u32 jsonbPayloadSize(
  const JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ប្រសិនបើ( i>=pParse->nBlob ){
    *pSz = 0;
    ត្រឡប់ 0;
  }បើមិនដូច្នេះ ប្រសិនបើ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }បើមិនដូច្នេះ ប្រសិនបើ( x==12 ){
    ប្រសិនបើ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ត្រឡប់ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }បើមិនដូច្នេះ{
    ត្រឡប់ jsonbPayloadSizeWide(pParse, i, pSz, x);
  }
  ប្រសិនបើ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ត្រឡប់ 0;
  }
  *pSz = sz;
  ត្រឡប់ n;
}

#ប្រសិនបើ SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
static SQLITE_INLINE u32 jsonbPayloadSizeInline(
  const JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ប្រសិនបើ( i>=pParse->nBlob ){
    *pSz = 0;
    ត្រឡប់ 0;
  }បើមិនដូច្នេះ ប្រសិនបើ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }បើមិនដូច្នេះ ប្រសិនបើ( x==12 ){
    ប្រសិនបើ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ត្រឡប់ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }បើមិនដូច្នេះ ប្រសិនបើ( x==13 ){
    ប្រសិនបើ( i+2>=pParse->nBlob ){
      *pSz = 0;
      ត្រឡប់ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }បើមិនដូច្នេះ ប្រសិនបើ( x==14 ){
    ប្រសិនបើ( i+4>=pParse->nBlob ){
      *pSz = 0;
      ត្រឡប់ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }បើមិនដូច្នេះ{
    ប្រសិនបើ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ត្រឡប់ 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  ប្រសិនបើ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ត្រឡប់ 0;
  }
  *pSz = sz;
  ត្រឡប់ n;
}
#បើមិនដូច្នេះ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif


#ប្រសិនបើ SQLITE_USES_INLINE
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
static SQLITE_INLINE u32 jsonbPayloadSizeSemiInline(
  const JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  assert( i<pParse->nBlob );
  ប្រសិនបើ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }បើមិនដូច្នេះ ប្រសិនបើ( x==12 ){
    ប្រសិនបើ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ត្រឡប់ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }បើមិនដូច្នេះ{
    ត្រឡប់ jsonbPayloadSizeWide(pParse,i,pSz,x);
  }
  ប្រសិនបើ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ត្រឡប់ 0;
  }
  *pSz = sz;
  ត្រឡប់ n;
}
#បើមិនដូច្នេះ /* if !SQLITE_USES_INLINE */
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
static u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSizeInline(pParse, i, &sz);
  ប្រសិនបើ( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    ត្រឡប់ pParse->nBlob+1;
  }
  switch( pParse->aBlob[i] & 0x0f ){
    pak_sa JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      ត្រឡប់ i+1;
    }
    pak_sa JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      ត្រឡប់ i+1;
    }
    pak_sa JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      ត្រឡប់ i+1;
    }
    pak_sa JSONB_INT:
    pak_sa JSONB_FLOAT: {
      ប្រសិនបើ( sz==0 ) goto malformed_jsonb;
      jsonAppendRaw(pOut, (const var_na*)&pParse->aBlob[i+n], sz);
      bha_nga;
    }
    pak_sa JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      const var_na *zIn = (const var_na*)&pParse->aBlob[i+n];
      ចំនួនគត់ bOverflow = 0;
      ប្រសិនបើ( sz==0 ) goto malformed_jsonb;
      ប្រសិនបើ( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }បើមិនដូច្នេះ ប្រសិនបើ( zIn[0]=='+' ){
        k++;
      }
      សម្រាប់(; k<sz; k++){
        ប្រសិនបើ( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          bha_nga;
        }បើមិនដូច្នេះ ប្រសិនបើ( (u>>60)!=0 ){
          bOverflow = 1;
        }បើមិនដូច្នេះ{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      bha_nga;
    }
    pak_sa JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      const var_na *zIn = (const var_na*)&pParse->aBlob[i+n];
      ប្រសិនបើ( sz==0 ) goto malformed_jsonb;
      ប្រសិនបើ( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        ប្រសិនបើ( sz<=1 ) goto malformed_jsonb;
        k = 1;
      }
      ប្រសិនបើ( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      សម្រាប់(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        ប្រសិនបើ( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      bha_nga;
    }
    pak_sa JSONB_TEXT:
    pak_sa JSONB_TEXTJ: {
      ប្រសិនបើ( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(const var_na*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      bha_nga;
    }
    pak_sa JSONB_TEXT5: {
      const var_na *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (const var_na*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      ខណៈពេល( sz2>0 ){
        សម្រាប់(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        ប្រសិនបើ( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          ប្រសិនបើ( k>=sz2 ){
            bha_nga;
          }
          zIn += k;
          sz2 -= k;
        }
        ប្រសិនបើ( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          continue;
        }
        ប្រសិនបើ( zIn[0]<=0x1f ){
          ប្រសិនបើ( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) bha_nga;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          continue;
        }
        assert( zIn[0]=='\\' );
        assert( sz2>=1 );
        ប្រសិនបើ( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          bha_nga;
        }
        switch( (u8)zIn[1] ){
          pak_sa '\'':
            jsonAppendChar(pOut, '\'');
            bha_nga;
          pak_sa 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            bha_nga;
          pak_sa 'x':
            ប្រសិនបើ( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              bha_nga;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            bha_nga;
          pak_sa '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            bha_nga;
          pak_sa '\r':
            ប្រសិនបើ( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            bha_nga;
          pak_sa '\n':
            bha_nga;
          pak_sa 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            ប្រសិនបើ( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              bha_nga;
            }
            zIn += 2;
            sz2 -= 2;
            bha_nga;
          default:
            jsonAppendRawNZ(pOut, zIn, 2);
            bha_nga;
        }
        assert( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      bha_nga;
    }
    pak_sa JSONB_TEXTRAW: {
      jsonAppendString(pOut, (const var_na*)&pParse->aBlob[i+n], sz);
      bha_nga;
    }
    pak_sa JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      ប្រសិនបើ( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      ខណៈពេល( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      ប្រសិនបើ( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      ប្រសិនបើ( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      bha_nga;
    }
    pak_sa JSONB_OBJECT: {
      ចំនួនគត់ x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      ប្រសិនបើ( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      ខណៈពេល( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      ប្រសិនបើ( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      ប្រសិនបើ( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      bha_nga;
    }

    default: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      bha_nga;
    }
  }
  ត្រឡប់ i+n+sz;
}

/* Context for recursion of json_pretty()
*/
typedef sa_nracanaa JsonPretty JsonPretty;
sa_nracanaa JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  const var_na *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
static void jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  សម្រាប់(jj=0; jj<pPretty->nIndent; jj++){
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
static u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  ប្រសិនបើ( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    ត្រឡប់ pParse->nBlob+1;
  }
  switch( pParse->aBlob[i] & 0x0f ){
    pak_sa JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      ប្រសិនបើ( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        ប្រសិនបើ( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        ខណៈពេល( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          ប្រសិនបើ( j>=iEnd ){
            ប្រសិនបើ( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            bha_nga;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      bha_nga;
    }
    pak_sa JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      ប្រសិនបើ( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        ប្រសិនបើ( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        ខណៈពេល( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          ប្រសិនបើ( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            bha_nga;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          ប្រសិនបើ( j>=iEnd ) bha_nga;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      bha_nga;
    }
    default: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      bha_nga;
    }
  }
  ត្រឡប់ i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
static u32 jsonbArrayCount(JsonParse *pParse, u32 iRoot){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, iRoot, &sz);
  iEnd = iRoot+n+sz;
  សម្រាប់(i=iRoot+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSizeSemiInline(pParse, i, &sz);
  }
  ត្រឡប់ k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
static void jsonAfterEditSizeAdjust(JsonParse *pParse, u32 iRoot){
  u32 sz = 0;
  u32 nBlob;
  assert( pParse->delta!=0 );
  assert( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (void)jsonbPayloadSize(pParse, iRoot, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, iRoot, sz);
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
static ចំនួនគត់ jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  const u8 *aIns,           /* New content */
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
  static const u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  ប្រសិនបើ( (aIns[0]&0x0f)<=2 ) ត្រឡប់ 0;    /* Cannot enlarge NULL, true, false */
  switch( aIns[0]>>4 ){
    default: {                         /* aIns[] header size 1 */
      ប្រសិនបើ( ((1<<d)&0x116)==0 ) ត្រឡប់ 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      bha_nga;
    }
    pak_sa 12: {                         /* aIns[] header size is 2 */
      ប្រសិនបើ( ((1<<d)&0x8a)==0) ត្រឡប់ 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      bha_nga;
    }
    pak_sa 13: {                         /* aIns[] header size is 3 */
      ប្រសិនបើ( d!=2 && d!=6 ) ត្រឡប់ 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      bha_nga;
    }
    pak_sa 14: {                         /* aIns[] header size is 5 */
      ប្រសិនបើ( d!=4 ) ត្រឡប់ 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      bha_nga;
    }
    pak_sa 15: {                         /* aIns[] header size is 9 */
      ត្រឡប់ 0;                          /* No solution */
    }
  }
  assert( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  ខណៈពេល( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    ប្រសិនបើ( i==1 ) bha_nga;
    szPayload >>= 8;
  }
  assert( (szPayload>>8)==0 );
  ត្រឡប់ 1;
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
static void jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  const u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  assert( pParse->nBlob >= (u64)iDel + (u64)nDel );
  ប្រសិនបើ( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (ចំនួនគត់)-d)
  ){
    ត្រឡប់;
  }
  ប្រសិនបើ( d!=0 ){
    ប្រសិនបើ( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      ប្រសិនបើ( pParse->oom ) ត្រឡប់;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  ប្រសិនបើ( nIns && aIns ){
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
static u32 jsonBytesToBypass(const var_na *z, u32 n){
  u32 i = 0;
  ខណៈពេល( i+1<n ){
    ប្រសិនបើ( z[i]!='\\' ) ត្រឡប់ i;
    ប្រសិនបើ( z[i+1]=='\n' ){
      i += 2;
      continue;
    }
    ប្រសិនបើ( z[i+1]=='\r' ){
      ប្រសិនបើ( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }បើមិនដូច្នេះ{
        i += 2;
      }
      continue;
    }
    ប្រសិនបើ( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      continue;
    }
    bha_nga;
  }
  ត្រឡប់ i;
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
static u32 jsonUnescapeOneChar(const var_na *z, u32 n, u32 *piOut){
  assert( n>0 );
  assert( z[0]=='\\' );
  ប្រសិនបើ( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    ត្រឡប់ n;
  }
  switch( (u8)z[1] ){
    pak_sa 'u': {
      u32 v, vlo;
      ប្រសិនបើ( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        ត្រឡប់ n;
      }
      v = jsonHexToInt4(&z[2]);
      ប្រសិនបើ( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        ត្រឡប់ 12;
      }បើមិនដូច្នេះ{
        *piOut = v;
        ត្រឡប់ 6;
      }
    }
    pak_sa 'b': {   *piOut = '\b';  ត្រឡប់ 2; }
    pak_sa 'f': {   *piOut = '\f';  ត្រឡប់ 2; }
    pak_sa 'n': {   *piOut = '\n';  ត្រឡប់ 2; }
    pak_sa 'r': {   *piOut = '\r';  ត្រឡប់ 2; }
    pak_sa 't': {   *piOut = '\t';  ត្រឡប់ 2; }
    pak_sa 'v': {   *piOut = '\v';  ត្រឡប់ 2; }
    pak_sa '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#បើមិនដូច្នេះ
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      ត្រឡប់ 2;
    }
    pak_sa '\'':
    pak_sa '"':
    pak_sa '/':
    pak_sa '\\':{   *piOut = z[1];  ត្រឡប់ 2; }
    pak_sa 'x': {
      ប្រសិនបើ( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        ត្រឡប់ n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      ត្រឡប់ 4;
    }
    pak_sa 0xe2:
    pak_sa '\r':
    pak_sa '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      ប្រសិនបើ( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        ត្រឡប់ n;
      }បើមិនដូច្នេះ ប្រសិនបើ( nSkip==n ){
        *piOut = 0;
        ត្រឡប់ n;
      }បើមិនដូច្នេះ ប្រសិនបើ( z[nSkip]=='\\' ){
        ត្រឡប់ nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }បើមិនដូច្នេះ{
        ចំនួនគត់ sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        ត្រឡប់ nSkip + sz;
      }
    }
    default: {
      *piOut = JSON_INVALID_CHAR;
      ត្រឡប់ 2;
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
static SQLITE_NOINLINE ចំនួនគត់ jsonLabelCompareEscaped(
  const var_na *zLeft,          /* The left label */
  u32 nLeft,                  /* Size of the left label in bytes */
  ចំនួនគត់ rawLeft,                /* True if zLeft contains no escapes */
  const var_na *zRight,         /* The right label */
  u32 nRight,                 /* Size of the right label in bytes */
  ចំនួនគត់ rawRight                /* True if zRight is escape-free */
){
  u32 cLeft, cRight;
  assert( rawLeft==0 || rawRight==0 );
  ខណៈពេល( 1 /*exit-by-return*/ ){
    ប្រសិនបើ( nLeft==0 ){
      cLeft = 0;
    }បើមិនដូច្នេះ ប្រសិនបើ( rawLeft || zLeft[0]!='\\' ){
      cLeft = ((u8*)zLeft)[0];
      ប្រសិនបើ( cLeft>=0xc0 ){
        ចំនួនគត់ sz = sqlite3Utf8ReadLimited((u8*)zLeft, nLeft, &cLeft);
        zLeft += sz;
        nLeft -= sz;
      }បើមិនដូច្នេះ{
        zLeft++;
        nLeft--;
      }
    }បើមិនដូច្នេះ{
      u32 n = jsonUnescapeOneChar(zLeft, nLeft, &cLeft);
      zLeft += n;
      assert( n<=nLeft );
      nLeft -= n;
    }
    ប្រសិនបើ( nRight==0 ){
      cRight = 0;
    }បើមិនដូច្នេះ ប្រសិនបើ( rawRight || zRight[0]!='\\' ){
      cRight = ((u8*)zRight)[0];
      ប្រសិនបើ( cRight>=0xc0 ){
        ចំនួនគត់ sz = sqlite3Utf8ReadLimited((u8*)zRight, nRight, &cRight);
        zRight += sz;
        nRight -= sz;
      }បើមិនដូច្នេះ{
        zRight++;
        nRight--;
      }
    }បើមិនដូច្នេះ{
      u32 n = jsonUnescapeOneChar(zRight, nRight, &cRight);
      zRight += n;
      assert( n<=nRight );
      nRight -= n;
    }
    ប្រសិនបើ( cLeft!=cRight ) ត្រឡប់ 0;
    ប្រសិនបើ( cLeft==0 ) ត្រឡប់ 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
static ចំនួនគត់ jsonLabelCompare(
  const var_na *zLeft,          /* The left label */
  u32 nLeft,                  /* Size of the left label in bytes */
  ចំនួនគត់ rawLeft,                /* True if zLeft contains no escapes */
  const var_na *zRight,         /* The right label */
  u32 nRight,                 /* Size of the right label in bytes */
  ចំនួនគត់ rawRight                /* True if zRight is escape-free */
){
  ប្រសិនបើ( rawLeft && rawRight ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    ប្រសិនបើ( nLeft!=nRight ) ត្រឡប់ 0;
    ត្រឡប់ memcmp(zLeft, zRight, nLeft)==0;
  }បើមិនដូច្នេះ{
    ត្រឡប់ jsonLabelCompareEscaped(zLeft, nLeft, rawLeft,
                                   zRight, nRight, rawRight);
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
static u32 jsonLookupStep(JsonParse*,u32,const var_na*,u32);


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
static u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  const var_na *zTail   /* Tail of the path that determines substructure */
){
  static const u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  ចំនួនគត់ rc;
  memset(pIns, 0, parimaa_na(*pIns));
  pIns->db = pParse->db;
  ប្រសិនបើ( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }បើមិនដូច្នេះ{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    ប្រសិនបើ( pIns->iDepth >= JSON_MAX_DEPTH ){
      ត្រឡប់ JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  ត្រឡប់ rc;  /* Error code only */
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
static u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 iRoot,              /* Begin the search at this element of aBlob[] */
  const var_na *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, nKey, sz, n, iEnd, rc;
  const var_na *zKey;
  u8 x;

  ប្រសិនបើ( zPath[0]==0 ){
    ប្រសិនបើ( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, iRoot, &sz);
      sz += n;
      ប្រសិនបើ( pParse->eEdit==JEDIT_DEL ){
        ប្រសិនបើ( iLabel>0 ){
          sz += iRoot - iLabel;
          iRoot = iLabel;
        }
        jsonBlobEdit(pParse, iRoot, sz, 0, 0);
      }បើមិនដូច្នេះ ប្រសិនបើ( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }បើមិនដូច្នេះ ប្រសិនបើ( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        ប្រសិនបើ( zPath[-1]!=']' ){
          ត្រឡប់ JSON_LOOKUP_NOTARRAY;
        }បើមិនដូច្នេះ{
          jsonBlobEdit(pParse, iRoot, 0, pParse->aIns, pParse->nIns);
        }
      }បើមិនដូច្នេះ{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, iRoot, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    ត្រឡប់ iRoot;
  }
  ប្រសិនបើ( zPath[0]=='.' ){
    ចំនួនគត់ rawKey = 1;
    x = pParse->aBlob[iRoot];
    zPath++;
    ប្រសិនបើ( zPath[0]=='"' ){
      zKey = zPath + 1;
      សម្រាប់(i=1; zPath[i] && zPath[i]!='"'; i++){
        ប្រសិនបើ( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      nKey = i-1;
      ប្រសិនបើ( zPath[i] ){
        i++;
      }បើមិនដូច្នេះ{
        ត្រឡប់ JSON_LOOKUP_PATHERROR;
      }
      testcase( nKey==0 );
      rawKey = memchr(zKey, '\\', nKey)==0;
    }បើមិនដូច្នេះ{
      zKey = zPath;
      សម្រាប់(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      nKey = i;
      ប្រសិនបើ( nKey==0 ){
        ត្រឡប់ JSON_LOOKUP_PATHERROR;
      }
    }
    ប្រសិនបើ( (x & 0x0f)!=JSONB_OBJECT ) ត្រឡប់ JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, iRoot, &sz);
    j = iRoot + n;  /* j is the index of a label */
    iEnd = j+sz;
    ខណៈពេល( j<iEnd ){
      ចំនួនគត់ rawLabel;
      const var_na *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      ប្រសិនបើ( x<JSONB_TEXT || x>JSONB_TEXTRAW ) ត្រឡប់ JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ប្រសិនបើ( n==0 ) ត្រឡប់ JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      ប្រសិនបើ( k+sz>=iEnd ) ត្រឡប់ JSON_LOOKUP_ERROR;
      zLabel = (const var_na*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      ប្រសិនបើ( jsonLabelCompare(zKey, nKey, rawKey, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        ប្រសិនបើ( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) ត្រឡប់ JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        ប្រសិនបើ( n==0 || v+n+sz>iEnd ) ត្រឡប់ JSON_LOOKUP_ERROR;
        assert( j>0 );
        ប្រសិនបើ( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          ត្រឡប់ JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        ប្រសិនបើ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, iRoot);
        ត្រឡប់ rc;
      }
      j = k+sz;
      ប្រសិនបើ( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) ត្រឡប់ JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ប្រសិនបើ( n==0 ) ត្រឡប់ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ប្រសិនបើ( j>iEnd ) ត្រឡប់ JSON_LOOKUP_ERROR;
    ប្រសិនបើ( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      ប្រសិនបើ( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        ត្រឡប់ JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, parimaa_na(ix));
      ix.db = pParse->db;
      jsonBlobAppendNode(&ix, rawKey?JSONB_TEXTRAW:JSONB_TEXT5, nKey, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      ប្រសិនបើ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+nKey+v.nBlob)
      ){
        assert( !pParse->oom );
        nIns = ix.nBlob + nKey + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        ប្រសិនបើ( !pParse->oom ){
          assert( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          assert( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], zKey, nKey);
          k += nKey;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          ប្រសិនបើ( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, iRoot);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      ត្រឡប់ rc;
    }
  }បើមិនដូច្នេះ ប្រសិនបើ( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[iRoot] & 0x0f;
    ប្រសិនបើ( x!=JSONB_ARRAY )  ត្រឡប់ JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, iRoot, &sz);
    i = 1;
    ខណៈពេល( sqlite3Isdigit(zPath[i]) ){
      ប្រសិនបើ( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    ប្រសិនបើ( i<2 || zPath[i]!=']' ){
      ប្រសិនបើ( zPath[1]=='#' ){
        kk = jsonbArrayCount(pParse, iRoot);
        i = 2;
        ប្រសិនបើ( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          do{
            ប្រសិនបើ( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }ខណៈពេល( sqlite3Isdigit(zPath[i]) );
          ប្រសិនបើ( nn>kk ) ត្រឡប់ JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        ប្រសិនបើ( zPath[i]!=']' ){
          ត្រឡប់ JSON_LOOKUP_PATHERROR;
        }
      }បើមិនដូច្នេះ{
        ត្រឡប់ JSON_LOOKUP_PATHERROR;
      }
    }
    j = iRoot+n;
    iEnd = j+sz;
    ខណៈពេល( j<iEnd ){
      ប្រសិនបើ( kk==0 ){
        ប្រសិនបើ( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          ត្រឡប់ JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        ប្រសិនបើ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, iRoot);
        ត្រឡប់ rc;
      }
      kk--;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      ប្រសិនបើ( n==0 ) ត្រឡប់ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ប្រសិនបើ( j>iEnd ) ត្រឡប់ JSON_LOOKUP_ERROR;
    ប្រសិនបើ( kk>0 ) ត្រឡប់ JSON_LOOKUP_NOTFOUND;
    ប្រសិនបើ( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      ប្រសិនបើ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        assert( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      ប្រសិនបើ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, iRoot);
      ត្រឡប់ rc;
    }
  }បើមិនដូច្នេះ{
    ត្រឡប់ JSON_LOOKUP_PATHERROR; 
  }
  ត្រឡប់ JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
static void jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  const u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  ប្រសិនបើ( NEVER(aBlob==0) ) ត្រឡប់;
  memset(&x, 0, parimaa_na(x));
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
static void jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  ចំនួនគត់ eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  ចំនួនគត់ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  assert( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  ប្រសិនបើ( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    ត្រឡប់;
  }
  switch( pParse->aBlob[i] & 0x0f ){
    pak_sa JSONB_NULL: {
      ប្រសិនបើ( sz ) goto returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      bha_nga;
    }
    pak_sa JSONB_TRUE: {
      ប្រសិនបើ( sz ) goto returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      bha_nga;
    }
    pak_sa JSONB_FALSE: {
      ប្រសិនបើ( sz ) goto returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      bha_nga;
    }
    pak_sa JSONB_INT5:
    pak_sa JSONB_INT: {
      sqlite3_int64 iRes = 0;
      var_na *z;
      ចំនួនគត់ bNeg = 0;
      var_na x;
      ប្រសិនបើ( sz==0 ) goto returnfromblob_malformed;
      x = (var_na)pParse->aBlob[i+n];
      ប្រសិនបើ( x=='-' ){
        ប្រសិនបើ( sz<2 ) goto returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (const var_na*)&pParse->aBlob[i+n], (ចំនួនគត់)sz);
      ប្រសិនបើ( z==0 ) goto returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      ប្រសិនបើ( rc==0 ){
        ប្រសិនបើ( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          dviguu_na r;
          r = (dviguu_na)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }បើមិនដូច្នេះ{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }បើមិនដូច្នេះ ប្រសិនបើ( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }បើមិនដូច្នេះ ប្រសិនបើ( rc==1 ){
        goto returnfromblob_malformed;
      }បើមិនដូច្នេះ{
        ប្រសិនបើ( bNeg ){ n--; sz++; }
        goto to_double;
      }
      bha_nga;
    }
    pak_sa JSONB_FLOAT5:
    pak_sa JSONB_FLOAT: {
      dviguu_na r;
      var_na *z;
      ប្រសិនបើ( sz==0 ) goto returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (const var_na*)&pParse->aBlob[i+n], (ចំនួនគត់)sz);
      ប្រសិនបើ( z==0 ) goto returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      ប្រសិនបើ( rc<=0 ) goto returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      bha_nga;
    }
    pak_sa JSONB_TEXTRAW:
    pak_sa JSONB_TEXT: {
      sqlite3_result_text(pCtx, (var_na*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      bha_nga;
    }
    pak_sa JSONB_TEXT5:
    pak_sa JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      const var_na *z;
      var_na *zOut;
      u32 nOut = sz;
      z = (const var_na*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      ប្រសិនបើ( zOut==0 ) goto returnfromblob_oom;
      សម្រាប់(iIn=iOut=0; iIn<sz; iIn++){
        var_na c = z[iIn];
        ប្រសិនបើ( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          ប្រសិនបើ( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }បើមិនដូច្នេះ ប្រសិនបើ( v<=0x7f ){
            zOut[iOut++] = (var_na)v;
          }បើមិនដូច្នេះ ប្រសិនបើ( v<=0x7ff ){
            assert( szEscape>=2 );
            zOut[iOut++] = (var_na)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }បើមិនដូច្នេះ ប្រសិនបើ( v<0x10000 ){
            assert( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }បើមិនដូច្នេះ ប្រសិនបើ( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }បើមិនដូច្នេះ{
            assert( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }បើមិនដូច្នេះ{
          zOut[iOut++] = c;
        }
      } /* end for() */
      assert( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      bha_nga;
    }
    pak_sa JSONB_ARRAY:
    pak_sa JSONB_OBJECT: {
      ប្រសិនបើ( eMode==0 ){
        ប្រសិនបើ( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }បើមិនដូច្នេះ{
          eMode = 1;
        }
      }
      ប្រសិនបើ( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }បើមិនដូច្នេះ{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      bha_nga;
    }
    default: {
      goto returnfromblob_malformed;
    }
  }
  ត្រឡប់;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  ត្រឡប់;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  ត្រឡប់;
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
static ចំនួនគត់ jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_value *pArg,
  JsonParse *pParse
){
  ចំនួនគត់ eType = sqlite3_value_type(pArg);
  static u8 aNull[] = { 0x00 };
  memset(pParse, 0, parimaa_na(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  switch( eType ){
    default: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      ត្រឡប់ 0;
    }
    pak_sa SQLITE_BLOB: {
      ប្រសិនបើ( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        ត្រឡប់ 1;
      }
      bha_nga;
    }
    pak_sa SQLITE_TEXT: {
      const var_na *zJson = (const var_na*)sqlite3_value_text(pArg);
      ចំនួនគត់ nJson = sqlite3_value_bytes(pArg);
      ប្រសិនបើ( zJson==0 ) ត្រឡប់ 1;
      ប្រសិនបើ( sqlite3_value_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (var_na*)zJson;
        pParse->nJson = nJson;
        ប្រសិនបើ( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, parimaa_na(pParse[0]));
          ត្រឡប់ 1;
        }
      }បើមិនដូច្នេះ{
        jsonBlobAppendNode(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      bha_nga;
    }
    pak_sa SQLITE_FLOAT: {
      ប្រសិនបើ( NEVER(sqlite3IsNaN(sqlite3_value_double(pArg))) ){
        jsonBlobAppendNode(pParse, JSONB_NULL, 0, 0);
      }បើមិនដូច្នេះ{
        ចំនួនគត់ n = sqlite3_value_bytes(pArg);
        const var_na *z = (const var_na*)sqlite3_value_text(pArg);
        ប្រសិនបើ( z==0 ) ត្រឡប់ 1;
        ប្រសិនបើ( z[0]=='I' ){
          jsonBlobAppendNode(pParse, JSONB_FLOAT, 5, "9e999");
        }បើមិនដូច្នេះ ប្រសិនបើ( z[0]=='-' && z[1]=='I' ){
          jsonBlobAppendNode(pParse, JSONB_FLOAT, 6, "-9e999");
        }បើមិនដូច្នេះ{
          jsonBlobAppendNode(pParse, JSONB_FLOAT, n, z);
        }
      }
      bha_nga;
    }
    pak_sa SQLITE_INTEGER: {
      ចំនួនគត់ n = sqlite3_value_bytes(pArg);
      const var_na *z = (const var_na*)sqlite3_value_text(pArg);
      ប្រសិនបើ( z==0 ) ត្រឡប់ 1;
      jsonBlobAppendNode(pParse, JSONB_INT, n, z);
      bha_nga;
    }
  }
  ប្រសិនបើ( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    ត្រឡប់ 1;
  }បើមិនដូច្នេះ{
    ត្រឡប់ 0;
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
static var_na *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  const var_na *zPath,        /* The path with the problem */
  ចំនួនគត់ rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  var_na *zMsg;
  ប្រសិនបើ( rc==(ចំនួនគត់)JSON_LOOKUP_NOTARRAY ){
    zMsg = sqlite3_mprintf("not an array element: %Q", zPath);
  }បើមិនដូច្នេះ ប្រសិនបើ( rc==(ចំនួនគត់)JSON_LOOKUP_ERROR ){
    zMsg = sqlite3_mprintf("malformed JSON");
  }បើមិនដូច្នេះ ប្រសិនបើ( rc==(ចំនួនគត់)JSON_LOOKUP_TOODEEP ){
    zMsg = sqlite3_mprintf("JSON path too deep");
  }បើមិនដូច្នេះ{
    zMsg = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  ប្រសិនបើ( ctx==0 ) ត្រឡប់ zMsg;
  ប្រសិនបើ( zMsg ){
    sqlite3_result_error(ctx, zMsg, -1);
    sqlite3_free(zMsg);
  }បើមិនដូច្នេះ{
    sqlite3_result_error_nomem(ctx);
  }
  ត្រឡប់ 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
static void jsonInsertIntoBlob(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv,
  ចំនួនគត់ eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  ចំនួនគត់ i;
  u32 rc = 0;
  const var_na *zPath = 0;
  ចំនួនគត់ flgs;
  JsonParse *p;
  JsonParse ax;

  assert( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  ប្រសិនបើ( p==0 ) ត្រឡប់;
  សម្រាប់(i=1; i<argc-1; i+=2){
    ប្រសិនបើ( sqlite3_value_type(argv[i])==SQLITE_NULL ) continue;
    zPath = (const var_na*)sqlite3_value_text(argv[i]);
    ប្រសិនបើ( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      ត្រឡប់;
    }
    ប្រសិនបើ( zPath[0]!='$' ) goto jsonInsertIntoBlob_patherror;
    ប្រសិនបើ( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      ត្រឡប់;
    }
    ប្រសិនបើ( zPath[1]==0 ){
      ប្រសិនបើ( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }បើមិនដូច្នេះ{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    ប្រសិនបើ( rc==JSON_LOOKUP_NOTFOUND ) continue;
    ប្រសិនបើ( JSON_LOOKUP_ISERROR(rc) ) goto jsonInsertIntoBlob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ត្រឡប់;

jsonInsertIntoBlob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  ត្រឡប់;
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
static ចំនួនគត់ jsonArgIsJsonb(sqlite3_value *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  ប្រសិនបើ( sqlite3_value_type(pArg)!=SQLITE_BLOB ) ត្រឡប់ 0;
  p->aBlob = (u8*)sqlite3_value_blob(pArg);
  p->nBlob = (u32)sqlite3_value_bytes(pArg);
  ប្រសិនបើ( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    ត្រឡប់ 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  ត្រឡប់ 0;
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
static JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_value *pArg,
  u32 flgs
){
  ចំនួនគត់ eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  assert( ctx!=0 );
  eType = sqlite3_value_type(pArg);
  ប្រសិនបើ( eType==SQLITE_NULL ){
    ត្រឡប់ 0;
  }
  ប្រសិនបើ( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = jsonCacheSearch(ctx, pArg);
  }
  ប្រសិនបើ( pFromCache ){
    pFromCache->nJPRef++;
    ប្រសិនបើ( (flgs & JSON_EDITABLE)==0 ){
      ត្រឡប់ pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, parimaa_na(*p));
  ប្រសិនបើ( p==0 ) goto json_pfa_oom;
  memset(p, 0, parimaa_na(*p));
  p->db = db;
  p->nJPRef = 1;
  ប្រសិនបើ( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    ប្រសិនបើ( p->aBlob==0 ) goto json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    ត្រឡប់ p;
  }
  ប្រសិនបើ( eType==SQLITE_BLOB ){
    ប្រសិនបើ( jsonArgIsJsonb(pArg,p) ){
      ប្រសិនបើ( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        goto json_pfa_oom;
      }
      ត្រឡប់ p;
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
  p->zJson = (var_na*)sqlite3_value_text(pArg);
  p->nJson = sqlite3_value_bytes(pArg);
  ប្រសិនបើ( db->mallocFailed ) goto json_pfa_oom;
  ប្រសិនបើ( p->nJson==0 ) goto json_pfa_malformed;
  assert( p->zJson!=0 );
  ប្រសិនបើ( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    ប្រសិនបើ( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      ត្រឡប់ p;
    }បើមិនដូច្នេះ{
      jsonParseFree(p);
      ត្រឡប់ 0;
    }
  }បើមិនដូច្នេះ{
    ចំនួនគត់ isRCStr = sqlite3ValueIsOfClass(pArg, sqlite3RCStrUnref);
    ចំនួនគត់ rc;
    ប្រសិនបើ( !isRCStr ){
      var_na *zNew = sqlite3RCStrNew( p->nJson );
      ប្រសិនបើ( zNew==0 ) goto json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }បើមិនដូច្នេះ{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = jsonCacheInsert(ctx, p);
    ប្រសិនបើ( rc==SQLITE_NOMEM ) goto json_pfa_oom;
    ប្រសិនបើ( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      goto rebuild_from_cache;
    }
  }
  ត្រឡប់ p;

json_pfa_malformed:
  ប្រសិនបើ( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    ត្រឡប់ p;
  }បើមិនដូច្នេះ{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    ត្រឡប់ 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  ត្រឡប់ 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
static void jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  ចំនួនគត់ flgs;
  ប្រសិនបើ( p->oom ){
    sqlite3_result_error_nomem(ctx);
    ត្រឡប់;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  ប្រសិនបើ( flgs & JSON_BLOB ){
    ប្រសិនបើ( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }បើមិនដូច្នេះ{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }បើមិនដូច្នេះ{
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

#ប្រសិនបើ SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
static void jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  ចំនួនគត់ nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  ខណៈពេល( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    ចំនួនគត់ showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    ប្រសិនបើ( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    ប្រសិនបើ( nn==0 ) nn = 1;
    ប្រសិនបើ( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    សម្រាប់(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    ប្រសិនបើ( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      continue;
    }
    pParse->nBlob = savedNBlob;
    ប្រសិនបើ( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      ប្រសិនបើ( iEnd>pParse->nBlob ){
        ប្រសិនបើ( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }បើមិនដូច្នេះ{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    switch( x ){
      pak_sa JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); bha_nga;
      pak_sa JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); bha_nga;
      pak_sa JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); bha_nga;
      pak_sa JSONB_INT:      sqlite3_str_appendall(pOut,"int"); bha_nga;
      pak_sa JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); bha_nga;
      pak_sa JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); bha_nga;
      pak_sa JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); bha_nga;
      pak_sa JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); bha_nga;
      pak_sa JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); bha_nga;
      pak_sa JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); bha_nga;
      pak_sa JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); bha_nga;
      pak_sa JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        bha_nga;
      }
      pak_sa JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        bha_nga;
      }
      default: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        bha_nga;
      }
    }
    ប្រសិនបើ( showContent ){
      ប្រសិនបើ( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }បើមិនដូច្នេះ{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        សម្រាប់(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          ប្រសិនបើ( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (var_na*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
static void jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  var_na zBuf[1000];
  ប្រសិនបើ( pParse==0 ){
    printf("NULL pointer\n");
    ត្រឡប់;
  }បើមិនដូច្នេះ{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    ប្រសិនបើ( pParse->nBlob==0 ) ត្រឡប់;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, parimaa_na(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_value(&out));
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
static void jsonParseFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  assert( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ប្រសិនបើ( p==0 ) ត្រឡប់;
  ប្រសិនបើ( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }បើមិនដូច្នេះ{
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
static void jsonQuoteFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  jsonAppendSqlValue(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
static void jsonArrayFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  ចំនួនគត់ i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  សម្រាប់(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    jsonAppendSqlValue(&jx, argv[i]);
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
static void jsonArrayLengthFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ប្រសិនបើ( p==0 ) ត្រឡប់;
  ប្រសិនបើ( argc==2 ){
    const var_na *zPath = (const var_na*)sqlite3_value_text(argv[1]);
    ប្រសិនបើ( zPath==0 ){
      jsonParseFree(p);
      ត្រឡប់;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    ប្រសិនបើ( JSON_LOOKUP_ISERROR(i) ){
      ប្រសិនបើ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }បើមិនដូច្នេះ{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }បើមិនដូច្នេះ{
    i = 0;
  }
  ប្រសិនបើ( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonbArrayCount(p, i);
  }
  ប្រសិនបើ( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
static ចំនួនគត់ jsonAllAlphanum(const var_na *z, ចំនួនគត់ n){
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  ត្រឡប់ i==n;
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
static void jsonExtractFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonParse *p = 0;      /* The parse */
  ចំនួនគត់ flags;             /* Flags associated with the function */
  ចំនួនគត់ i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  ប្រសិនបើ( argc<2 ) ត្រឡប់;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ប្រសិនបើ( p==0 ) ត្រឡប់;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  ប្រសិនបើ( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  សម្រាប់(i=1; i<argc; i++){
    /* With a single PATH argument */
    const var_na *zPath = (const var_na*)sqlite3_value_text(argv[i]);
    ចំនួនគត់ nPath;
    u32 j;
    ប្រសិនបើ( zPath==0 ) goto json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    ប្រសិនបើ( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }បើមិនដូច្នេះ ប្រសិនបើ( (flags & JSON_ABPATH) ){
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
      ប្រសិនបើ( sqlite3_value_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        ប្រសិនបើ( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }បើមិនដូច្នេះ ប្រសិនបើ( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }បើមិនដូច្នេះ ប្រសិនបើ( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }បើមិនដូច្នេះ{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }បើមិនដូច្នេះ{
      jsonBadPathError(ctx, zPath, 0);
      goto json_extract_error;
    }
    ប្រសិនបើ( j<p->nBlob ){
      ប្រសិនបើ( argc==2 ){
        ប្រសិនបើ( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          assert( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }បើមិនដូច្នេះ{
          jsonReturnFromBlob(p, j, ctx, 0);
          ប្រសិនបើ( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }បើមិនដូច្នេះ{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( j==JSON_LOOKUP_NOTFOUND ){
      ប្រសិនបើ( argc==2 ){
        goto json_extract_error;  /* Return NULL if not found */
      }បើមិនដូច្នេះ{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }បើមិនដូច្នេះ{
      jsonBadPathError(ctx, zPath, j);
      goto json_extract_error;
    }
  }
  ប្រសិនបើ( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    ប្រសិនបើ( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  ត្រឡប់;
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
static ចំនួនគត់ jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  const JsonParse *pPatch, /* The PATCH */
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

  assert( iTarget>=0 && iTarget<pTarget->nBlob );
  assert( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  ប្រសិនបើ( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    ត្រឡប់ pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  ប្រសិនបើ( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  ប្រសិនបើ( NEVER(n==0) ) ត្រឡប់ JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  ប្រសិនបើ( NEVER(n==0) ) ត្រឡប់ JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  ខណៈពេល( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    ប្រសិនបើ( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      ត្រឡប់ JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    ប្រសិនបើ( nPLabel==0 ) ត្រឡប់ JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    ប្រសិនបើ( iPValue>=iPEnd ) ត្រឡប់ JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    ប្រសិនបើ( nPValue==0 ) ត្រឡប់ JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    ប្រសិនបើ( iPCursor>iPEnd ) ត្រឡប់ JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    ខណៈពេល( iTCursor<iTEnd ){
      ចំនួនគត់ isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      ប្រសិនបើ( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        ត្រឡប់ JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      ប្រសិនបើ( nTLabel==0 ) ត្រឡប់ JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      ប្រសិនបើ( iTValue>=iTEnd ) ត្រឡប់ JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      ប្រសិនបើ( nTValue==0 ) ត្រឡប់ JSON_MERGE_BADTARGET;
      ប្រសិនបើ( iTValue + nTValue + szTValue > iTEnd ) ត្រឡប់ JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (const var_na*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (const var_na*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      ប្រសិនបើ( isEqual ) bha_nga;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    ប្រសិនបើ( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      ប្រសិនបើ( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        ប្រសិនបើ( NEVER(pTarget->oom) ) ត្រឡប់ JSON_MERGE_OOM;
      }បើមិនដូច្នេះ{
        /* Algorithm line 12 */
        ចំនួនគត់ rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        ប្រសិនបើ( iDepth>=JSON_MAX_DEPTH ) ត្រឡប់ JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        ប្រសិនបើ( rc ) ត្រឡប់ rc;
        pTarget->delta += savedDelta;
      }        
    }បើមិនដូច្នេះ ប្រសិនបើ( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      ប្រសិនបើ( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        ប្រសិនបើ( pTarget->oom ) ត្រឡប់ JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }បើមិនដូច្នេះ{
        ចំនួនគត់ rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        ប្រសិនបើ( pTarget->oom ) ត្រឡប់ JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        ប្រសិនបើ( iDepth>=JSON_MAX_DEPTH ) ត្រឡប់ JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        ប្រសិនបើ( rc ) ត្រឡប់ rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  ប្រសិនបើ( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  ត្រឡប់ pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
static void jsonPatchFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  ចំនួនគត់ rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  assert( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  ប្រសិនបើ( pTarget==0 ) ត្រឡប់;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  ប្រសិនបើ( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    ប្រសិនបើ( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }បើមិនដូច្នេះ ប្រសិនបើ( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }បើមិនដូច្នេះ ប្រសិនបើ( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }បើមិនដូច្នេះ{
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
static void jsonObjectFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  ចំនួនគត់ i;
  JsonString jx;
  const var_na *z;
  u32 n;

  ប្រសិនបើ( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    ត្រឡប់;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  សម្រាប់(i=0; i<argc; i+=2){
    ប្រសិនបើ( sqlite3_value_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      ត្រឡប់;
    }
    jsonAppendSeparator(&jx);
    z = (const var_na*)sqlite3_value_text(argv[i]);
    n = sqlite3_value_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    jsonAppendSqlValue(&jx, argv[i+1]);
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
static void jsonRemoveFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonParse *p;          /* The parse */
  const var_na *zPath = 0; /* Path of element to be removed */
  ចំនួនគត់ i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  ប្រសិនបើ( argc<1 ) ត្រឡប់;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  ប្រសិនបើ( p==0 ) ត្រឡប់;
  សម្រាប់(i=1; i<argc; i++){
    zPath = (const var_na*)sqlite3_value_text(argv[i]);
    ប្រសិនបើ( zPath==0 ){
      goto json_remove_done;
    }
    ប្រសិនបើ( zPath[0]!='$' ){
      goto json_remove_patherror;
    }
    ប្រសិនបើ( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      goto json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    ប្រសិនបើ( JSON_LOOKUP_ISERROR(rc) ){
      ប្រសិនបើ( rc==JSON_LOOKUP_NOTFOUND ){
        continue;  /* No-op */
      }បើមិនដូច្នេះ{
        jsonBadPathError(ctx, zPath, rc);
      }
      goto json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ត្រឡប់;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  ត្រឡប់;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
static void jsonReplaceFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  ប្រសិនបើ( argc<1 ) ត្រឡប់;
  ប្រសិនបើ( (argc&1)==0 ) {
    jsonWrongNumArgs(ctx, "replace");
    ត្រឡប់;
  }
  jsonInsertIntoBlob(ctx, argc, argv, JEDIT_REPL);
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
static void jsonSetFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  ចំនួនគត់ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  ចំនួនគត់ eInsType = JSON_INSERT_TYPE(flags);
  static const var_na *azInsType[] = { "insert", "set", "array_insert" };
  static const u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  ប្រសិនបើ( argc<1 ) ត្រឡប់;
  assert( eInsType>=0 && eInsType<=2 );
  ប្រសិនបើ( (argc&1)==0 ) {
    jsonWrongNumArgs(ctx, azInsType[eInsType]);
    ត្រឡប់;
  }
  jsonInsertIntoBlob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
static void jsonTypeFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonParse *p;          /* The parse */
  const var_na *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ប្រសិនបើ( p==0 ) ត្រឡប់;
  ប្រសិនបើ( argc==2 ){
    zPath = (const var_na*)sqlite3_value_text(argv[1]);
    ប្រសិនបើ( zPath==0 ) goto json_type_done;
    ប្រសិនបើ( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      goto json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    ប្រសិនបើ( JSON_LOOKUP_ISERROR(i) ){
      ប្រសិនបើ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }បើមិនដូច្នេះ{
        jsonBadPathError(ctx, zPath, i);
      }
      goto json_type_done;
    }
  }បើមិនដូច្នេះ{
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
static void jsonPrettyFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, parimaa_na(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  ប្រសិនបើ( x.pParse==0 ) ត្រឡប់;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  ប្រសិនបើ( argc==1 || (x.zIndent = (const var_na*)sqlite3_value_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }បើមិនដូច្នេះ{
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
static void jsonValidFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  ប្រសិនបើ( argc==2 ){
    i64 f = sqlite3_value_int64(argv[1]);
    ប្រសិនបើ( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      ត្រឡប់;
    }
    flags = f & 0x0f;
  }
  switch( sqlite3_value_type(argv[0]) ){
    pak_sa SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      ត្រឡប់;
    }
    pak_sa SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, parimaa_na(py));
      ប្រសិនបើ( jsonArgIsJsonb(argv[0], &py) ){
        ប្រសិនបើ( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }បើមិនដូច្នេះ ប្រសិនបើ( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        bha_nga;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    default: {
      JsonParse px;
      ប្រសិនបើ( (flags & 0x3)==0 ) bha_nga;
      memset(&px, 0, parimaa_na(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      ប្រសិនបើ( p ){
        ប្រសិនបើ( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }បើមិនដូច្នេះ ប្រសិនបើ( p->nErr ){
          /* no-op */
        }បើមិនដូច្នេះ ប្រសិនបើ( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }បើមិនដូច្នេះ{
        sqlite3_result_error_nomem(ctx);
      }
      bha_nga;
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
static void jsonErrorFunc(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, parimaa_na(s));
  s.db = sqlite3_context_db_handle(ctx);
  ប្រសិនបើ( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }បើមិនដូច្នេះ{
    s.zJson = (var_na*)sqlite3_value_text(argv[0]);
    ប្រសិនបើ( s.zJson==0 ) ត្រឡប់;  /* NULL input or OOM */
    s.nJson = sqlite3_value_bytes(argv[0]);
    ប្រសិនបើ( jsonConvertTextToBlob(&s,0) ){
      ប្រសិនបើ( s.oom ){
        iErrPos = -1;
      }បើមិនដូច្នេះ{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        assert( s.zJson!=0 );  /* Because s.oom is false */
        សម្រាប់(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          ប្រសិនបើ( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  ប្រសិនបើ( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }បើមិនដូច្នេះ{
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
static void jsonArrayStep(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, parimaa_na(*pStr));
  ប្រសិនបើ( pStr ){
    ប្រសិនបើ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }បើមិនដូច្នេះ ប្រសិនបើ( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    jsonAppendSqlValue(pStr, argv[0]);
  }
}
static void jsonArrayCompute(sqlite3_context *ctx, ចំនួនគត់ isFinal){
  JsonString *pStr;
  ចំនួនគត់ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ប្រសិនបើ( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    ប្រសិនបើ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ត្រឡប់;
    }បើមិនដូច្នេះ ប្រសិនបើ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ប្រសិនបើ( isFinal ){
        ប្រសិនបើ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }បើមិនដូច្នេះ{
        jsonStringTrimOneChar(pStr);
      }
      ត្រឡប់;
    }បើមិនដូច្នេះ ប្រសិនបើ( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (ចំនួនគត់)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }បើមិនដូច្នេះ{
      sqlite3_result_text(ctx, pStr->zBuf, (ចំនួនគត់)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }បើមិនដូច្នេះ ប្រសិនបើ( flags & JSON_BLOB ){
    static const u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }បើមិនដូច្នេះ{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
static void jsonArrayValue(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
static void jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
static void jsonGroupInverse(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  unsigned ចំនួនគត់ i;
  ចំនួនគត់ inStr = 0;
  ចំនួនគត់ nNest = 0;
  var_na *z;
  var_na c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  ប្រសិនបើ( NEVER(!pStr) ) ត្រឡប់;
  z = pStr->zBuf;
  សម្រាប់(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    ប្រសិនបើ( c=='"' ){
      inStr = !inStr;
    }បើមិនដូច្នេះ ប្រសិនបើ( c=='\\' ){
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( !inStr ){
      ប្រសិនបើ( c=='{' || c=='[' ) nNest++;
      ប្រសិនបើ( c=='}' || c==']' ) nNest--;
    }
  }
  ប្រសិនបើ( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }បើមិនដូច្នេះ{
    pStr->nUsed = 1;
  }
}
#បើមិនដូច្នេះ
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
static void jsonObjectStep(
  sqlite3_context *ctx,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  JsonString *pStr;
  const var_na *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, parimaa_na(*pStr));
  ប្រសិនបើ( pStr ){
    z = (const var_na*)sqlite3_value_text(argv[0]);
    n = sqlite3Strlen30(z);
    ប្រសិនបើ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }បើមិនដូច្នេះ ប្រសិនបើ( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    ប្រសិនបើ( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      jsonAppendSqlValue(pStr, argv[1]);
    }បើមិនដូច្នេះ{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
static void jsonObjectCompute(sqlite3_context *ctx, ចំនួនគត់ isFinal){
  JsonString *pStr;
  ចំនួនគត់ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ប្រសិនបើ( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    ប្រសិនបើ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ត្រឡប់;
    }
    ប្រសិនបើ( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      ចំនួនគត់ inStr = 0;
      ប្រសិនបើ( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        ប្រសិនបើ( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          ត្រឡប់;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      សម្រាប់(i=j=1; i<pStr->nUsed; i++){
        var_na c = pStr->zBuf[i];
        ប្រសិនបើ( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }បើមិនដូច្នេះ ប្រសិនបើ( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }បើមិនដូច្នេះ ប្រសិនបើ( c=='@' && !inStr ){
          assert( i+1<pStr->nUsed );
          ប្រសិនបើ( pStr->zBuf[i+1]==',' ){
            i++;
          }បើមិនដូច្នេះ ប្រសិនបើ( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }បើមិនដូច្នេះ{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    ប្រសិនបើ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ប្រសិនបើ( isFinal ){
        ប្រសិនបើ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }បើមិនដូច្នេះ{
        jsonStringTrimOneChar(pOgStr);
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (ចំនួនគត់)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }បើមិនដូច្នេះ{
      sqlite3_result_text(ctx, pStr->zBuf, (ចំនួនគត់)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    ប្រសិនបើ( pStr!=pOgStr ) jsonStringReset(pStr);
  }បើមិនដូច្នេះ ប្រសិនបើ( flags & JSON_BLOB ){
    static const unsigned var_na emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }បើមិនដូច្នេះ{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
static void jsonObjectValue(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
static void jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
typedef sa_nracanaa JsonParent JsonParent;
sa_nracanaa JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 iValue;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 iKey;                  /* Key for JSONB_ARRAY */
};

typedef sa_nracanaa JsonEachCursor JsonEachCursor;
sa_nracanaa JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 nRoot;                 /* Size of the root path in bytes */
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
typedef sa_nracanaa JsonEachConnection JsonEachConnection;
sa_nracanaa JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
static ចំនួនគត់ jsonEachConnect(
  sqlite3 *db,
  void *pAux,
  ចំនួនគត់ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  JsonEachConnection *pNew;
  ចំនួនគត់ rc;

/* Column numbers */
#define JEACH_KEY     0
#define JEACH_VALUE   1
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
#define JEACH_ROOT    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  ប្រសិនបើ( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, parimaa_na(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    ប្រសិនបើ( pNew==0 ) ត្រឡប់ SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  ត្រឡប់ rc;
}

/* destructor for json_each virtual table */
static ចំនួនគត់ jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  ត្រឡប់ SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
static ចំនួនគត់ jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, parimaa_na(*pCur));
  ប្រសិនបើ( pCur==0 ) ត្រឡប់ SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  ត្រឡប់ SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
static void jsonEachCursorReset(JsonEachCursor *p){
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
static ចំនួនគត់ jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  ត្រឡប់ SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
static ចំនួនគត់ jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ត្រឡប់ p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
static ចំនួនគត់ jsonSkipLabel(JsonEachCursor *p){
  ប្រសិនបើ( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    ប្រសិនបើ( sz >= p->sParse.nBlob ) sz = p->i;
    ត្រឡប់ sz;
  }បើមិនដូច្នេះ{
    ត្រឡប់ p->i;
  }
}

/*
** Append the path name for the current element.
*/
static void jsonAppendPathName(JsonEachCursor *p){
  assert( p->nParent>0 );
  assert( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  ប្រសិនបើ( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].iKey);
  }បើមិនដូច្នេះ{
    u32 n, sz = 0, k, i;
    const var_na *z;
    ចំនួនគត់ needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (const var_na*)&p->sParse.aBlob[k];
    ប្រសិនបើ( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }បើមិនដូច្នេះ{
      សម្រាប់(i=0; i<sz; i++){
        ប្រសិនបើ( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          bha_nga;
        }
      }
    }
    ប្រសិនបើ( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }បើមិនដូច្នេះ{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
static ចំនួនគត់ jsonEachMalformedInput(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->zErrMsg);
  cur->pVtab->zErrMsg = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  ត្រឡប់ cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
static ចំនួនគត់ jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ចំនួនគត់ rc = SQLITE_OK;
  ប្រសិនបើ( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    ប្រសិនបើ( n==0 )ត្រឡប់ jsonEachMalformedInput(cur);
    ប្រសិនបើ( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      ប្រសិនបើ( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, parimaa_na(JsonParent)*nNew);
        ប្រសិនបើ( pNew==0 ) ត្រឡប់ SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->iValue = i;
      pParent->iEnd = i + n + sz;
      pParent->iKey = -1;
      pParent->nPath = (u32)p->path.nUsed;
      ប្រសិនបើ( p->eType && p->nParent ){
        jsonAppendPathName(p);
        ប្រសិនបើ( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }បើមិនដូច្នេះ{
      p->i = i + n + sz;
    }
    ខណៈពេល( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    ប្រសិនបើ( levelChange ){
      ប្រសិនបើ( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->iValue;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }បើមិនដូច្នេះ{
        p->eType = 0;
      }
    }
  }បើមិនដូច្នេះ{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    ប្រសិនបើ( n==0 )ត្រឡប់ jsonEachMalformedInput(cur);
    p->i = i + n + sz;
  }
  ប្រសិនបើ( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].iKey++;
  }
  p->iRowid++;
  ត្រឡប់ rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
static ចំនួនគត់ jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  var_na *z = p->path.zBuf;
  ប្រសិនបើ( p->iRowid==0 && p->bRecursive && n>=2 ){
    ខណៈពេល( ALWAYS(n>1) ){
      n--;
      ប្រសិនបើ( z[n]=='[' || z[n]=='.' ){
        u32 x;
        var_na cSaved = z[n];
        z[n] = 0;
        assert( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        ប្រសិនបើ( !JSON_LOOKUP_ISERROR(x) ) bha_nga;
      }
    }
  }
  ត្រឡប់ n;
}

/* Return the value of a column */
static ចំនួនគត់ jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ចំនួនគត់ iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  switch( iColumn ){
    pak_sa JEACH_KEY: {
      ប្រសិនបើ( p->nParent==0 ){
        u32 n, j;
        ប្រសិនបើ( p->nRoot==1 ) bha_nga;
        j = jsonEachPathLength(p);
        n = p->nRoot - j;
        ប្រសិនបើ( n==0 ){
          bha_nga;
        }បើមិនដូច្នេះ ប្រសិនបើ( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }បើមិនដូច្នេះ ប្រសិនបើ( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }បើមិនដូច្នេះ{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        bha_nga;
      }
      ប្រសិនបើ( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }បើមិនដូច្នេះ{
        assert( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iKey);
      }
      bha_nga;
    }
    pak_sa JEACH_VALUE: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      ប្រសិនបើ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      bha_nga;
    }
    pak_sa JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      bha_nga;
    }
    pak_sa JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      ប្រសិនបើ( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      bha_nga;
    }
    pak_sa JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      bha_nga;
    }
    pak_sa JEACH_PARENT: {
      ប្រសិនបើ( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      bha_nga;
    }
    pak_sa JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      ប្រសិនបើ( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      bha_nga;
    }
    pak_sa JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      bha_nga;
    }
    default: {
      sqlite3_result_text(ctx, p->path.zBuf, p->nRoot, SQLITE_TRANSIENT);
      bha_nga;
    }
    pak_sa JEACH_JSON: {
      ប្រសិនបើ( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }បើមិនដូច្នេះ{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      bha_nga;
    }
  }
  ត្រឡប់ SQLITE_OK;
}

/* Return the current rowid value */
static ចំនួនគត់ jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  ត្រឡប់ SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
static ចំនួនគត់ jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  ចំនួនគត់ i;                     /* Loop counter or computed array index */
  ចំនួនគត់ aIdx[2];               /* Index of constraints for JSON and ROOT */
  ចំនួនគត់ unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  ចំនួនគត់ idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  const sa_nracanaa sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  assert( JEACH_ROOT == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  សម្រាប់(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ចំនួនគត់ iCol;
    ចំនួនគត់ iMask;
    ប្រសិនបើ( pConstraint->iColumn < JEACH_JSON ) continue;
    iCol = pConstraint->iColumn - JEACH_JSON;
    assert( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    ប្រសិនបើ( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }បើមិនដូច្នេះ ប្រសិនបើ( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  ប្រសិនបើ( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  ប្រសិនបើ( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    ត្រឡប់ SQLITE_CONSTRAINT;
  }
  ប្រសិនបើ( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idxNum = 0;
  }បើមិនដូច្នេះ{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ប្រសិនបើ( aIdx[1]<0 ){
      pIdxInfo->idxNum = 1;  /* Only JSON supplied.  Plan 1 */
    }បើមិនដូច្នេះ{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idxNum = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  ត្រឡប់ SQLITE_OK;
}

/* Start a search on a new JSON string */
static ចំនួនគត់ jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  ចំនួនគត់ idxNum, const var_na *idxStr,
  ចំនួនគត់ argc, sqlite3_value **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  const var_na *zRoot = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  ប្រសិនបើ( idxNum==0 ) ត្រឡប់ SQLITE_OK;
  memset(&p->sParse, 0, parimaa_na(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  ប្រសិនបើ( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }បើមិនដូច្នេះ{
    p->sParse.zJson = (var_na*)sqlite3_value_text(argv[0]);
    p->sParse.nJson = sqlite3_value_bytes(argv[0]);
    ប្រសិនបើ( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      ត្រឡប់ SQLITE_OK;
    }      
    ប្រសិនបើ( jsonConvertTextToBlob(&p->sParse, 0) ){
      ប្រសិនបើ( p->sParse.oom ){
        ត្រឡប់ SQLITE_NOMEM;
      }
      ត្រឡប់ jsonEachMalformedInput(cur);
    }
  }
  ប្រសិនបើ( idxNum==3 ){
    zRoot = (const var_na*)sqlite3_value_text(argv[1]);
    ប្រសិនបើ( zRoot==0 ) ត្រឡប់ SQLITE_OK;
    ប្រសិនបើ( zRoot[0]!='$' ){
      sqlite3_free(cur->pVtab->zErrMsg);
      cur->pVtab->zErrMsg = jsonBadPathError(0, zRoot, 0);
      jsonEachCursorReset(p);
      ត្រឡប់ cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->nRoot = sqlite3Strlen30(zRoot);
    ប្រសិនបើ( zRoot[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }បើមិនដូច្នេះ{
      i = jsonLookupStep(&p->sParse, 0, zRoot+1, 0);
      ប្រសិនបើ( JSON_LOOKUP_ISERROR(i) ){
        ប្រសិនបើ( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          ត្រឡប់ SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->zErrMsg);
        cur->pVtab->zErrMsg = jsonBadPathError(0, zRoot, 0);
        jsonEachCursorReset(p);
        ត្រឡប់ cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      ប្រសិនបើ( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }បើមិនដូច្នេះ{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, zRoot, p->nRoot);
  }បើមិនដូច្នេះ{
    i = p->i = 0;
    p->eType = 0;
    p->nRoot = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  ប្រសិនបើ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, parimaa_na(JsonParent));
    ប្រសិនបើ( p->aParent==0 ) ត្រឡប់ SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].iKey = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].iValue = i;
  }
  ត្រឡប់ SQLITE_OK;
}

/* The methods of the json_each virtual table */
static sqlite3_module jsonEachModule = {
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
void sqlite3RegisterJsonFunctions(void){
#ifndef SQLITE_OMIT_JSON
  static FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_insert, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_insert,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_insert,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_insert,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#ប្រសិនបើ SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, jsonArrayValue, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, jsonArrayValue, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, jsonObjectValue, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, jsonObjectValue, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3InsertBuiltinFuncs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#ប្រសិនបើ  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, const var_na *zName){
  unsigned ចំនួនគត់ i;
  static const var_na *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  assert( sqlite3HashFind(&db->aModule, zName)==0 );
  សម្រាប់(i=0; i<parimaa_na(azModule)/parimaa_na(azModule[0]); i++){
    ប្រសិនបើ( sqlite3StrICmp(azModule[i],zName)==0 ){
      ត្រឡប់ sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  ត្រឡប់ 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
