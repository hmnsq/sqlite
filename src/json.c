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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_JSON
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/* JSONB element types
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_NULL     0   /* "null" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_TRUE     1   /* "true" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_FALSE    2   /* "false" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_INT      3   /* integer acceptable to JSON and SQL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_INT5     4   /* integer in 0x000 notation */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_FLOAT    5   /* float acceptable to JSON and SQL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_FLOAT5   6   /* float with JSON5 extensions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_ਪਾਠ     7   /* Text compatible with both JSON and SQL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_TEXTJ    8   /* Text with JSON escapes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_TEXT5    9   /* Text with JSON-5 escape */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_TEXTRAW 10   /* SQL text that needs escaping for JSON */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_ARRAY   11   /* An array */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSONB_OBJECT  12   /* An object */

/* Human-readable names for the JSONB values.  The index for each
** string must correspond to the JSONB_* integer above.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ * ਅਚਲ jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ jsonIsSpace[] = {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
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
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
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
#ਜੇ_ਅੰਤ

};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ jsonIsspace(x) (jsonIsSpace[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
ਸਥਿਰ ਅਚਲ ਅੱਖਰ jsonSpaces[] = "\011\012\015\040";
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
ਸਥਿਰ ਅਚਲ ਅੱਖਰ jsonSpaces[] = "\005\045\015\100";
#ਜੇ_ਅੰਤ


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ jsonIsOk[256] = {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
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
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
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
#ਜੇ_ਅੰਤ
};

/* Objects */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ JsonCache JsonCache;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ JsonString JsonString;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_CACHE_ID    (-429938)  /* Cache entry */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_CACHE_ਮਾਪ  4          /* Max number of cache entries */

/*
** jsonUnescapeOneChar() returns this invalid code point if it encounters
** a syntax error.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_INVALID_CHAR 0x99999

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
ਬਣਤਰ JsonCache {
  sqlite3 *db;                    /* Database connection */
  ਪੂਰਨ n_ਵਰਤਿਆ;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_ਮਾਪ];  /* One line for each cache entry */
};

/* An instance of this object represents a JSON string
** under construction.  Really, this is a generic string accumulator
** that can be and is used to create strings other than JSON.
**
** If the generated string is longer than will fit into the zSpace[] buffer,
** then it will be an RCStr string.  This aids with caching of large
** JSON strings.
*/
ਬਣਤਰ JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  ਅੱਖਰ *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 n_ਵਰਤਿਆ;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  ਅੱਖਰ zSpace[100];        /* Initial static space */
};

/* Allowed values for JsonString.eErr */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSTRING_OOM         0x01   /* Out of memory */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSTRING_MALFORMED   0x02   /* Malformed JSONB */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSTRING_TOODEEP     0x04   /* JSON nested too deep */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSTRING_ERR         0x08   /* Error already sent to sqlite3_result */

/* The "subtype" set for text JSON values passed through using
** sqlite3_result_subtype() and sqlite3_value_subtype().
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_SUBTYPE  74    /* Ascii for "J" */

/*
** Bit values for the flags passed into various SQL function implementations
** via the sqlite3_user_data() value.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_JSON      0x01        /* Result is always JSON */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_SQL       0x02        /* Result is always SQL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_ABPATH    0x03        /* Allow abbreviated JSON path specs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_ISSET     0x04        /* json_set(), not json_insert() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_AINS      0x08        /* json_array_insert(), not json_insert() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_BLOB      0x10        /* Use the BLOB output format */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_ਪਾਓ_TYPE(X)  (((X)&0xC)>>2)


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
ਬਣਤਰ JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  ਅੱਖਰ *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  ਪੂਰਨ nJson;         /* Length of the zJson string in bytes */
  u32 nJPRef;        /* Number of references to this object */
  u32 iErr;          /* Error location in zJson[] */
  u16 i_ਡੂੰਘਾਈ;        /* Nesting depth */
  u8 nErr;           /* Number of errors seen */
  u8 oom;            /* Set to true if out of memory */
  u8 bJsonIsRCStr;   /* True if zJson is an RCStr */
  u8 hasNonstd;      /* True if input uses non-standard features like JSON5 */
  u8 bReadOnly;      /* Do not modify. */
  /* Search and edit information.  See jsonLookupStep() */
  u8 eEdit;          /* Edit operation to apply */
  ਪੂਰਨ delta;         /* Size change due to the edit */
  u32 nIns;          /* Number of bytes to insert */
  u32 iLabel;        /* Location of label if search landed on an object value */
  u8 *aIns;          /* Content to be inserted */
};

/* Allowed values for JsonParse.eEdit */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEDIT_DEL   1   /* Delete if exists */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEDIT_REPL  2   /* Overwrite if exists */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEDIT_INS   3   /* Insert if not exists */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEDIT_ਸੈੱਟ   4   /* Insert or overwrite */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEDIT_AINS  5   /* array_insert() */

/*
** Maximum nesting depth of JSON for this implementation.
**
** This limit is needed to avoid a stack overflow in the recursive
** descent parser.  A depth of 1000 is far deeper than any sane JSON
** should go.  Historical note: This limit was 2000 prior to version 3.42.0
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_JSON_MAX_ਡੂੰਘਾਈ
# define JSON_MAX_ਡੂੰਘਾਈ  1000
#ਨਹੀਂਤਾਂ
# define JSON_MAX_ਡੂੰਘਾਈ SQLITE_JSON_MAX_ਡੂੰਘਾਈ
#ਜੇ_ਅੰਤ

/*
** Allowed values for the flgs argument to jsonParseFuncArg();
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_EDITABLE  0x01   /* Generate a writable JsonParse object */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_KEEPERROR 0x02   /* Return non-NULL even if there is an error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_NOCACHE   0x04   /* Do not use the JSON cache */

/**************************************************************************
** Forward references
**************************************************************************/
ਸਥਿਰ ਖਾਲੀ jsonReturnStringAsBlob(JsonString*);
ਸਥਿਰ ਪੂਰਨ jsonArgIsJsonb(sqlite3_ਮੁੱਲ *pJson, JsonParse *p);
ਸਥਿਰ u32 json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(JsonParse*,u32,JsonString*);
ਸਥਿਰ ਖਾਲੀ jsonReturnParse(sqlite3_context*,JsonParse*);
ਸਥਿਰ JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_ਮੁੱਲ*,u32);
ਸਥਿਰ ਖਾਲੀ jsonParseFree(JsonParse*);
ਸਥਿਰ u32 jsonb_Payload_ਮਾਪ(ਅਚਲ JsonParse*, u32, u32*);
ਸਥਿਰ u32 jsonUnescapeOneChar(ਅਚਲ ਅੱਖਰ*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
ਸਥਿਰ ਖਾਲੀ jsonCacheDelete(JsonCache *p){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<p->n_ਵਰਤਿਆ; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
ਸਥਿਰ ਖਾਲੀ jsonCacheDeleteGeneric(ਖਾਲੀ *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
ਸਥਿਰ ਪੂਰਨ json_Cache_ਪਾਓ(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->zJson!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->bJsonIsRCStr );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ਜੇ( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, ਆਕਾਰ(*p));
    ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM;
    p->db = db;
    sqlite3_ਸੈੱਟ_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM;
  }
  ਜੇ( p->n_ਵਰਤਿਆ >= JSON_CACHE_ਮਾਪ ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_ਮਾਪ-1)*ਆਕਾਰ(p->a[0]));
    p->n_ਵਰਤਿਆ = JSON_CACHE_ਮਾਪ-1;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->n_ਵਰਤਿਆ] = pParse;
  p->n_ਵਰਤਿਆ++;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ JsonParse *json_Cache_ਖੋਜ(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_ਮੁੱਲ *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  ਪੂਰਨ i;
  ਅਚਲ ਅੱਖਰ *zJson;
  ਪੂਰਨ nJson;

  ਜੇ( sqlite3_ਮੁੱਲ_type(pArg)!=SQLITE_ਪਾਠ ){
    ਵਾਪਸ 0;
  }
  zJson = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pArg);
  ਜੇ( zJson==0 ) ਵਾਪਸ 0;
  nJson = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ਜੇ( p==0 ){
    ਵਾਪਸ 0;
  }
  ਲਈ(i=0; i<p->n_ਵਰਤਿਆ; i++){
    ਜੇ( p->a[i]->zJson==zJson ) ਰੋਕੋ;
  }
  ਜੇ( i>=p->n_ਵਰਤਿਆ ){
    ਲਈ(i=0; i<p->n_ਵਰਤਿਆ; i++){
      ਜੇ( p->a[i]->nJson!=nJson ) ਜਾਰੀ;
      ਜੇ( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) ਰੋਕੋ;
    }
  }
  ਜੇ( i<p->n_ਵਰਤਿਆ ){
    ਜੇ( i<p->n_ਵਰਤਿਆ-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->n_ਵਰਤਿਆ-i-1)*ਆਕਾਰ(tmp));
      p->a[p->n_ਵਰਤਿਆ-1] = tmp;
      i = p->n_ਵਰਤਿਆ - 1;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p->a[i]->delta==0 );
    ਵਾਪਸ p->a[i];
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
ਸਥਿਰ ਖਾਲੀ jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = ਆਕਾਰ(p->zSpace);
  p->n_ਵਰਤਿਆ = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
ਸਥਿਰ ਖਾਲੀ jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
ਸਥਿਰ ਖਾਲੀ jsonStringReset(JsonString *p){
  ਜੇ( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
ਸਥਿਰ ਖਾਲੀ jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  ਜੇ( p->pCtx ) sqlite3_ਨਤੀਜਾ_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
ਸਥਿਰ ਖਾਲੀ jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pCtx!=0 );
  sqlite3_ਨਤੀਜਾ_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
ਸਥਿਰ ਪੂਰਨ jsonStringGrow(JsonString *p, u32 N){
  u64 n_ਕੁੱਲ = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  ਅੱਖਰ *zNew;
  ਜੇ( p->bStatic ){
    ਜੇ( p->eErr ) ਵਾਪਸ 1;
    zNew = sqlite3RCStrNew(n_ਕੁੱਲ);
    ਜੇ( zNew==0 ){
      jsonStringOom(p);
      ਵਾਪਸ SQLITE_NOMEM;
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zNew, p->zBuf, (size_t)p->n_ਵਰਤਿਆ);
    p->zBuf = zNew;
    p->bStatic = 0;
  }ਨਹੀਂਤਾਂ{
    p->zBuf = sqlite3RCStrResize(p->zBuf, n_ਕੁੱਲ);
    ਜੇ( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      ਵਾਪਸ SQLITE_NOMEM;
    }
  }
  p->nAlloc = n_ਕੁੱਲ;
  ਵਾਪਸ SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ jsonStringExpandAndAppend(
  JsonString *p,
  ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ,
  u32 N
){
  ਪ੍ਰਤਿਜ੍ਞਾ( N>0 );
  ਜੇ( jsonStringGrow(p,N) ) ਵਾਪਸ;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->zBuf+p->n_ਵਰਤਿਆ, z_ਵਿੱਚ, N);
  p->n_ਵਰਤਿਆ += N;
}
ਸਥਿਰ ਖਾਲੀ jsonAppendRaw(JsonString *p, ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ, u32 N){
  ਜੇ( N==0 ) ਵਾਪਸ;
  ਜੇ( N+p->n_ਵਰਤਿਆ >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_ਵਿੱਚ,N);
  }ਨਹੀਂਤਾਂ{
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->zBuf+p->n_ਵਰਤਿਆ, z_ਵਿੱਚ, N);
    p->n_ਵਰਤਿਆ += N;
  }
}
ਸਥਿਰ ਖਾਲੀ jsonAppendRawNZ(JsonString *p, ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ, u32 N){
  ਪ੍ਰਤਿਜ੍ਞਾ( N>0 );
  ਜੇ( N+p->n_ਵਰਤਿਆ >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_ਵਿੱਚ,N);
  }ਨਹੀਂਤਾਂ{
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->zBuf+p->n_ਵਰਤਿਆ, z_ਵਿੱਚ, N);
    p->n_ਵਰਤਿਆ += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
ਸਥਿਰ ਖਾਲੀ jsonPrintf(ਪੂਰਨ N, JsonString *p, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;
  ਜੇ( (p->n_ਵਰਤਿਆ + N >= p->nAlloc) && jsonStringGrow(p, N) ) ਵਾਪਸ;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->n_ਵਰਤਿਆ, zFormat, ap);
  va_end(ap);
  p->n_ਵਰਤਿਆ += (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(p->zBuf+p->n_ਵਰਤਿਆ);
}

