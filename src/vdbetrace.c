/*
** 2009 November 25
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
** This file contains code used to insert the values of host parameters
** (aka "wildcards") into the SQL text output by sqlite3_trace().
**
** The Vdbe parse-tree explainer is also found here.
*/
#include "sqliteInt.h"
#include "vdbeInt.h"

#ifndef SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
static i64 findNextHostParameter(const var_na *zSql, i64 *pnToken){
  පූර්ණ_සංඛ්‍යා tokenType;
  i64 nTotal = 0;
  i64 n;

  *pnToken = 0;
  අතරතුර( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    assert( n>0 && tokenType!=TK_ILLEGAL );
    නම්( tokenType==TK_VARIABLE ){
      *pnToken = n;
      bha_nga;
    }
    nTotal += n;
    zSql += n;
  }
  ආපසු nTotal;
}

/*
** This function returns a pointer to a nul-terminated string in memory
** obtained from sqlite3DbMalloc(). If sqlite3.nVdbeExec is 1, then the
** string contains a copy of zRawSql but with host parameters expanded to 
** their current bindings. Or, if sqlite3.nVdbeExec is greater than 1, 
** then the returned string holds a copy of zRawSql with "-- " prepended
** to each line of text.
**
** If the SQLITE_TRACE_SIZE_LIMIT macro is defined to an integer, then
** then long strings and blobs are truncated to that many bytes.  This
** can be used to prevent unreasonably large trace strings when dealing
** with large (multi-megabyte) strings and blobs.
**
** The calling function is responsible for making sure the memory returned
** is eventually freed.
**
** ALGORITHM:  Scan the input string looking for host parameters in any of
** these forms:  ?, ?N, $A, @A, :A.  Take care to avoid text within
** string literals, quoted identifier names, and comments.  For text forms,
** the host parameter index is found by scanning the prepared
** statement for the corresponding OP_Variable opcode.  Once the host
** parameter index is known, locate the value in p->aVar[].  Then render
** the value as a literal in place of the host parameter name.
*/
var_na *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  const var_na *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  පූර්ණ_සංඛ්‍යා idx = 0;             /* Index of a host parameter */
  පූර්ණ_සංඛ්‍යා nextIndex = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  පූර්ණ_සංඛ්‍යා i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum out;            /* Accumulate the output here */
#ifndef SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#endif

  db = p->db;
  sqlite3StrAccumInit(&out, 0, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  නම්( db->nVdbeExec>1 ){
    අතරතුර( *zRawSql ){
      const var_na *zStart = zRawSql;
      අතරතුර( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&out, "-- ", 3);
      assert( (zRawSql - zStart) > 0 );
      sqlite3_str_append(&out, zStart, (පූර්ණ_සංඛ්‍යා)(zRawSql-zStart));
    }
  }නැතහොත් නම්( p->nVar==0 ){
    sqlite3_str_append(&out, zRawSql, sqlite3Strlen30(zRawSql));
  }නැතහොත්{
    අතරතුර( zRawSql[0] ){
      n = findNextHostParameter(zRawSql, &nToken);
      assert( n>0 );
      sqlite3_str_append(&out, zRawSql, n);
      zRawSql += n;
      assert( zRawSql[0] || nToken==0 );
      නම්( nToken==0 ) bha_nga;
      නම්( zRawSql[0]=='?' ){
        නම්( nToken>1 ){
          assert( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }නැතහොත්{
          idx = nextIndex;
        }
      }නැතහොත්{
        assert( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3VdbeParameterIndex(p, zRawSql, nToken);
        assert( idx>0 );
      }
      zRawSql += nToken;
      nextIndex = MAX(idx + 1, nextIndex);
      assert( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      නම්( pVar->flags & MEM_Null ){
        sqlite3_str_append(&out, "NULL", 4);
      }නැතහොත් නම්( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&out, "%lld", pVar->u.i);
      }නැතහොත් නම්( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&out, "%!.15g", pVar->u.r);
      }නැතහොත් නම්( pVar->flags & MEM_Str ){
        පූර්ණ_සංඛ්‍යා nOut;  /* Number of bytes of the string text to include in output */
#ifndef SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        නම්( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, parimaa_na(utf8));
          utf8.db = db;
          sqlite3VdbeMemSetStr(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          නම්( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            out.accError = SQLITE_NOMEM;
            out.nAlloc = 0;
          }
          pVar = &utf8;
        }
#endif
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        නම්( nOut>SQLITE_TRACE_SIZE_LIMIT ){
          nOut = SQLITE_TRACE_SIZE_LIMIT;
          අතරතුර( nOut<pVar->n && (pVar->z[nOut]&0xc0)==0x80 ){ nOut++; }
        }
#endif    
        sqlite3_str_appendf(&out, "'%.*q'", nOut, pVar->z);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        නම්( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
#ifndef SQLITE_OMIT_UTF16
        නම්( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#endif
      }නැතහොත් නම්( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&out, "zeroblob(%d)", pVar->u.nZero);
      }නැතහොත්{
        පූර්ණ_සංඛ්‍යා nOut;  /* Number of bytes of the blob to include in output */
        assert( pVar->flags & MEM_Blob );
        sqlite3_str_append(&out, "x'", 2);
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        නම්( nOut>SQLITE_TRACE_SIZE_LIMIT ) nOut = SQLITE_TRACE_SIZE_LIMIT;
#endif
        සඳහා(i=0; i<nOut; i++){
          sqlite3_str_appendf(&out, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&out, "'", 1);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        නම්( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
      }
    }
  }
  නම්( out.accError ) sqlite3_str_reset(&out);
  ආපසු sqlite3StrAccumFinish(&out);
}

#endif /* #ifndef SQLITE_OMIT_TRACE */
