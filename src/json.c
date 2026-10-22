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
#अगर_अघोषित SQLITE_OMIT_JSON
#समावेश "sqliteInt.h"

/* JSONB element types
*/
#घोषणा JSONB_NULL     0   /* "null" */
#घोषणा JSONB_TRUE     1   /* "true" */
#घोषणा JSONB_FALSE    2   /* "false" */
#घोषणा JSONB_INT      3   /* integer acceptable to JSON and SQL */
#घोषणा JSONB_INT5     4   /* integer in 0x000 notation */
#घोषणा JSONB_FLOAT    5   /* float acceptable to JSON and SQL */
#घोषणा JSONB_FLOAT5   6   /* float with JSON5 extensions */
#घोषणा JSONB_TEXT     7   /* Text compatible with both JSON and SQL */
#घोषणा JSONB_TEXTJ    8   /* Text with JSON escapes */
#घोषणा JSONB_TEXT5    9   /* Text with JSON-5 escape */
#घोषणा JSONB_TEXTRAW 10   /* SQL text that needs escaping for JSON */
#घोषणा JSONB_ARRAY   11   /* An array */
#घोषणा JSONB_OBJECT  12   /* An object */

/* Human-readable names for the JSONB values.  The index for each
** string must correspond to the JSONB_* integer above.
*/
अटल स्थिर अक्षर * स्थिर jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
अटल स्थिर अक्षर jsonIsSpace[] = {
#अगर_घोषित SQLITE_ASCII
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
#पूर्ण_अगर
#अगर_घोषित SQLITE_EBCDIC
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
#पूर्ण_अगर

};
#घोषणा jsonIsspace(x) (jsonIsSpace[(अचिन्हित अक्षर)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#अगर_घोषित SQLITE_ASCII
अटल स्थिर अक्षर jsonSpaces[] = "\011\012\015\040";
#पूर्ण_अगर
#अगर_घोषित SQLITE_EBCDIC
अटल स्थिर अक्षर jsonSpaces[] = "\005\045\015\100";
#पूर्ण_अगर


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
अटल स्थिर अक्षर jsonIsOk[256] = {
#अगर_घोषित SQLITE_ASCII
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
#पूर्ण_अगर
#अगर_घोषित SQLITE_EBCDIC
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
#पूर्ण_अगर
};

/* Objects */
प्रकार काष्ठा JsonCache JsonCache;
प्रकार काष्ठा JsonString JsonString;
प्रकार काष्ठा JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#घोषणा JSON_CACHE_ID    (-429938)  /* Cache entry */
#घोषणा JSON_CACHE_आकार  4          /* Max number of cache entries */

/*
** jsonUnescapeOneChar() returns this invalid code point if it encounters
** a syntax error.
*/
#घोषणा JSON_INVALID_CHAR 0x99999

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
काष्ठा JsonCache {
  sqlite3 *db;                    /* Database connection */
  पूर्णांक nUsed;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_आकार];  /* One line for each cache entry */
};

/* An instance of this object represents a JSON string
** under construction.  Really, this is a generic string accumulator
** that can be and is used to create strings other than JSON.
**
** If the generated string is longer than will fit into the zSpace[] buffer,
** then it will be an RCStr string.  This aids with caching of large
** JSON strings.
*/
काष्ठा JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  अक्षर *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 nUsed;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  अक्षर zSpace[100];        /* Initial static space */
};

/* Allowed values for JsonString.eErr */
#घोषणा JSTRING_OOM         0x01   /* Out of memory */
#घोषणा JSTRING_MALFORMED   0x02   /* Malformed JSONB */
#घोषणा JSTRING_TOODEEP     0x04   /* JSON nested too deep */
#घोषणा JSTRING_ERR         0x08   /* Error already sent to sqlite3_result */

/* The "subtype" set for text JSON values passed through using
** sqlite3_result_subtype() and sqlite3_value_subtype().
*/
#घोषणा JSON_SUBTYPE  74    /* Ascii for "J" */

/*
** Bit values for the flags passed into various SQL function implementations
** via the sqlite3_user_data() value.
*/
#घोषणा JSON_JSON      0x01        /* Result is always JSON */
#घोषणा JSON_SQL       0x02        /* Result is always SQL */
#घोषणा JSON_ABPATH    0x03        /* Allow abbreviated JSON path specs */
#घोषणा JSON_ISSET     0x04        /* json_set(), not json_insert() */
#घोषणा JSON_AINS      0x08        /* json_array_insert(), not json_insert() */
#घोषणा JSON_BLOB      0x10        /* Use the BLOB output format */