/* Append a single character
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ jsonAppendCharExpand(JsonString *p, ਅੱਖਰ c){
  ਜੇ( jsonStringGrow(p,1) ) ਵਾਪਸ;
  p->zBuf[p->n_ਵਰਤਿਆ++] = c;
}
ਸਥਿਰ ਖਾਲੀ jsonAppendChar(JsonString *p, ਅੱਖਰ c){
  ਜੇ( p->n_ਵਰਤਿਆ>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }ਨਹੀਂਤਾਂ{
    p->zBuf[p->n_ਵਰਤਿਆ++] = c;
  }
}

/* Remove a single character from the end of the string
*/
ਸਥਿਰ ਖਾਲੀ jsonStringTrimOneChar(JsonString *p){
  ਜੇ( p->eErr==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਵਰਤਿਆ>0 );
    p->n_ਵਰਤਿਆ--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
ਸਥਿਰ ਪੂਰਨ jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  ਵਾਪਸ p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
ਸਥਿਰ ਖਾਲੀ jsonAppendSeparator(JsonString *p){
  ਅੱਖਰ c;
  ਜੇ( p->n_ਵਰਤਿਆ==0 ) ਵਾਪਸ;
  c = p->zBuf[p->n_ਵਰਤਿਆ-1];
  ਜੇ( c=='[' || c=='{' ) ਵਾਪਸ;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
ਸਥਿਰ ਖਾਲੀ jsonAppendControlChar(JsonString *p, u8 c){
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(aSpecial)==32 );
  ਪ੍ਰਤਿਜ੍ਞਾ( aSpecial['\b']=='b' );
  ਪ੍ਰਤਿਜ੍ਞਾ( aSpecial['\f']=='f' );
  ਪ੍ਰਤਿਜ੍ਞਾ( aSpecial['\n']=='n' );
  ਪ੍ਰਤਿਜ੍ਞਾ( aSpecial['\r']=='r' );
  ਪ੍ਰਤਿਜ੍ਞਾ( aSpecial['\t']=='t' );
  ਪ੍ਰਤਿਜ੍ਞਾ( c>=0 && c<ਆਕਾਰ(aSpecial) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਵਰਤਿਆ+7 <= p->nAlloc );
  ਜੇ( aSpecial[c] ){
    p->zBuf[p->n_ਵਰਤਿਆ] = '\\';
    p->zBuf[p->n_ਵਰਤਿਆ+1] = aSpecial[c];
    p->n_ਵਰਤਿਆ += 2;
  }ਨਹੀਂਤਾਂ{
    p->zBuf[p->n_ਵਰਤਿਆ] = '\\';
    p->zBuf[p->n_ਵਰਤਿਆ+1] = 'u';
    p->zBuf[p->n_ਵਰਤਿਆ+2] = '0';
    p->zBuf[p->n_ਵਰਤਿਆ+3] = '0';
    p->zBuf[p->n_ਵਰਤਿਆ+4] = "0123456789abcdef"[c>>4];
    p->zBuf[p->n_ਵਰਤਿਆ+5] = "0123456789abcdef"[c&0xf];
    p->n_ਵਰਤਿਆ += 6;
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
ਸਥਿਰ ਖਾਲੀ jsonAppendString(JsonString *p, ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ, u32 N){
  u32 k;
  u8 c;
  ਅਚਲ u8 *z = (ਅਚਲ u8*)z_ਵਿੱਚ;
  ਜੇ( z==0 ) ਵਾਪਸ;
  ਜੇ( (N+p->n_ਵਰਤਿਆ+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) ਵਾਪਸ;
  p->zBuf[p->n_ਵਰਤਿਆ++] = '"';
  ਜਦੋਂ( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    ਜਦੋਂ( 1 /* Exit by break */ ){
      ਜੇ( k+3>=N ){
        ਜਦੋਂ( k<N && jsonIsOk[z[k]] ){ k++; }
        ਰੋਕੋ;
      }
      ਜੇ( !jsonIsOk[z[k]] ){
        ਰੋਕੋ;
      }
      ਜੇ( !jsonIsOk[z[k+1]] ){
        k += 1;
        ਰੋਕੋ;
      }
      ਜੇ( !jsonIsOk[z[k+2]] ){
        k += 2;
        ਰੋਕੋ;
      }
      ਜੇ( !jsonIsOk[z[k+3]] ){
        k += 3;
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ{
        k += 4;
      }
    }
    ਜੇ( k>=N ){
      ਜੇ( k>0 ){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->zBuf[p->n_ਵਰਤਿਆ], z, k);
        p->n_ਵਰਤਿਆ += k;
      }
      ਰੋਕੋ;
    }
    ਜੇ( k>0 ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->zBuf[p->n_ਵਰਤਿਆ], z, k);
      p->n_ਵਰਤਿਆ += k;
      z += k;
      N -= k;
    }
    c = z[0];
    ਜੇ( c=='"' || c=='\\' ){
      ਜੇ( (p->n_ਵਰਤਿਆ+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) ਵਾਪਸ;
      p->zBuf[p->n_ਵਰਤਿਆ++] = '\\';
      p->zBuf[p->n_ਵਰਤਿਆ++] = c;
    }ਨਹੀਂਤਾਂ ਜੇ( c=='\'' ){
      p->zBuf[p->n_ਵਰਤਿਆ++] = c;
    }ਨਹੀਂਤਾਂ{
      ਜੇ( (p->n_ਵਰਤਿਆ+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) ਵਾਪਸ;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->n_ਵਰਤਿਆ++] = '"';
  ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਵਰਤਿਆ<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
ਸਥਿਰ ਖਾਲੀ json_Append_Sql_ਮੁੱਲ(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_ਮੁੱਲ *p_ਮੁੱਲ          /* Value to append */
){
  ਚੋਣ( sqlite3_ਮੁੱਲ_type(p_ਮੁੱਲ) ){
    ਹਾਲਤ SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_ਮੁੱਲ_double(p_ਮੁੱਲ));
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_INTEGER: {
      ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(p_ਮੁੱਲ);
      u32 n = (u32)sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(p_ਮੁੱਲ);
      jsonAppendRaw(p, z, n);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_ਪਾਠ: {
      ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(p_ਮੁੱਲ);
      u32 n = (u32)sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(p_ਮੁੱਲ);
      ਜੇ( sqlite3_ਮੁੱਲ_subtype(p_ਮੁੱਲ)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }ਨਹੀਂਤਾਂ{
        jsonAppendString(p, z, n);
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      JsonParse px;
      memset(&px, 0, ਆਕਾਰ(px));
      ਜੇ( jsonArgIsJsonb(p_ਮੁੱਲ, &px) ){
        json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(&px, 0, p);
      }ਨਹੀਂਤਾਂ ਜੇ( p->eErr==0 ){
        sqlite3_ਨਤੀਜਾ_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      ਰੋਕੋ;
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
ਸਥਿਰ ਖਾਲੀ jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( (pParse!=0)==(ctx!=0) );
  ਪ੍ਰਤਿਜ੍ਞਾ( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  ਜੇ( p->eErr==0 ){
    ਪੂਰਨ flags = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(p->pCtx));
    ਜੇ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }ਨਹੀਂਤਾਂ ਜੇ( p->bStatic ){
      sqlite3_ਨਤੀਜਾ_text64(p->pCtx, p->zBuf, p->n_ਵਰਤਿਆ,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }ਨਹੀਂਤਾਂ{
      ਜੇ( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        ਪੂਰਨ rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->n_ਵਰਤਿਆ;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_ਪਾਓ(ctx, pParse);
        ਜੇ( rc==SQLITE_NOMEM ){
          sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
          jsonStringReset(p);
          ਵਾਪਸ;
        }
      }
      sqlite3_ਨਤੀਜਾ_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->n_ਵਰਤਿਆ,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p->eErr & JSTRING_OOM ){
    sqlite3_ਨਤੀਜਾ_error_nomem(p->pCtx);
  }ਨਹੀਂਤਾਂ ਜੇ( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }ਨਹੀਂਤਾਂ ਜੇ( p->eErr & JSTRING_MALFORMED ){
    sqlite3_ਨਤੀਜਾ_error(p->pCtx, "malformed JSON", -1);
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
ਸਥਿਰ ਖਾਲੀ jsonParseReset(JsonParse *pParse){
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nJPRef<=1 );
  ਜੇ( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  ਜੇ( pParse->nBlobAlloc ){
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
ਸਥਿਰ ਖਾਲੀ jsonParseFree(JsonParse *pParse){
  ਜੇ( pParse ){
    ਜੇ( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }ਨਹੀਂਤਾਂ{
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
ਸਥਿਰ u8 json_Hex_ਮੰਜ਼ਿਲ_Int(ਪੂਰਨ h){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
  h += 9*(1&(h>>6));
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#ਜੇ_ਅੰਤ
  ਵਾਪਸ (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
ਸਥਿਰ u32 json_Hex_ਮੰਜ਼ਿਲ_Int4(ਅਚਲ ਅੱਖਰ *z){
  u32 v;
  v = (json_Hex_ਮੰਜ਼ਿਲ_Int(z[0])<<12)
    + (json_Hex_ਮੰਜ਼ਿਲ_Int(z[1])<<8)
    + (json_Hex_ਮੰਜ਼ਿਲ_Int(z[2])<<4)
    + json_Hex_ਮੰਜ਼ਿਲ_Int(z[3]);
  ਵਾਪਸ v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
ਸਥਿਰ ਪੂਰਨ jsonIs2Hex(ਅਚਲ ਅੱਖਰ *z){
  ਵਾਪਸ sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
ਸਥਿਰ ਪੂਰਨ jsonIs4Hex(ਅਚਲ ਅੱਖਰ *z){
  ਵਾਪਸ jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
ਸਥਿਰ ਪੂਰਨ json5Whitespace(ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ){
  ਪੂਰਨ n = 0;
  ਅਚਲ u8 *z = (u8*)z_ਵਿੱਚ;
  ਜਦੋਂ( 1 /*exit by "goto whitespace_done"*/ ){
    ਚੋਣ( z[n] ){
      ਹਾਲਤ 0x09:
      ਹਾਲਤ 0x0a:
      ਹਾਲਤ 0x0b:
      ਹਾਲਤ 0x0c:
      ਹਾਲਤ 0x0d:
      ਹਾਲਤ 0x20: {
        n++;
        ਰੋਕੋ;
      }
      ਹਾਲਤ '/': {
        ਜੇ( z[n+1]=='*' && z[n+2]!=0 ){
          ਪੂਰਨ j;
          ਲਈ(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            ਜੇ( z[j]==0 ) ਜਾਓ whitespace_ਪੂਰਾ;
          }
          n = j+1;
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ ਜੇ( z[n+1]=='/' ){
          ਪੂਰਨ j;
          ਅੱਖਰ c;
          ਲਈ(j=n+2; (c = z[j])!=0; j++){
            ਜੇ( c=='\n' || c=='\r' ) ਰੋਕੋ;
            ਜੇ( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              ਰੋਕੋ;
            }
          }
          n = j;
          ਜੇ( z[n] ) n++;
          ਰੋਕੋ;
        }
        ਜਾਓ whitespace_ਪੂਰਾ;
      }
      ਹਾਲਤ 0xc2: {
        ਜੇ( z[n+1]==0xa0 ){
          n += 2;
          ਰੋਕੋ;
        }
        ਜਾਓ whitespace_ਪੂਰਾ;
      }
      ਹਾਲਤ 0xe1: {
        ਜੇ( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          ਰੋਕੋ;
        }
        ਜਾਓ whitespace_ਪੂਰਾ;
      }
      ਹਾਲਤ 0xe2: {
        ਜੇ( z[n+1]==0x80 ){
          u8 c = z[n+2];
          ਜੇ( c<0x80 ) ਜਾਓ whitespace_ਪੂਰਾ;
          ਜੇ( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            ਰੋਕੋ;
          }
        }ਨਹੀਂਤਾਂ ਜੇ( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          ਰੋਕੋ;
        }
        ਜਾਓ whitespace_ਪੂਰਾ;
      }
      ਹਾਲਤ 0xe3: {
        ਜੇ( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          ਰੋਕੋ;
        }
        ਜਾਓ whitespace_ਪੂਰਾ;
      }
      ਹਾਲਤ 0xef: {
        ਜੇ( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          ਰੋਕੋ;
        }
        ਜਾਓ whitespace_ਪੂਰਾ;
      }
      ਮੂਲ: {
        ਜਾਓ whitespace_ਪੂਰਾ;
      }
    }
  }
  whitespace_ਪੂਰਾ:
  ਵਾਪਸ n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
ਸਥਿਰ ਅਚਲ ਬਣਤਰ NanInfName {
  ਅੱਖਰ c1;
  ਅੱਖਰ c2;
  ਅੱਖਰ n;
  ਅੱਖਰ eType;
  ਅੱਖਰ nRepl;
  ਅੱਖਰ *z_ਮੇਲ;
  ਅੱਖਰ *zRepl;
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
ਸਥਿਰ ਖਾਲੀ json_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(
  sqlite3_context *pCtx,
  ਅਚਲ ਅੱਖਰ *zFuncName
){
  ਅੱਖਰ *z_ਸੁਨੇਹਾ = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_ਨਤੀਜਾ_error(pCtx, z_ਸੁਨੇਹਾ, -1);
  sqlite3_free(z_ਸੁਨੇਹਾ);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
ਸਥਿਰ ਪੂਰਨ jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  ਪ੍ਰਤਿਜ੍ਞਾ( N>pParse->nBlobAlloc );
  ਜੇ( pParse->nBlobAlloc==0 ){
    t = 100;
  }ਨਹੀਂਤਾਂ{
    t = pParse->nBlobAlloc*2;
  }
  ਜੇ( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  ਜੇ( aNew==0 ){ pParse->oom = 1; ਵਾਪਸ 1; }
  ਪ੍ਰਤਿਜ੍ਞਾ( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  ਵਾਪਸ 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
ਸਥਿਰ ਪੂਰਨ jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 n_ਮਾਪ;
  ਪ੍ਰਤਿਜ੍ਞਾ( !pParse->bReadOnly );
  ਜੇ( pParse->oom ) ਵਾਪਸ 0;
  ਜੇ( pParse->nBlobAlloc>0 ) ਵਾਪਸ 1;
  aOld = pParse->aBlob;
  n_ਮਾਪ = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  ਜੇ( jsonBlobExpand(pParse, n_ਮਾਪ) ){
    ਵਾਪਸ 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pParse->aBlob, aOld, pParse->nBlob);
  ਵਾਪਸ 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  ਜੇ( pParse->oom==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
ਸਥਿਰ ਖਾਲੀ jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  ਜੇ( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }ਨਹੀਂਤਾਂ{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
ਸਥਿਰ ਖਾਲੀ json_Blob_Append_ਗੰਢ(JsonParse*,u8,u64,ਅਚਲ ਖਾਲੀ*);
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ json_Blob_Expand_And_Append_ਗੰਢ(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ਅਚਲ ਖਾਲੀ *aPayload
){
  ਜੇ( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) ਵਾਪਸ;
  json_Blob_Append_ਗੰਢ(pParse, eType, szPayload, aPayload);
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
ਸਥਿਰ ਖਾਲੀ json_Blob_Append_ਗੰਢ(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  ਅਚਲ ਖਾਲੀ *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  ਜੇ( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_ਗੰਢ(pParse,eType,szPayload,aPayload);
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  ਜੇ( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }ਨਹੀਂਤਾਂ ਜੇ( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }ਨਹੀਂਤਾਂ ਜੇ( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }ਨਹੀਂਤਾਂ{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  ਜੇ( aPayload ){
    pParse->nBlob += szPayload;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
ਸਥਿਰ ਪੂਰਨ json_Blob_Change_Payload_ਮਾਪ(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  ਪੂਰਨ delta;
  ਜੇ( pParse->oom ) ਵਾਪਸ 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  ਜੇ( szType<=11 ){
    nExtra = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( szType==12 ){
    nExtra = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( szType==13 ){
    nExtra = 2;
  }ਨਹੀਂਤਾਂ ਜੇ( szType==14 ){
    nExtra = 4;
  }ਨਹੀਂਤਾਂ{
    nExtra = 8;
  }
  ਜੇ( szPayload<=11 ){
    nNeeded = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( szPayload<=0xff ){
    nNeeded = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( szPayload<=0xffff ){
    nNeeded = 2;
  }ਨਹੀਂਤਾਂ{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  ਜੇ( delta ){
    u64 new_ਮਾਪ_2 = (u64)pParse->nBlob + delta;
    ਜੇ( delta>0 ){
      ਜੇ( new_ਮਾਪ_2>pParse->nBlobAlloc && jsonBlobExpand(pParse, new_ਮਾਪ_2) ){
        ਵਾਪਸ 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }ਨਹੀਂਤਾਂ{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = new_ਮਾਪ_2;
  }
  ਜੇ( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }ਨਹੀਂਤਾਂ ਜੇ( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }ਨਹੀਂਤਾਂ ਜੇ( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }ਨਹੀਂਤਾਂ{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  ਵਾਪਸ delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
ਸਥਿਰ ਪੂਰਨ jsonIs4HexB(ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ *pOp){
  ਜੇ( z[0]!='u' ) ਵਾਪਸ 0;
  ਜੇ( !jsonIs4Hex(&z[1]) ) ਵਾਪਸ 0;
  *pOp = JSONB_TEXTJ;
  ਵਾਪਸ 1;
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
ਸਥਿਰ u32 jsonbValidityCheck(
  ਅਚਲ JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 i_ਸਿਰਾ,                   /* One more than the last byte of the element */
  u32 i_ਡੂੰਘਾਈ                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ਅਚਲ u8 *z;
  u8 x;
  ਜੇ( i_ਡੂੰਘਾਈ>JSON_MAX_ਡੂੰਘਾਈ ) ਵਾਪਸ i+1;
  sz = 0;
  n = jsonb_Payload_ਮਾਪ(pParse, i, &sz);
  ਜੇ( NEVER(n==0) ) ਵਾਪਸ i+1;          /* Checked by caller */
  ਜੇ( NEVER(i+n+sz!=i_ਸਿਰਾ) ) ਵਾਪਸ i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  ਚੋਣ( x ){
    ਹਾਲਤ JSONB_NULL:
    ਹਾਲਤ JSONB_TRUE:
    ਹਾਲਤ JSONB_FALSE: {
      ਵਾਪਸ n+sz==1 ? 0 : i+1;
    }
    ਹਾਲਤ JSONB_INT: {
      ਜੇ( sz<1 ) ਵਾਪਸ i+1;
      j = i+n;
      ਜੇ( z[j]=='-' ){
        j++;
        ਜੇ( sz<2 ) ਵਾਪਸ i+1;
      }
      k = i+n+sz;
      ਜਦੋਂ( j<k ){
        ਜੇ( sqlite3Isdigit(z[j]) ){
          j++;
        }ਨਹੀਂਤਾਂ{
          ਵਾਪਸ j+1;
        }
      }
      ਵਾਪਸ 0;
    }
    ਹਾਲਤ JSONB_INT5: {
      ਜੇ( sz<3 ) ਵਾਪਸ i+1;
      j = i+n;
      ਜੇ( z[j]=='-' ){
        ਜੇ( sz<4 ) ਵਾਪਸ i+1;
        j++;
      }
      ਜੇ( z[j]!='0' ) ਵਾਪਸ i+1;
      ਜੇ( z[j+1]!='x' && z[j+1]!='X' ) ਵਾਪਸ j+2;
      j += 2;
      k = i+n+sz;
      ਜਦੋਂ( j<k ){
        ਜੇ( sqlite3Isxdigit(z[j]) ){
          j++;
        }ਨਹੀਂਤਾਂ{
          ਵਾਪਸ j+1;
        }
      }
      ਵਾਪਸ 0;
    }
    ਹਾਲਤ JSONB_FLOAT:
    ਹਾਲਤ JSONB_FLOAT5: {
      u8 ਵੇਖਿਆ = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      ਜੇ( sz<2 ) ਵਾਪਸ i+1;
      j = i+n;
      k = j+sz;
      ਜੇ( z[j]=='-' ){
        j++;
        ਜੇ( sz<3 ) ਵਾਪਸ i+1;
      }
      ਜੇ( z[j]=='.' ){
        ਜੇ( x==JSONB_FLOAT ) ਵਾਪਸ j+1;
        ਜੇ( !sqlite3Isdigit(z[j+1]) ) ਵਾਪਸ j+1;
        j += 2;
        ਵੇਖਿਆ = 1;
      }ਨਹੀਂਤਾਂ ਜੇ( z[j]=='0' && x==JSONB_FLOAT ){
        ਜੇ( j+3>k ) ਵਾਪਸ j+1;
        ਜੇ( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) ਵਾਪਸ j+1;
        j++;
      }
      ਲਈ(; j<k; j++){
        ਜੇ( sqlite3Isdigit(z[j]) ) ਜਾਰੀ;
        ਜੇ( z[j]=='.' ){
          ਜੇ( ਵੇਖਿਆ>0 ) ਵਾਪਸ j+1;
          ਜੇ( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            ਵਾਪਸ j+1;
          }
          ਵੇਖਿਆ = 1;
          ਜਾਰੀ;
        }
        ਜੇ( z[j]=='e' || z[j]=='E' ){
          ਜੇ( ਵੇਖਿਆ==2 ) ਵਾਪਸ j+1;
          ਜੇ( j==k-1 ) ਵਾਪਸ j+1;
          ਜੇ( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            ਜੇ( j==k-1 ) ਵਾਪਸ j+1;
          }
          ਵੇਖਿਆ = 2;
          ਜਾਰੀ;
        }
        ਵਾਪਸ j+1;
      }
      ਜੇ( ਵੇਖਿਆ==0 ) ਵਾਪਸ i+1;
      ਵਾਪਸ 0;
    }
    ਹਾਲਤ JSONB_ਪਾਠ: {
      j = i+n;
      k = j+sz;
      ਜਦੋਂ( j<k ){
        ਜੇ( !jsonIsOk[z[j]] && z[j]!='\'' ) ਵਾਪਸ j+1;
        j++;
      }
      ਵਾਪਸ 0;
    }
    ਹਾਲਤ JSONB_TEXTJ:
    ਹਾਲਤ JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      ਜਦੋਂ( j<k ){
        ਜੇ( !jsonIsOk[z[j]] && z[j]!='\'' ){
          ਜੇ( z[j]=='"' ){
            ਜੇ( x==JSONB_TEXTJ ) ਵਾਪਸ j+1;
          }ਨਹੀਂਤਾਂ ਜੇ( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            ਜੇ( x==JSONB_TEXTJ ) ਵਾਪਸ j+1;
          }ਨਹੀਂਤਾਂ ਜੇ( NEVER(z[j]!='\\') || j+1>=k ){
            ਵਾਪਸ j+1;
          }ਨਹੀਂਤਾਂ ਜੇ( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }ਨਹੀਂਤਾਂ ਜੇ( z[j+1]=='u' ){
            ਜੇ( j+5>=k ) ਵਾਪਸ j+1;
            ਜੇ( !jsonIs4Hex((ਅਚਲ ਅੱਖਰ*)&z[j+2]) ) ਵਾਪਸ j+1;
            j++;
          }ਨਹੀਂਤਾਂ ਜੇ( x!=JSONB_TEXT5 ){
            ਵਾਪਸ j+1;
          }ਨਹੀਂਤਾਂ{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((ਅਚਲ ਅੱਖਰ*)&z[j], k-j, &c);
            ਜੇ( c==JSON_INVALID_CHAR ) ਵਾਪਸ j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      ਵਾਪਸ 0;
    }
    ਹਾਲਤ JSONB_TEXTRAW: {
      ਵਾਪਸ 0;
    }
    ਹਾਲਤ JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      ਜਦੋਂ( j<k ){
        sz = 0;
        n = jsonb_Payload_ਮਾਪ(pParse, j, &sz);
        ਜੇ( n==0 ) ਵਾਪਸ j+1;
        ਜੇ( j+n+sz>k ) ਵਾਪਸ j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_ਡੂੰਘਾਈ+1);
        ਜੇ( sub ) ਵਾਪਸ sub;
        j += n + sz;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( j==k );
      ਵਾਪਸ 0;
    }
    ਹਾਲਤ JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      ਜਦੋਂ( j<k ){
        sz = 0;
        n = jsonb_Payload_ਮਾਪ(pParse, j, &sz);
        ਜੇ( n==0 ) ਵਾਪਸ j+1;
        ਜੇ( j+n+sz>k ) ਵਾਪਸ j+1;
        ਜੇ( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          ਜੇ( x<JSONB_ਪਾਠ || x>JSONB_TEXTRAW ) ਵਾਪਸ j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_ਡੂੰਘਾਈ+1);
        ਜੇ( sub ) ਵਾਪਸ sub;
        cnt++;
        j += n + sz;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( j==k );
      ਜੇ( (cnt & 1)!=0 ) ਵਾਪਸ j+1;
      ਵਾਪਸ 0;
    }
    ਮੂਲ: {
      ਵਾਪਸ i+1;
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
ਸਥਿਰ ਪੂਰਨ json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(JsonParse *pParse, u32 i){
  ਅੱਖਰ c;
  u32 j;
  u32 iThis, i_ਸ਼ੁਰੂ;
  ਪੂਰਨ x;
  u8 t;
  ਅਚਲ ਅੱਖਰ *z = pParse->zJson;
json_parse_restart:
  ਚੋਣ( (u8)z[i] ){
  ਹਾਲਤ '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_ਗੰਢ(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    ਜੇ( ++pParse->i_ਡੂੰਘਾਈ > JSON_MAX_ਡੂੰਘਾਈ ){
      pParse->iErr = i;
      ਵਾਪਸ -1;
    }
    i_ਸ਼ੁਰੂ = pParse->nBlob;
    ਲਈ(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(pParse, j);
      ਜੇ( x<=0 ){
        ਪੂਰਨ op;
        ਜੇ( x==(-2) ){
          j = pParse->iErr;
          ਜੇ( pParse->nBlob!=(u32)i_ਸ਼ੁਰੂ ) pParse->hasNonstd = 1;
          ਰੋਕੋ;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_ਪਾਠ;
        ਜੇ( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          ਪੂਰਨ k = j+1;
          ਜਦੋਂ( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          ਪ੍ਰਤਿਜ੍ਞਾ( iBlob==pParse->nBlob );
          json_Blob_Append_ਗੰਢ(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }ਨਹੀਂਤਾਂ{
          ਜੇ( x!=-1 ) pParse->iErr = j;
          ਵਾਪਸ -1;
        }
      }
      ਜੇ( pParse->oom ) ਵਾਪਸ -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      ਜੇ( t<JSONB_ਪਾਠ || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        ਵਾਪਸ -1;
      }
      j = x;
      ਜੇ( z[j]==':' ){
        j++;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          ਕਰੋ{ j++; }ਜਦੋਂ( jsonIsspace(z[j]) );
          ਜੇ( z[j]==':' ){
            j++;
            ਜਾਓ parse_object_ਮੁੱਲ;
          }
        }
        x = json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(pParse, j);
        ਜੇ( x!=(-5) ){
          ਜੇ( x!=(-1) ) pParse->iErr = j;
          ਵਾਪਸ -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_ਮੁੱਲ:
      x = json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(pParse, j);
      ਜੇ( x<=0 ){
        ਜੇ( x!=(-1) ) pParse->iErr = j;
        ਵਾਪਸ -1;
      }
      j = x;
      ਜੇ( z[j]==',' ){
        ਜਾਰੀ;
      }ਨਹੀਂਤਾਂ ਜੇ( z[j]=='}' ){
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ਜੇ( z[j]==',' ){
            ਜਾਰੀ;
          }ਨਹੀਂਤਾਂ ਜੇ( z[j]=='}' ){
            ਰੋਕੋ;
          }
        }
        x = json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(pParse, j);
        ਜੇ( x==(-4) ){
          j = pParse->iErr;
          ਜਾਰੀ;
        }
        ਜੇ( x==(-2) ){
          j = pParse->iErr;
          ਰੋਕੋ;
        }
      }
      pParse->iErr = j;
      ਵਾਪਸ -1;
    }
    json_Blob_Change_Payload_ਮਾਪ(pParse, iThis, pParse->nBlob - i_ਸ਼ੁਰੂ);
    pParse->i_ਡੂੰਘਾਈ--;
    ਵਾਪਸ j+1;
  }
  ਹਾਲਤ '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    ਪ੍ਰਤਿਜ੍ਞਾ( i<=(u32)pParse->nJson );
    json_Blob_Append_ਗੰਢ(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    i_ਸ਼ੁਰੂ = pParse->nBlob;
    ਜੇ( pParse->oom ) ਵਾਪਸ -1;
    ਜੇ( ++pParse->i_ਡੂੰਘਾਈ > JSON_MAX_ਡੂੰਘਾਈ ){
      pParse->iErr = i;
      ਵਾਪਸ -1;
    }
    ਲਈ(j=i+1;;j++){
      x = json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(pParse, j);
      ਜੇ( x<=0 ){
        ਜੇ( x==(-3) ){
          j = pParse->iErr;
          ਜੇ( pParse->nBlob!=i_ਸ਼ੁਰੂ ) pParse->hasNonstd = 1;
          ਰੋਕੋ;
        }
        ਜੇ( x!=(-1) ) pParse->iErr = j;
        ਵਾਪਸ -1;
      }
      j = x;
      ਜੇ( z[j]==',' ){
        ਜਾਰੀ;
      }ਨਹੀਂਤਾਂ ਜੇ( z[j]==']' ){
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ਜੇ( z[j]==',' ){
            ਜਾਰੀ;
          }ਨਹੀਂਤਾਂ ਜੇ( z[j]==']' ){
            ਰੋਕੋ;
          }
        }
        x = json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(pParse, j);
        ਜੇ( x==(-4) ){
          j = pParse->iErr;
          ਜਾਰੀ;
        }
        ਜੇ( x==(-3) ){
          j = pParse->iErr;
          ਰੋਕੋ;
        }
      }
      pParse->iErr = j;
      ਵਾਪਸ -1;
    }
    json_Blob_Change_Payload_ਮਾਪ(pParse, iThis, pParse->nBlob - i_ਸ਼ੁਰੂ);
    pParse->i_ਡੂੰਘਾਈ--;
    ਵਾਪਸ j+1;
  }
  ਹਾਲਤ '\'': {
    u8 opcode;
    ਅੱਖਰ cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_ਪਾਠ;
    ਜਾਓ parse_string;
  ਹਾਲਤ '"':
    /* Parse string */
    opcode = JSONB_ਪਾਠ;
  parse_string:
    cDelim = z[i];
    j = i+1;
    ਜਦੋਂ( 1 /*exit-by-break*/ ){
      ਜੇ( jsonIsOk[(u8)z[j]] ){
        ਜੇ( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }ਨਹੀਂਤਾਂ ਜੇ( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }ਨਹੀਂਤਾਂ{
          j += 3;
          ਜਾਰੀ;
        }
      }
      c = z[j];
      ਜੇ( c==cDelim ){
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ ਜੇ( c=='\\' ){
        c = z[++j];
        ਜੇ( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          ਜੇ( opcode==JSONB_ਪਾਠ ) opcode = JSONB_TEXTJ;
        }ਨਹੀਂਤਾਂ ਜੇ( c=='\'' ||  c=='v' || c=='\n'
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#ਨਹੀਂਤਾਂ
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#ਜੇ_ਅੰਤ
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ਨਹੀਂਤਾਂ ਜੇ( c=='\r' ){
          ਜੇ( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ਨਹੀਂਤਾਂ{
          pParse->iErr = j;
          ਵਾਪਸ -1;
        }
      }ਨਹੀਂਤਾਂ ਜੇ( c<=0x1f ){
        ਜੇ( c==0 ){
          pParse->iErr = j;
          ਵਾਪਸ -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }ਨਹੀਂਤਾਂ ਜੇ( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_ਗੰਢ(pParse, opcode, j-1-i, &z[i+1]);
    ਵਾਪਸ j+1;
  }
  ਹਾਲਤ 't': {
    ਜੇ( ਲੜੀ_ਤੁਲਨਾ(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      ਵਾਪਸ i+4;
    }
    pParse->iErr = i;
    ਵਾਪਸ -1;
  }
  ਹਾਲਤ 'f': {
    ਜੇ( ਲੜੀ_ਤੁਲਨਾ(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      ਵਾਪਸ i+5;
    }
    pParse->iErr = i;
    ਵਾਪਸ -1;
  }
  ਹਾਲਤ '+': {
    u8 ਵੇਖਿਆ_E;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    ਜਾਓ parse_ਸੰਖਿਆ;
  ਹਾਲਤ '.':
    ਜੇ( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      ਵੇਖਿਆ_E = 0;
      ਜਾਓ parse_ਸੰਖਿਆ_2;
    }
    pParse->iErr = i;
    ਵਾਪਸ -1;
  ਹਾਲਤ '-':
  ਹਾਲਤ '0':
  ਹਾਲਤ '1':
  ਹਾਲਤ '2':
  ਹਾਲਤ '3':
  ਹਾਲਤ '4':
  ਹਾਲਤ '5':
  ਹਾਲਤ '6':
  ਹਾਲਤ '7':
  ਹਾਲਤ '8':
  ਹਾਲਤ '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_ਸੰਖਿਆ:
    ਵੇਖਿਆ_E = 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( '-' < '0' );
    ਪ੍ਰਤਿਜ੍ਞਾ( '+' < '0' );
    ਪ੍ਰਤਿਜ੍ਞਾ( '.' < '0' );
    c = z[i];

    ਜੇ( c<='0' ){
      ਜੇ( c=='0' ){
        ਜੇ( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          ਪ੍ਰਤਿਜ੍ਞਾ( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          ਲਈ(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          ਜਾਓ parse_ਸੰਖਿਆ_ਅੰਤ;
        }ਨਹੀਂਤਾਂ ਜੇ( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          ਵਾਪਸ -1;
        }
      }ਨਹੀਂਤਾਂ{
        ਜੇ( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          ਜੇ( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            ਜੇ( z[i]=='-' ){
              json_Blob_Append_ਗੰਢ(pParse, JSONB_FLOAT, 6, "-9e999");
            }ਨਹੀਂਤਾਂ{
              json_Blob_Append_ਗੰਢ(pParse, JSONB_FLOAT, 5, "9e999");
            }
            ਵਾਪਸ i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          ਜੇ( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            ਜਾਓ parse_ਸੰਖਿਆ_2;
          }
          pParse->iErr = i;
          ਵਾਪਸ -1;
        }
        ਜੇ( z[i+1]=='0' ){
          ਜੇ( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            ਵਾਪਸ -1;
          }ਨਹੀਂਤਾਂ ਜੇ( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            ਲਈ(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            ਜਾਓ parse_ਸੰਖਿਆ_ਅੰਤ;
          }
        }
      }
    }

  parse_ਸੰਖਿਆ_2:
    ਲਈ(j=i+1;; j++){
      c = z[j];
      ਜੇ( sqlite3Isdigit(c) ) ਜਾਰੀ;
      ਜੇ( c=='.' ){
        ਜੇ( (t & 0x02)!=0 ){
          pParse->iErr = j;
          ਵਾਪਸ -1;
        }
        t |= 0x02;
        ਜਾਰੀ;
      }
      ਜੇ( c=='e' || c=='E' ){
        ਜੇ( z[j-1]<'0' ){
          ਜੇ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }ਨਹੀਂਤਾਂ{
            pParse->iErr = j;
            ਵਾਪਸ -1;
          }
        }
        ਜੇ( ਵੇਖਿਆ_E ){
          pParse->iErr = j;
          ਵਾਪਸ -1;
        }
        t |= 0x02;
        ਵੇਖਿਆ_E = 1;
        c = z[j+1];
        ਜੇ( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        ਜੇ( c<'0' || c>'9' ){
          pParse->iErr = j;
          ਵਾਪਸ -1;
        }
        ਜਾਰੀ;
      }
      ਰੋਕੋ;
    }
    ਜੇ( z[j-1]<'0' ){
      ਜੇ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }ਨਹੀਂਤਾਂ{
        pParse->iErr = j;
        ਵਾਪਸ -1;
      }
    }
  parse_ਸੰਖਿਆ_ਅੰਤ:
    ਪ੍ਰਤਿਜ੍ਞਾ( JSONB_INT+0x01==JSONB_INT5 );
    ਪ੍ਰਤਿਜ੍ਞਾ( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    ਪ੍ਰਤਿਜ੍ਞਾ( JSONB_INT+0x02==JSONB_FLOAT );
    ਜੇ( z[i]=='+' ) i++;
    json_Blob_Append_ਗੰਢ(pParse, JSONB_INT+t, j-i, &z[i]);
    ਵਾਪਸ j;
  }
  ਹਾਲਤ '}': {
    pParse->iErr = i;
    ਵਾਪਸ -2;  /* End of {...} */
  }
  ਹਾਲਤ ']': {
    pParse->iErr = i;
    ਵਾਪਸ -3;  /* End of [...] */
  }
  ਹਾਲਤ ',': {
    pParse->iErr = i;
    ਵਾਪਸ -4;  /* List separator */
  }
  ਹਾਲਤ ':': {
    pParse->iErr = i;
    ਵਾਪਸ -5;  /* Object label/value separator */
  }
  ਹਾਲਤ 0: {
    ਵਾਪਸ 0;   /* End of file */
  }
  ਹਾਲਤ 0x09:
  ਹਾਲਤ 0x0a:
  ਹਾਲਤ 0x0d:
  ਹਾਲਤ 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    ਜਾਓ json_parse_restart;
  }
  ਹਾਲਤ 0x0b:
  ਹਾਲਤ 0x0c:
  ਹਾਲਤ '/':
  ਹਾਲਤ 0xc2:
  ਹਾਲਤ 0xe1:
  ਹਾਲਤ 0xe2:
  ਹਾਲਤ 0xe3:
  ਹਾਲਤ 0xef: {
    j = json5Whitespace(&z[i]);
    ਜੇ( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      ਜਾਓ json_parse_restart;
    }
    pParse->iErr = i;
    ਵਾਪਸ -1;
  }
  ਹਾਲਤ 'n': {
    ਜੇ( ਲੜੀ_ਤੁਲਨਾ(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      ਵਾਪਸ i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  ਮੂਲ: {
    u32 k;
    ਪੂਰਨ nn;
    c = z[i];
    ਲਈ(k=0; k<ਆਕਾਰ(aNanInfName)/ਆਕਾਰ(aNanInfName[0]); k++){
      ਜੇ( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) ਜਾਰੀ;
      nn = aNanInfName[k].n;
      ਜੇ( sqlite3StrNICmp(&z[i], aNanInfName[k].z_ਮੇਲ, nn)!=0 ){
        ਜਾਰੀ;
      }
      ਜੇ( sqlite3Isalnum(z[i+nn]) ) ਜਾਰੀ;
      ਜੇ( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_ਗੰਢ(pParse, JSONB_FLOAT, 5, "9e999");
      }ਨਹੀਂਤਾਂ{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      ਵਾਪਸ i + nn;
    }
    pParse->iErr = i;
    ਵਾਪਸ -1;  /* Syntax error */
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
ਸਥਿਰ ਪੂਰਨ json_Convert_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  ਪੂਰਨ i;
  ਅਚਲ ਅੱਖਰ *zJson = pParse->zJson;
  i = json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(pParse, 0);
  ਜੇ( pParse->oom ) i = -1;
  ਜੇ( i>0 ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->i_ਡੂੰਘਾਈ==0 );
    ਜੇ( sqlite3Config.bJsonSelfcheck ){
      ਪ੍ਰਤਿਜ੍ਞਾ( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#ਜੇ_ਅੰਤ
    ਜਦੋਂ( jsonIsspace(zJson[i]) ) i++;
    ਜੇ( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      ਜੇ( zJson[i] ){
        ਜੇ( pCtx ) sqlite3_ਨਤੀਜਾ_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        ਵਾਪਸ 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  ਜੇ( i<=0 ){
    ਜੇ( pCtx!=0 ){
      ਜੇ( pParse->oom ){
        sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
ਸਥਿਰ ਖਾਲੀ jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  ਪ੍ਰਤਿਜ੍ਞਾ( pStr->eErr==0 );
  memset(&px, 0, ਆਕਾਰ(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->n_ਵਰਤਿਆ;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (ਖਾਲੀ)json_Translate_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(&px, 0);
  ਜੇ( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_ਨਤੀਜਾ_error_nomem(pStr->pCtx);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( px.nBlobAlloc>0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( !px.bReadOnly );
    sqlite3_ਨਤੀਜਾ_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
ਸਥਿਰ u32 jsonb_Payload_ਮਾਪ_Wide(
  ਅਚਲ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  ਪ੍ਰਤਿਜ੍ਞਾ( i<pParse->nBlob );
  ਪ੍ਰਤਿਜ੍ਞਾ( x==(pParse->aBlob[i]>>4) );
  ਪ੍ਰਤਿਜ੍ਞਾ( x>=13 );
  ਜੇ( x==13 ){
    ਜੇ( i+2>=pParse->nBlob ){
      *pSz = 0;
      ਵਾਪਸ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ਨਹੀਂਤਾਂ ਜੇ( x==14 ){
    ਜੇ( i+4>=pParse->nBlob ){
      *pSz = 0;
      ਵਾਪਸ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ਵਾਪਸ 0;
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
  ਜੇ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    ਵਾਪਸ 0;
  }
  *pSz = sz;
  ਵਾਪਸ n;
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
ਸਥਿਰ u32 jsonb_Payload_ਮਾਪ(
  ਅਚਲ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ਜੇ( i>=pParse->nBlob ){
    *pSz = 0;
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ ਜੇ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( x==12 ){
    ਜੇ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ਵਾਪਸ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ jsonb_Payload_ਮਾਪ_Wide(pParse, i, pSz, x);
  }
  ਜੇ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ਵਾਪਸ 0;
  }
  *pSz = sz;
  ਵਾਪਸ n;
}

#ਜੇ SQLITE_ਵਰਤਦਾ_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
ਸਥਿਰ SQLITE_INLINE u32 jsonb_Payload_ਮਾਪ_Inline(
  ਅਚਲ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ਜੇ( i>=pParse->nBlob ){
    *pSz = 0;
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ ਜੇ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( x==12 ){
    ਜੇ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ਵਾਪਸ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ਨਹੀਂਤਾਂ ਜੇ( x==13 ){
    ਜੇ( i+2>=pParse->nBlob ){
      *pSz = 0;
      ਵਾਪਸ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ਨਹੀਂਤਾਂ ਜੇ( x==14 ){
    ਜੇ( i+4>=pParse->nBlob ){
      *pSz = 0;
      ਵਾਪਸ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ਵਾਪਸ 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  ਜੇ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ਵਾਪਸ 0;
  }
  *pSz = sz;
  ਵਾਪਸ n;
}
#ਨਹੀਂਤਾਂ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_ਮਾਪ_Inline(a,b,c) jsonb_Payload_ਮਾਪ(a,b,c)
#ਜੇ_ਅੰਤ


#ਜੇ SQLITE_ਵਰਤਦਾ_INLINE
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
ਸਥਿਰ SQLITE_INLINE u32 jsonb_Payload_ਮਾਪ_Semi_Inline(
  ਅਚਲ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ਪ੍ਰਤਿਜ੍ਞਾ( i<pParse->nBlob );
  ਜੇ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( x==12 ){
    ਜੇ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ਵਾਪਸ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ jsonb_Payload_ਮਾਪ_Wide(pParse,i,pSz,x);
  }
  ਜੇ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ਵਾਪਸ 0;
  }
  *pSz = sz;
  ਵਾਪਸ n;
}
#ਨਹੀਂਤਾਂ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_ਮਾਪ_Semi_Inline(a,b,c) jsonb_Payload_ਮਾਪ(a,b,c)
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ u32 json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *p_ਬਾਹਰਲਾ               /* Write JSON here */
){
  u32 sz, n, j, i_ਸਿਰਾ;

  n = jsonb_Payload_ਮਾਪ_Inline(pParse, i, &sz);
  ਜੇ( n==0 ){
    p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
    ਵਾਪਸ pParse->nBlob+1;
  }
  ਚੋਣ( pParse->aBlob[i] & 0x0f ){
    ਹਾਲਤ JSONB_NULL: {
      jsonAppendRawNZ(p_ਬਾਹਰਲਾ, "null", 4);
      ਵਾਪਸ i+1;
    }
    ਹਾਲਤ JSONB_TRUE: {
      jsonAppendRawNZ(p_ਬਾਹਰਲਾ, "true", 4);
      ਵਾਪਸ i+1;
    }
    ਹਾਲਤ JSONB_FALSE: {
      jsonAppendRawNZ(p_ਬਾਹਰਲਾ, "false", 5);
      ਵਾਪਸ i+1;
    }
    ਹਾਲਤ JSONB_INT:
    ਹਾਲਤ JSONB_FLOAT: {
      ਜੇ( sz==0 ) ਜਾਓ malformed_jsonb;
      jsonAppendRaw(p_ਬਾਹਰਲਾ, (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n], sz);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ = (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n];
      ਪੂਰਨ b_ਉਛਾਲ = 0;
      ਜੇ( sz==0 ) ਜਾਓ malformed_jsonb;
      ਜੇ( z_ਵਿੱਚ[0]=='-' ){
        jsonAppendChar(p_ਬਾਹਰਲਾ, '-');
        k++;
      }ਨਹੀਂਤਾਂ ਜੇ( z_ਵਿੱਚ[0]=='+' ){
        k++;
      }
      ਲਈ(; k<sz; k++){
        ਜੇ( !sqlite3Isxdigit(z_ਵਿੱਚ[k]) ){
          p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ ਜੇ( (u>>60)!=0 ){
          b_ਉਛਾਲ = 1;
        }ਨਹੀਂਤਾਂ{
          u = u*16 + sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(z_ਵਿੱਚ[k]);
        }
      }
      jsonPrintf(100,p_ਬਾਹਰਲਾ,b_ਉਛਾਲ?"9.0e999":"%llu", u);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ = (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n];
      ਜੇ( sz==0 ) ਜਾਓ malformed_jsonb;
      ਜੇ( z_ਵਿੱਚ[0]=='-' ){
        jsonAppendChar(p_ਬਾਹਰਲਾ, '-');
        ਜੇ( sz<=1 ) ਜਾਓ malformed_jsonb;
        k = 1;
      }
      ਜੇ( z_ਵਿੱਚ[k]=='.' ){
        jsonAppendChar(p_ਬਾਹਰਲਾ, '0');
      }
      ਲਈ(; k<sz; k++){
        jsonAppendChar(p_ਬਾਹਰਲਾ, z_ਵਿੱਚ[k]);
        ਜੇ( z_ਵਿੱਚ[k]=='.' && (k+1==sz || !sqlite3Isdigit(z_ਵਿੱਚ[k+1])) ){
          jsonAppendChar(p_ਬਾਹਰਲਾ, '0');
        }
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_ਪਾਠ:
    ਹਾਲਤ JSONB_TEXTJ: {
      ਜੇ( p_ਬਾਹਰਲਾ->n_ਵਰਤਿਆ+sz+2<=p_ਬਾਹਰਲਾ->nAlloc || jsonStringGrow(p_ਬਾਹਰਲਾ, sz+2)==0 ){
        p_ਬਾਹਰਲਾ->zBuf[p_ਬਾਹਰਲਾ->n_ਵਰਤਿਆ] = '"';
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਬਾਹਰਲਾ->zBuf+p_ਬਾਹਰਲਾ->n_ਵਰਤਿਆ+1,(ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n],sz);
        p_ਬਾਹਰਲਾ->zBuf[p_ਬਾਹਰਲਾ->n_ਵਰਤਿਆ+sz+1] = '"';
        p_ਬਾਹਰਲਾ->n_ਵਰਤਿਆ += sz+2;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_TEXT5: {
      ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ;
      u32 k;
      u32 sz2 = sz;
      z_ਵਿੱਚ = (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n];
      jsonAppendChar(p_ਬਾਹਰਲਾ, '"');
      ਜਦੋਂ( sz2>0 ){
        ਲਈ(k=0; k<sz2 && (jsonIsOk[(u8)z_ਵਿੱਚ[k]] || z_ਵਿੱਚ[k]=='\''); k++){}
        ਜੇ( k>0 ){
          jsonAppendRawNZ(p_ਬਾਹਰਲਾ, z_ਵਿੱਚ, k);
          ਜੇ( k>=sz2 ){
            ਰੋਕੋ;
          }
          z_ਵਿੱਚ += k;
          sz2 -= k;
        }
        ਜੇ( z_ਵਿੱਚ[0]=='"' ){
          jsonAppendRawNZ(p_ਬਾਹਰਲਾ, "\\\"", 2);
          z_ਵਿੱਚ++;
          sz2--;
          ਜਾਰੀ;
        }
        ਜੇ( z_ਵਿੱਚ[0]<=0x1f ){
          ਜੇ( p_ਬਾਹਰਲਾ->n_ਵਰਤਿਆ+7>p_ਬਾਹਰਲਾ->nAlloc && jsonStringGrow(p_ਬਾਹਰਲਾ,7) ) ਰੋਕੋ;
          jsonAppendControlChar(p_ਬਾਹਰਲਾ, z_ਵਿੱਚ[0]);
          z_ਵਿੱਚ++;
          sz2--;
          ਜਾਰੀ;
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( z_ਵਿੱਚ[0]=='\\' );
        ਪ੍ਰਤਿਜ੍ਞਾ( sz2>=1 );
        ਜੇ( sz2<2 ){
          p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
          ਰੋਕੋ;
        }
        ਚੋਣ( (u8)z_ਵਿੱਚ[1] ){
          ਹਾਲਤ '\'':
            jsonAppendChar(p_ਬਾਹਰਲਾ, '\'');
            ਰੋਕੋ;
          ਹਾਲਤ 'v':
            jsonAppendRawNZ(p_ਬਾਹਰਲਾ, "\\u000b", 6);
            ਰੋਕੋ;
          ਹਾਲਤ 'x':
            ਜੇ( sz2<4 ){
              p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ਰੋਕੋ;
            }
            jsonAppendRawNZ(p_ਬਾਹਰਲਾ, "\\u00", 4);
            jsonAppendRawNZ(p_ਬਾਹਰਲਾ, &z_ਵਿੱਚ[2], 2);
            z_ਵਿੱਚ += 2;
            sz2 -= 2;
            ਰੋਕੋ;
          ਹਾਲਤ '0':
            jsonAppendRawNZ(p_ਬਾਹਰਲਾ, "\\u0000", 6);
            ਰੋਕੋ;
          ਹਾਲਤ '\r':
            ਜੇ( sz2>2 && z_ਵਿੱਚ[2]=='\n' ){
              z_ਵਿੱਚ++;
              sz2--;
            }
            ਰੋਕੋ;
          ਹਾਲਤ '\n':
            ਰੋਕੋ;
          ਹਾਲਤ 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            ਜੇ( sz2<4
             || 0x80!=(u8)z_ਵਿੱਚ[2]
             || (0xa8!=(u8)z_ਵਿੱਚ[3] && 0xa9!=(u8)z_ਵਿੱਚ[3])
            ){
              p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ਰੋਕੋ;
            }
            z_ਵਿੱਚ += 2;
            sz2 -= 2;
            ਰੋਕੋ;
          ਮੂਲ:
            jsonAppendRawNZ(p_ਬਾਹਰਲਾ, z_ਵਿੱਚ, 2);
            ਰੋਕੋ;
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( sz2>=2 );
        z_ਵਿੱਚ += 2;
        sz2 -= 2;
      }
      jsonAppendChar(p_ਬਾਹਰਲਾ, '"');
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_TEXTRAW: {
      jsonAppendString(p_ਬਾਹਰਲਾ, (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n], sz);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_ARRAY: {
      jsonAppendChar(p_ਬਾਹਰਲਾ, '[');
      j = i+n;
      i_ਸਿਰਾ = j+sz;
      ਜੇ( ++pParse->i_ਡੂੰਘਾਈ > JSON_MAX_ਡੂੰਘਾਈ ){
        jsonStringTooDeep(p_ਬਾਹਰਲਾ);
      }
      ਜਦੋਂ( j<i_ਸਿਰਾ && p_ਬਾਹਰਲਾ->eErr==0 ){
        j = json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(pParse, j, p_ਬਾਹਰਲਾ);
        jsonAppendChar(p_ਬਾਹਰਲਾ, ',');
      }
      pParse->i_ਡੂੰਘਾਈ--;
      ਜੇ( j>i_ਸਿਰਾ ) p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
      ਜੇ( sz>0 ) jsonStringTrimOneChar(p_ਬਾਹਰਲਾ);
      jsonAppendChar(p_ਬਾਹਰਲਾ, ']');
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_OBJECT: {
      ਪੂਰਨ x = 0;
      jsonAppendChar(p_ਬਾਹਰਲਾ, '{');
      j = i+n;
      i_ਸਿਰਾ = j+sz;
      ਜੇ( ++pParse->i_ਡੂੰਘਾਈ > JSON_MAX_ਡੂੰਘਾਈ ){
        jsonStringTooDeep(p_ਬਾਹਰਲਾ);
      }
      ਜਦੋਂ( j<i_ਸਿਰਾ && p_ਬਾਹਰਲਾ->eErr==0 ){
        j = json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(pParse, j, p_ਬਾਹਰਲਾ);
        jsonAppendChar(p_ਬਾਹਰਲਾ, (x++ & 1) ? ',' : ':');
      }
      pParse->i_ਡੂੰਘਾਈ--;
      ਜੇ( (x & 1)!=0 || j>i_ਸਿਰਾ ) p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
      ਜੇ( sz>0 ) jsonStringTrimOneChar(p_ਬਾਹਰਲਾ);
      jsonAppendChar(p_ਬਾਹਰਲਾ, '}');
      ਰੋਕੋ;
    }

    ਮੂਲ: {
      malformed_jsonb:
      p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ i+n+sz;
}

/* Context for recursion of json_pretty()
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ JsonPretty JsonPretty;
ਬਣਤਰ JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *p_ਬਾਹਰਲਾ;         /* Generate pretty output into this string */
  ਅਚਲ ਅੱਖਰ *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
ਸਥਿਰ ਖਾਲੀ jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  ਲਈ(jj=0; jj<pPretty->nIndent; jj++){
    jsonAppendRaw(pPretty->p_ਬਾਹਰਲਾ, pPretty->zIndent, pPretty->szIndent);
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
ਸਥਿਰ u32 json_Translate_Blob_ਮੰਜ਼ਿਲ_Pretty_ਪਾਠ(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, i_ਸਿਰਾ;
  JsonParse *pParse = pPretty->pParse;
  JsonString *p_ਬਾਹਰਲਾ = pPretty->p_ਬਾਹਰਲਾ;
  n = jsonb_Payload_ਮਾਪ(pParse, i, &sz);
  ਜੇ( n==0 ){
    p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
    ਵਾਪਸ pParse->nBlob+1;
  }
  ਚੋਣ( pParse->aBlob[i] & 0x0f ){
    ਹਾਲਤ JSONB_ARRAY: {
      j = i+n;
      i_ਸਿਰਾ = j+sz;
      jsonAppendChar(p_ਬਾਹਰਲਾ, '[');
      ਜੇ( j<i_ਸਿਰਾ ){
        jsonAppendChar(p_ਬਾਹਰਲਾ, '\n');
        pPretty->nIndent++;
        ਜੇ( pPretty->nIndent >= JSON_MAX_ਡੂੰਘਾਈ ){
          jsonStringTooDeep(p_ਬਾਹਰਲਾ);
        }
        ਜਦੋਂ( p_ਬਾਹਰਲਾ->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_ਮੰਜ਼ਿਲ_Pretty_ਪਾਠ(pPretty, j);
          ਜੇ( j>=i_ਸਿਰਾ ){
            ਜੇ( j>i_ਸਿਰਾ ) p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
            ਰੋਕੋ;
          }
          jsonAppendRawNZ(p_ਬਾਹਰਲਾ, ",\n", 2);
        }
        jsonAppendChar(p_ਬਾਹਰਲਾ, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_ਬਾਹਰਲਾ, ']');
      i = i_ਸਿਰਾ;
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_OBJECT: {
      j = i+n;
      i_ਸਿਰਾ = j+sz;
      jsonAppendChar(p_ਬਾਹਰਲਾ, '{');
      ਜੇ( j<i_ਸਿਰਾ ){
        jsonAppendChar(p_ਬਾਹਰਲਾ, '\n');
        pPretty->nIndent++;
        ਜੇ( pPretty->nIndent >= JSON_MAX_ਡੂੰਘਾਈ ){
          jsonStringTooDeep(p_ਬਾਹਰਲਾ);
        }
        pParse->i_ਡੂੰਘਾਈ = pPretty->nIndent;
        ਜਦੋਂ( p_ਬਾਹਰਲਾ->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(pParse, j, p_ਬਾਹਰਲਾ);
          ਜੇ( j>i_ਸਿਰਾ ){
            p_ਬਾਹਰਲਾ->eErr |= JSTRING_MALFORMED;
            ਰੋਕੋ;
          }
          jsonAppendRawNZ(p_ਬਾਹਰਲਾ, ": ", 2);
          j = json_Translate_Blob_ਮੰਜ਼ਿਲ_Pretty_ਪਾਠ(pPretty, j);
          ਜੇ( j>=i_ਸਿਰਾ ) ਰੋਕੋ;
          jsonAppendRawNZ(p_ਬਾਹਰਲਾ, ",\n", 2);
        }
        jsonAppendChar(p_ਬਾਹਰਲਾ, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_ਬਾਹਰਲਾ, '}');
      i = i_ਸਿਰਾ;
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      i = json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(pParse, i, p_ਬਾਹਰਲਾ);
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
ਸਥਿਰ u32 jsonb_Array_ਗਿਣਤੀ(JsonParse *pParse, u32 i_ਜੜ੍ਹ){
  u32 n, sz, i, i_ਸਿਰਾ;
  u32 k = 0;
  n = jsonb_Payload_ਮਾਪ(pParse, i_ਜੜ੍ਹ, &sz);
  i_ਸਿਰਾ = i_ਜੜ੍ਹ+n+sz;
  ਲਈ(i=i_ਜੜ੍ਹ+n; n>0 && i<i_ਸਿਰਾ; i+=sz+n, k++){
    n = jsonb_Payload_ਮਾਪ_Semi_Inline(pParse, i, &sz);
  }
  ਵਾਪਸ k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
ਸਥਿਰ ਖਾਲੀ json_After_Edit_ਮਾਪ_Adjust(JsonParse *pParse, u32 i_ਜੜ੍ਹ){
  u32 sz = 0;
  u32 nBlob;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->delta!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (ਖਾਲੀ)jsonb_Payload_ਮਾਪ(pParse, i_ਜੜ੍ਹ, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += json_Blob_Change_Payload_ਮਾਪ(pParse, i_ਜੜ੍ਹ, sz);
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
ਸਥਿਰ ਪੂਰਨ jsonBlobOverwrite(
  u8 *a_ਬਾਹਰਲਾ,                 /* Overwrite here */
  ਅਚਲ u8 *aIns,           /* New content */
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
  ਸਥਿਰ ਅਚਲ u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  ਜੇ( (aIns[0]&0x0f)<=2 ) ਵਾਪਸ 0;    /* Cannot enlarge NULL, true, false */
  ਚੋਣ( aIns[0]>>4 ){
    ਮੂਲ: {                         /* aIns[] header size 1 */
      ਜੇ( ((1<<d)&0x116)==0 ) ਵਾਪਸ 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      ਰੋਕੋ;
    }
    ਹਾਲਤ 12: {                         /* aIns[] header size is 2 */
      ਜੇ( ((1<<d)&0x8a)==0) ਵਾਪਸ 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      ਰੋਕੋ;
    }
    ਹਾਲਤ 13: {                         /* aIns[] header size is 3 */
      ਜੇ( d!=2 && d!=6 ) ਵਾਪਸ 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      ਰੋਕੋ;
    }
    ਹਾਲਤ 14: {                         /* aIns[] header size is 5 */
      ਜੇ( d!=4 ) ਵਾਪਸ 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      ਰੋਕੋ;
    }
    ਹਾਲਤ 15: {                         /* aIns[] header size is 9 */
      ਵਾਪਸ 0;                          /* No solution */
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=2 && i<=9 && aType[i-2]!=0 );
  a_ਬਾਹਰਲਾ[0] = (aIns[0] & 0x0f) | aType[i-2];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&a_ਬਾਹਰਲਾ[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  ਜਦੋਂ( 1/*edit-by-break*/ ){
    i--;
    a_ਬਾਹਰਲਾ[i] = szPayload & 0xff;
    ਜੇ( i==1 ) ਰੋਕੋ;
    szPayload >>= 8;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (szPayload>>8)==0 );
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਖਾਲੀ jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  ਅਚਲ u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nBlob >= (u64)iDel + (u64)nDel );
  ਜੇ( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (ਪੂਰਨ)-d)
  ){
    ਵਾਪਸ;
  }
  ਜੇ( d!=0 ){
    ਜੇ( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      ਜੇ( pParse->oom ) ਵਾਪਸ;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  ਜੇ( nIns && aIns ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pParse->aBlob[iDel], aIns, nIns);
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
ਸਥਿਰ u32 json_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Bypass(ਅਚਲ ਅੱਖਰ *z, u32 n){
  u32 i = 0;
  ਜਦੋਂ( i+1<n ){
    ਜੇ( z[i]!='\\' ) ਵਾਪਸ i;
    ਜੇ( z[i+1]=='\n' ){
      i += 2;
      ਜਾਰੀ;
    }
    ਜੇ( z[i+1]=='\r' ){
      ਜੇ( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }ਨਹੀਂਤਾਂ{
        i += 2;
      }
      ਜਾਰੀ;
    }
    ਜੇ( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      ਜਾਰੀ;
    }
    ਰੋਕੋ;
  }
  ਵਾਪਸ i;
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
ਸਥਿਰ u32 jsonUnescapeOneChar(ਅਚਲ ਅੱਖਰ *z, u32 n, u32 *pi_ਬਾਹਰਲਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( n>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( z[0]=='\\' );
  ਜੇ( n<2 ){
    *pi_ਬਾਹਰਲਾ = JSON_INVALID_CHAR;
    ਵਾਪਸ n;
  }
  ਚੋਣ( (u8)z[1] ){
    ਹਾਲਤ 'u': {
      u32 v, vlo;
      ਜੇ( n<6 ){
        *pi_ਬਾਹਰਲਾ = JSON_INVALID_CHAR;
        ਵਾਪਸ n;
      }
      v = json_Hex_ਮੰਜ਼ਿਲ_Int4(&z[2]);
      ਜੇ( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = json_Hex_ਮੰਜ਼ਿਲ_Int4(&z[8]))&0xfc00)==0xdc00
      ){
        *pi_ਬਾਹਰਲਾ = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        ਵਾਪਸ 12;
      }ਨਹੀਂਤਾਂ{
        *pi_ਬਾਹਰਲਾ = v;
        ਵਾਪਸ 6;
      }
    }
    ਹਾਲਤ 'b': {   *pi_ਬਾਹਰਲਾ = '\b';  ਵਾਪਸ 2; }
    ਹਾਲਤ 'f': {   *pi_ਬਾਹਰਲਾ = '\f';  ਵਾਪਸ 2; }
    ਹਾਲਤ 'n': {   *pi_ਬਾਹਰਲਾ = '\n';  ਵਾਪਸ 2; }
    ਹਾਲਤ 'r': {   *pi_ਬਾਹਰਲਾ = '\r';  ਵਾਪਸ 2; }
    ਹਾਲਤ 't': {   *pi_ਬਾਹਰਲਾ = '\t';  ਵਾਪਸ 2; }
    ਹਾਲਤ 'v': {   *pi_ਬਾਹਰਲਾ = '\v';  ਵਾਪਸ 2; }
    ਹਾਲਤ '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *pi_ਬਾਹਰਲਾ = 0;
#ਨਹੀਂਤਾਂ
      /* Correct behavior */
      *pi_ਬਾਹਰਲਾ = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#ਜੇ_ਅੰਤ
      ਵਾਪਸ 2;
    }
    ਹਾਲਤ '\'':
    ਹਾਲਤ '"':
    ਹਾਲਤ '/':
    ਹਾਲਤ '\\':{   *pi_ਬਾਹਰਲਾ = z[1];  ਵਾਪਸ 2; }
    ਹਾਲਤ 'x': {
      ਜੇ( n<4 ){
        *pi_ਬਾਹਰਲਾ = JSON_INVALID_CHAR;
        ਵਾਪਸ n;
      }
      *pi_ਬਾਹਰਲਾ = (json_Hex_ਮੰਜ਼ਿਲ_Int(z[2])<<4) | json_Hex_ਮੰਜ਼ਿਲ_Int(z[3]);
      ਵਾਪਸ 4;
    }
    ਹਾਲਤ 0xe2:
    ਹਾਲਤ '\r':
    ਹਾਲਤ '\n': {
      u32 nSkip = json_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Bypass(z, n);
      ਜੇ( nSkip==0 ){
        *pi_ਬਾਹਰਲਾ = JSON_INVALID_CHAR;
        ਵਾਪਸ n;
      }ਨਹੀਂਤਾਂ ਜੇ( nSkip==n ){
        *pi_ਬਾਹਰਲਾ = 0;
        ਵਾਪਸ n;
      }ਨਹੀਂਤਾਂ ਜੇ( z[nSkip]=='\\' ){
        ਵਾਪਸ nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, pi_ਬਾਹਰਲਾ);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, pi_ਬਾਹਰਲਾ);
        ਵਾਪਸ nSkip + sz;
      }
    }
    ਮੂਲ: {
      *pi_ਬਾਹਰਲਾ = JSON_INVALID_CHAR;
      ਵਾਪਸ 2;
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ jsonLabelCompareEscaped(
  ਅਚਲ ਅੱਖਰ *z_ਖੱਬਾ,          /* The left label */
  u32 n_ਖੱਬਾ,                  /* Size of the left label in bytes */
  ਪੂਰਨ raw_ਖੱਬਾ,                /* True if zLeft contains no escapes */
  ਅਚਲ ਅੱਖਰ *z_ਸੱਜਾ,         /* The right label */
  u32 n_ਸੱਜਾ,                 /* Size of the right label in bytes */
  ਪੂਰਨ raw_ਸੱਜਾ                /* True if zRight is escape-free */
){
  u32 c_ਖੱਬਾ, c_ਸੱਜਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( raw_ਖੱਬਾ==0 || raw_ਸੱਜਾ==0 );
  ਜਦੋਂ( 1 /*exit-by-return*/ ){
    ਜੇ( n_ਖੱਬਾ==0 ){
      c_ਖੱਬਾ = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( raw_ਖੱਬਾ || z_ਖੱਬਾ[0]!='\\' ){
      c_ਖੱਬਾ = ((u8*)z_ਖੱਬਾ)[0];
      ਜੇ( c_ਖੱਬਾ>=0xc0 ){
        ਪੂਰਨ sz = sqlite3Utf8ReadLimited((u8*)z_ਖੱਬਾ, n_ਖੱਬਾ, &c_ਖੱਬਾ);
        z_ਖੱਬਾ += sz;
        n_ਖੱਬਾ -= sz;
      }ਨਹੀਂਤਾਂ{
        z_ਖੱਬਾ++;
        n_ਖੱਬਾ--;
      }
    }ਨਹੀਂਤਾਂ{
      u32 n = jsonUnescapeOneChar(z_ਖੱਬਾ, n_ਖੱਬਾ, &c_ਖੱਬਾ);
      z_ਖੱਬਾ += n;
      ਪ੍ਰਤਿਜ੍ਞਾ( n<=n_ਖੱਬਾ );
      n_ਖੱਬਾ -= n;
    }
    ਜੇ( n_ਸੱਜਾ==0 ){
      c_ਸੱਜਾ = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( raw_ਸੱਜਾ || z_ਸੱਜਾ[0]!='\\' ){
      c_ਸੱਜਾ = ((u8*)z_ਸੱਜਾ)[0];
      ਜੇ( c_ਸੱਜਾ>=0xc0 ){
        ਪੂਰਨ sz = sqlite3Utf8ReadLimited((u8*)z_ਸੱਜਾ, n_ਸੱਜਾ, &c_ਸੱਜਾ);
        z_ਸੱਜਾ += sz;
        n_ਸੱਜਾ -= sz;
      }ਨਹੀਂਤਾਂ{
        z_ਸੱਜਾ++;
        n_ਸੱਜਾ--;
      }
    }ਨਹੀਂਤਾਂ{
      u32 n = jsonUnescapeOneChar(z_ਸੱਜਾ, n_ਸੱਜਾ, &c_ਸੱਜਾ);
      z_ਸੱਜਾ += n;
      ਪ੍ਰਤਿਜ੍ਞਾ( n<=n_ਸੱਜਾ );
      n_ਸੱਜਾ -= n;
    }
    ਜੇ( c_ਖੱਬਾ!=c_ਸੱਜਾ ) ਵਾਪਸ 0;
    ਜੇ( c_ਖੱਬਾ==0 ) ਵਾਪਸ 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
ਸਥਿਰ ਪੂਰਨ jsonLabelCompare(
  ਅਚਲ ਅੱਖਰ *z_ਖੱਬਾ,          /* The left label */
  u32 n_ਖੱਬਾ,                  /* Size of the left label in bytes */
  ਪੂਰਨ raw_ਖੱਬਾ,                /* True if zLeft contains no escapes */
  ਅਚਲ ਅੱਖਰ *z_ਸੱਜਾ,         /* The right label */
  u32 n_ਸੱਜਾ,                 /* Size of the right label in bytes */
  ਪੂਰਨ raw_ਸੱਜਾ                /* True if zRight is escape-free */
){
  ਜੇ( raw_ਖੱਬਾ && raw_ਸੱਜਾ ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    ਜੇ( n_ਖੱਬਾ!=n_ਸੱਜਾ ) ਵਾਪਸ 0;
    ਵਾਪਸ memcmp(z_ਖੱਬਾ, z_ਸੱਜਾ, n_ਖੱਬਾ)==0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ jsonLabelCompareEscaped(z_ਖੱਬਾ, n_ਖੱਬਾ, raw_ਖੱਬਾ,
                                   z_ਸੱਜਾ, n_ਸੱਜਾ, raw_ਸੱਜਾ);
  }
}

/*
** Error returns from jsonLookupStep()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_LOOKUP_ERROR      0xffffffff
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_LOOKUP_NOTFOUND   0xfffffffe
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_LOOKUP_NOTARRAY   0xfffffffd
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_LOOKUP_TOODEEP    0xfffffffc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_LOOKUP_PATHERROR  0xfffffffb
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_LOOKUP_ISERROR(x) ((x)>=JSON_LOOKUP_PATHERROR)

/* Forward declaration */
ਸਥਿਰ u32 json_Lookup_ਕਦਮ(JsonParse*,u32,ਅਚਲ ਅੱਖਰ*,u32);


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
ਸਥਿਰ u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  ਅਚਲ ਅੱਖਰ *z_ਪੂਛ   /* Tail of the path that determines substructure */
){
  ਸਥਿਰ ਅਚਲ u8 ਖਾਲੀ_ਥਾਵਾਂ_Object[] = { JSONB_ARRAY, JSONB_OBJECT };
  ਪੂਰਨ rc;
  memset(pIns, 0, ਆਕਾਰ(*pIns));
  pIns->db = pParse->db;
  ਜੇ( z_ਪੂਛ[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }ਨਹੀਂਤਾਂ{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&ਖਾਲੀ_ਥਾਵਾਂ_Object[z_ਪੂਛ[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->i_ਡੂੰਘਾਈ = pParse->i_ਡੂੰਘਾਈ+1;
    ਜੇ( pIns->i_ਡੂੰਘਾਈ >= JSON_MAX_ਡੂੰਘਾਈ ){
      ਵਾਪਸ JSON_LOOKUP_TOODEEP;
    }
    rc = json_Lookup_ਕਦਮ(pIns, 0, z_ਪੂਛ, 0);
    pParse->i_ਡੂੰਘਾਈ--;
    pParse->oom |= pIns->oom;
  }
  ਵਾਪਸ rc;  /* Error code only */
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
ਸਥਿਰ u32 json_Lookup_ਕਦਮ(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_ਜੜ੍ਹ,              /* Begin the search at this element of aBlob[] */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_ਕੁੰਜੀ, sz, n, i_ਸਿਰਾ, rc;
  ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ;
  u8 x;

  ਜੇ( z_ਰਸਤਾ[0]==0 ){
    ਜੇ( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonb_Payload_ਮਾਪ(pParse, i_ਜੜ੍ਹ, &sz);
      sz += n;
      ਜੇ( pParse->eEdit==JEDIT_DEL ){
        ਜੇ( iLabel>0 ){
          sz += i_ਜੜ੍ਹ - iLabel;
          i_ਜੜ੍ਹ = iLabel;
        }
        jsonBlobEdit(pParse, i_ਜੜ੍ਹ, sz, 0, 0);
      }ਨਹੀਂਤਾਂ ਜੇ( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }ਨਹੀਂਤਾਂ ਜੇ( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        ਜੇ( z_ਰਸਤਾ[-1]!=']' ){
          ਵਾਪਸ JSON_LOOKUP_NOTARRAY;
        }ਨਹੀਂਤਾਂ{
          jsonBlobEdit(pParse, i_ਜੜ੍ਹ, 0, pParse->aIns, pParse->nIns);
        }
      }ਨਹੀਂਤਾਂ{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_ਜੜ੍ਹ, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    ਵਾਪਸ i_ਜੜ੍ਹ;
  }
  ਜੇ( z_ਰਸਤਾ[0]=='.' ){
    ਪੂਰਨ raw_ਕੁੰਜੀ = 1;
    x = pParse->aBlob[i_ਜੜ੍ਹ];
    z_ਰਸਤਾ++;
    ਜੇ( z_ਰਸਤਾ[0]=='"' ){
      z_ਕੁੰਜੀ = z_ਰਸਤਾ + 1;
      ਲਈ(i=1; z_ਰਸਤਾ[i] && z_ਰਸਤਾ[i]!='"'; i++){
        ਜੇ( z_ਰਸਤਾ[i]=='\\' && z_ਰਸਤਾ[i+1]!=0 ) i++;
      }
      n_ਕੁੰਜੀ = i-1;
      ਜੇ( z_ਰਸਤਾ[i] ){
        i++;
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ JSON_LOOKUP_PATHERROR;
      }
      testcase( n_ਕੁੰਜੀ==0 );
      raw_ਕੁੰਜੀ = memchr(z_ਕੁੰਜੀ, '\\', n_ਕੁੰਜੀ)==0;
    }ਨਹੀਂਤਾਂ{
      z_ਕੁੰਜੀ = z_ਰਸਤਾ;
      ਲਈ(i=0; z_ਰਸਤਾ[i] && z_ਰਸਤਾ[i]!='.' && z_ਰਸਤਾ[i]!='['; i++){}
      n_ਕੁੰਜੀ = i;
      ਜੇ( n_ਕੁੰਜੀ==0 ){
        ਵਾਪਸ JSON_LOOKUP_PATHERROR;
      }
    }
    ਜੇ( (x & 0x0f)!=JSONB_OBJECT ) ਵਾਪਸ JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_ਮਾਪ(pParse, i_ਜੜ੍ਹ, &sz);
    j = i_ਜੜ੍ਹ + n;  /* j is the index of a label */
    i_ਸਿਰਾ = j+sz;
    ਜਦੋਂ( j<i_ਸਿਰਾ ){
      ਪੂਰਨ rawLabel;
      ਅਚਲ ਅੱਖਰ *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      ਜੇ( x<JSONB_ਪਾਠ || x>JSONB_TEXTRAW ) ਵਾਪਸ JSON_LOOKUP_ERROR;
      n = jsonb_Payload_ਮਾਪ_Semi_Inline(pParse, j, &sz);
      ਜੇ( n==0 ) ਵਾਪਸ JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      ਜੇ( k+sz>=i_ਸਿਰਾ ) ਵਾਪਸ JSON_LOOKUP_ERROR;
      zLabel = (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[k];
      rawLabel = x==JSONB_ਪਾਠ || x==JSONB_TEXTRAW;
      ਜੇ( jsonLabelCompare(z_ਕੁੰਜੀ, n_ਕੁੰਜੀ, raw_ਕੁੰਜੀ, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        ਜੇ( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) ਵਾਪਸ JSON_LOOKUP_ERROR;
        n = jsonb_Payload_ਮਾਪ(pParse, v, &sz);
        ਜੇ( n==0 || v+n+sz>i_ਸਿਰਾ ) ਵਾਪਸ JSON_LOOKUP_ERROR;
        ਪ੍ਰਤਿਜ੍ਞਾ( j>0 );
        ਜੇ( ++pParse->i_ਡੂੰਘਾਈ >= JSON_MAX_ਡੂੰਘਾਈ ){
          ਵਾਪਸ JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_ਕਦਮ(pParse, v, &z_ਰਸਤਾ[i], j);
        pParse->i_ਡੂੰਘਾਈ--;
        ਜੇ( pParse->delta ) json_After_Edit_ਮਾਪ_Adjust(pParse, i_ਜੜ੍ਹ);
        ਵਾਪਸ rc;
      }
      j = k+sz;
      ਜੇ( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) ਵਾਪਸ JSON_LOOKUP_ERROR;
      n = jsonb_Payload_ਮਾਪ_Semi_Inline(pParse, j, &sz);
      ਜੇ( n==0 ) ਵਾਪਸ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ਜੇ( j>i_ਸਿਰਾ ) ਵਾਪਸ JSON_LOOKUP_ERROR;
    ਜੇ( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_ਸੈੱਟ );
      testcase( pParse->eEdit==JEDIT_AINS );
      ਜੇ( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&z_ਰਸਤਾ[i])!=0 ){
        ਵਾਪਸ JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, ਆਕਾਰ(ix));
      ix.db = pParse->db;
      json_Blob_Append_ਗੰਢ(&ix, raw_ਕੁੰਜੀ?JSONB_TEXTRAW:JSONB_TEXT5, n_ਕੁੰਜੀ, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &z_ਰਸਤਾ[i]);
      ਜੇ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_ਕੁੰਜੀ+v.nBlob)
      ){
        ਪ੍ਰਤਿਜ੍ਞਾ( !pParse->oom );
        nIns = ix.nBlob + n_ਕੁੰਜੀ + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        ਜੇ( !pParse->oom ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          ਪ੍ਰਤਿਜ੍ਞਾ( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pParse->aBlob[k], z_ਕੁੰਜੀ, n_ਕੁੰਜੀ);
          k += n_ਕੁੰਜੀ;
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pParse->aBlob[k], v.aBlob, v.nBlob);
          ਜੇ( ALWAYS(pParse->delta) ) json_After_Edit_ਮਾਪ_Adjust(pParse, i_ਜੜ੍ਹ);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      ਵਾਪਸ rc;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( z_ਰਸਤਾ[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_ਜੜ੍ਹ] & 0x0f;
    ਜੇ( x!=JSONB_ARRAY )  ਵਾਪਸ JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_ਮਾਪ(pParse, i_ਜੜ੍ਹ, &sz);
    i = 1;
    ਜਦੋਂ( sqlite3Isdigit(z_ਰਸਤਾ[i]) ){
      ਜੇ( kk<0xffffffff ) kk = kk*10 + z_ਰਸਤਾ[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    ਜੇ( i<2 || z_ਰਸਤਾ[i]!=']' ){
      ਜੇ( z_ਰਸਤਾ[1]=='#' ){
        kk = jsonb_Array_ਗਿਣਤੀ(pParse, i_ਜੜ੍ਹ);
        i = 2;
        ਜੇ( z_ਰਸਤਾ[2]=='-' && sqlite3Isdigit(z_ਰਸਤਾ[3]) ){
          u64 nn = 0;
          i = 3;
          ਕਰੋ{
            ਜੇ( nn<0xffffffff ) nn = nn*10 + z_ਰਸਤਾ[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }ਜਦੋਂ( sqlite3Isdigit(z_ਰਸਤਾ[i]) );
          ਜੇ( nn>kk ) ਵਾਪਸ JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        ਜੇ( z_ਰਸਤਾ[i]!=']' ){
          ਵਾਪਸ JSON_LOOKUP_PATHERROR;
        }
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_ਜੜ੍ਹ+n;
    i_ਸਿਰਾ = j+sz;
    ਜਦੋਂ( j<i_ਸਿਰਾ ){
      ਜੇ( kk==0 ){
        ਜੇ( ++pParse->i_ਡੂੰਘਾਈ >= JSON_MAX_ਡੂੰਘਾਈ ){
          ਵਾਪਸ JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_ਕਦਮ(pParse, j, &z_ਰਸਤਾ[i+1], 0);
        pParse->i_ਡੂੰਘਾਈ--;
        ਜੇ( pParse->delta ) json_After_Edit_ਮਾਪ_Adjust(pParse, i_ਜੜ੍ਹ);
        ਵਾਪਸ rc;
      }
      kk--;
      n = jsonb_Payload_ਮਾਪ_Semi_Inline(pParse, j, &sz);
      ਜੇ( n==0 ) ਵਾਪਸ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ਜੇ( j>i_ਸਿਰਾ ) ਵਾਪਸ JSON_LOOKUP_ERROR;
    ਜੇ( kk>0 ) ਵਾਪਸ JSON_LOOKUP_NOTFOUND;
    ਜੇ( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_ਸੈੱਟ );
      rc = jsonCreateEditSubstructure(pParse, &v, &z_ਰਸਤਾ[i+1]);
      ਜੇ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        ਪ੍ਰਤਿਜ੍ਞਾ( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      ਜੇ( pParse->delta ) json_After_Edit_ਮਾਪ_Adjust(pParse, i_ਜੜ੍ਹ);
      ਵਾਪਸ rc;
    }
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ JSON_LOOKUP_PATHERROR; 
  }
  ਵਾਪਸ JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
ਸਥਿਰ ਖਾਲੀ json_Return_ਪਾਠ_Json_From_Blob(
  sqlite3_context *ctx,
  ਅਚਲ u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  ਜੇ( NEVER(aBlob==0) ) ਵਾਪਸ;
  memset(&x, 0, ਆਕਾਰ(x));
  x.aBlob = (u8*)aBlob;
  x.nBlob = nBlob;
  jsonStringInit(&s, ctx);
  json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(&x, 0, &s);
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
ਸਥਿਰ ਖਾਲੀ jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  ਪੂਰਨ e_ਬਹੁਲਕ                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  ਪੂਰਨ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  ਪ੍ਰਤਿਜ੍ਞਾ( e_ਬਹੁਲਕ>=0 && e_ਬਹੁਲਕ<=2 );
  n = jsonb_Payload_ਮਾਪ(pParse, i, &sz);
  ਜੇ( n==0 ){
    sqlite3_ਨਤੀਜਾ_error(pCtx, "malformed JSON", -1);
    ਵਾਪਸ;
  }
  ਚੋਣ( pParse->aBlob[i] & 0x0f ){
    ਹਾਲਤ JSONB_NULL: {
      ਜੇ( sz ) ਜਾਓ returnfromblob_malformed;
      sqlite3_ਨਤੀਜਾ_null(pCtx);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_TRUE: {
      ਜੇ( sz ) ਜਾਓ returnfromblob_malformed;
      sqlite3_ਨਤੀਜਾ_int(pCtx, 1);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_FALSE: {
      ਜੇ( sz ) ਜਾਓ returnfromblob_malformed;
      sqlite3_ਨਤੀਜਾ_int(pCtx, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_INT5:
    ਹਾਲਤ JSONB_INT: {
      sqlite3_int64 iRes = 0;
      ਅੱਖਰ *z;
      ਪੂਰਨ bNeg = 0;
      ਅੱਖਰ x;
      ਜੇ( sz==0 ) ਜਾਓ returnfromblob_malformed;
      x = (ਅੱਖਰ)pParse->aBlob[i+n];
      ਜੇ( x=='-' ){
        ਜੇ( sz<2 ) ਜਾਓ returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n], (ਪੂਰਨ)sz);
      ਜੇ( z==0 ) ਜਾਓ returnfromblob_oom;
      rc = sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z, &iRes);
      sqlite3DbFree(db, z);
      ਜੇ( rc==0 ){
        ਜੇ( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          ਦੁੱਗਣਾ r;
          r = (ਦੁੱਗਣਾ)*(sqlite3_uint64*)&iRes;
          sqlite3_ਨਤੀਜਾ_double(pCtx, bNeg ? -r : r);
        }ਨਹੀਂਤਾਂ{
          sqlite3_ਨਤੀਜਾ_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }ਨਹੀਂਤਾਂ ਜੇ( rc==3 && bNeg ){
        sqlite3_ਨਤੀਜਾ_int64(pCtx, ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64);
      }ਨਹੀਂਤਾਂ ਜੇ( rc==1 ){
        ਜਾਓ returnfromblob_malformed;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( bNeg ){ n--; sz++; }
        ਜਾਓ ਮੰਜ਼ਿਲ_double;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_FLOAT5:
    ਹਾਲਤ JSONB_FLOAT: {
      ਦੁੱਗਣਾ r;
      ਅੱਖਰ *z;
      ਜੇ( sz==0 ) ਜਾਓ returnfromblob_malformed;
    ਮੰਜ਼ਿਲ_double:
      z = sqlite3DbStrNDup(db, (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n], (ਪੂਰਨ)sz);
      ਜੇ( z==0 ) ਜਾਓ returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      ਜੇ( rc<=0 ) ਜਾਓ returnfromblob_malformed;
      sqlite3_ਨਤੀਜਾ_double(pCtx, r);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_TEXTRAW:
    ਹਾਲਤ JSONB_ਪਾਠ: {
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(pCtx, (ਅੱਖਰ*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_TEXT5:
    ਹਾਲਤ JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 i_ਵਿੱਚ, i_ਬਾਹਰਲਾ;
      ਅਚਲ ਅੱਖਰ *z;
      ਅੱਖਰ *z_ਬਾਹਰਲਾ;
      u32 n_ਬਾਹਰਲਾ = sz;
      z = (ਅਚਲ ਅੱਖਰ*)&pParse->aBlob[i+n];
      z_ਬਾਹਰਲਾ = sqlite3DbMallocRaw(db, ((u64)n_ਬਾਹਰਲਾ)+1);
      ਜੇ( z_ਬਾਹਰਲਾ==0 ) ਜਾਓ returnfromblob_oom;
      ਲਈ(i_ਵਿੱਚ=i_ਬਾਹਰਲਾ=0; i_ਵਿੱਚ<sz; i_ਵਿੱਚ++){
        ਅੱਖਰ c = z[i_ਵਿੱਚ];
        ਜੇ( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[i_ਵਿੱਚ], sz-i_ਵਿੱਚ, &v);
          ਜੇ( v==0 && i_ਵਿੱਚ+szEscape==sz
           && (z[i_ਵਿੱਚ+1]=='\n' || z[i_ਵਿੱਚ+1]=='\r' || (u8)z[i_ਵਿੱਚ+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }ਨਹੀਂਤਾਂ ਜੇ( v<=0x7f ){
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = (ਅੱਖਰ)v;
          }ਨਹੀਂਤਾਂ ਜੇ( v<=0x7ff ){
            ਪ੍ਰਤਿਜ੍ਞਾ( szEscape>=2 );
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = (ਅੱਖਰ)(0xc0 | (v>>6));
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = 0x80 | (v&0x3f);
          }ਨਹੀਂਤਾਂ ਜੇ( v<0x10000 ){
            ਪ੍ਰਤਿਜ੍ਞਾ( szEscape>=3 );
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = 0xe0 | (v>>12);
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = 0x80 | ((v>>6)&0x3f);
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = 0x80 | (v&0x3f);
          }ਨਹੀਂਤਾਂ ਜੇ( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }ਨਹੀਂਤਾਂ{
            ਪ੍ਰਤਿਜ੍ਞਾ( szEscape>=4 );
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = 0xf0 | (v>>18);
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = 0x80 | ((v>>12)&0x3f);
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = 0x80 | ((v>>6)&0x3f);
            z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = 0x80 | (v&0x3f);
          }
          i_ਵਿੱਚ += szEscape - 1;
        }ਨਹੀਂਤਾਂ{
          z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ++] = c;
        }
      } /* end for() */
      ਪ੍ਰਤਿਜ੍ਞਾ( i_ਬਾਹਰਲਾ<=n_ਬਾਹਰਲਾ );
      z_ਬਾਹਰਲਾ[i_ਬਾਹਰਲਾ] = 0;
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(pCtx, z_ਬਾਹਰਲਾ, i_ਬਾਹਰਲਾ, SQLITE_DYNAMIC);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JSONB_ARRAY:
    ਹਾਲਤ JSONB_OBJECT: {
      ਜੇ( e_ਬਹੁਲਕ==0 ){
        ਜੇ( (SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(pCtx)) & JSON_BLOB)!=0 ){
          e_ਬਹੁਲਕ = 2;
        }ਨਹੀਂਤਾਂ{
          e_ਬਹੁਲਕ = 1;
        }
      }
      ਜੇ( e_ਬਹੁਲਕ==2 ){
        sqlite3_ਨਤੀਜਾ_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }ਨਹੀਂਤਾਂ{
        json_Return_ਪਾਠ_Json_From_Blob(pCtx, &pParse->aBlob[i], sz+n);
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      ਜਾਓ returnfromblob_malformed;
    }
  }
  ਵਾਪਸ;

returnfromblob_oom:
  sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
  ਵਾਪਸ;

returnfromblob_malformed:
  sqlite3_ਨਤੀਜਾ_error(pCtx, "malformed JSON", -1);
  ਵਾਪਸ;
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
ਸਥਿਰ ਪੂਰਨ json_Function_Arg_ਮੰਜ਼ਿਲ_Blob(
  sqlite3_context *ctx,
  sqlite3_ਮੁੱਲ *pArg,
  JsonParse *pParse
){
  ਪੂਰਨ eType = sqlite3_ਮੁੱਲ_type(pArg);
  ਸਥਿਰ u8 aNull[] = { 0x00 };
  memset(pParse, 0, ਆਕਾਰ(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  ਚੋਣ( eType ){
    ਮੂਲ: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      ਵਾਪਸ 0;
    }
    ਹਾਲਤ SQLITE_BLOB: {
      ਜੇ( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_ਨਤੀਜਾ_error(ctx, "JSON cannot hold BLOB values", -1);
        ਵਾਪਸ 1;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_ਪਾਠ: {
      ਅਚਲ ਅੱਖਰ *zJson = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pArg);
      ਪੂਰਨ nJson = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(pArg);
      ਜੇ( zJson==0 ) ਵਾਪਸ 1;
      ਜੇ( sqlite3_ਮੁੱਲ_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (ਅੱਖਰ*)zJson;
        pParse->nJson = nJson;
        ਜੇ( json_Convert_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(pParse, ctx) ){
          sqlite3_ਨਤੀਜਾ_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, ਆਕਾਰ(pParse[0]));
          ਵਾਪਸ 1;
        }
      }ਨਹੀਂਤਾਂ{
        json_Blob_Append_ਗੰਢ(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_FLOAT: {
      ਜੇ( NEVER(sqlite3IsNaN(sqlite3_ਮੁੱਲ_double(pArg))) ){
        json_Blob_Append_ਗੰਢ(pParse, JSONB_NULL, 0, 0);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(pArg);
        ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pArg);
        ਜੇ( z==0 ) ਵਾਪਸ 1;
        ਜੇ( z[0]=='I' ){
          json_Blob_Append_ਗੰਢ(pParse, JSONB_FLOAT, 5, "9e999");
        }ਨਹੀਂਤਾਂ ਜੇ( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_ਗੰਢ(pParse, JSONB_FLOAT, 6, "-9e999");
        }ਨਹੀਂਤਾਂ{
          json_Blob_Append_ਗੰਢ(pParse, JSONB_FLOAT, n, z);
        }
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_INTEGER: {
      ਪੂਰਨ n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(pArg);
      ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pArg);
      ਜੇ( z==0 ) ਵਾਪਸ 1;
      json_Blob_Append_ਗੰਢ(pParse, JSONB_INT, n, z);
      ਰੋਕੋ;
    }
  }
  ਜੇ( pParse->oom ){
    sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
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
ਸਥਿਰ ਅੱਖਰ *json_Bad_ਰਸਤਾ_Error(
  sqlite3_context *ctx,     /* The function call containing the error */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,        /* The path with the problem */
  ਪੂਰਨ rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  ਅੱਖਰ *z_ਸੁਨੇਹਾ;
  ਜੇ( rc==(ਪੂਰਨ)JSON_LOOKUP_NOTARRAY ){
    z_ਸੁਨੇਹਾ = sqlite3_mprintf("not an array element: %Q", z_ਰਸਤਾ);
  }ਨਹੀਂਤਾਂ ਜੇ( rc==(ਪੂਰਨ)JSON_LOOKUP_ERROR ){
    z_ਸੁਨੇਹਾ = sqlite3_mprintf("malformed JSON");
  }ਨਹੀਂਤਾਂ ਜੇ( rc==(ਪੂਰਨ)JSON_LOOKUP_TOODEEP ){
    z_ਸੁਨੇਹਾ = sqlite3_mprintf("JSON path too deep");
  }ਨਹੀਂਤਾਂ{
    z_ਸੁਨੇਹਾ = sqlite3_mprintf("bad JSON path: %Q", z_ਰਸਤਾ);
  }
  ਜੇ( ctx==0 ) ਵਾਪਸ z_ਸੁਨੇਹਾ;
  ਜੇ( z_ਸੁਨੇਹਾ ){
    sqlite3_ਨਤੀਜਾ_error(ctx, z_ਸੁਨੇਹਾ, -1);
    sqlite3_free(z_ਸੁਨੇਹਾ);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
  }
  ਵਾਪਸ 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
ਸਥਿਰ ਖਾਲੀ json_ਪਾਓ_Into_Blob(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv,
  ਪੂਰਨ eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  ਪੂਰਨ i;
  u32 rc = 0;
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ = 0;
  ਪੂਰਨ flgs;
  JsonParse *p;
  JsonParse ax;

  ਪ੍ਰਤਿਜ੍ਞਾ( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਲਈ(i=1; i<argc-1; i+=2){
    ਜੇ( sqlite3_ਮੁੱਲ_type(argv[i])==SQLITE_NULL ) ਜਾਰੀ;
    z_ਰਸਤਾ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
    ਜੇ( z_ਰਸਤਾ==0 ){
      sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
      jsonParseFree(p);
      ਵਾਪਸ;
    }
    ਜੇ( z_ਰਸਤਾ[0]!='$' ) ਜਾਓ json_ਪਾਓ_Into_Blob_patherror;
    ਜੇ( json_Function_Arg_ਮੰਜ਼ਿਲ_Blob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      ਵਾਪਸ;
    }
    ਜੇ( z_ਰਸਤਾ[1]==0 ){
      ਜੇ( eEdit==JEDIT_REPL || eEdit==JEDIT_ਸੈੱਟ ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }ਨਹੀਂਤਾਂ{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->i_ਡੂੰਘਾਈ = 0;
      rc = json_Lookup_ਕਦਮ(p, 0, z_ਰਸਤਾ+1, 0);
    }
    jsonParseReset(&ax);
    ਜੇ( rc==JSON_LOOKUP_NOTFOUND ) ਜਾਰੀ;
    ਜੇ( JSON_LOOKUP_ISERROR(rc) ) ਜਾਓ json_ਪਾਓ_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ਵਾਪਸ;

json_ਪਾਓ_Into_Blob_patherror:
  jsonParseFree(p);
  json_Bad_ਰਸਤਾ_Error(ctx, z_ਰਸਤਾ, rc);
  ਵਾਪਸ;
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
ਸਥਿਰ ਪੂਰਨ jsonArgIsJsonb(sqlite3_ਮੁੱਲ *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  ਜੇ( sqlite3_ਮੁੱਲ_type(pArg)!=SQLITE_BLOB ) ਵਾਪਸ 0;
  p->aBlob = (u8*)sqlite3_ਮੁੱਲ_blob(pArg);
  p->nBlob = (u32)sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(pArg);
  ਜੇ( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonb_Payload_ਮਾਪ(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    ਵਾਪਸ 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  ਵਾਪਸ 0;
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
ਸਥਿਰ JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_ਮੁੱਲ *pArg,
  u32 flgs
){
  ਪੂਰਨ eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  ਪ੍ਰਤਿਜ੍ਞਾ( ctx!=0 );
  eType = sqlite3_ਮੁੱਲ_type(pArg);
  ਜੇ( eType==SQLITE_NULL ){
    ਵਾਪਸ 0;
  }
  ਜੇ( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_ਖੋਜ(ctx, pArg);
  }
  ਜੇ( pFromCache ){
    pFromCache->nJPRef++;
    ਜੇ( (flgs & JSON_EDITABLE)==0 ){
      ਵਾਪਸ pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, ਆਕਾਰ(*p));
  ਜੇ( p==0 ) ਜਾਓ json_pfa_oom;
  memset(p, 0, ਆਕਾਰ(*p));
  p->db = db;
  p->nJPRef = 1;
  ਜੇ( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    ਜੇ( p->aBlob==0 ) ਜਾਓ json_pfa_oom;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    ਵਾਪਸ p;
  }
  ਜੇ( eType==SQLITE_BLOB ){
    ਜੇ( jsonArgIsJsonb(pArg,p) ){
      ਜੇ( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        ਜਾਓ json_pfa_oom;
      }
      ਵਾਪਸ p;
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
  p->zJson = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pArg);
  p->nJson = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(pArg);
  ਜੇ( db->mallocFailed ) ਜਾਓ json_pfa_oom;
  ਜੇ( p->nJson==0 ) ਜਾਓ json_pfa_malformed;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->zJson!=0 );
  ਜੇ( json_Convert_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    ਜੇ( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      ਵਾਪਸ p;
    }ਨਹੀਂਤਾਂ{
      jsonParseFree(p);
      ਵਾਪਸ 0;
    }
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ isRCStr = sqlite3_ਮੁੱਲ_Is_ਦਾ_Class(pArg, sqlite3RCStrUnref);
    ਪੂਰਨ rc;
    ਜੇ( !isRCStr ){
      ਅੱਖਰ *zNew = sqlite3RCStrNew( p->nJson );
      ਜੇ( zNew==0 ) ਜਾਓ json_pfa_oom;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }ਨਹੀਂਤਾਂ{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_ਪਾਓ(ctx, p);
    ਜੇ( rc==SQLITE_NOMEM ) ਜਾਓ json_pfa_oom;
    ਜੇ( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      ਜਾਓ rebuild_from_cache;
    }
  }
  ਵਾਪਸ p;

json_pfa_malformed:
  ਜੇ( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    ਵਾਪਸ p;
  }ਨਹੀਂਤਾਂ{
    jsonParseFree(p);
    sqlite3_ਨਤੀਜਾ_error(ctx, "malformed JSON", -1);
    ਵਾਪਸ 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
  ਵਾਪਸ 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
ਸਥਿਰ ਖਾਲੀ jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  ਪੂਰਨ flgs;
  ਜੇ( p->oom ){
    sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
    ਵਾਪਸ;
  }
  flgs = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(ctx));
  ਜੇ( flgs & JSON_BLOB ){
    ਜੇ( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_ਨਤੀਜਾ_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਨਤੀਜਾ_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }ਨਹੀਂਤਾਂ{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
  }
}

/****************************************************************************
** SQL functions used for testing and debugging
****************************************************************************/

#ਜੇ SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
ਸਥਿਰ ਖਾਲੀ jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 i_ਸ਼ੁਰੂ,        /* Start rendering here */
  u32 i_ਸਿਰਾ,          /* Do not render this byte or any byte after this one */
  ਪੂਰਨ nIndent,       /* Indent by this many spaces */
  sqlite3_str *p_ਬਾਹਰਲਾ  /* Generate output into this sqlite3_str object */
){
  ਜਦੋਂ( i_ਸ਼ੁਰੂ<i_ਸਿਰਾ ){
    u32 i, n, nn, sz = 0;
    ਪੂਰਨ showContent = 1;
    u8 x = pParse->aBlob[i_ਸ਼ੁਰੂ] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(p_ਬਾਹਰਲਾ, "%5d:%*s", i_ਸ਼ੁਰੂ, nIndent, "");
    ਜੇ( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonb_Payload_ਮਾਪ(pParse, i_ਸ਼ੁਰੂ, &sz);
    ਜੇ( nn==0 ) nn = 1;
    ਜੇ( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    ਲਈ(i=0; i<nn; i++){
      sqlite3_str_appendf(p_ਬਾਹਰਲਾ, " %02x", pParse->aBlob[i_ਸ਼ੁਰੂ+i]);
    }
    ਜੇ( n==0 ){
      sqlite3_str_appendf(p_ਬਾਹਰਲਾ, "   ERROR invalid node size\n");
      i_ਸ਼ੁਰੂ = n==0 ? i_ਸ਼ੁਰੂ+1 : i_ਸਿਰਾ;
      ਜਾਰੀ;
    }
    pParse->nBlob = savedNBlob;
    ਜੇ( i_ਸ਼ੁਰੂ+n+sz>i_ਸਿਰਾ ){
      i_ਸਿਰਾ = i_ਸ਼ੁਰੂ+n+sz;
      ਜੇ( i_ਸਿਰਾ>pParse->nBlob ){
        ਜੇ( pParse->nBlobAlloc>0 && i_ਸਿਰਾ>pParse->nBlobAlloc ){
          i_ਸਿਰਾ = pParse->nBlobAlloc;
        }ਨਹੀਂਤਾਂ{
          i_ਸਿਰਾ = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"  <-- ");
    ਚੋਣ( x ){
      ਹਾਲਤ JSONB_NULL:     sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"null"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_TRUE:     sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"true"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_FALSE:    sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"false"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_INT:      sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"int"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_INT5:     sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"int5"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_FLOAT:    sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"float"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_FLOAT5:   sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"float5"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_ਪਾਠ:     sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"text"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_TEXTJ:    sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"textj"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_TEXT5:    sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"text5"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_TEXTRAW:  sqlite3_str_appendall(p_ਬਾਹਰਲਾ,"textraw"); ਰੋਕੋ;
      ਹਾਲਤ JSONB_ARRAY: {
        sqlite3_str_appendf(p_ਬਾਹਰਲਾ,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_ਸ਼ੁਰੂ+n, i_ਸ਼ੁਰੂ+n+sz, nIndent+2, p_ਬਾਹਰਲਾ);
        showContent = 0;
        ਰੋਕੋ;
      }
      ਹਾਲਤ JSONB_OBJECT: {
        sqlite3_str_appendf(p_ਬਾਹਰਲਾ, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_ਸ਼ੁਰੂ+n, i_ਸ਼ੁਰੂ+n+sz, nIndent+2, p_ਬਾਹਰਲਾ);
        showContent = 0;
        ਰੋਕੋ;
      }
      ਮੂਲ: {
        sqlite3_str_appendall(p_ਬਾਹਰਲਾ, "ERROR: unknown node type\n");
        showContent = 0;
        ਰੋਕੋ;
      }
    }
    ਜੇ( showContent ){
      ਜੇ( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(p_ਬਾਹਰਲਾ, "\n", 1);
      }ਨਹੀਂਤਾਂ{
        u32 j;
        sqlite3_str_appendall(p_ਬਾਹਰਲਾ, ": \"");
        ਲਈ(j=i_ਸ਼ੁਰੂ+n; j<i_ਸ਼ੁਰੂ+n+sz; j++){
          u8 c = pParse->aBlob[j];
          ਜੇ( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(p_ਬਾਹਰਲਾ, (ਅੱਖਰ*)&c, 1);
        }
        sqlite3_str_append(p_ਬਾਹਰਲਾ, "\"\n", 2);
      }
    }
    i_ਸ਼ੁਰੂ += n + sz;
  }
}
ਸਥਿਰ ਖਾਲੀ jsonShowParse(JsonParse *pParse){
  sqlite3_str ਬਾਹਰਲਾ;
  ਅੱਖਰ zBuf[1000];
  ਜੇ( pParse==0 ){
    ਲਿਖੋ("NULL pointer\n");
    ਵਾਪਸ;
  }ਨਹੀਂਤਾਂ{
    ਲਿਖੋ("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    ਲਿਖੋ("nBlob = %u\n", pParse->nBlob);
    ਲਿਖੋ("delta = %d\n", pParse->delta);
    ਜੇ( pParse->nBlob==0 ) ਵਾਪਸ;
    ਲਿਖੋ("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&ਬਾਹਰਲਾ, 0, zBuf, ਆਕਾਰ(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &ਬਾਹਰਲਾ);
  ਲਿਖੋ("%s", sqlite3_str_ਮੁੱਲ(&ਬਾਹਰਲਾ));
  sqlite3_str_reset(&ਬਾਹਰਲਾ);
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** SQL function:   json_parse(JSON)
**
** Parse JSON using jsonParseFuncArg().  Return text that is a
** human-readable dump of the binary JSONB for the input parameter.
*/
ਸਥਿਰ ਖਾਲੀ jsonParseFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str ਬਾਹਰਲਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( argc>=1 );
  sqlite3StrAccumInit(&ਬਾਹਰਲਾ, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਜੇ( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &ਬਾਹਰਲਾ);
    sqlite3_ਨਤੀਜਾ_text64(ctx,ਬਾਹਰਲਾ.z_ਪਾਠ,ਬਾਹਰਲਾ.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }ਨਹੀਂਤਾਂ{
    jsonShowParse(p);
  }
  jsonParseFree(p);
  sqlite3_str_reset(&ਬਾਹਰਲਾ);
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

/****************************************************************************
** Scalar SQL function implementations
****************************************************************************/

/*
** Implementation of the json_quote(VALUE) function.  Return a JSON value
** corresponding to the SQL value input.  Mostly this means putting
** double-quotes around strings and returning the unquoted string "null"
** when given a NULL input.
*/
ਸਥਿਰ ਖਾਲੀ jsonQuoteFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonString jx;
  ਅਣਵਰਤਿਆ_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_ਮੁੱਲ(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
ਸਥਿਰ ਖਾਲੀ jsonArrayFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  ਲਈ(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_ਮੁੱਲ(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
ਸਥਿਰ ਖਾਲੀ json_Array_ਲੰਮਾਈ_Func(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਜੇ( argc==2 ){
    ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
    ਜੇ( z_ਰਸਤਾ==0 ){
      jsonParseFree(p);
      ਵਾਪਸ;
    }
    i = json_Lookup_ਕਦਮ(p, 0, z_ਰਸਤਾ[0]=='$' ? z_ਰਸਤਾ+1 : "@", 0);
    ਜੇ( JSON_LOOKUP_ISERROR(i) ){
      ਜੇ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ਨਹੀਂਤਾਂ{
        json_Bad_ਰਸਤਾ_Error(ctx, z_ਰਸਤਾ, i);
      }
      eErr = 1;
      i = 0;
    }
  }ਨਹੀਂਤਾਂ{
    i = 0;
  }
  ਜੇ( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_ਗਿਣਤੀ(p, i);
  }
  ਜੇ( !eErr ) sqlite3_ਨਤੀਜਾ_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
ਸਥਿਰ ਪੂਰਨ jsonAllAlphanum(ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ n){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  ਵਾਪਸ i==n;
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
ਸਥਿਰ ਖਾਲੀ json_ਕੱਢੋ_Func(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonParse *p = 0;      /* The parse */
  ਪੂਰਨ flags;             /* Flags associated with the function */
  ਪੂਰਨ i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  ਜੇ( argc<2 ) ਵਾਪਸ;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ਜੇ( p==0 ) ਵਾਪਸ;
  flags = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(ctx));
  jsonStringInit(&jx, ctx);
  ਜੇ( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  ਲਈ(i=1; i<argc; i++){
    /* With a single PATH argument */
    ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
    ਪੂਰਨ n_ਰਸਤਾ;
    u32 j;
    ਜੇ( z_ਰਸਤਾ==0 ) ਜਾਓ json_ਕੱਢੋ_error;
    n_ਰਸਤਾ = sqlite3Strlen30(z_ਰਸਤਾ);
    ਜੇ( z_ਰਸਤਾ[0]=='$' ){
      j = json_Lookup_ਕਦਮ(p, 0, z_ਰਸਤਾ+1, 0);
    }ਨਹੀਂਤਾਂ ਜੇ( (flags & JSON_ABPATH) ){
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
      ਜੇ( sqlite3_ਮੁੱਲ_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        ਜੇ( z_ਰਸਤਾ[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, z_ਰਸਤਾ, n_ਰਸਤਾ);
        jsonAppendRawNZ(&jx, "]", 2);
      }ਨਹੀਂਤਾਂ ਜੇ( jsonAllAlphanum(z_ਰਸਤਾ, n_ਰਸਤਾ) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, z_ਰਸਤਾ, n_ਰਸਤਾ);
      }ਨਹੀਂਤਾਂ ਜੇ( z_ਰਸਤਾ[0]=='[' && n_ਰਸਤਾ>=3 && z_ਰਸਤਾ[n_ਰਸਤਾ-1]==']' ){
        jsonAppendRaw(&jx, z_ਰਸਤਾ, n_ਰਸਤਾ);
      }ਨਹੀਂਤਾਂ{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, z_ਰਸਤਾ, n_ਰਸਤਾ);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = json_Lookup_ਕਦਮ(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }ਨਹੀਂਤਾਂ{
      json_Bad_ਰਸਤਾ_Error(ctx, z_ਰਸਤਾ, 0);
      ਜਾਓ json_ਕੱਢੋ_error;
    }
    ਜੇ( j<p->nBlob ){
      ਜੇ( argc==2 ){
        ਜੇ( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          ਪ੍ਰਤਿਜ੍ਞਾ( (flags & JSON_BLOB)==0 );
          sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
        }ਨਹੀਂਤਾਂ{
          jsonReturnFromBlob(p, j, ctx, 0);
          ਜੇ( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }ਨਹੀਂਤਾਂ{
        jsonAppendSeparator(&jx);
        json_Translate_Blob_ਮੰਜ਼ਿਲ_ਪਾਠ(p, j, &jx);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( j==JSON_LOOKUP_NOTFOUND ){
      ਜੇ( argc==2 ){
        ਜਾਓ json_ਕੱਢੋ_error;  /* Return NULL if not found */
      }ਨਹੀਂਤਾਂ{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }ਨਹੀਂਤਾਂ{
      json_Bad_ਰਸਤਾ_Error(ctx, z_ਰਸਤਾ, j);
      ਜਾਓ json_ਕੱਢੋ_error;
    }
  }
  ਜੇ( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    ਜੇ( (flags & JSON_BLOB)==0 ){
      sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_ਕੱਢੋ_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  ਵਾਪਸ;
}

/*
** Return codes for jsonMergePatch()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_ਮਿਲਾਓ_OK          0     /* Success */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_ਮਿਲਾਓ_BADTARGET   1     /* Malformed TARGET blob */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_ਮਿਲਾਓ_BADPATCH    2     /* Malformed PATCH blob */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_ਮਿਲਾਓ_OOM         3     /* Out-of-memory condition */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JSON_ਮਿਲਾਓ_TOODEEP     4     /* Nested too deep */

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
ਸਥਿਰ ਪੂਰਨ json_ਮਿਲਾਓ_Patch(
  JsonParse *p_ਨਿਸ਼ਾਨਾ,      /* The JSON parser that contains the TARGET */
  u32 i_ਨਿਸ਼ਾਨਾ,             /* Index of TARGET in pTarget->aBlob[] */
  ਅਚਲ JsonParse *pPatch, /* The PATCH */
  u32 iPatch,              /* Index of PATCH in pPatch->aBlob[] */
  u32 i_ਡੂੰਘਾਈ               /* Nesting depth */
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

  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਨਿਸ਼ਾਨਾ>=0 && i_ਨਿਸ਼ਾਨਾ<p_ਨਿਸ਼ਾਨਾ->nBlob );
  ਪ੍ਰਤਿਜ੍ਞਾ( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  ਜੇ( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 sz_ਨਿਸ਼ਾਨਾ;       /* Total size of the target, header+payload */
    n = jsonb_Payload_ਮਾਪ(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonb_Payload_ਮਾਪ(p_ਨਿਸ਼ਾਨਾ, i_ਨਿਸ਼ਾਨਾ, &sz);
    sz_ਨਿਸ਼ਾਨਾ = n+sz;
    jsonBlobEdit(p_ਨਿਸ਼ਾਨਾ, i_ਨਿਸ਼ਾਨਾ, sz_ਨਿਸ਼ਾਨਾ, pPatch->aBlob+iPatch, szPatch);
    ਵਾਪਸ p_ਨਿਸ਼ਾਨਾ->oom ? JSON_ਮਿਲਾਓ_OOM : JSON_ਮਿਲਾਓ_OK;  /* Line 03 */
  }
  x = p_ਨਿਸ਼ਾਨਾ->aBlob[i_ਨਿਸ਼ਾਨਾ] & 0x0f;
  ਜੇ( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonb_Payload_ਮਾਪ(p_ਨਿਸ਼ਾਨਾ, i_ਨਿਸ਼ਾਨਾ, &sz);
    jsonBlobEdit(p_ਨਿਸ਼ਾਨਾ, i_ਨਿਸ਼ਾਨਾ+n, sz, 0, 0);
    x = p_ਨਿਸ਼ਾਨਾ->aBlob[i_ਨਿਸ਼ਾਨਾ];
    p_ਨਿਸ਼ਾਨਾ->aBlob[i_ਨਿਸ਼ਾਨਾ] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonb_Payload_ਮਾਪ(pPatch, iPatch, &sz);
  ਜੇ( NEVER(n==0) ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonb_Payload_ਮਾਪ(p_ਨਿਸ਼ਾਨਾ, i_ਨਿਸ਼ਾਨਾ, &sz);
  ਜੇ( NEVER(n==0) ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADTARGET;
  iTStart = i_ਨਿਸ਼ਾਨਾ+n;
  iTEndBE = iTStart+sz;

  ਜਦੋਂ( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    ਜੇ( ePLabel<JSONB_ਪਾਠ || ePLabel>JSONB_TEXTRAW ){
      ਵਾਪਸ JSON_ਮਿਲਾਓ_BADPATCH;
    }
    nPLabel = jsonb_Payload_ਮਾਪ(pPatch, iPCursor, &szPLabel);
    ਜੇ( nPLabel==0 ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    ਜੇ( iPValue>=iPEnd ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADPATCH;
    nPValue = jsonb_Payload_ਮਾਪ(pPatch, iPValue, &szPValue);
    ਜੇ( nPValue==0 ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    ਜੇ( iPCursor>iPEnd ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + p_ਨਿਸ਼ਾਨਾ->delta;
    ਜਦੋਂ( iTCursor<iTEnd ){
      ਪੂਰਨ isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = p_ਨਿਸ਼ਾਨਾ->aBlob[iTCursor] & 0x0f;
      ਜੇ( eTLabel<JSONB_ਪਾਠ || eTLabel>JSONB_TEXTRAW ){
        ਵਾਪਸ JSON_ਮਿਲਾਓ_BADTARGET;
      }
      nTLabel = jsonb_Payload_ਮਾਪ(p_ਨਿਸ਼ਾਨਾ, iTCursor, &szTLabel);
      ਜੇ( nTLabel==0 ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      ਜੇ( iTValue>=iTEnd ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADTARGET;
      nTValue = jsonb_Payload_ਮਾਪ(p_ਨਿਸ਼ਾਨਾ, iTValue, &szTValue);
      ਜੇ( nTValue==0 ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADTARGET;
      ਜੇ( iTValue + nTValue + szTValue > iTEnd ) ਵਾਪਸ JSON_ਮਿਲਾਓ_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ਅਚਲ ਅੱਖਰ*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_ਪਾਠ || ePLabel==JSONB_TEXTRAW),
                   (ਅਚਲ ਅੱਖਰ*)&p_ਨਿਸ਼ਾਨਾ->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_ਪਾਠ || eTLabel==JSONB_TEXTRAW));
      ਜੇ( isEqual ) ਰੋਕੋ;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    ਜੇ( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      ਜੇ( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(p_ਨਿਸ਼ਾਨਾ, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        ਜੇ( NEVER(p_ਨਿਸ਼ਾਨਾ->oom) ) ਵਾਪਸ JSON_ਮਿਲਾਓ_OOM;
      }ਨਹੀਂਤਾਂ{
        /* Algorithm line 12 */
        ਪੂਰਨ rc, savedDelta = p_ਨਿਸ਼ਾਨਾ->delta;
        p_ਨਿਸ਼ਾਨਾ->delta = 0;
        ਜੇ( i_ਡੂੰਘਾਈ>=JSON_MAX_ਡੂੰਘਾਈ ) ਵਾਪਸ JSON_ਮਿਲਾਓ_TOODEEP;
        rc = json_ਮਿਲਾਓ_Patch(p_ਨਿਸ਼ਾਨਾ, iTValue, pPatch, iPValue, i_ਡੂੰਘਾਈ+1);
        ਜੇ( rc ) ਵਾਪਸ rc;
        p_ਨਿਸ਼ਾਨਾ->delta += savedDelta;
      }        
    }ਨਹੀਂਤਾਂ ਜੇ( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      ਜੇ( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(p_ਨਿਸ਼ਾਨਾ, iTEnd, 0, 0, szPValue+nPValue+szNew);
        ਜੇ( p_ਨਿਸ਼ਾਨਾ->oom ) ਵਾਪਸ JSON_ਮਿਲਾਓ_OOM;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p_ਨਿਸ਼ਾਨਾ->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p_ਨਿਸ਼ਾਨਾ->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ rc, savedDelta;
        jsonBlobEdit(p_ਨਿਸ਼ਾਨਾ, iTEnd, 0, 0, szNew+1);
        ਜੇ( p_ਨਿਸ਼ਾਨਾ->oom ) ਵਾਪਸ JSON_ਮਿਲਾਓ_OOM;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p_ਨਿਸ਼ਾਨਾ->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        p_ਨਿਸ਼ਾਨਾ->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = p_ਨਿਸ਼ਾਨਾ->delta;
        p_ਨਿਸ਼ਾਨਾ->delta = 0;
        ਜੇ( i_ਡੂੰਘਾਈ>=JSON_MAX_ਡੂੰਘਾਈ ) ਵਾਪਸ JSON_ਮਿਲਾਓ_TOODEEP;
        rc = json_ਮਿਲਾਓ_Patch(p_ਨਿਸ਼ਾਨਾ, iTEnd+szNew,pPatch,iPValue,i_ਡੂੰਘਾਈ+1);
        ਜੇ( rc ) ਵਾਪਸ rc;
        p_ਨਿਸ਼ਾਨਾ->delta += savedDelta;
      }
    }
  }
  ਜੇ( p_ਨਿਸ਼ਾਨਾ->delta ) json_After_Edit_ਮਾਪ_Adjust(p_ਨਿਸ਼ਾਨਾ, i_ਨਿਸ਼ਾਨਾ);
  ਵਾਪਸ p_ਨਿਸ਼ਾਨਾ->oom ? JSON_ਮਿਲਾਓ_OOM : JSON_ਮਿਲਾਓ_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
ਸਥਿਰ ਖਾਲੀ jsonPatchFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonParse *p_ਨਿਸ਼ਾਨਾ;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  ਪੂਰਨ rc;                /* Result code */

  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==2 );
  p_ਨਿਸ਼ਾਨਾ = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  ਜੇ( p_ਨਿਸ਼ਾਨਾ==0 ) ਵਾਪਸ;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  ਜੇ( pPatch ){
    rc = json_ਮਿਲਾਓ_Patch(p_ਨਿਸ਼ਾਨਾ, 0, pPatch, 0, 0);
    ਜੇ( rc==JSON_ਮਿਲਾਓ_OK ){
      jsonReturnParse(ctx, p_ਨਿਸ਼ਾਨਾ);
    }ਨਹੀਂਤਾਂ ਜੇ( rc==JSON_ਮਿਲਾਓ_OOM ){
      sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
    }ਨਹੀਂਤਾਂ ਜੇ( rc==JSON_ਮਿਲਾਓ_TOODEEP ){
      sqlite3_ਨਤੀਜਾ_error(ctx, "JSON nested too deep", -1);
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਨਤੀਜਾ_error(ctx, "malformed JSON", -1);
    }
    jsonParseFree(pPatch);
  }
  jsonParseFree(p_ਨਿਸ਼ਾਨਾ);
}


/*
** Implementation of the json_object(NAME,VALUE,...) function.  Return a JSON
** object that contains all name/value given in arguments.  Or if any name
** is not a string or if any value is a BLOB, throw an error.
*/
ਸਥਿਰ ਖਾਲੀ jsonObjectFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ i;
  JsonString jx;
  ਅਚਲ ਅੱਖਰ *z;
  u32 n;

  ਜੇ( argc&1 ){
    sqlite3_ਨਤੀਜਾ_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    ਵਾਪਸ;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  ਲਈ(i=0; i<argc; i+=2){
    ਜੇ( sqlite3_ਮੁੱਲ_type(argv[i])!=SQLITE_ਪਾਠ ){
      sqlite3_ਨਤੀਜਾ_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      ਵਾਪਸ;
    }
    jsonAppendSeparator(&jx);
    z = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
    n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_ਮੁੱਲ(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
}


/*
** json_remove(JSON, PATH, ...)
**
** Remove the named elements from JSON and return the result.  malformed
** JSON or PATH arguments result in an error.
*/
ਸਥਿਰ ਖਾਲੀ jsonRemoveFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonParse *p;          /* The parse */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ = 0; /* Path of element to be removed */
  ਪੂਰਨ i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  ਜੇ( argc<1 ) ਵਾਪਸ;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਲਈ(i=1; i<argc; i++){
    z_ਰਸਤਾ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
    ਜੇ( z_ਰਸਤਾ==0 ){
      ਜਾਓ json_remove_ਪੂਰਾ;
    }
    ਜੇ( z_ਰਸਤਾ[0]!='$' ){
      ਜਾਓ json_remove_patherror;
    }
    ਜੇ( z_ਰਸਤਾ[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      ਜਾਓ json_remove_ਪੂਰਾ;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = json_Lookup_ਕਦਮ(p, 0, z_ਰਸਤਾ+1, 0);
    ਜੇ( JSON_LOOKUP_ISERROR(rc) ){
      ਜੇ( rc==JSON_LOOKUP_NOTFOUND ){
        ਜਾਰੀ;  /* No-op */
      }ਨਹੀਂਤਾਂ{
        json_Bad_ਰਸਤਾ_Error(ctx, z_ਰਸਤਾ, rc);
      }
      ਜਾਓ json_remove_ਪੂਰਾ;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ਵਾਪਸ;

json_remove_patherror:
  json_Bad_ਰਸਤਾ_Error(ctx, z_ਰਸਤਾ, 0);

json_remove_ਪੂਰਾ:
  jsonParseFree(p);
  ਵਾਪਸ;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
ਸਥਿਰ ਖਾਲੀ jsonReplaceFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਜੇ( argc<1 ) ਵਾਪਸ;
  ਜੇ( (argc&1)==0 ) {
    json_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(ctx, "replace");
    ਵਾਪਸ;
  }
  json_ਪਾਓ_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
ਸਥਿਰ ਖਾਲੀ json_ਸੈੱਟ_Func(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ flags = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(ctx));
  ਪੂਰਨ eInsType = JSON_ਪਾਓ_TYPE(flags);
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azInsType[] = { "insert", "set", "array_insert" };
  ਸਥਿਰ ਅਚਲ u8 aEditType[] = { JEDIT_INS, JEDIT_ਸੈੱਟ, JEDIT_AINS };

  ਜੇ( argc<1 ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( eInsType>=0 && eInsType<=2 );
  ਜੇ( (argc&1)==0 ) {
    json_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(ctx, azInsType[eInsType]);
    ਵਾਪਸ;
  }
  json_ਪਾਓ_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
ਸਥਿਰ ਖਾਲੀ jsonTypeFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonParse *p;          /* The parse */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਜੇ( argc==2 ){
    z_ਰਸਤਾ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
    ਜੇ( z_ਰਸਤਾ==0 ) ਜਾਓ json_type_ਪੂਰਾ;
    ਜੇ( z_ਰਸਤਾ[0]!='$' ){
      json_Bad_ਰਸਤਾ_Error(ctx, z_ਰਸਤਾ, 0);
      ਜਾਓ json_type_ਪੂਰਾ;
    }
    i = json_Lookup_ਕਦਮ(p, 0, z_ਰਸਤਾ+1, 0);
    ਜੇ( JSON_LOOKUP_ISERROR(i) ){
      ਜੇ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ਨਹੀਂਤਾਂ{
        json_Bad_ਰਸਤਾ_Error(ctx, z_ਰਸਤਾ, i);
      }
      ਜਾਓ json_type_ਪੂਰਾ;
    }
  }ਨਹੀਂਤਾਂ{
    i = 0;
  }
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
json_type_ਪੂਰਾ:
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
ਸਥਿਰ ਖਾਲੀ jsonPrettyFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, ਆਕਾਰ(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  ਜੇ( x.pParse==0 ) ਵਾਪਸ;
  x.p_ਬਾਹਰਲਾ = &s;
  jsonStringInit(&s, ctx);
  ਜੇ( argc==1 || (x.zIndent = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }ਨਹੀਂਤਾਂ{
    x.szIndent = (u32)ਲੜੀ_ਲੰਬਾਈ(x.zIndent);
  }
  json_Translate_Blob_ਮੰਜ਼ਿਲ_Pretty_ਪਾਠ(&x, 0);
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
ਸਥਿਰ ਖਾਲੀ jsonValidFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  ਜੇ( argc==2 ){
    i64 f = sqlite3_ਮੁੱਲ_int64(argv[1]);
    ਜੇ( f<1 || f>15 ){
      sqlite3_ਨਤੀਜਾ_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      ਵਾਪਸ;
    }
    flags = f & 0x0f;
  }
  ਚੋਣ( sqlite3_ਮੁੱਲ_type(argv[0]) ){
    ਹਾਲਤ SQLITE_NULL: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_ਨਤੀਜਾ_int(ctx, 0);
#ਜੇ_ਅੰਤ
      ਵਾਪਸ;
    }
    ਹਾਲਤ SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, ਆਕਾਰ(py));
      ਜੇ( jsonArgIsJsonb(argv[0], &py) ){
        ਜੇ( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }ਨਹੀਂਤਾਂ ਜੇ( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        ਰੋਕੋ;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    ਮੂਲ: {
      JsonParse px;
      ਜੇ( (flags & 0x3)==0 ) ਰੋਕੋ;
      memset(&px, 0, ਆਕਾਰ(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      ਜੇ( p ){
        ਜੇ( p->oom ){
          sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
        }ਨਹੀਂਤਾਂ ਜੇ( p->nErr ){
          /* no-op */
        }ਨਹੀਂਤਾਂ ਜੇ( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
      }
      ਰੋਕੋ;
    }
  }
  sqlite3_ਨਤੀਜਾ_int(ctx, res);
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
ਸਥਿਰ ਖਾਲੀ jsonErrorFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  memset(&s, 0, ਆਕਾਰ(s));
  s.db = sqlite3_context_db_handle(ctx);
  ਜੇ( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }ਨਹੀਂਤਾਂ{
    s.zJson = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    ਜੇ( s.zJson==0 ) ਵਾਪਸ;  /* NULL input or OOM */
    s.nJson = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
    ਜੇ( json_Convert_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(&s,0) ){
      ਜੇ( s.oom ){
        iErrPos = -1;
      }ਨਹੀਂਤਾਂ{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        ਪ੍ਰਤਿਜ੍ਞਾ( s.zJson!=0 );  /* Because s.oom is false */
        ਲਈ(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          ਜੇ( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  ਜੇ( iErrPos<0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_int64(ctx, iErrPos);
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
ਸਥਿਰ ਖਾਲੀ json_Array_ਕਦਮ(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonString *pStr;
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, ਆਕਾਰ(*pStr));
  ਜੇ( pStr ){
    ਜੇ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }ਨਹੀਂਤਾਂ ਜੇ( pStr->n_ਵਰਤਿਆ>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_ਮੁੱਲ(pStr, argv[0]);
  }
}
ਸਥਿਰ ਖਾਲੀ jsonArrayCompute(sqlite3_context *ctx, ਪੂਰਨ isFinal){
  JsonString *pStr;
  ਪੂਰਨ flags = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ਜੇ( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    ਜੇ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ਵਾਪਸ;
    }ਨਹੀਂਤਾਂ ਜੇ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ਜੇ( isFinal ){
        ਜੇ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ਨਹੀਂਤਾਂ{
        jsonStringTrimOneChar(pStr);
      }
      ਵਾਪਸ;
    }ਨਹੀਂਤਾਂ ਜੇ( isFinal ){
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pStr->zBuf, (ਪੂਰਨ)pStr->n_ਵਰਤਿਆ,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pStr->zBuf, (ਪੂਰਨ)pStr->n_ਵਰਤਿਆ, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( flags & JSON_BLOB ){
    ਸਥਿਰ ਅਚਲ u8 ਖਾਲੀ_ਥਾਵਾਂ_Array = 0x0b;
    sqlite3_ਨਤੀਜਾ_blob(ctx, &ਖਾਲੀ_ਥਾਵਾਂ_Array, 1, SQLITE_STATIC);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
}
ਸਥਿਰ ਖਾਲੀ json_Array_ਮੁੱਲ(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
ਸਥਿਰ ਖਾਲੀ jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
ਸਥਿਰ ਖਾਲੀ json_ਸਮੂਹ_Inverse(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਪੂਰਨ ਵਿੱਚ_Str = 0;
  ਪੂਰਨ nNest = 0;
  ਅੱਖਰ *z;
  ਅੱਖਰ c;
  JsonString *pStr;
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਅਣਵਰਤਿਆ_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  ਜੇ( NEVER(!pStr) ) ਵਾਪਸ;
  z = pStr->zBuf;
  ਲਈ(i=1; i<pStr->n_ਵਰਤਿਆ && ((c = z[i])!=',' || ਵਿੱਚ_Str || nNest); i++){
    ਜੇ( c=='"' ){
      ਵਿੱਚ_Str = !ਵਿੱਚ_Str;
    }ਨਹੀਂਤਾਂ ਜੇ( c=='\\' ){
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( !ਵਿੱਚ_Str ){
      ਜੇ( c=='{' || c=='[' ) nNest++;
      ਜੇ( c=='}' || c==']' ) nNest--;
    }
  }
  ਜੇ( i<pStr->n_ਵਰਤਿਆ ){
    pStr->n_ਵਰਤਿਆ -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->n_ਵਰਤਿਆ-1);
    z[pStr->n_ਵਰਤਿਆ] = 0;
  }ਨਹੀਂਤਾਂ{
    pStr->n_ਵਰਤਿਆ = 1;
  }
}
#ਨਹੀਂਤਾਂ
# define json_ਸਮੂਹ_Inverse 0
#ਜੇ_ਅੰਤ


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
ਸਥਿਰ ਖਾਲੀ json_Object_ਕਦਮ(
  sqlite3_context *ctx,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  JsonString *pStr;
  ਅਚਲ ਅੱਖਰ *z;
  u32 n;
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, ਆਕਾਰ(*pStr));
  ਜੇ( pStr ){
    z = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    n = sqlite3Strlen30(z);
    ਜੇ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }ਨਹੀਂਤਾਂ ਜੇ( pStr->n_ਵਰਤਿਆ>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    ਜੇ( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_ਮੁੱਲ(pStr, argv[1]);
    }ਨਹੀਂਤਾਂ{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
ਸਥਿਰ ਖਾਲੀ jsonObjectCompute(sqlite3_context *ctx, ਪੂਰਨ isFinal){
  JsonString *pStr;
  ਪੂਰਨ flags = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(sqlite3_user_ਅੰਕੜੇ(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ਜੇ( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    ਜੇ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ਵਾਪਸ;
    }
    ਜੇ( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      ਪੂਰਨ ਵਿੱਚ_Str = 0;
      ਜੇ( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->n_ਵਰਤਿਆ+1);
        pStr = &tmpStr;
        ਜੇ( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          ਵਾਪਸ;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      ਲਈ(i=j=1; i<pStr->n_ਵਰਤਿਆ; i++){
        ਅੱਖਰ c = pStr->zBuf[i];
        ਜੇ( c=='"' ){
          ਵਿੱਚ_Str = !ਵਿੱਚ_Str;
          pStr->zBuf[j++] = '"';
        }ਨਹੀਂਤਾਂ ਜੇ( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }ਨਹੀਂਤਾਂ ਜੇ( c=='@' && !ਵਿੱਚ_Str ){
          ਪ੍ਰਤਿਜ੍ਞਾ( i+1<pStr->n_ਵਰਤਿਆ );
          ਜੇ( pStr->zBuf[i+1]==',' ){
            i++;
          }ਨਹੀਂਤਾਂ ਜੇ( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }ਨਹੀਂਤਾਂ{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->n_ਵਰਤਿਆ = j;    /* Truncate the string */
    }
    ਜੇ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ਜੇ( isFinal ){
        ਜੇ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ਨਹੀਂਤਾਂ{
        jsonStringTrimOneChar(pOgStr);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( isFinal ){
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pStr->zBuf, (ਪੂਰਨ)pStr->n_ਵਰਤਿਆ,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pStr->zBuf, (ਪੂਰਨ)pStr->n_ਵਰਤਿਆ, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    ਜੇ( pStr!=pOgStr ) jsonStringReset(pStr);
  }ਨਹੀਂਤਾਂ ਜੇ( flags & JSON_BLOB ){
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਖਾਲੀ_ਥਾਵਾਂ_Object = 0x0c;
    sqlite3_ਨਤੀਜਾ_blob(ctx, &ਖਾਲੀ_ਥਾਵਾਂ_Object, 1, SQLITE_STATIC); 
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
}
ਸਥਿਰ ਖਾਲੀ json_Object_ਮੁੱਲ(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
ਸਥਿਰ ਖਾਲੀ jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Json_ਮਾਪਾ Json_ਮਾਪਾ;
ਬਣਤਰ Json_ਮਾਪਾ {
  u32 i_ਸਿਰ;                 /* Start of object or array */
  u32 i_ਮੁੱਲ;                /* Start of the value */
  u32 i_ਸਿਰਾ;                  /* First byte past the end */
  u32 n_ਰਸਤਾ;                 /* Length of path */
  i64 i_ਕੁੰਜੀ;                  /* Key for JSONB_ARRAY */
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ JsonEachCursor JsonEachCursor;
ਬਣਤਰ JsonEachCursor {
  sqlite3_vtab_cursor ਅਧਾਰ;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 i_ਸਿਰਾ;                  /* EOF when i equals or exceeds this value */
  u32 n_ਜੜ੍ਹ;                 /* Size of the root path in bytes */
  u8 eType;                  /* Type of the container for element i */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
  u8 e_ਬਹੁਲਕ;                  /* 1 for json_each().  2 for jsonb_each() */
  u32 n_ਮਾਪਾ;               /* Current nesting depth */
  u32 n_ਮਾਪਾ_Alloc;          /* Space allocated for aParent[] */
  Json_ਮਾਪਾ *a_ਮਾਪਾ;       /* Parent elements of i */
  sqlite3 *db;               /* Database connection */
  JsonString ਰਸਤਾ;           /* Current path */
  JsonParse sParse;          /* Parse of the input JSON */
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ JsonEachConnection JsonEachConnection;
ਬਣਤਰ JsonEachConnection {
  sqlite3_vtab ਅਧਾਰ;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 e_ਬਹੁਲਕ;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
ਸਥਿਰ ਪੂਰਨ jsonEachConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  JsonEachConnection *pNew;
  ਪੂਰਨ rc;

/* Column numbers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_ਕੁੰਜੀ     0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_ਮੁੱਲ   1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_TYPE    2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_ATOM    3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_ID      4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_ਮਾਪਾ  5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_FULLKEY 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_ਰਸਤਾ    7
/* The xBestIndex method assumes that the JSON and ROOT columns are
** the last two columns in the table.  Should this ever changes, be
** sure to update the xBestIndex method. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_JSON    8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JEACH_ਜੜ੍ਹ    9

  ਅਣਵਰਤਿਆ_PARAMETER(pzErr);
  ਅਣਵਰਤਿਆ_PARAMETER(argv);
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਅਣਵਰਤਿਆ_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  ਜੇ( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, ਆਕਾਰ(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    ਜੇ( pNew==0 ) ਵਾਪਸ SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->e_ਬਹੁਲਕ = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->e_ਬਹੁਲਕ]=='t';
  }
  ਵਾਪਸ rc;
}

/* destructor for json_each virtual table */
ਸਥਿਰ ਪੂਰਨ jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  ਵਾਪਸ SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
ਸਥਿਰ ਪੂਰਨ jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  ਅਣਵਰਤਿਆ_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, ਆਕਾਰ(*pCur));
  ਜੇ( pCur==0 ) ਵਾਪਸ SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->e_ਬਹੁਲਕ = pVtab->e_ਬਹੁਲਕ;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->ਰਸਤਾ);
  *ppCursor = &pCur->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
ਸਥਿਰ ਖਾਲੀ jsonEachCursorReset(JsonEachCursor *p){
  jsonParseReset(&p->sParse);
  jsonStringReset(&p->ਰਸਤਾ);
  sqlite3DbFree(p->db, p->a_ਮਾਪਾ);
  p->iRowid = 0;
  p->i = 0;
  p->a_ਮਾਪਾ = 0;
  p->n_ਮਾਪਾ = 0;
  p->n_ਮਾਪਾ_Alloc = 0;
  p->i_ਸਿਰਾ = 0;
  p->eType = 0;
}

/* Destructor for a jsonEachCursor object */
ਸਥਿਰ ਪੂਰਨ jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  ਵਾਪਸ SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
ਸਥਿਰ ਪੂਰਨ jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ਵਾਪਸ p->i >= p->i_ਸਿਰਾ;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
ਸਥਿਰ ਪੂਰਨ jsonSkipLabel(JsonEachCursor *p){
  ਜੇ( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonb_Payload_ਮਾਪ(&p->sParse, p->i, &sz);
    sz += p->i + n;
    ਜੇ( sz >= p->sParse.nBlob ) sz = p->i;
    ਵਾਪਸ sz;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ p->i;
  }
}

/*
** Append the path name for the current element.
*/
ਸਥਿਰ ਖਾਲੀ json_Append_ਰਸਤਾ_Name(JsonEachCursor *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਮਾਪਾ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  ਜੇ( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->ਰਸਤਾ, "[%lld]", p->a_ਮਾਪਾ[p->n_ਮਾਪਾ-1].i_ਕੁੰਜੀ);
  }ਨਹੀਂਤਾਂ{
    u32 n, sz = 0, k, i;
    ਅਚਲ ਅੱਖਰ *z;
    ਪੂਰਨ needQuote = 0;
    n = jsonb_Payload_ਮਾਪ(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (ਅਚਲ ਅੱਖਰ*)&p->sParse.aBlob[k];
    ਜੇ( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }ਨਹੀਂਤਾਂ{
      ਲਈ(i=0; i<sz; i++){
        ਜੇ( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          ਰੋਕੋ;
        }
      }
    }
    ਜੇ( needQuote ){
      jsonPrintf(sz+4,&p->ਰਸਤਾ,".\"%.*s\"", sz, z);
    }ਨਹੀਂਤਾਂ{
      jsonPrintf(sz+2,&p->ਰਸਤਾ,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
ਸਥਿਰ ਪੂਰਨ json_Each_Malformed_ਨਿਵੇਸ਼(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_ਸੁਨੇਹਾ);
  cur->pVtab->z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  ਵਾਪਸ cur->pVtab->z_Err_ਸੁਨੇਹਾ ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
ਸਥਿਰ ਪੂਰਨ json_Each_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonb_Payload_ਮਾਪ(&p->sParse, i, &sz);
    ਜੇ( n==0 )ਵਾਪਸ json_Each_Malformed_ਨਿਵੇਸ਼(cur);
    ਜੇ( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      Json_ਮਾਪਾ *p_ਮਾਪਾ;
      ਜੇ( p->n_ਮਾਪਾ>=p->n_ਮਾਪਾ_Alloc ){
        Json_ਮਾਪਾ *pNew;
        u64 nNew;
        nNew = p->n_ਮਾਪਾ_Alloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->a_ਮਾਪਾ, ਆਕਾਰ(Json_ਮਾਪਾ)*nNew);
        ਜੇ( pNew==0 ) ਵਾਪਸ SQLITE_NOMEM;
        p->n_ਮਾਪਾ_Alloc = (u32)nNew;
        p->a_ਮਾਪਾ = pNew;
      }
      levelChange = 1;
      p_ਮਾਪਾ = &p->a_ਮਾਪਾ[p->n_ਮਾਪਾ];
      p_ਮਾਪਾ->i_ਸਿਰ = p->i;
      p_ਮਾਪਾ->i_ਮੁੱਲ = i;
      p_ਮਾਪਾ->i_ਸਿਰਾ = i + n + sz;
      p_ਮਾਪਾ->i_ਕੁੰਜੀ = -1;
      p_ਮਾਪਾ->n_ਰਸਤਾ = (u32)p->ਰਸਤਾ.n_ਵਰਤਿਆ;
      ਜੇ( p->eType && p->n_ਮਾਪਾ ){
        json_Append_ਰਸਤਾ_Name(p);
        ਜੇ( p->ਰਸਤਾ.eErr ) rc = SQLITE_NOMEM;
      }
      p->n_ਮਾਪਾ++;
      p->i = i + n;
    }ਨਹੀਂਤਾਂ{
      p->i = i + n + sz;
    }
    ਜਦੋਂ( p->n_ਮਾਪਾ>0 && p->i >= p->a_ਮਾਪਾ[p->n_ਮਾਪਾ-1].i_ਸਿਰਾ ){
      p->n_ਮਾਪਾ--;
      p->ਰਸਤਾ.n_ਵਰਤਿਆ = p->a_ਮਾਪਾ[p->n_ਮਾਪਾ].n_ਰਸਤਾ;
      levelChange = 1;
    }
    ਜੇ( levelChange ){
      ਜੇ( p->n_ਮਾਪਾ>0 ){
        Json_ਮਾਪਾ *p_ਮਾਪਾ = &p->a_ਮਾਪਾ[p->n_ਮਾਪਾ-1];
        u32 iVal = p_ਮਾਪਾ->i_ਮੁੱਲ;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }ਨਹੀਂਤਾਂ{
        p->eType = 0;
      }
    }
  }ਨਹੀਂਤਾਂ{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonb_Payload_ਮਾਪ(&p->sParse, i, &sz);
    ਜੇ( n==0 )ਵਾਪਸ json_Each_Malformed_ਨਿਵੇਸ਼(cur);
    p->i = i + n + sz;
  }
  ਜੇ( p->eType==JSONB_ARRAY && p->n_ਮਾਪਾ ){
    p->a_ਮਾਪਾ[p->n_ਮਾਪਾ-1].i_ਕੁੰਜੀ++;
  }
  p->iRowid++;
  ਵਾਪਸ rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
ਸਥਿਰ ਪੂਰਨ json_Each_ਰਸਤਾ_ਲੰਮਾਈ(JsonEachCursor *p){
  u32 n = p->ਰਸਤਾ.n_ਵਰਤਿਆ;
  ਅੱਖਰ *z = p->ਰਸਤਾ.zBuf;
  ਜੇ( p->iRowid==0 && p->bRecursive && n>=2 ){
    ਜਦੋਂ( ALWAYS(n>1) ){
      n--;
      ਜੇ( z[n]=='[' || z[n]=='.' ){
        u32 x;
        ਅੱਖਰ cSaved = z[n];
        z[n] = 0;
        ਪ੍ਰਤਿਜ੍ਞਾ( p->sParse.eEdit==0 );
        x = json_Lookup_ਕਦਮ(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        ਜੇ( !JSON_LOOKUP_ISERROR(x) ) ਰੋਕੋ;
      }
    }
  }
  ਵਾਪਸ n;
}

/* Return the value of a column */
ਸਥਿਰ ਪੂਰਨ json_Each_ਕਾਲਮ(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ਪੂਰਨ i_ਕਾਲਮ                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ਚੋਣ( i_ਕਾਲਮ ){
    ਹਾਲਤ JEACH_ਕੁੰਜੀ: {
      ਜੇ( p->n_ਮਾਪਾ==0 ){
        u32 n, j;
        ਜੇ( p->n_ਜੜ੍ਹ==1 ) ਰੋਕੋ;
        j = json_Each_ਰਸਤਾ_ਲੰਮਾਈ(p);
        n = p->n_ਜੜ੍ਹ - j;
        ਜੇ( n==0 ){
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ ਜੇ( p->ਰਸਤਾ.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->ਰਸਤਾ.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_ਨਤੀਜਾ_int64(ctx, x);
        }ਨਹੀਂਤਾਂ ਜੇ( p->ਰਸਤਾ.zBuf[j+1]=='"' ){
          sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, &p->ਰਸਤਾ.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }ਨਹੀਂਤਾਂ{
          sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, &p->ਰਸਤਾ.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        ਰੋਕੋ;
      }
      ਜੇ( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( p->eType==JSONB_ARRAY );
        sqlite3_ਨਤੀਜਾ_int64(ctx, p->a_ਮਾਪਾ[p->n_ਮਾਪਾ-1].i_ਕੁੰਜੀ);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ JEACH_ਮੁੱਲ: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->e_ਬਹੁਲਕ);
      ਜੇ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_ਨਤੀਜਾ_subtype(ctx, JSON_SUBTYPE);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      ਜੇ( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ JEACH_ID: {
      sqlite3_ਨਤੀਜਾ_int64(ctx, (sqlite3_int64)p->i);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JEACH_ਮਾਪਾ: {
      ਜੇ( p->n_ਮਾਪਾ>0 && p->bRecursive ){
        sqlite3_ਨਤੀਜਾ_int64(ctx, p->a_ਮਾਪਾ[p->n_ਮਾਪਾ-1].i_ਸਿਰ);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ JEACH_FULLKEY: {
      u64 n_ਅਧਾਰ = p->ਰਸਤਾ.n_ਵਰਤਿਆ;
      ਜੇ( p->n_ਮਾਪਾ ) json_Append_ਰਸਤਾ_Name(p);
      sqlite3_ਨਤੀਜਾ_text64(ctx, p->ਰਸਤਾ.zBuf, p->ਰਸਤਾ.n_ਵਰਤਿਆ,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->ਰਸਤਾ.n_ਵਰਤਿਆ = n_ਅਧਾਰ;
      ਰੋਕੋ;
    }
    ਹਾਲਤ JEACH_ਰਸਤਾ: {
      u32 n = json_Each_ਰਸਤਾ_ਲੰਮਾਈ(p);
      sqlite3_ਨਤੀਜਾ_text64(ctx, p->ਰਸਤਾ.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, p->ਰਸਤਾ.zBuf, p->n_ਜੜ੍ਹ, SQLITE_TRANSIENT);
      ਰੋਕੋ;
    }
    ਹਾਲਤ JEACH_JSON: {
      ਜੇ( p->sParse.zJson==0 ){
        sqlite3_ਨਤੀਜਾ_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

/* Return the current rowid value */
ਸਥਿਰ ਪੂਰਨ jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  ਵਾਪਸ SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
ਸਥਿਰ ਪੂਰਨ json_Each_ਉੱਤਮ_ਸੂਚਕਾਂਕ(
  sqlite3_vtab *tab,
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo
){
  ਪੂਰਨ i;                     /* Loop counter or computed array index */
  ਪੂਰਨ aIdx[2];               /* Index of constraints for JSON and ROOT */
  ਪੂਰਨ unusable_ਨਕਾਬ = 0;      /* Mask of unusable JSON and ROOT constraints */
  ਪੂਰਨ idx_ਨਕਾਬ = 0;           /* Mask of usable == constraints JSON and ROOT */
  ਅਚਲ ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  ਪ੍ਰਤਿਜ੍ਞਾ( JEACH_ਜੜ੍ਹ == JEACH_JSON+1 );
  ਅਣਵਰਤਿਆ_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  ਲਈ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ਪੂਰਨ i_ਖਾਨਾ;
    ਪੂਰਨ i_ਨਕਾਬ;
    ਜੇ( pConstraint->i_ਕਾਲਮ < JEACH_JSON ) ਜਾਰੀ;
    i_ਖਾਨਾ = pConstraint->i_ਕਾਲਮ - JEACH_JSON;
    ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ==0 || i_ਖਾਨਾ==1 );
    testcase( i_ਖਾਨਾ==0 );
    i_ਨਕਾਬ = 1 << i_ਖਾਨਾ;
    ਜੇ( pConstraint->usable==0 ){
      unusable_ਨਕਾਬ |= i_ਨਕਾਬ;
    }ਨਹੀਂਤਾਂ ਜੇ( pConstraint->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ ){
      aIdx[i_ਖਾਨਾ] = i;
      idx_ਨਕਾਬ |= i_ਨਕਾਬ;
    }
  }
  ਜੇ( pIdxInfo->n_ਕ੍ਰਮ_By>0
   && pIdxInfo->a_ਕ੍ਰਮ_By[0].i_ਕਾਲਮ<0
   && pIdxInfo->a_ਕ੍ਰਮ_By[0].desc==0
  ){
    pIdxInfo->ਕ੍ਰਮ_By_Consumed = 1;
  }

  ਜੇ( (unusable_ਨਕਾਬ & ~idx_ਨਕਾਬ)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    ਵਾਪਸ SQLITE_CONSTRAINT;
  }
  ਜੇ( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_ਅੰਕ = 0;
  }ਨਹੀਂਤਾਂ{
    pIdxInfo->estimated_ਲਾਗਤ = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argv_ਸੂਚਕਾਂਕ = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ਜੇ( aIdx[1]<0 ){
      pIdxInfo->idx_ਅੰਕ = 1;  /* Only JSON supplied.  Plan 1 */
    }ਨਹੀਂਤਾਂ{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argv_ਸੂਚਕਾਂਕ = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_ਅੰਕ = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

/* Start a search on a new JSON string */
ਸਥਿਰ ਪੂਰਨ jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ਅਚਲ ਅੱਖਰ *z_ਜੜ੍ਹ = 0;
  u32 i, n, sz;

  ਅਣਵਰਤਿਆ_PARAMETER(idxStr);
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  jsonEachCursorReset(p);
  ਜੇ( idx_ਅੰਕ==0 ) ਵਾਪਸ SQLITE_OK;
  memset(&p->sParse, 0, ਆਕਾਰ(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  ਜੇ( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }ਨਹੀਂਤਾਂ{
    p->sParse.zJson = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    p->sParse.nJson = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
    ਜੇ( p->sParse.zJson==0 ){
      p->i = p->i_ਸਿਰਾ = 0;
      ਵਾਪਸ SQLITE_OK;
    }      
    ਜੇ( json_Convert_ਪਾਠ_ਮੰਜ਼ਿਲ_Blob(&p->sParse, 0) ){
      ਜੇ( p->sParse.oom ){
        ਵਾਪਸ SQLITE_NOMEM;
      }
      ਵਾਪਸ json_Each_Malformed_ਨਿਵੇਸ਼(cur);
    }
  }
  ਜੇ( idx_ਅੰਕ==3 ){
    z_ਜੜ੍ਹ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
    ਜੇ( z_ਜੜ੍ਹ==0 ) ਵਾਪਸ SQLITE_OK;
    ਜੇ( z_ਜੜ੍ਹ[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_ਸੁਨੇਹਾ);
      cur->pVtab->z_Err_ਸੁਨੇਹਾ = json_Bad_ਰਸਤਾ_Error(0, z_ਜੜ੍ਹ, 0);
      jsonEachCursorReset(p);
      ਵਾਪਸ cur->pVtab->z_Err_ਸੁਨੇਹਾ ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_ਜੜ੍ਹ = sqlite3Strlen30(z_ਜੜ੍ਹ);
    ਜੇ( z_ਜੜ੍ਹ[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }ਨਹੀਂਤਾਂ{
      i = json_Lookup_ਕਦਮ(&p->sParse, 0, z_ਜੜ੍ਹ+1, 0);
      ਜੇ( JSON_LOOKUP_ISERROR(i) ){
        ਜੇ( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->i_ਸਿਰਾ = 0;
          ਵਾਪਸ SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_ਸੁਨੇਹਾ);
        cur->pVtab->z_Err_ਸੁਨੇਹਾ = json_Bad_ਰਸਤਾ_Error(0, z_ਜੜ੍ਹ, 0);
        jsonEachCursorReset(p);
        ਵਾਪਸ cur->pVtab->z_Err_ਸੁਨੇਹਾ ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      ਜੇ( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }ਨਹੀਂਤਾਂ{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->ਰਸਤਾ, z_ਜੜ੍ਹ, p->n_ਜੜ੍ਹ);
  }ਨਹੀਂਤਾਂ{
    i = p->i = 0;
    p->eType = 0;
    p->n_ਜੜ੍ਹ = 1;
    jsonAppendRaw(&p->ਰਸਤਾ, "$", 1);
  }
  p->n_ਮਾਪਾ = 0;
  n = jsonb_Payload_ਮਾਪ(&p->sParse, i, &sz);
  p->i_ਸਿਰਾ = i+n+sz;
  ਜੇ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->a_ਮਾਪਾ = sqlite3DbMallocZero(p->db, ਆਕਾਰ(Json_ਮਾਪਾ));
    ਜੇ( p->a_ਮਾਪਾ==0 ) ਵਾਪਸ SQLITE_NOMEM;
    p->n_ਮਾਪਾ = 1;
    p->n_ਮਾਪਾ_Alloc = 1;
    p->a_ਮਾਪਾ[0].i_ਕੁੰਜੀ = 0;
    p->a_ਮਾਪਾ[0].i_ਸਿਰਾ = p->i_ਸਿਰਾ;
    p->a_ਮਾਪਾ[0].i_ਸਿਰ = p->i;
    p->a_ਮਾਪਾ[0].i_ਮੁੱਲ = i;
  }
  ਵਾਪਸ SQLITE_OK;
}

/* The methods of the json_each virtual table */
ਸਥਿਰ sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  json_Each_ਉੱਤਮ_ਸੂਚਕਾਂਕ,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  json_Each_ਅਗਲਾ,              /* xNext - advance a cursor */
  jsonEachEof,               /* xEof - check for end of scan */
  json_Each_ਕਾਲਮ,            /* xColumn - read data */
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
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_JSON) */

/*
** Register JSON functions.
*/
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_Json_Functions(ਖਾਲੀ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_JSON
  ਸਥਿਰ FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_ਪਾਓ, -1,1,1, 1,0,JSON_AINS,  json_ਸੈੱਟ_Func),
    JFUNCTION(jsonb_array_ਪਾਓ,-1,1,0, 1,1,JSON_AINS,  json_ਸੈੱਟ_Func),
    JFUNCTION(json_array_ਲੰਮਾਈ,  1,1,0, 0,0,0,          json_Array_ਲੰਮਾਈ_Func),
    JFUNCTION(json_array_ਲੰਮਾਈ,  2,1,0, 0,0,0,          json_Array_ਲੰਮਾਈ_Func),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_ਕੱਢੋ,      -1,1,1, 0,0,0,          json_ਕੱਢੋ_Func),
    JFUNCTION(jsonb_ਕੱਢੋ,     -1,1,0, 0,1,0,          json_ਕੱਢੋ_Func),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  json_ਕੱਢੋ_Func),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   json_ਕੱਢੋ_Func),
    JFUNCTION(json_ਪਾਓ,       -1,1,1, 1,0,0,          json_ਸੈੱਟ_Func),
    JFUNCTION(jsonb_ਪਾਓ,      -1,1,0, 1,1,0,          json_ਸੈੱਟ_Func),
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
    JFUNCTION(json_ਸੈੱਟ,          -1,1,1, 1,0,JSON_ISSET, json_ਸੈੱਟ_Func),
    JFUNCTION(jsonb_ਸੈੱਟ,         -1,1,0, 1,1,JSON_ISSET, json_ਸੈੱਟ_Func),
    JFUNCTION(json_type,          1,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_type,          2,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_valid,         1,1,0, 0,0,0,          jsonValidFunc),
    JFUNCTION(json_valid,         2,1,0, 0,0,0,          jsonValidFunc),
#ਜੇ SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#ਜੇ_ਅੰਤ
    WAGGREGATE(json_ਸਮੂਹ_array,  1, 0, 0,
       json_Array_ਕਦਮ, jsonArrayFinal, json_Array_ਮੁੱਲ, json_ਸਮੂਹ_Inverse,
       SQLITE_SUBTYPE|SQLITE_ਨਤੀਜਾ_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_ਸਮੂਹ_array, 1, JSON_BLOB, 0,
       json_Array_ਕਦਮ, jsonArrayFinal, json_Array_ਮੁੱਲ, json_ਸਮੂਹ_Inverse,
       SQLITE_SUBTYPE|SQLITE_ਨਤੀਜਾ_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_ਸਮੂਹ_object, 2, 0, 0,
       json_Object_ਕਦਮ, jsonObjectFinal, json_Object_ਮੁੱਲ, json_ਸਮੂਹ_Inverse,
       SQLITE_SUBTYPE|SQLITE_ਨਤੀਜਾ_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_ਸਮੂਹ_object,2, JSON_BLOB, 0,
       json_Object_ਕਦਮ, jsonObjectFinal, json_Object_ਮੁੱਲ, json_ਸਮੂਹ_Inverse,
       SQLITE_SUBTYPE|SQLITE_ਨਤੀਜਾ_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_ਪਾਓ_Builtin_Funcs(aJsonFunc, Array_ਮਾਪ(aJsonFunc));
#ਜੇ_ਅੰਤ
}

#ਜੇ  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3_Json_Vtab_ਰਜਿਸਟਰ(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zName){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Hash_ਲੱਭੋ(&db->aModule, zName)==0 );
  ਲਈ(i=0; i<ਆਕਾਰ(azModule)/ਆਕਾਰ(azModule[0]); i++){
    ਜੇ( sqlite3StrICmp(azModule[i],zName)==0 ){
      ਵਾਪਸ sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
