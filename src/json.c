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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_JSON
#شامل "sqliteInt.h"

/* JSONB element types
*/
#تعریف_کرو JSONB_NULL     0   /* "null" */
#تعریف_کرو JSONB_TRUE     1   /* "true" */
#تعریف_کرو JSONB_FALSE    2   /* "false" */
#تعریف_کرو JSONB_INT      3   /* integer acceptable to JSON and SQL */
#تعریف_کرو JSONB_INT5     4   /* integer in 0x000 notation */
#تعریف_کرو JSONB_FLOAT    5   /* float acceptable to JSON and SQL */
#تعریف_کرو JSONB_FLOAT5   6   /* float with JSON5 extensions */
#تعریف_کرو JSONB_متن     7   /* Text compatible with both JSON and SQL */
#تعریف_کرو JSONB_TEXTJ    8   /* Text with JSON escapes */
#تعریف_کرو JSONB_TEXT5    9   /* Text with JSON-5 escape */
#تعریف_کرو JSONB_TEXTRAW 10   /* SQL text that needs escaping for JSON */
#تعریف_کرو JSONB_ARRAY   11   /* An array */
#تعریف_کرو JSONB_OBJECT  12   /* An object */

/* Human-readable names for the JSONB values.  The index for each
** string must correspond to the JSONB_* integer above.
*/
ساکن ثابت حرف * ثابت jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
ساکن ثابت حرف jsonIsSpace[] = {
#اگر_تعریف_شدہ SQLITE_ASCII
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
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EBCDIC
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
#اگر_ختم

};
#تعریف_کرو jsonIsspace(x) (jsonIsSpace[(غیرعلامتی حرف)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#اگر_تعریف_شدہ SQLITE_ASCII
ساکن ثابت حرف jsonSpaces[] = "\011\012\015\040";
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EBCDIC
ساکن ثابت حرف jsonSpaces[] = "\005\045\015\100";
#اگر_ختم


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
ساکن ثابت حرف jsonIsOk[256] = {
#اگر_تعریف_شدہ SQLITE_ASCII
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
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EBCDIC
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
#اگر_ختم
};

/* Objects */
قسمتعریف ساخت JsonCache JsonCache;
قسمتعریف ساخت JsonString JsonString;
قسمتعریف ساخت JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#تعریف_کرو JSON_CACHE_ID    (-429938)  /* Cache entry */
#تعریف_کرو JSON_CACHE_سائز  4          /* Max number of cache entries */

/*
** jsonUnescapeOneChar() returns this invalid code point if it encounters
** a syntax error.
*/
#تعریف_کرو JSON_INVALID_CHAR 0x99999

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
ساخت JsonCache {
  sqlite3 *db;                    /* Database connection */
  صحیح n_مستعمل;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_سائز];  /* One line for each cache entry */
};

/* An instance of this object represents a JSON string
** under construction.  Really, this is a generic string accumulator
** that can be and is used to create strings other than JSON.
**
** If the generated string is longer than will fit into the zSpace[] buffer,
** then it will be an RCStr string.  This aids with caching of large
** JSON strings.
*/
ساخت JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  حرف *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 n_مستعمل;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  حرف zSpace[100];        /* Initial static space */
};

/* Allowed values for JsonString.eErr */
#تعریف_کرو JSTRING_OOM         0x01   /* Out of memory */
#تعریف_کرو JSTRING_MALFORMED   0x02   /* Malformed JSONB */
#تعریف_کرو JSTRING_TOODEEP     0x04   /* JSON nested too deep */
#تعریف_کرو JSTRING_ERR         0x08   /* Error already sent to sqlite3_result */

/* The "subtype" set for text JSON values passed through using
** sqlite3_result_subtype() and sqlite3_value_subtype().
*/
#تعریف_کرو JSON_SUBTYPE  74    /* Ascii for "J" */

/*
** Bit values for the flags passed into various SQL function implementations
** via the sqlite3_user_data() value.
*/
#تعریف_کرو JSON_JSON      0x01        /* Result is always JSON */
#تعریف_کرو JSON_SQL       0x02        /* Result is always SQL */
#تعریف_کرو JSON_ABPATH    0x03        /* Allow abbreviated JSON path specs */
#تعریف_کرو JSON_ISSET     0x04        /* json_set(), not json_insert() */
#تعریف_کرو JSON_AINS      0x08        /* json_array_insert(), not json_insert() */
#تعریف_کرو JSON_BLOB      0x10        /* Use the BLOB output format */

#تعریف_کرو JSON_داخل_کرو_TYPE(X)  (((X)&0xC)>>2)


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
ساخت JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  حرف *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  صحیح nJson;         /* Length of the zJson string in bytes */
  u32 nJPRef;        /* Number of references to this object */
  u32 iErr;          /* Error location in zJson[] */
  u16 i_گہرائی;        /* Nesting depth */
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
#تعریف_کرو JEDIT_DEL   1   /* Delete if exists */
#تعریف_کرو JEDIT_REPL  2   /* Overwrite if exists */
#تعریف_کرو JEDIT_INS   3   /* Insert if not exists */
#تعریف_کرو JEDIT_سیٹ   4   /* Insert or overwrite */
#تعریف_کرو JEDIT_AINS  5   /* array_insert() */

/*
** Maximum nesting depth of JSON for this implementation.
**
** This limit is needed to avoid a stack overflow in the recursive
** descent parser.  A depth of 1000 is far deeper than any sane JSON
** should go.  Historical note: This limit was 2000 prior to version 3.42.0
*/
#اگر_غیر_تعریف_شدہ SQLITE_JSON_MAX_گہرائی
# define JSON_MAX_گہرائی  1000
#ورنہ
# define JSON_MAX_گہرائی SQLITE_JSON_MAX_گہرائی
#اگر_ختم

/*
** Allowed values for the flgs argument to jsonParseFuncArg();
*/
#تعریف_کرو JSON_EDITABLE  0x01   /* Generate a writable JsonParse object */
#تعریف_کرو JSON_KEEPERROR 0x02   /* Return non-NULL even if there is an error */
#تعریف_کرو JSON_NOCACHE   0x04   /* Do not use the JSON cache */

