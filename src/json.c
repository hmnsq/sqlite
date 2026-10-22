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
#که_نه_تعریف_شوی SQLITE_OMIT_JSON
#ضمن "sqliteInt.h"

/* JSONB element types
*/
#تعریف_کړه JSONB_NULL     0   /* "null" */
#تعریف_کړه JSONB_TRUE     1   /* "true" */
#تعریف_کړه JSONB_FALSE    2   /* "false" */
#تعریف_کړه JSONB_INT      3   /* integer acceptable to JSON and SQL */
#تعریف_کړه JSONB_INT5     4   /* integer in 0x000 notation */
#تعریف_کړه JSONB_FLOAT    5   /* float acceptable to JSON and SQL */
#تعریف_کړه JSONB_FLOAT5   6   /* float with JSON5 extensions */
#تعریف_کړه JSONB_متن     7   /* Text compatible with both JSON and SQL */
#تعریف_کړه JSONB_TEXTJ    8   /* Text with JSON escapes */
#تعریف_کړه JSONB_TEXT5    9   /* Text with JSON-5 escape */
#تعریف_کړه JSONB_TEXTRAW 10   /* SQL text that needs escaping for JSON */
#تعریف_کړه JSONB_ARRAY   11   /* An array */
#تعریف_کړه JSONB_OBJECT  12   /* An object */

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
#که_تعریف_شوی SQLITE_ASCII
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
#د_که_پای
#که_تعریف_شوی SQLITE_EBCDIC
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
#د_که_پای

};
#تعریف_کړه jsonIsspace(x) (jsonIsSpace[(بینښه حرف)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#که_تعریف_شوی SQLITE_ASCII
ساكن ثابت حرف jsonSpaces[] = "\011\012\015\040";
#د_که_پای
#که_تعریف_شوی SQLITE_EBCDIC
ساكن ثابت حرف jsonSpaces[] = "\005\045\015\100";
#د_که_پای


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
ساكن ثابت حرف jsonIsOk[256] = {
#که_تعریف_شوی SQLITE_ASCII
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
#د_که_پای
#که_تعریف_شوی SQLITE_EBCDIC
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
#د_که_پای
};

/* Objects */
تعريفنوع بنية JsonCache JsonCache;
تعريفنوع بنية JsonString JsonString;
تعريفنوع بنية JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#تعریف_کړه JSON_CACHE_ID    (-429938)  /* Cache entry */
#تعریف_کړه JSON_CACHE_اندازه  4          /* Max number of cache entries */

/*
** jsonUnescapeOneChar() returns this invalid code point if it encounters
** a syntax error.
*/
#تعریف_کړه JSON_INVALID_CHAR 0x99999

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
  صحيح n_کارول_شوی;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_اندازه];  /* One line for each cache entry */
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
  u64 n_کارول_شوی;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  حرف zSpace[100];        /* Initial static space */
};

/* Allowed values for JsonString.eErr */
#تعریف_کړه JSTRING_OOM         0x01   /* Out of memory */
#تعریف_کړه JSTRING_MALFORMED   0x02   /* Malformed JSONB */
#تعریف_کړه JSTRING_TOODEEP     0x04   /* JSON nested too deep */
#تعریف_کړه JSTRING_ERR         0x08   /* Error already sent to sqlite3_result */

/* The "subtype" set for text JSON values passed through using
** sqlite3_result_subtype() and sqlite3_value_subtype().
*/
#تعریف_کړه JSON_SUBTYPE  74    /* Ascii for "J" */

/*
** Bit values for the flags passed into various SQL function implementations
** via the sqlite3_user_data() value.
*/
#تعریف_کړه JSON_JSON      0x01        /* Result is always JSON */
#تعریف_کړه JSON_SQL       0x02        /* Result is always SQL */
#تعریف_کړه JSON_ABPATH    0x03        /* Allow abbreviated JSON path specs */
#تعریف_کړه JSON_ISSET     0x04        /* json_set(), not json_insert() */
#تعریف_کړه JSON_AINS      0x08        /* json_array_insert(), not json_insert() */
#تعریف_کړه JSON_BLOB      0x10        /* Use the BLOB output format */

#تعریف_کړه JSON_ورننباسه_TYPE(X)  (((X)&0xC)>>2)


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
  u16 i_ژوروالی;        /* Nesting depth */
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
#تعریف_کړه JEDIT_DEL   1   /* Delete if exists */
#تعریف_کړه JEDIT_REPL  2   /* Overwrite if exists */
#تعریف_کړه JEDIT_INS   3   /* Insert if not exists */
#تعریف_کړه JEDIT_سېټ   4   /* Insert or overwrite */
#تعریف_کړه JEDIT_AINS  5   /* array_insert() */

/*
** Maximum nesting depth of JSON for this implementation.
**
** This limit is needed to avoid a stack overflow in the recursive
** descent parser.  A depth of 1000 is far deeper than any sane JSON
** should go.  Historical note: This limit was 2000 prior to version 3.42.0
*/
#که_نه_تعریف_شوی SQLITE_JSON_MAX_ژوروالی
# define JSON_MAX_ژوروالی  1000
#وإلا
# define JSON_MAX_ژوروالی SQLITE_JSON_MAX_ژوروالی
#د_که_پای

/*
** Allowed values for the flgs argument to jsonParseFuncArg();
*/
#تعریف_کړه JSON_EDITABLE  0x01   /* Generate a writable JsonParse object */
#تعریف_کړه JSON_KEEPERROR 0x02   /* Return non-NULL even if there is an error */
#تعریف_کړه JSON_NOCACHE   0x04   /* Do not use the JSON cache */

