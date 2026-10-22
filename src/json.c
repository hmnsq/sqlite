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
#अन्तर्भाव "sqliteInt.h"

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
स्थिर स्थिरांक अक्षर * स्थिरांक jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
स्थिर स्थिरांक अक्षर jsonIsSpace[] = {
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
#define jsonIsspace(x) (jsonIsSpace[(अचिन्हित अक्षर)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
स्थिर स्थिरांक अक्षर jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
स्थिर स्थिरांक अक्षर jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
स्थिर स्थिरांक अक्षर jsonIsOk[256] = {
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
प्रकारव्याख्या संरचना JsonCache JsonCache;
प्रकारव्याख्या संरचना JsonString JsonString;
प्रकारव्याख्या संरचना JsonParse JsonParse;

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
संरचना JsonCache {
  sqlite3 *db;                    /* Database connection */
  पूर्णांक nUsed;                      /* Number of active entries in the cache */
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
संरचना JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  अक्षर *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  अक्षर zSpace[100];        /* Initial static space */
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

#define JSON_घाला_TYPE(X)  (((X)&0xC)>>2)


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
संरचना JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  अक्षर *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  पूर्णांक nJson;         /* Length of the zJson string in bytes */
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
  पूर्णांक delta;         /* Size change due to the edit */
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
#अन्यथा
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
स्थिर शून्य jsonReturnStringAsBlob(JsonString*);
स्थिर पूर्णांक jsonArgIsJsonb(sqlite3_मूल्य *pJson, JsonParse *p);
स्थिर u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
स्थिर शून्य jsonReturnParse(sqlite3_context*,JsonParse*);
स्थिर JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_मूल्य*,u32);
स्थिर शून्य jsonParseFree(JsonParse*);
स्थिर u32 jsonbPayloadSize(स्थिरांक JsonParse*, u32, u32*);
स्थिर u32 jsonUnescapeOneChar(स्थिरांक अक्षर*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
स्थिर शून्य jsonCacheDelete(JsonCache *p){
  पूर्णांक i;
  साठी(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
स्थिर शून्य jsonCacheDeleteGeneric(शून्य *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
स्थिर पूर्णांक json_Cache_घाला(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  प्रतिज्ञा( pParse->zJson!=0 );
  प्रतिज्ञा( pParse->bJsonIsRCStr );
  प्रतिज्ञा( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  जर( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, आकार(*p));
    जर( p==0 ) परत SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    जर( p==0 ) परत SQLITE_NOMEM;
  }
  जर( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    स_हटाओ(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*आकार(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  प्रतिज्ञा( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  परत SQLITE_OK;
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
स्थिर JsonParse *json_Cache_शोध(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_मूल्य *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  पूर्णांक i;
  स्थिरांक अक्षर *zJson;
  पूर्णांक nJson;

  जर( sqlite3_मूल्य_type(pArg)!=SQLITE_TEXT ){
    परत 0;
  }
  zJson = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(pArg);
  जर( zJson==0 ) परत 0;
  nJson = sqlite3_मूल्य_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  जर( p==0 ){
    परत 0;
  }
  साठी(i=0; i<p->nUsed; i++){
    जर( p->a[i]->zJson==zJson ) थांबा;
  }
  जर( i>=p->nUsed ){
    साठी(i=0; i<p->nUsed; i++){
      जर( p->a[i]->nJson!=nJson ) सुरूठेवा;
      जर( स_भेद(p->a[i]->zJson, zJson, nJson)==0 ) थांबा;
    }
  }
  जर( i<p->nUsed ){
    जर( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      स_हटाओ(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*आकार(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    प्रतिज्ञा( p->a[i]->delta==0 );
    परत p->a[i];
  }अन्यथा{
    परत 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
स्थिर शून्य jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = आकार(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
स्थिर शून्य jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
स्थिर शून्य jsonStringReset(JsonString *p){
  जर( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
स्थिर शून्य jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  जर( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
स्थिर शून्य jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  प्रतिज्ञा( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
स्थिर पूर्णांक jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  अक्षर *zNew;
  जर( p->bStatic ){
    जर( p->eErr ) परत 1;
    zNew = sqlite3RCStrNew(nTotal);
    जर( zNew==0 ){
      jsonStringOom(p);
      परत SQLITE_NOMEM;
    }
    स_नकल(zNew, p->zBuf, (माप_प्रकार)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }अन्यथा{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    जर( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      परत SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  परत SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
स्थिर SQLITE_NOINLINE शून्य jsonStringExpandAndAppend(
  JsonString *p,
  स्थिरांक अक्षर *zIn,
  u32 N
){
  प्रतिज्ञा( N>0 );
  जर( jsonStringGrow(p,N) ) परत;
  स_नकल(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
स्थिर शून्य jsonAppendRaw(JsonString *p, स्थिरांक अक्षर *zIn, u32 N){
  जर( N==0 ) परत;
  जर( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }अन्यथा{
    स_नकल(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
स्थिर शून्य jsonAppendRawNZ(JsonString *p, स्थिरांक अक्षर *zIn, u32 N){
  प्रतिज्ञा( N>0 );
  जर( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }अन्यथा{
    स_नकल(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
स्थिर शून्य jsonPrintf(पूर्णांक N, JsonString *p, स्थिरांक अक्षर *zFormat, ...){
  बहु_सूची ap;
  जर( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) परत;
  बहु_शुरू(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  बहु_पूर्ण(ap);
  p->nUsed += (पूर्णांक)म_माप(p->zBuf+p->nUsed);
}

/* Append a single character
*/
स्थिर SQLITE_NOINLINE शून्य jsonAppendCharExpand(JsonString *p, अक्षर c){
  जर( jsonStringGrow(p,1) ) परत;
  p->zBuf[p->nUsed++] = c;
}
स्थिर शून्य jsonAppendChar(JsonString *p, अक्षर c){
  जर( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }अन्यथा{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
स्थिर शून्य jsonStringTrimOneChar(JsonString *p){
  जर( p->eErr==0 ){
    प्रतिज्ञा( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
स्थिर पूर्णांक jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  परत p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
स्थिर शून्य jsonAppendSeparator(JsonString *p){
  अक्षर c;
  जर( p->nUsed==0 ) परत;
  c = p->zBuf[p->nUsed-1];
  जर( c=='[' || c=='{' ) परत;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
स्थिर शून्य jsonAppendControlChar(JsonString *p, u8 c){
  स्थिर स्थिरांक अक्षर aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  प्रतिज्ञा( आकार(aSpecial)==32 );
  प्रतिज्ञा( aSpecial['\b']=='b' );
  प्रतिज्ञा( aSpecial['\f']=='f' );
  प्रतिज्ञा( aSpecial['\n']=='n' );
  प्रतिज्ञा( aSpecial['\r']=='r' );
  प्रतिज्ञा( aSpecial['\t']=='t' );
  प्रतिज्ञा( c>=0 && c<आकार(aSpecial) );
  प्रतिज्ञा( p->nUsed+7 <= p->nAlloc );
  जर( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }अन्यथा{
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
स्थिर शून्य jsonAppendString(JsonString *p, स्थिरांक अक्षर *zIn, u32 N){
  u32 k;
  u8 c;
  स्थिरांक u8 *z = (स्थिरांक u8*)zIn;
  जर( z==0 ) परत;
  जर( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) परत;
  p->zBuf[p->nUsed++] = '"';
  पर्यंत( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    पर्यंत( 1 /* Exit by break */ ){
      जर( k+3>=N ){
        पर्यंत( k<N && jsonIsOk[z[k]] ){ k++; }
        थांबा;
      }
      जर( !jsonIsOk[z[k]] ){
        थांबा;
      }
      जर( !jsonIsOk[z[k+1]] ){
        k += 1;
        थांबा;
      }
      जर( !jsonIsOk[z[k+2]] ){
        k += 2;
        थांबा;
      }
      जर( !jsonIsOk[z[k+3]] ){
        k += 3;
        थांबा;
      }अन्यथा{
        k += 4;
      }
    }
    जर( k>=N ){
      जर( k>0 ){
        स_नकल(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      थांबा;
    }
    जर( k>0 ){
      स_नकल(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    जर( c=='"' || c=='\\' ){
      जर( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) परत;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }अन्यथा जर( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }अन्यथा{
      जर( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) परत;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  प्रतिज्ञा( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
स्थिर शून्य json_Append_Sql_मूल्य(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_मूल्य *p_मूल्य          /* Value to append */
){
  निवड( sqlite3_मूल्य_type(p_मूल्य) ){
    स्थिती SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      थांबा;
    }
    स्थिती SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_मूल्य_double(p_मूल्य));
      थांबा;
    }
    स्थिती SQLITE_INTEGER: {
      स्थिरांक अक्षर *z = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(p_मूल्य);
      u32 n = (u32)sqlite3_मूल्य_bytes(p_मूल्य);
      jsonAppendRaw(p, z, n);
      थांबा;
    }
    स्थिती SQLITE_TEXT: {
      स्थिरांक अक्षर *z = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(p_मूल्य);
      u32 n = (u32)sqlite3_मूल्य_bytes(p_मूल्य);
      जर( sqlite3_मूल्य_subtype(p_मूल्य)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }अन्यथा{
        jsonAppendString(p, z, n);
      }
      थांबा;
    }
    पूर्वनिर्धारित: {
      JsonParse px;
      स_रखो(&px, 0, आकार(px));
      जर( jsonArgIsJsonb(p_मूल्य, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }अन्यथा जर( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      थांबा;
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
स्थिर शून्य jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  प्रतिज्ञा( (pParse!=0)==(ctx!=0) );
  प्रतिज्ञा( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  जर( p->eErr==0 ){
    पूर्णांक flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    जर( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }अन्यथा जर( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }अन्यथा{
      जर( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        पूर्णांक rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_घाला(ctx, pParse);
        जर( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          परत;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }अन्यथा जर( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }अन्यथा जर( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }अन्यथा जर( p->eErr & JSTRING_MALFORMED ){
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
स्थिर शून्य jsonParseReset(JsonParse *pParse){
  प्रतिज्ञा( pParse->nJPRef<=1 );
  जर( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  जर( pParse->nBlobAlloc ){
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
स्थिर शून्य jsonParseFree(JsonParse *pParse){
  जर( pParse ){
    जर( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }अन्यथा{
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
स्थिर u8 jsonHexToInt(पूर्णांक h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  परत (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
स्थिर u32 jsonHexToInt4(स्थिरांक अक्षर *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  परत v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
स्थिर पूर्णांक jsonIs2Hex(स्थिरांक अक्षर *z){
  परत sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
स्थिर पूर्णांक jsonIs4Hex(स्थिरांक अक्षर *z){
  परत jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
स्थिर पूर्णांक json5Whitespace(स्थिरांक अक्षर *zIn){
  पूर्णांक n = 0;
  स्थिरांक u8 *z = (u8*)zIn;
  पर्यंत( 1 /*exit by "goto whitespace_done"*/ ){
    निवड( z[n] ){
      स्थिती 0x09:
      स्थिती 0x0a:
      स्थिती 0x0b:
      स्थिती 0x0c:
      स्थिती 0x0d:
      स्थिती 0x20: {
        n++;
        थांबा;
      }
      स्थिती '/': {
        जर( z[n+1]=='*' && z[n+2]!=0 ){
          पूर्णांक j;
          साठी(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            जर( z[j]==0 ) जा whitespace_done;
          }
          n = j+1;
          थांबा;
        }अन्यथा जर( z[n+1]=='/' ){
          पूर्णांक j;
          अक्षर c;
          साठी(j=n+2; (c = z[j])!=0; j++){
            जर( c=='\n' || c=='\r' ) थांबा;
            जर( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              थांबा;
            }
          }
          n = j;
          जर( z[n] ) n++;
          थांबा;
        }
        जा whitespace_done;
      }
      स्थिती 0xc2: {
        जर( z[n+1]==0xa0 ){
          n += 2;
          थांबा;
        }
        जा whitespace_done;
      }
      स्थिती 0xe1: {
        जर( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          थांबा;
        }
        जा whitespace_done;
      }
      स्थिती 0xe2: {
        जर( z[n+1]==0x80 ){
          u8 c = z[n+2];
          जर( c<0x80 ) जा whitespace_done;
          जर( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            थांबा;
          }
        }अन्यथा जर( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          थांबा;
        }
        जा whitespace_done;
      }
      स्थिती 0xe3: {
        जर( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          थांबा;
        }
        जा whitespace_done;
      }
      स्थिती 0xef: {
        जर( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          थांबा;
        }
        जा whitespace_done;
      }
      पूर्वनिर्धारित: {
        जा whitespace_done;
      }
    }
  }
  whitespace_done:
  परत n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
स्थिर स्थिरांक संरचना NanInfName {
  अक्षर c1;
  अक्षर c2;
  अक्षर n;
  अक्षर eType;
  अक्षर nRepl;
  अक्षर *zMatch;
  अक्षर *zRepl;
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
स्थिर शून्य json_Wrong_अंक_Args(
  sqlite3_context *pCtx,
  स्थिरांक अक्षर *zFuncName
){
  अक्षर *z_संदेश = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, z_संदेश, -1);
  sqlite3_free(z_संदेश);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
स्थिर पूर्णांक jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  प्रतिज्ञा( N>pParse->nBlobAlloc );
  जर( pParse->nBlobAlloc==0 ){
    t = 100;
  }अन्यथा{
    t = pParse->nBlobAlloc*2;
  }
  जर( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  जर( aNew==0 ){ pParse->oom = 1; परत 1; }
  प्रतिज्ञा( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  परत 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
स्थिर पूर्णांक jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 nSize;
  प्रतिज्ञा( !pParse->bReadOnly );
  जर( pParse->oom ) परत 0;
  जर( pParse->nBlobAlloc>0 ) परत 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  जर( jsonBlobExpand(pParse, nSize) ){
    परत 0;
  }
  प्रतिज्ञा( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  स_नकल(pParse->aBlob, aOld, pParse->nBlob);
  परत 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
स्थिर SQLITE_NOINLINE शून्य jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  जर( pParse->oom==0 ){
    प्रतिज्ञा( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
स्थिर शून्य jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  जर( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }अन्यथा{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
स्थिर शून्य json_Blob_Append_गाठ(JsonParse*,u8,u64,स्थिरांक शून्य*);
स्थिर SQLITE_NOINLINE शून्य json_Blob_Expand_And_Append_गाठ(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  स्थिरांक शून्य *aPayload
){
  जर( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) परत;
  json_Blob_Append_गाठ(pParse, eType, szPayload, aPayload);
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
स्थिर शून्य json_Blob_Append_गाठ(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  स्थिरांक शून्य *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  जर( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_गाठ(pParse,eType,szPayload,aPayload);
    परत;
  }
  प्रतिज्ञा( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  जर( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }अन्यथा जर( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }अन्यथा जर( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }अन्यथा{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  जर( aPayload ){
    pParse->nBlob += szPayload;
    स_नकल(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
स्थिर पूर्णांक jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  पूर्णांक delta;
  जर( pParse->oom ) परत 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  जर( szType<=11 ){
    nExtra = 0;
  }अन्यथा जर( szType==12 ){
    nExtra = 1;
  }अन्यथा जर( szType==13 ){
    nExtra = 2;
  }अन्यथा जर( szType==14 ){
    nExtra = 4;
  }अन्यथा{
    nExtra = 8;
  }
  जर( szPayload<=11 ){
    nNeeded = 0;
  }अन्यथा जर( szPayload<=0xff ){
    nNeeded = 1;
  }अन्यथा जर( szPayload<=0xffff ){
    nNeeded = 2;
  }अन्यथा{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  जर( delta ){
    u64 newSize = (u64)pParse->nBlob + delta;
    जर( delta>0 ){
      जर( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        परत 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      स_हटाओ(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }अन्यथा{
      स_हटाओ(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  जर( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }अन्यथा जर( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }अन्यथा जर( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }अन्यथा{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  परत delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
स्थिर पूर्णांक jsonIs4HexB(स्थिरांक अक्षर *z, पूर्णांक *pOp){
  जर( z[0]!='u' ) परत 0;
  जर( !jsonIs4Hex(&z[1]) ) परत 0;
  *pOp = JSONB_TEXTJ;
  परत 1;
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
स्थिर u32 jsonbValidityCheck(
  स्थिरांक JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  स्थिरांक u8 *z;
  u8 x;
  जर( iDepth>JSON_MAX_DEPTH ) परत i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  जर( NEVER(n==0) ) परत i+1;          /* Checked by caller */
  जर( NEVER(i+n+sz!=iEnd) ) परत i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  निवड( x ){
    स्थिती JSONB_NULL:
    स्थिती JSONB_TRUE:
    स्थिती JSONB_FALSE: {
      परत n+sz==1 ? 0 : i+1;
    }
    स्थिती JSONB_INT: {
      जर( sz<1 ) परत i+1;
      j = i+n;
      जर( z[j]=='-' ){
        j++;
        जर( sz<2 ) परत i+1;
      }
      k = i+n+sz;
      पर्यंत( j<k ){
        जर( sqlite3Isdigit(z[j]) ){
          j++;
        }अन्यथा{
          परत j+1;
        }
      }
      परत 0;
    }
    स्थिती JSONB_INT5: {
      जर( sz<3 ) परत i+1;
      j = i+n;
      जर( z[j]=='-' ){
        जर( sz<4 ) परत i+1;
        j++;
      }
      जर( z[j]!='0' ) परत i+1;
      जर( z[j+1]!='x' && z[j+1]!='X' ) परत j+2;
      j += 2;
      k = i+n+sz;
      पर्यंत( j<k ){
        जर( sqlite3Isxdigit(z[j]) ){
          j++;
        }अन्यथा{
          परत j+1;
        }
      }
      परत 0;
    }
    स्थिती JSONB_FLOAT:
    स्थिती JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      जर( sz<2 ) परत i+1;
      j = i+n;
      k = j+sz;
      जर( z[j]=='-' ){
        j++;
        जर( sz<3 ) परत i+1;
      }
      जर( z[j]=='.' ){
        जर( x==JSONB_FLOAT ) परत j+1;
        जर( !sqlite3Isdigit(z[j+1]) ) परत j+1;
        j += 2;
        seen = 1;
      }अन्यथा जर( z[j]=='0' && x==JSONB_FLOAT ){
        जर( j+3>k ) परत j+1;
        जर( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) परत j+1;
        j++;
      }
      साठी(; j<k; j++){
        जर( sqlite3Isdigit(z[j]) ) सुरूठेवा;
        जर( z[j]=='.' ){
          जर( seen>0 ) परत j+1;
          जर( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            परत j+1;
          }
          seen = 1;
          सुरूठेवा;
        }
        जर( z[j]=='e' || z[j]=='E' ){
          जर( seen==2 ) परत j+1;
          जर( j==k-1 ) परत j+1;
          जर( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            जर( j==k-1 ) परत j+1;
          }
          seen = 2;
          सुरूठेवा;
        }
        परत j+1;
      }
      जर( seen==0 ) परत i+1;
      परत 0;
    }
    स्थिती JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      पर्यंत( j<k ){
        जर( !jsonIsOk[z[j]] && z[j]!='\'' ) परत j+1;
        j++;
      }
      परत 0;
    }
    स्थिती JSONB_TEXTJ:
    स्थिती JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      पर्यंत( j<k ){
        जर( !jsonIsOk[z[j]] && z[j]!='\'' ){
          जर( z[j]=='"' ){
            जर( x==JSONB_TEXTJ ) परत j+1;
          }अन्यथा जर( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            जर( x==JSONB_TEXTJ ) परत j+1;
          }अन्यथा जर( NEVER(z[j]!='\\') || j+1>=k ){
            परत j+1;
          }अन्यथा जर( म_अक्षर("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }अन्यथा जर( z[j+1]=='u' ){
            जर( j+5>=k ) परत j+1;
            जर( !jsonIs4Hex((स्थिरांक अक्षर*)&z[j+2]) ) परत j+1;
            j++;
          }अन्यथा जर( x!=JSONB_TEXT5 ){
            परत j+1;
          }अन्यथा{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((स्थिरांक अक्षर*)&z[j], k-j, &c);
            जर( c==JSON_INVALID_CHAR ) परत j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      परत 0;
    }
    स्थिती JSONB_TEXTRAW: {
      परत 0;
    }
    स्थिती JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      पर्यंत( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        जर( n==0 ) परत j+1;
        जर( j+n+sz>k ) परत j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        जर( sub ) परत sub;
        j += n + sz;
      }
      प्रतिज्ञा( j==k );
      परत 0;
    }
    स्थिती JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      पर्यंत( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        जर( n==0 ) परत j+1;
        जर( j+n+sz>k ) परत j+1;
        जर( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          जर( x<JSONB_TEXT || x>JSONB_TEXTRAW ) परत j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        जर( sub ) परत sub;
        cnt++;
        j += n + sz;
      }
      प्रतिज्ञा( j==k );
      जर( (cnt & 1)!=0 ) परत j+1;
      परत 0;
    }
    पूर्वनिर्धारित: {
      परत i+1;
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
स्थिर पूर्णांक jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  अक्षर c;
  u32 j;
  u32 iThis, iStart;
  पूर्णांक x;
  u8 t;
  स्थिरांक अक्षर *z = pParse->zJson;
json_parse_restart:
  निवड( (u8)z[i] ){
  स्थिती '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_गाठ(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    जर( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      परत -1;
    }
    iStart = pParse->nBlob;
    साठी(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      जर( x<=0 ){
        पूर्णांक op;
        जर( x==(-2) ){
          j = pParse->iErr;
          जर( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          थांबा;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        जर( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          पूर्णांक k = j+1;
          पर्यंत( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          प्रतिज्ञा( iBlob==pParse->nBlob );
          json_Blob_Append_गाठ(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }अन्यथा{
          जर( x!=-1 ) pParse->iErr = j;
          परत -1;
        }
      }
      जर( pParse->oom ) परत -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      जर( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        परत -1;
      }
      j = x;
      जर( z[j]==':' ){
        j++;
      }अन्यथा{
        जर( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          करा{ j++; }पर्यंत( jsonIsspace(z[j]) );
          जर( z[j]==':' ){
            j++;
            जा parse_object_मूल्य;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        जर( x!=(-5) ){
          जर( x!=(-1) ) pParse->iErr = j;
          परत -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_मूल्य:
      x = jsonTranslateTextToBlob(pParse, j);
      जर( x<=0 ){
        जर( x!=(-1) ) pParse->iErr = j;
        परत -1;
      }
      j = x;
      जर( z[j]==',' ){
        सुरूठेवा;
      }अन्यथा जर( z[j]=='}' ){
        थांबा;
      }अन्यथा{
        जर( jsonIsspace(z[j]) ){
          j += 1 + (u32)म_अखोज(&z[j+1], jsonSpaces);
          जर( z[j]==',' ){
            सुरूठेवा;
          }अन्यथा जर( z[j]=='}' ){
            थांबा;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        जर( x==(-4) ){
          j = pParse->iErr;
          सुरूठेवा;
        }
        जर( x==(-2) ){
          j = pParse->iErr;
          थांबा;
        }
      }
      pParse->iErr = j;
      परत -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    परत j+1;
  }
  स्थिती '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    प्रतिज्ञा( i<=(u32)pParse->nJson );
    json_Blob_Append_गाठ(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    जर( pParse->oom ) परत -1;
    जर( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      परत -1;
    }
    साठी(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      जर( x<=0 ){
        जर( x==(-3) ){
          j = pParse->iErr;
          जर( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          थांबा;
        }
        जर( x!=(-1) ) pParse->iErr = j;
        परत -1;
      }
      j = x;
      जर( z[j]==',' ){
        सुरूठेवा;
      }अन्यथा जर( z[j]==']' ){
        थांबा;
      }अन्यथा{
        जर( jsonIsspace(z[j]) ){
          j += 1 + (u32)म_अखोज(&z[j+1], jsonSpaces);
          जर( z[j]==',' ){
            सुरूठेवा;
          }अन्यथा जर( z[j]==']' ){
            थांबा;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        जर( x==(-4) ){
          j = pParse->iErr;
          सुरूठेवा;
        }
        जर( x==(-3) ){
          j = pParse->iErr;
          थांबा;
        }
      }
      pParse->iErr = j;
      परत -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    परत j+1;
  }
  स्थिती '\'': {
    u8 opcode;
    अक्षर cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    जा parse_string;
  स्थिती '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    पर्यंत( 1 /*exit-by-break*/ ){
      जर( jsonIsOk[(u8)z[j]] ){
        जर( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }अन्यथा जर( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }अन्यथा{
          j += 3;
          सुरूठेवा;
        }
      }
      c = z[j];
      जर( c==cDelim ){
        थांबा;
      }अन्यथा जर( c=='\\' ){
        c = z[++j];
        जर( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          जर( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }अन्यथा जर( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#अन्यथा
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }अन्यथा जर( c=='\r' ){
          जर( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }अन्यथा{
          pParse->iErr = j;
          परत -1;
        }
      }अन्यथा जर( c<=0x1f ){
        जर( c==0 ){
          pParse->iErr = j;
          परत -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }अन्यथा जर( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_गाठ(pParse, opcode, j-1-i, &z[i+1]);
    परत j+1;
  }
  स्थिती 't': {
    जर( म_भेदन(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      परत i+4;
    }
    pParse->iErr = i;
    परत -1;
  }
  स्थिती 'f': {
    जर( म_भेदन(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      परत i+5;
    }
    pParse->iErr = i;
    परत -1;
  }
  स्थिती '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    जा parse_संख्या;
  स्थिती '.':
    जर( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      जा parse_संख्या_2;
    }
    pParse->iErr = i;
    परत -1;
  स्थिती '-':
  स्थिती '0':
  स्थिती '1':
  स्थिती '2':
  स्थिती '3':
  स्थिती '4':
  स्थिती '5':
  स्थिती '6':
  स्थिती '7':
  स्थिती '8':
  स्थिती '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_संख्या:
    seenE = 0;
    प्रतिज्ञा( '-' < '0' );
    प्रतिज्ञा( '+' < '0' );
    प्रतिज्ञा( '.' < '0' );
    c = z[i];

    जर( c<='0' ){
      जर( c=='0' ){
        जर( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          प्रतिज्ञा( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          साठी(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          जा parse_संख्या_finish;
        }अन्यथा जर( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          परत -1;
        }
      }अन्यथा{
        जर( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          जर( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            जर( z[i]=='-' ){
              json_Blob_Append_गाठ(pParse, JSONB_FLOAT, 6, "-9e999");
            }अन्यथा{
              json_Blob_Append_गाठ(pParse, JSONB_FLOAT, 5, "9e999");
            }
            परत i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          जर( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            जा parse_संख्या_2;
          }
          pParse->iErr = i;
          परत -1;
        }
        जर( z[i+1]=='0' ){
          जर( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            परत -1;
          }अन्यथा जर( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            साठी(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            जा parse_संख्या_finish;
          }
        }
      }
    }

  parse_संख्या_2:
    साठी(j=i+1;; j++){
      c = z[j];
      जर( sqlite3Isdigit(c) ) सुरूठेवा;
      जर( c=='.' ){
        जर( (t & 0x02)!=0 ){
          pParse->iErr = j;
          परत -1;
        }
        t |= 0x02;
        सुरूठेवा;
      }
      जर( c=='e' || c=='E' ){
        जर( z[j-1]<'0' ){
          जर( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }अन्यथा{
            pParse->iErr = j;
            परत -1;
          }
        }
        जर( seenE ){
          pParse->iErr = j;
          परत -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        जर( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        जर( c<'0' || c>'9' ){
          pParse->iErr = j;
          परत -1;
        }
        सुरूठेवा;
      }
      थांबा;
    }
    जर( z[j-1]<'0' ){
      जर( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }अन्यथा{
        pParse->iErr = j;
        परत -1;
      }
    }
  parse_संख्या_finish:
    प्रतिज्ञा( JSONB_INT+0x01==JSONB_INT5 );
    प्रतिज्ञा( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    प्रतिज्ञा( JSONB_INT+0x02==JSONB_FLOAT );
    जर( z[i]=='+' ) i++;
    json_Blob_Append_गाठ(pParse, JSONB_INT+t, j-i, &z[i]);
    परत j;
  }
  स्थिती '}': {
    pParse->iErr = i;
    परत -2;  /* End of {...} */
  }
  स्थिती ']': {
    pParse->iErr = i;
    परत -3;  /* End of [...] */
  }
  स्थिती ',': {
    pParse->iErr = i;
    परत -4;  /* List separator */
  }
  स्थिती ':': {
    pParse->iErr = i;
    परत -5;  /* Object label/value separator */
  }
  स्थिती 0: {
    परत 0;   /* End of file */
  }
  स्थिती 0x09:
  स्थिती 0x0a:
  स्थिती 0x0d:
  स्थिती 0x20: {
    i += 1 + (u32)म_अखोज(&z[i+1], jsonSpaces);
    जा json_parse_restart;
  }
  स्थिती 0x0b:
  स्थिती 0x0c:
  स्थिती '/':
  स्थिती 0xc2:
  स्थिती 0xe1:
  स्थिती 0xe2:
  स्थिती 0xe3:
  स्थिती 0xef: {
    j = json5Whitespace(&z[i]);
    जर( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      जा json_parse_restart;
    }
    pParse->iErr = i;
    परत -1;
  }
  स्थिती 'n': {
    जर( म_भेदन(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      परत i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  पूर्वनिर्धारित: {
    u32 k;
    पूर्णांक nn;
    c = z[i];
    साठी(k=0; k<आकार(aNanInfName)/आकार(aNanInfName[0]); k++){
      जर( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) सुरूठेवा;
      nn = aNanInfName[k].n;
      जर( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        सुरूठेवा;
      }
      जर( sqlite3Isalnum(z[i+nn]) ) सुरूठेवा;
      जर( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_गाठ(pParse, JSONB_FLOAT, 5, "9e999");
      }अन्यथा{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      परत i + nn;
    }
    pParse->iErr = i;
    परत -1;  /* Syntax error */
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
स्थिर पूर्णांक jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  पूर्णांक i;
  स्थिरांक अक्षर *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  जर( pParse->oom ) i = -1;
  जर( i>0 ){
#ifdef SQLITE_DEBUG
    प्रतिज्ञा( pParse->iDepth==0 );
    जर( sqlite3Config.bJsonSelfcheck ){
      प्रतिज्ञा( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    पर्यंत( jsonIsspace(zJson[i]) ) i++;
    जर( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      जर( zJson[i] ){
        जर( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        परत 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  जर( i<=0 ){
    जर( pCtx!=0 ){
      जर( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }अन्यथा{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    परत 1;
  }
  परत 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
स्थिर शून्य jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  प्रतिज्ञा( pStr->eErr==0 );
  स_रखो(&px, 0, आकार(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (शून्य)jsonTranslateTextToBlob(&px, 0);
  जर( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }अन्यथा{
    प्रतिज्ञा( px.nBlobAlloc>0 );
    प्रतिज्ञा( !px.bReadOnly );
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
स्थिर u32 jsonbPayloadSizeWide(
  स्थिरांक JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  प्रतिज्ञा( i<pParse->nBlob );
  प्रतिज्ञा( x==(pParse->aBlob[i]>>4) );
  प्रतिज्ञा( x>=13 );
  जर( x==13 ){
    जर( i+2>=pParse->nBlob ){
      *pSz = 0;
      परत 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }अन्यथा जर( x==14 ){
    जर( i+4>=pParse->nBlob ){
      *pSz = 0;
      परत 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }अन्यथा{
    जर( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      परत 0;
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
  जर( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    परत 0;
  }
  *pSz = sz;
  परत n;
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
स्थिर u32 jsonbPayloadSize(
  स्थिरांक JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  जर( i>=pParse->nBlob ){
    *pSz = 0;
    परत 0;
  }अन्यथा जर( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }अन्यथा जर( x==12 ){
    जर( i+1>=pParse->nBlob ){
      *pSz = 0;
      परत 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }अन्यथा{
    परत jsonbPayloadSizeWide(pParse, i, pSz, x);
  }
  जर( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    परत 0;
  }
  *pSz = sz;
  परत n;
}

#जर SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
स्थिर SQLITE_INLINE u32 jsonbPayloadSizeInline(
  स्थिरांक JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  जर( i>=pParse->nBlob ){
    *pSz = 0;
    परत 0;
  }अन्यथा जर( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }अन्यथा जर( x==12 ){
    जर( i+1>=pParse->nBlob ){
      *pSz = 0;
      परत 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }अन्यथा जर( x==13 ){
    जर( i+2>=pParse->nBlob ){
      *pSz = 0;
      परत 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }अन्यथा जर( x==14 ){
    जर( i+4>=pParse->nBlob ){
      *pSz = 0;
      परत 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }अन्यथा{
    जर( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      परत 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  जर( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    परत 0;
  }
  *pSz = sz;
  परत n;
}
#अन्यथा /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif


#जर SQLITE_USES_INLINE
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
स्थिर SQLITE_INLINE u32 jsonbPayloadSizeSemiInline(
  स्थिरांक JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  प्रतिज्ञा( i<pParse->nBlob );
  जर( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }अन्यथा जर( x==12 ){
    जर( i+1>=pParse->nBlob ){
      *pSz = 0;
      परत 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }अन्यथा{
    परत jsonbPayloadSizeWide(pParse,i,pSz,x);
  }
  जर( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    परत 0;
  }
  *pSz = sz;
  परत n;
}
#अन्यथा /* if !SQLITE_USES_INLINE */
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
स्थिर u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSizeInline(pParse, i, &sz);
  जर( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    परत pParse->nBlob+1;
  }
  निवड( pParse->aBlob[i] & 0x0f ){
    स्थिती JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      परत i+1;
    }
    स्थिती JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      परत i+1;
    }
    स्थिती JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      परत i+1;
    }
    स्थिती JSONB_INT:
    स्थिती JSONB_FLOAT: {
      जर( sz==0 ) जा malformed_jsonb;
      jsonAppendRaw(pOut, (स्थिरांक अक्षर*)&pParse->aBlob[i+n], sz);
      थांबा;
    }
    स्थिती JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      स्थिरांक अक्षर *zIn = (स्थिरांक अक्षर*)&pParse->aBlob[i+n];
      पूर्णांक bOverflow = 0;
      जर( sz==0 ) जा malformed_jsonb;
      जर( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }अन्यथा जर( zIn[0]=='+' ){
        k++;
      }
      साठी(; k<sz; k++){
        जर( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          थांबा;
        }अन्यथा जर( (u>>60)!=0 ){
          bOverflow = 1;
        }अन्यथा{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      थांबा;
    }
    स्थिती JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      स्थिरांक अक्षर *zIn = (स्थिरांक अक्षर*)&pParse->aBlob[i+n];
      जर( sz==0 ) जा malformed_jsonb;
      जर( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        जर( sz<=1 ) जा malformed_jsonb;
        k = 1;
      }
      जर( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      साठी(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        जर( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      थांबा;
    }
    स्थिती JSONB_TEXT:
    स्थिती JSONB_TEXTJ: {
      जर( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        स_नकल(pOut->zBuf+pOut->nUsed+1,(स्थिरांक अक्षर*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      थांबा;
    }
    स्थिती JSONB_TEXT5: {
      स्थिरांक अक्षर *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (स्थिरांक अक्षर*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      पर्यंत( sz2>0 ){
        साठी(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        जर( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          जर( k>=sz2 ){
            थांबा;
          }
          zIn += k;
          sz2 -= k;
        }
        जर( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          सुरूठेवा;
        }
        जर( zIn[0]<=0x1f ){
          जर( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) थांबा;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          सुरूठेवा;
        }
        प्रतिज्ञा( zIn[0]=='\\' );
        प्रतिज्ञा( sz2>=1 );
        जर( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          थांबा;
        }
        निवड( (u8)zIn[1] ){
          स्थिती '\'':
            jsonAppendChar(pOut, '\'');
            थांबा;
          स्थिती 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            थांबा;
          स्थिती 'x':
            जर( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              थांबा;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            थांबा;
          स्थिती '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            थांबा;
          स्थिती '\r':
            जर( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            थांबा;
          स्थिती '\n':
            थांबा;
          स्थिती 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            जर( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              थांबा;
            }
            zIn += 2;
            sz2 -= 2;
            थांबा;
          पूर्वनिर्धारित:
            jsonAppendRawNZ(pOut, zIn, 2);
            थांबा;
        }
        प्रतिज्ञा( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      थांबा;
    }
    स्थिती JSONB_TEXTRAW: {
      jsonAppendString(pOut, (स्थिरांक अक्षर*)&pParse->aBlob[i+n], sz);
      थांबा;
    }
    स्थिती JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      जर( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      पर्यंत( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      जर( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      जर( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      थांबा;
    }
    स्थिती JSONB_OBJECT: {
      पूर्णांक x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      जर( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      पर्यंत( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      जर( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      जर( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      थांबा;
    }

    पूर्वनिर्धारित: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      थांबा;
    }
  }
  परत i+n+sz;
}

/* Context for recursion of json_pretty()
*/
प्रकारव्याख्या संरचना JsonPretty JsonPretty;
संरचना JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  स्थिरांक अक्षर *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
स्थिर शून्य jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  साठी(jj=0; jj<pPretty->nIndent; jj++){
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
स्थिर u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  जर( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    परत pParse->nBlob+1;
  }
  निवड( pParse->aBlob[i] & 0x0f ){
    स्थिती JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      जर( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        जर( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        पर्यंत( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          जर( j>=iEnd ){
            जर( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            थांबा;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      थांबा;
    }
    स्थिती JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      जर( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        जर( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        पर्यंत( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          जर( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            थांबा;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          जर( j>=iEnd ) थांबा;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      थांबा;
    }
    पूर्वनिर्धारित: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      थांबा;
    }
  }
  परत i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
स्थिर u32 jsonb_Array_मोजणी(JsonParse *pParse, u32 i_मूळ){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, i_मूळ, &sz);
  iEnd = i_मूळ+n+sz;
  साठी(i=i_मूळ+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSizeSemiInline(pParse, i, &sz);
  }
  परत k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
स्थिर शून्य jsonAfterEditSizeAdjust(JsonParse *pParse, u32 i_मूळ){
  u32 sz = 0;
  u32 nBlob;
  प्रतिज्ञा( pParse->delta!=0 );
  प्रतिज्ञा( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (शून्य)jsonbPayloadSize(pParse, i_मूळ, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, i_मूळ, sz);
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
स्थिर पूर्णांक jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  स्थिरांक u8 *aIns,           /* New content */
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
  स्थिर स्थिरांक u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  जर( (aIns[0]&0x0f)<=2 ) परत 0;    /* Cannot enlarge NULL, true, false */
  निवड( aIns[0]>>4 ){
    पूर्वनिर्धारित: {                         /* aIns[] header size 1 */
      जर( ((1<<d)&0x116)==0 ) परत 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      थांबा;
    }
    स्थिती 12: {                         /* aIns[] header size is 2 */
      जर( ((1<<d)&0x8a)==0) परत 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      थांबा;
    }
    स्थिती 13: {                         /* aIns[] header size is 3 */
      जर( d!=2 && d!=6 ) परत 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      थांबा;
    }
    स्थिती 14: {                         /* aIns[] header size is 5 */
      जर( d!=4 ) परत 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      थांबा;
    }
    स्थिती 15: {                         /* aIns[] header size is 9 */
      परत 0;                          /* No solution */
    }
  }
  प्रतिज्ञा( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  स_नकल(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  पर्यंत( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    जर( i==1 ) थांबा;
    szPayload >>= 8;
  }
  प्रतिज्ञा( (szPayload>>8)==0 );
  परत 1;
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
स्थिर शून्य jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  स्थिरांक u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  प्रतिज्ञा( pParse->nBlob >= (u64)iDel + (u64)nDel );
  जर( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (पूर्णांक)-d)
  ){
    परत;
  }
  जर( d!=0 ){
    जर( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      जर( pParse->oom ) परत;
    }
    स_हटाओ(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  जर( nIns && aIns ){
    स_नकल(&pParse->aBlob[iDel], aIns, nIns);
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
स्थिर u32 jsonBytesToBypass(स्थिरांक अक्षर *z, u32 n){
  u32 i = 0;
  पर्यंत( i+1<n ){
    जर( z[i]!='\\' ) परत i;
    जर( z[i+1]=='\n' ){
      i += 2;
      सुरूठेवा;
    }
    जर( z[i+1]=='\r' ){
      जर( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }अन्यथा{
        i += 2;
      }
      सुरूठेवा;
    }
    जर( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      सुरूठेवा;
    }
    थांबा;
  }
  परत i;
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
स्थिर u32 jsonUnescapeOneChar(स्थिरांक अक्षर *z, u32 n, u32 *piOut){
  प्रतिज्ञा( n>0 );
  प्रतिज्ञा( z[0]=='\\' );
  जर( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    परत n;
  }
  निवड( (u8)z[1] ){
    स्थिती 'u': {
      u32 v, vlo;
      जर( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        परत n;
      }
      v = jsonHexToInt4(&z[2]);
      जर( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        परत 12;
      }अन्यथा{
        *piOut = v;
        परत 6;
      }
    }
    स्थिती 'b': {   *piOut = '\b';  परत 2; }
    स्थिती 'f': {   *piOut = '\f';  परत 2; }
    स्थिती 'n': {   *piOut = '\n';  परत 2; }
    स्थिती 'r': {   *piOut = '\r';  परत 2; }
    स्थिती 't': {   *piOut = '\t';  परत 2; }
    स्थिती 'v': {   *piOut = '\v';  परत 2; }
    स्थिती '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#अन्यथा
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      परत 2;
    }
    स्थिती '\'':
    स्थिती '"':
    स्थिती '/':
    स्थिती '\\':{   *piOut = z[1];  परत 2; }
    स्थिती 'x': {
      जर( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        परत n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      परत 4;
    }
    स्थिती 0xe2:
    स्थिती '\r':
    स्थिती '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      जर( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        परत n;
      }अन्यथा जर( nSkip==n ){
        *piOut = 0;
        परत n;
      }अन्यथा जर( z[nSkip]=='\\' ){
        परत nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }अन्यथा{
        पूर्णांक sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        परत nSkip + sz;
      }
    }
    पूर्वनिर्धारित: {
      *piOut = JSON_INVALID_CHAR;
      परत 2;
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
स्थिर SQLITE_NOINLINE पूर्णांक jsonLabelCompareEscaped(
  स्थिरांक अक्षर *z_डावा,          /* The left label */
  u32 n_डावा,                  /* Size of the left label in bytes */
  पूर्णांक raw_डावा,                /* True if zLeft contains no escapes */
  स्थिरांक अक्षर *z_उजवा,         /* The right label */
  u32 n_उजवा,                 /* Size of the right label in bytes */
  पूर्णांक raw_उजवा                /* True if zRight is escape-free */
){
  u32 c_डावा, c_उजवा;
  प्रतिज्ञा( raw_डावा==0 || raw_उजवा==0 );
  पर्यंत( 1 /*exit-by-return*/ ){
    जर( n_डावा==0 ){
      c_डावा = 0;
    }अन्यथा जर( raw_डावा || z_डावा[0]!='\\' ){
      c_डावा = ((u8*)z_डावा)[0];
      जर( c_डावा>=0xc0 ){
        पूर्णांक sz = sqlite3Utf8ReadLimited((u8*)z_डावा, n_डावा, &c_डावा);
        z_डावा += sz;
        n_डावा -= sz;
      }अन्यथा{
        z_डावा++;
        n_डावा--;
      }
    }अन्यथा{
      u32 n = jsonUnescapeOneChar(z_डावा, n_डावा, &c_डावा);
      z_डावा += n;
      प्रतिज्ञा( n<=n_डावा );
      n_डावा -= n;
    }
    जर( n_उजवा==0 ){
      c_उजवा = 0;
    }अन्यथा जर( raw_उजवा || z_उजवा[0]!='\\' ){
      c_उजवा = ((u8*)z_उजवा)[0];
      जर( c_उजवा>=0xc0 ){
        पूर्णांक sz = sqlite3Utf8ReadLimited((u8*)z_उजवा, n_उजवा, &c_उजवा);
        z_उजवा += sz;
        n_उजवा -= sz;
      }अन्यथा{
        z_उजवा++;
        n_उजवा--;
      }
    }अन्यथा{
      u32 n = jsonUnescapeOneChar(z_उजवा, n_उजवा, &c_उजवा);
      z_उजवा += n;
      प्रतिज्ञा( n<=n_उजवा );
      n_उजवा -= n;
    }
    जर( c_डावा!=c_उजवा ) परत 0;
    जर( c_डावा==0 ) परत 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
स्थिर पूर्णांक jsonLabelCompare(
  स्थिरांक अक्षर *z_डावा,          /* The left label */
  u32 n_डावा,                  /* Size of the left label in bytes */
  पूर्णांक raw_डावा,                /* True if zLeft contains no escapes */
  स्थिरांक अक्षर *z_उजवा,         /* The right label */
  u32 n_उजवा,                 /* Size of the right label in bytes */
  पूर्णांक raw_उजवा                /* True if zRight is escape-free */
){
  जर( raw_डावा && raw_उजवा ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    जर( n_डावा!=n_उजवा ) परत 0;
    परत स_भेद(z_डावा, z_उजवा, n_डावा)==0;
  }अन्यथा{
    परत jsonLabelCompareEscaped(z_डावा, n_डावा, raw_डावा,
                                   z_उजवा, n_उजवा, raw_उजवा);
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
स्थिर u32 jsonLookupStep(JsonParse*,u32,स्थिरांक अक्षर*,u32);


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
स्थिर u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  स्थिरांक अक्षर *zTail   /* Tail of the path that determines substructure */
){
  स्थिर स्थिरांक u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  पूर्णांक rc;
  स_रखो(pIns, 0, आकार(*pIns));
  pIns->db = pParse->db;
  जर( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }अन्यथा{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    जर( pIns->iDepth >= JSON_MAX_DEPTH ){
      परत JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  परत rc;  /* Error code only */
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
स्थिर u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_मूळ,              /* Begin the search at this element of aBlob[] */
  स्थिरांक अक्षर *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_किल्ली, sz, n, iEnd, rc;
  स्थिरांक अक्षर *z_किल्ली;
  u8 x;

  जर( zPath[0]==0 ){
    जर( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, i_मूळ, &sz);
      sz += n;
      जर( pParse->eEdit==JEDIT_DEL ){
        जर( iLabel>0 ){
          sz += i_मूळ - iLabel;
          i_मूळ = iLabel;
        }
        jsonBlobEdit(pParse, i_मूळ, sz, 0, 0);
      }अन्यथा जर( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }अन्यथा जर( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        जर( zPath[-1]!=']' ){
          परत JSON_LOOKUP_NOTARRAY;
        }अन्यथा{
          jsonBlobEdit(pParse, i_मूळ, 0, pParse->aIns, pParse->nIns);
        }
      }अन्यथा{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_मूळ, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    परत i_मूळ;
  }
  जर( zPath[0]=='.' ){
    पूर्णांक raw_किल्ली = 1;
    x = pParse->aBlob[i_मूळ];
    zPath++;
    जर( zPath[0]=='"' ){
      z_किल्ली = zPath + 1;
      साठी(i=1; zPath[i] && zPath[i]!='"'; i++){
        जर( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_किल्ली = i-1;
      जर( zPath[i] ){
        i++;
      }अन्यथा{
        परत JSON_LOOKUP_PATHERROR;
      }
      testcase( n_किल्ली==0 );
      raw_किल्ली = स_प्रथम(z_किल्ली, '\\', n_किल्ली)==0;
    }अन्यथा{
      z_किल्ली = zPath;
      साठी(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_किल्ली = i;
      जर( n_किल्ली==0 ){
        परत JSON_LOOKUP_PATHERROR;
      }
    }
    जर( (x & 0x0f)!=JSONB_OBJECT ) परत JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_मूळ, &sz);
    j = i_मूळ + n;  /* j is the index of a label */
    iEnd = j+sz;
    पर्यंत( j<iEnd ){
      पूर्णांक rawLabel;
      स्थिरांक अक्षर *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      जर( x<JSONB_TEXT || x>JSONB_TEXTRAW ) परत JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      जर( n==0 ) परत JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      जर( k+sz>=iEnd ) परत JSON_LOOKUP_ERROR;
      zLabel = (स्थिरांक अक्षर*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      जर( jsonLabelCompare(z_किल्ली, n_किल्ली, raw_किल्ली, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        जर( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) परत JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        जर( n==0 || v+n+sz>iEnd ) परत JSON_LOOKUP_ERROR;
        प्रतिज्ञा( j>0 );
        जर( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          परत JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        जर( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_मूळ);
        परत rc;
      }
      j = k+sz;
      जर( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) परत JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      जर( n==0 ) परत JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    जर( j>iEnd ) परत JSON_LOOKUP_ERROR;
    जर( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      जर( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        परत JSON_LOOKUP_NOTARRAY;
      }
      स_रखो(&ix, 0, आकार(ix));
      ix.db = pParse->db;
      json_Blob_Append_गाठ(&ix, raw_किल्ली?JSONB_TEXTRAW:JSONB_TEXT5, n_किल्ली, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      जर( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_किल्ली+v.nBlob)
      ){
        प्रतिज्ञा( !pParse->oom );
        nIns = ix.nBlob + n_किल्ली + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        जर( !pParse->oom ){
          प्रतिज्ञा( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          प्रतिज्ञा( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          स_नकल(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          स_नकल(&pParse->aBlob[k], z_किल्ली, n_किल्ली);
          k += n_किल्ली;
          स_नकल(&pParse->aBlob[k], v.aBlob, v.nBlob);
          जर( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, i_मूळ);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      परत rc;
    }
  }अन्यथा जर( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_मूळ] & 0x0f;
    जर( x!=JSONB_ARRAY )  परत JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_मूळ, &sz);
    i = 1;
    पर्यंत( sqlite3Isdigit(zPath[i]) ){
      जर( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    जर( i<2 || zPath[i]!=']' ){
      जर( zPath[1]=='#' ){
        kk = jsonb_Array_मोजणी(pParse, i_मूळ);
        i = 2;
        जर( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          करा{
            जर( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }पर्यंत( sqlite3Isdigit(zPath[i]) );
          जर( nn>kk ) परत JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        जर( zPath[i]!=']' ){
          परत JSON_LOOKUP_PATHERROR;
        }
      }अन्यथा{
        परत JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_मूळ+n;
    iEnd = j+sz;
    पर्यंत( j<iEnd ){
      जर( kk==0 ){
        जर( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          परत JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        जर( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_मूळ);
        परत rc;
      }
      kk--;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      जर( n==0 ) परत JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    जर( j>iEnd ) परत JSON_LOOKUP_ERROR;
    जर( kk>0 ) परत JSON_LOOKUP_NOTFOUND;
    जर( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      जर( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        प्रतिज्ञा( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      जर( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_मूळ);
      परत rc;
    }
  }अन्यथा{
    परत JSON_LOOKUP_PATHERROR; 
  }
  परत JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
स्थिर शून्य jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  स्थिरांक u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  जर( NEVER(aBlob==0) ) परत;
  स_रखो(&x, 0, आकार(x));
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
स्थिर शून्य jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  पूर्णांक eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  पूर्णांक rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  प्रतिज्ञा( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  जर( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    परत;
  }
  निवड( pParse->aBlob[i] & 0x0f ){
    स्थिती JSONB_NULL: {
      जर( sz ) जा returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      थांबा;
    }
    स्थिती JSONB_TRUE: {
      जर( sz ) जा returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      थांबा;
    }
    स्थिती JSONB_FALSE: {
      जर( sz ) जा returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      थांबा;
    }
    स्थिती JSONB_INT5:
    स्थिती JSONB_INT: {
      sqlite3_int64 iRes = 0;
      अक्षर *z;
      पूर्णांक bNeg = 0;
      अक्षर x;
      जर( sz==0 ) जा returnfromblob_malformed;
      x = (अक्षर)pParse->aBlob[i+n];
      जर( x=='-' ){
        जर( sz<2 ) जा returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (स्थिरांक अक्षर*)&pParse->aBlob[i+n], (पूर्णांक)sz);
      जर( z==0 ) जा returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      जर( rc==0 ){
        जर( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          द्विगुण r;
          r = (द्विगुण)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }अन्यथा{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }अन्यथा जर( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }अन्यथा जर( rc==1 ){
        जा returnfromblob_malformed;
      }अन्यथा{
        जर( bNeg ){ n--; sz++; }
        जा to_double;
      }
      थांबा;
    }
    स्थिती JSONB_FLOAT5:
    स्थिती JSONB_FLOAT: {
      द्विगुण r;
      अक्षर *z;
      जर( sz==0 ) जा returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (स्थिरांक अक्षर*)&pParse->aBlob[i+n], (पूर्णांक)sz);
      जर( z==0 ) जा returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      जर( rc<=0 ) जा returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      थांबा;
    }
    स्थिती JSONB_TEXTRAW:
    स्थिती JSONB_TEXT: {
      sqlite3_result_text(pCtx, (अक्षर*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      थांबा;
    }
    स्थिती JSONB_TEXT5:
    स्थिती JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      स्थिरांक अक्षर *z;
      अक्षर *zOut;
      u32 nOut = sz;
      z = (स्थिरांक अक्षर*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      जर( zOut==0 ) जा returnfromblob_oom;
      साठी(iIn=iOut=0; iIn<sz; iIn++){
        अक्षर c = z[iIn];
        जर( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          जर( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }अन्यथा जर( v<=0x7f ){
            zOut[iOut++] = (अक्षर)v;
          }अन्यथा जर( v<=0x7ff ){
            प्रतिज्ञा( szEscape>=2 );
            zOut[iOut++] = (अक्षर)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }अन्यथा जर( v<0x10000 ){
            प्रतिज्ञा( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }अन्यथा जर( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }अन्यथा{
            प्रतिज्ञा( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }अन्यथा{
          zOut[iOut++] = c;
        }
      } /* end for() */
      प्रतिज्ञा( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      थांबा;
    }
    स्थिती JSONB_ARRAY:
    स्थिती JSONB_OBJECT: {
      जर( eMode==0 ){
        जर( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }अन्यथा{
          eMode = 1;
        }
      }
      जर( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }अन्यथा{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      थांबा;
    }
    पूर्वनिर्धारित: {
      जा returnfromblob_malformed;
    }
  }
  परत;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  परत;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  परत;
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
स्थिर पूर्णांक jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_मूल्य *pArg,
  JsonParse *pParse
){
  पूर्णांक eType = sqlite3_मूल्य_type(pArg);
  स्थिर u8 aNull[] = { 0x00 };
  स_रखो(pParse, 0, आकार(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  निवड( eType ){
    पूर्वनिर्धारित: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      परत 0;
    }
    स्थिती SQLITE_BLOB: {
      जर( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        परत 1;
      }
      थांबा;
    }
    स्थिती SQLITE_TEXT: {
      स्थिरांक अक्षर *zJson = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(pArg);
      पूर्णांक nJson = sqlite3_मूल्य_bytes(pArg);
      जर( zJson==0 ) परत 1;
      जर( sqlite3_मूल्य_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (अक्षर*)zJson;
        pParse->nJson = nJson;
        जर( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          स_रखो(pParse, 0, आकार(pParse[0]));
          परत 1;
        }
      }अन्यथा{
        json_Blob_Append_गाठ(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      थांबा;
    }
    स्थिती SQLITE_FLOAT: {
      जर( NEVER(sqlite3IsNaN(sqlite3_मूल्य_double(pArg))) ){
        json_Blob_Append_गाठ(pParse, JSONB_NULL, 0, 0);
      }अन्यथा{
        पूर्णांक n = sqlite3_मूल्य_bytes(pArg);
        स्थिरांक अक्षर *z = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(pArg);
        जर( z==0 ) परत 1;
        जर( z[0]=='I' ){
          json_Blob_Append_गाठ(pParse, JSONB_FLOAT, 5, "9e999");
        }अन्यथा जर( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_गाठ(pParse, JSONB_FLOAT, 6, "-9e999");
        }अन्यथा{
          json_Blob_Append_गाठ(pParse, JSONB_FLOAT, n, z);
        }
      }
      थांबा;
    }
    स्थिती SQLITE_INTEGER: {
      पूर्णांक n = sqlite3_मूल्य_bytes(pArg);
      स्थिरांक अक्षर *z = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(pArg);
      जर( z==0 ) परत 1;
      json_Blob_Append_गाठ(pParse, JSONB_INT, n, z);
      थांबा;
    }
  }
  जर( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    परत 1;
  }अन्यथा{
    परत 0;
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
स्थिर अक्षर *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  स्थिरांक अक्षर *zPath,        /* The path with the problem */
  पूर्णांक rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  अक्षर *z_संदेश;
  जर( rc==(पूर्णांक)JSON_LOOKUP_NOTARRAY ){
    z_संदेश = sqlite3_mprintf("not an array element: %Q", zPath);
  }अन्यथा जर( rc==(पूर्णांक)JSON_LOOKUP_ERROR ){
    z_संदेश = sqlite3_mprintf("malformed JSON");
  }अन्यथा जर( rc==(पूर्णांक)JSON_LOOKUP_TOODEEP ){
    z_संदेश = sqlite3_mprintf("JSON path too deep");
  }अन्यथा{
    z_संदेश = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  जर( ctx==0 ) परत z_संदेश;
  जर( z_संदेश ){
    sqlite3_result_error(ctx, z_संदेश, -1);
    sqlite3_free(z_संदेश);
  }अन्यथा{
    sqlite3_result_error_nomem(ctx);
  }
  परत 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
स्थिर शून्य json_घाला_Into_Blob(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv,
  पूर्णांक eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  पूर्णांक i;
  u32 rc = 0;
  स्थिरांक अक्षर *zPath = 0;
  पूर्णांक flgs;
  JsonParse *p;
  JsonParse ax;

  प्रतिज्ञा( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  जर( p==0 ) परत;
  साठी(i=1; i<argc-1; i+=2){
    जर( sqlite3_मूल्य_type(argv[i])==SQLITE_NULL ) सुरूठेवा;
    zPath = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
    जर( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      परत;
    }
    जर( zPath[0]!='$' ) जा json_घाला_Into_Blob_patherror;
    जर( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      परत;
    }
    जर( zPath[1]==0 ){
      जर( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }अन्यथा{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    जर( rc==JSON_LOOKUP_NOTFOUND ) सुरूठेवा;
    जर( JSON_LOOKUP_ISERROR(rc) ) जा json_घाला_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  परत;

json_घाला_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  परत;
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
स्थिर पूर्णांक jsonArgIsJsonb(sqlite3_मूल्य *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  जर( sqlite3_मूल्य_type(pArg)!=SQLITE_BLOB ) परत 0;
  p->aBlob = (u8*)sqlite3_मूल्य_blob(pArg);
  p->nBlob = (u32)sqlite3_मूल्य_bytes(pArg);
  जर( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    परत 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  परत 0;
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
स्थिर JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_मूल्य *pArg,
  u32 flgs
){
  पूर्णांक eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  प्रतिज्ञा( ctx!=0 );
  eType = sqlite3_मूल्य_type(pArg);
  जर( eType==SQLITE_NULL ){
    परत 0;
  }
  जर( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_शोध(ctx, pArg);
  }
  जर( pFromCache ){
    pFromCache->nJPRef++;
    जर( (flgs & JSON_EDITABLE)==0 ){
      परत pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, आकार(*p));
  जर( p==0 ) जा json_pfa_oom;
  स_रखो(p, 0, आकार(*p));
  p->db = db;
  p->nJPRef = 1;
  जर( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    जर( p->aBlob==0 ) जा json_pfa_oom;
    स_नकल(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    परत p;
  }
  जर( eType==SQLITE_BLOB ){
    जर( jsonArgIsJsonb(pArg,p) ){
      जर( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        जा json_pfa_oom;
      }
      परत p;
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
  p->zJson = (अक्षर*)sqlite3_मूल्य_text(pArg);
  p->nJson = sqlite3_मूल्य_bytes(pArg);
  जर( db->mallocFailed ) जा json_pfa_oom;
  जर( p->nJson==0 ) जा json_pfa_malformed;
  प्रतिज्ञा( p->zJson!=0 );
  जर( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    जर( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      परत p;
    }अन्यथा{
      jsonParseFree(p);
      परत 0;
    }
  }अन्यथा{
    पूर्णांक isRCStr = sqlite3_मूल्य_Is_Of_Class(pArg, sqlite3RCStrUnref);
    पूर्णांक rc;
    जर( !isRCStr ){
      अक्षर *zNew = sqlite3RCStrNew( p->nJson );
      जर( zNew==0 ) जा json_pfa_oom;
      स_नकल(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }अन्यथा{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_घाला(ctx, p);
    जर( rc==SQLITE_NOMEM ) जा json_pfa_oom;
    जर( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      जा rebuild_from_cache;
    }
  }
  परत p;

json_pfa_malformed:
  जर( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    परत p;
  }अन्यथा{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    परत 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  परत 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
स्थिर शून्य jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  पूर्णांक flgs;
  जर( p->oom ){
    sqlite3_result_error_nomem(ctx);
    परत;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  जर( flgs & JSON_BLOB ){
    जर( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }अन्यथा{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }अन्यथा{
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

#जर SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
स्थिर शून्य jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  पूर्णांक nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  पर्यंत( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    पूर्णांक showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    जर( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    जर( nn==0 ) nn = 1;
    जर( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    साठी(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    जर( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      सुरूठेवा;
    }
    pParse->nBlob = savedNBlob;
    जर( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      जर( iEnd>pParse->nBlob ){
        जर( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }अन्यथा{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    निवड( x ){
      स्थिती JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); थांबा;
      स्थिती JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); थांबा;
      स्थिती JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); थांबा;
      स्थिती JSONB_INT:      sqlite3_str_appendall(pOut,"int"); थांबा;
      स्थिती JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); थांबा;
      स्थिती JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); थांबा;
      स्थिती JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); थांबा;
      स्थिती JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); थांबा;
      स्थिती JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); थांबा;
      स्थिती JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); थांबा;
      स्थिती JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); थांबा;
      स्थिती JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        थांबा;
      }
      स्थिती JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        थांबा;
      }
      पूर्वनिर्धारित: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        थांबा;
      }
    }
    जर( showContent ){
      जर( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }अन्यथा{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        साठी(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          जर( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (अक्षर*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
स्थिर शून्य jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  अक्षर zBuf[1000];
  जर( pParse==0 ){
    म_लिखो("NULL pointer\n");
    परत;
  }अन्यथा{
    म_लिखो("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    म_लिखो("nBlob = %u\n", pParse->nBlob);
    म_लिखो("delta = %d\n", pParse->delta);
    जर( pParse->nBlob==0 ) परत;
    म_लिखो("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, आकार(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  म_लिखो("%s", sqlite3_str_मूल्य(&out));
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
स्थिर शून्य jsonParseFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  प्रतिज्ञा( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  जर( p==0 ) परत;
  जर( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }अन्यथा{
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
स्थिर शून्य jsonQuoteFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_मूल्य(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
स्थिर शून्य jsonArrayFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  साठी(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_मूल्य(&jx, argv[i]);
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
स्थिर शून्य jsonArrayLengthFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  जर( p==0 ) परत;
  जर( argc==2 ){
    स्थिरांक अक्षर *zPath = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[1]);
    जर( zPath==0 ){
      jsonParseFree(p);
      परत;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    जर( JSON_LOOKUP_ISERROR(i) ){
      जर( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }अन्यथा{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }अन्यथा{
    i = 0;
  }
  जर( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_मोजणी(p, i);
  }
  जर( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
स्थिर पूर्णांक jsonAllAlphanum(स्थिरांक अक्षर *z, पूर्णांक n){
  पूर्णांक i;
  साठी(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  परत i==n;
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
स्थिर शून्य jsonExtractFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p = 0;      /* The parse */
  पूर्णांक flags;             /* Flags associated with the function */
  पूर्णांक i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  जर( argc<2 ) परत;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  जर( p==0 ) परत;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  जर( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  साठी(i=1; i<argc; i++){
    /* With a single PATH argument */
    स्थिरांक अक्षर *zPath = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
    पूर्णांक nPath;
    u32 j;
    जर( zPath==0 ) जा json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    जर( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }अन्यथा जर( (flags & JSON_ABPATH) ){
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
      जर( sqlite3_मूल्य_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        जर( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }अन्यथा जर( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }अन्यथा जर( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }अन्यथा{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }अन्यथा{
      jsonBadPathError(ctx, zPath, 0);
      जा json_extract_error;
    }
    जर( j<p->nBlob ){
      जर( argc==2 ){
        जर( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          प्रतिज्ञा( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }अन्यथा{
          jsonReturnFromBlob(p, j, ctx, 0);
          जर( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }अन्यथा{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }अन्यथा जर( j==JSON_LOOKUP_NOTFOUND ){
      जर( argc==2 ){
        जा json_extract_error;  /* Return NULL if not found */
      }अन्यथा{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }अन्यथा{
      jsonBadPathError(ctx, zPath, j);
      जा json_extract_error;
    }
  }
  जर( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    जर( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  परत;
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
स्थिर पूर्णांक jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  स्थिरांक JsonParse *pPatch, /* The PATCH */
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

  प्रतिज्ञा( iTarget>=0 && iTarget<pTarget->nBlob );
  प्रतिज्ञा( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  जर( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    परत pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  जर( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  जर( NEVER(n==0) ) परत JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  जर( NEVER(n==0) ) परत JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  पर्यंत( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    जर( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      परत JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    जर( nPLabel==0 ) परत JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    जर( iPValue>=iPEnd ) परत JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    जर( nPValue==0 ) परत JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    जर( iPCursor>iPEnd ) परत JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    पर्यंत( iTCursor<iTEnd ){
      पूर्णांक isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      जर( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        परत JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      जर( nTLabel==0 ) परत JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      जर( iTValue>=iTEnd ) परत JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      जर( nTValue==0 ) परत JSON_MERGE_BADTARGET;
      जर( iTValue + nTValue + szTValue > iTEnd ) परत JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (स्थिरांक अक्षर*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (स्थिरांक अक्षर*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      जर( isEqual ) थांबा;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    जर( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      जर( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        जर( NEVER(pTarget->oom) ) परत JSON_MERGE_OOM;
      }अन्यथा{
        /* Algorithm line 12 */
        पूर्णांक rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        जर( iDepth>=JSON_MAX_DEPTH ) परत JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        जर( rc ) परत rc;
        pTarget->delta += savedDelta;
      }        
    }अन्यथा जर( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      जर( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        जर( pTarget->oom ) परत JSON_MERGE_OOM;
        स_नकल(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        स_नकल(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }अन्यथा{
        पूर्णांक rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        जर( pTarget->oom ) परत JSON_MERGE_OOM;
        स_नकल(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        जर( iDepth>=JSON_MAX_DEPTH ) परत JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        जर( rc ) परत rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  जर( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  परत pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
स्थिर शून्य jsonPatchFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  पूर्णांक rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  प्रतिज्ञा( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  जर( pTarget==0 ) परत;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  जर( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    जर( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }अन्यथा जर( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }अन्यथा जर( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }अन्यथा{
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
स्थिर शून्य jsonObjectFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;
  JsonString jx;
  स्थिरांक अक्षर *z;
  u32 n;

  जर( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    परत;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  साठी(i=0; i<argc; i+=2){
    जर( sqlite3_मूल्य_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      परत;
    }
    jsonAppendSeparator(&jx);
    z = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
    n = sqlite3_मूल्य_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_मूल्य(&jx, argv[i+1]);
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
स्थिर शून्य jsonRemoveFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;          /* The parse */
  स्थिरांक अक्षर *zPath = 0; /* Path of element to be removed */
  पूर्णांक i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  जर( argc<1 ) परत;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  जर( p==0 ) परत;
  साठी(i=1; i<argc; i++){
    zPath = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
    जर( zPath==0 ){
      जा json_remove_done;
    }
    जर( zPath[0]!='$' ){
      जा json_remove_patherror;
    }
    जर( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      जा json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    जर( JSON_LOOKUP_ISERROR(rc) ){
      जर( rc==JSON_LOOKUP_NOTFOUND ){
        सुरूठेवा;  /* No-op */
      }अन्यथा{
        jsonBadPathError(ctx, zPath, rc);
      }
      जा json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  परत;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  परत;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
स्थिर शून्य jsonReplaceFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  जर( argc<1 ) परत;
  जर( (argc&1)==0 ) {
    json_Wrong_अंक_Args(ctx, "replace");
    परत;
  }
  json_घाला_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
स्थिर शून्य jsonSetFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  पूर्णांक eInsType = JSON_घाला_TYPE(flags);
  स्थिर स्थिरांक अक्षर *azInsType[] = { "insert", "set", "array_insert" };
  स्थिर स्थिरांक u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  जर( argc<1 ) परत;
  प्रतिज्ञा( eInsType>=0 && eInsType<=2 );
  जर( (argc&1)==0 ) {
    json_Wrong_अंक_Args(ctx, azInsType[eInsType]);
    परत;
  }
  json_घाला_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
स्थिर शून्य jsonTypeFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;          /* The parse */
  स्थिरांक अक्षर *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  जर( p==0 ) परत;
  जर( argc==2 ){
    zPath = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[1]);
    जर( zPath==0 ) जा json_type_done;
    जर( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      जा json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    जर( JSON_LOOKUP_ISERROR(i) ){
      जर( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }अन्यथा{
        jsonBadPathError(ctx, zPath, i);
      }
      जा json_type_done;
    }
  }अन्यथा{
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
स्थिर शून्य jsonPrettyFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  स_रखो(&x, 0, आकार(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  जर( x.pParse==0 ) परत;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  जर( argc==1 || (x.zIndent = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }अन्यथा{
    x.szIndent = (u32)म_माप(x.zIndent);
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
स्थिर शून्य jsonValidFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  जर( argc==2 ){
    i64 f = sqlite3_मूल्य_int64(argv[1]);
    जर( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      परत;
    }
    flags = f & 0x0f;
  }
  निवड( sqlite3_मूल्य_type(argv[0]) ){
    स्थिती SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      परत;
    }
    स्थिती SQLITE_BLOB: {
      JsonParse py;
      स_रखो(&py, 0, आकार(py));
      जर( jsonArgIsJsonb(argv[0], &py) ){
        जर( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }अन्यथा जर( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        थांबा;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    पूर्वनिर्धारित: {
      JsonParse px;
      जर( (flags & 0x3)==0 ) थांबा;
      स_रखो(&px, 0, आकार(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      जर( p ){
        जर( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }अन्यथा जर( p->nErr ){
          /* no-op */
        }अन्यथा जर( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }अन्यथा{
        sqlite3_result_error_nomem(ctx);
      }
      थांबा;
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
स्थिर शून्य jsonErrorFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  प्रतिज्ञा( argc==1 );
  UNUSED_PARAMETER(argc);
  स_रखो(&s, 0, आकार(s));
  s.db = sqlite3_context_db_handle(ctx);
  जर( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }अन्यथा{
    s.zJson = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
    जर( s.zJson==0 ) परत;  /* NULL input or OOM */
    s.nJson = sqlite3_मूल्य_bytes(argv[0]);
    जर( jsonConvertTextToBlob(&s,0) ){
      जर( s.oom ){
        iErrPos = -1;
      }अन्यथा{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        प्रतिज्ञा( s.zJson!=0 );  /* Because s.oom is false */
        साठी(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          जर( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  जर( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }अन्यथा{
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
स्थिर शून्य jsonArrayStep(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, आकार(*pStr));
  जर( pStr ){
    जर( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }अन्यथा जर( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_मूल्य(pStr, argv[0]);
  }
}
स्थिर शून्य jsonArrayCompute(sqlite3_context *ctx, पूर्णांक isFinal){
  JsonString *pStr;
  पूर्णांक flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  जर( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    जर( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      परत;
    }अन्यथा जर( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      जर( isFinal ){
        जर( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }अन्यथा{
        jsonStringTrimOneChar(pStr);
      }
      परत;
    }अन्यथा जर( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (पूर्णांक)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }अन्यथा{
      sqlite3_result_text(ctx, pStr->zBuf, (पूर्णांक)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }अन्यथा जर( flags & JSON_BLOB ){
    स्थिर स्थिरांक u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }अन्यथा{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
स्थिर शून्य json_Array_मूल्य(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
स्थिर शून्य jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
स्थिर शून्य jsonGroupInverse(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  अचिन्हित पूर्णांक i;
  पूर्णांक inStr = 0;
  पूर्णांक nNest = 0;
  अक्षर *z;
  अक्षर c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  जर( NEVER(!pStr) ) परत;
  z = pStr->zBuf;
  साठी(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    जर( c=='"' ){
      inStr = !inStr;
    }अन्यथा जर( c=='\\' ){
      i++;
    }अन्यथा जर( !inStr ){
      जर( c=='{' || c=='[' ) nNest++;
      जर( c=='}' || c==']' ) nNest--;
    }
  }
  जर( i<pStr->nUsed ){
    pStr->nUsed -= i;
    स_हटाओ(&z[1], &z[i+1], (माप_प्रकार)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }अन्यथा{
    pStr->nUsed = 1;
  }
}
#अन्यथा
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
स्थिर शून्य jsonObjectStep(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonString *pStr;
  स्थिरांक अक्षर *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, आकार(*pStr));
  जर( pStr ){
    z = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[0]);
    n = sqlite3Strlen30(z);
    जर( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }अन्यथा जर( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    जर( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_मूल्य(pStr, argv[1]);
    }अन्यथा{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
स्थिर शून्य jsonObjectCompute(sqlite3_context *ctx, पूर्णांक isFinal){
  JsonString *pStr;
  पूर्णांक flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  जर( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    जर( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      परत;
    }
    जर( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      पूर्णांक inStr = 0;
      जर( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        जर( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          परत;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      साठी(i=j=1; i<pStr->nUsed; i++){
        अक्षर c = pStr->zBuf[i];
        जर( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }अन्यथा जर( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }अन्यथा जर( c=='@' && !inStr ){
          प्रतिज्ञा( i+1<pStr->nUsed );
          जर( pStr->zBuf[i+1]==',' ){
            i++;
          }अन्यथा जर( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }अन्यथा{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    जर( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      जर( isFinal ){
        जर( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }अन्यथा{
        jsonStringTrimOneChar(pOgStr);
      }
    }अन्यथा जर( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (पूर्णांक)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }अन्यथा{
      sqlite3_result_text(ctx, pStr->zBuf, (पूर्णांक)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    जर( pStr!=pOgStr ) jsonStringReset(pStr);
  }अन्यथा जर( flags & JSON_BLOB ){
    स्थिर स्थिरांक अचिन्हित अक्षर emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }अन्यथा{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
स्थिर शून्य json_Object_मूल्य(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
स्थिर शून्य jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
प्रकारव्याख्या संरचना JsonParent JsonParent;
संरचना JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 i_मूल्य;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_किल्ली;                  /* Key for JSONB_ARRAY */
};

प्रकारव्याख्या संरचना JsonEachCursor JsonEachCursor;
संरचना JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_मूळ;                 /* Size of the root path in bytes */
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
प्रकारव्याख्या संरचना JsonEachConnection JsonEachConnection;
संरचना JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
स्थिर पूर्णांक jsonEachConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  JsonEachConnection *pNew;
  पूर्णांक rc;

/* Column numbers */
#define JEACH_किल्ली     0
#define JEACH_मूल्य   1
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
#define JEACH_मूळ    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  जर( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, आकार(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    जर( pNew==0 ) परत SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  परत rc;
}

/* destructor for json_each virtual table */
स्थिर पूर्णांक jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  परत SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
स्थिर पूर्णांक jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, आकार(*pCur));
  जर( pCur==0 ) परत SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  परत SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
स्थिर शून्य jsonEachCursorReset(JsonEachCursor *p){
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
स्थिर पूर्णांक jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  परत SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
स्थिर पूर्णांक jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  परत p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
स्थिर पूर्णांक jsonSkipLabel(JsonEachCursor *p){
  जर( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    जर( sz >= p->sParse.nBlob ) sz = p->i;
    परत sz;
  }अन्यथा{
    परत p->i;
  }
}

/*
** Append the path name for the current element.
*/
स्थिर शून्य jsonAppendPathName(JsonEachCursor *p){
  प्रतिज्ञा( p->nParent>0 );
  प्रतिज्ञा( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  जर( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_किल्ली);
  }अन्यथा{
    u32 n, sz = 0, k, i;
    स्थिरांक अक्षर *z;
    पूर्णांक needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (स्थिरांक अक्षर*)&p->sParse.aBlob[k];
    जर( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }अन्यथा{
      साठी(i=0; i<sz; i++){
        जर( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          थांबा;
        }
      }
    }
    जर( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }अन्यथा{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
स्थिर पूर्णांक json_Each_Malformed_आदान(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_संदेश);
  cur->pVtab->z_Err_संदेश = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  परत cur->pVtab->z_Err_संदेश ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
स्थिर पूर्णांक jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  पूर्णांक rc = SQLITE_OK;
  जर( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    जर( n==0 )परत json_Each_Malformed_आदान(cur);
    जर( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      जर( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, आकार(JsonParent)*nNew);
        जर( pNew==0 ) परत SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->i_मूल्य = i;
      pParent->iEnd = i + n + sz;
      pParent->i_किल्ली = -1;
      pParent->nPath = (u32)p->path.nUsed;
      जर( p->eType && p->nParent ){
        jsonAppendPathName(p);
        जर( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }अन्यथा{
      p->i = i + n + sz;
    }
    पर्यंत( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    जर( levelChange ){
      जर( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->i_मूल्य;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }अन्यथा{
        p->eType = 0;
      }
    }
  }अन्यथा{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    जर( n==0 )परत json_Each_Malformed_आदान(cur);
    p->i = i + n + sz;
  }
  जर( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_किल्ली++;
  }
  p->iRowid++;
  परत rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
स्थिर पूर्णांक jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  अक्षर *z = p->path.zBuf;
  जर( p->iRowid==0 && p->bRecursive && n>=2 ){
    पर्यंत( ALWAYS(n>1) ){
      n--;
      जर( z[n]=='[' || z[n]=='.' ){
        u32 x;
        अक्षर cSaved = z[n];
        z[n] = 0;
        प्रतिज्ञा( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        जर( !JSON_LOOKUP_ISERROR(x) ) थांबा;
      }
    }
  }
  परत n;
}

/* Return the value of a column */
स्थिर पूर्णांक jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  पूर्णांक iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  निवड( iColumn ){
    स्थिती JEACH_किल्ली: {
      जर( p->nParent==0 ){
        u32 n, j;
        जर( p->n_मूळ==1 ) थांबा;
        j = jsonEachPathLength(p);
        n = p->n_मूळ - j;
        जर( n==0 ){
          थांबा;
        }अन्यथा जर( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }अन्यथा जर( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }अन्यथा{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        थांबा;
      }
      जर( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }अन्यथा{
        प्रतिज्ञा( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].i_किल्ली);
      }
      थांबा;
    }
    स्थिती JEACH_मूल्य: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      जर( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      थांबा;
    }
    स्थिती JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      थांबा;
    }
    स्थिती JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      जर( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      थांबा;
    }
    स्थिती JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      थांबा;
    }
    स्थिती JEACH_PARENT: {
      जर( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      थांबा;
    }
    स्थिती JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      जर( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      थांबा;
    }
    स्थिती JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      थांबा;
    }
    पूर्वनिर्धारित: {
      sqlite3_result_text(ctx, p->path.zBuf, p->n_मूळ, SQLITE_TRANSIENT);
      थांबा;
    }
    स्थिती JEACH_JSON: {
      जर( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }अन्यथा{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      थांबा;
    }
  }
  परत SQLITE_OK;
}

/* Return the current rowid value */
स्थिर पूर्णांक jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  परत SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
स्थिर पूर्णांक jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  पूर्णांक i;                     /* Loop counter or computed array index */
  पूर्णांक aIdx[2];               /* Index of constraints for JSON and ROOT */
  पूर्णांक unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  पूर्णांक idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  स्थिरांक संरचना sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  प्रतिज्ञा( JEACH_मूळ == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  साठी(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    पूर्णांक iCol;
    पूर्णांक iMask;
    जर( pConstraint->iColumn < JEACH_JSON ) सुरूठेवा;
    iCol = pConstraint->iColumn - JEACH_JSON;
    प्रतिज्ञा( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    जर( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }अन्यथा जर( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  जर( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  जर( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    परत SQLITE_CONSTRAINT;
  }
  जर( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_अंक = 0;
  }अन्यथा{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    जर( aIdx[1]<0 ){
      pIdxInfo->idx_अंक = 1;  /* Only JSON supplied.  Plan 1 */
    }अन्यथा{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_अंक = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  परत SQLITE_OK;
}

/* Start a search on a new JSON string */
स्थिर पूर्णांक jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  पूर्णांक idx_अंक, स्थिरांक अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  स्थिरांक अक्षर *z_मूळ = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  जर( idx_अंक==0 ) परत SQLITE_OK;
  स_रखो(&p->sParse, 0, आकार(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  जर( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }अन्यथा{
    p->sParse.zJson = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
    p->sParse.nJson = sqlite3_मूल्य_bytes(argv[0]);
    जर( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      परत SQLITE_OK;
    }      
    जर( jsonConvertTextToBlob(&p->sParse, 0) ){
      जर( p->sParse.oom ){
        परत SQLITE_NOMEM;
      }
      परत json_Each_Malformed_आदान(cur);
    }
  }
  जर( idx_अंक==3 ){
    z_मूळ = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[1]);
    जर( z_मूळ==0 ) परत SQLITE_OK;
    जर( z_मूळ[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_संदेश);
      cur->pVtab->z_Err_संदेश = jsonBadPathError(0, z_मूळ, 0);
      jsonEachCursorReset(p);
      परत cur->pVtab->z_Err_संदेश ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_मूळ = sqlite3Strlen30(z_मूळ);
    जर( z_मूळ[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }अन्यथा{
      i = jsonLookupStep(&p->sParse, 0, z_मूळ+1, 0);
      जर( JSON_LOOKUP_ISERROR(i) ){
        जर( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          परत SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_संदेश);
        cur->pVtab->z_Err_संदेश = jsonBadPathError(0, z_मूळ, 0);
        jsonEachCursorReset(p);
        परत cur->pVtab->z_Err_संदेश ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      जर( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }अन्यथा{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_मूळ, p->n_मूळ);
  }अन्यथा{
    i = p->i = 0;
    p->eType = 0;
    p->n_मूळ = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  जर( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, आकार(JsonParent));
    जर( p->aParent==0 ) परत SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_किल्ली = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].i_मूल्य = i;
  }
  परत SQLITE_OK;
}

/* The methods of the json_each virtual table */
स्थिर sqlite3_module jsonEachModule = {
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
शून्य sqlite3RegisterJsonFunctions(शून्य){
#ifndef SQLITE_OMIT_JSON
  स्थिर FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_घाला, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_घाला,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_घाला,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_घाला,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#जर SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_मूल्य, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_मूल्य, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_मूल्य, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_मूल्य, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_घाला_Builtin_Funcs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#जर  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, स्थिरांक अक्षर *zName){
  अचिन्हित पूर्णांक i;
  स्थिर स्थिरांक अक्षर *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  प्रतिज्ञा( sqlite3HashFind(&db->aModule, zName)==0 );
  साठी(i=0; i<आकार(azModule)/आकार(azModule[0]); i++){
    जर( sqlite3StrICmp(azModule[i],zName)==0 ){
      परत sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  परत 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
