/*
** 2007 May 7
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file defines various limits of what SQLite can process.
*/

/*
** The maximum length of a TEXT or BLOB in bytes.   This also
** limits the size of a row in a table or index.
**
** The hard limit is the ability of a 32-bit signed integer
** to count the size: 2^31-1 or 2147483647.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_ڊيگهائي
# define SQLITE_MAX_ڊيگهائي 1000000000
#جيڪڏهن_پورو
#وصف_ڪريو SQLITE_MIN_ڊيگهائي 30   /* Minimum value for the length limit */

/*
** Maximum size of any single memory allocation.
**
** This is not a limit on the total amount of memory used.  This is
** a limit on the size parameter to sqlite3_malloc() and sqlite3_realloc().
**
** The upper bound is slightly less than 2GiB:  0x7ffffeff == 2,147,483,391
** This provides a 256-byte safety margin for defense against 32-bit 
** signed integer overflow bugs when computing memory allocation sizes.
** Paranoid applications might want to reduce the maximum allocation size
** further for an even larger safety margin.  0x3fffffff or 0x0fffffff
** or even smaller would be reasonable upper bounds on the size of a memory
** allocations for most applications.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_ورهاست_سائيز
# define SQLITE_MAX_ورهاست_سائيز  2147483391
#جيڪڏهن_پورو
#إذا SQLITE_MAX_ورهاست_سائيز>2147483391
# error وڌ_کان_وڌ سائيز لكل SQLITE_MAX_ورهاست_سائيز is 2147483391
#جيڪڏهن_پورو

/*
** This is the maximum number of
**
**    * Columns in a table
**    * Columns in an index
**    * Columns in a view
**    * Terms in the SET clause of an UPDATE statement
**    * Terms in the result set of a SELECT statement
**    * Terms in the GROUP BY or ORDER BY clauses of a SELECT statement.
**    * Terms in the VALUES clause of an INSERT statement
**
** The hard upper limit here is 32767.  Most database people will
** tell you that in a well-normalized database, you usually should
** not have more than a dozen or so columns in any table.  And if
** that is the case, there is no point in having more than a few
** dozen values in any of the other situations described above.
**
** An index can only have SQLITE_MAX_COLUMN columns from the user
** point of view, but the underlying b-tree that implements the index
** might have up to twice as many columns in a WITHOUT ROWID table,
** since must also store the primary key at the end.  Hence the
** column count for Index is u16 instead of i16.
*/
#إذا !defined(SQLITE_MAX_ڪالم)
# define SQLITE_MAX_ڪالم 2000
#وإلاإذا SQLITE_MAX_ڪالم>32767
# error SQLITE_MAX_ڪالم may not exceed 32767
#جيڪڏهن_پورو

