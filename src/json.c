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
#شامل "sqliteInt.h"

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
ایستا ثابت نویسه * ثابت jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
ایستا ثابت نویسه jsonIsSpace[] = {
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
#define jsonIsspace(x) (jsonIsSpace[(unsigned نویسه)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
ایستا ثابت نویسه jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
ایستا ثابت نویسه jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
ایستا ثابت نویسه jsonIsOk[256] = {
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
تعریفنوع ساختار JsonCache JsonCache;
تعریفنوع ساختار JsonString JsonString;
تعریفنوع ساختار JsonParse JsonParse;

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
ساختار JsonCache {
  sqlite3 *db;                    /* Database connection */
  صحیح nUsed;                      /* Number of active entries in the cache */
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
ساختار JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  نویسه *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  نویسه zSpace[100];        /* Initial static space */
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

#define JSON_درج_TYPE(X)  (((X)&0xC)>>2)


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
ساختار JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  نویسه *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  صحیح nJson;         /* Length of the zJson string in bytes */
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
  صحیح delta;         /* Size change due to the edit */
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
#وگرنه
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
ایستا تهی jsonReturnStringAsBlob(JsonString*);
ایستا صحیح jsonArgIsJsonb(sqlite3_مقدار *pJson, JsonParse *p);
ایستا u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
ایستا تهی jsonReturnParse(sqlite3_context*,JsonParse*);
ایستا JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_مقدار*,u32);
ایستا تهی jsonParseFree(JsonParse*);
ایستا u32 jsonbPayloadSize(ثابت JsonParse*, u32, u32*);
ایستا u32 jsonUnescapeOneChar(ثابت نویسه*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
ایستا تهی jsonCacheDelete(JsonCache *p){
  صحیح i;
  برای(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
ایستا تهی jsonCacheDeleteGeneric(تهی *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
ایستا صحیح json_Cache_درج(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  أكد( pParse->zJson!=0 );
  أكد( pParse->bJsonIsRCStr );
  أكد( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  اگر( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, اندازه(*p));
    اگر( p==0 ) بازگشت SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    اگر( p==0 ) بازگشت SQLITE_NOMEM;
  }
  اگر( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*اندازه(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  أكد( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  بازگشت SQLITE_OK;
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
ایستا JsonParse *json_Cache_جستجو(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_مقدار *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  صحیح i;
  ثابت نویسه *zJson;
  صحیح nJson;

  اگر( sqlite3_مقدار_type(pArg)!=SQLITE_TEXT ){
    بازگشت 0;
  }
  zJson = (ثابت نویسه*)sqlite3_مقدار_text(pArg);
  اگر( zJson==0 ) بازگشت 0;
  nJson = sqlite3_مقدار_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  اگر( p==0 ){
    بازگشت 0;
  }
  برای(i=0; i<p->nUsed; i++){
    اگر( p->a[i]->zJson==zJson ) بشکن;
  }
  اگر( i>=p->nUsed ){
    برای(i=0; i<p->nUsed; i++){
      اگر( p->a[i]->nJson!=nJson ) ادامه;
      اگر( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) بشکن;
    }
  }
  اگر( i<p->nUsed ){
    اگر( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*اندازه(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    أكد( p->a[i]->delta==0 );
    بازگشت p->a[i];
  }وگرنه{
    بازگشت 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
ایستا تهی jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = اندازه(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
ایستا تهی jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
ایستا تهی jsonStringReset(JsonString *p){
  اگر( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
ایستا تهی jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  اگر( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
ایستا تهی jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  أكد( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
ایستا صحیح jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  نویسه *zNew;
  اگر( p->bStatic ){
    اگر( p->eErr ) بازگشت 1;
    zNew = sqlite3RCStrNew(nTotal);
    اگر( zNew==0 ){
      jsonStringOom(p);
      بازگشت SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }وگرنه{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    اگر( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      بازگشت SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  بازگشت SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
ایستا SQLITE_NOINLINE تهی jsonStringExpandAndAppend(
  JsonString *p,
  ثابت نویسه *zIn,
  u32 N
){
  أكد( N>0 );
  اگر( jsonStringGrow(p,N) ) بازگشت;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
ایستا تهی jsonAppendRaw(JsonString *p, ثابت نویسه *zIn, u32 N){
  اگر( N==0 ) بازگشت;
  اگر( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }وگرنه{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
ایستا تهی jsonAppendRawNZ(JsonString *p, ثابت نویسه *zIn, u32 N){
  أكد( N>0 );
  اگر( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }وگرنه{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
ایستا تهی jsonPrintf(صحیح N, JsonString *p, ثابت نویسه *zFormat, ...){
  va_list ap;
  اگر( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) بازگشت;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (صحیح)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
ایستا SQLITE_NOINLINE تهی jsonAppendCharExpand(JsonString *p, نویسه c){
  اگر( jsonStringGrow(p,1) ) بازگشت;
  p->zBuf[p->nUsed++] = c;
}
ایستا تهی jsonAppendChar(JsonString *p, نویسه c){
  اگر( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }وگرنه{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
ایستا تهی jsonStringTrimOneChar(JsonString *p){
  اگر( p->eErr==0 ){
    أكد( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
ایستا صحیح jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  بازگشت p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
ایستا تهی jsonAppendSeparator(JsonString *p){
  نویسه c;
  اگر( p->nUsed==0 ) بازگشت;
  c = p->zBuf[p->nUsed-1];
  اگر( c=='[' || c=='{' ) بازگشت;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
ایستا تهی jsonAppendControlChar(JsonString *p, u8 c){
  ایستا ثابت نویسه aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  أكد( اندازه(aSpecial)==32 );
  أكد( aSpecial['\b']=='b' );
  أكد( aSpecial['\f']=='f' );
  أكد( aSpecial['\n']=='n' );
  أكد( aSpecial['\r']=='r' );
  أكد( aSpecial['\t']=='t' );
  أكد( c>=0 && c<اندازه(aSpecial) );
  أكد( p->nUsed+7 <= p->nAlloc );
  اگر( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }وگرنه{
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
ایستا تهی jsonAppendString(JsonString *p, ثابت نویسه *zIn, u32 N){
  u32 k;
  u8 c;
  ثابت u8 *z = (ثابت u8*)zIn;
  اگر( z==0 ) بازگشت;
  اگر( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) بازگشت;
  p->zBuf[p->nUsed++] = '"';
  وقتی( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    وقتی( 1 /* Exit by break */ ){
      اگر( k+3>=N ){
        وقتی( k<N && jsonIsOk[z[k]] ){ k++; }
        بشکن;
      }
      اگر( !jsonIsOk[z[k]] ){
        بشکن;
      }
      اگر( !jsonIsOk[z[k+1]] ){
        k += 1;
        بشکن;
      }
      اگر( !jsonIsOk[z[k+2]] ){
        k += 2;
        بشکن;
      }
      اگر( !jsonIsOk[z[k+3]] ){
        k += 3;
        بشکن;
      }وگرنه{
        k += 4;
      }
    }
    اگر( k>=N ){
      اگر( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      بشکن;
    }
    اگر( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    اگر( c=='"' || c=='\\' ){
      اگر( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) بازگشت;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }وگرنه اگر( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }وگرنه{
      اگر( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) بازگشت;
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
ایستا تهی json_Append_Sql_مقدار(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_مقدار *p_مقدار          /* Value to append */
){
  انتخاب( sqlite3_مقدار_type(p_مقدار) ){
    حالت SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      بشکن;
    }
    حالت SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_مقدار_double(p_مقدار));
      بشکن;
    }
    حالت SQLITE_INTEGER: {
      ثابت نویسه *z = (ثابت نویسه*)sqlite3_مقدار_text(p_مقدار);
      u32 n = (u32)sqlite3_مقدار_bytes(p_مقدار);
      jsonAppendRaw(p, z, n);
      بشکن;
    }
    حالت SQLITE_TEXT: {
      ثابت نویسه *z = (ثابت نویسه*)sqlite3_مقدار_text(p_مقدار);
      u32 n = (u32)sqlite3_مقدار_bytes(p_مقدار);
      اگر( sqlite3_مقدار_subtype(p_مقدار)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }وگرنه{
        jsonAppendString(p, z, n);
      }
      بشکن;
    }
    پیشفرض: {
      JsonParse px;
      memset(&px, 0, اندازه(px));
      اگر( jsonArgIsJsonb(p_مقدار, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }وگرنه اگر( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      بشکن;
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
ایستا تهی jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  أكد( (pParse!=0)==(ctx!=0) );
  أكد( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  اگر( p->eErr==0 ){
    صحیح flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    اگر( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }وگرنه اگر( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }وگرنه{
      اگر( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        صحیح rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_درج(ctx, pParse);
        اگر( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          بازگشت;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }وگرنه اگر( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }وگرنه اگر( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }وگرنه اگر( p->eErr & JSTRING_MALFORMED ){
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
ایستا تهی jsonParseReset(JsonParse *pParse){
  أكد( pParse->nJPRef<=1 );
  اگر( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  اگر( pParse->nBlobAlloc ){
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
ایستا تهی jsonParseFree(JsonParse *pParse){
  اگر( pParse ){
    اگر( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }وگرنه{
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
ایستا u8 jsonHexToInt(صحیح h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  بازگشت (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
ایستا u32 jsonHexToInt4(ثابت نویسه *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  بازگشت v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
ایستا صحیح jsonIs2Hex(ثابت نویسه *z){
  بازگشت sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
ایستا صحیح jsonIs4Hex(ثابت نویسه *z){
  بازگشت jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
ایستا صحیح json5Whitespace(ثابت نویسه *zIn){
  صحیح n = 0;
  ثابت u8 *z = (u8*)zIn;
  وقتی( 1 /*exit by "goto whitespace_done"*/ ){
    انتخاب( z[n] ){
      حالت 0x09:
      حالت 0x0a:
      حالت 0x0b:
      حالت 0x0c:
      حالت 0x0d:
      حالت 0x20: {
        n++;
        بشکن;
      }
      حالت '/': {
        اگر( z[n+1]=='*' && z[n+2]!=0 ){
          صحیح j;
          برای(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            اگر( z[j]==0 ) برو whitespace_done;
          }
          n = j+1;
          بشکن;
        }وگرنه اگر( z[n+1]=='/' ){
          صحیح j;
          نویسه c;
          برای(j=n+2; (c = z[j])!=0; j++){
            اگر( c=='\n' || c=='\r' ) بشکن;
            اگر( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              بشکن;
            }
          }
          n = j;
          اگر( z[n] ) n++;
          بشکن;
        }
        برو whitespace_done;
      }
      حالت 0xc2: {
        اگر( z[n+1]==0xa0 ){
          n += 2;
          بشکن;
        }
        برو whitespace_done;
      }
      حالت 0xe1: {
        اگر( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          بشکن;
        }
        برو whitespace_done;
      }
      حالت 0xe2: {
        اگر( z[n+1]==0x80 ){
          u8 c = z[n+2];
          اگر( c<0x80 ) برو whitespace_done;
          اگر( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            بشکن;
          }
        }وگرنه اگر( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          بشکن;
        }
        برو whitespace_done;
      }
      حالت 0xe3: {
        اگر( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          بشکن;
        }
        برو whitespace_done;
      }
      حالت 0xef: {
        اگر( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          بشکن;
        }
        برو whitespace_done;
      }
      پیشفرض: {
        برو whitespace_done;
      }
    }
  }
  whitespace_done:
  بازگشت n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
ایستا ثابت ساختار NanInfName {
  نویسه c1;
  نویسه c2;
  نویسه n;
  نویسه eType;
  نویسه nRepl;
  نویسه *zMatch;
  نویسه *zRepl;
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
ایستا تهی json_Wrong_رقم_Args(
  sqlite3_context *pCtx,
  ثابت نویسه *zFuncName
){
  نویسه *z_پیام = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, z_پیام, -1);
  sqlite3_free(z_پیام);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
ایستا صحیح jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  أكد( N>pParse->nBlobAlloc );
  اگر( pParse->nBlobAlloc==0 ){
    t = 100;
  }وگرنه{
    t = pParse->nBlobAlloc*2;
  }
  اگر( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  اگر( aNew==0 ){ pParse->oom = 1; بازگشت 1; }
  أكد( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  بازگشت 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
ایستا صحیح jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 nSize;
  أكد( !pParse->bReadOnly );
  اگر( pParse->oom ) بازگشت 0;
  اگر( pParse->nBlobAlloc>0 ) بازگشت 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  اگر( jsonBlobExpand(pParse, nSize) ){
    بازگشت 0;
  }
  أكد( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  بازگشت 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
ایستا SQLITE_NOINLINE تهی jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  اگر( pParse->oom==0 ){
    أكد( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
ایستا تهی jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  اگر( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }وگرنه{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
ایستا تهی json_Blob_Append_گره(JsonParse*,u8,u64,ثابت تهی*);
ایستا SQLITE_NOINLINE تهی json_Blob_Expand_And_Append_گره(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ثابت تهی *aPayload
){
  اگر( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) بازگشت;
  json_Blob_Append_گره(pParse, eType, szPayload, aPayload);
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
ایستا تهی json_Blob_Append_گره(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  ثابت تهی *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  اگر( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_گره(pParse,eType,szPayload,aPayload);
    بازگشت;
  }
  أكد( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  اگر( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }وگرنه اگر( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }وگرنه اگر( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }وگرنه{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  اگر( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
ایستا صحیح jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  صحیح delta;
  اگر( pParse->oom ) بازگشت 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  اگر( szType<=11 ){
    nExtra = 0;
  }وگرنه اگر( szType==12 ){
    nExtra = 1;
  }وگرنه اگر( szType==13 ){
    nExtra = 2;
  }وگرنه اگر( szType==14 ){
    nExtra = 4;
  }وگرنه{
    nExtra = 8;
  }
  اگر( szPayload<=11 ){
    nNeeded = 0;
  }وگرنه اگر( szPayload<=0xff ){
    nNeeded = 1;
  }وگرنه اگر( szPayload<=0xffff ){
    nNeeded = 2;
  }وگرنه{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  اگر( delta ){
    u64 newSize = (u64)pParse->nBlob + delta;
    اگر( delta>0 ){
      اگر( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        بازگشت 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }وگرنه{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  اگر( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }وگرنه اگر( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }وگرنه اگر( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }وگرنه{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  بازگشت delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
ایستا صحیح jsonIs4HexB(ثابت نویسه *z, صحیح *pOp){
  اگر( z[0]!='u' ) بازگشت 0;
  اگر( !jsonIs4Hex(&z[1]) ) بازگشت 0;
  *pOp = JSONB_TEXTJ;
  بازگشت 1;
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
ایستا u32 jsonbValidityCheck(
  ثابت JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ثابت u8 *z;
  u8 x;
  اگر( iDepth>JSON_MAX_DEPTH ) بازگشت i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  اگر( NEVER(n==0) ) بازگشت i+1;          /* Checked by caller */
  اگر( NEVER(i+n+sz!=iEnd) ) بازگشت i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  انتخاب( x ){
    حالت JSONB_NULL:
    حالت JSONB_TRUE:
    حالت JSONB_FALSE: {
      بازگشت n+sz==1 ? 0 : i+1;
    }
    حالت JSONB_INT: {
      اگر( sz<1 ) بازگشت i+1;
      j = i+n;
      اگر( z[j]=='-' ){
        j++;
        اگر( sz<2 ) بازگشت i+1;
      }
      k = i+n+sz;
      وقتی( j<k ){
        اگر( sqlite3Isdigit(z[j]) ){
          j++;
        }وگرنه{
          بازگشت j+1;
        }
      }
      بازگشت 0;
    }
    حالت JSONB_INT5: {
      اگر( sz<3 ) بازگشت i+1;
      j = i+n;
      اگر( z[j]=='-' ){
        اگر( sz<4 ) بازگشت i+1;
        j++;
      }
      اگر( z[j]!='0' ) بازگشت i+1;
      اگر( z[j+1]!='x' && z[j+1]!='X' ) بازگشت j+2;
      j += 2;
      k = i+n+sz;
      وقتی( j<k ){
        اگر( sqlite3Isxdigit(z[j]) ){
          j++;
        }وگرنه{
          بازگشت j+1;
        }
      }
      بازگشت 0;
    }
    حالت JSONB_FLOAT:
    حالت JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      اگر( sz<2 ) بازگشت i+1;
      j = i+n;
      k = j+sz;
      اگر( z[j]=='-' ){
        j++;
        اگر( sz<3 ) بازگشت i+1;
      }
      اگر( z[j]=='.' ){
        اگر( x==JSONB_FLOAT ) بازگشت j+1;
        اگر( !sqlite3Isdigit(z[j+1]) ) بازگشت j+1;
        j += 2;
        seen = 1;
      }وگرنه اگر( z[j]=='0' && x==JSONB_FLOAT ){
        اگر( j+3>k ) بازگشت j+1;
        اگر( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) بازگشت j+1;
        j++;
      }
      برای(; j<k; j++){
        اگر( sqlite3Isdigit(z[j]) ) ادامه;
        اگر( z[j]=='.' ){
          اگر( seen>0 ) بازگشت j+1;
          اگر( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            بازگشت j+1;
          }
          seen = 1;
          ادامه;
        }
        اگر( z[j]=='e' || z[j]=='E' ){
          اگر( seen==2 ) بازگشت j+1;
          اگر( j==k-1 ) بازگشت j+1;
          اگر( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            اگر( j==k-1 ) بازگشت j+1;
          }
          seen = 2;
          ادامه;
        }
        بازگشت j+1;
      }
      اگر( seen==0 ) بازگشت i+1;
      بازگشت 0;
    }
    حالت JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      وقتی( j<k ){
        اگر( !jsonIsOk[z[j]] && z[j]!='\'' ) بازگشت j+1;
        j++;
      }
      بازگشت 0;
    }
    حالت JSONB_TEXTJ:
    حالت JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      وقتی( j<k ){
        اگر( !jsonIsOk[z[j]] && z[j]!='\'' ){
          اگر( z[j]=='"' ){
            اگر( x==JSONB_TEXTJ ) بازگشت j+1;
          }وگرنه اگر( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            اگر( x==JSONB_TEXTJ ) بازگشت j+1;
          }وگرنه اگر( NEVER(z[j]!='\\') || j+1>=k ){
            بازگشت j+1;
          }وگرنه اگر( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }وگرنه اگر( z[j+1]=='u' ){
            اگر( j+5>=k ) بازگشت j+1;
            اگر( !jsonIs4Hex((ثابت نویسه*)&z[j+2]) ) بازگشت j+1;
            j++;
          }وگرنه اگر( x!=JSONB_TEXT5 ){
            بازگشت j+1;
          }وگرنه{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((ثابت نویسه*)&z[j], k-j, &c);
            اگر( c==JSON_INVALID_CHAR ) بازگشت j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      بازگشت 0;
    }
    حالت JSONB_TEXTRAW: {
      بازگشت 0;
    }
    حالت JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      وقتی( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        اگر( n==0 ) بازگشت j+1;
        اگر( j+n+sz>k ) بازگشت j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        اگر( sub ) بازگشت sub;
        j += n + sz;
      }
      أكد( j==k );
      بازگشت 0;
    }
    حالت JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      وقتی( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        اگر( n==0 ) بازگشت j+1;
        اگر( j+n+sz>k ) بازگشت j+1;
        اگر( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          اگر( x<JSONB_TEXT || x>JSONB_TEXTRAW ) بازگشت j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        اگر( sub ) بازگشت sub;
        cnt++;
        j += n + sz;
      }
      أكد( j==k );
      اگر( (cnt & 1)!=0 ) بازگشت j+1;
      بازگشت 0;
    }
    پیشفرض: {
      بازگشت i+1;
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
ایستا صحیح jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  نویسه c;
  u32 j;
  u32 iThis, iStart;
  صحیح x;
  u8 t;
  ثابت نویسه *z = pParse->zJson;
json_parse_restart:
  انتخاب( (u8)z[i] ){
  حالت '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_گره(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    اگر( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      بازگشت -1;
    }
    iStart = pParse->nBlob;
    برای(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      اگر( x<=0 ){
        صحیح op;
        اگر( x==(-2) ){
          j = pParse->iErr;
          اگر( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          بشکن;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        اگر( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          صحیح k = j+1;
          وقتی( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          أكد( iBlob==pParse->nBlob );
          json_Blob_Append_گره(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }وگرنه{
          اگر( x!=-1 ) pParse->iErr = j;
          بازگشت -1;
        }
      }
      اگر( pParse->oom ) بازگشت -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      اگر( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        بازگشت -1;
      }
      j = x;
      اگر( z[j]==':' ){
        j++;
      }وگرنه{
        اگر( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          انجامبده{ j++; }وقتی( jsonIsspace(z[j]) );
          اگر( z[j]==':' ){
            j++;
            برو parse_object_مقدار;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        اگر( x!=(-5) ){
          اگر( x!=(-1) ) pParse->iErr = j;
          بازگشت -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_مقدار:
      x = jsonTranslateTextToBlob(pParse, j);
      اگر( x<=0 ){
        اگر( x!=(-1) ) pParse->iErr = j;
        بازگشت -1;
      }
      j = x;
      اگر( z[j]==',' ){
        ادامه;
      }وگرنه اگر( z[j]=='}' ){
        بشکن;
      }وگرنه{
        اگر( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          اگر( z[j]==',' ){
            ادامه;
          }وگرنه اگر( z[j]=='}' ){
            بشکن;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        اگر( x==(-4) ){
          j = pParse->iErr;
          ادامه;
        }
        اگر( x==(-2) ){
          j = pParse->iErr;
          بشکن;
        }
      }
      pParse->iErr = j;
      بازگشت -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    بازگشت j+1;
  }
  حالت '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    أكد( i<=(u32)pParse->nJson );
    json_Blob_Append_گره(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    اگر( pParse->oom ) بازگشت -1;
    اگر( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      بازگشت -1;
    }
    برای(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      اگر( x<=0 ){
        اگر( x==(-3) ){
          j = pParse->iErr;
          اگر( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          بشکن;
        }
        اگر( x!=(-1) ) pParse->iErr = j;
        بازگشت -1;
      }
      j = x;
      اگر( z[j]==',' ){
        ادامه;
      }وگرنه اگر( z[j]==']' ){
        بشکن;
      }وگرنه{
        اگر( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          اگر( z[j]==',' ){
            ادامه;
          }وگرنه اگر( z[j]==']' ){
            بشکن;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        اگر( x==(-4) ){
          j = pParse->iErr;
          ادامه;
        }
        اگر( x==(-3) ){
          j = pParse->iErr;
          بشکن;
        }
      }
      pParse->iErr = j;
      بازگشت -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    بازگشت j+1;
  }
  حالت '\'': {
    u8 opcode;
    نویسه cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    برو parse_string;
  حالت '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    وقتی( 1 /*exit-by-break*/ ){
      اگر( jsonIsOk[(u8)z[j]] ){
        اگر( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }وگرنه اگر( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }وگرنه{
          j += 3;
          ادامه;
        }
      }
      c = z[j];
      اگر( c==cDelim ){
        بشکن;
      }وگرنه اگر( c=='\\' ){
        c = z[++j];
        اگر( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          اگر( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }وگرنه اگر( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#وگرنه
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }وگرنه اگر( c=='\r' ){
          اگر( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }وگرنه{
          pParse->iErr = j;
          بازگشت -1;
        }
      }وگرنه اگر( c<=0x1f ){
        اگر( c==0 ){
          pParse->iErr = j;
          بازگشت -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }وگرنه اگر( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_گره(pParse, opcode, j-1-i, &z[i+1]);
    بازگشت j+1;
  }
  حالت 't': {
    اگر( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      بازگشت i+4;
    }
    pParse->iErr = i;
    بازگشت -1;
  }
  حالت 'f': {
    اگر( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      بازگشت i+5;
    }
    pParse->iErr = i;
    بازگشت -1;
  }
  حالت '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    برو parse_عدد;
  حالت '.':
    اگر( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      برو parse_عدد_2;
    }
    pParse->iErr = i;
    بازگشت -1;
  حالت '-':
  حالت '0':
  حالت '1':
  حالت '2':
  حالت '3':
  حالت '4':
  حالت '5':
  حالت '6':
  حالت '7':
  حالت '8':
  حالت '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_عدد:
    seenE = 0;
    أكد( '-' < '0' );
    أكد( '+' < '0' );
    أكد( '.' < '0' );
    c = z[i];

    اگر( c<='0' ){
      اگر( c=='0' ){
        اگر( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          أكد( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          برای(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          برو parse_عدد_finish;
        }وگرنه اگر( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          بازگشت -1;
        }
      }وگرنه{
        اگر( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          اگر( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            اگر( z[i]=='-' ){
              json_Blob_Append_گره(pParse, JSONB_FLOAT, 6, "-9e999");
            }وگرنه{
              json_Blob_Append_گره(pParse, JSONB_FLOAT, 5, "9e999");
            }
            بازگشت i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          اگر( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            برو parse_عدد_2;
          }
          pParse->iErr = i;
          بازگشت -1;
        }
        اگر( z[i+1]=='0' ){
          اگر( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            بازگشت -1;
          }وگرنه اگر( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            برای(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            برو parse_عدد_finish;
          }
        }
      }
    }

  parse_عدد_2:
    برای(j=i+1;; j++){
      c = z[j];
      اگر( sqlite3Isdigit(c) ) ادامه;
      اگر( c=='.' ){
        اگر( (t & 0x02)!=0 ){
          pParse->iErr = j;
          بازگشت -1;
        }
        t |= 0x02;
        ادامه;
      }
      اگر( c=='e' || c=='E' ){
        اگر( z[j-1]<'0' ){
          اگر( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }وگرنه{
            pParse->iErr = j;
            بازگشت -1;
          }
        }
        اگر( seenE ){
          pParse->iErr = j;
          بازگشت -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        اگر( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        اگر( c<'0' || c>'9' ){
          pParse->iErr = j;
          بازگشت -1;
        }
        ادامه;
      }
      بشکن;
    }
    اگر( z[j-1]<'0' ){
      اگر( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }وگرنه{
        pParse->iErr = j;
        بازگشت -1;
      }
    }
  parse_عدد_finish:
    أكد( JSONB_INT+0x01==JSONB_INT5 );
    أكد( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    أكد( JSONB_INT+0x02==JSONB_FLOAT );
    اگر( z[i]=='+' ) i++;
    json_Blob_Append_گره(pParse, JSONB_INT+t, j-i, &z[i]);
    بازگشت j;
  }
  حالت '}': {
    pParse->iErr = i;
    بازگشت -2;  /* End of {...} */
  }
  حالت ']': {
    pParse->iErr = i;
    بازگشت -3;  /* End of [...] */
  }
  حالت ',': {
    pParse->iErr = i;
    بازگشت -4;  /* List separator */
  }
  حالت ':': {
    pParse->iErr = i;
    بازگشت -5;  /* Object label/value separator */
  }
  حالت 0: {
    بازگشت 0;   /* End of file */
  }
  حالت 0x09:
  حالت 0x0a:
  حالت 0x0d:
  حالت 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    برو json_parse_restart;
  }
  حالت 0x0b:
  حالت 0x0c:
  حالت '/':
  حالت 0xc2:
  حالت 0xe1:
  حالت 0xe2:
  حالت 0xe3:
  حالت 0xef: {
    j = json5Whitespace(&z[i]);
    اگر( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      برو json_parse_restart;
    }
    pParse->iErr = i;
    بازگشت -1;
  }
  حالت 'n': {
    اگر( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      بازگشت i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  پیشفرض: {
    u32 k;
    صحیح nn;
    c = z[i];
    برای(k=0; k<اندازه(aNanInfName)/اندازه(aNanInfName[0]); k++){
      اگر( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) ادامه;
      nn = aNanInfName[k].n;
      اگر( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        ادامه;
      }
      اگر( sqlite3Isalnum(z[i+nn]) ) ادامه;
      اگر( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_گره(pParse, JSONB_FLOAT, 5, "9e999");
      }وگرنه{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      بازگشت i + nn;
    }
    pParse->iErr = i;
    بازگشت -1;  /* Syntax error */
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
ایستا صحیح jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  صحیح i;
  ثابت نویسه *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  اگر( pParse->oom ) i = -1;
  اگر( i>0 ){
#ifdef SQLITE_DEBUG
    أكد( pParse->iDepth==0 );
    اگر( sqlite3Config.bJsonSelfcheck ){
      أكد( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    وقتی( jsonIsspace(zJson[i]) ) i++;
    اگر( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      اگر( zJson[i] ){
        اگر( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        بازگشت 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  اگر( i<=0 ){
    اگر( pCtx!=0 ){
      اگر( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }وگرنه{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    بازگشت 1;
  }
  بازگشت 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
ایستا تهی jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  أكد( pStr->eErr==0 );
  memset(&px, 0, اندازه(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (تهی)jsonTranslateTextToBlob(&px, 0);
  اگر( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }وگرنه{
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
ایستا u32 jsonbPayloadSizeWide(
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
  اگر( x==13 ){
    اگر( i+2>=pParse->nBlob ){
      *pSz = 0;
      بازگشت 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }وگرنه اگر( x==14 ){
    اگر( i+4>=pParse->nBlob ){
      *pSz = 0;
      بازگشت 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }وگرنه{
    اگر( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      بازگشت 0;
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
  اگر( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    بازگشت 0;
  }
  *pSz = sz;
  بازگشت n;
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
ایستا u32 jsonbPayloadSize(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  اگر( i>=pParse->nBlob ){
    *pSz = 0;
    بازگشت 0;
  }وگرنه اگر( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }وگرنه اگر( x==12 ){
    اگر( i+1>=pParse->nBlob ){
      *pSz = 0;
      بازگشت 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }وگرنه{
    بازگشت jsonbPayloadSizeWide(pParse, i, pSz, x);
  }
  اگر( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    بازگشت 0;
  }
  *pSz = sz;
  بازگشت n;
}

#اگر SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
ایستا SQLITE_INLINE u32 jsonbPayloadSizeInline(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  اگر( i>=pParse->nBlob ){
    *pSz = 0;
    بازگشت 0;
  }وگرنه اگر( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }وگرنه اگر( x==12 ){
    اگر( i+1>=pParse->nBlob ){
      *pSz = 0;
      بازگشت 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }وگرنه اگر( x==13 ){
    اگر( i+2>=pParse->nBlob ){
      *pSz = 0;
      بازگشت 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }وگرنه اگر( x==14 ){
    اگر( i+4>=pParse->nBlob ){
      *pSz = 0;
      بازگشت 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }وگرنه{
    اگر( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      بازگشت 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  اگر( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    بازگشت 0;
  }
  *pSz = sz;
  بازگشت n;
}
#وگرنه /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif


#اگر SQLITE_USES_INLINE
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
ایستا SQLITE_INLINE u32 jsonbPayloadSizeSemiInline(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  أكد( i<pParse->nBlob );
  اگر( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }وگرنه اگر( x==12 ){
    اگر( i+1>=pParse->nBlob ){
      *pSz = 0;
      بازگشت 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }وگرنه{
    بازگشت jsonbPayloadSizeWide(pParse,i,pSz,x);
  }
  اگر( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    بازگشت 0;
  }
  *pSz = sz;
  بازگشت n;
}
#وگرنه /* if !SQLITE_USES_INLINE */
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
ایستا u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSizeInline(pParse, i, &sz);
  اگر( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    بازگشت pParse->nBlob+1;
  }
  انتخاب( pParse->aBlob[i] & 0x0f ){
    حالت JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      بازگشت i+1;
    }
    حالت JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      بازگشت i+1;
    }
    حالت JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      بازگشت i+1;
    }
    حالت JSONB_INT:
    حالت JSONB_FLOAT: {
      اگر( sz==0 ) برو malformed_jsonb;
      jsonAppendRaw(pOut, (ثابت نویسه*)&pParse->aBlob[i+n], sz);
      بشکن;
    }
    حالت JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ثابت نویسه *zIn = (ثابت نویسه*)&pParse->aBlob[i+n];
      صحیح bOverflow = 0;
      اگر( sz==0 ) برو malformed_jsonb;
      اگر( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }وگرنه اگر( zIn[0]=='+' ){
        k++;
      }
      برای(; k<sz; k++){
        اگر( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          بشکن;
        }وگرنه اگر( (u>>60)!=0 ){
          bOverflow = 1;
        }وگرنه{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      بشکن;
    }
    حالت JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ثابت نویسه *zIn = (ثابت نویسه*)&pParse->aBlob[i+n];
      اگر( sz==0 ) برو malformed_jsonb;
      اگر( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        اگر( sz<=1 ) برو malformed_jsonb;
        k = 1;
      }
      اگر( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      برای(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        اگر( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      بشکن;
    }
    حالت JSONB_TEXT:
    حالت JSONB_TEXTJ: {
      اگر( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(ثابت نویسه*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      بشکن;
    }
    حالت JSONB_TEXT5: {
      ثابت نویسه *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (ثابت نویسه*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      وقتی( sz2>0 ){
        برای(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        اگر( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          اگر( k>=sz2 ){
            بشکن;
          }
          zIn += k;
          sz2 -= k;
        }
        اگر( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          ادامه;
        }
        اگر( zIn[0]<=0x1f ){
          اگر( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) بشکن;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          ادامه;
        }
        أكد( zIn[0]=='\\' );
        أكد( sz2>=1 );
        اگر( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          بشکن;
        }
        انتخاب( (u8)zIn[1] ){
          حالت '\'':
            jsonAppendChar(pOut, '\'');
            بشکن;
          حالت 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            بشکن;
          حالت 'x':
            اگر( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              بشکن;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            بشکن;
          حالت '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            بشکن;
          حالت '\r':
            اگر( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            بشکن;
          حالت '\n':
            بشکن;
          حالت 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            اگر( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              بشکن;
            }
            zIn += 2;
            sz2 -= 2;
            بشکن;
          پیشفرض:
            jsonAppendRawNZ(pOut, zIn, 2);
            بشکن;
        }
        أكد( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      بشکن;
    }
    حالت JSONB_TEXTRAW: {
      jsonAppendString(pOut, (ثابت نویسه*)&pParse->aBlob[i+n], sz);
      بشکن;
    }
    حالت JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      اگر( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      وقتی( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      اگر( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      اگر( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      بشکن;
    }
    حالت JSONB_OBJECT: {
      صحیح x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      اگر( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      وقتی( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      اگر( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      اگر( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      بشکن;
    }

    پیشفرض: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      بشکن;
    }
  }
  بازگشت i+n+sz;
}

/* Context for recursion of json_pretty()
*/
تعریفنوع ساختار JsonPretty JsonPretty;
ساختار JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  ثابت نویسه *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
ایستا تهی jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  برای(jj=0; jj<pPretty->nIndent; jj++){
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
ایستا u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  اگر( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    بازگشت pParse->nBlob+1;
  }
  انتخاب( pParse->aBlob[i] & 0x0f ){
    حالت JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      اگر( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        اگر( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        وقتی( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          اگر( j>=iEnd ){
            اگر( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            بشکن;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      بشکن;
    }
    حالت JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      اگر( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        اگر( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        وقتی( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          اگر( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            بشکن;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          اگر( j>=iEnd ) بشکن;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      بشکن;
    }
    پیشفرض: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      بشکن;
    }
  }
  بازگشت i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
ایستا u32 jsonb_Array_شمار(JsonParse *pParse, u32 i_ریشه){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, i_ریشه, &sz);
  iEnd = i_ریشه+n+sz;
  برای(i=i_ریشه+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSizeSemiInline(pParse, i, &sz);
  }
  بازگشت k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
ایستا تهی jsonAfterEditSizeAdjust(JsonParse *pParse, u32 i_ریشه){
  u32 sz = 0;
  u32 nBlob;
  أكد( pParse->delta!=0 );
  أكد( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (تهی)jsonbPayloadSize(pParse, i_ریشه, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, i_ریشه, sz);
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
ایستا صحیح jsonBlobOverwrite(
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
  ایستا ثابت u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  اگر( (aIns[0]&0x0f)<=2 ) بازگشت 0;    /* Cannot enlarge NULL, true, false */
  انتخاب( aIns[0]>>4 ){
    پیشفرض: {                         /* aIns[] header size 1 */
      اگر( ((1<<d)&0x116)==0 ) بازگشت 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      بشکن;
    }
    حالت 12: {                         /* aIns[] header size is 2 */
      اگر( ((1<<d)&0x8a)==0) بازگشت 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      بشکن;
    }
    حالت 13: {                         /* aIns[] header size is 3 */
      اگر( d!=2 && d!=6 ) بازگشت 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      بشکن;
    }
    حالت 14: {                         /* aIns[] header size is 5 */
      اگر( d!=4 ) بازگشت 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      بشکن;
    }
    حالت 15: {                         /* aIns[] header size is 9 */
      بازگشت 0;                          /* No solution */
    }
  }
  أكد( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  وقتی( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    اگر( i==1 ) بشکن;
    szPayload >>= 8;
  }
  أكد( (szPayload>>8)==0 );
  بازگشت 1;
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
ایستا تهی jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  ثابت u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  أكد( pParse->nBlob >= (u64)iDel + (u64)nDel );
  اگر( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (صحیح)-d)
  ){
    بازگشت;
  }
  اگر( d!=0 ){
    اگر( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      اگر( pParse->oom ) بازگشت;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  اگر( nIns && aIns ){
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
ایستا u32 jsonBytesToBypass(ثابت نویسه *z, u32 n){
  u32 i = 0;
  وقتی( i+1<n ){
    اگر( z[i]!='\\' ) بازگشت i;
    اگر( z[i+1]=='\n' ){
      i += 2;
      ادامه;
    }
    اگر( z[i+1]=='\r' ){
      اگر( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }وگرنه{
        i += 2;
      }
      ادامه;
    }
    اگر( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      ادامه;
    }
    بشکن;
  }
  بازگشت i;
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
ایستا u32 jsonUnescapeOneChar(ثابت نویسه *z, u32 n, u32 *piOut){
  أكد( n>0 );
  أكد( z[0]=='\\' );
  اگر( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    بازگشت n;
  }
  انتخاب( (u8)z[1] ){
    حالت 'u': {
      u32 v, vlo;
      اگر( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        بازگشت n;
      }
      v = jsonHexToInt4(&z[2]);
      اگر( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        بازگشت 12;
      }وگرنه{
        *piOut = v;
        بازگشت 6;
      }
    }
    حالت 'b': {   *piOut = '\b';  بازگشت 2; }
    حالت 'f': {   *piOut = '\f';  بازگشت 2; }
    حالت 'n': {   *piOut = '\n';  بازگشت 2; }
    حالت 'r': {   *piOut = '\r';  بازگشت 2; }
    حالت 't': {   *piOut = '\t';  بازگشت 2; }
    حالت 'v': {   *piOut = '\v';  بازگشت 2; }
    حالت '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#وگرنه
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      بازگشت 2;
    }
    حالت '\'':
    حالت '"':
    حالت '/':
    حالت '\\':{   *piOut = z[1];  بازگشت 2; }
    حالت 'x': {
      اگر( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        بازگشت n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      بازگشت 4;
    }
    حالت 0xe2:
    حالت '\r':
    حالت '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      اگر( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        بازگشت n;
      }وگرنه اگر( nSkip==n ){
        *piOut = 0;
        بازگشت n;
      }وگرنه اگر( z[nSkip]=='\\' ){
        بازگشت nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }وگرنه{
        صحیح sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        بازگشت nSkip + sz;
      }
    }
    پیشفرض: {
      *piOut = JSON_INVALID_CHAR;
      بازگشت 2;
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
ایستا SQLITE_NOINLINE صحیح jsonLabelCompareEscaped(
  ثابت نویسه *z_چپ,          /* The left label */
  u32 n_چپ,                  /* Size of the left label in bytes */
  صحیح raw_چپ,                /* True if zLeft contains no escapes */
  ثابت نویسه *z_راست,         /* The right label */
  u32 n_راست,                 /* Size of the right label in bytes */
  صحیح raw_راست                /* True if zRight is escape-free */
){
  u32 c_چپ, c_راست;
  أكد( raw_چپ==0 || raw_راست==0 );
  وقتی( 1 /*exit-by-return*/ ){
    اگر( n_چپ==0 ){
      c_چپ = 0;
    }وگرنه اگر( raw_چپ || z_چپ[0]!='\\' ){
      c_چپ = ((u8*)z_چپ)[0];
      اگر( c_چپ>=0xc0 ){
        صحیح sz = sqlite3Utf8ReadLimited((u8*)z_چپ, n_چپ, &c_چپ);
        z_چپ += sz;
        n_چپ -= sz;
      }وگرنه{
        z_چپ++;
        n_چپ--;
      }
    }وگرنه{
      u32 n = jsonUnescapeOneChar(z_چپ, n_چپ, &c_چپ);
      z_چپ += n;
      أكد( n<=n_چپ );
      n_چپ -= n;
    }
    اگر( n_راست==0 ){
      c_راست = 0;
    }وگرنه اگر( raw_راست || z_راست[0]!='\\' ){
      c_راست = ((u8*)z_راست)[0];
      اگر( c_راست>=0xc0 ){
        صحیح sz = sqlite3Utf8ReadLimited((u8*)z_راست, n_راست, &c_راست);
        z_راست += sz;
        n_راست -= sz;
      }وگرنه{
        z_راست++;
        n_راست--;
      }
    }وگرنه{
      u32 n = jsonUnescapeOneChar(z_راست, n_راست, &c_راست);
      z_راست += n;
      أكد( n<=n_راست );
      n_راست -= n;
    }
    اگر( c_چپ!=c_راست ) بازگشت 0;
    اگر( c_چپ==0 ) بازگشت 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
ایستا صحیح jsonLabelCompare(
  ثابت نویسه *z_چپ,          /* The left label */
  u32 n_چپ,                  /* Size of the left label in bytes */
  صحیح raw_چپ,                /* True if zLeft contains no escapes */
  ثابت نویسه *z_راست,         /* The right label */
  u32 n_راست,                 /* Size of the right label in bytes */
  صحیح raw_راست                /* True if zRight is escape-free */
){
  اگر( raw_چپ && raw_راست ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    اگر( n_چپ!=n_راست ) بازگشت 0;
    بازگشت memcmp(z_چپ, z_راست, n_چپ)==0;
  }وگرنه{
    بازگشت jsonLabelCompareEscaped(z_چپ, n_چپ, raw_چپ,
                                   z_راست, n_راست, raw_راست);
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
ایستا u32 jsonLookupStep(JsonParse*,u32,ثابت نویسه*,u32);


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
ایستا u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  ثابت نویسه *zTail   /* Tail of the path that determines substructure */
){
  ایستا ثابت u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  صحیح rc;
  memset(pIns, 0, اندازه(*pIns));
  pIns->db = pParse->db;
  اگر( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }وگرنه{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    اگر( pIns->iDepth >= JSON_MAX_DEPTH ){
      بازگشت JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  بازگشت rc;  /* Error code only */
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
ایستا u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_ریشه,              /* Begin the search at this element of aBlob[] */
  ثابت نویسه *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_کلید, sz, n, iEnd, rc;
  ثابت نویسه *z_کلید;
  u8 x;

  اگر( zPath[0]==0 ){
    اگر( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, i_ریشه, &sz);
      sz += n;
      اگر( pParse->eEdit==JEDIT_DEL ){
        اگر( iLabel>0 ){
          sz += i_ریشه - iLabel;
          i_ریشه = iLabel;
        }
        jsonBlobEdit(pParse, i_ریشه, sz, 0, 0);
      }وگرنه اگر( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }وگرنه اگر( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        اگر( zPath[-1]!=']' ){
          بازگشت JSON_LOOKUP_NOTARRAY;
        }وگرنه{
          jsonBlobEdit(pParse, i_ریشه, 0, pParse->aIns, pParse->nIns);
        }
      }وگرنه{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_ریشه, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    بازگشت i_ریشه;
  }
  اگر( zPath[0]=='.' ){
    صحیح raw_کلید = 1;
    x = pParse->aBlob[i_ریشه];
    zPath++;
    اگر( zPath[0]=='"' ){
      z_کلید = zPath + 1;
      برای(i=1; zPath[i] && zPath[i]!='"'; i++){
        اگر( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_کلید = i-1;
      اگر( zPath[i] ){
        i++;
      }وگرنه{
        بازگشت JSON_LOOKUP_PATHERROR;
      }
      testcase( n_کلید==0 );
      raw_کلید = memchr(z_کلید, '\\', n_کلید)==0;
    }وگرنه{
      z_کلید = zPath;
      برای(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_کلید = i;
      اگر( n_کلید==0 ){
        بازگشت JSON_LOOKUP_PATHERROR;
      }
    }
    اگر( (x & 0x0f)!=JSONB_OBJECT ) بازگشت JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_ریشه, &sz);
    j = i_ریشه + n;  /* j is the index of a label */
    iEnd = j+sz;
    وقتی( j<iEnd ){
      صحیح rawLabel;
      ثابت نویسه *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      اگر( x<JSONB_TEXT || x>JSONB_TEXTRAW ) بازگشت JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      اگر( n==0 ) بازگشت JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      اگر( k+sz>=iEnd ) بازگشت JSON_LOOKUP_ERROR;
      zLabel = (ثابت نویسه*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      اگر( jsonLabelCompare(z_کلید, n_کلید, raw_کلید, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        اگر( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) بازگشت JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        اگر( n==0 || v+n+sz>iEnd ) بازگشت JSON_LOOKUP_ERROR;
        أكد( j>0 );
        اگر( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          بازگشت JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        اگر( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ریشه);
        بازگشت rc;
      }
      j = k+sz;
      اگر( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) بازگشت JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      اگر( n==0 ) بازگشت JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    اگر( j>iEnd ) بازگشت JSON_LOOKUP_ERROR;
    اگر( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      اگر( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        بازگشت JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, اندازه(ix));
      ix.db = pParse->db;
      json_Blob_Append_گره(&ix, raw_کلید?JSONB_TEXTRAW:JSONB_TEXT5, n_کلید, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      اگر( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_کلید+v.nBlob)
      ){
        أكد( !pParse->oom );
        nIns = ix.nBlob + n_کلید + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        اگر( !pParse->oom ){
          أكد( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          أكد( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], z_کلید, n_کلید);
          k += n_کلید;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          اگر( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, i_ریشه);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      بازگشت rc;
    }
  }وگرنه اگر( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_ریشه] & 0x0f;
    اگر( x!=JSONB_ARRAY )  بازگشت JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_ریشه, &sz);
    i = 1;
    وقتی( sqlite3Isdigit(zPath[i]) ){
      اگر( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    اگر( i<2 || zPath[i]!=']' ){
      اگر( zPath[1]=='#' ){
        kk = jsonb_Array_شمار(pParse, i_ریشه);
        i = 2;
        اگر( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          انجامبده{
            اگر( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }وقتی( sqlite3Isdigit(zPath[i]) );
          اگر( nn>kk ) بازگشت JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        اگر( zPath[i]!=']' ){
          بازگشت JSON_LOOKUP_PATHERROR;
        }
      }وگرنه{
        بازگشت JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_ریشه+n;
    iEnd = j+sz;
    وقتی( j<iEnd ){
      اگر( kk==0 ){
        اگر( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          بازگشت JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        اگر( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ریشه);
        بازگشت rc;
      }
      kk--;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      اگر( n==0 ) بازگشت JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    اگر( j>iEnd ) بازگشت JSON_LOOKUP_ERROR;
    اگر( kk>0 ) بازگشت JSON_LOOKUP_NOTFOUND;
    اگر( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      اگر( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        أكد( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      اگر( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_ریشه);
      بازگشت rc;
    }
  }وگرنه{
    بازگشت JSON_LOOKUP_PATHERROR; 
  }
  بازگشت JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
ایستا تهی jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  ثابت u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  اگر( NEVER(aBlob==0) ) بازگشت;
  memset(&x, 0, اندازه(x));
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
ایستا تهی jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  صحیح eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  صحیح rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  أكد( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  اگر( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    بازگشت;
  }
  انتخاب( pParse->aBlob[i] & 0x0f ){
    حالت JSONB_NULL: {
      اگر( sz ) برو returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      بشکن;
    }
    حالت JSONB_TRUE: {
      اگر( sz ) برو returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      بشکن;
    }
    حالت JSONB_FALSE: {
      اگر( sz ) برو returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      بشکن;
    }
    حالت JSONB_INT5:
    حالت JSONB_INT: {
      sqlite3_int64 iRes = 0;
      نویسه *z;
      صحیح bNeg = 0;
      نویسه x;
      اگر( sz==0 ) برو returnfromblob_malformed;
      x = (نویسه)pParse->aBlob[i+n];
      اگر( x=='-' ){
        اگر( sz<2 ) برو returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (ثابت نویسه*)&pParse->aBlob[i+n], (صحیح)sz);
      اگر( z==0 ) برو returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      اگر( rc==0 ){
        اگر( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          دوگانه r;
          r = (دوگانه)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }وگرنه{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }وگرنه اگر( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }وگرنه اگر( rc==1 ){
        برو returnfromblob_malformed;
      }وگرنه{
        اگر( bNeg ){ n--; sz++; }
        برو to_double;
      }
      بشکن;
    }
    حالت JSONB_FLOAT5:
    حالت JSONB_FLOAT: {
      دوگانه r;
      نویسه *z;
      اگر( sz==0 ) برو returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (ثابت نویسه*)&pParse->aBlob[i+n], (صحیح)sz);
      اگر( z==0 ) برو returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      اگر( rc<=0 ) برو returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      بشکن;
    }
    حالت JSONB_TEXTRAW:
    حالت JSONB_TEXT: {
      sqlite3_result_text(pCtx, (نویسه*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      بشکن;
    }
    حالت JSONB_TEXT5:
    حالت JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      ثابت نویسه *z;
      نویسه *zOut;
      u32 nOut = sz;
      z = (ثابت نویسه*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      اگر( zOut==0 ) برو returnfromblob_oom;
      برای(iIn=iOut=0; iIn<sz; iIn++){
        نویسه c = z[iIn];
        اگر( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          اگر( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }وگرنه اگر( v<=0x7f ){
            zOut[iOut++] = (نویسه)v;
          }وگرنه اگر( v<=0x7ff ){
            أكد( szEscape>=2 );
            zOut[iOut++] = (نویسه)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }وگرنه اگر( v<0x10000 ){
            أكد( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }وگرنه اگر( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }وگرنه{
            أكد( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }وگرنه{
          zOut[iOut++] = c;
        }
      } /* end for() */
      أكد( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      بشکن;
    }
    حالت JSONB_ARRAY:
    حالت JSONB_OBJECT: {
      اگر( eMode==0 ){
        اگر( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }وگرنه{
          eMode = 1;
        }
      }
      اگر( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }وگرنه{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      بشکن;
    }
    پیشفرض: {
      برو returnfromblob_malformed;
    }
  }
  بازگشت;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  بازگشت;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  بازگشت;
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
ایستا صحیح jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_مقدار *pArg,
  JsonParse *pParse
){
  صحیح eType = sqlite3_مقدار_type(pArg);
  ایستا u8 aNull[] = { 0x00 };
  memset(pParse, 0, اندازه(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  انتخاب( eType ){
    پیشفرض: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      بازگشت 0;
    }
    حالت SQLITE_BLOB: {
      اگر( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        بازگشت 1;
      }
      بشکن;
    }
    حالت SQLITE_TEXT: {
      ثابت نویسه *zJson = (ثابت نویسه*)sqlite3_مقدار_text(pArg);
      صحیح nJson = sqlite3_مقدار_bytes(pArg);
      اگر( zJson==0 ) بازگشت 1;
      اگر( sqlite3_مقدار_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (نویسه*)zJson;
        pParse->nJson = nJson;
        اگر( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, اندازه(pParse[0]));
          بازگشت 1;
        }
      }وگرنه{
        json_Blob_Append_گره(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      بشکن;
    }
    حالت SQLITE_FLOAT: {
      اگر( NEVER(sqlite3IsNaN(sqlite3_مقدار_double(pArg))) ){
        json_Blob_Append_گره(pParse, JSONB_NULL, 0, 0);
      }وگرنه{
        صحیح n = sqlite3_مقدار_bytes(pArg);
        ثابت نویسه *z = (ثابت نویسه*)sqlite3_مقدار_text(pArg);
        اگر( z==0 ) بازگشت 1;
        اگر( z[0]=='I' ){
          json_Blob_Append_گره(pParse, JSONB_FLOAT, 5, "9e999");
        }وگرنه اگر( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_گره(pParse, JSONB_FLOAT, 6, "-9e999");
        }وگرنه{
          json_Blob_Append_گره(pParse, JSONB_FLOAT, n, z);
        }
      }
      بشکن;
    }
    حالت SQLITE_INTEGER: {
      صحیح n = sqlite3_مقدار_bytes(pArg);
      ثابت نویسه *z = (ثابت نویسه*)sqlite3_مقدار_text(pArg);
      اگر( z==0 ) بازگشت 1;
      json_Blob_Append_گره(pParse, JSONB_INT, n, z);
      بشکن;
    }
  }
  اگر( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    بازگشت 1;
  }وگرنه{
    بازگشت 0;
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
ایستا نویسه *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  ثابت نویسه *zPath,        /* The path with the problem */
  صحیح rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  نویسه *z_پیام;
  اگر( rc==(صحیح)JSON_LOOKUP_NOTARRAY ){
    z_پیام = sqlite3_mprintf("not an array element: %Q", zPath);
  }وگرنه اگر( rc==(صحیح)JSON_LOOKUP_ERROR ){
    z_پیام = sqlite3_mprintf("malformed JSON");
  }وگرنه اگر( rc==(صحیح)JSON_LOOKUP_TOODEEP ){
    z_پیام = sqlite3_mprintf("JSON path too deep");
  }وگرنه{
    z_پیام = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  اگر( ctx==0 ) بازگشت z_پیام;
  اگر( z_پیام ){
    sqlite3_result_error(ctx, z_پیام, -1);
    sqlite3_free(z_پیام);
  }وگرنه{
    sqlite3_result_error_nomem(ctx);
  }
  بازگشت 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
ایستا تهی json_درج_Into_Blob(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv,
  صحیح eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  صحیح i;
  u32 rc = 0;
  ثابت نویسه *zPath = 0;
  صحیح flgs;
  JsonParse *p;
  JsonParse ax;

  أكد( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  اگر( p==0 ) بازگشت;
  برای(i=1; i<argc-1; i+=2){
    اگر( sqlite3_مقدار_type(argv[i])==SQLITE_NULL ) ادامه;
    zPath = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
    اگر( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      بازگشت;
    }
    اگر( zPath[0]!='$' ) برو json_درج_Into_Blob_patherror;
    اگر( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      بازگشت;
    }
    اگر( zPath[1]==0 ){
      اگر( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }وگرنه{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    اگر( rc==JSON_LOOKUP_NOTFOUND ) ادامه;
    اگر( JSON_LOOKUP_ISERROR(rc) ) برو json_درج_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  بازگشت;

json_درج_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  بازگشت;
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
ایستا صحیح jsonArgIsJsonb(sqlite3_مقدار *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  اگر( sqlite3_مقدار_type(pArg)!=SQLITE_BLOB ) بازگشت 0;
  p->aBlob = (u8*)sqlite3_مقدار_blob(pArg);
  p->nBlob = (u32)sqlite3_مقدار_bytes(pArg);
  اگر( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    بازگشت 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  بازگشت 0;
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
ایستا JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_مقدار *pArg,
  u32 flgs
){
  صحیح eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  أكد( ctx!=0 );
  eType = sqlite3_مقدار_type(pArg);
  اگر( eType==SQLITE_NULL ){
    بازگشت 0;
  }
  اگر( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_جستجو(ctx, pArg);
  }
  اگر( pFromCache ){
    pFromCache->nJPRef++;
    اگر( (flgs & JSON_EDITABLE)==0 ){
      بازگشت pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, اندازه(*p));
  اگر( p==0 ) برو json_pfa_oom;
  memset(p, 0, اندازه(*p));
  p->db = db;
  p->nJPRef = 1;
  اگر( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    اگر( p->aBlob==0 ) برو json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    بازگشت p;
  }
  اگر( eType==SQLITE_BLOB ){
    اگر( jsonArgIsJsonb(pArg,p) ){
      اگر( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        برو json_pfa_oom;
      }
      بازگشت p;
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
  p->zJson = (نویسه*)sqlite3_مقدار_text(pArg);
  p->nJson = sqlite3_مقدار_bytes(pArg);
  اگر( db->mallocFailed ) برو json_pfa_oom;
  اگر( p->nJson==0 ) برو json_pfa_malformed;
  أكد( p->zJson!=0 );
  اگر( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    اگر( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      بازگشت p;
    }وگرنه{
      jsonParseFree(p);
      بازگشت 0;
    }
  }وگرنه{
    صحیح isRCStr = sqlite3_مقدار_Is_Of_Class(pArg, sqlite3RCStrUnref);
    صحیح rc;
    اگر( !isRCStr ){
      نویسه *zNew = sqlite3RCStrNew( p->nJson );
      اگر( zNew==0 ) برو json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }وگرنه{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_درج(ctx, p);
    اگر( rc==SQLITE_NOMEM ) برو json_pfa_oom;
    اگر( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      برو rebuild_from_cache;
    }
  }
  بازگشت p;

json_pfa_malformed:
  اگر( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    بازگشت p;
  }وگرنه{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    بازگشت 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  بازگشت 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
ایستا تهی jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  صحیح flgs;
  اگر( p->oom ){
    sqlite3_result_error_nomem(ctx);
    بازگشت;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  اگر( flgs & JSON_BLOB ){
    اگر( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }وگرنه{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }وگرنه{
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

#اگر SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
ایستا تهی jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  صحیح nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  وقتی( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    صحیح showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    اگر( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    اگر( nn==0 ) nn = 1;
    اگر( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    برای(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    اگر( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      ادامه;
    }
    pParse->nBlob = savedNBlob;
    اگر( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      اگر( iEnd>pParse->nBlob ){
        اگر( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }وگرنه{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    انتخاب( x ){
      حالت JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); بشکن;
      حالت JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); بشکن;
      حالت JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); بشکن;
      حالت JSONB_INT:      sqlite3_str_appendall(pOut,"int"); بشکن;
      حالت JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); بشکن;
      حالت JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); بشکن;
      حالت JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); بشکن;
      حالت JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); بشکن;
      حالت JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); بشکن;
      حالت JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); بشکن;
      حالت JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); بشکن;
      حالت JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        بشکن;
      }
      حالت JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        بشکن;
      }
      پیشفرض: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        بشکن;
      }
    }
    اگر( showContent ){
      اگر( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }وگرنه{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        برای(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          اگر( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (نویسه*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
ایستا تهی jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  نویسه zBuf[1000];
  اگر( pParse==0 ){
    printf("NULL pointer\n");
    بازگشت;
  }وگرنه{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    اگر( pParse->nBlob==0 ) بازگشت;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, اندازه(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_مقدار(&out));
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
ایستا تهی jsonParseFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  أكد( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( p==0 ) بازگشت;
  اگر( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }وگرنه{
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
ایستا تهی jsonQuoteFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_مقدار(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
ایستا تهی jsonArrayFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  برای(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_مقدار(&jx, argv[i]);
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
ایستا تهی jsonArrayLengthFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( p==0 ) بازگشت;
  اگر( argc==2 ){
    ثابت نویسه *zPath = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]);
    اگر( zPath==0 ){
      jsonParseFree(p);
      بازگشت;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    اگر( JSON_LOOKUP_ISERROR(i) ){
      اگر( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }وگرنه{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }وگرنه{
    i = 0;
  }
  اگر( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_شمار(p, i);
  }
  اگر( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
ایستا صحیح jsonAllAlphanum(ثابت نویسه *z, صحیح n){
  صحیح i;
  برای(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  بازگشت i==n;
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
ایستا تهی jsonExtractFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonParse *p = 0;      /* The parse */
  صحیح flags;             /* Flags associated with the function */
  صحیح i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  اگر( argc<2 ) بازگشت;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( p==0 ) بازگشت;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  اگر( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  برای(i=1; i<argc; i++){
    /* With a single PATH argument */
    ثابت نویسه *zPath = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
    صحیح nPath;
    u32 j;
    اگر( zPath==0 ) برو json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    اگر( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }وگرنه اگر( (flags & JSON_ABPATH) ){
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
      اگر( sqlite3_مقدار_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        اگر( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }وگرنه اگر( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }وگرنه اگر( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }وگرنه{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }وگرنه{
      jsonBadPathError(ctx, zPath, 0);
      برو json_extract_error;
    }
    اگر( j<p->nBlob ){
      اگر( argc==2 ){
        اگر( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          أكد( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }وگرنه{
          jsonReturnFromBlob(p, j, ctx, 0);
          اگر( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }وگرنه{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }وگرنه اگر( j==JSON_LOOKUP_NOTFOUND ){
      اگر( argc==2 ){
        برو json_extract_error;  /* Return NULL if not found */
      }وگرنه{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }وگرنه{
      jsonBadPathError(ctx, zPath, j);
      برو json_extract_error;
    }
  }
  اگر( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    اگر( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  بازگشت;
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
ایستا صحیح jsonMergePatch(
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
  اگر( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    بازگشت pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  اگر( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  اگر( NEVER(n==0) ) بازگشت JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  اگر( NEVER(n==0) ) بازگشت JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  وقتی( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    اگر( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      بازگشت JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    اگر( nPLabel==0 ) بازگشت JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    اگر( iPValue>=iPEnd ) بازگشت JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    اگر( nPValue==0 ) بازگشت JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    اگر( iPCursor>iPEnd ) بازگشت JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    وقتی( iTCursor<iTEnd ){
      صحیح isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      اگر( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        بازگشت JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      اگر( nTLabel==0 ) بازگشت JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      اگر( iTValue>=iTEnd ) بازگشت JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      اگر( nTValue==0 ) بازگشت JSON_MERGE_BADTARGET;
      اگر( iTValue + nTValue + szTValue > iTEnd ) بازگشت JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ثابت نویسه*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (ثابت نویسه*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      اگر( isEqual ) بشکن;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    اگر( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      اگر( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        اگر( NEVER(pTarget->oom) ) بازگشت JSON_MERGE_OOM;
      }وگرنه{
        /* Algorithm line 12 */
        صحیح rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        اگر( iDepth>=JSON_MAX_DEPTH ) بازگشت JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        اگر( rc ) بازگشت rc;
        pTarget->delta += savedDelta;
      }        
    }وگرنه اگر( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      اگر( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        اگر( pTarget->oom ) بازگشت JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }وگرنه{
        صحیح rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        اگر( pTarget->oom ) بازگشت JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        اگر( iDepth>=JSON_MAX_DEPTH ) بازگشت JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        اگر( rc ) بازگشت rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  اگر( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  بازگشت pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
ایستا تهی jsonPatchFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  صحیح rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  أكد( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  اگر( pTarget==0 ) بازگشت;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  اگر( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    اگر( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }وگرنه اگر( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }وگرنه اگر( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }وگرنه{
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
ایستا تهی jsonObjectFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح i;
  JsonString jx;
  ثابت نویسه *z;
  u32 n;

  اگر( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    بازگشت;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  برای(i=0; i<argc; i+=2){
    اگر( sqlite3_مقدار_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      بازگشت;
    }
    jsonAppendSeparator(&jx);
    z = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
    n = sqlite3_مقدار_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_مقدار(&jx, argv[i+1]);
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
ایستا تهی jsonRemoveFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonParse *p;          /* The parse */
  ثابت نویسه *zPath = 0; /* Path of element to be removed */
  صحیح i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  اگر( argc<1 ) بازگشت;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  اگر( p==0 ) بازگشت;
  برای(i=1; i<argc; i++){
    zPath = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
    اگر( zPath==0 ){
      برو json_remove_done;
    }
    اگر( zPath[0]!='$' ){
      برو json_remove_patherror;
    }
    اگر( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      برو json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    اگر( JSON_LOOKUP_ISERROR(rc) ){
      اگر( rc==JSON_LOOKUP_NOTFOUND ){
        ادامه;  /* No-op */
      }وگرنه{
        jsonBadPathError(ctx, zPath, rc);
      }
      برو json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  بازگشت;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  بازگشت;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
ایستا تهی jsonReplaceFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  اگر( argc<1 ) بازگشت;
  اگر( (argc&1)==0 ) {
    json_Wrong_رقم_Args(ctx, "replace");
    بازگشت;
  }
  json_درج_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
ایستا تهی jsonSetFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  صحیح eInsType = JSON_درج_TYPE(flags);
  ایستا ثابت نویسه *azInsType[] = { "insert", "set", "array_insert" };
  ایستا ثابت u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  اگر( argc<1 ) بازگشت;
  أكد( eInsType>=0 && eInsType<=2 );
  اگر( (argc&1)==0 ) {
    json_Wrong_رقم_Args(ctx, azInsType[eInsType]);
    بازگشت;
  }
  json_درج_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
ایستا تهی jsonTypeFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonParse *p;          /* The parse */
  ثابت نویسه *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( p==0 ) بازگشت;
  اگر( argc==2 ){
    zPath = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]);
    اگر( zPath==0 ) برو json_type_done;
    اگر( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      برو json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    اگر( JSON_LOOKUP_ISERROR(i) ){
      اگر( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }وگرنه{
        jsonBadPathError(ctx, zPath, i);
      }
      برو json_type_done;
    }
  }وگرنه{
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
ایستا تهی jsonPrettyFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, اندازه(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( x.pParse==0 ) بازگشت;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  اگر( argc==1 || (x.zIndent = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }وگرنه{
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
ایستا تهی jsonValidFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  اگر( argc==2 ){
    i64 f = sqlite3_مقدار_int64(argv[1]);
    اگر( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      بازگشت;
    }
    flags = f & 0x0f;
  }
  انتخاب( sqlite3_مقدار_type(argv[0]) ){
    حالت SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      بازگشت;
    }
    حالت SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, اندازه(py));
      اگر( jsonArgIsJsonb(argv[0], &py) ){
        اگر( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }وگرنه اگر( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        بشکن;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    پیشفرض: {
      JsonParse px;
      اگر( (flags & 0x3)==0 ) بشکن;
      memset(&px, 0, اندازه(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      اگر( p ){
        اگر( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }وگرنه اگر( p->nErr ){
          /* no-op */
        }وگرنه اگر( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }وگرنه{
        sqlite3_result_error_nomem(ctx);
      }
      بشکن;
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
ایستا تهی jsonErrorFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  أكد( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, اندازه(s));
  s.db = sqlite3_context_db_handle(ctx);
  اگر( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }وگرنه{
    s.zJson = (نویسه*)sqlite3_مقدار_text(argv[0]);
    اگر( s.zJson==0 ) بازگشت;  /* NULL input or OOM */
    s.nJson = sqlite3_مقدار_bytes(argv[0]);
    اگر( jsonConvertTextToBlob(&s,0) ){
      اگر( s.oom ){
        iErrPos = -1;
      }وگرنه{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        أكد( s.zJson!=0 );  /* Because s.oom is false */
        برای(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          اگر( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  اگر( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }وگرنه{
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
ایستا تهی jsonArrayStep(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, اندازه(*pStr));
  اگر( pStr ){
    اگر( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }وگرنه اگر( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_مقدار(pStr, argv[0]);
  }
}
ایستا تهی jsonArrayCompute(sqlite3_context *ctx, صحیح isFinal){
  JsonString *pStr;
  صحیح flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  اگر( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    اگر( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      بازگشت;
    }وگرنه اگر( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      اگر( isFinal ){
        اگر( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }وگرنه{
        jsonStringTrimOneChar(pStr);
      }
      بازگشت;
    }وگرنه اگر( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (صحیح)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }وگرنه{
      sqlite3_result_text(ctx, pStr->zBuf, (صحیح)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }وگرنه اگر( flags & JSON_BLOB ){
    ایستا ثابت u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }وگرنه{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
ایستا تهی json_Array_مقدار(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
ایستا تهی jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
ایستا تهی jsonGroupInverse(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  unsigned صحیح i;
  صحیح inStr = 0;
  صحیح nNest = 0;
  نویسه *z;
  نویسه c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  اگر( NEVER(!pStr) ) بازگشت;
  z = pStr->zBuf;
  برای(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    اگر( c=='"' ){
      inStr = !inStr;
    }وگرنه اگر( c=='\\' ){
      i++;
    }وگرنه اگر( !inStr ){
      اگر( c=='{' || c=='[' ) nNest++;
      اگر( c=='}' || c==']' ) nNest--;
    }
  }
  اگر( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }وگرنه{
    pStr->nUsed = 1;
  }
}
#وگرنه
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
ایستا تهی jsonObjectStep(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_مقدار **argv
){
  JsonString *pStr;
  ثابت نویسه *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, اندازه(*pStr));
  اگر( pStr ){
    z = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
    n = sqlite3Strlen30(z);
    اگر( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }وگرنه اگر( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    اگر( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_مقدار(pStr, argv[1]);
    }وگرنه{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
ایستا تهی jsonObjectCompute(sqlite3_context *ctx, صحیح isFinal){
  JsonString *pStr;
  صحیح flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  اگر( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    اگر( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      بازگشت;
    }
    اگر( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      صحیح inStr = 0;
      اگر( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        اگر( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          بازگشت;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      برای(i=j=1; i<pStr->nUsed; i++){
        نویسه c = pStr->zBuf[i];
        اگر( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }وگرنه اگر( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }وگرنه اگر( c=='@' && !inStr ){
          أكد( i+1<pStr->nUsed );
          اگر( pStr->zBuf[i+1]==',' ){
            i++;
          }وگرنه اگر( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }وگرنه{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    اگر( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      اگر( isFinal ){
        اگر( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }وگرنه{
        jsonStringTrimOneChar(pOgStr);
      }
    }وگرنه اگر( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (صحیح)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }وگرنه{
      sqlite3_result_text(ctx, pStr->zBuf, (صحیح)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    اگر( pStr!=pOgStr ) jsonStringReset(pStr);
  }وگرنه اگر( flags & JSON_BLOB ){
    ایستا ثابت unsigned نویسه emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }وگرنه{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
ایستا تهی json_Object_مقدار(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
ایستا تهی jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
تعریفنوع ساختار JsonParent JsonParent;
ساختار JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 i_مقدار;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_کلید;                  /* Key for JSONB_ARRAY */
};

تعریفنوع ساختار JsonEachCursor JsonEachCursor;
ساختار JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_ریشه;                 /* Size of the root path in bytes */
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
تعریفنوع ساختار JsonEachConnection JsonEachConnection;
ساختار JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
ایستا صحیح jsonEachConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  JsonEachConnection *pNew;
  صحیح rc;

/* Column numbers */
#define JEACH_کلید     0
#define JEACH_مقدار   1
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
#define JEACH_ریشه    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  اگر( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, اندازه(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    اگر( pNew==0 ) بازگشت SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  بازگشت rc;
}

/* destructor for json_each virtual table */
ایستا صحیح jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  بازگشت SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
ایستا صحیح jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, اندازه(*pCur));
  اگر( pCur==0 ) بازگشت SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  بازگشت SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
ایستا تهی jsonEachCursorReset(JsonEachCursor *p){
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
ایستا صحیح jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  بازگشت SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
ایستا صحیح jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  بازگشت p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
ایستا صحیح jsonSkipLabel(JsonEachCursor *p){
  اگر( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    اگر( sz >= p->sParse.nBlob ) sz = p->i;
    بازگشت sz;
  }وگرنه{
    بازگشت p->i;
  }
}

/*
** Append the path name for the current element.
*/
ایستا تهی jsonAppendPathName(JsonEachCursor *p){
  أكد( p->nParent>0 );
  أكد( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  اگر( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_کلید);
  }وگرنه{
    u32 n, sz = 0, k, i;
    ثابت نویسه *z;
    صحیح needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (ثابت نویسه*)&p->sParse.aBlob[k];
    اگر( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }وگرنه{
      برای(i=0; i<sz; i++){
        اگر( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          بشکن;
        }
      }
    }
    اگر( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }وگرنه{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
ایستا صحیح json_Each_Malformed_درون_داد(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_پیام);
  cur->pVtab->z_Err_پیام = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  بازگشت cur->pVtab->z_Err_پیام ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
ایستا صحیح jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  صحیح rc = SQLITE_OK;
  اگر( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    اگر( n==0 )بازگشت json_Each_Malformed_درون_داد(cur);
    اگر( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      اگر( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, اندازه(JsonParent)*nNew);
        اگر( pNew==0 ) بازگشت SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->i_مقدار = i;
      pParent->iEnd = i + n + sz;
      pParent->i_کلید = -1;
      pParent->nPath = (u32)p->path.nUsed;
      اگر( p->eType && p->nParent ){
        jsonAppendPathName(p);
        اگر( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }وگرنه{
      p->i = i + n + sz;
    }
    وقتی( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    اگر( levelChange ){
      اگر( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->i_مقدار;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }وگرنه{
        p->eType = 0;
      }
    }
  }وگرنه{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    اگر( n==0 )بازگشت json_Each_Malformed_درون_داد(cur);
    p->i = i + n + sz;
  }
  اگر( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_کلید++;
  }
  p->iRowid++;
  بازگشت rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
ایستا صحیح jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  نویسه *z = p->path.zBuf;
  اگر( p->iRowid==0 && p->bRecursive && n>=2 ){
    وقتی( ALWAYS(n>1) ){
      n--;
      اگر( z[n]=='[' || z[n]=='.' ){
        u32 x;
        نویسه cSaved = z[n];
        z[n] = 0;
        أكد( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        اگر( !JSON_LOOKUP_ISERROR(x) ) بشکن;
      }
    }
  }
  بازگشت n;
}

/* Return the value of a column */
ایستا صحیح jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحیح iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  انتخاب( iColumn ){
    حالت JEACH_کلید: {
      اگر( p->nParent==0 ){
        u32 n, j;
        اگر( p->n_ریشه==1 ) بشکن;
        j = jsonEachPathLength(p);
        n = p->n_ریشه - j;
        اگر( n==0 ){
          بشکن;
        }وگرنه اگر( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }وگرنه اگر( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }وگرنه{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        بشکن;
      }
      اگر( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }وگرنه{
        أكد( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].i_کلید);
      }
      بشکن;
    }
    حالت JEACH_مقدار: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      اگر( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      بشکن;
    }
    حالت JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      بشکن;
    }
    حالت JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      اگر( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      بشکن;
    }
    حالت JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      بشکن;
    }
    حالت JEACH_PARENT: {
      اگر( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      بشکن;
    }
    حالت JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      اگر( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      بشکن;
    }
    حالت JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      بشکن;
    }
    پیشفرض: {
      sqlite3_result_text(ctx, p->path.zBuf, p->n_ریشه, SQLITE_TRANSIENT);
      بشکن;
    }
    حالت JEACH_JSON: {
      اگر( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }وگرنه{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      بشکن;
    }
  }
  بازگشت SQLITE_OK;
}

/* Return the current rowid value */
ایستا صحیح jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  بازگشت SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
ایستا صحیح jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  صحیح i;                     /* Loop counter or computed array index */
  صحیح aIdx[2];               /* Index of constraints for JSON and ROOT */
  صحیح unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  صحیح idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  ثابت ساختار sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  أكد( JEACH_ریشه == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  برای(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    صحیح iCol;
    صحیح iMask;
    اگر( pConstraint->iColumn < JEACH_JSON ) ادامه;
    iCol = pConstraint->iColumn - JEACH_JSON;
    أكد( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    اگر( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }وگرنه اگر( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  اگر( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  اگر( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    بازگشت SQLITE_CONSTRAINT;
  }
  اگر( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_رقم = 0;
  }وگرنه{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    اگر( aIdx[1]<0 ){
      pIdxInfo->idx_رقم = 1;  /* Only JSON supplied.  Plan 1 */
    }وگرنه{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_رقم = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  بازگشت SQLITE_OK;
}

/* Start a search on a new JSON string */
ایستا صحیح jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ثابت نویسه *z_ریشه = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  اگر( idx_رقم==0 ) بازگشت SQLITE_OK;
  memset(&p->sParse, 0, اندازه(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  اگر( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }وگرنه{
    p->sParse.zJson = (نویسه*)sqlite3_مقدار_text(argv[0]);
    p->sParse.nJson = sqlite3_مقدار_bytes(argv[0]);
    اگر( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      بازگشت SQLITE_OK;
    }      
    اگر( jsonConvertTextToBlob(&p->sParse, 0) ){
      اگر( p->sParse.oom ){
        بازگشت SQLITE_NOMEM;
      }
      بازگشت json_Each_Malformed_درون_داد(cur);
    }
  }
  اگر( idx_رقم==3 ){
    z_ریشه = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]);
    اگر( z_ریشه==0 ) بازگشت SQLITE_OK;
    اگر( z_ریشه[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_پیام);
      cur->pVtab->z_Err_پیام = jsonBadPathError(0, z_ریشه, 0);
      jsonEachCursorReset(p);
      بازگشت cur->pVtab->z_Err_پیام ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_ریشه = sqlite3Strlen30(z_ریشه);
    اگر( z_ریشه[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }وگرنه{
      i = jsonLookupStep(&p->sParse, 0, z_ریشه+1, 0);
      اگر( JSON_LOOKUP_ISERROR(i) ){
        اگر( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          بازگشت SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_پیام);
        cur->pVtab->z_Err_پیام = jsonBadPathError(0, z_ریشه, 0);
        jsonEachCursorReset(p);
        بازگشت cur->pVtab->z_Err_پیام ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      اگر( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }وگرنه{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_ریشه, p->n_ریشه);
  }وگرنه{
    i = p->i = 0;
    p->eType = 0;
    p->n_ریشه = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  اگر( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, اندازه(JsonParent));
    اگر( p->aParent==0 ) بازگشت SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_کلید = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].i_مقدار = i;
  }
  بازگشت SQLITE_OK;
}

/* The methods of the json_each virtual table */
ایستا sqlite3_module jsonEachModule = {
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
تهی sqlite3RegisterJsonFunctions(تهی){
#ifndef SQLITE_OMIT_JSON
  ایستا FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_درج, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_درج,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_درج,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_درج,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#اگر SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_مقدار, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_مقدار, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_مقدار, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_مقدار, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_درج_Builtin_Funcs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#اگر  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, ثابت نویسه *zName){
  unsigned صحیح i;
  ایستا ثابت نویسه *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  أكد( sqlite3HashFind(&db->aModule, zName)==0 );
  برای(i=0; i<اندازه(azModule)/اندازه(azModule[0]); i++){
    اگر( sqlite3StrICmp(azModule[i],zName)==0 ){
      بازگشت sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  بازگشت 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
