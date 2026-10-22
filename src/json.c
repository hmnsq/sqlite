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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_JSON
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/* JSONB element types
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_NULL     0   /* "null" */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_TRUE     1   /* "true" */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_FALSE    2   /* "false" */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_INT      3   /* integer acceptable to JSON and SQL */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_INT5     4   /* integer in 0x000 notation */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_FLOAT    5   /* float acceptable to JSON and SQL */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_FLOAT5   6   /* float with JSON5 extensions */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_ಪಠ್ಯ     7   /* Text compatible with both JSON and SQL */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_TEXTJ    8   /* Text with JSON escapes */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_TEXT5    9   /* Text with JSON-5 escape */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_TEXTRAW 10   /* SQL text that needs escaping for JSON */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_ARRAY   11   /* An array */
#ವ್ಯಾಖ್ಯಾನಿಸು JSONB_OBJECT  12   /* An object */

/* Human-readable names for the JSONB values.  The index for each
** string must correspond to the JSONB_* integer above.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ jsonIsSpace[] = {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
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
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
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
#ಆದರೆ_ಅಂತ್ಯ

};
#ವ್ಯಾಖ್ಯಾನಿಸು jsonIsspace(x) (jsonIsSpace[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ jsonSpaces[] = "\011\012\015\040";
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ jsonSpaces[] = "\005\045\015\100";
#ಆದರೆ_ಅಂತ್ಯ


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ jsonIsOk[256] = {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
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
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
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
#ಆದರೆ_ಅಂತ್ಯ
};

/* Objects */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ JsonCache JsonCache;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ JsonString JsonString;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_CACHE_ID    (-429938)  /* Cache entry */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_CACHE_ಗಾತ್ರ  4          /* Max number of cache entries */

/*
** jsonUnescapeOneChar() returns this invalid code point if it encounters
** a syntax error.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_INVALID_CHAR 0x99999

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
ರಚನೆ JsonCache {
  sqlite3 *db;                    /* Database connection */
  ಪೂರ್ಣಾಂಕ n_ಬಳಸಿದ;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_ಗಾತ್ರ];  /* One line for each cache entry */
};

/* An instance of this object represents a JSON string
** under construction.  Really, this is a generic string accumulator
** that can be and is used to create strings other than JSON.
**
** If the generated string is longer than will fit into the zSpace[] buffer,
** then it will be an RCStr string.  This aids with caching of large
** JSON strings.
*/
ರಚನೆ JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  ಅಕ್ಷರ *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 n_ಬಳಸಿದ;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  ಅಕ್ಷರ zSpace[100];        /* Initial static space */
};

/* Allowed values for JsonString.eErr */
#ವ್ಯಾಖ್ಯಾನಿಸು JSTRING_OOM         0x01   /* Out of memory */
#ವ್ಯಾಖ್ಯಾನಿಸು JSTRING_MALFORMED   0x02   /* Malformed JSONB */
#ವ್ಯಾಖ್ಯಾನಿಸು JSTRING_TOODEEP     0x04   /* JSON nested too deep */
#ವ್ಯಾಖ್ಯಾನಿಸು JSTRING_ERR         0x08   /* Error already sent to sqlite3_result */

/* The "subtype" set for text JSON values passed through using
** sqlite3_result_subtype() and sqlite3_value_subtype().
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_SUBTYPE  74    /* Ascii for "J" */

/*
** Bit values for the flags passed into various SQL function implementations
** via the sqlite3_user_data() value.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_JSON      0x01        /* Result is always JSON */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_SQL       0x02        /* Result is always SQL */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_ABPATH    0x03        /* Allow abbreviated JSON path specs */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_ISSET     0x04        /* json_set(), not json_insert() */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_AINS      0x08        /* json_array_insert(), not json_insert() */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_BLOB      0x10        /* Use the BLOB output format */

#ವ್ಯಾಖ್ಯಾನಿಸು JSON_ಸೇರಿಸು_TYPE(X)  (((X)&0xC)>>2)


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
ರಚನೆ JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  ಅಕ್ಷರ *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  ಪೂರ್ಣಾಂಕ nJson;         /* Length of the zJson string in bytes */
  u32 nJPRef;        /* Number of references to this object */
  u32 iErr;          /* Error location in zJson[] */
  u16 i_ಆಳ;        /* Nesting depth */
  u8 nErr;           /* Number of errors seen */
  u8 oom;            /* Set to true if out of memory */
  u8 bJsonIsRCStr;   /* True if zJson is an RCStr */
  u8 hasNonstd;      /* True if input uses non-standard features like JSON5 */
  u8 bReadOnly;      /* Do not modify. */
  /* Search and edit information.  See jsonLookupStep() */
  u8 eEdit;          /* Edit operation to apply */
  ಪೂರ್ಣಾಂಕ delta;         /* Size change due to the edit */
  u32 nIns;          /* Number of bytes to insert */
  u32 iLabel;        /* Location of label if search landed on an object value */
  u8 *aIns;          /* Content to be inserted */
};

/* Allowed values for JsonParse.eEdit */
#ವ್ಯಾಖ್ಯಾನಿಸು JEDIT_DEL   1   /* Delete if exists */
#ವ್ಯಾಖ್ಯಾನಿಸು JEDIT_REPL  2   /* Overwrite if exists */
#ವ್ಯಾಖ್ಯಾನಿಸು JEDIT_INS   3   /* Insert if not exists */
#ವ್ಯಾಖ್ಯಾನಿಸು JEDIT_ಗಣ   4   /* Insert or overwrite */
#ವ್ಯಾಖ್ಯಾನಿಸು JEDIT_AINS  5   /* array_insert() */

/*
** Maximum nesting depth of JSON for this implementation.
**
** This limit is needed to avoid a stack overflow in the recursive
** descent parser.  A depth of 1000 is far deeper than any sane JSON
** should go.  Historical note: This limit was 2000 prior to version 3.42.0
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_JSON_MAX_ಆಳ
# define JSON_MAX_ಆಳ  1000
#ಇಲ್ಲದಿದ್ದರೆ
# define JSON_MAX_ಆಳ SQLITE_JSON_MAX_ಆಳ
#ಆದರೆ_ಅಂತ್ಯ

/*
** Allowed values for the flgs argument to jsonParseFuncArg();
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_EDITABLE  0x01   /* Generate a writable JsonParse object */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_KEEPERROR 0x02   /* Return non-NULL even if there is an error */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_NOCACHE   0x04   /* Do not use the JSON cache */