/*
** The maximum length of a single SQL statement in bytes.
**
** It used to be the case that setting this value to zero would
** turn the limit off.  That is no longer true.  It is not possible
** to turn this limit off.
**
** The hard limit is the largest possible 32-bit signed integer less
** 1024, or 2147482624.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_SQL_ڊيگهائي
# define SQLITE_MAX_SQL_ڊيگهائي 1000000000
#جيڪڏهن_پورو

/*
** The maximum depth of an expression tree. The expression tree depth
** is also limited indirectly by SQLITE_MAX_SQL_LENGTH and by
** SQLITE_MAX_PARSER_DEPTH.  Reducing the maximum complexity of
** expressions can help prevent excess memory usage by hostile SQL.
**
** A value of 0 for this compile-time option causes all expression
** depth limiting code to be omitted.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_اظهار_اونهائي
# define SQLITE_MAX_اظهار_اونهائي 1000
#جيڪڏهن_پورو

/*
** The maximum depth of the LALR(1) stack used in the parser that
** interprets SQL inputs. The parser stack depth can also be limited
** indirectly by SQLITE_MAX_SQL_LENGTH.  Limiting the parser stack
** depth can help prevent excess memory usage and excess CPU stack
** usage when processing hostile SQL.
**
** Prior to version 3.45.0 (2024-01-15), the parser stack was
** hard-coded to 100 entries, and that worked fine for almost all
** applications.  So the upper bound on this limit need not be large.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_PARSER_اونهائي
# define SQLITE_MAX_PARSER_اونهائي 2500
#جيڪڏهن_پورو

/*
** The maximum number of terms in a compound SELECT statement.
** The code generator for compound SELECT statements does one
** level of recursion for each term.  A stack overflow can result
** if the number of terms is too large.  In practice, most SQL
** never has more than 3 or 4 terms.  Use a value of 0 to disable
** any limit on the number of terms in a compound SELECT.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_COMPOUND_SELECT
# define SQLITE_MAX_COMPOUND_SELECT 500
#جيڪڏهن_پورو

/*
** The maximum number of opcodes in a VDBE program.
** Not currently enforced.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_VDBE_OP
# define SQLITE_MAX_VDBE_OP 250000000
#جيڪڏهن_پورو

/*
** The maximum number of arguments to an SQL function.
**
** This value has a hard upper limit of 32767 due to storage
** constraints (it needs to fit inside a i16).  We keep it
** lower than that to prevent abuse.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_FUNCTION_ARG
# define SQLITE_MAX_FUNCTION_ARG 1000
#جيڪڏهن_پورو

/*
** The suggested maximum number of in-memory pages to use for
** the main database table and for temporary tables.
**
** IMPLEMENTATION-OF: R-30185-15359 The default suggested cache size is -2000,
** which means the cache size is limited to 2048000 bytes of memory.
** IMPLEMENTATION-OF: R-48205-43578 The default suggested cache size can be
** altered using the SQLITE_DEFAULT_CACHE_SIZE compile-time options.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_DEFAULT_CACHE_سائيز
# define SQLITE_DEFAULT_CACHE_سائيز  -2000
#جيڪڏهن_پورو

/*
** The default number of frames to accumulate in the log file before
** checkpointing the database in WAL mode.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_DEFAULT_WAL_AUTOCHECKPOINT
# define SQLITE_DEFAULT_WAL_AUTOCHECKPOINT  1000
#جيڪڏهن_پورو

/*
** The maximum number of attached databases.  This must be between 0
** and 125.  The upper bound of 125 is because the attached databases are
** counted using a signed 8-bit integer which has a maximum value of 127
** and we have to allow 2 extra counts for the "main" and "temp" databases.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_ATTACHED
# define SQLITE_MAX_ATTACHED 10
#جيڪڏهن_پورو


/*
** The maximum value of a ?nnn wildcard that the parser will accept.
** If the value exceeds 32767 then extra space is required for the Expr
** structure.  But otherwise, we believe that the number can be as large
** as a signed 32-bit integer can hold.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_VARIABLE_عدد
# define SQLITE_MAX_VARIABLE_عدد 32766
#جيڪڏهن_پورو

/* Maximum page size.  The upper bound on this value is 65536.  This a limit
** imposed by the use of 16-bit offsets within each page.
**
** Earlier versions of SQLite allowed the user to change this value at
** compile time. This is no longer permitted, on the grounds that it creates
** a library that is technically incompatible with an SQLite library
** compiled with a different limit. If a process operating on a database
** with a page-size of 65536 bytes crashes, then an instance of SQLite
** compiled with the default page-size limit will not be able to rollback
** the aborted transaction. This could lead to database corruption.
*/
#جيڪڏهن_وصف_ٿيل SQLITE_MAX_صفحو_سائيز
# undef SQLITE_MAX_صفحو_سائيز
#جيڪڏهن_پورو
#وصف_ڪريو SQLITE_MAX_صفحو_سائيز 65536