/**************************************************************************
** Forward references
**************************************************************************/
ساكن فراغ jsonReturnStringAsBlob(JsonString*);
ساكن صحيح jsonArgIsJsonb(sqlite3_ارزښت *pJson, JsonParse *p);
ساكن u32 json_Translate_Blob_منزل_متن(JsonParse*,u32,JsonString*);
ساكن فراغ jsonReturnParse(sqlite3_context*,JsonParse*);
ساكن JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_ارزښت*,u32);
ساكن فراغ jsonParseFree(JsonParse*);
ساكن u32 jsonb_Payload_اندازه(ثابت JsonParse*, u32, u32*);
ساكن u32 jsonUnescapeOneChar(ثابت حرف*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
ساكن فراغ jsonCacheDelete(JsonCache *p){
  صحيح i;
  لكل(i=0; i<p->n_کارول_شوی; i++){
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
ساكن صحيح json_Cache_ورننباسه(
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
    sqlite3_سېټ_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    إذا( p==0 ) أرجع SQLITE_NOMEM;
  }
  إذا( p->n_کارول_شوی >= JSON_CACHE_اندازه ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_اندازه-1)*حجم(p->a[0]));
    p->n_کارول_شوی = JSON_CACHE_اندازه-1;
  }
  أكد( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->n_کارول_شوی] = pParse;
  p->n_کارول_شوی++;
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
ساكن JsonParse *json_Cache_پلټنه(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_ارزښت *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  صحيح i;
  ثابت حرف *zJson;
  صحيح nJson;

  إذا( sqlite3_ارزښت_type(pArg)!=SQLITE_متن ){
    أرجع 0;
  }
  zJson = (ثابت حرف*)sqlite3_ارزښت_متن(pArg);
  إذا( zJson==0 ) أرجع 0;
  nJson = sqlite3_ارزښت_بایټونه(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  إذا( p==0 ){
    أرجع 0;
  }
  لكل(i=0; i<p->n_کارول_شوی; i++){
    إذا( p->a[i]->zJson==zJson ) اكسر;
  }
  إذا( i>=p->n_کارول_شوی ){
    لكل(i=0; i<p->n_کارول_شوی; i++){
      إذا( p->a[i]->nJson!=nJson ) استمر;
      إذا( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) اكسر;
    }
  }
  إذا( i<p->n_کارول_شوی ){
    إذا( i<p->n_کارول_شوی-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->n_کارول_شوی-i-1)*حجم(tmp));
      p->a[p->n_کارول_شوی-1] = tmp;
      i = p->n_کارول_شوی - 1;
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
  p->n_کارول_شوی = 0;
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
  إذا( p->pCtx ) sqlite3_پایله_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
ساكن فراغ jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  أكد( p->pCtx!=0 );
  sqlite3_پایله_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
ساكن صحيح jsonStringGrow(JsonString *p, u32 N){
  u64 n_ټول = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  حرف *zNew;
  إذا( p->bStatic ){
    إذا( p->eErr ) أرجع 1;
    zNew = sqlite3RCStrNew(n_ټول);
    إذا( zNew==0 ){
      jsonStringOom(p);
      أرجع SQLITE_NOMEM;
    }
    حافظه_نقل(zNew, p->zBuf, (size_t)p->n_کارول_شوی);
    p->zBuf = zNew;
    p->bStatic = 0;
  }وإلا{
    p->zBuf = sqlite3RCStrResize(p->zBuf, n_ټول);
    إذا( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      أرجع SQLITE_NOMEM;
    }
  }
  p->nAlloc = n_ټول;
  أرجع SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
ساكن SQLITE_NOINLINE فراغ jsonStringExpandAndAppend(
  JsonString *p,
  ثابت حرف *z_دننه,
  u32 N
){
  أكد( N>0 );
  إذا( jsonStringGrow(p,N) ) أرجع;
  حافظه_نقل(p->zBuf+p->n_کارول_شوی, z_دننه, N);
  p->n_کارول_شوی += N;
}
ساكن فراغ jsonAppendRaw(JsonString *p, ثابت حرف *z_دننه, u32 N){
  إذا( N==0 ) أرجع;
  إذا( N+p->n_کارول_شوی >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_دننه,N);
  }وإلا{
    حافظه_نقل(p->zBuf+p->n_کارول_شوی, z_دننه, N);
    p->n_کارول_شوی += N;
  }
}
ساكن فراغ jsonAppendRawNZ(JsonString *p, ثابت حرف *z_دننه, u32 N){
  أكد( N>0 );
  إذا( N+p->n_کارول_شوی >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_دننه,N);
  }وإلا{
    حافظه_نقل(p->zBuf+p->n_کارول_شوی, z_دننه, N);
    p->n_کارول_شوی += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
ساكن فراغ jsonPrintf(صحيح N, JsonString *p, ثابت حرف *zFormat, ...){
  va_list ap;
  إذا( (p->n_کارول_شوی + N >= p->nAlloc) && jsonStringGrow(p, N) ) أرجع;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->n_کارول_شوی, zFormat, ap);
  va_end(ap);
  p->n_کارول_شوی += (صحيح)مزي_اوږدوالی(p->zBuf+p->n_کارول_شوی);
}

/* Append a single character
*/
ساكن SQLITE_NOINLINE فراغ jsonAppendCharExpand(JsonString *p, حرف c){
  إذا( jsonStringGrow(p,1) ) أرجع;
  p->zBuf[p->n_کارول_شوی++] = c;
}
ساكن فراغ jsonAppendChar(JsonString *p, حرف c){
  إذا( p->n_کارول_شوی>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }وإلا{
    p->zBuf[p->n_کارول_شوی++] = c;
  }
}

/* Remove a single character from the end of the string
*/
ساكن فراغ jsonStringTrimOneChar(JsonString *p){
  إذا( p->eErr==0 ){
    أكد( p->n_کارول_شوی>0 );
    p->n_کارول_شوی--;
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
  إذا( p->n_کارول_شوی==0 ) أرجع;
  c = p->zBuf[p->n_کارول_شوی-1];
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
  أكد( p->n_کارول_شوی+7 <= p->nAlloc );
  إذا( aSpecial[c] ){
    p->zBuf[p->n_کارول_شوی] = '\\';
    p->zBuf[p->n_کارول_شوی+1] = aSpecial[c];
    p->n_کارول_شوی += 2;
  }وإلا{
    p->zBuf[p->n_کارول_شوی] = '\\';
    p->zBuf[p->n_کارول_شوی+1] = 'u';
    p->zBuf[p->n_کارول_شوی+2] = '0';
    p->zBuf[p->n_کارول_شوی+3] = '0';
    p->zBuf[p->n_کارول_شوی+4] = "0123456789abcdef"[c>>4];
    p->zBuf[p->n_کارول_شوی+5] = "0123456789abcdef"[c&0xf];
    p->n_کارول_شوی += 6;
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
ساكن فراغ jsonAppendString(JsonString *p, ثابت حرف *z_دننه, u32 N){
  u32 k;
  u8 c;
  ثابت u8 *z = (ثابت u8*)z_دننه;
  إذا( z==0 ) أرجع;
  إذا( (N+p->n_کارول_شوی+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) أرجع;
  p->zBuf[p->n_کارول_شوی++] = '"';
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
        حافظه_نقل(&p->zBuf[p->n_کارول_شوی], z, k);
        p->n_کارول_شوی += k;
      }
      اكسر;
    }
    إذا( k>0 ){
      حافظه_نقل(&p->zBuf[p->n_کارول_شوی], z, k);
      p->n_کارول_شوی += k;
      z += k;
      N -= k;
    }
    c = z[0];
    إذا( c=='"' || c=='\\' ){
      إذا( (p->n_کارول_شوی+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) أرجع;
      p->zBuf[p->n_کارول_شوی++] = '\\';
      p->zBuf[p->n_کارول_شوی++] = c;
    }وإلا إذا( c=='\'' ){
      p->zBuf[p->n_کارول_شوی++] = c;
    }وإلا{
      إذا( (p->n_کارول_شوی+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) أرجع;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->n_کارول_شوی++] = '"';
  أكد( p->n_کارول_شوی<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
ساكن فراغ json_Append_Sql_ارزښت(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_ارزښت *p_ارزښت          /* Value to append */
){
  بدل( sqlite3_ارزښت_type(p_ارزښت) ){
    حالة SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      اكسر;
    }
    حالة SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_ارزښت_double(p_ارزښت));
      اكسر;
    }
    حالة SQLITE_INTEGER: {
      ثابت حرف *z = (ثابت حرف*)sqlite3_ارزښت_متن(p_ارزښت);
      u32 n = (u32)sqlite3_ارزښت_بایټونه(p_ارزښت);
      jsonAppendRaw(p, z, n);
      اكسر;
    }
    حالة SQLITE_متن: {
      ثابت حرف *z = (ثابت حرف*)sqlite3_ارزښت_متن(p_ارزښت);
      u32 n = (u32)sqlite3_ارزښت_بایټونه(p_ارزښت);
      إذا( sqlite3_ارزښت_subtype(p_ارزښت)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }وإلا{
        jsonAppendString(p, z, n);
      }
      اكسر;
    }
    افتراضي: {
      JsonParse px;
      memset(&px, 0, حجم(px));
      إذا( jsonArgIsJsonb(p_ارزښت, &px) ){
        json_Translate_Blob_منزل_متن(&px, 0, p);
      }وإلا إذا( p->eErr==0 ){
        sqlite3_پایله_error(p->pCtx, "JSON cannot hold BLOB values", -1);
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
    صحيح flags = SQLITE_PTR_منزل_INT(sqlite3_user_معلومات(p->pCtx));
    إذا( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }وإلا إذا( p->bStatic ){
      sqlite3_پایله_text64(p->pCtx, p->zBuf, p->n_کارول_شوی,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }وإلا{
      إذا( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        صحيح rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->n_کارول_شوی;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_ورننباسه(ctx, pParse);
        إذا( rc==SQLITE_NOMEM ){
          sqlite3_پایله_error_nomem(ctx);
          jsonStringReset(p);
          أرجع;
        }
      }
      sqlite3_پایله_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->n_کارول_شوی,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }وإلا إذا( p->eErr & JSTRING_OOM ){
    sqlite3_پایله_error_nomem(p->pCtx);
  }وإلا إذا( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }وإلا إذا( p->eErr & JSTRING_MALFORMED ){
    sqlite3_پایله_error(p->pCtx, "malformed JSON", -1);
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
ساكن u8 json_Hex_منزل_Int(صحيح h){
#که_تعریف_شوی SQLITE_ASCII
  h += 9*(1&(h>>6));
#د_که_پای
#که_تعریف_شوی SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#د_که_پای
  أرجع (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
ساكن u32 json_Hex_منزل_Int4(ثابت حرف *z){
  u32 v;
  v = (json_Hex_منزل_Int(z[0])<<12)
    + (json_Hex_منزل_Int(z[1])<<8)
    + (json_Hex_منزل_Int(z[2])<<4)
    + json_Hex_منزل_Int(z[3]);
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
ساكن صحيح json5Whitespace(ثابت حرف *z_دننه){
  صحيح n = 0;
  ثابت u8 *z = (u8*)z_دننه;
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
            إذا( z[j]==0 ) اذهب whitespace_بشپړ;
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
        اذهب whitespace_بشپړ;
      }
      حالة 0xc2: {
        إذا( z[n+1]==0xa0 ){
          n += 2;
          اكسر;
        }
        اذهب whitespace_بشپړ;
      }
      حالة 0xe1: {
        إذا( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          اكسر;
        }
        اذهب whitespace_بشپړ;
      }
      حالة 0xe2: {
        إذا( z[n+1]==0x80 ){
          u8 c = z[n+2];
          إذا( c<0x80 ) اذهب whitespace_بشپړ;
          إذا( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            اكسر;
          }
        }وإلا إذا( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          اكسر;
        }
        اذهب whitespace_بشپړ;
      }
      حالة 0xe3: {
        إذا( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          اكسر;
        }
        اذهب whitespace_بشپړ;
      }
      حالة 0xef: {
        إذا( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          اكسر;
        }
        اذهب whitespace_بشپړ;
      }
      افتراضي: {
        اذهب whitespace_بشپړ;
      }
    }
  }
  whitespace_بشپړ:
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
  حرف *z_سمون;
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
ساكن فراغ json_Wrong_عدد_دلیلونه(
  sqlite3_context *pCtx,
  ثابت حرف *zFuncName
){
  حرف *z_پیغام = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_پایله_error(pCtx, z_پیغام, -1);
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
  u64 n_اندازه;
  أكد( !pParse->bReadOnly );
  إذا( pParse->oom ) أرجع 0;
  إذا( pParse->nBlobAlloc>0 ) أرجع 1;
  aOld = pParse->aBlob;
  n_اندازه = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  إذا( jsonBlobExpand(pParse, n_اندازه) ){
    أرجع 0;
  }
  أكد( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  حافظه_نقل(pParse->aBlob, aOld, pParse->nBlob);
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
ساكن فراغ json_Blob_Append_غوټه(JsonParse*,u8,u64,ثابت فراغ*);
ساكن SQLITE_NOINLINE فراغ json_Blob_Expand_And_Append_غوټه(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ثابت فراغ *aPayload
){
  إذا( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) أرجع;
  json_Blob_Append_غوټه(pParse, eType, szPayload, aPayload);
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
ساكن فراغ json_Blob_Append_غوټه(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  ثابت فراغ *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  إذا( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_غوټه(pParse,eType,szPayload,aPayload);
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
    حافظه_نقل(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
ساكن صحيح json_Blob_Change_Payload_اندازه(
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
    u64 new_اندازه_2 = (u64)pParse->nBlob + delta;
    إذا( delta>0 ){
      إذا( new_اندازه_2>pParse->nBlobAlloc && jsonBlobExpand(pParse, new_اندازه_2) ){
        أرجع 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }وإلا{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = new_اندازه_2;
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
  u32 i_پای_ټکی,                   /* One more than the last byte of the element */
  u32 i_ژوروالی                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ثابت u8 *z;
  u8 x;
  إذا( i_ژوروالی>JSON_MAX_ژوروالی ) أرجع i+1;
  sz = 0;
  n = jsonb_Payload_اندازه(pParse, i, &sz);
  إذا( NEVER(n==0) ) أرجع i+1;          /* Checked by caller */
  إذا( NEVER(i+n+sz!=i_پای_ټکی) ) أرجع i+1;  /* Checked by caller */
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
      u8 کتل_شوی = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
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
        کتل_شوی = 1;
      }وإلا إذا( z[j]=='0' && x==JSONB_FLOAT ){
        إذا( j+3>k ) أرجع j+1;
        إذا( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) أرجع j+1;
        j++;
      }
      لكل(; j<k; j++){
        إذا( sqlite3Isdigit(z[j]) ) استمر;
        إذا( z[j]=='.' ){
          إذا( کتل_شوی>0 ) أرجع j+1;
          إذا( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            أرجع j+1;
          }
          کتل_شوی = 1;
          استمر;
        }
        إذا( z[j]=='e' || z[j]=='E' ){
          إذا( کتل_شوی==2 ) أرجع j+1;
          إذا( j==k-1 ) أرجع j+1;
          إذا( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            إذا( j==k-1 ) أرجع j+1;
          }
          کتل_شوی = 2;
          استمر;
        }
        أرجع j+1;
      }
      إذا( کتل_شوی==0 ) أرجع i+1;
      أرجع 0;
    }
    حالة JSONB_متن: {
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
        n = jsonb_Payload_اندازه(pParse, j, &sz);
        إذا( n==0 ) أرجع j+1;
        إذا( j+n+sz>k ) أرجع j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_ژوروالی+1);
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
        n = jsonb_Payload_اندازه(pParse, j, &sz);
        إذا( n==0 ) أرجع j+1;
        إذا( j+n+sz>k ) أرجع j+1;
        إذا( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          إذا( x<JSONB_متن || x>JSONB_TEXTRAW ) أرجع j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_ژوروالی+1);
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
ساكن صحيح json_Translate_متن_منزل_Blob(JsonParse *pParse, u32 i){
  حرف c;
  u32 j;
  u32 iThis, i_پیل;
  صحيح x;
  u8 t;
  ثابت حرف *z = pParse->zJson;
json_parse_restart:
  بدل( (u8)z[i] ){
  حالة '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_غوټه(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    إذا( ++pParse->i_ژوروالی > JSON_MAX_ژوروالی ){
      pParse->iErr = i;
      أرجع -1;
    }
    i_پیل = pParse->nBlob;
    لكل(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = json_Translate_متن_منزل_Blob(pParse, j);
      إذا( x<=0 ){
        صحيح op;
        إذا( x==(-2) ){
          j = pParse->iErr;
          إذا( pParse->nBlob!=(u32)i_پیل ) pParse->hasNonstd = 1;
          اكسر;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_متن;
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
          json_Blob_Append_غوټه(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }وإلا{
          إذا( x!=-1 ) pParse->iErr = j;
          أرجع -1;
        }
      }
      إذا( pParse->oom ) أرجع -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      إذا( t<JSONB_متن || t>JSONB_TEXTRAW ){
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
            اذهب parse_object_ارزښت;
          }
        }
        x = json_Translate_متن_منزل_Blob(pParse, j);
        إذا( x!=(-5) ){
          إذا( x!=(-1) ) pParse->iErr = j;
          أرجع -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_ارزښت:
      x = json_Translate_متن_منزل_Blob(pParse, j);
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
        x = json_Translate_متن_منزل_Blob(pParse, j);
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
    json_Blob_Change_Payload_اندازه(pParse, iThis, pParse->nBlob - i_پیل);
    pParse->i_ژوروالی--;
    أرجع j+1;
  }
  حالة '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    أكد( i<=(u32)pParse->nJson );
    json_Blob_Append_غوټه(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    i_پیل = pParse->nBlob;
    إذا( pParse->oom ) أرجع -1;
    إذا( ++pParse->i_ژوروالی > JSON_MAX_ژوروالی ){
      pParse->iErr = i;
      أرجع -1;
    }
    لكل(j=i+1;;j++){
      x = json_Translate_متن_منزل_Blob(pParse, j);
      إذا( x<=0 ){
        إذا( x==(-3) ){
          j = pParse->iErr;
          إذا( pParse->nBlob!=i_پیل ) pParse->hasNonstd = 1;
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
        x = json_Translate_متن_منزل_Blob(pParse, j);
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
    json_Blob_Change_Payload_اندازه(pParse, iThis, pParse->nBlob - i_پیل);
    pParse->i_ژوروالی--;
    أرجع j+1;
  }
  حالة '\'': {
    u8 opcode;
    حرف cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_متن;
    اذهب parse_string;
  حالة '"':
    /* Parse string */
    opcode = JSONB_متن;
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
          إذا( opcode==JSONB_متن ) opcode = JSONB_TEXTJ;
        }وإلا إذا( c=='\'' ||  c=='v' || c=='\n'
#که_تعریف_شوی SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#وإلا
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#د_که_پای
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
    json_Blob_Append_غوټه(pParse, opcode, j-1-i, &z[i+1]);
    أرجع j+1;
  }
  حالة 't': {
    إذا( مزي_پرتله(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      أرجع i+4;
    }
    pParse->iErr = i;
    أرجع -1;
  }
  حالة 'f': {
    إذا( مزي_پرتله(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      أرجع i+5;
    }
    pParse->iErr = i;
    أرجع -1;
  }
  حالة '+': {
    u8 کتل_شوی_E;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    اذهب parse_شمېره;
  حالة '.':
    إذا( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      کتل_شوی_E = 0;
      اذهب parse_شمېره_2;
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
  parse_شمېره:
    کتل_شوی_E = 0;
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
          اذهب parse_شمېره_پای;
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
              json_Blob_Append_غوټه(pParse, JSONB_FLOAT, 6, "-9e999");
            }وإلا{
              json_Blob_Append_غوټه(pParse, JSONB_FLOAT, 5, "9e999");
            }
            أرجع i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          إذا( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            اذهب parse_شمېره_2;
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
            اذهب parse_شمېره_پای;
          }
        }
      }
    }

  parse_شمېره_2:
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
        إذا( کتل_شوی_E ){
          pParse->iErr = j;
          أرجع -1;
        }
        t |= 0x02;
        کتل_شوی_E = 1;
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
  parse_شمېره_پای:
    أكد( JSONB_INT+0x01==JSONB_INT5 );
    أكد( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    أكد( JSONB_INT+0x02==JSONB_FLOAT );
    إذا( z[i]=='+' ) i++;
    json_Blob_Append_غوټه(pParse, JSONB_INT+t, j-i, &z[i]);
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
    إذا( مزي_پرتله(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
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
      إذا( sqlite3StrNICmp(&z[i], aNanInfName[k].z_سمون, nn)!=0 ){
        استمر;
      }
      إذا( sqlite3Isalnum(z[i+nn]) ) استمر;
      إذا( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_غوټه(pParse, JSONB_FLOAT, 5, "9e999");
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
ساكن صحيح json_Convert_متن_منزل_Blob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  صحيح i;
  ثابت حرف *zJson = pParse->zJson;
  i = json_Translate_متن_منزل_Blob(pParse, 0);
  إذا( pParse->oom ) i = -1;
  إذا( i>0 ){
#که_تعریف_شوی SQLITE_DEBUG
    أكد( pParse->i_ژوروالی==0 );
    إذا( sqlite3Config.bJsonSelfcheck ){
      أكد( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#د_که_پای
    طالما( jsonIsspace(zJson[i]) ) i++;
    إذا( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      إذا( zJson[i] ){
        إذا( pCtx ) sqlite3_پایله_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        أرجع 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  إذا( i<=0 ){
    إذا( pCtx!=0 ){
      إذا( pParse->oom ){
        sqlite3_پایله_error_nomem(pCtx);
      }وإلا{
        sqlite3_پایله_error(pCtx, "malformed JSON", -1);
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
  px.nJson = pStr->n_کارول_شوی;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (فراغ)json_Translate_متن_منزل_Blob(&px, 0);
  إذا( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_پایله_error_nomem(pStr->pCtx);
  }وإلا{
    أكد( px.nBlobAlloc>0 );
    أكد( !px.bReadOnly );
    sqlite3_پایله_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
ساكن u32 jsonb_Payload_اندازه_Wide(
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
ساكن u32 jsonb_Payload_اندازه(
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
    أرجع jsonb_Payload_اندازه_Wide(pParse, i, pSz, x);
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

#إذا SQLITE_کاروي_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
ساكن SQLITE_INLINE u32 jsonb_Payload_اندازه_Inline(
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
# define jsonb_Payload_اندازه_Inline(a,b,c) jsonb_Payload_اندازه(a,b,c)
#د_که_پای


#إذا SQLITE_کاروي_INLINE
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
ساكن SQLITE_INLINE u32 jsonb_Payload_اندازه_Semi_Inline(
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
    أرجع jsonb_Payload_اندازه_Wide(pParse,i,pSz,x);
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
# define jsonb_Payload_اندازه_Semi_Inline(a,b,c) jsonb_Payload_اندازه(a,b,c)
#د_که_پای

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
ساكن u32 json_Translate_Blob_منزل_متن(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *p_بهر               /* Write JSON here */
){
  u32 sz, n, j, i_پای_ټکی;

  n = jsonb_Payload_اندازه_Inline(pParse, i, &sz);
  إذا( n==0 ){
    p_بهر->eErr |= JSTRING_MALFORMED;
    أرجع pParse->nBlob+1;
  }
  بدل( pParse->aBlob[i] & 0x0f ){
    حالة JSONB_NULL: {
      jsonAppendRawNZ(p_بهر, "null", 4);
      أرجع i+1;
    }
    حالة JSONB_TRUE: {
      jsonAppendRawNZ(p_بهر, "true", 4);
      أرجع i+1;
    }
    حالة JSONB_FALSE: {
      jsonAppendRawNZ(p_بهر, "false", 5);
      أرجع i+1;
    }
    حالة JSONB_INT:
    حالة JSONB_FLOAT: {
      إذا( sz==0 ) اذهب malformed_jsonb;
      jsonAppendRaw(p_بهر, (ثابت حرف*)&pParse->aBlob[i+n], sz);
      اكسر;
    }
    حالة JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ثابت حرف *z_دننه = (ثابت حرف*)&pParse->aBlob[i+n];
      صحيح b_ډکېدنه = 0;
      إذا( sz==0 ) اذهب malformed_jsonb;
      إذا( z_دننه[0]=='-' ){
        jsonAppendChar(p_بهر, '-');
        k++;
      }وإلا إذا( z_دننه[0]=='+' ){
        k++;
      }
      لكل(; k<sz; k++){
        إذا( !sqlite3Isxdigit(z_دننه[k]) ){
          p_بهر->eErr |= JSTRING_MALFORMED;
          اكسر;
        }وإلا إذا( (u>>60)!=0 ){
          b_ډکېدنه = 1;
        }وإلا{
          u = u*16 + sqlite3_Hex_منزل_Int(z_دننه[k]);
        }
      }
      jsonPrintf(100,p_بهر,b_ډکېدنه?"9.0e999":"%llu", u);
      اكسر;
    }
    حالة JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ثابت حرف *z_دننه = (ثابت حرف*)&pParse->aBlob[i+n];
      إذا( sz==0 ) اذهب malformed_jsonb;
      إذا( z_دننه[0]=='-' ){
        jsonAppendChar(p_بهر, '-');
        إذا( sz<=1 ) اذهب malformed_jsonb;
        k = 1;
      }
      إذا( z_دننه[k]=='.' ){
        jsonAppendChar(p_بهر, '0');
      }
      لكل(; k<sz; k++){
        jsonAppendChar(p_بهر, z_دننه[k]);
        إذا( z_دننه[k]=='.' && (k+1==sz || !sqlite3Isdigit(z_دننه[k+1])) ){
          jsonAppendChar(p_بهر, '0');
        }
      }
      اكسر;
    }
    حالة JSONB_متن:
    حالة JSONB_TEXTJ: {
      إذا( p_بهر->n_کارول_شوی+sz+2<=p_بهر->nAlloc || jsonStringGrow(p_بهر, sz+2)==0 ){
        p_بهر->zBuf[p_بهر->n_کارول_شوی] = '"';
        حافظه_نقل(p_بهر->zBuf+p_بهر->n_کارول_شوی+1,(ثابت حرف*)&pParse->aBlob[i+n],sz);
        p_بهر->zBuf[p_بهر->n_کارول_شوی+sz+1] = '"';
        p_بهر->n_کارول_شوی += sz+2;
      }
      اكسر;
    }
    حالة JSONB_TEXT5: {
      ثابت حرف *z_دننه;
      u32 k;
      u32 sz2 = sz;
      z_دننه = (ثابت حرف*)&pParse->aBlob[i+n];
      jsonAppendChar(p_بهر, '"');
      طالما( sz2>0 ){
        لكل(k=0; k<sz2 && (jsonIsOk[(u8)z_دننه[k]] || z_دننه[k]=='\''); k++){}
        إذا( k>0 ){
          jsonAppendRawNZ(p_بهر, z_دننه, k);
          إذا( k>=sz2 ){
            اكسر;
          }
          z_دننه += k;
          sz2 -= k;
        }
        إذا( z_دننه[0]=='"' ){
          jsonAppendRawNZ(p_بهر, "\\\"", 2);
          z_دننه++;
          sz2--;
          استمر;
        }
        إذا( z_دننه[0]<=0x1f ){
          إذا( p_بهر->n_کارول_شوی+7>p_بهر->nAlloc && jsonStringGrow(p_بهر,7) ) اكسر;
          jsonAppendControlChar(p_بهر, z_دننه[0]);
          z_دننه++;
          sz2--;
          استمر;
        }
        أكد( z_دننه[0]=='\\' );
        أكد( sz2>=1 );
        إذا( sz2<2 ){
          p_بهر->eErr |= JSTRING_MALFORMED;
          اكسر;
        }
        بدل( (u8)z_دننه[1] ){
          حالة '\'':
            jsonAppendChar(p_بهر, '\'');
            اكسر;
          حالة 'v':
            jsonAppendRawNZ(p_بهر, "\\u000b", 6);
            اكسر;
          حالة 'x':
            إذا( sz2<4 ){
              p_بهر->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              اكسر;
            }
            jsonAppendRawNZ(p_بهر, "\\u00", 4);
            jsonAppendRawNZ(p_بهر, &z_دننه[2], 2);
            z_دننه += 2;
            sz2 -= 2;
            اكسر;
          حالة '0':
            jsonAppendRawNZ(p_بهر, "\\u0000", 6);
            اكسر;
          حالة '\r':
            إذا( sz2>2 && z_دننه[2]=='\n' ){
              z_دننه++;
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
             || 0x80!=(u8)z_دننه[2]
             || (0xa8!=(u8)z_دننه[3] && 0xa9!=(u8)z_دننه[3])
            ){
              p_بهر->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              اكسر;
            }
            z_دننه += 2;
            sz2 -= 2;
            اكسر;
          افتراضي:
            jsonAppendRawNZ(p_بهر, z_دننه, 2);
            اكسر;
        }
        أكد( sz2>=2 );
        z_دننه += 2;
        sz2 -= 2;
      }
      jsonAppendChar(p_بهر, '"');
      اكسر;
    }
    حالة JSONB_TEXTRAW: {
      jsonAppendString(p_بهر, (ثابت حرف*)&pParse->aBlob[i+n], sz);
      اكسر;
    }
    حالة JSONB_ARRAY: {
      jsonAppendChar(p_بهر, '[');
      j = i+n;
      i_پای_ټکی = j+sz;
      إذا( ++pParse->i_ژوروالی > JSON_MAX_ژوروالی ){
        jsonStringTooDeep(p_بهر);
      }
      طالما( j<i_پای_ټکی && p_بهر->eErr==0 ){
        j = json_Translate_Blob_منزل_متن(pParse, j, p_بهر);
        jsonAppendChar(p_بهر, ',');
      }
      pParse->i_ژوروالی--;
      إذا( j>i_پای_ټکی ) p_بهر->eErr |= JSTRING_MALFORMED;
      إذا( sz>0 ) jsonStringTrimOneChar(p_بهر);
      jsonAppendChar(p_بهر, ']');
      اكسر;
    }
    حالة JSONB_OBJECT: {
      صحيح x = 0;
      jsonAppendChar(p_بهر, '{');
      j = i+n;
      i_پای_ټکی = j+sz;
      إذا( ++pParse->i_ژوروالی > JSON_MAX_ژوروالی ){
        jsonStringTooDeep(p_بهر);
      }
      طالما( j<i_پای_ټکی && p_بهر->eErr==0 ){
        j = json_Translate_Blob_منزل_متن(pParse, j, p_بهر);
        jsonAppendChar(p_بهر, (x++ & 1) ? ',' : ':');
      }
      pParse->i_ژوروالی--;
      إذا( (x & 1)!=0 || j>i_پای_ټکی ) p_بهر->eErr |= JSTRING_MALFORMED;
      إذا( sz>0 ) jsonStringTrimOneChar(p_بهر);
      jsonAppendChar(p_بهر, '}');
      اكسر;
    }

    افتراضي: {
      malformed_jsonb:
      p_بهر->eErr |= JSTRING_MALFORMED;
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
  JsonString *p_بهر;         /* Generate pretty output into this string */
  ثابت حرف *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
ساكن فراغ jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  لكل(jj=0; jj<pPretty->nIndent; jj++){
    jsonAppendRaw(pPretty->p_بهر, pPretty->zIndent, pPretty->szIndent);
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
ساكن u32 json_Translate_Blob_منزل_Pretty_متن(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, i_پای_ټکی;
  JsonParse *pParse = pPretty->pParse;
  JsonString *p_بهر = pPretty->p_بهر;
  n = jsonb_Payload_اندازه(pParse, i, &sz);
  إذا( n==0 ){
    p_بهر->eErr |= JSTRING_MALFORMED;
    أرجع pParse->nBlob+1;
  }
  بدل( pParse->aBlob[i] & 0x0f ){
    حالة JSONB_ARRAY: {
      j = i+n;
      i_پای_ټکی = j+sz;
      jsonAppendChar(p_بهر, '[');
      إذا( j<i_پای_ټکی ){
        jsonAppendChar(p_بهر, '\n');
        pPretty->nIndent++;
        إذا( pPretty->nIndent >= JSON_MAX_ژوروالی ){
          jsonStringTooDeep(p_بهر);
        }
        طالما( p_بهر->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_منزل_Pretty_متن(pPretty, j);
          إذا( j>=i_پای_ټکی ){
            إذا( j>i_پای_ټکی ) p_بهر->eErr |= JSTRING_MALFORMED;
            اكسر;
          }
          jsonAppendRawNZ(p_بهر, ",\n", 2);
        }
        jsonAppendChar(p_بهر, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_بهر, ']');
      i = i_پای_ټکی;
      اكسر;
    }
    حالة JSONB_OBJECT: {
      j = i+n;
      i_پای_ټکی = j+sz;
      jsonAppendChar(p_بهر, '{');
      إذا( j<i_پای_ټکی ){
        jsonAppendChar(p_بهر, '\n');
        pPretty->nIndent++;
        إذا( pPretty->nIndent >= JSON_MAX_ژوروالی ){
          jsonStringTooDeep(p_بهر);
        }
        pParse->i_ژوروالی = pPretty->nIndent;
        طالما( p_بهر->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_منزل_متن(pParse, j, p_بهر);
          إذا( j>i_پای_ټکی ){
            p_بهر->eErr |= JSTRING_MALFORMED;
            اكسر;
          }
          jsonAppendRawNZ(p_بهر, ": ", 2);
          j = json_Translate_Blob_منزل_Pretty_متن(pPretty, j);
          إذا( j>=i_پای_ټکی ) اكسر;
          jsonAppendRawNZ(p_بهر, ",\n", 2);
        }
        jsonAppendChar(p_بهر, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_بهر, '}');
      i = i_پای_ټکی;
      اكسر;
    }
    افتراضي: {
      i = json_Translate_Blob_منزل_متن(pParse, i, p_بهر);
      اكسر;
    }
  }
  أرجع i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
ساكن u32 jsonb_Array_شمېر(JsonParse *pParse, u32 i_ریښه){
  u32 n, sz, i, i_پای_ټکی;
  u32 k = 0;
  n = jsonb_Payload_اندازه(pParse, i_ریښه, &sz);
  i_پای_ټکی = i_ریښه+n+sz;
  لكل(i=i_ریښه+n; n>0 && i<i_پای_ټکی; i+=sz+n, k++){
    n = jsonb_Payload_اندازه_Semi_Inline(pParse, i, &sz);
  }
  أرجع k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
ساكن فراغ json_After_Edit_اندازه_Adjust(JsonParse *pParse, u32 i_ریښه){
  u32 sz = 0;
  u32 nBlob;
  أكد( pParse->delta!=0 );
  أكد( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (فراغ)jsonb_Payload_اندازه(pParse, i_ریښه, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += json_Blob_Change_Payload_اندازه(pParse, i_ریښه, sz);
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
  u8 *a_بهر,                 /* Overwrite here */
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
  a_بهر[0] = (aIns[0] & 0x0f) | aType[i-2];
  حافظه_نقل(&a_بهر[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  طالما( 1/*edit-by-break*/ ){
    i--;
    a_بهر[i] = szPayload & 0xff;
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
    حافظه_نقل(&pParse->aBlob[iDel], aIns, nIns);
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
ساكن u32 json_بایټونه_منزل_Bypass(ثابت حرف *z, u32 n){
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
ساكن u32 jsonUnescapeOneChar(ثابت حرف *z, u32 n, u32 *pi_بهر){
  أكد( n>0 );
  أكد( z[0]=='\\' );
  إذا( n<2 ){
    *pi_بهر = JSON_INVALID_CHAR;
    أرجع n;
  }
  بدل( (u8)z[1] ){
    حالة 'u': {
      u32 v, vlo;
      إذا( n<6 ){
        *pi_بهر = JSON_INVALID_CHAR;
        أرجع n;
      }
      v = json_Hex_منزل_Int4(&z[2]);
      إذا( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = json_Hex_منزل_Int4(&z[8]))&0xfc00)==0xdc00
      ){
        *pi_بهر = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        أرجع 12;
      }وإلا{
        *pi_بهر = v;
        أرجع 6;
      }
    }
    حالة 'b': {   *pi_بهر = '\b';  أرجع 2; }
    حالة 'f': {   *pi_بهر = '\f';  أرجع 2; }
    حالة 'n': {   *pi_بهر = '\n';  أرجع 2; }
    حالة 'r': {   *pi_بهر = '\r';  أرجع 2; }
    حالة 't': {   *pi_بهر = '\t';  أرجع 2; }
    حالة 'v': {   *pi_بهر = '\v';  أرجع 2; }
    حالة '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#که_تعریف_شوی SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *pi_بهر = 0;
#وإلا
      /* Correct behavior */
      *pi_بهر = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#د_که_پای
      أرجع 2;
    }
    حالة '\'':
    حالة '"':
    حالة '/':
    حالة '\\':{   *pi_بهر = z[1];  أرجع 2; }
    حالة 'x': {
      إذا( n<4 ){
        *pi_بهر = JSON_INVALID_CHAR;
        أرجع n;
      }
      *pi_بهر = (json_Hex_منزل_Int(z[2])<<4) | json_Hex_منزل_Int(z[3]);
      أرجع 4;
    }
    حالة 0xe2:
    حالة '\r':
    حالة '\n': {
      u32 nSkip = json_بایټونه_منزل_Bypass(z, n);
      إذا( nSkip==0 ){
        *pi_بهر = JSON_INVALID_CHAR;
        أرجع n;
      }وإلا إذا( nSkip==n ){
        *pi_بهر = 0;
        أرجع n;
      }وإلا إذا( z[nSkip]=='\\' ){
        أرجع nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, pi_بهر);
      }وإلا{
        صحيح sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, pi_بهر);
        أرجع nSkip + sz;
      }
    }
    افتراضي: {
      *pi_بهر = JSON_INVALID_CHAR;
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
  ثابت حرف *z_کیڼ,          /* The left label */
  u32 n_کیڼ,                  /* Size of the left label in bytes */
  صحيح raw_کیڼ,                /* True if zLeft contains no escapes */
  ثابت حرف *z_ښی,         /* The right label */
  u32 n_ښی,                 /* Size of the right label in bytes */
  صحيح raw_ښی                /* True if zRight is escape-free */
){
  u32 c_کیڼ, c_ښی;
  أكد( raw_کیڼ==0 || raw_ښی==0 );
  طالما( 1 /*exit-by-return*/ ){
    إذا( n_کیڼ==0 ){
      c_کیڼ = 0;
    }وإلا إذا( raw_کیڼ || z_کیڼ[0]!='\\' ){
      c_کیڼ = ((u8*)z_کیڼ)[0];
      إذا( c_کیڼ>=0xc0 ){
        صحيح sz = sqlite3Utf8ReadLimited((u8*)z_کیڼ, n_کیڼ, &c_کیڼ);
        z_کیڼ += sz;
        n_کیڼ -= sz;
      }وإلا{
        z_کیڼ++;
        n_کیڼ--;
      }
    }وإلا{
      u32 n = jsonUnescapeOneChar(z_کیڼ, n_کیڼ, &c_کیڼ);
      z_کیڼ += n;
      أكد( n<=n_کیڼ );
      n_کیڼ -= n;
    }
    إذا( n_ښی==0 ){
      c_ښی = 0;
    }وإلا إذا( raw_ښی || z_ښی[0]!='\\' ){
      c_ښی = ((u8*)z_ښی)[0];
      إذا( c_ښی>=0xc0 ){
        صحيح sz = sqlite3Utf8ReadLimited((u8*)z_ښی, n_ښی, &c_ښی);
        z_ښی += sz;
        n_ښی -= sz;
      }وإلا{
        z_ښی++;
        n_ښی--;
      }
    }وإلا{
      u32 n = jsonUnescapeOneChar(z_ښی, n_ښی, &c_ښی);
      z_ښی += n;
      أكد( n<=n_ښی );
      n_ښی -= n;
    }
    إذا( c_کیڼ!=c_ښی ) أرجع 0;
    إذا( c_کیڼ==0 ) أرجع 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
ساكن صحيح jsonLabelCompare(
  ثابت حرف *z_کیڼ,          /* The left label */
  u32 n_کیڼ,                  /* Size of the left label in bytes */
  صحيح raw_کیڼ,                /* True if zLeft contains no escapes */
  ثابت حرف *z_ښی,         /* The right label */
  u32 n_ښی,                 /* Size of the right label in bytes */
  صحيح raw_ښی                /* True if zRight is escape-free */
){
  إذا( raw_کیڼ && raw_ښی ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    إذا( n_کیڼ!=n_ښی ) أرجع 0;
    أرجع memcmp(z_کیڼ, z_ښی, n_کیڼ)==0;
  }وإلا{
    أرجع jsonLabelCompareEscaped(z_کیڼ, n_کیڼ, raw_کیڼ,
                                   z_ښی, n_ښی, raw_ښی);
  }
}

/*
** Error returns from jsonLookupStep()
*/
#تعریف_کړه JSON_LOOKUP_ERROR      0xffffffff
#تعریف_کړه JSON_LOOKUP_NOTFOUND   0xfffffffe
#تعریف_کړه JSON_LOOKUP_NOTARRAY   0xfffffffd
#تعریف_کړه JSON_LOOKUP_TOODEEP    0xfffffffc
#تعریف_کړه JSON_LOOKUP_PATHERROR  0xfffffffb
#تعریف_کړه JSON_LOOKUP_ISERROR(x) ((x)>=JSON_LOOKUP_PATHERROR)

/* Forward declaration */
ساكن u32 json_Lookup_ګام(JsonParse*,u32,ثابت حرف*,u32);


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
  ثابت حرف *z_لکۍ   /* Tail of the path that determines substructure */
){
  ساكن ثابت u8 تشه_Object[] = { JSONB_ARRAY, JSONB_OBJECT };
  صحيح rc;
  memset(pIns, 0, حجم(*pIns));
  pIns->db = pParse->db;
  إذا( z_لکۍ[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }وإلا{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&تشه_Object[z_لکۍ[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->i_ژوروالی = pParse->i_ژوروالی+1;
    إذا( pIns->i_ژوروالی >= JSON_MAX_ژوروالی ){
      أرجع JSON_LOOKUP_TOODEEP;
    }
    rc = json_Lookup_ګام(pIns, 0, z_لکۍ, 0);
    pParse->i_ژوروالی--;
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
ساكن u32 json_Lookup_ګام(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_ریښه,              /* Begin the search at this element of aBlob[] */
  ثابت حرف *z_لار,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_کلی, sz, n, i_پای_ټکی, rc;
  ثابت حرف *z_کلی;
  u8 x;

  إذا( z_لار[0]==0 ){
    إذا( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonb_Payload_اندازه(pParse, i_ریښه, &sz);
      sz += n;
      إذا( pParse->eEdit==JEDIT_DEL ){
        إذا( iLabel>0 ){
          sz += i_ریښه - iLabel;
          i_ریښه = iLabel;
        }
        jsonBlobEdit(pParse, i_ریښه, sz, 0, 0);
      }وإلا إذا( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }وإلا إذا( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        إذا( z_لار[-1]!=']' ){
          أرجع JSON_LOOKUP_NOTARRAY;
        }وإلا{
          jsonBlobEdit(pParse, i_ریښه, 0, pParse->aIns, pParse->nIns);
        }
      }وإلا{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_ریښه, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    أرجع i_ریښه;
  }
  إذا( z_لار[0]=='.' ){
    صحيح raw_کلی = 1;
    x = pParse->aBlob[i_ریښه];
    z_لار++;
    إذا( z_لار[0]=='"' ){
      z_کلی = z_لار + 1;
      لكل(i=1; z_لار[i] && z_لار[i]!='"'; i++){
        إذا( z_لار[i]=='\\' && z_لار[i+1]!=0 ) i++;
      }
      n_کلی = i-1;
      إذا( z_لار[i] ){
        i++;
      }وإلا{
        أرجع JSON_LOOKUP_PATHERROR;
      }
      testcase( n_کلی==0 );
      raw_کلی = memchr(z_کلی, '\\', n_کلی)==0;
    }وإلا{
      z_کلی = z_لار;
      لكل(i=0; z_لار[i] && z_لار[i]!='.' && z_لار[i]!='['; i++){}
      n_کلی = i;
      إذا( n_کلی==0 ){
        أرجع JSON_LOOKUP_PATHERROR;
      }
    }
    إذا( (x & 0x0f)!=JSONB_OBJECT ) أرجع JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_اندازه(pParse, i_ریښه, &sz);
    j = i_ریښه + n;  /* j is the index of a label */
    i_پای_ټکی = j+sz;
    طالما( j<i_پای_ټکی ){
      صحيح rawLabel;
      ثابت حرف *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      إذا( x<JSONB_متن || x>JSONB_TEXTRAW ) أرجع JSON_LOOKUP_ERROR;
      n = jsonb_Payload_اندازه_Semi_Inline(pParse, j, &sz);
      إذا( n==0 ) أرجع JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      إذا( k+sz>=i_پای_ټکی ) أرجع JSON_LOOKUP_ERROR;
      zLabel = (ثابت حرف*)&pParse->aBlob[k];
      rawLabel = x==JSONB_متن || x==JSONB_TEXTRAW;
      إذا( jsonLabelCompare(z_کلی, n_کلی, raw_کلی, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        إذا( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) أرجع JSON_LOOKUP_ERROR;
        n = jsonb_Payload_اندازه(pParse, v, &sz);
        إذا( n==0 || v+n+sz>i_پای_ټکی ) أرجع JSON_LOOKUP_ERROR;
        أكد( j>0 );
        إذا( ++pParse->i_ژوروالی >= JSON_MAX_ژوروالی ){
          أرجع JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_ګام(pParse, v, &z_لار[i], j);
        pParse->i_ژوروالی--;
        إذا( pParse->delta ) json_After_Edit_اندازه_Adjust(pParse, i_ریښه);
        أرجع rc;
      }
      j = k+sz;
      إذا( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) أرجع JSON_LOOKUP_ERROR;
      n = jsonb_Payload_اندازه_Semi_Inline(pParse, j, &sz);
      إذا( n==0 ) أرجع JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    إذا( j>i_پای_ټکی ) أرجع JSON_LOOKUP_ERROR;
    إذا( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_سېټ );
      testcase( pParse->eEdit==JEDIT_AINS );
      إذا( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&z_لار[i])!=0 ){
        أرجع JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, حجم(ix));
      ix.db = pParse->db;
      json_Blob_Append_غوټه(&ix, raw_کلی?JSONB_TEXTRAW:JSONB_TEXT5, n_کلی, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &z_لار[i]);
      إذا( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_کلی+v.nBlob)
      ){
        أكد( !pParse->oom );
        nIns = ix.nBlob + n_کلی + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        إذا( !pParse->oom ){
          أكد( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          أكد( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          حافظه_نقل(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          حافظه_نقل(&pParse->aBlob[k], z_کلی, n_کلی);
          k += n_کلی;
          حافظه_نقل(&pParse->aBlob[k], v.aBlob, v.nBlob);
          إذا( ALWAYS(pParse->delta) ) json_After_Edit_اندازه_Adjust(pParse, i_ریښه);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      أرجع rc;
    }
  }وإلا إذا( z_لار[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_ریښه] & 0x0f;
    إذا( x!=JSONB_ARRAY )  أرجع JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_اندازه(pParse, i_ریښه, &sz);
    i = 1;
    طالما( sqlite3Isdigit(z_لار[i]) ){
      إذا( kk<0xffffffff ) kk = kk*10 + z_لار[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    إذا( i<2 || z_لار[i]!=']' ){
      إذا( z_لار[1]=='#' ){
        kk = jsonb_Array_شمېر(pParse, i_ریښه);
        i = 2;
        إذا( z_لار[2]=='-' && sqlite3Isdigit(z_لار[3]) ){
          u64 nn = 0;
          i = 3;
          افعل{
            إذا( nn<0xffffffff ) nn = nn*10 + z_لار[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }طالما( sqlite3Isdigit(z_لار[i]) );
          إذا( nn>kk ) أرجع JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        إذا( z_لار[i]!=']' ){
          أرجع JSON_LOOKUP_PATHERROR;
        }
      }وإلا{
        أرجع JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_ریښه+n;
    i_پای_ټکی = j+sz;
    طالما( j<i_پای_ټکی ){
      إذا( kk==0 ){
        إذا( ++pParse->i_ژوروالی >= JSON_MAX_ژوروالی ){
          أرجع JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_ګام(pParse, j, &z_لار[i+1], 0);
        pParse->i_ژوروالی--;
        إذا( pParse->delta ) json_After_Edit_اندازه_Adjust(pParse, i_ریښه);
        أرجع rc;
      }
      kk--;
      n = jsonb_Payload_اندازه_Semi_Inline(pParse, j, &sz);
      إذا( n==0 ) أرجع JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    إذا( j>i_پای_ټکی ) أرجع JSON_LOOKUP_ERROR;
    إذا( kk>0 ) أرجع JSON_LOOKUP_NOTFOUND;
    إذا( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_سېټ );
      rc = jsonCreateEditSubstructure(pParse, &v, &z_لار[i+1]);
      إذا( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        أكد( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      إذا( pParse->delta ) json_After_Edit_اندازه_Adjust(pParse, i_ریښه);
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
ساكن فراغ json_Return_متن_Json_From_Blob(
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
  json_Translate_Blob_منزل_متن(&x, 0, &s);
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
  صحيح e_ډېرپېښ                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  صحيح rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  أكد( e_ډېرپېښ>=0 && e_ډېرپېښ<=2 );
  n = jsonb_Payload_اندازه(pParse, i, &sz);
  إذا( n==0 ){
    sqlite3_پایله_error(pCtx, "malformed JSON", -1);
    أرجع;
  }
  بدل( pParse->aBlob[i] & 0x0f ){
    حالة JSONB_NULL: {
      إذا( sz ) اذهب returnfromblob_malformed;
      sqlite3_پایله_null(pCtx);
      اكسر;
    }
    حالة JSONB_TRUE: {
      إذا( sz ) اذهب returnfromblob_malformed;
      sqlite3_پایله_int(pCtx, 1);
      اكسر;
    }
    حالة JSONB_FALSE: {
      إذا( sz ) اذهب returnfromblob_malformed;
      sqlite3_پایله_int(pCtx, 0);
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
      rc = sqlite3_Dec_Or_Hex_منزل_I64(z, &iRes);
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
          sqlite3_پایله_double(pCtx, bNeg ? -r : r);
        }وإلا{
          sqlite3_پایله_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }وإلا إذا( rc==3 && bNeg ){
        sqlite3_پایله_int64(pCtx, تر_ټولو_کوچنی_INT64);
      }وإلا إذا( rc==1 ){
        اذهب returnfromblob_malformed;
      }وإلا{
        إذا( bNeg ){ n--; sz++; }
        اذهب منزل_double;
      }
      اكسر;
    }
    حالة JSONB_FLOAT5:
    حالة JSONB_FLOAT: {
      مزدوج r;
      حرف *z;
      إذا( sz==0 ) اذهب returnfromblob_malformed;
    منزل_double:
      z = sqlite3DbStrNDup(db, (ثابت حرف*)&pParse->aBlob[i+n], (صحيح)sz);
      إذا( z==0 ) اذهب returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      إذا( rc<=0 ) اذهب returnfromblob_malformed;
      sqlite3_پایله_double(pCtx, r);
      اكسر;
    }
    حالة JSONB_TEXTRAW:
    حالة JSONB_متن: {
      sqlite3_پایله_متن(pCtx, (حرف*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      اكسر;
    }
    حالة JSONB_TEXT5:
    حالة JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 i_دننه, i_بهر;
      ثابت حرف *z;
      حرف *z_بهر;
      u32 n_بهر = sz;
      z = (ثابت حرف*)&pParse->aBlob[i+n];
      z_بهر = sqlite3DbMallocRaw(db, ((u64)n_بهر)+1);
      إذا( z_بهر==0 ) اذهب returnfromblob_oom;
      لكل(i_دننه=i_بهر=0; i_دننه<sz; i_دننه++){
        حرف c = z[i_دننه];
        إذا( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[i_دننه], sz-i_دننه, &v);
          إذا( v==0 && i_دننه+szEscape==sz
           && (z[i_دننه+1]=='\n' || z[i_دننه+1]=='\r' || (u8)z[i_دننه+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }وإلا إذا( v<=0x7f ){
            z_بهر[i_بهر++] = (حرف)v;
          }وإلا إذا( v<=0x7ff ){
            أكد( szEscape>=2 );
            z_بهر[i_بهر++] = (حرف)(0xc0 | (v>>6));
            z_بهر[i_بهر++] = 0x80 | (v&0x3f);
          }وإلا إذا( v<0x10000 ){
            أكد( szEscape>=3 );
            z_بهر[i_بهر++] = 0xe0 | (v>>12);
            z_بهر[i_بهر++] = 0x80 | ((v>>6)&0x3f);
            z_بهر[i_بهر++] = 0x80 | (v&0x3f);
          }وإلا إذا( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }وإلا{
            أكد( szEscape>=4 );
            z_بهر[i_بهر++] = 0xf0 | (v>>18);
            z_بهر[i_بهر++] = 0x80 | ((v>>12)&0x3f);
            z_بهر[i_بهر++] = 0x80 | ((v>>6)&0x3f);
            z_بهر[i_بهر++] = 0x80 | (v&0x3f);
          }
          i_دننه += szEscape - 1;
        }وإلا{
          z_بهر[i_بهر++] = c;
        }
      } /* end for() */
      أكد( i_بهر<=n_بهر );
      z_بهر[i_بهر] = 0;
      sqlite3_پایله_متن(pCtx, z_بهر, i_بهر, SQLITE_DYNAMIC);
      اكسر;
    }
    حالة JSONB_ARRAY:
    حالة JSONB_OBJECT: {
      إذا( e_ډېرپېښ==0 ){
        إذا( (SQLITE_PTR_منزل_INT(sqlite3_user_معلومات(pCtx)) & JSON_BLOB)!=0 ){
          e_ډېرپېښ = 2;
        }وإلا{
          e_ډېرپېښ = 1;
        }
      }
      إذا( e_ډېرپېښ==2 ){
        sqlite3_پایله_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }وإلا{
        json_Return_متن_Json_From_Blob(pCtx, &pParse->aBlob[i], sz+n);
      }
      اكسر;
    }
    افتراضي: {
      اذهب returnfromblob_malformed;
    }
  }
  أرجع;

returnfromblob_oom:
  sqlite3_پایله_error_nomem(pCtx);
  أرجع;

returnfromblob_malformed:
  sqlite3_پایله_error(pCtx, "malformed JSON", -1);
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
ساكن صحيح json_Function_Arg_منزل_Blob(
  sqlite3_context *ctx,
  sqlite3_ارزښت *pArg,
  JsonParse *pParse
){
  صحيح eType = sqlite3_ارزښت_type(pArg);
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
        sqlite3_پایله_error(ctx, "JSON cannot hold BLOB values", -1);
        أرجع 1;
      }
      اكسر;
    }
    حالة SQLITE_متن: {
      ثابت حرف *zJson = (ثابت حرف*)sqlite3_ارزښت_متن(pArg);
      صحيح nJson = sqlite3_ارزښت_بایټونه(pArg);
      إذا( zJson==0 ) أرجع 1;
      إذا( sqlite3_ارزښت_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (حرف*)zJson;
        pParse->nJson = nJson;
        إذا( json_Convert_متن_منزل_Blob(pParse, ctx) ){
          sqlite3_پایله_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, حجم(pParse[0]));
          أرجع 1;
        }
      }وإلا{
        json_Blob_Append_غوټه(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      اكسر;
    }
    حالة SQLITE_FLOAT: {
      إذا( NEVER(sqlite3IsNaN(sqlite3_ارزښت_double(pArg))) ){
        json_Blob_Append_غوټه(pParse, JSONB_NULL, 0, 0);
      }وإلا{
        صحيح n = sqlite3_ارزښت_بایټونه(pArg);
        ثابت حرف *z = (ثابت حرف*)sqlite3_ارزښت_متن(pArg);
        إذا( z==0 ) أرجع 1;
        إذا( z[0]=='I' ){
          json_Blob_Append_غوټه(pParse, JSONB_FLOAT, 5, "9e999");
        }وإلا إذا( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_غوټه(pParse, JSONB_FLOAT, 6, "-9e999");
        }وإلا{
          json_Blob_Append_غوټه(pParse, JSONB_FLOAT, n, z);
        }
      }
      اكسر;
    }
    حالة SQLITE_INTEGER: {
      صحيح n = sqlite3_ارزښت_بایټونه(pArg);
      ثابت حرف *z = (ثابت حرف*)sqlite3_ارزښت_متن(pArg);
      إذا( z==0 ) أرجع 1;
      json_Blob_Append_غوټه(pParse, JSONB_INT, n, z);
      اكسر;
    }
  }
  إذا( pParse->oom ){
    sqlite3_پایله_error_nomem(ctx);
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
ساكن حرف *json_Bad_لار_Error(
  sqlite3_context *ctx,     /* The function call containing the error */
  ثابت حرف *z_لار,        /* The path with the problem */
  صحيح rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  حرف *z_پیغام;
  إذا( rc==(صحيح)JSON_LOOKUP_NOTARRAY ){
    z_پیغام = sqlite3_mprintf("not an array element: %Q", z_لار);
  }وإلا إذا( rc==(صحيح)JSON_LOOKUP_ERROR ){
    z_پیغام = sqlite3_mprintf("malformed JSON");
  }وإلا إذا( rc==(صحيح)JSON_LOOKUP_TOODEEP ){
    z_پیغام = sqlite3_mprintf("JSON path too deep");
  }وإلا{
    z_پیغام = sqlite3_mprintf("bad JSON path: %Q", z_لار);
  }
  إذا( ctx==0 ) أرجع z_پیغام;
  إذا( z_پیغام ){
    sqlite3_پایله_error(ctx, z_پیغام, -1);
    sqlite3_free(z_پیغام);
  }وإلا{
    sqlite3_پایله_error_nomem(ctx);
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
ساكن فراغ json_ورننباسه_Into_Blob(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv,
  صحيح eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  صحيح i;
  u32 rc = 0;
  ثابت حرف *z_لار = 0;
  صحيح flgs;
  JsonParse *p;
  JsonParse ax;

  أكد( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  إذا( p==0 ) أرجع;
  لكل(i=1; i<argc-1; i+=2){
    إذا( sqlite3_ارزښت_type(argv[i])==SQLITE_NULL ) استمر;
    z_لار = (ثابت حرف*)sqlite3_ارزښت_متن(argv[i]);
    إذا( z_لار==0 ){
      sqlite3_پایله_error_nomem(ctx);
      jsonParseFree(p);
      أرجع;
    }
    إذا( z_لار[0]!='$' ) اذهب json_ورننباسه_Into_Blob_patherror;
    إذا( json_Function_Arg_منزل_Blob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      أرجع;
    }
    إذا( z_لار[1]==0 ){
      إذا( eEdit==JEDIT_REPL || eEdit==JEDIT_سېټ ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }وإلا{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->i_ژوروالی = 0;
      rc = json_Lookup_ګام(p, 0, z_لار+1, 0);
    }
    jsonParseReset(&ax);
    إذا( rc==JSON_LOOKUP_NOTFOUND ) استمر;
    إذا( JSON_LOOKUP_ISERROR(rc) ) اذهب json_ورننباسه_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  أرجع;

json_ورننباسه_Into_Blob_patherror:
  jsonParseFree(p);
  json_Bad_لار_Error(ctx, z_لار, rc);
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
ساكن صحيح jsonArgIsJsonb(sqlite3_ارزښت *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  إذا( sqlite3_ارزښت_type(pArg)!=SQLITE_BLOB ) أرجع 0;
  p->aBlob = (u8*)sqlite3_ارزښت_blob(pArg);
  p->nBlob = (u32)sqlite3_ارزښت_بایټونه(pArg);
  إذا( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonb_Payload_اندازه(p, 0, &sz))>0
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
  sqlite3_ارزښت *pArg,
  u32 flgs
){
  صحيح eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  أكد( ctx!=0 );
  eType = sqlite3_ارزښت_type(pArg);
  إذا( eType==SQLITE_NULL ){
    أرجع 0;
  }
  إذا( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_پلټنه(ctx, pArg);
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
    حافظه_نقل(p->aBlob, pFromCache->aBlob, nBlob);
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
  p->zJson = (حرف*)sqlite3_ارزښت_متن(pArg);
  p->nJson = sqlite3_ارزښت_بایټونه(pArg);
  إذا( db->mallocFailed ) اذهب json_pfa_oom;
  إذا( p->nJson==0 ) اذهب json_pfa_malformed;
  أكد( p->zJson!=0 );
  إذا( json_Convert_متن_منزل_Blob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    إذا( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      أرجع p;
    }وإلا{
      jsonParseFree(p);
      أرجع 0;
    }
  }وإلا{
    صحيح isRCStr = sqlite3_ارزښت_Is_د_Class(pArg, sqlite3RCStrUnref);
    صحيح rc;
    إذا( !isRCStr ){
      حرف *zNew = sqlite3RCStrNew( p->nJson );
      إذا( zNew==0 ) اذهب json_pfa_oom;
      حافظه_نقل(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }وإلا{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_ورننباسه(ctx, p);
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
    sqlite3_پایله_error(ctx, "malformed JSON", -1);
    أرجع 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_پایله_error_nomem(ctx);
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
    sqlite3_پایله_error_nomem(ctx);
    أرجع;
  }
  flgs = SQLITE_PTR_منزل_INT(sqlite3_user_معلومات(ctx));
  إذا( flgs & JSON_BLOB ){
    إذا( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_پایله_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }وإلا{
      sqlite3_پایله_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }وإلا{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    json_Translate_Blob_منزل_متن(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
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
  u32 i_پیل,        /* Start rendering here */
  u32 i_پای_ټکی,          /* Do not render this byte or any byte after this one */
  صحيح nIndent,       /* Indent by this many spaces */
  sqlite3_str *p_بهر  /* Generate output into this sqlite3_str object */
){
  طالما( i_پیل<i_پای_ټکی ){
    u32 i, n, nn, sz = 0;
    صحيح showContent = 1;
    u8 x = pParse->aBlob[i_پیل] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(p_بهر, "%5d:%*s", i_پیل, nIndent, "");
    إذا( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonb_Payload_اندازه(pParse, i_پیل, &sz);
    إذا( nn==0 ) nn = 1;
    إذا( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    لكل(i=0; i<nn; i++){
      sqlite3_str_appendf(p_بهر, " %02x", pParse->aBlob[i_پیل+i]);
    }
    إذا( n==0 ){
      sqlite3_str_appendf(p_بهر, "   ERROR invalid node size\n");
      i_پیل = n==0 ? i_پیل+1 : i_پای_ټکی;
      استمر;
    }
    pParse->nBlob = savedNBlob;
    إذا( i_پیل+n+sz>i_پای_ټکی ){
      i_پای_ټکی = i_پیل+n+sz;
      إذا( i_پای_ټکی>pParse->nBlob ){
        إذا( pParse->nBlobAlloc>0 && i_پای_ټکی>pParse->nBlobAlloc ){
          i_پای_ټکی = pParse->nBlobAlloc;
        }وإلا{
          i_پای_ټکی = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(p_بهر,"  <-- ");
    بدل( x ){
      حالة JSONB_NULL:     sqlite3_str_appendall(p_بهر,"null"); اكسر;
      حالة JSONB_TRUE:     sqlite3_str_appendall(p_بهر,"true"); اكسر;
      حالة JSONB_FALSE:    sqlite3_str_appendall(p_بهر,"false"); اكسر;
      حالة JSONB_INT:      sqlite3_str_appendall(p_بهر,"int"); اكسر;
      حالة JSONB_INT5:     sqlite3_str_appendall(p_بهر,"int5"); اكسر;
      حالة JSONB_FLOAT:    sqlite3_str_appendall(p_بهر,"float"); اكسر;
      حالة JSONB_FLOAT5:   sqlite3_str_appendall(p_بهر,"float5"); اكسر;
      حالة JSONB_متن:     sqlite3_str_appendall(p_بهر,"text"); اكسر;
      حالة JSONB_TEXTJ:    sqlite3_str_appendall(p_بهر,"textj"); اكسر;
      حالة JSONB_TEXT5:    sqlite3_str_appendall(p_بهر,"text5"); اكسر;
      حالة JSONB_TEXTRAW:  sqlite3_str_appendall(p_بهر,"textraw"); اكسر;
      حالة JSONB_ARRAY: {
        sqlite3_str_appendf(p_بهر,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_پیل+n, i_پیل+n+sz, nIndent+2, p_بهر);
        showContent = 0;
        اكسر;
      }
      حالة JSONB_OBJECT: {
        sqlite3_str_appendf(p_بهر, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_پیل+n, i_پیل+n+sz, nIndent+2, p_بهر);
        showContent = 0;
        اكسر;
      }
      افتراضي: {
        sqlite3_str_appendall(p_بهر, "ERROR: unknown node type\n");
        showContent = 0;
        اكسر;
      }
    }
    إذا( showContent ){
      إذا( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(p_بهر, "\n", 1);
      }وإلا{
        u32 j;
        sqlite3_str_appendall(p_بهر, ": \"");
        لكل(j=i_پیل+n; j<i_پیل+n+sz; j++){
          u8 c = pParse->aBlob[j];
          إذا( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(p_بهر, (حرف*)&c, 1);
        }
        sqlite3_str_append(p_بهر, "\"\n", 2);
      }
    }
    i_پیل += n + sz;
  }
}
ساكن فراغ jsonShowParse(JsonParse *pParse){
  sqlite3_str بهر;
  حرف zBuf[1000];
  إذا( pParse==0 ){
    چاپ_کړه("NULL pointer\n");
    أرجع;
  }وإلا{
    چاپ_کړه("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    چاپ_کړه("nBlob = %u\n", pParse->nBlob);
    چاپ_کړه("delta = %d\n", pParse->delta);
    إذا( pParse->nBlob==0 ) أرجع;
    چاپ_کړه("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&بهر, 0, zBuf, حجم(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &بهر);
  چاپ_کړه("%s", sqlite3_str_ارزښت(&بهر));
  sqlite3_str_reset(&بهر);
}
#د_که_پای /* SQLITE_DEBUG */

#که_تعریف_شوی SQLITE_DEBUG
/*
** SQL function:   json_parse(JSON)
**
** Parse JSON using jsonParseFuncArg().  Return text that is a
** human-readable dump of the binary JSONB for the input parameter.
*/
ساكن فراغ jsonParseFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str بهر;

  أكد( argc>=1 );
  sqlite3StrAccumInit(&بهر, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( p==0 ) أرجع;
  إذا( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &بهر);
    sqlite3_پایله_text64(ctx,بهر.z_متن,بهر.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }وإلا{
    jsonShowParse(p);
  }
  jsonParseFree(p);
  sqlite3_str_reset(&بهر);
}
#د_که_پای /* SQLITE_DEBUG */

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
  sqlite3_ارزښت **argv
){
  JsonString jx;
  ناکارول_شوی_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_ارزښت(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
ساكن فراغ jsonArrayFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  صحيح i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  لكل(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_ارزښت(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
ساكن فراغ json_Array_اوږدتیا_Func(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( p==0 ) أرجع;
  إذا( argc==2 ){
    ثابت حرف *z_لار = (ثابت حرف*)sqlite3_ارزښت_متن(argv[1]);
    إذا( z_لار==0 ){
      jsonParseFree(p);
      أرجع;
    }
    i = json_Lookup_ګام(p, 0, z_لار[0]=='$' ? z_لار+1 : "@", 0);
    إذا( JSON_LOOKUP_ISERROR(i) ){
      إذا( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }وإلا{
        json_Bad_لار_Error(ctx, z_لار, i);
      }
      eErr = 1;
      i = 0;
    }
  }وإلا{
    i = 0;
  }
  إذا( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_شمېر(p, i);
  }
  إذا( !eErr ) sqlite3_پایله_int64(ctx, cnt);
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
ساكن فراغ json_ایستل_Func(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  JsonParse *p = 0;      /* The parse */
  صحيح flags;             /* Flags associated with the function */
  صحيح i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  إذا( argc<2 ) أرجع;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( p==0 ) أرجع;
  flags = SQLITE_PTR_منزل_INT(sqlite3_user_معلومات(ctx));
  jsonStringInit(&jx, ctx);
  إذا( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  لكل(i=1; i<argc; i++){
    /* With a single PATH argument */
    ثابت حرف *z_لار = (ثابت حرف*)sqlite3_ارزښت_متن(argv[i]);
    صحيح n_لار;
    u32 j;
    إذا( z_لار==0 ) اذهب json_ایستل_error;
    n_لار = sqlite3Strlen30(z_لار);
    إذا( z_لار[0]=='$' ){
      j = json_Lookup_ګام(p, 0, z_لار+1, 0);
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
      إذا( sqlite3_ارزښت_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        إذا( z_لار[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, z_لار, n_لار);
        jsonAppendRawNZ(&jx, "]", 2);
      }وإلا إذا( jsonAllAlphanum(z_لار, n_لار) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, z_لار, n_لار);
      }وإلا إذا( z_لار[0]=='[' && n_لار>=3 && z_لار[n_لار-1]==']' ){
        jsonAppendRaw(&jx, z_لار, n_لار);
      }وإلا{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, z_لار, n_لار);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = json_Lookup_ګام(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }وإلا{
      json_Bad_لار_Error(ctx, z_لار, 0);
      اذهب json_ایستل_error;
    }
    إذا( j<p->nBlob ){
      إذا( argc==2 ){
        إذا( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          json_Translate_Blob_منزل_متن(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          أكد( (flags & JSON_BLOB)==0 );
          sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
        }وإلا{
          jsonReturnFromBlob(p, j, ctx, 0);
          إذا( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }وإلا{
        jsonAppendSeparator(&jx);
        json_Translate_Blob_منزل_متن(p, j, &jx);
      }
    }وإلا إذا( j==JSON_LOOKUP_NOTFOUND ){
      إذا( argc==2 ){
        اذهب json_ایستل_error;  /* Return NULL if not found */
      }وإلا{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }وإلا{
      json_Bad_لار_Error(ctx, z_لار, j);
      اذهب json_ایستل_error;
    }
  }
  إذا( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    إذا( (flags & JSON_BLOB)==0 ){
      sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_ایستل_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  أرجع;
}

/*
** Return codes for jsonMergePatch()
*/
#تعریف_کړه JSON_یوځای_OK          0     /* Success */
#تعریف_کړه JSON_یوځای_BADTARGET   1     /* Malformed TARGET blob */
#تعریف_کړه JSON_یوځای_BADPATCH    2     /* Malformed PATCH blob */
#تعریف_کړه JSON_یوځای_OOM         3     /* Out-of-memory condition */
#تعریف_کړه JSON_یوځای_TOODEEP     4     /* Nested too deep */

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
ساكن صحيح json_یوځای_Patch(
  JsonParse *p_موخه,      /* The JSON parser that contains the TARGET */
  u32 i_موخه,             /* Index of TARGET in pTarget->aBlob[] */
  ثابت JsonParse *pPatch, /* The PATCH */
  u32 iPatch,              /* Index of PATCH in pPatch->aBlob[] */
  u32 i_ژوروالی               /* Nesting depth */
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

  أكد( i_موخه>=0 && i_موخه<p_موخه->nBlob );
  أكد( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  إذا( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 sz_موخه;       /* Total size of the target, header+payload */
    n = jsonb_Payload_اندازه(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonb_Payload_اندازه(p_موخه, i_موخه, &sz);
    sz_موخه = n+sz;
    jsonBlobEdit(p_موخه, i_موخه, sz_موخه, pPatch->aBlob+iPatch, szPatch);
    أرجع p_موخه->oom ? JSON_یوځای_OOM : JSON_یوځای_OK;  /* Line 03 */
  }
  x = p_موخه->aBlob[i_موخه] & 0x0f;
  إذا( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonb_Payload_اندازه(p_موخه, i_موخه, &sz);
    jsonBlobEdit(p_موخه, i_موخه+n, sz, 0, 0);
    x = p_موخه->aBlob[i_موخه];
    p_موخه->aBlob[i_موخه] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonb_Payload_اندازه(pPatch, iPatch, &sz);
  إذا( NEVER(n==0) ) أرجع JSON_یوځای_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonb_Payload_اندازه(p_موخه, i_موخه, &sz);
  إذا( NEVER(n==0) ) أرجع JSON_یوځای_BADTARGET;
  iTStart = i_موخه+n;
  iTEndBE = iTStart+sz;

  طالما( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    إذا( ePLabel<JSONB_متن || ePLabel>JSONB_TEXTRAW ){
      أرجع JSON_یوځای_BADPATCH;
    }
    nPLabel = jsonb_Payload_اندازه(pPatch, iPCursor, &szPLabel);
    إذا( nPLabel==0 ) أرجع JSON_یوځای_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    إذا( iPValue>=iPEnd ) أرجع JSON_یوځای_BADPATCH;
    nPValue = jsonb_Payload_اندازه(pPatch, iPValue, &szPValue);
    إذا( nPValue==0 ) أرجع JSON_یوځای_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    إذا( iPCursor>iPEnd ) أرجع JSON_یوځای_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + p_موخه->delta;
    طالما( iTCursor<iTEnd ){
      صحيح isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = p_موخه->aBlob[iTCursor] & 0x0f;
      إذا( eTLabel<JSONB_متن || eTLabel>JSONB_TEXTRAW ){
        أرجع JSON_یوځای_BADTARGET;
      }
      nTLabel = jsonb_Payload_اندازه(p_موخه, iTCursor, &szTLabel);
      إذا( nTLabel==0 ) أرجع JSON_یوځای_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      إذا( iTValue>=iTEnd ) أرجع JSON_یوځای_BADTARGET;
      nTValue = jsonb_Payload_اندازه(p_موخه, iTValue, &szTValue);
      إذا( nTValue==0 ) أرجع JSON_یوځای_BADTARGET;
      إذا( iTValue + nTValue + szTValue > iTEnd ) أرجع JSON_یوځای_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ثابت حرف*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_متن || ePLabel==JSONB_TEXTRAW),
                   (ثابت حرف*)&p_موخه->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_متن || eTLabel==JSONB_TEXTRAW));
      إذا( isEqual ) اكسر;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    إذا( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      إذا( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(p_موخه, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        إذا( NEVER(p_موخه->oom) ) أرجع JSON_یوځای_OOM;
      }وإلا{
        /* Algorithm line 12 */
        صحيح rc, savedDelta = p_موخه->delta;
        p_موخه->delta = 0;
        إذا( i_ژوروالی>=JSON_MAX_ژوروالی ) أرجع JSON_یوځای_TOODEEP;
        rc = json_یوځای_Patch(p_موخه, iTValue, pPatch, iPValue, i_ژوروالی+1);
        إذا( rc ) أرجع rc;
        p_موخه->delta += savedDelta;
      }        
    }وإلا إذا( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      إذا( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(p_موخه, iTEnd, 0, 0, szPValue+nPValue+szNew);
        إذا( p_موخه->oom ) أرجع JSON_یوځای_OOM;
        حافظه_نقل(&p_موخه->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        حافظه_نقل(&p_موخه->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }وإلا{
        صحيح rc, savedDelta;
        jsonBlobEdit(p_موخه, iTEnd, 0, 0, szNew+1);
        إذا( p_موخه->oom ) أرجع JSON_یوځای_OOM;
        حافظه_نقل(&p_موخه->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        p_موخه->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = p_موخه->delta;
        p_موخه->delta = 0;
        إذا( i_ژوروالی>=JSON_MAX_ژوروالی ) أرجع JSON_یوځای_TOODEEP;
        rc = json_یوځای_Patch(p_موخه, iTEnd+szNew,pPatch,iPValue,i_ژوروالی+1);
        إذا( rc ) أرجع rc;
        p_موخه->delta += savedDelta;
      }
    }
  }
  إذا( p_موخه->delta ) json_After_Edit_اندازه_Adjust(p_موخه, i_موخه);
  أرجع p_موخه->oom ? JSON_یوځای_OOM : JSON_یوځای_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
ساكن فراغ jsonPatchFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  JsonParse *p_موخه;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  صحيح rc;                /* Result code */

  ناکارول_شوی_PARAMETER(argc);
  أكد( argc==2 );
  p_موخه = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  إذا( p_موخه==0 ) أرجع;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  إذا( pPatch ){
    rc = json_یوځای_Patch(p_موخه, 0, pPatch, 0, 0);
    إذا( rc==JSON_یوځای_OK ){
      jsonReturnParse(ctx, p_موخه);
    }وإلا إذا( rc==JSON_یوځای_OOM ){
      sqlite3_پایله_error_nomem(ctx);
    }وإلا إذا( rc==JSON_یوځای_TOODEEP ){
      sqlite3_پایله_error(ctx, "JSON nested too deep", -1);
    }وإلا{
      sqlite3_پایله_error(ctx, "malformed JSON", -1);
    }
    jsonParseFree(pPatch);
  }
  jsonParseFree(p_موخه);
}


/*
** Implementation of the json_object(NAME,VALUE,...) function.  Return a JSON
** object that contains all name/value given in arguments.  Or if any name
** is not a string or if any value is a BLOB, throw an error.
*/
ساكن فراغ jsonObjectFunc(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  صحيح i;
  JsonString jx;
  ثابت حرف *z;
  u32 n;

  إذا( argc&1 ){
    sqlite3_پایله_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    أرجع;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  لكل(i=0; i<argc; i+=2){
    إذا( sqlite3_ارزښت_type(argv[i])!=SQLITE_متن ){
      sqlite3_پایله_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      أرجع;
    }
    jsonAppendSeparator(&jx);
    z = (ثابت حرف*)sqlite3_ارزښت_متن(argv[i]);
    n = sqlite3_ارزښت_بایټونه(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_ارزښت(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
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
  sqlite3_ارزښت **argv
){
  JsonParse *p;          /* The parse */
  ثابت حرف *z_لار = 0; /* Path of element to be removed */
  صحيح i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  إذا( argc<1 ) أرجع;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  إذا( p==0 ) أرجع;
  لكل(i=1; i<argc; i++){
    z_لار = (ثابت حرف*)sqlite3_ارزښت_متن(argv[i]);
    إذا( z_لار==0 ){
      اذهب json_remove_بشپړ;
    }
    إذا( z_لار[0]!='$' ){
      اذهب json_remove_patherror;
    }
    إذا( z_لار[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      اذهب json_remove_بشپړ;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = json_Lookup_ګام(p, 0, z_لار+1, 0);
    إذا( JSON_LOOKUP_ISERROR(rc) ){
      إذا( rc==JSON_LOOKUP_NOTFOUND ){
        استمر;  /* No-op */
      }وإلا{
        json_Bad_لار_Error(ctx, z_لار, rc);
      }
      اذهب json_remove_بشپړ;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  أرجع;

json_remove_patherror:
  json_Bad_لار_Error(ctx, z_لار, 0);

json_remove_بشپړ:
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
  sqlite3_ارزښت **argv
){
  إذا( argc<1 ) أرجع;
  إذا( (argc&1)==0 ) {
    json_Wrong_عدد_دلیلونه(ctx, "replace");
    أرجع;
  }
  json_ورننباسه_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
ساكن فراغ json_سېټ_Func(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  صحيح flags = SQLITE_PTR_منزل_INT(sqlite3_user_معلومات(ctx));
  صحيح eInsType = JSON_ورننباسه_TYPE(flags);
  ساكن ثابت حرف *azInsType[] = { "insert", "set", "array_insert" };
  ساكن ثابت u8 aEditType[] = { JEDIT_INS, JEDIT_سېټ, JEDIT_AINS };

  إذا( argc<1 ) أرجع;
  أكد( eInsType>=0 && eInsType<=2 );
  إذا( (argc&1)==0 ) {
    json_Wrong_عدد_دلیلونه(ctx, azInsType[eInsType]);
    أرجع;
  }
  json_ورننباسه_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
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
  sqlite3_ارزښت **argv
){
  JsonParse *p;          /* The parse */
  ثابت حرف *z_لار = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( p==0 ) أرجع;
  إذا( argc==2 ){
    z_لار = (ثابت حرف*)sqlite3_ارزښت_متن(argv[1]);
    إذا( z_لار==0 ) اذهب json_type_بشپړ;
    إذا( z_لار[0]!='$' ){
      json_Bad_لار_Error(ctx, z_لار, 0);
      اذهب json_type_بشپړ;
    }
    i = json_Lookup_ګام(p, 0, z_لار+1, 0);
    إذا( JSON_LOOKUP_ISERROR(i) ){
      إذا( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }وإلا{
        json_Bad_لار_Error(ctx, z_لار, i);
      }
      اذهب json_type_بشپړ;
    }
  }وإلا{
    i = 0;
  }
  sqlite3_پایله_متن(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
json_type_بشپړ:
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
  sqlite3_ارزښت **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, حجم(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  إذا( x.pParse==0 ) أرجع;
  x.p_بهر = &s;
  jsonStringInit(&s, ctx);
  إذا( argc==1 || (x.zIndent = (ثابت حرف*)sqlite3_ارزښت_متن(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }وإلا{
    x.szIndent = (u32)مزي_اوږدوالی(x.zIndent);
  }
  json_Translate_Blob_منزل_Pretty_متن(&x, 0);
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
  sqlite3_ارزښت **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  إذا( argc==2 ){
    i64 f = sqlite3_ارزښت_int64(argv[1]);
    إذا( f<1 || f>15 ){
      sqlite3_پایله_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      أرجع;
    }
    flags = f & 0x0f;
  }
  بدل( sqlite3_ارزښت_type(argv[0]) ){
    حالة SQLITE_NULL: {
#که_تعریف_شوی SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_پایله_int(ctx, 0);
#د_که_پای
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
          sqlite3_پایله_error_nomem(ctx);
        }وإلا إذا( p->nErr ){
          /* no-op */
        }وإلا إذا( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }وإلا{
        sqlite3_پایله_error_nomem(ctx);
      }
      اكسر;
    }
  }
  sqlite3_پایله_int(ctx, res);
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
  sqlite3_ارزښت **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  أكد( argc==1 );
  ناکارول_شوی_PARAMETER(argc);
  memset(&s, 0, حجم(s));
  s.db = sqlite3_context_db_handle(ctx);
  إذا( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }وإلا{
    s.zJson = (حرف*)sqlite3_ارزښت_متن(argv[0]);
    إذا( s.zJson==0 ) أرجع;  /* NULL input or OOM */
    s.nJson = sqlite3_ارزښت_بایټونه(argv[0]);
    إذا( json_Convert_متن_منزل_Blob(&s,0) ){
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
    sqlite3_پایله_error_nomem(ctx);
  }وإلا{
    sqlite3_پایله_int64(ctx, iErrPos);
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
ساكن فراغ json_Array_ګام(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  JsonString *pStr;
  ناکارول_شوی_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, حجم(*pStr));
  إذا( pStr ){
    إذا( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }وإلا إذا( pStr->n_کارول_شوی>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_ارزښت(pStr, argv[0]);
  }
}
ساكن فراغ jsonArrayCompute(sqlite3_context *ctx, صحيح isFinal){
  JsonString *pStr;
  صحيح flags = SQLITE_PTR_منزل_INT(sqlite3_user_معلومات(ctx));
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
      sqlite3_پایله_متن(ctx, pStr->zBuf, (صحيح)pStr->n_کارول_شوی,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }وإلا{
      sqlite3_پایله_متن(ctx, pStr->zBuf, (صحيح)pStr->n_کارول_شوی, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }وإلا إذا( flags & JSON_BLOB ){
    ساكن ثابت u8 تشه_Array = 0x0b;
    sqlite3_پایله_blob(ctx, &تشه_Array, 1, SQLITE_STATIC);
  }وإلا{
    sqlite3_پایله_متن(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
}
ساكن فراغ json_Array_ارزښت(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
ساكن فراغ jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
ساكن فراغ json_ډله_Inverse(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  بینښه صحيح i;
  صحيح دننه_Str = 0;
  صحيح nNest = 0;
  حرف *z;
  حرف c;
  JsonString *pStr;
  ناکارول_شوی_PARAMETER(argc);
  ناکارول_شوی_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  إذا( NEVER(!pStr) ) أرجع;
  z = pStr->zBuf;
  لكل(i=1; i<pStr->n_کارول_شوی && ((c = z[i])!=',' || دننه_Str || nNest); i++){
    إذا( c=='"' ){
      دننه_Str = !دننه_Str;
    }وإلا إذا( c=='\\' ){
      i++;
    }وإلا إذا( !دننه_Str ){
      إذا( c=='{' || c=='[' ) nNest++;
      إذا( c=='}' || c==']' ) nNest--;
    }
  }
  إذا( i<pStr->n_کارول_شوی ){
    pStr->n_کارول_شوی -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->n_کارول_شوی-1);
    z[pStr->n_کارول_شوی] = 0;
  }وإلا{
    pStr->n_کارول_شوی = 1;
  }
}
#وإلا
# define json_ډله_Inverse 0
#د_که_پای


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
ساكن فراغ json_Object_ګام(
  sqlite3_context *ctx,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  JsonString *pStr;
  ثابت حرف *z;
  u32 n;
  ناکارول_شوی_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, حجم(*pStr));
  إذا( pStr ){
    z = (ثابت حرف*)sqlite3_ارزښت_متن(argv[0]);
    n = sqlite3Strlen30(z);
    إذا( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }وإلا إذا( pStr->n_کارول_شوی>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    إذا( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_ارزښت(pStr, argv[1]);
    }وإلا{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
ساكن فراغ jsonObjectCompute(sqlite3_context *ctx, صحيح isFinal){
  JsonString *pStr;
  صحيح flags = SQLITE_PTR_منزل_INT(sqlite3_user_معلومات(ctx));
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
      صحيح دننه_Str = 0;
      إذا( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->n_کارول_شوی+1);
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
      لكل(i=j=1; i<pStr->n_کارول_شوی; i++){
        حرف c = pStr->zBuf[i];
        إذا( c=='"' ){
          دننه_Str = !دننه_Str;
          pStr->zBuf[j++] = '"';
        }وإلا إذا( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }وإلا إذا( c=='@' && !دننه_Str ){
          أكد( i+1<pStr->n_کارول_شوی );
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
      pStr->n_کارول_شوی = j;    /* Truncate the string */
    }
    إذا( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      إذا( isFinal ){
        إذا( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }وإلا{
        jsonStringTrimOneChar(pOgStr);
      }
    }وإلا إذا( isFinal ){
      sqlite3_پایله_متن(ctx, pStr->zBuf, (صحيح)pStr->n_کارول_شوی,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }وإلا{
      sqlite3_پایله_متن(ctx, pStr->zBuf, (صحيح)pStr->n_کارول_شوی, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    إذا( pStr!=pOgStr ) jsonStringReset(pStr);
  }وإلا إذا( flags & JSON_BLOB ){
    ساكن ثابت بینښه حرف تشه_Object = 0x0c;
    sqlite3_پایله_blob(ctx, &تشه_Object, 1, SQLITE_STATIC); 
  }وإلا{
    sqlite3_پایله_متن(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
}
ساكن فراغ json_Object_ارزښت(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
ساكن فراغ jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
تعريفنوع بنية Json_پلار Json_پلار;
بنية Json_پلار {
  u32 i_سر;                 /* Start of object or array */
  u32 i_ارزښت;                /* Start of the value */
  u32 i_پای_ټکی;                  /* First byte past the end */
  u32 n_لار;                 /* Length of path */
  i64 i_کلی;                  /* Key for JSONB_ARRAY */
};

تعريفنوع بنية JsonEachCursor JsonEachCursor;
بنية JsonEachCursor {
  sqlite3_vtab_cursor بنسټ;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 i_پای_ټکی;                  /* EOF when i equals or exceeds this value */
  u32 n_ریښه;                 /* Size of the root path in bytes */
  u8 eType;                  /* Type of the container for element i */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
  u8 e_ډېرپېښ;                  /* 1 for json_each().  2 for jsonb_each() */
  u32 n_پلار;               /* Current nesting depth */
  u32 n_پلار_Alloc;          /* Space allocated for aParent[] */
  Json_پلار *a_پلار;       /* Parent elements of i */
  sqlite3 *db;               /* Database connection */
  JsonString لار;           /* Current path */
  JsonParse sParse;          /* Parse of the input JSON */
};
تعريفنوع بنية JsonEachConnection JsonEachConnection;
بنية JsonEachConnection {
  sqlite3_vtab بنسټ;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 e_ډېرپېښ;                  /* 1 for json_each().  2 for jsonb_each() */
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
#تعریف_کړه JEACH_کلی     0
#تعریف_کړه JEACH_ارزښت   1
#تعریف_کړه JEACH_TYPE    2
#تعریف_کړه JEACH_ATOM    3
#تعریف_کړه JEACH_ID      4
#تعریف_کړه JEACH_پلار  5
#تعریف_کړه JEACH_FULLKEY 6
#تعریف_کړه JEACH_لار    7
/* The xBestIndex method assumes that the JSON and ROOT columns are
** the last two columns in the table.  Should this ever changes, be
** sure to update the xBestIndex method. */
#تعریف_کړه JEACH_JSON    8
#تعریف_کړه JEACH_ریښه    9

  ناکارول_شوی_PARAMETER(pzErr);
  ناکارول_شوی_PARAMETER(argv);
  ناکارول_شوی_PARAMETER(argc);
  ناکارول_شوی_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  إذا( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, حجم(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    إذا( pNew==0 ) أرجع SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->e_ډېرپېښ = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->e_ډېرپېښ]=='t';
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

  ناکارول_شوی_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, حجم(*pCur));
  إذا( pCur==0 ) أرجع SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->e_ډېرپېښ = pVtab->e_ډېرپېښ;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->لار);
  *ppCursor = &pCur->بنسټ;
  أرجع SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
ساكن فراغ jsonEachCursorReset(JsonEachCursor *p){
  jsonParseReset(&p->sParse);
  jsonStringReset(&p->لار);
  sqlite3DbFree(p->db, p->a_پلار);
  p->iRowid = 0;
  p->i = 0;
  p->a_پلار = 0;
  p->n_پلار = 0;
  p->n_پلار_Alloc = 0;
  p->i_پای_ټکی = 0;
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
  أرجع p->i >= p->i_پای_ټکی;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
ساكن صحيح jsonSkipLabel(JsonEachCursor *p){
  إذا( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonb_Payload_اندازه(&p->sParse, p->i, &sz);
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
ساكن فراغ json_Append_لار_Name(JsonEachCursor *p){
  أكد( p->n_پلار>0 );
  أكد( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  إذا( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->لار, "[%lld]", p->a_پلار[p->n_پلار-1].i_کلی);
  }وإلا{
    u32 n, sz = 0, k, i;
    ثابت حرف *z;
    صحيح needQuote = 0;
    n = jsonb_Payload_اندازه(&p->sParse, p->i, &sz);
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
      jsonPrintf(sz+4,&p->لار,".\"%.*s\"", sz, z);
    }وإلا{
      jsonPrintf(sz+2,&p->لار,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
ساكن صحيح json_Each_Malformed_ننوت(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_پیغام);
  cur->pVtab->z_Err_پیغام = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  أرجع cur->pVtab->z_Err_پیغام ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
ساكن صحيح json_Each_بل(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  صحيح rc = SQLITE_OK;
  إذا( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonb_Payload_اندازه(&p->sParse, i, &sz);
    إذا( n==0 )أرجع json_Each_Malformed_ننوت(cur);
    إذا( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      Json_پلار *p_پلار;
      إذا( p->n_پلار>=p->n_پلار_Alloc ){
        Json_پلار *pNew;
        u64 nNew;
        nNew = p->n_پلار_Alloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->a_پلار, حجم(Json_پلار)*nNew);
        إذا( pNew==0 ) أرجع SQLITE_NOMEM;
        p->n_پلار_Alloc = (u32)nNew;
        p->a_پلار = pNew;
      }
      levelChange = 1;
      p_پلار = &p->a_پلار[p->n_پلار];
      p_پلار->i_سر = p->i;
      p_پلار->i_ارزښت = i;
      p_پلار->i_پای_ټکی = i + n + sz;
      p_پلار->i_کلی = -1;
      p_پلار->n_لار = (u32)p->لار.n_کارول_شوی;
      إذا( p->eType && p->n_پلار ){
        json_Append_لار_Name(p);
        إذا( p->لار.eErr ) rc = SQLITE_NOMEM;
      }
      p->n_پلار++;
      p->i = i + n;
    }وإلا{
      p->i = i + n + sz;
    }
    طالما( p->n_پلار>0 && p->i >= p->a_پلار[p->n_پلار-1].i_پای_ټکی ){
      p->n_پلار--;
      p->لار.n_کارول_شوی = p->a_پلار[p->n_پلار].n_لار;
      levelChange = 1;
    }
    إذا( levelChange ){
      إذا( p->n_پلار>0 ){
        Json_پلار *p_پلار = &p->a_پلار[p->n_پلار-1];
        u32 iVal = p_پلار->i_ارزښت;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }وإلا{
        p->eType = 0;
      }
    }
  }وإلا{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonb_Payload_اندازه(&p->sParse, i, &sz);
    إذا( n==0 )أرجع json_Each_Malformed_ننوت(cur);
    p->i = i + n + sz;
  }
  إذا( p->eType==JSONB_ARRAY && p->n_پلار ){
    p->a_پلار[p->n_پلار-1].i_کلی++;
  }
  p->iRowid++;
  أرجع rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
ساكن صحيح json_Each_لار_اوږدتیا(JsonEachCursor *p){
  u32 n = p->لار.n_کارول_شوی;
  حرف *z = p->لار.zBuf;
  إذا( p->iRowid==0 && p->bRecursive && n>=2 ){
    طالما( ALWAYS(n>1) ){
      n--;
      إذا( z[n]=='[' || z[n]=='.' ){
        u32 x;
        حرف cSaved = z[n];
        z[n] = 0;
        أكد( p->sParse.eEdit==0 );
        x = json_Lookup_ګام(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        إذا( !JSON_LOOKUP_ISERROR(x) ) اكسر;
      }
    }
  }
  أرجع n;
}

/* Return the value of a column */
ساكن صحيح json_Each_ستنه(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحيح i_ستنه                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  بدل( i_ستنه ){
    حالة JEACH_کلی: {
      إذا( p->n_پلار==0 ){
        u32 n, j;
        إذا( p->n_ریښه==1 ) اكسر;
        j = json_Each_لار_اوږدتیا(p);
        n = p->n_ریښه - j;
        إذا( n==0 ){
          اكسر;
        }وإلا إذا( p->لار.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->لار.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_پایله_int64(ctx, x);
        }وإلا إذا( p->لار.zBuf[j+1]=='"' ){
          sqlite3_پایله_متن(ctx, &p->لار.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }وإلا{
          sqlite3_پایله_متن(ctx, &p->لار.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        اكسر;
      }
      إذا( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }وإلا{
        أكد( p->eType==JSONB_ARRAY );
        sqlite3_پایله_int64(ctx, p->a_پلار[p->n_پلار-1].i_کلی);
      }
      اكسر;
    }
    حالة JEACH_ارزښت: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->e_ډېرپېښ);
      إذا( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_پایله_subtype(ctx, JSON_SUBTYPE);
      }
      اكسر;
    }
    حالة JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_پایله_متن(ctx, jsonbType[eType], -1, SQLITE_STATIC);
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
      sqlite3_پایله_int64(ctx, (sqlite3_int64)p->i);
      اكسر;
    }
    حالة JEACH_پلار: {
      إذا( p->n_پلار>0 && p->bRecursive ){
        sqlite3_پایله_int64(ctx, p->a_پلار[p->n_پلار-1].i_سر);
      }
      اكسر;
    }
    حالة JEACH_FULLKEY: {
      u64 n_بنسټ = p->لار.n_کارول_شوی;
      إذا( p->n_پلار ) json_Append_لار_Name(p);
      sqlite3_پایله_text64(ctx, p->لار.zBuf, p->لار.n_کارول_شوی,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->لار.n_کارول_شوی = n_بنسټ;
      اكسر;
    }
    حالة JEACH_لار: {
      u32 n = json_Each_لار_اوږدتیا(p);
      sqlite3_پایله_text64(ctx, p->لار.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      اكسر;
    }
    افتراضي: {
      sqlite3_پایله_متن(ctx, p->لار.zBuf, p->n_ریښه, SQLITE_TRANSIENT);
      اكسر;
    }
    حالة JEACH_JSON: {
      إذا( p->sParse.zJson==0 ){
        sqlite3_پایله_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }وإلا{
        sqlite3_پایله_متن(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
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
ساكن صحيح json_Each_غوره_شاخص(
  sqlite3_vtab *tab,
  sqlite3_شاخص_info *pIdxInfo
){
  صحيح i;                     /* Loop counter or computed array index */
  صحيح aIdx[2];               /* Index of constraints for JSON and ROOT */
  صحيح unusable_ماسک = 0;      /* Mask of unusable JSON and ROOT constraints */
  صحيح idx_ماسک = 0;           /* Mask of usable == constraints JSON and ROOT */
  ثابت بنية sqlite3_شاخص_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  أكد( JEACH_ریښه == JEACH_JSON+1 );
  ناکارول_شوی_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  لكل(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    صحيح i_ستون;
    صحيح i_ماسک;
    إذا( pConstraint->i_ستنه < JEACH_JSON ) استمر;
    i_ستون = pConstraint->i_ستنه - JEACH_JSON;
    أكد( i_ستون==0 || i_ستون==1 );
    testcase( i_ستون==0 );
    i_ماسک = 1 << i_ستون;
    إذا( pConstraint->usable==0 ){
      unusable_ماسک |= i_ماسک;
    }وإلا إذا( pConstraint->op==SQLITE_شاخص_CONSTRAINT_EQ ){
      aIdx[i_ستون] = i;
      idx_ماسک |= i_ماسک;
    }
  }
  إذا( pIdxInfo->n_ترتیب_By>0
   && pIdxInfo->a_ترتیب_By[0].i_ستنه<0
   && pIdxInfo->a_ترتیب_By[0].desc==0
  ){
    pIdxInfo->ترتیب_By_Consumed = 1;
  }

  إذا( (unusable_ماسک & ~idx_ماسک)!=0 ){
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
    pIdxInfo->estimated_لګښت = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argv_شاخص = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    إذا( aIdx[1]<0 ){
      pIdxInfo->idx_عدد = 1;  /* Only JSON supplied.  Plan 1 */
    }وإلا{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argv_شاخص = 2;
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
  صحيح argc, sqlite3_ارزښت **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ثابت حرف *z_ریښه = 0;
  u32 i, n, sz;

  ناکارول_شوی_PARAMETER(idxStr);
  ناکارول_شوی_PARAMETER(argc);
  jsonEachCursorReset(p);
  إذا( idx_عدد==0 ) أرجع SQLITE_OK;
  memset(&p->sParse, 0, حجم(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  إذا( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }وإلا{
    p->sParse.zJson = (حرف*)sqlite3_ارزښت_متن(argv[0]);
    p->sParse.nJson = sqlite3_ارزښت_بایټونه(argv[0]);
    إذا( p->sParse.zJson==0 ){
      p->i = p->i_پای_ټکی = 0;
      أرجع SQLITE_OK;
    }      
    إذا( json_Convert_متن_منزل_Blob(&p->sParse, 0) ){
      إذا( p->sParse.oom ){
        أرجع SQLITE_NOMEM;
      }
      أرجع json_Each_Malformed_ننوت(cur);
    }
  }
  إذا( idx_عدد==3 ){
    z_ریښه = (ثابت حرف*)sqlite3_ارزښت_متن(argv[1]);
    إذا( z_ریښه==0 ) أرجع SQLITE_OK;
    إذا( z_ریښه[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_پیغام);
      cur->pVtab->z_Err_پیغام = json_Bad_لار_Error(0, z_ریښه, 0);
      jsonEachCursorReset(p);
      أرجع cur->pVtab->z_Err_پیغام ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_ریښه = sqlite3Strlen30(z_ریښه);
    إذا( z_ریښه[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }وإلا{
      i = json_Lookup_ګام(&p->sParse, 0, z_ریښه+1, 0);
      إذا( JSON_LOOKUP_ISERROR(i) ){
        إذا( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->i_پای_ټکی = 0;
          أرجع SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_پیغام);
        cur->pVtab->z_Err_پیغام = json_Bad_لار_Error(0, z_ریښه, 0);
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
    jsonAppendRaw(&p->لار, z_ریښه, p->n_ریښه);
  }وإلا{
    i = p->i = 0;
    p->eType = 0;
    p->n_ریښه = 1;
    jsonAppendRaw(&p->لار, "$", 1);
  }
  p->n_پلار = 0;
  n = jsonb_Payload_اندازه(&p->sParse, i, &sz);
  p->i_پای_ټکی = i+n+sz;
  إذا( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->a_پلار = sqlite3DbMallocZero(p->db, حجم(Json_پلار));
    إذا( p->a_پلار==0 ) أرجع SQLITE_NOMEM;
    p->n_پلار = 1;
    p->n_پلار_Alloc = 1;
    p->a_پلار[0].i_کلی = 0;
    p->a_پلار[0].i_پای_ټکی = p->i_پای_ټکی;
    p->a_پلار[0].i_سر = p->i;
    p->a_پلار[0].i_ارزښت = i;
  }
  أرجع SQLITE_OK;
}

/* The methods of the json_each virtual table */
ساكن sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  json_Each_غوره_شاخص,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  json_Each_بل,              /* xNext - advance a cursor */
  jsonEachEof,               /* xEof - check for end of scan */
  json_Each_ستنه,            /* xColumn - read data */
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
#د_که_پای /* SQLITE_OMIT_VIRTUALTABLE */
#د_که_پای /* !defined(SQLITE_OMIT_JSON) */

/*
** Register JSON functions.
*/
فراغ sqlite3_ثبتوونکی_Json_Functions(فراغ){
#که_نه_تعریف_شوی SQLITE_OMIT_JSON
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
    JFUNCTION(json_array_ورننباسه, -1,1,1, 1,0,JSON_AINS,  json_سېټ_Func),
    JFUNCTION(jsonb_array_ورننباسه,-1,1,0, 1,1,JSON_AINS,  json_سېټ_Func),
    JFUNCTION(json_array_اوږدتیا,  1,1,0, 0,0,0,          json_Array_اوږدتیا_Func),
    JFUNCTION(json_array_اوږدتیا,  2,1,0, 0,0,0,          json_Array_اوږدتیا_Func),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_ایستل,      -1,1,1, 0,0,0,          json_ایستل_Func),
    JFUNCTION(jsonb_ایستل,     -1,1,0, 0,1,0,          json_ایستل_Func),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  json_ایستل_Func),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   json_ایستل_Func),
    JFUNCTION(json_ورننباسه,       -1,1,1, 1,0,0,          json_سېټ_Func),
    JFUNCTION(jsonb_ورننباسه,      -1,1,0, 1,1,0,          json_سېټ_Func),
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
    JFUNCTION(json_سېټ,          -1,1,1, 1,0,JSON_ISSET, json_سېټ_Func),
    JFUNCTION(jsonb_سېټ,         -1,1,0, 1,1,JSON_ISSET, json_سېټ_Func),
    JFUNCTION(json_type,          1,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_type,          2,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_valid,         1,1,0, 0,0,0,          jsonValidFunc),
    JFUNCTION(json_valid,         2,1,0, 0,0,0,          jsonValidFunc),
#إذا SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#د_که_پای
    WAGGREGATE(json_ډله_array,  1, 0, 0,
       json_Array_ګام, jsonArrayFinal, json_Array_ارزښت, json_ډله_Inverse,
       SQLITE_SUBTYPE|SQLITE_پایله_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_ډله_array, 1, JSON_BLOB, 0,
       json_Array_ګام, jsonArrayFinal, json_Array_ارزښت, json_ډله_Inverse,
       SQLITE_SUBTYPE|SQLITE_پایله_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_ډله_object, 2, 0, 0,
       json_Object_ګام, jsonObjectFinal, json_Object_ارزښت, json_ډله_Inverse,
       SQLITE_SUBTYPE|SQLITE_پایله_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_ډله_object,2, JSON_BLOB, 0,
       json_Object_ګام, jsonObjectFinal, json_Object_ارزښت, json_ډله_Inverse,
       SQLITE_SUBTYPE|SQLITE_پایله_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_ورننباسه_Builtin_Funcs(aJsonFunc, Array_اندازه(aJsonFunc));
#د_که_پای
}

#إذا  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3_Json_Vtab_ثبتوونکی(sqlite3 *db, ثابت حرف *zName){
  بینښه صحيح i;
  ساكن ثابت حرف *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  أكد( sqlite3_Hash_پیدا_کړه(&db->aModule, zName)==0 );
  لكل(i=0; i<حجم(azModule)/حجم(azModule[0]); i++){
    إذا( sqlite3StrICmp(azModule[i],zName)==0 ){
      أرجع sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  أرجع 0;
}
#د_که_پای /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
