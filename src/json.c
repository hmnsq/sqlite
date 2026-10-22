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
#അന്തര്ഭാവ "sqliteInt.h"

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
സ്ഥിതം സ്ഥിരം അക്ഷരം * സ്ഥിരം jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം jsonIsSpace[] = {
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
#define jsonIsspace(x) (jsonIsSpace[(unsigned അക്ഷരം)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
സ്ഥിതം സ്ഥിരം അക്ഷരം jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
സ്ഥിതം സ്ഥിരം അക്ഷരം jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം jsonIsOk[256] = {
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
തരനിർവചനം ഘടന JsonCache JsonCache;
തരനിർവചനം ഘടന JsonString JsonString;
തരനിർവചനം ഘടന JsonParse JsonParse;

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
ഘടന JsonCache {
  sqlite3 *db;                    /* Database connection */
  പൂർണ്ണസംഖ്യ nUsed;                      /* Number of active entries in the cache */
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
ഘടന JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  അക്ഷരം *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  അക്ഷരം zSpace[100];        /* Initial static space */
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

#define JSON_തിരുകുക_TYPE(X)  (((X)&0xC)>>2)


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
ഘടന JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  അക്ഷരം *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  പൂർണ്ണസംഖ്യ nJson;         /* Length of the zJson string in bytes */
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
  പൂർണ്ണസംഖ്യ delta;         /* Size change due to the edit */
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
#അല്ലെങ്കിൽ
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
സ്ഥിതം ശൂന്യം jsonReturnStringAsBlob(JsonString*);
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonArgIsJsonb(sqlite3_മൂല്യം *pJson, JsonParse *p);
സ്ഥിതം u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
സ്ഥിതം ശൂന്യം jsonReturnParse(sqlite3_context*,JsonParse*);
സ്ഥിതം JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_മൂല്യം*,u32);
സ്ഥിതം ശൂന്യം jsonParseFree(JsonParse*);
സ്ഥിതം u32 jsonbPayloadSize(സ്ഥിരം JsonParse*, u32, u32*);
സ്ഥിതം u32 jsonUnescapeOneChar(സ്ഥിരം അക്ഷരം*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
സ്ഥിതം ശൂന്യം jsonCacheDelete(JsonCache *p){
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
സ്ഥിതം ശൂന്യം jsonCacheDeleteGeneric(ശൂന്യം *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ json_Cache_തിരുകുക(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  പ്രതിജ്ഞാ( pParse->zJson!=0 );
  പ്രതിജ്ഞാ( pParse->bJsonIsRCStr );
  പ്രതിജ്ഞാ( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  എങ്കിൽ( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, വലുപ്പം(*p));
    എങ്കിൽ( p==0 ) തിരികെ SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    എങ്കിൽ( p==0 ) തിരികെ SQLITE_NOMEM;
  }
  എങ്കിൽ( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*വലുപ്പം(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  പ്രതിജ്ഞാ( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  തിരികെ SQLITE_OK;
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
സ്ഥിതം JsonParse *json_Cache_തിരയൽ(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_മൂല്യം *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  പൂർണ്ണസംഖ്യ i;
  സ്ഥിരം അക്ഷരം *zJson;
  പൂർണ്ണസംഖ്യ nJson;

  എങ്കിൽ( sqlite3_മൂല്യം_type(pArg)!=SQLITE_TEXT ){
    തിരികെ 0;
  }
  zJson = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(pArg);
  എങ്കിൽ( zJson==0 ) തിരികെ 0;
  nJson = sqlite3_മൂല്യം_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  എങ്കിൽ( p==0 ){
    തിരികെ 0;
  }
  വേണ്ടി(i=0; i<p->nUsed; i++){
    എങ്കിൽ( p->a[i]->zJson==zJson ) നിർത്തുക;
  }
  എങ്കിൽ( i>=p->nUsed ){
    വേണ്ടി(i=0; i<p->nUsed; i++){
      എങ്കിൽ( p->a[i]->nJson!=nJson ) തുടരുക;
      എങ്കിൽ( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) നിർത്തുക;
    }
  }
  എങ്കിൽ( i<p->nUsed ){
    എങ്കിൽ( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*വലുപ്പം(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    പ്രതിജ്ഞാ( p->a[i]->delta==0 );
    തിരികെ p->a[i];
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
സ്ഥിതം ശൂന്യം jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = വലുപ്പം(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
സ്ഥിതം ശൂന്യം jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
സ്ഥിതം ശൂന്യം jsonStringReset(JsonString *p){
  എങ്കിൽ( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
സ്ഥിതം ശൂന്യം jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  എങ്കിൽ( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
സ്ഥിതം ശൂന്യം jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  പ്രതിജ്ഞാ( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  അക്ഷരം *zNew;
  എങ്കിൽ( p->bStatic ){
    എങ്കിൽ( p->eErr ) തിരികെ 1;
    zNew = sqlite3RCStrNew(nTotal);
    എങ്കിൽ( zNew==0 ){
      jsonStringOom(p);
      തിരികെ SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }അല്ലെങ്കിൽ{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    എങ്കിൽ( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      തിരികെ SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  തിരികെ SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം jsonStringExpandAndAppend(
  JsonString *p,
  സ്ഥിരം അക്ഷരം *zIn,
  u32 N
){
  പ്രതിജ്ഞാ( N>0 );
  എങ്കിൽ( jsonStringGrow(p,N) ) തിരികെ;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
സ്ഥിതം ശൂന്യം jsonAppendRaw(JsonString *p, സ്ഥിരം അക്ഷരം *zIn, u32 N){
  എങ്കിൽ( N==0 ) തിരികെ;
  എങ്കിൽ( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }അല്ലെങ്കിൽ{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
സ്ഥിതം ശൂന്യം jsonAppendRawNZ(JsonString *p, സ്ഥിരം അക്ഷരം *zIn, u32 N){
  പ്രതിജ്ഞാ( N>0 );
  എങ്കിൽ( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }അല്ലെങ്കിൽ{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
സ്ഥിതം ശൂന്യം jsonPrintf(പൂർണ്ണസംഖ്യ N, JsonString *p, സ്ഥിരം അക്ഷരം *zFormat, ...){
  va_list ap;
  എങ്കിൽ( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) തിരികെ;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (പൂർണ്ണസംഖ്യ)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം jsonAppendCharExpand(JsonString *p, അക്ഷരം c){
  എങ്കിൽ( jsonStringGrow(p,1) ) തിരികെ;
  p->zBuf[p->nUsed++] = c;
}
സ്ഥിതം ശൂന്യം jsonAppendChar(JsonString *p, അക്ഷരം c){
  എങ്കിൽ( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }അല്ലെങ്കിൽ{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
സ്ഥിതം ശൂന്യം jsonStringTrimOneChar(JsonString *p){
  എങ്കിൽ( p->eErr==0 ){
    പ്രതിജ്ഞാ( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  തിരികെ p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
സ്ഥിതം ശൂന്യം jsonAppendSeparator(JsonString *p){
  അക്ഷരം c;
  എങ്കിൽ( p->nUsed==0 ) തിരികെ;
  c = p->zBuf[p->nUsed-1];
  എങ്കിൽ( c=='[' || c=='{' ) തിരികെ;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
സ്ഥിതം ശൂന്യം jsonAppendControlChar(JsonString *p, u8 c){
  സ്ഥിതം സ്ഥിരം അക്ഷരം aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  പ്രതിജ്ഞാ( വലുപ്പം(aSpecial)==32 );
  പ്രതിജ്ഞാ( aSpecial['\b']=='b' );
  പ്രതിജ്ഞാ( aSpecial['\f']=='f' );
  പ്രതിജ്ഞാ( aSpecial['\n']=='n' );
  പ്രതിജ്ഞാ( aSpecial['\r']=='r' );
  പ്രതിജ്ഞാ( aSpecial['\t']=='t' );
  പ്രതിജ്ഞാ( c>=0 && c<വലുപ്പം(aSpecial) );
  പ്രതിജ്ഞാ( p->nUsed+7 <= p->nAlloc );
  എങ്കിൽ( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം ശൂന്യം jsonAppendString(JsonString *p, സ്ഥിരം അക്ഷരം *zIn, u32 N){
  u32 k;
  u8 c;
  സ്ഥിരം u8 *z = (സ്ഥിരം u8*)zIn;
  എങ്കിൽ( z==0 ) തിരികെ;
  എങ്കിൽ( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) തിരികെ;
  p->zBuf[p->nUsed++] = '"';
  ആകുമ്പോൾ( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    ആകുമ്പോൾ( 1 /* Exit by break */ ){
      എങ്കിൽ( k+3>=N ){
        ആകുമ്പോൾ( k<N && jsonIsOk[z[k]] ){ k++; }
        നിർത്തുക;
      }
      എങ്കിൽ( !jsonIsOk[z[k]] ){
        നിർത്തുക;
      }
      എങ്കിൽ( !jsonIsOk[z[k+1]] ){
        k += 1;
        നിർത്തുക;
      }
      എങ്കിൽ( !jsonIsOk[z[k+2]] ){
        k += 2;
        നിർത്തുക;
      }
      എങ്കിൽ( !jsonIsOk[z[k+3]] ){
        k += 3;
        നിർത്തുക;
      }അല്ലെങ്കിൽ{
        k += 4;
      }
    }
    എങ്കിൽ( k>=N ){
      എങ്കിൽ( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      നിർത്തുക;
    }
    എങ്കിൽ( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    എങ്കിൽ( c=='"' || c=='\\' ){
      എങ്കിൽ( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) തിരികെ;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }അല്ലെങ്കിൽ എങ്കിൽ( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) തിരികെ;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  പ്രതിജ്ഞാ( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
സ്ഥിതം ശൂന്യം json_Append_Sql_മൂല്യം(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_മൂല്യം *p_മൂല്യം          /* Value to append */
){
  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_type(p_മൂല്യം) ){
    അവസ്ഥ SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_മൂല്യം_double(p_മൂല്യം));
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_INTEGER: {
      സ്ഥിരം അക്ഷരം *z = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(p_മൂല്യം);
      u32 n = (u32)sqlite3_മൂല്യം_bytes(p_മൂല്യം);
      jsonAppendRaw(p, z, n);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TEXT: {
      സ്ഥിരം അക്ഷരം *z = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(p_മൂല്യം);
      u32 n = (u32)sqlite3_മൂല്യം_bytes(p_മൂല്യം);
      എങ്കിൽ( sqlite3_മൂല്യം_subtype(p_മൂല്യം)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }അല്ലെങ്കിൽ{
        jsonAppendString(p, z, n);
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      JsonParse px;
      memset(&px, 0, വലുപ്പം(px));
      എങ്കിൽ( jsonArgIsJsonb(p_മൂല്യം, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }അല്ലെങ്കിൽ എങ്കിൽ( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      നിർത്തുക;
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
സ്ഥിതം ശൂന്യം jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  പ്രതിജ്ഞാ( (pParse!=0)==(ctx!=0) );
  പ്രതിജ്ഞാ( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  എങ്കിൽ( p->eErr==0 ){
    പൂർണ്ണസംഖ്യ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    എങ്കിൽ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }അല്ലെങ്കിൽ എങ്കിൽ( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        പൂർണ്ണസംഖ്യ rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_തിരുകുക(ctx, pParse);
        എങ്കിൽ( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          തിരികെ;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }അല്ലെങ്കിൽ എങ്കിൽ( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }അല്ലെങ്കിൽ എങ്കിൽ( p->eErr & JSTRING_MALFORMED ){
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
സ്ഥിതം ശൂന്യം jsonParseReset(JsonParse *pParse){
  പ്രതിജ്ഞാ( pParse->nJPRef<=1 );
  എങ്കിൽ( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  എങ്കിൽ( pParse->nBlobAlloc ){
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
സ്ഥിതം ശൂന്യം jsonParseFree(JsonParse *pParse){
  എങ്കിൽ( pParse ){
    എങ്കിൽ( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }അല്ലെങ്കിൽ{
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
സ്ഥിതം u8 jsonHexToInt(പൂർണ്ണസംഖ്യ h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  തിരികെ (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
സ്ഥിതം u32 jsonHexToInt4(സ്ഥിരം അക്ഷരം *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  തിരികെ v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonIs2Hex(സ്ഥിരം അക്ഷരം *z){
  തിരികെ sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonIs4Hex(സ്ഥിരം അക്ഷരം *z){
  തിരികെ jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ json5Whitespace(സ്ഥിരം അക്ഷരം *zIn){
  പൂർണ്ണസംഖ്യ n = 0;
  സ്ഥിരം u8 *z = (u8*)zIn;
  ആകുമ്പോൾ( 1 /*exit by "goto whitespace_done"*/ ){
    തിരഞ്ഞെടുപ്പ്( z[n] ){
      അവസ്ഥ 0x09:
      അവസ്ഥ 0x0a:
      അവസ്ഥ 0x0b:
      അവസ്ഥ 0x0c:
      അവസ്ഥ 0x0d:
      അവസ്ഥ 0x20: {
        n++;
        നിർത്തുക;
      }
      അവസ്ഥ '/': {
        എങ്കിൽ( z[n+1]=='*' && z[n+2]!=0 ){
          പൂർണ്ണസംഖ്യ j;
          വേണ്ടി(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            എങ്കിൽ( z[j]==0 ) പോകുക whitespace_done;
          }
          n = j+1;
          നിർത്തുക;
        }അല്ലെങ്കിൽ എങ്കിൽ( z[n+1]=='/' ){
          പൂർണ്ണസംഖ്യ j;
          അക്ഷരം c;
          വേണ്ടി(j=n+2; (c = z[j])!=0; j++){
            എങ്കിൽ( c=='\n' || c=='\r' ) നിർത്തുക;
            എങ്കിൽ( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              നിർത്തുക;
            }
          }
          n = j;
          എങ്കിൽ( z[n] ) n++;
          നിർത്തുക;
        }
        പോകുക whitespace_done;
      }
      അവസ്ഥ 0xc2: {
        എങ്കിൽ( z[n+1]==0xa0 ){
          n += 2;
          നിർത്തുക;
        }
        പോകുക whitespace_done;
      }
      അവസ്ഥ 0xe1: {
        എങ്കിൽ( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          നിർത്തുക;
        }
        പോകുക whitespace_done;
      }
      അവസ്ഥ 0xe2: {
        എങ്കിൽ( z[n+1]==0x80 ){
          u8 c = z[n+2];
          എങ്കിൽ( c<0x80 ) പോകുക whitespace_done;
          എങ്കിൽ( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            നിർത്തുക;
          }
        }അല്ലെങ്കിൽ എങ്കിൽ( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          നിർത്തുക;
        }
        പോകുക whitespace_done;
      }
      അവസ്ഥ 0xe3: {
        എങ്കിൽ( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          നിർത്തുക;
        }
        പോകുക whitespace_done;
      }
      അവസ്ഥ 0xef: {
        എങ്കിൽ( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          നിർത്തുക;
        }
        പോകുക whitespace_done;
      }
      സ്ഥിരസ്ഥിതി: {
        പോകുക whitespace_done;
      }
    }
  }
  whitespace_done:
  തിരികെ n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
സ്ഥിതം സ്ഥിരം ഘടന NanInfName {
  അക്ഷരം c1;
  അക്ഷരം c2;
  അക്ഷരം n;
  അക്ഷരം eType;
  അക്ഷരം nRepl;
  അക്ഷരം *zMatch;
  അക്ഷരം *zRepl;
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
സ്ഥിതം ശൂന്യം json_Wrong_അക്കം_Args(
  sqlite3_context *pCtx,
  സ്ഥിരം അക്ഷരം *zFuncName
){
  അക്ഷരം *z_സന്ദേശം = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, z_സന്ദേശം, -1);
  sqlite3_free(z_സന്ദേശം);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  പ്രതിജ്ഞാ( N>pParse->nBlobAlloc );
  എങ്കിൽ( pParse->nBlobAlloc==0 ){
    t = 100;
  }അല്ലെങ്കിൽ{
    t = pParse->nBlobAlloc*2;
  }
  എങ്കിൽ( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  എങ്കിൽ( aNew==0 ){ pParse->oom = 1; തിരികെ 1; }
  പ്രതിജ്ഞാ( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  തിരികെ 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 nSize;
  പ്രതിജ്ഞാ( !pParse->bReadOnly );
  എങ്കിൽ( pParse->oom ) തിരികെ 0;
  എങ്കിൽ( pParse->nBlobAlloc>0 ) തിരികെ 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  എങ്കിൽ( jsonBlobExpand(pParse, nSize) ){
    തിരികെ 0;
  }
  പ്രതിജ്ഞാ( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  തിരികെ 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  എങ്കിൽ( pParse->oom==0 ){
    പ്രതിജ്ഞാ( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
സ്ഥിതം ശൂന്യം jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  എങ്കിൽ( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }അല്ലെങ്കിൽ{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
സ്ഥിതം ശൂന്യം json_Blob_Append_കണ്ണി(JsonParse*,u8,u64,സ്ഥിരം ശൂന്യം*);
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം json_Blob_Expand_And_Append_കണ്ണി(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  സ്ഥിരം ശൂന്യം *aPayload
){
  എങ്കിൽ( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) തിരികെ;
  json_Blob_Append_കണ്ണി(pParse, eType, szPayload, aPayload);
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
സ്ഥിതം ശൂന്യം json_Blob_Append_കണ്ണി(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  സ്ഥിരം ശൂന്യം *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  എങ്കിൽ( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_കണ്ണി(pParse,eType,szPayload,aPayload);
    തിരികെ;
  }
  പ്രതിജ്ഞാ( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  എങ്കിൽ( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }അല്ലെങ്കിൽ എങ്കിൽ( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }അല്ലെങ്കിൽ{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  എങ്കിൽ( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  പൂർണ്ണസംഖ്യ delta;
  എങ്കിൽ( pParse->oom ) തിരികെ 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  എങ്കിൽ( szType<=11 ){
    nExtra = 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( szType==12 ){
    nExtra = 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( szType==13 ){
    nExtra = 2;
  }അല്ലെങ്കിൽ എങ്കിൽ( szType==14 ){
    nExtra = 4;
  }അല്ലെങ്കിൽ{
    nExtra = 8;
  }
  എങ്കിൽ( szPayload<=11 ){
    nNeeded = 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( szPayload<=0xff ){
    nNeeded = 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( szPayload<=0xffff ){
    nNeeded = 2;
  }അല്ലെങ്കിൽ{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  എങ്കിൽ( delta ){
    u64 newSize = (u64)pParse->nBlob + delta;
    എങ്കിൽ( delta>0 ){
      എങ്കിൽ( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        തിരികെ 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }അല്ലെങ്കിൽ{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  എങ്കിൽ( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }അല്ലെങ്കിൽ എങ്കിൽ( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }അല്ലെങ്കിൽ എങ്കിൽ( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }അല്ലെങ്കിൽ{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  തിരികെ delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonIs4HexB(സ്ഥിരം അക്ഷരം *z, പൂർണ്ണസംഖ്യ *pOp){
  എങ്കിൽ( z[0]!='u' ) തിരികെ 0;
  എങ്കിൽ( !jsonIs4Hex(&z[1]) ) തിരികെ 0;
  *pOp = JSONB_TEXTJ;
  തിരികെ 1;
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
സ്ഥിതം u32 jsonbValidityCheck(
  സ്ഥിരം JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  സ്ഥിരം u8 *z;
  u8 x;
  എങ്കിൽ( iDepth>JSON_MAX_DEPTH ) തിരികെ i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  എങ്കിൽ( NEVER(n==0) ) തിരികെ i+1;          /* Checked by caller */
  എങ്കിൽ( NEVER(i+n+sz!=iEnd) ) തിരികെ i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  തിരഞ്ഞെടുപ്പ്( x ){
    അവസ്ഥ JSONB_NULL:
    അവസ്ഥ JSONB_TRUE:
    അവസ്ഥ JSONB_FALSE: {
      തിരികെ n+sz==1 ? 0 : i+1;
    }
    അവസ്ഥ JSONB_INT: {
      എങ്കിൽ( sz<1 ) തിരികെ i+1;
      j = i+n;
      എങ്കിൽ( z[j]=='-' ){
        j++;
        എങ്കിൽ( sz<2 ) തിരികെ i+1;
      }
      k = i+n+sz;
      ആകുമ്പോൾ( j<k ){
        എങ്കിൽ( sqlite3Isdigit(z[j]) ){
          j++;
        }അല്ലെങ്കിൽ{
          തിരികെ j+1;
        }
      }
      തിരികെ 0;
    }
    അവസ്ഥ JSONB_INT5: {
      എങ്കിൽ( sz<3 ) തിരികെ i+1;
      j = i+n;
      എങ്കിൽ( z[j]=='-' ){
        എങ്കിൽ( sz<4 ) തിരികെ i+1;
        j++;
      }
      എങ്കിൽ( z[j]!='0' ) തിരികെ i+1;
      എങ്കിൽ( z[j+1]!='x' && z[j+1]!='X' ) തിരികെ j+2;
      j += 2;
      k = i+n+sz;
      ആകുമ്പോൾ( j<k ){
        എങ്കിൽ( sqlite3Isxdigit(z[j]) ){
          j++;
        }അല്ലെങ്കിൽ{
          തിരികെ j+1;
        }
      }
      തിരികെ 0;
    }
    അവസ്ഥ JSONB_FLOAT:
    അവസ്ഥ JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      എങ്കിൽ( sz<2 ) തിരികെ i+1;
      j = i+n;
      k = j+sz;
      എങ്കിൽ( z[j]=='-' ){
        j++;
        എങ്കിൽ( sz<3 ) തിരികെ i+1;
      }
      എങ്കിൽ( z[j]=='.' ){
        എങ്കിൽ( x==JSONB_FLOAT ) തിരികെ j+1;
        എങ്കിൽ( !sqlite3Isdigit(z[j+1]) ) തിരികെ j+1;
        j += 2;
        seen = 1;
      }അല്ലെങ്കിൽ എങ്കിൽ( z[j]=='0' && x==JSONB_FLOAT ){
        എങ്കിൽ( j+3>k ) തിരികെ j+1;
        എങ്കിൽ( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) തിരികെ j+1;
        j++;
      }
      വേണ്ടി(; j<k; j++){
        എങ്കിൽ( sqlite3Isdigit(z[j]) ) തുടരുക;
        എങ്കിൽ( z[j]=='.' ){
          എങ്കിൽ( seen>0 ) തിരികെ j+1;
          എങ്കിൽ( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            തിരികെ j+1;
          }
          seen = 1;
          തുടരുക;
        }
        എങ്കിൽ( z[j]=='e' || z[j]=='E' ){
          എങ്കിൽ( seen==2 ) തിരികെ j+1;
          എങ്കിൽ( j==k-1 ) തിരികെ j+1;
          എങ്കിൽ( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            എങ്കിൽ( j==k-1 ) തിരികെ j+1;
          }
          seen = 2;
          തുടരുക;
        }
        തിരികെ j+1;
      }
      എങ്കിൽ( seen==0 ) തിരികെ i+1;
      തിരികെ 0;
    }
    അവസ്ഥ JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      ആകുമ്പോൾ( j<k ){
        എങ്കിൽ( !jsonIsOk[z[j]] && z[j]!='\'' ) തിരികെ j+1;
        j++;
      }
      തിരികെ 0;
    }
    അവസ്ഥ JSONB_TEXTJ:
    അവസ്ഥ JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      ആകുമ്പോൾ( j<k ){
        എങ്കിൽ( !jsonIsOk[z[j]] && z[j]!='\'' ){
          എങ്കിൽ( z[j]=='"' ){
            എങ്കിൽ( x==JSONB_TEXTJ ) തിരികെ j+1;
          }അല്ലെങ്കിൽ എങ്കിൽ( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            എങ്കിൽ( x==JSONB_TEXTJ ) തിരികെ j+1;
          }അല്ലെങ്കിൽ എങ്കിൽ( NEVER(z[j]!='\\') || j+1>=k ){
            തിരികെ j+1;
          }അല്ലെങ്കിൽ എങ്കിൽ( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }അല്ലെങ്കിൽ എങ്കിൽ( z[j+1]=='u' ){
            എങ്കിൽ( j+5>=k ) തിരികെ j+1;
            എങ്കിൽ( !jsonIs4Hex((സ്ഥിരം അക്ഷരം*)&z[j+2]) ) തിരികെ j+1;
            j++;
          }അല്ലെങ്കിൽ എങ്കിൽ( x!=JSONB_TEXT5 ){
            തിരികെ j+1;
          }അല്ലെങ്കിൽ{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((സ്ഥിരം അക്ഷരം*)&z[j], k-j, &c);
            എങ്കിൽ( c==JSON_INVALID_CHAR ) തിരികെ j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      തിരികെ 0;
    }
    അവസ്ഥ JSONB_TEXTRAW: {
      തിരികെ 0;
    }
    അവസ്ഥ JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      ആകുമ്പോൾ( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        എങ്കിൽ( n==0 ) തിരികെ j+1;
        എങ്കിൽ( j+n+sz>k ) തിരികെ j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        എങ്കിൽ( sub ) തിരികെ sub;
        j += n + sz;
      }
      പ്രതിജ്ഞാ( j==k );
      തിരികെ 0;
    }
    അവസ്ഥ JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      ആകുമ്പോൾ( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        എങ്കിൽ( n==0 ) തിരികെ j+1;
        എങ്കിൽ( j+n+sz>k ) തിരികെ j+1;
        എങ്കിൽ( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          എങ്കിൽ( x<JSONB_TEXT || x>JSONB_TEXTRAW ) തിരികെ j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        എങ്കിൽ( sub ) തിരികെ sub;
        cnt++;
        j += n + sz;
      }
      പ്രതിജ്ഞാ( j==k );
      എങ്കിൽ( (cnt & 1)!=0 ) തിരികെ j+1;
      തിരികെ 0;
    }
    സ്ഥിരസ്ഥിതി: {
      തിരികെ i+1;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  അക്ഷരം c;
  u32 j;
  u32 iThis, iStart;
  പൂർണ്ണസംഖ്യ x;
  u8 t;
  സ്ഥിരം അക്ഷരം *z = pParse->zJson;
json_parse_restart:
  തിരഞ്ഞെടുപ്പ്( (u8)z[i] ){
  അവസ്ഥ '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_കണ്ണി(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    എങ്കിൽ( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      തിരികെ -1;
    }
    iStart = pParse->nBlob;
    വേണ്ടി(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      എങ്കിൽ( x<=0 ){
        പൂർണ്ണസംഖ്യ op;
        എങ്കിൽ( x==(-2) ){
          j = pParse->iErr;
          എങ്കിൽ( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          നിർത്തുക;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        എങ്കിൽ( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          പൂർണ്ണസംഖ്യ k = j+1;
          ആകുമ്പോൾ( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          പ്രതിജ്ഞാ( iBlob==pParse->nBlob );
          json_Blob_Append_കണ്ണി(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }അല്ലെങ്കിൽ{
          എങ്കിൽ( x!=-1 ) pParse->iErr = j;
          തിരികെ -1;
        }
      }
      എങ്കിൽ( pParse->oom ) തിരികെ -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      എങ്കിൽ( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        തിരികെ -1;
      }
      j = x;
      എങ്കിൽ( z[j]==':' ){
        j++;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          ചെയ്യുക{ j++; }ആകുമ്പോൾ( jsonIsspace(z[j]) );
          എങ്കിൽ( z[j]==':' ){
            j++;
            പോകുക parse_object_മൂല്യം;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        എങ്കിൽ( x!=(-5) ){
          എങ്കിൽ( x!=(-1) ) pParse->iErr = j;
          തിരികെ -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_മൂല്യം:
      x = jsonTranslateTextToBlob(pParse, j);
      എങ്കിൽ( x<=0 ){
        എങ്കിൽ( x!=(-1) ) pParse->iErr = j;
        തിരികെ -1;
      }
      j = x;
      എങ്കിൽ( z[j]==',' ){
        തുടരുക;
      }അല്ലെങ്കിൽ എങ്കിൽ( z[j]=='}' ){
        നിർത്തുക;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          എങ്കിൽ( z[j]==',' ){
            തുടരുക;
          }അല്ലെങ്കിൽ എങ്കിൽ( z[j]=='}' ){
            നിർത്തുക;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        എങ്കിൽ( x==(-4) ){
          j = pParse->iErr;
          തുടരുക;
        }
        എങ്കിൽ( x==(-2) ){
          j = pParse->iErr;
          നിർത്തുക;
        }
      }
      pParse->iErr = j;
      തിരികെ -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    തിരികെ j+1;
  }
  അവസ്ഥ '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    പ്രതിജ്ഞാ( i<=(u32)pParse->nJson );
    json_Blob_Append_കണ്ണി(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    എങ്കിൽ( pParse->oom ) തിരികെ -1;
    എങ്കിൽ( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      തിരികെ -1;
    }
    വേണ്ടി(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      എങ്കിൽ( x<=0 ){
        എങ്കിൽ( x==(-3) ){
          j = pParse->iErr;
          എങ്കിൽ( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          നിർത്തുക;
        }
        എങ്കിൽ( x!=(-1) ) pParse->iErr = j;
        തിരികെ -1;
      }
      j = x;
      എങ്കിൽ( z[j]==',' ){
        തുടരുക;
      }അല്ലെങ്കിൽ എങ്കിൽ( z[j]==']' ){
        നിർത്തുക;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          എങ്കിൽ( z[j]==',' ){
            തുടരുക;
          }അല്ലെങ്കിൽ എങ്കിൽ( z[j]==']' ){
            നിർത്തുക;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        എങ്കിൽ( x==(-4) ){
          j = pParse->iErr;
          തുടരുക;
        }
        എങ്കിൽ( x==(-3) ){
          j = pParse->iErr;
          നിർത്തുക;
        }
      }
      pParse->iErr = j;
      തിരികെ -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    തിരികെ j+1;
  }
  അവസ്ഥ '\'': {
    u8 opcode;
    അക്ഷരം cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    പോകുക parse_string;
  അവസ്ഥ '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    ആകുമ്പോൾ( 1 /*exit-by-break*/ ){
      എങ്കിൽ( jsonIsOk[(u8)z[j]] ){
        എങ്കിൽ( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }അല്ലെങ്കിൽ എങ്കിൽ( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }അല്ലെങ്കിൽ{
          j += 3;
          തുടരുക;
        }
      }
      c = z[j];
      എങ്കിൽ( c==cDelim ){
        നിർത്തുക;
      }അല്ലെങ്കിൽ എങ്കിൽ( c=='\\' ){
        c = z[++j];
        എങ്കിൽ( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          എങ്കിൽ( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }അല്ലെങ്കിൽ എങ്കിൽ( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#അല്ലെങ്കിൽ
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }അല്ലെങ്കിൽ എങ്കിൽ( c=='\r' ){
          എങ്കിൽ( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }അല്ലെങ്കിൽ{
          pParse->iErr = j;
          തിരികെ -1;
        }
      }അല്ലെങ്കിൽ എങ്കിൽ( c<=0x1f ){
        എങ്കിൽ( c==0 ){
          pParse->iErr = j;
          തിരികെ -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }അല്ലെങ്കിൽ എങ്കിൽ( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_കണ്ണി(pParse, opcode, j-1-i, &z[i+1]);
    തിരികെ j+1;
  }
  അവസ്ഥ 't': {
    എങ്കിൽ( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      തിരികെ i+4;
    }
    pParse->iErr = i;
    തിരികെ -1;
  }
  അവസ്ഥ 'f': {
    എങ്കിൽ( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      തിരികെ i+5;
    }
    pParse->iErr = i;
    തിരികെ -1;
  }
  അവസ്ഥ '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    പോകുക parse_സംഖ്യ;
  അവസ്ഥ '.':
    എങ്കിൽ( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      പോകുക parse_സംഖ്യ_2;
    }
    pParse->iErr = i;
    തിരികെ -1;
  അവസ്ഥ '-':
  അവസ്ഥ '0':
  അവസ്ഥ '1':
  അവസ്ഥ '2':
  അവസ്ഥ '3':
  അവസ്ഥ '4':
  അവസ്ഥ '5':
  അവസ്ഥ '6':
  അവസ്ഥ '7':
  അവസ്ഥ '8':
  അവസ്ഥ '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_സംഖ്യ:
    seenE = 0;
    പ്രതിജ്ഞാ( '-' < '0' );
    പ്രതിജ്ഞാ( '+' < '0' );
    പ്രതിജ്ഞാ( '.' < '0' );
    c = z[i];

    എങ്കിൽ( c<='0' ){
      എങ്കിൽ( c=='0' ){
        എങ്കിൽ( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          പ്രതിജ്ഞാ( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          വേണ്ടി(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          പോകുക parse_സംഖ്യ_finish;
        }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          തിരികെ -1;
        }
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          എങ്കിൽ( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            എങ്കിൽ( z[i]=='-' ){
              json_Blob_Append_കണ്ണി(pParse, JSONB_FLOAT, 6, "-9e999");
            }അല്ലെങ്കിൽ{
              json_Blob_Append_കണ്ണി(pParse, JSONB_FLOAT, 5, "9e999");
            }
            തിരികെ i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          എങ്കിൽ( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            പോകുക parse_സംഖ്യ_2;
          }
          pParse->iErr = i;
          തിരികെ -1;
        }
        എങ്കിൽ( z[i+1]=='0' ){
          എങ്കിൽ( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            തിരികെ -1;
          }അല്ലെങ്കിൽ എങ്കിൽ( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            വേണ്ടി(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            പോകുക parse_സംഖ്യ_finish;
          }
        }
      }
    }

  parse_സംഖ്യ_2:
    വേണ്ടി(j=i+1;; j++){
      c = z[j];
      എങ്കിൽ( sqlite3Isdigit(c) ) തുടരുക;
      എങ്കിൽ( c=='.' ){
        എങ്കിൽ( (t & 0x02)!=0 ){
          pParse->iErr = j;
          തിരികെ -1;
        }
        t |= 0x02;
        തുടരുക;
      }
      എങ്കിൽ( c=='e' || c=='E' ){
        എങ്കിൽ( z[j-1]<'0' ){
          എങ്കിൽ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }അല്ലെങ്കിൽ{
            pParse->iErr = j;
            തിരികെ -1;
          }
        }
        എങ്കിൽ( seenE ){
          pParse->iErr = j;
          തിരികെ -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        എങ്കിൽ( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        എങ്കിൽ( c<'0' || c>'9' ){
          pParse->iErr = j;
          തിരികെ -1;
        }
        തുടരുക;
      }
      നിർത്തുക;
    }
    എങ്കിൽ( z[j-1]<'0' ){
      എങ്കിൽ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }അല്ലെങ്കിൽ{
        pParse->iErr = j;
        തിരികെ -1;
      }
    }
  parse_സംഖ്യ_finish:
    പ്രതിജ്ഞാ( JSONB_INT+0x01==JSONB_INT5 );
    പ്രതിജ്ഞാ( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    പ്രതിജ്ഞാ( JSONB_INT+0x02==JSONB_FLOAT );
    എങ്കിൽ( z[i]=='+' ) i++;
    json_Blob_Append_കണ്ണി(pParse, JSONB_INT+t, j-i, &z[i]);
    തിരികെ j;
  }
  അവസ്ഥ '}': {
    pParse->iErr = i;
    തിരികെ -2;  /* End of {...} */
  }
  അവസ്ഥ ']': {
    pParse->iErr = i;
    തിരികെ -3;  /* End of [...] */
  }
  അവസ്ഥ ',': {
    pParse->iErr = i;
    തിരികെ -4;  /* List separator */
  }
  അവസ്ഥ ':': {
    pParse->iErr = i;
    തിരികെ -5;  /* Object label/value separator */
  }
  അവസ്ഥ 0: {
    തിരികെ 0;   /* End of file */
  }
  അവസ്ഥ 0x09:
  അവസ്ഥ 0x0a:
  അവസ്ഥ 0x0d:
  അവസ്ഥ 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    പോകുക json_parse_restart;
  }
  അവസ്ഥ 0x0b:
  അവസ്ഥ 0x0c:
  അവസ്ഥ '/':
  അവസ്ഥ 0xc2:
  അവസ്ഥ 0xe1:
  അവസ്ഥ 0xe2:
  അവസ്ഥ 0xe3:
  അവസ്ഥ 0xef: {
    j = json5Whitespace(&z[i]);
    എങ്കിൽ( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      പോകുക json_parse_restart;
    }
    pParse->iErr = i;
    തിരികെ -1;
  }
  അവസ്ഥ 'n': {
    എങ്കിൽ( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      തിരികെ i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  സ്ഥിരസ്ഥിതി: {
    u32 k;
    പൂർണ്ണസംഖ്യ nn;
    c = z[i];
    വേണ്ടി(k=0; k<വലുപ്പം(aNanInfName)/വലുപ്പം(aNanInfName[0]); k++){
      എങ്കിൽ( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) തുടരുക;
      nn = aNanInfName[k].n;
      എങ്കിൽ( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        തുടരുക;
      }
      എങ്കിൽ( sqlite3Isalnum(z[i+nn]) ) തുടരുക;
      എങ്കിൽ( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_കണ്ണി(pParse, JSONB_FLOAT, 5, "9e999");
      }അല്ലെങ്കിൽ{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      തിരികെ i + nn;
    }
    pParse->iErr = i;
    തിരികെ -1;  /* Syntax error */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  പൂർണ്ണസംഖ്യ i;
  സ്ഥിരം അക്ഷരം *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  എങ്കിൽ( pParse->oom ) i = -1;
  എങ്കിൽ( i>0 ){
#ifdef SQLITE_DEBUG
    പ്രതിജ്ഞാ( pParse->iDepth==0 );
    എങ്കിൽ( sqlite3Config.bJsonSelfcheck ){
      പ്രതിജ്ഞാ( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    ആകുമ്പോൾ( jsonIsspace(zJson[i]) ) i++;
    എങ്കിൽ( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      എങ്കിൽ( zJson[i] ){
        എങ്കിൽ( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        തിരികെ 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  എങ്കിൽ( i<=0 ){
    എങ്കിൽ( pCtx!=0 ){
      എങ്കിൽ( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }അല്ലെങ്കിൽ{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    തിരികെ 1;
  }
  തിരികെ 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
സ്ഥിതം ശൂന്യം jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  പ്രതിജ്ഞാ( pStr->eErr==0 );
  memset(&px, 0, വലുപ്പം(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (ശൂന്യം)jsonTranslateTextToBlob(&px, 0);
  എങ്കിൽ( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( px.nBlobAlloc>0 );
    പ്രതിജ്ഞാ( !px.bReadOnly );
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
സ്ഥിതം u32 jsonbPayloadSizeWide(
  സ്ഥിരം JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  പ്രതിജ്ഞാ( i<pParse->nBlob );
  പ്രതിജ്ഞാ( x==(pParse->aBlob[i]>>4) );
  പ്രതിജ്ഞാ( x>=13 );
  എങ്കിൽ( x==13 ){
    എങ്കിൽ( i+2>=pParse->nBlob ){
      *pSz = 0;
      തിരികെ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }അല്ലെങ്കിൽ എങ്കിൽ( x==14 ){
    എങ്കിൽ( i+4>=pParse->nBlob ){
      *pSz = 0;
      തിരികെ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      തിരികെ 0;
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
  എങ്കിൽ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    തിരികെ 0;
  }
  *pSz = sz;
  തിരികെ n;
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
സ്ഥിതം u32 jsonbPayloadSize(
  സ്ഥിരം JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  എങ്കിൽ( i>=pParse->nBlob ){
    *pSz = 0;
    തിരികെ 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( x==12 ){
    എങ്കിൽ( i+1>=pParse->nBlob ){
      *pSz = 0;
      തിരികെ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }അല്ലെങ്കിൽ{
    തിരികെ jsonbPayloadSizeWide(pParse, i, pSz, x);
  }
  എങ്കിൽ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    തിരികെ 0;
  }
  *pSz = sz;
  തിരികെ n;
}

#എങ്കിൽ SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
സ്ഥിതം SQLITE_INLINE u32 jsonbPayloadSizeInline(
  സ്ഥിരം JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  എങ്കിൽ( i>=pParse->nBlob ){
    *pSz = 0;
    തിരികെ 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( x==12 ){
    എങ്കിൽ( i+1>=pParse->nBlob ){
      *pSz = 0;
      തിരികെ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }അല്ലെങ്കിൽ എങ്കിൽ( x==13 ){
    എങ്കിൽ( i+2>=pParse->nBlob ){
      *pSz = 0;
      തിരികെ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }അല്ലെങ്കിൽ എങ്കിൽ( x==14 ){
    എങ്കിൽ( i+4>=pParse->nBlob ){
      *pSz = 0;
      തിരികെ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      തിരികെ 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  എങ്കിൽ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    തിരികെ 0;
  }
  *pSz = sz;
  തിരികെ n;
}
#അല്ലെങ്കിൽ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif


#എങ്കിൽ SQLITE_USES_INLINE
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
സ്ഥിതം SQLITE_INLINE u32 jsonbPayloadSizeSemiInline(
  സ്ഥിരം JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  പ്രതിജ്ഞാ( i<pParse->nBlob );
  എങ്കിൽ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( x==12 ){
    എങ്കിൽ( i+1>=pParse->nBlob ){
      *pSz = 0;
      തിരികെ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }അല്ലെങ്കിൽ{
    തിരികെ jsonbPayloadSizeWide(pParse,i,pSz,x);
  }
  എങ്കിൽ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    തിരികെ 0;
  }
  *pSz = sz;
  തിരികെ n;
}
#അല്ലെങ്കിൽ /* if !SQLITE_USES_INLINE */
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
സ്ഥിതം u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSizeInline(pParse, i, &sz);
  എങ്കിൽ( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    തിരികെ pParse->nBlob+1;
  }
  തിരഞ്ഞെടുപ്പ്( pParse->aBlob[i] & 0x0f ){
    അവസ്ഥ JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      തിരികെ i+1;
    }
    അവസ്ഥ JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      തിരികെ i+1;
    }
    അവസ്ഥ JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      തിരികെ i+1;
    }
    അവസ്ഥ JSONB_INT:
    അവസ്ഥ JSONB_FLOAT: {
      എങ്കിൽ( sz==0 ) പോകുക malformed_jsonb;
      jsonAppendRaw(pOut, (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n], sz);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      സ്ഥിരം അക്ഷരം *zIn = (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n];
      പൂർണ്ണസംഖ്യ bOverflow = 0;
      എങ്കിൽ( sz==0 ) പോകുക malformed_jsonb;
      എങ്കിൽ( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }അല്ലെങ്കിൽ എങ്കിൽ( zIn[0]=='+' ){
        k++;
      }
      വേണ്ടി(; k<sz; k++){
        എങ്കിൽ( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          നിർത്തുക;
        }അല്ലെങ്കിൽ എങ്കിൽ( (u>>60)!=0 ){
          bOverflow = 1;
        }അല്ലെങ്കിൽ{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      സ്ഥിരം അക്ഷരം *zIn = (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n];
      എങ്കിൽ( sz==0 ) പോകുക malformed_jsonb;
      എങ്കിൽ( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        എങ്കിൽ( sz<=1 ) പോകുക malformed_jsonb;
        k = 1;
      }
      എങ്കിൽ( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      വേണ്ടി(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        എങ്കിൽ( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_TEXT:
    അവസ്ഥ JSONB_TEXTJ: {
      എങ്കിൽ( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_TEXT5: {
      സ്ഥിരം അക്ഷരം *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      ആകുമ്പോൾ( sz2>0 ){
        വേണ്ടി(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        എങ്കിൽ( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          എങ്കിൽ( k>=sz2 ){
            നിർത്തുക;
          }
          zIn += k;
          sz2 -= k;
        }
        എങ്കിൽ( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          തുടരുക;
        }
        എങ്കിൽ( zIn[0]<=0x1f ){
          എങ്കിൽ( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) നിർത്തുക;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          തുടരുക;
        }
        പ്രതിജ്ഞാ( zIn[0]=='\\' );
        പ്രതിജ്ഞാ( sz2>=1 );
        എങ്കിൽ( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          നിർത്തുക;
        }
        തിരഞ്ഞെടുപ്പ്( (u8)zIn[1] ){
          അവസ്ഥ '\'':
            jsonAppendChar(pOut, '\'');
            നിർത്തുക;
          അവസ്ഥ 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            നിർത്തുക;
          അവസ്ഥ 'x':
            എങ്കിൽ( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              നിർത്തുക;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            നിർത്തുക;
          അവസ്ഥ '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            നിർത്തുക;
          അവസ്ഥ '\r':
            എങ്കിൽ( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            നിർത്തുക;
          അവസ്ഥ '\n':
            നിർത്തുക;
          അവസ്ഥ 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            എങ്കിൽ( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              നിർത്തുക;
            }
            zIn += 2;
            sz2 -= 2;
            നിർത്തുക;
          സ്ഥിരസ്ഥിതി:
            jsonAppendRawNZ(pOut, zIn, 2);
            നിർത്തുക;
        }
        പ്രതിജ്ഞാ( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_TEXTRAW: {
      jsonAppendString(pOut, (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n], sz);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      എങ്കിൽ( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      ആകുമ്പോൾ( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      എങ്കിൽ( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      എങ്കിൽ( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_OBJECT: {
      പൂർണ്ണസംഖ്യ x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      എങ്കിൽ( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      ആകുമ്പോൾ( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      എങ്കിൽ( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      എങ്കിൽ( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      നിർത്തുക;
    }

    സ്ഥിരസ്ഥിതി: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      നിർത്തുക;
    }
  }
  തിരികെ i+n+sz;
}

/* Context for recursion of json_pretty()
*/
തരനിർവചനം ഘടന JsonPretty JsonPretty;
ഘടന JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  സ്ഥിരം അക്ഷരം *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
സ്ഥിതം ശൂന്യം jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  വേണ്ടി(jj=0; jj<pPretty->nIndent; jj++){
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
സ്ഥിതം u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  എങ്കിൽ( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    തിരികെ pParse->nBlob+1;
  }
  തിരഞ്ഞെടുപ്പ്( pParse->aBlob[i] & 0x0f ){
    അവസ്ഥ JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      എങ്കിൽ( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        എങ്കിൽ( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        ആകുമ്പോൾ( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          എങ്കിൽ( j>=iEnd ){
            എങ്കിൽ( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            നിർത്തുക;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      എങ്കിൽ( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        എങ്കിൽ( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        ആകുമ്പോൾ( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          എങ്കിൽ( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            നിർത്തുക;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          എങ്കിൽ( j>=iEnd ) നിർത്തുക;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      നിർത്തുക;
    }
  }
  തിരികെ i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
സ്ഥിതം u32 jsonb_Array_എണ്ണം(JsonParse *pParse, u32 i_വേര്){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, i_വേര്, &sz);
  iEnd = i_വേര്+n+sz;
  വേണ്ടി(i=i_വേര്+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSizeSemiInline(pParse, i, &sz);
  }
  തിരികെ k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
സ്ഥിതം ശൂന്യം jsonAfterEditSizeAdjust(JsonParse *pParse, u32 i_വേര്){
  u32 sz = 0;
  u32 nBlob;
  പ്രതിജ്ഞാ( pParse->delta!=0 );
  പ്രതിജ്ഞാ( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (ശൂന്യം)jsonbPayloadSize(pParse, i_വേര്, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, i_വേര്, sz);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  സ്ഥിരം u8 *aIns,           /* New content */
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
  സ്ഥിതം സ്ഥിരം u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  എങ്കിൽ( (aIns[0]&0x0f)<=2 ) തിരികെ 0;    /* Cannot enlarge NULL, true, false */
  തിരഞ്ഞെടുപ്പ്( aIns[0]>>4 ){
    സ്ഥിരസ്ഥിതി: {                         /* aIns[] header size 1 */
      എങ്കിൽ( ((1<<d)&0x116)==0 ) തിരികെ 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      നിർത്തുക;
    }
    അവസ്ഥ 12: {                         /* aIns[] header size is 2 */
      എങ്കിൽ( ((1<<d)&0x8a)==0) തിരികെ 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      നിർത്തുക;
    }
    അവസ്ഥ 13: {                         /* aIns[] header size is 3 */
      എങ്കിൽ( d!=2 && d!=6 ) തിരികെ 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      നിർത്തുക;
    }
    അവസ്ഥ 14: {                         /* aIns[] header size is 5 */
      എങ്കിൽ( d!=4 ) തിരികെ 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      നിർത്തുക;
    }
    അവസ്ഥ 15: {                         /* aIns[] header size is 9 */
      തിരികെ 0;                          /* No solution */
    }
  }
  പ്രതിജ്ഞാ( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  ആകുമ്പോൾ( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    എങ്കിൽ( i==1 ) നിർത്തുക;
    szPayload >>= 8;
  }
  പ്രതിജ്ഞാ( (szPayload>>8)==0 );
  തിരികെ 1;
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
സ്ഥിതം ശൂന്യം jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  സ്ഥിരം u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  പ്രതിജ്ഞാ( pParse->nBlob >= (u64)iDel + (u64)nDel );
  എങ്കിൽ( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (പൂർണ്ണസംഖ്യ)-d)
  ){
    തിരികെ;
  }
  എങ്കിൽ( d!=0 ){
    എങ്കിൽ( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      എങ്കിൽ( pParse->oom ) തിരികെ;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  എങ്കിൽ( nIns && aIns ){
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
സ്ഥിതം u32 jsonBytesToBypass(സ്ഥിരം അക്ഷരം *z, u32 n){
  u32 i = 0;
  ആകുമ്പോൾ( i+1<n ){
    എങ്കിൽ( z[i]!='\\' ) തിരികെ i;
    എങ്കിൽ( z[i+1]=='\n' ){
      i += 2;
      തുടരുക;
    }
    എങ്കിൽ( z[i+1]=='\r' ){
      എങ്കിൽ( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }അല്ലെങ്കിൽ{
        i += 2;
      }
      തുടരുക;
    }
    എങ്കിൽ( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      തുടരുക;
    }
    നിർത്തുക;
  }
  തിരികെ i;
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
സ്ഥിതം u32 jsonUnescapeOneChar(സ്ഥിരം അക്ഷരം *z, u32 n, u32 *piOut){
  പ്രതിജ്ഞാ( n>0 );
  പ്രതിജ്ഞാ( z[0]=='\\' );
  എങ്കിൽ( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    തിരികെ n;
  }
  തിരഞ്ഞെടുപ്പ്( (u8)z[1] ){
    അവസ്ഥ 'u': {
      u32 v, vlo;
      എങ്കിൽ( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        തിരികെ n;
      }
      v = jsonHexToInt4(&z[2]);
      എങ്കിൽ( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        തിരികെ 12;
      }അല്ലെങ്കിൽ{
        *piOut = v;
        തിരികെ 6;
      }
    }
    അവസ്ഥ 'b': {   *piOut = '\b';  തിരികെ 2; }
    അവസ്ഥ 'f': {   *piOut = '\f';  തിരികെ 2; }
    അവസ്ഥ 'n': {   *piOut = '\n';  തിരികെ 2; }
    അവസ്ഥ 'r': {   *piOut = '\r';  തിരികെ 2; }
    അവസ്ഥ 't': {   *piOut = '\t';  തിരികെ 2; }
    അവസ്ഥ 'v': {   *piOut = '\v';  തിരികെ 2; }
    അവസ്ഥ '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#അല്ലെങ്കിൽ
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      തിരികെ 2;
    }
    അവസ്ഥ '\'':
    അവസ്ഥ '"':
    അവസ്ഥ '/':
    അവസ്ഥ '\\':{   *piOut = z[1];  തിരികെ 2; }
    അവസ്ഥ 'x': {
      എങ്കിൽ( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        തിരികെ n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      തിരികെ 4;
    }
    അവസ്ഥ 0xe2:
    അവസ്ഥ '\r':
    അവസ്ഥ '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      എങ്കിൽ( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        തിരികെ n;
      }അല്ലെങ്കിൽ എങ്കിൽ( nSkip==n ){
        *piOut = 0;
        തിരികെ n;
      }അല്ലെങ്കിൽ എങ്കിൽ( z[nSkip]=='\\' ){
        തിരികെ nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        തിരികെ nSkip + sz;
      }
    }
    സ്ഥിരസ്ഥിതി: {
      *piOut = JSON_INVALID_CHAR;
      തിരികെ 2;
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
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ jsonLabelCompareEscaped(
  സ്ഥിരം അക്ഷരം *z_ഇടത്,          /* The left label */
  u32 n_ഇടത്,                  /* Size of the left label in bytes */
  പൂർണ്ണസംഖ്യ raw_ഇടത്,                /* True if zLeft contains no escapes */
  സ്ഥിരം അക്ഷരം *z_വലത്,         /* The right label */
  u32 n_വലത്,                 /* Size of the right label in bytes */
  പൂർണ്ണസംഖ്യ raw_വലത്                /* True if zRight is escape-free */
){
  u32 c_ഇടത്, c_വലത്;
  പ്രതിജ്ഞാ( raw_ഇടത്==0 || raw_വലത്==0 );
  ആകുമ്പോൾ( 1 /*exit-by-return*/ ){
    എങ്കിൽ( n_ഇടത്==0 ){
      c_ഇടത് = 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( raw_ഇടത് || z_ഇടത്[0]!='\\' ){
      c_ഇടത് = ((u8*)z_ഇടത്)[0];
      എങ്കിൽ( c_ഇടത്>=0xc0 ){
        പൂർണ്ണസംഖ്യ sz = sqlite3Utf8ReadLimited((u8*)z_ഇടത്, n_ഇടത്, &c_ഇടത്);
        z_ഇടത് += sz;
        n_ഇടത് -= sz;
      }അല്ലെങ്കിൽ{
        z_ഇടത്++;
        n_ഇടത്--;
      }
    }അല്ലെങ്കിൽ{
      u32 n = jsonUnescapeOneChar(z_ഇടത്, n_ഇടത്, &c_ഇടത്);
      z_ഇടത് += n;
      പ്രതിജ്ഞാ( n<=n_ഇടത് );
      n_ഇടത് -= n;
    }
    എങ്കിൽ( n_വലത്==0 ){
      c_വലത് = 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( raw_വലത് || z_വലത്[0]!='\\' ){
      c_വലത് = ((u8*)z_വലത്)[0];
      എങ്കിൽ( c_വലത്>=0xc0 ){
        പൂർണ്ണസംഖ്യ sz = sqlite3Utf8ReadLimited((u8*)z_വലത്, n_വലത്, &c_വലത്);
        z_വലത് += sz;
        n_വലത് -= sz;
      }അല്ലെങ്കിൽ{
        z_വലത്++;
        n_വലത്--;
      }
    }അല്ലെങ്കിൽ{
      u32 n = jsonUnescapeOneChar(z_വലത്, n_വലത്, &c_വലത്);
      z_വലത് += n;
      പ്രതിജ്ഞാ( n<=n_വലത് );
      n_വലത് -= n;
    }
    എങ്കിൽ( c_ഇടത്!=c_വലത് ) തിരികെ 0;
    എങ്കിൽ( c_ഇടത്==0 ) തിരികെ 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonLabelCompare(
  സ്ഥിരം അക്ഷരം *z_ഇടത്,          /* The left label */
  u32 n_ഇടത്,                  /* Size of the left label in bytes */
  പൂർണ്ണസംഖ്യ raw_ഇടത്,                /* True if zLeft contains no escapes */
  സ്ഥിരം അക്ഷരം *z_വലത്,         /* The right label */
  u32 n_വലത്,                 /* Size of the right label in bytes */
  പൂർണ്ണസംഖ്യ raw_വലത്                /* True if zRight is escape-free */
){
  എങ്കിൽ( raw_ഇടത് && raw_വലത് ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    എങ്കിൽ( n_ഇടത്!=n_വലത് ) തിരികെ 0;
    തിരികെ memcmp(z_ഇടത്, z_വലത്, n_ഇടത്)==0;
  }അല്ലെങ്കിൽ{
    തിരികെ jsonLabelCompareEscaped(z_ഇടത്, n_ഇടത്, raw_ഇടത്,
                                   z_വലത്, n_വലത്, raw_വലത്);
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
സ്ഥിതം u32 jsonLookupStep(JsonParse*,u32,സ്ഥിരം അക്ഷരം*,u32);


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
സ്ഥിതം u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  സ്ഥിരം അക്ഷരം *zTail   /* Tail of the path that determines substructure */
){
  സ്ഥിതം സ്ഥിരം u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  പൂർണ്ണസംഖ്യ rc;
  memset(pIns, 0, വലുപ്പം(*pIns));
  pIns->db = pParse->db;
  എങ്കിൽ( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }അല്ലെങ്കിൽ{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    എങ്കിൽ( pIns->iDepth >= JSON_MAX_DEPTH ){
      തിരികെ JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  തിരികെ rc;  /* Error code only */
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
സ്ഥിതം u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_വേര്,              /* Begin the search at this element of aBlob[] */
  സ്ഥിരം അക്ഷരം *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_താക്കോൽ, sz, n, iEnd, rc;
  സ്ഥിരം അക്ഷരം *z_താക്കോൽ;
  u8 x;

  എങ്കിൽ( zPath[0]==0 ){
    എങ്കിൽ( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, i_വേര്, &sz);
      sz += n;
      എങ്കിൽ( pParse->eEdit==JEDIT_DEL ){
        എങ്കിൽ( iLabel>0 ){
          sz += i_വേര് - iLabel;
          i_വേര് = iLabel;
        }
        jsonBlobEdit(pParse, i_വേര്, sz, 0, 0);
      }അല്ലെങ്കിൽ എങ്കിൽ( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }അല്ലെങ്കിൽ എങ്കിൽ( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        എങ്കിൽ( zPath[-1]!=']' ){
          തിരികെ JSON_LOOKUP_NOTARRAY;
        }അല്ലെങ്കിൽ{
          jsonBlobEdit(pParse, i_വേര്, 0, pParse->aIns, pParse->nIns);
        }
      }അല്ലെങ്കിൽ{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_വേര്, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    തിരികെ i_വേര്;
  }
  എങ്കിൽ( zPath[0]=='.' ){
    പൂർണ്ണസംഖ്യ raw_താക്കോൽ = 1;
    x = pParse->aBlob[i_വേര്];
    zPath++;
    എങ്കിൽ( zPath[0]=='"' ){
      z_താക്കോൽ = zPath + 1;
      വേണ്ടി(i=1; zPath[i] && zPath[i]!='"'; i++){
        എങ്കിൽ( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_താക്കോൽ = i-1;
      എങ്കിൽ( zPath[i] ){
        i++;
      }അല്ലെങ്കിൽ{
        തിരികെ JSON_LOOKUP_PATHERROR;
      }
      testcase( n_താക്കോൽ==0 );
      raw_താക്കോൽ = memchr(z_താക്കോൽ, '\\', n_താക്കോൽ)==0;
    }അല്ലെങ്കിൽ{
      z_താക്കോൽ = zPath;
      വേണ്ടി(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_താക്കോൽ = i;
      എങ്കിൽ( n_താക്കോൽ==0 ){
        തിരികെ JSON_LOOKUP_PATHERROR;
      }
    }
    എങ്കിൽ( (x & 0x0f)!=JSONB_OBJECT ) തിരികെ JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_വേര്, &sz);
    j = i_വേര് + n;  /* j is the index of a label */
    iEnd = j+sz;
    ആകുമ്പോൾ( j<iEnd ){
      പൂർണ്ണസംഖ്യ rawLabel;
      സ്ഥിരം അക്ഷരം *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      എങ്കിൽ( x<JSONB_TEXT || x>JSONB_TEXTRAW ) തിരികെ JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      എങ്കിൽ( n==0 ) തിരികെ JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      എങ്കിൽ( k+sz>=iEnd ) തിരികെ JSON_LOOKUP_ERROR;
      zLabel = (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      എങ്കിൽ( jsonLabelCompare(z_താക്കോൽ, n_താക്കോൽ, raw_താക്കോൽ, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        എങ്കിൽ( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) തിരികെ JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        എങ്കിൽ( n==0 || v+n+sz>iEnd ) തിരികെ JSON_LOOKUP_ERROR;
        പ്രതിജ്ഞാ( j>0 );
        എങ്കിൽ( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          തിരികെ JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        എങ്കിൽ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_വേര്);
        തിരികെ rc;
      }
      j = k+sz;
      എങ്കിൽ( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) തിരികെ JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      എങ്കിൽ( n==0 ) തിരികെ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    എങ്കിൽ( j>iEnd ) തിരികെ JSON_LOOKUP_ERROR;
    എങ്കിൽ( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      എങ്കിൽ( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        തിരികെ JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, വലുപ്പം(ix));
      ix.db = pParse->db;
      json_Blob_Append_കണ്ണി(&ix, raw_താക്കോൽ?JSONB_TEXTRAW:JSONB_TEXT5, n_താക്കോൽ, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      എങ്കിൽ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_താക്കോൽ+v.nBlob)
      ){
        പ്രതിജ്ഞാ( !pParse->oom );
        nIns = ix.nBlob + n_താക്കോൽ + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        എങ്കിൽ( !pParse->oom ){
          പ്രതിജ്ഞാ( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          പ്രതിജ്ഞാ( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], z_താക്കോൽ, n_താക്കോൽ);
          k += n_താക്കോൽ;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          എങ്കിൽ( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, i_വേര്);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      തിരികെ rc;
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_വേര്] & 0x0f;
    എങ്കിൽ( x!=JSONB_ARRAY )  തിരികെ JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_വേര്, &sz);
    i = 1;
    ആകുമ്പോൾ( sqlite3Isdigit(zPath[i]) ){
      എങ്കിൽ( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    എങ്കിൽ( i<2 || zPath[i]!=']' ){
      എങ്കിൽ( zPath[1]=='#' ){
        kk = jsonb_Array_എണ്ണം(pParse, i_വേര്);
        i = 2;
        എങ്കിൽ( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          ചെയ്യുക{
            എങ്കിൽ( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }ആകുമ്പോൾ( sqlite3Isdigit(zPath[i]) );
          എങ്കിൽ( nn>kk ) തിരികെ JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        എങ്കിൽ( zPath[i]!=']' ){
          തിരികെ JSON_LOOKUP_PATHERROR;
        }
      }അല്ലെങ്കിൽ{
        തിരികെ JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_വേര്+n;
    iEnd = j+sz;
    ആകുമ്പോൾ( j<iEnd ){
      എങ്കിൽ( kk==0 ){
        എങ്കിൽ( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          തിരികെ JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        എങ്കിൽ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_വേര്);
        തിരികെ rc;
      }
      kk--;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      എങ്കിൽ( n==0 ) തിരികെ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    എങ്കിൽ( j>iEnd ) തിരികെ JSON_LOOKUP_ERROR;
    എങ്കിൽ( kk>0 ) തിരികെ JSON_LOOKUP_NOTFOUND;
    എങ്കിൽ( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      എങ്കിൽ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        പ്രതിജ്ഞാ( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      എങ്കിൽ( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_വേര്);
      തിരികെ rc;
    }
  }അല്ലെങ്കിൽ{
    തിരികെ JSON_LOOKUP_PATHERROR; 
  }
  തിരികെ JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
സ്ഥിതം ശൂന്യം jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  സ്ഥിരം u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  എങ്കിൽ( NEVER(aBlob==0) ) തിരികെ;
  memset(&x, 0, വലുപ്പം(x));
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
സ്ഥിതം ശൂന്യം jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  പൂർണ്ണസംഖ്യ eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  പ്രതിജ്ഞാ( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  എങ്കിൽ( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    തിരികെ;
  }
  തിരഞ്ഞെടുപ്പ്( pParse->aBlob[i] & 0x0f ){
    അവസ്ഥ JSONB_NULL: {
      എങ്കിൽ( sz ) പോകുക returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_TRUE: {
      എങ്കിൽ( sz ) പോകുക returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_FALSE: {
      എങ്കിൽ( sz ) പോകുക returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_INT5:
    അവസ്ഥ JSONB_INT: {
      sqlite3_int64 iRes = 0;
      അക്ഷരം *z;
      പൂർണ്ണസംഖ്യ bNeg = 0;
      അക്ഷരം x;
      എങ്കിൽ( sz==0 ) പോകുക returnfromblob_malformed;
      x = (അക്ഷരം)pParse->aBlob[i+n];
      എങ്കിൽ( x=='-' ){
        എങ്കിൽ( sz<2 ) പോകുക returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n], (പൂർണ്ണസംഖ്യ)sz);
      എങ്കിൽ( z==0 ) പോകുക returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      എങ്കിൽ( rc==0 ){
        എങ്കിൽ( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          ഇരട്ട r;
          r = (ഇരട്ട)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }അല്ലെങ്കിൽ{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }അല്ലെങ്കിൽ എങ്കിൽ( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }അല്ലെങ്കിൽ എങ്കിൽ( rc==1 ){
        പോകുക returnfromblob_malformed;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( bNeg ){ n--; sz++; }
        പോകുക to_double;
      }
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_FLOAT5:
    അവസ്ഥ JSONB_FLOAT: {
      ഇരട്ട r;
      അക്ഷരം *z;
      എങ്കിൽ( sz==0 ) പോകുക returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n], (പൂർണ്ണസംഖ്യ)sz);
      എങ്കിൽ( z==0 ) പോകുക returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      എങ്കിൽ( rc<=0 ) പോകുക returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_TEXTRAW:
    അവസ്ഥ JSONB_TEXT: {
      sqlite3_result_text(pCtx, (അക്ഷരം*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_TEXT5:
    അവസ്ഥ JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      സ്ഥിരം അക്ഷരം *z;
      അക്ഷരം *zOut;
      u32 nOut = sz;
      z = (സ്ഥിരം അക്ഷരം*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      എങ്കിൽ( zOut==0 ) പോകുക returnfromblob_oom;
      വേണ്ടി(iIn=iOut=0; iIn<sz; iIn++){
        അക്ഷരം c = z[iIn];
        എങ്കിൽ( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          എങ്കിൽ( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }അല്ലെങ്കിൽ എങ്കിൽ( v<=0x7f ){
            zOut[iOut++] = (അക്ഷരം)v;
          }അല്ലെങ്കിൽ എങ്കിൽ( v<=0x7ff ){
            പ്രതിജ്ഞാ( szEscape>=2 );
            zOut[iOut++] = (അക്ഷരം)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }അല്ലെങ്കിൽ എങ്കിൽ( v<0x10000 ){
            പ്രതിജ്ഞാ( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }അല്ലെങ്കിൽ എങ്കിൽ( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }അല്ലെങ്കിൽ{
            പ്രതിജ്ഞാ( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }അല്ലെങ്കിൽ{
          zOut[iOut++] = c;
        }
      } /* end for() */
      പ്രതിജ്ഞാ( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      നിർത്തുക;
    }
    അവസ്ഥ JSONB_ARRAY:
    അവസ്ഥ JSONB_OBJECT: {
      എങ്കിൽ( eMode==0 ){
        എങ്കിൽ( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }അല്ലെങ്കിൽ{
          eMode = 1;
        }
      }
      എങ്കിൽ( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }അല്ലെങ്കിൽ{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      പോകുക returnfromblob_malformed;
    }
  }
  തിരികെ;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  തിരികെ;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  തിരികെ;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_മൂല്യം *pArg,
  JsonParse *pParse
){
  പൂർണ്ണസംഖ്യ eType = sqlite3_മൂല്യം_type(pArg);
  സ്ഥിതം u8 aNull[] = { 0x00 };
  memset(pParse, 0, വലുപ്പം(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  തിരഞ്ഞെടുപ്പ്( eType ){
    സ്ഥിരസ്ഥിതി: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      തിരികെ 0;
    }
    അവസ്ഥ SQLITE_BLOB: {
      എങ്കിൽ( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        തിരികെ 1;
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TEXT: {
      സ്ഥിരം അക്ഷരം *zJson = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(pArg);
      പൂർണ്ണസംഖ്യ nJson = sqlite3_മൂല്യം_bytes(pArg);
      എങ്കിൽ( zJson==0 ) തിരികെ 1;
      എങ്കിൽ( sqlite3_മൂല്യം_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (അക്ഷരം*)zJson;
        pParse->nJson = nJson;
        എങ്കിൽ( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, വലുപ്പം(pParse[0]));
          തിരികെ 1;
        }
      }അല്ലെങ്കിൽ{
        json_Blob_Append_കണ്ണി(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_FLOAT: {
      എങ്കിൽ( NEVER(sqlite3IsNaN(sqlite3_മൂല്യം_double(pArg))) ){
        json_Blob_Append_കണ്ണി(pParse, JSONB_NULL, 0, 0);
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ n = sqlite3_മൂല്യം_bytes(pArg);
        സ്ഥിരം അക്ഷരം *z = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(pArg);
        എങ്കിൽ( z==0 ) തിരികെ 1;
        എങ്കിൽ( z[0]=='I' ){
          json_Blob_Append_കണ്ണി(pParse, JSONB_FLOAT, 5, "9e999");
        }അല്ലെങ്കിൽ എങ്കിൽ( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_കണ്ണി(pParse, JSONB_FLOAT, 6, "-9e999");
        }അല്ലെങ്കിൽ{
          json_Blob_Append_കണ്ണി(pParse, JSONB_FLOAT, n, z);
        }
      }
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_INTEGER: {
      പൂർണ്ണസംഖ്യ n = sqlite3_മൂല്യം_bytes(pArg);
      സ്ഥിരം അക്ഷരം *z = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(pArg);
      എങ്കിൽ( z==0 ) തിരികെ 1;
      json_Blob_Append_കണ്ണി(pParse, JSONB_INT, n, z);
      നിർത്തുക;
    }
  }
  എങ്കിൽ( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    തിരികെ 1;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
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
സ്ഥിതം അക്ഷരം *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  സ്ഥിരം അക്ഷരം *zPath,        /* The path with the problem */
  പൂർണ്ണസംഖ്യ rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  അക്ഷരം *z_സന്ദേശം;
  എങ്കിൽ( rc==(പൂർണ്ണസംഖ്യ)JSON_LOOKUP_NOTARRAY ){
    z_സന്ദേശം = sqlite3_mprintf("not an array element: %Q", zPath);
  }അല്ലെങ്കിൽ എങ്കിൽ( rc==(പൂർണ്ണസംഖ്യ)JSON_LOOKUP_ERROR ){
    z_സന്ദേശം = sqlite3_mprintf("malformed JSON");
  }അല്ലെങ്കിൽ എങ്കിൽ( rc==(പൂർണ്ണസംഖ്യ)JSON_LOOKUP_TOODEEP ){
    z_സന്ദേശം = sqlite3_mprintf("JSON path too deep");
  }അല്ലെങ്കിൽ{
    z_സന്ദേശം = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  എങ്കിൽ( ctx==0 ) തിരികെ z_സന്ദേശം;
  എങ്കിൽ( z_സന്ദേശം ){
    sqlite3_result_error(ctx, z_സന്ദേശം, -1);
    sqlite3_free(z_സന്ദേശം);
  }അല്ലെങ്കിൽ{
    sqlite3_result_error_nomem(ctx);
  }
  തിരികെ 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
സ്ഥിതം ശൂന്യം json_തിരുകുക_Into_Blob(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv,
  പൂർണ്ണസംഖ്യ eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  പൂർണ്ണസംഖ്യ i;
  u32 rc = 0;
  സ്ഥിരം അക്ഷരം *zPath = 0;
  പൂർണ്ണസംഖ്യ flgs;
  JsonParse *p;
  JsonParse ax;

  പ്രതിജ്ഞാ( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  എങ്കിൽ( p==0 ) തിരികെ;
  വേണ്ടി(i=1; i<argc-1; i+=2){
    എങ്കിൽ( sqlite3_മൂല്യം_type(argv[i])==SQLITE_NULL ) തുടരുക;
    zPath = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
    എങ്കിൽ( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      തിരികെ;
    }
    എങ്കിൽ( zPath[0]!='$' ) പോകുക json_തിരുകുക_Into_Blob_patherror;
    എങ്കിൽ( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      തിരികെ;
    }
    എങ്കിൽ( zPath[1]==0 ){
      എങ്കിൽ( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }അല്ലെങ്കിൽ{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    എങ്കിൽ( rc==JSON_LOOKUP_NOTFOUND ) തുടരുക;
    എങ്കിൽ( JSON_LOOKUP_ISERROR(rc) ) പോകുക json_തിരുകുക_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  തിരികെ;

json_തിരുകുക_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  തിരികെ;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonArgIsJsonb(sqlite3_മൂല്യം *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  എങ്കിൽ( sqlite3_മൂല്യം_type(pArg)!=SQLITE_BLOB ) തിരികെ 0;
  p->aBlob = (u8*)sqlite3_മൂല്യം_blob(pArg);
  p->nBlob = (u32)sqlite3_മൂല്യം_bytes(pArg);
  എങ്കിൽ( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    തിരികെ 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  തിരികെ 0;
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
സ്ഥിതം JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_മൂല്യം *pArg,
  u32 flgs
){
  പൂർണ്ണസംഖ്യ eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  പ്രതിജ്ഞാ( ctx!=0 );
  eType = sqlite3_മൂല്യം_type(pArg);
  എങ്കിൽ( eType==SQLITE_NULL ){
    തിരികെ 0;
  }
  എങ്കിൽ( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_തിരയൽ(ctx, pArg);
  }
  എങ്കിൽ( pFromCache ){
    pFromCache->nJPRef++;
    എങ്കിൽ( (flgs & JSON_EDITABLE)==0 ){
      തിരികെ pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, വലുപ്പം(*p));
  എങ്കിൽ( p==0 ) പോകുക json_pfa_oom;
  memset(p, 0, വലുപ്പം(*p));
  p->db = db;
  p->nJPRef = 1;
  എങ്കിൽ( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    എങ്കിൽ( p->aBlob==0 ) പോകുക json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    തിരികെ p;
  }
  എങ്കിൽ( eType==SQLITE_BLOB ){
    എങ്കിൽ( jsonArgIsJsonb(pArg,p) ){
      എങ്കിൽ( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        പോകുക json_pfa_oom;
      }
      തിരികെ p;
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
  p->zJson = (അക്ഷരം*)sqlite3_മൂല്യം_text(pArg);
  p->nJson = sqlite3_മൂല്യം_bytes(pArg);
  എങ്കിൽ( db->mallocFailed ) പോകുക json_pfa_oom;
  എങ്കിൽ( p->nJson==0 ) പോകുക json_pfa_malformed;
  പ്രതിജ്ഞാ( p->zJson!=0 );
  എങ്കിൽ( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    എങ്കിൽ( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      തിരികെ p;
    }അല്ലെങ്കിൽ{
      jsonParseFree(p);
      തിരികെ 0;
    }
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ isRCStr = sqlite3_മൂല്യം_Is_Of_Class(pArg, sqlite3RCStrUnref);
    പൂർണ്ണസംഖ്യ rc;
    എങ്കിൽ( !isRCStr ){
      അക്ഷരം *zNew = sqlite3RCStrNew( p->nJson );
      എങ്കിൽ( zNew==0 ) പോകുക json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }അല്ലെങ്കിൽ{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_തിരുകുക(ctx, p);
    എങ്കിൽ( rc==SQLITE_NOMEM ) പോകുക json_pfa_oom;
    എങ്കിൽ( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      പോകുക rebuild_from_cache;
    }
  }
  തിരികെ p;

json_pfa_malformed:
  എങ്കിൽ( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    തിരികെ p;
  }അല്ലെങ്കിൽ{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    തിരികെ 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  തിരികെ 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
സ്ഥിതം ശൂന്യം jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  പൂർണ്ണസംഖ്യ flgs;
  എങ്കിൽ( p->oom ){
    sqlite3_result_error_nomem(ctx);
    തിരികെ;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  എങ്കിൽ( flgs & JSON_BLOB ){
    എങ്കിൽ( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }അല്ലെങ്കിൽ{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }അല്ലെങ്കിൽ{
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

#എങ്കിൽ SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
സ്ഥിതം ശൂന്യം jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  പൂർണ്ണസംഖ്യ nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  ആകുമ്പോൾ( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    പൂർണ്ണസംഖ്യ showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    എങ്കിൽ( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    എങ്കിൽ( nn==0 ) nn = 1;
    എങ്കിൽ( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    വേണ്ടി(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    എങ്കിൽ( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      തുടരുക;
    }
    pParse->nBlob = savedNBlob;
    എങ്കിൽ( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      എങ്കിൽ( iEnd>pParse->nBlob ){
        എങ്കിൽ( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }അല്ലെങ്കിൽ{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    തിരഞ്ഞെടുപ്പ്( x ){
      അവസ്ഥ JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); നിർത്തുക;
      അവസ്ഥ JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); നിർത്തുക;
      അവസ്ഥ JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); നിർത്തുക;
      അവസ്ഥ JSONB_INT:      sqlite3_str_appendall(pOut,"int"); നിർത്തുക;
      അവസ്ഥ JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); നിർത്തുക;
      അവസ്ഥ JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); നിർത്തുക;
      അവസ്ഥ JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); നിർത്തുക;
      അവസ്ഥ JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); നിർത്തുക;
      അവസ്ഥ JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); നിർത്തുക;
      അവസ്ഥ JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); നിർത്തുക;
      അവസ്ഥ JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); നിർത്തുക;
      അവസ്ഥ JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        നിർത്തുക;
      }
      അവസ്ഥ JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        നിർത്തുക;
      }
    }
    എങ്കിൽ( showContent ){
      എങ്കിൽ( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }അല്ലെങ്കിൽ{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        വേണ്ടി(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          എങ്കിൽ( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (അക്ഷരം*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
സ്ഥിതം ശൂന്യം jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  അക്ഷരം zBuf[1000];
  എങ്കിൽ( pParse==0 ){
    printf("NULL pointer\n");
    തിരികെ;
  }അല്ലെങ്കിൽ{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    എങ്കിൽ( pParse->nBlob==0 ) തിരികെ;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, വലുപ്പം(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_മൂല്യം(&out));
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
സ്ഥിതം ശൂന്യം jsonParseFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  പ്രതിജ്ഞാ( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  എങ്കിൽ( p==0 ) തിരികെ;
  എങ്കിൽ( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം ശൂന്യം jsonQuoteFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_മൂല്യം(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
സ്ഥിതം ശൂന്യം jsonArrayFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  വേണ്ടി(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_മൂല്യം(&jx, argv[i]);
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
സ്ഥിതം ശൂന്യം jsonArrayLengthFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  എങ്കിൽ( p==0 ) തിരികെ;
  എങ്കിൽ( argc==2 ){
    സ്ഥിരം അക്ഷരം *zPath = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[1]);
    എങ്കിൽ( zPath==0 ){
      jsonParseFree(p);
      തിരികെ;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    എങ്കിൽ( JSON_LOOKUP_ISERROR(i) ){
      എങ്കിൽ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }അല്ലെങ്കിൽ{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }അല്ലെങ്കിൽ{
    i = 0;
  }
  എങ്കിൽ( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_എണ്ണം(p, i);
  }
  എങ്കിൽ( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonAllAlphanum(സ്ഥിരം അക്ഷരം *z, പൂർണ്ണസംഖ്യ n){
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  തിരികെ i==n;
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
സ്ഥിതം ശൂന്യം jsonExtractFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonParse *p = 0;      /* The parse */
  പൂർണ്ണസംഖ്യ flags;             /* Flags associated with the function */
  പൂർണ്ണസംഖ്യ i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  എങ്കിൽ( argc<2 ) തിരികെ;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  എങ്കിൽ( p==0 ) തിരികെ;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  എങ്കിൽ( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  വേണ്ടി(i=1; i<argc; i++){
    /* With a single PATH argument */
    സ്ഥിരം അക്ഷരം *zPath = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
    പൂർണ്ണസംഖ്യ nPath;
    u32 j;
    എങ്കിൽ( zPath==0 ) പോകുക json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    എങ്കിൽ( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }അല്ലെങ്കിൽ എങ്കിൽ( (flags & JSON_ABPATH) ){
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
      എങ്കിൽ( sqlite3_മൂല്യം_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        എങ്കിൽ( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }അല്ലെങ്കിൽ എങ്കിൽ( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }അല്ലെങ്കിൽ എങ്കിൽ( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }അല്ലെങ്കിൽ{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }അല്ലെങ്കിൽ{
      jsonBadPathError(ctx, zPath, 0);
      പോകുക json_extract_error;
    }
    എങ്കിൽ( j<p->nBlob ){
      എങ്കിൽ( argc==2 ){
        എങ്കിൽ( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          പ്രതിജ്ഞാ( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }അല്ലെങ്കിൽ{
          jsonReturnFromBlob(p, j, ctx, 0);
          എങ്കിൽ( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }അല്ലെങ്കിൽ{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( j==JSON_LOOKUP_NOTFOUND ){
      എങ്കിൽ( argc==2 ){
        പോകുക json_extract_error;  /* Return NULL if not found */
      }അല്ലെങ്കിൽ{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }അല്ലെങ്കിൽ{
      jsonBadPathError(ctx, zPath, j);
      പോകുക json_extract_error;
    }
  }
  എങ്കിൽ( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    എങ്കിൽ( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  തിരികെ;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  സ്ഥിരം JsonParse *pPatch, /* The PATCH */
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

  പ്രതിജ്ഞാ( iTarget>=0 && iTarget<pTarget->nBlob );
  പ്രതിജ്ഞാ( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  എങ്കിൽ( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    തിരികെ pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  എങ്കിൽ( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  എങ്കിൽ( NEVER(n==0) ) തിരികെ JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  എങ്കിൽ( NEVER(n==0) ) തിരികെ JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  ആകുമ്പോൾ( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    എങ്കിൽ( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      തിരികെ JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    എങ്കിൽ( nPLabel==0 ) തിരികെ JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    എങ്കിൽ( iPValue>=iPEnd ) തിരികെ JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    എങ്കിൽ( nPValue==0 ) തിരികെ JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    എങ്കിൽ( iPCursor>iPEnd ) തിരികെ JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    ആകുമ്പോൾ( iTCursor<iTEnd ){
      പൂർണ്ണസംഖ്യ isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      എങ്കിൽ( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        തിരികെ JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      എങ്കിൽ( nTLabel==0 ) തിരികെ JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      എങ്കിൽ( iTValue>=iTEnd ) തിരികെ JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      എങ്കിൽ( nTValue==0 ) തിരികെ JSON_MERGE_BADTARGET;
      എങ്കിൽ( iTValue + nTValue + szTValue > iTEnd ) തിരികെ JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (സ്ഥിരം അക്ഷരം*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (സ്ഥിരം അക്ഷരം*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      എങ്കിൽ( isEqual ) നിർത്തുക;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    എങ്കിൽ( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      എങ്കിൽ( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        എങ്കിൽ( NEVER(pTarget->oom) ) തിരികെ JSON_MERGE_OOM;
      }അല്ലെങ്കിൽ{
        /* Algorithm line 12 */
        പൂർണ്ണസംഖ്യ rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        എങ്കിൽ( iDepth>=JSON_MAX_DEPTH ) തിരികെ JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        എങ്കിൽ( rc ) തിരികെ rc;
        pTarget->delta += savedDelta;
      }        
    }അല്ലെങ്കിൽ എങ്കിൽ( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      എങ്കിൽ( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        എങ്കിൽ( pTarget->oom ) തിരികെ JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        എങ്കിൽ( pTarget->oom ) തിരികെ JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        എങ്കിൽ( iDepth>=JSON_MAX_DEPTH ) തിരികെ JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        എങ്കിൽ( rc ) തിരികെ rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  എങ്കിൽ( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  തിരികെ pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
സ്ഥിതം ശൂന്യം jsonPatchFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  പൂർണ്ണസംഖ്യ rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  പ്രതിജ്ഞാ( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  എങ്കിൽ( pTarget==0 ) തിരികെ;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  എങ്കിൽ( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    എങ്കിൽ( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }അല്ലെങ്കിൽ എങ്കിൽ( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }അല്ലെങ്കിൽ എങ്കിൽ( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }അല്ലെങ്കിൽ{
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
സ്ഥിതം ശൂന്യം jsonObjectFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ i;
  JsonString jx;
  സ്ഥിരം അക്ഷരം *z;
  u32 n;

  എങ്കിൽ( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    തിരികെ;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  വേണ്ടി(i=0; i<argc; i+=2){
    എങ്കിൽ( sqlite3_മൂല്യം_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      തിരികെ;
    }
    jsonAppendSeparator(&jx);
    z = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
    n = sqlite3_മൂല്യം_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_മൂല്യം(&jx, argv[i+1]);
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
സ്ഥിതം ശൂന്യം jsonRemoveFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonParse *p;          /* The parse */
  സ്ഥിരം അക്ഷരം *zPath = 0; /* Path of element to be removed */
  പൂർണ്ണസംഖ്യ i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  എങ്കിൽ( argc<1 ) തിരികെ;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  എങ്കിൽ( p==0 ) തിരികെ;
  വേണ്ടി(i=1; i<argc; i++){
    zPath = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
    എങ്കിൽ( zPath==0 ){
      പോകുക json_remove_done;
    }
    എങ്കിൽ( zPath[0]!='$' ){
      പോകുക json_remove_patherror;
    }
    എങ്കിൽ( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      പോകുക json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    എങ്കിൽ( JSON_LOOKUP_ISERROR(rc) ){
      എങ്കിൽ( rc==JSON_LOOKUP_NOTFOUND ){
        തുടരുക;  /* No-op */
      }അല്ലെങ്കിൽ{
        jsonBadPathError(ctx, zPath, rc);
      }
      പോകുക json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  തിരികെ;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  തിരികെ;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
സ്ഥിതം ശൂന്യം jsonReplaceFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  എങ്കിൽ( argc<1 ) തിരികെ;
  എങ്കിൽ( (argc&1)==0 ) {
    json_Wrong_അക്കം_Args(ctx, "replace");
    തിരികെ;
  }
  json_തിരുകുക_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
സ്ഥിതം ശൂന്യം jsonSetFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  പൂർണ്ണസംഖ്യ eInsType = JSON_തിരുകുക_TYPE(flags);
  സ്ഥിതം സ്ഥിരം അക്ഷരം *azInsType[] = { "insert", "set", "array_insert" };
  സ്ഥിതം സ്ഥിരം u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  എങ്കിൽ( argc<1 ) തിരികെ;
  പ്രതിജ്ഞാ( eInsType>=0 && eInsType<=2 );
  എങ്കിൽ( (argc&1)==0 ) {
    json_Wrong_അക്കം_Args(ctx, azInsType[eInsType]);
    തിരികെ;
  }
  json_തിരുകുക_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
സ്ഥിതം ശൂന്യം jsonTypeFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonParse *p;          /* The parse */
  സ്ഥിരം അക്ഷരം *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  എങ്കിൽ( p==0 ) തിരികെ;
  എങ്കിൽ( argc==2 ){
    zPath = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[1]);
    എങ്കിൽ( zPath==0 ) പോകുക json_type_done;
    എങ്കിൽ( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      പോകുക json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    എങ്കിൽ( JSON_LOOKUP_ISERROR(i) ){
      എങ്കിൽ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }അല്ലെങ്കിൽ{
        jsonBadPathError(ctx, zPath, i);
      }
      പോകുക json_type_done;
    }
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം ശൂന്യം jsonPrettyFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, വലുപ്പം(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  എങ്കിൽ( x.pParse==0 ) തിരികെ;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  എങ്കിൽ( argc==1 || (x.zIndent = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം ശൂന്യം jsonValidFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  എങ്കിൽ( argc==2 ){
    i64 f = sqlite3_മൂല്യം_int64(argv[1]);
    എങ്കിൽ( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      തിരികെ;
    }
    flags = f & 0x0f;
  }
  തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_type(argv[0]) ){
    അവസ്ഥ SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      തിരികെ;
    }
    അവസ്ഥ SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, വലുപ്പം(py));
      എങ്കിൽ( jsonArgIsJsonb(argv[0], &py) ){
        എങ്കിൽ( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }അല്ലെങ്കിൽ എങ്കിൽ( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        നിർത്തുക;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    സ്ഥിരസ്ഥിതി: {
      JsonParse px;
      എങ്കിൽ( (flags & 0x3)==0 ) നിർത്തുക;
      memset(&px, 0, വലുപ്പം(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      എങ്കിൽ( p ){
        എങ്കിൽ( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }അല്ലെങ്കിൽ എങ്കിൽ( p->nErr ){
          /* no-op */
        }അല്ലെങ്കിൽ എങ്കിൽ( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }അല്ലെങ്കിൽ{
        sqlite3_result_error_nomem(ctx);
      }
      നിർത്തുക;
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
സ്ഥിതം ശൂന്യം jsonErrorFunc(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  പ്രതിജ്ഞാ( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, വലുപ്പം(s));
  s.db = sqlite3_context_db_handle(ctx);
  എങ്കിൽ( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }അല്ലെങ്കിൽ{
    s.zJson = (അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    എങ്കിൽ( s.zJson==0 ) തിരികെ;  /* NULL input or OOM */
    s.nJson = sqlite3_മൂല്യം_bytes(argv[0]);
    എങ്കിൽ( jsonConvertTextToBlob(&s,0) ){
      എങ്കിൽ( s.oom ){
        iErrPos = -1;
      }അല്ലെങ്കിൽ{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        പ്രതിജ്ഞാ( s.zJson!=0 );  /* Because s.oom is false */
        വേണ്ടി(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          എങ്കിൽ( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  എങ്കിൽ( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }അല്ലെങ്കിൽ{
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
സ്ഥിതം ശൂന്യം jsonArrayStep(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, വലുപ്പം(*pStr));
  എങ്കിൽ( pStr ){
    എങ്കിൽ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }അല്ലെങ്കിൽ എങ്കിൽ( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_മൂല്യം(pStr, argv[0]);
  }
}
സ്ഥിതം ശൂന്യം jsonArrayCompute(sqlite3_context *ctx, പൂർണ്ണസംഖ്യ isFinal){
  JsonString *pStr;
  പൂർണ്ണസംഖ്യ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  എങ്കിൽ( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    എങ്കിൽ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      തിരികെ;
    }അല്ലെങ്കിൽ എങ്കിൽ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      എങ്കിൽ( isFinal ){
        എങ്കിൽ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }അല്ലെങ്കിൽ{
        jsonStringTrimOneChar(pStr);
      }
      തിരികെ;
    }അല്ലെങ്കിൽ എങ്കിൽ( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (പൂർണ്ണസംഖ്യ)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }അല്ലെങ്കിൽ{
      sqlite3_result_text(ctx, pStr->zBuf, (പൂർണ്ണസംഖ്യ)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( flags & JSON_BLOB ){
    സ്ഥിതം സ്ഥിരം u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }അല്ലെങ്കിൽ{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
സ്ഥിതം ശൂന്യം json_Array_മൂല്യം(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
സ്ഥിതം ശൂന്യം jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
സ്ഥിതം ശൂന്യം jsonGroupInverse(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  unsigned പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ inStr = 0;
  പൂർണ്ണസംഖ്യ nNest = 0;
  അക്ഷരം *z;
  അക്ഷരം c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  എങ്കിൽ( NEVER(!pStr) ) തിരികെ;
  z = pStr->zBuf;
  വേണ്ടി(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    എങ്കിൽ( c=='"' ){
      inStr = !inStr;
    }അല്ലെങ്കിൽ എങ്കിൽ( c=='\\' ){
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( !inStr ){
      എങ്കിൽ( c=='{' || c=='[' ) nNest++;
      എങ്കിൽ( c=='}' || c==']' ) nNest--;
    }
  }
  എങ്കിൽ( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }അല്ലെങ്കിൽ{
    pStr->nUsed = 1;
  }
}
#അല്ലെങ്കിൽ
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
സ്ഥിതം ശൂന്യം jsonObjectStep(
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  JsonString *pStr;
  സ്ഥിരം അക്ഷരം *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, വലുപ്പം(*pStr));
  എങ്കിൽ( pStr ){
    z = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    n = sqlite3Strlen30(z);
    എങ്കിൽ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }അല്ലെങ്കിൽ എങ്കിൽ( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    എങ്കിൽ( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_മൂല്യം(pStr, argv[1]);
    }അല്ലെങ്കിൽ{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
സ്ഥിതം ശൂന്യം jsonObjectCompute(sqlite3_context *ctx, പൂർണ്ണസംഖ്യ isFinal){
  JsonString *pStr;
  പൂർണ്ണസംഖ്യ flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  എങ്കിൽ( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    എങ്കിൽ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      തിരികെ;
    }
    എങ്കിൽ( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      പൂർണ്ണസംഖ്യ inStr = 0;
      എങ്കിൽ( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        എങ്കിൽ( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          തിരികെ;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      വേണ്ടി(i=j=1; i<pStr->nUsed; i++){
        അക്ഷരം c = pStr->zBuf[i];
        എങ്കിൽ( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }അല്ലെങ്കിൽ എങ്കിൽ( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }അല്ലെങ്കിൽ എങ്കിൽ( c=='@' && !inStr ){
          പ്രതിജ്ഞാ( i+1<pStr->nUsed );
          എങ്കിൽ( pStr->zBuf[i+1]==',' ){
            i++;
          }അല്ലെങ്കിൽ എങ്കിൽ( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }അല്ലെങ്കിൽ{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    എങ്കിൽ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      എങ്കിൽ( isFinal ){
        എങ്കിൽ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }അല്ലെങ്കിൽ{
        jsonStringTrimOneChar(pOgStr);
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (പൂർണ്ണസംഖ്യ)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }അല്ലെങ്കിൽ{
      sqlite3_result_text(ctx, pStr->zBuf, (പൂർണ്ണസംഖ്യ)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    എങ്കിൽ( pStr!=pOgStr ) jsonStringReset(pStr);
  }അല്ലെങ്കിൽ എങ്കിൽ( flags & JSON_BLOB ){
    സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }അല്ലെങ്കിൽ{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
സ്ഥിതം ശൂന്യം json_Object_മൂല്യം(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
സ്ഥിതം ശൂന്യം jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
തരനിർവചനം ഘടന JsonParent JsonParent;
ഘടന JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 i_മൂല്യം;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_താക്കോൽ;                  /* Key for JSONB_ARRAY */
};

തരനിർവചനം ഘടന JsonEachCursor JsonEachCursor;
ഘടന JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_വേര്;                 /* Size of the root path in bytes */
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
തരനിർവചനം ഘടന JsonEachConnection JsonEachConnection;
ഘടന JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  JsonEachConnection *pNew;
  പൂർണ്ണസംഖ്യ rc;

/* Column numbers */
#define JEACH_താക്കോൽ     0
#define JEACH_മൂല്യം   1
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
#define JEACH_വേര്    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  എങ്കിൽ( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, വലുപ്പം(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    എങ്കിൽ( pNew==0 ) തിരികെ SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  തിരികെ rc;
}

/* destructor for json_each virtual table */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  തിരികെ SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, വലുപ്പം(*pCur));
  എങ്കിൽ( pCur==0 ) തിരികെ SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  തിരികെ SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
സ്ഥിതം ശൂന്യം jsonEachCursorReset(JsonEachCursor *p){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  തിരികെ SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  തിരികെ p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonSkipLabel(JsonEachCursor *p){
  എങ്കിൽ( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    എങ്കിൽ( sz >= p->sParse.nBlob ) sz = p->i;
    തിരികെ sz;
  }അല്ലെങ്കിൽ{
    തിരികെ p->i;
  }
}

/*
** Append the path name for the current element.
*/
സ്ഥിതം ശൂന്യം jsonAppendPathName(JsonEachCursor *p){
  പ്രതിജ്ഞാ( p->nParent>0 );
  പ്രതിജ്ഞാ( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  എങ്കിൽ( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_താക്കോൽ);
  }അല്ലെങ്കിൽ{
    u32 n, sz = 0, k, i;
    സ്ഥിരം അക്ഷരം *z;
    പൂർണ്ണസംഖ്യ needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (സ്ഥിരം അക്ഷരം*)&p->sParse.aBlob[k];
    എങ്കിൽ( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }അല്ലെങ്കിൽ{
      വേണ്ടി(i=0; i<sz; i++){
        എങ്കിൽ( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          നിർത്തുക;
        }
      }
    }
    എങ്കിൽ( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }അല്ലെങ്കിൽ{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ json_Each_Malformed_നിവേശം(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_സന്ദേശം);
  cur->pVtab->z_Err_സന്ദേശം = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  തിരികെ cur->pVtab->z_Err_സന്ദേശം ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  എങ്കിൽ( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    എങ്കിൽ( n==0 )തിരികെ json_Each_Malformed_നിവേശം(cur);
    എങ്കിൽ( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      എങ്കിൽ( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, വലുപ്പം(JsonParent)*nNew);
        എങ്കിൽ( pNew==0 ) തിരികെ SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->i_മൂല്യം = i;
      pParent->iEnd = i + n + sz;
      pParent->i_താക്കോൽ = -1;
      pParent->nPath = (u32)p->path.nUsed;
      എങ്കിൽ( p->eType && p->nParent ){
        jsonAppendPathName(p);
        എങ്കിൽ( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }അല്ലെങ്കിൽ{
      p->i = i + n + sz;
    }
    ആകുമ്പോൾ( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    എങ്കിൽ( levelChange ){
      എങ്കിൽ( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->i_മൂല്യം;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }അല്ലെങ്കിൽ{
        p->eType = 0;
      }
    }
  }അല്ലെങ്കിൽ{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    എങ്കിൽ( n==0 )തിരികെ json_Each_Malformed_നിവേശം(cur);
    p->i = i + n + sz;
  }
  എങ്കിൽ( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_താക്കോൽ++;
  }
  p->iRowid++;
  തിരികെ rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  അക്ഷരം *z = p->path.zBuf;
  എങ്കിൽ( p->iRowid==0 && p->bRecursive && n>=2 ){
    ആകുമ്പോൾ( ALWAYS(n>1) ){
      n--;
      എങ്കിൽ( z[n]=='[' || z[n]=='.' ){
        u32 x;
        അക്ഷരം cSaved = z[n];
        z[n] = 0;
        പ്രതിജ്ഞാ( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        എങ്കിൽ( !JSON_LOOKUP_ISERROR(x) ) നിർത്തുക;
      }
    }
  }
  തിരികെ n;
}

/* Return the value of a column */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  പൂർണ്ണസംഖ്യ iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  തിരഞ്ഞെടുപ്പ്( iColumn ){
    അവസ്ഥ JEACH_താക്കോൽ: {
      എങ്കിൽ( p->nParent==0 ){
        u32 n, j;
        എങ്കിൽ( p->n_വേര്==1 ) നിർത്തുക;
        j = jsonEachPathLength(p);
        n = p->n_വേര് - j;
        എങ്കിൽ( n==0 ){
          നിർത്തുക;
        }അല്ലെങ്കിൽ എങ്കിൽ( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }അല്ലെങ്കിൽ എങ്കിൽ( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }അല്ലെങ്കിൽ{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        നിർത്തുക;
      }
      എങ്കിൽ( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }അല്ലെങ്കിൽ{
        പ്രതിജ്ഞാ( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].i_താക്കോൽ);
      }
      നിർത്തുക;
    }
    അവസ്ഥ JEACH_മൂല്യം: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      എങ്കിൽ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      നിർത്തുക;
    }
    അവസ്ഥ JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      നിർത്തുക;
    }
    അവസ്ഥ JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      എങ്കിൽ( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      നിർത്തുക;
    }
    അവസ്ഥ JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      നിർത്തുക;
    }
    അവസ്ഥ JEACH_PARENT: {
      എങ്കിൽ( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      നിർത്തുക;
    }
    അവസ്ഥ JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      എങ്കിൽ( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      നിർത്തുക;
    }
    അവസ്ഥ JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      sqlite3_result_text(ctx, p->path.zBuf, p->n_വേര്, SQLITE_TRANSIENT);
      നിർത്തുക;
    }
    അവസ്ഥ JEACH_JSON: {
      എങ്കിൽ( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }അല്ലെങ്കിൽ{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      നിർത്തുക;
    }
  }
  തിരികെ SQLITE_OK;
}

/* Return the current rowid value */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  തിരികെ SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  പൂർണ്ണസംഖ്യ i;                     /* Loop counter or computed array index */
  പൂർണ്ണസംഖ്യ aIdx[2];               /* Index of constraints for JSON and ROOT */
  പൂർണ്ണസംഖ്യ unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  പൂർണ്ണസംഖ്യ idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  സ്ഥിരം ഘടന sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  പ്രതിജ്ഞാ( JEACH_വേര് == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  വേണ്ടി(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    പൂർണ്ണസംഖ്യ iCol;
    പൂർണ്ണസംഖ്യ iMask;
    എങ്കിൽ( pConstraint->iColumn < JEACH_JSON ) തുടരുക;
    iCol = pConstraint->iColumn - JEACH_JSON;
    പ്രതിജ്ഞാ( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    എങ്കിൽ( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }അല്ലെങ്കിൽ എങ്കിൽ( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  എങ്കിൽ( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  എങ്കിൽ( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    തിരികെ SQLITE_CONSTRAINT;
  }
  എങ്കിൽ( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_അക്കം = 0;
  }അല്ലെങ്കിൽ{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    എങ്കിൽ( aIdx[1]<0 ){
      pIdxInfo->idx_അക്കം = 1;  /* Only JSON supplied.  Plan 1 */
    }അല്ലെങ്കിൽ{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_അക്കം = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  തിരികെ SQLITE_OK;
}

/* Start a search on a new JSON string */
സ്ഥിതം പൂർണ്ണസംഖ്യ jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  സ്ഥിരം അക്ഷരം *z_വേര് = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  എങ്കിൽ( idx_അക്കം==0 ) തിരികെ SQLITE_OK;
  memset(&p->sParse, 0, വലുപ്പം(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  എങ്കിൽ( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }അല്ലെങ്കിൽ{
    p->sParse.zJson = (അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    p->sParse.nJson = sqlite3_മൂല്യം_bytes(argv[0]);
    എങ്കിൽ( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      തിരികെ SQLITE_OK;
    }      
    എങ്കിൽ( jsonConvertTextToBlob(&p->sParse, 0) ){
      എങ്കിൽ( p->sParse.oom ){
        തിരികെ SQLITE_NOMEM;
      }
      തിരികെ json_Each_Malformed_നിവേശം(cur);
    }
  }
  എങ്കിൽ( idx_അക്കം==3 ){
    z_വേര് = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[1]);
    എങ്കിൽ( z_വേര്==0 ) തിരികെ SQLITE_OK;
    എങ്കിൽ( z_വേര്[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_സന്ദേശം);
      cur->pVtab->z_Err_സന്ദേശം = jsonBadPathError(0, z_വേര്, 0);
      jsonEachCursorReset(p);
      തിരികെ cur->pVtab->z_Err_സന്ദേശം ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_വേര് = sqlite3Strlen30(z_വേര്);
    എങ്കിൽ( z_വേര്[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }അല്ലെങ്കിൽ{
      i = jsonLookupStep(&p->sParse, 0, z_വേര്+1, 0);
      എങ്കിൽ( JSON_LOOKUP_ISERROR(i) ){
        എങ്കിൽ( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          തിരികെ SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_സന്ദേശം);
        cur->pVtab->z_Err_സന്ദേശം = jsonBadPathError(0, z_വേര്, 0);
        jsonEachCursorReset(p);
        തിരികെ cur->pVtab->z_Err_സന്ദേശം ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      എങ്കിൽ( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }അല്ലെങ്കിൽ{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_വേര്, p->n_വേര്);
  }അല്ലെങ്കിൽ{
    i = p->i = 0;
    p->eType = 0;
    p->n_വേര് = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  എങ്കിൽ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, വലുപ്പം(JsonParent));
    എങ്കിൽ( p->aParent==0 ) തിരികെ SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_താക്കോൽ = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].i_മൂല്യം = i;
  }
  തിരികെ SQLITE_OK;
}

/* The methods of the json_each virtual table */
സ്ഥിതം sqlite3_module jsonEachModule = {
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
ശൂന്യം sqlite3RegisterJsonFunctions(ശൂന്യം){
#ifndef SQLITE_OMIT_JSON
  സ്ഥിതം FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_തിരുകുക, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_തിരുകുക,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_തിരുകുക,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_തിരുകുക,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#എങ്കിൽ SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_മൂല്യം, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_മൂല്യം, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_മൂല്യം, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_മൂല്യം, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_തിരുകുക_Builtin_Funcs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#എങ്കിൽ  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, സ്ഥിരം അക്ഷരം *zName){
  unsigned പൂർണ്ണസംഖ്യ i;
  സ്ഥിതം സ്ഥിരം അക്ഷരം *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  പ്രതിജ്ഞാ( sqlite3HashFind(&db->aModule, zName)==0 );
  വേണ്ടി(i=0; i<വലുപ്പം(azModule)/വലുപ്പം(azModule[0]); i++){
    എങ്കിൽ( sqlite3StrICmp(azModule[i],zName)==0 ){
      തിരികെ sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  തിരികെ 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