/**************************************************************************
** Forward references
**************************************************************************/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnStringAsBlob(JsonString*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonArgIsJsonb(sqlite3_ಮೌಲ್ಯ *pJson, JsonParse *p);
ಸ್ಥಿರ u32 json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(JsonParse*,u32,JsonString*);
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnParse(sqlite3_context*,JsonParse*);
ಸ್ಥಿರ JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_ಮೌಲ್ಯ*,u32);
ಸ್ಥಿರ ಶೂನ್ಯ jsonParseFree(JsonParse*);
ಸ್ಥಿರ u32 jsonb_Payload_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ JsonParse*, u32, u32*);
ಸ್ಥಿರ u32 jsonUnescapeOneChar(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonCacheDelete(JsonCache *p){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<p->n_ಬಳಸಿದ; i++){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Cache_ಸೇರಿಸು(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  ಪ್ರತಿಜ್ಞಾ( pParse->zJson!=0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->bJsonIsRCStr );
  ಪ್ರತಿಜ್ಞಾ( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ಆದರೆ( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, ಗಾತ್ರ(*p));
    ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    p->db = db;
    sqlite3_ಗಣ_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }
  ಆದರೆ( p->n_ಬಳಸಿದ >= JSON_CACHE_ಗಾತ್ರ ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_ಗಾತ್ರ-1)*ಗಾತ್ರ(p->a[0]));
    p->n_ಬಳಸಿದ = JSON_CACHE_ಗಾತ್ರ-1;
  }
  ಪ್ರತಿಜ್ಞಾ( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->n_ಬಳಸಿದ] = pParse;
  p->n_ಬಳಸಿದ++;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
  ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zJson;
  ಪೂರ್ಣಾಂಕ nJson;

  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(pArg)!=SQLITE_ಪಠ್ಯ ){
    ಹಿಂತಿರುಗಿ 0;
  }
  zJson = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pArg);
  ಆದರೆ( zJson==0 ) ಹಿಂತಿರುಗಿ 0;
  nJson = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ಆದರೆ( p==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಗಾಗಿ(i=0; i<p->n_ಬಳಸಿದ; i++){
    ಆದರೆ( p->a[i]->zJson==zJson ) ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ( i>=p->n_ಬಳಸಿದ ){
    ಗಾಗಿ(i=0; i<p->n_ಬಳಸಿದ; i++){
      ಆದರೆ( p->a[i]->nJson!=nJson ) ಮುಂದುವರಿ;
      ಆದರೆ( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( i<p->n_ಬಳಸಿದ ){
    ಆದರೆ( i<p->n_ಬಳಸಿದ-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->n_ಬಳಸಿದ-i-1)*ಗಾತ್ರ(tmp));
      p->a[p->n_ಬಳಸಿದ-1] = tmp;
      i = p->n_ಬಳಸಿದ - 1;
    }
    ಪ್ರತಿಜ್ಞಾ( p->a[i]->delta==0 );
    ಹಿಂತಿರುಗಿ p->a[i];
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
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
  p->nAlloc = ಗಾತ್ರ(p->zSpace);
  p->n_ಬಳಸಿದ = 0;
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
  ಆದರೆ( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  ಆದರೆ( p->pCtx ) sqlite3_ಫಲಿತಾಂಶ_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  ಪ್ರತಿಜ್ಞಾ( p->pCtx!=0 );
  sqlite3_ಫಲಿತಾಂಶ_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonStringGrow(JsonString *p, u32 N){
  u64 n_ಒಟ್ಟು = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  ಅಕ್ಷರ *zNew;
  ಆದರೆ( p->bStatic ){
    ಆದರೆ( p->eErr ) ಹಿಂತಿರುಗಿ 1;
    zNew = sqlite3RCStrNew(n_ಒಟ್ಟು);
    ಆದರೆ( zNew==0 ){
      jsonStringOom(p);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    ಸ್ಮರಣೆ_ನಕಲು(zNew, p->zBuf, (size_t)p->n_ಬಳಸಿದ);
    p->zBuf = zNew;
    p->bStatic = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->zBuf = sqlite3RCStrResize(p->zBuf, n_ಒಟ್ಟು);
    ಆದರೆ( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
  }
  p->nAlloc = n_ಒಟ್ಟು;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ jsonStringExpandAndAppend(
  JsonString *p,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ,
  u32 N
){
  ಪ್ರತಿಜ್ಞಾ( N>0 );
  ಆದರೆ( jsonStringGrow(p,N) ) ಹಿಂತಿರುಗಿ;
  ಸ್ಮರಣೆ_ನಕಲು(p->zBuf+p->n_ಬಳಸಿದ, z_ಒಳ, N);
  p->n_ಬಳಸಿದ += N;
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendRaw(JsonString *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ, u32 N){
  ಆದರೆ( N==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( N+p->n_ಬಳಸಿದ >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_ಒಳ,N);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸ್ಮರಣೆ_ನಕಲು(p->zBuf+p->n_ಬಳಸಿದ, z_ಒಳ, N);
    p->n_ಬಳಸಿದ += N;
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendRawNZ(JsonString *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ, u32 N){
  ಪ್ರತಿಜ್ಞಾ( N>0 );
  ಆದರೆ( N+p->n_ಬಳಸಿದ >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_ಒಳ,N);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸ್ಮರಣೆ_ನಕಲು(p->zBuf+p->n_ಬಳಸಿದ, z_ಒಳ, N);
    p->n_ಬಳಸಿದ += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonPrintf(ಪೂರ್ಣಾಂಕ N, JsonString *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;
  ಆದರೆ( (p->n_ಬಳಸಿದ + N >= p->nAlloc) && jsonStringGrow(p, N) ) ಹಿಂತಿರುಗಿ;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->n_ಬಳಸಿದ, zFormat, ap);
  va_end(ap);
  p->n_ಬಳಸಿದ += (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p->zBuf+p->n_ಬಳಸಿದ);
}

/* Append a single character
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ jsonAppendCharExpand(JsonString *p, ಅಕ್ಷರ c){
  ಆದರೆ( jsonStringGrow(p,1) ) ಹಿಂತಿರುಗಿ;
  p->zBuf[p->n_ಬಳಸಿದ++] = c;
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendChar(JsonString *p, ಅಕ್ಷರ c){
  ಆದರೆ( p->n_ಬಳಸಿದ>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->zBuf[p->n_ಬಳಸಿದ++] = c;
  }
}

/* Remove a single character from the end of the string
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonStringTrimOneChar(JsonString *p){
  ಆದರೆ( p->eErr==0 ){
    ಪ್ರತಿಜ್ಞಾ( p->n_ಬಳಸಿದ>0 );
    p->n_ಬಳಸಿದ--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  ಹಿಂತಿರುಗಿ p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendSeparator(JsonString *p){
  ಅಕ್ಷರ c;
  ಆದರೆ( p->n_ಬಳಸಿದ==0 ) ಹಿಂತಿರುಗಿ;
  c = p->zBuf[p->n_ಬಳಸಿದ-1];
  ಆದರೆ( c=='[' || c=='{' ) ಹಿಂತಿರುಗಿ;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendControlChar(JsonString *p, u8 c){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(aSpecial)==32 );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\b']=='b' );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\f']=='f' );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\n']=='n' );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\r']=='r' );
  ಪ್ರತಿಜ್ಞಾ( aSpecial['\t']=='t' );
  ಪ್ರತಿಜ್ಞಾ( c>=0 && c<ಗಾತ್ರ(aSpecial) );
  ಪ್ರತಿಜ್ಞಾ( p->n_ಬಳಸಿದ+7 <= p->nAlloc );
  ಆದರೆ( aSpecial[c] ){
    p->zBuf[p->n_ಬಳಸಿದ] = '\\';
    p->zBuf[p->n_ಬಳಸಿದ+1] = aSpecial[c];
    p->n_ಬಳಸಿದ += 2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->zBuf[p->n_ಬಳಸಿದ] = '\\';
    p->zBuf[p->n_ಬಳಸಿದ+1] = 'u';
    p->zBuf[p->n_ಬಳಸಿದ+2] = '0';
    p->zBuf[p->n_ಬಳಸಿದ+3] = '0';
    p->zBuf[p->n_ಬಳಸಿದ+4] = "0123456789abcdef"[c>>4];
    p->zBuf[p->n_ಬಳಸಿದ+5] = "0123456789abcdef"[c&0xf];
    p->n_ಬಳಸಿದ += 6;
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
ಸ್ಥಿರ ಶೂನ್ಯ jsonAppendString(JsonString *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ, u32 N){
  u32 k;
  u8 c;
  ಸ್ಥಿರಾಂಕ u8 *z = (ಸ್ಥಿರಾಂಕ u8*)z_ಒಳ;
  ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( (N+p->n_ಬಳಸಿದ+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) ಹಿಂತಿರುಗಿ;
  p->zBuf[p->n_ಬಳಸಿದ++] = '"';
  ಆಗ( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    ಆಗ( 1 /* Exit by break */ ){
      ಆದರೆ( k+3>=N ){
        ಆಗ( k<N && jsonIsOk[z[k]] ){ k++; }
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( !jsonIsOk[z[k]] ){
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( !jsonIsOk[z[k+1]] ){
        k += 1;
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( !jsonIsOk[z[k+2]] ){
        k += 2;
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( !jsonIsOk[z[k+3]] ){
        k += 3;
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ{
        k += 4;
      }
    }
    ಆದರೆ( k>=N ){
      ಆದರೆ( k>0 ){
        ಸ್ಮರಣೆ_ನಕಲು(&p->zBuf[p->n_ಬಳಸಿದ], z, k);
        p->n_ಬಳಸಿದ += k;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( k>0 ){
      ಸ್ಮರಣೆ_ನಕಲು(&p->zBuf[p->n_ಬಳಸಿದ], z, k);
      p->n_ಬಳಸಿದ += k;
      z += k;
      N -= k;
    }
    c = z[0];
    ಆದರೆ( c=='"' || c=='\\' ){
      ಆದರೆ( (p->n_ಬಳಸಿದ+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) ಹಿಂತಿರುಗಿ;
      p->zBuf[p->n_ಬಳಸಿದ++] = '\\';
      p->zBuf[p->n_ಬಳಸಿದ++] = c;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='\'' ){
      p->zBuf[p->n_ಬಳಸಿದ++] = c;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( (p->n_ಬಳಸಿದ+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) ಹಿಂತಿರುಗಿ;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->n_ಬಳಸಿದ++] = '"';
  ಪ್ರತಿಜ್ಞಾ( p->n_ಬಳಸಿದ<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Append_Sql_ಮೌಲ್ಯ(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ          /* Value to append */
){
  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type(p_ಮೌಲ್ಯ) ){
    ಸ್ಥಿತಿ SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_ಮೌಲ್ಯ_double(p_ಮೌಲ್ಯ));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_INTEGER: {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p_ಮೌಲ್ಯ);
      u32 n = (u32)sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(p_ಮೌಲ್ಯ);
      jsonAppendRaw(p, z, n);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_ಪಠ್ಯ: {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p_ಮೌಲ್ಯ);
      u32 n = (u32)sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(p_ಮೌಲ್ಯ);
      ಆದರೆ( sqlite3_ಮೌಲ್ಯ_subtype(p_ಮೌಲ್ಯ)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }ಇಲ್ಲದಿದ್ದರೆ{
        jsonAppendString(p, z, n);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      JsonParse px;
      memset(&px, 0, ಗಾತ್ರ(px));
      ಆದರೆ( jsonArgIsJsonb(p_ಮೌಲ್ಯ, &px) ){
        json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(&px, 0, p);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->eErr==0 ){
        sqlite3_ಫಲಿತಾಂಶ_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      ನಿಲ್ಲಿಸು;
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
  ಆದರೆ( p->eErr==0 ){
    ಪೂರ್ಣಾಂಕ flags = SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(p->pCtx));
    ಆದರೆ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->bStatic ){
      sqlite3_ಫಲಿತಾಂಶ_text64(p->pCtx, p->zBuf, p->n_ಬಳಸಿದ,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        ಪೂರ್ಣಾಂಕ rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->n_ಬಳಸಿದ;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_ಸೇರಿಸು(ctx, pParse);
        ಆದರೆ( rc==SQLITE_NOMEM ){
          sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
          jsonStringReset(p);
          ಹಿಂತಿರುಗಿ;
        }
      }
      sqlite3_ಫಲಿತಾಂಶ_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->n_ಬಳಸಿದ,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->eErr & JSTRING_OOM ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(p->pCtx);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->eErr & JSTRING_MALFORMED ){
    sqlite3_ಫಲಿತಾಂಶ_error(p->pCtx, "malformed JSON", -1);
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
  ಆದರೆ( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  ಆದರೆ( pParse->nBlobAlloc ){
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
  ಆದರೆ( pParse ){
    ಆದರೆ( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }ಇಲ್ಲದಿದ್ದರೆ{
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
ಸ್ಥಿರ u8 json_Hex_ಗಮ್ಯ_Int(ಪೂರ್ಣಾಂಕ h){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
  h += 9*(1&(h>>6));
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
ಸ್ಥಿರ u32 json_Hex_ಗಮ್ಯ_Int4(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  u32 v;
  v = (json_Hex_ಗಮ್ಯ_Int(z[0])<<12)
    + (json_Hex_ಗಮ್ಯ_Int(z[1])<<8)
    + (json_Hex_ಗಮ್ಯ_Int(z[2])<<4)
    + json_Hex_ಗಮ್ಯ_Int(z[3]);
  ಹಿಂತಿರುಗಿ v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonIs2Hex(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಹಿಂತಿರುಗಿ sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonIs4Hex(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಹಿಂತಿರುಗಿ jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json5Whitespace(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ){
  ಪೂರ್ಣಾಂಕ n = 0;
  ಸ್ಥಿರಾಂಕ u8 *z = (u8*)z_ಒಳ;
  ಆಗ( 1 /*exit by "goto whitespace_done"*/ ){
    ಆಯ್ಕೆ( z[n] ){
      ಸ್ಥಿತಿ 0x09:
      ಸ್ಥಿತಿ 0x0a:
      ಸ್ಥಿತಿ 0x0b:
      ಸ್ಥಿತಿ 0x0c:
      ಸ್ಥಿತಿ 0x0d:
      ಸ್ಥಿತಿ 0x20: {
        n++;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '/': {
        ಆದರೆ( z[n+1]=='*' && z[n+2]!=0 ){
          ಪೂರ್ಣಾಂಕ j;
          ಗಾಗಿ(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            ಆದರೆ( z[j]==0 ) ಹೋಗು whitespace_ಮುಗಿದ;
          }
          n = j+1;
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[n+1]=='/' ){
          ಪೂರ್ಣಾಂಕ j;
          ಅಕ್ಷರ c;
          ಗಾಗಿ(j=n+2; (c = z[j])!=0; j++){
            ಆದರೆ( c=='\n' || c=='\r' ) ನಿಲ್ಲಿಸು;
            ಆದರೆ( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              ನಿಲ್ಲಿಸು;
            }
          }
          n = j;
          ಆದರೆ( z[n] ) n++;
          ನಿಲ್ಲಿಸು;
        }
        ಹೋಗು whitespace_ಮುಗಿದ;
      }
      ಸ್ಥಿತಿ 0xc2: {
        ಆದರೆ( z[n+1]==0xa0 ){
          n += 2;
          ನಿಲ್ಲಿಸು;
        }
        ಹೋಗು whitespace_ಮುಗಿದ;
      }
      ಸ್ಥಿತಿ 0xe1: {
        ಆದರೆ( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          ನಿಲ್ಲಿಸು;
        }
        ಹೋಗು whitespace_ಮುಗಿದ;
      }
      ಸ್ಥಿತಿ 0xe2: {
        ಆದರೆ( z[n+1]==0x80 ){
          u8 c = z[n+2];
          ಆದರೆ( c<0x80 ) ಹೋಗು whitespace_ಮುಗಿದ;
          ಆದರೆ( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            ನಿಲ್ಲಿಸು;
          }
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          ನಿಲ್ಲಿಸು;
        }
        ಹೋಗು whitespace_ಮುಗಿದ;
      }
      ಸ್ಥಿತಿ 0xe3: {
        ಆದರೆ( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          ನಿಲ್ಲಿಸು;
        }
        ಹೋಗು whitespace_ಮುಗಿದ;
      }
      ಸ್ಥಿತಿ 0xef: {
        ಆದರೆ( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          ನಿಲ್ಲಿಸು;
        }
        ಹೋಗು whitespace_ಮುಗಿದ;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಹೋಗು whitespace_ಮುಗಿದ;
      }
    }
  }
  whitespace_ಮುಗಿದ:
  ಹಿಂತಿರುಗಿ n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ NanInfName {
  ಅಕ್ಷರ c1;
  ಅಕ್ಷರ c2;
  ಅಕ್ಷರ n;
  ಅಕ್ಷರ eType;
  ಅಕ್ಷರ nRepl;
  ಅಕ್ಷರ *z_ಹೊಂದಿಕೆ;
  ಅಕ್ಷರ *zRepl;
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
ಸ್ಥಿರ ಶೂನ್ಯ json_Wrong_ಅಂಕೆ_ವಾದಗಳು(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFuncName
){
  ಅಕ್ಷರ *z_ಸಂದೇಶ = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_ಫಲಿತಾಂಶ_error(pCtx, z_ಸಂದೇಶ, -1);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  ಪ್ರತಿಜ್ಞಾ( N>pParse->nBlobAlloc );
  ಆದರೆ( pParse->nBlobAlloc==0 ){
    t = 100;
  }ಇಲ್ಲದಿದ್ದರೆ{
    t = pParse->nBlobAlloc*2;
  }
  ಆದರೆ( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  ಆದರೆ( aNew==0 ){ pParse->oom = 1; ಹಿಂತಿರುಗಿ 1; }
  ಪ್ರತಿಜ್ಞಾ( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  ಹಿಂತಿರುಗಿ 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 n_ಗಾತ್ರ;
  ಪ್ರತಿಜ್ಞಾ( !pParse->bReadOnly );
  ಆದರೆ( pParse->oom ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( pParse->nBlobAlloc>0 ) ಹಿಂತಿರುಗಿ 1;
  aOld = pParse->aBlob;
  n_ಗಾತ್ರ = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  ಆದರೆ( jsonBlobExpand(pParse, n_ಗಾತ್ರ) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  ಸ್ಮರಣೆ_ನಕಲು(pParse->aBlob, aOld, pParse->nBlob);
  ಹಿಂತಿರುಗಿ 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  ಆದರೆ( pParse->oom==0 ){
    ಪ್ರತಿಜ್ಞಾ( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  ಆದರೆ( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Blob_Append_ಗಂಟು(JsonParse*,u8,u64,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*);
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ json_Blob_Expand_And_Append_ಗಂಟು(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *aPayload
){
  ಆದರೆ( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) ಹಿಂತಿರುಗಿ;
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
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  ಆದರೆ( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_ಗಂಟು(pParse,eType,szPayload,aPayload);
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  ಆದರೆ( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }ಇಲ್ಲದಿದ್ದರೆ{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  ಆದರೆ( aPayload ){
    pParse->nBlob += szPayload;
    ಸ್ಮರಣೆ_ನಕಲು(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Blob_Change_Payload_ಗಾತ್ರ(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  ಪೂರ್ಣಾಂಕ delta;
  ಆದರೆ( pParse->oom ) ಹಿಂತಿರುಗಿ 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  ಆದರೆ( szType<=11 ){
    nExtra = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( szType==12 ){
    nExtra = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( szType==13 ){
    nExtra = 2;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( szType==14 ){
    nExtra = 4;
  }ಇಲ್ಲದಿದ್ದರೆ{
    nExtra = 8;
  }
  ಆದರೆ( szPayload<=11 ){
    nNeeded = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( szPayload<=0xff ){
    nNeeded = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( szPayload<=0xffff ){
    nNeeded = 2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  ಆದರೆ( delta ){
    u64 new_ಗಾತ್ರ_2 = (u64)pParse->nBlob + delta;
    ಆದರೆ( delta>0 ){
      ಆದರೆ( new_ಗಾತ್ರ_2>pParse->nBlobAlloc && jsonBlobExpand(pParse, new_ಗಾತ್ರ_2) ){
        ಹಿಂತಿರುಗಿ 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }ಇಲ್ಲದಿದ್ದರೆ{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = new_ಗಾತ್ರ_2;
  }
  ಆದರೆ( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }ಇಲ್ಲದಿದ್ದರೆ{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  ಹಿಂತಿರುಗಿ delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonIs4HexB(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ *pOp){
  ಆದರೆ( z[0]!='u' ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( !jsonIs4Hex(&z[1]) ) ಹಿಂತಿರುಗಿ 0;
  *pOp = JSONB_TEXTJ;
  ಹಿಂತಿರುಗಿ 1;
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
  ಸ್ಥಿರಾಂಕ JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 i_ಕೊನೆ,                   /* One more than the last byte of the element */
  u32 i_ಆಳ                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ಸ್ಥಿರಾಂಕ u8 *z;
  u8 x;
  ಆದರೆ( i_ಆಳ>JSON_MAX_ಆಳ ) ಹಿಂತಿರುಗಿ i+1;
  sz = 0;
  n = jsonb_Payload_ಗಾತ್ರ(pParse, i, &sz);
  ಆದರೆ( NEVER(n==0) ) ಹಿಂತಿರುಗಿ i+1;          /* Checked by caller */
  ಆದರೆ( NEVER(i+n+sz!=i_ಕೊನೆ) ) ಹಿಂತಿರುಗಿ i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  ಆಯ್ಕೆ( x ){
    ಸ್ಥಿತಿ JSONB_NULL:
    ಸ್ಥಿತಿ JSONB_TRUE:
    ಸ್ಥಿತಿ JSONB_FALSE: {
      ಹಿಂತಿರುಗಿ n+sz==1 ? 0 : i+1;
    }
    ಸ್ಥಿತಿ JSONB_INT: {
      ಆದರೆ( sz<1 ) ಹಿಂತಿರುಗಿ i+1;
      j = i+n;
      ಆದರೆ( z[j]=='-' ){
        j++;
        ಆದರೆ( sz<2 ) ಹಿಂತಿರುಗಿ i+1;
      }
      k = i+n+sz;
      ಆಗ( j<k ){
        ಆದರೆ( sqlite3Isdigit(z[j]) ){
          j++;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಹಿಂತಿರುಗಿ j+1;
        }
      }
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಥಿತಿ JSONB_INT5: {
      ಆದರೆ( sz<3 ) ಹಿಂತಿರುಗಿ i+1;
      j = i+n;
      ಆದರೆ( z[j]=='-' ){
        ಆದರೆ( sz<4 ) ಹಿಂತಿರುಗಿ i+1;
        j++;
      }
      ಆದರೆ( z[j]!='0' ) ಹಿಂತಿರುಗಿ i+1;
      ಆದರೆ( z[j+1]!='x' && z[j+1]!='X' ) ಹಿಂತಿರುಗಿ j+2;
      j += 2;
      k = i+n+sz;
      ಆಗ( j<k ){
        ಆದರೆ( sqlite3Isxdigit(z[j]) ){
          j++;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಹಿಂತಿರುಗಿ j+1;
        }
      }
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಥಿತಿ JSONB_FLOAT:
    ಸ್ಥಿತಿ JSONB_FLOAT5: {
      u8 ನೋಡಿದ = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      ಆದರೆ( sz<2 ) ಹಿಂತಿರುಗಿ i+1;
      j = i+n;
      k = j+sz;
      ಆದರೆ( z[j]=='-' ){
        j++;
        ಆದರೆ( sz<3 ) ಹಿಂತಿರುಗಿ i+1;
      }
      ಆದರೆ( z[j]=='.' ){
        ಆದರೆ( x==JSONB_FLOAT ) ಹಿಂತಿರುಗಿ j+1;
        ಆದರೆ( !sqlite3Isdigit(z[j+1]) ) ಹಿಂತಿರುಗಿ j+1;
        j += 2;
        ನೋಡಿದ = 1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[j]=='0' && x==JSONB_FLOAT ){
        ಆದರೆ( j+3>k ) ಹಿಂತಿರುಗಿ j+1;
        ಆದರೆ( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) ಹಿಂತಿರುಗಿ j+1;
        j++;
      }
      ಗಾಗಿ(; j<k; j++){
        ಆದರೆ( sqlite3Isdigit(z[j]) ) ಮುಂದುವರಿ;
        ಆದರೆ( z[j]=='.' ){
          ಆದರೆ( ನೋಡಿದ>0 ) ಹಿಂತಿರುಗಿ j+1;
          ಆದರೆ( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            ಹಿಂತಿರುಗಿ j+1;
          }
          ನೋಡಿದ = 1;
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( z[j]=='e' || z[j]=='E' ){
          ಆದರೆ( ನೋಡಿದ==2 ) ಹಿಂತಿರುಗಿ j+1;
          ಆದರೆ( j==k-1 ) ಹಿಂತಿರುಗಿ j+1;
          ಆದರೆ( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            ಆದರೆ( j==k-1 ) ಹಿಂತಿರುಗಿ j+1;
          }
          ನೋಡಿದ = 2;
          ಮುಂದುವರಿ;
        }
        ಹಿಂತಿರುಗಿ j+1;
      }
      ಆದರೆ( ನೋಡಿದ==0 ) ಹಿಂತಿರುಗಿ i+1;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಥಿತಿ JSONB_ಪಠ್ಯ: {
      j = i+n;
      k = j+sz;
      ಆಗ( j<k ){
        ಆದರೆ( !jsonIsOk[z[j]] && z[j]!='\'' ) ಹಿಂತಿರುಗಿ j+1;
        j++;
      }
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಥಿತಿ JSONB_TEXTJ:
    ಸ್ಥಿತಿ JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      ಆಗ( j<k ){
        ಆದರೆ( !jsonIsOk[z[j]] && z[j]!='\'' ){
          ಆದರೆ( z[j]=='"' ){
            ಆದರೆ( x==JSONB_TEXTJ ) ಹಿಂತಿರುಗಿ j+1;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            ಆದರೆ( x==JSONB_TEXTJ ) ಹಿಂತಿರುಗಿ j+1;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( NEVER(z[j]!='\\') || j+1>=k ){
            ಹಿಂತಿರುಗಿ j+1;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[j+1]=='u' ){
            ಆದರೆ( j+5>=k ) ಹಿಂತಿರುಗಿ j+1;
            ಆದರೆ( !jsonIs4Hex((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&z[j+2]) ) ಹಿಂತಿರುಗಿ j+1;
            j++;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x!=JSONB_TEXT5 ){
            ಹಿಂತಿರುಗಿ j+1;
          }ಇಲ್ಲದಿದ್ದರೆ{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&z[j], k-j, &c);
            ಆದರೆ( c==JSON_INVALID_CHAR ) ಹಿಂತಿರುಗಿ j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಥಿತಿ JSONB_TEXTRAW: {
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಥಿತಿ JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      ಆಗ( j<k ){
        sz = 0;
        n = jsonb_Payload_ಗಾತ್ರ(pParse, j, &sz);
        ಆದರೆ( n==0 ) ಹಿಂತಿರುಗಿ j+1;
        ಆದರೆ( j+n+sz>k ) ಹಿಂತಿರುಗಿ j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_ಆಳ+1);
        ಆದರೆ( sub ) ಹಿಂತಿರುಗಿ sub;
        j += n + sz;
      }
      ಪ್ರತಿಜ್ಞಾ( j==k );
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಥಿತಿ JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      ಆಗ( j<k ){
        sz = 0;
        n = jsonb_Payload_ಗಾತ್ರ(pParse, j, &sz);
        ಆದರೆ( n==0 ) ಹಿಂತಿರುಗಿ j+1;
        ಆದರೆ( j+n+sz>k ) ಹಿಂತಿರುಗಿ j+1;
        ಆದರೆ( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          ಆದರೆ( x<JSONB_ಪಠ್ಯ || x>JSONB_TEXTRAW ) ಹಿಂತಿರುಗಿ j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_ಆಳ+1);
        ಆದರೆ( sub ) ಹಿಂತಿರುಗಿ sub;
        cnt++;
        j += n + sz;
      }
      ಪ್ರತಿಜ್ಞಾ( j==k );
      ಆದರೆ( (cnt & 1)!=0 ) ಹಿಂತಿರುಗಿ j+1;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಹಿಂತಿರುಗಿ i+1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(JsonParse *pParse, u32 i){
  ಅಕ್ಷರ c;
  u32 j;
  u32 iThis, i_ಆರಂಭ;
  ಪೂರ್ಣಾಂಕ x;
  u8 t;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = pParse->zJson;
json_parse_restart:
  ಆಯ್ಕೆ( (u8)z[i] ){
  ಸ್ಥಿತಿ '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_ಗಂಟು(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    ಆದರೆ( ++pParse->i_ಆಳ > JSON_MAX_ಆಳ ){
      pParse->iErr = i;
      ಹಿಂತಿರುಗಿ -1;
    }
    i_ಆರಂಭ = pParse->nBlob;
    ಗಾಗಿ(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(pParse, j);
      ಆದರೆ( x<=0 ){
        ಪೂರ್ಣಾಂಕ op;
        ಆದರೆ( x==(-2) ){
          j = pParse->iErr;
          ಆದರೆ( pParse->nBlob!=(u32)i_ಆರಂಭ ) pParse->hasNonstd = 1;
          ನಿಲ್ಲಿಸು;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_ಪಠ್ಯ;
        ಆದರೆ( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          ಪೂರ್ಣಾಂಕ k = j+1;
          ಆಗ( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          ಪ್ರತಿಜ್ಞಾ( iBlob==pParse->nBlob );
          json_Blob_Append_ಗಂಟು(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಆದರೆ( x!=-1 ) pParse->iErr = j;
          ಹಿಂತಿರುಗಿ -1;
        }
      }
      ಆದರೆ( pParse->oom ) ಹಿಂತಿರುಗಿ -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      ಆದರೆ( t<JSONB_ಪಠ್ಯ || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        ಹಿಂತಿರುಗಿ -1;
      }
      j = x;
      ಆದರೆ( z[j]==':' ){
        j++;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          ಮಾಡು{ j++; }ಆಗ( jsonIsspace(z[j]) );
          ಆದರೆ( z[j]==':' ){
            j++;
            ಹೋಗು parse_object_ಮೌಲ್ಯ;
          }
        }
        x = json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(pParse, j);
        ಆದರೆ( x!=(-5) ){
          ಆದರೆ( x!=(-1) ) pParse->iErr = j;
          ಹಿಂತಿರುಗಿ -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_ಮೌಲ್ಯ:
      x = json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(pParse, j);
      ಆದರೆ( x<=0 ){
        ಆದರೆ( x!=(-1) ) pParse->iErr = j;
        ಹಿಂತಿರುಗಿ -1;
      }
      j = x;
      ಆದರೆ( z[j]==',' ){
        ಮುಂದುವರಿ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[j]=='}' ){
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ಆದರೆ( z[j]==',' ){
            ಮುಂದುವರಿ;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[j]=='}' ){
            ನಿಲ್ಲಿಸು;
          }
        }
        x = json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(pParse, j);
        ಆದರೆ( x==(-4) ){
          j = pParse->iErr;
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( x==(-2) ){
          j = pParse->iErr;
          ನಿಲ್ಲಿಸು;
        }
      }
      pParse->iErr = j;
      ಹಿಂತಿರುಗಿ -1;
    }
    json_Blob_Change_Payload_ಗಾತ್ರ(pParse, iThis, pParse->nBlob - i_ಆರಂಭ);
    pParse->i_ಆಳ--;
    ಹಿಂತಿರುಗಿ j+1;
  }
  ಸ್ಥಿತಿ '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    ಪ್ರತಿಜ್ಞಾ( i<=(u32)pParse->nJson );
    json_Blob_Append_ಗಂಟು(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    i_ಆರಂಭ = pParse->nBlob;
    ಆದರೆ( pParse->oom ) ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ( ++pParse->i_ಆಳ > JSON_MAX_ಆಳ ){
      pParse->iErr = i;
      ಹಿಂತಿರುಗಿ -1;
    }
    ಗಾಗಿ(j=i+1;;j++){
      x = json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(pParse, j);
      ಆದರೆ( x<=0 ){
        ಆದರೆ( x==(-3) ){
          j = pParse->iErr;
          ಆದರೆ( pParse->nBlob!=i_ಆರಂಭ ) pParse->hasNonstd = 1;
          ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ( x!=(-1) ) pParse->iErr = j;
        ಹಿಂತಿರುಗಿ -1;
      }
      j = x;
      ಆದರೆ( z[j]==',' ){
        ಮುಂದುವರಿ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[j]==']' ){
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ಆದರೆ( z[j]==',' ){
            ಮುಂದುವರಿ;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[j]==']' ){
            ನಿಲ್ಲಿಸು;
          }
        }
        x = json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(pParse, j);
        ಆದರೆ( x==(-4) ){
          j = pParse->iErr;
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( x==(-3) ){
          j = pParse->iErr;
          ನಿಲ್ಲಿಸು;
        }
      }
      pParse->iErr = j;
      ಹಿಂತಿರುಗಿ -1;
    }
    json_Blob_Change_Payload_ಗಾತ್ರ(pParse, iThis, pParse->nBlob - i_ಆರಂಭ);
    pParse->i_ಆಳ--;
    ಹಿಂತಿರುಗಿ j+1;
  }
  ಸ್ಥಿತಿ '\'': {
    u8 opcode;
    ಅಕ್ಷರ cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_ಪಠ್ಯ;
    ಹೋಗು parse_string;
  ಸ್ಥಿತಿ '"':
    /* Parse string */
    opcode = JSONB_ಪಠ್ಯ;
  parse_string:
    cDelim = z[i];
    j = i+1;
    ಆಗ( 1 /*exit-by-break*/ ){
      ಆದರೆ( jsonIsOk[(u8)z[j]] ){
        ಆದರೆ( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }ಇಲ್ಲದಿದ್ದರೆ{
          j += 3;
          ಮುಂದುವರಿ;
        }
      }
      c = z[j];
      ಆದರೆ( c==cDelim ){
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='\\' ){
        c = z[++j];
        ಆದರೆ( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          ಆದರೆ( opcode==JSONB_ಪಠ್ಯ ) opcode = JSONB_TEXTJ;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='\'' ||  c=='v' || c=='\n'
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#ಇಲ್ಲದಿದ್ದರೆ
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#ಆದರೆ_ಅಂತ್ಯ
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='\r' ){
          ಆದರೆ( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pParse->iErr = j;
          ಹಿಂತಿರುಗಿ -1;
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c<=0x1f ){
        ಆದರೆ( c==0 ){
          pParse->iErr = j;
          ಹಿಂತಿರುಗಿ -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_ಗಂಟು(pParse, opcode, j-1-i, &z[i+1]);
    ಹಿಂತಿರುಗಿ j+1;
  }
  ಸ್ಥಿತಿ 't': {
    ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      ಹಿಂತಿರುಗಿ i+4;
    }
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -1;
  }
  ಸ್ಥಿತಿ 'f': {
    ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      ಹಿಂತಿರುಗಿ i+5;
    }
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -1;
  }
  ಸ್ಥಿತಿ '+': {
    u8 ನೋಡಿದ_E;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    ಹೋಗು parse_ಸಂಖ್ಯೆ;
  ಸ್ಥಿತಿ '.':
    ಆದರೆ( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      ನೋಡಿದ_E = 0;
      ಹೋಗು parse_ಸಂಖ್ಯೆ_2;
    }
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -1;
  ಸ್ಥಿತಿ '-':
  ಸ್ಥಿತಿ '0':
  ಸ್ಥಿತಿ '1':
  ಸ್ಥಿತಿ '2':
  ಸ್ಥಿತಿ '3':
  ಸ್ಥಿತಿ '4':
  ಸ್ಥಿತಿ '5':
  ಸ್ಥಿತಿ '6':
  ಸ್ಥಿತಿ '7':
  ಸ್ಥಿತಿ '8':
  ಸ್ಥಿತಿ '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_ಸಂಖ್ಯೆ:
    ನೋಡಿದ_E = 0;
    ಪ್ರತಿಜ್ಞಾ( '-' < '0' );
    ಪ್ರತಿಜ್ಞಾ( '+' < '0' );
    ಪ್ರತಿಜ್ಞಾ( '.' < '0' );
    c = z[i];

    ಆದರೆ( c<='0' ){
      ಆದರೆ( c=='0' ){
        ಆದರೆ( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          ಪ್ರತಿಜ್ಞಾ( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          ಗಾಗಿ(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          ಹೋಗು parse_ಸಂಖ್ಯೆ_ಮುಕ್ತಾಯ;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          ಹಿಂತಿರುಗಿ -1;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          ಆದರೆ( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            ಆದರೆ( z[i]=='-' ){
              json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 6, "-9e999");
            }ಇಲ್ಲದಿದ್ದರೆ{
              json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 5, "9e999");
            }
            ಹಿಂತಿರುಗಿ i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          ಆದರೆ( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            ಹೋಗು parse_ಸಂಖ್ಯೆ_2;
          }
          pParse->iErr = i;
          ಹಿಂತಿರುಗಿ -1;
        }
        ಆದರೆ( z[i+1]=='0' ){
          ಆದರೆ( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            ಹಿಂತಿರುಗಿ -1;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            ಗಾಗಿ(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            ಹೋಗು parse_ಸಂಖ್ಯೆ_ಮುಕ್ತಾಯ;
          }
        }
      }
    }

  parse_ಸಂಖ್ಯೆ_2:
    ಗಾಗಿ(j=i+1;; j++){
      c = z[j];
      ಆದರೆ( sqlite3Isdigit(c) ) ಮುಂದುವರಿ;
      ಆದರೆ( c=='.' ){
        ಆದರೆ( (t & 0x02)!=0 ){
          pParse->iErr = j;
          ಹಿಂತಿರುಗಿ -1;
        }
        t |= 0x02;
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( c=='e' || c=='E' ){
        ಆದರೆ( z[j-1]<'0' ){
          ಆದರೆ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }ಇಲ್ಲದಿದ್ದರೆ{
            pParse->iErr = j;
            ಹಿಂತಿರುಗಿ -1;
          }
        }
        ಆದರೆ( ನೋಡಿದ_E ){
          pParse->iErr = j;
          ಹಿಂತಿರುಗಿ -1;
        }
        t |= 0x02;
        ನೋಡಿದ_E = 1;
        c = z[j+1];
        ಆದರೆ( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        ಆದರೆ( c<'0' || c>'9' ){
          pParse->iErr = j;
          ಹಿಂತಿರುಗಿ -1;
        }
        ಮುಂದುವರಿ;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( z[j-1]<'0' ){
      ಆದರೆ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pParse->iErr = j;
        ಹಿಂತಿರುಗಿ -1;
      }
    }
  parse_ಸಂಖ್ಯೆ_ಮುಕ್ತಾಯ:
    ಪ್ರತಿಜ್ಞಾ( JSONB_INT+0x01==JSONB_INT5 );
    ಪ್ರತಿಜ್ಞಾ( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    ಪ್ರತಿಜ್ಞಾ( JSONB_INT+0x02==JSONB_FLOAT );
    ಆದರೆ( z[i]=='+' ) i++;
    json_Blob_Append_ಗಂಟು(pParse, JSONB_INT+t, j-i, &z[i]);
    ಹಿಂತಿರುಗಿ j;
  }
  ಸ್ಥಿತಿ '}': {
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -2;  /* End of {...} */
  }
  ಸ್ಥಿತಿ ']': {
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -3;  /* End of [...] */
  }
  ಸ್ಥಿತಿ ',': {
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -4;  /* List separator */
  }
  ಸ್ಥಿತಿ ':': {
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -5;  /* Object label/value separator */
  }
  ಸ್ಥಿತಿ 0: {
    ಹಿಂತಿರುಗಿ 0;   /* End of file */
  }
  ಸ್ಥಿತಿ 0x09:
  ಸ್ಥಿತಿ 0x0a:
  ಸ್ಥಿತಿ 0x0d:
  ಸ್ಥಿತಿ 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    ಹೋಗು json_parse_restart;
  }
  ಸ್ಥಿತಿ 0x0b:
  ಸ್ಥಿತಿ 0x0c:
  ಸ್ಥಿತಿ '/':
  ಸ್ಥಿತಿ 0xc2:
  ಸ್ಥಿತಿ 0xe1:
  ಸ್ಥಿತಿ 0xe2:
  ಸ್ಥಿತಿ 0xe3:
  ಸ್ಥಿತಿ 0xef: {
    j = json5Whitespace(&z[i]);
    ಆದರೆ( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      ಹೋಗು json_parse_restart;
    }
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -1;
  }
  ಸ್ಥಿತಿ 'n': {
    ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      ಹಿಂತಿರುಗಿ i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  ಪೂರ್ವನಿಯೋಜಿತ: {
    u32 k;
    ಪೂರ್ಣಾಂಕ nn;
    c = z[i];
    ಗಾಗಿ(k=0; k<ಗಾತ್ರ(aNanInfName)/ಗಾತ್ರ(aNanInfName[0]); k++){
      ಆದರೆ( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) ಮುಂದುವರಿ;
      nn = aNanInfName[k].n;
      ಆದರೆ( sqlite3StrNICmp(&z[i], aNanInfName[k].z_ಹೊಂದಿಕೆ, nn)!=0 ){
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( sqlite3Isalnum(z[i+nn]) ) ಮುಂದುವರಿ;
      ಆದರೆ( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 5, "9e999");
      }ಇಲ್ಲದಿದ್ದರೆ{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      ಹಿಂತಿರುಗಿ i + nn;
    }
    pParse->iErr = i;
    ಹಿಂತಿರುಗಿ -1;  /* Syntax error */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Convert_ಪಠ್ಯ_ಗಮ್ಯ_Blob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zJson = pParse->zJson;
  i = json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(pParse, 0);
  ಆದರೆ( pParse->oom ) i = -1;
  ಆದರೆ( i>0 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಪ್ರತಿಜ್ಞಾ( pParse->i_ಆಳ==0 );
    ಆದರೆ( sqlite3Config.bJsonSelfcheck ){
      ಪ್ರತಿಜ್ಞಾ( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#ಆದರೆ_ಅಂತ್ಯ
    ಆಗ( jsonIsspace(zJson[i]) ) i++;
    ಆದರೆ( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      ಆದರೆ( zJson[i] ){
        ಆದರೆ( pCtx ) sqlite3_ಫಲಿತಾಂಶ_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        ಹಿಂತಿರುಗಿ 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  ಆದರೆ( i<=0 ){
    ಆದರೆ( pCtx!=0 ){
      ಆದರೆ( pParse->oom ){
        sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  ಪ್ರತಿಜ್ಞಾ( pStr->eErr==0 );
  memset(&px, 0, ಗಾತ್ರ(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->n_ಬಳಸಿದ;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (ಶೂನ್ಯ)json_Translate_ಪಠ್ಯ_ಗಮ್ಯ_Blob(&px, 0);
  ಆದರೆ( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(pStr->pCtx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( px.nBlobAlloc>0 );
    ಪ್ರತಿಜ್ಞಾ( !px.bReadOnly );
    sqlite3_ಫಲಿತಾಂಶ_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
ಸ್ಥಿರ u32 jsonb_Payload_ಗಾತ್ರ_Wide(
  ಸ್ಥಿರಾಂಕ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  ಪ್ರತಿಜ್ಞಾ( i<pParse->nBlob );
  ಪ್ರತಿಜ್ಞಾ( x==(pParse->aBlob[i]>>4) );
  ಪ್ರತಿಜ್ಞಾ( x>=13 );
  ಆದರೆ( x==13 ){
    ಆದರೆ( i+2>=pParse->nBlob ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x==14 ){
    ಆದರೆ( i+4>=pParse->nBlob ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
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
  ಆದರೆ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    ಹಿಂತಿರುಗಿ 0;
  }
  *pSz = sz;
  ಹಿಂತಿರುಗಿ n;
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
ಸ್ಥಿರ u32 jsonb_Payload_ಗಾತ್ರ(
  ಸ್ಥಿರಾಂಕ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ಆದರೆ( i>=pParse->nBlob ){
    *pSz = 0;
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x==12 ){
    ಆದರೆ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ jsonb_Payload_ಗಾತ್ರ_Wide(pParse, i, pSz, x);
  }
  ಆದರೆ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ಹಿಂತಿರುಗಿ 0;
  }
  *pSz = sz;
  ಹಿಂತಿರುಗಿ n;
}

#ಆದರೆ SQLITE_ಬಳಸುತ್ತದೆ_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
ಸ್ಥಿರ SQLITE_INLINE u32 jsonb_Payload_ಗಾತ್ರ_Inline(
  ಸ್ಥಿರಾಂಕ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ಆದರೆ( i>=pParse->nBlob ){
    *pSz = 0;
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x==12 ){
    ಆದರೆ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x==13 ){
    ಆದರೆ( i+2>=pParse->nBlob ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x==14 ){
    ಆದರೆ( i+4>=pParse->nBlob ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  ಆದರೆ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ಹಿಂತಿರುಗಿ 0;
  }
  *pSz = sz;
  ಹಿಂತಿರುಗಿ n;
}
#ಇಲ್ಲದಿದ್ದರೆ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_ಗಾತ್ರ_Inline(a,b,c) jsonb_Payload_ಗಾತ್ರ(a,b,c)
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ SQLITE_ಬಳಸುತ್ತದೆ_INLINE
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
ಸ್ಥಿರ SQLITE_INLINE u32 jsonb_Payload_ಗಾತ್ರ_Semi_Inline(
  ಸ್ಥಿರಾಂಕ JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ಪ್ರತಿಜ್ಞಾ( i<pParse->nBlob );
  ಆದರೆ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x==12 ){
    ಆದರೆ( i+1>=pParse->nBlob ){
      *pSz = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ jsonb_Payload_ಗಾತ್ರ_Wide(pParse,i,pSz,x);
  }
  ಆದರೆ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    ಹಿಂತಿರುಗಿ 0;
  }
  *pSz = sz;
  ಹಿಂತಿರುಗಿ n;
}
#ಇಲ್ಲದಿದ್ದರೆ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_ಗಾತ್ರ_Semi_Inline(a,b,c) jsonb_Payload_ಗಾತ್ರ(a,b,c)
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ u32 json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *p_ಹೊರಹೋಗು               /* Write JSON here */
){
  u32 sz, n, j, i_ಕೊನೆ;

  n = jsonb_Payload_ಗಾತ್ರ_Inline(pParse, i, &sz);
  ಆದರೆ( n==0 ){
    p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
    ಹಿಂತಿರುಗಿ pParse->nBlob+1;
  }
  ಆಯ್ಕೆ( pParse->aBlob[i] & 0x0f ){
    ಸ್ಥಿತಿ JSONB_NULL: {
      jsonAppendRawNZ(p_ಹೊರಹೋಗು, "null", 4);
      ಹಿಂತಿರುಗಿ i+1;
    }
    ಸ್ಥಿತಿ JSONB_TRUE: {
      jsonAppendRawNZ(p_ಹೊರಹೋಗು, "true", 4);
      ಹಿಂತಿರುಗಿ i+1;
    }
    ಸ್ಥಿತಿ JSONB_FALSE: {
      jsonAppendRawNZ(p_ಹೊರಹೋಗು, "false", 5);
      ಹಿಂತಿರುಗಿ i+1;
    }
    ಸ್ಥಿತಿ JSONB_INT:
    ಸ್ಥಿತಿ JSONB_FLOAT: {
      ಆದರೆ( sz==0 ) ಹೋಗು malformed_jsonb;
      jsonAppendRaw(p_ಹೊರಹೋಗು, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n], sz);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n];
      ಪೂರ್ಣಾಂಕ b_ಅತಿಪ್ರವಾಹ = 0;
      ಆದರೆ( sz==0 ) ಹೋಗು malformed_jsonb;
      ಆದರೆ( z_ಒಳ[0]=='-' ){
        jsonAppendChar(p_ಹೊರಹೋಗು, '-');
        k++;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_ಒಳ[0]=='+' ){
        k++;
      }
      ಗಾಗಿ(; k<sz; k++){
        ಆದರೆ( !sqlite3Isxdigit(z_ಒಳ[k]) ){
          p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (u>>60)!=0 ){
          b_ಅತಿಪ್ರವಾಹ = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          u = u*16 + sqlite3_Hex_ಗಮ್ಯ_Int(z_ಒಳ[k]);
        }
      }
      jsonPrintf(100,p_ಹೊರಹೋಗು,b_ಅತಿಪ್ರವಾಹ?"9.0e999":"%llu", u);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n];
      ಆದರೆ( sz==0 ) ಹೋಗು malformed_jsonb;
      ಆದರೆ( z_ಒಳ[0]=='-' ){
        jsonAppendChar(p_ಹೊರಹೋಗು, '-');
        ಆದರೆ( sz<=1 ) ಹೋಗು malformed_jsonb;
        k = 1;
      }
      ಆದರೆ( z_ಒಳ[k]=='.' ){
        jsonAppendChar(p_ಹೊರಹೋಗು, '0');
      }
      ಗಾಗಿ(; k<sz; k++){
        jsonAppendChar(p_ಹೊರಹೋಗು, z_ಒಳ[k]);
        ಆದರೆ( z_ಒಳ[k]=='.' && (k+1==sz || !sqlite3Isdigit(z_ಒಳ[k+1])) ){
          jsonAppendChar(p_ಹೊರಹೋಗು, '0');
        }
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_ಪಠ್ಯ:
    ಸ್ಥಿತಿ JSONB_TEXTJ: {
      ಆದರೆ( p_ಹೊರಹೋಗು->n_ಬಳಸಿದ+sz+2<=p_ಹೊರಹೋಗು->nAlloc || jsonStringGrow(p_ಹೊರಹೋಗು, sz+2)==0 ){
        p_ಹೊರಹೋಗು->zBuf[p_ಹೊರಹೋಗು->n_ಬಳಸಿದ] = '"';
        ಸ್ಮರಣೆ_ನಕಲು(p_ಹೊರಹೋಗು->zBuf+p_ಹೊರಹೋಗು->n_ಬಳಸಿದ+1,(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n],sz);
        p_ಹೊರಹೋಗು->zBuf[p_ಹೊರಹೋಗು->n_ಬಳಸಿದ+sz+1] = '"';
        p_ಹೊರಹೋಗು->n_ಬಳಸಿದ += sz+2;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_TEXT5: {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ;
      u32 k;
      u32 sz2 = sz;
      z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n];
      jsonAppendChar(p_ಹೊರಹೋಗು, '"');
      ಆಗ( sz2>0 ){
        ಗಾಗಿ(k=0; k<sz2 && (jsonIsOk[(u8)z_ಒಳ[k]] || z_ಒಳ[k]=='\''); k++){}
        ಆದರೆ( k>0 ){
          jsonAppendRawNZ(p_ಹೊರಹೋಗು, z_ಒಳ, k);
          ಆದರೆ( k>=sz2 ){
            ನಿಲ್ಲಿಸು;
          }
          z_ಒಳ += k;
          sz2 -= k;
        }
        ಆದರೆ( z_ಒಳ[0]=='"' ){
          jsonAppendRawNZ(p_ಹೊರಹೋಗು, "\\\"", 2);
          z_ಒಳ++;
          sz2--;
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( z_ಒಳ[0]<=0x1f ){
          ಆದರೆ( p_ಹೊರಹೋಗು->n_ಬಳಸಿದ+7>p_ಹೊರಹೋಗು->nAlloc && jsonStringGrow(p_ಹೊರಹೋಗು,7) ) ನಿಲ್ಲಿಸು;
          jsonAppendControlChar(p_ಹೊರಹೋಗು, z_ಒಳ[0]);
          z_ಒಳ++;
          sz2--;
          ಮುಂದುವರಿ;
        }
        ಪ್ರತಿಜ್ಞಾ( z_ಒಳ[0]=='\\' );
        ಪ್ರತಿಜ್ಞಾ( sz2>=1 );
        ಆದರೆ( sz2<2 ){
          p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
          ನಿಲ್ಲಿಸು;
        }
        ಆಯ್ಕೆ( (u8)z_ಒಳ[1] ){
          ಸ್ಥಿತಿ '\'':
            jsonAppendChar(p_ಹೊರಹೋಗು, '\'');
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ 'v':
            jsonAppendRawNZ(p_ಹೊರಹೋಗು, "\\u000b", 6);
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ 'x':
            ಆದರೆ( sz2<4 ){
              p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ನಿಲ್ಲಿಸು;
            }
            jsonAppendRawNZ(p_ಹೊರಹೋಗು, "\\u00", 4);
            jsonAppendRawNZ(p_ಹೊರಹೋಗು, &z_ಒಳ[2], 2);
            z_ಒಳ += 2;
            sz2 -= 2;
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ '0':
            jsonAppendRawNZ(p_ಹೊರಹೋಗು, "\\u0000", 6);
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ '\r':
            ಆದರೆ( sz2>2 && z_ಒಳ[2]=='\n' ){
              z_ಒಳ++;
              sz2--;
            }
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ '\n':
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            ಆದರೆ( sz2<4
             || 0x80!=(u8)z_ಒಳ[2]
             || (0xa8!=(u8)z_ಒಳ[3] && 0xa9!=(u8)z_ಒಳ[3])
            ){
              p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ನಿಲ್ಲಿಸು;
            }
            z_ಒಳ += 2;
            sz2 -= 2;
            ನಿಲ್ಲಿಸು;
          ಪೂರ್ವನಿಯೋಜಿತ:
            jsonAppendRawNZ(p_ಹೊರಹೋಗು, z_ಒಳ, 2);
            ನಿಲ್ಲಿಸು;
        }
        ಪ್ರತಿಜ್ಞಾ( sz2>=2 );
        z_ಒಳ += 2;
        sz2 -= 2;
      }
      jsonAppendChar(p_ಹೊರಹೋಗು, '"');
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_TEXTRAW: {
      jsonAppendString(p_ಹೊರಹೋಗು, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n], sz);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_ARRAY: {
      jsonAppendChar(p_ಹೊರಹೋಗು, '[');
      j = i+n;
      i_ಕೊನೆ = j+sz;
      ಆದರೆ( ++pParse->i_ಆಳ > JSON_MAX_ಆಳ ){
        jsonStringTooDeep(p_ಹೊರಹೋಗು);
      }
      ಆಗ( j<i_ಕೊನೆ && p_ಹೊರಹೋಗು->eErr==0 ){
        j = json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(pParse, j, p_ಹೊರಹೋಗು);
        jsonAppendChar(p_ಹೊರಹೋಗು, ',');
      }
      pParse->i_ಆಳ--;
      ಆದರೆ( j>i_ಕೊನೆ ) p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
      ಆದರೆ( sz>0 ) jsonStringTrimOneChar(p_ಹೊರಹೋಗು);
      jsonAppendChar(p_ಹೊರಹೋಗು, ']');
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_OBJECT: {
      ಪೂರ್ಣಾಂಕ x = 0;
      jsonAppendChar(p_ಹೊರಹೋಗು, '{');
      j = i+n;
      i_ಕೊನೆ = j+sz;
      ಆದರೆ( ++pParse->i_ಆಳ > JSON_MAX_ಆಳ ){
        jsonStringTooDeep(p_ಹೊರಹೋಗು);
      }
      ಆಗ( j<i_ಕೊನೆ && p_ಹೊರಹೋಗು->eErr==0 ){
        j = json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(pParse, j, p_ಹೊರಹೋಗು);
        jsonAppendChar(p_ಹೊರಹೋಗು, (x++ & 1) ? ',' : ':');
      }
      pParse->i_ಆಳ--;
      ಆದರೆ( (x & 1)!=0 || j>i_ಕೊನೆ ) p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
      ಆದರೆ( sz>0 ) jsonStringTrimOneChar(p_ಹೊರಹೋಗು);
      jsonAppendChar(p_ಹೊರಹೋಗು, '}');
      ನಿಲ್ಲಿಸು;
    }

    ಪೂರ್ವನಿಯೋಜಿತ: {
      malformed_jsonb:
      p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ i+n+sz;
}

/* Context for recursion of json_pretty()
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ JsonPretty JsonPretty;
ರಚನೆ JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *p_ಹೊರಹೋಗು;         /* Generate pretty output into this string */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
ಸ್ಥಿರ ಶೂನ್ಯ jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  ಗಾಗಿ(jj=0; jj<pPretty->nIndent; jj++){
    jsonAppendRaw(pPretty->p_ಹೊರಹೋಗು, pPretty->zIndent, pPretty->szIndent);
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
ಸ್ಥಿರ u32 json_Translate_Blob_ಗಮ್ಯ_Pretty_ಪಠ್ಯ(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, i_ಕೊನೆ;
  JsonParse *pParse = pPretty->pParse;
  JsonString *p_ಹೊರಹೋಗು = pPretty->p_ಹೊರಹೋಗು;
  n = jsonb_Payload_ಗಾತ್ರ(pParse, i, &sz);
  ಆದರೆ( n==0 ){
    p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
    ಹಿಂತಿರುಗಿ pParse->nBlob+1;
  }
  ಆಯ್ಕೆ( pParse->aBlob[i] & 0x0f ){
    ಸ್ಥಿತಿ JSONB_ARRAY: {
      j = i+n;
      i_ಕೊನೆ = j+sz;
      jsonAppendChar(p_ಹೊರಹೋಗು, '[');
      ಆದರೆ( j<i_ಕೊನೆ ){
        jsonAppendChar(p_ಹೊರಹೋಗು, '\n');
        pPretty->nIndent++;
        ಆದರೆ( pPretty->nIndent >= JSON_MAX_ಆಳ ){
          jsonStringTooDeep(p_ಹೊರಹೋಗು);
        }
        ಆಗ( p_ಹೊರಹೋಗು->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_ಗಮ್ಯ_Pretty_ಪಠ್ಯ(pPretty, j);
          ಆದರೆ( j>=i_ಕೊನೆ ){
            ಆದರೆ( j>i_ಕೊನೆ ) p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
            ನಿಲ್ಲಿಸು;
          }
          jsonAppendRawNZ(p_ಹೊರಹೋಗು, ",\n", 2);
        }
        jsonAppendChar(p_ಹೊರಹೋಗು, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_ಹೊರಹೋಗು, ']');
      i = i_ಕೊನೆ;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_OBJECT: {
      j = i+n;
      i_ಕೊನೆ = j+sz;
      jsonAppendChar(p_ಹೊರಹೋಗು, '{');
      ಆದರೆ( j<i_ಕೊನೆ ){
        jsonAppendChar(p_ಹೊರಹೋಗು, '\n');
        pPretty->nIndent++;
        ಆದರೆ( pPretty->nIndent >= JSON_MAX_ಆಳ ){
          jsonStringTooDeep(p_ಹೊರಹೋಗು);
        }
        pParse->i_ಆಳ = pPretty->nIndent;
        ಆಗ( p_ಹೊರಹೋಗು->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(pParse, j, p_ಹೊರಹೋಗು);
          ಆದರೆ( j>i_ಕೊನೆ ){
            p_ಹೊರಹೋಗು->eErr |= JSTRING_MALFORMED;
            ನಿಲ್ಲಿಸು;
          }
          jsonAppendRawNZ(p_ಹೊರಹೋಗು, ": ", 2);
          j = json_Translate_Blob_ಗಮ್ಯ_Pretty_ಪಠ್ಯ(pPretty, j);
          ಆದರೆ( j>=i_ಕೊನೆ ) ನಿಲ್ಲಿಸು;
          jsonAppendRawNZ(p_ಹೊರಹೋಗು, ",\n", 2);
        }
        jsonAppendChar(p_ಹೊರಹೋಗು, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_ಹೊರಹೋಗು, '}');
      i = i_ಕೊನೆ;
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      i = json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(pParse, i, p_ಹೊರಹೋಗು);
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
ಸ್ಥಿರ u32 jsonb_Array_ಎಣಿಕೆ(JsonParse *pParse, u32 i_ಬೇರು){
  u32 n, sz, i, i_ಕೊನೆ;
  u32 k = 0;
  n = jsonb_Payload_ಗಾತ್ರ(pParse, i_ಬೇರು, &sz);
  i_ಕೊನೆ = i_ಬೇರು+n+sz;
  ಗಾಗಿ(i=i_ಬೇರು+n; n>0 && i<i_ಕೊನೆ; i+=sz+n, k++){
    n = jsonb_Payload_ಗಾತ್ರ_Semi_Inline(pParse, i, &sz);
  }
  ಹಿಂತಿರುಗಿ k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_After_Edit_ಗಾತ್ರ_Adjust(JsonParse *pParse, u32 i_ಬೇರು){
  u32 sz = 0;
  u32 nBlob;
  ಪ್ರತಿಜ್ಞಾ( pParse->delta!=0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (ಶೂನ್ಯ)jsonb_Payload_ಗಾತ್ರ(pParse, i_ಬೇರು, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += json_Blob_Change_Payload_ಗಾತ್ರ(pParse, i_ಬೇರು, sz);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonBlobOverwrite(
  u8 *a_ಹೊರಹೋಗು,                 /* Overwrite here */
  ಸ್ಥಿರಾಂಕ u8 *aIns,           /* New content */
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
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  ಆದರೆ( (aIns[0]&0x0f)<=2 ) ಹಿಂತಿರುಗಿ 0;    /* Cannot enlarge NULL, true, false */
  ಆಯ್ಕೆ( aIns[0]>>4 ){
    ಪೂರ್ವನಿಯೋಜಿತ: {                         /* aIns[] header size 1 */
      ಆದರೆ( ((1<<d)&0x116)==0 ) ಹಿಂತಿರುಗಿ 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 12: {                         /* aIns[] header size is 2 */
      ಆದರೆ( ((1<<d)&0x8a)==0) ಹಿಂತಿರುಗಿ 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 13: {                         /* aIns[] header size is 3 */
      ಆದರೆ( d!=2 && d!=6 ) ಹಿಂತಿರುಗಿ 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 14: {                         /* aIns[] header size is 5 */
      ಆದರೆ( d!=4 ) ಹಿಂತಿರುಗಿ 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 15: {                         /* aIns[] header size is 9 */
      ಹಿಂತಿರುಗಿ 0;                          /* No solution */
    }
  }
  ಪ್ರತಿಜ್ಞಾ( i>=2 && i<=9 && aType[i-2]!=0 );
  a_ಹೊರಹೋಗು[0] = (aIns[0] & 0x0f) | aType[i-2];
  ಸ್ಮರಣೆ_ನಕಲು(&a_ಹೊರಹೋಗು[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  ಆಗ( 1/*edit-by-break*/ ){
    i--;
    a_ಹೊರಹೋಗು[i] = szPayload & 0xff;
    ಆದರೆ( i==1 ) ನಿಲ್ಲಿಸು;
    szPayload >>= 8;
  }
  ಪ್ರತಿಜ್ಞಾ( (szPayload>>8)==0 );
  ಹಿಂತಿರುಗಿ 1;
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
  ಸ್ಥಿರಾಂಕ u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  ಪ್ರತಿಜ್ಞಾ( pParse->nBlob >= (u64)iDel + (u64)nDel );
  ಆದರೆ( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (ಪೂರ್ಣಾಂಕ)-d)
  ){
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( d!=0 ){
    ಆದರೆ( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      ಆದರೆ( pParse->oom ) ಹಿಂತಿರುಗಿ;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  ಆದರೆ( nIns && aIns ){
    ಸ್ಮರಣೆ_ನಕಲು(&pParse->aBlob[iDel], aIns, nIns);
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
ಸ್ಥಿರ u32 json_ಬೈಟುಗಳು_ಗಮ್ಯ_Bypass(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, u32 n){
  u32 i = 0;
  ಆಗ( i+1<n ){
    ಆದರೆ( z[i]!='\\' ) ಹಿಂತಿರುಗಿ i;
    ಆದರೆ( z[i+1]=='\n' ){
      i += 2;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( z[i+1]=='\r' ){
      ಆದರೆ( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }ಇಲ್ಲದಿದ್ದರೆ{
        i += 2;
      }
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      ಮುಂದುವರಿ;
    }
    ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ i;
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
ಸ್ಥಿರ u32 jsonUnescapeOneChar(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, u32 n, u32 *pi_ಹೊರಹೋಗು){
  ಪ್ರತಿಜ್ಞಾ( n>0 );
  ಪ್ರತಿಜ್ಞಾ( z[0]=='\\' );
  ಆದರೆ( n<2 ){
    *pi_ಹೊರಹೋಗು = JSON_INVALID_CHAR;
    ಹಿಂತಿರುಗಿ n;
  }
  ಆಯ್ಕೆ( (u8)z[1] ){
    ಸ್ಥಿತಿ 'u': {
      u32 v, vlo;
      ಆದರೆ( n<6 ){
        *pi_ಹೊರಹೋಗು = JSON_INVALID_CHAR;
        ಹಿಂತಿರುಗಿ n;
      }
      v = json_Hex_ಗಮ್ಯ_Int4(&z[2]);
      ಆದರೆ( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = json_Hex_ಗಮ್ಯ_Int4(&z[8]))&0xfc00)==0xdc00
      ){
        *pi_ಹೊರಹೋಗು = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        ಹಿಂತಿರುಗಿ 12;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *pi_ಹೊರಹೋಗು = v;
        ಹಿಂತಿರುಗಿ 6;
      }
    }
    ಸ್ಥಿತಿ 'b': {   *pi_ಹೊರಹೋಗು = '\b';  ಹಿಂತಿರುಗಿ 2; }
    ಸ್ಥಿತಿ 'f': {   *pi_ಹೊರಹೋಗು = '\f';  ಹಿಂತಿರುಗಿ 2; }
    ಸ್ಥಿತಿ 'n': {   *pi_ಹೊರಹೋಗು = '\n';  ಹಿಂತಿರುಗಿ 2; }
    ಸ್ಥಿತಿ 'r': {   *pi_ಹೊರಹೋಗು = '\r';  ಹಿಂತಿರುಗಿ 2; }
    ಸ್ಥಿತಿ 't': {   *pi_ಹೊರಹೋಗು = '\t';  ಹಿಂತಿರುಗಿ 2; }
    ಸ್ಥಿತಿ 'v': {   *pi_ಹೊರಹೋಗು = '\v';  ಹಿಂತಿರುಗಿ 2; }
    ಸ್ಥಿತಿ '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *pi_ಹೊರಹೋಗು = 0;
#ಇಲ್ಲದಿದ್ದರೆ
      /* Correct behavior */
      *pi_ಹೊರಹೋಗು = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#ಆದರೆ_ಅಂತ್ಯ
      ಹಿಂತಿರುಗಿ 2;
    }
    ಸ್ಥಿತಿ '\'':
    ಸ್ಥಿತಿ '"':
    ಸ್ಥಿತಿ '/':
    ಸ್ಥಿತಿ '\\':{   *pi_ಹೊರಹೋಗು = z[1];  ಹಿಂತಿರುಗಿ 2; }
    ಸ್ಥಿತಿ 'x': {
      ಆದರೆ( n<4 ){
        *pi_ಹೊರಹೋಗು = JSON_INVALID_CHAR;
        ಹಿಂತಿರುಗಿ n;
      }
      *pi_ಹೊರಹೋಗು = (json_Hex_ಗಮ್ಯ_Int(z[2])<<4) | json_Hex_ಗಮ್ಯ_Int(z[3]);
      ಹಿಂತಿರುಗಿ 4;
    }
    ಸ್ಥಿತಿ 0xe2:
    ಸ್ಥಿತಿ '\r':
    ಸ್ಥಿತಿ '\n': {
      u32 nSkip = json_ಬೈಟುಗಳು_ಗಮ್ಯ_Bypass(z, n);
      ಆದರೆ( nSkip==0 ){
        *pi_ಹೊರಹೋಗು = JSON_INVALID_CHAR;
        ಹಿಂತಿರುಗಿ n;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nSkip==n ){
        *pi_ಹೊರಹೋಗು = 0;
        ಹಿಂತಿರುಗಿ n;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[nSkip]=='\\' ){
        ಹಿಂತಿರುಗಿ nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, pi_ಹೊರಹೋಗು);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, pi_ಹೊರಹೋಗು);
        ಹಿಂತಿರುಗಿ nSkip + sz;
      }
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      *pi_ಹೊರಹೋಗು = JSON_INVALID_CHAR;
      ಹಿಂತಿರುಗಿ 2;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ jsonLabelCompareEscaped(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಎಡ,          /* The left label */
  u32 n_ಎಡ,                  /* Size of the left label in bytes */
  ಪೂರ್ಣಾಂಕ raw_ಎಡ,                /* True if zLeft contains no escapes */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಲ,         /* The right label */
  u32 n_ಬಲ,                 /* Size of the right label in bytes */
  ಪೂರ್ಣಾಂಕ raw_ಬಲ                /* True if zRight is escape-free */
){
  u32 c_ಎಡ, c_ಬಲ;
  ಪ್ರತಿಜ್ಞಾ( raw_ಎಡ==0 || raw_ಬಲ==0 );
  ಆಗ( 1 /*exit-by-return*/ ){
    ಆದರೆ( n_ಎಡ==0 ){
      c_ಎಡ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( raw_ಎಡ || z_ಎಡ[0]!='\\' ){
      c_ಎಡ = ((u8*)z_ಎಡ)[0];
      ಆದರೆ( c_ಎಡ>=0xc0 ){
        ಪೂರ್ಣಾಂಕ sz = sqlite3Utf8ReadLimited((u8*)z_ಎಡ, n_ಎಡ, &c_ಎಡ);
        z_ಎಡ += sz;
        n_ಎಡ -= sz;
      }ಇಲ್ಲದಿದ್ದರೆ{
        z_ಎಡ++;
        n_ಎಡ--;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      u32 n = jsonUnescapeOneChar(z_ಎಡ, n_ಎಡ, &c_ಎಡ);
      z_ಎಡ += n;
      ಪ್ರತಿಜ್ಞಾ( n<=n_ಎಡ );
      n_ಎಡ -= n;
    }
    ಆದರೆ( n_ಬಲ==0 ){
      c_ಬಲ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( raw_ಬಲ || z_ಬಲ[0]!='\\' ){
      c_ಬಲ = ((u8*)z_ಬಲ)[0];
      ಆದರೆ( c_ಬಲ>=0xc0 ){
        ಪೂರ್ಣಾಂಕ sz = sqlite3Utf8ReadLimited((u8*)z_ಬಲ, n_ಬಲ, &c_ಬಲ);
        z_ಬಲ += sz;
        n_ಬಲ -= sz;
      }ಇಲ್ಲದಿದ್ದರೆ{
        z_ಬಲ++;
        n_ಬಲ--;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      u32 n = jsonUnescapeOneChar(z_ಬಲ, n_ಬಲ, &c_ಬಲ);
      z_ಬಲ += n;
      ಪ್ರತಿಜ್ಞಾ( n<=n_ಬಲ );
      n_ಬಲ -= n;
    }
    ಆದರೆ( c_ಎಡ!=c_ಬಲ ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( c_ಎಡ==0 ) ಹಿಂತಿರುಗಿ 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonLabelCompare(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಎಡ,          /* The left label */
  u32 n_ಎಡ,                  /* Size of the left label in bytes */
  ಪೂರ್ಣಾಂಕ raw_ಎಡ,                /* True if zLeft contains no escapes */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಲ,         /* The right label */
  u32 n_ಬಲ,                 /* Size of the right label in bytes */
  ಪೂರ್ಣಾಂಕ raw_ಬಲ                /* True if zRight is escape-free */
){
  ಆದರೆ( raw_ಎಡ && raw_ಬಲ ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    ಆದರೆ( n_ಎಡ!=n_ಬಲ ) ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ memcmp(z_ಎಡ, z_ಬಲ, n_ಎಡ)==0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ jsonLabelCompareEscaped(z_ಎಡ, n_ಎಡ, raw_ಎಡ,
                                   z_ಬಲ, n_ಬಲ, raw_ಬಲ);
  }
}

/*
** Error returns from jsonLookupStep()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_LOOKUP_ERROR      0xffffffff
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_LOOKUP_NOTFOUND   0xfffffffe
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_LOOKUP_NOTARRAY   0xfffffffd
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_LOOKUP_TOODEEP    0xfffffffc
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_LOOKUP_PATHERROR  0xfffffffb
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_LOOKUP_ISERROR(x) ((x)>=JSON_LOOKUP_PATHERROR)

/* Forward declaration */
ಸ್ಥಿರ u32 json_Lookup_ಹೆಜ್ಜೆ(JsonParse*,u32,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,u32);


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
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಾಲ   /* Tail of the path that determines substructure */
){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 ಖಾಲಿಗಳು_Object[] = { JSONB_ARRAY, JSONB_OBJECT };
  ಪೂರ್ಣಾಂಕ rc;
  memset(pIns, 0, ಗಾತ್ರ(*pIns));
  pIns->db = pParse->db;
  ಆದರೆ( z_ಬಾಲ[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&ಖಾಲಿಗಳು_Object[z_ಬಾಲ[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->i_ಆಳ = pParse->i_ಆಳ+1;
    ಆದರೆ( pIns->i_ಆಳ >= JSON_MAX_ಆಳ ){
      ಹಿಂತಿರುಗಿ JSON_LOOKUP_TOODEEP;
    }
    rc = json_Lookup_ಹೆಜ್ಜೆ(pIns, 0, z_ಬಾಲ, 0);
    pParse->i_ಆಳ--;
    pParse->oom |= pIns->oom;
  }
  ಹಿಂತಿರುಗಿ rc;  /* Error code only */
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
ಸ್ಥಿರ u32 json_Lookup_ಹೆಜ್ಜೆ(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_ಬೇರು,              /* Begin the search at this element of aBlob[] */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_ಕೀಲಿ, sz, n, i_ಕೊನೆ, rc;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ;
  u8 x;

  ಆದರೆ( z_ಮಾರ್ಗ[0]==0 ){
    ಆದರೆ( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonb_Payload_ಗಾತ್ರ(pParse, i_ಬೇರು, &sz);
      sz += n;
      ಆದರೆ( pParse->eEdit==JEDIT_DEL ){
        ಆದರೆ( iLabel>0 ){
          sz += i_ಬೇರು - iLabel;
          i_ಬೇರು = iLabel;
        }
        jsonBlobEdit(pParse, i_ಬೇರು, sz, 0, 0);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        ಆದರೆ( z_ಮಾರ್ಗ[-1]!=']' ){
          ಹಿಂತಿರುಗಿ JSON_LOOKUP_NOTARRAY;
        }ಇಲ್ಲದಿದ್ದರೆ{
          jsonBlobEdit(pParse, i_ಬೇರು, 0, pParse->aIns, pParse->nIns);
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_ಬೇರು, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    ಹಿಂತಿರುಗಿ i_ಬೇರು;
  }
  ಆದರೆ( z_ಮಾರ್ಗ[0]=='.' ){
    ಪೂರ್ಣಾಂಕ raw_ಕೀಲಿ = 1;
    x = pParse->aBlob[i_ಬೇರು];
    z_ಮಾರ್ಗ++;
    ಆದರೆ( z_ಮಾರ್ಗ[0]=='"' ){
      z_ಕೀಲಿ = z_ಮಾರ್ಗ + 1;
      ಗಾಗಿ(i=1; z_ಮಾರ್ಗ[i] && z_ಮಾರ್ಗ[i]!='"'; i++){
        ಆದರೆ( z_ಮಾರ್ಗ[i]=='\\' && z_ಮಾರ್ಗ[i+1]!=0 ) i++;
      }
      n_ಕೀಲಿ = i-1;
      ಆದರೆ( z_ಮಾರ್ಗ[i] ){
        i++;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ JSON_LOOKUP_PATHERROR;
      }
      testcase( n_ಕೀಲಿ==0 );
      raw_ಕೀಲಿ = memchr(z_ಕೀಲಿ, '\\', n_ಕೀಲಿ)==0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      z_ಕೀಲಿ = z_ಮಾರ್ಗ;
      ಗಾಗಿ(i=0; z_ಮಾರ್ಗ[i] && z_ಮಾರ್ಗ[i]!='.' && z_ಮಾರ್ಗ[i]!='['; i++){}
      n_ಕೀಲಿ = i;
      ಆದರೆ( n_ಕೀಲಿ==0 ){
        ಹಿಂತಿರುಗಿ JSON_LOOKUP_PATHERROR;
      }
    }
    ಆದರೆ( (x & 0x0f)!=JSONB_OBJECT ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_ಗಾತ್ರ(pParse, i_ಬೇರು, &sz);
    j = i_ಬೇರು + n;  /* j is the index of a label */
    i_ಕೊನೆ = j+sz;
    ಆಗ( j<i_ಕೊನೆ ){
      ಪೂರ್ಣಾಂಕ rawLabel;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      ಆದರೆ( x<JSONB_ಪಠ್ಯ || x>JSONB_TEXTRAW ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
      n = jsonb_Payload_ಗಾತ್ರ_Semi_Inline(pParse, j, &sz);
      ಆದರೆ( n==0 ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      ಆದರೆ( k+sz>=i_ಕೊನೆ ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
      zLabel = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[k];
      rawLabel = x==JSONB_ಪಠ್ಯ || x==JSONB_TEXTRAW;
      ಆದರೆ( jsonLabelCompare(z_ಕೀಲಿ, n_ಕೀಲಿ, raw_ಕೀಲಿ, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        ಆದರೆ( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
        n = jsonb_Payload_ಗಾತ್ರ(pParse, v, &sz);
        ಆದರೆ( n==0 || v+n+sz>i_ಕೊನೆ ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
        ಪ್ರತಿಜ್ಞಾ( j>0 );
        ಆದರೆ( ++pParse->i_ಆಳ >= JSON_MAX_ಆಳ ){
          ಹಿಂತಿರುಗಿ JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_ಹೆಜ್ಜೆ(pParse, v, &z_ಮಾರ್ಗ[i], j);
        pParse->i_ಆಳ--;
        ಆದರೆ( pParse->delta ) json_After_Edit_ಗಾತ್ರ_Adjust(pParse, i_ಬೇರು);
        ಹಿಂತಿರುಗಿ rc;
      }
      j = k+sz;
      ಆದರೆ( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
      n = jsonb_Payload_ಗಾತ್ರ_Semi_Inline(pParse, j, &sz);
      ಆದರೆ( n==0 ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ಆದರೆ( j>i_ಕೊನೆ ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
    ಆದರೆ( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_ಗಣ );
      testcase( pParse->eEdit==JEDIT_AINS );
      ಆದರೆ( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&z_ಮಾರ್ಗ[i])!=0 ){
        ಹಿಂತಿರುಗಿ JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, ಗಾತ್ರ(ix));
      ix.db = pParse->db;
      json_Blob_Append_ಗಂಟು(&ix, raw_ಕೀಲಿ?JSONB_TEXTRAW:JSONB_TEXT5, n_ಕೀಲಿ, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &z_ಮಾರ್ಗ[i]);
      ಆದರೆ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_ಕೀಲಿ+v.nBlob)
      ){
        ಪ್ರತಿಜ್ಞಾ( !pParse->oom );
        nIns = ix.nBlob + n_ಕೀಲಿ + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        ಆದರೆ( !pParse->oom ){
          ಪ್ರತಿಜ್ಞಾ( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          ಪ್ರತಿಜ್ಞಾ( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          ಸ್ಮರಣೆ_ನಕಲು(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          ಸ್ಮರಣೆ_ನಕಲು(&pParse->aBlob[k], z_ಕೀಲಿ, n_ಕೀಲಿ);
          k += n_ಕೀಲಿ;
          ಸ್ಮರಣೆ_ನಕಲು(&pParse->aBlob[k], v.aBlob, v.nBlob);
          ಆದರೆ( ALWAYS(pParse->delta) ) json_After_Edit_ಗಾತ್ರ_Adjust(pParse, i_ಬೇರು);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      ಹಿಂತಿರುಗಿ rc;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_ಮಾರ್ಗ[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_ಬೇರು] & 0x0f;
    ಆದರೆ( x!=JSONB_ARRAY )  ಹಿಂತಿರುಗಿ JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_ಗಾತ್ರ(pParse, i_ಬೇರು, &sz);
    i = 1;
    ಆಗ( sqlite3Isdigit(z_ಮಾರ್ಗ[i]) ){
      ಆದರೆ( kk<0xffffffff ) kk = kk*10 + z_ಮಾರ್ಗ[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    ಆದರೆ( i<2 || z_ಮಾರ್ಗ[i]!=']' ){
      ಆದರೆ( z_ಮಾರ್ಗ[1]=='#' ){
        kk = jsonb_Array_ಎಣಿಕೆ(pParse, i_ಬೇರು);
        i = 2;
        ಆದರೆ( z_ಮಾರ್ಗ[2]=='-' && sqlite3Isdigit(z_ಮಾರ್ಗ[3]) ){
          u64 nn = 0;
          i = 3;
          ಮಾಡು{
            ಆದರೆ( nn<0xffffffff ) nn = nn*10 + z_ಮಾರ್ಗ[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }ಆಗ( sqlite3Isdigit(z_ಮಾರ್ಗ[i]) );
          ಆದರೆ( nn>kk ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        ಆದರೆ( z_ಮಾರ್ಗ[i]!=']' ){
          ಹಿಂತಿರುಗಿ JSON_LOOKUP_PATHERROR;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_ಬೇರು+n;
    i_ಕೊನೆ = j+sz;
    ಆಗ( j<i_ಕೊನೆ ){
      ಆದರೆ( kk==0 ){
        ಆದರೆ( ++pParse->i_ಆಳ >= JSON_MAX_ಆಳ ){
          ಹಿಂತಿರುಗಿ JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_ಹೆಜ್ಜೆ(pParse, j, &z_ಮಾರ್ಗ[i+1], 0);
        pParse->i_ಆಳ--;
        ಆದರೆ( pParse->delta ) json_After_Edit_ಗಾತ್ರ_Adjust(pParse, i_ಬೇರು);
        ಹಿಂತಿರುಗಿ rc;
      }
      kk--;
      n = jsonb_Payload_ಗಾತ್ರ_Semi_Inline(pParse, j, &sz);
      ಆದರೆ( n==0 ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ಆದರೆ( j>i_ಕೊನೆ ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_ERROR;
    ಆದರೆ( kk>0 ) ಹಿಂತಿರುಗಿ JSON_LOOKUP_NOTFOUND;
    ಆದರೆ( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_ಗಣ );
      rc = jsonCreateEditSubstructure(pParse, &v, &z_ಮಾರ್ಗ[i+1]);
      ಆದರೆ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        ಪ್ರತಿಜ್ಞಾ( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      ಆದರೆ( pParse->delta ) json_After_Edit_ಗಾತ್ರ_Adjust(pParse, i_ಬೇರು);
      ಹಿಂತಿರುಗಿ rc;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ JSON_LOOKUP_PATHERROR; 
  }
  ಹಿಂತಿರುಗಿ JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Return_ಪಠ್ಯ_Json_From_Blob(
  sqlite3_context *ctx,
  ಸ್ಥಿರಾಂಕ u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  ಆದರೆ( NEVER(aBlob==0) ) ಹಿಂತಿರುಗಿ;
  memset(&x, 0, ಗಾತ್ರ(x));
  x.aBlob = (u8*)aBlob;
  x.nBlob = nBlob;
  jsonStringInit(&s, ctx);
  json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(&x, 0, &s);
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
  ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  ಪ್ರತಿಜ್ಞಾ( e_ಬಹುಲಕ>=0 && e_ಬಹುಲಕ<=2 );
  n = jsonb_Payload_ಗಾತ್ರ(pParse, i, &sz);
  ಆದರೆ( n==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error(pCtx, "malformed JSON", -1);
    ಹಿಂತಿರುಗಿ;
  }
  ಆಯ್ಕೆ( pParse->aBlob[i] & 0x0f ){
    ಸ್ಥಿತಿ JSONB_NULL: {
      ಆದರೆ( sz ) ಹೋಗು returnfromblob_malformed;
      sqlite3_ಫಲಿತಾಂಶ_null(pCtx);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_TRUE: {
      ಆದರೆ( sz ) ಹೋಗು returnfromblob_malformed;
      sqlite3_ಫಲಿತಾಂಶ_int(pCtx, 1);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_FALSE: {
      ಆದರೆ( sz ) ಹೋಗು returnfromblob_malformed;
      sqlite3_ಫಲಿತಾಂಶ_int(pCtx, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_INT5:
    ಸ್ಥಿತಿ JSONB_INT: {
      sqlite3_int64 iRes = 0;
      ಅಕ್ಷರ *z;
      ಪೂರ್ಣಾಂಕ bNeg = 0;
      ಅಕ್ಷರ x;
      ಆದರೆ( sz==0 ) ಹೋಗು returnfromblob_malformed;
      x = (ಅಕ್ಷರ)pParse->aBlob[i+n];
      ಆದರೆ( x=='-' ){
        ಆದರೆ( sz<2 ) ಹೋಗು returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n], (ಪೂರ್ಣಾಂಕ)sz);
      ಆದರೆ( z==0 ) ಹೋಗು returnfromblob_oom;
      rc = sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z, &iRes);
      sqlite3DbFree(db, z);
      ಆದರೆ( rc==0 ){
        ಆದರೆ( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          ದ್ವಿಗುಣ r;
          r = (ದ್ವಿಗುಣ)*(sqlite3_uint64*)&iRes;
          sqlite3_ಫಲಿತಾಂಶ_double(pCtx, bNeg ? -r : r);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_ಫಲಿತಾಂಶ_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==3 && bNeg ){
        sqlite3_ಫಲಿತಾಂಶ_int64(pCtx, ಅತಿಚಿಕ್ಕ_INT64);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==1 ){
        ಹೋಗು returnfromblob_malformed;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( bNeg ){ n--; sz++; }
        ಹೋಗು ಗಮ್ಯ_double;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_FLOAT5:
    ಸ್ಥಿತಿ JSONB_FLOAT: {
      ದ್ವಿಗುಣ r;
      ಅಕ್ಷರ *z;
      ಆದರೆ( sz==0 ) ಹೋಗು returnfromblob_malformed;
    ಗಮ್ಯ_double:
      z = sqlite3DbStrNDup(db, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n], (ಪೂರ್ಣಾಂಕ)sz);
      ಆದರೆ( z==0 ) ಹೋಗು returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      ಆದರೆ( rc<=0 ) ಹೋಗು returnfromblob_malformed;
      sqlite3_ಫಲಿತಾಂಶ_double(pCtx, r);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_TEXTRAW:
    ಸ್ಥಿತಿ JSONB_ಪಠ್ಯ: {
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pCtx, (ಅಕ್ಷರ*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_TEXT5:
    ಸ್ಥಿತಿ JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 i_ಒಳ, i_ಹೊರಹೋಗು;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
      ಅಕ್ಷರ *z_ಹೊರಹೋಗು;
      u32 n_ಹೊರಹೋಗು = sz;
      z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pParse->aBlob[i+n];
      z_ಹೊರಹೋಗು = sqlite3DbMallocRaw(db, ((u64)n_ಹೊರಹೋಗು)+1);
      ಆದರೆ( z_ಹೊರಹೋಗು==0 ) ಹೋಗು returnfromblob_oom;
      ಗಾಗಿ(i_ಒಳ=i_ಹೊರಹೋಗು=0; i_ಒಳ<sz; i_ಒಳ++){
        ಅಕ್ಷರ c = z[i_ಒಳ];
        ಆದರೆ( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[i_ಒಳ], sz-i_ಒಳ, &v);
          ಆದರೆ( v==0 && i_ಒಳ+szEscape==sz
           && (z[i_ಒಳ+1]=='\n' || z[i_ಒಳ+1]=='\r' || (u8)z[i_ಒಳ+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v<=0x7f ){
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = (ಅಕ್ಷರ)v;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v<=0x7ff ){
            ಪ್ರತಿಜ್ಞಾ( szEscape>=2 );
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = (ಅಕ್ಷರ)(0xc0 | (v>>6));
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = 0x80 | (v&0x3f);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v<0x10000 ){
            ಪ್ರತಿಜ್ಞಾ( szEscape>=3 );
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = 0xe0 | (v>>12);
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = 0x80 | ((v>>6)&0x3f);
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = 0x80 | (v&0x3f);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪ್ರತಿಜ್ಞಾ( szEscape>=4 );
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = 0xf0 | (v>>18);
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = 0x80 | ((v>>12)&0x3f);
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = 0x80 | ((v>>6)&0x3f);
            z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = 0x80 | (v&0x3f);
          }
          i_ಒಳ += szEscape - 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು++] = c;
        }
      } /* end for() */
      ಪ್ರತಿಜ್ಞಾ( i_ಹೊರಹೋಗು<=n_ಹೊರಹೋಗು );
      z_ಹೊರಹೋಗು[i_ಹೊರಹೋಗು] = 0;
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pCtx, z_ಹೊರಹೋಗು, i_ಹೊರಹೋಗು, SQLITE_DYNAMIC);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JSONB_ARRAY:
    ಸ್ಥಿತಿ JSONB_OBJECT: {
      ಆದರೆ( e_ಬಹುಲಕ==0 ){
        ಆದರೆ( (SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(pCtx)) & JSON_BLOB)!=0 ){
          e_ಬಹುಲಕ = 2;
        }ಇಲ್ಲದಿದ್ದರೆ{
          e_ಬಹುಲಕ = 1;
        }
      }
      ಆದರೆ( e_ಬಹುಲಕ==2 ){
        sqlite3_ಫಲಿತಾಂಶ_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }ಇಲ್ಲದಿದ್ದರೆ{
        json_Return_ಪಠ್ಯ_Json_From_Blob(pCtx, &pParse->aBlob[i], sz+n);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಹೋಗು returnfromblob_malformed;
    }
  }
  ಹಿಂತಿರುಗಿ;

returnfromblob_oom:
  sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
  ಹಿಂತಿರುಗಿ;

returnfromblob_malformed:
  sqlite3_ಫಲಿತಾಂಶ_error(pCtx, "malformed JSON", -1);
  ಹಿಂತಿರುಗಿ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Function_Arg_ಗಮ್ಯ_Blob(
  sqlite3_context *ctx,
  sqlite3_ಮೌಲ್ಯ *pArg,
  JsonParse *pParse
){
  ಪೂರ್ಣಾಂಕ eType = sqlite3_ಮೌಲ್ಯ_type(pArg);
  ಸ್ಥಿರ u8 aNull[] = { 0x00 };
  memset(pParse, 0, ಗಾತ್ರ(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  ಆಯ್ಕೆ( eType ){
    ಪೂರ್ವನಿಯೋಜಿತ: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಸ್ಥಿತಿ SQLITE_BLOB: {
      ಆದರೆ( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_ಫಲಿತಾಂಶ_error(ctx, "JSON cannot hold BLOB values", -1);
        ಹಿಂತಿರುಗಿ 1;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_ಪಠ್ಯ: {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zJson = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pArg);
      ಪೂರ್ಣಾಂಕ nJson = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(pArg);
      ಆದರೆ( zJson==0 ) ಹಿಂತಿರುಗಿ 1;
      ಆದರೆ( sqlite3_ಮೌಲ್ಯ_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (ಅಕ್ಷರ*)zJson;
        pParse->nJson = nJson;
        ಆದರೆ( json_Convert_ಪಠ್ಯ_ಗಮ್ಯ_Blob(pParse, ctx) ){
          sqlite3_ಫಲಿತಾಂಶ_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, ಗಾತ್ರ(pParse[0]));
          ಹಿಂತಿರುಗಿ 1;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        json_Blob_Append_ಗಂಟು(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_FLOAT: {
      ಆದರೆ( NEVER(sqlite3IsNaN(sqlite3_ಮೌಲ್ಯ_double(pArg))) ){
        json_Blob_Append_ಗಂಟು(pParse, JSONB_NULL, 0, 0);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(pArg);
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pArg);
        ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ 1;
        ಆದರೆ( z[0]=='I' ){
          json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 5, "9e999");
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, 6, "-9e999");
        }ಇಲ್ಲದಿದ್ದರೆ{
          json_Blob_Append_ಗಂಟು(pParse, JSONB_FLOAT, n, z);
        }
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_INTEGER: {
      ಪೂರ್ಣಾಂಕ n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(pArg);
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pArg);
      ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ 1;
      json_Blob_Append_ಗಂಟು(pParse, JSONB_INT, n, z);
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( pParse->oom ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಅಕ್ಷರ *json_Bad_ಮಾರ್ಗ_Error(
  sqlite3_context *ctx,     /* The function call containing the error */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,        /* The path with the problem */
  ಪೂರ್ಣಾಂಕ rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  ಅಕ್ಷರ *z_ಸಂದೇಶ;
  ಆದರೆ( rc==(ಪೂರ್ಣಾಂಕ)JSON_LOOKUP_NOTARRAY ){
    z_ಸಂದೇಶ = sqlite3_mprintf("not an array element: %Q", z_ಮಾರ್ಗ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==(ಪೂರ್ಣಾಂಕ)JSON_LOOKUP_ERROR ){
    z_ಸಂದೇಶ = sqlite3_mprintf("malformed JSON");
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==(ಪೂರ್ಣಾಂಕ)JSON_LOOKUP_TOODEEP ){
    z_ಸಂದೇಶ = sqlite3_mprintf("JSON path too deep");
  }ಇಲ್ಲದಿದ್ದರೆ{
    z_ಸಂದೇಶ = sqlite3_mprintf("bad JSON path: %Q", z_ಮಾರ್ಗ);
  }
  ಆದರೆ( ctx==0 ) ಹಿಂತಿರುಗಿ z_ಸಂದೇಶ;
  ಆದರೆ( z_ಸಂದೇಶ ){
    sqlite3_ಫಲಿತಾಂಶ_error(ctx, z_ಸಂದೇಶ, -1);
    sqlite3_free(z_ಸಂದೇಶ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
  }
  ಹಿಂತಿರುಗಿ 0;
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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv,
  ಪೂರ್ಣಾಂಕ eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  ಪೂರ್ಣಾಂಕ i;
  u32 rc = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ = 0;
  ಪೂರ್ಣಾಂಕ flgs;
  JsonParse *p;
  JsonParse ax;

  ಪ್ರತಿಜ್ಞಾ( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(i=1; i<argc-1; i+=2){
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[i])==SQLITE_NULL ) ಮುಂದುವರಿ;
    z_ಮಾರ್ಗ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
    ಆದರೆ( z_ಮಾರ್ಗ==0 ){
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
      jsonParseFree(p);
      ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ( z_ಮಾರ್ಗ[0]!='$' ) ಹೋಗು json_ಸೇರಿಸು_Into_Blob_patherror;
    ಆದರೆ( json_Function_Arg_ಗಮ್ಯ_Blob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ( z_ಮಾರ್ಗ[1]==0 ){
      ಆದರೆ( eEdit==JEDIT_REPL || eEdit==JEDIT_ಗಣ ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }ಇಲ್ಲದಿದ್ದರೆ{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->i_ಆಳ = 0;
      rc = json_Lookup_ಹೆಜ್ಜೆ(p, 0, z_ಮಾರ್ಗ+1, 0);
    }
    jsonParseReset(&ax);
    ಆದರೆ( rc==JSON_LOOKUP_NOTFOUND ) ಮುಂದುವರಿ;
    ಆದರೆ( JSON_LOOKUP_ISERROR(rc) ) ಹೋಗು json_ಸೇರಿಸು_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ಹಿಂತಿರುಗಿ;

json_ಸೇರಿಸು_Into_Blob_patherror:
  jsonParseFree(p);
  json_Bad_ಮಾರ್ಗ_Error(ctx, z_ಮಾರ್ಗ, rc);
  ಹಿಂತಿರುಗಿ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonArgIsJsonb(sqlite3_ಮೌಲ್ಯ *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(pArg)!=SQLITE_BLOB ) ಹಿಂತಿರುಗಿ 0;
  p->aBlob = (u8*)sqlite3_ಮೌಲ್ಯ_blob(pArg);
  p->nBlob = (u32)sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(pArg);
  ಆದರೆ( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonb_Payload_ಗಾತ್ರ(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    ಹಿಂತಿರುಗಿ 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  ಹಿಂತಿರುಗಿ 0;
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
  ಪೂರ್ಣಾಂಕ eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  ಪ್ರತಿಜ್ಞಾ( ctx!=0 );
  eType = sqlite3_ಮೌಲ್ಯ_type(pArg);
  ಆದರೆ( eType==SQLITE_NULL ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_ಹುಡುಕು(ctx, pArg);
  }
  ಆದರೆ( pFromCache ){
    pFromCache->nJPRef++;
    ಆದರೆ( (flgs & JSON_EDITABLE)==0 ){
      ಹಿಂತಿರುಗಿ pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, ಗಾತ್ರ(*p));
  ಆದರೆ( p==0 ) ಹೋಗು json_pfa_oom;
  memset(p, 0, ಗಾತ್ರ(*p));
  p->db = db;
  p->nJPRef = 1;
  ಆದರೆ( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    ಆದರೆ( p->aBlob==0 ) ಹೋಗು json_pfa_oom;
    ಸ್ಮರಣೆ_ನಕಲು(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    ಹಿಂತಿರುಗಿ p;
  }
  ಆದರೆ( eType==SQLITE_BLOB ){
    ಆದರೆ( jsonArgIsJsonb(pArg,p) ){
      ಆದರೆ( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        ಹೋಗು json_pfa_oom;
      }
      ಹಿಂತಿರುಗಿ p;
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
  p->zJson = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pArg);
  p->nJson = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(pArg);
  ಆದರೆ( db->mallocFailed ) ಹೋಗು json_pfa_oom;
  ಆದರೆ( p->nJson==0 ) ಹೋಗು json_pfa_malformed;
  ಪ್ರತಿಜ್ಞಾ( p->zJson!=0 );
  ಆದರೆ( json_Convert_ಪಠ್ಯ_ಗಮ್ಯ_Blob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    ಆದರೆ( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      ಹಿಂತಿರುಗಿ p;
    }ಇಲ್ಲದಿದ್ದರೆ{
      jsonParseFree(p);
      ಹಿಂತಿರುಗಿ 0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ isRCStr = sqlite3_ಮೌಲ್ಯ_Is_ದ_Class(pArg, sqlite3RCStrUnref);
    ಪೂರ್ಣಾಂಕ rc;
    ಆದರೆ( !isRCStr ){
      ಅಕ್ಷರ *zNew = sqlite3RCStrNew( p->nJson );
      ಆದರೆ( zNew==0 ) ಹೋಗು json_pfa_oom;
      ಸ್ಮರಣೆ_ನಕಲು(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_ಸೇರಿಸು(ctx, p);
    ಆದರೆ( rc==SQLITE_NOMEM ) ಹೋಗು json_pfa_oom;
    ಆದರೆ( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      ಹೋಗು rebuild_from_cache;
    }
  }
  ಹಿಂತಿರುಗಿ p;

json_pfa_malformed:
  ಆದರೆ( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    ಹಿಂತಿರುಗಿ p;
  }ಇಲ್ಲದಿದ್ದರೆ{
    jsonParseFree(p);
    sqlite3_ಫಲಿತಾಂಶ_error(ctx, "malformed JSON", -1);
    ಹಿಂತಿರುಗಿ 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
  ಹಿಂತಿರುಗಿ 0;
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
  ಪೂರ್ಣಾಂಕ flgs;
  ಆದರೆ( p->oom ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
    ಹಿಂತಿರುಗಿ;
  }
  flgs = SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(ctx));
  ಆದರೆ( flgs & JSON_BLOB ){
    ಆದರೆ( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_ಫಲಿತಾಂಶ_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಫಲಿತಾಂಶ_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
  }
}

/****************************************************************************
** SQL functions used for testing and debugging
****************************************************************************/

#ಆದರೆ SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 i_ಆರಂಭ,        /* Start rendering here */
  u32 i_ಕೊನೆ,          /* Do not render this byte or any byte after this one */
  ಪೂರ್ಣಾಂಕ nIndent,       /* Indent by this many spaces */
  sqlite3_str *p_ಹೊರಹೋಗು  /* Generate output into this sqlite3_str object */
){
  ಆಗ( i_ಆರಂಭ<i_ಕೊನೆ ){
    u32 i, n, nn, sz = 0;
    ಪೂರ್ಣಾಂಕ showContent = 1;
    u8 x = pParse->aBlob[i_ಆರಂಭ] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(p_ಹೊರಹೋಗು, "%5d:%*s", i_ಆರಂಭ, nIndent, "");
    ಆದರೆ( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonb_Payload_ಗಾತ್ರ(pParse, i_ಆರಂಭ, &sz);
    ಆದರೆ( nn==0 ) nn = 1;
    ಆದರೆ( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    ಗಾಗಿ(i=0; i<nn; i++){
      sqlite3_str_appendf(p_ಹೊರಹೋಗು, " %02x", pParse->aBlob[i_ಆರಂಭ+i]);
    }
    ಆದರೆ( n==0 ){
      sqlite3_str_appendf(p_ಹೊರಹೋಗು, "   ERROR invalid node size\n");
      i_ಆರಂಭ = n==0 ? i_ಆರಂಭ+1 : i_ಕೊನೆ;
      ಮುಂದುವರಿ;
    }
    pParse->nBlob = savedNBlob;
    ಆದರೆ( i_ಆರಂಭ+n+sz>i_ಕೊನೆ ){
      i_ಕೊನೆ = i_ಆರಂಭ+n+sz;
      ಆದರೆ( i_ಕೊನೆ>pParse->nBlob ){
        ಆದರೆ( pParse->nBlobAlloc>0 && i_ಕೊನೆ>pParse->nBlobAlloc ){
          i_ಕೊನೆ = pParse->nBlobAlloc;
        }ಇಲ್ಲದಿದ್ದರೆ{
          i_ಕೊನೆ = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(p_ಹೊರಹೋಗು,"  <-- ");
    ಆಯ್ಕೆ( x ){
      ಸ್ಥಿತಿ JSONB_NULL:     sqlite3_str_appendall(p_ಹೊರಹೋಗು,"null"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_TRUE:     sqlite3_str_appendall(p_ಹೊರಹೋಗು,"true"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_FALSE:    sqlite3_str_appendall(p_ಹೊರಹೋಗು,"false"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_INT:      sqlite3_str_appendall(p_ಹೊರಹೋಗು,"int"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_INT5:     sqlite3_str_appendall(p_ಹೊರಹೋಗು,"int5"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_FLOAT:    sqlite3_str_appendall(p_ಹೊರಹೋಗು,"float"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_FLOAT5:   sqlite3_str_appendall(p_ಹೊರಹೋಗು,"float5"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_ಪಠ್ಯ:     sqlite3_str_appendall(p_ಹೊರಹೋಗು,"text"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_TEXTJ:    sqlite3_str_appendall(p_ಹೊರಹೋಗು,"textj"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_TEXT5:    sqlite3_str_appendall(p_ಹೊರಹೋಗು,"text5"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_TEXTRAW:  sqlite3_str_appendall(p_ಹೊರಹೋಗು,"textraw"); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ JSONB_ARRAY: {
        sqlite3_str_appendf(p_ಹೊರಹೋಗು,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_ಆರಂಭ+n, i_ಆರಂಭ+n+sz, nIndent+2, p_ಹೊರಹೋಗು);
        showContent = 0;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ JSONB_OBJECT: {
        sqlite3_str_appendf(p_ಹೊರಹೋಗು, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_ಆರಂಭ+n, i_ಆರಂಭ+n+sz, nIndent+2, p_ಹೊರಹೋಗು);
        showContent = 0;
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        sqlite3_str_appendall(p_ಹೊರಹೋಗು, "ERROR: unknown node type\n");
        showContent = 0;
        ನಿಲ್ಲಿಸು;
      }
    }
    ಆದರೆ( showContent ){
      ಆದರೆ( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(p_ಹೊರಹೋಗು, "\n", 1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        u32 j;
        sqlite3_str_appendall(p_ಹೊರಹೋಗು, ": \"");
        ಗಾಗಿ(j=i_ಆರಂಭ+n; j<i_ಆರಂಭ+n+sz; j++){
          u8 c = pParse->aBlob[j];
          ಆದರೆ( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(p_ಹೊರಹೋಗು, (ಅಕ್ಷರ*)&c, 1);
        }
        sqlite3_str_append(p_ಹೊರಹೋಗು, "\"\n", 2);
      }
    }
    i_ಆರಂಭ += n + sz;
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonShowParse(JsonParse *pParse){
  sqlite3_str ಹೊರಹೋಗು;
  ಅಕ್ಷರ zBuf[1000];
  ಆದರೆ( pParse==0 ){
    ಬರೆ("NULL pointer\n");
    ಹಿಂತಿರುಗಿ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಬರೆ("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    ಬರೆ("nBlob = %u\n", pParse->nBlob);
    ಬರೆ("delta = %d\n", pParse->delta);
    ಆದರೆ( pParse->nBlob==0 ) ಹಿಂತಿರುಗಿ;
    ಬರೆ("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&ಹೊರಹೋಗು, 0, zBuf, ಗಾತ್ರ(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &ಹೊರಹೋಗು);
  ಬರೆ("%s", sqlite3_str_ಮೌಲ್ಯ(&ಹೊರಹೋಗು));
  sqlite3_str_reset(&ಹೊರಹೋಗು);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** SQL function:   json_parse(JSON)
**
** Parse JSON using jsonParseFuncArg().  Return text that is a
** human-readable dump of the binary JSONB for the input parameter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonParseFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str ಹೊರಹೋಗು;

  ಪ್ರತಿಜ್ಞಾ( argc>=1 );
  sqlite3StrAccumInit(&ಹೊರಹೋಗು, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &ಹೊರಹೋಗು);
    sqlite3_ಫಲಿತಾಂಶ_text64(ctx,ಹೊರಹೋಗು.z_ಪಠ್ಯ,ಹೊರಹೋಗು.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }ಇಲ್ಲದಿದ್ದರೆ{
    jsonShowParse(p);
  }
  jsonParseFree(p);
  sqlite3_str_reset(&ಹೊರಹೋಗು);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonString jx;
  ಬಳಸದ_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_ಮೌಲ್ಯ(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonArrayFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  ಗಾಗಿ(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_ಮೌಲ್ಯ(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Array_ಉದ್ದಳತೆ_Func(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( argc==2 ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
    ಆದರೆ( z_ಮಾರ್ಗ==0 ){
      jsonParseFree(p);
      ಹಿಂತಿರುಗಿ;
    }
    i = json_Lookup_ಹೆಜ್ಜೆ(p, 0, z_ಮಾರ್ಗ[0]=='$' ? z_ಮಾರ್ಗ+1 : "@", 0);
    ಆದರೆ( JSON_LOOKUP_ISERROR(i) ){
      ಆದರೆ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ಇಲ್ಲದಿದ್ದರೆ{
        json_Bad_ಮಾರ್ಗ_Error(ctx, z_ಮಾರ್ಗ, i);
      }
      eErr = 1;
      i = 0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    i = 0;
  }
  ಆದರೆ( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_ಎಣಿಕೆ(p, i);
  }
  ಆದರೆ( !eErr ) sqlite3_ಫಲಿತಾಂಶ_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonAllAlphanum(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ n){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  ಹಿಂತಿರುಗಿ i==n;
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
ಸ್ಥಿರ ಶೂನ್ಯ json_ಹೊರತೆಗೆ_Func(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p = 0;      /* The parse */
  ಪೂರ್ಣಾಂಕ flags;             /* Flags associated with the function */
  ಪೂರ್ಣಾಂಕ i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  ಆದರೆ( argc<2 ) ಹಿಂತಿರುಗಿ;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  flags = SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(ctx));
  jsonStringInit(&jx, ctx);
  ಆದರೆ( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  ಗಾಗಿ(i=1; i<argc; i++){
    /* With a single PATH argument */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
    ಪೂರ್ಣಾಂಕ n_ಮಾರ್ಗ;
    u32 j;
    ಆದರೆ( z_ಮಾರ್ಗ==0 ) ಹೋಗು json_ಹೊರತೆಗೆ_error;
    n_ಮಾರ್ಗ = sqlite3Strlen30(z_ಮಾರ್ಗ);
    ಆದರೆ( z_ಮಾರ್ಗ[0]=='$' ){
      j = json_Lookup_ಹೆಜ್ಜೆ(p, 0, z_ಮಾರ್ಗ+1, 0);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags & JSON_ABPATH) ){
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
      ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        ಆದರೆ( z_ಮಾರ್ಗ[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, z_ಮಾರ್ಗ, n_ಮಾರ್ಗ);
        jsonAppendRawNZ(&jx, "]", 2);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( jsonAllAlphanum(z_ಮಾರ್ಗ, n_ಮಾರ್ಗ) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, z_ಮಾರ್ಗ, n_ಮಾರ್ಗ);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z_ಮಾರ್ಗ[0]=='[' && n_ಮಾರ್ಗ>=3 && z_ಮಾರ್ಗ[n_ಮಾರ್ಗ-1]==']' ){
        jsonAppendRaw(&jx, z_ಮಾರ್ಗ, n_ಮಾರ್ಗ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, z_ಮಾರ್ಗ, n_ಮಾರ್ಗ);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = json_Lookup_ಹೆಜ್ಜೆ(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }ಇಲ್ಲದಿದ್ದರೆ{
      json_Bad_ಮಾರ್ಗ_Error(ctx, z_ಮಾರ್ಗ, 0);
      ಹೋಗು json_ಹೊರತೆಗೆ_error;
    }
    ಆದರೆ( j<p->nBlob ){
      ಆದರೆ( argc==2 ){
        ಆದರೆ( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          ಪ್ರತಿಜ್ಞಾ( (flags & JSON_BLOB)==0 );
          sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
        }ಇಲ್ಲದಿದ್ದರೆ{
          jsonReturnFromBlob(p, j, ctx, 0);
          ಆದರೆ( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        jsonAppendSeparator(&jx);
        json_Translate_Blob_ಗಮ್ಯ_ಪಠ್ಯ(p, j, &jx);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( j==JSON_LOOKUP_NOTFOUND ){
      ಆದರೆ( argc==2 ){
        ಹೋಗು json_ಹೊರತೆಗೆ_error;  /* Return NULL if not found */
      }ಇಲ್ಲದಿದ್ದರೆ{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      json_Bad_ಮಾರ್ಗ_Error(ctx, z_ಮಾರ್ಗ, j);
      ಹೋಗು json_ಹೊರತೆಗೆ_error;
    }
  }
  ಆದರೆ( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    ಆದರೆ( (flags & JSON_BLOB)==0 ){
      sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_ಹೊರತೆಗೆ_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  ಹಿಂತಿರುಗಿ;
}

/*
** Return codes for jsonMergePatch()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_ವಿಲೀನ_OK          0     /* Success */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_ವಿಲೀನ_BADTARGET   1     /* Malformed TARGET blob */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_ವಿಲೀನ_BADPATCH    2     /* Malformed PATCH blob */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_ವಿಲೀನ_OOM         3     /* Out-of-memory condition */
#ವ್ಯಾಖ್ಯಾನಿಸು JSON_ವಿಲೀನ_TOODEEP     4     /* Nested too deep */

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_ವಿಲೀನ_Patch(
  JsonParse *p_ಗುರಿ,      /* The JSON parser that contains the TARGET */
  u32 i_ಗುರಿ,             /* Index of TARGET in pTarget->aBlob[] */
  ಸ್ಥಿರಾಂಕ JsonParse *pPatch, /* The PATCH */
  u32 iPatch,              /* Index of PATCH in pPatch->aBlob[] */
  u32 i_ಆಳ               /* Nesting depth */
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

  ಪ್ರತಿಜ್ಞಾ( i_ಗುರಿ>=0 && i_ಗುರಿ<p_ಗುರಿ->nBlob );
  ಪ್ರತಿಜ್ಞಾ( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  ಆದರೆ( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 sz_ಗುರಿ;       /* Total size of the target, header+payload */
    n = jsonb_Payload_ಗಾತ್ರ(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonb_Payload_ಗಾತ್ರ(p_ಗುರಿ, i_ಗುರಿ, &sz);
    sz_ಗುರಿ = n+sz;
    jsonBlobEdit(p_ಗುರಿ, i_ಗುರಿ, sz_ಗುರಿ, pPatch->aBlob+iPatch, szPatch);
    ಹಿಂತಿರುಗಿ p_ಗುರಿ->oom ? JSON_ವಿಲೀನ_OOM : JSON_ವಿಲೀನ_OK;  /* Line 03 */
  }
  x = p_ಗುರಿ->aBlob[i_ಗುರಿ] & 0x0f;
  ಆದರೆ( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonb_Payload_ಗಾತ್ರ(p_ಗುರಿ, i_ಗುರಿ, &sz);
    jsonBlobEdit(p_ಗುರಿ, i_ಗುರಿ+n, sz, 0, 0);
    x = p_ಗುರಿ->aBlob[i_ಗುರಿ];
    p_ಗುರಿ->aBlob[i_ಗುರಿ] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonb_Payload_ಗಾತ್ರ(pPatch, iPatch, &sz);
  ಆದರೆ( NEVER(n==0) ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonb_Payload_ಗಾತ್ರ(p_ಗುರಿ, i_ಗುರಿ, &sz);
  ಆದರೆ( NEVER(n==0) ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADTARGET;
  iTStart = i_ಗುರಿ+n;
  iTEndBE = iTStart+sz;

  ಆಗ( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    ಆದರೆ( ePLabel<JSONB_ಪಠ್ಯ || ePLabel>JSONB_TEXTRAW ){
      ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADPATCH;
    }
    nPLabel = jsonb_Payload_ಗಾತ್ರ(pPatch, iPCursor, &szPLabel);
    ಆದರೆ( nPLabel==0 ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    ಆದರೆ( iPValue>=iPEnd ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADPATCH;
    nPValue = jsonb_Payload_ಗಾತ್ರ(pPatch, iPValue, &szPValue);
    ಆದರೆ( nPValue==0 ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    ಆದರೆ( iPCursor>iPEnd ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + p_ಗುರಿ->delta;
    ಆಗ( iTCursor<iTEnd ){
      ಪೂರ್ಣಾಂಕ isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = p_ಗುರಿ->aBlob[iTCursor] & 0x0f;
      ಆದರೆ( eTLabel<JSONB_ಪಠ್ಯ || eTLabel>JSONB_TEXTRAW ){
        ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADTARGET;
      }
      nTLabel = jsonb_Payload_ಗಾತ್ರ(p_ಗುರಿ, iTCursor, &szTLabel);
      ಆದರೆ( nTLabel==0 ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      ಆದರೆ( iTValue>=iTEnd ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADTARGET;
      nTValue = jsonb_Payload_ಗಾತ್ರ(p_ಗುರಿ, iTValue, &szTValue);
      ಆದರೆ( nTValue==0 ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADTARGET;
      ಆದರೆ( iTValue + nTValue + szTValue > iTEnd ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_ಪಠ್ಯ || ePLabel==JSONB_TEXTRAW),
                   (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&p_ಗುರಿ->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_ಪಠ್ಯ || eTLabel==JSONB_TEXTRAW));
      ಆದರೆ( isEqual ) ನಿಲ್ಲಿಸು;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    ಆದರೆ( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      ಆದರೆ( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(p_ಗುರಿ, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        ಆದರೆ( NEVER(p_ಗುರಿ->oom) ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_OOM;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Algorithm line 12 */
        ಪೂರ್ಣಾಂಕ rc, savedDelta = p_ಗುರಿ->delta;
        p_ಗುರಿ->delta = 0;
        ಆದರೆ( i_ಆಳ>=JSON_MAX_ಆಳ ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_TOODEEP;
        rc = json_ವಿಲೀನ_Patch(p_ಗುರಿ, iTValue, pPatch, iPValue, i_ಆಳ+1);
        ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
        p_ಗುರಿ->delta += savedDelta;
      }        
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      ಆದರೆ( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(p_ಗುರಿ, iTEnd, 0, 0, szPValue+nPValue+szNew);
        ಆದರೆ( p_ಗುರಿ->oom ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_OOM;
        ಸ್ಮರಣೆ_ನಕಲು(&p_ಗುರಿ->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        ಸ್ಮರಣೆ_ನಕಲು(&p_ಗುರಿ->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ rc, savedDelta;
        jsonBlobEdit(p_ಗುರಿ, iTEnd, 0, 0, szNew+1);
        ಆದರೆ( p_ಗುರಿ->oom ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_OOM;
        ಸ್ಮರಣೆ_ನಕಲು(&p_ಗುರಿ->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        p_ಗುರಿ->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = p_ಗುರಿ->delta;
        p_ಗುರಿ->delta = 0;
        ಆದರೆ( i_ಆಳ>=JSON_MAX_ಆಳ ) ಹಿಂತಿರುಗಿ JSON_ವಿಲೀನ_TOODEEP;
        rc = json_ವಿಲೀನ_Patch(p_ಗುರಿ, iTEnd+szNew,pPatch,iPValue,i_ಆಳ+1);
        ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
        p_ಗುರಿ->delta += savedDelta;
      }
    }
  }
  ಆದರೆ( p_ಗುರಿ->delta ) json_After_Edit_ಗಾತ್ರ_Adjust(p_ಗುರಿ, i_ಗುರಿ);
  ಹಿಂತಿರುಗಿ p_ಗುರಿ->oom ? JSON_ವಿಲೀನ_OOM : JSON_ವಿಲೀನ_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonPatchFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p_ಗುರಿ;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  ಪೂರ್ಣಾಂಕ rc;                /* Result code */

  ಬಳಸದ_PARAMETER(argc);
  ಪ್ರತಿಜ್ಞಾ( argc==2 );
  p_ಗುರಿ = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  ಆದರೆ( p_ಗುರಿ==0 ) ಹಿಂತಿರುಗಿ;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  ಆದರೆ( pPatch ){
    rc = json_ವಿಲೀನ_Patch(p_ಗುರಿ, 0, pPatch, 0, 0);
    ಆದರೆ( rc==JSON_ವಿಲೀನ_OK ){
      jsonReturnParse(ctx, p_ಗುರಿ);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==JSON_ವಿಲೀನ_OOM ){
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==JSON_ವಿಲೀನ_TOODEEP ){
      sqlite3_ಫಲಿತಾಂಶ_error(ctx, "JSON nested too deep", -1);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಫಲಿತಾಂಶ_error(ctx, "malformed JSON", -1);
    }
    jsonParseFree(pPatch);
  }
  jsonParseFree(p_ಗುರಿ);
}


/*
** Implementation of the json_object(NAME,VALUE,...) function.  Return a JSON
** object that contains all name/value given in arguments.  Or if any name
** is not a string or if any value is a BLOB, throw an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonObjectFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ i;
  JsonString jx;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
  u32 n;

  ಆದರೆ( argc&1 ){
    sqlite3_ಫಲಿತಾಂಶ_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    ಹಿಂತಿರುಗಿ;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  ಗಾಗಿ(i=0; i<argc; i+=2){
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[i])!=SQLITE_ಪಠ್ಯ ){
      sqlite3_ಫಲಿತಾಂಶ_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      ಹಿಂತಿರುಗಿ;
    }
    jsonAppendSeparator(&jx);
    z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
    n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_ಮೌಲ್ಯ(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
}


/*
** json_remove(JSON, PATH, ...)
**
** Remove the named elements from JSON and return the result.  malformed
** JSON or PATH arguments result in an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonRemoveFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;          /* The parse */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ = 0; /* Path of element to be removed */
  ಪೂರ್ಣಾಂಕ i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  ಆದರೆ( argc<1 ) ಹಿಂತಿರುಗಿ;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(i=1; i<argc; i++){
    z_ಮಾರ್ಗ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
    ಆದರೆ( z_ಮಾರ್ಗ==0 ){
      ಹೋಗು json_remove_ಮುಗಿದ;
    }
    ಆದರೆ( z_ಮಾರ್ಗ[0]!='$' ){
      ಹೋಗು json_remove_patherror;
    }
    ಆದರೆ( z_ಮಾರ್ಗ[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      ಹೋಗು json_remove_ಮುಗಿದ;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = json_Lookup_ಹೆಜ್ಜೆ(p, 0, z_ಮಾರ್ಗ+1, 0);
    ಆದರೆ( JSON_LOOKUP_ISERROR(rc) ){
      ಆದರೆ( rc==JSON_LOOKUP_NOTFOUND ){
        ಮುಂದುವರಿ;  /* No-op */
      }ಇಲ್ಲದಿದ್ದರೆ{
        json_Bad_ಮಾರ್ಗ_Error(ctx, z_ಮಾರ್ಗ, rc);
      }
      ಹೋಗು json_remove_ಮುಗಿದ;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  ಹಿಂತಿರುಗಿ;

json_remove_patherror:
  json_Bad_ಮಾರ್ಗ_Error(ctx, z_ಮಾರ್ಗ, 0);

json_remove_ಮುಗಿದ:
  jsonParseFree(p);
  ಹಿಂತಿರುಗಿ;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ jsonReplaceFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಆದರೆ( argc<1 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( (argc&1)==0 ) {
    json_Wrong_ಅಂಕೆ_ವಾದಗಳು(ctx, "replace");
    ಹಿಂತಿರುಗಿ;
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
ಸ್ಥಿರ ಶೂನ್ಯ json_ಗಣ_Func(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ flags = SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(ctx));
  ಪೂರ್ಣಾಂಕ eInsType = JSON_ಸೇರಿಸು_TYPE(flags);
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azInsType[] = { "insert", "set", "array_insert" };
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 aEditType[] = { JEDIT_INS, JEDIT_ಗಣ, JEDIT_AINS };

  ಆದರೆ( argc<1 ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( eInsType>=0 && eInsType<=2 );
  ಆದರೆ( (argc&1)==0 ) {
    json_Wrong_ಅಂಕೆ_ವಾದಗಳು(ctx, azInsType[eInsType]);
    ಹಿಂತಿರುಗಿ;
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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;          /* The parse */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( argc==2 ){
    z_ಮಾರ್ಗ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
    ಆದರೆ( z_ಮಾರ್ಗ==0 ) ಹೋಗು json_type_ಮುಗಿದ;
    ಆದರೆ( z_ಮಾರ್ಗ[0]!='$' ){
      json_Bad_ಮಾರ್ಗ_Error(ctx, z_ಮಾರ್ಗ, 0);
      ಹೋಗು json_type_ಮುಗಿದ;
    }
    i = json_Lookup_ಹೆಜ್ಜೆ(p, 0, z_ಮಾರ್ಗ+1, 0);
    ಆದರೆ( JSON_LOOKUP_ISERROR(i) ){
      ಆದರೆ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ಇಲ್ಲದಿದ್ದರೆ{
        json_Bad_ಮಾರ್ಗ_Error(ctx, z_ಮಾರ್ಗ, i);
      }
      ಹೋಗು json_type_ಮುಗಿದ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    i = 0;
  }
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
json_type_ಮುಗಿದ:
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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, ಗಾತ್ರ(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  ಆದರೆ( x.pParse==0 ) ಹಿಂತಿರುಗಿ;
  x.p_ಹೊರಹೋಗು = &s;
  jsonStringInit(&s, ctx);
  ಆದರೆ( argc==1 || (x.zIndent = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }ಇಲ್ಲದಿದ್ದರೆ{
    x.szIndent = (u32)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(x.zIndent);
  }
  json_Translate_Blob_ಗಮ್ಯ_Pretty_ಪಠ್ಯ(&x, 0);
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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  ಆದರೆ( argc==2 ){
    i64 f = sqlite3_ಮೌಲ್ಯ_int64(argv[1]);
    ಆದರೆ( f<1 || f>15 ){
      sqlite3_ಫಲಿತಾಂಶ_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      ಹಿಂತಿರುಗಿ;
    }
    flags = f & 0x0f;
  }
  ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0]) ){
    ಸ್ಥಿತಿ SQLITE_NULL: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, 0);
#ಆದರೆ_ಅಂತ್ಯ
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, ಗಾತ್ರ(py));
      ಆದರೆ( jsonArgIsJsonb(argv[0], &py) ){
        ಆದರೆ( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        ನಿಲ್ಲಿಸು;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      JsonParse px;
      ಆದರೆ( (flags & 0x3)==0 ) ನಿಲ್ಲಿಸು;
      memset(&px, 0, ಗಾತ್ರ(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      ಆದರೆ( p ){
        ಆದರೆ( p->oom ){
          sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->nErr ){
          /* no-op */
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  sqlite3_ಫಲಿತಾಂಶ_int(ctx, res);
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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಬಳಸದ_PARAMETER(argc);
  memset(&s, 0, ಗಾತ್ರ(s));
  s.db = sqlite3_context_db_handle(ctx);
  ಆದರೆ( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    s.zJson = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    ಆದರೆ( s.zJson==0 ) ಹಿಂತಿರುಗಿ;  /* NULL input or OOM */
    s.nJson = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
    ಆದರೆ( json_Convert_ಪಠ್ಯ_ಗಮ್ಯ_Blob(&s,0) ){
      ಆದರೆ( s.oom ){
        iErrPos = -1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        ಪ್ರತಿಜ್ಞಾ( s.zJson!=0 );  /* Because s.oom is false */
        ಗಾಗಿ(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          ಆದರೆ( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  ಆದರೆ( iErrPos<0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_int64(ctx, iErrPos);
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
ಸ್ಥಿರ ಶೂನ್ಯ json_Array_ಹೆಜ್ಜೆ(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonString *pStr;
  ಬಳಸದ_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, ಗಾತ್ರ(*pStr));
  ಆದರೆ( pStr ){
    ಆದರೆ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pStr->n_ಬಳಸಿದ>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_ಮೌಲ್ಯ(pStr, argv[0]);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonArrayCompute(sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ isFinal){
  JsonString *pStr;
  ಪೂರ್ಣಾಂಕ flags = SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ಆದರೆ( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    ಆದರೆ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ಹಿಂತಿರುಗಿ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ಆದರೆ( isFinal ){
        ಆದರೆ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ಇಲ್ಲದಿದ್ದರೆ{
        jsonStringTrimOneChar(pStr);
      }
      ಹಿಂತಿರುಗಿ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( isFinal ){
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pStr->zBuf, (ಪೂರ್ಣಾಂಕ)pStr->n_ಬಳಸಿದ,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pStr->zBuf, (ಪೂರ್ಣಾಂಕ)pStr->n_ಬಳಸಿದ, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & JSON_BLOB ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 ಖಾಲಿಗಳು_Array = 0x0b;
    sqlite3_ಫಲಿತಾಂಶ_blob(ctx, &ಖಾಲಿಗಳು_Array, 1, SQLITE_STATIC);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
}
ಸ್ಥಿರ ಶೂನ್ಯ json_Array_ಮೌಲ್ಯ(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_ಗುಂಪು_Inverse(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ ಒಳ_Str = 0;
  ಪೂರ್ಣಾಂಕ nNest = 0;
  ಅಕ್ಷರ *z;
  ಅಕ್ಷರ c;
  JsonString *pStr;
  ಬಳಸದ_PARAMETER(argc);
  ಬಳಸದ_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  ಆದರೆ( NEVER(!pStr) ) ಹಿಂತಿರುಗಿ;
  z = pStr->zBuf;
  ಗಾಗಿ(i=1; i<pStr->n_ಬಳಸಿದ && ((c = z[i])!=',' || ಒಳ_Str || nNest); i++){
    ಆದರೆ( c=='"' ){
      ಒಳ_Str = !ಒಳ_Str;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='\\' ){
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !ಒಳ_Str ){
      ಆದರೆ( c=='{' || c=='[' ) nNest++;
      ಆದರೆ( c=='}' || c==']' ) nNest--;
    }
  }
  ಆದರೆ( i<pStr->n_ಬಳಸಿದ ){
    pStr->n_ಬಳಸಿದ -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->n_ಬಳಸಿದ-1);
    z[pStr->n_ಬಳಸಿದ] = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pStr->n_ಬಳಸಿದ = 1;
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define json_ಗುಂಪು_Inverse 0
#ಆದರೆ_ಅಂತ್ಯ


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
ಸ್ಥಿರ ಶೂನ್ಯ json_Object_ಹೆಜ್ಜೆ(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  JsonString *pStr;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
  u32 n;
  ಬಳಸದ_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, ಗಾತ್ರ(*pStr));
  ಆದರೆ( pStr ){
    z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    n = sqlite3Strlen30(z);
    ಆದರೆ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pStr->n_ಬಳಸಿದ>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    ಆದರೆ( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_ಮೌಲ್ಯ(pStr, argv[1]);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonObjectCompute(sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ isFinal){
  JsonString *pStr;
  ಪೂರ್ಣಾಂಕ flags = SQLITE_PTR_ಗಮ್ಯ_INT(sqlite3_user_ದತ್ತಾಂಶ(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ಆದರೆ( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    ಆದರೆ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      ಪೂರ್ಣಾಂಕ ಒಳ_Str = 0;
      ಆದರೆ( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->n_ಬಳಸಿದ+1);
        pStr = &tmpStr;
        ಆದರೆ( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          ಹಿಂತಿರುಗಿ;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      ಗಾಗಿ(i=j=1; i<pStr->n_ಬಳಸಿದ; i++){
        ಅಕ್ಷರ c = pStr->zBuf[i];
        ಆದರೆ( c=='"' ){
          ಒಳ_Str = !ಒಳ_Str;
          pStr->zBuf[j++] = '"';
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='@' && !ಒಳ_Str ){
          ಪ್ರತಿಜ್ಞಾ( i+1<pStr->n_ಬಳಸಿದ );
          ಆದರೆ( pStr->zBuf[i+1]==',' ){
            i++;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->n_ಬಳಸಿದ = j;    /* Truncate the string */
    }
    ಆದರೆ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ಆದರೆ( isFinal ){
        ಆದರೆ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ಇಲ್ಲದಿದ್ದರೆ{
        jsonStringTrimOneChar(pOgStr);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( isFinal ){
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pStr->zBuf, (ಪೂರ್ಣಾಂಕ)pStr->n_ಬಳಸಿದ,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pStr->zBuf, (ಪೂರ್ಣಾಂಕ)pStr->n_ಬಳಸಿದ, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    ಆದರೆ( pStr!=pOgStr ) jsonStringReset(pStr);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & JSON_BLOB ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಖಾಲಿಗಳು_Object = 0x0c;
    sqlite3_ಫಲಿತಾಂಶ_blob(ctx, &ಖಾಲಿಗಳು_Object, 1, SQLITE_STATIC); 
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
}
ಸ್ಥಿರ ಶೂನ್ಯ json_Object_ಮೌಲ್ಯ(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
ಸ್ಥಿರ ಶೂನ್ಯ jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Json_ಪೋಷಕ Json_ಪೋಷಕ;
ರಚನೆ Json_ಪೋಷಕ {
  u32 i_ತಲೆ;                 /* Start of object or array */
  u32 i_ಮೌಲ್ಯ;                /* Start of the value */
  u32 i_ಕೊನೆ;                  /* First byte past the end */
  u32 n_ಮಾರ್ಗ;                 /* Length of path */
  i64 i_ಕೀಲಿ;                  /* Key for JSONB_ARRAY */
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ JsonEachCursor JsonEachCursor;
ರಚನೆ JsonEachCursor {
  sqlite3_vtab_cursor ತಳ;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 i_ಕೊನೆ;                  /* EOF when i equals or exceeds this value */
  u32 n_ಬೇರು;                 /* Size of the root path in bytes */
  u8 eType;                  /* Type of the container for element i */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
  u8 e_ಬಹುಲಕ;                  /* 1 for json_each().  2 for jsonb_each() */
  u32 n_ಪೋಷಕ;               /* Current nesting depth */
  u32 n_ಪೋಷಕ_Alloc;          /* Space allocated for aParent[] */
  Json_ಪೋಷಕ *a_ಪೋಷಕ;       /* Parent elements of i */
  sqlite3 *db;               /* Database connection */
  JsonString ಮಾರ್ಗ;           /* Current path */
  JsonParse sParse;          /* Parse of the input JSON */
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ JsonEachConnection JsonEachConnection;
ರಚನೆ JsonEachConnection {
  sqlite3_vtab ತಳ;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 e_ಬಹುಲಕ;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonEachConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  JsonEachConnection *pNew;
  ಪೂರ್ಣಾಂಕ rc;

/* Column numbers */
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_ಕೀಲಿ     0
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_ಮೌಲ್ಯ   1
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_TYPE    2
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_ATOM    3
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_ID      4
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_ಪೋಷಕ  5
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_FULLKEY 6
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_ಮಾರ್ಗ    7
/* The xBestIndex method assumes that the JSON and ROOT columns are
** the last two columns in the table.  Should this ever changes, be
** sure to update the xBestIndex method. */
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_JSON    8
#ವ್ಯಾಖ್ಯಾನಿಸು JEACH_ಬೇರು    9

  ಬಳಸದ_PARAMETER(pzErr);
  ಬಳಸದ_PARAMETER(argv);
  ಬಳಸದ_PARAMETER(argc);
  ಬಳಸದ_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  ಆದರೆ( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, ಗಾತ್ರ(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->e_ಬಹುಲಕ = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->e_ಬಹುಲಕ]=='t';
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* destructor for json_each virtual table */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  ಬಳಸದ_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, ಗಾತ್ರ(*pCur));
  ಆದರೆ( pCur==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->e_ಬಹುಲಕ = pVtab->e_ಬಹುಲಕ;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->ಮಾರ್ಗ);
  *ppCursor = &pCur->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
ಸ್ಥಿರ ಶೂನ್ಯ jsonEachCursorReset(JsonEachCursor *p){
  jsonParseReset(&p->sParse);
  jsonStringReset(&p->ಮಾರ್ಗ);
  sqlite3DbFree(p->db, p->a_ಪೋಷಕ);
  p->iRowid = 0;
  p->i = 0;
  p->a_ಪೋಷಕ = 0;
  p->n_ಪೋಷಕ = 0;
  p->n_ಪೋಷಕ_Alloc = 0;
  p->i_ಕೊನೆ = 0;
  p->eType = 0;
}

/* Destructor for a jsonEachCursor object */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ಹಿಂತಿರುಗಿ p->i >= p->i_ಕೊನೆ;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonSkipLabel(JsonEachCursor *p){
  ಆದರೆ( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonb_Payload_ಗಾತ್ರ(&p->sParse, p->i, &sz);
    sz += p->i + n;
    ಆದರೆ( sz >= p->sParse.nBlob ) sz = p->i;
    ಹಿಂತಿರುಗಿ sz;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ p->i;
  }
}

/*
** Append the path name for the current element.
*/
ಸ್ಥಿರ ಶೂನ್ಯ json_Append_ಮಾರ್ಗ_Name(JsonEachCursor *p){
  ಪ್ರತಿಜ್ಞಾ( p->n_ಪೋಷಕ>0 );
  ಪ್ರತಿಜ್ಞಾ( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  ಆದರೆ( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->ಮಾರ್ಗ, "[%lld]", p->a_ಪೋಷಕ[p->n_ಪೋಷಕ-1].i_ಕೀಲಿ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    u32 n, sz = 0, k, i;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
    ಪೂರ್ಣಾಂಕ needQuote = 0;
    n = jsonb_Payload_ಗಾತ್ರ(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&p->sParse.aBlob[k];
    ಆದರೆ( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಗಾಗಿ(i=0; i<sz; i++){
        ಆದರೆ( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          ನಿಲ್ಲಿಸು;
        }
      }
    }
    ಆದರೆ( needQuote ){
      jsonPrintf(sz+4,&p->ಮಾರ್ಗ,".\"%.*s\"", sz, z);
    }ಇಲ್ಲದಿದ್ದರೆ{
      jsonPrintf(sz+2,&p->ಮಾರ್ಗ,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Each_Malformed_ಒಳಹರಿವು(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_ಸಂದೇಶ);
  cur->pVtab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  ಹಿಂತಿರುಗಿ cur->pVtab->z_Err_ಸಂದೇಶ ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Each_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonb_Payload_ಗಾತ್ರ(&p->sParse, i, &sz);
    ಆದರೆ( n==0 )ಹಿಂತಿರುಗಿ json_Each_Malformed_ಒಳಹರಿವು(cur);
    ಆದರೆ( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      Json_ಪೋಷಕ *p_ಪೋಷಕ;
      ಆದರೆ( p->n_ಪೋಷಕ>=p->n_ಪೋಷಕ_Alloc ){
        Json_ಪೋಷಕ *pNew;
        u64 nNew;
        nNew = p->n_ಪೋಷಕ_Alloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->a_ಪೋಷಕ, ಗಾತ್ರ(Json_ಪೋಷಕ)*nNew);
        ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
        p->n_ಪೋಷಕ_Alloc = (u32)nNew;
        p->a_ಪೋಷಕ = pNew;
      }
      levelChange = 1;
      p_ಪೋಷಕ = &p->a_ಪೋಷಕ[p->n_ಪೋಷಕ];
      p_ಪೋಷಕ->i_ತಲೆ = p->i;
      p_ಪೋಷಕ->i_ಮೌಲ್ಯ = i;
      p_ಪೋಷಕ->i_ಕೊನೆ = i + n + sz;
      p_ಪೋಷಕ->i_ಕೀಲಿ = -1;
      p_ಪೋಷಕ->n_ಮಾರ್ಗ = (u32)p->ಮಾರ್ಗ.n_ಬಳಸಿದ;
      ಆದರೆ( p->eType && p->n_ಪೋಷಕ ){
        json_Append_ಮಾರ್ಗ_Name(p);
        ಆದರೆ( p->ಮಾರ್ಗ.eErr ) rc = SQLITE_NOMEM;
      }
      p->n_ಪೋಷಕ++;
      p->i = i + n;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->i = i + n + sz;
    }
    ಆಗ( p->n_ಪೋಷಕ>0 && p->i >= p->a_ಪೋಷಕ[p->n_ಪೋಷಕ-1].i_ಕೊನೆ ){
      p->n_ಪೋಷಕ--;
      p->ಮಾರ್ಗ.n_ಬಳಸಿದ = p->a_ಪೋಷಕ[p->n_ಪೋಷಕ].n_ಮಾರ್ಗ;
      levelChange = 1;
    }
    ಆದರೆ( levelChange ){
      ಆದರೆ( p->n_ಪೋಷಕ>0 ){
        Json_ಪೋಷಕ *p_ಪೋಷಕ = &p->a_ಪೋಷಕ[p->n_ಪೋಷಕ-1];
        u32 iVal = p_ಪೋಷಕ->i_ಮೌಲ್ಯ;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }ಇಲ್ಲದಿದ್ದರೆ{
        p->eType = 0;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonb_Payload_ಗಾತ್ರ(&p->sParse, i, &sz);
    ಆದರೆ( n==0 )ಹಿಂತಿರುಗಿ json_Each_Malformed_ಒಳಹರಿವು(cur);
    p->i = i + n + sz;
  }
  ಆದರೆ( p->eType==JSONB_ARRAY && p->n_ಪೋಷಕ ){
    p->a_ಪೋಷಕ[p->n_ಪೋಷಕ-1].i_ಕೀಲಿ++;
  }
  p->iRowid++;
  ಹಿಂತಿರುಗಿ rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Each_ಮಾರ್ಗ_ಉದ್ದಳತೆ(JsonEachCursor *p){
  u32 n = p->ಮಾರ್ಗ.n_ಬಳಸಿದ;
  ಅಕ್ಷರ *z = p->ಮಾರ್ಗ.zBuf;
  ಆದರೆ( p->iRowid==0 && p->bRecursive && n>=2 ){
    ಆಗ( ALWAYS(n>1) ){
      n--;
      ಆದರೆ( z[n]=='[' || z[n]=='.' ){
        u32 x;
        ಅಕ್ಷರ cSaved = z[n];
        z[n] = 0;
        ಪ್ರತಿಜ್ಞಾ( p->sParse.eEdit==0 );
        x = json_Lookup_ಹೆಜ್ಜೆ(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        ಆದರೆ( !JSON_LOOKUP_ISERROR(x) ) ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಹಿಂತಿರುಗಿ n;
}

/* Return the value of a column */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Each_ಸ್ತಂಭ(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ಪೂರ್ಣಾಂಕ i_ಸ್ತಂಭ                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ಆಯ್ಕೆ( i_ಸ್ತಂಭ ){
    ಸ್ಥಿತಿ JEACH_ಕೀಲಿ: {
      ಆದರೆ( p->n_ಪೋಷಕ==0 ){
        u32 n, j;
        ಆದರೆ( p->n_ಬೇರು==1 ) ನಿಲ್ಲಿಸು;
        j = json_Each_ಮಾರ್ಗ_ಉದ್ದಳತೆ(p);
        n = p->n_ಬೇರು - j;
        ಆದರೆ( n==0 ){
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->ಮಾರ್ಗ.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->ಮಾರ್ಗ.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_ಫಲಿತಾಂಶ_int64(ctx, x);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->ಮಾರ್ಗ.zBuf[j+1]=='"' ){
          sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, &p->ಮಾರ್ಗ.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, &p->ಮಾರ್ಗ.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( p->eType==JSONB_ARRAY );
        sqlite3_ಫಲಿತಾಂಶ_int64(ctx, p->a_ಪೋಷಕ[p->n_ಪೋಷಕ-1].i_ಕೀಲಿ);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JEACH_ಮೌಲ್ಯ: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->e_ಬಹುಲಕ);
      ಆದರೆ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_ಫಲಿತಾಂಶ_subtype(ctx, JSON_SUBTYPE);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      ಆದರೆ( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JEACH_ID: {
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, (sqlite3_int64)p->i);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JEACH_ಪೋಷಕ: {
      ಆದರೆ( p->n_ಪೋಷಕ>0 && p->bRecursive ){
        sqlite3_ಫಲಿತಾಂಶ_int64(ctx, p->a_ಪೋಷಕ[p->n_ಪೋಷಕ-1].i_ತಲೆ);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JEACH_FULLKEY: {
      u64 n_ತಳ = p->ಮಾರ್ಗ.n_ಬಳಸಿದ;
      ಆದರೆ( p->n_ಪೋಷಕ ) json_Append_ಮಾರ್ಗ_Name(p);
      sqlite3_ಫಲಿತಾಂಶ_text64(ctx, p->ಮಾರ್ಗ.zBuf, p->ಮಾರ್ಗ.n_ಬಳಸಿದ,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->ಮಾರ್ಗ.n_ಬಳಸಿದ = n_ತಳ;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JEACH_ಮಾರ್ಗ: {
      u32 n = json_Each_ಮಾರ್ಗ_ಉದ್ದಳತೆ(p);
      sqlite3_ಫಲಿತಾಂಶ_text64(ctx, p->ಮಾರ್ಗ.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, p->ಮಾರ್ಗ.zBuf, p->n_ಬೇರು, SQLITE_TRANSIENT);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ JEACH_JSON: {
      ಆದರೆ( p->sParse.zJson==0 ){
        sqlite3_ಫಲಿತಾಂಶ_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Return the current rowid value */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ json_Each_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(
  sqlite3_vtab *tab,
  sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo
){
  ಪೂರ್ಣಾಂಕ i;                     /* Loop counter or computed array index */
  ಪೂರ್ಣಾಂಕ aIdx[2];               /* Index of constraints for JSON and ROOT */
  ಪೂರ್ಣಾಂಕ unusable_ಮುಖವಾಡ = 0;      /* Mask of unusable JSON and ROOT constraints */
  ಪೂರ್ಣಾಂಕ idx_ಮುಖವಾಡ = 0;           /* Mask of usable == constraints JSON and ROOT */
  ಸ್ಥಿರಾಂಕ ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  ಪ್ರತಿಜ್ಞಾ( JEACH_ಬೇರು == JEACH_JSON+1 );
  ಬಳಸದ_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  ಗಾಗಿ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;
    ಪೂರ್ಣಾಂಕ i_ಮುಖವಾಡ;
    ಆದರೆ( pConstraint->i_ಸ್ತಂಭ < JEACH_JSON ) ಮುಂದುವರಿ;
    i_ಲಂಬಸಾಲು = pConstraint->i_ಸ್ತಂಭ - JEACH_JSON;
    ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು==0 || i_ಲಂಬಸಾಲು==1 );
    testcase( i_ಲಂಬಸಾಲು==0 );
    i_ಮುಖವಾಡ = 1 << i_ಲಂಬಸಾಲು;
    ಆದರೆ( pConstraint->usable==0 ){
      unusable_ಮುಖವಾಡ |= i_ಮುಖವಾಡ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pConstraint->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ ){
      aIdx[i_ಲಂಬಸಾಲು] = i;
      idx_ಮುಖವಾಡ |= i_ಮುಖವಾಡ;
    }
  }
  ಆದರೆ( pIdxInfo->n_ಕ್ರಮ_By>0
   && pIdxInfo->a_ಕ್ರಮ_By[0].i_ಸ್ತಂಭ<0
   && pIdxInfo->a_ಕ್ರಮ_By[0].desc==0
  ){
    pIdxInfo->ಕ್ರಮ_By_Consumed = 1;
  }

  ಆದರೆ( (unusable_ಮುಖವಾಡ & ~idx_ಮುಖವಾಡ)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT;
  }
  ಆದರೆ( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_ಅಂಕೆ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pIdxInfo->estimated_ವೆಚ್ಚ = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argv_ಸೂಚ್ಯಂಕ = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ಆದರೆ( aIdx[1]<0 ){
      pIdxInfo->idx_ಅಂಕೆ = 1;  /* Only JSON supplied.  Plan 1 */
    }ಇಲ್ಲದಿದ್ದರೆ{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argv_ಸೂಚ್ಯಂಕ = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_ಅಂಕೆ = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Start a search on a new JSON string */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬೇರು = 0;
  u32 i, n, sz;

  ಬಳಸದ_PARAMETER(idxStr);
  ಬಳಸದ_PARAMETER(argc);
  jsonEachCursorReset(p);
  ಆದರೆ( idx_ಅಂಕೆ==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  memset(&p->sParse, 0, ಗಾತ್ರ(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  ಆದರೆ( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->sParse.zJson = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    p->sParse.nJson = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
    ಆದರೆ( p->sParse.zJson==0 ){
      p->i = p->i_ಕೊನೆ = 0;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }      
    ಆದರೆ( json_Convert_ಪಠ್ಯ_ಗಮ್ಯ_Blob(&p->sParse, 0) ){
      ಆದರೆ( p->sParse.oom ){
        ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      }
      ಹಿಂತಿರುಗಿ json_Each_Malformed_ಒಳಹರಿವು(cur);
    }
  }
  ಆದರೆ( idx_ಅಂಕೆ==3 ){
    z_ಬೇರು = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
    ಆದರೆ( z_ಬೇರು==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
    ಆದರೆ( z_ಬೇರು[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_ಸಂದೇಶ);
      cur->pVtab->z_Err_ಸಂದೇಶ = json_Bad_ಮಾರ್ಗ_Error(0, z_ಬೇರು, 0);
      jsonEachCursorReset(p);
      ಹಿಂತಿರುಗಿ cur->pVtab->z_Err_ಸಂದೇಶ ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_ಬೇರು = sqlite3Strlen30(z_ಬೇರು);
    ಆದರೆ( z_ಬೇರು[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      i = json_Lookup_ಹೆಜ್ಜೆ(&p->sParse, 0, z_ಬೇರು+1, 0);
      ಆದರೆ( JSON_LOOKUP_ISERROR(i) ){
        ಆದರೆ( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->i_ಕೊನೆ = 0;
          ಹಿಂತಿರುಗಿ SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_ಸಂದೇಶ);
        cur->pVtab->z_Err_ಸಂದೇಶ = json_Bad_ಮಾರ್ಗ_Error(0, z_ಬೇರು, 0);
        jsonEachCursorReset(p);
        ಹಿಂತಿರುಗಿ cur->pVtab->z_Err_ಸಂದೇಶ ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      ಆದರೆ( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }ಇಲ್ಲದಿದ್ದರೆ{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->ಮಾರ್ಗ, z_ಬೇರು, p->n_ಬೇರು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    i = p->i = 0;
    p->eType = 0;
    p->n_ಬೇರು = 1;
    jsonAppendRaw(&p->ಮಾರ್ಗ, "$", 1);
  }
  p->n_ಪೋಷಕ = 0;
  n = jsonb_Payload_ಗಾತ್ರ(&p->sParse, i, &sz);
  p->i_ಕೊನೆ = i+n+sz;
  ಆದರೆ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->a_ಪೋಷಕ = sqlite3DbMallocZero(p->db, ಗಾತ್ರ(Json_ಪೋಷಕ));
    ಆದರೆ( p->a_ಪೋಷಕ==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    p->n_ಪೋಷಕ = 1;
    p->n_ಪೋಷಕ_Alloc = 1;
    p->a_ಪೋಷಕ[0].i_ಕೀಲಿ = 0;
    p->a_ಪೋಷಕ[0].i_ಕೊನೆ = p->i_ಕೊನೆ;
    p->a_ಪೋಷಕ[0].i_ತಲೆ = p->i;
    p->a_ಪೋಷಕ[0].i_ಮೌಲ್ಯ = i;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* The methods of the json_each virtual table */
ಸ್ಥಿರ sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  json_Each_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  json_Each_ಮುಂದಿನ,              /* xNext - advance a cursor */
  jsonEachEof,               /* xEof - check for end of scan */
  json_Each_ಸ್ತಂಭ,            /* xColumn - read data */
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
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_JSON) */

/*
** Register JSON functions.
*/
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_Json_Functions(ಶೂನ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_JSON
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
    JFUNCTION(json_array_ಸೇರಿಸು, -1,1,1, 1,0,JSON_AINS,  json_ಗಣ_Func),
    JFUNCTION(jsonb_array_ಸೇರಿಸು,-1,1,0, 1,1,JSON_AINS,  json_ಗಣ_Func),
    JFUNCTION(json_array_ಉದ್ದಳತೆ,  1,1,0, 0,0,0,          json_Array_ಉದ್ದಳತೆ_Func),
    JFUNCTION(json_array_ಉದ್ದಳತೆ,  2,1,0, 0,0,0,          json_Array_ಉದ್ದಳತೆ_Func),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_ಹೊರತೆಗೆ,      -1,1,1, 0,0,0,          json_ಹೊರತೆಗೆ_Func),
    JFUNCTION(jsonb_ಹೊರತೆಗೆ,     -1,1,0, 0,1,0,          json_ಹೊರತೆಗೆ_Func),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  json_ಹೊರತೆಗೆ_Func),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   json_ಹೊರತೆಗೆ_Func),
    JFUNCTION(json_ಸೇರಿಸು,       -1,1,1, 1,0,0,          json_ಗಣ_Func),
    JFUNCTION(jsonb_ಸೇರಿಸು,      -1,1,0, 1,1,0,          json_ಗಣ_Func),
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
    JFUNCTION(json_ಗಣ,          -1,1,1, 1,0,JSON_ISSET, json_ಗಣ_Func),
    JFUNCTION(jsonb_ಗಣ,         -1,1,0, 1,1,JSON_ISSET, json_ಗಣ_Func),
    JFUNCTION(json_type,          1,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_type,          2,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_valid,         1,1,0, 0,0,0,          jsonValidFunc),
    JFUNCTION(json_valid,         2,1,0, 0,0,0,          jsonValidFunc),
#ಆದರೆ SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#ಆದರೆ_ಅಂತ್ಯ
    WAGGREGATE(json_ಗುಂಪು_array,  1, 0, 0,
       json_Array_ಹೆಜ್ಜೆ, jsonArrayFinal, json_Array_ಮೌಲ್ಯ, json_ಗುಂಪು_Inverse,
       SQLITE_SUBTYPE|SQLITE_ಫಲಿತಾಂಶ_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_ಗುಂಪು_array, 1, JSON_BLOB, 0,
       json_Array_ಹೆಜ್ಜೆ, jsonArrayFinal, json_Array_ಮೌಲ್ಯ, json_ಗುಂಪು_Inverse,
       SQLITE_SUBTYPE|SQLITE_ಫಲಿತಾಂಶ_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_ಗುಂಪು_object, 2, 0, 0,
       json_Object_ಹೆಜ್ಜೆ, jsonObjectFinal, json_Object_ಮೌಲ್ಯ, json_ಗುಂಪು_Inverse,
       SQLITE_SUBTYPE|SQLITE_ಫಲಿತಾಂಶ_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_ಗುಂಪು_object,2, JSON_BLOB, 0,
       json_Object_ಹೆಜ್ಜೆ, jsonObjectFinal, json_Object_ಮೌಲ್ಯ, json_ಗುಂಪು_Inverse,
       SQLITE_SUBTYPE|SQLITE_ಫಲಿತಾಂಶ_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_ಸೇರಿಸು_Builtin_Funcs(aJsonFunc, Array_ಗಾತ್ರ(aJsonFunc));
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3_Json_Vtab_ರಿಜಿಸ್ಟರ್(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Hash_ಕಂಡುಹಿಡಿ(&db->aModule, zName)==0 );
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(azModule)/ಗಾತ್ರ(azModule[0]); i++){
    ಆದರೆ( sqlite3StrICmp(azModule[i],zName)==0 ){
      ಹಿಂತಿರುಗಿ sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
