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
static i64 findNextHostParameter(const char *zSql, i64 *pnToken){
  정수 tokenType;
  i64 nTotal = 0;
  i64 n;

  *pnToken = 0;
  동안( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    assert( n>0 && tokenType!=TK_ILLEGAL );
    만약( tokenType==TK_VARIABLE ){
      *pnToken = n;
      break;
    }
    nTotal += n;
    zSql += n;
  }
  반환 nTotal;
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
char *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  const char *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  정수 idx = 0;             /* Index of a host parameter */
  정수 nextIndex = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  정수 i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum out;            /* Accumulate the output here */
#ifndef SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#endif

  db = p->db;
  sqlite3StrAccumInit(&out, 0, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  만약( db->nVdbeExec>1 ){
    동안( *zRawSql ){
      const char *zStart = zRawSql;
      동안( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&out, "-- ", 3);
      assert( (zRawSql - zStart) > 0 );
      sqlite3_str_append(&out, zStart, (정수)(zRawSql-zStart));
    }
  }아니면 만약( p->nVar==0 ){
    sqlite3_str_append(&out, zRawSql, sqlite3Strlen30(zRawSql));
  }아니면{
    동안( zRawSql[0] ){
      n = findNextHostParameter(zRawSql, &nToken);
      assert( n>0 );
      sqlite3_str_append(&out, zRawSql, n);
      zRawSql += n;
      assert( zRawSql[0] || nToken==0 );
      만약( nToken==0 ) break;
      만약( zRawSql[0]=='?' ){
        만약( nToken>1 ){
          assert( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }아니면{
          idx = nextIndex;
        }
      }아니면{
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
      만약( pVar->flags & MEM_Null ){
        sqlite3_str_append(&out, "NULL", 4);
      }아니면 만약( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&out, "%lld", pVar->u.i);
      }아니면 만약( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&out, "%!.15g", pVar->u.r);
      }아니면 만약( pVar->flags & MEM_Str ){
        정수 nOut;  /* Number of bytes of the string text to include in output */
#ifndef SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        만약( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, sizeof(utf8));
          utf8.db = db;
          sqlite3VdbeMemSetStr(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          만약( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            out.accError = SQLITE_NOMEM;
            out.nAlloc = 0;
          }
          pVar = &utf8;
        }
#endif
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        만약( nOut>SQLITE_TRACE_SIZE_LIMIT ){
          nOut = SQLITE_TRACE_SIZE_LIMIT;
          동안( nOut<pVar->n && (pVar->z[nOut]&0xc0)==0x80 ){ nOut++; }
        }
#endif    
        sqlite3_str_appendf(&out, "'%.*q'", nOut, pVar->z);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        만약( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
#ifndef SQLITE_OMIT_UTF16
        만약( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#endif
      }아니면 만약( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&out, "zeroblob(%d)", pVar->u.nZero);
      }아니면{
        정수 nOut;  /* Number of bytes of the blob to include in output */
        assert( pVar->flags & MEM_Blob );
        sqlite3_str_append(&out, "x'", 2);
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        만약( nOut>SQLITE_TRACE_SIZE_LIMIT ) nOut = SQLITE_TRACE_SIZE_LIMIT;
#endif
        위해(i=0; i<nOut; i++){
          sqlite3_str_appendf(&out, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&out, "'", 1);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        만약( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
      }
    }
  }
  만약( out.accError ) sqlite3_str_reset(&out);
  반환 sqlite3StrAccumFinish(&out);
}

#endif /* #ifndef SQLITE_OMIT_TRACE */
