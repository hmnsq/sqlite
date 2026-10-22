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
#অন্তর্ভাব "sqliteInt.h"

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
স্থির ধ্রুবক অক্ষর * ধ্রুবক jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
স্থির ধ্রুবক অক্ষর jsonIsSpace[] = {
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
#define jsonIsspace(x) (jsonIsSpace[(unsigned অক্ষর)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
স্থির ধ্রুবক অক্ষর jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
স্থির ধ্রুবক অক্ষর jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
স্থির ধ্রুবক অক্ষর jsonIsOk[256] = {
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
প্রকারলেখ কাঠামো JsonCache JsonCache;
প্রকারলেখ কাঠামো JsonString JsonString;
প্রকারলেখ কাঠামো JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#define JSON_CACHE_ID    (-429938)  /* Cache entry */
#define JSON_CACHE_আকার  4          /* Max number of cache entries */

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
কাঠামো JsonCache {
  sqlite3 *db;                    /* Database connection */
  পূর্ণসংখ্যা nUsed;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_আকার];  /* One line for each cache entry */
};

/* An instance of this object represents a JSON string
** under construction.  Really, this is a generic string accumulator
** that can be and is used to create strings other than JSON.
**
** If the generated string is longer than will fit into the zSpace[] buffer,
** then it will be an RCStr string.  This aids with caching of large
** JSON strings.
*/
কাঠামো JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  অক্ষর *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  অক্ষর zSpace[100];        /* Initial static space */
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

#define JSON_ঢোকাও_TYPE(X)  (((X)&0xC)>>2)


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
কাঠামো JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  অক্ষর *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  পূর্ণসংখ্যা nJson;         /* Length of the zJson string in bytes */
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
  পূর্ণসংখ্যা delta;         /* Size change due to the edit */
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
#অন্যথা
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
স্থির শূন্য jsonReturnStringAsBlob(JsonString*);
স্থির পূর্ণসংখ্যা jsonArgIsJsonb(sqlite3_মান *pJson, JsonParse *p);
স্থির u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
স্থির শূন্য jsonReturnParse(sqlite3_context*,JsonParse*);
স্থির JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_মান*,u32);
স্থির শূন্য jsonParseFree(JsonParse*);
স্থির u32 jsonb_Payload_আকার(ধ্রুবক JsonParse*, u32, u32*);
স্থির u32 jsonUnescapeOneChar(ধ্রুবক অক্ষর*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
স্থির শূন্য jsonCacheDelete(JsonCache *p){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
স্থির শূন্য jsonCacheDeleteGeneric(শূন্য *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
স্থির পূর্ণসংখ্যা json_Cache_ঢোকাও(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  প্রতিজ্ঞা( pParse->zJson!=0 );
  প্রতিজ্ঞা( pParse->bJsonIsRCStr );
  প্রতিজ্ঞা( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  যদি( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, আকার(*p));
    যদি( p==0 ) ফেরত SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    যদি( p==0 ) ফেরত SQLITE_NOMEM;
  }
  যদি( p->nUsed >= JSON_CACHE_আকার ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_আকার-1)*আকার(p->a[0]));
    p->nUsed = JSON_CACHE_আকার-1;
  }
  প্রতিজ্ঞা( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  ফেরত SQLITE_OK;
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
স্থির JsonParse *json_Cache_অনুসন্ধান(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_মান *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  পূর্ণসংখ্যা i;
  ধ্রুবক অক্ষর *zJson;
  পূর্ণসংখ্যা nJson;

  যদি( sqlite3_মান_type(pArg)!=SQLITE_TEXT ){
    ফেরত 0;
  }
  zJson = (ধ্রুবক অক্ষর*)sqlite3_মান_text(pArg);
  যদি( zJson==0 ) ফেরত 0;
  nJson = sqlite3_মান_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  যদি( p==0 ){
    ফেরত 0;
  }
  জন্য(i=0; i<p->nUsed; i++){
    যদি( p->a[i]->zJson==zJson ) বিরতি;
  }
  যদি( i>=p->nUsed ){
    জন্য(i=0; i<p->nUsed; i++){
      যদি( p->a[i]->nJson!=nJson ) চালু;
      যদি( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) বিরতি;
    }
  }
  যদি( i<p->nUsed ){
    যদি( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*আকার(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    প্রতিজ্ঞা( p->a[i]->delta==0 );
    ফেরত p->a[i];
  }অন্যথা{
    ফেরত 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
স্থির শূন্য jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = আকার(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
স্থির শূন্য jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
স্থির শূন্য jsonStringReset(JsonString *p){
  যদি( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
স্থির শূন্য jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  যদি( p->pCtx ) sqlite3_ফলাফল_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
স্থির শূন্য jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  প্রতিজ্ঞা( p->pCtx!=0 );
  sqlite3_ফলাফল_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
স্থির পূর্ণসংখ্যা jsonStringGrow(JsonString *p, u32 N){
  u64 n_যোগ = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  অক্ষর *zNew;
  যদি( p->bStatic ){
    যদি( p->eErr ) ফেরত 1;
    zNew = sqlite3RCStrNew(n_যোগ);
    যদি( zNew==0 ){
      jsonStringOom(p);
      ফেরত SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }অন্যথা{
    p->zBuf = sqlite3RCStrResize(p->zBuf, n_যোগ);
    যদি( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      ফেরত SQLITE_NOMEM;
    }
  }
  p->nAlloc = n_যোগ;
  ফেরত SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
স্থির SQLITE_NOINLINE শূন্য jsonStringExpandAndAppend(
  JsonString *p,
  ধ্রুবক অক্ষর *zIn,
  u32 N
){
  প্রতিজ্ঞা( N>0 );
  যদি( jsonStringGrow(p,N) ) ফেরত;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
স্থির শূন্য jsonAppendRaw(JsonString *p, ধ্রুবক অক্ষর *zIn, u32 N){
  যদি( N==0 ) ফেরত;
  যদি( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }অন্যথা{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
স্থির শূন্য jsonAppendRawNZ(JsonString *p, ধ্রুবক অক্ষর *zIn, u32 N){
  প্রতিজ্ঞা( N>0 );
  যদি( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }অন্যথা{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
স্থির শূন্য jsonPrintf(পূর্ণসংখ্যা N, JsonString *p, ধ্রুবক অক্ষর *zFormat, ...){
  va_list ap;
  যদি( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) ফেরত;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (পূর্ণসংখ্যা)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
স্থির SQLITE_NOINLINE শূন্য jsonAppendCharExpand(JsonString *p, অক্ষর c){
  যদি( jsonStringGrow(p,1) ) ফেরত;
  p->zBuf[p->nUsed++] = c;
}
স্থির শূন্য jsonAppendChar(JsonString *p, অক্ষর c){
  যদি( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }অন্যথা{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
স্থির শূন্য jsonStringTrimOneChar(JsonString *p){
  যদি( p->eErr==0 ){
    প্রতিজ্ঞা( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
স্থির পূর্ণসংখ্যা jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  ফেরত p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
স্থির শূন্য jsonAppendSeparator(JsonString *p){
  অক্ষর c;
  যদি( p->nUsed==0 ) ফেরত;
  c = p->zBuf[p->nUsed-1];
  যদি( c=='[' || c=='{' ) ফেরত;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
স্থির শূন্য jsonAppendControlChar(JsonString *p, u8 c){
  স্থির ধ্রুবক অক্ষর aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  প্রতিজ্ঞা( আকার(aSpecial)==32 );
  প্রতিজ্ঞা( aSpecial['\b']=='b' );
  প্রতিজ্ঞা( aSpecial['\f']=='f' );
  প্রতিজ্ঞা( aSpecial['\n']=='n' );
  প্রতিজ্ঞা( aSpecial['\r']=='r' );
  প্রতিজ্ঞা( aSpecial['\t']=='t' );
  প্রতিজ্ঞা( c>=0 && c<আকার(aSpecial) );
  প্রতিজ্ঞা( p->nUsed+7 <= p->nAlloc );
  যদি( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }অন্যথা{
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
স্থির শূন্য jsonAppendString(JsonString *p, ধ্রুবক অক্ষর *zIn, u32 N){
  u32 k;
  u8 c;
  ধ্রুবক u8 *z = (ধ্রুবক u8*)zIn;
  যদি( z==0 ) ফেরত;
  যদি( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) ফেরত;
  p->zBuf[p->nUsed++] = '"';
  যখন( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    যখন( 1 /* Exit by break */ ){
      যদি( k+3>=N ){
        যখন( k<N && jsonIsOk[z[k]] ){ k++; }
        বিরতি;
      }
      যদি( !jsonIsOk[z[k]] ){
        বিরতি;
      }
      যদি( !jsonIsOk[z[k+1]] ){
        k += 1;
        বিরতি;
      }
      যদি( !jsonIsOk[z[k+2]] ){
        k += 2;
        বিরতি;
      }
      যদি( !jsonIsOk[z[k+3]] ){
        k += 3;
        বিরতি;
      }অন্যথা{
        k += 4;
      }
    }
    যদি( k>=N ){
      যদি( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      বিরতি;
    }
    যদি( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    যদি( c=='"' || c=='\\' ){
      যদি( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) ফেরত;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }অন্যথা যদি( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }অন্যথা{
      যদি( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) ফেরত;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  প্রতিজ্ঞা( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
স্থির শূন্য json_Append_Sql_মান(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_মান *p_মান          /* Value to append */
){
  নির্বাচন( sqlite3_মান_type(p_মান) ){
    ক্ষেত্র SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      বিরতি;
    }
    ক্ষেত্র SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_মান_double(p_মান));
      বিরতি;
    }
    ক্ষেত্র SQLITE_INTEGER: {
      ধ্রুবক অক্ষর *z = (ধ্রুবক অক্ষর*)sqlite3_মান_text(p_মান);
      u32 n = (u32)sqlite3_মান_bytes(p_মান);
      jsonAppendRaw(p, z, n);
      বিরতি;
    }
    ক্ষেত্র SQLITE_TEXT: {
      ধ্রুবক অক্ষর *z = (ধ্রুবক অক্ষর*)sqlite3_মান_text(p_মান);
      u32 n = (u32)sqlite3_মান_bytes(p_মান);
      যদি( sqlite3_মান_subtype(p_মান)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }অন্যথা{
        jsonAppendString(p, z, n);
      }
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      JsonParse px;
      memset(&px, 0, আকার(px));
      যদি( jsonArgIsJsonb(p_মান, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }অন্যথা যদি( p->eErr==0 ){
        sqlite3_ফলাফল_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      বিরতি;
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
স্থির শূন্য jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  প্রতিজ্ঞা( (pParse!=0)==(ctx!=0) );
  প্রতিজ্ঞা( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  যদি( p->eErr==0 ){
    পূর্ণসংখ্যা flags = SQLITE_PTR_TO_INT(sqlite3_user_তথ্য(p->pCtx));
    যদি( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }অন্যথা যদি( p->bStatic ){
      sqlite3_ফলাফল_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }অন্যথা{
      যদি( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        পূর্ণসংখ্যা rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_ঢোকাও(ctx, pParse);
        যদি( rc==SQLITE_NOMEM ){
          sqlite3_ফলাফল_error_nomem(ctx);
          jsonStringReset(p);
          ফেরত;
        }
      }
      sqlite3_ফলাফল_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }অন্যথা যদি( p->eErr & JSTRING_OOM ){
    sqlite3_ফলাফল_error_nomem(p->pCtx);
  }অন্যথা যদি( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }অন্যথা যদি( p->eErr & JSTRING_MALFORMED ){
    sqlite3_ফলাফল_error(p->pCtx, "malformed JSON", -1);
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
স্থির শূন্য jsonParseReset(JsonParse *pParse){
  প্রতিজ্ঞা( pParse->nJPRef<=1 );
  যদি( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  যদি( pParse->nBlobAlloc ){
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
স্থির শূন্য jsonParseFree(JsonParse *pParse){
  যদি( pParse ){
    যদি( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }অন্যথা{
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
স্থির u8 jsonHexToInt(পূর্ণসংখ্যা h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  ফেরত (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
স্থির u32 jsonHexToInt4(ধ্রুবক অক্ষর *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  ফেরত v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
স্থির পূর্ণসংখ্যা jsonIs2Hex(ধ্রুবক অক্ষর *z){
  ফেরত sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
স্থির পূর্ণসংখ্যা jsonIs4Hex(ধ্রুবক অক্ষর *z){
  ফেরত jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
স্থির পূর্ণসংখ্যা json5Whitespace(ধ্রুবক অক্ষর *zIn){
  পূর্ণসংখ্যা n = 0;
  ধ্রুবক u8 *z = (u8*)zIn;
  যখন( 1 /*exit by "goto whitespace_done"*/ ){
    নির্বাচন( z[n] ){
      ক্ষেত্র 0x09:
      ক্ষেত্র 0x0a:
      ক্ষেত্র 0x0b:
      ক্ষেত্র 0x0c:
      ক্ষেত্র 0x0d:
      ক্ষেত্র 0x20: {
        n++;
        বিরতি;
      }
      ক্ষেত্র '/': {
        যদি( z[n+1]=='*' && z[n+2]!=0 ){
          পূর্ণসংখ্যা j;
          জন্য(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            যদি( z[j]==0 ) যাও whitespace_done;
          }
          n = j+1;
          বিরতি;
        }অন্যথা যদি( z[n+1]=='/' ){
          পূর্ণসংখ্যা j;
          অক্ষর c;
          জন্য(j=n+2; (c = z[j])!=0; j++){
            যদি( c=='\n' || c=='\r' ) বিরতি;
            যদি( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              বিরতি;
            }
          }
          n = j;
          যদি( z[n] ) n++;
          বিরতি;
        }
        যাও whitespace_done;
      }
      ক্ষেত্র 0xc2: {
        যদি( z[n+1]==0xa0 ){
          n += 2;
          বিরতি;
        }
        যাও whitespace_done;
      }
      ক্ষেত্র 0xe1: {
        যদি( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          বিরতি;
        }
        যাও whitespace_done;
      }
      ক্ষেত্র 0xe2: {
        যদি( z[n+1]==0x80 ){
          u8 c = z[n+2];
          যদি( c<0x80 ) যাও whitespace_done;
          যদি( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            বিরতি;
          }
        }অন্যথা যদি( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          বিরতি;
        }
        যাও whitespace_done;
      }
      ক্ষেত্র 0xe3: {
        যদি( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          বিরতি;
        }
        যাও whitespace_done;
      }
      ক্ষেত্র 0xef: {
        যদি( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          বিরতি;
        }
        যাও whitespace_done;
      }
      পূর্বনির্ধারিত: {
        যাও whitespace_done;
      }
    }
  }
  whitespace_done:
  ফেরত n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
স্থির ধ্রুবক কাঠামো NanInfName {
  অক্ষর c1;
  অক্ষর c2;
  অক্ষর n;
  অক্ষর eType;
  অক্ষর nRepl;
  অক্ষর *zMatch;
  অক্ষর *zRepl;
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
স্থির শূন্য json_Wrong_অঙ্ক_Args(
  sqlite3_context *pCtx,
  ধ্রুবক অক্ষর *zFuncName
){
  অক্ষর *z_বার্তা = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_ফলাফল_error(pCtx, z_বার্তা, -1);
  sqlite3_free(z_বার্তা);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
স্থির পূর্ণসংখ্যা jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  প্রতিজ্ঞা( N>pParse->nBlobAlloc );
  যদি( pParse->nBlobAlloc==0 ){
    t = 100;
  }অন্যথা{
    t = pParse->nBlobAlloc*2;
  }
  যদি( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  যদি( aNew==0 ){ pParse->oom = 1; ফেরত 1; }
  প্রতিজ্ঞা( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  ফেরত 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
স্থির পূর্ণসংখ্যা jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 n_আকার;
  প্রতিজ্ঞা( !pParse->bReadOnly );
  যদি( pParse->oom ) ফেরত 0;
  যদি( pParse->nBlobAlloc>0 ) ফেরত 1;
  aOld = pParse->aBlob;
  n_আকার = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  যদি( jsonBlobExpand(pParse, n_আকার) ){
    ফেরত 0;
  }
  প্রতিজ্ঞা( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  ফেরত 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
স্থির SQLITE_NOINLINE শূন্য jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  যদি( pParse->oom==0 ){
    প্রতিজ্ঞা( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
স্থির শূন্য jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  যদি( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }অন্যথা{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
স্থির শূন্য json_Blob_Append_গ্রন্থি(JsonParse*,u8,u64,ধ্রুবক শূন্য*);
স্থির SQLITE_NOINLINE শূন্য json_Blob_Expand_And_Append_গ্রন্থি(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ধ্রুবক শূন্য *aPayload
){
  যদি( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) ফেরত;
  json_Blob_Append_গ্রন্থি(pParse, eType, szPayload, aPayload);
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
স্থির শূন্য json_Blob_Append_গ্রন্থি(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  ধ্রুবক শূন্য *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  যদি( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_গ্রন্থি(pParse,eType,szPayload,aPayload);
    ফেরত;
  }
  প্রতিজ্ঞা( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  যদি( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }অন্যথা যদি( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }অন্যথা যদি( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }অন্যথা{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  যদি( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
স্থির পূর্ণসংখ্যা json_Blob_Change_Payload_আকার(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  পূর্ণসংখ্যা delta;
  যদি( pParse->oom ) ফেরত 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  যদি( szType<=11 ){
    nExtra = 0;
  }অন্যথা যদি( szType==12 ){
    nExtra = 1;
  }অন্যথা যদি( szType==13 ){
    nExtra = 2;
  }অন্যথা যদি( szType==14 ){
    nExtra = 4;
  }অন্যথা{
    nExtra = 8;
  }
  যদি( szPayload<=11 ){
    nNeeded = 0;
  }অন্যথা যদি( szPayload<=0xff ){
    nNeeded = 1;
  }অন্যথা যদি( szPayload<=0xffff ){
    nNeeded = 2;
  }অন্যথা{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  যদি( delta ){
    u64 new_আকার_2 = (u64)pParse->nBlob + delta;
    যদি( delta>0 ){
      যদি( new_আকার_2>pParse->nBlobAlloc && jsonBlobExpand(pParse, new_আকার_2) ){
        ফেরত 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }অন্যথা{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = new_আকার_2;
  }
  যদি( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }অন্যথা যদি( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }অন্যথা যদি( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }অন্যথা{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  ফেরত delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
স্থির পূর্ণসংখ্যা jsonIs4HexB(ধ্রুবক অক্ষর *z, পূর্ণসংখ্যা *pOp){
  যদি( z[0]!='u' ) ফেরত 0;
  যদি( !jsonIs4Hex(&z[1]) ) ফেরত 0;
  *pOp = JSONB_TEXTJ;
  ফেরত 1;
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
স্থির u32 jsonbValidityCheck(
  ধ্রুবক JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ধ্রুবক u8 *z;
  u8 x;
  যদি( iDepth>JSON_MAX_DEPTH ) ফেরত i+1;
  sz = 0;
  n = jsonb_Payload_আকার(pParse, i, &sz);
  যদি( NEVER(n==0) ) ফেরত i+1;          /* Checked by caller */
  যদি( NEVER(i+n+sz!=iEnd) ) ফেরত i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  নির্বাচন( x ){
    ক্ষেত্র JSONB_NULL:
    ক্ষেত্র JSONB_TRUE:
    ক্ষেত্র JSONB_FALSE: {
      ফেরত n+sz==1 ? 0 : i+1;
    }
    ক্ষেত্র JSONB_INT: {
      যদি( sz<1 ) ফেরত i+1;
      j = i+n;
      যদি( z[j]=='-' ){
        j++;
        যদি( sz<2 ) ফেরত i+1;
      }
      k = i+n+sz;
      যখন( j<k ){
        যদি( sqlite3Isdigit(z[j]) ){
          j++;
        }অন্যথা{
          ফেরত j+1;
        }
      }
      ফেরত 0;
    }
    ক্ষেত্র JSONB_INT5: {
      যদি( sz<3 ) ফেরত i+1;
      j = i+n;
      যদি( z[j]=='-' ){
        যদি( sz<4 ) ফেরত i+1;
        j++;
      }
      যদি( z[j]!='0' ) ফেরত i+1;
      যদি( z[j+1]!='x' && z[j+1]!='X' ) ফেরত j+2;
      j += 2;
      k = i+n+sz;
      যখন( j<k ){
        যদি( sqlite3Isxdigit(z[j]) ){
          j++;
        }অন্যথা{
          ফেরত j+1;
        }
      }
      ফেরত 0;
    }
    ক্ষেত্র JSONB_FLOAT:
    ক্ষেত্র JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      যদি( sz<2 ) ফেরত i+1;
      j = i+n;
      k = j+sz;
      যদি( z[j]=='-' ){
        j++;
        যদি( sz<3 ) ফেরত i+1;
      }
      যদি( z[j]=='.' ){
        যদি( x==JSONB_FLOAT ) ফেরত j+1;
        যদি( !sqlite3Isdigit(z[j+1]) ) ফেরত j+1;
        j += 2;
        seen = 1;
      }অন্যথা যদি( z[j]=='0' && x==JSONB_FLOAT ){
        যদি( j+3>k ) ফেরত j+1;
        যদি( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) ফেরত j+1;
        j++;
      }
      জন্য(; j<k; j++){
        যদি( sqlite3Isdigit(z[j]) ) চালু;
        যদি( z[j]=='.' ){
          যদি( seen>0 ) ফেরত j+1;
          যদি( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            ফেরত j+1;
          }
          seen = 1;
          চালু;
        }
        যদি( z[j]=='e' || z[j]=='E' ){
          যদি( seen==2 ) ফেরত j+1;
          যদি( j==k-1 ) ফেরত j+1;
          যদি( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            যদি( j==k-1 ) ফেরত j+1;
          }
          seen = 2;
          চালু;
        }
        ফেরত j+1;
      }
      যদি( seen==0 ) ফেরত i+1;
      ফেরত 0;
    }
    ক্ষেত্র JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      যখন( j<k ){
        যদি( !jsonIsOk[z[j]] && z[j]!='\'' ) ফেরত j+1;
        j++;
      }
      ফেরত 0;
    }
    ক্ষেত্র JSONB_TEXTJ:
    ক্ষেত্র JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      যখন( j<k ){
        যদি( !jsonIsOk[z[j]] && z[j]!='\'' ){
          যদি( z[j]=='"' ){
            যদি( x==JSONB_TEXTJ ) ফেরত j+1;
          }অন্যথা যদি( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            যদি( x==JSONB_TEXTJ ) ফেরত j+1;
          }অন্যথা যদি( NEVER(z[j]!='\\') || j+1>=k ){
            ফেরত j+1;
          }অন্যথা যদি( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }অন্যথা যদি( z[j+1]=='u' ){
            যদি( j+5>=k ) ফেরত j+1;
            যদি( !jsonIs4Hex((ধ্রুবক অক্ষর*)&z[j+2]) ) ফেরত j+1;
            j++;
          }অন্যথা যদি( x!=JSONB_TEXT5 ){
            ফেরত j+1;
          }অন্যথা{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((ধ্রুবক অক্ষর*)&z[j], k-j, &c);
            যদি( c==JSON_INVALID_CHAR ) ফেরত j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      ফেরত 0;
    }
    ক্ষেত্র JSONB_TEXTRAW: {
      ফেরত 0;
    }
    ক্ষেত্র JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      যখন( j<k ){
        sz = 0;
        n = jsonb_Payload_আকার(pParse, j, &sz);
        যদি( n==0 ) ফেরত j+1;
        যদি( j+n+sz>k ) ফেরত j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        যদি( sub ) ফেরত sub;
        j += n + sz;
      }
      প্রতিজ্ঞা( j==k );
      ফেরত 0;
    }
    ক্ষেত্র JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      যখন( j<k ){
        sz = 0;
        n = jsonb_Payload_আকার(pParse, j, &sz);
        যদি( n==0 ) ফেরত j+1;
        যদি( j+n+sz>k ) ফেরত j+1;
        যদি( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          যদি( x<JSONB_TEXT || x>JSONB_TEXTRAW ) ফেরত j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        যদি( sub ) ফেরত sub;
        cnt++;
        j += n + sz;
      }
      প্রতিজ্ঞা( j==k );
      যদি( (cnt & 1)!=0 ) ফেরত j+1;
      ফেরত 0;
    }
    পূর্বনির্ধারিত: {
      ফেরত i+1;
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
স্থির পূর্ণসংখ্যা jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  অক্ষর c;
  u32 j;
  u32 iThis, iStart;
  পূর্ণসংখ্যা x;
  u8 t;
  ধ্রুবক অক্ষর *z = pParse->zJson;
json_parse_restart:
  নির্বাচন( (u8)z[i] ){
  ক্ষেত্র '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_গ্রন্থি(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    যদি( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      ফেরত -1;
    }
    iStart = pParse->nBlob;
    জন্য(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      যদি( x<=0 ){
        পূর্ণসংখ্যা op;
        যদি( x==(-2) ){
          j = pParse->iErr;
          যদি( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          বিরতি;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        যদি( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          পূর্ণসংখ্যা k = j+1;
          যখন( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          প্রতিজ্ঞা( iBlob==pParse->nBlob );
          json_Blob_Append_গ্রন্থি(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }অন্যথা{
          যদি( x!=-1 ) pParse->iErr = j;
          ফেরত -1;
        }
      }
      যদি( pParse->oom ) ফেরত -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      যদি( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        ফেরত -1;
      }
      j = x;
      যদি( z[j]==':' ){
        j++;
      }অন্যথা{
        যদি( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          করো{ j++; }যখন( jsonIsspace(z[j]) );
          যদি( z[j]==':' ){
            j++;
            যাও parse_object_মান;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        যদি( x!=(-5) ){
          যদি( x!=(-1) ) pParse->iErr = j;
          ফেরত -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_মান:
      x = jsonTranslateTextToBlob(pParse, j);
      যদি( x<=0 ){
        যদি( x!=(-1) ) pParse->iErr = j;
        ফেরত -1;
      }
      j = x;
      যদি( z[j]==',' ){
        চালু;
      }অন্যথা যদি( z[j]=='}' ){
        বিরতি;
      }অন্যথা{
        যদি( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          যদি( z[j]==',' ){
            চালু;
          }অন্যথা যদি( z[j]=='}' ){
            বিরতি;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        যদি( x==(-4) ){
          j = pParse->iErr;
          চালু;
        }
        যদি( x==(-2) ){
          j = pParse->iErr;
          বিরতি;
        }
      }
      pParse->iErr = j;
      ফেরত -1;
    }
    json_Blob_Change_Payload_আকার(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    ফেরত j+1;
  }
  ক্ষেত্র '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    প্রতিজ্ঞা( i<=(u32)pParse->nJson );
    json_Blob_Append_গ্রন্থি(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    যদি( pParse->oom ) ফেরত -1;
    যদি( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      ফেরত -1;
    }
    জন্য(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      যদি( x<=0 ){
        যদি( x==(-3) ){
          j = pParse->iErr;
          যদি( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          বিরতি;
        }
        যদি( x!=(-1) ) pParse->iErr = j;
        ফেরত -1;
      }
      j = x;
      যদি( z[j]==',' ){
        চালু;
      }অন্যথা যদি( z[j]==']' ){
        বিরতি;
      }অন্যথা{
        যদি( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          যদি( z[j]==',' ){
            চালু;
          }অন্যথা যদি( z[j]==']' ){
            বিরতি;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        যদি( x==(-4) ){
          j = pParse->iErr;
          চালু;
        }
        যদি( x==(-3) ){
          j = pParse->iErr;
          বিরতি;
        }
      }
      pParse->iErr = j;
      ফেরত -1;
    }
    json_Blob_Change_Payload_আকার(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    ফেরত j+1;
  }
  ক্ষেত্র '\'': {
    u8 opcode;
    অক্ষর cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    যাও parse_string;
  ক্ষেত্র '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    যখন( 1 /*exit-by-break*/ ){
      যদি( jsonIsOk[(u8)z[j]] ){
        যদি( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }অন্যথা যদি( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }অন্যথা{
          j += 3;
          চালু;
        }
      }
      c = z[j];
      যদি( c==cDelim ){
        বিরতি;
      }অন্যথা যদি( c=='\\' ){
        c = z[++j];
        যদি( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          যদি( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }অন্যথা যদি( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#অন্যথা
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }অন্যথা যদি( c=='\r' ){
          যদি( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }অন্যথা{
          pParse->iErr = j;
          ফেরত -1;
        }
      }অন্যথা যদি( c<=0x1f ){
        যদি( c==0 ){
          pParse->iErr = j;
          ফেরত -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }অন্যথা যদি( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_গ্রন্থি(pParse, opcode, j-1-i, &z[i+1]);
    ফেরত j+1;
  }
  ক্ষেত্র 't': {
    যদি( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      ফেরত i+4;
    }
    pParse->iErr = i;
    ফেরত -1;
  }
  ক্ষেত্র 'f': {
    যদি( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      ফেরত i+5;
    }
    pParse->iErr = i;
    ফেরত -1;
  }
  ক্ষেত্র '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    যাও parse_সংখ্যা;
  ক্ষেত্র '.':
    যদি( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      যাও parse_সংখ্যা_2;
    }
    pParse->iErr = i;
    ফেরত -1;
  ক্ষেত্র '-':
  ক্ষেত্র '0':
  ক্ষেত্র '1':
  ক্ষেত্র '2':
  ক্ষেত্র '3':
  ক্ষেত্র '4':
  ক্ষেত্র '5':
  ক্ষেত্র '6':
  ক্ষেত্র '7':
  ক্ষেত্র '8':
  ক্ষেত্র '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_সংখ্যা:
    seenE = 0;
    প্রতিজ্ঞা( '-' < '0' );
    প্রতিজ্ঞা( '+' < '0' );
    প্রতিজ্ঞা( '.' < '0' );
    c = z[i];

    যদি( c<='0' ){
      যদি( c=='0' ){
        যদি( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          প্রতিজ্ঞা( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          জন্য(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          যাও parse_সংখ্যা_finish;
        }অন্যথা যদি( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          ফেরত -1;
        }
      }অন্যথা{
        যদি( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          যদি( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            যদি( z[i]=='-' ){
              json_Blob_Append_গ্রন্থি(pParse, JSONB_FLOAT, 6, "-9e999");
            }অন্যথা{
              json_Blob_Append_গ্রন্থি(pParse, JSONB_FLOAT, 5, "9e999");
            }
            ফেরত i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          যদি( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            যাও parse_সংখ্যা_2;
          }
          pParse->iErr = i;
          ফেরত -1;
        }
        যদি( z[i+1]=='0' ){
          যদি( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            ফেরত -1;
          }অন্যথা যদি( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            জন্য(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            যাও parse_সংখ্যা_finish;
          }
        }
      }
    }

  parse_সংখ্যা_2:
    জন্য(j=i+1;; j++){
      c = z[j];
      যদি( sqlite3Isdigit(c) ) চালু;
      যদি( c=='.' ){
        যদি( (t & 0x02)!=0 ){
          pParse->iErr = j;
          ফেরত -1;
        }
        t |= 0x02;
        চালু;
      }
      যদি( c=='e' || c=='E' ){
        যদি( z[j-1]<'0' ){
          যদি( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }অন্যথা{
            pParse->iErr = j;
            ফেরত -1;
          }
        }
        যদি( seenE ){
          pParse->iErr = j;
          ফেরত -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        যদি( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        যদি( c<'0' || c>'9' ){
          pParse->iErr = j;
          ফেরত -1;
        }
        চালু;
      }
      বিরতি;
    }
    যদি( z[j-1]<'0' ){
      যদি( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }অন্যথা{
        pParse->iErr = j;
        ফেরত -1;
      }
    }
  parse_সংখ্যা_finish:
    প্রতিজ্ঞা( JSONB_INT+0x01==JSONB_INT5 );
    প্রতিজ্ঞা( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    প্রতিজ্ঞা( JSONB_INT+0x02==JSONB_FLOAT );
    যদি( z[i]=='+' ) i++;
    json_Blob_Append_গ্রন্থি(pParse, JSONB_INT+t, j-i, &z[i]);
    ফেরত j;
  }
  ক্ষেত্র '}': {
    pParse->iErr = i;
    ফেরত -2;  /* End of {...} */
  }
  ক্ষেত্র ']': {
    pParse->iErr = i;
    ফেরত -3;  /* End of [...] */
  }
  ক্ষেত্র ',': {
    pParse->iErr = i;
    ফেরত -4;  /* List separator */
  }
  ক্ষেত্র ':': {
    pParse->iErr = i;
    ফেরত -5;  /* Object label/value separator */
  }
  ক্ষেত্র 0: {
    ফেরত 0;   /* End of file */
  }
  ক্ষেত্র 0x09:
  ক্ষেত্র 0x0a:
  ক্ষেত্র 0x0d:
  ক্ষেত্র 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    যাও json_parse_restart;
  }
  ক্ষেত্র 0x0b:
  ক্ষেত্র 0x0c:
  ক্ষেত্র '/':
  ক্ষেত্র 0xc2:
  ক্ষেত্র 0xe1:
  ক্ষেত্র 0xe2:
  ক্ষেত্র 0xe3:
  ক্ষেত্র 0xef: {
    j = json5Whitespace(&z[i]);
    যদি( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      যাও json_parse_restart;
    }
    pParse->iErr = i;
    ফেরত -1;
  }
  ক্ষেত্র 'n': {
    যদি( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      ফেরত i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  পূর্বনির্ধারিত: {
    u32 k;
    পূর্ণসংখ্যা nn;
    c = z[i];
    জন্য(k=0; k<আকার(aNanInfName)/আকার(aNanInfName[0]); k++){
      যদি( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) চালু;
      nn = aNanInfName[k].n;
      যদি( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        চালু;
      }
      যদি( sqlite3Isalnum(z[i+nn]) ) চালু;
      যদি( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_গ্রন্থি(pParse, JSONB_FLOAT, 5, "9e999");
      }অন্যথা{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      ফেরত i + nn;
    }
    pParse->iErr = i;
    ফেরত -1;  /* Syntax error */
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
স্থির পূর্ণসংখ্যা jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  পূর্ণসংখ্যা i;
  ধ্রুবক অক্ষর *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  যদি( pParse->oom ) i = -1;
  যদি( i>0 ){
#ifdef SQLITE_DEBUG
    প্রতিজ্ঞা( pParse->iDepth==0 );
    যদি( sqlite3Config.bJsonSelfcheck ){
      প্রতিজ্ঞা( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    যখন( jsonIsspace(zJson[i]) ) i++;
    যদি( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      যদি( zJson[i] ){
        যদি( pCtx ) sqlite3_ফলাফল_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        ফেরত 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  যদি( i<=0 ){
    যদি( pCtx!=0 ){
      যদি( pParse->oom ){
        sqlite3_ফলাফল_error_nomem(pCtx);
      }অন্যথা{
        sqlite3_ফলাফল_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    ফেরত 1;
  }
  ফেরত 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
স্থির শূন্য jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  প্রতিজ্ঞা( pStr->eErr==0 );
  memset(&px, 0, আকার(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (শূন্য)jsonTranslateTextToBlob(&px, 0);
  যদি( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_ফলাফল_error_nomem(pStr->pCtx);
  }অন্যথা{
    প্রতিজ্ঞা( px.nBlobAlloc>0 );
    প্রতিজ্ঞা( !px.bReadOnly );
    sqlite3_ফলাফল_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
স্থির u32 jsonb_Payload_আকার_Wide(
  ধ্রুবক JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  প্রতিজ্ঞা( i<pParse->nBlob );
  প্রতিজ্ঞা( x==(pParse->aBlob[i]>>4) );
  প্রতিজ্ঞা( x>=13 );
  যদি( x==13 ){
    যদি( i+2>=pParse->nBlob ){
      *pSz = 0;
      ফেরত 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }অন্যথা যদি( x==14 ){
    যদি( i+4>=pParse->nBlob ){
      *pSz = 0;
      ফেরত 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }অন্যথা{
    যদি( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ফেরত 0;
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
  যদি( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    ফেরত 0;
  }
  *pSz = sz;
  ফেরত n;
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
স্থির u32 jsonb_Payload_আকার(
  ধ্রুবক JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  যদি( i>=pParse->nBlob ){
    *pSz = 0;
    ফেরত 0;
  }অন্যথা যদি( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }অন্যথা যদি( x==12 ){
    যদি( i+1>=pParse->nBlob ){
      *pSz = 0;
      ফেরত 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }অন্যথা{
    ফেরত jsonb_Payload_আকার_Wide(pParse, i, pSz, x);
  }
  যদি( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ফেরত 0;
  }
  *pSz = sz;
  ফেরত n;
}

#যদি SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
স্থির SQLITE_INLINE u32 jsonb_Payload_আকার_Inline(
  ধ্রুবক JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  যদি( i>=pParse->nBlob ){
    *pSz = 0;
    ফেরত 0;
  }অন্যথা যদি( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }অন্যথা যদি( x==12 ){
    যদি( i+1>=pParse->nBlob ){
      *pSz = 0;
      ফেরত 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }অন্যথা যদি( x==13 ){
    যদি( i+2>=pParse->nBlob ){
      *pSz = 0;
      ফেরত 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }অন্যথা যদি( x==14 ){
    যদি( i+4>=pParse->nBlob ){
      *pSz = 0;
      ফেরত 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }অন্যথা{
    যদি( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ফেরত 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  যদি( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ফেরত 0;
  }
  *pSz = sz;
  ফেরত n;
}
#অন্যথা /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_আকার_Inline(a,b,c) jsonb_Payload_আকার(a,b,c)
#endif


#যদি SQLITE_USES_INLINE
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
স্থির SQLITE_INLINE u32 jsonb_Payload_আকার_Semi_Inline(
  ধ্রুবক JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  প্রতিজ্ঞা( i<pParse->nBlob );
  যদি( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }অন্যথা যদি( x==12 ){
    যদি( i+1>=pParse->nBlob ){
      *pSz = 0;
      ফেরত 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }অন্যথা{
    ফেরত jsonb_Payload_আকার_Wide(pParse,i,pSz,x);
  }
  যদি( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ফেরত 0;
  }
  *pSz = sz;
  ফেরত n;
}
#অন্যথা /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_আকার_Semi_Inline(a,b,c) jsonb_Payload_আকার(a,b,c)
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
স্থির u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonb_Payload_আকার_Inline(pParse, i, &sz);
  যদি( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    ফেরত pParse->nBlob+1;
  }
  নির্বাচন( pParse->aBlob[i] & 0x0f ){
    ক্ষেত্র JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      ফেরত i+1;
    }
    ক্ষেত্র JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      ফেরত i+1;
    }
    ক্ষেত্র JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      ফেরত i+1;
    }
    ক্ষেত্র JSONB_INT:
    ক্ষেত্র JSONB_FLOAT: {
      যদি( sz==0 ) যাও malformed_jsonb;
      jsonAppendRaw(pOut, (ধ্রুবক অক্ষর*)&pParse->aBlob[i+n], sz);
      বিরতি;
    }
    ক্ষেত্র JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ধ্রুবক অক্ষর *zIn = (ধ্রুবক অক্ষর*)&pParse->aBlob[i+n];
      পূর্ণসংখ্যা bOverflow = 0;
      যদি( sz==0 ) যাও malformed_jsonb;
      যদি( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }অন্যথা যদি( zIn[0]=='+' ){
        k++;
      }
      জন্য(; k<sz; k++){
        যদি( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          বিরতি;
        }অন্যথা যদি( (u>>60)!=0 ){
          bOverflow = 1;
        }অন্যথা{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      বিরতি;
    }
    ক্ষেত্র JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ধ্রুবক অক্ষর *zIn = (ধ্রুবক অক্ষর*)&pParse->aBlob[i+n];
      যদি( sz==0 ) যাও malformed_jsonb;
      যদি( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        যদি( sz<=1 ) যাও malformed_jsonb;
        k = 1;
      }
      যদি( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      জন্য(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        যদি( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      বিরতি;
    }
    ক্ষেত্র JSONB_TEXT:
    ক্ষেত্র JSONB_TEXTJ: {
      যদি( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(ধ্রুবক অক্ষর*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      বিরতি;
    }
    ক্ষেত্র JSONB_TEXT5: {
      ধ্রুবক অক্ষর *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (ধ্রুবক অক্ষর*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      যখন( sz2>0 ){
        জন্য(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        যদি( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          যদি( k>=sz2 ){
            বিরতি;
          }
          zIn += k;
          sz2 -= k;
        }
        যদি( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          চালু;
        }
        যদি( zIn[0]<=0x1f ){
          যদি( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) বিরতি;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          চালু;
        }
        প্রতিজ্ঞা( zIn[0]=='\\' );
        প্রতিজ্ঞা( sz2>=1 );
        যদি( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          বিরতি;
        }
        নির্বাচন( (u8)zIn[1] ){
          ক্ষেত্র '\'':
            jsonAppendChar(pOut, '\'');
            বিরতি;
          ক্ষেত্র 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            বিরতি;
          ক্ষেত্র 'x':
            যদি( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              বিরতি;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            বিরতি;
          ক্ষেত্র '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            বিরতি;
          ক্ষেত্র '\r':
            যদি( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            বিরতি;
          ক্ষেত্র '\n':
            বিরতি;
          ক্ষেত্র 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            যদি( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              বিরতি;
            }
            zIn += 2;
            sz2 -= 2;
            বিরতি;
          পূর্বনির্ধারিত:
            jsonAppendRawNZ(pOut, zIn, 2);
            বিরতি;
        }
        প্রতিজ্ঞা( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      বিরতি;
    }
    ক্ষেত্র JSONB_TEXTRAW: {
      jsonAppendString(pOut, (ধ্রুবক অক্ষর*)&pParse->aBlob[i+n], sz);
      বিরতি;
    }
    ক্ষেত্র JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      যদি( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      যখন( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      যদি( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      যদি( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      বিরতি;
    }
    ক্ষেত্র JSONB_OBJECT: {
      পূর্ণসংখ্যা x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      যদি( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      যখন( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      যদি( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      যদি( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      বিরতি;
    }

    পূর্বনির্ধারিত: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      বিরতি;
    }
  }
  ফেরত i+n+sz;
}

/* Context for recursion of json_pretty()
*/
প্রকারলেখ কাঠামো JsonPretty JsonPretty;
কাঠামো JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  ধ্রুবক অক্ষর *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
স্থির শূন্য jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  জন্য(jj=0; jj<pPretty->nIndent; jj++){
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
স্থির u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonb_Payload_আকার(pParse, i, &sz);
  যদি( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    ফেরত pParse->nBlob+1;
  }
  নির্বাচন( pParse->aBlob[i] & 0x0f ){
    ক্ষেত্র JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      যদি( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        যদি( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        যখন( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          যদি( j>=iEnd ){
            যদি( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            বিরতি;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      বিরতি;
    }
    ক্ষেত্র JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      যদি( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        যদি( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        যখন( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          যদি( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            বিরতি;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          যদি( j>=iEnd ) বিরতি;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      বিরতি;
    }
  }
  ফেরত i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
স্থির u32 jsonb_Array_গুনতি(JsonParse *pParse, u32 i_মূল){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonb_Payload_আকার(pParse, i_মূল, &sz);
  iEnd = i_মূল+n+sz;
  জন্য(i=i_মূল+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonb_Payload_আকার_Semi_Inline(pParse, i, &sz);
  }
  ফেরত k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
স্থির শূন্য json_After_Edit_আকার_Adjust(JsonParse *pParse, u32 i_মূল){
  u32 sz = 0;
  u32 nBlob;
  প্রতিজ্ঞা( pParse->delta!=0 );
  প্রতিজ্ঞা( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (শূন্য)jsonb_Payload_আকার(pParse, i_মূল, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += json_Blob_Change_Payload_আকার(pParse, i_মূল, sz);
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
স্থির পূর্ণসংখ্যা jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  ধ্রুবক u8 *aIns,           /* New content */
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
  স্থির ধ্রুবক u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  যদি( (aIns[0]&0x0f)<=2 ) ফেরত 0;    /* Cannot enlarge NULL, true, false */
  নির্বাচন( aIns[0]>>4 ){
    পূর্বনির্ধারিত: {                         /* aIns[] header size 1 */
      যদি( ((1<<d)&0x116)==0 ) ফেরত 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      বিরতি;
    }
    ক্ষেত্র 12: {                         /* aIns[] header size is 2 */
      যদি( ((1<<d)&0x8a)==0) ফেরত 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      বিরতি;
    }
    ক্ষেত্র 13: {                         /* aIns[] header size is 3 */
      যদি( d!=2 && d!=6 ) ফেরত 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      বিরতি;
    }
    ক্ষেত্র 14: {                         /* aIns[] header size is 5 */
      যদি( d!=4 ) ফেরত 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      বিরতি;
    }
    ক্ষেত্র 15: {                         /* aIns[] header size is 9 */
      ফেরত 0;                          /* No solution */
    }
  }
  প্রতিজ্ঞা( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  যখন( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    যদি( i==1 ) বিরতি;
    szPayload >>= 8;
  }
  প্রতিজ্ঞা( (szPayload>>8)==0 );
  ফেরত 1;
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
স্থির শূন্য jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  ধ্রুবক u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  প্রতিজ্ঞা( pParse->nBlob >= (u64)iDel + (u64)nDel );
  যদি( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (পূর্ণসংখ্যা)-d)
  ){
    ফেরত;
  }
  যদি( d!=0 ){
    যদি( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      যদি( pParse->oom ) ফেরত;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  যদি( nIns && aIns ){
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
স্থির u32 jsonBytesToBypass(ধ্রুবক অক্ষর *z, u32 n){
  u32 i = 0;
  যখন( i+1<n ){
    যদি( z[i]!='\\' ) ফেরত i;
    যদি( z[i+1]=='\n' ){
      i += 2;
      চালু;
    }
    যদি( z[i+1]=='\r' ){
      যদি( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }অন্যথা{
        i += 2;
      }
      চালু;
    }
    যদি( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      চালু;
    }
    বিরতি;
  }
  ফেরত i;
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
স্থির u32 jsonUnescapeOneChar(ধ্রুবক অক্ষর *z, u32 n, u32 *piOut){
  প্রতিজ্ঞা( n>0 );
  প্রতিজ্ঞা( z[0]=='\\' );
  যদি( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    ফেরত n;
  }
  নির্বাচন( (u8)z[1] ){
    ক্ষেত্র 'u': {
      u32 v, vlo;
      যদি( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        ফেরত n;
      }
      v = jsonHexToInt4(&z[2]);
      যদি( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        ফেরত 12;
      }অন্যথা{
        *piOut = v;
        ফেরত 6;
      }
    }
    ক্ষেত্র 'b': {   *piOut = '\b';  ফেরত 2; }
    ক্ষেত্র 'f': {   *piOut = '\f';  ফেরত 2; }
    ক্ষেত্র 'n': {   *piOut = '\n';  ফেরত 2; }
    ক্ষেত্র 'r': {   *piOut = '\r';  ফেরত 2; }
    ক্ষেত্র 't': {   *piOut = '\t';  ফেরত 2; }
    ক্ষেত্র 'v': {   *piOut = '\v';  ফেরত 2; }
    ক্ষেত্র '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#অন্যথা
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      ফেরত 2;
    }
    ক্ষেত্র '\'':
    ক্ষেত্র '"':
    ক্ষেত্র '/':
    ক্ষেত্র '\\':{   *piOut = z[1];  ফেরত 2; }
    ক্ষেত্র 'x': {
      যদি( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        ফেরত n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      ফেরত 4;
    }
    ক্ষেত্র 0xe2:
    ক্ষেত্র '\r':
    ক্ষেত্র '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      যদি( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        ফেরত n;
      }অন্যথা যদি( nSkip==n ){
        *piOut = 0;
        ফেরত n;
      }অন্যথা যদি( z[nSkip]=='\\' ){
        ফেরত nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }অন্যথা{
        পূর্ণসংখ্যা sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        ফেরত nSkip + sz;
      }
    }
    পূর্বনির্ধারিত: {
      *piOut = JSON_INVALID_CHAR;
      ফেরত 2;
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
স্থির SQLITE_NOINLINE পূর্ণসংখ্যা jsonLabelCompareEscaped(
  ধ্রুবক অক্ষর *z_বাম,          /* The left label */
  u32 n_বাম,                  /* Size of the left label in bytes */
  পূর্ণসংখ্যা raw_বাম,                /* True if zLeft contains no escapes */
  ধ্রুবক অক্ষর *z_ডান,         /* The right label */
  u32 n_ডান,                 /* Size of the right label in bytes */
  পূর্ণসংখ্যা raw_ডান                /* True if zRight is escape-free */
){
  u32 c_বাম, c_ডান;
  প্রতিজ্ঞা( raw_বাম==0 || raw_ডান==0 );
  যখন( 1 /*exit-by-return*/ ){
    যদি( n_বাম==0 ){
      c_বাম = 0;
    }অন্যথা যদি( raw_বাম || z_বাম[0]!='\\' ){
      c_বাম = ((u8*)z_বাম)[0];
      যদি( c_বাম>=0xc0 ){
        পূর্ণসংখ্যা sz = sqlite3Utf8ReadLimited((u8*)z_বাম, n_বাম, &c_বাম);
        z_বাম += sz;
        n_বাম -= sz;
      }অন্যথা{
        z_বাম++;
        n_বাম--;
      }
    }অন্যথা{
      u32 n = jsonUnescapeOneChar(z_বাম, n_বাম, &c_বাম);
      z_বাম += n;
      প্রতিজ্ঞা( n<=n_বাম );
      n_বাম -= n;
    }
    যদি( n_ডান==0 ){
      c_ডান = 0;
    }অন্যথা যদি( raw_ডান || z_ডান[0]!='\\' ){
      c_ডান = ((u8*)z_ডান)[0];
      যদি( c_ডান>=0xc0 ){
        পূর্ণসংখ্যা sz = sqlite3Utf8ReadLimited((u8*)z_ডান, n_ডান, &c_ডান);
        z_ডান += sz;
        n_ডান -= sz;
      }অন্যথা{
        z_ডান++;
        n_ডান--;
      }
    }অন্যথা{
      u32 n = jsonUnescapeOneChar(z_ডান, n_ডান, &c_ডান);
      z_ডান += n;
      প্রতিজ্ঞা( n<=n_ডান );
      n_ডান -= n;
    }
    যদি( c_বাম!=c_ডান ) ফেরত 0;
    যদি( c_বাম==0 ) ফেরত 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
স্থির পূর্ণসংখ্যা jsonLabelCompare(
  ধ্রুবক অক্ষর *z_বাম,          /* The left label */
  u32 n_বাম,                  /* Size of the left label in bytes */
  পূর্ণসংখ্যা raw_বাম,                /* True if zLeft contains no escapes */
  ধ্রুবক অক্ষর *z_ডান,         /* The right label */
  u32 n_ডান,                 /* Size of the right label in bytes */
  পূর্ণসংখ্যা raw_ডান                /* True if zRight is escape-free */
){
  যদি( raw_বাম && raw_ডান ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    যদি( n_বাম!=n_ডান ) ফেরত 0;
    ফেরত memcmp(z_বাম, z_ডান, n_বাম)==0;
  }অন্যথা{
    ফেরত jsonLabelCompareEscaped(z_বাম, n_বাম, raw_বাম,
                                   z_ডান, n_ডান, raw_ডান);
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
স্থির u32 jsonLookupStep(JsonParse*,u32,ধ্রুবক অক্ষর*,u32);


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
স্থির u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  ধ্রুবক অক্ষর *z_লেজ   /* Tail of the path that determines substructure */
){
  স্থির ধ্রুবক u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  পূর্ণসংখ্যা rc;
  memset(pIns, 0, আকার(*pIns));
  pIns->db = pParse->db;
  যদি( z_লেজ[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }অন্যথা{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[z_লেজ[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    যদি( pIns->iDepth >= JSON_MAX_DEPTH ){
      ফেরত JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, z_লেজ, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  ফেরত rc;  /* Error code only */
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
স্থির u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_মূল,              /* Begin the search at this element of aBlob[] */
  ধ্রুবক অক্ষর *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_চাবি, sz, n, iEnd, rc;
  ধ্রুবক অক্ষর *z_চাবি;
  u8 x;

  যদি( zPath[0]==0 ){
    যদি( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonb_Payload_আকার(pParse, i_মূল, &sz);
      sz += n;
      যদি( pParse->eEdit==JEDIT_DEL ){
        যদি( iLabel>0 ){
          sz += i_মূল - iLabel;
          i_মূল = iLabel;
        }
        jsonBlobEdit(pParse, i_মূল, sz, 0, 0);
      }অন্যথা যদি( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }অন্যথা যদি( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        যদি( zPath[-1]!=']' ){
          ফেরত JSON_LOOKUP_NOTARRAY;
        }অন্যথা{
          jsonBlobEdit(pParse, i_মূল, 0, pParse->aIns, pParse->nIns);
        }
      }অন্যথা{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_মূল, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    ফেরত i_মূল;
  }
  যদি( zPath[0]=='.' ){
    পূর্ণসংখ্যা raw_চাবি = 1;
    x = pParse->aBlob[i_মূল];
    zPath++;
    যদি( zPath[0]=='"' ){
      z_চাবি = zPath + 1;
      জন্য(i=1; zPath[i] && zPath[i]!='"'; i++){
        যদি( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_চাবি = i-1;
      যদি( zPath[i] ){
        i++;
      }অন্যথা{
        ফেরত JSON_LOOKUP_PATHERROR;
      }
      testcase( n_চাবি==0 );
      raw_চাবি = memchr(z_চাবি, '\\', n_চাবি)==0;
    }অন্যথা{
      z_চাবি = zPath;
      জন্য(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_চাবি = i;
      যদি( n_চাবি==0 ){
        ফেরত JSON_LOOKUP_PATHERROR;
      }
    }
    যদি( (x & 0x0f)!=JSONB_OBJECT ) ফেরত JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_আকার(pParse, i_মূল, &sz);
    j = i_মূল + n;  /* j is the index of a label */
    iEnd = j+sz;
    যখন( j<iEnd ){
      পূর্ণসংখ্যা rawLabel;
      ধ্রুবক অক্ষর *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      যদি( x<JSONB_TEXT || x>JSONB_TEXTRAW ) ফেরত JSON_LOOKUP_ERROR;
      n = jsonb_Payload_আকার_Semi_Inline(pParse, j, &sz);
      যদি( n==0 ) ফেরত JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      যদি( k+sz>=iEnd ) ফেরত JSON_LOOKUP_ERROR;
      zLabel = (ধ্রুবক অক্ষর*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      যদি( jsonLabelCompare(z_চাবি, n_চাবি, raw_চাবি, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        যদি( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) ফেরত JSON_LOOKUP_ERROR;
        n = jsonb_Payload_আকার(pParse, v, &sz);
        যদি( n==0 || v+n+sz>iEnd ) ফেরত JSON_LOOKUP_ERROR;
        প্রতিজ্ঞা( j>0 );
        যদি( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          ফেরত JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        যদি( pParse->delta ) json_After_Edit_আকার_Adjust(pParse, i_মূল);
        ফেরত rc;
      }
      j = k+sz;
      যদি( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) ফেরত JSON_LOOKUP_ERROR;
      n = jsonb_Payload_আকার_Semi_Inline(pParse, j, &sz);
      যদি( n==0 ) ফেরত JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    যদি( j>iEnd ) ফেরত JSON_LOOKUP_ERROR;
    যদি( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      যদি( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        ফেরত JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, আকার(ix));
      ix.db = pParse->db;
      json_Blob_Append_গ্রন্থি(&ix, raw_চাবি?JSONB_TEXTRAW:JSONB_TEXT5, n_চাবি, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      যদি( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_চাবি+v.nBlob)
      ){
        প্রতিজ্ঞা( !pParse->oom );
        nIns = ix.nBlob + n_চাবি + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        যদি( !pParse->oom ){
          প্রতিজ্ঞা( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          প্রতিজ্ঞা( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], z_চাবি, n_চাবি);
          k += n_চাবি;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          যদি( ALWAYS(pParse->delta) ) json_After_Edit_আকার_Adjust(pParse, i_মূল);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      ফেরত rc;
    }
  }অন্যথা যদি( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_মূল] & 0x0f;
    যদি( x!=JSONB_ARRAY )  ফেরত JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_আকার(pParse, i_মূল, &sz);
    i = 1;
    যখন( sqlite3Isdigit(zPath[i]) ){
      যদি( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    যদি( i<2 || zPath[i]!=']' ){
      যদি( zPath[1]=='#' ){
        kk = jsonb_Array_গুনতি(pParse, i_মূল);
        i = 2;
        যদি( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          করো{
            যদি( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }যখন( sqlite3Isdigit(zPath[i]) );
          যদি( nn>kk ) ফেরত JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        যদি( zPath[i]!=']' ){
          ফেরত JSON_LOOKUP_PATHERROR;
        }
      }অন্যথা{
        ফেরত JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_মূল+n;
    iEnd = j+sz;
    যখন( j<iEnd ){
      যদি( kk==0 ){
        যদি( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          ফেরত JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        যদি( pParse->delta ) json_After_Edit_আকার_Adjust(pParse, i_মূল);
        ফেরত rc;
      }
      kk--;
      n = jsonb_Payload_আকার_Semi_Inline(pParse, j, &sz);
      যদি( n==0 ) ফেরত JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    যদি( j>iEnd ) ফেরত JSON_LOOKUP_ERROR;
    যদি( kk>0 ) ফেরত JSON_LOOKUP_NOTFOUND;
    যদি( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      যদি( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        প্রতিজ্ঞা( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      যদি( pParse->delta ) json_After_Edit_আকার_Adjust(pParse, i_মূল);
      ফেরত rc;
    }
  }অন্যথা{
    ফেরত JSON_LOOKUP_PATHERROR; 
  }
  ফেরত JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
স্থির শূন্য jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  ধ্রুবক u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  যদি( NEVER(aBlob==0) ) ফেরত;
  memset(&x, 0, আকার(x));
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
স্থির শূন্য jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  পূর্ণসংখ্যা eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  পূর্ণসংখ্যা rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  প্রতিজ্ঞা( eMode>=0 && eMode<=2 );
  n = jsonb_Payload_আকার(pParse, i, &sz);
  যদি( n==0 ){
    sqlite3_ফলাফল_error(pCtx, "malformed JSON", -1);
    ফেরত;
  }
  নির্বাচন( pParse->aBlob[i] & 0x0f ){
    ক্ষেত্র JSONB_NULL: {
      যদি( sz ) যাও returnfromblob_malformed;
      sqlite3_ফলাফল_null(pCtx);
      বিরতি;
    }
    ক্ষেত্র JSONB_TRUE: {
      যদি( sz ) যাও returnfromblob_malformed;
      sqlite3_ফলাফল_int(pCtx, 1);
      বিরতি;
    }
    ক্ষেত্র JSONB_FALSE: {
      যদি( sz ) যাও returnfromblob_malformed;
      sqlite3_ফলাফল_int(pCtx, 0);
      বিরতি;
    }
    ক্ষেত্র JSONB_INT5:
    ক্ষেত্র JSONB_INT: {
      sqlite3_int64 iRes = 0;
      অক্ষর *z;
      পূর্ণসংখ্যা bNeg = 0;
      অক্ষর x;
      যদি( sz==0 ) যাও returnfromblob_malformed;
      x = (অক্ষর)pParse->aBlob[i+n];
      যদি( x=='-' ){
        যদি( sz<2 ) যাও returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (ধ্রুবক অক্ষর*)&pParse->aBlob[i+n], (পূর্ণসংখ্যা)sz);
      যদি( z==0 ) যাও returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      যদি( rc==0 ){
        যদি( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          দ্বিগুণ r;
          r = (দ্বিগুণ)*(sqlite3_uint64*)&iRes;
          sqlite3_ফলাফল_double(pCtx, bNeg ? -r : r);
        }অন্যথা{
          sqlite3_ফলাফল_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }অন্যথা যদি( rc==3 && bNeg ){
        sqlite3_ফলাফল_int64(pCtx, SMALLEST_INT64);
      }অন্যথা যদি( rc==1 ){
        যাও returnfromblob_malformed;
      }অন্যথা{
        যদি( bNeg ){ n--; sz++; }
        যাও to_double;
      }
      বিরতি;
    }
    ক্ষেত্র JSONB_FLOAT5:
    ক্ষেত্র JSONB_FLOAT: {
      দ্বিগুণ r;
      অক্ষর *z;
      যদি( sz==0 ) যাও returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (ধ্রুবক অক্ষর*)&pParse->aBlob[i+n], (পূর্ণসংখ্যা)sz);
      যদি( z==0 ) যাও returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      যদি( rc<=0 ) যাও returnfromblob_malformed;
      sqlite3_ফলাফল_double(pCtx, r);
      বিরতি;
    }
    ক্ষেত্র JSONB_TEXTRAW:
    ক্ষেত্র JSONB_TEXT: {
      sqlite3_ফলাফল_text(pCtx, (অক্ষর*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      বিরতি;
    }
    ক্ষেত্র JSONB_TEXT5:
    ক্ষেত্র JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      ধ্রুবক অক্ষর *z;
      অক্ষর *zOut;
      u32 nOut = sz;
      z = (ধ্রুবক অক্ষর*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      যদি( zOut==0 ) যাও returnfromblob_oom;
      জন্য(iIn=iOut=0; iIn<sz; iIn++){
        অক্ষর c = z[iIn];
        যদি( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          যদি( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }অন্যথা যদি( v<=0x7f ){
            zOut[iOut++] = (অক্ষর)v;
          }অন্যথা যদি( v<=0x7ff ){
            প্রতিজ্ঞা( szEscape>=2 );
            zOut[iOut++] = (অক্ষর)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }অন্যথা যদি( v<0x10000 ){
            প্রতিজ্ঞা( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }অন্যথা যদি( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }অন্যথা{
            প্রতিজ্ঞা( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }অন্যথা{
          zOut[iOut++] = c;
        }
      } /* end for() */
      প্রতিজ্ঞা( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_ফলাফল_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      বিরতি;
    }
    ক্ষেত্র JSONB_ARRAY:
    ক্ষেত্র JSONB_OBJECT: {
      যদি( eMode==0 ){
        যদি( (SQLITE_PTR_TO_INT(sqlite3_user_তথ্য(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }অন্যথা{
          eMode = 1;
        }
      }
      যদি( eMode==2 ){
        sqlite3_ফলাফল_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }অন্যথা{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      যাও returnfromblob_malformed;
    }
  }
  ফেরত;

returnfromblob_oom:
  sqlite3_ফলাফল_error_nomem(pCtx);
  ফেরত;

returnfromblob_malformed:
  sqlite3_ফলাফল_error(pCtx, "malformed JSON", -1);
  ফেরত;
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
স্থির পূর্ণসংখ্যা jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_মান *pArg,
  JsonParse *pParse
){
  পূর্ণসংখ্যা eType = sqlite3_মান_type(pArg);
  স্থির u8 aNull[] = { 0x00 };
  memset(pParse, 0, আকার(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  নির্বাচন( eType ){
    পূর্বনির্ধারিত: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      ফেরত 0;
    }
    ক্ষেত্র SQLITE_BLOB: {
      যদি( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_ফলাফল_error(ctx, "JSON cannot hold BLOB values", -1);
        ফেরত 1;
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_TEXT: {
      ধ্রুবক অক্ষর *zJson = (ধ্রুবক অক্ষর*)sqlite3_মান_text(pArg);
      পূর্ণসংখ্যা nJson = sqlite3_মান_bytes(pArg);
      যদি( zJson==0 ) ফেরত 1;
      যদি( sqlite3_মান_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (অক্ষর*)zJson;
        pParse->nJson = nJson;
        যদি( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_ফলাফল_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, আকার(pParse[0]));
          ফেরত 1;
        }
      }অন্যথা{
        json_Blob_Append_গ্রন্থি(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_FLOAT: {
      যদি( NEVER(sqlite3IsNaN(sqlite3_মান_double(pArg))) ){
        json_Blob_Append_গ্রন্থি(pParse, JSONB_NULL, 0, 0);
      }অন্যথা{
        পূর্ণসংখ্যা n = sqlite3_মান_bytes(pArg);
        ধ্রুবক অক্ষর *z = (ধ্রুবক অক্ষর*)sqlite3_মান_text(pArg);
        যদি( z==0 ) ফেরত 1;
        যদি( z[0]=='I' ){
          json_Blob_Append_গ্রন্থি(pParse, JSONB_FLOAT, 5, "9e999");
        }অন্যথা যদি( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_গ্রন্থি(pParse, JSONB_FLOAT, 6, "-9e999");
        }অন্যথা{
          json_Blob_Append_গ্রন্থি(pParse, JSONB_FLOAT, n, z);
        }
      }
      বিরতি;
    }
    ক্ষেত্র SQLITE_INTEGER: {
      পূর্ণসংখ্যা n = sqlite3_মান_bytes(pArg);
      ধ্রুবক অক্ষর *z = (ধ্রুবক অক্ষর*)sqlite3_মান_text(pArg);
      যদি( z==0 ) ফেরত 1;
      json_Blob_Append_গ্রন্থি(pParse, JSONB_INT, n, z);
      বিরতি;
    }
  }
  যদি( pParse->oom ){
    sqlite3_ফলাফল_error_nomem(ctx);
    ফেরত 1;
  }অন্যথা{
    ফেরত 0;
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
স্থির অক্ষর *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  ধ্রুবক অক্ষর *zPath,        /* The path with the problem */
  পূর্ণসংখ্যা rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  অক্ষর *z_বার্তা;
  যদি( rc==(পূর্ণসংখ্যা)JSON_LOOKUP_NOTARRAY ){
    z_বার্তা = sqlite3_mprintf("not an array element: %Q", zPath);
  }অন্যথা যদি( rc==(পূর্ণসংখ্যা)JSON_LOOKUP_ERROR ){
    z_বার্তা = sqlite3_mprintf("malformed JSON");
  }অন্যথা যদি( rc==(পূর্ণসংখ্যা)JSON_LOOKUP_TOODEEP ){
    z_বার্তা = sqlite3_mprintf("JSON path too deep");
  }অন্যথা{
    z_বার্তা = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  যদি( ctx==0 ) ফেরত z_বার্তা;
  যদি( z_বার্তা ){
    sqlite3_ফলাফল_error(ctx, z_বার্তা, -1);
    sqlite3_free(z_বার্তা);
  }অন্যথা{
    sqlite3_ফলাফল_error_nomem(ctx);
  }
  ফেরত 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
স্থির শূন্য json_ঢোকাও_Into_Blob(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv,
  পূর্ণসংখ্যা eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  পূর্ণসংখ্যা i;
  u32 rc = 0;
  ধ্রুবক অক্ষর *zPath = 0;
  পূর্ণসংখ্যা flgs;
  JsonParse *p;
  JsonParse ax;

  প্রতিজ্ঞা( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  যদি( p==0 ) ফেরত;
  জন্য(i=1; i<argc-1; i+=2){
    যদি( sqlite3_মান_type(argv[i])==SQLITE_NULL ) চালু;
    zPath = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
    যদি( zPath==0 ){
      sqlite3_ফলাফল_error_nomem(ctx);
      jsonParseFree(p);
      ফেরত;
    }
    যদি( zPath[0]!='$' ) যাও json_ঢোকাও_Into_Blob_patherror;
    যদি( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      ফেরত;
    }
    যদি( zPath[1]==0 ){
      যদি( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }অন্যথা{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    যদি( rc==JSON_LOOKUP_NOTFOUND ) চালু;
    যদি( JSON_LOOKUP_ISERROR(rc) ) যাও json_ঢোকাও_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ফেরত;

json_ঢোকাও_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  ফেরত;
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
স্থির পূর্ণসংখ্যা jsonArgIsJsonb(sqlite3_মান *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  যদি( sqlite3_মান_type(pArg)!=SQLITE_BLOB ) ফেরত 0;
  p->aBlob = (u8*)sqlite3_মান_blob(pArg);
  p->nBlob = (u32)sqlite3_মান_bytes(pArg);
  যদি( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonb_Payload_আকার(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    ফেরত 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  ফেরত 0;
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
স্থির JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_মান *pArg,
  u32 flgs
){
  পূর্ণসংখ্যা eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  প্রতিজ্ঞা( ctx!=0 );
  eType = sqlite3_মান_type(pArg);
  যদি( eType==SQLITE_NULL ){
    ফেরত 0;
  }
  যদি( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_অনুসন্ধান(ctx, pArg);
  }
  যদি( pFromCache ){
    pFromCache->nJPRef++;
    যদি( (flgs & JSON_EDITABLE)==0 ){
      ফেরত pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, আকার(*p));
  যদি( p==0 ) যাও json_pfa_oom;
  memset(p, 0, আকার(*p));
  p->db = db;
  p->nJPRef = 1;
  যদি( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    যদি( p->aBlob==0 ) যাও json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    ফেরত p;
  }
  যদি( eType==SQLITE_BLOB ){
    যদি( jsonArgIsJsonb(pArg,p) ){
      যদি( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        যাও json_pfa_oom;
      }
      ফেরত p;
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
  p->zJson = (অক্ষর*)sqlite3_মান_text(pArg);
  p->nJson = sqlite3_মান_bytes(pArg);
  যদি( db->mallocFailed ) যাও json_pfa_oom;
  যদি( p->nJson==0 ) যাও json_pfa_malformed;
  প্রতিজ্ঞা( p->zJson!=0 );
  যদি( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    যদি( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      ফেরত p;
    }অন্যথা{
      jsonParseFree(p);
      ফেরত 0;
    }
  }অন্যথা{
    পূর্ণসংখ্যা isRCStr = sqlite3_মান_Is_Of_Class(pArg, sqlite3RCStrUnref);
    পূর্ণসংখ্যা rc;
    যদি( !isRCStr ){
      অক্ষর *zNew = sqlite3RCStrNew( p->nJson );
      যদি( zNew==0 ) যাও json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }অন্যথা{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_ঢোকাও(ctx, p);
    যদি( rc==SQLITE_NOMEM ) যাও json_pfa_oom;
    যদি( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      যাও rebuild_from_cache;
    }
  }
  ফেরত p;

json_pfa_malformed:
  যদি( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    ফেরত p;
  }অন্যথা{
    jsonParseFree(p);
    sqlite3_ফলাফল_error(ctx, "malformed JSON", -1);
    ফেরত 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_ফলাফল_error_nomem(ctx);
  ফেরত 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
স্থির শূন্য jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  পূর্ণসংখ্যা flgs;
  যদি( p->oom ){
    sqlite3_ফলাফল_error_nomem(ctx);
    ফেরত;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_তথ্য(ctx));
  যদি( flgs & JSON_BLOB ){
    যদি( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_ফলাফল_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }অন্যথা{
      sqlite3_ফলাফল_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }অন্যথা{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    jsonTranslateBlobToText(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
  }
}

/****************************************************************************
** SQL functions used for testing and debugging
****************************************************************************/

#যদি SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
স্থির শূন্য jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  পূর্ণসংখ্যা nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  যখন( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    পূর্ণসংখ্যা showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    যদি( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonb_Payload_আকার(pParse, iStart, &sz);
    যদি( nn==0 ) nn = 1;
    যদি( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    জন্য(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    যদি( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      চালু;
    }
    pParse->nBlob = savedNBlob;
    যদি( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      যদি( iEnd>pParse->nBlob ){
        যদি( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }অন্যথা{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    নির্বাচন( x ){
      ক্ষেত্র JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); বিরতি;
      ক্ষেত্র JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); বিরতি;
      ক্ষেত্র JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); বিরতি;
      ক্ষেত্র JSONB_INT:      sqlite3_str_appendall(pOut,"int"); বিরতি;
      ক্ষেত্র JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); বিরতি;
      ক্ষেত্র JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); বিরতি;
      ক্ষেত্র JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); বিরতি;
      ক্ষেত্র JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); বিরতি;
      ক্ষেত্র JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); বিরতি;
      ক্ষেত্র JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); বিরতি;
      ক্ষেত্র JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); বিরতি;
      ক্ষেত্র JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        বিরতি;
      }
      ক্ষেত্র JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        বিরতি;
      }
      পূর্বনির্ধারিত: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        বিরতি;
      }
    }
    যদি( showContent ){
      যদি( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }অন্যথা{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        জন্য(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          যদি( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (অক্ষর*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
স্থির শূন্য jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  অক্ষর zBuf[1000];
  যদি( pParse==0 ){
    printf("NULL pointer\n");
    ফেরত;
  }অন্যথা{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    যদি( pParse->nBlob==0 ) ফেরত;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, আকার(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_মান(&out));
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
স্থির শূন্য jsonParseFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  প্রতিজ্ঞা( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  যদি( p==0 ) ফেরত;
  যদি( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_ফলাফল_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }অন্যথা{
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
স্থির শূন্য jsonQuoteFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_মান(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
স্থির শূন্য jsonArrayFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  জন্য(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_মান(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
স্থির শূন্য json_Array_দৈর্ঘ্য_Func(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  যদি( p==0 ) ফেরত;
  যদি( argc==2 ){
    ধ্রুবক অক্ষর *zPath = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[1]);
    যদি( zPath==0 ){
      jsonParseFree(p);
      ফেরত;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    যদি( JSON_LOOKUP_ISERROR(i) ){
      যদি( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }অন্যথা{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }অন্যথা{
    i = 0;
  }
  যদি( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_গুনতি(p, i);
  }
  যদি( !eErr ) sqlite3_ফলাফল_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
স্থির পূর্ণসংখ্যা jsonAllAlphanum(ধ্রুবক অক্ষর *z, পূর্ণসংখ্যা n){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  ফেরত i==n;
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
স্থির শূন্য jsonExtractFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonParse *p = 0;      /* The parse */
  পূর্ণসংখ্যা flags;             /* Flags associated with the function */
  পূর্ণসংখ্যা i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  যদি( argc<2 ) ফেরত;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  যদি( p==0 ) ফেরত;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_তথ্য(ctx));
  jsonStringInit(&jx, ctx);
  যদি( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  জন্য(i=1; i<argc; i++){
    /* With a single PATH argument */
    ধ্রুবক অক্ষর *zPath = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
    পূর্ণসংখ্যা nPath;
    u32 j;
    যদি( zPath==0 ) যাও json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    যদি( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }অন্যথা যদি( (flags & JSON_ABPATH) ){
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
      যদি( sqlite3_মান_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        যদি( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }অন্যথা যদি( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }অন্যথা যদি( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }অন্যথা{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }অন্যথা{
      jsonBadPathError(ctx, zPath, 0);
      যাও json_extract_error;
    }
    যদি( j<p->nBlob ){
      যদি( argc==2 ){
        যদি( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          প্রতিজ্ঞা( (flags & JSON_BLOB)==0 );
          sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
        }অন্যথা{
          jsonReturnFromBlob(p, j, ctx, 0);
          যদি( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }অন্যথা{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }অন্যথা যদি( j==JSON_LOOKUP_NOTFOUND ){
      যদি( argc==2 ){
        যাও json_extract_error;  /* Return NULL if not found */
      }অন্যথা{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }অন্যথা{
      jsonBadPathError(ctx, zPath, j);
      যাও json_extract_error;
    }
  }
  যদি( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    যদি( (flags & JSON_BLOB)==0 ){
      sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  ফেরত;
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
স্থির পূর্ণসংখ্যা jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  ধ্রুবক JsonParse *pPatch, /* The PATCH */
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

  প্রতিজ্ঞা( iTarget>=0 && iTarget<pTarget->nBlob );
  প্রতিজ্ঞা( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  যদি( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonb_Payload_আকার(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonb_Payload_আকার(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    ফেরত pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  যদি( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonb_Payload_আকার(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonb_Payload_আকার(pPatch, iPatch, &sz);
  যদি( NEVER(n==0) ) ফেরত JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonb_Payload_আকার(pTarget, iTarget, &sz);
  যদি( NEVER(n==0) ) ফেরত JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  যখন( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    যদি( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      ফেরত JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonb_Payload_আকার(pPatch, iPCursor, &szPLabel);
    যদি( nPLabel==0 ) ফেরত JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    যদি( iPValue>=iPEnd ) ফেরত JSON_MERGE_BADPATCH;
    nPValue = jsonb_Payload_আকার(pPatch, iPValue, &szPValue);
    যদি( nPValue==0 ) ফেরত JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    যদি( iPCursor>iPEnd ) ফেরত JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    যখন( iTCursor<iTEnd ){
      পূর্ণসংখ্যা isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      যদি( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        ফেরত JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonb_Payload_আকার(pTarget, iTCursor, &szTLabel);
      যদি( nTLabel==0 ) ফেরত JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      যদি( iTValue>=iTEnd ) ফেরত JSON_MERGE_BADTARGET;
      nTValue = jsonb_Payload_আকার(pTarget, iTValue, &szTValue);
      যদি( nTValue==0 ) ফেরত JSON_MERGE_BADTARGET;
      যদি( iTValue + nTValue + szTValue > iTEnd ) ফেরত JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ধ্রুবক অক্ষর*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (ধ্রুবক অক্ষর*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      যদি( isEqual ) বিরতি;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    যদি( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      যদি( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        যদি( NEVER(pTarget->oom) ) ফেরত JSON_MERGE_OOM;
      }অন্যথা{
        /* Algorithm line 12 */
        পূর্ণসংখ্যা rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        যদি( iDepth>=JSON_MAX_DEPTH ) ফেরত JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        যদি( rc ) ফেরত rc;
        pTarget->delta += savedDelta;
      }        
    }অন্যথা যদি( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      যদি( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        যদি( pTarget->oom ) ফেরত JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }অন্যথা{
        পূর্ণসংখ্যা rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        যদি( pTarget->oom ) ফেরত JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        যদি( iDepth>=JSON_MAX_DEPTH ) ফেরত JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        যদি( rc ) ফেরত rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  যদি( pTarget->delta ) json_After_Edit_আকার_Adjust(pTarget, iTarget);
  ফেরত pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
স্থির শূন্য jsonPatchFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  পূর্ণসংখ্যা rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  প্রতিজ্ঞা( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  যদি( pTarget==0 ) ফেরত;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  যদি( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    যদি( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }অন্যথা যদি( rc==JSON_MERGE_OOM ){
      sqlite3_ফলাফল_error_nomem(ctx);
    }অন্যথা যদি( rc==JSON_MERGE_TOODEEP ){
      sqlite3_ফলাফল_error(ctx, "JSON nested too deep", -1);
    }অন্যথা{
      sqlite3_ফলাফল_error(ctx, "malformed JSON", -1);
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
স্থির শূন্য jsonObjectFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা i;
  JsonString jx;
  ধ্রুবক অক্ষর *z;
  u32 n;

  যদি( argc&1 ){
    sqlite3_ফলাফল_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    ফেরত;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  জন্য(i=0; i<argc; i+=2){
    যদি( sqlite3_মান_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_ফলাফল_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      ফেরত;
    }
    jsonAppendSeparator(&jx);
    z = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
    n = sqlite3_মান_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_মান(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
}


/*
** json_remove(JSON, PATH, ...)
**
** Remove the named elements from JSON and return the result.  malformed
** JSON or PATH arguments result in an error.
*/
স্থির শূন্য jsonRemoveFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonParse *p;          /* The parse */
  ধ্রুবক অক্ষর *zPath = 0; /* Path of element to be removed */
  পূর্ণসংখ্যা i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  যদি( argc<1 ) ফেরত;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  যদি( p==0 ) ফেরত;
  জন্য(i=1; i<argc; i++){
    zPath = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
    যদি( zPath==0 ){
      যাও json_remove_done;
    }
    যদি( zPath[0]!='$' ){
      যাও json_remove_patherror;
    }
    যদি( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      যাও json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    যদি( JSON_LOOKUP_ISERROR(rc) ){
      যদি( rc==JSON_LOOKUP_NOTFOUND ){
        চালু;  /* No-op */
      }অন্যথা{
        jsonBadPathError(ctx, zPath, rc);
      }
      যাও json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ফেরত;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  ফেরত;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
স্থির শূন্য jsonReplaceFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  যদি( argc<1 ) ফেরত;
  যদি( (argc&1)==0 ) {
    json_Wrong_অঙ্ক_Args(ctx, "replace");
    ফেরত;
  }
  json_ঢোকাও_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
স্থির শূন্য jsonSetFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা flags = SQLITE_PTR_TO_INT(sqlite3_user_তথ্য(ctx));
  পূর্ণসংখ্যা eInsType = JSON_ঢোকাও_TYPE(flags);
  স্থির ধ্রুবক অক্ষর *azInsType[] = { "insert", "set", "array_insert" };
  স্থির ধ্রুবক u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  যদি( argc<1 ) ফেরত;
  প্রতিজ্ঞা( eInsType>=0 && eInsType<=2 );
  যদি( (argc&1)==0 ) {
    json_Wrong_অঙ্ক_Args(ctx, azInsType[eInsType]);
    ফেরত;
  }
  json_ঢোকাও_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
স্থির শূন্য jsonTypeFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonParse *p;          /* The parse */
  ধ্রুবক অক্ষর *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  যদি( p==0 ) ফেরত;
  যদি( argc==2 ){
    zPath = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[1]);
    যদি( zPath==0 ) যাও json_type_done;
    যদি( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      যাও json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    যদি( JSON_LOOKUP_ISERROR(i) ){
      যদি( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }অন্যথা{
        jsonBadPathError(ctx, zPath, i);
      }
      যাও json_type_done;
    }
  }অন্যথা{
    i = 0;
  }
  sqlite3_ফলাফল_text(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
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
স্থির শূন্য jsonPrettyFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, আকার(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  যদি( x.pParse==0 ) ফেরত;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  যদি( argc==1 || (x.zIndent = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }অন্যথা{
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
স্থির শূন্য jsonValidFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  যদি( argc==2 ){
    i64 f = sqlite3_মান_int64(argv[1]);
    যদি( f<1 || f>15 ){
      sqlite3_ফলাফল_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      ফেরত;
    }
    flags = f & 0x0f;
  }
  নির্বাচন( sqlite3_মান_type(argv[0]) ){
    ক্ষেত্র SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_ফলাফল_int(ctx, 0);
#endif
      ফেরত;
    }
    ক্ষেত্র SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, আকার(py));
      যদি( jsonArgIsJsonb(argv[0], &py) ){
        যদি( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }অন্যথা যদি( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        বিরতি;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    পূর্বনির্ধারিত: {
      JsonParse px;
      যদি( (flags & 0x3)==0 ) বিরতি;
      memset(&px, 0, আকার(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      যদি( p ){
        যদি( p->oom ){
          sqlite3_ফলাফল_error_nomem(ctx);
        }অন্যথা যদি( p->nErr ){
          /* no-op */
        }অন্যথা যদি( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }অন্যথা{
        sqlite3_ফলাফল_error_nomem(ctx);
      }
      বিরতি;
    }
  }
  sqlite3_ফলাফল_int(ctx, res);
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
স্থির শূন্য jsonErrorFunc(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  প্রতিজ্ঞা( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, আকার(s));
  s.db = sqlite3_context_db_handle(ctx);
  যদি( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }অন্যথা{
    s.zJson = (অক্ষর*)sqlite3_মান_text(argv[0]);
    যদি( s.zJson==0 ) ফেরত;  /* NULL input or OOM */
    s.nJson = sqlite3_মান_bytes(argv[0]);
    যদি( jsonConvertTextToBlob(&s,0) ){
      যদি( s.oom ){
        iErrPos = -1;
      }অন্যথা{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        প্রতিজ্ঞা( s.zJson!=0 );  /* Because s.oom is false */
        জন্য(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          যদি( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  যদি( iErrPos<0 ){
    sqlite3_ফলাফল_error_nomem(ctx);
  }অন্যথা{
    sqlite3_ফলাফল_int64(ctx, iErrPos);
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
স্থির শূন্য jsonArrayStep(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, আকার(*pStr));
  যদি( pStr ){
    যদি( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }অন্যথা যদি( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_মান(pStr, argv[0]);
  }
}
স্থির শূন্য jsonArrayCompute(sqlite3_context *ctx, পূর্ণসংখ্যা isFinal){
  JsonString *pStr;
  পূর্ণসংখ্যা flags = SQLITE_PTR_TO_INT(sqlite3_user_তথ্য(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  যদি( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    যদি( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ফেরত;
    }অন্যথা যদি( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      যদি( isFinal ){
        যদি( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }অন্যথা{
        jsonStringTrimOneChar(pStr);
      }
      ফেরত;
    }অন্যথা যদি( isFinal ){
      sqlite3_ফলাফল_text(ctx, pStr->zBuf, (পূর্ণসংখ্যা)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }অন্যথা{
      sqlite3_ফলাফল_text(ctx, pStr->zBuf, (পূর্ণসংখ্যা)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }অন্যথা যদি( flags & JSON_BLOB ){
    স্থির ধ্রুবক u8 emptyArray = 0x0b;
    sqlite3_ফলাফল_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }অন্যথা{
    sqlite3_ফলাফল_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
}
স্থির শূন্য json_Array_মান(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
স্থির শূন্য jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
স্থির শূন্য jsonGroupInverse(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  unsigned পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা inStr = 0;
  পূর্ণসংখ্যা nNest = 0;
  অক্ষর *z;
  অক্ষর c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  যদি( NEVER(!pStr) ) ফেরত;
  z = pStr->zBuf;
  জন্য(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    যদি( c=='"' ){
      inStr = !inStr;
    }অন্যথা যদি( c=='\\' ){
      i++;
    }অন্যথা যদি( !inStr ){
      যদি( c=='{' || c=='[' ) nNest++;
      যদি( c=='}' || c==']' ) nNest--;
    }
  }
  যদি( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }অন্যথা{
    pStr->nUsed = 1;
  }
}
#অন্যথা
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
স্থির শূন্য jsonObjectStep(
  sqlite3_context *ctx,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  JsonString *pStr;
  ধ্রুবক অক্ষর *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, আকার(*pStr));
  যদি( pStr ){
    z = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
    n = sqlite3Strlen30(z);
    যদি( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }অন্যথা যদি( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    যদি( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_মান(pStr, argv[1]);
    }অন্যথা{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
স্থির শূন্য jsonObjectCompute(sqlite3_context *ctx, পূর্ণসংখ্যা isFinal){
  JsonString *pStr;
  পূর্ণসংখ্যা flags = SQLITE_PTR_TO_INT(sqlite3_user_তথ্য(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  যদি( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    যদি( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ফেরত;
    }
    যদি( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      পূর্ণসংখ্যা inStr = 0;
      যদি( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        যদি( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          ফেরত;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      জন্য(i=j=1; i<pStr->nUsed; i++){
        অক্ষর c = pStr->zBuf[i];
        যদি( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }অন্যথা যদি( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }অন্যথা যদি( c=='@' && !inStr ){
          প্রতিজ্ঞা( i+1<pStr->nUsed );
          যদি( pStr->zBuf[i+1]==',' ){
            i++;
          }অন্যথা যদি( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }অন্যথা{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    যদি( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      যদি( isFinal ){
        যদি( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }অন্যথা{
        jsonStringTrimOneChar(pOgStr);
      }
    }অন্যথা যদি( isFinal ){
      sqlite3_ফলাফল_text(ctx, pStr->zBuf, (পূর্ণসংখ্যা)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }অন্যথা{
      sqlite3_ফলাফল_text(ctx, pStr->zBuf, (পূর্ণসংখ্যা)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    যদি( pStr!=pOgStr ) jsonStringReset(pStr);
  }অন্যথা যদি( flags & JSON_BLOB ){
    স্থির ধ্রুবক unsigned অক্ষর emptyObject = 0x0c;
    sqlite3_ফলাফল_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }অন্যথা{
    sqlite3_ফলাফল_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
}
স্থির শূন্য json_Object_মান(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
স্থির শূন্য jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
প্রকারলেখ কাঠামো JsonParent JsonParent;
কাঠামো JsonParent {
  u32 i_শীর্ষ;                 /* Start of object or array */
  u32 i_মান;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_চাবি;                  /* Key for JSONB_ARRAY */
};

প্রকারলেখ কাঠামো JsonEachCursor JsonEachCursor;
কাঠামো JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_মূল;                 /* Size of the root path in bytes */
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
প্রকারলেখ কাঠামো JsonEachConnection JsonEachConnection;
কাঠামো JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
স্থির পূর্ণসংখ্যা jsonEachConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  JsonEachConnection *pNew;
  পূর্ণসংখ্যা rc;

/* Column numbers */
#define JEACH_চাবি     0
#define JEACH_মান   1
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
#define JEACH_মূল    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  যদি( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, আকার(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    যদি( pNew==0 ) ফেরত SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  ফেরত rc;
}

/* destructor for json_each virtual table */
স্থির পূর্ণসংখ্যা jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  ফেরত SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
স্থির পূর্ণসংখ্যা jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, আকার(*pCur));
  যদি( pCur==0 ) ফেরত SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  ফেরত SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
স্থির শূন্য jsonEachCursorReset(JsonEachCursor *p){
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
স্থির পূর্ণসংখ্যা jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  ফেরত SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
স্থির পূর্ণসংখ্যা jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ফেরত p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
স্থির পূর্ণসংখ্যা jsonSkipLabel(JsonEachCursor *p){
  যদি( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonb_Payload_আকার(&p->sParse, p->i, &sz);
    sz += p->i + n;
    যদি( sz >= p->sParse.nBlob ) sz = p->i;
    ফেরত sz;
  }অন্যথা{
    ফেরত p->i;
  }
}

/*
** Append the path name for the current element.
*/
স্থির শূন্য jsonAppendPathName(JsonEachCursor *p){
  প্রতিজ্ঞা( p->nParent>0 );
  প্রতিজ্ঞা( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  যদি( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_চাবি);
  }অন্যথা{
    u32 n, sz = 0, k, i;
    ধ্রুবক অক্ষর *z;
    পূর্ণসংখ্যা needQuote = 0;
    n = jsonb_Payload_আকার(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (ধ্রুবক অক্ষর*)&p->sParse.aBlob[k];
    যদি( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }অন্যথা{
      জন্য(i=0; i<sz; i++){
        যদি( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          বিরতি;
        }
      }
    }
    যদি( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }অন্যথা{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
স্থির পূর্ণসংখ্যা json_Each_Malformed_প্রবেশ(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_বার্তা);
  cur->pVtab->z_Err_বার্তা = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  ফেরত cur->pVtab->z_Err_বার্তা ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
স্থির পূর্ণসংখ্যা json_Each_পরবর্তী(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  যদি( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonb_Payload_আকার(&p->sParse, i, &sz);
    যদি( n==0 )ফেরত json_Each_Malformed_প্রবেশ(cur);
    যদি( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      যদি( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, আকার(JsonParent)*nNew);
        যদি( pNew==0 ) ফেরত SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->i_শীর্ষ = p->i;
      pParent->i_মান = i;
      pParent->iEnd = i + n + sz;
      pParent->i_চাবি = -1;
      pParent->nPath = (u32)p->path.nUsed;
      যদি( p->eType && p->nParent ){
        jsonAppendPathName(p);
        যদি( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }অন্যথা{
      p->i = i + n + sz;
    }
    যখন( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    যদি( levelChange ){
      যদি( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->i_মান;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }অন্যথা{
        p->eType = 0;
      }
    }
  }অন্যথা{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonb_Payload_আকার(&p->sParse, i, &sz);
    যদি( n==0 )ফেরত json_Each_Malformed_প্রবেশ(cur);
    p->i = i + n + sz;
  }
  যদি( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_চাবি++;
  }
  p->iRowid++;
  ফেরত rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
স্থির পূর্ণসংখ্যা json_Each_Path_দৈর্ঘ্য(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  অক্ষর *z = p->path.zBuf;
  যদি( p->iRowid==0 && p->bRecursive && n>=2 ){
    যখন( ALWAYS(n>1) ){
      n--;
      যদি( z[n]=='[' || z[n]=='.' ){
        u32 x;
        অক্ষর cSaved = z[n];
        z[n] = 0;
        প্রতিজ্ঞা( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        যদি( !JSON_LOOKUP_ISERROR(x) ) বিরতি;
      }
    }
  }
  ফেরত n;
}

/* Return the value of a column */
স্থির পূর্ণসংখ্যা jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  পূর্ণসংখ্যা iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  নির্বাচন( iColumn ){
    ক্ষেত্র JEACH_চাবি: {
      যদি( p->nParent==0 ){
        u32 n, j;
        যদি( p->n_মূল==1 ) বিরতি;
        j = json_Each_Path_দৈর্ঘ্য(p);
        n = p->n_মূল - j;
        যদি( n==0 ){
          বিরতি;
        }অন্যথা যদি( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_ফলাফল_int64(ctx, x);
        }অন্যথা যদি( p->path.zBuf[j+1]=='"' ){
          sqlite3_ফলাফল_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }অন্যথা{
          sqlite3_ফলাফল_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        বিরতি;
      }
      যদি( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }অন্যথা{
        প্রতিজ্ঞা( p->eType==JSONB_ARRAY );
        sqlite3_ফলাফল_int64(ctx, p->aParent[p->nParent-1].i_চাবি);
      }
      বিরতি;
    }
    ক্ষেত্র JEACH_মান: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      যদি( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_ফলাফল_subtype(ctx, JSON_SUBTYPE);
      }
      বিরতি;
    }
    ক্ষেত্র JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_ফলাফল_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      বিরতি;
    }
    ক্ষেত্র JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      যদি( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      বিরতি;
    }
    ক্ষেত্র JEACH_ID: {
      sqlite3_ফলাফল_int64(ctx, (sqlite3_int64)p->i);
      বিরতি;
    }
    ক্ষেত্র JEACH_PARENT: {
      যদি( p->nParent>0 && p->bRecursive ){
        sqlite3_ফলাফল_int64(ctx, p->aParent[p->nParent-1].i_শীর্ষ);
      }
      বিরতি;
    }
    ক্ষেত্র JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      যদি( p->nParent ) jsonAppendPathName(p);
      sqlite3_ফলাফল_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      বিরতি;
    }
    ক্ষেত্র JEACH_PATH: {
      u32 n = json_Each_Path_দৈর্ঘ্য(p);
      sqlite3_ফলাফল_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      sqlite3_ফলাফল_text(ctx, p->path.zBuf, p->n_মূল, SQLITE_TRANSIENT);
      বিরতি;
    }
    ক্ষেত্র JEACH_JSON: {
      যদি( p->sParse.zJson==0 ){
        sqlite3_ফলাফল_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }অন্যথা{
        sqlite3_ফলাফল_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      বিরতি;
    }
  }
  ফেরত SQLITE_OK;
}

/* Return the current rowid value */
স্থির পূর্ণসংখ্যা jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  ফেরত SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
স্থির পূর্ণসংখ্যা json_Each_Best_সূচক(
  sqlite3_vtab *tab,
  sqlite3_সূচক_info *pIdxInfo
){
  পূর্ণসংখ্যা i;                     /* Loop counter or computed array index */
  পূর্ণসংখ্যা aIdx[2];               /* Index of constraints for JSON and ROOT */
  পূর্ণসংখ্যা unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  পূর্ণসংখ্যা idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  ধ্রুবক কাঠামো sqlite3_সূচক_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  প্রতিজ্ঞা( JEACH_মূল == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  জন্য(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    পূর্ণসংখ্যা iCol;
    পূর্ণসংখ্যা iMask;
    যদি( pConstraint->iColumn < JEACH_JSON ) চালু;
    iCol = pConstraint->iColumn - JEACH_JSON;
    প্রতিজ্ঞা( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    যদি( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }অন্যথা যদি( pConstraint->op==SQLITE_সূচক_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  যদি( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  যদি( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    ফেরত SQLITE_CONSTRAINT;
  }
  যদি( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_অঙ্ক = 0;
  }অন্যথা{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argv_সূচক = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    যদি( aIdx[1]<0 ){
      pIdxInfo->idx_অঙ্ক = 1;  /* Only JSON supplied.  Plan 1 */
    }অন্যথা{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argv_সূচক = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_অঙ্ক = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  ফেরত SQLITE_OK;
}

/* Start a search on a new JSON string */
স্থির পূর্ণসংখ্যা jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ধ্রুবক অক্ষর *z_মূল = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  যদি( idx_অঙ্ক==0 ) ফেরত SQLITE_OK;
  memset(&p->sParse, 0, আকার(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  যদি( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }অন্যথা{
    p->sParse.zJson = (অক্ষর*)sqlite3_মান_text(argv[0]);
    p->sParse.nJson = sqlite3_মান_bytes(argv[0]);
    যদি( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      ফেরত SQLITE_OK;
    }      
    যদি( jsonConvertTextToBlob(&p->sParse, 0) ){
      যদি( p->sParse.oom ){
        ফেরত SQLITE_NOMEM;
      }
      ফেরত json_Each_Malformed_প্রবেশ(cur);
    }
  }
  যদি( idx_অঙ্ক==3 ){
    z_মূল = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[1]);
    যদি( z_মূল==0 ) ফেরত SQLITE_OK;
    যদি( z_মূল[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_বার্তা);
      cur->pVtab->z_Err_বার্তা = jsonBadPathError(0, z_মূল, 0);
      jsonEachCursorReset(p);
      ফেরত cur->pVtab->z_Err_বার্তা ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_মূল = sqlite3Strlen30(z_মূল);
    যদি( z_মূল[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }অন্যথা{
      i = jsonLookupStep(&p->sParse, 0, z_মূল+1, 0);
      যদি( JSON_LOOKUP_ISERROR(i) ){
        যদি( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          ফেরত SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_বার্তা);
        cur->pVtab->z_Err_বার্তা = jsonBadPathError(0, z_মূল, 0);
        jsonEachCursorReset(p);
        ফেরত cur->pVtab->z_Err_বার্তা ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      যদি( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }অন্যথা{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_মূল, p->n_মূল);
  }অন্যথা{
    i = p->i = 0;
    p->eType = 0;
    p->n_মূল = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonb_Payload_আকার(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  যদি( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, আকার(JsonParent));
    যদি( p->aParent==0 ) ফেরত SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_চাবি = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].i_শীর্ষ = p->i;
    p->aParent[0].i_মান = i;
  }
  ফেরত SQLITE_OK;
}

/* The methods of the json_each virtual table */
স্থির sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  json_Each_Best_সূচক,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  json_Each_পরবর্তী,              /* xNext - advance a cursor */
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
শূন্য sqlite3RegisterJsonFunctions(শূন্য){
#ifndef SQLITE_OMIT_JSON
  স্থির FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_ঢোকাও, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_ঢোকাও,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_দৈর্ঘ্য,  1,1,0, 0,0,0,          json_Array_দৈর্ঘ্য_Func),
    JFUNCTION(json_array_দৈর্ঘ্য,  2,1,0, 0,0,0,          json_Array_দৈর্ঘ্য_Func),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_ঢোকাও,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_ঢোকাও,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#যদি SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_মান, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_ফলাফল_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_মান, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_ফলাফল_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_মান, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_ফলাফল_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_মান, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_ফলাফল_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_ঢোকাও_Builtin_Funcs(aJsonFunc, Array_আকার(aJsonFunc));
#endif
}

#যদি  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, ধ্রুবক অক্ষর *zName){
  unsigned পূর্ণসংখ্যা i;
  স্থির ধ্রুবক অক্ষর *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  প্রতিজ্ঞা( sqlite3HashFind(&db->aModule, zName)==0 );
  জন্য(i=0; i<আকার(azModule)/আকার(azModule[0]); i++){
    যদি( sqlite3StrICmp(azModule[i],zName)==0 ){
      ফেরত sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  ফেরত 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