/*
** The default size of a database page.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_DEFAULT_صفحو_سائيز
# define SQLITE_DEFAULT_صفحو_سائيز 4096
#جيڪڏهن_پورو
#إذا SQLITE_DEFAULT_صفحو_سائيز>SQLITE_MAX_صفحو_سائيز
# undef SQLITE_DEFAULT_صفحو_سائيز
# define SQLITE_DEFAULT_صفحو_سائيز SQLITE_MAX_صفحو_سائيز
#جيڪڏهن_پورو

/*
** Ordinarily, if no value is explicitly provided, SQLite creates databases
** with page size SQLITE_DEFAULT_PAGE_SIZE. However, based on certain
** device characteristics (sector-size and atomic write() support),
** SQLite may choose a larger value. This constant is the maximum value
** SQLite will choose on its own.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_DEFAULT_صفحو_سائيز
# define SQLITE_MAX_DEFAULT_صفحو_سائيز 8192
#جيڪڏهن_پورو
#إذا SQLITE_MAX_DEFAULT_صفحو_سائيز>SQLITE_MAX_صفحو_سائيز
# undef SQLITE_MAX_DEFAULT_صفحو_سائيز
# define SQLITE_MAX_DEFAULT_صفحو_سائيز SQLITE_MAX_صفحو_سائيز
#جيڪڏهن_پورو
#إذا SQLITE_MAX_DEFAULT_صفحو_سائيز<SQLITE_DEFAULT_صفحو_سائيز
# undef SQLITE_MAX_DEFAULT_صفحو_سائيز
# define SQLITE_MAX_DEFAULT_صفحو_سائيز SQLITE_DEFAULT_صفحو_سائيز
#جيڪڏهن_پورو


/*
** Maximum number of pages in one database file.
**
** This is really just the default value for the max_page_count pragma.
** This value can be lowered (or raised) at run-time using the
** max_page_count macro.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_صفحو_ڳڻپ
# define SQLITE_MAX_صفحو_ڳڻپ 0xfffffffe /* 4294967294 */
#جيڪڏهن_پورو

/*
** Maximum length (in bytes) of the pattern in a LIKE or GLOB
** operator.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_LIKE_نمونو_ڊيگهائي
# define SQLITE_MAX_LIKE_نمونو_ڊيگهائي 50000
#جيڪڏهن_پورو

/*
** Maximum depth of recursion for triggers.
**
** A value of 1 means that a trigger program will not be able to itself
** fire any triggers. A value of 0 means that no trigger programs at all
** may be executed.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_TRIGGER_اونهائي
# define SQLITE_MAX_TRIGGER_اونهائي 1000
#جيڪڏهن_پورو

/*
** Maximum number of objects defined by a single database schema.
** Objects include:
**
**   *  tables (including the sqlite_schema table)
**   *  virtual tables
**   *  named indexes
**   *  indexes created automatically by UNIQUE and PRIMARY KEY constraints
**   *  triggers
**   *  views
**
** The total of all of the above is the number of objects in the schema,
** and that number may not exceed this value.
**
** The maximum number of objects is restricted to forestall theoretical
** signed integer overflow attacks using databases with billions of
** schema objects.  Such attacks are "theoretical" because memory and
** disk space constraints would be reached long before billions of schema
** objects could be created.  Even so, it seems good to have a well defined
** upper limit on the complexity of the schema, for defense in depth.
** No real-world application should ever get anywhere close to hitting
** this limit.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_SCHEMA
# define SQLITE_MAX_SCHEMA 10000000
#جيڪڏهن_پورو

/*
** Maximum number of SQL statements allowed in the body of a single
** trigger.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_TRIGGER_قدمن
# define SQLITE_MAX_TRIGGER_قدمن 65000
#جيڪڏهن_پورو

/*
** Maximum number of CTE cursors (approximately).  This limit will be
** exceeded if any CTE uses a cursor number larger than this value, even
** of some of the cursors are used for something other than CTEs.
**
** The purpose of this limit is to prevent resource exhaustion caused
** by hostile SQL inputs such as:
**
**    WITH c0 AS (SELECT 1 AS x),
**         c1 AS (SELECT a.x AS x FROM c0 a, c0 b),
**         c2 AS (SELECT a.x AS x FROM c1 a, c1 b),
**         ...
**         c17 AS (SELECT a.x AS x FROM c16 a, c16 b)
**    SELECT count(*) FROM c17;
**
** See Bug 2026-10-04T05:35:14Z
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MAX_CTES
# define SQLITE_MAX_CTES 50000
#جيڪڏهن_پورو