/**************************************************************************
** Forward references
**************************************************************************/
ساکن خالی jsonReturnStringAsBlob(JsonString*);
ساکن صحیح jsonArgIsJsonb(sqlite3_قدر *pJson, JsonParse *p);
ساکن u32 json_Translate_Blob_مقصد_متن(JsonParse*,u32,JsonString*);
ساکن خالی jsonReturnParse(sqlite3_context*,JsonParse*);
ساکن JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_قدر*,u32);
ساکن خالی jsonParseFree(JsonParse*);
ساکن u32 jsonb_Payload_سائز(ثابت JsonParse*, u32, u32*);
ساکن u32 jsonUnescapeOneChar(ثابت حرف*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
ساکن خالی jsonCacheDelete(JsonCache *p){
  صحیح i;
  برائے(i=0; i<p->n_مستعمل; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
ساکن خالی jsonCacheDeleteGeneric(خالی *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
ساکن صحیح json_Cache_داخل_کرو(
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
    p = sqlite3DbMallocZero(db, حجم(*p));
    اگر( p==0 ) واپس SQLITE_NOMEM;
    p->db = db;
    sqlite3_سیٹ_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    اگر( p==0 ) واپس SQLITE_NOMEM;
  }
  اگر( p->n_مستعمل >= JSON_CACHE_سائز ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_سائز-1)*حجم(p->a[0]));
    p->n_مستعمل = JSON_CACHE_سائز-1;
  }
  أكد( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->n_مستعمل] = pParse;
  p->n_مستعمل++;
  واپس SQLITE_OK;
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
ساکن JsonParse *json_Cache_تلاش(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_قدر *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  صحیح i;
  ثابت حرف *zJson;
  صحیح nJson;

  اگر( sqlite3_قدر_type(pArg)!=SQLITE_متن ){
    واپس 0;
  }
  zJson = (ثابت حرف*)sqlite3_قدر_متن(pArg);
  اگر( zJson==0 ) واپس 0;
  nJson = sqlite3_قدر_بائٹیں(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  اگر( p==0 ){
    واپس 0;
  }
  برائے(i=0; i<p->n_مستعمل; i++){
    اگر( p->a[i]->zJson==zJson ) توڑو;
  }
  اگر( i>=p->n_مستعمل ){
    برائے(i=0; i<p->n_مستعمل; i++){
      اگر( p->a[i]->nJson!=nJson ) جاری;
      اگر( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) توڑو;
    }
  }
  اگر( i<p->n_مستعمل ){
    اگر( i<p->n_مستعمل-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->n_مستعمل-i-1)*حجم(tmp));
      p->a[p->n_مستعمل-1] = tmp;
      i = p->n_مستعمل - 1;
    }
    أكد( p->a[i]->delta==0 );
    واپس p->a[i];
  }ورنہ{
    واپس 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
ساکن خالی jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = حجم(p->zSpace);
  p->n_مستعمل = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
ساکن خالی jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
ساکن خالی jsonStringReset(JsonString *p){
  اگر( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
ساکن خالی jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  اگر( p->pCtx ) sqlite3_نتیجہ_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
ساکن خالی jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  أكد( p->pCtx!=0 );
  sqlite3_نتیجہ_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
ساکن صحیح jsonStringGrow(JsonString *p, u32 N){
  u64 n_کل = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  حرف *zNew;
  اگر( p->bStatic ){
    اگر( p->eErr ) واپس 1;
    zNew = sqlite3RCStrNew(n_کل);
    اگر( zNew==0 ){
      jsonStringOom(p);
      واپس SQLITE_NOMEM;
    }
    حافظہ_نقل(zNew, p->zBuf, (size_t)p->n_مستعمل);
    p->zBuf = zNew;
    p->bStatic = 0;
  }ورنہ{
    p->zBuf = sqlite3RCStrResize(p->zBuf, n_کل);
    اگر( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      واپس SQLITE_NOMEM;
    }
  }
  p->nAlloc = n_کل;
  واپس SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
ساکن SQLITE_NOINLINE خالی jsonStringExpandAndAppend(
  JsonString *p,
  ثابت حرف *z_اندر,
  u32 N
){
  أكد( N>0 );
  اگر( jsonStringGrow(p,N) ) واپس;
  حافظہ_نقل(p->zBuf+p->n_مستعمل, z_اندر, N);
  p->n_مستعمل += N;
}
ساکن خالی jsonAppendRaw(JsonString *p, ثابت حرف *z_اندر, u32 N){
  اگر( N==0 ) واپس;
  اگر( N+p->n_مستعمل >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_اندر,N);
  }ورنہ{
    حافظہ_نقل(p->zBuf+p->n_مستعمل, z_اندر, N);
    p->n_مستعمل += N;
  }
}
ساکن خالی jsonAppendRawNZ(JsonString *p, ثابت حرف *z_اندر, u32 N){
  أكد( N>0 );
  اگر( N+p->n_مستعمل >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_اندر,N);
  }ورنہ{
    حافظہ_نقل(p->zBuf+p->n_مستعمل, z_اندر, N);
    p->n_مستعمل += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
ساکن خالی jsonPrintf(صحیح N, JsonString *p, ثابت حرف *zFormat, ...){
  va_list ap;
  اگر( (p->n_مستعمل + N >= p->nAlloc) && jsonStringGrow(p, N) ) واپس;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->n_مستعمل, zFormat, ap);
  va_end(ap);
  p->n_مستعمل += (صحیح)لڑی_لمبائی(p->zBuf+p->n_مستعمل);
}

/* Append a single character
*/
ساکن SQLITE_NOINLINE خالی jsonAppendCharExpand(JsonString *p, حرف c){
  اگر( jsonStringGrow(p,1) ) واپس;
  p->zBuf[p->n_مستعمل++] = c;
}
ساکن خالی jsonAppendChar(JsonString *p, حرف c){
  اگر( p->n_مستعمل>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }ورنہ{
    p->zBuf[p->n_مستعمل++] = c;
  }
}

/* Remove a single character from the end of the string
*/
ساکن خالی jsonStringTrimOneChar(JsonString *p){
  اگر( p->eErr==0 ){
    أكد( p->n_مستعمل>0 );
    p->n_مستعمل--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
ساکن صحیح jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  واپس p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
ساکن خالی jsonAppendSeparator(JsonString *p){
  حرف c;
  اگر( p->n_مستعمل==0 ) واپس;
  c = p->zBuf[p->n_مستعمل-1];
  اگر( c=='[' || c=='{' ) واپس;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
ساکن خالی jsonAppendControlChar(JsonString *p, u8 c){
  ساکن ثابت حرف aSpecial[] = {
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
  أكد( p->n_مستعمل+7 <= p->nAlloc );
  اگر( aSpecial[c] ){
    p->zBuf[p->n_مستعمل] = '\\';
    p->zBuf[p->n_مستعمل+1] = aSpecial[c];
    p->n_مستعمل += 2;
  }ورنہ{
    p->zBuf[p->n_مستعمل] = '\\';
    p->zBuf[p->n_مستعمل+1] = 'u';
    p->zBuf[p->n_مستعمل+2] = '0';
    p->zBuf[p->n_مستعمل+3] = '0';
    p->zBuf[p->n_مستعمل+4] = "0123456789abcdef"[c>>4];
    p->zBuf[p->n_مستعمل+5] = "0123456789abcdef"[c&0xf];
    p->n_مستعمل += 6;
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
ساکن خالی jsonAppendString(JsonString *p, ثابت حرف *z_اندر, u32 N){
  u32 k;
  u8 c;
  ثابت u8 *z = (ثابت u8*)z_اندر;
  اگر( z==0 ) واپس;
  اگر( (N+p->n_مستعمل+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) واپس;
  p->zBuf[p->n_مستعمل++] = '"';
  جب( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    جب( 1 /* Exit by break */ ){
      اگر( k+3>=N ){
        جب( k<N && jsonIsOk[z[k]] ){ k++; }
        توڑو;
      }
      اگر( !jsonIsOk[z[k]] ){
        توڑو;
      }
      اگر( !jsonIsOk[z[k+1]] ){
        k += 1;
        توڑو;
      }
      اگر( !jsonIsOk[z[k+2]] ){
        k += 2;
        توڑو;
      }
      اگر( !jsonIsOk[z[k+3]] ){
        k += 3;
        توڑو;
      }ورنہ{
        k += 4;
      }
    }
    اگر( k>=N ){
      اگر( k>0 ){
        حافظہ_نقل(&p->zBuf[p->n_مستعمل], z, k);
        p->n_مستعمل += k;
      }
      توڑو;
    }
    اگر( k>0 ){
      حافظہ_نقل(&p->zBuf[p->n_مستعمل], z, k);
      p->n_مستعمل += k;
      z += k;
      N -= k;
    }
    c = z[0];
    اگر( c=='"' || c=='\\' ){
      اگر( (p->n_مستعمل+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) واپس;
      p->zBuf[p->n_مستعمل++] = '\\';
      p->zBuf[p->n_مستعمل++] = c;
    }ورنہ اگر( c=='\'' ){
      p->zBuf[p->n_مستعمل++] = c;
    }ورنہ{
      اگر( (p->n_مستعمل+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) واپس;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->n_مستعمل++] = '"';
  أكد( p->n_مستعمل<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
ساکن خالی json_Append_Sql_قدر(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_قدر *p_قدر          /* Value to append */
){
  انتخاب( sqlite3_قدر_type(p_قدر) ){
    حالت SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      توڑو;
    }
    حالت SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_قدر_double(p_قدر));
      توڑو;
    }
    حالت SQLITE_INTEGER: {
      ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_متن(p_قدر);
      u32 n = (u32)sqlite3_قدر_بائٹیں(p_قدر);
      jsonAppendRaw(p, z, n);
      توڑو;
    }
    حالت SQLITE_متن: {
      ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_متن(p_قدر);
      u32 n = (u32)sqlite3_قدر_بائٹیں(p_قدر);
      اگر( sqlite3_قدر_subtype(p_قدر)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }ورنہ{
        jsonAppendString(p, z, n);
      }
      توڑو;
    }
    طےشدہ: {
      JsonParse px;
      memset(&px, 0, حجم(px));
      اگر( jsonArgIsJsonb(p_قدر, &px) ){
        json_Translate_Blob_مقصد_متن(&px, 0, p);
      }ورنہ اگر( p->eErr==0 ){
        sqlite3_نتیجہ_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      توڑو;
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
ساکن خالی jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  أكد( (pParse!=0)==(ctx!=0) );
  أكد( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  اگر( p->eErr==0 ){
    صحیح flags = SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(p->pCtx));
    اگر( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }ورنہ اگر( p->bStatic ){
      sqlite3_نتیجہ_text64(p->pCtx, p->zBuf, p->n_مستعمل,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }ورنہ{
      اگر( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        صحیح rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->n_مستعمل;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_داخل_کرو(ctx, pParse);
        اگر( rc==SQLITE_NOMEM ){
          sqlite3_نتیجہ_error_nomem(ctx);
          jsonStringReset(p);
          واپس;
        }
      }
      sqlite3_نتیجہ_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->n_مستعمل,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }ورنہ اگر( p->eErr & JSTRING_OOM ){
    sqlite3_نتیجہ_error_nomem(p->pCtx);
  }ورنہ اگر( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }ورنہ اگر( p->eErr & JSTRING_MALFORMED ){
    sqlite3_نتیجہ_error(p->pCtx, "malformed JSON", -1);
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
ساکن خالی jsonParseReset(JsonParse *pParse){
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
ساکن خالی jsonParseFree(JsonParse *pParse){
  اگر( pParse ){
    اگر( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }ورنہ{
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
ساکن u8 json_Hex_مقصد_Int(صحیح h){
#اگر_تعریف_شدہ SQLITE_ASCII
  h += 9*(1&(h>>6));
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#اگر_ختم
  واپس (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
ساکن u32 json_Hex_مقصد_Int4(ثابت حرف *z){
  u32 v;
  v = (json_Hex_مقصد_Int(z[0])<<12)
    + (json_Hex_مقصد_Int(z[1])<<8)
    + (json_Hex_مقصد_Int(z[2])<<4)
    + json_Hex_مقصد_Int(z[3]);
  واپس v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
ساکن صحیح jsonIs2Hex(ثابت حرف *z){
  واپس sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
ساکن صحیح jsonIs4Hex(ثابت حرف *z){
  واپس jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
ساکن صحیح json5Whitespace(ثابت حرف *z_اندر){
  صحیح n = 0;
  ثابت u8 *z = (u8*)z_اندر;
  جب( 1 /*exit by "goto whitespace_done"*/ ){
    انتخاب( z[n] ){
      حالت 0x09:
      حالت 0x0a:
      حالت 0x0b:
      حالت 0x0c:
      حالت 0x0d:
      حالت 0x20: {
        n++;
        توڑو;
      }
      حالت '/': {
        اگر( z[n+1]=='*' && z[n+2]!=0 ){
          صحیح j;
          برائے(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            اگر( z[j]==0 ) جاؤ whitespace_مکمل;
          }
          n = j+1;
          توڑو;
        }ورنہ اگر( z[n+1]=='/' ){
          صحیح j;
          حرف c;
          برائے(j=n+2; (c = z[j])!=0; j++){
            اگر( c=='\n' || c=='\r' ) توڑو;
            اگر( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              توڑو;
            }
          }
          n = j;
          اگر( z[n] ) n++;
          توڑو;
        }
        جاؤ whitespace_مکمل;
      }
      حالت 0xc2: {
        اگر( z[n+1]==0xa0 ){
          n += 2;
          توڑو;
        }
        جاؤ whitespace_مکمل;
      }
      حالت 0xe1: {
        اگر( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          توڑو;
        }
        جاؤ whitespace_مکمل;
      }
      حالت 0xe2: {
        اگر( z[n+1]==0x80 ){
          u8 c = z[n+2];
          اگر( c<0x80 ) جاؤ whitespace_مکمل;
          اگر( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            توڑو;
          }
        }ورنہ اگر( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          توڑو;
        }
        جاؤ whitespace_مکمل;
      }
      حالت 0xe3: {
        اگر( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          توڑو;
        }
        جاؤ whitespace_مکمل;
      }
      حالت 0xef: {
        اگر( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          توڑو;
        }
        جاؤ whitespace_مکمل;
      }
      طےشدہ: {
        جاؤ whitespace_مکمل;
      }
    }
  }
  whitespace_مکمل:
  واپس n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
ساکن ثابت ساخت NanInfName {
  حرف c1;
  حرف c2;
  حرف n;
  حرف eType;
  حرف nRepl;
  حرف *z_مطابقت;
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
ساکن خالی json_Wrong_عدد_دلائل(
  sqlite3_context *pCtx,
  ثابت حرف *zFuncName
){
  حرف *z_پیغام = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_نتیجہ_error(pCtx, z_پیغام, -1);
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
ساکن صحیح jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  أكد( N>pParse->nBlobAlloc );
  اگر( pParse->nBlobAlloc==0 ){
    t = 100;
  }ورنہ{
    t = pParse->nBlobAlloc*2;
  }
  اگر( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  اگر( aNew==0 ){ pParse->oom = 1; واپس 1; }
  أكد( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  واپس 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
ساکن صحیح jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 n_سائز;
  أكد( !pParse->bReadOnly );
  اگر( pParse->oom ) واپس 0;
  اگر( pParse->nBlobAlloc>0 ) واپس 1;
  aOld = pParse->aBlob;
  n_سائز = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  اگر( jsonBlobExpand(pParse, n_سائز) ){
    واپس 0;
  }
  أكد( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  حافظہ_نقل(pParse->aBlob, aOld, pParse->nBlob);
  واپس 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
ساکن SQLITE_NOINLINE خالی jsonBlobExpandAndAppendOneByte(
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
ساکن خالی jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  اگر( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }ورنہ{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
ساکن خالی json_Blob_Append_گرہ(JsonParse*,u8,u64,ثابت خالی*);
ساکن SQLITE_NOINLINE خالی json_Blob_Expand_And_Append_گرہ(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  ثابت خالی *aPayload
){
  اگر( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) واپس;
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
ساکن خالی json_Blob_Append_گرہ(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  ثابت خالی *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  اگر( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_گرہ(pParse,eType,szPayload,aPayload);
    واپس;
  }
  أكد( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  اگر( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }ورنہ اگر( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }ورنہ اگر( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }ورنہ{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  اگر( aPayload ){
    pParse->nBlob += szPayload;
    حافظہ_نقل(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
ساکن صحیح json_Blob_Change_Payload_سائز(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  صحیح delta;
  اگر( pParse->oom ) واپس 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  اگر( szType<=11 ){
    nExtra = 0;
  }ورنہ اگر( szType==12 ){
    nExtra = 1;
  }ورنہ اگر( szType==13 ){
    nExtra = 2;
  }ورنہ اگر( szType==14 ){
    nExtra = 4;
  }ورنہ{
    nExtra = 8;
  }
  اگر( szPayload<=11 ){
    nNeeded = 0;
  }ورنہ اگر( szPayload<=0xff ){
    nNeeded = 1;
  }ورنہ اگر( szPayload<=0xffff ){
    nNeeded = 2;
  }ورنہ{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  اگر( delta ){
    u64 new_سائز_2 = (u64)pParse->nBlob + delta;
    اگر( delta>0 ){
      اگر( new_سائز_2>pParse->nBlobAlloc && jsonBlobExpand(pParse, new_سائز_2) ){
        واپس 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }ورنہ{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = new_سائز_2;
  }
  اگر( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }ورنہ اگر( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }ورنہ اگر( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }ورنہ{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  واپس delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
ساکن صحیح jsonIs4HexB(ثابت حرف *z, صحیح *pOp){
  اگر( z[0]!='u' ) واپس 0;
  اگر( !jsonIs4Hex(&z[1]) ) واپس 0;
  *pOp = JSONB_TEXTJ;
  واپس 1;
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
ساکن u32 jsonbValidityCheck(
  ثابت JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 i_انتہا,                   /* One more than the last byte of the element */
  u32 i_گہرائی                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  ثابت u8 *z;
  u8 x;
  اگر( i_گہرائی>JSON_MAX_گہرائی ) واپس i+1;
  sz = 0;
  n = jsonb_Payload_سائز(pParse, i, &sz);
  اگر( NEVER(n==0) ) واپس i+1;          /* Checked by caller */
  اگر( NEVER(i+n+sz!=i_انتہا) ) واپس i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  انتخاب( x ){
    حالت JSONB_NULL:
    حالت JSONB_TRUE:
    حالت JSONB_FALSE: {
      واپس n+sz==1 ? 0 : i+1;
    }
    حالت JSONB_INT: {
      اگر( sz<1 ) واپس i+1;
      j = i+n;
      اگر( z[j]=='-' ){
        j++;
        اگر( sz<2 ) واپس i+1;
      }
      k = i+n+sz;
      جب( j<k ){
        اگر( sqlite3Isdigit(z[j]) ){
          j++;
        }ورنہ{
          واپس j+1;
        }
      }
      واپس 0;
    }
    حالت JSONB_INT5: {
      اگر( sz<3 ) واپس i+1;
      j = i+n;
      اگر( z[j]=='-' ){
        اگر( sz<4 ) واپس i+1;
        j++;
      }
      اگر( z[j]!='0' ) واپس i+1;
      اگر( z[j+1]!='x' && z[j+1]!='X' ) واپس j+2;
      j += 2;
      k = i+n+sz;
      جب( j<k ){
        اگر( sqlite3Isxdigit(z[j]) ){
          j++;
        }ورنہ{
          واپس j+1;
        }
      }
      واپس 0;
    }
    حالت JSONB_FLOAT:
    حالت JSONB_FLOAT5: {
      u8 نظر_آیا = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      اگر( sz<2 ) واپس i+1;
      j = i+n;
      k = j+sz;
      اگر( z[j]=='-' ){
        j++;
        اگر( sz<3 ) واپس i+1;
      }
      اگر( z[j]=='.' ){
        اگر( x==JSONB_FLOAT ) واپس j+1;
        اگر( !sqlite3Isdigit(z[j+1]) ) واپس j+1;
        j += 2;
        نظر_آیا = 1;
      }ورنہ اگر( z[j]=='0' && x==JSONB_FLOAT ){
        اگر( j+3>k ) واپس j+1;
        اگر( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) واپس j+1;
        j++;
      }
      برائے(; j<k; j++){
        اگر( sqlite3Isdigit(z[j]) ) جاری;
        اگر( z[j]=='.' ){
          اگر( نظر_آیا>0 ) واپس j+1;
          اگر( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            واپس j+1;
          }
          نظر_آیا = 1;
          جاری;
        }
        اگر( z[j]=='e' || z[j]=='E' ){
          اگر( نظر_آیا==2 ) واپس j+1;
          اگر( j==k-1 ) واپس j+1;
          اگر( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            اگر( j==k-1 ) واپس j+1;
          }
          نظر_آیا = 2;
          جاری;
        }
        واپس j+1;
      }
      اگر( نظر_آیا==0 ) واپس i+1;
      واپس 0;
    }
    حالت JSONB_متن: {
      j = i+n;
      k = j+sz;
      جب( j<k ){
        اگر( !jsonIsOk[z[j]] && z[j]!='\'' ) واپس j+1;
        j++;
      }
      واپس 0;
    }
    حالت JSONB_TEXTJ:
    حالت JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      جب( j<k ){
        اگر( !jsonIsOk[z[j]] && z[j]!='\'' ){
          اگر( z[j]=='"' ){
            اگر( x==JSONB_TEXTJ ) واپس j+1;
          }ورنہ اگر( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            اگر( x==JSONB_TEXTJ ) واپس j+1;
          }ورنہ اگر( NEVER(z[j]!='\\') || j+1>=k ){
            واپس j+1;
          }ورنہ اگر( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }ورنہ اگر( z[j+1]=='u' ){
            اگر( j+5>=k ) واپس j+1;
            اگر( !jsonIs4Hex((ثابت حرف*)&z[j+2]) ) واپس j+1;
            j++;
          }ورنہ اگر( x!=JSONB_TEXT5 ){
            واپس j+1;
          }ورنہ{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((ثابت حرف*)&z[j], k-j, &c);
            اگر( c==JSON_INVALID_CHAR ) واپس j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      واپس 0;
    }
    حالت JSONB_TEXTRAW: {
      واپس 0;
    }
    حالت JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      جب( j<k ){
        sz = 0;
        n = jsonb_Payload_سائز(pParse, j, &sz);
        اگر( n==0 ) واپس j+1;
        اگر( j+n+sz>k ) واپس j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_گہرائی+1);
        اگر( sub ) واپس sub;
        j += n + sz;
      }
      أكد( j==k );
      واپس 0;
    }
    حالت JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      جب( j<k ){
        sz = 0;
        n = jsonb_Payload_سائز(pParse, j, &sz);
        اگر( n==0 ) واپس j+1;
        اگر( j+n+sz>k ) واپس j+1;
        اگر( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          اگر( x<JSONB_متن || x>JSONB_TEXTRAW ) واپس j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_گہرائی+1);
        اگر( sub ) واپس sub;
        cnt++;
        j += n + sz;
      }
      أكد( j==k );
      اگر( (cnt & 1)!=0 ) واپس j+1;
      واپس 0;
    }
    طےشدہ: {
      واپس i+1;
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
ساکن صحیح json_Translate_متن_مقصد_Blob(JsonParse *pParse, u32 i){
  حرف c;
  u32 j;
  u32 iThis, i_آغاز;
  صحیح x;
  u8 t;
  ثابت حرف *z = pParse->zJson;
json_parse_restart:
  انتخاب( (u8)z[i] ){
  حالت '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_گرہ(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    اگر( ++pParse->i_گہرائی > JSON_MAX_گہرائی ){
      pParse->iErr = i;
      واپس -1;
    }
    i_آغاز = pParse->nBlob;
    برائے(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = json_Translate_متن_مقصد_Blob(pParse, j);
      اگر( x<=0 ){
        صحیح op;
        اگر( x==(-2) ){
          j = pParse->iErr;
          اگر( pParse->nBlob!=(u32)i_آغاز ) pParse->hasNonstd = 1;
          توڑو;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_متن;
        اگر( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          صحیح k = j+1;
          جب( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          أكد( iBlob==pParse->nBlob );
          json_Blob_Append_گرہ(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }ورنہ{
          اگر( x!=-1 ) pParse->iErr = j;
          واپس -1;
        }
      }
      اگر( pParse->oom ) واپس -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      اگر( t<JSONB_متن || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        واپس -1;
      }
      j = x;
      اگر( z[j]==':' ){
        j++;
      }ورنہ{
        اگر( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          کرو{ j++; }جب( jsonIsspace(z[j]) );
          اگر( z[j]==':' ){
            j++;
            جاؤ parse_object_قدر;
          }
        }
        x = json_Translate_متن_مقصد_Blob(pParse, j);
        اگر( x!=(-5) ){
          اگر( x!=(-1) ) pParse->iErr = j;
          واپس -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_قدر:
      x = json_Translate_متن_مقصد_Blob(pParse, j);
      اگر( x<=0 ){
        اگر( x!=(-1) ) pParse->iErr = j;
        واپس -1;
      }
      j = x;
      اگر( z[j]==',' ){
        جاری;
      }ورنہ اگر( z[j]=='}' ){
        توڑو;
      }ورنہ{
        اگر( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          اگر( z[j]==',' ){
            جاری;
          }ورنہ اگر( z[j]=='}' ){
            توڑو;
          }
        }
        x = json_Translate_متن_مقصد_Blob(pParse, j);
        اگر( x==(-4) ){
          j = pParse->iErr;
          جاری;
        }
        اگر( x==(-2) ){
          j = pParse->iErr;
          توڑو;
        }
      }
      pParse->iErr = j;
      واپس -1;
    }
    json_Blob_Change_Payload_سائز(pParse, iThis, pParse->nBlob - i_آغاز);
    pParse->i_گہرائی--;
    واپس j+1;
  }
  حالت '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    أكد( i<=(u32)pParse->nJson );
    json_Blob_Append_گرہ(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    i_آغاز = pParse->nBlob;
    اگر( pParse->oom ) واپس -1;
    اگر( ++pParse->i_گہرائی > JSON_MAX_گہرائی ){
      pParse->iErr = i;
      واپس -1;
    }
    برائے(j=i+1;;j++){
      x = json_Translate_متن_مقصد_Blob(pParse, j);
      اگر( x<=0 ){
        اگر( x==(-3) ){
          j = pParse->iErr;
          اگر( pParse->nBlob!=i_آغاز ) pParse->hasNonstd = 1;
          توڑو;
        }
        اگر( x!=(-1) ) pParse->iErr = j;
        واپس -1;
      }
      j = x;
      اگر( z[j]==',' ){
        جاری;
      }ورنہ اگر( z[j]==']' ){
        توڑو;
      }ورنہ{
        اگر( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          اگر( z[j]==',' ){
            جاری;
          }ورنہ اگر( z[j]==']' ){
            توڑو;
          }
        }
        x = json_Translate_متن_مقصد_Blob(pParse, j);
        اگر( x==(-4) ){
          j = pParse->iErr;
          جاری;
        }
        اگر( x==(-3) ){
          j = pParse->iErr;
          توڑو;
        }
      }
      pParse->iErr = j;
      واپس -1;
    }
    json_Blob_Change_Payload_سائز(pParse, iThis, pParse->nBlob - i_آغاز);
    pParse->i_گہرائی--;
    واپس j+1;
  }
  حالت '\'': {
    u8 opcode;
    حرف cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_متن;
    جاؤ parse_string;
  حالت '"':
    /* Parse string */
    opcode = JSONB_متن;
  parse_string:
    cDelim = z[i];
    j = i+1;
    جب( 1 /*exit-by-break*/ ){
      اگر( jsonIsOk[(u8)z[j]] ){
        اگر( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }ورنہ اگر( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }ورنہ{
          j += 3;
          جاری;
        }
      }
      c = z[j];
      اگر( c==cDelim ){
        توڑو;
      }ورنہ اگر( c=='\\' ){
        c = z[++j];
        اگر( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          اگر( opcode==JSONB_متن ) opcode = JSONB_TEXTJ;
        }ورنہ اگر( c=='\'' ||  c=='v' || c=='\n'
#اگر_تعریف_شدہ SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#ورنہ
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#اگر_ختم
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ورنہ اگر( c=='\r' ){
          اگر( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }ورنہ{
          pParse->iErr = j;
          واپس -1;
        }
      }ورنہ اگر( c<=0x1f ){
        اگر( c==0 ){
          pParse->iErr = j;
          واپس -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }ورنہ اگر( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_گرہ(pParse, opcode, j-1-i, &z[i+1]);
    واپس j+1;
  }
  حالت 't': {
    اگر( لڑی_موازنہ(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      واپس i+4;
    }
    pParse->iErr = i;
    واپس -1;
  }
  حالت 'f': {
    اگر( لڑی_موازنہ(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      واپس i+5;
    }
    pParse->iErr = i;
    واپس -1;
  }
  حالت '+': {
    u8 نظر_آیا_E;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    جاؤ parse_نمبر;
  حالت '.':
    اگر( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      نظر_آیا_E = 0;
      جاؤ parse_نمبر_2;
    }
    pParse->iErr = i;
    واپس -1;
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
  parse_نمبر:
    نظر_آیا_E = 0;
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
          برائے(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          جاؤ parse_نمبر_اختتام;
        }ورنہ اگر( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          واپس -1;
        }
      }ورنہ{
        اگر( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          اگر( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            اگر( z[i]=='-' ){
              json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 6, "-9e999");
            }ورنہ{
              json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 5, "9e999");
            }
            واپس i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          اگر( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            جاؤ parse_نمبر_2;
          }
          pParse->iErr = i;
          واپس -1;
        }
        اگر( z[i+1]=='0' ){
          اگر( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            واپس -1;
          }ورنہ اگر( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            برائے(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            جاؤ parse_نمبر_اختتام;
          }
        }
      }
    }

  parse_نمبر_2:
    برائے(j=i+1;; j++){
      c = z[j];
      اگر( sqlite3Isdigit(c) ) جاری;
      اگر( c=='.' ){
        اگر( (t & 0x02)!=0 ){
          pParse->iErr = j;
          واپس -1;
        }
        t |= 0x02;
        جاری;
      }
      اگر( c=='e' || c=='E' ){
        اگر( z[j-1]<'0' ){
          اگر( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }ورنہ{
            pParse->iErr = j;
            واپس -1;
          }
        }
        اگر( نظر_آیا_E ){
          pParse->iErr = j;
          واپس -1;
        }
        t |= 0x02;
        نظر_آیا_E = 1;
        c = z[j+1];
        اگر( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        اگر( c<'0' || c>'9' ){
          pParse->iErr = j;
          واپس -1;
        }
        جاری;
      }
      توڑو;
    }
    اگر( z[j-1]<'0' ){
      اگر( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }ورنہ{
        pParse->iErr = j;
        واپس -1;
      }
    }
  parse_نمبر_اختتام:
    أكد( JSONB_INT+0x01==JSONB_INT5 );
    أكد( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    أكد( JSONB_INT+0x02==JSONB_FLOAT );
    اگر( z[i]=='+' ) i++;
    json_Blob_Append_گرہ(pParse, JSONB_INT+t, j-i, &z[i]);
    واپس j;
  }
  حالت '}': {
    pParse->iErr = i;
    واپس -2;  /* End of {...} */
  }
  حالت ']': {
    pParse->iErr = i;
    واپس -3;  /* End of [...] */
  }
  حالت ',': {
    pParse->iErr = i;
    واپس -4;  /* List separator */
  }
  حالت ':': {
    pParse->iErr = i;
    واپس -5;  /* Object label/value separator */
  }
  حالت 0: {
    واپس 0;   /* End of file */
  }
  حالت 0x09:
  حالت 0x0a:
  حالت 0x0d:
  حالت 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    جاؤ json_parse_restart;
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
      جاؤ json_parse_restart;
    }
    pParse->iErr = i;
    واپس -1;
  }
  حالت 'n': {
    اگر( لڑی_موازنہ(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      واپس i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  طےشدہ: {
    u32 k;
    صحیح nn;
    c = z[i];
    برائے(k=0; k<حجم(aNanInfName)/حجم(aNanInfName[0]); k++){
      اگر( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) جاری;
      nn = aNanInfName[k].n;
      اگر( sqlite3StrNICmp(&z[i], aNanInfName[k].z_مطابقت, nn)!=0 ){
        جاری;
      }
      اگر( sqlite3Isalnum(z[i+nn]) ) جاری;
      اگر( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 5, "9e999");
      }ورنہ{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      واپس i + nn;
    }
    pParse->iErr = i;
    واپس -1;  /* Syntax error */
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
ساکن صحیح json_Convert_متن_مقصد_Blob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  صحیح i;
  ثابت حرف *zJson = pParse->zJson;
  i = json_Translate_متن_مقصد_Blob(pParse, 0);
  اگر( pParse->oom ) i = -1;
  اگر( i>0 ){
#اگر_تعریف_شدہ SQLITE_DEBUG
    أكد( pParse->i_گہرائی==0 );
    اگر( sqlite3Config.bJsonSelfcheck ){
      أكد( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#اگر_ختم
    جب( jsonIsspace(zJson[i]) ) i++;
    اگر( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      اگر( zJson[i] ){
        اگر( pCtx ) sqlite3_نتیجہ_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        واپس 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  اگر( i<=0 ){
    اگر( pCtx!=0 ){
      اگر( pParse->oom ){
        sqlite3_نتیجہ_error_nomem(pCtx);
      }ورنہ{
        sqlite3_نتیجہ_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    واپس 1;
  }
  واپس 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
ساکن خالی jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  أكد( pStr->eErr==0 );
  memset(&px, 0, حجم(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->n_مستعمل;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (خالی)json_Translate_متن_مقصد_Blob(&px, 0);
  اگر( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_نتیجہ_error_nomem(pStr->pCtx);
  }ورنہ{
    أكد( px.nBlobAlloc>0 );
    أكد( !px.bReadOnly );
    sqlite3_نتیجہ_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
ساکن u32 jsonb_Payload_سائز_Wide(
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
      واپس 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ورنہ اگر( x==14 ){
    اگر( i+4>=pParse->nBlob ){
      *pSz = 0;
      واپس 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ورنہ{
    اگر( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      واپس 0;
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
    واپس 0;
  }
  *pSz = sz;
  واپس n;
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
ساکن u32 jsonb_Payload_سائز(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  اگر( i>=pParse->nBlob ){
    *pSz = 0;
    واپس 0;
  }ورنہ اگر( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ورنہ اگر( x==12 ){
    اگر( i+1>=pParse->nBlob ){
      *pSz = 0;
      واپس 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ورنہ{
    واپس jsonb_Payload_سائز_Wide(pParse, i, pSz, x);
  }
  اگر( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    واپس 0;
  }
  *pSz = sz;
  واپس n;
}

#اگر SQLITE_استعمال_کرتا_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
ساکن SQLITE_INLINE u32 jsonb_Payload_سائز_Inline(
  ثابت JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  اگر( i>=pParse->nBlob ){
    *pSz = 0;
    واپس 0;
  }ورنہ اگر( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }ورنہ اگر( x==12 ){
    اگر( i+1>=pParse->nBlob ){
      *pSz = 0;
      واپس 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ورنہ اگر( x==13 ){
    اگر( i+2>=pParse->nBlob ){
      *pSz = 0;
      واپس 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }ورنہ اگر( x==14 ){
    اگر( i+4>=pParse->nBlob ){
      *pSz = 0;
      واپس 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }ورنہ{
    اگر( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      واپس 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  اگر( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    واپس 0;
  }
  *pSz = sz;
  واپس n;
}
#ورنہ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_سائز_Inline(a,b,c) jsonb_Payload_سائز(a,b,c)
#اگر_ختم


#اگر SQLITE_استعمال_کرتا_INLINE
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
ساکن SQLITE_INLINE u32 jsonb_Payload_سائز_Semi_Inline(
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
  }ورنہ اگر( x==12 ){
    اگر( i+1>=pParse->nBlob ){
      *pSz = 0;
      واپس 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }ورنہ{
    واپس jsonb_Payload_سائز_Wide(pParse,i,pSz,x);
  }
  اگر( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    واپس 0;
  }
  *pSz = sz;
  واپس n;
}
#ورنہ /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_سائز_Semi_Inline(a,b,c) jsonb_Payload_سائز(a,b,c)
#اگر_ختم

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
ساکن u32 json_Translate_Blob_مقصد_متن(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *p_برآمد               /* Write JSON here */
){
  u32 sz, n, j, i_انتہا;

  n = jsonb_Payload_سائز_Inline(pParse, i, &sz);
  اگر( n==0 ){
    p_برآمد->eErr |= JSTRING_MALFORMED;
    واپس pParse->nBlob+1;
  }
  انتخاب( pParse->aBlob[i] & 0x0f ){
    حالت JSONB_NULL: {
      jsonAppendRawNZ(p_برآمد, "null", 4);
      واپس i+1;
    }
    حالت JSONB_TRUE: {
      jsonAppendRawNZ(p_برآمد, "true", 4);
      واپس i+1;
    }
    حالت JSONB_FALSE: {
      jsonAppendRawNZ(p_برآمد, "false", 5);
      واپس i+1;
    }
    حالت JSONB_INT:
    حالت JSONB_FLOAT: {
      اگر( sz==0 ) جاؤ malformed_jsonb;
      jsonAppendRaw(p_برآمد, (ثابت حرف*)&pParse->aBlob[i+n], sz);
      توڑو;
    }
    حالت JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      ثابت حرف *z_اندر = (ثابت حرف*)&pParse->aBlob[i+n];
      صحیح b_حد_سے_تجاوز = 0;
      اگر( sz==0 ) جاؤ malformed_jsonb;
      اگر( z_اندر[0]=='-' ){
        jsonAppendChar(p_برآمد, '-');
        k++;
      }ورنہ اگر( z_اندر[0]=='+' ){
        k++;
      }
      برائے(; k<sz; k++){
        اگر( !sqlite3Isxdigit(z_اندر[k]) ){
          p_برآمد->eErr |= JSTRING_MALFORMED;
          توڑو;
        }ورنہ اگر( (u>>60)!=0 ){
          b_حد_سے_تجاوز = 1;
        }ورنہ{
          u = u*16 + sqlite3_Hex_مقصد_Int(z_اندر[k]);
        }
      }
      jsonPrintf(100,p_برآمد,b_حد_سے_تجاوز?"9.0e999":"%llu", u);
      توڑو;
    }
    حالت JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      ثابت حرف *z_اندر = (ثابت حرف*)&pParse->aBlob[i+n];
      اگر( sz==0 ) جاؤ malformed_jsonb;
      اگر( z_اندر[0]=='-' ){
        jsonAppendChar(p_برآمد, '-');
        اگر( sz<=1 ) جاؤ malformed_jsonb;
        k = 1;
      }
      اگر( z_اندر[k]=='.' ){
        jsonAppendChar(p_برآمد, '0');
      }
      برائے(; k<sz; k++){
        jsonAppendChar(p_برآمد, z_اندر[k]);
        اگر( z_اندر[k]=='.' && (k+1==sz || !sqlite3Isdigit(z_اندر[k+1])) ){
          jsonAppendChar(p_برآمد, '0');
        }
      }
      توڑو;
    }
    حالت JSONB_متن:
    حالت JSONB_TEXTJ: {
      اگر( p_برآمد->n_مستعمل+sz+2<=p_برآمد->nAlloc || jsonStringGrow(p_برآمد, sz+2)==0 ){
        p_برآمد->zBuf[p_برآمد->n_مستعمل] = '"';
        حافظہ_نقل(p_برآمد->zBuf+p_برآمد->n_مستعمل+1,(ثابت حرف*)&pParse->aBlob[i+n],sz);
        p_برآمد->zBuf[p_برآمد->n_مستعمل+sz+1] = '"';
        p_برآمد->n_مستعمل += sz+2;
      }
      توڑو;
    }
    حالت JSONB_TEXT5: {
      ثابت حرف *z_اندر;
      u32 k;
      u32 sz2 = sz;
      z_اندر = (ثابت حرف*)&pParse->aBlob[i+n];
      jsonAppendChar(p_برآمد, '"');
      جب( sz2>0 ){
        برائے(k=0; k<sz2 && (jsonIsOk[(u8)z_اندر[k]] || z_اندر[k]=='\''); k++){}
        اگر( k>0 ){
          jsonAppendRawNZ(p_برآمد, z_اندر, k);
          اگر( k>=sz2 ){
            توڑو;
          }
          z_اندر += k;
          sz2 -= k;
        }
        اگر( z_اندر[0]=='"' ){
          jsonAppendRawNZ(p_برآمد, "\\\"", 2);
          z_اندر++;
          sz2--;
          جاری;
        }
        اگر( z_اندر[0]<=0x1f ){
          اگر( p_برآمد->n_مستعمل+7>p_برآمد->nAlloc && jsonStringGrow(p_برآمد,7) ) توڑو;
          jsonAppendControlChar(p_برآمد, z_اندر[0]);
          z_اندر++;
          sz2--;
          جاری;
        }
        أكد( z_اندر[0]=='\\' );
        أكد( sz2>=1 );
        اگر( sz2<2 ){
          p_برآمد->eErr |= JSTRING_MALFORMED;
          توڑو;
        }
        انتخاب( (u8)z_اندر[1] ){
          حالت '\'':
            jsonAppendChar(p_برآمد, '\'');
            توڑو;
          حالت 'v':
            jsonAppendRawNZ(p_برآمد, "\\u000b", 6);
            توڑو;
          حالت 'x':
            اگر( sz2<4 ){
              p_برآمد->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              توڑو;
            }
            jsonAppendRawNZ(p_برآمد, "\\u00", 4);
            jsonAppendRawNZ(p_برآمد, &z_اندر[2], 2);
            z_اندر += 2;
            sz2 -= 2;
            توڑو;
          حالت '0':
            jsonAppendRawNZ(p_برآمد, "\\u0000", 6);
            توڑو;
          حالت '\r':
            اگر( sz2>2 && z_اندر[2]=='\n' ){
              z_اندر++;
              sz2--;
            }
            توڑو;
          حالت '\n':
            توڑو;
          حالت 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            اگر( sz2<4
             || 0x80!=(u8)z_اندر[2]
             || (0xa8!=(u8)z_اندر[3] && 0xa9!=(u8)z_اندر[3])
            ){
              p_برآمد->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              توڑو;
            }
            z_اندر += 2;
            sz2 -= 2;
            توڑو;
          طےشدہ:
            jsonAppendRawNZ(p_برآمد, z_اندر, 2);
            توڑو;
        }
        أكد( sz2>=2 );
        z_اندر += 2;
        sz2 -= 2;
      }
      jsonAppendChar(p_برآمد, '"');
      توڑو;
    }
    حالت JSONB_TEXTRAW: {
      jsonAppendString(p_برآمد, (ثابت حرف*)&pParse->aBlob[i+n], sz);
      توڑو;
    }
    حالت JSONB_ARRAY: {
      jsonAppendChar(p_برآمد, '[');
      j = i+n;
      i_انتہا = j+sz;
      اگر( ++pParse->i_گہرائی > JSON_MAX_گہرائی ){
        jsonStringTooDeep(p_برآمد);
      }
      جب( j<i_انتہا && p_برآمد->eErr==0 ){
        j = json_Translate_Blob_مقصد_متن(pParse, j, p_برآمد);
        jsonAppendChar(p_برآمد, ',');
      }
      pParse->i_گہرائی--;
      اگر( j>i_انتہا ) p_برآمد->eErr |= JSTRING_MALFORMED;
      اگر( sz>0 ) jsonStringTrimOneChar(p_برآمد);
      jsonAppendChar(p_برآمد, ']');
      توڑو;
    }
    حالت JSONB_OBJECT: {
      صحیح x = 0;
      jsonAppendChar(p_برآمد, '{');
      j = i+n;
      i_انتہا = j+sz;
      اگر( ++pParse->i_گہرائی > JSON_MAX_گہرائی ){
        jsonStringTooDeep(p_برآمد);
      }
      جب( j<i_انتہا && p_برآمد->eErr==0 ){
        j = json_Translate_Blob_مقصد_متن(pParse, j, p_برآمد);
        jsonAppendChar(p_برآمد, (x++ & 1) ? ',' : ':');
      }
      pParse->i_گہرائی--;
      اگر( (x & 1)!=0 || j>i_انتہا ) p_برآمد->eErr |= JSTRING_MALFORMED;
      اگر( sz>0 ) jsonStringTrimOneChar(p_برآمد);
      jsonAppendChar(p_برآمد, '}');
      توڑو;
    }

    طےشدہ: {
      malformed_jsonb:
      p_برآمد->eErr |= JSTRING_MALFORMED;
      توڑو;
    }
  }
  واپس i+n+sz;
}

/* Context for recursion of json_pretty()
*/
قسمتعریف ساخت JsonPretty JsonPretty;
ساخت JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *p_برآمد;         /* Generate pretty output into this string */
  ثابت حرف *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
ساکن خالی jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  برائے(jj=0; jj<pPretty->nIndent; jj++){
    jsonAppendRaw(pPretty->p_برآمد, pPretty->zIndent, pPretty->szIndent);
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
ساکن u32 json_Translate_Blob_مقصد_Pretty_متن(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, i_انتہا;
  JsonParse *pParse = pPretty->pParse;
  JsonString *p_برآمد = pPretty->p_برآمد;
  n = jsonb_Payload_سائز(pParse, i, &sz);
  اگر( n==0 ){
    p_برآمد->eErr |= JSTRING_MALFORMED;
    واپس pParse->nBlob+1;
  }
  انتخاب( pParse->aBlob[i] & 0x0f ){
    حالت JSONB_ARRAY: {
      j = i+n;
      i_انتہا = j+sz;
      jsonAppendChar(p_برآمد, '[');
      اگر( j<i_انتہا ){
        jsonAppendChar(p_برآمد, '\n');
        pPretty->nIndent++;
        اگر( pPretty->nIndent >= JSON_MAX_گہرائی ){
          jsonStringTooDeep(p_برآمد);
        }
        جب( p_برآمد->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_مقصد_Pretty_متن(pPretty, j);
          اگر( j>=i_انتہا ){
            اگر( j>i_انتہا ) p_برآمد->eErr |= JSTRING_MALFORMED;
            توڑو;
          }
          jsonAppendRawNZ(p_برآمد, ",\n", 2);
        }
        jsonAppendChar(p_برآمد, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_برآمد, ']');
      i = i_انتہا;
      توڑو;
    }
    حالت JSONB_OBJECT: {
      j = i+n;
      i_انتہا = j+sz;
      jsonAppendChar(p_برآمد, '{');
      اگر( j<i_انتہا ){
        jsonAppendChar(p_برآمد, '\n');
        pPretty->nIndent++;
        اگر( pPretty->nIndent >= JSON_MAX_گہرائی ){
          jsonStringTooDeep(p_برآمد);
        }
        pParse->i_گہرائی = pPretty->nIndent;
        جب( p_برآمد->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_مقصد_متن(pParse, j, p_برآمد);
          اگر( j>i_انتہا ){
            p_برآمد->eErr |= JSTRING_MALFORMED;
            توڑو;
          }
          jsonAppendRawNZ(p_برآمد, ": ", 2);
          j = json_Translate_Blob_مقصد_Pretty_متن(pPretty, j);
          اگر( j>=i_انتہا ) توڑو;
          jsonAppendRawNZ(p_برآمد, ",\n", 2);
        }
        jsonAppendChar(p_برآمد, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_برآمد, '}');
      i = i_انتہا;
      توڑو;
    }
    طےشدہ: {
      i = json_Translate_Blob_مقصد_متن(pParse, i, p_برآمد);
      توڑو;
    }
  }
  واپس i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
ساکن u32 jsonb_Array_گنتی(JsonParse *pParse, u32 i_جڑ){
  u32 n, sz, i, i_انتہا;
  u32 k = 0;
  n = jsonb_Payload_سائز(pParse, i_جڑ, &sz);
  i_انتہا = i_جڑ+n+sz;
  برائے(i=i_جڑ+n; n>0 && i<i_انتہا; i+=sz+n, k++){
    n = jsonb_Payload_سائز_Semi_Inline(pParse, i, &sz);
  }
  واپس k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
ساکن خالی json_After_Edit_سائز_Adjust(JsonParse *pParse, u32 i_جڑ){
  u32 sz = 0;
  u32 nBlob;
  أكد( pParse->delta!=0 );
  أكد( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (خالی)jsonb_Payload_سائز(pParse, i_جڑ, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += json_Blob_Change_Payload_سائز(pParse, i_جڑ, sz);
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
ساکن صحیح jsonBlobOverwrite(
  u8 *a_برآمد,                 /* Overwrite here */
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
  ساکن ثابت u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  اگر( (aIns[0]&0x0f)<=2 ) واپس 0;    /* Cannot enlarge NULL, true, false */
  انتخاب( aIns[0]>>4 ){
    طےشدہ: {                         /* aIns[] header size 1 */
      اگر( ((1<<d)&0x116)==0 ) واپس 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      توڑو;
    }
    حالت 12: {                         /* aIns[] header size is 2 */
      اگر( ((1<<d)&0x8a)==0) واپس 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      توڑو;
    }
    حالت 13: {                         /* aIns[] header size is 3 */
      اگر( d!=2 && d!=6 ) واپس 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      توڑو;
    }
    حالت 14: {                         /* aIns[] header size is 5 */
      اگر( d!=4 ) واپس 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      توڑو;
    }
    حالت 15: {                         /* aIns[] header size is 9 */
      واپس 0;                          /* No solution */
    }
  }
  أكد( i>=2 && i<=9 && aType[i-2]!=0 );
  a_برآمد[0] = (aIns[0] & 0x0f) | aType[i-2];
  حافظہ_نقل(&a_برآمد[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  جب( 1/*edit-by-break*/ ){
    i--;
    a_برآمد[i] = szPayload & 0xff;
    اگر( i==1 ) توڑو;
    szPayload >>= 8;
  }
  أكد( (szPayload>>8)==0 );
  واپس 1;
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
ساکن خالی jsonBlobEdit(
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
    واپس;
  }
  اگر( d!=0 ){
    اگر( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      اگر( pParse->oom ) واپس;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  اگر( nIns && aIns ){
    حافظہ_نقل(&pParse->aBlob[iDel], aIns, nIns);
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
ساکن u32 json_بائٹیں_مقصد_Bypass(ثابت حرف *z, u32 n){
  u32 i = 0;
  جب( i+1<n ){
    اگر( z[i]!='\\' ) واپس i;
    اگر( z[i+1]=='\n' ){
      i += 2;
      جاری;
    }
    اگر( z[i+1]=='\r' ){
      اگر( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }ورنہ{
        i += 2;
      }
      جاری;
    }
    اگر( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      جاری;
    }
    توڑو;
  }
  واپس i;
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
ساکن u32 jsonUnescapeOneChar(ثابت حرف *z, u32 n, u32 *pi_برآمد){
  أكد( n>0 );
  أكد( z[0]=='\\' );
  اگر( n<2 ){
    *pi_برآمد = JSON_INVALID_CHAR;
    واپس n;
  }
  انتخاب( (u8)z[1] ){
    حالت 'u': {
      u32 v, vlo;
      اگر( n<6 ){
        *pi_برآمد = JSON_INVALID_CHAR;
        واپس n;
      }
      v = json_Hex_مقصد_Int4(&z[2]);
      اگر( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = json_Hex_مقصد_Int4(&z[8]))&0xfc00)==0xdc00
      ){
        *pi_برآمد = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        واپس 12;
      }ورنہ{
        *pi_برآمد = v;
        واپس 6;
      }
    }
    حالت 'b': {   *pi_برآمد = '\b';  واپس 2; }
    حالت 'f': {   *pi_برآمد = '\f';  واپس 2; }
    حالت 'n': {   *pi_برآمد = '\n';  واپس 2; }
    حالت 'r': {   *pi_برآمد = '\r';  واپس 2; }
    حالت 't': {   *pi_برآمد = '\t';  واپس 2; }
    حالت 'v': {   *pi_برآمد = '\v';  واپس 2; }
    حالت '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#اگر_تعریف_شدہ SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *pi_برآمد = 0;
#ورنہ
      /* Correct behavior */
      *pi_برآمد = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#اگر_ختم
      واپس 2;
    }
    حالت '\'':
    حالت '"':
    حالت '/':
    حالت '\\':{   *pi_برآمد = z[1];  واپس 2; }
    حالت 'x': {
      اگر( n<4 ){
        *pi_برآمد = JSON_INVALID_CHAR;
        واپس n;
      }
      *pi_برآمد = (json_Hex_مقصد_Int(z[2])<<4) | json_Hex_مقصد_Int(z[3]);
      واپس 4;
    }
    حالت 0xe2:
    حالت '\r':
    حالت '\n': {
      u32 nSkip = json_بائٹیں_مقصد_Bypass(z, n);
      اگر( nSkip==0 ){
        *pi_برآمد = JSON_INVALID_CHAR;
        واپس n;
      }ورنہ اگر( nSkip==n ){
        *pi_برآمد = 0;
        واپس n;
      }ورنہ اگر( z[nSkip]=='\\' ){
        واپس nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, pi_برآمد);
      }ورنہ{
        صحیح sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, pi_برآمد);
        واپس nSkip + sz;
      }
    }
    طےشدہ: {
      *pi_برآمد = JSON_INVALID_CHAR;
      واپس 2;
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
ساکن SQLITE_NOINLINE صحیح jsonLabelCompareEscaped(
  ثابت حرف *z_بایاں,          /* The left label */
  u32 n_بایاں,                  /* Size of the left label in bytes */
  صحیح raw_بایاں,                /* True if zLeft contains no escapes */
  ثابت حرف *z_دایاں,         /* The right label */
  u32 n_دایاں,                 /* Size of the right label in bytes */
  صحیح raw_دایاں                /* True if zRight is escape-free */
){
  u32 c_بایاں, c_دایاں;
  أكد( raw_بایاں==0 || raw_دایاں==0 );
  جب( 1 /*exit-by-return*/ ){
    اگر( n_بایاں==0 ){
      c_بایاں = 0;
    }ورنہ اگر( raw_بایاں || z_بایاں[0]!='\\' ){
      c_بایاں = ((u8*)z_بایاں)[0];
      اگر( c_بایاں>=0xc0 ){
        صحیح sz = sqlite3Utf8ReadLimited((u8*)z_بایاں, n_بایاں, &c_بایاں);
        z_بایاں += sz;
        n_بایاں -= sz;
      }ورنہ{
        z_بایاں++;
        n_بایاں--;
      }
    }ورنہ{
      u32 n = jsonUnescapeOneChar(z_بایاں, n_بایاں, &c_بایاں);
      z_بایاں += n;
      أكد( n<=n_بایاں );
      n_بایاں -= n;
    }
    اگر( n_دایاں==0 ){
      c_دایاں = 0;
    }ورنہ اگر( raw_دایاں || z_دایاں[0]!='\\' ){
      c_دایاں = ((u8*)z_دایاں)[0];
      اگر( c_دایاں>=0xc0 ){
        صحیح sz = sqlite3Utf8ReadLimited((u8*)z_دایاں, n_دایاں, &c_دایاں);
        z_دایاں += sz;
        n_دایاں -= sz;
      }ورنہ{
        z_دایاں++;
        n_دایاں--;
      }
    }ورنہ{
      u32 n = jsonUnescapeOneChar(z_دایاں, n_دایاں, &c_دایاں);
      z_دایاں += n;
      أكد( n<=n_دایاں );
      n_دایاں -= n;
    }
    اگر( c_بایاں!=c_دایاں ) واپس 0;
    اگر( c_بایاں==0 ) واپس 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
ساکن صحیح jsonLabelCompare(
  ثابت حرف *z_بایاں,          /* The left label */
  u32 n_بایاں,                  /* Size of the left label in bytes */
  صحیح raw_بایاں,                /* True if zLeft contains no escapes */
  ثابت حرف *z_دایاں,         /* The right label */
  u32 n_دایاں,                 /* Size of the right label in bytes */
  صحیح raw_دایاں                /* True if zRight is escape-free */
){
  اگر( raw_بایاں && raw_دایاں ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    اگر( n_بایاں!=n_دایاں ) واپس 0;
    واپس memcmp(z_بایاں, z_دایاں, n_بایاں)==0;
  }ورنہ{
    واپس jsonLabelCompareEscaped(z_بایاں, n_بایاں, raw_بایاں,
                                   z_دایاں, n_دایاں, raw_دایاں);
  }
}

/*
** Error returns from jsonLookupStep()
*/
#تعریف_کرو JSON_LOOKUP_ERROR      0xffffffff
#تعریف_کرو JSON_LOOKUP_NOTFOUND   0xfffffffe
#تعریف_کرو JSON_LOOKUP_NOTARRAY   0xfffffffd
#تعریف_کرو JSON_LOOKUP_TOODEEP    0xfffffffc
#تعریف_کرو JSON_LOOKUP_PATHERROR  0xfffffffb
#تعریف_کرو JSON_LOOKUP_ISERROR(x) ((x)>=JSON_LOOKUP_PATHERROR)

/* Forward declaration */
ساکن u32 json_Lookup_قدم(JsonParse*,u32,ثابت حرف*,u32);


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
ساکن u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  ثابت حرف *z_دم   /* Tail of the path that determines substructure */
){
  ساکن ثابت u8 خالی_خانے_Object[] = { JSONB_ARRAY, JSONB_OBJECT };
  صحیح rc;
  memset(pIns, 0, حجم(*pIns));
  pIns->db = pParse->db;
  اگر( z_دم[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }ورنہ{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&خالی_خانے_Object[z_دم[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->i_گہرائی = pParse->i_گہرائی+1;
    اگر( pIns->i_گہرائی >= JSON_MAX_گہرائی ){
      واپس JSON_LOOKUP_TOODEEP;
    }
    rc = json_Lookup_قدم(pIns, 0, z_دم, 0);
    pParse->i_گہرائی--;
    pParse->oom |= pIns->oom;
  }
  واپس rc;  /* Error code only */
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
ساکن u32 json_Lookup_قدم(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_جڑ,              /* Begin the search at this element of aBlob[] */
  ثابت حرف *z_راستہ,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_کلید, sz, n, i_انتہا, rc;
  ثابت حرف *z_کلید;
  u8 x;

  اگر( z_راستہ[0]==0 ){
    اگر( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonb_Payload_سائز(pParse, i_جڑ, &sz);
      sz += n;
      اگر( pParse->eEdit==JEDIT_DEL ){
        اگر( iLabel>0 ){
          sz += i_جڑ - iLabel;
          i_جڑ = iLabel;
        }
        jsonBlobEdit(pParse, i_جڑ, sz, 0, 0);
      }ورنہ اگر( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }ورنہ اگر( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        اگر( z_راستہ[-1]!=']' ){
          واپس JSON_LOOKUP_NOTARRAY;
        }ورنہ{
          jsonBlobEdit(pParse, i_جڑ, 0, pParse->aIns, pParse->nIns);
        }
      }ورنہ{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_جڑ, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    واپس i_جڑ;
  }
  اگر( z_راستہ[0]=='.' ){
    صحیح raw_کلید = 1;
    x = pParse->aBlob[i_جڑ];
    z_راستہ++;
    اگر( z_راستہ[0]=='"' ){
      z_کلید = z_راستہ + 1;
      برائے(i=1; z_راستہ[i] && z_راستہ[i]!='"'; i++){
        اگر( z_راستہ[i]=='\\' && z_راستہ[i+1]!=0 ) i++;
      }
      n_کلید = i-1;
      اگر( z_راستہ[i] ){
        i++;
      }ورنہ{
        واپس JSON_LOOKUP_PATHERROR;
      }
      testcase( n_کلید==0 );
      raw_کلید = memchr(z_کلید, '\\', n_کلید)==0;
    }ورنہ{
      z_کلید = z_راستہ;
      برائے(i=0; z_راستہ[i] && z_راستہ[i]!='.' && z_راستہ[i]!='['; i++){}
      n_کلید = i;
      اگر( n_کلید==0 ){
        واپس JSON_LOOKUP_PATHERROR;
      }
    }
    اگر( (x & 0x0f)!=JSONB_OBJECT ) واپس JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_سائز(pParse, i_جڑ, &sz);
    j = i_جڑ + n;  /* j is the index of a label */
    i_انتہا = j+sz;
    جب( j<i_انتہا ){
      صحیح rawLabel;
      ثابت حرف *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      اگر( x<JSONB_متن || x>JSONB_TEXTRAW ) واپس JSON_LOOKUP_ERROR;
      n = jsonb_Payload_سائز_Semi_Inline(pParse, j, &sz);
      اگر( n==0 ) واپس JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      اگر( k+sz>=i_انتہا ) واپس JSON_LOOKUP_ERROR;
      zLabel = (ثابت حرف*)&pParse->aBlob[k];
      rawLabel = x==JSONB_متن || x==JSONB_TEXTRAW;
      اگر( jsonLabelCompare(z_کلید, n_کلید, raw_کلید, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        اگر( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) واپس JSON_LOOKUP_ERROR;
        n = jsonb_Payload_سائز(pParse, v, &sz);
        اگر( n==0 || v+n+sz>i_انتہا ) واپس JSON_LOOKUP_ERROR;
        أكد( j>0 );
        اگر( ++pParse->i_گہرائی >= JSON_MAX_گہرائی ){
          واپس JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_قدم(pParse, v, &z_راستہ[i], j);
        pParse->i_گہرائی--;
        اگر( pParse->delta ) json_After_Edit_سائز_Adjust(pParse, i_جڑ);
        واپس rc;
      }
      j = k+sz;
      اگر( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) واپس JSON_LOOKUP_ERROR;
      n = jsonb_Payload_سائز_Semi_Inline(pParse, j, &sz);
      اگر( n==0 ) واپس JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    اگر( j>i_انتہا ) واپس JSON_LOOKUP_ERROR;
    اگر( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_سیٹ );
      testcase( pParse->eEdit==JEDIT_AINS );
      اگر( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&z_راستہ[i])!=0 ){
        واپس JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, حجم(ix));
      ix.db = pParse->db;
      json_Blob_Append_گرہ(&ix, raw_کلید?JSONB_TEXTRAW:JSONB_TEXT5, n_کلید, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &z_راستہ[i]);
      اگر( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_کلید+v.nBlob)
      ){
        أكد( !pParse->oom );
        nIns = ix.nBlob + n_کلید + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        اگر( !pParse->oom ){
          أكد( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          أكد( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          حافظہ_نقل(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          حافظہ_نقل(&pParse->aBlob[k], z_کلید, n_کلید);
          k += n_کلید;
          حافظہ_نقل(&pParse->aBlob[k], v.aBlob, v.nBlob);
          اگر( ALWAYS(pParse->delta) ) json_After_Edit_سائز_Adjust(pParse, i_جڑ);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      واپس rc;
    }
  }ورنہ اگر( z_راستہ[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_جڑ] & 0x0f;
    اگر( x!=JSONB_ARRAY )  واپس JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_سائز(pParse, i_جڑ, &sz);
    i = 1;
    جب( sqlite3Isdigit(z_راستہ[i]) ){
      اگر( kk<0xffffffff ) kk = kk*10 + z_راستہ[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    اگر( i<2 || z_راستہ[i]!=']' ){
      اگر( z_راستہ[1]=='#' ){
        kk = jsonb_Array_گنتی(pParse, i_جڑ);
        i = 2;
        اگر( z_راستہ[2]=='-' && sqlite3Isdigit(z_راستہ[3]) ){
          u64 nn = 0;
          i = 3;
          کرو{
            اگر( nn<0xffffffff ) nn = nn*10 + z_راستہ[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }جب( sqlite3Isdigit(z_راستہ[i]) );
          اگر( nn>kk ) واپس JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        اگر( z_راستہ[i]!=']' ){
          واپس JSON_LOOKUP_PATHERROR;
        }
      }ورنہ{
        واپس JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_جڑ+n;
    i_انتہا = j+sz;
    جب( j<i_انتہا ){
      اگر( kk==0 ){
        اگر( ++pParse->i_گہرائی >= JSON_MAX_گہرائی ){
          واپس JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_قدم(pParse, j, &z_راستہ[i+1], 0);
        pParse->i_گہرائی--;
        اگر( pParse->delta ) json_After_Edit_سائز_Adjust(pParse, i_جڑ);
        واپس rc;
      }
      kk--;
      n = jsonb_Payload_سائز_Semi_Inline(pParse, j, &sz);
      اگر( n==0 ) واپس JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    اگر( j>i_انتہا ) واپس JSON_LOOKUP_ERROR;
    اگر( kk>0 ) واپس JSON_LOOKUP_NOTFOUND;
    اگر( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_سیٹ );
      rc = jsonCreateEditSubstructure(pParse, &v, &z_راستہ[i+1]);
      اگر( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        أكد( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      اگر( pParse->delta ) json_After_Edit_سائز_Adjust(pParse, i_جڑ);
      واپس rc;
    }
  }ورنہ{
    واپس JSON_LOOKUP_PATHERROR; 
  }
  واپس JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
ساکن خالی json_Return_متن_Json_From_Blob(
  sqlite3_context *ctx,
  ثابت u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  اگر( NEVER(aBlob==0) ) واپس;
  memset(&x, 0, حجم(x));
  x.aBlob = (u8*)aBlob;
  x.nBlob = nBlob;
  jsonStringInit(&s, ctx);
  json_Translate_Blob_مقصد_متن(&x, 0, &s);
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
ساکن خالی jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  صحیح e_منوال                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  صحیح rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  أكد( e_منوال>=0 && e_منوال<=2 );
  n = jsonb_Payload_سائز(pParse, i, &sz);
  اگر( n==0 ){
    sqlite3_نتیجہ_error(pCtx, "malformed JSON", -1);
    واپس;
  }
  انتخاب( pParse->aBlob[i] & 0x0f ){
    حالت JSONB_NULL: {
      اگر( sz ) جاؤ returnfromblob_malformed;
      sqlite3_نتیجہ_null(pCtx);
      توڑو;
    }
    حالت JSONB_TRUE: {
      اگر( sz ) جاؤ returnfromblob_malformed;
      sqlite3_نتیجہ_int(pCtx, 1);
      توڑو;
    }
    حالت JSONB_FALSE: {
      اگر( sz ) جاؤ returnfromblob_malformed;
      sqlite3_نتیجہ_int(pCtx, 0);
      توڑو;
    }
    حالت JSONB_INT5:
    حالت JSONB_INT: {
      sqlite3_int64 iRes = 0;
      حرف *z;
      صحیح bNeg = 0;
      حرف x;
      اگر( sz==0 ) جاؤ returnfromblob_malformed;
      x = (حرف)pParse->aBlob[i+n];
      اگر( x=='-' ){
        اگر( sz<2 ) جاؤ returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (ثابت حرف*)&pParse->aBlob[i+n], (صحیح)sz);
      اگر( z==0 ) جاؤ returnfromblob_oom;
      rc = sqlite3_Dec_Or_Hex_مقصد_I64(z, &iRes);
      sqlite3DbFree(db, z);
      اگر( rc==0 ){
        اگر( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          دوہرا r;
          r = (دوہرا)*(sqlite3_uint64*)&iRes;
          sqlite3_نتیجہ_double(pCtx, bNeg ? -r : r);
        }ورنہ{
          sqlite3_نتیجہ_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }ورنہ اگر( rc==3 && bNeg ){
        sqlite3_نتیجہ_int64(pCtx, سب_سے_چھوٹا_INT64);
      }ورنہ اگر( rc==1 ){
        جاؤ returnfromblob_malformed;
      }ورنہ{
        اگر( bNeg ){ n--; sz++; }
        جاؤ مقصد_double;
      }
      توڑو;
    }
    حالت JSONB_FLOAT5:
    حالت JSONB_FLOAT: {
      دوہرا r;
      حرف *z;
      اگر( sz==0 ) جاؤ returnfromblob_malformed;
    مقصد_double:
      z = sqlite3DbStrNDup(db, (ثابت حرف*)&pParse->aBlob[i+n], (صحیح)sz);
      اگر( z==0 ) جاؤ returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      اگر( rc<=0 ) جاؤ returnfromblob_malformed;
      sqlite3_نتیجہ_double(pCtx, r);
      توڑو;
    }
    حالت JSONB_TEXTRAW:
    حالت JSONB_متن: {
      sqlite3_نتیجہ_متن(pCtx, (حرف*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      توڑو;
    }
    حالت JSONB_TEXT5:
    حالت JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 i_اندر, i_برآمد;
      ثابت حرف *z;
      حرف *z_برآمد;
      u32 n_برآمد = sz;
      z = (ثابت حرف*)&pParse->aBlob[i+n];
      z_برآمد = sqlite3DbMallocRaw(db, ((u64)n_برآمد)+1);
      اگر( z_برآمد==0 ) جاؤ returnfromblob_oom;
      برائے(i_اندر=i_برآمد=0; i_اندر<sz; i_اندر++){
        حرف c = z[i_اندر];
        اگر( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[i_اندر], sz-i_اندر, &v);
          اگر( v==0 && i_اندر+szEscape==sz
           && (z[i_اندر+1]=='\n' || z[i_اندر+1]=='\r' || (u8)z[i_اندر+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }ورنہ اگر( v<=0x7f ){
            z_برآمد[i_برآمد++] = (حرف)v;
          }ورنہ اگر( v<=0x7ff ){
            أكد( szEscape>=2 );
            z_برآمد[i_برآمد++] = (حرف)(0xc0 | (v>>6));
            z_برآمد[i_برآمد++] = 0x80 | (v&0x3f);
          }ورنہ اگر( v<0x10000 ){
            أكد( szEscape>=3 );
            z_برآمد[i_برآمد++] = 0xe0 | (v>>12);
            z_برآمد[i_برآمد++] = 0x80 | ((v>>6)&0x3f);
            z_برآمد[i_برآمد++] = 0x80 | (v&0x3f);
          }ورنہ اگر( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }ورنہ{
            أكد( szEscape>=4 );
            z_برآمد[i_برآمد++] = 0xf0 | (v>>18);
            z_برآمد[i_برآمد++] = 0x80 | ((v>>12)&0x3f);
            z_برآمد[i_برآمد++] = 0x80 | ((v>>6)&0x3f);
            z_برآمد[i_برآمد++] = 0x80 | (v&0x3f);
          }
          i_اندر += szEscape - 1;
        }ورنہ{
          z_برآمد[i_برآمد++] = c;
        }
      } /* end for() */
      أكد( i_برآمد<=n_برآمد );
      z_برآمد[i_برآمد] = 0;
      sqlite3_نتیجہ_متن(pCtx, z_برآمد, i_برآمد, SQLITE_DYNAMIC);
      توڑو;
    }
    حالت JSONB_ARRAY:
    حالت JSONB_OBJECT: {
      اگر( e_منوال==0 ){
        اگر( (SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(pCtx)) & JSON_BLOB)!=0 ){
          e_منوال = 2;
        }ورنہ{
          e_منوال = 1;
        }
      }
      اگر( e_منوال==2 ){
        sqlite3_نتیجہ_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }ورنہ{
        json_Return_متن_Json_From_Blob(pCtx, &pParse->aBlob[i], sz+n);
      }
      توڑو;
    }
    طےشدہ: {
      جاؤ returnfromblob_malformed;
    }
  }
  واپس;

returnfromblob_oom:
  sqlite3_نتیجہ_error_nomem(pCtx);
  واپس;

returnfromblob_malformed:
  sqlite3_نتیجہ_error(pCtx, "malformed JSON", -1);
  واپس;
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
ساکن صحیح json_Function_Arg_مقصد_Blob(
  sqlite3_context *ctx,
  sqlite3_قدر *pArg,
  JsonParse *pParse
){
  صحیح eType = sqlite3_قدر_type(pArg);
  ساکن u8 aNull[] = { 0x00 };
  memset(pParse, 0, حجم(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  انتخاب( eType ){
    طےشدہ: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      واپس 0;
    }
    حالت SQLITE_BLOB: {
      اگر( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_نتیجہ_error(ctx, "JSON cannot hold BLOB values", -1);
        واپس 1;
      }
      توڑو;
    }
    حالت SQLITE_متن: {
      ثابت حرف *zJson = (ثابت حرف*)sqlite3_قدر_متن(pArg);
      صحیح nJson = sqlite3_قدر_بائٹیں(pArg);
      اگر( zJson==0 ) واپس 1;
      اگر( sqlite3_قدر_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (حرف*)zJson;
        pParse->nJson = nJson;
        اگر( json_Convert_متن_مقصد_Blob(pParse, ctx) ){
          sqlite3_نتیجہ_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, حجم(pParse[0]));
          واپس 1;
        }
      }ورنہ{
        json_Blob_Append_گرہ(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      توڑو;
    }
    حالت SQLITE_FLOAT: {
      اگر( NEVER(sqlite3IsNaN(sqlite3_قدر_double(pArg))) ){
        json_Blob_Append_گرہ(pParse, JSONB_NULL, 0, 0);
      }ورنہ{
        صحیح n = sqlite3_قدر_بائٹیں(pArg);
        ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_متن(pArg);
        اگر( z==0 ) واپس 1;
        اگر( z[0]=='I' ){
          json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 5, "9e999");
        }ورنہ اگر( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_گرہ(pParse, JSONB_FLOAT, 6, "-9e999");
        }ورنہ{
          json_Blob_Append_گرہ(pParse, JSONB_FLOAT, n, z);
        }
      }
      توڑو;
    }
    حالت SQLITE_INTEGER: {
      صحیح n = sqlite3_قدر_بائٹیں(pArg);
      ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_متن(pArg);
      اگر( z==0 ) واپس 1;
      json_Blob_Append_گرہ(pParse, JSONB_INT, n, z);
      توڑو;
    }
  }
  اگر( pParse->oom ){
    sqlite3_نتیجہ_error_nomem(ctx);
    واپس 1;
  }ورنہ{
    واپس 0;
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
ساکن حرف *json_Bad_راستہ_Error(
  sqlite3_context *ctx,     /* The function call containing the error */
  ثابت حرف *z_راستہ,        /* The path with the problem */
  صحیح rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  حرف *z_پیغام;
  اگر( rc==(صحیح)JSON_LOOKUP_NOTARRAY ){
    z_پیغام = sqlite3_mprintf("not an array element: %Q", z_راستہ);
  }ورنہ اگر( rc==(صحیح)JSON_LOOKUP_ERROR ){
    z_پیغام = sqlite3_mprintf("malformed JSON");
  }ورنہ اگر( rc==(صحیح)JSON_LOOKUP_TOODEEP ){
    z_پیغام = sqlite3_mprintf("JSON path too deep");
  }ورنہ{
    z_پیغام = sqlite3_mprintf("bad JSON path: %Q", z_راستہ);
  }
  اگر( ctx==0 ) واپس z_پیغام;
  اگر( z_پیغام ){
    sqlite3_نتیجہ_error(ctx, z_پیغام, -1);
    sqlite3_free(z_پیغام);
  }ورنہ{
    sqlite3_نتیجہ_error_nomem(ctx);
  }
  واپس 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
ساکن خالی json_داخل_کرو_Into_Blob(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv,
  صحیح eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  صحیح i;
  u32 rc = 0;
  ثابت حرف *z_راستہ = 0;
  صحیح flgs;
  JsonParse *p;
  JsonParse ax;

  أكد( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  اگر( p==0 ) واپس;
  برائے(i=1; i<argc-1; i+=2){
    اگر( sqlite3_قدر_type(argv[i])==SQLITE_NULL ) جاری;
    z_راستہ = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
    اگر( z_راستہ==0 ){
      sqlite3_نتیجہ_error_nomem(ctx);
      jsonParseFree(p);
      واپس;
    }
    اگر( z_راستہ[0]!='$' ) جاؤ json_داخل_کرو_Into_Blob_patherror;
    اگر( json_Function_Arg_مقصد_Blob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      واپس;
    }
    اگر( z_راستہ[1]==0 ){
      اگر( eEdit==JEDIT_REPL || eEdit==JEDIT_سیٹ ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }ورنہ{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->i_گہرائی = 0;
      rc = json_Lookup_قدم(p, 0, z_راستہ+1, 0);
    }
    jsonParseReset(&ax);
    اگر( rc==JSON_LOOKUP_NOTFOUND ) جاری;
    اگر( JSON_LOOKUP_ISERROR(rc) ) جاؤ json_داخل_کرو_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  واپس;

json_داخل_کرو_Into_Blob_patherror:
  jsonParseFree(p);
  json_Bad_راستہ_Error(ctx, z_راستہ, rc);
  واپس;
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
ساکن صحیح jsonArgIsJsonb(sqlite3_قدر *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  اگر( sqlite3_قدر_type(pArg)!=SQLITE_BLOB ) واپس 0;
  p->aBlob = (u8*)sqlite3_قدر_blob(pArg);
  p->nBlob = (u32)sqlite3_قدر_بائٹیں(pArg);
  اگر( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonb_Payload_سائز(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    واپس 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  واپس 0;
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
ساکن JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_قدر *pArg,
  u32 flgs
){
  صحیح eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  أكد( ctx!=0 );
  eType = sqlite3_قدر_type(pArg);
  اگر( eType==SQLITE_NULL ){
    واپس 0;
  }
  اگر( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_تلاش(ctx, pArg);
  }
  اگر( pFromCache ){
    pFromCache->nJPRef++;
    اگر( (flgs & JSON_EDITABLE)==0 ){
      واپس pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, حجم(*p));
  اگر( p==0 ) جاؤ json_pfa_oom;
  memset(p, 0, حجم(*p));
  p->db = db;
  p->nJPRef = 1;
  اگر( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    اگر( p->aBlob==0 ) جاؤ json_pfa_oom;
    حافظہ_نقل(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    واپس p;
  }
  اگر( eType==SQLITE_BLOB ){
    اگر( jsonArgIsJsonb(pArg,p) ){
      اگر( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        جاؤ json_pfa_oom;
      }
      واپس p;
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
  p->zJson = (حرف*)sqlite3_قدر_متن(pArg);
  p->nJson = sqlite3_قدر_بائٹیں(pArg);
  اگر( db->mallocFailed ) جاؤ json_pfa_oom;
  اگر( p->nJson==0 ) جاؤ json_pfa_malformed;
  أكد( p->zJson!=0 );
  اگر( json_Convert_متن_مقصد_Blob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    اگر( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      واپس p;
    }ورنہ{
      jsonParseFree(p);
      واپس 0;
    }
  }ورنہ{
    صحیح isRCStr = sqlite3_قدر_Is_کا_Class(pArg, sqlite3RCStrUnref);
    صحیح rc;
    اگر( !isRCStr ){
      حرف *zNew = sqlite3RCStrNew( p->nJson );
      اگر( zNew==0 ) جاؤ json_pfa_oom;
      حافظہ_نقل(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }ورنہ{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_داخل_کرو(ctx, p);
    اگر( rc==SQLITE_NOMEM ) جاؤ json_pfa_oom;
    اگر( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      جاؤ rebuild_from_cache;
    }
  }
  واپس p;

json_pfa_malformed:
  اگر( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    واپس p;
  }ورنہ{
    jsonParseFree(p);
    sqlite3_نتیجہ_error(ctx, "malformed JSON", -1);
    واپس 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_نتیجہ_error_nomem(ctx);
  واپس 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
ساکن خالی jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  صحیح flgs;
  اگر( p->oom ){
    sqlite3_نتیجہ_error_nomem(ctx);
    واپس;
  }
  flgs = SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(ctx));
  اگر( flgs & JSON_BLOB ){
    اگر( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_نتیجہ_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }ورنہ{
      sqlite3_نتیجہ_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }ورنہ{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    json_Translate_Blob_مقصد_متن(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
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
ساکن خالی jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 i_آغاز,        /* Start rendering here */
  u32 i_انتہا,          /* Do not render this byte or any byte after this one */
  صحیح nIndent,       /* Indent by this many spaces */
  sqlite3_str *p_برآمد  /* Generate output into this sqlite3_str object */
){
  جب( i_آغاز<i_انتہا ){
    u32 i, n, nn, sz = 0;
    صحیح showContent = 1;
    u8 x = pParse->aBlob[i_آغاز] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(p_برآمد, "%5d:%*s", i_آغاز, nIndent, "");
    اگر( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonb_Payload_سائز(pParse, i_آغاز, &sz);
    اگر( nn==0 ) nn = 1;
    اگر( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    برائے(i=0; i<nn; i++){
      sqlite3_str_appendf(p_برآمد, " %02x", pParse->aBlob[i_آغاز+i]);
    }
    اگر( n==0 ){
      sqlite3_str_appendf(p_برآمد, "   ERROR invalid node size\n");
      i_آغاز = n==0 ? i_آغاز+1 : i_انتہا;
      جاری;
    }
    pParse->nBlob = savedNBlob;
    اگر( i_آغاز+n+sz>i_انتہا ){
      i_انتہا = i_آغاز+n+sz;
      اگر( i_انتہا>pParse->nBlob ){
        اگر( pParse->nBlobAlloc>0 && i_انتہا>pParse->nBlobAlloc ){
          i_انتہا = pParse->nBlobAlloc;
        }ورنہ{
          i_انتہا = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(p_برآمد,"  <-- ");
    انتخاب( x ){
      حالت JSONB_NULL:     sqlite3_str_appendall(p_برآمد,"null"); توڑو;
      حالت JSONB_TRUE:     sqlite3_str_appendall(p_برآمد,"true"); توڑو;
      حالت JSONB_FALSE:    sqlite3_str_appendall(p_برآمد,"false"); توڑو;
      حالت JSONB_INT:      sqlite3_str_appendall(p_برآمد,"int"); توڑو;
      حالت JSONB_INT5:     sqlite3_str_appendall(p_برآمد,"int5"); توڑو;
      حالت JSONB_FLOAT:    sqlite3_str_appendall(p_برآمد,"float"); توڑو;
      حالت JSONB_FLOAT5:   sqlite3_str_appendall(p_برآمد,"float5"); توڑو;
      حالت JSONB_متن:     sqlite3_str_appendall(p_برآمد,"text"); توڑو;
      حالت JSONB_TEXTJ:    sqlite3_str_appendall(p_برآمد,"textj"); توڑو;
      حالت JSONB_TEXT5:    sqlite3_str_appendall(p_برآمد,"text5"); توڑو;
      حالت JSONB_TEXTRAW:  sqlite3_str_appendall(p_برآمد,"textraw"); توڑو;
      حالت JSONB_ARRAY: {
        sqlite3_str_appendf(p_برآمد,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_آغاز+n, i_آغاز+n+sz, nIndent+2, p_برآمد);
        showContent = 0;
        توڑو;
      }
      حالت JSONB_OBJECT: {
        sqlite3_str_appendf(p_برآمد, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_آغاز+n, i_آغاز+n+sz, nIndent+2, p_برآمد);
        showContent = 0;
        توڑو;
      }
      طےشدہ: {
        sqlite3_str_appendall(p_برآمد, "ERROR: unknown node type\n");
        showContent = 0;
        توڑو;
      }
    }
    اگر( showContent ){
      اگر( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(p_برآمد, "\n", 1);
      }ورنہ{
        u32 j;
        sqlite3_str_appendall(p_برآمد, ": \"");
        برائے(j=i_آغاز+n; j<i_آغاز+n+sz; j++){
          u8 c = pParse->aBlob[j];
          اگر( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(p_برآمد, (حرف*)&c, 1);
        }
        sqlite3_str_append(p_برآمد, "\"\n", 2);
      }
    }
    i_آغاز += n + sz;
  }
}
ساکن خالی jsonShowParse(JsonParse *pParse){
  sqlite3_str برآمد;
  حرف zBuf[1000];
  اگر( pParse==0 ){
    لکھو("NULL pointer\n");
    واپس;
  }ورنہ{
    لکھو("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    لکھو("nBlob = %u\n", pParse->nBlob);
    لکھو("delta = %d\n", pParse->delta);
    اگر( pParse->nBlob==0 ) واپس;
    لکھو("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&برآمد, 0, zBuf, حجم(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &برآمد);
  لکھو("%s", sqlite3_str_قدر(&برآمد));
  sqlite3_str_reset(&برآمد);
}
#اگر_ختم /* SQLITE_DEBUG */

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** SQL function:   json_parse(JSON)
**
** Parse JSON using jsonParseFuncArg().  Return text that is a
** human-readable dump of the binary JSONB for the input parameter.
*/
ساکن خالی jsonParseFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str برآمد;

  أكد( argc>=1 );
  sqlite3StrAccumInit(&برآمد, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( p==0 ) واپس;
  اگر( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &برآمد);
    sqlite3_نتیجہ_text64(ctx,برآمد.z_متن,برآمد.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }ورنہ{
    jsonShowParse(p);
  }
  jsonParseFree(p);
  sqlite3_str_reset(&برآمد);
}
#اگر_ختم /* SQLITE_DEBUG */

/****************************************************************************
** Scalar SQL function implementations
****************************************************************************/

/*
** Implementation of the json_quote(VALUE) function.  Return a JSON value
** corresponding to the SQL value input.  Mostly this means putting
** double-quotes around strings and returning the unquoted string "null"
** when given a NULL input.
*/
ساکن خالی jsonQuoteFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonString jx;
  غیر_مستعمل_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_قدر(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
ساکن خالی jsonArrayFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  برائے(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_قدر(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
ساکن خالی json_Array_لمبائی_Func(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( p==0 ) واپس;
  اگر( argc==2 ){
    ثابت حرف *z_راستہ = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
    اگر( z_راستہ==0 ){
      jsonParseFree(p);
      واپس;
    }
    i = json_Lookup_قدم(p, 0, z_راستہ[0]=='$' ? z_راستہ+1 : "@", 0);
    اگر( JSON_LOOKUP_ISERROR(i) ){
      اگر( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ورنہ{
        json_Bad_راستہ_Error(ctx, z_راستہ, i);
      }
      eErr = 1;
      i = 0;
    }
  }ورنہ{
    i = 0;
  }
  اگر( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_گنتی(p, i);
  }
  اگر( !eErr ) sqlite3_نتیجہ_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
ساکن صحیح jsonAllAlphanum(ثابت حرف *z, صحیح n){
  صحیح i;
  برائے(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  واپس i==n;
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
ساکن خالی json_نکالو_Func(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonParse *p = 0;      /* The parse */
  صحیح flags;             /* Flags associated with the function */
  صحیح i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  اگر( argc<2 ) واپس;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( p==0 ) واپس;
  flags = SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(ctx));
  jsonStringInit(&jx, ctx);
  اگر( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  برائے(i=1; i<argc; i++){
    /* With a single PATH argument */
    ثابت حرف *z_راستہ = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
    صحیح n_راستہ;
    u32 j;
    اگر( z_راستہ==0 ) جاؤ json_نکالو_error;
    n_راستہ = sqlite3Strlen30(z_راستہ);
    اگر( z_راستہ[0]=='$' ){
      j = json_Lookup_قدم(p, 0, z_راستہ+1, 0);
    }ورنہ اگر( (flags & JSON_ABPATH) ){
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
      اگر( sqlite3_قدر_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        اگر( z_راستہ[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, z_راستہ, n_راستہ);
        jsonAppendRawNZ(&jx, "]", 2);
      }ورنہ اگر( jsonAllAlphanum(z_راستہ, n_راستہ) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, z_راستہ, n_راستہ);
      }ورنہ اگر( z_راستہ[0]=='[' && n_راستہ>=3 && z_راستہ[n_راستہ-1]==']' ){
        jsonAppendRaw(&jx, z_راستہ, n_راستہ);
      }ورنہ{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, z_راستہ, n_راستہ);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = json_Lookup_قدم(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }ورنہ{
      json_Bad_راستہ_Error(ctx, z_راستہ, 0);
      جاؤ json_نکالو_error;
    }
    اگر( j<p->nBlob ){
      اگر( argc==2 ){
        اگر( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          json_Translate_Blob_مقصد_متن(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          أكد( (flags & JSON_BLOB)==0 );
          sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
        }ورنہ{
          jsonReturnFromBlob(p, j, ctx, 0);
          اگر( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }ورنہ{
        jsonAppendSeparator(&jx);
        json_Translate_Blob_مقصد_متن(p, j, &jx);
      }
    }ورنہ اگر( j==JSON_LOOKUP_NOTFOUND ){
      اگر( argc==2 ){
        جاؤ json_نکالو_error;  /* Return NULL if not found */
      }ورنہ{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }ورنہ{
      json_Bad_راستہ_Error(ctx, z_راستہ, j);
      جاؤ json_نکالو_error;
    }
  }
  اگر( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    اگر( (flags & JSON_BLOB)==0 ){
      sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_نکالو_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  واپس;
}

/*
** Return codes for jsonMergePatch()
*/
#تعریف_کرو JSON_ملاؤ_OK          0     /* Success */
#تعریف_کرو JSON_ملاؤ_BADTARGET   1     /* Malformed TARGET blob */
#تعریف_کرو JSON_ملاؤ_BADPATCH    2     /* Malformed PATCH blob */
#تعریف_کرو JSON_ملاؤ_OOM         3     /* Out-of-memory condition */
#تعریف_کرو JSON_ملاؤ_TOODEEP     4     /* Nested too deep */

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
ساکن صحیح json_ملاؤ_Patch(
  JsonParse *p_ہدف,      /* The JSON parser that contains the TARGET */
  u32 i_ہدف,             /* Index of TARGET in pTarget->aBlob[] */
  ثابت JsonParse *pPatch, /* The PATCH */
  u32 iPatch,              /* Index of PATCH in pPatch->aBlob[] */
  u32 i_گہرائی               /* Nesting depth */
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

  أكد( i_ہدف>=0 && i_ہدف<p_ہدف->nBlob );
  أكد( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  اگر( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 sz_ہدف;       /* Total size of the target, header+payload */
    n = jsonb_Payload_سائز(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonb_Payload_سائز(p_ہدف, i_ہدف, &sz);
    sz_ہدف = n+sz;
    jsonBlobEdit(p_ہدف, i_ہدف, sz_ہدف, pPatch->aBlob+iPatch, szPatch);
    واپس p_ہدف->oom ? JSON_ملاؤ_OOM : JSON_ملاؤ_OK;  /* Line 03 */
  }
  x = p_ہدف->aBlob[i_ہدف] & 0x0f;
  اگر( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonb_Payload_سائز(p_ہدف, i_ہدف, &sz);
    jsonBlobEdit(p_ہدف, i_ہدف+n, sz, 0, 0);
    x = p_ہدف->aBlob[i_ہدف];
    p_ہدف->aBlob[i_ہدف] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonb_Payload_سائز(pPatch, iPatch, &sz);
  اگر( NEVER(n==0) ) واپس JSON_ملاؤ_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonb_Payload_سائز(p_ہدف, i_ہدف, &sz);
  اگر( NEVER(n==0) ) واپس JSON_ملاؤ_BADTARGET;
  iTStart = i_ہدف+n;
  iTEndBE = iTStart+sz;

  جب( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    اگر( ePLabel<JSONB_متن || ePLabel>JSONB_TEXTRAW ){
      واپس JSON_ملاؤ_BADPATCH;
    }
    nPLabel = jsonb_Payload_سائز(pPatch, iPCursor, &szPLabel);
    اگر( nPLabel==0 ) واپس JSON_ملاؤ_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    اگر( iPValue>=iPEnd ) واپس JSON_ملاؤ_BADPATCH;
    nPValue = jsonb_Payload_سائز(pPatch, iPValue, &szPValue);
    اگر( nPValue==0 ) واپس JSON_ملاؤ_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    اگر( iPCursor>iPEnd ) واپس JSON_ملاؤ_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + p_ہدف->delta;
    جب( iTCursor<iTEnd ){
      صحیح isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = p_ہدف->aBlob[iTCursor] & 0x0f;
      اگر( eTLabel<JSONB_متن || eTLabel>JSONB_TEXTRAW ){
        واپس JSON_ملاؤ_BADTARGET;
      }
      nTLabel = jsonb_Payload_سائز(p_ہدف, iTCursor, &szTLabel);
      اگر( nTLabel==0 ) واپس JSON_ملاؤ_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      اگر( iTValue>=iTEnd ) واپس JSON_ملاؤ_BADTARGET;
      nTValue = jsonb_Payload_سائز(p_ہدف, iTValue, &szTValue);
      اگر( nTValue==0 ) واپس JSON_ملاؤ_BADTARGET;
      اگر( iTValue + nTValue + szTValue > iTEnd ) واپس JSON_ملاؤ_BADTARGET;
      isEqual = jsonLabelCompare(
                   (ثابت حرف*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_متن || ePLabel==JSONB_TEXTRAW),
                   (ثابت حرف*)&p_ہدف->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_متن || eTLabel==JSONB_TEXTRAW));
      اگر( isEqual ) توڑو;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    اگر( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      اگر( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(p_ہدف, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        اگر( NEVER(p_ہدف->oom) ) واپس JSON_ملاؤ_OOM;
      }ورنہ{
        /* Algorithm line 12 */
        صحیح rc, savedDelta = p_ہدف->delta;
        p_ہدف->delta = 0;
        اگر( i_گہرائی>=JSON_MAX_گہرائی ) واپس JSON_ملاؤ_TOODEEP;
        rc = json_ملاؤ_Patch(p_ہدف, iTValue, pPatch, iPValue, i_گہرائی+1);
        اگر( rc ) واپس rc;
        p_ہدف->delta += savedDelta;
      }        
    }ورنہ اگر( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      اگر( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(p_ہدف, iTEnd, 0, 0, szPValue+nPValue+szNew);
        اگر( p_ہدف->oom ) واپس JSON_ملاؤ_OOM;
        حافظہ_نقل(&p_ہدف->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        حافظہ_نقل(&p_ہدف->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }ورنہ{
        صحیح rc, savedDelta;
        jsonBlobEdit(p_ہدف, iTEnd, 0, 0, szNew+1);
        اگر( p_ہدف->oom ) واپس JSON_ملاؤ_OOM;
        حافظہ_نقل(&p_ہدف->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        p_ہدف->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = p_ہدف->delta;
        p_ہدف->delta = 0;
        اگر( i_گہرائی>=JSON_MAX_گہرائی ) واپس JSON_ملاؤ_TOODEEP;
        rc = json_ملاؤ_Patch(p_ہدف, iTEnd+szNew,pPatch,iPValue,i_گہرائی+1);
        اگر( rc ) واپس rc;
        p_ہدف->delta += savedDelta;
      }
    }
  }
  اگر( p_ہدف->delta ) json_After_Edit_سائز_Adjust(p_ہدف, i_ہدف);
  واپس p_ہدف->oom ? JSON_ملاؤ_OOM : JSON_ملاؤ_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
ساکن خالی jsonPatchFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonParse *p_ہدف;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  صحیح rc;                /* Result code */

  غیر_مستعمل_PARAMETER(argc);
  أكد( argc==2 );
  p_ہدف = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  اگر( p_ہدف==0 ) واپس;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  اگر( pPatch ){
    rc = json_ملاؤ_Patch(p_ہدف, 0, pPatch, 0, 0);
    اگر( rc==JSON_ملاؤ_OK ){
      jsonReturnParse(ctx, p_ہدف);
    }ورنہ اگر( rc==JSON_ملاؤ_OOM ){
      sqlite3_نتیجہ_error_nomem(ctx);
    }ورنہ اگر( rc==JSON_ملاؤ_TOODEEP ){
      sqlite3_نتیجہ_error(ctx, "JSON nested too deep", -1);
    }ورنہ{
      sqlite3_نتیجہ_error(ctx, "malformed JSON", -1);
    }
    jsonParseFree(pPatch);
  }
  jsonParseFree(p_ہدف);
}


/*
** Implementation of the json_object(NAME,VALUE,...) function.  Return a JSON
** object that contains all name/value given in arguments.  Or if any name
** is not a string or if any value is a BLOB, throw an error.
*/
ساکن خالی jsonObjectFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح i;
  JsonString jx;
  ثابت حرف *z;
  u32 n;

  اگر( argc&1 ){
    sqlite3_نتیجہ_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    واپس;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  برائے(i=0; i<argc; i+=2){
    اگر( sqlite3_قدر_type(argv[i])!=SQLITE_متن ){
      sqlite3_نتیجہ_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      واپس;
    }
    jsonAppendSeparator(&jx);
    z = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
    n = sqlite3_قدر_بائٹیں(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_قدر(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
}


/*
** json_remove(JSON, PATH, ...)
**
** Remove the named elements from JSON and return the result.  malformed
** JSON or PATH arguments result in an error.
*/
ساکن خالی jsonRemoveFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;          /* The parse */
  ثابت حرف *z_راستہ = 0; /* Path of element to be removed */
  صحیح i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  اگر( argc<1 ) واپس;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  اگر( p==0 ) واپس;
  برائے(i=1; i<argc; i++){
    z_راستہ = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
    اگر( z_راستہ==0 ){
      جاؤ json_remove_مکمل;
    }
    اگر( z_راستہ[0]!='$' ){
      جاؤ json_remove_patherror;
    }
    اگر( z_راستہ[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      جاؤ json_remove_مکمل;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = json_Lookup_قدم(p, 0, z_راستہ+1, 0);
    اگر( JSON_LOOKUP_ISERROR(rc) ){
      اگر( rc==JSON_LOOKUP_NOTFOUND ){
        جاری;  /* No-op */
      }ورنہ{
        json_Bad_راستہ_Error(ctx, z_راستہ, rc);
      }
      جاؤ json_remove_مکمل;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  واپس;

json_remove_patherror:
  json_Bad_راستہ_Error(ctx, z_راستہ, 0);

json_remove_مکمل:
  jsonParseFree(p);
  واپس;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
ساکن خالی jsonReplaceFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  اگر( argc<1 ) واپس;
  اگر( (argc&1)==0 ) {
    json_Wrong_عدد_دلائل(ctx, "replace");
    واپس;
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
ساکن خالی json_سیٹ_Func(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح flags = SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(ctx));
  صحیح eInsType = JSON_داخل_کرو_TYPE(flags);
  ساکن ثابت حرف *azInsType[] = { "insert", "set", "array_insert" };
  ساکن ثابت u8 aEditType[] = { JEDIT_INS, JEDIT_سیٹ, JEDIT_AINS };

  اگر( argc<1 ) واپس;
  أكد( eInsType>=0 && eInsType<=2 );
  اگر( (argc&1)==0 ) {
    json_Wrong_عدد_دلائل(ctx, azInsType[eInsType]);
    واپس;
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
ساکن خالی jsonTypeFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;          /* The parse */
  ثابت حرف *z_راستہ = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( p==0 ) واپس;
  اگر( argc==2 ){
    z_راستہ = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
    اگر( z_راستہ==0 ) جاؤ json_type_مکمل;
    اگر( z_راستہ[0]!='$' ){
      json_Bad_راستہ_Error(ctx, z_راستہ, 0);
      جاؤ json_type_مکمل;
    }
    i = json_Lookup_قدم(p, 0, z_راستہ+1, 0);
    اگر( JSON_LOOKUP_ISERROR(i) ){
      اگر( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }ورنہ{
        json_Bad_راستہ_Error(ctx, z_راستہ, i);
      }
      جاؤ json_type_مکمل;
    }
  }ورنہ{
    i = 0;
  }
  sqlite3_نتیجہ_متن(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
json_type_مکمل:
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
ساکن خالی jsonPrettyFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, حجم(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  اگر( x.pParse==0 ) واپس;
  x.p_برآمد = &s;
  jsonStringInit(&s, ctx);
  اگر( argc==1 || (x.zIndent = (ثابت حرف*)sqlite3_قدر_متن(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }ورنہ{
    x.szIndent = (u32)لڑی_لمبائی(x.zIndent);
  }
  json_Translate_Blob_مقصد_Pretty_متن(&x, 0);
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
ساکن خالی jsonValidFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  اگر( argc==2 ){
    i64 f = sqlite3_قدر_int64(argv[1]);
    اگر( f<1 || f>15 ){
      sqlite3_نتیجہ_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      واپس;
    }
    flags = f & 0x0f;
  }
  انتخاب( sqlite3_قدر_type(argv[0]) ){
    حالت SQLITE_NULL: {
#اگر_تعریف_شدہ SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_نتیجہ_int(ctx, 0);
#اگر_ختم
      واپس;
    }
    حالت SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, حجم(py));
      اگر( jsonArgIsJsonb(argv[0], &py) ){
        اگر( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }ورنہ اگر( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        توڑو;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    طےشدہ: {
      JsonParse px;
      اگر( (flags & 0x3)==0 ) توڑو;
      memset(&px, 0, حجم(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      اگر( p ){
        اگر( p->oom ){
          sqlite3_نتیجہ_error_nomem(ctx);
        }ورنہ اگر( p->nErr ){
          /* no-op */
        }ورنہ اگر( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }ورنہ{
        sqlite3_نتیجہ_error_nomem(ctx);
      }
      توڑو;
    }
  }
  sqlite3_نتیجہ_int(ctx, res);
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
ساکن خالی jsonErrorFunc(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  أكد( argc==1 );
  غیر_مستعمل_PARAMETER(argc);
  memset(&s, 0, حجم(s));
  s.db = sqlite3_context_db_handle(ctx);
  اگر( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }ورنہ{
    s.zJson = (حرف*)sqlite3_قدر_متن(argv[0]);
    اگر( s.zJson==0 ) واپس;  /* NULL input or OOM */
    s.nJson = sqlite3_قدر_بائٹیں(argv[0]);
    اگر( json_Convert_متن_مقصد_Blob(&s,0) ){
      اگر( s.oom ){
        iErrPos = -1;
      }ورنہ{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        أكد( s.zJson!=0 );  /* Because s.oom is false */
        برائے(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          اگر( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  اگر( iErrPos<0 ){
    sqlite3_نتیجہ_error_nomem(ctx);
  }ورنہ{
    sqlite3_نتیجہ_int64(ctx, iErrPos);
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
ساکن خالی json_Array_قدم(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonString *pStr;
  غیر_مستعمل_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, حجم(*pStr));
  اگر( pStr ){
    اگر( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }ورنہ اگر( pStr->n_مستعمل>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_قدر(pStr, argv[0]);
  }
}
ساکن خالی jsonArrayCompute(sqlite3_context *ctx, صحیح isFinal){
  JsonString *pStr;
  صحیح flags = SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  اگر( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    اگر( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      واپس;
    }ورنہ اگر( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      اگر( isFinal ){
        اگر( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ورنہ{
        jsonStringTrimOneChar(pStr);
      }
      واپس;
    }ورنہ اگر( isFinal ){
      sqlite3_نتیجہ_متن(ctx, pStr->zBuf, (صحیح)pStr->n_مستعمل,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ورنہ{
      sqlite3_نتیجہ_متن(ctx, pStr->zBuf, (صحیح)pStr->n_مستعمل, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }ورنہ اگر( flags & JSON_BLOB ){
    ساکن ثابت u8 خالی_خانے_Array = 0x0b;
    sqlite3_نتیجہ_blob(ctx, &خالی_خانے_Array, 1, SQLITE_STATIC);
  }ورنہ{
    sqlite3_نتیجہ_متن(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
}
ساکن خالی json_Array_قدر(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
ساکن خالی jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
ساکن خالی json_گروہ_Inverse(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  غیرعلامتی صحیح i;
  صحیح اندر_Str = 0;
  صحیح nNest = 0;
  حرف *z;
  حرف c;
  JsonString *pStr;
  غیر_مستعمل_PARAMETER(argc);
  غیر_مستعمل_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  اگر( NEVER(!pStr) ) واپس;
  z = pStr->zBuf;
  برائے(i=1; i<pStr->n_مستعمل && ((c = z[i])!=',' || اندر_Str || nNest); i++){
    اگر( c=='"' ){
      اندر_Str = !اندر_Str;
    }ورنہ اگر( c=='\\' ){
      i++;
    }ورنہ اگر( !اندر_Str ){
      اگر( c=='{' || c=='[' ) nNest++;
      اگر( c=='}' || c==']' ) nNest--;
    }
  }
  اگر( i<pStr->n_مستعمل ){
    pStr->n_مستعمل -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->n_مستعمل-1);
    z[pStr->n_مستعمل] = 0;
  }ورنہ{
    pStr->n_مستعمل = 1;
  }
}
#ورنہ
# define json_گروہ_Inverse 0
#اگر_ختم


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
ساکن خالی json_Object_قدم(
  sqlite3_context *ctx,
  صحیح argc,
  sqlite3_قدر **argv
){
  JsonString *pStr;
  ثابت حرف *z;
  u32 n;
  غیر_مستعمل_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, حجم(*pStr));
  اگر( pStr ){
    z = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    n = sqlite3Strlen30(z);
    اگر( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }ورنہ اگر( pStr->n_مستعمل>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    اگر( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_قدر(pStr, argv[1]);
    }ورنہ{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
ساکن خالی jsonObjectCompute(sqlite3_context *ctx, صحیح isFinal){
  JsonString *pStr;
  صحیح flags = SQLITE_PTR_مقصد_INT(sqlite3_user_ڈیٹا(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  اگر( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    اگر( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      واپس;
    }
    اگر( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      صحیح اندر_Str = 0;
      اگر( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->n_مستعمل+1);
        pStr = &tmpStr;
        اگر( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          واپس;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      برائے(i=j=1; i<pStr->n_مستعمل; i++){
        حرف c = pStr->zBuf[i];
        اگر( c=='"' ){
          اندر_Str = !اندر_Str;
          pStr->zBuf[j++] = '"';
        }ورنہ اگر( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }ورنہ اگر( c=='@' && !اندر_Str ){
          أكد( i+1<pStr->n_مستعمل );
          اگر( pStr->zBuf[i+1]==',' ){
            i++;
          }ورنہ اگر( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }ورنہ{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->n_مستعمل = j;    /* Truncate the string */
    }
    اگر( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      اگر( isFinal ){
        اگر( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }ورنہ{
        jsonStringTrimOneChar(pOgStr);
      }
    }ورنہ اگر( isFinal ){
      sqlite3_نتیجہ_متن(ctx, pStr->zBuf, (صحیح)pStr->n_مستعمل,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }ورنہ{
      sqlite3_نتیجہ_متن(ctx, pStr->zBuf, (صحیح)pStr->n_مستعمل, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    اگر( pStr!=pOgStr ) jsonStringReset(pStr);
  }ورنہ اگر( flags & JSON_BLOB ){
    ساکن ثابت غیرعلامتی حرف خالی_خانے_Object = 0x0c;
    sqlite3_نتیجہ_blob(ctx, &خالی_خانے_Object, 1, SQLITE_STATIC); 
  }ورنہ{
    sqlite3_نتیجہ_متن(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
}
ساکن خالی json_Object_قدر(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
ساکن خالی jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
قسمتعریف ساخت Json_والد Json_والد;
ساخت Json_والد {
  u32 i_سر;                 /* Start of object or array */
  u32 i_قدر;                /* Start of the value */
  u32 i_انتہا;                  /* First byte past the end */
  u32 n_راستہ;                 /* Length of path */
  i64 i_کلید;                  /* Key for JSONB_ARRAY */
};

قسمتعریف ساخت JsonEachCursor JsonEachCursor;
ساخت JsonEachCursor {
  sqlite3_vtab_cursor بنیاد;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 i_انتہا;                  /* EOF when i equals or exceeds this value */
  u32 n_جڑ;                 /* Size of the root path in bytes */
  u8 eType;                  /* Type of the container for element i */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
  u8 e_منوال;                  /* 1 for json_each().  2 for jsonb_each() */
  u32 n_والد;               /* Current nesting depth */
  u32 n_والد_Alloc;          /* Space allocated for aParent[] */
  Json_والد *a_والد;       /* Parent elements of i */
  sqlite3 *db;               /* Database connection */
  JsonString راستہ;           /* Current path */
  JsonParse sParse;          /* Parse of the input JSON */
};
قسمتعریف ساخت JsonEachConnection JsonEachConnection;
ساخت JsonEachConnection {
  sqlite3_vtab بنیاد;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 e_منوال;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
ساکن صحیح jsonEachConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  JsonEachConnection *pNew;
  صحیح rc;

/* Column numbers */
#تعریف_کرو JEACH_کلید     0
#تعریف_کرو JEACH_قدر   1
#تعریف_کرو JEACH_TYPE    2
#تعریف_کرو JEACH_ATOM    3
#تعریف_کرو JEACH_ID      4
#تعریف_کرو JEACH_والد  5
#تعریف_کرو JEACH_FULLKEY 6
#تعریف_کرو JEACH_راستہ    7
/* The xBestIndex method assumes that the JSON and ROOT columns are
** the last two columns in the table.  Should this ever changes, be
** sure to update the xBestIndex method. */
#تعریف_کرو JEACH_JSON    8
#تعریف_کرو JEACH_جڑ    9

  غیر_مستعمل_PARAMETER(pzErr);
  غیر_مستعمل_PARAMETER(argv);
  غیر_مستعمل_PARAMETER(argc);
  غیر_مستعمل_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  اگر( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, حجم(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    اگر( pNew==0 ) واپس SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->e_منوال = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->e_منوال]=='t';
  }
  واپس rc;
}

/* destructor for json_each virtual table */
ساکن صحیح jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  واپس SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
ساکن صحیح jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  غیر_مستعمل_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, حجم(*pCur));
  اگر( pCur==0 ) واپس SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->e_منوال = pVtab->e_منوال;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->راستہ);
  *ppCursor = &pCur->بنیاد;
  واپس SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
ساکن خالی jsonEachCursorReset(JsonEachCursor *p){
  jsonParseReset(&p->sParse);
  jsonStringReset(&p->راستہ);
  sqlite3DbFree(p->db, p->a_والد);
  p->iRowid = 0;
  p->i = 0;
  p->a_والد = 0;
  p->n_والد = 0;
  p->n_والد_Alloc = 0;
  p->i_انتہا = 0;
  p->eType = 0;
}

/* Destructor for a jsonEachCursor object */
ساکن صحیح jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  واپس SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
ساکن صحیح jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  واپس p->i >= p->i_انتہا;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
ساکن صحیح jsonSkipLabel(JsonEachCursor *p){
  اگر( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonb_Payload_سائز(&p->sParse, p->i, &sz);
    sz += p->i + n;
    اگر( sz >= p->sParse.nBlob ) sz = p->i;
    واپس sz;
  }ورنہ{
    واپس p->i;
  }
}

/*
** Append the path name for the current element.
*/
ساکن خالی json_Append_راستہ_Name(JsonEachCursor *p){
  أكد( p->n_والد>0 );
  أكد( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  اگر( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->راستہ, "[%lld]", p->a_والد[p->n_والد-1].i_کلید);
  }ورنہ{
    u32 n, sz = 0, k, i;
    ثابت حرف *z;
    صحیح needQuote = 0;
    n = jsonb_Payload_سائز(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (ثابت حرف*)&p->sParse.aBlob[k];
    اگر( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }ورنہ{
      برائے(i=0; i<sz; i++){
        اگر( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          توڑو;
        }
      }
    }
    اگر( needQuote ){
      jsonPrintf(sz+4,&p->راستہ,".\"%.*s\"", sz, z);
    }ورنہ{
      jsonPrintf(sz+2,&p->راستہ,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
ساکن صحیح json_Each_Malformed_داخلہ(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_پیغام);
  cur->pVtab->z_Err_پیغام = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  واپس cur->pVtab->z_Err_پیغام ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
ساکن صحیح json_Each_اگلا(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  صحیح rc = SQLITE_OK;
  اگر( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonb_Payload_سائز(&p->sParse, i, &sz);
    اگر( n==0 )واپس json_Each_Malformed_داخلہ(cur);
    اگر( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      Json_والد *p_والد;
      اگر( p->n_والد>=p->n_والد_Alloc ){
        Json_والد *pNew;
        u64 nNew;
        nNew = p->n_والد_Alloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->a_والد, حجم(Json_والد)*nNew);
        اگر( pNew==0 ) واپس SQLITE_NOMEM;
        p->n_والد_Alloc = (u32)nNew;
        p->a_والد = pNew;
      }
      levelChange = 1;
      p_والد = &p->a_والد[p->n_والد];
      p_والد->i_سر = p->i;
      p_والد->i_قدر = i;
      p_والد->i_انتہا = i + n + sz;
      p_والد->i_کلید = -1;
      p_والد->n_راستہ = (u32)p->راستہ.n_مستعمل;
      اگر( p->eType && p->n_والد ){
        json_Append_راستہ_Name(p);
        اگر( p->راستہ.eErr ) rc = SQLITE_NOMEM;
      }
      p->n_والد++;
      p->i = i + n;
    }ورنہ{
      p->i = i + n + sz;
    }
    جب( p->n_والد>0 && p->i >= p->a_والد[p->n_والد-1].i_انتہا ){
      p->n_والد--;
      p->راستہ.n_مستعمل = p->a_والد[p->n_والد].n_راستہ;
      levelChange = 1;
    }
    اگر( levelChange ){
      اگر( p->n_والد>0 ){
        Json_والد *p_والد = &p->a_والد[p->n_والد-1];
        u32 iVal = p_والد->i_قدر;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }ورنہ{
        p->eType = 0;
      }
    }
  }ورنہ{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonb_Payload_سائز(&p->sParse, i, &sz);
    اگر( n==0 )واپس json_Each_Malformed_داخلہ(cur);
    p->i = i + n + sz;
  }
  اگر( p->eType==JSONB_ARRAY && p->n_والد ){
    p->a_والد[p->n_والد-1].i_کلید++;
  }
  p->iRowid++;
  واپس rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
ساکن صحیح json_Each_راستہ_لمبائی(JsonEachCursor *p){
  u32 n = p->راستہ.n_مستعمل;
  حرف *z = p->راستہ.zBuf;
  اگر( p->iRowid==0 && p->bRecursive && n>=2 ){
    جب( ALWAYS(n>1) ){
      n--;
      اگر( z[n]=='[' || z[n]=='.' ){
        u32 x;
        حرف cSaved = z[n];
        z[n] = 0;
        أكد( p->sParse.eEdit==0 );
        x = json_Lookup_قدم(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        اگر( !JSON_LOOKUP_ISERROR(x) ) توڑو;
      }
    }
  }
  واپس n;
}

/* Return the value of a column */
ساکن صحیح json_Each_عمود(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحیح i_عمود                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  انتخاب( i_عمود ){
    حالت JEACH_کلید: {
      اگر( p->n_والد==0 ){
        u32 n, j;
        اگر( p->n_جڑ==1 ) توڑو;
        j = json_Each_راستہ_لمبائی(p);
        n = p->n_جڑ - j;
        اگر( n==0 ){
          توڑو;
        }ورنہ اگر( p->راستہ.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->راستہ.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_نتیجہ_int64(ctx, x);
        }ورنہ اگر( p->راستہ.zBuf[j+1]=='"' ){
          sqlite3_نتیجہ_متن(ctx, &p->راستہ.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }ورنہ{
          sqlite3_نتیجہ_متن(ctx, &p->راستہ.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        توڑو;
      }
      اگر( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }ورنہ{
        أكد( p->eType==JSONB_ARRAY );
        sqlite3_نتیجہ_int64(ctx, p->a_والد[p->n_والد-1].i_کلید);
      }
      توڑو;
    }
    حالت JEACH_قدر: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->e_منوال);
      اگر( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_نتیجہ_subtype(ctx, JSON_SUBTYPE);
      }
      توڑو;
    }
    حالت JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_نتیجہ_متن(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      توڑو;
    }
    حالت JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      اگر( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      توڑو;
    }
    حالت JEACH_ID: {
      sqlite3_نتیجہ_int64(ctx, (sqlite3_int64)p->i);
      توڑو;
    }
    حالت JEACH_والد: {
      اگر( p->n_والد>0 && p->bRecursive ){
        sqlite3_نتیجہ_int64(ctx, p->a_والد[p->n_والد-1].i_سر);
      }
      توڑو;
    }
    حالت JEACH_FULLKEY: {
      u64 n_بنیاد = p->راستہ.n_مستعمل;
      اگر( p->n_والد ) json_Append_راستہ_Name(p);
      sqlite3_نتیجہ_text64(ctx, p->راستہ.zBuf, p->راستہ.n_مستعمل,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->راستہ.n_مستعمل = n_بنیاد;
      توڑو;
    }
    حالت JEACH_راستہ: {
      u32 n = json_Each_راستہ_لمبائی(p);
      sqlite3_نتیجہ_text64(ctx, p->راستہ.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      توڑو;
    }
    طےشدہ: {
      sqlite3_نتیجہ_متن(ctx, p->راستہ.zBuf, p->n_جڑ, SQLITE_TRANSIENT);
      توڑو;
    }
    حالت JEACH_JSON: {
      اگر( p->sParse.zJson==0 ){
        sqlite3_نتیجہ_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }ورنہ{
        sqlite3_نتیجہ_متن(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      توڑو;
    }
  }
  واپس SQLITE_OK;
}

/* Return the current rowid value */
ساکن صحیح jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  واپس SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
ساکن صحیح json_Each_بہترین_اشاریہ(
  sqlite3_vtab *tab,
  sqlite3_اشاریہ_info *pIdxInfo
){
  صحیح i;                     /* Loop counter or computed array index */
  صحیح aIdx[2];               /* Index of constraints for JSON and ROOT */
  صحیح unusable_نقاب = 0;      /* Mask of unusable JSON and ROOT constraints */
  صحیح idx_نقاب = 0;           /* Mask of usable == constraints JSON and ROOT */
  ثابت ساخت sqlite3_اشاریہ_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  أكد( JEACH_جڑ == JEACH_JSON+1 );
  غیر_مستعمل_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  برائے(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    صحیح i_ستون;
    صحیح i_نقاب;
    اگر( pConstraint->i_عمود < JEACH_JSON ) جاری;
    i_ستون = pConstraint->i_عمود - JEACH_JSON;
    أكد( i_ستون==0 || i_ستون==1 );
    testcase( i_ستون==0 );
    i_نقاب = 1 << i_ستون;
    اگر( pConstraint->usable==0 ){
      unusable_نقاب |= i_نقاب;
    }ورنہ اگر( pConstraint->op==SQLITE_اشاریہ_CONSTRAINT_EQ ){
      aIdx[i_ستون] = i;
      idx_نقاب |= i_نقاب;
    }
  }
  اگر( pIdxInfo->n_ترتیب_By>0
   && pIdxInfo->a_ترتیب_By[0].i_عمود<0
   && pIdxInfo->a_ترتیب_By[0].desc==0
  ){
    pIdxInfo->ترتیب_By_Consumed = 1;
  }

  اگر( (unusable_نقاب & ~idx_نقاب)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    واپس SQLITE_CONSTRAINT;
  }
  اگر( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_عدد = 0;
  }ورنہ{
    pIdxInfo->estimated_لاگت = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argv_اشاریہ = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    اگر( aIdx[1]<0 ){
      pIdxInfo->idx_عدد = 1;  /* Only JSON supplied.  Plan 1 */
    }ورنہ{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argv_اشاریہ = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_عدد = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  واپس SQLITE_OK;
}

/* Start a search on a new JSON string */
ساکن صحیح jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ثابت حرف *z_جڑ = 0;
  u32 i, n, sz;

  غیر_مستعمل_PARAMETER(idxStr);
  غیر_مستعمل_PARAMETER(argc);
  jsonEachCursorReset(p);
  اگر( idx_عدد==0 ) واپس SQLITE_OK;
  memset(&p->sParse, 0, حجم(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  اگر( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }ورنہ{
    p->sParse.zJson = (حرف*)sqlite3_قدر_متن(argv[0]);
    p->sParse.nJson = sqlite3_قدر_بائٹیں(argv[0]);
    اگر( p->sParse.zJson==0 ){
      p->i = p->i_انتہا = 0;
      واپس SQLITE_OK;
    }      
    اگر( json_Convert_متن_مقصد_Blob(&p->sParse, 0) ){
      اگر( p->sParse.oom ){
        واپس SQLITE_NOMEM;
      }
      واپس json_Each_Malformed_داخلہ(cur);
    }
  }
  اگر( idx_عدد==3 ){
    z_جڑ = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
    اگر( z_جڑ==0 ) واپس SQLITE_OK;
    اگر( z_جڑ[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_پیغام);
      cur->pVtab->z_Err_پیغام = json_Bad_راستہ_Error(0, z_جڑ, 0);
      jsonEachCursorReset(p);
      واپس cur->pVtab->z_Err_پیغام ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_جڑ = sqlite3Strlen30(z_جڑ);
    اگر( z_جڑ[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }ورنہ{
      i = json_Lookup_قدم(&p->sParse, 0, z_جڑ+1, 0);
      اگر( JSON_LOOKUP_ISERROR(i) ){
        اگر( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->i_انتہا = 0;
          واپس SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_پیغام);
        cur->pVtab->z_Err_پیغام = json_Bad_راستہ_Error(0, z_جڑ, 0);
        jsonEachCursorReset(p);
        واپس cur->pVtab->z_Err_پیغام ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      اگر( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }ورنہ{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->راستہ, z_جڑ, p->n_جڑ);
  }ورنہ{
    i = p->i = 0;
    p->eType = 0;
    p->n_جڑ = 1;
    jsonAppendRaw(&p->راستہ, "$", 1);
  }
  p->n_والد = 0;
  n = jsonb_Payload_سائز(&p->sParse, i, &sz);
  p->i_انتہا = i+n+sz;
  اگر( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->a_والد = sqlite3DbMallocZero(p->db, حجم(Json_والد));
    اگر( p->a_والد==0 ) واپس SQLITE_NOMEM;
    p->n_والد = 1;
    p->n_والد_Alloc = 1;
    p->a_والد[0].i_کلید = 0;
    p->a_والد[0].i_انتہا = p->i_انتہا;
    p->a_والد[0].i_سر = p->i;
    p->a_والد[0].i_قدر = i;
  }
  واپس SQLITE_OK;
}

/* The methods of the json_each virtual table */
ساکن sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  json_Each_بہترین_اشاریہ,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  json_Each_اگلا,              /* xNext - advance a cursor */
  jsonEachEof,               /* xEof - check for end of scan */
  json_Each_عمود,            /* xColumn - read data */
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
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */
#اگر_ختم /* !defined(SQLITE_OMIT_JSON) */

/*
** Register JSON functions.
*/
خالی sqlite3_رجسٹر_Json_Functions(خالی){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_JSON
  ساکن FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_داخل_کرو, -1,1,1, 1,0,JSON_AINS,  json_سیٹ_Func),
    JFUNCTION(jsonb_array_داخل_کرو,-1,1,0, 1,1,JSON_AINS,  json_سیٹ_Func),
    JFUNCTION(json_array_لمبائی,  1,1,0, 0,0,0,          json_Array_لمبائی_Func),
    JFUNCTION(json_array_لمبائی,  2,1,0, 0,0,0,          json_Array_لمبائی_Func),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_نکالو,      -1,1,1, 0,0,0,          json_نکالو_Func),
    JFUNCTION(jsonb_نکالو,     -1,1,0, 0,1,0,          json_نکالو_Func),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  json_نکالو_Func),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   json_نکالو_Func),
    JFUNCTION(json_داخل_کرو,       -1,1,1, 1,0,0,          json_سیٹ_Func),
    JFUNCTION(jsonb_داخل_کرو,      -1,1,0, 1,1,0,          json_سیٹ_Func),
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
    JFUNCTION(json_سیٹ,          -1,1,1, 1,0,JSON_ISSET, json_سیٹ_Func),
    JFUNCTION(jsonb_سیٹ,         -1,1,0, 1,1,JSON_ISSET, json_سیٹ_Func),
    JFUNCTION(json_type,          1,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_type,          2,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_valid,         1,1,0, 0,0,0,          jsonValidFunc),
    JFUNCTION(json_valid,         2,1,0, 0,0,0,          jsonValidFunc),
#اگر SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#اگر_ختم
    WAGGREGATE(json_گروہ_array,  1, 0, 0,
       json_Array_قدم, jsonArrayFinal, json_Array_قدر, json_گروہ_Inverse,
       SQLITE_SUBTYPE|SQLITE_نتیجہ_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_گروہ_array, 1, JSON_BLOB, 0,
       json_Array_قدم, jsonArrayFinal, json_Array_قدر, json_گروہ_Inverse,
       SQLITE_SUBTYPE|SQLITE_نتیجہ_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_گروہ_object, 2, 0, 0,
       json_Object_قدم, jsonObjectFinal, json_Object_قدر, json_گروہ_Inverse,
       SQLITE_SUBTYPE|SQLITE_نتیجہ_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_گروہ_object,2, JSON_BLOB, 0,
       json_Object_قدم, jsonObjectFinal, json_Object_قدر, json_گروہ_Inverse,
       SQLITE_SUBTYPE|SQLITE_نتیجہ_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_داخل_کرو_Builtin_Funcs(aJsonFunc, Array_سائز(aJsonFunc));
#اگر_ختم
}

#اگر  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3_Json_Vtab_رجسٹر(sqlite3 *db, ثابت حرف *zName){
  غیرعلامتی صحیح i;
  ساکن ثابت حرف *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  أكد( sqlite3_Hash_ڈھونڈو(&db->aModule, zName)==0 );
  برائے(i=0; i<حجم(azModule)/حجم(azModule[0]); i++){
    اگر( sqlite3StrICmp(azModule[i],zName)==0 ){
      واپس sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  واپس 0;
}
#اگر_ختم /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
