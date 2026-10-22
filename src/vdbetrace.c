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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "vdbeInt.h"

#ifndef SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
સ્થિર i64 findNextHostParameter(અચલ અક્ષર *zSql, i64 *pnToken){
  પૂર્ણાંક tokenType;
  i64 nTotal = 0;
  i64 n;

  *pnToken = 0;
  જ્યારે( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    પ્રતિજ્ઞા( n>0 && tokenType!=TK_ILLEGAL );
    જો( tokenType==TK_VARIABLE ){
      *pnToken = n;
      તોડો;
    }
    nTotal += n;
    zSql += n;
  }
  પાછા nTotal;
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
અક્ષર *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  અચલ અક્ષર *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  પૂર્ણાંક idx = 0;             /* Index of a host parameter */
  પૂર્ણાંક nextIndex = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  પૂર્ણાંક i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum out;            /* Accumulate the output here */
#ifndef SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#endif

  db = p->db;
  sqlite3StrAccumInit(&out, 0, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  જો( db->nVdbeExec>1 ){
    જ્યારે( *zRawSql ){
      અચલ અક્ષર *zStart = zRawSql;
      જ્યારે( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&out, "-- ", 3);
      પ્રતિજ્ઞા( (zRawSql - zStart) > 0 );
      sqlite3_str_append(&out, zStart, (પૂર્ણાંક)(zRawSql-zStart));
    }
  }નહીંતર જો( p->nVar==0 ){
    sqlite3_str_append(&out, zRawSql, sqlite3Strlen30(zRawSql));
  }નહીંતર{
    જ્યારે( zRawSql[0] ){
      n = findNextHostParameter(zRawSql, &nToken);
      પ્રતિજ્ઞા( n>0 );
      sqlite3_str_append(&out, zRawSql, n);
      zRawSql += n;
      પ્રતિજ્ઞા( zRawSql[0] || nToken==0 );
      જો( nToken==0 ) તોડો;
      જો( zRawSql[0]=='?' ){
        જો( nToken>1 ){
          પ્રતિજ્ઞા( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }નહીંતર{
          idx = nextIndex;
        }
      }નહીંતર{
        પ્રતિજ્ઞા( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3VdbeParameterIndex(p, zRawSql, nToken);
        પ્રતિજ્ઞા( idx>0 );
      }
      zRawSql += nToken;
      nextIndex = MAX(idx + 1, nextIndex);
      પ્રતિજ્ઞા( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      જો( pVar->flags & MEM_Null ){
        sqlite3_str_append(&out, "NULL", 4);
      }નહીંતર જો( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&out, "%lld", pVar->u.i);
      }નહીંતર જો( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&out, "%!.15g", pVar->u.r);
      }નહીંતર જો( pVar->flags & MEM_Str ){
        પૂર્ણાંક nOut;  /* Number of bytes of the string text to include in output */
#ifndef SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        જો( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, કદ(utf8));
          utf8.db = db;
          sqlite3VdbeMemSetStr(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          જો( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            out.accError = SQLITE_NOMEM;
            out.nAlloc = 0;
          }
          pVar = &utf8;
        }
#endif
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        જો( nOut>SQLITE_TRACE_SIZE_LIMIT ){
          nOut = SQLITE_TRACE_SIZE_LIMIT;
          જ્યારે( nOut<pVar->n && (pVar->z[nOut]&0xc0)==0x80 ){ nOut++; }
        }
#endif    
        sqlite3_str_appendf(&out, "'%.*q'", nOut, pVar->z);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        જો( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
#ifndef SQLITE_OMIT_UTF16
        જો( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#endif
      }નહીંતર જો( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&out, "zeroblob(%d)", pVar->u.nZero);
      }નહીંતર{
        પૂર્ણાંક nOut;  /* Number of bytes of the blob to include in output */
        પ્રતિજ્ઞા( pVar->flags & MEM_Blob );
        sqlite3_str_append(&out, "x'", 2);
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        જો( nOut>SQLITE_TRACE_SIZE_LIMIT ) nOut = SQLITE_TRACE_SIZE_LIMIT;
#endif
        માટે(i=0; i<nOut; i++){
          sqlite3_str_appendf(&out, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&out, "'", 1);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        જો( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
      }
    }
  }
  જો( out.accError ) sqlite3_str_reset(&out);
  પાછા sqlite3StrAccumFinish(&out);
}

#endif /* #ifndef SQLITE_OMIT_TRACE */
