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
#ضمن "sqliteInt.h"

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
ساكن ثابت حرف * ثابت jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
ساكن ثابت حرف jsonIsSpace[] = {
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
#define jsonIsspace(x) (jsonIsSpace[(unsigned حرف)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
ساكن ثابت حرف jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
ساكن ثابت حرف jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
ساكن ثابت حرف jsonIsOk[256] = {
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
تعريفنوع بنية JsonCache JsonCache;
تعريفنوع بنية JsonString JsonString;
تعريفنوع بنية JsonParse JsonParse;

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
بنية JsonCache {
  sqlite3 *db;                    /* Database connection */
  صحيح nUsed;                      /* Number of active entries in the cache */
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
بنية JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  حرف *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  حرف zSpace[100];        /* Initial static space */
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

#define JSON_داخل_کرو_TYPE(X)  (((X)&0xC)>>2)


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
بنية JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  حرف *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  صحيح nJson;         /* Length of the zJson string in bytes */
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
  صحيح delta;         /* Size change due to the edit */
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
#وإلا
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
ساكن فراغ jsonReturnStringAsBlob(JsonString*);
ساكن صحيح jsonArgIsJsonb(sqlite3_قدر *pJson, JsonParse *p);
ساكن u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
ساكن فراغ jsonReturnParse(sqlite3_context*,JsonParse*);
ساكن JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_قدر*,u32);
ساكن فراغ jsonParseFree(JsonParse*);
ساكن u32 jsonbPayloadSize(ثابت JsonParse*, u32, u32*);
ساكن u32 jsonUnescapeOneChar(ثابت حرف*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
ساكن فراغ jsonCacheDelete(JsonCache *p){
  صحيح i;
  لكل(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
ساكن فراغ jsonCacheDeleteGeneric(فراغ *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
ساكن صحيح json_Cache_داخل_کرو(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  أكد( pParse->zJson!=0 );
  أكد( pParse->bJsonIsRCStr );
  أكد( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  إذا( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, حجم(*p));
    إذا( p==0 ) أرجع SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    إذا( p==0 ) أرجع SQLITE_NOMEM;
  }
  إذا( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*حجم(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  أكد( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  أرجع SQLITE_OK;
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
ساكن JsonParse *json_Cache_تلاش(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_قدر *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  صحيح i;
  ثابت حرف *zJson;
  صحيح nJson;

  إذا( sqlite3_قدر_type(pArg)!=SQLITE_TEXT ){
    أرجع 0;
  }
  zJson = (ثابت حرف*)sqlite3_قدر_text(pArg);
  إذا( zJson==0 ) أرجع 0;
  nJson = sqlite3_قدر_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  إذا( p==0 ){
    أرجع 0;
  }
  لكل(i=0; i<p->nUsed; i++){
    إذا( p->a[i]->zJson==zJson ) اكسر;
  }
  إذا( i>=p->nUsed ){
    لكل(i=0; i<p->nUsed; i++){
      إذا( p->a[i]->nJson!=nJson ) استمر;
      إذا( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) اكسر;
    }
  }
  إذا( i<p->nUsed ){
    إذا( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*حجم(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    أكد( p->a[i]->delta==0 );
    أرجع p->a[i];
  }وإلا{
    أرجع 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
ساكن فراغ jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = حجم(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
ساكن فراغ jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
ساكن فراغ jsonStringReset(JsonString *p){
  إذا( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
ساكن فراغ jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  إذا( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
ساكن فراغ jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  أكد( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
ساكن صحيح jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  حرف *zNew;
  إذا( p->bStatic ){
    إذا( p->eErr ) أرجع 1;
    zNew = sqlite3RCStrNew(nTotal);
    إذا( zNew==0 ){
      jsonStringOom(p);
      أرجع SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }وإلا{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    إذا( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      أرجع SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  أرجع SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
ساكن SQLITE_NOINLINE فراغ jsonStringExpandAndAppend(
  JsonString *p,
  ثابت حرف *zIn,
  u32 N
){
  أكد( N>0 );
  إذا( jsonStringGrow(p,N) ) أرجع;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
ساكن فراغ jsonAppendRaw(JsonString *p, ثابت حرف *zIn, u32 N){
  إذا( N==0 ) أرجع;
  إذا( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }وإلا{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
ساكن فراغ jsonAppendRawNZ(JsonString *p, ثابت حرف *zIn, u32 N){
  أكد( N>0 );
  إذا( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }وإلا{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
ساكن فراغ jsonPrintf(صحيح N, JsonString *p, ثابت حرف *zFormat, ...){
  va_list ap;
  إذا( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) أرجع;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (صحيح)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
ساكن SQLITE_NOINLINE فراغ jsonAppendCharExpand(JsonString *p, حرف c){
  إذا( jsonStringGrow(p,1) ) أرجع;
  p->zBuf[p->nUsed++] = c;
}
ساكن فراغ jsonAppendChar(JsonString *p, حرف c){
  إذا( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }وإلا{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
ساكن فراغ jsonStringTrimOneChar(JsonString *p){
  إذا( p->eErr==0 ){
    أكد( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
ساكن صحيح jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  أرجع p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
ساكن فراغ jsonAppendSeparator(JsonString *p){
  حرف c;
  إذا( p->nUsed==0 ) أرجع;
  c = p->zBuf[p->nUsed-1];
  إذا( c=='[' || c=='{' ) أرجع;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
ساكن فراغ jsonAppendControlChar(JsonString *p, u8 c){
  ساكن ثابت حرف aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  أكد( حجم(aSpecial)==32 );
  أكد( aSpecial['\b']=='b' );
  أكد( aSpecial['\f']=='f' );
  أكد( aSpecial['\n']=='n' );
  أكد( aSpecial['\r']=='r' );
  أكد( aSpecial['\t']=='t' );
  أكد( c>=0 && c<حجم(aSpecial) );
  أكد( p->nUsed+7 <= p->nAlloc );
  إذا( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }وإلا{
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
ساكن فراغ jsonAppendString(JsonString *p, ثابت حرف *zIn, u32 N){
  u32 k;
  u8 c;
  ثابت u8 *z = (ثابت u8*)zIn;
  إذا( z==0 ) أرجع;
  إذا( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) أرجع;
  p->zBuf[p->nUsed++] = '"';
  طالما( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    طالما( 1 /* Exit by break */ ){
      إذا( k+3>=N ){
        طالما( k<N && jsonIsOk[z[k]] ){ k++; }
        اكسر;
      }
      إذا( !jsonIsOk[z[k]] ){
        اكسر;
      }
      إذا( !jsonIsOk[z[k+1]] ){
        k += 1;
        اكسر;
      }
      إذا( !jsonIsOk[z[k+2]] ){
        k += 2;
        اكسر;
      }
      إذا( !jsonIsOk[z[k+3]] ){
        k += 3;
        اكسر;
      }وإلا{
        k += 4;
      }
    }
    إذا( k>=N ){
      إذا( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      اكسر;
    }
    إذا( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    إذا( c=='"' || c=='\\' ){
      إذا( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) أرجع;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }وإلا إذا( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }وإلا{
      إذا( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) أرجع;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  أكد( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
ساكن فراغ json_Append_Sql_قدر(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_قدر *p_قدر          /* Value to append */
){
  بدل( sqlite3_قدر_type(p_قدر) ){
    حالة SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      اكسر;
    }
    حالة SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_قدر_double(p_قدر));
      اكسر;
    }
    حالة SQLITE_INTEGER: {
      ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_text(p_قدر);
      u32 n = (u32)sqlite3_قدر_bytes(p_قدر);
      jsonAppendRaw(p, z, n);
      اكسر;
    }
    حالة SQLITE_TEXT: {
      ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_text(p_قدر);
      u32 n = (u32)sqlite3_قدر_bytes(p_قدر);
      إذا( sqlite3_قدر_subtype(p_قدر)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }وإلا{
        jsonAppendString(p, z, n);
      }
      اكسر;
    }
    افتراضي: {
      JsonParse px;
      memset(&px, 0, حجم(px));
      إذا( jsonArgIsJsonb(p_قدر, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }وإلا إذا( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      اكسر;
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
ساكن فراغ jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  أكد( (pParse!=0)==(ctx!=0) );
  أكد( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  إذا( p->eErr==0 ){
    صحيح flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    إذا( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }وإلا إذا( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }وإلا{
      إذا( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        صحيح rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_داخل_کرو(ctx, pParse);
        إذا( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          أرجع;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }وإلا إذا( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }وإلا إذا( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }وإلا إذا( p->eErr & JSTRING_MALFORMED ){
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
ساكن فراغ jsonParseReset(JsonParse *pParse){
  أكد( pParse->nJPRef<=1 );
  إذا( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  إذا( pParse->nBlobAlloc ){
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
ساكن فراغ jsonParseFree(JsonParse *pParse){
  إذا( pParse ){
    إذا( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }وإلا{
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
ساكن u8 jsonHexToInt(صحيح h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  أرجع (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
ساكن u32 jsonHexToInt4(ثابت حرف *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  أرجع v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
ساكن صحيح jsonIs2Hex(ثابت حرف *z){
  أرجع sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
ساكن صحيح jsonIs4Hex(ثابت حرف *z){
  أرجع jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
ساكن صحيح json5Whitespace(ثابت حرف *zIn){
  صحيح n = 0;
  ثابت u8 *z = (u8*)zIn;
  طالما( 1 /*exit by "goto whitespace_done"*/ ){
    بدل( z[n] ){
      حالة 0x09:
      حالة 0x0a:
      حالة 0x0b:
      حالة 0x0c:
      حالة 0x0d:
      حالة 0x20: {
        n++;
        اكسر;
      }
      حالة '/': {
        إذا( z[n+1]=='*' && z[n+2]!=0 ){
          صحيح j;
          لكل(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            إذا( z[j]==0 ) اذهب whitespace_done;
          }
          n = j+1;
          اكسر;
        }وإلا إذا( z[n+1]=='/' ){
          صحيح j;
          حرف c;
          لكل(j=n+2; (c = z[j])!=0; j++){
            إذا( c=='\n' || c=='\r' ) اكسر;
            إذا( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              اكسر;
            }
          }
          n = j;
          إذا( z[n] ) n++;
          اكسر;
        }
        اذهب whitespace_done;
      }
      حالة 0xc2: {
        إذا( z[n+1]==0xa0 ){
          n += 2;
          اكسر;
        }
        اذهب whitespace_done;
      }
      حالة 0xe1: {
        إذا( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          اكسر;
        }
        اذهب whitespace_done;
      }
      حالة 0xe2: {
        إذا( z[n+1]==0x80 ){
          u8 c = z[n+2];
          إذا( c<0x80 ) اذهب whitespace_done;
          إذا( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            اكسر;
          }
        }وإلا إذا( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          اكسر;
        }
        اذهب whitespace_done;
      }
      حالة 0xe3: {
        إذا( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          اكسر;
        }
        اذهب whitespace_done;
      }
      حالة 0xef: {
        إذا( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          اكسر;
        }
        اذهب whitespace_done;
      }
      افتراضي: {
        اذهب whitespace_done;
      }
    }
  }
  whitespace_done:
  أرجع n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
ساكن ثابت بنية NanInfName {
  حرف c1;
  حرف c2;
  حرف n;
  حرف eType;
  حرف nRepl;
  حرف *zMatch;
  حرف *zRepl;
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
ساكن فراغ json_Wrong_عدد_Args(
  sqlite3_context *pCtx,
  ثابت حرف *zFuncName
){
  حرف *z_پیغام = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, z_پیغام, -1);
  sqlite3_free(z_پیغام);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
ساكن صحيح jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  أكد( N>pParse->nBlobAlloc );
  إذا( pParse->nBlobAlloc==0 ){
    t = 100;
  }وإلا{
    t = pParse->nBlobAlloc*2;
  }
  إذا( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  إذا( aNew==0 ){ pParse->oom = 1; أرجع 1; }
  أكد( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  أرجع 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
ساكن صحيح jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 nSize;
  أكد( !pParse->bReadOnly );
  إذا( pParse->oom ) أرجع 0;
  إذا( pParse->nBlobAlloc>0 ) أرجع 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  إذا( jsonBlobExpand(pParse, nSize) ){
    أرجع 0;
  }
  أكد( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  أرجع 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
ساكن SQLITE_NOINLINE فراغ jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  إذا( pParse->oom==0 ){
    أكد( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
ساكن فراغ jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  إذا( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }وإلا{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
ساكن فراغ json_Blob_Append_گرہ(JsonParse*,u8,u64,ثابت فراغ*);
ساكن SQLITE_NOINLINE فراغ json_Blob_Expand_And_Append_گرہ(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ثابت فراغ *aPayload
){
  إذا( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) أرجع;
  json_Blob_Append_گرہ(pParse, eType, szPayload, aPayload);
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
ساكن فراغ json_Blob_Append_گرہ(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  ثابت فراغ *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  إذا( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_گرہ(pParse,eType,szPayload,aPayload);
    أرجع;
  }
  أكد( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  إذا( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }وإلا إذا( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }وإلا إذا( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }وإلا{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  إذا( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
ساكن صحيح jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  صحيح delta;
  إذا( pParse->oom ) أرجع 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  إذا( szType<=11 ){
    nExtra = 0;
  }وإلا إذا( szType==12 ){
    nExtra = 1;
  }وإلا إذا( szType==13 ){
    nExtra = 2;
  }وإلا إذا( szType==14 ){
    nExtra = 4;
  }وإلا{
    nExtra = 8;
  }
  إذا( szPayload<=11 ){
    nNeeded = 0;
  }وإلا إذا( szPayload<=0xff ){
    nNeeded = 1;
  }وإلا إذا( szPayload<=0xffff ){
    nNeeded = 2;
  }وإلا{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  إذا( delta ){
    u64 newSize = (u64)pParse->nBlob + delta;
    إذا( delta>0 ){
      إذا( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        أرجع 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }وإلا{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  إذا( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }وإلا إذا( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }وإلا إذا( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }وإلا{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  أرجع delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
ساكن صحيح jsonIs4HexB(ثابت حرف *z, صحيح *pOp){
  إذا( z[0]!='u' ) أرجع 0;
  إذا( !jsonIs4Hex(&z[1]) ) أرجع 0;
  *pOp = JSONB_TEXTJ;
  أرجع 1;
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
ساكن u32 jsonbValidityCheck(
  ثابت JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ثابت u8 *z;
  u8 x;
  إذا( iDepth>JSON_MAX_DEPTH ) أرجع i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  إذا( NEVER(n==0) ) أرجع i+1;          /* Checked by caller */
  إذا( NEVER(i+n+sz!=iEnd) ) أرجع i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  بدل( x ){
    حالة JSONB_NULL:
    حالة JSONB_TRUE:
    حالة JSONB_FALSE: {
      أرجع n+sz==1 ? 0 : i+1;
    }
    حالة JSONB_INT: {
      إذا( sz<1 ) أرجع i+1;
      j = i+n;
      إذا( z[j]=='-' ){
        j++;
        إذا( sz<2 ) أرجع i+1;
      }
      k = i+n+sz;
      طالما( j<k ){
        إذا( sqlite3Isdigit(z[j]) ){
          j++;
        }وإلا{
          أرجع j+1;
        }
      }
      أرجع 0;
    }
    حالة JSONB_INT5: {
      إذا( sz<3 ) أرجع i+1;
      j = i+n;
      إذا( z[j]=='-' ){
        إذا( sz<4 ) أرجع i+1;
        j++;
      }
      إذا( z[j]!='0' ) أرجع i+1;
      إذا( z[j+1]!='x' && z[j+1]!='X' ) أرجع j+2;
      j += 2;
      k = i+n+sz;
      طالما( j<k ){
        إذا( sqlite3Isxdigit(z[j]) ){
          j++;
        }وإلا{
          أرجع j+1;
        }
      }
      أرجع 0;
    }
    حالة JSONB_FLOAT:
    حالة JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      إذا( sz<2 ) أرجع i+1;
      j = i+n;
      k = j+sz;
      إذا( z[j]=='-' ){
        j++;
        إذا( sz<3 ) أرجع i+1;
      }
      إذا( z[j]=='.' ){
        إذا( x==JSONB_FLOAT ) أرجع j+1;
        إذا( !sqlite3Isdigit(z[j+1]) ) أرجع j+1;
        j += 2;
        seen = 1;
      }وإلا إذا( z[j]=='0' && x==JSONB_FLOAT ){
        إذا( j+3>k ) أرجع j+1;
        إذا( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) أرجع j+1;
        j++;
      }
      لكل(; j<k; j++){
        إذا( sqlite3Isdigit(z[j]) ) استمر;
        إذا( z[j]=='.' ){
          إذا( seen>0 ) أرجع j+1;
          إذا( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            أرجع j+1;
          }
          seen = 1;
          استمر;
        }
        إذا( z[j]=='e' || z[j]=='E' ){
          إذا( seen==2 ) أرجع j+1;
          إذا( j==k-1 ) أرجع j+1;
          إذا( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            إذا( j==k-1 ) أرجع j+1;
          }
          seen = 2;
          استمر;
        }
        أرجع j+1;
      }
      إذا( seen==0 ) أرجع i+1;
      أرجع 0;
    }
    حالة JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      طالما( j<k ){
        إذا( !jsonIsOk[z[j]] && z[j]!='\'' ) أرجع j+1;
        j++;
      }
      أرجع 0;
    }
    حالة JSONB_TEXTJ:
    حالة JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      طالما( j<k ){
        إذا( !jsonIsOk[z[j]] && z[j]!='\'' ){
          إذا( z[j]=='"' ){
            إذا( x==JSONB_TEXTJ ) أرجع j+1;
          }وإلا إذا( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            إذا( x==JSONB_TEXTJ ) أرجع j+1;
          }وإلا إذا( NEVER(z[j]!='\\') || j+1>=k ){
            أرجع j+1;
          }وإلا إذا( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }وإلا إذا( z[j+1]=='u' ){
            إذا( j+5>=k ) أرجع j+1;
            إذا( !jsonIs4Hex((ثابت حرف*)&z[j+2]) ) أرجع j+1;
            j++;
          }وإلا إذا( x!=JSONB_TEXT5 ){
            أرجع j+1;
          }وإلا{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((ثابت حرف*)&z[j], k-j, &c);
            إذا( c==JSON_INVALID_CHAR ) أرجع j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      أرجع 0;
    }
    حالة JSONB_TEXTRAW: {
      أرجع 0;
    }
    حالة JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      طالما( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        إذا( n==0 ) أرجع j+1;
        إذا( j+n+sz>k ) أرجع j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        إذا( sub ) أرجع sub;
        j += n + sz;
      }
      أكد( j==k );
      أرجع 0;
    }
    حالة JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      طالما( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        إذا( n==0 ) أرجع j+1;
        إذا( j+n+sz>k ) أرجع j+1;
        إذا( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          إذا( x<JSONB_TEXT || x>JSONB_TEXTRAW ) أرجع j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        إذا( sub ) أرجع sub;
        cnt++;
        j += n + sz;
      }
      أكد( j==k );
      إذا( (cnt & 1)!=0 ) أرجع j+1;
      أرجع 0;
    }
    افتراضي: {
      أرجع i+1;
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
ساكن صحيح jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  حرف c;
  u32 j;
  u32 iThis, iStart;
  صحيح x;
  u8 t;
  ثابت حرف *z = pParse->zJson;
json_parse_restart:
  بدل( (u8)z[i] ){
  حالة '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_گرہ(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    إذا( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      أرجع -1;
    }
    iStart = pParse->nBlob;
    لكل(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      إذا( x<=0 ){
        صحيح op;
        إذا( x==(-2) ){
          j = pParse->iErr;
          إذا( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          اكسر;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        إذا( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          صحيح k = j+1;
          طالما( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          أكد( iBlob==pParse->nBlob );
          json_Blob_Append_گرہ(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }وإلا{
          إذا( x!=-1 ) pParse->iErr = j;
          أرجع -1;
        }
      }
      إذا( pParse->oom ) أرجع -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      إذا( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        أرجع -1;
      }
      j = x;
      إذا( z[j]==':' ){
        j++;
      }وإلا{
        إذا( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          افعل{ j++; }طالما( jsonIsspace(z[j]) );
          إذا( z[j]==':' ){
            j++;
            اذهب parse_object_قدر;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        إذا( x!=(-5) ){
          إذا( x!=(-1) ) pParse->iErr = j;
          أرجع -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_قدر:
      x = jsonTranslateTextToBlob(pParse, j);
      إذا( x<=0 ){
        إذا( x!=(-1) ) pParse->iErr = j;
        أرجع -1;
      }
      j = x;
      إذا( z[j]==',' ){
        استمر;
      }وإلا إذا( z[j]=='}' ){
        اكسر;
      }وإلا{
        إذا( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          إذا( z[j]==',' ){
            استمر;
          }وإلا إذا( z[j]=='}' ){
            اكسر;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        إذا( x==(-4) ){
          j = pParse->iErr;
          استمر;
        }
        إذا( x==(-2) ){
          j = pParse->iErr;
          اكسر;
        }
      }
      pParse->iErr = j;
      أرجع -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    أرجع j+1;
  }
  حالة '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    أكد( i<=(u32)pParse->nJson );
    json_Blob_Append_گرہ(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    إذا( pParse->oom ) أرجع -1;
    إذا( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      أرجع -1;
    }
    لكل(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      إذا( x<=0 ){
        إذا( x==(-3) ){
          j = pParse->iErr;
          إذا( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          اكسر;
        }
        إذا( x!=(-1) ) pParse->iErr = j;
        أرجع -1;
      }
      j = x;
      إذا( z[j]==',' ){
        استمر;
      }وإلا إذا( z[j]==']' ){
        اكسر;
      }وإلا{
        إذا( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          إذا( z[j]==',' ){
            استمر;
          }وإلا إذا( z[j]==']' ){
            اكسر;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        إذا( x==(-4) ){
          j = pParse->iErr;
          استمر;
        }
        إذا( x==(-3) ){
          j = pParse->iErr;
          اكسر;
        }
      }
      pParse->iErr = j;
      أرجع -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    أرجع j+1;
  }
  حالة '\'': {
    u8 opcode;
    حرف cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    اذهب parse_string;
  حالة '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    طالما( 1 /*exit-by-break*/ ){
      إذا( jsonIsOk[(u8)z[j]] ){
        إذا( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }وإلا إذا( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }وإلا{
          j += 3;
          استمر;
        }
      }
      c = z[j];
      إذا( c==cDelim ){
        اكسر;
      }وإلا إذا( c=='\\' ){
        c = z[++j];
        إذا( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          إذا( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }وإلا إذا( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#وإلا
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }وإلا إذا( c=='\r' ){
          إذا( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }وإلا{
          pParse->iErr = j;
          أرجع -1;
        }
      }وإلا إذا( c<=0x1f ){
        إذا( c==0 ){
          pParse->iErr = j;
          أرجع -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }وإلا إذا( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_گرہ(pParse, opcode, j-1-i, &z[i+1]);
    أرجع j+1;
  }
  حالة 't': {
    إذا( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      أرجع i+4;
    }
    pParse->iErr = i;
    أرجع -1;
  }
  حالة 'f': {
    إذا( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      أرجع i+5;
    }
    pParse->iErr = i;
    أرجع -1;
  }
  حالة '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    اذهب parse_نمبر;
  حالة '.':
    إذا( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      اذهب parse_نمبر_2;
    }
    pParse->iErr = i;
    أرجع -1;
  حالة '-':
  حالة '0':
  حالة '1':
  حالة '2':
  حالة '3':
  حالة '4':
  حالة '5':
  حالة '6':
  حالة '7':
  حالة '8':
  حالة '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_نمبر:
    seenE = 0;
    أكد( '-' < '0' );
    أكد( '+' < '0' );
    أكد( '.' < '0' );
    c = z[i];

    إذا( c<='0' ){
      إذا( c=='0' ){
        إذا( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          أكد( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          لكل(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          اذهب parse_نمبر_finish;
        }وإلا إذا( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          أرجع -1;
        }
      }وإلا{
        إذا( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          إذا( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            إذا( z[i]=='-' ){
              json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 6, "-9e999");
            }وإلا{
              json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 5, "9e999");
            }
            أرجع i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          إذا( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            اذهب parse_نمبر_2;
          }
          pParse->iErr = i;
          أرجع -1;
        }
        إذا( z[i+1]=='0' ){
          إذا( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            أرجع -1;
          }وإلا إذا( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            لكل(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            اذهب parse_نمبر_finish;
          }
        }
      }
    }

  parse_نمبر_2:
    لكل(j=i+1;; j++){
      c = z[j];
      إذا( sqlite3Isdigit(c) ) استمر;
      إذا( c=='.' ){
        إذا( (t & 0x02)!=0 ){
          pParse->iErr = j;
          أرجع -1;
        }
        t |= 0x02;
        استمر;
      }
      إذا( c=='e' || c=='E' ){
        إذا( z[j-1]<'0' ){
          إذا( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }وإلا{
            pParse->iErr = j;
            أرجع -1;
          }
        }
        إذا( seenE ){
          pParse->iErr = j;
          أرجع -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        إذا( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        إذا( c<'0' || c>'9' ){
          pParse->iErr = j;
          أرجع -1;
        }
        استمر;
      }
      اكسر;
    }
    إذا( z[j-1]<'0' ){
      إذا( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }وإلا{
        pParse->iErr = j;
        أرجع -1;
      }
    }
  parse_نمبر_finish:
    أكد( JSONB_INT+0x01==JSONB_INT5 );
    أكد( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    أكد( JSONB_INT+0x02==JSONB_FLOAT );
    إذا( z[i]=='+' ) i++;
    json_Blob_Append_گرہ(pParse, JSONB_INT+t, j-i, &z[i]);
    أرجع j;
  }
  حالة '}': {
    pParse->iErr = i;
    أرجع -2;  /* End of {...} */
  }
  حالة ']': {
    pParse->iErr = i;
    أرجع -3;  /* End of [...] */
  }
  حالة ',': {
    pParse->iErr = i;
    أرجع -4;  /* List separator */
  }
  حالة ':': {
    pParse->iErr = i;
    أرجع -5;  /* Object label/value separator */
  }
  حالة 0: {
    أرجع 0;   /* End of file */
  }
  حالة 0x09:
  حالة 0x0a:
  حالة 0x0d:
  حالة 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    اذهب json_parse_restart;
  }
  حالة 0x0b:
  حالة 0x0c:
  حالة '/':
  حالة 0xc2:
  حالة 0xe1:
  حالة 0xe2:
  حالة 0xe3:
  حالة 0xef: {
    j = json5Whitespace(&z[i]);
    إذا( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      اذهب json_parse_restart;
    }
    pParse->iErr = i;
    أرجع -1;
  }
  حالة 'n': {
    إذا( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      أرجع i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  افتراضي: {
    u32 k;
    صحيح nn;
    c = z[i];
    لكل(k=0; k<حجم(aNanInfName)/حجم(aNanInfName[0]); k++){
      إذا( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) استمر;
      nn = aNanInfName[k].n;
      إذا( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        استمر;
      }
      إذا( sqlite3Isalnum(z[i+nn]) ) استمر;
      إذا( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 5, "9e999");
      }وإلا{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      أرجع i + nn;
    }
    pParse->iErr = i;
    أرجع -1;  /* Syntax error */
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
ساكن صحيح jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  صحيح i;
  ثابت حرف *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  إذا( pParse->oom ) i = -1;
  إذا( i>0 ){
#ifdef SQLITE_DEBUG
    أكد( pParse->iDepth==0 );
    إذا( sqlite3Config.bJsonSelfcheck ){
      أكد( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    طالما( jsonIsspace(zJson[i]) ) i++;
    إذا( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      إذا( zJson[i] ){
        إذا( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        أرجع 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  إذا( i<=0 ){
    إذا( pCtx!=0 ){
      إذا( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }وإلا{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    أرجع 1;
  }
  أرجع 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
ساكن فراغ jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  أكد( pStr->eErr==0 );
  memset(&px, 0, حجم(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (فراغ)jsonTranslateTextToBlob(&px, 0);
  إذا( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }وإلا{
    أكد( px.nBlobAlloc>0 );
    أكد( !px.bReadOnly );
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
ساكن u32 jsonbPayloadSizeWide(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  أكد( i<pParse->nBlob );
  أكد( x==(pParse->aBlob[i]>>4) );
  أكد( x>=13 );
  إذا( x==13 ){
    إذا( i+2>=pParse->nBlob ){
      *pSz = 0;
      أرجع 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }وإلا إذا( x==14 ){
    إذا( i+4>=pParse->nBlob ){
      *pSz = 0;
      أرجع 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }وإلا{
    إذا( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      أرجع 0;
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
  إذا( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    أرجع 0;
  }
  *pSz = sz;
  أرجع n;
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
ساكن u32 jsonbPayloadSize(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  إذا( i>=pParse->nBlob ){
    *pSz = 0;
    أرجع 0;
  }وإلا إذا( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }وإلا إذا( x==12 ){
    إذا( i+1>=pParse->nBlob ){
      *pSz = 0;
      أرجع 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }وإلا{
    أرجع jsonbPayloadSizeWide(pParse, i, pSz, x);
  }
  إذا( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    أرجع 0;
  }
  *pSz = sz;
  أرجع n;
}

#إذا SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
ساكن SQLITE_INLINE u32 jsonbPayloadSizeInline(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  إذا( i>=pParse->nBlob ){
    *pSz = 0;
    أرجع 0;
  }وإلا إذا( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }وإلا إذا( x==12 ){
    إذا( i+1>=pParse->nBlob ){
      *pSz = 0;
      أرجع 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }وإلا إذا( x==13 ){
    إذا( i+2>=pParse->nBlob ){
      *pSz = 0;
      أرجع 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }وإلا إذا( x==14 ){
    إذا( i+4>=pParse->nBlob ){
      *pSz = 0;
      أرجع 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }وإلا{
    إذا( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      أرجع 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  إذا( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    أرجع 0;
  }
  *pSz = sz;
  أرجع n;
}
#وإلا /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif


#إذا SQLITE_USES_INLINE
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
ساكن SQLITE_INLINE u32 jsonbPayloadSizeSemiInline(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  أكد( i<pParse->nBlob );
  إذا( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }وإلا إذا( x==12 ){
    إذا( i+1>=pParse->nBlob ){
      *pSz = 0;
      أرجع 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }وإلا{
    أرجع jsonbPayloadSizeWide(pParse,i,pSz,x);
  }
  إذا( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    أرجع 0;
  }
  *pSz = sz;
  أرجع n;
}
#وإلا /* if !SQLITE_USES_INLINE */
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
ساكن u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSizeInline(pParse, i, &sz);
  إذا( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    أرجع pParse->nBlob+1;
  }
  بدل( pParse->aBlob[i] & 0x0f ){
    حالة JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      أرجع i+1;
    }
    حالة JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      أرجع i+1;
    }
    حالة JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      أرجع i+1;
    }
    حالة JSONB_INT:
    حالة JSONB_FLOAT: {
      إذا( sz==0 ) اذهب malformed_jsonb;
      jsonAppendRaw(pOut, (ثابت حرف*)&pParse->aBlob[i+n], sz);
      اكسر;
    }
    حالة JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ثابت حرف *zIn = (ثابت حرف*)&pParse->aBlob[i+n];
      صحيح bOverflow = 0;
      إذا( sz==0 ) اذهب malformed_jsonb;
      إذا( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }وإلا إذا( zIn[0]=='+' ){
        k++;
      }
      لكل(; k<sz; k++){
        إذا( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          اكسر;
        }وإلا إذا( (u>>60)!=0 ){
          bOverflow = 1;
        }وإلا{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      اكسر;
    }
    حالة JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ثابت حرف *zIn = (ثابت حرف*)&pParse->aBlob[i+n];
      إذا( sz==0 ) اذهب malformed_jsonb;
      إذا( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        إذا( sz<=1 ) اذهب malformed_jsonb;
        k = 1;
      }
      إذا( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      لكل(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        إذا( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      اكسر;
    }
    حالة JSONB_TEXT:
    حالة JSONB_TEXTJ: {
      إذا( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(ثابت حرف*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      اكسر;
    }
    حالة JSONB_TEXT5: {
      ثابت حرف *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (ثابت حرف*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      طالما( sz2>0 ){
        لكل(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        إذا( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          إذا( k>=sz2 ){
            اكسر;
          }
          zIn += k;
          sz2 -= k;
        }
        إذا( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          استمر;
        }
        إذا( zIn[0]<=0x1f ){
          إذا( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) اكسر;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          استمر;
        }
        أكد( zIn[0]=='\\' );
        أكد( sz2>=1 );
        إذا( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          اكسر;
        }
        بدل( (u8)zIn[1] ){
          حالة '\'':
            jsonAppendChar(pOut, '\'');
            اكسر;
          حالة 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            اكسر;
          حالة 'x':
            إذا( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              اكسر;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            اكسر;
          حالة '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            اكسر;
          حالة '\r':
            إذا( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            اكسر;
          حالة '\n':
            اكسر;
          حالة 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            إذا( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              اكسر;
            }
            zIn += 2;
            sz2 -= 2;
            اكسر;
          افتراضي:
            jsonAppendRawNZ(pOut, zIn, 2);
            اكسر;
        }
        أكد( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      اكسر;
    }
    حالة JSONB_TEXTRAW: {
      jsonAppendString(pOut, (ثابت حرف*)&pParse->aBlob[i+n], sz);
      اكسر;
    }
    حالة JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      إذا( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      طالما( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      إذا( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      إذا( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      اكسر;
    }
    حالة JSONB_OBJECT: {
      صحيح x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      إذا( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      طالما( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      إذا( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      إذا( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      اكسر;
    }

    افتراضي: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      اكسر;
    }
  }
  أرجع i+n+sz;
}

/* Context for recursion of json_pretty()
*/
تعريفنوع بنية JsonPretty JsonPretty;
بنية JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  ثابت حرف *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
ساكن فراغ jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  لكل(jj=0; jj<pPretty->nIndent; jj++){
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
ساكن u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  إذا( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    أرجع pParse->nBlob+1;
  }
  بدل( pParse->aBlob[i] & 0x0f ){
    حالة JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      إذا( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        إذا( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        طالما( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          إذا( j>=iEnd ){
            إذا( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            اكسر;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      اكسر;
    }
    حالة JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      إذا( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        إذا( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        طالما( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          إذا( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            اكسر;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          إذا( j>=iEnd ) اكسر;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      اكسر;
    }
    افتراضي: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      اكسر;
    }
  }
  أرجع i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
ساكن u32 jsonb_Array_گنتی(JsonParse *pParse, u32 i_جڑ){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, i_جڑ, &sz);
  iEnd = i_جڑ+n+sz;
  لكل(i=i_جڑ+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSizeSemiInline(pParse, i, &sz);
  }
  أرجع k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
ساكن فراغ jsonAfterEditSizeAdjust(JsonParse *pParse, u32 i_جڑ){
  u32 sz = 0;
  u32 nBlob;
  أكد( pParse->delta!=0 );
  أكد( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (فراغ)jsonbPayloadSize(pParse, i_جڑ, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, i_جڑ, sz);
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
ساكن صحيح jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  ثابت u8 *aIns,           /* New content */
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
  ساكن ثابت u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  إذا( (aIns[0]&0x0f)<=2 ) أرجع 0;    /* Cannot enlarge NULL, true, false */
  بدل( aIns[0]>>4 ){
    افتراضي: {                         /* aIns[] header size 1 */
      إذا( ((1<<d)&0x116)==0 ) أرجع 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      اكسر;
    }
    حالة 12: {                         /* aIns[] header size is 2 */
      إذا( ((1<<d)&0x8a)==0) أرجع 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      اكسر;
    }
    حالة 13: {                         /* aIns[] header size is 3 */
      إذا( d!=2 && d!=6 ) أرجع 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      اكسر;
    }
    حالة 14: {                         /* aIns[] header size is 5 */
      إذا( d!=4 ) أرجع 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      اكسر;
    }
    حالة 15: {                         /* aIns[] header size is 9 */
      أرجع 0;                          /* No solution */
    }
  }
  أكد( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  طالما( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    إذا( i==1 ) اكسر;
    szPayload >>= 8;
  }
  أكد( (szPayload>>8)==0 );
  أرجع 1;
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
ساكن فراغ jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  ثابت u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  أكد( pParse->nBlob >= (u64)iDel + (u64)nDel );
  إذا( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (صحيح)-d)
  ){
    أرجع;
  }
  إذا( d!=0 ){
    إذا( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      إذا( pParse->oom ) أرجع;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  إذا( nIns && aIns ){
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
ساكن u32 jsonBytesToBypass(ثابت حرف *z, u32 n){
  u32 i = 0;
  طالما( i+1<n ){
    إذا( z[i]!='\\' ) أرجع i;
    إذا( z[i+1]=='\n' ){
      i += 2;
      استمر;
    }
    إذا( z[i+1]=='\r' ){
      إذا( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }وإلا{
        i += 2;
      }
      استمر;
    }
    إذا( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      استمر;
    }
    اكسر;
  }
  أرجع i;
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
ساكن u32 jsonUnescapeOneChar(ثابت حرف *z, u32 n, u32 *piOut){
  أكد( n>0 );
  أكد( z[0]=='\\' );
  إذا( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    أرجع n;
  }
  بدل( (u8)z[1] ){
    حالة 'u': {
      u32 v, vlo;
      إذا( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        أرجع n;
      }
      v = jsonHexToInt4(&z[2]);
      إذا( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        أرجع 12;
      }وإلا{
        *piOut = v;
        أرجع 6;
      }
    }
    حالة 'b': {   *piOut = '\b';  أرجع 2; }
    حالة 'f': {   *piOut = '\f';  أرجع 2; }
    حالة 'n': {   *piOut = '\n';  أرجع 2; }
    حالة 'r': {   *piOut = '\r';  أرجع 2; }
    حالة 't': {   *piOut = '\t';  أرجع 2; }
    حالة 'v': {   *piOut = '\v';  أرجع 2; }
    حالة '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#وإلا
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      أرجع 2;
    }
    حالة '\'':
    حالة '"':
    حالة '/':
    حالة '\\':{   *piOut = z[1];  أرجع 2; }
    حالة 'x': {
      إذا( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        أرجع n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      أرجع 4;
    }
    حالة 0xe2:
    حالة '\r':
    حالة '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      إذا( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        أرجع n;
      }وإلا إذا( nSkip==n ){
        *piOut = 0;
        أرجع n;
      }وإلا إذا( z[nSkip]=='\\' ){
        أرجع nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }وإلا{
        صحيح sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        أرجع nSkip + sz;
      }
    }
    افتراضي: {
      *piOut = JSON_INVALID_CHAR;
      أرجع 2;
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
ساكن SQLITE_NOINLINE صحيح jsonLabelCompareEscaped(
  ثابت حرف *z_بایاں,          /* The left label */
  u32 n_بایاں,                  /* Size of the left label in bytes */
  صحيح raw_بایاں,                /* True if zLeft contains no escapes */
  ثابت حرف *z_دایاں,         /* The right label */
  u32 n_دایاں,                 /* Size of the right label in bytes */
  صحيح raw_دایاں                /* True if zRight is escape-free */
){
  u32 c_بایاں, c_دایاں;
  أكد( raw_بایاں==0 || raw_دایاں==0 );
  طالما( 1 /*exit-by-return*/ ){
    إذا( n_بایاں==0 ){
      c_بایاں = 0;
    }وإلا إذا( raw_بایاں || z_بایاں[0]!='\\' ){
      c_بایاں = ((u8*)z_بایاں)[0];
      إذا( c_بایاں>=0xc0 ){
        صحيح sz = sqlite3Utf8ReadLimited((u8*)z_بایاں, n_بایاں, &c_بایاں);
        z_بایاں += sz;
        n_بایاں -= sz;
      }وإلا{
        z_بایاں++;
        n_بایاں--;
      }
    }وإلا{
      u32 n = jsonUnescapeOneChar(z_بایاں, n_بایاں, &c_بایاں);
      z_بایاں += n;
      أكد( n<=n_بایاں );
      n_بایاں -= n;
    }
    إذا( n_دایاں==0 ){
      c_دایاں = 0;
    }وإلا إذا( raw_دایاں || z_دایاں[0]!='\\' ){
      c_دایاں = ((u8*)z_دایاں)[0];
      إذا( c_دایاں>=0xc0 ){
        صحيح sz = sqlite3Utf8ReadLimited((u8*)z_دایاں, n_دایاں, &c_دایاں);
        z_دایاں += sz;
        n_دایاں -= sz;
      }وإلا{
        z_دایاں++;
        n_دایاں--;
      }
    }وإلا{
      u32 n = jsonUnescapeOneChar(z_دایاں, n_دایاں, &c_دایاں);
      z_دایاں += n;
      أكد( n<=n_دایاں );
      n_دایاں -= n;
    }
    إذا( c_بایاں!=c_دایاں ) أرجع 0;
    إذا( c_بایاں==0 ) أرجع 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
ساكن صحيح jsonLabelCompare(
  ثابت حرف *z_بایاں,          /* The left label */
  u32 n_بایاں,                  /* Size of the left label in bytes */
  صحيح raw_بایاں,                /* True if zLeft contains no escapes */
  ثابت حرف *z_دایاں,         /* The right label */
  u32 n_دایاں,                 /* Size of the right label in bytes */
  صحيح raw_دایاں                /* True if zRight is escape-free */
){
  إذا( raw_بایاں && raw_دایاں ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    إذا( n_بایاں!=n_دایاں ) أرجع 0;
    أرجع memcmp(z_بایاں, z_دایاں, n_بایاں)==0;
  }وإلا{
    أرجع jsonLabelCompareEscaped(z_بایاں, n_بایاں, raw_بایاں,
                                   z_دایاں, n_دایاں, raw_دایاں);
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
ساكن u32 jsonLookupStep(JsonParse*,u32,ثابت حرف*,u32);


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
ساكن u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  ثابت حرف *zTail   /* Tail of the path that determines substructure */
){
  ساكن ثابت u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  صحيح rc;
  memset(pIns, 0, حجم(*pIns));
  pIns->db = pParse->db;
  إذا( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }وإلا{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    إذا( pIns->iDepth >= JSON_MAX_DEPTH ){
      أرجع JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  أرجع rc;  /* Error code only */
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
ساكن u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_جڑ,              /* Begin the search at this element of aBlob[] */
  ثابت حرف *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_کلید, sz, n, iEnd, rc;
  ثابت حرف *z_کلید;
  u8 x;

  إذا( zPath[0]==0 ){
    إذا( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, i_جڑ, &sz);
      sz += n;
      إذا( pParse->eEdit==JEDIT_DEL ){
        إذا( iLabel>0 ){
          sz += i_جڑ - iLabel;
          i_جڑ = iLabel;
        }
        jsonBlobEdit(pParse, i_جڑ, sz, 0, 0);
      }وإلا إذا( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }وإلا إذا( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        إذا( zPath[-1]!=']' ){
          أرجع JSON_LOOKUP_NOTARRAY;
        }وإلا{
          jsonBlobEdit(pParse, i_جڑ, 0, pParse->aIns, pParse->nIns);
        }
      }وإلا{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_جڑ, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    أرجع i_جڑ;
  }
  إذا( zPath[0]=='.' ){
    صحيح raw_کلید = 1;
    x = pParse->aBlob[i_جڑ];
    zPath++;
    إذا( zPath[0]=='"' ){
      z_کلید = zPath + 1;
      لكل(i=1; zPath[i] && zPath[i]!='"'; i++){
        إذا( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_کلید = i-1;
      إذا( zPath[i] ){
        i++;
      }وإلا{
        أرجع JSON_LOOKUP_PATHERROR;
      }
      testcase( n_کلید==0 );
      raw_کلید = memchr(z_کلید, '\\', n_کلید)==0;
    }وإلا{
      z_کلید = zPath;
      لكل(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_کلید = i;
      إذا( n_کلید==0 ){
        أرجع JSON_LOOKUP_PATHERROR;
      }
    }
    إذا( (x & 0x0f)!=JSONB_OBJECT ) أرجع JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_جڑ, &sz);
    j = i_جڑ + n;  /* j is the index of a label */
    iEnd = j+sz;
    طالما( j<iEnd ){
      صحيح rawLabel;
      ثابت حرف *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      إذا( x<JSONB_TEXT || x>JSONB_TEXTRAW ) أرجع JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      إذا( n==0 ) أرجع JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      إذا( k+sz>=iEnd ) أرجع JSON_LOOKUP_ERROR;
      zLabel = (ثابت حرف*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      إذا( jsonLabelCompare(z_کلید, n_کلید, raw_کلید, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        إذا( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) أرجع JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        إذا( n==0 || v+n+sz>iEnd ) أرجع JSON_LOOKUP_ERROR;
        أكد( j>0 );
        إذا( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          أرجع JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        إذا( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_جڑ);
        أرجع rc;
      }
      j = k+sz;
      إذا( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) أرجع JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      إذا( n==0 ) أرجع JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    إذا( j>iEnd ) أرجع JSON_LOOKUP_ERROR;
    إذا( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      إذا( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        أرجع JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, حجم(ix));
      ix.db = pParse->db;
      json_Blob_Append_گرہ(&ix, raw_کلید?JSONB_TEXTRAW:JSONB_TEXT5, n_کلید, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      إذا( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_کلید+v.nBlob)
      ){
        أكد( !pParse->oom );
        nIns = ix.nBlob + n_کلید + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        إذا( !pParse->oom ){
          أكد( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          أكد( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], z_کلید, n_کلید);
          k += n_کلید;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          إذا( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, i_جڑ);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      أرجع rc;
    }
  }وإلا إذا( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_جڑ] & 0x0f;
    إذا( x!=JSONB_ARRAY )  أرجع JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_جڑ, &sz);
    i = 1;
    طالما( sqlite3Isdigit(zPath[i]) ){
      إذا( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    إذا( i<2 || zPath[i]!=']' ){
      إذا( zPath[1]=='#' ){
        kk = jsonb_Array_گنتی(pParse, i_جڑ);
        i = 2;
        إذا( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          افعل{
            إذا( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }طالما( sqlite3Isdigit(zPath[i]) );
          إذا( nn>kk ) أرجع JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        إذا( zPath[i]!=']' ){
          أرجع JSON_LOOKUP_PATHERROR;
        }
      }وإلا{
        أرجع JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_جڑ+n;
    iEnd = j+sz;
    طالما( j<iEnd ){
      إذا( kk==0 ){
        إذا( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          أرجع JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        إذا( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_جڑ);
        أرجع rc;
      }
      kk--;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      إذا( n==0 ) أرجع JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    إذا( j>iEnd ) أرجع JSON_LOOKUP_ERROR;
    إذا( kk>0 ) أرجع JSON_LOOKUP_NOTFOUND;
    إذا( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      إذا( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        أكد( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      إذا( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_جڑ);
      أرجع rc;
    }
  }وإلا{
    أرجع JSON_LOOKUP_PATHERROR; 
  }
  أرجع JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
ساكن فراغ jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  ثابت u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  إذا( NEVER(aBlob==0) ) أرجع;
  memset(&x, 0, حجم(x));
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
ساكن فراغ jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  صحيح eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  صحيح rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  أكد( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  إذا( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    أرجع;
  }
  بدل( pParse->aBlob[i] & 0x0f ){
    حالة JSONB_NULL: {
      إذا( sz ) اذهب returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      اكسر;
    }
    حالة JSONB_TRUE: {
      إذا( sz ) اذهب returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      اكسر;
    }
    حالة JSONB_FALSE: {
      إذا( sz ) اذهب returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      اكسر;
    }
    حالة JSONB_INT5:
    حالة JSONB_INT: {
      sqlite3_int64 iRes = 0;
      حرف *z;
      صحيح bNeg = 0;
      حرف x;
      إذا( sz==0 ) اذهب returnfromblob_malformed;
      x = (حرف)pParse->aBlob[i+n];
      إذا( x=='-' ){
        إذا( sz<2 ) اذهب returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (ثابت حرف*)&pParse->aBlob[i+n], (صحيح)sz);
      إذا( z==0 ) اذهب returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      إذا( rc==0 ){
        إذا( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          مزدوج r;
          r = (مزدوج)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }وإلا{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }وإلا إذا( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }وإلا إذا( rc==1 ){
        اذهب returnfromblob_malformed;
      }وإلا{
        إذا( bNeg ){ n--; sz++; }
        اذهب to_double;
      }
      اكسر;
    }
    حالة JSONB_FLOAT5:
    حالة JSONB_FLOAT: {
      مزدوج r;
      حرف *z;
      إذا( sz==0 ) اذهب returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (ثابت حرف*)&pParse->aBlob[i+n], (صحيح)sz);
      إذا( z==0 ) اذهب returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      إذا( rc<=0 ) اذهب returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      اكسر;
    }
    حالة JSONB_TEXTRAW:
    حالة JSONB_TEXT: {
      sqlite3_result_text(pCtx, (حرف*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      اكسر;
    }
    حالة JSONB_TEXT5:
    حالة JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      ثابت حرف *z;
      حرف *zOut;
      u32 nOut = sz;
      z = (ثابت حرف*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      إذا( zOut==0 ) اذهب returnfromblob_oom;
      لكل(iIn=iOut=0; iIn<sz; iIn++){
        حرف c = z[iIn];
        إذا( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          إذا( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }وإلا إذا( v<=0x7f ){
            zOut[iOut++] = (حرف)v;
          }وإلا إذا( v<=0x7ff ){
            أكد( szEscape>=2 );
            zOut[iOut++] = (حرف)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }وإلا إذا( v<0x10000 ){
            أكد( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }وإلا إذا( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }وإلا{
            أكد( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }وإلا{
          zOut[iOut++] = c;
        }
      } /* end for() */
      أكد( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      اكسر;
    }
    حالة JSONB_ARRAY:
    حالة JSONB_OBJECT: {
      إذا( eMode==0 ){
        إذا( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }وإلا{
          eMode = 1;
        }
      }
      إذا( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }وإلا{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      اكسر;
    }
    افتراضي: {
      اذهب returnfromblob_malformed;
    }
  }
  أرجع;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  أرجع;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  أرجع;
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
ساكن صحيح jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_قدر *pArg,
  JsonParse *pParse
){
  صحيح eType = sqlite3_قدر_type(pArg);
  ساكن u8 aNull[] = { 0x00 };
  memset(pParse, 0, حجم(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  بدل( eType ){
    افتراضي: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      أرجع 0;
    }
    حالة SQLITE_BLOB: {
      إذا( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        أرجع 1;
      }
      اكسر;
    }
    حالة SQLITE_TEXT: {
      ثابت حرف *zJson = (ثابت حرف*)sqlite3_قدر_text(pArg);
      صحيح nJson = sqlite3_قدر_bytes(pArg);
      إذا( zJson==0 ) أرجع 1;
      إذا( sqlite3_قدر_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (حرف*)zJson;
        pParse->nJson = nJson;
        إذا( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, حجم(pParse[0]));
          أرجع 1;
        }
      }وإلا{
        json_Blob_Append_گرہ(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      اكسر;
    }
    حالة SQLITE_FLOAT: {
      إذا( NEVER(sqlite3IsNaN(sqlite3_قدر_double(pArg))) ){
        json_Blob_Append_گرہ(pParse, JSONB_NULL, 0, 0);
      }وإلا{
        صحيح n = sqlite3_قدر_bytes(pArg);
        ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_text(pArg);
        إذا( z==0 ) أرجع 1;
        إذا( z[0]=='I' ){
          json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 5, "9e999");
        }وإلا إذا( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 6, "-9e999");
        }وإلا{
          json_Blob_Append_گرہ(pParse, JSONB_FLOAT, n, z);
        }
      }
      اكسر;
    }
    حالة SQLITE_INTEGER: {
      صحيح n = sqlite3_قدر_bytes(pArg);
      ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_text(pArg);
      إذا( z==0 ) أرجع 1;
      json_Blob_Append_گرہ(pParse, JSONB_INT, n, z);
      اكسر;
    }
  }
  إذا( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    أرجع 1;
  }وإلا{
    أرجع 0;
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
ساكن حرف *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  ثابت حرف *zPath,        /* The path with the problem */
  صحيح rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  حرف *z_پیغام;
  إذا( rc==(صحيح)JSON_LOOKUP_NOTARRAY ){
    z_پیغام = sqlite3_mprintf("not an array element: %Q", zPath);
  }وإلا إذا( rc==(صحيح)JSON_LOOKUP_ERROR ){
    z_پیغام = sqlite3_mprintf("malformed JSON");
  }وإلا إذا( rc==(صحيح)JSON_LOOKUP_TOODEEP ){
    z_پیغام = sqlite3_mprintf("JSON path too deep");
  }وإلا{
    z_پیغام = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  إذا( ctx==0 ) أرجع z_پیغام;
  إذا( z_پیغام ){
    sqlite3_result_error(ctx, z_پیغام, -1);
    sqlite3_free(z_پیغام);
  }وإلا{
    sqlite3_result_error_nomem(ctx);
  }
  أرجع 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
ساكن فراغ json_داخل_کرو_Into_Blob(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv,
  صحيح eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  صحيح i;
  u32 rc = 0;
  ثابت حرف *zPath = 0;
  صحيح flgs;
  JsonParse *p;
  JsonParse ax;

  أكد( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  إذا( p==0 ) أرجع;
  لكل(i=1; i<argc-1; i+=2){
    إذا( sqlite3_قدر_type(argv[i])==SQLITE_NULL ) استمر;
    zPath = (ثابت حرف*)sqlite3_قدر_text(argv[i]);
    إذا( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      أرجع;
    }
    إذا( zPath[0]!='$' ) اذهب json_داخل_کرو_Into_Blob_patherror;
    إذا( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      أرجع;
    }
    إذا( zPath[1]==0 ){
      إذا( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }وإلا{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    إذا( rc==JSON_LOOKUP_NOTFOUND ) استمر;
    إذا( JSON_LOOKUP_ISERROR(rc) ) اذهب json_داخل_کرو_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  أرجع;

json_داخل_کرو_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  أرجع;
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
ساكن صحيح jsonArgIsJsonb(sqlite3_قدر *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  إذا( sqlite3_قدر_type(pArg)!=SQLITE_BLOB ) أرجع 0;
  p->aBlob = (u8*)sqlite3_قدر_blob(pArg);
  p->nBlob = (u32)sqlite3_قدر_bytes(pArg);
  إذا( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    أرجع 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  أرجع 0;
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
ساكن JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_قدر *pArg,
  u32 flgs
){
  صحيح eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  أكد( ctx!=0 );
  eType = sqlite3_قدر_type(pArg);
  إذا( eType==SQLITE_NULL ){
    أرجع 0;
  }
  إذا( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_تلاش(ctx, pArg);
  }
  إذا( pFromCache ){
    pFromCache->nJPRef++;
    إذا( (flgs & JSON_EDITABLE)==0 ){
      أرجع pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, حجم(*p));
  إذا( p==0 ) اذهب json_pfa_oom;
  memset(p, 0, حجم(*p));
  p->db = db;
  p->nJPRef = 1;
  إذا( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    إذا( p->aBlob==0 ) اذهب json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    أرجع p;
  }
  إذا( eType==SQLITE_BLOB ){
    إذا( jsonArgIsJsonb(pArg,p) ){
      إذا( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        اذهب json_pfa_oom;
      }
      أرجع p;
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
  p->zJson = (حرف*)sqlite3_قدر_text(pArg);
  p->nJson = sqlite3_قدر_bytes(pArg);
  إذا( db->mallocFailed ) اذهب json_pfa_oom;
  إذا( p->nJson==0 ) اذهب json_pfa_malformed;
  أكد( p->zJson!=0 );
  إذا( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    إذا( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      أرجع p;
    }وإلا{
      jsonParseFree(p);
      أرجع 0;
    }
  }وإلا{
    صحيح isRCStr = sqlite3_قدر_Is_Of_Class(pArg, sqlite3RCStrUnref);
    صحيح rc;
    إذا( !isRCStr ){
      حرف *zNew = sqlite3RCStrNew( p->nJson );
      إذا( zNew==0 ) اذهب json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }وإلا{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_داخل_کرو(ctx, p);
    إذا( rc==SQLITE_NOMEM ) اذهب json_pfa_oom;
    إذا( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      اذهب rebuild_from_cache;
    }
  }
  أرجع p;

json_pfa_malformed:
  إذا( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    أرجع p;
  }وإلا{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    أرجع 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  أرجع 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
ساكن فراغ jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  صحيح flgs;
  إذا( p->oom ){
    sqlite3_result_error_nomem(ctx);
    أرجع;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  إذا( flgs & JSON_BLOB ){
    إذا( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }وإلا{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }وإلا{
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

#إذا SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
ساكن فراغ jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  صحيح nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  طالما( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    صحيح showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    إذا( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    إذا( nn==0 ) nn = 1;
    إذا( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    لكل(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    إذا( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      استمر;
    }
    pParse->nBlob = savedNBlob;
    إذا( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      إذا( iEnd>pParse->nBlob ){
        إذا( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }وإلا{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    بدل( x ){
      حالة JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); اكسر;
      حالة JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); اكسر;
      حالة JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); اكسر;
      حالة JSONB_INT:      sqlite3_str_appendall(pOut,"int"); اكسر;
      حالة JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); اكسر;
      حالة JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); اكسر;
      حالة JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); اكسر;
      حالة JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); اكسر;
      حالة JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); اكسر;
      حالة JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); اكسر;
      حالة JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); اكسر;
      حالة JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        اكسر;
      }
      حالة JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        اكسر;
      }
      افتراضي: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        اكسر;
      }
    }
    إذا( showContent ){
      إذا( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }وإلا{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        لكل(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          إذا( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (حرف*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
ساكن فراغ jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  حرف zBuf[1000];
  إذا( pParse==0 ){
    printf("NULL pointer\n");
    أرجع;
  }وإلا{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    إذا( pParse->nBlob==0 ) أرجع;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, حجم(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_قدر(&out));
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
ساكن فراغ jsonParseFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  أكد( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( p==0 ) أرجع;
  إذا( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }وإلا{
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
ساكن فراغ jsonQuoteFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_قدر(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
ساكن فراغ jsonArrayFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  صحيح i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  لكل(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_قدر(&jx, argv[i]);
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
ساكن فراغ jsonArrayLengthFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( p==0 ) أرجع;
  إذا( argc==2 ){
    ثابت حرف *zPath = (ثابت حرف*)sqlite3_قدر_text(argv[1]);
    إذا( zPath==0 ){
      jsonParseFree(p);
      أرجع;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    إذا( JSON_LOOKUP_ISERROR(i) ){
      إذا( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }وإلا{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }وإلا{
    i = 0;
  }
  إذا( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_گنتی(p, i);
  }
  إذا( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
ساكن صحيح jsonAllAlphanum(ثابت حرف *z, صحيح n){
  صحيح i;
  لكل(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  أرجع i==n;
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
ساكن فراغ jsonExtractFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonParse *p = 0;      /* The parse */
  صحيح flags;             /* Flags associated with the function */
  صحيح i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  إذا( argc<2 ) أرجع;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( p==0 ) أرجع;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  إذا( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  لكل(i=1; i<argc; i++){
    /* With a single PATH argument */
    ثابت حرف *zPath = (ثابت حرف*)sqlite3_قدر_text(argv[i]);
    صحيح nPath;
    u32 j;
    إذا( zPath==0 ) اذهب json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    إذا( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }وإلا إذا( (flags & JSON_ABPATH) ){
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
      إذا( sqlite3_قدر_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        إذا( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }وإلا إذا( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }وإلا إذا( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }وإلا{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }وإلا{
      jsonBadPathError(ctx, zPath, 0);
      اذهب json_extract_error;
    }
    إذا( j<p->nBlob ){
      إذا( argc==2 ){
        إذا( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          أكد( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }وإلا{
          jsonReturnFromBlob(p, j, ctx, 0);
          إذا( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }وإلا{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }وإلا إذا( j==JSON_LOOKUP_NOTFOUND ){
      إذا( argc==2 ){
        اذهب json_extract_error;  /* Return NULL if not found */
      }وإلا{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }وإلا{
      jsonBadPathError(ctx, zPath, j);
      اذهب json_extract_error;
    }
  }
  إذا( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    إذا( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  أرجع;
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
ساكن صحيح jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  ثابت JsonParse *pPatch, /* The PATCH */
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

  أكد( iTarget>=0 && iTarget<pTarget->nBlob );
  أكد( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  إذا( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    أرجع pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  إذا( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  إذا( NEVER(n==0) ) أرجع JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  إذا( NEVER(n==0) ) أرجع JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  طالما( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    إذا( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      أرجع JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    إذا( nPLabel==0 ) أرجع JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    إذا( iPValue>=iPEnd ) أرجع JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    إذا( nPValue==0 ) أرجع JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    إذا( iPCursor>iPEnd ) أرجع JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    طالما( iTCursor<iTEnd ){
      صحيح isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      إذا( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        أرجع JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      إذا( nTLabel==0 ) أرجع JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      إذا( iTValue>=iTEnd ) أرجع JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      إذا( nTValue==0 ) أرجع JSON_MERGE_BADTARGET;
      إذا( iTValue + nTValue + szTValue > iTEnd ) أرجع JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ثابت حرف*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (ثابت حرف*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      إذا( isEqual ) اكسر;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    إذا( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      إذا( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        إذا( NEVER(pTarget->oom) ) أرجع JSON_MERGE_OOM;
      }وإلا{
        /* Algorithm line 12 */
        صحيح rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        إذا( iDepth>=JSON_MAX_DEPTH ) أرجع JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        إذا( rc ) أرجع rc;
        pTarget->delta += savedDelta;
      }        
    }وإلا إذا( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      إذا( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        إذا( pTarget->oom ) أرجع JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }وإلا{
        صحيح rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        إذا( pTarget->oom ) أرجع JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        إذا( iDepth>=JSON_MAX_DEPTH ) أرجع JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        إذا( rc ) أرجع rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  إذا( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  أرجع pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
ساكن فراغ jsonPatchFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  صحيح rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  أكد( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  إذا( pTarget==0 ) أرجع;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  إذا( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    إذا( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }وإلا إذا( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }وإلا إذا( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }وإلا{
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
ساكن فراغ jsonObjectFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  صحيح i;
  JsonString jx;
  ثابت حرف *z;
  u32 n;

  إذا( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    أرجع;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  لكل(i=0; i<argc; i+=2){
    إذا( sqlite3_قدر_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      أرجع;
    }
    jsonAppendSeparator(&jx);
    z = (ثابت حرف*)sqlite3_قدر_text(argv[i]);
    n = sqlite3_قدر_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_قدر(&jx, argv[i+1]);
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
ساكن فراغ jsonRemoveFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;          /* The parse */
  ثابت حرف *zPath = 0; /* Path of element to be removed */
  صحيح i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  إذا( argc<1 ) أرجع;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  إذا( p==0 ) أرجع;
  لكل(i=1; i<argc; i++){
    zPath = (ثابت حرف*)sqlite3_قدر_text(argv[i]);
    إذا( zPath==0 ){
      اذهب json_remove_done;
    }
    إذا( zPath[0]!='$' ){
      اذهب json_remove_patherror;
    }
    إذا( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      اذهب json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    إذا( JSON_LOOKUP_ISERROR(rc) ){
      إذا( rc==JSON_LOOKUP_NOTFOUND ){
        استمر;  /* No-op */
      }وإلا{
        jsonBadPathError(ctx, zPath, rc);
      }
      اذهب json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  أرجع;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  أرجع;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
ساكن فراغ jsonReplaceFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  إذا( argc<1 ) أرجع;
  إذا( (argc&1)==0 ) {
    json_Wrong_عدد_Args(ctx, "replace");
    أرجع;
  }
  json_داخل_کرو_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
ساكن فراغ jsonSetFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  صحيح flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  صحيح eInsType = JSON_داخل_کرو_TYPE(flags);
  ساكن ثابت حرف *azInsType[] = { "insert", "set", "array_insert" };
  ساكن ثابت u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  إذا( argc<1 ) أرجع;
  أكد( eInsType>=0 && eInsType<=2 );
  إذا( (argc&1)==0 ) {
    json_Wrong_عدد_Args(ctx, azInsType[eInsType]);
    أرجع;
  }
  json_داخل_کرو_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
ساكن فراغ jsonTypeFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;          /* The parse */
  ثابت حرف *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( p==0 ) أرجع;
  إذا( argc==2 ){
    zPath = (ثابت حرف*)sqlite3_قدر_text(argv[1]);
    إذا( zPath==0 ) اذهب json_type_done;
    إذا( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      اذهب json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    إذا( JSON_LOOKUP_ISERROR(i) ){
      إذا( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }وإلا{
        jsonBadPathError(ctx, zPath, i);
      }
      اذهب json_type_done;
    }
  }وإلا{
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
ساكن فراغ jsonPrettyFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, حجم(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( x.pParse==0 ) أرجع;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  إذا( argc==1 || (x.zIndent = (ثابت حرف*)sqlite3_قدر_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }وإلا{
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
ساكن فراغ jsonValidFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  إذا( argc==2 ){
    i64 f = sqlite3_قدر_int64(argv[1]);
    إذا( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      أرجع;
    }
    flags = f & 0x0f;
  }
  بدل( sqlite3_قدر_type(argv[0]) ){
    حالة SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      أرجع;
    }
    حالة SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, حجم(py));
      إذا( jsonArgIsJsonb(argv[0], &py) ){
        إذا( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }وإلا إذا( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        اكسر;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    افتراضي: {
      JsonParse px;
      إذا( (flags & 0x3)==0 ) اكسر;
      memset(&px, 0, حجم(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      إذا( p ){
        إذا( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }وإلا إذا( p->nErr ){
          /* no-op */
        }وإلا إذا( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }وإلا{
        sqlite3_result_error_nomem(ctx);
      }
      اكسر;
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
ساكن فراغ jsonErrorFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, حجم(s));
  s.db = sqlite3_context_db_handle(ctx);
  إذا( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }وإلا{
    s.zJson = (حرف*)sqlite3_قدر_text(argv[0]);
    إذا( s.zJson==0 ) أرجع;  /* NULL input or OOM */
    s.nJson = sqlite3_قدر_bytes(argv[0]);
    إذا( jsonConvertTextToBlob(&s,0) ){
      إذا( s.oom ){
        iErrPos = -1;
      }وإلا{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        أكد( s.zJson!=0 );  /* Because s.oom is false */
        لكل(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          إذا( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  إذا( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }وإلا{
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
ساكن فراغ jsonArrayStep(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, حجم(*pStr));
  إذا( pStr ){
    إذا( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }وإلا إذا( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_قدر(pStr, argv[0]);
  }
}
ساكن فراغ jsonArrayCompute(sqlite3_context *ctx, صحيح isFinal){
  JsonString *pStr;
  صحيح flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  إذا( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    إذا( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      أرجع;
    }وإلا إذا( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      إذا( isFinal ){
        إذا( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }وإلا{
        jsonStringTrimOneChar(pStr);
      }
      أرجع;
    }وإلا إذا( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (صحيح)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }وإلا{
      sqlite3_result_text(ctx, pStr->zBuf, (صحيح)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }وإلا إذا( flags & JSON_BLOB ){
    ساكن ثابت u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }وإلا{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
ساكن فراغ json_Array_قدر(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
ساكن فراغ jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
ساكن فراغ jsonGroupInverse(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  unsigned صحيح i;
  صحيح inStr = 0;
  صحيح nNest = 0;
  حرف *z;
  حرف c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  إذا( NEVER(!pStr) ) أرجع;
  z = pStr->zBuf;
  لكل(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    إذا( c=='"' ){
      inStr = !inStr;
    }وإلا إذا( c=='\\' ){
      i++;
    }وإلا إذا( !inStr ){
      إذا( c=='{' || c=='[' ) nNest++;
      إذا( c=='}' || c==']' ) nNest--;
    }
  }
  إذا( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }وإلا{
    pStr->nUsed = 1;
  }
}
#وإلا
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
ساكن فراغ jsonObjectStep(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_قدر **argv
){
  JsonString *pStr;
  ثابت حرف *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, حجم(*pStr));
  إذا( pStr ){
    z = (ثابت حرف*)sqlite3_قدر_text(argv[0]);
    n = sqlite3Strlen30(z);
    إذا( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }وإلا إذا( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    إذا( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_قدر(pStr, argv[1]);
    }وإلا{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
ساكن فراغ jsonObjectCompute(sqlite3_context *ctx, صحيح isFinal){
  JsonString *pStr;
  صحيح flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  إذا( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    إذا( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      أرجع;
    }
    إذا( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      صحيح inStr = 0;
      إذا( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        إذا( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          أرجع;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      لكل(i=j=1; i<pStr->nUsed; i++){
        حرف c = pStr->zBuf[i];
        إذا( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }وإلا إذا( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }وإلا إذا( c=='@' && !inStr ){
          أكد( i+1<pStr->nUsed );
          إذا( pStr->zBuf[i+1]==',' ){
            i++;
          }وإلا إذا( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }وإلا{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    إذا( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      إذا( isFinal ){
        إذا( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }وإلا{
        jsonStringTrimOneChar(pOgStr);
      }
    }وإلا إذا( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (صحيح)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }وإلا{
      sqlite3_result_text(ctx, pStr->zBuf, (صحيح)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    إذا( pStr!=pOgStr ) jsonStringReset(pStr);
  }وإلا إذا( flags & JSON_BLOB ){
    ساكن ثابت unsigned حرف emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }وإلا{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
ساكن فراغ json_Object_قدر(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
ساكن فراغ jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
تعريفنوع بنية JsonParent JsonParent;
بنية JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 i_قدر;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_کلید;                  /* Key for JSONB_ARRAY */
};

تعريفنوع بنية JsonEachCursor JsonEachCursor;
بنية JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_جڑ;                 /* Size of the root path in bytes */
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
تعريفنوع بنية JsonEachConnection JsonEachConnection;
بنية JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
ساكن صحيح jsonEachConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  JsonEachConnection *pNew;
  صحيح rc;

/* Column numbers */
#define JEACH_کلید     0
#define JEACH_قدر   1
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
#define JEACH_جڑ    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  إذا( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, حجم(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    إذا( pNew==0 ) أرجع SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  أرجع rc;
}

/* destructor for json_each virtual table */
ساكن صحيح jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  أرجع SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
ساكن صحيح jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, حجم(*pCur));
  إذا( pCur==0 ) أرجع SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  أرجع SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
ساكن فراغ jsonEachCursorReset(JsonEachCursor *p){
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
ساكن صحيح jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  أرجع SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
ساكن صحيح jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  أرجع p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
ساكن صحيح jsonSkipLabel(JsonEachCursor *p){
  إذا( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    إذا( sz >= p->sParse.nBlob ) sz = p->i;
    أرجع sz;
  }وإلا{
    أرجع p->i;
  }
}

/*
** Append the path name for the current element.
*/
ساكن فراغ jsonAppendPathName(JsonEachCursor *p){
  أكد( p->nParent>0 );
  أكد( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  إذا( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_کلید);
  }وإلا{
    u32 n, sz = 0, k, i;
    ثابت حرف *z;
    صحيح needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (ثابت حرف*)&p->sParse.aBlob[k];
    إذا( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }وإلا{
      لكل(i=0; i<sz; i++){
        إذا( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          اكسر;
        }
      }
    }
    إذا( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }وإلا{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
ساكن صحيح json_Each_Malformed_داخلہ(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_پیغام);
  cur->pVtab->z_Err_پیغام = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  أرجع cur->pVtab->z_Err_پیغام ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
ساكن صحيح jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  صحيح rc = SQLITE_OK;
  إذا( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    إذا( n==0 )أرجع json_Each_Malformed_داخلہ(cur);
    إذا( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      إذا( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, حجم(JsonParent)*nNew);
        إذا( pNew==0 ) أرجع SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->i_قدر = i;
      pParent->iEnd = i + n + sz;
      pParent->i_کلید = -1;
      pParent->nPath = (u32)p->path.nUsed;
      إذا( p->eType && p->nParent ){
        jsonAppendPathName(p);
        إذا( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }وإلا{
      p->i = i + n + sz;
    }
    طالما( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    إذا( levelChange ){
      إذا( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->i_قدر;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }وإلا{
        p->eType = 0;
      }
    }
  }وإلا{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    إذا( n==0 )أرجع json_Each_Malformed_داخلہ(cur);
    p->i = i + n + sz;
  }
  إذا( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_کلید++;
  }
  p->iRowid++;
  أرجع rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
ساكن صحيح jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  حرف *z = p->path.zBuf;
  إذا( p->iRowid==0 && p->bRecursive && n>=2 ){
    طالما( ALWAYS(n>1) ){
      n--;
      إذا( z[n]=='[' || z[n]=='.' ){
        u32 x;
        حرف cSaved = z[n];
        z[n] = 0;
        أكد( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        إذا( !JSON_LOOKUP_ISERROR(x) ) اكسر;
      }
    }
  }
  أرجع n;
}

/* Return the value of a column */
ساكن صحيح jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحيح iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  بدل( iColumn ){
    حالة JEACH_کلید: {
      إذا( p->nParent==0 ){
        u32 n, j;
        إذا( p->n_جڑ==1 ) اكسر;
        j = jsonEachPathLength(p);
        n = p->n_جڑ - j;
        إذا( n==0 ){
          اكسر;
        }وإلا إذا( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }وإلا إذا( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }وإلا{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        اكسر;
      }
      إذا( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }وإلا{
        أكد( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].i_کلید);
      }
      اكسر;
    }
    حالة JEACH_قدر: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      إذا( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      اكسر;
    }
    حالة JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      اكسر;
    }
    حالة JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      إذا( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      اكسر;
    }
    حالة JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      اكسر;
    }
    حالة JEACH_PARENT: {
      إذا( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      اكسر;
    }
    حالة JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      إذا( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      اكسر;
    }
    حالة JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      اكسر;
    }
    افتراضي: {
      sqlite3_result_text(ctx, p->path.zBuf, p->n_جڑ, SQLITE_TRANSIENT);
      اكسر;
    }
    حالة JEACH_JSON: {
      إذا( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }وإلا{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      اكسر;
    }
  }
  أرجع SQLITE_OK;
}

/* Return the current rowid value */
ساكن صحيح jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  أرجع SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
ساكن صحيح jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  صحيح i;                     /* Loop counter or computed array index */
  صحيح aIdx[2];               /* Index of constraints for JSON and ROOT */
  صحيح unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  صحيح idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  ثابت بنية sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  أكد( JEACH_جڑ == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  لكل(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    صحيح iCol;
    صحيح iMask;
    إذا( pConstraint->iColumn < JEACH_JSON ) استمر;
    iCol = pConstraint->iColumn - JEACH_JSON;
    أكد( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    إذا( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }وإلا إذا( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  إذا( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  إذا( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    أرجع SQLITE_CONSTRAINT;
  }
  إذا( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_عدد = 0;
  }وإلا{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    إذا( aIdx[1]<0 ){
      pIdxInfo->idx_عدد = 1;  /* Only JSON supplied.  Plan 1 */
    }وإلا{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_عدد = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  أرجع SQLITE_OK;
}

/* Start a search on a new JSON string */
ساكن صحيح jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  صحيح idx_عدد, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ثابت حرف *z_جڑ = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  إذا( idx_عدد==0 ) أرجع SQLITE_OK;
  memset(&p->sParse, 0, حجم(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  إذا( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }وإلا{
    p->sParse.zJson = (حرف*)sqlite3_قدر_text(argv[0]);
    p->sParse.nJson = sqlite3_قدر_bytes(argv[0]);
    إذا( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      أرجع SQLITE_OK;
    }      
    إذا( jsonConvertTextToBlob(&p->sParse, 0) ){
      إذا( p->sParse.oom ){
        أرجع SQLITE_NOMEM;
      }
      أرجع json_Each_Malformed_داخلہ(cur);
    }
  }
  إذا( idx_عدد==3 ){
    z_جڑ = (ثابت حرف*)sqlite3_قدر_text(argv[1]);
    إذا( z_جڑ==0 ) أرجع SQLITE_OK;
    إذا( z_جڑ[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_پیغام);
      cur->pVtab->z_Err_پیغام = jsonBadPathError(0, z_جڑ, 0);
      jsonEachCursorReset(p);
      أرجع cur->pVtab->z_Err_پیغام ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_جڑ = sqlite3Strlen30(z_جڑ);
    إذا( z_جڑ[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }وإلا{
      i = jsonLookupStep(&p->sParse, 0, z_جڑ+1, 0);
      إذا( JSON_LOOKUP_ISERROR(i) ){
        إذا( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          أرجع SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_پیغام);
        cur->pVtab->z_Err_پیغام = jsonBadPathError(0, z_جڑ, 0);
        jsonEachCursorReset(p);
        أرجع cur->pVtab->z_Err_پیغام ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      إذا( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }وإلا{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_جڑ, p->n_جڑ);
  }وإلا{
    i = p->i = 0;
    p->eType = 0;
    p->n_جڑ = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  إذا( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, حجم(JsonParent));
    إذا( p->aParent==0 ) أرجع SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_کلید = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].i_قدر = i;
  }
  أرجع SQLITE_OK;
}

/* The methods of the json_each virtual table */
ساكن sqlite3_module jsonEachModule = {
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
فراغ sqlite3RegisterJsonFunctions(فراغ){
#ifndef SQLITE_OMIT_JSON
  ساكن FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_داخل_کرو, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_داخل_کرو,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_داخل_کرو,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_داخل_کرو,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#إذا SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_قدر, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_قدر, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_قدر, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_قدر, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_داخل_کرو_Builtin_Funcs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#إذا  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, ثابت حرف *zName){
  unsigned صحيح i;
  ساكن ثابت حرف *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  أكد( sqlite3HashFind(&db->aModule, zName)==0 );
  لكل(i=0; i<حجم(azModule)/حجم(azModule[0]); i++){
    إذا( sqlite3StrICmp(azModule[i],zName)==0 ){
      أرجع sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  أرجع 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
