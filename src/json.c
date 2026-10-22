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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_JSON
#అన్తర్భావ "sqliteInt.h"

/* JSONB element types
*/
#నిర్వచించు JSONB_NULL     0   /* "null" */
#నిర్వచించు JSONB_TRUE     1   /* "true" */
#నిర్వచించు JSONB_FALSE    2   /* "false" */
#నిర్వచించు JSONB_INT      3   /* integer acceptable to JSON and SQL */
#నిర్వచించు JSONB_INT5     4   /* integer in 0x000 notation */
#నిర్వచించు JSONB_FLOAT    5   /* float acceptable to JSON and SQL */
#నిర్వచించు JSONB_FLOAT5   6   /* float with JSON5 extensions */
#నిర్వచించు JSONB_పాఠ్యం     7   /* Text compatible with both JSON and SQL */
#నిర్వచించు JSONB_TEXTJ    8   /* Text with JSON escapes */
#నిర్వచించు JSONB_TEXT5    9   /* Text with JSON-5 escape */
#నిర్వచించు JSONB_TEXTRAW 10   /* SQL text that needs escaping for JSON */
#నిర్వచించు JSONB_ARRAY   11   /* An array */
#నిర్వచించు JSONB_OBJECT  12   /* An object */

/* Human-readable names for the JSONB values.  The index for each
** string must correspond to the JSONB_* integer above.
*/
స్థిర స్థిరరాశి అక్షరం * స్థిరరాశి jsonbType[] = {
  "null", "true", "false", "integer", "integer", 
  "real", "real", "text",  "text",    "text",
  "text", "array", "object", "", "", "", ""
};

/*
** Growing our own isspace() routine this way is twice as fast as
** the library isspace() function, resulting in a 7% overall performance
** increase for the text-JSON parser.  (Ubuntu14.10 gcc 4.8.4 x64 with -Os).
*/
స్థిర స్థిరరాశి అక్షరం jsonIsSpace[] = {
#నిర్వచితమైతే SQLITE_ASCII
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
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EBCDIC
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
#అయితే_ముగింపు

};
#నిర్వచించు jsonIsspace(x) (jsonIsSpace[(గుర్తులేని అక్షరం)x])

/*
** The set of all space characters recognized by jsonIsspace().
** Useful as the second argument to strspn().
*/
#నిర్వచితమైతే SQLITE_ASCII
స్థిర స్థిరరాశి అక్షరం jsonSpaces[] = "\011\012\015\040";
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EBCDIC
స్థిర స్థిరరాశి అక్షరం jsonSpaces[] = "\005\045\015\100";
#అయితే_ముగింపు


/*
** Characters that are special to JSON.  Control characters,
** '"' and '\\' and '\''.  Actually, '\'' is not special to
** canonical JSON, but it is special in JSON-5, so we include
** it in the set of special characters.
*/
స్థిర స్థిరరాశి అక్షరం jsonIsOk[256] = {
#నిర్వచితమైతే SQLITE_ASCII
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
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EBCDIC
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
#అయితే_ముగింపు
};

/* Objects */
రకనిర్వచనం నిర్మాణం JsonCache JsonCache;
రకనిర్వచనం నిర్మాణం JsonString JsonString;
రకనిర్వచనం నిర్మాణం JsonParse JsonParse;

/*
** Magic number used for the JSON parse cache in sqlite3_get_auxdata()
*/
#నిర్వచించు JSON_CACHE_ID    (-429938)  /* Cache entry */
#నిర్వచించు JSON_CACHE_కొలత  4          /* Max number of cache entries */

/*
** jsonUnescapeOneChar() returns this invalid code point if it encounters
** a syntax error.
*/
#నిర్వచించు JSON_INVALID_CHAR 0x99999

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
నిర్మాణం JsonCache {
  sqlite3 *db;                    /* Database connection */
  పూర్ణాంకం n_వాడిన;                      /* Number of active entries in the cache */
  JsonParse *a[JSON_CACHE_కొలత];  /* One line for each cache entry */
};

/* An instance of this object represents a JSON string
** under construction.  Really, this is a generic string accumulator
** that can be and is used to create strings other than JSON.
**
** If the generated string is longer than will fit into the zSpace[] buffer,
** then it will be an RCStr string.  This aids with caching of large
** JSON strings.
*/
నిర్మాణం JsonString {
  sqlite3_context *pCtx;   /* Function context - put error messages here */
  అక్షరం *zBuf;              /* Append JSON content here */
  u64 nAlloc;              /* Bytes of storage available in zBuf[] */
  u64 n_వాడిన;               /* Bytes of zBuf[] currently used */
  u8 bStatic;              /* True if zBuf is static space */
  u8 eErr;                 /* True if an error has been encountered */
  అక్షరం zSpace[100];        /* Initial static space */
};

/* Allowed values for JsonString.eErr */
#నిర్వచించు JSTRING_OOM         0x01   /* Out of memory */
#నిర్వచించు JSTRING_MALFORMED   0x02   /* Malformed JSONB */
#నిర్వచించు JSTRING_TOODEEP     0x04   /* JSON nested too deep */
#నిర్వచించు JSTRING_ERR         0x08   /* Error already sent to sqlite3_result */

/* The "subtype" set for text JSON values passed through using
** sqlite3_result_subtype() and sqlite3_value_subtype().
*/
#నిర్వచించు JSON_SUBTYPE  74    /* Ascii for "J" */

/*
** Bit values for the flags passed into various SQL function implementations
** via the sqlite3_user_data() value.
*/
#నిర్వచించు JSON_JSON      0x01        /* Result is always JSON */
#నిర్వచించు JSON_SQL       0x02        /* Result is always SQL */
#నిర్వచించు JSON_ABPATH    0x03        /* Allow abbreviated JSON path specs */
#నిర్వచించు JSON_ISSET     0x04        /* json_set(), not json_insert() */
#నిర్వచించు JSON_AINS      0x08        /* json_array_insert(), not json_insert() */
#నిర్వచించు JSON_BLOB      0x10        /* Use the BLOB output format */

#నిర్వచించు JSON_చొప్పించు_TYPE(X)  (((X)&0xC)>>2)


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
నిర్మాణం JsonParse {
  u8 *aBlob;         /* JSONB representation of JSON value */
  u32 nBlob;         /* Bytes of aBlob[] actually used */
  u32 nBlobAlloc;    /* Bytes allocated to aBlob[].  0 if aBlob is external */
  అక్షరం *zJson;       /* Json text used for parsing */
  sqlite3 *db;       /* The database connection to which this object belongs */
  పూర్ణాంకం nJson;         /* Length of the zJson string in bytes */
  u32 nJPRef;        /* Number of references to this object */
  u32 iErr;          /* Error location in zJson[] */
  u16 i_లోతు;        /* Nesting depth */
  u8 nErr;           /* Number of errors seen */
  u8 oom;            /* Set to true if out of memory */
  u8 bJsonIsRCStr;   /* True if zJson is an RCStr */
  u8 hasNonstd;      /* True if input uses non-standard features like JSON5 */
  u8 bReadOnly;      /* Do not modify. */
  /* Search and edit information.  See jsonLookupStep() */
  u8 eEdit;          /* Edit operation to apply */
  పూర్ణాంకం delta;         /* Size change due to the edit */
  u32 nIns;          /* Number of bytes to insert */
  u32 iLabel;        /* Location of label if search landed on an object value */
  u8 *aIns;          /* Content to be inserted */
};

/* Allowed values for JsonParse.eEdit */
#నిర్వచించు JEDIT_DEL   1   /* Delete if exists */
#నిర్వచించు JEDIT_REPL  2   /* Overwrite if exists */
#నిర్వచించు JEDIT_INS   3   /* Insert if not exists */
#నిర్వచించు JEDIT_సమితి   4   /* Insert or overwrite */
#నిర్వచించు JEDIT_AINS  5   /* array_insert() */

/*
** Maximum nesting depth of JSON for this implementation.
**
** This limit is needed to avoid a stack overflow in the recursive
** descent parser.  A depth of 1000 is far deeper than any sane JSON
** should go.  Historical note: This limit was 2000 prior to version 3.42.0
*/
#నిర్వచితం_కాకపోతే SQLITE_JSON_MAX_లోతు
# define JSON_MAX_లోతు  1000
#లేకపోతే
# define JSON_MAX_లోతు SQLITE_JSON_MAX_లోతు
#అయితే_ముగింపు

/*
** Allowed values for the flgs argument to jsonParseFuncArg();
*/
#నిర్వచించు JSON_EDITABLE  0x01   /* Generate a writable JsonParse object */
#నిర్వచించు JSON_KEEPERROR 0x02   /* Return non-NULL even if there is an error */
#నిర్వచించు JSON_NOCACHE   0x04   /* Do not use the JSON cache */

/**************************************************************************
** Forward references
**************************************************************************/
స్థిర శూన్య jsonReturnStringAsBlob(JsonString*);
స్థిర పూర్ణాంకం jsonArgIsJsonb(sqlite3_విలువ *pJson, JsonParse *p);
స్థిర u32 json_Translate_Blob_గమ్యం_పాఠ్యం(JsonParse*,u32,JsonString*);
స్థిర శూన్య jsonReturnParse(sqlite3_context*,JsonParse*);
స్థిర JsonParse *jsonParseFuncArg(sqlite3_context*,sqlite3_విలువ*,u32);
స్థిర శూన్య jsonParseFree(JsonParse*);
స్థిర u32 jsonb_Payload_కొలత(స్థిరరాశి JsonParse*, u32, u32*);
స్థిర u32 jsonUnescapeOneChar(స్థిరరాశి అక్షరం*, u32, u32*);

/**************************************************************************
** Utility routines for dealing with JsonCache objects
**************************************************************************/

/*
** Free a JsonCache object.
*/
స్థిర శూన్య jsonCacheDelete(JsonCache *p){
  పూర్ణాంకం i;
  కొరకు(i=0; i<p->n_వాడిన; i++){
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}
స్థిర శూన్య jsonCacheDeleteGeneric(శూన్య *p){
  jsonCacheDelete((JsonCache*)p);
}

/*
** Insert a new entry into the cache.  If the cache is full, expel
** the least recently used entry.  Return SQLITE_OK on success or a
** result code otherwise.
**
** Cache entries are stored in age order, oldest first.
*/
స్థిర పూర్ణాంకం json_Cache_చొప్పించు(
  sqlite3_context *ctx,   /* The SQL statement context holding the cache */
  JsonParse *pParse       /* The parse object to be added to the cache */
){
  JsonCache *p;

  ప్రతిజ్ఞా( pParse->zJson!=0 );
  ప్రతిజ్ఞా( pParse->bJsonIsRCStr );
  ప్రతిజ్ఞా( pParse->delta==0 );
  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ఒకవేళ( p==0 ){
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, పరిమాణం(*p));
    ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM;
    p->db = db;
    sqlite3_సమితి_auxdata(ctx, JSON_CACHE_ID, p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
    ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM;
  }
  ఒకవేళ( p->n_వాడిన >= JSON_CACHE_కొలత ){
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (JSON_CACHE_కొలత-1)*పరిమాణం(p->a[0]));
    p->n_వాడిన = JSON_CACHE_కొలత-1;
  }
  ప్రతిజ్ఞా( pParse->nBlobAlloc>0 );
  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->n_వాడిన] = pParse;
  p->n_వాడిన++;
  తిరిగి SQLITE_OK;
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
స్థిర JsonParse *json_Cache_శోధన(
  sqlite3_context *ctx,    /* The SQL statement context holding the cache */
  sqlite3_విలువ *pArg      /* Function argument containing SQL text */
){
  JsonCache *p;
  పూర్ణాంకం i;
  స్థిరరాశి అక్షరం *zJson;
  పూర్ణాంకం nJson;

  ఒకవేళ( sqlite3_విలువ_type(pArg)!=SQLITE_పాఠ్యం ){
    తిరిగి 0;
  }
  zJson = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(pArg);
  ఒకవేళ( zJson==0 ) తిరిగి 0;
  nJson = sqlite3_విలువ_బైట్లు(pArg);

  p = sqlite3_get_auxdata(ctx, JSON_CACHE_ID);
  ఒకవేళ( p==0 ){
    తిరిగి 0;
  }
  కొరకు(i=0; i<p->n_వాడిన; i++){
    ఒకవేళ( p->a[i]->zJson==zJson ) ఆపు;
  }
  ఒకవేళ( i>=p->n_వాడిన ){
    కొరకు(i=0; i<p->n_వాడిన; i++){
      ఒకవేళ( p->a[i]->nJson!=nJson ) కొనసాగు;
      ఒకవేళ( memcmp(p->a[i]->zJson, zJson, nJson)==0 ) ఆపు;
    }
  }
  ఒకవేళ( i<p->n_వాడిన ){
    ఒకవేళ( i<p->n_వాడిన-1 ){
      /* Make the matching entry the most recently used entry */
      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i+1], (p->n_వాడిన-i-1)*పరిమాణం(tmp));
      p->a[p->n_వాడిన-1] = tmp;
      i = p->n_వాడిన - 1;
    }
    ప్రతిజ్ఞా( p->a[i]->delta==0 );
    తిరిగి p->a[i];
  }లేకపోతే{
    తిరిగి 0;
  }
}

/**************************************************************************
** Utility routines for dealing with JsonString objects
**************************************************************************/

/* Turn uninitialized bulk memory into a valid JsonString object
** holding a zero-length string.
*/
స్థిర శూన్య jsonStringZero(JsonString *p){
  p->zBuf = p->zSpace;
  p->nAlloc = పరిమాణం(p->zSpace);
  p->n_వాడిన = 0;
  p->bStatic = 1;
}

