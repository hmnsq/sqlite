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
#અન્તર્ભાવ "sqliteInt.h"

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
સ્થિર અચલ અક્ષર * અચલ jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
સ્થિર અચલ અક્ષર jsonIsSpace[] = {
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
#define jsonIsspace(x) (jsonIsSpace[(unsigned અક્ષર)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ifdef SQLITE_ASCII
સ્થિર અચલ અક્ષર jsonSpaces[] = "\011\012\015\040";
#endif
#ifdef SQLITE_EBCDIC
સ્થિર અચલ અક્ષર jsonSpaces[] = "\005\045\015\100";
#endif


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
સ્થિર અચલ અક્ષર jsonIsOk[256] = {
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
પ્રકારવ્યાખ્યા સંરચના JsonCache JsonCache;
પ્રકારવ્યાખ્યા સંરચના JsonString JsonString;
પ્રકારવ્યાખ્યા સંરચના JsonParse JsonParse;

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
સંરચના JsonCache {
  sqlite3 *db;                    /* Database connection */
  પૂર્ણાંક nUsed;                      /* Number of active entries in the cache */
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
સંરચના JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  અક્ષર *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  અક્ષર zSpace[100];        /* Initial static space */
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

#define JSON_ઉમેરો_TYPE(X)  (((X)&0xC)>>2)


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
સંરચના JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  અક્ષર *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  પૂર્ણાંક nJson;         /* Length of the zJson string in bytes */
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
  પૂર્ણાંક delta;         /* Size change due to the edit */
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
#નહીંતર
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
સ્થિર શૂન્ય jsonReturnStringAsBlob(JsonString*);
સ્થિર પૂર્ણાંક jsonArgIsJsonb(sqlite3_કિંમત *pJson, JsonParse *p);
સ્થિર u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
સ્થિર શૂન્ય jsonReturnParse(sqlite3_context*,JsonParse*);
સ્થિર JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_કિંમત*,u32);
સ્થિર શૂન્ય jsonParseFree(JsonParse*);
સ્થિર u32 jsonbPayloadSize(અચલ JsonParse*, u32, u32*);
સ્થિર u32 jsonUnescapeOneChar(અચલ અક્ષર*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
સ્થિર શૂન્ય jsonCacheDelete(JsonCache *p){
  પૂર્ણાંક i;
  માટે(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
સ્થિર શૂન્ય jsonCacheDeleteGeneric(શૂન્ય *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
સ્થિર પૂર્ણાંક json_Cache_ઉમેરો(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  પ્રતિજ્ઞા( pParse->zJson!=0 );
  પ્રતિજ્ઞા( pParse->bJsonIsRCStr );
  પ્રતિજ્ઞા( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  જો( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, કદ(*p));
    જો( p==0 ) પાછા SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    જો( p==0 ) પાછા SQLITE_NOMEM;
  }
  જો( p->nUsed >= JSON_CACHE_SIZE ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_SIZE-1)*કદ(p->a[0]));
    p->nUsed = JSON_CACHE_SIZE-1;
  }
  પ્રતિજ્ઞા( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  પાછા SQLITE_OK;
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
સ્થિર JsonParse *json_Cache_શોધ(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_કિંમત *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  પૂર્ણાંક i;
  અચલ અક્ષર *zJson;
  પૂર્ણાંક nJson;

  જો( sqlite3_કિંમત_type(pArg)!=SQLITE_TEXT ){
    પાછા 0;
  }
  zJson = (અચલ અક્ષર*)sqlite3_કિંમત_text(pArg);
  જો( zJson==0 ) પાછા 0;
  nJson = sqlite3_કિંમત_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  જો( p==0 ){
    પાછા 0;
  }
  માટે(i=0; i<p->nUsed; i++){
    જો( p->a[i]->zJson==zJson ) તોડો;
  }
  જો( i>=p->nUsed ){
    માટે(i=0; i<p->nUsed; i++){
      જો( p->a[i]->nJson!=nJson ) ચાલુ;
      જો( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) તોડો;
    }
  }
  જો( i<p->nUsed ){
    જો( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*કદ(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    પ્રતિજ્ઞા( p->a[i]->delta==0 );
    પાછા p->a[i];
  }નહીંતર{
    પાછા 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
સ્થિર શૂન્ય jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = કદ(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
સ્થિર શૂન્ય jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
સ્થિર શૂન્ય jsonStringReset(JsonString *p){
  જો( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
સ્થિર શૂન્ય jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  જો( p->pCtx ) sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
સ્થિર શૂન્ય jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  પ્રતિજ્ઞા( p->pCtx!=0 );
  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
સ્થિર પૂર્ણાંક jsonStringGrow(JsonString *p, u32 N){
  u64 nTotal = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  અક્ષર *zNew;
  જો( p->bStatic ){
    જો( p->eErr ) પાછા 1;
    zNew = sqlite3RCStrNew(nTotal);
    જો( zNew==0 ){
      jsonStringOom(p);
      પાછા SQLITE_NOMEM;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }નહીંતર{
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    જો( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      પાછા SQLITE_NOMEM;
    }
  }
  p->nAlloc = nTotal;
  પાછા SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય jsonStringExpandAndAppend(
  JsonString *p,
  અચલ અક્ષર *zIn,
  u32 N
){
  પ્રતિજ્ઞા( N>0 );
  જો( jsonStringGrow(p,N) ) પાછા;
  memcpy(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
સ્થિર શૂન્ય jsonAppendRaw(JsonString *p, અચલ અક્ષર *zIn, u32 N){
  જો( N==0 ) પાછા;
  જો( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }નહીંતર{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
સ્થિર શૂન્ય jsonAppendRawNZ(JsonString *p, અચલ અક્ષર *zIn, u32 N){
  પ્રતિજ્ઞા( N>0 );
  જો( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }નહીંતર{
    memcpy(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
સ્થિર શૂન્ય jsonPrintf(પૂર્ણાંક N, JsonString *p, અચલ અક્ષર *zFormat, ...){
  va_list ap;
  જો( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) પાછા;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  va_end(ap);
  p->nUsed += (પૂર્ણાંક)strlen(p->zBuf+p->nUsed);
}

/* Append a single character
*/
સ્થિર SQLITE_NOINLINE શૂન્ય jsonAppendCharExpand(JsonString *p, અક્ષર c){
  જો( jsonStringGrow(p,1) ) પાછા;
  p->zBuf[p->nUsed++] = c;
}
સ્થિર શૂન્ય jsonAppendChar(JsonString *p, અક્ષર c){
  જો( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }નહીંતર{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
સ્થિર શૂન્ય jsonStringTrimOneChar(JsonString *p){
  જો( p->eErr==0 ){
    પ્રતિજ્ઞા( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
સ્થિર પૂર્ણાંક jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  પાછા p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
સ્થિર શૂન્ય jsonAppendSeparator(JsonString *p){
  અક્ષર c;
  જો( p->nUsed==0 ) પાછા;
  c = p->zBuf[p->nUsed-1];
  જો( c=='[' || c=='{' ) પાછા;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
સ્થિર શૂન્ય jsonAppendControlChar(JsonString *p, u8 c){
  સ્થિર અચલ અક્ષર aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  પ્રતિજ્ઞા( કદ(aSpecial)==32 );
  પ્રતિજ્ઞા( aSpecial['\b']=='b' );
  પ્રતિજ્ઞા( aSpecial['\f']=='f' );
  પ્રતિજ્ઞા( aSpecial['\n']=='n' );
  પ્રતિજ્ઞા( aSpecial['\r']=='r' );
  પ્રતિજ્ઞા( aSpecial['\t']=='t' );
  પ્રતિજ્ઞા( c>=0 && c<કદ(aSpecial) );
  પ્રતિજ્ઞા( p->nUsed+7 <= p->nAlloc );
  જો( aSpecial[c] ){
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed+1] = aSpecial[c];
    p->nUsed += 2;
  }નહીંતર{
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
સ્થિર શૂન્ય jsonAppendString(JsonString *p, અચલ અક્ષર *zIn, u32 N){
  u32 k;
  u8 c;
  અચલ u8 *z = (અચલ u8*)zIn;
  જો( z==0 ) પાછા;
  જો( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) પાછા;
  p->zBuf[p->nUsed++] = '"';
  જ્યારે( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    જ્યારે( 1 /* Exit by break */ ){
      જો( k+3>=N ){
        જ્યારે( k<N && jsonIsOk[z[k]] ){ k++; }
        તોડો;
      }
      જો( !jsonIsOk[z[k]] ){
        તોડો;
      }
      જો( !jsonIsOk[z[k+1]] ){
        k += 1;
        તોડો;
      }
      જો( !jsonIsOk[z[k+2]] ){
        k += 2;
        તોડો;
      }
      જો( !jsonIsOk[z[k+3]] ){
        k += 3;
        તોડો;
      }નહીંતર{
        k += 4;
      }
    }
    જો( k>=N ){
      જો( k>0 ){
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      તોડો;
    }
    જો( k>0 ){
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    જો( c=='"' || c=='\\' ){
      જો( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) પાછા;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }નહીંતર જો( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }નહીંતર{
      જો( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) પાછા;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  પ્રતિજ્ઞા( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
સ્થિર શૂન્ય json_Append_Sql_કિંમત(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_કિંમત *p_કિંમત          /* Value to append */
){
  પસંદગી( sqlite3_કિંમત_type(p_કિંમત) ){
    સ્થિતિ SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      તોડો;
    }
    સ્થિતિ SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_કિંમત_double(p_કિંમત));
      તોડો;
    }
    સ્થિતિ SQLITE_INTEGER: {
      અચલ અક્ષર *z = (અચલ અક્ષર*)sqlite3_કિંમત_text(p_કિંમત);
      u32 n = (u32)sqlite3_કિંમત_bytes(p_કિંમત);
      jsonAppendRaw(p, z, n);
      તોડો;
    }
    સ્થિતિ SQLITE_TEXT: {
      અચલ અક્ષર *z = (અચલ અક્ષર*)sqlite3_કિંમત_text(p_કિંમત);
      u32 n = (u32)sqlite3_કિંમત_bytes(p_કિંમત);
      જો( sqlite3_કિંમત_subtype(p_કિંમત)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }નહીંતર{
        jsonAppendString(p, z, n);
      }
      તોડો;
    }
    મૂળભૂત: {
      JsonParse px;
      memset(&px, 0, કદ(px));
      જો( jsonArgIsJsonb(p_કિંમત, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }નહીંતર જો( p->eErr==0 ){
        sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      તોડો;
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
સ્થિર શૂન્ય jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  પ્રતિજ્ઞા( (pParse!=0)==(ctx!=0) );
  પ્રતિજ્ઞા( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  જો( p->eErr==0 ){
    પૂર્ણાંક flags = SQLITE_PTR_TO_INT(sqlite3_user_data(p->pCtx));
    જો( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }નહીંતર જો( p->bStatic ){
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }નહીંતર{
      જો( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        પૂર્ણાંક rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_ઉમેરો(ctx, pParse);
        જો( rc==SQLITE_NOMEM ){
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          પાછા;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }નહીંતર જો( p->eErr & JSTRING_OOM ){
    sqlite3_result_error_nomem(p->pCtx);
  }નહીંતર જો( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }નહીંતર જો( p->eErr & JSTRING_MALFORMED ){
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
સ્થિર શૂન્ય jsonParseReset(JsonParse *pParse){
  પ્રતિજ્ઞા( pParse->nJPRef<=1 );
  જો( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  જો( pParse->nBlobAlloc ){
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
સ્થિર શૂન્ય jsonParseFree(JsonParse *pParse){
  જો( pParse ){
    જો( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }નહીંતર{
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
સ્થિર u8 jsonHexToInt(પૂર્ણાંક h){
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  પાછા (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
સ્થિર u32 jsonHexToInt4(અચલ અક્ષર *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  પાછા v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
સ્થિર પૂર્ણાંક jsonIs2Hex(અચલ અક્ષર *z){
  પાછા sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
સ્થિર પૂર્ણાંક jsonIs4Hex(અચલ અક્ષર *z){
  પાછા jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
સ્થિર પૂર્ણાંક json5Whitespace(અચલ અક્ષર *zIn){
  પૂર્ણાંક n = 0;
  અચલ u8 *z = (u8*)zIn;
  જ્યારે( 1 /*exit by "goto whitespace_done"*/ ){
    પસંદગી( z[n] ){
      સ્થિતિ 0x09:
      સ્થિતિ 0x0a:
      સ્થિતિ 0x0b:
      સ્થિતિ 0x0c:
      સ્થિતિ 0x0d:
      સ્થિતિ 0x20: {
        n++;
        તોડો;
      }
      સ્થિતિ '/': {
        જો( z[n+1]=='*' && z[n+2]!=0 ){
          પૂર્ણાંક j;
          માટે(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            જો( z[j]==0 ) જાઓ whitespace_done;
          }
          n = j+1;
          તોડો;
        }નહીંતર જો( z[n+1]=='/' ){
          પૂર્ણાંક j;
          અક્ષર c;
          માટે(j=n+2; (c = z[j])!=0; j++){
            જો( c=='\n' || c=='\r' ) તોડો;
            જો( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              તોડો;
            }
          }
          n = j;
          જો( z[n] ) n++;
          તોડો;
        }
        જાઓ whitespace_done;
      }
      સ્થિતિ 0xc2: {
        જો( z[n+1]==0xa0 ){
          n += 2;
          તોડો;
        }
        જાઓ whitespace_done;
      }
      સ્થિતિ 0xe1: {
        જો( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          તોડો;
        }
        જાઓ whitespace_done;
      }
      સ્થિતિ 0xe2: {
        જો( z[n+1]==0x80 ){
          u8 c = z[n+2];
          જો( c<0x80 ) જાઓ whitespace_done;
          જો( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            તોડો;
          }
        }નહીંતર જો( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          તોડો;
        }
        જાઓ whitespace_done;
      }
      સ્થિતિ 0xe3: {
        જો( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          તોડો;
        }
        જાઓ whitespace_done;
      }
      સ્થિતિ 0xef: {
        જો( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          તોડો;
        }
        જાઓ whitespace_done;
      }
      મૂળભૂત: {
        જાઓ whitespace_done;
      }
    }
  }
  whitespace_done:
  પાછા n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
સ્થિર અચલ સંરચના NanInfName {
  અક્ષર c1;
  અક્ષર c2;
  અક્ષર n;
  અક્ષર eType;
  અક્ષર nRepl;
  અક્ષર *zMatch;
  અક્ષર *zRepl;
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
સ્થિર શૂન્ય json_Wrong_અંક_Args(
  sqlite3_context *pCtx,
  અચલ અક્ષર *zFuncName
){
  અક્ષર *z_સંદેશ = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_result_error(pCtx, z_સંદેશ, -1);
  sqlite3_free(z_સંદેશ);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
સ્થિર પૂર્ણાંક jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  પ્રતિજ્ઞા( N>pParse->nBlobAlloc );
  જો( pParse->nBlobAlloc==0 ){
    t = 100;
  }નહીંતર{
    t = pParse->nBlobAlloc*2;
  }
  જો( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  જો( aNew==0 ){ pParse->oom = 1; પાછા 1; }
  પ્રતિજ્ઞા( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  પાછા 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
સ્થિર પૂર્ણાંક jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 nSize;
  પ્રતિજ્ઞા( !pParse->bReadOnly );
  જો( pParse->oom ) પાછા 0;
  જો( pParse->nBlobAlloc>0 ) પાછા 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  જો( jsonBlobExpand(pParse, nSize) ){
    પાછા 0;
  }
  પ્રતિજ્ઞા( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  પાછા 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  જો( pParse->oom==0 ){
    પ્રતિજ્ઞા( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
સ્થિર શૂન્ય jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  જો( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }નહીંતર{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
સ્થિર શૂન્ય json_Blob_Append_ગાંઠ(JsonParse*,u8,u64,અચલ શૂન્ય*);
સ્થિર SQLITE_NOINLINE શૂન્ય json_Blob_Expand_And_Append_ગાંઠ(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  અચલ શૂન્ય *aPayload
){
  જો( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) પાછા;
  json_Blob_Append_ગાંઠ(pParse, eType, szPayload, aPayload);
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
સ્થિર શૂન્ય json_Blob_Append_ગાંઠ(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  અચલ શૂન્ય *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  જો( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_ગાંઠ(pParse,eType,szPayload,aPayload);
    પાછા;
  }
  પ્રતિજ્ઞા( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  જો( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }નહીંતર જો( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }નહીંતર જો( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }નહીંતર{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  જો( aPayload ){
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
સ્થિર પૂર્ણાંક jsonBlobChangePayloadSize(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  પૂર્ણાંક delta;
  જો( pParse->oom ) પાછા 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  જો( szType<=11 ){
    nExtra = 0;
  }નહીંતર જો( szType==12 ){
    nExtra = 1;
  }નહીંતર જો( szType==13 ){
    nExtra = 2;
  }નહીંતર જો( szType==14 ){
    nExtra = 4;
  }નહીંતર{
    nExtra = 8;
  }
  જો( szPayload<=11 ){
    nNeeded = 0;
  }નહીંતર જો( szPayload<=0xff ){
    nNeeded = 1;
  }નહીંતર જો( szPayload<=0xffff ){
    nNeeded = 2;
  }નહીંતર{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  જો( delta ){
    u64 newSize = (u64)pParse->nBlob + delta;
    જો( delta>0 ){
      જો( newSize>pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize) ){
        પાછા 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }નહીંતર{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = newSize;
  }
  જો( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }નહીંતર જો( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }નહીંતર જો( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }નહીંતર{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  પાછા delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
સ્થિર પૂર્ણાંક jsonIs4HexB(અચલ અક્ષર *z, પૂર્ણાંક *pOp){
  જો( z[0]!='u' ) પાછા 0;
  જો( !jsonIs4Hex(&z[1]) ) પાછા 0;
  *pOp = JSONB_TEXTJ;
  પાછા 1;
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
સ્થિર u32 jsonbValidityCheck(
  અચલ JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  અચલ u8 *z;
  u8 x;
  જો( iDepth>JSON_MAX_DEPTH ) પાછા i+1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  જો( NEVER(n==0) ) પાછા i+1;          /* Checked by caller */
  જો( NEVER(i+n+sz!=iEnd) ) પાછા i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  પસંદગી( x ){
    સ્થિતિ JSONB_NULL:
    સ્થિતિ JSONB_TRUE:
    સ્થિતિ JSONB_FALSE: {
      પાછા n+sz==1 ? 0 : i+1;
    }
    સ્થિતિ JSONB_INT: {
      જો( sz<1 ) પાછા i+1;
      j = i+n;
      જો( z[j]=='-' ){
        j++;
        જો( sz<2 ) પાછા i+1;
      }
      k = i+n+sz;
      જ્યારે( j<k ){
        જો( sqlite3Isdigit(z[j]) ){
          j++;
        }નહીંતર{
          પાછા j+1;
        }
      }
      પાછા 0;
    }
    સ્થિતિ JSONB_INT5: {
      જો( sz<3 ) પાછા i+1;
      j = i+n;
      જો( z[j]=='-' ){
        જો( sz<4 ) પાછા i+1;
        j++;
      }
      જો( z[j]!='0' ) પાછા i+1;
      જો( z[j+1]!='x' && z[j+1]!='X' ) પાછા j+2;
      j += 2;
      k = i+n+sz;
      જ્યારે( j<k ){
        જો( sqlite3Isxdigit(z[j]) ){
          j++;
        }નહીંતર{
          પાછા j+1;
        }
      }
      પાછા 0;
    }
    સ્થિતિ JSONB_FLOAT:
    સ્થિતિ JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      જો( sz<2 ) પાછા i+1;
      j = i+n;
      k = j+sz;
      જો( z[j]=='-' ){
        j++;
        જો( sz<3 ) પાછા i+1;
      }
      જો( z[j]=='.' ){
        જો( x==JSONB_FLOAT ) પાછા j+1;
        જો( !sqlite3Isdigit(z[j+1]) ) પાછા j+1;
        j += 2;
        seen = 1;
      }નહીંતર જો( z[j]=='0' && x==JSONB_FLOAT ){
        જો( j+3>k ) પાછા j+1;
        જો( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) પાછા j+1;
        j++;
      }
      માટે(; j<k; j++){
        જો( sqlite3Isdigit(z[j]) ) ચાલુ;
        જો( z[j]=='.' ){
          જો( seen>0 ) પાછા j+1;
          જો( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            પાછા j+1;
          }
          seen = 1;
          ચાલુ;
        }
        જો( z[j]=='e' || z[j]=='E' ){
          જો( seen==2 ) પાછા j+1;
          જો( j==k-1 ) પાછા j+1;
          જો( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            જો( j==k-1 ) પાછા j+1;
          }
          seen = 2;
          ચાલુ;
        }
        પાછા j+1;
      }
      જો( seen==0 ) પાછા i+1;
      પાછા 0;
    }
    સ્થિતિ JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      જ્યારે( j<k ){
        જો( !jsonIsOk[z[j]] && z[j]!='\'' ) પાછા j+1;
        j++;
      }
      પાછા 0;
    }
    સ્થિતિ JSONB_TEXTJ:
    સ્થિતિ JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      જ્યારે( j<k ){
        જો( !jsonIsOk[z[j]] && z[j]!='\'' ){
          જો( z[j]=='"' ){
            જો( x==JSONB_TEXTJ ) પાછા j+1;
          }નહીંતર જો( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            જો( x==JSONB_TEXTJ ) પાછા j+1;
          }નહીંતર જો( NEVER(z[j]!='\\') || j+1>=k ){
            પાછા j+1;
          }નહીંતર જો( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }નહીંતર જો( z[j+1]=='u' ){
            જો( j+5>=k ) પાછા j+1;
            જો( !jsonIs4Hex((અચલ અક્ષર*)&z[j+2]) ) પાછા j+1;
            j++;
          }નહીંતર જો( x!=JSONB_TEXT5 ){
            પાછા j+1;
          }નહીંતર{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((અચલ અક્ષર*)&z[j], k-j, &c);
            જો( c==JSON_INVALID_CHAR ) પાછા j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      પાછા 0;
    }
    સ્થિતિ JSONB_TEXTRAW: {
      પાછા 0;
    }
    સ્થિતિ JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      જ્યારે( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        જો( n==0 ) પાછા j+1;
        જો( j+n+sz>k ) પાછા j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        જો( sub ) પાછા sub;
        j += n + sz;
      }
      પ્રતિજ્ઞા( j==k );
      પાછા 0;
    }
    સ્થિતિ JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      જ્યારે( j<k ){
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        જો( n==0 ) પાછા j+1;
        જો( j+n+sz>k ) પાછા j+1;
        જો( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          જો( x<JSONB_TEXT || x>JSONB_TEXTRAW ) પાછા j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        જો( sub ) પાછા sub;
        cnt++;
        j += n + sz;
      }
      પ્રતિજ્ઞા( j==k );
      જો( (cnt & 1)!=0 ) પાછા j+1;
      પાછા 0;
    }
    મૂળભૂત: {
      પાછા i+1;
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
સ્થિર પૂર્ણાંક jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  અક્ષર c;
  u32 j;
  u32 iThis, iStart;
  પૂર્ણાંક x;
  u8 t;
  અચલ અક્ષર *z = pParse->zJson;
json_parse_restart:
  પસંદગી( (u8)z[i] ){
  સ્થિતિ '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_ગાંઠ(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    જો( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      પાછા -1;
    }
    iStart = pParse->nBlob;
    માટે(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      જો( x<=0 ){
        પૂર્ણાંક op;
        જો( x==(-2) ){
          j = pParse->iErr;
          જો( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          તોડો;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        જો( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          પૂર્ણાંક k = j+1;
          જ્યારે( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          પ્રતિજ્ઞા( iBlob==pParse->nBlob );
          json_Blob_Append_ગાંઠ(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }નહીંતર{
          જો( x!=-1 ) pParse->iErr = j;
          પાછા -1;
        }
      }
      જો( pParse->oom ) પાછા -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      જો( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        પાછા -1;
      }
      j = x;
      જો( z[j]==':' ){
        j++;
      }નહીંતર{
        જો( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          કરો{ j++; }જ્યારે( jsonIsspace(z[j]) );
          જો( z[j]==':' ){
            j++;
            જાઓ parse_object_કિંમત;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        જો( x!=(-5) ){
          જો( x!=(-1) ) pParse->iErr = j;
          પાછા -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_કિંમત:
      x = jsonTranslateTextToBlob(pParse, j);
      જો( x<=0 ){
        જો( x!=(-1) ) pParse->iErr = j;
        પાછા -1;
      }
      j = x;
      જો( z[j]==',' ){
        ચાલુ;
      }નહીંતર જો( z[j]=='}' ){
        તોડો;
      }નહીંતર{
        જો( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          જો( z[j]==',' ){
            ચાલુ;
          }નહીંતર જો( z[j]=='}' ){
            તોડો;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        જો( x==(-4) ){
          j = pParse->iErr;
          ચાલુ;
        }
        જો( x==(-2) ){
          j = pParse->iErr;
          તોડો;
        }
      }
      pParse->iErr = j;
      પાછા -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    પાછા j+1;
  }
  સ્થિતિ '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    પ્રતિજ્ઞા( i<=(u32)pParse->nJson );
    json_Blob_Append_ગાંઠ(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    જો( pParse->oom ) પાછા -1;
    જો( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      પાછા -1;
    }
    માટે(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      જો( x<=0 ){
        જો( x==(-3) ){
          j = pParse->iErr;
          જો( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          તોડો;
        }
        જો( x!=(-1) ) pParse->iErr = j;
        પાછા -1;
      }
      j = x;
      જો( z[j]==',' ){
        ચાલુ;
      }નહીંતર જો( z[j]==']' ){
        તોડો;
      }નહીંતર{
        જો( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          જો( z[j]==',' ){
            ચાલુ;
          }નહીંતર જો( z[j]==']' ){
            તોડો;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        જો( x==(-4) ){
          j = pParse->iErr;
          ચાલુ;
        }
        જો( x==(-3) ){
          j = pParse->iErr;
          તોડો;
        }
      }
      pParse->iErr = j;
      પાછા -1;
    }
    jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    પાછા j+1;
  }
  સ્થિતિ '\'': {
    u8 opcode;
    અક્ષર cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    જાઓ parse_string;
  સ્થિતિ '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    જ્યારે( 1 /*exit-by-break*/ ){
      જો( jsonIsOk[(u8)z[j]] ){
        જો( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }નહીંતર જો( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }નહીંતર{
          j += 3;
          ચાલુ;
        }
      }
      c = z[j];
      જો( c==cDelim ){
        તોડો;
      }નહીંતર જો( c=='\\' ){
        c = z[++j];
        જો( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          જો( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }નહીંતર જો( c=='\'' ||  c=='v' || c=='\n'
#ifdef SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#નહીંતર
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#endif
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }નહીંતર જો( c=='\r' ){
          જો( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }નહીંતર{
          pParse->iErr = j;
          પાછા -1;
        }
      }નહીંતર જો( c<=0x1f ){
        જો( c==0 ){
          pParse->iErr = j;
          પાછા -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }નહીંતર જો( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_ગાંઠ(pParse, opcode, j-1-i, &z[i+1]);
    પાછા j+1;
  }
  સ્થિતિ 't': {
    જો( strncmp(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      પાછા i+4;
    }
    pParse->iErr = i;
    પાછા -1;
  }
  સ્થિતિ 'f': {
    જો( strncmp(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      પાછા i+5;
    }
    pParse->iErr = i;
    પાછા -1;
  }
  સ્થિતિ '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    જાઓ parse_સંખ્યા;
  સ્થિતિ '.':
    જો( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      જાઓ parse_સંખ્યા_2;
    }
    pParse->iErr = i;
    પાછા -1;
  સ્થિતિ '-':
  સ્થિતિ '0':
  સ્થિતિ '1':
  સ્થિતિ '2':
  સ્થિતિ '3':
  સ્થિતિ '4':
  સ્થિતિ '5':
  સ્થિતિ '6':
  સ્થિતિ '7':
  સ્થિતિ '8':
  સ્થિતિ '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_સંખ્યા:
    seenE = 0;
    પ્રતિજ્ઞા( '-' < '0' );
    પ્રતિજ્ઞા( '+' < '0' );
    પ્રતિજ્ઞા( '.' < '0' );
    c = z[i];

    જો( c<='0' ){
      જો( c=='0' ){
        જો( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          પ્રતિજ્ઞા( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          માટે(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          જાઓ parse_સંખ્યા_finish;
        }નહીંતર જો( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          પાછા -1;
        }
      }નહીંતર{
        જો( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          જો( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            જો( z[i]=='-' ){
              json_Blob_Append_ગાંઠ(pParse, JSONB_FLOAT, 6, "-9e999");
            }નહીંતર{
              json_Blob_Append_ગાંઠ(pParse, JSONB_FLOAT, 5, "9e999");
            }
            પાછા i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          જો( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            જાઓ parse_સંખ્યા_2;
          }
          pParse->iErr = i;
          પાછા -1;
        }
        જો( z[i+1]=='0' ){
          જો( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            પાછા -1;
          }નહીંતર જો( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            માટે(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            જાઓ parse_સંખ્યા_finish;
          }
        }
      }
    }

  parse_સંખ્યા_2:
    માટે(j=i+1;; j++){
      c = z[j];
      જો( sqlite3Isdigit(c) ) ચાલુ;
      જો( c=='.' ){
        જો( (t & 0x02)!=0 ){
          pParse->iErr = j;
          પાછા -1;
        }
        t |= 0x02;
        ચાલુ;
      }
      જો( c=='e' || c=='E' ){
        જો( z[j-1]<'0' ){
          જો( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }નહીંતર{
            pParse->iErr = j;
            પાછા -1;
          }
        }
        જો( seenE ){
          pParse->iErr = j;
          પાછા -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        જો( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        જો( c<'0' || c>'9' ){
          pParse->iErr = j;
          પાછા -1;
        }
        ચાલુ;
      }
      તોડો;
    }
    જો( z[j-1]<'0' ){
      જો( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }નહીંતર{
        pParse->iErr = j;
        પાછા -1;
      }
    }
  parse_સંખ્યા_finish:
    પ્રતિજ્ઞા( JSONB_INT+0x01==JSONB_INT5 );
    પ્રતિજ્ઞા( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    પ્રતિજ્ઞા( JSONB_INT+0x02==JSONB_FLOAT );
    જો( z[i]=='+' ) i++;
    json_Blob_Append_ગાંઠ(pParse, JSONB_INT+t, j-i, &z[i]);
    પાછા j;
  }
  સ્થિતિ '}': {
    pParse->iErr = i;
    પાછા -2;  /* End of {...} */
  }
  સ્થિતિ ']': {
    pParse->iErr = i;
    પાછા -3;  /* End of [...] */
  }
  સ્થિતિ ',': {
    pParse->iErr = i;
    પાછા -4;  /* List separator */
  }
  સ્થિતિ ':': {
    pParse->iErr = i;
    પાછા -5;  /* Object label/value separator */
  }
  સ્થિતિ 0: {
    પાછા 0;   /* End of file */
  }
  સ્થિતિ 0x09:
  સ્થિતિ 0x0a:
  સ્થિતિ 0x0d:
  સ્થિતિ 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    જાઓ json_parse_restart;
  }
  સ્થિતિ 0x0b:
  સ્થિતિ 0x0c:
  સ્થિતિ '/':
  સ્થિતિ 0xc2:
  સ્થિતિ 0xe1:
  સ્થિતિ 0xe2:
  સ્થિતિ 0xe3:
  સ્થિતિ 0xef: {
    j = json5Whitespace(&z[i]);
    જો( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      જાઓ json_parse_restart;
    }
    pParse->iErr = i;
    પાછા -1;
  }
  સ્થિતિ 'n': {
    જો( strncmp(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      પાછા i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  મૂળભૂત: {
    u32 k;
    પૂર્ણાંક nn;
    c = z[i];
    માટે(k=0; k<કદ(aNanInfName)/કદ(aNanInfName[0]); k++){
      જો( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) ચાલુ;
      nn = aNanInfName[k].n;
      જો( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        ચાલુ;
      }
      જો( sqlite3Isalnum(z[i+nn]) ) ચાલુ;
      જો( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_ગાંઠ(pParse, JSONB_FLOAT, 5, "9e999");
      }નહીંતર{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      પાછા i + nn;
    }
    pParse->iErr = i;
    પાછા -1;  /* Syntax error */
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
સ્થિર પૂર્ણાંક jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  પૂર્ણાંક i;
  અચલ અક્ષર *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  જો( pParse->oom ) i = -1;
  જો( i>0 ){
#ifdef SQLITE_DEBUG
    પ્રતિજ્ઞા( pParse->iDepth==0 );
    જો( sqlite3Config.bJsonSelfcheck ){
      પ્રતિજ્ઞા( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#endif
    જ્યારે( jsonIsspace(zJson[i]) ) i++;
    જો( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      જો( zJson[i] ){
        જો( pCtx ) sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        પાછા 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  જો( i<=0 ){
    જો( pCtx!=0 ){
      જો( pParse->oom ){
        sqlite3_result_error_nomem(pCtx);
      }નહીંતર{
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    પાછા 1;
  }
  પાછા 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
સ્થિર શૂન્ય jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  પ્રતિજ્ઞા( pStr->eErr==0 );
  memset(&px, 0, કદ(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (શૂન્ય)jsonTranslateTextToBlob(&px, 0);
  જો( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  }નહીંતર{
    પ્રતિજ્ઞા( px.nBlobAlloc>0 );
    પ્રતિજ્ઞા( !px.bReadOnly );
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
સ્થિર u32 jsonbPayloadSizeWide(
  અચલ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  પ્રતિજ્ઞા( i<pParse->nBlob );
  પ્રતિજ્ઞા( x==(pParse->aBlob[i]>>4) );
  પ્રતિજ્ઞા( x>=13 );
  જો( x==13 ){
    જો( i+2>=pParse->nBlob ){
      *pSz = 0;
      પાછા 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }નહીંતર જો( x==14 ){
    જો( i+4>=pParse->nBlob ){
      *pSz = 0;
      પાછા 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }નહીંતર{
    જો( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      પાછા 0;
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
  જો( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    પાછા 0;
  }
  *pSz = sz;
  પાછા n;
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
સ્થિર u32 jsonbPayloadSize(
  અચલ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  જો( i>=pParse->nBlob ){
    *pSz = 0;
    પાછા 0;
  }નહીંતર જો( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }નહીંતર જો( x==12 ){
    જો( i+1>=pParse->nBlob ){
      *pSz = 0;
      પાછા 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }નહીંતર{
    પાછા jsonbPayloadSizeWide(pParse, i, pSz, x);
  }
  જો( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    પાછા 0;
  }
  *pSz = sz;
  પાછા n;
}

#જો SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
સ્થિર SQLITE_INLINE u32 jsonbPayloadSizeInline(
  અચલ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  જો( i>=pParse->nBlob ){
    *pSz = 0;
    પાછા 0;
  }નહીંતર જો( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }નહીંતર જો( x==12 ){
    જો( i+1>=pParse->nBlob ){
      *pSz = 0;
      પાછા 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }નહીંતર જો( x==13 ){
    જો( i+2>=pParse->nBlob ){
      *pSz = 0;
      પાછા 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }નહીંતર જો( x==14 ){
    જો( i+4>=pParse->nBlob ){
      *pSz = 0;
      પાછા 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }નહીંતર{
    જો( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      પાછા 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  જો( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    પાછા 0;
  }
  *pSz = sz;
  પાછા n;
}
#નહીંતર /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonbPayloadSizeInline(a,b,c) jsonbPayloadSize(a,b,c)
#endif


#જો SQLITE_USES_INLINE
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
સ્થિર SQLITE_INLINE u32 jsonbPayloadSizeSemiInline(
  અચલ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  પ્રતિજ્ઞા( i<pParse->nBlob );
  જો( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }નહીંતર જો( x==12 ){
    જો( i+1>=pParse->nBlob ){
      *pSz = 0;
      પાછા 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }નહીંતર{
    પાછા jsonbPayloadSizeWide(pParse,i,pSz,x);
  }
  જો( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    પાછા 0;
  }
  *pSz = sz;
  પાછા n;
}
#નહીંતર /* if !SQLITE_USES_INLINE */
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
સ્થિર u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSizeInline(pParse, i, &sz);
  જો( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    પાછા pParse->nBlob+1;
  }
  પસંદગી( pParse->aBlob[i] & 0x0f ){
    સ્થિતિ JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      પાછા i+1;
    }
    સ્થિતિ JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      પાછા i+1;
    }
    સ્થિતિ JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      પાછા i+1;
    }
    સ્થિતિ JSONB_INT:
    સ્થિતિ JSONB_FLOAT: {
      જો( sz==0 ) જાઓ malformed_jsonb;
      jsonAppendRaw(pOut, (અચલ અક્ષર*)&pParse->aBlob[i+n], sz);
      તોડો;
    }
    સ્થિતિ JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      અચલ અક્ષર *zIn = (અચલ અક્ષર*)&pParse->aBlob[i+n];
      પૂર્ણાંક bOverflow = 0;
      જો( sz==0 ) જાઓ malformed_jsonb;
      જો( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }નહીંતર જો( zIn[0]=='+' ){
        k++;
      }
      માટે(; k<sz; k++){
        જો( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          તોડો;
        }નહીંતર જો( (u>>60)!=0 ){
          bOverflow = 1;
        }નહીંતર{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      તોડો;
    }
    સ્થિતિ JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      અચલ અક્ષર *zIn = (અચલ અક્ષર*)&pParse->aBlob[i+n];
      જો( sz==0 ) જાઓ malformed_jsonb;
      જો( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        જો( sz<=1 ) જાઓ malformed_jsonb;
        k = 1;
      }
      જો( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      માટે(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        જો( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      તોડો;
    }
    સ્થિતિ JSONB_TEXT:
    સ્થિતિ JSONB_TEXTJ: {
      જો( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf+pOut->nUsed+1,(અચલ અક્ષર*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      તોડો;
    }
    સ્થિતિ JSONB_TEXT5: {
      અચલ અક્ષર *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (અચલ અક્ષર*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      જ્યારે( sz2>0 ){
        માટે(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        જો( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          જો( k>=sz2 ){
            તોડો;
          }
          zIn += k;
          sz2 -= k;
        }
        જો( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          ચાલુ;
        }
        જો( zIn[0]<=0x1f ){
          જો( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) તોડો;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          ચાલુ;
        }
        પ્રતિજ્ઞા( zIn[0]=='\\' );
        પ્રતિજ્ઞા( sz2>=1 );
        જો( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          તોડો;
        }
        પસંદગી( (u8)zIn[1] ){
          સ્થિતિ '\'':
            jsonAppendChar(pOut, '\'');
            તોડો;
          સ્થિતિ 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            તોડો;
          સ્થિતિ 'x':
            જો( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              તોડો;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            તોડો;
          સ્થિતિ '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            તોડો;
          સ્થિતિ '\r':
            જો( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            તોડો;
          સ્થિતિ '\n':
            તોડો;
          સ્થિતિ 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            જો( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              તોડો;
            }
            zIn += 2;
            sz2 -= 2;
            તોડો;
          મૂળભૂત:
            jsonAppendRawNZ(pOut, zIn, 2);
            તોડો;
        }
        પ્રતિજ્ઞા( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      તોડો;
    }
    સ્થિતિ JSONB_TEXTRAW: {
      jsonAppendString(pOut, (અચલ અક્ષર*)&pParse->aBlob[i+n], sz);
      તોડો;
    }
    સ્થિતિ JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      જો( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      જ્યારે( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      જો( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      જો( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      તોડો;
    }
    સ્થિતિ JSONB_OBJECT: {
      પૂર્ણાંક x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      જો( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      જ્યારે( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      જો( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      જો( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      તોડો;
    }

    મૂળભૂત: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      તોડો;
    }
  }
  પાછા i+n+sz;
}

/* Context for recursion of json_pretty()
*/
પ્રકારવ્યાખ્યા સંરચના JsonPretty JsonPretty;
સંરચના JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  અચલ અક્ષર *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
સ્થિર શૂન્ય jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  માટે(jj=0; jj<pPretty->nIndent; jj++){
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
સ્થિર u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  જો( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    પાછા pParse->nBlob+1;
  }
  પસંદગી( pParse->aBlob[i] & 0x0f ){
    સ્થિતિ JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      જો( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        જો( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        જ્યારે( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          જો( j>=iEnd ){
            જો( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            તોડો;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      તોડો;
    }
    સ્થિતિ JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      જો( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        જો( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        જ્યારે( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          જો( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            તોડો;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          જો( j>=iEnd ) તોડો;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      તોડો;
    }
    મૂળભૂત: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      તોડો;
    }
  }
  પાછા i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
સ્થિર u32 jsonb_Array_ગણતરી(JsonParse *pParse, u32 i_મૂળ){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, i_મૂળ, &sz);
  iEnd = i_મૂળ+n+sz;
  માટે(i=i_મૂળ+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonbPayloadSizeSemiInline(pParse, i, &sz);
  }
  પાછા k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
સ્થિર શૂન્ય jsonAfterEditSizeAdjust(JsonParse *pParse, u32 i_મૂળ){
  u32 sz = 0;
  u32 nBlob;
  પ્રતિજ્ઞા( pParse->delta!=0 );
  પ્રતિજ્ઞા( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (શૂન્ય)jsonbPayloadSize(pParse, i_મૂળ, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, i_મૂળ, sz);
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
સ્થિર પૂર્ણાંક jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  અચલ u8 *aIns,           /* New content */
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
  સ્થિર અચલ u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  જો( (aIns[0]&0x0f)<=2 ) પાછા 0;    /* Cannot enlarge NULL, true, false */
  પસંદગી( aIns[0]>>4 ){
    મૂળભૂત: {                         /* aIns[] header size 1 */
      જો( ((1<<d)&0x116)==0 ) પાછા 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      તોડો;
    }
    સ્થિતિ 12: {                         /* aIns[] header size is 2 */
      જો( ((1<<d)&0x8a)==0) પાછા 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      તોડો;
    }
    સ્થિતિ 13: {                         /* aIns[] header size is 3 */
      જો( d!=2 && d!=6 ) પાછા 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      તોડો;
    }
    સ્થિતિ 14: {                         /* aIns[] header size is 5 */
      જો( d!=4 ) પાછા 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      તોડો;
    }
    સ્થિતિ 15: {                         /* aIns[] header size is 9 */
      પાછા 0;                          /* No solution */
    }
  }
  પ્રતિજ્ઞા( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  memcpy(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  જ્યારે( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    જો( i==1 ) તોડો;
    szPayload >>= 8;
  }
  પ્રતિજ્ઞા( (szPayload>>8)==0 );
  પાછા 1;
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
સ્થિર શૂન્ય jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  અચલ u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  પ્રતિજ્ઞા( pParse->nBlob >= (u64)iDel + (u64)nDel );
  જો( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (પૂર્ણાંક)-d)
  ){
    પાછા;
  }
  જો( d!=0 ){
    જો( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      જો( pParse->oom ) પાછા;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  જો( nIns && aIns ){
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
સ્થિર u32 jsonBytesToBypass(અચલ અક્ષર *z, u32 n){
  u32 i = 0;
  જ્યારે( i+1<n ){
    જો( z[i]!='\\' ) પાછા i;
    જો( z[i+1]=='\n' ){
      i += 2;
      ચાલુ;
    }
    જો( z[i+1]=='\r' ){
      જો( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }નહીંતર{
        i += 2;
      }
      ચાલુ;
    }
    જો( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      ચાલુ;
    }
    તોડો;
  }
  પાછા i;
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
સ્થિર u32 jsonUnescapeOneChar(અચલ અક્ષર *z, u32 n, u32 *piOut){
  પ્રતિજ્ઞા( n>0 );
  પ્રતિજ્ઞા( z[0]=='\\' );
  જો( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    પાછા n;
  }
  પસંદગી( (u8)z[1] ){
    સ્થિતિ 'u': {
      u32 v, vlo;
      જો( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        પાછા n;
      }
      v = jsonHexToInt4(&z[2]);
      જો( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        પાછા 12;
      }નહીંતર{
        *piOut = v;
        પાછા 6;
      }
    }
    સ્થિતિ 'b': {   *piOut = '\b';  પાછા 2; }
    સ્થિતિ 'f': {   *piOut = '\f';  પાછા 2; }
    સ્થિતિ 'n': {   *piOut = '\n';  પાછા 2; }
    સ્થિતિ 'r': {   *piOut = '\r';  પાછા 2; }
    સ્થિતિ 't': {   *piOut = '\t';  પાછા 2; }
    સ્થિતિ 'v': {   *piOut = '\v';  પાછા 2; }
    સ્થિતિ '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ifdef SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#નહીંતર
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#endif
      પાછા 2;
    }
    સ્થિતિ '\'':
    સ્થિતિ '"':
    સ્થિતિ '/':
    સ્થિતિ '\\':{   *piOut = z[1];  પાછા 2; }
    સ્થિતિ 'x': {
      જો( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        પાછા n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      પાછા 4;
    }
    સ્થિતિ 0xe2:
    સ્થિતિ '\r':
    સ્થિતિ '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      જો( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        પાછા n;
      }નહીંતર જો( nSkip==n ){
        *piOut = 0;
        પાછા n;
      }નહીંતર જો( z[nSkip]=='\\' ){
        પાછા nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }નહીંતર{
        પૂર્ણાંક sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        પાછા nSkip + sz;
      }
    }
    મૂળભૂત: {
      *piOut = JSON_INVALID_CHAR;
      પાછા 2;
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
સ્થિર SQLITE_NOINLINE પૂર્ણાંક jsonLabelCompareEscaped(
  અચલ અક્ષર *z_ડાબું,          /* The left label */
  u32 n_ડાબું,                  /* Size of the left label in bytes */
  પૂર્ણાંક raw_ડાબું,                /* True if zLeft contains no escapes */
  અચલ અક્ષર *z_જમણું,         /* The right label */
  u32 n_જમણું,                 /* Size of the right label in bytes */
  પૂર્ણાંક raw_જમણું                /* True if zRight is escape-free */
){
  u32 c_ડાબું, c_જમણું;
  પ્રતિજ્ઞા( raw_ડાબું==0 || raw_જમણું==0 );
  જ્યારે( 1 /*exit-by-return*/ ){
    જો( n_ડાબું==0 ){
      c_ડાબું = 0;
    }નહીંતર જો( raw_ડાબું || z_ડાબું[0]!='\\' ){
      c_ડાબું = ((u8*)z_ડાબું)[0];
      જો( c_ડાબું>=0xc0 ){
        પૂર્ણાંક sz = sqlite3Utf8ReadLimited((u8*)z_ડાબું, n_ડાબું, &c_ડાબું);
        z_ડાબું += sz;
        n_ડાબું -= sz;
      }નહીંતર{
        z_ડાબું++;
        n_ડાબું--;
      }
    }નહીંતર{
      u32 n = jsonUnescapeOneChar(z_ડાબું, n_ડાબું, &c_ડાબું);
      z_ડાબું += n;
      પ્રતિજ્ઞા( n<=n_ડાબું );
      n_ડાબું -= n;
    }
    જો( n_જમણું==0 ){
      c_જમણું = 0;
    }નહીંતર જો( raw_જમણું || z_જમણું[0]!='\\' ){
      c_જમણું = ((u8*)z_જમણું)[0];
      જો( c_જમણું>=0xc0 ){
        પૂર્ણાંક sz = sqlite3Utf8ReadLimited((u8*)z_જમણું, n_જમણું, &c_જમણું);
        z_જમણું += sz;
        n_જમણું -= sz;
      }નહીંતર{
        z_જમણું++;
        n_જમણું--;
      }
    }નહીંતર{
      u32 n = jsonUnescapeOneChar(z_જમણું, n_જમણું, &c_જમણું);
      z_જમણું += n;
      પ્રતિજ્ઞા( n<=n_જમણું );
      n_જમણું -= n;
    }
    જો( c_ડાબું!=c_જમણું ) પાછા 0;
    જો( c_ડાબું==0 ) પાછા 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
સ્થિર પૂર્ણાંક jsonLabelCompare(
  અચલ અક્ષર *z_ડાબું,          /* The left label */
  u32 n_ડાબું,                  /* Size of the left label in bytes */
  પૂર્ણાંક raw_ડાબું,                /* True if zLeft contains no escapes */
  અચલ અક્ષર *z_જમણું,         /* The right label */
  u32 n_જમણું,                 /* Size of the right label in bytes */
  પૂર્ણાંક raw_જમણું                /* True if zRight is escape-free */
){
  જો( raw_ડાબું && raw_જમણું ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    જો( n_ડાબું!=n_જમણું ) પાછા 0;
    પાછા memcmp(z_ડાબું, z_જમણું, n_ડાબું)==0;
  }નહીંતર{
    પાછા jsonLabelCompareEscaped(z_ડાબું, n_ડાબું, raw_ડાબું,
                                   z_જમણું, n_જમણું, raw_જમણું);
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
સ્થિર u32 jsonLookupStep(JsonParse*,u32,અચલ અક્ષર*,u32);


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
સ્થિર u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  અચલ અક્ષર *zTail   /* Tail of the path that determines substructure */
){
  સ્થિર અચલ u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  પૂર્ણાંક rc;
  memset(pIns, 0, કદ(*pIns));
  pIns->db = pParse->db;
  જો( zTail[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }નહીંતર{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[zTail[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    જો( pIns->iDepth >= JSON_MAX_DEPTH ){
      પાછા JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  પાછા rc;  /* Error code only */
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
સ્થિર u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_મૂળ,              /* Begin the search at this element of aBlob[] */
  અચલ અક્ષર *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_ચાવી, sz, n, iEnd, rc;
  અચલ અક્ષર *z_ચાવી;
  u8 x;

  જો( zPath[0]==0 ){
    જો( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonbPayloadSize(pParse, i_મૂળ, &sz);
      sz += n;
      જો( pParse->eEdit==JEDIT_DEL ){
        જો( iLabel>0 ){
          sz += i_મૂળ - iLabel;
          i_મૂળ = iLabel;
        }
        jsonBlobEdit(pParse, i_મૂળ, sz, 0, 0);
      }નહીંતર જો( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }નહીંતર જો( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        જો( zPath[-1]!=']' ){
          પાછા JSON_LOOKUP_NOTARRAY;
        }નહીંતર{
          jsonBlobEdit(pParse, i_મૂળ, 0, pParse->aIns, pParse->nIns);
        }
      }નહીંતર{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_મૂળ, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    પાછા i_મૂળ;
  }
  જો( zPath[0]=='.' ){
    પૂર્ણાંક raw_ચાવી = 1;
    x = pParse->aBlob[i_મૂળ];
    zPath++;
    જો( zPath[0]=='"' ){
      z_ચાવી = zPath + 1;
      માટે(i=1; zPath[i] && zPath[i]!='"'; i++){
        જો( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_ચાવી = i-1;
      જો( zPath[i] ){
        i++;
      }નહીંતર{
        પાછા JSON_LOOKUP_PATHERROR;
      }
      testcase( n_ચાવી==0 );
      raw_ચાવી = memchr(z_ચાવી, '\\', n_ચાવી)==0;
    }નહીંતર{
      z_ચાવી = zPath;
      માટે(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_ચાવી = i;
      જો( n_ચાવી==0 ){
        પાછા JSON_LOOKUP_PATHERROR;
      }
    }
    જો( (x & 0x0f)!=JSONB_OBJECT ) પાછા JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_મૂળ, &sz);
    j = i_મૂળ + n;  /* j is the index of a label */
    iEnd = j+sz;
    જ્યારે( j<iEnd ){
      પૂર્ણાંક rawLabel;
      અચલ અક્ષર *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      જો( x<JSONB_TEXT || x>JSONB_TEXTRAW ) પાછા JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      જો( n==0 ) પાછા JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      જો( k+sz>=iEnd ) પાછા JSON_LOOKUP_ERROR;
      zLabel = (અચલ અક્ષર*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      જો( jsonLabelCompare(z_ચાવી, n_ચાવી, raw_ચાવી, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        જો( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) પાછા JSON_LOOKUP_ERROR;
        n = jsonbPayloadSize(pParse, v, &sz);
        જો( n==0 || v+n+sz>iEnd ) પાછા JSON_LOOKUP_ERROR;
        પ્રતિજ્ઞા( j>0 );
        જો( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          પાછા JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        જો( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_મૂળ);
        પાછા rc;
      }
      j = k+sz;
      જો( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) પાછા JSON_LOOKUP_ERROR;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      જો( n==0 ) પાછા JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    જો( j>iEnd ) પાછા JSON_LOOKUP_ERROR;
    જો( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      જો( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        પાછા JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, કદ(ix));
      ix.db = pParse->db;
      json_Blob_Append_ગાંઠ(&ix, raw_ચાવી?JSONB_TEXTRAW:JSONB_TEXT5, n_ચાવી, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      જો( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_ચાવી+v.nBlob)
      ){
        પ્રતિજ્ઞા( !pParse->oom );
        nIns = ix.nBlob + n_ચાવી + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        જો( !pParse->oom ){
          પ્રતિજ્ઞા( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          પ્રતિજ્ઞા( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], z_ચાવી, n_ચાવી);
          k += n_ચાવી;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          જો( ALWAYS(pParse->delta) ) jsonAfterEditSizeAdjust(pParse, i_મૂળ);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      પાછા rc;
    }
  }નહીંતર જો( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_મૂળ] & 0x0f;
    જો( x!=JSONB_ARRAY )  પાછા JSON_LOOKUP_NOTFOUND;
    n = jsonbPayloadSize(pParse, i_મૂળ, &sz);
    i = 1;
    જ્યારે( sqlite3Isdigit(zPath[i]) ){
      જો( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    જો( i<2 || zPath[i]!=']' ){
      જો( zPath[1]=='#' ){
        kk = jsonb_Array_ગણતરી(pParse, i_મૂળ);
        i = 2;
        જો( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          કરો{
            જો( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }જ્યારે( sqlite3Isdigit(zPath[i]) );
          જો( nn>kk ) પાછા JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        જો( zPath[i]!=']' ){
          પાછા JSON_LOOKUP_PATHERROR;
        }
      }નહીંતર{
        પાછા JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_મૂળ+n;
    iEnd = j+sz;
    જ્યારે( j<iEnd ){
      જો( kk==0 ){
        જો( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          પાછા JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        જો( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_મૂળ);
        પાછા rc;
      }
      kk--;
      n = jsonbPayloadSizeSemiInline(pParse, j, &sz);
      જો( n==0 ) પાછા JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    જો( j>iEnd ) પાછા JSON_LOOKUP_ERROR;
    જો( kk>0 ) પાછા JSON_LOOKUP_NOTFOUND;
    જો( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      જો( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        પ્રતિજ્ઞા( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      જો( pParse->delta ) jsonAfterEditSizeAdjust(pParse, i_મૂળ);
      પાછા rc;
    }
  }નહીંતર{
    પાછા JSON_LOOKUP_PATHERROR; 
  }
  પાછા JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
સ્થિર શૂન્ય jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  અચલ u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  જો( NEVER(aBlob==0) ) પાછા;
  memset(&x, 0, કદ(x));
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
સ્થિર શૂન્ય jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  પૂર્ણાંક eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  પૂર્ણાંક rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  પ્રતિજ્ઞા( eMode>=0 && eMode<=2 );
  n = jsonbPayloadSize(pParse, i, &sz);
  જો( n==0 ){
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    પાછા;
  }
  પસંદગી( pParse->aBlob[i] & 0x0f ){
    સ્થિતિ JSONB_NULL: {
      જો( sz ) જાઓ returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      તોડો;
    }
    સ્થિતિ JSONB_TRUE: {
      જો( sz ) જાઓ returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      તોડો;
    }
    સ્થિતિ JSONB_FALSE: {
      જો( sz ) જાઓ returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      તોડો;
    }
    સ્થિતિ JSONB_INT5:
    સ્થિતિ JSONB_INT: {
      sqlite3_int64 iRes = 0;
      અક્ષર *z;
      પૂર્ણાંક bNeg = 0;
      અક્ષર x;
      જો( sz==0 ) જાઓ returnfromblob_malformed;
      x = (અક્ષર)pParse->aBlob[i+n];
      જો( x=='-' ){
        જો( sz<2 ) જાઓ returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (અચલ અક્ષર*)&pParse->aBlob[i+n], (પૂર્ણાંક)sz);
      જો( z==0 ) જાઓ returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      જો( rc==0 ){
        જો( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          દ્વિગુણ r;
          r = (દ્વિગુણ)*(sqlite3_uint64*)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        }નહીંતર{
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }નહીંતર જો( rc==3 && bNeg ){
        sqlite3_result_int64(pCtx, SMALLEST_INT64);
      }નહીંતર જો( rc==1 ){
        જાઓ returnfromblob_malformed;
      }નહીંતર{
        જો( bNeg ){ n--; sz++; }
        જાઓ to_double;
      }
      તોડો;
    }
    સ્થિતિ JSONB_FLOAT5:
    સ્થિતિ JSONB_FLOAT: {
      દ્વિગુણ r;
      અક્ષર *z;
      જો( sz==0 ) જાઓ returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (અચલ અક્ષર*)&pParse->aBlob[i+n], (પૂર્ણાંક)sz);
      જો( z==0 ) જાઓ returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      જો( rc<=0 ) જાઓ returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      તોડો;
    }
    સ્થિતિ JSONB_TEXTRAW:
    સ્થિતિ JSONB_TEXT: {
      sqlite3_result_text(pCtx, (અક્ષર*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      તોડો;
    }
    સ્થિતિ JSONB_TEXT5:
    સ્થિતિ JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      અચલ અક્ષર *z;
      અક્ષર *zOut;
      u32 nOut = sz;
      z = (અચલ અક્ષર*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      જો( zOut==0 ) જાઓ returnfromblob_oom;
      માટે(iIn=iOut=0; iIn<sz; iIn++){
        અક્ષર c = z[iIn];
        જો( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          જો( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }નહીંતર જો( v<=0x7f ){
            zOut[iOut++] = (અક્ષર)v;
          }નહીંતર જો( v<=0x7ff ){
            પ્રતિજ્ઞા( szEscape>=2 );
            zOut[iOut++] = (અક્ષર)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }નહીંતર જો( v<0x10000 ){
            પ્રતિજ્ઞા( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }નહીંતર જો( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }નહીંતર{
            પ્રતિજ્ઞા( szEscape>=4 );
            zOut[iOut++] = 0xf0 | (v>>18);
            zOut[iOut++] = 0x80 | ((v>>12)&0x3f);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }
          iIn += szEscape - 1;
        }નહીંતર{
          zOut[iOut++] = c;
        }
      } /* end for() */
      પ્રતિજ્ઞા( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      તોડો;
    }
    સ્થિતિ JSONB_ARRAY:
    સ્થિતિ JSONB_OBJECT: {
      જો( eMode==0 ){
        જો( (SQLITE_PTR_TO_INT(sqlite3_user_data(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }નહીંતર{
          eMode = 1;
        }
      }
      જો( eMode==2 ){
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }નહીંતર{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      તોડો;
    }
    મૂળભૂત: {
      જાઓ returnfromblob_malformed;
    }
  }
  પાછા;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  પાછા;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  પાછા;
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
સ્થિર પૂર્ણાંક jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_કિંમત *pArg,
  JsonParse *pParse
){
  પૂર્ણાંક eType = sqlite3_કિંમત_type(pArg);
  સ્થિર u8 aNull[] = { 0x00 };
  memset(pParse, 0, કદ(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  પસંદગી( eType ){
    મૂળભૂત: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      પાછા 0;
    }
    સ્થિતિ SQLITE_BLOB: {
      જો( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
        પાછા 1;
      }
      તોડો;
    }
    સ્થિતિ SQLITE_TEXT: {
      અચલ અક્ષર *zJson = (અચલ અક્ષર*)sqlite3_કિંમત_text(pArg);
      પૂર્ણાંક nJson = sqlite3_કિંમત_bytes(pArg);
      જો( zJson==0 ) પાછા 1;
      જો( sqlite3_કિંમત_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (અક્ષર*)zJson;
        pParse->nJson = nJson;
        જો( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_result_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, કદ(pParse[0]));
          પાછા 1;
        }
      }નહીંતર{
        json_Blob_Append_ગાંઠ(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      તોડો;
    }
    સ્થિતિ SQLITE_FLOAT: {
      જો( NEVER(sqlite3IsNaN(sqlite3_કિંમત_double(pArg))) ){
        json_Blob_Append_ગાંઠ(pParse, JSONB_NULL, 0, 0);
      }નહીંતર{
        પૂર્ણાંક n = sqlite3_કિંમત_bytes(pArg);
        અચલ અક્ષર *z = (અચલ અક્ષર*)sqlite3_કિંમત_text(pArg);
        જો( z==0 ) પાછા 1;
        જો( z[0]=='I' ){
          json_Blob_Append_ગાંઠ(pParse, JSONB_FLOAT, 5, "9e999");
        }નહીંતર જો( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_ગાંઠ(pParse, JSONB_FLOAT, 6, "-9e999");
        }નહીંતર{
          json_Blob_Append_ગાંઠ(pParse, JSONB_FLOAT, n, z);
        }
      }
      તોડો;
    }
    સ્થિતિ SQLITE_INTEGER: {
      પૂર્ણાંક n = sqlite3_કિંમત_bytes(pArg);
      અચલ અક્ષર *z = (અચલ અક્ષર*)sqlite3_કિંમત_text(pArg);
      જો( z==0 ) પાછા 1;
      json_Blob_Append_ગાંઠ(pParse, JSONB_INT, n, z);
      તોડો;
    }
  }
  જો( pParse->oom ){
    sqlite3_result_error_nomem(ctx);
    પાછા 1;
  }નહીંતર{
    પાછા 0;
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
સ્થિર અક્ષર *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  અચલ અક્ષર *zPath,        /* The path with the problem */
  પૂર્ણાંક rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  અક્ષર *z_સંદેશ;
  જો( rc==(પૂર્ણાંક)JSON_LOOKUP_NOTARRAY ){
    z_સંદેશ = sqlite3_mprintf("not an array element: %Q", zPath);
  }નહીંતર જો( rc==(પૂર્ણાંક)JSON_LOOKUP_ERROR ){
    z_સંદેશ = sqlite3_mprintf("malformed JSON");
  }નહીંતર જો( rc==(પૂર્ણાંક)JSON_LOOKUP_TOODEEP ){
    z_સંદેશ = sqlite3_mprintf("JSON path too deep");
  }નહીંતર{
    z_સંદેશ = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  જો( ctx==0 ) પાછા z_સંદેશ;
  જો( z_સંદેશ ){
    sqlite3_result_error(ctx, z_સંદેશ, -1);
    sqlite3_free(z_સંદેશ);
  }નહીંતર{
    sqlite3_result_error_nomem(ctx);
  }
  પાછા 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
સ્થિર શૂન્ય json_ઉમેરો_Into_Blob(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv,
  પૂર્ણાંક eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  પૂર્ણાંક i;
  u32 rc = 0;
  અચલ અક્ષર *zPath = 0;
  પૂર્ણાંક flgs;
  JsonParse *p;
  JsonParse ax;

  પ્રતિજ્ઞા( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  જો( p==0 ) પાછા;
  માટે(i=1; i<argc-1; i+=2){
    જો( sqlite3_કિંમત_type(argv[i])==SQLITE_NULL ) ચાલુ;
    zPath = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
    જો( zPath==0 ){
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      પાછા;
    }
    જો( zPath[0]!='$' ) જાઓ json_ઉમેરો_Into_Blob_patherror;
    જો( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      પાછા;
    }
    જો( zPath[1]==0 ){
      જો( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }નહીંતર{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath+1, 0);
    }
    jsonParseReset(&ax);
    જો( rc==JSON_LOOKUP_NOTFOUND ) ચાલુ;
    જો( JSON_LOOKUP_ISERROR(rc) ) જાઓ json_ઉમેરો_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  પાછા;

json_ઉમેરો_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  પાછા;
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
સ્થિર પૂર્ણાંક jsonArgIsJsonb(sqlite3_કિંમત *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  જો( sqlite3_કિંમત_type(pArg)!=SQLITE_BLOB ) પાછા 0;
  p->aBlob = (u8*)sqlite3_કિંમત_blob(pArg);
  p->nBlob = (u32)sqlite3_કિંમત_bytes(pArg);
  જો( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonbPayloadSize(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    પાછા 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  પાછા 0;
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
સ્થિર JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_કિંમત *pArg,
  u32 flgs
){
  પૂર્ણાંક eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  પ્રતિજ્ઞા( ctx!=0 );
  eType = sqlite3_કિંમત_type(pArg);
  જો( eType==SQLITE_NULL ){
    પાછા 0;
  }
  જો( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_શોધ(ctx, pArg);
  }
  જો( pFromCache ){
    pFromCache->nJPRef++;
    જો( (flgs & JSON_EDITABLE)==0 ){
      પાછા pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, કદ(*p));
  જો( p==0 ) જાઓ json_pfa_oom;
  memset(p, 0, કદ(*p));
  p->db = db;
  p->nJPRef = 1;
  જો( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    જો( p->aBlob==0 ) જાઓ json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    પાછા p;
  }
  જો( eType==SQLITE_BLOB ){
    જો( jsonArgIsJsonb(pArg,p) ){
      જો( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        જાઓ json_pfa_oom;
      }
      પાછા p;
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
  p->zJson = (અક્ષર*)sqlite3_કિંમત_text(pArg);
  p->nJson = sqlite3_કિંમત_bytes(pArg);
  જો( db->mallocFailed ) જાઓ json_pfa_oom;
  જો( p->nJson==0 ) જાઓ json_pfa_malformed;
  પ્રતિજ્ઞા( p->zJson!=0 );
  જો( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    જો( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      પાછા p;
    }નહીંતર{
      jsonParseFree(p);
      પાછા 0;
    }
  }નહીંતર{
    પૂર્ણાંક isRCStr = sqlite3_કિંમત_Is_Of_Class(pArg, sqlite3RCStrUnref);
    પૂર્ણાંક rc;
    જો( !isRCStr ){
      અક્ષર *zNew = sqlite3RCStrNew( p->nJson );
      જો( zNew==0 ) જાઓ json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }નહીંતર{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_ઉમેરો(ctx, p);
    જો( rc==SQLITE_NOMEM ) જાઓ json_pfa_oom;
    જો( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      જાઓ rebuild_from_cache;
    }
  }
  પાછા p;

json_pfa_malformed:
  જો( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    પાછા p;
  }નહીંતર{
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    પાછા 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  પાછા 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
સ્થિર શૂન્ય jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  પૂર્ણાંક flgs;
  જો( p->oom ){
    sqlite3_result_error_nomem(ctx);
    પાછા;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  જો( flgs & JSON_BLOB ){
    જો( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }નહીંતર{
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }નહીંતર{
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

#જો SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
સ્થિર શૂન્ય jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  પૂર્ણાંક nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  જ્યારે( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    પૂર્ણાંક showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    જો( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonbPayloadSize(pParse, iStart, &sz);
    જો( nn==0 ) nn = 1;
    જો( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    માટે(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    જો( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      ચાલુ;
    }
    pParse->nBlob = savedNBlob;
    જો( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      જો( iEnd>pParse->nBlob ){
        જો( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }નહીંતર{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    પસંદગી( x ){
      સ્થિતિ JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); તોડો;
      સ્થિતિ JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); તોડો;
      સ્થિતિ JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); તોડો;
      સ્થિતિ JSONB_INT:      sqlite3_str_appendall(pOut,"int"); તોડો;
      સ્થિતિ JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); તોડો;
      સ્થિતિ JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); તોડો;
      સ્થિતિ JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); તોડો;
      સ્થિતિ JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); તોડો;
      સ્થિતિ JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); તોડો;
      સ્થિતિ JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); તોડો;
      સ્થિતિ JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); તોડો;
      સ્થિતિ JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        તોડો;
      }
      સ્થિતિ JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        તોડો;
      }
      મૂળભૂત: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        તોડો;
      }
    }
    જો( showContent ){
      જો( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }નહીંતર{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        માટે(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          જો( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (અક્ષર*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
સ્થિર શૂન્ય jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  અક્ષર zBuf[1000];
  જો( pParse==0 ){
    printf("NULL pointer\n");
    પાછા;
  }નહીંતર{
    printf("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    printf("nBlob = %u\n", pParse->nBlob);
    printf("delta = %d\n", pParse->delta);
    જો( pParse->nBlob==0 ) પાછા;
    printf("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, કદ(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  printf("%s", sqlite3_str_કિંમત(&out));
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
સ્થિર શૂન્ય jsonParseFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  પ્રતિજ્ઞા( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  જો( p==0 ) પાછા;
  જો( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_result_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }નહીંતર{
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
સ્થિર શૂન્ય jsonQuoteFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_કિંમત(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
સ્થિર શૂન્ય jsonArrayFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  માટે(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_કિંમત(&jx, argv[i]);
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
સ્થિર શૂન્ય jsonArrayLengthFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  જો( p==0 ) પાછા;
  જો( argc==2 ){
    અચલ અક્ષર *zPath = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]);
    જો( zPath==0 ){
      jsonParseFree(p);
      પાછા;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    જો( JSON_LOOKUP_ISERROR(i) ){
      જો( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }નહીંતર{
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  }નહીંતર{
    i = 0;
  }
  જો( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_ગણતરી(p, i);
  }
  જો( !eErr ) sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
સ્થિર પૂર્ણાંક jsonAllAlphanum(અચલ અક્ષર *z, પૂર્ણાંક n){
  પૂર્ણાંક i;
  માટે(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  પાછા i==n;
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
સ્થિર શૂન્ય jsonExtractFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonParse *p = 0;      /* The parse */
  પૂર્ણાંક flags;             /* Flags associated with the function */
  પૂર્ણાંક i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  જો( argc<2 ) પાછા;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  જો( p==0 ) પાછા;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  jsonStringInit(&jx, ctx);
  જો( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  માટે(i=1; i<argc; i++){
    /* With a single PATH argument */
    અચલ અક્ષર *zPath = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
    પૂર્ણાંક nPath;
    u32 j;
    જો( zPath==0 ) જાઓ json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    જો( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }નહીંતર જો( (flags & JSON_ABPATH) ){
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
      જો( sqlite3_કિંમત_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        જો( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }નહીંતર જો( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }નહીંતર જો( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
        jsonAppendRaw(&jx, zPath, nPath);
      }નહીંતર{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }નહીંતર{
      jsonBadPathError(ctx, zPath, 0);
      જાઓ json_extract_error;
    }
    જો( j<p->nBlob ){
      જો( argc==2 ){
        જો( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          પ્રતિજ્ઞા( (flags & JSON_BLOB)==0 );
          sqlite3_result_subtype(ctx, JSON_SUBTYPE);
        }નહીંતર{
          jsonReturnFromBlob(p, j, ctx, 0);
          જો( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_result_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }નહીંતર{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }નહીંતર જો( j==JSON_LOOKUP_NOTFOUND ){
      જો( argc==2 ){
        જાઓ json_extract_error;  /* Return NULL if not found */
      }નહીંતર{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }નહીંતર{
      jsonBadPathError(ctx, zPath, j);
      જાઓ json_extract_error;
    }
  }
  જો( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    જો( (flags & JSON_BLOB)==0 ){
      sqlite3_result_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  પાછા;
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
સ્થિર પૂર્ણાંક jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  અચલ JsonParse *pPatch, /* The PATCH */
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

  પ્રતિજ્ઞા( iTarget>=0 && iTarget<pTarget->nBlob );
  પ્રતિજ્ઞા( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  જો( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    પાછા pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  જો( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  જો( NEVER(n==0) ) પાછા JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  જો( NEVER(n==0) ) પાછા JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  જ્યારે( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    જો( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      પાછા JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    જો( nPLabel==0 ) પાછા JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    જો( iPValue>=iPEnd ) પાછા JSON_MERGE_BADPATCH;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    જો( nPValue==0 ) પાછા JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    જો( iPCursor>iPEnd ) પાછા JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    જ્યારે( iTCursor<iTEnd ){
      પૂર્ણાંક isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      જો( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        પાછા JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      જો( nTLabel==0 ) પાછા JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      જો( iTValue>=iTEnd ) પાછા JSON_MERGE_BADTARGET;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      જો( nTValue==0 ) પાછા JSON_MERGE_BADTARGET;
      જો( iTValue + nTValue + szTValue > iTEnd ) પાછા JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (અચલ અક્ષર*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (અચલ અક્ષર*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      જો( isEqual ) તોડો;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    જો( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      જો( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        જો( NEVER(pTarget->oom) ) પાછા JSON_MERGE_OOM;
      }નહીંતર{
        /* Algorithm line 12 */
        પૂર્ણાંક rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        જો( iDepth>=JSON_MAX_DEPTH ) પાછા JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        જો( rc ) પાછા rc;
        pTarget->delta += savedDelta;
      }        
    }નહીંતર જો( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      જો( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        જો( pTarget->oom ) પાછા JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }નહીંતર{
        પૂર્ણાંક rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        જો( pTarget->oom ) પાછા JSON_MERGE_OOM;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        જો( iDepth>=JSON_MAX_DEPTH ) પાછા JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        જો( rc ) પાછા rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  જો( pTarget->delta ) jsonAfterEditSizeAdjust(pTarget, iTarget);
  પાછા pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
સ્થિર શૂન્ય jsonPatchFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  પૂર્ણાંક rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  પ્રતિજ્ઞા( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  જો( pTarget==0 ) પાછા;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  જો( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    જો( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }નહીંતર જો( rc==JSON_MERGE_OOM ){
      sqlite3_result_error_nomem(ctx);
    }નહીંતર જો( rc==JSON_MERGE_TOODEEP ){
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    }નહીંતર{
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
સ્થિર શૂન્ય jsonObjectFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક i;
  JsonString jx;
  અચલ અક્ષર *z;
  u32 n;

  જો( argc&1 ){
    sqlite3_result_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    પાછા;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  માટે(i=0; i<argc; i+=2){
    જો( sqlite3_કિંમત_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      પાછા;
    }
    jsonAppendSeparator(&jx);
    z = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
    n = sqlite3_કિંમત_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_કિંમત(&jx, argv[i+1]);
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
સ્થિર શૂન્ય jsonRemoveFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonParse *p;          /* The parse */
  અચલ અક્ષર *zPath = 0; /* Path of element to be removed */
  પૂર્ણાંક i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  જો( argc<1 ) પાછા;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  જો( p==0 ) પાછા;
  માટે(i=1; i<argc; i++){
    zPath = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
    જો( zPath==0 ){
      જાઓ json_remove_done;
    }
    જો( zPath[0]!='$' ){
      જાઓ json_remove_patherror;
    }
    જો( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      જાઓ json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    જો( JSON_LOOKUP_ISERROR(rc) ){
      જો( rc==JSON_LOOKUP_NOTFOUND ){
        ચાલુ;  /* No-op */
      }નહીંતર{
        jsonBadPathError(ctx, zPath, rc);
      }
      જાઓ json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  પાછા;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  પાછા;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
સ્થિર શૂન્ય jsonReplaceFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  જો( argc<1 ) પાછા;
  જો( (argc&1)==0 ) {
    json_Wrong_અંક_Args(ctx, "replace");
    પાછા;
  }
  json_ઉમેરો_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
સ્થિર શૂન્ય jsonSetFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  પૂર્ણાંક eInsType = JSON_ઉમેરો_TYPE(flags);
  સ્થિર અચલ અક્ષર *azInsType[] = { "insert", "set", "array_insert" };
  સ્થિર અચલ u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  જો( argc<1 ) પાછા;
  પ્રતિજ્ઞા( eInsType>=0 && eInsType<=2 );
  જો( (argc&1)==0 ) {
    json_Wrong_અંક_Args(ctx, azInsType[eInsType]);
    પાછા;
  }
  json_ઉમેરો_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
સ્થિર શૂન્ય jsonTypeFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonParse *p;          /* The parse */
  અચલ અક્ષર *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  જો( p==0 ) પાછા;
  જો( argc==2 ){
    zPath = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]);
    જો( zPath==0 ) જાઓ json_type_done;
    જો( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      જાઓ json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    જો( JSON_LOOKUP_ISERROR(i) ){
      જો( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }નહીંતર{
        jsonBadPathError(ctx, zPath, i);
      }
      જાઓ json_type_done;
    }
  }નહીંતર{
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
સ્થિર શૂન્ય jsonPrettyFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, કદ(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  જો( x.pParse==0 ) પાછા;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  જો( argc==1 || (x.zIndent = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }નહીંતર{
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
સ્થિર શૂન્ય jsonValidFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  જો( argc==2 ){
    i64 f = sqlite3_કિંમત_int64(argv[1]);
    જો( f<1 || f>15 ){
      sqlite3_result_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      પાછા;
    }
    flags = f & 0x0f;
  }
  પસંદગી( sqlite3_કિંમત_type(argv[0]) ){
    સ્થિતિ SQLITE_NULL: {
#ifdef SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_result_int(ctx, 0);
#endif
      પાછા;
    }
    સ્થિતિ SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, કદ(py));
      જો( jsonArgIsJsonb(argv[0], &py) ){
        જો( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }નહીંતર જો( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        તોડો;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    મૂળભૂત: {
      JsonParse px;
      જો( (flags & 0x3)==0 ) તોડો;
      memset(&px, 0, કદ(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      જો( p ){
        જો( p->oom ){
          sqlite3_result_error_nomem(ctx);
        }નહીંતર જો( p->nErr ){
          /* no-op */
        }નહીંતર જો( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }નહીંતર{
        sqlite3_result_error_nomem(ctx);
      }
      તોડો;
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
સ્થિર શૂન્ય jsonErrorFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  પ્રતિજ્ઞા( argc==1 );
  UNUSED_PARAMETER(argc);
  memset(&s, 0, કદ(s));
  s.db = sqlite3_context_db_handle(ctx);
  જો( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }નહીંતર{
    s.zJson = (અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    જો( s.zJson==0 ) પાછા;  /* NULL input or OOM */
    s.nJson = sqlite3_કિંમત_bytes(argv[0]);
    જો( jsonConvertTextToBlob(&s,0) ){
      જો( s.oom ){
        iErrPos = -1;
      }નહીંતર{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        પ્રતિજ્ઞા( s.zJson!=0 );  /* Because s.oom is false */
        માટે(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          જો( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  જો( iErrPos<0 ){
    sqlite3_result_error_nomem(ctx);
  }નહીંતર{
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
સ્થિર શૂન્ય jsonArrayStep(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, કદ(*pStr));
  જો( pStr ){
    જો( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }નહીંતર જો( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_કિંમત(pStr, argv[0]);
  }
}
સ્થિર શૂન્ય jsonArrayCompute(sqlite3_context *ctx, પૂર્ણાંક isFinal){
  JsonString *pStr;
  પૂર્ણાંક flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  જો( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    જો( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      પાછા;
    }નહીંતર જો( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      જો( isFinal ){
        જો( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }નહીંતર{
        jsonStringTrimOneChar(pStr);
      }
      પાછા;
    }નહીંતર જો( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (પૂર્ણાંક)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }નહીંતર{
      sqlite3_result_text(ctx, pStr->zBuf, (પૂર્ણાંક)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }નહીંતર જો( flags & JSON_BLOB ){
    સ્થિર અચલ u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }નહીંતર{
    sqlite3_result_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
સ્થિર શૂન્ય json_Array_કિંમત(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
સ્થિર શૂન્ય jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
સ્થિર શૂન્ય jsonGroupInverse(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  unsigned પૂર્ણાંક i;
  પૂર્ણાંક inStr = 0;
  પૂર્ણાંક nNest = 0;
  અક્ષર *z;
  અક્ષર c;
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  જો( NEVER(!pStr) ) પાછા;
  z = pStr->zBuf;
  માટે(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    જો( c=='"' ){
      inStr = !inStr;
    }નહીંતર જો( c=='\\' ){
      i++;
    }નહીંતર જો( !inStr ){
      જો( c=='{' || c=='[' ) nNest++;
      જો( c=='}' || c==']' ) nNest--;
    }
  }
  જો( i<pStr->nUsed ){
    pStr->nUsed -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }નહીંતર{
    pStr->nUsed = 1;
  }
}
#નહીંતર
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
સ્થિર શૂન્ય jsonObjectStep(
  sqlite3_context *ctx,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  JsonString *pStr;
  અચલ અક્ષર *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, કદ(*pStr));
  જો( pStr ){
    z = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    n = sqlite3Strlen30(z);
    જો( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }નહીંતર જો( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    જો( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_કિંમત(pStr, argv[1]);
    }નહીંતર{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
સ્થિર શૂન્ય jsonObjectCompute(sqlite3_context *ctx, પૂર્ણાંક isFinal){
  JsonString *pStr;
  પૂર્ણાંક flags = SQLITE_PTR_TO_INT(sqlite3_user_data(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  જો( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    જો( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      પાછા;
    }
    જો( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      પૂર્ણાંક inStr = 0;
      જો( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        જો( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          પાછા;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      માટે(i=j=1; i<pStr->nUsed; i++){
        અક્ષર c = pStr->zBuf[i];
        જો( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }નહીંતર જો( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }નહીંતર જો( c=='@' && !inStr ){
          પ્રતિજ્ઞા( i+1<pStr->nUsed );
          જો( pStr->zBuf[i+1]==',' ){
            i++;
          }નહીંતર જો( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }નહીંતર{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    જો( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      જો( isFinal ){
        જો( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }નહીંતર{
        jsonStringTrimOneChar(pOgStr);
      }
    }નહીંતર જો( isFinal ){
      sqlite3_result_text(ctx, pStr->zBuf, (પૂર્ણાંક)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }નહીંતર{
      sqlite3_result_text(ctx, pStr->zBuf, (પૂર્ણાંક)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    જો( pStr!=pOgStr ) jsonStringReset(pStr);
  }નહીંતર જો( flags & JSON_BLOB ){
    સ્થિર અચલ unsigned અક્ષર emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }નહીંતર{
    sqlite3_result_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_result_subtype(ctx, JSON_SUBTYPE);
}
સ્થિર શૂન્ય json_Object_કિંમત(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
સ્થિર શૂન્ય jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ifndef SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
પ્રકારવ્યાખ્યા સંરચના JsonParent JsonParent;
સંરચના JsonParent {
  u32 iHead;                 /* Start of object or array */
  u32 i_કિંમત;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_ચાવી;                  /* Key for JSONB_ARRAY */
};

પ્રકારવ્યાખ્યા સંરચના JsonEachCursor JsonEachCursor;
સંરચના JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_મૂળ;                 /* Size of the root path in bytes */
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
પ્રકારવ્યાખ્યા સંરચના JsonEachConnection JsonEachConnection;
સંરચના JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
સ્થિર પૂર્ણાંક jsonEachConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  JsonEachConnection *pNew;
  પૂર્ણાંક rc;

/* Column numbers */
#define JEACH_ચાવી     0
#define JEACH_કિંમત   1
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
#define JEACH_મૂળ    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  જો( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, કદ(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    જો( pNew==0 ) પાછા SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  પાછા rc;
}

/* destructor for json_each virtual table */
સ્થિર પૂર્ણાંક jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  પાછા SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
સ્થિર પૂર્ણાંક jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, કદ(*pCur));
  જો( pCur==0 ) પાછા SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  પાછા SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
સ્થિર શૂન્ય jsonEachCursorReset(JsonEachCursor *p){
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
સ્થિર પૂર્ણાંક jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  પાછા SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
સ્થિર પૂર્ણાંક jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  પાછા p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
સ્થિર પૂર્ણાંક jsonSkipLabel(JsonEachCursor *p){
  જો( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    જો( sz >= p->sParse.nBlob ) sz = p->i;
    પાછા sz;
  }નહીંતર{
    પાછા p->i;
  }
}

/*
** Append the path name for the current element.
*/
સ્થિર શૂન્ય jsonAppendPathName(JsonEachCursor *p){
  પ્રતિજ્ઞા( p->nParent>0 );
  પ્રતિજ્ઞા( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  જો( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_ચાવી);
  }નહીંતર{
    u32 n, sz = 0, k, i;
    અચલ અક્ષર *z;
    પૂર્ણાંક needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (અચલ અક્ષર*)&p->sParse.aBlob[k];
    જો( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }નહીંતર{
      માટે(i=0; i<sz; i++){
        જો( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          તોડો;
        }
      }
    }
    જો( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }નહીંતર{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
સ્થિર પૂર્ણાંક json_Each_Malformed_આગત(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_સંદેશ);
  cur->pVtab->z_Err_સંદેશ = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  પાછા cur->pVtab->z_Err_સંદેશ ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
સ્થિર પૂર્ણાંક jsonEachNext(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  પૂર્ણાંક rc = SQLITE_OK;
  જો( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    જો( n==0 )પાછા json_Each_Malformed_આગત(cur);
    જો( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      જો( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, કદ(JsonParent)*nNew);
        જો( pNew==0 ) પાછા SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->i_કિંમત = i;
      pParent->iEnd = i + n + sz;
      pParent->i_ચાવી = -1;
      pParent->nPath = (u32)p->path.nUsed;
      જો( p->eType && p->nParent ){
        jsonAppendPathName(p);
        જો( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }નહીંતર{
      p->i = i + n + sz;
    }
    જ્યારે( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    જો( levelChange ){
      જો( p->nParent>0 ){
        JsonParent *pParent = &p->aParent[p->nParent-1];
        u32 iVal = pParent->i_કિંમત;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }નહીંતર{
        p->eType = 0;
      }
    }
  }નહીંતર{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    જો( n==0 )પાછા json_Each_Malformed_આગત(cur);
    p->i = i + n + sz;
  }
  જો( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_ચાવી++;
  }
  p->iRowid++;
  પાછા rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
સ્થિર પૂર્ણાંક jsonEachPathLength(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  અક્ષર *z = p->path.zBuf;
  જો( p->iRowid==0 && p->bRecursive && n>=2 ){
    જ્યારે( ALWAYS(n>1) ){
      n--;
      જો( z[n]=='[' || z[n]=='.' ){
        u32 x;
        અક્ષર cSaved = z[n];
        z[n] = 0;
        પ્રતિજ્ઞા( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        જો( !JSON_LOOKUP_ISERROR(x) ) તોડો;
      }
    }
  }
  પાછા n;
}

/* Return the value of a column */
સ્થિર પૂર્ણાંક jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  પૂર્ણાંક iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  પસંદગી( iColumn ){
    સ્થિતિ JEACH_ચાવી: {
      જો( p->nParent==0 ){
        u32 n, j;
        જો( p->n_મૂળ==1 ) તોડો;
        j = jsonEachPathLength(p);
        n = p->n_મૂળ - j;
        જો( n==0 ){
          તોડો;
        }નહીંતર જો( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        }નહીંતર જો( p->path.zBuf[j+1]=='"' ){
          sqlite3_result_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }નહીંતર{
          sqlite3_result_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        તોડો;
      }
      જો( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }નહીંતર{
        પ્રતિજ્ઞા( p->eType==JSONB_ARRAY );
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].i_ચાવી);
      }
      તોડો;
    }
    સ્થિતિ JEACH_કિંમત: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      જો( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_result_subtype(ctx, JSON_SUBTYPE);
      }
      તોડો;
    }
    સ્થિતિ JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      તોડો;
    }
    સ્થિતિ JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      જો( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      તોડો;
    }
    સ્થિતિ JEACH_ID: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      તોડો;
    }
    સ્થિતિ JEACH_PARENT: {
      જો( p->nParent>0 && p->bRecursive ){
        sqlite3_result_int64(ctx, p->aParent[p->nParent-1].iHead);
      }
      તોડો;
    }
    સ્થિતિ JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      જો( p->nParent ) jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      તોડો;
    }
    સ્થિતિ JEACH_PATH: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      તોડો;
    }
    મૂળભૂત: {
      sqlite3_result_text(ctx, p->path.zBuf, p->n_મૂળ, SQLITE_TRANSIENT);
      તોડો;
    }
    સ્થિતિ JEACH_JSON: {
      જો( p->sParse.zJson==0 ){
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }નહીંતર{
        sqlite3_result_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      તોડો;
    }
  }
  પાછા SQLITE_OK;
}

/* Return the current rowid value */
સ્થિર પૂર્ણાંક jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  પાછા SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
સ્થિર પૂર્ણાંક jsonEachBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  પૂર્ણાંક i;                     /* Loop counter or computed array index */
  પૂર્ણાંક aIdx[2];               /* Index of constraints for JSON and ROOT */
  પૂર્ણાંક unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  પૂર્ણાંક idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  અચલ સંરચના sqlite3_index_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  પ્રતિજ્ઞા( JEACH_મૂળ == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  માટે(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    પૂર્ણાંક iCol;
    પૂર્ણાંક iMask;
    જો( pConstraint->iColumn < JEACH_JSON ) ચાલુ;
    iCol = pConstraint->iColumn - JEACH_JSON;
    પ્રતિજ્ઞા( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    જો( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }નહીંતર જો( pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  જો( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  જો( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    પાછા SQLITE_CONSTRAINT;
  }
  જો( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_અંક = 0;
  }નહીંતર{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    જો( aIdx[1]<0 ){
      pIdxInfo->idx_અંક = 1;  /* Only JSON supplied.  Plan 1 */
    }નહીંતર{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_અંક = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  પાછા SQLITE_OK;
}

/* Start a search on a new JSON string */
સ્થિર પૂર્ણાંક jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  અચલ અક્ષર *z_મૂળ = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  જો( idx_અંક==0 ) પાછા SQLITE_OK;
  memset(&p->sParse, 0, કદ(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  જો( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }નહીંતર{
    p->sParse.zJson = (અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    p->sParse.nJson = sqlite3_કિંમત_bytes(argv[0]);
    જો( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      પાછા SQLITE_OK;
    }      
    જો( jsonConvertTextToBlob(&p->sParse, 0) ){
      જો( p->sParse.oom ){
        પાછા SQLITE_NOMEM;
      }
      પાછા json_Each_Malformed_આગત(cur);
    }
  }
  જો( idx_અંક==3 ){
    z_મૂળ = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]);
    જો( z_મૂળ==0 ) પાછા SQLITE_OK;
    જો( z_મૂળ[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_સંદેશ);
      cur->pVtab->z_Err_સંદેશ = jsonBadPathError(0, z_મૂળ, 0);
      jsonEachCursorReset(p);
      પાછા cur->pVtab->z_Err_સંદેશ ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_મૂળ = sqlite3Strlen30(z_મૂળ);
    જો( z_મૂળ[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }નહીંતર{
      i = jsonLookupStep(&p->sParse, 0, z_મૂળ+1, 0);
      જો( JSON_LOOKUP_ISERROR(i) ){
        જો( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          પાછા SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_સંદેશ);
        cur->pVtab->z_Err_સંદેશ = jsonBadPathError(0, z_મૂળ, 0);
        jsonEachCursorReset(p);
        પાછા cur->pVtab->z_Err_સંદેશ ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      જો( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }નહીંતર{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_મૂળ, p->n_મૂળ);
  }નહીંતર{
    i = p->i = 0;
    p->eType = 0;
    p->n_મૂળ = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  જો( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, કદ(JsonParent));
    જો( p->aParent==0 ) પાછા SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_ચાવી = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].i_કિંમત = i;
  }
  પાછા SQLITE_OK;
}

/* The methods of the json_each virtual table */
સ્થિર sqlite3_module jsonEachModule = {
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
શૂન્ય sqlite3RegisterJsonFunctions(શૂન્ય){
#ifndef SQLITE_OMIT_JSON
  સ્થિર FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_ઉમેરો, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_ઉમેરો,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_length,  1,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_array_length,  2,1,0, 0,0,0,          jsonArrayLengthFunc),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_ઉમેરો,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_ઉમેરો,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#જો SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#endif
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_કિંમત, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_કિંમત, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_કિંમત, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_કિંમત, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_RESULT_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_ઉમેરો_Builtin_Funcs(aJsonFunc, ArraySize(aJsonFunc));
#endif
}

#જો  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, અચલ અક્ષર *zName){
  unsigned પૂર્ણાંક i;
  સ્થિર અચલ અક્ષર *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  પ્રતિજ્ઞા( sqlite3HashFind(&db->aModule, zName)==0 );
  માટે(i=0; i<કદ(azModule)/કદ(azModule[0]); i++){
    જો( sqlite3StrICmp(azModule[i],zName)==0 ){
      પાછા sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  પાછા 0;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