#घोषणा JSON_जोड़ो_TYPE(X)  (((X)&0xC)>>2)


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
काष्ठा JsonParse {
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
#घोषणा JEDIT_DEL   1   /* Delete if exists */
#घोषणा JEDIT_REPL  2   /* Overwrite if exists */
#घोषणा JEDIT_INS   3   /* Insert if not exists */
#घोषणा JEDIT_SET   4   /* Insert or overwrite */
#घोषणा JEDIT_AINS  5   /* array_insert() */

/*
** Maximum nesting depth of JSON for this implementation.
**
** This limit is needed to avoid a stack overflow in the recursive
** descent parser.  A depth of 1000 is far deeper than any sane JSON
** should go.  Historical note: This limit was 2000 prior to version 3.42.0
*/
#अगर_अघोषित SQLITE_JSON_MAX_DEPTH
# define JSON_MAX_DEPTH  1000
#अन्यथा
# define JSON_MAX_DEPTH SQLITE_JSON_MAX_DEPTH
#पूर्ण_अगर

/*
** Allowed values for the flgs argument to jsonParseFuncArg();
*/
#घोषणा JSON_EDITABLE  0x01   /* Generate a writable JsonParse object */
#घोषणा JSON_KEEPERROR 0x02   /* Return non-NULL even if there is an error */
#घोषणा JSON_NOCACHE   0x04   /* Do not use the JSON cache */

/**************************************************************************
** Forward references
**************************************************************************/
अटल व्योम jsonReturnStringAsBlob(JsonString*);
अटल पूर्णांक jsonArgIsJsonb(sqlite3_मूल्य *pJson, JsonParse *p);
अटल u32 jsonTranslateBlobToText(JsonParse*,u32,JsonString*);
अटल व्योम jsonReturnParse(sqlite3_context*,JsonParse*);
अटल JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_मूल्य*,u32);
अटल व्योम jsonParseFree(JsonParse*);
अटल u32 jsonb_Payload_आकार(स्थिर JsonParse*, u32, u32*);
अटल u32 jsonUnescapeOneChar(स्थिर अक्षर*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
अटल व्योम jsonCacheDelete(JsonCache *p){
  पूर्णांक i;
  क्रम(i=0; i<p->nUsed; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
अटल व्योम jsonCacheDeleteGeneric(व्योम *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
अटल पूर्णांक json_Cache_जोड़ो(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  निश्चित( pParse->zJson!=0 );
  निश्चित( pParse->bJsonIsRCStr );
  निश्चित( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  अगर( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, माप(*p));
    अगर( p==0 ) वापस SQLITE_NOMEM;
    p->db = db;
    sqlite3_set_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    अगर( p==0 ) वापस SQLITE_NOMEM;
  }
  अगर( p->nUsed >= JSON_CACHE_आकार ){
    jsonParseFree(p->a[0]);
    स_हटाओ(p->a, &p->a[1], (JSON_CACHE_आकार-1)*माप(p->a[0]));
    p->nUsed = JSON_CACHE_आकार-1;
  }
  निश्चित( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  वापस SQLITE_OK;
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
अटल JsonParse *json_Cache_खोज(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_मूल्य *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  पूर्णांक i;
  स्थिर अक्षर *zJson;
  पूर्णांक nJson;

  अगर( sqlite3_मूल्य_type(pArg)!=SQLITE_TEXT ){
    वापस 0;
  }
  zJson = (स्थिर अक्षर*)sqlite3_मूल्य_text(pArg);
  अगर( zJson==0 ) वापस 0;
  nJson = sqlite3_मूल्य_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  अगर( p==0 ){
    वापस 0;
  }
  क्रम(i=0; i<p->nUsed; i++){
    अगर( p->a[i]->zJson==zJson ) अवरोध;
  }
  अगर( i>=p->nUsed ){
    क्रम(i=0; i<p->nUsed; i++){
      अगर( p->a[i]->nJson!=nJson ) जारी;
      अगर( स_भेद(p->a[i]->zJson, zJson, nJson)==0 ) अवरोध;
    }
  }
  अगर( i<p->nUsed ){
    अगर( i<p->nUsed-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      स_हटाओ(&p->a[i], &p->a[i+1], (p->nUsed-i-1)*माप(tmp));
      p->a[p->nUsed-1] = tmp;
      i = p->nUsed - 1;
    }
    निश्चित( p->a[i]->delta==0 );
    वापस p->a[i];
  }अन्यथा{
    वापस 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
अटल व्योम jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = माप(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
अटल व्योम jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
अटल व्योम jsonStringReset(JsonString *p){
  अगर( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
अटल व्योम jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  अगर( p->pCtx ) sqlite3_परिणाम_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
अटल व्योम jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  निश्चित( p->pCtx!=0 );
  sqlite3_परिणाम_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
अटल पूर्णांक jsonStringGrow(JsonString *p, u32 N){
  u64 n_योग = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  अक्षर *zNew;
  अगर( p->bStatic ){
    अगर( p->eErr ) वापस 1;
    zNew = sqlite3RCStrNew(n_योग);
    अगर( zNew==0 ){
      jsonStringOom(p);
      वापस SQLITE_NOMEM;
    }
    स_नकल(zNew, p->zBuf, (माप_प्रकार)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  }अन्यथा{
    p->zBuf = sqlite3RCStrResize(p->zBuf, n_योग);
    अगर( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      वापस SQLITE_NOMEM;
    }
  }
  p->nAlloc = n_योग;
  वापस SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
अटल SQLITE_NOINLINE व्योम jsonStringExpandAndAppend(
  JsonString *p,
  स्थिर अक्षर *zIn,
  u32 N
){
  निश्चित( N>0 );
  अगर( jsonStringGrow(p,N) ) वापस;
  स_नकल(p->zBuf+p->nUsed, zIn, N);
  p->nUsed += N;
}
अटल व्योम jsonAppendRaw(JsonString *p, स्थिर अक्षर *zIn, u32 N){
  अगर( N==0 ) वापस;
  अगर( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }अन्यथा{
    स_नकल(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}
अटल व्योम jsonAppendRawNZ(JsonString *p, स्थिर अक्षर *zIn, u32 N){
  निश्चित( N>0 );
  अगर( N+p->nUsed >= p->nAlloc ){
    jsonStringExpandAndAppend(p,zIn,N);
  }अन्यथा{
    स_नकल(p->zBuf+p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
अटल व्योम jsonPrintf(पूर्णांक N, JsonString *p, स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  अगर( (p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N) ) वापस;
  बहु_शुरू(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->nUsed, zFormat, ap);
  बहु_पूर्ण(ap);
  p->nUsed += (पूर्णांक)म_माप(p->zBuf+p->nUsed);
}

/* Append a single character
*/
अटल SQLITE_NOINLINE व्योम jsonAppendCharExpand(JsonString *p, अक्षर c){
  अगर( jsonStringGrow(p,1) ) वापस;
  p->zBuf[p->nUsed++] = c;
}
अटल व्योम jsonAppendChar(JsonString *p, अक्षर c){
  अगर( p->nUsed>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }अन्यथा{
    p->zBuf[p->nUsed++] = c;
  }
}

/* Remove a single character from the end of the string
*/
अटल व्योम jsonStringTrimOneChar(JsonString *p){
  अगर( p->eErr==0 ){
    निश्चित( p->nUsed>0 );
    p->nUsed--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
अटल पूर्णांक jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  वापस p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
अटल व्योम jsonAppendSeparator(JsonString *p){
  अक्षर c;
  अगर( p->nUsed==0 ) वापस;
  c = p->zBuf[p->nUsed-1];
  अगर( c=='[' || c=='{' ) वापस;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
अटल व्योम jsonAppendControlChar(JsonString *p, u8 c){
  अटल स्थिर अक्षर aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  निश्चित( माप(aSpecial)==32 );
  निश्चित( aSpecial['\b']=='b' );
  निश्चित( aSpecial['\f']=='f' );
  निश्चित( aSpecial['\n']=='n' );
  निश्चित( aSpecial['\r']=='r' );
  निश्चित( aSpecial['\t']=='t' );
  निश्चित( c>=0 && c<माप(aSpecial) );
  निश्चित( p->nUsed+7 <= p->nAlloc );
  अगर( aSpecial[c] ){
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
अटल व्योम jsonAppendString(JsonString *p, स्थिर अक्षर *zIn, u32 N){
  u32 k;
  u8 c;
  स्थिर u8 *z = (स्थिर u8*)zIn;
  अगर( z==0 ) वापस;
  अगर( (N+p->nUsed+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) वापस;
  p->zBuf[p->nUsed++] = '"';
  जबतक( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    जबतक( 1 /* Exit by break */ ){
      अगर( k+3>=N ){
        जबतक( k<N && jsonIsOk[z[k]] ){ k++; }
        अवरोध;
      }
      अगर( !jsonIsOk[z[k]] ){
        अवरोध;
      }
      अगर( !jsonIsOk[z[k+1]] ){
        k += 1;
        अवरोध;
      }
      अगर( !jsonIsOk[z[k+2]] ){
        k += 2;
        अवरोध;
      }
      अगर( !jsonIsOk[z[k+3]] ){
        k += 3;
        अवरोध;
      }अन्यथा{
        k += 4;
      }
    }
    अगर( k>=N ){
      अगर( k>0 ){
        स_नकल(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      अवरोध;
    }
    अगर( k>0 ){
      स_नकल(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    अगर( c=='"' || c=='\\' ){
      अगर( (p->nUsed+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) वापस;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    }अन्यथा अगर( c=='\'' ){
      p->zBuf[p->nUsed++] = c;
    }अन्यथा{
      अगर( (p->nUsed+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) वापस;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
  निश्चित( p->nUsed<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
अटल व्योम json_Append_Sql_मूल्य(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_मूल्य *p_मूल्य          /* Value to append */
){
  चयन( sqlite3_मूल्य_type(p_मूल्य) ){
    हाल SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      अवरोध;
    }
    हाल SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_मूल्य_double(p_मूल्य));
      अवरोध;
    }
    हाल SQLITE_INTEGER: {
      स्थिर अक्षर *z = (स्थिर अक्षर*)sqlite3_मूल्य_text(p_मूल्य);
      u32 n = (u32)sqlite3_मूल्य_bytes(p_मूल्य);
      jsonAppendRaw(p, z, n);
      अवरोध;
    }
    हाल SQLITE_TEXT: {
      स्थिर अक्षर *z = (स्थिर अक्षर*)sqlite3_मूल्य_text(p_मूल्य);
      u32 n = (u32)sqlite3_मूल्य_bytes(p_मूल्य);
      अगर( sqlite3_मूल्य_subtype(p_मूल्य)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }अन्यथा{
        jsonAppendString(p, z, n);
      }
      अवरोध;
    }
    शेष: {
      JsonParse px;
      स_रखो(&px, 0, माप(px));
      अगर( jsonArgIsJsonb(p_मूल्य, &px) ){
        jsonTranslateBlobToText(&px, 0, p);
      }अन्यथा अगर( p->eErr==0 ){
        sqlite3_परिणाम_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      अवरोध;
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
अटल व्योम jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  निश्चित( (pParse!=0)==(ctx!=0) );
  निश्चित( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  अगर( p->eErr==0 ){
    पूर्णांक flags = SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(p->pCtx));
    अगर( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }अन्यथा अगर( p->bStatic ){
      sqlite3_परिणाम_text64(p->pCtx, p->zBuf, p->nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }अन्यथा{
      अगर( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        पूर्णांक rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_जोड़ो(ctx, pParse);
        अगर( rc==SQLITE_NOMEM ){
          sqlite3_परिणाम_error_nomem(ctx);
          jsonStringReset(p);
          वापस;
        }
      }
      sqlite3_परिणाम_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }अन्यथा अगर( p->eErr & JSTRING_OOM ){
    sqlite3_परिणाम_error_nomem(p->pCtx);
  }अन्यथा अगर( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }अन्यथा अगर( p->eErr & JSTRING_MALFORMED ){
    sqlite3_परिणाम_error(p->pCtx, "malformed JSON", -1);
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
अटल व्योम jsonParseReset(JsonParse *pParse){
  निश्चित( pParse->nJPRef<=1 );
  अगर( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  अगर( pParse->nBlobAlloc ){
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
अटल व्योम jsonParseFree(JsonParse *pParse){
  अगर( pParse ){
    अगर( pParse->nJPRef>1 ){
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
अटल u8 jsonHexToInt(पूर्णांक h){
#अगर_घोषित SQLITE_ASCII
  h += 9*(1&(h>>6));
#पूर्ण_अगर
#अगर_घोषित SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#पूर्ण_अगर
  वापस (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
अटल u32 jsonHexToInt4(स्थिर अक्षर *z){
  u32 v;
  v = (jsonHexToInt(z[0])<<12)
    + (jsonHexToInt(z[1])<<8)
    + (jsonHexToInt(z[2])<<4)
    + jsonHexToInt(z[3]);
  वापस v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
अटल पूर्णांक jsonIs2Hex(स्थिर अक्षर *z){
  वापस sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
अटल पूर्णांक jsonIs4Hex(स्थिर अक्षर *z){
  वापस jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
अटल पूर्णांक json5Whitespace(स्थिर अक्षर *zIn){
  पूर्णांक n = 0;
  स्थिर u8 *z = (u8*)zIn;
  जबतक( 1 /*exit by "goto whitespace_done"*/ ){
    चयन( z[n] ){
      हाल 0x09:
      हाल 0x0a:
      हाल 0x0b:
      हाल 0x0c:
      हाल 0x0d:
      हाल 0x20: {
        n++;
        अवरोध;
      }
      हाल '/': {
        अगर( z[n+1]=='*' && z[n+2]!=0 ){
          पूर्णांक j;
          क्रम(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            अगर( z[j]==0 ) जाओ whitespace_done;
          }
          n = j+1;
          अवरोध;
        }अन्यथा अगर( z[n+1]=='/' ){
          पूर्णांक j;
          अक्षर c;
          क्रम(j=n+2; (c = z[j])!=0; j++){
            अगर( c=='\n' || c=='\r' ) अवरोध;
            अगर( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              अवरोध;
            }
          }
          n = j;
          अगर( z[n] ) n++;
          अवरोध;
        }
        जाओ whitespace_done;
      }
      हाल 0xc2: {
        अगर( z[n+1]==0xa0 ){
          n += 2;
          अवरोध;
        }
        जाओ whitespace_done;
      }
      हाल 0xe1: {
        अगर( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          अवरोध;
        }
        जाओ whitespace_done;
      }
      हाल 0xe2: {
        अगर( z[n+1]==0x80 ){
          u8 c = z[n+2];
          अगर( c<0x80 ) जाओ whitespace_done;
          अगर( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            अवरोध;
          }
        }अन्यथा अगर( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          अवरोध;
        }
        जाओ whitespace_done;
      }
      हाल 0xe3: {
        अगर( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          अवरोध;
        }
        जाओ whitespace_done;
      }
      हाल 0xef: {
        अगर( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          अवरोध;
        }
        जाओ whitespace_done;
      }
      शेष: {
        जाओ whitespace_done;
      }
    }
  }
  whitespace_done:
  वापस n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
अटल स्थिर काष्ठा NanInfName {
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
अटल व्योम json_Wrong_अंक_Args(
  sqlite3_context *pCtx,
  स्थिर अक्षर *zFuncName
){
  अक्षर *z_संदेश = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_परिणाम_error(pCtx, z_संदेश, -1);
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
अटल पूर्णांक jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  निश्चित( N>pParse->nBlobAlloc );
  अगर( pParse->nBlobAlloc==0 ){
    t = 100;
  }अन्यथा{
    t = pParse->nBlobAlloc*2;
  }
  अगर( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  अगर( aNew==0 ){ pParse->oom = 1; वापस 1; }
  निश्चित( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  वापस 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
अटल पूर्णांक jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 n_आकार;
  निश्चित( !pParse->bReadOnly );
  अगर( pParse->oom ) वापस 0;
  अगर( pParse->nBlobAlloc>0 ) वापस 1;
  aOld = pParse->aBlob;
  n_आकार = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  अगर( jsonBlobExpand(pParse, n_आकार) ){
    वापस 0;
  }
  निश्चित( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  स_नकल(pParse->aBlob, aOld, pParse->nBlob);
  वापस 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
अटल SQLITE_NOINLINE व्योम jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  अगर( pParse->oom==0 ){
    निश्चित( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
अटल व्योम jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  अगर( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }अन्यथा{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
अटल व्योम json_Blob_Append_गाँठ(JsonParse*,u8,u64,स्थिर व्योम*);
अटल SQLITE_NOINLINE व्योम json_Blob_Expand_And_Append_गाँठ(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  स्थिर व्योम *aPayload
){
  अगर( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) वापस;
  json_Blob_Append_गाँठ(pParse, eType, szPayload, aPayload);
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
अटल व्योम json_Blob_Append_गाँठ(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  स्थिर व्योम *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  अगर( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_गाँठ(pParse,eType,szPayload,aPayload);
    वापस;
  }
  निश्चित( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  अगर( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }अन्यथा अगर( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }अन्यथा अगर( szPayload<=0xffff ){
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
  अगर( aPayload ){
    pParse->nBlob += szPayload;
    स_नकल(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
अटल पूर्णांक json_Blob_Change_Payload_आकार(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  पूर्णांक delta;
  अगर( pParse->oom ) वापस 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  अगर( szType<=11 ){
    nExtra = 0;
  }अन्यथा अगर( szType==12 ){
    nExtra = 1;
  }अन्यथा अगर( szType==13 ){
    nExtra = 2;
  }अन्यथा अगर( szType==14 ){
    nExtra = 4;
  }अन्यथा{
    nExtra = 8;
  }
  अगर( szPayload<=11 ){
    nNeeded = 0;
  }अन्यथा अगर( szPayload<=0xff ){
    nNeeded = 1;
  }अन्यथा अगर( szPayload<=0xffff ){
    nNeeded = 2;
  }अन्यथा{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  अगर( delta ){
    u64 new_आकार_2 = (u64)pParse->nBlob + delta;
    अगर( delta>0 ){
      अगर( new_आकार_2>pParse->nBlobAlloc && jsonBlobExpand(pParse, new_आकार_2) ){
        वापस 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      स_हटाओ(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }अन्यथा{
      स_हटाओ(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = new_आकार_2;
  }
  अगर( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }अन्यथा अगर( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }अन्यथा अगर( nNeeded==2 ){
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
  वापस delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
अटल पूर्णांक jsonIs4HexB(स्थिर अक्षर *z, पूर्णांक *pOp){
  अगर( z[0]!='u' ) वापस 0;
  अगर( !jsonIs4Hex(&z[1]) ) वापस 0;
  *pOp = JSONB_TEXTJ;
  वापस 1;
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
अटल u32 jsonbValidityCheck(
  स्थिर JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 iEnd,                   /* One more than the last byte of the element */
  u32 iDepth                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  स्थिर u8 *z;
  u8 x;
  अगर( iDepth>JSON_MAX_DEPTH ) वापस i+1;
  sz = 0;
  n = jsonb_Payload_आकार(pParse, i, &sz);
  अगर( NEVER(n==0) ) वापस i+1;          /* Checked by caller */
  अगर( NEVER(i+n+sz!=iEnd) ) वापस i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  चयन( x ){
    हाल JSONB_NULL:
    हाल JSONB_TRUE:
    हाल JSONB_FALSE: {
      वापस n+sz==1 ? 0 : i+1;
    }
    हाल JSONB_INT: {
      अगर( sz<1 ) वापस i+1;
      j = i+n;
      अगर( z[j]=='-' ){
        j++;
        अगर( sz<2 ) वापस i+1;
      }
      k = i+n+sz;
      जबतक( j<k ){
        अगर( sqlite3Isdigit(z[j]) ){
          j++;
        }अन्यथा{
          वापस j+1;
        }
      }
      वापस 0;
    }
    हाल JSONB_INT5: {
      अगर( sz<3 ) वापस i+1;
      j = i+n;
      अगर( z[j]=='-' ){
        अगर( sz<4 ) वापस i+1;
        j++;
      }
      अगर( z[j]!='0' ) वापस i+1;
      अगर( z[j+1]!='x' && z[j+1]!='X' ) वापस j+2;
      j += 2;
      k = i+n+sz;
      जबतक( j<k ){
        अगर( sqlite3Isxdigit(z[j]) ){
          j++;
        }अन्यथा{
          वापस j+1;
        }
      }
      वापस 0;
    }
    हाल JSONB_FLOAT:
    हाल JSONB_FLOAT5: {
      u8 seen = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      अगर( sz<2 ) वापस i+1;
      j = i+n;
      k = j+sz;
      अगर( z[j]=='-' ){
        j++;
        अगर( sz<3 ) वापस i+1;
      }
      अगर( z[j]=='.' ){
        अगर( x==JSONB_FLOAT ) वापस j+1;
        अगर( !sqlite3Isdigit(z[j+1]) ) वापस j+1;
        j += 2;
        seen = 1;
      }अन्यथा अगर( z[j]=='0' && x==JSONB_FLOAT ){
        अगर( j+3>k ) वापस j+1;
        अगर( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) वापस j+1;
        j++;
      }
      क्रम(; j<k; j++){
        अगर( sqlite3Isdigit(z[j]) ) जारी;
        अगर( z[j]=='.' ){
          अगर( seen>0 ) वापस j+1;
          अगर( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            वापस j+1;
          }
          seen = 1;
          जारी;
        }
        अगर( z[j]=='e' || z[j]=='E' ){
          अगर( seen==2 ) वापस j+1;
          अगर( j==k-1 ) वापस j+1;
          अगर( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            अगर( j==k-1 ) वापस j+1;
          }
          seen = 2;
          जारी;
        }
        वापस j+1;
      }
      अगर( seen==0 ) वापस i+1;
      वापस 0;
    }
    हाल JSONB_TEXT: {
      j = i+n;
      k = j+sz;
      जबतक( j<k ){
        अगर( !jsonIsOk[z[j]] && z[j]!='\'' ) वापस j+1;
        j++;
      }
      वापस 0;
    }
    हाल JSONB_TEXTJ:
    हाल JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      जबतक( j<k ){
        अगर( !jsonIsOk[z[j]] && z[j]!='\'' ){
          अगर( z[j]=='"' ){
            अगर( x==JSONB_TEXTJ ) वापस j+1;
          }अन्यथा अगर( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            अगर( x==JSONB_TEXTJ ) वापस j+1;
          }अन्यथा अगर( NEVER(z[j]!='\\') || j+1>=k ){
            वापस j+1;
          }अन्यथा अगर( म_अक्षर("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }अन्यथा अगर( z[j+1]=='u' ){
            अगर( j+5>=k ) वापस j+1;
            अगर( !jsonIs4Hex((स्थिर अक्षर*)&z[j+2]) ) वापस j+1;
            j++;
          }अन्यथा अगर( x!=JSONB_TEXT5 ){
            वापस j+1;
          }अन्यथा{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((स्थिर अक्षर*)&z[j], k-j, &c);
            अगर( c==JSON_INVALID_CHAR ) वापस j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      वापस 0;
    }
    हाल JSONB_TEXTRAW: {
      वापस 0;
    }
    हाल JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      जबतक( j<k ){
        sz = 0;
        n = jsonb_Payload_आकार(pParse, j, &sz);
        अगर( n==0 ) वापस j+1;
        अगर( j+n+sz>k ) वापस j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        अगर( sub ) वापस sub;
        j += n + sz;
      }
      निश्चित( j==k );
      वापस 0;
    }
    हाल JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      जबतक( j<k ){
        sz = 0;
        n = jsonb_Payload_आकार(pParse, j, &sz);
        अगर( n==0 ) वापस j+1;
        अगर( j+n+sz>k ) वापस j+1;
        अगर( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          अगर( x<JSONB_TEXT || x>JSONB_TEXTRAW ) वापस j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, iDepth+1);
        अगर( sub ) वापस sub;
        cnt++;
        j += n + sz;
      }
      निश्चित( j==k );
      अगर( (cnt & 1)!=0 ) वापस j+1;
      वापस 0;
    }
    शेष: {
      वापस i+1;
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
अटल पूर्णांक jsonTranslateTextToBlob(JsonParse *pParse, u32 i){
  अक्षर c;
  u32 j;
  u32 iThis, iStart;
  पूर्णांक x;
  u8 t;
  स्थिर अक्षर *z = pParse->zJson;
json_parse_restart:
  चयन( (u8)z[i] ){
  हाल '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_गाँठ(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    अगर( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      वापस -1;
    }
    iStart = pParse->nBlob;
    क्रम(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = jsonTranslateTextToBlob(pParse, j);
      अगर( x<=0 ){
        पूर्णांक op;
        अगर( x==(-2) ){
          j = pParse->iErr;
          अगर( pParse->nBlob!=(u32)iStart ) pParse->hasNonstd = 1;
          अवरोध;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_TEXT;
        अगर( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          पूर्णांक k = j+1;
          जबतक( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          निश्चित( iBlob==pParse->nBlob );
          json_Blob_Append_गाँठ(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }अन्यथा{
          अगर( x!=-1 ) pParse->iErr = j;
          वापस -1;
        }
      }
      अगर( pParse->oom ) वापस -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      अगर( t<JSONB_TEXT || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        वापस -1;
      }
      j = x;
      अगर( z[j]==':' ){
        j++;
      }अन्यथा{
        अगर( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          करो{ j++; }जबतक( jsonIsspace(z[j]) );
          अगर( z[j]==':' ){
            j++;
            जाओ parse_object_मूल्य;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        अगर( x!=(-5) ){
          अगर( x!=(-1) ) pParse->iErr = j;
          वापस -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_मूल्य:
      x = jsonTranslateTextToBlob(pParse, j);
      अगर( x<=0 ){
        अगर( x!=(-1) ) pParse->iErr = j;
        वापस -1;
      }
      j = x;
      अगर( z[j]==',' ){
        जारी;
      }अन्यथा अगर( z[j]=='}' ){
        अवरोध;
      }अन्यथा{
        अगर( jsonIsspace(z[j]) ){
          j += 1 + (u32)म_अखोज(&z[j+1], jsonSpaces);
          अगर( z[j]==',' ){
            जारी;
          }अन्यथा अगर( z[j]=='}' ){
            अवरोध;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        अगर( x==(-4) ){
          j = pParse->iErr;
          जारी;
        }
        अगर( x==(-2) ){
          j = pParse->iErr;
          अवरोध;
        }
      }
      pParse->iErr = j;
      वापस -1;
    }
    json_Blob_Change_Payload_आकार(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    वापस j+1;
  }
  हाल '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    निश्चित( i<=(u32)pParse->nJson );
    json_Blob_Append_गाँठ(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    iStart = pParse->nBlob;
    अगर( pParse->oom ) वापस -1;
    अगर( ++pParse->iDepth > JSON_MAX_DEPTH ){
      pParse->iErr = i;
      वापस -1;
    }
    क्रम(j=i+1;;j++){
      x = jsonTranslateTextToBlob(pParse, j);
      अगर( x<=0 ){
        अगर( x==(-3) ){
          j = pParse->iErr;
          अगर( pParse->nBlob!=iStart ) pParse->hasNonstd = 1;
          अवरोध;
        }
        अगर( x!=(-1) ) pParse->iErr = j;
        वापस -1;
      }
      j = x;
      अगर( z[j]==',' ){
        जारी;
      }अन्यथा अगर( z[j]==']' ){
        अवरोध;
      }अन्यथा{
        अगर( jsonIsspace(z[j]) ){
          j += 1 + (u32)म_अखोज(&z[j+1], jsonSpaces);
          अगर( z[j]==',' ){
            जारी;
          }अन्यथा अगर( z[j]==']' ){
            अवरोध;
          }
        }
        x = jsonTranslateTextToBlob(pParse, j);
        अगर( x==(-4) ){
          j = pParse->iErr;
          जारी;
        }
        अगर( x==(-3) ){
          j = pParse->iErr;
          अवरोध;
        }
      }
      pParse->iErr = j;
      वापस -1;
    }
    json_Blob_Change_Payload_आकार(pParse, iThis, pParse->nBlob - iStart);
    pParse->iDepth--;
    वापस j+1;
  }
  हाल '\'': {
    u8 opcode;
    अक्षर cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_TEXT;
    जाओ parse_string;
  हाल '"':
    /* Parse string */
    opcode = JSONB_TEXT;
  parse_string:
    cDelim = z[i];
    j = i+1;
    जबतक( 1 /*exit-by-break*/ ){
      अगर( jsonIsOk[(u8)z[j]] ){
        अगर( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }अन्यथा अगर( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }अन्यथा{
          j += 3;
          जारी;
        }
      }
      c = z[j];
      अगर( c==cDelim ){
        अवरोध;
      }अन्यथा अगर( c=='\\' ){
        c = z[++j];
        अगर( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          अगर( opcode==JSONB_TEXT ) opcode = JSONB_TEXTJ;
        }अन्यथा अगर( c=='\'' ||  c=='v' || c=='\n'
#अगर_घोषित SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#अन्यथा
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#पूर्ण_अगर
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }अन्यथा अगर( c=='\r' ){
          अगर( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }अन्यथा{
          pParse->iErr = j;
          वापस -1;
        }
      }अन्यथा अगर( c<=0x1f ){
        अगर( c==0 ){
          pParse->iErr = j;
          वापस -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }अन्यथा अगर( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_गाँठ(pParse, opcode, j-1-i, &z[i+1]);
    वापस j+1;
  }
  हाल 't': {
    अगर( म_भेदन(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      वापस i+4;
    }
    pParse->iErr = i;
    वापस -1;
  }
  हाल 'f': {
    अगर( म_भेदन(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      वापस i+5;
    }
    pParse->iErr = i;
    वापस -1;
  }
  हाल '+': {
    u8 seenE;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    जाओ parse_संख्या;
  हाल '.':
    अगर( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      seenE = 0;
      जाओ parse_संख्या_2;
    }
    pParse->iErr = i;
    वापस -1;
  हाल '-':
  हाल '0':
  हाल '1':
  हाल '2':
  हाल '3':
  हाल '4':
  हाल '5':
  हाल '6':
  हाल '7':
  हाल '8':
  हाल '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_संख्या:
    seenE = 0;
    निश्चित( '-' < '0' );
    निश्चित( '+' < '0' );
    निश्चित( '.' < '0' );
    c = z[i];

    अगर( c<='0' ){
      अगर( c=='0' ){
        अगर( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          निश्चित( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          क्रम(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          जाओ parse_संख्या_finish;
        }अन्यथा अगर( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          वापस -1;
        }
      }अन्यथा{
        अगर( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          अगर( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            अगर( z[i]=='-' ){
              json_Blob_Append_गाँठ(pParse, JSONB_FLOAT, 6, "-9e999");
            }अन्यथा{
              json_Blob_Append_गाँठ(pParse, JSONB_FLOAT, 5, "9e999");
            }
            वापस i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          अगर( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            जाओ parse_संख्या_2;
          }
          pParse->iErr = i;
          वापस -1;
        }
        अगर( z[i+1]=='0' ){
          अगर( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            वापस -1;
          }अन्यथा अगर( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            क्रम(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            जाओ parse_संख्या_finish;
          }
        }
      }
    }

  parse_संख्या_2:
    क्रम(j=i+1;; j++){
      c = z[j];
      अगर( sqlite3Isdigit(c) ) जारी;
      अगर( c=='.' ){
        अगर( (t & 0x02)!=0 ){
          pParse->iErr = j;
          वापस -1;
        }
        t |= 0x02;
        जारी;
      }
      अगर( c=='e' || c=='E' ){
        अगर( z[j-1]<'0' ){
          अगर( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }अन्यथा{
            pParse->iErr = j;
            वापस -1;
          }
        }
        अगर( seenE ){
          pParse->iErr = j;
          वापस -1;
        }
        t |= 0x02;
        seenE = 1;
        c = z[j+1];
        अगर( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        अगर( c<'0' || c>'9' ){
          pParse->iErr = j;
          वापस -1;
        }
        जारी;
      }
      अवरोध;
    }
    अगर( z[j-1]<'0' ){
      अगर( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }अन्यथा{
        pParse->iErr = j;
        वापस -1;
      }
    }
  parse_संख्या_finish:
    निश्चित( JSONB_INT+0x01==JSONB_INT5 );
    निश्चित( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    निश्चित( JSONB_INT+0x02==JSONB_FLOAT );
    अगर( z[i]=='+' ) i++;
    json_Blob_Append_गाँठ(pParse, JSONB_INT+t, j-i, &z[i]);
    वापस j;
  }
  हाल '}': {
    pParse->iErr = i;
    वापस -2;  /* End of {...} */
  }
  हाल ']': {
    pParse->iErr = i;
    वापस -3;  /* End of [...] */
  }
  हाल ',': {
    pParse->iErr = i;
    वापस -4;  /* List separator */
  }
  हाल ':': {
    pParse->iErr = i;
    वापस -5;  /* Object label/value separator */
  }
  हाल 0: {
    वापस 0;   /* End of file */
  }
  हाल 0x09:
  हाल 0x0a:
  हाल 0x0d:
  हाल 0x20: {
    i += 1 + (u32)म_अखोज(&z[i+1], jsonSpaces);
    जाओ json_parse_restart;
  }
  हाल 0x0b:
  हाल 0x0c:
  हाल '/':
  हाल 0xc2:
  हाल 0xe1:
  हाल 0xe2:
  हाल 0xe3:
  हाल 0xef: {
    j = json5Whitespace(&z[i]);
    अगर( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      जाओ json_parse_restart;
    }
    pParse->iErr = i;
    वापस -1;
  }
  हाल 'n': {
    अगर( म_भेदन(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      वापस i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  शेष: {
    u32 k;
    पूर्णांक nn;
    c = z[i];
    क्रम(k=0; k<माप(aNanInfName)/माप(aNanInfName[0]); k++){
      अगर( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) जारी;
      nn = aNanInfName[k].n;
      अगर( sqlite3StrNICmp(&z[i], aNanInfName[k].zMatch, nn)!=0 ){
        जारी;
      }
      अगर( sqlite3Isalnum(z[i+nn]) ) जारी;
      अगर( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_गाँठ(pParse, JSONB_FLOAT, 5, "9e999");
      }अन्यथा{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      वापस i + nn;
    }
    pParse->iErr = i;
    वापस -1;  /* Syntax error */
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
अटल पूर्णांक jsonConvertTextToBlob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  पूर्णांक i;
  स्थिर अक्षर *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  अगर( pParse->oom ) i = -1;
  अगर( i>0 ){
#अगर_घोषित SQLITE_DEBUG
    निश्चित( pParse->iDepth==0 );
    अगर( sqlite3Config.bJsonSelfcheck ){
      निश्चित( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#पूर्ण_अगर
    जबतक( jsonIsspace(zJson[i]) ) i++;
    अगर( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      अगर( zJson[i] ){
        अगर( pCtx ) sqlite3_परिणाम_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        वापस 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  अगर( i<=0 ){
    अगर( pCtx!=0 ){
      अगर( pParse->oom ){
        sqlite3_परिणाम_error_nomem(pCtx);
      }अन्यथा{
        sqlite3_परिणाम_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    वापस 1;
  }
  वापस 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
अटल व्योम jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  निश्चित( pStr->eErr==0 );
  स_रखो(&px, 0, माप(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (व्योम)jsonTranslateTextToBlob(&px, 0);
  अगर( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_परिणाम_error_nomem(pStr->pCtx);
  }अन्यथा{
    निश्चित( px.nBlobAlloc>0 );
    निश्चित( !px.bReadOnly );
    sqlite3_परिणाम_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
अटल u32 jsonb_Payload_आकार_Wide(
  स्थिर JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  निश्चित( i<pParse->nBlob );
  निश्चित( x==(pParse->aBlob[i]>>4) );
  निश्चित( x>=13 );
  अगर( x==13 ){
    अगर( i+2>=pParse->nBlob ){
      *pSz = 0;
      वापस 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }अन्यथा अगर( x==14 ){
    अगर( i+4>=pParse->nBlob ){
      *pSz = 0;
      वापस 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }अन्यथा{
    अगर( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      वापस 0;
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
  अगर( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    वापस 0;
  }
  *pSz = sz;
  वापस n;
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
अटल u32 jsonb_Payload_आकार(
  स्थिर JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  अगर( i>=pParse->nBlob ){
    *pSz = 0;
    वापस 0;
  }अन्यथा अगर( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }अन्यथा अगर( x==12 ){
    अगर( i+1>=pParse->nBlob ){
      *pSz = 0;
      वापस 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }अन्यथा{
    वापस jsonb_Payload_आकार_Wide(pParse, i, pSz, x);
  }
  अगर( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    वापस 0;
  }
  *pSz = sz;
  वापस n;
}

#अगर SQLITE_USES_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
अटल SQLITE_INLINE u32 jsonb_Payload_आकार_Inline(
  स्थिर JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  अगर( i>=pParse->nBlob ){
    *pSz = 0;
    वापस 0;
  }अन्यथा अगर( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }अन्यथा अगर( x==12 ){
    अगर( i+1>=pParse->nBlob ){
      *pSz = 0;
      वापस 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }अन्यथा अगर( x==13 ){
    अगर( i+2>=pParse->nBlob ){
      *pSz = 0;
      वापस 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }अन्यथा अगर( x==14 ){
    अगर( i+4>=pParse->nBlob ){
      *pSz = 0;
      वापस 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }अन्यथा{
    अगर( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      वापस 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  अगर( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    वापस 0;
  }
  *pSz = sz;
  वापस n;
}
#अन्यथा /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_आकार_Inline(a,b,c) jsonb_Payload_आकार(a,b,c)
#पूर्ण_अगर


#अगर SQLITE_USES_INLINE
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
अटल SQLITE_INLINE u32 jsonb_Payload_आकार_Semi_Inline(
  स्थिर JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  निश्चित( i<pParse->nBlob );
  अगर( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }अन्यथा अगर( x==12 ){
    अगर( i+1>=pParse->nBlob ){
      *pSz = 0;
      वापस 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }अन्यथा{
    वापस jsonb_Payload_आकार_Wide(pParse,i,pSz,x);
  }
  अगर( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    वापस 0;
  }
  *pSz = sz;
  वापस n;
}
#अन्यथा /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_आकार_Semi_Inline(a,b,c) jsonb_Payload_आकार(a,b,c)
#पूर्ण_अगर

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
अटल u32 jsonTranslateBlobToText(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *pOut               /* Write JSON here */
){
  u32 sz, n, j, iEnd;

  n = jsonb_Payload_आकार_Inline(pParse, i, &sz);
  अगर( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    वापस pParse->nBlob+1;
  }
  चयन( pParse->aBlob[i] & 0x0f ){
    हाल JSONB_NULL: {
      jsonAppendRawNZ(pOut, "null", 4);
      वापस i+1;
    }
    हाल JSONB_TRUE: {
      jsonAppendRawNZ(pOut, "true", 4);
      वापस i+1;
    }
    हाल JSONB_FALSE: {
      jsonAppendRawNZ(pOut, "false", 5);
      वापस i+1;
    }
    हाल JSONB_INT:
    हाल JSONB_FLOAT: {
      अगर( sz==0 ) जाओ malformed_jsonb;
      jsonAppendRaw(pOut, (स्थिर अक्षर*)&pParse->aBlob[i+n], sz);
      अवरोध;
    }
    हाल JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      स्थिर अक्षर *zIn = (स्थिर अक्षर*)&pParse->aBlob[i+n];
      पूर्णांक bOverflow = 0;
      अगर( sz==0 ) जाओ malformed_jsonb;
      अगर( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        k++;
      }अन्यथा अगर( zIn[0]=='+' ){
        k++;
      }
      क्रम(; k<sz; k++){
        अगर( !sqlite3Isxdigit(zIn[k]) ){
          pOut->eErr |= JSTRING_MALFORMED;
          अवरोध;
        }अन्यथा अगर( (u>>60)!=0 ){
          bOverflow = 1;
        }अन्यथा{
          u = u*16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100,pOut,bOverflow?"9.0e999":"%llu", u);
      अवरोध;
    }
    हाल JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      स्थिर अक्षर *zIn = (स्थिर अक्षर*)&pParse->aBlob[i+n];
      अगर( sz==0 ) जाओ malformed_jsonb;
      अगर( zIn[0]=='-' ){
        jsonAppendChar(pOut, '-');
        अगर( sz<=1 ) जाओ malformed_jsonb;
        k = 1;
      }
      अगर( zIn[k]=='.' ){
        jsonAppendChar(pOut, '0');
      }
      क्रम(; k<sz; k++){
        jsonAppendChar(pOut, zIn[k]);
        अगर( zIn[k]=='.' && (k+1==sz || !sqlite3Isdigit(zIn[k+1])) ){
          jsonAppendChar(pOut, '0');
        }
      }
      अवरोध;
    }
    हाल JSONB_TEXT:
    हाल JSONB_TEXTJ: {
      अगर( pOut->nUsed+sz+2<=pOut->nAlloc || jsonStringGrow(pOut, sz+2)==0 ){
        pOut->zBuf[pOut->nUsed] = '"';
        स_नकल(pOut->zBuf+pOut->nUsed+1,(स्थिर अक्षर*)&pParse->aBlob[i+n],sz);
        pOut->zBuf[pOut->nUsed+sz+1] = '"';
        pOut->nUsed += sz+2;
      }
      अवरोध;
    }
    हाल JSONB_TEXT5: {
      स्थिर अक्षर *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (स्थिर अक्षर*)&pParse->aBlob[i+n];
      jsonAppendChar(pOut, '"');
      जबतक( sz2>0 ){
        क्रम(k=0; k<sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k]=='\''); k++){}
        अगर( k>0 ){
          jsonAppendRawNZ(pOut, zIn, k);
          अगर( k>=sz2 ){
            अवरोध;
          }
          zIn += k;
          sz2 -= k;
        }
        अगर( zIn[0]=='"' ){
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          जारी;
        }
        अगर( zIn[0]<=0x1f ){
          अगर( pOut->nUsed+7>pOut->nAlloc && jsonStringGrow(pOut,7) ) अवरोध;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          जारी;
        }
        निश्चित( zIn[0]=='\\' );
        निश्चित( sz2>=1 );
        अगर( sz2<2 ){
          pOut->eErr |= JSTRING_MALFORMED;
          अवरोध;
        }
        चयन( (u8)zIn[1] ){
          हाल '\'':
            jsonAppendChar(pOut, '\'');
            अवरोध;
          हाल 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            अवरोध;
          हाल 'x':
            अगर( sz2<4 ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              अवरोध;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            अवरोध;
          हाल '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            अवरोध;
          हाल '\r':
            अगर( sz2>2 && zIn[2]=='\n' ){
              zIn++;
              sz2--;
            }
            अवरोध;
          हाल '\n':
            अवरोध;
          हाल 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            अगर( sz2<4
             || 0x80!=(u8)zIn[2]
             || (0xa8!=(u8)zIn[3] && 0xa9!=(u8)zIn[3])
            ){
              pOut->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              अवरोध;
            }
            zIn += 2;
            sz2 -= 2;
            अवरोध;
          शेष:
            jsonAppendRawNZ(pOut, zIn, 2);
            अवरोध;
        }
        निश्चित( sz2>=2 );
        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      अवरोध;
    }
    हाल JSONB_TEXTRAW: {
      jsonAppendString(pOut, (स्थिर अक्षर*)&pParse->aBlob[i+n], sz);
      अवरोध;
    }
    हाल JSONB_ARRAY: {
      jsonAppendChar(pOut, '[');
      j = i+n;
      iEnd = j+sz;
      अगर( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      जबतक( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      अगर( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      अगर( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      अवरोध;
    }
    हाल JSONB_OBJECT: {
      पूर्णांक x = 0;
      jsonAppendChar(pOut, '{');
      j = i+n;
      iEnd = j+sz;
      अगर( ++pParse->iDepth > JSON_MAX_DEPTH ){
        jsonStringTooDeep(pOut);
      }
      जबतक( j<iEnd && pOut->eErr==0 ){
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      अगर( (x & 1)!=0 || j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
      अगर( sz>0 ) jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      अवरोध;
    }

    शेष: {
      malformed_jsonb:
      pOut->eErr |= JSTRING_MALFORMED;
      अवरोध;
    }
  }
  वापस i+n+sz;
}

/* Context for recursion of json_pretty()
*/
प्रकार काष्ठा JsonPretty JsonPretty;
काष्ठा JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *pOut;         /* Generate pretty output into this string */
  स्थिर अक्षर *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
अटल व्योम jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  क्रम(jj=0; jj<pPretty->nIndent; jj++){
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
अटल u32 jsonTranslateBlobToPrettyText(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonb_Payload_आकार(pParse, i, &sz);
  अगर( n==0 ){
    pOut->eErr |= JSTRING_MALFORMED;
    वापस pParse->nBlob+1;
  }
  चयन( pParse->aBlob[i] & 0x0f ){
    हाल JSONB_ARRAY: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '[');
      अगर( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        अगर( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        जबतक( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          अगर( j>=iEnd ){
            अगर( j>iEnd ) pOut->eErr |= JSTRING_MALFORMED;
            अवरोध;
          }
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, ']');
      i = iEnd;
      अवरोध;
    }
    हाल JSONB_OBJECT: {
      j = i+n;
      iEnd = j+sz;
      jsonAppendChar(pOut, '{');
      अगर( j<iEnd ){
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent++;
        अगर( pPretty->nIndent >= JSON_MAX_DEPTH ){
          jsonStringTooDeep(pOut);
        }
        pParse->iDepth = pPretty->nIndent;
        जबतक( pOut->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = jsonTranslateBlobToText(pParse, j, pOut);
          अगर( j>iEnd ){
            pOut->eErr |= JSTRING_MALFORMED;
            अवरोध;
          }
          jsonAppendRawNZ(pOut, ": ", 2);
          j = jsonTranslateBlobToPrettyText(pPretty, j);
          अगर( j>=iEnd ) अवरोध;
          jsonAppendRawNZ(pOut, ",\n", 2);
        }
        jsonAppendChar(pOut, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(pOut, '}');
      i = iEnd;
      अवरोध;
    }
    शेष: {
      i = jsonTranslateBlobToText(pParse, i, pOut);
      अवरोध;
    }
  }
  वापस i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
अटल u32 jsonb_Array_गिनती(JsonParse *pParse, u32 i_मूल){
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonb_Payload_आकार(pParse, i_मूल, &sz);
  iEnd = i_मूल+n+sz;
  क्रम(i=i_मूल+n; n>0 && i<iEnd; i+=sz+n, k++){
    n = jsonb_Payload_आकार_Semi_Inline(pParse, i, &sz);
  }
  वापस k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
अटल व्योम json_After_Edit_आकार_Adjust(JsonParse *pParse, u32 i_मूल){
  u32 sz = 0;
  u32 nBlob;
  निश्चित( pParse->delta!=0 );
  निश्चित( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (व्योम)jsonb_Payload_आकार(pParse, i_मूल, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += json_Blob_Change_Payload_आकार(pParse, i_मूल, sz);
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
अटल पूर्णांक jsonBlobOverwrite(
  u8 *aOut,                 /* Overwrite here */
  स्थिर u8 *aIns,           /* New content */
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
  अटल स्थिर u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  अगर( (aIns[0]&0x0f)<=2 ) वापस 0;    /* Cannot enlarge NULL, true, false */
  चयन( aIns[0]>>4 ){
    शेष: {                         /* aIns[] header size 1 */
      अगर( ((1<<d)&0x116)==0 ) वापस 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      अवरोध;
    }
    हाल 12: {                         /* aIns[] header size is 2 */
      अगर( ((1<<d)&0x8a)==0) वापस 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      अवरोध;
    }
    हाल 13: {                         /* aIns[] header size is 3 */
      अगर( d!=2 && d!=6 ) वापस 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      अवरोध;
    }
    हाल 14: {                         /* aIns[] header size is 5 */
      अगर( d!=4 ) वापस 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      अवरोध;
    }
    हाल 15: {                         /* aIns[] header size is 9 */
      वापस 0;                          /* No solution */
    }
  }
  निश्चित( i>=2 && i<=9 && aType[i-2]!=0 );
  aOut[0] = (aIns[0] & 0x0f) | aType[i-2];
  स_नकल(&aOut[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  जबतक( 1/*edit-by-break*/ ){
    i--;
    aOut[i] = szPayload & 0xff;
    अगर( i==1 ) अवरोध;
    szPayload >>= 8;
  }
  निश्चित( (szPayload>>8)==0 );
  वापस 1;
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
अटल व्योम jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  स्थिर u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  निश्चित( pParse->nBlob >= (u64)iDel + (u64)nDel );
  अगर( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (पूर्णांक)-d)
  ){
    वापस;
  }
  अगर( d!=0 ){
    अगर( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      अगर( pParse->oom ) वापस;
    }
    स_हटाओ(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  अगर( nIns && aIns ){
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
अटल u32 jsonBytesToBypass(स्थिर अक्षर *z, u32 n){
  u32 i = 0;
  जबतक( i+1<n ){
    अगर( z[i]!='\\' ) वापस i;
    अगर( z[i+1]=='\n' ){
      i += 2;
      जारी;
    }
    अगर( z[i+1]=='\r' ){
      अगर( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }अन्यथा{
        i += 2;
      }
      जारी;
    }
    अगर( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      जारी;
    }
    अवरोध;
  }
  वापस i;
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
अटल u32 jsonUnescapeOneChar(स्थिर अक्षर *z, u32 n, u32 *piOut){
  निश्चित( n>0 );
  निश्चित( z[0]=='\\' );
  अगर( n<2 ){
    *piOut = JSON_INVALID_CHAR;
    वापस n;
  }
  चयन( (u8)z[1] ){
    हाल 'u': {
      u32 v, vlo;
      अगर( n<6 ){
        *piOut = JSON_INVALID_CHAR;
        वापस n;
      }
      v = jsonHexToInt4(&z[2]);
      अगर( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = jsonHexToInt4(&z[8]))&0xfc00)==0xdc00
      ){
        *piOut = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        वापस 12;
      }अन्यथा{
        *piOut = v;
        वापस 6;
      }
    }
    हाल 'b': {   *piOut = '\b';  वापस 2; }
    हाल 'f': {   *piOut = '\f';  वापस 2; }
    हाल 'n': {   *piOut = '\n';  वापस 2; }
    हाल 'r': {   *piOut = '\r';  वापस 2; }
    हाल 't': {   *piOut = '\t';  वापस 2; }
    हाल 'v': {   *piOut = '\v';  वापस 2; }
    हाल '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#अगर_घोषित SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *piOut = 0;
#अन्यथा
      /* Correct behavior */
      *piOut = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#पूर्ण_अगर
      वापस 2;
    }
    हाल '\'':
    हाल '"':
    हाल '/':
    हाल '\\':{   *piOut = z[1];  वापस 2; }
    हाल 'x': {
      अगर( n<4 ){
        *piOut = JSON_INVALID_CHAR;
        वापस n;
      }
      *piOut = (jsonHexToInt(z[2])<<4) | jsonHexToInt(z[3]);
      वापस 4;
    }
    हाल 0xe2:
    हाल '\r':
    हाल '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      अगर( nSkip==0 ){
        *piOut = JSON_INVALID_CHAR;
        वापस n;
      }अन्यथा अगर( nSkip==n ){
        *piOut = 0;
        वापस n;
      }अन्यथा अगर( z[nSkip]=='\\' ){
        वापस nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, piOut);
      }अन्यथा{
        पूर्णांक sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, piOut);
        वापस nSkip + sz;
      }
    }
    शेष: {
      *piOut = JSON_INVALID_CHAR;
      वापस 2;
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
अटल SQLITE_NOINLINE पूर्णांक jsonLabelCompareEscaped(
  स्थिर अक्षर *z_बायाँ,          /* The left label */
  u32 n_बायाँ,                  /* Size of the left label in bytes */
  पूर्णांक raw_बायाँ,                /* True if zLeft contains no escapes */
  स्थिर अक्षर *z_दायाँ,         /* The right label */
  u32 n_दायाँ,                 /* Size of the right label in bytes */
  पूर्णांक raw_दायाँ                /* True if zRight is escape-free */
){
  u32 c_बायाँ, c_दायाँ;
  निश्चित( raw_बायाँ==0 || raw_दायाँ==0 );
  जबतक( 1 /*exit-by-return*/ ){
    अगर( n_बायाँ==0 ){
      c_बायाँ = 0;
    }अन्यथा अगर( raw_बायाँ || z_बायाँ[0]!='\\' ){
      c_बायाँ = ((u8*)z_बायाँ)[0];
      अगर( c_बायाँ>=0xc0 ){
        पूर्णांक sz = sqlite3Utf8ReadLimited((u8*)z_बायाँ, n_बायाँ, &c_बायाँ);
        z_बायाँ += sz;
        n_बायाँ -= sz;
      }अन्यथा{
        z_बायाँ++;
        n_बायाँ--;
      }
    }अन्यथा{
      u32 n = jsonUnescapeOneChar(z_बायाँ, n_बायाँ, &c_बायाँ);
      z_बायाँ += n;
      निश्चित( n<=n_बायाँ );
      n_बायाँ -= n;
    }
    अगर( n_दायाँ==0 ){
      c_दायाँ = 0;
    }अन्यथा अगर( raw_दायाँ || z_दायाँ[0]!='\\' ){
      c_दायाँ = ((u8*)z_दायाँ)[0];
      अगर( c_दायाँ>=0xc0 ){
        पूर्णांक sz = sqlite3Utf8ReadLimited((u8*)z_दायाँ, n_दायाँ, &c_दायाँ);
        z_दायाँ += sz;
        n_दायाँ -= sz;
      }अन्यथा{
        z_दायाँ++;
        n_दायाँ--;
      }
    }अन्यथा{
      u32 n = jsonUnescapeOneChar(z_दायाँ, n_दायाँ, &c_दायाँ);
      z_दायाँ += n;
      निश्चित( n<=n_दायाँ );
      n_दायाँ -= n;
    }
    अगर( c_बायाँ!=c_दायाँ ) वापस 0;
    अगर( c_बायाँ==0 ) वापस 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
अटल पूर्णांक jsonLabelCompare(
  स्थिर अक्षर *z_बायाँ,          /* The left label */
  u32 n_बायाँ,                  /* Size of the left label in bytes */
  पूर्णांक raw_बायाँ,                /* True if zLeft contains no escapes */
  स्थिर अक्षर *z_दायाँ,         /* The right label */
  u32 n_दायाँ,                 /* Size of the right label in bytes */
  पूर्णांक raw_दायाँ                /* True if zRight is escape-free */
){
  अगर( raw_बायाँ && raw_दायाँ ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    अगर( n_बायाँ!=n_दायाँ ) वापस 0;
    वापस स_भेद(z_बायाँ, z_दायाँ, n_बायाँ)==0;
  }अन्यथा{
    वापस jsonLabelCompareEscaped(z_बायाँ, n_बायाँ, raw_बायाँ,
                                   z_दायाँ, n_दायाँ, raw_दायाँ);
  }
}

/*
** Error returns from jsonLookupStep()
*/
#घोषणा JSON_LOOKUP_ERROR      0xffffffff
#घोषणा JSON_LOOKUP_NOTFOUND   0xfffffffe
#घोषणा JSON_LOOKUP_NOTARRAY   0xfffffffd
#घोषणा JSON_LOOKUP_TOODEEP    0xfffffffc
#घोषणा JSON_LOOKUP_PATHERROR  0xfffffffb
#घोषणा JSON_LOOKUP_ISERROR(x) ((x)>=JSON_LOOKUP_PATHERROR)

/* Forward declaration */
अटल u32 jsonLookupStep(JsonParse*,u32,स्थिर अक्षर*,u32);


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
अटल u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  स्थिर अक्षर *z_पुच्छ   /* Tail of the path that determines substructure */
){
  अटल स्थिर u8 emptyObject[] = { JSONB_ARRAY, JSONB_OBJECT };
  पूर्णांक rc;
  स_रखो(pIns, 0, माप(*pIns));
  pIns->db = pParse->db;
  अगर( z_पुच्छ[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }अन्यथा{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&emptyObject[z_पुच्छ[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth+1;
    अगर( pIns->iDepth >= JSON_MAX_DEPTH ){
      वापस JSON_LOOKUP_TOODEEP;
    }
    rc = jsonLookupStep(pIns, 0, z_पुच्छ, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  वापस rc;  /* Error code only */
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
अटल u32 jsonLookupStep(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_मूल,              /* Begin the search at this element of aBlob[] */
  स्थिर अक्षर *zPath,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_कुंजी, sz, n, iEnd, rc;
  स्थिर अक्षर *z_कुंजी;
  u8 x;

  अगर( zPath[0]==0 ){
    अगर( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonb_Payload_आकार(pParse, i_मूल, &sz);
      sz += n;
      अगर( pParse->eEdit==JEDIT_DEL ){
        अगर( iLabel>0 ){
          sz += i_मूल - iLabel;
          i_मूल = iLabel;
        }
        jsonBlobEdit(pParse, i_मूल, sz, 0, 0);
      }अन्यथा अगर( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }अन्यथा अगर( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        अगर( zPath[-1]!=']' ){
          वापस JSON_LOOKUP_NOTARRAY;
        }अन्यथा{
          jsonBlobEdit(pParse, i_मूल, 0, pParse->aIns, pParse->nIns);
        }
      }अन्यथा{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_मूल, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    वापस i_मूल;
  }
  अगर( zPath[0]=='.' ){
    पूर्णांक raw_कुंजी = 1;
    x = pParse->aBlob[i_मूल];
    zPath++;
    अगर( zPath[0]=='"' ){
      z_कुंजी = zPath + 1;
      क्रम(i=1; zPath[i] && zPath[i]!='"'; i++){
        अगर( zPath[i]=='\\' && zPath[i+1]!=0 ) i++;
      }
      n_कुंजी = i-1;
      अगर( zPath[i] ){
        i++;
      }अन्यथा{
        वापस JSON_LOOKUP_PATHERROR;
      }
      testcase( n_कुंजी==0 );
      raw_कुंजी = स_प्रथम(z_कुंजी, '\\', n_कुंजी)==0;
    }अन्यथा{
      z_कुंजी = zPath;
      क्रम(i=0; zPath[i] && zPath[i]!='.' && zPath[i]!='['; i++){}
      n_कुंजी = i;
      अगर( n_कुंजी==0 ){
        वापस JSON_LOOKUP_PATHERROR;
      }
    }
    अगर( (x & 0x0f)!=JSONB_OBJECT ) वापस JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_आकार(pParse, i_मूल, &sz);
    j = i_मूल + n;  /* j is the index of a label */
    iEnd = j+sz;
    जबतक( j<iEnd ){
      पूर्णांक rawLabel;
      स्थिर अक्षर *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      अगर( x<JSONB_TEXT || x>JSONB_TEXTRAW ) वापस JSON_LOOKUP_ERROR;
      n = jsonb_Payload_आकार_Semi_Inline(pParse, j, &sz);
      अगर( n==0 ) वापस JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      अगर( k+sz>=iEnd ) वापस JSON_LOOKUP_ERROR;
      zLabel = (स्थिर अक्षर*)&pParse->aBlob[k];
      rawLabel = x==JSONB_TEXT || x==JSONB_TEXTRAW;
      अगर( jsonLabelCompare(z_कुंजी, n_कुंजी, raw_कुंजी, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        अगर( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) वापस JSON_LOOKUP_ERROR;
        n = jsonb_Payload_आकार(pParse, v, &sz);
        अगर( n==0 || v+n+sz>iEnd ) वापस JSON_LOOKUP_ERROR;
        निश्चित( j>0 );
        अगर( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          वापस JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        अगर( pParse->delta ) json_After_Edit_आकार_Adjust(pParse, i_मूल);
        वापस rc;
      }
      j = k+sz;
      अगर( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) वापस JSON_LOOKUP_ERROR;
      n = jsonb_Payload_आकार_Semi_Inline(pParse, j, &sz);
      अगर( n==0 ) वापस JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    अगर( j>iEnd ) वापस JSON_LOOKUP_ERROR;
    अगर( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_SET );
      testcase( pParse->eEdit==JEDIT_AINS );
      अगर( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&zPath[i])!=0 ){
        वापस JSON_LOOKUP_NOTARRAY;
      }
      स_रखो(&ix, 0, माप(ix));
      ix.db = pParse->db;
      json_Blob_Append_गाँठ(&ix, raw_कुंजी?JSONB_TEXTRAW:JSONB_TEXT5, n_कुंजी, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      अगर( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_कुंजी+v.nBlob)
      ){
        निश्चित( !pParse->oom );
        nIns = ix.nBlob + n_कुंजी + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        अगर( !pParse->oom ){
          निश्चित( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          निश्चित( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          स_नकल(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          स_नकल(&pParse->aBlob[k], z_कुंजी, n_कुंजी);
          k += n_कुंजी;
          स_नकल(&pParse->aBlob[k], v.aBlob, v.nBlob);
          अगर( ALWAYS(pParse->delta) ) json_After_Edit_आकार_Adjust(pParse, i_मूल);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      वापस rc;
    }
  }अन्यथा अगर( zPath[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_मूल] & 0x0f;
    अगर( x!=JSONB_ARRAY )  वापस JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_आकार(pParse, i_मूल, &sz);
    i = 1;
    जबतक( sqlite3Isdigit(zPath[i]) ){
      अगर( kk<0xffffffff ) kk = kk*10 + zPath[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    अगर( i<2 || zPath[i]!=']' ){
      अगर( zPath[1]=='#' ){
        kk = jsonb_Array_गिनती(pParse, i_मूल);
        i = 2;
        अगर( zPath[2]=='-' && sqlite3Isdigit(zPath[3]) ){
          u64 nn = 0;
          i = 3;
          करो{
            अगर( nn<0xffffffff ) nn = nn*10 + zPath[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }जबतक( sqlite3Isdigit(zPath[i]) );
          अगर( nn>kk ) वापस JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        अगर( zPath[i]!=']' ){
          वापस JSON_LOOKUP_PATHERROR;
        }
      }अन्यथा{
        वापस JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_मूल+n;
    iEnd = j+sz;
    जबतक( j<iEnd ){
      अगर( kk==0 ){
        अगर( ++pParse->iDepth >= JSON_MAX_DEPTH ){
          वापस JSON_LOOKUP_TOODEEP;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i+1], 0);
        pParse->iDepth--;
        अगर( pParse->delta ) json_After_Edit_आकार_Adjust(pParse, i_मूल);
        वापस rc;
      }
      kk--;
      n = jsonb_Payload_आकार_Semi_Inline(pParse, j, &sz);
      अगर( n==0 ) वापस JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    अगर( j>iEnd ) वापस JSON_LOOKUP_ERROR;
    अगर( kk>0 ) वापस JSON_LOOKUP_NOTFOUND;
    अगर( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_SET );
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i+1]);
      अगर( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        निश्चित( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      अगर( pParse->delta ) json_After_Edit_आकार_Adjust(pParse, i_मूल);
      वापस rc;
    }
  }अन्यथा{
    वापस JSON_LOOKUP_PATHERROR; 
  }
  वापस JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
अटल व्योम jsonReturnTextJsonFromBlob(
  sqlite3_context *ctx,
  स्थिर u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  अगर( NEVER(aBlob==0) ) वापस;
  स_रखो(&x, 0, माप(x));
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
अटल व्योम jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  पूर्णांक eMode                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  पूर्णांक rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  निश्चित( eMode>=0 && eMode<=2 );
  n = jsonb_Payload_आकार(pParse, i, &sz);
  अगर( n==0 ){
    sqlite3_परिणाम_error(pCtx, "malformed JSON", -1);
    वापस;
  }
  चयन( pParse->aBlob[i] & 0x0f ){
    हाल JSONB_NULL: {
      अगर( sz ) जाओ returnfromblob_malformed;
      sqlite3_परिणाम_null(pCtx);
      अवरोध;
    }
    हाल JSONB_TRUE: {
      अगर( sz ) जाओ returnfromblob_malformed;
      sqlite3_परिणाम_int(pCtx, 1);
      अवरोध;
    }
    हाल JSONB_FALSE: {
      अगर( sz ) जाओ returnfromblob_malformed;
      sqlite3_परिणाम_int(pCtx, 0);
      अवरोध;
    }
    हाल JSONB_INT5:
    हाल JSONB_INT: {
      sqlite3_int64 iRes = 0;
      अक्षर *z;
      पूर्णांक bNeg = 0;
      अक्षर x;
      अगर( sz==0 ) जाओ returnfromblob_malformed;
      x = (अक्षर)pParse->aBlob[i+n];
      अगर( x=='-' ){
        अगर( sz<2 ) जाओ returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (स्थिर अक्षर*)&pParse->aBlob[i+n], (पूर्णांक)sz);
      अगर( z==0 ) जाओ returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      अगर( rc==0 ){
        अगर( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          द्विगुन r;
          r = (द्विगुन)*(sqlite3_uint64*)&iRes;
          sqlite3_परिणाम_double(pCtx, bNeg ? -r : r);
        }अन्यथा{
          sqlite3_परिणाम_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }अन्यथा अगर( rc==3 && bNeg ){
        sqlite3_परिणाम_int64(pCtx, SMALLEST_INT64);
      }अन्यथा अगर( rc==1 ){
        जाओ returnfromblob_malformed;
      }अन्यथा{
        अगर( bNeg ){ n--; sz++; }
        जाओ to_double;
      }
      अवरोध;
    }
    हाल JSONB_FLOAT5:
    हाल JSONB_FLOAT: {
      द्विगुन r;
      अक्षर *z;
      अगर( sz==0 ) जाओ returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (स्थिर अक्षर*)&pParse->aBlob[i+n], (पूर्णांक)sz);
      अगर( z==0 ) जाओ returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      अगर( rc<=0 ) जाओ returnfromblob_malformed;
      sqlite3_परिणाम_double(pCtx, r);
      अवरोध;
    }
    हाल JSONB_TEXTRAW:
    हाल JSONB_TEXT: {
      sqlite3_परिणाम_text(pCtx, (अक्षर*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      अवरोध;
    }
    हाल JSONB_TEXT5:
    हाल JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 iIn, iOut;
      स्थिर अक्षर *z;
      अक्षर *zOut;
      u32 nOut = sz;
      z = (स्थिर अक्षर*)&pParse->aBlob[i+n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut)+1);
      अगर( zOut==0 ) जाओ returnfromblob_oom;
      क्रम(iIn=iOut=0; iIn<sz; iIn++){
        अक्षर c = z[iIn];
        अगर( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz-iIn, &v);
          अगर( v==0 && iIn+szEscape==sz
           && (z[iIn+1]=='\n' || z[iIn+1]=='\r' || (u8)z[iIn+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }अन्यथा अगर( v<=0x7f ){
            zOut[iOut++] = (अक्षर)v;
          }अन्यथा अगर( v<=0x7ff ){
            निश्चित( szEscape>=2 );
            zOut[iOut++] = (अक्षर)(0xc0 | (v>>6));
            zOut[iOut++] = 0x80 | (v&0x3f);
          }अन्यथा अगर( v<0x10000 ){
            निश्चित( szEscape>=3 );
            zOut[iOut++] = 0xe0 | (v>>12);
            zOut[iOut++] = 0x80 | ((v>>6)&0x3f);
            zOut[iOut++] = 0x80 | (v&0x3f);
          }अन्यथा अगर( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }अन्यथा{
            निश्चित( szEscape>=4 );
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
      निश्चित( iOut<=nOut );
      zOut[iOut] = 0;
      sqlite3_परिणाम_text(pCtx, zOut, iOut, SQLITE_DYNAMIC);
      अवरोध;
    }
    हाल JSONB_ARRAY:
    हाल JSONB_OBJECT: {
      अगर( eMode==0 ){
        अगर( (SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(pCtx)) & JSON_BLOB)!=0 ){
          eMode = 2;
        }अन्यथा{
          eMode = 1;
        }
      }
      अगर( eMode==2 ){
        sqlite3_परिणाम_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }अन्यथा{
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz+n);
      }
      अवरोध;
    }
    शेष: {
      जाओ returnfromblob_malformed;
    }
  }
  वापस;

returnfromblob_oom:
  sqlite3_परिणाम_error_nomem(pCtx);
  वापस;

returnfromblob_malformed:
  sqlite3_परिणाम_error(pCtx, "malformed JSON", -1);
  वापस;
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
अटल पूर्णांक jsonFunctionArgToBlob(
  sqlite3_context *ctx,
  sqlite3_मूल्य *pArg,
  JsonParse *pParse
){
  पूर्णांक eType = sqlite3_मूल्य_type(pArg);
  अटल u8 aNull[] = { 0x00 };
  स_रखो(pParse, 0, माप(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  चयन( eType ){
    शेष: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      वापस 0;
    }
    हाल SQLITE_BLOB: {
      अगर( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_परिणाम_error(ctx, "JSON cannot hold BLOB values", -1);
        वापस 1;
      }
      अवरोध;
    }
    हाल SQLITE_TEXT: {
      स्थिर अक्षर *zJson = (स्थिर अक्षर*)sqlite3_मूल्य_text(pArg);
      पूर्णांक nJson = sqlite3_मूल्य_bytes(pArg);
      अगर( zJson==0 ) वापस 1;
      अगर( sqlite3_मूल्य_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (अक्षर*)zJson;
        pParse->nJson = nJson;
        अगर( jsonConvertTextToBlob(pParse, ctx) ){
          sqlite3_परिणाम_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          स_रखो(pParse, 0, माप(pParse[0]));
          वापस 1;
        }
      }अन्यथा{
        json_Blob_Append_गाँठ(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      अवरोध;
    }
    हाल SQLITE_FLOAT: {
      अगर( NEVER(sqlite3IsNaN(sqlite3_मूल्य_double(pArg))) ){
        json_Blob_Append_गाँठ(pParse, JSONB_NULL, 0, 0);
      }अन्यथा{
        पूर्णांक n = sqlite3_मूल्य_bytes(pArg);
        स्थिर अक्षर *z = (स्थिर अक्षर*)sqlite3_मूल्य_text(pArg);
        अगर( z==0 ) वापस 1;
        अगर( z[0]=='I' ){
          json_Blob_Append_गाँठ(pParse, JSONB_FLOAT, 5, "9e999");
        }अन्यथा अगर( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_गाँठ(pParse, JSONB_FLOAT, 6, "-9e999");
        }अन्यथा{
          json_Blob_Append_गाँठ(pParse, JSONB_FLOAT, n, z);
        }
      }
      अवरोध;
    }
    हाल SQLITE_INTEGER: {
      पूर्णांक n = sqlite3_मूल्य_bytes(pArg);
      स्थिर अक्षर *z = (स्थिर अक्षर*)sqlite3_मूल्य_text(pArg);
      अगर( z==0 ) वापस 1;
      json_Blob_Append_गाँठ(pParse, JSONB_INT, n, z);
      अवरोध;
    }
  }
  अगर( pParse->oom ){
    sqlite3_परिणाम_error_nomem(ctx);
    वापस 1;
  }अन्यथा{
    वापस 0;
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
अटल अक्षर *jsonBadPathError(
  sqlite3_context *ctx,     /* The function call containing the error */
  स्थिर अक्षर *zPath,        /* The path with the problem */
  पूर्णांक rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  अक्षर *z_संदेश;
  अगर( rc==(पूर्णांक)JSON_LOOKUP_NOTARRAY ){
    z_संदेश = sqlite3_mprintf("not an array element: %Q", zPath);
  }अन्यथा अगर( rc==(पूर्णांक)JSON_LOOKUP_ERROR ){
    z_संदेश = sqlite3_mprintf("malformed JSON");
  }अन्यथा अगर( rc==(पूर्णांक)JSON_LOOKUP_TOODEEP ){
    z_संदेश = sqlite3_mprintf("JSON path too deep");
  }अन्यथा{
    z_संदेश = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  अगर( ctx==0 ) वापस z_संदेश;
  अगर( z_संदेश ){
    sqlite3_परिणाम_error(ctx, z_संदेश, -1);
    sqlite3_free(z_संदेश);
  }अन्यथा{
    sqlite3_परिणाम_error_nomem(ctx);
  }
  वापस 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
अटल व्योम json_जोड़ो_Into_Blob(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv,
  पूर्णांक eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  पूर्णांक i;
  u32 rc = 0;
  स्थिर अक्षर *zPath = 0;
  पूर्णांक flgs;
  JsonParse *p;
  JsonParse ax;

  निश्चित( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  अगर( p==0 ) वापस;
  क्रम(i=1; i<argc-1; i+=2){
    अगर( sqlite3_मूल्य_type(argv[i])==SQLITE_NULL ) जारी;
    zPath = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
    अगर( zPath==0 ){
      sqlite3_परिणाम_error_nomem(ctx);
      jsonParseFree(p);
      वापस;
    }
    अगर( zPath[0]!='$' ) जाओ json_जोड़ो_Into_Blob_patherror;
    अगर( jsonFunctionArgToBlob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      वापस;
    }
    अगर( zPath[1]==0 ){
      अगर( eEdit==JEDIT_REPL || eEdit==JEDIT_SET ){
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
    अगर( rc==JSON_LOOKUP_NOTFOUND ) जारी;
    अगर( JSON_LOOKUP_ISERROR(rc) ) जाओ json_जोड़ो_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  वापस;

json_जोड़ो_Into_Blob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  वापस;
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
अटल पूर्णांक jsonArgIsJsonb(sqlite3_मूल्य *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  अगर( sqlite3_मूल्य_type(pArg)!=SQLITE_BLOB ) वापस 0;
  p->aBlob = (u8*)sqlite3_मूल्य_blob(pArg);
  p->nBlob = (u32)sqlite3_मूल्य_bytes(pArg);
  अगर( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonb_Payload_आकार(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    वापस 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  वापस 0;
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
अटल JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_मूल्य *pArg,
  u32 flgs
){
  पूर्णांक eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  निश्चित( ctx!=0 );
  eType = sqlite3_मूल्य_type(pArg);
  अगर( eType==SQLITE_NULL ){
    वापस 0;
  }
  अगर( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_खोज(ctx, pArg);
  }
  अगर( pFromCache ){
    pFromCache->nJPRef++;
    अगर( (flgs & JSON_EDITABLE)==0 ){
      वापस pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, माप(*p));
  अगर( p==0 ) जाओ json_pfa_oom;
  स_रखो(p, 0, माप(*p));
  p->db = db;
  p->nJPRef = 1;
  अगर( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    अगर( p->aBlob==0 ) जाओ json_pfa_oom;
    स_नकल(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    वापस p;
  }
  अगर( eType==SQLITE_BLOB ){
    अगर( jsonArgIsJsonb(pArg,p) ){
      अगर( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        जाओ json_pfa_oom;
      }
      वापस p;
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
  अगर( db->mallocFailed ) जाओ json_pfa_oom;
  अगर( p->nJson==0 ) जाओ json_pfa_malformed;
  निश्चित( p->zJson!=0 );
  अगर( jsonConvertTextToBlob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    अगर( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      वापस p;
    }अन्यथा{
      jsonParseFree(p);
      वापस 0;
    }
  }अन्यथा{
    पूर्णांक isRCStr = sqlite3_मूल्य_Is_Of_Class(pArg, sqlite3RCStrUnref);
    पूर्णांक rc;
    अगर( !isRCStr ){
      अक्षर *zNew = sqlite3RCStrNew( p->nJson );
      अगर( zNew==0 ) जाओ json_pfa_oom;
      स_नकल(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }अन्यथा{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_जोड़ो(ctx, p);
    अगर( rc==SQLITE_NOMEM ) जाओ json_pfa_oom;
    अगर( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      जाओ rebuild_from_cache;
    }
  }
  वापस p;

json_pfa_malformed:
  अगर( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    वापस p;
  }अन्यथा{
    jsonParseFree(p);
    sqlite3_परिणाम_error(ctx, "malformed JSON", -1);
    वापस 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_परिणाम_error_nomem(ctx);
  वापस 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
अटल व्योम jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  पूर्णांक flgs;
  अगर( p->oom ){
    sqlite3_परिणाम_error_nomem(ctx);
    वापस;
  }
  flgs = SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(ctx));
  अगर( flgs & JSON_BLOB ){
    अगर( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_परिणाम_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }अन्यथा{
      sqlite3_परिणाम_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }अन्यथा{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    jsonTranslateBlobToText(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
  }
}

/****************************************************************************
** SQL functions used for testing and debugging
****************************************************************************/

#अगर SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
अटल व्योम jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 iStart,        /* Start rendering here */
  u32 iEnd,          /* Do not render this byte or any byte after this one */
  पूर्णांक nIndent,       /* Indent by this many spaces */
  sqlite3_str *pOut  /* Generate output into this sqlite3_str object */
){
  जबतक( iStart<iEnd ){
    u32 i, n, nn, sz = 0;
    पूर्णांक showContent = 1;
    u8 x = pParse->aBlob[iStart] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(pOut, "%5d:%*s", iStart, nIndent, "");
    अगर( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonb_Payload_आकार(pParse, iStart, &sz);
    अगर( nn==0 ) nn = 1;
    अगर( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    क्रम(i=0; i<nn; i++){
      sqlite3_str_appendf(pOut, " %02x", pParse->aBlob[iStart+i]);
    }
    अगर( n==0 ){
      sqlite3_str_appendf(pOut, "   ERROR invalid node size\n");
      iStart = n==0 ? iStart+1 : iEnd;
      जारी;
    }
    pParse->nBlob = savedNBlob;
    अगर( iStart+n+sz>iEnd ){
      iEnd = iStart+n+sz;
      अगर( iEnd>pParse->nBlob ){
        अगर( pParse->nBlobAlloc>0 && iEnd>pParse->nBlobAlloc ){
          iEnd = pParse->nBlobAlloc;
        }अन्यथा{
          iEnd = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(pOut,"  <-- ");
    चयन( x ){
      हाल JSONB_NULL:     sqlite3_str_appendall(pOut,"null"); अवरोध;
      हाल JSONB_TRUE:     sqlite3_str_appendall(pOut,"true"); अवरोध;
      हाल JSONB_FALSE:    sqlite3_str_appendall(pOut,"false"); अवरोध;
      हाल JSONB_INT:      sqlite3_str_appendall(pOut,"int"); अवरोध;
      हाल JSONB_INT5:     sqlite3_str_appendall(pOut,"int5"); अवरोध;
      हाल JSONB_FLOAT:    sqlite3_str_appendall(pOut,"float"); अवरोध;
      हाल JSONB_FLOAT5:   sqlite3_str_appendall(pOut,"float5"); अवरोध;
      हाल JSONB_TEXT:     sqlite3_str_appendall(pOut,"text"); अवरोध;
      हाल JSONB_TEXTJ:    sqlite3_str_appendall(pOut,"textj"); अवरोध;
      हाल JSONB_TEXT5:    sqlite3_str_appendall(pOut,"text5"); अवरोध;
      हाल JSONB_TEXTRAW:  sqlite3_str_appendall(pOut,"textraw"); अवरोध;
      हाल JSONB_ARRAY: {
        sqlite3_str_appendf(pOut,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        अवरोध;
      }
      हाल JSONB_OBJECT: {
        sqlite3_str_appendf(pOut, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, iStart+n, iStart+n+sz, nIndent+2, pOut);
        showContent = 0;
        अवरोध;
      }
      शेष: {
        sqlite3_str_appendall(pOut, "ERROR: unknown node type\n");
        showContent = 0;
        अवरोध;
      }
    }
    अगर( showContent ){
      अगर( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(pOut, "\n", 1);
      }अन्यथा{
        u32 j;
        sqlite3_str_appendall(pOut, ": \"");
        क्रम(j=iStart+n; j<iStart+n+sz; j++){
          u8 c = pParse->aBlob[j];
          अगर( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(pOut, (अक्षर*)&c, 1);
        }
        sqlite3_str_append(pOut, "\"\n", 2);
      }
    }
    iStart += n + sz;
  }
}
अटल व्योम jsonShowParse(JsonParse *pParse){
  sqlite3_str out;
  अक्षर zBuf[1000];
  अगर( pParse==0 ){
    म_लिखो("NULL pointer\n");
    वापस;
  }अन्यथा{
    म_लिखो("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    म_लिखो("nBlob = %u\n", pParse->nBlob);
    म_लिखो("delta = %d\n", pParse->delta);
    अगर( pParse->nBlob==0 ) वापस;
    म_लिखो("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&out, 0, zBuf, माप(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &out);
  म_लिखो("%s", sqlite3_str_मूल्य(&out));
  sqlite3_str_reset(&out);
}
#पूर्ण_अगर /* SQLITE_DEBUG */

#अगर_घोषित SQLITE_DEBUG
/*
** SQL function:   json_parse(JSON)
**
** Parse JSON using jsonParseFuncArg().  Return text that is a
** human-readable dump of the binary JSONB for the input parameter.
*/
अटल व्योम jsonParseFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str out;

  निश्चित( argc>=1 );
  sqlite3StrAccumInit(&out, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  अगर( p==0 ) वापस;
  अगर( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &out);
    sqlite3_परिणाम_text64(ctx,out.zText,out.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }अन्यथा{
    jsonShowParse(p);
  }
  jsonParseFree(p);
  sqlite3_str_reset(&out);
}
#पूर्ण_अगर /* SQLITE_DEBUG */

/****************************************************************************
** Scalar SQL function implementations
****************************************************************************/

/*
** Implementation of the json_quote(VALUE) function.  Return a JSON value
** corresponding to the SQL value input.  Mostly this means putting
** double-quotes around strings and returning the unquoted string "null"
** when given a NULL input.
*/
अटल व्योम jsonQuoteFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonString jx;
  UNUSED_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_मूल्य(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
अटल व्योम jsonArrayFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  क्रम(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_मूल्य(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
अटल व्योम json_Array_लंबाई_Func(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  अगर( p==0 ) वापस;
  अगर( argc==2 ){
    स्थिर अक्षर *zPath = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]);
    अगर( zPath==0 ){
      jsonParseFree(p);
      वापस;
    }
    i = jsonLookupStep(p, 0, zPath[0]=='$' ? zPath+1 : "@", 0);
    अगर( JSON_LOOKUP_ISERROR(i) ){
      अगर( i==JSON_LOOKUP_NOTFOUND ){
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
  अगर( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_गिनती(p, i);
  }
  अगर( !eErr ) sqlite3_परिणाम_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
अटल पूर्णांक jsonAllAlphanum(स्थिर अक्षर *z, पूर्णांक n){
  पूर्णांक i;
  क्रम(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  वापस i==n;
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
अटल व्योम jsonExtractFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p = 0;      /* The parse */
  पूर्णांक flags;             /* Flags associated with the function */
  पूर्णांक i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  अगर( argc<2 ) वापस;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  अगर( p==0 ) वापस;
  flags = SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(ctx));
  jsonStringInit(&jx, ctx);
  अगर( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  क्रम(i=1; i<argc; i++){
    /* With a single PATH argument */
    स्थिर अक्षर *zPath = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
    पूर्णांक nPath;
    u32 j;
    अगर( zPath==0 ) जाओ json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    अगर( zPath[0]=='$' ){
      j = jsonLookupStep(p, 0, zPath+1, 0);
    }अन्यथा अगर( (flags & JSON_ABPATH) ){
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
      अगर( sqlite3_मूल्य_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        अगर( zPath[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      }अन्यथा अगर( jsonAllAlphanum(zPath, nPath) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      }अन्यथा अगर( zPath[0]=='[' && nPath>=3 && zPath[nPath-1]==']' ){
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
      जाओ json_extract_error;
    }
    अगर( j<p->nBlob ){
      अगर( argc==2 ){
        अगर( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          निश्चित( (flags & JSON_BLOB)==0 );
          sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
        }अन्यथा{
          jsonReturnFromBlob(p, j, ctx, 0);
          अगर( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }अन्यथा{
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    }अन्यथा अगर( j==JSON_LOOKUP_NOTFOUND ){
      अगर( argc==2 ){
        जाओ json_extract_error;  /* Return NULL if not found */
      }अन्यथा{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }अन्यथा{
      jsonBadPathError(ctx, zPath, j);
      जाओ json_extract_error;
    }
  }
  अगर( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    अगर( (flags & JSON_BLOB)==0 ){
      sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  वापस;
}

/*
** Return codes for jsonMergePatch()
*/
#घोषणा JSON_MERGE_OK          0     /* Success */
#घोषणा JSON_MERGE_BADTARGET   1     /* Malformed TARGET blob */
#घोषणा JSON_MERGE_BADPATCH    2     /* Malformed PATCH blob */
#घोषणा JSON_MERGE_OOM         3     /* Out-of-memory condition */
#घोषणा JSON_MERGE_TOODEEP     4     /* Nested too deep */

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
अटल पूर्णांक jsonMergePatch(
  JsonParse *pTarget,      /* The JSON parser that contains the TARGET */
  u32 iTarget,             /* Index of TARGET in pTarget->aBlob[] */
  स्थिर JsonParse *pPatch, /* The PATCH */
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

  निश्चित( iTarget>=0 && iTarget<pTarget->nBlob );
  निश्चित( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  अगर( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 szTarget;       /* Total size of the target, header+payload */
    n = jsonb_Payload_आकार(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonb_Payload_आकार(pTarget, iTarget, &sz);
    szTarget = n+sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob+iPatch, szPatch);
    वापस pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;  /* Line 03 */
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  अगर( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonb_Payload_आकार(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget+n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonb_Payload_आकार(pPatch, iPatch, &sz);
  अगर( NEVER(n==0) ) वापस JSON_MERGE_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonb_Payload_आकार(pTarget, iTarget, &sz);
  अगर( NEVER(n==0) ) वापस JSON_MERGE_BADTARGET;
  iTStart = iTarget+n;
  iTEndBE = iTStart+sz;

  जबतक( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    अगर( ePLabel<JSONB_TEXT || ePLabel>JSONB_TEXTRAW ){
      वापस JSON_MERGE_BADPATCH;
    }
    nPLabel = jsonb_Payload_आकार(pPatch, iPCursor, &szPLabel);
    अगर( nPLabel==0 ) वापस JSON_MERGE_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    अगर( iPValue>=iPEnd ) वापस JSON_MERGE_BADPATCH;
    nPValue = jsonb_Payload_आकार(pPatch, iPValue, &szPValue);
    अगर( nPValue==0 ) वापस JSON_MERGE_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    अगर( iPCursor>iPEnd ) वापस JSON_MERGE_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    जबतक( iTCursor<iTEnd ){
      पूर्णांक isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      अगर( eTLabel<JSONB_TEXT || eTLabel>JSONB_TEXTRAW ){
        वापस JSON_MERGE_BADTARGET;
      }
      nTLabel = jsonb_Payload_आकार(pTarget, iTCursor, &szTLabel);
      अगर( nTLabel==0 ) वापस JSON_MERGE_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      अगर( iTValue>=iTEnd ) वापस JSON_MERGE_BADTARGET;
      nTValue = jsonb_Payload_आकार(pTarget, iTValue, &szTValue);
      अगर( nTValue==0 ) वापस JSON_MERGE_BADTARGET;
      अगर( iTValue + nTValue + szTValue > iTEnd ) वापस JSON_MERGE_BADTARGET;
      isEqual = jsonLabelCompare(
                   (स्थिर अक्षर*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_TEXT || ePLabel==JSONB_TEXTRAW),
                   (स्थिर अक्षर*)&pTarget->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_TEXT || eTLabel==JSONB_TEXTRAW));
      अगर( isEqual ) अवरोध;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    अगर( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      अगर( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(pTarget, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        अगर( NEVER(pTarget->oom) ) वापस JSON_MERGE_OOM;
      }अन्यथा{
        /* Algorithm line 12 */
        पूर्णांक rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        अगर( iDepth>=JSON_MAX_DEPTH ) वापस JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth+1);
        अगर( rc ) वापस rc;
        pTarget->delta += savedDelta;
      }        
    }अन्यथा अगर( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      अगर( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue+nPValue+szNew);
        अगर( pTarget->oom ) वापस JSON_MERGE_OOM;
        स_नकल(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        स_नकल(&pTarget->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }अन्यथा{
        पूर्णांक rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew+1);
        अगर( pTarget->oom ) वापस JSON_MERGE_OOM;
        स_नकल(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        अगर( iDepth>=JSON_MAX_DEPTH ) वापस JSON_MERGE_TOODEEP;
        rc = jsonMergePatch(pTarget, iTEnd+szNew,pPatch,iPValue,iDepth+1);
        अगर( rc ) वापस rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  अगर( pTarget->delta ) json_After_Edit_आकार_Adjust(pTarget, iTarget);
  वापस pTarget->oom ? JSON_MERGE_OOM : JSON_MERGE_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
अटल व्योम jsonPatchFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *pTarget;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  पूर्णांक rc;                /* Result code */

  UNUSED_PARAMETER(argc);
  निश्चित( argc==2 );
  pTarget = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  अगर( pTarget==0 ) वापस;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  अगर( pPatch ){
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    अगर( rc==JSON_MERGE_OK ){
      jsonReturnParse(ctx, pTarget);
    }अन्यथा अगर( rc==JSON_MERGE_OOM ){
      sqlite3_परिणाम_error_nomem(ctx);
    }अन्यथा अगर( rc==JSON_MERGE_TOODEEP ){
      sqlite3_परिणाम_error(ctx, "JSON nested too deep", -1);
    }अन्यथा{
      sqlite3_परिणाम_error(ctx, "malformed JSON", -1);
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
अटल व्योम jsonObjectFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;
  JsonString jx;
  स्थिर अक्षर *z;
  u32 n;

  अगर( argc&1 ){
    sqlite3_परिणाम_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    वापस;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  क्रम(i=0; i<argc; i+=2){
    अगर( sqlite3_मूल्य_type(argv[i])!=SQLITE_TEXT ){
      sqlite3_परिणाम_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      वापस;
    }
    jsonAppendSeparator(&jx);
    z = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
    n = sqlite3_मूल्य_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_मूल्य(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
}


/*
** json_remove(JSON, PATH, ...)
**
** Remove the named elements from JSON and return the result.  malformed
** JSON or PATH arguments result in an error.
*/
अटल व्योम jsonRemoveFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;          /* The parse */
  स्थिर अक्षर *zPath = 0; /* Path of element to be removed */
  पूर्णांक i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  अगर( argc<1 ) वापस;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  अगर( p==0 ) वापस;
  क्रम(i=1; i<argc; i++){
    zPath = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
    अगर( zPath==0 ){
      जाओ json_remove_done;
    }
    अगर( zPath[0]!='$' ){
      जाओ json_remove_patherror;
    }
    अगर( zPath[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      जाओ json_remove_done;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath+1, 0);
    अगर( JSON_LOOKUP_ISERROR(rc) ){
      अगर( rc==JSON_LOOKUP_NOTFOUND ){
        जारी;  /* No-op */
      }अन्यथा{
        jsonBadPathError(ctx, zPath, rc);
      }
      जाओ json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  वापस;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  वापस;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
अटल व्योम jsonReplaceFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  अगर( argc<1 ) वापस;
  अगर( (argc&1)==0 ) {
    json_Wrong_अंक_Args(ctx, "replace");
    वापस;
  }
  json_जोड़ो_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
अटल व्योम jsonSetFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक flags = SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(ctx));
  पूर्णांक eInsType = JSON_जोड़ो_TYPE(flags);
  अटल स्थिर अक्षर *azInsType[] = { "insert", "set", "array_insert" };
  अटल स्थिर u8 aEditType[] = { JEDIT_INS, JEDIT_SET, JEDIT_AINS };

  अगर( argc<1 ) वापस;
  निश्चित( eInsType>=0 && eInsType<=2 );
  अगर( (argc&1)==0 ) {
    json_Wrong_अंक_Args(ctx, azInsType[eInsType]);
    वापस;
  }
  json_जोड़ो_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
अटल व्योम jsonTypeFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;          /* The parse */
  स्थिर अक्षर *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  अगर( p==0 ) वापस;
  अगर( argc==2 ){
    zPath = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]);
    अगर( zPath==0 ) जाओ json_type_done;
    अगर( zPath[0]!='$' ){
      jsonBadPathError(ctx, zPath, 0);
      जाओ json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath+1, 0);
    अगर( JSON_LOOKUP_ISERROR(i) ){
      अगर( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }अन्यथा{
        jsonBadPathError(ctx, zPath, i);
      }
      जाओ json_type_done;
    }
  }अन्यथा{
    i = 0;
  }
  sqlite3_परिणाम_text(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
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
अटल व्योम jsonPrettyFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  स_रखो(&x, 0, माप(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  अगर( x.pParse==0 ) वापस;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  अगर( argc==1 || (x.zIndent = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]))==0 ){
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
अटल व्योम jsonValidFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  अगर( argc==2 ){
    i64 f = sqlite3_मूल्य_int64(argv[1]);
    अगर( f<1 || f>15 ){
      sqlite3_परिणाम_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      वापस;
    }
    flags = f & 0x0f;
  }
  चयन( sqlite3_मूल्य_type(argv[0]) ){
    हाल SQLITE_NULL: {
#अगर_घोषित SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_परिणाम_int(ctx, 0);
#पूर्ण_अगर
      वापस;
    }
    हाल SQLITE_BLOB: {
      JsonParse py;
      स_रखो(&py, 0, माप(py));
      अगर( jsonArgIsJsonb(argv[0], &py) ){
        अगर( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }अन्यथा अगर( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        अवरोध;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    शेष: {
      JsonParse px;
      अगर( (flags & 0x3)==0 ) अवरोध;
      स_रखो(&px, 0, माप(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      अगर( p ){
        अगर( p->oom ){
          sqlite3_परिणाम_error_nomem(ctx);
        }अन्यथा अगर( p->nErr ){
          /* no-op */
        }अन्यथा अगर( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }अन्यथा{
        sqlite3_परिणाम_error_nomem(ctx);
      }
      अवरोध;
    }
  }
  sqlite3_परिणाम_int(ctx, res);
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
अटल व्योम jsonErrorFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  स_रखो(&s, 0, माप(s));
  s.db = sqlite3_context_db_handle(ctx);
  अगर( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }अन्यथा{
    s.zJson = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
    अगर( s.zJson==0 ) वापस;  /* NULL input or OOM */
    s.nJson = sqlite3_मूल्य_bytes(argv[0]);
    अगर( jsonConvertTextToBlob(&s,0) ){
      अगर( s.oom ){
        iErrPos = -1;
      }अन्यथा{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        निश्चित( s.zJson!=0 );  /* Because s.oom is false */
        क्रम(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          अगर( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  अगर( iErrPos<0 ){
    sqlite3_परिणाम_error_nomem(ctx);
  }अन्यथा{
    sqlite3_परिणाम_int64(ctx, iErrPos);
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
अटल व्योम jsonArrayStep(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonString *pStr;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, माप(*pStr));
  अगर( pStr ){
    अगर( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }अन्यथा अगर( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_मूल्य(pStr, argv[0]);
  }
}
अटल व्योम jsonArrayCompute(sqlite3_context *ctx, पूर्णांक isFinal){
  JsonString *pStr;
  पूर्णांक flags = SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  अगर( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    अगर( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      वापस;
    }अन्यथा अगर( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      अगर( isFinal ){
        अगर( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }अन्यथा{
        jsonStringTrimOneChar(pStr);
      }
      वापस;
    }अन्यथा अगर( isFinal ){
      sqlite3_परिणाम_text(ctx, pStr->zBuf, (पूर्णांक)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }अन्यथा{
      sqlite3_परिणाम_text(ctx, pStr->zBuf, (पूर्णांक)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }अन्यथा अगर( flags & JSON_BLOB ){
    अटल स्थिर u8 emptyArray = 0x0b;
    sqlite3_परिणाम_blob(ctx, &emptyArray, 1, SQLITE_STATIC);
  }अन्यथा{
    sqlite3_परिणाम_text(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
}
अटल व्योम json_Array_मूल्य(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
अटल व्योम jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
अटल व्योम jsonGroupInverse(
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
  अगर( NEVER(!pStr) ) वापस;
  z = pStr->zBuf;
  क्रम(i=1; i<pStr->nUsed && ((c = z[i])!=',' || inStr || nNest); i++){
    अगर( c=='"' ){
      inStr = !inStr;
    }अन्यथा अगर( c=='\\' ){
      i++;
    }अन्यथा अगर( !inStr ){
      अगर( c=='{' || c=='[' ) nNest++;
      अगर( c=='}' || c==']' ) nNest--;
    }
  }
  अगर( i<pStr->nUsed ){
    pStr->nUsed -= i;
    स_हटाओ(&z[1], &z[i+1], (माप_प्रकार)pStr->nUsed-1);
    z[pStr->nUsed] = 0;
  }अन्यथा{
    pStr->nUsed = 1;
  }
}
#अन्यथा
# define jsonGroupInverse 0
#पूर्ण_अगर


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
अटल व्योम jsonObjectStep(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  JsonString *pStr;
  स्थिर अक्षर *z;
  u32 n;
  UNUSED_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, माप(*pStr));
  अगर( pStr ){
    z = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
    n = sqlite3Strlen30(z);
    अगर( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }अन्यथा अगर( pStr->nUsed>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    अगर( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_मूल्य(pStr, argv[1]);
    }अन्यथा{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
अटल व्योम jsonObjectCompute(sqlite3_context *ctx, पूर्णांक isFinal){
  JsonString *pStr;
  पूर्णांक flags = SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  अगर( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    अगर( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      वापस;
    }
    अगर( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      पूर्णांक inStr = 0;
      अगर( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed+1);
        pStr = &tmpStr;
        अगर( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          वापस;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      क्रम(i=j=1; i<pStr->nUsed; i++){
        अक्षर c = pStr->zBuf[i];
        अगर( c=='"' ){
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        }अन्यथा अगर( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }अन्यथा अगर( c=='@' && !inStr ){
          निश्चित( i+1<pStr->nUsed );
          अगर( pStr->zBuf[i+1]==',' ){
            i++;
          }अन्यथा अगर( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }अन्यथा{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->nUsed = j;    /* Truncate the string */
    }
    अगर( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      अगर( isFinal ){
        अगर( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }अन्यथा{
        jsonStringTrimOneChar(pOgStr);
      }
    }अन्यथा अगर( isFinal ){
      sqlite3_परिणाम_text(ctx, pStr->zBuf, (पूर्णांक)pStr->nUsed,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }अन्यथा{
      sqlite3_परिणाम_text(ctx, pStr->zBuf, (पूर्णांक)pStr->nUsed, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    अगर( pStr!=pOgStr ) jsonStringReset(pStr);
  }अन्यथा अगर( flags & JSON_BLOB ){
    अटल स्थिर अचिन्हित अक्षर emptyObject = 0x0c;
    sqlite3_परिणाम_blob(ctx, &emptyObject, 1, SQLITE_STATIC); 
  }अन्यथा{
    sqlite3_परिणाम_text(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
}
अटल व्योम json_Object_मूल्य(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
अटल व्योम jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
प्रकार काष्ठा JsonParent JsonParent;
काष्ठा JsonParent {
  u32 i_शीर्ष;                 /* Start of object or array */
  u32 i_मूल्य;                /* Start of the value */
  u32 iEnd;                  /* First byte past the end */
  u32 nPath;                 /* Length of path */
  i64 i_कुंजी;                  /* Key for JSONB_ARRAY */
};

प्रकार काष्ठा JsonEachCursor JsonEachCursor;
काष्ठा JsonEachCursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 iEnd;                  /* EOF when i equals or exceeds this value */
  u32 n_मूल;                 /* Size of the root path in bytes */
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
प्रकार काष्ठा JsonEachConnection JsonEachConnection;
काष्ठा JsonEachConnection {
  sqlite3_vtab base;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 eMode;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
अटल पूर्णांक jsonEachConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  JsonEachConnection *pNew;
  पूर्णांक rc;

/* Column numbers */
#घोषणा JEACH_कुंजी     0
#घोषणा JEACH_मूल्य   1
#घोषणा JEACH_TYPE    2
#घोषणा JEACH_ATOM    3
#घोषणा JEACH_ID      4
#घोषणा JEACH_PARENT  5
#घोषणा JEACH_FULLKEY 6
#घोषणा JEACH_PATH    7
/* The xBestIndex method assumes that the JSON and ROOT columns are
** the last two columns in the table.  Should this ever changes, be
** sure to update the xBestIndex method. */
#घोषणा JEACH_JSON    8
#घोषणा JEACH_मूल    9

  UNUSED_PARAMETER(pzErr);
  UNUSED_PARAMETER(argv);
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  अगर( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, माप(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    अगर( pNew==0 ) वापस SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->eMode]=='t';
  }
  वापस rc;
}

/* destructor for json_each virtual table */
अटल पूर्णांक jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  वापस SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
अटल पूर्णांक jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  UNUSED_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, माप(*pCur));
  अगर( pCur==0 ) वापस SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  वापस SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
अटल व्योम jsonEachCursorReset(JsonEachCursor *p){
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
अटल पूर्णांक jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  वापस SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
अटल पूर्णांक jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  वापस p->i >= p->iEnd;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
अटल पूर्णांक jsonSkipLabel(JsonEachCursor *p){
  अगर( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonb_Payload_आकार(&p->sParse, p->i, &sz);
    sz += p->i + n;
    अगर( sz >= p->sParse.nBlob ) sz = p->i;
    वापस sz;
  }अन्यथा{
    वापस p->i;
  }
}

/*
** Append the path name for the current element.
*/
अटल व्योम jsonAppendPathName(JsonEachCursor *p){
  निश्चित( p->nParent>0 );
  निश्चित( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  अगर( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent-1].i_कुंजी);
  }अन्यथा{
    u32 n, sz = 0, k, i;
    स्थिर अक्षर *z;
    पूर्णांक needQuote = 0;
    n = jsonb_Payload_आकार(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (स्थिर अक्षर*)&p->sParse.aBlob[k];
    अगर( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }अन्यथा{
      क्रम(i=0; i<sz; i++){
        अगर( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          अवरोध;
        }
      }
    }
    अगर( needQuote ){
      jsonPrintf(sz+4,&p->path,".\"%.*s\"", sz, z);
    }अन्यथा{
      jsonPrintf(sz+2,&p->path,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
अटल पूर्णांक json_Each_Malformed_निवेश(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_संदेश);
  cur->pVtab->z_Err_संदेश = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  वापस cur->pVtab->z_Err_संदेश ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
अटल पूर्णांक json_Each_अग्र(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  पूर्णांक rc = SQLITE_OK;
  अगर( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonb_Payload_आकार(&p->sParse, i, &sz);
    अगर( n==0 )वापस json_Each_Malformed_निवेश(cur);
    अगर( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      JsonParent *pParent;
      अगर( p->nParent>=p->nParentAlloc ){
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->aParent, माप(JsonParent)*nNew);
        अगर( pNew==0 ) वापस SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->i_शीर्ष = p->i;
      pParent->i_मूल्य = i;
      pParent->iEnd = i + n + sz;
      pParent->i_कुंजी = -1;
      pParent->nPath = (u32)p->path.nUsed;
      अगर( p->eType && p->nParent ){
        jsonAppendPathName(p);
        अगर( p->path.eErr ) rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    }अन्यथा{
      p->i = i + n + sz;
    }
    जबतक( p->nParent>0 && p->i >= p->aParent[p->nParent-1].iEnd ){
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    अगर( levelChange ){
      अगर( p->nParent>0 ){
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
    n = jsonb_Payload_आकार(&p->sParse, i, &sz);
    अगर( n==0 )वापस json_Each_Malformed_निवेश(cur);
    p->i = i + n + sz;
  }
  अगर( p->eType==JSONB_ARRAY && p->nParent ){
    p->aParent[p->nParent-1].i_कुंजी++;
  }
  p->iRowid++;
  वापस rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
अटल पूर्णांक json_Each_Path_लंबाई(JsonEachCursor *p){
  u32 n = p->path.nUsed;
  अक्षर *z = p->path.zBuf;
  अगर( p->iRowid==0 && p->bRecursive && n>=2 ){
    जबतक( ALWAYS(n>1) ){
      n--;
      अगर( z[n]=='[' || z[n]=='.' ){
        u32 x;
        अक्षर cSaved = z[n];
        z[n] = 0;
        निश्चित( p->sParse.eEdit==0 );
        x = jsonLookupStep(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        अगर( !JSON_LOOKUP_ISERROR(x) ) अवरोध;
      }
    }
  }
  वापस n;
}

/* Return the value of a column */
अटल पूर्णांक jsonEachColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  पूर्णांक iColumn                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  चयन( iColumn ){
    हाल JEACH_कुंजी: {
      अगर( p->nParent==0 ){
        u32 n, j;
        अगर( p->n_मूल==1 ) अवरोध;
        j = json_Each_Path_लंबाई(p);
        n = p->n_मूल - j;
        अगर( n==0 ){
          अवरोध;
        }अन्यथा अगर( p->path.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_परिणाम_int64(ctx, x);
        }अन्यथा अगर( p->path.zBuf[j+1]=='"' ){
          sqlite3_परिणाम_text(ctx, &p->path.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }अन्यथा{
          sqlite3_परिणाम_text(ctx, &p->path.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        अवरोध;
      }
      अगर( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }अन्यथा{
        निश्चित( p->eType==JSONB_ARRAY );
        sqlite3_परिणाम_int64(ctx, p->aParent[p->nParent-1].i_कुंजी);
      }
      अवरोध;
    }
    हाल JEACH_मूल्य: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      अगर( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_परिणाम_subtype(ctx, JSON_SUBTYPE);
      }
      अवरोध;
    }
    हाल JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_परिणाम_text(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      अवरोध;
    }
    हाल JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      अगर( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      अवरोध;
    }
    हाल JEACH_ID: {
      sqlite3_परिणाम_int64(ctx, (sqlite3_int64)p->i);
      अवरोध;
    }
    हाल JEACH_PARENT: {
      अगर( p->nParent>0 && p->bRecursive ){
        sqlite3_परिणाम_int64(ctx, p->aParent[p->nParent-1].i_शीर्ष);
      }
      अवरोध;
    }
    हाल JEACH_FULLKEY: {
      u64 nBase = p->path.nUsed;
      अगर( p->nParent ) jsonAppendPathName(p);
      sqlite3_परिणाम_text64(ctx, p->path.zBuf, p->path.nUsed,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->path.nUsed = nBase;
      अवरोध;
    }
    हाल JEACH_PATH: {
      u32 n = json_Each_Path_लंबाई(p);
      sqlite3_परिणाम_text64(ctx, p->path.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      अवरोध;
    }
    शेष: {
      sqlite3_परिणाम_text(ctx, p->path.zBuf, p->n_मूल, SQLITE_TRANSIENT);
      अवरोध;
    }
    हाल JEACH_JSON: {
      अगर( p->sParse.zJson==0 ){
        sqlite3_परिणाम_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }अन्यथा{
        sqlite3_परिणाम_text(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      अवरोध;
    }
  }
  वापस SQLITE_OK;
}

/* Return the current rowid value */
अटल पूर्णांक jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  वापस SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
अटल पूर्णांक json_Each_Best_सूचकांक(
  sqlite3_vtab *tab,
  sqlite3_सूचकांक_info *pIdxInfo
){
  पूर्णांक i;                     /* Loop counter or computed array index */
  पूर्णांक aIdx[2];               /* Index of constraints for JSON and ROOT */
  पूर्णांक unusableMask = 0;      /* Mask of unusable JSON and ROOT constraints */
  पूर्णांक idxMask = 0;           /* Mask of usable == constraints JSON and ROOT */
  स्थिर काष्ठा sqlite3_सूचकांक_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  निश्चित( JEACH_मूल == JEACH_JSON+1 );
  UNUSED_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  क्रम(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    पूर्णांक iCol;
    पूर्णांक iMask;
    अगर( pConstraint->iColumn < JEACH_JSON ) जारी;
    iCol = pConstraint->iColumn - JEACH_JSON;
    निश्चित( iCol==0 || iCol==1 );
    testcase( iCol==0 );
    iMask = 1 << iCol;
    अगर( pConstraint->usable==0 ){
      unusableMask |= iMask;
    }अन्यथा अगर( pConstraint->op==SQLITE_सूचकांक_CONSTRAINT_EQ ){
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  अगर( pIdxInfo->nOrderBy>0
   && pIdxInfo->aOrderBy[0].iColumn<0
   && pIdxInfo->aOrderBy[0].desc==0
  ){
    pIdxInfo->orderByConsumed = 1;
  }

  अगर( (unusableMask & ~idxMask)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    वापस SQLITE_CONSTRAINT;
  }
  अगर( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_अंक = 0;
  }अन्यथा{
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argv_सूचकांक = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    अगर( aIdx[1]<0 ){
      pIdxInfo->idx_अंक = 1;  /* Only JSON supplied.  Plan 1 */
    }अन्यथा{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argv_सूचकांक = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_अंक = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  वापस SQLITE_OK;
}

/* Start a search on a new JSON string */
अटल पूर्णांक jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  स्थिर अक्षर *z_मूल = 0;
  u32 i, n, sz;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(argc);
  jsonEachCursorReset(p);
  अगर( idx_अंक==0 ) वापस SQLITE_OK;
  स_रखो(&p->sParse, 0, माप(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  अगर( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }अन्यथा{
    p->sParse.zJson = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
    p->sParse.nJson = sqlite3_मूल्य_bytes(argv[0]);
    अगर( p->sParse.zJson==0 ){
      p->i = p->iEnd = 0;
      वापस SQLITE_OK;
    }      
    अगर( jsonConvertTextToBlob(&p->sParse, 0) ){
      अगर( p->sParse.oom ){
        वापस SQLITE_NOMEM;
      }
      वापस json_Each_Malformed_निवेश(cur);
    }
  }
  अगर( idx_अंक==3 ){
    z_मूल = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]);
    अगर( z_मूल==0 ) वापस SQLITE_OK;
    अगर( z_मूल[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_संदेश);
      cur->pVtab->z_Err_संदेश = jsonBadPathError(0, z_मूल, 0);
      jsonEachCursorReset(p);
      वापस cur->pVtab->z_Err_संदेश ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_मूल = sqlite3Strlen30(z_मूल);
    अगर( z_मूल[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }अन्यथा{
      i = jsonLookupStep(&p->sParse, 0, z_मूल+1, 0);
      अगर( JSON_LOOKUP_ISERROR(i) ){
        अगर( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          वापस SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_संदेश);
        cur->pVtab->z_Err_संदेश = jsonBadPathError(0, z_मूल, 0);
        jsonEachCursorReset(p);
        वापस cur->pVtab->z_Err_संदेश ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      अगर( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }अन्यथा{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->path, z_मूल, p->n_मूल);
  }अन्यथा{
    i = p->i = 0;
    p->eType = 0;
    p->n_मूल = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonb_Payload_आकार(&p->sParse, i, &sz);
  p->iEnd = i+n+sz;
  अगर( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = sqlite3DbMallocZero(p->db, माप(JsonParent));
    अगर( p->aParent==0 ) वापस SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].i_कुंजी = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].i_शीर्ष = p->i;
    p->aParent[0].i_मूल्य = i;
  }
  वापस SQLITE_OK;
}

/* The methods of the json_each virtual table */
अटल sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  json_Each_Best_सूचकांक,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  json_Each_अग्र,              /* xNext - advance a cursor */
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
#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */
#पूर्ण_अगर /* !defined(SQLITE_OMIT_JSON) */

/*
** Register JSON functions.
*/
व्योम sqlite3RegisterJsonFunctions(व्योम){
#अगर_अघोषित SQLITE_OMIT_JSON
  अटल FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_जोड़ो, -1,1,1, 1,0,JSON_AINS,  jsonSetFunc),
    JFUNCTION(jsonb_array_जोड़ो,-1,1,0, 1,1,JSON_AINS,  jsonSetFunc),
    JFUNCTION(json_array_लंबाई,  1,1,0, 0,0,0,          json_Array_लंबाई_Func),
    JFUNCTION(json_array_लंबाई,  2,1,0, 0,0,0,          json_Array_लंबाई_Func),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_extract,      -1,1,1, 0,0,0,          jsonExtractFunc),
    JFUNCTION(jsonb_extract,     -1,1,0, 0,1,0,          jsonExtractFunc),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  jsonExtractFunc),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   jsonExtractFunc),
    JFUNCTION(json_जोड़ो,       -1,1,1, 1,0,0,          jsonSetFunc),
    JFUNCTION(jsonb_जोड़ो,      -1,1,0, 1,1,0,          jsonSetFunc),
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
#अगर SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#पूर्ण_अगर
    WAGGREGATE(json_group_array,  1, 0, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_मूल्य, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_परिणाम_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_array, 1, JSON_BLOB, 0,
       jsonArrayStep, jsonArrayFinal, json_Array_मूल्य, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_परिणाम_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_group_object, 2, 0, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_मूल्य, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_परिणाम_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_group_object,2, JSON_BLOB, 0,
       jsonObjectStep, jsonObjectFinal, json_Object_मूल्य, jsonGroupInverse,
       SQLITE_SUBTYPE|SQLITE_परिणाम_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_जोड़ो_Builtin_Funcs(aJsonFunc, Array_आकार(aJsonFunc));
#पूर्ण_अगर
}

#अगर  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3JsonVtabRegister(sqlite3 *db, स्थिर अक्षर *zName){
  अचिन्हित पूर्णांक i;
  अटल स्थिर अक्षर *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  निश्चित( sqlite3HashFind(&db->aModule, zName)==0 );
  क्रम(i=0; i<माप(azModule)/माप(azModule[0]); i++){
    अगर( sqlite3StrICmp(azModule[i],zName)==0 ){
      वापस sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  वापस 0;
}
#पूर्ण_अगर /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