/* Initialize the JsonString object
*/
స్థిర శూన్య jsonStringInit(JsonString *p, sqlite3_context *pCtx){
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

/* Free all allocated memory and reset the JsonString object back to its
** initial state.
*/
స్థిర శూన్య jsonStringReset(JsonString *p){
  ఒకవేళ( !p->bStatic ) sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

/* Report an out-of-memory (OOM) condition
*/
స్థిర శూన్య jsonStringOom(JsonString *p){
  p->eErr |= JSTRING_OOM;
  ఒకవేళ( p->pCtx ) sqlite3_ఫలితం_error_nomem(p->pCtx);
  jsonStringReset(p);
}

/* Report JSON nested too deep
*/
స్థిర శూన్య jsonStringTooDeep(JsonString *p){
  p->eErr |= JSTRING_TOODEEP;
  ప్రతిజ్ఞా( p->pCtx!=0 );
  sqlite3_ఫలితం_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

/* Enlarge pJson->zBuf so that it can hold at least N more bytes.
** Return zero on success.  Return non-zero on an OOM error
*/
స్థిర పూర్ణాంకం jsonStringGrow(JsonString *p, u32 N){
  u64 n_మొత్తం = N<p->nAlloc ? p->nAlloc*2 : p->nAlloc+N+10;
  అక్షరం *zNew;
  ఒకవేళ( p->bStatic ){
    ఒకవేళ( p->eErr ) తిరిగి 1;
    zNew = sqlite3RCStrNew(n_మొత్తం);
    ఒకవేళ( zNew==0 ){
      jsonStringOom(p);
      తిరిగి SQLITE_NOMEM;
    }
    జ్ఞాపకం_నకలు(zNew, p->zBuf, (size_t)p->n_వాడిన);
    p->zBuf = zNew;
    p->bStatic = 0;
  }లేకపోతే{
    p->zBuf = sqlite3RCStrResize(p->zBuf, n_మొత్తం);
    ఒకవేళ( p->zBuf==0 ){
      p->eErr |= JSTRING_OOM;
      jsonStringZero(p);
      తిరిగి SQLITE_NOMEM;
    }
  }
  p->nAlloc = n_మొత్తం;
  తిరిగి SQLITE_OK;
}

/* Append N bytes from zIn onto the end of the JsonString string.
*/
స్థిర SQLITE_NOINLINE శూన్య jsonStringExpandAndAppend(
  JsonString *p,
  స్థిరరాశి అక్షరం *z_లోపల,
  u32 N
){
  ప్రతిజ్ఞా( N>0 );
  ఒకవేళ( jsonStringGrow(p,N) ) తిరిగి;
  జ్ఞాపకం_నకలు(p->zBuf+p->n_వాడిన, z_లోపల, N);
  p->n_వాడిన += N;
}
స్థిర శూన్య jsonAppendRaw(JsonString *p, స్థిరరాశి అక్షరం *z_లోపల, u32 N){
  ఒకవేళ( N==0 ) తిరిగి;
  ఒకవేళ( N+p->n_వాడిన >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_లోపల,N);
  }లేకపోతే{
    జ్ఞాపకం_నకలు(p->zBuf+p->n_వాడిన, z_లోపల, N);
    p->n_వాడిన += N;
  }
}
స్థిర శూన్య jsonAppendRawNZ(JsonString *p, స్థిరరాశి అక్షరం *z_లోపల, u32 N){
  ప్రతిజ్ఞా( N>0 );
  ఒకవేళ( N+p->n_వాడిన >= p->nAlloc ){
    jsonStringExpandAndAppend(p,z_లోపల,N);
  }లేకపోతే{
    జ్ఞాపకం_నకలు(p->zBuf+p->n_వాడిన, z_లోపల, N);
    p->n_వాడిన += N;
  }
}

/* Append formatted text (not to exceed N bytes) to the JsonString.
*/
స్థిర శూన్య jsonPrintf(పూర్ణాంకం N, JsonString *p, స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;
  ఒకవేళ( (p->n_వాడిన + N >= p->nAlloc) && jsonStringGrow(p, N) ) తిరిగి;
  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf+p->n_వాడిన, zFormat, ap);
  va_end(ap);
  p->n_వాడిన += (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(p->zBuf+p->n_వాడిన);
}

/* Append a single character
*/
స్థిర SQLITE_NOINLINE శూన్య jsonAppendCharExpand(JsonString *p, అక్షరం c){
  ఒకవేళ( jsonStringGrow(p,1) ) తిరిగి;
  p->zBuf[p->n_వాడిన++] = c;
}
స్థిర శూన్య jsonAppendChar(JsonString *p, అక్షరం c){
  ఒకవేళ( p->n_వాడిన>=p->nAlloc ){
    jsonAppendCharExpand(p,c);
  }లేకపోతే{
    p->zBuf[p->n_వాడిన++] = c;
  }
}

/* Remove a single character from the end of the string
*/
స్థిర శూన్య jsonStringTrimOneChar(JsonString *p){
  ఒకవేళ( p->eErr==0 ){
    ప్రతిజ్ఞా( p->n_వాడిన>0 );
    p->n_వాడిన--;
  }
}


/* Make sure there is a zero terminator on p->zBuf[]
**
** Return true on success.  Return false if an OOM prevents this
** from happening.
*/
స్థిర పూర్ణాంకం jsonStringTerminate(JsonString *p){
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  తిరిగి p->eErr==0;
}

/* Append a comma separator to the output buffer, if the previous
** character is not '[' or '{'.
*/
స్థిర శూన్య jsonAppendSeparator(JsonString *p){
  అక్షరం c;
  ఒకవేళ( p->n_వాడిన==0 ) తిరిగి;
  c = p->zBuf[p->n_వాడిన-1];
  ఒకవేళ( c=='[' || c=='{' ) తిరిగి;
  jsonAppendChar(p, ',');
}

/* c is a control character.  Append the canonical JSON representation
** of that control character to p.
**
** This routine assumes that the output buffer has already been enlarged
** sufficiently to hold the worst-case encoding plus a nul terminator.
*/
స్థిర శూన్య jsonAppendControlChar(JsonString *p, u8 c){
  స్థిర స్థిరరాశి అక్షరం aSpecial[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, 0
  };
  ప్రతిజ్ఞా( పరిమాణం(aSpecial)==32 );
  ప్రతిజ్ఞా( aSpecial['\b']=='b' );
  ప్రతిజ్ఞా( aSpecial['\f']=='f' );
  ప్రతిజ్ఞా( aSpecial['\n']=='n' );
  ప్రతిజ్ఞా( aSpecial['\r']=='r' );
  ప్రతిజ్ఞా( aSpecial['\t']=='t' );
  ప్రతిజ్ఞా( c>=0 && c<పరిమాణం(aSpecial) );
  ప్రతిజ్ఞా( p->n_వాడిన+7 <= p->nAlloc );
  ఒకవేళ( aSpecial[c] ){
    p->zBuf[p->n_వాడిన] = '\\';
    p->zBuf[p->n_వాడిన+1] = aSpecial[c];
    p->n_వాడిన += 2;
  }లేకపోతే{
    p->zBuf[p->n_వాడిన] = '\\';
    p->zBuf[p->n_వాడిన+1] = 'u';
    p->zBuf[p->n_వాడిన+2] = '0';
    p->zBuf[p->n_వాడిన+3] = '0';
    p->zBuf[p->n_వాడిన+4] = "0123456789abcdef"[c>>4];
    p->zBuf[p->n_వాడిన+5] = "0123456789abcdef"[c&0xf];
    p->n_వాడిన += 6;
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
స్థిర శూన్య jsonAppendString(JsonString *p, స్థిరరాశి అక్షరం *z_లోపల, u32 N){
  u32 k;
  u8 c;
  స్థిరరాశి u8 *z = (స్థిరరాశి u8*)z_లోపల;
  ఒకవేళ( z==0 ) తిరిగి;
  ఒకవేళ( (N+p->n_వాడిన+2 >= p->nAlloc) && jsonStringGrow(p,N+2)!=0 ) తిరిగి;
  p->zBuf[p->n_వాడిన++] = '"';
  అయితే( 1 /*exit-by-break*/ ){
    k = 0;
    /* The following while() is the 4-way unwound equivalent of
    **
    **     while( k<N && jsonIsOk[z[k]] ){ k++; }
    */
    అయితే( 1 /* Exit by break */ ){
      ఒకవేళ( k+3>=N ){
        అయితే( k<N && jsonIsOk[z[k]] ){ k++; }
        ఆపు;
      }
      ఒకవేళ( !jsonIsOk[z[k]] ){
        ఆపు;
      }
      ఒకవేళ( !jsonIsOk[z[k+1]] ){
        k += 1;
        ఆపు;
      }
      ఒకవేళ( !jsonIsOk[z[k+2]] ){
        k += 2;
        ఆపు;
      }
      ఒకవేళ( !jsonIsOk[z[k+3]] ){
        k += 3;
        ఆపు;
      }లేకపోతే{
        k += 4;
      }
    }
    ఒకవేళ( k>=N ){
      ఒకవేళ( k>0 ){
        జ్ఞాపకం_నకలు(&p->zBuf[p->n_వాడిన], z, k);
        p->n_వాడిన += k;
      }
      ఆపు;
    }
    ఒకవేళ( k>0 ){
      జ్ఞాపకం_నకలు(&p->zBuf[p->n_వాడిన], z, k);
      p->n_వాడిన += k;
      z += k;
      N -= k;
    }
    c = z[0];
    ఒకవేళ( c=='"' || c=='\\' ){
      ఒకవేళ( (p->n_వాడిన+N+3 > p->nAlloc) && jsonStringGrow(p,N+3)!=0 ) తిరిగి;
      p->zBuf[p->n_వాడిన++] = '\\';
      p->zBuf[p->n_వాడిన++] = c;
    }లేకపోతే ఒకవేళ( c=='\'' ){
      p->zBuf[p->n_వాడిన++] = c;
    }లేకపోతే{
      ఒకవేళ( (p->n_వాడిన+N+7 > p->nAlloc) && jsonStringGrow(p,N+7)!=0 ) తిరిగి;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->n_వాడిన++] = '"';
  ప్రతిజ్ఞా( p->n_వాడిన<p->nAlloc );
}

/*
** Append an sqlite3_value (such as a function parameter) to the JSON
** string under construction in p.
*/
స్థిర శూన్య json_Append_Sql_విలువ(
  JsonString *p,                 /* Append to this JSON string */
  sqlite3_విలువ *p_విలువ          /* Value to append */
){
  ఎంపిక( sqlite3_విలువ_type(p_విలువ) ){
    స్థితి SQLITE_NULL: {
      jsonAppendRawNZ(p, "null", 4);
      ఆపు;
    }
    స్థితి SQLITE_FLOAT: {
      jsonPrintf(100, p, "%!0.17g", sqlite3_విలువ_double(p_విలువ));
      ఆపు;
    }
    స్థితి SQLITE_INTEGER: {
      స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(p_విలువ);
      u32 n = (u32)sqlite3_విలువ_బైట్లు(p_విలువ);
      jsonAppendRaw(p, z, n);
      ఆపు;
    }
    స్థితి SQLITE_పాఠ్యం: {
      స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(p_విలువ);
      u32 n = (u32)sqlite3_విలువ_బైట్లు(p_విలువ);
      ఒకవేళ( sqlite3_విలువ_subtype(p_విలువ)==JSON_SUBTYPE ){
        jsonAppendRaw(p, z, n);
      }లేకపోతే{
        jsonAppendString(p, z, n);
      }
      ఆపు;
    }
    అప్రమేయ: {
      JsonParse px;
      memset(&px, 0, పరిమాణం(px));
      ఒకవేళ( jsonArgIsJsonb(p_విలువ, &px) ){
        json_Translate_Blob_గమ్యం_పాఠ్యం(&px, 0, p);
      }లేకపోతే ఒకవేళ( p->eErr==0 ){
        sqlite3_ఫలితం_error(p->pCtx, "JSON cannot hold BLOB values", -1);
        p->eErr = JSTRING_ERR;
        jsonStringReset(p);
      }
      ఆపు;
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
స్థిర శూన్య jsonReturnString(
  JsonString *p,            /* String to return */
  JsonParse *pParse,        /* JSONB source or NULL */
  sqlite3_context *ctx      /* Where to cache */
){
  ప్రతిజ్ఞా( (pParse!=0)==(ctx!=0) );
  ప్రతిజ్ఞా( ctx==0 || ctx==p->pCtx );
  jsonStringTerminate(p);
  ఒకవేళ( p->eErr==0 ){
    పూర్ణాంకం flags = SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(p->pCtx));
    ఒకవేళ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(p);
    }లేకపోతే ఒకవేళ( p->bStatic ){
      sqlite3_ఫలితం_text64(p->pCtx, p->zBuf, p->n_వాడిన,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
    }లేకపోతే{
      ఒకవేళ( pParse && pParse->bJsonIsRCStr==0 && pParse->nBlobAlloc>0 ){
        పూర్ణాంకం rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->n_వాడిన;
        pParse->bJsonIsRCStr = 1;
        rc = json_Cache_చొప్పించు(ctx, pParse);
        ఒకవేళ( rc==SQLITE_NOMEM ){
          sqlite3_ఫలితం_error_nomem(ctx);
          jsonStringReset(p);
          తిరిగి;
        }
      }
      sqlite3_ఫలితం_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->n_వాడిన,
                            sqlite3RCStrUnref,
                            SQLITE_UTF8);
    }
  }లేకపోతే ఒకవేళ( p->eErr & JSTRING_OOM ){
    sqlite3_ఫలితం_error_nomem(p->pCtx);
  }లేకపోతే ఒకవేళ( p->eErr & JSTRING_TOODEEP ){
    /* error already in p->pCtx */
  }లేకపోతే ఒకవేళ( p->eErr & JSTRING_MALFORMED ){
    sqlite3_ఫలితం_error(p->pCtx, "malformed JSON", -1);
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
స్థిర శూన్య jsonParseReset(JsonParse *pParse){
  ప్రతిజ్ఞా( pParse->nJPRef<=1 );
  ఒకవేళ( pParse->bJsonIsRCStr ){
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  ఒకవేళ( pParse->nBlobAlloc ){
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
స్థిర శూన్య jsonParseFree(JsonParse *pParse){
  ఒకవేళ( pParse ){
    ఒకవేళ( pParse->nJPRef>1 ){
      pParse->nJPRef--;
    }లేకపోతే{
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
స్థిర u8 json_Hex_గమ్యం_Int(పూర్ణాంకం h){
#నిర్వచితమైతే SQLITE_ASCII
  h += 9*(1&(h>>6));
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#అయితే_ముగింపు
  తిరిగి (u8)(h & 0xf);
}

/*
** Convert a 4-byte hex string into an integer
*/
స్థిర u32 json_Hex_గమ్యం_Int4(స్థిరరాశి అక్షరం *z){
  u32 v;
  v = (json_Hex_గమ్యం_Int(z[0])<<12)
    + (json_Hex_గమ్యం_Int(z[1])<<8)
    + (json_Hex_గమ్యం_Int(z[2])<<4)
    + json_Hex_గమ్యం_Int(z[3]);
  తిరిగి v;
}

/*
** Return true if z[] begins with 2 (or more) hexadecimal digits
*/
స్థిర పూర్ణాంకం jsonIs2Hex(స్థిరరాశి అక్షరం *z){
  తిరిగి sqlite3Isxdigit(z[0]) && sqlite3Isxdigit(z[1]);
}

/*
** Return true if z[] begins with 4 (or more) hexadecimal digits
*/
స్థిర పూర్ణాంకం jsonIs4Hex(స్థిరరాశి అక్షరం *z){
  తిరిగి jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
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
స్థిర పూర్ణాంకం json5Whitespace(స్థిరరాశి అక్షరం *z_లోపల){
  పూర్ణాంకం n = 0;
  స్థిరరాశి u8 *z = (u8*)z_లోపల;
  అయితే( 1 /*exit by "goto whitespace_done"*/ ){
    ఎంపిక( z[n] ){
      స్థితి 0x09:
      స్థితి 0x0a:
      స్థితి 0x0b:
      స్థితి 0x0c:
      స్థితి 0x0d:
      స్థితి 0x20: {
        n++;
        ఆపు;
      }
      స్థితి '/': {
        ఒకవేళ( z[n+1]=='*' && z[n+2]!=0 ){
          పూర్ణాంకం j;
          కొరకు(j=n+3; z[j]!='/' || z[j-1]!='*'; j++){
            ఒకవేళ( z[j]==0 ) వెళ్ళు whitespace_పూర్తి;
          }
          n = j+1;
          ఆపు;
        }లేకపోతే ఒకవేళ( z[n+1]=='/' ){
          పూర్ణాంకం j;
          అక్షరం c;
          కొరకు(j=n+2; (c = z[j])!=0; j++){
            ఒకవేళ( c=='\n' || c=='\r' ) ఆపు;
            ఒకవేళ( 0xe2==(u8)c && 0x80==(u8)z[j+1]
             && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2])
            ){
              j += 2;
              ఆపు;
            }
          }
          n = j;
          ఒకవేళ( z[n] ) n++;
          ఆపు;
        }
        వెళ్ళు whitespace_పూర్తి;
      }
      స్థితి 0xc2: {
        ఒకవేళ( z[n+1]==0xa0 ){
          n += 2;
          ఆపు;
        }
        వెళ్ళు whitespace_పూర్తి;
      }
      స్థితి 0xe1: {
        ఒకవేళ( z[n+1]==0x9a && z[n+2]==0x80 ){
          n += 3;
          ఆపు;
        }
        వెళ్ళు whitespace_పూర్తి;
      }
      స్థితి 0xe2: {
        ఒకవేళ( z[n+1]==0x80 ){
          u8 c = z[n+2];
          ఒకవేళ( c<0x80 ) వెళ్ళు whitespace_పూర్తి;
          ఒకవేళ( c<=0x8a || c==0xa8 || c==0xa9 || c==0xaf ){
            n += 3;
            ఆపు;
          }
        }లేకపోతే ఒకవేళ( z[n+1]==0x81 && z[n+2]==0x9f ){
          n += 3;
          ఆపు;
        }
        వెళ్ళు whitespace_పూర్తి;
      }
      స్థితి 0xe3: {
        ఒకవేళ( z[n+1]==0x80 && z[n+2]==0x80 ){
          n += 3;
          ఆపు;
        }
        వెళ్ళు whitespace_పూర్తి;
      }
      స్థితి 0xef: {
        ఒకవేళ( z[n+1]==0xbb && z[n+2]==0xbf ){
          n += 3;
          ఆపు;
        }
        వెళ్ళు whitespace_పూర్తి;
      }
      అప్రమేయ: {
        వెళ్ళు whitespace_పూర్తి;
      }
    }
  }
  whitespace_పూర్తి:
  తిరిగి n;
}

/*
** Extra floating-point literals to allow in JSON.
*/
స్థిర స్థిరరాశి నిర్మాణం NanInfName {
  అక్షరం c1;
  అక్షరం c2;
  అక్షరం n;
  అక్షరం eType;
  అక్షరం nRepl;
  అక్షరం *z_సరిపోలిక;
  అక్షరం *zRepl;
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
స్థిర శూన్య json_Wrong_అంకె_వాదనలు(
  sqlite3_context *pCtx,
  స్థిరరాశి అక్షరం *zFuncName
){
  అక్షరం *z_సందేశం = sqlite3_mprintf("json_%s() needs an odd number of arguments",
                               zFuncName);
  sqlite3_ఫలితం_error(pCtx, z_సందేశం, -1);
  sqlite3_free(z_సందేశం);
}

/****************************************************************************
** Utility routines for dealing with the binary BLOB representation of JSON
****************************************************************************/

/*
** Expand pParse->aBlob so that it holds at least N bytes.
**
** Return the number of errors.
*/
స్థిర పూర్ణాంకం jsonBlobExpand(JsonParse *pParse, u64 N){
  u8 *aNew;
  u64 t;
  ప్రతిజ్ఞా( N>pParse->nBlobAlloc );
  ఒకవేళ( pParse->nBlobAlloc==0 ){
    t = 100;
  }లేకపోతే{
    t = pParse->nBlobAlloc*2;
  }
  ఒకవేళ( t<N ) t = N+100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  ఒకవేళ( aNew==0 ){ pParse->oom = 1; తిరిగి 1; }
  ప్రతిజ్ఞా( t<0x7fffffff );
  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  తిరిగి 0;
}

/*
** If pParse->aBlob is not previously editable (because it is taken
** from sqlite3_value_blob(), as indicated by the fact that
** pParse->nBlobAlloc==0 and pParse->nBlob>0) then make it editable
** by making a copy into space obtained from malloc.
**
** Return true on success.  Return false on OOM.
*/
స్థిర పూర్ణాంకం jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra){
  u8 *aOld;
  u64 n_కొలత;
  ప్రతిజ్ఞా( !pParse->bReadOnly );
  ఒకవేళ( pParse->oom ) తిరిగి 0;
  ఒకవేళ( pParse->nBlobAlloc>0 ) తిరిగి 1;
  aOld = pParse->aBlob;
  n_కొలత = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  ఒకవేళ( jsonBlobExpand(pParse, n_కొలత) ){
    తిరిగి 0;
  }
  ప్రతిజ్ఞా( pParse->nBlobAlloc >= pParse->nBlob + nExtra );
  జ్ఞాపకం_నకలు(pParse->aBlob, aOld, pParse->nBlob);
  తిరిగి 1;
}

/* Expand pParse->aBlob and append one bytes.
*/
స్థిర SQLITE_NOINLINE శూన్య jsonBlobExpandAndAppendOneByte(
  JsonParse *pParse,
  u8 c
){
  jsonBlobExpand(pParse, (u64)pParse->nBlob+1);
  ఒకవేళ( pParse->oom==0 ){
    ప్రతిజ్ఞా( pParse->nBlob+1<=pParse->nBlobAlloc );
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Append a single character.
*/
స్థిర శూన్య jsonBlobAppendOneByte(JsonParse *pParse, u8 c){
  ఒకవేళ( pParse->nBlob >= pParse->nBlobAlloc ){
    jsonBlobExpandAndAppendOneByte(pParse, c);
  }లేకపోతే{
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

/* Slow version of jsonBlobAppendNode() that first resizes the
** pParse->aBlob structure.
*/
స్థిర శూన్య json_Blob_Append_కణుపు(JsonParse*,u8,u64,స్థిరరాశి శూన్య*);
స్థిర SQLITE_NOINLINE శూన్య json_Blob_Expand_And_Append_కణుపు(
  JsonParse *pParse,
  u8 eType,
  u64 szPayload,
  స్థిరరాశి శూన్య *aPayload
){
  ఒకవేళ( jsonBlobExpand(pParse, (u64)pParse->nBlob+(u64)szPayload+9) ) తిరిగి;
  json_Blob_Append_కణుపు(pParse, eType, szPayload, aPayload);
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
స్థిర శూన్య json_Blob_Append_కణుపు(
  JsonParse *pParse,          /* The JsonParse object under construction */
  u8 eType,                   /* Node type.  One of JSONB_* */
  u64 szPayload,              /* Number of bytes of payload */
  స్థిరరాశి శూన్య *aPayload        /* The payload.  Might be NULL */
){
  u8 *a;
  ఒకవేళ( pParse->nBlob+szPayload+9 > pParse->nBlobAlloc ){
    json_Blob_Expand_And_Append_కణుపు(pParse,eType,szPayload,aPayload);
    తిరిగి;
  }
  ప్రతిజ్ఞా( pParse->aBlob!=0 );
  a = &pParse->aBlob[pParse->nBlob];
  ఒకవేళ( szPayload<=11 ){
    a[0] = eType | (szPayload<<4);
    pParse->nBlob += 1;
  }లేకపోతే ఒకవేళ( szPayload<=0xff ){
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  }లేకపోతే ఒకవేళ( szPayload<=0xffff ){
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  }లేకపోతే{
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  ఒకవేళ( aPayload ){
    pParse->nBlob += szPayload;
    జ్ఞాపకం_నకలు(&pParse->aBlob[pParse->nBlob-szPayload], aPayload, szPayload);
  }
}

/* Change the payload size for the node at index i to be szPayload.
*/
స్థిర పూర్ణాంకం json_Blob_Change_Payload_కొలత(
  JsonParse *pParse,
  u32 i,
  u32 szPayload
){
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  పూర్ణాంకం delta;
  ఒకవేళ( pParse->oom ) తిరిగి 0;
  a = &pParse->aBlob[i];
  szType = a[0]>>4;
  ఒకవేళ( szType<=11 ){
    nExtra = 0;
  }లేకపోతే ఒకవేళ( szType==12 ){
    nExtra = 1;
  }లేకపోతే ఒకవేళ( szType==13 ){
    nExtra = 2;
  }లేకపోతే ఒకవేళ( szType==14 ){
    nExtra = 4;
  }లేకపోతే{
    nExtra = 8;
  }
  ఒకవేళ( szPayload<=11 ){
    nNeeded = 0;
  }లేకపోతే ఒకవేళ( szPayload<=0xff ){
    nNeeded = 1;
  }లేకపోతే ఒకవేళ( szPayload<=0xffff ){
    nNeeded = 2;
  }లేకపోతే{
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  ఒకవేళ( delta ){
    u64 new_కొలత_2 = (u64)pParse->nBlob + delta;
    ఒకవేళ( delta>0 ){
      ఒకవేళ( new_కొలత_2>pParse->nBlobAlloc && jsonBlobExpand(pParse, new_కొలత_2) ){
        తిరిగి 0;  /* OOM error.  Error state recorded in pParse->oom. */
      }
      a = &pParse->aBlob[i];
      memmove(&a[1+delta], &a[1], pParse->nBlob - (i+1));
    }లేకపోతే{
      memmove(&a[1], &a[1-delta], pParse->nBlob - (i+1-delta));
    }
    pParse->nBlob = new_కొలత_2;
  }
  ఒకవేళ( nNeeded==0 ){
    a[0] = (a[0] & 0x0f) | (szPayload<<4);
  }లేకపోతే ఒకవేళ( nNeeded==1 ){
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  }లేకపోతే ఒకవేళ( nNeeded==2 ){
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  }లేకపోతే{
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  తిరిగి delta;
}

/*
** If z[0] is 'u' and is followed by exactly 4 hexadecimal character,
** then set *pOp to JSONB_TEXTJ and return true.  If not, do not make
** any changes to *pOp and return false.
*/
స్థిర పూర్ణాంకం jsonIs4HexB(స్థిరరాశి అక్షరం *z, పూర్ణాంకం *pOp){
  ఒకవేళ( z[0]!='u' ) తిరిగి 0;
  ఒకవేళ( !jsonIs4Hex(&z[1]) ) తిరిగి 0;
  *pOp = JSONB_TEXTJ;
  తిరిగి 1;
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
స్థిర u32 jsonbValidityCheck(
  స్థిరరాశి JsonParse *pParse,    /* Input JSONB.  Only aBlob and nBlob are used */
  u32 i,                      /* Start of element as pParse->aBlob[i] */
  u32 i_చివర,                   /* One more than the last byte of the element */
  u32 i_లోతు                  /* Current nesting depth */
){
  u32 n, sz, j, k;
  స్థిరరాశి u8 *z;
  u8 x;
  ఒకవేళ( i_లోతు>JSON_MAX_లోతు ) తిరిగి i+1;
  sz = 0;
  n = jsonb_Payload_కొలత(pParse, i, &sz);
  ఒకవేళ( NEVER(n==0) ) తిరిగి i+1;          /* Checked by caller */
  ఒకవేళ( NEVER(i+n+sz!=i_చివర) ) తిరిగి i+1;  /* Checked by caller */
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  ఎంపిక( x ){
    స్థితి JSONB_NULL:
    స్థితి JSONB_TRUE:
    స్థితి JSONB_FALSE: {
      తిరిగి n+sz==1 ? 0 : i+1;
    }
    స్థితి JSONB_INT: {
      ఒకవేళ( sz<1 ) తిరిగి i+1;
      j = i+n;
      ఒకవేళ( z[j]=='-' ){
        j++;
        ఒకవేళ( sz<2 ) తిరిగి i+1;
      }
      k = i+n+sz;
      అయితే( j<k ){
        ఒకవేళ( sqlite3Isdigit(z[j]) ){
          j++;
        }లేకపోతే{
          తిరిగి j+1;
        }
      }
      తిరిగి 0;
    }
    స్థితి JSONB_INT5: {
      ఒకవేళ( sz<3 ) తిరిగి i+1;
      j = i+n;
      ఒకవేళ( z[j]=='-' ){
        ఒకవేళ( sz<4 ) తిరిగి i+1;
        j++;
      }
      ఒకవేళ( z[j]!='0' ) తిరిగి i+1;
      ఒకవేళ( z[j+1]!='x' && z[j+1]!='X' ) తిరిగి j+2;
      j += 2;
      k = i+n+sz;
      అయితే( j<k ){
        ఒకవేళ( sqlite3Isxdigit(z[j]) ){
          j++;
        }లేకపోతే{
          తిరిగి j+1;
        }
      }
      తిరిగి 0;
    }
    స్థితి JSONB_FLOAT:
    స్థితి JSONB_FLOAT5: {
      u8 చూసిన = 0;   /* 0: initial.  1: '.' seen  2: 'e' seen */
      ఒకవేళ( sz<2 ) తిరిగి i+1;
      j = i+n;
      k = j+sz;
      ఒకవేళ( z[j]=='-' ){
        j++;
        ఒకవేళ( sz<3 ) తిరిగి i+1;
      }
      ఒకవేళ( z[j]=='.' ){
        ఒకవేళ( x==JSONB_FLOAT ) తిరిగి j+1;
        ఒకవేళ( !sqlite3Isdigit(z[j+1]) ) తిరిగి j+1;
        j += 2;
        చూసిన = 1;
      }లేకపోతే ఒకవేళ( z[j]=='0' && x==JSONB_FLOAT ){
        ఒకవేళ( j+3>k ) తిరిగి j+1;
        ఒకవేళ( z[j+1]!='.' && z[j+1]!='e' && z[j+1]!='E' ) తిరిగి j+1;
        j++;
      }
      కొరకు(; j<k; j++){
        ఒకవేళ( sqlite3Isdigit(z[j]) ) కొనసాగు;
        ఒకవేళ( z[j]=='.' ){
          ఒకవేళ( చూసిన>0 ) తిరిగి j+1;
          ఒకవేళ( x==JSONB_FLOAT && (j==k-1 || !sqlite3Isdigit(z[j+1])) ){
            తిరిగి j+1;
          }
          చూసిన = 1;
          కొనసాగు;
        }
        ఒకవేళ( z[j]=='e' || z[j]=='E' ){
          ఒకవేళ( చూసిన==2 ) తిరిగి j+1;
          ఒకవేళ( j==k-1 ) తిరిగి j+1;
          ఒకవేళ( z[j+1]=='+' || z[j+1]=='-' ){
            j++;
            ఒకవేళ( j==k-1 ) తిరిగి j+1;
          }
          చూసిన = 2;
          కొనసాగు;
        }
        తిరిగి j+1;
      }
      ఒకవేళ( చూసిన==0 ) తిరిగి i+1;
      తిరిగి 0;
    }
    స్థితి JSONB_పాఠ్యం: {
      j = i+n;
      k = j+sz;
      అయితే( j<k ){
        ఒకవేళ( !jsonIsOk[z[j]] && z[j]!='\'' ) తిరిగి j+1;
        j++;
      }
      తిరిగి 0;
    }
    స్థితి JSONB_TEXTJ:
    స్థితి JSONB_TEXT5: {
      j = i+n;
      k = j+sz;
      అయితే( j<k ){
        ఒకవేళ( !jsonIsOk[z[j]] && z[j]!='\'' ){
          ఒకవేళ( z[j]=='"' ){
            ఒకవేళ( x==JSONB_TEXTJ ) తిరిగి j+1;
          }లేకపోతే ఒకవేళ( z[j]<=0x1f ){
            /* Control characters in JSON5 string literals are ok */
            ఒకవేళ( x==JSONB_TEXTJ ) తిరిగి j+1;
          }లేకపోతే ఒకవేళ( NEVER(z[j]!='\\') || j+1>=k ){
            తిరిగి j+1;
          }లేకపోతే ఒకవేళ( strchr("\"\\/bfnrt",z[j+1])!=0 ){
            j++;
          }లేకపోతే ఒకవేళ( z[j+1]=='u' ){
            ఒకవేళ( j+5>=k ) తిరిగి j+1;
            ఒకవేళ( !jsonIs4Hex((స్థిరరాశి అక్షరం*)&z[j+2]) ) తిరిగి j+1;
            j++;
          }లేకపోతే ఒకవేళ( x!=JSONB_TEXT5 ){
            తిరిగి j+1;
          }లేకపోతే{
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((స్థిరరాశి అక్షరం*)&z[j], k-j, &c);
            ఒకవేళ( c==JSON_INVALID_CHAR ) తిరిగి j+1;
            j += szC - 1;
          }
        }
        j++;
      }
      తిరిగి 0;
    }
    స్థితి JSONB_TEXTRAW: {
      తిరిగి 0;
    }
    స్థితి JSONB_ARRAY: {
      u32 sub;
      j = i+n;
      k = j+sz;
      అయితే( j<k ){
        sz = 0;
        n = jsonb_Payload_కొలత(pParse, j, &sz);
        ఒకవేళ( n==0 ) తిరిగి j+1;
        ఒకవేళ( j+n+sz>k ) తిరిగి j+1;
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_లోతు+1);
        ఒకవేళ( sub ) తిరిగి sub;
        j += n + sz;
      }
      ప్రతిజ్ఞా( j==k );
      తిరిగి 0;
    }
    స్థితి JSONB_OBJECT: {
      u32 cnt = 0;
      u32 sub;
      j = i+n;
      k = j+sz;
      అయితే( j<k ){
        sz = 0;
        n = jsonb_Payload_కొలత(pParse, j, &sz);
        ఒకవేళ( n==0 ) తిరిగి j+1;
        ఒకవేళ( j+n+sz>k ) తిరిగి j+1;
        ఒకవేళ( (cnt & 1)==0 ){
          x = z[j] & 0x0f;
          ఒకవేళ( x<JSONB_పాఠ్యం || x>JSONB_TEXTRAW ) తిరిగి j+1;
        }
        sub = jsonbValidityCheck(pParse, j, j+n+sz, i_లోతు+1);
        ఒకవేళ( sub ) తిరిగి sub;
        cnt++;
        j += n + sz;
      }
      ప్రతిజ్ఞా( j==k );
      ఒకవేళ( (cnt & 1)!=0 ) తిరిగి j+1;
      తిరిగి 0;
    }
    అప్రమేయ: {
      తిరిగి i+1;
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
స్థిర పూర్ణాంకం json_Translate_పాఠ్యం_గమ్యం_Blob(JsonParse *pParse, u32 i){
  అక్షరం c;
  u32 j;
  u32 iThis, i_ప్రారంభం;
  పూర్ణాంకం x;
  u8 t;
  స్థిరరాశి అక్షరం *z = pParse->zJson;
json_parse_restart:
  ఎంపిక( (u8)z[i] ){
  స్థితి '{': {
    /* Parse object */
    iThis = pParse->nBlob;
    json_Blob_Append_కణుపు(pParse, JSONB_OBJECT, pParse->nJson-i, 0);
    ఒకవేళ( ++pParse->i_లోతు > JSON_MAX_లోతు ){
      pParse->iErr = i;
      తిరిగి -1;
    }
    i_ప్రారంభం = pParse->nBlob;
    కొరకు(j=i+1;;j++){
      u32 iBlob = pParse->nBlob;
      x = json_Translate_పాఠ్యం_గమ్యం_Blob(pParse, j);
      ఒకవేళ( x<=0 ){
        పూర్ణాంకం op;
        ఒకవేళ( x==(-2) ){
          j = pParse->iErr;
          ఒకవేళ( pParse->nBlob!=(u32)i_ప్రారంభం ) pParse->hasNonstd = 1;
          ఆపు;
        }
        j += json5Whitespace(&z[j]);
        op = JSONB_పాఠ్యం;
        ఒకవేళ( sqlite3JsonId1(z[j]) 
         || (z[j]=='\\' && jsonIs4HexB(&z[j+1], &op))
        ){
          పూర్ణాంకం k = j+1;
          అయితే( (sqlite3JsonId2(z[k]) && json5Whitespace(&z[k])==0)
            || (z[k]=='\\' && jsonIs4HexB(&z[k+1], &op))
          ){
            k++;
          }
          ప్రతిజ్ఞా( iBlob==pParse->nBlob );
          json_Blob_Append_కణుపు(pParse, op, k-j, &z[j]);
          pParse->hasNonstd = 1;
          x = k;
        }లేకపోతే{
          ఒకవేళ( x!=-1 ) pParse->iErr = j;
          తిరిగి -1;
        }
      }
      ఒకవేళ( pParse->oom ) తిరిగి -1;
      t = pParse->aBlob[iBlob] & 0x0f;
      ఒకవేళ( t<JSONB_పాఠ్యం || t>JSONB_TEXTRAW ){
        pParse->iErr = j;
        తిరిగి -1;
      }
      j = x;
      ఒకవేళ( z[j]==':' ){
        j++;
      }లేకపోతే{
        ఒకవేళ( jsonIsspace(z[j]) ){
          /* strspn() is not helpful here */
          చేయి{ j++; }అయితే( jsonIsspace(z[j]) );
          ఒకవేళ( z[j]==':' ){
            j++;
            వెళ్ళు parse_object_విలువ;
          }
        }
        x = json_Translate_పాఠ్యం_గమ్యం_Blob(pParse, j);
        ఒకవేళ( x!=(-5) ){
          ఒకవేళ( x!=(-1) ) pParse->iErr = j;
          తిరిగి -1;
        }
        j = pParse->iErr+1;
      }
    parse_object_విలువ:
      x = json_Translate_పాఠ్యం_గమ్యం_Blob(pParse, j);
      ఒకవేళ( x<=0 ){
        ఒకవేళ( x!=(-1) ) pParse->iErr = j;
        తిరిగి -1;
      }
      j = x;
      ఒకవేళ( z[j]==',' ){
        కొనసాగు;
      }లేకపోతే ఒకవేళ( z[j]=='}' ){
        ఆపు;
      }లేకపోతే{
        ఒకవేళ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ఒకవేళ( z[j]==',' ){
            కొనసాగు;
          }లేకపోతే ఒకవేళ( z[j]=='}' ){
            ఆపు;
          }
        }
        x = json_Translate_పాఠ్యం_గమ్యం_Blob(pParse, j);
        ఒకవేళ( x==(-4) ){
          j = pParse->iErr;
          కొనసాగు;
        }
        ఒకవేళ( x==(-2) ){
          j = pParse->iErr;
          ఆపు;
        }
      }
      pParse->iErr = j;
      తిరిగి -1;
    }
    json_Blob_Change_Payload_కొలత(pParse, iThis, pParse->nBlob - i_ప్రారంభం);
    pParse->i_లోతు--;
    తిరిగి j+1;
  }
  స్థితి '[': {
    /* Parse array */
    iThis = pParse->nBlob;
    ప్రతిజ్ఞా( i<=(u32)pParse->nJson );
    json_Blob_Append_కణుపు(pParse, JSONB_ARRAY, pParse->nJson - i, 0);
    i_ప్రారంభం = pParse->nBlob;
    ఒకవేళ( pParse->oom ) తిరిగి -1;
    ఒకవేళ( ++pParse->i_లోతు > JSON_MAX_లోతు ){
      pParse->iErr = i;
      తిరిగి -1;
    }
    కొరకు(j=i+1;;j++){
      x = json_Translate_పాఠ్యం_గమ్యం_Blob(pParse, j);
      ఒకవేళ( x<=0 ){
        ఒకవేళ( x==(-3) ){
          j = pParse->iErr;
          ఒకవేళ( pParse->nBlob!=i_ప్రారంభం ) pParse->hasNonstd = 1;
          ఆపు;
        }
        ఒకవేళ( x!=(-1) ) pParse->iErr = j;
        తిరిగి -1;
      }
      j = x;
      ఒకవేళ( z[j]==',' ){
        కొనసాగు;
      }లేకపోతే ఒకవేళ( z[j]==']' ){
        ఆపు;
      }లేకపోతే{
        ఒకవేళ( jsonIsspace(z[j]) ){
          j += 1 + (u32)strspn(&z[j+1], jsonSpaces);
          ఒకవేళ( z[j]==',' ){
            కొనసాగు;
          }లేకపోతే ఒకవేళ( z[j]==']' ){
            ఆపు;
          }
        }
        x = json_Translate_పాఠ్యం_గమ్యం_Blob(pParse, j);
        ఒకవేళ( x==(-4) ){
          j = pParse->iErr;
          కొనసాగు;
        }
        ఒకవేళ( x==(-3) ){
          j = pParse->iErr;
          ఆపు;
        }
      }
      pParse->iErr = j;
      తిరిగి -1;
    }
    json_Blob_Change_Payload_కొలత(pParse, iThis, pParse->nBlob - i_ప్రారంభం);
    pParse->i_లోతు--;
    తిరిగి j+1;
  }
  స్థితి '\'': {
    u8 opcode;
    అక్షరం cDelim;
    pParse->hasNonstd = 1;
    opcode = JSONB_పాఠ్యం;
    వెళ్ళు parse_string;
  స్థితి '"':
    /* Parse string */
    opcode = JSONB_పాఠ్యం;
  parse_string:
    cDelim = z[i];
    j = i+1;
    అయితే( 1 /*exit-by-break*/ ){
      ఒకవేళ( jsonIsOk[(u8)z[j]] ){
        ఒకవేళ( !jsonIsOk[(u8)z[j+1]] ){
          j += 1;
        }లేకపోతే ఒకవేళ( !jsonIsOk[(u8)z[j+2]] ){
          j += 2;
        }లేకపోతే{
          j += 3;
          కొనసాగు;
        }
      }
      c = z[j];
      ఒకవేళ( c==cDelim ){
        ఆపు;
      }లేకపోతే ఒకవేళ( c=='\\' ){
        c = z[++j];
        ఒకవేళ( c=='"' || c=='\\' || c=='/' || c=='b' || c=='f'
           || c=='n' || c=='r' || c=='t'
           || (c=='u' && jsonIs4Hex(&z[j+1])) ){
          ఒకవేళ( opcode==JSONB_పాఠ్యం ) opcode = JSONB_TEXTJ;
        }లేకపోతే ఒకవేళ( c=='\'' ||  c=='v' || c=='\n'
#నిర్వచితమైతే SQLITE_BUG_COMPATIBLE_20250510
           || (c=='0')                            /* Legacy bug compatible */
#లేకపోతే
           || (c=='0' && !sqlite3Isdigit(z[j+1])) /* Correct implementation */
#అయితే_ముగింపు
           || (0xe2==(u8)c && 0x80==(u8)z[j+1]
                && (0xa8==(u8)z[j+2] || 0xa9==(u8)z[j+2]))
           || (c=='x' && jsonIs2Hex(&z[j+1])) ){
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }లేకపోతే ఒకవేళ( c=='\r' ){
          ఒకవేళ( z[j+1]=='\n' ) j++;
          opcode = JSONB_TEXT5;
          pParse->hasNonstd = 1;
        }లేకపోతే{
          pParse->iErr = j;
          తిరిగి -1;
        }
      }లేకపోతే ఒకవేళ( c<=0x1f ){
        ఒకవేళ( c==0 ){
          pParse->iErr = j;
          తిరిగి -1;
        }
        /* Control characters are not allowed in canonical JSON string
        ** literals, but are allowed in JSON5 string literals. */
        opcode = JSONB_TEXT5;
        pParse->hasNonstd = 1;
      }లేకపోతే ఒకవేళ( c=='"' ){
        opcode = JSONB_TEXT5;
      }
      j++;
    }
    json_Blob_Append_కణుపు(pParse, opcode, j-1-i, &z[i+1]);
    తిరిగి j+1;
  }
  స్థితి 't': {
    ఒకవేళ( అక్షరశ్రేణి_పోలిక(z+i,"true",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_TRUE);
      తిరిగి i+4;
    }
    pParse->iErr = i;
    తిరిగి -1;
  }
  స్థితి 'f': {
    ఒకవేళ( అక్షరశ్రేణి_పోలిక(z+i,"false",5)==0 && !sqlite3Isalnum(z[i+5]) ){
      jsonBlobAppendOneByte(pParse, JSONB_FALSE);
      తిరిగి i+5;
    }
    pParse->iErr = i;
    తిరిగి -1;
  }
  స్థితి '+': {
    u8 చూసిన_E;
    pParse->hasNonstd = 1;
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
    వెళ్ళు parse_సంఖ్య;
  స్థితి '.':
    ఒకవేళ( sqlite3Isdigit(z[i+1]) ){
      pParse->hasNonstd = 1;
      t = 0x03;          /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
      చూసిన_E = 0;
      వెళ్ళు parse_సంఖ్య_2;
    }
    pParse->iErr = i;
    తిరిగి -1;
  స్థితి '-':
  స్థితి '0':
  స్థితి '1':
  స్థితి '2':
  స్థితి '3':
  స్థితి '4':
  స్థితి '5':
  స్థితి '6':
  స్థితి '7':
  స్థితి '8':
  స్థితి '9':
    /* Parse number */
    t = 0x00;            /* Bit 0x01:  JSON5.   Bit 0x02:  FLOAT */
  parse_సంఖ్య:
    చూసిన_E = 0;
    ప్రతిజ్ఞా( '-' < '0' );
    ప్రతిజ్ఞా( '+' < '0' );
    ప్రతిజ్ఞా( '.' < '0' );
    c = z[i];

    ఒకవేళ( c<='0' ){
      ఒకవేళ( c=='0' ){
        ఒకవేళ( (z[i+1]=='x' || z[i+1]=='X') && sqlite3Isxdigit(z[i+2]) ){
          ప్రతిజ్ఞా( t==0x00 );
          pParse->hasNonstd = 1;
          t = 0x01;
          కొరకు(j=i+3; sqlite3Isxdigit(z[j]); j++){}
          వెళ్ళు parse_సంఖ్య_ముగింపు;
        }లేకపోతే ఒకవేళ( sqlite3Isdigit(z[i+1]) ){
          pParse->iErr = i+1;
          తిరిగి -1;
        }
      }లేకపోతే{
        ఒకవేళ( !sqlite3Isdigit(z[i+1]) ){
          /* JSON5 allows for "+Infinity" and "-Infinity" using exactly
          ** that case.  SQLite also allows these in any case and it allows
          ** "+inf" and "-inf". */
          ఒకవేళ( (z[i+1]=='I' || z[i+1]=='i')
           && sqlite3StrNICmp(&z[i+1], "inf",3)==0
          ){
            pParse->hasNonstd = 1;
            ఒకవేళ( z[i]=='-' ){
              json_Blob_Append_కణుపు(pParse, JSONB_FLOAT, 6, "-9e999");
            }లేకపోతే{
              json_Blob_Append_కణుపు(pParse, JSONB_FLOAT, 5, "9e999");
            }
            తిరిగి i + (sqlite3StrNICmp(&z[i+4],"inity",5)==0 ? 9 : 4);
          }
          ఒకవేళ( z[i+1]=='.' ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            వెళ్ళు parse_సంఖ్య_2;
          }
          pParse->iErr = i;
          తిరిగి -1;
        }
        ఒకవేళ( z[i+1]=='0' ){
          ఒకవేళ( sqlite3Isdigit(z[i+2]) ){
            pParse->iErr = i+1;
            తిరిగి -1;
          }లేకపోతే ఒకవేళ( (z[i+2]=='x' || z[i+2]=='X') && sqlite3Isxdigit(z[i+3]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
            కొరకు(j=i+4; sqlite3Isxdigit(z[j]); j++){}
            వెళ్ళు parse_సంఖ్య_ముగింపు;
          }
        }
      }
    }

  parse_సంఖ్య_2:
    కొరకు(j=i+1;; j++){
      c = z[j];
      ఒకవేళ( sqlite3Isdigit(c) ) కొనసాగు;
      ఒకవేళ( c=='.' ){
        ఒకవేళ( (t & 0x02)!=0 ){
          pParse->iErr = j;
          తిరిగి -1;
        }
        t |= 0x02;
        కొనసాగు;
      }
      ఒకవేళ( c=='e' || c=='E' ){
        ఒకవేళ( z[j-1]<'0' ){
          ఒకవేళ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
            pParse->hasNonstd = 1;
            t |= 0x01;
          }లేకపోతే{
            pParse->iErr = j;
            తిరిగి -1;
          }
        }
        ఒకవేళ( చూసిన_E ){
          pParse->iErr = j;
          తిరిగి -1;
        }
        t |= 0x02;
        చూసిన_E = 1;
        c = z[j+1];
        ఒకవేళ( c=='+' || c=='-' ){
          j++;
          c = z[j+1];
        }
        ఒకవేళ( c<'0' || c>'9' ){
          pParse->iErr = j;
          తిరిగి -1;
        }
        కొనసాగు;
      }
      ఆపు;
    }
    ఒకవేళ( z[j-1]<'0' ){
      ఒకవేళ( ALWAYS(z[j-1]=='.') && ALWAYS(j-2>=i) && sqlite3Isdigit(z[j-2]) ){
        pParse->hasNonstd = 1;
        t |= 0x01;
      }లేకపోతే{
        pParse->iErr = j;
        తిరిగి -1;
      }
    }
  parse_సంఖ్య_ముగింపు:
    ప్రతిజ్ఞా( JSONB_INT+0x01==JSONB_INT5 );
    ప్రతిజ్ఞా( JSONB_FLOAT+0x01==JSONB_FLOAT5 );
    ప్రతిజ్ఞా( JSONB_INT+0x02==JSONB_FLOAT );
    ఒకవేళ( z[i]=='+' ) i++;
    json_Blob_Append_కణుపు(pParse, JSONB_INT+t, j-i, &z[i]);
    తిరిగి j;
  }
  స్థితి '}': {
    pParse->iErr = i;
    తిరిగి -2;  /* End of {...} */
  }
  స్థితి ']': {
    pParse->iErr = i;
    తిరిగి -3;  /* End of [...] */
  }
  స్థితి ',': {
    pParse->iErr = i;
    తిరిగి -4;  /* List separator */
  }
  స్థితి ':': {
    pParse->iErr = i;
    తిరిగి -5;  /* Object label/value separator */
  }
  స్థితి 0: {
    తిరిగి 0;   /* End of file */
  }
  స్థితి 0x09:
  స్థితి 0x0a:
  స్థితి 0x0d:
  స్థితి 0x20: {
    i += 1 + (u32)strspn(&z[i+1], jsonSpaces);
    వెళ్ళు json_parse_restart;
  }
  స్థితి 0x0b:
  స్థితి 0x0c:
  స్థితి '/':
  స్థితి 0xc2:
  స్థితి 0xe1:
  స్థితి 0xe2:
  స్థితి 0xe3:
  స్థితి 0xef: {
    j = json5Whitespace(&z[i]);
    ఒకవేళ( j>0 ){
      i += j;
      pParse->hasNonstd = 1;
      వెళ్ళు json_parse_restart;
    }
    pParse->iErr = i;
    తిరిగి -1;
  }
  స్థితి 'n': {
    ఒకవేళ( అక్షరశ్రేణి_పోలిక(z+i,"null",4)==0 && !sqlite3Isalnum(z[i+4]) ){
      jsonBlobAppendOneByte(pParse, JSONB_NULL);
      తిరిగి i+4;
    }
    /* fall-through into the default case that checks for NaN */
    /* no break */ deliberate_fall_through
  }
  అప్రమేయ: {
    u32 k;
    పూర్ణాంకం nn;
    c = z[i];
    కొరకు(k=0; k<పరిమాణం(aNanInfName)/పరిమాణం(aNanInfName[0]); k++){
      ఒకవేళ( c!=aNanInfName[k].c1 && c!=aNanInfName[k].c2 ) కొనసాగు;
      nn = aNanInfName[k].n;
      ఒకవేళ( sqlite3StrNICmp(&z[i], aNanInfName[k].z_సరిపోలిక, nn)!=0 ){
        కొనసాగు;
      }
      ఒకవేళ( sqlite3Isalnum(z[i+nn]) ) కొనసాగు;
      ఒకవేళ( aNanInfName[k].eType==JSONB_FLOAT ){
        json_Blob_Append_కణుపు(pParse, JSONB_FLOAT, 5, "9e999");
      }లేకపోతే{
        jsonBlobAppendOneByte(pParse, JSONB_NULL);
      }
      pParse->hasNonstd = 1;
      తిరిగి i + nn;
    }
    pParse->iErr = i;
    తిరిగి -1;  /* Syntax error */
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
స్థిర పూర్ణాంకం json_Convert_పాఠ్యం_గమ్యం_Blob(
  JsonParse *pParse,           /* Initialize and fill this JsonParse object */
  sqlite3_context *pCtx        /* Report errors here */
){
  పూర్ణాంకం i;
  స్థిరరాశి అక్షరం *zJson = pParse->zJson;
  i = json_Translate_పాఠ్యం_గమ్యం_Blob(pParse, 0);
  ఒకవేళ( pParse->oom ) i = -1;
  ఒకవేళ( i>0 ){
#నిర్వచితమైతే SQLITE_DEBUG
    ప్రతిజ్ఞా( pParse->i_లోతు==0 );
    ఒకవేళ( sqlite3Config.bJsonSelfcheck ){
      ప్రతిజ్ఞా( jsonbValidityCheck(pParse, 0, pParse->nBlob, 0)==0 );
    }   
#అయితే_ముగింపు
    అయితే( jsonIsspace(zJson[i]) ) i++;
    ఒకవేళ( zJson[i] ){
      i += json5Whitespace(&zJson[i]);
      ఒకవేళ( zJson[i] ){
        ఒకవేళ( pCtx ) sqlite3_ఫలితం_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        తిరిగి 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  ఒకవేళ( i<=0 ){
    ఒకవేళ( pCtx!=0 ){
      ఒకవేళ( pParse->oom ){
        sqlite3_ఫలితం_error_nomem(pCtx);
      }లేకపోతే{
        sqlite3_ఫలితం_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** The input string pStr is a well-formed JSON text string.  Convert
** this into the JSONB format and make it the return value of the
** SQL function.
*/
స్థిర శూన్య jsonReturnStringAsBlob(JsonString *pStr){
  JsonParse px;
  ప్రతిజ్ఞా( pStr->eErr==0 );
  memset(&px, 0, పరిమాణం(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->n_వాడిన;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (శూన్య)json_Translate_పాఠ్యం_గమ్యం_Blob(&px, 0);
  ఒకవేళ( px.oom ){
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_ఫలితం_error_nomem(pStr->pCtx);
  }లేకపోతే{
    ప్రతిజ్ఞా( px.nBlobAlloc>0 );
    ప్రతిజ్ఞా( !px.bReadOnly );
    sqlite3_ఫలితం_blob(pStr->pCtx, px.aBlob, px.nBlob, SQLITE_DYNAMIC);
  }
}

/*
** This a helper routine for jsonbPayloadSize() and
** jsonbPayloadSizeSemiInline().  This routine is called using tail
** recursion to handle the (relatively uncommon) cases where the
** the payload size is a 2, 4, or 8 byte integer.
*/
స్థిర u32 jsonb_Payload_కొలత_Wide(
  స్థిరరాశి JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz,                  /* Write payload size here */
  u8 x                       /* pParse->aBlob[i]>>4 */
){
  u32 sz;
  u32 n;
  ప్రతిజ్ఞా( i<pParse->nBlob );
  ప్రతిజ్ఞా( x==(pParse->aBlob[i]>>4) );
  ప్రతిజ్ఞా( x>=13 );
  ఒకవేళ( x==13 ){
    ఒకవేళ( i+2>=pParse->nBlob ){
      *pSz = 0;
      తిరిగి 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }లేకపోతే ఒకవేళ( x==14 ){
    ఒకవేళ( i+4>=pParse->nBlob ){
      *pSz = 0;
      తిరిగి 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }లేకపోతే{
    ఒకవేళ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      తిరిగి 0;
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
  ఒకవేళ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta  /*NO_TEST*/
  ){
    *pSz = 0;
    తిరిగి 0;
  }
  *pSz = sz;
  తిరిగి n;
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
స్థిర u32 jsonb_Payload_కొలత(
  స్థిరరాశి JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ఒకవేళ( i>=pParse->nBlob ){
    *pSz = 0;
    తిరిగి 0;
  }లేకపోతే ఒకవేళ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }లేకపోతే ఒకవేళ( x==12 ){
    ఒకవేళ( i+1>=pParse->nBlob ){
      *pSz = 0;
      తిరిగి 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }లేకపోతే{
    తిరిగి jsonb_Payload_కొలత_Wide(pParse, i, pSz, x);
  }
  ఒకవేళ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    తిరిగి 0;
  }
  *pSz = sz;
  తిరిగి n;
}

#ఒకవేళ SQLITE_ఉపయోగిస్తుంది_INLINE
/* A separate inline version of jsonbPayloadSize(), used in one particulary
** performance-critical place.
*/
స్థిర SQLITE_INLINE u32 jsonb_Payload_కొలత_Inline(
  స్థిరరాశి JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ఒకవేళ( i>=pParse->nBlob ){
    *pSz = 0;
    తిరిగి 0;
  }లేకపోతే ఒకవేళ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }లేకపోతే ఒకవేళ( x==12 ){
    ఒకవేళ( i+1>=pParse->nBlob ){
      *pSz = 0;
      తిరిగి 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }లేకపోతే ఒకవేళ( x==13 ){
    ఒకవేళ( i+2>=pParse->nBlob ){
      *pSz = 0;
      తిరిగి 0;
    }
    sz = (pParse->aBlob[i+1]<<8) + pParse->aBlob[i+2];
    n = 3;
  }లేకపోతే ఒకవేళ( x==14 ){
    ఒకవేళ( i+4>=pParse->nBlob ){
      *pSz = 0;
      తిరిగి 0;
    }
    sz = ((u32)pParse->aBlob[i+1]<<24) + (pParse->aBlob[i+2]<<16) +
         (pParse->aBlob[i+3]<<8) + pParse->aBlob[i+4];
    n = 5;
  }లేకపోతే{
    ఒకవేళ( i+8>=pParse->nBlob
     || pParse->aBlob[i+1]!=0
     || pParse->aBlob[i+2]!=0
     || pParse->aBlob[i+3]!=0
     || pParse->aBlob[i+4]!=0
    ){
      *pSz = 0;
      తిరిగి 0;
    }
    sz = ((u32)pParse->aBlob[i+5]<<24) + (pParse->aBlob[i+6]<<16) +
         (pParse->aBlob[i+7]<<8) + pParse->aBlob[i+8];
    n = 9;
  }
  ఒకవేళ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    తిరిగి 0;
  }
  *pSz = sz;
  తిరిగి n;
}
#లేకపోతే /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_కొలత_Inline(a,b,c) jsonb_Payload_కొలత(a,b,c)
#అయితే_ముగింపు


#ఒకవేళ SQLITE_ఉపయోగిస్తుంది_INLINE
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
స్థిర SQLITE_INLINE u32 jsonb_Payload_కొలత_Semi_Inline(
  స్థిరరాశి JsonParse *pParse,   /* JSON parsing context */
  u32 i,                     /* Index of the node type-code */
  u32 *pSz                   /* Write payload size here */
){
  u8 x;
  u32 sz;
  u32 n;
  ప్రతిజ్ఞా( i<pParse->nBlob );
  ఒకవేళ( (x = pParse->aBlob[i]>>4)<=11 ){
    sz = x;
    n = 1;
  }లేకపోతే ఒకవేళ( x==12 ){
    ఒకవేళ( i+1>=pParse->nBlob ){
      *pSz = 0;
      తిరిగి 0;
    }
    sz = pParse->aBlob[i+1];
    n = 2;
  }లేకపోతే{
    తిరిగి jsonb_Payload_కొలత_Wide(pParse,i,pSz,x);
  }
  ఒకవేళ( (i64)i+sz+n > pParse->nBlob
   && (i64)i+sz+n > pParse->nBlob-pParse->delta
  ){
    *pSz = 0;
    తిరిగి 0;
  }
  *pSz = sz;
  తిరిగి n;
}
#లేకపోతే /* if !SQLITE_USES_INLINE */
  /* On compilers that do not support in-lining, just call the original
  ** jsonbPayloadSize().  It's slower, but it gets the right answer. */
# define jsonb_Payload_కొలత_Semi_Inline(a,b,c) jsonb_Payload_కొలత(a,b,c)
#అయితే_ముగింపు

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
స్థిర u32 json_Translate_Blob_గమ్యం_పాఠ్యం(
  JsonParse *pParse,             /* the complete parse of the JSON */
  u32 i,                         /* Start rendering at this index */
  JsonString *p_బహిర్గతం               /* Write JSON here */
){
  u32 sz, n, j, i_చివర;

  n = jsonb_Payload_కొలత_Inline(pParse, i, &sz);
  ఒకవేళ( n==0 ){
    p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
    తిరిగి pParse->nBlob+1;
  }
  ఎంపిక( pParse->aBlob[i] & 0x0f ){
    స్థితి JSONB_NULL: {
      jsonAppendRawNZ(p_బహిర్గతం, "null", 4);
      తిరిగి i+1;
    }
    స్థితి JSONB_TRUE: {
      jsonAppendRawNZ(p_బహిర్గతం, "true", 4);
      తిరిగి i+1;
    }
    స్థితి JSONB_FALSE: {
      jsonAppendRawNZ(p_బహిర్గతం, "false", 5);
      తిరిగి i+1;
    }
    స్థితి JSONB_INT:
    స్థితి JSONB_FLOAT: {
      ఒకవేళ( sz==0 ) వెళ్ళు malformed_jsonb;
      jsonAppendRaw(p_బహిర్గతం, (స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n], sz);
      ఆపు;
    }
    స్థితి JSONB_INT5: {  /* Integer literal in hexadecimal notation */
      u32 k = 2;
      sqlite3_uint64 u = 0;
      స్థిరరాశి అక్షరం *z_లోపల = (స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n];
      పూర్ణాంకం b_అధికప్రవాహం = 0;
      ఒకవేళ( sz==0 ) వెళ్ళు malformed_jsonb;
      ఒకవేళ( z_లోపల[0]=='-' ){
        jsonAppendChar(p_బహిర్గతం, '-');
        k++;
      }లేకపోతే ఒకవేళ( z_లోపల[0]=='+' ){
        k++;
      }
      కొరకు(; k<sz; k++){
        ఒకవేళ( !sqlite3Isxdigit(z_లోపల[k]) ){
          p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
          ఆపు;
        }లేకపోతే ఒకవేళ( (u>>60)!=0 ){
          b_అధికప్రవాహం = 1;
        }లేకపోతే{
          u = u*16 + sqlite3_Hex_గమ్యం_Int(z_లోపల[k]);
        }
      }
      jsonPrintf(100,p_బహిర్గతం,b_అధికప్రవాహం?"9.0e999":"%llu", u);
      ఆపు;
    }
    స్థితి JSONB_FLOAT5: { /* Float literal missing digits beside "." */
      u32 k = 0;
      స్థిరరాశి అక్షరం *z_లోపల = (స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n];
      ఒకవేళ( sz==0 ) వెళ్ళు malformed_jsonb;
      ఒకవేళ( z_లోపల[0]=='-' ){
        jsonAppendChar(p_బహిర్గతం, '-');
        ఒకవేళ( sz<=1 ) వెళ్ళు malformed_jsonb;
        k = 1;
      }
      ఒకవేళ( z_లోపల[k]=='.' ){
        jsonAppendChar(p_బహిర్గతం, '0');
      }
      కొరకు(; k<sz; k++){
        jsonAppendChar(p_బహిర్గతం, z_లోపల[k]);
        ఒకవేళ( z_లోపల[k]=='.' && (k+1==sz || !sqlite3Isdigit(z_లోపల[k+1])) ){
          jsonAppendChar(p_బహిర్గతం, '0');
        }
      }
      ఆపు;
    }
    స్థితి JSONB_పాఠ్యం:
    స్థితి JSONB_TEXTJ: {
      ఒకవేళ( p_బహిర్గతం->n_వాడిన+sz+2<=p_బహిర్గతం->nAlloc || jsonStringGrow(p_బహిర్గతం, sz+2)==0 ){
        p_బహిర్గతం->zBuf[p_బహిర్గతం->n_వాడిన] = '"';
        జ్ఞాపకం_నకలు(p_బహిర్గతం->zBuf+p_బహిర్గతం->n_వాడిన+1,(స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n],sz);
        p_బహిర్గతం->zBuf[p_బహిర్గతం->n_వాడిన+sz+1] = '"';
        p_బహిర్గతం->n_వాడిన += sz+2;
      }
      ఆపు;
    }
    స్థితి JSONB_TEXT5: {
      స్థిరరాశి అక్షరం *z_లోపల;
      u32 k;
      u32 sz2 = sz;
      z_లోపల = (స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n];
      jsonAppendChar(p_బహిర్గతం, '"');
      అయితే( sz2>0 ){
        కొరకు(k=0; k<sz2 && (jsonIsOk[(u8)z_లోపల[k]] || z_లోపల[k]=='\''); k++){}
        ఒకవేళ( k>0 ){
          jsonAppendRawNZ(p_బహిర్గతం, z_లోపల, k);
          ఒకవేళ( k>=sz2 ){
            ఆపు;
          }
          z_లోపల += k;
          sz2 -= k;
        }
        ఒకవేళ( z_లోపల[0]=='"' ){
          jsonAppendRawNZ(p_బహిర్గతం, "\\\"", 2);
          z_లోపల++;
          sz2--;
          కొనసాగు;
        }
        ఒకవేళ( z_లోపల[0]<=0x1f ){
          ఒకవేళ( p_బహిర్గతం->n_వాడిన+7>p_బహిర్గతం->nAlloc && jsonStringGrow(p_బహిర్గతం,7) ) ఆపు;
          jsonAppendControlChar(p_బహిర్గతం, z_లోపల[0]);
          z_లోపల++;
          sz2--;
          కొనసాగు;
        }
        ప్రతిజ్ఞా( z_లోపల[0]=='\\' );
        ప్రతిజ్ఞా( sz2>=1 );
        ఒకవేళ( sz2<2 ){
          p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
          ఆపు;
        }
        ఎంపిక( (u8)z_లోపల[1] ){
          స్థితి '\'':
            jsonAppendChar(p_బహిర్గతం, '\'');
            ఆపు;
          స్థితి 'v':
            jsonAppendRawNZ(p_బహిర్గతం, "\\u000b", 6);
            ఆపు;
          స్థితి 'x':
            ఒకవేళ( sz2<4 ){
              p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ఆపు;
            }
            jsonAppendRawNZ(p_బహిర్గతం, "\\u00", 4);
            jsonAppendRawNZ(p_బహిర్గతం, &z_లోపల[2], 2);
            z_లోపల += 2;
            sz2 -= 2;
            ఆపు;
          స్థితి '0':
            jsonAppendRawNZ(p_బహిర్గతం, "\\u0000", 6);
            ఆపు;
          స్థితి '\r':
            ఒకవేళ( sz2>2 && z_లోపల[2]=='\n' ){
              z_లోపల++;
              sz2--;
            }
            ఆపు;
          స్థితి '\n':
            ఆపు;
          స్థితి 0xe2:
            /* '\' followed by either U+2028 or U+2029 is ignored as
            ** whitespace.  Note that in UTF8, U+2028 is 0xe2 0x80 0x29.
            ** U+2029 is the same except for the last byte */
            ఒకవేళ( sz2<4
             || 0x80!=(u8)z_లోపల[2]
             || (0xa8!=(u8)z_లోపల[3] && 0xa9!=(u8)z_లోపల[3])
            ){
              p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
              sz2 = 2;
              ఆపు;
            }
            z_లోపల += 2;
            sz2 -= 2;
            ఆపు;
          అప్రమేయ:
            jsonAppendRawNZ(p_బహిర్గతం, z_లోపల, 2);
            ఆపు;
        }
        ప్రతిజ్ఞా( sz2>=2 );
        z_లోపల += 2;
        sz2 -= 2;
      }
      jsonAppendChar(p_బహిర్గతం, '"');
      ఆపు;
    }
    స్థితి JSONB_TEXTRAW: {
      jsonAppendString(p_బహిర్గతం, (స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n], sz);
      ఆపు;
    }
    స్థితి JSONB_ARRAY: {
      jsonAppendChar(p_బహిర్గతం, '[');
      j = i+n;
      i_చివర = j+sz;
      ఒకవేళ( ++pParse->i_లోతు > JSON_MAX_లోతు ){
        jsonStringTooDeep(p_బహిర్గతం);
      }
      అయితే( j<i_చివర && p_బహిర్గతం->eErr==0 ){
        j = json_Translate_Blob_గమ్యం_పాఠ్యం(pParse, j, p_బహిర్గతం);
        jsonAppendChar(p_బహిర్గతం, ',');
      }
      pParse->i_లోతు--;
      ఒకవేళ( j>i_చివర ) p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
      ఒకవేళ( sz>0 ) jsonStringTrimOneChar(p_బహిర్గతం);
      jsonAppendChar(p_బహిర్గతం, ']');
      ఆపు;
    }
    స్థితి JSONB_OBJECT: {
      పూర్ణాంకం x = 0;
      jsonAppendChar(p_బహిర్గతం, '{');
      j = i+n;
      i_చివర = j+sz;
      ఒకవేళ( ++pParse->i_లోతు > JSON_MAX_లోతు ){
        jsonStringTooDeep(p_బహిర్గతం);
      }
      అయితే( j<i_చివర && p_బహిర్గతం->eErr==0 ){
        j = json_Translate_Blob_గమ్యం_పాఠ్యం(pParse, j, p_బహిర్గతం);
        jsonAppendChar(p_బహిర్గతం, (x++ & 1) ? ',' : ':');
      }
      pParse->i_లోతు--;
      ఒకవేళ( (x & 1)!=0 || j>i_చివర ) p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
      ఒకవేళ( sz>0 ) jsonStringTrimOneChar(p_బహిర్గతం);
      jsonAppendChar(p_బహిర్గతం, '}');
      ఆపు;
    }

    అప్రమేయ: {
      malformed_jsonb:
      p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
      ఆపు;
    }
  }
  తిరిగి i+n+sz;
}

/* Context for recursion of json_pretty()
*/
రకనిర్వచనం నిర్మాణం JsonPretty JsonPretty;
నిర్మాణం JsonPretty {
  JsonParse *pParse;        /* The BLOB being rendered */
  JsonString *p_బహిర్గతం;         /* Generate pretty output into this string */
  స్థిరరాశి అక్షరం *zIndent;      /* Use this text for indentation */
  u32 szIndent;             /* Bytes in zIndent[] */
  u32 nIndent;              /* Current level of indentation */
};

/* Append indentation to the pretty JSON under construction */
స్థిర శూన్య jsonPrettyIndent(JsonPretty *pPretty){
  u32 jj;
  కొరకు(jj=0; jj<pPretty->nIndent; jj++){
    jsonAppendRaw(pPretty->p_బహిర్గతం, pPretty->zIndent, pPretty->szIndent);
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
స్థిర u32 json_Translate_Blob_గమ్యం_Pretty_పాఠ్యం(
  JsonPretty *pPretty,       /* Pretty-printing context */
  u32 i                      /* Start rendering at this index */
){
  u32 sz, n, j, i_చివర;
  JsonParse *pParse = pPretty->pParse;
  JsonString *p_బహిర్గతం = pPretty->p_బహిర్గతం;
  n = jsonb_Payload_కొలత(pParse, i, &sz);
  ఒకవేళ( n==0 ){
    p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
    తిరిగి pParse->nBlob+1;
  }
  ఎంపిక( pParse->aBlob[i] & 0x0f ){
    స్థితి JSONB_ARRAY: {
      j = i+n;
      i_చివర = j+sz;
      jsonAppendChar(p_బహిర్గతం, '[');
      ఒకవేళ( j<i_చివర ){
        jsonAppendChar(p_బహిర్గతం, '\n');
        pPretty->nIndent++;
        ఒకవేళ( pPretty->nIndent >= JSON_MAX_లోతు ){
          jsonStringTooDeep(p_బహిర్గతం);
        }
        అయితే( p_బహిర్గతం->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_గమ్యం_Pretty_పాఠ్యం(pPretty, j);
          ఒకవేళ( j>=i_చివర ){
            ఒకవేళ( j>i_చివర ) p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
            ఆపు;
          }
          jsonAppendRawNZ(p_బహిర్గతం, ",\n", 2);
        }
        jsonAppendChar(p_బహిర్గతం, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_బహిర్గతం, ']');
      i = i_చివర;
      ఆపు;
    }
    స్థితి JSONB_OBJECT: {
      j = i+n;
      i_చివర = j+sz;
      jsonAppendChar(p_బహిర్గతం, '{');
      ఒకవేళ( j<i_చివర ){
        jsonAppendChar(p_బహిర్గతం, '\n');
        pPretty->nIndent++;
        ఒకవేళ( pPretty->nIndent >= JSON_MAX_లోతు ){
          jsonStringTooDeep(p_బహిర్గతం);
        }
        pParse->i_లోతు = pPretty->nIndent;
        అయితే( p_బహిర్గతం->eErr==0 ){
          jsonPrettyIndent(pPretty);
          j = json_Translate_Blob_గమ్యం_పాఠ్యం(pParse, j, p_బహిర్గతం);
          ఒకవేళ( j>i_చివర ){
            p_బహిర్గతం->eErr |= JSTRING_MALFORMED;
            ఆపు;
          }
          jsonAppendRawNZ(p_బహిర్గతం, ": ", 2);
          j = json_Translate_Blob_గమ్యం_Pretty_పాఠ్యం(pPretty, j);
          ఒకవేళ( j>=i_చివర ) ఆపు;
          jsonAppendRawNZ(p_బహిర్గతం, ",\n", 2);
        }
        jsonAppendChar(p_బహిర్గతం, '\n');
        pPretty->nIndent--;
        jsonPrettyIndent(pPretty);
      }
      jsonAppendChar(p_బహిర్గతం, '}');
      i = i_చివర;
      ఆపు;
    }
    అప్రమేయ: {
      i = json_Translate_Blob_గమ్యం_పాఠ్యం(pParse, i, p_బహిర్గతం);
      ఆపు;
    }
  }
  తిరిగి i;
}

/*
** Given that a JSONB_ARRAY object starts at offset i, return
** the number of entries in that array.
*/
స్థిర u32 jsonb_Array_లెక్క(JsonParse *pParse, u32 i_మూలం){
  u32 n, sz, i, i_చివర;
  u32 k = 0;
  n = jsonb_Payload_కొలత(pParse, i_మూలం, &sz);
  i_చివర = i_మూలం+n+sz;
  కొరకు(i=i_మూలం+n; n>0 && i<i_చివర; i+=sz+n, k++){
    n = jsonb_Payload_కొలత_Semi_Inline(pParse, i, &sz);
  }
  తిరిగి k;
}

/*
** Edit the payload size of the element at iRoot by the amount in
** pParse->delta.
*/
స్థిర శూన్య json_After_Edit_కొలత_Adjust(JsonParse *pParse, u32 i_మూలం){
  u32 sz = 0;
  u32 nBlob;
  ప్రతిజ్ఞా( pParse->delta!=0 );
  ప్రతిజ్ఞా( pParse->nBlobAlloc >= pParse->nBlob );
  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (శూన్య)jsonb_Payload_కొలత(pParse, i_మూలం, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += json_Blob_Change_Payload_కొలత(pParse, i_మూలం, sz);
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
స్థిర పూర్ణాంకం jsonBlobOverwrite(
  u8 *a_బహిర్గతం,                 /* Overwrite here */
  స్థిరరాశి u8 *aIns,           /* New content */
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
  స్థిర స్థిరరాశి u8 aType[] = { 0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0 };

  ఒకవేళ( (aIns[0]&0x0f)<=2 ) తిరిగి 0;    /* Cannot enlarge NULL, true, false */
  ఎంపిక( aIns[0]>>4 ){
    అప్రమేయ: {                         /* aIns[] header size 1 */
      ఒకవేళ( ((1<<d)&0x116)==0 ) తిరిగి 0;  /* d must be 1, 2, 4, or 8 */
      i = d + 1;                         /* New hdr sz: 2, 3, 5, or 9 */
      szHdr = 1;
      ఆపు;
    }
    స్థితి 12: {                         /* aIns[] header size is 2 */
      ఒకవేళ( ((1<<d)&0x8a)==0) తిరిగి 0;    /* d must be 1, 3, or 7 */
      i = d + 2;                         /* New hdr sz: 2, 5, or 9 */
      szHdr = 2;
      ఆపు;
    }
    స్థితి 13: {                         /* aIns[] header size is 3 */
      ఒకవేళ( d!=2 && d!=6 ) తిరిగి 0;       /* d must be 2 or 6 */
      i = d + 3;                         /* New hdr sz: 5 or 9 */
      szHdr = 3;
      ఆపు;
    }
    స్థితి 14: {                         /* aIns[] header size is 5 */
      ఒకవేళ( d!=4 ) తిరిగి 0;               /* d must be 4 */
      i = 9;                             /* New hdr sz: 9 */
      szHdr = 5;
      ఆపు;
    }
    స్థితి 15: {                         /* aIns[] header size is 9 */
      తిరిగి 0;                          /* No solution */
    }
  }
  ప్రతిజ్ఞా( i>=2 && i<=9 && aType[i-2]!=0 );
  a_బహిర్గతం[0] = (aIns[0] & 0x0f) | aType[i-2];
  జ్ఞాపకం_నకలు(&a_బహిర్గతం[i], &aIns[szHdr], nIns-szHdr);
  szPayload = nIns - szHdr;
  అయితే( 1/*edit-by-break*/ ){
    i--;
    a_బహిర్గతం[i] = szPayload & 0xff;
    ఒకవేళ( i==1 ) ఆపు;
    szPayload >>= 8;
  }
  ప్రతిజ్ఞా( (szPayload>>8)==0 );
  తిరిగి 1;
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
స్థిర శూన్య jsonBlobEdit(
  JsonParse *pParse,     /* The JSONB to be modified is in pParse->aBlob */
  u32 iDel,              /* First byte to be removed */
  u32 nDel,              /* Number of bytes to remove */
  స్థిరరాశి u8 *aIns,        /* Content to insert */
  u32 nIns               /* Bytes of content to insert */
){
  i64 d = (i64)nIns - (i64)nDel;
  ప్రతిజ్ఞా( pParse->nBlob >= (u64)iDel + (u64)nDel );
  ఒకవేళ( d<0 && d>=(-8) && aIns!=0
   && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (పూర్ణాంకం)-d)
  ){
    తిరిగి;
  }
  ఒకవేళ( d!=0 ){
    ఒకవేళ( pParse->nBlob + d > pParse->nBlobAlloc ){
      jsonBlobExpand(pParse, (u64)pParse->nBlob+d);
      ఒకవేళ( pParse->oom ) తిరిగి;
    }
    memmove(&pParse->aBlob[iDel+nIns],
            &pParse->aBlob[iDel+nDel],
            pParse->nBlob - (iDel+nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  ఒకవేళ( nIns && aIns ){
    జ్ఞాపకం_నకలు(&pParse->aBlob[iDel], aIns, nIns);
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
స్థిర u32 json_బైట్లు_గమ్యం_Bypass(స్థిరరాశి అక్షరం *z, u32 n){
  u32 i = 0;
  అయితే( i+1<n ){
    ఒకవేళ( z[i]!='\\' ) తిరిగి i;
    ఒకవేళ( z[i+1]=='\n' ){
      i += 2;
      కొనసాగు;
    }
    ఒకవేళ( z[i+1]=='\r' ){
      ఒకవేళ( i+2<n && z[i+2]=='\n' ){
        i += 3;
      }లేకపోతే{
        i += 2;
      }
      కొనసాగు;
    }
    ఒకవేళ( 0xe2==(u8)z[i+1]
     && i+3<n
     && 0x80==(u8)z[i+2]
     && (0xa8==(u8)z[i+3] || 0xa9==(u8)z[i+3])
    ){
      i += 4;
      కొనసాగు;
    }
    ఆపు;
  }
  తిరిగి i;
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
స్థిర u32 jsonUnescapeOneChar(స్థిరరాశి అక్షరం *z, u32 n, u32 *pi_బహిర్గతం){
  ప్రతిజ్ఞా( n>0 );
  ప్రతిజ్ఞా( z[0]=='\\' );
  ఒకవేళ( n<2 ){
    *pi_బహిర్గతం = JSON_INVALID_CHAR;
    తిరిగి n;
  }
  ఎంపిక( (u8)z[1] ){
    స్థితి 'u': {
      u32 v, vlo;
      ఒకవేళ( n<6 ){
        *pi_బహిర్గతం = JSON_INVALID_CHAR;
        తిరిగి n;
      }
      v = json_Hex_గమ్యం_Int4(&z[2]);
      ఒకవేళ( (v & 0xfc00)==0xd800
       && n>=12
       && z[6]=='\\'
       && z[7]=='u'
       && ((vlo = json_Hex_గమ్యం_Int4(&z[8]))&0xfc00)==0xdc00
      ){
        *pi_బహిర్గతం = ((v&0x3ff)<<10) + (vlo&0x3ff) + 0x10000;
        తిరిగి 12;
      }లేకపోతే{
        *pi_బహిర్గతం = v;
        తిరిగి 6;
      }
    }
    స్థితి 'b': {   *pi_బహిర్గతం = '\b';  తిరిగి 2; }
    స్థితి 'f': {   *pi_బహిర్గతం = '\f';  తిరిగి 2; }
    స్థితి 'n': {   *pi_బహిర్గతం = '\n';  తిరిగి 2; }
    స్థితి 'r': {   *pi_బహిర్గతం = '\r';  తిరిగి 2; }
    స్థితి 't': {   *pi_బహిర్గతం = '\t';  తిరిగి 2; }
    స్థితి 'v': {   *pi_బహిర్గతం = '\v';  తిరిగి 2; }
    స్థితి '0': {
      /* JSON5 requires that the \0 escape not be followed by a digit.
      ** But SQLite did not enforce this restriction in versions 3.42.0
      ** through 3.49.2.  That was a bug.  But some applications might have
      ** come to depend on that bug.  Use the SQLITE_BUG_COMPATIBLE_20250510
      ** option to restore the old buggy behavior. */
#నిర్వచితమైతే SQLITE_BUG_COMPATIBLE_20250510
      /* Legacy bug-compatible behavior */
      *pi_బహిర్గతం = 0;
#లేకపోతే
      /* Correct behavior */
      *pi_బహిర్గతం = (n>2 && sqlite3Isdigit(z[2])) ? JSON_INVALID_CHAR : 0;
#అయితే_ముగింపు
      తిరిగి 2;
    }
    స్థితి '\'':
    స్థితి '"':
    స్థితి '/':
    స్థితి '\\':{   *pi_బహిర్గతం = z[1];  తిరిగి 2; }
    స్థితి 'x': {
      ఒకవేళ( n<4 ){
        *pi_బహిర్గతం = JSON_INVALID_CHAR;
        తిరిగి n;
      }
      *pi_బహిర్గతం = (json_Hex_గమ్యం_Int(z[2])<<4) | json_Hex_గమ్యం_Int(z[3]);
      తిరిగి 4;
    }
    స్థితి 0xe2:
    స్థితి '\r':
    స్థితి '\n': {
      u32 nSkip = json_బైట్లు_గమ్యం_Bypass(z, n);
      ఒకవేళ( nSkip==0 ){
        *pi_బహిర్గతం = JSON_INVALID_CHAR;
        తిరిగి n;
      }లేకపోతే ఒకవేళ( nSkip==n ){
        *pi_బహిర్గతం = 0;
        తిరిగి n;
      }లేకపోతే ఒకవేళ( z[nSkip]=='\\' ){
        తిరిగి nSkip + jsonUnescapeOneChar(&z[nSkip], n-nSkip, pi_బహిర్గతం);
      }లేకపోతే{
        పూర్ణాంకం sz = sqlite3Utf8ReadLimited((u8*)&z[nSkip], n-nSkip, pi_బహిర్గతం);
        తిరిగి nSkip + sz;
      }
    }
    అప్రమేయ: {
      *pi_బహిర్గతం = JSON_INVALID_CHAR;
      తిరిగి 2;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం jsonLabelCompareEscaped(
  స్థిరరాశి అక్షరం *z_ఎడమ,          /* The left label */
  u32 n_ఎడమ,                  /* Size of the left label in bytes */
  పూర్ణాంకం raw_ఎడమ,                /* True if zLeft contains no escapes */
  స్థిరరాశి అక్షరం *z_కుడి,         /* The right label */
  u32 n_కుడి,                 /* Size of the right label in bytes */
  పూర్ణాంకం raw_కుడి                /* True if zRight is escape-free */
){
  u32 c_ఎడమ, c_కుడి;
  ప్రతిజ్ఞా( raw_ఎడమ==0 || raw_కుడి==0 );
  అయితే( 1 /*exit-by-return*/ ){
    ఒకవేళ( n_ఎడమ==0 ){
      c_ఎడమ = 0;
    }లేకపోతే ఒకవేళ( raw_ఎడమ || z_ఎడమ[0]!='\\' ){
      c_ఎడమ = ((u8*)z_ఎడమ)[0];
      ఒకవేళ( c_ఎడమ>=0xc0 ){
        పూర్ణాంకం sz = sqlite3Utf8ReadLimited((u8*)z_ఎడమ, n_ఎడమ, &c_ఎడమ);
        z_ఎడమ += sz;
        n_ఎడమ -= sz;
      }లేకపోతే{
        z_ఎడమ++;
        n_ఎడమ--;
      }
    }లేకపోతే{
      u32 n = jsonUnescapeOneChar(z_ఎడమ, n_ఎడమ, &c_ఎడమ);
      z_ఎడమ += n;
      ప్రతిజ్ఞా( n<=n_ఎడమ );
      n_ఎడమ -= n;
    }
    ఒకవేళ( n_కుడి==0 ){
      c_కుడి = 0;
    }లేకపోతే ఒకవేళ( raw_కుడి || z_కుడి[0]!='\\' ){
      c_కుడి = ((u8*)z_కుడి)[0];
      ఒకవేళ( c_కుడి>=0xc0 ){
        పూర్ణాంకం sz = sqlite3Utf8ReadLimited((u8*)z_కుడి, n_కుడి, &c_కుడి);
        z_కుడి += sz;
        n_కుడి -= sz;
      }లేకపోతే{
        z_కుడి++;
        n_కుడి--;
      }
    }లేకపోతే{
      u32 n = jsonUnescapeOneChar(z_కుడి, n_కుడి, &c_కుడి);
      z_కుడి += n;
      ప్రతిజ్ఞా( n<=n_కుడి );
      n_కుడి -= n;
    }
    ఒకవేళ( c_ఎడమ!=c_కుడి ) తిరిగి 0;
    ఒకవేళ( c_ఎడమ==0 ) తిరిగి 1;
  }
}

/*
** Compare two object labels.  Return 1 if they are equal and
** 0 if they differ.  Return -1 if an OOM occurs.
*/
స్థిర పూర్ణాంకం jsonLabelCompare(
  స్థిరరాశి అక్షరం *z_ఎడమ,          /* The left label */
  u32 n_ఎడమ,                  /* Size of the left label in bytes */
  పూర్ణాంకం raw_ఎడమ,                /* True if zLeft contains no escapes */
  స్థిరరాశి అక్షరం *z_కుడి,         /* The right label */
  u32 n_కుడి,                 /* Size of the right label in bytes */
  పూర్ణాంకం raw_కుడి                /* True if zRight is escape-free */
){
  ఒకవేళ( raw_ఎడమ && raw_కుడి ){
    /* Simpliest case:  Neither label contains escapes.  A simple
    ** memcmp() is sufficient. */
    ఒకవేళ( n_ఎడమ!=n_కుడి ) తిరిగి 0;
    తిరిగి memcmp(z_ఎడమ, z_కుడి, n_ఎడమ)==0;
  }లేకపోతే{
    తిరిగి jsonLabelCompareEscaped(z_ఎడమ, n_ఎడమ, raw_ఎడమ,
                                   z_కుడి, n_కుడి, raw_కుడి);
  }
}

/*
** Error returns from jsonLookupStep()
*/
#నిర్వచించు JSON_LOOKUP_ERROR      0xffffffff
#నిర్వచించు JSON_LOOKUP_NOTFOUND   0xfffffffe
#నిర్వచించు JSON_LOOKUP_NOTARRAY   0xfffffffd
#నిర్వచించు JSON_LOOKUP_TOODEEP    0xfffffffc
#నిర్వచించు JSON_LOOKUP_PATHERROR  0xfffffffb
#నిర్వచించు JSON_LOOKUP_ISERROR(x) ((x)>=JSON_LOOKUP_PATHERROR)

/* Forward declaration */
స్థిర u32 json_Lookup_అడుగు(JsonParse*,u32,స్థిరరాశి అక్షరం*,u32);


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
స్థిర u32 jsonCreateEditSubstructure(
  JsonParse *pParse,  /* The original JSONB that is being edited */
  JsonParse *pIns,    /* Populate this with the blob data to insert */
  స్థిరరాశి అక్షరం *z_తోక   /* Tail of the path that determines substructure */
){
  స్థిర స్థిరరాశి u8 ఖాళీలు_Object[] = { JSONB_ARRAY, JSONB_OBJECT };
  పూర్ణాంకం rc;
  memset(pIns, 0, పరిమాణం(*pIns));
  pIns->db = pParse->db;
  ఒకవేళ( z_తోక[0]==0 ){
    /* No substructure.  Just insert what is given in pParse. */
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  }లేకపోతే{
    /* Construct the binary substructure */
    pIns->nBlob = 1;
    pIns->aBlob = (u8*)&ఖాళీలు_Object[z_తోక[0]=='.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->i_లోతు = pParse->i_లోతు+1;
    ఒకవేళ( pIns->i_లోతు >= JSON_MAX_లోతు ){
      తిరిగి JSON_LOOKUP_TOODEEP;
    }
    rc = json_Lookup_అడుగు(pIns, 0, z_తోక, 0);
    pParse->i_లోతు--;
    pParse->oom |= pIns->oom;
  }
  తిరిగి rc;  /* Error code only */
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
స్థిర u32 json_Lookup_అడుగు(
  JsonParse *pParse,      /* The JSON to search */
  u32 i_మూలం,              /* Begin the search at this element of aBlob[] */
  స్థిరరాశి అక్షరం *z_మార్గం,      /* The path to search */
  u32 iLabel              /* Label if iRoot is a value of in an object */
){
  u32 i, j, k, n_కీలకం, sz, n, i_చివర, rc;
  స్థిరరాశి అక్షరం *z_కీలకం;
  u8 x;

  ఒకవేళ( z_మార్గం[0]==0 ){
    ఒకవేళ( pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns) ){
      n = jsonb_Payload_కొలత(pParse, i_మూలం, &sz);
      sz += n;
      ఒకవేళ( pParse->eEdit==JEDIT_DEL ){
        ఒకవేళ( iLabel>0 ){
          sz += i_మూలం - iLabel;
          i_మూలం = iLabel;
        }
        jsonBlobEdit(pParse, i_మూలం, sz, 0, 0);
      }లేకపోతే ఒకవేళ( pParse->eEdit==JEDIT_INS ){
        /* Already exists, so json_insert() is a no-op */
      }లేకపోతే ఒకవేళ( pParse->eEdit==JEDIT_AINS ){
        /* json_array_insert() */
        ఒకవేళ( z_మార్గం[-1]!=']' ){
          తిరిగి JSON_LOOKUP_NOTARRAY;
        }లేకపోతే{
          jsonBlobEdit(pParse, i_మూలం, 0, pParse->aIns, pParse->nIns);
        }
      }లేకపోతే{
        /* json_set() or json_replace() */
        jsonBlobEdit(pParse, i_మూలం, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    తిరిగి i_మూలం;
  }
  ఒకవేళ( z_మార్గం[0]=='.' ){
    పూర్ణాంకం raw_కీలకం = 1;
    x = pParse->aBlob[i_మూలం];
    z_మార్గం++;
    ఒకవేళ( z_మార్గం[0]=='"' ){
      z_కీలకం = z_మార్గం + 1;
      కొరకు(i=1; z_మార్గం[i] && z_మార్గం[i]!='"'; i++){
        ఒకవేళ( z_మార్గం[i]=='\\' && z_మార్గం[i+1]!=0 ) i++;
      }
      n_కీలకం = i-1;
      ఒకవేళ( z_మార్గం[i] ){
        i++;
      }లేకపోతే{
        తిరిగి JSON_LOOKUP_PATHERROR;
      }
      testcase( n_కీలకం==0 );
      raw_కీలకం = memchr(z_కీలకం, '\\', n_కీలకం)==0;
    }లేకపోతే{
      z_కీలకం = z_మార్గం;
      కొరకు(i=0; z_మార్గం[i] && z_మార్గం[i]!='.' && z_మార్గం[i]!='['; i++){}
      n_కీలకం = i;
      ఒకవేళ( n_కీలకం==0 ){
        తిరిగి JSON_LOOKUP_PATHERROR;
      }
    }
    ఒకవేళ( (x & 0x0f)!=JSONB_OBJECT ) తిరిగి JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_కొలత(pParse, i_మూలం, &sz);
    j = i_మూలం + n;  /* j is the index of a label */
    i_చివర = j+sz;
    అయితే( j<i_చివర ){
      పూర్ణాంకం rawLabel;
      స్థిరరాశి అక్షరం *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      ఒకవేళ( x<JSONB_పాఠ్యం || x>JSONB_TEXTRAW ) తిరిగి JSON_LOOKUP_ERROR;
      n = jsonb_Payload_కొలత_Semi_Inline(pParse, j, &sz);
      ఒకవేళ( n==0 ) తిరిగి JSON_LOOKUP_ERROR;
      k = j+n;  /* k is the index of the label text */
      ఒకవేళ( k+sz>=i_చివర ) తిరిగి JSON_LOOKUP_ERROR;
      zLabel = (స్థిరరాశి అక్షరం*)&pParse->aBlob[k];
      rawLabel = x==JSONB_పాఠ్యం || x==JSONB_TEXTRAW;
      ఒకవేళ( jsonLabelCompare(z_కీలకం, n_కీలకం, raw_కీలకం, zLabel, sz, rawLabel) ){
        u32 v = k+sz;  /* v is the index of the value */
        ఒకవేళ( ((pParse->aBlob[v])&0x0f)>JSONB_OBJECT ) తిరిగి JSON_LOOKUP_ERROR;
        n = jsonb_Payload_కొలత(pParse, v, &sz);
        ఒకవేళ( n==0 || v+n+sz>i_చివర ) తిరిగి JSON_LOOKUP_ERROR;
        ప్రతిజ్ఞా( j>0 );
        ఒకవేళ( ++pParse->i_లోతు >= JSON_MAX_లోతు ){
          తిరిగి JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_అడుగు(pParse, v, &z_మార్గం[i], j);
        pParse->i_లోతు--;
        ఒకవేళ( pParse->delta ) json_After_Edit_కొలత_Adjust(pParse, i_మూలం);
        తిరిగి rc;
      }
      j = k+sz;
      ఒకవేళ( ((pParse->aBlob[j])&0x0f)>JSONB_OBJECT ) తిరిగి JSON_LOOKUP_ERROR;
      n = jsonb_Payload_కొలత_Semi_Inline(pParse, j, &sz);
      ఒకవేళ( n==0 ) తిరిగి JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ఒకవేళ( j>i_చివర ) తిరిగి JSON_LOOKUP_ERROR;
    ఒకవేళ( pParse->eEdit>=JEDIT_INS ){
      u32 nIns;          /* Total bytes to insert (label+value) */
      JsonParse v;       /* BLOB encoding of the value to be inserted */
      JsonParse ix;      /* Header of the label to be inserted */
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_సమితి );
      testcase( pParse->eEdit==JEDIT_AINS );
      ఒకవేళ( pParse->eEdit==JEDIT_AINS && sqlite3_strglob("*]",&z_మార్గం[i])!=0 ){
        తిరిగి JSON_LOOKUP_NOTARRAY;
      }
      memset(&ix, 0, పరిమాణం(ix));
      ix.db = pParse->db;
      json_Blob_Append_కణుపు(&ix, raw_కీలకం?JSONB_TEXTRAW:JSONB_TEXT5, n_కీలకం, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &z_మార్గం[i]);
      ఒకవేళ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, ix.nBlob+n_కీలకం+v.nBlob)
      ){
        ప్రతిజ్ఞా( !pParse->oom );
        nIns = ix.nBlob + n_కీలకం + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        ఒకవేళ( !pParse->oom ){
          ప్రతిజ్ఞా( pParse->aBlob!=0 ); /* Because pParse->oom!=0 */
          ప్రతిజ్ఞా( ix.aBlob!=0 );      /* Because pPasre->oom!=0 */
          జ్ఞాపకం_నకలు(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          జ్ఞాపకం_నకలు(&pParse->aBlob[k], z_కీలకం, n_కీలకం);
          k += n_కీలకం;
          జ్ఞాపకం_నకలు(&pParse->aBlob[k], v.aBlob, v.nBlob);
          ఒకవేళ( ALWAYS(pParse->delta) ) json_After_Edit_కొలత_Adjust(pParse, i_మూలం);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      తిరిగి rc;
    }
  }లేకపోతే ఒకవేళ( z_మార్గం[0]=='[' ){
    u64 kk = 0;
    x = pParse->aBlob[i_మూలం] & 0x0f;
    ఒకవేళ( x!=JSONB_ARRAY )  తిరిగి JSON_LOOKUP_NOTFOUND;
    n = jsonb_Payload_కొలత(pParse, i_మూలం, &sz);
    i = 1;
    అయితే( sqlite3Isdigit(z_మార్గం[i]) ){
      ఒకవేళ( kk<0xffffffff ) kk = kk*10 + z_మార్గం[i] - '0';
      /*     ^^^^^^^^^^--- Allow kk to be bigger than any JSON array so that
      ** we get NOTFOUND instead of PATHERROR, without overflowing kk. */
      i++;
    }
    ఒకవేళ( i<2 || z_మార్గం[i]!=']' ){
      ఒకవేళ( z_మార్గం[1]=='#' ){
        kk = jsonb_Array_లెక్క(pParse, i_మూలం);
        i = 2;
        ఒకవేళ( z_మార్గం[2]=='-' && sqlite3Isdigit(z_మార్గం[3]) ){
          u64 nn = 0;
          i = 3;
          చేయి{
            ఒకవేళ( nn<0xffffffff ) nn = nn*10 + z_మార్గం[i] - '0';
            /*     ^^^^^^^^^^--- Allow nn to be bigger than any JSON array to
            ** get NOTFOUND instead of PATHERROR, without overflowing nn. */
            i++;
          }అయితే( sqlite3Isdigit(z_మార్గం[i]) );
          ఒకవేళ( nn>kk ) తిరిగి JSON_LOOKUP_NOTFOUND;
          kk -= nn;
        }
        ఒకవేళ( z_మార్గం[i]!=']' ){
          తిరిగి JSON_LOOKUP_PATHERROR;
        }
      }లేకపోతే{
        తిరిగి JSON_LOOKUP_PATHERROR;
      }
    }
    j = i_మూలం+n;
    i_చివర = j+sz;
    అయితే( j<i_చివర ){
      ఒకవేళ( kk==0 ){
        ఒకవేళ( ++pParse->i_లోతు >= JSON_MAX_లోతు ){
          తిరిగి JSON_LOOKUP_TOODEEP;
        }
        rc = json_Lookup_అడుగు(pParse, j, &z_మార్గం[i+1], 0);
        pParse->i_లోతు--;
        ఒకవేళ( pParse->delta ) json_After_Edit_కొలత_Adjust(pParse, i_మూలం);
        తిరిగి rc;
      }
      kk--;
      n = jsonb_Payload_కొలత_Semi_Inline(pParse, j, &sz);
      ఒకవేళ( n==0 ) తిరిగి JSON_LOOKUP_ERROR;
      j += n+sz;
    }
    ఒకవేళ( j>i_చివర ) తిరిగి JSON_LOOKUP_ERROR;
    ఒకవేళ( kk>0 ) తిరిగి JSON_LOOKUP_NOTFOUND;
    ఒకవేళ( pParse->eEdit>=JEDIT_INS ){
      JsonParse v;
      testcase( pParse->eEdit==JEDIT_INS );
      testcase( pParse->eEdit==JEDIT_AINS );
      testcase( pParse->eEdit==JEDIT_సమితి );
      rc = jsonCreateEditSubstructure(pParse, &v, &z_మార్గం[i+1]);
      ఒకవేళ( !JSON_LOOKUP_ISERROR(rc)
       && jsonBlobMakeEditable(pParse, v.nBlob)
      ){
        ప్రతిజ్ఞా( !pParse->oom );
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      ఒకవేళ( pParse->delta ) json_After_Edit_కొలత_Adjust(pParse, i_మూలం);
      తిరిగి rc;
    }
  }లేకపోతే{
    తిరిగి JSON_LOOKUP_PATHERROR; 
  }
  తిరిగి JSON_LOOKUP_NOTFOUND;
}

/*
** Convert a JSON BLOB into text and make that text the return value
** of an SQL function.
*/
స్థిర శూన్య json_Return_పాఠ్యం_Json_From_Blob(
  sqlite3_context *ctx,
  స్థిరరాశి u8 *aBlob,
  u32 nBlob
){
  JsonParse x;
  JsonString s;

  ఒకవేళ( NEVER(aBlob==0) ) తిరిగి;
  memset(&x, 0, పరిమాణం(x));
  x.aBlob = (u8*)aBlob;
  x.nBlob = nBlob;
  jsonStringInit(&s, ctx);
  json_Translate_Blob_గమ్యం_పాఠ్యం(&x, 0, &s);
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
స్థిర శూన్య jsonReturnFromBlob(
  JsonParse *pParse,          /* Complete JSON parse tree */
  u32 i,                      /* Index of the node */
  sqlite3_context *pCtx,      /* Return value for this function */
  పూర్ణాంకం e_బహుళకం                   /* Format of return: text of JSONB */
){
  u32 n, sz;
  పూర్ణాంకం rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  ప్రతిజ్ఞా( e_బహుళకం>=0 && e_బహుళకం<=2 );
  n = jsonb_Payload_కొలత(pParse, i, &sz);
  ఒకవేళ( n==0 ){
    sqlite3_ఫలితం_error(pCtx, "malformed JSON", -1);
    తిరిగి;
  }
  ఎంపిక( pParse->aBlob[i] & 0x0f ){
    స్థితి JSONB_NULL: {
      ఒకవేళ( sz ) వెళ్ళు returnfromblob_malformed;
      sqlite3_ఫలితం_null(pCtx);
      ఆపు;
    }
    స్థితి JSONB_TRUE: {
      ఒకవేళ( sz ) వెళ్ళు returnfromblob_malformed;
      sqlite3_ఫలితం_int(pCtx, 1);
      ఆపు;
    }
    స్థితి JSONB_FALSE: {
      ఒకవేళ( sz ) వెళ్ళు returnfromblob_malformed;
      sqlite3_ఫలితం_int(pCtx, 0);
      ఆపు;
    }
    స్థితి JSONB_INT5:
    స్థితి JSONB_INT: {
      sqlite3_int64 iRes = 0;
      అక్షరం *z;
      పూర్ణాంకం bNeg = 0;
      అక్షరం x;
      ఒకవేళ( sz==0 ) వెళ్ళు returnfromblob_malformed;
      x = (అక్షరం)pParse->aBlob[i+n];
      ఒకవేళ( x=='-' ){
        ఒకవేళ( sz<2 ) వెళ్ళు returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n], (పూర్ణాంకం)sz);
      ఒకవేళ( z==0 ) వెళ్ళు returnfromblob_oom;
      rc = sqlite3_Dec_Or_Hex_గమ్యం_I64(z, &iRes);
      sqlite3DbFree(db, z);
      ఒకవేళ( rc==0 ){
        ఒకవేళ( iRes<0 ){
          /* A hexadecimal literal with 16 significant digits and with the
          ** high-order bit set is a negative integer in SQLite (and hence
          ** iRes comes back as negative) but should be interpreted as a
          ** positive value if it occurs within JSON.  The value is too
          ** large to appear as an SQLite integer so it must be converted
          ** into floating point. */
          ద్విగుణ r;
          r = (ద్విగుణ)*(sqlite3_uint64*)&iRes;
          sqlite3_ఫలితం_double(pCtx, bNeg ? -r : r);
        }లేకపోతే{
          sqlite3_ఫలితం_int64(pCtx, bNeg ? -iRes : iRes);
        }
      }లేకపోతే ఒకవేళ( rc==3 && bNeg ){
        sqlite3_ఫలితం_int64(pCtx, అతిచిన్న_INT64);
      }లేకపోతే ఒకవేళ( rc==1 ){
        వెళ్ళు returnfromblob_malformed;
      }లేకపోతే{
        ఒకవేళ( bNeg ){ n--; sz++; }
        వెళ్ళు గమ్యం_double;
      }
      ఆపు;
    }
    స్థితి JSONB_FLOAT5:
    స్థితి JSONB_FLOAT: {
      ద్విగుణ r;
      అక్షరం *z;
      ఒకవేళ( sz==0 ) వెళ్ళు returnfromblob_malformed;
    గమ్యం_double:
      z = sqlite3DbStrNDup(db, (స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n], (పూర్ణాంకం)sz);
      ఒకవేళ( z==0 ) వెళ్ళు returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      ఒకవేళ( rc<=0 ) వెళ్ళు returnfromblob_malformed;
      sqlite3_ఫలితం_double(pCtx, r);
      ఆపు;
    }
    స్థితి JSONB_TEXTRAW:
    స్థితి JSONB_పాఠ్యం: {
      sqlite3_ఫలితం_పాఠ్యం(pCtx, (అక్షరం*)&pParse->aBlob[i+n], sz,
                          SQLITE_TRANSIENT);
      ఆపు;
    }
    స్థితి JSONB_TEXT5:
    స్థితి JSONB_TEXTJ: {
      /* Translate JSON formatted string into raw text */
      u32 i_లోపల, i_బహిర్గతం;
      స్థిరరాశి అక్షరం *z;
      అక్షరం *z_బహిర్గతం;
      u32 n_బహిర్గతం = sz;
      z = (స్థిరరాశి అక్షరం*)&pParse->aBlob[i+n];
      z_బహిర్గతం = sqlite3DbMallocRaw(db, ((u64)n_బహిర్గతం)+1);
      ఒకవేళ( z_బహిర్గతం==0 ) వెళ్ళు returnfromblob_oom;
      కొరకు(i_లోపల=i_బహిర్గతం=0; i_లోపల<sz; i_లోపల++){
        అక్షరం c = z[i_లోపల];
        ఒకవేళ( c=='\\' ){
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[i_లోపల], sz-i_లోపల, &v);
          ఒకవేళ( v==0 && i_లోపల+szEscape==sz
           && (z[i_లోపల+1]=='\n' || z[i_లోపల+1]=='\r' || (u8)z[i_లోపల+1]==0xe2)
          ){
            /* A JSON5 line continuation at the end of the string
            ** contributes no characters */
          }లేకపోతే ఒకవేళ( v<=0x7f ){
            z_బహిర్గతం[i_బహిర్గతం++] = (అక్షరం)v;
          }లేకపోతే ఒకవేళ( v<=0x7ff ){
            ప్రతిజ్ఞా( szEscape>=2 );
            z_బహిర్గతం[i_బహిర్గతం++] = (అక్షరం)(0xc0 | (v>>6));
            z_బహిర్గతం[i_బహిర్గతం++] = 0x80 | (v&0x3f);
          }లేకపోతే ఒకవేళ( v<0x10000 ){
            ప్రతిజ్ఞా( szEscape>=3 );
            z_బహిర్గతం[i_బహిర్గతం++] = 0xe0 | (v>>12);
            z_బహిర్గతం[i_బహిర్గతం++] = 0x80 | ((v>>6)&0x3f);
            z_బహిర్గతం[i_బహిర్గతం++] = 0x80 | (v&0x3f);
          }లేకపోతే ఒకవేళ( v==JSON_INVALID_CHAR ){
            /* Silently ignore illegal unicode */
          }లేకపోతే{
            ప్రతిజ్ఞా( szEscape>=4 );
            z_బహిర్గతం[i_బహిర్గతం++] = 0xf0 | (v>>18);
            z_బహిర్గతం[i_బహిర్గతం++] = 0x80 | ((v>>12)&0x3f);
            z_బహిర్గతం[i_బహిర్గతం++] = 0x80 | ((v>>6)&0x3f);
            z_బహిర్గతం[i_బహిర్గతం++] = 0x80 | (v&0x3f);
          }
          i_లోపల += szEscape - 1;
        }లేకపోతే{
          z_బహిర్గతం[i_బహిర్గతం++] = c;
        }
      } /* end for() */
      ప్రతిజ్ఞా( i_బహిర్గతం<=n_బహిర్గతం );
      z_బహిర్గతం[i_బహిర్గతం] = 0;
      sqlite3_ఫలితం_పాఠ్యం(pCtx, z_బహిర్గతం, i_బహిర్గతం, SQLITE_DYNAMIC);
      ఆపు;
    }
    స్థితి JSONB_ARRAY:
    స్థితి JSONB_OBJECT: {
      ఒకవేళ( e_బహుళకం==0 ){
        ఒకవేళ( (SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(pCtx)) & JSON_BLOB)!=0 ){
          e_బహుళకం = 2;
        }లేకపోతే{
          e_బహుళకం = 1;
        }
      }
      ఒకవేళ( e_బహుళకం==2 ){
        sqlite3_ఫలితం_blob(pCtx, &pParse->aBlob[i], sz+n, SQLITE_TRANSIENT);
      }లేకపోతే{
        json_Return_పాఠ్యం_Json_From_Blob(pCtx, &pParse->aBlob[i], sz+n);
      }
      ఆపు;
    }
    అప్రమేయ: {
      వెళ్ళు returnfromblob_malformed;
    }
  }
  తిరిగి;

returnfromblob_oom:
  sqlite3_ఫలితం_error_nomem(pCtx);
  తిరిగి;

returnfromblob_malformed:
  sqlite3_ఫలితం_error(pCtx, "malformed JSON", -1);
  తిరిగి;
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
స్థిర పూర్ణాంకం json_Function_Arg_గమ్యం_Blob(
  sqlite3_context *ctx,
  sqlite3_విలువ *pArg,
  JsonParse *pParse
){
  పూర్ణాంకం eType = sqlite3_విలువ_type(pArg);
  స్థిర u8 aNull[] = { 0x00 };
  memset(pParse, 0, పరిమాణం(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  ఎంపిక( eType ){
    అప్రమేయ: {
      pParse->aBlob = aNull;
      pParse->nBlob = 1;
      తిరిగి 0;
    }
    స్థితి SQLITE_BLOB: {
      ఒకవేళ( !jsonArgIsJsonb(pArg, pParse) ){
        sqlite3_ఫలితం_error(ctx, "JSON cannot hold BLOB values", -1);
        తిరిగి 1;
      }
      ఆపు;
    }
    స్థితి SQLITE_పాఠ్యం: {
      స్థిరరాశి అక్షరం *zJson = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(pArg);
      పూర్ణాంకం nJson = sqlite3_విలువ_బైట్లు(pArg);
      ఒకవేళ( zJson==0 ) తిరిగి 1;
      ఒకవేళ( sqlite3_విలువ_subtype(pArg)==JSON_SUBTYPE ){
        pParse->zJson = (అక్షరం*)zJson;
        pParse->nJson = nJson;
        ఒకవేళ( json_Convert_పాఠ్యం_గమ్యం_Blob(pParse, ctx) ){
          sqlite3_ఫలితం_error(ctx, "malformed JSON", -1);
          sqlite3DbFree(pParse->db, pParse->aBlob);
          memset(pParse, 0, పరిమాణం(pParse[0]));
          తిరిగి 1;
        }
      }లేకపోతే{
        json_Blob_Append_కణుపు(pParse, JSONB_TEXTRAW, nJson, zJson);
      }
      ఆపు;
    }
    స్థితి SQLITE_FLOAT: {
      ఒకవేళ( NEVER(sqlite3IsNaN(sqlite3_విలువ_double(pArg))) ){
        json_Blob_Append_కణుపు(pParse, JSONB_NULL, 0, 0);
      }లేకపోతే{
        పూర్ణాంకం n = sqlite3_విలువ_బైట్లు(pArg);
        స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(pArg);
        ఒకవేళ( z==0 ) తిరిగి 1;
        ఒకవేళ( z[0]=='I' ){
          json_Blob_Append_కణుపు(pParse, JSONB_FLOAT, 5, "9e999");
        }లేకపోతే ఒకవేళ( z[0]=='-' && z[1]=='I' ){
          json_Blob_Append_కణుపు(pParse, JSONB_FLOAT, 6, "-9e999");
        }లేకపోతే{
          json_Blob_Append_కణుపు(pParse, JSONB_FLOAT, n, z);
        }
      }
      ఆపు;
    }
    స్థితి SQLITE_INTEGER: {
      పూర్ణాంకం n = sqlite3_విలువ_బైట్లు(pArg);
      స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(pArg);
      ఒకవేళ( z==0 ) తిరిగి 1;
      json_Blob_Append_కణుపు(pParse, JSONB_INT, n, z);
      ఆపు;
    }
  }
  ఒకవేళ( pParse->oom ){
    sqlite3_ఫలితం_error_nomem(ctx);
    తిరిగి 1;
  }లేకపోతే{
    తిరిగి 0;
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
స్థిర అక్షరం *json_Bad_మార్గం_Error(
  sqlite3_context *ctx,     /* The function call containing the error */
  స్థిరరాశి అక్షరం *z_మార్గం,        /* The path with the problem */
  పూర్ణాంకం rc                    /* Maybe JSON_LOOKUP_NOTARRAY */
){
  అక్షరం *z_సందేశం;
  ఒకవేళ( rc==(పూర్ణాంకం)JSON_LOOKUP_NOTARRAY ){
    z_సందేశం = sqlite3_mprintf("not an array element: %Q", z_మార్గం);
  }లేకపోతే ఒకవేళ( rc==(పూర్ణాంకం)JSON_LOOKUP_ERROR ){
    z_సందేశం = sqlite3_mprintf("malformed JSON");
  }లేకపోతే ఒకవేళ( rc==(పూర్ణాంకం)JSON_LOOKUP_TOODEEP ){
    z_సందేశం = sqlite3_mprintf("JSON path too deep");
  }లేకపోతే{
    z_సందేశం = sqlite3_mprintf("bad JSON path: %Q", z_మార్గం);
  }
  ఒకవేళ( ctx==0 ) తిరిగి z_సందేశం;
  ఒకవేళ( z_సందేశం ){
    sqlite3_ఫలితం_error(ctx, z_సందేశం, -1);
    sqlite3_free(z_సందేశం);
  }లేకపోతే{
    sqlite3_ఫలితం_error_nomem(ctx);
  }
  తిరిగి 0;
}

/* argv[0] is a BLOB that seems likely to be a JSONB.  Subsequent
** arguments come in pairs where each pair contains a JSON path and
** content to insert or set at that patch.  Do the updates
** and return the result.
**
** The specific operation is determined by eEdit, which can be one
** of JEDIT_INS, JEDIT_REPL, JEDIT_SET, or JEDIT_AINS.
*/
స్థిర శూన్య json_చొప్పించు_Into_Blob(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv,
  పూర్ణాంకం eEdit                /* JEDIT_INS, JEDIT_REPL, JEDIT_SET, JEDIT_AINS */
){
  పూర్ణాంకం i;
  u32 rc = 0;
  స్థిరరాశి అక్షరం *z_మార్గం = 0;
  పూర్ణాంకం flgs;
  JsonParse *p;
  JsonParse ax;

  ప్రతిజ్ఞా( (argc&1)==1 );
  flgs = argc==1 ? 0 : JSON_EDITABLE;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  ఒకవేళ( p==0 ) తిరిగి;
  కొరకు(i=1; i<argc-1; i+=2){
    ఒకవేళ( sqlite3_విలువ_type(argv[i])==SQLITE_NULL ) కొనసాగు;
    z_మార్గం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
    ఒకవేళ( z_మార్గం==0 ){
      sqlite3_ఫలితం_error_nomem(ctx);
      jsonParseFree(p);
      తిరిగి;
    }
    ఒకవేళ( z_మార్గం[0]!='$' ) వెళ్ళు json_చొప్పించు_Into_Blob_patherror;
    ఒకవేళ( json_Function_Arg_గమ్యం_Blob(ctx, argv[i+1], &ax) ){
      jsonParseReset(&ax);
      jsonParseFree(p);
      తిరిగి;
    }
    ఒకవేళ( z_మార్గం[1]==0 ){
      ఒకవేళ( eEdit==JEDIT_REPL || eEdit==JEDIT_సమితి ){
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
   }లేకపోతే{
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->i_లోతు = 0;
      rc = json_Lookup_అడుగు(p, 0, z_మార్గం+1, 0);
    }
    jsonParseReset(&ax);
    ఒకవేళ( rc==JSON_LOOKUP_NOTFOUND ) కొనసాగు;
    ఒకవేళ( JSON_LOOKUP_ISERROR(rc) ) వెళ్ళు json_చొప్పించు_Into_Blob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  తిరిగి;

json_చొప్పించు_Into_Blob_patherror:
  jsonParseFree(p);
  json_Bad_మార్గం_Error(ctx, z_మార్గం, rc);
  తిరిగి;
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
స్థిర పూర్ణాంకం jsonArgIsJsonb(sqlite3_విలువ *pArg, JsonParse *p){
  u32 n, sz = 0;
  u8 c;
  ఒకవేళ( sqlite3_విలువ_type(pArg)!=SQLITE_BLOB ) తిరిగి 0;
  p->aBlob = (u8*)sqlite3_విలువ_blob(pArg);
  p->nBlob = (u32)sqlite3_విలువ_బైట్లు(pArg);
  ఒకవేళ( p->nBlob>0
   && ALWAYS(p->aBlob!=0)
   && ((c = p->aBlob[0]) & 0x0f)<=JSONB_OBJECT
   && (n = jsonb_Payload_కొలత(p, 0, &sz))>0
   && sz+n==p->nBlob
   && ((c & 0x0f)>JSONB_FALSE || sz==0)
   && (sz>7 
      || (c!=0x7b && c!=0x5b && !sqlite3Isdigit(c))
      || jsonbValidityCheck(p, 0, p->nBlob, 1)==0)
  ){
    తిరిగి 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  తిరిగి 0;
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
స్థిర JsonParse *jsonParseFuncArg(
  sqlite3_context *ctx,
  sqlite3_విలువ *pArg,
  u32 flgs
){
  పూర్ణాంకం eType;                   /* Datatype of pArg */
  JsonParse *p = 0;            /* Value to be returned */
  JsonParse *pFromCache = 0;   /* Value taken from cache */
  sqlite3 *db;                 /* The database connection */
  
  ప్రతిజ్ఞా( ctx!=0 );
  eType = sqlite3_విలువ_type(pArg);
  ఒకవేళ( eType==SQLITE_NULL ){
    తిరిగి 0;
  }
  ఒకవేళ( (flgs & JSON_NOCACHE)==0 ){
    pFromCache = json_Cache_శోధన(ctx, pArg);
  }
  ఒకవేళ( pFromCache ){
    pFromCache->nJPRef++;
    ఒకవేళ( (flgs & JSON_EDITABLE)==0 ){
      తిరిగి pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, పరిమాణం(*p));
  ఒకవేళ( p==0 ) వెళ్ళు json_pfa_oom;
  memset(p, 0, పరిమాణం(*p));
  p->db = db;
  p->nJPRef = 1;
  ఒకవేళ( pFromCache!=0 ){
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    ఒకవేళ( p->aBlob==0 ) వెళ్ళు json_pfa_oom;
    జ్ఞాపకం_నకలు(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    తిరిగి p;
  }
  ఒకవేళ( eType==SQLITE_BLOB ){
    ఒకవేళ( jsonArgIsJsonb(pArg,p) ){
      ఒకవేళ( (flgs & JSON_EDITABLE)!=0 && jsonBlobMakeEditable(p, 0)==0 ){
        వెళ్ళు json_pfa_oom;
      }
      తిరిగి p;
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
  p->zJson = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(pArg);
  p->nJson = sqlite3_విలువ_బైట్లు(pArg);
  ఒకవేళ( db->mallocFailed ) వెళ్ళు json_pfa_oom;
  ఒకవేళ( p->nJson==0 ) వెళ్ళు json_pfa_malformed;
  ప్రతిజ్ఞా( p->zJson!=0 );
  ఒకవేళ( json_Convert_పాఠ్యం_గమ్యం_Blob(p, (flgs & JSON_KEEPERROR) ? 0 : ctx) ){
    ఒకవేళ( flgs & JSON_KEEPERROR ){
      p->nErr = 1;
      తిరిగి p;
    }లేకపోతే{
      jsonParseFree(p);
      తిరిగి 0;
    }
  }లేకపోతే{
    పూర్ణాంకం isRCStr = sqlite3_విలువ_Is_యొక్క_Class(pArg, sqlite3RCStrUnref);
    పూర్ణాంకం rc;
    ఒకవేళ( !isRCStr ){
      అక్షరం *zNew = sqlite3RCStrNew( p->nJson );
      ఒకవేళ( zNew==0 ) వెళ్ళు json_pfa_oom;
      జ్ఞాపకం_నకలు(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    }లేకపోతే{
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = json_Cache_చొప్పించు(ctx, p);
    ఒకవేళ( rc==SQLITE_NOMEM ) వెళ్ళు json_pfa_oom;
    ఒకవేళ( flgs & JSON_EDITABLE ){
      pFromCache = p;
      p = 0;
      వెళ్ళు rebuild_from_cache;
    }
  }
  తిరిగి p;

json_pfa_malformed:
  ఒకవేళ( flgs & JSON_KEEPERROR ){
    p->nErr = 1;
    తిరిగి p;
  }లేకపోతే{
    jsonParseFree(p);
    sqlite3_ఫలితం_error(ctx, "malformed JSON", -1);
    తిరిగి 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_ఫలితం_error_nomem(ctx);
  తిరిగి 0;
}

/*
** Make the return value of a JSON function either the raw JSONB blob
** or make it JSON text, depending on whether the JSON_BLOB flag is
** set on the function.
*/
స్థిర శూన్య jsonReturnParse(
  sqlite3_context *ctx,
  JsonParse *p
){
  పూర్ణాంకం flgs;
  ఒకవేళ( p->oom ){
    sqlite3_ఫలితం_error_nomem(ctx);
    తిరిగి;
  }
  flgs = SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(ctx));
  ఒకవేళ( flgs & JSON_BLOB ){
    ఒకవేళ( p->nBlobAlloc>0 && !p->bReadOnly ){
      sqlite3_ఫలితం_blob(ctx, p->aBlob, p->nBlob, SQLITE_DYNAMIC);
      p->nBlobAlloc = 0;
    }లేకపోతే{
      sqlite3_ఫలితం_blob(ctx, p->aBlob, p->nBlob, SQLITE_TRANSIENT);
    }
  }లేకపోతే{
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    json_Translate_Blob_గమ్యం_పాఠ్యం(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
  }
}

/****************************************************************************
** SQL functions used for testing and debugging
****************************************************************************/

#ఒకవేళ SQLITE_DEBUG
/*
** Decode JSONB bytes in aBlob[] starting at iStart through but not 
** including iEnd.  Indent the
** content by nIndent spaces.
*/
స్థిర శూన్య jsonDebugPrintBlob(
  JsonParse *pParse, /* JSON content */
  u32 i_ప్రారంభం,        /* Start rendering here */
  u32 i_చివర,          /* Do not render this byte or any byte after this one */
  పూర్ణాంకం nIndent,       /* Indent by this many spaces */
  sqlite3_str *p_బహిర్గతం  /* Generate output into this sqlite3_str object */
){
  అయితే( i_ప్రారంభం<i_చివర ){
    u32 i, n, nn, sz = 0;
    పూర్ణాంకం showContent = 1;
    u8 x = pParse->aBlob[i_ప్రారంభం] & 0x0f;
    u32 savedNBlob = pParse->nBlob;
    sqlite3_str_appendf(p_బహిర్గతం, "%5d:%*s", i_ప్రారంభం, nIndent, "");
    ఒకవేళ( pParse->nBlobAlloc>pParse->nBlob ){
      pParse->nBlob = pParse->nBlobAlloc;
    }
    nn = n = jsonb_Payload_కొలత(pParse, i_ప్రారంభం, &sz);
    ఒకవేళ( nn==0 ) nn = 1;
    ఒకవేళ( sz>0 && x<JSONB_ARRAY ){
      nn += sz;
    }
    కొరకు(i=0; i<nn; i++){
      sqlite3_str_appendf(p_బహిర్గతం, " %02x", pParse->aBlob[i_ప్రారంభం+i]);
    }
    ఒకవేళ( n==0 ){
      sqlite3_str_appendf(p_బహిర్గతం, "   ERROR invalid node size\n");
      i_ప్రారంభం = n==0 ? i_ప్రారంభం+1 : i_చివర;
      కొనసాగు;
    }
    pParse->nBlob = savedNBlob;
    ఒకవేళ( i_ప్రారంభం+n+sz>i_చివర ){
      i_చివర = i_ప్రారంభం+n+sz;
      ఒకవేళ( i_చివర>pParse->nBlob ){
        ఒకవేళ( pParse->nBlobAlloc>0 && i_చివర>pParse->nBlobAlloc ){
          i_చివర = pParse->nBlobAlloc;
        }లేకపోతే{
          i_చివర = pParse->nBlob;
        }
      }
    }
    sqlite3_str_appendall(p_బహిర్గతం,"  <-- ");
    ఎంపిక( x ){
      స్థితి JSONB_NULL:     sqlite3_str_appendall(p_బహిర్గతం,"null"); ఆపు;
      స్థితి JSONB_TRUE:     sqlite3_str_appendall(p_బహిర్గతం,"true"); ఆపు;
      స్థితి JSONB_FALSE:    sqlite3_str_appendall(p_బహిర్గతం,"false"); ఆపు;
      స్థితి JSONB_INT:      sqlite3_str_appendall(p_బహిర్గతం,"int"); ఆపు;
      స్థితి JSONB_INT5:     sqlite3_str_appendall(p_బహిర్గతం,"int5"); ఆపు;
      స్థితి JSONB_FLOAT:    sqlite3_str_appendall(p_బహిర్గతం,"float"); ఆపు;
      స్థితి JSONB_FLOAT5:   sqlite3_str_appendall(p_బహిర్గతం,"float5"); ఆపు;
      స్థితి JSONB_పాఠ్యం:     sqlite3_str_appendall(p_బహిర్గతం,"text"); ఆపు;
      స్థితి JSONB_TEXTJ:    sqlite3_str_appendall(p_బహిర్గతం,"textj"); ఆపు;
      స్థితి JSONB_TEXT5:    sqlite3_str_appendall(p_బహిర్గతం,"text5"); ఆపు;
      స్థితి JSONB_TEXTRAW:  sqlite3_str_appendall(p_బహిర్గతం,"textraw"); ఆపు;
      స్థితి JSONB_ARRAY: {
        sqlite3_str_appendf(p_బహిర్గతం,"array, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_ప్రారంభం+n, i_ప్రారంభం+n+sz, nIndent+2, p_బహిర్గతం);
        showContent = 0;
        ఆపు;
      }
      స్థితి JSONB_OBJECT: {
        sqlite3_str_appendf(p_బహిర్గతం, "object, %u bytes\n", sz);
        jsonDebugPrintBlob(pParse, i_ప్రారంభం+n, i_ప్రారంభం+n+sz, nIndent+2, p_బహిర్గతం);
        showContent = 0;
        ఆపు;
      }
      అప్రమేయ: {
        sqlite3_str_appendall(p_బహిర్గతం, "ERROR: unknown node type\n");
        showContent = 0;
        ఆపు;
      }
    }
    ఒకవేళ( showContent ){
      ఒకవేళ( sz==0 && x<=JSONB_FALSE ){
        sqlite3_str_append(p_బహిర్గతం, "\n", 1);
      }లేకపోతే{
        u32 j;
        sqlite3_str_appendall(p_బహిర్గతం, ": \"");
        కొరకు(j=i_ప్రారంభం+n; j<i_ప్రారంభం+n+sz; j++){
          u8 c = pParse->aBlob[j];
          ఒకవేళ( c<0x20 || c>=0x7f ) c = '.';
          sqlite3_str_append(p_బహిర్గతం, (అక్షరం*)&c, 1);
        }
        sqlite3_str_append(p_బహిర్గతం, "\"\n", 2);
      }
    }
    i_ప్రారంభం += n + sz;
  }
}
స్థిర శూన్య jsonShowParse(JsonParse *pParse){
  sqlite3_str బహిర్గతం;
  అక్షరం zBuf[1000];
  ఒకవేళ( pParse==0 ){
    రాయి("NULL pointer\n");
    తిరిగి;
  }లేకపోతే{
    రాయి("nBlobAlloc = %u\n", pParse->nBlobAlloc);
    రాయి("nBlob = %u\n", pParse->nBlob);
    రాయి("delta = %d\n", pParse->delta);
    ఒకవేళ( pParse->nBlob==0 ) తిరిగి;
    రాయి("content (bytes 0..%u):\n", pParse->nBlob-1);
  }
  sqlite3StrAccumInit(&బహిర్గతం, 0, zBuf, పరిమాణం(zBuf), 1000000);
  jsonDebugPrintBlob(pParse, 0, pParse->nBlob, 0, &బహిర్గతం);
  రాయి("%s", sqlite3_str_విలువ(&బహిర్గతం));
  sqlite3_str_reset(&బహిర్గతం);
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

#నిర్వచితమైతే SQLITE_DEBUG
/*
** SQL function:   json_parse(JSON)
**
** Parse JSON using jsonParseFuncArg().  Return text that is a
** human-readable dump of the binary JSONB for the input parameter.
*/
స్థిర శూన్య jsonParseFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonParse *p;        /* The parse */
  sqlite3_str బహిర్గతం;

  ప్రతిజ్ఞా( argc>=1 );
  sqlite3StrAccumInit(&బహిర్గతం, 0, 0, 0, 1000000);
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ఒకవేళ( p==0 ) తిరిగి;
  ఒకవేళ( argc==1 ){
    jsonDebugPrintBlob(p, 0, p->nBlob, 0, &బహిర్గతం);
    sqlite3_ఫలితం_text64(ctx,బహిర్గతం.z_పాఠ్యం,బహిర్గతం.nChar,SQLITE_TRANSIENT,SQLITE_UTF8);
  }లేకపోతే{
    jsonShowParse(p);
  }
  jsonParseFree(p);
  sqlite3_str_reset(&బహిర్గతం);
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

/****************************************************************************
** Scalar SQL function implementations
****************************************************************************/

/*
** Implementation of the json_quote(VALUE) function.  Return a JSON value
** corresponding to the SQL value input.  Mostly this means putting
** double-quotes around strings and returning the unquoted string "null"
** when given a NULL input.
*/
స్థిర శూన్య jsonQuoteFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonString jx;
  వాడని_PARAMETER(argc);

  jsonStringInit(&jx, ctx);
  json_Append_Sql_విలువ(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
}

/*
** Implementation of the json_array(VALUE,...) function.  Return a JSON
** array that contains all values given in arguments.  Or if any argument
** is a BLOB, throw an error.
*/
స్థిర శూన్య jsonArrayFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  కొరకు(i=0; i<argc; i++){
    jsonAppendSeparator(&jx);
    json_Append_Sql_విలువ(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
}

/*
** json_array_length(JSON)
** json_array_length(JSON, PATH)
**
** Return the number of elements in the top-level JSON array.
** Return 0 if the input is not a well-formed JSON array.
*/
స్థిర శూన్య json_Array_నిడివి_Func(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonParse *p;          /* The parse */
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ఒకవేళ( p==0 ) తిరిగి;
  ఒకవేళ( argc==2 ){
    స్థిరరాశి అక్షరం *z_మార్గం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);
    ఒకవేళ( z_మార్గం==0 ){
      jsonParseFree(p);
      తిరిగి;
    }
    i = json_Lookup_అడుగు(p, 0, z_మార్గం[0]=='$' ? z_మార్గం+1 : "@", 0);
    ఒకవేళ( JSON_LOOKUP_ISERROR(i) ){
      ఒకవేళ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }లేకపోతే{
        json_Bad_మార్గం_Error(ctx, z_మార్గం, i);
      }
      eErr = 1;
      i = 0;
    }
  }లేకపోతే{
    i = 0;
  }
  ఒకవేళ( (p->aBlob[i] & 0x0f)==JSONB_ARRAY ){
    cnt = jsonb_Array_లెక్క(p, i);
  }
  ఒకవేళ( !eErr ) sqlite3_ఫలితం_int64(ctx, cnt);
  jsonParseFree(p);
}

/* True if the string is all alphanumerics and underscores */
స్థిర పూర్ణాంకం jsonAllAlphanum(స్థిరరాశి అక్షరం *z, పూర్ణాంకం n){
  పూర్ణాంకం i;
  కొరకు(i=0; i<n && (sqlite3Isalnum(z[i]) || z[i]=='_'); i++){}
  తిరిగి i==n;
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
స్థిర శూన్య json_వెలికితీయి_Func(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonParse *p = 0;      /* The parse */
  పూర్ణాంకం flags;             /* Flags associated with the function */
  పూర్ణాంకం i;                 /* Loop counter */
  JsonString jx;         /* String for array result */

  ఒకవేళ( argc<2 ) తిరిగి;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  ఒకవేళ( p==0 ) తిరిగి;
  flags = SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(ctx));
  jsonStringInit(&jx, ctx);
  ఒకవేళ( argc>2 ){
    jsonAppendChar(&jx, '[');
  }
  కొరకు(i=1; i<argc; i++){
    /* With a single PATH argument */
    స్థిరరాశి అక్షరం *z_మార్గం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
    పూర్ణాంకం n_మార్గం;
    u32 j;
    ఒకవేళ( z_మార్గం==0 ) వెళ్ళు json_వెలికితీయి_error;
    n_మార్గం = sqlite3Strlen30(z_మార్గం);
    ఒకవేళ( z_మార్గం[0]=='$' ){
      j = json_Lookup_అడుగు(p, 0, z_మార్గం+1, 0);
    }లేకపోతే ఒకవేళ( (flags & JSON_ABPATH) ){
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
      ఒకవేళ( sqlite3_విలువ_type(argv[i])==SQLITE_INTEGER ){
        jsonAppendRawNZ(&jx, "[", 1);
        ఒకవేళ( z_మార్గం[0]=='-' ) jsonAppendRawNZ(&jx,"#",1);
        jsonAppendRaw(&jx, z_మార్గం, n_మార్గం);
        jsonAppendRawNZ(&jx, "]", 2);
      }లేకపోతే ఒకవేళ( jsonAllAlphanum(z_మార్గం, n_మార్గం) ){
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, z_మార్గం, n_మార్గం);
      }లేకపోతే ఒకవేళ( z_మార్గం[0]=='[' && n_మార్గం>=3 && z_మార్గం[n_మార్గం-1]==']' ){
        jsonAppendRaw(&jx, z_మార్గం, n_మార్గం);
      }లేకపోతే{
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, z_మార్గం, n_మార్గం);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = json_Lookup_అడుగు(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    }లేకపోతే{
      json_Bad_మార్గం_Error(ctx, z_మార్గం, 0);
      వెళ్ళు json_వెలికితీయి_error;
    }
    ఒకవేళ( j<p->nBlob ){
      ఒకవేళ( argc==2 ){
        ఒకవేళ( flags & JSON_JSON ){
          jsonStringInit(&jx, ctx);
          json_Translate_Blob_గమ్యం_పాఠ్యం(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);
          ప్రతిజ్ఞా( (flags & JSON_BLOB)==0 );
          sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
        }లేకపోతే{
          jsonReturnFromBlob(p, j, ctx, 0);
          ఒకవేళ( (flags & (JSON_SQL|JSON_BLOB))==0
           && (p->aBlob[j]&0x0f)>=JSONB_ARRAY
          ){
            sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
          }
        }
      }లేకపోతే{
        jsonAppendSeparator(&jx);
        json_Translate_Blob_గమ్యం_పాఠ్యం(p, j, &jx);
      }
    }లేకపోతే ఒకవేళ( j==JSON_LOOKUP_NOTFOUND ){
      ఒకవేళ( argc==2 ){
        వెళ్ళు json_వెలికితీయి_error;  /* Return NULL if not found */
      }లేకపోతే{
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    }లేకపోతే{
      json_Bad_మార్గం_Error(ctx, z_మార్గం, j);
      వెళ్ళు json_వెలికితీయి_error;
    }
  }
  ఒకవేళ( argc>2 ){
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    ఒకవేళ( (flags & JSON_BLOB)==0 ){
      sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
    }
  }
json_వెలికితీయి_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  తిరిగి;
}

/*
** Return codes for jsonMergePatch()
*/
#నిర్వచించు JSON_విలీనం_OK          0     /* Success */
#నిర్వచించు JSON_విలీనం_BADTARGET   1     /* Malformed TARGET blob */
#నిర్వచించు JSON_విలీనం_BADPATCH    2     /* Malformed PATCH blob */
#నిర్వచించు JSON_విలీనం_OOM         3     /* Out-of-memory condition */
#నిర్వచించు JSON_విలీనం_TOODEEP     4     /* Nested too deep */

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
స్థిర పూర్ణాంకం json_విలీనం_Patch(
  JsonParse *p_లక్ష్యం,      /* The JSON parser that contains the TARGET */
  u32 i_లక్ష్యం,             /* Index of TARGET in pTarget->aBlob[] */
  స్థిరరాశి JsonParse *pPatch, /* The PATCH */
  u32 iPatch,              /* Index of PATCH in pPatch->aBlob[] */
  u32 i_లోతు               /* Nesting depth */
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

  ప్రతిజ్ఞా( i_లక్ష్యం>=0 && i_లక్ష్యం<p_లక్ష్యం->nBlob );
  ప్రతిజ్ఞా( iPatch>=0 && iPatch<pPatch->nBlob );
  x = pPatch->aBlob[iPatch] & 0x0f;
  ఒకవేళ( x!=JSONB_OBJECT ){  /* Algorithm line 02 */
    u32 szPatch;        /* Total size of the patch, header+payload */
    u32 sz_లక్ష్యం;       /* Total size of the target, header+payload */
    n = jsonb_Payload_కొలత(pPatch, iPatch, &sz);
    szPatch = n+sz;
    sz = 0;
    n = jsonb_Payload_కొలత(p_లక్ష్యం, i_లక్ష్యం, &sz);
    sz_లక్ష్యం = n+sz;
    jsonBlobEdit(p_లక్ష్యం, i_లక్ష్యం, sz_లక్ష్యం, pPatch->aBlob+iPatch, szPatch);
    తిరిగి p_లక్ష్యం->oom ? JSON_విలీనం_OOM : JSON_విలీనం_OK;  /* Line 03 */
  }
  x = p_లక్ష్యం->aBlob[i_లక్ష్యం] & 0x0f;
  ఒకవేళ( x!=JSONB_OBJECT ){  /* Algorithm line 05 */
    n = jsonb_Payload_కొలత(p_లక్ష్యం, i_లక్ష్యం, &sz);
    jsonBlobEdit(p_లక్ష్యం, i_లక్ష్యం+n, sz, 0, 0);
    x = p_లక్ష్యం->aBlob[i_లక్ష్యం];
    p_లక్ష్యం->aBlob[i_లక్ష్యం] = (x & 0xf0) | JSONB_OBJECT;
  }
  n = jsonb_Payload_కొలత(pPatch, iPatch, &sz);
  ఒకవేళ( NEVER(n==0) ) తిరిగి JSON_విలీనం_BADPATCH;
  iPCursor = iPatch+n;
  iPEnd = iPCursor+sz;
  n = jsonb_Payload_కొలత(p_లక్ష్యం, i_లక్ష్యం, &sz);
  ఒకవేళ( NEVER(n==0) ) తిరిగి JSON_విలీనం_BADTARGET;
  iTStart = i_లక్ష్యం+n;
  iTEndBE = iTStart+sz;

  అయితే( iPCursor<iPEnd ){  /* Algorithm line 07 */
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    ఒకవేళ( ePLabel<JSONB_పాఠ్యం || ePLabel>JSONB_TEXTRAW ){
      తిరిగి JSON_విలీనం_BADPATCH;
    }
    nPLabel = jsonb_Payload_కొలత(pPatch, iPCursor, &szPLabel);
    ఒకవేళ( nPLabel==0 ) తిరిగి JSON_విలీనం_BADPATCH;
    iPValue = iPCursor + nPLabel + szPLabel;
    ఒకవేళ( iPValue>=iPEnd ) తిరిగి JSON_విలీనం_BADPATCH;
    nPValue = jsonb_Payload_కొలత(pPatch, iPValue, &szPValue);
    ఒకవేళ( nPValue==0 ) తిరిగి JSON_విలీనం_BADPATCH;
    iPCursor = iPValue + nPValue + szPValue;
    ఒకవేళ( iPCursor>iPEnd ) తిరిగి JSON_విలీనం_BADPATCH;

    iTCursor = iTStart;
    iTEnd = iTEndBE + p_లక్ష్యం->delta;
    అయితే( iTCursor<iTEnd ){
      పూర్ణాంకం isEqual;   /* true if the patch and target labels match */
      iTLabel = iTCursor;
      eTLabel = p_లక్ష్యం->aBlob[iTCursor] & 0x0f;
      ఒకవేళ( eTLabel<JSONB_పాఠ్యం || eTLabel>JSONB_TEXTRAW ){
        తిరిగి JSON_విలీనం_BADTARGET;
      }
      nTLabel = jsonb_Payload_కొలత(p_లక్ష్యం, iTCursor, &szTLabel);
      ఒకవేళ( nTLabel==0 ) తిరిగి JSON_విలీనం_BADTARGET;
      iTValue = iTLabel + nTLabel + szTLabel;
      ఒకవేళ( iTValue>=iTEnd ) తిరిగి JSON_విలీనం_BADTARGET;
      nTValue = jsonb_Payload_కొలత(p_లక్ష్యం, iTValue, &szTValue);
      ఒకవేళ( nTValue==0 ) తిరిగి JSON_విలీనం_BADTARGET;
      ఒకవేళ( iTValue + nTValue + szTValue > iTEnd ) తిరిగి JSON_విలీనం_BADTARGET;
      isEqual = jsonLabelCompare(
                   (స్థిరరాశి అక్షరం*)&pPatch->aBlob[iPLabel+nPLabel],
                   szPLabel,
                   (ePLabel==JSONB_పాఠ్యం || ePLabel==JSONB_TEXTRAW),
                   (స్థిరరాశి అక్షరం*)&p_లక్ష్యం->aBlob[iTLabel+nTLabel],
                   szTLabel,
                   (eTLabel==JSONB_పాఠ్యం || eTLabel==JSONB_TEXTRAW));
      ఒకవేళ( isEqual ) ఆపు;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    ఒకవేళ( iTCursor<iTEnd ){
      /* A match was found.  Algorithm line 08 */
      ఒకవేళ( x==0 ){
        /* Patch value is NULL.  Algorithm line 09 */
        jsonBlobEdit(p_లక్ష్యం, iTLabel, nTLabel+szTLabel+nTValue+szTValue, 0,0);
        /*  vvvvvv----- No OOM on a delete-only edit */
        ఒకవేళ( NEVER(p_లక్ష్యం->oom) ) తిరిగి JSON_విలీనం_OOM;
      }లేకపోతే{
        /* Algorithm line 12 */
        పూర్ణాంకం rc, savedDelta = p_లక్ష్యం->delta;
        p_లక్ష్యం->delta = 0;
        ఒకవేళ( i_లోతు>=JSON_MAX_లోతు ) తిరిగి JSON_విలీనం_TOODEEP;
        rc = json_విలీనం_Patch(p_లక్ష్యం, iTValue, pPatch, iPValue, i_లోతు+1);
        ఒకవేళ( rc ) తిరిగి rc;
        p_లక్ష్యం->delta += savedDelta;
      }        
    }లేకపోతే ఒకవేళ( x>0 ){  /* Algorithm line 13 */
      /* No match and patch value is not NULL */
      u32 szNew = szPLabel+nPLabel;
      ఒకవేళ( (pPatch->aBlob[iPValue] & 0x0f)!=JSONB_OBJECT ){  /* Line 14 */
        jsonBlobEdit(p_లక్ష్యం, iTEnd, 0, 0, szPValue+nPValue+szNew);
        ఒకవేళ( p_లక్ష్యం->oom ) తిరిగి JSON_విలీనం_OOM;
        జ్ఞాపకం_నకలు(&p_లక్ష్యం->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        జ్ఞాపకం_నకలు(&p_లక్ష్యం->aBlob[iTEnd+szNew], 
               &pPatch->aBlob[iPValue], szPValue+nPValue);
      }లేకపోతే{
        పూర్ణాంకం rc, savedDelta;
        jsonBlobEdit(p_లక్ష్యం, iTEnd, 0, 0, szNew+1);
        ఒకవేళ( p_లక్ష్యం->oom ) తిరిగి JSON_విలీనం_OOM;
        జ్ఞాపకం_నకలు(&p_లక్ష్యం->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        p_లక్ష్యం->aBlob[iTEnd+szNew] = 0x00;
        savedDelta = p_లక్ష్యం->delta;
        p_లక్ష్యం->delta = 0;
        ఒకవేళ( i_లోతు>=JSON_MAX_లోతు ) తిరిగి JSON_విలీనం_TOODEEP;
        rc = json_విలీనం_Patch(p_లక్ష్యం, iTEnd+szNew,pPatch,iPValue,i_లోతు+1);
        ఒకవేళ( rc ) తిరిగి rc;
        p_లక్ష్యం->delta += savedDelta;
      }
    }
  }
  ఒకవేళ( p_లక్ష్యం->delta ) json_After_Edit_కొలత_Adjust(p_లక్ష్యం, i_లక్ష్యం);
  తిరిగి p_లక్ష్యం->oom ? JSON_విలీనం_OOM : JSON_విలీనం_OK;
}


/*
** Implementation of the json_mergepatch(JSON1,JSON2) function.  Return a JSON
** object that is the result of running the RFC 7396 MergePatch() algorithm
** on the two arguments.
*/
స్థిర శూన్య jsonPatchFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonParse *p_లక్ష్యం;    /* The TARGET */
  JsonParse *pPatch;     /* The PATCH */
  పూర్ణాంకం rc;                /* Result code */

  వాడని_PARAMETER(argc);
  ప్రతిజ్ఞా( argc==2 );
  p_లక్ష్యం = jsonParseFuncArg(ctx, argv[0], JSON_EDITABLE);
  ఒకవేళ( p_లక్ష్యం==0 ) తిరిగి;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  ఒకవేళ( pPatch ){
    rc = json_విలీనం_Patch(p_లక్ష్యం, 0, pPatch, 0, 0);
    ఒకవేళ( rc==JSON_విలీనం_OK ){
      jsonReturnParse(ctx, p_లక్ష్యం);
    }లేకపోతే ఒకవేళ( rc==JSON_విలీనం_OOM ){
      sqlite3_ఫలితం_error_nomem(ctx);
    }లేకపోతే ఒకవేళ( rc==JSON_విలీనం_TOODEEP ){
      sqlite3_ఫలితం_error(ctx, "JSON nested too deep", -1);
    }లేకపోతే{
      sqlite3_ఫలితం_error(ctx, "malformed JSON", -1);
    }
    jsonParseFree(pPatch);
  }
  jsonParseFree(p_లక్ష్యం);
}


/*
** Implementation of the json_object(NAME,VALUE,...) function.  Return a JSON
** object that contains all name/value given in arguments.  Or if any name
** is not a string or if any value is a BLOB, throw an error.
*/
స్థిర శూన్య jsonObjectFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం i;
  JsonString jx;
  స్థిరరాశి అక్షరం *z;
  u32 n;

  ఒకవేళ( argc&1 ){
    sqlite3_ఫలితం_error(ctx, "json_object() requires an even number "
                                  "of arguments", -1);
    తిరిగి;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  కొరకు(i=0; i<argc; i+=2){
    ఒకవేళ( sqlite3_విలువ_type(argv[i])!=SQLITE_పాఠ్యం ){
      sqlite3_ఫలితం_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      తిరిగి;
    }
    jsonAppendSeparator(&jx);
    z = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
    n = sqlite3_విలువ_బైట్లు(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    json_Append_Sql_విలువ(&jx, argv[i+1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
}


/*
** json_remove(JSON, PATH, ...)
**
** Remove the named elements from JSON and return the result.  malformed
** JSON or PATH arguments result in an error.
*/
స్థిర శూన్య jsonRemoveFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonParse *p;          /* The parse */
  స్థిరరాశి అక్షరం *z_మార్గం = 0; /* Path of element to be removed */
  పూర్ణాంకం i;                 /* Loop counter */
  u32 rc;                /* Subroutine return code */

  ఒకవేళ( argc<1 ) తిరిగి;
  p = jsonParseFuncArg(ctx, argv[0], argc>1 ? JSON_EDITABLE : 0);
  ఒకవేళ( p==0 ) తిరిగి;
  కొరకు(i=1; i<argc; i++){
    z_మార్గం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
    ఒకవేళ( z_మార్గం==0 ){
      వెళ్ళు json_remove_పూర్తి;
    }
    ఒకవేళ( z_మార్గం[0]!='$' ){
      వెళ్ళు json_remove_patherror;
    }
    ఒకవేళ( z_మార్గం[1]==0 ){
      /* json_remove(j,'$') returns NULL */
      వెళ్ళు json_remove_పూర్తి;
    }
    p->eEdit = JEDIT_DEL;
    p->delta = 0;
    rc = json_Lookup_అడుగు(p, 0, z_మార్గం+1, 0);
    ఒకవేళ( JSON_LOOKUP_ISERROR(rc) ){
      ఒకవేళ( rc==JSON_LOOKUP_NOTFOUND ){
        కొనసాగు;  /* No-op */
      }లేకపోతే{
        json_Bad_మార్గం_Error(ctx, z_మార్గం, rc);
      }
      వెళ్ళు json_remove_పూర్తి;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  తిరిగి;

json_remove_patherror:
  json_Bad_మార్గం_Error(ctx, z_మార్గం, 0);

json_remove_పూర్తి:
  jsonParseFree(p);
  తిరిగి;
}

/*
** json_replace(JSON, PATH, VALUE, ...)
**
** Replace the value at PATH with VALUE.  If PATH does not already exist,
** this routine is a no-op.  If JSON or PATH is malformed, throw an error.
*/
స్థిర శూన్య jsonReplaceFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  ఒకవేళ( argc<1 ) తిరిగి;
  ఒకవేళ( (argc&1)==0 ) {
    json_Wrong_అంకె_వాదనలు(ctx, "replace");
    తిరిగి;
  }
  json_చొప్పించు_Into_Blob(ctx, argc, argv, JEDIT_REPL);
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
స్థిర శూన్య json_సమితి_Func(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం flags = SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(ctx));
  పూర్ణాంకం eInsType = JSON_చొప్పించు_TYPE(flags);
  స్థిర స్థిరరాశి అక్షరం *azInsType[] = { "insert", "set", "array_insert" };
  స్థిర స్థిరరాశి u8 aEditType[] = { JEDIT_INS, JEDIT_సమితి, JEDIT_AINS };

  ఒకవేళ( argc<1 ) తిరిగి;
  ప్రతిజ్ఞా( eInsType>=0 && eInsType<=2 );
  ఒకవేళ( (argc&1)==0 ) {
    json_Wrong_అంకె_వాదనలు(ctx, azInsType[eInsType]);
    తిరిగి;
  }
  json_చొప్పించు_Into_Blob(ctx, argc, argv, aEditType[eInsType]);
}

/*
** json_type(JSON)
** json_type(JSON, PATH)
**
** Return the top-level "type" of a JSON string.  json_type() raises an
** error if either the JSON or PATH inputs are not well-formed.
*/
స్థిర శూన్య jsonTypeFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonParse *p;          /* The parse */
  స్థిరరాశి అక్షరం *z_మార్గం = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  ఒకవేళ( p==0 ) తిరిగి;
  ఒకవేళ( argc==2 ){
    z_మార్గం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);
    ఒకవేళ( z_మార్గం==0 ) వెళ్ళు json_type_పూర్తి;
    ఒకవేళ( z_మార్గం[0]!='$' ){
      json_Bad_మార్గం_Error(ctx, z_మార్గం, 0);
      వెళ్ళు json_type_పూర్తి;
    }
    i = json_Lookup_అడుగు(p, 0, z_మార్గం+1, 0);
    ఒకవేళ( JSON_LOOKUP_ISERROR(i) ){
      ఒకవేళ( i==JSON_LOOKUP_NOTFOUND ){
        /* no-op */
      }లేకపోతే{
        json_Bad_మార్గం_Error(ctx, z_మార్గం, i);
      }
      వెళ్ళు json_type_పూర్తి;
    }
  }లేకపోతే{
    i = 0;
  }
  sqlite3_ఫలితం_పాఠ్యం(ctx, jsonbType[p->aBlob[i]&0x0f], -1, SQLITE_STATIC);
json_type_పూర్తి:
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
స్థిర శూన్య jsonPrettyFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonString s;          /* The output string */
  JsonPretty x;          /* Pretty printing context */

  memset(&x, 0, పరిమాణం(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  ఒకవేళ( x.pParse==0 ) తిరిగి;
  x.p_బహిర్గతం = &s;
  jsonStringInit(&s, ctx);
  ఒకవేళ( argc==1 || (x.zIndent = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]))==0 ){
    x.zIndent = "    ";
    x.szIndent = 4;
  }లేకపోతే{
    x.szIndent = (u32)అక్షరశ్రేణి_పొడవు(x.zIndent);
  }
  json_Translate_Blob_గమ్యం_Pretty_పాఠ్యం(&x, 0);
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
స్థిర శూన్య jsonValidFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonParse *p;          /* The parse */
  u8 flags = 1;
  u8 res = 0;
  ఒకవేళ( argc==2 ){
    i64 f = sqlite3_విలువ_int64(argv[1]);
    ఒకవేళ( f<1 || f>15 ){
      sqlite3_ఫలితం_error(ctx, "FLAGS parameter to json_valid() must be"
                                " between 1 and 15", -1);
      తిరిగి;
    }
    flags = f & 0x0f;
  }
  ఎంపిక( sqlite3_విలువ_type(argv[0]) ){
    స్థితి SQLITE_NULL: {
#నిర్వచితమైతే SQLITE_LEGACY_JSON_VALID
      /* Incorrect legacy behavior was to return FALSE for a NULL input */
      sqlite3_ఫలితం_int(ctx, 0);
#అయితే_ముగింపు
      తిరిగి;
    }
    స్థితి SQLITE_BLOB: {
      JsonParse py;
      memset(&py, 0, పరిమాణం(py));
      ఒకవేళ( jsonArgIsJsonb(argv[0], &py) ){
        ఒకవేళ( flags & 0x04 ){
          /* Superficial checking only - accomplished by the
          ** jsonArgIsJsonb() call above. */
          res = 1;
        }లేకపోతే ఒకవేళ( flags & 0x08 ){
          /* Strict checking.  Check by translating BLOB->TEXT->BLOB.  If
          ** no errors occur, call that a "strict check". */
          res = 0==jsonbValidityCheck(&py, 0, py.nBlob, 1);
        }
        ఆపు;
      }
      /* Fall through into interpreting the input as text.  See note
      ** above at tag-20240123-a. */
      /* no break */ deliberate_fall_through
    }
    అప్రమేయ: {
      JsonParse px;
      ఒకవేళ( (flags & 0x3)==0 ) ఆపు;
      memset(&px, 0, పరిమాణం(px));
     
      p = jsonParseFuncArg(ctx, argv[0], JSON_KEEPERROR|JSON_NOCACHE);
      ఒకవేళ( p ){
        ఒకవేళ( p->oom ){
          sqlite3_ఫలితం_error_nomem(ctx);
        }లేకపోతే ఒకవేళ( p->nErr ){
          /* no-op */
        }లేకపోతే ఒకవేళ( (flags & 0x02)!=0 || p->hasNonstd==0 ){
          res = 1;
        }
        jsonParseFree(p);
      }లేకపోతే{
        sqlite3_ఫలితం_error_nomem(ctx);
      }
      ఆపు;
    }
  }
  sqlite3_ఫలితం_int(ctx, res);
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
స్థిర శూన్య jsonErrorFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  i64 iErrPos = 0;       /* Error position to be returned */
  JsonParse s;

  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  memset(&s, 0, పరిమాణం(s));
  s.db = sqlite3_context_db_handle(ctx);
  ఒకవేళ( jsonArgIsJsonb(argv[0], &s) ){
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  }లేకపోతే{
    s.zJson = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    ఒకవేళ( s.zJson==0 ) తిరిగి;  /* NULL input or OOM */
    s.nJson = sqlite3_విలువ_బైట్లు(argv[0]);
    ఒకవేళ( json_Convert_పాఠ్యం_గమ్యం_Blob(&s,0) ){
      ఒకవేళ( s.oom ){
        iErrPos = -1;
      }లేకపోతే{
        /* Convert byte-offset s.iErr into a character offset */
        u32 k;
        ప్రతిజ్ఞా( s.zJson!=0 );  /* Because s.oom is false */
        కొరకు(k=0; k<s.iErr && ALWAYS(s.zJson[k]); k++){
          ఒకవేళ( (s.zJson[k] & 0xc0)!=0x80 ) iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  ఒకవేళ( iErrPos<0 ){
    sqlite3_ఫలితం_error_nomem(ctx);
  }లేకపోతే{
    sqlite3_ఫలితం_int64(ctx, iErrPos);
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
స్థిర శూన్య json_Array_అడుగు(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonString *pStr;
  వాడని_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, పరిమాణం(*pStr));
  ఒకవేళ( pStr ){
    ఒకవేళ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    }లేకపోతే ఒకవేళ( pStr->n_వాడిన>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    json_Append_Sql_విలువ(pStr, argv[0]);
  }
}
స్థిర శూన్య jsonArrayCompute(sqlite3_context *ctx, పూర్ణాంకం isFinal){
  JsonString *pStr;
  పూర్ణాంకం flags = SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ఒకవేళ( pStr ){
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    ఒకవేళ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      తిరిగి;
    }లేకపోతే ఒకవేళ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ఒకవేళ( isFinal ){
        ఒకవేళ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }లేకపోతే{
        jsonStringTrimOneChar(pStr);
      }
      తిరిగి;
    }లేకపోతే ఒకవేళ( isFinal ){
      sqlite3_ఫలితం_పాఠ్యం(ctx, pStr->zBuf, (పూర్ణాంకం)pStr->n_వాడిన,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                              sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }లేకపోతే{
      sqlite3_ఫలితం_పాఠ్యం(ctx, pStr->zBuf, (పూర్ణాంకం)pStr->n_వాడిన, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pStr);
    }
  }లేకపోతే ఒకవేళ( flags & JSON_BLOB ){
    స్థిర స్థిరరాశి u8 ఖాళీలు_Array = 0x0b;
    sqlite3_ఫలితం_blob(ctx, &ఖాళీలు_Array, 1, SQLITE_STATIC);
  }లేకపోతే{
    sqlite3_ఫలితం_పాఠ్యం(ctx, "[]", 2, SQLITE_STATIC);
  }
  sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
}
స్థిర శూన్య json_Array_విలువ(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 0);
}
స్థిర శూన్య jsonArrayFinal(sqlite3_context *ctx){
  jsonArrayCompute(ctx, 1);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
/*
** This method works for both json_group_array() and json_group_object().
** It works by removing the first element of the group by searching forward
** to the first comma (",") that is not within a string and deleting all
** text through that comma.
*/
స్థిర శూన్య json_సమూహం_Inverse(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  గుర్తులేని పూర్ణాంకం i;
  పూర్ణాంకం లోపల_Str = 0;
  పూర్ణాంకం nNest = 0;
  అక్షరం *z;
  అక్షరం c;
  JsonString *pStr;
  వాడని_PARAMETER(argc);
  వాడని_PARAMETER(argv);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  /* pStr is always non-NULL since jsonArrayStep() or jsonObjectStep() will
  ** always have been called to initialize it */
  ఒకవేళ( NEVER(!pStr) ) తిరిగి;
  z = pStr->zBuf;
  కొరకు(i=1; i<pStr->n_వాడిన && ((c = z[i])!=',' || లోపల_Str || nNest); i++){
    ఒకవేళ( c=='"' ){
      లోపల_Str = !లోపల_Str;
    }లేకపోతే ఒకవేళ( c=='\\' ){
      i++;
    }లేకపోతే ఒకవేళ( !లోపల_Str ){
      ఒకవేళ( c=='{' || c=='[' ) nNest++;
      ఒకవేళ( c=='}' || c==']' ) nNest--;
    }
  }
  ఒకవేళ( i<pStr->n_వాడిన ){
    pStr->n_వాడిన -= i;
    memmove(&z[1], &z[i+1], (size_t)pStr->n_వాడిన-1);
    z[pStr->n_వాడిన] = 0;
  }లేకపోతే{
    pStr->n_వాడిన = 1;
  }
}
#లేకపోతే
# define json_సమూహం_Inverse 0
#అయితే_ముగింపు


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
స్థిర శూన్య json_Object_అడుగు(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  JsonString *pStr;
  స్థిరరాశి అక్షరం *z;
  u32 n;
  వాడని_PARAMETER(argc);
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, పరిమాణం(*pStr));
  ఒకవేళ( pStr ){
    z = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    n = sqlite3Strlen30(z);
    ఒకవేళ( pStr->zBuf==0 ){
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    }లేకపోతే ఒకవేళ( pStr->n_వాడిన>1 ){
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    ఒకవేళ( z!=0 ){
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      json_Append_Sql_విలువ(pStr, argv[1]);
    }లేకపోతే{
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}
స్థిర శూన్య jsonObjectCompute(sqlite3_context *ctx, పూర్ణాంకం isFinal){
  JsonString *pStr;
  పూర్ణాంకం flags = SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(ctx));
  pStr = (JsonString*)sqlite3_aggregate_context(ctx, 0);
  ఒకవేళ( pStr ){
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);  /* Ensure it is zero-terminated */
    jsonStringTrimOneChar(pOgStr);    /* Remove the zero terminator */
    pStr->pCtx = ctx;
    ఒకవేళ( pStr->eErr ){
      jsonReturnString(pStr, 0, 0);
      తిరిగి;
    }
    ఒకవేళ( pStr->zBuf[0]!='{' ){
      /* The string contains null entries that need to be removed */
      u64 i, j;
      పూర్ణాంకం లోపల_Str = 0;
      ఒకవేళ( !isFinal ){
        /* Work with a temporary copy of the string if this is not the
        ** final result */
        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->n_వాడిన+1);
        pStr = &tmpStr;
        ఒకవేళ( pStr->eErr ){
          jsonReturnString(pStr, 0, 0);
          తిరిగి;
        }
        jsonStringTrimOneChar(pStr);  /* Remove zero terminator */
      }
      /* Fix up the string by changing the initial "@" flag back to
      ** to "{" and removing all subsequence "@" entries, with their
      ** associated comma delimeters. */
      pStr->zBuf[0] = '{';
      కొరకు(i=j=1; i<pStr->n_వాడిన; i++){
        అక్షరం c = pStr->zBuf[i];
        ఒకవేళ( c=='"' ){
          లోపల_Str = !లోపల_Str;
          pStr->zBuf[j++] = '"';
        }లేకపోతే ఒకవేళ( c=='\\' ){
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        }లేకపోతే ఒకవేళ( c=='@' && !లోపల_Str ){
          ప్రతిజ్ఞా( i+1<pStr->n_వాడిన );
          ఒకవేళ( pStr->zBuf[i+1]==',' ){
            i++;
          }లేకపోతే ఒకవేళ( pStr->zBuf[j-1]==',' ){
            j--;
          }
        }లేకపోతే{
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;  /* Restore zero terminator */
      pStr->n_వాడిన = j;    /* Truncate the string */
    }
    ఒకవేళ( flags & JSON_BLOB ){
      jsonReturnStringAsBlob(pStr);
      ఒకవేళ( isFinal ){
        ఒకవేళ( !pStr->bStatic ) sqlite3RCStrUnref(pStr->zBuf);
      }లేకపోతే{
        jsonStringTrimOneChar(pOgStr);
      }
    }లేకపోతే ఒకవేళ( isFinal ){
      sqlite3_ఫలితం_పాఠ్యం(ctx, pStr->zBuf, (పూర్ణాంకం)pStr->n_వాడిన,
                          pStr->bStatic ? SQLITE_TRANSIENT :
                          sqlite3RCStrUnref);
      pStr->bStatic = 1;
    }లేకపోతే{
      sqlite3_ఫలితం_పాఠ్యం(ctx, pStr->zBuf, (పూర్ణాంకం)pStr->n_వాడిన, SQLITE_TRANSIENT);
      jsonStringTrimOneChar(pOgStr);
    }
    ఒకవేళ( pStr!=pOgStr ) jsonStringReset(pStr);
  }లేకపోతే ఒకవేళ( flags & JSON_BLOB ){
    స్థిర స్థిరరాశి గుర్తులేని అక్షరం ఖాళీలు_Object = 0x0c;
    sqlite3_ఫలితం_blob(ctx, &ఖాళీలు_Object, 1, SQLITE_STATIC); 
  }లేకపోతే{
    sqlite3_ఫలితం_పాఠ్యం(ctx, "{}", 2, SQLITE_STATIC);
  }
  sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
}
స్థిర శూన్య json_Object_విలువ(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 0);
}
స్థిర శూన్య jsonObjectFinal(sqlite3_context *ctx){
  jsonObjectCompute(ctx, 1);
}



#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/****************************************************************************
** The json_each virtual table
****************************************************************************/
రకనిర్వచనం నిర్మాణం Json_మాతృ Json_మాతృ;
నిర్మాణం Json_మాతృ {
  u32 i_తల;                 /* Start of object or array */
  u32 i_విలువ;                /* Start of the value */
  u32 i_చివర;                  /* First byte past the end */
  u32 n_మార్గం;                 /* Length of path */
  i64 i_కీలకం;                  /* Key for JSONB_ARRAY */
};

రకనిర్వచనం నిర్మాణం JsonEachCursor JsonEachCursor;
నిర్మాణం JsonEachCursor {
  sqlite3_vtab_cursor పునాది;  /* Base class - must be first */
  u32 iRowid;                /* The rowid */
  u32 i;                     /* Index in sParse.aBlob[] of current row */
  u32 i_చివర;                  /* EOF when i equals or exceeds this value */
  u32 n_మూలం;                 /* Size of the root path in bytes */
  u8 eType;                  /* Type of the container for element i */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
  u8 e_బహుళకం;                  /* 1 for json_each().  2 for jsonb_each() */
  u32 n_మాతృ;               /* Current nesting depth */
  u32 n_మాతృ_Alloc;          /* Space allocated for aParent[] */
  Json_మాతృ *a_మాతృ;       /* Parent elements of i */
  sqlite3 *db;               /* Database connection */
  JsonString మార్గం;           /* Current path */
  JsonParse sParse;          /* Parse of the input JSON */
};
రకనిర్వచనం నిర్మాణం JsonEachConnection JsonEachConnection;
నిర్మాణం JsonEachConnection {
  sqlite3_vtab పునాది;         /* Base class - must be first */
  sqlite3 *db;               /* Database connection */
  u8 e_బహుళకం;                  /* 1 for json_each().  2 for jsonb_each() */
  u8 bRecursive;             /* True for json_tree().  False for json_each() */
};


/* Constructor for the json_each virtual table */
స్థిర పూర్ణాంకం jsonEachConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  JsonEachConnection *pNew;
  పూర్ణాంకం rc;

/* Column numbers */
#నిర్వచించు JEACH_కీలకం     0
#నిర్వచించు JEACH_విలువ   1
#నిర్వచించు JEACH_TYPE    2
#నిర్వచించు JEACH_ATOM    3
#నిర్వచించు JEACH_ID      4
#నిర్వచించు JEACH_మాతృ  5
#నిర్వచించు JEACH_FULLKEY 6
#నిర్వచించు JEACH_మార్గం    7
/* The xBestIndex method assumes that the JSON and ROOT columns are
** the last two columns in the table.  Should this ever changes, be
** sure to update the xBestIndex method. */
#నిర్వచించు JEACH_JSON    8
#నిర్వచించు JEACH_మూలం    9

  వాడని_PARAMETER(pzErr);
  వాడని_PARAMETER(argv);
  వాడని_PARAMETER(argc);
  వాడని_PARAMETER(pAux);
  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                    "json HIDDEN,root HIDDEN)");
  ఒకవేళ( rc==SQLITE_OK ){
    pNew = (JsonEachConnection*)sqlite3DbMallocZero(db, పరిమాణం(*pNew));
    *ppVtab = (sqlite3_vtab*)pNew;
    ఒకవేళ( pNew==0 ) తిరిగి SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->e_బహుళకం = argv[0][4]=='b' ? 2 : 1;
    pNew->bRecursive = argv[0][4+pNew->e_బహుళకం]=='t';
  }
  తిరిగి rc;
}

/* destructor for json_each virtual table */
స్థిర పూర్ణాంకం jsonEachDisconnect(sqlite3_vtab *pVtab){
  JsonEachConnection *p = (JsonEachConnection*)pVtab;
  sqlite3DbFree(p->db, pVtab);
  తిరిగి SQLITE_OK;
}

/* constructor for a JsonEachCursor object for json_each()/json_tree(). */
స్థిర పూర్ణాంకం jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  JsonEachConnection *pVtab = (JsonEachConnection*)p;
  JsonEachCursor *pCur;

  వాడని_PARAMETER(p);
  pCur = sqlite3DbMallocZero(pVtab->db, పరిమాణం(*pCur));
  ఒకవేళ( pCur==0 ) తిరిగి SQLITE_NOMEM;
  pCur->db = pVtab->db;
  pCur->e_బహుళకం = pVtab->e_బహుళకం;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->మార్గం);
  *ppCursor = &pCur->పునాది;
  తిరిగి SQLITE_OK;
}

/* Reset a JsonEachCursor back to its original state.  Free any memory
** held. */
స్థిర శూన్య jsonEachCursorReset(JsonEachCursor *p){
  jsonParseReset(&p->sParse);
  jsonStringReset(&p->మార్గం);
  sqlite3DbFree(p->db, p->a_మాతృ);
  p->iRowid = 0;
  p->i = 0;
  p->a_మాతృ = 0;
  p->n_మాతృ = 0;
  p->n_మాతృ_Alloc = 0;
  p->i_చివర = 0;
  p->eType = 0;
}

/* Destructor for a jsonEachCursor object */
స్థిర పూర్ణాంకం jsonEachClose(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  jsonEachCursorReset(p);
  
  sqlite3DbFree(p->db, cur);
  తిరిగి SQLITE_OK;
}

/* Return TRUE if the jsonEachCursor object has been advanced off the end
** of the JSON object */
స్థిర పూర్ణాంకం jsonEachEof(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  తిరిగి p->i >= p->i_చివర;
}

/*
** If the cursor is currently pointing at the label of a object entry,
** then return the index of the value.  For all other cases, return the
** current pointer position, which is the value.
*/
స్థిర పూర్ణాంకం jsonSkipLabel(JsonEachCursor *p){
  ఒకవేళ( p->eType==JSONB_OBJECT ){
    u32 sz = 0;
    u32 n = jsonb_Payload_కొలత(&p->sParse, p->i, &sz);
    sz += p->i + n;
    ఒకవేళ( sz >= p->sParse.nBlob ) sz = p->i;
    తిరిగి sz;
  }లేకపోతే{
    తిరిగి p->i;
  }
}

/*
** Append the path name for the current element.
*/
స్థిర శూన్య json_Append_మార్గం_Name(JsonEachCursor *p){
  ప్రతిజ్ఞా( p->n_మాతృ>0 );
  ప్రతిజ్ఞా( p->eType==JSONB_ARRAY || p->eType==JSONB_OBJECT );
  ఒకవేళ( p->eType==JSONB_ARRAY ){
    jsonPrintf(30, &p->మార్గం, "[%lld]", p->a_మాతృ[p->n_మాతృ-1].i_కీలకం);
  }లేకపోతే{
    u32 n, sz = 0, k, i;
    స్థిరరాశి అక్షరం *z;
    పూర్ణాంకం needQuote = 0;
    n = jsonb_Payload_కొలత(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (స్థిరరాశి అక్షరం*)&p->sParse.aBlob[k];
    ఒకవేళ( sz==0 || !sqlite3Isalpha(z[0]) ){
      needQuote = 1;
    }లేకపోతే{
      కొరకు(i=0; i<sz; i++){
        ఒకవేళ( !sqlite3Isalnum(z[i]) ){
          needQuote = 1;
          ఆపు;
        }
      }
    }
    ఒకవేళ( needQuote ){
      jsonPrintf(sz+4,&p->మార్గం,".\"%.*s\"", sz, z);
    }లేకపోతే{
      jsonPrintf(sz+2,&p->మార్గం,".%.*s", sz, z);
    }
  }
}

/* Report a "malformed JSON" or OOM error against the cursor.
*/
స్థిర పూర్ణాంకం json_Each_Malformed_ఆదానం(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur->pVtab->z_Err_సందేశం);
  cur->pVtab->z_Err_సందేశం = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset((JsonEachCursor*)cur);
  తిరిగి cur->pVtab->z_Err_సందేశం ? SQLITE_ERROR : SQLITE_NOMEM;
}

/* Advance the cursor to the next element for json_tree() */
స్థిర పూర్ణాంకం json_Each_తదుపరి(sqlite3_vtab_cursor *cur){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( p->bRecursive ){
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonb_Payload_కొలత(&p->sParse, i, &sz);
    ఒకవేళ( n==0 )తిరిగి json_Each_Malformed_ఆదానం(cur);
    ఒకవేళ( x==JSONB_OBJECT || x==JSONB_ARRAY ){
      Json_మాతృ *p_మాతృ;
      ఒకవేళ( p->n_మాతృ>=p->n_మాతృ_Alloc ){
        Json_మాతృ *pNew;
        u64 nNew;
        nNew = p->n_మాతృ_Alloc*2 + 3;
        pNew = sqlite3DbRealloc(p->db, p->a_మాతృ, పరిమాణం(Json_మాతృ)*nNew);
        ఒకవేళ( pNew==0 ) తిరిగి SQLITE_NOMEM;
        p->n_మాతృ_Alloc = (u32)nNew;
        p->a_మాతృ = pNew;
      }
      levelChange = 1;
      p_మాతృ = &p->a_మాతృ[p->n_మాతృ];
      p_మాతృ->i_తల = p->i;
      p_మాతృ->i_విలువ = i;
      p_మాతృ->i_చివర = i + n + sz;
      p_మాతృ->i_కీలకం = -1;
      p_మాతృ->n_మార్గం = (u32)p->మార్గం.n_వాడిన;
      ఒకవేళ( p->eType && p->n_మాతృ ){
        json_Append_మార్గం_Name(p);
        ఒకవేళ( p->మార్గం.eErr ) rc = SQLITE_NOMEM;
      }
      p->n_మాతృ++;
      p->i = i + n;
    }లేకపోతే{
      p->i = i + n + sz;
    }
    అయితే( p->n_మాతృ>0 && p->i >= p->a_మాతృ[p->n_మాతృ-1].i_చివర ){
      p->n_మాతృ--;
      p->మార్గం.n_వాడిన = p->a_మాతృ[p->n_మాతృ].n_మార్గం;
      levelChange = 1;
    }
    ఒకవేళ( levelChange ){
      ఒకవేళ( p->n_మాతృ>0 ){
        Json_మాతృ *p_మాతృ = &p->a_మాతృ[p->n_మాతృ-1];
        u32 iVal = p_మాతృ->i_విలువ;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      }లేకపోతే{
        p->eType = 0;
      }
    }
  }లేకపోతే{
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonb_Payload_కొలత(&p->sParse, i, &sz);
    ఒకవేళ( n==0 )తిరిగి json_Each_Malformed_ఆదానం(cur);
    p->i = i + n + sz;
  }
  ఒకవేళ( p->eType==JSONB_ARRAY && p->n_మాతృ ){
    p->a_మాతృ[p->n_మాతృ-1].i_కీలకం++;
  }
  p->iRowid++;
  తిరిగి rc;
}

/* Length of the path for rowid==0 in bRecursive mode.
*/
స్థిర పూర్ణాంకం json_Each_మార్గం_నిడివి(JsonEachCursor *p){
  u32 n = p->మార్గం.n_వాడిన;
  అక్షరం *z = p->మార్గం.zBuf;
  ఒకవేళ( p->iRowid==0 && p->bRecursive && n>=2 ){
    అయితే( ALWAYS(n>1) ){
      n--;
      ఒకవేళ( z[n]=='[' || z[n]=='.' ){
        u32 x;
        అక్షరం cSaved = z[n];
        z[n] = 0;
        ప్రతిజ్ఞా( p->sParse.eEdit==0 );
        x = json_Lookup_అడుగు(&p->sParse, 0, z+1, 0);
        z[n] = cSaved;
        ఒకవేళ( !JSON_LOOKUP_ISERROR(x) ) ఆపు;
      }
    }
  }
  తిరిగి n;
}

/* Return the value of a column */
స్థిర పూర్ణాంకం json_Each_స్తంభం(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  పూర్ణాంకం i_స్తంభం                 /* Which column to return */
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  ఎంపిక( i_స్తంభం ){
    స్థితి JEACH_కీలకం: {
      ఒకవేళ( p->n_మాతృ==0 ){
        u32 n, j;
        ఒకవేళ( p->n_మూలం==1 ) ఆపు;
        j = json_Each_మార్గం_నిడివి(p);
        n = p->n_మూలం - j;
        ఒకవేళ( n==0 ){
          ఆపు;
        }లేకపోతే ఒకవేళ( p->మార్గం.zBuf[j]=='[' ){
          i64 x;
          sqlite3Atoi64(&p->మార్గం.zBuf[j+1], &x, n-1, SQLITE_UTF8);
          sqlite3_ఫలితం_int64(ctx, x);
        }లేకపోతే ఒకవేళ( p->మార్గం.zBuf[j+1]=='"' ){
          sqlite3_ఫలితం_పాఠ్యం(ctx, &p->మార్గం.zBuf[j+2], n-3, SQLITE_TRANSIENT);
        }లేకపోతే{
          sqlite3_ఫలితం_పాఠ్యం(ctx, &p->మార్గం.zBuf[j+1], n-1, SQLITE_TRANSIENT);
        }
        ఆపు;
      }
      ఒకవేళ( p->eType==JSONB_OBJECT ){
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      }లేకపోతే{
        ప్రతిజ్ఞా( p->eType==JSONB_ARRAY );
        sqlite3_ఫలితం_int64(ctx, p->a_మాతృ[p->n_మాతృ-1].i_కీలకం);
      }
      ఆపు;
    }
    స్థితి JEACH_విలువ: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->e_బహుళకం);
      ఒకవేళ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY ){
        sqlite3_ఫలితం_subtype(ctx, JSON_SUBTYPE);
      }
      ఆపు;
    }
    స్థితి JEACH_TYPE: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_ఫలితం_పాఠ్యం(ctx, jsonbType[eType], -1, SQLITE_STATIC);
      ఆపు;
    }
    స్థితి JEACH_ATOM: {
      u32 i = jsonSkipLabel(p);
      ఒకవేళ( (p->sParse.aBlob[i] & 0x0f)<JSONB_ARRAY ){
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      ఆపు;
    }
    స్థితి JEACH_ID: {
      sqlite3_ఫలితం_int64(ctx, (sqlite3_int64)p->i);
      ఆపు;
    }
    స్థితి JEACH_మాతృ: {
      ఒకవేళ( p->n_మాతృ>0 && p->bRecursive ){
        sqlite3_ఫలితం_int64(ctx, p->a_మాతృ[p->n_మాతృ-1].i_తల);
      }
      ఆపు;
    }
    స్థితి JEACH_FULLKEY: {
      u64 n_పునాది = p->మార్గం.n_వాడిన;
      ఒకవేళ( p->n_మాతృ ) json_Append_మార్గం_Name(p);
      sqlite3_ఫలితం_text64(ctx, p->మార్గం.zBuf, p->మార్గం.n_వాడిన,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      p->మార్గం.n_వాడిన = n_పునాది;
      ఆపు;
    }
    స్థితి JEACH_మార్గం: {
      u32 n = json_Each_మార్గం_నిడివి(p);
      sqlite3_ఫలితం_text64(ctx, p->మార్గం.zBuf, n,
                            SQLITE_TRANSIENT, SQLITE_UTF8);
      ఆపు;
    }
    అప్రమేయ: {
      sqlite3_ఫలితం_పాఠ్యం(ctx, p->మార్గం.zBuf, p->n_మూలం, SQLITE_TRANSIENT);
      ఆపు;
    }
    స్థితి JEACH_JSON: {
      ఒకవేళ( p->sParse.zJson==0 ){
        sqlite3_ఫలితం_blob(ctx, p->sParse.aBlob, p->sParse.nBlob,
                            SQLITE_TRANSIENT);
      }లేకపోతే{
        sqlite3_ఫలితం_పాఠ్యం(ctx, p->sParse.zJson, -1, SQLITE_TRANSIENT);
      }
      ఆపు;
    }
  }
  తిరిగి SQLITE_OK;
}

/* Return the current rowid value */
స్థిర పూర్ణాంకం jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  *pRowid = p->iRowid;
  తిరిగి SQLITE_OK;
}

/* The query strategy is to look for an equality constraint on the json
** column.  Without such a constraint, the table cannot operate.  idxNum is
** 1 if the constraint is found, 3 if the constraint and zRoot are found,
** and 0 otherwise.
*/
స్థిర పూర్ణాంకం json_Each_ఉత్తమ_సూచీ(
  sqlite3_vtab *tab,
  sqlite3_సూచీ_info *pIdxInfo
){
  పూర్ణాంకం i;                     /* Loop counter or computed array index */
  పూర్ణాంకం aIdx[2];               /* Index of constraints for JSON and ROOT */
  పూర్ణాంకం unusable_ముసుగు = 0;      /* Mask of unusable JSON and ROOT constraints */
  పూర్ణాంకం idx_ముసుగు = 0;           /* Mask of usable == constraints JSON and ROOT */
  స్థిరరాశి నిర్మాణం sqlite3_సూచీ_constraint *pConstraint;

  /* This implementation assumes that JSON and ROOT are the last two
  ** columns in the table */
  ప్రతిజ్ఞా( JEACH_మూలం == JEACH_JSON+1 );
  వాడని_PARAMETER(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  కొరకు(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    పూర్ణాంకం i_నిలువు;
    పూర్ణాంకం i_ముసుగు;
    ఒకవేళ( pConstraint->i_స్తంభం < JEACH_JSON ) కొనసాగు;
    i_నిలువు = pConstraint->i_స్తంభం - JEACH_JSON;
    ప్రతిజ్ఞా( i_నిలువు==0 || i_నిలువు==1 );
    testcase( i_నిలువు==0 );
    i_ముసుగు = 1 << i_నిలువు;
    ఒకవేళ( pConstraint->usable==0 ){
      unusable_ముసుగు |= i_ముసుగు;
    }లేకపోతే ఒకవేళ( pConstraint->op==SQLITE_సూచీ_CONSTRAINT_EQ ){
      aIdx[i_నిలువు] = i;
      idx_ముసుగు |= i_ముసుగు;
    }
  }
  ఒకవేళ( pIdxInfo->n_క్రమం_By>0
   && pIdxInfo->a_క్రమం_By[0].i_స్తంభం<0
   && pIdxInfo->a_క్రమం_By[0].desc==0
  ){
    pIdxInfo->క్రమం_By_Consumed = 1;
  }

  ఒకవేళ( (unusable_ముసుగు & ~idx_ముసుగు)!=0 ){
    /* If there are any unusable constraints on JSON or ROOT, then reject
    ** this entire plan */
    తిరిగి SQLITE_CONSTRAINT;
  }
  ఒకవేళ( aIdx[0]<0 ){
    /* No JSON input.  Leave estimatedCost at the huge value that it was
    ** initialized to to discourage the query planner from selecting this
    ** plan. */
    pIdxInfo->idx_అంకె = 0;
  }లేకపోతే{
    pIdxInfo->estimated_ఖర్చు = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argv_సూచీ = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    ఒకవేళ( aIdx[1]<0 ){
      pIdxInfo->idx_అంకె = 1;  /* Only JSON supplied.  Plan 1 */
    }లేకపోతే{
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argv_సూచీ = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_అంకె = 3;  /* Both JSON and ROOT are supplied.  Plan 3 */
    }
  }
  తిరిగి SQLITE_OK;
}

/* Start a search on a new JSON string */
స్థిర పూర్ణాంకం jsonEachFilter(
  sqlite3_vtab_cursor *cur,
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  JsonEachCursor *p = (JsonEachCursor*)cur;
  స్థిరరాశి అక్షరం *z_మూలం = 0;
  u32 i, n, sz;

  వాడని_PARAMETER(idxStr);
  వాడని_PARAMETER(argc);
  jsonEachCursorReset(p);
  ఒకవేళ( idx_అంకె==0 ) తిరిగి SQLITE_OK;
  memset(&p->sParse, 0, పరిమాణం(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  ఒకవేళ( jsonArgIsJsonb(argv[0], &p->sParse) ){
    /* We have JSONB */
  }లేకపోతే{
    p->sParse.zJson = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    p->sParse.nJson = sqlite3_విలువ_బైట్లు(argv[0]);
    ఒకవేళ( p->sParse.zJson==0 ){
      p->i = p->i_చివర = 0;
      తిరిగి SQLITE_OK;
    }      
    ఒకవేళ( json_Convert_పాఠ్యం_గమ్యం_Blob(&p->sParse, 0) ){
      ఒకవేళ( p->sParse.oom ){
        తిరిగి SQLITE_NOMEM;
      }
      తిరిగి json_Each_Malformed_ఆదానం(cur);
    }
  }
  ఒకవేళ( idx_అంకె==3 ){
    z_మూలం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);
    ఒకవేళ( z_మూలం==0 ) తిరిగి SQLITE_OK;
    ఒకవేళ( z_మూలం[0]!='$' ){
      sqlite3_free(cur->pVtab->z_Err_సందేశం);
      cur->pVtab->z_Err_సందేశం = json_Bad_మార్గం_Error(0, z_మూలం, 0);
      jsonEachCursorReset(p);
      తిరిగి cur->pVtab->z_Err_సందేశం ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->n_మూలం = sqlite3Strlen30(z_మూలం);
    ఒకవేళ( z_మూలం[1]==0 ){
      i = p->i = 0;
      p->eType = 0;
    }లేకపోతే{
      i = json_Lookup_అడుగు(&p->sParse, 0, z_మూలం+1, 0);
      ఒకవేళ( JSON_LOOKUP_ISERROR(i) ){
        ఒకవేళ( i==JSON_LOOKUP_NOTFOUND ){
          p->i = 0;
          p->eType = 0;
          p->i_చివర = 0;
          తిరిగి SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->z_Err_సందేశం);
        cur->pVtab->z_Err_సందేశం = json_Bad_మార్గం_Error(0, z_మూలం, 0);
        jsonEachCursorReset(p);
        తిరిగి cur->pVtab->z_Err_సందేశం ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      ఒకవేళ( p->sParse.iLabel ){
        p->i = p->sParse.iLabel;
        p->eType = JSONB_OBJECT;
      }లేకపోతే{
        p->i = i;
        p->eType = JSONB_ARRAY;
      }
    }
    jsonAppendRaw(&p->మార్గం, z_మూలం, p->n_మూలం);
  }లేకపోతే{
    i = p->i = 0;
    p->eType = 0;
    p->n_మూలం = 1;
    jsonAppendRaw(&p->మార్గం, "$", 1);
  }
  p->n_మాతృ = 0;
  n = jsonb_Payload_కొలత(&p->sParse, i, &sz);
  p->i_చివర = i+n+sz;
  ఒకవేళ( (p->sParse.aBlob[i] & 0x0f)>=JSONB_ARRAY && !p->bRecursive ){
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->a_మాతృ = sqlite3DbMallocZero(p->db, పరిమాణం(Json_మాతృ));
    ఒకవేళ( p->a_మాతృ==0 ) తిరిగి SQLITE_NOMEM;
    p->n_మాతృ = 1;
    p->n_మాతృ_Alloc = 1;
    p->a_మాతృ[0].i_కీలకం = 0;
    p->a_మాతృ[0].i_చివర = p->i_చివర;
    p->a_మాతృ[0].i_తల = p->i;
    p->a_మాతృ[0].i_విలువ = i;
  }
  తిరిగి SQLITE_OK;
}

/* The methods of the json_each virtual table */
స్థిర sqlite3_module jsonEachModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  jsonEachConnect,           /* xConnect */
  json_Each_ఉత్తమ_సూచీ,         /* xBestIndex */
  jsonEachDisconnect,        /* xDisconnect */
  0,                         /* xDestroy */
  jsonEachOpen,              /* xOpen - open a cursor */
  jsonEachClose,             /* xClose - close a cursor */
  jsonEachFilter,            /* xFilter - configure scan constraints */
  json_Each_తదుపరి,              /* xNext - advance a cursor */
  jsonEachEof,               /* xEof - check for end of scan */
  json_Each_స్తంభం,            /* xColumn - read data */
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
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_JSON) */

/*
** Register JSON functions.
*/
శూన్య sqlite3_రిజిస్టరు_Json_Functions(శూన్య){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_JSON
  స్థిర FuncDef aJsonFunc[] = {
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
    JFUNCTION(json_array_చొప్పించు, -1,1,1, 1,0,JSON_AINS,  json_సమితి_Func),
    JFUNCTION(jsonb_array_చొప్పించు,-1,1,0, 1,1,JSON_AINS,  json_సమితి_Func),
    JFUNCTION(json_array_నిడివి,  1,1,0, 0,0,0,          json_Array_నిడివి_Func),
    JFUNCTION(json_array_నిడివి,  2,1,0, 0,0,0,          json_Array_నిడివి_Func),
    JFUNCTION(json_error_position,1,1,0, 0,0,0,          jsonErrorFunc),
    JFUNCTION(json_వెలికితీయి,      -1,1,1, 0,0,0,          json_వెలికితీయి_Func),
    JFUNCTION(jsonb_వెలికితీయి,     -1,1,0, 0,1,0,          json_వెలికితీయి_Func),
    JFUNCTION(->,                 2,1,1, 0,0,JSON_JSON,  json_వెలికితీయి_Func),
    JFUNCTION(->>,                2,1,0, 0,0,JSON_SQL,   json_వెలికితీయి_Func),
    JFUNCTION(json_చొప్పించు,       -1,1,1, 1,0,0,          json_సమితి_Func),
    JFUNCTION(jsonb_చొప్పించు,      -1,1,0, 1,1,0,          json_సమితి_Func),
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
    JFUNCTION(json_సమితి,          -1,1,1, 1,0,JSON_ISSET, json_సమితి_Func),
    JFUNCTION(jsonb_సమితి,         -1,1,0, 1,1,JSON_ISSET, json_సమితి_Func),
    JFUNCTION(json_type,          1,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_type,          2,1,0, 0,0,0,          jsonTypeFunc),
    JFUNCTION(json_valid,         1,1,0, 0,0,0,          jsonValidFunc),
    JFUNCTION(json_valid,         2,1,0, 0,0,0,          jsonValidFunc),
#ఒకవేళ SQLITE_DEBUG
    JFUNCTION(json_parse,         1,1,0, 0,0,0,          jsonParseFunc),
#అయితే_ముగింపు
    WAGGREGATE(json_సమూహం_array,  1, 0, 0,
       json_Array_అడుగు, jsonArrayFinal, json_Array_విలువ, json_సమూహం_Inverse,
       SQLITE_SUBTYPE|SQLITE_ఫలితం_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_సమూహం_array, 1, JSON_BLOB, 0,
       json_Array_అడుగు, jsonArrayFinal, json_Array_విలువ, json_సమూహం_Inverse,
       SQLITE_SUBTYPE|SQLITE_ఫలితం_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(json_సమూహం_object, 2, 0, 0,
       json_Object_అడుగు, jsonObjectFinal, json_Object_విలువ, json_సమూహం_Inverse,
       SQLITE_SUBTYPE|SQLITE_ఫలితం_SUBTYPE|SQLITE_UTF8|SQLITE_DETERMINISTIC),
    WAGGREGATE(jsonb_సమూహం_object,2, JSON_BLOB, 0,
       json_Object_అడుగు, jsonObjectFinal, json_Object_విలువ, json_సమూహం_Inverse,
       SQLITE_SUBTYPE|SQLITE_ఫలితం_SUBTYPE|SQLITE_UTF8|
       SQLITE_DETERMINISTIC)
  };
  sqlite3_చొప్పించు_Builtin_Funcs(aJsonFunc, Array_కొలత(aJsonFunc));
#అయితే_ముగింపు
}

#ఒకవేళ  !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
/*
** Register the JSON table-valued function named zName and return a
** pointer to its Module object.  Return NULL if something goes wrong.
*/
Module *sqlite3_Json_Vtab_రిజిస్టరు(sqlite3 *db, స్థిరరాశి అక్షరం *zName){
  గుర్తులేని పూర్ణాంకం i;
  స్థిర స్థిరరాశి అక్షరం *azModule[] = {
    "json_each", "json_tree", "jsonb_each", "jsonb_tree"
  };
  ప్రతిజ్ఞా( sqlite3_Hash_కనుగొను(&db->aModule, zName)==0 );
  కొరకు(i=0; i<పరిమాణం(azModule)/పరిమాణం(azModule[0]); i++){
    ఒకవేళ( sqlite3StrICmp(azModule[i],zName)==0 ){
      తిరిగి sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  తిరిగి 0;
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON) */
